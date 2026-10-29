////////////////////////////////////////////////////////////////////////////////
// Key binds, in the manner of Hyprland's ("// @inline binds.cpp", in the
// Switcher after the tiling and the borders, whose keyboard hook it shares),
// and tournectl, the way scripts and pieces control Tourne and hear what
// happens (Hyprland's hyprctl and its event socket).
//
// Binds are written as Hyprland writes them, one a line:
//
//     $mod = WIN
//     bind = $mod, Q, killactive
//     bind = $mod SHIFT, 2, movetoworkspace, 2
//     bind = $mod, R, submap, resize
//     bind = $mod, N, run, workspace 2; wait 200; exec notepad
//     submap = resize
//     binde = , right, resizeactive, 20 0
//     bind = , escape, submap, reset
//     submap = reset
//
// bind takes flags as Hyprland's does: e (repeats while held), r (on
// release), l (in fullscreen programs too), n (the program in front hears
// the key as well), d (a description after the key: bindd = MODS, key,
// what it does, command, argument), and two of Tourne's own: k (a double press: the key twice within
// binds.doubleMs, 300 ms unless set) and o (held down: o alone for binds.holdMs, 500 ms unless set, or with
// its own time in ms, as bindo800). A modifier can be the key, tapped on its own (pressed and let go within
// binds.holdMs with nothing else pressed): bind = SUPER, SUPER_L, launcher (or , WIN / SHIFT_R / ALT_L ...).
// A Win or Alt tap that runs a bind doesn't open Start or the menu bar. A key may be a combination of keys held together, as Hyprland writes
// them: escape&end is End pressed while Esc is held (the keys before the last are held, in any order).
// Plugins' hotkeys come in as binds too (the PluginBinds lines Tourne Desktop's plugin runtime writes,
// with the plugin dispatcher). Nothing is bound until the user writes a line, and the
// whole part is off until turned on: a bound key is taken by Tourne, so the
// program in front never hears it.
//
// Off, nothing here runs: the hook returns at once and no pipe is made. With
// control on, one thread waits on the pipe (no work while no one calls), and
// events are only watched while a listener is connected.

struct Bind {
    UINT mods = 0;  // MOD_ flags, exactly these held
    DWORD vk = 0;
    bool catchAll = false;  // "catchall": any key not bound otherwise, in a submap
    bool repeat = false, release = false, locked = false, passThrough = false;
    std::vector<DWORD> chord;  // other keys held with it (escape&end: escape)
    bool twice = false;        // k: a double press
    int holdMs = 0;            // o: held this long
    int submap = 0;  // 0: the global binds
    std::wstring dispatcher, arg, line;
    std::wstring description;  // bindd's: what the bind does, in words (the keybind guide shows it)
};

struct BindTable {
    std::vector<Bind> binds;
    // bindm = MODS, mouse:272, movewindow (or resizewindow): drag a window with a button held with them.
    struct MouseBind {
        UINT mods;
        int button;  // 0 left (mouse:272), 1 right (273), 2 middle (274)
        bool resize;
        std::wstring description, line;  // (bindmd's words; the line, for the keybind guide)
    };
    std::vector<MouseBind> mouseBinds;
    std::vector<std::wstring> submaps{L""};
    std::vector<std::wstring> ruleLines;  // windowrule lines, for rules.cpp
    // idle = SECONDS, command, argument (hypridle's listeners): run once that long after the last input.
    struct Idle {
        int seconds;
        std::wstring dispatcher, arg;
    };
    std::vector<Idle> idle;
};

constexpr UINT WM_APP_BIND = WM_APP + 40;      // wParam: a bind's index; lParam: the table's generation
constexpr UINT WM_APP_BINDCALL = WM_APP + 41;  // lParam: a BindCall, sent by the control thread
constexpr UINT WM_APP_BINDWATCH = WM_APP + 42;  // wParam: listeners there are (watch events or stop)
constexpr UINT WM_APP_MOUSEDRAG = WM_APP + 43;  // wParam: 0 start (lParam: resize), 1 moved, 2 let go
constexpr UINT WM_APP_BINDHOLD = WM_APP + 44;   // wParam: a held bind's index; lParam: the table's generation
constexpr UINT WM_APP_BINDTAP = WM_APP + 45;    // wParam: a key let go before its hold bind's time (pressed again)
constexpr UINT_PTR kTimerBindHold = 0x4248;
constexpr UINT_PTR kTimerBindTapLater = 0x4249;  // a tap waiting out the double press's time
int g_tapLater = -1;                              // that tap's plain bind (switcher thread)
unsigned g_tapLaterGeneration = 0;
ULONGLONG g_holdPressedAt = 0;                    // when the held key went down (switcher thread)
constexpr ULONG_PTR kBindReplayTag = 0x42494E44;  // "BIND": input this sent

std::mutex g_bindsLock;  // g_binds is read by the hook thread
BindTable g_binds;
std::atomic<bool> g_bindsOn{false};
std::atomic<int> g_submap{0};
std::atomic<unsigned> g_bindsGeneration{0};

// On the hook thread: keys kept back, so their release (and repeats) are kept
// back too; with the bind to run on release, if it is one.
struct HeldBind {
    DWORD vk;
    int runOnRelease;  // a bind's index, or -1
    bool passThrough;
};
std::vector<HeldBind> g_bindKeysHeld;
std::vector<DWORD> g_keysDown;  // every key down now but the modifiers (hook thread), for combinations
// A hold bind waiting for its time: its index (-1: none), the key, whether it ran; the timer is the switcher
// window's (WM_APP_BINDHOLD starts it).
std::atomic<int> g_holdBind{-1};
std::atomic<DWORD> g_holdVk{0};
std::atomic<bool> g_holdRan{false};
unsigned g_holdGeneration = 0;  // (the switcher window's thread)
// Double presses: the last press of each double bind, with the table's generation then (hook thread).
std::map<int, std::pair<unsigned, ULONGLONG>> g_lastPress;
int g_doubleMs = 300, g_holdDefaultMs = 500;

////////////////////////////////////////////////////////////////////////////////
// Reading the lines

std::wstring Trimmed(std::wstring s) {
    size_t a = s.find_first_not_of(L" \t\r\n");
    if (a == std::wstring::npos) return L"";
    size_t b = s.find_last_not_of(L" \t\r\n");
    return s.substr(a, b - a + 1);
}

// The command language all four surfaces speak (a key, a menu item, a bar
// click and tournectl), and its own wait step.
// @inline command.cpp

UINT ModsByName(std::wstring const& text, bool* ok) {
    UINT mods = 0;
    *ok = true;
    std::wstring part;
    auto take = [&] {
        std::wstring p = Lower(part);
        part.clear();
        if (p.empty()) return;
        if (p == L"win" || p == L"super" || p == L"mod4" || p == L"logo" || p == L"meta") mods |= MOD_WIN;
        else if (p == L"ctrl" || p == L"control") mods |= MOD_CONTROL;
        else if (p == L"alt" || p == L"mod1") mods |= MOD_ALT;
        else if (p == L"shift") mods |= MOD_SHIFT;
        else *ok = false;
    };
    for (wchar_t c : text) {
        if (c == L' ' || c == L'_' || c == L'+' || c == L'\t') take();
        else part += c;
    }
    take();
    return mods;
}

