////////////////////////////////////////////////////////////////////////////////
// The desktops pager ("// @inline desktops-pager.cpp", in taskbar-host.cpp after the YASB bar's files):
// a mark for each of Windows' virtual desktops (the ones Win+Tab makes), the one showing lit. A click
// goes to that desktop, the wheel steps through them. It sits on the main bar where it's put (after
// Start, before the tray or after the clock), and dragged off the bar it floats anywhere on the screen,
// over the bar too: a floating pager never docks by itself (its menu's places do that). Its right-click
// menu locks it in place, picks its look and adds or closes desktops; the taskbar's "Desktops pager
// look" settings give its marks' size, shape and colours, and a floating one's panel, opacity and fade.
// The desktops are read from Explorer's registry (css-data.cpp's reader; the bar's thread already
// watches the key), and its choices are kept beside the tray's, in kTrayStoreKey.

void Redraw(HWND wnd);
double NowMs();
void PaintFloatingPager();
void HidePreview();
void HideDesktopPreview();
void HideDesktopPreviewSoon();
void PagerGoTo(int target);
int RunMenu(HWND wnd, HMENU menu, UINT flags);

constexpr WCHAR kPagerClass[] = L"TourneDesktops_" WH_MOD_ID;
enum : int { kMarkPixel, kMarkSquare, kMarkCircle, kMarkDiamond, kMarkPill, kMarkBar };
constexpr UINT_PTR kPagerTimerFade = 1, kPagerTimerIdle = 2;

struct PagerChoices {
    int place = kPagerBeforeTray;
    int style = kPagerDots;
    bool locked = false;
    bool onTop = true;
    bool hideSingle = false;
    POINT at{INT_MIN, INT_MIN};  // floating: its top left on the screen (none yet)
};

struct PagerDesktops {
    int count = 1, current = 0;
    std::vector<std::wstring> names;  // as named in Task view; empty for "Desktop N"
    std::vector<GUID> ids;            // (none until a second desktop has ever been made)
    bool operator==(PagerDesktops const&) const = default;
};

PagerChoices g_pagerChoices;
PagerDesktops g_pagerDesktops;
HINSTANCE g_pagerInstance = nullptr;
HWND g_pagerWnd = nullptr;       // the floating pager
Surface* g_pagerSurface = nullptr;  // its last picture, for fading without drawing it again
std::vector<RECT> g_pagerFloatItems;
SIZE g_pagerFloatSize{};
int g_pagerFloatHot = -1, g_pagerFloatPressed = -1;
bool g_pagerFloatTracking = false;
double g_pagerFade = 1, g_pagerFadeTarget = 1;  // a floating pager's fade: 1 is its full opacity
ULONGLONG g_pagerFadeTick = 0;
double g_pagerFadeFrom = 1, g_pagerFadeStart = 0;  // on a motion line: where the fade set off, and when
HFONT g_pagerTextFont = nullptr, g_pagerTextFontOf = nullptr;  // g_font, greyscale-smoothed, for the floating one
bool g_pagerDrag = false;        // pressed on it, unlocked: may turn into carrying it
bool g_pagerCarry = false;       // being carried (shown floating under the pointer)
bool g_pagerCarryFromBar = false;  // carried off the bar: letting go on the bar docks it again
bool g_pagerOverBar = false;     // carried from the bar and over it, where letting go docks it
POINT g_pagerDragStart{};        // where the press was, in the pressed window
POINT g_pagerGrab{};             // the pointer's place in the floating pager while carried

////////////////////////////////////////////////////////////////////////////////
// Its choices and the desktops

DWORD PagerValue(PCWSTR name, DWORD fallback) {
    DWORD value = fallback, size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER, kTrayStoreKey, name, RRF_RT_REG_DWORD, nullptr, &value, &size) != ERROR_SUCCESS)
        value = fallback;
    return value;
}

void LoadPagerChoices() {
    PagerChoices c;
    c.place = (int)std::min<DWORD>(PagerValue(L"PagerPlace", c.place), kPagerFloating);
    c.style = (int)std::min<DWORD>(PagerValue(L"PagerStyle", c.style), kPagerNames);
    c.locked = PagerValue(L"PagerLocked", c.locked) != 0;
    c.onTop = PagerValue(L"PagerOnTop", c.onTop) != 0;
    c.hideSingle = PagerValue(L"PagerHideSingle", c.hideSingle) != 0;
    c.at = {(LONG)(int)PagerValue(L"PagerX", (DWORD)INT_MIN), (LONG)(int)PagerValue(L"PagerY", (DWORD)INT_MIN)};
    g_pagerChoices = c;
}

void SavePagerChoices() {
    auto put = [](PCWSTR name, DWORD value) {
        RegSetKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, name, REG_DWORD, &value, sizeof(value));
    };
    PagerChoices const& c = g_pagerChoices;
    put(L"PagerPlace", c.place);
    put(L"PagerStyle", c.style);
    put(L"PagerLocked", c.locked);
    put(L"PagerOnTop", c.onTop);
    put(L"PagerHideSingle", c.hideSingle);
    put(L"PagerX", (DWORD)c.at.x);
    put(L"PagerY", (DWORD)c.at.y);
}

PagerDesktops ReadPagerDesktops() {
    PagerDesktops d;
    std::vector<GUID> ids = CssReadGuids(L"VirtualDesktopIDs");
    std::vector<GUID> current = CssReadGuids(L"CurrentVirtualDesktop");
    d.count = std::max<int>(1, (int)ids.size());
    for (size_t i = 0; i < ids.size() && !current.empty(); i++)
        if (!memcmp(&ids[i], &current[0], sizeof(GUID))) d.current = (int)i;
    for (GUID const& id : ids) {
        WCHAR guid[40] = L"";
        StringFromGUID2(id, guid, ARRAYSIZE(guid));
        std::wstring key = std::wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops\\Desktops\\") + guid;
        WCHAR name[256] = L"";
        DWORD size = sizeof(name);
        if (RegGetValueW(HKEY_CURRENT_USER, key.c_str(), L"Name", RRF_RT_REG_SZ, nullptr, name, &size) != ERROR_SUCCESS) name[0] = 0;
        d.names.push_back(name);
    }
    d.names.resize(d.count);
    d.ids = ids;
    return d;
}

std::wstring PagerLabel(int i) {
    if (i >= 0 && i < (int)g_pagerDesktops.names.size() && !g_pagerDesktops.names[i].empty()) return g_pagerDesktops.names[i];
    return L"Desktop " + std::to_wstring(i + 1);
}

bool PagerShown() {
    return g_settings.desktops && !(g_pagerChoices.hideSingle && g_pagerDesktops.count < 2);
}

