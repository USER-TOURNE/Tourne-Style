#include "app-bars.h"

#include <shellapi.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "td.h"

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"app bars", format, args);
    va_end(args);
}

// SHAppBarMessage's request (WM_COPYDATA, dwData 0): the same from 32 and
// 64-bit programs, with handles as 32 bits (they fit) and lParam as 64.
struct AppBarData {
    DWORD cbSize;
    DWORD hWnd;
    DWORD uCallbackMessage;
    DWORD uEdge;
    RECT rc;
    int64_t lParam;
};
struct AppBarRequest {
    AppBarData abd;
    DWORD dwMessage;
    DWORD padding1;
    int64_t hSharedMemory;  // the APPBARDATA to answer in, for the messages that answer with a rectangle
    DWORD dwSourceProcessId;
    DWORD padding2;
};
static_assert(sizeof(AppBarData) == 40 && sizeof(AppBarRequest) == 64, "SHAppBarMessage's layout");

constexpr UINT kGetAutoHideBarEx = 0x0B;  // ABM_GETAUTOHIDEBAREX
constexpr UINT kSetAutoHideBarEx = 0x0C;  // ABM_SETAUTOHIDEBAREX

struct Bar {
    HWND wnd = nullptr;
    UINT callback = 0;
    UINT edge = ABE_BOTTOM;
    RECT rc{};
    bool placed = false;  // (after its first ABM_SETPOS)
};
struct AutoHide {
    HMONITOR screen;
    UINT edge;
    HWND wnd;
};

std::vector<Bar> g_bars;  // in the order they came: earlier ones keep their place
std::vector<AutoHide> g_autoHide;
UINT g_state = ABS_ALWAYSONTOP;  // (what ABM_GETSTATE tells: Tourne's bar is always on top)
bool g_setWorkArea;              // a work area of ours is in place
HMONITOR g_fullscreen;           // the screen a program fills, if one does
HWINEVENTHOOK g_frontHook, g_moveHook;
DWORD g_movePid;

using LockShared = void*(WINAPI*)(HANDLE, DWORD);
using UnlockShared = BOOL(WINAPI*)(void*);
LockShared g_lock;
UnlockShared g_unlock;

// The program behind a window, for the log.
std::wstring ProgramOf(HWND wnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    HANDLE process = pid ? OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) : nullptr;
    if (!process) return L"?";
    wchar_t path[MAX_PATH * 2];
    DWORD length = MAX_PATH * 2;
    std::wstring name = QueryFullProcessImageNameW(process, 0, path, &length) ? path : L"?";
    CloseHandle(process);
    size_t slash = name.find_last_of(L"\\/");
    return slash == std::wstring::npos ? name : name.substr(slash + 1);
}

const wchar_t* EdgeName(UINT edge) {
    static const wchar_t* names[] = {L"left", L"top", L"right", L"bottom"};
    return edge < 4 ? names[edge] : L"?";
}

HMONITOR ScreenOf(const RECT& rc) { return MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST); }

Bar* Find(HWND wnd) {
    for (auto& b : g_bars)
        if (b.wnd == wnd) return &b;
    return nullptr;
}

// Tells bars (all, or those on one screen) something, without waiting on them.
void Tell(HWND except, WPARAM code, LPARAM value, HMONITOR only = nullptr) {
    for (auto const& b : g_bars) {
        if (b.wnd == except || !b.callback) continue;
        if (only && (b.placed ? ScreenOf(b.rc) : MonitorFromWindow(b.wnd, MONITOR_DEFAULTTONEAREST)) != only) continue;
        PostMessageW(b.wnd, b.callback, code, value);
    }
}

