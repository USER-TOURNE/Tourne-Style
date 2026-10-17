#include "glass.h"

#include <windows.h>
#include <d2d1.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <shlobj.h>
#include <wrl/client.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <map>
#include <memory>

#include "glass-engine.h"
#include "screen-effects.h"
#include "td.h"

using Microsoft::WRL::ComPtr;

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"glass", format, args);
    va_end(args);
}

////////////////////////////////////////////////////////////////////////////////
// Looks and settings

// The settings a look sets, and their limits (the Studio's sliders).
struct Param {
    const wchar_t* key;
    const wchar_t* name;
    const wchar_t* description;
    int min, max;
};
const Param kParams[] = {
    {L"blur", L"Blur", L"How soft the wallpaper behind is (pixels).", 0, 64},
    {L"refraction", L"Edge bend", L"How much the glass bends what's behind it at its edges.", 0, 100},
    {L"fringe", L"Rainbow fringe", L"Colours split apart where the edge bends, like a prism.", 0, 100},
    {L"edgeGlow", L"Edge glow", L"Light caught along the rim.", 0, 100},
    {L"highlight", L"Highlight", L"Light from above on the top edge, and a thin bright line round it.", 0, 100},
    {L"bevel", L"Bevel", L"How far in from the edge the glass curves (pixels).", 0, 48},
    {L"lens", L"Lens", L"The middle magnified, like a dome.", 0, 100},
    {L"tint", L"Tint", L"The glass's own colour. A theme role (Accent, Ground) follows the theme.", 0, 0},
    {L"tintAmount", L"Tint strength", L"0 is clear, 100 is solid tint.", 0, 100},
    {L"saturation", L"Saturation (%)", L"Of what's behind: 100 leaves it, 0 is grey, above 100 is richer.", 0, 200},
    {L"brightness", L"Brightness", L"Of what's behind: below 0 darker, above lighter.", -50, 50},
    {L"radius", L"Corners", L"Corner roundness of glass panels (pixels).", 0, 40},
    {L"shadow", L"Shadow", L"A soft shadow under glass panels.", 0, 100},
    {L"oilFilm", L"Oil film", L"Slow swirls of rainbow colour, like oil on water. It moves, so it costs a little while it shows; it rests a few seconds after things stop changing.", 0, 100},
};

const std::vector<GlassLookPreset> kLooks = {
    {L"clear", L"Clear",
     {{L"blur", L"4"}, {L"refraction", L"55"}, {L"fringe", L"30"}, {L"edgeGlow", L"40"}, {L"highlight", L"60"}, {L"bevel", L"18"},
      {L"lens", L"20"}, {L"tint", L"#FFFFFF"}, {L"tintAmount", L"6"}, {L"saturation", L"115"}, {L"brightness", L"0"},
      {L"radius", L"18"}, {L"shadow", L"45"}, {L"oilFilm", L"0"}}},
    {L"liquid", L"Liquid (Apple-ish)",
     {{L"blur", L"12"}, {L"refraction", L"45"}, {L"fringe", L"18"}, {L"edgeGlow", L"30"}, {L"highlight", L"75"}, {L"bevel", L"24"},
      {L"lens", L"35"}, {L"tint", L"#FFFFFF"}, {L"tintAmount", L"12"}, {L"saturation", L"140"}, {L"brightness", L"4"},
      {L"radius", L"24"}, {L"shadow", L"40"}, {L"oilFilm", L"0"}}},
    {L"frosted", L"Frosted",
     {{L"blur", L"36"}, {L"refraction", L"15"}, {L"fringe", L"5"}, {L"edgeGlow", L"22"}, {L"highlight", L"35"}, {L"bevel", L"10"},
      {L"lens", L"0"}, {L"tint", L"#1C2026"}, {L"tintAmount", L"28"}, {L"saturation", L"120"}, {L"brightness", L"0"},
      {L"radius", L"14"}, {L"shadow", L"50"}, {L"oilFilm", L"0"}}},
    {L"aero", L"Aero",
     {{L"blur", L"20"}, {L"refraction", L"10"}, {L"fringe", L"0"}, {L"edgeGlow", L"45"}, {L"highlight", L"85"}, {L"bevel", L"8"},
      {L"lens", L"0"}, {L"tint", L"#3A7BD5"}, {L"tintAmount", L"30"}, {L"saturation", L"100"}, {L"brightness", L"6"},
      {L"radius", L"8"}, {L"shadow", L"55"}, {L"oilFilm", L"0"}}},
    {L"smoke", L"Smoke",
     {{L"blur", L"28"}, {L"refraction", L"25"}, {L"fringe", L"8"}, {L"edgeGlow", L"20"}, {L"highlight", L"30"}, {L"bevel", L"14"},
      {L"lens", L"0"}, {L"tint", L"#0B0D10"}, {L"tintAmount", L"55"}, {L"saturation", L"90"}, {L"brightness", L"-4"},
      {L"radius", L"16"}, {L"shadow", L"60"}, {L"oilFilm", L"0"}}},
    {L"prism", L"Prism",
     {{L"blur", L"6"}, {L"refraction", L"70"}, {L"fringe", L"85"}, {L"edgeGlow", L"55"}, {L"highlight", L"65"}, {L"bevel", L"28"},
      {L"lens", L"25"}, {L"tint", L"#FFFFFF"}, {L"tintAmount", L"4"}, {L"saturation", L"130"}, {L"brightness", L"2"},
      {L"radius", L"22"}, {L"shadow", L"35"}, {L"oilFilm", L"0"}}},
    {L"oil", L"Oil film",
     {{L"blur", L"10"}, {L"refraction", L"45"}, {L"fringe", L"35"}, {L"edgeGlow", L"35"}, {L"highlight", L"70"}, {L"bevel", L"22"},
      {L"lens", L"30"}, {L"tint", L"#FFFFFF"}, {L"tintAmount", L"8"}, {L"saturation", L"130"}, {L"brightness", L"2"},
      {L"radius", L"22"}, {L"shadow", L"40"}, {L"oilFilm", L"60"}}},
};

