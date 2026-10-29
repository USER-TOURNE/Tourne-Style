// A role's steps (palette-gen.cpp's StepsFor), printed and checked without a
// screen: hover, pressed, disabled, a line around it and what goes on top,
// for every role of a palette.
//
//   g++ -std=c++23 -I../../desktop/host -o palette-steps-test palette-steps-test.cpp
//   (or x86_64-w64-mingw32-g++, the same flags, for the PC)
//
// With no arguments it runs over Tourne's own palette, a light palette and a
// palette made from a seed colour. An argument of six hex digits prints the
// steps for that one colour on Tourne's ground.
#include "palette-gen.cpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

int g_failures;

// How much colour is in it, by the same measure the palette uses (OKLab
// chroma), so a step that washes a role out is visible here.
double Saturation(uint32_t c) {
    td::Lab lab = td::ToLab(c & 0xFFFFFF);
    return td::Chroma(lab);
}

struct NamedRole {
    const char* name;
    uint32_t td::Roles::*at;
};

constexpr NamedRole kRoles[] = {{"ground", &td::Roles::ground}, {"raised", &td::Roles::raised},
                                {"line", &td::Roles::line},     {"edge", &td::Roles::edge},
                                {"text", &td::Roles::text},     {"muted", &td::Roles::muted},
                                {"dim", &td::Roles::dim},       {"accent", &td::Roles::accent},
                                {"accent2", &td::Roles::accent2}, {"warn", &td::Roles::warn},
                                {"bad", &td::Roles::bad},       {"good", &td::Roles::good}};

void Fail(const char* set, const char* role, const char* what) {
    printf("  FAIL %s %s: %s\n", set, role, what);
    g_failures++;
}

// Tourne's own colours (knowledge/tourne-palette.json), as a palette to step.
td::Roles TournesOwn() {
    td::Roles r{};
    r.ground = 0x121c21, r.raised = 0x1e2e36, r.line = 0x10171d, r.edge = 0x2a3f49;
    r.text = 0x15ffa2, r.accent = 0x15ffa2, r.muted = 0x64aa89, r.dim = 0x5d8d6f;
    r.accent2 = 0x66ea97, r.warn = 0xffd48f, r.bad = 0xff8f8f, r.good = 0x66ea97;
    return r;
}

void Row(const char* name, uint32_t role, uint32_t ground, bool light) {
    td::Steps s = td::StepsFor(role, ground, light);
    printf("  %-8s %06x  hover %06x  press %06x  off %06x  edge %06x  on %06x   (%.1f:1 on the ground)\n", name, role,
           s.hover, s.press, s.off, s.edge, s.on, td::Contrast(role, ground));
}

void Check(const char* set, td::Roles const& roles, bool light) {
    printf("%s\n", set);
    for (NamedRole const& r : kRoles) {
        uint32_t role = roles.*r.at;
        td::Steps s = td::StepsFor(role, roles.ground, light);
        Row(r.name, role, roles.ground, light);
        if (s.hover == role) Fail(set, r.name, "hover is the same colour as the role");
        if (s.press == s.hover) Fail(set, r.name, "pressed is the same colour as hover");
        // Pressed is one step further than hover, whichever way the role went.
        if (td::Contrast(s.press, role) <= td::Contrast(s.hover, role))
            Fail(set, r.name, "pressed is no further from the role than hover");
        // Neither step throws the colour away (a role with colour in it keeps
        // most of it; a grey one has none to keep).
        double was = Saturation(role);
        if (was > 0.02 && (Saturation(s.hover) < was * 0.65 || Saturation(s.press) < was * 0.6))
            Fail(set, r.name, "the steps wash the colour out");
        if (r.name != std::string("ground") && td::Contrast(s.off, roles.ground) > td::Contrast(role, roles.ground))
            Fail(set, r.name, "disabled is further from the ground than the role");
        if (td::Contrast(s.edge, role) < 1.35) Fail(set, r.name, "a line around it wouldn't be seen");
        if (td::Contrast(s.on, role) < 3.0) Fail(set, r.name, "what goes on top of it wouldn't be readable");
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 1) {
        uint32_t one = (uint32_t)strtoul(argv[1], nullptr, 16) & 0xFFFFFF;
        printf("one colour on Tourne's ground\n");
        Row("yours", one, TournesOwn().ground, false);
        return 0;
    }
    Check("Tourne's own palette", TournesOwn(), false);

    td::PaletteRequest dark;
    Check("made from a teal seed", td::PaletteFromSeed(0x15ffa2, dark), false);

    td::PaletteRequest lightRequest;
    lightRequest.light = true;
    Check("a light palette from the same seed", td::PaletteFromSeed(0x15ffa2, lightRequest), true);

    printf("\n%s\n", g_failures ? "failures above" : "every role's steps check out");
    return g_failures ? 1 : 0;
}
