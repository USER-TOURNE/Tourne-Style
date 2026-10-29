////////////////////////////////////////////////////////////////////////////////
// Where windows go ("// @inline tiling-layouts.cpp", after tiling.cpp, in the
// Switcher): every window's place when it closed, given back when one of its
// kind opens, and layouts (the windows' places and each screen's tiling,
// saved under a name), kept in a window of their own (the tiling key and M).

////////////////////////////////////////////////////////////////////////////////
// Places on close

struct SavedPlace {
    std::wstring key;
    RECT frame;
    bool maximized;
};
struct TrackedWindow {
    std::wstring key;
    RECT frame{};
    RECT insets{};  // its invisible resize borders (the window's rect less its frame), as last seen
    bool have = false, maximized = false;
};
std::vector<SavedPlace> g_places;  // newest last
std::unordered_map<HWND, TrackedWindow> g_tracked;
HWND g_placesFront = nullptr;  // the window in front, whose frame is noted when it's left
constexpr size_t kMostPlaces = 256;
constexpr WCHAR kPlacesValue[] = L"switcher.tiling.places";
constexpr UINT_PTR kTimerPlaces = 4;
bool g_placesDirty = false;

// Without the characters the saved lists use between their parts.
std::wstring Sanitized(std::wstring s) {
    std::replace_if(s.begin(), s.end(), [](WCHAR c) { return c == L';' || c == L'=' || c == L'\n' || c == L'\r' || c == 0x1E; }, L'_');
    return s;
}

// A window's kind: its program and class (as for kept places), and for a
// dialog or an owned window its title too, as one program has many.
std::wstring PlaceKey(HWND h, std::wstring const& kept) {
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!GetWindow(h, GW_OWNER) && wcscmp(cls, L"#32770")) return kept;
    std::wstring title = Title(h);
    CharLowerBuffW(title.data(), (DWORD)title.size());
    return kept + L"|" + Sanitized(title);
}

