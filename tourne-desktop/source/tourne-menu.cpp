////////////////////////////////////////////////////////////////////////////////
// The Tourne Menu ("// @inline tourne-menu.cpp", in taskbar-host.cpp after start-menu.cpp): a full LiteMenu
// replacement, in Tourne's menu look. It opens on the desktop's empty space (Tourne Desktop's desktop
// posts TourneMenuMessage to a bar), on the bar's empty space and from a hotkey, as set in menu.*.
// Its items are menu.items rows (depth, name, command, args); with none, LiteMenu's own config.xml is
// read at each opening, so moving over from LiteMenu needs nothing. Beyond LiteMenu: tourne: actions and
// live submenus (open windows, desktops, programs, recent items, a folder).
// research/plans/tourne-menu-litemenu-replacement.md

UINT TourneMenuMessage() {
    static UINT msg = RegisterWindowMessageW(L"TourneDesktop.ShowMenu");
    return msg;
}

struct TourneMenuItem {
    int type = 1;  // 0 a submenu, 1 an item, 2 a separator
    std::wstring name, command, args;
    bool shellApp = false;  // LiteMenu's: the command is an app id
    std::vector<TourneMenuItem> items;
};

constexpr wchar_t kLiteMenuPackage[] = L"RedBaronDevelopment.PopEx_36fx7gy0frrbc";

// An item may be a Tourne command (command.cpp's language), which the part that
// runs commands is asked to do.
// @inline tourne-pipe.cpp

////////////////////////////////////////////////////////////////////////////////
// LiteMenu's config.xml: enough XML for it (elements, quoted attributes, text, the prolog, comments).

struct TmXml {
    std::wstring name, text;
    std::vector<std::pair<std::wstring, std::wstring>> attrs;
    std::vector<TmXml> children;
    TmXml const* Child(const wchar_t* n) const {
        for (auto const& c : children)
            if (c.name == n) return &c;
        return nullptr;
    }
    std::wstring ChildText(const wchar_t* n) const {
        TmXml const* c = Child(n);
        return c ? c->text : L"";
    }
    std::wstring Attr(const wchar_t* n) const {
        for (auto const& [k, v] : attrs)
            if (k == n) return v;
        return L"";
    }
};

std::wstring TmXmlDecode(std::wstring_view s) {
    std::wstring out;
    for (size_t i = 0; i < s.size(); i++) {
        size_t semi = s[i] == L'&' ? s.find(L';', i) : std::wstring_view::npos;
        if (semi == std::wstring_view::npos) {
            out += s[i];
            continue;
        }
        std::wstring e(s.substr(i + 1, semi - i - 1));
        if (e == L"amp") out += L'&';
        else if (e == L"lt") out += L'<';
        else if (e == L"gt") out += L'>';
        else if (e == L"quot") out += L'"';
        else if (e == L"apos") out += L'\'';
        else if (e.size() > 1 && e[0] == L'#') out += (wchar_t)(e[1] == L'x' ? wcstoul(e.c_str() + 2, nullptr, 16) : wcstoul(e.c_str() + 1, nullptr, 10));
        else {
            out += s[i];
            continue;
        }
        i = semi;
    }
    return out;
}

void TmXmlSkip(std::wstring_view s, size_t& at) {
    for (;;) {
        while (at < s.size() && iswspace(s[at])) at++;
        size_t close = std::wstring_view::npos;
        if (s.compare(at, 2, L"<?") == 0) close = s.find(L"?>", at), close = close == std::wstring_view::npos ? s.size() : close + 2;
        else if (s.compare(at, 4, L"<!--") == 0) close = s.find(L"-->", at), close = close == std::wstring_view::npos ? s.size() : close + 3;
        else return;
        at = close;
    }
}

std::wstring TmTrim(std::wstring s) {
    while (!s.empty() && iswspace(s.back())) s.pop_back();
    size_t lead = 0;
    while (lead < s.size() && iswspace(s[lead])) lead++;
    return s.substr(lead);
}

