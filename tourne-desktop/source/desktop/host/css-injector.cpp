#include <winsock2.h>

#include "css-injector.h"

#include <oledb.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <tlhelp32.h>
#include <winhttp.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "css-gen.h"
#include "json-lite.h"
#include "td.h"

namespace td {

namespace {

constexpr UINT WM_TD_CSS_CHANGED = WM_APP + 30;  // a style sheet was saved
constexpr UINT WM_TD_INJECT_ENDED = WM_APP + 31; // an app's connection closed (wp: its pid)
constexpr UINT WM_TD_INJECT_ASK = WM_APP + 32;   // ask about apps turned on (after the config is applied)
constexpr UINT WM_TD_FOREGROUND = WM_APP + 33;   // a window came to the front (wp: its pid)
constexpr UINT WM_TD_INJECT_FOUND = WM_APP + 34; // a search for an app's program finished
constexpr UINT_PTR kTimerCss = 30;

// Apps Tourne knows where to find. Squirrel apps keep each version in an
// "app-x.y.z" folder: the newest is used, started directly (its launcher
// would drop the DevTools switch).
struct KnownApp {
    const wchar_t* name;
    const wchar_t* title;
    const wchar_t* exe;
    const wchar_t* folder;
    bool squirrel;
    const char* style;  // its starter style in the tourne folder
};
constexpr KnownApp kKnown[] = {
    {L"discord", L"Discord", L"Discord.exe", L"%LOCALAPPDATA%\\Discord", true, "discord"},
    {L"discordptb", L"Discord PTB", L"DiscordPTB.exe", L"%LOCALAPPDATA%\\DiscordPTB", true, "discord"},
    {L"discordcanary", L"Discord Canary", L"DiscordCanary.exe", L"%LOCALAPPDATA%\\DiscordCanary", true, "discord"},
    {L"claude", L"Claude", L"claude.exe", L"%LOCALAPPDATA%\\AnthropicClaude", true, "claude"},
    {L"spotify", L"Spotify", L"Spotify.exe", L"%APPDATA%\\Spotify", false, "spotify"},
    {L"slack", L"Slack", L"slack.exe", L"%LOCALAPPDATA%\\slack", true, "generic"},
    {L"obsidian", L"Obsidian", L"Obsidian.exe", L"%LOCALAPPDATA%\\Programs\\Obsidian", false, "generic"},
    {L"vscode", L"Visual Studio Code", L"Code.exe", L"%LOCALAPPDATA%\\Programs\\Microsoft VS Code", false, "generic"},
};

// The script given to each page: puts the style sheet in (last in <head>, so
// it wins over the page's own, and kept last when the page adds more).
constexpr const char kApplyScript[] = R"js((function (css) {
  var id = 'tourne-css';
  var apply = function () {
    var root = document.documentElement;
    if (!root) return;
    var s = document.getElementById(id);
    if (!s) { s = document.createElement('style'); s.id = id; }
    if (s.textContent !== css) s.textContent = css;
    var parent = document.head || root;
    if (s.parentNode !== parent || s.nextElementSibling) parent.appendChild(s);
    if (document.head && !window.__tourneCssWatch) {
      window.__tourneCssWatch = new MutationObserver(function () {
        var t = document.getElementById(id);
        if (t && t.nextElementSibling) document.head.appendChild(t);
      });
      window.__tourneCssWatch.observe(document.head, { childList: true });
    }
  };
  apply();
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', apply, { once: true });
})()js";
constexpr const char kRemoveScript[] =
    "(function(){var s=document.getElementById('tourne-css');if(s)s.remove();"
    "if(window.__tourneCssWatch){window.__tourneCssWatch.disconnect();window.__tourneCssWatch=null;}})()";

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"inject", format, args);
    va_end(args);
}

std::wstring Expand(const wchar_t* text) {
    wchar_t out[MAX_PATH * 2];
    DWORD n = ExpandEnvironmentStringsW(text, out, ARRAYSIZE(out));
    return n && n <= ARRAYSIZE(out) ? out : text;
}

std::string Utf8(const std::wstring& w) { return WideToUtf8(w); }

std::wstring CssFolder() {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path))) out = std::wstring(path) + L"\\Tourne\\Desktop\\css";
    CoTaskMemFree(path);
    return out;
}

bool ReadUtf8(const std::wstring& path, std::string* out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    GetFileSizeEx(f, &size);
    out->assign((size_t)std::min<LONGLONG>(size.QuadPart, 8 << 20), '\0');
    DWORD got = 0;
    BOOL ok = ReadFile(f, out->data(), (DWORD)out->size(), &got, nullptr);
    CloseHandle(f);
    out->resize(got);
    if (out->size() >= 3 && (unsigned char)(*out)[0] == 0xEF) out->erase(0, 3);  // BOM
    return ok;
}

void WriteIfChanged(const std::wstring& path, const std::string& text) {
    std::string old;
    if (ReadUtf8(path, &old) && old == text) return;
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD wrote;
    WriteFile(f, text.data(), (DWORD)text.size(), &wrote, nullptr);
    CloseHandle(f);
}

std::wstring Setting(const std::wstring& key) {
    const wchar_t* v = GetString(L"inject", key);
    std::wstring out = v ? v : L"";
    FreeString(v);
    out.erase(0, out.find_first_not_of(L" \t"));
    out.erase(out.find_last_not_of(L" \t") + 1);
    return out;
}

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

// "1.2.10" after "1.2.9".
bool NewerVersion(const std::wstring& a, const std::wstring& b) {
    size_t i = 0, j = 0;
    while (i < a.size() || j < b.size()) {
        unsigned long x = wcstoul(a.c_str() + i, nullptr, 10), y = wcstoul(b.c_str() + j, nullptr, 10);
        if (x != y) return x > y;
        i = a.find(L'.', i);
        j = b.find(L'.', j);
        if (i == std::wstring::npos || j == std::wstring::npos) return i != std::wstring::npos;
        i++, j++;
    }
    return false;
}