// Any window with a title bar or a resizing border: programs' windows,
// dialogs, tool palettes. Not menus, tips, the shell's own or the suite's.
bool Placeable(HWND h) {
    if (!IsWindowVisible(h) || GetAncestor(h, GA_ROOT) != h) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((style & WS_CHILD) || (ex & WS_EX_NOACTIVATE)) return false;
    if ((style & WS_CAPTION) != WS_CAPTION && !(style & WS_THICKFRAME)) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return false;
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    for (PCWSTR shell : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW", L"#32768"})
        if (!wcscmp(cls, shell)) return false;
    return wcsncmp(cls, L"Tourne", 6) != 0;
}

void LoadPlaces() {
    g_places.clear();
    std::wstring text(65536, L'\0');
    Wh_GetStringValue(kPlacesValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring_view item = rest.substr(0, end);
        size_t eq = item.rfind(L'=');
        RECT r;
        int maximized = 0;
        if (eq != std::wstring_view::npos &&
            swscanf(std::wstring(item.substr(eq + 1)).c_str(), L"%ld,%ld,%ld,%ld,%d", &r.left, &r.top, &r.right, &r.bottom,
                    &maximized) >= 4)
            g_places.push_back({std::wstring(item.substr(0, eq)), r, maximized != 0});
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SavePlaces() {
    g_placesDirty = false;
    if (g_places.size() > kMostPlaces) g_places.erase(g_places.begin(), g_places.end() - kMostPlaces);
    std::wstring text;
    for (auto const& p : g_places) {
        WCHAR numbers[64];
        swprintf_s(numbers, L"=%ld,%ld,%ld,%ld,%d;", p.frame.left, p.frame.top, p.frame.right, p.frame.bottom,
                   p.maximized ? 1 : 0);
        text += p.key + numbers;
    }
    Wh_SetStringValue(kPlacesValue, text.c_str());
}

// Written a little later, once for a burst of windows closing.
void SavePlacesSoon() {
    g_placesDirty = true;
    if (HWND wnd = g_wnd) SetTimer(wnd, kTimerPlaces, 2000, nullptr);
}

void PlacesTimer(HWND wnd) {
    KillTimer(wnd, kTimerPlaces);
    if (g_placesDirty) SavePlaces();
}

// A window's frame as it is, noted when it opens, is left for another,
// minimises, is let go after a drag, or hides (no hook follows every move
// any window makes, which would wake the suite all the time). Minimised or
// maximised, it's the frame it goes back to (and a maximised one is
// maximised again); hidden, the one it had.
void NoteFrame(HWND h) {
    auto it = g_tracked.find(h);
    if (it == g_tracked.end() || !IsWindow(h)) return;
    TrackedWindow& t = it->second;
    RECT r{};
    if (!IsIconic(h) && !IsZoomed(h)) {
        if (!GetWindowRect(h, &r)) return;
        if (IsWindowVisible(h)) {
            RECT frame = FrameOf(h);
            t.insets = {frame.left - r.left, frame.top - r.top, r.right - frame.right, r.bottom - frame.bottom};
        }
        t.maximized = false;
    } else {
        WINDOWPLACEMENT wp{sizeof(wp)};
        if (!GetWindowPlacement(h, &wp)) return;
        r = wp.rcNormalPosition;
        // In workspace coordinates (the work area's, not the screen's),
        // unless it's a tool window.
        if (!(GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) {
            MONITORINFO mi{sizeof(mi)};
            if (GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi))
                OffsetRect(&r, mi.rcWork.left - mi.rcMonitor.left, mi.rcWork.top - mi.rcMonitor.top);
        }
        t.maximized = IsZoomed(h) || (wp.flags & WPF_RESTORETOMAXIMIZED);
    }
    t.frame = {r.left + t.insets.left, r.top + t.insets.top, r.right - t.insets.right, r.bottom - t.insets.bottom};
    t.have = true;
}

// Puts a window's visible frame on a rect: moved and sized, or only moved if
// it can't be resized. Over time when asked to and the motion settings move
// windows (StartWindowMove), else at once.
void MoveFrameTo(HWND h, RECT const& want, bool animate) {
    if (animate && StartWindowMove(h, want, false)) return;
    ForgetWindowMove(h);
    if (IsZoomed(h) || IsIconic(h)) ShowWindow(h, SW_SHOWNOACTIVATE);
    RECT window{}, frame = FrameOf(h);
    GetWindowRect(h, &window);
    int il = frame.left - window.left, it = frame.top - window.top;
    int ir = window.right - frame.right, ib = window.bottom - frame.bottom;
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
    if (!(GetWindowLongW(h, GWL_STYLE) & WS_THICKFRAME)) flags |= SWP_NOSIZE;
    if (!SetWindowPos(h, nullptr, want.left - il, want.top - it, (want.right - want.left) + il + ir,
                      (want.bottom - want.top) + it + ib, flags) &&
        GetLastError() == ERROR_ACCESS_DENIED)
        PlaceThroughAgent(h, want, false);  // a program running as administrator
}

bool WillBeTiled(HWND h) {
    return g_autoTile && Tileable(h) && LayoutFor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)) != TileLayout::Off;
}

// A window opening: back where its kind was when one last closed, unless the
// tiling places it (or a place kept for it by hand does).
void WindowShown(HWND h) {
    if (g_tracked.count(h)) {
        NoteFrame(h);
        return;
    }
    if (!Placeable(h)) return;
    std::wstring kept = KeptKey(h);
    std::wstring key = PlaceKey(h, kept);
    g_tracked[h].key = key;
    bool keptByHand = g_tiling.keepResized &&
                      std::any_of(g_keptPlaces.begin(), g_keptPlaces.end(), [&](KeptPlace const& p) { return p.key == kept; });
    auto place = std::find_if(g_places.begin(), g_places.end(), [&](SavedPlace const& p) { return p.key == key; });
    if (place != g_places.end() && !keptByHand && !WillBeTiled(h) && MonitorFromRect(&place->frame, MONITOR_DEFAULTTONULL)) {
        MoveFrameTo(h, place->frame, false);  // (opening: at once, then maximised and noted)
        if (place->maximized && (GetWindowLongW(h, GWL_STYLE) & WS_MAXIMIZEBOX)) ShowWindow(h, SW_MAXIMIZE);
    }
    NoteFrame(h);
}

// Closed (or hidden, as programs that close to the tray do): its place is
// kept for the next of its kind.
void WindowGone(HWND h, bool destroyed) {
    auto it = g_tracked.find(h);
    if (it == g_tracked.end()) return;
    TrackedWindow const& t = it->second;
    if (t.have) {
        std::erase_if(g_places, [&](SavedPlace const& p) { return p.key == t.key; });
        g_places.push_back({t.key, t.frame, t.maximized});
        SavePlacesSoon();
    }
    if (destroyed) g_tracked.erase(it);
}

// The windows already open, followed from now (not moved).
void TrackOpenWindows() {
    EnumWindows(
        [](HWND h, LPARAM) __stdcall -> BOOL {
            if (!g_tracked.count(h) && Placeable(h)) {
                g_tracked[h].key = PlaceKey(h, KeptKey(h));
                NoteFrame(h);
            }
            return TRUE;
        },
        0);
}

void PlacesEvent(DWORD event, HWND h) {
    if (!g_tiling.rememberAll) return;
    switch (event) {
        case EVENT_OBJECT_SHOW:
        case EVENT_OBJECT_UNCLOAKED:
            WindowShown(h);
            break;
        case EVENT_SYSTEM_FOREGROUND:
            if (g_placesFront && g_placesFront != h) NoteFrame(g_placesFront);
            g_placesFront = h;
            NoteFrame(h);
            break;
        case EVENT_SYSTEM_MOVESIZEEND:
        case EVENT_SYSTEM_MINIMIZESTART:
        case EVENT_SYSTEM_MINIMIZEEND:
            NoteFrame(h);
            break;
        case EVENT_OBJECT_HIDE:
            NoteFrame(h);  // (if it's still there: the event comes a little after)
            WindowGone(h, false);
            break;
        case EVENT_OBJECT_DESTROY:
            if (h == g_placesFront) g_placesFront = nullptr;
            WindowGone(h, true);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Layouts: each window's place and whether it's tiled, and each screen's
// tiling layout, as text that can be edited:
//   screen 0 0 2560 1440 verticalStack
//   tile 6 4 1274 1396 chrome.exe|chrome_widgetwin_1
//   float 300 200 1100 800 notepad.exe|notepad

struct SavedLayout {
    std::wstring name, text;
};
std::vector<SavedLayout> g_layouts;
constexpr WCHAR kLayoutsValue[] = L"switcher.tiling.layouts";

constexpr std::pair<TileLayout, PCWSTR> kLayoutNames[] = {
    {TileLayout::Off, L"off"},
    {TileLayout::Columns, L"columns"},
    {TileLayout::Rows, L"rows"},
    {TileLayout::VerticalStack, L"verticalStack"},
    {TileLayout::RightMainVerticalStack, L"rightMainVerticalStack"},
    {TileLayout::HorizontalStack, L"horizontalStack"},
    {TileLayout::UltrawideVerticalStack, L"ultrawideVerticalStack"},
    {TileLayout::Grid, L"grid"},
    {TileLayout::Bsp, L"bsp"},
    {TileLayout::Spiral, L"spiral"},
    {TileLayout::ThreeColumn, L"threeColumn"},
    {TileLayout::Slice, L"slice"},
    {TileLayout::ScreenSplit, L"screenSplit"},
    {TileLayout::Script, L"script"},
    {TileLayout::Monocle, L"monocle"},
    {TileLayout::Layers, L"layers"},
};

PCWSTR LayoutName(TileLayout layout) {
    for (auto [l, name] : kLayoutNames)
        if (l == layout) return name;
    return L"columns";
}

void LoadLayouts() {
    g_layouts.clear();
    std::wstring text(262144, L'\0');
    Wh_GetStringValue(kLayoutsValue, text.data(), text.size());
    text.resize(wcslen(text.c_str()));
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L'\x1E');
        std::wstring_view item = rest.substr(0, end);
        size_t line = item.find(L'\n');
        if (!item.empty())
            g_layouts.push_back({std::wstring(item.substr(0, line)),
                                 line == std::wstring_view::npos ? L"" : std::wstring(item.substr(line + 1))});
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SaveLayouts() {
    std::wstring text;
    for (auto const& l : g_layouts) {
        if (!text.empty()) text += L'\x1E';
        std::wstring body = l.text;
        std::erase(body, L'\x1E');
        text += Sanitized(l.name) + L"\n" + body;
    }
    Wh_SetStringValue(kLayoutsValue, text.c_str());
}

// The windows open now, as a layout's text: the tiled ones in their order,
// then the rest from the top down.
std::wstring SnapshotLayout() {
    std::wstring text =
        L"# A screen: its edges (left top right bottom) and its tiling layout\r\n"
        L"# (off, columns, rows, verticalStack, rightMainVerticalStack, horizontalStack,\r\n"
        L"# ultrawideVerticalStack, grid, bsp, spiral, threeColumn, slice,\r\n"
        L"# screenSplit, script, monocle,\r\n"
        L"# layers).\r\n"
        L"# A window: tile or float, its edges, then program|class.\r\n";
    struct Screens {
        std::wstring* text;
    } screens{&text};
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM param) -> BOOL {
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(monitor, &mi);
            WCHAR line[160];
            swprintf_s(line, L"screen %ld %ld %ld %ld %ls\r\n", mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right,
                       mi.rcMonitor.bottom, LayoutName(LayoutFor(monitor)));
            *((Screens*)param)->text += line;
            return TRUE;
        },
        (LPARAM)&screens);
    std::vector<HWND> windows;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (Placeable(h) && !IsIconic(h)) ((std::vector<HWND>*)param)->push_back(h);
            return TRUE;
        },
        (LPARAM)&windows);
    auto line = [&](HWND h, bool tiled) {
        RECT r = FrameOf(h);
        WCHAR numbers[80];
        swprintf_s(numbers, L"%ls %ld %ld %ld %ld ", tiled ? L"tile" : L"float", r.left, r.top, r.right, r.bottom);
        text += numbers + KeptKey(h) + L"\r\n";
    };
    std::vector<HWND> tiled;
    for (HWND h : g_tileOrder)
        if (std::find(windows.begin(), windows.end(), h) != windows.end() && Tileable(h) &&
            LayoutFor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST)) != TileLayout::Off)
            tiled.push_back(h);
    for (HWND h : tiled) line(h, true);
    for (HWND h : windows)
        if (std::find(tiled.begin(), tiled.end(), h) == tiled.end()) line(h, false);
    return text;
}

