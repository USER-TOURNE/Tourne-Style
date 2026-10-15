// ==WindhawkMod==
// @id              local@tourne-dark-magic
// @name            Tourne Dark Magic
// @description     Dark mode for Windows' classic programs and dialogs, as StartAllBack's Dark Magic gives them
// @version         1.0.0
// @author          Tourne
// @include         *
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- when: auto
  $name: When
  $description: >-
    With Windows' apps in dark mode (Settings, Personalization, Colors), or
    always.
  $options:
  - auto: When apps use dark mode
  - always: Always
- programs: shell
  $name: Programs
  $description: >-
    Windows' own classic programs (Explorer's windows and dialogs, Task
    Manager, the volume mixer, Control Panel applets, MMC consoles, the
    Registry Editor and the like), as StartAllBack's Dark Magic covers them,
    or every program.
  $options:
  - shell: Windows' classic programs
  - all: Every program
- controls: true
  $name: Dark controls
  $description: >-
    Buttons, check boxes, text boxes, drop-downs, lists, trees, headers,
    scroll bars, tabs, progress bars, sliders, status bars and toolbars take
    the visual style's dark variants.
- tooltips: true
  $name: Dark tooltips
- titleBars: true
  $name: Dark title bars
- menuBars: true
  $name: Dark menu bars
  $description: A window's menu bar (File, Edit...), when the visual style draws it light.
- dialogs: true
  $name: Dark dialogs
  $description: >-
    Dialog backgrounds and text, and list and tree backgrounds, when the
    visual style leaves them light. Colours come from Theme Gaps' setting.
- menuMode: forceDark
  $name: Menus
  $description: The dark mode Windows' own menus use in these programs.
  $options:
  - forceDark: Always dark
  - allowDark: Dark when the program asks
  - leave: Leave as the program has it
- excludeApps: ''
  $name: Leave these programs alone
  $description: Program file names, separated by commas, like game.exe.
*/
// ==/WindhawkModSettings==

// StartAllBack's Dark Magic (DarkMagicX64.dll) is loaded into Windows' shell
// programs and turns their classic Win32 parts dark: controls get the visual
// style's DarkMode_ classes, title bars and tooltips go dark, and what no theme
// draws is painted in dark colours. This part does the same from inside each
// program: every window made in the program is seen as it's created (an
// in-process WinEvent hook, which also sees the controls Windows makes for
// dialogs), and given its dark look there, on its own thread.

#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <dwmapi.h>
#include <commctrl.h>

#include <string>
#include <string_view>
#include <cstdint>
#include <vector>
#include <mutex>
#include <algorithm>
#include <atomic>

// @inline palettes.cpp
// @inline theme-gaps-palette.cpp

////////////////////////////////////////////////////////////////////////////////
// Settings

struct DarkSettings {
    bool always = false;
    bool allPrograms = false;
    bool controls = true;
    bool tooltips = true;
    bool titleBars = true;
    bool menuBars = true;
    bool dialogs = true;
    int menuMode = 2;  // SetPreferredAppMode: 0 leave, 1 allow dark, 2 force dark
};
DarkSettings g_dm;