// Apps given glass by default (each off until turned on).
struct AppDefault {
    const wchar_t* name;
    const wchar_t* exe;
    const wchar_t* backdrop;
    const wchar_t* opacity;
};
const AppDefault kApps[] = {
    {L"terminal", L"WindowsTerminal.exe", L"acrylic", L"100"},
    {L"explorer", L"explorer.exe", L"mica", L"100"},
    {L"notepad", L"Notepad.exe", L"mica", L"100"},
    {L"code", L"Code.exe", L"frosted", L"92"},
    {L"discord", L"Discord.exe", L"frosted", L"92"},
    {L"spotify", L"Spotify.exe", L"frosted", L"92"},
};

std::wstring Text(const std::wstring& key) {
    const wchar_t* v = GetString(L"glass", key);
    std::wstring out = v ? v : L"";
    FreeString(v);
    return out;
}

bool ParseColour(std::wstring v, uint32_t* out) {
    v.erase(0, v.find_first_not_of(L" \t#"));
    v.erase(v.find_last_not_of(L" \t") + 1);
    if (v.size() != 6 || v.find_first_not_of(L"0123456789abcdefABCDEF") != std::wstring::npos) return false;
    *out = (uint32_t)wcstoul(v.c_str(), nullptr, 16);
    return true;
}

// A look setting: yours (or the identity's) if set, else the look's.
std::wstring LookSetting(const std::wstring& key) {
    if (IsSetOrIdentity(L"glass", key)) return Text(key);
    std::wstring v = GlassLookValue(Text(L"look"), L"glass." + key);
    return v.empty() ? Text(key) : v;
}

float Number(const std::wstring& key) { return (float)_wtof(LookSetting(key).c_str()); }

GlassLook ReadLook() {
    GlassLook l;
    l.blur = Number(L"blur");
    l.refraction = Number(L"refraction") / 100;
    l.fringe = Number(L"fringe") / 100;
    l.edgeGlow = Number(L"edgeGlow") / 100;
    l.highlight = Number(L"highlight") / 100;
    l.bevel = Number(L"bevel");
    l.lens = Number(L"lens") / 100;
    if (!ParseColour(LookSetting(L"tint"), &l.tint)) l.tint = 0xFFFFFF;
    l.tintAmount = Number(L"tintAmount") / 100;
    l.saturation = Number(L"saturation") / 100;
    l.brightness = Number(L"brightness") / 100;
    l.radius = Number(L"radius");
    l.shadow = Number(L"shadow") / 100;
    l.oil = Number(L"oilFilm") / 100;
    l.effect = EffectByName(Text(L"effect"));
    l.effectAmount = Number(L"effectAmount") / 100;
    return l;
}

struct Wanted {
    bool on = false, studio = false, rims = false, clickGlow = true;
    int rimWidth = 6, rimRadius = 8;
    GlassLook look;
};

Wanted ReadWanted() {
    Wanted w;
    w.on = GetInt(L"glass", L"enabled") != 0;
    w.studio = w.on && GetInt(L"glass", L"studio") != 0;
    w.rims = w.on && GetInt(L"glass", L"rims.on") != 0;
    w.clickGlow = GetInt(L"glass", L"clickGlow") != 0;
    w.rimWidth = std::clamp(GetInt(L"glass", L"rims.width"), 2, 24);
    w.rimRadius = std::clamp(GetInt(L"glass", L"rims.radius"), 0, 24);
    w.look = ReadLook();
    return w;
}

////////////////////////////////////////////////////////////////////////////////
// State (the host's thread only)

constexpr UINT WM_TD_BACKDROP = WM_APP + 40;     // a backdrop was composed: lp = Backdrop*
constexpr UINT WM_TD_GLASS_FOLDER = WM_APP + 41;  // the wallpaper folder changed
constexpr UINT WM_TD_GLASS_EVENT = WM_APP + 42;   // a window event for the rims
constexpr UINT_PTR kTimerFrame = 40, kTimerBackdrop = 41;  // (the host parts each keep their own tens)

HWND g_host;
GlassEngine g_engine;
Wanted g_wanted;
bool g_compare = false;          // held: glass off
bool g_backdropBusy = false;
std::atomic<unsigned> g_backdropGen;
HANDLE g_folderThread, g_folderStop;
DWORD g_start = GetTickCount();
DWORD g_activeUntil = 0;          // animating (oil film, glow) until then
DWORD g_glowFrom = 0;             // the click glow started then
bool g_framePending;              // a frame is on its way
ComPtr<ID2D1Factory> g_d2d;
ComPtr<IDWriteFactory> g_dw;

float Seconds() { return (GetTickCount() - g_start) / 1000.f; }

bool Needed();
void RedrawAll();
void Wake(DWORD ms);

////////////////////////////////////////////////////////////////////////////////
// Layered windows: a premultiplied bitmap put on the screen as it is.

void Show(HWND wnd, const RECT& r, const std::vector<uint32_t>& pixels) {
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0 || pixels.size() < (size_t)w * h) return;
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    void* bits = nullptr;
    HDC screen = GetDC(nullptr), dc = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bmp && bits) {
        memcpy(bits, pixels.data(), (size_t)w * h * 4);
        HGDIOBJ old = SelectObject(dc, bmp);
        POINT dst{r.left, r.top}, src{0, 0};
        SIZE size{w, h};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(wnd, screen, &dst, &size, dc, &src, 0, &blend, ULW_ALPHA);
        SelectObject(dc, old);
    }
    if (bmp) DeleteObject(bmp);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
}

float ScaleOf(HWND wnd) { return (wnd ? GetDpiForWindow(wnd) : 96) / 96.f; }

////////////////////////////////////////////////////////////////////////////////
// The backdrop: composed on a thread (pictures take a moment to read).

