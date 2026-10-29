// Screen capture on the Print Screen key: see capture.h.
//
// One thread of its own holds the keyboard hook, its window (which owns the clipboard and gets the key from the
// hook), the window picker, the hint and the recording's red dot. The hook only posts: tap, double tap and hold
// are told apart here, on timers. Pictures are read off the screen, a window's too while nothing from another app
// is over it, so what Tourne draws on it (the visualizer in the taskbar, glass, panels) comes out as it's seen;
// a covered window is asked to draw itself (PrintWindow) so it still comes out whole. They go on the clipboard
// as a bitmap and as PNG, and are saved as PNG. A recording is ffmpeg reading an area of the screen (a window:
// its area) into an MP4, by Desktop Duplication (ddagrab) when ffmpeg has it and the area is on one screen, else
// GDI (gdigrab); on the graphics card when ffmpeg can (NVENC), else on the processor. It's told to stop with "q",
// so the file is whole, and the file goes on the clipboard; ffmpeg's complaints go to a log, read when it fails.
// While Screen Toolkit runs, a recording is its job instead (recordRect: its codec, sound, frame rate, folder and
// copy to the clipboard), and pictures go to its screenshot folder; it tells this side how its recording stands
// (CaptureToolkit), so the key and the red dot stop one it started too. What to record is picked on a bar at the
// top of the screen, as Snipping Tool's, in the theme's colours.

#include "capture.h"

#include <d2d1.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <dxgi.h>
#include <mmdeviceapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wincodec.h>
#include <windowsx.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <mutex>
#include <cstdarg>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#include "plugin-runtime.h"
#include "studio.h"
#include "td.h"

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"capture", format, args);
    va_end(args);
}

struct Config {
    bool on = true;
    int holdMs = 600, doubleTapMs = 300, fps = 60;
    // How recordings are made (Studio's Capture page; Screen Toolkit records with these for Print Screen).
    std::wstring encoder = L"auto", codec = L"hevc", method = L"dda", soundDevice = L"default", micDevice, after = L"none";
    int quality = 23, height = 0;
    bool sound = true, mic = false, afterCopy = true;
    bool annotate = false;  // screenshots marked up before they're kept (capture-annotate.inl)
    bool cursor = true;
    std::wstring pictures, videos, ffmpeg;
    UINT keyMods = 0, keyVk = VK_SNAPSHOT;  // the key (capture.hotkey; empty: Print Screen alone)
};

// "ctrl+alt+44", "print", "f12", "shift+s": the Studio's hotkey text. False if it isn't one.
bool ParseKey(std::wstring text, UINT* mods, UINT* vk) {
    *mods = 0, *vk = 0;
    for (auto& ch : text) ch = (wchar_t)towlower(ch);
    if (size_t sp = text.find(L' '); sp != std::wstring::npos) text = text.substr(0, sp);  // (how it's pressed: not here)
    std::vector<std::wstring> parts;
    for (size_t at = 0; at <= text.size();) {
        size_t plus = text.find(L'+', at);
        if (plus == std::wstring::npos) plus = text.size();
        if (plus > at) parts.push_back(text.substr(at, plus - at));
        at = plus + 1;
    }
    if (parts.empty()) return false;
    for (size_t i = 0; i < parts.size(); i++) {
        std::wstring const& k = parts[i];
        if (k == L"ctrl" || k == L"control") *mods |= MOD_CONTROL;
        else if (k == L"alt") *mods |= MOD_ALT;
        else if (k == L"shift") *mods |= MOD_SHIFT;
        else if (k == L"win") *mods |= MOD_WIN;
        else if (i + 1 != parts.size()) return false;
        else if (k == L"print" || k == L"printscreen" || k == L"prtsc") *vk = VK_SNAPSHOT;
        else if (k.find_first_not_of(L"0123456789") == std::wstring::npos) *vk = (UINT)_wtoi(k.c_str());
        else if (k.size() >= 2 && k[0] == L'f' && iswdigit(k[1])) *vk = VK_F1 + _wtoi(k.c_str() + 1) - 1;
        else if (k.size() == 1) {
            SHORT scan = VkKeyScanW(k[0]);
            if (scan != -1) *vk = scan & 0xFF;
        }
    }
    return *vk > 0 && *vk < 255;
}

Config ReadConfig() {
    Config c;
    c.on = GetInt(L"capture", L"enabled") != 0;
    c.holdMs = std::clamp(GetInt(L"capture", L"holdMs"), 250, 2000);
    c.doubleTapMs = std::clamp(GetInt(L"capture", L"doubleTapMs"), 150, 800);
    c.fps = std::clamp(GetInt(L"capture", L"fps"), 10, 240);
    c.quality = std::clamp(GetInt(L"capture", L"quality"), 10, 45);
    c.height = std::clamp(GetInt(L"capture", L"resolution"), 0, 4320);
    c.sound = GetInt(L"capture", L"sound") != 0;
    c.mic = GetInt(L"capture", L"mic") != 0;
    c.afterCopy = GetInt(L"capture", L"afterCopy") != 0;
    c.annotate = GetInt(L"capture", L"annotate") != 0;
    auto word = [](const wchar_t* key, const wchar_t* fallback) {
        std::wstring out = fallback;
        if (const wchar_t* v = GetString(L"capture", key)) {
            if (*v) out = v;
            FreeString(v);
        }
        return out;
    };
    c.encoder = word(L"encoder", L"auto");
    c.codec = word(L"codec", L"hevc");
    c.method = word(L"method", L"dda");
    c.soundDevice = word(L"soundDevice", L"default");
    c.micDevice = word(L"micDevice", L"");
    c.after = word(L"after", L"none");
    c.cursor = GetInt(L"capture", L"cursor") != 0;
    auto text = [](const wchar_t* key) {
        std::wstring out;
        if (const wchar_t* v = GetString(L"capture", key)) {
            wchar_t expanded[MAX_PATH * 2]{};
            if (*v && ExpandEnvironmentStringsW(v, expanded, ARRAYSIZE(expanded))) out = expanded;
            FreeString(v);
        }
        while (!out.empty() && (out.back() == L'\\' || out.back() == L'/')) out.pop_back();
        return out;
    };
    c.pictures = text(L"pictures");
    c.videos = text(L"videos");
    c.ffmpeg = text(L"ffmpeg");
    std::wstring key = word(L"hotkey", L"");
    UINT mods = 0, vk = 0;
    if (!key.empty() && ParseKey(key, &mods, &vk)) c.keyMods = mods, c.keyVk = vk;
    return c;
}

Config g_cfg;
HWND g_host = nullptr, g_wnd = nullptr;
DWORD g_threadId = 0;
std::thread g_thread;
std::atomic<bool> g_on{false};
HHOOK g_hook = nullptr;  // (owned by its thread, g_hookThread)
std::atomic<DWORD> g_hookThread{0};
std::atomic<UINT> g_keyVk{VK_SNAPSHOT}, g_keyMods{0};  // (g_cfg's, for the hook)
bool g_keyTaken = false;  // the key went down as ours: its release is ours too (hook thread)

constexpr UINT kMsgDown = WM_APP + 1, kMsgUp = WM_APP + 2, kMsgApply = WM_APP + 3, kMsgRecorded = WM_APP + 4, kMsgToolkit = WM_APP + 5;
// While the picker or the record bar is up, the pointer and keys come to it through hooks (it never takes the
// focus, so an open menu or flyout stays open to be captured): kMsgPointer (wParam: the mouse message; lParam:
// the point on the screen) and kMsgKey (wParam: the key).
constexpr UINT kMsgPointer = WM_APP + 6, kMsgKey = WM_APP + 7;
constexpr UINT_PTR kTimerHold = 1, kTimerTap = 2, kTimerHint = 3, kTimerRec = 4, kTimerDelegate = 5;

// Screen Toolkit as it last said (any thread writes, this one reads).
std::mutex g_linkLock;
std::wstring g_tkState = L"off", g_tkPictures;

std::wstring ToolkitState() {
    std::lock_guard lock(g_linkLock);
    return g_tkState;
}

// The theme's colours (System Icons' flyout palette, else the theme's roles, else dark).
struct Look {
    COLORREF ground, raised, line, text, muted, accent;
};

Look Themed() {
    auto c = [](uint32_t v) { return RGB(v >> 16 & 255, v >> 8 & 255, v & 255); };
    uint32_t p[11];
    if (FlyoutPalette(p)) return {c(p[0]), c(p[10]), c(p[2]), c(p[4]), c(p[6]), c(p[5])};
    Look look{RGB(32, 32, 32), RGB(52, 52, 52), RGB(64, 64, 64), RGB(235, 235, 235), RGB(160, 160, 160), RGB(79, 195, 247)};
    uint32_t v;
    for (int i = 0; i < kThemeRoleCount; i++) {
        if (!ThemeRoleColor(kThemeRoles[i], &v)) continue;
        std::wstring t = kThemeRoles[i].token;
        if (t == L"@ground") look.ground = c(v);
        else if (t == L"@raised") look.raised = c(v);
        else if (t == L"@line") look.line = c(v);
        else if (t == L"@text") look.text = c(v);
        else if (t == L"@muted") look.muted = c(v);
        else if (t == L"@accent") look.accent = c(v);
    }
    return look;
}

HFONT UiFont(int px, bool icons = false) {
    return CreateFontW(-px, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0,
                       icons ? L"Segoe Fluent Icons" : L"Segoe UI Variable Text");
}

void Rounded(HWND w, COLORREF border) {
    DWORD corner = 2;  // DWMWCP_ROUND
    DwmSetWindowAttribute(w, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corner, sizeof(corner));
    DwmSetWindowAttribute(w, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));
}

