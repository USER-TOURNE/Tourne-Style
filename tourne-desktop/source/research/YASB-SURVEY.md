# YASB theme survey and CSS spike (2026-09-29)

Source: all 68 themes in `amnweb/yasb-themes` (each has `styles.css`, `config.yaml`, `readme.md`, `image.png`, a `.json`), saved under `research/yasb-themes/` for testing only (not for redistribution: they belong to their authors).

## What themes use

- **Size.** 2.6 KB to 51 KB of CSS, 13 KB average, 876 KB in all.
- **Custom properties are the norm.** 53 of 68 use `:root { --name: ... }` and `var()` (4,407 uses). 3 also use YASB's generated `--yasb-*` colours.
- **Values.** `rgba()` 1,667 uses, `rgb()` 345, hex everywhere, `px` only (plus `s` in transitions).
- **Properties by use:** color 2,997; background-color 2,618; font-size 2,090; padding 1,644; border-radius 1,304; font-weight 1,114; border 1,027; margin 991; font-family 979; background 415; min/max-width; height; outline; transition 59; cursor 33; opacity 20.
- **Rare:** gradients (2 themes: `qlineargradient`, `linear-gradient`), `box-shadow` (2 themes, 12 uses), `-qproperty-*` (2, the adaptive bar), `@import` (3), `!important` (6), `url()` (1). `transition` is in 11 themes (`background-color 0.25s ease`, `all 0.2s ease`, `background-color 0.08s, opacity 0.08s`).
- **Pseudo-classes:** `:hover` 850, `:selected` 87, `:disabled` 67, `:focus` 56, `:pressed` 49, `:checked` 42, `:unchecked` 26, `:clicked` 24, `:active` 3.
- **Pseudo-elements (Qt sub-controls): 420 selectors** (`::item`, `::groove`, `::handle`, `::chunk`, `::indicator`, `::sub-page`): sliders, scroll bars and menus inside **popups**, not the bar.
- **Fonts:** Segoe UI first, then Nerd Fonts (JetBrainsMono NFP, 0xProto Nerd Font), Segoe Fluent Icons, Material Symbols. Icons are glyphs in text.
- **Popups are about 60% of the CSS.** Only 39% of rules (2,712 of 6,908) are about the bar itself. The bar rules use color, padding, font-size, background-color, margin, border-radius, border, font-weight, font-family, width/height, transition, `backdrop-filter` (10 uses, blur), `box-shadow` (8).

## What the configs use

- 53 of 68 turn on the acrylic blur (`blur_effect`). 68 use `watch_stylesheet`. 2 use the adaptive style.
- Widgets by number of themes: clock 68, volume 65, media 58, power menu 56, komorebi workspaces 51, weather 48, notifications 44, home 40, active window 37, memory 31, cpu 30, custom 29, systray 27, wifi 22, applications 22, wallpapers 22, komorebi layout 21, **taskbar 20**, traffic 19, disk 17, battery 17.
- YASB themes are status bars. Only 20 of 68 show taskbar buttons.

## What the spike does

`css-engine.cpp` (parser, selectors, cascade, var(), shorthands, gradients, transitions read), `css-draw.cpp` (rounded boxes, borders, gradients, text with icon-font fallback, icons; software, GDI only), `yaml-lite.cpp` (config.yaml) and `tools/css-spike/cssspike.cpp` (element tree like YASB's, layout, picture).

Survey of the 68 themes (`cssspike --survey`):
- All 68 configs read; all 68 layouts built.
- 6,908 rules: 6,551 kept, 360 skipped or partly skipped (almost all the pseudo-element ones).
- 19,445 declarations: 19,256 recognised (99%). Ignored, by use: `background` values we can't read (23), `display` 17, `backdrop-filter` 17, `align-items` 15, `box-shadow` 12, `text-transform` 11, `justify-content` 10, `text-align` 9, `qproperty-text` 9, `transform` 4.
- 10 of 68 themes have nothing skipped or ignored.
- Widgets: 433 built with their own shape (clock, volume, weather, power menu, workspaces, layout, applications, taskbar, active window, custom), 329 generic (home, notifications, systray, memory, cpu, wifi, battery, disk, traffic and others).

Drawing: six themes rendered from their own `styles.css` and `config.yaml` (images 24 and 25 in the GitHub folder). Layout, colours, radii, borders and spacing match the themes' own screenshots closely. Known differences: fonts, acrylic blur, rounded window corners, real widget data.

## Findings that shape the plan

1. Variables (`:root` and `var()`) are essential, 78% of themes.
2. Bar-only scope is small: popups (calendar, media, control centre, power menu) need a real popup system; skip them at first.
3. Gradients, shadows and transitions are rare, so they come after the basics. Transitions are wanted for `background-color` and `opacity`, at 0.08 to 0.25 s.
4. Icon fonts decide the look. Themes rely on Nerd Font glyphs; advance widths differ per font, so spacing differs unless the theme's own font is installed. Ship or ask for the font.
5. Widgets are the real work: 40 widget kinds with data (weather, media, cpu, memory, wifi, battery...). Start with the ones our suite already knows.
6. In this environment Windhawk's own text mods swap fonts in every process, so the spike's text renders in a pixel font. That is our environment, not the engine.
