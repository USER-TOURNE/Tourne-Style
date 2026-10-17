// The visual style's own colours, for filling in what it doesn't cover
// ("// @inline theme-gaps-palette.cpp"; Theme Gaps and Explorer Style's dark
// dialogs). They come from the style's system colours (its sysmetrics), so
// reading them needs no window and is cheap enough to do while painting.

struct GapPalette {
    COLORREF ground;  // backgrounds
    COLORREF raised;  // controls on them: edits, lists
    COLORREF text;
    COLORREF accent;  // the style's highlight
    bool custom;      // to be used: a custom visual style, or a scheme chosen
};

COLORREF GapMix(COLORREF a, COLORREF b, double t) {
    auto channel = [&](int shift) {
        int x = (a >> shift) & 0xFF, y = (b >> shift) & 0xFF;
        return (COLORREF)((int)(x + (y - x) * t + 0.5) & 0xFF) << shift;
    };
    return channel(0) | channel(8) | channel(16);
}

// Windows' own styles: aero (and its dark and high contrast variants) live in
// Resources\Themes\aero; anything else is a custom style.
bool CustomStyleInUse() {
    WCHAR file[MAX_PATH] = L"";
    if (FAILED(GetCurrentThemeName(file, ARRAYSIZE(file), nullptr, 0, nullptr, 0)) || !file[0]) return false;
    PCWSTR name = wcsrchr(file, L'\\');
    name = name ? name + 1 : file;
    return _wcsicmp(name, L"aero.msstyles") != 0 && _wcsicmp(name, L"aerolite.msstyles") != 0;
}

// Where the colours come from: "visualStyle", or a colour scheme's key. Read
// from Theme Gaps' setting (by its full name, through a variable, so the
// build doesn't file it under the part reading it), as settings change.
// A fixed buffer, not a string: painting threads read it while settings change.
WCHAR g_gapSource[64] = L"visualStyle";
LONG g_gapGeneration = 0;

void LoadGapSource() {
    auto text = [](PCWSTR name) {
        PCWSTR value = Wh_GetStringSetting(name);
        std::wstring s = value ? value : L"";
        Wh_FreeStringSetting(value);
        return s;
    };
    constexpr PCWSTR kColors = L"themeGaps.colors", kIconsScheme = L"systemIcons.scheme";
    std::wstring source = text(kColors);
    if (source == L"matchIcons") source = text(kIconsScheme);
    if (source.empty() || source == L"taskbarStyle") source = L"visualStyle";
    wcsncpy_s(g_gapSource, source.c_str(), _TRUNCATE);
    InterlockedIncrement(&g_gapGeneration);
}

GapPalette CurrentGapPalette() {
    // The style can change while a program runs: read again now and then.
    static GapPalette cached{};
    static DWORD cachedAt = 0;
    static LONG cachedGeneration = -1;
    static bool have = false;
    DWORD now = GetTickCount();
    if (have && now - cachedAt < 2000 && cachedGeneration == g_gapGeneration) return cached;
    GapPalette p;
    if (wcscmp(g_gapSource, L"visualStyle") != 0) {
        Palette scheme = SchemePalette(g_gapSource);
        p.ground = PaletteRef(scheme.ground);
        p.raised = PaletteRef(scheme.raised);
        p.text = PaletteRef(scheme.text);
        p.accent = PaletteRef(scheme.accent);
        p.custom = true;
        cached = p;
        cachedAt = now;
        cachedGeneration = g_gapGeneration;
        have = true;
        return p;
    }
    p.ground = GetSysColor(COLOR_BTNFACE);
    p.text = GetSysColor(COLOR_BTNTEXT);
    p.raised = GapMix(p.ground, p.text, 0.08);
    p.accent = GetSysColor(COLOR_HIGHLIGHT);
    p.custom = CustomStyleInUse();
    cached = p;
    cachedAt = now;
    cachedGeneration = g_gapGeneration;
    have = true;
    return p;
}
