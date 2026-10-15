////////////////////////////////////////////////////////////////////////////////
// Desktops changing, animated as Hyprland's workspaces ("// @inline desktop-slide.cpp", at the end of
// virtual-desktops.cpp): the motion lines workspacesIn and workspacesOut (or workspaces, global above them),
// styles slide, slidevert, fade, slidefade N%, slidefadevert N%. With no line set anywhere up the tree the
// switch is Windows' plain one, as before.
//
// Tourne doesn't draw other programs' windows, so for the length of the move a window of its own covers the
// screens and shows them through DWM thumbnails: the desktop you leave sliding (or fading) out, the one you go
// to coming in, and what's on every desktop (the wallpaper, the bar, pinned windows) staying still. Windows'
// own switch happens under it once it's up; it goes when the move ends, with the real windows already there.
// A tool window has no place on a desktop, so the cover isn't hidden by the switch itself. Its own thread,
// so a part's hooks never wait on it.

#include <shobjidl.h>

#include <atomic>

struct DesktopSlide {
    motion::Anim in, out;
    GUID from, to;
    int dir;  // 1: to a desktop further right
    HANDLE ready, go;
};

HANDLE g_slideThread;
std::atomic<bool> g_slideCancel;

LRESULT CALLBACK DesktopSlideProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_ERASEBKGND) {
        PaintDesktop((HDC)wParam);  // (under the thumbnails, in case nothing shows the wallpaper)
        return 1;
    }
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

DWORD WINAPI DesktopSlideThread(void* param) {
    auto* job = (DesktopSlide*)param;
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IVirtualDesktopManager* vdm = nullptr;
    CoCreateInstance(CLSID_VirtualDesktopManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&vdm));

    enum Kind { Still, Out, In };
    struct Item {
        HWND h;
        RECT r, monitor;
        Kind kind;
        bool topmost;
        HTHUMBNAIL thumb;
        SIZE source;
    };
    struct Walk {
        std::vector<Item> items;  // frontmost first
        IVirtualDesktopManager* vdm;
        DesktopSlide* job;
    } walk{{}, vdm, job};
    EnumWindows(
        [](HWND h, LPARAM p) -> BOOL {
            auto* w = (Walk*)p;
            if (w->items.size() >= 80) return FALSE;
            if (!IsWindowVisible(h) || IsIconic(h)) return TRUE;
            RECT r;
            if (!GetWindowRect(h, &r) || r.right - r.left < 8 || r.bottom - r.top < 8) return TRUE;
            // Overlays that let clicks through or show nothing (NVIDIA's, Tourne's own visualiser): their
            // thumbnails come out solid (DWM doesn't keep a layered window's see-through), so they're left out.
            LONG ex = GetWindowLongW(h, GWL_EXSTYLE);
            BYTE alpha = 255;
            DWORD how = 0;
            if ((ex & WS_EX_TRANSPARENT) || ((ex & WS_EX_LAYERED) && GetLayeredWindowAttributes(h, nullptr, &alpha, &how) && (how & LWA_ALPHA) && !alpha))
                return TRUE;
            GUID d{};
            bool onOne = w->vdm && SUCCEEDED(w->vdm->GetWindowDesktopId(h, &d)) && d != GUID{};
            DWORD cloaked = 0;
            DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
            Kind kind;
            if (onOne && d == w->job->from && !cloaked) kind = Out;
            else if (onOne && d == w->job->to) kind = In;
            else if (!onOne && !cloaked) kind = Still;
            else return TRUE;
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(MonitorFromRect(&r, MONITOR_DEFAULTTONEAREST), &mi);
            bool topmost = GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_TOPMOST;
            w->items.push_back({h, r, mi.rcMonitor, kind, topmost, nullptr, {}});
            return TRUE;
        },
        (LPARAM)&walk);
    if (vdm) vdm->Release();

    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN), vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN), vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    WNDCLASSW wc{};
    wc.lpfnWndProc = DesktopSlideProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneDesktopSlide";
    RegisterClassW(&wc);  // (once a process; a second register fails harmlessly)
    HWND cover = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT, wc.lpszClassName, L"",
                                 WS_POPUP, vx, vy, vw, vh, nullptr, nullptr, wc.hInstance, nullptr);

    // Thumbnails are drawn in the order made: what stays below, the desktop coming, the one going, then what's
    // kept on top.
    auto& items = walk.items;
    std::vector<Item*> order;
    for (int pass = 0; pass < 4; pass++)
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            int at = it->kind == Still ? (it->topmost ? 3 : 0) : it->kind == In ? 1 : 2;
            if (at == pass) order.push_back(&*it);
        }
    for (Item* i : order) {
        if (!cover || FAILED(DwmRegisterThumbnail(cover, i->h, &i->thumb))) {
            i->thumb = nullptr;
            continue;
        }
        if (FAILED(DwmQueryThumbnailSourceSize(i->thumb, &i->source)) || !i->source.cx) i->source = {i->r.right - i->r.left, i->r.bottom - i->r.top};
    }

    auto style = [](motion::Anim const& a) { return motion::StyleWord(a); };
    std::wstring inStyle = style(job->in), outStyle = style(job->out);
    // One window at its place for the progress e (0 to 1, or past it on an overshoot) of the move.
    auto place = [&](Item const& i, double e) {
        if (!i.thumb) return;
        std::wstring s = i.kind == In ? inStyle : outStyle;
        motion::Anim const& a = i.kind == In ? job->in : job->out;
        bool fade = s == L"fade" || s.rfind(L"slidefade", 0) == 0;
        bool vert = s == L"slidevert" || s == L"slidefadevert";
        double share = s == L"fade" ? 0 : s.rfind(L"slidefade", 0) == 0 ? motion::StylePercent(a, 0.2) : 1;
        RECT const& m = i.monitor;
        double span = (vert ? m.bottom - m.top : m.right - m.left) * share;
        double shift = 0, alpha = 1;
        if (i.kind == Out) shift = -job->dir * span * e, alpha = fade ? 1 - e : 1;
        if (i.kind == In) shift = job->dir * span * (1 - e), alpha = fade ? e : 1;
        RECT dest = i.r;
        OffsetRect(&dest, vert ? 0 : (int)lround(shift), vert ? (int)lround(shift) : 0);
        RECT seen;
        DWM_THUMBNAIL_PROPERTIES p{};
        p.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE | DWM_TNP_OPACITY | DWM_TNP_SOURCECLIENTAREAONLY;
        p.fSourceClientAreaOnly = FALSE;
        p.opacity = (BYTE)std::clamp(lround(alpha * 255), 0L, 255L);
        p.fVisible = IntersectRect(&seen, &dest, &m) && p.opacity > 0;
        if (p.fVisible) {
            // The part on its own screen only: a window going doesn't slide onto the next screen.
            double sx = (double)i.source.cx / std::max(1L, i.r.right - i.r.left), sy = (double)i.source.cy / std::max(1L, i.r.bottom - i.r.top);
            p.rcSource = {(LONG)lround((seen.left - dest.left) * sx), (LONG)lround((seen.top - dest.top) * sy),
                          (LONG)lround((seen.right - dest.left) * sx), (LONG)lround((seen.bottom - dest.top) * sy)};
            p.rcDestination = seen;
            OffsetRect(&p.rcDestination, -vx, -vy);
        }
        DwmUpdateThumbnailProperties(i.thumb, &p);
    };
    for (Item* i : order) place(*i, 0);
    if (cover) {
        SetWindowPos(cover, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        UpdateWindow(cover);
        DwmFlush();
        DwmFlush();  // (on screen before anything under it changes)
    }
    SetEvent(job->ready);
    WaitForSingleObject(job->go, 1000);

    double inMs = std::max(1.0, motion::Ms(job->in)), outMs = std::max(1.0, motion::Ms(job->out));
    ULONGLONG start = GetTickCount64();
    while (cover && !g_slideCancel) {
        double t = (double)(GetTickCount64() - start);
        double ein = motion::Ease(job->in, t / inMs), eout = motion::Ease(job->out, t / outMs);
        for (Item* i : order) place(*i, i->kind == In ? ein : eout);
        DwmFlush();
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
        if (t >= inMs && t >= outMs) break;
    }

    for (Item* i : order)
        if (i->thumb) DwmUnregisterThumbnail(i->thumb);
    if (cover) DestroyWindow(cover);
    if (SUCCEEDED(com)) CoUninitialize();
    CloseHandle(job->ready);
    CloseHandle(job->go);
    delete job;
    return 0;
}

