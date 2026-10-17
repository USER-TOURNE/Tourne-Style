// Tests desktop/host/plugins.cpp outside Tourne Desktop: reads the built-in
// index, prints each plugin's card lines, and installs and removes one in a
// scratch folder (TOURNE_PLUGINS_DIR).
//
//   clang++ -std=c++23 -DUNICODE -D_UNICODE -municode -static -I desktop/host -I desktop/build/gen
//     tools/plugins/plugintest.cpp desktop/host/plugins.cpp desktop/host/json-lite.cpp
//     -lwinhttp -lole32 -lshell32 -luuid -o plugintest.exe
//   set TOURNE_PLUGINS_DIR=<scratch> & plugintest.exe [plugin id to install]
#include <windows.h>

#include <cstdio>
#include <string>

#include "plugins.h"

namespace td {
std::wstring Utf8ToWide(const std::string& s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}
std::string WideToUtf8(const std::wstring& w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}
void LogV(const wchar_t* prefix, const wchar_t* format, va_list args) {
    wchar_t text[2048];
    _vsnwprintf(text, 2047, format, args);
    text[2047] = 0;
    wprintf(L"  [%ls] %ls\n", prefix, text);
}
}  // namespace td

int wmain(int argc, wchar_t** argv) {
    using namespace td;
    auto const& all = PluginCatalogue();
    wprintf(L"%zu plugins, %d left out\n", all.size(), PluginsLeftOut());
    for (auto const& p : all) {
        std::wstring shows;
        for (auto const& s : PluginSurfaces(p)) shows += (shows.empty() ? L"" : L", ") + s;
        wprintf(L"%-30ls %-34ls | %ls\n", p.name.c_str(), shows.c_str(), PluginNeedsLine(p).c_str());
    }
    if (argc < 2) return 0;
    for (auto const& p : all) {
        if (p.id != argv[1]) continue;
        wprintf(L"\nInstall %ls: it will be allowed to\n", p.id.c_str());
        for (auto const& n : PluginNeedsList(p)) wprintf(L"  - %ls\n", n.c_str());
        HWND sink = CreateWindowW(L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        PluginInstall(p, sink, WM_APP + 1);
        MSG m;
        DWORD start = GetTickCount();
        while (PluginStateOf(p) == PluginState::Installing && GetTickCount() - start < 60000) {
            while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
            Sleep(50);
        }
        PluginState s = PluginStateOf(p);
        wprintf(L"state: %ls %ls\n", s == PluginState::Installed ? L"installed" : s == PluginState::Failed ? L"failed" : L"?",
                PluginError(p).c_str());
        if (s != PluginState::Installed) return 1;
        WIN32_FIND_DATAW fd;
        std::wstring folder = PluginFolder(p);
        HANDLE f = FindFirstFileW((folder + L"\\*").c_str(), &fd);
        int count = 0;
        if (f != INVALID_HANDLE_VALUE) {
            do count += fd.cFileName[0] != L'.';
            while (FindNextFileW(f, &fd));
            FindClose(f);
        }
        wprintf(L"%d entries in %ls\n", count, folder.c_str());
        bool removed = PluginRemove(p);
        wprintf(L"removed: %ls, state now %ls\n", removed ? L"yes" : L"no",
                PluginStateOf(p) == PluginState::NotInstalled ? L"not installed" : L"?");
        return removed ? 0 : 1;
    }
    wprintf(L"no plugin %ls\n", argv[1]);
    return 1;
}
