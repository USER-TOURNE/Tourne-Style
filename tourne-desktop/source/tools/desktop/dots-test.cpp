// Reads tools/desktop/dots-sample/hyprland.conf through desktop/host/dots.cpp and prints what it would set, then
// exports the settings in the folder given (a copy of tourne-gui.conf there) to look.tourne and reads that back.
//
// Build (Windhawk's clang), from the repo:
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -O1 -static -Idesktop\host tools\desktop\dots-test.cpp desktop\host\dots.cpp desktop\host\extensions.cpp
//     desktop\host\json-lite.cpp desktop\host\config.cpp -o tools\desktop\dots-test.exe -lshell32
// Run: dots-test.exe <repo> <folder with a tourne-gui.conf>
#include <windows.h>

#include <cstdio>

#include "dots.h"

std::wstring g_folder;
namespace td {
std::wstring ConfigFolder() { return g_folder; }
}  // namespace td

int wmain(int argc, wchar_t** argv) {
    if (argc < 3) return 1;
    g_folder = argv[2];
    td::DotsLook look;
    std::wstring error;
    bool ok = td::DotsRead(std::wstring(argv[1]) + L"/tools/desktop/dots-sample/hyprland.conf", &look, &error);
    wprintf(L"hyprland.conf: %ls %ls\n", ok ? L"read" : L"not read", error.c_str());
    for (auto const& [k, v] : look.settings) wprintf(L"  %ls = %ls\n", k.c_str(), v.c_str());
    for (auto const& s : look.skipped) wprintf(L"  left out: %ls\n", s.c_str());
    std::wstring out = g_folder + L"/look.tourne";
    if (!td::DotsExport(out, L"test", &error)) return wprintf(L"export failed: %ls\n", error.c_str()), 1;
    td::DotsLook back;
    ok = td::DotsRead(out, &back, &error);
    wprintf(L"look.tourne: %ls, %d settings, %d left out, name %ls\n", ok ? L"read" : L"not read", (int)back.settings.size(),
            (int)back.skipped.size(), back.name.c_str());
    return 0;
}
