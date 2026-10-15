// Tourne's built-in presets: palettes and looks gathered from the ricing
// world (base16 and base24 schemes, terminal themes, Omarchy, HyDE, JaKooLit,
// YASB, HyprPanel, Windhawk styles and more), kept as facts only: colours,
// sizes and where they came from. See desktop/presets/README.md.
//
// A preset's palette becomes the Tourne theme (theme.source = preset), so
// every piece set to "Tourne theme" and every colour set to a theme role
// follows it. A full look also sets the tiling gaps, window borders and
// corners it states. An identity (family "Identity", listed first) sets a
// whole look: its palette (or the seed it's made from), shape, material and
// motion.
#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "palette-gen.h"

namespace td {

struct Preset {
    const wchar_t* id;
    const wchar_t* name;
    const wchar_t* family;
    const wchar_t* source;
    bool light;
    int pick;             // a one-click pick's rank (1 first); 0: not one
    uint32_t roles[12];   // Roles' order: ground, raised, line, edge, text, accent, muted, dim, warn, bad, accent2, good
    const wchar_t* settings;  // "key=value\n" lines

    Roles AsRoles() const;
    std::vector<std::pair<std::wstring, std::wstring>> Settings() const;
};

const std::vector<Preset>& Presets();
const Preset* FindPreset(const std::wstring& id);

// What choosing a preset sets: the theme from it, the pieces following the
// theme, and a full look's own settings.
std::vector<std::pair<std::wstring, std::wstring>> PresetChoice(const Preset& p);

}  // namespace td
