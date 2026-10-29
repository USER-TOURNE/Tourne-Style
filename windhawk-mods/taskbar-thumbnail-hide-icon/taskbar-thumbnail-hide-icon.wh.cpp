// ==WindhawkMod==
// @id                  taskbar-thumbnail-hide-icon
// @name                Hide the app icon in taskbar thumbnail previews
// @description         Removes the small app icon from the header of the classic taskbar hover preview, keeping the window title, and recolors the preview border
// @version             1.4.0
// @author              USER-TOURNE
// @github              https://github.com/USER-TOURNE
// @donateUrl           https://ko-fi.com/tourne
// @license             GPL-3.0
// @include             explorer.exe
// @architecture        x86-64
// @compilerOptions     -lcomctl32 -ldwmapi -lgdi32 -luser32 -luxtheme
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Hide the app icon in taskbar thumbnail previews

Targets the classic thumbnail window (`TaskListThumbnailWnd`), which is what you get
with StartAllBack's enhanced classic taskbar or ExplorerPatcher's Windows 10 taskbar.
It is not the XAML thumbnail of the stock Windows 11 taskbar.

The mod subclasses that window and raises a flag while it paints. Anything drawn inside
that flag that looks like the header icon gets dropped:

- icon APIs (`DrawIconEx`, `DrawIcon`, `DrawIconIndirect`, image lists, `DrawThemeIcon`)
  are dropped unconditionally, and
- blits (`BitBlt`, `StretchBlt`, `AlphaBlend`, `TransparentBlt`, `MaskBlt`) are dropped
  only when the destination is a small rectangle inside the icon zone, so the header
  background and the preview itself are left alone.

## Tuning

The icon zone defaults suit a header whose title starts near x=53. If your title sits
elsewhere, turn on **Verbose logging**, hover a taskbar button, and read the logged
destination rectangles: the icon is the small square just left of the title. Set
**Icon zone right/bottom** just past it.

**Title indent:** removing the icon leaves the title where it was. Use **Shift title
left** to pull it back over the freed space.

## Border color

The outline around the preview is painted by the visual style, as part 1 of the theme
background that fills the card. **Border color** repaints it in place, right after the
theme has drawn it, so it works whatever msstyles is loaded.

Accepted values, with or without the leading hash: `AARRGGBB`, `RRGGBB`, `default` to
leave the theme's own outline alone, and `none` to erase it. Alpha is honored: the
border is alpha-blended over the card, so `80` gives a half-strength outline and `00`
is the same as `none`. Use **Border thickness** for anything heavier than a hairline.

Erasing works by copying the pixels just inside the outline over it, so it follows
whatever gradient or texture the visual style put behind it.

The DWM window border is set as well, for setups where the preview has one. On
StartAllBack it does nothing, and the log says so with `hr 0x00000000` and no visible
change.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- logging: false
  $name: Verbose logging
  $description: Log draw calls made while the thumbnail window paints
- suppressSmallDraws: true
  $name: Drop small blits in the icon zone
  $description: Turn off to log only, which is useful while tuning the zone
- iconZoneRight: 52
  $name: Icon zone right edge (pixels)
- iconZoneBottom: 64
  $name: Icon zone bottom edge (pixels)
- iconMaxSize: 48
  $name: Icon zone max draw size (pixels)
  $description: Draws larger than this in either direction are left alone
- shiftTitleLeft: 0
  $name: Shift title left (pixels)
  $description: Move the header title left to close the gap the icon left behind. 0 disables.
- borderColor: default
  $name: Border color
  $description: 'Outline around the preview. Format is #AARRGGBB or #RRGGBB, plus the keywords default to keep the visual style''s own outline and none to erase it. Alpha is blended, so 00 is the same as none.'
- borderThickness: 1
  $name: Border thickness (pixels)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>