// A key by Hyprland's name for it (xkb's), or "code:NN" for Windows' own key
// code. 0 if it isn't one.
DWORD KeyByName(std::wstring const& text) {
    std::wstring n = Lower(Trimmed(text));
    if (n.size() == 1) {
        wchar_t c = n[0];
        if ((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9')) return (DWORD)towupper(c);
        switch (c) {
            case L'-': return VK_OEM_MINUS;
            case L'=': return VK_OEM_PLUS;
            case L',': return VK_OEM_COMMA;
            case L'.': return VK_OEM_PERIOD;
            case L'/': return VK_OEM_2;
            case L';': return VK_OEM_1;
            case L'\'': return VK_OEM_7;
            case L'`': return VK_OEM_3;
            case L'[': return VK_OEM_4;
            case L']': return VK_OEM_6;
            case L'\\': return VK_OEM_5;
        }
        return 0;
    }
    if (n.rfind(L"code:", 0) == 0) {
        int code = _wtoi(n.c_str() + 5);
        return code > 0 && code < 0xFF ? (DWORD)code : 0;
    }
    if (n.size() <= 3 && n[0] == L'f' && iswdigit(n[1])) {
        int f = _wtoi(n.c_str() + 1);
        return f >= 1 && f <= 24 ? (DWORD)(VK_F1 + f - 1) : 0;
    }
    static const std::pair<PCWSTR, DWORD> kNames[] = {
        {L"space", VK_SPACE}, {L"tab", VK_TAB}, {L"return", VK_RETURN}, {L"enter", VK_RETURN},
        {L"escape", VK_ESCAPE}, {L"esc", VK_ESCAPE}, {L"backspace", VK_BACK}, {L"delete", VK_DELETE},
        {L"insert", VK_INSERT}, {L"home", VK_HOME}, {L"end", VK_END}, {L"prior", VK_PRIOR},
        {L"pageup", VK_PRIOR}, {L"next", VK_NEXT}, {L"pagedown", VK_NEXT}, {L"left", VK_LEFT},
        {L"right", VK_RIGHT}, {L"up", VK_UP}, {L"down", VK_DOWN}, {L"print", VK_SNAPSHOT},
        {L"pause", VK_PAUSE}, {L"menu", VK_APPS}, {L"minus", VK_OEM_MINUS}, {L"equal", VK_OEM_PLUS},
        {L"comma", VK_OEM_COMMA}, {L"period", VK_OEM_PERIOD}, {L"slash", VK_OEM_2},
        {L"semicolon", VK_OEM_1}, {L"apostrophe", VK_OEM_7}, {L"grave", VK_OEM_3},
        {L"bracketleft", VK_OEM_4}, {L"bracketright", VK_OEM_6}, {L"backslash", VK_OEM_5},
        {L"xf86audioraisevolume", VK_VOLUME_UP}, {L"xf86audiolowervolume", VK_VOLUME_DOWN},
        {L"xf86audiomute", VK_VOLUME_MUTE}, {L"xf86audioplay", VK_MEDIA_PLAY_PAUSE},
        {L"xf86audiopause", VK_MEDIA_PLAY_PAUSE}, {L"xf86audionext", VK_MEDIA_NEXT_TRACK},
        {L"xf86audioprev", VK_MEDIA_PREV_TRACK}, {L"xf86audiostop", VK_MEDIA_STOP},
        // Modifiers as the key: tapped on their own (no side named: either side).
        {L"super_l", VK_LWIN}, {L"super_r", VK_RWIN}, {L"lwin", VK_LWIN}, {L"rwin", VK_RWIN},
        {L"shift_l", VK_LSHIFT}, {L"shift_r", VK_RSHIFT}, {L"lshift", VK_LSHIFT}, {L"rshift", VK_RSHIFT},
        {L"control_l", VK_LCONTROL}, {L"control_r", VK_RCONTROL}, {L"lctrl", VK_LCONTROL}, {L"rctrl", VK_RCONTROL},
        {L"alt_l", VK_LMENU}, {L"alt_r", VK_RMENU}, {L"lalt", VK_LMENU}, {L"ralt", VK_RMENU},
        {L"win", VK_LWIN | 0x100}, {L"super", VK_LWIN | 0x100}, {L"shift", VK_SHIFT}, {L"ctrl", VK_CONTROL},
        {L"control", VK_CONTROL}, {L"alt", VK_MENU},
    };
    for (auto const& [name, vk] : kNames)
        if (n == name) return vk;
    return 0;
}

// The modifiers, and any other keys named with them (ESCAPE in "ESCAPE, END": held, as escape&end), into keys.
UINT ModsAndKeysByName(std::wstring const& text, std::vector<DWORD>* keys, bool* ok) {
    UINT mods = 0;
    *ok = true;
    std::wstring part;
    auto take = [&] {
        std::wstring p = part;
        part.clear();
        if (p.empty()) return;
        bool one = false;
        UINT m = ModsByName(p, &one);
        if (one) {
            mods |= m;
            return;
        }
        if (DWORD vk = KeyByName(p)) keys->push_back(vk);
        else *ok = false;
    };
    for (wchar_t c : text) {
        if (c == L' ' || c == L'_' || c == L'+' || c == L'\t' || c == L'&') take();
        else part += c;
    }
    take();
    return mods;
}

// The text split at its first commas (at most parts - 1 of them: the last
// part keeps the rest), each trimmed.
std::vector<std::wstring> SplitArgs(std::wstring const& text, size_t parts) {
    std::vector<std::wstring> out;
    size_t at = 0;
    while (out.size() + 1 < parts) {
        size_t comma = text.find(L',', at);
        if (comma == std::wstring::npos) break;
        out.push_back(Trimmed(text.substr(at, comma - at)));
        at = comma + 1;
    }
    out.push_back(Trimmed(text.substr(std::min(at, text.size()))));
    return out;
}

// The lines read into binds; lines it can't read are logged and left out.
BindTable ReadBinds(std::vector<std::wstring> const& lines) {
    BindTable t;
    std::vector<std::pair<std::wstring, std::wstring>> vars;  // longest name first, when used
    int submap = 0;
    for (std::wstring line : lines) {
        line = Trimmed(line);
        if (line.empty() || line[0] == L'#') continue;
        // $name = value, and $name used after it
        if (line[0] == L'$') {
            size_t eq = line.find(L'=');
            if (eq != std::wstring::npos) {
                vars.push_back({Trimmed(line.substr(0, eq)), Trimmed(line.substr(eq + 1))});
                std::sort(vars.begin(), vars.end(), [](auto const& a, auto const& b) { return a.first.size() > b.first.size(); });
            }
            continue;
        }
        for (auto const& [name, value] : vars)
            for (size_t at = line.find(name); at != std::wstring::npos; at = line.find(name, at + value.size()))
                line.replace(at, name.size(), value);
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) {
            Wh_Log(L"binds: not a line I can read: %s", line.c_str());
            continue;
        }
        std::wstring keyword = Lower(Trimmed(line.substr(0, eq))), rest = Trimmed(line.substr(eq + 1));
        if (keyword == L"submap") {
            std::wstring name = Lower(rest);
            if (name.empty() || name == L"reset") {
                submap = 0;
                continue;
            }
            auto it = std::find(t.submaps.begin(), t.submaps.end(), name);
            submap = (int)(it - t.submaps.begin());
            if (it == t.submaps.end()) t.submaps.push_back(name);
            continue;
        }
        if (keyword == L"idle") {
            auto parts = SplitArgs(rest, 3);
            int seconds = _wtoi(parts[0].c_str());
            if (seconds > 0 && parts.size() > 1)
                t.idle.push_back({seconds, Lower(parts[1]), parts.size() > 2 ? parts[2] : L""});
            else
                Wh_Log(L"binds: idle needs seconds and a command: %s", line.c_str());
            continue;
        }
        if (keyword == L"windowrule" || keyword == L"windowrulev2") {
            t.ruleLines.push_back(rest);
            continue;
        }
        bool unbind = keyword == L"unbind";
        if (!unbind && keyword.rfind(L"bind", 0) != 0) {
            Wh_Log(L"binds: not a bind: %s", line.c_str());
            continue;
        }
        Bind b;
        b.submap = submap;
        b.line = line;
        bool mouse = false, described = false;
        for (wchar_t f : unbind ? std::wstring() : keyword.substr(4)) {
            if (f == L'e') b.repeat = true;
            else if (f == L'r') b.release = true;
            else if (f == L'l') b.locked = true;
            else if (f == L'n') b.passThrough = true;
            else if (f == L'd') described = true;  // bindd: a description after the key
            else if (f == L'm') mouse = true;
            else if (f == L'k') b.twice = true;
            else if (f == L'o') b.holdMs = -1;  // (its time follows, or the default)
            else if (iswdigit(f) && b.holdMs) b.holdMs = (b.holdMs < 0 ? 0 : b.holdMs) * 10 + (f - L'0');
            // (others, like i, t, s, have no meaning here and are ignored)
        }
        if (b.holdMs < 0) b.holdMs = g_holdDefaultMs;
        if (b.holdMs) b.holdMs = std::clamp(b.holdMs, 100, 10000);
        auto parts = SplitArgs(rest, described ? 5 : 4);
        if (described && parts.size() > 3) {
            b.description = parts[2];
            parts.erase(parts.begin() + 2);
        }
        if (mouse) {
            bool modsOk = false;
            UINT mods = parts.size() >= 3 ? ModsByName(parts[0], &modsOk) : 0;
            std::wstring button = parts.size() >= 3 ? Lower(Trimmed(parts[1])) : L"", what = parts.size() >= 3 ? Lower(Trimmed(parts[2])) : L"";
            int which = button == L"mouse:272" ? 0 : button == L"mouse:273" ? 1 : button == L"mouse:274" ? 2 : -1;
            if (!modsOk || !mods || which < 0 || (what != L"movewindow" && what != L"resizewindow")) {
                Wh_Log(L"binds: a mouse bind is MODS, mouse:272/273/274, movewindow/resizewindow (with a modifier): %s", line.c_str());
                continue;
            }
            t.mouseBinds.push_back({mods, which, what == L"resizewindow", b.description, line});
            continue;
        }
        if (parts.size() < (unbind ? 2u : 3u)) {
            Wh_Log(L"binds: needs mods, a key and a command: %s", line.c_str());
            continue;
        }
        bool modsOk = false;
        b.mods = ModsAndKeysByName(parts[0], &b.chord, &modsOk);
        if (Lower(parts[1]) == L"catchall") {
            b.catchAll = true;
        } else {
            // escape&end: the keys before the last are held with it
            std::wstring keys = parts[1];
            for (size_t at = 0;;) {
                size_t amp = keys.find(L'&', at);
                DWORD vk = KeyByName(keys.substr(at, amp == std::wstring::npos ? amp : amp - at));
                if (!vk) {
                    b.vk = 0;
                    break;
                }
                if (amp == std::wstring::npos) {
                    b.vk = vk;
                    break;
                }
                b.chord.push_back(vk);
                at = amp + 1;
            }
        }
        if (!modsOk || (!b.vk && !b.catchAll)) {
            Wh_Log(L"binds: unknown modifier or key: %s", line.c_str());
            continue;
        }
        if (unbind) {
            std::erase_if(t.binds, [&](Bind const& o) {
                return o.mods == b.mods && o.vk == b.vk && o.catchAll == b.catchAll && o.chord == b.chord;
            });
            continue;
        }
        b.dispatcher = Lower(parts[2]);
        b.arg = parts.size() > 3 ? parts[3] : L"";
        t.binds.push_back(std::move(b));
    }
    return t;
}

std::vector<std::wstring> ReadBindLines() {
    std::vector<std::wstring> lines;
    int empty = 0;
    for (int i = 0; i < 1024 && empty < 8; i++) {
        PCWSTR value = Wh_GetStringSetting(L"binds.lines[%d]", i);
        std::wstring line = value ? value : L"";
        Wh_FreeStringSetting(value);
        empty = line.empty() ? empty + 1 : 0;
        lines.push_back(line);
    }
    // And a file of them, written as hyprland.conf's binds are.
    std::wstring file = TextSetting(L"binds.file");
    if (!file.empty()) {
        WCHAR path[MAX_PATH * 2] = L"";
        ExpandEnvironmentStringsW(file.c_str(), path, ARRAYSIZE(path));
        HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h != INVALID_HANDLE_VALUE) {
            std::string bytes;
            char buf[8192];
            DWORD got = 0;
            while (ReadFile(h, buf, sizeof(buf), &got, nullptr) && got && bytes.size() < (1 << 20)) bytes.append(buf, got);
            CloseHandle(h);
            if (bytes.rfind("\xEF\xBB\xBF", 0) == 0) bytes.erase(0, 3);
            int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
            std::wstring text(n, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), text.data(), n);
            size_t at = 0;
            while (at <= text.size()) {
                size_t end = text.find(L'\n', at);
                lines.push_back(text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at));
                if (end == std::wstring::npos) break;
                at = end + 1;
            }
        } else {
            Wh_Log(L"binds: can't read %s", path);
        }
    }
    // Plugins' hotkeys (Tourne Desktop's plugin runtime writes them as bind lines with the plugin dispatcher).
    HKEY key = nullptr;
    if (!RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Desktop", 0, KEY_READ, &key)) {
        DWORD type = 0, size = 0;
        if (!RegQueryValueExW(key, L"PluginBinds", nullptr, &type, nullptr, &size) && type == REG_MULTI_SZ && size) {
            std::wstring all(size / sizeof(wchar_t) + 1, L'\0');
            if (!RegQueryValueExW(key, L"PluginBinds", nullptr, nullptr, (BYTE*)all.data(), &size)) {
                lines.push_back(L"submap = reset");
                for (const wchar_t* p = all.c_str(); *p; p += wcslen(p) + 1) lines.push_back(p);
            }
        }
        RegCloseKey(key);
    }
    return lines;
}

////////////////////////////////////////////////////////////////////////////////
// The hook (on the hook thread)

bool IsModifierKey(DWORD vk) {
    switch (vk) {
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
        case VK_MENU: case VK_LMENU: case VK_RMENU: case VK_LWIN: case VK_RWIN:
            return true;
    }
    return false;
}

// A modifier tapped on its own (hook thread): the one down with nothing else, and since when (0: none).
DWORD g_loneVk = 0;
ULONGLONG g_loneAt = 0;

// A bind on this modifier tapped alone: its index, or -1. Its modifiers may be none or the key's own.
int LoneBind(DWORD vk) {
    UINT own = vk == VK_LWIN || vk == VK_RWIN ? MOD_WIN : vk == VK_LMENU || vk == VK_RMENU ? MOD_ALT
             : vk == VK_LCONTROL || vk == VK_RCONTROL ? MOD_CONTROL : MOD_SHIFT;
    DWORD either = own == MOD_WIN ? (VK_LWIN | 0x100) : own == MOD_ALT ? VK_MENU : own == MOD_CONTROL ? VK_CONTROL : VK_SHIFT;
    std::lock_guard<std::mutex> lock(g_bindsLock);
    for (int i = 0; i < (int)g_binds.binds.size(); i++) {
        Bind const& b = g_binds.binds[i];
        if (b.submap != g_submap || b.catchAll || b.twice || b.holdMs || !b.chord.empty()) continue;
        if ((b.vk != vk && b.vk != either) || (b.mods && b.mods != own)) continue;
        if (!b.locked && ForegroundIsFullscreen()) continue;
        return i;
    }
    return -1;
}

// A modifier's press or release: a tap on its own runs its bind (-1: passed on; 1: kept back).
int LoneModifierHook(bool down, DWORD vk) {
    if (down) {
        if (g_loneVk == vk) return -1;  // (repeating while held)
        bool others = !g_keysDown.empty();
        for (int other : {VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL, VK_LMENU, VK_RMENU, VK_LWIN, VK_RWIN})
            if ((DWORD)other != vk && GetAsyncKeyState(other) < 0) others = true;
        g_loneVk = others ? 0 : vk;
        g_loneAt = GetTickCount64();
        return -1;
    }
    if (g_loneVk != vk) return -1;
    g_loneVk = 0;
    if (GetTickCount64() - g_loneAt > (ULONGLONG)g_holdDefaultMs) return -1;
    int bind = LoneBind(vk);
    if (bind < 0 || !g_wnd) return -1;
    PostMessageW(g_wnd, WM_APP_BIND, bind, g_bindsGeneration);
    if (vk != VK_LWIN && vk != VK_RWIN && vk != VK_LMENU && vk != VK_RMENU) return -1;
    // Win or Alt let go alone opens Start or the menu bar: the release kept back, a key nothing uses pressed,
    // then the release sent on (marked as the binds' own, so it passes here).
    INPUT in[3] = {};
    for (auto& i : in) i.type = INPUT_KEYBOARD, i.ki.dwExtraInfo = kBindReplayTag;
    in[0].ki.wVk = in[1].ki.wVk = kDummyKey;
    in[1].ki.dwFlags = KEYEVENTF_KEYUP;
    in[2].ki.wVk = (WORD)vk;
    in[2].ki.dwFlags = KEYEVENTF_KEYUP | (vk == VK_RMENU || vk == VK_RWIN || vk == VK_LWIN ? KEYEVENTF_EXTENDEDKEY : 0);
    SendInput(3, in, sizeof(INPUT));
    return 1;
}

