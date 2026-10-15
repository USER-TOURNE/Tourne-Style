#include "palette-gen.h"

#include <algorithm>
#include <cmath>

namespace td {

namespace {

constexpr double kPi = 3.14159265358979323846;

struct Lab {
    double L, a, b;
};

double ToLinear(double c) { return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4); }
double ToGamma(double c) { return c <= 0.0031308 ? 12.92 * c : 1.055 * std::pow(c, 1 / 2.4) - 0.055; }

Lab ToLab(uint32_t rgb) {
    double r = ToLinear(((rgb >> 16) & 0xFF) / 255.0), g = ToLinear(((rgb >> 8) & 0xFF) / 255.0),
           b = ToLinear((rgb & 0xFF) / 255.0);
    double l = std::cbrt(0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b);
    double m = std::cbrt(0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b);
    double s = std::cbrt(0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b);
    return {0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s, 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s,
            0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s};
}

// Linear sRGB of a Lab colour; false if it's outside what a screen shows.
bool ToLinearRgb(Lab c, double out[3]) {
    double l = c.L + 0.3963377774 * c.a + 0.2158037573 * c.b;
    double m = c.L - 0.1055613458 * c.a - 0.0638541728 * c.b;
    double s = c.L - 0.0894841775 * c.a - 1.2914855480 * c.b;
    l = l * l * l, m = m * m * m, s = s * s * s;
    out[0] = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    out[1] = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    out[2] = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;
    for (int i = 0; i < 3; i++)
        if (out[i] < -1e-4 || out[i] > 1 + 1e-4) return false;
    return true;
}

// Lightness, chroma, hue (degrees) to a colour, with the chroma lowered as
// far as needed to stay on screen (the hue and lightness are kept).
uint32_t FromLch(double L, double C, double h) {
    L = std::clamp(L, 0.0, 1.0);
    double rad = h * kPi / 180, rgb[3];
    auto at = [&](double c) { return Lab{L, c * std::cos(rad), c * std::sin(rad)}; };
    if (!ToLinearRgb(at(C), rgb)) {
        double lo = 0, hi = C;
        for (int i = 0; i < 20; i++) {
            double mid = (lo + hi) / 2;
            (ToLinearRgb(at(mid), rgb) ? lo : hi) = mid;
        }
        ToLinearRgb(at(lo), rgb);
    }
    uint32_t out = 0;
    for (int i = 0; i < 3; i++) {
        int v = (int)std::lround(ToGamma(std::clamp(rgb[i], 0.0, 1.0)) * 255);
        out = out << 8 | (uint32_t)std::clamp(v, 0, 255);
    }
    return out;
}

double Chroma(Lab c) { return std::hypot(c.a, c.b); }
double Hue(Lab c) {
    double h = std::atan2(c.b, c.a) * 180 / kPi;
    return h < 0 ? h + 360 : h;
}
double HueDistance(double a, double b) {
    double d = std::fabs(a - b);
    return d > 180 ? 360 - d : d;
}

double Luminance(uint32_t rgb) {
    return 0.2126 * ToLinear(((rgb >> 16) & 0xFF) / 255.0) + 0.7152 * ToLinear(((rgb >> 8) & 0xFF) / 255.0) +
           0.0722 * ToLinear((rgb & 0xFF) / 255.0);
}

struct Cluster {
    Lab centre;
    double weight;  // share of the picture, 0..1
};

// k-means in Lab, started the same way every time (the same picture always
// gives the same palette).
std::vector<Cluster> Clusters(const std::vector<uint32_t>& pixels, int k) {
    std::vector<Lab> points;
    points.reserve(pixels.size());
    for (uint32_t p : pixels) points.push_back(ToLab(p & 0xFFFFFF));
    std::vector<Cluster> out;
    if (points.empty()) return out;
    auto dist = [](Lab const& x, Lab const& y) {
        double dl = x.L - y.L, da = x.a - y.a, db = x.b - y.b;
        return dl * dl + da * da + db * db;
    };
    // k-means++ with a fixed sequence: the next centre is the point farthest
    // (weighted) from those chosen, picked by a deterministic walk.
    std::vector<Lab> centres{points[points.size() / 2]};
    std::vector<double> nearest(points.size(), 1e9);
    uint32_t seed = 0x2545F491;
    while ((int)centres.size() < k) {
        double total = 0;
        for (size_t i = 0; i < points.size(); i++) {
            nearest[i] = std::min(nearest[i], dist(points[i], centres.back()));
            total += nearest[i];
        }
        if (total <= 0) break;
        seed = seed * 1664525 + 1013904223;
        double pick = (seed >> 8) / double(1 << 24) * total;
        size_t chosen = points.size() - 1;
        for (size_t i = 0; i < points.size(); i++) {
            pick -= nearest[i];
            if (pick <= 0) {
                chosen = i;
                break;
            }
        }
        centres.push_back(points[chosen]);
    }
    std::vector<int> owner(points.size(), 0);
    for (int iteration = 0; iteration < 10; iteration++) {
        for (size_t i = 0; i < points.size(); i++) {
            double best = 1e9;
            for (size_t c = 0; c < centres.size(); c++) {
                double d = dist(points[i], centres[c]);
                if (d < best) best = d, owner[i] = (int)c;
            }
        }
        std::vector<Lab> sum(centres.size(), Lab{0, 0, 0});
        std::vector<int> count(centres.size(), 0);
        for (size_t i = 0; i < points.size(); i++) {
            Lab& s = sum[owner[i]];
            s.L += points[i].L, s.a += points[i].a, s.b += points[i].b;
            count[owner[i]]++;
        }
        for (size_t c = 0; c < centres.size(); c++)
            if (count[c]) centres[c] = {sum[c].L / count[c], sum[c].a / count[c], sum[c].b / count[c]};
        if (iteration == 9)
            for (size_t c = 0; c < centres.size(); c++)
                if (count[c]) out.push_back({centres[c], count[c] / double(points.size())});
    }
    std::sort(out.begin(), out.end(), [](Cluster const& x, Cluster const& y) { return x.weight > y.weight; });
    return out;
}

struct Tuning {
    double groundChroma;           // most a ground may be tinted
    double accentMin, accentMax;   // accent chroma
    double accentScale;            // of the picture's chroma
    double accentDark, accentLight;  // accent lightness on dark and light grounds
};

Tuning TuningFor(Profile profile) {
    switch (profile) {
        case Profile::Monochrome: return {0, 0, 0, 0, 0.86, 0.40};
        case Profile::Muted: return {0.015, 0.04, 0.09, 0.6, 0.74, 0.50};
        case Profile::Vivid: return {0.045, 0.12, 0.26, 1.3, 0.74, 0.55};
        case Profile::Pastel: return {0.025, 0.05, 0.10, 0.8, 0.84, 0.62};
        default: return {0.03, 0.06, 0.18, 1.0, 0.74, 0.52};
    }
}

// Moves a colour's lightness away from the ground until it has the contrast
// asked for (or can go no further).
uint32_t WithContrast(double L, double C, double h, uint32_t ground, double ratio, bool light) {
    uint32_t c = FromLch(L, C, h);
    for (int i = 0; i < 100 && Contrast(c, ground) < ratio; i++) {
        L += light ? -0.01 : 0.01;
        if (L < 0 || L > 1) break;
        c = FromLch(L, C, h);
    }
    return c;
}

// The roles from what the picture gave: its main colour (the grounds' tint)
// and its accent colours.
Roles Build(Lab main, double mainWeightL, Lab accent, Lab accent2, bool grey, const PaletteRequest& r) {
    Tuning t = TuningFor(r.profile);
    bool light = r.light;
    double h0 = Hue(main), ha = Hue(accent), ha2 = Hue(accent2);
    double gc = std::min(Chroma(main) * 0.35, t.groundChroma);
    double ac = grey && r.profile != Profile::Vivid ? 0 : std::clamp(Chroma(accent) * t.accentScale, t.accentMin, t.accentMax);
    double ac2 = grey && r.profile != Profile::Vivid ? 0 : std::clamp(Chroma(accent2) * t.accentScale, t.accentMin, t.accentMax);
    if (r.profile == Profile::Monochrome) {
        // One tint (the main colour's, faint) through every role: a grey
        // seed gives pure greys, a coloured one a tinted monochrome.
        gc = std::min(Chroma(main) * 0.35, 0.012);
        ac = ac2 = std::min(Chroma(main) * 0.5, 0.03);
        ha = ha2 = h0;
    }
    // Dark grounds follow the picture a little (a bright picture, a lighter
    // ground), light ones stay near white.
    double g = light ? 0.97 : std::clamp(mainWeightL * 0.35 + 0.10, 0.15, 0.27);
    double step = light ? -0.04 : 0.05;
    Roles out{};
    out.ground = FromLch(g, gc, h0);
    out.raised = FromLch(g + step, gc, h0);
    out.line = FromLch(g + step * 2, gc, h0);
    out.edge = WithContrast(g + step * 4, gc, h0, out.ground, 1.5, light);
    out.text = WithContrast(light ? 0.22 : 0.93, gc * 0.5, h0, out.ground, std::max(7.0, r.textContrast), light);
    out.muted = WithContrast(light ? 0.45 : 0.72, gc * 0.7, h0, out.ground, r.textContrast, light);
    out.dim = WithContrast(light ? 0.62 : 0.52, gc * 0.7, h0, out.ground, 2.2, light);
    out.accent = WithContrast(light ? t.accentLight : t.accentDark, ac, ha, out.ground, std::max(3.0, r.textContrast), light);
    out.accent2 = WithContrast(light ? t.accentLight : t.accentDark, ac2, ha2, out.ground, 3.0, light);
    bool mono = r.profile == Profile::Monochrome;
    double sc = mono ? 0.06 : 0.14;
    out.warn = WithContrast(light ? 0.55 : 0.80, sc, 75, out.ground, 3.0, light);
    out.bad = WithContrast(light ? 0.52 : 0.68, mono ? 0.08 : 0.17, 25, out.ground, 3.0, light);
    out.good = WithContrast(light ? 0.50 : 0.76, sc, 145, out.ground, 3.0, light);
    return out;
}

}  // namespace