// Windows 11 22000+ attributes, spelled out because the toolchain headers predate them.
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif
#ifndef DWMWA_COLOR_DEFAULT
#define DWMWA_COLOR_DEFAULT 0xFFFFFFFF
#endif
#ifndef DWMWA_COLOR_NONE
#define DWMWA_COLOR_NONE 0xFFFFFFFE
#endif

constexpr WCHAR kThumbnailClass[] = L"TaskListThumbnailWnd";
constexpr UINT_PTR kSubclassId = 'thmb';

// Non-zero only on the taskbar UI thread, and only while the thumbnail window paints.
thread_local int g_paintDepth = 0;

bool g_logging = false;
bool g_suppressSmallDraws = true;
int g_iconZoneRight = 52;
int g_iconZoneBottom = 64;
int g_iconMaxSize = 48;
int g_shiftTitleLeft = 0;

enum class BorderMode {
    Theme,  // leave whatever the visual style drew
    Erase,  // cover it with the pixels just inside it
    Paint,  // alpha-blend our own color over it
};

BorderMode g_borderMode = BorderMode::Theme;
BYTE g_borderAlpha = 0xFF;
COLORREF g_borderRgb = 0;
DWORD g_borderDwmColor = DWMWA_COLOR_DEFAULT;
int g_borderThickness = 1;

// Accepts #AARRGGBB, #RRGGBB, the same spellings without the hash, and the keywords
// default and none. Alpha 00 is the same as none.
static void ParseBorderColor(PCWSTR value) {
    g_borderMode = BorderMode::Theme;
    g_borderAlpha = 0xFF;
    g_borderRgb = 0;
    g_borderDwmColor = DWMWA_COLOR_DEFAULT;

    while (*value == L' ' || *value == L'\t') {
        value++;
    }

    if (!*value || _wcsicmp(value, L"default") == 0) {
        return;
    }
    if (_wcsicmp(value, L"none") == 0) {
        g_borderMode = BorderMode::Erase;
        g_borderDwmColor = DWMWA_COLOR_NONE;
        return;
    }

    if (*value == L'#') {
        value++;
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
            Wh_Log(L"border color: unexpected character, keeping the theme border");
            return;
        }

        packed = (packed << 4) | digit;
        if (++digits > 8) {
            Wh_Log(L"border color: too many digits, keeping the theme border");
            return;
        }
    }

    BYTE alpha = 0xFF;
    if (digits == 8) {
        alpha = (BYTE)(packed >> 24);
    } else if (digits != 6) {
        Wh_Log(L"border color: expected 6 or 8 digits, keeping the theme border");
        return;
    }

    if (alpha == 0) {
        g_borderMode = BorderMode::Erase;
        g_borderDwmColor = DWMWA_COLOR_NONE;
        return;
    }

    g_borderMode = BorderMode::Paint;
    g_borderAlpha = alpha;
    g_borderRgb = RGB((packed >> 16) & 0xFF, (packed >> 8) & 0xFF, packed & 0xFF);
    g_borderDwmColor = g_borderRgb;
}

static void LoadSettings() {
    g_logging = Wh_GetIntSetting(L"logging") != 0;
    g_suppressSmallDraws = Wh_GetIntSetting(L"suppressSmallDraws") != 0;
    g_iconZoneRight = Wh_GetIntSetting(L"iconZoneRight");
    g_iconZoneBottom = Wh_GetIntSetting(L"iconZoneBottom");
    g_iconMaxSize = Wh_GetIntSetting(L"iconMaxSize");
    g_shiftTitleLeft = Wh_GetIntSetting(L"shiftTitleLeft");
    g_borderThickness = Wh_GetIntSetting(L"borderThickness");

    PCWSTR borderColor = Wh_GetStringSetting(L"borderColor");
    ParseBorderColor(borderColor);
    Wh_FreeStringSetting(borderColor);
}

using DwmSetWindowAttribute_t = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
DwmSetWindowAttribute_t DwmSetWindowAttribute_Original;

