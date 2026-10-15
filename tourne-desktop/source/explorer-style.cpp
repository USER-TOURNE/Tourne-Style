// ==WindhawkMod==
// @id              local@tourne-explorer-style
// @name            Tourne Explorer Style
// @description     Explorer's windows in a full visual style of your choosing, without patching Windows
// @version         1.0.0
// @author          Tourne
// @include         explorer.exe
// @compilerOptions -luxtheme -ldwmapi -lcomctl32 -lole32 -lshlwapi
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- style: none
  $name: Visual style
  $description: >-
    A full visual style (.msstyles) for Explorer's windows (folders, the
    desktop, its dialogs), without patching Windows: one from
    %LOCALAPPDATA%\Tourne\Styles or StartAllBack's Styles folder, or a full
    path. Parts the style doesn't have stay Windows' own; none leaves
    Explorer as Windows draws it.
  #! $dynamicSelect: true
- commandBar: windows11
  $name: Command bar
  $description: >-
    What sits above Explorer's folders, as StartAllBack's Explorer style
    does: Windows 11's command bar, Windows 10's ribbon, or Windows 7's
    command bar. The ribbon and Windows 7's bar come with the classic address
    bar and without Windows 11's header and tabs. New windows get it; open
    ones keep theirs until they're reopened (the header may need Explorer
    restarted).
  $options:
  - windows11: Windows 11 command bar
  - ribbon: Windows 10 ribbon
  - windows7: Windows 7 command bar
- detailsPane: right
  $name: Details pane
  $description: >-
    Where the details pane sits: Windows 11's, on the right, or Windows 7's,
    along the bottom of the window (the preview pane stays on the right).
    New windows get it.
  $options:
  - right: On the right
  - bottom: Along the bottom
- driveGrouping: false
  $name: Group drives by type
  $description: >-
    This PC's drives in Windows 7's groups (Hard Disk Drives, Devices with
    Removable Storage, Other) instead of one "Devices and drives" group.
    Folders opened after the change get it.
- styleBars: true
  $name: Progress bars from the visual style
  $description: >-
    Explorer's dark-mode progress bars (like the copy dialog's) from the
    visual style's regular bars when the style left its dark ones as
    Windows' own; a style that draws its own dark bars keeps them. Only
    while a custom visual style is in use. New windows get it.
- darkDialogs: followWindows
  $name: Dark dialogs
  $description: >-
    Explorer's classic dialogs (Properties, Run, Folder Options, file
    conflicts and the like) dark to match, as StartAllBack's restyled
    controls do: title bar, background, text, buttons, lists and text boxes.
    Colours come from the visual style when it's a custom one, else Windows'
    dark ones. Dialogs already open keep their look until they're opened
    again.
  $options:
  - followWindows: When Windows' apps are dark
  - always: Always
  - off: Off
- navHideLines: false
  $name: Hide the navigation pane's separator lines
  $description: >-
    The lines Windows draws between the navigation pane's groups (Home and
    Gallery, the pinned folders, This PC). Painted over with the pane's own
    background after it draws, only where a whole row across a group's row is
    one flat colour. Idea from Languster's Explorer TreeLine Killer.
- navCompact: false
  $name: Close the navigation pane's gaps
  $description: >-
    The groups' rows are twice the height of the others; this makes them
    single height, so the pane is compact. Given back when switched off.
*/
// ==/WindhawkModSettings==

// Every window opens its theme data through uxtheme's OpenThemeData family;
// here the classes are opened from the chosen style's file instead (loaded
// the way the taskbar loads its theme, see theme-palette.cpp), falling back to
// Windows' own style for any class the file lacks.

#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <windhawk_utils.h>
#include <shobjidl.h>
#include <shlwapi.h>

#include <atomic>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// @inline palettes.cpp
// @inline theme-palette.cpp
// @inline theme-gaps-palette.cpp

ThemeFileData g_styleFile;
bool g_styleLoaded = false;
SRWLOCK g_styleLock = SRWLOCK_INIT;
// Files from earlier settings: their sections stay mapped, as windows may
// still hold theme handles opened from them.
std::vector<ThemeFileData> g_oldStyleFiles;
thread_local bool g_inOpen = false;

using OpenThemeData_t = HTHEME(WINAPI*)(HWND, LPCWSTR);
using OpenThemeDataEx_t = HTHEME(WINAPI*)(HWND, LPCWSTR, DWORD);
using OpenThemeDataForDpi_t = HTHEME(WINAPI*)(HWND, LPCWSTR, UINT);
OpenThemeData_t OpenThemeData_Original;
OpenThemeDataEx_t OpenThemeDataEx_Original;
OpenThemeDataForDpi_t OpenThemeDataForDpi_Original;

