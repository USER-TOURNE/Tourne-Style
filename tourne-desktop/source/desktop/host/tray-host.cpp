#include "tray-host.h"

#include <shellapi.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <string>
#include <vector>

#include "app-bars.h"
#include "shell-mode.h"
#include "td.h"

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"tray", format, args);
    va_end(args);
}

// What Shell_NotifyIcon sends the tray (WM_COPYDATA, dwData 1): the same on
// 32 and 64-bit Windows, with handles as 32 bits (they fit).
#pragma pack(push, 1)
struct NotifyData {
    DWORD cbSize;
    DWORD hWnd;
    DWORD uID;
    DWORD uFlags;
    DWORD uCallbackMessage;
    DWORD hIcon;
    wchar_t szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    wchar_t szInfo[256];
    DWORD uVersion;  // (uTimeout too)
    wchar_t szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    DWORD hBalloonIcon;
};
struct TrayData {
    DWORD signature;  // 0x34753423
    DWORD message;    // NIM_*
    NotifyData nid;
};
#pragma pack(pop)

constexpr ULONG_PTR kAppBarCopyId = 0;  // (app-bars.cpp)
constexpr ULONG_PTR kTrayCopyId = 1;
constexpr ULONG_PTR kIconRectCopyId = 3;
constexpr ULONG_PTR kHiddenIconsCopyId = 0x54484931;  // (tray-shared.cpp: the packet the bar reads)
constexpr UINT_PTR kTimerChanged = 1;
constexpr LPARAM kOurBroadcast = 0x54524E45;  // "TaskbarCreated" from here, not from Explorer

struct Icon {
    HWND owner = nullptr;
    UINT id = 0;
    GUID guid{};
    bool byGuid = false;
    UINT callback = 0;
    UINT version = 0;
    UINT state = 0;
    HICON icon = nullptr;  // (our copy)
    std::wstring tip;
};

HWND g_tray, g_notify;
std::vector<Icon> g_icons;
UINT g_msgOverflow, g_msgTrayChanged, g_msgTaskbarCreated;
UINT g_msgExplorerEnded = RegisterWindowMessageW(L"TourneDesktop_ExplorerUnderneathEnded");

Icon* Find(const NotifyData& n, bool byGuid) {
    for (auto& i : g_icons)
        if (byGuid ? (i.byGuid && IsEqualGUID(i.guid, n.guidItem)) : (!i.byGuid && i.owner == (HWND)(ULONG_PTR)n.hWnd && i.id == n.uID))
            return &i;
    return nullptr;
}

std::wstring Text(const wchar_t* s, size_t max) { return std::wstring(s, wcsnlen(s, max)); }

void Changed() {
    if (g_tray) SetTimer(g_tray, kTimerChanged, 60, nullptr);  // (a burst of changes: once)
}

// The program's own small icon, for when the icon an app sent can't be
// copied (it was already destroyed, or its handle isn't valid here).
HICON ProgramIcon(HWND owner) {
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    HANDLE process = pid ? OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) : nullptr;
    if (!process) return nullptr;
    wchar_t path[MAX_PATH * 2];
    DWORD length = MAX_PATH * 2;
    HICON small = nullptr;
    if (QueryFullProcessImageNameW(process, 0, path, &length)) ExtractIconExW(path, 0, nullptr, &small, 1);
    CloseHandle(process);
    return small;
}

// A name for an icon whose program sent no tooltip (Claude's, among others): the
// program's own name for itself (its file description), else its file name.
std::wstring ProgramName(HWND owner) {
    static std::vector<std::pair<std::wstring, std::wstring>> known;  // path, name
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    HANDLE process = pid ? OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) : nullptr;
    if (!process) return L"";
    wchar_t path[MAX_PATH * 2];
    DWORD length = MAX_PATH * 2;
    bool found = QueryFullProcessImageNameW(process, 0, path, &length);
    CloseHandle(process);
    if (!found) return L"";
    for (auto const& [file, name] : known)
        if (file == path) return name;
    std::wstring name;
    DWORD handle = 0, bytes = GetFileVersionInfoSizeW(path, &handle);
    std::vector<BYTE> data(bytes);
    struct Translation { WORD language, codePage; }* translations = nullptr;
    UINT size = 0;
    if (bytes && GetFileVersionInfoW(path, 0, bytes, data.data()) &&
        VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (void**)&translations, &size) && size) {
        wchar_t query[64];
        swprintf(query, 64, L"\\StringFileInfo\\%04x%04x\\FileDescription", translations[0].language, translations[0].codePage);
        wchar_t* value = nullptr;
        if (VerQueryValueW(data.data(), query, (void**)&value, &size) && size > 1) name = value;
    }
    while (!name.empty() && iswspace(name.back())) name.pop_back();
    if (name.empty()) {
        std::wstring file = path;
        file = file.substr(file.find_last_of(L"\\/") + 1);
        if (file.size() > 4 && !_wcsicmp(file.c_str() + file.size() - 4, L".exe")) file.resize(file.size() - 4);
        name = file;
    }
    known.emplace_back(path, name);
    return name;
}

