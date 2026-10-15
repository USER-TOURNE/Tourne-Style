// Colour schemes: Tourne's own, and the palettes the ricing community lives
// in, so the flyouts, tray icons and taskbar can match a system themed with
// any of them. Placed into System Icons and the taskbar by the build
// ("// @inline palettes.cpp").
//
// Each scheme fills the same roles:
//   ground  the flyout and bar background
//   raised  a selected row, a pressed or active button
//   line    dividers
//   edge    borders
//   text    main text, and the "ink" icon tone
//   accent  links, headings, switches, the "green" tone
//   muted   secondary text, the "sage" tone
//   dim     faint text, the "alt" tone
//   warn    warnings and values, the "coral" tone
//   bad     errors, the "red" tone
// Hover is text at 16% over ground, as in the original design.

struct Scheme {
    PCWSTR key;
    uint32_t ground, raised, line, edge, text, accent, muted, dim, warn, bad;
};

constexpr Scheme kSchemes[] = {
    {L"tourne",              0x121C21, 0x1B2A31, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x64AA89, 0x5D8D6F, 0xFF6F77, 0xFF8F8F},
    {L"gruvboxDarkHard",     0x1D2021, 0x3C3836, 0x32302F, 0x665C54, 0xEBDBB2, 0xB8BB26, 0xA89984, 0x7C6F64, 0xFE8019, 0xFB4934},
    {L"gruvboxDark",         0x282828, 0x3C3836, 0x3C3836, 0x665C54, 0xEBDBB2, 0xB8BB26, 0xA89984, 0x7C6F64, 0xFE8019, 0xFB4934},
    {L"gruvboxDarkSoft",     0x32302F, 0x504945, 0x3C3836, 0x665C54, 0xEBDBB2, 0xB8BB26, 0xA89984, 0x7C6F64, 0xFE8019, 0xFB4934},
    {L"gruvboxLight",        0xFBF1C7, 0xEBDBB2, 0xD5C4A1, 0xBDAE93, 0x3C3836, 0x79740E, 0x7C6F64, 0x928374, 0xAF3A03, 0x9D0006},
    {L"monokai",             0x272822, 0x3E3D32, 0x3E3D32, 0x75715E, 0xF8F8F2, 0xA6E22E, 0xCFCFC2, 0x75715E, 0xFD971F, 0xF92672},
    {L"catppuccinMocha",     0x1E1E2E, 0x313244, 0x313244, 0x45475A, 0xCDD6F4, 0xCBA6F7, 0xA6ADC8, 0x6C7086, 0xFAB387, 0xF38BA8},
    {L"catppuccinMacchiato", 0x24273A, 0x363A4F, 0x363A4F, 0x494D64, 0xCAD3F5, 0xC6A0F6, 0xA5ADCB, 0x6E738D, 0xF5A97F, 0xED8796},
    {L"catppuccinFrappe",    0x303446, 0x414559, 0x414559, 0x51576D, 0xC6D0F5, 0xCA9EE6, 0xA5ADCE, 0x737994, 0xEF9F76, 0xE78284},
    {L"catppuccinLatte",     0xEFF1F5, 0xCCD0DA, 0xCCD0DA, 0xBCC0CC, 0x4C4F69, 0x8839EF, 0x6C6F85, 0x9CA0B0, 0xFE640B, 0xD20F39},
    {L"everforestDarkHard",  0x272E33, 0x374145, 0x2E383C, 0x495156, 0xD3C6AA, 0xA7C080, 0x9DA9A0, 0x7A8478, 0xE69875, 0xE67E80},
    {L"everforestDark",      0x2D353B, 0x3D484D, 0x343F44, 0x4F585E, 0xD3C6AA, 0xA7C080, 0x9DA9A0, 0x7A8478, 0xE69875, 0xE67E80},
    {L"everforestDarkSoft",  0x333C43, 0x434F55, 0x3A464C, 0x555F66, 0xD3C6AA, 0xA7C080, 0x9DA9A0, 0x7A8478, 0xE69875, 0xE67E80},
    {L"everforestLightHard", 0xFFFBEF, 0xF2EFDF, 0xF8F5E4, 0xBEC5B2, 0x5C6A72, 0x8DA101, 0x829181, 0xA6B0A0, 0xF57D26, 0xF85552},
    {L"everforestLight",     0xFDF6E3, 0xEFEBD4, 0xF4F0D9, 0xBDC3AF, 0x5C6A72, 0x8DA101, 0x829181, 0xA6B0A0, 0xF57D26, 0xF85552},
    {L"everforestLightSoft", 0xF3EAD3, 0xE5DFC5, 0xEAE4CA, 0xB9C0AB, 0x5C6A72, 0x8DA101, 0x829181, 0xA6B0A0, 0xF57D26, 0xF85552},
    {L"nord",                0x2E3440, 0x3B4252, 0x3B4252, 0x4C566A, 0xECEFF4, 0x88C0D0, 0x81A1C1, 0x616E88, 0xD08770, 0xBF616A},
    {L"dracula",             0x282A36, 0x44475A, 0x44475A, 0x6272A4, 0xF8F8F2, 0xBD93F9, 0x8BE9FD, 0x6272A4, 0xFFB86C, 0xFF5555},
    {L"tokyoNight",          0x1A1B26, 0x292E42, 0x292E42, 0x414868, 0xC0CAF5, 0x7AA2F7, 0xA9B1D6, 0x565F89, 0xFF9E64, 0xF7768E},
    {L"tokyoNightStorm",     0x24283B, 0x292E42, 0x292E42, 0x414868, 0xC0CAF5, 0x7AA2F7, 0xA9B1D6, 0x565F89, 0xFF9E64, 0xF7768E},
    {L"rosePine",            0x191724, 0x26233A, 0x1F1D2E, 0x403D52, 0xE0DEF4, 0xC4A7E7, 0x908CAA, 0x6E6A86, 0xF6C177, 0xEB6F92},
    {L"rosePineMoon",        0x232136, 0x393552, 0x2A273F, 0x44415A, 0xE0DEF4, 0xC4A7E7, 0x908CAA, 0x6E6A86, 0xF6C177, 0xEB6F92},
    {L"rosePineDawn",        0xFAF4ED, 0xF2E9E1, 0xFFFAF3, 0xDFDAD9, 0x575279, 0x907AA9, 0x797593, 0x9893A5, 0xEA9D34, 0xB4637A},
    {L"kanagawa",            0x1F1F28, 0x2A2A37, 0x2A2A37, 0x54546D, 0xDCD7BA, 0x7E9CD8, 0xC8C093, 0x727169, 0xFFA066, 0xFF5D62},
    {L"oneDark",             0x282C34, 0x2C313A, 0x2C313A, 0x3E4451, 0xABB2BF, 0x61AFEF, 0x828997, 0x5C6370, 0xD19A66, 0xE06C75},
    {L"solarizedDark",       0x002B36, 0x073642, 0x073642, 0x586E75, 0x93A1A1, 0x268BD2, 0x839496, 0x586E75, 0xCB4B16, 0xDC322F},
    {L"solarizedLight",      0xFDF6E3, 0xEEE8D5, 0xEEE8D5, 0x93A1A1, 0x586E75, 0x268BD2, 0x657B83, 0x93A1A1, 0xCB4B16, 0xDC322F},
};

