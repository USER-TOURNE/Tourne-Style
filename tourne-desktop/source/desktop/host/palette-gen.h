// Tourne's palette maker: a wallpaper (or one seed colour) in, the suite's
// colour roles out. Plain C++ with no Windows in it, so it can be tested
// anywhere.
//
// The picture's colours are grouped in OKLab (where equal steps look equal to
// the eye), then each role is given a lightness of its own, so a palette made
// from any picture reads like a hand-made scheme: dark grounds, readable text,
// one accent taken from the picture. Every text colour is then checked against
// its ground and moved until it passes the contrast asked for.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace td {

// The suite's colour roles (palettes.cpp), plus the two the theme contract
// adds. Colours are 0xRRGGBB.
struct Roles {
    uint32_t ground, raised, line, edge, text, accent, muted, dim, warn, bad;
    uint32_t accent2, good;
};

enum class Profile { Adaptive, Monochrome, Muted, Vivid, Pastel };

struct PaletteRequest {
    Profile profile = Profile::Adaptive;
    bool light = false;           // light grounds, dark text
    double textContrast = 4.5;    // WCAG: 4.5 is AA, 7 is AAA
};

// pixels: 0xRRGGBB, any order (a picture already made small: a few thousand).
Roles PaletteFromPixels(const std::vector<uint32_t>& pixels, const PaletteRequest& request);
Roles PaletteFromSeed(uint32_t seed, const PaletteRequest& request);

bool ParseProfile(const std::wstring& name, Profile* out);

// WCAG contrast ratio between two colours (1 to 21).
double Contrast(uint32_t a, uint32_t b);

}  // namespace td
