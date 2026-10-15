// Loads a StartAllBack .msstyles through uxtheme's file ordinals and draws
// every part/state each class defines onto a contact sheet (a 32-bit BMP).
#include <windows.h>
#include <uxtheme.h>
#include <vssym32.h>

#include <cstdio>
#include <string>
#include <vector>

// uxtheme #92: parses a .msstyles into two memory sections.
using LoaderLoadTheme_t = HRESULT(WINAPI*)(HANDLE themeFile, HINSTANCE instance, LPCWSTR fileName, LPCWSTR color,
                                           LPCWSTR size, HANDLE* sharableSection, LPWSTR sharableName, int sharableChars,
                                           HANDLE* nonSharableSection, LPWSTR nonSharableName, int nonSharableChars,
                                           PVOID customLoadHandler, HANDLE* reuseSection, int a, int b, BOOL emulateGlobal);
// What #16 takes as the loaded file: the two sections, mapped, between markers.
struct ThemeFile {
    char header[7];
    void* sharableView;
    HANDLE sharableSection;
    void* nonSharableView;
    HANDLE nonSharableSection;
    char end[3];
};
using OpenThemeDataFromFile_t = HTHEME(WINAPI*)(ThemeFile* themeFile, HWND wnd, LPCWSTR classList, DWORD flags);

struct Cell {
    int part, state;
    SIZE size;
};

