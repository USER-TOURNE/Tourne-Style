// ==WindhawkMod==
// @id              local@tourne-explorer-colors
// @name            Tourne Explorer Colours
// @description     Explorer's drive bars, selections and preview pane in your own colours, keeping the visual style's shapes
// @version         1.0.0
// @author          Tourne
// @include         explorer.exe
// @include         prevhost.exe
// @compilerOptions -luxtheme -lmsimg32 -lgdi32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- driveBars: style
  $name: Drive bars
  $description: >-
    The bars under This PC's drives: the visual style's own; the style's own
    bars (their segments, edges and shading) in your colours; or flat bars in
    your colours. Changes show at once.
  $options:
  - style: The visual style's own
  - recolour: The style's shape, in your colours
  - flat: Flat, in your colours
- barFill: "A95D5D"
  $name: Filled part
  $description: >-
    RRGGBB, or accent for the visual style's highlight colour. Empty keeps the
    style's own.
  #! $showIf: {driveBars: [recolour, flat]}
- barFillEnd: ""
  $name: Filled part, fading to
  $description: A second colour the filled part fades to along the bar. Empty for one colour.
  #! $showIf: {driveBars: [recolour, flat]}
- barFull: "E57474"
  $name: Filled part of a nearly full drive
  $description: Empty keeps the style's own.
  #! $showIf: {driveBars: [recolour, flat]}
- barFullEnd: ""
  $name: Nearly full, fading to
  #! $showIf: {driveBars: [recolour, flat]}
- barEmpty: ""
  $name: Empty part
  $description: The rest of the bar. Empty keeps the style's own.
  #! $showIf: {driveBars: [recolour, flat]}
- barBorder: "64AA89"
  $name: Flat bars' border
  $description: >-
    Drawn around a flat bar's empty part, with the filled part kept inside it.
    Empty for none. Needs an empty part colour.
  #! $showIf: {driveBars: flat}
- barRadius: 0
  $name: Flat bars' corner rounding
  #! $min: 0
  #! $max: 6
  #! $showIf: {driveBars: flat}
- selection: none
  $name: Selections
  $description: >-
    Selected and pointed-at items in Explorer's file list and navigation pane,
    in your colours. Changes show at once.
  $options:
  - none: The visual style's own
  - both: File list and navigation pane
  - files: File list only
  - navigation: Navigation pane only
- hoverFill: "121C21"
  $name: Pointed at
  $description: RRGGBB, or accent. Also an item that's both selected and pointed at.
  #! $showIf: {selection: [both, files, navigation]}
- hoverBorder: "E57474"
  $name: Pointed at, border
  $description: Empty for none.
  #! $showIf: {selection: [both, files, navigation]}
- selectedFill: "121C21"
  $name: Selected
  #! $showIf: {selection: [both, files, navigation]}
- selectedBorder: "64AA89"
  $name: Selected, border
  $description: Empty for none.
  #! $showIf: {selection: [both, files, navigation]}
- unfocusedFill: ""
  $name: Selected, in a window that isn't active
  $description: Empty keeps the style's own.
  #! $showIf: {selection: [both, files, navigation]}
- unfocusedBorder: ""
  $name: Selected in a window that isn't active, border
  #! $showIf: {selection: [both, files, navigation]}
- selectionBorderWidth: 1
  $name: Border width
  #! $min: 0
  #! $max: 3
  #! $showIf: {selection: [both, files, navigation]}
- selectionRadius: 0
  $name: Corner rounding
  #! $min: 0
  #! $max: 6
  #! $showIf: {selection: [both, files, navigation]}
- focusPill: true
  $name: Focus pill
  $description: The upright mark beside the folder you're on in the navigation pane.
  #! $showIf: {selection: [both, navigation]}
- focusPillColor: "A95D5D"
  $name: Focus pill colour
  #! $showIf: {selection: [both, navigation]}
- focusPillWidth: 3
  $name: Focus pill width
  #! $min: 1
  #! $max: 6
  #! $showIf: {selection: [both, navigation]}
- previewPane: false
  $name: Preview pane colour
  $description: >-
    The preview pane's background, and plain text previews, in your colour
    instead of the grey that doesn't match the rest of a dark Explorer.
    Explorer needs restarting after a change.
- previewColor: "121C21"
  $name: Preview pane background
  #! $showIf: {previewPane: true}
- previewText: ""
  $name: Preview text
  $description: Empty picks light or dark text to suit the background.
  #! $showIf: {previewPane: true}
- otherPrograms: ""
  $name: Also in these programs
  $description: >-
    Program file names, separated by commas, whose Open and Save dialogs get
    the same selections and drive bars, like notepad.exe. 64-bit programs only.
- taskManager: false
  $name: Task Manager too
  $description: >-
    Task Manager's usage cells (CPU, memory, disk, network: shaded more as use
    goes up) in the drive bars' colours, from the empty part through the
    filled part to nearly full, and its rows' selections in yours. Task
    Manager needs reopening after a change.
- taskManagerBackground: "121C21"
  $name: Task Manager's lists, background
  $description: >-
    The Details and Services tabs' lists, which are white whatever the style.
    RRGGBB; empty leaves them as they are.
  #! $showIf: {taskManager: true}
- taskManagerText: ""
  $name: Task Manager's lists, text
  $description: RRGGBB. Empty for a light grey on a dark background, a dark one on a light background.
  #! $showIf: {taskManager: true}
*/
// ==/WindhawkModSettings==

// How it works. Explorer draws these through uxtheme, so the parts and
// states it uses are caught there: the drive bar's filled part (Progress,
// part 5, state 4; state 2 when nearly full) and its empty part (part 11,
// state 2), the navigation pane's rows (TreeView) and the file list's
// (ListView, and ItemsView's selection layer). A recoloured bar is the
// style's own picture drawn aside and remapped by brightness, so its most
// common colour becomes exactly yours and every segment, gap and shade stays.
// Which parts to catch, and the file list's stale selection that needs
// DirectUI's help to spot, come from VitalS's Explorer Visual Tweaks.
//
// Nothing is hooked unless something is switched on, and the checks come
// cheapest first: a part and state number, then the class.

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <vssym32.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Settings

