#include "shell-mode.h"

#include <shellapi.h>
#include <shlobj.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "td.h"
#include "desktop-host.h"
#include "killswitch.h"
#include "tray-host.h"

namespace td {

namespace {

constexpr wchar_t kWinlogon[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
constexpr wchar_t kOurKey[] = L"Software\\Tourne\\Desktop";
constexpr wchar_t kSessionKey[] = L"Software\\Tourne\\Desktop\\Session";  // volatile: gone at sign-out
// Volatile too, the watchdog's own (not kSessionKey: its first making starts the startup apps). Its
// PlannedRestart value tells Swap-TourneDesktop.ps1 this watchdog knows kPlannedRestart.
constexpr wchar_t kWatchdogKey[] = L"Software\\Tourne\\Desktop\\Watchdog";
constexpr DWORD kQuickCrash = 120000;  // a second crash within this of the first turns shell mode off
constexpr DWORD kLateExplorer = 120000;  // Explorer underneath still taken on when it comes up this late
constexpr wchar_t kWindowsAppsValue[] = L"ShellWindowsApps";   // shell.windowsApps, for the watchdog (no settings there)
constexpr wchar_t kAutoHideValue[] = L"UnderneathTaskbarState";  // Explorer's taskbar state before, plus one

std::wstring Folder(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, nullptr, &path))) out = path;
    CoTaskMemFree(path);
    return out;
}

std::wstring DataFolder() { return Folder(FOLDERID_LocalAppData) + L"\\Tourne\\Desktop"; }

// shell.log, always written: sign-in is when there's nobody to ask.
void Note(const wchar_t* format, ...) {
    wchar_t text[1024];
    va_list args;
    va_start(args, format);
    vswprintf(text, 1024, format, args);
    va_end(args);
    CreateDirectoryW((Folder(FOLDERID_LocalAppData) + L"\\Tourne").c_str(), nullptr);
    CreateDirectoryW(DataFolder().c_str(), nullptr);
    if (FILE* f = _wfopen((DataFolder() + L"\\shell.log").c_str(), L"a, ccs=UTF-8")) {
        time_t now = time(nullptr);
        tm local;
        localtime_s(&local, &now);
        wchar_t stamp[32];
        wcsftime(stamp, 32, L"%Y-%m-%d %H:%M:%S", &local);
        fwprintf(f, L"%ls %ls\n", stamp, text);
        fclose(f);
    }
}

std::wstring ThisExe() {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    return exe;
}

std::wstring KeptExe() { return DataFolder() + L"\\bin\\TourneDesktop.exe"; }

std::wstring ShellCommand() { return L"\"" + KeptExe() + L"\" --shell"; }

std::wstring CurrentShell() {
    wchar_t value[1024];
    DWORD size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER, kWinlogon, L"Shell", RRF_RT_REG_SZ, nullptr, value, &size) != ERROR_SUCCESS) return L"";
    return value;
}

bool ShellIsOurs() { return CurrentShell().find(L"TourneDesktop") != std::wstring::npos; }

bool SetShell(const std::wstring& command) {
    return RegSetKeyValueW(HKEY_CURRENT_USER, kWinlogon, L"Shell", REG_SZ, command.c_str(),
                           (DWORD)((command.size() + 1) * sizeof(wchar_t))) == ERROR_SUCCESS;
}

void ClearShell() {
    if (ShellIsOurs()) RegDeleteKeyValueW(HKEY_CURRENT_USER, kWinlogon, L"Shell");
}

bool SameFile(const std::wstring& a, const std::wstring& b) {
    WIN32_FILE_ATTRIBUTE_DATA x, y;
    if (!GetFileAttributesExW(a.c_str(), GetFileExInfoStandard, &x) || !GetFileAttributesExW(b.c_str(), GetFileExInfoStandard, &y))
        return false;
    return x.nFileSizeLow == y.nFileSizeLow && x.nFileSizeHigh == y.nFileSizeHigh &&
           CompareFileTime(&x.ftLastWriteTime, &y.ftLastWriteTime) == 0;
}

// This exe (and its manifest, if it has one beside it) kept where the shell
// value points. One running from there is replaced by moving it aside first.
bool KeepCopy() {
    std::wstring self = ThisExe(), kept = KeptExe();
    if (_wcsicmp(self.c_str(), kept.c_str()) == 0 || SameFile(self, kept)) return true;
    CreateDirectoryW((DataFolder() + L"\\bin").c_str(), nullptr);
    std::wstring old = kept + L".old";
    DeleteFileW(old.c_str());
    if (GetFileAttributesW(kept.c_str()) != INVALID_FILE_ATTRIBUTES && !MoveFileExW(kept.c_str(), old.c_str(), MOVEFILE_REPLACE_EXISTING))
        return false;
    if (!CopyFileW(self.c_str(), kept.c_str(), FALSE)) {
        MoveFileExW(old.c_str(), kept.c_str(), MOVEFILE_REPLACE_EXISTING);
        return false;
    }
    std::wstring manifest = self + L".manifest";
    if (GetFileAttributesW(manifest.c_str()) != INVALID_FILE_ATTRIBUTES) CopyFileW(manifest.c_str(), (kept + L".manifest").c_str(), FALSE);
    return true;
}

HWND WaitForWindow(const wchar_t* cls, DWORD ms) {
    for (DWORD waited = 0; waited < ms; waited += 250) {
        if (HWND w = FindWindowW(cls, nullptr)) return w;
        Sleep(250);
    }
    return nullptr;
}

