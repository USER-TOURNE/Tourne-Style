// The flyouts: see flyouts.h.
//
// One thread holds a hidden popup. The default speakers' volume tells it when it changes (IAudioEndpointVolume's
// callback, moved to the new default device when that changes); the keyboard comes as raw input (Caps, Num and
// Scroll Lock, the media keys). Each shows the popup for a moment: a glyph, words, and for the volume a bar,
// inside the menus' frame (TourneMenuPaintFrame) in their colours and font.

#include "flyouts.h"

#include <dwmapi.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <cmath>
#include <cstdarg>
#include <string>
#include <thread>
#include <vector>

#include "plugin-runtime.h"
#include "standalone.h"
#include "td.h"

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"flyouts", format, args);
    va_end(args);
}

constexpr UINT kMsgVolume = WM_APP + 1, kMsgDevice = WM_APP + 2, kMsgApply = WM_APP + 3, kMsgBalloon = WM_APP + 4,
               kMsgVolumeKey = WM_APP + 5;
constexpr UINT_PTR kTimerHide = 1, kTimerLock = 2, kTimerBalloon = 3;

struct Config {
    bool on = true, volume = true, media = true, locks = true, balloons = true, hideWindows = true;
    int balloonMs = 6000;
    int ms = 1500;
    int place = 0;  // 0 bottom middle (over the bar), 1 top middle, 2 middle of the screen
};

Config ReadConfig() {
    Config c;
    c.on = GetInt(L"flyouts", L"enabled") != 0;
    c.volume = GetInt(L"flyouts", L"volume") != 0;
    c.media = GetInt(L"flyouts", L"media") != 0;
    c.locks = GetInt(L"flyouts", L"lockKeys") != 0;
    c.balloons = GetInt(L"flyouts", L"balloons") != 0;
    c.hideWindows = GetInt(L"flyouts", L"hideWindows") != 0;
    c.balloonMs = std::clamp(GetInt(L"flyouts", L"balloonMs"), 1000, 60000);
    c.ms = std::clamp(GetInt(L"flyouts", L"ms"), 300, 10000);
    if (const wchar_t* v = GetString(L"flyouts", L"place")) {
        std::wstring p = v;
        FreeString(v);
        c.place = p == L"top" ? 1 : p == L"middle" ? 2 : 0;
    }
    return c;
}

// The menus' look (taskbar-host.cpp's TourneMenuLookFor; the layout must match its TourneMenuLookC).
struct MenuLookC {
    uint32_t ground, line, edge, text, accent, dim, hover, hoverText, band;
    HFONT font;
    int scale, frame, caps;
    uint32_t frameLine;
};

COLORREF Rgb(uint32_t c) { return RGB(c >> 16 & 255, c >> 8 & 255, c & 255); }

Config g_cfg;
HWND g_wnd = nullptr;
std::thread g_thread;
std::atomic<bool> g_quit{false};

// What's shown now.
std::wstring g_glyph, g_title, g_detail;
int g_level = -1;  // the bar, 0 to 100 (-1: none)
bool g_off = false;  // the bar's muted look

////////////////////////////////////////////////////////////////////////////////
// The volume

struct VolumeCallback : IAudioEndpointVolumeCallback {
    LONG refs = 1;
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&refs); }
    ULONG STDMETHODCALLTYPE Release() override { return InterlockedDecrement(&refs); }  // (one, kept for the thread's life)
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** out) override {
        if (id == __uuidof(IUnknown) || id == __uuidof(IAudioEndpointVolumeCallback)) return *out = this, S_OK;
        return *out = nullptr, E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA d) override {
        if (g_wnd && d) PostMessageW(g_wnd, kMsgVolume, (WPARAM)std::lround(d->fMasterVolume * 100), d->bMuted);
        return S_OK;
    }
};

struct DeviceCallback : IMMNotificationClient {
    LONG refs = 1;
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&refs); }
    ULONG STDMETHODCALLTYPE Release() override { return InterlockedDecrement(&refs); }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id, void** out) override {
        if (id == __uuidof(IUnknown) || id == __uuidof(IMMNotificationClient)) return *out = this, S_OK;
        return *out = nullptr, E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) override {
        if (flow == eRender && role == eConsole && g_wnd) PostMessageW(g_wnd, kMsgDevice, 0, 0);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override { return S_OK; }
};

