// ==WindhawkMod==
// @id                  tourne-system-text-color
// @name                Tourne'Text [System Text Color]
// @description         Recolors white system text to any color you pick, across classic Win32 and themed surfaces, tooltips and menu separators included
// @version             1.8.1
// @author              USER-TOURNE
// @github              https://github.com/USER-TOURNE
// @donateUrl           https://ko-fi.com/tourne
// @license             MIT
// @include             *
// @architecture        x86-64
// @compilerOptions     -lcomctl32 -lgdi32 -lmsimg32 -luser32 -luxtheme
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Text` **[System Text Color]**

Repaints white system text in whatever color you want.

Text that looks white reaches the screen two ways on a classic Win32 surface, and this
mod covers both:

- **GDI text.** `SetTextColor` sets the color on the device context, and everything from
  `TextOut` to `DrawText` picks it up from there. Any color bright enough to count as
  white is swapped for yours on the way through.
- **Themed text.** `DrawThemeText` and `DrawThemeTextEx` take their color from the visual
  style. The theme's color is read first, and if it is white, the call is redirected to
  yours.

Menus are the awkward case. Windows paints them from inside user32 and uxtheme, which
call their own internal drawing rather than the exported entry points, so neither hook
above ever sees them. They are caught one level down instead, at `ExtTextOut` and
`TextOut`, where every piece of GDI text ends up no matter who set the color: if the
color already on the device context looks white, it is swapped for the duration of that
one call. Monochrome device contexts are skipped, because text drawn into a 1bpp bitmap
is a mask, not something anybody sees.

## Tooltips

Tooltips get their own two settings, because neither rule above fits them: their text is
usually dark rather than white, and their background is painted by the visual style.

Tooltip windows are subclassed, including ones that already existed when the mod loaded,
so the mod knows when one is painting. During that paint the background is filled with
**Tooltip background**, over the theme's own, and every piece of text is drawn in
**Tooltip text color** whatever color it started as. Leave either empty to keep what the
visual style does.

Some tooltips are not painted with GDI at all. StartAllBack renders its taskbar tooltip
itself, and no amount of hooking the drawing reaches it. Those are covered from the other
end instead, by answering the questions any renderer has to ask: `COLOR_INFOTEXT` and
`COLOR_INFOBK` among the system colors, and `TMT_TEXTCOLOR`, `TMT_FILLCOLOR` and
`TMT_BORDERCOLOR` on any theme opened for a tooltip class. Both are answered with the
same settings, so a tooltip is colored whether it draws itself or lets Windows do it.

**Keep tooltip border** insets the fill by one pixel so the outline the style drew
survives. Turn it off to fill the tooltip edge to edge.

**Tooltip border color** repaints that outline instead of preserving it, drawn over the
fill at the tooltip's own bounds, with **Tooltip border thickness** for anything heavier
than a hairline. Setting it overrides Keep tooltip border, since the fill is then inset
by the thickness of the new outline rather than by one pixel.

Both colors take alpha, and the background really is blended, so `#40000000` gives a
tint rather than a solid fill.

## Menu separators

A classic menu separator is an etched edge, and Windows draws it from two system colors:
`COLOR_3DSHADOW` for the upper line, `COLOR_3DHILIGHT` for the lower one. Setting those
directly does work, but they are system wide, so every other etched edge changes with
them: group box frames, classic dialog borders, toolbar dividers.

**Menu separator color** repaints the separator from inside the draw call instead, gated
on the device context belonging to a menu, so nothing outside a menu is touched. Leave it
empty and separators are left exactly as Windows draws them.

**Menu separator shadow color** is the upper of the two lines, the one that gives the
separator its etched look. Leave it empty for a single flat line in the separator color,
which is usually what suits a dark menu.

Both take alpha and are genuinely blended against what is behind them, so `#80000000` is
a real half-transparent black rather than a pre-mixed approximation of one.

**Menu separator thickness** applies to each line on its own, so a separator with a
shadow set to thickness 2 stands four pixels tall.

## What it cannot reach

**XAML and WinUI surfaces** are not drawn with GDI: Settings, Start, the stock Windows 11
taskbar and the modern context menus render their own text. Use the Windhawk stylers for
those, overriding brushes such as `TextFillColorPrimaryBrush`.

**Apps that draw their own text**, such as browsers and Electron apps, are untouched by
design. Leave them out of the process list and nothing in them changes.

**A visual style's own colors** can be edited at the source with msstyleEditor instead,
which needs no injection at all. That is the better tool for a blanket change; this mod
is for everything the style does not own, and for per-process control.

## Scope

**Processes** decides where the mod loads. It is a list of executable names, and `*`
means everywhere, which is a big hammer: every process on the system gets the hooks. The
default list is the shell and the classic control-panel hosts.

## Tuning

**White threshold** decides what counts as white. A color qualifies when all three of its
channels are at or above it, so 200 catches off-whites and light greys, while 255 matches
pure white only. Lower it too far and colored text starts changing too.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- logText: false
  $name: Log replaced text
  $description: With verbose logging on, also log the first words of each run that gets recolored. Noisy, for tracking down a surface that will not change.
- textColor: 64aa89
  $name: Text color
  $description: 'Replacement color. Format is #RRGGBB or #AARRGGBB, with or without the hash. Alpha is accepted for consistency with the other Tourne mods but GDI text has no alpha channel, so it is ignored.'
- threshold: 200
  $name: White threshold
  $description: A color counts as white when red, green and blue are all at or above this. 255 matches pure white only.
- processes: explorer.exe, taskmgr.exe, dllhost.exe, rundll32.exe, mmc.exe, regedit.exe
  $name: Processes
  $description: 'Comma separated executable names to load in. Use * for every process.'
- tooltipBackground: ""
  $name: Tooltip background
  $description: 'Fill painted over the tooltip background. Format is #RRGGBB or #AARRGGBB, and alpha is blended. Leave empty to keep the visual style''s own.'
- tooltipTextColor: ""
  $name: Tooltip text color
  $description: 'Applied to every piece of tooltip text whatever color it started as, so it catches dark tooltip text too. Leave empty to keep the visual style''s own.'
- tooltipKeepBorder: true
  $name: Keep tooltip border
  $description: Inset the fill so the outline the visual style drew survives. Turn off to fill the tooltip edge to edge.
- tooltipBorderColor: ""
  $name: Tooltip border color
  $description: 'Repaints the tooltip outline. Format is #RRGGBB or #AARRGGBB, and alpha is blended. Leave empty to keep the visual style''s own.'
- tooltipBorderThickness: 1
  $name: Tooltip border thickness (pixels)
- separatorColor: ""
  $name: Menu separator color
  $description: 'Repaints the separator lines in classic menus and leaves every other etched edge alone. Format is #AARRGGBB or #RRGGBB, and alpha is blended. Leave empty to keep the system''s own.'
- separatorShadowColor: ""
  $name: Menu separator shadow color
  $description: 'The upper of the separator''s two lines, the one that gives it the etched look. Leave empty for a single flat line in Menu separator color.'
- separatorThickness: 1
  $name: Menu separator thickness (pixels)
  $description: Applied to each line on its own, so a separator with a shadow set to thickness 2 stands four pixels tall.
- recolorGdi: true
  $name: Recolor GDI text
- recolorThemed: true
  $name: Recolor themed text
- recolorChromeMenus: true
  $name: Recolor Chrome menus
  $description: >-
    Chrome draws its menus itself, so the text hooks above never see them.
    With chrome.exe in Processes, the light colors a Chrome menu item asks for
    (its label, shortcut text, submenu arrow and checkmark) take the text color.
    Needs Chrome's public symbols once per Chrome version, the same ones the
    Chrome Context Menu Items mod uses.