void Apply(Icon& i, const NotifyData& n) {
    if (n.uFlags & NIF_MESSAGE) i.callback = n.uCallbackMessage;
    if (n.uFlags & NIF_ICON) {
        if (i.icon) DestroyIcon(i.icon);
        i.icon = n.hIcon ? CopyIcon((HICON)(ULONG_PTR)n.hIcon) : nullptr;
        if (n.hIcon && !i.icon) i.icon = ProgramIcon(i.owner);
    }
    if (n.uFlags & NIF_TIP) i.tip = Text(n.szTip, 128);
    if (n.uFlags & NIF_STATE) i.state = (i.state & ~n.dwStateMask) | (n.dwState & n.dwStateMask);
    if (n.uFlags & NIF_INFO) {
        std::wstring title = Text(n.szInfoTitle, 64), info = Text(n.szInfo, 256);
        if (!info.empty()) Log(L"balloon from %s: %s %s", i.tip.c_str(), title.c_str(), info.c_str());  // (shown by Tourne later)
    }
}

void Remove(std::vector<Icon>::iterator it) {
    if (it->icon) DestroyIcon(it->icon);
    g_icons.erase(it);
}

LRESULT OnNotify(const TrayData& d) {
    const NotifyData& n = d.nid;
    bool byGuid = (n.uFlags & NIF_GUID) != 0;
    Icon* i = Find(n, byGuid);
    switch (d.message) {
        case NIM_ADD: {
            if (i) return FALSE;  // (as Explorer: already there)
            Icon fresh;
            fresh.owner = (HWND)(ULONG_PTR)n.hWnd;
            fresh.id = n.uID;
            fresh.byGuid = byGuid;
            fresh.guid = n.guidItem;
            Apply(fresh, n);
            g_icons.push_back(std::move(fresh));
            Changed();
            return TRUE;
        }
        case NIM_MODIFY:
            if (!i) return FALSE;
            if (!byGuid && n.hWnd) i->owner = (HWND)(ULONG_PTR)n.hWnd;
            Apply(*i, n);
            Changed();
            return TRUE;
        case NIM_DELETE:
            if (!i) return FALSE;
            Remove(g_icons.begin() + (i - g_icons.data()));
            Changed();
            return TRUE;
        case NIM_SETVERSION:
            if (!i) return FALSE;
            i->version = n.uVersion;
            return TRUE;
        case NIM_SETFOCUS: return i ? TRUE : FALSE;
    }
    return FALSE;
}

// The icon as premultiplied pixels: drawn on black and on white, so icons
// without an alpha channel come out right too.
std::vector<uint32_t> Pixels(HICON icon, int size) {
    std::vector<uint32_t> out;
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = size;
    bi.bmiHeader.biHeight = -size;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    void *black = nullptr, *white = nullptr;
    HBITMAP onBlack = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &black, nullptr, 0);
    HBITMAP onWhite = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &white, nullptr, 0);
    if (onBlack && onWhite) {
        memset(black, 0x00, (size_t)size * size * 4);
        memset(white, 0xFF, (size_t)size * size * 4);
        HDC dc = CreateCompatibleDC(nullptr);
        HGDIOBJ old = SelectObject(dc, onBlack);
        DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        SelectObject(dc, onWhite);
        DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        SelectObject(dc, old);
        DeleteDC(dc);
        GdiFlush();
        auto* b = (const uint32_t*)black;
        auto* w = (const uint32_t*)white;
        out.resize((size_t)size * size);
        for (int p = 0; p < size * size; p++) {
            int seen = 0;
            for (int shift = 0; shift < 24; shift += 8) seen = std::max(seen, (int)((w[p] >> shift) & 0xFF) - (int)((b[p] >> shift) & 0xFF));
            uint32_t alpha = 255 - std::clamp(seen, 0, 255);
            out[p] = alpha ? (alpha << 24) | (b[p] & 0xFFFFFF) : 0;
        }
    }
    if (onBlack) DeleteObject(onBlack);
    if (onWhite) DeleteObject(onWhite);
    return out;
}