VolumeCallback g_volumeCallback;
DeviceCallback g_deviceCallback;
IMMDeviceEnumerator* g_devices = nullptr;
IAudioEndpointVolume* g_volume = nullptr;

void ListenToSpeakers() {
    if (g_volume) {
        g_volume->UnregisterControlChangeNotify(&g_volumeCallback);
        g_volume->Release();
        g_volume = nullptr;
    }
    IMMDevice* d = nullptr;
    if (g_devices && SUCCEEDED(g_devices->GetDefaultAudioEndpoint(eRender, eConsole, &d))) {
        if (SUCCEEDED(d->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&g_volume)))
            g_volume->RegisterControlChangeNotify(&g_volumeCallback);
        d->Release();
    }
}

// The volume keys taken here (with "Hide Windows' own"): Explorer never sees them, so its pop-up never opens (hiding
// it as it shows left it flashing), and the volume is stepped as Windows steps it. On a thread of its own that
// does nothing else, the hook taken again each minute, as Capture's is (Windows drops a slow hook without a word).
HHOOK g_keyHook = nullptr;
std::atomic<DWORD> g_keyThread{0};

LRESULT CALLBACK VolumeKeyProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        auto* k = (KBDLLHOOKSTRUCT*)lp;
        bool volumeKey = k->vkCode == VK_VOLUME_UP || k->vkCode == VK_VOLUME_DOWN || k->vkCode == VK_VOLUME_MUTE;
        // (sent ones too: a knob's software, or a keyboard's own, sends them as keys; nothing here sends them)
        if (volumeKey && g_cfg.on && g_cfg.volume && g_cfg.hideWindows && g_volume && g_wnd) {
            if (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN) PostMessageW(g_wnd, kMsgVolumeKey, k->vkCode, (k->flags & LLKHF_INJECTED) ? 1 : 0);
            return 1;
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

void StartVolumeKeys() {
    std::thread([] {
        g_keyThread = GetCurrentThreadId();
        HINSTANCE inst = GetModuleHandleW(nullptr);
        g_keyHook = SetWindowsHookExW(WH_KEYBOARD_LL, VolumeKeyProc, inst, 0);
        SetTimer(nullptr, 0, 60000, nullptr);
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) {
            if (m.message != WM_TIMER) continue;
            if (HHOOK again = SetWindowsHookExW(WH_KEYBOARD_LL, VolumeKeyProc, inst, 0)) {
                if (g_keyHook) UnhookWindowsHookEx(g_keyHook);
                g_keyHook = again;
            }
        }
        if (g_keyHook) UnhookWindowsHookEx(g_keyHook);
        g_keyHook = nullptr;
    }).detach();
}

void StopVolumeKeys() {
    if (g_keyThread) PostThreadMessageW(g_keyThread, WM_QUIT, 0, 0);
    g_keyThread = 0;
}