// The class from the chosen style, or null for Windows' own.
HTHEME OpenFromStyle(HWND wnd, LPCWSTR classList, DWORD flags) {
    if (g_inOpen || !classList) return nullptr;
    HTHEME theme = nullptr;
    AcquireSRWLockShared(&g_styleLock);
    if (g_styleLoaded) {
        g_inOpen = true;
        if (auto open = ThemeFileOpener()) theme = open(&g_styleFile, wnd, classList, flags);
        g_inOpen = false;
    }
    ReleaseSRWLockShared(&g_styleLock);
    return theme;
}

// Progress bars (This PC's drive bars, copy dialogs) as the visual style draws
// them, as StartAllBack's restyled controls have them: dark mode asks for
// DarkMode_...::Progress, which custom styles tend to leave as Windows' solid
// bars, so the style's own Progress is opened instead. Only with a custom
// style: Windows' own light Progress would be out of place in dark mode.
// A style that does draw its own dark bars keeps them: its dark class is
// compared with Windows' own style's, and only a copy left as Windows had it
// is passed over.
std::atomic<bool> g_styleBars{true};
thread_local bool g_inBarCheck = false;

// A progress bar (track and fill) drawn into a small bitmap.
std::vector<uint32_t> BarPixels(HTHEME theme) {
    constexpr int kWidth = 48, kHeight = 12;
    std::vector<uint32_t> pixels;
    if (!theme) return pixels;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), kWidth, -kHeight, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        memset(bits, 0, kWidth * kHeight * 4);
        RECT track{0, 0, kWidth, kHeight}, fill{1, 1, kWidth / 2, kHeight - 1};
        DrawThemeBackground(theme, dc, PP_TRANSPARENTBAR, PBBS_NORMAL, &track, nullptr);
        DrawThemeBackground(theme, dc, PP_FILL, PBFS_NORMAL, &fill, nullptr);
        GdiFlush();
        pixels.assign((uint32_t*)bits, (uint32_t*)bits + kWidth * kHeight);
        SelectObject(dc, old);
        DeleteObject(bitmap);
    }
    DeleteDC(dc);
    return pixels;
}

// Whether the style in use left this dark class as Windows' own style has it.
bool DarkBarLeftAsWindows(LPCWSTR darkClass) {
    static SRWLOCK lock = SRWLOCK_INIT;
    static std::wstring checkedStyle;
    static std::vector<std::pair<std::wstring, bool>> checked;
    static ThemeFileData windowsStyle;
    static bool windowsLoaded = false;
    std::wstring style = CurrentThemeFile();
    AcquireSRWLockExclusive(&lock);
    if (style != checkedStyle) {
        checkedStyle = style;
        checked.clear();
    }
    for (auto const& [name, left] : checked) {
        if (!_wcsicmp(name.c_str(), darkClass)) {
            ReleaseSRWLockExclusive(&lock);
            return left;
        }
    }
    if (!windowsLoaded) {
        WCHAR aero[MAX_PATH];
        ExpandEnvironmentStringsW(L"%WINDIR%\\Resources\\Themes\\aero\\aero.msstyles", aero, MAX_PATH);
        windowsLoaded = LoadThemeFile(aero, &windowsStyle);  // stays mapped for later checks
    }
    bool left = false;
    auto open = ThemeFileOpener();
    if (windowsLoaded && open) {
        HTHEME ours = OpenThemeData_Original(nullptr, darkClass);
        HTHEME windows = open(&windowsStyle, nullptr, darkClass, 0);
        std::vector<uint32_t> a = BarPixels(ours), b = BarPixels(windows);
        left = !a.empty() && a == b;
        if (ours) CloseThemeData(ours);
        if (windows) CloseThemeData(windows);
    }
    checked.emplace_back(darkClass, left);
    ReleaseSRWLockExclusive(&lock);
    Wh_Log(L"%ls: %ls", darkClass, left ? L"left as Windows', using the style's Progress" : L"the style's own");
    return left;
}

LPCWSTR StyleBarClass(LPCWSTR classList) {
    if (!g_styleBars || g_inBarCheck || !classList || _wcsnicmp(classList, L"DarkMode", 8) != 0) return classList;
    size_t length = wcslen(classList);
    constexpr size_t kSuffix = 10;  // "::Progress"
    if (length < kSuffix || _wcsicmp(classList + length - kSuffix, L"::Progress") != 0) return classList;
    if (!CustomStyleInUse()) return classList;
    // Opening the classes to compare comes back through these hooks.
    g_inBarCheck = true;
    bool left = DarkBarLeftAsWindows(classList);
    g_inBarCheck = false;
    if (!left) return classList;
    return StrStrIW(classList, L"Indeterminate") ? L"Indeterminate::Progress" : L"Progress";
}

