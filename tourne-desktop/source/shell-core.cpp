////////////////////////////////////////////////////////////////////////////////
// Shell core ("// @inline shell-core.cpp", in the taskbar and System Icons,
// after tray-shared.cpp): what every surface beside the bar leans on, in one
// place, so none of them works it out its own way.
//
// - Where each bar is: its screen, its edge (from its settings, not guessed
//   from the space it reserves), the rect it has when shown (also while it's
//   hidden) and its DPI. Each bar publishes this on its own window; any
//   popup reads it from there, so it works across parts and namespaces.
// - Where a popup goes: on that bar's screen, on the inner side of its edge,
//   beside what it was opened from. Never from Explorer's hidden taskbar.
// - Bringing a window to the front, checked and retried, as Explorer does.
// - Popups kept above windows: raised every time they show, never owned by a
//   bar that can drop behind a fullscreen program.
//
// Nothing here runs on its own: no timers and no hooks. A bar writes its
// record when it's laid out; a popup reads it when it opens.

enum ShellEdge { kShellTop = 0, kShellBottom = 1, kShellLeft = 2, kShellRight = 3 };

struct ShellBar {
    HWND wnd = nullptr;
    HMONITOR monitor = nullptr;
    RECT shown{};  // the bar's rect when shown, on the screen
    int edge = kShellBottom;
    UINT dpi = 96;
    bool main = false;
};

[[maybe_unused]] constexpr WCHAR kShellBarClassPrefix[] = L"TourneTaskbarTest_";
[[maybe_unused]] constexpr WCHAR kShellRectProp[] = L"TourneShell_BarRect";  // four int16s
[[maybe_unused]] constexpr WCHAR kShellInfoProp[] = L"TourneShell_BarInfo";  // edge | main << 4 | dpi << 8, plus one

inline uint64_t ShellPackRect(RECT const& r) {
    return (uint64_t)(uint16_t)(int16_t)r.left | (uint64_t)(uint16_t)(int16_t)r.top << 16 |
           (uint64_t)(uint16_t)(int16_t)r.right << 32 | (uint64_t)(uint16_t)(int16_t)r.bottom << 48;
}

inline RECT ShellUnpackRect(uint64_t v) {
    return {(int16_t)(v & 0xFFFF), (int16_t)(v >> 16 & 0xFFFF), (int16_t)(v >> 32 & 0xFFFF), (int16_t)(v >> 48 & 0xFFFF)};
}

// A bar says where it is (from its layout, each time that changes).
[[maybe_unused]] void ShellPublishBar(HWND bar, RECT const& shown, int edge, UINT dpi, bool main) {
    if (!bar) return;
    SetPropW(bar, kShellRectProp, (HANDLE)(ULONG_PTR)ShellPackRect(shown));
    ULONG_PTR info = 1 + ((ULONG_PTR)(edge & 0xF) | (ULONG_PTR)(main ? 1 : 0) << 4 | (ULONG_PTR)(dpi & 0xFFFF) << 8);
    SetPropW(bar, kShellInfoProp, (HANDLE)info);
}

[[maybe_unused]] void ShellForgetBar(HWND bar) {
    if (!bar) return;
    RemovePropW(bar, kShellRectProp);
    RemovePropW(bar, kShellInfoProp);
}

bool ShellReadBar(HWND bar, ShellBar* out) {
    ULONG_PTR info = (ULONG_PTR)GetPropW(bar, kShellInfoProp);
    if (!info || !IsWindow(bar)) return false;
    info -= 1;
    out->wnd = bar;
    out->shown = ShellUnpackRect((uint64_t)(ULONG_PTR)GetPropW(bar, kShellRectProp));
    out->edge = (int)(info & 0xF);
    out->main = (info >> 4 & 1) != 0;
    out->dpi = (UINT)(info >> 8 & 0xFFFF);
    if (!out->dpi) out->dpi = 96;
    out->monitor = MonitorFromRect(&out->shown, MONITOR_DEFAULTTONEAREST);
    return true;
}