struct Palette {
    uint32_t ground, raised, line, edge, text, accent, muted, dim, warn, bad, hover;
    bool tourne;
};

// b over a at t (0..1), per channel.
constexpr uint32_t MixRgb(uint32_t a, uint32_t b, double t) {
    auto ch = [&](int shift) {
        double x = ((a >> shift) & 0xFF) * (1 - t) + ((b >> shift) & 0xFF) * t;
        return (uint32_t)(x + 0.5) << shift;
    };
    return ch(16) | ch(8) | ch(0);
}

Palette SchemePalette(std::wstring_view key) {
    Scheme const* s = &kSchemes[0];
    for (auto const& each : kSchemes) {
        if (key == each.key) s = &each;
    }
    return {s->ground, s->raised, s->line,  s->edge, s->text,
            s->accent, s->muted,  s->dim,   s->warn, s->bad,
            MixRgb(s->ground, s->text, 0.16), s == &kSchemes[0]};
}

// The one-step darker shade drawn under icon pixels. Tourne keeps its
// hand-picked shades; other schemes take the colour halfway to the ground.
uint32_t PaletteShade(Palette const& p, uint32_t color) {
    if (p.tourne) {
        switch (color) {
            case 0x64AA89: return 0x3D6657;
            case 0x66EA97: return 0x3C865C;
            case 0x15FFA2: return 0x1B9650;
            case 0xFF8F8F: return 0x8D585A;
            case 0xFF6F77: return 0xA95D5D;
            case 0x5D8D6F: return 0x366E53;
        }
        return color;
    }
    return MixRgb(p.ground, color, 0.5);
}

constexpr COLORREF PaletteRef(uint32_t rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}
