////////////////////////////////////////////////////////////////////////////////
// Drawn window borders ("// @inline borders.cpp", after tiling-layouts.cpp, in
// the Switcher), in the manner of Tacky Borders: a border of any width round
// each window, its own colour or gradient for the window in front and for the
// rest, rounded as the window is, set off from its edge, with a glow and a
// shadow, a fade between the two looks and a gradient that turns.
//
// A border is four see-through strips (top, bottom, left, right) kept beside
// the window in the z-order, not one window the size of the window it frames:
// only the pixels near the edge are drawn, kept and sent to DWM. The window's
// moves are followed through Windows' events while the borders are on (as
// Tacky Borders does); a move only moves the strips, and they're drawn again
// only when the size, the look or an animation changes.

struct BorderPaint {
    uint32_t lut[256]{};  // 0xAARRGGBB along the gradient (straight alpha)
    bool gradient = false;
    float sx = 0, sy = 0, ex = 1, ey = 0;  // the gradient's start and end, as fractions of the border's box
};

struct BorderSettings {
    bool on = false;
    int width = 3, offset = -1;
    int radius = -1;  // px at 100%; -1: as the window's own corners
    bool below = false;
    bool followNative = true;
    int initDelay = 200, unminimizeDelay = 150;
    std::wstring activeText, inactiveText;
    int spinActive = 0, spinInactive = 0;  // 1 clockwise, -1 the other way, 0 none
    int spinMs = 1800;
    bool spinOnce = true;    // one turn as a window comes to the front or leaves it, then still
    bool frontMoves = true;  // window moves followed for the window in front only (and drags, and tiling)
    bool fade = false;
    int fadeMs = 200;
    int fps = 60;
    int glow = 0, shadow = 0;  // kEffectNone, kEffectActive or kEffectAll
    int glowBlur = 8, glowSpread = 0, glowOpacity = 100;
    int shadowBlur = 8, shadowOpacity = 100, shadowX = 0, shadowY = 0;
    std::wstring skipApps, skipClasses, skipTitles;
    // Motion lines (motion.cpp) for the fade between the two looks, by part
    // (kFadeBorder...): eased by the line's curve when one is set. Unset, the
    // border fades as before, and its glow and shadow with it.
    motion::Anim fadeLine[3];
    // "glowangle": the glow (drawn in the border's gradient) turning at its own
    // speed (glowSpin..., as spinActive and spinInactive); unset, with the border.
    bool glowTurns = false;
    int glowSpinActive = 0, glowSpinInactive = 0, glowSpinMs = 1800;
};

enum { kEffectNone, kEffectActive, kEffectAll };
enum { kFadeBorder, kFadeGlow, kFadeShadow };

constexpr WCHAR kBorderClass[] = L"TourneBorder_" WH_MOD_ID;
constexpr UINT_PTR kTimerBorders = 5;  // pending borders and animations
constexpr UINT_PTR kTimerBorderZ = 6;  // windows reordered: borders put back beside theirs

BorderSettings g_borderSettings;
BorderPaint g_activePaint, g_inactivePaint;
HWINEVENTHOOK g_borderHooks[6];
HWINEVENTHOOK g_borderMoveHook = nullptr;  // moves of the window in front's thread (frontMoves)
DWORD g_borderMoveThread = 0;
std::unordered_set<HWND> g_borderSkipped;  // windows left out, not asked again while they're open
HWND g_borderFront = nullptr;  // the window in front, wearing the active look
bool g_borderTimer = false;
ULONGLONG g_borderTick = 0;
bool g_borderClass = false;

struct BorderStrip {
    HWND wnd = nullptr;
    RECT rc{};  // in the border's canvas
    std::vector<uint8_t> ring, glow;  // coverage of the border itself and its glow, 0 to 255
    std::vector<float> shadow;        // the shadow's, 0 to 1 before its opacity: kept fine, so its long fade has no steps
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ old = nullptr;
    uint32_t* bits = nullptr;
    int bw = 0, bh = 0;  // the bitmap's size
};

struct Border {
    HWND target = nullptr;
    BorderStrip strips[4];
    bool shown = false;
    ULONGLONG showAt = 0;    // shown at this tick (after a delay); 0: not waiting
    RECT frame{};            // the window's frame when last placed
    RECT canvas{};           // the strips' whole area, on the screen
    RECT box{};              // the border's outer edge, in the canvas
    int dpi = 96, w = 0, pad = 0, band = 0;
    float outerR = 0, innerR = 0;
    float mix = 0, mixTarget = 0;  // 1: the active look, 0: the inactive one
    float appear = 1;              // 0 to 1 as it fades in
    float turn = 0;                // the gradient's turn, 0 to 1
    float glowTurn = 0;            // the glow's, when it turns on its own
    float spinLeft = 0, glowSpinLeft = 0;  // turns still to go, with the spin set to once
    // Each part's share of the active look, while a motion line fades it:
    // where it is, where it set off from, and how far along (1: there).
    struct Fade {
        float shown = 0, from = 0, t = 1;
    } fades[3];
};

std::unordered_map<HWND, std::unique_ptr<Border>> g_borders;

////////////////////////////////////////////////////////////////////////////////
// Colours: "accent", "#RGB", "#RGBA", "#RRGGBB" or "#RRGGBBAA"; several make
// a gradient, followed by its direction: an angle ("45deg") or a start and an
// end point ("0,1 1,0", 0,0 the top left and 1,1 the bottom right).

uint32_t BorderAccent() {
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, nullptr,
                     &abgr, &size) == ERROR_SUCCESS)
        return 0xFF000000 | ((abgr & 0xFF) << 16) | (abgr & 0xFF00) | ((abgr >> 16) & 0xFF);
    DWORD argb = 0;
    BOOL opaque = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&argb, &opaque))) return 0xFF000000 | (argb & 0xFFFFFF);
    return 0xFF0078D4;
}

// The other windows' accent colour, where the Windows 11 Accent Window Border mod reads it
// (HKCU\Software\Microsoft\Windows\DWM\AccentColorInactive, BGR); black when it isn't set, as there.
uint32_t BorderAccentInactive() {
    DWORD bgr = 0, size = sizeof(bgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColorInactive", RRF_RT_REG_DWORD,
                     nullptr, &bgr, &size) != ERROR_SUCCESS)
        return 0xFF000000;
    return 0xFF000000 | ((bgr & 0xFF) << 16) | (bgr & 0xFF00) | ((bgr >> 16) & 0xFF);
}

