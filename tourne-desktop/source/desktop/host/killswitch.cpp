#include "killswitch.h"

#include <shellapi.h>

#include <algorithm>
#include <vector>

#include "shell-mode.h"
#include "standalone.h"
#include "td.h"

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"killswitch", format, args);
    va_end(args);
}

constexpr wchar_t kKey[] = L"Software\\Tourne\\Desktop\\Killswitch";
constexpr wchar_t kSavedValue[] = L"TaskbarState";  // the auto-hide state before, plus one (0: none saved)
constexpr wchar_t kGuardClass[] = L"TourneDesktop_EdgeGuard";
constexpr int kGuardThickness = 3;

HWND g_host;
bool g_on, g_keysOn, g_edgeOn, g_started;
UINT g_taskbarCreated;
HHOOK g_keyHook;
HWINEVENTHOOK g_frontHook;
std::vector<HWND> g_guards;

// Keys
bool g_winHeld, g_winUsed, g_masked, g_winOwned;
DWORD g_swallowUp;  // a key whose release goes with a press swallowed

bool Setting(const wchar_t* key) { return GetInt(L"killswitch", key) != 0; }

std::vector<HWND> SystemTaskbars() {
    std::vector<HWND> bars;
    if (HWND tray = ExplorerWindow(L"Shell_TrayWnd")) bars.push_back(tray);  // (not Tourne's own, in shell mode)
    for (HWND h = nullptr; (h = FindWindowExW(nullptr, h, L"Shell_SecondaryTrayWnd", nullptr));) bars.push_back(h);
    return bars;
}

DWORD Saved() {
    DWORD value = 0, size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER, kKey, kSavedValue, RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value;
}

void AutoHide() {
    if (ShellUnderneath()) return;  // (shell mode does it, and its taskbar is hidden: shell-mode.cpp)
    APPBARDATA abd{sizeof(abd)};
    abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!abd.hWnd) return;
    UINT state = (UINT)SHAppBarMessage(ABM_GETSTATE, &abd);
    if (!Saved()) {
        DWORD value = state + 1;
        RegSetKeyValueW(HKEY_CURRENT_USER, kKey, kSavedValue, REG_DWORD, &value, sizeof(value));
    }
    if (!(state & ABS_AUTOHIDE)) {
        abd.lParam = state | ABS_AUTOHIDE;
        SHAppBarMessage(ABM_SETSTATE, &abd);
    }
}

////////////////////////////////////////////////////////////////////////////////
// The edge guards: one over each taskbar's waiting edge.

LRESULT CALLBACK GuardProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEACTIVATE: return MA_NOACTIVATE;
        case WM_NCHITTEST: return HTCLIENT;
        case WM_SETCURSOR: SetCursor(LoadCursorW(nullptr, IDC_ARROW)); return TRUE;
        case WM_DISPLAYCHANGE:
        case WM_SETTINGCHANGE:
            if (g_host) PostMessageW(g_host, WM_APP + 50, 0, 0);  // (laid out again, once)
            break;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void RaiseGuards();

void CloseGuards() {
    for (HWND g : g_guards) DestroyWindow(g);
    g_guards.clear();
}

// The edge a taskbar waits at: the one its middle is nearest (an auto-hidden
// bar is almost all past it).
RECT GuardRect(HWND bar) {
    RECT r{}, out{};
    GetWindowRect(bar, &r);
    MONITORINFO mi{sizeof(mi)};
    if (!GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi)) return out;
    RECT m = mi.rcMonitor;
    long cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
    long toLeft = cx - m.left, toRight = m.right - cx, toTop = cy - m.top, toBottom = m.bottom - cy;
    long nearest = std::min(std::min(toLeft, toRight), std::min(toTop, toBottom));
    out = m;
    if (nearest == toBottom) out.top = m.bottom - kGuardThickness;
    else if (nearest == toTop) out.bottom = m.top + kGuardThickness;
    else if (nearest == toLeft) out.right = m.left + kGuardThickness;
    else out.left = m.right - kGuardThickness;
    return out;
}

void LayOutGuards() {
    CloseGuards();
    if (!g_on || !g_edgeOn) return;
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = GuardProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kGuardClass;
        registered = RegisterClassW(&wc) != 0;
    }
    for (HWND bar : SystemTaskbars()) {
        RECT r = GuardRect(bar);
        if (r.right <= r.left || r.bottom <= r.top) continue;
        HWND g = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE, kGuardClass, L"", WS_POPUP,
                                 r.left, r.top, r.right - r.left, r.bottom - r.top, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!g) continue;
        SetLayeredWindowAttributes(g, 0, 1, LWA_ALPHA);  // (all but invisible, and still under the pointer)
        ShowWindow(g, SW_SHOWNOACTIVATE);
        g_guards.push_back(g);
    }
    RaiseGuards();
}