DWORD WINAPI ComposeThread(LPVOID param) {
    unsigned gen = (unsigned)(UINT_PTR)param;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    auto b = std::make_unique<Backdrop>();
    bool ok = ComposeWallpaper(b.get());
    CoUninitialize();
    if (!ok) {  // (no picture to be had: a gradient the size of the desk)
        int w = std::max(GetSystemMetrics(SM_CXVIRTUALSCREEN), 1), h = std::max(GetSystemMetrics(SM_CYVIRTUALSCREEN), 1);
        float s = std::min(1.f, 2048.f / std::max(w, h));
        GradientBackdrop(std::max((int)(w * s), 1), std::max((int)(h * s), 1), b.get());
        b->scale = s;
        b->origin = {GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN)};
    }
    if (PostMessageW(g_host, WM_TD_BACKDROP, gen, (LPARAM)b.get())) b.release();
    return 0;
}

void ComposeSoon(UINT ms) {
    if (g_host) SetTimer(g_host, kTimerBackdrop, ms, nullptr);
}

void Compose() {
    if (!Needed()) return;
    HANDLE t = CreateThread(nullptr, 0, ComposeThread, (LPVOID)(UINT_PTR)++g_backdropGen, 0, nullptr);
    if (t) CloseHandle(t), g_backdropBusy = true;
}

std::wstring ThemesFolder() {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path))) out = std::wstring(path) + L"\\Microsoft\\Windows\\Themes";
    CoTaskMemFree(path);
    return out;
}

// Windows rewrites its copy of the wallpaper there on every change, slideshow
// steps included: a thread that sleeps until then.
DWORD WINAPI FolderThread(LPVOID) {
    HANDLE change = FindFirstChangeNotificationW(ThemesFolder().c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);
    if (change == INVALID_HANDLE_VALUE) return 0;
    HANDLE waits[2] = {change, g_folderStop};
    while (WaitForMultipleObjects(2, waits, FALSE, INFINITE) == WAIT_OBJECT_0) {
        PostMessageW(g_host, WM_TD_GLASS_FOLDER, 0, 0);
        if (!FindNextChangeNotification(change)) break;
    }
    FindCloseChangeNotification(change);
    return 0;
}

void WatchFolder(bool on) {
    if (on == (g_folderThread != nullptr)) return;
    if (on) {
        g_folderStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        g_folderThread = CreateThread(nullptr, 0, FolderThread, nullptr, 0, nullptr);
        return;
    }
    SetEvent(g_folderStop);
    WaitForSingleObject(g_folderThread, 2000);
    CloseHandle(g_folderThread);
    CloseHandle(g_folderStop);
    g_folderThread = g_folderStop = nullptr;
}

bool EngineReady() {
    if (!g_engine.Started() && !g_engine.Start()) {
        static bool told;
        if (!told) Log(L"Direct3D 11 isn't available: no glass"), told = true;
        return false;
    }
    if (!g_engine.HasBackdrop() && !g_backdropBusy) Compose();
    return g_engine.HasBackdrop();
}

////////////////////////////////////////////////////////////////////////////////
// The test window: a glass card to drag over the wallpaper.

constexpr wchar_t kTestClass[] = L"TourneGlass_Test", kRimClass[] = L"TourneGlass_Rim";
HWND g_test;
float g_testMargin = 28;  // room for the shadow (at 100%)

void DrawLabel(std::vector<uint32_t>& pixels, int w, int h, float scale, float margin) {
    if (!g_d2d && FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, g_d2d.GetAddressOf()))) return;
    if (!g_dw && FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)g_dw.GetAddressOf()))) return;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(bi.bmiHeader), w, -h, 1, 32};
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bmp || !bits) {
        DeleteDC(dc);
        return;
    }
    // The words on their own, then laid over the glass (drawing straight onto
    // it leans on the render target keeping what's there, which not all do).
    memset(bits, 0, (size_t)w * h * 4);
    HGDIOBJ old = SelectObject(dc, bmp);
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 96 * scale, 96 * scale);
    ComPtr<ID2D1DCRenderTarget> rt;
    if (SUCCEEDED(g_d2d->CreateDCRenderTarget(&props, &rt))) {
        RECT rc{0, 0, w, h};
        rt->BindDC(dc, &rc);
        rt->BeginDraw();
        rt->Clear(D2D1::ColorF(0, 0, 0, 0));
        ComPtr<ID2D1SolidColorBrush> brush;
        rt->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1, 0.92f), &brush);
        ComPtr<IDWriteTextFormat> big, small;
        g_dw->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 26,
                               L"", &big);
        g_dw->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 13,
                               L"", &small);
        float x = margin + 28, y = margin + 26, right = w / scale - margin - 20;
        std::wstring look = Text(L"look");
        for (auto const& l : kLooks)
            if (look == l.id) look = l.label;
        std::wstring title = L"Tourne glass", sub = look + L"\nDrag me over your wallpaper. Right-click or Esc to close.";
        if (big) rt->DrawText(title.c_str(), (UINT32)title.size(), big.Get(), {x, y, right, y + 40}, brush.Get());
        brush->SetColor(D2D1::ColorF(1, 1, 1, 0.75f));
        if (small) rt->DrawText(sub.c_str(), (UINT32)sub.size(), small.Get(), {x, y + 44, right, y + 120}, brush.Get());
        if (SUCCEEDED(rt->EndDraw())) {
            GdiFlush();
            std::vector<uint32_t> words((uint32_t*)bits, (uint32_t*)bits + (size_t)w * h);
            BlendOnto(pixels, w, h, words, w, h, 0, 0);
        }
    }
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
}

void DrawTest() {
    if (!g_test || !EngineReady()) return;
    RECT r;
    GetWindowRect(g_test, &r);
    float s = ScaleOf(g_test), m = g_testMargin * s;
    GlassShape shape;
    shape.view = r;
    shape.shape = {(LONG)(r.left + m), (LONG)(r.top + m), (LONG)(r.right - m), (LONG)(r.bottom - m)};
    shape.scale = s;
    shape.time = Seconds();
    GlassLook look = g_wanted.look;
    if (g_compare) look = GlassLook{0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, look.radius, 0, 0};
    std::vector<uint32_t> px;
    if (!g_engine.Render(look, shape, &px)) return;
    DrawLabel(px, r.right - r.left, r.bottom - r.top, s, g_testMargin);
    Show(g_test, r, px);
}