// To a desktop: animated when a motion line says so, else straight there. False if Windows didn't switch.
bool VdmSwitchTo(GUID const& id) {
    std::vector<GUID> ids = VdReadGuids(L"VirtualDesktopIDs");
    int from = VdCurrentIndex(ids), to = -1;
    for (size_t i = 0; i < ids.size(); i++)
        if (ids[i] == id) to = (int)i;
    motion::Anim in = motion::Get(L"workspacesIn"), out = motion::Get(L"workspacesOut");
    bool animate = to >= 0 && to != from && in.set && in.on && motion::Ms(in) > 0 && !GetSystemMetrics(SM_REMOTESESSION);
    if (g_slideThread) {
        // One still moving: to its end at once, so the next starts from the desktop really shown.
        g_slideCancel = true;
        WaitForSingleObject(g_slideThread, 1000);
        CloseHandle(g_slideThread);
        g_slideThread = nullptr;
    }
    if (!animate) return VdmSwitchNow(id);
    g_slideCancel = false;
    auto* job = new DesktopSlide{in, out, ids[from], id, to > from ? 1 : -1, CreateEventW(nullptr, TRUE, FALSE, nullptr),
                                 CreateEventW(nullptr, TRUE, FALSE, nullptr)};
    HANDLE ready = job->ready, go = job->go;
    g_slideThread = CreateThread(nullptr, 0, DesktopSlideThread, job, 0, nullptr);
    if (!g_slideThread) {
        CloseHandle(ready);
        CloseHandle(go);
        delete job;
        return VdmSwitchNow(id);
    }
    WaitForSingleObject(ready, 250);
    bool ok = VdmSwitchNow(id);
    if (!ok) g_slideCancel = true;
    SetEvent(go);
    return ok;
}
