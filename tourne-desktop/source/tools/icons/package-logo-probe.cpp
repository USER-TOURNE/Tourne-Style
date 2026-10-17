// How a packaged app's logo is read in a process (Task Manager shows Claude's package logo, not Claude.exe's
// icon): hooks CreateFileW and CreateFile2, then asks the shell for the app's picture the ways a program might,
// and prints every file opened under WindowsApps. Built with MinHook (desktop/third_party/minhook).
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <string>
#include <initializer_list>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <cstdio>
#include "MinHook.h"
thread_local int t_in = 0;
decltype(&CreateFileW) oCreateFileW;
decltype(&CreateFile2) oCreateFile2;
void Note(const wchar_t* api, LPCWSTR name) {
    if (t_in || !name || !wcsstr(name, L"WindowsApps")) return;
    t_in++;
    wprintf(L"  %ls %ls\n", api, name);
    t_in--;
}
HANDLE WINAPI hCreateFileW(LPCWSTR n, DWORD a, DWORD s, LPSECURITY_ATTRIBUTES sa, DWORD c, DWORD f, HANDLE t) {
    Note(L"CreateFileW", n);
    return oCreateFileW(n, a, s, sa, c, f, t);
}
HANDLE WINAPI hCreateFile2(LPCWSTR n, DWORD a, DWORD s, DWORD c, LPCREATEFILE2_EXTENDED_PARAMETERS p) {
    Note(L"CreateFile2", n);
    return oCreateFile2(n, a, s, c, p);
}
int wmain(int argc, wchar_t** argv) {
    const wchar_t* aumid = argc > 1 ? argv[1] : L"Claude_pzs8sxrjxfjjc!Claude";
    MH_Initialize();
    HMODULE kb = GetModuleHandleW(L"kernelbase.dll");
    MH_CreateHook((void*)GetProcAddress(kb, "CreateFileW"), (void*)hCreateFileW, (void**)&oCreateFileW);
    MH_CreateHook((void*)GetProcAddress(kb, "CreateFile2"), (void*)hCreateFile2, (void**)&oCreateFile2);
    MH_EnableHook(MH_ALL_HOOKS);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::wstring parsing = std::wstring(L"shell:AppsFolder\\") + aumid;
    IShellItem* item = nullptr;
    HRESULT hr = SHCreateItemFromParsingName(parsing.c_str(), nullptr, IID_PPV_ARGS(&item));
    wprintf(L"item 0x%08lx\n", hr);
    if (item) {
        IShellItemImageFactory* f = nullptr;
        if (SUCCEEDED(item->QueryInterface(IID_PPV_ARGS(&f)))) {
            for (int size : {16, 24, 32}) {
                HBITMAP bmp = nullptr;
                wprintf(L"GetImage %d (icon only)\n", size);
                hr = f->GetImage({size, size}, SIIGBF_ICONONLY, &bmp);
                wprintf(L"  -> 0x%08lx\n", hr);
                if (bmp) DeleteObject(bmp);
            }
            f->Release();
        }
        PIDLIST_ABSOLUTE pidl = nullptr;
        SHGetIDListFromObject(item, &pidl);
        SHFILEINFOW sfi{};
        wprintf(L"SHGetFileInfo (pidl, icon)\n");
        SHGetFileInfoW((LPCWSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON);
        wprintf(L"  icon %p\n", sfi.hIcon);
        IExtractIconW* extract = nullptr;
        wprintf(L"IExtractIconW location\n");
        if (SUCCEEDED(item->BindToHandler(nullptr, BHID_SFUIObject, IID_IExtractIconW, (void**)&extract))) {
            WCHAR loc[MAX_PATH] = L"";
            int index = 0;
            UINT flags = 0;
            extract->GetIconLocation(GIL_FORSHELL, loc, MAX_PATH, &index, &flags);
            wprintf(L"  location %ls,%d flags 0x%x\n", loc, index, flags);
            extract->Release();
        }
        item->Release();
    }
    fflush(stdout);
    return 0;
}
