// Palette CSS: the theme's colours written into style sheets for apps made of
// web pages (Discord, Claude, Spotify, browsers' pages...), ready for the CSS
// injector. Plain C++.
//
// Templates name colours as {{role}} (#rrggbb), {{role.rgb}} (r, g, b),
// {{role.hsl}} (h s% l%) or {{role.hex}} (rrggbb), with the roles of
// palette-gen.h: ground raised line edge text accent muted dim warn bad
// accent2 good.
//
// A role can carry one of the states palette-gen's StepsFor works out, as
// {{role-state}} (and {{role-state.rgb}} and so on): hover, press, off (the
// disabled shade), edge (a line around it) and on (what goes on top of it). So
// a hover shade comes out of the palette rather than being written by hand per
// app.
#pragma once

#include <string>
#include <vector>

#include "palette-gen.h"

namespace td {

std::string FillTemplate(const std::string& text, const Roles& roles);

// The apps Tourne writes a starter style for, and that style ("generic" for
// any other app: the colours as --tourne-* variables, scrollbars and
// selection).
const std::vector<std::string>& StyledApps();
// The glass (Studio, Glass) and the menus' frame, which the sheets follow too: pages and panels see-through by
// the tint's strength, floating parts (menus, popups, tooltips, dialogs) frosted, every corner square.
struct CssGlass {
    bool on = false;
    int blur = 0, saturation = 100, brightness = 0, tintAmount = 35, shadow = 50;
    uint32_t tint = 0x0B0D10, frame = 0x4F8F6A;
};
std::string AppCss(const std::string& app, const Roles& roles, const CssGlass& glass);

}  // namespace td