std::wstring TextSetting(PCWSTR name) {
    PCWSTR value = Wh_GetStringSetting(name);
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

void LoadDarkSettings() {
    g_dm.always = TextSetting(L"when") == L"always";
    g_dm.allPrograms = TextSetting(L"programs") == L"all";
    g_dm.controls = Wh_GetIntSetting(L"controls") != 0;
    g_dm.tooltips = Wh_GetIntSetting(L"tooltips") != 0;
    g_dm.titleBars = Wh_GetIntSetting(L"titleBars") != 0;
    g_dm.menuBars = Wh_GetIntSetting(L"menuBars") != 0;
    g_dm.dialogs = Wh_GetIntSetting(L"dialogs") != 0;
    std::wstring mode = TextSetting(L"menuMode");
    g_dm.menuMode = mode == L"leave" ? 0 : mode == L"allowDark" ? 1 : 2;
    LoadGapSource();
}

// Windows' own classic programs, as Dark Magic's list has them.
constexpr PCWSTR kShellPrograms[] = {
    L"explorer.exe", L"taskmgr.exe", L"sndvol.exe", L"rundll32.exe", L"control.exe", L"mmc.exe", L"dllhost.exe",
    L"regedit.exe", L"msconfig.exe", L"systempropertiesadvanced.exe", L"systempropertiescomputername.exe",
    L"systempropertiesperformance.exe", L"systempropertiesprotection.exe", L"systempropertiesremote.exe",
    L"systempropertieshardware.exe", L"systempropertiesdataexecutionprevention.exe", L"charmap.exe", L"cleanmgr.exe",
    L"dfrgui.exe", L"msinfo32.exe", L"mstsc.exe", L"odbcad32.exe", L"openwith.exe", L"perfmon.exe", L"resmon.exe",
    L"winver.exe", L"colorcpl.exe", L"cttune.exe", L"iscsicpl.exe", L"fsquirt.exe", L"devicepairingwizard.exe",
    L"wordpad.exe", L"write.exe", L"msdt.exe", L"recdisc.exe", L"shrpubw.exe", L"sdclt.exe",
    L"systemsettingsadminflows.exe", L"useraccountcontrolsettings.exe", L"rekeywiz.exe", L"credwiz.exe",
    L"tabcal.exe", L"printui.exe", L"fontview.exe", L"eudcedit.exe", L"dxdiag.exe", L"msra.exe", L"wf.exe",
    L"eventvwr.exe", L"compmgmtlauncher.exe", L"netplwiz.exe", L"lpksetup.exe", L"bthudtask.exe",
    L"startallbackcfg.exe", L"windhawk.exe",
};

std::wstring ProcessName() {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) return {};
    PCWSTR name = wcsrchr(path, L'\\');
    return name ? name + 1 : path;
}

bool InList(std::wstring const& items, std::wstring const& name) {
    size_t start = 0;
    while (start <= items.size()) {
        size_t comma = items.find(L',', start);
        std::wstring item = items.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty() && !_wcsicmp(item.c_str(), name.c_str())) return true;
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return false;
}

bool ShouldLoadInThisProcess() {
    if (!GetModuleHandleW(L"user32.dll")) return false;
    std::wstring name = ProcessName();
    if (name.empty() || InList(TextSetting(L"excludeApps"), name)) return false;
    if (TextSetting(L"programs") == L"all") return true;
    for (PCWSTR program : kShellPrograms)
        if (!_wcsicmp(program, name.c_str())) return true;
    return false;
}

// Windows' apps setting: AppsUseLightTheme 0 means dark.
bool AppsUseDark() {
    DWORD light = 1, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light == 0;
}

std::atomic<bool> g_dark{false};

bool DarkNow() {
    return g_dm.always || AppsUseDark();
}

////////////////////////////////////////////////////////////////////////////////
// Colours

double DmLuma(COLORREF c) {
    return (0.299 * GetRValue(c) + 0.587 * GetGValue(c) + 0.114 * GetBValue(c)) / 255.0;
}

// The visual style draws classic parts light (Windows' own aero, say): only
// then are colours painted over; a dark custom style keeps its own.
bool StyleIsLight() {
    return DmLuma(GetSysColor(COLOR_BTNFACE)) > 0.5;
}

struct DarkColors {
    COLORREF ground, raised, text, hot, line;
};

// Theme Gaps' colours when they're dark, else Windows' own dark ones.
DarkColors CurrentDarkColors() {
    GapPalette p = CurrentGapPalette();
    DarkColors c;
    if (DmLuma(p.ground) < 0.5) {
        c.ground = p.ground;
        c.raised = p.raised;
        c.text = p.text;
    } else {
        c.ground = RGB(0x20, 0x20, 0x20);
        c.raised = RGB(0x2B, 0x2B, 0x2B);
        c.text = RGB(0xFF, 0xFF, 0xFF);
    }
    c.hot = GapMix(c.ground, c.text, 0.15);
    c.line = GapMix(c.ground, c.text, 0.25);
    return c;
}

// Solid brushes, kept for the life of the program (a handful of colours).
HBRUSH DarkBrush(COLORREF color) {
    static std::mutex lock;
    static std::vector<std::pair<COLORREF, HBRUSH>> made;
    std::lock_guard guard(lock);
    for (auto const& [c, b] : made)
        if (c == color) return b;
    HBRUSH brush = CreateSolidBrush(color);
    made.push_back({color, brush});
    return brush;
}