bool TmXmlParse(std::wstring_view s, size_t& at, TmXml& node, int depth = 0) {
    if (depth > 64) return false;
    TmXmlSkip(s, at);
    if (at >= s.size() || s[at] != L'<') return false;
    size_t end = at + 1;
    while (end < s.size() && !iswspace(s[end]) && s[end] != L'>' && s[end] != L'/') end++;
    node.name = std::wstring(s.substr(at + 1, end - at - 1));
    at = end;
    for (;;) {  // its attributes
        while (at < s.size() && iswspace(s[at])) at++;
        if (at >= s.size()) return false;
        if (s[at] == L'/' || s[at] == L'>') {
            bool empty = s[at] == L'/';
            at = s.find(L'>', at);
            if (at == std::wstring_view::npos) return false;
            at++;
            if (empty) return true;
            break;
        }
        size_t eq = s.find(L'=', at);
        size_t q1 = eq == std::wstring_view::npos ? eq : s.find(L'"', eq);
        size_t q2 = q1 == std::wstring_view::npos ? q1 : s.find(L'"', q1 + 1);
        if (q2 == std::wstring_view::npos) return false;
        node.attrs.push_back({TmTrim(std::wstring(s.substr(at, eq - at))), TmXmlDecode(s.substr(q1 + 1, q2 - q1 - 1))});
        at = q2 + 1;
    }
    for (;;) {  // its contents, to the closing tag
        size_t lt = s.find(L'<', at);
        if (lt == std::wstring_view::npos) return false;
        node.text += TmXmlDecode(s.substr(at, lt - at));
        at = lt;
        if (s.compare(at, 2, L"</") == 0) {
            at = s.find(L'>', at);
            if (at == std::wstring_view::npos) return false;
            at++;
            break;
        }
        if (s.compare(at, 4, L"<!--") == 0) {
            TmXmlSkip(s, at);
            continue;
        }
        TmXml child;
        if (!TmXmlParse(s, at, child, depth + 1)) return false;
        node.children.push_back(std::move(child));
    }
    node.text = TmTrim(node.text);
    return true;
}

void TmLiteItems(TmXml const& parent, std::vector<TourneMenuItem>& out) {
    for (auto const& c : parent.children) {
        if (c.name != L"MenuItem") continue;
        TourneMenuItem item;
        std::wstring type = c.Attr(L"type");
        item.type = type.empty() ? 1 : _wtoi(type.c_str());
        item.name = c.ChildText(L"DisplayName");
        item.command = c.ChildText(L"Command");
        item.args = c.ChildText(L"Parameters");
        item.shellApp = _wtoi(c.ChildText(L"IsShellApp").c_str()) != 0;
        if (item.type == 0) TmLiteItems(c, item.items);
        out.push_back(std::move(item));
    }
}

// LiteMenu's menu, from its own file (read now, so its settings' edits show at once).
std::vector<TourneMenuItem> LiteMenuItems() {
    std::vector<TourneMenuItem> items;
    PWSTR local = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) return items;
    std::wstring path = std::wstring(local) + L"\\Packages\\" + kLiteMenuPackage + L"\\LocalState\\config.xml";
    CoTaskMemFree(local);
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) return items;
    std::string bytes;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart < (1 << 22)) {
        bytes.resize((size_t)size.QuadPart);
        DWORD got = 0;
        if (!ReadFile(file, bytes.data(), (DWORD)bytes.size(), &got, nullptr)) got = 0;
        bytes.resize(got);
    }
    CloseHandle(file);
    size_t skip = bytes.rfind("\xEF\xBB\xBF", 0) == 0 ? 3 : 0;
    int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), nullptr, 0);
    if (n <= 0) return items;
    std::wstring text(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), text.data(), n);
    TmXml root;
    size_t at = 0;
    if (!TmXmlParse(text, at, root) || root.name != L"Settings") return items;
    if (TmXml const* menu = root.Child(L"Menu")) TmLiteItems(*menu, items);
    return items;
}

////////////////////////////////////////////////////////////////////////////////
// What's in it

// The live submenus a command can name.
bool TmIsLiveList(std::wstring const& command) {
    return command == L"tourne:windows" || command == L"tourne:desktops" || command == L"tourne:programs" ||
           command == L"tourne:recent" || command.rfind(L"tourne:folder:", 0) == 0;
}

// Your rows as a tree: one more depth than the row above puts a row in its submenu; "-" is a separator.
std::vector<TourneMenuItem> RowItems(std::vector<MenuRow> const& rows) {
    std::vector<TourneMenuItem> top;
    std::vector<std::vector<TourneMenuItem>*> levels{&top};
    for (size_t i = 0; i < rows.size(); i++) {
        MenuRow const& r = rows[i];
        int depth = std::clamp(r.depth, 0, (int)levels.size() - 1);
        levels.resize(depth + 1);
        TourneMenuItem item;
        item.name = r.name;
        item.command = r.command;
        item.args = r.args;
        bool opens = i + 1 < rows.size() && rows[i + 1].depth > depth;
        int type = r.name == L"-" ? 2 : opens || TmIsLiveList(r.command) ? 0 : 1;
        item.type = type;
        levels[depth]->push_back(std::move(item));
        if (type == 0) levels.push_back(&levels[depth]->back().items);
    }
    return top;
}

