// Plays window-motion.cpp for real: Character Map opened with the style given, a picture of the screen it's on
// partway (open-mid.bmp) and after (open-end.bmp), then closed.
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -O1 -static -I. tools\desktop\window-motion-test.cpp
//     -o tools\desktop\window-motion-test.exe -ldwmapi -lgdi32 -luser32
// Run: window-motion-test.exe <folder> ["popin 80%" | slide | gnomed | fade] [tenths]
#define WH_MOD_ID L"test"
#include <windows.h>
#include <dwmapi.h>

#include <cstdio>
#include <cwctype>
#include <string>

std::wstring g_line;
PCWSTR Wh_GetStringSetting(PCWSTR key) { return _wcsdup(std::wstring(key) == L"motion.windowsIn" ? g_line.c_str() : L""); }
void Wh_FreeStringSetting(PCWSTR s) { free((void*)s); }
void Wh_Log(PCWSTR f, ...) {}
std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

#include "motion.cpp"
#include "window-motion.cpp"

void Shot(std::wstring const& path) {
    HMONITOR mon = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    RECT r = mi.rcMonitor;
    int w = r.right - r.left, h = r.bottom - r.top;
    HDC screen = GetDC(nullptr), mem = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits;
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, r.left, r.top, SRCCOPY | CAPTUREBLT);
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader) + w * h * 4), 0, 0, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader))};
    if (FILE* f = _wfopen(path.c_str(), L"wb")) {
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&bi.bmiHeader, sizeof(bi.bmiHeader), 1, f);
        fwrite(bits, 4, (size_t)w * h, f);
        fclose(f);
    }
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
}

void Pump(DWORD ms) {
    ULONGLONG end = GetTickCount64() + ms;
    while (GetTickCount64() < end) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
        Sleep(5);
    }
}

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    std::wstring dir = argc > 1 ? argv[1] : L".", style = argc > 2 ? argv[2] : L"popin 80%", tenths = argc > 3 ? argv[3] : L"8";
    g_line = L"1, " + tenths + L", default, " + style;
    StartWindowMotion();
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    wchar_t cmd[] = L"charmap.exe";
    if (!CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) return wprintf(L"no charmap\n"), 1;
    WaitForInputIdle(pi.hProcess, 3000);
    Pump((DWORD)(_wtof(tenths.c_str()) * 100 * 0.3));
    Shot(dir + L"\\open-mid.bmp");
    Pump(1500);
    Shot(dir + L"\\open-end.bmp");
    struct F { DWORD pid; HWND h; } f{pi.dwProcessId, nullptr};
    EnumWindows([](HWND h, LPARAM p) -> BOOL {
        auto* f = (F*)p;
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        if (pid == f->pid && IsWindowVisible(h)) f->h = h;
        return !f->h;
    }, (LPARAM)&f);
    wprintf(L"window %p ex %08lx alpha-layered %d\n", f.h, f.h ? GetWindowLongW(f.h, GWL_EXSTYLE) : 0, f.h ? !!(GetWindowLongW(f.h, GWL_EXSTYLE) & WS_EX_LAYERED) : -1);
    if (f.h) PostMessageW(f.h, WM_CLOSE, 0, 0);
    EndWindowMotion();
    return 0;
}