////////////////////////////////////////////////////////////////////////////////
// uxtheme's unnamed dark mode calls

using SetPreferredAppMode_t = int(WINAPI*)(int);
using AllowDarkModeForWindow_t = BOOL(WINAPI*)(HWND, BOOL);
using FlushMenuThemes_t = void(WINAPI*)();
using RefreshImmersiveColorPolicyState_t = void(WINAPI*)();
SetPreferredAppMode_t SetPreferredAppMode_;
AllowDarkModeForWindow_t AllowDarkModeForWindow_;
FlushMenuThemes_t FlushMenuThemes_;
RefreshImmersiveColorPolicyState_t RefreshImmersiveColorPolicyState_;

void LoadDarkCalls() {
    HMODULE ux = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!ux) return;
    SetPreferredAppMode_ = (SetPreferredAppMode_t)GetProcAddress(ux, MAKEINTRESOURCEA(135));
    AllowDarkModeForWindow_ = (AllowDarkModeForWindow_t)GetProcAddress(ux, MAKEINTRESOURCEA(133));
    FlushMenuThemes_ = (FlushMenuThemes_t)GetProcAddress(ux, MAKEINTRESOURCEA(136));
    RefreshImmersiveColorPolicyState_ = (RefreshImmersiveColorPolicyState_t)GetProcAddress(ux, MAKEINTRESOURCEA(104));
}

void ApplyAppMode() {
    if (!SetPreferredAppMode_ || !g_dm.menuMode) return;
    SetPreferredAppMode_(g_dark ? g_dm.menuMode : 0);
    if (RefreshImmersiveColorPolicyState_) RefreshImmersiveColorPolicyState_();
    if (FlushMenuThemes_) FlushMenuThemes_();
}

////////////////////////////////////////////////////////////////////////////////
// SetWindowTheme: the classic app names asked for by programs (Explorer's
// "Explorer" on its lists, dialogs' "CFD") are given their dark variants, as
// Windows does for itself in dark mode.

using SetWindowTheme_t = HRESULT(WINAPI*)(HWND, LPCWSTR, LPCWSTR);
SetWindowTheme_t SetWindowTheme_Original;

PCWSTR DarkAppName(LPCWSTR app) {
    if (!app) return nullptr;
    if (!_wcsicmp(app, L"Explorer")) return L"DarkMode_Explorer";
    if (!_wcsicmp(app, L"CFD")) return L"DarkMode_CFD";
    if (!_wcsicmp(app, L"ItemsView")) return L"DarkMode_ItemsView";
    return nullptr;
}

HRESULT WINAPI SetWindowTheme_Hook(HWND wnd, LPCWSTR app, LPCWSTR idList) {
    if (g_dark && g_dm.controls)
        if (PCWSTR dark = DarkAppName(app)) app = dark;
    return SetWindowTheme_Original(wnd, app, idList);
}

////////////////////////////////////////////////////////////////////////////////
// Painting what the style leaves light: dialog grounds and text, and the menu
// bar (Windows draws it through its undocumented UAH messages).

constexpr UINT WM_UAHDRAWMENU = 0x0091, WM_UAHDRAWMENUITEM = 0x0092;

struct UAHMENU {
    HMENU hmenu;
    HDC hdc;
    DWORD dwFlags;
};
struct UAHMENUITEMMETRICS {
    DWORD rgsize[8];
};
struct UAHMENUPOPUPMETRICS {
    DWORD rgcx[4];
    DWORD fUpdateMaxWidths;
};
struct UAHMENUITEM {
    int iPosition;
    UAHMENUITEMMETRICS umim;
    UAHMENUPOPUPMETRICS umpm;
};
struct UAHDRAWMENUITEM {
    DRAWITEMSTRUCT dis;
    UAHMENU um;
    UAHMENUITEM umi;
};

UINT g_unsubclassMessage;  // sent to each subclassed window when this part unloads
std::mutex g_subclassedLock;
std::vector<HWND> g_subclassed;

void Unsubclass(HWND wnd);