LRESULT CALLBACK TestProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_NCHITTEST: return HTCAPTION;
        case WM_NCRBUTTONUP:
        case WM_RBUTTONUP: DestroyWindow(wnd); return 0;
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) DestroyWindow(wnd);
            return 0;
        case WM_WINDOWPOSCHANGED:
            // Moved: what's behind changed.
            Wake(4000);
            break;
        case WM_DPICHANGED: {
            auto* r = (RECT*)lp;
            SetWindowPos(wnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_DESTROY:
            g_test = nullptr;
            if (!Needed()) GlassApply();
            StudioGlassChanged();
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////
// Rims: glass round the window in front, in four strips (so a big window
// costs no more than its edges).

struct Rim {
    HWND target = nullptr;
    HWND strips[4]{};
    RECT frame{};       // the target's visible frame
    bool shown = false;
    HWINEVENTHOOK local[3]{};  // the target's own process
    DWORD process = 0;
} g_rim;
HWINEVENTHOOK g_foregroundHook;

bool Eligible(HWND w) {
    if (!w || !IsWindowVisible(w) || IsIconic(w) || IsZoomed(w)) return false;
    if (GetAncestor(w, GA_ROOT) != w) return false;
    LONG_PTR ex = GetWindowLongPtrW(w, GWL_EXSTYLE), style = GetWindowLongPtrW(w, GWL_STYLE);
    if ((ex & WS_EX_TOOLWINDOW) || (ex & WS_EX_NOACTIVATE)) return false;
    if ((style & WS_CAPTION) != WS_CAPTION && !(style & WS_THICKFRAME)) return false;  // (popups, menus, splash screens)
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (pid == GetCurrentProcessId()) return false;
    wchar_t cls[64] = L"";
    GetClassNameW(w, cls, 64);
    for (auto skip : {L"Progman", L"WorkerW", L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Windows.UI.Core.CoreWindow"})
        if (!wcscmp(cls, skip)) return false;
    BOOL cloaked = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(w, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    // Full screen (a game, a video): no rim.
    RECT r;
    GetWindowRect(w, &r);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST), &mi);
    if (r.left <= mi.rcMonitor.left && r.top <= mi.rcMonitor.top && r.right >= mi.rcMonitor.right && r.bottom >= mi.rcMonitor.bottom)
        return false;
    return true;
}

void HideRim() {
    for (HWND s : g_rim.strips)
        if (s) ShowWindow(s, SW_HIDE);
    g_rim.shown = false;
}

void CALLBACK RimEvent(HWINEVENTHOOK, DWORD event, HWND w, LONG obj, LONG child, DWORD, DWORD) {
    if (obj != OBJID_WINDOW || child != CHILDID_SELF || !w) return;
    if (event != EVENT_SYSTEM_FOREGROUND && w != g_rim.target) return;
    PostMessageW(g_host, WM_TD_GLASS_EVENT, event, (LPARAM)w);
}

void Unhook(HWINEVENTHOOK* hooks, int n) {
    for (int i = 0; i < n; i++)
        if (hooks[i]) UnhookWinEvent(hooks[i]), hooks[i] = nullptr;
}

void Follow(HWND target) {
    if (target == g_rim.target) return;
    Unhook(g_rim.local, 3);
    g_rim.target = target;
    g_rim.process = 0;
    if (!target) return HideRim();
    GetWindowThreadProcessId(target, &g_rim.process);
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    // Only that program's events: the rest of the desktop costs nothing.
    g_rim.local[0] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_HIDE, nullptr, RimEvent, g_rim.process, 0, flags);
    g_rim.local[1] = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr, RimEvent, g_rim.process, 0, flags);
    g_rim.local[2] = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, nullptr, RimEvent, g_rim.process, 0, flags);
}

HWND MakeStrip() {
    return CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kRimClass, L"", WS_POPUP, 0, 0, 0, 0,
                           nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
}