// The key: down (not its repeats) and up, and what's under way.
bool g_keyDown = false, g_tapPending = false, g_holdFired = false, g_consumed = false;

std::wstring KnownFolder(REFKNOWNFOLDERID id) {
    PWSTR p = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_CREATE, nullptr, &p))) out = p;
    CoTaskMemFree(p);
    return out;
}

std::wstring Stamp() {
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t s[40];
    swprintf(s, 40, L"%04u-%02u-%02u %02u%02u%02u", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return s;
}

std::wstring FolderFor(bool video) {
    std::wstring dir = video ? g_cfg.videos : g_cfg.pictures;
    if (dir.empty() && !video) {
        std::lock_guard lock(g_linkLock);
        if (g_tkState != L"off") dir = g_tkPictures;
    }
    if (dir.empty()) dir = KnownFolder(video ? FOLDERID_Videos : FOLDERID_Pictures) + (video ? L"\\Captures" : L"\\Screenshots");
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
    return dir;
}

////////////////////////////////////////////////////////////////////////////////
// The hint: a few words by the pointer for a moment ("Copied", what to do next). Captures leave it out.

HWND g_hint = nullptr;
std::wstring g_hintText;

LRESULT CALLBACK HintProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(wnd, &ps);
        RECT rc;
        GetClientRect(wnd, &rc);
        Look look = Themed();
        HBRUSH back = CreateSolidBrush(look.ground);
        FillRect(dc, &rc, back);
        DeleteObject(back);
        HBRUSH edge = CreateSolidBrush(look.line);
        FrameRect(dc, &rc, edge);
        DeleteObject(edge);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, look.text);
        HFONT font = UiFont(MulDiv(14, GetDpiForWindow(wnd), 96));
        HGDIOBJ old = SelectObject(dc, font);
        DrawTextW(dc, g_hintText.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, old);
        DeleteObject(font);
        EndPaint(wnd, &ps);
        return 0;
    }
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProcW(wnd, msg, wp, lp);
}

void Hint(std::wstring const& text, UINT ms = 1600) {
    if (!g_hint) {
        g_hint = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED,
                                 L"TourneCaptureHint", L"", WS_POPUP, 0, 0, 10, 10, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        SetLayeredWindowAttributes(g_hint, 0, 235, LWA_ALPHA);
        SetWindowDisplayAffinity(g_hint, WDA_EXCLUDEFROMCAPTURE);
    }
    g_hintText = text;
    POINT pt;
    GetCursorPos(&pt);
    SetWindowPos(g_hint, nullptr, pt.x + 16, pt.y + 20, 10, 10, SWP_NOACTIVATE | SWP_NOZORDER);  // (its DPI is that screen's)
    int dpi = GetDpiForWindow(g_hint);
    HDC dc = GetDC(g_hint);
    HFONT font = UiFont(MulDiv(14, dpi, 96));
    HGDIOBJ old = SelectObject(dc, font);
    SIZE size{};
    GetTextExtentPoint32W(dc, text.c_str(), (int)text.size(), &size);
    SelectObject(dc, old);
    DeleteObject(font);
    ReleaseDC(g_hint, dc);
    int w = size.cx + MulDiv(28, dpi, 96), h = size.cy + MulDiv(16, dpi, 96);
    HRGN round = CreateRoundRectRgn(0, 0, w + 1, h + 1, MulDiv(10, dpi, 96), MulDiv(10, dpi, 96));
    SetWindowRgn(g_hint, round, FALSE);
    SetWindowPos(g_hint, HWND_TOPMOST, pt.x + 16, pt.y + 20, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(g_hint, nullptr, TRUE);
    SetTimer(g_wnd, kTimerHint, ms, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// Pictures.

struct Picture {
    int w = 0, h = 0;
    std::vector<uint32_t> px;  // top-down BGRA
};

Picture FromDc(HDC from, int x, int y, int w, int h) {
    Picture p;
    if (w <= 0 || h <= 0) return p;
    p.w = w, p.h = h;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC mem = CreateCompatibleDC(from);
    HBITMAP bmp = CreateDIBSection(from, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, from, x, y, SRCCOPY | CAPTUREBLT);
    GdiFlush();
    p.px.assign((uint32_t*)bits, (uint32_t*)bits + (size_t)w * h);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    for (auto& v : p.px) v |= 0xFF000000;
    return p;
}

Picture GrabScreenRect(RECT r) {
    HDC screen = GetDC(nullptr);
    Picture p = FromDc(screen, r.left, r.top, r.right - r.left, r.bottom - r.top);
    ReleaseDC(nullptr, screen);
    return p;
}

RECT FrameOf(HWND h) {
    RECT r{};
    if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(h, &r);
    return r;
}

bool Cloaked(HWND h);

RECT VirtualScreen() {
    RECT v{GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0, 0};
    v.right = v.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    v.bottom = v.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    return v;
}

DWORD ProcessOf(HWND h) {
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    return pid;
}

// Whether what's on the screen over r is h as it's seen: nothing from another app above it there. Its own
// app's windows (its menus, popups) and Tourne's (the visualizer, glass, panels drawn over the taskbar) belong
// in the picture; windows left out of captures and see-through overlays that never take clicks don't count.
bool SeenWhole(HWND h, RECT r) {
    RECT v = VirtualScreen(), in;
    if (!IntersectRect(&in, &r, &v) || !EqualRect(&in, &r)) return false;  // (partly off the screens)
    DWORD app = ProcessOf(h), own = GetCurrentProcessId();
    for (HWND w = GetWindow(h, GW_HWNDPREV); w; w = GetWindow(w, GW_HWNDPREV)) {
        if (!IsWindowVisible(w) || IsIconic(w) || Cloaked(w)) continue;
        DWORD pid = ProcessOf(w);
        if (pid == app || pid == own) continue;
        DWORD affinity = 0;
        if (GetWindowDisplayAffinity(w, &affinity) && affinity == WDA_EXCLUDEFROMCAPTURE) continue;
        LONG ex = GetWindowLongW(w, GWL_EXSTYLE);
        if ((ex & WS_EX_TRANSPARENT) && (ex & WS_EX_LAYERED)) continue;
        RECT f = FrameOf(w), x;
        if (IntersectRect(&x, &f, &r)) return false;
    }
    return true;
}

// A window as it's seen (the screen, with what Tourne draws over it), when nothing else covers it; else as it
// draws itself (PrintWindow: whole even when covered), cut to its frame; off the screen instead when that gives
// back nothing (some games and video players draw where PrintWindow can't see).
Picture GrabWindow(HWND h) {
    RECT window, frame = FrameOf(h);
    if (SeenWhole(h, frame)) return GrabScreenRect(frame);
    GetWindowRect(h, &window);
    int w = window.right - window.left, hh = window.bottom - window.top;
    Picture whole;
    if (w > 0 && hh > 0) {
        HDC screen = GetDC(nullptr);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -hh, 1, 32, BI_RGB};
        void* bits = nullptr;
        HDC mem = CreateCompatibleDC(screen);
        HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        HGDIOBJ old = SelectObject(mem, bmp);
        bool printed = PrintWindow(h, mem, PW_RENDERFULLCONTENT) != FALSE;
        GdiFlush();
        if (printed) {
            whole.w = w, whole.h = hh;
            whole.px.assign((uint32_t*)bits, (uint32_t*)bits + (size_t)w * hh);
        }
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
        ReleaseDC(nullptr, screen);
    }
    bool blank = whole.px.empty() || std::all_of(whole.px.begin(), whole.px.end(), [](uint32_t v) { return (v & 0xFFFFFF) == 0; });
    if (blank) return GrabScreenRect(frame);
    Picture out;
    int fx = frame.left - window.left, fy = frame.top - window.top;
    out.w = std::min<int>(frame.right - frame.left, w - fx), out.h = std::min<int>(frame.bottom - frame.top, hh - fy);
    if (fx < 0 || fy < 0 || out.w <= 0 || out.h <= 0) return whole;
    out.px.resize((size_t)out.w * out.h);
    for (int y = 0; y < out.h; y++)
        for (int x = 0; x < out.w; x++) out.px[(size_t)y * out.w + x] = whole.px[(size_t)(y + fy) * w + x + fx] | 0xFF000000;
    return out;
}

std::vector<BYTE> EncodePng(Picture const& p) {
    std::vector<BYTE> out;
    IWICImagingFactory* f = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) return out;
    IStream* stream = nullptr;
    IWICBitmapEncoder* enc = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    if (SUCCEEDED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)) && SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &enc)) &&
        SUCCEEDED(enc->Initialize(stream, WICBitmapEncoderNoCache)) && SUCCEEDED(enc->CreateNewFrame(&frame, nullptr)) &&
        SUCCEEDED(frame->Initialize(nullptr)) && SUCCEEDED(frame->SetSize(p.w, p.h))) {
        WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
        frame->SetPixelFormat(&fmt);
        if (SUCCEEDED(frame->WritePixels(p.h, p.w * 4, (UINT)(p.px.size() * 4), (BYTE*)p.px.data())) && SUCCEEDED(frame->Commit()) &&
            SUCCEEDED(enc->Commit())) {
            HGLOBAL mem = nullptr;
            STATSTG st{};
            if (SUCCEEDED(stream->Stat(&st, STATFLAG_NONAME)) && SUCCEEDED(GetHGlobalFromStream(stream, &mem))) {
                out.resize((size_t)st.cbSize.QuadPart);
                memcpy(out.data(), GlobalLock(mem), out.size());
                GlobalUnlock(mem);
            }
        }
    }
    if (frame) frame->Release();
    if (enc) enc->Release();
    if (stream) stream->Release();
    f->Release();
    return out;
}

HGLOBAL Global(const void* data, size_t size) {
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (mem) {
        memcpy(GlobalLock(mem), data, size);
        GlobalUnlock(mem);
    }
    return mem;
}

