// Points at a spot (a pager mark) long enough for its desktop preview, captures the preview from the screen
// into <file>.bmp, and puts the pointer back. Needs the PC unlocked.
//
//   pager-preview-test.exe <x> <y> <file.bmp>
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <cstdio>
#include <cstdlib>

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (argc < 4) return 2;
    POINT was;
    GetCursorPos(&was);
    int x = _wtoi(argv[1]), y = _wtoi(argv[2]);
    for (int i = 0; i < 4; i++) {  // (a few small moves, so the bar sees the pointer arrive)
        SetCursorPos(x - 3 + i * 2, y);
        mouse_event(MOUSEEVENTF_MOVE, 0, 0, 0, 0);
        Sleep(60);
    }
    HWND preview = nullptr;
    for (int i = 0; i < 40 && !(preview && IsWindowVisible(preview)); i++) {
        Sleep(50);
        preview = FindWindowW(L"TourneDesktopPreview_tourne-desktop", nullptr);
    }
    Sleep(300);
    RECT r{};
    bool shown = preview && IsWindowVisible(preview) && GetWindowRect(preview, &r);
    wprintf(L"preview %d: %ld,%ld %ldx%ld\n", shown, r.left, r.top, r.right - r.left, r.bottom - r.top);
    if (shown) {
        int w = r.right - r.left, h = r.bottom - r.top;
        HDC screen = GetDC(nullptr), dc = CreateCompatibleDC(screen);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 32, BI_RGB};
        void* bits = nullptr;
        HBITMAP bitmap = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        HGDIOBJ old = SelectObject(dc, bitmap);
        BitBlt(dc, 0, 0, w, h, screen, r.left, r.top, SRCCOPY | CAPTUREBLT);
        GdiFlush();
        HANDLE f = CreateFileW(argv[3], GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
        DWORD size = w * h * 4, wrote = 0;
        BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader) + size), 0, 0, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader))};
        WriteFile(f, &fh, sizeof(fh), &wrote, nullptr);
        WriteFile(f, &bi.bmiHeader, sizeof(bi.bmiHeader), &wrote, nullptr);
        WriteFile(f, bits, size, &wrote, nullptr);
        CloseHandle(f);
        SelectObject(dc, old);
        DeleteObject(bitmap);
        DeleteDC(dc);
        ReleaseDC(nullptr, screen);
    }
    SetCursorPos(was.x, was.y);
    return shown ? 0 : 1;
}
