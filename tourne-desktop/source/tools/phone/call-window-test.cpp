// Shows Tourne Desktop's call window for a made-up call (as Tourne Calls would tell it), with moving levels,
// captures it from the screen into <folder>\call-window.bmp, then hides it again. Needs the PC unlocked.
//
//   call-window-test.exe <folder> [phone]
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

struct CallData {
    uint32_t on, status, here, muted, sound, seconds;
    wchar_t name[128], mic[128], out[128];
};

bool Tell(HWND tourne, CallData const& d) {
    COPYDATASTRUCT copy{0x4C414354, sizeof(d), (void*)&d};
    DWORD_PTR result = 0;
    return SendMessageTimeoutW(tourne, WM_COPYDATA, 0, (LPARAM)&copy, SMTO_ABORTIFHUNG, 2000, &result) && result;
}

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (argc < 2) return 2;
    std::wstring folder = argv[1];
    bool phone = argc > 2;
    HWND tourne = FindWindowW(L"TourneStartMenu_tourne-desktop", nullptr);
    if (!tourne) return wprintf(L"no Tourne Desktop\n"), 1;
    CallData d{1, 3, phone ? 0u : 1u, 0, phone ? 0u : 1u, 83};
    wcscpy(d.name, L"Alex Morgan");
    wcscpy(d.mic, L"Microphone (Tourne Headset)");
    wcscpy(d.out, L"Headphones (Tourne Headset Chat)");
    wprintf(L"told: %d\n", Tell(tourne, d));
    UINT levels = RegisterWindowMessageW(L"TourneCallsLevels");
    for (int i = 0; i < 30; i++) {
        PostMessageW(tourne, levels, 500 + (int)(400 * std::sin(i * 0.7)), 150 + (int)(100 * std::cos(i * 0.5)));
        Sleep(50);
    }
    HWND call = FindWindowW(L"TourneStartCall_tourne-desktop", nullptr);
    RECT r{};
    if (!call || !GetWindowRect(call, &r)) return wprintf(L"no call window\n"), 1;
    wprintf(L"window %ld,%ld %ldx%ld visible %d\n", r.left, r.top, r.right - r.left, r.bottom - r.top, IsWindowVisible(call));
    int w = r.right - r.left + 24, h = r.bottom - r.top + 24;
    HDC screen = GetDC(nullptr), dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bitmap);
    BitBlt(dc, 0, 0, w, h, screen, r.left - 12, r.top - 12, SRCCOPY | CAPTUREBLT);
    GdiFlush();
    std::wstring path = folder + L"\\call-window" + (phone ? L"-phone" : L"") + L".bmp";
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
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
    d.on = 0;
    wprintf(L"hidden: %d\n", Tell(tourne, d));
    return 0;
}
