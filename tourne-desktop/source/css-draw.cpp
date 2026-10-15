// Drawing for css-engine.cpp's styles: boxes with rounded corners, borders and gradients, and text with the fallback
// fonts icon glyphs need, composited into a premultiplied 32-bit surface. Software only, so it works the same on x86
// and x64 and needs nothing but GDI.
#include <windows.h>

#include <map>

namespace cssdraw {

struct Surface {
    int w = 0, h = 0;
    std::vector<uint32_t> px;  // premultiplied ARGB
    Surface(int width, int height) : w(width), h(height), px((size_t)width * height, 0) {}
};

// A straight colour with an alpha over a premultiplied pixel.
inline void Over(uint32_t& dst, int r, int g, int b, double a) {
    if (a <= 0) return;
    a = std::min(a, 1.0);
    double inv = 1 - a;
    int da = (int)(dst >> 24), dr = (int)((dst >> 16) & 255), dg = (int)((dst >> 8) & 255), db = (int)(dst & 255);
    int oa = (int)std::lround(a * 255 + da * inv), orr = (int)std::lround(r * a + dr * inv), og = (int)std::lround(g * a + dg * inv),
        ob = (int)std::lround(b * a + db * inv);
    dst = ((uint32_t)std::min(oa, 255) << 24) | ((uint32_t)std::min(orr, 255) << 16) | ((uint32_t)std::min(og, 255) << 8) | (uint32_t)std::min(ob, 255);
}

struct RectD {
    double x0, y0, x1, y1;
    bool Empty() const { return x1 <= x0 || y1 <= y0; }
};

// Distance from (px, py) to a rounded rectangle's outline: negative inside.
inline double Distance(double px, double py, RectD const& b, double const radius[4]) {
    double cx = (b.x0 + b.x1) / 2, cy = (b.y0 + b.y1) / 2, hx = (b.x1 - b.x0) / 2, hy = (b.y1 - b.y0) / 2;
    double r = px < cx ? (py < cy ? radius[0] : radius[3]) : (py < cy ? radius[1] : radius[2]);
    r = std::min({r, hx, hy});
    double qx = std::fabs(px - cx) - (hx - r), qy = std::fabs(py - cy) - (hy - r);
    return std::hypot(std::max(qx, 0.0), std::max(qy, 0.0)) + std::min(std::max(qx, qy), 0.0) - r;
}

// How much of the pixel centred on (px, py) a rounded rectangle covers.
inline double Coverage(double px, double py, RectD const& b, double const radius[4]) {
    if (b.Empty()) return 0;
    return std::clamp(0.5 - Distance(px, py, b, radius), 0.0, 1.0);
}

// The node's box-shadows, under its box (a shadow doesn't show through the box itself).
inline void PaintShadows(Surface& sf, css::Node const& n, double opacity) {
    css::Style const& st = n.style;
    for (auto it = st.shadows.rbegin(); it != st.shadows.rend(); ++it) {
        css::Shadow const& sh = *it;
        RectD box{n.x + sh.dx - sh.spread, n.y + sh.dy - sh.spread, n.x + n.w + sh.dx + sh.spread, n.y + n.h + sh.dy + sh.spread};
        if (box.Empty() || sh.color.a <= 0) continue;
        double sigma = std::max(sh.blur / 2, 0.01), reach = sh.blur * 1.2 + 1;
        double radius[4] = {st.radius[0] + sh.spread, st.radius[1] + sh.spread, st.radius[2] + sh.spread, st.radius[3] + sh.spread};
        RectD outer{n.x, n.y, n.x + n.w, n.y + n.h};
        int x0 = std::max(0, (int)std::floor(box.x0 - reach)), y0 = std::max(0, (int)std::floor(box.y0 - reach));
        int x1 = std::min(sf.w, (int)std::ceil(box.x1 + reach)), y1 = std::min(sf.h, (int)std::ceil(box.y1 + reach));
        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                double px = x + 0.5, py = y + 0.5;
                double dist = Distance(px, py, box, radius);
                double a = sh.blur > 0 ? 0.5 * std::erfc(dist / (sigma * 1.41421356)) : std::clamp(0.5 - dist, 0.0, 1.0);
                if (a <= 0.002) continue;
                a *= 1 - Coverage(px, py, outer, st.radius);
                Over(sf.px[(size_t)y * sf.w + x], sh.color.r, sh.color.g, sh.color.b, sh.color.a * a * opacity);
            }
        }
    }
}