// -1: not the binds' business; 1: the key is kept back.
int BindsKeyHook(WPARAM wParam, KBDLLHOOKSTRUCT const* k) {
    if (!g_bindsOn || k->dwExtraInfo == kBindReplayTag) return -1;
    bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    DWORD vk = k->vkCode;
    if (IsModifierKey(vk)) return LoneModifierHook(down, vk);
    if (down) g_loneVk = 0;  // (another key pressed: no lone tap)
    // The keys down now, for combinations (escape&end). A repeat is a key already down; a key whose release
    // was never seen (taken by another program's hook) is dropped once Windows says it's up.
    bool again = std::find(g_keysDown.begin(), g_keysDown.end(), vk) != g_keysDown.end();
    if (down) {
        std::erase_if(g_keysDown, [&](DWORD other) { return other != vk && GetAsyncKeyState((int)other) >= 0; });
        if (!again) g_keysDown.push_back(vk);
    } else {
        std::erase(g_keysDown, vk);
    }
    auto held = std::find_if(g_bindKeysHeld.begin(), g_bindKeysHeld.end(), [&](HeldBind const& h) { return h.vk == vk; });
    if (!down) {
        // A hold bind's key let go before its time: a plain press after all (WM_APP_BINDTAP).
        if (g_holdVk == vk) {
            int hold = g_holdBind.exchange(-1);
            g_holdVk = 0;
            if (hold >= 0 && !g_holdRan && g_wnd) PostMessageW(g_wnd, WM_APP_BINDTAP, hold, g_bindsGeneration);
        }
        if (held == g_bindKeysHeld.end()) return -1;
        HeldBind h = *held;
        g_bindKeysHeld.erase(held);
        if (h.runOnRelease >= 0 && g_wnd) PostMessageW(g_wnd, WM_APP_BIND, h.runOnRelease, g_bindsGeneration);
        return h.passThrough ? -1 : 1;
    }
    UINT mods = 0;
    if (GetAsyncKeyState(VK_SHIFT) < 0) mods |= MOD_SHIFT;
    if (GetAsyncKeyState(VK_CONTROL) < 0) mods |= MOD_CONTROL;
    if (k->flags & LLKHF_ALTDOWN) mods |= MOD_ALT;
    if (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0) mods |= MOD_WIN;
    // The binds for this key with these modifiers, the ones naming the most held keys first: a plain press, a
    // double press and a hold may share a key.
    int plain = -1, twice = -1, hold = -1;
    bool repeat = false, release = false, passThrough = false, twicePasses = false;
    {
        std::lock_guard<std::mutex> lock(g_bindsLock);
        int submap = g_submap, catchAll = -1;
        size_t most = 0;
        bool any = false;
        for (int i = 0; i < (int)g_binds.binds.size(); i++) {
            Bind const& b = g_binds.binds[i];
            if (b.submap != submap || b.mods != mods) continue;
            if (b.catchAll) {
                if (catchAll < 0) catchAll = i;
                continue;
            }
            if (b.vk != vk) continue;
            bool chordHeld = std::all_of(b.chord.begin(), b.chord.end(), [](DWORD c) {
                return std::find(g_keysDown.begin(), g_keysDown.end(), c) != g_keysDown.end();
            });
            if (!chordHeld || (any && b.chord.size() < most)) continue;
            if (!any || b.chord.size() > most) plain = twice = hold = -1;
            any = true;
            most = b.chord.size();
            int& slot = b.twice ? twice : b.holdMs ? hold : plain;
            if (slot < 0) slot = i;
        }
        if (!any) plain = catchAll;
        auto usable = [&](int& i) {
            if (i >= 0 && !g_binds.binds[i].locked && ForegroundIsFullscreen()) i = -1;
        };
        usable(plain), usable(twice), usable(hold);
        if (plain >= 0) {
            Bind const& b = g_binds.binds[plain];
            repeat = b.repeat, release = b.release, passThrough = b.passThrough;
        }
        if (twice >= 0) twicePasses = g_binds.binds[twice].passThrough;
    }
    if (again) {
        // The key repeating while held: kept back if its press was, and a repeating bind runs again.
        if (held == g_bindKeysHeld.end()) return -1;
        if (plain >= 0 && repeat && !release && !held->passThrough && g_holdBind < 0 && g_wnd)
            PostMessageW(g_wnd, WM_APP_BIND, plain, g_bindsGeneration);
        return held->passThrough ? -1 : 1;
    }
    if (held != g_bindKeysHeld.end()) g_bindKeysHeld.erase(held);  // (its release was missed)
    // A double press: the second press within the time runs it; the first goes on as below.
    if (twice >= 0) {
        ULONGLONG now = GetTickCount64();
        auto last = g_lastPress.find(twice);
        if (last != g_lastPress.end() && last->second.first == g_bindsGeneration && now - last->second.second <= (ULONGLONG)g_doubleMs) {
            g_lastPress.erase(last);
            g_bindKeysHeld.push_back({vk, -1, twicePasses});
            if (g_wnd) PostMessageW(g_wnd, WM_APP_BIND, twice, g_bindsGeneration);
            if (!twicePasses && (mods & (MOD_WIN | MOD_ALT))) PressDummyKey();
            return twicePasses ? -1 : 1;
        }
        g_lastPress[twice] = {g_bindsGeneration.load(), now};
    }
    // A hold: the key is kept back; held long enough, the bind runs (the switcher window's timer); let go
    // sooner, it's a plain press (WM_APP_BINDTAP: the plain bind, or the key itself).
    if (hold >= 0) {
        g_holdRan = false;
        g_holdVk = vk;
        g_holdBind = hold;
        g_bindKeysHeld.push_back({vk, -1, false});
        if (g_wnd) PostMessageW(g_wnd, WM_APP_BINDHOLD, hold, g_bindsGeneration);
        if (mods & (MOD_WIN | MOD_ALT)) PressDummyKey();
        return 1;
    }
    if (plain < 0) return -1;
    g_bindKeysHeld.push_back({vk, release ? plain : -1, passThrough});
    if (!release && g_wnd) PostMessageW(g_wnd, WM_APP_BIND, plain, g_bindsGeneration);
    // Win or Alt let go after a key kept back would open Start or the
    // program's menu bar: a key nothing uses is pressed between.
    if (!passThrough && (mods & (MOD_WIN | MOD_ALT))) PressDummyKey();
    return passThrough ? -1 : 1;
}

// Mouse binds, on the hook thread. The mouse hook is only there while a mouse
// bind is (or the wheel is wanted): a click without the modifiers goes on at once.
std::atomic<bool> g_mouseBindsOn{false};
int g_mouseDragButton = -1;               // the button dragging a window (hook thread)
std::atomic<bool> g_mouseDragPosted{false};  // a "moved" not yet handled: no more are sent till it is

int MouseBindsHook(WPARAM wParam, MSLLHOOKSTRUCT const* m) {
    if (!g_mouseBindsOn || (m->flags & LLMHF_INJECTED)) return -1;
    if (wParam == WM_MOUSEMOVE) {
        if (g_mouseDragButton >= 0 && g_wnd && !g_mouseDragPosted.exchange(true)) PostMessageW(g_wnd, WM_APP_MOUSEDRAG, 1, 0);
        return -1;
    }
    int button = -1;
    bool down = false;
    switch (wParam) {
        case WM_LBUTTONDOWN: down = true; [[fallthrough]];
        case WM_LBUTTONUP: button = 0; break;
        case WM_RBUTTONDOWN: down = true; [[fallthrough]];
        case WM_RBUTTONUP: button = 1; break;
        case WM_MBUTTONDOWN: down = true; [[fallthrough]];
        case WM_MBUTTONUP: button = 2; break;
    }
    if (button < 0) return -1;
    if (!down) {
        if (button != g_mouseDragButton) return -1;
        g_mouseDragButton = -1;
        if (g_wnd) PostMessageW(g_wnd, WM_APP_MOUSEDRAG, 2, 0);
        return 1;  // (its press was kept back too)
    }
    if (g_mouseDragButton >= 0) return -1;
    UINT mods = 0;
    if (GetAsyncKeyState(VK_SHIFT) < 0) mods |= MOD_SHIFT;
    if (GetAsyncKeyState(VK_CONTROL) < 0) mods |= MOD_CONTROL;
    if (GetAsyncKeyState(VK_MENU) < 0) mods |= MOD_ALT;
    if (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0) mods |= MOD_WIN;
    if (!mods) return -1;
    int resize = -1;
    {
        std::lock_guard<std::mutex> lock(g_bindsLock);
        for (auto const& b : g_binds.mouseBinds)
            if (b.mods == mods && b.button == button) {
                resize = b.resize;
                break;
            }
    }
    if (resize < 0 || ForegroundIsFullscreen() || !g_wnd) return -1;
    g_mouseDragButton = button;
    g_mouseDragPosted = false;
    PostMessageW(g_wnd, WM_APP_MOUSEDRAG, 0, resize);
    if (mods & (MOD_WIN | MOD_ALT)) PressDummyKey();  // (letting go of Win would open Start)
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Windows, for the commands

// A window a person would switch to: in view on this desktop, an app's own.
bool BindWindow(HWND h) {
    if (!h || !IsWindowVisible(h) || GetAncestor(h, GA_ROOT) != h) return false;
    LONG ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((ex & WS_EX_TOOLWINDOW) || ((ex & WS_EX_NOACTIVATE) && !(ex & WS_EX_APPWINDOW))) return false;
    if (GetWindow(h, GW_OWNER) && !(ex & WS_EX_APPWINDOW)) return false;
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    for (PCWSTR shell : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW"})
        if (!wcscmp(cls, shell)) return false;
    if (!wcsncmp(cls, L"Tourne", 6)) return false;
    return GetWindowTextLengthW(h) > 0;
}

// Every such window, front first.
std::vector<HWND> BindWindows() {
    std::vector<HWND> out;
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT))
        if (BindWindow(h)) out.push_back(h);
    return out;
}

HWND ActiveWindow() {
    HWND fg = GetForegroundWindow();
    return fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
}

std::wstring ClassOf(HWND h) {
    WCHAR cls[256] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    return cls;
}

std::wstring FileNameOf(HWND h) {
    std::wstring program = ProgramOf(h);
    return program.substr(program.find_last_of(L'\\') + 1);
}

// The window under the pointer: an app's own window (its owner, for one of its popups), not the desktop, a bar
// or anything else of Tourne's.
HWND HoveredWindow() {
    POINT pt;
    if (!GetCursorPos(&pt)) return nullptr;
    HWND h = WindowFromPoint(pt);
    h = h ? GetAncestor(h, GA_ROOTOWNER) : nullptr;
    if (!h || h == GetShellWindow()) return nullptr;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return nullptr;
    std::wstring cls = ClassOf(h);
    for (PCWSTR shell : {L"Progman", L"WorkerW", L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"})
        if (cls == shell) return nullptr;
    return h;
}

// Hyprland's window argument: class:..., title:..., exe:... (or
// initialclass:, initialtitle:, pid:, address:0x...), activewindow; * and ?
// match any text. Without a prefix, the title. Tourne's own: hovered (the window under the pointer).
HWND WindowBy(std::wstring spec) {
    spec = Trimmed(spec);
    if (spec.empty() || Lower(spec) == L"activewindow") return ActiveWindow();
    if (Lower(spec) == L"hovered" || Lower(spec) == L"mouse") return HoveredWindow();
    size_t colon = spec.find(L':');
    std::wstring kind = colon == std::wstring::npos ? L"title" : Lower(spec.substr(0, colon));
    std::wstring want = colon == std::wstring::npos ? spec : Trimmed(spec.substr(colon + 1));
    if (kind == L"address") return (HWND)(uintptr_t)wcstoull(want.c_str(), nullptr, 16);
    for (HWND h : BindWindows()) {
        if (kind == L"class" || kind == L"initialclass") {
            if (MatchesList(want, ClassOf(h))) return h;
        } else if (kind == L"title" || kind == L"initialtitle") {
            if (MatchesList(want, Title(h))) return h;
        } else if (kind == L"exe" || kind == L"program") {
            if (MatchesList(want, FileNameOf(h))) return h;
        } else if (kind == L"pid") {
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            if (pid == (DWORD)_wtoi(want.c_str())) return h;
        }
    }
    return nullptr;
}

int DirectionByName(std::wstring const& text) {
    std::wstring d = Lower(Trimmed(text));
    if (d == L"l" || d == L"left") return kLeft;
    if (d == L"r" || d == L"right") return kRight;
    if (d == L"u" || d == L"t" || d == L"up" || d == L"top") return kUp;
    if (d == L"d" || d == L"b" || d == L"down" || d == L"bottom") return kDown;
    return -1;
}

POINT Middle(RECT const& r) {
    return {(r.left + r.right) / 2, (r.top + r.bottom) / 2};
}

bool LiesThatWay(POINT a, POINT b, int direction) {
    int dx = b.x - a.x, dy = b.y - a.y;
    return direction == kLeft  ? dx < 0 && std::abs(dx) >= std::abs(dy) / 2
         : direction == kRight ? dx > 0 && std::abs(dx) >= std::abs(dy) / 2
         : direction == kUp    ? dy < 0 && std::abs(dy) >= std::abs(dx) / 2
                               : dy > 0 && std::abs(dy) >= std::abs(dx) / 2;
}

// The window in view nearest that way from this one (on any screen): one
// whose middle is its own, not covered by another.
HWND WindowThatWay(HWND from, int direction) {
    POINT a = Middle(FrameOf(from));
    HWND best = nullptr;
    long long bestDistance = 0;
    for (HWND h : BindWindows()) {
        if (h == from || IsIconic(h)) continue;
        POINT b = Middle(FrameOf(h));
        HWND there = WindowFromPoint(b);
        if (!there || GetAncestor(there, GA_ROOT) != h || !LiesThatWay(a, b, direction)) continue;
        long long distance = (long long)(b.x - a.x) * (b.x - a.x) + (long long)(b.y - a.y) * (b.y - a.y);
        if (!best || distance < bestDistance) best = h, bestDistance = distance;
    }
    return best;
}

std::vector<HMONITOR> Monitors() {
    std::vector<HMONITOR> out;
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM p) __stdcall -> BOOL {
            ((std::vector<HMONITOR>*)p)->push_back(m);
            return TRUE;
        },
        (LPARAM)&out);
    std::sort(out.begin(), out.end(), [](HMONITOR a, HMONITOR b) {
        MONITORINFO ma{sizeof(ma)}, mb{sizeof(mb)};
        GetMonitorInfoW(a, &ma);
        GetMonitorInfoW(b, &mb);
        return ma.rcMonitor.left != mb.rcMonitor.left ? ma.rcMonitor.left < mb.rcMonitor.left : ma.rcMonitor.top < mb.rcMonitor.top;
    });
    return out;
}