#include "capture-annotate.inl"

// On the clipboard as a bitmap (any app) and as PNG (browsers, chat apps keep it sharp), and kept as a file.
// With capture.annotate on, marked up first (capture-annotate.inl); Esc there keeps it plain.
void PictureOut(Picture const& shot, std::wstring const& what) {
    if (shot.px.empty()) return Hint(L"Nothing to capture there");
    Picture p = shot;
    if (g_cfg.annotate) annotate::Run(p);
    std::vector<BYTE> png = EncodePng(p);
    std::wstring path = FolderFor(false) + L"\\Tourne " + Stamp() + L".png";
    if (!png.empty()) {
        HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        DWORD wrote = 0;
        if (file != INVALID_HANDLE_VALUE) {
            WriteFile(file, png.data(), (DWORD)png.size(), &wrote, nullptr);
            CloseHandle(file);
        }
    }
    // CF_DIB: bottom-up rows after the header.
    std::vector<BYTE> dib(sizeof(BITMAPINFOHEADER) + p.px.size() * 4);
    BITMAPINFOHEADER bh{sizeof(BITMAPINFOHEADER), p.w, p.h, 1, 32, BI_RGB};
    bh.biSizeImage = (DWORD)(p.px.size() * 4);
    memcpy(dib.data(), &bh, sizeof(bh));
    for (int y = 0; y < p.h; y++) memcpy(dib.data() + sizeof(bh) + (size_t)y * p.w * 4, &p.px[(size_t)(p.h - 1 - y) * p.w], (size_t)p.w * 4);
    if (OpenClipboard(g_wnd)) {
        EmptyClipboard();
        if (HGLOBAL m = Global(dib.data(), dib.size())) SetClipboardData(CF_DIB, m);
        static UINT pngFormat = RegisterClipboardFormatW(L"PNG");
        if (!png.empty())
            if (HGLOBAL m = Global(png.data(), png.size())) SetClipboardData(pngFormat, m);
        CloseClipboard();
    }
    Log(L"%ls: %dx%d copied, kept as %ls", what.c_str(), p.w, p.h, path.c_str());
    Hint(L"Copied: " + what + L" (Ctrl+V)");
}

// A file on the clipboard, as Explorer puts one (Ctrl+V into a chat attaches it).
void FileOut(std::wstring const& path) {
    std::vector<BYTE> drop(sizeof(DROPFILES) + (path.size() + 2) * sizeof(wchar_t), 0);
    auto* d = (DROPFILES*)drop.data();
    d->pFiles = sizeof(DROPFILES);
    d->fWide = TRUE;
    memcpy(drop.data() + sizeof(DROPFILES), path.c_str(), path.size() * sizeof(wchar_t));
    if (!OpenClipboard(g_wnd)) return;
    EmptyClipboard();
    if (HGLOBAL m = Global(drop.data(), drop.size())) SetClipboardData(CF_HDROP, m);
    DWORD copy = DROPEFFECT_COPY;
    static UINT effect = RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT);
    if (HGLOBAL m = Global(&copy, sizeof(copy))) SetClipboardData(effect, m);
    CloseClipboard();
}

RECT ScreenUnderPointer() {
    POINT pt;
    GetCursorPos(&pt);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    return mi.rcMonitor;
}

////////////////////////////////////////////////////////////////////////////////
// The window picker: a see-through layer over every screen (it takes the clicks), with the window under the
// pointer outlined. A click takes it (the desktop: the whole screen); Esc or a right click gives up.

enum class PickFor { Picture, Recording };
HWND g_pick = nullptr;
PickFor g_pickFor = PickFor::Picture;
HWND g_picked = nullptr;
RECT g_virtual{};

void StartRecording(RECT area, HWND window, std::wstring const& what);

bool Cloaked(HWND h) {
    DWORD c = 0;
    return SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &c, sizeof(c))) && c;
}

// The window a click at pt would land on, leaving the picker (and Tourne's own see-through layers) out.
HWND WindowAt(POINT pt) {
    for (HWND h = GetTopWindow(nullptr); h; h = GetWindow(h, GW_HWNDNEXT)) {
        if (h == g_pick || h == g_hint || !IsWindowVisible(h) || IsIconic(h) || Cloaked(h)) continue;
        LONG ex = GetWindowLongW(h, GWL_EXSTYLE);
        if (ex & WS_EX_TRANSPARENT) continue;
        RECT r = FrameOf(h);
        if (!PtInRect(&r, pt)) continue;
        return h;
    }
    return nullptr;
}

bool IsDesktop(HWND h) {
    wchar_t cls[32] = L"";
    if (h) GetClassNameW(h, cls, 32);
    return !h || !wcscmp(cls, L"Progman") || !wcscmp(cls, L"WorkerW");
}

void DrawPick() {
    int w = g_virtual.right - g_virtual.left, h = g_virtual.bottom - g_virtual.top;
    HDC screen = GetDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mem, bmp);
    auto* px = (uint32_t*)bits;
    std::fill(px, px + (size_t)w * h, 0x01000000u);  // (all but see-through: it takes the clicks)
    RECT r = IsDesktop(g_picked) ? ScreenUnderPointer() : FrameOf(g_picked);
    OffsetRect(&r, -g_virtual.left, -g_virtual.top);
    const uint32_t edge = 0xFF4FC3F7;  // (premultiplied: opaque)
    for (int t = 0; t < 4; t++)
        for (int y = std::max<int>(0, r.top + t); y < std::min<int>(h, r.bottom - t); y++)
            for (int x = std::max<int>(0, r.left + t); x < std::min<int>(w, r.right - t); x++)
                if (y == r.top + t || y == r.bottom - t - 1 || x == r.left + t || x == r.right - t - 1) px[(size_t)y * w + x] = edge;
    POINT dst{g_virtual.left, g_virtual.top}, src{0, 0};
    SIZE size{w, h};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(g_pick, screen, &dst, &size, mem, &src, 0, &blend, ULW_ALPHA);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
}

void WatchPointer();
void EndPick() {
    if (g_pick) DestroyWindow(g_pick);
    g_pick = nullptr;
    WatchPointer();
}

void Picked() {
    HWND h = g_picked;
    PickFor what = g_pickFor;
    EndPick();
    Sleep(60);  // (the layer gone from the screen before it's read)
    if (what == PickFor::Picture) {
        if (IsDesktop(h)) PictureOut(GrabScreenRect(ScreenUnderPointer()), L"the screen");
        else PictureOut(GrabWindow(h), L"the window");
    } else {
        if (IsDesktop(h)) StartRecording(ScreenUnderPointer(), nullptr, L"the screen");
        else StartRecording(FrameOf(h), h, L"the window");
    }
}

LRESULT CALLBACK PickProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_MOUSEMOVE: {
            POINT pt;
            GetCursorPos(&pt);
            HWND h = WindowAt(pt);
            if (h != g_picked) {
                g_picked = h;
                DrawPick();
            }
            return 0;
        }
        case WM_LBUTTONUP:
            Picked();
            return 0;
        case WM_RBUTTONUP:
            EndPick();
            return 0;
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) EndPick();
            else if (wp == VK_RETURN) Picked();
            return 0;
        case WM_SETCURSOR:
            SetCursor(LoadCursorW(nullptr, IDC_CROSS));
            return TRUE;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void StartPick(PickFor what) {
    EndPick();
    g_pickFor = what;
    g_virtual = {GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0, 0};
    g_virtual.right = g_virtual.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    g_virtual.bottom = g_virtual.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    g_pick = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE, L"TourneCapturePick", L"", WS_POPUP, g_virtual.left,
                             g_virtual.top, g_virtual.right - g_virtual.left, g_virtual.bottom - g_virtual.top, nullptr, nullptr,
                             GetModuleHandleW(nullptr), nullptr);
    SetWindowDisplayAffinity(g_pick, WDA_EXCLUDEFROMCAPTURE);
    POINT pt;
    GetCursorPos(&pt);
    g_picked = WindowAt(pt);
    DrawPick();
    ShowWindow(g_pick, SW_SHOWNOACTIVATE);
    WatchPointer();
    Hint(what == PickFor::Picture ? L"Click a window to capture it (Esc: never mind)" : L"Click a window to record it (Esc: never mind)", 2500);
}

////////////////////////////////////////////////////////////////////////////////
// Recording.

struct Recording {
    HANDLE process = nullptr, input = nullptr;
    std::wstring path, what;
    ULONGLONG started = 0;
    RECT area{};
    std::function<void()> soundStop;  // (the PC's sound, ended once ffmpeg is done with it)
};
Recording g_rec;
std::wstring g_firstMic;  // the first microphone ffmpeg lists (DirectShow), for Microphone left empty
HWND g_dot = nullptr;  // the red dot over the recorded screen (left out of the recording; a click stops)
ULONGLONG g_dotStarted = 0;
// Handed to Screen Toolkit: asked (until it says it's recording, or gives up), and recording.
bool g_pending = false, g_delegated = false;
RECT g_pendingArea{};
std::atomic<int> g_nvenc{-1};  // whether ffmpeg has NVENC (-1: not known yet; asked off this thread, as the hook lives here)
std::atomic<int> g_dda{-1};    // whether it has Desktop Duplication (ddagrab), the same way

std::wstring FindFfmpeg() {
    if (!g_cfg.ffmpeg.empty() && GetFileAttributesW(g_cfg.ffmpeg.c_str()) != INVALID_FILE_ATTRIBUTES) return g_cfg.ffmpeg;
    wchar_t found[MAX_PATH];
    if (SearchPathW(nullptr, L"ffmpeg.exe", nullptr, MAX_PATH, found, nullptr)) return found;
    std::wstring local = KnownFolder(FOLDERID_LocalAppData);
    for (const wchar_t* rel : {L"\\Programs\\Convertify\\resources\\tools\\ffmpeg\\bin\\ffmpeg.exe",
                               L"\\Microsoft\\WinGet\\Links\\ffmpeg.exe"}) {
        std::wstring p = local + rel;
        if (GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES) return p;
    }
    return L"";
}