bool IsFile(const std::wstring& path) {
    DWORD a = path.empty() ? INVALID_FILE_ATTRIBUTES : GetFileAttributesW(path.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring FileNameOf(const std::wstring& path) { return path.substr(path.find_last_of(L"\\/") + 1); }
std::wstring FolderOf(const std::wstring& path) {
    size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"" : path.substr(0, slash);
}

// The program in a folder: directly, or (a Squirrel app) in its newest "app-x.y.z" folder.
std::wstring ExeIn(const std::wstring& folder, const std::wstring& exe, bool squirrel) {
    if (folder.empty()) return L"";
    if (!squirrel) return IsFile(folder + L"\\" + exe) ? folder + L"\\" + exe : L"";
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"\\app-*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return IsFile(folder + L"\\" + exe) ? folder + L"\\" + exe : L"";
    std::wstring best, bestVersion;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        std::wstring version = fd.cFileName + 4;
        std::wstring path = folder + L"\\" + fd.cFileName + L"\\" + exe;
        if (!IsFile(path)) continue;
        if (best.empty() || NewerVersion(version, bestVersion)) best = path, bestVersion = version;
    } while (FindNextFileW(find, &fd));
    FindClose(find);
    return best;
}

std::wstring FindExe(const KnownApp& app) {
    std::wstring folder = Expand(app.folder);
    if (!app.squirrel) {
        std::wstring exe = folder + L"\\" + app.exe;
        return GetFileAttributesW(exe.c_str()) != INVALID_FILE_ATTRIBUTES ? exe : L"";
    }
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"\\app-*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return L"";
    std::wstring best, bestVersion;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        std::wstring version = fd.cFileName + 4;
        std::wstring exe = folder + L"\\" + fd.cFileName + L"\\" + app.exe;
        if (GetFileAttributesW(exe.c_str()) == INVALID_FILE_ATTRIBUTES) continue;
        if (best.empty() || NewerVersion(version, bestVersion)) best = exe, bestVersion = version;
    } while (FindNextFileW(find, &fd));
    FindClose(find);
    return best;
}

std::wstring ProcessPath(DWORD pid) {
    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!p) return L"";
    wchar_t path[MAX_PATH * 2];
    DWORD n = ARRAYSIZE(path);
    std::wstring out = QueryFullProcessImageNameW(p, 0, path, &n) ? path : L"";
    CloseHandle(p);
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Finding an app's program wherever it was installed, when it isn't in its usual folder (or only its
// file name was given). In order: the app running now, Windows' App Paths, the installed programs list,
// Start menu shortcuts, then the Windows search index. Off the host's thread (the index can be slow);
// what's found is kept for this run and the apps are brought up to date once it's in.

struct Wanted {
    std::wstring name;  // the app's name in the settings
    std::wstring exe;   // its program's file name
    bool squirrel;
};

// A path to the program, from a path that may be the program, a Squirrel app's Update.exe, or its folder.
std::wstring ExeFrom(const std::wstring& path, const Wanted& w) {
    if (path.empty()) return L"";
    if (_wcsicmp(FileNameOf(path).c_str(), w.exe.c_str()) == 0 && IsFile(path)) {
        // A Squirrel app's versioned copy: its newest version instead.
        std::wstring folder = FolderOf(path);
        if (w.squirrel && _wcsnicmp(FileNameOf(folder).c_str(), L"app-", 4) == 0) {
            std::wstring newest = ExeIn(FolderOf(folder), w.exe, true);
            if (!newest.empty()) return newest;
        }
        return path;
    }
    if (_wcsicmp(FileNameOf(path).c_str(), L"update.exe") == 0) return ExeIn(FolderOf(path), w.exe, true);
    DWORD a = GetFileAttributesW(path.c_str());
    if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY)) {
        std::wstring found = ExeIn(path, w.exe, w.squirrel);
        return found.empty() && w.squirrel ? ExeIn(path, w.exe, false) : found;
    }
    return L"";
}

std::wstring FromRunning(const Wanted& w) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return L"";
    PROCESSENTRY32W pe{sizeof(pe)};
    std::wstring found;
    for (BOOL more = Process32FirstW(snap, &pe); more && found.empty(); more = Process32NextW(snap, &pe))
        if (_wcsicmp(pe.szExeFile, w.exe.c_str()) == 0) found = ExeFrom(ProcessPath(pe.th32ProcessID), w);
    CloseHandle(snap);
    return found;
}

std::wstring RegString(HKEY root, const std::wstring& key, const wchar_t* value, REGSAM view = 0) {
    HKEY k;
    if (RegOpenKeyExW(root, key.c_str(), 0, KEY_READ | view, &k) != ERROR_SUCCESS) return L"";
    wchar_t buffer[MAX_PATH * 2];
    DWORD size = sizeof(buffer), type = 0;
    std::wstring out;
    if (RegQueryValueExW(k, value, nullptr, &type, (BYTE*)buffer, &size) == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ)) {
        buffer[std::min<size_t>(size / sizeof(wchar_t), ARRAYSIZE(buffer) - 1)] = 0;
        out = type == REG_EXPAND_SZ ? Expand(buffer) : buffer;
    }
    RegCloseKey(k);
    // A program path may come quoted, or with an icon index after it.
    if (!out.empty() && out[0] == L'"') out = out.substr(1, out.find(L'"', 1) - 1);
    if (size_t comma = out.rfind(L','); comma != std::wstring::npos && comma > out.rfind(L'\\')) out.resize(comma);
    return out;
}

std::wstring FromAppPaths(const Wanted& w) {
    for (HKEY root : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE}) {
        std::wstring path = RegString(root, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" + w.exe, nullptr);
        if (std::wstring found = ExeFrom(path, w); !found.empty()) return found;
    }
    return L"";
}