std::wstring TextSetting(PCWSTR name) {
    PCWSTR value = Wh_GetStringSetting(name);
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

// RRGGBB (a # is fine), or "accent" for the visual style's highlight.
bool ParseColor(std::wstring s, COLORREF* out) {
    s.erase(0, s.find_first_not_of(L" \t#"));
    s.erase(s.find_last_not_of(L" \t") + 1);
    if (_wcsicmp(s.c_str(), L"accent") == 0) {
        *out = GetSysColor(COLOR_HIGHLIGHT);
        return true;
    }
    if (s.size() != 6 || s.find_first_not_of(L"0123456789abcdefABCDEF") != std::wstring::npos) {
        return false;
    }
    unsigned long v = wcstoul(s.c_str(), nullptr, 16);
    *out = RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
    return true;
}

// A colour that can fade to a second one along a bar. Unset means the
// style's own.
struct Fade {
    bool set = false;
    COLORREF from = 0, to = 0;
};

Fade MakeFade(std::wstring const& from, std::wstring const& to) {
    Fade f;
    f.set = ParseColor(from, &f.from);
    if (!ParseColor(to, &f.to)) {
        f.to = f.from;
    }
    return f;
}

struct Look {
    bool set = false;  // Unset: the style's own.
    COLORREF fill = 0, border = 0;
    bool hasBorder = false;
};

Look MakeLook(std::wstring const& fill, std::wstring const& border) {
    Look l;
    l.set = ParseColor(fill, &l.fill);
    l.hasBorder = ParseColor(border, &l.border);
    return l;
}

enum class BarMode { Style, Recolour, Flat };
constexpr int kSelectFiles = 1, kSelectNavigation = 2;

struct Config {
    BarMode bars = BarMode::Style;
    Fade fill, full, empty;
    bool border = false;
    COLORREF borderColor = 0;
    int radius = 0;

    int selection = 0;
    Look hover, selected, unfocused;
    int borderWidth = 1, selectionRadius = 0;
    bool pill = false;
    COLORREF pillColor = 0;
    int pillWidth = 3;

    bool preview = false;
    COLORREF previewColor = 0, previewText = 0;

    bool taskManager = false;
};

Config ReadConfig() {
    Config c;
    std::wstring bars = TextSetting(L"driveBars");
    c.bars = bars == L"recolour" ? BarMode::Recolour : bars == L"flat" ? BarMode::Flat : BarMode::Style;
    c.fill = MakeFade(TextSetting(L"barFill"), TextSetting(L"barFillEnd"));
    c.full = MakeFade(TextSetting(L"barFull"), TextSetting(L"barFullEnd"));
    c.empty = MakeFade(TextSetting(L"barEmpty"), L"");
    c.border = ParseColor(TextSetting(L"barBorder"), &c.borderColor) && c.empty.set;
    c.radius = std::clamp(Wh_GetIntSetting(L"barRadius"), 0, 6);

    std::wstring where = TextSetting(L"selection");
    c.selection = where == L"both"         ? kSelectFiles | kSelectNavigation
                  : where == L"files"      ? kSelectFiles
                  : where == L"navigation" ? kSelectNavigation
                                           : 0;
    c.hover = MakeLook(TextSetting(L"hoverFill"), TextSetting(L"hoverBorder"));
    c.selected = MakeLook(TextSetting(L"selectedFill"), TextSetting(L"selectedBorder"));
    c.unfocused = MakeLook(TextSetting(L"unfocusedFill"), TextSetting(L"unfocusedBorder"));
    c.borderWidth = std::clamp(Wh_GetIntSetting(L"selectionBorderWidth"), 0, 3);
    c.selectionRadius = std::clamp(Wh_GetIntSetting(L"selectionRadius"), 0, 6);
    c.pill = Wh_GetIntSetting(L"focusPill") != 0 && ParseColor(TextSetting(L"focusPillColor"), &c.pillColor);
    c.pillWidth = std::clamp(Wh_GetIntSetting(L"focusPillWidth"), 1, 6);

    c.preview = Wh_GetIntSetting(L"previewPane") != 0 && ParseColor(TextSetting(L"previewColor"), &c.previewColor);
    if (!ParseColor(TextSetting(L"previewText"), &c.previewText)) {
        int luma = (GetRValue(c.previewColor) * 299 + GetGValue(c.previewColor) * 587 +
                    GetBValue(c.previewColor) * 114) / 1000;
        c.previewText = luma <= 128 ? RGB(0xDC, 0xDC, 0xDC) : RGB(0x20, 0x20, 0x20);
    }
    c.taskManager = Wh_GetIntSetting(L"taskManager") != 0;
    return c;
}

// Read by the drawing threads under g_lock (shared), replaced under it
// (exclusive) when settings change.
SRWLOCK g_lock = SRWLOCK_INIT;
Config g_config;
// What's on, checked before anything else so other drawing pays one load.
std::atomic<bool> g_barsOn;
std::atomic<int> g_selectionOn;
bool g_inTaskManager;  // fixed when the part starts

void SetConfig(Config const& c) {
    AcquireSRWLockExclusive(&g_lock);
    g_config = c;
    ReleaseSRWLockExclusive(&g_lock);
    g_barsOn = c.bars != BarMode::Style;
    g_selectionOn = c.selection;
}

Config CurrentConfig() {
    AcquireSRWLockShared(&g_lock);
    Config c = g_config;
    ReleaseSRWLockShared(&g_lock);
    return c;
}

// Which hooks the settings need, fixed when the part starts; a change to
// this needs a reload.
struct Needs {
    bool themeBackground = false;  // Drive bars, navigation pane
    bool themeBackgroundEx = false;  // File list
    bool preview = false;
    bool heat = false;  // Task Manager's usage cells
    bool operator==(Needs const&) const = default;
};

Needs NeedsFor(Config const& c) {
    // Task Manager: its Processes rows come through the plain call, Details' and Services' lists through either.
    if (g_inTaskManager)
        return {c.bars != BarMode::Style || c.selection != 0, c.selection != 0, c.preview, c.taskManager};
    return {c.bars != BarMode::Style || (c.selection & kSelectNavigation), (c.selection & kSelectFiles) != 0,
            c.preview};
}

Needs g_needs;

////////////////////////////////////////////////////////////////////////////////
// Drawing

using GetThemeClass_t = HRESULT(WINAPI*)(HTHEME, LPWSTR, int);
GetThemeClass_t g_getThemeClass;  // uxtheme's ordinal 74

// The class a theme handle was opened for, ignoring an App:: prefix.
bool ClassIs(HTHEME theme, PCWSTR want) {
    WCHAR name[96];
    if (!g_getThemeClass || FAILED(g_getThemeClass(theme, name, ARRAYSIZE(name)))) {
        return false;
    }
    size_t n = wcslen(name), k = wcslen(want);
    if (n < k || _wcsicmp(name + n - k, want) != 0) {
        return false;
    }
    return n == k || (n >= k + 2 && name[n - k - 1] == L':' && name[n - k - 2] == L':');
}

int DpiOf(HDC dc) {
    if (HWND wnd = WindowFromDC(dc)) {
        if (UINT dpi = GetDpiForWindow(wnd)) {
            return (int)dpi;
        }
    }
    int dpi = GetDeviceCaps(dc, LOGPIXELSY);
    return dpi > 0 ? dpi : 96;
}

int Luma(COLORREF c) {
    return (GetRValue(c) * 299 + GetGValue(c) * 587 + GetBValue(c) * 114) / 1000;
}

COLORREF Mix(COLORREF a, COLORREF b, float t) {
    auto ch = [&](int x, int y) { return (int)(x + (y - x) * t + 0.5f); };
    return RGB(ch(GetRValue(a), GetRValue(b)), ch(GetGValue(a), GetGValue(b)), ch(GetBValue(a), GetBValue(b)));
}

// One 32-bit picture to draw into before blending, reused; only touched
// under g_drawLock. Explorer paints from several threads, and a memory DC
// isn't safe to share between them.
SRWLOCK g_drawLock = SRWLOCK_INIT;
struct Scratch {
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ old = nullptr;
    DWORD* bits = nullptr;
    int w = 0, h = 0;

    bool Ensure(int width, int height) {
        if (bits && width <= w && height <= h) {
            return true;
        }
        Free();
        width = std::max(width, 64);
        height = std::max(height, 32);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB};
        dc = CreateCompatibleDC(nullptr);
        bitmap = dc ? CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0) : nullptr;
        if (!bitmap) {
            Free();
            return false;
        }
        old = SelectObject(dc, bitmap);
        w = width;
        h = height;
        return true;
    }
    void Clear(int width, int height) {
        for (int y = 0; y < height; y++) {
            memset(bits + (size_t)y * w, 0, (size_t)width * 4);
        }
    }
    DWORD& At(int x, int y) { return bits[(size_t)y * w + x]; }
    void Free() {
        if (dc) {
            if (old) SelectObject(dc, old);
            DeleteDC(dc);
        }
        if (bitmap) DeleteObject(bitmap);
        *this = Scratch{};
    }
};
Scratch g_scratch;
std::vector<DWORD> g_overBlack;  // A recoloured part as drawn over black