int PagerIndexOf(int hit) {
    return hit >= kHitDesktops && hit < kHitDesktops + std::min(g_pagerDesktops.count, kPagerMax) ? hit - kHitDesktops : -1;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing: shapes smoothed from 4 x 4 samples a pixel, straight into the picture's pixels. On the bar
// the picture is opaque (colours are mixed in, its alpha left as the bar's drawing needs it); floating
// it's premultiplied, so a panel can be see-through and the marks can stand without one.

struct PagerTarget {
    uint32_t* bits;
    int w, h;
    HDC dc;
    bool premul;
};

struct PagerShape {
    int kind = 0;        // 0 a rectangle with rounded corners (radius), 1 an ellipse, 2 a diamond,
    float radius = 0;    // 3 a pixel dot: a rectangle with its corner pixels (radius wide) off
};

bool InPagerShape(PagerShape s, RECT const& r, float x, float y) {
    float w = (float)(r.right - r.left), h = (float)(r.bottom - r.top);
    float dx = fabsf(x - (r.left + w / 2)), dy = fabsf(y - (r.top + h / 2));
    if (dx >= w / 2 || dy >= h / 2) return false;
    switch (s.kind) {
        case 1: return (dx * dx) / (w * w / 4) + (dy * dy) / (h * h / 4) <= 1;
        case 2: return dx / (w / 2) + dy / (h / 2) <= 1;
        case 3: return !(dx >= w / 2 - s.radius && dy >= h / 2 - s.radius);
        default: {
            float radius = std::min({s.radius, w / 2, h / 2});
            float ex = std::max(dx - (w / 2 - radius), 0.f), ey = std::max(dy - (h / 2 - radius), 0.f);
            return ex * ex + ey * ey <= radius * radius;
        }
    }
}

// A shape (less a hole in it, for an outline) in a colour, at an opacity of 0 to 255.
void PaintPagerShape(PagerTarget& t, RECT r, PagerShape s, uint32_t rgb, int alpha, RECT const* hole = nullptr,
                     PagerShape holeShape = {}) {
    if (alpha <= 0) return;
    RECT clip{std::max(0, (int)r.left), std::max(0, (int)r.top), std::min(t.w, (int)r.right), std::min(t.h, (int)r.bottom)};
    uint32_t cr = (rgb >> 16) & 0xFF, cg = (rgb >> 8) & 0xFF, cb = rgb & 0xFF;
    for (int y = clip.top; y < clip.bottom; y++)
        for (int x = clip.left; x < clip.right; x++) {
            int in = 0;
            for (int j = 0; j < 4; j++)
                for (int i = 0; i < 4; i++) {
                    float fx = x + (i + 0.5f) / 4, fy = y + (j + 0.5f) / 4;
                    if (InPagerShape(s, r, fx, fy) && !(hole && InPagerShape(holeShape, *hole, fx, fy))) in++;
                }
            if (!in) continue;
            uint32_t a = (uint32_t)alpha * in / 16;
            uint32_t& px = t.bits[(size_t)y * t.w + x];
            uint32_t pa = px >> 24, pr = (px >> 16) & 0xFF, pg = (px >> 8) & 0xFF, pb = px & 0xFF;
            if (t.premul) {
                // Premultiplied "over".
                px = ((a + pa * (255 - a) / 255) << 24) | ((cr * a / 255 + pr * (255 - a) / 255) << 16) |
                     ((cg * a / 255 + pg * (255 - a) / 255) << 8) | (cb * a / 255 + pb * (255 - a) / 255);
            } else {
                px = (px & 0xFF000000) | ((cr * a + pr * (255 - a)) / 255 << 16) | ((cg * a + pg * (255 - a)) / 255 << 8) |
                     ((cb * a + pb * (255 - a)) / 255);
            }
        }
}

// A shape with an outline: the outer colour in a ring as wide as the outline, the inner one inside.
void PaintPagerMark(PagerTarget& t, RECT r, PagerShape s, uint32_t inner, uint32_t outer, int outline, int alpha = 255) {
    if (outline > 0) {
        RECT in = r;
        InflateRect(&in, -outline, -outline);
        PagerShape innerShape = s;
        if (innerShape.kind == 0) innerShape.radius = std::max(0.f, s.radius - outline);
        PaintPagerShape(t, r, s, outer, alpha, &in, innerShape);
        if (in.right > in.left && in.bottom > in.top) PaintPagerShape(t, in, innerShape, inner, alpha);
        return;
    }
    PaintPagerShape(t, r, s, inner, alpha);
}

uint32_t MixRgb(uint32_t a, uint32_t b, int towardB) {  // towardB of 255
    auto ch = [&](int shift) { return (((a >> shift) & 0xFF) * (255 - towardB) + ((b >> shift) & 0xFF) * towardB) / 255 << shift; };
    return ch(16) | ch(8) | ch(0);
}

struct PagerColors {
    uint32_t mark, markEdge, current, currentEdge, hot, panel, panelEdge;
};

PagerColors PagerColorsNow() {
    auto const& l = g_settings.pagerLook;
    auto const& p = g_settings.palette;
    if (l.custom)
        return {l.mark, l.markOutline, l.current, l.currentOutline, MixRgb(l.mark, l.current, 128), l.panel, l.panelEdge};
    return {p.dim, p.line, p.accent, p.text, p.text, p.ground, g_pagerChoices.locked ? p.line : p.edge};
}

// Names don't fit down a vertical run: numbers there.
int PagerStyleFor(bool vertical) {
    return vertical && g_pagerChoices.style == kPagerNames ? kPagerNumbers : g_pagerChoices.style;
}

int PagerMarkSize(bool current) { return Px(g_settings.pagerLook.size + (current ? 2 : 0)); }

// A dot's size along and across the run.
SIZE PagerDotSize(bool current) {
    int s = PagerMarkSize(current);
    switch (g_settings.pagerLook.shape) {
        case kMarkPill: return {current ? 2 * s : s, s};
        case kMarkBar: return {2 * s, std::max(Px(2), s / 2)};
        default: return {s, s};
    }
}

int PagerBoxSize() { return std::max(Px(18), Px(g_settings.pagerLook.size * 3)); }

// The box behind a number or a name, shaped like the marks.
PagerShape PagerBoxShape(RECT const& box) {
    int h = std::min(box.right - box.left, box.bottom - box.top);
    switch (g_settings.pagerLook.shape) {
        case kMarkPixel: return {3, (float)g_scale};
        case kMarkCircle:
        case kMarkPill: return {0, h / 2.f};
        case kMarkDiamond: return {0, (float)Px(3)};
        default: return {0, 0};
    }
}

std::vector<int> PagerItemLengths(bool vertical) {
    std::vector<int> lengths;
    int count = std::min(g_pagerDesktops.count, kPagerMax);
    int style = PagerStyleFor(vertical);
    int spacing = Px(g_settings.pagerLook.spacing);
    HDC dc = style == kPagerNames ? GetDC(nullptr) : nullptr;
    HGDIOBJ old = dc ? SelectObject(dc, g_font) : nullptr;
    for (int i = 0; i < count; i++) {
        if (style == kPagerDots) {
            lengths.push_back(PagerDotSize(i == g_pagerDesktops.current).cx + spacing);
        } else if (style == kPagerNumbers) {
            lengths.push_back(PagerBoxSize() + spacing / 2);
        } else {
            std::wstring label = PagerLabel(i);
            SIZE extent{};
            GetTextExtentPoint32W(dc, label.c_str(), (int)label.size(), &extent);
            lengths.push_back(std::min((int)extent.cx, Px(160)) + Px(8) + spacing);
        }
    }
    if (dc) {
        SelectObject(dc, old);
        ReleaseDC(nullptr, dc);
    }
    return lengths;
}

// How thick the run is across, floating.
int PagerAcross() {
    switch (PagerStyleFor(g_settings.pagerLook.vertical)) {
        case kPagerDots: return std::max(Px(28), (int)PagerDotSize(true).cy + Px(14));
        case kPagerNumbers: return PagerBoxSize() + Px(10);
        default: {
            HDC dc = GetDC(nullptr);
            HGDIOBJ old = SelectObject(dc, g_font);
            TEXTMETRICW tm{};
            GetTextMetricsW(dc, &tm);
            SelectObject(dc, old);
            ReleaseDC(nullptr, dc);
            return std::max(Px(28), (int)tm.tmHeight + Px(14));
        }
    }
}

// The marks along an area, centred across it.
std::vector<RECT> LayPagerItems(RECT area, bool vertical, int lead) {
    std::vector<RECT> items;
    int at = (vertical ? area.top : area.left) + lead;
    for (int length : PagerItemLengths(vertical)) {
        items.push_back(vertical ? RECT{area.left, at, area.right, at + length} : RECT{at, area.top, at + length, area.bottom});
        at += length;
    }
    return items;
}

// A grey-smoothed copy of the bar's font: text drawn white on black into a mask gives its coverage,
// which a see-through picture needs (ClearType's colour fringes don't make a coverage).
HFONT PagerTextFont() {
    if (g_pagerTextFontOf != g_font || !g_pagerTextFont) {
        if (g_pagerTextFont) DeleteObject(g_pagerTextFont);
        LOGFONTW lf{};
        GetObjectW(g_font, sizeof(lf), &lf);
        lf.lfQuality = ANTIALIASED_QUALITY;
        g_pagerTextFont = CreateFontIndirectW(&lf);
        g_pagerTextFontOf = g_font;
    }
    return g_pagerTextFont;
}

void PagerText(PagerTarget& t, RECT rc, std::wstring const& text, uint32_t rgb) {
    if (!t.premul) {
        SetBkMode(t.dc, TRANSPARENT);
        SetTextColor(t.dc, PaletteRef(rgb));
        DrawTextW(t.dc, text.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        GdiFlush();
        return;
    }
    RECT clip{std::max(0, (int)rc.left), std::max(0, (int)rc.top), std::min(t.w, (int)rc.right), std::min(t.h, (int)rc.bottom)};
    if (clip.right <= clip.left || clip.bottom <= clip.top) return;
    int w = clip.right - clip.left, h = clip.bottom - clip.top;
    Surface mask(t.dc, w, h);
    RECT local{rc.left - clip.left, rc.top - clip.top, rc.right - clip.left, rc.bottom - clip.top};
    HGDIOBJ old = SelectObject(mask.dc, PagerTextFont());
    SetBkMode(mask.dc, TRANSPARENT);
    SetTextColor(mask.dc, RGB(255, 255, 255));
    DrawTextW(mask.dc, text.c_str(), -1, &local, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    SelectObject(mask.dc, old);
    GdiFlush();
    uint32_t cr = (rgb >> 16) & 0xFF, cg = (rgb >> 8) & 0xFF, cb = rgb & 0xFF;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            uint32_t m = mask.bits[(size_t)y * w + x];
            uint32_t a = std::max({(m >> 16) & 0xFF, (m >> 8) & 0xFF, m & 0xFF});
            if (!a) continue;
            uint32_t& px = t.bits[(size_t)(y + clip.top) * t.w + x + clip.left];
            uint32_t pa = px >> 24, pr = (px >> 16) & 0xFF, pg = (px >> 8) & 0xFF, pb = px & 0xFF;
            px = ((a + pa * (255 - a) / 255) << 24) | ((cr * a / 255 + pr * (255 - a) / 255) << 16) |
                 ((cg * a / 255 + pg * (255 - a) / 255) << 8) | (cb * a / 255 + pb * (255 - a) / 255);
        }
}

// The lit desktop moving to the one gone to, on the motion engine's "workspacesIn" line (or a parent's):
// slid there (slide, slidevert, or no style), or faded across ("fade": the one left fading out on
// "workspacesOut"). Its frames come from a timer of the bar's thread, only while it moves (PagerMoveTick).
struct PagerMove {
    UINT_PTR timer = 0;      // (none: still)
    bool fade = false;
    double from = 0;         // where it set off, in desktops (between two, when a move was cut short)
    double start = 0;
    double at = 0;           // slide: where it is now
    double in = 1, out = 0;  // fade: how lit the one gone to and the one left are
};
PagerMove g_pagerMove;

// The move as drawn now: none if still, or if the desktops it was between are gone.
bool PagerMoving(int count) {
    auto const& m = g_pagerMove;
    int to = g_pagerDesktops.current;
    return m.timer && count > 1 && to >= 0 && to < count && m.from >= 0 && m.from <= count - 1;
}

RECT LerpRect(RECT const& a, RECT const& b, double f) {
    auto mix = [f](LONG x, LONG y) { return (LONG)std::lround(x + (y - x) * f); };
    return {mix(a.left, b.left), mix(a.top, b.top), mix(a.right, b.right), mix(a.bottom, b.bottom)};
}

void DrawPagerItems(PagerTarget& t, std::vector<RECT> const& items, int hot, int pressed, bool vertical) {
    auto const& l = g_settings.pagerLook;
    PagerColors c = PagerColorsNow();
    int style = PagerStyleFor(vertical);
    int outline = Px(l.outline);
    int count = (int)items.size();
    bool moving = PagerMoving(count);
    // A desktop's dot (lit or not), its shape, and the box behind a number or a name.
    auto dotOf = [&](RECT const& r, bool current) {
        SIZE d = PagerDotSize(current);
        int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
        int w = vertical ? d.cy : d.cx, h = vertical ? d.cx : d.cy;
        return RECT{cx - w / 2, cy - h / 2, cx - w / 2 + w, cy - h / 2 + h};
    };
    auto dotShape = [&](RECT const& dot) {
        int w = dot.right - dot.left, h = dot.bottom - dot.top;
        return l.shape == kMarkCircle    ? PagerShape{1, 0}
               : l.shape == kMarkDiamond ? PagerShape{2, 0}
               : l.shape == kMarkPixel   ? PagerShape{3, (float)g_scale}
               : l.shape == kMarkPill    ? PagerShape{0, std::min(w, h) / 2.f}
                                         : PagerShape{0, 0};
    };
    auto boxOf = [&](RECT const& r) {
        RECT box = r;
        int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
        if (style == kPagerNumbers) {
            int s = std::min({PagerBoxSize(), (int)(r.right - r.left) - Px(2), (int)(r.bottom - r.top) - Px(4)});
            box = {cx - s / 2, cy - s / 2, cx - s / 2 + s, cy - s / 2 + s};
        } else {
            int spacing = Px(l.spacing);
            InflateRect(&box, -spacing / 2, -Px(4));
        }
        return box;
    };
    GdiFlush();
    for (int i = 0; i < count; i++) {
        RECT r = items[i];
        bool current = i == g_pagerDesktops.current && !moving;  // (moving: lit where the move has it, below)
        bool lit = i == hot || i == pressed;
        if (style == kPagerDots) {
            RECT dot = dotOf(r, current);
            int w = dot.right - dot.left, h = dot.bottom - dot.top;
            uint32_t inner = current ? c.current : lit ? c.hot : c.mark;
            PaintPagerMark(t, dot, dotShape(dot), inner, current ? c.currentEdge : c.markEdge, std::min(outline, std::min(w, h) / 2));
            continue;
        }
        RECT box = boxOf(r);
        PagerShape shape = PagerBoxShape(box);
        uint32_t text = c.mark;
        if (current && style == kPagerNumbers) {
            PaintPagerMark(t, box, shape, c.current, c.currentEdge, outline);
            text = c.panel;
        } else if (current) {
            PaintPagerMark(t, box, shape, MixRgb(c.panel, c.current, 48), c.currentEdge, outline);
            RECT line = vertical ? RECT{box.left, box.top, box.left + Px(2), box.bottom} : RECT{box.left, box.bottom - Px(2), box.right, box.bottom};
            PaintPagerShape(t, line, {}, c.current, 255);
            text = c.hot;
        } else if (lit) {
            PaintPagerMark(t, box, shape, MixRgb(c.panel, c.mark, 64), c.markEdge, outline);
            text = c.hot;
        } else if (outline) {
            PaintPagerMark(t, box, shape, c.panel, c.markEdge, outline, 255);
        }
        if (moving) continue;  // (its text goes over the lit mark, below)
        RECT tr = box;
        InflateRect(&tr, style == kPagerNames ? -Px(4) : 0, 0);
        PagerText(t, tr, style == kPagerNumbers ? std::to_wstring(i + 1) : PagerLabel(i), text);
    }
    if (!moving) return;

    // The lit mark where the move has it: one sliding between two desktops' places, or two fading.
    auto const& m = g_pagerMove;
    int to = g_pagerDesktops.current, left = (int)std::lround(m.from);
    auto place = [&](int i) { return style == kPagerDots ? dotOf(items[i], true) : boxOf(items[i]); };
    std::vector<std::pair<RECT, double>> marks;  // where, and how lit (0 to 1)
    if (m.fade) {
        if (left != to) marks.push_back({place(left), std::clamp(m.out, 0.0, 1.0)});
        marks.push_back({place(to), std::clamp(m.in, 0.0, 1.0)});
    } else {
        int i = std::clamp((int)std::floor(m.at), 0, count - 2);
        marks.push_back({LerpRect(place(i), place(i + 1), m.at - i), 1.0});
    }
    for (auto const& [rc, level] : marks) {
        int alpha = (int)std::lround(255 * level), w = rc.right - rc.left, h = rc.bottom - rc.top;
        if (style == kPagerDots) {
            PaintPagerMark(t, rc, dotShape(rc), c.current, c.currentEdge, std::min(outline, std::min(w, h) / 2), alpha);
        } else if (style == kPagerNumbers) {
            PaintPagerMark(t, rc, PagerBoxShape(rc), c.current, c.currentEdge, outline, alpha);
        } else {
            PaintPagerMark(t, rc, PagerBoxShape(rc), MixRgb(c.panel, c.current, 48), c.currentEdge, outline, alpha);
            RECT line = vertical ? RECT{rc.left, rc.top, rc.left + Px(2), rc.bottom} : RECT{rc.left, rc.bottom - Px(2), rc.right, rc.bottom};
            PaintPagerShape(t, line, {}, c.current, alpha);
        }
    }
    if (style == kPagerDots) return;
    // The numbers or names over it, in the lit colour as much as the lit mark covers them.
    for (int i = 0; i < count; i++) {
        RECT box = boxOf(items[i]);
        double covered = 0;
        for (auto const& [rc, level] : marks) {
            RECT over;
            if (!IntersectRect(&over, &box, &rc)) continue;
            double span = vertical ? box.bottom - box.top : box.right - box.left;
            double part = vertical ? over.bottom - over.top : over.right - over.left;
            covered = std::max(covered, span > 0 ? level * part / span : 0);
        }
        bool lit = i == hot || i == pressed;
        uint32_t plain = lit ? c.hot : c.mark, onMark = style == kPagerNumbers ? c.panel : c.hot;
        RECT tr = box;
        InflateRect(&tr, style == kPagerNames ? -Px(4) : 0, 0);
        PagerText(t, tr, style == kPagerNumbers ? std::to_wstring(i + 1) : PagerLabel(i),
                  MixRgb(plain, onMark, (int)std::lround(255 * std::clamp(covered, 0.0, 1.0))));
    }
}

////////////////////////////////////////////////////////////////////////////////
// Docked on the main bar (Layout gives it its room)

int DockedPagerPlace() { return g_pagerChoices.place; }

int DockedPagerLength() {
    if (!PagerShown() || g_pagerChoices.place == kPagerFloating || (g_pagerCarry && g_pagerCarryFromBar) || !IsMainBar() ||
        CssActive())
        return 0;
    int length = Px(4);
    for (int l : PagerItemLengths(Vertical())) length += l;
    return length;
}

void PlaceDockedPager(RECT area) {
    g_pagerRect = area;
    g_pagerItems = LayPagerItems(area, Vertical(), Px(2));
}

void DrawDockedPager(HDC dc, uint32_t* bits, int w, int h) {
    if (IsRectEmpty(&g_pagerRect)) return;
    PagerTarget t{bits, w, h, dc, false};
    DrawPagerItems(t, g_pagerItems, PagerIndexOf(g_hot), PagerIndexOf(g_pressed), Vertical());
}

int DockedPagerHit(POINT pt) {
    if (IsRectEmpty(&g_pagerRect) || !PtInRect(&g_pagerRect, pt)) return -1;
    for (int i = 0; i < (int)g_pagerItems.size(); i++)
        if (PtInRect(&g_pagerItems[i], pt)) return kHitDesktops + i;
    return -1;
}

// Where on the bar a pager carried off it and let go there docks again: the nearest of its three places.
// (While it's carried its room on the bar is given back, so the places are where they'd be without it.)
int NearestPagerPlace(POINT pt) {
    bool vertical = Vertical();
    auto start = [&](RECT const& r) { return (int)(vertical ? r.top : r.left); };
    auto end = [&](RECT const& r) { return (int)(vertical ? r.bottom : r.right); };
    RECT bar;
    GetClientRect(g_wnd, &bar);
    int barEnd = IsRectEmpty(&g_showDesktopRect) ? end(bar) : start(g_showDesktopRect);
    bool clock = !IsRectEmpty(&g_clockRect);
    int beforeTray = !g_tray.empty() ? start(g_tray[0].rc) : clock ? start(g_clockRect) : barEnd;
    int afterClock = clock ? end(g_clockRect) : !g_tray.empty() ? end(g_tray.back().rc) : barEnd;
    int along = vertical ? pt.y : pt.x;
    std::vector<std::pair<int, int>> places{{kPagerBeforeTray, beforeTray}, {kPagerAfterClock, afterClock}};
    if (!IsRectEmpty(&g_orbRect)) places.push_back({kPagerAfterStart, end(g_orbRect)});
    int best = kPagerBeforeTray, distance = INT_MAX;
    for (auto const& [place, at] : places)
        if (abs(along - at) < distance) {
            distance = abs(along - at);
            best = place;
        }
    return best;
}

////////////////////////////////////////////////////////////////////////////////
// Floating

// Its opacity now: as set, times the fade, and see-through while carried over the bar to dock.
BYTE FloatingPagerAlpha() {
    double alpha = 255.0 * g_settings.pagerLook.opacity / 100 * g_pagerFade;
    if (g_pagerOverBar) alpha = std::min(alpha, 150.0);
    return (BYTE)std::clamp(alpha, 0.0, 255.0);
}

// Shown at its opacity now, from the last picture.
void PresentFloatingPager() {
    if (!g_pagerWnd || !g_pagerSurface) return;
    RECT wr;
    GetWindowRect(g_pagerWnd, &wr);
    POINT dst{wr.left, wr.top}, src{0, 0};
    SIZE size{g_pagerSurface->w, g_pagerSurface->h};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, FloatingPagerAlpha(), AC_SRC_ALPHA};
    HDC screen = GetDC(nullptr);
    UpdateLayeredWindow(g_pagerWnd, screen, &dst, &size, g_pagerSurface->dc, &src, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
}

void PaintFloatingPager() {
    if (!g_pagerWnd) return;
    BarScope scope(&g_mainBar);
    int w = g_pagerFloatSize.cx, h = g_pagerFloatSize.cy;
    if (w <= 0 || h <= 0) return;
    auto const& l = g_settings.pagerLook;
    if (!g_pagerSurface || g_pagerSurface->w != w || g_pagerSurface->h != h) {
        delete g_pagerSurface;
        HDC screen = GetDC(nullptr);
        g_pagerSurface = new Surface(screen, w, h);
        ReleaseDC(nullptr, screen);
    }
    Surface& s = *g_pagerSurface;
    GdiFlush();
    memset(s.bits, 0, (size_t)w * h * 4);
    PagerTarget t{s.bits, w, h, s.dc, true};
    // The panel: its edge a pixel wide in the edge colour, the rest in its own, at the panel's opacity.
    if (l.panelShape != 3) {
        PagerColors c = PagerColorsNow();
        RECT rc{0, 0, w, h};
        float radius = l.panelShape == 1 ? 0.f : l.panelShape == 2 ? std::min(w, h) / 2.f : (float)Px(4);
        PaintPagerMark(t, rc, {0, radius}, c.panel, c.panelEdge, g_scale, 255 * l.panelOpacity / 100);
    }
    DrawPagerItems(t, g_pagerFloatItems, g_pagerFloatHot, g_pagerFloatPressed, l.vertical);
    PresentFloatingPager();
}

// Its marks and size, from the desktops and the look.
void MeasureFloatingPager() {
    BarScope scope(&g_mainBar);
    bool vertical = g_settings.pagerLook.vertical;
    int across = PagerAcross();
    // A pill-shaped panel keeps the marks clear of its round ends.
    int lead = g_settings.pagerLook.panelShape == 2 ? across / 2 : Px(6);
    RECT area = vertical ? RECT{0, 0, across, 0} : RECT{0, 0, 0, across};
    g_pagerFloatItems = LayPagerItems(area, vertical, lead);
    int length = g_pagerFloatItems.empty() ? Px(24) : (int)(vertical ? g_pagerFloatItems.back().bottom : g_pagerFloatItems.back().right) + lead;
    g_pagerFloatSize = vertical ? SIZE{across, length} : SIZE{length, across};
}

// A top left that keeps the whole pager on a screen: the whole screen, the taskbar's part of it included.
POINT KeepPagerOnScreen(POINT at) {
    MONITORINFO mi{sizeof(mi)};
    POINT middle{at.x + g_pagerFloatSize.cx / 2, at.y + g_pagerFloatSize.cy / 2};
    GetMonitorInfoW(MonitorFromPoint(middle, MONITOR_DEFAULTTONEAREST), &mi);
    RECT const& screen = mi.rcMonitor;
    at.x = std::clamp<LONG>(at.x, screen.left, std::max<LONG>(screen.left, screen.right - g_pagerFloatSize.cx));
    at.y = std::clamp<LONG>(at.y, screen.top, std::max<LONG>(screen.top, screen.bottom - g_pagerFloatSize.cy));
    return at;
}

// The fade's motion line: "fadeLayersIn" coming up, "fadeLayersOut" going down (or a parent's). None set: its
// own steady fade.
motion::Anim const& PagerFadeMotion() {
    return g_pagerFadeTarget >= g_pagerFadeFrom ? g_settings.motionLayersIn : g_settings.motionLayersOut;
}

// The fade: towards its target a step at a time, then still.
void StartPagerFade(double target) {
    g_pagerFadeTarget = target;
    if (fabs(g_pagerFade - target) < 0.001 || !g_pagerWnd) return;
    g_pagerFadeTick = GetTickCount64();
    g_pagerFadeFrom = g_pagerFade;
    g_pagerFadeStart = NowMs();
    auto const& a = PagerFadeMotion();
    if (a.set && (!a.on || motion::Ms(a) <= 0)) {
        // Its line is off: there at once.
        g_pagerFade = target;
        KillTimer(g_pagerWnd, kPagerTimerFade);
        PresentFloatingPager();
        return;
    }
    SetTimer(g_pagerWnd, kPagerTimerFade, 15, nullptr);
}

void PagerFadeStep() {
    if (auto const& a = PagerFadeMotion(); a.set) {
        double ms = std::min(motion::Ms(a), 3000.0);
        double t = ms > 0 && a.on ? (NowMs() - g_pagerFadeStart) / ms : 1;
        double eased = t >= 1 ? 1 : motion::Ease(a, t);
        g_pagerFade = std::clamp(g_pagerFadeFrom + (g_pagerFadeTarget - g_pagerFadeFrom) * eased, 0.0, 1.0);
        if (t >= 1) {
            g_pagerFade = g_pagerFadeTarget;
            KillTimer(g_pagerWnd, kPagerTimerFade);
        }
        PresentFloatingPager();
        return;
    }
    ULONGLONG now = GetTickCount64();
    double step = (now - g_pagerFadeTick) / 220.0;  // the whole way in about a fifth of a second
    g_pagerFadeTick = now;
    g_pagerFade = g_pagerFade < g_pagerFadeTarget ? std::min(g_pagerFadeTarget, g_pagerFade + step)
                                                  : std::max(g_pagerFadeTarget, g_pagerFade - step);
    if (fabs(g_pagerFade - g_pagerFadeTarget) < 0.001) KillTimer(g_pagerWnd, kPagerTimerFade);
    PresentFloatingPager();
}

// Faded, or not, as the setting and the pointer say.
void SettlePagerFade() {
    if (!g_pagerWnd) return;
    KillTimer(g_pagerWnd, kPagerTimerIdle);
    if (!g_settings.pagerLook.fade || g_pagerCarry || g_pagerFloatTracking) {
        g_pagerFade = g_pagerFadeTarget = 1;
        KillTimer(g_pagerWnd, kPagerTimerFade);
        return;
    }
    SetTimer(g_pagerWnd, kPagerTimerIdle, std::max(1, g_settings.pagerLook.fadeDelay), nullptr);
}

void ShowFloatingPager(POINT at, bool keepOnScreen = true) {
    MeasureFloatingPager();
    if (keepOnScreen) at = KeepPagerOnScreen(at);
    if (!g_pagerWnd) {
        // A tool window: on every virtual desktop, as the bar is, and never on the taskbar or in Alt+Tab.
        g_pagerWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED, kPagerClass, L"Desktops", WS_POPUP,
                                     at.x, at.y, g_pagerFloatSize.cx, g_pagerFloatSize.cy, nullptr, nullptr, g_pagerInstance, nullptr);
        if (!g_pagerWnd) return;
        KeepOutOfPeek(g_pagerWnd);
        g_pagerFade = g_pagerFadeTarget = 1;
    }
    // On top: owned by the bar, so it always stays above it, even laid over it. Otherwise windows cover it.
    bool top = g_pagerChoices.onTop || g_pagerCarry;
    HWND bar = g_wnd;
    SetWindowLongPtrW(g_pagerWnd, GWLP_HWNDPARENT, (LONG_PTR)(top ? bar : nullptr));
    SetWindowPos(g_pagerWnd, top ? HWND_TOPMOST : HWND_NOTOPMOST, at.x, at.y, g_pagerFloatSize.cx, g_pagerFloatSize.cy,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    PaintFloatingPager();
    if (!g_pagerCarry) SettlePagerFade();
}

void HideFloatingPager() {
    if (g_pagerWnd) {
        ShowWindow(g_pagerWnd, SW_HIDE);
        KillTimer(g_pagerWnd, kPagerTimerFade);
        KillTimer(g_pagerWnd, kPagerTimerIdle);
    }
    g_pagerFloatHot = g_pagerFloatPressed = -1;
}

// The main bar laid out again (the pager came, went or changed size there).
void RelayoutPagerBar() {
    HWND bar = g_wnd;
    if (!bar) return;
    BarScope scope(&g_mainBar);
    Layout(bar);
    Redraw(bar);
}

// The floating pager shown where its choices put it, or hidden when it's docked or off.
void PlaceFloatingPager() {
    if (g_pagerCarry) return;
    bool floating = PagerShown() && g_pagerChoices.place == kPagerFloating;
    if (floating) {
        POINT at = g_pagerChoices.at;
        if (at.x == INT_MIN || at.y == INT_MIN) {
            // Never placed yet: above the middle of the main screen's bottom.
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
            at = {(mi.rcWork.left + mi.rcWork.right) / 2, mi.rcWork.bottom - Px(80)};
        }
        ShowFloatingPager(at);
    } else {
        HideFloatingPager();
    }
}

// The pager where its choices put it, after they or the desktops changed.
void ApplyPager() {
    if (g_pagerCarry) return;
    PlaceFloatingPager();
    RelayoutPagerBar();
}

////////////////////////////////////////////////////////////////////////////////
// Carrying it. Off the bar it becomes a floating pager under the pointer; let go on the bar it docks at
// the nearest place, anywhere else it floats there. A floating pager carried about stays floating
// wherever it's let go, over the bar too.

void StartPagerCarry(HWND from, POINT pt) {
    g_pagerCarry = true;
    g_pagerCarryFromBar = from != g_pagerWnd;
    g_pagerFade = g_pagerFadeTarget = 1;
    HidePreview();
    HideDesktopPreview();
    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
    POINT screen = pt;
    ClientToScreen(from, &screen);
    if (!g_pagerCarryFromBar) {
        g_pagerGrab = pt;
        KillTimer(g_pagerWnd, kPagerTimerIdle);
        KillTimer(g_pagerWnd, kPagerTimerFade);
        PresentFloatingPager();
        return;
    }
    // From the bar: held by the same mark, as near as the floating one's run allows.
    MeasureFloatingPager();
    int index = PagerIndexOf(DockedPagerHit(pt));
    RECT const* mark = index >= 0 && index < (int)g_pagerFloatItems.size() ? &g_pagerFloatItems[index] : nullptr;
    g_pagerGrab = mark ? POINT{(mark->left + mark->right) / 2, (mark->top + mark->bottom) / 2}
                       : POINT{g_pagerFloatSize.cx / 2, g_pagerFloatSize.cy / 2};
    g_pressed = -2;
    ShowFloatingPager({screen.x - g_pagerGrab.x, screen.y - g_pagerGrab.y}, false);
    RelayoutPagerBar();  // its room on the bar closes while it's carried
}

void CarryPager(POINT screen) {
    if (!g_pagerWnd) return;
    SetWindowPos(g_pagerWnd, nullptr, screen.x - g_pagerGrab.x, screen.y - g_pagerGrab.y, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    if (!g_pagerCarryFromBar) return;
    HWND bar = g_wnd;
    RECT br{};
    if (bar) GetWindowRect(bar, &br);
    bool over = bar && IsWindowVisible(bar) && !CssActive() && PtInRect(&br, screen);
    if (over != g_pagerOverBar) {
        g_pagerOverBar = over;
        PresentFloatingPager();
    }
}

void DropPager(POINT screen) {
    g_pagerCarry = false;
    bool dock = g_pagerCarryFromBar && g_pagerOverBar;
    g_pagerOverBar = false;
    g_pagerCarryFromBar = false;
    if (dock) {
        BarScope scope(&g_mainBar);
        POINT pt = screen;
        ScreenToClient(g_wnd, &pt);
        g_pagerChoices.place = NearestPagerPlace(pt);
    } else if (g_pagerWnd) {
        RECT wr;
        GetWindowRect(g_pagerWnd, &wr);
        g_pagerChoices.place = kPagerFloating;
        g_pagerChoices.at = KeepPagerOnScreen({wr.left, wr.top});
    }
    SavePagerChoices();
    ApplyPager();
}

// Let go of some other way (the capture taken away): back where it was.
void CancelPagerCarry() {
    g_pagerCarry = false;
    g_pagerOverBar = false;
    g_pagerCarryFromBar = false;
    ApplyPager();
}

////////////////////////////////////////////////////////////////////////////////
// Previews: pointing at a mark for a moment shows that desktop as Task view would, its wallpaper with its
// windows in their places, live (DWM thumbnails, which windows on other desktops still have, cloaked as
// they are). Only while it shows: nothing is kept or watched otherwise. It stays while the pointer is on it,
// and a click on it goes to that desktop. Leaving the mark starts a short pause before it closes, so the
// pointer can cross the gap onto it.

constexpr WCHAR kDesktopPreviewClass[] = L"TourneDesktopPreview_" WH_MOD_ID;
constexpr UINT_PTR kTimerDesktopPreview = 21;  // on the bar or the floating pager: the pause before one shows
constexpr UINT_PTR kTimerDesktopPreviewHide = 22;  // on the preview: the pause after the pointer left its mark
constexpr DWORD kDesktopPreviewHideDelay = 250;

struct DesktopPreview {
    HWND wnd = nullptr;
    int desktop = -1;          // the one showing, or -1
    RECT area{};               // where the desktop is drawn, in the preview
    std::vector<HTHUMBNAIL> thumbnails;
    // The wallpaper, fitted to the area, kept for the next one at the same size.
    std::wstring wallpaperPath;
    SIZE wallpaperSize{};
    HBITMAP wallpaper = nullptr;
    // One waiting to show (after the pause).
    int pending = -1;
    RECT pendingAnchor{};
    bool pendingFromBar = false;
    HWND timerWnd = nullptr;
    bool tracking = false;  // the pointer's leave is asked for on the preview
};
DesktopPreview g_deskPreview;

// A desktop's own wallpaper (Windows keeps one per desktop once it's set), else the desktop's.
std::wstring DesktopWallpaper(int desktop) {
    if (desktop >= 0 && desktop < (int)g_pagerDesktops.ids.size()) {
        WCHAR guid[40] = L"";
        StringFromGUID2(g_pagerDesktops.ids[desktop], guid, ARRAYSIZE(guid));
        std::wstring key = std::wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops\\Desktops\\") + guid;
        WCHAR path[MAX_PATH * 2] = L"";
        DWORD size = sizeof(path);
        if (RegGetValueW(HKEY_CURRENT_USER, key.c_str(), L"Wallpaper", RRF_RT_REG_SZ, nullptr, path, &size) == ERROR_SUCCESS &&
            *path && GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES)
            return path;
    }
    WCHAR path[MAX_PATH] = L"";
    SystemParametersInfoW(SPI_GETDESKWALLPAPER, ARRAYSIZE(path), path, 0);
    return path;
}

// A picture cropped to cover a size and scaled to it (as Windows' "Fill"), premultiplied; null if it can't be read.
HBITMAP LoadCoverPicture(std::wstring const& path, int w, int h) {
    if (path.empty() || w <= 0 || h <= 0) return nullptr;
    // The pager's thread may have no COM yet; without it the factory fails (CO_E_NOTINITIALIZED) and the preview
    // falls back to a flat colour.
    HRESULT co = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IWICImagingFactory* factory = nullptr;
    HRESULT made = CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory),
                                    (void**)&factory);
    if (FAILED(made)) {
        Wh_Log(L"pager: no picture factory (0x%08X) for %s", (unsigned)made, path.c_str());
        if (SUCCEEDED(co)) CoUninitialize();
        return nullptr;
    }
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICBitmapClipper* clipper = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    IWICFormatConverter* converter = nullptr;
    HBITMAP bitmap = nullptr;
    UINT sw = 0, sh = 0;
    bool ok = SUCCEEDED(factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand,
                                                           &decoder)) &&
              SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(frame->GetSize(&sw, &sh)) && sw && sh;
    if (ok) {
        double scale = std::max((double)w / sw, (double)h / sh);
        INT cw = std::min((INT)sw, (INT)std::ceil(w / scale)), ch = std::min((INT)sh, (INT)std::ceil(h / scale));
        WICRect crop{((INT)sw - cw) / 2, ((INT)sh - ch) / 2, cw, ch};
        ok = SUCCEEDED(factory->CreateBitmapClipper(&clipper)) && SUCCEEDED(clipper->Initialize(frame, &crop)) &&
             SUCCEEDED(factory->CreateBitmapScaler(&scaler)) &&
             SUCCEEDED(scaler->Initialize(clipper, w, h, WICBitmapInterpolationModeFant)) &&
             SUCCEEDED(factory->CreateFormatConverter(&converter)) &&
             SUCCEEDED(converter->Initialize(scaler, kWicPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0,
                                             WICBitmapPaletteTypeCustom));
    }
    if (ok) {
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
        void* bits = nullptr;
        bitmap = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (bitmap && FAILED(converter->CopyPixels(nullptr, w * 4, w * h * 4, (BYTE*)bits))) {
            DeleteObject(bitmap);
            bitmap = nullptr;
        }
    }
    for (IUnknown* p : std::initializer_list<IUnknown*>{converter, scaler, clipper, frame, decoder})
        if (p) p->Release();
    factory->Release();
    if (!bitmap) Wh_Log(L"pager: couldn't read %s", path.c_str());
    if (SUCCEEDED(co)) CoUninitialize();
    return bitmap;
}

