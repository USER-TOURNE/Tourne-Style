////////////////////////////////////////////////////////////////////////////////
// Window rules, in the manner of Hyprland's ("// @inline window-rules.cpp", in
// the Switcher after binds.cpp, whose lines and file they're written in):
//
//     windowrule = float, class:^(Calculator)$
//     windowrule = workspace 2 silent, exe:Discord.exe
//     windowrule = size 50% 60%, title:Picture-in-Picture
//     windowrule = center, float, class:Notepad
//
// The rules first, then the windows they're for: class:, title:, exe: (or
// initialClass:, initialTitle:), with * and ? matching any text, several
// joined by commas all having to match. Hyprland's simple regexes are read
// too: ^(a|b)$ is "a or b", .* is *.
//
// Each rule is for a window as it opens (once). Rules: float, tile, center,
// size W H, move X Y (px or % of the screen's work area, from its top left),
// workspace N [silent] (Windows' desktops), monitor N, pin, maximize (and
// fullscreen), minimize, opacity A (0 to 1), noborder, keep [S]: for S
// seconds (15 if not given) after it opens, the window's monitor, move, size and
// center rules are put back whenever the program itself moves or sizes it (as
// Discord with Vencord does once it's up); a drag of yours ends it.
//
// exec's [rule; rule] (in a bind) are rules for the first window the program
// it starts opens, within 20 seconds.
//
// Nothing is watched while there are no rules and no exec waits for a window.

struct WindowRule {
    std::vector<std::wstring> actions;              // "float", "size 50% 60%"...
    std::vector<std::pair<std::wstring, std::wstring>> match;  // ("class", "a, b"), ...
};

struct ExpectedWindow {
    DWORD pid;
    ULONGLONG until;
    std::vector<std::wstring> actions;
};

std::vector<WindowRule> g_windowRules;
std::vector<ExpectedWindow> g_expectedWindows;
std::unordered_set<HWND> g_ruledWindows;  // windows the rules have seen (each is ruled once)
HWINEVENTHOOK g_ruleHooks[2];

// "^(firefox|Chromium)$" as "firefox, Chromium"; "Code.*" as "Code*".
std::wstring RulePattern(std::wstring p) {
    p = Trimmed(p);
    std::wstring out;
    for (size_t i = 0; i < p.size(); i++) {
        wchar_t c = p[i];
        if (c == L'^' || c == L'$' || c == L'(' || c == L')') continue;
        if (c == L'|') out += L", ";
        else if (c == L'.' && i + 1 < p.size() && p[i + 1] == L'*') out += L'*', i++;
        else if (c == L'\\' && i + 1 < p.size()) out += p[++i];
        else out += c;
    }
    return out;
}

void ReadWindowRules(std::vector<std::wstring> const& lines) {
    g_windowRules.clear();
    for (auto const& line : lines) {
        WindowRule r;
        for (auto const& part : SplitArgs(line, 64)) {
            if (part.empty()) continue;
            size_t colon = part.find(L':');
            std::wstring key = colon == std::wstring::npos ? L"" : Lower(Trimmed(part.substr(0, colon)));
            if (key == L"class" || key == L"title" || key == L"exe" || key == L"initialclass" || key == L"initialtitle")
                r.match.push_back({key.rfind(L"initial", 0) == 0 ? key.substr(7) : key, RulePattern(part.substr(colon + 1))});
            else if (colon == std::wstring::npos)
                r.actions.push_back(part);
            else
                Wh_Log(L"rules: %s isn't a match I know (class:, title:, exe:): %s", key.c_str(), line.c_str());
        }
        if (r.actions.empty() || r.match.empty()) {
            Wh_Log(L"rules: needs a rule and a window: %s", line.c_str());
            continue;
        }
        g_windowRules.push_back(std::move(r));
    }
}

bool RuleMatches(WindowRule const& r, HWND h) {
    for (auto const& [key, pattern] : r.match) {
        std::wstring value = key == L"class" ? ClassOf(h) : key == L"title" ? Title(h) : FileNameOf(h);
        if (!MatchesList(pattern, value)) return false;
    }
    return true;
}

