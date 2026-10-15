// Theme files, and colour schemes taken from them. Shared by System Icons, the
// taskbar and the Switcher ("// @inline theme-palette.cpp", after
// palettes.cpp).
//
// Windows only offers visual styles for the whole system, so a theme file is
// loaded the way theme tools do it: uxtheme #92 (LoaderLoadTheme) parses it
// into two memory sections, and #16 (OpenThemeDataFromFile) opens a class
// from them. A class like "Taskbar2Composited::TaskBar" is the app name
// "Taskbar2Composited", set on a window with SetWindowTheme, plus "TaskBar".
//
// Two schemes come from a theme instead of the list in palettes.cpp:
//   visualStyle   the .msstyles Windows is using
//   taskbarStyle  the taskbar's theme (its Visual style setting, else the one
//                 StartAllBack is set to), else the visual style
// Both read the theme's menu look (its popup background, border, hot item and
// text), as Windows draws menus and flyouts with it. A theme without one, or
// no theme, gives Windows' own colours.

using LoaderLoadTheme_t = HRESULT(WINAPI*)(HANDLE themeFile, HINSTANCE instance, LPCWSTR fileName, LPCWSTR color,
                                           LPCWSTR size, HANDLE* sharableSection, LPWSTR sharableName,
                                           int sharableChars, HANDLE* nonSharableSection, LPWSTR nonSharableName,
                                           int nonSharableChars, PVOID customLoadHandler, HANDLE* reuseSection,
                                           int a, int b, BOOL emulateGlobal);

struct ThemeFileData {
    char header[7];
    void* sharableView;
    HANDLE sharableSection;
    void* nonSharableView;
    HANDLE nonSharableSection;
    char end[3];
};

using OpenThemeDataFromFile_t = HTHEME(WINAPI*)(ThemeFileData* themeFile, HWND wnd, LPCWSTR classList, DWORD flags);

OpenThemeDataFromFile_t ThemeFileOpener() {
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    return ux ? (OpenThemeDataFromFile_t)GetProcAddress(ux, MAKEINTRESOURCEA(16)) : nullptr;
}

bool LoadThemeFile(std::wstring const& path, ThemeFileData* file) {
    *file = {};
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    auto load = ux ? (LoaderLoadTheme_t)GetProcAddress(ux, MAKEINTRESOURCEA(92)) : nullptr;
    if (!load || !ThemeFileOpener() || path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
        return false;
    HANDLE sharable = nullptr, nonSharable = nullptr;
    // No section names: with names it tries to create named sections and
    // gets access denied.
    HRESULT hr = load(nullptr, nullptr, path.c_str(), L"NormalColor", L"NormalSize", &sharable, nullptr, 0,
                      &nonSharable, nullptr, 0, nullptr, nullptr, 0, 0, FALSE);
    if (FAILED(hr) || !sharable || !nonSharable) {
        Wh_Log(L"Couldn't load %s: 0x%08X", path.c_str(), hr);
        if (sharable) CloseHandle(sharable);
        if (nonSharable) CloseHandle(nonSharable);
        return false;
    }
    memcpy(file->header, "thmfile", 7);
    memcpy(file->end, "end", 3);
    file->sharableSection = sharable;
    file->nonSharableSection = nonSharable;
    file->sharableView = MapViewOfFile(sharable, FILE_MAP_READ, 0, 0, 0);
    file->nonSharableView = MapViewOfFile(nonSharable, FILE_MAP_READ, 0, 0, 0);
    return true;
}

void CloseThemeFile(ThemeFileData* file) {
    if (file->sharableView) UnmapViewOfFile(file->sharableView);
    if (file->nonSharableView) UnmapViewOfFile(file->nonSharableView);
    if (file->sharableSection) CloseHandle(file->sharableSection);
    if (file->nonSharableSection) CloseHandle(file->nonSharableSection);
    *file = {};
}

// One pixel of a part drawn into a small bitmap, or the fallback when the
// part isn't defined or draws nothing there.
uint32_t PartPixel(HTHEME theme, int part, int state, int x, int y, uint32_t fallback) {
    constexpr int kSize = 9;
    if (!IsThemePartDefined(theme, part, 0)) return fallback;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), kSize, -kSize, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    uint32_t result = fallback;
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        memset(bits, 0, kSize * kSize * 4);
        RECT r{0, 0, kSize, kSize};
        if (SUCCEEDED(DrawThemeBackground(theme, dc, part, state, &r, nullptr))) {
            GdiFlush();
            uint32_t pixel = ((uint32_t*)bits)[y * kSize + x];
            if (pixel) result = pixel & 0xFFFFFF;
        }
        SelectObject(dc, old);
        DeleteObject(bitmap);
    }
    DeleteDC(dc);
    return result;
}

