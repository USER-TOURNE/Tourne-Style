// Runs the taskbar part outside Windhawk for a few seconds and captures it.
#include <winsock2.h>
#include <windows.h>
#include <cstdarg>
#include <cstdio>

#define WH_MOD_ID L"bartest"
void Wh_Log(PCWSTR format, ...) {
    va_list args;
    va_start(args, format);
    vwprintf(format, args);
    va_end(args);
    wprintf(L"\n");
}
// Settings from environment variables TB_<name>, else the defaults.
PCWSTR Wh_GetStringSetting(PCWSTR name, ...) {
    static wchar_t buf[512];
    wchar_t formatted[96], var[128];
    va_list args;
    va_start(args, name);
    vswprintf(formatted, 96, name, args);
    va_end(args);
    swprintf(var, 128, L"TB_%ls", formatted);
    if (GetEnvironmentVariableW(var, buf, 512)) return buf;
    return L"";
}
void Wh_FreeStringSetting(PCWSTR) {}
int Wh_GetIntSetting(PCWSTR name, ...) {
    wchar_t var[64], buf[32];
    swprintf(var, 64, L"TB_%ls", name);
    if (GetEnvironmentVariableW(var, buf, 32)) return _wtoi(buf);
    return 0;
}
static int g_optionCalls = 0;
BOOL Wh_SetStringValue(PCWSTR name, PCWSTR value) {
    g_optionCalls++;
    if (!GetEnvironmentVariableW(L"TB_perf", nullptr, 0)) wprintf(L"option %ls = %ls\n", name, value);
    return TRUE;
}

#include <cstdint>
#include <string>
#include <string_view>
#include <commctrl.h>
#include <shellapi.h>
#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <string>
#include <vector>
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <cmath>
#include <thread>
#include <map>
#include <functional>

// The last frame each window handed to Windows (premultiplied BGRA), so a test can see what the bar drew
// without reading the screen (a locked screen reads back black).
struct TestFrame { int w = 0, h = 0; std::vector<uint32_t> px; };
static std::map<HWND, TestFrame> g_testFrames;
static BOOL WINAPI TestUpdateLayeredWindow(HWND h, HDC dst, POINT* pt, SIZE* size, HDC src, POINT* srcPt, COLORREF key,
                                           BLENDFUNCTION* blend, DWORD flags) {
    if (src && size) {
        DIBSECTION ds{};
        HBITMAP bmp = (HBITMAP)GetCurrentObject(src, OBJ_BITMAP);
        if (GetObjectW(bmp, sizeof(ds), &ds) == sizeof(ds) && ds.dsBm.bmBits && ds.dsBm.bmBitsPixel == 32) {
            GdiFlush();
            TestFrame& f = g_testFrames[h];
            f.w = size->cx;
            f.h = size->cy;
            f.px.assign((size_t)f.w * f.h, 0);
            int sx = srcPt ? srcPt->x : 0, sy = srcPt ? srcPt->y : 0;
            // The bar's pictures are all top-down; GetObject reports biHeight positive either way.
            bool bottomUp = false;
            int bw = ds.dsBm.bmWidth, bh = ds.dsBm.bmHeight;
            for (int y = 0; y < f.h && sy + y < bh; y++) {
                int row = bottomUp ? bh - 1 - (sy + y) : sy + y;
                auto* line = (uint32_t*)((BYTE*)ds.dsBm.bmBits + (size_t)row * ds.dsBm.bmWidthBytes);
                for (int x = 0; x < f.w && sx + x < bw; x++) f.px[(size_t)y * f.w + x] = line[sx + x];
            }
        }
    }
    return UpdateLayeredWindow(h, dst, pt, size, src, srcPt, key, blend, flags);
}
#define UpdateLayeredWindow TestUpdateLayeredWindow

#include "taskbar-inlined.cpp"

// That frame, over a dark grey, as a 24-bit BMP (scaled up by `zoom`, nearest pixel).
static void CaptureFrame(PCWSTR path, HWND h, int zoom = 1) {
    auto it = g_testFrames.find(h);
    if (it == g_testFrames.end()) { wprintf(L"no frame for %p\n", h); return; }
    TestFrame const& f = it->second;
    int w = f.w * zoom, hh = f.h * zoom, stride = (w * 3 + 3) & ~3;
    std::vector<BYTE> out((size_t)stride * hh);
    for (int y = 0; y < hh; y++)
        for (int x = 0; x < w; x++) {
            uint32_t p = f.px[(size_t)(y / zoom) * f.w + x / zoom];
            int a = p >> 24;
            BYTE* o = &out[(size_t)(hh - 1 - y) * stride + x * 3];
            for (int c = 0; c < 3; c++) o[c] = (BYTE)std::min(255, (int)((p >> (8 * c)) & 0xFF) + 0x1E * (255 - a) / 255);
        }
    BITMAPINFOHEADER bi{sizeof(bi), w, hh, 1, 24, BI_RGB};
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi) + out.size()), 0, 0, sizeof(fh) + sizeof(bi)};
    FILE* file = _wfopen(path, L"wb");
    fwrite(&fh, sizeof(fh), 1, file);
    fwrite(&bi, sizeof(bi), 1, file);
    fwrite(out.data(), out.size(), 1, file);
    fclose(file);
}

static void Capture(PCWSTR path, int w, int h, int top = 0, int left = 0) {
    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 24, BI_RGB};
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, left, top, SRCCOPY | CAPTUREBLT);
    GdiFlush();
    int stride = (w * 3 + 3) & ~3;
    FILE* f = _wfopen(path, L"wb");
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + stride * h), 0, 0, 54};
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(bits, stride * h, 1, f);
    fclose(f);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
}