// Hyprland's monitor argument: l/r/u/d, +1/-1, or a number from 1 (left to
// right).
HMONITOR MonitorBy(std::wstring const& text, HMONITOR from) {
    std::vector<HMONITOR> all = Monitors();
    if (all.empty()) return nullptr;
    int at = (int)(std::find(all.begin(), all.end(), from) - all.begin());
    if (at >= (int)all.size()) at = 0;
    std::wstring t = Lower(Trimmed(text));
    if (t.rfind(L"mon:", 0) == 0) t = t.substr(4);
    if (int direction = DirectionByName(t); direction >= 0) {
        MONITORINFO mf{sizeof(mf)};
        GetMonitorInfoW(from, &mf);
        POINT a = Middle(mf.rcMonitor);
        HMONITOR best = nullptr;
        long long bestDistance = 0;
        for (HMONITOR m : all) {
            if (m == from) continue;
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(m, &mi);
            POINT b = Middle(mi.rcMonitor);
            if (!LiesThatWay(a, b, direction)) continue;
            long long distance = (long long)(b.x - a.x) * (b.x - a.x) + (long long)(b.y - a.y) * (b.y - a.y);
            if (!best || distance < bestDistance) best = m, bestDistance = distance;
        }
        return best;
    }
    int n = (int)all.size();
    if (!t.empty() && (t[0] == L'+' || t[0] == L'-')) return all[((at + _wtoi(t.c_str())) % n + n) % n];
    int index = _wtoi(t.c_str());
    return index >= 1 && index <= n ? all[index - 1] : nullptr;
}