// The installed programs list: an entry whose icon is the program, or whose folder holds it.
std::wstring FromInstalled(const Wanted& w) {
    const wchar_t* kUninstall = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    struct Place {
        HKEY root;
        REGSAM view;
    };
    for (Place place : {Place{HKEY_CURRENT_USER, 0}, Place{HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY}, Place{HKEY_LOCAL_MACHINE, KEY_WOW64_32KEY}}) {
        HKEY key;
        if (RegOpenKeyExW(place.root, kUninstall, 0, KEY_READ | place.view, &key) != ERROR_SUCCESS) continue;
        wchar_t sub[256];
        std::wstring found;
        for (DWORD i = 0; found.empty(); i++) {
            DWORD length = ARRAYSIZE(sub);
            LONG result = RegEnumKeyExW(key, i, sub, &length, nullptr, nullptr, nullptr, nullptr);
            if (result == ERROR_NO_MORE_ITEMS) break;
            if (result != ERROR_SUCCESS) continue;
            std::wstring entry = std::wstring(kUninstall) + L"\\" + sub;
            std::wstring icon = RegString(place.root, entry, L"DisplayIcon", place.view);
            if (_wcsicmp(FileNameOf(icon).c_str(), w.exe.c_str()) == 0) found = ExeFrom(icon, w);
            if (found.empty()) {
                std::wstring folder = RegString(place.root, entry, L"InstallLocation", place.view);
                while (!folder.empty() && (folder.back() == L'\\' || folder.back() == L'/')) folder.pop_back();
                if (!folder.empty()) found = ExeFrom(folder, w);
            }
        }
        RegCloseKey(key);
        if (!found.empty()) return found;
    }
    return L"";
}

// Start menu shortcuts (yours and everyone's): one that starts the program, or a Squirrel app's Update.exe
// asked to start it.
std::wstring FromShortcuts(const Wanted& w) {
    IShellLinkW* link = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link)))) return L"";
    IPersistFile* file = nullptr;
    link->QueryInterface(IID_PPV_ARGS(&file));
    std::wstring found;
    std::function<void(const std::wstring&, int)> walk = [&](const std::wstring& folder, int depth) {
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((folder + L"\\*").c_str(), &fd);
        if (find == INVALID_HANDLE_VALUE) return;
        do {
            std::wstring name = fd.cFileName, path = folder + L"\\" + name;
            if (name == L"." || name == L"..") continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (depth < 4) walk(path, depth + 1);
                continue;
            }
            if (name.size() < 4 || _wcsicmp(name.c_str() + name.size() - 4, L".lnk") != 0 || !file) continue;
            if (FAILED(file->Load(path.c_str(), STGM_READ))) continue;
            wchar_t target[MAX_PATH * 2] = {}, args[1024] = {};
            if (FAILED(link->GetPath(target, ARRAYSIZE(target), nullptr, SLGP_RAWPATH))) continue;
            std::wstring t = Expand(target);
            if (_wcsicmp(FileNameOf(t).c_str(), L"update.exe") == 0) {
                link->GetArguments(args, ARRAYSIZE(args));
                if (!wcsstr(Lower(args).c_str(), Lower(w.exe).c_str())) continue;
            } else if (_wcsicmp(FileNameOf(t).c_str(), w.exe.c_str()) != 0) {
                continue;
            }
            found = ExeFrom(t, w);
        } while (found.empty() && FindNextFileW(find, &fd));
        FindClose(find);
    };
    for (auto folderId : {FOLDERID_Programs, FOLDERID_CommonPrograms}) {
        PWSTR folder = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, nullptr, &folder))) walk(folder, 0);
        CoTaskMemFree(folder);
        if (!found.empty()) break;
    }
    if (file) file->Release();
    link->Release();
    return found;
}

// The Windows search index, by file name (only folders it indexes, so last). (OLE DB's ids by name: MinGW has
// no __uuidof for them.) One query at a time: at startup the searches for every app not found yet reached
// here together, and Tourne's PC crashed there once (2026-10-06, a call to 0x0 while three ran at once).
std::mutex g_searchIndexLock;
std::wstring FromSearchIndex(const Wanted& w) {
    std::lock_guard lock(g_searchIndexLock);
    constexpr GUID kDefaultDialect = {0xc8b521fb, 0x5cf3, 0x11ce, {0xad, 0xe5, 0x00, 0xaa, 0x00, 0x44, 0x77, 0x3d}};
    CLSID collator;
    if (FAILED(CLSIDFromProgID(L"Search.CollatorDSO", &collator))) return L"";
    IDBInitialize* init = nullptr;
    if (FAILED(CoCreateInstance(collator, nullptr, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (void**)&init))) return L"";
    std::wstring found;
    IDBCreateSession* sessions = nullptr;
    IDBCreateCommand* commands = nullptr;
    ICommandText* command = nullptr;
    IRowset* rows = nullptr;
    IAccessor* accessor = nullptr;
    std::wstring name;
    for (wchar_t c : w.exe) name += c == L'\'' ? std::wstring(L"''") : std::wstring(1, c);
    std::wstring sql = L"SELECT System.ItemPathDisplay FROM SystemIndex WHERE System.FileName = '" + name + L"'";
    if (SUCCEEDED(init->Initialize()) && SUCCEEDED(init->QueryInterface(IID_IDBCreateSession, (void**)&sessions)) &&
        SUCCEEDED(sessions->CreateSession(nullptr, IID_IDBCreateCommand, (IUnknown**)&commands)) &&
        SUCCEEDED(commands->CreateCommand(nullptr, IID_ICommandText, (IUnknown**)&command)) &&
        SUCCEEDED(command->SetCommandText(kDefaultDialect, sql.c_str())) &&
        SUCCEEDED(command->Execute(nullptr, IID_IRowset, nullptr, nullptr, (IUnknown**)&rows)) && rows &&
        SUCCEEDED(rows->QueryInterface(IID_IAccessor, (void**)&accessor))) {
        struct Row {
            DBSTATUS status;
            DBLENGTH length;
            wchar_t path[MAX_PATH * 2];
        };
        DBBINDING binding{};
        binding.iOrdinal = 1;
        binding.obValue = offsetof(Row, path);
        binding.obLength = offsetof(Row, length);
        binding.obStatus = offsetof(Row, status);
        binding.dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
        binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
        binding.cbMaxLen = sizeof(Row::path);
        binding.wType = DBTYPE_WSTR;
        HACCESSOR handle = 0;
        if (SUCCEEDED(accessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, sizeof(Row), &handle, nullptr))) {
            for (int batch = 0; batch < 10 && found.empty(); batch++) {
                HROW got[16];
                HROW* list = got;
                DBCOUNTITEM count = 0;
                if (FAILED(rows->GetNextRows(DB_NULL_HCHAPTER, 0, ARRAYSIZE(got), &count, &list)) || !count) break;
                for (DBCOUNTITEM i = 0; i < count && found.empty(); i++) {
                    Row row{};
                    if (SUCCEEDED(rows->GetData(got[i], handle, &row)) && row.status == DBSTATUS_S_OK) found = ExeFrom(row.path, w);
                }
                rows->ReleaseRows(count, got, nullptr, nullptr, nullptr);
            }
            accessor->ReleaseAccessor(handle, nullptr);
        }
    }
    for (IUnknown* u : std::initializer_list<IUnknown*>{accessor, rows, command, commands, sessions}) if (u) u->Release();
    init->Uninitialize();
    init->Release();
    return found;
}

