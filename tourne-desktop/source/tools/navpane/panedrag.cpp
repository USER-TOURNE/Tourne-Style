// Explorer's preview pane, dragged in a real folder window without touching the pointer: finds the
// window (CabinetWClass, newest), saves it as a picture, and with "drag x y dx" posts a press on the
// frame's DirectUI window at x,y (client), moves by dx in steps and lets go, then saves it again.
//   panedrag shot <name>          capture to <name>.bmp, print the DirectUI windows
//   panedrag drag <x> <y> <dx> <name>
#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

static HWND g_cab;

static void Save(PCWSTR path, HWND h) {
    RECT r;
    GetWindowRect(h, &r);
    int w = r.right - r.left, hh = r.bottom - r.top;
    BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -hh, 1, 32, BI_RGB}};
    void* bits = nullptr;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bmp);
    PrintWindow(h, dc, 2);
    GdiFlush();
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(BITMAPINFOHEADER) + w * hh * 4), 0, 0,
                        sizeof(fh) + sizeof(BITMAPINFOHEADER)};
    FILE* f = _wfopen(path, L"wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(bits, (size_t)w * hh * 4, 1, f);
    fclose(f);
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
}

static std::vector<HWND> g_duis;
static BOOL CALLBACK Children(HWND h, LPARAM) {
    WCHAR cls[64];
    GetClassNameW(h, cls, 64);
    if (!wcscmp(cls, L"DirectUIHWND")) g_duis.push_back(h);
    return TRUE;
}

// The frame's DirectUI window: the one holding the whole page (the biggest).
static HWND FrameDui() {
    g_duis.clear();
    EnumChildWindows(g_cab, Children, 0);
    HWND best = nullptr;
    LONG area = 0;
    for (HWND h : g_duis) {
        RECT r;
        GetClientRect(h, &r);
        if (r.right * r.bottom > area) area = r.right * r.bottom, best = h;
    }
    return best;
}

int wmain(int argc, wchar_t** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_cab = FindWindowW(L"CabinetWClass", nullptr);  // the topmost folder window
    if (WCHAR h[32]; GetEnvironmentVariableW(L"PANE_HWND", h, 32)) g_cab = (HWND)(ULONG_PTR)wcstoull(h, nullptr, 16);
    if (!g_cab || argc < 3) {
        wprintf(L"no folder window or arguments\n");
        return 1;
    }
    RECT wr;
    GetWindowRect(g_cab, &wr);
    wprintf(L"folder window %p at %ld,%ld %ldx%ld\n", g_cab, wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top);
    HWND dui = FrameDui();
    for (HWND h : g_duis) {
        RECT r;
        GetWindowRect(h, &r);
        wprintf(L"  DirectUIHWND %p at %ld,%ld %ldx%ld%ls\n", h, r.left - wr.left, r.top - wr.top, r.right - r.left,
                r.bottom - r.top, h == dui ? L"  (frame)" : L"");
    }
    std::wstring what = argv[1];
    // auto <name>: the file view's right edge is the preview pane's sizer; the pane is grown (sizer 60 px
    // left) then shrunk (100 px right), and the view's width printed after each.
    // steps <name> <dx>...: as auto, then each dx dragged in turn (positive: the pane smaller), the view's
    // width printed after each.
    if (what == L"auto" || what == L"steps") {
        auto viewWidth = [&](RECT* rc) {
            FrameDui();
            HWND frame = FrameDui(), view = nullptr;
            LONG area = 0;
            for (HWND h : g_duis) {
                RECT r;
                GetClientRect(h, &r);
                if (h != frame && r.right * r.bottom > area) area = r.right * r.bottom, view = h;
            }
            GetWindowRect(view, rc);
            return rc->right - rc->left;
        };
        auto drag = [&](int screenX, int screenY, int dx) {
            POINT pt{screenX, screenY};
            ScreenToClient(dui, &pt);
            PostMessageW(dui, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
            Sleep(50);
            PostMessageW(dui, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
            Sleep(80);
            for (int i = 1; i <= 10; i++) {
                PostMessageW(dui, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pt.x + dx * i / 10, pt.y));
                Sleep(40);
            }
            PostMessageW(dui, WM_LBUTTONUP, 0, MAKELPARAM(pt.x + dx, pt.y));
            Sleep(400);
        };
        RECT view;
        int w0 = viewWidth(&view);
        int y = (view.top + view.bottom) / 2;
        // The sizer: the first spot right of the view where a small drag (pane 12 px bigger) moves anything.
        int at = -1;
        for (int off = 0; off <= 12 && at < 0; off++) {
            drag(view.right + off, y, -12);
            RECT now;
            if (viewWidth(&now) != w0) at = off;
        }
        if (at < 0) {
            wprintf(L"file view %d; no sizer found right of it\n", w0);
            Save((std::wstring(argv[2]) + L".bmp").c_str(), g_cab);
            return 0;
        }
        int w1 = viewWidth(&view);
        if (what == L"steps") {
            int before = w1;
            wprintf(L"view %d, sizer at +%d, after a 12 px grow %d\n", w0, at, w1);
            for (int i = 3; i < argc; i++) {
                int dx = _wtoi(argv[i]);
                drag(view.right + at, y, dx);
                int now = viewWidth(&view);
                wprintf(L"drag %+d: pane %+d (view %d)\n", dx, -(now - before), now);
                before = now;
            }
            Save((std::wstring(argv[2]) + L".bmp").c_str(), g_cab);
            return 0;
        }
        drag(view.right + at, y, 100);  // the pane 100 px smaller
        int w2 = viewWidth(&view);
        drag(view.right + at, y, -60);  // 60 px bigger
        int w3 = viewWidth(&view);
        drag(view.right + at, y, 100);  // smaller again
        int w4 = viewWidth(&view);
        wprintf(L"view %d, sizer at +%d; pane bigger by 12: %+d; smaller by 100: %+d; bigger by 60: %+d; smaller by 100: %+d\n",
                w0, at, -(w1 - w0), -(w2 - w1), -(w3 - w2), -(w4 - w3));
        Save((std::wstring(argv[2]) + L".bmp").c_str(), g_cab);
        return 0;
    }
    if (what == L"shot") {
        Save((std::wstring(argv[2]) + L".bmp").c_str(), g_cab);
        return 0;
    }
    if (what == L"drag" && argc >= 6 && dui) {
        int x = _wtoi(argv[2]), y = _wtoi(argv[3]), dx = _wtoi(argv[4]);
        // x and y are in the folder window's own picture: into the DirectUI window's client.
        POINT pt{wr.left + x, wr.top + y};
        ScreenToClient(dui, &pt);
        PostMessageW(dui, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
        Sleep(50);
        PostMessageW(dui, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
        Sleep(80);
        for (int i = 1; i <= 10; i++) {
            PostMessageW(dui, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pt.x + dx * i / 10, pt.y));
            Sleep(40);
        }
        PostMessageW(dui, WM_LBUTTONUP, 0, MAKELPARAM(pt.x + dx, pt.y));
        Sleep(400);
        Save((std::wstring(argv[5]) + L".bmp").c_str(), g_cab);
        wprintf(L"dragged from %d,%d by %d\n", x, y, dx);
    }
    return 0;
}