// Set while we are the ones calling, so the hook below leaves our own call alone.
thread_local bool g_applyingBorderColor = false;

static void ApplyBorderColor(HWND hWnd, bool log) {
    if (!DwmSetWindowAttribute_Original) {
        return;
    }

    DWORD color = g_borderDwmColor;

    g_applyingBorderColor = true;
    HRESULT hr = DwmSetWindowAttribute_Original(hWnd, DWMWA_BORDER_COLOR, &color, sizeof(color));
    g_applyingBorderColor = false;

    if (log && g_logging) {
        Wh_Log(L"border color 0x%08X on %p, hr 0x%08X", color, hWnd, hr);
    }
}

// --- context detection ----------------------------------------------------

static bool IsThumbnailWindow(HWND hWnd) {
    WCHAR className[64];
    return hWnd && GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
           _wcsicmp(className, kThumbnailClass) == 0;
}

// StartAllBack themes the preview frame, so it may set its own border color right after
// the window is created. Ours wins.
HRESULT WINAPI DwmSetWindowAttribute_Hook(HWND hWnd, DWORD dwAttribute, LPCVOID pvAttribute,
                                          DWORD cbAttribute) {
    if (!g_applyingBorderColor && dwAttribute == DWMWA_BORDER_COLOR &&
        g_borderMode != BorderMode::Theme && IsThumbnailWindow(hWnd)) {
        if (g_logging) {
            DWORD incoming = (pvAttribute && cbAttribute == sizeof(DWORD))
                                 ? *reinterpret_cast<const DWORD*>(pvAttribute)
                                 : 0;
            Wh_Log(L"overrode foreign border color 0x%08X on %p", incoming, hWnd);
        }

        DWORD color = g_borderDwmColor;
        return DwmSetWindowAttribute_Original(hWnd, DWMWA_BORDER_COLOR, &color, sizeof(color));
    }

    return DwmSetWindowAttribute_Original(hWnd, dwAttribute, pvAttribute, cbAttribute);
}

// The paint flag covers the common case. The DC lookup is the fallback for drawing that
// happens outside the subclassed paint, and it only runs for draws already small enough
// to be the icon, so it stays off explorer's hot blit paths.
static bool InThumbnailContext(HDC hdc) {
    if (g_paintDepth > 0) {
        return true;
    }

    HWND hWnd = WindowFromDC(hdc);
    if (!hWnd) {
        return false;
    }

    return IsThumbnailWindow(hWnd) || IsThumbnailWindow(GetAncestor(hWnd, GA_ROOT));
}

static bool IsIconZoneDraw(int x, int y, int cx, int cy) {
    if (cx <= 0 || cy <= 0 || cx > g_iconMaxSize || cy > g_iconMaxSize) {
        return false;
    }
    return x + cx <= g_iconZoneRight && y + cy <= g_iconZoneBottom;
}

// Returns true when the caller should skip the draw entirely.
static bool ShouldDropBlit(const WCHAR* api, HDC hdcDst, int x, int y, int cx, int cy) {
    if (!IsIconZoneDraw(x, y, cx, cy) || !InThumbnailContext(hdcDst)) {
        return false;
    }

    if (g_logging) {
        Wh_Log(L"%s in icon zone at %d,%d size %dx%d%s", api, x, y, cx, cy,
               g_suppressSmallDraws ? L" (dropped)" : L" (kept)");
    }

    return g_suppressSmallDraws;
}

// --- icon APIs ------------------------------------------------------------

using DrawIconEx_t = BOOL(WINAPI*)(HDC, int, int, HICON, int, int, UINT, HBRUSH, UINT);
DrawIconEx_t DrawIconEx_Original;