void ClearDesktopThumbnails() {
    for (HTHUMBNAIL thumbnail : g_deskPreview.thumbnails) DwmUnregisterThumbnail(thumbnail);
    g_deskPreview.thumbnails.clear();
}

void HideDesktopPreview() {
    if (g_deskPreview.timerWnd) KillTimer(g_deskPreview.timerWnd, kTimerDesktopPreview);
    if (g_deskPreview.wnd) KillTimer(g_deskPreview.wnd, kTimerDesktopPreviewHide);
    g_deskPreview.pending = -1;
    g_deskPreview.timerWnd = nullptr;
    ClearDesktopThumbnails();
    if (g_deskPreview.wnd) ShowWindow(g_deskPreview.wnd, SW_HIDE);
    g_deskPreview.desktop = -1;
}

// Closes the preview after a short pause, unless the pointer gets onto it (or back onto a mark) first.
void HideDesktopPreviewSoon() {
    if (g_deskPreview.wnd && g_deskPreview.desktop >= 0 && IsWindowVisible(g_deskPreview.wnd))
        SetTimer(g_deskPreview.wnd, kTimerDesktopPreviewHide, kDesktopPreviewHideDelay, nullptr);
    else
        HideDesktopPreview();
}

LRESULT CALLBACK DesktopPreviewProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_MOUSEMOVE:
            // On it: stays open, and asks for the leave.
            if (!g_deskPreview.tracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                g_deskPreview.tracking = true;
            }
            KillTimer(wnd, kTimerDesktopPreviewHide);
            return 0;
        case WM_MOUSELEAVE:
            g_deskPreview.tracking = false;
            HideDesktopPreview();
            return 0;
        case WM_LBUTTONUP: {
            int desktop = g_deskPreview.desktop;
            PagerGoTo(desktop);
            return 0;
        }
        case WM_TIMER:
            if (wParam == kTimerDesktopPreviewHide) HideDesktopPreview();
            return 0;
        case WM_NCHITTEST:
            return HTCLIENT;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            BarScope scope(&g_mainBar);
            auto const& p = g_settings.palette;
            RECT rc;
            GetClientRect(wnd, &rc);
            Fill(dc, rc, p.ground);
            RECT const& area = g_deskPreview.area;
            if (g_deskPreview.wallpaper) {
                HDC mem = CreateCompatibleDC(dc);
                HGDIOBJ old = SelectObject(mem, g_deskPreview.wallpaper);
                BitBlt(dc, area.left, area.top, area.right - area.left, area.bottom - area.top, mem, 0, 0, SRCCOPY);
                SelectObject(mem, old);
                DeleteDC(mem);
            } else {
                SetDCBrushColor(dc, GetSysColor(COLOR_BACKGROUND));
                FillRect(dc, &area, (HBRUSH)GetStockObject(DC_BRUSH));
            }
            RECT label{area.left, 0, area.right, area.top};
            HGDIOBJ oldFont = SelectObject(dc, g_tipFont ? g_tipFont : g_font);
            SetBkMode(dc, TRANSPARENT);
            bool current = g_deskPreview.desktop == g_pagerDesktops.current;
            SetTextColor(dc, PaletteRef(current ? p.accent : p.text));
            std::wstring text = PagerLabel(g_deskPreview.desktop) + (current ? L"  (showing)" : L"");
            DrawTextW(dc, text.c_str(), -1, &label, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            SelectObject(dc, oldFont);
            EndPaint(wnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// The windows on a desktop, frontmost first: the ones on it, and the ones shown on every desktop.
std::vector<HWND> DesktopWindows(int desktop) {
    struct Walk {
        std::vector<HWND> windows;
        IVirtualDesktopManager* vdm;
        GUID target;
        bool known;  // the desktops have ids to tell them apart
    } walk{{}, CssVdm(), GUID{}, desktop >= 0 && desktop < (int)g_pagerDesktops.ids.size()};
    if (walk.known) walk.target = g_pagerDesktops.ids[desktop];
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            auto* w = (Walk*)param;
            if (!IsTaskWindow(h, true) || IsIconic(h)) return TRUE;
            GUID id{};
            bool onOne = w->vdm && SUCCEEDED(w->vdm->GetWindowDesktopId(h, &id)) && id != GUID{};
            if (w->known && onOne && id != w->target) return TRUE;
            // Hidden and on no desktop: never shown (Windows' touch keyboard host, a full-screen window whose teal
            // surface covered the wallpaper in every preview).
            DWORD cloaked = 0;
            if (!onOne && SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return TRUE;
            w->windows.push_back(h);
            return TRUE;
        },
        (LPARAM)&walk);
    return walk.windows;
}

void ShowDesktopPreview(int desktop, RECT anchor, bool fromBar) {
    if (desktop < 0 || desktop >= g_pagerDesktops.count) return;
    BarScope scope(&g_mainBar);
    auto const& l = g_settings.pagerLook;
    HidePreview();  // (a button's)
    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
    ClearDesktopThumbnails();
    g_deskPreview.desktop = desktop;

    // The screen the mark is on, drawn at its own shape.
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &mi);
    RECT screen = mi.rcMonitor;
    int sw = screen.right - screen.left, sh = screen.bottom - screen.top;
    int w = Px(l.previewWidth), h = sw ? MulDiv(w, sh, sw) : w;
    int pad = Px(8), label = Px(22);
    g_deskPreview.area = {pad, pad + label, pad + w, pad + label + h};
    int width = w + 2 * pad, height = label + h + 2 * pad;

    // Beside the bar for a docked pager, as a button's preview is; beside the floating one, where there's room.
    int x =(anchor.left + anchor.right - width) / 2, y = (anchor.top + anchor.bottom - height) / 2;
    if (fromBar) {
        RECT bar;
        GetWindowRect(g_wnd.load(), &bar);
        switch (g_settings.edge) {
            case Edge::Top: y = bar.bottom + Px(4); break;
            case Edge::Bottom: y = bar.top - height - Px(4); break;
            case Edge::Left: x = bar.right + Px(4); break;
            case Edge::Right: x = bar.left - width - Px(4); break;
        }
    } else if (g_pagerWnd) {
        RECT pager;
        GetWindowRect(g_pagerWnd, &pager);
        if (l.vertical) x = pager.right + Px(6) + width <= screen.right ? pager.right + Px(6) : pager.left - Px(6) - width;
        else y = pager.bottom + Px(6) + height <= screen.bottom ? pager.bottom + Px(6) : pager.top - Px(6) - height;
    }
    x = std::clamp(x, (int)screen.left + Px(4), std::max((int)screen.left + Px(4), (int)screen.right - width - Px(4)));
    y = std::clamp(y, (int)screen.top + Px(4), std::max((int)screen.top + Px(4), (int)screen.bottom - height - Px(4)));

    if (!g_deskPreview.wnd) {
        g_deskPreview.wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
                                            kDesktopPreviewClass, L"", WS_POPUP, x, y, width, height, nullptr, nullptr,
                                            g_pagerInstance, nullptr);
        if (!g_deskPreview.wnd) return;
        KeepOutOfPeek(g_deskPreview.wnd);
        DWORD corners = 3;  // DWMWCP_ROUNDSMALL
        DwmSetWindowAttribute(g_deskPreview.wnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
        BOOL noFade = TRUE;
        DwmSetWindowAttribute(g_deskPreview.wnd, 3 /* DWMWA_TRANSITIONS_FORCEDISABLED */, &noFade, sizeof(noFade));
    }
    COLORREF border = MenuRgb(g_settings.palette.edge);
    DwmSetWindowAttribute(g_deskPreview.wnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));

    // The wallpaper, read again only for another picture or size.
    std::wstring path = DesktopWallpaper(desktop);
    if (path != g_deskPreview.wallpaperPath || g_deskPreview.wallpaperSize.cx != w || g_deskPreview.wallpaperSize.cy != h) {
        if (g_deskPreview.wallpaper) DeleteObject(g_deskPreview.wallpaper);
        g_deskPreview.wallpaper = LoadCoverPicture(path, w, h);
        Wh_Log(L"pager: desktop %d's wallpaper \"%s\" at %dx%d: %s", desktop + 1, path.c_str(), w, h,
               g_deskPreview.wallpaper ? L"read" : L"none");
        g_deskPreview.wallpaperPath = path;
        g_deskPreview.wallpaperSize = {w, h};
    }
    SetWindowPos(g_deskPreview.wnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    // Its windows, back to front, each where it is on the screen (only its part on this screen).
    double scale = sw ? (double)w / sw : 1;
    std::vector<HWND> windows = DesktopWindows(desktop);
    for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
        HWND h = *it;
        RECT frame{}, shown, whole;
        if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(frame)))) GetWindowRect(h, &frame);
        if (!IntersectRect(&shown, &frame, &screen) || !GetWindowRect(h, &whole)) continue;
        HTHUMBNAIL thumbnail = nullptr;
        if (FAILED(DwmRegisterThumbnail(g_deskPreview.wnd, h, &thumbnail))) continue;
        RECT const& area = g_deskPreview.area;
        DWM_THUMBNAIL_PROPERTIES props{};
        props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE | DWM_TNP_VISIBLE | DWM_TNP_OPACITY |
                        DWM_TNP_SOURCECLIENTAREAONLY;
        props.rcSource = {shown.left - whole.left, shown.top - whole.top, shown.right - whole.left, shown.bottom - whole.top};
        props.rcDestination = {area.left + (int)std::lround((shown.left - screen.left) * scale),
                               area.top + (int)std::lround((shown.top - screen.top) * scale),
                               area.left + (int)std::lround((shown.right - screen.left) * scale),
                               area.top + (int)std::lround((shown.bottom - screen.top) * scale)};
        props.fVisible = TRUE;
        props.opacity = 255;
        props.fSourceClientAreaOnly = FALSE;
        DwmUpdateThumbnailProperties(thumbnail, &props);
        g_deskPreview.thumbnails.push_back(thumbnail);
    }
    InvalidateRect(g_deskPreview.wnd, nullptr, FALSE);
}

