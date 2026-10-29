////////////////////////////////////////////////////////////////////////////////
// Window tiling, in the manner of komorebi ("// @inline tiling.cpp", in the
// Switcher, whose keyboard hook it shares): the windows on a screen laid out
// by a layout, with gaps around and between them, clear of the taskbar. By
// hand with chords on a key the user picks (the tiling key, none until set),
// or automatically as windows come and go. Each screen keeps the layout
// picked for it on each of Windows' desktops, across restarts too.
//
// Windows are placed by their visible frames (DWM's extended frame bounds),
// so the gaps are what the eye sees, not the invisible resize borders.

// @inline tiling-geometry.cpp

struct TilingSettings {
    UINT leaderMods = 0, leaderVk = 0;  // the tiling key (MOD_ flags and a key); none when 0
    bool autoTile = false;
    TileLayout layout = TileLayout::Columns;
    TileLayout otherLayout = TileLayout::Same;
    int ratio = 50;
    int gapTop = 4, gapBottom = 2, gapLeft = 6, gapRight = 6, gapInner = 6;
    bool reserveTaskbar = true;
    bool restoreMaximized = true;
    bool tileElevated = false;
    bool keepResized = true;
    int newWindow = 0;  // kNewEnd, kNewFirst, kNewAfterFocused or kNewBeforeFocused
    bool rememberAll = true;
    int fit = 0;  // kFitForce, kFitOverlap or kFitFloat
    int minWidth = 0, minHeight = 0;
    std::wstring floatApps, floatTitles, floatClasses;
    // Slice: which window keeps the strip (its program, class or title, with *
    // and ?; empty: the first window on the screen), which edge, how much of
    // the screen it takes and what lays out the rest.
    std::wstring sliceWindow;
    SliceSpec slice;
    // ScreenSplit: the screen's regions, from the tiling.split lines.
    struct Region {
        std::wstring name, match;              // match: programs, classes or titles; empty: everything left over
        int l = 0, t = 0, w = 100, h = 100;    // its share of the screen, in percent
        TileLayout layout = TileLayout::Columns;
    };
    std::vector<Region> regions;
    // Script: a layout of the user's own, from tiling.script ("spread, 3": the
    // file in the Layouts folder, then whatever the script reads as options).
    std::wstring script, scriptOptions;
    int focusBorder = 0;        // kBorderNone, kBorderTiled or kBorderAll
    bool focusAccent = true;    // Windows' accent colour, or focusColor
    COLORREF focusColor = 0;
    motion::Anim move;          // the motion line "windowsMove" (motion.cpp): windows sliding to their places
    // komorebi's layout_rules: from so many windows on a screen, this layout
    // (fewest first). Screens whose layout was picked by hand keep theirs.
    std::vector<std::pair<int, TileLayout>> rules;
};

enum { kBorderNone, kBorderTiled, kBorderAll };

// A window that won't shrink to its tile (its own minimum size): pushed onto
// it anyway, as dwm with resizehints = 0 does; left at its own size over its
// neighbours; or left floating and the rest tiled without it.
enum { kFitForce, kFitOverlap, kFitFloat };

// Tiling's helper in programs running as administrator (see the suite's
// suite_tile_agent, which these must match).
constexpr WCHAR kTileAgentClass[] = L"TourneTileAgent_" WH_MOD_ID;
constexpr ULONG_PTR kTilePlaceCopyId = 0x54504C43;  // "TPLC"
struct TilePlaceRequest {
    uint64_t hwnd;
    RECT frame;
    int force;
};

TilingSettings g_tiling;
std::atomic<UINT> g_leaderVk, g_leaderMods;  // read by the hook thread

// What the chords ask for (WM_APP_TILE's wParam; lParam the direction or step).
enum TileCommand { kTileNow, kTileMove, kTileFocus, kTileFloat, kTileLayout, kTileRatio, kTileAuto, kTileLayouts, kTileApplyLayout };
enum TileDirection { kLeft, kRight, kUp, kDown };
constexpr UINT WM_APP_TILE = WM_APP + 12;
constexpr UINT_PTR kTimerTile = 3;

// The running state, on the switcher's thread.
std::vector<HWND> g_tileOrder;  // every tiled window, in its order
HWND g_focused, g_focusedBefore;  // the window in front, and the one before it (where a new window lands)
std::vector<HWND> g_floating;   // set floating with the tiling key and Space
bool g_autoTile = false;
bool g_userMoving = false;
HWND g_moving;                  // the window being dragged or resized by hand
RECT g_movingFrame;             // and its frame when that began
HWINEVENTHOOK g_tileHooks[4];

// Windows resized by hand: left floating where they were put, and the place
// remembered for the program (its file name and window class), so its next
// window opens there too. One window at a time holds a program's place.
struct KeptPlace {
    std::wstring key;
    RECT frame;
};
std::vector<KeptPlace> g_keptPlaces;                      // saved, newest last
std::vector<std::pair<HWND, std::wstring>> g_keptWindows;  // the windows holding them
constexpr size_t kMostKeptPlaces = 64;
constexpr WCHAR kKeptPlacesValue[] = L"switcher.tiling.keptPlaces";

// Each screen's layout and main window's share as picked (the tiling key
// and N, [ or ], or a layout applied), one for each of Windows' desktops,
// kept across restarts. A screen is known by its monitor's device name, so
// it keeps its choice when the screens are rearranged.
struct LayoutChoice {
    std::wstring key;                      // the screen's and the desktop's names, as "screen|desktop"
    TileLayout layout = TileLayout::Same;  // Same: none picked, the settings' layout
    int ratioStep = 0;                     // added to the settings' share
};
std::vector<LayoutChoice> g_layoutChoices;  // newest last
std::vector<std::pair<std::wstring, std::wstring>> g_screenKeys;  // names found, by the screen's GDI name
constexpr size_t kMostLayoutChoices = 64;
constexpr WCHAR kLayoutChoicesValue[] = L"switcher.tiling.choices";

// The layouts window and the places (tiling-layouts.cpp).
void OpenLayoutsWindow();
void ApplyLayoutNumber(int index);
PCWSTR LayoutName(TileLayout layout);
bool Placeable(HWND h);

TileLayout ParseLayout(std::wstring const& v, TileLayout fallback) {
    if (v == L"off") return TileLayout::Off;
    if (v == L"same") return TileLayout::Same;
    if (v == L"columns") return TileLayout::Columns;
    if (v == L"rows") return TileLayout::Rows;
    if (v == L"verticalStack") return TileLayout::VerticalStack;
    if (v == L"rightMainVerticalStack") return TileLayout::RightMainVerticalStack;
    if (v == L"horizontalStack") return TileLayout::HorizontalStack;
    if (v == L"ultrawideVerticalStack") return TileLayout::UltrawideVerticalStack;
    if (v == L"grid") return TileLayout::Grid;
    if (v == L"bsp") return TileLayout::Bsp;
    if (v == L"spiral") return TileLayout::Spiral;
    if (v == L"threeColumn") return TileLayout::ThreeColumn;
    if (v == L"slice") return TileLayout::Slice;
    if (v == L"screenSplit") return TileLayout::ScreenSplit;
    if (v == L"script") return TileLayout::Script;
    if (v == L"monocle") return TileLayout::Monocle;
    if (v == L"layers") return TileLayout::Layers;
    return fallback;
}

// "1: bsp, 4: grid" (komorebi's layout_rules; "=" works too, and the
// names in any case, so "4: Grid" from a komorebi.json reads).
std::vector<std::pair<int, TileLayout>> ParseLayoutRules(std::wstring const& text) {
    std::vector<std::pair<int, TileLayout>> rules;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find_first_of(L",;", at);
        std::wstring item = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? text.size() : end + 1;
        size_t mark = item.find_first_of(L":=");
        if (mark == std::wstring::npos) continue;
        int count = _wtoi(item.c_str());
        std::wstring name;
        for (wchar_t c : item.substr(mark + 1))
            if (!iswspace(c)) name += c;
        TileLayout layout = TileLayout::Same;
        for (TileLayout l : kLayoutCycle)
            if (!_wcsicmp(LayoutName(l), name.c_str())) layout = l;
        if (count < 1 || layout == TileLayout::Same) continue;
        std::erase_if(rules, [count](auto const& r) { return r.first == count; });
        rules.push_back({count, layout});
    }
    std::sort(rules.begin(), rules.end(), [](auto const& a, auto const& b) { return a.first < b.first; });
    return rules;
}