// A theme file's menu class. In dark mode, the style's dark menus first (as
// Windows draws them for apps in dark mode), then its plain ones; in light
// mode the other way round. A style made for one mode may only have that
// one. Null when the style has no menu look of its own.
HTHEME OpenThemeFileMenu(ThemeFileData* file, HWND host, bool dark) {
    auto open = ThemeFileOpener();
    if (!open) return nullptr;
    PCWSTR darkFirst[] = {L"DarkMode_ImmersiveStart", L"ImmersiveStartDark", L"DarkMode", L""};
    PCWSTR lightFirst[] = {L"", L"DarkMode_ImmersiveStart", L"ImmersiveStartDark", L"DarkMode"};
    HTHEME found = nullptr;
    for (PCWSTR app : dark ? darkFirst : lightFirst) {
        SetWindowTheme(host, app, nullptr);
        HTHEME menu = open(file, host, L"Menu", 0);
        if (menu && IsThemePartDefined(menu, MENU_POPUPITEM, 0)) {
            found = menu;
            break;
        }
        if (menu) CloseThemeData(menu);
    }
    SetWindowTheme(host, L"", nullptr);
    return found;
}

// The visual style Windows is using now.
std::wstring CurrentThemeFile() {
    WCHAR file[MAX_PATH] = L"";
    if (FAILED(GetCurrentThemeName(file, ARRAYSIZE(file), nullptr, 0, nullptr, 0))) return {};
    return file;
}

// The taskbar and its flyouts follow the system mode, not the apps' one.
bool SystemUsesLightTheme() {
    DWORD light = 0, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light != 0;
}

// Windows' accent colour, as RGB.
uint32_t WindowsAccent() {
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD,
                     nullptr, &abgr, &size) != ERROR_SUCCESS)
        return 0x0078D4;
    return ((abgr & 0xFF) << 16) | (abgr & 0xFF00) | ((abgr >> 16) & 0xFF);
}

double PaletteLuma(uint32_t rgb) {
    return (0.299 * ((rgb >> 16) & 0xFF) + 0.587 * ((rgb >> 8) & 0xFF) + 0.114 * (rgb & 0xFF)) / 255;
}

// Windows' own colours, for what no theme sets.
Palette NativePalette(bool light) {
    uint32_t accent = WindowsAccent();
    Palette p = light ? Palette{0xF3F3F3, 0xE5E5E5, 0xD6D6D6, 0xB8B8B8, 0x1A1A1A, accent,
                                0x5C5C5C, 0x8A8A8A, 0x9D5D00, 0xC42B1C, 0xE5E5E5, false}
                      : Palette{0x202020, 0x2D2D2D, 0x3A3A3A, 0x4A4A4A, 0xFFFFFF, accent,
                                0xC8C8C8, 0x8A8A8A, 0xFCE100, 0xFF99A4, 0x2D2D2D, false};
    if (std::abs(PaletteLuma(p.accent) - PaletteLuma(p.ground)) < 0.25) p.accent = MixRgb(p.accent, p.text, 0.5);
    return p;
}

