// ==WindhawkMod==
// @id              tourne-menus
// @name            Tourne Menus
// @description     Tourne Desktop's menus in File Explorer, the Open and Save dialogs and Windows' own menus: its look, the wheel scrolling long menus, its hide and More options rules and how menus open
// @version         1.0.6
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @donateUrl       https://ko-fi.com/tourne
// @include         *
// @include         windhawk.exe
// @exclude         TourneDesktop.exe
// @exclude         vgc.exe
// @compilerOptions -lcomctl32 -ldwmapi -luxtheme -lgdi32 -lshlwapi -lole32 -ladvapi32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tourne Menus

Tourne Desktop draws its own menus (the bar's, Start's, the desktop's) in
Tourne's look. This mod draws the right-click menus of other places the same
way, each with its own switch:

- **File Explorer windows**: files, folders, the empty space, the navigation
  pane. Windows 11's new menu is skipped for them, so the full menu shows at
  once (hold Shift to get Windows' own menu instead).
- **Open and Save dialogs**, in every program.
- **Windows' own menus**: the ones Windows would draw as "immersive" menus
  (Explorer's taskbar and desktop when Explorer is the shell, the volume,
  network, language and Windows Security icons, Task Manager, Store apps'
  title bar menus). They're made plain menus first, as Classic Menus does.

All three start off: turn on the ones you want.

What the menus look like, how they open and what's left out of them all come
from Tourne Desktop: its Menus, Menu frame, Menus open, Leave out of
right-click menus and Move into More options settings (Studio, Taskbar). It
writes them to `HKCU\Software\Tourne\Menus` whenever its settings apply, and
this mod reads them as each menu opens, so a change shows on the next menu.
Without Tourne Desktop, menus get Tourne's green scheme.

In every menu drawn here, the mouse wheel scrolls a menu taller than the
screen (3 items a notch).

Not drawn here: menus whose items draw themselves (a few programs' own), and
the icons some shell extensions draw themselves (Open with, Send to); those
items show without them. Hold Shift while right-clicking for Windows' own
menu.

The way Windows 11's new File Explorer menu is skipped follows m417z's
"Classic context menu" Windhawk mod (MIT); the idea for this mod came from
Bolt-Scripts' Custom Cached Context Menu (MIT). No code was copied from either.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- explorer: false
  $name: File Explorer windows
  $description: >-
    Right-click menus in File Explorer windows, with Windows 11's new menu
    skipped for them (Shift + right-click still gives Windows' own).
- fileDialogs: false
  $name: Open and Save dialogs
  $description: Right-click menus in every program's Open, Save and folder dialogs.
- windowsMenus: false
  $name: Windows' own menus
  $description: >-
    The menus Windows draws as "immersive" menus: Explorer's taskbar and
    desktop (when Explorer is the shell), the volume, network, language and
    Windows Security icons, Task Manager, Store apps' title bar menus.
- excludeApps: ''
  $name: Leave these programs alone
  $description: Program file names, separated by commas, like game.exe.
*/
// ==/WindhawkModSettings==

// This file is made from tools/menus/tourne-menus.cpp by tools/menus/make-menus.py, which puts in the
// shared files where "// @inline" lines name them. Edit those, not tourne-menus.wh.cpp.

#include <windhawk_utils.h>

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <string>
#include <vector>

// Every thread its own menu state: each File Explorer window has a thread of its own.
#define MENU_STYLE_STORAGE thread_local

// @inline menu-style.cpp
// @inline motion.cpp

////////////////////////////////////////////////////////////////////////////////
// Which menus are drawn here.

struct Settings {
    bool explorer = false, dialogs = false, windowsMenus = false;
};
Settings g_settings;
bool g_inExplorer = false;  // this program is explorer.exe

std::wstring ProgramName(HMODULE module = nullptr) {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(module, path, ARRAYSIZE(path));
    if (!length || length == ARRAYSIZE(path)) return L"";
    PCWSTR name = wcsrchr(path, L'\\');
    return name ? name + 1 : path;
}

bool ClassIs(HWND wnd, PCWSTR name) {
    WCHAR cls[64];
    return wnd && GetClassNameW(wnd, cls, ARRAYSIZE(cls)) && !_wcsicmp(cls, name);
}

BOOL CALLBACK FindShellView(HWND child, LPARAM lp) {
    if (ClassIs(child, L"SHELLDLL_DefView") || ClassIs(child, L"DUIViewWndClassName")) {
        *(bool*)lp = true;
        return FALSE;
    }
    return TRUE;
}

// A dialog with a folder view in it: the Open, Save and folder pickers (the old and the new kind).
bool IsFileDialog(HWND root) {
    if (!ClassIs(root, L"#32770")) return false;
    bool found = false;
    EnumChildWindows(root, FindShellView, (LPARAM)&found);
    return found;
}

// Where Windows keeps the code that shows its own ("immersive") menus: as Classic Menus lists them
// (immersive-menus.cpp), read from Windows itself. shell32 and the File Explorer modules count in
// explorer.exe only (elsewhere they're a file dialog's).
constexpr PCWSTR kWindowsMenuModules[] = {
    L"Taskbar.dll", L"twinui.pcshell.dll", L"twinui.dll", L"explorer.exe", L"SndVolSSO.dll", L"InputSwitch.dll",
    L"SecurityHealthSsoUdk.dll", L"SecurityHealthSSO.dll", L"pnidui.dll", L"stobject.dll", L"Taskmgr.exe",
    L"Narrator.exe", L"ApplicationFrame.dll", L"museuxdocked.dll",
};
constexpr PCWSTR kExplorerModules[] = {L"shell32.dll", L"ExplorerFrame.dll", L"Windows.UI.FileExplorer.dll"};

bool CalledFromWindows(void* caller) {
    HMODULE module = nullptr;
    if (!caller || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       (LPCWSTR)caller, &module))
        return false;
    std::wstring name = ProgramName(module);
    for (PCWSTR m : kWindowsMenuModules)
        if (!_wcsicmp(m, name.c_str())) return true;
    if (g_inExplorer)
        for (PCWSTR m : kExplorerModules)
            if (!_wcsicmp(m, name.c_str())) return true;
    return false;
}

// Whether this menu is drawn here: by where its owner is, or who shows it.
bool WantMenu(HMENU menu, HWND owner, void* caller) {
    if (!menu || !IsMenu(menu) || !owner || GetWindowThreadProcessId(owner, nullptr) != GetCurrentThreadId()) return false;
    if (GetKeyState(VK_SHIFT) < 0) return false;  // (Windows' own, as asked)
    HWND root = GetAncestor(owner, GA_ROOT);
    if (g_settings.explorer && g_inExplorer && ClassIs(root, L"CabinetWClass")) return true;
    if (g_settings.dialogs && IsFileDialog(root)) return true;
    if (g_settings.windowsMenus && CalledFromWindows(caller)) return !ClassIs(root, L"CabinetWClass") || g_settings.explorer;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Tourne Desktop's menus, as it writes them (taskbar-host.cpp, PublishMenuLook): read again only when its
// stamp changes. Fonts are made for each scale they're asked for.

// A theme file, loaded the way theme tools do (as theme-palette.cpp does).
using LoaderLoadTheme_t = HRESULT(WINAPI*)(HANDLE themeFile, HINSTANCE instance, LPCWSTR fileName, LPCWSTR color,
                                           LPCWSTR size, HANDLE* sharableSection, LPWSTR sharableName,
                                           int sharableChars, HANDLE* nonSharableSection, LPWSTR nonSharableName,
                                           int nonSharableChars, PVOID customLoadHandler, HANDLE* reuseSection,
                                           int a, int b, BOOL emulateGlobal);
struct ThemeFileData {
    char header[7];
    void* sharableView;
    HANDLE sharableSection;
    void* nonSharableView;
    HANDLE nonSharableSection;
    char end[3];
};
using OpenThemeDataFromFile_t = HTHEME(WINAPI*)(ThemeFileData* themeFile, HWND wnd, LPCWSTR classList, DWORD flags);

// The theme's menu class, or null.
HTHEME OpenStyleMenu(std::wstring const& path) {
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    auto load = ux ? (LoaderLoadTheme_t)GetProcAddress(ux, MAKEINTRESOURCEA(92)) : nullptr;
    auto open = ux ? (OpenThemeDataFromFile_t)GetProcAddress(ux, MAKEINTRESOURCEA(16)) : nullptr;
    if (!load || !open || path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) return nullptr;
    HANDLE sharable = nullptr, nonSharable = nullptr;
    if (FAILED(load(nullptr, nullptr, path.c_str(), L"NormalColor", L"NormalSize", &sharable, nullptr, 0, &nonSharable, nullptr,
                    0, nullptr, nullptr, 0, 0, FALSE)) ||
        !sharable || !nonSharable) {
        if (sharable) CloseHandle(sharable);
        if (nonSharable) CloseHandle(nonSharable);
        return nullptr;
    }
    // Kept for as long as the program runs: the theme's handles point into it.
    auto* file = new ThemeFileData{};
    memcpy(file->header, "thmfile", 7);
    memcpy(file->end, "end", 3);
    file->sharableSection = sharable;
    file->nonSharableSection = nonSharable;
    file->sharableView = MapViewOfFile(sharable, FILE_MAP_READ, 0, 0, 0);
    file->nonSharableView = MapViewOfFile(nonSharable, FILE_MAP_READ, 0, 0, 0);
    static HWND host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    HTHEME menu = open(file, host, L"Menu", 0);
    if (menu && !IsThemePartDefined(menu, MENU_POPUPITEM, 0)) {
        CloseThemeData(menu);
        menu = nullptr;
    }
    return menu;
}

struct Published {
    DWORD stamp = 0;
    bool found = false;
    MenuLook look;  // without its font and theme
    LOGFONTW font{};
    bool hasFont = false;
    int scale = 1;  // the font's
    std::wstring theme;
    int animation = 0;
    double ms = 150;
    bool flyoutClick = false;
    motion::Anim curve;  // set: the motion engine's curve
    MenuRules rules;
};

SRWLOCK g_publishedLock = SRWLOCK_INIT;
Published g_published;  // (guarded by g_publishedLock)
DWORD g_publishedStamp = 0xFFFFFFFF;
struct ScaledFont {
    DWORD stamp;
    int scale;
    HFONT font;
};
std::vector<ScaledFont> g_fonts;  // (guarded by g_publishedLock; old ones kept: a menu may still use them)
std::wstring g_themePath;
HTHEME g_themeMenu = nullptr;

constexpr wchar_t kPublishedKey[] = L"Software\\Tourne\\Menus";

DWORD RegDword(HKEY key, PCWSTR name, DWORD fallback) {
    DWORD value = 0, size = sizeof(value), type = 0;
    return RegQueryValueExW(key, name, nullptr, &type, (BYTE*)&value, &size) == ERROR_SUCCESS && type == REG_DWORD ? value : fallback;
}

std::wstring RegText(HKEY key, PCWSTR name) {
    WCHAR buffer[2048] = L"";
    DWORD size = sizeof(buffer) - sizeof(WCHAR), type = 0;
    if (RegQueryValueExW(key, name, nullptr, &type, (BYTE*)buffer, &size) != ERROR_SUCCESS || type != REG_SZ) return L"";
    buffer[size / sizeof(WCHAR)] = 0;
    return buffer;
}

// A curve as written: "bezier X1 Y1 X2 Y2" or "spring MASS STIFFNESS DAMPING".
bool ParseCurve(std::wstring const& text, motion::Curve* out) {
    double v[4]{};
    if (swscanf_s(text.c_str(), L"bezier %lf %lf %lf %lf", &v[0], &v[1], &v[2], &v[3]) == 4) {
        *out = motion::Bezier(v[0], v[1], v[2], v[3]);
        return true;
    }
    if (swscanf_s(text.c_str(), L"spring %lf %lf %lf", &v[0], &v[1], &v[2]) == 3) {
        *out = motion::Spring(v[0], v[1], v[2]);
        return true;
    }
    return false;
}

void ReadPublished(Published& p) {
    p = Published{};
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kPublishedKey, 0, KEY_READ, &key)) return;
    p.found = true;
    p.stamp = RegDword(key, L"Stamp", 0);
    MenuLook& l = p.look;
    l.ground = RegDword(key, L"Ground", l.ground), l.line = RegDword(key, L"Line", l.line), l.edge = RegDword(key, L"Edge", l.edge);
    l.text = RegDword(key, L"Text", l.text), l.accent = RegDword(key, L"Accent", l.accent), l.dim = RegDword(key, L"Dim", l.dim);
    l.hover = RegDword(key, L"Hover", l.hover), l.hoverText = RegDword(key, L"HoverText", l.hoverText);
    l.arrow = RegDword(key, L"Arrow", l.arrow), l.frameLine = RegDword(key, L"FrameLine", l.frameLine), l.band = RegDword(key, L"Band", l.band);
    l.native = RegDword(key, L"Native", 0), l.classic = RegDword(key, L"Classic", 0), l.ownText = RegDword(key, L"OwnText", 0);
    l.frame = RegDword(key, L"Frame", 0), l.caps = RegDword(key, L"Caps", 0), l.noIcons = RegDword(key, L"NoIcons", 0);
    l.windows = RegDword(key, L"Windows", 0);
    l.frameGap = (int)RegDword(key, L"FrameGap", 6), l.frameGlow = (int)RegDword(key, L"FrameGlow", 0);
    l.frameStopCount = std::clamp((int)RegDword(key, L"FrameStopCount", 0), 0, 3);
    DWORD size = sizeof(l.frameStops), type = 0;
    if (RegQueryValueExW(key, L"FrameStops", nullptr, &type, (BYTE*)l.frameStops, &size) != ERROR_SUCCESS) l.frameStopCount = 0;
    p.scale = std::max(1, (int)RegDword(key, L"Scale", 1));
    size = sizeof(p.font);
    p.hasFont = RegQueryValueExW(key, L"Font", nullptr, &type, (BYTE*)&p.font, &size) == ERROR_SUCCESS && size == sizeof(p.font);
    p.theme = RegText(key, L"Theme");
    p.animation = (int)RegDword(key, L"Animation", 0);
    p.flyoutClick = RegDword(key, L"FlyoutClick", 0) != 0;
    p.ms = (double)RegDword(key, L"AnimationMs", 150);
    p.curve.set = ParseCurve(RegText(key, L"Curve"), &p.curve.curve);
    if (p.curve.set && p.curve.curve.spring) p.curve.springSeconds = motion::SpringSeconds(p.curve.curve);
    p.rules.hide = MenuRuleList(RegText(key, L"Hide"));
    p.rules.more = MenuRuleList(RegText(key, L"More"));
    if (std::wstring label = RegText(key, L"MoreLabel"); !label.empty()) p.rules.moreLabel = label;
    RegCloseKey(key);
}

// Tourne's pixel font, when it's installed (as Tourne Desktop's bar picks it), else Windows' menu font.
HFONT DefaultMenuFont(int scale) {
    for (PCWSTR face : {L"CozetteVector", L"scientifica"}) {
        LOGFONTW lf{};
        lf.lfHeight = -13 * scale;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfQuality = NONANTIALIASED_QUALITY;
        wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
        HFONT font = CreateFontIndirectW(&lf);
        HDC dc = GetDC(nullptr);
        HGDIOBJ old = SelectObject(dc, font);
        WCHAR got[LF_FACESIZE] = L"";
        GetTextFaceW(dc, LF_FACESIZE, got);
        SelectObject(dc, old);
        ReleaseDC(nullptr, dc);
        if (!_wcsicmp(got, face)) return font;
        DeleteObject(font);
    }
    NONCLIENTMETRICSW metrics{sizeof(metrics)};
    if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, 96 * scale))
        return CreateFontIndirectW(&metrics.lfMenuFont);
    return nullptr;
}