- squareChromeMenus: true
  $name: Square Chrome menus
  $description: >-
    Chrome rounds the corners of its menus and of the outline other mods draw
    around them. On, they're square, to match the pixel style. Needs chrome.exe
    in Processes, like Recolor Chrome menus.
- logging: false
  $name: Verbose logging
  $description: Log each color that gets replaced, once per distinct color
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

#include <algorithm>
#include <atomic>
#include <string>
#include <unordered_map>

COLORREF g_textColor = RGB(0x64, 0xAA, 0x89);
int g_threshold = 200;
bool g_recolorGdi = true;
bool g_recolorThemed = true;
bool g_recolorChromeMenus = true;
bool g_squareChromeMenus = true;
bool g_logging = false;
bool g_logText = false;

bool g_hasTooltipBackground = false;
COLORREF g_tooltipBackground = 0;
BYTE g_tooltipBackgroundAlpha = 0xFF;
bool g_hasTooltipTextColor = false;
COLORREF g_tooltipTextColor = 0;
bool g_tooltipKeepBorder = true;
bool g_hasTooltipBorder = false;
COLORREF g_tooltipBorder = 0;
BYTE g_tooltipBorderAlpha = 0xFF;
int g_tooltipBorderThickness = 1;

bool g_hasSeparator = false;
COLORREF g_separator = 0;
BYTE g_separatorAlpha = 0xFF;
bool g_hasSeparatorShadow = false;
COLORREF g_separatorShadow = 0;
BYTE g_separatorShadowAlpha = 0xFF;
int g_separatorThickness = 1;

// Non-zero only while a tooltip window is painting, on that window's thread.
thread_local int g_tooltipPaintDepth = 0;
thread_local HWND g_tooltipPaintWindow = nullptr;

// One fill per paint, whichever path gets there first.
thread_local bool g_tooltipFilled = false;

// Last color we reported, so verbose logging does not fire on every glyph run.
COLORREF g_lastLogged = CLR_INVALID;

// Accepts #AARRGGBB, #RRGGBB and the same spellings without the hash. Alpha is reported
// when the caller asks for it, and ignored otherwise: GDI text is opaque.
static bool ParseColor(PCWSTR value, COLORREF* out, BYTE* alphaOut = nullptr) {
    while (*value == L' ' || *value == L'\t') {
        value++;
    }
    if (*value == L'#') {
        value++;
    }
    if (!*value) {
        return false;
    }

    unsigned int packed = 0;
    int digits = 0;
    for (PCWSTR p = value; *p; p++) {
        int digit;
        if (*p >= L'0' && *p <= L'9') {
            digit = *p - L'0';
        } else if (*p >= L'a' && *p <= L'f') {
            digit = *p - L'a' + 10;
        } else if (*p >= L'A' && *p <= L'F') {
            digit = *p - L'A' + 10;
        } else {
            return false;
        }

        packed = (packed << 4) | digit;
        if (++digits > 8) {
            return false;
        }
    }

    if (digits != 6 && digits != 8) {
        return false;
    }

    if (alphaOut) {
        *alphaOut = digits == 8 ? (BYTE)(packed >> 24) : 0xFF;
    }

    *out = RGB((packed >> 16) & 0xFF, (packed >> 8) & 0xFF, packed & 0xFF);
    return true;
}

static bool IsWhitish(COLORREF color) {
    if (color == CLR_INVALID) {
        return false;
    }
    return GetRValue(color) >= g_threshold && GetGValue(color) >= g_threshold &&
           GetBValue(color) >= g_threshold;
}

static COLORREF Recolor(COLORREF color) {
    if (!IsWhitish(color)) {
        return color;
    }

    if (g_logging && color != g_lastLogged) {
        g_lastLogged = color;
        Wh_Log(L"replacing 0x%06X", (unsigned int)color);
    }

    return g_textColor;
}

// --- tooltip surface ------------------------------------------------------

static bool IsTooltipWindow(HWND hWnd) {
    WCHAR className[64];
    return hWnd && GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
           _wcsicmp(className, TOOLTIPS_CLASSW) == 0;
}

// Defined with the theme hooks further down, and used here: a draw made with a theme
// opened for a tooltip class is a tooltip, whatever window or surface it lands on.
static bool IsTooltipTheme(HTHEME hTheme);

struct TooltipSearch {
    LONG width;
    LONG height;
    HWND found;
    int visibleCount;
};

static BOOL CALLBACK FindVisibleTooltipProc(HWND hWnd, LPARAM lParam) {
    TooltipSearch* search = reinterpret_cast<TooltipSearch*>(lParam);

    if (!IsWindowVisible(hWnd) || !IsTooltipWindow(hWnd)) {
        return TRUE;
    }

    search->visibleCount++;

    RECT client;
    if (!GetClientRect(hWnd, &client)) {
        return TRUE;
    }

    // When the surface size is known, only the tooltip it matches is the right one.
    if (search->width > 0) {
        if (client.right == search->width && client.bottom == search->height) {
            search->found = hWnd;
            return FALSE;
        }
        return TRUE;
    }

    search->found = hWnd;
    return TRUE;
}

// A tooltip painting into a memory buffer has no window behind its device context, so it
// is matched by surface size against the visible tooltips on this thread instead.
static HWND FindTooltipByPaintSurface(HDC hdc) {
    TooltipSearch search = {};

    HGDIOBJ bitmap = GetCurrentObject(hdc, OBJ_BITMAP);
    BITMAP info;
    if (bitmap && GetObjectW(bitmap, sizeof(info), &info) == sizeof(info)) {
        search.width = info.bmWidth;
        search.height = info.bmHeight;
    }

    EnumThreadWindows(GetCurrentThreadId(), FindVisibleTooltipProc,
                      reinterpret_cast<LPARAM>(&search));

    // Without a size to match on, only an unambiguous single tooltip is accepted.
    if (search.width == 0 && search.visibleCount != 1) {
        return nullptr;
    }

    return search.found;
}

// Three ways in, in order of confidence: the subclass flag, the window that owns the
// device context, then a size match against the visible tooltips on this thread.
static HWND CurrentTooltipWindow(HDC hdc) {
    if (g_tooltipPaintDepth > 0) {
        return g_tooltipPaintWindow;
    }

    if (!g_hasTooltipBackground && !g_hasTooltipTextColor && !g_hasTooltipBorder) {
        return nullptr;
    }

    HWND hWnd = WindowFromDC(hdc);
    if (hWnd) {
        return IsTooltipWindow(hWnd) ? hWnd : nullptr;
    }

    return FindTooltipByPaintSurface(hdc);
}

// Tooltip background parts. 5 is the close button and 6 the balloon stem, both left be.
static bool IsTooltipBackgroundPart(int iPartId) {
    return iPartId >= TTP_STANDARD && iPartId <= TTP_BALLOONTITLE;
}

// A single premultiplied pixel, stretched over each rect it is handed. Keeps partial
// alpha correct on a per-pixel-alpha surface as well as an opaque one.
class SolidBlender {
   public:
    SolidBlender(HDC hdc, COLORREF color, BYTE alpha)
        : m_target(hdc), m_sourceDc(nullptr), m_bitmap(nullptr), m_previous(nullptr) {
        BITMAPINFO info = {};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = 1;
        info.bmiHeader.biHeight = -1;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        m_bitmap = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!m_bitmap) {
            return;
        }

