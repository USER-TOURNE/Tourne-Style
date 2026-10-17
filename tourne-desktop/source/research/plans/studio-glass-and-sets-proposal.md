# TourneSuite one-click setups, glass and Studio proposal

Written 2026-10-04. Proposal only: no code until I say go.

My ask (2026-10-04):

- Make the looks in the uploaded clips and the "12 themes one shell" video into **one-click setups**.
- Auto palettes like omarchy-auto-theme (YouTube HBVhvTdfqvY) and pywal.
- Settings like **hyprmod**: "its whole feature set if we can get it all to work in windows".
- Window manager presets: a **PrinceWM** style option, plus **komorebi** and **AltSnap** presets.
- Above all, "a super clean glass effect like that first video has", applied system wide from a GUI like hyprmod.

Stills from every clip are in `tourne-research/reference-visuals/one-click-setups/` (contact sheets).

## 1. What the references are (checked)

| Reference | What it is | Licence | How I may use it |
| --- | --- | --- | --- |
| Clip 1 (Omarchy "Glass Tuner") | A liquid glass look on every window: thick glass rim with edge bend, rainbow fringe, edge glow and top highlight; an animated "oil film" option; a "click glow". A tuner panel has saved looks, live sliders, "hold to compare: glass off", "Load Apple-ish starting point", a test window and next wallpaper. | Unknown (a Hyprland shader setup) | Ideas only |
| Clip 2 | A Caelestia-style shell: big clock, a glass lock screen, kitty terminals with rainbow glass edges, and a notification sidebar with quick toggles. | Caelestia is GPL | Ideas only (standing rule) |
| Clip 3 | **Windows 11**: a forest wallpaper, a glass terminal running fastfetch, a lyrics widget, a matrix rain panel, an audio visualiser and a big pixel clock. Proof that this look already works on Windows. | n/a | Look target |
| Clip 4 | A desktop pet ("Mochi") with a "Pocket": drop a file, link, image or text on it and it holds it. A shelf. | Unknown | Ideas only |
| 12 themes video | One shell in 12 looks: a launcher panel with a mascot, a wallpaper gallery picker, a themes list, a visualiser, a system monitor, quick notes, a calendar, a "newspaper" theme. | Unknown | Ideas only (handoff section 12 is the breakdown) |
| [omarchy-auto-theme](https://github.com/AccursedGalaxy/omarchy-auto-theme) | When the wallpaper changes, matugen makes a Material You palette and renders it into Omarchy's own theme files, so every app that already has a template reloads. Settings: prefer (darkness, lightness, saturation...), mode, rotation interval. No GUI. | Repo licence not checked; matugen is GPL (inferred) | Ideas only. The colour science it uses (Material You) is Google's [material-color-utilities](https://github.com/material-foundation/material-color-utilities), Apache 2.0, which has a C++ version Tourne can use with attribution |
| [pywal](https://github.com/dylanaraps/pywal) | Makes a 16-colour terminal palette from a wallpaper and writes templates for many apps. | MIT | Ideas, and its template list |
| [HyprMod](https://github.com/BlueManCZ/hyprmod) | A GTK settings app for Hyprland (details in section 3). **It does not make palettes.** Its colour support is colour and gradient pickers for Hyprland's own colour options, and the app itself follows the system accent colour. | GPL 3 | Ideas only |
| [hyprglass](https://github.com/hyprnux/hyprglass) | A Hyprland plugin for liquid glass: blur, edge refraction, chromatic aberration, fresnel edge glow, specular highlight, bevel, tint, lens dome, presets, dark and light. | Not stated in the README | Ideas, and its option names as a vocabulary |
| [PrinceWM](https://github.com/Ravestars/PrinceWM) | An **Alt+Tab replacement**, not a tiler: every window as a live preview on an infinite canvas. It has real most-recently-used order, app stacks you drill into, a pinboard (paste images and notes), paint mode, live style editing, and drag-to-tile two windows. C#, .NET 9, DirectX 11; it needs admin rights because of a global keyboard hook. | MIT | Ideas freely; code could be borrowed with credit, but it is C#, so Tourne would rewrite it |
| [komorebi](https://github.com/LGUG2Z/komorebi) | A tiling window manager for Windows. | Komorebi licence (source available, personal use) | Ideas only, plus importing a user's own `komorebi.json` |
| [AltSnap](https://github.com/RamonUnch/AltSnap) | Alt+drag to move, Alt+right-drag to resize, and more on mouse buttons and the wheel, with snapping. Injects nothing. | GPL | Ideas only, plus importing a user's own `AltSnap.ini` |

## 2. The glass effect, honestly

Clip 1 is a compositor shader: Hyprland draws every window through it. On Windows, DWM draws other apps' windows and nobody else can run a shader on them. So the glass comes in three layers, each switched on separately:

| Layer | Where | How | Looks like the clip? |
| --- | --- | --- | --- |
| **A. Tourne surfaces** | Bars, switcher, launcher, panels, widgets, notifications, OSD, the Studio itself | Tourne draws real liquid glass with a Direct3D 11 pixel shader: blur, edge refraction, rainbow fringe (chromatic aberration), edge glow (fresnel), top highlight (specular), bevel, tint, lens dome, oil film. The backdrop is the **wallpaper image Tourne already reads**, so refraction costs nothing per frame. It redraws only when the surface moves or changes. | Yes, fully |
| **B. The glass rim on every window** | Around any app window | The border overlay (`borders.cpp`, already there, no hook) becomes a glass rim: edge bend, rainbow fringe, edge glow and top highlight drawn in a band around the window, sampling the wallpaper behind it. Click glow plays once on focus. | Close at the edges, which is where the clip's look mostly lives. The middle of an opaque app stays the app |
| **C. See-through glass inside apps** | Opt-in, per app, through window rules | Windows 11 backdrop (`DWMWA_SYSTEMBACKDROP_TYPE`: Acrylic or Mica) for apps that allow it. Apps with their own transparency (Windows Terminal, many terminals, Electron apps through the CSS injector) are set to it. For the rest, an opt-in "frosted" mode: layered opacity plus the acrylic blur-behind policy (the route tools like Glass8 use). Their text gets slightly see-through, so it is off by default and per app. | Yes for terminals (clip 3 is exactly this), partly for others |

The **oil film** (animated swirls) costs a frame on every refresh while visible. So it is off by default, runs only on Tourne surfaces and the focused window's rim, and stops when nothing changes on screen for a moment (inferred: a few seconds is the right default).

**Glass Tuner page** (the panel in clip 1, inside the Studio): saved looks, every slider live, "hold to compare" (glass off while held), a starting-point button (Apple-ish, Clear, Frosted, Aero), a test window, next wallpaper, and Save or Revert. Settings are named after hyprglass's vocabulary so users who know it feel at home: blur, refraction, fringe, edge glow, highlight, bevel, tint, lens, plus Tourne's oil film and click glow.

## 3. Tourne Studio: HyprMod's feature set on Windows

HyprMod's features, read from its source (pages: animations, autostart, binds, cursor, env vars, layer rules, layouts, monitors, pending, plugins, profiles, settings, window rules, workspaces):

| HyprMod feature | How HyprMod does it | Tourne Studio | Possible? |
| --- | --- | --- | --- |
| Live preview of every option | Sends each change to the running compositor over its socket (`keyword`) at once | Sends each change to Tourne Desktop over its pipe (the `tournectl` pipe from the parity map); parts re-read only what changed, as now | Yes |
| Your config stays untouched | Writes only its own `hyprland-gui.conf`, which the user's config sources | Writes only `tourne-gui.conf`, sourced last by `tourne.conf`. Lines from the user's own file show read-only, marked "set in tourne.conf" | Yes |
| Undo and redo (Ctrl+Z) | An undo stack of applied changes | Same | Yes |
| Pending changes page | Every unsaved edit listed by category, with revert and "go to" | Same | Yes |
| Remove override | Drops the GUI's line so the user's own value applies again | Same | Yes |
| Profiles: save, name, share, switch | Snapshot folders with metadata; CLI `profile list/apply/next/previous` | These become **sets** (section 4): a set is a profile plus wallpaper, palette, glass and pieces. `tournectl set apply <name>`, plus a tray menu | Yes |
| Config DNA | A small picture made from a hash of the settings, so profiles are told apart at a glance | Same idea, coloured with the set's palette | Yes |
| Global search (Ctrl+F) | Searches every option | Same, across all parts | Yes |
| Schema-driven option pages | `options.json` lists every option with type and default | Tourne already builds `defaults.conf` from each part's settings schema; the Studio reads the same schema | Yes (base exists) |
| Colour and gradient pickers | Colour rows; gradient = colour list plus angle | Same, plus "use palette role" (Accent, Ground...) so a colour follows the theme | Yes |
| Bezier curve editor with live preview and presets | Canvas editor | Same, for the animations engine (springs too) | Yes |
| Monitor layout editor (VRR, 10-bit, HDR), with a confirm-or-revert timer | Monitor cards and preview | Drag screens, set mode and position through Windows' display API, with the same 15 second "keep these settings?" revert. Scale and HDR link to Windows Settings where no API exists | Approximate |
| Keybind editor with key capture, including mouse binds | Captures keys; bindm for mouse drag | Same, with the binds engine; every key starts unassigned and shows the risk wording | Yes |
| Window, layer and workspace rule editors with live preview, window picker and app picker | Dialogs; click a window to fill in its class | Same, with the rules engine; the picker fills exe, class and title | Yes |
| Autostart (exec, exec-once) | List editor | Tourne autostart list | Yes |
| Environment variables | List editor | Not useful on Windows; left out | No (by choice) |
| Cursor theme picker with live preview | Lists cursor themes | Windows pointer schemes, plus sets can carry one | Yes |
| Plugins page and plugin option schemas | Per-plugin schema files | Pieces page: each piece brings its own schema | Yes |
| Onboarding, config errors, deprecation and migration banners | Dialogs and banners | First-run page (pick a set or start blank); a config error banner with the line | Yes |
| Roadmap: backups with history, translations, hyprpaper/hypridle/hyprlock pages | Planned | Backups on save from day one; wallpaper, idle and lock-look pages are Tourne parts anyway | Yes |

What the Studio adds beyond HyprMod: the Glass Tuner, the palette page (section 5), the wallpaper manager (its own proposal), the set gallery, and WM presets.

The Studio is a Tourne Desktop window drawn by Tourne (Direct2D, with Layer A glass), opened from the tray. It does no work while closed.

## 4. One-click setups (sets)

A **set** is one folder, shared as one file (zip), applied with one click and undone with one click:

| In a set | Example |
| --- | --- |
| Settings snapshot (`tourne-gui.conf` for that set) | Bar style, positions, gaps, borders |
| Wallpapers (one or a rotation) and their recolour settings | The clip 3 forest |
| Palette rule | From the wallpaper (Material You or Tourne's OKLab), or fixed colours |
| Glass look | A saved Glass Tuner look |
| Pieces and their layout | Clock, lyrics, visualiser, system monitor, quick notes, launcher with mascot, notification sidebar, pet shelf |
| WM preset | Tourne tiling, komorebi-style, AltSnap mouse, or none |
| Fonts and cursor | A pixel font, a pointer scheme |
| App templates | Terminal, VS Code, Discord, browser colours |
| Keymap | Suggested only: applying a set never assigns a key without the user confirming |

Applying a set keeps a backup of everything it changes, so "Undo set" puts the desktop back exactly. The format grows out of the v1 Theme Contract (handoff) and section 12 of the 12 themes breakdown, once those reports are in this project.

**Starter sets, from the references:** Liquid Glass (clip 1), Forest Terminal (clip 3), Caelestia-style Night (clip 2; my own design, ideas only), Mochi Desk (clip 4, with the pet shelf), and up to 12 more from the 12 themes video. Each needs pieces that don't exist yet, so sets arrive as their pieces do.

## 5. Auto palettes (omarchy-auto-theme and pywal, made for Windows)

The pipeline is the same as omarchy-auto-theme's, with no terminal and no systemd:

1. The wallpaper changes. The theme engine already sees it with a folder watch, no polling.
2. A palette is made. Two generators, picked in the Studio:
   - **Tourne** (exists): OKLab clustering with the profiles adaptive, monochrome, muted, vivid and pastel, contrast-checked.
   - **Material You** (new): Google's colour utilities (Apache 2.0, C++), with omarchy-auto-theme's choices: prefer darkness, lightness, saturation, less saturation or value; dark or light.
   - Both also make a **16-colour terminal palette** (pywal's job).
3. The palette goes into **templates**, one per app, each writing that app's own theme file (as omarchy-auto-theme does), so apps keep their normal theming:
   - Windows Terminal: a colour scheme in its settings.
   - VS Code: a colour theme.
   - Discord, Spotify and Claude: CSS through the CSS injector.
   - Firefox and Chromium browsers: the Tourne extension from the auto theming plan.
   - Oh My Posh, YASB, btop-likes, Notepad++, Explorer and the suite: their own formats.
   - Users can add a template (a file with `{{accent}}`-style holes) without code.
4. Each app reloads when it can. Apps that can't are told on their next start.
5. Rotation: every N minutes, or daily, optional.

Off by default, as now (`theme.source = off`).

## 6. Window manager presets

| Preset | What the user gets | Built from | Exists |
| --- | --- | --- | --- |
| **Tourne tiling** | Today's tiling: columns, rows, stacks, grid, bsp, monocle, per screen and desktop | tiling.cpp | Yes |
| **komorebi-style** | The same layouts komorebi has (Tourne's list already matches: bsp, columns, rows, vertical stack, horizontal stack, ultrawide, grid, monocle), with komorebi-like gaps and borders, plus "import my komorebi.json" (workspace names, layouts, float rules, border colours) | Tiling plus the rules engine | Mostly; import is new |
| **AltSnap-style mouse** | Alt+drag moves, Alt+right-drag resizes, Alt+middle maximizes, wheel changes opacity or desktop, snapping to edges and windows, plus "import my AltSnap.ini" | Mouse binds from the parity map: a mouse hook that exists only while the modifier is held. The modifier is unassigned until the user picks one | No |
| **PrinceWM-style switcher** | A Switcher style: Alt+Tab opens an infinite canvas of live previews in real recent order, with app stacks you drill into, a pinboard (paste images and notes), paint, drag one tile onto another to tile them side by side, and the chosen preview growing into the real window | DWM live thumbnails (`DwmRegisterThumbnail`), which need no admin rights and no global hook, unlike PrinceWM. Drawn by Tourne with Layer A glass | No |

Presets are options in the Studio's Window manager page; picking one changes only the window manager settings, so they mix with any set.

## 7. Build order

1. **Studio core**: pipe and live apply, `tourne-gui.conf`, undo, pending, search, schema pages for what exists today, colour pickers with palette roles. This is the base everything else is shown in.
2. **Glass**: Layer A shader for Tourne surfaces and the Glass Tuner; then Layer B rims on the border overlay; then Layer C per-app rules.
3. **Palettes**: Material You generator, 16-colour output, templates for Terminal, VS Code and the injector apps.
4. **Sets**: format, apply and undo, gallery, the first starter sets.
5. **WM presets**: PrinceWM-style switcher, AltSnap-style mouse binds, komorebi import (needs the binds and rules engines from the parity map).
6. **Pieces** for the starter sets: clock, lyrics, visualiser, system monitor, quick notes, launcher with mascot, notification sidebar, pet shelf.

The wallpaper manager (its own proposal) sits beside steps 3 and 4.

## 8. Decisions for Tourne

1. Build order as above, starting with the Studio core (recommended), or start with glass so the look shows first?
2. Layer C "frosted" mode for apps that don't support transparency: allow it as an opt-in per app (recommended), or leave such apps alone?
3. The default palette generator: Tourne's OKLab (current) or Material You (recommended, as omarchy-auto-theme uses it)?
4. The PrinceWM-style canvas: a Switcher style beside the current list (recommended), or its own part?