// The packet the bar reads (tray-shared.cpp, UnpackHiddenIcons).
std::vector<BYTE> Packet() {
    std::vector<BYTE> packet(4);
    auto put = [&](const void* data, size_t size) { packet.insert(packet.end(), (const BYTE*)data, (const BYTE*)data + size); };
    UINT count = 0;
    int size = GetSystemMetrics(SM_CXSMICON);
    for (auto it = g_icons.begin(); it != g_icons.end();) {
        if (!IsWindow(it->owner)) {  // its program ended without taking it out
            Remove(it);
            continue;
        }
        Icon const& i = *it++;
        if ((i.state & NIS_HIDDEN) || !i.callback) continue;
        // In System Icons' design when it has one (in Explorer the suite swaps them as they arrive;
        // here the tray is ours, so they're swapped here, each time, as tooltips change).
        HICON shown = TrayDesign(i.owner, i.tip);
        if (!shown) shown = i.icon;
        std::vector<uint32_t> pixels = shown ? Pixels(shown, size) : std::vector<uint32_t>();
        uint64_t owner = (uint64_t)(ULONG_PTR)i.owner;
        std::wstring tip = i.tip.empty() ? ProgramName(i.owner) : i.tip;
        UINT flags = 0, iconSize = pixels.empty() ? 0 : size, chars = (UINT)tip.size();
        put(&owner, 8);
        put(&i.id, 4);
        put(&i.callback, 4);
        put(&i.version, 4);
        put(&flags, 4);
        put(&iconSize, 4);
        put(&chars, 4);
        put(tip.data(), chars * sizeof(wchar_t));
        put(pixels.data(), pixels.size() * 4);
        count++;
    }
    memcpy(packet.data(), &count, 4);
    return packet;
}

// A Tourne bar: Tourne Desktop's own, or the Windhawk suite's (its taskbar
// runs in Windhawk's tool process, and in shell mode asks this tray too).
bool IsTourneBar(HWND w) {
    wchar_t name[64];
    return GetClassNameW(w, name, 64) && wcsncmp(name, L"TourneTaskbarTest_", 18) == 0;
}

// Who may ask for the icons: a bar, or the system icons (their overflow flyout
// lists the icons placed in the overflow; turned away, it showed none, and an
// icon moved there was gone from both).
bool IsTourneAsker(HWND w) {
    wchar_t name[64];
    return IsTourneBar(w) || (GetClassNameW(w, name, 64) && wcsncmp(name, L"TourneTraySystemIcons_", 22) == 0);
}

// Where an icon is (Shell_NotifyIconGetRect, WM_COPYDATA kIconRectCopyId): a signature, then 1 for its left and
// top or 2 for its width and height, answered packed low and high (MAKELONG; measured: shell32 adds the two), then the icon. Answered with a small
// box at the pointer, where the click on it just was: answering 0 (failed) made Tauri's tray (Handy) ignore every
// click, as it asks before acting on one.
LRESULT IconRect(const COPYDATASTRUCT* cds) {
    if (cds->cbData < sizeof(DWORD) * 2) return 0;
    DWORD which = 0;
    memcpy(&which, (const BYTE*)cds->lpData + sizeof(DWORD), sizeof(DWORD));
    POINT pt;
    if (!GetCursorPos(&pt)) return 0;
    constexpr int kHalf = 12;
    if (which == 1) return (LRESULT)(DWORD)MAKELONG((WORD)(SHORT)(pt.x - kHalf), (WORD)(SHORT)(pt.y - kHalf));
    if (which == 2) return (LRESULT)(DWORD)MAKELONG(2 * kHalf, 2 * kHalf);
    return 0;
}

LRESULT CALLBACK TrayProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_COPYDATA) {
        auto* cds = (const COPYDATASTRUCT*)lp;
        if (!cds || !cds->lpData) return FALSE;
        if (cds->dwData == kTrayCopyId && cds->cbData >= sizeof(DWORD) * 2 + offsetof(NotifyData, szTip)) {
            TrayData d{};
            memcpy(&d, cds->lpData, std::min<size_t>(cds->cbData, sizeof(d)));
            if (d.nid.cbSize < offsetof(NotifyData, guidItem) + sizeof(GUID)) d.nid.uFlags &= ~NIF_GUID;  // (an old app's short record)
            return OnNotify(d);
        }
        if (cds->dwData == kAppBarCopyId) return AppBarsMessage(cds);
        if (cds->dwData == kIconRectCopyId) return IconRect(cds);
        return FALSE;
    }
    if (g_msgOverflow && msg == g_msgOverflow) {  // a bar or the system icons ask for the icons, as they ask the relay
        HWND bar = (HWND)wp;
        if (!IsWindow(bar) || !IsTourneAsker(bar)) return 0;
        std::vector<BYTE> packet = Packet();
        static UINT told = ~0u;
        if (UINT count = *(const UINT*)packet.data(); count != told && IsTourneBar(bar)) Log(L"%u icons in the bar's tray", told = count);
        COPYDATASTRUCT copy{kHiddenIconsCopyId, (DWORD)packet.size(), packet.data()};
        DWORD_PTR sent = 0;
        SendMessageTimeoutW(bar, WM_COPYDATA, (WPARAM)wnd, (LPARAM)&copy, SMTO_ABORTIFHUNG, 2000, &sent);
        return 1;
    }
    if (g_msgTaskbarCreated && msg == g_msgTaskbarCreated && lp != kOurBroadcast) {  // Explorer (underneath) started again
        ShellExplorerRestarted();
        return 0;
    }
    if (g_msgExplorerEnded && msg == g_msgExplorerEnded) {
        ShellRelaunchExplorer();  // (this window is made again after)
        return 0;
    }
    if (msg == WM_DISPLAYCHANGE) {
        AppBarsScreensChanged();
        return 0;
    }
    if (msg == WM_TIMER && wp == kTimerChanged) {
        KillTimer(wnd, kTimerChanged);
        EnumWindows(
            [](HWND w, LPARAM) -> BOOL {
                if (IsTourneBar(w)) PostMessageW(w, g_msgTrayChanged, 0, 0);
                return TRUE;
            },
            0);
        return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

}  // namespace