// "alt+win+84": modifiers, then the virtual-key code in decimal (Windhawk's
// hotkey setting, as the taskbar reads its own).
void ParseHotkey(std::wstring const& key, UINT* mods, UINT* vk) {
    *mods = *vk = 0;
    size_t at = 0;
    while (at <= key.size()) {
        size_t end = key.find(L'+', at);
        std::wstring part = key.substr(at, end == std::wstring::npos ? end : end - at);
        if (part == L"ctrl") *mods |= MOD_CONTROL;
        else if (part == L"alt") *mods |= MOD_ALT;
        else if (part == L"shift") *mods |= MOD_SHIFT;
        else if (part == L"win") *mods |= MOD_WIN;
        else if (!part.empty()) *vk = (UINT)_wtoi(part.c_str());
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    // A modifier on its own isn't a key to hold.
    switch (*vk) {
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
        case VK_MENU: case VK_LMENU: case VK_RMENU: case VK_LWIN: case VK_RWIN:
            *vk = 0;
    }
    if (*vk > 0xFE) *vk = 0;
}

// "#RRGGBB" (or without the #). False if it isn't one.
bool ParseColor(std::wstring v, COLORREF* out) {
    if (!v.empty() && v[0] == L'#') v.erase(0, 1);
    if (v.size() != 6 || v.find_first_not_of(L"0123456789abcdefABCDEF") != std::wstring::npos) return false;
    unsigned long rgb = wcstoul(v.c_str(), nullptr, 16);
    *out = RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    return true;
}

void LoadLayoutChoices();

// The ScreenSplit regions, one a line, as the window rules are written:
//
//     region = sidebar, 0 0 25 100, verticalStack, Spotify.exe, Discord.exe
//     region = main, 25 0 75 100, grid
//
// The name is for reading, the four numbers are the region's share of the
// screen in percent (left top width height), then its layout, then the
// programs, classes or titles whose windows go in it. A region with none named
// takes everything left over; with no such region, the last one does.
std::vector<TilingSettings::Region> ReadRegions() {
    std::vector<TilingSettings::Region> out;
    // Its own trim: the one in binds.cpp comes after this file.
    auto trim = [](std::wstring const& text) {
        size_t a = text.find_first_not_of(L" \t\r\n"), b = text.find_last_not_of(L" \t\r\n");
        return a == std::wstring::npos ? std::wstring() : text.substr(a, b - a + 1);
    };
    int empty = 0;
    for (int i = 0; i < 64 && empty < 4; i++) {
        PCWSTR value = Wh_GetStringSetting(L"tiling.split[%d]", i);
        std::wstring line = trim(value ? value : L"");
        Wh_FreeStringSetting(value);
        if (line.empty() || line[0] == L'#') {
            empty++;
            continue;
        }
        empty = 0;
        if (size_t eq = line.find(L'='); eq != std::wstring::npos && Lower(trim(line.substr(0, eq))) == L"region")
            line = trim(line.substr(eq + 1));
        std::vector<std::wstring> parts;
        for (size_t at = 0; at <= line.size();) {
            size_t comma = line.find(L',', at);
            parts.push_back(trim(line.substr(at, comma == std::wstring::npos ? comma : comma - at)));
            if (comma == std::wstring::npos) break;
            at = comma + 1;
        }
        if (parts.size() < 2) {
            Wh_Log(L"tiling: a region needs a name and its share of the screen: %s", line.c_str());
            continue;
        }
        TilingSettings::Region r;
        r.name = parts[0];
        if (swscanf(parts[1].c_str(), L"%d %d %d %d", &r.l, &r.t, &r.w, &r.h) != 4) {
            Wh_Log(L"tiling: a region's share reads as four numbers, left top width height: %s", parts[1].c_str());
            continue;
        }
        if (parts.size() > 2 && !parts[2].empty()) r.layout = ParseLayout(parts[2], TileLayout::Columns);
        for (size_t i2 = 3; i2 < parts.size(); i2++)
            if (!parts[i2].empty()) r.match += (r.match.empty() ? L"" : L",") + parts[i2];
        out.push_back(std::move(r));
    }
    return out;
}

void LoadTilingSettings() {
    TilingSettings s;
    ParseHotkey(TextSetting(L"tiling.leader"), &s.leaderMods, &s.leaderVk);
    s.autoTile = Wh_GetIntSetting(L"tiling.auto") != 0;
    s.layout = ParseLayout(TextSetting(L"tiling.layout"), TileLayout::Columns);
    if (s.layout == TileLayout::Same || s.layout == TileLayout::Off) s.layout = TileLayout::Columns;
    s.otherLayout = ParseLayout(TextSetting(L"tiling.otherLayout"), TileLayout::Same);
    s.rules = ParseLayoutRules(TextSetting(L"tiling.layoutRules"));
    int ratio = Wh_GetIntSetting(L"tiling.mainRatio");
    s.ratio = ratio > 0 ? std::clamp(ratio, 10, 90) : 50;
    s.gapTop = IntSetting(L"tiling.gapTop", 0, 400);
    s.gapBottom = IntSetting(L"tiling.gapBottom", 0, 400);
    s.gapLeft = IntSetting(L"tiling.gapLeft", 0, 400);
    s.gapRight = IntSetting(L"tiling.gapRight", 0, 400);
    s.gapInner = IntSetting(L"tiling.gapInner", 0, 400);
    s.reserveTaskbar = Wh_GetIntSetting(L"tiling.reserveTaskbar") != 0;
    s.restoreMaximized = Wh_GetIntSetting(L"tiling.restoreMaximized") != 0;
    s.floatApps = TextSetting(L"tiling.floatApps");
    s.floatTitles = TextSetting(L"tiling.floatTitles");
    s.floatClasses = TextSetting(L"tiling.floatClasses");
    s.sliceWindow = TextSetting(L"tiling.sliceWindow");
    std::wstring edge = TextSetting(L"tiling.sliceEdge");
    s.slice.edge = edge == L"right" ? kSliceRight : edge == L"top" ? kSliceTop : edge == L"bottom" ? kSliceBottom : kSliceLeft;
    s.slice.percent = IntSetting(L"tiling.sliceSize", 0, 90);
    s.slice.fallback = ParseLayout(TextSetting(L"tiling.sliceLayout"), TileLayout::VerticalStack);
    s.regions = ReadRegions();
    // "spread, 3": the file, then the rest of the line for the script itself.
    std::wstring script = TextSetting(L"tiling.script");
    size_t comma = script.find(L',');
    s.script = script.substr(0, comma);
    s.scriptOptions = comma == std::wstring::npos ? L"" : script.substr(comma + 1);
    while (!s.script.empty() && iswspace(s.script.back())) s.script.pop_back();
    while (!s.scriptOptions.empty() && iswspace(s.scriptOptions.front())) s.scriptOptions.erase(0, 1);
    s.tileElevated = Wh_GetIntSetting(L"tiling.tileElevated") != 0;
    s.keepResized = Wh_GetIntSetting(L"tiling.keepResized") != 0;
    std::wstring landing = TextSetting(L"tiling.newWindow");
    s.newWindow = landing == L"first" ? kNewFirst
                  : landing == L"afterFocused" ? kNewAfterFocused
                  : landing == L"beforeFocused" ? kNewBeforeFocused
                                                 : kNewEnd;
    s.rememberAll = Wh_GetIntSetting(L"tiling.rememberAll") != 0;
    std::wstring fit = TextSetting(L"tiling.tooSmall");
    s.fit = fit == L"overlap" ? kFitOverlap : fit == L"float" ? kFitFloat : kFitForce;
    s.minWidth = IntSetting(L"tiling.minWidth", 0, 4000);
    s.minHeight = IntSetting(L"tiling.minHeight", 0, 4000);
    std::wstring border = TextSetting(L"tiling.focusBorder");
    s.focusBorder = border == L"tiled" ? kBorderTiled : border == L"all" ? kBorderAll : kBorderNone;
    s.focusAccent = TextSetting(L"tiling.focusColors") != L"custom" || !ParseColor(TextSetting(L"tiling.focusColor"), &s.focusColor);
    s.move = motion::Get(L"windowsMove");
    g_tiling = s;
    g_leaderVk = s.leaderVk;
    g_leaderMods = s.leaderMods;
    g_autoTile = s.autoTile;
    g_screenKeys.clear();
    LoadLayoutChoices();
}

////////////////////////////////////////////////////////////////////////////////
// Which windows, and where

// A window's frame as drawn (its invisible resize borders left out). DWM's bounds can still be the old ones for a
// moment after the window moved or resized (a window just opened, or slow to draw: Discord opened as a small square
// when its rules moved it to another screen and then sized it, each step reading the frame the last one left). So
// they're checked against the window's own rect, which is never late: borders that can't be real (more than 40
// pixels, or the frame outside the window) mean DWM hasn't caught up, and the borders last seen for it are used.
RECT FrameOf(HWND h) {
    static std::unordered_map<HWND, RECT> borders;  // (left, top, right, bottom insets, last seen good)
    static std::mutex lock;  // (the switcher's thread and the binds' both ask)
    RECT window{}, r{};
    if (!GetWindowRect(h, &window)) return window;
    if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) return window;
    RECT in{r.left - window.left, r.top - window.top, window.right - r.right, window.bottom - r.bottom};
    auto real = [](LONG v) { return v >= 0 && v <= 40; };
    std::lock_guard<std::mutex> held(lock);
    if (real(in.left) && real(in.top) && real(in.right) && real(in.bottom)) {
        if (borders.size() > 512) borders.clear();
        borders[h] = in;
        return r;
    }
    RECT b{};
    if (auto it = borders.find(h); it != borders.end()) {
        b = it->second;
    } else if (GetWindowLongW(h, GWL_STYLE) & WS_THICKFRAME) {
        UINT dpi = GetDpiForWindow(h);
        int edge = GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
        b = {edge, 0, edge, edge};
    }
    return {window.left + b.left, window.top + b.top, window.right - b.right, window.bottom - b.bottom};
}