// Tourne's own bars (the taskbar part's windows): a guard leaves out where one
// lies, so the bar keeps every click at the edge.
std::vector<HWND> TourneBars() {
    std::vector<HWND> bars;
    EnumWindows(
        [](HWND h, LPARAM lp) -> BOOL {
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            wchar_t name[64];
            if (pid == GetCurrentProcessId() && IsWindowVisible(h) && GetClassNameW(h, name, 64) &&
                wcsncmp(name, L"TourneTaskbarTest_", 18) == 0)
                ((std::vector<HWND>*)lp)->push_back(h);
            return TRUE;
        },
        (LPARAM)&bars);
    return bars;
}

void RaiseGuards() {
    auto bars = TourneBars();
    for (HWND g : g_guards) {
        RECT gr;
        GetWindowRect(g, &gr);
        HRGN shape = CreateRectRgn(0, 0, gr.right - gr.left, gr.bottom - gr.top);
        for (HWND bar : bars) {
            RECT br, both;
            GetWindowRect(bar, &br);
            if (!IntersectRect(&both, &br, &gr)) continue;
            HRGN cut = CreateRectRgn(both.left - gr.left, both.top - gr.top, both.right - gr.left, both.bottom - gr.top);
            CombineRgn(shape, shape, cut, RGN_DIFF);
            DeleteObject(cut);
        }
        SetWindowRgn(g, shape, FALSE);  // (the window owns it now)
        SetWindowPos(g, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
}

void CALLBACK FrontChanged(HWINEVENTHOOK, DWORD, HWND, LONG, LONG, DWORD, DWORD) { RaiseGuards(); }

////////////////////////////////////////////////////////////////////////////////
// The keys

void SendMask() {
    INPUT in[2]{};
    in[0].type = in[1].type = INPUT_KEYBOARD;
    in[0].ki.wVk = in[1].ki.wVk = 0xE8;  // unassigned: Windows then sees the Windows key used
    in[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, in, sizeof(INPUT));
}

bool Down(WPARAM wp) { return wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN; }

LRESULT CALLBACK KeyProc(int code, WPARAM wp, LPARAM lp) {
    if (code != HC_ACTION) return CallNextHookEx(nullptr, code, wp, lp);
    auto* key = (KBDLLHOOKSTRUCT*)lp;
    DWORD vk = key->vkCode;
    bool down = Down(wp);
    // Tourne's own Start swallows a lone Windows key's release and sends its
    // own: that one counts as letting go too
    if (!down && (vk == VK_LWIN || vk == VK_RWIN) && (key->flags & LLKHF_INJECTED)) g_winHeld = false;
    if (key->flags & LLKHF_INJECTED) return CallNextHookEx(nullptr, code, wp, lp);
    if (!down && vk == g_swallowUp) {
        g_swallowUp = 0;
        return 1;
    }
    if (vk == VK_LWIN || vk == VK_RWIN) {
        if (down) {
            if (!g_winHeld) g_winUsed = g_masked = false;
            g_winHeld = true;
            return CallNextHookEx(nullptr, code, wp, lp);
        }
        bool alone = g_winHeld && !g_winUsed;
        g_winHeld = false;
        if (alone && !g_winOwned) {  // Windows' Start would open: it doesn't
            INPUT in[3]{};
            in[0].type = in[1].type = in[2].type = INPUT_KEYBOARD;
            in[0].ki.wVk = in[1].ki.wVk = 0xE8;
            in[1].ki.dwFlags = KEYEVENTF_KEYUP;
            in[2].ki.wVk = (WORD)vk;
            in[2].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY;
            SendInput(3, in, sizeof(INPUT));
            return 1;
        }
        return CallNextHookEx(nullptr, code, wp, lp);
    }
    // (a Windows key let go unseen, by another hook or around sign-in, mustn't
    // leave T, B and the numbers blocked: it has to be down now)
    if (g_winHeld && GetAsyncKeyState(VK_LWIN) >= 0 && GetAsyncKeyState(VK_RWIN) >= 0) g_winHeld = g_winUsed = false;
    if (down && g_winHeld) {
        g_winUsed = true;
        bool taskbarKey = vk == 'T' || vk == 'B' || (vk >= '0' && vk <= '9');
        if (taskbarKey) {
            if (!g_masked) SendMask(), g_masked = true;
            g_swallowUp = vk;
            return 1;
        }
    }
    if (down && vk == VK_ESCAPE && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && !g_winHeld) {  // Ctrl+Esc: Windows' Start
        g_swallowUp = vk;
        return 1;
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

// Tourne's own Start menu has the lone Windows key (start-menu.cpp).
bool TourneStartHasWinKey() {
    return GetInt(L"taskbar", L"taskbar.enabled") && GetInt(L"taskbar", L"taskbar.startMenu.enabled") &&
           GetInt(L"taskbar", L"taskbar.startMenu.winKey");
}

void Hook(bool on) {
    if (on && !g_keyHook) g_keyHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyProc, GetModuleHandleW(nullptr), 0);
    else if (!on && g_keyHook) UnhookWindowsHookEx(g_keyHook), g_keyHook = nullptr;
    g_winHeld = g_winUsed = g_masked = false;
    g_swallowUp = 0;
}

void TurnOn() {
    AutoHide();
    LayOutGuards();
    Hook(g_keysOn && !ExcludedInFront());  // (no hook while a program Tourne leaves alone, a game, is in front)
    if (g_edgeOn && !g_frontHook)
        g_frontHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, FrontChanged, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (!g_edgeOn && g_frontHook) UnhookWinEvent(g_frontHook), g_frontHook = nullptr;
}

void TurnOff() {
    Hook(false);
    if (g_frontHook) UnhookWinEvent(g_frontHook), g_frontHook = nullptr;
    CloseGuards();
    KillswitchRestoreSaved();
}

}  // namespace

void KillswitchDefaults(Settings& out) {
    out[L"killswitch.enabled"] = L"0";
    out[L"killswitch.keys"] = L"1";
    out[L"killswitch.edge"] = L"1";
}

void KillswitchDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"killswitch", L"Windows' taskbar off",
        L"Windows' taskbar and Start kept away while Tourne Desktop runs, from outside Explorer. Quitting Tourne Desktop, or "
        L"TourneDesktop.exe --restore after a crash, puts the taskbar back as it was. An app flashing for attention can still "
        L"make it peek.");
    add(L"killswitch.enabled", L"Windows' taskbar and Start off", L"It's set to auto-hide, and kept from coming back.").isSwitch = true;
    add(L"killswitch.keys", L"Catch the keys that open them",
        L"Ctrl+Esc, Win+T, Win+B, Win+1 to 0, and the Windows key on its own (unless Tourne's Start menu has it).")
        .isSwitch = true;
    add(L"killswitch.edge", L"Cover the edge it waits at",
        L"An invisible sliver over that screen edge, so pointing there doesn't bring it up.")
        .isSwitch = true;
}

