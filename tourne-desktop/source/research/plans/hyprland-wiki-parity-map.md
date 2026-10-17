# TourneSuite Hyprland wiki parity map

Written 2026-10-04. My ask: "reference the entire wiki for a full ecosystem of what I'd like part wise ... it should have every functionality we can, just made to be used with windows instead of for linux".

**Sources.** wiki.hypr.land is blocked from the server, so every Hyprland claim here comes from the wiki's own source files, at github.com/hyprwm/hyprland-wiki (branch main, the "latest git" wiki). Each section names its source file. The site address is the same path (for example `content/configuring/core/dispatchers.md` is wiki.hypr.land/configuring/core/dispatchers/; that mapping is inferred from the site's structure). A copy of every page used is in `tourne-research/hyprland-wiki-cache/`. Plugins come from github.com/hyprwm/hyprland-plugins and Pyprland from github.com/hyprland-community/pyprland. Anything not read from a source is marked "inferred".

**Grounding.** The repo (tiling.cpp, tiling-layouts.cpp, borders.cpp, switcher-host.cpp, taskbar-host.cpp, desktops-pager.cpp, desktop/, DEVLOG.md) and the 2026-10-04 handoff. The older reports (window manager and workspaces, auto theming, 12 themes, native taskbar off, shell core) were not in this project's files, so their content is used only as far as the handoff summarises it. The workspace model follows Tourne's earlier pick: **hybrid**, Windows' own virtual desktops stay and the suite builds on top.

**Note on the wiki itself.** Hyprland now configures in Lua (`hl.config`, `hl.dsp.*`, `hl.animation`) instead of `hyprland.conf`. Source: [configuring/core/config-options](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/config-options.md). Tourne keeps its Hyprland-style `tourne.conf`.

## What full parity means on Windows

Hyprland is the compositor: it draws every window, so it can blur, bend, fade and animate all of them. On Windows, DWM is the compositor and no other program may draw other apps' windows. Full parity therefore means: **every function Hyprland offers, done the Windows way**, with Tourne drawing its own overlay windows around other apps (as `borders.cpp` already does), using DWM's public window attributes, moving windows with SetWindowPos, and owning its own surfaces (bars, switcher, menus, popups) completely.

The hard limits, and the closest substitute for each:

| Limit | Why | Closest substitute |
| --- | --- | --- |
| No shaders on, or reshaping of, other apps' windows (blur behind any window, wobble, motion blur, trails, custom corner curves, open and close animations) | DWM owns composition | Overlays around windows (borders, glow, shadow, dim); DWM attributes (corner preference, backdrop type, border colour, transitions off); everything on Tourne's own surfaces |
| No lock screen replacement | Windows' secure desktop | Theme the lock picture; a cosmetic overlay before locking (not security) |
| No Wayland protocols (layer shell, input capture, xdg tags, portals, screencopy permissions) | Not on Windows | Win32 app bars (SHAppBarMessage), WinEvents, Windows privacy settings |
| Workspaces belong to every screen, not one (hybrid model) | Windows virtual desktops span screens | Per screen and desktop layouts (exists); move all windows between screens |
| No per-app input tuning, few per-device settings | Windows exposes global settings | Global settings offered in Tourne's settings window |

Tourne's rules apply to every row below: new features default **off**, hotkeys default **unassigned**, and idle costs nothing (event driven, no polling).

## How to read the tables