bool IsFloating(HWND h) {
    return std::find(g_floating.begin(), g_floating.end(), h) != g_floating.end();
}

// A window the tiling looks after: an app's own resizable top-level window,
// in view, not set floating.
bool Tileable(HWND h) {
    if (!IsWindowVisible(h) || IsIconic(h) || GetWindow(h, GW_OWNER)) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((style & WS_CHILD) || !(style & WS_THICKFRAME) || (ex & (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE))) return false;
    if (IsZoomed(h) && !g_tiling.restoreMaximized) return false;
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return false;
    if (LeftAlone(h)) return false;  // a program Tourne leaves alone (a game): never moved or sized
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!wcsncmp(cls, L"Tourne", 6)) return false;
    if (IsFloating(h) || IsHungAppWindow(h)) return false;
    if (!g_tiling.floatApps.empty()) {
        std::wstring program = ProgramOf(h);
        if (MatchesList(g_tiling.floatApps, program.substr(program.find_last_of(L'\\') + 1))) return false;
    }
    if (!g_tiling.floatTitles.empty() && MatchesList(g_tiling.floatTitles, Title(h))) return false;
    if (!g_tiling.floatClasses.empty() && MatchesList(g_tiling.floatClasses, cls)) return false;
    // Smaller than the thresholds (as komorebi's minimum_window_width and
    // _height): a little window of its own, not one to tile.
    if (g_tiling.minWidth || g_tiling.minHeight) {
        RECT r = FrameOf(h);
        UINT dpi = GetDpiForWindow(h);
        if (r.right - r.left < MulDiv(g_tiling.minWidth, dpi ? dpi : 96, 96) ||
            r.bottom - r.top < MulDiv(g_tiling.minHeight, dpi ? dpi : 96, 96))
            return false;
    }
    return true;
}

UINT MonitorDpi(HMONITOR monitor) {
    using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    static auto getDpi = (GetDpiForMonitor_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
    UINT x = 96, y = 96;
    if (getDpi) getDpi(monitor, 0 /* MDT_EFFECTIVE_DPI */, &x, &y);
    return x ? x : 96;
}

// The space the windows share on a screen: its work area, clear of the
// suite's taskbar where that doesn't reserve its space (hiding), less the
// outer gaps.
RECT TileArea(HMONITOR monitor) {
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    RECT area = mi.rcWork;
    UINT dpi = MonitorDpi(monitor);
    auto scaled = [dpi](int v) { return MulDiv(v, dpi, 96); };
    // The taskbar's own settings (through variables, so the build doesn't
    // file them under this part). It sits on the main screen.
    constexpr PCWSTR kTaskbarEnabled = L"taskbar.enabled", kTaskbarThickness = L"taskbar.thickness";
    constexpr PCWSTR kTaskbarLocation = L"taskbar.location";
    if (g_tiling.reserveTaskbar && (mi.dwFlags & MONITORINFOF_PRIMARY) && Wh_GetIntSetting(kTaskbarEnabled)) {
        int thickness = Wh_GetIntSetting(kTaskbarThickness);
        thickness = scaled(thickness <= 0 ? 40 : std::clamp(thickness, 20, 96));
        PCWSTR value = Wh_GetStringSetting(kTaskbarLocation);
        std::wstring location = value ? value : L"";
        Wh_FreeStringSetting(value);
        RECT const& m = mi.rcMonitor;
        if (location == L"top") {
            if (area.top == m.top) area.top += thickness;
        } else if (location == L"left") {
            if (area.left == m.left) area.left += thickness;
        } else if (location == L"right") {
            if (area.right == m.right) area.right -= thickness;
        } else if (area.bottom == m.bottom) {
            area.bottom -= thickness;
        }
    }
    area.left += scaled(g_tiling.gapLeft);
    area.top += scaled(g_tiling.gapTop);
    area.right -= scaled(g_tiling.gapRight);
    area.bottom -= scaled(g_tiling.gapBottom);
    return area;
}

// A screen's lasting name: its monitor's device path (which names the
// monitor and the port it's on), or its GDI name if there's none.
std::wstring ScreenKey(HMONITOR monitor) {
    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(monitor, (MONITORINFO*)&mi)) return L"";
    for (auto const& [gdi, key] : g_screenKeys)
        if (gdi == mi.szDevice) return key;
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    std::wstring key = EnumDisplayDevicesW(mi.szDevice, 0, &dd, EDD_GET_DEVICE_INTERFACE_NAME) && dd.DeviceID[0]
                           ? dd.DeviceID
                           : mi.szDevice;
    CharLowerBuffW(key.data(), (DWORD)key.size());
    std::replace_if(key.begin(), key.end(), [](WCHAR c) { return c == L';' || c == L'=' || c == L'|' || c == L','; }, L'_');
    g_screenKeys.push_back({mi.szDevice, key});
    return key;
}

// The desktop showing (Windows' own, of Win+Tab), as its id; empty while
// there has only ever been one.
std::wstring DesktopKey() {
    GUID id{};
    DWORD size = sizeof(id);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops",
                     L"CurrentVirtualDesktop", RRF_RT_REG_BINARY, nullptr, &id, &size) != ERROR_SUCCESS ||
        size != sizeof(id))
        return L"";
    WCHAR text[40] = L"";
    StringFromGUID2(id, text, ARRAYSIZE(text));
    return text;
}

std::wstring ChoiceKey(HMONITOR monitor) {
    return ScreenKey(monitor) + L"|" + DesktopKey();
}

LayoutChoice const* FindLayoutChoice(HMONITOR monitor) {
    std::wstring key = ChoiceKey(monitor);
    for (auto const& c : g_layoutChoices)
        if (c.key == key) return &c;
    return nullptr;
}

// What the choices were made against: when the settings' layouts or share
// change, the choices are let go, so the new settings show.
std::wstring ChoicesMadeWith() {
    return std::wstring(LayoutName(g_tiling.layout)) + L"," +
           (g_tiling.otherLayout == TileLayout::Same ? L"same" : LayoutName(g_tiling.otherLayout)) + L"," +
           std::to_wstring(g_tiling.ratio);
}

void SaveLayoutChoices() {
    if (g_layoutChoices.size() > kMostLayoutChoices)
        g_layoutChoices.erase(g_layoutChoices.begin(), g_layoutChoices.end() - kMostLayoutChoices);
    std::wstring text = L"~settings=" + ChoicesMadeWith() + L";";
    for (auto const& c : g_layoutChoices)
        text += c.key + L"=" + (c.layout == TileLayout::Same ? L"same" : LayoutName(c.layout)) + L"," +
                std::to_wstring(c.ratioStep) + L";";
    Wh_SetStringValue(kLayoutChoicesValue, text.c_str());
}

