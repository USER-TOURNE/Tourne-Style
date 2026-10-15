// Reads desktop/extensions' examples through desktop/host/extensions.cpp: the good one is added (into a temporary
// folder standing in for Tourne's), the one that sets a bind is refused.
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -O1 -static -Idesktop\host tools\desktop\extensions-test.cpp desktop\host\extensions.cpp
//     desktop\host\json-lite.cpp -o tools\desktop\extensions-test.exe -lshell32
// Run: extensions-test.exe <repo folder> <scratch folder>
#include <windows.h>

#include <cstdio>
#include <string>

#include "extensions.h"

std::wstring g_scratch;
namespace td {
std::wstring ConfigFolder() { return g_scratch; }
std::wstring Utf8ToWide(const std::string& s) {
    std::wstring w(s.size(), L'\0');
    w.resize(MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), (int)w.size()));
    return w;
}
std::string WideToUtf8(const std::wstring& w) {
    std::string s(w.size() * 3, '\0');
    s.resize(WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), (int)s.size(), nullptr, nullptr));
    return s;
}
}  // namespace td

int wmain(int argc, wchar_t** argv) {
    if (argc < 3) return 1;
    g_scratch = argv[2];
    std::wstring repo = argv[1], error;
    bool good = td::ExtensionAdd(repo + L"\\desktop\\extensions\\example-amber\\extension.json", &error);
    wprintf(L"amber: %ls %ls\n", good ? L"added" : L"refused", error.c_str());
    error.clear();
    bool bad = td::ExtensionAdd(repo + L"\\desktop\\extensions\\example-bad", &error);
    wprintf(L"bad: %ls (%ls)\n", bad ? L"added (wrong)" : L"refused", error.c_str());
    for (auto const& e : td::ExtensionList(true)) wprintf(L"  %ls %ls %ls, %d settings\n", e.builtIn ? L"built in" : L"added   ", e.id.c_str(), e.kind.c_str(), (int)e.settings.size());
    return good && !bad ? 0 : 1;
}
