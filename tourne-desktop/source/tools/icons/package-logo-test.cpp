// Icon Redirect (tourne-icon-redirect.wh.cpp) run outside Windhawk with b53's iconRedirect settings, to check
// packaged apps' logos: the shell asks for Claude's picture, and the redirected logo files are listed.
// Build: see the bash line in the session note (clang++ -Ishim, MinHook objects).
#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <string>
#include "MinHook.h"
std::wstring g_storage;
std::wstring SettingName(PCWSTR name, va_list args) {
    wchar_t key[512];
    _vsnwprintf(key, 511, name, args);
    key[511] = 0;
    return std::wstring(L"iconRedirect.") + key;
}
std::wstring ReadSetting(std::wstring const& key) {
    HKEY k;
    std::wstring out;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods\\local@tourne-suite-b53\\Settings", 0, KEY_READ, &k)) return out;
    wchar_t buf[4096];
    DWORD size = sizeof(buf), type = 0;
    if (!RegQueryValueExW(k, key.c_str(), nullptr, &type, (BYTE*)buf, &size)) {
        if (type == REG_SZ) out = buf;
        else if (type == REG_DWORD) out = std::to_wstring(*(DWORD*)buf);
    }
    RegCloseKey(k);
    return out;
}
PCWSTR Wh_GetStringSetting(PCWSTR name, ...) {
    va_list a;
    va_start(a, name);
    std::wstring v = ReadSetting(SettingName(name, a));
    va_end(a);
    return _wcsdup(v.c_str());
}
void Wh_FreeStringSetting(PCWSTR s) { free((void*)s); }
int Wh_GetIntSetting(PCWSTR name, ...) {
    va_list a;
    va_start(a, name);
    std::wstring v = ReadSetting(SettingName(name, a));
    va_end(a);
    return _wtoi(v.c_str());
}
void Wh_Log(PCWSTR f, ...) {
    va_list a;
    va_start(a, f);
    wprintf(L"[log] ");
    vwprintf(f, a);
    wprintf(L"\n");
    va_end(a);
}
size_t Wh_GetModStoragePath(PWSTR b, size_t n) {
    wcsncpy(b, g_storage.c_str(), n);
    return wcslen(b);
}
BOOL Wh_SetFunctionHook(void* t, void* h, void** o) { return MH_CreateHook(t, h, o) == MH_OK; }
struct WH_GET_URL_CONTENT_OPTIONS { size_t optionsSize; PCWSTR targetFilePath; };
struct WH_URL_CONTENT { const char* data; size_t length; int statusCode; };
const WH_URL_CONTENT* Wh_GetUrlContent(PCWSTR, const WH_GET_URL_CONTENT_OPTIONS*) { return nullptr; }
void Wh_FreeUrlContent(const WH_URL_CONTENT*) {}
namespace WindhawkUtils {
template <typename P> BOOL SetFunctionHook(P* t, P* h, P** o) { return Wh_SetFunctionHook((void*)t, (void*)h, (void**)o); }
}
#define WH_MOD_ID L"test"
#include "icon-redirect-expanded.cpp"
#include <shlobj.h>
#include <shobjidl.h>
int wmain(int argc, wchar_t** argv) {
    WCHAR temp[MAX_PATH];
    GetTempPathW(MAX_PATH, temp);
    g_storage = std::wstring(temp) + L"tourne-logo-test";
    CreateDirectoryW(g_storage.c_str(), nullptr);
    MH_Initialize();
    if (!Wh_ModInit()) return 1;
    MH_EnableHook(MH_ALL_HOOKS);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IShellItem* item = nullptr;
    HRESULT made = SHCreateItemFromParsingName(argc > 1 ? argv[1] : L"shell:AppsFolder\\Claude_pzs8sxrjxfjjc!Claude", nullptr, IID_PPV_ARGS(&item));
    wprintf(L"item 0x%08lx\n", made);
    if (!item) return 2;
    IShellItemImageFactory* f = nullptr;
    item->QueryInterface(IID_PPV_ARGS(&f));
    for (int size : {16, 24, 32, 48}) {
        HBITMAP bmp = nullptr;
        HRESULT hr = f->GetImage({size, size}, SIIGBF_ICONONLY, &bmp);
        BITMAP bm{};
        if (bmp) GetObjectW(bmp, sizeof(bm), &bm);
        wprintf(L"GetImage %d: 0x%08lx %ldx%ld\n", size, hr, bm.bmWidth, bm.bmHeight);
        if (bmp) DeleteObject(bmp);
    }
    for (auto const& [logo, used] : g_logos) wprintf(L"logo %ls\n  -> %ls\n", logo.c_str(), used.empty() ? L"(as it is)" : used.c_str());
    fflush(stdout);
    ExitProcess(0);
}