// What this thread's next menu looks like, at the owner's scale.
thread_local motion::Anim t_curve;
double CurveEase(double t) { return motion::Ease(t_curve, t); }

MenuRules LoadMenuLook(HWND owner) {
    int scale = std::max(1, (int)(GetDpiForWindow(owner) + 48) / 96);
    AcquireSRWLockExclusive(&g_publishedLock);
    HKEY key = nullptr;
    DWORD stamp = 0;
    if (!RegOpenKeyExW(HKEY_CURRENT_USER, kPublishedKey, 0, KEY_READ, &key)) {
        stamp = RegDword(key, L"Stamp", 0);
        RegCloseKey(key);
    }
    if (stamp != g_publishedStamp) {
        ReadPublished(g_published);
        g_publishedStamp = stamp;
        if (g_published.theme != g_themePath) {
            g_themePath = g_published.theme;
            g_themeMenu = OpenStyleMenu(g_themePath);  // (the last one stays open: menus may be drawing with it)
        }
    }
    Published const& p = g_published;
    MenuLook look = p.look;
    look.scale = scale;
    look.theme = p.theme.empty() ? nullptr : g_themeMenu;
    HFONT font = nullptr;
    for (auto const& f : g_fonts)
        if (f.stamp == stamp && f.scale == scale) font = f.font;
    if (!font) {
        if (p.hasFont) {
            LOGFONTW lf = p.font;
            lf.lfHeight = MulDiv(lf.lfHeight, scale, p.scale);
            font = CreateFontIndirectW(&lf);
        } else {
            font = DefaultMenuFont(scale);
        }
        g_fonts.push_back({stamp, scale, font});
    }
    look.font = font;
    MenuRules rules = p.rules;
    MenuMotion motion;
    motion.style = p.animation;
    g_menuFlyoutClick = p.flyoutClick;
    motion.ms = p.ms;
    t_curve = p.curve;
    motion.ease = p.curve.set ? CurveEase : nullptr;
    ReleaseSRWLockExclusive(&g_publishedLock);
    g_menuLook = look;
    g_menuMotion = motion;
    return rules;
}