// Puts the open windows where a layout has them: each line takes the first
// window of its program and class not taken yet. False if nothing matched.
bool ApplyLayout(std::wstring const& text) {
    struct Candidate {
        HWND h;
        std::wstring key;
        bool used;
    };
    std::vector<Candidate> candidates;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            if (Placeable(h) || (IsIconic(h) && GetAncestor(h, GA_ROOT) == h && IsWindowVisible(h)))
                ((std::vector<Candidate>*)param)->push_back({h, {}, false});
            return TRUE;
        },
        (LPARAM)&candidates);
    for (auto& c : candidates) c.key = KeptKey(c.h);
    std::vector<HWND> tiledOrder;
    bool any = false;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? text.size() : end + 1;
        while (!line.empty() && iswspace(line.back())) line.pop_back();
        WCHAR kind[16] = L"";
        RECT r{};
        int used = 0;
        if (line.empty() || line[0] == L'#' ||
            swscanf_s(line.c_str(), L"%15s %ld %ld %ld %ld %n", kind, (unsigned)ARRAYSIZE(kind), &r.left, &r.top,
                      &r.right, &r.bottom, &used) < 5 ||
            !used)
            continue;
        std::wstring rest = line.substr(used);
        if (!wcscmp(kind, L"screen")) {
            HMONITOR monitor = MonitorFromRect(&r, MONITOR_DEFAULTTONULL);
            if (!monitor) continue;
            SetLayoutChoice(monitor, ParseLayout(rest, TileLayout::Columns));
            continue;
        }
        bool tile = !wcscmp(kind, L"tile");
        if (!tile && wcscmp(kind, L"float")) continue;
        CharLowerBuffW(rest.data(), (DWORD)rest.size());
        auto c = std::find_if(candidates.begin(), candidates.end(), [&](Candidate const& c) { return !c.used && c.key == rest; });
        if (c == candidates.end()) continue;
        c->used = true;
        any = true;
        if (IsIconic(c->h)) ShowWindow(c->h, SW_SHOWNOACTIVATE);
        if (tile) {
            std::erase(g_floating, c->h);
            ForgetPlace(c->h);
            tiledOrder.push_back(c->h);
        } else {
            if (!IsFloating(c->h)) g_floating.push_back(c->h);
            if (MonitorFromRect(&r, MONITOR_DEFAULTTONULL)) MoveFrameTo(c->h, r, true);
        }
    }
    // The tiled ones first, in the layout's order.
    std::erase_if(g_tileOrder, [&](HWND h) { return std::find(tiledOrder.begin(), tiledOrder.end(), h) != tiledOrder.end(); });
    g_tileOrder.insert(g_tileOrder.begin(), tiledOrder.begin(), tiledOrder.end());
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM) -> BOOL {
            TileMonitor(monitor, false);
            return TRUE;
        },
        0);
    return any;
}

