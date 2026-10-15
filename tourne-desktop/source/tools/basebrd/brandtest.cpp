// Icon Redirect's picture folders against Windows' own branding code, outside
// Windhawk: the part is compiled in, winbrand.dll's imports of the resource
// functions are pointed at its hooks, and BrandingLoadImage is asked for the
// logo. Each result is saved as out-<id>.bmp.
//   brandtest.exe <folder> [id ...]
#include <windows.h>
#include <cstdarg>
#include <cstdio>

// Windhawk's own headers, with the engine's functions supplied here.
#define WH_MOD
#define WH_MOD_ID L"brandtest"
#include <windhawk_api.h>

extern "C" {
BOOL InternalWh_IsLogEnabled(void*) { return TRUE; }
void InternalWh_Log(void*, PCWSTR format, va_list args) {
    vwprintf(format, args);
    wprintf(L"\n");
}
// Settings from environment variables TB_<name>.
PCWSTR InternalWh_GetStringSetting(void*, PCWSTR name, va_list args) {
    wchar_t formatted[128], var[160];
    vswprintf(formatted, 128, name, args);
    swprintf(var, 160, L"TB_%ls", formatted);
    auto* buf = new wchar_t[1024];
    if (!GetEnvironmentVariableW(var, buf, 1024)) buf[0] = 0;
    return buf;
}
void InternalWh_FreeStringSetting(void*, PCWSTR s) { delete[] s; }
int InternalWh_GetIntSetting(void*, PCWSTR name, va_list args) {
    wchar_t formatted[128], var[160], buf[32];
    vswprintf(formatted, 128, name, args);
    swprintf(var, 160, L"TB_%ls", formatted);
    return GetEnvironmentVariableW(var, buf, 32) ? _wtoi(buf) : 0;
}
size_t InternalWh_GetModStoragePath(void*, PWSTR out, size_t size) { return GetTempPathW((DWORD)size, out); }
// No hooking here: the originals are the real functions.
BOOL InternalWh_SetFunctionHook(void*, void* target, void*, void** original) {
    *original = target;
    return TRUE;
}
const WH_URL_CONTENT* InternalWh_GetUrlContent(void*, PCWSTR, const WH_GET_URL_CONTENT_OPTIONS*) { return nullptr; }
void InternalWh_FreeUrlContent(void*, const WH_URL_CONTENT*) {}
}

#include "../../tourne-icon-redirect.wh.cpp"

// Points one of a module's imports at a replacement.
bool PatchImport(HMODULE module, PCSTR name, void* replacement) {
    auto base = (BYTE*)module;
    auto nt = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
    auto dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    bool patched = false;
    for (auto* desc = (IMAGE_IMPORT_DESCRIPTOR*)(base + dir.VirtualAddress); desc->Name; desc++) {
        auto* names = (IMAGE_THUNK_DATA*)(base + desc->OriginalFirstThunk);
        auto* slots = (IMAGE_THUNK_DATA*)(base + desc->FirstThunk);
        for (; names->u1.AddressOfData; names++, slots++) {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
            auto* byName = (IMAGE_IMPORT_BY_NAME*)(base + names->u1.AddressOfData);
            if (strcmp((PCSTR)byName->Name, name) != 0) continue;
            DWORD old;
            VirtualProtect(&slots->u1.Function, sizeof(void*), PAGE_READWRITE, &old);
            slots->u1.Function = (ULONG_PTR)replacement;
            VirtualProtect(&slots->u1.Function, sizeof(void*), old, &old);
            patched = true;
        }
    }
    return patched;
}

void SaveBitmap(HBITMAP bitmap, PCWSTR path) {
    DIBSECTION ds{};
    if (GetObjectW(bitmap, sizeof(ds), &ds) != sizeof(ds) || !ds.dsBm.bmBits) return;
    BITMAPFILEHEADER file{0x4D42};
    BITMAPINFOHEADER info = ds.dsBmih;
    DWORD bytes = ds.dsBm.bmWidthBytes * ds.dsBm.bmHeight;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + bytes;
    HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    DWORD written;
    WriteFile(h, &file, sizeof(file), &written, nullptr);
    WriteFile(h, &info, sizeof(info), &written, nullptr);
    WriteFile(h, ds.dsBm.bmBits, bytes, &written, nullptr);
    CloseHandle(h);
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) return 1;
    SetEnvironmentVariableW(L"TB_brandingImages", argv[1]);
    SetEnvironmentVariableW(L"TB_allResourceRedirect", L"1");
    Wh_ModInit();
    HMODULE winbrand = LoadLibraryW(L"winbrand.dll");
    bool ok = PatchImport(winbrand, "FindResourceExW", (void*)FindResourceExW_Hook) |
              PatchImport(winbrand, "FindResourceW", (void*)FindResourceW_Hook);
    ok = PatchImport(winbrand, "LoadResource", (void*)LoadResource_Hook) && ok;
    ok = PatchImport(winbrand, "SizeofResource", (void*)SizeofResource_Hook) && ok;
    wprintf(L"winbrand imports patched: %d\n", ok);
    using Load_t = HANDLE(WINAPI*)(LPCWSTR, UINT, UINT, int, int, UINT);
    auto load = (Load_t)GetProcAddress(winbrand, "BrandingLoadImage");
    std::vector<UINT> ids;
    for (int i = 2; i < argc; i++) ids.push_back(_wtoi(argv[i]));
    if (ids.empty()) ids = {123, 1123, 2123, 124};
    for (UINT id : ids) {
        HANDLE bitmap = load(L"Basebrd", id, IMAGE_BITMAP, 0, 0, 0);
        BITMAP bm{};
        if (bitmap) GetObjectW(bitmap, sizeof(bm), &bm);
        wprintf(L"%u: %ldx%ld\n", id, bm.bmWidth, bm.bmHeight);
        wchar_t out[64];
        swprintf(out, 64, L"out-%u.bmp", id);
        if (bitmap) SaveBitmap((HBITMAP)bitmap, out);
    }
}
