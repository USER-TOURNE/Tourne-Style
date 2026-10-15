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
        if (info.fType & MFT_OWNERDRAW) continue;  // styled already, or drawn by its owner
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
// left doubled, first or last go too. UndoMenuRules puts everything back where it was, for a caller that
// keeps the menu (Explorer's).

struct MenuRules {
    std::vector<std::wstring> hide, more;  // lower case
    std::wstring moreLabel = L"More options";
    bool Any() const { return !hide.empty() || !more.empty(); }
};

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
    }
    // Separators that would be first, last or doubled once the rest is gone.
    bool any = false;
    for (int f : fate) any |= f != 0;
    if (!any) return undo;
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
    if (!undo.menu || undo.taken.empty()) return;
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