// A volume key, as Windows does it: a step up or down (and out of mute), or mute turned over.
void VolumeKey(UINT vk) {
    if (!g_volume) return;
    BOOL muted = FALSE;
    g_volume->GetMute(&muted);
    if (vk == VK_VOLUME_MUTE) {
        g_volume->SetMute(!muted, nullptr);
        return;
    }
    if (muted) g_volume->SetMute(FALSE, nullptr);
    if (vk == VK_VOLUME_UP) g_volume->VolumeStepUp(nullptr);
    else g_volume->VolumeStepDown(nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// The popup

int S(int v) { return MulDiv(v, g_wnd ? GetDpiForWindow(g_wnd) : 96, 96); }

// The tray's design for a flyout's glyph (system-icons-host.cpp's ArtFor), or "".
std::wstring DesignFor(std::wstring const& glyph, int level) {
    if (glyph == L"volume-off") return L"audio-mute";
    if (glyph.rfind(L"volume", 0) == 0) return level <= 0 ? L"audio-0" : level < 34 ? L"audio-low" : level < 67 ? L"audio-mid" : L"audio-high";
    if (glyph == L"player-play") return L"media-playpause";
    if (glyph == L"player-track-next") return L"media-next";
    if (glyph == L"player-track-prev") return L"media-prev";
    if (glyph == L"player-stop") return L"media-stop";
    if (glyph == L"lock" || glyph == L"lock-open" || glyph == L"123" || glyph.rfind(L"letter", 0) == 0 || glyph.rfind(L"abc", 0) == 0)
        return L"keyboard";
    return L"";
}

void Paint(HDC dc) {
    RECT rc;
    GetClientRect(g_wnd, &rc);
    MenuLookC look{0x121C21, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x5D8D6F, 0x1F3D34, 0xFFFFFFFF, 0x151F24, nullptr, 1, 0, 0, 0xFFFFFFFF};
    using LookFn = BOOL (*)(MenuLookC*, int);
    using FrameFn = void (*)(HDC, int, int, RECT*);
    static auto lookFor = (LookFn)GetProcAddress(GetModuleHandleW(nullptr), "TourneMenuLookFor");
    static auto frame = (FrameFn)GetProcAddress(GetModuleHandleW(nullptr), "TourneMenuPaintFrame");
    if (lookFor) lookFor(&look, (int)sizeof(look));
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(mem, bmp);
    RECT in = {1, 1, rc.right - 1, rc.bottom - 1};
    if (frame) {
        frame(mem, rc.right, rc.bottom, &in);
    } else {
        SetDCBrushColor(mem, Rgb(look.ground));
        FillRect(mem, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
        SetDCBrushColor(mem, Rgb(look.edge));
        FrameRect(mem, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
    }
    SetBkMode(mem, TRANSPARENT);
    int pad = S(12);
    // The icon (the tray's own pixel designs, at a whole multiple of their 12 pixels so they stay sharp; the
    // icon font where there's no design), then the words beside it, the bar under them.
    HFONT glyphs = CreateFontW(-S(22), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"tabler-icons");
    HGDIOBJ oldFont = SelectObject(mem, glyphs);
    SetTextColor(mem, Rgb(look.accent));
    RECT g{in.left + pad, in.top, in.left + pad + S(28), in.bottom};
    std::wstring design = DesignFor(g_glyph, g_level);
    int size = std::max(12, S(24) / 12 * 12);
    HICON icon = design.empty() ? nullptr : part_system_icons::DesignIconAt(L"tourne:" + design, size);
    if (icon) {
        DrawIconEx(mem, (g.left + g.right - size) / 2, (g.top + g.bottom - size) / 2, icon, size, size, 0, nullptr, DI_NORMAL);
        DestroyIcon(icon);
    } else {
        std::wstring glyph = LauncherGlyph(g_glyph);
        DrawTextW(mem, glyph.c_str(), -1, &g, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    HFONT text = look.font ? look.font : (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SelectObject(mem, text);
    int x = g.right + S(10);
    bool bar = g_level >= 0;
    RECT t{x, in.top, in.right - pad, bar ? (in.top + in.bottom) / 2 + S(2) : in.bottom};
    std::wstring title = g_title;
    if (look.caps)
        for (auto& c : title) c = towupper(c);
    SetTextColor(mem, Rgb(look.text));
    DrawTextW(mem, title.c_str(), -1, &t, DT_LEFT | (bar ? DT_BOTTOM : DT_VCENTER) | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    if (!g_detail.empty()) {
        SetTextColor(mem, Rgb(look.dim));
        DrawTextW(mem, g_detail.c_str(), -1, &t, DT_RIGHT | (bar ? DT_BOTTOM : DT_VCENTER) | DT_SINGLELINE | DT_NOPREFIX);
    }
    if (bar) {
        // A thin track in the menus' line, filled with the accent (dim when muted), square as the menus are.
        int y = (in.top + in.bottom) / 2 + S(8), h = std::max(2, S(4));
        RECT track{x, y, in.right - pad, y + h};
        SetDCBrushColor(mem, Rgb(look.line));
        FillRect(mem, &track, (HBRUSH)GetStockObject(DC_BRUSH));
        RECT fill = track;
        fill.right = track.left + (track.right - track.left) * std::clamp(g_level, 0, 100) / 100;
        SetDCBrushColor(mem, Rgb(g_off ? look.dim : look.accent));
        FillRect(mem, &fill, (HBRUSH)GetStockObject(DC_BRUSH));
    }
    BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldFont);
    SelectObject(mem, oldBmp);
    DeleteObject(glyphs);
    DeleteObject(bmp);
    DeleteDC(mem);
}

// Where a screen's room ends at the bottom: above Tourne's bar there when it sits at the bottom (it doesn't always
// take the screen's work area), else the work area's end.
int AboveBar(MONITORINFO const& mi) {
    RECT const& work = mi.rcWork;
    int bottom = work.bottom;
    for (HWND bar = nullptr; (bar = FindWindowExW(nullptr, bar, L"TourneTaskbarTest_tourne-desktop", nullptr));) {
        RECT b;
        if (IsWindowVisible(bar) && GetWindowRect(bar, &b) && b.left < work.right && b.right > work.left &&
            b.top > (work.top + work.bottom) / 2 && b.top < bottom && b.bottom >= mi.rcMonitor.bottom - 2)
            bottom = b.top;
    }
    return bottom;
}

// Windows' own pop-ups for the same keys (its volume and media one, Explorer's or ShellHost's, which go on showing
// in shell mode): any small top-level window of those programs that shows within a moment of a volume or media
// key, or of ours, and isn't one of the shell's lasting windows, is hidden. Each class hidden is logged once, so
// what it was on this Windows can be read in the log.
DWORD g_keyAt = 0;  // a volume or media key, or our volume flyout (GetTickCount)

bool OsdProgram(HWND w) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    bool ours = false;
    if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        wchar_t path[MAX_PATH];
        DWORD n = MAX_PATH;
        if (QueryFullProcessImageNameW(p, 0, path, &n)) {
            const wchar_t* name = wcsrchr(path, L'\\') ? wcsrchr(path, L'\\') + 1 : path;
            ours = !_wcsicmp(name, L"explorer.exe") || !_wcsicmp(name, L"ShellHost.exe") || !_wcsicmp(name, L"ShellExperienceHost.exe");
        }
        CloseHandle(p);
    }
    return ours;
}

bool HideIfOsd(HWND w) {
    if (!w || GetAncestor(w, GA_ROOT) != w || !IsWindowVisible(w)) return false;
    wchar_t cls[128] = L"";
    GetClassNameW(w, cls, 128);
    std::wstring c = cls;
    if (c != L"NativeHWNDHost" && c != L"XamlExplorerHostIslandWindow" && c != L"Windows.UI.Core.CoreWindow" &&
        c != L"Shell_Flyout" && c.rfind(L"ShellHost", 0) != 0)
        return false;
    RECT r;
    if (!GetWindowRect(w, &r) || r.bottom - r.top > S(420) || r.right - r.left > S(900)) return false;  // (Start, a panel)
    if (!OsdProgram(w)) return false;
    ShowWindowAsync(w, SW_HIDE);
    static std::vector<std::wstring> logged;
    if (std::find(logged.begin(), logged.end(), c) == logged.end()) {
        logged.push_back(c);
        Log(L"Windows' own pop-up hidden: %ls, %ldx%ld", c.c_str(), r.right - r.left, r.bottom - r.top);
    }
    return true;
}

void CALLBACK OsdShown(HWINEVENTHOOK, DWORD, HWND w, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF) return;
    if (!g_cfg.on || !g_cfg.hideWindows || GetTickCount() - g_keyAt > 1500) return;
    HideIfOsd(w);
}

// One shown before the key reached us: looked for again as ours shows.
void HideShownOsd() {
    if (!g_cfg.on || !g_cfg.hideWindows) return;
    EnumWindows([](HWND w, LPARAM) -> BOOL { return HideIfOsd(w), TRUE; }, 0);
}

void Show(const std::wstring& glyph, const std::wstring& title, const std::wstring& detail, int level, bool off) {
    if (!g_cfg.on) return;
    g_glyph = glyph, g_title = title, g_detail = detail, g_level = level, g_off = off;
    // Over the screen the pointer is on, by the setting's place.
    POINT pt;
    GetCursorPos(&pt);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    RECT const& work = mi.rcWork;
    if (!IsWindowVisible(g_wnd)) SetWindowPos(g_wnd, nullptr, work.left, work.top, 10, 10, SWP_NOZORDER | SWP_NOACTIVATE);  // (its DPI)
    int w = S(280), h = S(60);
    int x = (work.left + work.right - w) / 2;
    int bottom = AboveBar(mi);
    int y = g_cfg.place == 1 ? work.top + S(24) : g_cfg.place == 2 ? (work.top + work.bottom - h) / 2 : bottom - h - S(16);
    SetWindowPos(g_wnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(g_wnd, nullptr, FALSE);
    SetTimer(g_wnd, kTimerHide, g_cfg.ms, nullptr);
    if (level >= 0 || glyph.rfind(L"player-", 0) == 0) {  // (volume and media: Windows has its own for those)
        g_keyAt = GetTickCount();
        HideShownOsd();
    }
}

// A lock key's state now: the window in front's view of it (whose thread the key went to).
bool Toggled(int vk) {
    DWORD front = GetWindowThreadProcessId(GetForegroundWindow(), nullptr), mine = GetCurrentThreadId();
    bool attached = front && front != mine && AttachThreadInput(mine, front, TRUE);
    bool on = (GetKeyState(vk) & 1) != 0;
    if (attached) AttachThreadInput(mine, front, FALSE);
    return on;
}

int g_lockVk = 0;

void OnKey(USHORT vk) {
    switch (vk) {
        case VK_CAPITAL:
        case VK_NUMLOCK:
        case VK_SCROLL:
            if (!g_cfg.locks) return;
            g_lockVk = vk;
            SetTimer(g_wnd, kTimerLock, 40, nullptr);  // (once the program in front has taken the key)
            return;
        case VK_VOLUME_UP:
        case VK_VOLUME_DOWN:
        case VK_VOLUME_MUTE:
            if (g_cfg.volume) g_keyAt = GetTickCount();  // (ours shows when the level changes)
            return;
        case VK_MEDIA_PLAY_PAUSE:
            if (g_cfg.media) Show(L"player-play", L"Play / pause", L"", -1, false);
            return;
        case VK_MEDIA_NEXT_TRACK:
            if (g_cfg.media) Show(L"player-track-next", L"Next track", L"", -1, false);
            return;
        case VK_MEDIA_PREV_TRACK:
            if (g_cfg.media) Show(L"player-track-prev", L"Previous track", L"", -1, false);
            return;
        case VK_MEDIA_STOP:
            if (g_cfg.media) Show(L"player-stop", L"Stop", L"", -1, false);
            return;
    }
}

void ShowBalloon(Balloon* incoming);  // (below)

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INPUT: {
            // A consumer-control device (a media knob or keys on their own, usage page 0x0C) that doesn't come as keys:
            // Windows' pop-up may follow it, so it's watched for as after a key; what came is logged once.
            RAWINPUTHEADER head{};
            UINT headSize = sizeof(head);
            if (GetRawInputData((HRAWINPUT)lp, RID_HEADER, &head, &headSize, sizeof(RAWINPUTHEADER)) != (UINT)-1 &&
                head.dwType == RIM_TYPEHID) {
                g_keyAt = GetTickCount();
                static bool told = false;
                if (!told) told = true, Log(L"a consumer-control device (a media knob or keys) seen");
                return DefWindowProcW(wnd, msg, wp, lp);
            }
            RAWINPUT raw{};
            UINT size = sizeof(raw);
            if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1 &&
                raw.header.dwType == RIM_TYPEKEYBOARD &&
                (raw.data.keyboard.Message == WM_KEYDOWN || raw.data.keyboard.Message == WM_SYSKEYDOWN))
                OnKey(raw.data.keyboard.VKey);
            return DefWindowProcW(wnd, msg, wp, lp);
        }
        case kMsgVolume:
            if (g_cfg.volume)
                Show(lp ? L"volume-off" : wp == 0 ? L"volume-3" : wp < 50 ? L"volume-2" : L"volume", lp ? L"Muted" : L"Volume",
                     std::to_wstring(wp), (int)wp, lp != 0);
            return 0;
        case kMsgDevice:
            ListenToSpeakers();
            return 0;
        case kMsgVolumeKey: {
            static bool told[2];
            if (!told[lp ? 1 : 0]) {
                told[lp ? 1 : 0] = true;
                Log(L"a volume key taken (%ls)", lp ? L"sent by a program" : L"from a keyboard");
            }
            g_keyAt = GetTickCount();
            VolumeKey((UINT)wp);
            return 0;
        }
        case kMsgBalloon:
            ShowBalloon((Balloon*)lp);
            return 0;
        case kMsgApply:
            g_cfg = ReadConfig();
            if (!g_cfg.on) ShowWindow(wnd, SW_HIDE);
            return 0;
        case WM_TIMER:
            if (wp == kTimerHide) {
                KillTimer(wnd, kTimerHide);
                ShowWindow(wnd, SW_HIDE);
            } else if (wp == kTimerLock) {
                KillTimer(wnd, kTimerLock);
                bool on = Toggled(g_lockVk);
                const wchar_t* name = g_lockVk == VK_CAPITAL ? L"Caps Lock" : g_lockVk == VK_NUMLOCK ? L"Num Lock" : L"Scroll Lock";
                const wchar_t* glyph = g_lockVk == VK_CAPITAL ? (on ? L"letter-case-upper" : L"letter-case")
                                     : g_lockVk == VK_NUMLOCK ? L"123" : (on ? L"lock" : L"lock-open");
                Show(glyph, name, on ? L"on" : L"off", -1, false);
            }
            return 0;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_NCHITTEST:
            return HTTRANSPARENT;  // (clicks go through to what's under it)
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            Paint(dc);
            EndPaint(wnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////
// Balloons: one at a time (a new one takes the place), over the corner by the tray.

HWND g_balloonWnd = nullptr;
Balloon g_balloon;

void TellBalloon(UINT what) {
    Balloon const& b = g_balloon;
    if (!b.owner || !b.callback || !IsWindow(b.owner)) return;
    if (b.version >= 4) {
        POINT pt;
        GetCursorPos(&pt);
        PostMessageW(b.owner, b.callback, MAKEWPARAM(pt.x, pt.y), MAKELPARAM(what, b.id));
    } else {
        PostMessageW(b.owner, b.callback, b.id, what);
    }
}

void EndBalloon(UINT why) {
    KillTimer(g_balloonWnd, kTimerBalloon);
    if (IsWindowVisible(g_balloonWnd)) {
        ShowWindow(g_balloonWnd, SW_HIDE);
        TellBalloon(why);
    }
    if (g_balloon.icon) DestroyIcon(g_balloon.icon);
    g_balloon = {};
}

void PaintBalloon(HDC dc) {
    RECT rc;
    GetClientRect(g_balloonWnd, &rc);
    MenuLookC look{0x121C21, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x5D8D6F, 0x1F3D34, 0xFFFFFFFF, 0x151F24, nullptr, 1, 0, 0, 0xFFFFFFFF};
    using LookFn = BOOL (*)(MenuLookC*, int);
    using FrameFn = void (*)(HDC, int, int, RECT*);
    static auto lookFor = (LookFn)GetProcAddress(GetModuleHandleW(nullptr), "TourneMenuLookFor");
    static auto frame = (FrameFn)GetProcAddress(GetModuleHandleW(nullptr), "TourneMenuPaintFrame");
    if (lookFor) lookFor(&look, (int)sizeof(look));
    int dpi = GetDpiForWindow(g_balloonWnd);
    auto px = [&](int v) { return MulDiv(v, dpi, 96); };
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(mem, bmp);
    RECT in = {1, 1, rc.right - 1, rc.bottom - 1};
    if (frame) {
        frame(mem, rc.right, rc.bottom, &in);
    } else {
        SetDCBrushColor(mem, Rgb(look.ground));
        FillRect(mem, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
        SetDCBrushColor(mem, Rgb(look.edge));
        FrameRect(mem, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
    }
    SetBkMode(mem, TRANSPARENT);
    int pad = px(12), icon = px(32);
    if (g_balloon.icon) DrawIconEx(mem, in.left + pad, in.top + pad, g_balloon.icon, icon, icon, 0, nullptr, DI_NORMAL);
    HFONT text = look.font ? look.font : (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HGDIOBJ oldFont = SelectObject(mem, text);
    int x = in.left + pad + icon + px(12);
    RECT app{x, in.top + pad - px(2), in.right - pad, in.top + pad + px(14)};
    SetTextColor(mem, Rgb(look.dim));
    DrawTextW(mem, g_balloon.app.c_str(), -1, &app, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    RECT title{x, app.bottom + px(2), in.right - pad, app.bottom + px(20)};
    SetTextColor(mem, Rgb(look.text));
    DrawTextW(mem, g_balloon.title.c_str(), -1, &title, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    RECT body{x, title.bottom + px(2), in.right - pad, in.bottom - pad};
    SetTextColor(mem, Rgb(look.dim));
    DrawTextW(mem, g_balloon.body.c_str(), -1, &body, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS | DT_EDITCONTROL);
    BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldFont);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
}

void ShowBalloon(Balloon* incoming) {
    std::unique_ptr<Balloon> b(incoming);
    if (!g_cfg.on || !g_cfg.balloons) {
        if (b->icon) DestroyIcon(b->icon);
        return;
    }
    EndBalloon(NIN_BALLOONHIDE);
    g_balloon = std::move(*b);
    // Over the corner by the tray: the main screen's bottom right, inside its work area.
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    RECT const& work = mi.rcWork;
    SetWindowPos(g_balloonWnd, nullptr, work.right - 20, work.bottom - 20, 10, 10, SWP_NOZORDER | SWP_NOACTIVATE);  // (its DPI)
    int dpi = GetDpiForWindow(g_balloonWnd);
    int w = MulDiv(360, dpi, 96), h = MulDiv(g_balloon.body.size() > 90 ? 112 : 92, dpi, 96), gap = MulDiv(12, dpi, 96);
    SetWindowPos(g_balloonWnd, HWND_TOPMOST, work.right - w - gap, AboveBar(mi) - h - gap, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(g_balloonWnd, nullptr, FALSE);
    SetTimer(g_balloonWnd, kTimerBalloon, g_cfg.balloonMs, nullptr);
    TellBalloon(NIN_BALLOONSHOW);
}

LRESULT CALLBACK BalloonProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_TIMER:
            if (wp == kTimerBalloon) {
                // (not while the pointer is on it: it waits)
                POINT pt;
                RECT r;
                GetCursorPos(&pt);
                GetWindowRect(wnd, &r);
                if (!PtInRect(&r, pt)) EndBalloon(NIN_BALLOONTIMEOUT);
            }
            return 0;
        case WM_LBUTTONUP: {
            Balloon const& b = g_balloon;
            DWORD pid = 0;
            if (b.owner) GetWindowThreadProcessId(b.owner, &pid);
            if (pid) AllowSetForegroundWindow(pid);  // (so the app can come to the front for it)
            KillTimer(wnd, kTimerBalloon);
            ShowWindow(wnd, SW_HIDE);
            TellBalloon(NIN_BALLOONUSERCLICK);
            if (g_balloon.icon) DestroyIcon(g_balloon.icon);
            g_balloon = {};
            return 0;
        }
        case WM_RBUTTONUP:
            EndBalloon(NIN_BALLOONHIDE);
            return 0;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_SETCURSOR:
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return TRUE;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PaintBalloon(dc);
            EndPaint(wnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void Thread() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);  // (the audio callbacks come on Windows' threads)
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HINSTANCE inst = GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.lpszClassName = L"TourneFlyout";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    g_wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"TourneFlyout", L"Tourne flyout", WS_POPUP, 0, 0,
                            10, 10, nullptr, nullptr, inst, nullptr);
    DWORD square = 1;  // DWMWCP_DONOTROUND: square, as the menus are
    DwmSetWindowAttribute(g_wnd, 33, &square, sizeof(square));
    WNDCLASSW bc{};
    bc.style = CS_DROPSHADOW;
    bc.lpfnWndProc = BalloonProc;
    bc.hInstance = inst;
    bc.lpszClassName = L"TourneBalloon";
    RegisterClassW(&bc);
    g_balloonWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"TourneBalloon", L"Tourne balloon", WS_POPUP, 0, 0,
                                   10, 10, nullptr, nullptr, inst, nullptr);
    DwmSetWindowAttribute(g_balloonWnd, 33, &square, sizeof(square));
    g_cfg = ReadConfig();
    StartVolumeKeys();
    HWINEVENTHOOK osd = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, OsdShown, 0, 0, WINEVENT_OUTOFCONTEXT);
    RAWINPUTDEVICE rid[] = {{0x01, 0x06, RIDEV_INPUTSINK, g_wnd},   // keyboards, while anything is in front
                            {0x0C, 0x01, RIDEV_INPUTSINK, g_wnd}};  // consumer controls (media knobs and keys)
    if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0]))) Log(L"couldn't listen to the keyboard (%lu)", GetLastError());
    if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&g_devices))) {
        g_devices->RegisterEndpointNotificationCallback(&g_deviceCallback);
        ListenToSpeakers();
    }
    Log(L"on: volume %ls, media keys %ls, lock keys %ls", g_cfg.volume ? L"shown" : L"not", g_cfg.media ? L"shown" : L"not",
        g_cfg.locks ? L"shown" : L"not");
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    if (osd) UnhookWinEvent(osd);
    StopVolumeKeys();
    if (g_volume) {
        g_volume->UnregisterControlChangeNotify(&g_volumeCallback);
        g_volume->Release();
        g_volume = nullptr;
    }
    if (g_devices) {
        g_devices->UnregisterEndpointNotificationCallback(&g_deviceCallback);
        g_devices->Release();
        g_devices = nullptr;
    }
    if (g_balloon.icon) DestroyIcon(g_balloon.icon);
    g_balloon = {};
    DestroyWindow(g_balloonWnd);
    g_balloonWnd = nullptr;
    HWND w = g_wnd;
    g_wnd = nullptr;
    DestroyWindow(w);
    CoUninitialize();
}

}  // namespace