HTHEME WINAPI OpenThemeData_Hook(HWND wnd, LPCWSTR classList) {
    classList = StyleBarClass(classList);
    if (HTHEME theme = OpenFromStyle(wnd, classList, 0)) return theme;
    return OpenThemeData_Original(wnd, classList);
}

HTHEME WINAPI OpenThemeDataEx_Hook(HWND wnd, LPCWSTR classList, DWORD flags) {
    classList = StyleBarClass(classList);
    if (HTHEME theme = OpenFromStyle(wnd, classList, flags)) return theme;
    return OpenThemeDataEx_Original(wnd, classList, flags);
}

HTHEME WINAPI OpenThemeDataForDpi_Hook(HWND wnd, LPCWSTR classList, UINT dpi) {
    classList = StyleBarClass(classList);
    if (HTHEME theme = OpenFromStyle(wnd, classList, 0)) return theme;
    return OpenThemeDataForDpi_Original(wnd, classList, dpi);
}

std::wstring ChosenStyleFile() {
    PCWSTR value = Wh_GetStringSetting(L"style");
    std::wstring name = value ? value : L"";
    Wh_FreeStringSetting(value);
    if (name.empty() || name == L"none") return {};
    return ThemeStyleFile(name);
}

// The styles to choose from, for the setting's list.
void PublishStyleChoices() {
    auto option = [](std::wstring const& value, std::wstring const& label) {
        // The setting's full name in the suite: the build doesn't prefix this one.
        std::wstring key = L"::wh_select_option::explorerStyle.style::" + value;
        Wh_SetStringValue(key.c_str(), label.c_str());
    };
    option(L"none", L"None: Windows' own");
    for (PCWSTR folder : {L"%ProgramFiles%\\StartAllBack\\Styles\\", L"%LOCALAPPDATA%\\Tourne\\Styles\\"}) {
        WCHAR expanded[MAX_PATH * 2];
        if (!ExpandEnvironmentStringsW(folder, expanded, ARRAYSIZE(expanded))) continue;
        bool tourne = wcsstr(folder, L"Tourne") != nullptr;
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((std::wstring(expanded) + L"*.msstyles").c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) continue;
        do {
            std::wstring name = fd.cFileName;
            std::wstring stem = name.substr(0, name.rfind(L'.'));
            if (tourne) option(std::wstring(expanded) + name, stem + L" (Tourne)");
            else option(name, stem);
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }
}

// Explorer's windows open their theme data again, from the style now chosen.
BOOL CALLBACK RethemeChild(HWND wnd, LPARAM) {
    SendMessageTimeoutW(wnd, WM_THEMECHANGED, 0, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 200, nullptr);
    return TRUE;
}

BOOL CALLBACK RethemeWindow(HWND wnd, LPARAM) {
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    if (pid != GetCurrentProcessId()) return TRUE;
    RethemeChild(wnd, 0);
    EnumChildWindows(wnd, RethemeChild, 0);
    RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
    return TRUE;
}

void ApplyStyle(bool retheme) {
    std::wstring path = ChosenStyleFile();

    ThemeFileData file{};
    bool loaded = !path.empty() && LoadThemeFile(path, &file);
    AcquireSRWLockExclusive(&g_styleLock);
    if (g_styleLoaded) g_oldStyleFiles.push_back(g_styleFile);
    g_styleFile = file;
    g_styleLoaded = loaded;
    ReleaseSRWLockExclusive(&g_styleLock);
    Wh_Log(L"Explorer style: %s", loaded ? path.c_str() : L"Windows' own");
    if (retheme) EnumWindows(RethemeWindow, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Dark dialogs: Explorer's classic dialogs, which Windows leaves light. Each
// dialog is caught as it's created; once its controls exist (WM_INITDIALOG)
// they get Windows' own dark control styles, and the dialog paints the
// backgrounds and text dark.

enum DarkMode { kDarkFollow, kDarkAlways, kDarkOff };
DarkMode g_darkMode = kDarkFollow;
// The visual style's own colours when it's a custom one (see
// theme-gaps-palette.cpp), else Windows' dark ones.
COLORREF kDarkGround = RGB(32, 32, 32);
COLORREF kDarkControl = RGB(43, 43, 43);
COLORREF kDarkText = RGB(240, 240, 240);
HBRUSH g_darkGround, g_darkControl;

void UpdateDarkColors() {
    GapPalette p = CurrentGapPalette();
    COLORREF ground = p.custom ? p.ground : RGB(32, 32, 32);
    COLORREF control = p.custom ? p.raised : RGB(43, 43, 43);
    COLORREF text = p.custom ? p.text : RGB(240, 240, 240);
    if (g_darkGround && ground == kDarkGround && control == kDarkControl && text == kDarkText) return;
    // The old brushes stay: dialogs still open may be painting with them.
    kDarkGround = ground;
    kDarkControl = control;
    kDarkText = text;
    g_darkGround = CreateSolidBrush(ground);
    g_darkControl = CreateSolidBrush(control);
}

bool WantDark() {
    if (g_darkMode == kDarkOff) return false;
    if (g_darkMode == kDarkAlways) return true;
    DWORD light = 1, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    return !light;
}

// uxtheme's own switch for a window's dark controls (#133).
void AllowDarkFor(HWND wnd) {
    using AllowDarkModeForWindow_t = BOOL(WINAPI*)(HWND, BOOL);
    static auto allow =
        (AllowDarkModeForWindow_t)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(133));
    if (allow) allow(wnd, TRUE);
}

BOOL CALLBACK DarkenControl(HWND control, LPARAM) {
    WCHAR cls[64] = L"";
    GetClassNameW(control, cls, ARRAYSIZE(cls));
    AllowDarkFor(control);
    if (!_wcsicmp(cls, WC_EDITW) || !_wcsicmp(cls, WC_COMBOBOXW) || !_wcsicmp(cls, L"ComboBoxEx32")) {
        SetWindowTheme(control, L"DarkMode_CFD", nullptr);
    } else if (!_wcsicmp(cls, WC_LISTVIEWW)) {
        SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
        ListView_SetBkColor(control, kDarkControl);
        ListView_SetTextBkColor(control, kDarkControl);
        ListView_SetTextColor(control, kDarkText);
    } else if (!_wcsicmp(cls, WC_TREEVIEWW)) {
        SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
        TreeView_SetBkColor(control, kDarkControl);
        TreeView_SetTextColor(control, kDarkText);
    } else if (!_wcsicmp(cls, WC_BUTTONW) || !_wcsicmp(cls, WC_SCROLLBARW) || !_wcsicmp(cls, WC_LISTBOXW)) {
        SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
    } else if (!_wcsicmp(cls, L"#32770")) {
        EnableThemeDialogTexture(control, ETDT_DISABLE);
    }
    return TRUE;
}

LRESULT CALLBACK DarkDialogProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_INITDIALOG: {
            LRESULT result = DefSubclassProc(wnd, msg, wParam, lParam);
            EnableThemeDialogTexture(wnd, ETDT_DISABLE);
            EnumChildWindows(wnd, DarkenControl, 0);
            return result;
        }
        case WM_CTLCOLORDLG:
            return (LRESULT)g_darkGround;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC dc = (HDC)wParam;
            SetTextColor(dc, kDarkText);
            SetBkColor(dc, kDarkGround);
            return (LRESULT)g_darkGround;
        }
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC dc = (HDC)wParam;
            SetTextColor(dc, kDarkText);
            SetBkColor(dc, kDarkControl);
            return (LRESULT)g_darkControl;
        }
        case WM_NCDESTROY:
            RemoveWindowSubclass(wnd, DarkDialogProc, 0);
            break;
    }
    return DefSubclassProc(wnd, msg, wParam, lParam);
}