BOOL WINAPI DrawIconEx_Hook(HDC hdc, int x, int y, HICON hIcon, int cx, int cy, UINT istep,
                            HBRUSH hbr, UINT flags) {
    if (InThumbnailContext(hdc)) {
        if (g_logging) {
            Wh_Log(L"dropped DrawIconEx at %d,%d size %dx%d", x, y, cx, cy);
        }
        return TRUE;
    }
    return DrawIconEx_Original(hdc, x, y, hIcon, cx, cy, istep, hbr, flags);
}

using DrawIcon_t = BOOL(WINAPI*)(HDC, int, int, HICON);
DrawIcon_t DrawIcon_Original;

BOOL WINAPI DrawIcon_Hook(HDC hdc, int x, int y, HICON hIcon) {
    if (InThumbnailContext(hdc)) {
        if (g_logging) {
            Wh_Log(L"dropped DrawIcon at %d,%d", x, y);
        }
        return TRUE;
    }
    return DrawIcon_Original(hdc, x, y, hIcon);
}

using DrawIconIndirect_t = BOOL(WINAPI*)(HDC, const DRAWITEMSTRUCT*);
using DrawStateW_t = BOOL(WINAPI*)(HDC, HBRUSH, DRAWSTATEPROC, LPARAM, WPARAM, int, int, int,
                                   int, UINT);
DrawStateW_t DrawStateW_Original;

BOOL WINAPI DrawStateW_Hook(HDC hdc, HBRUSH hbrFore, DRAWSTATEPROC qfnCallBack, LPARAM lData,
                            WPARAM wData, int x, int y, int cx, int cy, UINT uFlags) {
    if ((uFlags & DST_ICON) && InThumbnailContext(hdc)) {
        if (g_logging) {
            Wh_Log(L"dropped DrawStateW icon at %d,%d size %dx%d", x, y, cx, cy);
        }
        return TRUE;
    }
    return DrawStateW_Original(hdc, hbrFore, qfnCallBack, lData, wData, x, y, cx, cy, uFlags);
}

// ImageList_Draw and ImageList_DrawEx both funnel through this one.
using ImageList_DrawIndirect_t = BOOL(WINAPI*)(IMAGELISTDRAWPARAMS*);
ImageList_DrawIndirect_t ImageList_DrawIndirect_Original;

BOOL WINAPI ImageList_DrawIndirect_Hook(IMAGELISTDRAWPARAMS* pimldp) {
    if (pimldp && InThumbnailContext(pimldp->hdcDst)) {
        if (g_logging) {
            Wh_Log(L"dropped ImageList_DrawIndirect at %d,%d index %d", pimldp->x, pimldp->y,
                   pimldp->i);
        }
        return TRUE;
    }
    return ImageList_DrawIndirect_Original(pimldp);
}

using DrawThemeIcon_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, HIMAGELIST, int);
DrawThemeIcon_t DrawThemeIcon_Original;

HRESULT WINAPI DrawThemeIcon_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                  LPCRECT pRect, HIMAGELIST himl, int iImageIndex) {
    if (InThumbnailContext(hdc)) {
        if (g_logging) {
            Wh_Log(L"dropped DrawThemeIcon part %d state %d", iPartId, iStateId);
        }
        return S_OK;
    }
    return DrawThemeIcon_Original(hTheme, hdc, iPartId, iStateId, pRect, himl, iImageIndex);
}

// --- blit APIs ------------------------------------------------------------

using BitBlt_t = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, DWORD);
BitBlt_t BitBlt_Original;

BOOL WINAPI BitBlt_Hook(HDC hdcDst, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1,
                        DWORD rop) {
    if (ShouldDropBlit(L"BitBlt", hdcDst, x, y, cx, cy)) {
        return TRUE;
    }
    return BitBlt_Original(hdcDst, x, y, cx, cy, hdcSrc, x1, y1, rop);
}

using StretchBlt_t = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
StretchBlt_t StretchBlt_Original;