// The pointer on a mark (or off them, -1): one shows after a pause, or at once when one is already up.
void PagerPreviewHover(HWND from, int desktop, RECT anchor, bool fromBar) {
    // Off the marks while one shows: the pointer may be on its way to the preview, so it closes after a pause.
    if (desktop < 0 && g_deskPreview.desktop >= 0 && g_settings.pagerLook.previews && !g_pagerCarry) {
        HideDesktopPreviewSoon();
        return;
    }
    if (!g_settings.pagerLook.previews || g_pagerCarry || desktop < 0) {
        HideDesktopPreview();
        return;
    }
    if (g_deskPreview.wnd) KillTimer(g_deskPreview.wnd, kTimerDesktopPreviewHide);
    if (g_deskPreview.desktop >= 0) {
        if (g_deskPreview.desktop != desktop) ShowDesktopPreview(desktop, anchor, fromBar);
        return;
    }
    if (g_deskPreview.timerWnd && g_deskPreview.timerWnd != from) KillTimer(g_deskPreview.timerWnd, kTimerDesktopPreview);
    g_deskPreview.pending = desktop;
    g_deskPreview.pendingAnchor = anchor;
    g_deskPreview.pendingFromBar = fromBar;
    g_deskPreview.timerWnd = from;
    SetTimer(from, kTimerDesktopPreview, 400, nullptr);
}