DWORD Premultiplied(COLORREF c, float alpha) {
    int a = (int)(alpha * 255 + 0.5f);
    return (DWORD)a << 24 | (DWORD)(GetRValue(c) * a / 255) << 16 | (DWORD)(GetGValue(c) * a / 255) << 8 |
           (DWORD)(GetBValue(c) * a / 255);
}

// The scratch's top-left w x h over the DC at r, inside the clip.
void BlendScratch(HDC dc, RECT const& r, RECT const* clip, int w, int h) {
    int saved = SaveDC(dc);
    if (clip) {
        IntersectClipRect(dc, clip->left, clip->top, clip->right, clip->bottom);
    }
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    AlphaBlend(dc, r.left, r.top, w, h, g_scratch.dc, 0, 0, w, h, blend);
    RestoreDC(dc, saved);
}

// How much of pixel (x, y) a rounded rectangle covers, sampled 4x4 only
// where an edge or corner passes through it.
float Coverage(int x, int y, float left, float top, float right, float bottom, float radius) {
    if (x + 1 <= left || x >= right || y + 1 <= top || y >= bottom) {
        return 0;
    }
    bool edge = x < left || x + 1 > right || y < top || y + 1 > bottom;
    bool corner = radius > 0 && (x < left + radius || x + 1 > right - radius) &&
                  (y < top + radius || y + 1 > bottom - radius);
    if (!edge && !corner) {
        return 1;
    }
    int inside = 0;
    for (int sy = 0; sy < 4; sy++) {
        for (int sx = 0; sx < 4; sx++) {
            float fx = x + (sx + 0.5f) / 4, fy = y + (sy + 0.5f) / 4;
            if (fx < left || fx >= right || fy < top || fy >= bottom) {
                continue;
            }
            float cx = std::clamp(fx, left + radius, right - radius);
            float cy = std::clamp(fy, top + radius, bottom - radius);
            float dx = fx - cx, dy = fy - cy;
            inside += dx * dx + dy * dy <= radius * radius;
        }
    }
    return inside / 16.0f;
}

// A rounded box into the scratch: fill inside, and a ring of `ring` pixels
// in the border colour when there is one. Fill fades along the long side.
void PaintBox(int w, int h, float radius, float ring, Fade const& fill, bool hasBorder, COLORREF border) {
    bool horizontal = w >= h;
    int length = std::max(1, (horizontal ? w : h) - 1);
    radius = std::min(radius, std::min(w, h) / 2.0f);
    float innerRadius = std::max(0.0f, radius - ring);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float outer = Coverage(x, y, 0, 0, (float)w, (float)h, radius);
            if (outer <= 0) {
                continue;
            }
            float inner = ring > 0 ? Coverage(x, y, ring, ring, w - ring, h - ring, innerRadius) : outer;
            COLORREF color = fill.from == fill.to ? fill.from
                                                  : Mix(fill.from, fill.to, (float)(horizontal ? x : y) / length);
            if (hasBorder && inner < outer) {
                // Border over the edge's share, fill over the rest.
                float t = (outer - inner) / outer;
                color = Mix(color, border, t);
            } else if (!hasBorder) {
                outer = inner;
            }
            g_scratch.At(x, y) = Premultiplied(color, outer);
        }
    }
}

// The style's own picture for a part, remapped to a colour by brightness.
bool DrawRecoloured(decltype(&DrawThemeBackground) draw, HTHEME theme, HDC dc, int part, int state, RECT const& r,
                    RECT const* clip, Fade const& fade) {
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0 || w > 8192 || h > 1024 || !g_scratch.Ensure(w, h)) {
        return false;
    }
    // Drawn over black and again over white: a part drawn with plain GDI
    // leaves alpha untouched, so what shows through tells the alpha instead.
    RECT local{0, 0, w, h};
    SetLayout(g_scratch.dc, 0);
    g_scratch.Clear(w, h);
    if (FAILED(draw(theme, g_scratch.dc, part, state, &local, nullptr))) {
        return false;
    }
    GdiFlush();
    g_overBlack.resize((size_t)w * h);
    for (int y = 0; y < h; y++) {
        memcpy(g_overBlack.data() + (size_t)y * w, &g_scratch.At(0, y), (size_t)w * 4);
        std::fill_n(&g_scratch.At(0, y), w, 0x00FFFFFFu);
    }
    draw(theme, g_scratch.dc, part, state, &local, nullptr);
    GdiFlush();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            DWORD black = g_overBlack[(size_t)y * w + x], white = g_scratch.At(x, y);
            int through = ((white >> 16 & 0xFF) - (black >> 16 & 0xFF) + (white >> 8 & 0xFF) - (black >> 8 & 0xFF) +
                           (white & 0xFF) - (black & 0xFF)) / 3;
            DWORD a = (DWORD)std::clamp(255 - through, 0, 255);
            // Over black the colour is already premultiplied.
            auto channel = [&](int shift) { return std::min<DWORD>(a, black >> shift & 0xFF) << shift; };
            g_scratch.At(x, y) = a << 24 | channel(16) | channel(8) | channel(0);
        }
    }

    // The picture's own colour: its most common solid one.
    struct Bin {
        DWORD color;
        int count;
    } bins[16];
    int used = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            DWORD p = g_scratch.At(x, y);
            if ((p >> 24) != 255) {
                continue;
            }
            DWORD c = p & 0xFFFFFF;
            int i = 0;
            while (i < used && bins[i].color != c) i++;
            if (i < used) {
                bins[i].count++;
            } else if (used < 16) {
                bins[used++] = {c, 1};
            }
        }
    }
    if (!used) {
        return false;  // Nothing solid to go by: the style's own.
    }
    DWORD own = std::max_element(bins, bins + used, [](Bin a, Bin b) { return a.count < b.count; })->color;
    float ownLuma = ((own >> 16 & 0xFF) * 299 + (own >> 8 & 0xFF) * 587 + (own & 0xFF) * 114) / 1000.0f;

    bool horizontal = w >= h;
    int length = std::max(1, (horizontal ? w : h) - 1);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            DWORD& p = g_scratch.At(x, y);
            DWORD a = p >> 24;
            if (!a) {
                continue;
            }
            // Brightness against the picture's own colour, alpha included
            // (the pixels are premultiplied), so that colour maps to exactly
            // the new one and lighter or darker shades stay so.
            float luma = ((p >> 16 & 0xFF) * 299 + (p >> 8 & 0xFF) * 587 + (p & 0xFF) * 114) / 1000.0f;
            float scale = ownLuma >= 8 ? luma / ownLuma : a / 255.0f;
            COLORREF color =
                fade.from == fade.to ? fade.from : Mix(fade.from, fade.to, (float)(horizontal ? x : y) / length);
            auto channel = [&](int c) { return (DWORD)std::min<float>((float)a, c * scale + 0.5f); };
            p = a << 24 | channel(GetRValue(color)) << 16 | channel(GetGValue(color)) << 8 | channel(GetBValue(color));
        }
    }
    BlendScratch(dc, r, clip, w, h);
    return true;
}