// A window put at a place given by its visible frame.
void PlaceWindowFrame(HWND h, RECT const& want) {
    if (IsZoomed(h)) ShowWindow(h, SW_RESTORE);
    RECT frame = FrameOf(h), outer{};
    GetWindowRect(h, &outer);
    // The invisible resize borders, kept out of the frame given.
    RECT r{want.left - (frame.left - outer.left), want.top - (frame.top - outer.top), want.right + (outer.right - frame.right),
           want.bottom + (outer.bottom - frame.bottom)};
    SetWindowPos(h, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

// The window on another screen, at the same place relative to its work area.
void SendToMonitor(HWND h, HMONITOR to) {
    HMONITOR from = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
    if (!to || to == from) return;
    MONITORINFO mf{sizeof(mf)}, mt{sizeof(mt)};
    GetMonitorInfoW(from, &mf);
    GetMonitorInfoW(to, &mt);
    bool zoomed = IsZoomed(h);
    if (zoomed) ShowWindow(h, SW_RESTORE);
    RECT f = FrameOf(h), a = mf.rcWork, b = mt.rcWork;
    auto scale = [](int v, int a0, int a1, int b0, int b1) { return a1 == a0 ? b0 : b0 + MulDiv(v - a0, b1 - b0, a1 - a0); };
    int w = std::min<int>(f.right - f.left, b.right - b.left), hgt = std::min<int>(f.bottom - f.top, b.bottom - b.top);
    int x = std::clamp(scale(f.left, a.left, a.right, b.left, b.right), (int)b.left, (int)b.right - w);
    int y = std::clamp(scale(f.top, a.top, a.bottom, b.top, b.bottom), (int)b.top, (int)b.bottom - hgt);
    PlaceWindowFrame(h, {x, y, x + w, y + hgt});
    if (zoomed) ShowWindow(h, SW_MAXIMIZE);
}

// "x y" as numbers (px, or % of the screen with %).
bool TwoNumbers(std::wstring const& text, HMONITOR monitor, int* x, int* y) {
    std::wstring t = Trimmed(text);
    for (auto& c : t)
        if (c == L',') c = L' ';
    WCHAR a[32] = L"", b[32] = L"";
    if (swscanf(t.c_str(), L"%31s %31s", a, b) != 2) return false;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    auto value = [](PCWSTR v, int whole) { return wcschr(v, L'%') ? (int)(_wtof(v) * whole / 100) : _wtoi(v); };
    *x = value(a, mi.rcWork.right - mi.rcWork.left);
    *y = value(b, mi.rcWork.bottom - mi.rcWork.top);
    return true;
}

bool IsTiled(HWND h) {
    return std::find(g_tileOrder.begin(), g_tileOrder.end(), h) != g_tileOrder.end();
}

void SendKeys(UINT mods, DWORD vk) {
    std::vector<INPUT> in;
    auto key = [&](WORD k, bool up) {
        INPUT i{INPUT_KEYBOARD};
        i.ki.wVk = k;
        i.ki.dwFlags = up ? KEYEVENTF_KEYUP : 0;
        i.ki.dwExtraInfo = kBindReplayTag;
        in.push_back(i);
    };
    std::vector<WORD> held;
    if (mods & MOD_CONTROL) held.push_back(VK_CONTROL);
    if (mods & MOD_ALT) held.push_back(VK_MENU);
    if (mods & MOD_SHIFT) held.push_back(VK_SHIFT);
    if (mods & MOD_WIN) held.push_back(VK_LWIN);
    for (WORD k : held) key(k, false);
    key((WORD)vk, false);
    key((WORD)vk, true);
    for (auto it = held.rbegin(); it != held.rend(); ++it) key(*it, true);
    SendInput((UINT)in.size(), in.data(), sizeof(INPUT));
}

void ExpectWindowRules(DWORD pid, std::wstring const& rules);

bool RunProgram(std::wstring command) {
    command = Trimmed(command);
    std::wstring rules;  // Hyprland's [rules; ...] before the command: for the program's first window
    if (!command.empty() && command[0] == L'[') {
        size_t close = command.find(L']');
        rules = command.substr(1, close == std::wstring::npos ? std::wstring::npos : close - 1);
        command = Trimmed(close == std::wstring::npos ? L"" : command.substr(close + 1));
    }
    if (command.empty()) return false;
    WCHAR expanded[4096] = L"";
    ExpandEnvironmentStringsW(command.c_str(), expanded, ARRAYSIZE(expanded));
    std::wstring line = expanded, file, params;
    if (line[0] == L'"') {
        size_t close = line.find(L'"', 1);
        file = line.substr(1, close == std::wstring::npos ? std::wstring::npos : close - 1);
        params = close == std::wstring::npos ? L"" : Trimmed(line.substr(close + 1));
    } else {
        size_t space = line.find(L' ');
        file = line.substr(0, space);
        params = space == std::wstring::npos ? L"" : Trimmed(line.substr(space + 1));
    }
    SHELLEXECUTEINFOW info{sizeof(info)};
    info.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI | (rules.empty() ? 0 : SEE_MASK_NOCLOSEPROCESS);
    info.lpFile = file.c_str();
    info.lpParameters = params.empty() ? nullptr : params.c_str();
    info.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExW(&info)) {
        if (info.hProcess) {
            ExpectWindowRules(GetProcessId(info.hProcess), rules);
            CloseHandle(info.hProcess);
        }
        return true;
    }
    // Not a program of its own (a pipe, a shell built-in...): through cmd, unseen.
    std::wstring cmd = L"/d /c " + line;
    info.lpFile = L"cmd.exe";
    info.lpParameters = cmd.c_str();
    info.nShow = SW_HIDE;
    return ShellExecuteExW(&info) != FALSE;
}

void BindsEmit(std::wstring const& event);

// Scratchpads (Hyprland's special workspaces): windows put away, minimised,
// and brought back over whatever desktop you're on, floating in the middle of
// the screen, with togglespecialworkspace. By name; "" is the plain one.
struct Scratchpad {
    std::vector<HWND> windows;
    bool shown = false;
};
std::unordered_map<std::wstring, Scratchpad> g_scratchpads;

void ForgetScratch(HWND h) {
    for (auto& [name, pad] : g_scratchpads) std::erase(pad.windows, h);
}

std::wstring ScratchName(std::wstring const& arg) {
    std::wstring a = Lower(Trimmed(arg));
    return a.rfind(L"special:", 0) == 0 ? a.substr(8) : L"";
}

void ShowScratchpad(Scratchpad& pad, HMONITOR monitor) {
    std::erase_if(pad.windows, [](HWND h) { return !IsWindow(h); });
    std::vector<GUID> ids = VdReadGuids(L"VirtualDesktopIDs");
    int current = VdCurrentIndex(ids);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    RECT work = mi.rcWork;
    int w = (work.right - work.left) * 4 / 5, h = (work.bottom - work.top) * 4 / 5;
    int step = MulDiv(24, MonitorDpi(monitor), 96), i = 0;
    for (HWND win : pad.windows) {
        if (!ids.empty()) VdmSendWindow(win, ids[current], false);  // (here, whichever desktop it was put away on)
        ShowWindow(win, SW_RESTORE);
        int x = work.left + (work.right - work.left - w) / 2 + step * i, y = work.top + (work.bottom - work.top - h) / 2 + step * i;
        PlaceWindowFrame(win, {x, y, x + w, y + h});
        i++;
    }
    if (!pad.windows.empty()) Activate(pad.windows.back());
    pad.shown = true;
}

int g_previousDesktop = -1;  // the desktop Tourne last switched away from ("workspace previous")
bool g_screensOff = false;

// A desktop by Hyprland's workspace argument: a number from 1, +N / -N (or
// r+N, e+N, m+N), previous. -1 if there's none such.
int DesktopBy(std::wstring const& text, int current, int count) {
    std::wstring t = Lower(Trimmed(text));
    if (t == L"previous" || t == L"previous_per_monitor") return g_previousDesktop;
    if (t.size() > 1 && (t[0] == L'r' || t[0] == L'e' || t[0] == L'm')) t = t.substr(1);
    if (!t.empty() && (t[0] == L'+' || t[0] == L'-')) {
        int to = current + _wtoi(t.c_str());
        return to >= 0 && to < count ? to : -1;
    }
    int n = _wtoi(t.c_str());
    return n >= 1 && n <= count ? n - 1 : -1;
}

////////////////////////////////////////////////////////////////////////////////
// The commands (Hyprland's dispatchers), on the switcher's thread. The answer
// is "ok", or what went wrong.

std::wstring GroupCommand(std::wstring const& name, std::wstring const& arg, HWND active);
std::wstring RunCommand(std::wstring const& line);  // several steps with waits between (below)

std::wstring Dispatch(std::wstring const& name, std::wstring const& arg) {
    HWND active = ActiveWindow();
    HMONITOR monitor = active ? MonitorFromWindow(active, MONITOR_DEFAULTTONEAREST) : ActiveMonitor();
    auto needWindow = [&](HWND h) { return h && IsWindow(h) && h != GetShellWindow(); };

    // Several steps in one command, with waits between: "run" is the language's
    // own, and what a menu item, a bar click and tournectl all end up calling.
    if (name == L"run") return RunCommand(arg);

    if (name == L"exec" || name == L"execr") return RunProgram(arg) ? L"ok" : L"error: couldn't start it";

    if (name == L"killactive" || name == L"closewindow") {
        HWND h = name == L"killactive" ? active : WindowBy(arg);
        if (!needWindow(h)) return L"error: no window";
        PostMessageW(h, WM_SYSCOMMAND, SC_CLOSE, 0);
        return L"ok";
    }
    if (name == L"forcekillactive" || name == L"killwindow") {
        HWND h = name == L"forcekillactive" ? active : WindowBy(arg);
        if (!needWindow(h)) return L"error: no window";
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        if (!pid || pid == GetCurrentProcessId() || !_wcsicmp(FileNameOf(h).c_str(), L"explorer.exe"))
            return L"error: not that one";
        HANDLE p = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        bool ok = p && TerminateProcess(p, 1);
        if (p) CloseHandle(p);
        return ok ? L"ok" : L"error: couldn't end it";
    }

    // A plugin's panel (Tourne Desktop's plugin runtime): "author/plugin:panel", opened or closed.
    if (name == L"plugin") {
        HWND host = FindWindowExW(HWND_MESSAGE, nullptr, L"TourneDesktopPlugins", nullptr);
        if (!host) return L"error: the plugins aren't running";
        ATOM atom = GlobalAddAtomW(arg.substr(0, 250).c_str());
        if (!atom || !PostMessageW(host, RegisterWindowMessageW(L"TourneDesktop.PluginPanel"), atom, 0)) {
            if (atom) GlobalDeleteAtom(atom);
            return L"error: couldn't reach the plugins";
        }
        return L"ok";
    }

    // The launcher (Tourne Desktop's launcher.cpp): open, or close when it's open.
    // Tourne's bar or Explorer's own taskbar in its place (shell mode, Explorer underneath): explorer, tourne, or
    // nothing to swap. For when something only Explorer's taskbar does is wanted for a moment.
    if (name == L"explorerbar") {
        HWND host = FindWindowW(L"TourneDesktop_Host", nullptr);
        if (!host) return L"error: Tourne Desktop isn't running";
        std::wstring a = Lower(Trimmed(arg));
        WPARAM to = a == L"explorer" || a == L"on" ? 1 : a == L"tourne" || a == L"off" ? 2 : 0;
        PostMessageW(host, RegisterWindowMessageW(L"TourneDesktop.ExplorerBar"), to, 0);
        return L"ok";
    }

    // One of the system icons' flyouts (the per-app volume mixer for Win+A, the notifications, network...):
    // opened, or closed again if it's open.
    if (name == L"flyout") {
        HWND icons = FindWindowW(L"TourneTraySystemIcons_" WH_MOD_ID, nullptr);
        if (!icons) return L"error: the system icons aren't running";
        static const std::pair<PCWSTR, int> kKinds[] = {{L"network", 1}, {L"wifi", 1}, {L"power", 2}, {L"battery", 2},
            {L"brightness", 3}, {L"vpn", 5}, {L"security", 6}, {L"mixer", 7}, {L"volume", 7}, {L"sound", 7},
            {L"notifications", 8}, {L"notify", 8}, {L"calendar", 8}, {L"bluetooth", 9}, {L"devices", 10}};
        std::wstring a = Lower(Trimmed(arg));
        int kind = 0;
        for (auto const& [n, k] : kKinds)
            if (a == n) kind = k;
        if (!kind) return L"error: which flyout? mixer, notifications, network, power, bluetooth, devices, brightness, vpn, security";
        PostMessageW(icons, RegisterWindowMessageW(L"TourneTray_Flyout"), kind, 0);
        return L"ok";
    }

    // Designer Mode: pick a colour off the screen, click a part of Tourne, choose the setting it goes in.
    if (name == L"designer") {
        HWND host = FindWindowW(L"TourneDesktop_Host", nullptr);
        if (!host) return L"error: Tourne Desktop isn't running";
        PostMessageW(host, RegisterWindowMessageW(L"TourneDesktop.Designer"), 0, 0);
        return L"ok";
    }

    // The look as it is now saved into the look gallery (a pack and its screenshot).
    if (name == L"looksave") {
        HWND host = FindWindowW(L"TourneDesktop_Host", nullptr);
        if (!host) return L"error: Tourne Desktop isn't running";
        PostMessageW(host, RegisterWindowMessageW(L"TourneDesktop.LookSave"), 0, 0);
        return L"ok";
    }

    // Edit desktop mode (the plugins' desktop widgets placed and moved in one go): on, or off again.
    if (name == L"editdesktop") {
        HWND plugins = FindWindowExW(HWND_MESSAGE, nullptr, L"TourneDesktopPlugins", nullptr);
        if (!plugins) return L"error: the plugins aren't running";
        PostMessageW(plugins, RegisterWindowMessageW(L"TourneDesktop.EditDesktop"), 0, 0);
        return L"ok";
    }

    if (name == L"launcher") {
        HWND launcher = FindWindowW(L"TourneLauncher", nullptr);
        if (!launcher) return L"error: the launcher isn't running";
        PostMessageW(launcher, RegisterWindowMessageW(L"TourneDesktop.Launcher"), 0, 0);
        return L"ok";
    }

    if (name == L"togglefloating" || name == L"setfloating" || name == L"settiled") {
        HWND h = arg.empty() ? active : WindowBy(arg);
        if (!needWindow(h)) return L"error: no window";
        bool floating = IsFloating(h);
        if ((name == L"setfloating" && floating) || (name == L"settiled" && !floating)) return L"ok";
        if (h != active) Activate(h);
        TileCommandRun(kTileFloat, 0);
        return L"ok";
    }
    if (name == L"fullscreen" || name == L"maximize") {
        // (Hyprland's fullscreen takes a mode number; a window is named by anything else)
        bool named = !arg.empty() && !iswdigit(Trimmed(arg)[0]);
        HWND h = named ? WindowBy(arg) : active;
        if (!needWindow(h)) return L"error: no window";
        ShowWindow(h, IsZoomed(h) ? SW_RESTORE : SW_MAXIMIZE);
        return L"ok";
    }
    // Every window on the screen tiled, none floating, the main share back to half: an even split down the
    // middle (clear of the taskbar, as the tiling keeps it).
    if (name == L"tileall") {
        HMONITOR screen = active ? MonitorFromWindow(active, MONITOR_DEFAULTTONEAREST) : ActiveMonitor();
        std::erase_if(g_floating, [&](HWND w) { return !IsWindow(w) || MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST) == screen; });
        LayoutChoice& c = ChoiceFor(screen);
        c.ratioStep = 50 - g_tiling.ratio;
        SaveLayoutChoices();
        TileMonitor(screen, true);
        return L"ok";
    }
    // One window with its screen to itself (Monocle for one), and back: taken out of the tiling while it has it.
    // "tileothers" after the window (or alone, for the active one): the screen's other windows tiled behind it.
    if (name == L"monocle") {
        std::wstring spec = Trimmed(arg);
        bool tileOthers = false;
        if (size_t at = Lower(spec).find(L"tileothers"); at != std::wstring::npos) {
            tileOthers = true;
            spec = Trimmed(spec.erase(at, 10));
        }
        HWND h = spec.empty() ? active : WindowBy(spec);
        if (!needWindow(h)) return L"error: no window";
        static struct {
            HWND h = nullptr;
            RECT was{};
            bool tiled = false;
            std::vector<HWND> order;  // the tiling's order before, so it goes back to its own tile
        } g_alone;
        // Back in the tiling at the place it had (a window set floating and back otherwise comes first).
        auto retile = [](HWND w) {
            std::vector<HWND> order;
            for (HWND o : g_alone.order)
                if (IsWindow(o)) order.push_back(o);
            for (HWND o : g_tileOrder)
                if (std::find(order.begin(), order.end(), o) == order.end()) order.push_back(o);
            g_tileOrder = order;
            std::erase(g_floating, w);
            TileMonitor(MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST), false);
        };
        HMONITOR screen = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
        if (g_alone.h == h && IsWindow(h)) {
            g_alone.h = nullptr;
            if (g_alone.tiled) {
                retile(h);
            } else {
                PlaceWindowFrame(h, g_alone.was);
            }
            return L"ok";
        }
        if (g_alone.h && IsWindow(g_alone.h) && g_alone.tiled) retile(g_alone.h);  // (the one before goes back first)
        if (IsZoomed(h) || IsIconic(h)) ShowWindow(h, SW_RESTORE);
        g_alone.h = h;
        g_alone.was = FrameOf(h);
        g_alone.tiled = IsTiled(h);
        g_alone.order = g_tileOrder;
        if (tileOthers)
            std::erase_if(g_floating, [&](HWND w) { return !IsWindow(w) || (w != h && MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST) == screen); });
        if (g_alone.tiled) g_floating.push_back(h);  // (out of the tiling while it has the screen; the others close up)
        if (g_alone.tiled || tileOthers) TileMonitor(screen, false);
        PlaceWindowFrame(h, TileArea(screen));
        SetWindowPos(h, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        return L"ok";
    }
    if (name == L"minimize") {
        // The window under the pointer: pressed again with the pointer where it was, the one it just put away
        // comes back (it's no longer under the pointer to be asked for).
        static HWND s_putAway;
        static POINT s_putAwayAt;
        bool hovered = Lower(Trimmed(arg)) == L"hovered" || Lower(Trimmed(arg)) == L"mouse";
        POINT pt{};
        GetCursorPos(&pt);
        if (hovered && s_putAway && IsWindow(s_putAway) && IsIconic(s_putAway) && std::abs(pt.x - s_putAwayAt.x) <= 8 &&
            std::abs(pt.y - s_putAwayAt.y) <= 8) {
            Activate(s_putAway);
            s_putAway = nullptr;
            return L"ok";
        }
        HWND h = arg.empty() ? active : WindowBy(arg);
        if (!needWindow(h)) return L"error: no window";
        ShowWindow(h, SW_MINIMIZE);
        if (hovered) s_putAway = h, s_putAwayAt = pt;
        return L"ok";
    }
    if (name == L"pin") {
        HWND h = arg.empty() ? active : WindowBy(arg);
        if (!needWindow(h)) return L"error: no window";
        bool top = GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_TOPMOST;
        SetWindowPos(h, top ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        return L"ok";
    }

    if (name == L"movefocus") {
        int direction = DirectionByName(arg);
        if (direction < 0) return L"error: give l, r, u or d";
        if (HWND to = active ? WindowThatWay(active, direction) : nullptr) {
            Activate(to);
            return L"ok";
        }
        if (active && IsTiled(active)) TileCommandRun(kTileFocus, direction);  // (Monocle: the next one)
        return L"ok";
    }
    if (name == L"movewindow" || name == L"swapwindow") {
        if (!needWindow(active)) return L"error: no window";
        int direction = DirectionByName(arg);
        if (direction >= 0 && IsTiled(active)) {
            TileCommandRun(kTileMove, direction);
            return L"ok";
        }
        if (name == L"swapwindow") {
            HWND other = direction >= 0 ? WindowThatWay(active, direction) : nullptr;
            if (!other) return L"error: nothing that way";
            RECT a = FrameOf(active), b = FrameOf(other);
            PlaceWindowFrame(active, b);
            PlaceWindowFrame(other, a);
            return L"ok";
        }
        HMONITOR to = MonitorBy(arg, monitor);
        if (!to) return L"error: no screen that way";
        SendToMonitor(active, to);
        return L"ok";
    }
    if (name == L"resizeactive" || name == L"moveactive" || name == L"resizewindowpixel" || name == L"movewindowpixel") {
        bool resize = name.rfind(L"resize", 0) == 0;
        std::wstring numbers = arg;
        HWND h = active;
        if (name.find(L"pixel") != std::wstring::npos) {
            auto parts = SplitArgs(arg, 2);
            numbers = parts[0];
            h = parts.size() > 1 ? WindowBy(parts[1]) : nullptr;
        }
        if (!needWindow(h)) return L"error: no window";
        bool exact = Lower(numbers).rfind(L"exact", 0) == 0;
        if (exact) numbers = numbers.substr(5);
        int x = 0, y = 0;
        if (!TwoNumbers(numbers, MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST), &x, &y)) return L"error: give two numbers";
        RECT f = FrameOf(h);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST), &mi);
        if (resize) {
            int w = std::max(50, exact ? x : (int)(f.right - f.left) + x), hh = std::max(50, exact ? y : (int)(f.bottom - f.top) + y);
            f.right = f.left + w, f.bottom = f.top + hh;
        } else {
            int dx = exact ? mi.rcWork.left + x - f.left : x, dy = exact ? mi.rcWork.top + y - f.top : y;
            OffsetRect(&f, dx, dy);
        }
        PlaceWindowFrame(h, f);
        return L"ok";
    }
    if (name == L"centerwindow") {
        if (!needWindow(active)) return L"error: no window";
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(monitor, &mi);
        RECT f = FrameOf(active);
        int w = f.right - f.left, h = f.bottom - f.top;
        int x = (mi.rcWork.left + mi.rcWork.right - w) / 2, y = (mi.rcWork.top + mi.rcWork.bottom - h) / 2;
        PlaceWindowFrame(active, {x, y, x + w, y + h});
        return L"ok";
    }

    if (name == L"cyclenext") {
        // The windows on this screen in reading order (left to right, top to bottom): the next, or with
        // prev the one before; tiled or floating to only those.
        std::wstring a = Lower(arg);
        bool prev = a.find(L"prev") != std::wstring::npos, onlyTiled = a.find(L"tiled") != std::wstring::npos,
             onlyFloating = a.find(L"floating") != std::wstring::npos;
        std::vector<HWND> list;
        for (HWND h : BindWindows())
            if (!IsIconic(h) && MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST) == monitor && (!onlyTiled || IsTiled(h)) &&
                (!onlyFloating || !IsTiled(h)))
                list.push_back(h);
        if (list.empty()) return L"error: no window";
        std::sort(list.begin(), list.end(), [](HWND x, HWND y) {
            RECT a = FrameOf(x), b = FrameOf(y);
            return a.left != b.left ? a.left < b.left : a.top != b.top ? a.top < b.top : x < y;
        });
        int at = (int)(std::find(list.begin(), list.end(), active) - list.begin()), n = (int)list.size();
        Activate(list[at >= n ? 0 : (at + (prev ? n - 1 : 1)) % n]);
        return L"ok";
    }
    if (name == L"focuswindow") {
        HWND h = WindowBy(arg);
        if (!h) return L"error: no such window";
        Activate(h);
        return L"ok";
    }
    if (name == L"focuscurrentorlast" || name == L"focusurgentorlast") {
        // The window that was in front before this one: the next in Windows' order.
        for (HWND h : BindWindows())
            if (h != active && !IsIconic(h)) {
                Activate(h);
                return L"ok";
            }
        return L"error: no other window";
    }
    if (name == L"focusmonitor") {
        HMONITOR to = MonitorBy(arg, monitor);
        if (!to) return L"error: no such screen";
        for (HWND h : BindWindows())
            if (!IsIconic(h) && MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST) == to) {
                Activate(h);
                return L"ok";
            }
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(to, &mi);
        POINT m = Middle(mi.rcWork);
        SetCursorPos(m.x, m.y);
        return L"ok";
    }

    if (name == L"workspace" || name == L"movetoworkspace" || name == L"movetoworkspacesilent" || name == L"renameworkspace") {
        std::vector<GUID> ids = VdReadGuids(L"VirtualDesktopIDs");
        int current = VdCurrentIndex(ids), count = std::max(1, (int)ids.size());
        std::wstring which = arg, rest;
        if (name == L"renameworkspace") {
            size_t space = Trimmed(arg).find(L' ');
            which = Trimmed(arg).substr(0, space);
            rest = space == std::wstring::npos ? L"" : Trimmed(Trimmed(arg).substr(space + 1));
        } else if (name != L"workspace") {
            auto parts = SplitArgs(arg, 2);  // "N, window"
            which = parts[0];
            rest = parts.size() > 1 ? parts[1] : L"";
        }
        if (Lower(Trimmed(which)).rfind(L"special", 0) == 0 && name != L"workspace" && name != L"renameworkspace") {
            HWND h = rest.empty() ? active : WindowBy(rest);
            if (!needWindow(h)) return L"error: no window";
            ForgetScratch(h);
            g_scratchpads[ScratchName(which)].windows.push_back(h);
            if (!IsFloating(h)) {
                g_floating.push_back(h);
                if (IsTiled(h)) TileMonitor(monitor, false);
            }
            ShowWindow(h, SW_MINIMIZE);
            return L"ok";
        }
        if (name != L"workspace" && name != L"renameworkspace") ForgetScratch(rest.empty() ? active : WindowBy(rest));
        int to = DesktopBy(which, current, count);
        // "workspace 5" with fewer desktops: made, as Hyprland makes a workspace on its first visit (up to 20).
        int wanted = _wtoi(Trimmed(which).c_str());
        if (to < 0 && name != L"renameworkspace" && wanted > count && wanted <= 20 && iswdigit(Trimmed(which)[0])) {
            if (ids.empty()) ids.resize(1);  // (the one desktop there is: never named in the registry until a second is made)
            while ((int)ids.size() < wanted) {
                GUID made = VdmCreate();
                if (made == GUID_NULL) return L"error: Windows didn't make a desktop";
                ids.push_back(made);
            }
            to = wanted - 1;
        }
        if (to < 0 || ids.empty()) return L"error: no such desktop";
        if (name == L"renameworkspace") return VdmRename(ids[to], rest) ? L"ok" : L"error: Windows didn't rename it";
        if (name == L"workspace") {
            if (to == current) return L"ok";
            if (!VdmSwitchTo(ids[to])) return L"error: Windows didn't switch";
        } else {
            HWND h = rest.empty() ? active : WindowBy(rest);
            if (!needWindow(h)) return L"error: no window";
            bool follow = name == L"movetoworkspace";
            if (!VdmSendWindow(h, ids[to], follow)) return L"error: Windows didn't move it";
            if (!follow) return L"ok";
        }
        g_previousDesktop = current;
        return L"ok";
    }

    if (name == L"togglespecialworkspace") {
        auto it = g_scratchpads.find(Lower(Trimmed(arg)));
        if (it == g_scratchpads.end() || it->second.windows.empty()) return L"error: nothing put away there";
        Scratchpad& pad = it->second;
        if (pad.shown) {
            std::erase_if(pad.windows, [](HWND h) { return !IsWindow(h); });
            for (HWND h : pad.windows) ShowWindow(h, SW_MINIMIZE);
            pad.shown = false;
        } else {
            ShowScratchpad(pad, monitor);
        }
        return L"ok";
    }

    if (name == L"layoutmsg") {
        std::wstring a = Lower(Trimmed(arg));
        if (a == L"cycle" || a == L"cyclenext" || a == L"next") TileCommandRun(kTileLayout, 0);
        else if (a == L"tile") TileCommandRun(kTileNow, 0);
        else if (a == L"auto") TileCommandRun(kTileAuto, 0);
        else if (a == L"layouts") TileCommandRun(kTileLayouts, 0);
        else if (a.rfind(L"apply", 0) == 0) TileCommandRun(kTileApplyLayout, std::max(0, _wtoi(a.c_str() + 5) - 1));
        else if (a.rfind(L"mfact", 0) == 0 || a.rfind(L"ratio", 0) == 0) {
            double v = _wtof(a.c_str() + 5);
            TileCommandRun(kTileRatio, (LPARAM)(std::fabs(v) < 1 ? v * 100 : v));
        } else return L"error: give cycle, tile, auto, layouts, apply N or mfact N";
        return L"ok";
    }
    if (name == L"splitratio") {
        double v = _wtof(Trimmed(arg).c_str());
        TileCommandRun(kTileRatio, (LPARAM)(std::fabs(v) < 1 ? v * 100 : v));
        return L"ok";
    }

    if (name == L"submap") {
        std::wstring want = Lower(Trimmed(arg));
        int to = 0;
        if (!want.empty() && want != L"reset") {
            std::lock_guard<std::mutex> lock(g_bindsLock);
            auto it = std::find(g_binds.submaps.begin(), g_binds.submaps.end(), want);
            if (it == g_binds.submaps.end()) return L"error: no such submap";
            to = (int)(it - g_binds.submaps.begin());
        }
        g_submap = to;
        BindsEmit(L"submap>>" + (to ? want : std::wstring()));
        return L"ok";
    }
    if (name == L"sendshortcut") {
        auto parts = SplitArgs(arg, 3);  // "MODS, key, window"
        bool ok = false;
        UINT mods = ModsByName(parts[0], &ok);
        DWORD vk = parts.size() > 1 ? KeyByName(parts[1]) : 0;
        if (!ok || !vk) return L"error: unknown modifier or key";
        if (parts.size() > 2 && !parts[2].empty()) {
            HWND h = WindowBy(parts[2]);
            if (!h) return L"error: no such window";
            Activate(h);
        }
        SendKeys(mods, vk);
        return L"ok";
    }
    if (name == L"alterzorder" || name == L"bringactivetotop") {
        auto parts = SplitArgs(arg, 2);
        bool bottom = name == L"alterzorder" && Lower(parts[0]) == L"bottom";
        HWND h = parts.size() > 1 ? WindowBy(parts[1]) : active;
        if (!needWindow(h)) return L"error: no window";
        SetWindowPos(h, bottom ? HWND_BOTTOM : HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        return L"ok";
    }
    if (name == L"movecursor") {
        int x = 0, y = 0;
        if (!TwoNumbers(arg, monitor, &x, &y)) return L"error: give two numbers";
        SetCursorPos(x, y);
        return L"ok";
    }
    if (name == L"movecursortocorner") {
        if (!needWindow(active)) return L"error: no window";
        RECT f = FrameOf(active);
        int c = _wtoi(arg.c_str());  // 0 bottom left, 1 bottom right, 2 top right, 3 top left
        SetCursorPos(c == 1 || c == 2 ? f.right - 1 : f.left, c == 0 || c == 1 ? f.bottom - 1 : f.top);
        return L"ok";
    }
    if (name == L"dpms") {
        std::wstring a = Lower(Trimmed(arg));
        bool off = a == L"off" || (a == L"toggle" && !g_screensOff) || a.empty();
        if (off) {
            g_screensOff = true;
            DefWindowProcW(g_wnd, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        } else {
            g_screensOff = false;
            INPUT i{INPUT_MOUSE};
            i.mi.dwFlags = MOUSEEVENTF_MOVE;  // (a move of nothing wakes them)
            SendInput(1, &i, sizeof(i));
        }
        return L"ok";
    }
    if (name == L"togglegroup" || name == L"changegroupactive" || name == L"moveintogroup" || name == L"moveoutofgroup")
        return GroupCommand(name, arg, active);
    if (name == L"overview" || name == L"hyprexpo:expo") {
        // Every window on every screen at once (the switcher's Mission Control); again closes it.
        if (g_open) PostMessageW(g_wnd, WM_APP_CANCEL, 0, 0);
        else PostMessageW(g_wnd, WM_APP_STEP, 1, MAKELPARAM(kFromWinTab, kFlagSticky));
        return L"ok";
    }
    if (name == L"pickcolor") {
        // The colour under the pointer, as #rrggbb on the clipboard (hyprpicker's job).
        POINT pt;
        GetCursorPos(&pt);
        HDC screen = GetDC(nullptr);
        COLORREF c = GetPixel(screen, pt.x, pt.y);
        ReleaseDC(nullptr, screen);
        if (c == CLR_INVALID) return L"error: couldn't read the screen there";
        WCHAR hex[8];
        swprintf(hex, ARRAYSIZE(hex), L"#%02x%02x%02x", GetRValue(c), GetGValue(c), GetBValue(c));
        if (OpenClipboard(g_wnd)) {
            EmptyClipboard();
            if (HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(hex))) {
                memcpy(GlobalLock(mem), hex, sizeof(hex));
                GlobalUnlock(mem);
                if (!SetClipboardData(CF_UNICODETEXT, mem)) GlobalFree(mem);
            }
            CloseClipboard();
        }
        BindsEmit(std::wstring(L"colorpicked>>") + hex);
        return hex;
    }
    if (name == L"lockscreen") return LockWorkStation() ? L"ok" : L"error: couldn't lock";

    return L"error: unknown command " + name;
}