// Explorer as the shell for this session: it is the shell when the shell value
// names it, so ours is taken out while it starts (and put back if keep).
void StartExplorer(bool keep) {
    std::wstring ours = CurrentShell();
    if (!ours.empty() && ShellIsOurs()) RegDeleteKeyValueW(HKEY_CURRENT_USER, kWinlogon, L"Shell");
    wchar_t windows[MAX_PATH];
    GetWindowsDirectoryW(windows, MAX_PATH);
    std::wstring explorer = std::wstring(windows) + L"\\explorer.exe";
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(explorer.c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, windows, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Note(L"Explorer started%ls", WaitForWindow(L"Shell_TrayWnd", 20000) ? L" (its taskbar is up)" : L", but its taskbar didn't appear");
    } else {
        Note(L"Explorer didn't start (error %lu)", GetLastError());
    }
    if (keep && !ours.empty() && ShellIsOurs() == false && ours.find(L"TourneDesktop") != std::wstring::npos) SetShell(ours);
}

////////////////////////////////////////////////////////////////////////////////
// Explorer underneath (shell.windowsApps): the shell Windows apps need, kept
// out of sight.

bool IsExplorerProcess(DWORD pid) {
    if (!pid || pid == GetCurrentProcessId()) return false;
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return false;
    wchar_t path[MAX_PATH];
    DWORD size = MAX_PATH;
    bool explorer = false;
    if (QueryFullProcessImageNameW(process, 0, path, &size)) {
        const wchar_t* name = wcsrchr(path, L'\\');
        explorer = !_wcsicmp(name ? name + 1 : path, L"explorer.exe");
    }
    CloseHandle(process);
    return explorer;
}

bool WindowsAppsWanted() {
    DWORD v = 0, size = sizeof(v);
    return RegGetValueW(HKEY_CURRENT_USER, kOurKey, kWindowsAppsValue, RRF_RT_REG_DWORD, nullptr, &v, &size) == ERROR_SUCCESS && v;
}

void LaunchExplorerUnderneath() {
    wchar_t windows[MAX_PATH];
    GetWindowsDirectoryW(windows, MAX_PATH);
    std::wstring explorer = std::wstring(windows) + L"\\explorer.exe";
    std::wstring line = L"\"" + explorer + L"\" /NoShellRegistrationCheck";
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(explorer.c_str(), line.data(), nullptr, nullptr, FALSE, 0, nullptr, windows, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        Note(L"Explorer underneath didn't start (error %lu)", GetLastError());
    }
}

// (Looked for often: its taskbar and desktop are hidden as soon as they're there, and Tourne starts sooner.)
HWND WaitForExplorerTray(DWORD ms) {
    for (DWORD waited = 0; waited < ms; waited += 25) {
        if (HWND w = ExplorerWindow(L"Shell_TrayWnd")) return w;
        Sleep(25);
    }
    return nullptr;
}

// Explorer's taskbars and its desktop: hidden (or shown again). The desktop too, so it never paints over the
// screen in the moment before Tourne's own is up.
void ShowExplorerTaskbars(bool show) {
    for (const wchar_t* cls : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman"})
        for (HWND h = nullptr; (h = FindWindowExW(nullptr, h, cls, nullptr));) {
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            if (IsExplorerProcess(pid)) ShowWindow(h, show ? SW_SHOWNA : SW_HIDE);
        }
}

// Its auto-hide state as it was (Tourne's own tray, when there is one, would take the message instead).
void RestoreExplorerAutoHide() {
    DWORD saved = 0, size = sizeof(saved);
    if (RegGetValueW(HKEY_CURRENT_USER, kOurKey, kAutoHideValue, RRF_RT_REG_DWORD, nullptr, &saved, &size) != ERROR_SUCCESS || !saved)
        return;
    APPBARDATA abd{sizeof(abd)};
    abd.hWnd = ExplorerWindow(L"Shell_TrayWnd");
    if (abd.hWnd && FindWindowW(L"Shell_TrayWnd", nullptr) == abd.hWnd) {
        abd.lParam = saved - 1;
        SHAppBarMessage(ABM_SETSTATE, &abd);
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kOurKey, kAutoHideValue);
    }
}

// The watchdog's part, before Tourne Desktop starts: Explorer up as the shell
// (it is, with no shell window yet and the registration check skipped).
void StartExplorerUnderneath() {
    if (ExplorerWindow(L"Shell_TrayWnd")) return;
    ULONGLONG start = GetTickCount64();
    LaunchExplorerUnderneath();
    if (WaitForExplorerTray(20000)) {
        ShowExplorerTaskbars(false);  // (Tourne Desktop keeps them hidden from here)
        ULONGLONG tray = GetTickCount64() - start;
        // Its desktop comes just after its taskbar: hidden too when it does (briefly looked for; Tourne Desktop
        // hides it otherwise).
        bool desktop = false;
        for (int i = 0; i < 40 && !desktop; i++) {
            desktop = ExplorerWindow(L"Progman") != nullptr;
            if (!desktop) Sleep(25);
        }
        if (desktop) ShowExplorerTaskbars(false);
        Note(L"Explorer started underneath, for Windows apps (taskbar after %llu ms, desktop %ls; hidden)", tray,
             desktop ? L"after it" : L"not yet");
    } else {
        Note(L"Explorer underneath didn't come up: Tourne Desktop alone");
    }
}

std::atomic<bool> g_underneath;
DWORD g_explorerPid;
HANDLE g_explorerProcess, g_explorerWait;
HWINEVENTHOOK g_shownHook, g_frontHook;
std::vector<HWND> g_hidden;  // Explorer's windows hidden here (shown again on quitting)

