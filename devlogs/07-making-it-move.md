# 07. Making it move, and making it Hyprland

*October 5 to 14, 2026.*

The look was getting there. What was missing was the *feel*: the way a Hyprland setup moves, tiles and answers the keyboard.

## The motion engine

Tourne's animations speak Hyprland. An `animations { }` block from a Hyprland config pastes straight in: `animation = LEAF, ON, SPEED, CURVE, STYLE` and `bezier = NAME, ...`, plus the newer Lua-style lines. A leaf with no line takes its parent's, the same tree Hyprland uses, with Tourne's bar, Start, popups and switcher hung under `layers`. Curves are solved exactly, not sampled, and there are springs (mass, stiffness, damping) whose length is worked out from when they settle.

Then I asked a fair question: **do the animations even work?** Partly. Only 5 of the 38 lines were read by anything. Every one of them moves something now:

- Start opens and closes faded or slid from the bar's edge.
- Flyouts, previews and the calendar fade in; notifications and plugin panels slide.
- The pager's lit mark slides to the new desktop.
- Tiling moves windows to their tiles along the curve.
- **Desktop switches slide.** A tool window covers every screen with live thumbnails: what's on every desktop stays still, the old desktop's windows slide out and the new one's slide in, and the real switch happens underneath.
- **Windows pop in as they open.** Windows refuses to cloak another program's window, but it does let me turn its transitions off, and a layered window at alpha 0 still has a live thumbnail. So: the window goes invisible in its own show event, a see-through cover animates its thumbnail into place, then the window comes back.

The lines it can't honestly do (dimming, the DPMS fade, zoom) say "Not used yet" in the Studio instead of pretending.

## Switcher views

Alt+Tab got seven views, drawn in Direct3D 11: **List, Flip 3D, Cover Flow, Grid, Mission Control, Ring and Shelf**. Windows fly from where they are into place, the screen behind blurs and dims, and the pictures are live through Windows' own capture API (no yellow border). Win+Tab can open Mission Control instead of Task view.

## Tiling, the Hyprland way

- **Tiling sets:** komorebi (mine), Hyprland, GlazeWM, dwm, i3, Monocle, each one click, and komorebi's layout rules ("1: bsp, 4: grid").
- **Binds:** Hyprland's `bind = MODS, key, command, argument` lines, submaps, 39 commands (focus, move, swap, resize, float, workspaces, scratchpads, groups with a tab bar), mouse binds, idle listeners (hypridle), and window rules (`windowrule = float, class:...`). A rule called `keep` puts Discord back where its rule says, because Discord restores its own size a moment after it shows.
- **tournectl:** a hyprctl of our own on a local pipe, with an event stream.
- **Layers of three:** one window full, two at half, three as a main and two stacked, then the next three as a layer over them.
- **Combos, double press and hold** (from my own request): `escape&end`, double-press binds and hold-for-N-ms binds, with the Studio's key picker recording keys held together and asking how they're pressed.

### Things that bit

- **The tiling shuffled every window** when a console flashed open for a second during a build, then shuffled them all back. New windows wait a moment before tiling, and consoles float by default.
- **Tray clicks sometimes didn't land.** A shy bar that hides is made click-through, and a hide cut short could leave it that way.

## Shaders and extensions

- **Effects:** CRT, VHS, grain, chroma, pixelate, dither, halftone, glitch, vignette, duotone, bloom, invert, as small shader pieces that plug into the glass pass or a **screen shader**: Desktop Duplication per screen, the effect on a click-through overlay that's excluded from capture, so recordings never see it. Rotated screens needed their own sampling. It steps aside for full-screen games.
- **Extensions:** looks as data, a folder with an `extension.json` that may only set looks (motion, glass, shaders, views, tiling, borders, theme), never programs, commands or paths.
- **Dots:** export my whole look as one `.tourne` file, or import someone's `hyprland.conf` and see what changes, page by page, before keeping it.

## Idle means idle

I asked for Tourne Desktop to cost nothing over a default Windows install when nothing is happening. Measured: Explorer wakes about 3 times a second idle; Tourne Desktop was waking about **300 times a second**. The biggest share was the taskbar checking hiding and fading every 100 ms on three bars. After two seconds with no input, the bars now **rest**: those checks stop until a hook sees input again. Borders stopped following every window's moves, the mouse pointer's included, and only follow the front window.