// A theme file's colours, from its menu look. False when it has none.
bool PaletteFromThemeFile(std::wstring const& path, bool light, Palette* out) {
    // Loading a theme takes a moment, and settings are read often: the last
    // one read is kept, until its file or the mode changes.
    static SRWLOCK lock = SRWLOCK_INIT;
    static std::wstring cachedKey;
    static bool cachedOk = false;
    static Palette cached{};
    WIN32_FILE_ATTRIBUTE_DATA info{};
    if (path.empty() || !GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &info)) return false;
    std::wstring key = path + L"|" + std::to_wstring(info.ftLastWriteTime.dwLowDateTime) + L"|" +
                       std::to_wstring(info.ftLastWriteTime.dwHighDateTime) + (light ? L"|light" : L"|dark") +
                       L"|" + std::to_wstring(WindowsAccent());
    AcquireSRWLockExclusive(&lock);
    if (key == cachedKey) {
        bool ok = cachedOk;
        if (ok) *out = cached;
        ReleaseSRWLockExclusive(&lock);
        return ok;
    }
    ReleaseSRWLockExclusive(&lock);

    ThemeFileData file;
    if (!LoadThemeFile(path, &file)) return false;
    HWND host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    HTHEME menu = host ? OpenThemeFileMenu(&file, host, !light) : nullptr;
    Palette p = NativePalette(light);
    bool ok = menu != nullptr;
    if (menu) {
        // The style's own system colours (its sysmetrics, which become Windows' colours when it's applied)
        // fill in what its menu look doesn't say. Only a full style has a menu class, and only a full
        // style's system colours mean anything: a StartAllBack-only style carries Plain8's light defaults.
        auto sys = [&](int index) {
            COLORREF c = GetThemeSysColor(menu, index);
            return (uint32_t)((GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c));
        };
        uint32_t sysGround = sys(COLOR_MENU), sysText = sys(COLOR_MENUTEXT);
        bool sysUsable = std::abs(PaletteLuma(sysGround) - PaletteLuma(sysText)) >= 0.25;
        if (sysUsable) {
            p.ground = sysGround;
            p.text = sysText;
            p.edge = sys(COLOR_ACTIVEBORDER);
        }
        p.ground = PartPixel(menu, MENU_POPUPBACKGROUND, 0, 4, 4, p.ground);
        p.edge = PartPixel(menu, MENU_POPUPBORDERS, 0, 0, 0, p.edge);
        uint32_t hot = PartPixel(menu, MENU_POPUPITEM, MPI_HOT, 4, 4, MixRgb(p.ground, p.text, 0.16));
        COLORREF text;
        if (SUCCEEDED(GetThemeColor(menu, MENU_POPUPITEM, MPI_NORMAL, TMT_TEXTCOLOR, &text))) {
            p.text = (GetRValue(text) << 16) | (GetGValue(text) << 8) | GetBValue(text);
        } else if (!sysUsable) {
            p.text = PaletteLuma(p.ground) < 0.5 ? 0xFFFFFF : 0x000000;
        }
        // The theme's own hot item for hovering, unless it hardly shows on
        // the ground; a quieter step of it for a selected row or a pressed
        // button.
        if (std::abs(PaletteLuma(hot) - PaletteLuma(p.ground)) < 0.06) hot = MixRgb(p.ground, p.text, 0.16);
        p.hover = hot;
        p.raised = MixRgb(p.ground, hot, 0.5);
        p.line = PartPixel(menu, MENU_POPUPSEPARATOR, 0, 4, 4, MixRgb(p.ground, p.text, 0.2));
        if (p.line == p.ground) p.line = MixRgb(p.ground, p.text, 0.2);
        p.muted = MixRgb(p.ground, p.text, 0.72);
        p.dim = MixRgb(p.ground, p.text, 0.45);
        // Its own grey text for faint text, when that can be read on the ground.
        if (sysUsable && std::abs(PaletteLuma(sys(COLOR_GRAYTEXT)) - PaletteLuma(p.ground)) >= 0.18)
            p.dim = sys(COLOR_GRAYTEXT);
        // Warnings and errors keep their meaning; only their lightness
        // follows the ground.
        bool dark = PaletteLuma(p.ground) < 0.5;
        p.warn = dark ? 0xF5A97F : 0xB35A00;
        p.bad = dark ? 0xF38BA8 : 0xC42B1C;
        // Its own highlight as the accent, when it stands out from the
        // ground; then the style's Highlight system colour; Windows' accent
        // colour last.
        uint32_t styleAccent = sysUsable ? sys(COLOR_HIGHLIGHT) : WindowsAccent();
        if (std::abs(PaletteLuma(styleAccent) - PaletteLuma(p.ground)) < 0.25) styleAccent = WindowsAccent();
        p.accent = std::abs(PaletteLuma(hot) - PaletteLuma(p.ground)) >= 0.25 ? hot : styleAccent;
        if (std::abs(PaletteLuma(p.accent) - PaletteLuma(p.ground)) < 0.25) p.accent = MixRgb(p.accent, p.text, 0.5);
        p.tourne = false;
        CloseThemeData(menu);
    }
    if (host) DestroyWindow(host);
    CloseThemeFile(&file);

    AcquireSRWLockExclusive(&lock);
    cachedKey = key;
    cachedOk = ok;
    cached = p;
    ReleaseSRWLockExclusive(&lock);
    if (ok) *out = p;
    return ok;
}