BOOL WINAPI StretchBlt_Hook(HDC hdcDst, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1,
                            int cx1, int cy1, DWORD rop) {
    if (ShouldDropBlit(L"StretchBlt", hdcDst, x, y, cx, cy)) {
        return TRUE;
    }
    return StretchBlt_Original(hdcDst, x, y, cx, cy, hdcSrc, x1, y1, cx1, cy1, rop);
}

using MaskBlt_t = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, HBITMAP, int, int, DWORD);
MaskBlt_t MaskBlt_Original;

BOOL WINAPI MaskBlt_Hook(HDC hdcDst, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1,
                         HBITMAP hbmMask, int xMask, int yMask, DWORD rop) {
    if (ShouldDropBlit(L"MaskBlt", hdcDst, x, y, cx, cy)) {
        return TRUE;
    }
    return MaskBlt_Original(hdcDst, x, y, cx, cy, hdcSrc, x1, y1, hbmMask, xMask, yMask, rop);
}

// msimg32's AlphaBlend and TransparentBlt are forwarders onto these two.
using GdiAlphaBlend_t = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, int, int,
                                      BLENDFUNCTION);
GdiAlphaBlend_t GdiAlphaBlend_Original;

BOOL WINAPI GdiAlphaBlend_Hook(HDC hdcDst, int x, int y, int cx, int cy, HDC hdcSrc, int x1,
                               int y1, int cx1, int cy1, BLENDFUNCTION bf) {
    if (ShouldDropBlit(L"AlphaBlend", hdcDst, x, y, cx, cy)) {
        return TRUE;
    }
    return GdiAlphaBlend_Original(hdcDst, x, y, cx, cy, hdcSrc, x1, y1, cx1, cy1, bf);
}

using GdiTransparentBlt_t = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
GdiTransparentBlt_t GdiTransparentBlt_Original;

BOOL WINAPI GdiTransparentBlt_Hook(HDC hdcDst, int x, int y, int cx, int cy, HDC hdcSrc, int x1,
                                   int y1, int cx1, int cy1, UINT crTransparent) {
    if (ShouldDropBlit(L"TransparentBlt", hdcDst, x, y, cx, cy)) {
        return TRUE;
    }
    return GdiTransparentBlt_Original(hdcDst, x, y, cx, cy, hdcSrc, x1, y1, cx1, cy1,
                                      crTransparent);
}

// --- frame diagnostics ----------------------------------------------------
//
// Logging only. If the outline is not the DWM border, it has to be one of these, and the
// log tells us which one and at what rectangle.

static void LogFrameDraw(const WCHAR* api, const RECT* rect) {
    if (g_paintDepth > 0 && g_logging && rect) {
        Wh_Log(L"%s during paint, rect %d,%d-%d,%d (%dx%d)", api, rect->left, rect->top,
               rect->right, rect->bottom, rect->right - rect->left, rect->bottom - rect->top);
    }
}

using FillRect_t = int(WINAPI*)(HDC, const RECT*, HBRUSH);
FillRect_t FillRect_Original;

int WINAPI FillRect_Hook(HDC hdc, const RECT* lprc, HBRUSH hbr) {
    LogFrameDraw(L"FillRect", lprc);
    return FillRect_Original(hdc, lprc, hbr);
}

using FrameRect_t = int(WINAPI*)(HDC, const RECT*, HBRUSH);
FrameRect_t FrameRect_Original;

int WINAPI FrameRect_Hook(HDC hdc, const RECT* lprc, HBRUSH hbr) {
    LogFrameDraw(L"FrameRect", lprc);
    return FrameRect_Original(hdc, lprc, hbr);
}

using PatBlt_t = BOOL(WINAPI*)(HDC, int, int, int, int, DWORD);
PatBlt_t PatBlt_Original;

BOOL WINAPI PatBlt_Hook(HDC hdc, int x, int y, int cx, int cy, DWORD rop) {
    RECT rect = {x, y, x + cx, y + cy};
    LogFrameDraw(L"PatBlt", &rect);
    return PatBlt_Original(hdc, x, y, cx, cy, rop);
}