void KillswitchStart(HWND host) {
    g_host = host;
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    g_started = true;
    if (!Setting(L"enabled")) KillswitchRestoreSaved();  // (left auto-hidden by a crash)
    KillswitchApply();
    // A program left alone (a game) coming to the front or going: the keys' hook let go or back, here.
    static bool watching = false;
    if (!watching) {
        watching = true;
        WatchExcludedFront([](bool) {
            if (g_host) PostMessageW(g_host, WM_APP + 51, 0, 0);
        });
    }
}

void KillswitchApply() {
    if (!g_started) return;
    bool on = Setting(L"enabled") || ShellUnderneath();  // (Explorer hidden underneath: its keys caught too)
    bool keys = on && Setting(L"keys"), edge = on && Setting(L"edge");
    g_winOwned = TourneStartHasWinKey();
    if (on == g_on && keys == g_keysOn && edge == g_edgeOn) return;
    g_on = on, g_keysOn = keys, g_edgeOn = edge;
    if (on) {
        TurnOn();
        Log(L"on: Windows' taskbar auto-hidden, keys %s, edge %s", keys ? L"caught" : L"left", edge ? L"covered" : L"left");
    } else {
        TurnOff();
        Log(L"off: Windows' taskbar as it was");
    }
}

void KillswitchStop() {
    if (!g_started) return;
    if (g_on) TurnOff();
    g_on = g_keysOn = g_edgeOn = g_started = false;
}

bool KillswitchMessage(HWND, UINT msg, WPARAM, LPARAM) {
    if (!g_on) return false;
    if (msg == g_taskbarCreated && g_taskbarCreated) {  // Explorer restarted: all of it again
        TurnOn();
        Log(L"Explorer restarted: Windows' taskbar off again");
        return false;  // (the host's tray icon wants it too)
    }
    if (msg == WM_APP + 50) {
        LayOutGuards();
        return true;
    }
    if (msg == WM_APP + 51) {  // a program left alone came to the front, or went
        Hook(g_keysOn && !ExcludedInFront());
        return true;
    }
    return false;
}

void KillswitchRestoreSaved() {
    DWORD saved = Saved();
    if (!saved || ShellUnderneath()) return;  // (Tourne's tray would take the message: kept for later)
    APPBARDATA abd{sizeof(abd)};
    abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (abd.hWnd) {
        abd.lParam = saved - 1;
        SHAppBarMessage(ABM_SETSTATE, &abd);
    }
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kKey, kSavedValue);
}

}  // namespace td