        m_sourceDc = CreateCompatibleDC(hdc);
        if (!m_sourceDc) {
            DeleteObject(m_bitmap);
            m_bitmap = nullptr;
            return;
        }

        BYTE red = (BYTE)(GetRValue(color) * alpha / 255);
        BYTE green = (BYTE)(GetGValue(color) * alpha / 255);
        BYTE blue = (BYTE)(GetBValue(color) * alpha / 255);
        *reinterpret_cast<DWORD*>(bits) =
            ((DWORD)alpha << 24) | ((DWORD)red << 16) | ((DWORD)green << 8) | blue;

        m_previous = SelectObject(m_sourceDc, m_bitmap);
    }

    ~SolidBlender() {
        if (m_sourceDc) {
            SelectObject(m_sourceDc, m_previous);
            DeleteDC(m_sourceDc);
        }
        if (m_bitmap) {
            DeleteObject(m_bitmap);
        }
    }

    SolidBlender(const SolidBlender&) = delete;
    SolidBlender& operator=(const SolidBlender&) = delete;

    bool Ready() const { return m_sourceDc != nullptr; }

    void Fill(const RECT& rect) const {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        if (!m_sourceDc || width <= 0 || height <= 0) {
            return;
        }

        BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        AlphaBlend(m_target, rect.left, rect.top, width, height, m_sourceDc, 0, 0, 1, 1, blend);
    }

    void Frame(const RECT& rect, int thickness) const {
        const RECT edges[] = {
            {rect.left, rect.top, rect.right, rect.top + thickness},
            {rect.left, rect.bottom - thickness, rect.right, rect.bottom},
            {rect.left, rect.top + thickness, rect.left + thickness, rect.bottom - thickness},
            {rect.right - thickness, rect.top + thickness, rect.right, rect.bottom - thickness},
        };
        for (const RECT& edge : edges) {
            Fill(edge);
        }
    }

   private:
    HDC m_target;
    HDC m_sourceDc;
    HBITMAP m_bitmap;
    HGDIOBJ m_previous;
};

static int TooltipBorderThickness() {
    if (g_tooltipBorderThickness < 1) {
        return 1;
    }
    return g_tooltipBorderThickness > 32 ? 32 : g_tooltipBorderThickness;
}

// Paints the tooltip's own surface: the fill, then the outline over it.
static void FillTooltipBackground(HDC hdc, RECT rect) {
    int inset = g_hasTooltipBorder ? TooltipBorderThickness() : (g_tooltipKeepBorder ? 1 : 0);

    if (g_hasTooltipBackground) {
        RECT fill = rect;
        InflateRect(&fill, -inset, -inset);

        SolidBlender blender(hdc, g_tooltipBackground, g_tooltipBackgroundAlpha);
        if (blender.Ready()) {
            blender.Fill(fill);
        }
    }

    if (g_hasTooltipBorder) {
        int thickness = TooltipBorderThickness();
        if (rect.right - rect.left > thickness * 2 && rect.bottom - rect.top > thickness * 2) {
            SolidBlender blender(hdc, g_tooltipBorder, g_tooltipBorderAlpha);
            if (blender.Ready()) {
                blender.Frame(rect, thickness);
            }
        }
    }

    if (g_logging) {
        Wh_Log(L"painted tooltip surface %d,%d-%d,%d", rect.left, rect.top, rect.right,
               rect.bottom);
    }
}

// A paint is only bounded when the tooltip is subclassed, where one flag is exact.
// Without it, repeat fills are held off by time instead: a paint's draws land within
// microseconds of each other, while two separate showings are seconds apart.
constexpr DWORD kFillDebounceMs = 50;

thread_local HWND g_lastFillWindow = nullptr;
thread_local HDC g_lastFillDc = nullptr;
thread_local DWORD g_lastFillTick = 0;

static bool ShouldFillTooltipBackground(HWND hWnd, HDC hdc) {
    if (!g_hasTooltipBackground && !g_hasTooltipBorder) {
        return false;
    }

    if (g_tooltipPaintDepth > 0) {
        return !g_tooltipFilled;
    }

    return hWnd != g_lastFillWindow || hdc != g_lastFillDc ||
           GetTickCount() - g_lastFillTick >= kFillDebounceMs;
}

static void MarkTooltipFilled(HWND hWnd, HDC hdc) {
    if (g_tooltipPaintDepth > 0) {
        g_tooltipFilled = true;
        return;
    }

    g_lastFillWindow = hWnd;
    g_lastFillDc = hdc;
    g_lastFillTick = GetTickCount();
}

// Last resort, and the one that does not care how the background was painted: fill the
// tooltip's client area immediately before its first piece of text goes down.
static void FillTooltipBackgroundBeforeText(HDC hdc, HWND hWnd) {
    if (!hWnd || !ShouldFillTooltipBackground(hWnd, hdc)) {
        return;
    }

    RECT client;
    if (!GetClientRect(hWnd, &client)) {
        return;
    }

    MarkTooltipFilled(hWnd, hdc);
    FillTooltipBackground(hdc, client);
}

// --- GDI text -------------------------------------------------------------

using SetTextColor_t = COLORREF(WINAPI*)(HDC, COLORREF);
SetTextColor_t SetTextColor_Original;

COLORREF WINAPI SetTextColor_Hook(HDC hdc, COLORREF color) {
    // The return value is the real previous color, so callers that save and restore it
    // keep working.
    return SetTextColor_Original(hdc, g_recolorGdi ? Recolor(color) : color);
}

// Text drawn into a monochrome bitmap is a mask being built, not something on screen.
// Recoloring it would change which pixels land on the wrong side of the 1bpp threshold.
static bool IsMaskDc(HDC hdc) {
    return GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES) <= 1;
}

// Swaps the device context's text color for the length of one draw call. This is the
// catch-all: it does not care who set the color or which drawing routine is running, so
// it reaches menus and anything else painted from inside user32 or uxtheme.
class ScopedTextColor {
   public:
    ScopedTextColor(HDC hdc, PCWSTR text, int count) : m_hdc(nullptr), m_previous(CLR_INVALID) {
        if (!SetTextColor_Original || IsMaskDc(hdc)) {
            return;
        }

        HWND tooltip = CurrentTooltipWindow(hdc);
        if (tooltip) {
            FillTooltipBackgroundBeforeText(hdc, tooltip);
        }

        if (g_logging && g_logText && text && count != 0) {
            HWND owner = WindowFromDC(hdc);
            WCHAR className[64] = L"(memory dc)";
            if (owner) {
                GetClassNameW(owner, className, ARRAYSIZE(className));
            }
            Wh_Log(L"text \"%.32s\" dc window %s, tooltip %p, paint depth %d", text, className,
                   tooltip, g_tooltipPaintDepth);
        }

        COLORREF current = GetTextColor(hdc);
        COLORREF replacement;

        // Inside a tooltip the color is set outright, since tooltip text is usually dark
        // and would never match the whiteness test.
        if (tooltip && g_hasTooltipTextColor) {
            replacement = g_tooltipTextColor;
        } else if (g_recolorGdi && IsWhitish(current)) {
            replacement = Recolor(current);
        } else {
            return;
        }

        if (replacement == current) {
            return;
        }

        m_hdc = hdc;
        m_previous = current;
        SetTextColor_Original(hdc, replacement);
    }

    ~ScopedTextColor() {
        if (m_hdc) {
            SetTextColor_Original(m_hdc, m_previous);
        }
    }

