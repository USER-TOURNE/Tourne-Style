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
}

::selection { background: {{accent}}; color: {{ground}}; }
::-webkit-scrollbar { width: 10px; height: 10px; background: transparent; }
::-webkit-scrollbar-corner { background: transparent; }
::-webkit-scrollbar-thumb { background: {{line}}; border-radius: 5px; border: 2px solid transparent; background-clip: padding-box; }
::-webkit-scrollbar-thumb:hover { background: {{edge}}; border: 2px solid transparent; background-clip: padding-box; }
)css";

// Discord's own colour variables, both the older names and those of its
// 2025 look. A starting point: Discord renames them now and then.
constexpr const char kDiscord[] = R"css(
.theme-dark, .theme-light, .theme-darker, .theme-midnight, .visual-refresh {
  --background-primary: {{ground}} !important;
  --background-secondary: {{raised}} !important;
  --background-secondary-alt: {{line}} !important;
  --background-tertiary: {{raised}} !important;
  --background-floating: {{raised}} !important;
  --background-accent: {{edge}} !important;
  --background-base-lowest: {{ground}} !important;
  --background-base-lower: {{ground}} !important;
  --background-base-low: {{raised}} !important;
  --background-surface-high: {{raised}} !important;
  --background-surface-higher: {{line}} !important;
  --background-surface-highest: {{line}} !important;
  --background-modifier-hover: rgba({{line.rgb}}, 0.6) !important;
  --background-modifier-active: rgba({{edge.rgb}}, 0.6) !important;
  --background-modifier-selected: rgba({{edge.rgb}}, 0.7) !important;
  --chat-background-default: {{ground}} !important;
  --channeltextarea-background: {{raised}} !important;
  --modal-background: {{raised}} !important;
  --input-background: {{raised}} !important;
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
  --background-base: {{ground}} !important;
  --background-highlight: {{raised}} !important;
  --background-press: {{line}} !important;
  --background-elevated-base: {{raised}} !important;
  --background-elevated-highlight: {{line}} !important;
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
        std::string name = text.substr(open + 2, close - open - 2), form;
        size_t dot = name.find('.');
        if (dot != std::string::npos) form = name.substr(dot + 1), name.resize(dot);
        bool known = false;
        for (auto const& r : kRoles)
            if (name == r.name) {
                out += Format(roles.*r.role, form);
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

std::string AppCss(const std::string& app, const Roles& roles) {
    std::string text = std::string(kHeader) + kGeneric;
    if (app == "discord") text += kDiscord;
    else if (app == "claude") text += kClaude;
    else if (app == "spotify") text += kSpotify;
    return FillTemplate(text, roles);
}

}  // namespace td
