// Programs Tourne Desktop leaves alone, as Windhawk has its exclusions. Tourne never loads into another
// program; two lists say what else it keeps from them:
//   - shell.exclude, games (shell.excludeGames: where games install, and anti-cheats' own programs) and
//     Windhawk's own exclusions (shell.excludeWindhawk, read from its settings): no border, shadow or
//     tiling around their windows, and the bar keeps away.
//   - shell.excludeKeys: while one of these is in front, Tourne's keyboard and mouse hooks (Alt+Tab, the
//     Start key, the killswitch) are let go, then put back. Empty by default: Alt+Tab and the Windows key
//     work in games unless a program is named here.
// Idle cost: none (it wakes when the window in front changes).
#include <windows.h>
#include <shlwapi.h>

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "standalone.h"
#include "td.h"

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"exclude", format, args);
    va_end(args);
}

// Where games install, and anti-cheats' own programs (lower case).
const wchar_t* const kGamePatterns[] = {
    L"*\\steamapps\\common\\*",  L"*\\epic games\\*",        L"*\\riot games\\*",       L"*\\riot vanguard\\*",
    L"*\\xboxgames\\*",          L"*\\ea games\\*",          L"*\\electronic arts\\*",  L"*\\battle.net\\*",
    L"*\\blizzard entertainment\\*", L"*\\ubisoft game launcher\\games\\*", L"*\\gog galaxy\\games\\*",
    L"*\\gog games\\*",          L"easyanticheat*.exe",      L"beservice*.exe",         L"eaanticheat*.exe",
    L"vgc.exe",                  L"vgtray.exe",              L"faceit*.exe",
};

struct Verdict {
    bool alone = false;  // no window effects
    bool keys = false;   // no hooks while in front
};

SRWLOCK g_lock = SRWLOCK_INIT;
std::vector<std::wstring> g_alone, g_keys;  // patterns, lower case
ULONGLONG g_patternsRead = 0;               // GetTickCount64 when read; 0: never
std::map<DWORD, Verdict> g_byProcess;       // pid: what's kept from it (cleared with the patterns)
std::vector<void (*)(bool)> g_watchers;
std::atomic<bool> g_front{false};           // a program in shell.excludeKeys is in front
std::once_flag g_watchStarted;

std::wstring Lower(std::wstring s) {
    if (!s.empty()) CharLowerBuffW(s.data(), (DWORD)s.size());
    return s;
}

void AddList(std::vector<std::wstring>& out, std::wstring const& list) {
    size_t at = 0;
    while (at <= list.size()) {
        size_t end = list.find_first_of(L"|,;\n", at);
        std::wstring item = list.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        while (!item.empty() && iswspace(item.back())) item.pop_back();
        while (!item.empty() && iswspace(item.front())) item.erase(0, 1);
        if (item.size() >= 2 && item.front() == L'"' && item.back() == L'"') item = item.substr(1, item.size() - 2);
        if (!item.empty()) out.push_back(Lower(item));
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
}

void AddSetting(std::vector<std::wstring>& out, const wchar_t* key) {
    const wchar_t* list = GetString(L"shell", key);
    if (list) AddList(out, list);
    FreeString(list);
}

// The lists, read again when they're a few seconds old (so a change in the Studio or Windhawk shows soon,
// with nothing watched in between).
void ReadPatterns() {
    ULONGLONG now = GetTickCount64();
    if (g_patternsRead && now - g_patternsRead < 3000) return;
    std::vector<std::wstring> alone, keys;
    AddSetting(alone, L"exclude");
    if (GetInt(L"shell", L"excludeGames"))
        for (auto p : kGamePatterns) alone.push_back(p);
    if (GetInt(L"shell", L"excludeWindhawk")) {
        WCHAR list[32768] = L"";
        DWORD bytes = sizeof(list);
        if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Settings", L"Exclude", RRF_RT_REG_SZ, nullptr, list,
                         &bytes) == ERROR_SUCCESS)
            AddList(alone, list);
    }
    AddSetting(keys, L"excludeKeys");
    g_alone.swap(alone);
    g_keys.swap(keys);
    g_byProcess.clear();
    g_patternsRead = now;
}