std::vector<TourneMenuItem> TourneMenuItems() {
    if (!g_settings.menuRows.empty()) return RowItems(g_settings.menuRows);
    return LiteMenuItems();
}

// A menu's text: & would underline the letter after it.
std::wstring TmLabel(std::wstring const& text) {
    std::wstring out;
    for (wchar_t ch : text) {
        if (ch == L'&') out += L'&';
        out += ch;
    }
    return out.empty() ? L" " : out;
}

struct TourneMenuBuild {
    std::vector<std::function<void()>> actions;  // command id - 1
    int added = 0;                                // items so far, against a folder of thousands
};

UINT TmAdd(TourneMenuBuild& b, std::function<void()> action) {
    b.actions.push_back(std::move(action));
    return (UINT)b.actions.size();
}

std::wstring TmKnownPath(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path))) out = path;
    CoTaskMemFree(path);
    return out;
}

void TmOpen(std::wstring const& target, std::wstring const& args = L"") {
    ShellExecuteW(nullptr, nullptr, target.c_str(), args.empty() ? nullptr : args.c_str(), nullptr, SW_SHOWNORMAL);
}

// A folder's contents, folders first, as submenus to a few levels; shortcuts without their .lnk.
void TmFolder(HMENU menu, std::vector<std::wstring> const& dirs, TourneMenuBuild& b, int depth) {
    struct Entry {
        std::wstring name, path;
        bool folder;
    };
    std::vector<Entry> entries;
    std::map<std::wstring, size_t> folders;  // lower-case name: its entry, so the same folder in two places merges
    std::map<std::wstring, std::vector<std::wstring>> folderDirs;
    for (auto const& dir : dirs) {
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((dir + L"\\*").c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) continue;
        do {
            if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..") || (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
                !_wcsicmp(fd.cFileName, L"desktop.ini"))
                continue;
            std::wstring path = dir + L"\\" + fd.cFileName;
            bool folder = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            std::wstring name = fd.cFileName;
            if (folder) {
                std::wstring key = Lower(name);
                folderDirs[key].push_back(path);
                if (folders.count(key)) continue;
                folders[key] = entries.size();
            } else {
                size_t dot = name.rfind(L'.');
                if (dot != std::wstring::npos && (!_wcsicmp(name.c_str() + dot, L".lnk") || !_wcsicmp(name.c_str() + dot, L".url")))
                    name.resize(dot);
            }
            entries.push_back({name, path, folder});
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }
    std::sort(entries.begin(), entries.end(), [](Entry const& a, Entry const& c) {
        if (a.folder != c.folder) return a.folder;
        return _wcsicmp(a.name.c_str(), c.name.c_str()) < 0;
    });
    for (auto const& e : entries) {
        if (++b.added > 600) {
            AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(more not shown)");
            return;
        }
        if (e.folder && depth < 4) {
            HMENU sub = CreatePopupMenu();
            TmFolder(sub, folderDirs[Lower(e.name)], b, depth + 1);
            std::wstring path = e.path;
            AppendMenuW(sub, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(sub, MF_STRING, TmAdd(b, [path] { TmOpen(path); }), L"Open this folder");
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)sub, TmLabel(e.name).c_str());
        } else {
            std::wstring path = e.path;
            AppendMenuW(menu, MF_STRING, TmAdd(b, [path] { TmOpen(path); }), TmLabel(e.name).c_str());
        }
    }
    if (entries.empty()) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(empty)");
}

// The windows open on this desktop; a click brings one up.
void TmWindows(HMENU menu, TourneMenuBuild& b) {
    int count = 0;
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT)) {
        if (!IsTaskWindow(h)) continue;
        WCHAR title[256] = L"";
        GetWindowTextW(h, title, ARRAYSIZE(title));
        std::wstring text = title;
        if (text.size() > 60) text = text.substr(0, 59) + L"\x2026";
        AppendMenuW(menu, MF_STRING, TmAdd(b, [h] {
                        if (!IsWindow(h)) return;
                        if (IsIconic(h)) ShowWindowAsync(h, SW_RESTORE);
                        SetForegroundWindow(h);
                    }),
                    TmLabel(text).c_str());
        count++;
    }
    if (!count) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(no windows)");
}