bool ParseBorderColor(std::wstring v, uint32_t* out) {
    CharLowerBuffW(v.data(), (DWORD)v.size());
    if (v == L"accent") {
        *out = BorderAccent();
        return true;
    }
    if (v == L"accentinactive") {
        *out = BorderAccentInactive();
        return true;
    }
    if (v.empty() || v[0] != L'#') return false;
    v.erase(0, 1);
    if (v.find_first_not_of(L"0123456789abcdef") != std::wstring::npos) return false;
    if (v.size() == 3 || v.size() == 4) {
        std::wstring longer;
        for (WCHAR c : v) longer += std::wstring(2, c);
        v = longer;
    }
    if (v.size() == 6) v += L"ff";
    if (v.size() != 8) return false;
    unsigned long rgba = wcstoul(v.c_str(), nullptr, 16);
    *out = (rgba >> 8) | ((rgba & 0xFF) << 24);
    return true;
}

BorderPaint ParseBorderPaint(std::wstring const& text, uint32_t fallback) {
    std::vector<uint32_t> colors;
    std::vector<std::pair<float, float>> points;
    BorderPaint p;
    bool angled = false;
    float degrees = 0;
    size_t at = 0;
    while (at < text.size()) {
        size_t start = text.find_first_not_of(L" \t,;", at);
        if (start == std::wstring::npos) break;
        size_t end = text.find_first_of(L" \t;", start);
        std::wstring token = text.substr(start, end == std::wstring::npos ? std::wstring::npos : end - start);
        at = end == std::wstring::npos ? text.size() : end;
        while (!token.empty() && token.back() == L',') token.pop_back();
        uint32_t c;
        float x, y;
        if (ParseBorderColor(token, &c)) {
            colors.push_back(c);
        } else if (token.size() > 3 && !_wcsicmp(token.c_str() + token.size() - 3, L"deg")) {
            angled = true;
            degrees = (float)_wtof(token.c_str());
        } else if (swscanf(token.c_str(), L"%f,%f", &x, &y) == 2) {
            points.push_back({x, y});
        }
    }
    if (colors.empty()) colors.push_back(fallback);
    if (colors.size() == 1) {
        std::fill(std::begin(p.lut), std::end(p.lut), colors[0]);
        return p;
    }
    p.gradient = true;
    if (points.size() >= 2) {
        p.sx = points[0].first, p.sy = points[0].second, p.ex = points[1].first, p.ey = points[1].second;
    } else {
        // As Tacky Borders reads an angle: 0deg left to right, 90deg bottom
        // to top, reaching the box's edges.
        double rad = (angled ? degrees : 0.0) * 3.14159265358979 / 180.0;
        double x0 = std::cos(rad), y0 = -std::sin(rad);
        double scale = 0.5 / std::max(std::abs(x0), std::abs(y0));
        p.sx = (float)(0.5 - x0 * scale), p.sy = (float)(0.5 - y0 * scale);
        p.ex = (float)(0.5 + x0 * scale), p.ey = (float)(0.5 + y0 * scale);
    }
    // The stops evenly spread, mixed as they're written (sRGB).
    int n = (int)colors.size();
    for (int i = 0; i < 256; i++) {
        float t = i / 255.0f * (n - 1);
        int k = std::min((int)t, n - 2);
        float f = t - k;
        uint32_t a = colors[k], b = colors[k + 1], out = 0;
        for (int shift = 0; shift < 32; shift += 8) {
            float va = (float)((a >> shift) & 0xFF), vb = (float)((b >> shift) & 0xFF);
            out |= (uint32_t)std::lround(va + (vb - va) * f) << shift;
        }
        p.lut[i] = out;
    }
    return p;
}

void LoadBorderPaints() {
    g_activePaint = ParseBorderPaint(g_borderSettings.activeText, BorderAccent());
    g_inactivePaint = ParseBorderPaint(g_borderSettings.inactiveText, 0x00000000);
}