void LoadLayoutChoices() {
    g_layoutChoices.clear();
    std::wstring text(16384, L'\0');
    Wh_GetStringValue(kLayoutChoicesValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    bool current = false;
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring item(rest.substr(0, end));
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
        size_t eq = item.rfind(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring key = item.substr(0, eq), value = item.substr(eq + 1);
        if (key == L"~settings") {
            current = value == ChoicesMadeWith();
            continue;
        }
        size_t comma = value.find(L',');
        LayoutChoice c{key, ParseLayout(value.substr(0, comma), TileLayout::Same)};
        if (comma != std::wstring::npos) c.ratioStep = std::clamp(_wtoi(value.c_str() + comma + 1), -80, 80);
        g_layoutChoices.push_back(c);
    }
    if (!current && !g_layoutChoices.empty()) {
        g_layoutChoices.clear();
        SaveLayoutChoices();
    }
}

// This screen's choice on the desktop showing, made if there's none yet,
// and moved to the newest.
LayoutChoice& ChoiceFor(HMONITOR monitor) {
    std::wstring key = ChoiceKey(monitor);
    LayoutChoice c{key};
    auto it = std::find_if(g_layoutChoices.begin(), g_layoutChoices.end(), [&](LayoutChoice const& x) { return x.key == key; });
    if (it != g_layoutChoices.end()) {
        c = *it;
        g_layoutChoices.erase(it);
    }
    g_layoutChoices.push_back(c);
    return g_layoutChoices.back();
}

void SetLayoutChoice(HMONITOR monitor, TileLayout layout) {
    ChoiceFor(monitor).layout = layout;
    SaveLayoutChoices();
}

// The main window's share (percent) on a screen.
int RatioFor(HMONITOR monitor) {
    LayoutChoice const* c = FindLayoutChoice(monitor);
    return std::clamp(g_tiling.ratio + (c ? c->ratioStep : 0), 10, 90);
}

TileLayout LayoutFor(HMONITOR monitor) {
    if (LayoutChoice const* c = FindLayoutChoice(monitor); c && c->layout != TileLayout::Same) return c->layout;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    if (mi.dwFlags & MONITORINFOF_PRIMARY) return g_tiling.layout;
    return g_tiling.otherLayout == TileLayout::Same ? g_tiling.layout : g_tiling.otherLayout;
}

// The layout for n windows: the layout rules' (the last one n reaches),
// unless the screen's layout was picked by hand or it isn't tiled.
TileLayout LayoutFor(HMONITOR monitor, int n) {
    TileLayout layout = LayoutFor(monitor);
    if (layout == TileLayout::Off || g_tiling.rules.empty()) return layout;
    if (LayoutChoice const* c = FindLayoutChoice(monitor); c && c->layout != TileLayout::Same) return layout;
    for (auto const& [count, ruled] : g_tiling.rules)
        if (n >= count) layout = ruled;
    return layout;
}

////////////////////////////////////////////////////////////////////////////////
// Tiling

// A program running as administrator places its own window, through the
// suite's helper in it. 1: placed; 0: it wouldn't shrink; -1: no helper.
int PlaceThroughAgent(HWND h, RECT const& want, bool force) {
    if (!g_tiling.tileElevated) return -1;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    for (HWND agent = nullptr; (agent = FindWindowExW(HWND_MESSAGE, agent, kTileAgentClass, nullptr));) {
        DWORD agentPid = 0;
        if (!GetWindowThreadProcessId(agent, &agentPid) || agentPid != pid) continue;
        TilePlaceRequest request{(uint64_t)(ULONG_PTR)h, want, force ? 1 : 0};
        COPYDATASTRUCT data{kTilePlaceCopyId, sizeof(request), &request};
        DWORD_PTR result = 0;
        if (!SendMessageTimeoutW(agent, WM_COPYDATA, 0, (LPARAM)&data, SMTO_ABORTIFHUNG, 500, &result) || !result)
            return -1;
        return result == 2 ? 1 : 0;
    }
    return -1;
}

// A window on its way to a place (moved over time: below).
struct WindowMove {
    HWND h = nullptr;
    RECT from{}, to{}, at{};  // its frame when it set off, where it's going, and where it was last put
    ULONGLONG start = 0;      // 0: sets off at the next step, with the rest of its layout
    bool tiled = false;       // to a tile (placed as the tiling does), else to a place (MoveFrameTo)
    bool sized = true;        // resized on the way too (not for a window that can't be)
};
std::vector<WindowMove> g_windowMoves;
constexpr UINT_PTR kTimerWindowMoves = 7;

// Placed at once: any move it was making let go, so it doesn't carry on.
void ForgetWindowMove(HWND h) {
    std::erase_if(g_windowMoves, [h](WindowMove const& m) { return m.h == h; });
}

// Tiles a window wouldn't shrink to (its own minimum size: a window taller than its tile, say), kept so the next
// tiling leaves it where it settled instead of setting off for the same tile again: it bounced, over and over.
std::vector<std::pair<HWND, RECT>> g_refusedTiles;

bool RefusedTile(HWND h, RECT const& want) {
    std::erase_if(g_refusedTiles, [](auto const& r) { return !IsWindow(r.first); });
    for (auto const& [w, r] : g_refusedTiles)
        if (w == h) return EqualRect(&r, &want) != 0;
    return false;
}

void NoteRefusedTile(HWND h, RECT const& want, bool refused) {
    std::erase_if(g_refusedTiles, [h](auto const& r) { return r.first == h; });
    if (refused) g_refusedTiles.push_back({h, want});
}

// Puts a window's visible frame on a rect. False: it won't shrink to it and
// is to be left floating.
bool PlaceFrame(HWND h, RECT const& want) {
    ForgetWindowMove(h);
    bool force = g_tiling.fit == kFitForce;
    if (IsZoomed(h)) ShowWindow(h, SW_SHOWNOACTIVATE);
    RECT window{}, frame = FrameOf(h);
    GetWindowRect(h, &window);
    int il = frame.left - window.left, it = frame.top - window.top;
    int ir = window.right - frame.right, ib = window.bottom - frame.bottom;
    int x = want.left - il, y = want.top - it;
    int cx = (want.right - want.left) + il + ir, cy = (want.bottom - want.top) + it + ib;
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
    if (!SetWindowPos(h, nullptr, x, y, cx, cy, flags)) {
        // Refused outright: a program running as administrator.
        if (GetLastError() == ERROR_ACCESS_DENIED) {
            int placed = PlaceThroughAgent(h, want, force);
            return placed != 0 || g_tiling.fit != kFitFloat;
        }
        return true;
    }
    // Where it went, from its window rect less the borders measured above: DWM's frame bounds can still be the
    // old ones for a moment after a resize, and a window slow to draw (Notepad++) looked too big for its tile.
    RECT placed{};
    GetWindowRect(h, &placed);
    RECT now{placed.left + il, placed.top + it, placed.right - ir, placed.bottom - ib};
    bool fits = EqualRect(&now, &want) != 0;
    // Bigger than asked: its own minimum size, or the size it was snapped at.
    bool bigger = !fits && (now.right - now.left > want.right - want.left || now.bottom - now.top > want.bottom - want.top);
    NoteRefusedTile(h, want, bigger);
    if (fits) return true;
    if (bigger && g_tiling.fit == kFitFloat) return false;
    if (bigger && g_tiling.fit == kFitOverlap) return true;
    // Placed again without being asked.
    SetWindowPos(h, nullptr, x, y, cx, cy, flags | SWP_NOSENDCHANGING);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Windows moved over time, when the motion settings have a "windowsMove" line
// (unset, they jump to their places as before; off, too). The windows of one
// layout change set off together, on a timer that runs only while one moves,
// and each one's last step is the placing it would have had at once.

void TileMonitor(HMONITOR monitor, bool byPosition);
void MoveFrameTo(HWND h, RECT const& want, bool animate);

// A window set off toward a frame. False: it's to be placed at once (no
// "windowsMove" line, or it's off).
bool StartWindowMove(HWND h, RECT const& want, bool tiled) {
    auto it = std::find_if(g_windowMoves.begin(), g_windowMoves.end(), [h](WindowMove const& m) { return m.h == h; });
    motion::Anim const& line = g_tiling.move;
    if (line.set && motion::Ms(line) > 0 && g_wnd) {
        if (it != g_windowMoves.end() && EqualRect(&it->to, &want)) {
            it->tiled = tiled;  // (on its way there already)
            return true;
        }
        if (IsZoomed(h) || IsIconic(h)) ShowWindow(h, SW_SHOWNOACTIVATE);
        RECT now = FrameOf(h);
        if (!EqualRect(&now, &want)) {
            if (it == g_windowMoves.end()) {
                if (g_windowMoves.empty()) SetTimer(g_wnd, kTimerWindowMoves, 10, nullptr);
                it = g_windowMoves.insert(g_windowMoves.end(), WindowMove{h});
            }
            // A new place for a window on its way sets it off again from where it is.
            it->from = it->at = now, it->to = want, it->start = 0, it->tiled = tiled;
            it->sized = tiled || (GetWindowLongW(h, GWL_STYLE) & WS_THICKFRAME);
            return true;
        }
    }
    return false;
}

// Puts a tiled window on its tile: over time if set, else at once. False:
// it won't shrink to it and is to be left floating (moved over time, that's
// found at the end, and the screen is tiled again then).
bool TilePlace(HWND h, RECT const& want) {
    if (RefusedTile(h, want) && !IsZoomed(h) && !IsIconic(h)) return true;  // (where it settled last time: left there)
    return StartWindowMove(h, want, true) || PlaceFrame(h, want);
}

// A frame partway between two (t past 1 or under 0 for a curve that overshoots).
RECT FrameBetween(RECT const& a, RECT const& b, double t) {
    auto at = [t](LONG x, LONG y) { return (LONG)std::lround(x + (y - x) * t); };
    RECT r{at(a.left, b.left), at(a.top, b.top), at(a.right, b.right), at(a.bottom, b.bottom)};
    r.right = std::max(r.right, r.left + 1), r.bottom = std::max(r.bottom, r.top + 1);
    return r;
}

// One step: the window's visible frame put on a rect, its invisible borders
// measured each time (they change with the screen's DPI on the way). False
// if Windows refused (a program running as administrator).
bool StepFrame(WindowMove const& m, RECT const& r) {
    RECT window{}, frame = FrameOf(m.h);
    if (!GetWindowRect(m.h, &window)) return true;
    int il = frame.left - window.left, it = frame.top - window.top;
    int ir = window.right - frame.right, ib = window.bottom - frame.bottom;
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | (m.sized ? 0 : SWP_NOSIZE);
    return SetWindowPos(m.h, nullptr, r.left - il, r.top - it, (r.right - r.left) + il + ir, (r.bottom - r.top) + it + ib,
                        flags) ||
           GetLastError() != ERROR_ACCESS_DENIED;
}

// Its last step: placed as it would have been at once. A tiled window that
// won't shrink to its tile is left floating, and its screen given back.
void LandWindowMove(WindowMove const& m, std::vector<HMONITOR>* refit) {
    if (!IsWindow(m.h)) return;
    if (!m.tiled) {
        MoveFrameTo(m.h, m.to, false);
        return;
    }
    if (PlaceFrame(m.h, m.to) || IsFloating(m.h)) return;
    g_floating.push_back(m.h);
    if (!refit) return;
    HMONITOR monitor = MonitorFromWindow(m.h, MONITOR_DEFAULTTONEAREST);
    if (std::find(refit->begin(), refit->end(), monitor) == refit->end()) refit->push_back(monitor);
}

// Every window moving a step on (the timer, while any moves).
void WindowMovesTimer(HWND wnd) {
    motion::Anim const& line = g_tiling.move;
    double length = line.set ? motion::Ms(line) : 0;
    ULONGLONG now = GetTickCount64();
    std::vector<WindowMove> landed;
    for (size_t i = 0; i < g_windowMoves.size();) {
        WindowMove& m = g_windowMoves[i];
        // Closed, minimised, maximised, taken by hand or not answering: let go where it is.
        if (!IsWindow(m.h) || IsIconic(m.h) || IsZoomed(m.h) || m.h == g_moving || IsHungAppWindow(m.h)) {
            g_windowMoves.erase(g_windowMoves.begin() + i);
            continue;
        }
        if (!m.start) m.start = now;
        double t = length > 0 ? (double)(now - m.start) / length : 1;
        RECT r = t < 1 ? FrameBetween(m.from, m.to, motion::Ease(line, t)) : m.to;
        // At the end, or refused (as administrator: placed through the helper at once).
        if (t >= 1 || (!EqualRect(&r, &m.at) && !StepFrame(m, r))) {
            landed.push_back(m);
            g_windowMoves.erase(g_windowMoves.begin() + i);
            continue;
        }
        m.at = r;
        i++;
    }
    std::vector<HMONITOR> refit;
    for (auto const& m : landed) LandWindowMove(m, &refit);
    for (HMONITOR monitor : refit) TileMonitor(monitor, false);
    if (g_windowMoves.empty()) KillTimer(wnd, kTimerWindowMoves);
}

// The switcher closing: every window still moving put where it was going.
void EndWindowMoves() {
    std::vector<WindowMove> moves;
    moves.swap(g_windowMoves);
    if (g_wnd) KillTimer(g_wnd, kTimerWindowMoves);
    for (auto const& m : moves)
        if (IsWindow(m.h) && !IsHungAppWindow(m.h)) LandWindowMove(m, nullptr);
}

struct TileScan {
    std::vector<std::pair<HWND, HMONITOR>> windows;
};

std::vector<std::pair<HWND, HMONITOR>> ScanTileable() {
    TileScan scan;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (Tileable(h))
                ((TileScan*)param)->windows.push_back({h, MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)});
            return TRUE;
        },
        (LPARAM)&scan);
    return scan.windows;
}