// Windows' virtual desktops; a click goes there.
void TmDesktops(HMENU menu, TourneMenuBuild& b) {
    for (int i = 0; i < g_pagerDesktops.count; i++)
        AppendMenuW(menu, MF_STRING | (i == g_pagerDesktops.current ? MF_CHECKED : 0), TmAdd(b, [i] { PagerGoTo(i); }),
                    TmLabel(PagerLabel(i)).c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, TmAdd(b, [] { SendKeys({VK_LCONTROL, VK_LWIN, 'D'}); }), L"New desktop");
}

// Recent items, newest first.
void TmRecent(HMENU menu, TourneMenuBuild& b) {
    std::wstring dir = TmKnownPath(FOLDERID_Recent);
    struct Recent {
        std::wstring name, path;
        FILETIME when;
    };
    std::vector<Recent> recent;
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((dir + L"\\*.lnk").c_str(), &fd);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            std::wstring name = fd.cFileName;
            name.resize(name.size() - 4);
            recent.push_back({name, dir + L"\\" + fd.cFileName, fd.ftLastWriteTime});
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }
    std::sort(recent.begin(), recent.end(), [](Recent const& a, Recent const& c) { return CompareFileTime(&a.when, &c.when) > 0; });
    if (recent.size() > 20) recent.resize(20);
    for (auto const& r : recent) {
        std::wstring path = r.path;
        AppendMenuW(menu, MF_STRING, TmAdd(b, [path] { TmOpen(path); }), TmLabel(r.name).c_str());
    }
    if (recent.empty()) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(none)");
}

void TmLive(HMENU menu, std::wstring const& command, TourneMenuBuild& b) {
    if (command == L"tourne:windows") TmWindows(menu, b);
    else if (command == L"tourne:desktops") TmDesktops(menu, b);
    else if (command == L"tourne:recent") TmRecent(menu, b);
    else if (command == L"tourne:programs") TmFolder(menu, {TmKnownPath(FOLDERID_Programs), TmKnownPath(FOLDERID_CommonPrograms)}, b, 0);
    else if (command.rfind(L"tourne:folder:", 0) == 0) TmFolder(menu, {Expand(command.substr(14))}, b, 0);
}

////////////////////////////////////////////////////////////////////////////////
// What it does

void TmPower(bool restart) {
    ShellExecuteW(nullptr, nullptr, L"shutdown.exe", restart ? L"/r /t 0" : L"/s /t 0", nullptr, SW_HIDE);
}

void TmSleep() {
    using SetSuspendStateFn = BOOLEAN(WINAPI*)(BOOLEAN, BOOLEAN, BOOLEAN);
    if (HMODULE power = LoadLibraryW(L"powrprof.dll")) {
        if (auto suspend = (SetSuspendStateFn)GetProcAddress(power, "SetSuspendState")) suspend(FALSE, FALSE, FALSE);
        FreeLibrary(power);
    }
}

// LiteMenu's own processes (its package's), while it still runs beside the Tourne Menu.
std::vector<DWORD> LiteMenuProcesses() {
    std::vector<DWORD> found;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return found;
    PROCESSENTRY32W pe{sizeof(pe)};
    for (BOOL more = Process32FirstW(snap, &pe); more; more = Process32NextW(snap, &pe)) {
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
        if (!process) continue;
        WCHAR path[MAX_PATH];
        DWORD size = ARRAYSIZE(path);
        if (QueryFullProcessImageNameW(process, 0, path, &size) && wcsstr(path, kLiteMenuPackage)) found.push_back(pe.th32ProcessID);
        CloseHandle(process);
    }
    CloseHandle(snap);
    return found;
}

// LiteMenu's "exit" item: LiteMenu itself is closed (the Tourne Menu stands in for it).
void CloseLiteMenu() {
    for (DWORD pid : LiteMenuProcesses())
        if (HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, pid)) {
            TerminateProcess(process, 0);
            CloseHandle(process);
        }
}

// The menu's settings: in Tourne Desktop, its own window laid out like LiteMenu's (desktop/host/menu-editor.cpp:
// the menu as a tree, opening on LiteMenu's menu while there are no rows, which OK copies in). In the suite,
// Windhawk's window.
void TmMenuSettings() {
#ifdef TD_PREFIX
    td::OpenMenuEditor();
#else
    OpenOwnSettings(L"taskbar.menu.");
#endif
}