bool IsExplorerPart(HWND w) {
    wchar_t cls[64];
    if (GetAncestor(w, GA_PARENT) != GetDesktopWindow() || !GetClassNameW(w, cls, 64)) return false;
    for (const wchar_t* part : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW"})
        if (!wcscmp(cls, part)) return true;
    return false;
}

void HidePart(HWND w) {
    if (!IsWindowVisible(w)) return;
    ShowWindow(w, SW_HIDE);
    if (std::find(g_hidden.begin(), g_hidden.end(), w) == g_hidden.end()) g_hidden.push_back(w);
}

void HideExplorerParts() {
    EnumWindows(
        [](HWND w, LPARAM) -> BOOL {
            DWORD pid = 0;
            GetWindowThreadProcessId(w, &pid);
            if (pid == g_explorerPid && IsExplorerPart(w)) HidePart(w);
            return TRUE;
        },
        0);
    ShowExplorerTaskbars(false);  // (hidden by the watchdog already: not in g_hidden, shown again all the same)
}

bool g_explorerBarShown;  // Explorer's taskbars shown on purpose (the explorerbar command): left alone

bool IsExplorerTaskbar(HWND w) {
    wchar_t cls[64];
    return GetClassNameW(w, cls, 64) && (!wcscmp(cls, L"Shell_TrayWnd") || !wcscmp(cls, L"Shell_SecondaryTrayWnd"));
}

void CALLBACK PartShown(HWINEVENTHOOK, DWORD, HWND w, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || !w) return;
    if (g_explorerBarShown && IsExplorerTaskbar(w)) return;
    if (IsExplorerPart(w)) HidePart(w);
    TrayHostRaise();
}

void CALLBACK FrontChanged(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD) { TrayHostRaise(); }

void CALLBACK ExplorerEnded(PVOID, BOOLEAN) {
    // (Windows may start it again itself; its "TaskbarCreated" then hides it again)
    Sleep(3000);
    if (ExplorerWindow(L"Shell_TrayWnd")) return;
    TrayHostExplorerEnded();  // (RelaunchExplorer, on the main thread)
}

void Unwatch() {
    if (g_explorerWait) UnregisterWaitEx(g_explorerWait, nullptr);
    if (g_explorerProcess) CloseHandle(g_explorerProcess);
    if (g_shownHook) UnhookWinEvent(g_shownHook);
    if (g_frontHook) UnhookWinEvent(g_frontHook);
    g_explorerWait = g_explorerProcess = nullptr;
    g_shownHook = g_frontHook = nullptr;
}

// Explorer's windows hidden and kept so, its auto-hide on (a hidden taskbar
// that isn't still keeps its space), and its end noticed.
void Watch(HWND tray) {
    Unwatch();
    GetWindowThreadProcessId(tray, &g_explorerPid);
    APPBARDATA abd{sizeof(abd)};
    abd.hWnd = tray;
    if (FindWindowW(L"Shell_TrayWnd", nullptr) == tray) {  // (before Tourne's tray comes first)
        UINT state = (UINT)SHAppBarMessage(ABM_GETSTATE, &abd);
        if (!(state & ABS_AUTOHIDE)) {
            DWORD saved = 0, size = sizeof(saved);
            if (RegGetValueW(HKEY_CURRENT_USER, kOurKey, kAutoHideValue, RRF_RT_REG_DWORD, nullptr, &saved, &size) != ERROR_SUCCESS) {
                DWORD value = state + 1;
                RegSetKeyValueW(HKEY_CURRENT_USER, kOurKey, kAutoHideValue, REG_DWORD, &value, sizeof(value));
            }
            abd.lParam = state | ABS_AUTOHIDE;
            SHAppBarMessage(ABM_SETSTATE, &abd);
        }
    }
    HideExplorerParts();
    g_shownHook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, PartShown, g_explorerPid, 0, WINEVENT_OUTOFCONTEXT);
    g_frontHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, FrontChanged, 0, 0, WINEVENT_OUTOFCONTEXT);
    g_explorerProcess = OpenProcess(SYNCHRONIZE, FALSE, g_explorerPid);
    if (g_explorerProcess)
        RegisterWaitForSingleObject(&g_explorerWait, g_explorerProcess, ExplorerEnded, nullptr, INFINITE, WT_EXECUTEONLYONCE);
}

// Explorer started while Tourne's "Shell_TrayWnd" or "Progman" is there takes itself for a second copy
// (folder windows only, no shell), and minimized windows then sit as little title bars at the bottom left.
// So Tourne's tray and desktop step aside until Explorer's taskbar is up (a moment's blank desktop), and are
// made again in front of it.
UINT_PTR g_relaunchTimer;
DWORD g_relaunchStarted;

void CALLBACK RelaunchCheck(HWND, UINT, UINT_PTR, DWORD) {
    HWND tray = ExplorerWindow(L"Shell_TrayWnd");
    if (!tray && GetTickCount() - g_relaunchStarted < 20000) return;
    KillTimer(nullptr, g_relaunchTimer);
    g_relaunchTimer = 0;
    if (tray) {
        TrayHostStart();
        DesktopHostStart();
        Watch(tray);
        TrayHostRetake();
        Note(L"Explorer underneath started again: hidden, tray taken back");
    } else {
        Unwatch();
        g_underneath = false;
        TrayHostStart();
        DesktopHostStart();  // (Windows' shell window now)
        Note(L"Explorer underneath didn't come back: Tourne Desktop alone");
    }
}

void RelaunchExplorer() {
    if (g_relaunchTimer || ExplorerWindow(L"Shell_TrayWnd")) return;
    Note(L"Explorer underneath ended: starting it again, Tourne's tray and desktop aside meanwhile");
    TrayHostStop();
    DesktopHostStop();
    LaunchExplorerUnderneath();
    g_relaunchStarted = GetTickCount();
    g_relaunchTimer = SetTimer(nullptr, 0, 250, RelaunchCheck);
}

// Explorer underneath slow to come up (after a restart, say): Tourne Desktop
// started without it, so it is taken on when it appears, for a while.
std::atomic<bool> g_waiting;
DWORD g_waitStarted;
UINT_PTR g_waitTimer;

