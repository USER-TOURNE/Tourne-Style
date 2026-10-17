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