// The drive bar's parts: true when drawn here.
bool DrawDriveBar(decltype(&DrawThemeBackground) draw, HTHEME theme, HDC dc, int part, int state,
                  RECT const* rect, RECT const* clip) {
    bool filled = part == PP_FILL && (state == PBFS_PARTIAL || state == PBFS_ERROR);
    bool track = part == PP_TRANSPARENTBAR && state == PBBS_PARTIAL;
    if ((!filled && !track) || !rect || !ClassIs(theme, L"Progress")) {
        return false;
    }
    Config c = CurrentConfig();
    Fade const& fade = track ? c.empty : state == PBFS_ERROR ? c.full : c.fill;
    if (c.bars == BarMode::Style || !fade.set) {
        return false;
    }
    RECT r = *rect;
    if (r.left > r.right) std::swap(r.left, r.right);
    if (r.top > r.bottom) std::swap(r.top, r.bottom);
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0) {
        return true;
    }

    AcquireSRWLockExclusive(&g_drawLock);
    bool drawn;
    if (c.bars == BarMode::Recolour) {
        drawn = DrawRecoloured(draw, theme, dc, part, state, r, clip, fade);
    } else {
        int dpi = DpiOf(dc);
        float radius = (float)MulDiv(c.radius, dpi, 96);
        float ring = c.border ? (float)std::max(1, MulDiv(1, dpi, 96)) : 0;
        drawn = w <= 8192 && h <= 1024 && g_scratch.Ensure(w, h);
        if (drawn) {
            g_scratch.Clear(w, h);
            if (track) {
                PaintBox(w, h, radius, ring, fade, c.border, c.borderColor);
            } else {
                // The filled part sits inside the empty part's border.
                RECT inner = r;
                if (c.border) {
                    InflateRect(&inner, -(int)ring, -(int)ring);
                }
                int iw = inner.right - inner.left, ih = inner.bottom - inner.top;
                if (iw > 0 && ih > 0) {
                    PaintBox(iw, ih, std::max(0.0f, radius - ring), 0, fade, false, 0);
                    BlendScratch(dc, inner, clip, iw, ih);
                }
            }
            if (track) {
                BlendScratch(dc, r, clip, w, h);
            }
        }
    }
    ReleaseSRWLockExclusive(&g_drawLock);
    return drawn;
}

// A selection box: fill, border and rounding from a look.
void DrawSelection(HDC dc, RECT const& r, Look const& look, int borderWidth, int radius, bool pill,
                   COLORREF pillColor, int pillWidth) {
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0 || w > 8192 || h > 1024) {
        return;
    }
    int dpi = DpiOf(dc);
    float ring = look.hasBorder && borderWidth ? (float)std::max(1, MulDiv(borderWidth, dpi, 96)) : 0;
    Fade fill{true, look.fill, look.fill};
    AcquireSRWLockExclusive(&g_drawLock);
    if (g_scratch.Ensure(w, h)) {
        g_scratch.Clear(w, h);
        PaintBox(w, h, (float)MulDiv(radius, dpi, 96), ring, fill, ring > 0, look.border);
        if (pill) {
            // An upright capsule inside the left edge, 3 px clear of the top and bottom.
            int pw = std::max(1, MulDiv(pillWidth, dpi, 96)), inset = MulDiv(3, dpi, 96);
            int px = (int)ring + (ring > 0 ? 0 : MulDiv(1, dpi, 96)), ph = h - 2 * inset;
            for (int y = 0; y < ph && pw < w; y++) {
                for (int x = 0; x < pw; x++) {
                    float cover = Coverage(x, y, 0, 0, (float)pw, (float)ph, pw / 2.0f);
                    if (cover > 0) {
                        DWORD over = Premultiplied(pillColor, cover);
                        DWORD& p = g_scratch.At(px + x, inset + y);
                        // The pill over the box.
                        DWORD keep = 255 - (over >> 24);
                        auto mix = [&](int shift) {
                            return ((over >> shift & 0xFF) + (p >> shift & 0xFF) * keep / 255) << shift;
                        };
                        p = mix(24) | mix(16) | mix(8) | mix(0);
                    }
                }
            }
        }
        BlendScratch(dc, r, nullptr, w, h);
    }
    ReleaseSRWLockExclusive(&g_drawLock);
}

////////////////////////////////////////////////////////////////////////////////
// Selections

// DirectUI's file list paints a stale selection behind the item a click is
// moving to; which element is painting, and which one the click went to,
// tell the two apart.
using PaintBackground_t = void (*)(void*, HDC, void*, const RECT*, const RECT*, const RECT*, const RECT*);
using ElementBool_t = bool (*)(void*);
using ElementParent_t = void* (*)(void*);
using KeyFocused_t = void* (*)();
PaintBackground_t PaintBackground_Original;
ElementBool_t g_getSelected, g_getMouseFocused;
ElementParent_t g_getParent;
KeyFocused_t g_getKeyFocused;
thread_local void* t_paintingElement;  // A plain pointer: nothing to destroy.

void PaintBackground_Hook(void* element, HDC dc, void* value, const RECT* a, const RECT* b, const RECT* c,
                          const RECT* d) {
    void* previous = t_paintingElement;
    t_paintingElement = element;
    PaintBackground_Original(element, dc, value, a, b, c, d);
    t_paintingElement = previous;
}

// The item a mouse click is selecting, before it shows as selected.
void* PendingClickElement() {
    void* focused = g_getKeyFocused ? g_getKeyFocused() : nullptr;
    return focused && !g_getSelected(focused) && g_getMouseFocused(focused) ? focused : nullptr;
}

bool StaleSelection() {
    void* painting = t_paintingElement;
    void* pending = painting ? PendingClickElement() : nullptr;
    return pending && pending != painting && g_getParent(pending) == g_getParent(painting);
}

bool MultiSelecting() {
    return (GetAsyncKeyState(VK_CONTROL) & 0x8000) || (GetAsyncKeyState(VK_SHIFT) & 0x8000);
}

