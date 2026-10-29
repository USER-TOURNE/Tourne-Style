// ==WindhawkMod==
// @id              tourne-menus
// @name            Tourne Menus
// @description     Tourne Desktop's menus in File Explorer, the Open and Save dialogs, Windows' own menus, every program's menus and menu bars: its look, the wheel scrolling long menus, its hide, More options and no-duplicates rules and how menus open
// @version         1.1.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @donateUrl       https://ko-fi.com/tourne
// @include         *
// @include         windhawk.exe
// @exclude         TourneDesktop.exe
// @exclude         vgc.exe
// @compilerOptions -lcomctl32 -ldwmapi -luxtheme -lgdi32 -lshlwapi -lole32 -ladvapi32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tourne Menus

Tourne Desktop draws its own menus (the bar's, Start's, the desktop's) in
Tourne's look. This mod draws the right-click menus of other places the same
way, each with its own switch:

- **File Explorer windows**: files, folders, the empty space, the navigation
  pane. Windows 11's new menu is skipped for them, so the full menu shows at
  once (hold Shift to get Windows' own menu instead).
- **Open and Save dialogs**, in every program.
- **Windows' own menus**: the ones Windows would draw as "immersive" menus
  (Explorer's taskbar and desktop when Explorer is the shell, the volume,
  network, language and Windows Security icons, Task Manager, Store apps'
  title bar menus). They're made plain menus first, as Classic Menus does.
- **Every program's menus**: right-click and pop-up menus in any program,
  as the Classic Menus and Dark Menus mods restyle them, but in Tourne's look.
- **Menu bars**: the File, Edit, View row of programs that have one, in
  Tourne's colours, as Dark Menus does.
- **Duplicate items**: items listed by name (Convertify, say) that some
  menus show twice, kept once, as Menu No Duplicates does.

With these on, Classic Menus, Dark Menus, Eradicate Immersive Menus and
Tourne'Suite's Classic Menus, Menu No Icons and Menu No Duplicates parts
have nothing left to do: turn them off, so menus aren't styled twice.
(Menu No Icons: Tourne Desktop's own "No icons in menus" setting reaches
every menu drawn here.)

All three start off: turn on the ones you want.

What the menus look like, how they open and what's left out of them all come
from Tourne Desktop: its Menus, Menu frame, Menus open, Leave out of
right-click menus and Move into More options settings (Studio, Taskbar). It
writes them to `HKCU\Software\Tourne\Menus` whenever its settings apply, and
this mod reads them as each menu opens, so a change shows on the next menu.
Without Tourne Desktop, menus get Tourne's green scheme.

In every menu drawn here, the mouse wheel scrolls a menu taller than the
screen (3 items a notch).

Not drawn here: menus whose items draw themselves (a few programs' own), and
the icons some shell extensions draw themselves (Open with, Send to); those
items show without them. Hold Shift while right-clicking for Windows' own
menu.

The way Windows 11's new File Explorer menu is skipped follows m417z's
"Classic context menu" Windhawk mod (MIT); the idea for this mod came from
Bolt-Scripts' Custom Cached Context Menu (MIT). No code was copied from either.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- explorer: false
  $name: File Explorer windows
  $description: >-
    Right-click menus in File Explorer windows, with Windows 11's new menu
    skipped for them (Shift + right-click still gives Windows' own).
- fileDialogs: false
  $name: Open and Save dialogs
  $description: Right-click menus in every program's Open, Save and folder dialogs.
- windowsMenus: false
  $name: Windows' own menus
  $description: >-
    The menus Windows draws as "immersive" menus: Explorer's taskbar and
    desktop (when Explorer is the shell), the volume, network, language and
    Windows Security icons, Task Manager, Store apps' title bar menus.
- allMenus: false
  $name: Every program's menus
  $description: >-
    Right-click and pop-up menus in every program, as Classic Menus and Dark
    Menus do, in Tourne's look. Menus a program draws itself stay its own.
- menuBars: false
  $name: Menu bars
  $description: >-
    The File, Edit, View row of programs that have one, in Tourne's colours
    (as Dark Menus does).
- noDuplicates: ''
  $name: Show these items once
  $description: >-
    Item names, separated by commas, like Convertify: when a menu shows one
    of them more than once, only one stays (as Menu No Duplicates does).
- noDuplicatesKeep: last
  $name: Which one stays
  $options:
  - first: The first
  - last: The last
- excludeApps: ''
  $name: Leave these programs alone
  $description: Program file names, separated by commas, like game.exe.
*/
// ==/WindhawkModSettings==

// This file is made from tools/menus/tourne-menus.cpp by tools/menus/make-menus.py, which puts in the
// shared files where "// @inline" lines name them. Edit those, not tourne-menus.wh.cpp.

#include <windhawk_utils.h>

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <string>
#include <vector>

// Every thread its own menu state: each File Explorer window has a thread of its own.
#define MENU_STYLE_STORAGE thread_local

// Popup menus in the Tourne style, shared by the taskbar and System Icons
// ("// @inline menu-style.cpp"): every item owner-drawn from the colour
// scheme, or from a visual style's own menu look when it has one, in the
// Tourne pixel font, with a square frame in the scheme's edge colour. The
// look is handed in each time a menu opens, so a changed theme or scheme
// shows on the next menu.
//
// Use: set g_menuLook, call TrackStyledMenu instead of TrackPopupMenuEx, and
// pass WM_MEASUREITEM and WM_DRAWITEM from the owner window to
// MenuMeasureItem and MenuDrawItem. AppendMenuHeader adds a section's title
// (a jump list's "Tasks"), AppendMenuIcon an item with an icon; the caller
// keeps the icon until the menu closes.
//
// Also here, for every menu that goes through TrackStyledMenu: the mouse wheel
// scrolls a menu taller than its screen (MenuWheel*), menus can open with a
// short animation (g_menuMotion, none by default), and labels or verbs can be
// taken out of a shell menu or moved into a "More options" submenu
// (ApplyMenuRules, UndoMenuRules).
//
// A program with menus on several threads (Tourne Menus, in Explorer) defines
// MENU_STYLE_STORAGE as thread_local before this file, so each thread has its own.

#ifndef MENU_STYLE_STORAGE
#define MENU_STYLE_STORAGE
#endif

struct MenuLook {
    uint32_t ground = 0x121C21, line = 0x1E2F36, edge = 0x4F8F6A, text = 0x66EA97, accent = 0x15FFA2,
             dim = 0x5D8D6F, hover = 0x1F3D34;
    HFONT font = nullptr;    // null: the system's menu font
    HTHEME theme = nullptr;  // a visual style's "Menu" class, or null for the scheme
    int scale = 1;
    bool noIcons = false;    // as Menu No Icons does for other programs' menus
    // Windows' own classic menus, drawn by the visual style (StartAllBack's
    // are): nothing owner-drawn, an item's icon given as its bitmap.
    bool native = false;
    bool classic = false;  // with a theme: Windows' classic menu metrics
    bool ownText = false;  // the text colour stays, over the theme's
    // StartAllBack's own menus with a colour style: a line round the menu, a band, a second line,
    // and the items inside it (measured from its Bouquet menu: 1 px line, 6 px band, 1 px line).
    bool frame = false;
    bool caps = false;  // item text in capitals, as its icon title font is used
    uint32_t frameLine = 0xADA69E, band = 0x151F24;
    // The frame as set (taskbar.menuFrame*): the band between the lines, the lines' colours (none: frameLine;
    // two or three: a gradient from the top left to the bottom right), and a glow round them, 0 to 100.
    int frameGap = 6;
    uint32_t frameStops[3]{};
    int frameStopCount = 0;
    int frameGlow = 0;
    // Windows' own classic menus, as other programs show them here (LookLikeWindowsMenus): flat rows,
    // Windows' row sizes and insets, and the arrow and the chosen item's text in their own colours.
    bool windows = false;
    uint32_t hoverText = 0xFFFFFFFF, arrow = 0xFFFFFFFF;  // all ones: the text colour
};

MENU_STYLE_STORAGE MenuLook g_menuLook;
// A foreign menu's items Windows' shell has made owner-drawn for its own look (its immersive menu helper, as
// in a file dialog with no hook to stop it) taken over too: those with text, and separators. Set for one menu.
MENU_STYLE_STORAGE bool g_menuTakeOwnerDrawn = false;

// How menus open (taskbar.menuAnimation): at once (none, the default), or with a short animation timed by
// the motion engine (its popups line), set by whoever opens the menu.
struct MenuMotion {
    int style = 0;  // 0 none, 1 fade, 2 slide (and fade), 3 unfold
    double ms = 150;
    double (*ease)(double t) = nullptr;  // the motion engine's curve (may pass 1 on the way); null: an ease-out
};
MENU_STYLE_STORAGE MenuMotion g_menuMotion;
// Submenus open on a click (or Right / Enter), not as the pointer passes over them (taskbar.menuFlyoutClick here;
// FlyoutClick in Tourne Menus' published look).
MENU_STYLE_STORAGE bool g_menuFlyoutClick = false;

// The motion styles by the names the settings use.
int MenuMotionStyle(std::wstring const& name) {
    return name == L"fade" ? 1 : name == L"slide" ? 2 : name == L"unfold" ? 3 : 0;
}

// A motion line's style word for menus (motion.popups = 1, 3, default, popin 80%): popin and gnomed unfold, slide
// and slidefade slide, fade fades; any other word (or none) keeps the menuAnimation setting's.
int MenuMotionStyleWord(std::wstring const& word, int fallback) {
    if (word == L"popin" || word == L"gnomed" || word == L"unfold") return 3;
    if (word == L"slide" || word == L"slidefade" || word == L"slidevert" || word == L"slidefadevert") return 2;
    if (word == L"fade") return 1;
    return fallback;
}

constexpr COLORREF MenuRgb(uint32_t rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

// An owner-drawn item keeps its text here, and what it was, to be put back.
struct StyledMenuItem {
    std::wstring text;
    bool separator = false;
    bool submenu = false;
    bool header = false;
    HICON icon = nullptr;
    bool padTop = false, padBottom = false;  // the first and last items carry the room above and below the items inside a frame
    int extraBottom = 0;  // a shorter column's last item, taller by this so every column ends together (FitStyledMenu)
    UINT type = 0;
    ULONG_PTR data = 0;
    // A shell item's own bitmap: taken off while the item is ours, since
    // Windows sizes an owner-drawn item with a bitmap from the bitmap alone
    // (a 22 px sliver of a submenu), and drawn as its icon instead.
    HBITMAP bitmap = nullptr;
};

// A real bitmap, not one of HBMMENU_CALLBACK and the other stand-ins.
bool MenuBitmapDrawable(HBITMAP bitmap) {
    return bitmap && (ULONG_PTR)bitmap > 11 && bitmap != HBMMENU_CALLBACK;
}

// The items styled now, so another owner's owner-drawn items (a shell
// context menu's) are never taken for ours.
MENU_STYLE_STORAGE std::vector<StyledMenuItem*> g_styledItems;

StyledMenuItem* StyledItemOf(ULONG_PTR data) {
    auto* item = (StyledMenuItem*)data;
    return data && std::find(g_styledItems.begin(), g_styledItems.end(), item) != g_styledItems.end() ? item : nullptr;
}

// An item's data, before the style takes it over: a header, or an icon.
constexpr ULONG_PTR kMenuHeaderTag = 1;

void AppendMenuHeader(HMENU menu, LPCWSTR text) {
    MENUITEMINFOW info{sizeof(info)};
    info.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_STRING | MIIM_DATA;
    info.fType = MFT_STRING;
    info.fState = MFS_DISABLED;
    info.dwTypeData = (LPWSTR)text;
    info.dwItemData = kMenuHeaderTag;
    InsertMenuItemW(menu, GetMenuItemCount(menu), TRUE, &info);
}

void AppendMenuIcon(HMENU menu, UINT flags, UINT_PTR id, LPCWSTR text, HICON icon) {
    AppendMenuW(menu, flags, id, text);
    if (!icon) return;
    MENUITEMINFOW info{sizeof(info)};
    info.fMask = MIIM_DATA;
    info.dwItemData = (ULONG_PTR)icon;
    SetMenuItemInfoW(menu, GetMenuItemCount(menu) - 1, TRUE, &info);
}

// An icon as a menu item's bitmap (32 bpp, alpha premultiplied, as menus
// draw them), for native menus.
MENU_STYLE_STORAGE std::vector<HBITMAP> g_nativeMenuBitmaps;  // made for menus open now

HBITMAP MenuIconBitmap(HICON icon) {
    int size = 16 * g_menuLook.scale;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        memset(bits, 0, size * size * 4);
        DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        GdiFlush();
        // An icon without alpha leaves it at 0: whatever it drew is opaque.
        auto* px = (uint32_t*)bits;
        bool alpha = false;
        for (int i = 0; i < size * size && !alpha; i++) alpha = px[i] >> 24;
        if (!alpha)
            for (int i = 0; i < size * size; i++)
                if (px[i]) px[i] |= 0xFF000000;
        SelectObject(dc, old);
    }
    DeleteDC(dc);
    return bitmap;
}

// A native menu's own items with icons get them as bitmaps.
void NativeMenuItems(HMENU menu, bool foreign) {
    int count = GetMenuItemCount(menu);
    for (int i = 0; i < count; i++) {
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_BITMAP;
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        if (info.hSubMenu) NativeMenuItems(info.hSubMenu, foreign);
        if (foreign || g_menuLook.noIcons || info.hbmpItem || !info.dwItemData || info.dwItemData == kMenuHeaderTag)
            continue;
        HBITMAP bitmap = MenuIconBitmap((HICON)info.dwItemData);
        if (!bitmap) continue;
        g_nativeMenuBitmaps.push_back(bitmap);
        MENUITEMINFOW set{sizeof(set)};
        set.fMask = MIIM_BITMAP;
        set.hbmpItem = bitmap;
        SetMenuItemInfoW(menu, i, TRUE, &set);
    }
}

// Styles a menu's items. A foreign menu (a shell context menu) keeps its own
// item data, which is put back afterwards, and draws its own owner-drawn items.
void StyleMenuItems(HMENU menu, bool foreign = false) {
    if (g_menuLook.native) {
        NativeMenuItems(menu, foreign);
        return;
    }
    int count = GetMenuItemCount(menu);
    for (int i = 0; i < count; i++) {
        WCHAR text[256] = L"";
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_SUBMENU | MIIM_DATA | MIIM_BITMAP;
        info.dwTypeData = text;
        info.cch = ARRAYSIZE(text);
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        if (info.hSubMenu) StyleMenuItems(info.hSubMenu, foreign);
        if (info.fType & MFT_OWNERDRAW) {  // styled already, or drawn by its owner
            bool shellDrawn = foreign && g_menuTakeOwnerDrawn && !StyledItemOf(info.dwItemData) &&
                              (text[0] || (info.fType & MFT_SEPARATOR));
            if (!shellDrawn) continue;
        }
        if (g_menuLook.caps && !foreign) CharUpperBuffW(text, (DWORD)wcslen(text));
        auto* item = new StyledMenuItem{text, (info.fType & MFT_SEPARATOR) != 0, info.hSubMenu != nullptr};
        item->type = info.fType;
        item->data = info.dwItemData;
        item->bitmap = info.hbmpItem;
        if (!foreign && info.dwItemData == kMenuHeaderTag) {
            item->header = true;
        } else if (!foreign && info.dwItemData) {
            item->icon = (HICON)info.dwItemData;
        }
        g_styledItems.push_back(item);
        MENUITEMINFOW set{sizeof(set)};
        set.fMask = MIIM_FTYPE | MIIM_DATA | (item->bitmap ? MIIM_BITMAP : 0);
        set.fType = (info.fType & ~MFT_STRING) | MFT_OWNERDRAW;
        set.dwItemData = (ULONG_PTR)item;
        set.hbmpItem = nullptr;
        SetMenuItemInfoW(menu, i, TRUE, &set);
    }
    // A frame's room above and below the items (in every menu and submenu, ours or a shell's):
    // carried by the first and last item, which are taller by it, not items of their own.
    if (g_menuLook.frame && count > 0) {
        for (int at : {0, count - 1}) {
            MENUITEMINFOW edge{sizeof(edge)};
            edge.fMask = MIIM_DATA;
            if (!GetMenuItemInfoW(menu, at, TRUE, &edge)) continue;
            if (StyledMenuItem* item = StyledItemOf(edge.dwItemData)) {
                if (at == 0) item->padTop = true;
                if (at == count - 1) item->padBottom = true;
            }
        }
    }    // The background between items, and around them.
    MENUINFO mi{sizeof(mi)};
    mi.fMask = MIM_BACKGROUND;
    static MENU_STYLE_STORAGE HBRUSH brush;
    static MENU_STYLE_STORAGE uint32_t brushColor = 0xFFFFFFFF;
    if (brushColor != g_menuLook.ground) {
        if (brush) DeleteObject(brush);
        brush = CreateSolidBrush(MenuRgb(g_menuLook.ground));
        brushColor = g_menuLook.ground;
    }
    mi.hbrBack = brush;
    SetMenuInfo(menu, &mi);
}

void FreeStyledMenuItems(HMENU menu) {
    int count = GetMenuItemCount(menu);
    for (int i = count - 1; i >= 0; i--) {  // (backwards, as it once removed items)
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_DATA | MIIM_SUBMENU;
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        if (info.hSubMenu) FreeStyledMenuItems(info.hSubMenu);
        StyledMenuItem* item = StyledItemOf(info.dwItemData);
        if (!item) continue;

        MENUITEMINFOW set{sizeof(set)};
        set.fMask = MIIM_FTYPE | MIIM_DATA | (item->bitmap ? MIIM_BITMAP : 0);
        set.fType = item->type;
        set.dwItemData = item->data;
        set.hbmpItem = item->bitmap;
        SetMenuItemInfoW(menu, i, TRUE, &set);
        std::erase(g_styledItems, item);
        delete item;
    }
}

// An item's height without a frame's room or a column's extra (MenuMeasureItem, FitStyledMenu).
UINT StyledItemHeight(StyledMenuItem const* item) {
    int s = g_menuLook.scale;
    bool classic = (g_menuLook.classic && g_menuLook.theme) || g_menuLook.windows;
    if (item->separator) return (classic ? 9 : 7) * s;
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = g_menuLook.font ? SelectObject(dc, g_menuLook.font) : nullptr;
    std::wstring main = item->text.substr(0, item->text.find(L'\t'));
    SIZE left{};
    GetTextExtentPoint32W(dc, main.empty() ? L" " : main.c_str(), main.empty() ? 1 : (int)main.size(), &left);
    if (old) SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    return item->header ? left.cy + 6 * s : classic ? std::max<UINT>(18 * s, left.cy + 5 * s) : std::max<UINT>(22 * s, left.cy + 8 * s);
}

bool MenuMeasureItem(MEASUREITEMSTRUCT* measure) {
    if (!measure || measure->CtlType != ODT_MENU) return false;
    StyledMenuItem* item = StyledItemOf(measure->itemData);
    if (!item) return false;
    int s = g_menuLook.scale;
    bool classic = (g_menuLook.classic && g_menuLook.theme) || g_menuLook.windows;

    if (item->separator) {
        measure->itemWidth = 1;
        measure->itemHeight = StyledItemHeight(item) + item->extraBottom;
        if (g_menuLook.frame) measure->itemHeight += ((item->padTop ? 12 : 0) + (item->padBottom ? 12 : 0)) * s;
        return true;
    }
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = g_menuLook.font ? SelectObject(dc, g_menuLook.font) : nullptr;
    std::wstring text = item->text;
    size_t tab = text.find(L'\t');
    SIZE left{}, right{};
    std::wstring main = text.substr(0, tab);
    GetTextExtentPoint32W(dc, main.empty() ? L" " : main.c_str(), main.empty() ? 1 : (int)main.size(), &left);
    if (tab != std::wstring::npos) {
        std::wstring accel = text.substr(tab + 1);
        GetTextExtentPoint32W(dc, accel.c_str(), (int)accel.size(), &right);
    }
    if (old) SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    // Room for a check or icon on the left and an arrow or the extra text on
    // the right. A header starts further left and sits closer to its items.
    measure->itemWidth = (item->header ? 8 * s : (g_menuLook.windows ? 18 : classic ? 22 : 26) * s) + left.cx + (right.cx ? 16 * s + right.cx : 0) +
                         22 * s + (g_menuLook.frame ? (8 + 11 - 5) * s * 2 : 0);
    // Classic: Windows' own 18 px rows at the menu font's size.
    measure->itemHeight = StyledItemHeight(item) + item->extraBottom;
    if (g_menuLook.frame) measure->itemHeight += ((item->padTop ? 12 : 0) + (item->padBottom ? 12 : 0)) * s;
    return true;
}

uint32_t MenuMix(uint32_t a, uint32_t b, float t) {
    auto channel = [&](int shift) {
        int x = (a >> shift) & 0xFF, y = (b >> shift) & 0xFF;
        return (uint32_t)std::clamp((int)(x + (y - x) * t + 0.5f), 0, 255) << shift;
    };
    return channel(16) | channel(8) | channel(0);
}

// The frame line's colour at a point of the menu window (x, y in it; w, h its size).
uint32_t FrameLineAt(MenuLook const& look, int x, int y, int w, int h) {
    if (look.frameStopCount <= 0) return look.frameLine;
    if (look.frameStopCount == 1) return look.frameStops[0];
    float t = std::clamp((x / (float)std::max(1, w - 1) + y / (float)std::max(1, h - 1)) / 2, 0.f, 1.f);
    float at = t * (look.frameStopCount - 1);
    int i = std::min((int)at, look.frameStopCount - 2);
    return MenuMix(look.frameStops[i], look.frameStops[i + 1], at - i);
}

// Where the inner frame line is, in from the menu window's edge (the outer line is at 0).
int FrameInnerAt(int scale) { return (std::clamp(g_menuLook.frameGap, 1, 16) + 2) * scale - 1; }

// A frame line one pixel across, from (x, y) for length pixels, across or down, each pixel coloured by where it
// sits on the menu window (at: where this dc's 0,0 is in the window; size: the window's), with its glow over the
// colours to each side (before: above or left, after: below or right). As plain as before without a colour
// gradient or a glow set. pass: 1 the glow only, 2 the line only, 3 both (glows first for every line, then the
// lines, so a glow never covers the line it meets at a corner).
void FrameLine(HDC dc, int x, int y, int length, bool across, POINT at, SIZE size, uint32_t before, uint32_t after,
               bool glowBefore = true, bool glowAfter = true, int pass = 3) {
    auto const& look = g_menuLook;
    if (look.frameStopCount <= 0 && look.frameGlow <= 0) {
        if (!(pass & 2)) return;
        RECT r = across ? RECT{x, y, x + length, y + 1} : RECT{x, y, x + 1, y + length};
        SetDCBrushColor(dc, MenuRgb(look.frameLine));
        FillRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
        return;
    }
    if (length <= 0) return;
    float glow = std::clamp(look.frameGlow, 0, 100) / 100.f;
    // Each strip (the line, and each glow row beside it) is worked out here and drawn in one call: pixel by pixel,
    // a flyout's frame was thousands of trips into the kernel, which made flyouts open slowly and held up the pointer.
    auto strip = [&](int sx, int sy, std::vector<uint32_t> const& px) {
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), across ? length : 1, -(across ? 1 : length), 1, 32, BI_RGB};
        SetDIBitsToDevice(dc, sx, sy, across ? length : 1, across ? 1 : length, 0, 0, 0, across ? 1 : length, px.data(), &bi,
                          DIB_RGB_COLORS);
    };
    auto dib = [](uint32_t rgb) {  // (0xRRGGBB, as MenuRgb takes it, to a DIB pixel)
        COLORREF c = MenuRgb(rgb);
        return (uint32_t)GetRValue(c) << 16 | (uint32_t)GetGValue(c) << 8 | GetBValue(c);
    };
    std::vector<uint32_t> line(length);
    for (int i = 0; i < length; i++)
        line[i] = FrameLineAt(look, (across ? x + i : x) + at.x, (across ? y : y + i) + at.y, size.cx, size.cy);
    if (glow > 0 && (pass & 1)) {
        // One pixel each side up to half strength, two beyond (a frame is only a few pixels across).
        int reach = look.frameGlow > 50 ? 2 : 1;
        std::vector<uint32_t> row(length);
        for (int d = 1; d <= reach; d++) {
            float a = glow * (d == 1 ? 0.55f : 0.22f);
            if (glowBefore) {
                for (int i = 0; i < length; i++) row[i] = dib(MenuMix(before, line[i], a));
                strip(across ? x : x - d, across ? y - d : y, row);
            }
            if (glowAfter) {
                for (int i = 0; i < length; i++) row[i] = dib(MenuMix(after, line[i], a));
                strip(across ? x : x + d, across ? y + d : y, row);
            }
        }
    }
    if (pass & 2) {
        std::vector<uint32_t> row(length);
        for (int i = 0; i < length; i++) row[i] = dib(line[i]);
        strip(x, y, row);
    }
}

bool MenuDrawItem(DRAWITEMSTRUCT* draw) {
    if (!draw || draw->CtlType != ODT_MENU) return false;
    StyledMenuItem* item = StyledItemOf(draw->itemData);
    if (!item) return false;
    auto const& look = g_menuLook;
    HDC dc = draw->hDC;
    RECT r = draw->rcItem;
    int s = look.scale;
    bool selected = draw->itemState & ODS_SELECTED;
    bool disabled = draw->itemState & (ODS_DISABLED | ODS_GRAYED);
    bool checked = draw->itemState & ODS_CHECKED;
    HTHEME theme = look.theme;
    bool classic = look.classic && theme;

    auto fill = [&](RECT const& area, uint32_t rgb) {
        SetDCBrushColor(dc, MenuRgb(rgb));
        FillRect(dc, &area, (HBRUSH)GetStockObject(DC_BRUSH));
    };

    if (look.frame) {
        // The band and the inner line round the items; what follows draws inside them.
        int nc = 3 * s, ncTop = 3 * s;  // the menu window's own border, measured from the window when it can be
        SIZE windowSize{r.right - r.left + 2 * nc, 0};
        if (HWND menuWindow = WindowFromDC(dc)) {
            RECT window, client;
            POINT origin{0, 0};
            if (GetWindowRect(menuWindow, &window) && GetClientRect(menuWindow, &client) && ClientToScreen(menuWindow, &origin)) {
                nc = origin.x - window.left;
                ncTop = origin.y - window.top;
                windowSize = {window.right - window.left, window.bottom - window.top};
            }
        }
        // The inner line sits frameGap past the outer one (StartAllBack's: 6). Closer than the window's own border
        // is wide, it's in that border, drawn with the outer one (StyledMenuWindowProc), and the items fill to
        // their edges.
        int innerAt = FrameInnerAt(s);
        bool lineInEdge = innerAt < nc;
        int inset = lineInEdge ? 0 : innerAt + 1 - nc;
        RECT whole = r;
        fill(whole, look.band);
        int xl = r.left + inset, xr = r.right - inset;
        RECT inside{xl, r.top, xr, r.bottom};
        int lineTop = r.top, lineBottom = r.bottom;  // how far down the side lines run
        if (item->padTop) {
            inside.top = r.top + inset;
            lineTop = r.top + inset - 1;
        }
        if (item->padBottom) {
            inside.bottom = r.bottom - inset;
            lineBottom = r.bottom - inset + 1;
        }
        fill(inside, look.ground);
        POINT at{nc, ncTop};
        if (lineInEdge) {
            // The line is in the border; only its glow reaches in here, when it sits right beside the items.
            if (innerAt == nc - 1) {
                if (item->padTop) FrameLine(dc, r.left - 1, r.top - 1, r.right - r.left + 2, true, at, windowSize, look.band, look.ground, false, true, 1);
                if (item->padBottom)
                    FrameLine(dc, r.left - 1, r.bottom, r.right - r.left + 2, true, at, windowSize, look.ground, look.band, true, false, 1);
                FrameLine(dc, r.left - 1, r.top, r.bottom - r.top, false, at, windowSize, look.band, look.ground, false, true, 1);
                FrameLine(dc, r.right, r.top, r.bottom - r.top, false, at, windowSize, look.ground, look.band, true, false, 1);
            }
        } else for (int pass : {1, 2}) {
            if (item->padTop) FrameLine(dc, xl - 1, lineTop, xr + 1 - (xl - 1), true, at, windowSize, look.band, look.ground, true, true, pass);
            if (item->padBottom)
                FrameLine(dc, xl - 1, inside.bottom, xr + 1 - (xl - 1), true, at, windowSize, look.ground, look.band, true, true, pass);
            FrameLine(dc, xl - 1, lineTop, lineBottom - lineTop, false, at, windowSize, look.band, look.ground, true, true, pass);
            FrameLine(dc, xr, lineTop, lineBottom - lineTop, false, at, windowSize, look.ground, look.band, true, true, pass);
        }
        r = RECT{xl, r.top + (item->padTop ? 12 * s : 0), xr, r.bottom - (item->padBottom ? 12 * s : 0) - item->extraBottom};
    }
    if (look.frame) {
        // (the ground is down already)
    } else if (theme) {
        DrawThemeBackground(theme, dc, MENU_POPUPBACKGROUND, 0, &r, nullptr);
    } else {
        fill(r, look.ground);
    }
    if (!look.frame) r.bottom -= item->extraBottom;  // (a shorter column's last item: what it draws sits at its top)
    if (item->separator) {
        RECT line{r.left + 6 * s, (r.top + r.bottom) / 2, r.right - 6 * s, (r.top + r.bottom) / 2 + s};
        if (look.windows) {
            // Windows' flat separator: its lower line (the upper one is the border's colour), 1 px in from each side.
            line.left = r.left + s;
            line.right = r.right - s;
            fill(line, look.line);
        } else if (classic && look.ownText) {
            // A 1 px rule in the text colour, as System Text Color leaves them.
            line.right = r.right - 2 * s;
            fill(line, look.text);
        } else if (theme) {
            RECT sep{r.left, r.top, r.right, r.bottom};
            DrawThemeBackground(theme, dc, MENU_POPUPSEPARATOR, 0, &sep, nullptr);
        } else {
            fill(line, look.line);
        }
        ExcludeClipRect(dc, draw->rcItem.left, draw->rcItem.top, draw->rcItem.right, draw->rcItem.bottom);
        return true;
    }
    if (selected && !disabled) {
        RECT hot{r.left + 2 * s, r.top, r.right - 2 * s, r.bottom};
        if (look.windows) hot = r;  // (a flat menu's chosen row runs the whole width)
        if (theme) {
            DrawThemeBackground(theme, dc, MENU_POPUPITEM, MPI_HOT, &hot, nullptr);
        } else {
            fill(hot, look.hover);
        }
    }

    bool hoverText = selected && !disabled && look.hoverText != 0xFFFFFFFF;
    COLORREF color = MenuRgb(disabled ? look.dim : hoverText ? look.hoverText : look.text);
    if (theme && (!look.ownText || disabled))
        GetThemeColor(theme, MENU_POPUPITEM, disabled ? MPI_DISABLED : selected ? MPI_HOT : MPI_NORMAL, TMT_TEXTCOLOR,
                      &color);
    if (item->header) {
        HGDIOBJ old = look.font ? SelectObject(dc, look.font) : nullptr;
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, color);
        RECT area{r.left + 8 * s, r.top + 2 * s, r.right - 8 * s, r.bottom};
        DrawTextW(dc, item->text.c_str(), -1, &area, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
        if (old) SelectObject(dc, old);
        ExcludeClipRect(dc, draw->rcItem.left, draw->rcItem.top, draw->rcItem.right, draw->rcItem.bottom);
        return true;
    }
    if (item->icon && !look.noIcons) {
        int size = 16 * s;
        DrawIconEx(dc, r.left + (look.frame ? 11 : 5) * s, (r.top + r.bottom - size) / 2, item->icon, size, size, 0, nullptr, DI_NORMAL);
    } else if (MenuBitmapDrawable(item->bitmap) && !look.noIcons && !checked) {
        // A shell item's bitmap (32 bpp with alpha, as the shell makes them).
        BITMAP info{};
        GetObjectW(item->bitmap, sizeof(info), &info);
        int size = 16 * s;
        HDC mem = CreateCompatibleDC(dc);
        HGDIOBJ oldBitmap = SelectObject(mem, item->bitmap);
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, (BYTE)(info.bmBitsPixel == 32 ? AC_SRC_ALPHA : 0)};
        GdiAlphaBlend(dc, r.left + (look.frame ? 11 : 5) * s, (r.top + r.bottom - size) / 2, size, size, mem, 0, 0, info.bmWidth,
                      std::abs(info.bmHeight), blend);
        SelectObject(mem, oldBitmap);
        DeleteDC(mem);
    } else if (checked && classic) {
        // The style's own tick (MENU_POPUPCHECK), as Windows draws it.
        int size = 16 * s;
        RECT tick{r.left + 3 * s, (r.top + r.bottom - size) / 2, r.left + 3 * s + size, (r.top + r.bottom + size) / 2};
        DrawThemeBackground(theme, dc, MENU_POPUPCHECK, disabled ? MC_CHECKMARKDISABLED : MC_CHECKMARKNORMAL, &tick,
                            nullptr);
    } else if (checked) {
        // A checked item: a small square in the accent colour.
        int size = 6 * s;
        int x = r.left + (look.frame ? 15 : 9) * s, y = (r.top + r.bottom - size) / 2;
        RECT mark{x, y, x + size, y + size};
        fill(mark, disabled ? look.dim : look.accent);
    }
    HGDIOBJ old = look.font ? SelectObject(dc, look.font) : nullptr;
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    std::wstring text = item->text;
    size_t tab = text.find(L'\t');
    RECT area{r.left + (look.frame ? 31 : look.windows ? 18 : classic ? 22 : 26) * s, r.top, r.right - 22 * s, r.bottom};
    DrawTextW(dc, text.substr(0, tab).c_str(), -1, &area, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_HIDEPREFIX);
    if (tab != std::wstring::npos)
        DrawTextW(dc, text.substr(tab + 1).c_str(), -1, &area, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOPREFIX);
    // A submenu's arrow, drawn here: the tray overflow's chevron (System Icons' design, turned to point
    // right), 4 x 8 pixels at the menu's scale. The system's own is clipped away below, over the whole
    // item, as a frame's band reaches past what's drawn here.
    if (item->submenu) {
        static constexpr const char* kChevron[] = {"#...", "##..", ".##.", "..##", "..##", ".##.", "##..", "#..."};
        int x = r.right - 12 * s, y = (r.top + r.bottom) / 2 - 4 * s;
        if (look.windows) x = r.right - 10 * s, y -= 2 * s;  // (where Windows' own arrow glyph sits)
        COLORREF arrow = look.arrow != 0xFFFFFFFF && !disabled ? MenuRgb(look.arrow) : color;
        SetDCBrushColor(dc, arrow);
        for (int row = 0; row < 8; row++)
            for (int col = 0; col < 4; col++)
                if (kChevron[row][col] == '#') {
                    RECT dot{x + col * s, y + row * s, x + (col + 1) * s, y + (row + 1) * s};
                    FillRect(dc, &dot, (HBRUSH)GetStockObject(DC_BRUSH));
                }
    }
    if (old) SelectObject(dc, old);
    ExcludeClipRect(dc, draw->rcItem.left, draw->rcItem.top, draw->rcItem.right, draw->rcItem.bottom);
    return true;
}

// The menu windows' frame: the scheme's ground with a 1 px edge, square. The
// windows are caught as they're created on this thread.
LRESULT CALLBACK StyledMenuWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_NCPAINT:
        case WM_PRINT: {
            LRESULT result = msg == WM_PRINT ? DefSubclassProc(hwnd, msg, wParam, lParam) : 0;
            HDC dc = msg == WM_PRINT ? (HDC)wParam : GetWindowDC(hwnd);
            RECT window, client;
            GetWindowRect(hwnd, &window);
            GetClientRect(hwnd, &client);
            MapWindowPoints(hwnd, nullptr, (POINT*)&client, 2);
            OffsetRect(&client, -window.left, -window.top);
            OffsetRect(&window, -window.left, -window.top);
            HRGN frame = CreateRectRgnIndirect(&window);
            HRGN inside = CreateRectRgnIndirect(&client);
            CombineRgn(frame, frame, inside, RGN_DIFF);
            SetDCBrushColor(dc, MenuRgb(g_menuLook.frame ? g_menuLook.band : g_menuLook.ground));
            FillRgn(dc, frame, (HBRUSH)GetStockObject(DC_BRUSH));
            if (g_menuLook.frame) {
                // The outer line, its glow falling inwards onto the band (the frame as set: FrameLine).
                POINT at{0, 0};
                SIZE size{window.right, window.bottom};
                uint32_t band = g_menuLook.band, ground = g_menuLook.ground;
                // The inner line too when it falls in this border (a small gap), with the items' ground inside it.
                int q = FrameInnerAt(std::max(1, g_menuLook.scale));
                bool inner = q < client.left && q < client.top;
                if (inner) {
                    RECT in{q + 1, q + 1, window.right - q - 1, window.bottom - q - 1};
                    HRGN ring = CreateRectRgnIndirect(&in);
                    HRGN items = CreateRectRgnIndirect(&client);
                    CombineRgn(ring, ring, items, RGN_DIFF);
                    SetDCBrushColor(dc, MenuRgb(ground));
                    FillRgn(dc, ring, (HBRUSH)GetStockObject(DC_BRUSH));
                    DeleteObject(ring);
                    DeleteObject(items);
                }
                for (int pass : {1, 2}) {
                    FrameLine(dc, 0, 0, window.right, true, at, size, band, band, false, true, pass);
                    FrameLine(dc, 0, window.bottom - 1, window.right, true, at, size, band, band, true, false, pass);
                    FrameLine(dc, 0, 0, window.bottom, false, at, size, band, band, false, true, pass);
                    FrameLine(dc, window.right - 1, 0, window.bottom, false, at, size, band, band, true, false, pass);
                    if (!inner) continue;
                    int w = window.right - 2 * q, h = window.bottom - 2 * q;
                    FrameLine(dc, q, q, w, true, at, size, band, ground, true, true, pass);
                    FrameLine(dc, q, window.bottom - 1 - q, w, true, at, size, ground, band, true, true, pass);
                    FrameLine(dc, q, q, h, false, at, size, band, ground, true, true, pass);
                    FrameLine(dc, window.right - 1 - q, q, h, false, at, size, ground, band, true, true, pass);
                }
            } else {
                SetDCBrushColor(dc, MenuRgb(g_menuLook.edge));
                FrameRect(dc, &window, (HBRUSH)GetStockObject(DC_BRUSH));
            }
            DeleteObject(frame);
            DeleteObject(inside);
            if (msg == WM_NCPAINT) ReleaseDC(hwnd, dc);
            return result;
        }
        case WM_NCDESTROY:
            RemoveWindowSubclass(hwnd, StyledMenuWindowProc, 0);
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MenuMotionProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR);

