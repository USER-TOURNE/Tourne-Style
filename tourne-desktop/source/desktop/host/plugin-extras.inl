// More of Noctalia's calls answered on Windows, and Tourne's extras for what plugins ported from Linux need,
// included into plugin-runtime.cpp after plugin-system.inl (inside its namespace, with its helpers):
// - noctalia.getenv (with HOME and the XDG folders as Windows has them), runInTerminal, download,
//   readFileAsync, openSettings, setWallpaperMask;
// - the bar's pictures (barWidget.setImage, SVG too) and glyph colour;
// - tourne.processInfo and focusProcess (a program's window, or the terminal it runs in), processes' start
//   time, path and command line;
// - tourne.jsonLines, findFiles and trash: other programs' JSON lines files read a line at a time, a search below a
//   folder, and the Recycle Bin, on threads of their own (Claude Code's transcripts and folders: jq, find and rm on
//   Linux);
// - tourne.screenPick: Tourne's own picker over a frozen picture of the screens (a region, a window, a
//   screen, a colour, or an area to record).
//
// Idle cost: none. A watched bar picture is looked at once a second while a plugin watches it; the picker
// draws only while it's open.

////////////////////////////////////////////////////////////////////////////////
// The environment

// noctalia.getenv(name): Windows' own, and what a Linux plugin looks for where Windows keeps it: HOME
// (%USERPROFILE%), XDG_CONFIG_HOME (%APPDATA%), XDG_DATA_HOME, XDG_STATE_HOME and XDG_CACHE_HOME
// (%LOCALAPPDATA%), XDG_RUNTIME_DIR (%TEMP%), USER (%USERNAME%), when they aren't set.
int ApiGetenv(lua_State* L) {
    std::wstring name = W(Arg(L, 1));
    auto read = [](const std::wstring& n) {
        DWORD size = GetEnvironmentVariableW(n.c_str(), nullptr, 0);
        if (!size) return std::wstring();
        std::wstring v(size, L'\0');
        v.resize(GetEnvironmentVariableW(n.c_str(), v.data(), size));
        return v;
    };
    std::wstring v = read(name);
    if (v.empty()) {
        static const std::map<std::wstring, const wchar_t*> windows = {
            {L"HOME", L"USERPROFILE"}, {L"XDG_CONFIG_HOME", L"APPDATA"}, {L"XDG_DATA_HOME", L"LOCALAPPDATA"},
            {L"XDG_STATE_HOME", L"LOCALAPPDATA"}, {L"XDG_CACHE_HOME", L"LOCALAPPDATA"}, {L"XDG_RUNTIME_DIR", L"TEMP"},
            {L"USER", L"USERNAME"}, {L"LOGNAME", L"USERNAME"}, {L"HOSTNAME", L"COMPUTERNAME"}};
        if (auto it = windows.find(name); it != windows.end()) v = read(it->second);
    }
    if (v.empty()) return lua_pushnil(L), 1;
    std::string s = U(v);
    lua_pushlstring(L, s.data(), s.size());
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Programs in a terminal

// noctalia.runInTerminal(cmd [, cwd [, env]]): a command line of a program the plugin may run (its first word), in a
// new Windows Terminal tab when Windows Terminal is there, else in a console window of its own. cwd (Tourne's
// addition): the folder it starts in. It isn't ended with Tourne Desktop: it's the user's terminal.
// The program argv names, found, if it's one this plugin may run; else empty and why.
std::wstring ProgramFor(Entry& e, const std::vector<std::wstring>& argv, std::string* why) {
    std::wstring exe = argv.empty() ? L"" : FindProgram(argv[0]);
    if (argv.empty()) *why = "no program named";
    else if (!Granted(e, L"program")) *why = "it wasn't allowed to run programs when installed";
    else if (exe.empty()) *why = U(argv[0]) + " isn't installed (or isn't on the PATH)";
    else if (!AllowedPrograms(e).count(ProgramStem(exe))) *why = "it may not run " + U(ProgramStem(exe));
    return why->empty() ? exe : L"";
}

// A working folder a plugin gave: as expandPath reads it, if it's a folder.
std::wstring WorkingFolder(const std::string& given) {
    if (given.empty()) return L"";
    std::wstring dir = Full(ExpandPath(given));
    DWORD a = GetFileAttributesW(dir.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) ? dir : L"";
}

int ApiRunInTerminal(lua_State* L) {
    Entry& e = E(L);
    std::vector<std::wstring> argv = ArgvAt(L, 1);
    std::string why;
    std::wstring exe = ProgramFor(e, argv, &why);
    if (exe.empty()) {
        TellOnce(e, "a terminal wasn't opened: " + why);
        return FailBool(L, why);
    }
    std::wstring dir = WorkingFolder(OptArg(L, 2));
    std::wstring line = QuoteArg(exe);
    for (size_t i = 1; i < argv.size(); i++) line += L" " + QuoteArg(argv[i]);
    // env (Tourne's addition): {NAME = value} added to the program's environment (CLAUDE_CONFIG_DIR for another
    // account). A new tab of a Windows Terminal that's already open would start with that window's own, so it gets a
    // console window of its own instead.
    struct AnyCase {
        bool operator()(const std::wstring& a, const std::wstring& b) const { return _wcsicmp(a.c_str(), b.c_str()) < 0; }
    };
    std::map<std::wstring, std::wstring, AnyCase> env;
    if (lua_istable(L, 3)) {
        lua_pushnil(L);
        while (lua_next(L, 3)) {
            if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING) {
                std::wstring name = W(lua_tostring(L, -2));
                if (!name.empty() && name.find(L'=') == std::wstring::npos) env[name] = W(lua_tostring(L, -1));
            }
            lua_pop(L, 1);
        }
    }
    std::wstring block;
    if (!env.empty()) {
        if (LPWCH own = GetEnvironmentStringsW()) {
            for (LPWCH at = own; *at; at += wcslen(at) + 1) {
                std::wstring pair = at;
                size_t eq = pair.find(L'=', 1);
                if (eq != std::wstring::npos && !env.count(pair.substr(0, eq))) env[pair.substr(0, eq)] = pair.substr(eq + 1);
            }
            FreeEnvironmentStringsW(own);
        }
        for (auto const& [name, value] : env) block += name + L"=" + value + L'\0';
        block += L'\0';
    }
    std::wstring terminal = env.empty() ? FindProgram(L"wt") : L"";
    std::wstring command;
    if (!terminal.empty()) {
        std::wstring escaped;  // (Windows Terminal reads ";" as the start of another command)
        for (wchar_t c : line) escaped += c == L';' ? std::wstring(L"\\;") : std::wstring(1, c);
        command = QuoteArg(terminal) + L" new-tab" + (dir.empty() ? L"" : L" -d " + QuoteArg(dir)) + L" -- " + escaped;
    }
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    bool ok = !terminal.empty() ? CreateProcessW(terminal.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr,
                                                 dir.empty() ? nullptr : dir.c_str(), &si, &pi)
                                : CreateProcessW(exe.c_str(), line.data(), nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
                                                 block.empty() ? nullptr : block.data(), dir.empty() ? nullptr : dir.c_str(), &si, &pi);
    if (ok) {
        CloseHandle(pi.hThread), CloseHandle(pi.hProcess);
        Remember(Name(e) + L" opened a terminal: " + line.substr(0, 200));
    }
    lua_pushboolean(L, ok);
    return 1;
}