    ScopedTextColor(const ScopedTextColor&) = delete;
    ScopedTextColor& operator=(const ScopedTextColor&) = delete;

   private:
    HDC m_hdc;
    COLORREF m_previous;
};

using ExtTextOutW_t = BOOL(WINAPI*)(HDC, int, int, UINT, const RECT*, LPCWSTR, UINT, const INT*);
ExtTextOutW_t ExtTextOutW_Original;

BOOL WINAPI ExtTextOutW_Hook(HDC hdc, int x, int y, UINT options, const RECT* lprect,
                             LPCWSTR lpString, UINT c, const INT* lpDx) {
    ScopedTextColor scoped(hdc, lpString, (int)c);
    return ExtTextOutW_Original(hdc, x, y, options, lprect, lpString, c, lpDx);
}

// Implemented separately inside gdi32, so the ExtTextOut hook does not cover it.
using TextOutW_t = BOOL(WINAPI*)(HDC, int, int, LPCWSTR, int);
TextOutW_t TextOutW_Original;

BOOL WINAPI TextOutW_Hook(HDC hdc, int x, int y, LPCWSTR lpString, int c) {
    ScopedTextColor scoped(hdc, lpString, c);
    return TextOutW_Original(hdc, x, y, lpString, c);
}

// --- themed text ----------------------------------------------------------

using DrawThemeTextEx_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT,
                                           const DTTOPTS*);
DrawThemeTextEx_t DrawThemeTextEx_Original;

// Works out what color this call would have painted with: the caller's override if there
// is one, otherwise the style's own color for that part and state, otherwise the DC's.
static COLORREF ResolveThemeTextColor(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                      const DTTOPTS* pOptions) {
    if (pOptions && pOptions->dwSize >= sizeof(DTTOPTS) && (pOptions->dwFlags & DTT_TEXTCOLOR)) {
        return pOptions->crText;
    }

    COLORREF color = CLR_INVALID;
    if (SUCCEEDED(GetThemeColor(hTheme, iPartId, iStateId, TMT_TEXTCOLOR, &color))) {
        return color;
    }

    return GetTextColor(hdc);
}

HRESULT WINAPI DrawThemeTextEx_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                    LPCWSTR pszText, int cchText, DWORD dwFlags, LPRECT pRect,
                                    const DTTOPTS* pOptions) {
    HWND tooltip = CurrentTooltipWindow(hdc);
    if (tooltip) {
        FillTooltipBackgroundBeforeText(hdc, tooltip);
    }

    bool tooltipTheme = IsTooltipTheme(hTheme);
    if (g_logging && tooltipTheme) {
        Wh_Log(L"tooltip theme text \"%.32s\" part %d state %d", pszText ? pszText : L"", iPartId,
               iStateId);
    }

    bool inTooltip = (tooltip || tooltipTheme) && g_hasTooltipTextColor;
    if (!g_recolorThemed && !inTooltip) {
        return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwFlags,
                                        pRect, pOptions);
    }

    COLORREF replacement;
    if (inTooltip) {
        replacement = g_tooltipTextColor;
    } else {
        COLORREF current = ResolveThemeTextColor(hTheme, hdc, iPartId, iStateId, pOptions);
        if (!IsWhitish(current)) {
            return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText,
                                            dwFlags, pRect, pOptions);
        }
        replacement = Recolor(current);
    }

    DTTOPTS options = {};
    if (pOptions && pOptions->dwSize >= sizeof(DTTOPTS)) {
        options = *pOptions;
    }
    options.dwSize = sizeof(DTTOPTS);
    options.dwFlags |= DTT_TEXTCOLOR;
    options.crText = replacement;

    return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwFlags,
                                    pRect, &options);
}

using DrawThemeText_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD,
                                         LPCRECT);
DrawThemeText_t DrawThemeText_Original;

HRESULT WINAPI DrawThemeText_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                  LPCWSTR pszText, int cchText, DWORD dwTextFlags,
                                  DWORD dwTextFlags2, LPCRECT pRect) {
    HWND tooltip = CurrentTooltipWindow(hdc);
    if (tooltip) {
        FillTooltipBackgroundBeforeText(hdc, tooltip);
    }

    bool tooltipTheme = IsTooltipTheme(hTheme);
    if (g_logging && tooltipTheme) {
        Wh_Log(L"tooltip theme text \"%.32s\" part %d state %d (non-ex)",
               pszText ? pszText : L"", iPartId, iStateId);
    }

    bool inTooltip = (tooltip || tooltipTheme) && g_hasTooltipTextColor;
    if ((!g_recolorThemed && !inTooltip) || !pRect) {
        return DrawThemeText_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText,
                                      dwTextFlags, dwTextFlags2, pRect);
    }

    COLORREF replacement;
    if (inTooltip) {
        replacement = g_tooltipTextColor;
    } else {
        COLORREF current = ResolveThemeTextColor(hTheme, hdc, iPartId, iStateId, nullptr);
        if (!IsWhitish(current)) {
            return DrawThemeText_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText,
                                          dwTextFlags, dwTextFlags2, pRect);
        }
        replacement = Recolor(current);
    }

    // Routed through the Ex form, which is the only one that takes a color override.
    DTTOPTS options = {};
    options.dwSize = sizeof(DTTOPTS);
    options.dwFlags = DTT_TEXTCOLOR;
    options.crText = replacement;

    RECT rect = *pRect;
    return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags,
                                    &rect, &options);
}

// --- tooltips -------------------------------------------------------------

using DrawThemeBackground_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
DrawThemeBackground_t DrawThemeBackground_Original;

HRESULT WINAPI DrawThemeBackground_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                        LPCRECT pRect, LPCRECT pClipRect) {
    HRESULT hr = DrawThemeBackground_Original(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);

    bool tooltipTheme = IsTooltipTheme(hTheme);
    if (g_logging && tooltipTheme && pRect) {
        Wh_Log(L"tooltip theme background part %d state %d, rect %d,%d-%d,%d", iPartId, iStateId,
               pRect->left, pRect->top, pRect->right, pRect->bottom);
    }

    if (!pRect || FAILED(hr) || !IsTooltipBackgroundPart(iPartId)) {
        return hr;
    }

    HWND tooltip = CurrentTooltipWindow(hdc);
    if ((tooltipTheme || tooltip) && ShouldFillTooltipBackground(tooltip, hdc)) {
        MarkTooltipFilled(tooltip, hdc);
        FillTooltipBackground(hdc, *pRect);
    }

    return hr;
}

using DrawThemeBackgroundEx_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT,
                                                 const DTBGOPTS*);
DrawThemeBackgroundEx_t DrawThemeBackgroundEx_Original;

HRESULT WINAPI DrawThemeBackgroundEx_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                          LPCRECT pRect, const DTBGOPTS* pOptions) {
    HRESULT hr = DrawThemeBackgroundEx_Original(hTheme, hdc, iPartId, iStateId, pRect, pOptions);

    bool tooltipTheme = IsTooltipTheme(hTheme);
    if (g_logging && tooltipTheme && pRect) {
        Wh_Log(L"tooltip theme background (ex) part %d state %d, rect %d,%d-%d,%d", iPartId,
               iStateId, pRect->left, pRect->top, pRect->right, pRect->bottom);
    }

    if (!pRect || FAILED(hr) || !IsTooltipBackgroundPart(iPartId)) {
        return hr;
    }

    HWND tooltip = CurrentTooltipWindow(hdc);
    if ((tooltipTheme || tooltip) && ShouldFillTooltipBackground(tooltip, hdc)) {
        MarkTooltipFilled(tooltip, hdc);
        FillTooltipBackground(hdc, *pRect);
    }

    return hr;
}

