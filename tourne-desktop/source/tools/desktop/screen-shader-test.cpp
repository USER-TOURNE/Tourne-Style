// Runs desktop/host/screen-shader.cpp's overlay for real for a few seconds on the main screen, with the effect
// given, printing its log, then whether clicks go through it (the window under the screen's middle isn't it).
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -O1 -static -Idesktop\host tools\desktop\screen-shader-test.cpp
//     desktop\host\screen-shader.cpp -o tools\desktop\screen-shader-test.exe -ld3d11 -ldxgi -ldcomp -luser32 -lgdi32
// Run: screen-shader-test.exe [effect] [seconds]
#include <windows.h>

#include <cstdio>
#include <map>
#include <string>

#include "config.h"
#include "screen-shader.h"

namespace td {
std::map<std::wstring, std::wstring> g_values;
int GetInt(const wchar_t*, const std::wstring& key) { return _wtoi(g_values[key].c_str()); }
const wchar_t* GetString(const wchar_t*, const std::wstring& key) { return _wcsdup(g_values[key].c_str()); }
void FreeString(const wchar_t* s) { free((void*)s); }
bool FlyoutPalette(uint32_t*) { return false; }
void LogV(const wchar_t* part, const wchar_t* format, va_list args) {
    fwprintf(stdout, L"[%ls] ", part);
    vfwprintf(stdout, format, args);
    fwprintf(stdout, L"\n");
    fflush(stdout);
}
}  // namespace td

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    td::g_values[L"screen"] = argc > 1 ? argv[1] : L"crt";
    td::g_values[L"amount"] = L"80";
    td::g_values[L"screens"] = L"main";
    td::g_values[L"pauseFullscreen"] = L"1";
    int seconds = argc > 2 ? _wtoi(argv[2]) : 6;
    td::ScreenShaderStart(nullptr);
    Sleep(1500);
    HWND overlay = FindWindowW(L"TourneScreenShader", nullptr);
    POINT mid{960, 540};
    HWND under = WindowFromPoint(mid);
    wchar_t cls[64] = L"";
    GetClassNameW(under, cls, 64);
    wprintf(L"overlay %p visible %d; under the middle: %ls (%ls)\n", overlay, overlay ? IsWindowVisible(overlay) : 0, cls,
            under == overlay ? L"the overlay: clicks would NOT go through" : L"clicks go through");
    Sleep(seconds * 1000 - 1500);
    td::ScreenShaderStop();
    wprintf(L"stopped; overlay now %p\n", FindWindowW(L"TourneScreenShader", nullptr));
    return 0;
}
