// Explorer Colours outside Explorer: drive bars and selections drawn from the
// current visual style through the part's own hooks, one row per setting
// set, saved (3x, over the style's dark ground) to colortest.bmp.
#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <vector>

#define WH_MOD
#define WH_MOD_ID L"colortest"
#include <windhawk_api.h>

extern "C" {
BOOL InternalWh_IsLogEnabled(void*) { return TRUE; }
void InternalWh_Log(void*, PCWSTR format, va_list args) {
    vwprintf(format, args);
    wprintf(L"\n");
}
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
BOOL InternalWh_SetFunctionHook(void*, void* target, void*, void** original) {
    *original = target;
    return TRUE;
}
}

#include "../../explorer-colors.cpp"

constexpr int kW = 260, kRowH = 24, kZoom = 3;

struct Canvas {
    HDC dc;
    HBITMAP bmp;
    DWORD* bits;
    int w, h;
};

Canvas MakeCanvas(int w, int h) {
    Canvas c{CreateCompatibleDC(nullptr), nullptr, nullptr, w, h};
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    c.bmp = CreateDIBSection(c.dc, &bi, DIB_RGB_COLORS, (void**)&c.bits, nullptr, 0);
    SelectObject(c.dc, c.bmp);
    for (int i = 0; i < w * h; i++) c.bits[i] = 0xFF121C21;
    return c;
}

void Save(Canvas const& c, PCWSTR path) {
    int w = c.w * kZoom, h = c.h * kZoom, stride = (w * 3 + 3) & ~3;
    std::vector<BYTE> out((size_t)stride * h);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            DWORD p = c.bits[(size_t)(y / kZoom) * c.w + x / kZoom];
            BYTE* o = &out[(size_t)(h - 1 - y) * stride + x * 3];
            o[0] = p & 0xFF;
            o[1] = p >> 8 & 0xFF;
            o[2] = p >> 16 & 0xFF;
        }
    BITMAPFILEHEADER f{0x4D42};
    BITMAPINFOHEADER i{sizeof(i), w, h, 1, 24, BI_RGB};
    f.bfOffBits = sizeof(f) + sizeof(i);
    f.bfSize = f.bfOffBits + (DWORD)out.size();
    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    DWORD written;
    WriteFile(file, &f, sizeof(f), &written, nullptr);
    WriteFile(file, &i, sizeof(i), &written, nullptr);
    WriteFile(file, out.data(), (DWORD)out.size(), &written, nullptr);
    CloseHandle(file);
}

void Set(PCWSTR name, PCWSTR value) {
    wchar_t var[96];
    swprintf(var, 96, L"TB_%ls", name);
    SetEnvironmentVariableW(var, value);
}