using Rectangle_t = BOOL(WINAPI*)(HDC, int, int, int, int);
Rectangle_t Rectangle_Original;

BOOL WINAPI Rectangle_Hook(HDC hdc, int left, int top, int right, int bottom) {
    RECT rect = {left, top, right, bottom};
    LogFrameDraw(L"Rectangle", &rect);
    return Rectangle_Original(hdc, left, top, right, bottom);
}

using RoundRect_t = BOOL(WINAPI*)(HDC, int, int, int, int, int, int);
RoundRect_t RoundRect_Original;

BOOL WINAPI RoundRect_Hook(HDC hdc, int left, int top, int right, int bottom, int width,
                           int height) {
    RECT rect = {left, top, right, bottom};
    LogFrameDraw(L"RoundRect", &rect);
    return RoundRect_Original(hdc, left, top, right, bottom, width, height);
}

// --- preview border -------------------------------------------------------
//
// The outline belongs to the visual style: it comes with part 1 of the theme background
// that fills the card. So we let the theme draw, then repaint the outline in place.

constexpr int kCardPartId = 1;

static int BorderThickness() {
    if (g_borderThickness < 1) {
        return 1;
    }
    return g_borderThickness > 32 ? 32 : g_borderThickness;
}

// Alpha-blends the border color over the outline the theme just drew. The source is a
// single premultiplied pixel stretched along each edge, which keeps partial alpha
// correct on a per-pixel-alpha surface as well as an opaque one.
static void PaintBorder(HDC hdc, const RECT& rect, int thickness) {
    if (!GdiAlphaBlend_Original) {
        return;
    }

    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = 1;
    info.bmiHeader.biHeight = -1;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap) {
        return;
    }

    HDC sourceDc = CreateCompatibleDC(hdc);
    if (!sourceDc) {
        DeleteObject(bitmap);
        return;
    }

    BYTE alpha = g_borderAlpha;
    BYTE red = (BYTE)(GetRValue(g_borderRgb) * alpha / 255);
    BYTE green = (BYTE)(GetGValue(g_borderRgb) * alpha / 255);
    BYTE blue = (BYTE)(GetBValue(g_borderRgb) * alpha / 255);
    *reinterpret_cast<DWORD*>(bits) =
        ((DWORD)alpha << 24) | ((DWORD)red << 16) | ((DWORD)green << 8) | blue;

    HGDIOBJ previous = SelectObject(sourceDc, bitmap);

    const RECT edges[] = {
        {rect.left, rect.top, rect.right, rect.top + thickness},
        {rect.left, rect.bottom - thickness, rect.right, rect.bottom},
        {rect.left, rect.top + thickness, rect.left + thickness, rect.bottom - thickness},
        {rect.right - thickness, rect.top + thickness, rect.right, rect.bottom - thickness},
    };

    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    for (const RECT& edge : edges) {
        GdiAlphaBlend_Original(hdc, edge.left, edge.top, edge.right - edge.left,
                               edge.bottom - edge.top, sourceDc, 0, 0, 1, 1, blend);
    }

    SelectObject(sourceDc, previous);
    DeleteDC(sourceDc);
    DeleteObject(bitmap);
}

// Covers the outline with the pixels just inside it, so whatever gradient or texture the
// visual style painted behind the outline carries through.
static void EraseBorder(HDC hdc, const RECT& rect, int thickness) {
    if (!BitBlt_Original) {
        return;
    }

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    BitBlt_Original(hdc, rect.left, rect.top, width, thickness, hdc, rect.left,
                    rect.top + thickness, SRCCOPY);
    BitBlt_Original(hdc, rect.left, rect.bottom - thickness, width, thickness, hdc, rect.left,
                    rect.bottom - thickness * 2, SRCCOPY);
    BitBlt_Original(hdc, rect.left, rect.top, thickness, height, hdc, rect.left + thickness,
                    rect.top, SRCCOPY);
    BitBlt_Original(hdc, rect.right - thickness, rect.top, thickness, height, hdc,
                    rect.right - thickness * 2, rect.top, SRCCOPY);
}