std::vector<HWND> FoldGroups(std::vector<HWND> const& windows);

// The windows that have just opened, put where tiling.newWindow says
// (NewWindowAt, in tiling-geometry.cpp, which the order check covers).
void InsertFresh(std::vector<HWND>* ordered, std::vector<HWND> const& fresh) {
    if (fresh.empty()) return;
    size_t at = NewWindowAt(*ordered, fresh, g_tiling.newWindow, g_focused, g_focusedBefore);
    ordered->insert(ordered->begin() + at, fresh.begin(), fresh.end());
}

// A window against a list of programs, classes or titles (commas between, * and
// ? matching any text): what the Slice layout's strip and the ScreenSplit
// regions name their windows with.
bool MatchesWindow(std::wstring const& list, HWND h) {
    if (list.empty()) return false;
    std::wstring program = ProgramOf(h);
    if (MatchesList(list, program.substr(program.find_last_of(L'\\') + 1))) return true;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    return MatchesList(list, cls) || MatchesList(list, Title(h));
}

// The windows of each ScreenSplit region, in the regions' own order: a window
// goes in the first region that names it, and the rest in the first region that
// names none (the last region if every one of them names something).
std::vector<std::vector<HWND>> SplitGroups(std::vector<HWND> const& windows) {
    std::vector<std::vector<HWND>> groups(g_tiling.regions.size());
    if (groups.empty()) return groups;
    size_t spare = groups.size() - 1;
    for (size_t r = 0; r < g_tiling.regions.size(); r++)
        if (g_tiling.regions[r].match.empty()) {
            spare = r;
            break;
        }
    for (HWND h : windows) {
        size_t to = spare;
        for (size_t r = 0; r < g_tiling.regions.size(); r++)
            if (MatchesWindow(g_tiling.regions[r].match, h)) {
                to = r;
                break;
            }
        groups[to].push_back(h);
    }
    return groups;
}

// A screen's tiled windows in their order: the known ones as they were, new
// ones where tiling.newWindow says (the end unless it's changed), sorted among
// themselves by where they are. byPosition: all of them by where they are.
std::vector<HWND> WindowsOn(HMONITOR monitor, bool byPosition) {
    std::erase_if(g_tileOrder, [](HWND h) { return !IsWindow(h); });
    std::vector<HWND> here, fresh;
    for (auto const& [h, m] : ScanTileable())
        if (m == monitor) here.push_back(h);
    std::vector<HWND> ordered;
    if (!byPosition)
        for (HWND h : g_tileOrder)
            if (std::find(here.begin(), here.end(), h) != here.end()) ordered.push_back(h);
    for (HWND h : here)
        if (std::find(ordered.begin(), ordered.end(), h) == ordered.end()) fresh.push_back(h);
    bool vertical = LayoutFor(monitor, (int)here.size()) == TileLayout::Rows;
    std::sort(fresh.begin(), fresh.end(), [vertical](HWND a, HWND b) {
        RECT ra = FrameOf(a), rb = FrameOf(b);
        int ax = (ra.left + ra.right) / 2, ay = (ra.top + ra.bottom) / 2;
        int bx = (rb.left + rb.right) / 2, by = (rb.top + rb.bottom) / 2;
        return vertical ? std::tie(ay, ax) < std::tie(by, bx) : std::tie(ax, ay) < std::tie(bx, by);
    });
    InsertFresh(&ordered, fresh);
    // Slice: the window the strip is for goes first, because the strip is the
    // layout's first place.
    if (!g_tiling.sliceWindow.empty() && LayoutFor(monitor, (int)here.size()) == TileLayout::Slice)
        for (size_t i = 1; i < ordered.size(); i++)
            if (MatchesWindow(g_tiling.sliceWindow, ordered[i])) {
                std::rotate(ordered.begin(), ordered.begin() + i, ordered.begin() + i + 1);
                break;
            }
    // ScreenSplit: region by region, so a window and its place in the layout
    // are the same number along (RectsFor groups them the same way).
    if (!g_tiling.regions.empty() && LayoutFor(monitor, (int)here.size()) == TileLayout::ScreenSplit) {
        std::vector<HWND> byRegion;
        for (auto const& group : SplitGroups(ordered))
            for (HWND h : group) byRegion.push_back(h);
        ordered = std::move(byRegion);
    }
    // The order kept: this screen's, after the others'.
    std::erase_if(g_tileOrder, [&](HWND h) { return std::find(ordered.begin(), ordered.end(), h) != ordered.end(); });
    g_tileOrder.insert(g_tileOrder.end(), ordered.begin(), ordered.end());
    return FoldGroups(ordered);  // (a group of windows takes one tile: tiling-groups.cpp)
}

// A layout of the user's own: the windows handed to the script in the Layouts
// folder (layout-scripts.cpp), and what it answered with. Empty when there is
// no script, when it stopped, or when it answered with something else, and
// empty in the Windhawk suite, which has no Luau in it.
std::vector<RECT> ScriptRects(RECT box, std::vector<HWND> const& windows, int gap, int percent) {
#ifdef TD_PREFIX
    if (g_tiling.script.empty() || windows.empty()) return {};
    td::LayoutScriptAsk ask;
    ask.script = g_tiling.script;
    ask.options = g_tiling.scriptOptions;
    ask.screen = box;
    ask.gap = gap;
    ask.ratio = percent;
    HWND front = GetForegroundWindow();
    for (HWND h : windows) {
        td::LayoutScriptWindow w;
        w.rect = FrameOf(h);
        std::wstring program = ProgramOf(h);
        w.program = program.substr(program.find_last_of(L'\\') + 1);
        WCHAR cls[64] = L"";
        GetClassNameW(h, cls, ARRAYSIZE(cls));
        w.cls = cls;
        w.title = Title(h);
        w.focused = h == front;
        ask.windows.push_back(std::move(w));
    }
    return td::LayoutScriptRects(ask);
#else
    (void)box, (void)windows, (void)gap, (void)percent;
    return {};
#endif
}