// A tourne: action, or LiteMenu's own command (<lock-computer> and the rest); false when it's neither.
bool TmAction(std::wstring const& command) {
    std::wstring a = command;
    if (a == L"<lock-computer>") a = L"tourne:lock";
    else if (a == L"<restart-computer>") a = L"tourne:restart";
    else if (a == L"<shutdown-computer>") a = L"tourne:shutdown";
    else if (a == L"<setup-litemenu>") a = L"tourne:menu-settings";
    else if (a == L"<close-litemenu>") a = L"tourne:close-litemenu";
    if (a.rfind(L"tourne:", 0) != 0) return false;
    a = a.substr(7);
    // "tourne:do <command>": the command language, so a menu item can do what a
    // key does, several steps and all ("tourne:do workspace 2; wait 200; exec notepad").
    if (a.rfind(L"do ", 0) == 0 || a == L"do") {
        std::wstring command = TmTrim(a.size() > 2 ? a.substr(3) : L"");
        if (command.empty()) {
            Wh_Log(L"this item says tourne:do with no command after it");
            return true;
        }
        std::wstring answer = AskTourne(L"run " + command);
        if (answer.empty())
            Wh_Log(L"\"%s\" needs Key binds on with \"Let scripts control Tourne\"", command.c_str());
        else if (TmTrim(answer).rfind(L"error", 0) == 0)
            Wh_Log(L"\"%s\": %s", command.c_str(), TmTrim(answer).c_str());
        return true;
    }
    if (a == L"show-desktop") ToggleShowDesktop();
    else if (a == L"run") ShowRunDialog();
    else if (a == L"start") ToggleStartMenu();
    else if (a == L"task-manager") Launch(L"taskmgr.exe");
    else if (a == L"windhawk") Launch(WindhawkExe());
    else if (a == L"menu-settings") TmMenuSettings();
    else if (a == L"close-litemenu") CloseLiteMenu();
    else if (a == L"studio") {
#ifdef TD_PREFIX
        td::OpenStudio(L"");
#else
        OpenOwnSettings();
#endif
    } else if (a == L"settings") TmOpen(L"ms-settings:");
    else if (a == L"lock") LockWorkStation();
    else if (a == L"sleep") TmSleep();
    else if (a == L"sign-out") ExitWindowsEx(EWX_LOGOFF, SHTDN_REASON_FLAG_PLANNED);
    else if (a == L"restart") TmPower(true);
    else if (a == L"shutdown") TmPower(false);
    else if (a == L"launcher") {
        if (HWND launcher = FindWindowW(L"TourneLauncher", nullptr))
            PostMessageW(launcher, RegisterWindowMessageW(L"TourneDesktop.Launcher"), 0, 0);
    } else if (a == L"desktop-next") PagerGoTo(std::min(g_pagerDesktops.current + 1, g_pagerDesktops.count - 1));
    else if (a == L"desktop-previous") PagerGoTo(std::max(g_pagerDesktops.current - 1, 0));
    else if (a == L"desktop-new") SendKeys({VK_LCONTROL, VK_LWIN, 'D'});
    else if (a == L"desktop-close") SendKeys({VK_LCONTROL, VK_LWIN, VK_F4});
    else if (a.rfind(L"desktop:", 0) == 0) PagerGoTo(_wtoi(a.c_str() + 8) - 1);
    return true;
}

void TmRun(TourneMenuItem const& item) {
    std::wstring cmd = item.command;
    if (cmd.size() >= 2 && cmd.front() == L'"' && cmd.back() == L'"') cmd = cmd.substr(1, cmd.size() - 2);
    if (TmAction(cmd)) return;
    if (item.shellApp) {
        if (cmd == L"Microsoft.Windows.Shell.RunDialog") ShowRunDialog();
        else TmOpen(L"shell:AppsFolder\\" + cmd);
        return;
    }
    if (!cmd.empty()) TmOpen(Expand(cmd), item.args);
}

