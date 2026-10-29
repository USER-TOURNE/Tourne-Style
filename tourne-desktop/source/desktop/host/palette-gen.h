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

// The same colour with its lightness moved by steps: one step is 0.05 in OKLab
// lightness, about the smallest shade the eye reads as a change, and the same
// step looks the same whatever the hue. Positive is lighter. Hue is kept and
// chroma is lowered only as far as the screen needs.
uint32_t Stepped(uint32_t colour, int steps);

// Two colours mixed in OKLab (0 is all a, 1 is all b), so a mix of two roles
// lands where the eye expects it rather than halfway in sRGB.
uint32_t Mixed(uint32_t a, uint32_t b, double towardsB);

// A role at the lightnesses the states drawn from it want, so hover, pressed,
// disabled and borders come out of the palette instead of being written by hand
// per app. Away from the ground means lighter on a dark theme and darker on a
// light one.
struct Steps {
    uint32_t hover;  // the pointer over it: one step away from the ground
    uint32_t press;  // held down: two steps
    uint32_t off;    // disabled: most of the way to the ground, with half the colour
    uint32_t edge;   // a line around it: far enough from it to be seen
    uint32_t on;     // text or an icon on top of it
};
Steps StepsFor(uint32_t role, uint32_t ground, bool light);

}  // namespace td