// The navigation pane's lines and gaps (see below).
std::atomic<bool> g_navHideLines{false}, g_navCompact{false};
bool IsNavTree(HWND tree);
void AttachNav(HWND tree);

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;

HWND WINAPI CreateWindowExW_Hook(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y,
                                 int width, int height, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param) {
    HWND wnd = CreateWindowExW_Original(exStyle, className, windowName, style, x, y, width, height, parent, menu,
                                        instance, param);
    if (wnd && g_darkMode != kDarkOff) {
        WCHAR cls[16] = L"";
        if (GetClassNameW(wnd, cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"#32770") && WantDark()) {
            UpdateDarkColors();
            AllowDarkFor(wnd);
            if (!(style & WS_CHILD)) {
                BOOL dark = TRUE;
                DwmSetWindowAttribute(wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));
            }
            SetWindowSubclass(wnd, DarkDialogProc, 0, 0);
        }
    }
    // A navigation pane, while its lines or gaps are being changed.
    if (wnd && (g_navHideLines || g_navCompact) && className && !IS_INTRESOURCE(className) &&
        !_wcsicmp(className, L"SysTreeView32") && IsNavTree(wnd))
        AttachNav(wnd);
    return wnd;
}

void LoadDarkSetting() {
    PCWSTR value = Wh_GetStringSetting(L"darkDialogs");
    std::wstring mode = value ? value : L"";
    Wh_FreeStringSetting(value);
    g_darkMode = mode == L"off" ? kDarkOff : mode == L"always" ? kDarkAlways : kDarkFollow;
}