// A rectangle a bar asks for, moved off the bars before it on its screen:
// past those on the same edge, and inside those on the edges either side.
void Fit(const Bar* self, UINT edge, RECT& rc) {
    HMONITOR screen = ScreenOf(rc);
    for (auto const& b : g_bars) {
        if (&b == self) break;
        if (!b.placed || ScreenOf(b.rc) != screen) continue;
        bool across = edge == ABE_TOP || edge == ABE_BOTTOM;
        switch (b.edge) {
            case ABE_TOP:
                if (edge == ABE_TOP || !across) rc.top = std::max(rc.top, b.rc.bottom);
                break;
            case ABE_BOTTOM:
                if (edge == ABE_BOTTOM || !across) rc.bottom = std::min(rc.bottom, b.rc.top);
                break;
            case ABE_LEFT:
                if (edge == ABE_LEFT || across) rc.left = std::max(rc.left, b.rc.right);
                break;
            case ABE_RIGHT:
                if (edge == ABE_RIGHT || across) rc.right = std::min(rc.right, b.rc.left);
                break;
        }
    }
    rc.bottom = std::max(rc.bottom, rc.top);
    rc.right = std::max(rc.right, rc.left);
}

struct Maximised {
    HWND wnd;
    RECT rc;
};

// Maximised windows on a screen, where they are now.
std::vector<Maximised> MaximisedOn(HMONITOR screen) {
    struct Search {
        HMONITOR screen;
        std::vector<Maximised> found;
    } search{screen, {}};
    EnumWindows(
        [](HWND w, LPARAM lp) -> BOOL {
            auto* s = (Search*)lp;
            RECT rc;
            if (IsWindowVisible(w) && IsZoomed(w) && MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST) == s->screen &&
                GetWindowRect(w, &rc))
                s->found.push_back({w, rc});
            return TRUE;
        },
        (LPARAM)&search);
    return search.found;
}

void SetWorkArea(HMONITOR screen, const RECT& was, const RECT& now) {
    std::vector<Maximised> windows = MaximisedOn(screen);
    if (!SystemParametersInfoW(SPI_SETWORKAREA, 0, (void*)&now, 0)) {
        Log(L"couldn't set a work area (error %lu)", GetLastError());
        return;
    }
    g_setWorkArea = true;
    Log(L"work area %ld %ld %ld %ld", now.left, now.top, now.right, now.bottom);
    // Maximised windows follow it, by as much as each side moved (their
    // frames hang past the work area, and keep doing so). Ones Windows has
    // already moved, or that weren't fitted to the old work area, are left be.
    auto fitted = [&](const RECT& rc) {
        auto out = [](long by) { return by >= 0 && by <= 32; };
        return out(was.left - rc.left) && out(was.top - rc.top) && out(rc.right - was.right) && out(rc.bottom - was.bottom);
    };
    for (auto const& m : windows) {
        RECT rc;
        if (!IsWindow(m.wnd) || !GetWindowRect(m.wnd, &rc) || !EqualRect(&rc, &m.rc) || !fitted(rc)) continue;
        RECT to{rc.left + now.left - was.left, rc.top + now.top - was.top, rc.right + now.right - was.right,
                rc.bottom + now.bottom - was.bottom};
        SetWindowPos(m.wnd, nullptr, to.left, to.top, to.right - to.left, to.bottom - to.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
    }
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETWORKAREA, 0);
}

std::vector<HMONITOR> Screens() {
    std::vector<HMONITOR> screens;
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM lp) -> BOOL {
            ((std::vector<HMONITOR>*)lp)->push_back(m);
            return TRUE;
        },
        (LPARAM)&screens);
    return screens;
}

