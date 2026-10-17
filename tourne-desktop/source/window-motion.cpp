////////////////////////////////////////////////////////////////////////////////
// Windows opening, animated as Hyprland's ("// @inline window-motion.cpp", in the Switcher after
// window-rules.cpp): the motion line windowsIn (or windows, global above it), styles popin N% (the default,
// 85%), slide [left|right|top|bottom] (from the nearest edge if not said), gnomed, fade; fadeIn, when set,
// times the fade on its own.
//
// Tourne doesn't draw other programs' windows and Windows won't let another program hide one (DWMWA_CLOAK
// answers access denied), so as a window shows: Windows' own transition for it is turned off
// (DWMWA_TRANSITIONS_FORCEDISABLED, which it does allow), the window is made see-through (layered, alpha 0;
// DWM thumbnails still show what it draws), and a click-through cover over its screen moves its thumbnail in.
// At the end the window gets its own style back, and the cover goes once it has drawn. Windows that are
// already layered, not normal app windows, the size of the screen (games, video) or Windows' own app frames
// are left as they are, and so is everything while no line is set. Closing, minimising and maximising stay
// Windows' own (a window is gone before anything outside it hears it's closing).

#include <atomic>
#include <unordered_map>

struct OpeningWindow {
    HWND h;
    LONG ex;  // its own extended style, put back at the end
    motion::Anim in, fade;
};

HWINEVENTHOOK g_openHook;
motion::Anim g_openIn, g_openFade;
std::unordered_map<HWND, ULONGLONG> g_openedAt;  // (a window shown again at once isn't animated twice)
std::atomic<int> g_openingNow;

LRESULT CALLBACK OpeningCoverProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_ERASEBKGND) {
        RECT r;
        GetClientRect(wnd, &r);
        FillRect((HDC)wParam, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));  // (see-through: the frame covers it all)
        return 1;
    }
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

void PutBackOpening(OpeningWindow const& o) {
    if (!IsWindow(o.h)) return;
    SetLayeredWindowAttributes(o.h, 0, 255, LWA_ALPHA);
    SetWindowLongW(o.h, GWL_EXSTYLE, o.ex);
    RedrawWindow(o.h, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME | RDW_UPDATENOW);
    BOOL off = FALSE;
    DwmSetWindowAttribute(o.h, DWMWA_TRANSITIONS_FORCEDISABLED, &off, sizeof(off));
}