////////////////////////////////////////////////////////////////////////////////
// The navigation pane's separator lines and gaps (the idea is Languster's Explorer TreeLine
// Killer). Windows sets the pane's groups apart with rows of double height holding a line. The gaps
// can close: those rows made single height, remembered, and given back when switched off. The lines
// can go: after the pane paints, any row across a group's row that is one flat colour (not the
// background) is painted over with the background, so nothing depends on where Windows puts it.
// Nothing is subclassed unless one of them is on.

// Posted to a pane to redo its rows on its own thread (registered on first use, not while loading).
UINT NavRecompactMessage() {
    static UINT message = RegisterWindowMessageW(L"TourneExplorerStyle_NavRecompact");
    return message;
}
SRWLOCK g_navLock = SRWLOCK_INIT;
struct NavTree {
    HWND tree, parent;
    std::vector<std::pair<HTREEITEM, int>> compacted;  // rows made single height, and their height before
    bool pending = false;
};
std::vector<NavTree> g_navTrees;
thread_local bool t_navSetting = false;  // our own row changes, not the pane's

bool IsNavTree(HWND tree) {
    WCHAR cls[32];
    if (!GetClassNameW(tree, cls, ARRAYSIZE(cls)) || wcscmp(cls, L"SysTreeView32")) return false;
    HWND h = GetParent(tree);
    for (int i = 0; h && i < 6; i++, h = GetParent(h))
        if (GetClassNameW(h, cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"NamespaceTreeControl")) return true;
    return false;
}

int NavDepth(HWND tree, HTREEITEM item) {
    int depth = 0;
    for (HTREEITEM p = TreeView_GetParent(tree, item); p && depth < 8; p = TreeView_GetParent(tree, p)) depth++;
    return depth;
}

// The group rows (the pane's top two levels) at single height, or given back.
void Recompact(HWND tree) {
    bool compact = g_navCompact;
    std::vector<std::pair<HTREEITEM, int>> done;
    AcquireSRWLockExclusive(&g_navLock);
    for (auto& t : g_navTrees)
        if (t.tree == tree) {
            done.swap(t.compacted);
            t.pending = false;
        }
    ReleaseSRWLockExclusive(&g_navLock);
    t_navSetting = true;
    auto setIntegral = [&](HTREEITEM item, int integral) {
        TVITEMEXW tv{};
        tv.mask = TVIF_HANDLE | TVIF_INTEGRAL;
        tv.hItem = item;
        tv.iIntegral = integral;
        SendMessageW(tree, TVM_SETITEMW, 0, (LPARAM)&tv);
    };
    if (!compact) {
        for (auto const& [item, integral] : done) setIntegral(item, integral);
        done.clear();
    } else {
        auto visit = [&](HTREEITEM item) {
            TVITEMEXW tv{};
            tv.mask = TVIF_HANDLE | TVIF_INTEGRAL;
            tv.hItem = item;
            if (!SendMessageW(tree, TVM_GETITEMW, 0, (LPARAM)&tv) || tv.iIntegral <= 1) return;
            done.push_back({item, tv.iIntegral});
            setIntegral(item, 1);
        };
        for (HTREEITEM root = TreeView_GetRoot(tree); root; root = TreeView_GetNextSibling(tree, root)) {
            visit(root);
            for (HTREEITEM kid = TreeView_GetChild(tree, root); kid; kid = TreeView_GetNextSibling(tree, kid)) visit(kid);
        }
    }
    t_navSetting = false;
    AcquireSRWLockExclusive(&g_navLock);
    for (auto& t : g_navTrees)
        if (t.tree == tree) t.compacted.insert(t.compacted.end(), done.begin(), done.end());
    ReleaseSRWLockExclusive(&g_navLock);
}