// One rule's action on a window.
void RunWindowRule(HWND h, std::wstring const& action) {
    size_t space = action.find(L' ');
    std::wstring name = Lower(action.substr(0, space)), arg = space == std::wstring::npos ? L"" : Trimmed(action.substr(space + 1));
    HMONITOR monitor = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    if (name == L"float") {
        if (!IsFloating(h)) {
            g_floating.push_back(h);
            if (g_autoTile) TileMonitor(monitor, false);  // (in case the tiling placed it first)
        }
    } else if (name == L"tile") {
        if (IsFloating(h)) {
            std::erase(g_floating, h);
            if (g_autoTile) TileMonitor(monitor, false);
        }
    } else if (name == L"center") {
        RECT f = FrameOf(h);
        int w = f.right - f.left, hh = f.bottom - f.top;
        int x = (mi.rcWork.left + mi.rcWork.right - w) / 2, y = (mi.rcWork.top + mi.rcWork.bottom - hh) / 2;
        PlaceWindowFrame(h, {x, y, x + w, y + hh});
    } else if (name == L"size" || name == L"move") {
        int x = 0, y = 0;
        if (!TwoNumbers(arg, monitor, &x, &y)) return;
        RECT f = FrameOf(h);
        if (name == L"size") f.right = f.left + std::max(50, x), f.bottom = f.top + std::max(50, y);
        else OffsetRect(&f, mi.rcWork.left + x - f.left, mi.rcWork.top + y - f.top);
        PlaceWindowFrame(h, f);
    } else if (name == L"workspace") {
        bool silent = Lower(arg).find(L"silent") != std::wstring::npos;
        std::vector<GUID> ids = VdReadGuids(L"VirtualDesktopIDs");
        int to = _wtoi(arg.c_str()) - 1;
        if (to >= 0 && to < (int)ids.size() && to != VdCurrentIndex(ids)) VdmSendWindow(h, ids[to], !silent);
    } else if (name == L"monitor") {
        SendToMonitor(h, MonitorBy(arg, monitor));
    } else if (name == L"pin") {
        SetWindowPos(h, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else if (name == L"maximize" || name == L"fullscreen") {
        ShowWindow(h, SW_MAXIMIZE);
    } else if (name == L"minimize") {
        ShowWindow(h, SW_MINIMIZE);
    } else if (name == L"opacity") {
        double a = std::clamp(_wtof(arg.c_str()), 0.1, 1.0);
        LONG ex = GetWindowLongW(h, GWL_EXSTYLE);
        if (!(ex & WS_EX_LAYERED)) SetWindowLongW(h, GWL_EXSTYLE, ex | WS_EX_LAYERED);
        SetLayeredWindowAttributes(h, 0, (BYTE)(a * 255), LWA_ALPHA);
    } else if (name == L"noborder") {
        g_noBorder.insert(h);
    } else if (name == L"keep") {
        // (RuleNewWindow's)
    } else {
        Wh_Log(L"rules: unknown rule %s", name.c_str());
    }
}

// keep: a window's place rules, put back when its program moves it for a while.
struct HeldWindow {
    HWND h;
    ULONGLONG until;
    std::vector<std::wstring> actions;  // monitor, move, size, center
    RECT placed;                        // the frame they gave
    HWINEVENTHOOK hooks[2];
};
std::vector<HeldWindow> g_heldWindows;
bool g_keeping;  // (putting one back: its own moves aren't the program's)

// Those whose time is up or that are gone, and h's (all with nullptr).
void EndKeeping(HWND h) {
    ULONGLONG now = GetTickCount64();
    for (auto it = g_heldWindows.begin(); it != g_heldWindows.end();) {
        if (it->h != h && it->until > now && IsWindow(it->h)) {
            ++it;
            continue;
        }
        for (auto hook : it->hooks)
            if (hook) UnhookWinEvent(hook);
        it = g_heldWindows.erase(it);
    }
}

void CALLBACK KeepEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || !h || g_keeping) return;
    auto it = std::find_if(g_heldWindows.begin(), g_heldWindows.end(), [&](HeldWindow const& k) { return k.h == h; });
    if (it == g_heldWindows.end()) return;
    if (event == EVENT_SYSTEM_MOVESIZESTART || it->until < GetTickCount64()) {
        if (event == EVENT_SYSTEM_MOVESIZESTART) Wh_Log(L"rules: keep ended by a drag");
        EndKeeping(h);
        return;
    }
    if (IsIconic(h) || !IsWindowVisible(h)) return;
    RECT f = FrameOf(h);
    if (EqualRect(&f, &it->placed)) return;
    g_keeping = true;
    for (auto const& a : it->actions) RunWindowRule(h, a);
    g_keeping = false;
    it->placed = FrameOf(h);
    Wh_Log(L"rules: kept in place (it had gone to %d,%d %dx%d)", (int)f.left, (int)f.top, (int)(f.right - f.left), (int)(f.bottom - f.top));
}