LRESULT CALLBACK TooltipSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                     DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_PAINT:
        case WM_NCPAINT:
        case WM_PRINTCLIENT:
        case WM_ERASEBKGND: {
            HWND previousWindow = g_tooltipPaintWindow;
            bool previousFilled = g_tooltipFilled;

            g_tooltipPaintWindow = hWnd;
            g_tooltipFilled = false;
            g_tooltipPaintDepth++;

            if (g_logging) {
                Wh_Log(L"tooltip %p painting, message 0x%04X", hWnd, uMsg);
            }

            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

            g_tooltipPaintDepth--;
            g_tooltipPaintWindow = previousWindow;
            g_tooltipFilled = previousFilled;
            return result;
        }

        case WM_NCDESTROY:
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd, TooltipSubclassProc);
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

static void TrackTooltip(HWND hWnd) {
    BOOL subclassed = WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd, TooltipSubclassProc, 0);
    if (g_logging) {
        Wh_Log(L"tooltip %p %s", hWnd, subclassed ? L"tracked" : L"could not be subclassed");
    }
}

using CreateWindowExW_t = HWND(WINAPI*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND,
                                        HMENU, HINSTANCE, LPVOID);
CreateWindowExW_t CreateWindowExW_Original;

HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,
                                 DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                 HWND hWndParent, HMENU hMenu, HINSTANCE hInstance,
                                 LPVOID lpParam) {
    HWND hWnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y,
                                         nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    // Checked on the created window rather than the class argument, which may be an atom.
    if (hWnd && IsTooltipWindow(hWnd)) {
        TrackTooltip(hWnd);
    }

    return hWnd;
}

static BOOL CALLBACK TrackExistingTooltip(HWND hWnd, LPARAM lParam) {
    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);
    if (processId == GetCurrentProcessId() && IsTooltipWindow(hWnd)) {
        TrackTooltip(hWnd);
    }
    return TRUE;
}

// --- tooltips drawn by someone else ---------------------------------------
//
// A custom renderer, StartAllBack's among them, does not paint tooltip text with GDI, so
// nothing above can see it. It still has to ask the system what color a tooltip should
// be, and there are only two places to ask: the system colors, and the visual style.
// Both are answered here, which reaches the tooltip whatever it draws with.

static bool ContainsNoCase(PCWSTR haystack, PCWSTR needle) {
    size_t length = wcslen(needle);
    for (PCWSTR p = haystack; *p; p++) {
        if (_wcsnicmp(p, needle, length) == 0) {
            return true;
        }
    }
    return false;
}

SRWLOCK g_themeClassLock = SRWLOCK_INIT;
std::unordered_map<HTHEME, std::wstring>* g_themeClasses;

static void RememberThemeClass(HTHEME hTheme, PCWSTR classList) {
    if (!hTheme || !classList || !g_themeClasses) {
        return;
    }

    AcquireSRWLockExclusive(&g_themeClassLock);
    (*g_themeClasses)[hTheme] = classList;
    ReleaseSRWLockExclusive(&g_themeClassLock);

    if (g_logging && ContainsNoCase(classList, L"tooltip")) {
        Wh_Log(L"tooltip theme opened: %s", classList);
    }
}

bool IsTooltipTheme(HTHEME hTheme) {
    if (!hTheme || !g_themeClasses) {
        return false;
    }

    bool result = false;
    AcquireSRWLockShared(&g_themeClassLock);
    auto entry = g_themeClasses->find(hTheme);
    if (entry != g_themeClasses->end()) {
        result = ContainsNoCase(entry->second.c_str(), L"tooltip");
    }
    ReleaseSRWLockShared(&g_themeClassLock);
    return result;
}

using OpenThemeData_t = HTHEME(WINAPI*)(HWND, LPCWSTR);
OpenThemeData_t OpenThemeData_Original;

HTHEME WINAPI OpenThemeData_Hook(HWND hWnd, LPCWSTR pszClassList) {
    HTHEME hTheme = OpenThemeData_Original(hWnd, pszClassList);
    RememberThemeClass(hTheme, pszClassList);
    return hTheme;
}

using OpenThemeDataForDpi_t = HTHEME(WINAPI*)(HWND, LPCWSTR, UINT);
OpenThemeDataForDpi_t OpenThemeDataForDpi_Original;

HTHEME WINAPI OpenThemeDataForDpi_Hook(HWND hWnd, LPCWSTR pszClassList, UINT dpi) {
    HTHEME hTheme = OpenThemeDataForDpi_Original(hWnd, pszClassList, dpi);
    RememberThemeClass(hTheme, pszClassList);
    return hTheme;
}

using OpenThemeDataEx_t = HTHEME(WINAPI*)(HWND, LPCWSTR, DWORD);
OpenThemeDataEx_t OpenThemeDataEx_Original;

HTHEME WINAPI OpenThemeDataEx_Hook(HWND hWnd, LPCWSTR pszClassList, DWORD dwFlags) {
    HTHEME hTheme = OpenThemeDataEx_Original(hWnd, pszClassList, dwFlags);
    RememberThemeClass(hTheme, pszClassList);
    return hTheme;
}

using CloseThemeData_t = HRESULT(WINAPI*)(HTHEME);
CloseThemeData_t CloseThemeData_Original;

HRESULT WINAPI CloseThemeData_Hook(HTHEME hTheme) {
    if (g_themeClasses) {
        AcquireSRWLockExclusive(&g_themeClassLock);
        g_themeClasses->erase(hTheme);
        ReleaseSRWLockExclusive(&g_themeClassLock);
    }
    return CloseThemeData_Original(hTheme);
}

using GetThemeColor_t = HRESULT(WINAPI*)(HTHEME, int, int, int, COLORREF*);
GetThemeColor_t GetThemeColor_Original;

HRESULT WINAPI GetThemeColor_Hook(HTHEME hTheme, int iPartId, int iStateId, int iPropId,
                                  COLORREF* pColor) {
    HRESULT hr = GetThemeColor_Original(hTheme, iPartId, iStateId, iPropId, pColor);

    if (!pColor || !IsTooltipTheme(hTheme)) {
        return hr;
    }

    if (g_logging) {
        Wh_Log(L"tooltip theme color part %d state %d prop %d is 0x%06X", iPartId, iStateId,
               iPropId, (unsigned int)*pColor);
    }

    if (iPropId == TMT_TEXTCOLOR && g_hasTooltipTextColor) {
        *pColor = g_tooltipTextColor;
        return S_OK;
    }

    if (g_hasTooltipBackground &&
        (iPropId == TMT_FILLCOLOR || iPropId == TMT_GRADIENTCOLOR1 ||
         iPropId == TMT_GRADIENTCOLOR2)) {
        *pColor = g_tooltipBackground;
        return S_OK;
    }

    if (iPropId == TMT_BORDERCOLOR && g_hasTooltipBorder) {
        *pColor = g_tooltipBorder;
        return S_OK;
    }

    return hr;
}

// Logging only, to show what a renderer actually asks the Tooltip theme for when it does
// not ask for colors.

using GetThemeFont_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, int, LOGFONTW*);
GetThemeFont_t GetThemeFont_Original;

HRESULT WINAPI GetThemeFont_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId,
                                 LOGFONTW* pFont) {
    if (g_logging && IsTooltipTheme(hTheme)) {
        Wh_Log(L"tooltip theme font, part %d state %d prop %d", iPartId, iStateId, iPropId);
    }
    return GetThemeFont_Original(hTheme, hdc, iPartId, iStateId, iPropId, pFont);
}