////////////////////////////////////////////////////////////////////////////////
// The layouts window

constexpr WCHAR kLayoutsClass[] = L"TourneTileLayouts_" WH_MOD_ID;
enum LayoutControl : int { kLyList = 201, kLyName, kLyText, kLyNew, kLyApply, kLyCapture, kLySave, kLyDelete, kLyStatus };

struct LayoutsWindow {
    HWND wnd = nullptr;
    COLORREF raised = 0;  // the fields: the ground a little toward the text
    HFONT font = nullptr;
    HBRUSH ground = nullptr, field = nullptr;
    int selected = -1;
} g_ly;

std::wstring ControlText(HWND parent, int id) {
    HWND control = GetDlgItem(parent, id);
    std::wstring text(GetWindowTextLengthW(control), L'\0');
    if (!text.empty()) GetWindowTextW(control, text.data(), (int)text.size() + 1);
    return text;
}

void LayoutsStatus(PCWSTR text) {
    if (g_ly.wnd) SetDlgItemTextW(g_ly.wnd, kLyStatus, text);
}

void FillLayoutsList(int select) {
    HWND list = GetDlgItem(g_ly.wnd, kLyList);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (auto const& l : g_layouts) SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)l.name.c_str());
    g_ly.selected = g_layouts.empty() ? -1 : std::clamp(select, 0, (int)g_layouts.size() - 1);
    SendMessageW(list, LB_SETCURSEL, g_ly.selected, 0);
    if (g_ly.selected >= 0) {
        SetDlgItemTextW(g_ly.wnd, kLyName, g_layouts[g_ly.selected].name.c_str());
        SetDlgItemTextW(g_ly.wnd, kLyText, g_layouts[g_ly.selected].text.c_str());
    } else {
        SetDlgItemTextW(g_ly.wnd, kLyName, L"");
        SetDlgItemTextW(g_ly.wnd, kLyText, L"");
    }
}