void DrawRim() {
    HWND t = g_rim.target;
    if (!g_wanted.rims || g_compare || !t || !Eligible(t) || !EngineReady()) return HideRim();
    RECT f;
    if (FAILED(DwmGetWindowAttribute(t, DWMWA_EXTENDED_FRAME_BOUNDS, &f, sizeof(f)))) GetWindowRect(t, &f);
    float s = ScaleOf(t);
    int width = (int)std::lround(g_wanted.rimWidth * s), hr = (int)std::lround(g_wanted.rimRadius * s);
    RECT outer{f.left - width, f.top - width, f.right + width, f.bottom + width};
    // Top and bottom take the corners; the sides fill between.
    RECT parts[4] = {{outer.left, outer.top, outer.right, f.top + hr},
                     {outer.left, f.bottom - hr, outer.right, outer.bottom},
                     {outer.left, f.top + hr, f.left, f.bottom - hr},
                     {f.right, f.top + hr, outer.right, f.bottom - hr}};
    GlassLook look = g_wanted.look;
    look.shadow = 0;
    look.radius = (float)(g_wanted.rimRadius + g_wanted.rimWidth);
    look.bevel = std::min(look.bevel, (float)g_wanted.rimWidth);  // the whole band curves
    GlassShape shape;
    shape.shape = outer;
    shape.hole = f;
    shape.holeRadius = (float)g_wanted.rimRadius;
    shape.scale = s;
    shape.time = Seconds();
    if (g_wanted.clickGlow && g_glowFrom) shape.glow = std::max(0.f, 1 - (GetTickCount() - g_glowFrom) / 600.f);
    // Just above the target, whatever comes and goes above it.
    HWND above = GetWindow(t, GW_HWNDPREV);
    while (above && std::find(std::begin(g_rim.strips), std::end(g_rim.strips), above) != std::end(g_rim.strips))
        above = GetWindow(above, GW_HWNDPREV);
    bool topmost = GetWindowLongPtrW(t, GWL_EXSTYLE) & WS_EX_TOPMOST;
    for (int i = 0; i < 4; i++) {
        RECT r = parts[i];
        if (r.right <= r.left || r.bottom <= r.top) continue;
        if (!g_rim.strips[i]) g_rim.strips[i] = MakeStrip();
        shape.view = r;
        std::vector<uint32_t> px;
        if (!g_engine.Render(look, shape, &px)) continue;
        Show(g_rim.strips[i], r, px);
        HWND after = above ? above : (topmost ? HWND_TOPMOST : HWND_TOP);
        SetWindowPos(g_rim.strips[i], after, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    g_rim.frame = f;
    g_rim.shown = true;
}

bool g_rimsOn;

void RimsOn(bool on) {
    if (on == g_rimsOn) return;
    g_rimsOn = on;
    if (on) {
        HWND fg = GetForegroundWindow();
        Follow(Eligible(fg) ? fg : nullptr);
        DrawRim();
        return;
    }
    Follow(nullptr);
    for (HWND& s : g_rim.strips)
        if (s) DestroyWindow(s), s = nullptr;
}

// Which window is in front: for the rims and the app glass. One event per
// focus change, nothing else.
void WatchForeground(bool on) {
    if (on == (g_foregroundHook != nullptr)) return;
    if (on) {
        g_foregroundHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, RimEvent, 0, 0,
                                           WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        return;
    }
    UnhookWinEvent(g_foregroundHook);
    g_foregroundHook = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Apps: Windows' own backdrops, or frosted, in apps you choose.

struct AccentPolicy {
    int state, flags;
    DWORD color;  // AABBGGRR
    int animation;
};
struct CompositionData {
    int attribute;
    void* data;
    SIZE_T size;
};
using SetCompositionFn = BOOL(WINAPI*)(HWND, CompositionData*);

SetCompositionFn SetComposition() {
    static SetCompositionFn fn = (SetCompositionFn)(void*)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute");
    return fn;
}

struct AppRule {
    std::wstring exe, backdrop;
    int opacity = 100;
};
struct Applied {
    std::wstring backdrop;
    bool addedLayered = false;
};
std::vector<AppRule> g_rules;
std::map<HWND, Applied> g_applied;

std::vector<AppRule> ReadRules() {
    std::vector<AppRule> out;
    if (!GetInt(L"glass", L"enabled")) return out;
    std::map<std::wstring, AppRule> apps;
    std::map<std::wstring, bool> on;
    for (auto const& [key, value] : SettingsWithPrefix(L"glass.app.")) {
        std::wstring rest = key.substr(10);
        size_t dot = rest.rfind(L'.');
        if (dot == std::wstring::npos) continue;
        std::wstring app = rest.substr(0, dot), field = rest.substr(dot + 1);
        if (field == L"on") on[app] = ToInt(value) != 0;
        else if (field == L"exe") apps[app].exe = value;
        else if (field == L"backdrop") apps[app].backdrop = value;
        else if (field == L"opacity") apps[app].opacity = std::clamp(ToInt(value), 30, 100);
    }
    for (auto& [name, rule] : apps)
        if (on[name] && !rule.exe.empty()) out.push_back(rule);
    return out;
}

std::wstring ExeOf(HWND w) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!p) return L"";
    wchar_t path[MAX_PATH];
    DWORD n = MAX_PATH;
    std::wstring out;
    if (QueryFullProcessImageNameW(p, 0, path, &n)) {
        out = path;
        out = out.substr(out.find_last_of(L"\\/") + 1);
    }
    CloseHandle(p);
    return out;
}

void SetBackdropType(HWND w, int type) { DwmSetWindowAttribute(w, 38 /* DWMWA_SYSTEMBACKDROP_TYPE */, &type, sizeof(type)); }

void SetAccent(HWND w, int state, DWORD abgr) {
    if (auto set = SetComposition()) {
        AccentPolicy a{state, 2, abgr, 0};
        CompositionData d{19 /* WCA_ACCENT_POLICY */, &a, sizeof(a)};
        set(w, &d);
    }
}

void Undo(HWND w, const Applied& a) {
    if (!IsWindow(w)) return;
    if (a.backdrop == L"frosted") SetAccent(w, 0, 0);
    else SetBackdropType(w, 0);
    LONG_PTR ex = GetWindowLongPtrW(w, GWL_EXSTYLE);
    if (a.addedLayered) SetWindowLongPtrW(w, GWL_EXSTYLE, ex & ~WS_EX_LAYERED);
    else if (ex & WS_EX_LAYERED) SetLayeredWindowAttributes(w, 0, 255, LWA_ALPHA);
}

void ApplyTo(HWND w) {
    if (!w || g_rules.empty() || g_applied.count(w)) return;
    wchar_t cls[64] = L"";
    GetClassNameW(w, cls, 64);
    for (auto skip : {L"Progman", L"WorkerW", L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"})
        if (!wcscmp(cls, skip)) return;
    std::wstring exe = ExeOf(w);
    if (exe.empty()) return;
    for (auto const& rule : g_rules) {
        if (_wcsicmp(rule.exe.c_str(), exe.c_str())) continue;
        if (!_wcsicmp(exe.c_str(), L"explorer.exe") && wcscmp(cls, L"CabinetWClass")) return;  // its folder windows only
        Applied a{rule.backdrop};
        int type = rule.backdrop == L"mica" ? 2 : rule.backdrop == L"acrylic" ? 3 : rule.backdrop == L"tabbed" ? 4 : 0;
        if (type) SetBackdropType(w, type);
        if (rule.backdrop == L"frosted") {
            uint32_t tint = 0x1C2026;
            ParseColour(LookSetting(L"tint"), &tint);
            DWORD abgr = 0x60u << 24 | (tint & 255) << 16 | (tint & 0xFF00) | (tint >> 16 & 255);
            SetAccent(w, 4 /* ACCENT_ENABLE_ACRYLICBLURBEHIND */, abgr);
        }
        if (rule.opacity < 100) {
            LONG_PTR ex = GetWindowLongPtrW(w, GWL_EXSTYLE);
            BYTE alpha;
            DWORD flags;
            // A window that draws its own see-through pixels (UpdateLayeredWindow) is left as it is.
            if (!(ex & WS_EX_LAYERED) || GetLayeredWindowAttributes(w, nullptr, &alpha, &flags)) {
                if (!(ex & WS_EX_LAYERED)) SetWindowLongPtrW(w, GWL_EXSTYLE, ex | WS_EX_LAYERED), a.addedLayered = true;
                SetLayeredWindowAttributes(w, 0, (BYTE)(rule.opacity * 255 / 100), LWA_ALPHA);
            }
        }
        g_applied[w] = a;
        Log(L"%s glass for %s", rule.backdrop.c_str(), exe.c_str());
        return;
    }
}

BOOL CALLBACK ApplyEach(HWND w, LPARAM) {
    if (IsWindowVisible(w) && GetAncestor(w, GA_ROOT) == w) ApplyTo(w);
    return TRUE;
}

void ApplyRules() {
    auto rules = ReadRules();
    bool same = rules.size() == g_rules.size();
    for (size_t i = 0; same && i < rules.size(); i++)
        same = rules[i].exe == g_rules[i].exe && rules[i].backdrop == g_rules[i].backdrop && rules[i].opacity == g_rules[i].opacity;
    if (same) return;
    for (auto const& [w, a] : g_applied) Undo(w, a);
    g_applied.clear();
    g_rules = rules;
    if (!g_rules.empty()) EnumWindows(ApplyEach, 0);
}

////////////////////////////////////////////////////////////////////////////////

bool Needed() { return g_wanted.studio || g_wanted.rims || g_test; }

void RedrawAll() {
    DrawTest();
    DrawRim();
    StudioGlassChanged();
}

bool Animating() {
    bool moves = g_wanted.look.oil > 0 || EffectMoves(g_wanted.look.effect);
    return (moves && (g_test || g_rim.shown)) || (g_glowFrom && GetTickCount() - g_glowFrom < 650);
}

void Frame() {
    g_framePending = false;
    DrawTest();
    DrawRim();
    if (g_wanted.look.oil > 0 || EffectMoves(g_wanted.look.effect)) StudioGlassChanged();
    if (Animating() && GetTickCount() < g_activeUntil) g_framePending = SetTimer(g_host, kTimerFrame, 33, nullptr) != 0;
    else if (g_glowFrom && GetTickCount() - g_glowFrom >= 650) g_glowFrom = 0;
}

// A frame soon (at most one waiting: a stream of moves draws every 15 ms,
// not after they stop).
void Wake(DWORD ms) {
    g_activeUntil = std::max(g_activeUntil, GetTickCount() + ms);
    if (!g_framePending) g_framePending = SetTimer(g_host, kTimerFrame, 15, nullptr) != 0;
}

LRESULT CALLBACK RimProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProcW(wnd, msg, wp, lp);
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////

const std::vector<GlassLookPreset>& GlassLooks() { return kLooks; }

std::wstring GlassLookValue(const std::wstring& look, const std::wstring& key) {
    if (key.rfind(L"glass.", 0) != 0) return L"";
    std::wstring k = key.substr(6);
    for (auto const& l : kLooks)
        if (look == l.id)
            for (auto const& [name, value] : l.values)
                if (k == name) return value;
    return L"";
}

bool GlassIsLookSetting(const std::wstring& key) {
    if (key.rfind(L"glass.", 0) != 0) return false;
    for (auto const& p : kParams)
        if (key.substr(6) == p.key) return true;
    return false;
}

void GlassDefaults(Settings& out) {
    out[L"glass.enabled"] = L"0";
    out[L"glass.look"] = L"clear";
    for (auto const& [name, value] : kLooks[0].values) out[std::wstring(L"glass.") + name] = value;
    out[L"glass.effect"] = L"none";
    out[L"glass.effectAmount"] = L"60";
    out[L"glass.clickGlow"] = L"1";
    out[L"glass.studio"] = L"1";
    out[L"glass.rims.on"] = L"0";
    out[L"glass.rims.width"] = L"6";
    out[L"glass.rims.radius"] = L"8";
    for (auto const& a : kApps) {
        std::wstring base = std::wstring(L"glass.app.") + a.name;
        out[base + L".on"] = L"0";
        out[base + L".exe"] = a.exe;
        out[base + L".backdrop"] = a.backdrop;
        out[base + L".opacity"] = a.opacity;
    }
}

void GlassDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"glass", L"Glass",
        L"Liquid glass drawn over your wallpaper: bent and lit at its edges, blurred and tinted. Pick a look, then change anything; "
        L"what you change wins over the look.");
    add(L"glass.enabled", L"Glass on", L"The Studio's own glass, rims and app glass below. The test window works with it off, to try a look.")
        .isSwitch = true;
    SettingInfo& look = add(L"glass.look", L"Look", L"A starting point for every setting below. Choosing one puts back the ones you changed.");
    for (auto const& l : kLooks) look.options.push_back({l.id, l.label});
    for (auto const& p : kParams) {
        SettingInfo& si = add(std::wstring(L"glass.") + p.key, p.name, p.description);
        if (p.max > p.min) si.hasMin = si.hasMax = true, si.min = p.min, si.max = p.max;
    }
    SettingInfo& effect = add(L"glass.effect", L"Effect",
                              L"A shader over the glass, after its own: CRT scanlines, VHS, film grain, pixelate, dither and more. VHS, "
                              L"grain and glitch move, so they cost a little while they show.");
    for (int i = 0; i < kEffectCount; i++) effect.options.push_back({kEffectIds[i], kEffectNames[i]});
    SettingInfo& amount = add(L"glass.effectAmount", L"Effect strength", L"0 to 100.");
    amount.hasMin = amount.hasMax = true, amount.min = 0, amount.max = 100;
    add(L"glass.clickGlow", L"Glow on focus", L"A rim glows for a moment when its window comes to the front.").isSwitch = true;
    add(L"glass.studio", L"The Studio in glass", L"This window wears the glass too.").isSwitch = true;
    add(L"glass.rims", L"Rims", L"A glass rim round the window in front. Apps stay as they are inside it.");
    add(L"glass.rims.on", L"Rim round the window in front", L"Not on full-screen or maximised windows.").isSwitch = true;
    SettingInfo& w = add(L"glass.rims.width", L"Width", L"Pixels at 100% scaling.");
    w.hasMin = w.hasMax = true, w.min = 2, w.max = 24;
    SettingInfo& r = add(L"glass.rims.radius", L"Window corners", L"Windows 11 rounds windows by 8; 0 for square windows.");
    r.hasMin = r.hasMax = true, r.min = 0, r.max = 24;
    add(L"glass.app", L"Apps",
        L"See-through glass inside apps. Mica, Acrylic and Tabbed are Windows' own and show where the app lets them (Terminal, Explorer, "
        L"Notepad). Frosted blurs what's behind any app and makes the whole window see-through, text too, so it's per app and off "
        L"until you turn it on.");
    for (auto const& a : kApps) {
        std::wstring base = std::wstring(L"glass.app.") + a.name;
        std::wstring title = a.name;
        title[0] = towupper(title[0]);
        add(base, title.c_str(), L"");
        add(base + L".on", L"Glass in it", L"").isSwitch = true;
        add(base + L".exe", L"Program", L"Its file name.");
        add(base + L".backdrop", L"Kind", L"").options = {
            {L"mica", L"Mica"}, {L"acrylic", L"Acrylic"}, {L"tabbed", L"Tabbed"}, {L"frosted", L"Frosted (any app)"}, {L"none", L"None"}};
        SettingInfo& o = add(base + L".opacity", L"Opacity (%)", L"Below 100 the whole window is see-through, text too.");
        o.hasMin = o.hasMax = true, o.min = 30, o.max = 100;
    }
}

void GlassStart(HWND host) {
    g_host = host;
    WNDCLASSW wc{};
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_SIZEALL);
    wc.lpfnWndProc = TestProc;
    wc.lpszClassName = kTestClass;
    RegisterClassW(&wc);
    wc.lpfnWndProc = RimProc;
    wc.lpszClassName = kRimClass;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    GlassApply();
}