static bool g_captured = false;
static WNDPROC g_shyOld = nullptr;
static std::vector<double> g_frameTimes;  // the animation's frames as the bar got them
static LRESULT CALLBACK ShyTestProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_APP_SHYFRAME) g_frameTimes.push_back(NowMs());
    if (m == WM_APP + 90) {
        SlideTo(h, w != 0);
        return 0;
    }
    return CallWindowProcW(g_shyOld, h, m, w, l);
}
// TB_perf=1: what the bar's work costs, timed on the bar's own thread (WM_APP+91, wParam the test).
static WNDPROC g_perfOld = nullptr;
static double TimeIt(std::function<void()> const& f, int times = 1) {
    double start = NowMs();
    for (int i = 0; i < times; i++) f();
    return (NowMs() - start) / times;
}
// The Start menu's open, less showing it (which would take the focus): its data, its rows, and a paint into memory.
static double StartOpenCost() {
    static int round = 0;
    bool trace = round++ == 0;
    return TimeIt([trace] {
        double t = NowMs();
        auto step = [&](PCWSTR what) {
            if (trace) wprintf(L"    %ls: %.1f ms\n", what, NowMs() - t);
            t = NowMs();
        };
        wprintf(L"    (warm-up: icon thread %ld, icons ready %d, cached %zu, places %d, home %d)\n", g_iconThreads,
                g_iconsWarm != nullptr, g_startIcons.size(), g_placesValid, g_homeValid);
        EnsureStartWindow();
        step(L"window");
        StartTakeApps();
        if (g_startApps.empty() && !StartAppsReading()) ReadStartApps();
        step(L"programs");
        if (!g_placesValid) LoadPlaces();
        step(L"places");
        if (trace) {
            StartFont();
            step(L"  font");
            g_startView = kViewHome;
            BuildStartRows();
            step(L"  home rows");
            StartCase(PowerLabel(g_startConfig.powerButton));
            step(L"  power label");
        }
        StartComputeLayout();
        step(L"layout");
        g_startView = kViewHome;
        BuildStartRows();
        step(L"rows");
        RECT r;
        GetClientRect(g_startWnd, &r);
        HDC screen = GetDC(nullptr);
        HDC mem = CreateCompatibleDC(screen);
        HBITMAP bmp = CreateCompatibleBitmap(screen, std::max(1L, r.right), std::max(1L, r.bottom));
        HGDIOBJ old = SelectObject(mem, bmp);
        PaintStart(g_startWnd, mem);
        step(L"painted");
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
        ReleaseDC(nullptr, screen);
    });
}
static LRESULT CALLBACK PerfProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m != WM_APP + 91) return CallWindowProcW(g_perfOld, h, m, w, l);
    switch (w) {
        case 0:
            wprintf(L"redraw: %.2f ms (average of 40)\n", TimeIt([h] { Redraw(h); }, 40));
            break;
        case 1: {
            // The glow following the pointer: a redraw with a button hot and the aura on.
            bool was = g_settings.dynamicAura;
            int hot = g_hot;
            g_settings.dynamicAura = true;
            g_hot = g_buttons.empty() ? -1 : 0;
            wprintf(L"redraw with the aura following: %.2f ms (average of 40)\n", TimeIt([h] { Redraw(h); }, 40));
            g_settings.dynamicAura = was;
            g_hot = hot;
            break;
        }
        case 2:
            wprintf(L"reload (what a display change does): %.1f ms (average of 3)\n",
                    TimeIt([h] { ApplySettings(h); }, 3));
            wprintf(L"one reload, step by step (%d option writes so far):\n", g_optionCalls);
            g_perfTrace = true;
            g_perfLast = 0;
            ApplySettings(h);
            g_perfTrace = false;
            break;
        case 3:
            wprintf(L"start menu, first open in this process: %.1f ms\n", StartOpenCost());
            wprintf(L"start menu, again: %.1f ms\n", StartOpenCost());
            ApplySettings(h, true);
            wprintf(L"start menu, after a display change: %.1f ms\n", StartOpenCost());
            ClearStartIcons();
            wprintf(L"start menu, icons dropped (as after a settings change): %.1f ms\n", StartOpenCost());
            wprintf(L"start menu apps folder read on this thread: %.1f ms (%zu apps)\n",
                    TimeIt([] { ReadStartApps(); }), g_startApps.size());
            break;
        case 5: {
            // "All programs" pressed: the folders read, the rows built, and a paint (its icons made as it draws).
            EnsureStartWindow();
            StartTakeApps();
            if (g_startApps.empty() && !StartAppsReading()) ReadStartApps();
            StartComputeLayout();
            auto paint = [] {
                RECT r;
                GetClientRect(g_startWnd, &r);
                HDC screen = GetDC(nullptr);
                HDC mem = CreateCompatibleDC(screen);
                HBITMAP bmp = CreateCompatibleBitmap(screen, std::max(1L, r.right), std::max(1L, r.bottom));
                HGDIOBJ old = SelectObject(mem, bmp);
                PaintStart(g_startWnd, mem);
                SelectObject(mem, old);
                DeleteObject(bmp);
                DeleteDC(mem);
                ReleaseDC(nullptr, screen);
            };
            // Rounds: cold (nothing fetched yet), after the idle warm-up the menu does while closed, and again.
            for (int round = 0; round < 3; round++) {
                g_allRoot = {};  // as every open of the menu does
                if (round < 2) {
                    ClearStartIcons();
                    g_progNames.clear();
                }
                if (round == 1) {
                    double w = NowMs();
                    SendMessageW(g_startWnd, WM_TIMER, kTimerApps, 0);
                    while (g_iconThreads > 0) Sleep(5);
                    wprintf(L"  (idle warm-up on its thread: %.0f ms)\n", NowMs() - w);
                }
                double t = NowMs();
                g_allRoot.dirs = {KnownFolderPath(FOLDERID_Programs), KnownFolderPath(FOLDERID_CommonPrograms)};
                ReadProgKids(g_allRoot);
                double read = NowMs() - t;
                t = NowMs();
                g_startView = kViewAll;
                g_scrollPx = 0;
                StartViewWidth();
                BuildStartRows();
                double rows = NowMs() - t;
                t = NowMs();
                paint();
                double painted = NowMs() - t;
                wprintf(L"all programs %ls: folders read %.1f ms (%zu entries), rows %.1f ms (%zu), paint %.1f ms\n",
                        round == 0 ? L"(cold)" : round == 1 ? L"(after the warm-up)" : L"(next open)", read,
                        g_allRoot.kids.size(), rows, g_rows.size(), painted);
            }
            // What the names cost alone: the shell's display name against the file's own name.
            std::vector<std::wstring> paths;
            for (auto const& dir : g_allRoot.dirs) {
                WIN32_FIND_DATAW fd;
                HANDLE find = FindFirstFileW((dir + L"\\*").c_str(), &fd);
                if (find == INVALID_HANDLE_VALUE) continue;
                do {
                    if (fd.cFileName[0] != L'.') paths.push_back(dir + L"\\" + fd.cFileName);
                } while (FindNextFileW(find, &fd));
                FindClose(find);
            }
            wprintf(L"  names: %zu display names %.1f ms\n", paths.size(),
                    TimeIt([&] {
                        for (auto const& p : paths) FileDisplayName(p);
                    }));
            g_startView = kViewHome;
            break;
        }
        case 4:
            wprintf(L"icons: shortcuts and every window's icon read again: %.1f ms (%zu windows)\n",
                    TimeIt([h] {
                        RefreshShortcuts(true);
                        for (auto& win : g_wins) FetchIcon(win);
                    }),
                    g_wins.size());
            break;
    }
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    double initStart = NowMs();
    if (GetEnvironmentVariableW(L"TB_perf", nullptr, 0)) {
        g_perfTrace = true;  // the first reload, the one starting the bar, step by step
        wprintf(L"starting, step by step:\n");
    }
    if (!Wh_ModInit()) {
        wprintf(L"init failed\n");
        return 1;
    }
    double initMs = NowMs() - initStart;
    Wh_ModAfterInit();  // as Windhawk does once every hook is in: the bar then sets itself up
    if (GetEnvironmentVariableW(L"TB_perf", nullptr, 0)) {
        // The bar being set up, until it's drawn (the startup trace prints as it goes).
        for (int i = 0; i < 400 && !IsWindowVisible(g_wnd); i++) Sleep(5);
        wprintf(L"bar shown %.1f ms after starting\n", NowMs() - initStart);
    }
    g_perfTrace = false;
    Sleep(2500);
    if (wchar_t pf[8]; GetEnvironmentVariableW(L"TB_perf", pf, 8)) {
        wprintf(L"the rest of the suite waited for the bar: %.1f ms\n", initMs);
        // TB_perfwait=ms: let the Start menu's idle warm-up (2.5 s after starting) run first.
        if (wchar_t pw[16]; GetEnvironmentVariableW(L"TB_perfwait", pw, 16)) Sleep(_wtoi(pw));
        g_perfOld = (WNDPROC)SetWindowLongPtrW(g_wnd, GWLP_WNDPROC, (LONG_PTR)PerfProc);
        // The Start menu first, while its warm-up is still there (a full reload drops it).
        // TB_perf=all: only "All programs".
        if (!wcscmp(pf, L"all")) {
            SendMessageW(g_wnd, WM_APP + 91, 5, 0);
            return 0;
        }
        for (int test : {3, 0, 1, 4, 2}) SendMessageW(g_wnd, WM_APP + 91, test, 0);
        return 0;
    }
    HWND bar = g_wnd;
    RECT rc{};
    GetWindowRect(bar, &rc);
    wprintf(L"bar %p at %ld,%ld %ldx%ld, %zu buttons, orb %dx%d (%d frames)\n", bar, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, g_buttons.size(), g_orb.width, g_orb.height, g_orb.frames);
    wprintf(L"tipTheme=%p menuTheme=%p bar=%p band=%p font=%p\n", g_theme.tip, g_theme.menu, g_theme.bar, g_theme.band, g_tipFont);
    wprintf(L"settings: themed=%d segments=%d icon=%d margin=%d thick=%d startAt=%d ground=%06X\n", g_themed, (int)g_settings.segments, g_settings.iconSize, g_settings.margin, g_settings.thickness, (int)g_settings.startAt, g_settings.palette.ground);
    // TB_shytest=1 (with TB_dynamicShyness=1): a window of ours moves onto the bar and away again; the
    // bar's place, opacity and progress are sampled as it goes and comes back (the real mouse isn't touched).
    // TB_shyshot=1 also captures the screen's edge once it's away (shy-away.bmp).
    if (wchar_t st[8]; GetEnvironmentVariableW(L"TB_shytest", st, 8)) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"ShyTestWindow";
        wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
        RegisterClassW(&wc);
        RECT shown = BarRect(true);
        HWND win = CreateWindowExW(0, wc.lpszClassName, L"shy test", WS_OVERLAPPEDWINDOW, shown.left + 300, shown.top - 400, 500, 300,
                                   nullptr, nullptr, wc.hInstance, nullptr);
        ShowWindow(win, SW_SHOWNOACTIVATE);
        auto pump = [] {
            MSG m;
            while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
        };
        auto watch = [&](PCWSTR label, double ms) {
            double start = NowMs(), last = -1, firstMove = -1, lastMove = -1;
            int frames = 0;
            LONG lastTop = LONG_MIN, lastLeft = LONG_MIN;
            BYTE lastAlpha = 0;
            double worst = 0;
            std::vector<double> gaps;
            while (NowMs() - start < ms) {
                pump();
                RECT r;
                GetWindowRect(bar, &r);
                BYTE alpha = g_barAlpha;
                if (r.top != lastTop || r.left != lastLeft || alpha != lastAlpha) {
                    double now = NowMs() - start;
                    if (firstMove < 0) firstMove = now;
                    else gaps.push_back(now - last), worst = std::max(worst, now - last);
                    if (frames < 40 || frames % 5 == 0)
                        wprintf(L"  %ls %7.1f ms  at %ld,%ld  alpha %3d  slide %.3f\n", label, now, r.left, r.top, alpha, g_slide);
                    last = lastMove = now;
                    lastTop = r.top;
                    lastLeft = r.left;
                    lastAlpha = alpha;
                    frames++;
                }
                Sleep(1);
            }
            std::sort(gaps.begin(), gaps.end());
            if (g_frameTimes.size() > 2) {
                std::vector<double> f = g_frameTimes, d;
                for (size_t i = 1; i < f.size(); i++) d.push_back(f[i] - f[i - 1]);
                std::sort(d.begin(), d.end());
                wprintf(L"%ls: %zu frames from the bar over %.0f ms, median %.2f ms, slowest %.2f ms\n", label, f.size(), f.back() - f.front(), d[d.size() / 2], d.back());
            }
            g_frameTimes.clear();
            double median = gaps.empty() ? 0 : gaps[gaps.size() / 2];
            wprintf(L"%ls: first change at %.0f ms, %d changes over %.0f ms, median gap %.1f ms, worst %.1f ms, ex=%08lX\n", label,
                    firstMove, frames, lastMove - firstMove, median, worst, (DWORD)GetWindowLongPtrW(bar, GWL_EXSTYLE));
        };
        if (_wtoi(st) == 2) {
            // Driven directly: the bar is told to go and to come back (on its own thread, through its window).
            g_shyOld = (WNDPROC)SetWindowLongPtrW(bar, GWLP_WNDPROC, (LONG_PTR)ShyTestProc);
            watch(L"settle", 300);
            for (int round = 0; round < 2; round++) {
                PostMessageW(bar, WM_APP + 90, 0, 0);
                watch(L"hide", 900);
                if (round == 0 && GetEnvironmentVariableW(L"TB_shyshot", st, 8)) {
                    RECT m = BarRect(true);
                    Capture(L"shy-away.bmp", 800, 24, m.bottom - 24, m.left + 200);
                }
                PostMessageW(bar, WM_APP + 90, 1, 0);
                watch(L"show", 700);
            }
            // Reversed halfway: it turns back from where it got to.
            PostMessageW(bar, WM_APP + 90, 0, 0);
            watch(L"half", 110);
            PostMessageW(bar, WM_APP + 90, 1, 0);
            watch(L"back", 500);
            DestroyWindow(win);
            return 0;
        }
        watch(L"settle", 1500);
        // Onto the bar: it goes after the hide delay.
        SetWindowPos(win, nullptr, shown.left + 300, shown.top - 200, 500, 300, SWP_NOZORDER | SWP_NOACTIVATE);
        watch(L"hide", 2000);
        if (wchar_t ss[8]; GetEnvironmentVariableW(L"TB_shyshot", ss, 8)) {
            RECT m = BarRect(true);
            Capture(L"shy-away.bmp", 800, 24, m.bottom - 24, m.left + 200);
        }
        // Away again: it comes back after the return delay.
        SetWindowPos(win, nullptr, shown.left + 300, shown.top - 400, 500, 300, SWP_NOZORDER | SWP_NOACTIVATE);
        watch(L"show", 2000);
        DestroyWindow(win);
        return 0;
    }
    // TB_progress=1: the first running buttons get progress as the relay would send it (normal 50%, error 70%,
    // paused 30%, busy, normal 100%), and the bar's own frame is saved (twice for the busy sweep), zoomed 2x.
    if (wchar_t pg[8]; GetEnvironmentVariableW(L"TB_progress", pg, 8)) {
        UINT msg = RegisterWindowMessageW(L"TourneSuite_Progress");
        struct { int state, value; } sets[] = {{TBPF_NORMAL, 0x8000}, {TBPF_ERROR, 0xB333}, {TBPF_PAUSED, 0x4CCC},
                                               {TBPF_INDETERMINATE, 0}, {TBPF_NORMAL, 0xFFFF}};
        int given = 0;
        for (auto const& b : g_buttons) {
            if (b.windows.empty() || given >= (int)std::size(sets)) continue;
            SendMessageW(bar, msg, (WPARAM)b.windows[0], MAKELPARAM(sets[given].value, sets[given].state));
            wprintf(L"progress: button at %ld..%ld state %d value %d\n", b.rc.left, b.rc.right, sets[given].state, sets[given].value);
            given++;
        }
        MSG m;
        for (DWORD end = GetTickCount() + 300; (int)(end - GetTickCount()) > 0; Sleep(5))
            while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
        CaptureFrame(L"progress-a.bmp", bar, 2);
        for (DWORD end = GetTickCount() + 400; (int)(end - GetTickCount()) > 0; Sleep(5))
            while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
        CaptureFrame(L"progress-b.bmp", bar, 2);
        // Cleared again: nothing left drawn.
        for (auto const& b : g_buttons)
            if (!b.windows.empty()) SendMessageW(bar, msg, (WPARAM)b.windows[0], 0);
        wprintf(L"progress table after clearing: %zu\n", g_progress.size());
        return 0;
    }
    // TB_hoverat=x,y (bar-local; a negative x counts from the right end): the pointer there, as a message, and
    // the bar's own frame saved (hoverat.bmp, 2x). TB_pressat=1 adds a press (no release, so nothing runs).
    if (wchar_t ha[32]; GetEnvironmentVariableW(L"TB_hoverat", ha, 32)) {
        int x = _wtoi(ha), y = 0;
        if (wchar_t* comma = wcschr(ha, L',')) y = _wtoi(comma + 1);
        RECT r;
        GetClientRect(bar, &r);
        if (x < 0) x += r.right;
        g_tracking = true;  // so the bar asks for no WM_MOUSELEAVE
        SendMessageW(bar, WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
        Sleep(400);
        wprintf(L"hoverat %d,%d hot=%d showDesktop %ld..%ld clock %ld..%ld themed=%d arrowThemes=%d%d%d%d clock=%d toolbar=%d sd=%d%d preview=%d\n",
                x, y, g_hot, g_showDesktopRect.left, g_showDesktopRect.right, g_clockRect.left, g_clockRect.right, g_themed,
                g_theme.trayArrow[0] != nullptr, g_theme.trayArrow[1] != nullptr, g_theme.trayArrow[2] != nullptr,
                g_theme.trayArrow[3] != nullptr, g_theme.clock != nullptr, g_theme.trayButton != nullptr,
                g_theme.showDesktop[0] != nullptr, g_theme.showDesktop[1] != nullptr, g_theme.preview != nullptr);
        for (auto& t : g_tray) wprintf(L"tray %ld..%ld arrow=%d\n", t.rc.left, t.rc.right, IsOverflowArrow(t));
        HTHEME picked = TrayArrowTheme();
        int pickedIndex = -1;
        for (int i = 0; i < 4; i++)
            if (picked && picked == g_theme.trayArrow[i]) pickedIndex = i;
        wprintf(L"arrow theme index %d (open %d clicked %d)\n", pickedIndex, ArrowOpen(), g_arrowClicked);
        // What each arrow handle draws (state 1), as text: '#' opaque light pixels.
        for (int i = 0; i < 4; i++) {
            if (!g_theme.trayArrow[i]) continue;
            HDC dc = CreateCompatibleDC(nullptr);
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), 22, -22, 1, 32, BI_RGB};
            void* bits = nullptr;
            HBITMAP b = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
            HGDIOBJ old = SelectObject(dc, b);
            memset(bits, 0, 22 * 22 * 4);
            RECT r{0, 0, 22, 22};
            DrawThemeBackground(g_theme.trayArrow[i], dc, 1, 1, &r, nullptr);
            GdiFlush();
            wprintf(L"arrow %d handle %p:\n", i, g_theme.trayArrow[i]);
            for (int y = 0; y < 22; y++) {
                bool any = false;
                std::wstring row;
                for (int x = 0; x < 22; x++) {
                    unsigned p = ((unsigned*)bits)[y * 22 + x];
                    bool on = (p >> 24) > 128;
                    any |= on;
                    row += on ? L'#' : L'.';
                }
                if (any) wprintf(L"  %ls\n", row.c_str());
            }
            SelectObject(dc, old);
            DeleteObject(b);
            DeleteDC(dc);
        }
        // The same arrow through DrawThemePartAlpha, into a cell-sized picture.
        if (g_theme.trayArrow[0]) {
            HDC ref = GetDC(nullptr);
            Surface s(ref, 22, 38);
            ReleaseDC(nullptr, ref);
            memset(s.bits, 0, 22 * 38 * 4);
            RECT cell{0, 0, 22, 38};
            DrawThemePartAlpha(s.dc, g_theme.trayArrow[0], 1, 1, cell);
            GdiFlush();
            wprintf(L"arrow 0 through DrawThemePartAlpha:\n");
            for (int y = 0; y < 38; y++) {
                std::wstring row;
                bool any = false;
                for (int x = 0; x < 22; x++) {
                    unsigned p = s.bits[y * 22 + x];
                    bool on = (p >> 24) > 128;
                    any |= on;
                    row += on ? L'#' : L'.';
                }
                if (any) wprintf(L"  %ls\n", row.c_str());
            }
        }
        // The arrow's cell as the bar drew it (the frame given to Windows): '#' light pixels.
        if (auto it = g_testFrames.find(bar); it != g_testFrames.end())
            for (auto const& t : g_tray)
                if (IsOverflowArrow(t)) {
                    wprintf(L"arrow cell as drawn, %ld..%ld x %ld..%ld:\n", t.rc.left, t.rc.right, t.rc.top, t.rc.bottom);
                    for (int y = t.rc.top; y < t.rc.bottom; y++) {
                        std::wstring row;
                        for (int x = t.rc.left; x < t.rc.right; x++) {
                            unsigned p = it->second.px[(size_t)y * it->second.w + x];
                            int lum = ((p & 0xFF) + ((p >> 8) & 0xFF) + ((p >> 16) & 0xFF)) / 3;
                            row += lum > 150 ? L'#' : lum > 60 ? L'+' : L'.';
                        }
                        wprintf(L"  %ls\n", row.c_str());
                    }
                }
        wprintf(L"other handles: bar %p band %p tip %p menu %p clock %p toolbar %p sd %p %p preview %p\n", g_theme.bar,
                g_theme.band, g_theme.tip, g_theme.menu, g_theme.clock, g_theme.trayButton, g_theme.showDesktop[0],
                g_theme.showDesktop[1], g_theme.preview);
        CaptureFrame(L"hoverat.bmp", bar, 2);
        if (wchar_t pr[4]; GetEnvironmentVariableW(L"TB_pressat", pr, 4)) {
            SendMessageW(bar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
            Sleep(200);
            CaptureFrame(L"pressat.bmp", bar, 2);
        }
        return 0;
    }
    // TB_knowntray=1: the tray icons Windows has seen, as the tray menu's "Not running now" lists them.
    if (GetEnvironmentVariableW(L"TB_knowntray", nullptr, 0)) {
        auto known = KnownTrayIcons();
        wprintf(L"known tray icons: %zu\n", known.size());
        for (size_t i = 0; i < known.size() && i < 8; i++) wprintf(L"  %ls = %ls\n", known[i].key.c_str(), known[i].label.c_str());
        return 0;
    }    // TB_idletest=1: fading when idle, driven directly (the pointer stays where it is): the bar's opacity
    // after the idle check, then after its fade has run, then after waking.
    if (GetEnvironmentVariableW(L"TB_idletest", nullptr, 0)) {
        wprintf(L"idle: fade %d after %d s to %d%%, alpha %d\n", g_settings.idleFade, g_settings.idleAfter, g_settings.idleOpacity, g_barAlpha);
        g_idle.lastActive = GetTickCount64() - 3600 * 1000;  // left alone for an hour
        IdleTick(bar);
        wprintf(L"idle: after the check alpha %d (running %d, to %.2f)\n", g_barAlpha, g_idle.running, g_idle.to);
        for (int i = 0; i < 200 && g_idle.running; i++) {
            Sleep(5);
            IdleFrame(bar);
        }
        wprintf(L"idle: after the fade alpha %d (level %.2f)\n", g_barAlpha, g_idle.level);
        g_idle.lastActive = GetTickCount64();
        IdleTick(bar);
        for (int i = 0; i < 200 && g_idle.running; i++) {
            Sleep(5);
            IdleFrame(bar);
        }
        wprintf(L"idle: after waking alpha %d\n", g_barAlpha);
        return 0;
    }    // TB_stacktest=1: the style's stack parts (TaskBand2 13 and 14, states 2 and 3) drawn at a few sizes by uxtheme,
    // printed as pixels (grey level of the colour, or . for clear), with the part's own size and margins.
    if (GetEnvironmentVariableW(L"TB_stacktest", nullptr, 0)) {
        HTHEME band = g_theme.band;
        wprintf(L"stack: band %p\n", band);
        if (!band) return 0;
        for (int part : {kBandButton, kBandStack}) {
            MARGINS m{};
            GetThemeMargins(band, nullptr, part, 1, TMT_SIZINGMARGINS, nullptr, &m);
            int width = 0;
            HRESULT hw = GetThemeInt(band, part, 1, TMT_WIDTH, &width);
            SIZE natural{};
            GetThemePartSize(band, nullptr, part, 2, nullptr, TS_TRUE, &natural);
            wprintf(L"part %d: margins l%d r%d t%d b%d, width %d (%08lx), natural %ldx%ld\n", part, m.cxLeftWidth, m.cxRightWidth,
                    m.cyTopHeight, m.cyBottomHeight, width, hw, natural.cx, natural.cy);
        }
        auto dump = [&](int part, int state, int w, int h) {
            BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
            uint32_t* bits = nullptr;
            HDC dc = CreateCompatibleDC(nullptr);
            HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
            HGDIOBJ old = SelectObject(dc, bmp);
            RECT r{0, 0, w, h};
            DrawThemeBackground(band, dc, part, state, &r, nullptr);
            GdiFlush();
            wprintf(L"part %d state %d at %dx%d:\n", part, state, w, h);
            for (int y = 0; y < h; y++) {
                if (y > 4 && y < h - 5 && y != h / 2) continue;  // the ends and the middle
                wprintf(L"  %2d ", y);
                for (int x = std::max(0, w - 12); x < w; x++) {
                    uint32_t px = bits[y * w + x];
                    int a = px >> 24, g = (px >> 8) & 0xFF;
                    if (!a) wprintf(L" ..");
                    else wprintf(L" %02X", g * 255 / a);
                }
                wprintf(L"\n");
            }
            SelectObject(dc, old);
            DeleteObject(bmp);
            DeleteDC(dc);
        };
        dump(kBandButton, 1, 40, 28);
        for (int w : {6, 8, 9})
            for (int state : {2, 3}) dump(kBandStack, state, w, 28);
        dump(kBandStack, 3, 8, 40);
        return 0;
    }
    // TB_previewtest=N: the Nth task button's preview popup painted into a bitmap (DWM's thumbnails aren't in
    // it), with its first window pointed at (preview-hot.bmp) and its close button (preview-close.bmp).
    if (wchar_t pv[8]; GetEnvironmentVariableW(L"TB_previewtest", pv, 8)) {
        int n = _wtoi(pv);
        // A pinned button with no windows has no preview: the next one that has.
        while (n >= 0 && n < (int)g_buttons.size() && g_buttons[n].windows.empty()) n++;
        wprintf(L"preview button %d of %zu\n", n, g_buttons.size());
        if (n >= 0 && n < (int)g_buttons.size()) {
            RECT b = g_buttons[n].rc;
            g_tracking = true;
            SendMessageW(bar, WM_MOUSEMOVE, 0, MAKELPARAM((b.left + b.right) / 2, (b.top + b.bottom) / 2));
            // This runs on the bar's thread, so its show timer can't fire: open it directly.
            KillTimer(bar, kTimerPreviewShow);
            ShowPreview(bar, n);
            wprintf(L"preview %p cells %zu themed %d\n", g_preview.wnd, g_preview.cells.size(), g_theme.preview != nullptr);
            {
                RECT pr{};
                GetWindowRect(g_preview.wnd, &pr);
                wprintf(L"preview at %ld,%ld %ldx%ld, button %ld..%ld, pointer %ld\n", pr.left, pr.top, pr.right - pr.left, pr.bottom - pr.top, g_preview.buttonRc.left, g_preview.buttonRc.right, g_preview.at.x); wprintf(L"preview hooks: mouse %d keys %d\n", g_previewMouseHook != nullptr, g_previewKeyHook != nullptr);
            }
            auto render = [](PCWSTR path) {
                RECT c;
                GetClientRect(g_preview.wnd, &c);
                HDC screen = GetDC(nullptr);
                HDC mem = CreateCompatibleDC(screen);
                BITMAPINFO bi{};
                bi.bmiHeader = {sizeof(BITMAPINFOHEADER), c.right, -c.bottom, 1, 32, BI_RGB};
                void* bits = nullptr;
                HBITMAP dib = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
                HGDIOBJ old = SelectObject(mem, dib);
                PaintPreview(g_preview.wnd, mem);
                GdiFlush();
                TestFrame& f = g_testFrames[g_preview.wnd];
                f.w = c.right;
                f.h = c.bottom;
                f.px.assign((uint32_t*)bits, (uint32_t*)bits + (size_t)c.right * c.bottom);
                for (auto& px : f.px) px |= 0xFF000000;
                CaptureFrame(path, g_preview.wnd, 2);
                SelectObject(mem, old);
                DeleteObject(dib);
                DeleteDC(mem);
                ReleaseDC(nullptr, screen);
            };
            if (g_preview.wnd && !g_preview.cells.empty()) {
                g_preview.hot = 0;
                g_preview.hotClose = false;
                render(L"preview-hot.bmp");
                g_preview.hotClose = true;
                render(L"preview-close.bmp");
            }
        }
        return 0;
    }
    // TB_btntest=1: drag the first button onto the fourth, then off the bar.
    if (wchar_t bt[8]; GetEnvironmentVariableW(L"TB_btntest", bt, 8) && g_buttons.size() >= 4) {
        auto show = [](PCWSTR label) {
            wprintf(L"%ls:", label);
            for (auto const& b : g_buttons) wprintf(L" [%ls]", ButtonKey(b).substr(ButtonKey(b).find_last_of(L"\\/!") + 1).c_str());
            wprintf(L"\n");
        };
        auto mid = [](RECT r) { return MAKELPARAM((r.left + r.right) / 2, (r.top + r.bottom) / 2); };
        show(L"before");
        RECT from = g_buttons[0].rc, to = g_buttons[3].rc;
        SendMessageW(bar, WM_LBUTTONDOWN, MK_LBUTTON, mid(from));
        for (int step = 1; step <= 10; step++)
            SendMessageW(bar, WM_MOUSEMOVE, MK_LBUTTON,
                         MAKELPARAM((from.left + from.right) / 2 + ((to.left + to.right) / 2 - (from.left + from.right) / 2) * step / 10,
                                    (from.top + from.bottom) / 2));
        show(L"dragging");
        SendMessageW(bar, WM_LBUTTONUP, 0, mid(g_buttons[3].rc));
        show(L"dropped");
        wprintf(L"saved order entries: %zu\n", LoadTrayList(kButtonOrderValue).size());
        // A second drag, dropped off the bar: back as it was.
        from = g_buttons[0].rc;
        SendMessageW(bar, WM_LBUTTONDOWN, MK_LBUTTON, mid(from));
        for (int step = 1; step <= 10; step++)
            SendMessageW(bar, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM((from.left + from.right) / 2 + step * 20, (from.top + from.bottom) / 2));
        SendMessageW(bar, WM_LBUTTONUP, 0, MAKELPARAM((from.left + from.right) / 2 + 200, 300));
        show(L"cancelled");
        SaveTrayList(kButtonOrderValue, {});  // leave no order behind
        return 0;
    }
    // TB_dragtest=1: drag the first ordinary tray icon onto the overflow arrow
    // with posted messages (the real mouse isn't touched), then report.
    if (wchar_t dt[8]; GetEnvironmentVariableW(L"TB_dragtest", dt, 8)) {
        int arrow = -1, icon = -1;
        for (int i = 0; i < (int)g_tray.size(); i++) {
            bool isArrow = IsOverflowArrow(g_tray[i]);
            wprintf(L"tray[%d] %ls kind=%d arrow=%d rc=%ld,%ld-%ld,%ld\n", i, g_tray[i].key.c_str(),
                    SystemIconKind(g_tray[i].icon), isArrow, g_tray[i].rc.left, g_tray[i].rc.top, g_tray[i].rc.right,
                    g_tray[i].rc.bottom);
            if (isArrow && arrow < 0) arrow = i;
            else if (!isArrow && icon < 0 && SystemIconKind(g_tray[i].icon) == 0) icon = i;
        }
        wprintf(L"icon %d arrow %d\n", icon, arrow);
        if (icon >= 0 && arrow >= 0) {
            std::wstring key = g_tray[icon].key;
            auto mid = [](RECT r) { return MAKELPARAM((r.left + r.right) / 2, (r.top + r.bottom) / 2); };
            RECT from = g_tray[icon].rc, to = g_tray[arrow].rc;
            SendMessageW(bar, WM_LBUTTONDOWN, MK_LBUTTON, mid(from));
            for (int step = 1; step <= 12; step++) {
                int x = from.left + (to.left - from.left) * step / 12 + (from.right - from.left) / 2;
                int y = (from.top + from.bottom) / 2;
                SendMessageW(bar, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x, y));
                wprintf(L"  move x=%d drag=%d moved=%d onArrow=%d\n", x, g_trayDrag, g_trayDragMoved, g_trayDragOnArrow);
            }
            // The arrow's rect again: icons may have moved while dragging.
            for (auto const& item : g_tray)
                if (IsOverflowArrow(item)) to = item.rc;
            SendMessageW(bar, WM_MOUSEMOVE, MK_LBUTTON, mid(to));
            wprintf(L"  on arrow: drag=%d onArrow=%d\n", g_trayDrag, g_trayDragOnArrow);
            SendMessageW(bar, WM_LBUTTONUP, 0, mid(to));
            wprintf(L"after drop: place of %ls = %d, in tray: %d\n", key.c_str(), TrayPlaces::Load().Of(key),
                    (int)std::count_if(g_tray.begin(), g_tray.end(), [&](TrayItem const& t) { return t.key == key; }));
            TrayPlaces::Load().Set(key, kPlaceDefault);  // put it back
        }
        return 0;
    }
    // TB_install=<path>: what installing a file, folder or .zip does.
    if (wchar_t ip[512]; GetEnvironmentVariableW(L"TB_install", ip, 512)) {
        InstallResult result;
        InstallPath(ip, result);
        wprintf(L"install: %d styles, %d orbs:", result.styles, result.orbs);
        for (auto const& n : result.names) wprintf(L" [%ls]", n.c_str());
        wprintf(L"\n");
        return 0;
    }
    // TB_gallery=<n>: opens the YASB gallery, gets the list, and installs the nth theme of it (the window's own buttons are pressed).
    if (wchar_t gl[8]; GetEnvironmentVariableW(L"TB_gallery", gl, 8)) {
        auto pump = [](int ms) {
            DWORD end = GetTickCount() + ms;
            MSG m;
            while (GetTickCount() < end) {
                while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&m);
                    DispatchMessageW(&m);
                }
                Sleep(10);
            }
        };
        gallery::Open(nullptr);
        wprintf(L"gallery: window %d\n", gallery::g_win != nullptr);
        pump(300);
        SendMessageW(gallery::g_win, WM_COMMAND, MAKEWPARAM(gallery::kIdGet, BN_CLICKED), (LPARAM)gallery::g_get);
        for (int i = 0; i < 240 && gallery::g_entries.empty(); i++) pump(500);
        wprintf(L"gallery: %d themes; status: ", (int)gallery::g_entries.size());
        WCHAR text[512];
        GetWindowTextW(gallery::g_status, text, 512);
        wprintf(L"%ls\n", text);
        for (size_t i = 0; i < gallery::g_entries.size() && i < 5; i++)
            wprintf(L"  %ls | %ls | %ls\n", gallery::g_entries[i].name.c_str(), gallery::g_entries[i].author.c_str(), gallery::g_entries[i].style.c_str());
        int pick = _wtoi(gl);
        SendMessageW(gallery::g_list, LB_SETCURSEL, pick, 0);
        SendMessageW(gallery::g_win, WM_COMMAND, MAKEWPARAM(gallery::kIdList, LBN_SELCHANGE), (LPARAM)gallery::g_list);
        GetWindowTextW(gallery::g_detail, text, 512);
        wprintf(L"gallery: detail [%ls]\n", text);
        SendMessageW(gallery::g_win, WM_COMMAND, MAKEWPARAM(gallery::kIdInstall, BN_CLICKED), (LPARAM)gallery::g_install);
        for (int i = 0; i < 120 && gallery::g_busy; i++) pump(250);
        GetWindowTextW(gallery::g_status, text, 512);
        wprintf(L"gallery: after install: %ls\n", text);
        // A pasted GitHub link instead of the list's pick.
        SetWindowTextW(gallery::g_link, L"https://github.com/amnweb/yasb-themes/tree/main/themes/03a6eaa2-6d0b-485d-94aa-2d87e98f3c64");
        SendMessageW(gallery::g_win, WM_COMMAND, MAKEWPARAM(gallery::kIdInstall, BN_CLICKED), (LPARAM)gallery::g_install);
        for (int i = 0; i < 120 && gallery::g_busy; i++) pump(250);
        GetWindowTextW(gallery::g_status, text, 512);
        wprintf(L"gallery: pasted link: %ls\n", text);
        RECT r;
        GetWindowRect(gallery::g_win, &r);
        Capture(L"gallery.bmp", r.right - r.left, r.bottom - r.top, r.top, r.left);
        wprintf(L"installed folders:");
        WIN32_FIND_DATAW fd;
        HANDLE find = FindFirstFileW((TourneYasb() + L"*").c_str(), &fd);
        if (find != INVALID_HANDLE_VALUE) {
            do wprintf(L" [%ls]", fd.cFileName); while (FindNextFileW(find, &fd));
            FindClose(find);
        }
        wprintf(L"\n");
        gallery::Close();
        return 0;
    }
    // TB_reload=1: a file dropped into the styles folder (and taken out again) makes the bar read its settings again.
    if (wchar_t rl[8]; GetEnvironmentVariableW(L"TB_reload", rl, 8)) {
        std::wstring file = TourneStyles() + L"\\zz-reload-test.tmp";
        int before = g_optionCalls;
        HANDLE h = CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        Sleep(1500);
        int afterCreate = g_optionCalls;
        DeleteFileW(file.c_str());
        Sleep(1500);
        wprintf(L"reload: created file -> %d option publishes, removed -> %d (folder %ls)\n", afterCreate - before,
                g_optionCalls - afterCreate, TourneStyles().c_str());
        return 0;
    }
    // TB_bars=1: every bar (one per screen): where it is, what it holds, and a picture of each.
    if (wchar_t bt[8]; GetEnvironmentVariableW(L"TB_bars", bt, 8)) {
        wprintf(L"%zu bars\n", g_bars.size());
        for (size_t i = 0; i < g_bars.size(); i++) {
            BarState* s = g_bars[i];
            RECT r{};
            GetWindowRect(s->hwnd, &r);
            // The main bar's state is in the globals (it is the active one); the others' in their own.
            size_t buttons = s == g_active ? g_buttons.size() : s->buttons.size();
            wprintf(L"bar %zu %p monitor %p at %ld,%ld %ldx%ld: %zu buttons, orb %ld,%ld-%ld,%ld\n", i, s->hwnd, s->monitor, r.left,
                    r.top, r.right - r.left, r.bottom - r.top, buttons, s == g_active ? g_orbRect.left : s->orbRect.left,
                    s == g_active ? g_orbRect.top : s->orbRect.top, s == g_active ? g_orbRect.right : s->orbRect.right,
                    s == g_active ? g_orbRect.bottom : s->orbRect.bottom);
            wprintf(L"  tray icons on this bar: %zu\n", s == g_active ? g_tray.size() : s->tray.size());
            std::wstring name = L"bar" + std::to_wstring(i) + L".bmp";
            Capture(name.c_str(), r.right - r.left, r.bottom - r.top, r.top, r.left);
        }
        // TB_barhover=N: the pointer on the first button of bar N (its glow and preview), captured.
        if (wchar_t bh[8]; GetEnvironmentVariableW(L"TB_barhover", bh, 8)) {
            size_t n = (size_t)_wtoi(bh);
            if (n < g_bars.size() && !g_bars[n]->buttons.empty()) {
                RECT r{}, b = g_bars[n]->buttons[0].rc;
                GetWindowRect(g_bars[n]->hwnd, &r);
                POINT old;
                GetCursorPos(&old);
                SetCursorPos(r.left + (b.left + b.right) / 2, r.top + (b.top + b.bottom) / 2);
                Sleep(300);
                {
                    POINT c;
                    GetCursorPos(&c);
                    HWND under = WindowFromPoint(c);
                    WCHAR cls[64] = L"";
                    if (under) GetClassNameW(under, cls, 64);
                    wprintf(L"cursor %ld,%ld over %p (%ls), bar is %p\n", c.x, c.y, under, cls, g_bars[n]->hwnd);
                }
                Sleep(1500);
                RECT pr{};
                if (g_preview.wnd) GetWindowRect(g_preview.wnd, &pr);
                wprintf(L"preview wnd %p bar %p (this bar %p) visible %d at %ld,%ld %ldx%ld cells %zu\n", g_preview.wnd, g_preview.bar,
                        g_bars[n]->hwnd, g_preview.wnd ? IsWindowVisible(g_preview.wnd) : 0, pr.left, pr.top, pr.right - pr.left,
                        pr.bottom - pr.top, g_preview.cells.size());
                Capture(L"barhover.bmp", r.right - r.left, r.bottom - r.top, r.top, r.left);
                // and the preview above it
                RECT above{r.left, r.top - 200, r.left + std::min<LONG>(r.right - r.left, 600), r.top};
                Capture(L"barpreview.bmp", above.right - above.left, above.bottom - above.top, above.top, above.left);
                SetCursorPos(old.x, old.y);
            }
        }
        // TB_hover=N: the pointer (as a message) on the Nth task button; the bar captured once the glow has faded in.
        // (These blocks are reached only with TB_bars=1.)
        if (wchar_t hb[8]; GetEnvironmentVariableW(L"TB_hover", hb, 8)) {
            int n = _wtoi(hb);
            if (n >= 0 && n < (int)g_buttons.size()) {
                RECT b = g_buttons[n].rc, r{};
                GetWindowRect(g_wnd, &r);
                g_tracking = true;  // so the bar asks for no WM_MOUSELEAVE (the real pointer is elsewhere)
                SendMessageW(g_wnd, WM_MOUSEMOVE, 0, MAKELPARAM((b.left + b.right) / 2, (b.top + b.bottom) / 2));
                Sleep(500);
                wprintf(L"hover: button %d rect %ld,%ld-%ld,%ld hot=%d\n", n, b.left, b.top, b.right, b.bottom, g_hot);
                Capture(L"hover.bmp", r.right - r.left, r.bottom - r.top, r.top, r.left);
            }
        }
        // TB_csshover=N: a YASB theme's Nth button of its own pointed at (N = -1 the Start button), captured.
        if (wchar_t ch[8]; GetEnvironmentVariableW(L"TB_csshover", ch, 8)) {
            wprintf(L"css targets: %zu; start button %ld,%ld-%ld,%ld; clock %ld,%ld-%ld,%ld\n", g_cssTargets.size(), g_orbRect.left, g_orbRect.top,
                    g_orbRect.right, g_orbRect.bottom, g_clockRect.left, g_clockRect.top, g_clockRect.right, g_clockRect.bottom);
            for (size_t i = 0; i < g_cssTargets.size(); i++)
                wprintf(L"  target %zu: %ld,%ld-%ld,%ld left=%ls\n", i, g_cssTargets[i].rc.left, g_cssTargets[i].rc.top, g_cssTargets[i].rc.right,
                        g_cssTargets[i].rc.bottom, g_cssTargets[i].left.c_str());
            int n = _wtoi(ch);
            RECT target = n < 0 ? g_orbRect : n < (int)g_cssTargets.size() ? g_cssTargets[n].rc : RECT{};
            RECT r{};
            GetWindowRect(g_wnd, &r);
            // The pointer is sent to the bar as a message (the real pointer may be in use): the bar draws its hover state at once.
            SendMessageW(g_wnd, WM_MOUSEMOVE, 0, MAKELPARAM((target.left + target.right) / 2, (target.top + target.bottom) / 2));
            wprintf(L"hover: hot=%d\n", g_hot);
            Sleep(60);
            POINT old{};            Capture(L"csshover.bmp", r.right - r.left, r.bottom - r.top, r.top, r.left);
        }
        // TB_popup=calendar|volume|power: the theme's popup opened by a click on its widget (sent as messages), then captured.
        if (wchar_t pw[16]; GetEnvironmentVariableW(L"TB_popup", pw, 16)) {
            RECT target{};
            if (!wcscmp(pw, L"calendar")) target = g_clockRect;
            for (auto const& t : g_cssTargets)
                if (t.left == std::wstring(L"popup:") + pw) target = t.rc;
            wprintf(L"popup %ls: target %ld,%ld-%ld,%ld\n", pw, target.left, target.top, target.right, target.bottom);
            LPARAM at = MAKELPARAM((target.left + target.right) / 2, (target.top + target.bottom) / 2);
            SendMessageW(g_wnd, WM_LBUTTONDOWN, MK_LBUTTON, at);
            SendMessageW(g_wnd, WM_LBUTTONUP, 0, at);
            Sleep(700);
            HWND popup = FindWindowW(L"TourneCssPopup", nullptr);
            wprintf(L"popup window: %p visible %d\n", popup, popup ? IsWindowVisible(popup) : 0);
            if (popup) {
                RECT pr;
                GetWindowRect(popup, &pr);
                wprintf(L"popup rect %ld,%ld %ldx%ld\n", pr.left, pr.top, pr.right - pr.left, pr.bottom - pr.top);
                Capture(L"popup.bmp", pr.right - pr.left, pr.bottom - pr.top, pr.top, pr.left);
                if (wchar_t hv[8]; GetEnvironmentVariableW(L"TB_popuphot", hv, 8)) {
                    // A pointer over the nth button of the popup, as a message: its hover.
                    int n = _wtoi(hv);
                    SendMessageW(popup, WM_MOUSEMOVE, 0, MAKELPARAM(80 + n * 10, 80));
                    Sleep(200);
                    Capture(L"popuphot.bmp", pr.right - pr.left, pr.bottom - pr.top, pr.top, pr.left);
                }
                SendMessageW(popup, WM_KEYDOWN, VK_ESCAPE, 0);
                Sleep(200);
                wprintf(L"popup after Escape: %d\n", FindWindowW(L"TourneCssPopup", nullptr) != nullptr);
            }
        }
        // TB_cssclick=N: a click on the Nth button of the theme's own (N = -1 the Start button), sent as messages, and the screen captured.
        if (wchar_t cc[8]; GetEnvironmentVariableW(L"TB_cssclick", cc, 8)) {
            int n = _wtoi(cc);
            RECT target = n < 0 ? g_orbRect : n < (int)g_cssTargets.size() ? g_cssTargets[n].rc : RECT{};
            LPARAM at = MAKELPARAM((target.left + target.right) / 2, (target.top + target.bottom) / 2);
            wprintf(L"click %d on %ld,%ld-%ld,%ld\n", n, target.left, target.top, target.right, target.bottom);
            SendMessageW(g_wnd, WM_MOUSEMOVE, 0, at);
            SendMessageW(g_wnd, WM_LBUTTONDOWN, MK_LBUTTON, at);
            SendMessageW(g_wnd, WM_LBUTTONUP, 0, at);
            Sleep(1200);
            Capture(L"cssclick.bmp", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0);
        }
        // The other screens' bars, worked like the main one: menus, dragging, hiding, scale, the install dialog and dropping.
        wchar_t bh0[512];
        BarState* second = nullptr;
        for (size_t i = 1; i < g_bars.size() && !second; i++)
            if (g_bars[i]->buttons.size() >= 2) second = g_bars[i];
        auto mid = [](RECT r) { return MAKELPARAM((r.left + r.right) / 2, (r.top + r.bottom) / 2); };
        auto menusOpen = [] {
            int count = 0;
            EnumWindows(
                [](HWND h, LPARAM param) -> BOOL {
                    WCHAR cls[16];
                    DWORD pid = 0;
                    GetWindowThreadProcessId(h, &pid);
                    if (pid == GetCurrentProcessId() && IsWindowVisible(h) && GetClassNameW(h, cls, 16) && !wcscmp(cls, L"#32768")) (*(int*)param)++;
                    return TRUE;
                },
                (LPARAM)&count);
            return count;
        };
        if (second && GetEnvironmentVariableW(L"TB_secmenu", bh0, 8)) {
            RECT r{};
            GetWindowRect(second->hwnd, &r);
            POINT old;
            GetCursorPos(&old);
            for (int pass = 0; pass < 2; pass++) {
                RECT b = second->buttons[0].rc;
                POINT at = pass == 0 ? POINT{(b.left + b.right) / 2, (b.top + b.bottom) / 2} : POINT{500, 19};
                SetCursorPos(r.left + at.x, r.top + at.y);
                PostMessageW(second->hwnd, WM_RBUTTONUP, 0, MAKELPARAM(at.x, at.y));
                Sleep(1500);
                wprintf(L"secmenu pass %d (%ls): %d menu windows open\n", pass, pass == 0 ? L"a button" : L"empty bar", menusOpen());
                Capture(pass == 0 ? L"secmenu0.bmp" : L"secmenu1.bmp", 560, 360, r.top - 340, r.left);
                DWORD_PTR result;
                SendMessageTimeoutW(second->hwnd, WM_CANCELMODE, 0, 0, SMTO_ABORTIFHUNG, 2000, &result);
                Sleep(500);
                wprintf(L"  after cancel: %d\n", menusOpen());
            }
            SetCursorPos(old.x, old.y);
        }
        if (second && GetEnvironmentVariableW(L"TB_secdrag", bh0, 8)) {
            RECT from = second->buttons[0].rc, to = second->buttons[1].rc;
            size_t before = LoadTrayList(kButtonOrderValue).size();
            SendMessageW(second->hwnd, WM_LBUTTONDOWN, MK_LBUTTON, mid(from));
            for (int step = 1; step <= 10; step++)
                SendMessageW(second->hwnd, WM_MOUSEMOVE, MK_LBUTTON,
                             MAKELPARAM((from.left + from.right) / 2 + ((to.left + to.right) / 2 - (from.left + from.right) / 2) * step / 10 + 2,
                                        (from.top + from.bottom) / 2));
            SendMessageW(second->hwnd, WM_LBUTTONUP, 0, mid(to));
            Sleep(300);
            wprintf(L"secdrag: saved order entries %zu -> %zu\n", before, LoadTrayList(kButtonOrderValue).size());
            SaveTrayList(kButtonOrderValue, {});  // leave no order behind
        }
        if (GetEnvironmentVariableW(L"TB_secautohide", bh0, 8) && g_bars.size() > 1) {
            SetCursorPos(500, 300);
            Sleep(2500);
            for (size_t i = 0; i < g_bars.size(); i++) {
                RECT r{};
                GetWindowRect(g_bars[i]->hwnd, &r);
                wprintf(L"autohide: bar %zu away from the pointer: top %ld height %ld\n", i, r.top, r.bottom - r.top);
            }
            RECT r{};
            GetWindowRect(g_bars[1]->hwnd, &r);
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(g_bars[1]->monitor, &mi);
            SetCursorPos(mi.rcMonitor.left + 200, mi.rcMonitor.bottom - 1);
            Sleep(1000);
            GetWindowRect(g_bars[1]->hwnd, &r);
            wprintf(L"autohide: bar 1 with the pointer at its edge: top %ld height %ld (monitor bottom %ld)\n", r.top, r.bottom - r.top, mi.rcMonitor.bottom);
            SetCursorPos(500, 300);
            Sleep(1500);
            GetWindowRect(g_bars[1]->hwnd, &r);
            wprintf(L"autohide: bar 1 after it leaves: top %ld\n", r.top);
        }
        if (second && GetEnvironmentVariableW(L"TB_secscale", bh0, 8)) {
            wprintf(L"secscale: main %d, second %d\n", g_scale, second->scale);
            second->scale = 2;  // as if its screen were at 200%
            Sleep(1400);        // the next refresh draws it so
            RECT r{};
            GetWindowRect(second->hwnd, &r);
            wprintf(L"secscale: second now %d, main still %d; its first button is %ld px wide (main's is %ld)\n", second->scale, g_scale,
                    second->buttons[0].rc.right - second->buttons[0].rc.left, g_buttons[0].rc.right - g_buttons[0].rc.left);
            Capture(L"secscale.bmp", r.right - r.left, r.bottom - r.top, r.top, r.left);
            PostMessageW(g_wnd, WM_APP_SETTINGS, 0, 0);  // a reload reads each screen's own scale again
            Sleep(1500);
            wprintf(L"secscale: after a reload second %d\n", second->scale);
        }
        if (second && GetEnvironmentVariableW(L"TB_secdialog", bh0, 8)) {
            HWND owner = second->hwnd;
            std::thread dialog([owner] {
                CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
                InstallThemesDialog(owner);
                CoUninitialize();
            });
            Sleep(2000);
            HWND d = FindWindowW(nullptr, L"Install a theme or orb");
            wprintf(L"secdialog: dialog window %p visible %d\n", d, d ? IsWindowVisible(d) : 0);
            if (d) PostMessageW(d, WM_CLOSE, 0, 0);
            dialog.join();
            wprintf(L"secdialog: closed\n");
        }
        if (second && GetEnvironmentVariableW(L"TB_secdrop", bh0, 512)) {
            std::wstring path = bh0;
            size_t bytes = sizeof(DROPFILES) + (path.size() + 2) * sizeof(WCHAR);
            HGLOBAL mem = GlobalAlloc(GHND, bytes);
            auto* df = (DROPFILES*)GlobalLock(mem);
            df->pFiles = sizeof(DROPFILES);
            df->fWide = TRUE;
            memcpy((BYTE*)df + sizeof(DROPFILES), path.c_str(), path.size() * sizeof(WCHAR));
            GlobalUnlock(mem);
            SendMessageW(second->hwnd, WM_DROPFILES, (WPARAM)mem, 0);
            Sleep(800);
            std::wstring installed = TourneStyles() + path.substr(path.find_last_of(L'\\') + 1);
            wprintf(L"secdrop: %ls installed: %d\n", installed.c_str(), GetFileAttributesW(installed.c_str()) != INVALID_FILE_ATTRIBUTES);
            DeleteFileW(installed.c_str());
        }
        // TB_barsflip=1: the setting turned off and on again, live: the other screens' bars close and open.
        if (wchar_t bf[8]; GetEnvironmentVariableW(L"TB_barsflip", bf, 8)) {
            for (PCWSTR value : {L"never", L"always", L"never", L"always"}) {
                SetEnvironmentVariableW(L"TB_multiMonitor", value);
                Wh_ModSettingsChanged();
                Sleep(1500);
                wprintf(L"multiMonitor=%ls: %zu bars, %zu windows on screen\n", value, g_bars.size(), (size_t)std::count_if(
                    g_bars.begin(), g_bars.end(), [](BarState* s) { return s->hwnd && IsWindowVisible(s->hwnd); }));
            }
        }
        g_captured = true;
        return 0;
    }
    // Hover the first button, to see a hover state too (TB_idle=1: leave the bar alone).
    if (!g_buttons.empty() && !GetEnvironmentVariableW(L"TB_idle", nullptr, 0)) {
        RECT b = g_buttons[0].rc;
        PostMessageW(bar, WM_MOUSEMOVE, 0, MAKELPARAM((b.left + b.right) / 2, (b.top + b.bottom) / 2));
    }
    Sleep(600);
    {
        wchar_t hv[16];
        if (GetEnvironmentVariableW(L"TB_hover", hv, 16)) {
            int k = _wtoi(hv);
            if (k >= 0 && k < (int)g_buttons.size()) {
                // Pointed at by messages, not the real pointer, and saved from the bar's own frame (the
                // screen may be locked).
                RECT b = g_buttons[k].rc;
                PostMessageW(g_wnd, WM_MOUSEMOVE, 0, MAKELPARAM((b.left + b.right) / 2, (b.top + b.bottom) / 2));
                MSG m;
                for (DWORD until = GetTickCount() + 1800; GetTickCount() < until; Sleep(10))
                    while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
                CaptureFrame(argc > 1 ? argv[1] : L"bar.bmp", g_wnd);
                g_captured = true;
            }
        }
    }
    {
        wchar_t mv[32];
        if (GetEnvironmentVariableW(L"TB_menu", mv, 32)) {
            POINT old;
            GetCursorPos(&old);
            SetCursorPos(900, 300);
            // TB_menu = a pin's name: right-click its button; else the bar.
            POINT at{1350, 20};
            for (auto const& b : g_buttons)
                if (b.pin >= 0 && Lower(g_pins[b.pin].name).find(Lower(mv)) != std::wstring::npos)
                    at = {(b.rc.left + b.rc.right) / 2, (b.rc.top + b.rc.bottom) / 2};
            wprintf(L"menu at %ld,%ld\n", at.x, at.y);
            PostMessageW(bar, WM_RBUTTONUP, 0, MAKELPARAM(at.x, at.y));
            Sleep(1500);
            Capture(L"menu.bmp", 1500, 600, 270);
            // TB_shellx/TB_shelly: right-click there (the app entry), capture again.
            wchar_t sx[16], sy[16];
            if (GetEnvironmentVariableW(L"TB_shellx", sx, 16) && GetEnvironmentVariableW(L"TB_shelly", sy, 16)) {
                SetCursorPos(_wtoi(sx), _wtoi(sy));
                Sleep(300);
                INPUT r[2] = {{INPUT_MOUSE}, {INPUT_MOUSE}};
                r[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                r[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(2, r, sizeof(INPUT));
                Sleep(2000);
                Capture(L"menu2.bmp", 1500, 700, 270);
                INPUT esc[2] = {{INPUT_KEYBOARD}, {INPUT_KEYBOARD}};
                esc[0].ki.wVk = VK_ESCAPE;
                esc[1].ki.wVk = VK_ESCAPE;
                esc[1].ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(2, esc, sizeof(INPUT));
                Sleep(300);
            }
            INPUT in[2] = {{INPUT_KEYBOARD}, {INPUT_KEYBOARD}};
            in[0].ki.wVk = VK_ESCAPE;
            in[1].ki.wVk = VK_ESCAPE;
            in[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, in, sizeof(INPUT));
            Sleep(300);
            SetCursorPos(old.x, old.y);
        }
    }
    if (!g_captured) CaptureFrame(argc > 1 ? argv[1] : L"bar.bmp", g_wnd);
    for (size_t i = 0; i < g_buttons.size(); i++) {
        if (g_buttons[i].windows.empty()) {
            wprintf(L"button %zu: pin %ls\n", i, g_pins[g_buttons[i].pin].name.c_str());
            continue;
        }
        Win* w = FindWin(g_buttons[i].windows[0]);
        if (!w) continue;
        Shortcut const* s = FindShortcut(*w, true);
        wprintf(L"button %zu: %ls | id=%ls | shortcut=%ls | best=%ls\n", i, w->exe.c_str(), w->appId.c_str(), s ? s->file.c_str() : L"-", w->shortcut.c_str());
    }
    if (GetEnvironmentVariableW(L"TB_debugparts", nullptr, 0) && g_theme.bar) {
        for (int part = 1; part <= 4; part++)
            for (int state = 0; state <= 2; state++) {
                RECT r{0, 0, 60, 38};
                HDC sd = GetDC(nullptr);
                HDC md = CreateCompatibleDC(sd);
                BITMAPINFO bi{};
                bi.bmiHeader = {sizeof(BITMAPINFOHEADER), 60, -38, 1, 32, BI_RGB};
                uint32_t* bits = nullptr;
                HBITMAP dib = CreateDIBSection(sd, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
                SelectObject(md, dib);
                memset(bits, 0, 60 * 38 * 4);
                HRESULT hr = DrawThemeBackground(g_theme.bar, md, part, state, &r, nullptr);
                GdiFlush();
                wprintf(L"DEBUG bar part %d state %d: defined=%d partiallyTransparent=%d hr=%08lX px(30,2)=%08X px(30,19)=%08X px(30,36)=%08X\n", part, state,
                        IsThemePartDefined(g_theme.bar, part, state), IsThemeBackgroundPartiallyTransparent(g_theme.bar, part, state), hr,
                        bits[2 * 60 + 30], bits[19 * 60 + 30], bits[36 * 60 + 30]);
                DeleteDC(md); DeleteObject(dib); ReleaseDC(nullptr, sd);
            }
    }
    // TB_menutest=<path>: the taskbar's app menu as the button menu builds it, drawn off screen.
    if (wchar_t mt[260]; GetEnvironmentVariableW(L"TB_menutest", mt, 260)) {
        auto const& p = g_settings.palette;
        g_menuLook = {p.ground, p.line, p.edge, p.text, p.accent, p.dim, p.hover, g_tipFont, nullptr, g_scale, false};
        if (HFONT font = MenuIconFont()) g_menuLook.font = font;
        g_menuLook.frame = true;
        g_menuLook.caps = true;
        g_menuLook.text = 0x5A9A7D;
        HICON app = (HICON)LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 16, 16, LR_SHARED);
        struct Row { PCWSTR text; HICON icon; bool hot; };
        Row rows[] = {{L"File Explorer", app, false}, {L"Unpin from taskbar", GlyphIcon(kGlyphUnpin), true},
                      {L"End task", GlyphIcon(kGlyphEndTask), false}, {L"Close all windows on this display", GlyphIcon(kGlyphClose), false}};
        std::vector<StyledMenuItem*> items;
        auto add = [&](StyledMenuItem* item) { g_styledItems.push_back(item); items.push_back(item); };
        
        for (auto& row : rows) {
            std::wstring text = row.text;
            CharUpperBuffW(text.data(), (DWORD)text.size());
            auto* item = new StyledMenuItem{text};
            item->icon = row.icon;
            add(item);
        }
        items.front()->padTop = true;
        items.back()->padBottom = true;
        int width = 0, height = 0;
        std::vector<int> heights;
        for (auto* item : items) {
            MEASUREITEMSTRUCT m{ODT_MENU, 0, 0, 0, 0, (ULONG_PTR)item};
            MenuMeasureItem(&m);
            width = std::max<int>(width, m.itemWidth);
            heights.push_back(m.itemHeight);
            height += m.itemHeight;
        }
        int nc = 3 * g_scale, W = width + 2 * nc, H = height + 2 * nc;
        HDC sd = GetDC(nullptr);
        HDC md = CreateCompatibleDC(sd);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), W, H, 1, 24, BI_RGB};
        void* bits = nullptr;
        HBITMAP dib = CreateDIBSection(sd, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        SelectObject(md, dib);
        RECT all{0, 0, W, H};
        SetDCBrushColor(md, MenuRgb(g_menuLook.band));
        FillRect(md, &all, (HBRUSH)GetStockObject(DC_BRUSH));
        SetDCBrushColor(md, MenuRgb(g_menuLook.frameLine));
        FrameRect(md, &all, (HBRUSH)GetStockObject(DC_BRUSH));
        int y = nc, idx = 0;
        for (auto* item : items) {
            DRAWITEMSTRUCT d{};
            d.CtlType = ODT_MENU;
            d.hDC = md;
            d.rcItem = {nc, y, nc + width, y + heights[idx]};
            d.itemData = (ULONG_PTR)item;
            d.itemState = (idx == 1) ? ODS_SELECTED : 0;
            MenuDrawItem(&d);
            y += heights[idx++];
        }
        GdiFlush();
        int stride = (W * 3 + 3) & ~3;
        FILE* f = _wfopen(mt, L"wb");
        BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + stride * H), 0, 0, 54};
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
        fwrite(bits, stride * H, 1, f);
        fclose(f);
        wprintf(L"menu drawn %dx%d\n", W, H);
    }    // TB_bandparts=<path>: every part and state the style's task band defines, on a sheet (rows: parts, columns: states).
    if (wchar_t bp[260]; GetEnvironmentVariableW(L"TB_bandparts", bp, 260) && g_theme.band) {
        const int cw = 60, ch = 40, parts = 14, states = 12;
        int W = cw * (states + 1), H = ch * parts;
        HDC sd = GetDC(nullptr);
        HDC md = CreateCompatibleDC(sd);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), W, -H, 1, 32, BI_RGB};
        uint32_t* bits = nullptr;
        HBITMAP dib = CreateDIBSection(sd, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
        SelectObject(md, dib);
        for (int i = 0; i < W * H; i++) bits[i] = 0xFF141414;
        SetBkMode(md, TRANSPARENT);
        SetTextColor(md, 0xFFFFFF);
        for (int part = 1; part <= parts; part++) {
            wchar_t label[16];
            swprintf(label, 16, L"part %d", part);
            RECT lr{2, (part - 1) * ch + 2, cw, part * ch};
            DrawTextW(md, label, -1, &lr, DT_LEFT | DT_TOP);
            for (int state = 1; state <= states; state++) {
                RECT r{state * cw + 6, (part - 1) * ch + 4, state * cw + 6 + 40, (part - 1) * ch + 4 + 32};
                DrawThemePartAlpha(md, g_theme.band, part, state, r);
                wprintf(L"BAND part %d state %d defined\n", part, state);
            }
        }
        GdiFlush();
        int stride = W * 4;
        FILE* f = _wfopen(bp, L"wb");
        BITMAPINFOHEADER hdr{sizeof(BITMAPINFOHEADER), W, -H, 1, 32, BI_RGB};
        BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + stride * H), 0, 0, 54};
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&hdr, sizeof(hdr), 1, f);
        fwrite(bits, (size_t)stride * H, 1, f);
        fclose(f);
    }    // TB_stacktest=<path>: parts 13 and 14 (states 1-3) drawn beside a button frame, at several widths, to see what they are.
    if (wchar_t st[260]; GetEnvironmentVariableW(L"TB_stacktest", st, 260) && g_theme.band) {
        const int cw = 90, ch = 40, rows = 6, cols = 4;
        int W = cw * cols, H = ch * rows;
        HDC sd = GetDC(nullptr);
        HDC md = CreateCompatibleDC(sd);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), W, -H, 1, 32, BI_RGB};
        uint32_t* bits = nullptr;
        HBITMAP dib = CreateDIBSection(sd, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
        SelectObject(md, dib);
        for (int i = 0; i < W * H; i++) bits[i] = 0xFF121C21;
        int row = 0;
        for (int part : {13, 14}) {
            for (int widthCase = 0; widthCase < 3; widthCase++) {
                int pw = widthCase == 0 ? 6 : widthCase == 1 ? 10 : 33;
                for (int state = 1; state <= 3; state++) {
                    RECT front{state * 0 + 4 + (state - 1) * cw, row * ch + 6, 4 + (state - 1) * cw + 24, row * ch + 34};
                    RECT behind{front.right - 2, front.top, front.right - 2 + pw, front.bottom};
                    DrawThemePartAlpha(md, g_theme.band, part, state, behind);
                    DrawThemePartAlpha(md, g_theme.band, 5, 1, front);
                }
                row++;
                if (row >= rows) break;
            }
            if (row >= rows) break;
        }
        GdiFlush();
        FILE* f = _wfopen(st, L"wb");
        BITMAPINFOHEADER hdr{sizeof(BITMAPINFOHEADER), W, -H, 1, 32, BI_RGB};
        BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + W * 4 * H), 0, 0, 54};
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&hdr, sizeof(hdr), 1, f);
        fwrite(bits, (size_t)W * 4 * H, 1, f);
        fclose(f);
    }    // TB_menuparts=1: the style's Menu-class parts (popup background, borders...) drawn 270x110 to menupart<N>.bmp.
    if (GetEnvironmentVariableW(L"TB_menuparts", nullptr, 0) && g_theme.menu) {
        for (int part = 7; part <= 16; part++)
            for (int state = 0; state <= 1; state++) {
                if (!IsThemePartDefined(g_theme.menu, part, state)) { wprintf(L"MENU part %d state %d undefined\n", part, state); continue; }
                int w = 270, h = 110;
                HDC sd = GetDC(nullptr);
                HDC md = CreateCompatibleDC(sd);
                BITMAPINFO bi{};
                bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 32, BI_RGB};
                uint32_t* bits = nullptr;
                HBITMAP dib = CreateDIBSection(sd, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
                SelectObject(md, dib);
                for (int i = 0; i < w * h; i++) bits[i] = 0xFFFF00FF;  // magenta: what the part leaves clear
                RECT r{0, 0, w, h};
                HRESULT hr = DrawThemeBackground(g_theme.menu, md, part, state, &r, nullptr);
                GdiFlush();
                wchar_t name[64];
                swprintf(name, 64, L"menupart%d_%d.bmp", part, state);
                FILE* f = _wfopen(name, L"wb");
                BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + w * h * 4), 0, 0, 54};
                fwrite(&fh, sizeof(fh), 1, f);
                fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
                fwrite(bits, w * h * 4, 1, f);
                fclose(f);
                wprintf(L"MENU part %d state %d hr=%08lX -> %ls\n", part, state, hr, name);
                DeleteDC(md); DeleteObject(dib); ReleaseDC(nullptr, sd);
            }
    }
    // Geometry for the comparison: the orb, islands and buttons, in bar coordinates.
    wprintf(L"GEOM orb %ld %ld %ld %ld\n", g_orbRect.left, g_orbRect.top, g_orbRect.right, g_orbRect.bottom);
    for (auto const& r : g_islands) wprintf(L"GEOM island %ld %ld %ld %ld\n", r.left, r.top, r.right, r.bottom);
    for (auto const& b : g_buttons) wprintf(L"GEOM button %ld %ld %ld %ld\n", b.rc.left, b.rc.top, b.rc.right, b.rc.bottom);
    for (auto const& tr : g_tray) wprintf(L"GEOM tray %ld %ld %ld %ld\n", tr.rc.left, tr.rc.top, tr.rc.right, tr.rc.bottom);
    wprintf(L"GEOM clock %ld %ld %ld %ld\n", g_clockRect.left, g_clockRect.top, g_clockRect.right, g_clockRect.bottom);    Wh_ModUninit();
    // What was behind the bar, so a diff shows only our pixels.
    if (argc > 2) {
        Sleep(700);
        Capture(argv[2], GetSystemMetrics(SM_CXSCREEN), rc.bottom - rc.top, rc.top);
    }
    wprintf(L"done\n");
    return 0;
}