- **Tourne today**: Exists, Partial, Missing, or Not applicable, with the file or setting name. "Missing (a Windows setting)" means Windows has it and Tourne only needs to offer it.
- **Possible?**: Yes; Approximate (works with a stated limit, or only on Tourne's own surfaces); No.
- **Phase**: Done, P1 to P7 (see Build order), or "-" for not applicable.

## Summary

| Rows | Exists | Partial | Missing | Not applicable | Impossible (Possible? = No) |
| --- | --- | --- | --- | --- | --- |
| 878 | 43 | 75 | 588 | 172 | 170 |

Top 5 gaps (most value for my goal, all possible):

1. **Binds engine with submaps and a window rules engine** (P1). Every Hyprland setup is built on these two; Tourne has per-part hotkeys and float lists only.
2. **Workspace tools on top of Windows desktops** (P2): scratchpads, pin to all desktops, rename, persistent desktops, back and forth, send to screen.
3. **Layouts**: dwindle and master messages, and the scrolling layout (P2).
4. **IPC and `tournectl`** with Hyprland-like events (P1), so bars, scripts and the settings window share one control path.
5. **Ecosystem parts**: wallpaper manager with recolourer (first, as I asked), idle, colour picker, screenshot, launcher, OSD, clipboard (P5).

## 1. Config language

Source: [configuring/core/_index](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/_index.md), [configuring/naming-conventions](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/naming-conventions.md), [configuring/core/environment-variables](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/environment-variables.md), [configuring/core/advanced-configuration/permissions](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/advanced-configuration/permissions.md).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `Config file (hyprland.lua; older hyprland.conf)` | One file for everything | Exists (tourne.conf, Hyprland-style key = value and groups) | Keep Tourne's format; names match Hyprland where meaning matches | Yes | Done |
| `hl.config({...}) / category.option` | Set options | Exists (group { } and dotted keys) | Done | Yes | Done |
| `Auto reload on save` | Apply on save | Exists | Folder watch, only changed modules told | Yes | Done |
| `Sourcing other files (source = ...)` | Split config across files | Missing | `source = path` in tourne.conf; sets use it | Yes | P1 |
| `Variables ($var)` | Reusable values | Missing | Variables in tourne.conf | Yes | P1 |
| `Lua scripting, lambdas, conditional binds` | Programmable config | Missing | Not planned: I asked for no terminal. Rules, binds and events cover the uses; a plugin API later (inferred decision, still mine to confirm) | Approximate | P7 |
| `Expressions in rules (monitor_w * 0.5)` | Math in sizes | Missing | Small expression evaluator for rule values | Yes | P1 |
| `Data types: color (rgba, gradients with angle)` | Colour syntax | Exists (borders: "#64aa89 #121c21 0,1 1,0" gradient form) | Accept Hyprland's rgba() and 0xAARRGGBB too | Yes | P1 |
| `Config errors shown on screen` | Error bar | Partial (logged, skipped; desktop.log) | Tourne toast with the line number | Yes | P1 |
| `exec-once / exec (autostart)` | Run programs at start and reload | Missing | Tourne autostart list (Windows Startup still works) | Yes | P1 |
| `Environment variables (env =)` | Set env for children | Not applicable | Linux toolkits; Windows apps don't read these. Per-launch env possible for exec | Approximate | - |
| `Permissions (ecosystem.enforce_permissions)` | Allow screencopy, plugins, keyboards | Not applicable | Windows privacy settings own capture and input | No | - |
| `Keywords page items: monitor, workspace, windowrule, bind, animation, bezier, gesture, device` | Top-level keywords | Partial | Each mapped in its own table here | Yes | P1 |
| `Per-device config (device { name })` | Settings per mouse or keyboard | Missing | Raw input device ids; Windows has few per-device knobs | Approximate | P6 |

## 2. Variables (367, one row each)

Source: [configuring/core/config-options](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/config-options.md). Layout options are in section 7.


#### `general` (14)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `general.allow_tearing` | Master switch for allowing tearing to occur. See the Tearing page | Not applicable | Windows decides tearing per game (flip model, VRR). Nothing to set. | No | - |
| `general.border_size` | Size of the border around windows | Exists | borders.width; tiling.focusBorder uses the DWM 1 px border. | Yes | Done |
| `general.extend_border_grab_area` | Extends the area around the border where you can click and drag on, only used when `general.resize_on_border` is on | Missing | With resize_on_border: a hit area outside the window, from the border overlay. | Yes | P2 |
| `general.float_gaps` | Gaps between windows and monitor edges for floating windows `-1` means default | Missing | Keep floating windows this far from screen edges (with snap). | Yes | P2 |
| `general.gaps_in` | Gaps between windows | Exists | tiling.gapInner. | Yes | Done |
| `general.gaps_out` | Gaps between windows and monitor edges | Exists | tiling.gapTop, gapBottom, gapLeft, gapRight (Hyprland allows the same four). | Yes | Done |
| `general.gaps_workspaces` | Gaps between workspaces. Stacks with gaps_out | Not applicable | Only matters for Hyprland's side by side workspace animation. | No | - |
| `general.hover_icon_on_border` | Show a cursor icon when hovering over borders, only used when `general.resize_on_border` is on | Missing | With resize_on_border. | Yes | P2 |
| `general.layout` | Which layout to use. Options: `"dwindle"`/`"master"`/`"scrolling"`/`"monocle"` | Exists | tiling.layout: columns, rows, verticalStack, rightMainVerticalStack, horizontalStack, ultrawideVerticalStack, grid, bsp, monocle. Dwindle is close to bsp; master is verticalStack; scrolling is missing. | Yes | Done |
| `general.locale` | Overrides the system locale, e.g. `"en_US"`, `"es"` | Missing (a Windows setting) | Windows display language. | Yes | P6 |
| `general.modal_parent_blocking` | Whether parent windows of modals will be interactive | Missing (a Windows setting) | Windows already blocks the owner of a modal dialog. | Yes | P6 |
| `general.no_focus_fallback` | If true, will not fall back to the next available window when moving focus in a direction where no window was found | Missing | An option of directional focus (tiling chords). | Yes | P2 |
| `general.resize_corner` | Force floating windows to use a specific corner when being resized (1-4 going clockwise from top left, 0 to disable) | Missing | Mouse resize bind (see binds). | Yes | P2 |
| `general.resize_on_border` | Enables resizing windows by clicking and dragging on borders and gaps | Missing | Drag the gap between two tiled windows to resize both. Needs a low level mouse hook only while the pointer is over a gap, or a thin Tourne window placed in each gap (no hook). | Approximate | P2 |

#### `general.col` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `general.col.active_border` | Border color for the active window | Exists | borders.cpp activeColor/inactiveColor (gradients and spin), or DWMWA_BORDER_COLOR (tiling.focusBorder). | Yes | Done |
| `general.col.inactive_border` | Border color for inactive windows | Exists | borders.cpp activeColor/inactiveColor (gradients and spin), or DWMWA_BORDER_COLOR (tiling.focusBorder). | Yes | Done |
| `general.col.nogroup_border` | Border color for inactive windows that cannot be added to a group | Exists | borders.cpp activeColor/inactiveColor (gradients and spin), or DWMWA_BORDER_COLOR (tiling.focusBorder). | Yes | Done |
| `general.col.nogroup_border_active` | Border color for the active window that cannot be added to a group | Exists | borders.cpp activeColor/inactiveColor (gradients and spin), or DWMWA_BORDER_COLOR (tiling.focusBorder). | Yes | Done |

#### `general.snap` (5)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `general.snap.enabled` | Enable snapping for floating windows | Missing | Snap floating windows to edges and neighbours while dragged: EVENT_SYSTEM_MOVESIZESTART/END WinEvent, then adjust in WM_MOVING is not ours, so snap on drop with SetWindowPos. | Yes | P2 |
| `general.snap.border_overlap` | If enabled, windows snap such that only one border's worth of space is between them | Missing | Snap floating windows to edges and neighbours while dragged: EVENT_SYSTEM_MOVESIZESTART/END WinEvent, then adjust in WM_MOVING is not ours, so snap on drop with SetWindowPos. | Yes | P2 |
| `general.snap.respect_gaps` | If enabled, snapping will respect gaps between windows (set in general:gaps_in) | Missing | Snap floating windows to edges and neighbours while dragged: EVENT_SYSTEM_MOVESIZESTART/END WinEvent, then adjust in WM_MOVING is not ours, so snap on drop with SetWindowPos. | Yes | P2 |
| `general.snap.monitor_gap` | Minimum gap in pixels between window and monitor edges before snapping | Missing | Snap floating windows to edges and neighbours while dragged: EVENT_SYSTEM_MOVESIZESTART/END WinEvent, then adjust in WM_MOVING is not ours, so snap on drop with SetWindowPos. | Yes | P2 |
| `general.snap.window_gap` | Minimum gap in pixels between windows before snapping | Missing | Snap floating windows to edges and neighbours while dragged: EVENT_SYSTEM_MOVESIZESTART/END WinEvent, then adjust in WM_MOVING is not ours, so snap on drop with SetWindowPos. | Yes | P2 |

#### `decoration` (12)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.active_opacity` | Opacity of active windows | Missing | SetLayeredWindowAttributes on the window (WS_EX_LAYERED). Works on most Win32 windows, breaks some (games, UWP). Default off, with an app skip list. | Approximate | P3 |
| `decoration.border_part_of_window` | Whether the window border should be a part of the window | Exists | borders.offset (border inside, on, or outside the frame). | Yes | Done |
| `decoration.dim_around` | How much the `dim_around` window rule should dim by | Missing | A Tourne overlay window under the chosen window that dims the rest. Click-through, no work while nothing changes. | Approximate | P3 |
| `decoration.dim_inactive` | Enables dimming of inactive windows | Missing | A click-through dim overlay per inactive window (Tacky style overlay, like the border overlay). Phase 2. | Approximate | P3 |
| `decoration.dim_modal` | Enables dimming of parents of modal windows | Missing | As dim_around, under a modal dialog. | Yes | P3 |
| `decoration.dim_special` | How much to dim the rest of the screen by when a special workspace is open. [0.0 - 1.0] | Missing | With the scratchpad (special workspace). | Yes | P3 |
| `decoration.dim_strength` | How much inactive windows should be dimmed | Missing | With dim_inactive. | Yes | P3 |
| `decoration.fullscreen_opacity` | Opacity of fullscreen windows | Missing | As active_opacity. | Approximate | P3 |
| `decoration.inactive_opacity` | Opacity of inactive windows | Missing | As active_opacity, switched on focus change (EVENT_SYSTEM_FOREGROUND). | Approximate | P3 |
| `decoration.rounding` | Rounded corners' radius (in layout px) | Partial | Windows 11 rounds every window itself (DWMWA_WINDOW_CORNER_PREFERENCE: square, small, round). Any radius only for Tourne surfaces; borders.radius already follows it. | Approximate | P3 |
| `decoration.rounding_power` | Adjusts the curve used for rounding corners, larger is smoother, 1.0 is a triangular corner, 2.0 is a circle, 4.0 is... | Missing | Tourne surfaces only. | Approximate (Tourne surfaces only) | P3 |
| `decoration.screen_shader` | A path to a custom shader to be applied at the end of rendering. See `examples/screenShader.frag` for an example | Partial | Whole-screen colour only: Magnification API colour effect (MagSetFullscreenColorEffect), the same route as hyprsunset. No custom shaders. | Approximate | P3 |

#### `decoration.blur` (17)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.enabled` | Enable kawase window background blur | Partial | Tourne surfaces have acrylic (switcher theme.backdrop). Other apps: the Mica or Acrylic backdrop switch, where the app allows it. | Approximate | P3 |
| `decoration.blur.brightness` | Brightness modulation for blur | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.contrast` | Contrast modulation for blur | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.ignore_opacity` | Make the blur layer ignore the opacity of the window | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.input_methods` | Whether to blur input methods, e.g. `fcitx5` | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.input_methods_ignorealpha` | Works like ignore_alpha in layer rules. If pixel opacity is below the set value, will not blur | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.new_optimizations` | Whether to enable further optimizations to the blur. Recommended to leave on, as it will massively improve performance | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.noise` | How much noise to apply | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.passes` | The amount of passes to perform | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.popups` | Whether to blur popups, e.g. `right-click menus` | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.popups_ignorealpha` | Works like ignore_alpha in layer rules. If pixel opacity is below the set value, will not blur | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.size` | Blur size (distance) | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.special` | Whether to blur behind the special workspace (note: expensive) | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.variant` | Blur pattern variant. May significantly increase GPU and CPU usage | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.vibrancy` | Increase saturation of blurred colors | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.vibrancy_darkness` | How strong the effect of `vibrancy` is on dark areas | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.xray` | If enabled, floating windows will ignore tiled windows in their blur. Will reduce overhead on floating blur... | Missing | Other apps: only DWMWA_SYSTEMBACKDROP_TYPE (Mica, Acrylic, Tabbed) or the undocumented accent policy, and only where the app draws a see-through frame. Tourne bars, menus and popups: full control. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.acrylic` (5)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.acrylic.aberration` | Relative chromatic separation in the acrylic lens | Missing | A blur look for Tourne surfaces only (bars, switcher, menus), drawn by Tourne over a captured backdrop. Static, so cheap. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.acrylic.bulb` | Width of the curved acrylic edge in pixels | Missing | A blur look for Tourne surfaces only (bars, switcher, menus), drawn by Tourne over a captured backdrop. Static, so cheap. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.acrylic.clarity` | Amount of sharp backdrop passing through the acrylic surface | Missing | A blur look for Tourne surfaces only (bars, switcher, menus), drawn by Tourne over a captured backdrop. Static, so cheap. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.acrylic.refraction` | Maximum acrylic lens displacement in pixels | Missing | A blur look for Tourne surfaces only (bars, switcher, menus), drawn by Tourne over a captured backdrop. Static, so cheap. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.acrylic.tint` | Acrylic tint color. Alpha channel controls optical absorption | Missing | A blur look for Tourne surfaces only (bars, switcher, menus), drawn by Tourne over a captured backdrop. Static, so cheap. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.aurora` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.aurora.color1` | First aurora curtain color. Alpha controls its contribution | Missing | Tourne surfaces only. Animated, so it redraws every frame while shown: off by default, and paused when the surface is hidden. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.aurora.color2` | Second aurora curtain color. Alpha controls its contribution | Missing | Tourne surfaces only. Animated, so it redraws every frame while shown: off by default, and paused when the surface is hidden. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.aurora.intensity` | Strength of the aurora color contribution | Missing | Tourne surfaces only. Animated, so it redraws every frame while shown: off by default, and paused when the surface is hidden. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.aurora.speed` | Animation speed. 0 freezes the animation | Missing | Tourne surfaces only. Animated, so it redraws every frame while shown: off by default, and paused when the surface is hidden. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.drops` (1)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.drops.speed` | Animation speed. 0 disables the animation | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.fluid_jar` (7)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.fluid_jar.color` | Fluid color | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.fluid_jar.distortion` | Fluid refraction distortion multiplier | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.fluid_jar.fill_amount` | Fill level for of the fluid | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.fluid_jar.mass` | Fluid's Inertial mass | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.fluid_jar.precision` | Fluid simulation precision multiplier. 2x is a good compromise. 4x is expensive. 8x is extreme and unnecessary | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.fluid_jar.speed` | Animation speed | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.fluid_jar.turbulence` | Interior fluid turbulence multiplier | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.glass` (3)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.glass.refraction` | Maximum refraction displacement for glass blur types in pixels | Missing | As acrylic: Tourne surfaces only, static. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.glass.roughness` | Strength of the glass relief shading | Missing | As acrylic: Tourne surfaces only, static. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.glass.size` | Pattern size for glass blur types in pixels | Missing | As acrylic: Tourne surfaces only, static. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.haze` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.haze.intensity` | Strength of the haze pearlescent sheen | Missing | As acrylic: Tourne surfaces only, static. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.haze.iridescence` | Strength of the haze pearlescent color shift | Missing | As acrylic: Tourne surfaces only, static. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.heat_shimmer` (1)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.heat_shimmer.speed` | Animation speed. 0 disables the animation | Missing | As aurora: animated, off by default. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.ripple` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.ripple.duration` | Duration in seconds | Missing | Tourne surfaces only; plays once on a click, so no idle cost. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.ripple.radius` | Maximum radius in pixels | Missing | Tourne surfaces only; plays once on a click, so no idle cost. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.ripple.strength` | Maximum refraction displacement in pixels | Missing | Tourne surfaces only; plays once on a click, so no idle cost. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.ripple.width` | Width of waves in pixels | Missing | Tourne surfaces only; plays once on a click, so no idle cost. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.blur.water` (5)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.blur.water.damping` | Decay damping | Missing | Tourne surfaces only; reacts to the pointer, idle when still. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.water.duration` | Maximum duration in seconds | Missing | Tourne surfaces only; reacts to the pointer, idle when still. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.water.radius` | Pointer radius in pixels | Missing | Tourne surfaces only; reacts to the pointer, idle when still. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.water.speed` | Propagation speed | Missing | Tourne surfaces only; reacts to the pointer, idle when still. | Approximate (Tourne surfaces only) | P3 |
| `decoration.blur.water.strength` | Maximum refraction displacement and injection strength in pixels | Missing | Tourne surfaces only; reacts to the pointer, idle when still. | Approximate (Tourne surfaces only) | P3 |

#### `decoration.shadow` (8)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.shadow.enabled` | Enable drop shadows on windows | Exists | borders.shadow. | Yes | Done |
| `decoration.shadow.color` | Active window shadow's color. Alpha dictates the opacity | Partial | Border overlay shadow follows the border colours. A colour of its own is a small add. | Yes | P3 |
| `decoration.shadow.color_inactive` | Inactive window shadow's color. If not set, will fall back to `color` | Partial | As color. | Yes | P3 |
| `decoration.shadow.offset` | Shadow's rendering offset | Exists | borders.shadowX, shadowY. | Yes | Done |
| `decoration.shadow.range` | Shadow range (size) in pixels | Exists | borders.shadowBlur. | Yes | Done |
| `decoration.shadow.render_power` | In what power to render the falloff. More power, the faster the falloff | Missing | Falloff curve of the border overlay shadow. | Yes | P3 |
| `decoration.shadow.scale` | Shadow's scale | Missing | Border overlay shadow. | Yes | P3 |
| `decoration.shadow.sharp` | If enabled, will make the shadows sharp, akin to an infinite render power | Missing | Border overlay shadow. | Yes | P3 |

#### `decoration.glow` (5)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.glow.enabled` | Enable inner glow on windows | Exists | borders.cpp glow (blur, spread, opacity). | Yes | Done |
| `decoration.glow.color` | Active window glow's color. Alpha dictates opacity | Partial | Follows the border colours now. | Yes | P3 |
| `decoration.glow.color_inactive` | Inactive window glow's color. If not set, will fall back to `color` | Partial | As color. | Yes | P3 |
| `decoration.glow.range` | Glow range (size) in pixels | Exists | borders.cpp glow (blur, spread, opacity). | Yes | Done |
| `decoration.glow.render_power` | In what power to render the falloff. More power, the faster the falloff | Missing | Falloff curve of the border overlay glow. | Yes | P3 |

#### `decoration.motion_blur` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.motion_blur.enabled` | enable motion blur on moving/resizing windows | Not applicable | Needs the compositor. DWM does not let anyone draw other windows. Skip. | No | - |
| `decoration.motion_blur.samples` | The amount of samples to render. More will mean clearer blur, at the cost of more compute | Not applicable | Needs the compositor. DWM does not let anyone draw other windows. Skip. | No | - |

#### `decoration.wobble` (8)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `decoration.wobble.enabled` | enable wobble on moving/resizing windows | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.mesh` | amount of wobble mesh vertices per edge | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.stiffness` | spring stiffness for wobble deformation | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.damping` | spring damping for wobble deformation | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.mass` | spring mass for wobble deformation | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.intensity` | wobble deformation impulse multiplier | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.value_epsilon` | position epsilon below which wobble is considered stable | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |
| `decoration.wobble.velocity_epsilon` | velocity epsilon below which wobble is considered stable | Not applicable | Needs the compositor (mesh deformation of other windows). Skip. | No | - |

#### `animations` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `animations.enabled` | Enable animations | Partial | Tourne surfaces fade (borders.fade, switcher). Windows' own window animations are one switch (SPI_SETCLIENTAREAANIMATION). | Approximate | P3 |
| `animations.workspace_wraparound` | Enable workspace wraparound, causing directional workspace animations to animate as if the first and last workspaces... | Missing | Desktop next and previous wrap around (taskbar desktop keys). | Yes | P3 |

#### `input` (30)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input.accel_profile` | Sets the cursor acceleration profile. See the note below. Leave empty to use `libinput`'s default mode for your... | Missing (a Windows setting) | "Enhance pointer precision" (SPI_SETMOUSE). | Yes | P6 |
| `input.emulate_discrete_scroll` | Emulates discrete scrolling from high resolution scrolling events. `0` disables it, `1` enables handling of... | Not applicable | libinput. | No | - |
| `input.float_switch_override_focus` | If `1`/`2` focus will change to the window under the cursor when changing from tiled-to-floating and vice versa. If... | Missing | With tiling float toggle. | Yes | P6 |
| `input.focus_on_close` | Controls the window focus behavior when a window is closed. When set to `0`, focus will shift to the next window... | Missing | Tiling: which window gets focus when one closes (next in the layout or under the pointer). | Yes | P6 |
| `input.follow_mouse` | Specify if and how cursor movement should affect window focus. See the note below | Missing (a Windows setting) | Windows "activate a window by hovering over it" (SPI_SETACTIVEWINDOWTRACKING, with SPI_SETACTIVEWNDTRKTIMEOUT and SPI_SETACTIVEWNDTRKZORDER). Offer as one choice. | Yes | P6 |
| `input.follow_mouse_shrink` | Shrinks the inactive window hitboxes used for focus detection by the specified number of pixels. This creates a dead... | Not applicable | No Windows control over the hover hit box. | No | - |
| `input.follow_mouse_threshold` | The smallest distance in logical pixels the mouse needs to travel for the window under it to get focused. Works only... | Not applicable | As follow_mouse_shrink. | No | - |
| `input.force_no_accel` | Force no cursor acceleration. This bypasses most of your pointer settings to get as raw of a signal as possible.... | Missing (a Windows setting) | "Enhance pointer precision" off. | Yes | P6 |
| `input.kb_file` | If you prefer, you can use a path to your custom .xkb file | Not applicable | XKB. | No | - |
| `input.kb_layout` | Appropriate XKB keymap parameter | Missing (a Windows setting) | Windows input languages; switching with ActivateKeyboardLayout. | Yes | P6 |
| `input.kb_model` | Appropriate XKB keymap parameter. See the note below | Not applicable | XKB. | No | - |
| `input.kb_options` | Appropriate XKB keymap parameter | Partial | XKB. The common asks (Caps as Ctrl or Esc) are the Scancode Map registry value or a Tourne remap; offer only Caps Lock. | Approximate | P6 |
| `input.kb_rules` | Appropriate XKB keymap parameter | Not applicable | XKB. | No | - |
| `input.kb_variant` | Appropriate XKB keymap parameter | Missing (a Windows setting) | Windows input language variants. | Yes | P6 |
| `input.left_handed` | Switches RMB and LMB | Missing (a Windows setting) | SwapMouseButton. | Yes | P6 |
| `input.mouse_refocus` | If enabled, mouse focus will switch to the hovered window when the pointer crosses a window boundary. Works only... | Missing (a Windows setting) | Part of active window tracking. | Yes | P6 |
| `input.natural_scroll` | Inverts scrolling direction. When enabled, scrolling moves content directly, rather than manipulating a scrollbar | Missing (a Windows setting) | Per device FlipFlopWheel in the registry (needs the device to be re-plugged or a sign out). | Yes | P6 |
| `input.numlock_by_default` | Enable numlock by default | Missing (a Windows setting) | InitialKeyboardIndicators in the registry. | Yes | P6 |
| `input.off_window_axis_events` | Handles axis events around (gaps/border for tiled, dragarea/border for floated) a focused window. `0` ignores axis... | Not applicable | Scrolling the gap between tiled windows; nothing to scroll on Windows. | No | - |
| `input.repeat_delay` | Delay before a held-down key is repeated, in milliseconds | Missing (a Windows setting) | SPI_SETKEYBOARDDELAY (4 steps only). | Yes | P6 |
| `input.repeat_rate` | The repeat rate for held-down keys, in repeats per second | Missing (a Windows setting) | SPI_SETKEYBOARDSPEED (0 to 31). | Yes | P6 |
| `input.resolve_binds_by_sym` | Determines how keybinds act when multiple layouts are used. If false, keybinds will always act as if the first... | Missing | Tourne hotkeys use virtual keys, which already follow the active layout. | Yes | P6 |
| `input.rotation` | Sets the rotation of a device in degrees clockwise off the logical neutral position | Not applicable | Device rotation for libinput. | No | - |
| `input.scroll_button` | Sets the scroll button. Check `wev` for the ID. `0` means default | Not applicable | libinput on-button scrolling. Vendor mouse software does this. | No | - |
| `input.scroll_button_lock` | If the scroll button lock is enabled, the button does not need to be held down. Pressing and releasing the button... | Not applicable | As scroll_button. | No | - |
| `input.scroll_points` | Sets the scroll acceleration profile, when `accel_profile` is set to `"custom"`. Has to be in the form `"<step>... | Not applicable | libinput custom curve. | No | - |
| `input.scroll_method` | Sets the scroll method. Additional info: libinput#scrolling. Options:... | Missing (a Windows setting) | Touchpad two finger scrolling switch. | Yes | P6 |
| `input.scroll_factor` | Multiplier added to scroll movement for external mice. Note that there is a separate setting for touchpad scroll_factor | Missing (a Windows setting) | SPI_SETWHEELSCROLLLINES. | Yes | P6 |
| `input.sensitivity` | Sets the mouse input sensitivity. Additional info: libinput#pointer-acceleration | Missing (a Windows setting) | Pointer speed (SPI_SETMOUSESPEED, 1 to 20). | Yes | P6 |
| `input.special_fallthrough` | if enabled, having only floating windows in the special workspace will not block focusing windows in the regular... | Missing | With the scratchpad. | Yes | P6 |

#### `input.touchpad` (12)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input.touchpad.clickfinger_behavior` | Button presses with 1, 2, or 3 fingers will be mapped to LMB, RMB, and MMB respectively. This disables... | Missing (a Windows setting) | Windows "press the lower right corner to right click". | Yes | P6 |
| `input.touchpad.disable_while_typing` | Disable the touchpad while typing | Missing (a Windows setting) | Windows precision touchpad settings (HKCU\Software\Microsoft\Windows\CurrentVersion\PrecisionTouchPad). Offer as switches. | Yes | P6 |
| `input.touchpad.drag_3fg` | Enables three finger drag. Additional info: libinput#drag-3fg | Missing | Windows has no three finger drag. Route: raw HID touchpad input. Phase 3. | Yes | P6 |
| `input.touchpad.drag_lock` | When enabled, lifting the finger off while dragging will not drop the dragged item. 0 - disabled, 1 - enabled with... | Not applicable | No Windows setting. | No | - |
| `input.touchpad.flip_x` | Inverts the horizontal movement of the touchpad | Not applicable | No Windows setting. | No | - |
| `input.touchpad.flip_y` | Inverts the vertical movement of the touchpad | Not applicable | No Windows setting. | No | - |
| `input.touchpad.middle_button_emulation` | Sending LMB and RMB simultaneously will be interpreted as a middle click. This disables any touchpad area that would... | Not applicable | No Windows setting. | No | - |
| `input.touchpad.natural_scroll` | Inverts scrolling direction. When enabled, scrolling moves content directly, rather than manipulating a scrollbar | Missing (a Windows setting) | Windows precision touchpad settings (HKCU\Software\Microsoft\Windows\CurrentVersion\PrecisionTouchPad). Offer as switches. | Yes | P6 |
| `input.touchpad.scroll_factor` | Multiplier applied to the amount of scroll movement | Missing (a Windows setting) | Windows precision touchpad settings (HKCU\Software\Microsoft\Windows\CurrentVersion\PrecisionTouchPad). Offer as switches. | Yes | P6 |
| `input.touchpad.tap_and_drag` | Sets the tap and drag mode for the touchpad | Missing (a Windows setting) | Windows precision touchpad settings (HKCU\Software\Microsoft\Windows\CurrentVersion\PrecisionTouchPad). Offer as switches. | Yes | P6 |
| `input.touchpad.tap_button_map` | Sets the tap button mapping for touchpad button emulation. When empty, defaults to `"lrm"`. L - Left, M - Middle, R... | Not applicable | No Windows setting. | No | - |
| `input.touchpad.tap_to_click` | Tapping on the touchpad with 1, 2, or 3 fingers will send LMB, RMB, and MMB respectively | Missing (a Windows setting) | Windows precision touchpad settings (HKCU\Software\Microsoft\Windows\CurrentVersion\PrecisionTouchPad). Offer as switches. | Yes | P6 |

#### `input.touchdevice` (3)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input.touchdevice.enabled` | Whether input is enabled for touch devices | Missing (a Windows setting) | Windows pen and touch settings, and the Tablet PC "Calibrate" monitor mapping. | Yes | P6 |
| `input.touchdevice.output` | The monitor to bind touch devices. The default is auto-detection. To stop auto-detection, use an empty string | Missing (a Windows setting) | Windows pen and touch settings, and the Tablet PC "Calibrate" monitor mapping. | Yes | P6 |
| `input.touchdevice.transform` | Transform the input from touchdevices. The possible transformations are the same as those of the monitors | Missing (a Windows setting) | Windows pen and touch settings, and the Tablet PC "Calibrate" monitor mapping. | Yes | P6 |

#### `input.virtualkeyboard` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input.virtualkeyboard.release_pressed_on_close` | Release all pressed keys by virtual keyboard on close | Not applicable | Wayland virtual keyboard protocol. | No | - |
| `input.virtualkeyboard.share_states` | Unify key down states and modifier states with other keyboards. 0 - no, 1 - yes, 2 - yes unless IME client | Not applicable | Wayland virtual keyboard protocol. | No | - |

#### `input.tablet` (10)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input.tablet.enabled` | Whether input is enabled for tablets | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.output` | The monitor to bind tablets. Can be `"current"` or a monitor name. Leave empty to map across all monitors | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.transform` | Transform the input from tablets. The possible transformations are the same as those of the monitors | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.absolute_region_position` | Whether to treat the `region_position` as an absolute position in monitor layout. Only applies when `output` is empty | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.active_area_position` | Position of the active area in mm | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.active_area_size` | Size of tablet's active area in mm | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.left_handed` | If enabled, the tablet will be rotated 180 degrees | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.region_position` | Position of the mapped region in monitor layout relative to the top left corner of the bound monitor or all monitors | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.region_size` | Size of the mapped region. When this variable is set, tablet input will be mapped to the region. `{0, 0}` or invalid... | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |
| `input.tablet.relative_input` | Whether the input should be relative | Missing (a Windows setting) | Wacom or Windows Ink driver panels own the mapping. Link to them; no Tourne code. | Yes | P6 |

#### `input.tablettool` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input.tablettool.eraser_button_mode` | Change the eraser button behavior on the tool. When set to `0`, use the default hardware behavior of the tool. When... | Missing (a Windows setting) | Pen driver settings. | Yes | P6 |
| `input.tablettool.eraser_button_override` | Set a button to be button event when eraser_button_mode is set to `1`. Must be a valid button (e.g., BTN_STYLUS)... | Missing (a Windows setting) | Pen driver settings. | Yes | P6 |
| `input.tablettool.pressure_range_min` | Set the minimum pressure range for the tool. Negative values mean it will use device defaults. Usually it is `0.0` | Missing (a Windows setting) | Pen driver settings. | Yes | P6 |
| `input.tablettool.pressure_range_max` | Set the maximum pressure range for the tool. Negative values mean it will use device defaults. Usually it is `1.0` | Missing (a Windows setting) | Pen driver settings. | Yes | P6 |

#### `gestures` (12)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `gestures.close_max_timeout` | The timeout for a window to close when using a 1:1 gesture, in ms | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_cancel_ratio` | How much the swipe has to proceed in order to commence it. Example, when set to `0.7`: if more than 70% of the... | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_create_new` | Whether a swipe right on the last workspace should create a new one | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_direction_lock` | If enabled, switching direction will be locked when you swipe past the `direction_lock_threshold` (touchpad only) | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_direction_lock_threshold` | In pixels, the distance to swipe before direction lock activates (touchpad only) | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_distance` | In pixels, the distance of the touchpad gesture | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_forever` | If enabled, swiping will not clamp at the neighboring workspaces but continue to the further ones | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_invert` | Invert the swipe direction (touchpad only) | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_min_speed_to_force` | Minimum speed in pixels per timepoint to force the change ignoring `cancel_ratio`. `0` means disabled | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_touch` | Enable workspace swiping from the edge of a touchscreen | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_touch_invert` | Invert the swipe direction (touchscreen only) | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |
| `gestures.workspace_swipe_use_r` | If enabled, swiping will use the `r` prefix instead of the `m` prefix for finding workspaces | Missing | Precision touchpad gestures: Windows reserves 3 and 4 finger swipes, which can be set to "nothing" and then caught as raw HID touchpad input (RegisterRawInputDevices, usage page 0x0D). Workspace swipe maps to desktop switching. | Yes | P6 |

#### `gestures.scrolling` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `gestures.scrolling.move_snap_to_grid` | When releasing the scroll move gesture, whether it should try to snap to the grid | Missing | Only with a scrolling layout (see layouts). | Yes | P6 |
| `gestures.scrolling.move_snap_cursor` | When releasing the scroll move gesture, whether it should snap the cursor to the newly focused window | Missing | Only with a scrolling layout (see layouts). | Yes | P6 |

#### `group` (8)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `group.auto_group` | Whether new windows will be automatically grouped into the focused unlocked group. *Note: if you want to disable... | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.drag_into_group` | Whether dragging a window into an unlocked group will merge them. `0` - disabled, `1` - enabled, `2` - only when... | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.focus_removed_window` | Whether Hyprland should focus on the window that has just been moved out of the group | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.group_on_movetoworkspace` | Whether using hl.dsp.window.move({ workspace }) will merge the window into the workspace's solitary unlocked group | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.insert_after_current` | Whether new windows in a group spawn after current or at group tail | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.merge_floated_into_tiled_on_groupbar` | Whether dragging a floating window into a tiled window groupbar will merge them | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.merge_groups_on_drag` | Whether window groups can be dragged into other groups | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |
| `group.merge_groups_on_groupbar` | Whether one group will be merged with another when dragged into its groupbar. *Note: enabling this option only works... | Missing | Groups are tabbed windows. Route: a Tourne tab strip window owned by the group, one member shown and the rest hidden (ShowWindow), like komorebi stacks. Phase 3. | Yes | P4 |

#### `group.col` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `group.col.border_active` | Active group border color | Missing | Group border colours, through borders.cpp once groups exist. | Yes | P4 |
| `group.col.border_inactive` | Inactive group border color | Missing | Group border colours, through borders.cpp once groups exist. | Yes | P4 |
| `group.col.border_locked_active` | Active locked group border color | Missing | Group border colours, through borders.cpp once groups exist. | Yes | P4 |
| `group.col.border_locked_inactive` | Inactive locked group border color | Missing | Group border colours, through borders.cpp once groups exist. | Yes | P4 |

#### `group.groupbar` (31)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `group.groupbar.enabled` | Enables groupbars | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.blur` | Applies blur to the groupbar indicators and gradients | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.disable_when_only` | Disable groupbar if it contains a single window | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.font_family` | Font used to display groupbar titles, use `misc.font_family` if not specified | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.font_size` | Font size of groupbar title | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.font_weight_active` | Font weight of active groupbar title | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.font_weight_inactive` | Font weight of inactive groupbar title | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.gaps_in` | Gap size between gradients | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.gaps_out` | Gap size between gradients and window | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.gradients` | Enables gradients | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.gradient_round_only_edges` | Round only the gradient edges of the entire groupbar | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.gradient_rounding` | How much to round the gradients | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.gradient_rounding_power` | Adjusts the curve used for rounding gradient corners, larger is smoother, 1.0 is a triangular corner, 2.0 is a... | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.height` | Height of the groupbar | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.indicator_gap` | Height of gap between groupbar indicator and title | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.indicator_height` | Height of the groupbar indicator | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.keep_upper_gap` | Add or remove upper gap | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.middle_click_close` | Whether middle clicking the groupbar closes the clicked window | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.priority` | Sets the decoration priority for groupbars | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.render_titles` | Whether to render titles in the group bar decoration | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.round_only_edges` | Round only the indicator edges of the entire groupbar | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.rounding` | How much to round the indicator | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.rounding_power` | Adjusts the curve used for rounding groupbar corners, larger is smoother, 1.0 is a triangular corner, 2.0 is a... | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.scrolling` | Whether scrolling in the groupbar changes group active window | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.stacked` | Render the groupbar as a vertical stack | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.text_color` | Color for window titles in the groupbar | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.text_color_inactive` | Color for inactive windows' titles in the groupbar, if unset, defaults to text_color | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.text_color_locked_active` | Color for the active window's title in a locked group, if unset, defaults to text_color | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.text_color_locked_inactive` | Color for inactive windows' titles in locked groups, if unset, defaults to text_color_inactive | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.text_offset` | Adjust vertical position for titles | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |
| `group.groupbar.text_padding` | Set horizontal padding for titles | Missing | The tab strip itself, drawn by Tourne, so every look option is doable. | Yes | P4 |

#### `group.groupbar.col` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `group.groupbar.col.active` | Active group bar background color | Missing | Tab strip colours, from the Tourne theme by default. | Yes | P4 |
| `group.groupbar.col.inactive` | Inactive (out of focus) group bar background color | Missing | Tab strip colours, from the Tourne theme by default. | Yes | P4 |
| `group.groupbar.col.locked_active` | Active locked group bar background color | Missing | Tab strip colours, from the Tourne theme by default. | Yes | P4 |
| `group.groupbar.col.locked_inactive` | Inactive locked group bar background color | Missing | Tab strip colours, from the Tourne theme by default. | Yes | P4 |

#### `misc` (42)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `misc.allow_session_lock_restore` | If true, will allow you to restart a lock screen app in case it crashes | Not applicable | Windows owns the lock screen. | No | - |
| `misc.always_follow_on_dnd` | Will make mouse focus follow the mouse when dragging and dropping. Recommended to leave it enabled, especially for... | Not applicable | Windows has its own drag and drop focus. | No | - |
| `misc.animate_manual_resizes` | If true, will animate manual window resizes/moves | Not applicable | Needs the compositor. | No | - |
| `misc.animate_mouse_windowdragging` | If true, will animate windows being dragged by mouse, note that this can cause weird behavior on some curves | Not applicable | Needs the compositor. | No | - |
| `misc.anr_missed_pings` | Number of missed pings before showing the ANR dialog | Missing (a Windows setting) | Windows shows "Not responding" itself (HungAppTimeout). | Yes | P6 |
| `misc.background_color` | Change the background color (requires enabled `disable_hyprland_logo`) | Missing (a Windows setting) | Solid colour desktop background. Part of the wallpaper manager. | Yes | P6 |
| `misc.bell_sound` | Path to custom wav/ogg system bell. "none" or an empty string mutes it. "default" uses the system's current one | Missing (a Windows setting) | Sound scheme (Default Beep). The wallpaper manager's sets could carry sounds later. | Yes | P6 |
| `misc.close_special_on_empty` | Close the special workspace if the last window is removed | Missing | With the scratchpad. | Yes | P2 |
| `misc.disable_autoreload` | If true, the config will not reload automatically on save, and instead needs to be reloaded with `hyprctl reload`.... | Exists | Tourne Desktop applies tourne.conf on save. A switch to turn that off is a small add. | Yes | Done |
| `misc.disable_hyprland_guiutils_check` | Disable the warning if hyprland-guiutils is not installed | Not applicable | Hyprland. | No | - |
| `misc.disable_hyprland_logo` | Disables the random Hyprland logo/anime girl background. :( | Not applicable | Hyprland. | No | - |
| `misc.disable_scale_notification` | Disables notification popup when a monitor fails to set a suitable scale | Not applicable | Hyprland. | No | - |
| `misc.disable_splash_rendering` | Disables the Hyprland splash rendering (the small random bit of text on the background). Requires a monitor reload... | Not applicable | Hyprland. | No | - |
| `misc.disable_watchdog_warning` | Disables the warning about not using start-hyprland | Not applicable | Hyprland. | No | - |
| `misc.disable_xdg_env_checks` | Disable the warning if XDG environment is externally managed | Not applicable | Linux. | No | - |
| `misc.enable_anr_dialog` | Whether to enable the ANR (app not responding) dialog when your apps hang | Missing (a Windows setting) | As anr_missed_pings. | Yes | P6 |
| `misc.enable_swallow` | Enable window swallowing | Missing | A terminal that starts a GUI app hides itself until the app closes. Route: EVENT_OBJECT_SHOW plus the new window's parent process (NtQueryInformationProcess); default off. | Yes | P2 |
| `misc.exit_window_retains_fullscreen` | Whether closing a fullscreen window makes the next focused window to be fullscreened. 0 - disabled, 1 - enabled, 2 -... | Missing | Tiling monocle and fullscreen. | Yes | P2 |
| `misc.float_force_onscreen` | whether/how existing floating windows should be constrained to stay on-screen. 0 - no constraints, 1 - must be... | Missing | Pull floating windows that are off every screen back on (on display change, WM_DISPLAYCHANGE). Fixes a real Windows annoyance. | Yes | P2 |
| `misc.new_float_force_onscreen` | same as `float_force_onscreen`, but specifically for newly-spawned floating windows | Missing | As float_force_onscreen, for new windows. | Yes | P2 |
| `misc.focus_on_activate` | Whether Hyprland should focus an app that requests to be focused (an `activate` request) | Missing (a Windows setting) | Windows foreground lock (SPI_SETFOREGROUNDLOCKTIMEOUT). Tourne can offer: flash only, or let apps take focus. | Yes | P6 |
| `misc.font_family` | Sets the global default font used to render text | Partial | Each Tourne part has its own font setting (switcher font.family). A shared default font is a sets item. | Yes | P2 |
| `misc.force_default_wallpaper` | Enforce any of the 3 default wallpapers. 0 - disables the anime background, 1 - disables the anime background, 2 -... | Not applicable | Hyprland's own wallpapers. | No | - |
| `misc.initial_workspace_token_timeout` | the time in seconds a window has to open on its invoked workspace before the tracking token expires. | Missing | With initial_workspace_tracking. | Yes | P2 |
| `misc.initial_workspace_tracking` | If enabled, windows will open on the workspace they were invoked on. 0 - disabled, 1 - single-shot, 2 - persistent... | Missing | Open an app on the desktop it was started from: note the desktop at launch (from Tourne's Start or launcher) and move the window there when it shows. | Yes | P2 |
| `misc.key_press_enables_dpms` | If DPMS is set to off, wake up the monitors if a key is pressed | Missing (a Windows setting) | Windows wakes screens on input already. | Yes | P6 |
| `misc.layers_hog_keyboard_focus` | If true, will make keyboard-interactive layers keep their focus on mouse move, e.g. `wofi`, `bemenu` | Missing | Tourne popups keep keyboard focus until closed. | Approximate (Tourne surfaces only) | P2 |
| `misc.lockdead_screen_delay` | Delay in ms after which the "lockdead" screen will appear in case a locks creen app fails to cover all the outputs | Not applicable | Windows owns the lock screen. | No | - |
| `misc.middle_click_paste` | Whether to enable middle-click-paste (aka primary selection) | Missing | Linux primary selection. Possible as an opt-in: copy on select is not knowable on Windows, so it would paste the clipboard on middle click. Low value; park. | Approximate | P2 |
| `misc.mouse_move_enables_dpms` | If DPMS is set to off, wake up the monitors if the mouse moves | Missing (a Windows setting) | As key_press_enables_dpms. | Yes | P6 |
| `misc.mouse_move_focuses_monitor` | Whether mouse moving into a different monitor should focus it | Missing | Tourne's "active screen" follows the pointer (switcher position.monitor = cursor already). | Yes | P2 |
| `misc.name_vk_after_proc` | Name virtual keyboards after the processes that create them, e.g. `/usr/bin/fcitx5` will have hl-virtual-keyboard-fcitx5 | Not applicable | Wayland. | No | - |
| `misc.on_focus_under_fullscreen` | If there is a fullscreen or maximized window, decide whether a tiled window requested to focus should replace it,... | Missing | Tiling monocle and fullscreen. | Yes | P2 |
| `misc.render_unfocused_fps` | The maximum limit for render_unfocused windows' FPS in the background (see also Window-Rules, e.g. `render_unfocused`) | Not applicable | Compositor. | No | - |
| `misc.screencopy_force_8b` | Forces 8 bit screencopy | Not applicable | Compositor. | No | - |
| `misc.session_lock_blur` | Enables blur for lock screen. `session_lock_xray` must be enabled | Not applicable | Windows owns the lock screen (it blurs its own picture). | No | - |
| `misc.session_lock_xray` | If true, keep rendering workspaces below your lock screen | Not applicable | Windows owns the lock screen. | No | - |
| `misc.size_limits_tiled` | Whether to apply min_size and max_size rules to tiled windows | Exists | tiling.minWidth, minHeight, tooSmall. | Yes | Done |
| `misc.splash_font_family` | Changes the font used to render the splash text, selected from system fonts (requires a monitor reload to take effect) | Not applicable | Hyprland. | No | - |
| `misc.swallow_exception_regex` | The _title_ regex to be used for windows that should _not_ be swallowed by the windows specified in swallow_regex,... | Missing | With enable_swallow. | Yes | P2 |
| `misc.swallow_regex` | The _class_ regex to be used for windows that should be swallowed (usually, a terminal) | Missing | With enable_swallow (match by exe instead of class). | Yes | P2 |
| `misc.vrr` | Controls the VRR (Adaptive Sync) of your monitors. 0 - off, 1 - on, 2 - fullscreen only, 3 - fullscreen with `video`... | Missing (a Windows setting) | Windows "Variable refresh rate" and per monitor dynamic refresh. Link to Settings. | Yes | P6 |

#### `misc.col` (1)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `misc.col.splash` | Changes the color of the splash text (requires a monitor reload to take effect) | Not applicable | Hyprland splash text. | No | - |

#### `layout` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `layout.single_window_aspect_ratio` | Whenever only a single window is shown on a screen, add padding so that it conforms to the specified aspect ratio. A... | Missing | Tiling: pad a lone window to an aspect ratio. Small change in tiling.cpp. | Yes | P2 |
| `layout.single_window_aspect_ratio_tolerance` | Sets a tolerance for `single_window_aspect_ratio`, so that if the padding that would have been added is smaller than... | Missing | Tiling: pad a lone window to an aspect ratio. Small change in tiling.cpp. | Yes | P2 |

#### `binds` (14)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `binds.allow_pin_fullscreen` | If enabled, allow fullscreen to pinned windows, and restore their pinned status afterwards | Missing | With pin. | Yes | P1 |
| `binds.allow_workspace_cycles` | If enabled, workspaces don't forget their previous workspace, so cycles can be created by switching to the first... | Missing | Desktop back and forth (taskbar desktop keys). | Yes | P1 |
| `binds.disable_keybind_grabbing` | If enabled, apps that request keybinds to be disabled, e.g. `VMs`, will not be able to do so | Not applicable | Wayland keyboard shortcut inhibit. On Windows a game or VM in front gets keys unless a hotkey is registered. | No | - |
| `binds.drag_threshold` | Movement threshold in pixels for window dragging and `click`/`drag` bind flags. `0` means disabled | Missing | With mouse binds. | Yes | P1 |
| `binds.focus_preferred_method` | Sets the preferred focus finding method when using `hl.dsp.focus({ direction })`/`hl.dsp.window.move({ direction... | Missing | Directional focus (tiling chords). | Yes | P1 |
| `binds.hide_special_on_workspace_change` | If enabled, changing the active workspace (including to itself) will hide the special workspace on the monitor where... | Missing | With the scratchpad. | Yes | P1 |
| `binds.ignore_group_lock` | If enabled, dispatchers like `hl.dsp.window.move({ into_group })` and `hl.dsp.window.move({ out_of_group })` will... | Missing | With groups. | Yes | P1 |
| `binds.movefocus_cycles_fullscreen` | If enabled, when focus is on a fullscreen window, it allows `hl.dsp.focus({ direction })` to seek windows. If... | Missing | Directional focus. | Yes | P1 |
| `binds.movefocus_cycles_groupfirst` | If enabled, when in a grouped window, `hl.dsp.focus({ direction })` will cycle windows in the groups first, then at... | Missing | With groups. | Yes | P1 |
| `binds.pass_mouse_when_bound` | If enabled, will pass the mouse events to apps/dragging windows around if a keybind has been triggered | Missing | With mouse binds. | Yes | P1 |
| `binds.scroll_event_delay` | In ms, how many ms to wait after a scroll event to allow passing another one for the binds | Missing | With wheel binds (the pager and switcher wheel already debounce). | Yes | P1 |
| `binds.window_direction_monitor_fallback` | If enabled, moving a window or focus over the edge of a monitor with a direction will move it to the next monitor in... | Partial | Tiling move and focus between screens. | Yes | P1 |
| `binds.workspace_back_and_forth` | An attempt to switch to the currently focused workspace will instead switch to the previous workspace, akin to i3's... | Missing | Desktop keys: pressing the current desktop goes to the previous one. | Yes | P1 |
| `binds.workspace_center_on` | Whether switching workspaces should center the cursor on the workspace (0) or on the last active window for that... | Missing | With cursor warp. | Yes | P1 |

#### `xwayland` (4)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `xwayland.enabled` | Allow running applications using X11 | Not applicable | X11 compatibility on Linux. Windows has no equivalent. | No | - |
| `xwayland.create_abstract_socket` | Create the abstract Unix domain socket for Xwayland connections. Xwayland restart is required for changes to take... | Not applicable | X11 compatibility on Linux. Windows has no equivalent. | No | - |
| `xwayland.force_zero_scaling` | Forces a scale of 1 on Xwayland windows on scaled displays | Not applicable | X11 compatibility on Linux. Windows has no equivalent. | No | - |
| `xwayland.use_nearest_neighbor` | Uses the nearest neighbor filtering for Xwayland apps, making them pixelated rather than blurry | Not applicable | X11 compatibility on Linux. Windows has no equivalent. | No | - |

#### `opengl` (1)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `opengl.nvidia_anti_flicker` | Reduces flickering on NVIDIA at the cost of possible frame drops on lower-end GPUs. On non-NVIDIA, this is ignored | Not applicable | Compositor renderer. | No | - |

#### `render` (19)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `render.async_commit` | Send DRM commits asynchronously when possible, reducing hw cursor latency | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.cm_auto_hdr` | Auto-switch to HDR in fullscreen when needed. `0` - disabled, `1` - switch to `hdr`, `2` - switch to `hdredid` | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.cm_enabled` | Whether the color management pipeline should be enabled or not. Requires restart | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.cm_sdr_eotf` | Default transfer function for displaying SDR apps. `default` - Use default value (sRGB), `gamma22` - treat... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.ctm_animation` | Whether to enable a fade animation for CTM changes (hyprsunset). 2 means "auto" which disables them on NVIDIA | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.commit_timing_enabled` | Enable commit timing proto. Requires restart | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.direct_scanout` | Enables direct scanout. Direct scanout attempts to reduce lag when there is only one fullscreen application on a... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.expand_undersized_textures` | Whether to expand undersized textures along the edge, or rather stretch the entire texture | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.fp16_sdr_tf` | Internal workbuffer transfer function for fp16 in SDR mode. 0 - monitor, 1 - linear | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.icc_vcgt_enabled` | Enable sending VCGT ramps to KMS with ICC profiles | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.keep_unmodified_copy` | Keep unmodified SDR frame copy for screensharing. `0` - disabled, `1` - enabled, `2` - auto (enabled in HDR with SDR... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.new_render_scheduling` | Automatically uses triple buffering when needed, improves FPS on underpowered devices | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.not_shown_fifo_lock` | Control fifo locking for not shown surfaces. always - use fifo lock for any surface, ignore_unfocused - ignore... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.non_shader_cm` | Enable CM without shader. `0` - disable, `1` - whenever possible, `2` - DS and passthrough only, `3` - disable and... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.non_shader_cm_interop` | `0` - external ctm (hyprsunset, etc.) is disabled in fullscreen, `1` - external ctm is enabled in fullscreen, `2` -... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.send_content_type` | Report content type to allow monitor profile autoswitch (may result in a black screen during the switch) | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.use_fp16` | Use FP16 buffers internally. `0` - disabled, `1` - enabled, `2` - enabled in HDR mode | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.use_shader_blur_blend` | Use experimental blurred bg blending (glitched on rotated screens). Set to `true` if blur is missing with fp16 or... | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |
| `render.xp_mode` | Disables back buffer and bottom layer rendering | Not applicable | Compositor renderer and colour pipeline. HDR and ICC are Windows display settings. | No | - |

#### `cursor` (23)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `cursor.default_monitor` | The name of a default monitor for the cursor to be set to on startup (see `hyprctl monitors` for names) | Missing | Put the pointer on a chosen screen at sign in (SetCursorPos). | Yes | P5 |
| `cursor.enable_hyprcursor` | Whether to enable hyprcursor support | Missing (a Windows setting) | Cursor themes are Windows pointer schemes (.cur and .ani, HKCU\Control Panel\Cursors). The sets format can carry one. | Yes | P6 |
| `cursor.hide_on_key_press` | Hides the cursor when you press any key until the mouse is moved | Missing (a Windows setting) | Windows "Hide pointer while typing" (SPI_SETMOUSEVANISH). | Yes | P6 |
| `cursor.hide_on_tablet` | Hides the cursor when the last input was a tablet input until a mouse input is done | Missing (a Windows setting) | Windows Ink does this. | Yes | P6 |
| `cursor.hide_on_touch` | Hides the cursor when the last input was a touch input until a mouse input is done | Missing (a Windows setting) | Windows does this. | Yes | P6 |
| `cursor.hotspot_padding` | The padding, in logical px, between screen edges and the cursor | Not applicable | Compositor. | No | - |
| `cursor.inactive_timeout` | In seconds, after how many seconds of cursor's inactivity to hide it. Set to `0` for never | Missing | Hide the pointer after N seconds still. No public API hides it for every app; route: a transparent cursor scheme swapped in (SetSystemCursor) and restored on move. One timer armed on the last move, so no polling. | Approximate | P5 |
| `cursor.invisible` | Don't render cursors | Missing | As inactive_timeout, always. | Approximate | P5 |
| `cursor.min_refresh_rate` | Minimum refresh rate for cursor movement when `no_break_fs_vrr` is active. Set to minimum supported refresh rate or... | Not applicable | Compositor. | No | - |
| `cursor.no_break_fs_vrr` | Disables scheduling new frames on cursor movement for fullscreen apps with VRR enabled to avoid framerate spikes... | Not applicable | Compositor. | No | - |
| `cursor.no_hardware_cursors` | Disables hardware cursors. `0` - use hw cursors if possible, `1` - don't use hw cursors, `2` - auto (disable when... | Not applicable | Compositor. | No | - |
| `cursor.no_warps` | If true, will not warp the cursor in many cases (focusing, keybinds, etc.) | Missing | Off switch for every Tourne pointer warp. | Yes | P5 |
| `cursor.persistent_warps` | When a window is refocused, the cursor returns to its last position relative to that window, rather than to the centre | Missing | Cursor warp option. | Yes | P5 |
| `cursor.sync_gsettings_theme` | Sync XCursor theme with GSettings. It applies cursor-theme and cursor-size on theme load to GSettings making most... | Not applicable | GTK. | No | - |
| `cursor.use_cpu_buffer` | Makes HW cursors use a CPU buffer. Required on NVIDIA to have HW cursors. `0` - disabled, `1` - enabled, `2` - auto... | Not applicable | Compositor. | No | - |
| `cursor.warp_back_after_non_mouse_input` | Warp the cursor back to where it was after using a non-mouse input to move it, and then returning back to the mouse | Missing | Cursor warp option. | Yes | P5 |
| `cursor.warp_on_change_workspace` | Move the cursor to the last focused window after changing the workspace. `0` - Disabled, `1` - Enabled, `2` - Force... | Missing | Move the pointer to the focused window after a desktop switch. | Yes | P5 |
| `cursor.warp_on_monitor_change` | Move the cursor to the last focused window when focusing a different monitor. `-1` - follow value of... | Missing | Move the pointer when focus moves to another screen (tiling chords). | Yes | P5 |
| `cursor.warp_on_toggle_special` | Move the cursor to the last focused window when toggling a special workspace. `0` - Disabled, `1` - Enabled, `2` -... | Missing | With the scratchpad. | Yes | P5 |
| `cursor.zoom_detached_camera` | Detach the camera from the mouse when zoomed in, only ever moving the camera to keep the mouse in view when it goes... | Missing | With zoom. | Yes | P5 |
| `cursor.zoom_disable_aa` | Disable antialiasing when zooming, which means things will be pixelated instead of blurry | Missing (a Windows setting) | Magnifier "smooth edges" setting. | Yes | P6 |
| `cursor.zoom_factor` | The factor to zoom by around the cursor. Like a magnifying glass. Minimum 1.0 (meaning no zoom) | Missing | Magnification API full screen transform (MagSetFullscreenTransform), the Magnifier's own route. A bind sets the factor. | Yes | P5 |
| `cursor.zoom_rigid` | Whether the zoom should follow the cursor rigidly (cursor is always centered if it can be) or loosely | Missing | With zoom. | Yes | P5 |

#### `ecosystem` (3)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `ecosystem.enforce_permissions` | Whether to enable permission control | Not applicable | Hyprland nags and permission control. | No | - |
| `ecosystem.no_donation_nag` | Disable the popup that shows up twice a year encouraging to donate | Not applicable | Hyprland nags and permission control. | No | - |
| `ecosystem.no_update_news` | Disable the popup that shows up when you update Hyprland to a new version | Not applicable | Hyprland nags and permission control. | No | - |

#### `quirks` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `quirks.prefer_hdr` | Report HDR mode as preferred. `0` - disabled, `1` - always, `2` - gamescope only | Not applicable | Compositor quirks. | No | - |
| `quirks.skip_non_kms_dmabuf_formats` | do not report dmabuf formats which cannot be imported into KMS | Not applicable | Compositor quirks. | No | - |

#### `input-capture` (2)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `input-capture.capture_modifiers` | if enabled, modifiers are also captured and sent to the program | Not applicable | Wayland input capture protocol. | No | - |
| `input-capture.enforce_barriers` | if enabled, throw a Wayland error when an invalid barrier is received | Not applicable | Wayland input capture protocol. | No | - |

#### `debug` (22)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `debug.colored_stdout_logs` | Enables colors in the stdout logs | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.damage_blink` | (epilepsy warning!) Flash areas updated with damage tracking | Missing | Tourne surfaces could flash their redraws for testing. | Approximate (Tourne surfaces only) | P1 |
| `debug.damage_tracking` | Redraw only the needed bits of the display. Do **not** change. None - 0, monitor - 1, full - 2 | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.disable_logs` | Disable logging to a file | Exists | desktop.log. | Yes | Done |
| `debug.disable_scale_checks` | Disables verification of the scale factors. Will result in pixel alignment and rounding errors | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.disable_time` | Disables time logging | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.ds_handle_same_buffer` | special case for direct scanout with unmodified buffer | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.ds_handle_same_buffer_fifo` | special case for direct scanout with unmodified buffer unlocks fifo | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.enable_stdout_logs` | Enables logging to stdout | Not applicable | No console for users. | No | - |
| `debug.error_limit` | Limits the number of displayed config file parsing errors | Missing | With the error notice. | Yes | P1 |
| `debug.error_position` | Sets the position of the error bar. `0` - top, `1` bottom | Missing | With the error notice. | Yes | P1 |
| `debug.fifo_pending_workaround` | fifo workaround for empty pending list | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.full_cm_proto` | Claims support for all cm proto features (requires restart) | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.gl_debugging` | Enables OpenGL debugging with glGetError and EGL_KHR_debug, requires a restart after changing | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.invalidate_buffers` | Allow buffer invalidation (invalidation increases performance but produces glitches on some systems). 0 - not... | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.log_damage` | Enables logging the damage | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.manual_crash` | Set to 1 and then back to 0 to crash Hyprland | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.overlay` | Print the debug performance overlay. Disable VFR for accurate results | Missing | A small Tourne overlay with its own CPU, memory and redraw counts. Useful for the resource-efficient goal. | Yes | P1 |
| `debug.pass` | Enables render pass debugging | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.render_solitary_wo_damage` | render solitary window with empty damage | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |
| `debug.suppress_errors` | If enabled, Hyprland will not display config file parsing errors | Missing | Tourne Desktop logs bad lines; a visible error bar (tray balloon) for config mistakes is worth adding, with this as its off switch. | Yes | P1 |
| `debug.vfr` | Controls the VFR status of Hyprland. Heavily recommended to leave enabled to conserve resources | Not applicable | Compositor or renderer internal; no Windows meaning. | No | - |

#### `experimental` (1)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `experimental.wp_cm_1_2` | Allow wp-cm-v1 version 2 | Not applicable | Wayland protocol. | No | - |

## 3. Binds

Source: [configuring/core/binds/_index](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/_index.md), [binds/flags](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/flags.md), [binds/submaps](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/submaps.md), [binds/globals](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/globals.md), [binds/devices/mouse](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/devices/mouse.md), [binds/keyboard-layouts](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/keyboard-layouts.md).

Route for all key binds: RegisterHotKey (no hook). Anything that needs key-up, pass-through or lone modifiers uses a low level hook that exists only while a bound modifier is held, so no always-on hook.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `bind (keys to dispatcher)` | Key binds | Partial (per part: tiling.leader chords, taskbar desktopKeys, hideHotkey, switcher keys) | Shared bind table in tourne.conf; RegisterHotKey (no hook). Unassigned by default | Yes | P1 |
| `Keycode binds` | Bind by scancode | Missing | Virtual key or scancode in the bind table | Yes | P1 |
| `Modifier-only binds` | Bind a lone modifier (e.g. SUPER) | Partial (tiling leader dummy key trick) | Short-lived keyboard hook from modifier down to up | Approximate | P1 |
| `Multiple binds to one key` | Several actions on one key | Missing | Bind table runs them in order | Yes | P1 |
| `unbind` | Remove a bind | Missing | Bind table | Yes | P1 |
| `flag locked` | Works on the lock screen | Not applicable | Nothing runs on the Windows lock screen | No | - |
| `flag release` | Fire on key release | Missing | Short-lived hook while a bound modifier is down | Yes | P1 |
| `flag click` | Fire on click without drag | Missing | Mouse binds | Yes | P2 |
| `flag drag` | Fire on drag | Missing | Mouse binds | Yes | P2 |
| `flag long_press` | Fire on long press | Missing | Short-lived hook timing key down | Yes | P1 |
| `flag repeating` | Repeat while held | Missing | WM_HOTKEY repeats are not sent; hook while held (inferred) | Yes | P1 |
| `flag non_consuming` | Also pass the key to the app | Missing | Hook that does not swallow; RegisterHotKey always swallows | Approximate | P1 |
| `flag auto_consuming` | Pass the key on if the action fails | Missing | Re-send with SendInput on failure | Approximate | P1 |
| `flag mouse` | Mouse button binds (move, resize) | Missing | Low level mouse hook only while the modifier is held | Yes | P2 |
| `flag transparent` | Cannot be shadowed | Missing | Bind table priority | Yes | P1 |
| `flag ignore_mods` | Ignore modifiers | Missing | Register under every modifier combination | Yes | P1 |
| `flag description` | Text for the bind | Missing | Shown in a Tourne keys cheat sheet | Yes | P1 |
| `flag dont_inhibit` | Ignore shortcut inhibit | Not applicable | Wayland | No | - |
| `flag submap_universal` | Active in every submap | Missing | Submaps | Yes | P1 |
| `flag device` | Per-device binds | Missing | Raw input device id, only with the short-lived hook | Approximate | P6 |
| `flag allow_input_capture` | Works during input capture | Not applicable | Wayland | No | - |
| `Submaps` | Key modes | Missing | Register on enter, unregister on leave | Yes | P1 |
| `Submap nesting, auto close, catch-all` | Submap behaviours | Missing | Catch-all needs the hook while the submap is active | Yes | P1 |
| `Disable all binds with one key` | Master off switch | Missing | A submap with only the way back | Yes | P1 |
| `Global shortcuts (D-Bus)` | App-registered shortcuts | Not applicable | Linux | No | - |
| `Keyboard layout switching (switchxkblayout)` | Change layout by bind | Missing | ActivateKeyboardLayout / WM_INPUTLANGCHANGEREQUEST | Yes | P1 |

## 4. Dispatchers (65, one row each)

Source: [configuring/core/dispatchers](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/dispatchers.md).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `dpms` | Turn screens off or on | Missing | WM_SYSCOMMAND SC_MONITORPOWER broadcast; on with any input | Yes | P1 |
| `event` | Send a custom event to the event socket | Missing | Tourne event pipe (tournectl) | Yes | P1 |
| `exec_cmd` | Run a command, optionally with window rules for what it opens | Missing | ShellExecuteEx; rules applied to the first window of that process (rules engine) | Yes | P1 |
| `exec_raw` | Run a command without a shell | Missing | CreateProcess | Yes | P1 |
| `exit` | Quit Hyprland | Exists (tray Quit, `--stop`) | Bindable action | Yes | P1 |
| `focus { direction }` | Focus the window in a direction | Partial (tiling chords inside a layout) | Geometry search over visible top-level windows on all screens; SetForegroundWindow through the Alt trick the tiling leader already uses | Yes | P1 |
| `focus { monitor }` | Focus another screen | Missing | Top window on that screen (EnumWindows by z-order) | Yes | P1 |
| `focus { workspace }` | Go to a workspace | Exists (taskbar desktopKeys, pager) | IVirtualDesktopManagerInternal SwitchDesktop (hybrid model: Windows desktops) | Yes | Done |
| `focus { window }` | Focus a window by selector | Missing | Selector match (class, exe, title, tag) then SetForegroundWindow | Yes | P1 |
| `focus { urgent_or_last }` | Focus the urgent window, else the last one | Missing | Urgent from RegisterShellHookWindow HSHELL_FLASH; last from a focus history | Yes | P1 |
| `focus { last }` | Focus the previous window | Missing | Focus history from EVENT_SYSTEM_FOREGROUND | Yes | P1 |
| `force_idle` | Pretend the user has been idle N seconds | Missing | Idle part runs its steps up to N | Yes | P5 |
| `force_renderer_reload` | Reload the renderer | Not applicable | Compositor internal | No | - |
| `global` | Trigger a D-Bus global shortcut | Not applicable | Linux D-Bus. Nearest: send_shortcut to an app | No | - |
| `layout` | Send a message to the current layout | Partial (N, [ and ] chords) | Layout message table per layout (see Layouts) | Yes | P2 |
| `no_op` | Do nothing | Missing | Bind table | Yes | P1 |
| `pass` | Pass the key to a window | Missing | PostMessage WM_KEYDOWN/UP; games that read raw input will not see it (inferred) | Approximate | P1 |
| `release_input_capture` | End an input capture session | Not applicable | Wayland protocol | No | - |
| `reload_config` | Reload the config | Exists (reload on save) | Bindable action | Yes | P1 |
| `send_key_state` | Send a key down or up to a window | Missing | SendInput (focused) or PostMessage (other window) | Approximate | P1 |
| `send_shortcut` | Send a shortcut to a window | Missing | As send_key_state | Approximate | P1 |
| `submap` | Enter a submap (key mode) | Missing | Register the submap's hotkeys on enter, unregister on leave; badge shows the mode | Yes | P1 |
| `window.alter_zorder` | Raise or lower a window | Missing | SetWindowPos HWND_TOP / HWND_BOTTOM | Yes | P1 |
| `window.center` | Centre a floating window | Missing | SetWindowPos to the work area centre | Yes | P1 |
| `window.clear_tags` | Remove all tags | Missing | Rules engine tag store | Yes | P1 |
| `window.close` | Ask a window to close | Exists (switcher close button) | WM_CLOSE as a bindable action | Yes | P1 |
| `window.cycle_next` | Focus the next window (tiled or floating filters) | Partial (monocle cycles; Switcher is Alt+Tab) | Z-order or layout order walk | Yes | P1 |
| `window.deny_from_group` | Keep a window out of groups | Missing | Groups part | Yes | P4 |
| `window.drag` | Start moving a window with the mouse | Missing | Mouse bind: low level mouse hook installed only while the bind's modifier is held, then SetWindowPos | Yes | P2 |
| `window.float` | Float, tile or toggle | Exists (tiling float toggle, floatApps/Titles/Classes) | Bindable action with selector | Yes | Done |
| `window.fullscreen_state` | Set internal and client fullscreen separately | Missing | Tourne borderless fullscreen (strip WS_CAPTION, WS_THICKFRAME, size to screen) vs ShowWindow SW_MAXIMIZE told to the app | Approximate | P2 |
| `window.fullscreen` | Fullscreen or maximize | Partial (Windows maximize) | As fullscreen_state; layout_aware means monocle-like in tiling | Yes | P2 |
| `window.kill` | Kill the owning process | Missing | TerminateProcess after a confirm | Yes | P1 |
| `window.move { direction }` | Move a window in a direction (into groups if asked) | Partial (tiling swap in layout) | Floating: step by gap; tiled: swap with the neighbour | Yes | P2 |
| `window.move { workspace }` | Send to a workspace (optionally follow) | Exists (desktopKeys moveTo, follow) | IApplicationViewCollection + MoveViewToDesktop | Yes | Done |
| `window.move { monitor }` | Send to another screen | Missing | SetWindowPos, scaled to the target work area; re-tile both | Yes | P2 |
| `window.move { x, y }` | Move to or by a point | Missing | SetWindowPos | Yes | P1 |
| `window.move { into_group }` | Join the group in a direction | Missing | Groups part | Yes | P4 |
| `window.move { into_or_create_group }` | Join or start a group | Missing | Groups part | Yes | P4 |
| `window.move { out_of_group }` | Leave a group | Missing | Groups part | Yes | P4 |
| `window.pin` | Show on every workspace | Missing | IVirtualDesktopPinnedApps PinView (same COM family as the desktop keys); optionally also topmost | Yes | P1 |
| `window.pseudo` | Pseudotile: keep own size inside the tile | Missing | tiling.cpp: centre the window's floating size in its slot | Yes | P2 |
| `window.resize { interactive }` | Start resizing with the mouse | Missing | Mouse bind, as window.drag | Yes | P2 |
| `window.resize { x, y }` | Resize to or by a size | Partial (tiling [ and ] change the main share) | SetWindowPos; tiled windows change their split | Yes | P2 |
| `window.set_prop` | Change a window property live | Missing | Dynamic rule effects applied at once | Yes | P1 |
| `window.signal` | Send a POSIX signal | Not applicable | No signals on Windows; kill and close cover the common uses | No | - |
| `window.swap { direction }` | Swap with the neighbour | Partial (tiling order swap) | Geometry neighbour then swap slots | Yes | P2 |
| `window.swap { target }` | Swap with a chosen window | Missing | Selector then swap slots | Yes | P2 |
| `window.swap { next }` | Swap with the next | Partial (tiling) | Layout order | Yes | P2 |
| `window.swap { prev }` | Swap with the previous | Partial (tiling) | Layout order | Yes | P2 |
| `window.tag` | Add, remove or toggle a tag | Missing | Rules engine tag store | Yes | P1 |
| `window.toggle_swallow` | Show swallowed windows | Missing | Swallow feature (misc.enable_swallow) | Yes | P2 |
| `workspace.move { monitor }` | Move a workspace to a screen | Not applicable (hybrid model) | Windows desktops cover every screen. Closest: move all of this desktop's windows on one screen to another | Approximate | P2 |
| `workspace.rename` | Rename a workspace | Missing (pager reads names) | IVirtualDesktopManagerInternal SetName | Yes | P2 |
| `workspace.swap_monitors` | Swap two screens' workspaces | Missing | Swap all windows between two screens | Approximate | P2 |
| `workspace.toggle_special` | Show or hide a special workspace (scratchpad) | Missing | Scratchpad part: named window sets hidden (SW_HIDE or DWMWA_CLOAK) and shown over the current desktop, dimmed behind | Yes | P2 |
| `group.active` | Pick a group member by index | Missing | Groups part (tab strip) | Yes | P4 |
| `group.lock` | Lock a group | Missing | Groups part | Yes | P4 |
| `group.lock_active` | Lock the active group | Missing | Groups part | Yes | P4 |
| `group.move_window` | Reorder inside the group | Missing | Groups part | Yes | P4 |
| `group.next` | Next tab | Missing | Groups part | Yes | P4 |
| `group.prev` | Previous tab | Missing | Groups part | Yes | P4 |
| `group.toggle` | Make or break a group | Missing | Groups part | Yes | P4 |
| `cursor.move` | Move the pointer to a point | Missing | SetCursorPos | Yes | P1 |
| `cursor.move_to_corner` | Move the pointer to a window corner | Missing | GetWindowRect then SetCursorPos | Yes | P1 |

## 5. Window rules

Source: [configuring/core/rules/window-rules](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/rules/window-rules.md).

### Matchers

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `class` | Window class (regex) | Partial (tiling.floatClasses) | GetClassName | Yes | P1 |
| `initial_class` | Class at first map | Missing | Class noted on EVENT_OBJECT_SHOW | Yes | P1 |
| `title` | Title (regex) | Partial (tiling.floatTitles) | GetWindowText, re-checked on EVENT_OBJECT_NAMECHANGE | Yes | P1 |
| `initial_title` | Title at first map | Missing | Noted at first show | Yes | P1 |
| (exe, Tourne addition) | Program file name | Partial (tiling.floatApps, borders.skipApps) | QueryFullProcessImageName | Yes | P1 |
| `content` | Content type: none, photo, video, game | Missing | Guess: exclusive or borderless fullscreen, known game list, GPU use (inferred) | Approximate | P1 |
| `focus` | The focused window | Missing | GetForegroundWindow | Yes | P1 |
| `fullscreen` | Fullscreen windows | Partial (taskbar fullscreenStayAway detects them) | Window rect covers the monitor rect | Yes | P1 |
| `fullscreen_state_client` | Fullscreen state the app was told | Missing | IsZoomed vs Tourne fullscreen | Approximate | P2 |
| `fullscreen_state_internal` | Fullscreen state the compositor uses | Missing | As above | Approximate | P2 |
| `float` | Floating windows | Exists (tiling) | Tiling state | Yes | Done |
| `group` | Grouped windows | Missing | Groups part | Yes | P4 |
| `modal` | Modal dialogs | Missing | Owned window whose owner is disabled (GetWindow GW_OWNER, IsWindowEnabled) | Yes | P1 |
| `pin` | Pinned windows | Missing | IVirtualDesktopPinnedApps IsViewPinned | Yes | P1 |
| `tag` | Tagged windows | Missing | Rules engine tags | Yes | P1 |
| `workspace` | Windows on a workspace | Missing | IVirtualDesktopManager GetWindowDesktopId | Yes | P1 |
| `xdg_tag` | Wayland toplevel tag | Not applicable | Wayland | No | - |
| `xwayland` | X11 apps | Not applicable | Nearest Windows idea: UWP vs Win32 (ApplicationFrameWindow) | No | - |

### Static effects (on open)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `center` | Centre a floating window | Missing | SetWindowPos at first show | Yes | P1 |
| `content` | Set the content type | Missing | Mark for games-only rules (shell core) | Yes | P1 |
| `float` | Float a window | Exists (tiling float lists) | Rule form of the float lists | Yes | P1 |
| `fullscreen` | Open fullscreen | Missing | Tourne borderless fullscreen at first show | Approximate | P2 |
| `fullscreen_state` | Open with a set fullscreen state | Missing | As fullscreen | Approximate | P2 |
| `group` | Group options (set, lock, barred, deny...) | Missing | Groups part | Yes | P4 |
| `maximize` | Open maximized | Missing | ShowWindow SW_MAXIMIZE | Yes | P1 |
| `monitor` | Open on a screen | Missing | SetWindowPos to that screen at first show | Yes | P1 |
| `move` | Open at a position (expressions allowed) | Missing | SetWindowPos; expressions with monitor_w, window_w etc. evaluated by Tourne | Yes | P1 |
| `no_close_for` | Refuse closing for N ms after open | Missing | Tourne close actions refuse; the app's own X button still works (inferred limit) | Approximate | P1 |
| `no_initial_focus` | Do not focus on open | Missing | Push behind the previous foreground window on show; apps that force focus win | Approximate | P1 |
| `pin` | Open pinned | Missing | As window.pin | Yes | P1 |
| `pseudo` | Open pseudotiled | Missing | As window.pseudo | Yes | P2 |
| `scrolling_width` | Column width in the scrolling layout | Missing | Scrolling layout | Yes | P2 |
| `size` | Open at a size (expressions allowed) | Missing | SetWindowPos | Yes | P1 |
| `suppress_event` | Ignore fullscreen, maximize, activate requests | Missing | Undo after the fact (EVENT_OBJECT_LOCATIONCHANGE on that one window only) | Approximate | P2 |
| `tile` | Force tiling | Missing | Tiling override | Yes | P1 |
| `workspace` | Open on a workspace (silent or not) | Missing | MoveViewToDesktop at first show (desktop keys route) | Yes | P1 |

### Dynamic effects (while matched)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `allows_input` | Force input to an X11 window | Not applicable | Xwayland | No | - |
| `animation` | Animation style for this window | Not applicable | DWM animates other windows; only on/off (DWMWA_TRANSITIONS_FORCEDISABLED) | No | - |
| `border_color` | Border colour or gradient for this window | Partial (borders.cpp global colours) | Per-window colour from rules in the border overlay | Yes | P3 |
| `border_size` | Border size for this window | Partial (borders.width global) | Per-window width in the overlay | Yes | P3 |
| `confine_pointer` | Lock the pointer to the window | Missing | ClipCursor while it is focused | Yes | P2 |
| `dim_around` | Dim everything around it | Missing | Click-through dim overlay below it | Yes | P3 |
| `decorate` | Draw decorations or not | Partial (borders.skipApps) | Per-window overlay off; optional caption removal (SetWindowLong WS_CAPTION) as its own opt-in | Yes | P3 |
| `focus_on_activate` | Let the app take focus | Missing | Bring forward when it flashes (HSHELL_FLASH) | Approximate | P2 |
| `idle_inhibit` | Keep the PC awake (always, focus, fullscreen) | Missing | SetThreadExecutionState from Tourne while the condition holds | Yes | P5 |
| `immediate` | Allow tearing | Not applicable | Game and driver side | No | - |
| `keep_aspect_ratio` | Keep aspect while mouse resizing | Missing | Mouse resize bind | Yes | P2 |
| `max_size` | Largest floating size | Missing | Clamp on resize end (EVENT_SYSTEM_MOVESIZEEND) | Yes | P2 |
| `min_size` | Smallest floating size | Partial (tiling.minWidth/minHeight) | Clamp on resize end | Yes | P2 |
| `nearest_neighbor` | Pixelated scaling | Not applicable | Renderer | No | - |
| `no_anim` | No animations for it | Missing | DWMWA_TRANSITIONS_FORCEDISABLED | Yes | P3 |
| `no_auto_hdr` | No auto HDR for it | Not applicable | Windows graphics settings per app | No | - |
| `no_blur` | No blur | Not applicable | Nothing blurs other windows | No | - |
| `no_dim` | Never dim it | Missing | Dim overlay skip | Yes | P3 |
| `no_focus` | Never focus it | Missing | Refocus the previous window on EVENT_SYSTEM_FOREGROUND; WS_EX_NOACTIVATE cannot be set safely on other apps (inferred) | Approximate | P2 |
| `no_follow_mouse` | Hover focus skips it | Missing | With Tourne hover focus | Yes | P2 |
| `no_glow` | No glow | Partial (borders.glow global) | Per-window overlay setting | Yes | P3 |
| `no_max_size` | Ignore max size | Missing | Rules engine | Yes | P2 |
| `no_screen_share` | Hide from capture | Missing | SetWindowDisplayAffinity only works on a program's own windows: Tourne surfaces only | Approximate (Tourne surfaces only) | P3 |
| `no_shadow` | No shadow | Partial (borders.shadow global) | Overlay shadow off; DWM's own shadow stays | Approximate | P3 |
| `no_shortcuts_inhibit` | App can't block shortcuts | Not applicable | Wayland | No | - |
| `no_vrr` | No VRR for it | Not applicable | Windows graphics settings | No | - |
| `no_wobble` | No wobble | Not applicable | No wobble on Windows | No | - |
| `no_xdg_drags` | Disable XDG drags | Not applicable | Wayland | No | - |
| `opacity` | Opacity (active, inactive, fullscreen) | Missing | SetLayeredWindowAttributes; skip list for apps that break | Approximate | P3 |
| `opaque` | Force full opacity | Missing | Remove the layered alpha Tourne set | Yes | P3 |
| `force_rgbx` | Ignore the app's own transparency | Not applicable | Renderer | No | - |
| `persistent_size` | Remember floating size | Partial (tiling.keepResized, rememberAll) | Size store per exe and class | Yes | P2 |
| `render_unfocused` | Keep rendering when hidden | Not applicable | Compositor | No | - |
| `rounding` | Corner radius | Partial (borders.radius) | DWMWA_WINDOW_CORNER_PREFERENCE (square, small, round) on Windows 11 | Approximate | P3 |
| `rounding_power` | Corner curve | Not applicable | DWM picks it | No | - |
| `scroll_mouse` | Per-app wheel speed | Not applicable | No per-app wheel setting (inferred: a hook could scale it; not worth an always-on hook) | No | - |
| `scroll_touchpad` | Per-app touchpad speed | Not applicable | As scroll_mouse | No | - |
| `stay_focused` | Keep focus while visible | Missing | Refocus on EVENT_SYSTEM_FOREGROUND | Yes | P2 |
| `sync_fullscreen` | Keep both fullscreen states equal | Missing | With Tourne fullscreen | Yes | P2 |
| `tag` | Apply a tag | Missing | Rules engine | Yes | P1 |
| `tonemap` | HDR tone mapping | Not applicable | Windows HDR settings | No | - |
| `xray` | Blur x-ray mode | Not applicable | No blur of other windows | No | - |

### Layer rules (Tourne's own surfaces)

Source: [configuring/core/rules/layer-rules](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/rules/layer-rules.md).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `namespace (matcher)` | Match a layer by name | Missing | Tourne part names: taskbar, pager, switcher, menu, osd, launcher... | Yes (Tourne surfaces) | P3 |
| `above_lock` | Show above the lock screen | Not applicable | Windows owns the lock screen | No | - |
| `animation` | Layer animation style | Partial (taskbar shyAnimation, pager fade) | Animations engine for Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `blur` | Blur behind the layer | Partial (switcher theme.backdrop acrylic, taskbar coloringBlur) | DWM backdrop on Tourne windows | Approximate (Tourne surfaces only) | P3 |
| `blur_popups` | Blur its popups | Missing | Same, on Tourne menus | Approximate (Tourne surfaces only) | P3 |
| `dim_around` | Dim behind it | Missing | Dim overlay under a Tourne popup | Yes | P3 |
| `ignore_alpha` | Blur only opaque pixels | Missing | Tourne-drawn blur variants | Approximate (Tourne surfaces only) | P3 |
| `no_anim` | No animation | Partial (per-part switches) | Animations engine | Yes | P3 |
| `no_screen_share` | Hide from capture | Missing | SetWindowDisplayAffinity WDA_EXCLUDEFROMCAPTURE on Tourne windows | Yes | P3 |
| `order` | Space reservation order | Partial (taskbar reserves its edge) | SHAppBarMessage order between Tourne bars | Yes | P3 |
| `xray` | Blur x-ray | Missing | Tourne-drawn blur | Approximate (Tourne surfaces only) | P3 |

## 6. Workspace rules

Source: [configuring/core/rules/workspace-rules](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/rules/workspace-rules.md). Hybrid model: a Hyprland workspace is a Windows desktop.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `animation` | Workspace animation style | Not applicable | Windows animates desktop switches | No | - |
| `border_size` | Border size on this workspace | Missing | Per-desktop overlay override | Yes | P3 |
| `decorate` | Decorations on or off | Missing | Per-desktop overlay override | Yes | P3 |
| `default_name` | Default name | Missing | SetName when Tourne makes the desktop | Yes | P2 |
| `float_gaps` | Floating gaps | Missing | Per-desktop tiling override | Yes | P2 |
| `gaps_in` | Inner gaps | Missing (global tiling.gapInner) | Per-desktop override | Yes | P2 |
| `gaps_out` | Outer gaps | Missing (global tiling.gap*) | Per-desktop override | Yes | P2 |
| `layout` | Layout for this workspace | Exists (switcher.tiling.choices per screen and desktop) | Done; add it to the rules file | Yes | Done |
| `layout_opts` | Layout options for this workspace | Partial (main share per screen and desktop) | Per-desktop layout options | Yes | P2 |
| `monitor` | Bind workspace to a screen | Not applicable (hybrid model) | Desktops span screens | No | - |
| `default` | Default workspace of a screen | Not applicable | As monitor | No | - |
| `no_border` | No borders here | Missing | Per-desktop overlay override | Yes | P3 |
| `no_rounding` | No rounding here | Missing | Per-desktop corner preference | Approximate | P3 |
| `no_shadow` | No shadows here | Missing | Per-desktop overlay override | Yes | P3 |
| `on_created_empty` | Run something on a new empty workspace | Missing | Desktop created notification (IVirtualDesktopNotification) | Yes | P2 |
| `persistent` | Keep the workspace alive | Missing | Create missing desktops at start | Yes | P2 |

## 7. Layouts

Sources: [layouts/dwindle-layout](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/layouts/dwindle-layout.md), [master-layout](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/layouts/master-layout.md), [scrolling-layout](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/layouts/scrolling-layout.md), [monocle-layout](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/layouts/monocle-layout.md).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| **dwindle** | Binary tree: each new window splits the focused one | Partial (tiling `bsp`) | tiling-layouts.cpp gains a kept split tree per screen and desktop | Yes | P2 |
| dwindle `force_split` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `preserve_split` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `smart_split` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `smart_resizing` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `permanent_direction_override` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `special_scale_factor` | Dwindle option (see source) | Missing | Split tree option in tiling | Approximate | P2 |
| dwindle `split_width_multiplier` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `use_active_for_splits` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `default_split_ratio` | Dwindle option (see source) | Partial (tiling.mainRatio) | Split tree option in tiling | Yes | P2 |
| dwindle `split_bias` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle `precise_mouse_move` | Dwindle option (see source) | Missing | Split tree option in tiling | Yes | P2 |
| dwindle message `movetoroot` | Dwindle layout message | Missing | Bindable layout message | Yes | P2 |
| dwindle message `preselect` | Dwindle layout message | Missing | Bindable layout message | Yes | P2 |
| dwindle message `rotatesplit` | Dwindle layout message | Missing | Bindable layout message | Yes | P2 |
| dwindle message `splitratio` | Dwindle layout message | Missing | Bindable layout message | Yes | P2 |
| dwindle message `swapsplit` | Dwindle layout message | Missing | Bindable layout message | Yes | P2 |
| dwindle message `togglesplit` | Dwindle layout message | Missing | Bindable layout message | Yes | P2 |
| **master** | One or more masters plus a stack | Partial (verticalStack, rightMainVerticalStack, horizontalStack, ultrawideVerticalStack) | Named master layout with orientation | Yes | P2 |
| master `allow_small_split` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `special_scale_factor` | Master option (see source) | Missing | Master layout option | Approximate | P2 |
| master `mfact` | Master option (see source) | Exists (tiling.mainRatio) | Master layout option | Yes | Done |
| master `new_status` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `new_on_top` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `new_on_active` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `orientation` | Master option (see source) | Partial (the four stack layouts) | Master layout option | Yes | P2 |
| master `slave_count_for_center_master` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `center_master_fallback` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `smart_resizing` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `drop_at_cursor` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `always_keep_position` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master `focus_master_on_close` | Master option (see source) | Missing | Master layout option | Yes | P2 |
| master message `addmaster` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `cyclenext` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `cycleprev` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `focusmaster` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `mfact` | Master layout message | Partial (N, [ and ] chords) | Bindable layout message | Yes | P2 |
| master message `orientationbottom` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationcenter` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationcycle` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationleft` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationnext` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationprev` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationright` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `orientationtop` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `removemaster` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `rollnext` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `rollprev` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `swapnext` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `swapprev` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| master message `swapwithmaster` | Master layout message | Missing | Bindable layout message | Yes | P2 |
| **scrolling** | Columns on an endless horizontal strip (Niri style) | Missing | New layout: off-screen columns cloaked (DWMWA_CLOAK on Tourne-moved windows is not allowed for other apps; route is moving them beyond the work area, inferred)  | Approximate | P2 |
| scrolling `fullscreen_on_one_column` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `column_width` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `focus_fit_method` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `follow_focus` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `follow_min_visible` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `explicit_column_widths` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `wrap_focus` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `wrap_swapcol` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling `direction` | Scrolling option (see source) | Missing | Scrolling layout option | Yes | P2 |
| scrolling message `center` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `colresize` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `consume` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `consume_or_expel` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `expel` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `fit` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `fit_into_view` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `focus` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `inhibit_scroll` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `move` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `promote` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| scrolling message `swapcol` | Scrolling layout message | Missing | Bindable layout message | Yes | P2 |
| **monocle** | One window at a time | Exists (tiling `monocle`) | Done | Yes | Done |
| monocle message `cyclenext` / `cycleprev` | Cycle windows | Exists (monocle focus and move cycling) | Done | Yes | Done |
| (Tourne extras) columns, rows, grid | Layouts Hyprland lacks | Exists | tiling-layouts.cpp | Yes | Done |
| `layout.single_window_aspect_ratio` | Pad a lone window to an aspect ratio | Missing | Tiling option | Yes | P2 |
| Per-workspace layout | Different layout per workspace | Exists (per screen and desktop) | switcher.tiling.choices | Yes | Done |

## 8. Monitors

Source: [configuring/core/monitors/_index](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/monitors/_index.md) and its modes, positioning and output-selection pages.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `output / desc: selection` | Pick a screen by name or description | Exists (tiling keys screens by monitor device path) | QueryDisplayConfig names | Yes | Done |
| `mode (resolution@hz)` | Set resolution and refresh | Missing (a Windows setting) | SetDisplayConfig / ChangeDisplaySettingsEx | Yes | P6 |
| `position` | Arrange screens | Missing (a Windows setting) | SetDisplayConfig | Yes | P6 |
| `scale` | Scale factor | Missing (a Windows setting) | No public API; link to Settings (an undocumented route exists, inferred risky) | Approximate | P6 |
| `transform` | Rotate or flip | Missing (a Windows setting) | SetDisplayConfig rotation (no flip) | Approximate | P6 |
| `disabled` | Turn a screen off in the layout | Missing (a Windows setting) | SetDisplayConfig | Yes | P6 |
| `mirror` | Mirror a screen | Missing (a Windows setting) | SetDisplayConfig clone topology | Approximate | P6 |
| `bitdepth, cm, sdr_eotf, icc` | Colour depth and management | Missing (a Windows setting) | Windows HDR and colour profile settings | Approximate | P6 |
| `vrr` | Variable refresh | Missing (a Windows setting) | Windows VRR setting | Approximate | P6 |
| `HDR and luminance fields` | HDR tuning | Missing (a Windows setting) | Windows HDR calibration | Approximate | P6 |
| `reserved_area` | Reserve edge space | Partial (taskbar reserves its edge) | SHAppBarMessage for any Tourne bar | Yes | P3 |
| `Default workspace per monitor` | Workspace bound to a screen | Not applicable (hybrid model) | Desktops span screens | No | - |

## 9. Animations (every leaf, curve and style)

Source: [configuring/core/animations](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/animations.md); hyprfocus leaves from its plugin README.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `global` | Root of the tree | Missing | Animations engine: one easing module every part uses; master switch | Yes | P3 |
| `windows` | Window open, close, move (styles slide, popin, gnomed) | Not applicable | Other apps' windows are animated by DWM. Only Windows' own animation switch (SPI_SETCLIENTAREAANIMATION) | No | - |
| `windowsIn` | Window open | Not applicable | As windows | No | - |
| `windowsOut` | Window close | Not applicable | As windows | No | - |
| `windowsMove` | Window moving and resizing | Missing | Tiling eases windows into place with a few SetWindowPos steps (DeferWindowPos per frame). Heavy apps stutter (inferred). Off by default | Approximate | P3 |
| `layers` | Layer open and close (slide, popin, fade) | Partial (taskbar shyAnimation slide) | Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `layersIn` | Layer open | Partial | Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `layersOut` | Layer close | Partial | Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `fade` | Fades | Partial | Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `fadeIn` | Fade in on window open | Not applicable | Other windows | No | - |
| `fadeOut` | Fade out on window close | Not applicable | Other windows | No | - |
| `fadeSwitch` | Fade on focus change opacity | Missing | Only if Tourne opacity is on: step the layered alpha over a few frames | Approximate | P3 |
| `fadeShadow` | Shadow fade on focus change | Exists (borders.fade, fadeDuration) | Border overlay | Yes | Done |
| `fadeGlow` | Glow fade on focus change | Exists (borders.fade) | Border overlay | Yes | Done |
| `fadeDim` | Dimming fade | Missing | Dim overlay alpha steps | Yes | P3 |
| `fadeLayers` | Layer fades | Partial (floating pager fade) | Tourne surfaces | Yes | P3 |
| `fadeLayersIn` | Layer fade in | Partial | Tourne surfaces | Yes | P3 |
| `fadeLayersOut` | Layer fade out | Partial | Tourne surfaces | Yes | P3 |
| `fadePopups` | Popup fades | Missing | Tourne menus and flyouts | Yes | P3 |
| `fadePopupsIn` | Popup fade in | Missing | Tourne menus | Yes | P3 |
| `fadePopupsOut` | Popup fade out | Missing | Tourne menus | Yes | P3 |
| `fadeDpms` | Fade when screens turn off | Missing | Full-screen black Tourne overlay fades in, then screens off | Approximate | P5 |
| `border` | Border colour change | Exists (borders.fade) | Border overlay | Yes | Done |
| `borderangle` | Gradient angle (once, loop) | Exists (borders.activeSpin, spinDuration) | Border overlay | Yes | Done |
| `shadowangle` | Shadow gradient angle | Missing | Border overlay | Yes | P3 |
| `glowangle` | Glow gradient angle | Missing | Border overlay | Yes | P3 |
| `workspaces` | Workspace switch (slide, slidevert, fade, slidefade, slidefadevert) | Not applicable (hybrid model) | Windows animates desktop switches; Tourne cannot replace it without its own workspaces | No | - |
| `workspacesIn` | Workspace in | Not applicable | As workspaces | No | - |
| `workspacesOut` | Workspace out | Not applicable | As workspaces | No | - |
| `specialWorkspace` | Scratchpad show and hide | Missing | Scratchpad is Tourne's: slide or fade its dim layer; the windows themselves just show | Approximate | P3 |
| `specialWorkspaceIn` | Scratchpad in | Missing | As specialWorkspace | Approximate | P3 |
| `specialWorkspaceOut` | Scratchpad out | Missing | As specialWorkspace | Approximate | P3 |
| `zoomFactor` | Screen zoom | Missing | MagSetFullscreenTransform in steps | Yes | P5 |
| `monitorAdded` | Monitor added zoom | Not applicable | Compositor | No | - |
| `hyprfocusIn / hyprfocusOut (plugin)` | Flash on focus | Missing | Border overlay flash or shrink on focus | Yes | P3 |
| `Curve: bezier` | Cubic bezier easing | Missing (fixed easing per part) | Animations engine | Yes | P3 |
| `Curve: spring (mass, stiffness, dampening; critical, over, under damped)` | Physics easing | Missing | Animations engine | Yes | P3 |
| `Style: popin N%` | Pop in from N percent | Missing | Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `Style: slide N%` | Slide by N percent | Partial (taskbar shy slide) | Tourne surfaces | Approximate (Tourne surfaces only) | P3 |
| `animations speed (deciseconds)` | Duration per leaf | Partial (per-part durations: shyDuration, fadeDuration, spinDuration) | One unit across parts | Yes | P3 |

## 10. Decorations

Source: config-options (decoration groups) and the plugin READMEs.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `Borders (size, colour, gradient, angle)` | Window borders | Exists (borders.cpp, Tacky Borders style; tiling.focusBorder via DWMWA_BORDER_COLOR) | Overlay windows, no hook | Yes | Done |
| `Rounding` | Rounded corners | Partial (borders.radius; Windows 11 rounds windows) | DWMWA_WINDOW_CORNER_PREFERENCE | Approximate | P3 |
| `Shadows` | Drop shadows | Partial (borders.shadow) | Overlay; DWM shadow stays | Approximate | P3 |
| `Glow (inner)` | Glow | Partial (borders.glow is outer) | Inner glow is inside other windows: not drawable. Outer only | Approximate | P3 |
| `Blur (kawase) and variants (acrylic, aurora, drops, fluid_jar, glass, haze, heat_shimmer, ripple, water)` | Background blur | Partial (acrylic backdrop on switcher, taskbar coloringBlur) | Other apps: DWMWA_SYSTEMBACKDROP_TYPE where the app allows; Tourne surfaces: any | Approximate | P3 |
| `Dim inactive, dim around, dim modal, dim special` | Dimming | Missing | Click-through dim overlays | Approximate | P3 |
| `Opacity active / inactive / fullscreen` | Transparency | Missing | SetLayeredWindowAttributes | Approximate | P3 |
| `screen_shader` | Whole screen shader | Missing | Magnification colour matrix only (no custom GLSL) | Approximate | P5 |
| `Motion blur, wobble` | Effects on moving windows | Not applicable | Needs the compositor | No | - |
| `Group bars` | Tab strips | Missing | Groups part | Yes | P4 |
| `hyprbars (plugin): title bars with buttons` | Custom title bars | Missing | An overlay bar glued above a window (like the border overlay), optional caption removal. Inferred: works for Win32, not for apps that draw their own caption (Chrome, UWP) | Approximate | P4 |
| `borders-plus-plus (plugin): extra borders` | One or two more borders | Partial (borders.cpp draws one gradient border with glow and shadow) | More rings in the overlay | Yes | P3 |

## 11. Gestures

Source: [configuring/core/binds/gestures](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/binds/gestures.md).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `gesture (fingers, direction, action)` | Touchpad gestures | Missing | Windows gesture set to Nothing, then raw HID touchpad contacts (usage page 0x0D) read only while touched | Approximate | P6 |
| `direction swipe / horizontal / vertical / left / right / up / down` | Swipe directions | Missing | Raw touchpad input | Approximate | P6 |
| `direction pinch / pinchin / pinchout` | Pinch | Missing | Raw touchpad input (or WM_GESTURE on touch screens) | Approximate | P6 |
| `mods, scale, disable_inhibit` | Gesture modifiers and speed | Missing | Gesture settings | Yes | P6 |
| `action workspace` | Swipe between workspaces | Partial (Windows 4 finger swipe switches desktops) | Windows does this natively; Tourne can add wraparound | Yes | P6 |
| `action special` | Toggle scratchpad | Missing | Scratchpad | Yes | P6 |
| `action close / float / fullscreen / move / resize` | Window actions | Missing | Dispatchers | Yes | P6 |
| `action cursor_zoom` | Zoom at the pointer | Missing | Magnification API | Yes | P6 |
| `action scroll_move` | Scroll the strip | Missing | Scrolling layout | Yes | P6 |
| `Lua and live gestures` | Scripted gestures following the fingers | Missing | Live only for Tourne surfaces and zoom | Approximate | P6 |
| `Touchscreen edge swipe (workspace_swipe_touch)` | Edge swipe | Not applicable | Windows owns edge swipes | No | - |

## 12. IPC and hyprctl

Sources: [ipc/_index](https://github.com/hyprwm/hyprland-wiki/blob/main/content/ipc/_index.md), [using-hyprctl](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/advanced-configuration/using-hyprctl.md), [events](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/advanced-configuration/events.md), [notifications](https://github.com/hyprwm/hyprland-wiki/blob/main/content/configuring/core/advanced-configuration/notifications.md).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `hyprctl (CLI)` | Command line control | Partial (TourneDesktop.exe --import, --install, --stop) | `tournectl` CLI talking to a named pipe; never needed by users | Yes | P1 |
| `hyprctl dispatch` | Run a dispatcher | Missing | tournectl dispatch | Yes | P1 |
| `hyprctl reload` | Reload config | Exists (on save) | tournectl reload | Yes | P1 |
| `hyprctl eval / keyword` | Set options at runtime | Missing | tournectl set key value (not saved) | Yes | P1 |
| `hyprctl repl` | Interactive prompt | Missing | Low value; skip | Yes | - |
| `hyprctl kill` | Click to kill a window | Missing | Pick mode then TerminateProcess | Yes | P1 |
| `hyprctl setcursor` | Set cursor theme and size | Missing | Pointer scheme switch (SystemParametersInfo SPI_SETCURSORS) | Yes | P5 |
| `hyprctl output` | Make headless outputs | Not applicable | Virtual display drivers only | No | - |
| `hyprctl switchxkblayout` | Change keyboard layout | Missing | ActivateKeyboardLayout | Yes | P1 |
| `hyprctl seterror` | Show an error bar | Missing | Tourne toast | Yes | P1 |
| `hyprctl getprop` | Read a window property | Missing | tournectl get | Yes | P1 |
| `hyprctl notify / dismissnotify` | Compositor notifications | Missing | Tourne toasts | Yes | P1 |
| `Info queries (clients, workspaces, monitors, activewindow, layers, devices, binds, version)` | State as text or JSON | Missing | tournectl with --json | Yes | P1 |
| `--batch, -j flags` | Batch and JSON | Missing | tournectl flags | Yes | P1 |
| `Socket 1 (requests)` | Request socket | Missing | Named pipe \\.\pipe\Tourne\<session> | Yes | P1 |
| `Socket 2 (events)` | Event stream | Missing | Second named pipe; events from WinEvents Tourne already listens to | Yes | P1 |

### Socket 2 events (one row each)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| event `workspace` | Socket 2 event (see source) | Missing | Desktop switch (IVirtualDesktopNotification) | Yes | P1 |
| event `workspacev2` | Socket 2 event (see source) | Missing | Desktop switch (IVirtualDesktopNotification) | Yes | P1 |
| event `focusedmon` | Socket 2 event (see source) | Missing | Foreground window's screen changes | Yes | P1 |
| event `focusedmonv2` | Socket 2 event (see source) | Missing | Foreground window's screen changes | Yes | P1 |
| event `activewindow` | Socket 2 event (see source) | Missing | EVENT_SYSTEM_FOREGROUND | Yes | P1 |
| event `activewindowv2` | Socket 2 event (see source) | Missing | EVENT_SYSTEM_FOREGROUND | Yes | P1 |
| event `fullscreen` | Socket 2 event (see source) | Missing | Fullscreen detection (taskbar already has it) | Yes | P1 |
| event `monitorremoved` | Socket 2 event (see source) | Missing | WM_DISPLAYCHANGE / device notifications | Yes | P1 |
| event `monitorremovedv2` | Socket 2 event (see source) | Missing | WM_DISPLAYCHANGE / device notifications | Yes | P1 |
| event `monitoradded` | Socket 2 event (see source) | Missing | WM_DISPLAYCHANGE / device notifications | Yes | P1 |
| event `monitoraddedv2` | Socket 2 event (see source) | Missing | WM_DISPLAYCHANGE / device notifications | Yes | P1 |
| event `createworkspace` | Socket 2 event (see source) | Missing | Desktop created notification | Yes | P1 |
| event `createworkspacev2` | Socket 2 event (see source) | Missing | Desktop created notification | Yes | P1 |
| event `destroyworkspace` | Socket 2 event (see source) | Missing | Desktop destroyed notification | Yes | P1 |
| event `destroyworkspacev2` | Socket 2 event (see source) | Missing | Desktop destroyed notification | Yes | P1 |
| event `moveworkspace` | Socket 2 event (see source) | Not applicable | Not applicable (desktops span screens) | No | - |
| event `moveworkspacev2` | Socket 2 event (see source) | Not applicable | Not applicable (desktops span screens) | No | - |
| event `renameworkspace` | Socket 2 event (see source) | Missing | Desktop renamed notification | Yes | P1 |
| event `activespecial` | Socket 2 event (see source) | Missing | Scratchpad shown | Yes | P1 |
| event `activespecialv2` | Socket 2 event (see source) | Missing | Scratchpad shown | Yes | P1 |
| event `activelayout` | Socket 2 event (see source) | Missing | WM_INPUTLANGCHANGE on the foreground | Yes | P1 |
| event `openwindow` | Socket 2 event (see source) | Missing | EVENT_OBJECT_SHOW (top-level) | Yes | P1 |
| event `closewindow` | Socket 2 event (see source) | Missing | EVENT_OBJECT_DESTROY | Yes | P1 |
| event `kill` | Socket 2 event (see source) | Missing | Tourne kill | Yes | P1 |
| event `movewindow` | Socket 2 event (see source) | Missing | View moved to desktop notification | Yes | P1 |
| event `movewindowv2` | Socket 2 event (see source) | Missing | View moved to desktop notification | Yes | P1 |
| event `openlayer` | Socket 2 event (see source) | Missing | Tourne surface shown | Yes | P1 |
| event `closelayer` | Socket 2 event (see source) | Missing | Tourne surface hidden | Yes | P1 |
| event `submap` | Socket 2 event (see source) | Missing | Submap change | Yes | P1 |
| event `changefloatingmode` | Socket 2 event (see source) | Missing | Tiling float toggle | Yes | P1 |
| event `urgent` | Socket 2 event (see source) | Missing | HSHELL_FLASH | Yes | P1 |
| event `screencast` | Socket 2 event (see source) | Not applicable | Not knowable reliably (inferred: capture apps don't announce) | No | - |
| event `screencastv2` | Socket 2 event (see source) | Not applicable | Not knowable reliably (inferred: capture apps don't announce) | No | - |
| event `windowtitle` | Socket 2 event (see source) | Missing | EVENT_OBJECT_NAMECHANGE | Yes | P1 |
| event `windowtitlev2` | Socket 2 event (see source) | Missing | EVENT_OBJECT_NAMECHANGE | Yes | P1 |
| event `togglegroup` | Socket 2 event (see source) | Missing | Groups | Yes | P4 |
| event `moveintogroup` | Socket 2 event (see source) | Missing | Groups | Yes | P4 |
| event `moveoutofgroup` | Socket 2 event (see source) | Missing | Groups | Yes | P4 |
| event `ignoregrouplock` | Socket 2 event (see source) | Missing | Groups | Yes | P4 |
| event `lockgroups` | Socket 2 event (see source) | Missing | Groups | Yes | P4 |
| event `configreloaded` | Socket 2 event (see source) | Missing | tourne.conf applied | Yes | P1 |
| event `pin` | Socket 2 event (see source) | Missing | Pin dispatcher | Yes | P1 |
| event `minimized` | Socket 2 event (see source) | Missing | EVENT_SYSTEM_MINIMIZESTART/END | Yes | P1 |
| event `bell` | Socket 2 event (see source) | Not applicable | Not knowable (inferred) | No | - |

### Lua events (one row each)

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `config.props_refreshed` | Event for scripts and rules (see source) | Not applicable | No Windows equivalent or only a Lua internal | No | - |
| `config.reloaded` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `config.unload` | Event for scripts and rules (see source) | Not applicable | No Windows equivalent or only a Lua internal | No | - |
| `hyprland.shutdown` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `hyprland.start` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `input.keyboard.key` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `keybinds.submap` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `layer.closed` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `layer.opened` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `monitor.added` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `monitor.focused` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `monitor.layout_changed` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `monitor.removed` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `screenshare.state` | Event for scripts and rules (see source) | Not applicable | No Windows equivalent or only a Lua internal | No | - |
| `window.active` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.bell` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.class` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.close` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.destroy` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.fullscreen` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.kill` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.minimize` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.move_to_workspace` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.open` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.open_early` | Event for scripts and rules (see source) | Not applicable | No Windows equivalent or only a Lua internal | No | - |
| `window.pin` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.title` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.update_rules` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `window.urgent` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `workspace.active` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `workspace.created` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `workspace.move_to_monitor` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `workspace.removed` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |
| `workspace.special_active` | Event for scripts and rules (see source) | Missing | Same event offered to rules and the event pipe | Yes | P1 |

## 13. Plugins and Pyprland

Sources: [hyprland-plugins README](https://github.com/hyprwm/hyprland-plugins) (its plugin list now names borders-plus-plus, csgo-vulkan-fix, hyprbars and hyprfocus; the others below are from memory of earlier versions and marked inferred), [Pyprland README](https://github.com/hyprland-community/pyprland) (plugin names from its change history; list inferred as not complete).

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `hyprpm / plugin loading` | Install and load plugins | Missing | Tourne pieces: modules built into Tourne Desktop plus a pieces folder; sets format describes them | Yes | P7 |
| `borders-plus-plus` | Extra borders | Partial (borders.cpp) | More overlay rings | Yes | P3 |
| `csgo-vulkan-fix` | Fake resolution for a game | Not applicable | Games on Windows pick resolution themselves; GPU control panels do custom resolutions | No | - |
| `hyprbars` | Title bars | Missing | Overlay title bar | Approximate | P4 |
| `hyprfocus` | Flash on focus | Missing | Overlay flash, shrink or fade | Yes | P3 |
| `hyprexpo (inferred: no longer in the plugin repo's main branch)` | Workspace overview grid | Partial (taskbar previews, pager) | Overview window with DWM thumbnails (DwmRegisterThumbnail) of each desktop's windows; Windows only renders windows of the current desktop live (inferred), so other desktops show cached shots | Approximate | P4 |
| `hyprtrails (inferred, as above)` | Trails behind moving windows | Not applicable | Needs the compositor | No | - |
| `hyprwinwrap (inferred, as above)` | A window as the wallpaper | Missing | Parent a window under WorkerW behind desktop icons (the known live wallpaper route) | Yes | P5 |
| `xtra-dispatchers (inferred, as above)` | Extra dispatchers (e.g. close all) | Missing | More dispatchers | Yes | P1 |
| `hyprscrolling (inferred: now the built-in scrolling layout)` | Scrolling layout | Missing | Scrolling layout | Approximate | P2 |
| `Pyprland: scratchpads` | Dropdown app windows (animated, per app) | Missing | Scratchpad part with per-app slide-in | Yes | P2 |
| `Pyprland: expose` | Show all windows | Partial (Switcher, Task view) | Overview | Yes | P4 |
| `Pyprland: magnify` | Zoom | Missing | Magnification API | Yes | P5 |
| `Pyprland: monitors` | Place screens by rules when plugged | Missing | SetDisplayConfig on device arrival | Yes | P6 |
| `Pyprland: shortcuts_menu, fetch_client_menu` | Popup menus of commands and windows | Partial (tray menu, Switcher) | Tourne launcher modules | Yes | P5 |
| `Pyprland: layout_center` | Centre the focused window with others around | Missing | Tiling layout | Yes | P2 |
| `Pyprland: lost_windows` | Bring off-screen windows back | Missing | misc.float_force_onscreen route | Yes | P2 |
| `Pyprland: toggle_dpms` | Screens off toggle | Missing | dpms dispatcher | Yes | P1 |
| `Pyprland: workspaces_follow_focus` | Workspace follows focused screen | Not applicable (hybrid model) | Desktops span screens | No | - |
| `Pyprland: system_notifier` | Notifications from log watches | Missing | Tourne toasts | Yes | P5 |
| `Pyprland: wallpapers` | Wallpaper rotation with rounded corners | Missing | Wallpaper manager | Yes | P5 |
| `Pyprland: fcitx5_switcher` | Input method switch | Not applicable | Windows IME | No | - |

## 14. Hypr ecosystem tools

Source: [hypr-ecosystem/_index](https://github.com/hyprwm/hyprland-wiki/blob/main/content/hypr-ecosystem/_index.md) and each tool's page under `hypr-ecosystem/user/`.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `hyprpaper` | Wallpaper daemon (per monitor, preload, IPC) | Partial (theme engine reads the wallpaper; Windows sets it) | Wallpaper manager with recolourer: IDesktopWallpaper per screen (proposal written) | Yes | P5 (first) |
| `hypridle` | Idle actions (dim, lock, screens off, suspend) | Partial (taskbar idleFade for the bar only) | One timer to the next step; GetLastInputInfo read when it fires; inhibited by idle_inhibit rules | Yes | P5 |
| `hyprlock` | Lock screen with widgets | Missing | Windows owns the lock screen. Theme its picture; an optional "lock overlay" before LockWorkStation is cosmetic only, not security (inferred) | Approximate | P5 |
| `hyprpicker` | Colour picker | Missing | Frozen screenshot with loupe, copy hex, send to theme seed | Yes | P5 |
| `hyprsunset` | Blue light filter | Missing (Windows Night light) | Night light switch, or theme-tinted gamma ramp (SetDeviceGammaRamp) on a schedule | Yes | P5 |
| `hyprpolkitagent` | Admin prompts | Not applicable | UAC | No | - |
| `hyprsysteminfo` | System info window | Missing | Tourne about page | Yes | P7 |
| `hyprland-qt-support, hyprqt6engine` | Qt app styling | Partial (CSS injector for Electron apps) | Per-app templates (auto theming plan) | Approximate | P5 |
| `hyprcursor` | Cursor theme format | Missing | Windows pointer schemes; sets carry one | Yes | P5 |
| `hyprlauncher` | Launcher and pickers | Partial (start-menu.cpp) | Tourne launcher: apps, calculator, windows, emoji, commands | Yes | P5 |
| `hyprshutdown` | Graceful quit screen | Not applicable | Windows shutdown | No | - |
| `hyprpwcenter` | Audio control centre | Partial (tray volume flyout, system-icons-host) | Windows sound settings for routing | Approximate | - |
| `hyprtoolkit, hyprland-guiutils` | GUI toolkit and dialogs | Partial (Tourne's own drawing code) | Shared Tourne UI kit | Yes | P5 |
| `xdg-desktop-portal-hyprland` | Screen share and pickers | Not applicable | Windows has these | No | - |
| `hyprshot (third party)` | Screenshots of window, region, screen | Missing (Snipping Tool exists) | Tourne screenshot tool: region, window, screen, to file and clipboard | Yes | P5 |

## 15. Useful utilities and other pages

Source: [useful-utilities/_index](https://github.com/hyprwm/hyprland-wiki/blob/main/content/useful-utilities/_index.md) and its pages; getting-started, nix and faq pages only briefly.

| Hyprland feature | What it does | Tourne today | Windows route | Possible? | Phase |
| --- | --- | --- | --- | --- | --- |
| `Status bars (Waybar, eww, AGS, Quickshell, ...)` | Bars and widgets | Exists (Tourne taskbar; YASB support) | Taskbar and future top bar | Yes | Done |
| `App launchers (hyprlauncher, rofi, wofi, fuzzel, walker, anyrun, tofi)` | Launch apps | Partial (start-menu.cpp) | Launcher part | Yes | P5 |
| `Wallpapers (hyprpaper, swaybg, wpaperd, mpvpaper, awww, waypaper, wallrizz)` | Static, animated, video wallpapers | Missing | Wallpaper manager; video via WorkerW window, paused when covered | Yes | P5 |
| `Notification daemons (mako, dunst, swaync, fnott)` | Notifications | Missing | Windows notifications stay. A Tourne notification centre needs UserNotificationListener (an app identity; inferred feasible from a packaged helper) | Approximate | P5 |
| `OSDs (swayosd, avizo)` | Volume and brightness popups | Missing | Tourne OSD on media keys (WM_APPCOMMAND seen without a hook via the shell hook, inferred) and hiding Windows' own flyout | Approximate | P5 |
| `Clipboard managers (cliphist, clipse, copyq)` | Clipboard history | Missing (Win+V exists) | AddClipboardFormatListener history with a picker | Yes | P5 |
| `Colour pickers (hyprpicker)` | Pick a screen colour | Missing | Colour picker part | Yes | P5 |
| `Screenshots and recording (grim, slurp, hyprshot, wf-recorder, OBS)` | Capture | Missing | Screenshot tool (Windows.Graphics.Capture); recording left to OBS and Game Bar | Yes | P5 |
| `Screen sharing (PipeWire, portal)` | Share screens in calls | Not applicable | Windows does this | No | - |
| `File managers (dolphin, nautilus, thunar, yazi...)` | Files | Partial (Explorer styling parts) | Explorer stays; the suite themes it | Yes | Done |
| `Must-have (polkit agent, portals, qt and gtk theming, auth agent)` | Session plumbing | Not applicable | Windows | No | - |
| `Other (workspace managers, pyprland)` | Extras | Missing | See Plugins | Yes | P2 |
| `UWSM, systemd start` | Session manager | Not applicable | Tourne Desktop --install starts with Windows | No | - |
| `Nix, installation, preconfigured setups, FAQ` | Install guides | Not applicable | Tourne installs with one click (Install Tourne Desktop.cmd) | No | - |
| `Getting started: Master tutorial` | First run guide | Missing | A first-run page in the settings window: pick a set, layout, keys (all off until chosen) | Yes | P1 |
| `Uncommon tips and code snippets` | Recipes (toggle animations, zoom, minimize to special, etc.) | Missing | Each recipe becomes a ready-made bind or piece | Yes | P7 |
| `Tearing, Multi-GPU, XWayland, Performance pages` | Platform tuning | Not applicable | Windows graphics settings | No | - |

## The ecosystem: Tourne parts and pieces

Each is a Tourne Desktop module (hook-free) unless it says Windhawk. Each covers the wiki areas named.

| Part | Covers from the wiki | Exists today |
| --- | --- | --- |
| **Tiling window manager** | general (gaps, layout), dwindle, master, scrolling, monocle, layout messages, pseudotile, swap and move dispatchers | Partial: tiling.cpp, tiling-layouts.cpp (columns, rows, stacks, grid, bsp, monocle), per screen and desktop choices |
| **Workspaces and overview** | focus and move to workspace, rename, persistent, back and forth, pin, workspace rules, hyprexpo, expose | Partial: taskbar desktopKeys, desktops-pager.cpp; Windows Task view |
| **Scratchpads** | special workspaces, toggle_special, dim_special, Pyprland scratchpads | Missing |
| **Binds engine with submaps** | binds, flags, submaps, mouse binds, keyboard layouts, cheat sheet | Partial: per-part hotkeys (tiling.leader, desktopKeys, hideHotkey) |
| **Window rules engine** | window rules (matchers, static, dynamic), tags, set_prop, swallow, workspace rules | Partial: tiling float lists, borders skip lists |
| **Decorations** | borders, glow, shadow, rounding, dim, opacity, borders-plus-plus, hyprfocus, hyprbars | Partial: borders.cpp |
| **Groups (tabbed windows)** | group options, groupbar, group dispatchers | Missing |
| **Animations engine** | animation tree for Tourne surfaces, bezier and spring curves, window move easing | Partial: per-part fades and slides |
| **Bar** | status bars | Exists: taskbar (Windhawk and Tourne Desktop), YASB support |
| **Launcher** | hyprlauncher, rofi-likes, shortcuts menus | Partial: start-menu.cpp (Windhawk) |
| **Notifications centre and toasts** | notification daemons, hyprctl notify, config errors | Missing |
| **OSD** | swayosd-like volume and brightness popups | Missing |
| **Clipboard manager** | cliphist-likes | Missing (Win+V) |
| **Colour picker** | hyprpicker | Missing |
| **Screenshot tool** | hyprshot, grim and slurp | Missing (Snipping Tool) |
| **Wallpaper manager with recolourer** | hyprpaper, wallpaper tools, hyprwinwrap, Pyprland wallpapers | Missing; proposal written (`TourneSuite wallpaper manager proposal.md`); theme engine reads wallpapers |
| **Idle, night light, lock overlay, zoom** | hypridle, hyprsunset, hyprlock (look only), cursor zoom, dpms | Partial: taskbar idleFade |
| **Cursor themes** | hyprcursor, cursor options | Missing |
| **Theme engine and app styling** | colours across everything, qt support (as app templates) | Exists: theme-engine.cpp, palette-gen.cpp, CSS injector |
| **IPC and `tournectl`** | hyprctl, sockets, events | Missing (only exe flags) |
| **Settings window** | every Windows-setting row (input, monitors), first-run guide, no terminal | Missing (tourne.conf plus tray menu today) |
| **Config language** | sourcing, variables, expressions, autostart, error toasts | Partial: tourne.conf with live reload |
| **Plugin API (pieces and sets)** | hyprpm, plugins, sets | Missing; format not written yet (handoff) |

## Build order

Starting from what exists: taskbar, switcher with tiling and borders, pager, theme engine, CSS injector, Tourne Desktop host.

| Phase | What gets built | How the user's day changes |
| --- | --- | --- |
| **P1 Control** | Binds engine with submaps and flags; window rules engine (matchers, static effects, tags); simple dispatchers; IPC pipe and `tournectl`; events; config sourcing, variables, expressions, autostart; error toasts; first-run page | One place to bind any action; apps open where and how the user wants; bars and scripts can follow Tourne. Nothing changes until the user sets it |
| **P2 Workspaces and layouts** | Scratchpads; pin, rename, persistent desktops; dwindle and master messages; scrolling layout; mouse move and resize binds; snapping; off-screen rescue; focus and warp options; swallow | Feels like Hyprland's window handling on top of Windows desktops |
| **P3 Look and motion** | Animations engine with bezier and spring; dim inactive and around; opacity; per-window border rules; shadow and glow options; corner preference; Tourne surface blur variants; hyprfocus flash | The look of a riced Hyprland setup, within DWM's limits |
| **P4 Groups and overview** | Tab groups with a tab strip; overview of all desktops; title bars (hyprbars) | Tabbed windows and an expo view |
| **P5 Ecosystem parts** | Wallpaper manager with recolourer (first, in parallel with P1); idle actions; night tint; zoom; colour picker; screenshot tool; launcher modules; OSD; clipboard manager; notification centre; cursor themes; app templates | The rest of a Hyprland desktop, each part optional |
| **P6 Input, monitors, gestures** | Settings window pages for every Windows setting row; touchpad gestures; monitor rules on plug | No more digging through Windows Settings |
| **P7 Pieces and sets** | Plugin API and sets format; recipes from "uncommon tips" as ready-made pieces; about page | Users make and share whole setups |