// Each screen's work area: the screen less the bars on its edges.
void WorkAreas() {
    for (HMONITOR screen : Screens()) {
        MONITORINFO mi{sizeof(mi)};
        if (!GetMonitorInfoW(screen, &mi)) continue;
        RECT area = mi.rcMonitor;
        for (auto const& b : g_bars) {
            if (!b.placed || ScreenOf(b.rc) != screen) continue;
            switch (b.edge) {
                case ABE_TOP: area.top = std::max(area.top, b.rc.bottom); break;
                case ABE_BOTTOM: area.bottom = std::min(area.bottom, b.rc.top); break;
                case ABE_LEFT: area.left = std::max(area.left, b.rc.right); break;
                case ABE_RIGHT: area.right = std::min(area.right, b.rc.left); break;
            }
        }
        if (area.right - area.left < 64 || area.bottom - area.top < 64) area = mi.rcMonitor;  // (bars filling the screen: ignored)
        if (!EqualRect(&area, &mi.rcWork)) SetWorkArea(screen, mi.rcWork, area);
    }
}

// Bars whose windows are gone are taken out.
void Prune() {
    size_t before = g_bars.size();
    std::erase_if(g_bars, [](Bar const& b) { return !IsWindow(b.wnd); });
    std::erase_if(g_autoHide, [](AutoHide const& a) { return !IsWindow(a.wnd); });
    if (g_bars.size() != before) {
        WorkAreas();
        Tell(nullptr, ABN_POSCHANGED, 0);
    }
}

// Runs f on the APPBARDATA the asking program shared for the answer.
template <class F>
bool WithShared(const AppBarRequest& r, F&& f) {
    if (!g_lock || !g_unlock || !r.hSharedMemory) return false;
    void* view = g_lock((HANDLE)(ULONG_PTR)r.hSharedMemory, r.dwSourceProcessId);
    if (!view) return false;
    f(*(AppBarData*)view);
    g_unlock(view);
    return true;
}

// Where Tourne's bar is, for ABM_GETTASKBARPOS: the one on the main screen.
bool TaskbarPos(AppBarData& d) {
    struct Search {
        HWND found = nullptr;
    } search;
    EnumWindows(
        [](HWND w, LPARAM lp) -> BOOL {
            DWORD pid = 0;
            GetWindowThreadProcessId(w, &pid);
            wchar_t cls[64];
            if (pid != GetCurrentProcessId() || !IsWindowVisible(w) || !GetClassNameW(w, cls, 64) ||
                wcsncmp(cls, L"TourneTaskbarTest_", 18) != 0)
                return TRUE;
            auto* s = (Search*)lp;
            HMONITOR main = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
            if (!s->found || MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST) == main) s->found = w;
            return TRUE;
        },
        (LPARAM)&search);
    RECT rc;
    if (!search.found || !GetWindowRect(search.found, &rc)) return false;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromWindow(search.found, MONITOR_DEFAULTTONEAREST), &mi);
    long cx = (rc.left + rc.right) / 2, cy = (rc.top + rc.bottom) / 2;
    long mx = (mi.rcMonitor.left + mi.rcMonitor.right) / 2, my = (mi.rcMonitor.top + mi.rcMonitor.bottom) / 2;
    d.rc = rc;
    d.uEdge = rc.right - rc.left >= rc.bottom - rc.top ? (cy < my ? ABE_TOP : ABE_BOTTOM) : (cx < mx ? ABE_LEFT : ABE_RIGHT);
    d.hWnd = (DWORD)(ULONG_PTR)search.found;
    return true;
}

HWND AutoHideBar(HMONITOR screen, UINT edge) {
    for (auto const& a : g_autoHide)
        if (a.screen == screen && a.edge == edge) return a.wnd;
    return nullptr;
}

bool SetAutoHideBar(HWND wnd, HMONITOR screen, UINT edge, bool on) {
    HWND owner = AutoHideBar(screen, edge);
    if (on) {
        if (owner && owner != wnd) return false;  // (one per edge)
        if (!owner) g_autoHide.push_back({screen, edge, wnd});
        return true;
    }
    std::erase_if(g_autoHide, [&](AutoHide const& a) { return a.screen == screen && a.edge == edge && a.wnd == wnd; });
    return true;
}