// Every bar that has said where it is, the main one first.
std::vector<ShellBar> ShellBars() {
    std::vector<ShellBar> bars;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            WCHAR cls[96];
            if (!GetClassNameW(h, cls, ARRAYSIZE(cls)) || wcsncmp(cls, kShellBarClassPrefix, ARRAYSIZE(kShellBarClassPrefix) - 1))
                return TRUE;
            ShellBar b;
            if (ShellReadBar(h, &b)) ((std::vector<ShellBar>*)param)->push_back(b);
            return TRUE;
        },
        (LPARAM)&bars);
    std::stable_sort(bars.begin(), bars.end(), [](ShellBar const& a, ShellBar const& b) { return a.main && !b.main; });
    return bars;
}

// The bar a popup belongs to: the one it was opened from (the window given,
// or the bar holding the point), else the bar on that point's screen, else
// the main bar. False when the suite's bar isn't running.
[[maybe_unused]] bool ShellFindBar(POINT pt, HWND from, ShellBar* out) {
    std::vector<ShellBar> bars = ShellBars();
    if (bars.empty()) return false;
    if (from) {
        HWND root = GetAncestor(from, GA_ROOT);
        for (auto const& b : bars)
            if (b.wnd == from || b.wnd == root) return *out = b, true;
    }
    for (auto const& b : bars) {
        RECT r = b.shown;
        InflateRect(&r, 2, 2);
        if (PtInRect(&r, pt)) return *out = b, true;
    }
    HMONITOR m = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    for (auto const& b : bars)
        if (b.monitor == m) return *out = b, true;
    *out = bars[0];
    return true;
}

// The screen a bar is on, without the bar: where its popups may go (the
// bar's shown rect is taken away even while it's hidden, so popups never
// land under it).
RECT ShellRoomBesideBar(ShellBar const& bar) {
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(bar.monitor, &mi);
    RECT r = mi.rcMonitor;
    switch (bar.edge) {
        case kShellTop: r.top = std::max(r.top, bar.shown.bottom); break;
        case kShellBottom: r.bottom = std::min(r.bottom, bar.shown.top); break;
        case kShellLeft: r.left = std::max(r.left, bar.shown.right); break;
        case kShellRight: r.right = std::min(r.right, bar.shown.left); break;
    }
    return r;
}

// Where a popup of this size goes: against the bar's inner side (gap px
// away), lined up with what it came from (its rect on the screen; centred on
// it, or at the cursor when empty), kept on that screen.
[[maybe_unused]] RECT ShellPlaceBesideBar(ShellBar const& bar, SIZE size, RECT const& from, int gap) {
    RECT room = ShellRoomBesideBar(bar);
    POINT c;
    if (IsRectEmpty(&from)) GetCursorPos(&c);
    else c = {(from.left + from.right) / 2, (from.top + from.bottom) / 2};
    int x = c.x - size.cx / 2, y = c.y - size.cy / 2;
    switch (bar.edge) {
        case kShellTop: y = room.top + gap; break;
        case kShellBottom: y = room.bottom - gap - size.cy; break;
        case kShellLeft: x = room.left + gap; break;
        case kShellRight: x = room.right - gap - size.cx; break;
    }
    x = std::clamp(x, (int)room.left, std::max((int)room.left, (int)room.right - (int)size.cx));
    y = std::clamp(y, (int)room.top, std::max((int)room.top, (int)room.bottom - (int)size.cy));
    return {x, y, x + size.cx, y + size.cy};
}

// The same for a menu shown with TrackPopupMenu: the point and flags that
// open it away from the bar's edge, on its screen.
[[maybe_unused]] UINT ShellMenuAlign(ShellBar const& bar, POINT* pt) {
    RECT room = ShellRoomBesideBar(bar);
    pt->x = std::clamp(pt->x, room.left, room.right - 1);
    pt->y = std::clamp(pt->y, room.top, room.bottom - 1);
    switch (bar.edge) {
        case kShellTop: return TPM_TOPALIGN | TPM_LEFTALIGN | TPM_VERTICAL;
        case kShellLeft: return TPM_LEFTALIGN | TPM_TOPALIGN;
        case kShellRight: return TPM_RIGHTALIGN | TPM_TOPALIGN;
        default: return TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_VERTICAL;
    }
}