bool HookDirectUi() {
    HMODULE dui = LoadLibraryExW(L"dui70.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!dui) {
        return false;
    }
    auto paint = (void*)GetProcAddress(
        dui, "?PaintBackground@Element@DirectUI@@QEAAXPEAUHDC__@@PEAVValue@2@AEBUtagRECT@@222@Z");
    g_getSelected = (ElementBool_t)GetProcAddress(dui, "?GetSelected@Element@DirectUI@@QEAA_NXZ");
    g_getMouseFocused = (ElementBool_t)GetProcAddress(dui, "?GetMouseFocused@Element@DirectUI@@QEAA_NXZ");
    g_getParent = (ElementParent_t)GetProcAddress(dui, "?GetParent@Element@DirectUI@@QEAAPEAV12@XZ");
    g_getKeyFocused =
        (KeyFocused_t)GetProcAddress(dui, "?GetKeyFocusedElement@HWNDElement@DirectUI@@SAPEAVElement@2@XZ");
    if (!paint || !g_getSelected || !g_getMouseFocused || !g_getParent || !g_getKeyFocused) {
        Wh_Log(L"DirectUI's exports weren't found");
        g_getKeyFocused = nullptr;
        return false;
    }
    // dui70 stays loaded for good once hooked.
    HMODULE pinned;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)paint,
                       &pinned);
    return Wh_SetFunctionHook(paint, (void*)PaintBackground_Hook, (void**)&PaintBackground_Original);
}

using DrawThemeBackground_t = decltype(&DrawThemeBackground);
using DrawThemeBackgroundEx_t = decltype(&DrawThemeBackgroundEx);
DrawThemeBackground_t DrawThemeBackground_Original;
DrawThemeBackgroundEx_t DrawThemeBackgroundEx_Original;

HRESULT WINAPI DrawThemeBackground_Hook(HTHEME theme, HDC dc, int part, int state, const RECT* rect,
                                        const RECT* clip) {
    if (part == PP_FILL || part == PP_TRANSPARENTBAR) {
        if (g_barsOn.load(std::memory_order_relaxed) &&
            DrawDriveBar(DrawThemeBackground_Original, theme, dc, part, state, rect, clip)) {
            return S_OK;
        }
    } else if (part == TVP_TREEITEM && rect && state >= TREIS_HOT && state <= TREIS_HOTSELECTED &&
               state != TREIS_DISABLED && (g_selectionOn.load(std::memory_order_relaxed) & kSelectNavigation) &&
               ClassIs(theme, L"TreeView")) {
        Config c = CurrentConfig();
        if (c.selection & kSelectNavigation) {
            auto draw = [&](Look const& look, bool pill) {
                if (!look.set) return false;
                DrawSelection(dc, *rect, look, c.borderWidth, c.selectionRadius, pill && c.pill, c.pillColor,
                              c.pillWidth);
                return true;
            };
            switch (state) {
                case TREIS_HOT:
                    if (draw(c.hover, false)) return S_OK;
                    break;
                case TREIS_SELECTED:
                    // Explorer paints the row a click left as selected for a
                    // moment; only a Ctrl or Shift selection is real.
                    if (!MultiSelecting() || draw(c.selected, false)) return S_OK;
                    break;
                case TREIS_SELECTEDNOTFOCUS:
                    if (draw(c.unfocused, false)) return S_OK;
                    break;
                case TREIS_HOTSELECTED:
                    if (draw(c.hover, true)) return S_OK;
                    break;
            }
        }
    } else if (g_inTaskManager && part == LVP_LISTITEM && rect && state >= LISS_HOT && state <= LISS_HOTSELECTED &&
               state != LISS_DISABLED && (g_selectionOn.load(std::memory_order_relaxed) & kSelectFiles) &&
               ClassIs(theme, L"ListView")) {
        // Task Manager's rows, which it draws without the Ex call Explorer's file list uses.
        Config c = CurrentConfig();
        Look const& look = state == LISS_SELECTED ? c.selected : state == LISS_SELECTEDNOTFOCUS ? c.unfocused : c.hover;
        if (look.set) {
            DrawSelection(dc, *rect, look, c.borderWidth, c.selectionRadius, false, 0, 0);
            return S_OK;
        }
    }
    return DrawThemeBackground_Original(theme, dc, part, state, rect, clip);
}

constexpr int kItemsViewSelectionLayer = 3;  // ItemsView's part; no SDK name
constexpr int kItemsViewPendingState = 2;

HRESULT WINAPI DrawThemeBackgroundEx_Hook(HTHEME theme, HDC dc, int part, int state, const RECT* rect,
                                          const DTBGOPTS* options) {
    if (rect && (part == kItemsViewSelectionLayer || part == LVP_LISTITEM) &&
        (g_selectionOn.load(std::memory_order_relaxed) & kSelectFiles)) {
        Config c = CurrentConfig();
        if (c.selection & kSelectFiles) {
            if (part == kItemsViewSelectionLayer && ClassIs(theme, L"ItemsView")) {
                // Windows' own layer goes; a click's new item is drawn as pointed at.
                void* painting = t_paintingElement;
                if (state == kItemsViewPendingState && painting && c.hover.set &&
                    PendingClickElement() == painting) {
                    DrawSelection(dc, *rect, c.hover, c.borderWidth, c.selectionRadius, false, 0, 0);
                }
                return S_OK;
            }
            if (part == LVP_LISTITEM && state >= LISS_HOT && state <= LISS_HOTSELECTED && state != LISS_DISABLED &&
                ClassIs(theme, L"ListView")) {
                Look const* look = state == LISS_SELECTED            ? &c.selected
                                   : state == LISS_SELECTEDNOTFOCUS ? &c.unfocused
                                                                    : &c.hover;
                if (state == LISS_SELECTED && !MultiSelecting() && StaleSelection()) {
                    return S_OK;
                }
                if (look->set) {
                    DrawSelection(dc, *rect, *look, c.borderWidth, c.selectionRadius, false, 0, 0);
                    return S_OK;
                }
            }
        }
    }
    return DrawThemeBackgroundEx_Original(theme, dc, part, state, rect, options);
}

////////////////////////////////////////////////////////////////////////////////
// Preview pane

// The preview pane's window classes get a background brush in the chosen
// colour when they're registered, its theme colour follows, and in the
// preview host the plain text previewer (a RichEdit) draws in it too.
enum class Host { Explorer, Previewer, Dialog };
Host g_host = Host::Explorer;
COLORREF g_previewColor, g_previewText;

bool PreviewClass(LPCWSTR name) {
    if (!name || IS_INTRESOURCE(name)) {
        return false;
    }
    return g_host == Host::Previewer ? _wcsicmp(name, L"Shell Preview Extension Host Previewer") == 0
                                     : _wcsicmp(name, L"Shell Preview Extension Host") == 0;
}

using RegisterClassExW_t = decltype(&RegisterClassExW);
RegisterClassExW_t RegisterClassExW_Original;
ATOM WINAPI RegisterClassExW_Hook(const WNDCLASSEXW* wc) {
    if (!wc || !PreviewClass(wc->lpszClassName)) {
        return RegisterClassExW_Original(wc);
    }
    // The class owns its brush once registered; Windows deletes it with the class.
    WNDCLASSEXW copy = *wc;
    copy.hbrBackground = CreateSolidBrush(g_previewColor);
    ATOM atom = RegisterClassExW_Original(&copy);
    if (!atom && copy.hbrBackground) {
        DWORD error = GetLastError();
        DeleteObject(copy.hbrBackground);
        SetLastError(error);
    }
    return atom;
}