// A program filling its whole screen, in front: bars on that screen are told,
// so they can keep out of its way.
bool FillsScreen(HWND w) {
    if (!w || !IsWindowVisible(w) || IsIconic(w)) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (pid == GetCurrentProcessId()) return false;  // (Tourne's own menus and previews)
    wchar_t cls[64] = L"";
    GetClassNameW(w, cls, 64);
    for (const wchar_t* desktop : {L"Progman", L"WorkerW"})
        if (!wcscmp(cls, desktop)) return false;
    RECT rc;
    MONITORINFO mi{sizeof(mi)};
    if (!GetWindowRect(w, &rc) || !GetMonitorInfoW(MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST), &mi)) return false;
    if (rc.left > mi.rcMonitor.left || rc.top > mi.rcMonitor.top || rc.right < mi.rcMonitor.right || rc.bottom < mi.rcMonitor.bottom)
        return false;
    // (a window with a title bar just bigger than its screen isn't full screen)
    return EqualRect(&rc, &mi.rcMonitor) || !(GetWindowLongW(w, GWL_STYLE) & WS_CAPTION);
}

void CheckFullscreen() {
    HWND front = GetForegroundWindow();
    HMONITOR now = FillsScreen(front) ? MonitorFromWindow(front, MONITOR_DEFAULTTONEAREST) : nullptr;
    if (now == g_fullscreen) return;
    Log(now ? L"%s fills its screen" : L"no program fills a screen", now ? ProgramOf(front).c_str() : L"");
    if (g_fullscreen) Tell(nullptr, ABN_FULLSCREENAPP, FALSE, g_fullscreen);
    if (now) Tell(nullptr, ABN_FULLSCREENAPP, TRUE, now);
    g_fullscreen = now;
}

void CALLBACK OnMove(HWINEVENTHOOK, DWORD, HWND w, LONG object, LONG, DWORD, DWORD) {
    if (object == OBJID_WINDOW && w == GetForegroundWindow()) CheckFullscreen();
}

// Moves are watched in the program in front only, so the rest cost nothing.
void CALLBACK OnFront(HWINEVENTHOOK, DWORD, HWND w, LONG, LONG, DWORD, DWORD) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (pid != g_movePid) {
        if (g_moveHook) UnhookWinEvent(g_moveHook);
        g_moveHook = pid ? SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr, OnMove, pid, 0,
                                           WINEVENT_OUTOFCONTEXT)
                         : nullptr;
        g_movePid = pid;
    }
    CheckFullscreen();
}

}  // namespace