void StopWaiting() {
    if (g_waitTimer) KillTimer(nullptr, g_waitTimer);
    g_waitTimer = 0;
    g_waiting = false;
}

void TakeOnLateExplorer(HWND tray) {
    g_underneath = true;
    StopWaiting();
    Watch(tray);
    TrayHostRetake();  // (Tourne's tray first in line, and the icons asked for again)
    KillswitchApply();  // (its keys caught too)
    Note(L"Explorer underneath came up after Tourne Desktop (%lu s): hidden, tray taken over",
         (GetTickCount() - g_waitStarted) / 1000);
}

void CALLBACK LateExplorerCheck(HWND, UINT, UINT_PTR, DWORD) {
    if (!g_waiting) return;
    if (HWND tray = ExplorerWindow(L"Shell_TrayWnd")) {
        TakeOnLateExplorer(tray);
        return;
    }
    if (GetTickCount() - g_waitStarted < kLateExplorer) return;
    StopWaiting();
    DesktopHostClaimShell();  // (kept back for Explorer: Tourne's desktop is Windows' shell window after all)
    Note(L"Explorer underneath never came up: Tourne Desktop alone");
}

void SignalShellReady() {
    for (const wchar_t* name : {L"msgina: ShellReadyEvent", L"ShellDesktopSwitchEvent"}) {
        if (HANDLE e = OpenEventW(EVENT_MODIFY_STATE, FALSE, name)) {
            SetEvent(e);
            CloseHandle(e);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Startup apps, as Explorer starts them.

// Task Manager's Startup page keeps "disabled" here: the first byte odd.
bool Disabled(HKEY root, const wchar_t* approvedKey, const std::wstring& name) {
    BYTE data[16];
    DWORD size = sizeof(data);
    if (RegGetValueW(root, approvedKey, name.c_str(), RRF_RT_REG_BINARY, nullptr, data, &size) != ERROR_SUCCESS || size == 0)
        return false;
    return (data[0] & 1) != 0;
}

void Launch(const std::wstring& command) {
    wchar_t expanded[4096];
    ExpandEnvironmentStringsW(command.c_str(), expanded, 4096);
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    std::vector<wchar_t> line(expanded, expanded + wcslen(expanded) + 1);
    if (CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return;
    }
    // A bare document or a path with arguments CreateProcess can't split: the shell's way.
    std::wstring file = expanded, params;
    if (!file.empty() && file[0] == L'"') {
        size_t end = file.find(L'"', 1);
        if (end != std::wstring::npos) params = file.substr(end + 1), file = file.substr(1, end - 1);
    }
    ShellExecuteW(nullptr, nullptr, file.c_str(), params.empty() ? nullptr : params.c_str(), nullptr, SW_SHOWNORMAL);
}

void RunKey(HKEY root, const wchar_t* key, REGSAM view, const wchar_t* approvedKey, bool once, int* started) {
    HKEY k;
    if (RegOpenKeyExW(root, key, 0, KEY_READ | (once ? KEY_SET_VALUE : 0) | view, &k) != ERROR_SUCCESS) return;
    std::vector<std::pair<std::wstring, std::wstring>> items;
    wchar_t name[256];
    for (DWORD i = 0;; i++) {
        DWORD nameLen = 256, type = 0, size = 0;
        if (RegEnumValueW(k, i, name, &nameLen, nullptr, &type, nullptr, &size) != ERROR_SUCCESS) break;
        if (type != REG_SZ && type != REG_EXPAND_SZ) continue;
        std::wstring value(size / sizeof(wchar_t) + 1, L'\0');
        DWORD got = (DWORD)(value.size() * sizeof(wchar_t));
        nameLen = 256;
        if (RegEnumValueW(k, i, name, &nameLen, nullptr, nullptr, (BYTE*)value.data(), &got) != ERROR_SUCCESS) continue;
        value.resize(wcslen(value.c_str()));
        items.push_back({name, value});
    }
    for (auto const& [n, command] : items) {
        if (once) RegDeleteValueW(k, n.c_str());
        if (n == L"TourneDesktop" || (approvedKey && Disabled(root, approvedKey, n))) continue;
        Note(L"startup: %ls", n.c_str());
        Launch(command);
        ++*started;
        Sleep(150);
    }
    RegCloseKey(k);
}

void RunFolder(REFKNOWNFOLDERID id, HKEY approvedRoot, int* started) {
    std::wstring folder = Folder(id);
    if (folder.empty()) return;
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"\\*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)) continue;
        if (_wcsicmp(fd.cFileName, L"desktop.ini") == 0) continue;
        if (Disabled(approvedRoot, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder", fd.cFileName))
            continue;
        Note(L"startup: %ls", fd.cFileName);
        ShellExecuteW(nullptr, nullptr, (folder + L"\\" + fd.cFileName).c_str(), nullptr, folder.c_str(), SW_SHOWNORMAL);
        ++*started;
        Sleep(150);
    } while (FindNextFileW(find, &fd));
    FindClose(find);
}

// Explorer's own mark that it ran them this sign-in. Explorer underneath never does (the shell value
// names Tourne, so it isn't the shell to itself: 2026-10-16, nothing from Run or Startup came up).
bool ExplorerRanStartupApps() {
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    wchar_t key[128];
    swprintf(key, 128, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\StartupHasBeenRun", session);
    HKEY k;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &k) != ERROR_SUCCESS) return false;
    RegCloseKey(k);
    return true;
}

DWORD WINAPI StartupApps(LPVOID) {
    Sleep(1500);  // (Tourne's own bar up first)
    if (ExplorerRanStartupApps()) {
        Note(L"startup apps: Explorer started them already");
        return 0;
    }
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    constexpr wchar_t kRun[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    constexpr wchar_t kRunOnce[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    constexpr wchar_t kApproved[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";
    constexpr wchar_t kApproved32[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32";
    int started = 0;
    RunKey(HKEY_CURRENT_USER, kRunOnce, 0, nullptr, true, &started);
    RunKey(HKEY_LOCAL_MACHINE, kRun, KEY_WOW64_64KEY, kApproved, false, &started);
    RunKey(HKEY_LOCAL_MACHINE, kRun, KEY_WOW64_32KEY, kApproved32, false, &started);
    RunKey(HKEY_CURRENT_USER, kRun, 0, kApproved, false, &started);
    RunFolder(FOLDERID_CommonStartup, HKEY_LOCAL_MACHINE, &started);
    RunFolder(FOLDERID_Startup, HKEY_CURRENT_USER, &started);
    Note(L"startup apps: %d started", started);
    CoUninitialize();
    return 0;
}

bool g_asShell;

}  // namespace

void ShellDefaults(Settings& out) {
    out[L"shell.enabled"] = L"0";
    out[L"shell.startupApps"] = L"1";
    out[L"shell.desktop"] = L"1";
    out[L"shell.windowsApps"] = L"0";
    out[L"shell.marquee.fade"] = L"0";
    out[L"shell.marquee.duration"] = L"300";
    out[L"shell.marquee.speed"] = L"100";
    out[L"shell.marquee.autoOpacity"] = L"0";
    out[L"shell.marquee.opacity"] = L"90";
    out[L"shell.marquee.quickWhenCovered"] = L"1";
    out[L"shell.marquee.oneAtATime"] = L"0";
    out[L"shell.marquee.fill"] = L"";
    out[L"shell.marquee.border"] = L"";
    out[L"shell.iconLook"] = L"windows";
    out[L"shell.iconLook.pickedFill"] = L"";
    out[L"shell.iconLook.pickedEdge"] = L"";
    out[L"shell.iconLook.pickedOpacity"] = L"90";
    out[L"shell.iconLook.hoverFill"] = L"";
    out[L"shell.iconLook.hoverEdge"] = L"";
    out[L"shell.iconLook.hoverOpacity"] = L"45";
    out[L"shell.iconLook.edgeOpacity"] = L"220";
    out[L"shell.iconLook.outline"] = L"1";
    out[L"shell.iconLook.outlineColor"] = L"";
    out[L"shell.desktopMenu"] = L"0";
    out[L"shell.wallpaper.folder"] = L"";
    out[L"shell.wallpaper.minutes"] = L"0";
    out[L"shell.wallpaper.shuffle"] = L"0";
    out[L"shell.exclude"] = L"";
    out[L"shell.excludeGames"] = L"1";
    out[L"shell.excludeWindhawk"] = L"1";
    out[L"shell.excludeKeys"] = L"";
}

void ShellDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"shell", L"Shell mode",
        L"Tourne Desktop as your shell in place of Explorer: Windows' taskbar, Start and desktop never start, until you turn this "
        L"off. File Explorer windows still work. From your next sign-in. Hold Shift while signing in for Explorer once; if Tourne "
        L"Desktop crashes twice in a row, Explorer comes back and this turns off. TourneDesktop.exe --restore also turns it off.");
    add(L"shell.enabled", L"Tourne Desktop is my shell",
        L"Kept in %LOCALAPPDATA%\\Tourne\\Desktop\\bin, so the copy you run now can be moved. Apps' tray icons show in Tourne's "
        L"bar. Not here: Win+A and Win+N (they're Explorer's).")
        .isSwitch = true;
    add(L"shell.startupApps", L"Start my startup apps",
        L"As Explorer does: the apps in Task Manager's Startup page that are on, once each sign-in.")
        .isSwitch = true;
    add(L"shell.desktop", L"Show the desktop",
        L"Your wallpaper and desktop icons, with the desktop's right-click menu, as Explorer shows them. Off, the screen behind "
        L"windows is Windows' plain wallpaper with no icons.")
        .isSwitch = true;
    add(L"shell.windowsApps", L"Windows apps work (Explorer hidden underneath)",
        L"Settings, the Store and its apps, Windows Security and Phone Link only open with Explorer running as the shell. On, "
        L"Explorer starts first, out of sight: its taskbar and desktop stay hidden, Tourne's bar keeps the tray icons, and "
        L"Windows' own notification pop-ups come back. Explorer starts your startup apps then. From your next sign-in.")
        .isSwitch = true;
    auto range = [](SettingInfo& si, double low, double high) {
        si.hasMin = si.hasMax = true;
        si.min = low;
        si.max = high;
    };
    add(L"shell.marquee", L"Selection box",
        L"The box you drag on the desktop to pick icons. As the Desktop Marquee Fade mod did for Explorer's desktop, done here: "
        L"it fades out when you let go instead of vanishing.");
    add(L"shell.marquee.fade", L"Fade it out when let go", L"").isSwitch = true;
    range(add(L"shell.marquee.duration", L"Fade time (ms)", L"How long the fade lasts at 100% speed."), 100, 2000);
    range(add(L"shell.marquee.speed", L"Speed (%)", L"Over 100 shortens the fade, under 100 lengthens it."), 50, 500);
    add(L"shell.marquee.autoOpacity", L"Opacity to suit the time",
        L"Starts brighter for a short fade and dimmer for a long one, instead of the opacity below.")
        .isSwitch = true;
    range(add(L"shell.marquee.opacity", L"Starting opacity", L"Of the box's inside, 1 to 255. Its edge starts at 190."), 1, 255);
    add(L"shell.marquee.quickWhenCovered", L"Quicker when a window comes to the front",
        L"Four times as fast while the desktop isn't the window in front, so it's out of the way.")
        .isSwitch = true;
    add(L"shell.marquee.oneAtATime", L"One at a time", L"A new box ends the last one's fade.").isSwitch = true;
    add(L"shell.marquee.fill", L"Inside colour", L"A hex colour, like #000000. Empty: Windows' selection colour.");
    add(L"shell.marquee.border", L"Edge colour", L"A hex colour, like #15FFA2. Empty: Windows' selection colour.");
    add(L"shell.iconLook", L"Desktop icons' hover and selection",
        L"How a desktop icon looks under the pointer, when picked, and its keyboard outline. Windows' own comes from the "
        L"visual style; Tourne's uses the colours below. A colour is a hex colour (#15FFA2) or a Tourne theme colour's name "
        L"(accent, accent2, text, muted, dim, edge, line, raised, ground, good, warn, bad), so it follows the theme.")
        .options = {{L"windows", L"Windows' own"}, {L"tourne", L"Tourne's colours"}};
    add(L"shell.iconLook.pickedFill", L"Picked: inside", L"Empty: the theme's accent.");
    add(L"shell.iconLook.pickedEdge", L"Picked: edge", L"Empty: the theme's accent.");
    range(add(L"shell.iconLook.pickedOpacity", L"Picked: inside opacity", L"0 to 255."), 0, 255);
    add(L"shell.iconLook.hoverFill", L"Under the pointer: inside", L"Empty: the theme's text colour.");
    add(L"shell.iconLook.hoverEdge", L"Under the pointer: edge", L"Empty: the theme's edge colour.");
    range(add(L"shell.iconLook.hoverOpacity", L"Under the pointer: inside opacity", L"0 to 255."), 0, 255);
    range(add(L"shell.iconLook.edgeOpacity", L"Edge opacity", L"Both edges, 0 to 255 (0: no edge)."), 0, 255);
    add(L"shell.iconLook.outline", L"Keyboard outline",
        L"The dotted outline on the icon the keyboard is at (Windows shows it once you've used the keyboard). Off: never.")
        .isSwitch = true;
    add(L"shell.iconLook.outlineColor", L"Keyboard outline colour", L"Empty: the theme's text colour.");
    add(L"shell.desktopMenu", L"The desktop's right-click menu in Tourne's look",
        L"The shell's own menu for the desktop and its icons (Paste, New, Open in Terminal...), with View, Sort by and Refresh, "
        L"styled like the bar's menus and without icons when Menu No Icons is on. Shift + right-click gives Windows' own.")
        .isSwitch = true;
    add(L"shell.wallpaper", L"Wallpaper",
        L"Chosen from the desktop's right-click menu (Wallpaper: Choose a picture, Next picture, the last few, Placement), set "
        L"as Windows sets it, so Settings and the lock screen agree. Needs the menu in Tourne's look, above.");
    add(L"shell.wallpaper.folder", L"Pictures folder",
        L"Where Next picture and the slideshow take pictures from. Empty: the folder of the one showing.");
    range(add(L"shell.wallpaper.minutes", L"Change every (minutes)", L"A slideshow from the folder above. 0: off."), 0, 1440);
    add(L"shell.wallpaper.shuffle", L"Shuffle", L"Any picture in the folder next, instead of the one after by name.").isSwitch = true;
    add(L"shell.exclude", L"Programs left alone",
        L"As Windhawk's exclusions: Tourne never loads into another program, and for these it also draws no border, shadow or "
        L"tiling around their windows, and the bar keeps away. Alt+Tab and the Windows key still work in them (see Keys left "
        L"alone in). File names or full paths, with * and ?, with | between, like League of Legends.exe|*\\Riot Games\\*.");
    add(L"shell.excludeGames", L"Games and anti-cheats",
        L"Everything in Steam's, Epic's, Riot's, Xbox's, EA's, Battle.net's, Ubisoft's and GOG's game folders, and Vanguard, "
        L"EasyAntiCheat, BattlEye, EA AntiCheat and FACEIT.")
        .isSwitch = true;
    add(L"shell.excludeWindhawk", L"Windhawk's exclusions too",
        L"Whatever Windhawk is set to stay out of (its Settings, Advanced, Process exclusion list), read from it.")
        .isSwitch = true;
    add(L"shell.excludeKeys", L"Keys left alone in",
        L"Programs where Tourne lets go of its keyboard and mouse hooks while one is in front (Alt+Tab, the Windows key, the "
        L"killswitch), put back when you leave it: for a game whose anti-cheat objects. Empty: they work everywhere. Same "
        L"form as above.");
}

bool TurnedOffByCrash() {
    DWORD v = 0, size = sizeof(v);
    return RegGetValueW(HKEY_CURRENT_USER, kOurKey, L"ShellTurnedOff", RRF_RT_REG_DWORD, nullptr, &v, &size) == ERROR_SUCCESS && v;
}

void ShellTryAgain() { RegDeleteKeyValueW(HKEY_CURRENT_USER, kOurKey, L"ShellTurnedOff"); }

void ShellApply() {
    DesktopHostSettingsChanged();
    DWORD apps = GetInt(L"shell", L"windowsApps") ? 1 : 0;
    if (apps != (WindowsAppsWanted() ? 1u : 0u))
        RegSetKeyValueW(HKEY_CURRENT_USER, kOurKey, kWindowsAppsValue, REG_DWORD, &apps, sizeof(apps));
    bool on = GetInt(L"shell", L"enabled") != 0;
    if (on && TurnedOffByCrash()) {  // (until a setting is saved, as the crash guard does)
        static bool told = false;
        if (!told) Note(L"shell mode stays off: Tourne Desktop crashed twice at sign-in (save a setting to try again)");
        told = true;
        return;
    }
    if (on) {
        if (!KeepCopy()) {
            Note(L"couldn't keep a copy in %%LOCALAPPDATA%%\\Tourne\\Desktop\\bin: shell mode not set");
            return;
        }
        if (CurrentShell() != ShellCommand()) {
            SetShell(ShellCommand());
            Note(L"shell mode on: %ls from the next sign-in", ShellCommand().c_str());
        }
    } else if (ShellIsOurs() && !g_asShell) {
        ClearShell();
        Note(L"shell mode off: Explorer from the next sign-in");
    }
}

int ShellWatchdog() {
    // One a session: Explorer started underneath as the shell starts the registered shell too (this),
    // and two would both start Tourne Desktop again after it ends.
    CreateMutexW(nullptr, TRUE, L"Local\\TourneDesktop_ShellWatchdog");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Note(L"a watchdog is running already: this one leaves it to that");
        return 0;
    }
    // In front of anyone reading this log after a bad start.
    Note(L"stuck with no desktop? Task Manager > Run new task > \"%ls\" --panic puts Explorer back", ThisExe().c_str());
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        Note(L"Shift held at sign-in: Explorer this time");
        StartExplorer(true);
        return 0;
    }
    TrayCollectorStart();  // (apps started at sign-in find a tray before Explorer's: elevated ones never hear ours later)
    if (WindowsAppsWanted()) StartExplorerUnderneath();  // (Explorer started again by Windows: this too)
    std::wstring self = ThisExe();
    HKEY watchdog;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kWatchdogKey, 0, nullptr, REG_OPTION_VOLATILE, KEY_SET_VALUE, nullptr, &watchdog,
                        nullptr) == ERROR_SUCCESS) {
        DWORD one = 1;
        RegSetValueExW(watchdog, L"PlannedRestart", 0, REG_DWORD, (const BYTE*)&one, sizeof(one));
        RegCloseKey(watchdog);
    }
    DWORD firstCrash = 0;
    for (;;) {
        std::wstring line = L"\"" + self + L"\" --shell-child";
        STARTUPINFOW si{sizeof(si)};
        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(self.c_str(), line.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            Note(L"Tourne Desktop didn't start (error %lu): Explorer instead", GetLastError());
            StartExplorer(true);
            return 1;
        }
        CloseHandle(pi.hThread);
        Note(L"Tourne Desktop started as the shell");
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hProcess);
        if (code == kAlreadyRunning) {  // a copy was running already: it's the shell, watched instead
            DWORD pid = 0;
            if (HWND host = FindWindowW(L"TourneDesktop_Host", nullptr)) GetWindowThreadProcessId(host, &pid);
            HANDLE other = pid ? OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) : nullptr;
            if (!other) {
                Sleep(1000);
                continue;
            }
            Note(L"Tourne Desktop was already running: watching that one");
            WaitForSingleObject(other, INFINITE);
            code = 1;
            GetExitCodeProcess(other, &code);
            CloseHandle(other);
        }
        // Signing out, restarting or shutting down ends it from outside (0x40010004, or 0xC000026B
        // when it was starting again): not a crash, and nothing to start or turn off.
        if (GetSystemMetrics(SM_SHUTTINGDOWN) || code == 0x40010004 || code == 0xC000026B) {
            Note(L"Tourne Desktop ended with the session (code 0x%08lX)", code);
            return 0;
        }
        if (code == kPlannedRestart) {  // a new build swapped in: started again, and no crash counted
            Note(L"Tourne Desktop restarted for a new build");
            continue;
        }
        bool underneath = ExplorerWindow(L"Shell_TrayWnd") != nullptr;
        if (code == 0) {  // quit on purpose: nobody is left without a shell
            Note(L"Tourne Desktop quit: Explorer for the rest of this session");
            if (underneath) ShowExplorerTaskbars(true), RestoreExplorerAutoHide();
            else StartExplorer(true);
            return 0;
        }
        DWORD now = GetTickCount();
        if (firstCrash && now - firstCrash < kQuickCrash) {
            Note(L"Tourne Desktop ended again (code 0x%08lX): Explorer, and shell mode off", code);
            DWORD one = 1;
            RegSetKeyValueW(HKEY_CURRENT_USER, kOurKey, L"ShellTurnedOff", REG_DWORD, &one, sizeof(one));
            if (underneath) {
                ShowExplorerTaskbars(true);
                RestoreExplorerAutoHide();
                if (HWND desk = ExplorerWindow(L"Progman")) ShowWindow(desk, SW_SHOWNA);
                ClearShell();
            } else {
                StartExplorer(false);
            }
            return 1;
        }
        Note(L"Tourne Desktop ended (code 0x%08lX): starting it again", code);
        firstCrash = now;
    }
}