using GetThemeColor_t = decltype(&GetThemeColor);
GetThemeColor_t GetThemeColor_Original;
HRESULT WINAPI GetThemeColor_Hook(HTHEME theme, int part, int state, int prop, COLORREF* color) {
    HRESULT hr = GetThemeColor_Original(theme, part, state, prop, color);
    if (hr == S_OK && color && part == 1 && prop == TMT_FILLCOLOR && ClassIs(theme, L"ReadingPane")) {
        *color = g_previewColor;
    }
    return hr;
}

bool FromRichEdit(void* returnAddress) {
    HMODULE caller = nullptr;
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              (LPCWSTR)returnAddress, &caller) &&
           caller && caller == GetModuleHandleW(L"msftedit.dll");
}

using GetSysColor_t = decltype(&GetSysColor);
GetSysColor_t GetSysColor_Original;
DWORD WINAPI GetSysColor_Hook(int index) {
    if ((index == COLOR_WINDOW || index == COLOR_WINDOWTEXT) && FromRichEdit(__builtin_return_address(0))) {
        return index == COLOR_WINDOW ? g_previewColor : g_previewText;
    }
    return GetSysColor_Original(index);
}

using AllowDarkModeForWindow_t = BOOL(WINAPI*)(HWND, BOOL);
AllowDarkModeForWindow_t g_allowDarkModeForWindow;  // uxtheme's ordinal 133

// Task Manager's Details and Services tabs are plain list controls, white whatever the style: their background,
// text and column headers in the Task Manager colours. Their dialog makes them inside user32 (no CreateWindowExW
// call to catch) and Task Manager sets their colours again itself, so each list is caught as it's made (an
// in-process window event, as Dark Magic does it), and subclassed so the colours it's given become these.
COLORREF g_listGround, g_listText;
bool g_listColours;
HWINEVENTHOOK g_listHook;

using SetWindowSubclass_t = BOOL(WINAPI*)(HWND, SUBCLASSPROC, UINT_PTR, DWORD_PTR);
using DefSubclassProc_t = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);
SetWindowSubclass_t g_setWindowSubclass;
DefSubclassProc_t g_defSubclassProc;

constexpr UINT_PTR kListSubclassId = 0x544D4C53;
UINT g_removeMessage = RegisterWindowMessageW(L"TourneExplorerColours_Unsubclass");
SRWLOCK g_listsLock = SRWLOCK_INIT;
std::vector<HWND> g_lists;  // subclassed, so they're let go when the part stops

LRESULT CALLBACK ListSubclass(HWND list, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (message == g_removeMessage && g_removeMessage) {
        using RemoveWindowSubclass_t = BOOL(WINAPI*)(HWND, SUBCLASSPROC, UINT_PTR);
        ((RemoveWindowSubclass_t)wParam)(list, ListSubclass, kListSubclassId);
        return 0;
    }
    if (message == LVM_SETBKCOLOR || message == LVM_SETTEXTBKCOLOR) lParam = g_listGround;
    else if (message == LVM_SETTEXTCOLOR) lParam = g_listText;
    return g_defSubclassProc(list, message, wParam, lParam);
}

void ColourList(HWND list) {
    if (GetPropW(list, L"TourneTaskManagerList")) return;
    SetPropW(list, L"TourneTaskManagerList", (HANDLE)1);
    if (g_setWindowSubclass && g_defSubclassProc && g_setWindowSubclass(list, ListSubclass, kListSubclassId, 0)) {
        AcquireSRWLockExclusive(&g_listsLock);
        g_lists.push_back(list);
        ReleaseSRWLockExclusive(&g_listsLock);
    }
    SendMessageW(list, LVM_SETBKCOLOR, 0, g_listGround);
    SendMessageW(list, LVM_SETTEXTBKCOLOR, 0, g_listGround);
    SendMessageW(list, LVM_SETTEXTCOLOR, 0, g_listText);
    if (HWND header = (HWND)SendMessageW(list, LVM_GETHEADER, 0, 0))
        if (Luma(g_listGround) <= 128) SetWindowTheme(header, L"DarkMode_ItemsView", nullptr);
    InvalidateRect(list, nullptr, TRUE);
}

bool IsList(HWND wnd) {
    WCHAR cls[32];
    return GetClassNameW(wnd, cls, ARRAYSIZE(cls)) && _wcsicmp(cls, WC_LISTVIEWW) == 0;
}

void CALLBACK ListEvent(HWINEVENTHOOK, DWORD, HWND wnd, LONG object, LONG child, DWORD, DWORD) {
    if (wnd && object == OBJID_WINDOW && child == CHILDID_SELF && IsList(wnd)) ColourList(wnd);
}

HMODULE ThisModule() {
    HMODULE module = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&ThisModule, &module);
    return module;
}

void StartListColours() {
    HMODULE comctl = GetModuleHandleW(L"comctl32.dll");
    if (!comctl) comctl = LoadLibraryExW(L"comctl32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (comctl) {
        g_setWindowSubclass = (SetWindowSubclass_t)GetProcAddress(comctl, "SetWindowSubclass");
        g_defSubclassProc = (DefSubclassProc_t)GetProcAddress(comctl, "DefSubclassProc");
    }
    // (its creation and its showing, on the thread that owns it, which subclassing needs; the lists are made
    // when their tab is first opened, long after this)
    g_listHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_SHOW, ThisModule(), ListEvent, GetCurrentProcessId(),
                                 0, WINEVENT_INCONTEXT);
}

// Before this part's code goes: the lists let go of it.
void StopListColours() {
    if (g_listHook) UnhookWinEvent(g_listHook);
    g_listHook = nullptr;
    HMODULE comctl = GetModuleHandleW(L"comctl32.dll");
    using RemoveWindowSubclass_t = BOOL(WINAPI*)(HWND, SUBCLASSPROC, UINT_PTR);
    auto remove = comctl ? (RemoveWindowSubclass_t)GetProcAddress(comctl, "RemoveWindowSubclass") : nullptr;
    AcquireSRWLockExclusive(&g_listsLock);
    std::vector<HWND> lists = std::move(g_lists);
    ReleaseSRWLockExclusive(&g_listsLock);
    for (HWND list : lists) {
        if (!IsWindow(list)) continue;
        // (on the list's own thread, as subclassing must be)
        if (remove) SendMessageTimeoutW(list, g_removeMessage, (WPARAM)remove, 0, SMTO_ABORTIFHUNG, 1000, nullptr);
        RemovePropW(list, L"TourneTaskManagerList");
    }
}

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;
HWND WINAPI CreateWindowExW_Hook(DWORD exStyle, LPCWSTR className, LPCWSTR title, DWORD style, int x, int y,
                                 int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param) {
    HWND wnd = CreateWindowExW_Original(exStyle, className, title, style, x, y, w, h, parent, menu, instance, param);
    // A dark background wants the text previewer's scroll bars dark too.
    if (wnd && className && !IS_INTRESOURCE(className) && _wcsicmp(className, L"RICHEDIT50W") == 0 &&
        Luma(g_previewColor) <= 128) {
        if (g_allowDarkModeForWindow) g_allowDarkModeForWindow(wnd, TRUE);
        SetWindowTheme(wnd, L"Explorer", nullptr);
    }
    return wnd;
}