void LoadBorderSettings() {
    BorderSettings s;
    s.on = Wh_GetIntSetting(L"borders.enabled") != 0;
    s.width = IntSetting(L"borders.width", 0, 64);
    s.offset = IntSetting(L"borders.offset", -64, 64);
    std::wstring radius = TextSetting(L"borders.radius");
    s.radius = radius == L"square" ? 0 : radius == L"round" ? 8 : radius == L"roundSmall" ? 4
             : radius == L"custom" ? IntSetting(L"borders.radiusCustom", 0, 200) : -1;
    s.below = TextSetting(L"borders.zOrder") == L"below";
    s.followNative = Wh_GetIntSetting(L"borders.followNative") != 0;
    s.initDelay = IntSetting(L"borders.initDelay", 0, 5000);
    s.unminimizeDelay = IntSetting(L"borders.unminimizeDelay", 0, 5000);
    s.activeText = TextSetting(L"borders.activeColor");
    s.inactiveText = TextSetting(L"borders.inactiveColor");
    auto spin = [](std::wstring const& v) { return v == L"spiral" ? 1 : v == L"reverseSpiral" ? -1 : 0; };
    s.spinActive = spin(TextSetting(L"borders.activeSpin"));
    s.spinInactive = spin(TextSetting(L"borders.inactiveSpin"));
    s.spinMs = IntSetting(L"borders.spinDuration", 100, 60000);
    s.spinOnce = TextSetting(L"borders.spinRepeat") != L"always";
    s.frontMoves = TextSetting(L"borders.followMoves") != L"all";
    s.fade = Wh_GetIntSetting(L"borders.fade") != 0;
    s.fadeMs = IntSetting(L"borders.fadeDuration", 1, 5000);
    // The motion tree's lines, when set (motion.cpp): "border" for the colour
    // fade (else "fadeSwitch", the active window changing: "border" is the
    // colours' own, so it wins), "fadeGlow" and "fadeShadow" for the glow's
    // and the shadow's, "glowangle" and "borderangle" for one turn of the
    // glow's gradient and of the border's.
    if (motion::Anim m = motion::Get(L"border"); m.set) {
        s.fade = m.on;
        if (m.on) s.fadeMs = std::clamp((int)motion::Ms(m), 1, 5000);
        s.fadeLine[kFadeBorder] = m;
    } else if (motion::Anim m = motion::Get(L"fadeSwitch"); m.set) {
        s.fade = m.on;
        if (m.on) s.fadeMs = std::clamp((int)motion::Ms(m), 1, 5000);
        s.fadeLine[kFadeBorder] = m;
    }
    s.fadeLine[kFadeGlow] = motion::Get(L"fadeGlow");
    s.fadeLine[kFadeShadow] = motion::Get(L"fadeShadow");
    if (motion::Anim m = motion::Get(L"glowangle"); m.set) {
        s.glowTurns = true;
        if (m.on) s.glowSpinMs = std::clamp((int)motion::Ms(m), 100, 60000);
        s.glowSpinActive = m.on ? s.spinActive : 0;
        s.glowSpinInactive = m.on ? s.spinInactive : 0;
    }
    if (motion::Anim m = motion::Get(L"borderangle"); m.set) {
        if (m.on) s.spinMs = std::clamp((int)motion::Ms(m), 100, 60000);
        else s.spinActive = s.spinInactive = 0;  // (off, or every animation off: it stands still)
    }
    s.fps = IntSetting(L"borders.fps", 1, 240);
    auto effect = [](std::wstring const& v) { return v == L"active" ? kEffectActive : v == L"all" ? kEffectAll : kEffectNone; };
    s.glow = effect(TextSetting(L"borders.glow"));
    s.glowBlur = IntSetting(L"borders.glowBlur", 1, 64);
    s.glowSpread = IntSetting(L"borders.glowSpread", 0, 64);
    s.glowOpacity = IntSetting(L"borders.glowOpacity", 0, 100);
    s.shadow = effect(TextSetting(L"borders.shadow"));
    s.shadowBlur = IntSetting(L"borders.shadowBlur", 1, 64);
    s.shadowOpacity = IntSetting(L"borders.shadowOpacity", 0, 100);
    s.shadowX = IntSetting(L"borders.shadowX", -200, 200);
    s.shadowY = IntSetting(L"borders.shadowY", -200, 200);
    s.skipApps = TextSetting(L"borders.skipApps");
    s.skipClasses = TextSetting(L"borders.skipClasses");
    s.skipTitles = TextSetting(L"borders.skipTitles");
    g_borderSettings = s;
    LoadBorderPaints();
}

////////////////////////////////////////////////////////////////////////////////
// Which windows

// A window that may wear a border: an app's own top-level window, not a tool
// window, a menu, the shell's or the suite's, and not left out by the rules.
std::unordered_set<HWND> g_noBorder;  // windows a window rule (noborder) leaves without one

bool BorderEligible(HWND h) {
    if (!IsWindow(h) || GetAncestor(h, GA_ROOT) != h) return false;
    if (g_noBorder.contains(h)) return false;
    LONG style = GetWindowLongW(h, GWL_STYLE), ex = GetWindowLongW(h, GWL_EXSTYLE);
    if ((style & WS_CHILD) || (ex & (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE))) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId()) return false;
    WCHAR cls[128] = L"";
    GetClassNameW(h, cls, ARRAYSIZE(cls));
    if (!wcsncmp(cls, L"Tourne", 6)) return false;
    for (PCWSTR shell : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd", L"Progman", L"WorkerW", L"#32768"})
        if (!wcscmp(cls, shell)) return false;
    auto const& s = g_borderSettings;
    if (!s.skipClasses.empty() && MatchesList(s.skipClasses, cls)) return false;
    if (!s.skipTitles.empty() && MatchesList(s.skipTitles, Title(h))) return false;
    if (!s.skipApps.empty()) {
        std::wstring program = ProgramOf(h);
        if (MatchesList(s.skipApps, program.substr(program.find_last_of(L'\\') + 1))) return false;
    }
    return true;
}

// Whether its border shows now: in view, and (following Windows) only while
// Windows would draw a border of its own, so not maximised or full screen.
bool BorderWanted(HWND h) {
    if (!IsWindowVisible(h) || IsIconic(h)) return false;
    if (LeftAlone(h)) return false;  // a program Tourne leaves alone (a game): nothing drawn around it
    DWORD cloaked = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return false;
    if (g_borderSettings.followNative &&
        ((GetWindowLongW(h, GWL_STYLE) & WS_MAXIMIZE) || !(GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_WINDOWEDGE)))
        return false;
    return true;
}

