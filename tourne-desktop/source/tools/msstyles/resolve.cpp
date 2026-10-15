// Asks uxtheme itself what a visual style gives for a class, part, state and property, and where the value
// came from (GetThemePropertyOrigin: the state, the part, the class, the globals, or not found). Settles how
// inheritance really works: a derived class "App::Class" missing a value, or missing altogether.
// The style file is loaded the way the suite loads it (uxtheme #92 and #16), never applied.
//   resolve.exe <queries.txt>   lines: file|app|class|part|state|propId|label   (app may be empty)
#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <stdio.h>
#include <map>
#include <string>

using LoaderLoadTheme_t = HRESULT(WINAPI*)(HANDLE, HINSTANCE, LPCWSTR, LPCWSTR, LPCWSTR, HANDLE*, LPWSTR, int, HANDLE*,
                                           LPWSTR, int, PVOID, HANDLE*, int, int, BOOL);
struct ThemeFileData {
    char header[7];
    void* sharableView;
    HANDLE sharableSection;
    void* nonSharableView;
    HANDLE nonSharableSection;
    char end[3];
};
using OpenThemeDataFromFile_t = HTHEME(WINAPI*)(ThemeFileData*, HWND, LPCWSTR, DWORD);

static bool Load(std::wstring const& path, ThemeFileData* file) {
    *file = {};
    HMODULE ux = LoadLibraryW(L"uxtheme.dll");
    auto load = (LoaderLoadTheme_t)GetProcAddress(ux, MAKEINTRESOURCEA(92));
    HANDLE a = nullptr, b = nullptr;
    HRESULT hr = load(nullptr, nullptr, path.c_str(), L"NormalColor", L"NormalSize", &a, nullptr, 0, &b, nullptr, 0,
                      nullptr, nullptr, 0, 0, FALSE);
    if (FAILED(hr) || !a || !b) { wprintf(L"load failed %ls 0x%08lX\n", path.c_str(), hr); return false; }
    memcpy(file->header, "thmfile", 7);
    memcpy(file->end, "end", 3);
    file->sharableSection = a;
    file->nonSharableSection = b;
    file->sharableView = MapViewOfFile(a, FILE_MAP_READ, 0, 0, 0);
    file->nonSharableView = MapViewOfFile(b, FILE_MAP_READ, 0, 0, 0);
    return true;
}

static PCWSTR Origin(PROPERTYORIGIN o) {
    switch (o) {
        case PO_STATE: return L"state";
        case PO_PART: return L"part";
        case PO_CLASS: return L"class";
        case PO_GLOBAL: return L"globals";
        default: return L"not found";
    }
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) return 1;
    auto open = (OpenThemeDataFromFile_t)GetProcAddress(LoadLibraryW(L"uxtheme.dll"), MAKEINTRESOURCEA(16));
    std::map<std::wstring, ThemeFileData> files;
    HWND host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    FILE* in = _wfopen(argv[1], L"r, ccs=UTF-8");
    wchar_t line[1024];
    while (fgetws(line, 1024, in)) {
        wchar_t* f[7] = {};
        int n = 0;
        for (wchar_t* p = line; n < 7;) {
            f[n++] = p;
            wchar_t* bar = wcschr(p, L'|');
            if (!bar) { p[wcscspn(p, L"\r\n")] = 0; break; }
            *bar = 0;
            p = bar + 1;
        }
        if (n < 6) continue;
        std::wstring path = f[0];
        if (!files.count(path) && !Load(path, &files[path])) continue;
        SetWindowTheme(host, *f[1] ? f[1] : L"", nullptr);
        HTHEME t = open(&files[path], host, f[2], 0);
        int part = _wtoi(f[3]), state = _wtoi(f[4]), prop = _wtoi(f[5]);
        if (!t) { wprintf(L"%ls|%ls|%ls|%d|%d|%d|no class\n", n > 6 ? f[6] : L"", f[1], f[2], part, state, prop); continue; }
        PROPERTYORIGIN origin = PO_NOTFOUND;
        GetThemePropertyOrigin(t, part, state, prop, &origin);
        wchar_t value[64] = L"-";
        COLORREF c;
        int i;
        if (SUCCEEDED(GetThemeColor(t, part, state, prop, &c)))
            swprintf(value, 64, L"%d, %d, %d", GetRValue(c), GetGValue(c), GetBValue(c));
        else if (SUCCEEDED(GetThemeInt(t, part, state, prop, &i)) || SUCCEEDED(GetThemeEnumValue(t, part, state, prop, &i)))
            swprintf(value, 64, L"%d", i);
        wprintf(L"%ls|%ls|%ls|%d|%d|%d|%ls|%ls|partDefined=%d\n", n > 6 ? f[6] : L"", f[1], f[2], part, state, prop,
                Origin(origin), value, IsThemePartDefined(t, part, 0));
        CloseThemeData(t);
    }
    return 0;
}