int wmain() {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    // Everything on, so every hook is in; each row then picks its settings.
    Set(L"driveBars", L"recolour");
    Set(L"selection", L"both");
    Set(L"barFill", L"A95D5D");
    Set(L"barFull", L"E57474");
    Set(L"hoverFill", L"121C21");
    Set(L"hoverBorder", L"E57474");
    Set(L"selectedFill", L"121C21");
    Set(L"selectedBorder", L"64AA89");
    Set(L"selectionBorderWidth", L"1");
    Set(L"focusPill", L"1");
    Set(L"focusPillColor", L"A95D5D");
    Set(L"focusPillWidth", L"3");
    Wh_ModInit();

    // Plain Progress: Explorer Style hands Explorer the style's regular bars
    // in place of dark ones the style left as Windows' own.
    HTHEME progress = OpenThemeData(nullptr, L"Progress");
    HWND list = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 10, 10, nullptr, nullptr, nullptr, nullptr);
    SetWindowTheme(list, L"DarkMode_ItemsView", nullptr);
    HTHEME listView = OpenThemeData(list, L"ListView");
    HWND tree = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 10, 10, nullptr, nullptr, nullptr, nullptr);
    SetWindowTheme(tree, L"DarkMode_Explorer", nullptr);
    HTHEME treeView = OpenThemeData(tree, L"TreeView");
    WCHAR name[96] = L"?";
    if (g_getThemeClass) g_getThemeClass(progress, name, 96);
    wprintf(L"GetThemeClass: %p, progress class '%ls'\n", g_getThemeClass, name);

    struct Row {
        PCWSTR label;
        PCWSTR bars, fill, fillEnd, full, empty, border, radius;
    } rows[] = {
        {L"style's own", L"style", L"", L"", L"", L"", L"", L"0"},
        {L"recolour", L"recolour", L"A95D5D", L"", L"E57474", L"", L"", L"0"},
        {L"recolour + empty + fade", L"recolour", L"66EA97", L"15FFA2", L"FF6F77", L"1E2F36", L"", L"0"},
        {L"flat", L"flat", L"A95D5D", L"64AA89", L"E57474", L"10171D", L"64AA89", L"3"},
    };
    int n = ARRAYSIZE(rows);
    Canvas c = MakeCanvas(kW * 2 + 30, (n + 3) * kRowH);
    for (int i = 0; i < n; i++) {
        Set(L"driveBars", rows[i].bars);
        Set(L"barFill", rows[i].fill);
        Set(L"barFillEnd", rows[i].fillEnd);
        Set(L"barFull", rows[i].full);
        Set(L"barEmpty", rows[i].empty);
        Set(L"barBorder", rows[i].border);
        Set(L"barRadius", rows[i].radius);
        Wh_ModSettingsChanged();
        for (int k = 0; k < 2; k++) {
            int x = 10 + k * (kW + 10), y = i * kRowH + 5;
            RECT bar{x, y, x + kW, y + 14};
            DrawThemeBackground_Hook(progress, c.dc, PP_TRANSPARENTBAR, PBBS_PARTIAL, &bar, nullptr);
            RECT fill = bar;
            fill.right = x + (k ? kW * 93 / 100 : kW * 45 / 100);
            DrawThemeBackground_Hook(progress, c.dc, PP_FILL, k ? PBFS_ERROR : PBFS_PARTIAL, &fill, nullptr);
        }
        wprintf(L"row %d: %ls\n", i, rows[i].label);
    }
    // Selections: file list hovered and selected, navigation pane hovered-selected with its pill.
    int y = n * kRowH + 4;
    RECT a{10, y, 10 + kW, y + 20}, b{kW + 20, y, kW * 2 + 20, y + 20};
    DrawThemeBackgroundEx_Hook(listView, c.dc, LVP_LISTITEM, LISS_HOT, &a, nullptr);
    DrawThemeBackgroundEx_Hook(listView, c.dc, LVP_LISTITEM, LISS_SELECTED, &b, nullptr);
    y += kRowH;
    RECT t{10, y, 10 + kW, y + 22};
    DrawThemeBackground_Hook(treeView, c.dc, TVP_TREEITEM, TREIS_HOTSELECTED, &t, nullptr);
    Set(L"selectionRadius", L"4");
    Set(L"selectionBorderWidth", L"2");
    Wh_ModSettingsChanged();
    RECT t2{kW + 20, y, kW * 2 + 20, y + 22};
    DrawThemeBackground_Hook(treeView, c.dc, TVP_TREEITEM, TREIS_HOTSELECTED, &t2, nullptr);
    y += kRowH;
    // The same rows with selections off: Windows' own drawing, for comparison.
    Set(L"selection", L"none");
    Wh_ModSettingsChanged();
    RECT a2{10, y, 10 + kW, y + 20}, b2{kW + 20, y, kW * 2 + 20, y + 20};
    DrawThemeBackgroundEx_Hook(listView, c.dc, LVP_LISTITEM, LISS_HOT, &a2, nullptr);
    DrawThemeBackgroundEx_Hook(listView, c.dc, LVP_LISTITEM, LISS_SELECTED, &b2, nullptr);
    GdiFlush();
    Save(c, L"colortest.bmp");
    wprintf(L"saved\n");
}
