#include "css-gen.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace td {

namespace {

struct Named {
    const char* name;
    uint32_t Roles::*role;
};

constexpr Named kRoles[] = {{"ground", &Roles::ground}, {"raised", &Roles::raised}, {"line", &Roles::line},
                            {"edge", &Roles::edge},     {"text", &Roles::text},     {"accent2", &Roles::accent2},
                            {"accent", &Roles::accent}, {"muted", &Roles::muted},   {"dim", &Roles::dim},
                            {"warn", &Roles::warn},     {"bad", &Roles::bad},       {"good", &Roles::good}};

// A role's name may carry a state: "accent-hover". Takes it off the name and
// gives the colour that state asks for, or the role itself.
uint32_t WithState(uint32_t role, const Roles& roles, const std::string& state) {
    if (state.empty()) return role;
    bool light = Contrast(roles.ground, 0xFFFFFF) < Contrast(roles.ground, 0x000000);
    Steps s = StepsFor(role, roles.ground, light);
    if (state == "hover") return s.hover;
    if (state == "press") return s.press;
    if (state == "off") return s.off;
    if (state == "edge") return s.edge;
    if (state == "on") return s.on;
    return role;
}

bool IsState(const std::string& name) {
    return name == "hover" || name == "press" || name == "off" || name == "edge" || name == "on";
}

std::string Format(uint32_t c, const std::string& form) {
    int r = c >> 16 & 0xFF, g = c >> 8 & 0xFF, b = c & 0xFF;
    char text[48];
    if (form == "rgb") {
        snprintf(text, sizeof(text), "%d, %d, %d", r, g, b);
    } else if (form == "hex") {
        snprintf(text, sizeof(text), "%06x", c & 0xFFFFFF);
    } else if (form == "hsl") {
        double R = r / 255.0, G = g / 255.0, B = b / 255.0, hi = std::max({R, G, B}), lo = std::min({R, G, B});
        double l = (hi + lo) / 2, d = hi - lo, h = 0, s = 0;
        if (d > 1e-9) {
            s = d / (1 - std::fabs(2 * l - 1));
            h = hi == R ? std::fmod((G - B) / d, 6.0) : hi == G ? (B - R) / d + 2 : (R - G) / d + 4;
            h *= 60;
            if (h < 0) h += 360;
        }
        snprintf(text, sizeof(text), "%.0f %.1f%% %.1f%%", h, s * 100, l * 100);
    } else {
        snprintf(text, sizeof(text), "#%06x", c & 0xFFFFFF);
    }
    return text;
}

constexpr const char kHeader[] =
    "/* Made by Tourne from your theme, and rewritten each time the theme changes:\n"
    "   to change it, copy it next to the tourne folder and use your copy instead. */\n\n";

constexpr const char kGeneric[] = R"css(:root {
  --tourne-ground: {{ground}};
  --tourne-raised: {{raised}};
  --tourne-line: {{line}};
  --tourne-edge: {{edge}};
  --tourne-text: {{text}};
  --tourne-muted: {{muted}};
  --tourne-dim: {{dim}};
  --tourne-accent: {{accent}};
  --tourne-accent2: {{accent2}};
  --tourne-warn: {{warn}};
  --tourne-bad: {{bad}};
  --tourne-good: {{good}};
  /* The states, worked out from the roles above. */
  --tourne-hover: {{raised-hover}};
  --tourne-press: {{raised-press}};
  --tourne-disabled: {{text-off}};
  --tourne-accent-hover: {{accent-hover}};
  --tourne-accent-press: {{accent-press}};
  --tourne-accent-on: {{accent-on}};
  /* Tourne's menus: their frame line (where its colour gradient ends). */
  --tourne-frame: {{frame}};
  /* The glass (Studio, Glass): how see-through pages and panels are, so the app's Mica or Acrylic shows where the
     app lets it, and the frosted glass drawn on floating parts (menus, popups, tooltips, dialogs). */
  --tourne-glass-tint: {{glass.tint}};
  --tourne-glass-page: {{glass.page}};
  --tourne-glass-panel: {{glass.panel}};
  --tourne-glass-float: {{glass.float}};
  --tourne-glass-filter: {{glass.filter}};
  --tourne-glass-shadow: {{glass.shadow}};
}

/* The menus' look: square corners, thin lines, the accent for focus and selection. */
*, *::before, *::after { border-radius: 0 !important; }
::selection { background: var(--tourne-accent); color: var(--tourne-ground); }
:focus-visible { outline: 1px solid var(--tourne-accent) !important; outline-offset: 0 !important; }
html { background: var(--tourne-glass-page) !important; color: var(--tourne-text); }
body { background: transparent !important; }
hr { border: 0; border-top: 1px solid var(--tourne-line); }
::-webkit-scrollbar { width: 8px; height: 8px; background: transparent; }
::-webkit-scrollbar-corner { background: transparent; }
::-webkit-scrollbar-thumb { background: var(--tourne-line); border: 2px solid transparent; background-clip: padding-box; }
::-webkit-scrollbar-thumb:hover { background: var(--tourne-edge); border: 2px solid transparent; background-clip: padding-box; }