inline css::Rgba GradientAt(css::Gradient const& g, double t) {
    t = std::clamp(t, 0.0, 1.0);
    auto const& s = g.stops;
    if (t <= s.front().at) return s.front().color;
    for (size_t i = 1; i < s.size(); i++) {
        if (t <= s[i].at) {
            double span = s[i].at - s[i - 1].at, f = span > 0 ? (t - s[i - 1].at) / span : 1;
            auto const &a = s[i - 1].color, &b = s[i].color;
            return {(int)std::lround(a.r + (b.r - a.r) * f), (int)std::lround(a.g + (b.g - a.g) * f),
                    (int)std::lround(a.b + (b.b - a.b) * f), a.a + (b.a - a.a) * f};
        }
    }
    return s.back().color;
}

// The node's box: background (or gradient), then border, both clipped to its rounded outline.
inline void PaintBox(Surface& sf, css::Node const& n, double opacity = 1) {
    css::Style const& st = n.style;
    RectD outer{n.x, n.y, n.x + n.w, n.y + n.h};
    if (outer.Empty()) return;
    if (!st.shadows.empty()) PaintShadows(sf, n, opacity);
    double bt = st.borderWidth.top, br = st.borderWidth.right, bb = st.borderWidth.bottom, bl = st.borderWidth.left;
    RectD inner{outer.x0 + bl, outer.y0 + bt, outer.x1 - br, outer.y1 - bb};
    double innerR[4] = {std::max(0.0, st.radius[0] - std::max(bl, bt)), std::max(0.0, st.radius[1] - std::max(br, bt)),
                        std::max(0.0, st.radius[2] - std::max(br, bb)), std::max(0.0, st.radius[3] - std::max(bl, bb))};
    bool hasBorder = bt > 0 || br > 0 || bb > 0 || bl > 0;
    bool hasFill = st.hasGradient || st.background.a > 0;
    if (!hasBorder && !hasFill) return;
    int x0 = std::max(0, (int)std::floor(outer.x0)), y0 = std::max(0, (int)std::floor(outer.y0));
    int x1 = std::min(sf.w, (int)std::ceil(outer.x1)), y1 = std::min(sf.h, (int)std::ceil(outer.y1));
    double angle = st.gradient.angle * 3.14159265358979 / 180, dx = std::sin(angle), dy = -std::cos(angle);
    double len = std::fabs(n.w * dx) + std::fabs(n.h * dy);
    double cx = (outer.x0 + outer.x1) / 2, cy = (outer.y0 + outer.y1) / 2;
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            double px = x + 0.5, py = y + 0.5;
            double co = Coverage(px, py, outer, st.radius);
            if (co <= 0) continue;
            uint32_t& dst = sf.px[(size_t)y * sf.w + x];
            if (hasFill) {
                css::Rgba c = st.background;
                if (st.hasGradient) c = GradientAt(st.gradient, len > 0 ? ((px - cx) * dx + (py - cy) * dy) / len + 0.5 : 0);
                Over(dst, c.r, c.g, c.b, c.a * co * opacity);
            }
            if (hasBorder) {
                double ci = Coverage(px, py, inner, innerR);
                double ring = std::max(0.0, co - ci);
                if (ring > 0) {
                    // The side this pixel belongs to: the nearest edge that has a border.
                    double d[4] = {bt > 0 ? py - outer.y0 : 1e9, br > 0 ? outer.x1 - px : 1e9, bb > 0 ? outer.y1 - py : 1e9,
                                   bl > 0 ? px - outer.x0 : 1e9};
                    int side = (int)(std::min_element(d, d + 4) - d);
                    css::Rgba c = st.borderColor[side];
                    Over(dst, c.r, c.g, c.b, c.a * ring * opacity);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------- text

struct Fonts {
    std::map<std::wstring, HFONT> cache;
    std::map<std::pair<HFONT, wchar_t>, bool> glyphs;
    HDC dc = nullptr;
    Fonts() { dc = CreateCompatibleDC(nullptr); }
    ~Fonts() {
        for (auto& [k, f] : cache) DeleteObject(f);
        DeleteDC(dc);
    }
    void Clear() {
        for (auto& [k, f] : cache) DeleteObject(f);
        cache.clear();
        glyphs.clear();
    }
    HFONT Get(std::wstring const& family, double px, int weight) {
        std::wstring key = family + L"|" + std::to_wstring((int)std::lround(px)) + L"|" + std::to_wstring(weight);
        auto it = cache.find(key);
        if (it != cache.end()) return it->second;
        LOGFONTW lf{};
        lf.lfHeight = -(int)std::lround(px);
        lf.lfWeight = weight;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfQuality = ANTIALIASED_QUALITY;
        wcsncpy_s(lf.lfFaceName, family.c_str(), _TRUNCATE);
        HFONT f = CreateFontIndirectW(&lf);
        cache[key] = f;
        return f;
    }
    bool Installed(std::wstring const& family) {
        static std::map<std::wstring, bool> known;
        auto it = known.find(family);
        if (it != known.end()) return it->second;
        HFONT f = Get(family, 12, 400);
        HGDIOBJ old = SelectObject(dc, f);
        WCHAR got[LF_FACESIZE] = L"";
        GetTextFaceW(dc, LF_FACESIZE, got);
        SelectObject(dc, old);
        bool ok = !_wcsicmp(got, family.c_str());
        known[family] = ok;
        return ok;
    }
    bool Has(HFONT f, wchar_t c) {
        auto key = std::make_pair(f, c);
        auto it = glyphs.find(key);
        if (it != glyphs.end()) return it->second;
        HGDIOBJ old = SelectObject(dc, f);
        WORD idx = 0;
        bool ok = GetGlyphIndicesW(dc, &c, 1, &idx, GGI_MARK_NONEXISTING_GLYPHS) != GDI_ERROR && idx != 0xFFFF;
        SelectObject(dc, old);
        glyphs[key] = ok;
        return ok;
    }
};

inline Fonts& FontCache() {
    static Fonts fonts;
    return fonts;
}

struct Run {
    std::wstring text;
    HFONT font;
};

// Splits text into runs, each in the first font of the style's list (then the icon fonts) that has its characters:
// what Qt does for a theme whose icon font isn't the first family.
inline std::vector<Run> Shape(std::wstring const& text, css::Style const& st) {
    Fonts& fc = FontCache();
    std::vector<std::wstring> families = st.fontFamily;
    if (families.empty()) families.push_back(L"Segoe UI");
    std::vector<std::wstring> fallbacks = {L"FiraCode Nerd Font Propo", L"Segoe Fluent Icons", L"Segoe MDL2 Assets", L"Segoe UI Symbol", L"Segoe UI"};
    std::vector<HFONT> chain;
    for (auto const& f : families)
        if (f != L"sans-serif" && f != L"monospace" && fc.Installed(f)) chain.push_back(fc.Get(f, st.fontSize, st.fontWeight));
    if (chain.empty()) chain.push_back(fc.Get(L"Segoe UI", st.fontSize, st.fontWeight));
    size_t primaryCount = chain.size();
    for (auto const& f : fallbacks)
        if (fc.Installed(f)) chain.push_back(fc.Get(f, st.fontSize, f.find(L"Icons") != std::wstring::npos || f.find(L"MDL2") != std::wstring::npos ? 400 : st.fontWeight));
    std::vector<Run> runs;
    for (size_t i = 0; i < text.size(); i++) {
        wchar_t c = text[i];
        std::wstring piece(1, c);
        bool pair = c >= 0xD800 && c <= 0xDBFF && i + 1 < text.size();
        if (pair) piece += text[++i];
        HFONT use = chain[0];
        if (pair) {
            // Beyond the BMP (Nerd Font's Material Design icons): the icon fonts, GDI can't test them.
            use = primaryCount < chain.size() ? chain[primaryCount] : chain[0];
        } else if (!fc.Has(chain[0], c) && c > 0x7F) {
            for (HFONT f : chain)
                if (fc.Has(f, c)) { use = f; break; }
        }
        if (!runs.empty() && runs.back().font == use) runs.back().text += piece;
        else runs.push_back({piece, use});
    }
    return runs;
}

struct TextSize {
    double w = 0, h = 0;
};

inline TextSize Measure(std::wstring const& text, css::Style const& st) {
    TextSize size;
    Fonts& fc = FontCache();
    for (auto const& r : Shape(text, st)) {
        HGDIOBJ old = SelectObject(fc.dc, r.font);
        SIZE e{};
        GetTextExtentPoint32W(fc.dc, r.text.c_str(), (int)r.text.size(), &e);
        TEXTMETRICW tm{};
        GetTextMetricsW(fc.dc, &tm);
        SelectObject(fc.dc, old);
        size.w += e.cx;
        size.h = std::max(size.h, (double)tm.tmHeight);
    }
    if (size.h == 0) {
        HGDIOBJ old = SelectObject(fc.dc, Shape(L"x", st)[0].font);
        TEXTMETRICW tm{};
        GetTextMetricsW(fc.dc, &tm);
        SelectObject(fc.dc, old);
        size.h = tm.tmHeight;
    }
    return size;
}

// Grayscale-antialiased text at (x, top): drawn white on black in a scratch bitmap, the result used as coverage.
inline void PaintText(Surface& sf, std::wstring const& text, css::Style const& st, double x, double top, double opacity = 1) {
    if (text.empty() || st.color.a <= 0) return;
    Fonts& fc = FontCache();
    TextSize size = Measure(text, st);
    int w = (int)std::ceil(size.w) + 8, h = (int)std::ceil(size.h) + 8;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(fc.dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ oldBmp = SelectObject(fc.dc, bmp);
    memset(bits, 0, (size_t)w * h * 4);
    SetBkMode(fc.dc, TRANSPARENT);
    SetTextColor(fc.dc, RGB(255, 255, 255));
    double cursor = 4;
    for (auto const& r : Shape(text, st)) {
        HGDIOBJ old = SelectObject(fc.dc, r.font);
        TextOutW(fc.dc, (int)std::lround(cursor), 4, r.text.c_str(), (int)r.text.size());
        SIZE e{};
        GetTextExtentPoint32W(fc.dc, r.text.c_str(), (int)r.text.size(), &e);
        cursor += e.cx;
        SelectObject(fc.dc, old);
    }
    GdiFlush();
    auto const* src = (uint32_t const*)bits;
    int ox = (int)std::lround(x) - 4, oy = (int)std::lround(top) - 4;
    for (int yy = 0; yy < h; yy++) {
        int dy = oy + yy;
        if (dy < 0 || dy >= sf.h) continue;
        for (int xx = 0; xx < w; xx++) {
            int dx = ox + xx;
            if (dx < 0 || dx >= sf.w) continue;
            uint32_t p = src[(size_t)yy * w + xx];
            double cov = (((p >> 8) & 255) / 255.0);
            if (cov > 0) Over(sf.px[(size_t)dy * sf.w + dx], st.color.r, st.color.g, st.color.b, cov * st.color.a * opacity);
        }
    }
    SelectObject(fc.dc, oldBmp);
    DeleteObject(bmp);
}

// ---------------------------------------------------------------------------------------------------- layout

// Row layout as YASB's Qt layouts do it: a node's children go left to right, each as tall as the node's content box
// (less its own margins) and as wide as what it holds, with text and icons centred in their boxes.
// Returns the node's outer width (margins included) and leaves its border-box width in n.w.
inline double LayoutMeasure(css::Node& n) {
    css::Style const& s = n.style;
    double content = 0;
    if (n.iconSize > 0) content = n.iconSize;
    else if (n.kids.empty()) content = n.text.empty() ? 0 : Measure(n.text, s).w;
    else
        for (auto& k : n.kids) content += LayoutMeasure(*k);
    if (s.width >= 0) content = s.width;
    if (s.minWidth >= 0) content = std::max(content, s.minWidth);
    if (s.maxWidth >= 0) content = std::min(content, s.maxWidth);
    n.w = content + s.padding.left + s.padding.right + s.borderWidth.left + s.borderWidth.right;
    return n.w + s.margin.left + s.margin.right;
}

inline void LayoutPlace(css::Node& n, double x, double y, double outerH) {
    css::Style const& s = n.style;
    n.x = x + s.margin.left;
    n.y = y + s.margin.top;
    n.h = outerH - s.margin.top - s.margin.bottom;
    if (s.height >= 0) {
        n.h = s.height + s.padding.top + s.padding.bottom + s.borderWidth.top + s.borderWidth.bottom;
        n.y = y + (outerH - n.h) / 2;
    }
    if (s.minHeight >= 0) n.h = std::max(n.h, s.minHeight);
    if (s.maxHeight >= 0 && n.h > s.maxHeight) {
        n.y += (n.h - s.maxHeight) / 2;
        n.h = s.maxHeight;
    }
    double cx = n.x + s.borderWidth.left + s.padding.left, cy = n.y + s.borderWidth.top + s.padding.top;
    double ch = n.h - s.borderWidth.top - s.borderWidth.bottom - s.padding.top - s.padding.bottom;
    for (auto& k : n.kids) {
        LayoutPlace(*k, cx, cy, ch);
        cx += k->w + k->style.margin.left + k->style.margin.right;
    }
}

// A popup's layout: a node's children in a column (vertical) or a row, the node as large as what it holds unless its
// style says otherwise. PopupMeasure fills n.w and n.h (the border box) and returns the outer size, margins included.
struct PopupSize {
    double w = 0, h = 0;
};

inline PopupSize PopupMeasure(css::Node& n) {
    css::Style const& s = n.style;
    double cw = 0, ch = 0;
    if (n.iconSize > 0) {
        cw = ch = n.iconSize;
    } else if (n.kids.empty()) {
        if (!n.text.empty()) {
            TextSize ts = Measure(n.text, s);
            cw = ts.w;
            ch = ts.h;
        }
    } else {
        for (auto& k : n.kids) {
            PopupSize ks = PopupMeasure(*k);
            if (n.vertical) {
                cw = std::max(cw, ks.w);
                ch += ks.h;
            } else {
                cw += ks.w;
                ch = std::max(ch, ks.h);
            }
        }
    }
    if (s.width >= 0) cw = s.width;
    if (s.minWidth >= 0) cw = std::max(cw, s.minWidth);
    if (s.maxWidth >= 0) cw = std::min(cw, s.maxWidth);
    if (s.height >= 0) ch = s.height;
    if (s.minHeight >= 0) ch = std::max(ch, s.minHeight);
    if (s.maxHeight >= 0) ch = std::min(ch, s.maxHeight);
    n.w = cw + s.padding.left + s.padding.right + s.borderWidth.left + s.borderWidth.right;
    n.h = ch + s.padding.top + s.padding.bottom + s.borderWidth.top + s.borderWidth.bottom;
    return {n.w + s.margin.left + s.margin.right, n.h + s.margin.top + s.margin.bottom};
}

// (x, y) is the corner of the node's margin box.
inline void PopupPlace(css::Node& n, double x, double y) {
    css::Style const& s = n.style;
    n.x = x + s.margin.left;
    n.y = y + s.margin.top;
    double cx = n.x + s.borderWidth.left + s.padding.left, cy = n.y + s.borderWidth.top + s.padding.top;
    double innerW = n.w - s.borderWidth.left - s.borderWidth.right - s.padding.left - s.padding.right;
    double innerH = n.h - s.borderWidth.top - s.borderWidth.bottom - s.padding.top - s.padding.bottom;
    auto outerW = [](css::Node const& k) { return k.w + k.style.margin.left + k.style.margin.right; };
    auto outerH = [](css::Node const& k) { return k.h + k.style.margin.top + k.style.margin.bottom; };
    double total = 0;
    for (auto& k : n.kids) total += n.vertical ? outerH(*k) : outerW(*k);
    if (n.centerKids) {
        if (n.vertical) cy += std::max(0.0, (innerH - total) / 2);
        else cx += std::max(0.0, (innerW - total) / 2);
    }
    for (auto& k : n.kids) {
        if (n.vertical) {
            PopupPlace(*k, cx + (n.centerKids ? std::max(0.0, (innerW - outerW(*k)) / 2) : 0), cy);
            cy += outerH(*k);
        } else {
            PopupPlace(*k, cx, cy + std::max(0.0, (innerH - outerH(*k)) / 2));
            cx += outerW(*k);
        }
    }
}

// The content box of a laid-out node.
inline RectD ContentBox(css::Node const& n) {
    css::Style const& s = n.style;
    return {n.x + s.borderWidth.left + s.padding.left, n.y + s.borderWidth.top + s.padding.top,
            n.x + n.w - s.borderWidth.right - s.padding.right, n.y + n.h - s.borderWidth.bottom - s.padding.bottom};
}

// Boxes, then what they hold: text by default; icon nodes are left to the caller's function (which draws it in the box).
template <class IconFn>
inline void PaintTree(Surface& sf, css::Node& n, IconFn&& icon);

template <class IconFn>
inline void PaintTree(Surface& sf, css::Node& n, IconFn&& icon) {
    PaintBox(sf, n, n.style.opacity);
    RectD c = ContentBox(n);
    if (n.iconSize > 0) {
        icon(n, c);
    } else if (n.kids.empty() && !n.text.empty()) {
        TextSize ts = Measure(n.text, n.style);
        PaintText(sf, n.text, n.style, c.x0 + ((c.x1 - c.x0) - ts.w) / 2, c.y0 + ((c.y1 - c.y0) - ts.h) / 2, n.style.opacity);
    }
    for (auto& k : n.kids) PaintTree(sf, *k, icon);
}

// An icon at (x, y), size px square, with its own alpha (drawn on black and on white to tell it apart).
inline void PaintIcon(Surface& sf, HICON icon, int x, int y, int size, double opacity = 1) {
    if (!icon) return;
    Fonts& fc = FontCache();
    uint32_t* layers[2];
    HBITMAP bmps[2];
    for (int k = 0; k < 2; k++) {
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
        void* bits = nullptr;
        bmps[k] = CreateDIBSection(fc.dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        layers[k] = (uint32_t*)bits;
        HGDIOBJ old = SelectObject(fc.dc, bmps[k]);
        for (int i = 0; i < size * size; i++) layers[k][i] = k ? 0x00FFFFFF : 0;
        DrawIconEx(fc.dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        GdiFlush();
        SelectObject(fc.dc, old);
    }
    for (int yy = 0; yy < size; yy++) {
        for (int xx = 0; xx < size; xx++) {
            uint32_t b = layers[0][yy * size + xx], w = layers[1][yy * size + xx];
            int alpha = 255 - (int)(((w & 255) - (b & 255) + ((w >> 8) & 255) - ((b >> 8) & 255) + ((w >> 16) & 255) - ((b >> 16) & 255)) / 3);
            alpha = std::clamp(alpha, 0, 255);
            int dx = x + xx, dy = y + yy;
            if (alpha <= 0 || dx < 0 || dy < 0 || dx >= sf.w || dy >= sf.h) continue;
            double a = alpha / 255.0;
            // b holds the colour already multiplied by alpha: take it back out.
            int r = std::min(255, (int)(((b >> 16) & 255) / a)), g = std::min(255, (int)(((b >> 8) & 255) / a)), bl = std::min(255, (int)((b & 255) / a));
            Over(sf.px[(size_t)dy * sf.w + dx], r, g, bl, a * opacity);
        }
    }
    for (int k = 0; k < 2; k++) DeleteObject(bmps[k]);
}

}  // namespace cssdraw