// The pause is over (the bar's or the floating pager's timer).
void PagerPreviewTimer(HWND from) {
    KillTimer(from, kTimerDesktopPreview);
    int desktop = g_deskPreview.pending;
    g_deskPreview.pending = -1;
    g_deskPreview.timerWnd = nullptr;
    if (desktop >= 0) ShowDesktopPreview(desktop, g_deskPreview.pendingAnchor, g_deskPreview.pendingFromBar);
}

// A docked mark pointed at, on the bar (from its mouse handling).
void PagerHoverOnBar(HWND bar, int hit) {
    int desktop = IsRectEmpty(&g_pagerRect) ? -1 : PagerIndexOf(hit);
    RECT anchor{};
    if (desktop >= 0 && desktop < (int)g_pagerItems.size()) {
        anchor = g_pagerItems[desktop];
        MapWindowPoints(bar, nullptr, (POINT*)&anchor, 2);
    } else {
        desktop = -1;
    }
    PagerPreviewHover(bar, desktop, anchor, true);
}

////////////////////////////////////////////////////////////////////////////////
// What it does

// The lit desktop's move (PagerMove, drawn in DrawPagerItems): set off, a frame, and still again.
void PagerMoveStop() {
    if (g_pagerMove.timer) KillTimer(nullptr, g_pagerMove.timer);
    g_pagerMove = {};
}