/* What floats (menus, popups, tooltips, dialogs) is a Tourne menu: frosted glass, the frame's line, its shadow. */
[role="menu"], [role="listbox"], [role="tooltip"], [role="dialog"], dialog, menu, select, datalist {
  background-color: var(--tourne-glass-float) !important;
  -webkit-backdrop-filter: var(--tourne-glass-filter) !important;
  backdrop-filter: var(--tourne-glass-filter) !important;
  border: 1px solid var(--tourne-frame) !important;
  box-shadow: var(--tourne-glass-shadow) !important;
  color: var(--tourne-text) !important;
}
[role="menuitem"]:hover, [role="menuitem"][aria-selected="true"], [role="menuitem"]:focus,
[role="option"]:hover, [role="option"][aria-selected="true"], [role="option"]:focus {
  background-color: var(--tourne-hover) !important;
  color: var(--tourne-text) !important;
}
[role="separator"] { background-color: var(--tourne-line) !important; }
input, textarea, button { border-radius: 0 !important; }
input:focus, textarea:focus { border-color: var(--tourne-edge) !important; }
)css";

// Discord's own colour variables, both the older names and those of its
// 2025 look. A starting point: Discord renames them now and then.
constexpr const char kDiscord[] = R"css(
.theme-dark, .theme-light, .theme-darker, .theme-midnight, .visual-refresh {
  --background-primary: var(--tourne-glass-page) !important;
  --background-secondary: var(--tourne-glass-panel) !important;
  --background-secondary-alt: var(--tourne-glass-panel) !important;
  --background-tertiary: var(--tourne-glass-panel) !important;
  --background-floating: var(--tourne-glass-float) !important;
  --background-accent: {{edge}} !important;
  --background-base-lowest: var(--tourne-glass-page) !important;
  --background-base-lower: var(--tourne-glass-page) !important;
  --background-base-low: var(--tourne-glass-panel) !important;
  --background-surface-high: var(--tourne-glass-panel) !important;
  --background-surface-higher: var(--tourne-glass-float) !important;
  --background-surface-highest: var(--tourne-glass-float) !important;
  --background-modifier-hover: rgba({{line.rgb}}, 0.6) !important;
  --background-modifier-active: rgba({{edge.rgb}}, 0.6) !important;
  --background-modifier-selected: rgba({{edge.rgb}}, 0.7) !important;
  --chat-background-default: var(--tourne-glass-page) !important;
  --channeltextarea-background: var(--tourne-glass-panel) !important;
  --modal-background: var(--tourne-glass-float) !important;
  --input-background: var(--tourne-glass-panel) !important;
  --text-normal: {{text}} !important;
  --text-default: {{text}} !important;
  --text-strong: {{text}} !important;
  --header-primary: {{text}} !important;
  --header-secondary: {{muted}} !important;
  --text-muted: {{muted}} !important;
  --text-link: {{accent}} !important;
  --channels-default: {{muted}} !important;
  --interactive-normal: {{muted}} !important;
  --interactive-hover: {{text}} !important;
  --interactive-active: {{text}} !important;
  --interactive-muted: {{dim}} !important;
  --brand-500: {{accent}} !important;
  --brand-560: {{accent}} !important;
  --brand-experiment: {{accent}} !important;
  --button-filled-brand-background: {{accent}} !important;
  --border-subtle: {{line}} !important;
  --border-faint: {{line}} !important;
  --status-danger: {{bad}} !important;
  --status-warning: {{warn}} !important;
  --status-positive: {{good}} !important;
}
)css";

// Claude's app (as claude.ai): colours kept as "h s% l%" variables. A
// starting point: the names can change with an update.
constexpr const char kClaude[] = R"css(
:root, :root[data-mode], html.dark, .dark, [data-theme] {
  --bg-000: {{raised.hsl}} !important;
  --bg-100: {{ground.hsl}} !important;
  --bg-200: {{ground.hsl}} !important;
  --bg-300: {{line.hsl}} !important;
  --bg-400: {{edge.hsl}} !important;
  --bg-500: {{edge.hsl}} !important;
  --text-000: {{text.hsl}} !important;
  --text-100: {{text.hsl}} !important;
  --text-200: {{muted.hsl}} !important;
  --text-300: {{muted.hsl}} !important;
  --text-400: {{dim.hsl}} !important;
  --text-500: {{dim.hsl}} !important;
  --border-100: {{line.hsl}} !important;
  --border-200: {{line.hsl}} !important;
  --border-300: {{edge.hsl}} !important;
  --border-400: {{edge.hsl}} !important;
  --accent-main-000: {{accent.hsl}} !important;
  --accent-main-100: {{accent.hsl}} !important;
  --accent-main-200: {{accent.hsl}} !important;
  --accent-secondary-000: {{accent2.hsl}} !important;
  --accent-secondary-100: {{accent2.hsl}} !important;
  --accent-secondary-200: {{accent2.hsl}} !important;
  --danger-000: {{bad.hsl}} !important;
  --danger-100: {{bad.hsl}} !important;
  --danger-200: {{bad.hsl}} !important;
}
)css";