HWND g_wnd;
std::mutex g_foundLock;
std::map<std::wstring, std::wstring> g_found;  // app name -> its program ("" when none was found)
std::set<std::wstring> g_searching;

DWORD WINAPI SearchThread(LPVOID param) {
    std::unique_ptr<Wanted> w((Wanted*)param);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::wstring found;
    const wchar_t* where = L"";
    struct Step {
        const wchar_t* where;
        std::wstring (*find)(const Wanted&);
    };
    for (Step step : {Step{L"it running", FromRunning}, Step{L"App Paths", FromAppPaths}, Step{L"the installed programs", FromInstalled},
                      Step{L"a Start menu shortcut", FromShortcuts}, Step{L"the search index", FromSearchIndex}}) {
        found = step.find(*w);
        if (!found.empty()) {
            where = step.where;
            break;
        }
    }
    CoUninitialize();
    if (found.empty()) Log(L"%s: %s wasn't found (set its program in the Studio)", w->name.c_str(), w->exe.c_str());
    else Log(L"%s: found %s, from %s", w->name.c_str(), found.c_str(), where);
    {
        std::lock_guard lock(g_foundLock);
        g_found[w->name] = found;
        g_searching.erase(w->name);
    }
    if (g_wnd && !found.empty()) PostMessageW(g_wnd, WM_TD_INJECT_FOUND, 0, 0);
    return 0;
}

// What was found for an app so far ("" while it's looked for, or if nothing was), starting a search the
// first time.
std::wstring Located(const std::wstring& name, const std::wstring& exe, bool squirrel) {
    std::lock_guard lock(g_foundLock);
    if (auto it = g_found.find(name); it != g_found.end()) {
        if (it->second.empty() || IsFile(it->second)) return it->second;
        g_found.erase(it);  // (moved or uninstalled since: looked for again)
    }
    if (g_searching.count(name)) return L"";
    g_searching.insert(name);
    if (HANDLE t = CreateThread(nullptr, 0, SearchThread, new Wanted{name, exe, squirrel}, 0, nullptr)) CloseHandle(t);
    else g_searching.erase(name);
    return L"";
}

////////////////////////////////////////////////////////////////////////////////
// One app's DevTools connection: its own thread, reading what the app sends;
// the host thread sends style updates through it.

class Connection {
public:
    Connection(DWORD pid, int port) : pid_(pid), port_(port) {}
    ~Connection() { Close(); }

    void Start(std::string css) {
        {
            std::lock_guard<std::mutex> lock(lock_);
            css_ = std::move(css);
        }
        thread_ = CreateThread(nullptr, 0, ThreadProc, this, 0, nullptr);
    }

    // New style sheets: every page now, and every page opened from now on.
    void SetCss(std::string css) {
        std::lock_guard<std::mutex> lock(lock_);
        css_ = std::move(css);
        for (auto& [session, page] : pages_) StyleLocked(session, page);
    }