// After the pane has painted: on each group row, a row of pixels that is one flat colour across (not
// the background) is a separator line, and is painted over with the background.
void HideNavLines(HWND tree, HDC dc) {
    RECT client;
    GetClientRect(tree, &client);
    int w = client.right;
    if (w < 16) return;
    COLORREF bg = TreeView_GetBkColor(tree);
    bool sampleBg = bg == (COLORREF)-1;
    HDC mem = CreateCompatibleDC(dc);
    for (HTREEITEM item = TreeView_GetFirstVisible(tree); item; item = TreeView_GetNextVisible(tree, item)) {
        RECT r{};
        if (!TreeView_GetItemRect(tree, item, &r, FALSE) || r.top >= client.bottom) break;
        if (NavDepth(tree, item) > 1) continue;
        int h = r.bottom - r.top;
        if (h <= 0) continue;
        BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
        uint32_t* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
        if (!bmp) continue;
        HGDIOBJ old = SelectObject(mem, bmp);
        BitBlt(mem, 0, 0, w, h, dc, 0, r.top, SRCCOPY);
        GdiFlush();
        uint32_t ground = sampleBg ? bits[w - 1] & 0xFFFFFF : (GetRValue(bg) << 16) | (GetGValue(bg) << 8) | GetBValue(bg);
        auto alike = [](uint32_t a, uint32_t b, int tolerance) {
            for (int shift = 0; shift < 24; shift += 8)
                if (std::abs((int)((a >> shift) & 0xFF) - (int)((b >> shift) & 0xFF)) > tolerance) return false;
            return true;
        };
        HBRUSH brush = CreateSolidBrush(RGB((ground >> 16) & 0xFF, (ground >> 8) & 0xFF, ground & 0xFF));
        for (int y = 0; y < h; y++) {
            uint32_t const* row = bits + (size_t)y * w;
            uint32_t first = row[w * 15 / 100] & 0xFFFFFF;
            if (alike(first, ground, 10)) continue;
            bool flat = true;
            for (int k = 1; k <= 15 && flat; k++) flat = alike(row[w * 15 / 100 + (w * 70 / 100) * k / 15] & 0xFFFFFF, first, 8);
            if (!flat) continue;
            RECT line{0, r.top + y, w, r.top + y + 1};
            FillRect(dc, &line, brush);
        }
        DeleteObject(brush);
        SelectObject(mem, old);
        DeleteObject(bmp);
    }
    DeleteDC(mem);
}

LRESULT CALLBACK NavTreeProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam, DWORD_PTR) {
    if (msg == NavRecompactMessage()) {
        Recompact(wnd);
        return 0;
    }
    LRESULT result = DefSubclassProc(wnd, msg, wParam, lParam);
    // The pane added or changed rows: they're made compact again, once, after it's done.
    if ((msg == TVM_INSERTITEMW || msg == TVM_SETITEMW || msg == TVM_EXPAND) && g_navCompact && !t_navSetting) {
        bool post = false;
        AcquireSRWLockExclusive(&g_navLock);
        for (auto& t : g_navTrees)
            if (t.tree == wnd && !t.pending) post = t.pending = true;
        ReleaseSRWLockExclusive(&g_navLock);
        if (post) PostMessageW(wnd, NavRecompactMessage(), 0, 0);
    }
    return result;
}

LRESULT CALLBACK NavParentProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam, DWORD_PTR tree) {
    if (msg == WM_NOTIFY && g_navHideLines) {
        auto* nm = (NMHDR*)lParam;
        if (nm && nm->hwndFrom == (HWND)tree && nm->code == NM_CUSTOMDRAW) {
            auto* cd = (NMTVCUSTOMDRAW*)lParam;
            LRESULT result = DefSubclassProc(wnd, msg, wParam, lParam);
            if (cd->nmcd.dwDrawStage == CDDS_PREPAINT) return result | CDRF_NOTIFYPOSTPAINT;
            if (cd->nmcd.dwDrawStage == CDDS_POSTPAINT) HideNavLines((HWND)tree, cd->nmcd.hdc);
            return result;
        }
    }
    return DefSubclassProc(wnd, msg, wParam, lParam);
}

void AttachNav(HWND tree) {
    HWND parent = GetParent(tree);
    if (!parent) return;
    AcquireSRWLockExclusive(&g_navLock);
    bool known = std::any_of(g_navTrees.begin(), g_navTrees.end(), [&](NavTree const& t) { return t.tree == tree; });
    if (!known) g_navTrees.push_back({tree, parent});
    ReleaseSRWLockExclusive(&g_navLock);
    if (known) return;
    WindhawkUtils::SetWindowSubclassFromAnyThread(tree, NavTreeProc, 0);
    WindhawkUtils::SetWindowSubclassFromAnyThread(parent, NavParentProc, (DWORD_PTR)tree);
    if (g_navCompact) PostMessageW(tree, NavRecompactMessage(), 0, 0);
    InvalidateRect(tree, nullptr, TRUE);
}

BOOL CALLBACK FindNavTrees(HWND h, LPARAM) {
    if (IsNavTree(h)) AttachNav(h);
    return TRUE;
}

BOOL CALLBACK FindNavWindows(HWND h, LPARAM) {
    WCHAR cls[32];
    if (GetClassNameW(h, cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"CabinetWClass")) EnumChildWindows(h, FindNavTrees, 0);
    return TRUE;
}