using GetThemeMargins_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, int, LPCRECT, MARGINS*);
GetThemeMargins_t GetThemeMargins_Original;

HRESULT WINAPI GetThemeMargins_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                    int iPropId, LPCRECT prc, MARGINS* pMargins) {
    if (g_logging && IsTooltipTheme(hTheme)) {
        Wh_Log(L"tooltip theme margins, part %d state %d prop %d", iPartId, iStateId, iPropId);
    }
    return GetThemeMargins_Original(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
}

using GetThemePartSize_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, THEMESIZE, SIZE*);
GetThemePartSize_t GetThemePartSize_Original;

HRESULT WINAPI GetThemePartSize_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                     LPCRECT prc, THEMESIZE eSize, SIZE* psz) {
    if (g_logging && IsTooltipTheme(hTheme)) {
        Wh_Log(L"tooltip theme part size, part %d state %d", iPartId, iStateId);
    }
    return GetThemePartSize_Original(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
}

// COLOR_INFOTEXT and COLOR_INFOBK exist for tooltips and almost nothing else, so
// answering them is narrow enough to be safe.
using GetSysColor_t = DWORD(WINAPI*)(int);
GetSysColor_t GetSysColor_Original;

DWORD WINAPI GetSysColor_Hook(int nIndex) {
    if (nIndex == COLOR_INFOTEXT && g_hasTooltipTextColor) {
        return g_tooltipTextColor;
    }
    if (nIndex == COLOR_INFOBK && g_hasTooltipBackground) {
        return g_tooltipBackground;
    }
    return GetSysColor_Original(nIndex);
}

// Kept alive for the process lifetime: callers treat system brushes as permanent and
// never delete them.
HBRUSH g_tooltipTextBrush;
HBRUSH g_tooltipBackgroundBrush;

using GetSysColorBrush_t = HBRUSH(WINAPI*)(int);
GetSysColorBrush_t GetSysColorBrush_Original;

HBRUSH WINAPI GetSysColorBrush_Hook(int nIndex) {
    if (nIndex == COLOR_INFOTEXT && g_hasTooltipTextColor) {
        if (!g_tooltipTextBrush) {
            g_tooltipTextBrush = CreateSolidBrush(g_tooltipTextColor);
        }
        if (g_tooltipTextBrush) {
            return g_tooltipTextBrush;
        }
    }

    if (nIndex == COLOR_INFOBK && g_hasTooltipBackground) {
        if (!g_tooltipBackgroundBrush) {
            g_tooltipBackgroundBrush = CreateSolidBrush(g_tooltipBackground);
        }
        if (g_tooltipBackgroundBrush) {
            return g_tooltipBackgroundBrush;
        }
    }

    return GetSysColorBrush_Original(nIndex);
}

static void ResetTooltipBrushes() {
    if (g_tooltipTextBrush) {
        DeleteObject(g_tooltipTextBrush);
        g_tooltipTextBrush = nullptr;
    }
    if (g_tooltipBackgroundBrush) {
        DeleteObject(g_tooltipBackgroundBrush);
        g_tooltipBackgroundBrush = nullptr;
    }
}

// --- menu separators ------------------------------------------------------
//
// A classic menu separator is an etched edge, drawn by user32 as
// DrawEdge(EDGE_ETCHED, BF_TOP), which takes its two lines from COLOR_3DSHADOW and
// COLOR_3DHILIGHT. Setting those system colors works, but they are global: every other
// etched edge on the desktop moves with them, group box frames and dialog borders
// included. The draw is intercepted here instead, gated on the device context belonging
// to a menu, so only menus are repainted.

static bool IsMenuWindow(HWND hWnd) {
    WCHAR className[16];
    // #32768 is the popup menu class, and nothing else is registered under it.
    return hWnd && GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
           wcscmp(className, L"#32768") == 0;
}

static BOOL CALLBACK FindVisibleMenuProc(HWND hWnd, LPARAM lParam) {
    if (IsWindowVisible(hWnd) && IsMenuWindow(hWnd)) {
        *reinterpret_cast<HWND*>(lParam) = hWnd;
        return FALSE;
    }
    return TRUE;
}

// A menu painted into a memory buffer has no window behind its device context, so the
// visible menus on this thread are checked instead. A submenu means more than one is up,
// and any of them is enough: the question is only whether a menu is on screen at all.
static bool IsMenuPaint(HDC hdc) {
    HWND hWnd = WindowFromDC(hdc);
    if (hWnd) {
        return IsMenuWindow(hWnd);
    }

    HWND found = nullptr;
    EnumThreadWindows(GetCurrentThreadId(), FindVisibleMenuProc,
                      reinterpret_cast<LPARAM>(&found));
    return found != nullptr;
}

static int SeparatorThickness() {
    if (g_separatorThickness < 1) {
        return 1;
    }
    return g_separatorThickness > 32 ? 32 : g_separatorThickness;
}

using DrawEdge_t = BOOL(WINAPI*)(HDC, LPRECT, UINT, UINT);
DrawEdge_t DrawEdge_Original;

BOOL WINAPI DrawEdge_Hook(HDC hdc, LPRECT qrc, UINT edge, UINT grfFlags) {
    // Logged before anything is decided, so a silent hook can be told apart from one
    // that fires and then declines the draw.
    if (g_logging) {
        HWND owner = WindowFromDC(hdc);
        WCHAR className[64] = L"(memory dc)";
        if (owner) {
            GetClassNameW(owner, className, ARRAYSIZE(className));
        }
        Wh_Log(L"DrawEdge edge 0x%04X flags 0x%04X on %s", edge, grfFlags, className);
    }

    // Only an etched top edge is a separator. Every other edge, in a menu or anywhere
    // else, goes through untouched.
    if (!g_hasSeparator || !qrc || edge != EDGE_ETCHED || !(grfFlags & BF_TOP) ||
        !IsMenuPaint(hdc)) {
        return DrawEdge_Original(hdc, qrc, edge, grfFlags);
    }

    int thickness = SeparatorThickness();
    RECT line = {qrc->left, qrc->top, qrc->right, qrc->top + thickness};

    // Drawn in the order of the edge it replaces: shadow above, main line below it.
    if (g_hasSeparatorShadow) {
        SolidBlender shadowBlender(hdc, g_separatorShadow, g_separatorShadowAlpha);
        if (shadowBlender.Ready()) {
            shadowBlender.Fill(line);
        }
        OffsetRect(&line, 0, thickness);
    }

    SolidBlender lineBlender(hdc, g_separator, g_separatorAlpha);
    if (lineBlender.Ready()) {
        lineBlender.Fill(line);
    }

    if (g_logging) {
        Wh_Log(L"menu separator %d,%d-%d,%d", qrc->left, qrc->top, qrc->right, line.bottom);
    }

    // BF_ADJUST asks for the rect back shrunk by what was drawn. Menus do not ask for
    // it, but a hook standing in for every caller should still honour it.
    if (grfFlags & BF_ADJUST) {
        qrc->top = line.bottom;
    }

    return TRUE;
}

// --- Chrome menus ------------------------------------------------------------
//
// Chrome paints its menus itself, with Skia, so none of the text hooks above
// see them. Instead, while a menu item paints, the light colors it asks Chrome's
// color provider for (its label, shortcut text, submenu arrow and checkmark)
// take the text color; dark ones, like its background, are left alone. The two
// functions are found through Chrome's public symbols, resolved on a thread of
// their own so Chrome never waits for them, and cached by Windhawk after that.

#ifdef _WIN64
constexpr WCHAR kChromeSymbolServer[] =
    L"https://chromium-browser-symsrv.commondatastorage.googleapis.com";

bool g_isChromeBrowser = false;
std::atomic<bool> g_chromeInitDone{false};
std::atomic<bool> g_chromeSetupStarted{false};
std::atomic<bool> g_chromeUnloading{false};
HANDLE g_chromeSetupThread = nullptr;
thread_local int g_chromeMenuPaintDepth = 0;

using MenuItemViewOnPaint_t = void (*)(void* self, void* canvas);
MenuItemViewOnPaint_t MenuItemViewOnPaint_Original;
void MenuItemViewOnPaint_Hook(void* self, void* canvas) {
    g_chromeMenuPaintDepth++;
    MenuItemViewOnPaint_Original(self, canvas);
    g_chromeMenuPaintDepth--;
}

using ColorProviderGetColor_t = uint32_t (*)(const void* self, int colorId);
ColorProviderGetColor_t ColorProviderGetColor_Original;
uint32_t ColorProviderGetColor_Hook(const void* self, int colorId) {
    uint32_t color = ColorProviderGetColor_Original(self, colorId);
    if (g_chromeMenuPaintDepth <= 0 || !g_recolorChromeMenus || g_chromeUnloading) {
        return color;
    }
    // SkColor is 0xAARRGGBB. Text is light: a dark request is a background or
    // a border, so it stays whatever the White threshold says.
    BYTE r = (color >> 16) & 0xFF, g = (color >> 8) & 0xFF, b = color & 0xFF;
    if (std::max({r, g, b}) < 128 || !IsWhitish(RGB(r, g, b))) {
        return color;
    }
    return (color & 0xFF000000u) | ((uint32_t)GetRValue(g_textColor) << 16) |
           ((uint32_t)GetGValue(g_textColor) << 8) | GetBValue(g_textColor);
}

// A menu's corner radius, for Chrome's own frame and for the outline other
// mods draw around it: zero while square menus are on.
using MenuContainerGetCornerRadius_t = int (*)(const void* self);
MenuContainerGetCornerRadius_t MenuContainerGetCornerRadius_Original;
int MenuContainerGetCornerRadius_Hook(const void* self) {
    if (g_squareChromeMenus && !g_chromeUnloading) {
        return 0;
    }
    return MenuContainerGetCornerRadius_Original(self);
}

using CornerRadiusForMenu_t = int (*)(const void* self, const void* controller);
CornerRadiusForMenu_t CornerRadiusForMenu_Original;
int CornerRadiusForMenu_Hook(const void* self, const void* controller) {
    if (g_squareChromeMenus && !g_chromeUnloading) {
        return 0;
    }
    return CornerRadiusForMenu_Original(self, controller);
}

static DWORD WINAPI ChromeMenuSetupThread(void* parameter) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(?OnPaint@MenuItemView@views@@UEAAXPEAVCanvas@gfx@@@Z)"},
         &MenuItemViewOnPaint_Original, MenuItemViewOnPaint_Hook},
        {{LR"(?GetColor@ColorProvider@ui@@QEBAIH@Z)"},
         &ColorProviderGetColor_Original, ColorProviderGetColor_Hook},
        // Optional: without them the menus just keep Chrome's rounded corners.
        {{LR"(?GetCornerRadius@MenuScrollViewContainer@views@@AEBAHXZ)"},
         &MenuContainerGetCornerRadius_Original, MenuContainerGetCornerRadius_Hook, true},
        {{LR"(?CornerRadiusForMenu@MenuConfig@views@@QEBAHPEBVMenuController@2@@Z)"},
         &CornerRadiusForMenu_Original, CornerRadiusForMenu_Hook, true},
    };
    WH_HOOK_SYMBOLS_OPTIONS options = {};
    options.optionsSize = sizeof(options);
    options.symbolServer = kChromeSymbolServer;
    // Chrome's symbols run to gigabytes; the decorated names are all this needs.
    options.noUndecoratedSymbols = TRUE;
    if (!WindhawkUtils::HookSymbols((HMODULE)parameter, hooks, ARRAYSIZE(hooks), &options)) {
        Wh_Log(L"Chrome menus: Chrome's menu functions weren't found");
        return 0;
    }
    if (!g_chromeUnloading) {
        Wh_ApplyHookOperations();
        Wh_Log(L"Chrome menus: ready");
    }
    return 0;
}