// The radius at 100% the window's own corners have: Windows 11 rounds them
// (8 px, or 4 for small ones) unless a window asks it not to, and squares
// them while a window is snapped.
int AutoRadius(HWND h) {
    using IsArranged_t = BOOL(WINAPI*)(HWND);
    static auto isArranged = (IsArranged_t)GetProcAddress(GetModuleHandleW(L"user32.dll"), "IsWindowArranged");
    if (isArranged && isArranged(h)) return 0;
    DWORD corner = 0;  // DWMWCP_DEFAULT
    if (FAILED(DwmGetWindowAttribute(h, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corner, sizeof(corner))))
        return 0;  // (Windows 10: square)
    return corner == 1 /* DONOTROUND */ ? 0 : corner == 3 /* ROUNDSMALL */ ? 4 : 8;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing

// The signed distance from a point to a rounded rectangle: negative inside.
float RoundRectDistance(float px, float py, float l, float t, float r, float b, float radius) {
    float hx = (r - l) / 2, hy = (b - t) / 2;
    if (hx <= 0 || hy <= 0) return 1e9f;  // nothing there
    float rr = std::clamp(radius, 0.0f, std::min(hx, hy));
    float qx = std::abs(px - (l + r) / 2) - (hx - rr), qy = std::abs(py - (t + b) / 2) - (hy - rr);
    float outside = std::hypot(std::max(qx, 0.0f), std::max(qy, 0.0f));
    return outside + std::min(std::max(qx, qy), 0.0f) - rr;
}

void FreeStripBitmap(BorderStrip& s) {
    if (s.dc) {
        SelectObject(s.dc, s.old);
        DeleteDC(s.dc);
    }
    if (s.bitmap) DeleteObject(s.bitmap);
    s.dc = nullptr, s.bitmap = nullptr, s.old = nullptr, s.bits = nullptr, s.bw = s.bh = 0;
}

// A band's coverage once blurred (a Gaussian blur, as Tacky Borders' glow
// and shadow): m is the distance from the band's middle, half its half width.
inline float BlurredBand(float m, float half, float sigma) {
    float k = 1.0f / (sigma * 1.41421356f);
    return 0.5f * (std::erf((m + half) * k) - std::erf((m - half) * k));
}

// A filled shape's coverage once blurred (Gaussian): d is the distance from its edge, inside negative.
inline float BlurredEdge(float d, float sigma) {
    return 0.5f * std::erfc(d / (sigma * 1.41421356f));
}

// The border's shape for the window's frame now: the canvas, the strips and
// how much of each pixel the border, its glow and its shadow cover. True if
// it changed.
bool LayOutBorder(Border& b, RECT const& frame) {
    auto const& s = g_borderSettings;
    UINT dpi = MonitorDpi(MonitorFromWindow(b.target, MONITOR_DEFAULTTONEAREST));
    int radius = s.radius < 0 ? AutoRadius(b.target) : s.radius;
    auto scaled = [dpi](float v) { return v * (float)dpi / 96.0f; };
    int w = (int)std::lround(scaled((float)s.width)), offset = (int)std::lround(scaled((float)s.offset));
    float innerR = radius ? scaled((float)radius) : 0, outerR = radius ? innerR + w : 0;
    bool glows = s.glow != kEffectNone && s.glowOpacity > 0, shadows = s.shadow != kEffectNone && s.shadowOpacity > 0;
    float glowSigma = scaled((float)s.glowBlur), glowSpread = scaled((float)s.glowSpread);
    float shadowSigma = scaled((float)s.shadowBlur), shadowX = scaled((float)s.shadowX), shadowY = scaled((float)s.shadowY);
    // Room for the glow and the shadow around the border (outside it and in).
    int pad = 0;
    if (glows) pad = std::max(pad, (int)std::ceil(3 * glowSigma + glowSpread));
    if (shadows) pad = std::max(pad, (int)std::ceil(3 * shadowSigma + std::max(std::abs(shadowX), std::abs(shadowY))));
    int cw = (frame.right - frame.left) + 2 * (w + offset + pad), ch = (frame.bottom - frame.top) + 2 * (w + offset + pad);
    bool same = b.dpi == (int)dpi && b.w == w && b.pad == pad && b.outerR == outerR &&
                b.canvas.right - b.canvas.left == cw && b.canvas.bottom - b.canvas.top == ch;
    b.frame = frame;
    b.canvas = {frame.left - (w + offset + pad), frame.top - (w + offset + pad), 0, 0};
    b.canvas.right = b.canvas.left + cw, b.canvas.bottom = b.canvas.top + ch;
    if (same && !b.strips[0].ring.empty()) return false;
    b.dpi = (int)dpi, b.w = w, b.pad = pad, b.outerR = outerR, b.innerR = innerR;
    b.box = {pad, pad, cw - pad, ch - pad};
    // The strips: deep enough to hold the border where its corners curve in,
    // and its glow and shadow on both sides.
    int band = pad + w + (int)std::ceil(outerR * 0.293f) + pad + 2;
    band = std::max(1, std::min({band, cw / 2, ch / 2}));
    b.band = band;
    b.strips[0].rc = {0, 0, cw, band};
    b.strips[1].rc = {0, ch - band, cw, ch};
    b.strips[2].rc = {0, band, band, ch - band};
    b.strips[3].rc = {cw - band, band, cw, ch - band};
    RECT const& o = b.box;
    float ol = (float)o.left, ot = (float)o.top, orr = (float)o.right, ob = (float)o.bottom;
    float il = ol + w, it = ot + w, ir = orr - w, ib = ob - w;
    float half = w / 2.0f;
    float glowOpacity = s.glowOpacity / 100.0f;
    float tightSigma = std::max(0.75f, shadowSigma * 0.3f);
    for (auto& strip : b.strips) {
        int sw = std::max(0, (int)(strip.rc.right - strip.rc.left)), sh = std::max(0, (int)(strip.rc.bottom - strip.rc.top));
        size_t count = (size_t)sw * sh;
        strip.ring.assign(count, 0);
        strip.glow.assign(glows ? count : 0, 0);
        strip.shadow.assign(shadows ? count : 0, 0);
        for (int y = 0; y < sh; y++) {
            float py = strip.rc.top + y + 0.5f;
            for (int x = 0; x < sw; x++) {
                float px = strip.rc.left + x + 0.5f;
                size_t i = (size_t)y * sw + x;
                float dOuter = RoundRectDistance(px, py, ol, ot, orr, ob, outerR);
                float dInner = RoundRectDistance(px, py, il, it, ir, ib, innerR);
                float ring = std::clamp(0.5f - dOuter, 0.0f, 1.0f) * std::clamp(0.5f + dInner, 0.0f, 1.0f);
                strip.ring[i] = (uint8_t)std::lround(ring * 255);
                if (glows) {
                    float g = BlurredBand(std::abs(dOuter + half), half + glowSpread, glowSigma);
                    strip.glow[i] = (uint8_t)std::lround(std::clamp(g * glowOpacity, 0.0f, 1.0f) * 255);
                }
                if (shadows) {
                    // A drop shadow: the window's whole shape blurred, as a lit window casts one. A tight
                    // layer close under it (half the offset) and a wide soft one (the whole offset), and
                    // only outside the border, as it's drawn over the window's edge.
                    float soft = BlurredEdge(RoundRectDistance(px - shadowX, py - shadowY, ol, ot, orr, ob, outerR), shadowSigma);
                    float tight = BlurredEdge(RoundRectDistance(px - shadowX * 0.5f, py - shadowY * 0.5f, ol, ot, orr, ob, outerR),
                                              tightSigma);
                    float outside = std::clamp(dOuter + 0.5f, 0.0f, 1.0f);
                    strip.shadow[i] = (0.6f * soft + 0.4f * tight) * outside;
                }
            }
        }
    }
    return true;
}

// The colour (0xAARRGGBB, straight) a paint gives a point of the canvas, its
// gradient turned by the border's turn.
inline uint32_t PaintAt(BorderPaint const& p, Border const& b, float px, float py, float cosT, float sinT) {
    if (!p.gradient) return p.lut[0];
    float bw = (float)(b.box.right - b.box.left), bh = (float)(b.box.bottom - b.box.top);
    float cx = b.box.left + bw / 2, cy = b.box.top + bh / 2;
    // The point turned back, so the gradient turns forward.
    float dx = px - cx, dy = py - cy;
    float rx = cx + dx * cosT + dy * sinT, ry = cy - dx * sinT + dy * cosT;
    float sx = b.box.left + p.sx * bw, sy = b.box.top + p.sy * bh;
    float ex = b.box.left + p.ex * bw, ey = b.box.top + p.ey * bh;
    float vx = ex - sx, vy = ey - sy, len = vx * vx + vy * vy;
    float t = len > 0 ? ((rx - sx) * vx + (ry - sy) * vy) / len : 0;
    return p.lut[(int)std::lround(std::clamp(t, 0.0f, 1.0f) * 255)];
}

// A part's share of the active look now (0 to 1): by its own motion line's
// fade, else as the border's (by its line, else eased in and out).
float BorderWeight(Border const& b, int part) {
    auto const& s = g_borderSettings;
    if (part != kFadeBorder && !s.fadeLine[part].set) part = kFadeBorder;
    if (s.fadeLine[part].set) return b.fades[part].shown;
    float mix = b.mix;
    return s.fade ? (mix < 0.5f ? 2 * mix * mix : 1 - 2 * (1 - mix) * (1 - mix)) : mix;
}

// The glow turning on its own ("glowangle"), there being a glow.
bool GlowTurnsAlone() {
    auto const& s = g_borderSettings;
    return s.glowTurns && s.glow != kEffectNone && s.glowOpacity > 0;
}

// Draws the strips as the border looks now and gives them to DWM (moved and
// sized too).
void PaintBorder(Border& b) {
    auto const& s = g_borderSettings;
    HDC screen = GetDC(nullptr);
    float turn = b.turn * 6.28318531f, cosT = std::cos(turn), sinT = std::sin(turn);
    bool glowAlone = GlowTurnsAlone();
    float glowTurn = (glowAlone ? b.glowTurn : b.turn) * 6.28318531f, cosG = std::cos(glowTurn), sinG = std::sin(glowTurn);
    float mixRing = BorderWeight(b, kFadeBorder), mixGlow = BorderWeight(b, kFadeGlow), mixShadow = BorderWeight(b, kFadeShadow);
    bool glowActive = s.glow != kEffectNone, glowInactive = s.glow == kEffectAll;
    bool shadowActive = s.shadow != kEffectNone, shadowInactive = s.shadow == kEffectAll;
    float shadowOpacity = s.shadowOpacity / 100.0f;
    // A 4 x 4 ordered dither on the alpha: a faint fade over a dark wallpaper has only a few dozen
    // steps of 8-bit alpha, which show as rings; spread by under a step each, they read as smooth.
    static constexpr float kDither[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5};
    for (auto& strip : b.strips) {
        int sw = strip.rc.right - strip.rc.left, sh = strip.rc.bottom - strip.rc.top;
        if (!strip.wnd || sw <= 0 || sh <= 0 || strip.ring.size() != (size_t)sw * sh) continue;
        if (strip.bw != sw || strip.bh != sh) {
            FreeStripBitmap(strip);
            BITMAPINFO bi{};
            bi.bmiHeader = {sizeof(BITMAPINFOHEADER), sw, -sh, 1, 32, BI_RGB};
            void* bits = nullptr;
            strip.dc = CreateCompatibleDC(screen);
            strip.bitmap = CreateDIBSection(strip.dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (!strip.bitmap) {
                FreeStripBitmap(strip);
                continue;
            }
            strip.old = SelectObject(strip.dc, strip.bitmap);
            strip.bits = (uint32_t*)bits;
            strip.bw = sw, strip.bh = sh;
        }
        for (int y = 0; y < sh; y++) {
            float py = strip.rc.top + y + 0.5f;
            for (int x = 0; x < sw; x++) {
                size_t i = (size_t)y * sw + x;
                float ring = strip.ring[i] / 255.0f;
                float glow = strip.glow.empty() ? 0 : strip.glow[i] / 255.0f;
                float shadow = strip.shadow.empty() ? 0 : strip.shadow[i] * shadowOpacity;
                if (ring <= 0 && glow <= 0 && shadow <= 0) {
                    strip.bits[i] = 0;
                    continue;
                }
                float px = strip.rc.left + x + 0.5f;
                float r = 0, g = 0, bl = 0, a = 0;
                auto paint = [&](uint32_t c, float cover, float weight) {
                    float ca = (c >> 24) / 255.0f * cover * weight;
                    r += ((c >> 16) & 0xFF) * ca, g += ((c >> 8) & 0xFF) * ca, bl += (c & 0xFF) * ca, a += ca;
                };
                // One look's pixel, each part weighted: the border over its
                // glow (both in its colour) over its shadow (black),
                // premultiplied.
                auto add = [&](BorderPaint const& p, float wRing, float wGlow, float wShadow, bool glows, bool shadows) {
                    if (wRing <= 0 && wGlow <= 0 && wShadow <= 0) return;
                    float halo = glows ? glow * (1 - ring) : 0, cover = ring + halo;
                    float dark = shadows ? shadow * (1 - cover) : 0;
                    if (glowAlone && halo > 0 && p.gradient) {
                        paint(PaintAt(p, b, px, py, cosT, sinT), ring, wRing);
                        paint(PaintAt(p, b, px, py, cosG, sinG), halo, wGlow);
                    } else if (cover > 0) {
                        uint32_t c = PaintAt(p, b, px, py, cosT, sinT);
                        if (wGlow == wRing) paint(c, cover, wRing);  // (fading together: as it always was)
                        else paint(c, ring * wRing + halo * wGlow, 1);
                    }
                    a += dark * wShadow;
                };
                add(g_activePaint, mixRing, mixGlow, mixShadow, glowActive, shadowActive);
                add(g_inactivePaint, 1 - mixRing, 1 - mixGlow, 1 - mixShadow, glowInactive, shadowInactive);
                float k = b.appear;
                float dither = (kDither[(y & 3) * 4 + (x & 3)] + 0.5f) / 16.0f - 0.5f;
                uint32_t a8 = (uint32_t)std::clamp((long)std::lround(std::min(a * k, 1.0f) * 255 + dither), 0L, 255L);
                auto channel = [&](float v) { return std::min((uint32_t)std::lround(std::min(v * k, 255.0f)), a8); };  // (premultiplied: never over alpha)
                strip.bits[i] = (a8 << 24) | (channel(r) << 16) | (channel(g) << 8) | channel(bl);
            }
        }
        GdiFlush();
        POINT src{0, 0}, dst{b.canvas.left + strip.rc.left, b.canvas.top + strip.rc.top};
        SIZE size{sw, sh};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(strip.wnd, screen, &dst, &size, strip.dc, &src, 0, &blend, ULW_ALPHA);
    }
    ReleaseDC(nullptr, screen);
}

////////////////////////////////////////////////////////////////////////////////
// Following the windows

bool IsBorderStrip(Border const& b, HWND h) {
    for (auto const& strip : b.strips)
        if (strip.wnd && strip.wnd == h) return true;
    return false;
}

// The strips put beside the window in the z-order (just above it, or just
// below), moved to the canvas and shown; or hidden.
void PlaceStrips(Border& b, bool show, bool move) {
    auto const& s = g_borderSettings;
    HWND insertAfter = b.target;
    UINT zFlag = 0;
    if (show && !s.below) {
        HWND above = GetWindow(b.target, GW_HWNDPREV);
        bool topmost = GetWindowLongW(b.target, GWL_EXSTYLE) & WS_EX_TOPMOST;
        if (above && IsBorderStrip(b, above))
            zFlag = SWP_NOZORDER;
        else if (!above || (!topmost && (GetWindowLongW(above, GWL_EXSTYLE) & WS_EX_TOPMOST)))
            insertAfter = topmost ? HWND_TOPMOST : HWND_TOP;  // (not among the always-on-top windows above it)
        else
            insertAfter = above;
    } else if (show) {
        HWND next = GetWindow(b.target, GW_HWNDNEXT);
        if (next && IsBorderStrip(b, next)) zFlag = SWP_NOZORDER;
    }
    // Already there: nothing to do (the usual case, as windows reorder).
    if (show && b.shown && !move && zFlag) return;
    auto flagsFor = [&](BorderStrip const& strip) {
        int sw = strip.rc.right - strip.rc.left, sh = strip.rc.bottom - strip.rc.top;
        bool here = show && sw > 0 && sh > 0;
        return SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING | zFlag |
               (here ? SWP_SHOWWINDOW : SWP_HIDEWINDOW | SWP_NOZORDER) | (move ? 0 : SWP_NOMOVE);
    };
    // Together, so they move as one; one by one if that's refused. Next to a
    // program running as administrator, Windows won't let them be put in the
    // z-order beside it (Tacky Borders' "Access is denied"): they're moved
    // and shown without it then.
    HDWP defer = BeginDeferWindowPos(4);
    for (auto& strip : b.strips) {
        if (!strip.wnd || !defer) continue;
        defer = DeferWindowPos(defer, strip.wnd, insertAfter, b.canvas.left + strip.rc.left, b.canvas.top + strip.rc.top, 0, 0,
                               flagsFor(strip));
    }
    if (!defer || !EndDeferWindowPos(defer)) {
        for (auto& strip : b.strips) {
            if (!strip.wnd) continue;
            int x = b.canvas.left + strip.rc.left, y = b.canvas.top + strip.rc.top;
            if (!SetWindowPos(strip.wnd, insertAfter, x, y, 0, 0, flagsFor(strip)))
                SetWindowPos(strip.wnd, nullptr, x, y, 0, 0, flagsFor(strip) | SWP_NOZORDER);
        }
    }
    b.shown = show;
}

void StartBorderTimer();

// Whether a border's gradient turns, or its glow's (which keeps the timer going).
bool BorderTurns() {
    auto const& s = g_borderSettings;
    return s.spinActive || s.spinInactive || (GlowTurnsAlone() && (s.glowSpinActive || s.glowSpinInactive));
}

// Whether this border has turning to do: always with the spin set to always,
// else until its one turn is done.
bool BorderSpinning(Border const& b) {
    return BorderTurns() && (!g_borderSettings.spinOnce || b.spinLeft > 0 || b.glowSpinLeft > 0);
}

// The parts with a motion line of their own set off toward the border's new
// look from where they are (or put there at once, the line being off). True
// if any fades.
bool RetargetBorderFades(Border& b) {
    bool fades = false;
    for (int part = kFadeBorder; part <= kFadeShadow; part++) {
        motion::Anim const& line = g_borderSettings.fadeLine[part];
        if (!line.set) continue;
        auto& f = b.fades[part];
        f.from = f.shown, f.t = 0;
        if (motion::Ms(line) > 0) fades = true;
        else f.shown = b.mixTarget, f.t = 1;
    }
    return fades;
}

// Those fades a step on (dt in ms), eased by their lines' curves. True while
// any is still on its way.
bool StepBorderFades(Border& b, float dt) {
    bool fading = false;
    for (int part = kFadeBorder; part <= kFadeShadow; part++) {
        motion::Anim const& line = g_borderSettings.fadeLine[part];
        auto& f = b.fades[part];
        if (!line.set || f.t >= 1) continue;
        double ms = motion::Ms(line);
        f.t = ms > 0 ? std::min(1.0f, f.t + (float)(dt / ms)) : 1.0f;
        float eased = (float)motion::Ease(line, f.t);
        f.shown = f.t >= 1 ? b.mixTarget : std::clamp(f.from + (b.mixTarget - f.from) * eased, 0.0f, 1.0f);
        fading = true;
    }
    return fading;
}

// Brought up to date with its window: hidden if it's not to show, else laid
// out again (drawn again if its shape changed) and put beside it.
void RefreshBorder(Border& b) {
    if (b.showAt) return;  // waiting to show
    if (!BorderWanted(b.target)) {
        if (b.shown) PlaceStrips(b, false, false);
        return;
    }
    RECT frame{};
    if (FAILED(DwmGetWindowAttribute(b.target, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(frame))))
        GetWindowRect(b.target, &frame);
    bool wasShown = b.shown;
    bool moved = !EqualRect(&frame, &b.frame);
    if (LayOutBorder(b, frame) || !wasShown) {
        PaintBorder(b);
        PlaceStrips(b, true, true);
    } else if (moved) {
        PlaceStrips(b, true, true);
    } else {
        PlaceStrips(b, true, false);
    }
}

void DestroyBorder(Border& b) {
    for (auto& strip : b.strips) {
        FreeStripBitmap(strip);
        if (strip.wnd) DestroyWindow(strip.wnd);
        strip.wnd = nullptr;
    }
}

Border* FindBorder(HWND h) {
    auto it = g_borders.find(h);
    return it == g_borders.end() ? nullptr : it->second.get();
}

// A window shown (or its border called for): its border made if it has none,
// and shown after the delay given (for Windows' opening animation).
void ShowBorderFor(HWND h, int delay) {
    Border* b = FindBorder(h);
    if (!b) {
        if (g_borderSkipped.contains(h) || GetAncestor(h, GA_ROOT) != h || !BorderWanted(h)) return;
        if (!BorderEligible(h)) {
            if (IsWindow(h)) g_borderSkipped.insert(h);
            return;
        }
        auto made = std::make_unique<Border>();
        made->target = h;
        HINSTANCE instance = (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE);
        for (auto& strip : made->strips) {
            strip.wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kBorderClass,
                                        L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
            // (Not kept out of a peek: a peek fades them with the windows, rather than leaving every hidden
            // window's outline in view, as Windows' own accent borders were.)
        }
        bool front = h == g_borderFront;
        made->mix = made->mixTarget = front ? 1.0f : 0.0f;
        if (front) made->spinLeft = made->glowSpinLeft = 1;
        for (auto& f : made->fades) f.shown = made->mix;
        b = made.get();
        g_borders[h] = std::move(made);
    }
    if (g_borderSettings.fade && !b->shown) b->appear = 0;
    if (delay > 0) {
        b->showAt = GetTickCount64() + delay;
        if (b->shown) PlaceStrips(*b, false, false);
        StartBorderTimer();
        return;
    }
    b->showAt = 0;
    RefreshBorder(*b);
    if (b->appear < 1 || BorderSpinning(*b)) StartBorderTimer();
}

void HideBorderFor(HWND h) {
    if (Border* b = FindBorder(h)) {
        b->showAt = 0;
        if (b->shown) PlaceStrips(*b, false, false);
    }
}

void CALLBACK BorderEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD);

// With frontMoves, window moves are followed for the window in front's thread
// only (one hook, moved along with the front), rather than every move on the
// PC, the mouse pointer's included. Drags end with a move-size event, and the
// tiling refreshes the borders of the windows it moves.
void FollowFrontMoves(HWND front) {
    if (!g_borderSettings.frontMoves || !g_borderHooks[0]) return;
    DWORD pid = 0, tid = front ? GetWindowThreadProcessId(front, &pid) : 0;
    if (g_borderMoveHook && tid == g_borderMoveThread) return;
    if (g_borderMoveHook) UnhookWinEvent(g_borderMoveHook);
    g_borderMoveHook = nullptr;
    g_borderMoveThread = 0;
    if (!tid || pid == GetCurrentProcessId()) return;
    g_borderMoveHook = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr, BorderEventProc, pid, tid,
                                       WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    if (g_borderMoveHook) g_borderMoveThread = tid;
}

// Every border brought up to date with its window (after the tiling moved
// windows, or a reorder, with frontMoves).
void RefreshAllBorders() {
    for (auto& [h, b] : g_borders)
        if (!b->showAt) RefreshBorder(*b);
}

// The window in front changed: the border that wore the active look takes
// the inactive one, and the window now in front's the active one (faded, if
// set).
void SetBorderFront(HWND h) {
    HWND front = h ? GetAncestor(h, GA_ROOT) : nullptr;
    if (front == g_borderFront) return;
    HWND before = g_borderFront;
    g_borderFront = front;
    bool fade = g_borderSettings.fade, partsFade = false;
    for (HWND which : {before, front}) {
        Border* b = which ? FindBorder(which) : nullptr;
        if (!b) continue;
        b->mixTarget = which == front ? 1.0f : 0.0f;
        b->spinLeft = b->glowSpinLeft = 1;  // (one turn in its new look, with the spin set to once)
        if (!fade) b->mix = b->mixTarget;
        if (RetargetBorderFades(*b)) partsFade = true;
        if (b->shown) {
            // The glow may differ between the two looks; the window in front
            // also comes up in the z-order, and its border with it.
            PaintBorder(*b);
            PlaceStrips(*b, true, false);
        }
    }
    FollowFrontMoves(front);
    if (fade || partsFade || BorderTurns()) StartBorderTimer();
}

////////////////////////////////////////////////////////////////////////////////
// Animations and waiting borders, on one timer that runs only while there are
// any (a turning gradient keeps it running).

void StartBorderTimer() {
    if (g_borderTimer || !g_wnd) return;
    g_borderTimer = true;
    g_borderTick = GetTickCount64();
    SetTimer(g_wnd, kTimerBorders, std::max(1, 1000 / g_borderSettings.fps), nullptr);
}

void BordersTimer(HWND wnd) {
    auto const& s = g_borderSettings;
    ULONGLONG now = GetTickCount64();
    float dt = (float)(now - g_borderTick);
    g_borderTick = now;
    bool busy = false;
    for (auto& [h, b] : g_borders) {
        if (b->showAt) {
            if (now >= b->showAt) {
                b->showAt = 0;
                RefreshBorder(*b);
            }
            busy = true;
            continue;
        }
        if (!b->shown) {
            b->mix = b->mixTarget;  // (no fade to see)
            for (auto& f : b->fades) f.shown = b->mixTarget, f.t = 1;
            continue;
        }
        bool again = false;
        float step = s.fade ? dt / s.fadeMs : 1.0f;
        if (b->mix != b->mixTarget) {
            b->mix = b->mix < b->mixTarget ? std::min(b->mixTarget, b->mix + step) : std::max(b->mixTarget, b->mix - step);
            again = true;
        }
        if (StepBorderFades(*b, dt)) again = true;
        if (b->appear < 1) {
            b->appear = std::min(1.0f, b->appear + step);
            again = true;
        }
        // With the spin set to once, the turn goes on until a whole one is
        // done (back where it started), then stands still: nothing drawn.
        auto turnBy = [&](float* turn, float* left, int way, int ms) {
            if (!way || (s.spinOnce && *left <= 0)) return;
            float part = dt / ms;
            if (s.spinOnce) {
                part = std::min(part, *left);
                *left -= part;
            }
            *turn = std::fmod(*turn + way * part + 1.0f, 1.0f);
            again = true;
        };
        turnBy(&b->turn, &b->spinLeft, b->mixTarget >= 1 ? s.spinActive : s.spinInactive, s.spinMs);
        turnBy(&b->glowTurn, &b->glowSpinLeft,
               !GlowTurnsAlone() ? 0 : b->mixTarget >= 1 ? s.glowSpinActive : s.glowSpinInactive, s.glowSpinMs);
        if (again) {
            PaintBorder(*b);
            busy = true;
        }
    }
    if (!busy) {
        KillTimer(wnd, kTimerBorders);
        g_borderTimer = false;
    }
}

// Windows reordered somewhere: each border shown is put back beside its
// window if it was left behind (a little later, once for a burst).
void BordersReorderTimer(HWND wnd) {
    KillTimer(wnd, kTimerBorderZ);
    if (g_borderSettings.frontMoves) {
        RefreshAllBorders();  // (a window behind may have moved too: this catches it up)
        return;
    }
    for (auto& [h, b] : g_borders)
        if (b->shown) PlaceStrips(*b, true, false);
}

void CALLBACK BorderEventProc(HWINEVENTHOOK, DWORD event, HWND h, LONG object, LONG child, DWORD, DWORD) {
    if (object == OBJID_CURSOR) return;
    switch (event) {
        case EVENT_OBJECT_LOCATIONCHANGE:
            if (object != OBJID_WINDOW || child != CHILDID_SELF || !h) return;
            if (Border* b = FindBorder(h)) RefreshBorder(*b);
            else ShowBorderFor(h, 0);  // (back from full screen, say)
            return;
        case EVENT_SYSTEM_MOVESIZEEND:
            if (Border* b = h ? FindBorder(h) : nullptr) RefreshBorder(*b);
            return;
        case EVENT_OBJECT_REORDER:
            if (object == OBJID_CLIENT && !g_borders.empty() && g_wnd) SetTimer(g_wnd, kTimerBorderZ, 16, nullptr);
            return;
        case EVENT_SYSTEM_FOREGROUND: {
            HWND fg = GetForegroundWindow();
            SetBorderFront(fg ? fg : h);
            return;
        }
        case EVENT_OBJECT_SHOW:
        case EVENT_OBJECT_UNCLOAKED:
            if (object == OBJID_WINDOW && child == CHILDID_SELF && h)
                ShowBorderFor(h, FindBorder(h) ? 0 : g_borderSettings.initDelay);
            return;
        case EVENT_OBJECT_HIDE:
        case EVENT_OBJECT_CLOAKED:
            if (object == OBJID_WINDOW && child == CHILDID_SELF && h) HideBorderFor(h);
            return;
        case EVENT_SYSTEM_MINIMIZESTART:
            HideBorderFor(h);
            return;
        case EVENT_SYSTEM_MINIMIZEEND:
            ShowBorderFor(h, g_borderSettings.unminimizeDelay);
            return;
        case EVENT_OBJECT_DESTROY:
            if (object == OBJID_WINDOW && child == CHILDID_SELF && h) {
                auto it = g_borders.find(h);
                if (it != g_borders.end()) {
                    DestroyBorder(*it->second);
                    g_borders.erase(it);
                }
                if (h == g_borderFront) g_borderFront = nullptr;
                g_borderSkipped.erase(h);
            }
            return;
    }
}

void StopBorders() {
    for (auto& hook : g_borderHooks) {
        if (hook) UnhookWinEvent(hook);
        hook = nullptr;
    }
    if (g_borderMoveHook) UnhookWinEvent(g_borderMoveHook);
    g_borderMoveHook = nullptr;
    g_borderMoveThread = 0;
    if (g_wnd) {
        KillTimer(g_wnd, kTimerBorders);
        KillTimer(g_wnd, kTimerBorderZ);
    }
    g_borderTimer = false;
    for (auto& [h, b] : g_borders) DestroyBorder(*b);
    g_borders.clear();
    g_borderSkipped.clear();
    g_borderFront = nullptr;
}

// On the switcher's thread. Nothing is followed while the borders are off.
void StartBorders() {
    StopBorders();
    if (!g_borderSettings.on || !g_wnd) return;
    if (!g_borderClass) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = DefWindowProcW;  // (nothing of this module's: see-through, never clicked)
        wc.hInstance = (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE);
        wc.lpszClassName = kBorderClass;
        g_borderClass = RegisterClassW(&wc) != 0;
    }
    DWORD flags = WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS;
    g_borderHooks[0] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[1] = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[2] = SetWinEventHook(EVENT_OBJECT_DESTROY, EVENT_OBJECT_REORDER, nullptr, BorderEventProc, 0, 0, flags);
    if (!g_borderSettings.frontMoves)
        g_borderHooks[3] = SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[4] = SetWinEventHook(EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED, nullptr, BorderEventProc, 0, 0, flags);
    g_borderHooks[5] = SetWinEventHook(EVENT_SYSTEM_MOVESIZEEND, EVENT_SYSTEM_MOVESIZEEND, nullptr, BorderEventProc, 0, 0, flags);
    HWND fg = GetForegroundWindow();
    g_borderFront = fg ? GetAncestor(fg, GA_ROOT) : nullptr;
    FollowFrontMoves(g_borderFront);
    // The windows open now, at once (no opening animation to wait for).
    std::vector<HWND> open;
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            ((std::vector<HWND>*)param)->push_back(h);
            return TRUE;
        },
        (LPARAM)&open);
    for (HWND h : open) ShowBorderFor(h, 0);
}

// Windows' accent colour changed: borders that use it take the new one.
void BordersAccentChanged() {
    if (g_borders.empty()) return;
    LoadBorderPaints();
    for (auto& [h, b] : g_borders)
        if (b->shown) PaintBorder(*b);
}

// The switcher closing: the borders go, and their window class with them.
void EndBorders() {
    StopBorders();
    if (g_borderClass && g_wnd) UnregisterClassW(kBorderClass, (HINSTANCE)GetWindowLongPtrW(g_wnd, GWLP_HINSTANCE));
    g_borderClass = false;
}