////////////////////////////////////////////////////////////////////////////////
// A command of several steps, on the switcher's thread. The steps run in order
// until a wait, which leaves the rest to a timer rather than holding the
// thread: a key press has to come back at once, and the switcher's thread is
// the one drawing. A step that answers with an error stops the rest.

constexpr UINT_PTR kTimerCommand = 78;
constexpr size_t kMostWaitingCommands = 8;

struct WaitingCommand {
    std::vector<CommandStep> steps;
    size_t at = 0;
    ULONGLONG due = 0;
};
std::vector<WaitingCommand> g_waiting;  // (the switcher window's thread)

void CommandTimerSet() {
    if (!g_wnd) {
        g_waiting.clear();
        return;
    }
    if (g_waiting.empty()) {
        KillTimer(g_wnd, kTimerCommand);
        return;
    }
    ULONGLONG now = GetTickCount64(), soonest = g_waiting[0].due;
    for (auto const& w : g_waiting) soonest = std::min(soonest, w.due);
    SetTimer(g_wnd, kTimerCommand, (UINT)std::max<ULONGLONG>(soonest > now ? soonest - now : 0, 10), nullptr);
}

// From where it got to, until a wait or the end.
std::wstring StepsFrom(WaitingCommand& w) {
    while (w.at < w.steps.size()) {
        CommandStep const& s = w.steps[w.at++];
        if (s.name == L"wait") {
            w.due = GetTickCount64() + (ULONGLONG)s.waitMs;
            return L"ok";
        }
        std::wstring said = Dispatch(s.name, s.arg);
        if (said.rfind(L"error", 0) == 0) {
            Wh_Log(L"command stopped at \"%s %s\": %s", s.name.c_str(), s.arg.c_str(), said.c_str());
            return said;
        }
    }
    return L"ok";
}

void CommandTimer() {
    ULONGLONG now = GetTickCount64();
    for (size_t i = 0; i < g_waiting.size();) {
        if (g_waiting[i].due > now) {
            i++;
            continue;
        }
        WaitingCommand w = std::move(g_waiting[i]);
        g_waiting.erase(g_waiting.begin() + i);
        std::wstring answer = StepsFrom(w);
        if (w.at < w.steps.size() && answer.rfind(L"error", 0) != 0) {
            g_waiting.insert(g_waiting.begin() + i, std::move(w));
            i++;
        }
    }
    CommandTimerSet();
}