int wmain(int argc, wchar_t** argv) {
    if (argc < 4) {
        fwprintf(stderr, L"sheet <file.msstyles> <class> <out.bmp> [cellW cellH]\n");
        return 1;
    }
    setvbuf(stdout, nullptr, _IONBF, 0);
    int cellW = argc > 5 ? _wtoi(argv[4]) : 160;
    int cellH = argc > 5 ? _wtoi(argv[5]) : 48;
    HMODULE ux = LoadLibraryW(L"uxtheme.dll");
    auto loadTheme = (LoaderLoadTheme_t)GetProcAddress(ux, MAKEINTRESOURCEA(92));
    auto openFromFile = (OpenThemeDataFromFile_t)GetProcAddress(ux, MAKEINTRESOURCEA(16));
    wprintf(L"ordinals: #92=%p #16=%p\n", loadTheme, openFromFile);

    HANDLE sharable = nullptr, nonSharable = nullptr, reuse = nullptr;
    wchar_t sharableName[MAX_PATH]{}, nonSharableName[MAX_PATH]{};
    wprintf(L"calling LoaderLoadTheme\n");
    (void)sharableName;
    (void)nonSharableName;
    (void)reuse;
    HRESULT hr = loadTheme(nullptr, nullptr, argv[1], L"NormalColor", L"NormalSize", &sharable, nullptr, 0,
                           &nonSharable, nullptr, 0, nullptr, nullptr, 0, 0, FALSE);
    wprintf(L"LoaderLoadTheme: 0x%08lX sharable=%p nonSharable=%p\n", hr, sharable, nonSharable);
    if (FAILED(hr) || !sharable || !nonSharable) {
        return 2;
    }
    ThemeFile file{};
    memcpy(file.header, "thmfile", 7);
    memcpy(file.end, "end", 3);
    file.sharableSection = sharable;
    file.nonSharableSection = nonSharable;
    file.sharableView = MapViewOfFile(sharable, FILE_MAP_READ, 0, 0, 0);
    file.nonSharableView = MapViewOfFile(nonSharable, FILE_MAP_READ, 0, 0, 0);
    wprintf(L"views: %p %p\n", file.sharableView, file.nonSharableView);
    HWND wnd = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, nullptr, nullptr);
    // "App::Class" is the window's theme app name plus the class.
    std::wstring classArg = argv[2];
    size_t sep = classArg.find(L"::");
    if (sep != std::wstring::npos) {
        SetWindowTheme(wnd, classArg.substr(0, sep).c_str(), nullptr);
        classArg = classArg.substr(sep + 2);
    }
    HTHEME theme = openFromFile(&file, wnd, classArg.c_str(), 0);
    wprintf(L"OpenThemeDataFromFile(%ls): %p\n", argv[2], theme);
    if (!theme) {
        return 3;
    }

    // Parts that exist, and the states that have their own image or colour.
    std::vector<Cell> cells;
    for (int part = 0; part <= 80; part++) {
        if (part && !IsThemePartDefined(theme, part, 0)) {
            continue;
        }
        for (int state = 0; state <= 12; state++) {
            // The part's own look (state 0), and each state that changes it.
            PROPERTYORIGIN best = PO_NOTFOUND;
            for (int prop : {TMT_IMAGEFILE, TMT_IMAGEFILE1, TMT_FILLCOLOR, TMT_BGTYPE, TMT_BORDERCOLOR}) {
                PROPERTYORIGIN o = PO_NOTFOUND;
                if (SUCCEEDED(GetThemePropertyOrigin(theme, part, state, prop, &o)) && o < best) {
                    best = o;
                }
            }
            // States usually share one image strip (ImageCount frames), so
            // every frame of a part counts as a state.
            int frames = 1;
            GetThemeInt(theme, part, 0, TMT_IMAGECOUNT, &frames);
            bool own = (state == 0 && best <= PO_CLASS && frames <= 1) || (state > 0 && best == PO_STATE) ||
                       (state > 0 && state <= frames && frames > 1);
            if (!own) {
                continue;
            }
            PROPERTYORIGIN origin = best, fill = best;
            SIZE size{};
            GetThemePartSize(theme, nullptr, part, state, nullptr, TS_TRUE, &size);
            cells.push_back({part, state, size});
            wprintf(L"  part %2d state %2d  true size %ldx%ld  origin img=%d fill=%d\n", part, state, size.cx, size.cy,
                    origin, fill);
        }
    }
    if (cells.empty()) {
        wprintf(L"no parts\n");
        return 4;
    }

    int columns = 6;
    int labelH = 14;
    int pad = 8;
    int rows = ((int)cells.size() + columns - 1) / columns;
    int W = columns * (cellW + pad) + pad;
    int H = rows * (cellH + labelH + pad) + pad;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), W, -H, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP dib = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    SelectObject(dc, dib);
    // A checkerboard shows what's transparent.
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            ((DWORD*)bits)[y * W + x] = ((x / 8 + y / 8) & 1) ? 0xFF3A3A3A : 0xFF2A2A2A;
        }
    }
    HFONT font = CreateFontW(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, NONANTIALIASED_QUALITY, 0, L"Consolas");
    SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(120, 220, 160));
    for (size_t i = 0; i < cells.size(); i++) {
        int cx = pad + (int)(i % columns) * (cellW + pad);
        int cy = pad + (int)(i / columns) * (cellH + labelH + pad);
        wchar_t label[64];
        swprintf(label, 64, L"p%d s%d (%ldx%ld)", cells[i].part, cells[i].state, cells[i].size.cx, cells[i].size.cy);
        TextOutW(dc, cx, cy, label, (int)wcslen(label));
        RECT rc{cx, cy + labelH, cx + cellW, cy + labelH + cellH};
        DrawThemeBackground(theme, dc, cells[i].part, cells[i].state, &rc, nullptr);
    }
    GdiFlush();

    // 32-bit BMP, top-down rows written bottom-up.
    FILE* out = _wfopen(argv[3], L"wb");
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + W * H * 4), 0, 0,
                        sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)};
    BITMAPINFOHEADER ih = bi.bmiHeader;
    ih.biHeight = H;
    fwrite(&fh, sizeof(fh), 1, out);
    fwrite(&ih, sizeof(ih), 1, out);
    for (int y = H - 1; y >= 0; y--) {
        fwrite((DWORD*)bits + y * W, 4, W, out);
    }
    fclose(out);
    wprintf(L"%zu cells -> %ls\n", cells.size(), argv[3]);

    CloseThemeData(theme);
    return 0;
}