// The menu bar's rectangle in window coordinates.
bool MenuBarRect(HWND wnd, RECT* bar) {
    MENUBARINFO info{sizeof(info)};
    if (!GetMenu(wnd) || !GetMenuBarInfo(wnd, OBJID_MENU, 0, &info)) return false;
    RECT window;
    GetWindowRect(wnd, &window);
    *bar = info.rcBar;
    OffsetRect(bar, -window.left, -window.top);
    return true;
}

// The light line Windows leaves between the menu bar and the window's client.
void CoverMenuBarLine(HWND wnd) {
    RECT bar;
    if (!MenuBarRect(wnd, &bar)) return;
    RECT client, window;
    GetClientRect(wnd, &client);
    MapWindowPoints(wnd, nullptr, (POINT*)&client, 2);
    GetWindowRect(wnd, &window);
    OffsetRect(&client, -window.left, -window.top);
    RECT line{client.left, client.top - 1, client.right, client.top};
    HDC dc = GetWindowDC(wnd);
    FillRect(dc, &line, DarkBrush(CurrentDarkColors().ground));
    ReleaseDC(wnd, dc);
}

void DrawMenuBarItem(HWND, UAHDRAWMENUITEM* item) {
    DarkColors c = CurrentDarkColors();
    UINT state = item->dis.itemState;
    WCHAR text[256] = L"";
    MENUITEMINFOW info{sizeof(info)};
    info.fMask = MIIM_STRING;
    info.dwTypeData = text;
    info.cch = ARRAYSIZE(text) - 1;
    GetMenuItemInfoW(item->um.hmenu, item->umi.iPosition, TRUE, &info);
    COLORREF ground = (state & (ODS_SELECTED)) ? GapMix(c.ground, c.text, 0.22)
                      : (state & ODS_HOTLIGHT) ? c.hot
                                               : c.ground;
    FillRect(item->um.hdc, &item->dis.rcItem, DarkBrush(ground));
    bool disabled = state & (ODS_GRAYED | ODS_DISABLED | ODS_INACTIVE);
    SetBkMode(item->um.hdc, TRANSPARENT);
    SetTextColor(item->um.hdc, disabled ? GapMix(c.ground, c.text, 0.5) : c.text);
    UINT flags = DT_CENTER | DT_SINGLELINE | DT_VCENTER | ((state & ODS_NOACCEL) ? DT_HIDEPREFIX : 0);
    DrawTextW(item->um.hdc, text, -1, &item->dis.rcItem, flags);
}

// Is the brush one of the system's own (what a dialog uses when the program
// gave none)? Only those are painted over.
bool SystemBrush(HBRUSH brush) {
    for (int index : {COLOR_BTNFACE, COLOR_WINDOW, COLOR_3DFACE, COLOR_MENU})
        if (brush == GetSysColorBrush(index)) return true;
    return !brush;
}

LRESULT CALLBACK DarkSubclassProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == g_unsubclassMessage && g_unsubclassMessage) {
        RemoveWindowSubclass(wnd, DarkSubclassProc, 0);
        return 0;
    }
    bool paint = g_dark && StyleIsLight();
    switch (msg) {
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            LRESULT result = DefSubclassProc(wnd, msg, wParam, lParam);
            if (!paint || !g_dm.dialogs || !SystemBrush((HBRUSH)result)) return result;
            DarkColors c = CurrentDarkColors();
            bool field = msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX;
            COLORREF ground = field ? c.raised : c.ground;
            SetTextColor((HDC)wParam, c.text);
            SetBkColor((HDC)wParam, ground);
            return (LRESULT)DarkBrush(ground);
        }
        case WM_UAHDRAWMENU: {
            if (!paint || !g_dm.menuBars) break;
            auto* menu = (UAHMENU*)lParam;
            RECT bar;
            if (MenuBarRect(wnd, &bar)) {
                bar.top -= 1;
                FillRect(menu->hdc, &bar, DarkBrush(CurrentDarkColors().ground));
            }
            return TRUE;
        }
        case WM_UAHDRAWMENUITEM:
            if (!paint || !g_dm.menuBars) break;
            DrawMenuBarItem(wnd, (UAHDRAWMENUITEM*)lParam);
            return TRUE;
        case WM_NCPAINT:
        case WM_NCACTIVATE: {
            LRESULT result = DefSubclassProc(wnd, msg, wParam, lParam);
            if (paint && g_dm.menuBars) CoverMenuBarLine(wnd);
            return result;
        }
        case WM_SETTINGCHANGE:
            // Windows switched between dark and light: follow it.
            if (lParam && !wcscmp((PCWSTR)lParam, L"ImmersiveColorSet")) {
                bool dark = DarkNow();
                if (dark != g_dark) {
                    g_dark = dark;
                    ApplyAppMode();
                    RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
                }
            }
            break;
        case WM_NCDESTROY:
            Unsubclass(wnd);
            break;
    }
    return DefSubclassProc(wnd, msg, wParam, lParam);
}