// The settings read and put in place: panes open now are subclassed, or let go (rows given back first).
void ApplyNavSettings(bool unloading) {
    g_navHideLines = !unloading && Wh_GetIntSetting(L"navHideLines") != 0;
    g_navCompact = !unloading && Wh_GetIntSetting(L"navCompact") != 0;
    AcquireSRWLockShared(&g_navLock);
    std::vector<NavTree> trees = g_navTrees;
    ReleaseSRWLockShared(&g_navLock);
    for (auto const& t : trees) {
        if (!IsWindow(t.tree)) continue;
        DWORD_PTR ignored;
        SendMessageTimeoutW(t.tree, NavRecompactMessage(), 0, 0, SMTO_ABORTIFHUNG, 1000, &ignored);
        InvalidateRect(t.tree, nullptr, TRUE);
    }
    if (g_navHideLines || g_navCompact) {
        EnumWindows(FindNavWindows, 0);
        return;
    }
    for (auto const& t : trees) {
        if (!IsWindow(t.tree)) continue;
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(t.tree, NavTreeProc);
        if (IsWindow(t.parent)) WindhawkUtils::RemoveWindowSubclassFromAnyThread(t.parent, NavParentProc);
    }
    AcquireSRWLockExclusive(&g_navLock);
    g_navTrees.clear();
    ReleaseSRWLockExclusive(&g_navLock);
}

////////////////////////////////////////////////////////////////////////////////
// The command bar, in explorerframe.dll: Windows 11's (a XAML island) unless
// it's refused, then Windows 10's ribbon; with the ribbon refused too, the
// folder view shows the Windows 7 command bar Control Panel still uses.

enum CommandBar { kBarWindows11, kBarRibbon, kBarWindows7 };
std::atomic<int> g_commandBar{kBarWindows11};

using IsCommandBarEnabledAndAvailable_t = bool(WINAPI*)(void* frame);
using ShouldShowRibbon_t = bool(WINAPI*)(void* browser, void* item);
using CanShowModernNavBar_t = bool(WINAPI*)();
IsCommandBarEnabledAndAvailable_t IsCommandBarEnabledAndAvailable_Original;
ShouldShowRibbon_t ShouldShowRibbon_Original;
CanShowModernNavBar_t CanShowModernNavBar_Original;

bool WINAPI IsCommandBarEnabledAndAvailable_Hook(void* frame) {
    if (g_commandBar != kBarWindows11) return false;
    return IsCommandBarEnabledAndAvailable_Original(frame);
}

bool WINAPI ShouldShowRibbon_Hook(void* browser, void* item) {
    if (g_commandBar == kBarWindows7) return false;
    return ShouldShowRibbon_Original(browser, item);
}

bool WINAPI CanShowModernNavBar_Hook() {
    if (g_commandBar != kBarWindows11) return false;
    return CanShowModernNavBar_Original();
}

// Windows 11's header (tabs, address bar, command bar) is a XAML island of
// its own; with the island said to be unavailable, windows get Windows 10's
// frame instead, as with StartAllBack.
using IsAvailable_t = bool(WINAPI*)();
IsAvailable_t IsXamlIslandAvailable_Original;
IsAvailable_t IsTabsAvailable_Original;

bool WINAPI IsXamlIslandAvailable_Hook() {
    if (g_commandBar != kBarWindows11) return false;
    return IsXamlIslandAvailable_Original();
}

bool WINAPI IsTabsAvailable_Hook() {
    if (g_commandBar != kBarWindows11) return false;
    return IsTabsAvailable_Original();
}

// The Welcome page's preset (the taskbar part's, read by its full name
// through a variable, so the build doesn't file it under this part), which
// decides the command bar and details pane while chosen.
std::wstring ExplorerPreset() {
    constexpr PCWSTR kPreset = L"taskbar.preset";
    PCWSTR value = Wh_GetStringSetting(kPreset);
    std::wstring preset = value ? value : L"";
    Wh_FreeStringSetting(value);
    return preset;
}

// @inline explorer-layout.cpp

HANDLE g_symbolThread;
std::atomic<bool> g_unloading{false};