using DrawThemeBackground_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
DrawThemeBackground_t DrawThemeBackground_Original;

HRESULT WINAPI DrawThemeBackground_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                        LPCRECT pRect, LPCRECT pClipRect) {
    HRESULT hr = DrawThemeBackground_Original(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);

    if (g_paintDepth == 0 || !pRect) {
        return hr;
    }

    if (g_logging) {
        Wh_Log(L"DrawThemeBackground part %d state %d, rect %d,%d-%d,%d", iPartId, iStateId,
               pRect->left, pRect->top, pRect->right, pRect->bottom);
    }

    if (iPartId != kCardPartId || g_borderMode == BorderMode::Theme || FAILED(hr)) {
        return hr;
    }

    int thickness = BorderThickness();
    if (pRect->right - pRect->left <= thickness * 2 ||
        pRect->bottom - pRect->top <= thickness * 2) {
        return hr;
    }

    if (g_borderMode == BorderMode::Erase) {
        EraseBorder(hdc, *pRect, thickness);
    } else {
        PaintBorder(hdc, *pRect, thickness);
    }

    return hr;
}

// --- title text -----------------------------------------------------------

using DrawThemeTextEx_t = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT,
                                           const DTTOPTS*);
DrawThemeTextEx_t DrawThemeTextEx_Original;

HRESULT WINAPI DrawThemeTextEx_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
                                    LPCWSTR pszText, int cchText, DWORD dwFlags, LPRECT pRect,
                                    const DTTOPTS* pOptions) {
    if (g_paintDepth > 0) {
        if (g_logging && pRect) {
            Wh_Log(L"thumbnail text \"%.40s\" rect %d,%d-%d,%d part %d",
                   pszText ? pszText : L"", pRect->left, pRect->top, pRect->right,
                   pRect->bottom, iPartId);
        }
        if (g_shiftTitleLeft > 0 && pRect) {
            RECT shifted = *pRect;
            shifted.left -= g_shiftTitleLeft;
            if (shifted.left < 0) {
                shifted.left = 0;
            }
            return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText,
                                            dwFlags, &shifted, pOptions);
        }
    }
    return DrawThemeTextEx_Original(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwFlags,
                                    pRect, pOptions);
}

// --- thumbnail window tracking -------------------------------------------

LRESULT CALLBACK ThumbnailSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                       UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_PAINT:
        case WM_NCPAINT:
        case WM_PRINTCLIENT:
        case WM_ERASEBKGND: {
            g_paintDepth++;
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            g_paintDepth--;
            return result;
        }

        // Re-assert the color late, in case something set its own after creation.
        case WM_SHOWWINDOW:
        case WM_WINDOWPOSCHANGED:
            ApplyBorderColor(hWnd, false);
            break;

        case WM_NCDESTROY:
            RemoveWindowSubclass(hWnd, ThumbnailSubclassProc, kSubclassId);
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
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

    // Created on the taskbar UI thread, so plain SetWindowSubclass is safe here.
    if (hWnd && !IS_INTRESOURCE(lpClassName) && _wcsicmp(lpClassName, kThumbnailClass) == 0) {
        SetWindowSubclass(hWnd, ThumbnailSubclassProc, kSubclassId, 0);
        ApplyBorderColor(hWnd, true);
        if (g_logging) {
            Wh_Log(L"subclassed thumbnail window %p", hWnd);
        }
    }

    return hWnd;
}

// --- mod lifecycle --------------------------------------------------------

static bool HookExport(HMODULE module, const char* name, void* hook, void** original) {
    void* target = (void*)GetProcAddress(module, name);
    if (!target) {
        Wh_Log(L"missing export %S", name);
        return false;
    }
    return Wh_SetFunctionHook(target, hook, original);
}

