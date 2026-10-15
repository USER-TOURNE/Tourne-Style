// ==WindhawkMod==
// @id              local@tourne-theme-gaps
// @name            Tourne Theme Gaps
// @description     The parts of Windows a visual style doesn't reach, filled in from the style's own colours
// @version         1.0.0
// @author          Tourne
// @include         *
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- colors: visualStyle
  $name: Colours to fill in with
  $description: >-
    The visual style's own (its background and text), or one of the suite's
    colour schemes: the same as System Icons', or one chosen here. Also used
    for Explorer Style's dark dialogs.
  $options:
  - visualStyle: The visual style's own
  - matchIcons: Same as System Icons
  - tourne: Tourne
  - gruvboxDarkHard: Gruvbox dark (hard)
  - gruvboxDark: Gruvbox dark
  - gruvboxDarkSoft: Gruvbox dark (soft)
  - gruvboxLight: Gruvbox light
  - monokai: Monokai
  - catppuccinMocha: Catppuccin Mocha
  - catppuccinMacchiato: Catppuccin Macchiato
  - catppuccinFrappe: Catppuccin Frappé
  - catppuccinLatte: Catppuccin Latte
  - everforestDarkHard: Everforest dark (hard)
  - everforestDark: Everforest dark
  - everforestDarkSoft: Everforest dark (soft)
  - everforestLightHard: Everforest light (hard)
  - everforestLight: Everforest light
  - everforestLightSoft: Everforest light (soft)
  - nord: Nord
  - dracula: Dracula
  - tokyoNight: Tokyo Night
  - tokyoNightStorm: Tokyo Night Storm
  - rosePine: Rosé Pine
  - rosePineMoon: Rosé Pine Moon
  - rosePineDawn: Rosé Pine Dawn
  - kanagawa: Kanagawa
  - oneDark: One Dark
  - solarizedDark: Solarized dark
  - solarizedLight: Solarized light
- accentLeftovers: false
  $name: Windows' blue left in the style
  $description: >-
    A few places even a finished visual style keeps Windows' own blue: property
    text and selections in dark Explorer, the command bar's split buttons, list
    glows. Those take the style's own highlight colour instead (a darker mix of
    it for fills). Data colours, like Task Manager's graphs, stay as they are.
- excludeApps: ''
  $name: Leave these programs alone
  $description: Program file names, separated by commas, like game.exe.
*/
// ==/WindhawkModSettings==

// Some of Windows' own UI paints fixed colours in dark mode, whatever the
// visual style says: the Open and Save dialogs' footer (#383838) is the
// plainest case. Those colours are caught where Windows' shell and dialog
// code asks GDI for them, and the style's own colours put in their place.
// Only while a custom visual style is in use; Windows' own style keeps its
// look.

#include <windows.h>
#include <uxtheme.h>

#include <string>
#include <string_view>
#include <cstdint>

// @inline palettes.cpp
// @inline theme-gaps-palette.cpp

// Windows' fixed dark-mode colours, and the part of the palette each stands
// for. Exact values only: nothing else is touched.
struct GapColor {
    COLORREF windows;
    int role;  // 0 ground, 1 raised
};
constexpr GapColor kGapColors[] = {
    {RGB(0x38, 0x38, 0x38), 0},  // the Open/Save dialog footer
};

// The shell's and dialogs' own code, where those colours come from.
bool FromWindowsUi(void* returnAddress) {
    HMODULE caller = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCWSTR)returnAddress, &caller) ||
        !caller)
        return false;
    for (PCWSTR name : {L"dui70.dll", L"duser.dll", L"explorerframe.dll", L"shell32.dll", L"comdlg32.dll",
                        L"windows.storage.dll"}) {
        if (caller == GetModuleHandleW(name)) return true;
    }
    return false;
}

// The theme's colour for one of Windows' fixed ones, when this applies.
bool GapReplacement(COLORREF color, void* returnAddress, COLORREF* out) {
    for (auto const& gap : kGapColors) {
        if ((color & 0xFFFFFF) != gap.windows) continue;
        if (!FromWindowsUi(returnAddress)) return false;
        GapPalette p = CurrentGapPalette();
        if (!p.custom) return false;  // Windows' own style, and its own colours chosen
        *out = gap.role == 1 ? p.raised : p.ground;
        return true;
    }
    return false;
}

using CreateSolidBrush_t = HBRUSH(WINAPI*)(COLORREF);
using SetDCBrushColor_t = COLORREF(WINAPI*)(HDC, COLORREF);
using SetBkColor_t = COLORREF(WINAPI*)(HDC, COLORREF);
CreateSolidBrush_t CreateSolidBrush_Original;
SetDCBrushColor_t SetDCBrushColor_Original;
SetBkColor_t SetBkColor_Original;

HBRUSH WINAPI CreateSolidBrush_Hook(COLORREF color) {
    COLORREF themed;
    if (GapReplacement(color, __builtin_return_address(0), &themed)) color = themed;
    return CreateSolidBrush_Original(color);
}