// Runs ffmpeg with words and waits for it (for asking it things); its output.
std::string RunFor(std::wstring const& exe, std::wstring const& args) {
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE read = nullptr, write = nullptr;
    if (!CreatePipe(&read, &write, &sa, 0)) return {};
    SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW si{sizeof(si)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = si.hStdError = write;
    PROCESS_INFORMATION pi{};
    std::wstring line = L"\"" + exe + L"\" " + args;
    std::string out;
    if (CreateProcessW(nullptr, line.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        CloseHandle(write);
        write = nullptr;
        char buf[4096];
        DWORD got;
        while (ReadFile(read, buf, sizeof(buf), &got, nullptr) && got) out.append(buf, got);
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    if (write) CloseHandle(write);
    CloseHandle(read);
    return out;
}

LRESULT CALLBACK DotProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(wnd, &ps);
        RECT rc;
        GetClientRect(wnd, &rc);
        Look look = Themed();
        int dpi = GetDpiForWindow(wnd);
        HBRUSH back = CreateSolidBrush(look.ground);
        FillRect(dc, &rc, back);
        DeleteObject(back);
        HBRUSH red = CreateSolidBrush(RGB(232, 64, 64));
        HGDIOBJ oldBrush = SelectObject(dc, red);
        HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
        int d = MulDiv(10, dpi, 96), left = MulDiv(12, dpi, 96);
        Ellipse(dc, left, (rc.bottom - d) / 2, left + d, (rc.bottom - d) / 2 + d);
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        DeleteObject(red);
        ULONGLONG s = (GetTickCount64() - g_dotStarted) / 1000;
        wchar_t time[32];
        swprintf(time, 32, L"%llu:%02llu", s / 60, s % 60);
        SetBkMode(dc, TRANSPARENT);
        HFONT font = UiFont(MulDiv(14, dpi, 96));
        HGDIOBJ oldFont = SelectObject(dc, font);
        RECT t = rc;
        t.left = left + d + MulDiv(8, dpi, 96);
        t.right -= MulDiv(12, dpi, 96);
        SetTextColor(dc, look.text);
        DrawTextW(dc, time, -1, &t, DT_VCENTER | DT_SINGLELINE);
        SetTextColor(dc, look.muted);
        DrawTextW(dc, L"Print Screen stops", -1, &t, DT_VCENTER | DT_SINGLELINE | DT_RIGHT);
        SelectObject(dc, oldFont);
        DeleteObject(font);
        EndPaint(wnd, &ps);
        return 0;
    }
    if (msg == WM_LBUTTONUP) {
        PostMessageW(g_wnd, kMsgDown, 1, 0);  // (as the key: stops)
        return 0;
    }
    if (msg == WM_SETCURSOR) {
        SetCursor(LoadCursorW(nullptr, IDC_HAND));
        return TRUE;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

// The red dot at the top of the recorded area, with the time.
void ShowDot(RECT area) {
    if (!g_dot) {
        g_dot = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"TourneCaptureDot", L"", WS_POPUP, 0, 0, 10, 10,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        SetWindowDisplayAffinity(g_dot, WDA_EXCLUDEFROMCAPTURE);
    }
    g_dotStarted = GetTickCount64();
    SetWindowPos(g_dot, nullptr, area.left + 8, area.top + 8, 10, 10, SWP_NOACTIVATE | SWP_NOZORDER);  // (its DPI is that screen's)
    int dpi = GetDpiForWindow(g_dot);
    int dw = MulDiv(220, dpi, 96), dh = MulDiv(34, dpi, 96);
    Rounded(g_dot, Themed().line);
    SetWindowPos(g_dot, HWND_TOPMOST, (area.left + area.right - dw) / 2, area.top + MulDiv(10, dpi, 96), dw, dh,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(g_dot, nullptr, FALSE);
    SetTimer(g_wnd, kTimerRec, 1000, nullptr);
}

void HideDot() {
    KillTimer(g_wnd, kTimerRec);
    if (g_dot) ShowWindow(g_dot, SW_HIDE);
}

// A message to Screen Toolkit's service, off this thread (it can wait up to 3 s; the keyboard hook lives here).
void SendToolkit(std::vector<std::wstring> words) {
    words.insert(words.begin(), {L"plugin", L"tourne/screen-toolkit:service", L"all"});
    std::thread([words] { PluginMessage(words); }).detach();
}

// A JSON string.
std::wstring Quoted(std::wstring const& s) {
    std::wstring out = L"\"";
    for (wchar_t c : s) {
        if (c == L'"' || c == L'\\') out += L'\\';
        if (c >= 32) out += c;
    }
    return out + L"\"";
}

// The encoder family Capture's "auto" means: the graphics card's when ffmpeg has it (NVIDIA here), else the processor.
std::wstring EncoderFamily() {
    if (g_cfg.encoder != L"auto") return g_cfg.encoder;
    return g_nvenc == 1 ? L"nvidia" : L"cpu";
}

void Delegate(RECT area, HWND window, std::wstring const& what) {
    std::wstring state = ToolkitState();
    if (state == L"recording" || state == L"converting") return Hint(L"Screen Toolkit is still busy with a recording", 2500);
    int w = area.right - area.left, h = area.bottom - area.top;
    wchar_t hex[24];
    swprintf(hex, 24, L"0x%llx", (unsigned long long)(ULONG_PTR)window);
    // Everything Capture's page sets, as one JSON word (recordWith in Screen Toolkit's service).
    auto const& c = g_cfg;
    std::wstring json = L"{\"x\":" + std::to_wstring(area.left) + L",\"y\":" + std::to_wstring(area.top) + L",\"w\":" + std::to_wstring(w) +
                        L",\"h\":" + std::to_wstring(h) + (window ? L",\"hwnd\":" + Quoted(hex) : L"") +
                        L",\"encoder\":" + Quoted(EncoderFamily()) + L",\"codec\":" + Quoted(c.codec) + L",\"quality\":" +
                        std::to_wstring(c.quality) + L",\"height\":" + std::to_wstring(c.height) + L",\"fps\":" + std::to_wstring(c.fps) +
                        L",\"method\":" + Quoted(c.method) + L",\"cursor\":" + (c.cursor ? L"true" : L"false") + L",\"sound\":" +
                        (c.sound ? L"true" : L"false") + L",\"soundDevice\":" + Quoted(c.soundDevice) + L",\"mic\":" +
                        (c.mic ? L"true" : L"false") + L",\"micDevice\":" + Quoted(c.micDevice) + L",\"folder\":" + Quoted(FolderFor(true)) +
                        L",\"after\":" + Quoted(c.after) + L",\"afterCopy\":" + (c.afterCopy ? L"true" : L"false") + L"}";
    SendToolkit({L"recordWith", json});
    g_pending = true;
    g_pendingArea = area;
    SetTimer(g_wnd, kTimerDelegate, 10000, nullptr);
    Log(L"recording %ls (%dx%d at %d,%d) through Screen Toolkit", what.c_str(), w, h, (int)area.left, (int)area.top);
}

// Screen Toolkit's recording changed: the dot shows while it records (one asked from here), and the key stops it.
void ToolkitChanged() {
    std::wstring state = ToolkitState();
    if (state == L"recording") {
        KillTimer(g_wnd, kTimerDelegate);
        g_delegated = true;
        if (g_pending) ShowDot(g_pendingArea);
        g_pending = false;
    } else if (g_delegated) {
        g_delegated = false;
        HideDot();
    }
}

// Desktop Duplication's screen for an area: ffmpeg's ddagrab reads outputs of the first graphics adapter, by
// their place in its list. False when the area isn't wholly on one of them (it then goes by GDI).
bool DdaOutput(RECT area, int* index, POINT* origin) {
    IDXGIFactory1* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;
    bool found = false;
    IDXGIAdapter1* adapter = nullptr;
    if (SUCCEEDED(factory->EnumAdapters1(0, &adapter))) {
        IDXGIOutput* out = nullptr;
        for (UINT i = 0; !found && adapter->EnumOutputs(i, &out) != DXGI_ERROR_NOT_FOUND; i++) {
            DXGI_OUTPUT_DESC d{};
            if (out && SUCCEEDED(out->GetDesc(&d)) && d.AttachedToDesktop && d.Rotation <= DXGI_MODE_ROTATION_IDENTITY) {
                RECT r = d.DesktopCoordinates, in;
                if (IntersectRect(&in, &area, &r) && EqualRect(&in, &area)) {
                    *index = (int)i;
                    *origin = {r.left, r.top};
                    found = true;
                }
            }
            if (out) out->Release(), out = nullptr;
        }
        adapter->Release();
    }
    factory->Release();
    return found;
}

std::wstring FfmpegLog() {
    wchar_t temp[MAX_PATH];
    GetTempPathW(MAX_PATH, temp);
    return std::wstring(temp) + L"tourne-capture-ffmpeg.log";
}

// ffmpeg's last words, for the log when a recording didn't come out.
std::wstring FfmpegSaid() {
    HANDLE f = CreateFileW(FfmpegLog().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return L"";
    char buf[1024];
    DWORD got = 0;
    LARGE_INTEGER size{};
    GetFileSizeEx(f, &size);
    if (size.QuadPart > (LONGLONG)sizeof(buf) - 1) {
        LARGE_INTEGER at;
        at.QuadPart = size.QuadPart - (LONGLONG)sizeof(buf) + 1;
        SetFilePointerEx(f, at, nullptr, FILE_BEGIN);
    }
    ReadFile(f, buf, sizeof(buf) - 1, &got, nullptr);
    CloseHandle(f);
    std::string s(buf, got);
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    for (char& c : s)
        if (c == '\n' || c == '\r') c = ' ';
    return std::wstring(s.begin(), s.end());
}

void StartRecording(RECT area, HWND window, std::wstring const& what) {
    // A window is recorded as its area, here and by Screen Toolkit: what's drawn over it (the taskbar's visualizer)
    // is in it, and apps drawn on the graphics card don't come out black, as they do when ffmpeg reads the window.
    window = nullptr;
    if (ToolkitState() != L"off") return Delegate(area, window, what);
    std::wstring ffmpeg = FindFfmpeg();
    if (ffmpeg.empty()) {
        Log(L"no ffmpeg found: recording needs it (install it, or set capture.ffmpeg)");
        return Hint(L"Recording needs ffmpeg: install it, or set its path in Studio (Capture)", 4000);
    }

    // Inside the virtual screen, even sizes (h264 needs them).
    RECT v = VirtualScreen();
    IntersectRect(&area, &area, &v);
    int w = (area.right - area.left) & ~1, h = (area.bottom - area.top) & ~1;
    if (w < 16 || h < 16) return Hint(L"Nothing to record there");
    area.right = area.left + w, area.bottom = area.top + h;
    std::wstring path = FolderFor(true) + L"\\Tourne " + Stamp() + L".mp4";
    // Capture's codec, encoder and quality.
    std::wstring family = EncoderFamily(), q = std::to_wstring(g_cfg.quality);
    std::wstring codecName = g_cfg.codec == L"av1" ? L"av1" : g_cfg.codec == L"h264" ? L"h264" : L"hevc";
    std::wstring codec;
    if (family == L"nvidia") codec = L"-c:v " + codecName + L"_nvenc -preset p4 -rc vbr -cq " + q + L" -b:v 0";
    else if (family == L"amd") codec = L"-c:v " + codecName + L"_amf -rc cqp -qp_i " + q + L" -qp_p " + q;
    else if (family == L"intel") codec = L"-c:v " + codecName + L"_qsv -global_quality " + q;
    else codec = codecName == L"av1" ? L"-c:v libsvtav1 -preset 8 -crf " + q : codecName == L"h264" ? L"-c:v libx264 -preset veryfast -crf " + q
                                                                                    : L"-c:v libx265 -preset veryfast -crf " + q;
    if (codecName == L"hevc") codec += L" -tag:v hvc1";
    std::wstring fps = std::to_wstring(g_cfg.fps), mouse = g_cfg.cursor ? L"1" : L"0";
    // Desktop Duplication when it's asked for, ffmpeg has it and the area is on one screen: the graphics card's
    // copy of the screen, brought down for the encoder. Else GDI. Either way the area as it's seen (a window too:
    // its area, so what Tourne draws over it is in it, and apps drawn on the graphics card don't come out black).
    int output = -1;
    POINT origin{};
    bool dda = g_cfg.method == L"dda" && g_dda == 1 && DdaOutput(area, &output, &origin);
    std::wstring filters = dda ? L"hwdownload,format=bgra" : L"";
    if (g_cfg.height > 0 && g_cfg.height < h) filters += (filters.empty() ? L"" : L",") + std::wstring(L"scale=-2:") + std::to_wstring(g_cfg.height);
    std::wstring input = dda ? L"-f lavfi -i ddagrab=output_idx=" + std::to_wstring(output) + L":framerate=" + fps + L":offset_x=" +
                                   std::to_wstring(area.left - origin.x) + L":offset_y=" + std::to_wstring(area.top - origin.y) +
                                   L":video_size=" + std::to_wstring(w) + L"x" + std::to_wstring(h) + L":draw_mouse=" + mouse
                             : L"-f gdigrab -framerate " + fps + L" -draw_mouse " + mouse + L" -offset_x " + std::to_wstring(area.left) +
                                   L" -offset_y " + std::to_wstring(area.top) + L" -video_size " + std::to_wstring(w) + L"x" +
                                   std::to_wstring(h) + L" -i desktop";
    // The PC's sound (Tourne's own loopback, as Screen Toolkit gets it) and a microphone, mixed.
    std::wstring audio;
    int inputs = 0;
    RecorderSoundTap sound;
    if (g_cfg.sound) {
        if (RecorderSound(g_cfg.soundDevice, &sound)) {
            audio += L" -thread_queue_size 1024 -f " + sound.format + L" -ar " + std::to_wstring(sound.rate) + L" -ac " +
                     std::to_wstring(sound.channels) + L" -i \"" + sound.input + L"\"";
            inputs++;
        } else {
            Log(L"recording without the PC's sound: %ls", sound.why.c_str());
        }
    }
    std::wstring mic = g_cfg.micDevice;
    if (mic.empty()) {
        std::lock_guard lock(g_linkLock);
        mic = g_firstMic;
    }
    if (g_cfg.mic && !mic.empty()) {
        audio += L" -thread_queue_size 1024 -f dshow -i \"audio=" + mic + L"\"";
        inputs++;
    } else if (g_cfg.mic) {
        Log(L"recording without a microphone: ffmpeg lists none");
    }
    if (inputs == 2) audio += L" -filter_complex [1:a][2:a]amix=inputs=2:duration=longest:normalize=0[a] -map 0:v -map [a]";
    else if (inputs == 1) audio += L" -map 0:v -map 1:a";
    std::wstring args = L"-hide_banner -loglevel error -y " + input + audio + (filters.empty() ? L"" : L" -vf " + filters) + L" " +
                        codec + L" -pix_fmt yuv420p" + (inputs ? L" -c:a aac -b:a 160k" : L"") + L" -movflags +faststart \"" + path + L"\"";
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE read = nullptr, write = nullptr;
    if (!CreatePipe(&read, &write, &sa, 0)) return;
    SetHandleInformation(write, HANDLE_FLAG_INHERIT, 0);
    HANDLE said = CreateFileW(FfmpegLog().c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    STARTUPINFOW si{sizeof(si)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = read;
    si.hStdError = said != INVALID_HANDLE_VALUE ? said : nullptr;
    PROCESS_INFORMATION pi{};
    std::wstring line = L"\"" + ffmpeg + L"\" " + args;
    BOOL started = CreateProcessW(nullptr, line.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS, nullptr,
                                  nullptr, &si, &pi);
    if (said != INVALID_HANDLE_VALUE) CloseHandle(said);
    if (!started) {
        CloseHandle(read);
        CloseHandle(write);
        if (sound.stop) sound.stop();
        return Hint(L"ffmpeg didn't start");
    }
    CloseHandle(read);
    CloseHandle(pi.hThread);
    g_rec = {pi.hProcess, write, path, what, GetTickCount64(), area, sound.stop};
    Log(L"recording %ls (%dx%d at %d,%d, %ls, %ls, %ls) to %ls", what.c_str(), w, h, (int)area.left, (int)area.top,
        dda ? L"Desktop Duplication" : L"GDI", EncoderFamily().c_str(),
        inputs == 2 ? L"PC sound and microphone" : inputs ? (sound.stop ? L"PC sound" : L"microphone") : L"no sound", path.c_str());
    ShowDot(area);
}

// Told to stop: ffmpeg finishes the file on a thread of its own (the keyboard hook lives on this one, and can't
// wait), then Recorded has it.
void StopRecording() {
    if (!g_rec.process) return;
    HideDot();
    DWORD wrote = 0;
    WriteFile(g_rec.input, "q", 1, &wrote, nullptr);  // (ffmpeg finishes the file and ends)
    CloseHandle(g_rec.input);
    auto* done = new Recording(g_rec);
    g_rec = {};
    Hint(L"Saving the recording...", 4000);
    HWND to = g_wnd;
    std::thread([done, to] {
        if (WaitForSingleObject(done->process, 15000) != WAIT_OBJECT_0) TerminateProcess(done->process, 1);
        CloseHandle(done->process);
        if (done->soundStop) done->soundStop();
        if (!to || !PostMessageW(to, kMsgRecorded, 0, (LPARAM)done)) delete done;
    }).detach();
}

void Recorded(Recording const& done) {
    WIN32_FILE_ATTRIBUTE_DATA fa{};
    if (!GetFileAttributesExW(done.path.c_str(), GetFileExInfoStandard, &fa) || (!fa.nFileSizeHigh && fa.nFileSizeLow < 1024)) {
        std::wstring said = FfmpegSaid();
        Log(L"recording didn't come out (%ls): %ls", done.path.c_str(), said.empty() ? L"ffmpeg said nothing" : said.c_str());
        DeleteFileW(done.path.c_str());
        return Hint(L"The recording didn't come out (the reason is in Tourne's log)", 3500);
    }
    FileOut(done.path);
    ULONGLONG s = (GetTickCount64() - done.started) / 1000;
    Log(L"recorded %llus of %ls: copied, kept as %ls", s, done.what.c_str(), done.path.c_str());
    Hint(L"Copied: the recording (Ctrl+V)", 2000);
}

// Held: what to record, on a bar at the top of the screen under the pointer (as Snipping Tool's): each screen (left
// to right), every screen, a window to click, and close. A click, the arrows and Enter, or 1 to 9, A, W; Esc, Print
// Screen or a click elsewhere gives up.
struct BarItem {
    const wchar_t* glyph;
    std::wstring label, what;
    int kind;  // 0 a screen, 1 every screen, 2 a window, 3 close
    RECT area, hit;
};
HWND g_bar = nullptr;
std::vector<BarItem> g_barItems;
int g_barHot = -1, g_barSel = 0;

void EndBar() {
    if (!g_bar) return;
    HWND b = g_bar;
    g_bar = nullptr;
    DestroyWindow(b);
    WatchPointer();
}

void BarChoose(int i) {
    if (i < 0 || i >= (int)g_barItems.size()) return;
    BarItem item = g_barItems[i];
    EndBar();
    if (item.kind <= 1) StartRecording(item.area, nullptr, item.what);
    else if (item.kind == 2) StartPick(PickFor::Recording);
}

int BarHit(POINT pt) {
    for (size_t i = 0; i < g_barItems.size(); i++)
        if (PtInRect(&g_barItems[i].hit, pt)) return (int)i;
    return -1;
}

void PaintBar(HWND wnd, HDC dc) {
    RECT rc;
    GetClientRect(wnd, &rc);
    int dpi = GetDpiForWindow(wnd);
    auto S = [dpi](int v) { return MulDiv(v, dpi, 96); };
    Look look = Themed();
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(mem, bmp);
    HBRUSH ground = CreateSolidBrush(look.ground), raised = CreateSolidBrush(look.raised), accent = CreateSolidBrush(look.accent),
           red = CreateSolidBrush(RGB(232, 64, 64)), line = CreateSolidBrush(look.line);
    FillRect(mem, &rc, ground);
    HFONT text = UiFont(S(14)), icons = UiFont(S(16), true);
    SetBkMode(mem, TRANSPARENT);
    HGDIOBJ oldPen = SelectObject(mem, GetStockObject(NULL_PEN));
    HGDIOBJ oldBrush = SelectObject(mem, red);
    HGDIOBJ oldFont = SelectObject(mem, text);
    int d = S(10), mid = rc.bottom / 2;
    Ellipse(mem, S(14), mid - d / 2, S(14) + d, mid - d / 2 + d);  // (the record badge)
    for (size_t i = 0; i < g_barItems.size(); i++) {
        BarItem const& it = g_barItems[i];
        RECT r = it.hit;
        bool hot = (int)i == g_barHot, sel = (int)i == g_barSel;
        if (it.kind == 3) {
            RECT sep{r.left - S(5), r.top + S(8), r.left - S(4), r.bottom - S(8)};
            FillRect(mem, &sep, line);
        }
        if (hot || sel) {
            SelectObject(mem, raised);
            RoundRect(mem, r.left, r.top, r.right + 1, r.bottom + 1, S(8), S(8));
        }
        if (sel && it.kind != 3) {
            int cx = (r.left + r.right) / 2;
            SelectObject(mem, accent);
            RoundRect(mem, cx - S(8), r.bottom - S(3), cx + S(8) + 1, r.bottom + 1, S(3), S(3));
        }
        SetTextColor(mem, look.text);
        SelectObject(mem, icons);
        RECT g = r;
        if (it.label.empty()) {
            DrawTextW(mem, it.glyph, -1, &g, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            continue;
        }
        g.left += S(10);
        g.right = g.left + S(18);
        DrawTextW(mem, it.glyph, -1, &g, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(mem, text);
        RECT t = r;
        t.left = g.right + S(8);
        DrawTextW(mem, it.label.c_str(), -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    SelectObject(mem, oldFont);
    SelectObject(mem, oldBrush);
    SelectObject(mem, oldPen);
    BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    for (HGDIOBJ o : {(HGDIOBJ)bmp, (HGDIOBJ)ground, (HGDIOBJ)raised, (HGDIOBJ)accent, (HGDIOBJ)red, (HGDIOBJ)line, (HGDIOBJ)text,
                      (HGDIOBJ)icons})
        DeleteObject(o);
    DeleteDC(mem);
}

LRESULT CALLBACK BarProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PaintBar(wnd, dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_MOUSEMOVE: {
            int hot = BarHit({(short)LOWORD(lp), (short)HIWORD(lp)});
            if (hot != g_barHot) {
                g_barHot = hot;
                if (hot >= 0) g_barSel = hot;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            TRACKMOUSEEVENT t{sizeof(t), TME_LEAVE, wnd, 0};
            TrackMouseEvent(&t);
            return 0;
        }
        case WM_MOUSELEAVE:
            g_barHot = -1;
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONUP: {
            int at = BarHit({(short)LOWORD(lp), (short)HIWORD(lp)});
            if (at >= 0) {
                if (g_barItems[at].kind == 3) EndBar();
                else BarChoose(at);
            }
            return 0;
        }
        case WM_RBUTTONUP:
            EndBar();
            return 0;
        case WM_KEYDOWN: {
            int n = (int)g_barItems.size();
            if (wp == VK_ESCAPE) {
                EndBar();
            } else if (wp == VK_LEFT || wp == VK_RIGHT) {
                g_barSel = (g_barSel + (wp == VK_LEFT ? n - 2 : 1)) % (n - 1);  // (close isn't stepped onto)
                InvalidateRect(wnd, nullptr, FALSE);
            } else if (wp == VK_RETURN || wp == VK_SPACE) {
                if (g_barItems[g_barSel].kind == 3) EndBar();
                else BarChoose(g_barSel);
            } else if (wp >= '1' && wp <= '9') {
                int k = (int)(wp - '1');
                if (k < n && g_barItems[k].kind == 0) BarChoose(k);
            } else {
                for (int i = 0; i < n; i++)
                    if ((wp == 'A' && g_barItems[i].kind == 1) || (wp == 'W' && g_barItems[i].kind == 2)) {
                        BarChoose(i);
                        break;
                    }
            }
            return 0;
        }
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_CLOSE:
            if (wnd == g_bar) EndBar();
            return 0;
        case WM_SETCURSOR:
            SetCursor(LoadCursorW(nullptr, IDC_HAND));
            return TRUE;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void RecordBar() {
    EndBar();
    struct Screen {
        RECT r;
        bool main;
    };
    std::vector<Screen> screens;
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR m, HDC, LPRECT, LPARAM out) -> BOOL {
        MONITORINFO mi{sizeof(mi)};
        if (GetMonitorInfoW(m, &mi)) ((std::vector<Screen>*)out)->push_back({mi.rcMonitor, (mi.dwFlags & MONITORINFOF_PRIMARY) != 0});
        return TRUE;
    }, (LPARAM)&screens);
    std::sort(screens.begin(), screens.end(), [](Screen const& a, Screen const& b) { return a.r.left < b.r.left; });
    RECT under = ScreenUnderPointer();
    g_barItems.clear();
    g_barSel = 0;
    g_barHot = -1;
    for (size_t i = 0; i < screens.size(); i++) {
        std::wstring n = std::to_wstring(i + 1);
        g_barItems.push_back({L"", screens.size() > 1 ? L"Screen " + n : L"Screen", L"screen " + n, 0, screens[i].r, {}});
        if (EqualRect(&screens[i].r, &under)) g_barSel = (int)i;
    }
    if (screens.size() > 1) {
        RECT all{GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0, 0};
        all.right = all.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
        all.bottom = all.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
        g_barItems.push_back({L"", L"All screens", L"every screen", 1, all, {}});
    }
    g_barItems.push_back({L"", L"Window", L"a window", 2, {}, {}});
    g_barItems.push_back({L"", L"", L"", 3, {}, {}});

    g_bar = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"TourneCaptureBar", L"Record", WS_POPUP, under.left + 8, under.top + 8, 10,
                            10, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    SetWindowDisplayAffinity(g_bar, WDA_EXCLUDEFROMCAPTURE);
    int dpi = GetDpiForWindow(g_bar);
    auto S = [dpi](int v) { return MulDiv(v, dpi, 96); };
    HDC dc = GetDC(g_bar);
    HFONT font = UiFont(S(14));
    HGDIOBJ old = SelectObject(dc, font);
    int pad = S(6), h = S(40), x = S(34);  // (after the record badge)
    for (auto& it : g_barItems) {
        int w = S(40);
        if (!it.label.empty()) {
            SIZE size{};
            GetTextExtentPoint32W(dc, it.label.c_str(), (int)it.label.size(), &size);
            w = S(10) + S(18) + S(8) + size.cx + S(14);
        }
        if (it.kind == 3) x += S(8);
        it.hit = {x, pad, x + w, pad + h};
        x += w + S(2);
    }
    SelectObject(dc, old);
    DeleteObject(font);
    ReleaseDC(g_bar, dc);
    int bw = x - S(2) + pad, bh = h + 2 * pad;
    Rounded(g_bar, Themed().line);
    SetWindowPos(g_bar, HWND_TOPMOST, (under.left + under.right - bw) / 2, under.top + S(12), bw, bh, SWP_SHOWWINDOW | SWP_NOACTIVATE);
    WatchPointer();
}

// The pointer and keys while the picker or the bar is up (see kMsgPointer). The hook only posts; a click is kept
// from whatever is under it (a menu would close), and so are the keys the picker and the bar use.
HHOOK g_mouseHook = nullptr;
LRESULT CALLBACK PointerProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && (g_pick || g_bar)) {
        auto* m = (MSLLHOOKSTRUCT*)lp;
        if (!(m->flags & LLMHF_INJECTED)) {
            switch (wp) {
                case WM_MOUSEMOVE:
                    PostMessageW(g_wnd, kMsgPointer, wp, MAKELPARAM((short)m->pt.x, (short)m->pt.y));
                    break;
                case WM_LBUTTONDOWN: case WM_RBUTTONDOWN: case WM_MBUTTONDOWN:
                    return 1;
                case WM_LBUTTONUP: case WM_RBUTTONUP: case WM_MBUTTONUP:
                    PostMessageW(g_wnd, kMsgPointer, wp, MAKELPARAM((short)m->pt.x, (short)m->pt.y));
                    return 1;
            }
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

void WatchPointer() {
    bool want = g_pick || g_bar;
    if (want && !g_mouseHook) g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, PointerProc, GetModuleHandleW(nullptr), 0);
    else if (!want && g_mouseHook) UnhookWindowsHookEx(g_mouseHook), g_mouseHook = nullptr;
}

// What the hooks posted, handed to the picker or the bar as its own messages.
void PointerTo(UINT msg, POINT pt) {
    if (g_bar) {
        RECT r;
        GetWindowRect(g_bar, &r);
        if (!PtInRect(&r, pt)) {
            if (msg == WM_MOUSEMOVE) {
                if (g_barHot != -1) g_barHot = -1, InvalidateRect(g_bar, nullptr, FALSE);
            } else {
                EndBar();  // (a click elsewhere: never mind)
            }
            return;
        }
        ScreenToClient(g_bar, &pt);
        if (msg == WM_MBUTTONUP) return;
        SendMessageW(g_bar, msg, 0, MAKELPARAM(pt.x, pt.y));
        return;
    }
    if (g_pick) {
        ScreenToClient(g_pick, &pt);
        if (msg == WM_MBUTTONUP) return;
        SendMessageW(g_pick, msg, 0, MAKELPARAM(pt.x, pt.y));
    }
}

////////////////////////////////////////////////////////////////////////////////
// The key.

LRESULT CALLBACK KeyProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && g_on.load(std::memory_order_relaxed)) {
        auto* k = (KBDLLHOOKSTRUCT*)lp;
        // The picker's and the bar's keys (they never have the focus).
        if ((g_pick || g_bar) && !(k->flags & LLKHF_INJECTED)) {
            DWORD vk = k->vkCode;
            bool used = vk == VK_ESCAPE || vk == VK_RETURN ||
                        (g_bar && (vk == VK_SPACE || vk == VK_LEFT || vk == VK_RIGHT || (vk >= '1' && vk <= '9') || vk == 'A' || vk == 'W'));
            if (used) {
                if (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN) PostMessageW(g_wnd, kMsgKey, vk, 0);
                return 1;
            }
        }
        if (k->vkCode == g_keyVk.load(std::memory_order_relaxed) && !(k->flags & LLKHF_INJECTED)) {
            UINT mods = 0;
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) mods |= MOD_CONTROL;
            if (GetAsyncKeyState(VK_MENU) & 0x8000) mods |= MOD_ALT;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) mods |= MOD_SHIFT;
            if ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000) mods |= MOD_WIN;
            bool down = wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN;
            // Down with exactly the modifiers set; its release is ours whatever is held by then.
            if (down ? mods == g_keyMods.load(std::memory_order_relaxed) : g_keyTaken) {
                g_keyTaken = down;
                PostMessageW(g_wnd, down ? kMsgDown : kMsgUp, 0, 0);
                return 1;  // (Snipping Tool, and anything else on the key, never sees it)
            }
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

void KeyDown(bool fromDot) {
    if (g_keyDown && !fromDot) return;  // (its repeats while held)
    if (!fromDot) g_keyDown = true;
    if (g_rec.process || g_delegated) {  // recording: the key stops it
        g_consumed = !fromDot;
        if (g_delegated) {
            g_delegated = false;
            HideDot();
            SendToolkit({L"recordStop"});
            Hint(L"Saving the recording...", 2500);
        } else {
            StopRecording();
        }
        return;
    }
    if (fromDot) return;
    if (g_pick || g_bar) {  // picking: the key again gives up
        g_consumed = true;
        EndPick();
        EndBar();
        return;
    }
    if (g_tapPending) {  // a second tap in time: a window
        KillTimer(g_wnd, kTimerTap);
        g_tapPending = false;
        g_consumed = true;
        StartPick(PickFor::Picture);
        return;
    }
    g_holdFired = false;
    SetTimer(g_wnd, kTimerHold, g_cfg.holdMs, nullptr);
}

void KeyUp() {
    g_keyDown = false;
    if (g_consumed) {
        g_consumed = false;
        return;
    }
    KillTimer(g_wnd, kTimerHold);
    if (g_holdFired) {
        g_holdFired = false;
        return;
    }
    g_tapPending = true;  // (one tap: unless a second comes in time)
    SetTimer(g_wnd, kTimerTap, g_cfg.doubleTapMs, nullptr);
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case kMsgDown:
            KeyDown(wp == 1);
            return 0;
        case kMsgPointer: {
            // (the cursor as this thread sees it, in the same pixels as the windows' places)
            POINT pt;
            GetCursorPos(&pt);
            PointerTo((UINT)wp, pt);
            return 0;
        }
        case kMsgKey:
            if (g_bar) SendMessageW(g_bar, WM_KEYDOWN, wp, 0);
            else if (g_pick) SendMessageW(g_pick, WM_KEYDOWN, wp, 0);
            return 0;
        case kMsgUp:
            KeyUp();
            return 0;
        case kMsgRecorded: {
            std::unique_ptr<Recording> done((Recording*)lp);
            Recorded(*done);
            return 0;
        }
        case kMsgToolkit:
            ToolkitChanged();
            return 0;
        case kMsgApply:
            g_cfg = ReadConfig(), g_keyVk = g_cfg.keyVk, g_keyMods = g_cfg.keyMods;
            g_on = g_cfg.on;
            return 0;
        case WM_TIMER:
            KillTimer(wnd, wp == kTimerRec ? 0 : wp);
            if (wp == kTimerHold) {
                g_holdFired = true;
                RecordBar();
            } else if (wp == kTimerTap) {
                g_tapPending = false;
                PictureOut(GrabScreenRect(ScreenUnderPointer()), L"the screen");
            } else if (wp == kTimerHint) {
                if (g_hint) ShowWindow(g_hint, SW_HIDE);
            } else if (wp == kTimerRec && g_rec.process && WaitForSingleObject(g_rec.process, 0) == WAIT_OBJECT_0) {
                Log(L"ffmpeg ended on its own while recording");
                StopRecording();  // (the dot goes, and what it left is looked at)
            } else if (wp == kTimerRec && g_dot) {
                InvalidateRect(g_dot, nullptr, FALSE);
            } else if (wp == kTimerDelegate && g_pending) {
                g_pending = false;
                Log(L"Screen Toolkit didn't start the recording");
                Hint(L"Screen Toolkit didn't start the recording", 3000);
            }
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void Thread() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (g_nvenc < 0)
        std::thread([] {
            std::wstring ffmpeg = FindFfmpeg();
            g_nvenc = !ffmpeg.empty() && RunFor(ffmpeg, L"-hide_banner -encoders").find("h264_nvenc") != std::string::npos ? 1 : 0;
            g_dda = !ffmpeg.empty() && RunFor(ffmpeg, L"-hide_banner -filters").find("ddagrab") != std::string::npos ? 1 : 0;
            // The first microphone, as DirectShow names it: [dshow @ ...] "Microphone (Realtek Audio)" (audio)
            std::string list = ffmpeg.empty() ? "" : RunFor(ffmpeg, L"-hide_banner -list_devices true -f dshow -i dummy");
            for (size_t at = 0; (at = list.find("(audio)", at)) != std::string::npos; at++) {
                size_t end = list.rfind('"', at), start = end == std::string::npos ? end : list.rfind('"', end - 1);
                size_t line = list.rfind('\n', at);
                if (start == std::string::npos || (line != std::string::npos && start < line)) continue;
                std::string name = list.substr(start + 1, end - start - 1);
                int n = MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), nullptr, 0);
                std::wstring wide(n, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wide.data(), n);
                std::lock_guard lock(g_linkLock);
                g_firstMic = wide;
                break;
            }
        }).detach();
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HINSTANCE inst = GetModuleHandleW(nullptr);
    auto reg = [&](const wchar_t* name, WNDPROC proc) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = proc;
        wc.hInstance = inst;
        wc.lpszClassName = name;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        RegisterClassW(&wc);
    };
    reg(L"TourneCapture", WndProc);
    reg(L"TourneCaptureHint", HintProc);
    reg(L"TourneCapturePick", PickProc);
    reg(L"TourneCaptureDot", DotProc);
    reg(L"TourneCaptureBar", BarProc);
    // (a real window, never shown: the recording menu needs one in front to close as menus do)
    g_wnd = CreateWindowExW(WS_EX_TOOLWINDOW, L"TourneCapture", L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, inst, nullptr);
    // The key's hook on a thread of its own that does nothing else: Windows drops a keyboard hook, without a
    // word, whose thread is once too slow to answer (this one is busy taking and saving pictures), and then
    // Print Screen goes to Snipping Tool. Taken again each minute too, so it's first in line again.
    std::thread([inst] {
        g_hookThread = GetCurrentThreadId();
        g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyProc, inst, 0);
        Log(g_hook ? L"on: Print Screen is Tourne's (tap, double tap, hold)" : L"couldn't watch the keyboard (error %lu)", GetLastError());
        SetTimer(nullptr, 0, 60000, nullptr);
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) {
            if (m.message != WM_TIMER) continue;
            if (HHOOK again = SetWindowsHookExW(WH_KEYBOARD_LL, KeyProc, inst, 0)) {
                if (g_hook) UnhookWindowsHookEx(g_hook);
                g_hook = again;
            }
        }
        if (g_hook) UnhookWindowsHookEx(g_hook);
        g_hook = nullptr;
    }).detach();
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    StopRecording();
    EndPick();
    EndBar();
    if (g_hookThread) PostThreadMessageW(g_hookThread, WM_QUIT, 0, 0);  // (the hook goes with its thread)
    g_hookThread = 0;
    for (HWND* w : {&g_hint, &g_dot}) {
        if (*w) DestroyWindow(*w);
        *w = nullptr;
    }
    DestroyWindow(g_wnd);
    g_wnd = nullptr;
    CoUninitialize();
}

}  // namespace

void CaptureDefaults(Settings& out) {
    out[L"capture.enabled"] = L"1";
    out[L"capture.holdMs"] = L"600";
    out[L"capture.hotkey"] = L"";
    out[L"capture.doubleTapMs"] = L"300";
    out[L"capture.fps"] = L"60";
    out[L"capture.encoder"] = L"auto";
    out[L"capture.codec"] = L"hevc";
    out[L"capture.quality"] = L"23";
    out[L"capture.resolution"] = L"0";
    out[L"capture.method"] = L"dda";
    out[L"capture.sound"] = L"1";
    out[L"capture.soundDevice"] = L"default";
    out[L"capture.mic"] = L"0";
    out[L"capture.micDevice"] = L"";
    out[L"capture.after"] = L"none";
    out[L"capture.afterCopy"] = L"1";
    out[L"capture.annotate"] = L"0";
    out[L"capture.cursor"] = L"1";
    out[L"capture.pictures"] = L"";
    out[L"capture.videos"] = L"";
    out[L"capture.ffmpeg"] = L"";
}

void CaptureDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"capture", L"Capture",
        L"Print Screen, Tourne's way, in place of Snipping Tool: tap it for the screen under the pointer, tap twice quickly to "
        L"click a window, hold it to record (a screen, every screen or a window; Print Screen again stops). Each one is on the "
        L"clipboard at once for Ctrl+V, a recording as its file, and kept in your Pictures or Videos. With Ctrl, Alt, Shift or "
        L"Windows held, Print Screen does what it did before.");
    add(L"capture.enabled", L"Print Screen captures", L"Off: Print Screen is left to Windows (Snipping Tool).").isSwitch = true;
    add(L"capture.hotkey", L"Capture key",
        L"The key that does all this, in place of Print Screen: tap, double tap, hold. With modifiers it needs them held "
        L"(Ctrl+F12). None: Print Screen alone.").format = L"hotkey";
    SettingInfo& h = add(L"capture.holdMs", L"Hold to record (ms)", L"How long Print Screen is held before the recording menu opens.");
    h.hasMin = h.hasMax = true, h.min = 250, h.max = 2000;
    SettingInfo& d = add(L"capture.doubleTapMs", L"Double tap within (ms)", L"Two taps closer than this pick a window; one tap waits this long.");
    d.hasMin = d.hasMax = true, d.min = 150, d.max = 800;
    SettingInfo& f = add(L"capture.fps", L"Recording frame rate", L"Frames a second, 10 to 240.");
    f.hasMin = f.hasMax = true, f.min = 10, f.max = 240;
    add(L"capture.encoder", L"Encoder",
        L"What makes the video. The graphics card's are quick and leave the processor alone; the processor's are slower and "
        L"a little smaller for the same quality. Automatic: NVIDIA here when ffmpeg has it.")
        .options = {{L"auto", L"Automatic"}, {L"nvidia", L"NVIDIA (NVENC)"}, {L"amd", L"AMD (AMF)"}, {L"intel", L"Intel (Quick Sync)"},
                    {L"cpu", L"Processor (x264, x265, SVT-AV1)"}};
    add(L"capture.codec", L"Codec",
        L"H.265 (HEVC) is about half H.264's size at the same quality; AV1 is smaller still (RTX 40 and 50 cards make it "
        L"in hardware) but older apps may not play it. H.264 plays everywhere.")
        .options = {{L"hevc", L"H.265 / HEVC"}, {L"av1", L"AV1"}, {L"h264", L"H.264"}};
    SettingInfo& q = add(L"capture.quality", L"Quality",
                         L"Constant quality, 10 to 45: lower is better and bigger. 18 looks like the screen, 23 is very good, 28 is "
                         L"small, 33 and up shows.");
    q.hasMin = q.hasMax = true, q.min = 10, q.max = 45;
    add(L"capture.resolution", L"Resolution", L"Recordings scaled down to this height (never up).")
        .options = {{L"0", L"As the screen"}, {L"2160", L"2160p"}, {L"1440", L"1440p"}, {L"1080", L"1080p"}, {L"720", L"720p"}};
    add(L"capture.method", L"Capture",
        L"Desktop Duplication copies the screen on the graphics card and keeps up at high frame rates; GDI is the old way, "
        L"slow past 30 to 60 fps. An area across two screens, or on a second graphics card, is always GDI. A window is "
        L"recorded as its area, as it's seen, so what Tourne draws over it (the taskbar's visualizer) is in it.")
        .options = {{L"dda", L"Desktop Duplication (ddagrab)"}, {L"gdi", L"GDI (gdigrab)"}};
    add(L"capture.cursor", L"Pointer in recordings", L"").isSwitch = true;
    add(L"capture.sound", L"Record the PC's sound", L"What you hear, from the output below.").isSwitch = true;
    std::vector<std::pair<std::wstring, std::wstring>> outputs{{L"default", L"Windows' default output"}}, mics{{L"", L"The first ffmpeg finds"}};
    // The sound devices now, by name (an output by its id, which the recorder takes; a microphone by the name ffmpeg lists).
    // (on a thread of its own, so the caller's COM is left as it is)
    std::thread([&] {
        HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        IMMDeviceEnumerator* devices = nullptr;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&devices)))) {
            for (EDataFlow flow : {eRender, eCapture}) {
                IMMDeviceCollection* list = nullptr;
                if (FAILED(devices->EnumAudioEndpoints(flow, DEVICE_STATE_ACTIVE, &list))) continue;
                UINT n = 0;
                list->GetCount(&n);
                for (UINT i = 0; i < n; i++) {
                    IMMDevice* d = nullptr;
                    if (FAILED(list->Item(i, &d))) continue;
                    LPWSTR id = nullptr;
                    IPropertyStore* props = nullptr;
                    PROPVARIANT name;
                    PropVariantInit(&name);
                    static const PROPERTYKEY kFriendlyName{{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};
                    if (SUCCEEDED(d->GetId(&id)) && SUCCEEDED(d->OpenPropertyStore(STGM_READ, &props)) &&
                        SUCCEEDED(props->GetValue(kFriendlyName, &name)) && name.vt == VT_LPWSTR) {
                        if (flow == eRender) outputs.push_back({id, name.pwszVal});
                        else mics.push_back({name.pwszVal, name.pwszVal});
                    }
                    PropVariantClear(&name);
                    if (props) props->Release();
                    CoTaskMemFree(id);
                    d->Release();
                }
                list->Release();
            }
            devices->Release();
        }
        if (SUCCEEDED(com)) CoUninitialize();
    }).join();
    add(L"capture.soundDevice", L"Sound from", L"The output whose sound is recorded.").options = outputs;
    add(L"capture.mic", L"Record a microphone", L"Mixed in with the PC's sound.").isSwitch = true;
    add(L"capture.micDevice", L"Microphone", L"").options = mics;
    add(L"capture.after", L"After it's copied",
        L"Once the recording is on the clipboard, also make a copy of it: smaller (AV1 on the graphics card, the best size "
        L"for the quality there is; HEVC for apps without AV1), sized for Discord's upload limits, or a GIF. Kept beside the "
        L"recording (needs Screen Toolkit running).")
        .options = {{L"none", L"Nothing more"}, {L"av1", L"A smaller copy (AV1)"}, {L"hevc", L"A smaller copy (HEVC)"},
                    {L"discord10", L"Under 10 MB (Discord)"}, {L"discord50", L"Under 50 MB (Discord Nitro Basic)"},
                    {L"discord500", L"Under 500 MB (Discord Nitro)"}, {L"gif", L"A GIF"}};
    add(L"capture.afterCopy", L"Copy that instead", L"When the copy is made, it replaces the recording on the clipboard.").isSwitch = true;
    add(L"capture.annotate", L"Mark up screenshots",
        L"Each screenshot opens over the screen first, to draw on: pen, arrow, box, highlighter, text, numbered markers and "
        L"redact (squares over what's private, the pixels themselves replaced). 1 to 7 pick a tool, C the colour, Ctrl+Z undoes, "
        L"Enter keeps it marked, Esc keeps it plain.").isSwitch = true;
    add(L"capture.pictures", L"Pictures folder", L"Empty: Pictures\\Screenshots.");
    add(L"capture.videos", L"Recordings folder", L"Empty: Videos\\Captures.");
    add(L"capture.ffmpeg", L"ffmpeg", L"Recording uses ffmpeg. Empty: the one installed, or the copy Convertify brings.");
}

void CaptureStart(HWND host) {
    g_host = host;
    if (g_thread.joinable()) return;
    g_cfg = ReadConfig(), g_keyVk = g_cfg.keyVk, g_keyMods = g_cfg.keyMods;
    g_on = g_cfg.on;
    g_thread = std::thread(Thread);
}

void CaptureApply() {
    if (g_wnd) PostMessageW(g_wnd, kMsgApply, 0, 0);
}

void CaptureToolkit(const std::wstring& state, const std::wstring& pictures) {
    {
        std::lock_guard lock(g_linkLock);
        g_tkState = state.empty() ? L"idle" : state;
        g_tkPictures = pictures;
        while (!g_tkPictures.empty() && (g_tkPictures.back() == L'\\' || g_tkPictures.back() == L'/')) g_tkPictures.pop_back();
    }
    if (g_wnd) PostMessageW(g_wnd, kMsgToolkit, 0, 0);
}

void CaptureStop() {
    g_on = false;
    if (!g_thread.joinable()) return;
    for (int i = 0; i < 100 && !g_wnd; i++) Sleep(10);
    if (g_wnd) PostMessageW(g_wnd, WM_QUIT, 0, 0);
    g_thread.join();
}

}  // namespace td