void GlassApply() {
    if (!g_host) return;
    g_wanted = ReadWanted();
    bool need = Needed();
    Log(L"%s, Studio %d, rims %d, test window %d", g_wanted.on ? L"on" : L"off", g_wanted.studio, g_wanted.rims, g_test != nullptr);
    WatchFolder(need);
    RimsOn(g_wanted.rims);
    if (need) {
        if (EngineReady()) RedrawAll();
        if (g_wanted.look.oil > 0 || EffectMoves(g_wanted.look.effect)) Wake(6000);
    } else {
        g_engine.Stop();  // nothing to draw: nothing kept, the GPU's memory included
        StudioGlassChanged();
    }
    ApplyRules();
    WatchForeground(g_wanted.rims || !g_rules.empty());
}

void GlassStop() {
    WatchFolder(false);
    RimsOn(false);
    WatchForeground(false);
    for (auto const& [w, a] : g_applied) Undo(w, a);
    g_applied.clear();
    g_rules.clear();
    if (g_test) DestroyWindow(g_test);
    g_engine.Stop();
    if (g_host) KillTimer(g_host, kTimerFrame), KillTimer(g_host, kTimerBackdrop);
    g_backdropGen++;
}

bool GlassMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_TD_BACKDROP: {
            std::unique_ptr<Backdrop> b((Backdrop*)lp);
            if ((unsigned)wp != g_backdropGen) return true;
            g_backdropBusy = false;
            bool set = Needed() && g_engine.Started() && g_engine.SetBackdrop(*b);
            Log(L"backdrop %dx%d %s", b->width, b->height, set ? L"in use" : L"not used");
            if (set) RedrawAll();
            return true;
        }
        case WM_TD_GLASS_FOLDER: ComposeSoon(400); return true;
        case WM_SETTINGCHANGE:
            if (wp == SPI_SETDESKWALLPAPER && Needed()) ComposeSoon(300);
            return false;
        case WM_DISPLAYCHANGE:
            if (Needed()) ComposeSoon(500);
            return false;
        case WM_TIMER:
            if (wp == kTimerBackdrop) {
                KillTimer(wnd, kTimerBackdrop);
                Compose();
                return true;
            }
            if (wp == kTimerFrame) {
                KillTimer(wnd, kTimerFrame);
                Frame();
                return true;
            }
            return false;
        case WM_TD_GLASS_EVENT: {
            HWND w = (HWND)lp;
            DWORD event = (DWORD)wp;
            if (event == EVENT_SYSTEM_FOREGROUND) {
                ApplyTo(w);
                if (!g_wanted.rims) return true;
                Follow(Eligible(w) ? w : nullptr);
                if (g_rim.target) g_glowFrom = GetTickCount(), Wake(800);
                else HideRim();
                return true;
            }
            if (w != g_rim.target) return true;
            if (event == EVENT_OBJECT_DESTROY || event == EVENT_OBJECT_HIDE || event == EVENT_SYSTEM_MINIMIZESTART) {
                if (event == EVENT_OBJECT_DESTROY) Follow(nullptr);
                HideRim();
            } else {
                Wake(event == EVENT_OBJECT_LOCATIONCHANGE ? 300 : 800);  // drawn at the next frame
            }
            return true;
        }
    }
    return false;
}