COLORREF WINAPI SetDCBrushColor_Hook(HDC dc, COLORREF color) {
    COLORREF themed;
    if (GapReplacement(color, __builtin_return_address(0), &themed)) color = themed;
    return SetDCBrushColor_Original(dc, color);
}

COLORREF WINAPI SetBkColor_Hook(HDC dc, COLORREF color) {
    COLORREF themed;
    if (GapReplacement(color, __builtin_return_address(0), &themed)) color = themed;
    return SetBkColor_Original(dc, color);
}

// Windows' own accent blues that visual styles leave behind, from comparing
// finished styles with Windows' aero value by value (research/msstyles):
// dark Explorer's property text and fills, the command bar's split buttons,
// list glows, the newer controls' accent hints, and Windows' accent itself.
// Task Manager's graph colours aren't here: they mean something.
constexpr COLORREF kStockAccents[] = {
    RGB(0, 120, 212),  RGB(0, 103, 192),  RGB(0, 95, 184),   RGB(0, 102, 204),  RGB(0, 99, 177),
    RGB(35, 151, 201), RGB(58, 153, 232), RGB(12, 59, 123),  RGB(23, 102, 165), RGB(30, 57, 91),
    RGB(63, 118, 179), RGB(4, 204, 255),  RGB(19, 112, 171), RGB(38, 92, 192),
};

bool g_accentLeftovers;  // as the part started; a change reloads it (NeedsReload)

bool AccentLeftoversSetting() {
    return Wh_GetIntSetting(L"accentLeftovers") != 0;
}

using GetThemeColor_t = HRESULT(WINAPI*)(HTHEME, int, int, int, COLORREF*);
GetThemeColor_t GetThemeColor_Original;

HRESULT WINAPI GetThemeColor_Hook(HTHEME theme, int part, int state, int prop, COLORREF* color) {
    HRESULT hr = GetThemeColor_Original(theme, part, state, prop, color);
    if (FAILED(hr) || !color) return hr;
    COLORREF c = *color & 0xFFFFFF;
    for (COLORREF stock : kStockAccents) {
        if (c != stock) continue;
        GapPalette p = CurrentGapPalette();
        if (!p.custom) break;
        // A dark blue is a fill or a quiet line: the accent mixed into the
        // background at the same weight. A bright one is text or a glow.
        double luma = (0.299 * GetRValue(c) + 0.587 * GetGValue(c) + 0.114 * GetBValue(c)) / 255;
        *color = luma < 0.3 ? GapMix(p.ground, p.accent, 0.45) : p.accent;
        break;
    }
    return hr;
}

// A change to the leftovers setting needs the hook put in or taken out.
bool NeedsReload() {
    return AccentLeftoversSetting() != g_accentLeftovers;
}

bool ShouldLoadInThisProcess() {
    // Only programs with windows; not the ones set to be left alone.
    if (!GetModuleHandleW(L"user32.dll")) return false;
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) return false;
    PCWSTR name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    PCWSTR list = Wh_GetStringSetting(L"excludeApps");
    bool excluded = false;
    std::wstring items = list ? list : L"";
    Wh_FreeStringSetting(list);
    size_t start = 0;
    while (start <= items.size() && !excluded) {
        size_t comma = items.find(L',', start);
        std::wstring item = items.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty() && !_wcsicmp(item.c_str(), name)) excluded = true;
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return !excluded;
}

BOOL Wh_ModInit() {
    LoadGapSource();
    // The implementations, so calls through gdi32's forwarders are caught too.
    HMODULE gdi = GetModuleHandleW(L"gdi32full.dll");
    if (!gdi) gdi = GetModuleHandleW(L"gdi32.dll");
    if (!gdi) return FALSE;
    auto createSolidBrush = GetProcAddress(gdi, "CreateSolidBrush");
    auto setDcBrushColor = GetProcAddress(gdi, "SetDCBrushColor");
    auto setBkColor = GetProcAddress(gdi, "SetBkColor");
    if (createSolidBrush)
        Wh_SetFunctionHook((void*)createSolidBrush, (void*)CreateSolidBrush_Hook, (void**)&CreateSolidBrush_Original);
    if (setDcBrushColor)
        Wh_SetFunctionHook((void*)setDcBrushColor, (void*)SetDCBrushColor_Hook, (void**)&SetDCBrushColor_Original);
    if (setBkColor) Wh_SetFunctionHook((void*)setBkColor, (void*)SetBkColor_Hook, (void**)&SetBkColor_Original);
    // Only while wanted, so the default costs nothing; programs that draw
    // themed parts have uxtheme loaded by now.
    g_accentLeftovers = AccentLeftoversSetting();
    if (g_accentLeftovers) {
        if (HMODULE ux = GetModuleHandleW(L"uxtheme.dll")) {
            if (auto getThemeColor = GetProcAddress(ux, "GetThemeColor"))
                Wh_SetFunctionHook((void*)getThemeColor, (void*)GetThemeColor_Hook, (void**)&GetThemeColor_Original);
        }
    }
    return TRUE;
}

void Wh_ModUninit() {}

void Wh_ModSettingsChanged() {
    LoadGapSource();
}