void Subclass(HWND wnd) {
    if (!SetWindowSubclass(wnd, DarkSubclassProc, 0, 0)) return;
    std::lock_guard guard(g_subclassedLock);
    g_subclassed.push_back(wnd);
}

void Unsubclass(HWND wnd) {
    RemoveWindowSubclass(wnd, DarkSubclassProc, 0);
    std::lock_guard guard(g_subclassedLock);
    std::erase(g_subclassed, wnd);
}

////////////////////////////////////////////////////////////////////////////////
// Each window, as it's made

// The dark app name for a control class, or null for none.
PCWSTR ControlTheme(std::wstring_view cls) {
    struct Entry {
        PCWSTR cls, app;
    };
    static constexpr Entry kEntries[] = {
        {L"Button", L"DarkMode_Explorer"},          {L"ScrollBar", L"DarkMode_Explorer"},
        {L"Edit", L"DarkMode_CFD"},                 {L"ComboBox", L"DarkMode_CFD"},
        {L"ComboBoxEx32", L"DarkMode_CFD"},         {L"ComboLBox", L"DarkMode_Explorer"},
        {L"ListBox", L"DarkMode_Explorer"},         {L"SysListView32", L"DarkMode_ItemsView"},
        {L"SysTreeView32", L"DarkMode_Explorer"},   {L"SysHeader32", L"DarkMode_ItemsView"},
        {L"msctls_progress32", L"DarkMode_Explorer"}, {L"msctls_trackbar32", L"DarkMode_Explorer"},
        {L"SysTabControl32", L"DarkMode_Explorer"}, {L"msctls_statusbar32", L"DarkMode_Explorer"},
        {L"ToolbarWindow32", L"DarkMode_Explorer"}, {L"ReBarWindow32", L"DarkMode_Explorer"},
        {L"SysLink", L"DarkMode_Explorer"},         {L"msctls_updown32", L"DarkMode_Explorer"},
    };
    for (auto const& e : kEntries)
        if (cls.size() == wcslen(e.cls) && !_wcsnicmp(cls.data(), e.cls, cls.size())) return e.app;
    return nullptr;
}

// Lists and trees keep Windows' white ground under a light style: theirs is
// set to the dark colours while it's still the default.
void DarkenListColors(HWND wnd, std::wstring_view cls) {
    if (!g_dm.dialogs || !StyleIsLight()) return;
    DarkColors c = CurrentDarkColors();
    COLORREF window = GetSysColor(COLOR_WINDOW);
    if (cls == L"SysListView32") {
        if (ListView_GetBkColor(wnd) != window) return;
        ListView_SetBkColor(wnd, c.raised);
        ListView_SetTextBkColor(wnd, c.raised);
        ListView_SetTextColor(wnd, c.text);
    } else if (cls == L"SysTreeView32") {
        COLORREF bk = TreeView_GetBkColor(wnd);
        if (bk != (COLORREF)-1 && bk != window) return;
        TreeView_SetBkColor(wnd, c.raised);
        TreeView_SetTextColor(wnd, c.text);
    }
}