std::wstring RunCommand(std::wstring const& line) {
    Command c = ParseCommand(line);
    if (!c.ok()) return L"error: " + c.error;
    WaitingCommand w;
    w.steps = std::move(c.steps);
    std::wstring answer = StepsFrom(w);
    if (w.at < w.steps.size() && answer.rfind(L"error", 0) != 0) {
        if (g_waiting.size() >= kMostWaitingCommands) return L"error: too many commands waiting their turn";
        g_waiting.push_back(std::move(w));
        CommandTimerSet();
    }
    return answer;
}

////////////////////////////////////////////////////////////////////////////////
// tournectl: a pipe for this user's session, \\.\pipe\tourne-ctl-<session>.
// A caller writes one line and reads the answer; "listen" keeps the pipe
// open and gets events as lines, as Hyprland's event socket gives them.

struct BindCall {
    std::wstring line, answer;
};

struct Listener {
    HANDLE pipe = INVALID_HANDLE_VALUE;
    OVERLAPPED ov{};
    bool writing = false;
    std::string out;
};

std::mutex g_listenLock;
std::vector<std::unique_ptr<Listener>> g_listeners;
std::thread g_controlThread;
HANDLE g_controlStop = nullptr;
std::atomic<bool> g_controlOn{false};
HWINEVENTHOOK g_bindEventHooks[2];
std::unordered_set<HWND> g_announced;  // windows an openwindow was sent for

std::wstring ControlPipeName() {
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    return L"\\\\.\\pipe\\tourne-ctl-" + std::to_wstring(session);
}

std::string Utf8(std::wstring const& s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

void DropListener(Listener& l) {
    if (l.pipe != INVALID_HANDLE_VALUE) {
        CancelIoEx(l.pipe, nullptr);
        if (l.writing) {
            DWORD done = 0;
            GetOverlappedResult(l.pipe, &l.ov, &done, TRUE);
        }
        CloseHandle(l.pipe);
    }
    if (l.ov.hEvent) CloseHandle(l.ov.hEvent);
    l.pipe = INVALID_HANDLE_VALUE;
    l.ov.hEvent = nullptr;
}

// An event line to every listener. One that hasn't read the last one yet
// misses this one; one that's gone is let go.
void BindsEmit(std::wstring const& event) {
    if (!g_controlOn) return;
    std::lock_guard<std::mutex> lock(g_listenLock);
    if (g_listeners.empty()) return;
    std::string line = Utf8(event) + "\n";
    bool dropped = false;
    for (auto& l : g_listeners) {
        if (l->writing) {
            DWORD done = 0;
            if (!GetOverlappedResult(l->pipe, &l->ov, &done, FALSE)) {
                if (GetLastError() == ERROR_IO_INCOMPLETE) continue;
                DropListener(*l);
                dropped = true;
                continue;
            }
            l->writing = false;
        }
        l->out = line;
        ResetEvent(l->ov.hEvent);
        if (WriteFile(l->pipe, l->out.data(), (DWORD)l->out.size(), nullptr, &l->ov)) continue;
        if (GetLastError() == ERROR_IO_PENDING) {
            l->writing = true;
            continue;
        }
        DropListener(*l);
        dropped = true;
    }
    if (dropped) {
        std::erase_if(g_listeners, [](auto const& l) { return l->pipe == INVALID_HANDLE_VALUE; });
        if (g_listeners.empty() && g_wnd) PostMessageW(g_wnd, WM_APP_BINDWATCH, 0, 0);
    }
}

std::wstring Hex(HWND h) {
    WCHAR text[24];
    swprintf(text, ARRAYSIZE(text), L"%llx", (unsigned long long)(uintptr_t)h);
    return text;
}

std::wstring WindowLine(HWND h) {
    return Hex(h) + L"," + ClassOf(h) + L"," + FileNameOf(h) + L"," + Title(h);
}

void CALLBACK BindEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || !h) return;
    switch (event) {
        case EVENT_SYSTEM_FOREGROUND: {
            HWND a = ActiveWindow();
            if (!a) return;
            BindsEmit(L"activewindow>>" + ClassOf(a) + L"," + Title(a));
            BindsEmit(L"activewindowv2>>" + Hex(a));
            return;
        }
        case EVENT_OBJECT_SHOW:
            if (!g_announced.contains(h) && BindWindow(h)) {
                g_announced.insert(h);
                BindsEmit(L"openwindow>>" + WindowLine(h));
            }
            return;
        case EVENT_OBJECT_DESTROY:
            if (g_announced.erase(h)) BindsEmit(L"closewindow>>" + Hex(h));
            return;
    }
}

// Events watched only while someone listens (on the switcher's thread).
void WatchBindEvents(bool on) {
    for (auto& hook : g_bindEventHooks) {
        if (hook) UnhookWinEvent(hook);
        hook = nullptr;
    }
    g_announced.clear();
    if (!on) return;
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    g_bindEventHooks[0] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, BindEventProc, 0, 0, flags);
    g_bindEventHooks[1] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_SHOW, nullptr, BindEventProc, 0, 0, flags);
    for (HWND h : BindWindows()) g_announced.insert(h);
}

// A command line from tournectl, on the switcher's thread.
std::wstring ControlCommand(std::wstring const& line) {
    std::wstring text = Trimmed(line);
    size_t space = text.find(L' ');
    std::wstring word = Lower(text.substr(0, space)), rest = space == std::wstring::npos ? L"" : Trimmed(text.substr(space + 1));
    if (word == L"dispatch") {
        size_t s2 = rest.find(L' ');
        return Dispatch(Lower(rest.substr(0, s2)), s2 == std::wstring::npos ? L"" : Trimmed(rest.substr(s2 + 1)));
    }
    if (word == L"run") return RunCommand(rest);
    // What a command would do, without doing any of it (the menu editor's help
    // asks this, and so does tools/binds/cmdtest.cpp).
    if (word == L"check") return DescribeCommand(ParseCommand(rest));
    if (word == L"version") return L"Tourne (tournectl 1)";
    if (word == L"reload") {
        if (g_wnd) PostMessageW(g_wnd, WM_APP_SETTINGS, 0, 0);
        return L"ok";
    }
    if (word == L"activewindow") {
        HWND a = ActiveWindow();
        return a ? WindowLine(a) : L"error: no window";
    }
    if (word == L"clients") {
        std::wstring out;
        for (HWND h : BindWindows()) out += WindowLine(h) + L"\n";
        return out.empty() ? L"" : out.substr(0, out.size() - 1);
    }
    if (word == L"workspaces" || word == L"activeworkspace") {
        std::vector<GUID> ids = VdReadGuids(L"VirtualDesktopIDs");
        int current = VdCurrentIndex(ids), count = std::max(1, (int)ids.size());
        if (word == L"activeworkspace") return std::to_wstring(current + 1);
        std::wstring out;
        for (int i = 0; i < count; i++) out += std::to_wstring(i + 1) + (i == current ? L" (active)\n" : L"\n");
        return out.substr(0, out.size() - 1);
    }
    if (word == L"monitors") {
        std::wstring out;
        int i = 1;
        for (HMONITOR m : Monitors()) {
            MONITORINFOEXW mi{};
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(m, &mi);
            WCHAR row[160];
            swprintf(row, ARRAYSIZE(row), L"%d %s %ldx%ld at %ld,%ld%s\n", i++, mi.szDevice, mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top, mi.rcMonitor.left, mi.rcMonitor.top,
                     (mi.dwFlags & MONITORINFOF_PRIMARY) ? L" (main)" : L"");
            out += row;
        }
        return out.empty() ? L"" : out.substr(0, out.size() - 1);
    }
    if (word == L"binds" || word == L"submap") {
        std::lock_guard<std::mutex> lock(g_bindsLock);
        if (word == L"submap") return g_binds.submaps[std::min<size_t>(g_submap, g_binds.submaps.size() - 1)];
        std::wstring out;
        for (auto const& b : g_binds.binds) out += b.line + L"\n";
        return out.empty() ? L"" : out.substr(0, out.size() - 1);
    }
    return L"error: unknown request " + word +
           L" (dispatch, activewindow, clients, workspaces, activeworkspace, monitors, binds, submap, reload, version, listen)";
}

// Waits on the pipe for a caller, one at a time; nothing runs between calls.
void ControlThread(HANDLE stop) {
    std::wstring name = ControlPipeName();
    HANDLE ev = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HANDLE regEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HKEY desktops = nullptr;
    bool regArmed = false;
    int lastDesktop = -1;
    RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops", 0, KEY_NOTIFY | KEY_READ,
                  &desktops);
    for (;;) {
        HANDLE pipe = CreateNamedPipeW(name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                       PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                                       PIPE_UNLIMITED_INSTANCES, 64 * 1024, 4096, 0, nullptr);
        if (pipe == INVALID_HANDLE_VALUE) {
            Wh_Log(L"tournectl: no pipe (%u)", GetLastError());
            break;
        }
        OVERLAPPED ov{};
        ov.hEvent = ev;
        ResetEvent(ev);
        bool connected = ConnectNamedPipe(pipe, &ov) || GetLastError() == ERROR_PIPE_CONNECTED;
        bool stopping = false;
        while (!connected && GetLastError() == ERROR_IO_PENDING) {
            // While someone listens, Windows' desktop switches are watched too (the registry tells, no polling).
            bool listening;
            {
                std::lock_guard<std::mutex> lock(g_listenLock);
                listening = !g_listeners.empty();
            }
            if (listening && desktops && !regArmed) {
                ResetEvent(regEvent);
                regArmed = RegNotifyChangeKeyValue(desktops, FALSE, REG_NOTIFY_CHANGE_LAST_SET, regEvent, TRUE) == ERROR_SUCCESS;
                if (lastDesktop < 0) lastDesktop = VdCurrentIndex(VdReadGuids(L"VirtualDesktopIDs"));
            }
            HANDLE waits[3] = {ev, stop, regEvent};
            DWORD w = WaitForMultipleObjects(regArmed ? 3 : 2, waits, FALSE, regArmed || !listening ? INFINITE : 1000);
            if (w == WAIT_OBJECT_0 + 1) {
                stopping = true;
                break;
            }
            if (w == WAIT_OBJECT_0 + 2) {
                regArmed = false;
                int now = VdCurrentIndex(VdReadGuids(L"VirtualDesktopIDs"));
                if (now != lastDesktop) {
                    lastDesktop = now;
                    BindsEmit(L"workspace>>" + std::to_wstring(now + 1));
                }
                continue;
            }
            if (w == WAIT_TIMEOUT) continue;
            DWORD done = 0;
            connected = GetOverlappedResult(pipe, &ov, &done, FALSE) != FALSE;
            break;
        }
        if (stopping || !connected) {
            CancelIoEx(pipe, nullptr);
            CloseHandle(pipe);
            if (stopping) break;
            continue;
        }
        // One line, within a moment.
        std::string got;
        char buf[1024];
        while (got.find('\n') == std::string::npos && got.size() < 16384) {
            ResetEvent(ev);
            DWORD n = 0;
            if (!ReadFile(pipe, buf, sizeof(buf), &n, &ov)) {
                if (GetLastError() != ERROR_IO_PENDING) break;
                HANDLE waits[2] = {ev, stop};
                if (WaitForMultipleObjects(2, waits, FALSE, 2000) != WAIT_OBJECT_0) {
                    CancelIoEx(pipe, nullptr);
                    GetOverlappedResult(pipe, &ov, &n, TRUE);
                    break;
                }
                if (!GetOverlappedResult(pipe, &ov, &n, FALSE)) break;
            }
            if (!n) break;
            got.append(buf, n);
        }
        int wn = MultiByteToWideChar(CP_UTF8, 0, got.data(), (int)got.size(), nullptr, 0);
        std::wstring line(wn, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, got.data(), (int)got.size(), line.data(), wn);
        line = Trimmed(line.substr(0, line.find(L'\n')));
        if (Lower(line) == L"listen") {
            auto l = std::make_unique<Listener>();
            l->pipe = pipe;
            l->ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            size_t count;
            {
                std::lock_guard<std::mutex> lock(g_listenLock);
                g_listeners.push_back(std::move(l));
                count = g_listeners.size();
            }
            if (count == 1 && g_wnd) PostMessageW(g_wnd, WM_APP_BINDWATCH, 1, 0);
            continue;
        }
        BindCall call{line, L""};
        DWORD_PTR result = 0;
        if (!g_wnd || !SendMessageTimeoutW(g_wnd, WM_APP_BINDCALL, 0, (LPARAM)&call, SMTO_ABORTIFHUNG, 5000, &result))
            call.answer = L"error: Tourne didn't answer";
        std::string answer = Utf8(call.answer) + "\n";
        ResetEvent(ev);
        DWORD n = 0;
        if (!WriteFile(pipe, answer.data(), (DWORD)answer.size(), &n, &ov) && GetLastError() == ERROR_IO_PENDING) {
            HANDLE waits[2] = {ev, stop};
            if (WaitForMultipleObjects(2, waits, FALSE, 2000) != WAIT_OBJECT_0) CancelIoEx(pipe, nullptr);
            GetOverlappedResult(pipe, &ov, &n, TRUE);
        }
        FlushFileBuffers(pipe);
        DisconnectNamedPipe(pipe);
        CloseHandle(pipe);
    }
    if (desktops) RegCloseKey(desktops);
    CloseHandle(regEvent);
    CloseHandle(ev);
}