DWORD WINAPI OpeningThread(void* param) {
    auto* o = (OpeningWindow*)param;
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RECT r;
    GetWindowRect(o->h, &r);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi);
    RECT m = mi.rcMonitor;

    WNDCLASSW wc{};
    wc.lpfnWndProc = OpeningCoverProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneOpeningCover";
    RegisterClassW(&wc);
    HWND cover = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT, wc.lpszClassName, L"", WS_POPUP,
                                 m.left, m.top, m.right - m.left, m.bottom - m.top, nullptr, nullptr, wc.hInstance, nullptr);
    HTHUMBNAIL thumb = nullptr;
    if (cover) {
        MARGINS all{-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(cover, &all);
        if (FAILED(DwmRegisterThumbnail(cover, o->h, &thumb))) thumb = nullptr;
    }

    std::wstring style = motion::StyleWord(o->in);
    std::wstring rest = Lower(o->in.style);
    double from = motion::StylePercent(o->in, 0.85);
    // Which edge a slide comes from: said, or the nearest.
    int edge = -1;  // 0 left, 1 right, 2 top, 3 bottom
    if (rest.find(L"left") != std::wstring::npos) edge = 0;
    else if (rest.find(L"right") != std::wstring::npos) edge = 1;
    else if (rest.find(L"top") != std::wstring::npos) edge = 2;
    else if (rest.find(L"bottom") != std::wstring::npos) edge = 3;
    if (edge < 0) {
        long d[4] = {(r.left + r.right) / 2 - m.left, m.right - (r.left + r.right) / 2, (r.top + r.bottom) / 2 - m.top,
                     m.bottom - (r.top + r.bottom) / 2};
        edge = (int)(std::min_element(d, d + 4) - d);
    }

    // (at most 1.5 s: the window is see-through until the end)
    double inMs = std::clamp(motion::Ms(o->in), 1.0, 1500.0), fadeMs = o->fade.set && o->fade.on ? std::clamp(motion::Ms(o->fade), 1.0, 1500.0) : inMs;
    ULONGLONG start = GetTickCount64();
    bool shown = false;
    while (thumb && IsWindow(o->h)) {
        double t = (double)(GetTickCount64() - start);
        double e = motion::Ease(o->in, t / inMs);
        double f = o->fade.set && o->fade.on ? motion::Ease(o->fade, t / fadeMs) : std::clamp(t / (inMs * 0.6), 0.0, 1.0);
        GetWindowRect(o->h, &r);  // (tiling or a rule may move it while it comes in)
        double w = r.right - r.left, h = r.bottom - r.top, cx = (r.left + r.right) / 2.0, cy = (r.top + r.bottom) / 2.0;
        double l = r.left, tp = r.top, rr = r.right, b = r.bottom, alpha = 1;
        if (style == L"slide") {
            double dx = 0, dy = 0;
            if (edge == 0) dx = (m.left - r.right) * (1 - e);
            if (edge == 1) dx = (m.right - r.left) * (1 - e);
            if (edge == 2) dy = (m.top - r.bottom) * (1 - e);
            if (edge == 3) dy = (m.bottom - r.top) * (1 - e);
            l += dx, rr += dx, tp += dy, b += dy;
        } else if (style == L"gnomed") {
            double sh = std::max(0.0, e);
            tp = cy - h * sh / 2, b = cy + h * sh / 2;
            alpha = f;
        } else if (style == L"fade") {
            alpha = f;
        } else {  // popin
            double s = from + (1 - from) * e;
            l = cx - w * s / 2, rr = cx + w * s / 2, tp = cy - h * s / 2, b = cy + h * s / 2;
            alpha = f;
        }
        DWM_THUMBNAIL_PROPERTIES p{};
        p.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION | DWM_TNP_OPACITY | DWM_TNP_SOURCECLIENTAREAONLY;
        p.fSourceClientAreaOnly = FALSE;
        p.fVisible = TRUE;
        p.opacity = (BYTE)std::clamp(lround(alpha * 255), 0L, 255L);
        p.rcDestination = {lround(l) - m.left, lround(tp) - m.top, lround(rr) - m.left, lround(b) - m.top};
        DwmUpdateThumbnailProperties(thumb, &p);
        if (!shown) {
            SetWindowPos(cover, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            shown = true;
        }
        DwmFlush();
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
        if (t >= inMs && t >= fadeMs) break;
    }

    PutBackOpening(*o);
    DwmFlush();
    DwmFlush();  // (the window drawn again before the cover goes)
    if (thumb) DwmUnregisterThumbnail(thumb);
    if (cover) DestroyWindow(cover);
    g_openingNow--;
    delete o;
    return 0;
}

// A window worth animating as it opens: a normal app window or its dialog, on screen, not the size of it.
bool OpensAnimated(HWND h) {
    if (!h || !IsWindowVisible(h) || IsIconic(h) || IsZoomed(h) || GetAncestor(h, GA_ROOT) != h) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((ex & (WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOREDIRECTIONBITMAP)) || (style & WS_CHILD)) return false;
    if ((ex & WS_EX_NOACTIVATE) && !(ex & WS_EX_APPWINDOW)) return false;
    if (GetWindow(h, GW_OWNER) && (style & WS_CAPTION) != WS_CAPTION) return false;  // (owned: dialogs only)
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    for (PCWSTR skip : {L"ApplicationFrameWindow", L"Windows.UI.Core.CoreWindow", L"Shell_TrayWnd", L"Progman", L"WorkerW", L"#32768",
                        L"tooltips_class32", L"ConsoleWindowClass"})
        if (!wcscmp(cls, skip)) return false;
    if (!wcsncmp(cls, L"Tourne", 6)) return false;
    RECT r;
    GetWindowRect(h, &r);
    if (r.right - r.left < 120 || r.bottom - r.top < 80) return false;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi);
    RECT both;
    IntersectRect(&both, &r, &mi.rcMonitor);
    if (EqualRect(&both, &mi.rcMonitor)) return false;  // (full screen)
    return true;
}

void CALLBACK OpenEventProc(HWINEVENTHOOK, DWORD, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || g_openingNow >= 6 || !OpensAnimated(h)) return;
    ULONGLONG now = GetTickCount64();
    if (auto it = g_openedAt.find(h); it != g_openedAt.end() && now - it->second < 1500) return;
    if (g_openedAt.size() > 200) g_openedAt.clear();
    g_openedAt[h] = now;
    // Hidden at once, here, before the thread is even up: the sooner, the less of it is seen first.
    auto* o = new OpeningWindow{h, GetWindowLongW(h, GWL_EXSTYLE), g_openIn, g_openFade};
    BOOL on = TRUE;
    DwmSetWindowAttribute(h, DWMWA_TRANSITIONS_FORCEDISABLED, &on, sizeof(on));
    SetWindowLongW(h, GWL_EXSTYLE, o->ex | WS_EX_LAYERED);
    if (!SetLayeredWindowAttributes(h, 0, 0, LWA_ALPHA)) {
        PutBackOpening(*o);
        delete o;
        return;
    }
    g_openingNow++;
    if (HANDLE t = CreateThread(nullptr, 0, OpeningThread, o, 0, nullptr)) {
        CloseHandle(t);
    } else {
        g_openingNow--;
        PutBackOpening(*o);
        delete o;
    }
}

// The line read again (settings changed): the hook there only while windows open animated.
void StartWindowMotion() {
    g_openIn = motion::Get(L"windowsIn");
    g_openFade = motion::Get(L"fadeIn");
    bool want = g_openIn.set && g_openIn.on && motion::Ms(g_openIn) > 0 && !GetSystemMetrics(SM_REMOTESESSION);
    if (want == (g_openHook != nullptr)) return;
    if (g_openHook) UnhookWinEvent(g_openHook), g_openHook = nullptr;
    if (want) {
        g_openHook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, OpenEventProc, 0, 0,
                                     WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        Wh_Log(L"motion: windows open animated (%s)", g_openIn.style.empty() ? L"popin" : g_openIn.style.c_str());
    }
}

void EndWindowMotion() {
    if (g_openHook) UnhookWinEvent(g_openHook), g_openHook = nullptr;
    // (an animation still running puts its window back itself)
}