BOOL Wh_ModInit() {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    HMODULE gdi32 = GetModuleHandleW(L"gdi32.dll");
    HMODULE comctl32 = LoadLibraryW(L"comctl32.dll");
    HMODULE uxtheme = LoadLibraryW(L"uxtheme.dll");
    HMODULE dwmapi = LoadLibraryW(L"dwmapi.dll");
    if (!user32 || !gdi32 || !comctl32 || !uxtheme || !dwmapi) {
        Wh_Log(L"failed to resolve a required module");
        return FALSE;
    }

    // Before LoadSettings, because the border color is applied through the original.
    HookExport(dwmapi, "DwmSetWindowAttribute", (void*)DwmSetWindowAttribute_Hook,
               (void**)&DwmSetWindowAttribute_Original);

    LoadSettings();

    HookExport(user32, "CreateWindowExW", (void*)CreateWindowExW_Hook,
               (void**)&CreateWindowExW_Original);
    HookExport(user32, "DrawIconEx", (void*)DrawIconEx_Hook, (void**)&DrawIconEx_Original);
    HookExport(user32, "DrawIcon", (void*)DrawIcon_Hook, (void**)&DrawIcon_Original);
    HookExport(user32, "DrawStateW", (void*)DrawStateW_Hook, (void**)&DrawStateW_Original);
    HookExport(user32, "FillRect", (void*)FillRect_Hook, (void**)&FillRect_Original);
    HookExport(user32, "FrameRect", (void*)FrameRect_Hook, (void**)&FrameRect_Original);

    HookExport(gdi32, "BitBlt", (void*)BitBlt_Hook, (void**)&BitBlt_Original);
    HookExport(gdi32, "StretchBlt", (void*)StretchBlt_Hook, (void**)&StretchBlt_Original);
    HookExport(gdi32, "MaskBlt", (void*)MaskBlt_Hook, (void**)&MaskBlt_Original);
    HookExport(gdi32, "GdiAlphaBlend", (void*)GdiAlphaBlend_Hook, (void**)&GdiAlphaBlend_Original);
    HookExport(gdi32, "GdiTransparentBlt", (void*)GdiTransparentBlt_Hook,
               (void**)&GdiTransparentBlt_Original);
    HookExport(gdi32, "PatBlt", (void*)PatBlt_Hook, (void**)&PatBlt_Original);
    HookExport(gdi32, "Rectangle", (void*)Rectangle_Hook, (void**)&Rectangle_Original);
    HookExport(gdi32, "RoundRect", (void*)RoundRect_Hook, (void**)&RoundRect_Original);

    HookExport(comctl32, "ImageList_DrawIndirect", (void*)ImageList_DrawIndirect_Hook,
               (void**)&ImageList_DrawIndirect_Original);

    HookExport(uxtheme, "DrawThemeIcon", (void*)DrawThemeIcon_Hook,
               (void**)&DrawThemeIcon_Original);
    HookExport(uxtheme, "DrawThemeBackground", (void*)DrawThemeBackground_Hook,
               (void**)&DrawThemeBackground_Original);
    HookExport(uxtheme, "DrawThemeTextEx", (void*)DrawThemeTextEx_Hook,
               (void**)&DrawThemeTextEx_Original);

    return TRUE;
}

static BOOL CALLBACK ApplyBorderColorToExisting(HWND hWnd, LPARAM lParam) {
    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);
    if (processId == GetCurrentProcessId() && IsThumbnailWindow(hWnd)) {
        ApplyBorderColor(hWnd, true);
    }
    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();

    // Previews are recreated on every hover, so this only matters for one already on
    // screen, but it makes the color land without a rehover.
    EnumWindows(ApplyBorderColorToExisting, 0);
}

void Wh_ModUninit() {
    // Subclasses are torn down with explorer; nothing else to release.
}