void DarkenWindow(HWND wnd, bool ownThread) {
    if (!g_dark || !IsWindow(wnd)) return;
    WCHAR name[64] = L"";
    if (!GetClassNameW(wnd, name, ARRAYSIZE(name))) return;
    std::wstring_view cls = name;
    if (cls == L"ConsoleWindowClass" || cls == L"IME" || cls == L"MSCTFIME UI") return;
    LONG_PTR style = GetWindowLongPtrW(wnd, GWL_STYLE);
    bool topLevel = !(style & WS_CHILD);
    if (AllowDarkModeForWindow_) AllowDarkModeForWindow_(wnd, TRUE);

    if (cls == L"tooltips_class32") {
        if (g_dm.tooltips) SetWindowTheme_Original(wnd, L"DarkMode_Explorer", nullptr);
        return;
    }
    if (g_dm.controls) {
        if (PCWSTR app = ControlTheme(cls)) {
            SetWindowTheme_Original(wnd, app, nullptr);
            DarkenListColors(wnd, cls);
            return;
        }
    }
    if (topLevel && g_dm.titleBars && (style & WS_CAPTION) == WS_CAPTION) {
        BOOL on = TRUE;
        if (FAILED(DwmSetWindowAttribute(wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &on, sizeof(on))))
            DwmSetWindowAttribute(wnd, 19, &on, sizeof(on));  // before Windows 10 20H1
    }
    // Dialogs (and property sheet pages, which are child dialogs) for their
    // grounds, and captioned windows for their menu bars. Only on the
    // window's own thread, where subclassing is allowed.
    if (ownThread && (cls == L"#32770" || (topLevel && (style & WS_CAPTION) == WS_CAPTION))) Subclass(wnd);
}

HWINEVENTHOOK g_createHook;

void CALLBACK WindowCreated(HWINEVENTHOOK, DWORD, HWND wnd, LONG object, LONG, DWORD, DWORD) {
    if (object != OBJID_WINDOW || !wnd) return;
    DarkenWindow(wnd, GetWindowThreadProcessId(wnd, nullptr) == GetCurrentThreadId());
}

// Windows already open when the part starts, from a worker thread: themes
// and colours, but no subclassing (only a window's own thread may).
BOOL CALLBACK DarkenOpenChild(HWND child, LPARAM) {
    DarkenWindow(child, false);
    return TRUE;
}

BOOL CALLBACK DarkenOpenWindow(HWND wnd, LPARAM) {
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    if (pid != GetCurrentProcessId() || IsHungAppWindow(wnd)) return TRUE;
    DarkenWindow(wnd, false);
    EnumChildWindows(wnd, DarkenOpenChild, 0);
    return TRUE;
}

DWORD WINAPI DarkenOpenWindows(void*) {
    EnumWindows(DarkenOpenWindow, 0);
    return 0;
}

HMODULE ThisModule() {
    HMODULE module = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&ThisModule, &module);
    return module;
}

BOOL Wh_ModInit() {
    LoadDarkSettings();
    LoadDarkCalls();
    g_dark = DarkNow();
    g_unsubclassMessage = RegisterWindowMessageW(L"TourneDarkMagic_Unsubclass");
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    if (!ux) ux = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (auto setWindowTheme = ux ? GetProcAddress(ux, "SetWindowTheme") : nullptr)
        Wh_SetFunctionHook((void*)setWindowTheme, (void*)SetWindowTheme_Hook, (void**)&SetWindowTheme_Original);
    if (!SetWindowTheme_Original) SetWindowTheme_Original = SetWindowTheme;
    ApplyAppMode();
    // In-process and in-context: called on the thread that made the window,
    // for this program's windows only.
    g_createHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, ThisModule(), WindowCreated,
                                   GetCurrentProcessId(), 0, WINEVENT_INCONTEXT);
    if (HANDLE thread = CreateThread(nullptr, 0, DarkenOpenWindows, nullptr, 0, nullptr)) CloseHandle(thread);
    return TRUE;
}

void Wh_ModUninit() {
    if (g_createHook) UnhookWinEvent(g_createHook);
    g_createHook = nullptr;
    std::vector<HWND> windows;
    {
        std::lock_guard guard(g_subclassedLock);
        windows = g_subclassed;
    }
    for (HWND wnd : windows) {
        DWORD_PTR result;
        SendMessageTimeoutW(wnd, g_unsubclassMessage, 0, 0, SMTO_ABORTIFHUNG, 1000, &result);
    }
}

void Wh_ModSettingsChanged() {
    LoadDarkSettings();
    g_dark = DarkNow();
    ApplyAppMode();
}
