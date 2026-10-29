// tourne-appstyler-attach <pid> [colours]: puts Tourne App Styler (TourneAppStyler.dll, beside this) into a
// running XAML app through Windows' XAML diagnostics, for trying it by hand. Tourne Desktop does the same itself.
#include <windows.h>

#include <cstdio>
#include <string>

constexpr CLSID kClsid = {0x6f3c2b71, 0x3d44, 0x4e7b, {0x9c, 0x1a, 0x5b, 0x8e, 0x2f, 0x0d, 0x7a, 0x41}};
using Initialize = HRESULT(WINAPI*)(PCWSTR, DWORD, PCWSTR, PCWSTR, CLSID, PCWSTR);

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) return fwprintf(stderr, L"tourne-appstyler-attach <pid> [ground=#..;raised=#..;...]\n"), 2;
    DWORD pid = wcstoul(argv[1], nullptr, 10);
    std::wstring colours = argc > 2 ? argv[2] : L"";
    wchar_t self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    std::wstring dll = self;
    dll = dll.substr(0, dll.find_last_of(L'\\') + 1) + L"TourneAppStyler.dll";
    HMODULE xaml = LoadLibraryExW(L"Windows.UI.Xaml.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    auto init = xaml ? (Initialize)GetProcAddress(xaml, "InitializeXamlDiagnosticsEx") : nullptr;
    if (!init) return fwprintf(stderr, L"InitializeXamlDiagnosticsEx not found\n"), 1;
    // (the endpoint's name is the one Windows' own tools use; each attach to a process gets the next number)
    HRESULT hr = E_FAIL;
    for (int n = 1; n <= 8; n++) {
        std::wstring endpoint = L"VisualDiagConnection" + std::to_wstring(n);
        // (an app just started may not have its XAML up yet: asked again for a few seconds)
        for (int wait = 0; wait < 40; wait++) {
            hr = init(endpoint.c_str(), pid, nullptr, dll.c_str(), kClsid, colours.c_str());
            if (hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) break;
            Sleep(250);
        }
        wprintf(L"%ls: 0x%08lX\n", endpoint.c_str(), (unsigned long)hr);
        if (hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) && hr != HRESULT_FROM_WIN32(ERROR_PIPE_BUSY)) break;
    }
    wprintf(L"attach %lu: 0x%08lX\n", pid, (unsigned long)hr);
    return SUCCEEDED(hr) ? 0 : 1;
}