void GlassTestWindow(bool show) {
    if (!g_host) return;
    if (!show) {
        if (g_test) DestroyWindow(g_test);
        return;
    }
    if (g_test) {
        SetForegroundWindow(g_test);
        return;
    }
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    UINT dpi = 96, dpiY;
    using GetDpiForMonitorFn = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    if (HMODULE shcore = LoadLibraryExW(L"shcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32))
        if (auto get = (GetDpiForMonitorFn)(void*)GetProcAddress(shcore, "GetDpiForMonitor")) get(mon, 0, &dpi, &dpiY);
    float s = dpi / 96.f;
    int w = (int)((440 + 2 * g_testMargin) * s), h = (int)((240 + 2 * g_testMargin) * s);
    RECT work = mi.rcWork;
    int x = work.left + (work.right - work.left - w) / 2, y = work.top + (work.bottom - work.top - h) / 3;
    g_test = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOOLWINDOW, kTestClass, L"Tourne glass", WS_POPUP, x, y, w, h, nullptr, nullptr,
                             GetModuleHandleW(nullptr), nullptr);
    if (!g_test) return;
    GlassApply();
    DrawTest();
    ShowWindow(g_test, SW_SHOW);
    SetForegroundWindow(g_test);
}

bool GlassTestWindowShown() { return g_test != nullptr; }

void GlassCompare(bool holding) {
    if (g_compare == holding) return;
    g_compare = holding;
    RedrawAll();
}

bool GlassStudioOn() { return g_wanted.studio && !g_compare; }

bool GlassForWindow(HWND wnd, std::vector<uint32_t>* pixels, int* width, int* height) {
    if (!GlassStudioOn() || !EngineReady()) return false;
    RECT rc;
    GetClientRect(wnd, &rc);
    MapWindowPoints(wnd, nullptr, (POINT*)&rc, 2);
    GlassShape shape;
    shape.view = rc;
    shape.shape = rc;
    shape.scale = ScaleOf(wnd);
    shape.time = Seconds();
    GlassLook look = g_wanted.look;
    look.radius = 0;
    look.shadow = 0;
    look.lens = 0;  // (a whole window as a dome is too much)
    look.oil *= 0.5f;  // (and a whole window of oil film too loud)
    if (!g_engine.Render(look, shape, pixels)) return false;
    *width = rc.right - rc.left, *height = rc.bottom - rc.top;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// --glass-preview: the looks drawn on a picture, without the desktop.

int GlassPreview(const std::wstring& args) {
    auto arg = [&](const wchar_t* name) -> std::wstring {
        size_t at = args.find(name);
        if (at == std::wstring::npos) return L"";
        at += wcslen(name);
        while (at < args.size() && args[at] == L' ') at++;
        if (at < args.size() && args[at] == L'"') {
            size_t end = args.find(L'"', at + 1);
            return args.substr(at + 1, end == std::wstring::npos ? std::wstring::npos : end - at - 1);
        }
        size_t end = args.find(L' ', at);
        return args.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
    };
    std::wstring out = arg(L"--glass-preview"), picture = arg(L"--wallpaper"), only = arg(L"--look"), effect = arg(L"--effect");
    // --effect name (or all: a picture for each, of the first look)
    int effectFrom = effect == L"all" ? 1 : EffectByName(effect), effectTo = effect == L"all" ? kEffectCount - 1 : effectFrom;
    if (out.empty()) out = L"glass-preview.png";
    const int W = 1600, H = 1000;
    Backdrop b;
    if (picture.empty() || !PictureBackdrop(picture, W, H, &b)) GradientBackdrop(W, H, &b);
    GlassEngine engine;
    if (!engine.Start() || !engine.SetBackdrop(b)) {
        fwprintf(stderr, L"Direct3D 11 or its shader compiler isn't available\n");
        return 1;
    }
    int made = 0;
    for (auto const& l : kLooks) {
        if (!only.empty() && only != L"all" && only != l.id) continue;
        if (only.empty() && made) break;  // (just the first look unless asked)
        for (int fx = effectFrom; fx <= effectTo; fx++) {
        GlassLook look;
        auto num = [&](const wchar_t* key) { return (float)_wtof(GlassLookValue(l.id, std::wstring(L"glass.") + key).c_str()); };
        look.blur = num(L"blur"), look.refraction = num(L"refraction") / 100, look.fringe = num(L"fringe") / 100;
        look.edgeGlow = num(L"edgeGlow") / 100, look.highlight = num(L"highlight") / 100, look.bevel = num(L"bevel");
        look.lens = num(L"lens") / 100, look.tintAmount = num(L"tintAmount") / 100, look.saturation = num(L"saturation") / 100;
        look.brightness = num(L"brightness") / 100, look.radius = num(L"radius"), look.shadow = num(L"shadow") / 100;
        look.oil = num(L"oilFilm") / 100;
        look.effect = fx, look.effectAmount = 0.8f;
        ParseColour(GlassLookValue(l.id, L"glass.tint"), &look.tint);

        std::vector<uint32_t> canvas = b.pixels, px;
        // A window with a rim round it.
        RECT win{930, 170, 1460, 640};
        for (int y = win.top; y < win.bottom; y++)
            for (int x = win.left; x < win.right; x++)
                canvas[(size_t)y * W + x] = y < win.top + 34 ? 0xFF2B3036 : 0xFF1B1F24;
        RECT all{0, 0, W, H};
        GlassShape rim;
        rim.shape = {win.left - 7, win.top - 7, win.right + 7, win.bottom + 7};
        rim.hole = win;
        rim.holeRadius = 8;
        rim.view = all;
        GlassLook rimLook = look;
        rimLook.shadow = 0, rimLook.radius = 15, rimLook.bevel = std::min(look.bevel, 7.f);
        if (engine.Render(rimLook, rim, &px)) BlendOnto(canvas, W, H, px, W, H, 0, 0);
        // A card, and a bar.
        GlassShape card;
        card.shape = {150, 180, 790, 600};
        card.view = all;
        card.time = 3;
        if (engine.Render(look, card, &px)) BlendOnto(canvas, W, H, px, W, H, 0, 0);
        GlassShape bar;
        bar.shape = {260, 890, 1340, 950};
        bar.view = all;
        GlassLook barLook = look;
        barLook.radius = 30;
        if (engine.Render(barLook, bar, &px)) BlendOnto(canvas, W, H, px, W, H, 0, 0);
        std::wstring path = out;
        if (effect == L"all") {
            size_t dot = path.rfind(L'.');
            path = path.substr(0, dot) + L"-" + kEffectIds[fx] + (dot == std::wstring::npos ? L".png" : path.substr(dot));
        }
        if (only == L"all") {
            size_t dot = path.rfind(L'.');
            path = path.substr(0, dot) + L"-" + l.id + (dot == std::wstring::npos ? L".png" : path.substr(dot));
        }
        if (!SavePng(path, canvas, W, H)) {
            fwprintf(stderr, L"couldn't write %s\n", path.c_str());
            return 1;
        }
        made++;
        }
    }
    return made ? 0 : 1;
}

}  // namespace td