HMENU TmBuild(std::vector<TourneMenuItem> const& items, TourneMenuBuild& b) {
    HMENU menu = CreatePopupMenu();
    for (auto const& item : items) {
        if (item.type == 2) {
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        } else if (item.type == 0 || TmIsLiveList(item.command)) {
            HMENU sub = TmBuild(item.items, b);
            if (TmIsLiveList(item.command)) {
                if (!item.items.empty()) AppendMenuW(sub, MF_SEPARATOR, 0, nullptr);
                TmLive(sub, item.command, b);
            }
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)sub, TmLabel(item.name).c_str());
        } else {
            TourneMenuItem copy = item;
            copy.items.clear();
            bool runs = !item.command.empty();
            // (LiteMenu's exit closes LiteMenu: nothing to do once it's gone)
            bool closesLiteMenu = item.command == L"<close-litemenu>" || item.command == L"tourne:close-litemenu";
            if (closesLiteMenu && LiteMenuProcesses().empty()) runs = false;
            std::wstring label = TmLabel(item.name) + (closesLiteMenu && !runs ? L"\t(LiteMenu isn't running)" : L"");
            AppendMenuW(menu, MF_STRING | (runs ? 0 : MF_GRAYED), runs ? TmAdd(b, [copy] { TmRun(copy); }) : 0, label.c_str());
        }
    }
    return menu;
}

// A click outside the open menu (the desktop's, the Tourne Menu) closes it and still reaches what's under it: a right-click elsewhere
// opens a new menu there, a middle-click the Tourne Menu (again, where it was clicked), a left-click picks what it's on. (Windows closes a
// menu only on the left or right button, and keeps that click.)
struct PassedClick {
    UINT down = 0;  // the button's down message, while one is waiting
    POINT at{};
};
thread_local PassedClick t_passedClick;

LRESULT CALLBACK PassClickFilter(int code, WPARAM wParam, LPARAM lParam) {
    if (code == MSGF_MENU && !t_passedClick.down) {
        MSG const* m = (MSG const*)lParam;
        UINT down = 0;
        switch (m->message) {
            case WM_LBUTTONDOWN: case WM_NCLBUTTONDOWN: down = WM_LBUTTONDOWN; break;
            case WM_RBUTTONDOWN: case WM_NCRBUTTONDOWN: down = WM_RBUTTONDOWN; break;
            case WM_MBUTTONDOWN: case WM_NCMBUTTONDOWN: down = WM_MBUTTONDOWN; break;
        }
        WCHAR cls[16] = L"";
        if (down && !(GetClassNameW(WindowFromPoint(m->pt), cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"#32768"))) {
            t_passedClick = {down, m->pt};
            EndMenu();
            return TRUE;  // (taken here, and given again once the menu has gone)
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

// The click again, where it was, now nothing holds the mouse; let go too if the button is up by now.
void PassClickOn() {
    PassedClick c = t_passedClick;
    t_passedClick = {};
    if (!c.down) return;
    // (input and the keys' state are the physical buttons; the messages, after a left-handed swap)
    bool physicalLeft = (c.down == WM_LBUTTONDOWN) != (c.down != WM_MBUTTONDOWN && GetSystemMetrics(SM_SWAPBUTTON));
    DWORD press = c.down == WM_MBUTTONDOWN ? MOUSEEVENTF_MIDDLEDOWN : physicalLeft ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    int key = c.down == WM_MBUTTONDOWN ? VK_MBUTTON : physicalLeft ? VK_LBUTTON : VK_RBUTTON;
    INPUT in[2] = {};
    in[0].type = in[1].type = INPUT_MOUSE;
    in[0].mi.dwFlags = press;
    in[1].mi.dwFlags = press << 1;  // (each button's UP flag is its DOWN flag's next bit)
    SendInput(GetAsyncKeyState(key) & 0x8000 ? 1 : 2, in, sizeof(INPUT));
}

// The menu at the pointer, from the bar (any of them: it only owns the menu).
void ShowTourneMenu(HWND wnd) {
    if (!g_settings.menuEnabled) return;
    std::vector<TourneMenuItem> items = TourneMenuItems();
    if (items.empty()) return;
    TourneMenuBuild b;
    HMENU menu = TmBuild(items, b);
    g_menuFromButton = false;
    t_passedClick = {};
    HHOOK pass = SetWindowsHookExW(WH_MSGFILTER, PassClickFilter, nullptr, GetCurrentThreadId());
    int cmd = RunMenu(wnd, menu);
    if (pass) UnhookWindowsHookEx(pass);
    DestroyMenu(menu);
    if (cmd > 0 && cmd <= (int)b.actions.size()) b.actions[cmd - 1]();
    PassClickOn();
}