void KeepWindow(HWND h, std::vector<std::wstring> const& actions, std::wstring const& keep) {
    HeldWindow k{h, 0, {}, FrameOf(h), {}};
    for (auto const& a : actions)
        for (const wchar_t* w : {L"monitor", L"move", L"size", L"center"})
            if (Lower(a).rfind(w, 0) == 0) k.actions.push_back(a);
    if (k.actions.empty()) return;
    size_t space = keep.find(L' ');
    int seconds = space == std::wstring::npos ? 15 : std::clamp(_wtoi(keep.c_str() + space + 1), 1, 600);
    k.until = GetTickCount64() + seconds * 1000ull;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    k.hooks[0] = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr, KeepEventProc, pid, 0, flags);
    k.hooks[1] = SetWinEventHook(EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MOVESIZESTART, nullptr, KeepEventProc, pid, 0, flags);
    g_heldWindows.push_back(std::move(k));
    // Over once the time's up, even if the program never moves it again.
    SetTimer(nullptr, 0, seconds * 1000 + 50, [](HWND, UINT, UINT_PTR id, DWORD) {
        KillTimer(nullptr, id);
        EndKeeping(nullptr);
    });
}

void WatchNewWindows();

// A window opening: the rules for it, and an exec's that waits for it.
void RuleNewWindow(HWND h) {
    if (g_ruledWindows.contains(h) || !BindWindow(h)) return;
    g_ruledWindows.insert(h);
    std::vector<std::wstring> actions;
    for (auto const& r : g_windowRules)
        if (RuleMatches(r, h)) actions.insert(actions.end(), r.actions.begin(), r.actions.end());
    if (!g_expectedWindows.empty()) {
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        ULONGLONG now = GetTickCount64();
        for (auto it = g_expectedWindows.begin(); it != g_expectedWindows.end();) {
            if (it->until < now) {
                it = g_expectedWindows.erase(it);
            } else if (it->pid == pid) {
                actions.insert(actions.end(), it->actions.begin(), it->actions.end());
                it = g_expectedWindows.erase(it);
            } else {
                ++it;
            }
        }
    }
    // Where it goes before how big, so a size in % is of its own screen.
    std::stable_sort(actions.begin(), actions.end(), [](std::wstring const& a, std::wstring const& b) {
        auto rank = [](std::wstring const& x) { return x.rfind(L"monitor", 0) == 0 ? 0 : x.rfind(L"workspace", 0) == 0 ? 2 : 1; };
        return rank(a) < rank(b);
    });
    for (auto const& a : actions) RunWindowRule(h, a);
    for (auto const& a : actions)
        if (Lower(a).rfind(L"keep", 0) == 0) {
            KeepWindow(h, actions, a);
            break;
        }
    WatchNewWindows();  // (an exec's window found, or its wait over: maybe nothing left to watch)
}

void CALLBACK RuleEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || !h) return;
    if (event == EVENT_OBJECT_DESTROY) {
        g_ruledWindows.erase(h);
        g_noBorder.erase(h);
        if (!g_heldWindows.empty()) EndKeeping(h);
        return;
    }
    RuleNewWindow(h);
}

void WatchNewWindows() {
    bool want = !g_windowRules.empty() || !g_expectedWindows.empty();
    if (want == (g_ruleHooks[0] != nullptr)) return;
    for (auto& hook : g_ruleHooks) {
        if (hook) UnhookWinEvent(hook);
        hook = nullptr;
    }
    if (!want) return;
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    g_ruleHooks[0] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_SHOW, nullptr, RuleEventProc, 0, 0, flags);
    g_ruleHooks[1] = SetWinEventHook(EVENT_OBJECT_UNCLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, RuleEventProc, 0, 0, flags);
    // The windows open now aren't new: rules are for windows as they open.
    for (HWND h : BindWindows()) g_ruledWindows.insert(h);
}

void ExpectWindowRules(DWORD pid, std::wstring const& rules) {
    if (!pid || rules.empty()) return;
    ExpectedWindow e{pid, GetTickCount64() + 20000, {}};
    size_t at = 0;
    while (at <= rules.size()) {
        size_t semi = rules.find(L';', at);
        std::wstring a = Trimmed(rules.substr(at, semi == std::wstring::npos ? std::wstring::npos : semi - at));
        if (!a.empty()) e.actions.push_back(a);
        if (semi == std::wstring::npos) break;
        at = semi + 1;
    }
    if (e.actions.empty()) return;
    g_expectedWindows.push_back(std::move(e));
    WatchNewWindows();
}

void ApplyWindowRules(std::vector<std::wstring> const& lines) {
    ReadWindowRules(lines);
    if (!g_windowRules.empty()) Wh_Log(L"rules: %d read", (int)g_windowRules.size());
    WatchNewWindows();
}

void EndWindowRules() {
    g_windowRules.clear();
    g_expectedWindows.clear();
    for (auto& k : g_heldWindows) k.until = 0;
    EndKeeping(nullptr);
    WatchNewWindows();
    g_ruledWindows.clear();
}