bool StartedAsShell(const std::wstring& args) {
    g_asShell = args.find(L"--shell-child") != std::wstring::npos;
    return g_asShell;
}

void ShellSessionStart() {
    if (!g_asShell) return;
    HKEY session;
    DWORD made = 0;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kSessionKey, 0, nullptr, REG_OPTION_VOLATILE, KEY_READ, nullptr, &session, &made) ==
        ERROR_SUCCESS)
        RegCloseKey(session);
    bool firstStart = made == REG_CREATED_NEW_KEY;  // (once a sign-in)
    // Started again mid-session (a new build, a crash) with Explorer underneath gone: the watchdog brings
    // it up only at sign-in, so it's brought up here, before Tourne's tray is there to put it off.
    if (!firstStart && WindowsAppsWanted() && !ExplorerWindow(L"Shell_TrayWnd")) {
        LaunchExplorerUnderneath();
        WaitForExplorerTray(15000);
    }
    if (HWND tray = WindowsAppsWanted() ? ExplorerWindow(L"Shell_TrayWnd") : nullptr) {
        g_underneath = true;
        Watch(tray);
        Note(L"Explorer runs underneath, hidden (for Windows apps)");
    } else if (WindowsAppsWanted()) {
        g_waiting = true;
        g_waitStarted = GetTickCount();
        g_waitTimer = SetTimer(nullptr, 0, 1000, LateExplorerCheck);
        Note(L"Explorer underneath isn't up yet: waiting for it");
    }
    TrayHostStart();  // (before Windows is told: apps starting now find the tray)
    DesktopHostStart();
    if (!g_underneath) SignalShellReady();
    if (firstStart && GetInt(L"shell", L"startupApps"))  // (Explorer underneath isn't the shell to itself: ours too)
        if (HANDLE t = CreateThread(nullptr, 0, StartupApps, nullptr, 0, nullptr)) CloseHandle(t);
}