double Contrast(uint32_t a, uint32_t b) {
    double x = Luminance(a), y = Luminance(b);
    if (x < y) std::swap(x, y);
    return (x + 0.05) / (y + 0.05);
}

Roles PaletteFromPixels(const std::vector<uint32_t>& pixels, const PaletteRequest& request) {
    auto clusters = Clusters(pixels, 8);
    if (clusters.empty()) return PaletteFromSeed(0x808080, request);
    Cluster const& main = clusters[0];
    // The accent: the most colourful part that's more than a speck.
    auto score = [](Cluster const& c) { return Chroma(c.centre) * std::sqrt(c.weight); };
    Cluster const* accent = &clusters[0];
    for (auto const& c : clusters)
        if (c.weight >= 0.02 && score(c) > score(*accent)) accent = &c;
    // A mostly grey picture with one strong colour in it (a neon sign, a
    // red window): that colour, even if small.
    if (Chroma(accent->centre) < 0.035)
        for (auto const& c : clusters)
            if (c.weight >= 0.004 && Chroma(c.centre) >= 0.08 && Chroma(c.centre) > Chroma(accent->centre)) accent = &c;
    // A second accent of another hue, if the picture has one.
    Cluster const* accent2 = nullptr;
    for (auto const& c : clusters)
        if (&c != accent && c.weight >= 0.02 && Chroma(c.centre) > 0.03 && HueDistance(Hue(c.centre), Hue(accent->centre)) >= 35 &&
            (!accent2 || score(c) > score(*accent2)))
            accent2 = &c;
    Lab second = accent->centre;
    if (accent2) {
        second = accent2->centre;
    } else {  // none: the accent's hue turned a little
        double h = (Hue(second) + 50) * kPi / 180, c = Chroma(second);
        second = {second.L, c * std::cos(h), c * std::sin(h)};
    }
    bool grey = Chroma(accent->centre) < 0.035;
    double meanL = 0;
    for (auto const& c : clusters) meanL += c.centre.L * c.weight;
    return Build(main.centre, meanL, accent->centre, second, grey, request);
}

Roles PaletteFromSeed(uint32_t seed, const PaletteRequest& request) {
    Lab c = ToLab(seed & 0xFFFFFF);
    double h = (Hue(c) + 50) * kPi / 180, ch = Chroma(c);
    Lab second{c.L, ch * std::cos(h), ch * std::sin(h)};
    return Build(c, 0.3, c, second, ch < 0.035, request);
}

bool ParseProfile(const std::wstring& name, Profile* out) {
    struct {
        const wchar_t* name;
        Profile profile;
    } const kNames[] = {{L"adaptive", Profile::Adaptive}, {L"monochrome", Profile::Monochrome}, {L"muted", Profile::Muted},
                        {L"vivid", Profile::Vivid},       {L"pastel", Profile::Pastel}};
    for (auto const& n : kNames)
        if (name == n.name) return *out = n.profile, true;
    return false;
}

}  // namespace td