// The pager drawn again where it shows: docked on the main bar, or floating.
void PagerMoveRedraw() {
    if (HWND bar = g_wnd) {
        BarScope scope(&g_mainBar);
        if (!IsRectEmpty(&g_pagerRect)) Redraw(bar);
    }
    if (g_pagerWnd && IsWindowVisible(g_pagerWnd)) PaintFloatingPager();
}

void CALLBACK PagerMoveTick(HWND, UINT, UINT_PTR, DWORD) {
    auto& m = g_pagerMove;
    auto const& in = g_settings.motionDesksIn;
    auto const& out = g_settings.motionDesksOut.set ? g_settings.motionDesksOut : in;
    double now = NowMs(), msIn = std::min(motion::Ms(in), 2000.0), msOut = std::min(motion::Ms(out), 2000.0);
    double tIn = msIn > 0 ? (now - m.start) / msIn : 1, tOut = msOut > 0 ? (now - m.start) / msOut : 1;
    if (!in.set || !in.on || (tIn >= 1 && (!m.fade || tOut >= 1))) {
        PagerMoveStop();  // there: lit where it is, as ever
    } else if (m.fade) {
        m.in = tIn >= 1 ? 1 : motion::Ease(in, tIn);
        m.out = tOut >= 1 ? 0 : 1 - motion::Ease(out, tOut);
    } else {
        m.at = m.from + (g_pagerDesktops.current - m.from) * motion::Ease(in, tIn);
    }
    PagerMoveRedraw();
}

// The lit desktop changed (from the one given to the current one): it moves there, when a motion line says so.
void PagerMoveStart(int from) {
    auto const& in = g_settings.motionDesksIn;
    int to = g_pagerDesktops.current;
    if (!in.set || !in.on || motion::Ms(in) <= 0 || from == to || from < 0 || from >= g_pagerDesktops.count) {
        PagerMoveStop();
        return;
    }
    auto& m = g_pagerMove;
    bool fade = motion::StyleWord(in) == L"fade";
    // A move cut short sets off again from where it had got to (kept on the run, past any overshoot).
    double at = m.timer && !m.fade && !fade ? std::clamp(m.at, 0.0, g_pagerDesktops.count - 1.0) : from;
    m.fade = fade;
    m.from = m.at = at;
    m.in = 0;
    m.out = 1;
    m.start = NowMs();
    if (!m.timer) m.timer = SetTimer(nullptr, 0, USER_TIMER_MINIMUM, PagerMoveTick);
}

// Straight there through Windows' desktop manager where it answers, else Ctrl+Win+Left or Right once per desktop
// between, as Win+Tab's own switching (css-data.cpp). For clicks, the wheel and the desktop keys alike.
void PagerShowCurrent(int target);

void PagerGoTo(int target) {
    HideDesktopPreview();
    if (target < 0 || target >= g_pagerDesktops.count || target == g_pagerDesktops.current) return;
    CssGoToDesktop(target);
    PagerShowCurrent(target);
}

// Shown at once; the registry says so a moment later.
void PagerShowCurrent(int target) {
    int was = g_pagerDesktops.current;
    g_pagerDesktops.current = target;
    PagerMoveStart(was);
    if (g_settings.pagerLook.shape == kMarkPill) {
        ApplyPager();  // the wide mark moved: the run's lengths changed
        return;
    }
    if (HWND bar = g_wnd) {
        BarScope scope(&g_mainBar);
        Redraw(bar);
    }
    if (g_pagerWnd && IsWindowVisible(g_pagerWnd)) PaintFloatingPager();
}

void PagerStep(int wheel) {
    PagerGoTo(std::clamp(g_pagerDesktops.current + (wheel < 0 ? 1 : -1), 0, g_pagerDesktops.count - 1));
}