// A theme name, as StartAllBack stores it, to its file in StartAllBack's
// Styles folder or Tourne's; a full path stays as it is.
std::wstring ThemeStyleFile(std::wstring const& name) {
    if (name.empty() || name == L"none") return {};
    auto exists = [](std::wstring const& path) {
        DWORD a = GetFileAttributesW(path.c_str());
        return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
    };
    if (name.find(L'\\') != std::wstring::npos) return exists(name) ? name : L"";
    for (PCWSTR folder : {L"%ProgramFiles%\\StartAllBack\\Styles\\", L"%LOCALAPPDATA%\\Tourne\\Styles\\"}) {
        WCHAR expanded[MAX_PATH * 2];
        if (!ExpandEnvironmentStringsW(folder, expanded, ARRAYSIZE(expanded))) continue;
        std::wstring path = expanded + name;
        if (exists(path)) return path;
    }
    return {};
}

// The taskbar's theme file: its Visual style setting (read by its full name,
// through a variable, so the build doesn't file it under the part reading
// it), else the one StartAllBack is set to. Empty for none.
std::wstring TaskbarThemeFile() {
    constexpr PCWSTR kTaskbarStyleSetting = L"taskbar.style";
    PCWSTR value = Wh_GetStringSetting(kTaskbarStyleSetting);
    std::wstring name = value ? value : L"";
    Wh_FreeStringSetting(value);
    if (name.empty() || name == L"startallback") {
        WCHAR sab[MAX_PATH * 2] = L"";
        DWORD size = sizeof(sab);
        RegGetValueW(HKEY_CURRENT_USER, L"Software\\StartIsBack", L"TaskbarStyle", RRF_RT_REG_SZ, nullptr, sab, &size);
        name = sab;
    }
    return ThemeStyleFile(name);
}

// The Tourne theme: colours Tourne Desktop makes from the wallpaper or one
// seed colour, kept in one record (HKCU\Software\Tourne\Theme, a DWORD
// 0xRRGGBB per role) that every part set to it reads. After writing a new
// record Tourne Desktop posts "TourneThemeChanged" to every top-level window;
// a part reads its colours again then. Before there's a record, Tourne's own.
constexpr PCWSTR kTourneThemeScheme = L"tourneTheme";
bool g_followsTourneTheme;  // this part has used it (so it follows changes)

UINT TourneThemeMessage() {
    static UINT message = RegisterWindowMessageW(L"TourneThemeChanged");
    return message;
}

bool ReadTourneTheme(Palette* out) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Theme", 0, KEY_READ, &key) != ERROR_SUCCESS) return false;
    Palette p = *out;
    struct { PCWSTR name; uint32_t* role; } const roles[] = {
        {L"Ground", &p.ground}, {L"Raised", &p.raised}, {L"Line", &p.line}, {L"Edge", &p.edge}, {L"Text", &p.text},
        {L"Accent", &p.accent}, {L"Muted", &p.muted},   {L"Dim", &p.dim},   {L"Warn", &p.warn}, {L"Bad", &p.bad}};
    bool all = true;
    for (auto const& role : roles) {
        DWORD value = 0, size = sizeof(value);
        if (RegGetValueW(key, nullptr, role.name, RRF_RT_REG_DWORD, nullptr, &value, &size) != ERROR_SUCCESS) all = false;
        else *role.role = value & 0xFFFFFF;
    }
    RegCloseKey(key);
    if (!all) return false;
    p.hover = MixRgb(p.ground, p.text, 0.16);
    p.tourne = false;
    *out = p;
    return true;
}

bool IsThemeScheme(std::wstring_view key) {
    return key == L"visualStyle" || key == L"taskbarStyle";
}

// A colour scheme by its key, including the two taken from a theme.
Palette SuiteSchemePalette(std::wstring_view key) {
    if (key == kTourneThemeScheme) {
        g_followsTourneTheme = true;
        Palette p = SchemePalette(L"tourne");
        ReadTourneTheme(&p);
        return p;
    }
    if (!IsThemeScheme(key)) return SchemePalette(key);
    bool light = SystemUsesLightTheme();
    Palette p = NativePalette(light);
    if (key == L"taskbarStyle" && PaletteFromThemeFile(TaskbarThemeFile(), light, &p)) return p;
    PaletteFromThemeFile(CurrentThemeFile(), light, &p);
    return p;
}
