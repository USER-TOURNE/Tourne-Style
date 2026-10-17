////////////////////////////////////////////////////////////////////////////////
// Window groups, in the manner of Hyprland's ("// @inline tiling-groups.cpp",
// in the Switcher after the binds): windows stacked in one place with a bar of
// tabs over them, one shown at a time. The tiling gives a group one tile.
//
// Made and used with the binds' commands: togglegroup (the window in front
// starts a group, or its group comes apart), changegroupactive f|b|N,
// moveintogroup l|r|u|d (into the group that way), moveoutofgroup, and
// lockactivegroup is not there yet. Clicking a tab shows that window.
//
// Nothing runs for groups while there are none.

struct WindowGroup {
    std::vector<HWND> members;
    int active = 0;
    HWND bar = nullptr;
    RECT barRect{};
};

std::vector<std::unique_ptr<WindowGroup>> g_groups;
bool g_groupBarClass = false;
HWINEVENTHOOK g_groupHook = nullptr;  // the window in front, for raising its group's bar (only while groups exist)
constexpr WCHAR kGroupBarClass[] = L"TourneGroupBar_" WH_MOD_ID;

WindowGroup* GroupOf(HWND h) {
    for (auto& g : g_groups)
        if (std::find(g->members.begin(), g->members.end(), h) != g->members.end()) return g.get();
    return nullptr;
}

HWND GroupActive(WindowGroup const& g) {
    return g.members.empty() ? nullptr : g.members[std::clamp(g.active, 0, (int)g.members.size() - 1)];
}

int GroupBarHeight(HWND where) {
    return MulDiv(24, MonitorDpi(MonitorFromWindow(where, MONITOR_DEFAULTTONEAREST)), 96);
}

void PaintGroupBar(HWND wnd, HDC dc) {
    WindowGroup* group = nullptr;
    for (auto& g : g_groups)
        if (g->bar == wnd) group = g.get();
    RECT rc;
    GetClientRect(wnd, &rc);
    HBRUSH back = CreateSolidBrush(RGB(20, 24, 28));
    FillRect(dc, &rc, back);
    DeleteObject(back);
    if (!group || group->members.empty()) return;
    DWORD color = 0;
    BOOL opaque = FALSE;
    COLORREF accent = SUCCEEDED(DwmGetColorizationColor(&color, &opaque))
                          ? RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF)
                          : RGB(100, 170, 137);
    int n = (int)group->members.size(), w = rc.right - rc.left;
    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    HFONT font = CreateFontIndirectW(&ncm.lfCaptionFont);
    HGDIOBJ old = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    for (int i = 0; i < n; i++) {
        RECT tab{rc.left + w * i / n + 1, rc.top + 2, rc.left + w * (i + 1) / n - 1, rc.bottom};
        bool on = i == group->active;
        HBRUSH fill = CreateSolidBrush(on ? accent : RGB(38, 44, 50));
        FillRect(dc, &tab, fill);
        DeleteObject(fill);
        WCHAR title[256] = L"";
        GetWindowTextW(group->members[i], title, ARRAYSIZE(title));
        RECT text = tab;
        InflateRect(&text, -8, 0);
        SetTextColor(dc, on ? RGB(12, 14, 16) : RGB(220, 224, 228));
        DrawTextW(dc, title, -1, &text, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
    }
    SelectObject(dc, old);
    DeleteObject(font);
}

void ShowGroupMember(WindowGroup& g, int index);