////////////////////////////////////////////////////////////////////////////////
// Task Manager

// Task Manager's usage cells are brushes DirectUI makes in six fixed shades,
// lightest for the least use (read off with a probe of its GDI calls). Each
// becomes a step along the drive bar's colours: the empty part, then the
// filled part fading to its end, then the nearly full colour at the top.
constexpr COLORREF kHeatShades[] = {
    RGB(0xFF, 0xF4, 0xC4), RGB(0xF9, 0xEC, 0xA8), RGB(0xFF, 0xE4, 0x87),
    RGB(0xFF, 0xD2, 0x64), RGB(0xFC, 0xA7, 0x1D), RGB(0xFC, 0x68, 0x2A),
};
COLORREF g_heat[ARRAYSIZE(kHeatShades)];
bool g_heatSet[ARRAYSIZE(kHeatShades)];
HMODULE g_dui;

void SetHeat(Config const& c) {
    COLORREF from = c.fill.from, to = c.fill.to;
    g_heatSet[0] = c.empty.set, g_heat[0] = c.empty.from;
    constexpr float kSteps[] = {0.0f, 0.35f, 0.7f, 1.0f};
    for (int i = 0; i < 4; i++) g_heatSet[i + 1] = c.fill.set, g_heat[i + 1] = Mix(from, to, kSteps[i]);
    g_heatSet[5] = c.full.set || c.fill.set, g_heat[5] = c.full.set ? c.full.from : to;
}

// Grouped by type, a group's heading row ("Apps (5)") fills its usage cells with one more shade, a cream; it
// takes the list's own ground (the cells read as part of the heading), else the drive bar's empty colour.
constexpr COLORREF kHeatGroupShade = RGB(0xF3, 0xED, 0xD9);

bool HeatShade(COLORREF color, void* returnAddress, COLORREF* out) {
    color &= 0xFFFFFF;
    // (from whichever part of Task Manager asks: it isn't DirectUI's own brush, and the shade is used for nothing else)
    if (color == kHeatGroupShade && (g_listColours || g_heatSet[0])) {
        *out = g_listColours ? g_listGround : g_heat[0];
        return true;
    }
    for (int i = 0; i < (int)ARRAYSIZE(kHeatShades); i++) {
        if (color != kHeatShades[i]) continue;
        if (!g_heatSet[i]) return false;
        HMODULE caller = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                (LPCWSTR)returnAddress, &caller) ||
            !caller || caller != g_dui)
            return false;
        *out = g_heat[i];
        return true;
    }
    return false;
}

using CreateSolidBrush_t = decltype(&CreateSolidBrush);
using SetDCBrushColor_t = decltype(&SetDCBrushColor);
CreateSolidBrush_t CreateSolidBrush_Original;
SetDCBrushColor_t SetDCBrushColor_Original;

HBRUSH WINAPI CreateSolidBrush_Hook(COLORREF color) {
    COLORREF mine;
    if (HeatShade(color, __builtin_return_address(0), &mine)) color = mine;
    return CreateSolidBrush_Original(color);
}

using CreateBrushIndirect_t = decltype(&CreateBrushIndirect);
using SetBkColor_t = decltype(&SetBkColor);
CreateBrushIndirect_t CreateBrushIndirect_Original;
SetBkColor_t SetBkColor_Original;

HBRUSH WINAPI CreateBrushIndirect_Hook(const LOGBRUSH* brush) {
    COLORREF mine;
    if (brush && brush->lbStyle == BS_SOLID && HeatShade(brush->lbColor, __builtin_return_address(0), &mine)) {
        LOGBRUSH b = *brush;
        b.lbColor = mine;
        return CreateBrushIndirect_Original(&b);
    }
    return CreateBrushIndirect_Original(brush);
}

// A fill made as text's background (SetBkColor, then ExtTextOut with ETO_OPAQUE): the group rows' cream only.
COLORREF WINAPI SetBkColor_Hook(HDC dc, COLORREF color) {
    if ((color & 0xFFFFFF) == kHeatGroupShade && (g_listColours || g_heatSet[0])) color = g_listColours ? g_listGround : g_heat[0];
    return SetBkColor_Original(dc, color);
}

// A fill made as a gradient (DirectUI's fills go through GradientFill): each corner in the group rows' cream
// takes the same colour as the other paths give it. A vertex's channels are 16 bits, the colour in the top 8.
using GdiGradientFill_t = BOOL(WINAPI*)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
GdiGradientFill_t GdiGradientFill_Original;

BOOL WINAPI GdiGradientFill_Hook(HDC dc, PTRIVERTEX vertices, ULONG count, PVOID mesh, ULONG meshCount, ULONG mode) {
    if (vertices && count && count <= 64 && (g_listColours || g_heatSet[0])) {
        TRIVERTEX copy[64];
        bool changed = false;
        COLORREF to = g_listColours ? g_listGround : g_heat[0];
        for (ULONG i = 0; i < count; i++) {
            copy[i] = vertices[i];
            COLORREF c = RGB(vertices[i].Red >> 8, vertices[i].Green >> 8, vertices[i].Blue >> 8);
            if (c != kHeatGroupShade) continue;
            copy[i].Red = (COLOR16)(GetRValue(to) << 8), copy[i].Green = (COLOR16)(GetGValue(to) << 8), copy[i].Blue = (COLOR16)(GetBValue(to) << 8);
            changed = true;
        }
        if (changed) return GdiGradientFill_Original(dc, copy, count, mesh, meshCount, mode);
    }
    return GdiGradientFill_Original(dc, vertices, count, mesh, meshCount, mode);
}

// A see-through fill (DirectUI makes those with GDI+, its colour as ARGB): the group rows' cream band, its alpha kept.
using GdipCreateSolidFill_t = int(WINAPI*)(DWORD argb, void** brush);
GdipCreateSolidFill_t GdipCreateSolidFill_Original;

int WINAPI GdipCreateSolidFill_Hook(DWORD argb, void** brush) {
    COLORREF rgb = RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);
    if (rgb == kHeatGroupShade && (g_listColours || g_heatSet[0])) {
        COLORREF to = g_listColours ? g_listGround : g_heat[0];
        argb = (argb & 0xFF000000) | ((DWORD)GetRValue(to) << 16) | ((DWORD)GetGValue(to) << 8) | GetBValue(to);
    }
    return GdipCreateSolidFill_Original(argb, brush);
}

COLORREF WINAPI SetDCBrushColor_Hook(HDC dc, COLORREF color) {
    COLORREF mine;
    if (HeatShade(color, __builtin_return_address(0), &mine)) color = mine;
    return SetDCBrushColor_Original(dc, color);
}