// Spotify's desktop app (its Encore colour variables).
constexpr const char kSpotify[] = R"css(
:root, .encore-dark-theme, .encore-base-set, .encore-dark-theme .encore-base-set {
  --background-base: var(--tourne-glass-page) !important;
  --background-highlight: var(--tourne-hover) !important;
  --background-press: var(--tourne-press) !important;
  --background-elevated-base: var(--tourne-glass-panel) !important;
  --background-elevated-highlight: var(--tourne-hover) !important;
  --background-tinted-base: rgba({{text.rgb}}, 0.07) !important;
  --background-tinted-highlight: rgba({{text.rgb}}, 0.1) !important;
  --text-base: {{text}} !important;
  --text-subdued: {{muted}} !important;
  --text-bright-accent: {{accent}} !important;
  --essential-base: {{text}} !important;
  --essential-subdued: {{dim}} !important;
  --essential-bright-accent: {{accent}} !important;
  --essential-negative: {{bad}} !important;
  --essential-warning: {{warn}} !important;
  --decorative-base: {{text}} !important;
  --decorative-subdued: {{line}} !important;
}
)css";

}  // namespace

std::string FillTemplate(const std::string& text, const Roles& roles) {
    std::string out;
    out.reserve(text.size() + 256);
    size_t at = 0;
    while (at < text.size()) {
        size_t open = text.find("{{", at);
        if (open == std::string::npos) break;
        size_t close = text.find("}}", open + 2);
        if (close == std::string::npos) break;
        out.append(text, at, open - at);
        std::string name = text.substr(open + 2, close - open - 2), form, state;
        size_t dot = name.find('.');
        if (dot != std::string::npos) form = name.substr(dot + 1), name.resize(dot);
        size_t dash = name.rfind('-');
        if (dash != std::string::npos && IsState(name.substr(dash + 1)))
            state = name.substr(dash + 1), name.resize(dash);
        bool known = false;
        for (auto const& r : kRoles)
            if (name == r.name) {
                out += Format(WithState(roles.*r.role, roles, state), form);
                known = true;
                break;
            }
        if (!known) out.append(text, open, close + 2 - open);  // left for the reader to see
        at = close + 2;
    }
    out.append(text, at, std::string::npos);
    return out;
}

const std::vector<std::string>& StyledApps() {
    static const std::vector<std::string> apps{"generic", "discord", "claude", "spotify"};
    return apps;
}

std::string AppCss(const std::string& app, const Roles& roles, const CssGlass& glass) {
    std::string text = std::string(kHeader) + kGeneric;
    if (app == "discord") text += kDiscord;
    else if (app == "claude") text += kClaude;
    else if (app == "spotify") text += kSpotify;
    text = FillTemplate(text, roles);
    // The glass: pages and panels see-through by the tint's strength (more tint, more solid; glass off, solid),
    // floating parts frosted with the glass's blur, saturation and brightness.
    auto rgba = [](uint32_t c, double a) {
        char b[64];
        snprintf(b, sizeof(b), "rgba(%d, %d, %d, %.2f)", (int)(c >> 16 & 0xFF), (int)(c >> 8 & 0xFF), (int)(c & 0xFF), a);
        return std::string(b);
    };
    double t = std::clamp(glass.tintAmount / 100.0, 0.0, 1.0);
    double page = glass.on ? std::clamp(0.55 + t * 0.4, 0.55, 0.95) : 1.0, panel = glass.on ? std::min(1.0, page + 0.05) : 1.0;
    double floating = glass.on ? std::clamp(0.62 + t * 0.3, 0.62, 0.92) : 1.0;
    char filter[96];
    snprintf(filter, sizeof(filter), "blur(%dpx) saturate(%d%%) brightness(%d%%)", glass.on ? std::max(8, glass.blur * 2 + 12) : 0,
             glass.saturation, 100 + glass.brightness);
    char shadow[64];
    snprintf(shadow, sizeof(shadow), "0 6px 22px rgba(0, 0, 0, %.2f)", std::clamp(glass.shadow / 100.0, 0.0, 1.0) * 0.6);
    const std::pair<const char*, std::string> values[] = {
        {"{{glass.tint}}", Format(glass.tint, "")},
        {"{{glass.page}}", rgba(roles.ground, page)},
        {"{{glass.panel}}", rgba(roles.raised, panel)},
        {"{{glass.float}}", rgba(glass.tint, floating)},
        {"{{glass.filter}}", glass.on ? filter : "none"},
        {"{{glass.shadow}}", shadow},
        {"{{frame}}", Format(glass.frame, "")},
    };
    for (auto const& [key, value] : values)
        for (size_t at; (at = text.find(key)) != std::string::npos;) text.replace(at, strlen(key), value);
    return text;
}

}  // namespace td