void StopControl() {
    if (g_controlThread.joinable()) {
        SetEvent(g_controlStop);
        g_controlThread.join();
    }
    if (g_controlStop) CloseHandle(g_controlStop);
    g_controlStop = nullptr;
    g_controlOn = false;
    std::lock_guard<std::mutex> lock(g_listenLock);
    for (auto& l : g_listeners) DropListener(*l);
    g_listeners.clear();
}

void StartControl() {
    g_controlStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_controlOn = true;
    g_controlThread = std::thread(ControlThread, g_controlStop);
}

////////////////////////////////////////////////////////////////////////////////
// Idle (hypridle): each idle line runs once you've been away that long. One
// timer, set for the next one due; once all have run, a look every 2 s while
// you're away, to set them up again when you're back. None without idle lines.

constexpr UINT_PTR kTimerIdle = 77;
std::vector<bool> g_idleRan;

DWORD IdleMs() {
    LASTINPUTINFO li{sizeof(li)};
    GetLastInputInfo(&li);
    return GetTickCount() - li.dwTime;
}

void IdleTimer() {
    std::vector<BindTable::Idle> idle;
    {
        std::lock_guard<std::mutex> lock(g_bindsLock);
        idle = g_binds.idle;
    }
    if (!g_wnd) return;
    KillTimer(g_wnd, kTimerIdle);
    if (idle.empty()) return;
    g_idleRan.resize(idle.size(), false);
    DWORD away = IdleMs();
    bool watching = ForegroundIsFullscreen();  // (a film or a game: nothing runs)
    DWORD next = 0xFFFFFFFF;
    bool anyRan = false, allRan = true;
    for (size_t i = 0; i < idle.size(); i++) {
        DWORD due = (DWORD)idle[i].seconds * 1000;
        if (away < 1000) g_idleRan[i] = false;  // back: each runs again next time
        if (!g_idleRan[i] && away >= due && !watching) {
            g_idleRan[i] = true;
            std::wstring answer = Dispatch(idle[i].dispatcher, idle[i].arg);
            if (answer != L"ok") Wh_Log(L"idle %d: %s", idle[i].seconds, answer.c_str());
        }
        if (g_idleRan[i]) anyRan = true;
        else allRan = false, next = std::min(next, due > away ? due - away : 1000);
    }
    SetTimer(g_wnd, kTimerIdle, anyRan && (allRan || watching) ? 2000 : anyRan ? std::min<DWORD>(next, 2000) : std::max<DWORD>(next, 250), nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// A window dragged by a mouse bind, on the switcher's thread: it floats (as in
// Hyprland), comes to the front and follows the pointer; a resize moves the
// corner nearest to where the drag started.

struct MouseDrag {
    HWND window = nullptr;
    bool resize = false, left = false, top = false;
    POINT start{};
    RECT frame{};
};
MouseDrag g_mouseDrag;

void MouseDragStep(WPARAM what, LPARAM lParam) {
    POINT pt;
    GetCursorPos(&pt);
    if (what == 0) {
        g_mouseDrag = {};
        HWND h = GetAncestor(WindowFromPoint(pt), GA_ROOT);
        if (!BindWindow(h)) return;
        if (IsZoomed(h)) {
            // Restored under the pointer, at the same place across it.
            RECT before = FrameOf(h);
            ShowWindow(h, SW_RESTORE);
            RECT after = FrameOf(h);
            int w = after.right - after.left;
            double across = before.right > before.left ? (pt.x - before.left) / (double)(before.right - before.left) : 0.5;
            int x = pt.x - (int)(w * across);
            PlaceWindowFrame(h, {x, before.top, x + w, before.top + after.bottom - after.top});
        }
        if (!IsFloating(h)) {
            bool tiled = IsTiled(h);
            g_floating.push_back(h);
            if (tiled) TileMonitor(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST), false);
        }
        SetForegroundWindow(h);
        g_mouseDrag.window = h;
        g_mouseDrag.resize = lParam != 0;
        g_mouseDrag.start = pt;
        g_mouseDrag.frame = FrameOf(h);
        RECT const& f = g_mouseDrag.frame;
        g_mouseDrag.left = pt.x < (f.left + f.right) / 2;
        g_mouseDrag.top = pt.y < (f.top + f.bottom) / 2;
        return;
    }
    g_mouseDragPosted = false;
    MouseDrag const& d = g_mouseDrag;
    if (!d.window || !IsWindow(d.window)) return;
    int dx = pt.x - d.start.x, dy = pt.y - d.start.y;
    RECT r = d.frame;
    if (!d.resize) {
        OffsetRect(&r, dx, dy);
    } else {
        constexpr int kMin = 120;
        if (d.left) r.left = std::min(r.left + dx, r.right - kMin);
        else r.right = std::max(r.right + dx, r.left + kMin);
        if (d.top) r.top = std::min(r.top + dy, r.bottom - kMin);
        else r.bottom = std::max(r.bottom + dy, r.top + kMin);
    }
    PlaceWindowFrame(d.window, r);
    if (what == 2) g_mouseDrag = {};
}

////////////////////////////////////////////////////////////////////////////////
// Settings, and the switcher's window's share

// On the switcher's thread, with the other settings.
void ApplyWindowRules(std::vector<std::wstring> const& lines);
void EndWindowRules();

void ApplyBindSettings() {
    bool on = Wh_GetIntSetting(L"binds.enabled") != 0;
    int doubleMs = Wh_GetIntSetting(L"binds.doubleMs"), holdMs = Wh_GetIntSetting(L"binds.holdMs");
    g_doubleMs = doubleMs > 0 ? std::clamp(doubleMs, 100, 1000) : 300;
    g_holdDefaultMs = holdMs > 0 ? std::clamp(holdMs, 100, 10000) : 500;
    BindTable table = on ? ReadBinds(ReadBindLines()) : BindTable{};
    {
        std::lock_guard<std::mutex> lock(g_bindsLock);
        g_binds = std::move(table);
        g_bindsGeneration++;
        g_submap = 0;
    }
    g_bindsOn = on && !g_binds.binds.empty();
    bool mouse = on && !g_binds.mouseBinds.empty();
    if (mouse != g_mouseBindsOn) {
        g_mouseBindsOn = mouse;
        WatchWheel(g_open);  // (the mouse hook: there while a mouse bind or the wheel wants it)
    }
    ApplyWindowRules(g_binds.ruleLines);
    g_waiting.clear();  // (a command part-way through doesn't carry over a reload)
    CommandTimerSet();
    g_idleRan.clear();
    IdleTimer();
    if (on) Wh_Log(L"binds: %d read", (int)g_binds.binds.size());
    bool control = on && Wh_GetIntSetting(L"binds.control") != 0;
    if (control != g_controlOn) {
        WatchBindEvents(false);
        StopControl();
        if (control) StartControl();
    }
}

void EndBinds() {
    g_bindsOn = false;
    g_mouseBindsOn = false;
    EndWindowRules();
    WatchBindEvents(false);
    StopControl();
}

// A bind run from the switcher's window: its dispatcher, and the borders after it.
void RunBind(size_t index, unsigned generation) {
    Bind b;
    {
        std::lock_guard<std::mutex> lock(g_bindsLock);
        if (generation != g_bindsGeneration || index >= g_binds.binds.size()) return;
        b = g_binds.binds[index];
    }
    std::wstring answer = Dispatch(b.dispatcher, b.arg);
    if (answer != L"ok") Wh_Log(L"binds: %s: %s", b.line.c_str(), answer.c_str());
    UpdateFocusBorder(GetForegroundWindow());
    if (g_borderSettings.frontMoves) RefreshAllBorders();
}

// The switcher's window procedure's share: true if the message was the binds'.
bool BindsMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* result) {
    // Tourne Desktop's plugin runtime wrote new PluginBinds: read the binds again.
    static UINT reload = RegisterWindowMessageW(L"TourneDesktop.BindsReload");
    if (msg == reload && reload) {
        ApplyBindSettings();
        *result = 0;
        return true;
    }
    switch (msg) {
        case WM_APP_BINDHOLD: {
            int holdMs = 0;
            {
                std::lock_guard<std::mutex> lock(g_bindsLock);
                if ((unsigned)lParam == g_bindsGeneration && wParam < g_binds.binds.size()) holdMs = g_binds.binds[wParam].holdMs;
            }
            g_holdGeneration = (unsigned)lParam;
            g_holdPressedAt = GetTickCount64();
            if (holdMs) SetTimer(g_wnd, kTimerBindHold, holdMs, nullptr);
            *result = 0;
            return true;
        }
        case WM_APP_BINDTAP: {
            // Let go too soon: the plain bind on that key, if there is one, else the key pressed again for the
            // program in front.
            KillTimer(g_wnd, kTimerBindHold);
            int plain = -1;
            bool doubled = false;  // a double press on the same key too
            UINT mods = 0;
            DWORD vk = 0;
            {
                std::lock_guard<std::mutex> lock(g_bindsLock);
                if ((unsigned)lParam != g_bindsGeneration || wParam >= g_binds.binds.size()) return true;
                Bind const& h = g_binds.binds[wParam];
                mods = h.mods, vk = h.vk;
                for (int i = 0; i < (int)g_binds.binds.size(); i++) {
                    Bind const& b = g_binds.binds[i];
                    if (b.submap != h.submap || b.mods != h.mods || b.vk != h.vk || b.chord != h.chord) continue;
                    if (b.twice) doubled = true;
                    else if (!b.holdMs && plain < 0) plain = i;
                }
            }
            // With a double press there, the tap waits till a second press can't come any more (binds.doubleMs
            // from the first press).
            if (plain >= 0 && doubled) {
                ULONGLONG since = GetTickCount64() - g_holdPressedAt;
                g_tapLater = plain;
                g_tapLaterGeneration = (unsigned)lParam;
                SetTimer(g_wnd, kTimerBindTapLater, since < (ULONGLONG)g_doubleMs ? (UINT)(g_doubleMs - since) + 10 : 10, nullptr);
            } else if (plain >= 0) {
                RunBind(plain, (unsigned)lParam);
            } else {
                SendKeys(0, vk);
            }
            (void)mods;
            *result = 0;
            return true;
        }
        case WM_APP_MOUSEDRAG:
            MouseDragStep(wParam, lParam);
            return true;
        case WM_APP_BIND: {
            bool twice = false;
            {
                std::lock_guard<std::mutex> lock(g_bindsLock);
                twice = (unsigned)lParam == g_bindsGeneration && wParam < g_binds.binds.size() && g_binds.binds[wParam].twice;
            }
            if (twice && g_tapLater >= 0) {  // (the double press it was waiting for: no tap)
                KillTimer(g_wnd, kTimerBindTapLater);
                g_tapLater = -1;
            }
            RunBind(wParam, (unsigned)lParam);
        }
            *result = 0;
            return true;
        case WM_APP_BINDCALL: {
            auto* call = (BindCall*)lParam;
            call->answer = ControlCommand(call->line);
            UpdateFocusBorder(GetForegroundWindow());
            if (g_borderSettings.frontMoves) RefreshAllBorders();
            *result = 1;
            return true;
        }
        case WM_TIMER:
            if (wParam == kTimerBindHold) {
                // Held long enough (and still held): the hold bind runs, once.
                KillTimer(g_wnd, kTimerBindHold);
                int hold = g_holdBind;
                DWORD vk = g_holdVk;
                if (hold >= 0 && vk && GetAsyncKeyState((int)vk) < 0 && !g_holdRan.exchange(true)) RunBind(hold, g_holdGeneration);
                *result = 0;
                return true;
            }
            if (wParam == kTimerBindTapLater) {
                KillTimer(g_wnd, kTimerBindTapLater);
                int tap = std::exchange(g_tapLater, -1);
                if (tap >= 0) RunBind(tap, g_tapLaterGeneration);
                *result = 0;
                return true;
            }
            if (wParam == kTimerCommand) {
                CommandTimer();
                *result = 0;
                return true;
            }
            if (wParam != kTimerIdle) return false;
            IdleTimer();
            *result = 0;
            return true;
        case WM_APP_BINDWATCH:
            WatchBindEvents(wParam != 0);
            *result = 0;
            return true;
    }
    return false;
}