LRESULT CALLBACK GroupBarProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PaintGroupBar(wnd, dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN:
            for (auto& g : g_groups)
                if (g->bar == wnd && !g->members.empty()) {
                    RECT rc;
                    GetClientRect(wnd, &rc);
                    int n = (int)g->members.size(), w = std::max(1, (int)(rc.right - rc.left));
                    ShowGroupMember(*g, std::clamp(GET_X_LPARAM(lParam) * n / w, 0, n - 1));
                    break;
                }
            return 0;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// The bar put over the group's place (bar: its rectangle on the screen), just above the window shown.
void PlaceGroupBar(WindowGroup& g, RECT const& bar) {
    if (!g.bar) {
        HINSTANCE instance = (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE);
        if (!g_groupBarClass) {
            WNDCLASSW wc{};
            wc.lpfnWndProc = GroupBarProc;
            wc.hInstance = instance;
            wc.hCursor = LoadCursorW(nullptr, IDC_HAND);
            wc.lpszClassName = kGroupBarClass;
            g_groupBarClass = RegisterClassW(&wc) != 0;
        }
        g.bar = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kGroupBarClass, L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr,
                                instance, nullptr);
    }
    if (!g.bar) return;
    g.barRect = bar;
    HWND shown = GroupActive(g);
    // Just above the window shown: in front of it, behind whatever covers it.
    HWND above = shown ? GetWindow(shown, GW_HWNDPREV) : HWND_TOP;
    SetWindowPos(g.bar, above ? above : HWND_TOP, bar.left, bar.top, bar.right - bar.left, bar.bottom - bar.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(g.bar, nullptr, FALSE);
}

// A group's windows stacked at a place (frame: where the windows go; the bar above it).
void ArrangeGroup(WindowGroup& g, RECT frame, bool tiled) {
    HWND shown = GroupActive(g);
    for (HWND h : g.members) {
        if (tiled) TilePlace(h, frame);
        else PlaceWindowFrame(h, frame);
    }
    if (shown) SetWindowPos(shown, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    int barH = GroupBarHeight(shown ? shown : (HWND)g_wnd);
    PlaceGroupBar(g, {frame.left, frame.top - barH, frame.right, frame.top});
}

void EndGroup(WindowGroup* g) {
    if (g->bar) DestroyWindow(g->bar);
    std::erase_if(g_groups, [&](auto const& p) { return p.get() == g; });
}

void CALLBACK GroupEventProc(HWINEVENTHOOK, DWORD, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || !h) return;
    WindowGroup* g = GroupOf(GetAncestor(h, GA_ROOT));
    if (!g || !g->bar) return;
    int at = (int)(std::find(g->members.begin(), g->members.end(), GetAncestor(h, GA_ROOT)) - g->members.begin());
    if (at != g->active) {  // (another of its windows came to the front: its tab goes with it)
        g->active = at;
        InvalidateRect(g->bar, nullptr, FALSE);
    }
    SetWindowPos(g->bar, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void WatchGroups() {
    bool want = !g_groups.empty();
    if (want == (g_groupHook != nullptr)) return;
    if (g_groupHook) UnhookWinEvent(g_groupHook);
    g_groupHook = want ? SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, GroupEventProc, 0, 0,
                                         WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS)
                       : nullptr;
}

// Windows gone are let go, and groups left empty end.
void PruneGroups() {
    for (auto& g : g_groups) {
        HWND shown = GroupActive(*g);
        std::erase_if(g->members, [](HWND h) { return !IsWindow(h); });
        auto it = std::find(g->members.begin(), g->members.end(), shown);
        g->active = it == g->members.end() ? 0 : (int)(it - g->members.begin());
    }
    std::vector<WindowGroup*> empty;
    for (auto& g : g_groups)
        if (g->members.empty()) empty.push_back(g.get());
    for (auto* g : empty) EndGroup(g);
    WatchGroups();
}

// The tiling's list of a screen's windows with each group as one: its shown window.
std::vector<HWND> FoldGroups(std::vector<HWND> const& windows) {
    if (g_groups.empty()) return windows;
    PruneGroups();
    std::vector<HWND> out;
    for (HWND h : windows) {
        WindowGroup* g = GroupOf(h);
        if (!g) {
            out.push_back(h);
            continue;
        }
        HWND shown = GroupActive(*g);
        if (std::find(windows.begin(), windows.end(), shown) == windows.end()) shown = h;  // (its shown one isn't here)
        if (std::find(out.begin(), out.end(), shown) == out.end()) out.push_back(shown);
    }
    return out;
}

// A tile for a window: true if it's a group's, which takes the tile (the bar
// at its top) and is put there.
bool PlaceGroupTile(HWND h, RECT const& tile) {
    WindowGroup* g = g_groups.empty() ? nullptr : GroupOf(h);
    if (!g) return false;
    RECT frame = tile;
    frame.top += GroupBarHeight(h);
    ArrangeGroup(*g, frame, true);
    return true;
}

void ShowGroupMember(WindowGroup& g, int index) {
    if (g.members.empty()) return;
    int n = (int)g.members.size();
    g.active = ((index % n) + n) % n;
    HWND shown = GroupActive(g);
    Activate(shown);
    if (g.bar) {
        InvalidateRect(g.bar, nullptr, FALSE);
        SetWindowPos(g.bar, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

// The binds' group commands. The answer is "ok", or what went wrong.
std::wstring GroupCommand(std::wstring const& name, std::wstring const& arg, HWND active) {
    PruneGroups();
    if (!active) return L"error: no window";
    WindowGroup* g = GroupOf(active);
    HMONITOR monitor = MonitorFromWindow(active, MONITOR_DEFAULTTONEAREST);
    auto relayout = [&](WindowGroup* which) {
        if (IsTiled(active) || (which && IsTiled(GroupActive(*which)))) TileMonitor(monitor, false);
        else if (which) ArrangeGroup(*which, FrameOf(GroupActive(*which)), false);
    };
    if (name == L"togglegroup") {
        if (g) {
            EndGroup(g);
            WatchGroups();
            relayout(nullptr);
            return L"ok";
        }
        auto made = std::make_unique<WindowGroup>();
        made->members.push_back(active);
        g = made.get();
        g_groups.push_back(std::move(made));
        WatchGroups();
        // Floating, the window keeps its place and the bar goes over it.
        RECT f = FrameOf(active);
        if (!IsTiled(active)) f.top += GroupBarHeight(active);
        ArrangeGroup(*g, f, false);
        relayout(g);
        return L"ok";
    }
    if (name == L"changegroupactive") {
        if (!g) return L"error: not in a group";
        std::wstring a = Lower(Trimmed(arg));
        int to = a == L"b" ? g->active - 1 : a.empty() || a == L"f" ? g->active + 1 : _wtoi(a.c_str()) - 1;
        ShowGroupMember(*g, to);
        return L"ok";
    }
    if (name == L"moveintogroup") {
        int direction = DirectionByName(arg);
        if (direction < 0) return L"error: give l, r, u or d";
        HWND other = WindowThatWay(active, direction);
        WindowGroup* into = other ? GroupOf(other) : nullptr;
        if (!into || into == g) return L"error: no group that way";
        if (g) std::erase(g->members, active);
        into->members.push_back(active);
        into->active = (int)into->members.size() - 1;
        PruneGroups();
        relayout(into);
        ShowGroupMember(*into, into->active);
        return L"ok";
    }
    if (name == L"moveoutofgroup") {
        if (!g) return L"error: not in a group";
        std::erase(g->members, active);
        HWND rest = GroupActive(*g);
        PruneGroups();
        relayout(rest ? GroupOf(rest) : nullptr);
        if (!IsTiled(active)) {
            RECT f = FrameOf(active);
            OffsetRect(&f, GroupBarHeight(active), GroupBarHeight(active));
            PlaceWindowFrame(active, f);
        }
        return L"ok";
    }
    return L"error: unknown command " + name;
}

void EndGroups() {
    while (!g_groups.empty()) EndGroup(g_groups.back().get());
    WatchGroups();
    if (g_groupBarClass && g_wnd) UnregisterClassW(kGroupBarClass, (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE));
    g_groupBarClass = false;
}