////////////////////////////////////////////////////////////////////////////////
// The menu itself: drawn here while its owner passes on what it doesn't draw itself, its submenus styled
// as the owner fills them in.

thread_local bool t_busy = false;  // a menu drawn here is open on this thread

LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_MEASUREITEM:
            if (MenuMeasureItem((MEASUREITEMSTRUCT*)lParam)) return TRUE;
            break;
        case WM_DRAWITEM:
            if (MenuDrawItem((DRAWITEMSTRUCT*)lParam)) return TRUE;
            break;
        case WM_INITMENUPOPUP: {
            // A submenu ("Send to", "New") filling in, then styled.
            LRESULT result = DefSubclassProc(hwnd, msg, wParam, lParam);
            StyleMenuItems((HMENU)wParam, true);
            return result;
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

UINT TrackHere(HMENU menu, UINT flags, int x, int y, HWND owner, RECT const* exclude) {
    t_busy = true;
    MenuRules rules = LoadMenuLook(owner);
    MenuRulesUndo undo = ApplyMenuRules(menu, rules, [](UINT) { return std::wstring(); });
    SetWindowSubclass(owner, OwnerProc, 0, 0);
    UINT result = TrackStyledMenu(menu, flags, x, y, owner, true, exclude && !IsRectEmpty(exclude) ? exclude : nullptr);
    RemoveWindowSubclass(owner, OwnerProc, 0);
    UndoMenuRules(undo);
    t_busy = false;
    return result;
}

using TrackPopupMenuEx_t = decltype(&TrackPopupMenuEx);
TrackPopupMenuEx_t TrackPopupMenuEx_Original;
BOOL WINAPI TrackPopupMenuEx_Hook(HMENU menu, UINT flags, int x, int y, HWND owner, LPTPMPARAMS params) {
    void* caller = __builtin_return_address(0);
    if (t_busy || !WantMenu(menu, owner, caller)) return TrackPopupMenuEx_Original(menu, flags, x, y, owner, params);
    return (BOOL)TrackHere(menu, flags, x, y, owner, params ? &params->rcExclude : nullptr);
}

using TrackPopupMenu_t = decltype(&TrackPopupMenu);
TrackPopupMenu_t TrackPopupMenu_Original;
BOOL WINAPI TrackPopupMenu_Hook(HMENU menu, UINT flags, int x, int y, int reserved, HWND owner, const RECT* rect) {
    void* caller = __builtin_return_address(0);
    if (t_busy || !WantMenu(menu, owner, caller)) return TrackPopupMenu_Original(menu, flags, x, y, reserved, owner, rect);
    return (BOOL)TrackHere(menu, flags, x, y, owner, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// Windows' own menus as plain ones (Windows' own menus on): the helper each part of the shell carries asks
// first whether it may draw a menu as an immersive one (ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu),
// and here the answer is no, as in Classic Menus. Hooked in each module as it loads.

using CanApplyOwnerDrawToMenu_t = bool(WINAPI*)(HMENU, HWND);
bool WINAPI CanApplyOwnerDrawToMenu_Hook(HMENU, HWND) {
    return false;
}

SRWLOCK g_hookedLock = SRWLOCK_INIT;
std::vector<HMODULE> g_hooked;  // modules looked at (guarded by g_hookedLock)
std::atomic<bool> g_initDone;

// Windows 11's new File Explorer menu skipped: the folder view asks for its presenter, and gets none, so
// it shows the full menu (m417z's way). The navigation pane decides for itself (CNscTree::ShouldShowMiniMenu).
using ShouldShowMiniMenu_t = bool(WINAPI*)(void*, void*);
ShouldShowMiniMenu_t ShouldShowMiniMenu_Original;
bool WINAPI ShouldShowMiniMenu_Hook(void* self, void* item) {
    if (GetKeyState(VK_SHIFT) < 0) return ShouldShowMiniMenu_Original ? ShouldShowMiniMenu_Original(self, item) : true;
    return false;
}

bool SkipNewMenuHere() { return g_inExplorer ? g_settings.explorer : g_settings.dialogs; }

// The modules each hook is in, by file name.
bool HookModule(HMODULE module) {
    if (!module) return false;
    AcquireSRWLockExclusive(&g_hookedLock);
    bool already = std::find(g_hooked.begin(), g_hooked.end(), module) != g_hooked.end();
    if (!already) g_hooked.push_back(module);
    ReleaseSRWLockExclusive(&g_hookedLock);
    if (already) return false;
    std::wstring name = ProgramName(module);
    bool hooked = false;
    if (SkipNewMenuHere() && !_wcsicmp(name.c_str(), L"ExplorerFrame.dll")) {
        WindhawkUtils::SYMBOL_HOOK hook[] = {
            {{LR"(private: bool __cdecl CNscTree::ShouldShowMiniMenu(struct _TREEITEM *))"},
             &ShouldShowMiniMenu_Original,
             ShouldShowMiniMenu_Hook,
             true},
        };
        hooked |= WindhawkUtils::HookSymbols(module, hook, ARRAYSIZE(hook)) && ShouldShowMiniMenu_Original;
    }
    bool helper = false;
    for (PCWSTR m : kWindowsMenuModules) helper |= !_wcsicmp(m, name.c_str());
    for (PCWSTR m : kExplorerModules) helper |= !_wcsicmp(m, name.c_str());
    if (g_settings.windowsMenus && helper) {
        static CanApplyOwnerDrawToMenu_t originals[64];
        static std::atomic<int> used;
        int slot = used++;
        if (slot < (int)ARRAYSIZE(originals)) {
            WindhawkUtils::SYMBOL_HOOK hook[] = {
                {{
#ifdef _WIN64
                     LR"(bool __cdecl ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *))"
#else
                     LR"(bool __stdcall ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *))"
#endif
                 },
                 &originals[slot],
                 CanApplyOwnerDrawToMenu_Hook,
                 true},
            };
            hooked |= WindhawkUtils::HookSymbols(module, hook, ARRAYSIZE(hook)) && originals[slot];
        }
    }
    return hooked;
}

using LoadLibraryExW_t = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR fileName, HANDLE file, DWORD flags) {
    HMODULE module = LoadLibraryExW_Original(fileName, file, flags);
    if (module && g_initDone && !((ULONG_PTR)module & 3) &&
        !(flags & (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE))) {
        if (HookModule(module)) Wh_ApplyHookOperations();
    }
    return module;
}

// The presenter the folder view asks for (shell32, CDefView::TryGetContextMenuPresenter), and the
// interface it wants (changed in KB5052093 of 24H2).
constexpr GUID kPresenterService = {0xb306c5b1, 0xb4f2, 0x473c, {0xb6, 0xff, 0x70, 0x1b, 0x24, 0x6c, 0xe2, 0xd2}};
constexpr GUID kPresenterIid = {0x706461d1, 0xac5f, 0x4730, {0xbf, 0xe3, 0xca, 0xc6, 0xca, 0xd5, 0xef, 0x5e}};
constexpr GUID kPresenterIid24H2 = {0x37a472f7, 0x63cf, 0x4ccf, {0xa8, 0x8b, 0x52, 0x31, 0xa3, 0xc7, 0xd8, 0xb6}};

using IUnknown_QueryService_t = HRESULT(WINAPI*)(IUnknown*, REFGUID, REFIID, void**);
IUnknown_QueryService_t IUnknown_QueryService_Original;
HRESULT WINAPI IUnknown_QueryService_Hook(IUnknown* object, REFGUID service, REFIID iid, void** out) {
    if (IsEqualGUID(service, kPresenterService) && (IsEqualGUID(iid, kPresenterIid) || IsEqualGUID(iid, kPresenterIid24H2)) &&
        GetKeyState(VK_SHIFT) >= 0) {
        if (out) *out = nullptr;
        return E_FAIL;
    }
    return IUnknown_QueryService_Original(object, service, iid, out);
}

////////////////////////////////////////////////////////////////////////////////

// Programs Windows' own menus aren't in, or where hooking isn't wanted, and the ones set here.
bool ShouldRunHere(std::wstring const& name) {
    for (PCWSTR skip : {L"dwm.exe", L"ShellExperienceHost.exe", L"wlanext.exe", L"svchost.exe",
                        L"SearchIndexer.exe", L"consent.exe", L"LogonUI.exe", L"winlogon.exe", L"csrss.exe",
                        L"TourneDesktop.exe"}) {
        if (!_wcsicmp(name.c_str(), skip)) return false;
    }
    PCWSTR list = Wh_GetStringSetting(L"excludeApps");
    std::wstring items = list ? list : L"";
    Wh_FreeStringSetting(list);
    for (auto const& item : MenuRuleList(items))
        if (!_wcsicmp(item.c_str(), name.c_str())) return false;
    return true;
}

BOOL Wh_ModInit() {
    std::wstring name = ProgramName();
    if (!GetModuleHandleW(L"user32.dll") || !ShouldRunHere(name)) return FALSE;
    g_inExplorer = !_wcsicmp(name.c_str(), L"explorer.exe");
    // Windhawk's own process (where Tourne'Icons shows its Open dialog): its dialogs only.
    bool inWindhawk = !_wcsicmp(name.c_str(), L"windhawk.exe");
    g_settings.explorer = Wh_GetIntSetting(L"explorer") != 0;
    g_settings.dialogs = Wh_GetIntSetting(L"fileDialogs") != 0;
    g_settings.windowsMenus = Wh_GetIntSetting(L"windowsMenus") != 0 && !inWindhawk;
    // Nothing to do in this program for now: no hooks, but still loaded, so a switch turned on later
    // reaches programs already running (one whose init said no would need restarting).
    if (!(g_settings.explorer && g_inExplorer) && !g_settings.dialogs && !g_settings.windowsMenus) return TRUE;

    WindhawkUtils::SetFunctionHook(TrackPopupMenuEx, TrackPopupMenuEx_Hook, &TrackPopupMenuEx_Original);
    WindhawkUtils::SetFunctionHook(TrackPopupMenu, TrackPopupMenu_Hook, &TrackPopupMenu_Original);
    if (SkipNewMenuHere()) {
        HMODULE shcore = GetModuleHandleW(L"shcore.dll");
        if (auto query = shcore ? (IUnknown_QueryService_t)GetProcAddress(shcore, "IUnknown_QueryService") : nullptr)
            WindhawkUtils::SetFunctionHook(query, IUnknown_QueryService_Hook, &IUnknown_QueryService_Original);
    }
    // The modules already loaded, and the program itself; the rest as they load.
    HookModule(GetModuleHandleW(nullptr));
    for (PCWSTR m : kWindowsMenuModules) HookModule(GetModuleHandleW(m));
    for (PCWSTR m : kExplorerModules) HookModule(GetModuleHandleW(m));
    HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
    if (auto loadLibraryExW = kernelBase ? (LoadLibraryExW_t)GetProcAddress(kernelBase, "LoadLibraryExW") : nullptr)
        WindhawkUtils::SetFunctionHook(loadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    g_initDone = true;
    return TRUE;
}

void Wh_ModUninit() {}

// A switch changed: the hooks are put in again as the settings now say.
BOOL Wh_ModSettingsChanged(BOOL* reload) {
    *reload = TRUE;
    return TRUE;
}
