// Runs the taskbar part outside Windhawk for a few seconds and captures it.
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
PCWSTR Wh_GetStringSetting(PCWSTR, ...) { return L""; }
void Wh_FreeStringSetting(PCWSTR) {}
int Wh_GetIntSetting(PCWSTR, ...) { return 1; }

#include "%USERPROFILE%/Downloads/tourne-suite-redirect-branch-wintweaks/taskbar-host.cpp"

static void Capture(PCWSTR path, int w, int h) {
    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 24, BI_RGB};
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, 0, 0, SRCCOPY | CAPTUREBLT);
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

int wmain(int argc, wchar_t** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (!Wh_ModInit()) {
        wprintf(L"init failed\n");
        return 1;
    }
    Sleep(2500);
    HWND bar = g_wnd;
    RECT rc{};
    GetWindowRect(bar, &rc);
    wprintf(L"bar %p at %ld,%ld %ldx%ld, %zu buttons, orb %dx%d (%d frames)\n", bar, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, g_items.size(), g_orb.width, g_orb.height, g_orb.frames);
    // Hover the first button, to see a hover state too.
    if (!g_items.empty()) {
        RECT b = g_items[0].rc;
        PostMessageW(bar, WM_MOUSEMOVE, 0, MAKELPARAM((b.left + b.right) / 2, (b.top + b.bottom) / 2));
    }
    Sleep(600);
    Capture(argc > 1 ? argv[1] : L"bar.bmp", GetSystemMetrics(SM_CXSCREEN), rc.bottom - rc.top);
    Wh_ModUninit();
    wprintf(L"done\n");
    return 0;
}