// A layout's places for these windows in this box. The same as LayoutRects,
// except that a script layout is handed the windows themselves, and columns
// stand in for a script that didn't answer.
std::vector<RECT> RectsIn(TileLayout layout, RECT box, std::vector<HWND> const& windows, int gap, int percent) {
    if (layout == TileLayout::Script) {
        std::vector<RECT> out = ScriptRects(box, windows, gap, percent);
        if (!out.empty()) return out;
        layout = TileLayout::Columns;
    }
    return LayoutRects(layout, box, (int)windows.size(), gap, percent, g_tiling.slice);
}

std::vector<RECT> RectsFor(HMONITOR monitor, int n) {
    UINT dpi = MonitorDpi(monitor);
    return LayoutRects(LayoutFor(monitor, n), TileArea(monitor), n, MulDiv(g_tiling.gapInner, dpi, 96), RatioFor(monitor),
                       g_tiling.slice);
}

// With the windows themselves, which is what ScreenSplit needs: each region
// lays out its own windows in its own share of the screen, in the order
// WindowsOn put them in. Every other layout only needs the count.
std::vector<RECT> RectsFor(HMONITOR monitor, std::vector<HWND> const& windows) {
    int n = (int)windows.size();
    TileLayout layout = LayoutFor(monitor, n);
    UINT dpi = MonitorDpi(monitor);
    int gap = MulDiv(g_tiling.gapInner, dpi, 96), percent = RatioFor(monitor);
    RECT area = TileArea(monitor);
    if (layout != TileLayout::ScreenSplit || g_tiling.regions.empty())
        return RectsIn(layout, area, windows, gap, percent);
    std::vector<std::vector<HWND>> groups = SplitGroups(windows);
    std::vector<RECT> out;
    for (size_t r = 0; r < groups.size(); r++) {
        if (groups[r].empty()) continue;
        TilingSettings::Region const& region = g_tiling.regions[r];
        TileLayout inside = region.layout == TileLayout::ScreenSplit || region.layout == TileLayout::Off ||
                                    region.layout == TileLayout::Same
                                ? TileLayout::Columns
                                : region.layout;
        RECT box = RegionArea(area, region.l, region.t, region.w, region.h, gap);
        for (RECT const& rect : RectsIn(inside, box, groups[r], gap, percent)) out.push_back(rect);
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Places kept for windows resized by hand

void LoadKeptPlaces() {
    g_keptPlaces.clear();
    std::wstring text(16384, L'\0');
    Wh_GetStringValue(kKeptPlacesValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring_view item = rest.substr(0, end);
        size_t eq = item.rfind(L'=');
        RECT r;
        if (eq != std::wstring_view::npos &&
            swscanf(std::wstring(item.substr(eq + 1)).c_str(), L"%ld,%ld,%ld,%ld", &r.left, &r.top, &r.right, &r.bottom) == 4)
            g_keptPlaces.push_back({std::wstring(item.substr(0, eq)), r});
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SaveKeptPlaces() {
    if (g_keptPlaces.size() > kMostKeptPlaces)
        g_keptPlaces.erase(g_keptPlaces.begin(), g_keptPlaces.end() - kMostKeptPlaces);
    std::wstring text;
    for (auto const& [key, r] : g_keptPlaces)
        text += key + L"=" + std::to_wstring(r.left) + L"," + std::to_wstring(r.top) + L"," + std::to_wstring(r.right) +
                L"," + std::to_wstring(r.bottom) + L";";
    Wh_SetStringValue(kKeptPlacesValue, text.c_str());
}

// A program's key: its file name and the window's class, without the ; and =
// the saved list uses.
std::wstring KeptKey(HWND h) {
    std::wstring program = ProgramOf(h);
    WCHAR cls[128] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    std::wstring key = program.substr(program.find_last_of(L'\\') + 1) + L"|" + cls;
    CharLowerBuffW(key.data(), (DWORD)key.size());
    std::replace_if(key.begin(), key.end(), [](WCHAR c) { return c == L';' || c == L'='; }, L'_');
    return key;
}

bool IsKept(HWND h) {
    return std::any_of(g_keptWindows.begin(), g_keptWindows.end(), [h](auto const& w) { return w.first == h; });
}

// A window left floating where it is, its program's place kept.
void KeepPlace(HWND h) {
    std::wstring key = KeptKey(h);
    if (!IsFloating(h)) g_floating.push_back(h);
    std::erase_if(g_keptWindows, [&](auto const& w) { return w.first == h || w.second == key || !IsWindow(w.first); });
    g_keptWindows.push_back({h, key});
    std::erase_if(g_keptPlaces, [&](KeptPlace const& p) { return p.key == key; });
    g_keptPlaces.push_back({key, FrameOf(h)});
    SaveKeptPlaces();
}

// Put back in the tiling (the tiling key and Space): its program's place is
// let go too.
void ForgetPlace(HWND h) {
    auto it = std::find_if(g_keptWindows.begin(), g_keptWindows.end(), [h](auto const& w) { return w.first == h; });
    if (it == g_keptWindows.end()) return;
    std::wstring key = it->second;
    g_keptWindows.erase(it);
    std::erase_if(g_keptPlaces, [&](KeptPlace const& p) { return p.key == key; });
    SaveKeptPlaces();
}

// The end of a drag or resize by hand. A tiled window that was resized, or
// one already kept, stays where it was put. True when it did.
bool MovedByHand(HWND h) {
    RECT now = FrameOf(h);
    bool resized = now.right - now.left != g_movingFrame.right - g_movingFrame.left ||
                   now.bottom - now.top != g_movingFrame.bottom - g_movingFrame.top;
    bool tiled = std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end() && !IsFloating(h);
    if (!IsKept(h) && !(resized && tiled)) return false;
    KeepPlace(h);
    return true;
}

// Windows not seen before whose program has a kept place (and no window
// holding it): put there, floating.
void PlaceKeptWindows() {
    if (!g_tiling.keepResized || g_keptPlaces.empty()) return;
    std::erase_if(g_keptWindows, [](auto const& w) { return !IsWindow(w.first); });
    for (auto const& [h, monitor] : ScanTileable()) {
        if (std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end()) continue;
        std::wstring key = KeptKey(h);
        auto place = std::find_if(g_keptPlaces.begin(), g_keptPlaces.end(), [&](KeptPlace const& p) { return p.key == key; });
        if (place == g_keptPlaces.end()) continue;
        if (std::any_of(g_keptWindows.begin(), g_keptWindows.end(), [&](auto const& w) { return w.second == key; })) continue;
        if (!MonitorFromRect(&place->frame, MONITOR_DEFAULTTONULL)) continue;  // its screen is gone
        PlaceFrame(h, place->frame);
        g_floating.push_back(h);
        g_keptWindows.push_back({h, key});
    }
}

bool PlaceGroupTile(HWND h, RECT const& tile);

void TileMonitor(HMONITOR monitor, bool byPosition) {
    if (!monitor || LayoutFor(monitor) == TileLayout::Off) return;
    // A window that won't fit is left floating and the rest tiled again
    // without it (a few times at most).
    for (int pass = 0; pass < 4; pass++) {
        std::vector<HWND> windows = WindowsOn(monitor, byPosition && pass == 0);
        std::vector<RECT> rects = RectsFor(monitor, windows);
        bool again = false;
        for (size_t i = 0; i < windows.size() && i < rects.size(); i++) {
            if (PlaceGroupTile(windows[i], rects[i])) continue;
            if (!TilePlace(windows[i], rects[i])) {
                g_floating.push_back(windows[i]);
                again = true;
            }
        }
        if (!again) break;
    }
}

void TileAll() {
    PlaceKeptWindows();
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM) -> BOOL {
            TileMonitor(monitor, false);
            return TRUE;
        },
        0);
}

HMONITOR ActiveMonitor() {
    if (HWND fg = GetForegroundWindow()) return MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST);
    POINT pt;
    GetCursorPos(&pt);
    return MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
}

// The tiled window next to one, in a direction: the nearest whose middle lies
// that way.
int Neighbour(std::vector<RECT> const& rects, int from, int direction) {
    if (from < 0 || from >= (int)rects.size()) return -1;
    auto mid = [](RECT const& r) { return POINT{(r.left + r.right) / 2, (r.top + r.bottom) / 2}; };
    POINT a = mid(rects[from]);
    int best = -1;
    long long bestDistance = 0;
    for (int i = 0; i < (int)rects.size(); i++) {
        if (i == from) continue;
        POINT b = mid(rects[i]);
        int dx = b.x - a.x, dy = b.y - a.y;
        bool that = direction == kLeft ? dx < 0 && std::abs(dx) >= std::abs(dy) / 2
                  : direction == kRight ? dx > 0 && std::abs(dx) >= std::abs(dy) / 2
                  : direction == kUp    ? dy < 0 && std::abs(dy) >= std::abs(dx) / 2
                                        : dy > 0 && std::abs(dy) >= std::abs(dx) / 2;
        if (!that) continue;
        long long distance = (long long)dx * dx + (long long)dy * dy;
        if (best < 0 || distance < bestDistance) best = i, bestDistance = distance;
    }
    return best;
}

void SwapInOrder(HWND a, HWND b) {
    auto ia = std::find(g_tileOrder.begin(), g_tileOrder.end(), a);
    auto ib = std::find(g_tileOrder.begin(), g_tileOrder.end(), b);
    if (ia != g_tileOrder.end() && ib != g_tileOrder.end()) std::iter_swap(ia, ib);
}

void TileCommandRun(WPARAM command, LPARAM arg) {
    HWND fg = GetForegroundWindow();
    HMONITOR monitor = ActiveMonitor();
    switch (command) {
        case kTileNow:
            PlaceKeptWindows();
            TileMonitor(monitor, true);
            break;
        case kTileMove:
        case kTileFocus: {
            std::vector<HWND> windows = WindowsOn(monitor, false);
            std::vector<RECT> rects = RectsFor(monitor, windows);
            int n = (int)windows.size();
            int from = (int)(std::find(windows.begin(), windows.end(), fg) - windows.begin());
            int to = -1;
            if (LayoutFor(monitor, n) == TileLayout::Layers && from < n) {
                // Only the windows of its own layer (the others lie under or over them).
                int first = from / 3 * 3, count = std::min(3, n - first);
                std::vector<RECT> layer(rects.begin() + first, rects.begin() + first + count);
                int along = Neighbour(layer, from - first, (int)arg);
                to = along < 0 ? -1 : first + along;
            } else {
                to = Neighbour(rects, from, (int)arg);
            }
            // Monocle: every window has the whole screen, so none lies any
            // way of another. Left and up go to the one before, right and
            // down to the one after, round and round.
            if (LayoutFor(monitor, n) == TileLayout::Monocle && from < n && n > 1)
                to = (from + (arg == kLeft || arg == kUp ? n - 1 : 1)) % n;
            if (to < 0) break;
            if (command == kTileFocus) {
                Activate(windows[to]);
            } else {
                SwapInOrder(windows[from], windows[to]);
                TileMonitor(monitor, false);
            }
            break;
        }
        case kTileFloat:
            if (!fg) break;
            if (IsFloating(fg)) {
                std::erase(g_floating, fg);
                ForgetPlace(fg);
            } else {
                g_floating.push_back(fg);
            }
            TileMonitor(monitor, false);
            break;
        case kTileLayout: {
            TileLayout now = LayoutFor(monitor, (int)WindowsOn(monitor, false).size());
            auto it = std::find(std::begin(kLayoutCycle), std::end(kLayoutCycle), now);
            TileLayout next = kLayoutCycle[0];
            // The cycle skips a layout there is nothing to lay out with: the
            // split screen with no regions written, a script with no file
            // named. Both would look like columns and leave no way back.
            for (size_t step = 0; step < std::size(kLayoutCycle); step++) {
                size_t at = it == std::end(kLayoutCycle) ? step
                                                         : (size_t)(it - std::begin(kLayoutCycle)) + 1 + step;
                next = kLayoutCycle[at % std::size(kLayoutCycle)];
                bool nothingToUse = (next == TileLayout::ScreenSplit && g_tiling.regions.empty()) ||
                                    (next == TileLayout::Script && g_tiling.script.empty());
                if (!nothingToUse) break;
            }
            SetLayoutChoice(monitor, next);
            TileMonitor(monitor, false);
            break;
        }
        case kTileRatio: {
            LayoutChoice& c = ChoiceFor(monitor);
            c.ratioStep = std::clamp(c.ratioStep + (int)arg, 10 - g_tiling.ratio, 90 - g_tiling.ratio);
            SaveLayoutChoices();
            TileMonitor(monitor, false);
            break;
        }
        case kTileLayouts:
            OpenLayoutsWindow();
            break;
        case kTileApplyLayout:
            ApplyLayoutNumber((int)arg);
            break;
        case kTileAuto:
            g_autoTile = !g_autoTile;
            if (g_autoTile) TileAll();
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Automatic tiling: windows opening, closing, minimising and being dragged.

// A dragged tiled window dropped on another's place swaps with it.
void DropTiled(HWND h) {
    if (std::find(g_tileOrder.begin(), g_tileOrder.end(), h) == g_tileOrder.end()) return;
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    std::vector<HWND> windows = WindowsOn(monitor, false);
    std::vector<RECT> rects = RectsFor(monitor, windows);
    // The place under the pointer; where places lie over each other (Layers), the window seen there.
    std::vector<HWND> under;
    for (size_t i = 0; i < rects.size() && i < windows.size(); i++)
        if (windows[i] != h && PtInRect(&rects[i], pt)) under.push_back(windows[i]);
    HWND target = under.size() == 1 ? under[0] : nullptr;
    for (HWND z = GetTopWindow(nullptr); !target && z && under.size() > 1; z = GetWindow(z, GW_HWNDNEXT))
        if (std::find(under.begin(), under.end(), z) != under.end()) target = z;
    if (target) SwapInOrder(h, target);
}

// Where windows go when they open and close (tiling-layouts.cpp).
void PlacesEvent(DWORD event, HWND h);
void TrackOpenWindows();

////////////////////////////////////////////////////////////////////////////////
// The focused window's border: Windows 11 draws a window's thin border in a
// colour asked of it (DWMWA_BORDER_COLOR), so the window in front wears the
// accent and the one before is given back Windows' own border.

constexpr DWORD kDwmBorderColor = 34;               // DWMWA_BORDER_COLOR
constexpr COLORREF kDwmColorDefault = 0xFFFFFFFF;   // DWMWA_COLOR_DEFAULT
HWND g_bordered = nullptr;                          // the window wearing it

COLORREF FocusColor() {
    if (!g_tiling.focusAccent) return g_tiling.focusColor;
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, nullptr,
                     &abgr, &size) == ERROR_SUCCESS)
        return abgr & 0xFFFFFF;
    DWORD argb = 0;
    BOOL opaque = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&argb, &opaque))) return RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);
    return RGB(0x00, 0x78, 0xD4);
}