    // Styles taken out of every page (the app keeps running).
    void Unstyle() {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto& [session, page] : pages_) {
            if (!page.script.empty()) Call("Page.removeScriptToEvaluateOnNewDocument", "{\"identifier\":" + json::Quote(page.script) + "}", session);
            page.script.clear();
            Call("Runtime.evaluate", "{\"expression\":" + json::Quote(kRemoveScript) + "}", session);
        }
        css_.clear();
    }

    void Close() {
        stopping_ = true;
        // (Shutdown, not Close: Close would wait for the reply the reading thread is waiting for.)
        if (socket_) WinHttpWebSocketShutdown(socket_, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
        if (thread_) {
            WaitForSingleObject(thread_, 3000);
            CloseHandle(thread_);
            thread_ = nullptr;
        }
        Cleanup();
    }

    DWORD pid() const { return pid_; }
    int port() const { return port_; }

private:
    struct Page {
        std::string script;  // its new-document script's identifier
    };

    static DWORD WINAPI ThreadProc(LPVOID self) {
        ((Connection*)self)->Run();
        return 0;
    }

    void Cleanup() {
        if (socket_) WinHttpCloseHandle(socket_), socket_ = nullptr;
        if (connect_) WinHttpCloseHandle(connect_), connect_ = nullptr;
        if (session_) WinHttpCloseHandle(session_), session_ = nullptr;
    }

    // GET http://127.0.0.1:port/path
    bool Get(const wchar_t* path, std::string* out) {
        HINTERNET req = WinHttpOpenRequest(connect_, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if (!req) return false;
        bool ok = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) && WinHttpReceiveResponse(req, nullptr);
        out->clear();
        for (DWORD avail = 0; ok && WinHttpQueryDataAvailable(req, &avail) && avail;) {
            std::string chunk(avail, '\0');
            DWORD got = 0;
            if (!WinHttpReadData(req, chunk.data(), avail, &got)) break;
            out->append(chunk, 0, got);
        }
        WinHttpCloseHandle(req);
        return ok && !out->empty();
    }

    void Run() {
        session_ = WinHttpOpen(L"Tourne Desktop", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (session_) connect_ = WinHttpConnect(session_, L"127.0.0.1", (INTERNET_PORT)port_, 0);
        // The app takes a moment to start (an update check first, for some).
        std::string version;
        for (int i = 0; connect_ && !stopping_ && i < 120 && !Get(L"/json/version", &version); i++) Sleep(500);
        json::Value v;
        std::string url = json::Parse(version, &v) ? v["webSocketDebuggerUrl"].text : "";
        size_t slash = url.find('/', url.find("//") + 2);
        if (url.empty() || slash == std::string::npos) {
            if (!stopping_) Log(L"no DevTools on port %d (the app may not allow it)", port_);
            PostEnded();
            return;
        }
        std::wstring path = Utf8ToWide(url.substr(slash));
        HINTERNET req = WinHttpOpenRequest(connect_, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if (req && WinHttpSetOption(req, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0) &&
            WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) && WinHttpReceiveResponse(req, nullptr))
            socket_ = WinHttpWebSocketCompleteUpgrade(req, 0);
        if (req) WinHttpCloseHandle(req);
        if (!socket_) {
            Log(L"couldn't connect to the app's DevTools");
            PostEnded();
            return;
        }
        Log(L"connected to pid %lu on port %d", pid_, port_);
        {
            std::lock_guard<std::mutex> lock(lock_);
            Call("Target.setDiscoverTargets", "{\"discover\":true}", "");  // every page, now and later
        }
        std::string message;
        std::vector<char> buffer(64 * 1024);
        while (!stopping_) {
            DWORD got = 0;
            WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
            if (WinHttpWebSocketReceive(socket_, buffer.data(), (DWORD)buffer.size(), &got, &type) != NO_ERROR) break;
            if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) break;
            message.append(buffer.data(), got);
            if (type == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE || type == WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE) continue;
            Handle(message);
            message.clear();
        }
        Log(L"pid %lu's DevTools closed", pid_);
        PostEnded();
    }

    void PostEnded() {
        if (!stopping_ && g_host) PostMessageW(g_host, WM_TD_INJECT_ENDED, pid_, 0);
    }

    void Handle(const std::string& text) {
        json::Value m;
        if (!json::Parse(text, &m)) return;
        std::lock_guard<std::mutex> lock(lock_);
        std::string const& method = m["method"].text;
        if (method == "Target.targetCreated") {
            json::Value const& info = m["params"]["targetInfo"];
            std::string const& type = info["type"].text;
            if ((type == "page" || type == "webview") && attached_.insert(info["targetId"].text).second) {
                int id = Call("Target.attachToTarget", "{\"targetId\":" + json::Quote(info["targetId"].text) + ",\"flatten\":true}", "");
                attaching_[id] = true;
            }
        } else if (method == "Target.detachedFromTarget") {
            pages_.erase(m["params"]["sessionId"].text);
        } else if (method == "Target.targetDestroyed") {
            attached_.erase(m["params"]["targetId"].text);
        } else if (!m["id"].IsNull()) {
            int id = (int)m["id"].number;
            if (attaching_.erase(id)) {
                std::string session = m["result"]["sessionId"].text;
                if (!session.empty()) {
                    Page& page = pages_[session];
                    Call("Page.enable", "{}", session);  // (new-document scripts need it)
                    StyleLocked(session, page);
                }
            } else if (auto it = scripts_.find(id); it != scripts_.end()) {
                auto page = pages_.find(it->second);
                if (page != pages_.end()) page->second.script = m["result"]["identifier"].text;
                scripts_.erase(it);
            }
        }
    }

    void StyleLocked(const std::string& session, Page& page) {
        if (!page.script.empty()) Call("Page.removeScriptToEvaluateOnNewDocument", "{\"identifier\":" + json::Quote(page.script) + "}", session);
        page.script.clear();
        if (css_.empty()) {
            Call("Runtime.evaluate", "{\"expression\":" + json::Quote(kRemoveScript) + "}", session);
            return;
        }
        std::string script = std::string(kApplyScript) + json::Quote(css_) + ");";  // (the script ends "})(")
        scripts_[Call("Page.addScriptToEvaluateOnNewDocument", "{\"source\":" + json::Quote(script) + "}", session)] = session;
        Call("Runtime.evaluate", "{\"expression\":" + json::Quote(script) + "}", session);
    }

    // Sends a command (lock held); its id.
    int Call(const char* method, const std::string& params, const std::string& session) {
        int id = ++next_;
        std::string m = "{\"id\":" + std::to_string(id) + ",\"method\":\"" + method + "\",\"params\":" + params;
        if (!session.empty()) m += ",\"sessionId\":" + json::Quote(session);
        m += "}";
        if (socket_) WinHttpWebSocketSend(socket_, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, m.data(), (DWORD)m.size());
        return id;
    }

public:
    static HWND g_host;

private:
    DWORD pid_;
    int port_;
    HANDLE thread_ = nullptr;
    HINTERNET session_ = nullptr, connect_ = nullptr, socket_ = nullptr;
    std::atomic<bool> stopping_{false};
    std::mutex lock_;
    std::string css_;
    int next_ = 0;
    std::map<std::string, Page> pages_;  // by session
    std::set<std::string> attached_;     // target ids
    std::map<int, bool> attaching_;      // attachToTarget calls waiting for their session
    std::map<int, std::string> scripts_; // addScript calls waiting for their identifier
};
HWND Connection::g_host;

////////////////////////////////////////////////////////////////////////////////
// The apps

struct App {
    std::wstring name, title, exe, restart, style;
    std::vector<std::wstring> css;
    bool on = false;
    std::unique_ptr<Connection> connection;
};

std::vector<std::unique_ptr<App>> g_apps;
std::set<std::wstring> g_wasOn;   // apps on before this config (an app turned on is asked about)
std::set<DWORD> g_askedPids;      // apps opened without styles, already asked about
std::set<std::wstring> g_toAsk;   // apps turned on, to be asked about
HWINEVENTHOOK g_foregroundHook;
HANDLE g_cssThread, g_cssStop;
bool g_asking;
bool g_firstApply = true;

App* FindApp(const std::wstring& name) {
    for (auto& a : g_apps)
        if (a->name == name) return a.get();
    return nullptr;
}

bool Consented(const App& app) { return GetIntValue(L"inject", (L"Consent_" + app.name).c_str(), 0) != 0; }

// The app's style sheets, one after another.
std::string CssOf(const App& app) {
    std::string all;
    std::wstring folder = CssFolder();
    for (auto const& file : app.css) {
        std::wstring path = file.size() > 1 && (file[1] == L':' || file[0] == L'\\') ? file : folder + L"\\" + file;
        for (auto& c : path)
            if (c == L'/') c = L'\\';
        std::string text;
        if (ReadUtf8(path, &text)) all += "/* " + Utf8(file) + " */\n" + text + "\n";
        else Log(L"%s: no style sheet at \"%s\"", app.name.c_str(), path.c_str());
    }
    return all;
}