std::wstring NewLayoutName() {
    for (int n = (int)g_layouts.size() + 1;; n++) {
        std::wstring name = L"Layout " + std::to_wstring(n);
        if (std::none_of(g_layouts.begin(), g_layouts.end(), [&](SavedLayout const& l) { return l.name == name; })) return name;
    }
}

void LayoutsCommand(HWND wnd, int id, int code) {
    switch (id) {
        case kLyList:
            if (code == LBN_SELCHANGE) {
                FillLayoutsList((int)SendDlgItemMessageW(wnd, kLyList, LB_GETCURSEL, 0, 0));
                LayoutsStatus(L"");
            }
            break;
        case kLyNew:
            g_layouts.push_back({NewLayoutName(), SnapshotLayout()});
            SaveLayouts();
            FillLayoutsList((int)g_layouts.size() - 1);
            SetFocus(GetDlgItem(wnd, kLyName));
            SendDlgItemMessageW(wnd, kLyName, EM_SETSEL, 0, -1);
            LayoutsStatus(L"Saved the windows open now as a new layout. Type a name and press Save to rename it.");
            break;
        case kLyApply:
            LayoutsStatus(ApplyLayout(ControlText(wnd, kLyText)) ? L"Applied." : L"None of its windows are open.");
            break;
        case kLyCapture:
            if (g_ly.selected < 0) break;
            g_layouts[g_ly.selected].text = SnapshotLayout();
            SaveLayouts();
            FillLayoutsList(g_ly.selected);
            LayoutsStatus(L"Updated from the windows open now.");
            break;
        case kLySave: {
            std::wstring name = Sanitized(ControlText(wnd, kLyName));
            if (name.empty()) {
                LayoutsStatus(L"Give it a name first.");
                break;
            }
            if (g_ly.selected < 0) {
                g_layouts.push_back({});
                g_ly.selected = (int)g_layouts.size() - 1;
            }
            g_layouts[g_ly.selected] = {name, ControlText(wnd, kLyText)};
            SaveLayouts();
            FillLayoutsList(g_ly.selected);
            LayoutsStatus(L"Saved.");
            break;
        }
        case kLyDelete: {
            if (g_ly.selected < 0) break;
            std::wstring ask = L"Delete the layout \"" + g_layouts[g_ly.selected].name + L"\"?";
            if (MessageBoxW(wnd, ask.c_str(), L"Window layouts", MB_YESNO | MB_ICONQUESTION) != IDYES) break;
            g_layouts.erase(g_layouts.begin() + g_ly.selected);
            SaveLayouts();
            FillLayoutsList(g_ly.selected);
            LayoutsStatus(L"Deleted.");
            break;
        }
        case IDCANCEL:
            DestroyWindow(wnd);
            break;
    }
}