// A popup kept above other windows: called each time it shows (and when its
// bar is raised), so a window put on top since doesn't cover it.
[[maybe_unused]] void ShellRaise(HWND popup) {
    if (popup && IsWindowVisible(popup))
        SetWindowPos(popup, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

// A window brought to the front, as Explorer's taskbar does it: restored
// first if minimised, then asked for, checked, and asked again the ways
// Windows allows if it was refused. False if Windows still refused.
[[maybe_unused]] bool ShellBringToFront(HWND w) {
    if (!w || !IsWindow(w)) return false;
    HWND root = GetAncestor(w, GA_ROOTOWNER);
    HWND popup = GetLastActivePopup(root ? root : w);
    HWND target = popup && IsWindowVisible(popup) && IsWindowEnabled(popup) ? popup : w;
    if (IsIconic(w)) PostMessageW(w, WM_SYSCOMMAND, SC_RESTORE, 0);
    auto inFront = [&] {
        HWND fg = GetForegroundWindow();
        return fg == target || fg == w || (fg && GetAncestor(fg, GA_ROOTOWNER) == (root ? root : w));
    };
    if (SetForegroundWindow(target) && inFront()) return true;
    // Windows lets the process that had the last input event take the front:
    // a key nothing uses, pressed and let go.
    INPUT key[2]{};
    key[0].type = key[1].type = INPUT_KEYBOARD;
    key[0].ki.wVk = key[1].ki.wVk = 0xE8;
    key[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, key, sizeof(INPUT));
    if (SetForegroundWindow(target) && inFront()) return true;
    // Joined to the window in front's input for a moment.
    HWND fg = GetForegroundWindow();
    DWORD them = fg ? GetWindowThreadProcessId(fg, nullptr) : 0, me = GetCurrentThreadId();
    bool joined = them && them != me && AttachThreadInput(me, them, TRUE);
    BringWindowToTop(target);
    SetForegroundWindow(target);
    if (joined) AttachThreadInput(me, them, FALSE);
    if (inFront()) return true;
    SwitchToThisWindow(target, TRUE);
    bool ok = inFront();
    Wh_Log(L"Shell core: bringing %p to the front %s (in front: %p)", target, ok ? L"took a last try" : L"was refused",
           GetForegroundWindow());
    return ok;
}

// Whether a program is a game, as Windows' own Game Bar knows it (the games
// it has seen, in GameConfigStore) or by where it's installed. Asked only
// while a maximised window is in front, and kept per process.
[[maybe_unused]] bool ShellIsGame(HWND w) {
    static std::unordered_map<DWORD, bool> known;
    static std::vector<std::wstring> gameStore;
    static ULONGLONG storeRead = 0;
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (!pid) return false;
    auto it = known.find(pid);
    if (it != known.end()) return it->second;
    if (known.size() > 256) known.clear();
    WCHAR path[MAX_PATH] = L"";
    DWORD len = ARRAYSIZE(path);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process) {
        if (!QueryFullProcessImageNameW(process, 0, path, &len)) path[0] = 0;
        CloseHandle(process);
    }
    std::wstring lower = path;
    CharLowerBuffW(lower.data(), (DWORD)lower.size());
    bool game = false;
    for (PCWSTR place : {L"\\steamapps\\common\\", L"\\epic games\\", L"\\gog galaxy\\games\\", L"\\xboxgames\\",
                         L"\\riot games\\", L"\\ubisoft game launcher\\games\\", L"\\ea games\\", L"\\battle.net\\"})
        if (lower.find(place) != std::wstring::npos) game = true;
    if (!game && !lower.empty()) {
        ULONGLONG now = GetTickCount64();
        if (!storeRead || now - storeRead > 10 * 60 * 1000) {
            storeRead = now;
            gameStore.clear();
            HKEY children;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, L"System\\GameConfigStore\\Children", 0, KEY_READ, &children) == ERROR_SUCCESS) {
                WCHAR name[128];
                for (DWORD i = 0;; i++) {
                    DWORD nameLen = ARRAYSIZE(name);
                    if (RegEnumKeyExW(children, i, name, &nameLen, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
                    WCHAR exe[MAX_PATH];
                    DWORD size = sizeof(exe);
                    if (RegGetValueW(children, name, L"MatchedExeFullPath", RRF_RT_REG_SZ, nullptr, exe, &size) == ERROR_SUCCESS) {
                        std::wstring e = exe;
                        CharLowerBuffW(e.data(), (DWORD)e.size());
                        gameStore.push_back(std::move(e));
                    }
                }
                RegCloseKey(children);
            }
        }
        game = std::find(gameStore.begin(), gameStore.end(), lower) != gameStore.end();
    }
    known[pid] = game;
    return game;
}

////////////////////////////////////////////////////////////////////////////////
// Opening a Start entry (shell:AppsFolder\<id>) without Explorer. Opening
// those through the shell needs Explorer running (without it, Windows says
// "Class not registered"), so a packaged app is started as Windows' own Start
// starts it, and any other app through the program its entry points at.
// False when it's neither (the caller opens it through the shell then).

bool ShellLaunchAppsFolder(std::wstring const& parsing, PCWSTR args = nullptr) {
    constexpr std::wstring_view kApps = L"shell:appsfolder\\";
    if (parsing.size() <= kApps.size() || _wcsnicmp(parsing.c_str(), kApps.data(), kApps.size()) != 0) return false;
    std::wstring id = parsing.substr(kApps.size());
    if (id.find(L'!') != std::wstring::npos) {
        constexpr CLSID kActivationManager = {0x45BA127D, 0x10A8, 0x46EA, {0x8A, 0xB7, 0x56, 0xEA, 0x90, 0x78, 0x94, 0x3C}};
        constexpr IID kIidActivationManager = {0x2E941141, 0x7F97, 0x4756, {0xBA, 0x1D, 0x9D, 0xEC, 0xDE, 0x89, 0x4A, 0x3D}};
        IApplicationActivationManager* manager = nullptr;
        if (SUCCEEDED(CoCreateInstance(kActivationManager, nullptr, CLSCTX_LOCAL_SERVER, kIidActivationManager, (void**)&manager))) {
            DWORD pid = 0;
            HRESULT hr = manager->ActivateApplication(id.c_str(), args, AO_NONE, &pid);
            manager->Release();
            if (SUCCEEDED(hr)) return true;
        }
    }
    // A desktop app's entry: the program (and arguments) its shortcut points at.
    IShellItem2* item = nullptr;
    if (FAILED(SHCreateItemFromParsingName(parsing.c_str(), nullptr, IID_PPV_ARGS(&item)))) return false;
    constexpr PROPERTYKEY kTarget = {{0xB9B4B3FC, 0x2B51, 0x4A42, {0xB5, 0xD8, 0x32, 0x41, 0x46, 0xAF, 0xCF, 0x25}}, 2};
    constexpr PROPERTYKEY kArguments = {{0x436F2667, 0x14E2, 0x4FEB, {0xB3, 0x0A, 0x14, 0x6C, 0x53, 0xB5, 0xB6, 0x74}}, 100};
    LPWSTR target = nullptr, arguments = nullptr;
    bool ok = false;
    if (SUCCEEDED(item->GetString(kTarget, &target)) && target && *target) {
        item->GetString(kArguments, &arguments);
        std::wstring all = arguments ? arguments : L"";
        if (args && *args) all += (all.empty() ? L"" : L" ") + std::wstring(args);
        ok = (INT_PTR)ShellExecuteW(nullptr, nullptr, target, all.empty() ? nullptr : all.c_str(), nullptr, SW_SHOWNORMAL) > 32;
    }
    CoTaskMemFree(target);
    CoTaskMemFree(arguments);
    item->Release();
    return ok;
}