// Every process running the app's program, its main one (the oldest) first.
std::vector<DWORD> RunningPids(const App& app) {
    std::vector<std::pair<ULONGLONG, DWORD>> found;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return {};
    std::wstring want = Lower(app.exe), file = want.substr(want.find_last_of(L'\\') + 1);
    PROCESSENTRY32W pe{sizeof(pe)};
    for (BOOL more = Process32FirstW(snap, &pe); more; more = Process32NextW(snap, &pe)) {
        if (Lower(pe.szExeFile) != file) continue;
        std::wstring path = Lower(ProcessPath(pe.th32ProcessID));
        // Same program, or another version of a Squirrel app.
        std::wstring parent = want.substr(0, want.find_last_of(L'\\'));
        std::wstring root = parent.substr(0, parent.find_last_of(L'\\'));
        if (path != want && path.rfind(root + L"\\app-", 0) != 0) continue;
        FILETIME created, a, b, c;
        ULONGLONG when = ~0ull;
        if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID)) {
            if (GetProcessTimes(p, &created, &a, &b, &c)) when = (ULONGLONG)created.dwHighDateTime << 32 | created.dwLowDateTime;
            CloseHandle(p);
        }
        found.push_back({when, pe.th32ProcessID});
    }
    CloseHandle(snap);
    std::sort(found.begin(), found.end());
    std::vector<DWORD> pids;
    for (auto& f : found) pids.push_back(f.second);
    return pids;
}

int FreePort() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) return 0;
    int port = 0;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int len = sizeof(addr);
    if (s != INVALID_SOCKET && !bind(s, (sockaddr*)&addr, sizeof(addr)) && !getsockname(s, (sockaddr*)&addr, &len)) port = ntohs(addr.sin_port);
    if (s != INVALID_SOCKET) closesocket(s);
    WSACleanup();
    return port;
}

void Connect(App& app, DWORD pid, int port) {
    app.connection = std::make_unique<Connection>(pid, port);
    app.connection->Start(CssOf(app));
    SetIntValue(L"inject", (L"Pid_" + app.name).c_str(), (int)pid);
    SetIntValue(L"inject", (L"Port_" + app.name).c_str(), port);
}