void TrayHostStart() {
    if (g_tray) return;
    // (TOURNE_TEST_TRAY: hosted beside another, for testing under Wine, whose desktop always has one)
    if (FindWindowW(L"Shell_TrayWnd", nullptr) && !ShellExplorerExpected() && !GetEnvironmentVariableW(L"TOURNE_TEST_TRAY", nullptr, 0)) {
        Log(L"there's a tray already (Explorer's?): not hosting one");
        return;
    }
    g_msgOverflow = RegisterWindowMessageW(L"TourneSuite_RelayOverflow");
    g_msgTrayChanged = RegisterWindowMessageW(L"TourneSuite_TrayChanged");
    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = TrayProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"Shell_TrayWnd";
    RegisterClassW(&wc);
    WNDCLASSW child{};
    child.lpfnWndProc = DefWindowProcW;
    child.hInstance = instance;
    child.lpszClassName = L"TrayNotifyWnd";
    RegisterClassW(&child);
    // No size and never shown: apps only need to find it.
    g_tray = CreateWindowExW(WS_EX_TOOLWINDOW | (ShellUnderneath() ? WS_EX_TOPMOST : 0), L"Shell_TrayWnd", L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    if (!g_tray) {
        Log(L"couldn't make the tray window (error %lu)", GetLastError());
        return;
    }
    g_notify = CreateWindowExW(0, L"TrayNotifyWnd", L"", WS_CHILD, 0, 0, 0, 0, g_tray, nullptr, instance, nullptr);
    // Apps with fewer rights than Tourne Desktop reach it too.
    ChangeWindowMessageFilterEx(g_tray, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
    g_msgTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    ChangeWindowMessageFilterEx(g_tray, g_msgTaskbarCreated, MSGFLT_ALLOW, nullptr);
    // Apps already running put their icons in again.
    TrayHostRetake();
    AppBarsStart();
    // Minimized windows: Explorer has Windows hide them (the taskbar shows
    // them), and without it they sit as little title bars at the bottom left.
    MINIMIZEDMETRICS mm{sizeof(mm)};
    if (SystemParametersInfoW(SPI_GETMINIMIZEDMETRICS, sizeof(mm), &mm, 0) && !(mm.iArrange & ARW_HIDE)) {
        mm.iArrange |= ARW_HIDE;
        SystemParametersInfoW(SPI_SETMINIMIZEDMETRICS, sizeof(mm), &mm, 0);
    }
    // (ones minimized before that keep their place: they go where Windows now puts them)
    EnumWindows(
        [](HWND w, LPARAM) -> BOOL {
            RECT rc;
            if (IsIconic(w) && IsWindowVisible(w) && GetWindowRect(w, &rc) && rc.left > -30000)
                SetWindowPos(w, nullptr, -32000, -32000, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
            return TRUE;
        },
        0);
    Log(L"hosting the tray");
}

void TrayHostRaise() {
    if (g_tray && ShellUnderneath())
        SetWindowPos(g_tray, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void TrayHostRetake() {
    if (!g_tray) return;
    TrayHostRaise();
    PostMessageW(HWND_BROADCAST, g_msgTaskbarCreated, 0, kOurBroadcast);
}

void TrayHostExplorerEnded() {
    if (g_tray) PostMessageW(g_tray, g_msgExplorerEnded, 0, 0);
}

void TrayHostStop() {
    if (!g_tray) return;
    AppBarsStop();
    for (auto& i : g_icons)
        if (i.icon) DestroyIcon(i.icon);
    g_icons.clear();
    DestroyWindow(g_tray);
    g_tray = g_notify = nullptr;
}

}  // namespace td