static void StartChromeMenuSetup(HMODULE chromeDll) {
    if (g_chromeUnloading || g_chromeSetupStarted.exchange(true)) {
        return;
    }
    g_chromeSetupThread = CreateThread(nullptr, 0, ChromeMenuSetupThread, chromeDll, 0, nullptr);
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR fileName, HANDLE file, DWORD flags) {
    HMODULE module = LoadLibraryExW_Original(fileName, file, flags);
    if (module && g_chromeInitDone && !g_chromeSetupStarted &&
        module == GetModuleHandleW(L"chrome.dll")) {
        StartChromeMenuSetup(module);
    }
    return module;
}

// Tabs, the GPU and Chrome's other helpers run as "chrome.exe --type=...";
// only the browser process shows menus.
static bool IsChromeBrowserProcess() {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return false;
    }
    PCWSTR name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    return _wcsicmp(name, L"chrome.exe") == 0 && !wcsstr(GetCommandLineW(), L"--type=");
}

static void InitChromeMenus() {
    g_isChromeBrowser = IsChromeBrowserProcess();
    if (!g_isChromeBrowser || GetModuleHandleW(L"chrome.dll")) {
        return;  // Already loaded: the after-init step starts the setup.
    }
    HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
    auto loadLibraryExW =
        kernelBase ? (LoadLibraryExW_t)GetProcAddress(kernelBase, "LoadLibraryExW") : nullptr;
    if (loadLibraryExW) {
        WindhawkUtils::SetFunctionHook(loadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }
}

static void AfterInitChromeMenus() {
    g_chromeInitDone = true;
    if (g_isChromeBrowser) {
        if (HMODULE chromeDll = GetModuleHandleW(L"chrome.dll")) {
            StartChromeMenuSetup(chromeDll);
        }
    }
}

static void UninitChromeMenus() {
    g_chromeUnloading = true;
    // The setup thread must be done with this module's code before it unloads.
    if (g_chromeSetupThread) {
        WaitForSingleObject(g_chromeSetupThread, INFINITE);
        CloseHandle(g_chromeSetupThread);
        g_chromeSetupThread = nullptr;
    }
}
#endif  // _WIN64
// --- settings and lifecycle ----------------------------------------------

static void LoadSettings() {
    PCWSTR textColor = Wh_GetStringSetting(L"textColor");
    COLORREF parsed;
    if (ParseColor(textColor, &parsed)) {
        g_textColor = parsed;
    } else {
        Wh_Log(L"text color: unreadable value, keeping 0x%06X", (unsigned int)g_textColor);
    }
    Wh_FreeStringSetting(textColor);

    g_threshold = Wh_GetIntSetting(L"threshold");
    if (g_threshold < 0) {
        g_threshold = 0;
    } else if (g_threshold > 255) {
        g_threshold = 255;
    }

    PCWSTR tooltipBackground = Wh_GetStringSetting(L"tooltipBackground");
    g_hasTooltipBackground =
        ParseColor(tooltipBackground, &g_tooltipBackground, &g_tooltipBackgroundAlpha);
    Wh_FreeStringSetting(tooltipBackground);

    PCWSTR tooltipTextColor = Wh_GetStringSetting(L"tooltipTextColor");
    g_hasTooltipTextColor = ParseColor(tooltipTextColor, &g_tooltipTextColor);
    Wh_FreeStringSetting(tooltipTextColor);

    PCWSTR tooltipBorder = Wh_GetStringSetting(L"tooltipBorderColor");
    g_hasTooltipBorder = ParseColor(tooltipBorder, &g_tooltipBorder, &g_tooltipBorderAlpha);
    Wh_FreeStringSetting(tooltipBorder);

    g_tooltipBorderThickness = Wh_GetIntSetting(L"tooltipBorderThickness");

    PCWSTR separator = Wh_GetStringSetting(L"separatorColor");
    g_hasSeparator = ParseColor(separator, &g_separator, &g_separatorAlpha);
    Wh_FreeStringSetting(separator);

    PCWSTR separatorShadow = Wh_GetStringSetting(L"separatorShadowColor");
    g_hasSeparatorShadow =
        ParseColor(separatorShadow, &g_separatorShadow, &g_separatorShadowAlpha);
    Wh_FreeStringSetting(separatorShadow);

    g_separatorThickness = Wh_GetIntSetting(L"separatorThickness");

    ResetTooltipBrushes();
    g_tooltipKeepBorder = Wh_GetIntSetting(L"tooltipKeepBorder") != 0;
    g_recolorGdi = Wh_GetIntSetting(L"recolorGdi") != 0;
    g_recolorThemed = Wh_GetIntSetting(L"recolorThemed") != 0;
    g_recolorChromeMenus = Wh_GetIntSetting(L"recolorChromeMenus") != 0;
    g_squareChromeMenus = Wh_GetIntSetting(L"squareChromeMenus") != 0;
    g_logging = Wh_GetIntSetting(L"logging") != 0;
    g_logText = Wh_GetIntSetting(L"logText") != 0;
    g_lastLogged = CLR_INVALID;
}

// Compares one comma separated entry against this process, ignoring surrounding space.
static bool EntryMatchesProcess(PCWSTR begin, PCWSTR end, PCWSTR processName) {
    while (begin < end && (*begin == L' ' || *begin == L'\t')) {
        begin++;
    }
    while (end > begin && (end[-1] == L' ' || end[-1] == L'\t')) {
        end--;
    }

    if (begin == end) {
        return false;
    }
    if (end - begin == 1 && *begin == L'*') {
        return true;
    }

    size_t length = (size_t)(end - begin);
    return wcslen(processName) == length && _wcsnicmp(begin, processName, length) == 0;
}

static bool ShouldLoadInThisProcess() {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return false;
    }

    PCWSTR processName = wcsrchr(path, L'\\');
    processName = processName ? processName + 1 : path;

    PCWSTR processes = Wh_GetStringSetting(L"processes");
    bool matched = false;

    PCWSTR entry = processes;
    for (PCWSTR p = processes;; p++) {
        if (*p == L',' || !*p) {
            if (EntryMatchesProcess(entry, p, processName)) {
                matched = true;
                break;
            }
            if (!*p) {
                break;
            }
            entry = p + 1;
        }
    }

    Wh_FreeStringSetting(processes);

    if (!matched) {
        Wh_Log(L"%s is not in the process list, not loading", processName);
    }

    return matched;
}