void ClearFocusBorder() {
    if (g_bordered && IsWindow(g_bordered)) {
        COLORREF none = kDwmColorDefault;
        DwmSetWindowAttribute(g_bordered, kDwmBorderColor, &none, sizeof(none));
    }
    g_bordered = nullptr;
}

// The window in front now (or none): it wears the border if it's one the
// setting names, and whichever wore it before gives it back.
void UpdateFocusBorder(HWND h) {
    bool wear = false;
    if (h && g_tiling.focusBorder != kBorderNone) {
        if (g_tiling.focusBorder == kBorderAll)
            wear = Placeable(h);
        else
            wear = std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end() && Tileable(h) &&
                   LayoutFor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)) != TileLayout::Off;
    }
    if (wear && h == g_bordered) return;
    ClearFocusBorder();
    if (!wear) return;
    COLORREF color = FocusColor();
    if (SUCCEEDED(DwmSetWindowAttribute(h, kDwmBorderColor, &color, sizeof(color)))) g_bordered = h;
}

void CALLBACK TileEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (!h || object != OBJID_WINDOW || child != CHILDID_SELF) return;
    PlacesEvent(event, h);
    if (event == EVENT_SYSTEM_FOREGROUND) {
        if (h != g_focused) g_focusedBefore = g_focused, g_focused = h;
        UpdateFocusBorder(h);
        return;
    }
    if (event == EVENT_OBJECT_DESTROY && h == g_bordered) g_bordered = nullptr;
    // Drags and resizes are followed with or without automatic tiling, so a
    // window resized by hand isn't put back by the next tiling either.
    if (event == EVENT_SYSTEM_MOVESIZESTART) {
        g_userMoving = true;
        g_moving = h;
        g_movingFrame = FrameOf(h);
        return;
    }
    if (event == EVENT_SYSTEM_MOVESIZEEND) {
        g_userMoving = false;
        bool kept = h == g_moving && g_tiling.keepResized && MovedByHand(h);
        g_moving = nullptr;
        if (!g_autoTile) return;
        if (!kept) DropTiled(h);
    } else if (!g_autoTile || GetAncestor(h, GA_ROOT) != h) {
        return;
    }
    if (HWND wnd = g_wnd) SetTimer(wnd, kTimerTile, 150, nullptr);
}

