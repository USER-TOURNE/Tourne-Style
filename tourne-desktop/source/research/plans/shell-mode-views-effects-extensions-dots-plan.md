# TourneSuite plan: shell mode, views, shaders, motion, extensions and dots

Written 2026-10-05. Proposal only: no code until I say go.

My asks (2026-10-05):

- **Shell mode** for the killswitch: Windows' taskbar and Start gone for real (chosen on the card over the outside-only route).
- **Many switcher views** like CKFlip3D's Flip 3D and Cover Flow, next to PrinceWM's infinite canvas. "So varied and expressive."
- **Glass shaders** like ShaderGlass (RetroArch's 1,200+ shaders): a curated set built in, each tunable, with an easy way to fetch more later.
- **Better shadows, glows, blurs, blooms and bleeds**, and animation like Hyprland (`animation = NAME, ONOFF, SPEED, CURVE`), Cinnamon (physics: mass, stiffness, friction), Picom and Compiz.
- **An extensions grid**: compact cards with a description, a small moving preview, and Install or Built in.
- **Dots**: share your whole look as one file; someone imports it and gets every CSS and native theme in one click.

## 1. What the references are (checked)

| Reference | What it is | Licence | How I may use it |
| --- | --- | --- | --- |
| [CKFlip3D](https://github.com/CYMERKAROL/CKFlip3D) | Flip 3D and Cover Flow for Windows 11: live window tiles in a 3D stack, Tab or wheel to cycle, hover lifts a tile, close animation reflows the stack. C++20, Direct3D 11. | PolyForm Noncommercial 1.0.0 (checked in its README) | Ideas only |
| [PrinceWM](https://github.com/Ravestars/PrinceWM) | Infinite-canvas Alt+Tab. | MIT | Ideas (already planned as a Switcher style) |
| [ShaderGlass](https://github.com/mausimus/ShaderGlass) | An overlay that runs RetroArch shaders over the desktop or a window; imports `.slang`/`.slangp`. | GPL 3 (checked) | Ideas only |
| [libretro slang-shaders](https://github.com/libretro/slang-shaders) | RetroArch's shader library (CRT, VHS, handheld, upscalers, blur...). | Each shader has its own licence; many are GPL, some public domain or MIT | Never bundled. Tourne can **run** shaders a person downloads themselves (the "get more later" route), and show each file's licence |
| [Hyprland animations](https://wiki.hypr.land) | An animation tree (windows, layers, fade, border, workspaces...), each with on/off, speed in deciseconds, a curve (Bézier or spring: mass, stiffness, damping) and a style (slide, popin 80%, gnomed, slidefade 20%...). | BSD 3 | Ideas, and its config syntax (Tourne can read it) |
| [Hyprland plugins](https://hypr.land/plugins/) | hyprexpo and hyprspace (overviews), hymission and gloview (Mission Control), Hyprtrails (window trails), Hyprfocus (focus flash), Borders++ (extra borders), Image borders, Hyprglass (liquid glass), hypr-darkwindow (invert), hypr-dynamic-cursors (cursor physics), hypr-hot-edge, hyprland-easymotion (letter jump), hyprwinwrap (an app as wallpaper), hy3, hyprNStack, hyprRiver (layouts). | Mixed | Ideas only; the list is the menu for section 4 |
| Hyprland example configs | end_4, ML4W, fufexan, linuxmobile, flick0, iamverysimp1e, notusknot, coffebar dotfiles. | Mixed (mostly GPL or MIT) | Ideas, and the target for "import a Hyprland dots file" |
| [Cairo Desktop](https://github.com/cairoshell/cairoshell) / [ManagedShell](https://github.com/cairoshell/ManagedShell) | A working Windows shell replacement (tray host, task list, appbars) in C#. | Apache 2.0 | Ideas freely; logic can be ported to C++ with credit |

## 2. Shell mode (first)

**What it is.** Windows lets each account choose its shell: `HKCU\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell`. Set it to `TourneDesktop.exe --shell` and, at sign-in, Windows starts Tourne Desktop instead of Explorer. Explorer's taskbar and Start never exist, so nothing can bring them back until you switch back. When Explorer isn't the shell, `explorer.exe` still opens file windows normally.

**What Tourne Desktop must then do itself** (today it borrows these from Explorer through the Windhawk relay):

| Job | Today | In shell mode |
| --- | --- | --- |
| Tray icons | Mirrored from Explorer's tray by the relay | **Tourne hosts the tray**: a `Shell_TrayWnd` window that receives every app's `Shell_NotifyIcon` and draws the icons in Tourne's bar. This is the biggest piece. |
| Task list | Shell hook | The same (`RegisterShellHookWindow` and `SetTaskmanWindow`) |
| Start, Win key, Win+E/R/D/X | Hooks over Explorer | Tourne's own, no masking needed |
| Wallpaper and desktop | Explorer's | Tourne's wallpaper manager draws it (planned anyway); desktop icons optional |
| App bars, work area | Explorer | Tourne answers `SHAppBarMessage` and sets the work area |
| Autostart apps | Explorer runs them | Tourne runs the Run keys and Startup folders, once per sign-in |
| Volume, brightness OSD; toasts | Explorer and ShellExperienceHost | Tourne's own OSD; toasts kept where Windows still shows them, else Tourne notifications later |

**What stays out of reach:** Action centre and Quick Settings (Win+A, Win+N) belong to Explorer's shell experience and won't open. Tourne would offer its own quick toggles later.

**Safety, so you can never get stuck:**

- **Turning it on** is a Studio switch with a clear warning. It applies at the next sign-in, and `--restore` or the Studio switches back.
- **If Tourne Desktop crashes or doesn't start** within a few seconds of sign-in, a tiny watchdog in the same exe starts Explorer as the shell for that session. Two crashes in a row switch shell mode off until you turn it back on.
- **Escape hatch:** hold Shift while signing in and you get Explorer. Ctrl+Alt+Del, then Task Manager, then running `explorer.exe` always works too.
- The outside killswitch already built stays as a lighter option, off by default. Say if you'd rather it went.

**Build order:** (a) the shell switch, the watchdog and autostart; (b) the tray host, with icons in the bar, overflow and balloon tips; (c) app bars and work area; (d) wallpaper and desktop from the wallpaper manager; (e) Tourne quick toggles. Each step is tested under Wine first, then on your PC.

## 3. Motion: a Hyprland-style animation engine

One engine used by everything Tourne draws: bars, Start, popups, the switcher views, the Studio, overlays.

- **Curves:** Bézier (`bezier = NAME, X0, Y0, X1, Y1`) and spring (mass, stiffness, damping; the Cinnamon physics feel), plus named presets (easeOutQuint, overshoot, rubber...).
- **The tree**, with Hyprland's names where they fit: `global > windows (In, Out, Move) > layers > fade (In, Out, Switch, Shadow, Glow, Dim) > border, borderangle > workspaces > switcher > bar > start > popups`. An unset leaf inherits from its parent.
- **Syntax:** tourne.conf reads Hyprland's own lines as they are (`animation = windowsIn, 1, 4, overshoot, popin 80%`) and Hyprland's newer Lua form, so animation sections can be pasted straight from a Hyprland dots file.
- **Studio › Motion:** a curve editor with the curve drawn and a live dot moving along it, and every leaf as a row with on/off, speed, curve and style. Each row has a small preview playing.
- The identity's `motion` and `speed` (already built) become presets of this tree.
- **Idle cost:** none. Frames are drawn only while something animates. Looping styles (border angle `loop`) are marked as always-drawing, as Hyprland warns.

## 4. Switcher views

All views share one 3D scene in Direct3D 11. Each window's live image comes from Windows' own capture API (Windows.Graphics.Capture, no injection). Windows 11 can hide the capture border; Windows 10 shows a yellow one. Every view follows the identity (corners, frames, material) and the motion engine.

| View | Like | Notes |
| --- | --- | --- |
| Flip 3D | Vista / CKFlip3D | Diagonal stack, Tab and wheel cycle, hover lifts, close reflows |
| Cover Flow | iTunes / CKFlip3D 1.5 | Centre tile flat, the rest angled |
| Expo grid | hyprexpo | Every desktop as a tile, windows inside |
| Mission Control | hymission, gloview, macOS | All windows spread out so none overlap, a strip of desktops on top |
| Spaces overview | hyprspace | Desktops along the top, windows below, drag between |
| Ring | Compiz Ring | Windows on a circle, front one big |
| Cube | Compiz Cube | Virtual desktops on a cube's faces, rotate to switch |
| Shelf | dock-like | One row, magnified under the pointer |
| Easymotion | hyprland-easymotion | Letters over each window: type one to jump |
| Infinite canvas | PrinceWM | Already chosen as a Switcher style |

Each view is a card in the Studio's Views page with a moving preview. Views beyond the first few can ship as extensions (section 7), so the base install stays small.

## 5. Effects: shadows, glows, blooms and shaders

**Quality pass first.** Tourne draws its own soft shadows (separable Gaussian, large radius, colour, offset, "render power" like Hyprland's `shadow:render_power`), outer glow, bloom (bright-pass then blur), inner shadow and bevel, and a dual-Kawase blur (fast at any radius). These go on Tourne's surfaces, and on the front window as an extension of the rims (a shadow or glow window that follows it, as the rim already does). Windows' own DWM shadows on other apps can't be switched off from outside. Tourne's shadow is drawn over or around them.

**Shader slots.** Every glass surface gets a shader slot after the glass pass. There's an optional full-screen or per-window overlay mode too (the ShaderGlass idea), off by default because it draws every frame.

**Built-in originals** (written for Tourne, nothing copied): CRT (scanlines, curvature, mask, glow), VHS (wobble, chroma bleed, noise), film grain, chromatic aberration, pixelate, dither (Bayer, ordered), halftone, ASCII, glitch, vignette, duotone and tint, bloom, invert or dark (hypr-darkwindow), and the liquid glass and oil film already built. Each has sliders, saved per surface, and the identity can set a default.

**More later:** an importer for RetroArch `.slang`/`.slangp` files the person downloads themselves. They're compiled to Direct3D with glslang and SPIRV-Cross (both permissive licences) and listed with their own licence and author.

## 6. Extras from the plugin list

Window trails (Hyprtrails), focus flash (Hyprfocus), extra borders (Borders++, image borders), dim inactive, cursor effects (shake to find, tilt; hypr-dynamic-cursors), hot edges (hypr-hot-edge), an app as wallpaper (hyprwinwrap; pairs with the wallpaper manager). These are drawn by Tourne over windows, never inside them, and each is an extension card, off by default.

## 7. The Extensions grid

**Studio › Extensions:** a compact grid. Each card has its name, one line on what it does, a small looping preview, its author and licence, and **Built in** (a switch) or **Install**.

- **What an extension can be:** a shader, a switcher view preset, an animation preset, a bar (YASB-style CSS and config, which the CSS bar engine already reads), an identity pack, a CSS app style, a wallpaper set. Later, widgets (WebView2, as decided).
- **The format:** a folder or `.zip` with `extension.json` (name, author, licence, kind, version, preview) and its files.
- **Where they come from:** a file you drop in, a link, or a Tourne index (a plain list file on GitHub that Tourne reads only when you open the page).
- **The rule (Omarchy's):** an extension changes how the desktop looks, never what it runs. No programs or DLLs, only data, CSS and shader source that Tourne compiles. Native plugins could come much later, in their own process, so a bad one can't take Tourne down.
- **Against bloat:** a small curated set is built in. Everything else is a card you install, and nothing installed costs anything while it's off.

## 8. Dots: share your whole look

- **Export** makes one `.tourne` file. It holds your Tourne settings (not machine paths), the theme and identity, your CSS app styles, bar configs, shader choices and shader files, animation curves, a list of extensions with where to get them, and optionally the wallpaper.
- **Import** shows a card first: what it changes, what it will download, whose it is. It takes a snapshot, applies everything at once, and **Undo** puts your old look back exactly.
- **From Hyprland:** import a Hyprland dots folder or `hyprland.conf` (old or Lua). Its animations, Béziers, gaps, borders and gradients, rounding, blur and shadow settings, and colours map onto Tourne's. A Linux rice can then be copied onto Windows in one click (as far as Windows allows).
- Same rule as extensions: a dots file never runs anything.

## 9. Recommended order

1. **Shell mode** (section 2): the killswitch you chose. The tray host is its core.
2. **Motion engine** (section 3): everything after it animates with it.
3. **Switcher views** (section 4): Flip 3D and Cover Flow first, then the rest.
4. **Effects quality pass and shader slots** (section 5), with the first built-in shaders.
5. **Extensions grid and dots** (sections 7 and 8), then the Hyprland dots import.
6. **Extras** (section 6) as extension cards, and the RetroArch shader importer.

Each step: built and tested under Wine here, then on your PC; off by default until you turn it on.