// @inline virtual-desktops.cpp

// The window in front sent to a desktop (the "Desktop keys"). Not the desktop itself or the taskbar.
void PagerSendWindow(int target) {
    if (target < 0 || target >= (int)g_pagerDesktops.ids.size() || target == g_pagerDesktops.current) return;
    HWND fg = GetForegroundWindow();
    HWND h = fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
    if (!h || h == GetShellWindow()) return;
    WCHAR cls[64] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    for (PCWSTR shell : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW"})
        if (!wcscmp(cls, shell)) return;
    bool follow = g_settings.followMoved;
    if (!VdmSendWindow(h, g_pagerDesktops.ids[target], follow)) {
        MessageBeep(MB_ICONWARNING);
        return;
    }
    HideDesktopPreview();
    CssDesktopsChanged();
    if (follow) PagerShowCurrent(target);
}

// The taskbar's "Desktop keys", while the pager is on (it knows which desktop is showing). A key another
// program already holds just isn't taken.
void UnregisterDesktopKeys(HWND wnd) {
    UnregisterHotKey(wnd, kHotkeyNextDesktop);
    UnregisterHotKey(wnd, kHotkeyPrevDesktop);
    UnregisterHotKey(wnd, kHotkeyMoveNext);
    UnregisterHotKey(wnd, kHotkeyMovePrev);
    for (int i = 0; i < 9; i++) {
        UnregisterHotKey(wnd, kHotkeyDesktop1 + i);
        UnregisterHotKey(wnd, kHotkeyMove1 + i);
    }
}

void RegisterDesktopKeys(HWND wnd) {
    UnregisterDesktopKeys(wnd);
    auto const& s = g_settings;
    if (!s.desktops) return;
    if (s.nextDesktopVk) RegisterHotKey(wnd, kHotkeyNextDesktop, s.nextDesktopMods, s.nextDesktopVk);
    if (s.prevDesktopVk) RegisterHotKey(wnd, kHotkeyPrevDesktop, s.prevDesktopMods, s.prevDesktopVk);
    if (s.goToDesktopMods)
        for (int i = 0; i < 9; i++) RegisterHotKey(wnd, kHotkeyDesktop1 + i, s.goToDesktopMods | MOD_NOREPEAT, '1' + i);
    if (s.moveNextVk) RegisterHotKey(wnd, kHotkeyMoveNext, s.moveNextMods | MOD_NOREPEAT, s.moveNextVk);
    if (s.movePrevVk) RegisterHotKey(wnd, kHotkeyMovePrev, s.movePrevMods | MOD_NOREPEAT, s.movePrevVk);
    if (s.moveToDesktopMods)
        for (int i = 0; i < 9; i++) RegisterHotKey(wnd, kHotkeyMove1 + i, s.moveToDesktopMods | MOD_NOREPEAT, '1' + i);
}

////////////////////////////////////////////////////////////////////////////////
// Renaming: a middle click on a mark (or "Rename" in its menu) opens a box over it with the name;
// Enter keeps it, Esc or clicking away leaves it as it was.

constexpr WCHAR kRenameClass[] = L"TourneDesktopRename_" WH_MOD_ID;
constexpr UINT kMsgRenameClose = WM_APP + 1;  // to the box: clicked or tabbed away from

struct RenameBox {
    HWND wnd = nullptr, edit = nullptr;
    GUID id{};
    HBRUSH ground = nullptr;
};
RenameBox g_rename;

void CloseRenameBox(bool keep) {
    HWND wnd = g_rename.wnd;
    if (!wnd) return;
    g_rename.wnd = nullptr;  // (first: destroying it takes the focus away, which would close it again)
    if (keep) {
        WCHAR text[256] = L"";
        GetWindowTextW(g_rename.edit, text, ARRAYSIZE(text));
        if (!VdmRename(g_rename.id, text)) MessageBeep(MB_ICONWARNING);
    }
    DestroyWindow(wnd);
    g_rename.edit = nullptr;
    if (g_rename.ground) DeleteObject(g_rename.ground);
    g_rename.ground = nullptr;
}

LRESULT CALLBACK RenameEditProc(HWND edit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN || wParam == VK_ESCAPE) {
                CloseRenameBox(wParam == VK_RETURN);
                return 0;
            }
            break;
        case WM_CHAR:
            if (wParam == VK_RETURN || wParam == VK_ESCAPE) return 0;  // (no beep)
            break;
        case WM_KILLFOCUS:
            PostMessageW(GetParent(edit), kMsgRenameClose, 0, 0);  // (not while the focus is moving)
            break;
        case WM_NCDESTROY:
            RemoveWindowSubclass(edit, RenameEditProc, 0);
            break;
    }
    return DefSubclassProc(edit, msg, wParam, lParam);
}

LRESULT CALLBACK RenameBoxProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CTLCOLOREDIT && g_rename.ground) {
        auto const& p = g_settings.palette;
        SetTextColor((HDC)wParam, PaletteRef(p.text));
        SetBkColor((HDC)wParam, PaletteRef(p.raised));
        return (LRESULT)g_rename.ground;
    }
    if (msg == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE) {
        PostMessageW(wnd, kMsgRenameClose, 0, 0);
        return 0;
    }
    if (msg == kMsgRenameClose) {
        if (wnd == g_rename.wnd) CloseRenameBox(false);
        return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// Over the mark (anchor, on the screen): beside the bar for a docked pager, below or above a floating one.
void PagerRename(int desktop, RECT anchor, bool fromBar) {
    CloseRenameBox(false);
    HideDesktopPreview();
    if (desktop < 0 || desktop >= (int)g_pagerDesktops.ids.size()) {
        MessageBeep(MB_ICONWARNING);  // (Windows gives desktops ids once a second one has been made)
        return;
    }
    BarScope scope(&g_mainBar);
    auto const& p = g_settings.palette;
    int width = std::max<int>(anchor.right - anchor.left, Px(180)), height = Px(26);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &mi);
    RECT screen = mi.rcMonitor;
    int x = (anchor.left + anchor.right - width) / 2;
    int below = anchor.bottom + Px(4), above = anchor.top - Px(4) - height;
    int y = below + height <= screen.bottom ? below : above;
    if (fromBar) {
        RECT bar;
        GetWindowRect(g_wnd.load(), &bar);
        if (g_settings.edge == Edge::Bottom) y = bar.top - height - Px(4);
        else if (g_settings.edge == Edge::Top) y = bar.bottom + Px(4);
        else if (g_settings.edge == Edge::Left) x = bar.right + Px(4), y = anchor.top;
        else x = bar.left - width - Px(4), y = anchor.top;
    }
    x = std::clamp(x, (int)screen.left + Px(4), std::max((int)screen.left + Px(4), (int)screen.right - width - Px(4)));
    y = std::clamp(y, (int)screen.top + Px(4), std::max((int)screen.top + Px(4), (int)screen.bottom - height - Px(4)));
    g_rename.id = g_pagerDesktops.ids[desktop];
    g_rename.ground = CreateSolidBrush(PaletteRef(p.raised));
    g_rename.wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kRenameClass, L"", WS_POPUP | WS_BORDER, x, y, width,
                                   height, nullptr, nullptr, g_pagerInstance, nullptr);
    if (!g_rename.wnd) {
        DeleteObject(g_rename.ground);
        g_rename.ground = nullptr;
        return;
    }
    KeepOutOfPeek(g_rename.wnd);
    RECT client;
    GetClientRect(g_rename.wnd, &client);
    g_rename.edit = CreateWindowExW(0, L"EDIT", g_pagerDesktops.names[desktop].c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                    Px(4), (client.bottom - Px(18)) / 2, client.right - Px(8), Px(18), g_rename.wnd,
                                    nullptr, g_pagerInstance, nullptr);
    SendMessageW(g_rename.edit, WM_SETFONT, (WPARAM)g_font, FALSE);
    SendMessageW(g_rename.edit, EM_LIMITTEXT, 255, 0);
    SendMessageW(g_rename.edit, EM_SETSEL, 0, -1);
    SetWindowSubclass(g_rename.edit, RenameEditProc, 0, 0);
    ShowWindow(g_rename.wnd, SW_SHOW);
    SetForegroundWindow(g_rename.wnd);  // (allowed: the click that asked for it was on this process's window)
    if (GetForegroundWindow() != g_rename.wnd) {  // (Windows said no: a box that can't be typed in isn't left up)
        CloseRenameBox(false);
        MessageBeep(MB_ICONWARNING);
        return;
    }
    SetFocus(g_rename.edit);
}

// A docked mark's place on the screen (for the box, and the menu's "Rename").
RECT PagerMarkOnScreen(HWND bar, int desktop) {
    RECT anchor{};
    if (desktop < 0 || desktop >= (int)g_pagerItems.size()) return anchor;
    anchor = g_pagerItems[desktop];
    MapWindowPoints(bar, nullptr, (POINT*)&anchor, 2);
    return anchor;
}

// The registry's desktops changed (one added, closed, renamed or switched to).
void PagerDesktopsChanged() {
    if (!g_settings.desktops) return;
    PagerDesktops now = ReadPagerDesktops();
    if (now == g_pagerDesktops) return;
    bool resized = now.count != g_pagerDesktops.count || now.names != g_pagerDesktops.names ||
                   (now.current != g_pagerDesktops.current && g_settings.pagerLook.shape == kMarkPill);
    bool recounted = now.count != g_pagerDesktops.count;
    int was = g_pagerDesktops.current;
    g_pagerDesktops = std::move(now);
    if (recounted) PagerMoveStop();  // (desktops came or went: lit where it is now)
    else if (was != g_pagerDesktops.current) PagerMoveStart(was);
    if (g_deskPreview.desktop >= g_pagerDesktops.count) HideDesktopPreview();
    else if (g_deskPreview.desktop >= 0) InvalidateRect(g_deskPreview.wnd, nullptr, FALSE);  // its label
    if (resized) {
        ApplyPager();
        return;
    }
    if (HWND bar = g_wnd) {
        BarScope scope(&g_mainBar);
        Redraw(bar);
    }
    if (g_pagerWnd && IsWindowVisible(g_pagerWnd)) PaintFloatingPager();
}

enum : int { kCmdPagerTaskView = 9100, kCmdPagerNew, kCmdPagerClose, kCmdPagerLock, kCmdPagerTop, kCmdPagerSingle,
             kCmdPagerLookSettings, kCmdPagerRename, kCmdPagerStyle = 9120, kCmdPagerPlace = 9130 };