static bool HookExport(HMODULE module, const char* name, void* hook, void** original) {
    void* target = (void*)GetProcAddress(module, name);
    if (!target) {
        Wh_Log(L"missing export %S", name);
        return false;
    }
    return Wh_SetFunctionHook(target, hook, original);
}

BOOL Wh_ModInit() {
    if (!ShouldLoadInThisProcess()) {
        return FALSE;
    }

    g_themeClasses = new std::unordered_map<HTHEME, std::wstring>();

    LoadSettings();

    HMODULE gdi32 = GetModuleHandleW(L"gdi32.dll");
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    HMODULE uxtheme = LoadLibraryW(L"uxtheme.dll");
    if (!gdi32 || !user32 || !uxtheme) {
        Wh_Log(L"failed to resolve a required module");
        return FALSE;
    }

    HookExport(user32, "CreateWindowExW", (void*)CreateWindowExW_Hook,
               (void**)&CreateWindowExW_Original);
    HookExport(user32, "GetSysColor", (void*)GetSysColor_Hook, (void**)&GetSysColor_Original);
    HookExport(user32, "GetSysColorBrush", (void*)GetSysColorBrush_Hook,
               (void**)&GetSysColorBrush_Original);
    HookExport(user32, "DrawEdge", (void*)DrawEdge_Hook, (void**)&DrawEdge_Original);

    HookExport(uxtheme, "OpenThemeData", (void*)OpenThemeData_Hook,
               (void**)&OpenThemeData_Original);
    HookExport(uxtheme, "OpenThemeDataForDpi", (void*)OpenThemeDataForDpi_Hook,
               (void**)&OpenThemeDataForDpi_Original);
    HookExport(uxtheme, "OpenThemeDataEx", (void*)OpenThemeDataEx_Hook,
               (void**)&OpenThemeDataEx_Original);
    HookExport(uxtheme, "CloseThemeData", (void*)CloseThemeData_Hook,
               (void**)&CloseThemeData_Original);
    HookExport(uxtheme, "GetThemeColor", (void*)GetThemeColor_Hook,
               (void**)&GetThemeColor_Original);
    HookExport(uxtheme, "GetThemeFont", (void*)GetThemeFont_Hook, (void**)&GetThemeFont_Original);
    HookExport(uxtheme, "GetThemeMargins", (void*)GetThemeMargins_Hook,
               (void**)&GetThemeMargins_Original);
    HookExport(uxtheme, "GetThemePartSize", (void*)GetThemePartSize_Hook,
               (void**)&GetThemePartSize_Original);
    HookExport(uxtheme, "DrawThemeBackground", (void*)DrawThemeBackground_Hook,
               (void**)&DrawThemeBackground_Original);
    HookExport(uxtheme, "DrawThemeBackgroundEx", (void*)DrawThemeBackgroundEx_Hook,
               (void**)&DrawThemeBackgroundEx_Original);

    // Tooltips are long lived, so the ones already up would never reach the hook above.
    EnumWindows(TrackExistingTooltip, 0);

#ifdef _WIN64
    InitChromeMenus();
#endif

    HookExport(gdi32, "SetTextColor", (void*)SetTextColor_Hook, (void**)&SetTextColor_Original);
    HookExport(gdi32, "ExtTextOutW", (void*)ExtTextOutW_Hook, (void**)&ExtTextOutW_Original);
    HookExport(gdi32, "TextOutW", (void*)TextOutW_Hook, (void**)&TextOutW_Original);
    HookExport(uxtheme, "DrawThemeTextEx", (void*)DrawThemeTextEx_Hook,
               (void**)&DrawThemeTextEx_Original);
    HookExport(uxtheme, "DrawThemeText", (void*)DrawThemeText_Hook,
               (void**)&DrawThemeText_Original);

    return TRUE;
}

void Wh_ModAfterInit() {
#ifdef _WIN64
    AfterInitChromeMenus();
#endif
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}

void Wh_ModUninit() {
#ifdef _WIN64
    UninitChromeMenus();
#endif
    WindhawkUtils::RemoveAllWindowSubclasses();
    ResetTooltipBrushes();

    // Leaked deliberately: a hook still running on another thread may reach for it while
    // the mod unloads, and the process is on its way out anyway.
    g_themeClasses = nullptr;
}