void FlyoutsDefaults(Settings& out) {
    out[L"flyouts.enabled"] = L"1";
    out[L"flyouts.volume"] = L"1";
    out[L"flyouts.media"] = L"1";
    out[L"flyouts.lockKeys"] = L"1";
    out[L"flyouts.ms"] = L"1500";
    out[L"flyouts.place"] = L"bottom";
    out[L"flyouts.balloons"] = L"1";
    out[L"flyouts.balloonMs"] = L"6000";
    out[L"flyouts.hideWindows"] = L"1";
}

void FlyoutsDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"flyouts", L"Flyouts",
        L"What shows over the screen when the volume changes, a media key is pressed or Caps, Num or Scroll Lock is "
        L"turned, in your menus' look. In shell mode Explorer is hidden, so without these nothing shows.");
    add(L"flyouts.enabled", L"Flyouts", L"Off: nothing is shown (Windows' own don't come back while Explorer is hidden).").isSwitch = true;
    add(L"flyouts.volume", L"Volume", L"The level and mute, whatever changed them (keys, apps, the mixer).").isSwitch = true;
    add(L"flyouts.media", L"Media keys", L"Play, pause, next, previous and stop.").isSwitch = true;
    add(L"flyouts.lockKeys", L"Lock keys", L"Caps Lock, Num Lock and Scroll Lock, on or off.").isSwitch = true;
    add(L"flyouts.ms", L"Shown for", L"Milliseconds a flyout stays before it goes (300 to 10000).");
    add(L"flyouts.place", L"Place", L"bottom (over the bar, the middle of the screen), top, or middle.");
    add(L"flyouts.balloons", L"Tray balloons",
        L"The messages tray icons pop up (\"update ready\", \"download done\"), over the corner by the tray. A click tells the "
        L"app, as Windows' would; a right click puts it away.").isSwitch = true;
    add(L"flyouts.balloonMs", L"Balloons shown for", L"Milliseconds (1000 to 60000); it waits while the pointer is on it.");
    add(L"flyouts.hideWindows", L"Hide Windows' own",
        L"Windows' own volume and media pop-up kept away while these are on: the volume keys are taken here (stepped as "
        L"Windows steps them), so Explorer's never opens; a media key's is hidden as it shows.").isSwitch = true;
}

void FlyoutsStart(HWND) {
    if (g_thread.joinable()) return;
    g_thread = std::thread(Thread);
}

void FlyoutsApply() {
    if (g_wnd) PostMessageW(g_wnd, kMsgApply, 0, 0);
}

void FlyoutsBalloon(const Balloon& b) {
    if (!g_wnd) return;
    auto* copy = new Balloon(b);
    copy->icon = b.icon ? CopyIcon(b.icon) : nullptr;
    if (!PostMessageW(g_wnd, kMsgBalloon, 0, (LPARAM)copy)) {
        if (copy->icon) DestroyIcon(copy->icon);
        delete copy;
    }
}

void FlyoutsStop() {
    if (!g_thread.joinable()) return;
    for (int i = 0; i < 100 && !g_wnd; i++) Sleep(10);
    if (g_wnd) PostMessageW(g_wnd, WM_QUIT, 0, 0);
    g_thread.join();
}

}  // namespace td
