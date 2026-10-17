# Tourne Desktop's built-in presets

`catalogue.json` holds the 824 presets Tourne Studio's Presets page shows: every
palette and look from the October 2026 research catalogue that has all twelve
of the Tourne theme's colour roles, each set of colours kept once.
`desktop/tools/make-presets.py` turns it into `desktop/host/presets-data.inc`:

    python3 desktop/tools/make-presets.py                 # from catalogue.json
    python3 desktop/tools/make-presets.py presets.json    # from the full research file (rewrites catalogue.json)

## What's kept

Facts only: colours, a few sizes, names, and where each came from
(`source`, `source_url`, `licence`). No theme files, configs or pictures.
A palette is a list of colours, which is a fact; the names and links credit
the people who chose them.

Each entry has the twelve roles (`ground`, `raised`, `line`, `edge`, `text`,
`muted`, `dim`, `accent`, `accent2`, `good`, `warn`, `bad`). Where a source
gave fewer (base16 slots, terminal colours), the research mapped or blended
the rest; the full research file lists every value that was worked out.

Full looks (127) also carry the settings Tourne has a home for: tiling gaps
(`switcher.tiling.gap*`), window borders (`switcher.borders.width`,
`activeColor`, `inactiveColor`, corners) and corner radii
(`switcher.corners.radius`, `glass.radius`).

## Where they came from

| Presets | Source | Licence |
|---:|---|---|
| 273 | tinted-theming/schemes (base16) | MIT, each scheme by its author |
| 156 | tinted-theming/schemes (base24) | MIT, each scheme by its author |
| 143 | alacritty/alacritty-theme | Apache-2.0 |
| 90 | Omarchy community themes | various; some not stated (colours and names only) |
| 64 | HyDE gallery and official themes | GPL-3.0 or not stated (colours only) |
| 28 | HyprPanel themes | MIT |
| 18 | gh0stzk dotfiles (BSPWM rices) | GPL-3.0 (colours and sizes only) |
| 17 | Omarchy built-in themes | MIT |
| 15 | My picks (the research's recommended one-click looks) | see each component |
| 9 | YASB themes gallery | MIT gallery; themes their own |
| 4 | JaKooLit Hyprland-Dots Waybar styles | GPL-3.0 (colours only) |
| 3 | Tokyo Night (official extras) | Apache-2.0 |
| 2 | Catppuccin (official palette) | MIT |
| 1 | windots | MIT |
| 1 | Rosé Pine (official palette) | MIT |

## Left out

- **Material You baseline** (dark and light): written from memory of the
  Material spec, not read from it. Material You comes from the palette maker
  instead.
- **niivu's visual styles**: no colours were stated, and they're
  no-derivatives (CC BY-NC-ND), mostly for subscribers. Names and links at most.
- **Windhawk taskbar and Start styles, YASB and Waybar bar layouts** without
  a full palette: kept in the research file for the sets step, where a bar's
  layout and a Start menu's style become settings.
- Reddit couldn't be read from the build server; GitHub repositories tagged
  unixporn stood in.