LRESULT CALLBACK LayoutsWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Look const& look = g_settings.look;
    switch (msg) {
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
            SetTextColor((HDC)wParam, Rgb(look.text));
            SetBkColor((HDC)wParam, g_ly.raised);
            return (LRESULT)g_ly.field;
        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, Rgb(look.text));
            SetBkColor((HDC)wParam, Rgb(look.ground));
            return (LRESULT)g_ly.ground;
        case WM_ERASEBKGND: {
            RECT rc;
            GetClientRect(wnd, &rc);
            FillRect((HDC)wParam, &rc, g_ly.ground);
            return 1;
        }
        case WM_MEASUREITEM: {
            auto* item = (MEASUREITEMSTRUCT*)lParam;
            HDC dc = GetDC(wnd);
            HGDIOBJ old = SelectObject(dc, g_ly.font);
            TEXTMETRICW tm;
            GetTextMetricsW(dc, &tm);
            SelectObject(dc, old);
            ReleaseDC(wnd, dc);
            item->itemHeight = tm.tmHeight + tm.tmHeight / 2;
            return TRUE;
        }
        case WM_DRAWITEM: {
            auto* item = (DRAWITEMSTRUCT*)lParam;
            bool selected = item->itemState & ODS_SELECTED, focused = item->itemState & ODS_FOCUS;
            std::wstring text;
            if (item->CtlType == ODT_LISTBOX) {
                if (item->itemID == (UINT)-1) return TRUE;
                text.resize(SendMessageW(item->hwndItem, LB_GETTEXTLEN, item->itemID, 0));
                SendMessageW(item->hwndItem, LB_GETTEXT, item->itemID, (LPARAM)text.data());
                HBRUSH fill = CreateSolidBrush(selected ? Rgb(look.highlight) : g_ly.raised);
                FillRect(item->hDC, &item->rcItem, fill);
                DeleteObject(fill);
                RECT r = item->rcItem;
                r.left += 6;
                SetBkMode(item->hDC, TRANSPARENT);
                SetTextColor(item->hDC, Rgb(selected ? look.selectedText : look.text));
                HGDIOBJ old = SelectObject(item->hDC, g_ly.font);
                DrawTextW(item->hDC, text.c_str(), -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                SelectObject(item->hDC, old);
                return TRUE;
            }
            text.resize(GetWindowTextLengthW(item->hwndItem));
            GetWindowTextW(item->hwndItem, text.data(), (int)text.size() + 1);
            HBRUSH fill = CreateSolidBrush(selected ? Rgb(look.highlight) : g_ly.raised);
            FillRect(item->hDC, &item->rcItem, fill);
            DeleteObject(fill);
            HBRUSH frame = CreateSolidBrush(Rgb(focused ? look.highlightBorder : look.border));
            FrameRect(item->hDC, &item->rcItem, frame);
            DeleteObject(frame);
            SetBkMode(item->hDC, TRANSPARENT);
            SetTextColor(item->hDC, Rgb(selected ? look.selectedText : look.text));
            HGDIOBJ old = SelectObject(item->hDC, g_ly.font);
            DrawTextW(item->hDC, text.c_str(), -1, &item->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(item->hDC, old);
            return TRUE;
        }
        case WM_COMMAND:
            LayoutsCommand(wnd, LOWORD(wParam), HIWORD(wParam));
            return 0;
        case WM_CLOSE:
            DestroyWindow(wnd);
            return 0;
        case WM_DESTROY:
            if (g_ly.font) DeleteObject(g_ly.font);
            if (g_ly.ground) DeleteObject(g_ly.ground);
            if (g_ly.field) DeleteObject(g_ly.field);
            g_ly = {};
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// For the thread's message loop: Tab and Escape in the layouts window.
bool LayoutsDialogMessage(MSG* msg) {
    return g_ly.wnd && IsDialogMessageW(g_ly.wnd, msg);
}

void OpenLayoutsWindow() {
    if (g_ly.wnd) {
        SetForegroundWindow(g_ly.wnd);
        return;
    }
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&LayoutsWndProc, &instance);
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = LayoutsWndProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kLayoutsClass;
        registered = RegisterClassW(&wc) != 0;
    }
    HMONITOR monitor = ActiveMonitor();
    UINT dpi = MonitorDpi(monitor);
    auto px = [dpi](int v) { return MulDiv(v, dpi, 96); };
    Look const& look = g_settings.look;
    LOGFONTW lf{};
    lf.lfHeight = -MulDiv(std::max(g_settings.fontSize, 9), dpi, 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfQuality = NONANTIALIASED_QUALITY;
    wcsncpy_s(lf.lfFaceName, g_settings.font.c_str(), _TRUNCATE);
    g_ly.font = CreateFontIndirectW(&lf);
    g_ly.ground = CreateSolidBrush(Rgb(look.ground));
    auto toward = [](uint32_t from, uint32_t to, int part) {  // part in 16ths
        auto mix = [&](int shift) { return (((from >> shift) & 0xFF) * (16 - part) + ((to >> shift) & 0xFF) * part) / 16; };
        return RGB(mix(16), mix(8), mix(0));
    };
    g_ly.raised = toward(look.ground, look.text, 2);
    g_ly.field = CreateSolidBrush(g_ly.raised);
    int width = px(820), height = px(500), pad = px(12), listWidth = px(200), row = px(26), button = px(24);
    RECT frame{0, 0, width, height};
    AdjustWindowRectExForDpi(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    int w = frame.right - frame.left, h = frame.bottom - frame.top;
    RECT const& work = mi.rcWork;
    int x = work.left + std::max(0, (int)(work.right - work.left - w) / 2);
    int y = work.top + std::max(0, (int)(work.bottom - work.top - h) / 2);
    g_ly.wnd = CreateWindowExW(WS_EX_CONTROLPARENT, kLayoutsClass, L"Window layouts",
                               WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_POPUP, x, y, w, h, nullptr, nullptr,
                               instance, nullptr);
    if (!g_ly.wnd) return;
    bool dark = (GetRValue(Rgb(look.ground)) + GetGValue(Rgb(look.ground)) + GetBValue(Rgb(look.ground))) < 3 * 128;
    BOOL darkFlag = dark;
    DwmSetWindowAttribute(g_ly.wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkFlag, sizeof(darkFlag));
    COLORREF caption = Rgb(look.ground), captionText = Rgb(look.text), border = Rgb(look.border);
    DwmSetWindowAttribute(g_ly.wnd, 35 /* DWMWA_CAPTION_COLOR */, &caption, sizeof(caption));
    DwmSetWindowAttribute(g_ly.wnd, 36 /* DWMWA_TEXT_COLOR */, &captionText, sizeof(captionText));
    DwmSetWindowAttribute(g_ly.wnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));

    auto add = [&](PCWSTR cls, PCWSTR text, DWORD style, DWORD ex, int id, int left, int top, int right, int bottom) {
        HWND control = CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, left, top, right - left,
                                       bottom - top, g_ly.wnd, (HMENU)(INT_PTR)id, instance, nullptr);
        SendMessageW(control, WM_SETFONT, (WPARAM)g_ly.font, FALSE);
        if (dark) SetWindowTheme(control, L"DarkMode_Explorer", nullptr);
        return control;
    };
    int right = width - pad, textLeft = pad + listWidth + pad;
    int bottomRow = height - pad - button - row;  // the buttons, above the status line
    add(L"STATIC", L"Layouts", SS_LEFT, 0, -1, pad, pad, pad + listWidth, pad + px(18));
    add(L"LISTBOX", L"", WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
        0, kLyList, pad, pad + px(22), pad + listWidth, bottomRow - pad);
    add(L"STATIC", L"Name", SS_LEFT | SS_CENTERIMAGE, 0, -1, textLeft, pad, textLeft + px(50), pad + px(22));
    add(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, 0, kLyName, textLeft + px(54), pad, right, pad + px(22));
    add(L"EDIT", L"", WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN,
        0, kLyText, textLeft, pad + px(30), right, bottomRow - pad);
    struct Button {
        int id;
        PCWSTR text;
        int width;
    } const buttons[] = {{kLyNew, L"Save current as new", 160}, {kLyApply, L"Apply", 70},
                         {kLyCapture, L"Update from windows", 160}, {kLySave, L"Save", 70},
                         {kLyDelete, L"Delete", 70},         {IDCANCEL, L"Close", 70}};
    int left = pad;
    for (auto const& b : buttons) {
        int bw = px(b.width);
        if (b.id == IDCANCEL) left = right - bw;
        add(L"BUTTON", b.text, WS_TABSTOP | BS_OWNERDRAW, 0, b.id, left, bottomRow, left + bw, bottomRow + button);
        left += bw + px(8);
    }
    add(L"STATIC", L"", SS_LEFT | SS_CENTERIMAGE | SS_ENDELLIPSIS, 0, kLyStatus, pad, height - pad - row, right,
        height - pad);
    FillLayoutsList(0);
    if (g_layouts.empty())
        LayoutsStatus(L"No layouts yet: arrange your windows, then Save current as new.");
    else
        LayoutsStatus(L"The tiling key and 1 to 9 apply the first nine layouts.");
    ShowWindow(g_ly.wnd, SW_SHOW);
    SetForegroundWindow(g_ly.wnd);
    SetFocus(GetDlgItem(g_ly.wnd, kLyList));
}

// The tiling key and a digit: the layout in that place in the list.
void ApplyLayoutNumber(int index) {
    if (index >= 0 && index < (int)g_layouts.size()) ApplyLayout(g_layouts[index].text);
}