void PagerMenu(int index, RECT anchor) {
    HWND bar = g_wnd;
    if (!bar) return;
    HideDesktopPreview();
    BarScope scope(&g_mainBar);
    PagerChoices& c = g_pagerChoices;
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, (index >= 0 ? PagerLabel(index) : std::wstring(L"Desktops")).c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    if (index >= 0) AppendMenuW(menu, MF_STRING, kCmdPagerRename, L"Rename (middle click)");
    AppendMenuW(menu, MF_STRING, kCmdPagerTaskView, L"Task view");
    AppendMenuW(menu, MF_STRING, kCmdPagerNew, L"New desktop");
    AppendMenuW(menu, MF_STRING | (g_pagerDesktops.count > 1 ? 0 : MF_GRAYED), kCmdPagerClose, L"Close the desktop showing");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    PCWSTR styles[] = {L"Dots", L"Numbers", L"Names"};
    for (int i = 0; i < 3; i++)
        AppendMenuW(menu, MF_STRING | (c.style == i ? MF_CHECKED : 0), kCmdPagerStyle + i, styles[i]);
    AppendMenuW(menu, MF_STRING, kCmdPagerLookSettings, L"Size, shape and colours...");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    PCWSTR places[] = {L"On the bar, before the tray", L"On the bar, after Start", L"On the bar, after the clock", L"Floating"};
    for (int i = 0; i < 4; i++)
        AppendMenuW(menu, MF_STRING | (c.place == i ? MF_CHECKED : 0) | (i != kPagerFloating && CssActive() ? MF_GRAYED : 0),
                    kCmdPagerPlace + i, places[i]);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING | (c.locked ? MF_CHECKED : 0), kCmdPagerLock, L"Lock in place");
    AppendMenuW(menu, MF_STRING | (c.onTop ? MF_CHECKED : 0) | (c.place == kPagerFloating ? 0 : MF_GRAYED), kCmdPagerTop,
                L"Keep on top");
    AppendMenuW(menu, MF_STRING | (c.hideSingle ? MF_CHECKED : 0), kCmdPagerSingle, L"Hide with only one desktop");
    int cmd = RunMenu(bar, menu, TPM_RIGHTBUTTON | TPM_NONOTIFY);
    if (cmd == kCmdPagerRename) {
        PagerRename(index, anchor, c.place != kPagerFloating);
        return;
    }
    if (cmd == kCmdPagerTaskView) {
        SendKeys({VK_LWIN, VK_TAB});
        return;
    }
    if (cmd == kCmdPagerNew) {
        SendKeys({VK_LCONTROL, VK_LWIN, 'D'});
        return;
    }
    if (cmd == kCmdPagerClose) {
        SendKeys({VK_LCONTROL, VK_LWIN, VK_F4});
        return;
    }
    if (cmd == kCmdPagerLookSettings) {
        OpenOwnSettings(L"desktopslook.");  // (the look lives in the taskbar's settings)
        return;
    }
    if (cmd >= kCmdPagerStyle && cmd < kCmdPagerStyle + 3) {
        c.style = cmd - kCmdPagerStyle;
    } else if (cmd >= kCmdPagerPlace && cmd < kCmdPagerPlace + 4) {
        int place = cmd - kCmdPagerPlace;
        if (place == kPagerFloating && c.place != kPagerFloating && !IsRectEmpty(&g_pagerRect)) {
            // Off the bar: just beside where it was.
            RECT r = g_pagerRect;
            MapWindowPoints(bar, nullptr, (POINT*)&r, 2);
            RECT br;
            GetWindowRect(bar, &br);
            bool vertical = Vertical();
            MeasureFloatingPager();
            c.at = vertical ? POINT{g_settings.edge == Edge::Left ? br.right + Px(8) : br.left - Px(8) - g_pagerFloatSize.cx, r.top}
                            : POINT{r.left, g_settings.edge == Edge::Top ? br.bottom + Px(8) : br.top - Px(8) - g_pagerFloatSize.cy};
        }
        c.place = place;
    } else if (cmd == kCmdPagerLock) {
        c.locked = !c.locked;
    } else if (cmd == kCmdPagerTop) {
        c.onTop = !c.onTop;
    } else if (cmd == kCmdPagerSingle) {
        c.hideSingle = !c.hideSingle;
    } else {
        return;
    }
    SavePagerChoices();
    ApplyPager();
}

LRESULT CALLBACK PagerProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto hit = [&](POINT pt) {
        for (int i = 0; i < (int)g_pagerFloatItems.size(); i++)
            if (PtInRect(&g_pagerFloatItems[i], pt)) return i;
        return -1;
    };
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_MOUSEMOVE: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_pagerDrag && (wParam & MK_LBUTTON)) {
                if (!g_pagerCarry && (abs(pt.x - g_pagerDragStart.x) > GetSystemMetrics(SM_CXDRAG) ||
                                      abs(pt.y - g_pagerDragStart.y) > GetSystemMetrics(SM_CYDRAG))) {
                    g_pagerFloatPressed = -1;
                    StartPagerCarry(wnd, g_pagerDragStart);
                    PaintFloatingPager();
                }
                if (g_pagerCarry) {
                    POINT screen = pt;
                    ClientToScreen(wnd, &screen);
                    CarryPager(screen);
                    return 0;
                }
            }
            if (!g_pagerFloatTracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                g_pagerFloatTracking = true;
                // Pointed at: back to full at once.
                KillTimer(wnd, kPagerTimerIdle);
                StartPagerFade(1);
            }
            int at = hit(pt);
            if (at != g_pagerFloatHot) {
                g_pagerFloatHot = at;
                PaintFloatingPager();
                RECT anchor = at >= 0 ? g_pagerFloatItems[at] : RECT{};
                MapWindowPoints(wnd, nullptr, (POINT*)&anchor, 2);
                if (!g_pagerDrag) PagerPreviewHover(wnd, at, anchor, false);
            }
            return 0;
        }

        case WM_MOUSELEAVE:
            HideDesktopPreviewSoon();
            g_pagerFloatTracking = false;
            if (g_pagerFloatHot >= 0) {
                g_pagerFloatHot = -1;
                PaintFloatingPager();
            }
            if (!g_pagerCarry) SettlePagerFade();
            return 0;

        case WM_TIMER:
            if (wParam == kPagerTimerFade) {
                PagerFadeStep();
            } else if (wParam == kPagerTimerIdle) {
                KillTimer(wnd, kPagerTimerIdle);
                if (!g_pagerFloatTracking && !g_pagerCarry) StartPagerFade(g_settings.pagerLook.fadeTo / 100.0);
            } else if (wParam == kTimerDesktopPreview) {
                PagerPreviewTimer(wnd);
            }
            return 0;

        case WM_LBUTTONDOWN: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            HideDesktopPreview();
            g_pagerFloatPressed = hit(pt);
            g_pagerDrag = !g_pagerChoices.locked;
            g_pagerDragStart = pt;
            SetCapture(wnd);
            PaintFloatingPager();
            return 0;
        }

        case WM_LBUTTONUP: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            bool carried = g_pagerCarry;
            g_pagerDrag = false;  // before ReleaseCapture, so it isn't taken for a cancel
            int pressed = g_pagerFloatPressed;
            g_pagerFloatPressed = -1;
            ReleaseCapture();
            if (carried) {
                POINT screen = pt;
                ClientToScreen(wnd, &screen);
                DropPager(screen);
                return 0;
            }
            PaintFloatingPager();
            if (pressed >= 0 && hit(pt) == pressed) PagerGoTo(pressed);
            return 0;
        }

        case WM_CAPTURECHANGED:
            if (g_pagerDrag && (HWND)lParam != wnd) {
                g_pagerDrag = false;
                g_pagerFloatPressed = -1;
                if (g_pagerCarry) CancelPagerCarry();
                else PaintFloatingPager();
            }
            return 0;

        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            int at = hit({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            RECT anchor = at >= 0 ? g_pagerFloatItems[at] : RECT{};
            MapWindowPoints(wnd, nullptr, (POINT*)&anchor, 2);
            if (msg == WM_RBUTTONUP) PagerMenu(at, anchor);
            else if (at >= 0) PagerRename(at, anchor, false);
            return 0;
        }

        case WM_MOUSEWHEEL:
            PagerStep(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;

        case WM_DISPLAYCHANGE:
            // A screen went: back onto one that's there.
            if (IsWindowVisible(wnd) && !g_pagerCarry) ApplyPager();
            return 0;

        case WM_NCDESTROY:
            // (Owned by the bar while on top, so it may go with the bar.)
            if (g_pagerWnd == wnd) g_pagerWnd = nullptr;
            break;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

void RegisterPagerClass(HINSTANCE instance) {
    g_pagerInstance = instance;
    WNDCLASSW wc{};
    wc.lpfnWndProc = PagerProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kPagerClass;
    RegisterClassW(&wc);
    WNDCLASSW preview{};
    preview.lpfnWndProc = DesktopPreviewProc;
    preview.hInstance = instance;
    preview.lpszClassName = kDesktopPreviewClass;
    RegisterClassW(&preview);
    WNDCLASSW rename{};
    rename.lpfnWndProc = RenameBoxProc;
    rename.hInstance = instance;
    rename.hCursor = LoadCursorW(nullptr, IDC_IBEAM);
    rename.lpszClassName = kRenameClass;
    RegisterClassW(&rename);
}

void ClosePager() {
    PagerMoveStop();
    CloseRenameBox(false);
    if (g_pagerInstance) UnregisterClassW(kRenameClass, g_pagerInstance);
    HideDesktopPreview();
    if (g_deskPreview.wnd) DestroyWindow(g_deskPreview.wnd);
    g_deskPreview.wnd = nullptr;
    if (g_deskPreview.wallpaper) DeleteObject(g_deskPreview.wallpaper);
    g_deskPreview.wallpaper = nullptr;
    g_deskPreview.wallpaperPath.clear();
    if (g_pagerInstance) UnregisterClassW(kDesktopPreviewClass, g_pagerInstance);
    if (g_pagerWnd) DestroyWindow(g_pagerWnd);
    g_pagerWnd = nullptr;
    delete g_pagerSurface;
    g_pagerSurface = nullptr;
    if (g_pagerTextFont) DeleteObject(g_pagerTextFont);
    g_pagerTextFont = g_pagerTextFontOf = nullptr;
    g_pagerCarry = g_pagerDrag = g_pagerOverBar = g_pagerCarryFromBar = false;
    if (g_pagerInstance) UnregisterClassW(kPagerClass, g_pagerInstance);
}