void ShellSessionStop() {
    StopWaiting();
    if (!g_underneath) return;
    Unwatch();
    for (HWND w : g_hidden)
        if (IsWindow(w)) ShowWindow(w, SW_SHOWNA);
    g_hidden.clear();
    ShowExplorerTaskbars(true);
    RestoreExplorerAutoHide();  // (Tourne's tray is gone by now: Explorer's takes it)
    g_underneath = false;
}

bool ShellUnderneath() { return g_underneath; }

bool ShellShowExplorerBar(bool show) {
    if (!g_underneath) return false;
    g_explorerBarShown = show;
    // Shown without waiting on Explorer (a synchronous call into its taskbar from here once never came back). Its
    // auto-hide is left as Tourne set it: the taskbar slides up when the pointer reaches the screen's edge.
    for (const wchar_t* cls : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"})
        for (HWND h = nullptr; (h = FindWindowExW(nullptr, h, cls, nullptr));) {
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            if (IsExplorerProcess(pid)) ShowWindowAsync(h, show ? SW_SHOWNA : SW_HIDE);
        }
    Note(show ? L"Explorer's taskbar shown in Tourne's place" : L"Explorer's taskbar hidden again, Tourne's bar back");
    return true;
}

bool ShellExplorerExpected() { return g_underneath || g_waiting; }

HWND ExplorerWindow(const wchar_t* cls) {
    for (HWND h = nullptr; (h = FindWindowExW(nullptr, h, cls, nullptr));) {
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        if (IsExplorerProcess(pid)) return h;
    }
    return nullptr;
}

void ShellRelaunchExplorer() { RelaunchExplorer(); }

void ShellExplorerRestarted() {
    if (g_waiting) {  // (its first start, late: taken on now rather than at the next check)
        if (HWND tray = ExplorerWindow(L"Shell_TrayWnd")) TakeOnLateExplorer(tray);
        return;
    }
    if (!g_underneath) return;
    HWND tray = ExplorerWindow(L"Shell_TrayWnd");
    if (!tray) return;
    Watch(tray);
    TrayHostRetake();
    Note(L"Explorer underneath started again: hidden, tray taken back");
}

void ShellRestore() {
    if (ShellIsOurs()) {
        ClearShell();
        Note(L"--restore: Explorer is the shell again from the next sign-in");
    }
}

////////////////////////////////////////////////////////////////////////////////
// --panic: Explorer back now, not at the next sign-in. For when there's no
// desktop left to click in, run from Task Manager's "Run new task".

namespace {

// Every other TourneDesktop.exe ended. Only for a copy that wouldn't close:
// the watchdog goes too, or it starts the next one over the top of Explorer.
void EndOtherCopies() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;
    PROCESSENTRY32W entry{sizeof(entry)};
    for (BOOL more = Process32FirstW(snap, &entry); more; more = Process32NextW(snap, &entry)) {
        if (entry.th32ProcessID == GetCurrentProcessId() || _wcsicmp(entry.szExeFile, L"TourneDesktop.exe")) continue;
        if (HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID)) {
            TerminateProcess(process, 1);
            CloseHandle(process);
        }
    }
    CloseHandle(snap);
}

}  // namespace