////////////////////////////////////////////////////////////////////////////////
// Part

std::wstring ProcessName() {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) {
        return {};
    }
    PCWSTR name = wcsrchr(path, L'\\');
    return name ? name + 1 : path;
}

bool ListedProgram(std::wstring const& name) {
    std::wstring list = TextSetting(L"otherPrograms");
    size_t start = 0;
    while (start <= list.size()) {
        size_t comma = list.find(L',', start);
        std::wstring item = list.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty() && _wcsicmp(item.c_str(), name.c_str()) == 0) {
            return true;
        }
        if (comma == std::wstring::npos) {
            break;
        }
        start = comma + 1;
    }
    return false;
}

bool ShouldLoadInThisProcess() {
    std::wstring name = ProcessName();
    if (_wcsicmp(name.c_str(), L"explorer.exe") == 0 || _wcsicmp(name.c_str(), L"prevhost.exe") == 0) {
        return true;
    }
    if (_wcsicmp(name.c_str(), L"taskmgr.exe") == 0 && Wh_GetIntSetting(L"taskManager")) {
        return true;
    }
    return !name.empty() && ListedProgram(name);
}

// Hooks come and go with what's switched on, so a change there reloads.
bool NeedsReload() {
    Needs now = NeedsFor(ReadConfig());
    if (g_host == Host::Previewer) {
        now.themeBackground = now.themeBackgroundEx = false;
    }
    return !(now == g_needs);
}

BOOL Wh_ModInit() {
    std::wstring name = ProcessName();
    g_host = _wcsicmp(name.c_str(), L"prevhost.exe") == 0   ? Host::Previewer
             : _wcsicmp(name.c_str(), L"explorer.exe") == 0 ? Host::Explorer
                                                            : Host::Dialog;
    g_inTaskManager = _wcsicmp(name.c_str(), L"taskmgr.exe") == 0;
    HMODULE uxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!uxtheme) {
        return FALSE;
    }
    g_getThemeClass = (GetThemeClass_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(74));
    g_allowDarkModeForWindow = (AllowDarkModeForWindow_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(133));

    SetConfig(ReadConfig());
    g_needs = NeedsFor(g_config);
    // The preview host only shows previews.
    if (g_host == Host::Previewer) {
        g_needs.themeBackground = g_needs.themeBackgroundEx = false;
    }
    g_previewColor = g_config.previewColor;
    g_previewText = g_config.previewText;

    if (g_needs.heat) {
        g_listColours = ParseColor(TextSetting(L"taskManagerBackground"), &g_listGround);
        if (g_listColours && !ParseColor(TextSetting(L"taskManagerText"), &g_listText))
            g_listText = Luma(g_listGround) <= 128 ? RGB(0xDC, 0xDC, 0xDC) : RGB(0x20, 0x20, 0x20);
        if (g_listColours) StartListColours();
        g_dui = LoadLibraryExW(L"dui70.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        HMODULE gdi = GetModuleHandleW(L"gdi32full.dll");
        if (!gdi) gdi = GetModuleHandleW(L"gdi32.dll");
        if (g_dui && gdi) {
            SetHeat(g_config);
            Wh_SetFunctionHook((void*)GetProcAddress(gdi, "CreateSolidBrush"), (void*)CreateSolidBrush_Hook,
                               (void**)&CreateSolidBrush_Original);
            Wh_SetFunctionHook((void*)GetProcAddress(gdi, "SetDCBrushColor"), (void*)SetDCBrushColor_Hook,
                               (void**)&SetDCBrushColor_Original);
            Wh_SetFunctionHook((void*)GetProcAddress(gdi, "CreateBrushIndirect"), (void*)CreateBrushIndirect_Hook,
                               (void**)&CreateBrushIndirect_Original);
            Wh_SetFunctionHook((void*)GetProcAddress(gdi, "SetBkColor"), (void*)SetBkColor_Hook, (void**)&SetBkColor_Original);
            if (void* gradient = (void*)GetProcAddress(gdi, "GdiGradientFill"))
                Wh_SetFunctionHook(gradient, (void*)GdiGradientFill_Hook, (void**)&GdiGradientFill_Original);
            // (the GDI+ DirectUI binds to: loaded with it, else loaded now as it would load it, by name)
            HMODULE gdiplus = GetModuleHandleW(L"gdiplus.dll");
            if (!gdiplus) gdiplus = LoadLibraryW(L"gdiplus.dll");
            if (gdiplus)
                if (void* fill = (void*)GetProcAddress(gdiplus, "GdipCreateSolidFill"))
                    Wh_SetFunctionHook(fill, (void*)GdipCreateSolidFill_Hook, (void**)&GdipCreateSolidFill_Original);
        }
    }

    if (g_needs.themeBackground || g_needs.themeBackgroundEx) {
        if (!g_getThemeClass) {
            Wh_Log(L"uxtheme's GetThemeClass wasn't found");
            return FALSE;
        }
        Wh_SetFunctionHook((void*)GetProcAddress(uxtheme, "DrawThemeBackground"), (void*)DrawThemeBackground_Hook,
                           (void**)&DrawThemeBackground_Original);
    }
    if (g_needs.themeBackgroundEx) {
        // Without DirectUI's help, a click's stale selection just shows for a moment. (Not in Task Manager, whose
        // DirectUI isn't a file list: its clicks would be misread.)
        if (!g_inTaskManager) HookDirectUi();
        Wh_SetFunctionHook((void*)GetProcAddress(uxtheme, "DrawThemeBackgroundEx"),
                           (void*)DrawThemeBackgroundEx_Hook, (void**)&DrawThemeBackgroundEx_Original);
    }
    if (g_needs.preview) {
        Wh_SetFunctionHook((void*)RegisterClassExW, (void*)RegisterClassExW_Hook, (void**)&RegisterClassExW_Original);
        if (g_getThemeClass) {
            Wh_SetFunctionHook((void*)GetProcAddress(uxtheme, "GetThemeColor"), (void*)GetThemeColor_Hook,
                               (void**)&GetThemeColor_Original);
        }
        if (g_host == Host::Previewer) {
            Wh_SetFunctionHook((void*)GetSysColor, (void*)GetSysColor_Hook, (void**)&GetSysColor_Original);
            Wh_SetFunctionHook((void*)CreateWindowExW, (void*)CreateWindowExW_Hook, (void**)&CreateWindowExW_Original);
            // Dark scroll bars need the process to allow dark mode.
            using SetPreferredAppMode_t = int(WINAPI*)(int);
            auto setMode = (SetPreferredAppMode_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(135));
            if (setMode && Luma(g_previewColor) <= 128) {
                setMode(2);  // Force dark
            }
        }
    }
    return TRUE;
}

void Wh_ModUninit() {
    StopListColours();
    AcquireSRWLockExclusive(&g_drawLock);
    g_scratch.Free();
    g_overBlack = {};
    ReleaseSRWLockExclusive(&g_drawLock);
}

void Wh_ModSettingsChanged() {
    SetConfig(ReadConfig());
    if (g_inTaskManager) SetHeat(CurrentConfig());
}