// Closes the app (every process of it) and starts it again, styled.
void OpenStyled(App& app) {
    if (app.exe.empty() || GetFileAttributesW(app.exe.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(nullptr, (L"Tourne couldn't find " + app.title + L". Set inject." + app.name + L".exe in tourne.conf to its program.").c_str(),
                    L"Tourne Desktop", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
        return;
    }
    if (app.connection) {
        app.connection->Close();
        app.connection.reset();
    }
    for (DWORD pid : RunningPids(app)) {
        if (HANDLE p = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid)) {
            TerminateProcess(p, 0);
            WaitForSingleObject(p, 3000);
            CloseHandle(p);
        }
    }
    int port = FreePort();
    if (!port) return;
    std::wstring cmd = L"\"" + app.exe + L"\" --remote-debugging-port=" + std::to_wstring(port);
    std::wstring dir = app.exe.substr(0, app.exe.find_last_of(L'\\'));
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(app.exe.c_str(), cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &si, &pi)) {
        Log(L"%s: couldn't start \"%s\" (%lu)", app.name.c_str(), app.exe.c_str(), GetLastError());
        return;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    g_askedPids.insert(pi.dwProcessId);
    Log(L"%s started styled (pid %lu, port %d)", app.name.c_str(), pi.dwProcessId, port);
    Connect(app, pi.dwProcessId, port);
}

constexpr const wchar_t kWarning[] =
    L"Tourne will add your style sheets to %s's pages. %s doesn't support this:\n"
    L"\x2022 an update of %s can undo the styles or make it look broken (turn it off again in tourne.conf to go back);\n"
    L"\x2022 to do it, Tourne starts %s with a debugging connection on this PC. While %s runs, other programs on this PC "
    L"could use that connection to read or control it.\n\n";

std::wstring Fill(const wchar_t* format, const std::wstring& title) {
    std::wstring out;
    for (const wchar_t* p = format; *p; p++) {
        if (p[0] == L'%' && p[1] == L's') out += title, p++;
        else out += *p;
    }
    return out;
}

// An app just turned on: the warning, once.
void AskToStyle(App& app) {
    bool running = !RunningPids(app).empty();
    std::wstring text = Fill(kWarning, app.title) +
                        (running ? L"Yes: restart " + app.title + L" with the styles now (it's closed first).\nNo: style it the next time it opens.\n"
                                 : L"Yes: open " + app.title + L" with the styles now.\nNo: style it the next time it opens.\n") +
                        L"Cancel: don't style it.";
    int answer = MessageBoxW(nullptr, text.c_str(), (L"Style " + app.title + L"?").c_str(),
                             MB_YESNOCANCEL | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
    if (answer == IDCANCEL) {
        Log(L"%s: not styled (declined)", app.name.c_str());
        return;
    }
    SetIntValue(L"inject", (L"Consent_" + app.name).c_str(), 1);
    if (answer == IDYES) OpenStyled(app);
}

void WatchForeground(bool on);

void Rebuild() {
    std::map<std::wstring, std::unique_ptr<App>> old;
    for (auto& a : g_apps) old[a->name] = std::move(a);
    g_apps.clear();
    std::set<std::wstring> names;
    for (auto const& k : kKnown) names.insert(k.name);
    for (auto const& [key, value] : SettingsWithPrefix(L"inject.")) {
        std::wstring rest = key.substr(7);
        size_t dot = rest.find(L'.');
        if (dot != std::wstring::npos && dot) names.insert(rest.substr(0, dot));
    }
    for (auto const& name : names) {
        auto app = std::make_unique<App>();
        app->name = name;
        app->title = name;
        app->style = L"generic";
        KnownApp const* known = nullptr;
        for (auto const& k : kKnown)
            if (name == k.name) known = &k;
        if (known) app->title = known->title, app->style = Utf8ToWide(known->style);
        app->on = ToInt(Setting(name + L".on")) != 0;
        app->exe = Setting(name + L".exe");
        if (app->exe.empty() && known && app->on) app->exe = FindExe(*known);
        app->exe = Expand(app->exe.c_str());
        std::replace(app->exe.begin(), app->exe.end(), L'/', L'\\');  // (a path typed with forward slashes)
        // Not in its usual folder, or given by name only: found wherever it was installed.
        if (app->on && (app->exe.empty() ? known != nullptr : app->exe.find(L'\\') == std::wstring::npos)) {
            std::wstring exe = app->exe.empty() ? std::wstring(known->exe) : app->exe;
            if (exe.size() < 4 || _wcsicmp(exe.c_str() + exe.size() - 4, L".exe") != 0) exe += L".exe";
            std::wstring found = Located(name, exe, known && known->squirrel);
            if (!found.empty()) app->exe = found;
        }
        if (!known && app->exe.empty()) continue;  // a name with nothing to start
        if (!known) app->title = app->exe.substr(app->exe.find_last_of(L'\\') + 1);
        app->restart = Setting(name + L".restart");
        std::wstring css = Setting(name + L".css");
        if (css.empty()) css = L"tourne/" + app->style + L".css";
        for (size_t at = 0; at <= css.size();) {
            size_t comma = css.find(L',', at);
            std::wstring one = css.substr(at, comma == std::wstring::npos ? std::wstring::npos : comma - at);
            one.erase(0, one.find_first_not_of(L" \t"));
            one.erase(one.find_last_not_of(L" \t") + 1);
            if (!one.empty()) app->css.push_back(one);
            if (comma == std::wstring::npos) break;
            at = comma + 1;
        }
        if (auto it = old.find(name); it != old.end() && it->second->connection) app->connection = std::move(it->second->connection);
        g_apps.push_back(std::move(app));
    }
    for (auto& [name, app] : old)
        if (app && app->connection) app->connection->Close();
}

void Update() {
    Rebuild();
    std::set<std::wstring> nowOn;
    bool anyAsk = false;
    for (auto& app : g_apps) {
        if (!app->on) {
            if (app->connection) {
                app->connection->Unstyle();
                app->connection->Close();
                app->connection.reset();
            }
            continue;
        }
        nowOn.insert(app->name);
        if (!Consented(*app)) {
            // Asked when turned on (or at the first start with it on, once).
            if (!g_wasOn.count(app->name) && (!g_firstApply || !GetIntValue(L"inject", (L"Asked_" + app->name).c_str(), 0))) {
                g_toAsk.insert(app->name);
                anyAsk = true;
            }
            continue;
        }
        if (app->connection) app->connection->SetCss(CssOf(*app));
        else if (g_firstApply) {
            // Started styled before Tourne Desktop restarted: connected again.
            DWORD pid = (DWORD)GetIntValue(L"inject", (L"Pid_" + app->name).c_str(), 0);
            int port = GetIntValue(L"inject", (L"Port_" + app->name).c_str(), 0);
            auto pids = RunningPids(*app);
            if (pid && port && std::find(pids.begin(), pids.end(), pid) != pids.end()) {
                g_askedPids.insert(pid);
                Connect(*app, pid, port);
            }
        }
    }
    g_wasOn = nowOn;
    g_firstApply = false;
    bool watch = false;
    for (auto& app : g_apps) watch |= app->on && Consented(*app) && app->restart != L"never";
    WatchForeground(watch);
    if (anyAsk) PostMessageW(g_wnd, WM_TD_INJECT_ASK, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Watching: style sheets saved, and apps opened without their styles.

DWORD WINAPI CssFolderThread(LPVOID) {
    HANDLE change = FindFirstChangeNotificationW(CssFolder().c_str(), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);
    if (change == INVALID_HANDLE_VALUE) return 0;
    HANDLE waits[2] = {change, g_cssStop};
    while (WaitForMultipleObjects(2, waits, FALSE, INFINITE) == WAIT_OBJECT_0) {
        PostMessageW(g_wnd, WM_TD_CSS_CHANGED, 0, 0);
        if (!FindNextChangeNotification(change)) break;
    }
    FindCloseChangeNotification(change);
    return 0;
}

void CALLBACK OnForeground(HWINEVENTHOOK, DWORD, HWND wnd, LONG idObject, LONG, DWORD, DWORD) {
    if (idObject != OBJID_WINDOW || !wnd) return;
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    if (pid && !g_askedPids.count(pid)) PostMessageW(g_wnd, WM_TD_FOREGROUND, pid, 0);
}

void WatchForeground(bool on) {
    if (on == (g_foregroundHook != nullptr)) return;
    if (on) {
        // Out of context: only a message to this thread when the front window changes.
        g_foregroundHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, OnForeground, 0, 0, WINEVENT_OUTOFCONTEXT);
    } else {
        UnhookWinEvent(g_foregroundHook);
        g_foregroundHook = nullptr;
    }
}

void OnAppInFront(DWORD pid) {
    if (g_askedPids.count(pid) || g_asking) return;
    std::wstring path = Lower(ProcessPath(pid));
    if (path.empty()) return;
    for (auto& app : g_apps) {
        if (!app->on || !Consented(*app) || app->exe.empty()) continue;
        std::wstring want = Lower(app->exe);
        std::wstring parent = want.substr(0, want.find_last_of(L'\\'));
        std::wstring root = parent.substr(0, parent.find_last_of(L'\\'));
        std::wstring file = L"\\" + want.substr(want.find_last_of(L'\\') + 1);  // ("\name.exe"; a bare name too)
        bool same = path == want || (path.rfind(root + L"\\app-", 0) == 0 && path.size() >= file.size() &&
                                     path.compare(path.size() - file.size(), file.size(), file) == 0);
        if (!same) continue;
        // Its own process, connected: nothing to do. Another of its processes too.
        if (app->connection) {
            auto pids = RunningPids(*app);
            if (std::find(pids.begin(), pids.end(), app->connection->pid()) != pids.end()) {
                g_askedPids.insert(pid);
                return;
            }
        }
        for (DWORD p : RunningPids(*app)) g_askedPids.insert(p);
        if (app->restart == L"always") {
            OpenStyled(*app);
            return;
        }
        g_asking = true;
        int answer = MessageBoxW(nullptr,
                                 (app->title + L" opened without your styles. Restart it with them? It's closed first.\n\n"
                                               L"To always restart it without asking: inject." + app->name +
                                  L".restart = always\nTo never ask: inject." + app->name + L".restart = never")
                                     .c_str(),
                                 L"Tourne Desktop", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST);
        g_asking = false;
        if (answer == IDYES) OpenStyled(*app);
        return;
    }
}

// The palette CSS: one for each app Tourne has a starter style for.
Roles g_roles = {0x121C21, 0x1B2A31, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x64AA89, 0x5D8D6F, 0xFF6F77, 0xFF8F8F, 0x66EA97, 0x15FFA2};

void WritePaletteCss() {
    std::wstring folder = CssFolder() + L"\\tourne";
    SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr);
    for (auto const& app : StyledApps()) WriteIfChanged(folder + L"\\" + Utf8ToWide(app) + L".css", AppCss(app, g_roles));
    WriteIfChanged(CssFolder() + L"\\README.txt",
                   "Style sheets for Tourne's CSS injector.\r\n\r\n"
                   "tourne\\  Tourne's own, made from your theme's colours and written again when the theme changes.\r\n"
                   "         Copy one out of it to change it, and point inject.<app>.css at your copy.\r\n"
                   "Anything else here is yours: your own styles, or ones you've downloaded.\r\n");
}

// The record's colours, if Tourne made a theme.
bool ReadRecord(Roles* r) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Theme", 0, KEY_READ, &key) != ERROR_SUCCESS) return false;
    struct { const wchar_t* name; uint32_t* role; } const roles[] = {
        {L"Ground", &r->ground}, {L"Raised", &r->raised}, {L"Line", &r->line}, {L"Edge", &r->edge}, {L"Text", &r->text},
        {L"Accent", &r->accent}, {L"Muted", &r->muted},   {L"Dim", &r->dim},   {L"Warn", &r->warn}, {L"Bad", &r->bad},
        {L"Accent2", &r->accent2}, {L"Good", &r->good}};
    Roles copy = *r;
    bool all = true;
    for (auto const& role : roles) {
        DWORD v = 0, size = sizeof(v);
        if (RegGetValueW(key, nullptr, role.name, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS) all = false;
        else *role.role = v & 0xFFFFFF;
    }
    RegCloseKey(key);
    if (!all) *r = copy;
    return all;
}

}  // namespace

void InjectorDefaults(Settings& out) {
    for (auto const& k : kKnown) {
        std::wstring base = std::wstring(L"inject.") + k.name;
        out[base + L".on"] = L"false";
        out[base + L".css"] = L"tourne/" + Utf8ToWide(k.style) + L".css";
        out[base + L".exe"] = L"";
        out[base + L".restart"] = L"ask";
    }
}

void InjectorStart(HWND host) {
    g_wnd = host;
    Connection::g_host = host;
    SHCreateDirectoryExW(nullptr, CssFolder().c_str(), nullptr);
    ReadRecord(&g_roles);
    WritePaletteCss();
    g_cssStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_cssThread = CreateThread(nullptr, 0, CssFolderThread, nullptr, 0, nullptr);
    Update();
}

void InjectorApply() { Update(); }

void InjectorThemeChanged(const Roles& roles) {
    g_roles = roles;
    WritePaletteCss();  // (the folder watch then restyles the apps using them)
}

void InjectorStop() {
    WatchForeground(false);
    if (g_cssThread) {
        SetEvent(g_cssStop);
        WaitForSingleObject(g_cssThread, 2000);
        CloseHandle(g_cssThread);
        CloseHandle(g_cssStop);
        g_cssThread = g_cssStop = nullptr;
    }
    // The styles stay in the apps until they close; the connections end here.
    for (auto& app : g_apps)
        if (app->connection) app->connection->Close();
    g_apps.clear();
}

bool InjectorMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_TD_CSS_CHANGED:
            SetTimer(wnd, kTimerCss, 200, nullptr);  // editors save in steps
            return true;
        case WM_TIMER:
            if (wp != kTimerCss) return false;
            KillTimer(wnd, kTimerCss);
            for (auto& app : g_apps)
                if (app->connection) app->connection->SetCss(CssOf(*app));
            return true;
        case WM_TD_INJECT_ENDED:
            for (auto& app : g_apps)
                if (app->connection && app->connection->pid() == (DWORD)wp) {
                    app->connection->Close();
                    app->connection.reset();
                }
            return true;
        case WM_TD_INJECT_ASK:
            if (g_asking) return true;
            g_asking = true;
            for (auto const& name : std::set<std::wstring>(std::move(g_toAsk))) {
                App* app = FindApp(name);
                if (!app || !app->on || Consented(*app)) continue;
                SetIntValue(L"inject", (L"Asked_" + app->name).c_str(), 1);
                AskToStyle(*app);
            }
            g_toAsk.clear();
            g_asking = false;
            Update();
            return true;
        case WM_TD_FOREGROUND:
            OnAppInFront((DWORD)wp);
            return true;
        case WM_TD_INJECT_FOUND:
            Update();
            return true;
    }
    (void)lp;
    return false;
}

void InjectorMenu(HMENU menu, UINT firstId) {
    HMENU sub = CreatePopupMenu();
    int i = 0;
    for (auto& app : g_apps) {
        if (app->on && Consented(*app))
            AppendMenuW(sub, MF_STRING | (app->connection ? MF_CHECKED : 0), firstId + 1 + i, (L"Open " + app->title + L" styled").c_str());
        i++;
    }
    if (GetMenuItemCount(sub)) AppendMenuW(sub, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(sub, MF_STRING, firstId, L"Open the style sheets folder");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)sub, L"Style apps");
}

bool InjectorMenuCommand(UINT id, UINT firstId) {
    if (id == firstId) {
        ShellExecuteW(nullptr, L"open", CssFolder().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return true;
    }
    size_t index = id - firstId - 1;
    if (id <= firstId || index >= g_apps.size()) return false;
    OpenStyled(*g_apps[index]);
    return true;
}

}  // namespace td