LRESULT AppBarsMessage(const COPYDATASTRUCT* data) {
    if (data->cbData != sizeof(AppBarRequest)) {
        static bool told;
        if (!told) Log(L"an app bar message of a size not known here (%lu bytes)", data->cbData);
        told = true;
        return 0;
    }
    AppBarRequest r;
    memcpy(&r, data->lpData, sizeof(r));
    HWND wnd = (HWND)(ULONG_PTR)r.abd.hWnd;
    Prune();
    switch (r.dwMessage) {
        case ABM_NEW: {
            if (!IsWindow(wnd) || Find(wnd)) return FALSE;
            g_bars.push_back({wnd, r.abd.uCallbackMessage});
            Log(L"%s is an app bar", ProgramOf(wnd).c_str());
            return TRUE;
        }
        case ABM_REMOVE: {
            if (!Find(wnd)) return FALSE;
            std::erase_if(g_bars, [&](Bar const& b) { return b.wnd == wnd; });
            std::erase_if(g_autoHide, [&](AutoHide const& a) { return a.wnd == wnd; });
            Log(L"%s is no longer an app bar", ProgramOf(wnd).c_str());
            WorkAreas();
            Tell(nullptr, ABN_POSCHANGED, 0);
            return TRUE;
        }
        case ABM_QUERYPOS:
        case ABM_SETPOS: {
            Bar* bar = Find(wnd);
            if (!bar) return FALSE;
            bool set = r.dwMessage == ABM_SETPOS, moved = false;
            auto place = [&](AppBarData& d) {
                RECT rc = d.rc;
                UINT edge = d.uEdge <= ABE_BOTTOM ? d.uEdge : ABE_BOTTOM;
                Fit(bar, edge, rc);
                d.rc = rc;
                if (!set) return;
                moved = !bar->placed || bar->edge != edge || !EqualRect(&bar->rc, &rc);
                bar->edge = edge;
                bar->rc = rc;
                bar->placed = true;
            };
            if (!WithShared(r, place)) place(r.abd);  // (no answer reaches it then, but the space is kept)
            if (moved) {
                Log(L"%s on the %s edge: %ld %ld %ld %ld", ProgramOf(wnd).c_str(), EdgeName(bar->edge), bar->rc.left,
                    bar->rc.top, bar->rc.right, bar->rc.bottom);
                WorkAreas();
                Tell(wnd, ABN_POSCHANGED, 0);
            }
            return TRUE;
        }
        case ABM_GETSTATE:
            return g_state;
        case ABM_SETSTATE:
            g_state = (UINT)r.abd.lParam & (ABS_AUTOHIDE | ABS_ALWAYSONTOP);
            Tell(nullptr, ABN_STATECHANGE, 0);
            return TRUE;
        case ABM_GETTASKBARPOS: {
            bool found = false;
            WithShared(r, [&](AppBarData& d) { found = TaskbarPos(d); });
            return found;
        }
        case ABM_ACTIVATE:
        case ABM_WINDOWPOSCHANGED:
            return TRUE;
        case ABM_GETAUTOHIDEBAR:
            return (LRESULT)AutoHideBar(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), r.abd.uEdge);
        case kGetAutoHideBarEx:
            return (LRESULT)AutoHideBar(ScreenOf(r.abd.rc), r.abd.uEdge);
        case ABM_SETAUTOHIDEBAR:
            return SetAutoHideBar(wnd, MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), r.abd.uEdge, r.abd.lParam != 0);
        case kSetAutoHideBarEx:
            return SetAutoHideBar(wnd, ScreenOf(r.abd.rc), r.abd.uEdge, r.abd.lParam != 0);
    }
    return 0;
}

void AppBarsScreensChanged() {
    Prune();
    WorkAreas();
}

void AppBarsStart() {
    // SHLockShared: shell32's by name, else shlwapi's (by number only on some).
    HMODULE shell = LoadLibraryW(L"shell32.dll"), lightweight = LoadLibraryW(L"shlwapi.dll");
    g_lock = shell ? (LockShared)(void*)GetProcAddress(shell, "SHLockShared") : nullptr;
    g_unlock = shell ? (UnlockShared)(void*)GetProcAddress(shell, "SHUnlockShared") : nullptr;
    if ((!g_lock || !g_unlock) && lightweight) {
        g_lock = (LockShared)(void*)GetProcAddress(lightweight, MAKEINTRESOURCEA(8));
        g_unlock = (UnlockShared)(void*)GetProcAddress(lightweight, MAKEINTRESOURCEA(9));
    }
    if (!g_lock || !g_unlock) Log(L"SHLockShared not found: app bars are kept but not answered");
    g_frontHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, OnFront, 0, 0, WINEVENT_OUTOFCONTEXT);
    OnFront(nullptr, 0, GetForegroundWindow(), 0, 0, 0, 0);
}

void AppBarsStop() {
    if (g_frontHook) UnhookWinEvent(g_frontHook);
    if (g_moveHook) UnhookWinEvent(g_moveHook);
    g_frontHook = g_moveHook = nullptr;
    g_movePid = 0;
    g_fullscreen = nullptr;
    g_bars.clear();
    g_autoHide.clear();
    // Whole screens again, so nothing is left reserved for bars that are gone.
    if (g_setWorkArea) WorkAreas();
    g_setWorkArea = false;
}

}  // namespace td