bool Matches(std::vector<std::wstring> const& patterns, std::wstring const& path) {
    std::wstring file = PathFindFileNameW(path.c_str());
    for (auto const& p : patterns) {
        bool full = p.find(L'\\') != std::wstring::npos;
        if (PathMatchSpecW((full ? path : file).c_str(), p.c_str())) return true;
    }
    return false;
}

std::wstring ProcessPath(DWORD pid) {
    WCHAR path[MAX_PATH * 2] = L"";
    DWORD size = ARRAYSIZE(path);
    if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        if (!QueryFullProcessImageNameW(p, 0, path, &size)) path[0] = 0;
        CloseHandle(p);
    }
    return path;
}

Verdict VerdictFor(HWND window) {
    DWORD pid = 0;
    if (!window || !GetWindowThreadProcessId(window, &pid) || !pid || pid == GetCurrentProcessId()) return {};
    AcquireSRWLockExclusive(&g_lock);
    ReadPatterns();
    auto it = g_byProcess.find(pid);
    bool known = it != g_byProcess.end();
    Verdict v = known ? it->second : Verdict{};
    ReleaseSRWLockExclusive(&g_lock);
    if (known) return v;
    std::wstring path = Lower(ProcessPath(pid));
    AcquireSRWLockExclusive(&g_lock);
    if (!path.empty()) v = {Matches(g_alone, path), Matches(g_keys, path)};
    g_byProcess[pid] = v;
    ReleaseSRWLockExclusive(&g_lock);
    return v;
}

void CALLBACK FrontChanged(HWINEVENTHOOK, DWORD, HWND window, LONG object, LONG, DWORD, DWORD) {
    if (object != OBJID_WINDOW || !window) return;
    bool now = VerdictFor(window).keys;
    if (now == g_front.exchange(now)) return;
    if (now) {
        DWORD pid = 0;
        GetWindowThreadProcessId(window, &pid);
        Log(L"in front, hooks let go: %s", ProcessPath(pid).c_str());
    } else {
        Log(L"hooks back");
    }
    std::vector<void (*)(bool)> watchers;
    AcquireSRWLockShared(&g_lock);
    watchers = g_watchers;
    ReleaseSRWLockShared(&g_lock);
    for (auto w : watchers) w(now);
}

DWORD WINAPI WatchThread(LPVOID) {
    HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, FrontChanged, 0, 0,
                                         WINEVENT_OUTOFCONTEXT);  // (Tourne's own windows in front count as back)
    FrontChanged(nullptr, 0, GetForegroundWindow(), OBJID_WINDOW, 0, 0, 0);
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) DispatchMessageW(&msg);
    if (hook) UnhookWinEvent(hook);
    return 0;
}

void StartWatching() {
    std::call_once(g_watchStarted, [] {
        if (HANDLE thread = CreateThread(nullptr, 0, WatchThread, nullptr, 0, nullptr)) CloseHandle(thread);
    });
}

}  // namespace

bool ProgramPathExcluded(std::wstring const& path) {
    if (path.empty()) return false;
    AcquireSRWLockExclusive(&g_lock);
    ReadPatterns();
    bool excluded = Matches(g_alone, Lower(path));
    ReleaseSRWLockExclusive(&g_lock);
    return excluded;
}

bool ProgramExcluded(HWND window) {
    return VerdictFor(window).alone;
}

bool ExcludedInFront() {
    StartWatching();
    return g_front.load();
}

void WatchExcludedFront(void (*callback)(bool inFront)) {
    AcquireSRWLockExclusive(&g_lock);
    g_watchers.push_back(callback);
    ReleaseSRWLockExclusive(&g_lock);
    StartWatching();
}

}  // namespace td