void StopTileEvents() {
    for (auto& hook : g_tileHooks) {
        if (hook) UnhookWinEvent(hook);
        hook = nullptr;
    }
    ClearFocusBorder();
}

// On the switcher's thread, whose messages carry the events. Only what's
// turned on is followed, so with the tiling, the places and the border all
// off nothing is: no event wakes the suite as windows open, close or move.
void StartTileEvents() {
    StopTileEvents();
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    bool tiling = g_tiling.leaderVk || g_tiling.autoTile;
    if (tiling || g_tiling.rememberAll) {
        g_tileHooks[0] = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MINIMIZEEND, nullptr, TileEventProc, 0, 0, flags);
        g_tileHooks[1] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_HIDE, nullptr, TileEventProc, 0, 0, flags);
        g_tileHooks[2] = SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, TileEventProc, 0, 0, flags);
    }
    // The focus moving: for the border, for the places, whose frames are noted
    // as windows are left (rather than at every move any window makes), and to
    // know which window a new one opens beside.
    bool beside = g_tiling.newWindow == kNewAfterFocused || g_tiling.newWindow == kNewBeforeFocused;
    if (g_tiling.rememberAll || g_tiling.focusBorder != kBorderNone || (tiling && beside))
        g_tileHooks[3] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, TileEventProc, 0, 0, flags);
    if (g_tiling.rememberAll) TrackOpenWindows();
    UpdateFocusBorder(GetForegroundWindow());
}

// The taskbar holds this while a drag has windows pulled back to make room for a drop: tiling then would put
// them straight back. The tiling waits and runs once it's let go.
bool DropRoomOpen() {
    static HANDLE held = CreateEventW(nullptr, TRUE, FALSE, L"Local\\TourneDropRoom");
    return held && WaitForSingleObject(held, 0) == WAIT_OBJECT_0;
}

void TileTimer(HWND wnd) {
    KillTimer(wnd, kTimerTile);
    if (DropRoomOpen()) {
        SetTimer(wnd, kTimerTile, 150, nullptr);
        return;
    }
    if (g_autoTile && !g_userMoving) {
        TileAll();
        UpdateFocusBorder(GetForegroundWindow());
    }
}

////////////////////////////////////////////////////////////////////////////////
// The chords, on the hook thread, held with the tiling key (a key and its
// modifiers, as the user set it; none until then). The key, pressed with
// exactly its modifiers, is kept back until it's clear what it's for: a chord
// key runs a command, anything else (typing) sends the key first after all,
// and letting go of it alone sends it then. Held down past the key repeat
// delay it goes through too, so holding it still works (as games need), and
// in a fullscreen app it's never kept back.

constexpr ULONG_PTR kTileReplayTag = 0x54494C45;  // "TILE": input this sent
bool g_leaderDown = false, g_leaderUsed = false;
DWORD g_leaderScan = 0;
std::vector<DWORD> g_chordKeysDown;  // chord keys whose key up is kept back too

void ReplayKeys(std::initializer_list<std::pair<DWORD, DWORD>> keys, bool up) {
    std::vector<INPUT> in;
    for (auto [vk, scan] : keys) {
        INPUT i{INPUT_KEYBOARD};
        i.ki.wVk = (WORD)vk;
        i.ki.wScan = (WORD)scan;
        i.ki.dwExtraInfo = kTileReplayTag;
        in.push_back(i);
    }
    if (up) {
        INPUT i{INPUT_KEYBOARD};
        i.ki.wVk = (WORD)keys.begin()->first;
        i.ki.wScan = (WORD)keys.begin()->second;
        i.ki.dwFlags = KEYEVENTF_KEYUP;
        i.ki.dwExtraInfo = kTileReplayTag;
        in.push_back(i);
    }
    SendInput((UINT)in.size(), in.data(), sizeof(INPUT));
}

bool ForegroundIsFullscreen() {
    HWND fg = GetForegroundWindow();
    if (!fg || fg == GetShellWindow()) return false;
    RECT r;
    MONITORINFO mi{sizeof(mi)};
    if (!GetWindowRect(fg, &r) || !GetMonitorInfoW(MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST), &mi)) return false;
    RECT const& m = mi.rcMonitor;
    if (r.left > m.left || r.top > m.top || r.right < m.right || r.bottom < m.bottom) return false;
    // Covering the screen isn't enough: a window with a title bar that's simply bigger than the screen (its own
    // minimum size, left floating) isn't a film or a game, and the binds have to keep working over it.
    if (EqualRect(&r, &m)) return true;
    return !(GetWindowLongW(fg, GWL_STYLE) & WS_CAPTION);
}

bool ChordFor(DWORD vk, WPARAM* command, LPARAM* arg) {
    switch (vk) {
        case VK_TAB: *command = kTileNow; return true;
        case VK_LEFT: *command = kTileMove, *arg = kLeft; return true;
        case VK_RIGHT: *command = kTileMove, *arg = kRight; return true;
        case VK_UP: *command = kTileMove, *arg = kUp; return true;
        case VK_DOWN: *command = kTileMove, *arg = kDown; return true;
        case 'H': *command = kTileFocus, *arg = kLeft; return true;
        case 'L': *command = kTileFocus, *arg = kRight; return true;
        case 'K': *command = kTileFocus, *arg = kUp; return true;
        case 'J': *command = kTileFocus, *arg = kDown; return true;
        case VK_SPACE: *command = kTileFloat; return true;
        case 'N': *command = kTileLayout; return true;
        case VK_OEM_4: *command = kTileRatio, *arg = -5; return true;  // [
        case VK_OEM_6: *command = kTileRatio, *arg = 5; return true;   // ]
        case 'A': *command = kTileAuto; return true;
        case 'M': *command = kTileLayouts; return true;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            *command = kTileApplyLayout, *arg = (LPARAM)(vk - '1');
            return true;
    }
    return false;
}

// -1: not the chords' business; otherwise the key is kept back.
int TilingKeyHook(WPARAM wParam, KBDLLHOOKSTRUCT const* k) {
    if (k->dwExtraInfo == kTileReplayTag) return -1;
    bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    DWORD vk = k->vkCode;
    if (!down) {
        if (auto it = std::find(g_chordKeysDown.begin(), g_chordKeysDown.end(), vk); it != g_chordKeysDown.end()) {
            g_chordKeysDown.erase(it);
            return 1;
        }
    }
    DWORD leader = g_leaderVk;
    if (leader && vk == leader) {
        if (down) {
            if (g_leaderDown) {
                // Held: the key repeat. Unless a chord was used, it's the key
                // held, which goes through from here.
                if (!g_leaderUsed) {
                    g_leaderDown = false;
                    ReplayKeys({{leader, g_leaderScan}}, false);
                    return -1;
                }
                return 1;
            }
            // With exactly the modifiers set, no more and no fewer.
            UINT mods = g_leaderMods;
            bool shift = GetAsyncKeyState(VK_SHIFT) < 0, ctrl = GetAsyncKeyState(VK_CONTROL) < 0;
            bool alt = (k->flags & LLKHF_ALTDOWN) != 0, win = GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0;
            bool exact = shift == ((mods & MOD_SHIFT) != 0) && ctrl == ((mods & MOD_CONTROL) != 0) &&
                         alt == ((mods & MOD_ALT) != 0) && win == ((mods & MOD_WIN) != 0);
            if (exact && !ForegroundIsFullscreen()) {
                g_leaderDown = true;
                g_leaderUsed = false;
                g_leaderScan = k->scanCode;
                // Win or Alt let go after a key that was kept back would open
                // Start or the program's menu bar: a key nothing uses is
                // pressed between, as the switcher does.
                if (mods & (MOD_WIN | MOD_ALT)) ReplayKeys({{kDummyKey, 0}}, true);
                return 1;
            }
            return -1;
        }
        if (g_leaderDown) {
            g_leaderDown = false;
            if (!g_leaderUsed) ReplayKeys({{leader, g_leaderScan}}, true);
            return 1;
        }
        return -1;
    }
    if (!g_leaderDown || !down) return -1;
    if (vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT || vk == VK_CONTROL || vk == VK_LCONTROL ||
        vk == VK_RCONTROL || vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU || vk == VK_LWIN || vk == VK_RWIN)
        return -1;
    WPARAM command = 0;
    LPARAM arg = 0;
    if (ChordFor(vk, &command, &arg)) {
        g_leaderUsed = true;
        g_chordKeysDown.push_back(vk);
        if (HWND wnd = g_wnd) PostMessageW(wnd, WM_APP_TILE, command, arg);
        return 1;
    }
    // Typing: the tiling key first, then this key.
    g_leaderDown = false;
    if (g_leaderUsed) return -1;
    ReplayKeys({{leader, g_leaderScan}, {vk, k->scanCode}}, false);
    return 1;
}
