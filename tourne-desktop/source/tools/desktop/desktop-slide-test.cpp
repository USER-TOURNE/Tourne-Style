// Plays desktop-slide.cpp's move for real: to the next desktop and back, each with the style given, saving a
// picture of the screens partway through each (slide-N.bmp in the folder given).
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -O1 -static -I. tools\desktop\desktop-slide-test.cpp
//     -o tools\desktop\desktop-slide-test.exe -ldwmapi -lole32 -luuid -lgdi32 -luser32 -ladvapi32
// Run: desktop-slide-test.exe <folder> [style, e.g. "slide" or "slidefade 30%"] [tenths]
#define WH_MOD_ID L"test"
#include <windows.h>
#include <dwmapi.h>

#include <cstdio>
#include <string>

std::wstring g_line = L"1, 6, default, slide";
PCWSTR Wh_GetStringSetting(PCWSTR key) {
    std::wstring k = key;
    return _wcsdup(k == L"motion.workspaces" ? g_line.c_str() : L"");
}
void Wh_FreeStringSetting(PCWSTR s) { free((void*)s); }

#include "motion.cpp"
#include "virtual-desktops.cpp"
#include "desktop-slide.cpp"

void Shot(std::wstring const& path) {
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN), y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN), h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    HDC screen = GetDC(nullptr), mem = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits;
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, x, y, SRCCOPY | CAPTUREBLT);
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

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::wstring dir = argc > 1 ? argv[1] : L".";
    std::wstring style = argc > 2 ? argv[2] : L"slide", tenths = argc > 3 ? argv[3] : L"6";
    g_line = L"1, " + tenths + L", default, " + style;
    std::vector<GUID> ids = VdReadGuids(L"VirtualDesktopIDs");
    if (ids.size() < 2) return wprintf(L"needs two desktops\n"), 1;
    int home = VdCurrentIndex(ids), next = (home + 1) % (int)ids.size();
    double ms = _wtof(tenths.c_str()) * 100;
    for (int round = 0; round < 2; round++) {
        ULONGLONG t0 = GetTickCount64();
        bool ok = VdmSwitchTo(ids[round == 0 ? next : home]);
        wprintf(L"switch %d: %ls in %llu ms\n", round, ok ? L"ok" : L"failed", GetTickCount64() - t0);
        Sleep((DWORD)(ms * 0.4));
        Shot(dir + L"\\slide-" + std::to_wstring(round) + L".bmp");
        if (g_slideThread) WaitForSingleObject(g_slideThread, 3000);
        Sleep(400);
    }
    return 0;
}
