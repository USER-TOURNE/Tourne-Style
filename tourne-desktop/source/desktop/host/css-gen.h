// Palette CSS: the theme's colours written into style sheets for apps made of
// web pages (Discord, Claude, Spotify, browsers' pages...), ready for the CSS
// injector. Plain C++.
//
// Templates name colours as {{role}} (#rrggbb), {{role.rgb}} (r, g, b),
// {{role.hsl}} (h s% l%) or {{role.hex}} (rrggbb), with the roles of
// palette-gen.h: ground raised line edge text accent muted dim warn bad
// accent2 good.
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
std::string AppCss(const std::string& app, const Roles& roles);

}  // namespace td