// Whether the menu windows made now are drawn here (TrackStyledMenu), or only animated (a native menu).
MENU_STYLE_STORAGE bool g_menuWindowsStyled = true;

LRESULT CALLBACK StyledMenuHookProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        auto* call = (CWPSTRUCT*)lParam;
        if (call->message == WM_CREATE) {
            WCHAR cls[16];
            if (GetClassNameW(call->hwnd, cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"#32768")) {
                if (g_menuMotion.style) SetWindowSubclass(call->hwnd, MenuMotionProc, 0, 0);
                if (!g_menuWindowsStyled) return CallNextHookEx(nullptr, code, wParam, lParam);
                SetWindowSubclass(call->hwnd, StyledMenuWindowProc, 0, 0);
                DWORD square = 1;  // DWMWCP_DONOTROUND
                DwmSetWindowAttribute(call->hwnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &square, sizeof(square));
                // No border from Windows: the frame drawn here is the edge (both together made it 2 px).
                COLORREF border = 0xFFFFFFFE;  // DWMWA_COLOR_NONE
                DwmSetWindowAttribute(call->hwnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

// A plain-colour theme part's colour at a point (0xRRGGBB), drawn small.
uint32_t ThemePartPixel(HTHEME theme, int part, int x, int y, uint32_t fallback) {
    constexpr int kSize = 9;
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
        if (IsThemePartDefined(theme, part, 0) && SUCCEEDED(DrawThemeBackground(theme, dc, part, 0, &r, nullptr))) {
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

// TrackPopupMenuEx with the style. The menu keeps its commands and returns
// the same; its items are restored before this returns. A foreign menu's
// submenus fill in as they open: pass WM_INITMENUPOPUP to its owner, then
// StyleMenuItems(submenu, true).
// Takes a native menu's icon bitmaps back off its items.
void UnNativeMenuItems(HMENU menu) {
    int count = GetMenuItemCount(menu);
    for (int i = 0; i < count; i++) {
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_SUBMENU | MIIM_BITMAP;
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        if (info.hSubMenu) UnNativeMenuItems(info.hSubMenu);
        if (!info.hbmpItem || std::find(g_nativeMenuBitmaps.begin(), g_nativeMenuBitmaps.end(), info.hbmpItem) ==
                                  g_nativeMenuBitmaps.end())
            continue;
        MENUITEMINFOW set{sizeof(set)};
        set.fMask = MIIM_BITMAP;
        set.hbmpItem = nullptr;
        SetMenuItemInfoW(menu, i, TRUE, &set);
    }
}

// The theme's menu ground and edge, sampled once per theme (reset when the theme reloads).
MENU_STYLE_STORAGE HTHEME g_menuSampled = nullptr;
MENU_STYLE_STORAGE uint32_t g_menuSampledGround = 0, g_menuSampledEdge = 0;

// A menu stays on one screen: the one "from" is on (what opened it: a flyout, the taskbar, an icon), or
// else the one under its point. A point on or past that screen's edge is brought back inside, and with
// "from" the menu doesn't cover it: Windows shows it on the other side when there's no room on this one
// (TPM_VERTICAL in flags: above or below first; else left or right first).
UINT TrackMenuOnScreen(HMENU menu, UINT flags, int x, int y, HWND owner, RECT const* from) {
    POINT pt{x, y};
    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(from ? MonitorFromRect(from, MONITOR_DEFAULTTONEAREST) : MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST),
                    &monitor);
    RECT const& m = monitor.rcMonitor;
    x = std::clamp(x, (int)m.left, (int)m.right - 1);
    y = std::clamp(y, (int)m.top, (int)m.bottom - 1);
    TPMPARAMS params{sizeof(params)};
    if (from) IntersectRect(&params.rcExclude, from, &m);
    // Submenus open the moment the pointer is on them: Windows' hover delay (400 ms unless changed) is 0 while one
    // of these menus is open, and put back after (not saved, so nothing else keeps it). Set to open on a click,
    // the delay is a minute instead, so hovering never opens one.
    constexpr PCWSTR kFlyoutClick = L"taskbar.menuFlyoutClick";  // (through a variable: another part's setting)
    UINT want = g_menuFlyoutClick || Wh_GetIntSetting(kFlyoutClick) ? 60000 : 0;
    static MENU_STYLE_STORAGE int open;
    UINT delay = 0;
    bool changed = !open++ && SystemParametersInfoW(SPI_GETMENUSHOWDELAY, 0, &delay, 0) && delay != want &&
                   SystemParametersInfoW(SPI_SETMENUSHOWDELAY, want, nullptr, 0);
    UINT result = TrackPopupMenuEx(menu, flags, x, y, owner, from && !IsRectEmpty(&params.rcExclude) ? &params : nullptr);
    if (changed) SystemParametersInfoW(SPI_SETMENUSHOWDELAY, delay, nullptr, 0);
    open--;
    return result;
}

// StartAllBack's menu look, as the taskbar gives its menus with a StartAllBack style and its bar
// background (taskbar-host.cpp, RunMenu): framed, in the icon title font, in capitals when the Start
// menu has them, with its item text as measured from StartAllBack's Bouquet menu.
void FrameLikeStartAllBack(MenuLook& look, HFONT iconTitleFont, bool caps) {
    if (iconTitleFont) look.font = iconTitleFont;
    look.frame = true;
    look.caps = caps;
    look.text = 0x5A9A7D;
    // The frame as set in the taskbar (read by full names through variables, so the build doesn't file them
    // under the part reading them): the gap between the lines, their colours, their glow.
    constexpr PCWSTR kGap = L"taskbar.menuFrameGap", kColors = L"taskbar.menuFrameColors", kGlow = L"taskbar.menuFrameGlow";
    int gap = Wh_GetIntSetting(kGap);
    look.frameGap = gap > 0 ? std::clamp(gap, 1, 16) : 6;
    look.frameGlow = std::clamp(Wh_GetIntSetting(kGlow), 0, 100);
    look.frameStopCount = 0;
    PCWSTR colors = Wh_GetStringSetting(kColors);
    for (PCWSTR p = colors; p && *p && look.frameStopCount < 3;) {
        while (*p && *p != L'#') p++;
        if (!*p) break;
        wchar_t* end = nullptr;
        unsigned long rgb = wcstoul(p + 1, &end, 16);
        if (end - (p + 1) == 6) look.frameStops[look.frameStopCount++] = (uint32_t)rgb;
        p = end && end > p + 1 ? end : p + 1;
    }
    Wh_FreeStringSetting(colors);
}

// Whether the taskbar frames its menus so (its settings, read by their full names through variables,
// so the build doesn't file them under the part reading them): for the system icons' menus, so every
// menu from the bar looks the same. (The taskbar also leaves them unframed for a style with menus of
// its own, which can't be told from here.)
bool TaskbarFramesMenus() {
    constexpr PCWSTR kStyle = L"taskbar.style", kBackground = L"taskbar.barBackground", kLook = L"taskbar.menuLook";
    auto text = [](PCWSTR name) {
        PCWSTR value = Wh_GetStringSetting(name);
        std::wstring s = value ? value : L"";
        Wh_FreeStringSetting(value);
        return s;
    };
    std::wstring style = text(kStyle);
    return !style.empty() && style != L"none" && text(kBackground) != L"scheme" && text(kLook) != L"windows";
}

bool StartMenuCapitals() {
    constexpr PCWSTR kCaps = L"taskbar.startMenu.capitals";
    return Wh_GetIntSetting(kCaps) != 0;
}

// Whether every menu here is to look like Windows' own (the taskbar's "Menus" setting, read by its full
// name through a variable, so the build doesn't file it under the part reading it).
bool MenusLikeWindows() {
    constexpr PCWSTR kMenuLook = L"taskbar.menuLook";
    PCWSTR value = Wh_GetStringSetting(kMenuLook);
    bool windows = value && !wcscmp(value, L"windows");
    Wh_FreeStringSetting(value);
    return windows;
}

// Windows' menu font at this scale (made again when it changes).
HFONT WindowsMenuFont(int scale) {
    static HFONT font;
    static LOGFONTW made;
    NONCLIENTMETRICSW metrics{sizeof(metrics)};
    if (!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, 96 * std::max(1, scale)))
        return font;
    if (!font || memcmp(&made, &metrics.lfMenuFont, sizeof(made)) != 0) {
        if (font) DeleteObject(font);
        made = metrics.lfMenuFont;
        font = CreateFontIndirectW(&made);
    }
    return font;
}

// Windows' own classic menus, as other programs show them here: the menu colours and font set in
// Windows, flat and square, its border colour (COLOR_BTNSHADOW) or, where that's the ground and so
// shows nothing, the caller's edge. System Text Color's colour goes over text it would recolour in those programs (each
// channel at its White threshold or more); the arrow is a glyph it leaves as it is. Taken fresh at
// each menu, so a changed colour shows on the next one. Icons stay as the caller set them.
void LookLikeWindowsMenus(MenuLook& look) {
    auto sys = [](int index) {
        COLORREF c = GetSysColor(index);
        return (uint32_t)((GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c));
    };
    constexpr PCWSTR kTextOn = L"textColor.enabled", kTextColor = L"textColor.textColor", kThreshold = L"textColor.threshold";
    bool recolors = false;
    uint32_t recolor = 0;
    int threshold = 200;
    if (Wh_GetIntSetting(kTextOn)) {
        PCWSTR value = Wh_GetStringSetting(kTextColor);
        std::wstring hex = value ? value : L"";
        Wh_FreeStringSetting(value);
        std::erase_if(hex, [](wchar_t c) { return c == L'"' || c == L'#' || iswspace(c); });
        if (hex.size() == 6 && hex.find_first_not_of(L"0123456789abcdefABCDEF") == std::wstring::npos) {
            recolor = (uint32_t)wcstoul(hex.c_str(), nullptr, 16);
            recolors = true;
        }
        threshold = std::clamp(Wh_GetIntSetting(kThreshold), 0, 255);
    }
    auto text = [&](int index) {
        uint32_t c = sys(index);
        bool bright = (int)((c >> 16) & 0xFF) >= threshold && (int)((c >> 8) & 0xFF) >= threshold && (int)(c & 0xFF) >= threshold;
        return recolors && bright ? recolor : c;
    };
    look.ground = sys(COLOR_MENU);
    look.text = text(COLOR_MENUTEXT);
    look.hover = sys(COLOR_MENUHILIGHT);
    look.hoverText = text(COLOR_HIGHLIGHTTEXT);
    look.dim = text(COLOR_GRAYTEXT);
    look.line = sys(COLOR_3DHILIGHT);
    if (sys(COLOR_BTNSHADOW) != look.ground) look.edge = sys(COLOR_BTNSHADOW);
    look.accent = look.text;
    look.arrow = sys(COLOR_MENUTEXT);
    if (HFONT font = WindowsMenuFont(look.scale)) look.font = font;
    look.theme = nullptr;
    look.native = look.classic = look.ownText = look.frame = look.caps = false;
    look.windows = true;
}

// A menu taller than its screen: Windows would make it scroll, with its arrows where a frame is
// painted (so they don't show, and the bottom items and the frame's last line go missing). Instead
// it's split into columns side by side, as even as the items allow, each framed like a whole menu
// and ending together. Only menus whose items are all styled here.
void FitStyledMenu(HMENU menu, int x, int y, RECT const* from) {
    int count = GetMenuItemCount(menu);
    if (count < 2) return;
    std::vector<StyledMenuItem*> items;
    std::vector<int> heights;
    long long sum = 0;
    for (int i = 0; i < count; i++) {
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_DATA | MIIM_FTYPE;
        if (!GetMenuItemInfoW(menu, i, TRUE, &info) || (info.fType & (MFT_MENUBREAK | MFT_MENUBARBREAK))) return;
        StyledMenuItem* item = StyledItemOf(info.dwItemData);
        if (!item) return;
        items.push_back(item);
        heights.push_back((int)StyledItemHeight(item));
        sum += heights.back();
    }
    int s = g_menuLook.scale;
    POINT pt{x, y};
    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(from ? MonitorFromRect(from, MONITOR_DEFAULTTONEAREST) : MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &monitor);
    int room = (monitor.rcWork.bottom - monitor.rcWork.top) - 2 * 3 * s - 8 * s;  // (the menu's own border, and a margin)
    int pads = g_menuLook.frame ? 24 * s : 0;                                     // a frame's room above and below each column
    if (sum + pads <= room) return;
    int tallest = *std::max_element(heights.begin(), heights.end());
    if (tallest + pads > room) return;  // (no split would fit)
    // The fewest columns that fit, each as near the same height as the items allow.
    std::vector<int> starts;
    for (int columns = 2; columns <= count; columns++) {
        int target = std::max<int>(tallest, (int)((sum + columns - 1) / columns));
        int limit = std::min(room - pads, target + tallest - 1);
        starts = {0};
        int height = 0;
        for (int i = 0; i < count; i++) {
            if (height && height + heights[i] > limit) {
                starts.push_back(i);
                height = 0;
            }
            height += heights[i];
        }
        if ((int)starts.size() <= columns) break;
    }
    if (starts.size() < 2) return;
    starts.push_back(count);
    int longest = 0;
    std::vector<int> columnHeights;
    for (size_t c = 0; c + 1 < starts.size(); c++) {
        int height = 0;
        for (int i = starts[c]; i < starts[c + 1]; i++) height += heights[i];
        columnHeights.push_back(height);
        longest = std::max(longest, height);
    }
    for (size_t c = 0; c + 1 < starts.size(); c++) {
        int first = starts[c], last = starts[c + 1] - 1;
        if (c > 0) {
            MENUITEMINFOW set{sizeof(set)};
            set.fMask = MIIM_FTYPE;
            GetMenuItemInfoW(menu, first, TRUE, &set);
            set.fType |= MFT_MENUBREAK;
            SetMenuItemInfoW(menu, first, TRUE, &set);
        }
        if (g_menuLook.frame) {
            items[first]->padTop = true;
            items[last]->padBottom = true;
            items[last]->extraBottom = longest - columnHeights[c];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// The mouse wheel over a menu taller than its screen (Windows only scrolls one with its arrows, while
// they're pointed at): each notch becomes Up or Down presses that select the new bottom or top item, so
// the menu scrolls 3 items a notch, never wrapping (as Tourne Mod Tray 1.0.1 does). A low-level mouse
// hook takes the wheel over the menu wherever Windows would send it (the menu's thread isn't always in
// front); a message filter hook does the same for wheel messages that reach the menu, should the first
// not be allowed. Over a menu with a submenu open, the submenu closes first.

MENU_STYLE_STORAGE HHOOK g_menuWheelFilter = nullptr;
// The low-level hook lives on a thread of its own while a menu is open: on the menu's thread every mouse move
// anywhere waited for the menu to finish painting (the pointer hitched over items).
MENU_STYLE_STORAGE HANDLE g_menuWheelHookThread = nullptr, g_menuWheelHookStop = nullptr;
MENU_STYLE_STORAGE int g_menuWheelDepth = 0;  // TrackStyledMenu calls open on this thread
MENU_STYLE_STORAGE int g_menuWheelDelta = 0;  // the part of a notch not used yet (touchpads)
MENU_STYLE_STORAGE DWORD g_menuWheelThread = 0;
constexpr int kMenuWheelItems = 3;
constexpr UINT kMnGetHMenu = 0x01E1;  // MN_GETHMENU: the menu a menu window shows
// Where the last turn left a menu, until its key presses have been taken (turns come faster than the menu
// takes them): its first item showing and the item selected.
struct MenuWheelPending {
    HWND window = nullptr;
    int top = -1, hilite = -1;
};
MENU_STYLE_STORAGE MenuWheelPending g_menuWheelPending;

struct MenuWindowsFound {
    std::vector<HWND> windows;  // this thread's menu windows showing, the topmost (the deepest open) first
};

BOOL CALLBACK FindMenuWindows(HWND hwnd, LPARAM lp) {
    WCHAR cls[16];
    if (IsWindowVisible(hwnd) && GetClassNameW(hwnd, cls, ARRAYSIZE(cls)) && !wcscmp(cls, L"#32768"))
        ((MenuWindowsFound*)lp)->windows.push_back(hwnd);
    return TRUE;
}

// The first or last item showing (between the arrows), by position; -1 if none.
int MenuItemAtEdge(HWND menuWnd, HMENU menu, bool top) {
    RECT rc;
    GetWindowRect(menuWnd, &rc);
    int x = (rc.left + rc.right) / 2;
    for (int d = 1; d < 80; d += 2) {
        POINT pt{x, top ? rc.top + d : rc.bottom - 1 - d};
        int i = MenuItemFromPoint(nullptr, menu, pt);
        if (i >= 0) return i;
    }
    return -1;
}

bool MenuItemSelectable(HMENU menu, int i) {
    UINT state = GetMenuState(menu, i, MF_BYPOSITION);
    return state != (UINT)-1 && !(state & MF_SEPARATOR);
}

// notches > 0: up (the wheel turned away), < 0: down. Over the menu window that's at the point.
void MenuWheelScroll(POINT pt, int notches) {
    MenuWindowsFound found;
    EnumThreadWindows(g_menuWheelThread, FindMenuWindows, (LPARAM)&found);
    size_t at = 0;
    for (; at < found.windows.size(); at++) {
        RECT rc;
        if (GetWindowRect(found.windows[at], &rc) && PtInRect(&rc, pt)) break;
    }
    if (at == found.windows.size()) return;
    HWND menuWnd = found.windows[at];
    HMENU menu = (HMENU)SendMessageW(menuWnd, kMnGetHMenu, 0, 0);
    int count = menu ? GetMenuItemCount(menu) : 0;
    int first = MenuItemAtEdge(menuWnd, menu, true), last = MenuItemAtEdge(menuWnd, menu, false);
    if (count <= 0 || first < 0 || last < first || (first == 0 && last == count - 1)) return;  // (all of it shows)
    int shown = last - first + 1;
    int top = std::clamp(first - notches * kMenuWheelItems, 0, count - shown);
    if (top == first) return;
    int hilite = -1;
    for (int i = 0; i < count && hilite < 0; i++)
        if (GetMenuState(menu, i, MF_BYPOSITION) & MF_HILITE) hilite = i;
    // The last turn's presses not taken yet: on from where they'll leave it.
    auto& pending = g_menuWheelPending;
    MSG queued;
    if (pending.window == menuWnd && pending.hilite >= 0 && PeekMessageW(&queued, menuWnd, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE)) {
        first = pending.top;
        hilite = pending.hilite;
        top = std::clamp(first - notches * kMenuWheelItems, 0, count - shown);
        if (top == first) return;
    }
    // The keys go to the deepest menu open: the ones over this close first (each Esc closes one).
    for (size_t i = 0; i < at; i++) PostMessageW(menuWnd, WM_KEYDOWN, VK_ESCAPE, 0);
    // The menu scrolls to keep the selected item showing: the new bottom item going down, the new top going up.
    UINT key = VK_DOWN;
    int steps = 0, to = hilite;
    if (hilite < 0 || top > first) {
        for (int i = hilite < 0 ? 0 : hilite + 1; i <= top + shown - 1; i++)  // (with none selected, Down picks the first)
            if (MenuItemSelectable(menu, i)) steps++, to = i;
    } else {
        key = VK_UP;
        for (int i = hilite - 1; i >= top; i--)
            if (MenuItemSelectable(menu, i)) steps++, to = i;
    }
    for (int i = 0; i < steps; i++) PostMessageW(menuWnd, WM_KEYDOWN, key, 0);
    Wh_Log(L"wheel %d: showing %d..%d of %d, selected %d, %d x %ls", notches, first, first + shown - 1, count, hilite, steps,
           key == VK_DOWN ? L"Down" : L"Up");
    pending = {menuWnd, top, steps ? to : -1};
}

void MenuWheelBy(POINT pt, int delta) {
    g_menuWheelDelta += delta;
    int notches = g_menuWheelDelta / WHEEL_DELTA;
    g_menuWheelDelta -= notches * WHEEL_DELTA;
    if (notches) MenuWheelScroll(pt, notches);
}

// Over a menu window of this thread.
bool OverMenuWindow(POINT pt) {
    MenuWindowsFound found;
    EnumThreadWindows(g_menuWheelThread, FindMenuWindows, (LPARAM)&found);
    for (HWND w : found.windows) {
        RECT rc;
        if (GetWindowRect(w, &rc) && PtInRect(&rc, pt)) return true;
    }
    return false;
}

// On the hook's own thread (g_menuWheelThread there is the menu's): a turn over a menu window goes to that
// window as a wheel message, which MenuWheelFilterProc takes on the menu's thread. Moves pass at once.
LRESULT CALLBACK MenuWheelLLProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && wp == WM_MOUSEWHEEL) {
        auto* ms = (MSLLHOOKSTRUCT*)lp;
        MenuWindowsFound found;
        EnumThreadWindows(g_menuWheelThread, FindMenuWindows, (LPARAM)&found);
        for (HWND w : found.windows) {
            RECT rc;
            if (!GetWindowRect(w, &rc) || !PtInRect(&rc, ms->pt)) continue;
            PostMessageW(w, WM_MOUSEWHEEL, MAKEWPARAM(0, HIWORD(ms->mouseData)), MAKELPARAM(ms->pt.x, ms->pt.y));
            return 1;  // (taken: nothing else scrolls)
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

struct MenuWheelHookStart {
    DWORD menuThread;
    HANDLE stop;
};

DWORD WINAPI MenuWheelHookThread(void* p) {
    auto start = *(MenuWheelHookStart*)p;
    delete (MenuWheelHookStart*)p;
    g_menuWheelThread = start.menuThread;  // (this thread's copy, where storage is per thread)
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&MenuWheelLLProc, &self);
    HHOOK hook = SetWindowsHookExW(WH_MOUSE_LL, MenuWheelLLProc, self, 0);
    while (MsgWaitForMultipleObjects(1, &start.stop, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0) {
        MSG m;
        while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
    }
    if (hook) UnhookWindowsHookEx(hook);
    return 0;
}

LRESULT CALLBACK MenuWheelFilterProc(int code, WPARAM wp, LPARAM lp) {
    if (code == MSGF_MENU && g_menuWheelDepth) {
        MSG* m = (MSG*)lp;
        if (m->message == WM_MOUSEWHEEL) {
            POINT pt{(short)LOWORD(m->lParam), (short)HIWORD(m->lParam)};
            MenuWheelBy(pt, GET_WHEEL_DELTA_WPARAM(m->wParam));
            return TRUE;
        }
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

void MenuWheelStart() {
    if (g_menuWheelDepth++) return;
    g_menuWheelDelta = 0;
    g_menuWheelPending = {};
    g_menuWheelThread = GetCurrentThreadId();
    g_menuWheelFilter = SetWindowsHookExW(WH_MSGFILTER, MenuWheelFilterProc, nullptr, GetCurrentThreadId());
    g_menuWheelHookStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (g_menuWheelHookStop)
        g_menuWheelHookThread = CreateThread(nullptr, 0, MenuWheelHookThread, new MenuWheelHookStart{g_menuWheelThread, g_menuWheelHookStop}, 0, nullptr);
}

void MenuWheelStop() {
    if (--g_menuWheelDepth) return;
    if (g_menuWheelFilter) UnhookWindowsHookEx(g_menuWheelFilter);
    g_menuWheelFilter = nullptr;
    if (g_menuWheelHookStop) SetEvent(g_menuWheelHookStop);
    if (g_menuWheelHookThread) {
        WaitForSingleObject(g_menuWheelHookThread, 500);  // (it only unhooks)
        CloseHandle(g_menuWheelHookThread);
    }
    if (g_menuWheelHookStop) CloseHandle(g_menuWheelHookStop);
    g_menuWheelHookThread = g_menuWheelHookStop = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Menus opening with a short animation (g_menuMotion): each menu window, as it's first shown, fades in,
// slides 10 px in (and fades) from the side away from where it opens, or unfolds from that side, over
// g_menuMotion.ms on the motion engine's curve. Windows' own menu fade stays off (TPM_NOANIMATION).

constexpr UINT_PTR kMenuMotionTimer = 0x7E51;

struct MenuMotionState {
    HWND window = nullptr;
    DWORD start = 0;
    POINT at{};  // where it ends up
    SIZE size{};
    int dy = 0;  // where it starts, above (< 0) or below (> 0) that
    bool up = false;  // it opens upwards (unfolds from the bottom)
};
MENU_STYLE_STORAGE std::vector<MenuMotionState>* g_menuMotions = nullptr;

MenuMotionState* MenuMotionOf(HWND hwnd) {
    if (!g_menuMotions) return nullptr;
    for (auto& m : *g_menuMotions)
        if (m.window == hwnd) return &m;
    return nullptr;
}

void MenuMotionDrop(HWND hwnd) {
    if (g_menuMotions) std::erase_if(*g_menuMotions, [&](MenuMotionState const& m) { return m.window == hwnd; });
}

// One step: t is the eased progress (may pass 1 on the way).
void MenuMotionApply(MenuMotionState const& m, double t, bool done) {
    int style = g_menuMotion.style;
    if (style == 1 || style == 2) {
        int alpha = done ? 255 : std::clamp((int)(t * 255 + 0.5), 0, 255);
        SetLayeredWindowAttributes(m.window, 0, (BYTE)alpha, LWA_ALPHA);
    }
    if (style == 2) {
        int y = m.at.y + (done ? 0 : (int)std::lround(m.dy * (1 - t)));
        SetWindowPos(m.window, nullptr, m.at.x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
    if (style == 3) {
        if (done) {
            SetWindowRgn(m.window, nullptr, TRUE);
        } else {
            int h = std::clamp((int)std::lround(m.size.cy * t), 1, (int)m.size.cy);
            HRGN rgn = m.up ? CreateRectRgn(0, m.size.cy - h, m.size.cx, m.size.cy) : CreateRectRgn(0, 0, m.size.cx, h);
            SetWindowRgn(m.window, rgn, TRUE);  // (the window owns the region now)
        }
    }
}

double MenuMotionEase(double p) {
    if (g_menuMotion.ease) return g_menuMotion.ease(p);
    double q = 1 - p;
    return 1 - q * q * q;  // (an ease-out)
}

LRESULT CALLBACK MenuMotionProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_WINDOWPOSCHANGING: {
            auto* pos = (WINDOWPOS*)lParam;
            if (!(pos->flags & SWP_SHOWWINDOW) || MenuMotionOf(hwnd) || !g_menuMotion.style || g_menuMotion.ms < 1) break;
            // Only the menu itself moves in; its submenus are there at once (a submenu that eases in reads as a hang).
            {
                bool another = false;
                for (HWND w = FindWindowExW(nullptr, nullptr, L"#32768", nullptr); w && !another; w = FindWindowExW(nullptr, w, L"#32768", nullptr))
                    another = w != hwnd && IsWindowVisible(w) && GetWindowThreadProcessId(w, nullptr) == GetCurrentThreadId();
                if (another) break;
            }
            RECT r;
            GetWindowRect(hwnd, &r);
            MenuMotionState m;
            m.window = hwnd;
            m.at = (pos->flags & SWP_NOMOVE) ? POINT{r.left, r.top} : POINT{pos->x, pos->y};
            m.size = (pos->flags & SWP_NOSIZE) ? SIZE{r.right - r.left, r.bottom - r.top} : SIZE{pos->cx, pos->cy};
            POINT cursor;
            GetCursorPos(&cursor);
            m.up = m.at.y + m.size.cy <= cursor.y + 2;  // (it ends above the pointer)
            m.dy = (m.up ? 10 : -10) * std::max(1, g_menuLook.scale);
            m.start = GetTickCount();
            if (!g_menuMotions) g_menuMotions = new std::vector<MenuMotionState>;
            g_menuMotions->push_back(m);
            // Its first look, before it shows.
            if (g_menuMotion.style == 1 || g_menuMotion.style == 2) {
                SetWindowLongPtrW(hwnd, GWL_EXSTYLE, GetWindowLongPtrW(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
                SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
            }
            if (g_menuMotion.style == 2) {
                pos->x = m.at.x;
                pos->y = m.at.y + m.dy;
                pos->flags &= ~SWP_NOMOVE;
            }
            if (g_menuMotion.style == 3) {
                HRGN rgn = m.up ? CreateRectRgn(0, m.size.cy - 1, m.size.cx, m.size.cy) : CreateRectRgn(0, 0, m.size.cx, 1);
                SetWindowRgn(hwnd, rgn, FALSE);
            }
            SetTimer(hwnd, kMenuMotionTimer, 10, nullptr);
            break;
        }
        case WM_TIMER: {
            if (wParam != kMenuMotionTimer) break;
            MenuMotionState* m = MenuMotionOf(hwnd);
            if (!m) {
                KillTimer(hwnd, kMenuMotionTimer);
                return 0;
            }
            double p = (GetTickCount() - m->start) / std::max(1.0, g_menuMotion.ms);
            bool done = p >= 1;
            MenuMotionApply(*m, done ? 1 : MenuMotionEase(p), done);
            if (done) KillTimer(hwnd, kMenuMotionTimer);
            return 0;
        }
        case WM_NCDESTROY:
            KillTimer(hwnd, kMenuMotionTimer);
            MenuMotionDrop(hwnd);
            RemoveWindowSubclass(hwnd, MenuMotionProc, 0);
            break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
// Items taken out of a shell menu, or moved into a "More options" submenu at its end, by their label
// (without its & and what follows a tab, any case, a trailing "..." optional) or, where the caller can
// tell, their verb ("cut", "properties", "openas"). Only the menu's own items, not its submenus'. Separators
// left doubled, first or last go too. Items can also be renamed, and new ones added that run a command (the
// menu file, menus.conf, below). UndoMenuRules puts everything back where it was, for a caller that keeps the
// menu (Explorer's).

struct MenuRules {
    std::vector<std::wstring> hide, more;  // lower case
    std::wstring moreLabel = L"More options";
    struct Rename {
        std::wstring from, to;  // from: lower case
    };
    std::vector<Rename> rename;
    struct Add {
        std::wstring label, command;  // label "-": a separator
        bool top = false;             // first in the menu, else last (before "More options")
    };
    std::vector<Add> add;
    bool Any() const { return !hide.empty() || !more.empty() || !rename.empty() || !add.empty(); }
};

// The ids of added items: above the shell's (1 to 0x7FFF) and the desktop menu's own (0x8000 on).
constexpr UINT kMenuAddFirst = 0xFE00, kMenuAddLast = 0xFEFF;

// "Cast to Device, Give access to" -> {"cast to device", "give access to"}.
std::vector<std::wstring> MenuRuleList(std::wstring const& text) {
    std::vector<std::wstring> out;
    size_t start = 0;
    while (start <= text.size()) {
        size_t comma = text.find(L',', start);
        std::wstring item = text.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        item.erase(0, item.find_first_not_of(L" \t\""));
        size_t end = item.find_last_not_of(L" \t\"");
        item.erase(end == std::wstring::npos ? 0 : end + 1);
        for (auto& c : item) c = (wchar_t)towlower(c);
        if (!item.empty()) out.push_back(item);
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return out;
}

// A label as the rules compare it.
std::wstring MenuRuleLabel(std::wstring const& text) {
    std::wstring out;
    for (size_t i = 0; i < text.size() && text[i] != L'\t'; i++) {
        if (text[i] == L'&') {
            if (i + 1 < text.size() && text[i + 1] == L'&') out += L'&', i++;
            continue;
        }
        out += (wchar_t)towlower(text[i]);
    }
    while (!out.empty() && (iswspace(out.back()) || out.back() == L'.' || out.back() == 0x2026)) out.pop_back();
    out.erase(0, out.find_first_not_of(L" \t"));
    return out;
}

struct MenuRulesUndo {
    HMENU menu = nullptr, more = nullptr;
    bool separator = false;  // a separator was added before the submenu
    struct Renamed {
        int at;  // where it was, before anything was taken out
        std::wstring text;
    };
    std::vector<Renamed> renamed;
    bool added = false;
    std::vector<std::wstring> commands;  // the added items', by id - kMenuAddFirst (kept after the undo: RunMenuRule)
    struct Taken {
        int at;  // where it was
        MENUITEMINFOW info;
        std::wstring text;
    };
    std::vector<Taken> taken;  // in the order they were
};

// verbOf (may be null) gives an item's verb from its command id.
template <class VerbOf>
MenuRulesUndo ApplyMenuRules(HMENU menu, MenuRules const& rules, VerbOf&& verbOf) {
    MenuRulesUndo undo;
    undo.menu = menu;
    if (!rules.Any() || !menu) return undo;
    // (an entry ending " (greyed)" only takes the item while it's greyed out, as Paste with nothing to paste)
    auto listed = [](std::vector<std::wstring> const& list, std::wstring const& a, std::wstring const& b, bool greyed) {
        constexpr std::wstring_view kGreyed = L" (greyed)";
        for (auto const& entry : list) {
            std::wstring_view x = entry;
            if (x.size() > kGreyed.size() && x.substr(x.size() - kGreyed.size()) == kGreyed) {
                if (!greyed) continue;
                x.remove_suffix(kGreyed.size());
            }
            if ((!a.empty() && x == a) || (!b.empty() && x == b)) return true;
        }
        return false;
    };
    // What each item is: 0 stays, 1 goes, 2 moves.
    int count = GetMenuItemCount(menu);
    std::vector<int> fate(std::max(0, count), 0);
    std::vector<bool> separator(std::max(0, count), false);
    for (int i = 0; i < count; i++) {
        WCHAR text[256] = L"";
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_SUBMENU | MIIM_STATE;
        info.dwTypeData = text;
        info.cch = ARRAYSIZE(text);
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        separator[i] = info.fType & MFT_SEPARATOR;
        bool greyed = info.fState & MFS_DISABLED;
        if (separator[i]) continue;
        std::wstring label = MenuRuleLabel(text), verb;
        if (!info.hSubMenu || info.wID) verb = verbOf(info.wID);
        for (auto& c : verb) c = (wchar_t)towlower(c);
        if (listed(rules.hide, label, verb, greyed)) fate[i] = 1;
        else if (listed(rules.more, label, verb, greyed)) fate[i] = 2;
        for (auto const& r : rules.rename) {
            if (r.from != label && (verb.empty() || r.from != verb)) continue;
            // (what follows a tab, the shortcut shown on the right, stays)
            std::wstring now = text, to = r.to;
            if (size_t tab = now.find(L'\t'); tab != std::wstring::npos) to += now.substr(tab);
            MENUITEMINFOW set{sizeof(set)};
            set.fMask = MIIM_STRING;
            set.dwTypeData = (LPWSTR)to.c_str();
            if (SetMenuItemInfoW(menu, i, TRUE, &set)) undo.renamed.push_back({i, now});
            break;
        }
    }
    // The added ones, at the top or the end: separators round them where the menu has items.
    auto addItems = [&] {
        for (int pass = 0; pass < 2; pass++) {
            std::vector<MenuRules::Add const*> list;
            for (auto const& a : rules.add)
                if (a.top == (pass == 0)) list.push_back(&a);
            if (list.empty()) continue;
            int at = pass == 0 ? 0 : GetMenuItemCount(menu);
            auto insert = [&](MenuRules::Add const* a) {
                UINT id = kMenuAddFirst + (UINT)undo.commands.size();
                if (id > kMenuAddLast) return;
                undo.commands.push_back(a ? a->command : L"");
                MENUITEMINFOW info{sizeof(info)};
                info.fMask = MIIM_FTYPE | MIIM_ID | (a && a->label != L"-" ? MIIM_STRING : 0);
                info.fType = a && a->label != L"-" ? MFT_STRING : MFT_SEPARATOR;
                info.wID = id;
                info.dwTypeData = a ? (LPWSTR)a->label.c_str() : nullptr;
                if (InsertMenuItemW(menu, at, TRUE, &info)) at++, undo.added = true;
            };
            int n = GetMenuItemCount(menu);
            bool edge = pass == 0 ? n == 0 || (GetMenuState(menu, 0, MF_BYPOSITION) & MF_SEPARATOR)
                                  : n == 0 || (GetMenuState(menu, n - 1, MF_BYPOSITION) & MF_SEPARATOR);
            if (pass == 1 && !edge && list.front()->label != L"-") insert(nullptr);
            for (auto* a : list) insert(a);
            if (pass == 0 && !edge && list.back()->label != L"-") insert(nullptr);
        }
    };
    // Separators that would be first, last or doubled once the rest is gone.
    bool any = false;
    for (int f : fate) any |= f != 0;
    if (!any) {
        addItems();
        return undo;
    }
    int lastKept = -1;  // the last item staying in the menu, before i
    for (int i = 0; i < count; i++) {
        if (fate[i]) continue;
        if (separator[i] && (lastKept < 0 || separator[lastKept])) fate[i] = 1;
        else lastKept = i;
    }
    if (lastKept >= 0 && separator[lastKept]) fate[lastKept] = 1;
    // Taken out from the end, so the places of the rest stay as they were.
    std::vector<MenuRulesUndo::Taken> taken;
    for (int i = count - 1; i >= 0; i--) {
        if (!fate[i]) continue;
        MenuRulesUndo::Taken t{i, {sizeof(MENUITEMINFOW)}, {}};
        WCHAR text[256] = L"";
        t.info.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_STRING | MIIM_DATA | MIIM_BITMAP | MIIM_CHECKMARKS;
        t.info.dwTypeData = text;
        t.info.cch = ARRAYSIZE(text);
        if (!GetMenuItemInfoW(menu, i, TRUE, &t.info)) continue;
        t.text = text;
        if (!RemoveMenu(menu, i, MF_BYPOSITION)) continue;
        taken.insert(taken.begin(), t);
    }
    undo.taken = taken;
    addItems();
    // The moved ones, in their order, in a submenu at the end (after a separator).
    for (auto& t : undo.taken) {
        if (fate[t.at] != 2) continue;
        if (!undo.more) undo.more = CreatePopupMenu();
        MENUITEMINFOW info = t.info;
        info.dwTypeData = (LPWSTR)t.text.c_str();
        InsertMenuItemW(undo.more, GetMenuItemCount(undo.more), TRUE, &info);
    }
    if (undo.more) {
        int n = GetMenuItemCount(menu);
        if (n > 0 && !(GetMenuState(menu, n - 1, MF_BYPOSITION) & MF_SEPARATOR)) {
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            undo.separator = true;
        }
        AppendMenuW(menu, MF_POPUP | MF_STRING, (UINT_PTR)undo.more, rules.moreLabel.empty() ? L"More options" : rules.moreLabel.c_str());
    }
    return undo;
}

void UndoMenuRules(MenuRulesUndo& undo) {
    if (!undo.menu) return;
    if (undo.added) {
        for (int i = GetMenuItemCount(undo.menu) - 1; i >= 0; i--) {
            UINT id = GetMenuItemID(undo.menu, i);
            if (id >= kMenuAddFirst && id <= kMenuAddLast) RemoveMenu(undo.menu, i, MF_BYPOSITION);
        }
        undo.added = false;
    }
    auto renameBack = [&] {
        for (auto& r : undo.renamed) {
            MENUITEMINFOW set{sizeof(set)};
            set.fMask = MIIM_STRING;
            set.dwTypeData = (LPWSTR)r.text.c_str();
            SetMenuItemInfoW(undo.menu, r.at, TRUE, &set);
        }
        undo.renamed.clear();
    };
    if (undo.taken.empty()) return renameBack();
    if (undo.more) {
        // The submenu and the separator added before it.
        for (int i = GetMenuItemCount(undo.menu) - 1; i >= 0; i--) {
            if (GetSubMenu(undo.menu, i) != undo.more) continue;
            RemoveMenu(undo.menu, i, MF_BYPOSITION);
            if (undo.separator && i > 0) RemoveMenu(undo.menu, i - 1, MF_BYPOSITION);
            break;
        }
        while (GetMenuItemCount(undo.more) > 0) RemoveMenu(undo.more, 0, MF_BYPOSITION);  // (theirs again: not destroyed)
        DestroyMenu(undo.more);
        undo.more = nullptr;
    }
    for (auto& t : undo.taken) {
        MENUITEMINFOW info = t.info;
        info.dwTypeData = (LPWSTR)t.text.c_str();
        InsertMenuItemW(undo.menu, std::min(t.at, GetMenuItemCount(undo.menu)), TRUE, &info);
    }
    undo.taken.clear();
    renameBack();
}

////////////////////////////////////////////////////////////////////////////////
// The menu file, %APPDATA%\Tourne\Desktop\menus.conf: what each menu shows, a line each, under sections
// that say which menus they're for. Read again whenever it changes; what didn't make sense is written to
// menus.log beside it.
//
//   # a note
//   hide Cast to Device              (a label, any case, or a verb: properties, openas; "Paste (greyed)" too)
//   more Give access to              (into the "More options" submenu at the end)
//   more-label More...
//   rename Copy as path -> Copy path
//   add Open in Terminal = wt.exe -d %folder%
//   add top Edit in Notepad = notepad.exe %file%
//   add separator
//   [when menu desktop, files *.txt *.md]   (every condition holds; ! before one turns it round)
//   [all]                                   (back to every menu)
//
// Conditions: menu NAME... (desktop, taskbar, explorer, dialog, windows), app NAME.exe..., files PATTERN...
// (every chosen item's name matches one), folder (every chosen item is a folder), background (nothing
// chosen: the empty space), shift. Where a menu doesn't know what's chosen (Explorer's, through Tourne Menus),
// files, folder and background never hold, nor their opposites. In commands: %file% (the first chosen, quoted),
// %files% (all of them), %folder% (where the menu opened, or the first one's folder), and %VARIABLES%.

struct MenuContext {
    std::wstring menu;                // desktop, taskbar, explorer, dialog or windows
    std::wstring app;                 // the program's file name, lower case
    bool known = false;               // what's chosen is known
    std::vector<std::wstring> files;  // full paths
    std::wstring folder;
};

struct MenuFile {
    struct Line {
        std::wstring verb, item, value;
        bool top = false;
    };
    struct Section {
        std::vector<std::wstring> when;  // lower case, one condition each
        std::vector<Line> lines;
    };
    std::vector<Section> sections;
};

// "*.txt" against "notes.TXT".
bool MenuWild(wchar_t const* pattern, wchar_t const* text) {
    if (!*pattern) return !*text;
    if (*pattern == L'*') return MenuWild(pattern + 1, text) || (*text && MenuWild(pattern, text + 1));
    if (!*text) return false;
    if (*pattern != L'?' && towlower(*pattern) != towlower(*text)) return false;
    return MenuWild(pattern + 1, text + 1);
}

std::wstring MenuTrim(std::wstring x) {
    x.erase(0, x.find_first_not_of(L" \t"));
    size_t end = x.find_last_not_of(L" \t\r");
    x.erase(end == std::wstring::npos ? 0 : end + 1);
    return x;
}

MenuFile ParseMenuFile(std::wstring const& text, std::wstring* errors) {
    MenuFile file;
    file.sections.push_back({});
    int number = 0;
    size_t start = 0;
    auto error = [&](std::wstring const& why) {
        if (errors) *errors += L"line " + std::to_wstring(number) + L": " + why + L"\r\n";
    };
    while (start < text.size()) {
        size_t end = text.find(L'\n', start);
        std::wstring line = MenuTrim(text.substr(start, end == std::wstring::npos ? std::wstring::npos : end - start));
        start = end == std::wstring::npos ? text.size() : end + 1;
        number++;
        if (line.empty() || line[0] == L'#') continue;
        if (line[0] == L'[') {
            if (line.back() != L']') {
                error(L"a section needs its closing ]");
                continue;
            }
            std::wstring inner = MenuTrim(line.substr(1, line.size() - 2));
            for (auto& c : inner) c = (wchar_t)towlower(c);
            MenuFile::Section section;
            if (inner != L"all") {
                if (inner.compare(0, 5, L"when ")) {
                    error(L"a section is [when ...] or [all]");
                    continue;
                }
                for (auto const& c : MenuRuleList(inner.substr(5))) {
                    std::wstring key = c[0] == L'!' ? MenuTrim(c.substr(1)) : c;
                    key = key.substr(0, key.find(L' '));
                    if (key != L"menu" && key != L"app" && key != L"files" && key != L"folder" && key != L"background" &&
                        key != L"shift")
                        error(L"no condition \"" + key + L"\" (menu, app, files, folder, background, shift)");
                    else section.when.push_back(c);
                }
            }
            file.sections.push_back(section);
            continue;
        }
        size_t space = line.find_first_of(L" \t");
        MenuFile::Line l;
        l.verb = line.substr(0, space);
        for (auto& c : l.verb) c = (wchar_t)towlower(c);
        std::wstring rest = space == std::wstring::npos ? L"" : MenuTrim(line.substr(space));
        if (l.verb == L"add") {
            if (!_wcsnicmp(rest.c_str(), L"top ", 4)) l.top = true, rest = MenuTrim(rest.substr(4));
            if (!_wcsicmp(rest.c_str(), L"separator") || rest == L"-") {
                l.item = L"-";
            } else {
                size_t eq = rest.find(L'=');
                if (eq == std::wstring::npos || MenuTrim(rest.substr(0, eq)).empty() || MenuTrim(rest.substr(eq + 1)).empty()) {
                    error(L"add needs a label and a command: add LABEL = COMMAND");
                    continue;
                }
                l.item = MenuTrim(rest.substr(0, eq));
                l.value = MenuTrim(rest.substr(eq + 1));
            }
        } else if (l.verb == L"rename") {
            size_t arrow = rest.find(L"->");
            if (arrow == std::wstring::npos || MenuTrim(rest.substr(0, arrow)).empty() || MenuTrim(rest.substr(arrow + 2)).empty()) {
                error(L"rename needs both names: rename OLD -> NEW");
                continue;
            }
            l.item = MenuRuleLabel(MenuTrim(rest.substr(0, arrow)));
            l.value = MenuTrim(rest.substr(arrow + 2));
        } else if (l.verb == L"hide" || l.verb == L"more" || l.verb == L"more-label") {
            if (rest.empty()) {
                error(l.verb + L" needs an item");
                continue;
            }
            l.item = rest;
            // (as the settings' lists are compared: lower case, " (greyed)" kept)
            if (l.verb != L"more-label")
                for (auto& c : l.item) c = (wchar_t)towlower(c);
        } else {
            error(L"no verb \"" + l.verb + L"\" (hide, more, more-label, rename, add)");
            continue;
        }
        file.sections.back().lines.push_back(l);
    }
    return file;
}

// 1 holds, 0 doesn't, -1 can't tell (what's chosen isn't known).
int MenuConditionHolds(std::wstring const& condition, MenuContext const& c) {
    bool flip = condition[0] == L'!';
    std::wstring x = flip ? MenuTrim(condition.substr(1)) : condition;
    size_t space = x.find(L' ');
    std::wstring key = x.substr(0, space);
    std::vector<std::wstring> words;
    for (size_t at = space; at != std::wstring::npos && at < x.size();) {
        size_t from = x.find_first_not_of(L' ', at);
        if (from == std::wstring::npos) break;
        at = x.find(L' ', from);
        words.push_back(x.substr(from, at == std::wstring::npos ? std::wstring::npos : at - from));
    }
    int holds = 0;
    if (key == L"menu" || key == L"app") {
        std::wstring const& what = key == L"menu" ? c.menu : c.app;
        for (auto const& w : words) holds |= MenuWild(w.c_str(), what.c_str());
    } else if (key == L"shift") {
        holds = GetKeyState(VK_SHIFT) < 0;
    } else {
        if (!c.known) return -1;
        if (key == L"background") {
            holds = c.files.empty();
        } else if (key == L"folder") {
            holds = !c.files.empty();
            for (auto const& f : c.files) {
                DWORD a = GetFileAttributesW(f.c_str());
                if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY)) holds = 0;
            }
        } else if (key == L"files") {
            holds = !c.files.empty();
            for (auto const& f : c.files) {
                size_t slash = f.find_last_of(L"\\/");
                std::wstring name = slash == std::wstring::npos ? f : f.substr(slash + 1);
                bool one = false;
                for (auto const& w : words) one |= MenuWild(w.c_str(), name.c_str());
                if (!one) holds = 0;
            }
        }
    }
    return flip ? !holds : holds;
}

// The rules a menu gets: the settings' (base), then the file's sections that hold for it, in order.
MenuRules MenuRulesFor(MenuFile const& file, MenuContext const& c, MenuRules rules) {
    for (auto const& section : file.sections) {
        bool holds = true;
        for (auto const& w : section.when) holds &= MenuConditionHolds(w, c) == 1;
        if (!holds) continue;
        for (auto const& l : section.lines) {
            if (l.verb == L"hide") rules.hide.push_back(l.item);
            else if (l.verb == L"more") rules.more.push_back(l.item);
            else if (l.verb == L"more-label") rules.moreLabel = l.item;
            else if (l.verb == L"rename") rules.rename.push_back({l.item, l.value});
            else if (l.verb == L"add") rules.add.push_back({l.item, l.value, l.top});
        }
    }
    return rules;
}

std::wstring MenuFilePath() {
    WCHAR roaming[MAX_PATH] = L"";  // (from the environment: nothing to link, in any program)
    DWORD n = GetEnvironmentVariableW(L"APPDATA", roaming, ARRAYSIZE(roaming));
    return n && n < ARRAYSIZE(roaming) ? std::wstring(roaming) + L"\\Tourne\\Desktop\\menus.conf" : L"";
}

// The file as it is now: parsed again only when it changed (its time and size), its errors then written down.
MenuFile const& LoadMenuFile() {
    static MENU_STYLE_STORAGE MenuFile* s_file;
    static MENU_STYLE_STORAGE ULONGLONG s_seen;  // its write time, when last read
    static MENU_STYLE_STORAGE ULONGLONG s_size;
    static MENU_STYLE_STORAGE DWORD s_checked;
    if (!s_file) s_file = new MenuFile;
    if (s_checked && GetTickCount() - s_checked < 500) return *s_file;  // (several menus at once: one look)
    s_checked = GetTickCount() | 1;
    std::wstring path = MenuFilePath();
    WIN32_FILE_ATTRIBUTE_DATA a{};
    if (path.empty() || !GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &a)) {
        if (s_seen) *s_file = {}, s_seen = 0;
        return *s_file;
    }
    ULONGLONG time = ((ULONGLONG)a.ftLastWriteTime.dwHighDateTime << 32) | a.ftLastWriteTime.dwLowDateTime;
    ULONGLONG size = ((ULONGLONG)a.nFileSizeHigh << 32) | a.nFileSizeLow;
    if (time == s_seen && size == s_size) return *s_file;
    s_seen = time, s_size = size;
    std::wstring text;
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        std::string bytes((size_t)std::min<ULONGLONG>(size, 1 << 20), '\0');
        DWORD got = 0;
        if (!bytes.empty() && ReadFile(h, bytes.data(), (DWORD)bytes.size(), &got, nullptr)) {
            bytes.resize(got);
            if (bytes.size() >= 3 && !bytes.compare(0, 3, "\xEF\xBB\xBF")) bytes.erase(0, 3);
            int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
            text.resize(n);
            MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), text.data(), n);
        }
        CloseHandle(h);
    }
    std::wstring errors;
    *s_file = ParseMenuFile(text, &errors);
    // menus.log: what didn't make sense, or that all of it did.
    std::wstring log = path.substr(0, path.size() - 5) + L".log";
    std::wstring report = errors.empty() ? L"menus.conf read: every line made sense\r\n" : L"menus.conf: lines left out\r\n" + errors;
    HANDLE out = CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (out != INVALID_HANDLE_VALUE) {
        int n = WideCharToMultiByte(CP_UTF8, 0, report.c_str(), (int)report.size(), nullptr, 0, nullptr, nullptr);
        std::string utf8(n, '\0');
        WideCharToMultiByte(CP_UTF8, 0, report.c_str(), (int)report.size(), utf8.data(), n, nullptr, nullptr);
        DWORD wrote = 0;
        WriteFile(out, utf8.data(), (DWORD)utf8.size(), &wrote, nullptr);
        CloseHandle(out);
    }
    return *s_file;
}

// This program's file name, lower case (for "app" conditions).
std::wstring MenuAppName(HWND owner = nullptr) {
    DWORD pid = GetCurrentProcessId();
    if (owner) GetWindowThreadProcessId(owner, &pid);
    std::wstring name;
    if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        WCHAR path[MAX_PATH];
        DWORD n = ARRAYSIZE(path);
        if (QueryFullProcessImageNameW(p, 0, path, &n)) {
            name = path;
            name = name.substr(name.find_last_of(L'\\') + 1);
        }
        CloseHandle(p);
    }
    for (auto& c : name) c = (wchar_t)towlower(c);
    return name;
}

// The chosen command, when it's one the rules added: run, and true.
bool RunMenuRule(MenuRulesUndo const& undo, UINT cmd, MenuContext const& c) {
    if (cmd < kMenuAddFirst || cmd > kMenuAddLast || cmd - kMenuAddFirst >= undo.commands.size()) return false;
    std::wstring command = undo.commands[cmd - kMenuAddFirst];
    if (command.empty()) return true;
    std::wstring folder = c.folder;
    if (folder.empty() && !c.files.empty()) folder = c.files[0].substr(0, c.files[0].find_last_of(L"\\/"));
    auto quoted = [](std::wstring const& x) { return L"\"" + x + L"\""; };
    std::wstring all;
    for (auto const& f : c.files) all += (all.empty() ? L"" : L" ") + quoted(f);
    auto put = [&](std::wstring const& name, std::wstring const& value) {
        for (size_t at = 0; (at = command.find(name, at)) != std::wstring::npos; at += value.size()) command.replace(at, name.size(), value);
    };
    put(L"%files%", all);
    put(L"%file%", c.files.empty() ? L"" : quoted(c.files[0]));
    put(L"%folder%", folder.empty() ? L"" : quoted(folder));
    WCHAR expanded[4096];
    if (ExpandEnvironmentStringsW(command.c_str(), expanded, ARRAYSIZE(expanded))) command = expanded;
    // The program (quoted, or up to the first space), then its arguments.
    std::wstring program, args;
    command = MenuTrim(command);
    if (!command.empty() && command[0] == L'"') {
        size_t close = command.find(L'"', 1);
        program = command.substr(1, close == std::wstring::npos ? std::wstring::npos : close - 1);
        args = close == std::wstring::npos ? L"" : MenuTrim(command.substr(close + 1));
    } else {
        size_t space = command.find(L' ');
        program = command.substr(0, space);
        args = space == std::wstring::npos ? L"" : MenuTrim(command.substr(space));
    }
    ShellExecuteW(nullptr, nullptr, program.c_str(), args.empty() ? nullptr : args.c_str(), folder.empty() ? nullptr : folder.c_str(), SW_SHOWNORMAL);
    return true;
}

UINT TrackStyledMenu(HMENU menu, UINT flags, int x, int y, HWND owner, bool foreign = false, RECT const* from = nullptr) {
    if (MenusLikeWindows()) LookLikeWindowsMenus(g_menuLook);
    MenuWheelStart();
    if (g_menuLook.native) {
        static MENU_STYLE_STORAGE int depth;
        NativeMenuItems(menu, foreign);
        depth++;
        // Windows' own menus, only animated here when set.
        bool styled = g_menuWindowsStyled;
        g_menuWindowsStyled = false;
        HHOOK hook = g_menuMotion.style ? SetWindowsHookExW(WH_CALLWNDPROC, StyledMenuHookProc, nullptr, GetCurrentThreadId()) : nullptr;
        UINT result = TrackMenuOnScreen(menu, flags | TPM_NOANIMATION, x, y, owner, from);  // no fade: it opens at once
        if (hook) UnhookWindowsHookEx(hook);
        g_menuWindowsStyled = styled;
        depth--;
        UnNativeMenuItems(menu);
        if (!depth) {
            for (HBITMAP bitmap : g_nativeMenuBitmaps) DeleteObject(bitmap);
            g_nativeMenuBitmaps.clear();
        }
        MenuWheelStop();
        return result;
    }
    // A visual style's menu: its own ground and border around the items too.
    if (g_menuLook.theme) {
        // Sampled once per theme, not on every menu.
        if (g_menuSampled != g_menuLook.theme) {
            g_menuSampledGround = ThemePartPixel(g_menuLook.theme, MENU_POPUPBACKGROUND, 4, 4, g_menuLook.ground);
            g_menuSampledEdge = ThemePartPixel(g_menuLook.theme, MENU_POPUPBORDERS, 0, 0, g_menuLook.edge);
            g_menuSampled = g_menuLook.theme;
        }
        g_menuLook.ground = g_menuSampledGround;
        g_menuLook.edge = g_menuSampledEdge;
    }
    StyleMenuItems(menu, foreign);
    FitStyledMenu(menu, x, y, from);
    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, StyledMenuHookProc, nullptr, GetCurrentThreadId());
    UINT result = TrackMenuOnScreen(menu, flags | TPM_NOANIMATION, x, y, owner, from);
    if (hook) UnhookWindowsHookEx(hook);
    FreeStyledMenuItems(menu);
    MenuWheelStop();
    return result;
}
// Motion: Hyprland-style animation curves and the tree of what animates.
// Inlined into each part that animates ("// @inline motion.cpp") and into
// Tourne Desktop's host, so a curve means the same thing everywhere.
//
// Settings, each one line in the form Hyprland's config uses:
//   motion.enabled = false                         every animation off
//   motion.<leaf> = ON, SPEED, CURVE[, STYLE]      motion.bar = 1, 4, overshoot, slide
//   motion.bezier.<name> = X0, Y0, X1, Y1          a cubic Bezier from (0, 0) to (1, 1)
//   motion.spring.<name> = MASS, STIFFNESS, DAMPING
// SPEED is in tenths of a second, as in Hyprland; a spring's own physics sets
// its length instead. A leaf left unset takes its parent's line, up to global.
// Nothing set anywhere up the tree means the part keeps its own animation, so
// with no motion lines at all nothing changes.
//
// Tourne Desktop's config also reads Hyprland's own lines unchanged
// ("animation = windowsIn, 1, 4, overshoot, popin 80%", "bezier = ...",
// an "animations { }" group, and the Lua form) into these settings.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cwctype>
#include <string>
#include <vector>

namespace motion {

struct Curve {
    bool spring = false;
    double x1 = 0, y1 = 0.75, x2 = 0.15, y2 = 1;     // Bezier control points (Hyprland's "default")
    double mass = 1, stiffness = 100, damping = 10;  // spring
};

struct Anim {
    bool set = false;  // a motion line up the tree says how this animates
    bool on = true;
    double speed = 8;  // tenths of a second
    std::wstring curveName = L"default";
    Curve curve;
    std::wstring style;  // as written: "slide", "popin 80%", "loop", ...
    std::wstring from;   // the leaf whose line it is
    double springSeconds = 0;  // (a spring's length, worked out once)
};

// The tree, Hyprland's leaves first, then Tourne's own (its bars, Start,
// popups and switcher are layers, as a bar or launcher is in Hyprland, so a
// dots file's "layers" line moves them too).
struct Leaf {
    const wchar_t* name;
    const wchar_t* parent;
    const wchar_t* about;
};

inline const std::vector<Leaf>& Tree() {
    static const std::vector<Leaf> tree = {
        {L"global", nullptr, L"Everything, unless a leaf below says otherwise"},
        {L"windows", L"global", L"Windows: slide, popin, gnomed"},
        {L"windowsIn", L"windows", L"A window opening"},
        {L"windowsOut", L"windows", L"A window closing"},
        {L"windowsMove", L"windows", L"Moving, dragging and resizing"},
        {L"layers", L"global", L"Bars, menus and overlays: slide, popin, fade"},
        {L"layersIn", L"layers", L"A layer opening"},
        {L"layersOut", L"layers", L"A layer closing"},
        {L"bar", L"layers", L"Tourne's bar hiding and coming back: slide, fade, slidefade"},
        {L"start", L"layers", L"The Start menu"},
        {L"popups", L"layers", L"Flyouts and popup menus"},
        {L"switcher", L"layers", L"The window switcher and its views"},
        {L"fade", L"global", L"Fades"},
        {L"fadeIn", L"fade", L"Fading in a window that opens"},
        {L"fadeOut", L"fade", L"Fading out a window that closes"},
        {L"fadeSwitch", L"fade", L"The active window changing"},
        {L"fadeShadow", L"fade", L"Shadows, as the active window changes"},
        {L"fadeGlow", L"fade", L"Glows, as the active window changes"},
        {L"fadeDim", L"fade", L"Dimming inactive windows"},
        {L"fadeLayers", L"fade", L"Fades on layers"},
        {L"fadeLayersIn", L"fadeLayers", L"A layer fading in"},
        {L"fadeLayersOut", L"fadeLayers", L"A layer fading out"},
        {L"fadePopups", L"fade", L"Fades on popups"},
        {L"fadePopupsIn", L"fadePopups", L"A popup fading in"},
        {L"fadePopupsOut", L"fadePopups", L"A popup fading out"},
        {L"fadeDpms", L"fade", L"Screens turning off and on"},
        {L"border", L"global", L"Border colours changing"},
        {L"borderangle", L"global", L"A border's gradient turning: once, loop"},
        {L"shadowangle", L"global", L"A shadow's gradient turning: once, loop"},
        {L"glowangle", L"global", L"A glow's gradient turning: once, loop"},
        {L"workspaces", L"global", L"Desktops changing: slide, slidevert, fade, slidefade, slidefadevert"},
        {L"workspacesIn", L"workspaces", L"The desktop you go to"},
        {L"workspacesOut", L"workspaces", L"The desktop you leave"},
        {L"specialWorkspace", L"workspaces", L"The scratchpad desktop"},
        {L"specialWorkspaceIn", L"specialWorkspace", L"The scratchpad coming in"},
        {L"specialWorkspaceOut", L"specialWorkspace", L"The scratchpad going"},
        {L"zoomFactor", L"global", L"Screen zoom"},
        {L"monitorAdded", L"global", L"A screen being added"},
    };
    return tree;
}

inline const wchar_t* ParentOf(std::wstring const& leaf) {
    for (auto const& l : Tree())
        if (leaf == l.name) return l.parent;
    return nullptr;
}

inline Curve Bezier(double x1, double y1, double x2, double y2) {
    Curve c;
    c.x1 = x1, c.y1 = y1, c.x2 = x2, c.y2 = y2;
    return c;
}

inline Curve Spring(double mass, double stiffness, double damping) {
    Curve c;
    c.spring = true;
    c.mass = mass, c.stiffness = stiffness, c.damping = damping;
    return c;
}

// Curves with a name of their own: Hyprland's, CSS's, the easings.net set,
// and a few springs.
struct Named {
    const wchar_t* name;
    Curve curve;
};

inline const std::vector<Named>& BuiltIn() {
    static const std::vector<Named> named = {
        {L"default", Bezier(0, 0.75, 0.15, 1)},
        {L"linear", Bezier(0, 0, 1, 1)},
        {L"ease", Bezier(0.25, 0.1, 0.25, 1)},
        {L"ease-in", Bezier(0.42, 0, 1, 1)},
        {L"ease-out", Bezier(0, 0, 0.58, 1)},
        {L"ease-in-out", Bezier(0.42, 0, 0.58, 1)},
        {L"overshoot", Bezier(0.5, 0.9, 0.1, 1.1)},
        {L"overshot", Bezier(0.05, 0.9, 0.1, 1.1)},
        {L"easeInSine", Bezier(0.12, 0, 0.39, 0)},
        {L"easeOutSine", Bezier(0.61, 1, 0.88, 1)},
        {L"easeInOutSine", Bezier(0.37, 0, 0.63, 1)},
        {L"easeInQuad", Bezier(0.11, 0, 0.5, 0)},
        {L"easeOutQuad", Bezier(0.5, 1, 0.89, 1)},
        {L"easeInOutQuad", Bezier(0.45, 0, 0.55, 1)},
        {L"easeInCubic", Bezier(0.32, 0, 0.67, 0)},
        {L"easeOutCubic", Bezier(0.33, 1, 0.68, 1)},
        {L"easeInOutCubic", Bezier(0.65, 0, 0.35, 1)},
        {L"easeInQuart", Bezier(0.5, 0, 0.75, 0)},
        {L"easeOutQuart", Bezier(0.25, 1, 0.5, 1)},
        {L"easeInOutQuart", Bezier(0.76, 0, 0.24, 1)},
        {L"easeInQuint", Bezier(0.64, 0, 0.78, 0)},
        {L"easeOutQuint", Bezier(0.22, 1, 0.36, 1)},
        {L"easeInOutQuint", Bezier(0.83, 0, 0.17, 1)},
        {L"easeInExpo", Bezier(0.7, 0, 0.84, 0)},
        {L"easeOutExpo", Bezier(0.16, 1, 0.3, 1)},
        {L"easeInOutExpo", Bezier(0.87, 0, 0.13, 1)},
        {L"easeInCirc", Bezier(0.55, 0, 1, 0.45)},
        {L"easeOutCirc", Bezier(0, 0.55, 0.45, 1)},
        {L"easeInOutCirc", Bezier(0.85, 0, 0.15, 1)},
        {L"easeInBack", Bezier(0.36, 0, 0.66, -0.56)},
        {L"easeOutBack", Bezier(0.34, 1.56, 0.64, 1)},
        {L"easeInOutBack", Bezier(0.68, -0.6, 0.32, 1.6)},
        {L"rubber", Spring(1, 70, 10)},
        {L"gentle", Spring(1, 120, 18)},
        {L"snappy", Spring(1, 300, 30)},
        {L"bouncy", Spring(1, 150, 8)},
        {L"wobbly", Spring(1, 90, 5)},
    };
    return named;
}

inline std::wstring Trimmed(std::wstring const& s) {
    size_t a = s.find_first_not_of(L" \t\r\n"), b = s.find_last_not_of(L" \t\r\n");
    return a == std::wstring::npos ? L"" : s.substr(a, b - a + 1);
}

// "a, b, c": the parts, trimmed.
inline std::vector<std::wstring> Parts(std::wstring const& s) {
    std::vector<std::wstring> parts;
    size_t start = 0;
    while (start <= s.size()) {
        size_t comma = s.find(L',', start);
        if (comma == std::wstring::npos) comma = s.size();
        parts.push_back(Trimmed(s.substr(start, comma - start)));
        start = comma + 1;
    }
    return parts;
}

inline bool Number(std::wstring const& s, double* out) {
    if (s.empty()) return false;
    wchar_t* end = nullptr;
    double v = wcstod(s.c_str(), &end);
    if (!end || *end) return false;
    *out = v;
    return true;
}

inline bool Truth(std::wstring const& s, bool* out) {
    std::wstring v;
    for (wchar_t c : s) v += (wchar_t)towlower(c);
    if (v == L"1" || v == L"true" || v == L"yes" || v == L"on") return *out = true, true;
    if (v == L"0" || v == L"false" || v == L"no" || v == L"off") return *out = false, true;
    return false;
}

// "X0, Y0, X1, Y1" (x kept between 0 and 1, as CSS and Hyprland need).
inline bool ParseBezier(std::wstring const& value, Curve* out) {
    auto p = Parts(value);
    double v[4];
    if (p.size() != 4) return false;
    for (int i = 0; i < 4; i++)
        if (!Number(p[i], &v[i])) return false;
    *out = Bezier(std::clamp(v[0], 0.0, 1.0), v[1], std::clamp(v[2], 0.0, 1.0), v[3]);
    return true;
}

// "MASS, STIFFNESS, DAMPING".
inline bool ParseSpring(std::wstring const& value, Curve* out) {
    auto p = Parts(value);
    double v[3];
    if (p.size() != 3) return false;
    for (int i = 0; i < 3; i++)
        if (!Number(p[i], &v[i]) || v[i] <= 0) return false;
    *out = Spring(v[0], v[1], v[2]);
    return true;
}

// "ON, SPEED, CURVE[, STYLE]", or just "ON".
inline bool ParseLine(std::wstring const& value, Anim* out) {
    auto p = Parts(value);
    if (p.empty() || !Truth(p[0], &out->on)) return false;
    if (p.size() >= 2 && !p[1].empty()) {
        double speed;
        if (!Number(p[1], &speed) || speed < 0) return false;
        out->speed = speed;
    }
    if (p.size() >= 3 && !p[2].empty()) out->curveName = p[2];
    if (p.size() >= 4) out->style = p[3];
    return true;
}

// A Bezier's y where its x is t.
inline double BezierAt(Curve const& c, double t) {
    t = std::clamp(t, 0.0, 1.0);
    auto at = [](double a, double b, double s) { return 3 * a * s * (1 - s) * (1 - s) + 3 * b * s * s * (1 - s) + s * s * s; };
    auto slope = [](double a, double b, double s) { return 3 * a * (1 - s) * (1 - s) + 6 * (b - a) * s * (1 - s) + 3 * (1 - b) * s * s; };
    double s = t;
    for (int i = 0; i < 8; i++) {  // Newton, falling back to halving where the slope is flat
        double x = at(c.x1, c.x2, s) - t, d = slope(c.x1, c.x2, s);
        if (std::fabs(x) < 1e-7) return at(c.y1, c.y2, s);
        if (std::fabs(d) < 1e-6) break;
        s = std::clamp(s - x / d, 0.0, 1.0);
    }
    double lo = 0, hi = 1;
    s = t;
    for (int i = 0; i < 40; i++) {
        double x = at(c.x1, c.x2, s);
        if (std::fabs(x - t) < 1e-7) break;
        (x < t ? lo : hi) = s;
        s = (lo + hi) / 2;
    }
    return at(c.y1, c.y2, s);
}

// A spring let go at 0, pulled to 1: where it is after some seconds.
inline double SpringAt(Curve const& c, double seconds) {
    double w0 = std::sqrt(c.stiffness / c.mass), zeta = c.damping / (2 * std::sqrt(c.stiffness * c.mass)), t = seconds;
    double y;  // how far from 1
    if (zeta < 1) {
        double wd = w0 * std::sqrt(1 - zeta * zeta);
        y = -std::exp(-zeta * w0 * t) * (std::cos(wd * t) + zeta * w0 / wd * std::sin(wd * t));
    } else if (zeta == 1) {
        y = -(1 + w0 * t) * std::exp(-w0 * t);
    } else {
        double root = std::sqrt(zeta * zeta - 1), r1 = -w0 * (zeta - root), r2 = -w0 * (zeta + root);
        double a = r2 / (r1 - r2);
        y = a * std::exp(r1 * t) + (-1 - a) * std::exp(r2 * t);
    }
    return 1 + y;
}

// How long a spring takes to settle (within a thousandth), at most 5 s.
inline double SpringSeconds(Curve const& c) {
    double settled = 0;
    for (double t = 0; t < 5; t += 0.004) {
        if (std::fabs(SpringAt(c, t) - 1) > 0.001) settled = t + 0.004;
    }
    return std::max(settled, 0.05);
}

inline double SpringLength(Anim const& a) { return a.springSeconds > 0 ? a.springSeconds : SpringSeconds(a.curve); }

// The animation's length, in milliseconds (0 when it's off).
inline double Ms(Anim const& a) {
    if (!a.on) return 0;
    return a.curve.spring ? 1000 * SpringLength(a) : a.speed * 100;
}

// Progress t (0 to 1 of its length) eased; may pass 1 on the way (overshoot).
inline double Ease(Anim const& a, double t) {
    t = std::clamp(t, 0.0, 1.0);
    if (t >= 1) return 1;
    return a.curve.spring ? SpringAt(a.curve, t * SpringLength(a)) : BezierAt(a.curve, t);
}

// The curve a name stands for: one of the settings' own, else one built in.
template <class Read>
bool CurveNamed(std::wstring const& name, Read&& read, Curve* out) {
    if (name.empty()) return false;
    if (ParseBezier(read(L"motion.bezier." + name), out)) return true;
    if (ParseSpring(read(L"motion.spring." + name), out)) return true;
    for (auto const& n : BuiltIn()) {
        if (name == n.name) {
            *out = n.curve;
            return true;
        }
    }
    return false;
}

// How a leaf animates: its line, else its parent's, up to global. read(key)
// gives a setting's text ("" when unset).
template <class Read>
Anim Resolve(std::wstring const& leaf, Read&& read) {
    Anim a;
    bool all = true;
    if (Truth(read(L"motion.enabled"), &all) && !all) {
        a.set = true;
        a.on = false;
        a.from = L"enabled";
        return a;
    }
    std::wstring at = leaf;
    while (!at.empty()) {
        Anim line;
        if (ParseLine(read(L"motion." + at), &line)) {
            a = line;
            a.set = true;
            a.from = at;
            break;
        }
        const wchar_t* parent = ParentOf(at);
        at = parent ? parent : L"";
    }
    if (a.set && !CurveNamed(a.curveName, read, &a.curve)) a.curve = Curve();  // (a name not known: Hyprland's default)
    if (a.curve.spring) a.springSeconds = SpringSeconds(a.curve);
    return a;
}

// The style's first word, lowercase ("popin 80%" -> "popin").
inline std::wstring StyleWord(Anim const& a) {
    std::wstring w;
    for (wchar_t c : a.style) {
        if (iswspace(c)) break;
        w += (wchar_t)towlower(c);
    }
    return w;
}

// The style's percentage as a fraction ("popin 80%" -> 0.8), or the fallback.
inline double StylePercent(Anim const& a, double fallback) {
    size_t pct = a.style.find(L'%');
    if (pct == std::wstring::npos) return fallback;
    size_t start = a.style.find_last_of(L" \t", pct);
    start = start == std::wstring::npos ? 0 : start + 1;
    double v;
    return Number(a.style.substr(start, pct - start), &v) ? std::clamp(v / 100, 0.0, 1.0) : fallback;
}

#ifdef WH_MOD_ID
// In a part: settings by their full name (the motion lines aren't the part's own).
inline std::wstring Setting(std::wstring const& key) {
    PCWSTR v = Wh_GetStringSetting(key.c_str());
    std::wstring text = v ? v : L"";
    if (v) Wh_FreeStringSetting(v);
    return text;
}

// A leaf as the settings have it.
inline Anim Get(std::wstring const& leaf) { return Resolve(leaf, Setting); }

// Curves looked up by name, kept until the settings change (Forget).
struct NamedCache {
    std::vector<std::pair<std::wstring, Anim>> found;
};
inline NamedCache& Cache() {
    static NamedCache cache;
    return cache;
}
inline void Forget() { Cache().found.clear(); }

// A curve by name, as an animation of it; false when there's none by that name.
inline bool CurveByName(std::wstring const& name, Anim* out) {
    for (auto const& [n, a] : Cache().found) {
        if (n == name) {
            *out = a;
            return a.set;
        }
    }
    Anim a;
    a.set = CurveNamed(name, Setting, &a.curve);
    a.curveName = name;
    if (a.set && a.curve.spring) a.springSeconds = SpringSeconds(a.curve);
    Cache().found.push_back({name, a});
    *out = a;
    return a.set;
}
#endif

}  // namespace motion

////////////////////////////////////////////////////////////////////////////////
// Which menus are drawn here.

struct Settings {
    bool explorer = false, dialogs = false, windowsMenus = false, allMenus = false, menuBars = false;
    std::vector<std::wstring> noDuplicates;  // lower case
    bool keepLast = true;
};
Settings g_settings;
bool g_inExplorer = false;  // this program is explorer.exe

std::wstring ProgramName(HMODULE module = nullptr) {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(module, path, ARRAYSIZE(path));
    if (!length || length == ARRAYSIZE(path)) return L"";
    PCWSTR name = wcsrchr(path, L'\\');
    return name ? name + 1 : path;
}

bool ClassIs(HWND wnd, PCWSTR name) {
    WCHAR cls[64];
    return wnd && GetClassNameW(wnd, cls, ARRAYSIZE(cls)) && !_wcsicmp(cls, name);
}

BOOL CALLBACK FindShellView(HWND child, LPARAM lp) {
    if (ClassIs(child, L"SHELLDLL_DefView") || ClassIs(child, L"DUIViewWndClassName")) {
        *(bool*)lp = true;
        return FALSE;
    }
    return TRUE;
}

// A dialog with a folder view in it: the Open, Save and folder pickers (the old and the new kind).
bool IsFileDialog(HWND root) {
    if (!ClassIs(root, L"#32770")) return false;
    bool found = false;
    EnumChildWindows(root, FindShellView, (LPARAM)&found);
    return found;
}

// Where Windows keeps the code that shows its own ("immersive") menus: as Classic Menus lists them
// (immersive-menus.cpp), read from Windows itself. shell32 and the File Explorer modules count in
// explorer.exe only (elsewhere they're a file dialog's).
constexpr PCWSTR kWindowsMenuModules[] = {
    L"Taskbar.dll", L"twinui.pcshell.dll", L"twinui.dll", L"explorer.exe", L"SndVolSSO.dll", L"InputSwitch.dll",
    L"SecurityHealthSsoUdk.dll", L"SecurityHealthSSO.dll", L"pnidui.dll", L"stobject.dll", L"Taskmgr.exe",
    L"Narrator.exe", L"ApplicationFrame.dll", L"museuxdocked.dll",
};
constexpr PCWSTR kExplorerModules[] = {L"shell32.dll", L"ExplorerFrame.dll", L"Windows.UI.FileExplorer.dll"};

bool CalledFromWindows(void* caller) {
    HMODULE module = nullptr;
    if (!caller || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       (LPCWSTR)caller, &module))
        return false;
    std::wstring name = ProgramName(module);
    for (PCWSTR m : kWindowsMenuModules)
        if (!_wcsicmp(m, name.c_str())) return true;
    if (g_inExplorer)
        for (PCWSTR m : kExplorerModules)
            if (!_wcsicmp(m, name.c_str())) return true;
    return false;
}

// Whether this menu is drawn here: by where its owner is, or who shows it.
bool WantMenu(HMENU menu, HWND owner, void* caller) {
    if (!menu || !IsMenu(menu) || !owner || GetWindowThreadProcessId(owner, nullptr) != GetCurrentThreadId()) return false;
    if (GetKeyState(VK_SHIFT) < 0) return false;  // (Windows' own, as asked)
    HWND root = GetAncestor(owner, GA_ROOT);
    if (g_settings.explorer && g_inExplorer && ClassIs(root, L"CabinetWClass")) return true;
    if (g_settings.dialogs && IsFileDialog(root)) return true;
    if (g_settings.windowsMenus && CalledFromWindows(caller)) return !ClassIs(root, L"CabinetWClass") || g_settings.explorer;
    if (g_settings.allMenus) return !ClassIs(root, L"CabinetWClass") || g_settings.explorer;
    return false;
}

// Items named in "Show these items once" kept once (the first or the last), as Menu No Duplicates does;
// put back with the hide rules' undo when the menu closes, since the menu is the program's.
MenuRulesUndo DropDuplicates(HMENU menu) {
    MenuRulesUndo undo;
    undo.menu = menu;
    if (g_settings.noDuplicates.empty() || !menu) return undo;
    int count = GetMenuItemCount(menu);
    std::vector<std::wstring> labels(std::max(0, count));
    for (int i = 0; i < count; i++) {
        WCHAR text[256] = L"";
        if (GetMenuStringW(menu, i, text, ARRAYSIZE(text), MF_BYPOSITION)) labels[i] = MenuRuleLabel(text);
    }
    std::vector<bool> drop(std::max(0, count), false);
    for (auto const& name : g_settings.noDuplicates) {
        std::vector<int> at;
        for (int i = 0; i < count; i++)
            if (labels[i] == name) at.push_back(i);
        for (size_t k = 0; k + 1 < at.size(); k++) drop[g_settings.keepLast ? at[k] : at[k + 1]] = true;
    }
    for (int i = count - 1; i >= 0; i--) {
        if (!drop[i]) continue;
        MenuRulesUndo::Taken t{i, {sizeof(MENUITEMINFOW)}, {}};
        WCHAR text[256] = L"";
        t.info.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_STRING | MIIM_DATA | MIIM_BITMAP | MIIM_CHECKMARKS;
        t.info.dwTypeData = text;
        t.info.cch = ARRAYSIZE(text);
        if (!GetMenuItemInfoW(menu, i, TRUE, &t.info)) continue;
        t.text = text;
        if (RemoveMenu(menu, i, MF_BYPOSITION)) undo.taken.insert(undo.taken.begin(), t);
    }
    return undo;
}

////////////////////////////////////////////////////////////////////////////////
// Tourne Desktop's menus, as it writes them (taskbar-host.cpp, PublishMenuLook): read again only when its
// stamp changes. Fonts are made for each scale they're asked for.

// A theme file, loaded the way theme tools do (as theme-palette.cpp does).
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

// The theme's menu class, or null.
HTHEME OpenStyleMenu(std::wstring const& path) {
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    auto load = ux ? (LoaderLoadTheme_t)GetProcAddress(ux, MAKEINTRESOURCEA(92)) : nullptr;
    auto open = ux ? (OpenThemeDataFromFile_t)GetProcAddress(ux, MAKEINTRESOURCEA(16)) : nullptr;
    if (!load || !open || path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) return nullptr;
    HANDLE sharable = nullptr, nonSharable = nullptr;
    if (FAILED(load(nullptr, nullptr, path.c_str(), L"NormalColor", L"NormalSize", &sharable, nullptr, 0, &nonSharable, nullptr,
                    0, nullptr, nullptr, 0, 0, FALSE)) ||
        !sharable || !nonSharable) {
        if (sharable) CloseHandle(sharable);
        if (nonSharable) CloseHandle(nonSharable);
        return nullptr;
    }
    // Kept for as long as the program runs: the theme's handles point into it.
    auto* file = new ThemeFileData{};
    memcpy(file->header, "thmfile", 7);
    memcpy(file->end, "end", 3);
    file->sharableSection = sharable;
    file->nonSharableSection = nonSharable;
    file->sharableView = MapViewOfFile(sharable, FILE_MAP_READ, 0, 0, 0);
    file->nonSharableView = MapViewOfFile(nonSharable, FILE_MAP_READ, 0, 0, 0);
    static HWND host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    HTHEME menu = open(file, host, L"Menu", 0);
    if (menu && !IsThemePartDefined(menu, MENU_POPUPITEM, 0)) {
        CloseThemeData(menu);
        menu = nullptr;
    }
    return menu;
}

struct Published {
    DWORD stamp = 0;
    bool found = false;
    MenuLook look;  // without its font and theme
    LOGFONTW font{};
    bool hasFont = false;
    int scale = 1;  // the font's
    std::wstring theme;
    int animation = 0;
    double ms = 150;
    bool flyoutClick = false;
    motion::Anim curve;  // set: the motion engine's curve
    MenuRules rules;
};

SRWLOCK g_publishedLock = SRWLOCK_INIT;
Published g_published;  // (guarded by g_publishedLock)
DWORD g_publishedStamp = 0xFFFFFFFF;
struct ScaledFont {
    DWORD stamp;
    int scale;
    HFONT font;
};
std::vector<ScaledFont> g_fonts;  // (guarded by g_publishedLock; old ones kept: a menu may still use them)
std::wstring g_themePath;
HTHEME g_themeMenu = nullptr;

constexpr wchar_t kPublishedKey[] = L"Software\\Tourne\\Menus";

DWORD RegDword(HKEY key, PCWSTR name, DWORD fallback) {
    DWORD value = 0, size = sizeof(value), type = 0;
    return RegQueryValueExW(key, name, nullptr, &type, (BYTE*)&value, &size) == ERROR_SUCCESS && type == REG_DWORD ? value : fallback;
}

std::wstring RegText(HKEY key, PCWSTR name) {
    WCHAR buffer[2048] = L"";
    DWORD size = sizeof(buffer) - sizeof(WCHAR), type = 0;
    if (RegQueryValueExW(key, name, nullptr, &type, (BYTE*)buffer, &size) != ERROR_SUCCESS || type != REG_SZ) return L"";
    buffer[size / sizeof(WCHAR)] = 0;
    return buffer;
}

// A curve as written: "bezier X1 Y1 X2 Y2" or "spring MASS STIFFNESS DAMPING".
bool ParseCurve(std::wstring const& text, motion::Curve* out) {
    double v[4]{};
    if (swscanf_s(text.c_str(), L"bezier %lf %lf %lf %lf", &v[0], &v[1], &v[2], &v[3]) == 4) {
        *out = motion::Bezier(v[0], v[1], v[2], v[3]);
        return true;
    }
    if (swscanf_s(text.c_str(), L"spring %lf %lf %lf", &v[0], &v[1], &v[2]) == 3) {
        *out = motion::Spring(v[0], v[1], v[2]);
        return true;
    }
    return false;
}

void ReadPublished(Published& p) {
    p = Published{};
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kPublishedKey, 0, KEY_READ, &key)) return;
    p.found = true;
    p.stamp = RegDword(key, L"Stamp", 0);
    MenuLook& l = p.look;
    l.ground = RegDword(key, L"Ground", l.ground), l.line = RegDword(key, L"Line", l.line), l.edge = RegDword(key, L"Edge", l.edge);
    l.text = RegDword(key, L"Text", l.text), l.accent = RegDword(key, L"Accent", l.accent), l.dim = RegDword(key, L"Dim", l.dim);
    l.hover = RegDword(key, L"Hover", l.hover), l.hoverText = RegDword(key, L"HoverText", l.hoverText);
    l.arrow = RegDword(key, L"Arrow", l.arrow), l.frameLine = RegDword(key, L"FrameLine", l.frameLine), l.band = RegDword(key, L"Band", l.band);
    l.native = RegDword(key, L"Native", 0), l.classic = RegDword(key, L"Classic", 0), l.ownText = RegDword(key, L"OwnText", 0);
    l.frame = RegDword(key, L"Frame", 0), l.caps = RegDword(key, L"Caps", 0), l.noIcons = RegDword(key, L"NoIcons", 0);
    l.windows = RegDword(key, L"Windows", 0);
    l.frameGap = (int)RegDword(key, L"FrameGap", 6), l.frameGlow = (int)RegDword(key, L"FrameGlow", 0);
    l.frameStopCount = std::clamp((int)RegDword(key, L"FrameStopCount", 0), 0, 3);
    DWORD size = sizeof(l.frameStops), type = 0;
    if (RegQueryValueExW(key, L"FrameStops", nullptr, &type, (BYTE*)l.frameStops, &size) != ERROR_SUCCESS) l.frameStopCount = 0;
    p.scale = std::max(1, (int)RegDword(key, L"Scale", 1));
    size = sizeof(p.font);
    p.hasFont = RegQueryValueExW(key, L"Font", nullptr, &type, (BYTE*)&p.font, &size) == ERROR_SUCCESS && size == sizeof(p.font);
    p.theme = RegText(key, L"Theme");
    p.animation = (int)RegDword(key, L"Animation", 0);
    p.flyoutClick = RegDword(key, L"FlyoutClick", 0) != 0;
    p.ms = (double)RegDword(key, L"AnimationMs", 150);
    p.curve.set = ParseCurve(RegText(key, L"Curve"), &p.curve.curve);
    if (p.curve.set && p.curve.curve.spring) p.curve.springSeconds = motion::SpringSeconds(p.curve.curve);
    p.rules.hide = MenuRuleList(RegText(key, L"Hide"));
    p.rules.more = MenuRuleList(RegText(key, L"More"));
    if (std::wstring label = RegText(key, L"MoreLabel"); !label.empty()) p.rules.moreLabel = label;
    RegCloseKey(key);
}

// Tourne's pixel font, when it's installed (as Tourne Desktop's bar picks it), else Windows' menu font.
HFONT DefaultMenuFont(int scale) {
    for (PCWSTR face : {L"CozetteVector", L"scientifica"}) {
        LOGFONTW lf{};
        lf.lfHeight = -13 * scale;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfQuality = NONANTIALIASED_QUALITY;
        wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
        HFONT font = CreateFontIndirectW(&lf);
        HDC dc = GetDC(nullptr);
        HGDIOBJ old = SelectObject(dc, font);
        WCHAR got[LF_FACESIZE] = L"";
        GetTextFaceW(dc, LF_FACESIZE, got);
        SelectObject(dc, old);
        ReleaseDC(nullptr, dc);
        if (!_wcsicmp(got, face)) return font;
        DeleteObject(font);
    }
    NONCLIENTMETRICSW metrics{sizeof(metrics)};
    if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, 96 * scale))
        return CreateFontIndirectW(&metrics.lfMenuFont);
    return nullptr;
}

// What this thread's next menu looks like, at the owner's scale.
thread_local motion::Anim t_curve;
double CurveEase(double t) { return motion::Ease(t_curve, t); }

MenuRules LoadMenuLook(HWND owner) {
    int scale = std::max(1, (int)(GetDpiForWindow(owner) + 48) / 96);
    AcquireSRWLockExclusive(&g_publishedLock);
    HKEY key = nullptr;
    DWORD stamp = 0;
    if (!RegOpenKeyExW(HKEY_CURRENT_USER, kPublishedKey, 0, KEY_READ, &key)) {
        stamp = RegDword(key, L"Stamp", 0);
        RegCloseKey(key);
    }
    if (stamp != g_publishedStamp) {
        ReadPublished(g_published);
        g_publishedStamp = stamp;
        if (g_published.theme != g_themePath) {
            g_themePath = g_published.theme;
            g_themeMenu = OpenStyleMenu(g_themePath);  // (the last one stays open: menus may be drawing with it)
        }
    }
    Published const& p = g_published;
    MenuLook look = p.look;
    look.scale = scale;
    look.theme = p.theme.empty() ? nullptr : g_themeMenu;
    HFONT font = nullptr;
    for (auto const& f : g_fonts)
        if (f.stamp == stamp && f.scale == scale) font = f.font;
    if (!font) {
        if (p.hasFont) {
            LOGFONTW lf = p.font;
            lf.lfHeight = MulDiv(lf.lfHeight, scale, p.scale);
            font = CreateFontIndirectW(&lf);
        } else {
            font = DefaultMenuFont(scale);
        }
        g_fonts.push_back({stamp, scale, font});
    }
    look.font = font;
    MenuRules rules = p.rules;
    MenuMotion motion;
    motion.style = p.animation;
    g_menuFlyoutClick = p.flyoutClick;
    motion.ms = p.ms;
    t_curve = p.curve;
    motion.ease = p.curve.set ? CurveEase : nullptr;
    ReleaseSRWLockExclusive(&g_publishedLock);
    g_menuLook = look;
    g_menuMotion = motion;
    return rules;
}

////////////////////////////////////////////////////////////////////////////////
// The menu itself: drawn here while its owner passes on what it doesn't draw itself, its submenus styled
// as the owner fills them in.

thread_local bool t_busy = false;  // a menu drawn here is open on this thread

LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_MEASUREITEM:
            if (MenuMeasureItem((MEASUREITEMSTRUCT*)lParam)) return TRUE;
            break;
        case WM_DRAWITEM:
            if (MenuDrawItem((DRAWITEMSTRUCT*)lParam)) return TRUE;
            break;
        case WM_INITMENUPOPUP: {
            // A submenu ("Send to", "New") filling in, then styled.
            LRESULT result = DefSubclassProc(hwnd, msg, wParam, lParam);
            StyleMenuItems((HMENU)wParam, true);
            return result;
        }
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

UINT TrackHere(HMENU menu, UINT flags, int x, int y, HWND owner, RECT const* exclude) {
    t_busy = true;
    MenuRules rules = LoadMenuLook(owner);
    // The menu file's sections for this program's menus (what's chosen isn't known here).
    MenuContext context;
    context.app = MenuAppName();
    context.menu = context.app == L"explorer.exe" ? L"explorer" : L"windows";
    rules = MenuRulesFor(LoadMenuFile(), context, rules);
    MenuRulesUndo once = DropDuplicates(menu);
    MenuRulesUndo undo = ApplyMenuRules(menu, rules, [](UINT) { return std::wstring(); });
    SetWindowSubclass(owner, OwnerProc, 0, 0);
    // With items of ours, the choice comes back here first; one of theirs then goes on as Windows would send it.
    bool ours = !rules.add.empty() && !(flags & TPM_RETURNCMD);
    UINT result = TrackStyledMenu(menu, flags | (ours ? TPM_RETURNCMD : 0), x, y, owner, true,
                                  exclude && !IsRectEmpty(exclude) ? exclude : nullptr);
    RemoveWindowSubclass(owner, OwnerProc, 0);
    UndoMenuRules(undo);
    UndoMenuRules(once);
    t_busy = false;
    if (RunMenuRule(undo, result, context)) return ours ? TRUE : 0;
    if (ours) {
        if (result && !(flags & TPM_NONOTIFY)) PostMessageW(owner, WM_COMMAND, result, 0);
        return result != 0;
    }
    return result;
}

using TrackPopupMenuEx_t = decltype(&TrackPopupMenuEx);
TrackPopupMenuEx_t TrackPopupMenuEx_Original;
BOOL WINAPI TrackPopupMenuEx_Hook(HMENU menu, UINT flags, int x, int y, HWND owner, LPTPMPARAMS params) {
    void* caller = __builtin_return_address(0);
    if (t_busy || !WantMenu(menu, owner, caller)) return TrackPopupMenuEx_Original(menu, flags, x, y, owner, params);
    return (BOOL)TrackHere(menu, flags, x, y, owner, params ? &params->rcExclude : nullptr);
}

using TrackPopupMenu_t = decltype(&TrackPopupMenu);
TrackPopupMenu_t TrackPopupMenu_Original;
BOOL WINAPI TrackPopupMenu_Hook(HMENU menu, UINT flags, int x, int y, int reserved, HWND owner, const RECT* rect) {
    void* caller = __builtin_return_address(0);
    if (t_busy || !WantMenu(menu, owner, caller)) return TrackPopupMenu_Original(menu, flags, x, y, reserved, owner, rect);
    return (BOOL)TrackHere(menu, flags, x, y, owner, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// Windows' own menus as plain ones (Windows' own menus on): the helper each part of the shell carries asks
// first whether it may draw a menu as an immersive one (ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu),
// and here the answer is no, as in Classic Menus. Hooked in each module as it loads.

using CanApplyOwnerDrawToMenu_t = bool(WINAPI*)(HMENU, HWND);
bool WINAPI CanApplyOwnerDrawToMenu_Hook(HMENU, HWND) {
    return false;
}

SRWLOCK g_hookedLock = SRWLOCK_INIT;
std::vector<HMODULE> g_hooked;  // modules looked at (guarded by g_hookedLock)
std::atomic<bool> g_initDone;

// Windows 11's new File Explorer menu skipped: the folder view asks for its presenter, and gets none, so
// it shows the full menu (m417z's way). The navigation pane decides for itself (CNscTree::ShouldShowMiniMenu).
using ShouldShowMiniMenu_t = bool(WINAPI*)(void*, void*);
ShouldShowMiniMenu_t ShouldShowMiniMenu_Original;
bool WINAPI ShouldShowMiniMenu_Hook(void* self, void* item) {
    if (GetKeyState(VK_SHIFT) < 0) return ShouldShowMiniMenu_Original ? ShouldShowMiniMenu_Original(self, item) : true;
    return false;
}

bool SkipNewMenuHere() { return g_inExplorer ? g_settings.explorer : g_settings.dialogs; }

// The modules each hook is in, by file name.
bool HookModule(HMODULE module) {
    if (!module) return false;
    AcquireSRWLockExclusive(&g_hookedLock);
    bool already = std::find(g_hooked.begin(), g_hooked.end(), module) != g_hooked.end();
    if (!already) g_hooked.push_back(module);
    ReleaseSRWLockExclusive(&g_hookedLock);
    if (already) return false;
    std::wstring name = ProgramName(module);
    bool hooked = false;
    if (SkipNewMenuHere() && !_wcsicmp(name.c_str(), L"ExplorerFrame.dll")) {
        WindhawkUtils::SYMBOL_HOOK hook[] = {
            {{LR"(private: bool __cdecl CNscTree::ShouldShowMiniMenu(struct _TREEITEM *))"},
             &ShouldShowMiniMenu_Original,
             ShouldShowMiniMenu_Hook,
             true},
        };
        hooked |= WindhawkUtils::HookSymbols(module, hook, ARRAYSIZE(hook)) && ShouldShowMiniMenu_Original;
    }
    bool helper = false;
    for (PCWSTR m : kWindowsMenuModules) helper |= !_wcsicmp(m, name.c_str());
    for (PCWSTR m : kExplorerModules) helper |= !_wcsicmp(m, name.c_str());
    if (g_settings.windowsMenus && helper) {
        static CanApplyOwnerDrawToMenu_t originals[64];
        static std::atomic<int> used;
        int slot = used++;
        if (slot < (int)ARRAYSIZE(originals)) {
            WindhawkUtils::SYMBOL_HOOK hook[] = {
                {{
#ifdef _WIN64
                     LR"(bool __cdecl ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *))"
#else
                     LR"(bool __stdcall ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *))"
#endif
                 },
                 &originals[slot],
                 CanApplyOwnerDrawToMenu_Hook,
                 true},
            };
            hooked |= WindhawkUtils::HookSymbols(module, hook, ARRAYSIZE(hook)) && originals[slot];
        }
    }
    return hooked;
}

using LoadLibraryExW_t = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR fileName, HANDLE file, DWORD flags) {
    HMODULE module = LoadLibraryExW_Original(fileName, file, flags);
    if (module && g_initDone && !((ULONG_PTR)module & 3) &&
        !(flags & (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE))) {
        if (HookModule(module)) Wh_ApplyHookOperations();
    }
    return module;
}

// The presenter the folder view asks for (shell32, CDefView::TryGetContextMenuPresenter), and the
// interface it wants (changed in KB5052093 of 24H2).
constexpr GUID kPresenterService = {0xb306c5b1, 0xb4f2, 0x473c, {0xb6, 0xff, 0x70, 0x1b, 0x24, 0x6c, 0xe2, 0xd2}};
constexpr GUID kPresenterIid = {0x706461d1, 0xac5f, 0x4730, {0xbf, 0xe3, 0xca, 0xc6, 0xca, 0xd5, 0xef, 0x5e}};
constexpr GUID kPresenterIid24H2 = {0x37a472f7, 0x63cf, 0x4ccf, {0xa8, 0x8b, 0x52, 0x31, 0xa3, 0xc7, 0xd8, 0xb6}};

using IUnknown_QueryService_t = HRESULT(WINAPI*)(IUnknown*, REFGUID, REFIID, void**);
IUnknown_QueryService_t IUnknown_QueryService_Original;
HRESULT WINAPI IUnknown_QueryService_Hook(IUnknown* object, REFGUID service, REFIID iid, void** out) {
    if (IsEqualGUID(service, kPresenterService) && (IsEqualGUID(iid, kPresenterIid) || IsEqualGUID(iid, kPresenterIid24H2)) &&
        GetKeyState(VK_SHIFT) >= 0) {
        if (out) *out = nullptr;
        return E_FAIL;
    }
    return IUnknown_QueryService_Original(object, service, iid, out);
}

////////////////////////////////////////////////////////////////////////////////

// Menu bars (Menu bars on): Windows asks the window's default procedure to draw the bar (WM_UAHDRAWMENU)
// and each of its items (WM_UAHDRAWMENUITEM) before drawing them itself; they're drawn here instead, in
// Tourne's colours, and the light line Windows leaves under the bar is painted over.
constexpr UINT kUahDrawMenu = 0x91, kUahDrawMenuItem = 0x92;
struct UahMenu {
    HMENU menu;
    HDC dc;
    DWORD flags;
};
struct UahMenuItem {
    int position;
    DWORD metrics[8];  // (sizes Windows keeps for the bar and its popups)
    DWORD popupWidths[4];
    DWORD updateWidths;
};
struct UahDrawMenuItem {
    DRAWITEMSTRUCT draw;
    UahMenu menu;
    UahMenuItem item;
};

COLORREF Rgb(uint32_t c) { return RGB((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF); }

bool BarRect(HWND wnd, RECT* bar) {
    MENUBARINFO info{sizeof(info)};
    if (!GetMenuBarInfo(wnd, OBJID_MENU, 0, &info)) return false;
    RECT window;
    GetWindowRect(wnd, &window);
    *bar = info.rcBar;
    OffsetRect(bar, -window.left, -window.top);
    return true;
}

bool DrawMenuBarHere(HWND wnd, UINT msg, LPARAM lp) {
    if (msg == kUahDrawMenu) {
        auto* m = (UahMenu*)lp;
        RECT bar;
        if (!m || !BarRect(wnd, &bar)) return false;
        LoadMenuLook(wnd);
        HBRUSH ground = CreateSolidBrush(Rgb(g_menuLook.ground));
        FillRect(m->dc, &bar, ground);
        DeleteObject(ground);
        return true;
    }
    auto* d = (UahDrawMenuItem*)lp;
    if (!d) return false;
    LoadMenuLook(wnd);
    MenuLook const& look = g_menuLook;
    WCHAR text[256] = L"";
    MENUITEMINFOW info{sizeof(info)};
    info.fMask = MIIM_STRING;
    info.dwTypeData = text;
    info.cch = ARRAYSIZE(text);
    GetMenuItemInfoW(d->menu.menu, d->item.position, TRUE, &info);
    UINT state = d->draw.itemState;
    bool hot = state & (ODS_HOTLIGHT | ODS_SELECTED), off = state & (ODS_INACTIVE | ODS_DISABLED | ODS_GRAYED);
    HBRUSH back = CreateSolidBrush(Rgb(hot ? look.hover : look.ground));
    FillRect(d->menu.dc, &d->draw.rcItem, back);
    DeleteObject(back);
    HGDIOBJ old = look.font ? SelectObject(d->menu.dc, look.font) : nullptr;
    SetBkMode(d->menu.dc, TRANSPARENT);
    SetTextColor(d->menu.dc, Rgb(off ? look.dim : hot ? look.accent : look.text));
    RECT r = d->draw.rcItem;
    DrawTextW(d->menu.dc, text, -1, &r,
              DT_CENTER | DT_SINGLELINE | DT_VCENTER | ((state & ODS_NOACCEL) ? DT_HIDEPREFIX : 0));
    if (old) SelectObject(d->menu.dc, old);
    return true;
}

// The one-pixel line Windows draws under the bar, in the menus' ground.
void CoverBarLine(HWND wnd) {
    RECT bar;
    if (!BarRect(wnd, &bar)) return;
    RECT line = {bar.left, bar.bottom, bar.right, bar.bottom + 1};
    HDC dc = GetWindowDC(wnd);
    if (!dc) return;
    LoadMenuLook(wnd);
    HBRUSH ground = CreateSolidBrush(Rgb(g_menuLook.ground));
    FillRect(dc, &line, ground);
    DeleteObject(ground);
    ReleaseDC(wnd, dc);
}

using DefWindowProcW_t = decltype(&DefWindowProcW);
DefWindowProcW_t DefWindowProcW_Original, DefWindowProcA_Original;
LRESULT MenuBarProc(DefWindowProcW_t original, HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if ((msg == kUahDrawMenu || msg == kUahDrawMenuItem) && DrawMenuBarHere(wnd, msg, lp)) return 0;
    LRESULT result = original(wnd, msg, wp, lp);
    if (msg == WM_NCPAINT || msg == WM_NCACTIVATE) CoverBarLine(wnd);
    return result;
}
LRESULT WINAPI DefWindowProcW_Hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    return MenuBarProc(DefWindowProcW_Original, wnd, msg, wp, lp);
}
LRESULT WINAPI DefWindowProcA_Hook(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    return MenuBarProc(DefWindowProcA_Original, wnd, msg, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////

// Programs Windows' own menus aren't in, or where hooking isn't wanted, and the ones set here.
bool ShouldRunHere(std::wstring const& name) {
    for (PCWSTR skip : {L"dwm.exe", L"ShellExperienceHost.exe", L"wlanext.exe", L"svchost.exe",
                        L"SearchIndexer.exe", L"consent.exe", L"LogonUI.exe", L"winlogon.exe", L"csrss.exe",
                        L"TourneDesktop.exe"}) {
        if (!_wcsicmp(name.c_str(), skip)) return false;
    }
    PCWSTR list = Wh_GetStringSetting(L"excludeApps");
    std::wstring items = list ? list : L"";
    Wh_FreeStringSetting(list);
    for (auto const& item : MenuRuleList(items))
        if (!_wcsicmp(item.c_str(), name.c_str())) return false;
    return true;
}

BOOL Wh_ModInit() {
    std::wstring name = ProgramName();
    if (!GetModuleHandleW(L"user32.dll") || !ShouldRunHere(name)) return FALSE;
    g_inExplorer = !_wcsicmp(name.c_str(), L"explorer.exe");
    // Windhawk's own process (where Tourne'Icons shows its Open dialog): its dialogs only.
    bool inWindhawk = !_wcsicmp(name.c_str(), L"windhawk.exe");
    g_settings.explorer = Wh_GetIntSetting(L"explorer") != 0;
    g_settings.dialogs = Wh_GetIntSetting(L"fileDialogs") != 0;
    g_settings.windowsMenus = Wh_GetIntSetting(L"windowsMenus") != 0 && !inWindhawk;
    g_settings.allMenus = Wh_GetIntSetting(L"allMenus") != 0 && !inWindhawk;
    g_settings.menuBars = Wh_GetIntSetting(L"menuBars") != 0;
    PCWSTR once = Wh_GetStringSetting(L"noDuplicates");
    g_settings.noDuplicates = MenuRuleList(once ? once : L"");
    Wh_FreeStringSetting(once);
    PCWSTR keep = Wh_GetStringSetting(L"noDuplicatesKeep");
    g_settings.keepLast = !keep || _wcsicmp(keep, L"first") != 0;
    Wh_FreeStringSetting(keep);
    if (g_settings.menuBars) {
        WindhawkUtils::SetFunctionHook(DefWindowProcW, DefWindowProcW_Hook, &DefWindowProcW_Original);
        WindhawkUtils::SetFunctionHook((DefWindowProcW_t)DefWindowProcA, DefWindowProcA_Hook, &DefWindowProcA_Original);
    }
    // Nothing more to do in this program for now: no hooks, but still loaded, so a switch turned on later
    // reaches programs already running (one whose init said no would need restarting).
    if (!(g_settings.explorer && g_inExplorer) && !g_settings.dialogs && !g_settings.windowsMenus && !g_settings.allMenus)
        return TRUE;

    WindhawkUtils::SetFunctionHook(TrackPopupMenuEx, TrackPopupMenuEx_Hook, &TrackPopupMenuEx_Original);
    WindhawkUtils::SetFunctionHook(TrackPopupMenu, TrackPopupMenu_Hook, &TrackPopupMenu_Original);
    if (SkipNewMenuHere()) {
        HMODULE shcore = GetModuleHandleW(L"shcore.dll");
        if (auto query = shcore ? (IUnknown_QueryService_t)GetProcAddress(shcore, "IUnknown_QueryService") : nullptr)
            WindhawkUtils::SetFunctionHook(query, IUnknown_QueryService_Hook, &IUnknown_QueryService_Original);
    }
    // The modules already loaded, and the program itself; the rest as they load.
    HookModule(GetModuleHandleW(nullptr));
    for (PCWSTR m : kWindowsMenuModules) HookModule(GetModuleHandleW(m));
    for (PCWSTR m : kExplorerModules) HookModule(GetModuleHandleW(m));
    HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
    if (auto loadLibraryExW = kernelBase ? (LoadLibraryExW_t)GetProcAddress(kernelBase, "LoadLibraryExW") : nullptr)
        WindhawkUtils::SetFunctionHook(loadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    g_initDone = true;
    return TRUE;
}

void Wh_ModUninit() {}

// A switch changed: the hooks are put in again as the settings now say.
BOOL Wh_ModSettingsChanged(BOOL* reload) {
    *reload = TRUE;
    return TRUE;
}