// tourne.launch(argvOrLine [, cwd]): starts an app of its program list on its own, as the taskbar starts one: no
// output comes back, it isn't ended after a minute, and it stays open when the plugin stops or Tourne Desktop
// restarts (runAsync's and runStream's programs end with them). An editor, Discord. True if it started.
int ApiLaunch(lua_State* L) {
    Entry& e = E(L);
    std::vector<std::wstring> argv = ArgvAt(L, 1);
    std::string why;
    std::wstring exe = ProgramFor(e, argv, &why);
    if (exe.empty()) {
        TellOnce(e, "an app wasn't started: " + why);
        return FailBool(L, why);
    }
    std::wstring dir = WorkingFolder(OptArg(L, 2)), args;
    for (size_t i = 1; i < argv.size(); i++) args += (i > 1 ? L" " : L"") + QuoteArg(argv[i]);
    AllowSetForegroundWindow(ASFW_ANY);  // (its window comes to the front)
    SHELLEXECUTEINFOW info{sizeof(info)};
    info.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
    info.lpVerb = L"open";
    info.lpFile = exe.c_str();
    info.lpParameters = args.empty() ? nullptr : args.c_str();
    info.lpDirectory = dir.empty() ? nullptr : dir.c_str();
    info.nShow = SW_SHOWNORMAL;
    bool ok = ShellExecuteExW(&info);
    if (ok) Remember(Name(e) + L" started " + (QuoteArg(exe) + L" " + args).substr(0, 200));
    lua_pushboolean(L, ok);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Files and the web, off the runtime's thread

// noctalia.readFileAsync(path, onResult(contents, err)): read on a thread of its own.
int ApiReadFileAsync(lua_State* L) {
    Entry& e = E(L);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    std::wstring path;
    std::string why;
    bool allowed = Resolve(e, Arg(L, 1), false, &path, &why);
    int ref = lua_ref(L, 2);
    uint64_t serial = e.serial;
    std::thread([=] {
        std::string body, error = why;
        if (allowed && !ReadAll(path, &body)) error = "couldn't read it";
        Post([=, body = std::move(body)] {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) return;
            Entry& to = *found->second;
            Guarded(to, [&] {
                lua_settop(to.T, 0);
                lua_getref(to.T, ref);
                lua_unref(to.T, ref);
                if (error.empty()) lua_pushlstring(to.T, body.data(), body.size()), lua_pushnil(to.T);
                else lua_pushnil(to.T), lua_pushstring(to.T, error.c_str());
                return Call(to, 2);
            });
        });
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

// Why the plugin may not fetch this address (empty: it may), and its parts.
std::string HttpRefused(Entry& e, const std::string& url, std::wstring* host, std::wstring* path, INTERNET_PORT* port, bool* secure) {
    std::wstring wurl = W(url);
    URL_COMPONENTSW parts{sizeof(parts)};
    wchar_t h[256] = L"", p[4096] = L"";
    parts.lpszHostName = h, parts.dwHostNameLength = 256;
    parts.lpszUrlPath = p, parts.dwUrlPathLength = 4096;
    wchar_t extra[4096] = L"";
    parts.lpszExtraInfo = extra, parts.dwExtraInfoLength = 4096;
    bool parsed = WinHttpCrackUrl(wurl.c_str(), 0, 0, &parts);
    if (!Granted(e, L"internet")) return "it wasn't allowed the internet when installed";
    if (!parsed || (parts.nScheme != INTERNET_SCHEME_HTTPS && parts.nScheme != INTERNET_SCHEME_HTTP)) return "not a web address: " + url;
    *host = h, *path = std::wstring(p) + extra, *port = parts.nPort, *secure = parts.nScheme == INTERNET_SCHEME_HTTPS;
    if (!e.plugin->hosts.empty()) {
        bool listed = false;
        for (auto const& allowed : e.plugin->hosts)
            if (!_wcsicmp(h, allowed.c_str()) || (wcslen(h) > allowed.size() && !_wcsicmp(h + wcslen(h) - allowed.size(), allowed.c_str()) &&
                                                  h[wcslen(h) - allowed.size() - 1] == L'.'))
                listed = true;
        if (!listed) return "it may reach only the hosts it was installed with, not " + U(h);
    }
    return "";
}

// noctalia.download(url, destPath, onDone(ok)): a GET saved to a file where writeFile may write (whole, or not
// at all: it's written beside it first). At most 512 MB.
int ApiDownload(lua_State* L) {
    Entry& e = E(L);
    std::string url = Arg(L, 1);
    std::wstring dest;
    std::string why;
    bool hasCallback = lua_isfunction(L, 3);
    int ref = hasCallback ? lua_ref(L, 3) : LUA_NOREF;
    uint64_t serial = e.serial;
    auto done = [serial, ref](bool ok) {
        if (ref == LUA_NOREF) return;
        Post([=] {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) return;
            Entry& to = *found->second;
            Guarded(to, [&] {
                lua_settop(to.T, 0);
                lua_getref(to.T, ref);
                lua_unref(to.T, ref);
                lua_pushboolean(to.T, ok);
                return Call(to, 1);
            });
        });
    };
    std::wstring host, path;
    INTERNET_PORT port = 0;
    bool secure = true;
    if (!Resolve(e, Arg(L, 2), true, &dest, &why) || !(why = HttpRefused(e, url, &host, &path, &port, &secure)).empty()) {
        TellOnce(e, "a download was refused: " + why);
        done(false);
        lua_pushboolean(L, false);
        return 1;
    }
    std::thread([=] {
        HINTERNET session = WinHttpOpen(L"Tourne Desktop", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        HINTERNET connect = session ? WinHttpConnect(session, host.c_str(), port, 0) : nullptr;
        HINTERNET request = connect ? WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                         secure ? WINHTTP_FLAG_SECURE : 0)
                                    : nullptr;
        if (session) WinHttpSetTimeouts(session, 10000, 10000, 15000, 60000);
        bool ok = request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                  WinHttpReceiveResponse(request, nullptr);
        DWORD status = 0, size = sizeof(status);
        if (ok) WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size, nullptr);
        ok = ok && status >= 200 && status < 300;
        std::wstring temp = dest + L".downloading";
        HANDLE file = ok ? CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr) : INVALID_HANDLE_VALUE;
        ok = file != INVALID_HANDLE_VALUE;
        unsigned long long total = 0;
        std::vector<char> buffer(64 << 10);
        while (ok) {
            DWORD got = 0, written = 0;
            if (!WinHttpReadData(request, buffer.data(), (DWORD)buffer.size(), &got)) ok = false;
            if (!ok || !got) break;
            total += got;
            if (total > (512ull << 20) || !WriteFile(file, buffer.data(), got, &written, nullptr) || written != got) ok = false;
        }
        if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
        ok = ok && MoveFileExW(temp.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING);
        if (!ok) DeleteFileW(temp.c_str());
        if (request) WinHttpCloseHandle(request);
        if (connect) WinHttpCloseHandle(connect);
        if (session) WinHttpCloseHandle(session);
        done(ok);
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

// noctalia.openSettings(): Tourne Studio's Plugins page, found at this plugin.
int ApiOpenSettings(lua_State* L) {
    Entry& e = E(L);
    std::string name = e.plugin->manifest["name"].text;
    OpenStudio(L"plugins.", W(name.empty() ? U(e.plugin->id) : name));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Wallpaper depth

// noctalia.setWallpaperMask(connector, {path = mask, wallpaperPath = wallpaper} or nil): desktop widgets on that
// screen drawn under the wallpaper's foreground (plugin-panel.cpp). The mask is one of the plugin's own files; the
// wallpaper is the one showing or one in the wallpaper folder.
int ApiSetWallpaperMask(lua_State* L) {
    Entry& e = E(L);
    std::wstring screen = W(Arg(L, 1));
    if (!Granted(e, L"wallpaper")) {
        TellOnce(e, "noctalia.setWallpaperMask: it wasn't allowed the wallpaper when installed");
        return 0;
    }
    if (!lua_istable(L, 2)) {
        panel::SetDepthMask(screen, L"", L"");
        return 0;
    }
    lua_getfield(L, 2, "path");
    lua_getfield(L, 2, "wallpaperPath");
    std::string maskGiven = lua_isstring(L, -2) ? lua_tostring(L, -2) : "", wallGiven = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_pop(L, 2);
    std::wstring mask, wall;
    std::string why;
    if (!Resolve(e, maskGiven, false, &mask, &why)) {
        TellOnce(e, "noctalia.setWallpaperMask: the mask " + why);
        return 0;
    }
    wall = Full(ExpandPath(wallGiven));
    std::wstring now = Full(CurrentWallpaperPath());
    if (!(CompareStringOrdinal(wall.c_str(), -1, now.c_str(), -1, TRUE) == CSTR_EQUAL) && !Resolve(e, wallGiven, false, &wall, &why)) {
        TellOnce(e, "noctalia.setWallpaperMask: the wallpaper " + why);
        return 0;
    }
    panel::SetDepthMask(screen, mask, wall);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// The bar's pictures

// The pixels the bar draws for an entry's picture, kept in a small file of its own (the bar is told its path):
// "TBI1", width and height (32 bits each), then premultiplied BGRA rows, top-down.
std::wstring BarPictureFolder() {
    wchar_t temp[MAX_PATH] = L"";
    GetTempPathW(MAX_PATH, temp);
    std::wstring folder = std::wstring(temp) + L"TourneDesktop-bar";
    CreateDirectoryW(folder.c_str(), nullptr);
    return folder;
}

// Pixels fitted in a box (its shape kept, an area average when it's smaller), kept in a file for the bar; its
// path, or "" when it couldn't be written.
std::wstring WriteBarPicture(const std::vector<uint32_t>& px, UINT w, UINT h, int boxW, int boxH, uint64_t serial) {
    double k = std::min((double)boxW / w, (double)boxH / h);
    UINT ow = std::max(1u, (UINT)std::lround(w * k)), oh = std::max(1u, (UINT)std::lround(h * k));
    std::vector<uint32_t> out((size_t)ow * oh);
    for (UINT y = 0; y < oh; y++)
        for (UINT x = 0; x < ow; x++) {
            UINT x0 = x * w / ow, x1 = std::max(x0 + 1, (x + 1) * w / ow), y0 = y * h / oh, y1 = std::max(y0 + 1, (y + 1) * h / oh);
            uint64_t sum[4] = {0, 0, 0, 0}, n = 0;
            for (UINT sy = y0; sy < y1 && sy < h; sy++)
                for (UINT sx = x0; sx < x1 && sx < w; sx++, n++) {
                    uint32_t p = px[(size_t)sy * w + sx];
                    for (int c = 0; c < 4; c++) sum[c] += p >> (c * 8) & 0xFF;
                }
            uint32_t v = 0;
            for (int c = 0; c < 4; c++) v |= (uint32_t)(n ? sum[c] / n : 0) << (c * 8);
            out[(size_t)y * ow + x] = v;
        }
    static unsigned counter = 0;
    std::wstring path = BarPictureFolder() + L"\\" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(serial) + L"-" +
                        std::to_wstring(++counter) + L".tbi";
    std::string bytes = "TBI1";
    bytes.append((const char*)&ow, 4).append((const char*)&oh, 4).append((const char*)out.data(), out.size() * 4);
    return WriteAll(path, bytes) ? path : L"";
}

double ScreenScale() {
    HDC screen = GetDC(nullptr);
    double scale = GetDeviceCaps(screen, LOGPIXELSX) / 96.0;
    ReleaseDC(nullptr, screen);
    return scale;
}

// Draws the entry's picture at its size (96-dpi pixels, at the screen's scale) for the bar.
void RenderBarPicture(Entry& e) {
    if (!e.imageShown.empty()) DeleteFileW(e.imageShown.c_str()), e.imageShown.clear();
    if (e.imageSource.empty()) return BarChanged();
    WIN32_FILE_ATTRIBUTE_DATA a{};
    GetFileAttributesExW(e.imageSource.c_str(), GetFileExInfoStandard, &a);
    e.imageWritten = a.ftLastWriteTime;
    double scale = ScreenScale();
    int boxW = (int)std::lround((e.imageW > 0 ? e.imageW : 18) * scale), boxH = (int)std::lround((e.imageH > 0 ? e.imageH : 18) * scale);
    std::vector<uint32_t> px;
    UINT w = 0, h = 0;
    if (!panel::ReadPicturePixels(e.imageSource, std::max(boxW, boxH) * 2, &px, &w, &h)) {
        TellOnce(e, "barWidget.setImage: the picture didn't read");
        return BarChanged();
    }
    e.imageShown = WriteBarPicture(px, w, h, boxW, boxH, e.serial);
    BarChanged();
}

// The user's own icons for bar items (Shift+right-click on one, Change icon; kPluginIconsValue): each drawn
// in place of the item's glyph or picture, at the same size, into a picture file of its own.
struct IconSwap {
    std::wstring source, shown;
    bool tried = false;  // (drawn once: one that didn't read isn't tried again on every tick)
};
std::map<std::wstring, IconSwap> g_iconSwaps;  // by the item's place key, "plugin:<key>"

void ReadIconSwaps() {
    auto rows = LoadPluginIcons();
    for (auto it = g_iconSwaps.begin(); it != g_iconSwaps.end();) {
        auto row = std::find_if(rows.begin(), rows.end(), [&](auto const& r) { return r.first == it->first; });
        if (row != rows.end() && row->second == it->second.source) {
            ++it;
            continue;
        }
        if (!it->second.shown.empty()) DeleteFileW(it->second.shown.c_str());
        it = g_iconSwaps.erase(it);
    }
    for (auto const& [key, file] : rows)
        if (!g_iconSwaps.count(key)) g_iconSwaps[key].source = file;
}

// An icon's pixels, premultiplied: drawn over black and over white, the difference is how see-through each is.
bool IconPixels(HICON icon, int size, std::vector<uint32_t>* out) {
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
    HDC dc = CreateCompatibleDC(nullptr);
    void* bits[2] = {};
    HBITMAP maps[2] = {CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits[0], nullptr, 0), CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits[1], nullptr, 0)};
    bool ok = maps[0] && maps[1];
    for (int i = 0; ok && i < 2; i++) {
        HGDIOBJ old = SelectObject(dc, maps[i]);
        memset(bits[i], i ? 0xFF : 0, (size_t)size * size * 4);
        DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        GdiFlush();
        SelectObject(dc, old);
    }
    if (ok) {
        out->assign((size_t)size * size, 0);
        auto const* black = (const uint32_t*)bits[0];
        auto const* white = (const uint32_t*)bits[1];
        for (size_t i = 0; i < out->size(); i++) {
            int a = 255 - std::clamp((int)(white[i] & 0xFF) - (int)(black[i] & 0xFF), 0, 255);
            (*out)[i] = a ? (uint32_t)a << 24 | (black[i] & 0xFFFFFF) : 0;
        }
    }
    for (HBITMAP m : maps)
        if (m) DeleteObject(m);
    DeleteDC(dc);
    return ok;
}

// Draws an item's own icon at the size of what it stands in for: the entry's picture, or its glyph (16).
std::wstring RenderIconSwap(const Entry& e, const std::wstring& source) {
    double scale = ScreenScale();
    bool picture = !e.imageSource.empty();
    int boxW = (int)std::lround((picture && e.imageW > 0 ? e.imageW : picture ? 18 : 16) * scale);
    int boxH = (int)std::lround((picture && e.imageH > 0 ? e.imageH : picture ? 18 : 16) * scale);
    std::wstring path = source;
    int index = 0;
    size_t comma = path.rfind(L',');
    bool numbered = comma != std::wstring::npos && comma + 1 < path.size() && (iswdigit(path[comma + 1]) || path[comma + 1] == L'-');
    if (numbered) index = _wtoi(path.c_str() + comma + 1), path.resize(comma);
    std::wstring ext = path.size() > 4 ? path.substr(path.size() - 4) : L"";
    for (auto& c : ext) c = towlower(c);
    std::vector<uint32_t> px;
    UINT w = 0, h = 0;
    if (numbered || ext == L".ico" || ext == L".exe" || ext == L".dll" || ext == L".icl" || ext == L".cpl") {
        // An icon (of a program or library, or a .ico): the one nearest the size, from Windows.
        int size = std::max(boxW, boxH);
        HICON icon = nullptr;
        UINT id = 0;
        if (PrivateExtractIconsW(path.c_str(), index, size, size, &icon, &id, 1, LR_DEFAULTCOLOR) != 1 || !icon) return L"";
        bool ok = IconPixels(icon, size, &px);
        DestroyIcon(icon);
        if (!ok) return L"";
        w = h = (UINT)size;
    } else if (!panel::ReadPicturePixels(path, std::max(boxW, boxH) * 2, &px, &w, &h)) {
        return L"";
    }
    return WriteBarPicture(px, w, h, boxW, boxH, e.serial);
}

// The item's own icon's picture file, drawn the first time it's asked for; "" when it has none.
std::wstring IconSwapFor(const Entry& e, const std::wstring& key) {
    auto it = g_iconSwaps.find(L"plugin:" + key);
    if (it == g_iconSwaps.end()) return L"";
    if (!it->second.tried) {
        it->second.tried = true;
        it->second.shown = RenderIconSwap(e, it->second.source);
        if (it->second.shown.empty()) Log(L"the icon %ls for %ls didn't read", it->second.source.c_str(), key.c_str());
    }
    return it->second.shown;
}

// barWidget.setImage(path [, watch, width, height]): a picture in place of its glyph (its own or its data folder's;
// PNG, JPEG, BMP, GIF, ICO or SVG); "" takes it away. watch: drawn again when the file changes.
int BarSetImage(lua_State* L) {
    Entry& e = E(L);
    std::string given = OptArg(L, 1);
    e.imageWatch = lua_toboolean(L, 2);
    e.imageW = (int)luaL_optnumber(L, 3, 0), e.imageH = (int)luaL_optnumber(L, 4, 0);
    if (e.imageW > 0 && e.imageH <= 0) e.imageH = e.imageW;
    if (e.imageH > 0 && e.imageW <= 0) e.imageW = e.imageH;
    e.imageW = std::min(e.imageW, 256), e.imageH = std::min(e.imageH, 64);
    std::wstring full;
    std::string why;
    if (given.empty()) e.imageSource.clear();
    else if (!Resolve(e, given, false, &full, &why)) {
        TellOnce(e, "barWidget.setImage: " + why);
        e.imageSource.clear();
    } else e.imageSource = full;
    RenderBarPicture(e);
    return 0;
}

// Watched pictures drawn again when their file changed (looked at once a second, only while one is watched).
void CheckBarPictures() {
    static ULONGLONG last = 0;
    ULONGLONG now = GetTickCount64();
    if (now - last < 1000) return;
    last = now;
    for (auto& p : g_plugins)
        for (auto& e : p->entries) {
            if (!e->L || !e->imageWatch || e->imageSource.empty()) continue;
            WIN32_FILE_ATTRIBUTE_DATA a{};
            if (GetFileAttributesExW(e->imageSource.c_str(), GetFileExInfoStandard, &a) && CompareFileTime(&a.ftLastWriteTime, &e->imageWritten))
                RenderBarPicture(*e);
        }
}

bool WatchingBarPictures() {
    for (auto& p : g_plugins)
        for (auto& e : p->entries)
            if (e->L && e->imageWatch && !e->imageSource.empty()) return true;
    return false;
}

int BarSetGlyphColor(lua_State* L) {
    E(L).glyphColor = lua_isstring(L, 1) ? RoleColor(lua_tostring(L, 1)) : CLR_INVALID;
    BarChanged();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Processes: one's details, and its window

struct ProcessDetails {
    ULONGLONG created = 0;  // (FILETIME units: with the pid, which process it is)
    std::wstring path, commandLine;
};

ULONGLONG CreateTimeOf(HANDLE process) {
    FILETIME created{}, exited{}, kernel{}, user{};
    if (!GetProcessTimes(process, &created, &exited, &kernel, &user)) return 0;
    return (ULONGLONG)created.dwHighDateTime << 32 | created.dwLowDateTime;
}

// A process's exe and command line (when Windows lets Tourne read them), kept while the process lives.
ProcessDetails DetailsOf(DWORD pid) {
    static std::unordered_map<DWORD, ProcessDetails> known;
    ProcessDetails d;
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return d;
    d.created = CreateTimeOf(h);
    if (auto it = known.find(pid); it != known.end() && it->second.created == d.created) {
        CloseHandle(h);
        return it->second;
    }
    wchar_t path[MAX_PATH * 2];
    DWORD size = ARRAYSIZE(path);
    if (QueryFullProcessImageNameW(h, 0, path, &size)) d.path.assign(path, size);
    using Query = LONG(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    static Query query = (Query)(void*)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");
    std::vector<BYTE> buffer(8192);
    ULONG need = 0;
    LONG status = query ? query(h, 60 /*ProcessCommandLineInformation*/, buffer.data(), (ULONG)buffer.size(), &need) : -1;
    if (status == (LONG)0xC0000004L && need && need < (1u << 20)) {
        buffer.resize(need);
        status = query(h, 60, buffer.data(), (ULONG)buffer.size(), &need);
    }
    if (status >= 0) {
        auto* text = (UNICODE_STRING*)buffer.data();
        if (text->Buffer) d.commandLine.assign(text->Buffer, text->Length / sizeof(wchar_t));
    }
    CloseHandle(h);
    if (known.size() > 4096) known.clear();
    known[pid] = d;
    return d;
}

double MsSince1970(ULONGLONG filetime) { return filetime ? (double)((filetime - 116444736000000000ULL) / 10000ULL) : 0; }

// tourne.processInfo(pid): {pid, parent, name, started (ms since 1970), path, commandLine}, or nil when it isn't running.
int ApiProcessInfo(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"systemInfo")) return FailBool(L, "it wasn't allowed to read system information when installed");
    DWORD pid = (DWORD)luaL_checkinteger(L, 1);
    for (auto const& s : SampleProcesses()) {
        if (s.pid != pid) continue;
        ProcessDetails d = DetailsOf(pid);
        lua_createtable(L, 0, 6);
        lua_pushinteger(L, (int)s.pid);
        lua_setfield(L, -2, "pid");
        lua_pushinteger(L, (int)s.parent);
        lua_setfield(L, -2, "parent");
        lua_pushstring(L, U(s.name).c_str());
        lua_setfield(L, -2, "name");
        if (d.created) SetNumber(L, "started", MsSince1970(d.created));
        if (!d.path.empty()) lua_pushstring(L, U(d.path).c_str()), lua_setfield(L, -2, "path");
        if (!d.commandLine.empty()) lua_pushstring(L, U(d.commandLine).c_str()), lua_setfield(L, -2, "commandLine");
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

struct WindowOf {
    DWORD pid;
    HWND found = nullptr;
};

BOOL CALLBACK FindProcessWindow(HWND w, LPARAM p) {
    auto& f = *(WindowOf*)p;
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (pid != f.pid || !IsWindowVisible(w) || GetWindow(w, GW_OWNER) || (GetWindowLongW(w, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) return TRUE;
    BOOL cloaked = FALSE;
    DwmGetWindowAttribute(w, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked) return TRUE;
    f.found = w;
    return FALSE;
}

// tourne.focusProcess(pid): its window brought forward, or (a program in a terminal) the window of the nearest
// process it was started from that has one. True when one came forward.
int ApiFocusProcess(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"systemInfo")) return FailBool(L, "it wasn't allowed to read system information when installed");
    DWORD pid = (DWORD)luaL_checkinteger(L, 1);
    std::unordered_map<DWORD, DWORD> parents;
    for (auto const& s : SampleProcesses()) parents[s.pid] = s.parent;
    HWND window = nullptr;
    ULONGLONG childStarted = ~0ULL;
    for (int hop = 0; hop < 16 && pid > 4 && parents.count(pid); hop++) {
        ULONGLONG started = DetailsOf(pid).created;
        if (started && started > childStarted) break;  // (a parent that ended, its number taken by a newer one)
        WindowOf f{pid};
        EnumWindows(FindProcessWindow, (LPARAM)&f);
        if ((window = f.found)) break;
        if (started) childStarted = started;
        pid = parents[pid];
    }
    if (!window) return lua_pushboolean(L, false), 1;
    if (IsIconic(window)) ShowWindow(window, SW_RESTORE);
    SetForegroundWindow(window);
    if (GetForegroundWindow() != window) {  // (Tourne isn't in front, so Windows may refuse: as the Start menu does it)
        HWND front = GetForegroundWindow();
        DWORD frontThread = front ? GetWindowThreadProcessId(front, nullptr) : 0, me = GetCurrentThreadId();
        bool attached = frontThread && frontThread != me && AttachThreadInput(me, frontThread, TRUE);
        SetForegroundWindow(window);
        BringWindowToTop(window);
        if (attached) AttachThreadInput(me, frontThread, FALSE);
    }
    lua_pushboolean(L, true);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// The screen picker

// One at a time: the screens as they were when it opened, and what's under the pointer.
struct ScreenPick {
    uint64_t serial = 0;
    int ref = LUA_NOREF;
    std::string mode;  // region, window, screen, color, area
    std::wstring save;  // where its picture goes (region, window, screen), resolved
    HWND wnd = nullptr;
    RECT screens{};  // the virtual screen
    HBITMAP shot = nullptr, dim = nullptr;
    uint32_t* shotBits = nullptr;
    POINT down{}, now{};
    bool dragging = false;
    std::vector<RECT> windows;  // top-level windows, front to back, where they show
    RECT hover{};
    bool done = false;
};
ScreenPick* g_pick = nullptr;
constexpr wchar_t kPickClass[] = L"TourneDesktopScreenPick";

std::wstring ScreenNameAt(POINT p) {
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(MonitorFromPoint(p, MONITOR_DEFAULTTONEAREST), &info)) return L"";
    std::wstring name = info.szDevice;
    return name.rfind(L"\\\\.\\", 0) == 0 ? name.substr(4) : name;
}

BOOL CALLBACK ListPickWindows(HWND w, LPARAM p) {
    auto& list = *(std::vector<RECT>*)p;
    if (!IsWindowVisible(w) || IsIconic(w)) return TRUE;
    wchar_t cls[64] = L"";
    GetClassNameW(w, cls, 64);
    if (!wcscmp(cls, L"Progman") || !wcscmp(cls, L"WorkerW") || !wcscmp(cls, kPickClass)) return TRUE;
    BOOL cloaked = FALSE;
    DwmGetWindowAttribute(w, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked || (GetWindowLongW(w, GWL_EXSTYLE) & WS_EX_TRANSPARENT)) return TRUE;
    RECT r{};
    if (FAILED(DwmGetWindowAttribute(w, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(w, &r);
    if (r.right - r.left < 8 || r.bottom - r.top < 8) return TRUE;
    list.push_back(r);
    return TRUE;
}

RECT PickRect(const ScreenPick& p) {
    if (p.mode == "region" || p.mode == "area")
        return {std::min(p.down.x, p.now.x), std::min(p.down.y, p.now.y), std::max(p.down.x, p.now.x) + 1, std::max(p.down.y, p.now.y) + 1};
    return p.hover;
}

void PickHover(ScreenPick& p) {
    p.hover = {};
    if (p.mode == "window") {
        for (auto const& r : p.windows)
            if (PtInRect(&r, p.now)) {
                p.hover = r;
                break;
            }
        IntersectRect(&p.hover, &p.hover, &p.screens);
    } else if (p.mode == "screen") {
        MONITORINFO mi{sizeof(mi)};
        if (GetMonitorInfoW(MonitorFromPoint(p.now, MONITOR_DEFAULTTONEAREST), &mi)) p.hover = mi.rcMonitor;
    }
}

uint32_t PickPixel(const ScreenPick& p, POINT at) {
    int w = p.screens.right - p.screens.left, h = p.screens.bottom - p.screens.top;
    int x = std::clamp((int)(at.x - p.screens.left), 0, w - 1), y = std::clamp((int)(at.y - p.screens.top), 0, h - 1);
    return p.shotBits ? p.shotBits[(size_t)y * w + x] & 0xFFFFFF : 0;
}

// Its result for the plugin (nil: cancelled), after the window has gone.
void PickFinish(bool picked) {
    ScreenPick* p = g_pick;
    if (!p || p->done) return;
    p->done = true;
    RECT r = PickRect(*p);
    std::wstring savedTo;
    std::string color;
    if (picked && p->mode == "color") {
        uint32_t c = PickPixel(*p, p->now);
        char hex[8];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", c >> 16 & 0xFF, c >> 8 & 0xFF, c & 0xFF);
        color = hex;
        r = {p->now.x, p->now.y, p->now.x + 1, p->now.y + 1};
    }
    if (picked && p->mode != "color" && p->mode != "area" && !p->save.empty() && !IsRectEmpty(&r)) {
        int sw = p->screens.right - p->screens.left, w = r.right - r.left, h = r.bottom - r.top;
        std::vector<uint32_t> px((size_t)w * h);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                px[(size_t)y * w + x] = p->shotBits[(size_t)(r.top - p->screens.top + y) * sw + (r.left - p->screens.left + x)] | 0xFF000000;
        std::string png;
        std::error_code ignored;
        std::filesystem::create_directories(std::filesystem::path(p->save).parent_path(), ignored);
        if (EncodePng(px, w, h, &png) && WriteAll(p->save, png)) savedTo = p->save;
    }
    picked = picked && !IsRectEmpty(&r);
    POINT center{(r.left + r.right) / 2, (r.top + r.bottom) / 2};
    std::wstring screen = ScreenNameAt(center);
    uint64_t serial = p->serial;
    int ref = p->ref;
    if (p->wnd) DestroyWindow(p->wnd);
    if (p->shot) DeleteObject(p->shot);
    if (p->dim) DeleteObject(p->dim);
    delete p;
    g_pick = nullptr;
    Later([=] {
        auto found = g_bySerial.find(serial);
        if (found == g_bySerial.end()) return;
        Entry& to = *found->second;
        Guarded(to, [&] {
            lua_settop(to.T, 0);
            lua_getref(to.T, ref);
            lua_unref(to.T, ref);
            if (!picked) {
                lua_pushnil(to.T);
                return Call(to, 1);
            }
            lua_createtable(to.T, 0, 7);
            SetNumber(to.T, "x", r.left);
            SetNumber(to.T, "y", r.top);
            SetNumber(to.T, "w", r.right - r.left);
            SetNumber(to.T, "h", r.bottom - r.top);
            lua_pushstring(to.T, U(screen).c_str());
            lua_setfield(to.T, -2, "connector");
            if (!savedTo.empty()) lua_pushstring(to.T, U(savedTo).c_str()), lua_setfield(to.T, -2, "path");
            if (!color.empty()) lua_pushstring(to.T, color.c_str()), lua_setfield(to.T, -2, "color");
            return Call(to, 1);
        });
    });
    if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
}

void PickPaint(ScreenPick& p, HDC screenDc) {
    int w = p.screens.right - p.screens.left, h = p.screens.bottom - p.screens.top;
    HDC dc = CreateCompatibleDC(screenDc), src = CreateCompatibleDC(screenDc);
    HBITMAP canvas = CreateCompatibleBitmap(screenDc, w, h);
    HGDIOBJ oldCanvas = SelectObject(dc, canvas), oldSrc = SelectObject(src, p.dim);
    BitBlt(dc, 0, 0, w, h, src, 0, 0, SRCCOPY);
    SelectObject(src, p.shot);
    COLORREF accent = RoleColor("primary");
    auto frame = [&](RECT r) {
        HBRUSH b = CreateSolidBrush(accent);
        for (int i = 0; i < 2; i++) {
            FrameRect(dc, &r, b);
            InflateRect(&r, 1, 1);
        }
        DeleteObject(b);
    };
    auto label = [&](const std::wstring& text, POINT at) {
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, RGB(0x12, 0x1C, 0x21));
        SetTextColor(dc, accent);
        HGDIOBJ old = SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));
        TextOutW(dc, at.x, at.y, (L" " + text + L" ").c_str(), (int)text.size() + 2);
        SelectObject(dc, old);
    };
    if (p.mode == "color") {
        // A magnifier beside the pointer: 15 by 15 pixels, 8 times as big, the middle one framed, and its colour.
        int cell = 8, side = 15 * cell;
        POINT at{p.now.x - p.screens.left + 24, p.now.y - p.screens.top + 24};
        if (at.x + side > w) at.x -= side + 48;
        if (at.y + side + 24 > h) at.y -= side + 72;
        SetStretchBltMode(dc, COLORONCOLOR);
        StretchBlt(dc, at.x, at.y, side, side, src, p.now.x - p.screens.left - 7, p.now.y - p.screens.top - 7, 15, 15, SRCCOPY);
        frame({at.x, at.y, at.x + side, at.y + side});
        RECT middle{at.x + 7 * cell, at.y + 7 * cell, at.x + 8 * cell, at.y + 8 * cell};
        FrameRect(dc, &middle, (HBRUSH)GetStockObject(WHITE_BRUSH));
        uint32_t c = PickPixel(p, p.now);
        wchar_t hex[16];
        swprintf(hex, 16, L"#%02X%02X%02X", c >> 16 & 0xFF, c >> 8 & 0xFF, c & 0xFF);
        label(hex, {at.x, at.y + side + 4});
    } else {
        RECT r = PickRect(p);
        bool show = p.mode == "region" || p.mode == "area" ? p.dragging : !IsRectEmpty(&r);
        if (show && !IsRectEmpty(&r)) {
            OffsetRect(&r, -p.screens.left, -p.screens.top);
            BitBlt(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, src, r.left, r.top, SRCCOPY);
            frame(r);
            label(std::to_wstring(r.right - r.left) + L" × " + std::to_wstring(r.bottom - r.top), {r.left, std::max(0L, r.top - 22)});
        }
        static const std::map<std::string, std::wstring> hints = {
            {"region", L"Drag over what to take. Right-click or Esc: cancel"}, {"area", L"Drag over what to record. Right-click or Esc: cancel"},
            {"window", L"Click a window. Right-click or Esc: cancel"}, {"screen", L"Click a screen. Right-click or Esc: cancel"}};
        if (auto it = hints.find(p.mode); it != hints.end()) {
            POINT corner{p.now.x - p.screens.left + 18, p.now.y - p.screens.top + 18};
            if (!p.dragging) label(it->second, corner);
        }
    }
    BitBlt(screenDc, 0, 0, w, h, dc, 0, 0, SRCCOPY);
    SelectObject(src, oldSrc);
    SelectObject(dc, oldCanvas);
    DeleteObject(canvas);
    DeleteDC(src);
    DeleteDC(dc);
}

LRESULT CALLBACK PickProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    ScreenPick* p = g_pick;
    if (!p || p->wnd != wnd) return DefWindowProcW(wnd, msg, wp, lp);
    auto pointer = [&] {
        POINT at{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        ClientToScreen(wnd, &at);
        return at;
    };
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PickPaint(*p, dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND: return 1;
        case WM_SETCURSOR: SetCursor(LoadCursorW(nullptr, IDC_CROSS)); return TRUE;
        case WM_MOUSEMOVE:
            p->now = pointer();
            PickHover(*p);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONDOWN:
            p->now = p->down = pointer();
            p->dragging = p->mode == "region" || p->mode == "area";
            SetCapture(wnd);
            return 0;
        case WM_LBUTTONUP: {
            ReleaseCapture();
            p->now = pointer();
            PickHover(*p);
            RECT r = PickRect(*p);
            if ((p->mode == "region" || p->mode == "area") && (r.right - r.left < 3 || r.bottom - r.top < 3)) {
                p->dragging = false;  // (a click, not a drag: still picking)
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            PickFinish(true);
            return 0;
        }
        case WM_RBUTTONUP: PickFinish(false); return 0;
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) PickFinish(false);
            if (wp == VK_RETURN && (p->mode == "screen" || p->mode == "window" || p->mode == "color")) PickFinish(true);
            return 0;
        case WM_ACTIVATE:
            if (LOWORD(wp) == WA_INACTIVE && !p->done) PostMessageW(wnd, WM_APP, 0, 0);
            return 0;
        case WM_APP:  // (another window came in front: cancelled, so a picker never stays over everything)
            if (GetForegroundWindow() != wnd && !p->done) PickFinish(false);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

// tourne.screenPick(options, onDone(result)): options.mode "region" (the default), "window", "screen", "color" or
// "area"; options.save: where the picture goes (a PNG; where writeFile may write). onDone gets {x, y, w, h,
// connector, path?, color?} in screen pixels, or nil when cancelled (Esc or a right-click). False when it
// couldn't open (another is open, or it wasn't allowed).
int ApiScreenPick(lua_State* L) {
    Entry& e = E(L);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    if (!Granted(e, L"screen")) return FailBool(L, "it wasn't allowed pictures of the screen when installed");
    if (g_pick) return FailBool(L, "a screen picker is open already");
    std::string mode = "region", save;
    if (lua_istable(L, 1)) {
        lua_getfield(L, 1, "mode");
        if (lua_isstring(L, -1)) mode = lua_tostring(L, -1);
        lua_getfield(L, 1, "save");
        if (lua_isstring(L, -1)) save = lua_tostring(L, -1);
        lua_pop(L, 2);
    }
    if (mode != "region" && mode != "window" && mode != "screen" && mode != "color" && mode != "area")
        return FailBool(L, "screenPick's mode is region, window, screen, color or area, not " + mode);
    std::wstring savePath;
    std::string why;
    if (!save.empty() && !Resolve(e, save, true, &savePath, &why)) return FailBool(L, "screenPick's save: " + why);

    auto* p = new ScreenPick;
    p->serial = e.serial, p->mode = mode, p->save = savePath;
    p->screens = {GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0, 0};
    p->screens.right = p->screens.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    p->screens.bottom = p->screens.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int w = p->screens.right - p->screens.left, h = p->screens.bottom - p->screens.top;
    // The screens now, and a darker copy to draw around what's picked.
    HDC screen = GetDC(nullptr), dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    void* dimBits = nullptr;
    p->shot = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    p->dim = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &dimBits, nullptr, 0);
    bool ok = p->shot && p->dim;
    if (ok) {
        HGDIOBJ old = SelectObject(dc, p->shot);
        ok = BitBlt(dc, 0, 0, w, h, screen, p->screens.left, p->screens.top, SRCCOPY | CAPTUREBLT);
        SelectObject(dc, old);
        GdiFlush();
        p->shotBits = (uint32_t*)bits;
        auto* from = (const uint32_t*)bits;
        auto* to = (uint32_t*)dimBits;
        for (size_t i = 0, n = (size_t)w * h; ok && i < n; i++) to[i] = (from[i] >> 1 & 0x7F7F7F) - (from[i] >> 3 & 0x1F1F1F);  // (about 38%)
    }
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    if (!ok) {
        if (p->shot) DeleteObject(p->shot);
        if (p->dim) DeleteObject(p->dim);
        delete p;
        return FailBool(L, "the screens couldn't be read");
    }
    if (mode == "window") EnumWindows(ListPickWindows, (LPARAM)&p->windows);
    static bool registered = [] {
        WNDCLASSW wc{};
        wc.lpfnWndProc = PickProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kPickClass;
        return RegisterClassW(&wc) != 0;
    }();
    (void)registered;
    p->ref = lua_ref(L, 2);
    g_pick = p;
    GetCursorPos(&p->now);
    PickHover(*p);
    p->wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, kPickClass, L"Tourne screen picker", WS_POPUP, p->screens.left, p->screens.top, w, h,
                             nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!p->wnd) {
        lua_unref(L, p->ref);
        DeleteObject(p->shot), DeleteObject(p->dim);
        delete p;
        g_pick = nullptr;
        return FailBool(L, "its window couldn't open");
    }
    ShowWindow(p->wnd, SW_SHOW);
    SetForegroundWindow(p->wnd);
    if (GetForegroundWindow() != p->wnd) {
        HWND front = GetForegroundWindow();
        DWORD frontThread = front ? GetWindowThreadProcessId(front, nullptr) : 0, me = GetCurrentThreadId();
        bool attached = frontThread && frontThread != me && AttachThreadInput(me, frontThread, TRUE);
        SetForegroundWindow(p->wnd);
        if (attached) AttachThreadInput(me, frontThread, FALSE);
    }
    SetFocus(p->wnd);
    lua_pushboolean(L, true);
    return 1;
}

// A picker an entry opened, closed with it (a reload): its callback is gone with the VM.
void StopPick(const Entry& e) {
    if (!g_pick || g_pick->serial != e.serial) return;
    ScreenPick* p = g_pick;
    p->done = true;
    if (p->wnd) DestroyWindow(p->wnd);
    if (p->shot) DeleteObject(p->shot);
    if (p->dim) DeleteObject(p->dim);
    delete p;
    g_pick = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Other programs' files, off the runtime's thread: JSON lines (Claude Code's transcripts, as jq and grep read them on
// Linux), a search below a folder (find), and the Recycle Bin (rm, for what the user asked to delete)

// A value of a line by its dotted path ("message.usage.input_tokens"), or null.
const json::Value& AtPath(const json::Value& v, const std::string& path) {
    const json::Value* at = &v;
    for (size_t start = 0;;) {
        size_t dot = path.find('.', start);
        at = &(*at)[path.substr(start, dot == std::string::npos ? std::string::npos : dot - start).c_str()];
        if (dot == std::string::npos || at->IsNull()) return *at;
        start = dot + 1;
    }
}

struct JsonLinesFile {
    std::string path, error;
    double size = 0, mtime = 0;
    bool truncated = false;
    std::vector<json::Value> rows;
};

void ReadJsonLines(const std::wstring& path, const std::vector<std::string>& match, const std::vector<std::string>& fields, size_t first,
                   size_t last, size_t tail, size_t* budget, JsonLinesFile* out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                           FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (f == INVALID_HANDLE_VALUE) return void(out->error = "couldn't read it");
    BY_HANDLE_FILE_INFORMATION info{};
    GetFileInformationByHandle(f, &info);
    ULONGLONG size = (ULONGLONG)info.nFileSizeHigh << 32 | info.nFileSizeLow;
    ULARGE_INTEGER t{{info.ftLastWriteTime.dwLowDateTime, info.ftLastWriteTime.dwHighDateTime}};
    out->size = (double)size, out->mtime = (double)((long long)(t.QuadPart / 10000000ULL) - 11644473600LL);
    bool skipFirst = false;
    if (tail && size > tail) {
        LARGE_INTEGER at{};
        at.QuadPart = (LONGLONG)(size - tail);
        SetFilePointerEx(f, at, nullptr, FILE_BEGIN);
        skipFirst = true;  // (a line cut in two)
    }
    std::deque<json::Value> kept;
    auto take = [&](const std::string& line) {
        if (line.empty() || line.size() > (64u << 20)) return true;
        if (!match.empty()) {
            bool any = false;
            for (auto const& m : match) any = any || line.find(m) != std::string::npos;
            if (!any) return true;
        }
        json::Value v;
        if (!json::Parse(line, &v)) return true;
        if (!fields.empty()) {
            json::Value row;
            row.kind = json::Value::Object;
            for (auto const& field : fields)
                if (const json::Value& at = AtPath(v, field); !at.IsNull()) row.fields.emplace_back(field, at);
            v = std::move(row);
        }
        if (!*budget) return out->truncated = true, false;
        --*budget;
        kept.push_back(std::move(v));
        if (last && kept.size() > last) kept.pop_front(), ++*budget;
        return !(first && kept.size() >= first);
    };
    std::string pending;
    std::vector<char> buffer(1 << 20);
    DWORD got = 0;
    bool going = true;
    while (going && ReadFile(f, buffer.data(), (DWORD)buffer.size(), &got, nullptr) && got) {
        size_t start = 0;
        for (size_t i = 0; i < got && going; i++) {
            if (buffer[i] != '\n') continue;
            pending.append(buffer.data() + start, i - start);
            start = i + 1;
            if (!pending.empty() && pending.back() == '\r') pending.pop_back();
            if (skipFirst) skipFirst = false;
            else going = take(pending);
            pending.clear();
        }
        if (going) pending.append(buffer.data() + start, got - start);
    }
    if (going && !pending.empty() && !skipFirst) take(pending);
    CloseHandle(f);
    out->rows.assign(std::make_move_iterator(kept.begin()), std::make_move_iterator(kept.end()));
}

// The strings at a stack index: one, or a list of them.
std::vector<std::string> StringsIn(lua_State* L, int idx) {
    std::vector<std::string> out;
    size_t len = 0;
    if (lua_type(L, idx) == LUA_TSTRING) {
        const char* s = lua_tolstring(L, idx, &len);
        out.emplace_back(s, len);
    } else if (lua_istable(L, idx))
        for (int i = 1, n = lua_objlen(L, idx); i <= n; i++) {
            lua_rawgeti(L, idx, i);
            if (const char* s = lua_type(L, -1) == LUA_TSTRING ? lua_tolstring(L, -1, &len) : nullptr) out.emplace_back(s, len);
            lua_pop(L, 1);
        }
    return out;
}

std::vector<std::string> StringsAt(lua_State* L, int table, const char* key) {
    lua_getfield(L, table, key);
    std::vector<std::string> out = StringsIn(L, -1);
    lua_pop(L, 1);
    return out;
}

double NumberAt(lua_State* L, int table, const char* key, double fallback) {
    lua_getfield(L, table, key);
    double v = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : fallback;
    lua_pop(L, 1);
    return v;
}

// Calls back on the runtime's thread with what push puts on the stack (one value).
void Answer(uint64_t serial, int ref, std::function<void(lua_State*)> push) {
    Post([=] {
        auto found = g_bySerial.find(serial);
        if (found == g_bySerial.end()) return;
        Entry& to = *found->second;
        Guarded(to, [&] {
            lua_settop(to.T, 0);
            lua_getref(to.T, ref);
            lua_unref(to.T, ref);
            push(to.T);
            return Call(to, 1);
        });
    });
}

// tourne.jsonLines(spec, onResult(files)): JSON lines files read on a thread of their own. spec: files (paths it may
// read), match (strings: only a line holding one of them is read, as grep -F), fields (dotted paths: each row is just
// those, keyed by the path; none: the whole line), first / last (a file's first or last N rows), tail (only a file's
// last N bytes), maxRows (all files together; 100000 unless said, at most 500000). files[i]: {path, size, mtime, rows,
// truncated} or {path, error}. Each file is read a line at a time, so any size will do.
int ApiJsonLines(lua_State* L) {
    Entry& e = E(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    std::vector<std::pair<std::string, std::wstring>> files;
    std::vector<std::string> refused;
    for (auto const& given : StringsAt(L, 1, "files")) {
        std::wstring full;
        std::string why;
        files.emplace_back(given, Resolve(e, given, false, &full, &why) ? full : L"");
        refused.push_back(why);
    }
    std::vector<std::string> match = StringsAt(L, 1, "match"), fields = StringsAt(L, 1, "fields");
    size_t first = (size_t)std::max(0.0, NumberAt(L, 1, "first", 0)), last = (size_t)std::max(0.0, NumberAt(L, 1, "last", 0));
    size_t tail = (size_t)std::max(0.0, NumberAt(L, 1, "tail", 0));
    size_t budget = (size_t)std::clamp(NumberAt(L, 1, "maxRows", 100000), 1.0, 500000.0);
    int ref = lua_ref(L, 2);
    uint64_t serial = e.serial;
    std::thread([=]() mutable {
        auto results = std::make_shared<std::vector<JsonLinesFile>>();
        for (size_t i = 0; i < files.size(); i++) {
            JsonLinesFile r;
            r.path = files[i].first;
            if (files[i].second.empty()) r.error = refused[i].empty() ? "not a usable path" : refused[i];
            else ReadJsonLines(files[i].second, match, fields, first, last, tail, &budget, &r);
            results->push_back(std::move(r));
        }
        Answer(serial, ref, [results](lua_State* T) {
            lua_createtable(T, (int)results->size(), 0);
            for (size_t i = 0; i < results->size(); i++) {
                auto const& r = (*results)[i];
                lua_createtable(T, 0, 6);
                lua_pushlstring(T, r.path.data(), r.path.size());
                lua_setfield(T, -2, "path");
                if (!r.error.empty()) {
                    lua_pushstring(T, r.error.c_str());
                    lua_setfield(T, -2, "error");
                } else {
                    SetNumber(T, "size", r.size);
                    SetNumber(T, "mtime", r.mtime);
                    lua_pushboolean(T, r.truncated);
                    lua_setfield(T, -2, "truncated");
                    lua_createtable(T, (int)r.rows.size(), 0);
                    for (size_t k = 0; k < r.rows.size(); k++) PushJson(T, r.rows[k]), lua_rawseti(T, -2, (int)k + 1);
                    lua_setfield(T, -2, "rows");
                }
                lua_rawseti(T, -2, (int)i + 1);
            }
        });
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

struct FoundFile {
    std::string path;
    double size, mtime;
    bool dir;
};

// tourne.findFiles(root, spec, onResult(found)): what's below a folder it may read whose name matches, as
// find does, on a thread of its own. spec: name (a pattern such as "*.jsonl" or "CLAUDE.md", any case; or a list),
// prune (folder names, or patterns, never gone into), maxDepth (1: the root's own entries; 32 unless said), newerThan
// (seconds since 1970: only what changed since), dirs (true: match folders instead of files), max (5000 unless said;
// a search also stops after a minute). found[i]: {path, size, mtime, isDir}. Linked folders (junctions) aren't
// followed; linked files are found.
int ApiFindFiles(lua_State* L) {
    Entry& e = E(L);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    std::wstring root;
    std::string why;
    bool allowed = Resolve(e, Arg(L, 1), false, &root, &why);
    std::vector<std::wstring> names, prune;
    for (auto const& n : StringsAt(L, 2, "name")) names.push_back(W(n));
    for (auto const& n : StringsAt(L, 2, "prune")) prune.push_back(W(n));
    if (names.empty()) names.push_back(L"*");
    int maxDepth = (int)std::clamp(NumberAt(L, 2, "maxDepth", 32), 1.0, 64.0);
    double newer = NumberAt(L, 2, "newerThan", 0);
    size_t max = (size_t)std::clamp(NumberAt(L, 2, "max", 5000), 1.0, 100000.0);
    lua_getfield(L, 2, "dirs");
    bool dirs = lua_toboolean(L, -1);
    lua_pop(L, 1);
    int ref = lua_ref(L, 3);
    uint64_t serial = e.serial;
    if (!allowed) TellOnce(e, "a search was refused: " + why);
    std::thread([=] {
        auto found = std::make_shared<std::vector<FoundFile>>();
        ULONGLONG until = GetTickCount64() + 60000;
        auto matches = [](const std::wstring& name, const std::vector<std::wstring>& patterns) {
            for (auto const& p : patterns)
                if (PathMatchSpecW(name.c_str(), p.c_str())) return true;
            return false;
        };
        std::function<void(const std::wstring&, int)> walk = [&](const std::wstring& dir, int depth) {
            WIN32_FIND_DATAW fd;
            HANDLE h = FindFirstFileExW((dir + L"\\*").c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
            if (h == INVALID_HANDLE_VALUE) return;
            do {
                std::wstring name = fd.cFileName;
                if (name == L"." || name == L"..") continue;
                if (found->size() >= max || GetTickCount64() > until) break;
                bool isDir = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
                std::wstring path = dir + L"\\" + name;
                ULARGE_INTEGER t{{fd.ftLastWriteTime.dwLowDateTime, fd.ftLastWriteTime.dwHighDateTime}};
                double mtime = (double)((long long)(t.QuadPart / 10000000ULL) - 11644473600LL);
                if (isDir == dirs && matches(name, names) && mtime >= newer)
                    found->push_back({U(path), (double)((ULONGLONG)fd.nFileSizeHigh << 32 | fd.nFileSizeLow), mtime, isDir});
                if (isDir && !(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && depth < maxDepth && !matches(name, prune))
                    walk(path, depth + 1);
            } while (FindNextFileW(h, &fd));
            FindClose(h);
        };
        if (allowed) walk(root, 1);
        Answer(serial, ref, [found](lua_State* T) {
            lua_createtable(T, (int)found->size(), 0);
            for (size_t i = 0; i < found->size(); i++) {
                auto const& f = (*found)[i];
                lua_createtable(T, 0, 4);
                lua_pushlstring(T, f.path.data(), f.path.size());
                lua_setfield(T, -2, "path");
                SetNumber(T, "size", f.size);
                SetNumber(T, "mtime", f.mtime);
                lua_pushboolean(T, f.dir);
                lua_setfield(T, -2, "isDir");
                lua_rawseti(T, -2, (int)i + 1);
            }
        });
    }).detach();
    lua_pushboolean(L, allowed);
    return 1;
}

// tourne.trash(path or paths [, onDone(ok)]): files or folders moved to the Recycle Bin, only inside the folders its
// "deletes" need names (never those folders themselves), for what the user asked to delete. Windows deletes outright
// what can't go to the Recycle Bin (a network drive). A link (symlink or junction) is removed itself, as rm does,
// never what it points at.
int ApiTrash(lua_State* L) {
    Entry& e = E(L);
    luaL_checkany(L, 1);
    std::vector<std::string> given = StringsIn(L, 1);
    int ref = lua_isfunction(L, 2) ? lua_ref(L, 2) : LUA_NOREF;
    std::wstring list;
    std::vector<std::pair<std::wstring, bool>> links;  // (path, is a folder)
    std::string why = given.empty() ? "nothing named" : "";
    auto folders = ReadsFolders(e, L"deletes");
    if (!Granted(e, L"deletes")) why = "it wasn't allowed to delete files when installed";
    for (auto const& g : given) {
        std::wstring p = W(g);
        for (auto& c : p)
            if (c == L'/') c = L'\\';
        std::wstring full = Full(p);
        while (full.size() > 3 && full.back() == L'\\') full.pop_back();
        bool inside = false;
        for (auto const& f : folders) inside = inside || (InsideNeed(full, f) && Full(f) != full);
        if (full.empty() || full.find_first_of(L"*?") != std::wstring::npos || !inside) {
            if (why.empty()) why = "it may delete only inside the folders it was installed with, not " + g;
            continue;
        }
        DWORD attrs = GetFileAttributesW(full.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) continue;
        if (attrs & FILE_ATTRIBUTE_REPARSE_POINT) links.emplace_back(full, (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);
        else list += full + L'\0';
    }
    if (!why.empty()) {
        TellOnce(e, "a delete was refused: " + why);
        if (ref != LUA_NOREF) Answer(e.serial, ref, [](lua_State* T) { lua_pushboolean(T, false); });
        return FailBool(L, why);
    }
    Remember(Name(e) + L" moved to the Recycle Bin: " + list.substr(0, 200));
    for (auto const& [path, dir] : links) Remember(Name(e) + L" removed the link " + path);
    uint64_t serial = e.serial;
    std::thread([=] {
        bool ok = true;
        for (auto const& [path, dir] : links) ok = (dir ? RemoveDirectoryW(path.c_str()) : DeleteFileW(path.c_str())) && ok;
        if (!list.empty()) {
            HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            std::wstring from = list + L'\0';
            SHFILEOPSTRUCTW op{};
            op.wFunc = FO_DELETE;
            op.pFrom = from.c_str();
            op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
            ok = SHFileOperationW(&op) == 0 && !op.fAnyOperationsAborted;
            if (SUCCEEDED(com)) CoUninitialize();
        }
        if (ref != LUA_NOREF) Answer(serial, ref, [ok](lua_State* T) { lua_pushboolean(T, ok); });
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Tables

void OpenExtrasApi(lua_State* L) {
    // On the noctalia table (at the top of the stack).
    SetFunctions(L, {{"getenv", ApiGetenv}, {"runInTerminal", ApiRunInTerminal}, {"readFileAsync", ApiReadFileAsync},
                     {"download", ApiDownload}, {"openSettings", ApiOpenSettings}, {"setWallpaperMask", ApiSetWallpaperMask}});
}

void OpenTourneExtras(lua_State* L) {
    lua_getglobal(L, "tourne");
    SetFunctions(L, {{"processInfo", ApiProcessInfo}, {"focusProcess", ApiFocusProcess}, {"screenPick", ApiScreenPick}, {"launch", ApiLaunch},
                     {"jsonLines", ApiJsonLines}, {"findFiles", ApiFindFiles}, {"trash", ApiTrash}});
    lua_pop(L, 1);
}