int ShellPanic() {
    Note(L"--panic: putting Explorer back");
    bool wasOurs = ShellIsOurs();
    ClearShell();
    DWORD one = 1;  // the same flag two crashes set: shell mode stays off until the switch is saved again
    RegSetKeyValueW(HKEY_CURRENT_USER, kOurKey, L"ShellTurnedOff", REG_DWORD, &one, sizeof(one));

    // Asked to quit first: quitting on purpose is what hands the session to
    // Explorer, so the watchdog does the rest itself.
    bool running = false, closed = false;
    if (HWND host = FindWindowW(L"TourneDesktop_Host", nullptr)) {  // (kHostClass, which lives in host.cpp)
        running = true;
        DWORD pid = 0;
        GetWindowThreadProcessId(host, &pid);
        HANDLE process = pid ? OpenProcess(SYNCHRONIZE, FALSE, pid) : nullptr;
        PostMessageW(host, WM_CLOSE, 0, 0);
        if (process) {
            closed = WaitForSingleObject(process, 8000) == WAIT_OBJECT_0;
            CloseHandle(process);
        }
        Note(closed ? L"--panic: Tourne Desktop quit" : L"--panic: Tourne Desktop wouldn't quit, ending it");
        if (!closed) EndOtherCopies();
    }

    // Explorer's own taskbar and desktop are already there when it ran
    // underneath; otherwise it has to be started.
    if (ExplorerWindow(L"Shell_TrayWnd")) {
        ShowExplorerTaskbars(true);
        RestoreExplorerAutoHide();
        if (HWND desk = ExplorerWindow(L"Progman")) ShowWindow(desk, SW_SHOWNA);
        Note(L"--panic: Explorer was underneath, it's showing now");
    } else if (!(running && closed && wasOurs)) {
        StartExplorer(false);  // (a clean quit of our own shell leaves the watchdog to start it)
    } else {
        Note(L"--panic: the watchdog is starting Explorer");
    }
    Note(L"--panic: done. Shell mode is off until you turn it on again in Studio.");
    return 0;
}

}  // namespace td