// explorerframe's functions are found by their symbols (Windhawk fetches
// them the first time), off the startup path. The drive categorizer's are
// found here too, as finding them makes a COM object.
DWORD WINAPI SymbolThread(LPVOID) {
    bool drives = HookDriveGroups();
    if (!drives) Wh_Log(L"The drive type categorizer wasn't found");
    // The side panes' sizer minimum (shell32's), so they can be dragged narrower again.
    if (HookSizerSymbols()) drives = true;  // (something to apply either way)
    HMODULE frame = LoadLibraryExW(L"explorerframe.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!frame) {
        if (drives && !g_unloading) Wh_ApplyHookOperations();
        return 0;
    }
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(?IsCommandBarEnabledAndAvailable@CExplorerFrame@@AEAA_NXZ)"},
         &IsCommandBarEnabledAndAvailable_Original, IsCommandBarEnabledAndAvailable_Hook, true},
        {{LR"(?_ShouldShowRibbon@CShellBrowser@@AEAA_NPEAUIShellItem@@@Z)"},
         &ShouldShowRibbon_Original, ShouldShowRibbon_Hook, true},
        {{LR"(?CanShowModernNavBar@@YA_NXZ)"}, &CanShowModernNavBar_Original, CanShowModernNavBar_Hook, true},
        {{LR"(?IsXamlIslandAvailable@CachedExplorerExtensionState@@SA_NXZ)"}, &IsXamlIslandAvailable_Original,
         IsXamlIslandAvailable_Hook, true},
        {{LR"(?IsTabsAvailable@CachedExplorerExtensionState@@SA_NXZ)"}, &IsTabsAvailable_Original,
         IsTabsAvailable_Hook, true},
    };
    WH_HOOK_SYMBOLS_OPTIONS options = {};
    options.optionsSize = sizeof(options);
    options.noUndecoratedSymbols = TRUE;
    if (!WindhawkUtils::HookSymbols(frame, hooks, ARRAYSIZE(hooks), &options)) {
        Wh_Log(L"Explorer's command bar functions weren't found");
        if (drives && !g_unloading) Wh_ApplyHookOperations();
        return 0;
    }
    if (!g_unloading) Wh_ApplyHookOperations();
    return 0;
}

void LoadCommandBarSetting() {
    PCWSTR value = Wh_GetStringSetting(L"commandBar");
    std::wstring bar = value ? value : L"";
    Wh_FreeStringSetting(value);
    std::wstring preset = ExplorerPreset();
    if (preset == L"proper11") bar = L"windows11";
    else if (preset == L"kinda10") bar = L"ribbon";
    else if (preset == L"remastered7") bar = L"windows7";
    g_commandBar = bar == L"ribbon" ? kBarRibbon : bar == L"windows7" ? kBarWindows7 : kBarWindows11;
}

BOOL Wh_ModInit() {
    LoadCommandBarSetting();
    LoadLayoutSettings();
    HookLayouts();
    HMODULE ux = LoadLibraryW(L"uxtheme.dll");
    if (!ux) return FALSE;
    LoadGapSource();
    UpdateDarkColors();
    LoadDarkSetting();
    Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook, (void**)&CreateWindowExW_Original);
    auto openThemeData = (OpenThemeData_t)GetProcAddress(ux, "OpenThemeData");
    auto openThemeDataEx = (OpenThemeDataEx_t)GetProcAddress(ux, "OpenThemeDataEx");
    auto openThemeDataForDpi = (OpenThemeDataForDpi_t)GetProcAddress(ux, "OpenThemeDataForDpi");
    if (openThemeData) Wh_SetFunctionHook((void*)openThemeData, (void*)OpenThemeData_Hook, (void**)&OpenThemeData_Original);    if (openThemeDataEx)
        Wh_SetFunctionHook((void*)openThemeDataEx, (void*)OpenThemeDataEx_Hook, (void**)&OpenThemeDataEx_Original);
    if (openThemeDataForDpi)
        Wh_SetFunctionHook((void*)openThemeDataForDpi, (void*)OpenThemeDataForDpi_Hook,
                           (void**)&OpenThemeDataForDpi_Original);
    PublishStyleChoices();
    ApplyStyle(false);
    return TRUE;
}

void Wh_ModAfterInit() {
    // Windows already open get the style too, and their navigation panes the line and gap settings.
    EnumWindows(RethemeWindow, 0);
    ApplyNavSettings(false);
    g_symbolThread = CreateThread(nullptr, 0, SymbolThread, nullptr, 0, nullptr);
}

void Wh_ModUninit() {
    // The symbol thread must be done with this module's code before it unloads.
    g_unloading = true;
    if (g_symbolThread) {
        WaitForSingleObject(g_symbolThread, INFINITE);
        CloseHandle(g_symbolThread);
        g_symbolThread = nullptr;
    }
    ApplyNavSettings(true);  // the panes' rows given back, and let go
    AcquireSRWLockExclusive(&g_styleLock);
    g_styleLoaded = false;
    ReleaseSRWLockExclusive(&g_styleLock);
    EnumWindows(RethemeWindow, 0);
    // The files stay mapped: handles opened from them may still be in use.
}

void Wh_ModSettingsChanged() {
    LoadCommandBarSetting();
    LoadLayoutSettings();
    LoadGapSource();
    LoadDarkSetting();
    PublishStyleChoices();
    ApplyStyle(true);
    ApplyNavSettings(false);
}
