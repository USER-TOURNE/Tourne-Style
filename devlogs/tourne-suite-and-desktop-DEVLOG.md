# Devlog: our own taskbar, builds b8 to b51

Two days (2026-09-28 and 09-29) spent turning the test-mode taskbar into a 1:1 stand-in for StartAllBack (SAB). The bar and Start menu are drawn straight from SAB's `.msstyles`, so a theme made for SAB should look the same in both. Every change below was checked against SAB side by side where that was possible.

## How it's built and checked

- **One file per build.** `builds/tourne-suite-bN.wh.cpp` is generated from the parts, with its own mod id (`local@tourne-suite-bN`), so a new build installs next to the old one. Settings are copied from whichever build is running, so nothing has to be set up again.
- **A test bar outside Windhawk.** The same code runs as a plain exe (`bartest`), driven by `TB_*` environment variables, so a change can be photographed and compared without touching the live taskbar. `Compare-Matrix` shoots SAB and ours for four styles (Default, Bouquet SAB, Plain8, Windows 7), idle and hovered, and diffs them.
- **Ground truth is a screenshot from the real screen.** Our own BitBlt and DXGI photos show SAB's acrylic areas as copper, which they are not. Colours were measured from real screenshots only.
- Both x64 and x86 are compile-checked for every build.

![Ours against StartAllBack, Bouquet SAB](images/18-parity-bouquet.png)

Rows one and three are ours (idle, hovered), rows two and four are StartAllBack. The orb differs only because the test bar doesn't load SAB's orb. The Windows 7 style, same layout:

![Ours against StartAllBack, Windows 7](images/19-parity-windows7.png)

## b8 to b10: matching the look

- **Follows StartAllBack live.** Changing SAB's style, orb or colours now reloads our bar at once. Before, the two drifted apart until a restart.
- **Bar frame.** A 3 px frame (#1E2E36) around the inner #121C21, measured from SAB, kept out of the custom colouring pass.
- **Button frame and glow.** SAB draws a button's frame 28 px high, 4 px in from the side facing the screen's centre, and keeps the hover glow inside it. Ours now does the same, with our own dynamic aura kept on top.
- **Start menu.** Video-frame comparison against SAB: row height 22, right column pitch 29 px, separator moved up 4 px, cue text colour, search focus, and the capitals setting.

## b9 to b11: lightning fast

The complaint was that right clicks and the Start menu felt heavy. What changed:

- The apps list, folder contents and jump lists are read on background threads and cached, and the caches are dropped when settings or pins change. Hovering a button reads its jump list ahead of the right click.
- The Start menu keeps its back buffers and repaints only the rows that changed. Its home rows are built once while idle.
- Windows' own fade and menu animations are switched off for our popups (`DWMWA_TRANSITIONS_FORCEDISABLED`, `TPM_NOANIMATION`), and menu icons are filled in lazily.

## b12 to b14: the details you notice

- **Click a group.** Clicking a button with several windows brings up the last one you used (Ctrl+click still lists them).
- **Tooltips keep their lines.** Multi-line tray tooltips are no longer squashed into one, and the wrongly sized ones are fixed.
- **Menu frame.** Right-click menus and every shell menu and submenu they open get SAB's double frame (a 1 px line, a 6 px band, another line, then the interior), with the row and text colours measured from Bouquet SAB.
- **Menu icons** come from the icon theme's own imageres.dll (pin, shredder, X), and there's a new **End task** entry.
- **Stacked windows.** A group is drawn as a stack, its edge peeking out behind the button, using the theme's own stack parts (13 for a button, 14 for the active one; state 2 for two windows, 3 for more), with a little room so it doesn't run into the next button.

| Flyout menu | The stack parts, from Bouquet SAB |
|---|---|
| ![Flyout menu](images/20-flyout-menu.png) | ![Stack edge](images/21-stack-edge.png) |

The style's band class, as far as it's decoded (every state of every part, drawn from the file with no lock on it):

![Band parts](images/22-band-parts.png)

Parts 1 to 4 are the bar background, 5 the button (normal, hot, pressed, active, active hot, active pressed, flashing), 6 to 8 its variants, 9 to 12 the progress bar, 13 and 14 the stack edge.

## What happens with StartAllBack switched off

Our tray already hosts the icons itself (it takes them from the tray notifications directly, not from SAB), and the bar and Start menu are ours. With SAB off, Windows' own taskbar comes back, which "Hide StartAllBack's taskbar" plus "Turn StartAllBack off" handle today. What SAB did to Explorer itself (its restyling) is still open.

## b15: four ideas from studying YASB

YASB (a Windows status bar) has four things worth having. All four are in.

- **A bar per screen.** Every other screen gets a bar with its own windows and the clock. The main bar keeps the Start button, the tray and the watching of windows. Which windows go where follows Windows' "Show taskbar buttons on" (all bars, main plus where open, only where open), and pinned apps stay on the main bar unless Windows shares every button everywhere. New setting: *A bar on every screen* (as Windows is set, always, never). Screens plugged in or removed are picked up live.
- **Hot reload.** Save a theme file, drop one in the Styles or Orbs folder, or change something in StartAllBack, and the bar reloads about 0.4 s later. A burst of changes is one reload.
- **Theme install.** Drop a `.msstyles`, an orb picture, a folder or a `.zip` on the bar, or use *Install a theme or orb...* in its menu. Themes go to `%LOCALAPPDATA%\Tourne\Styles`, orbs to the orbs folder, and a small toast says what was installed.
- **Hover glow fade.** The glow fades in over about 90 ms and out over about 140 ms, from wherever it had got to, like SAB.

![The three screens' bars](images/23-multi-monitor.png)

Main bar on top; then the bar of the second screen (a Discord window and one more) and the third (nothing open there yet). Each has its own clock.

**How it's done.** The drawing code works on globals, so each bar keeps its own copy of the few that differ (buttons, hover state, tooltips, tray, layout) and they're swapped in when a message arrives for that bar and out again after, which keeps menus and previews on one bar from confusing another. The test bar turned three screens on and off live, and the hot reload and zip install were run in it too.

## b16: the other screens, finished

The gaps left after b15 are closed, each one tried in the test bar on the three screens:

- **Each screen has its own scale.** Sizes, fonts and the clock are worked out per bar from its screen's DPI, so a 200% screen next to a 100% one gets its own sizes (the scale is a whole number, so 100% and 125% are the same size, as on the main bar). A bar's popups (previews, menus) follow its scale too. Tested by setting one bar to 2x: its buttons came out twice the size and the main bar didn't change; a reload put it back.
- **Menus, dragging, hiding, dialog and drop** all work on the other bars: the button's jump list and the bar's own menu open on that screen and close cleanly; dragging a button saves its order; auto-hide slides a bar to its 2 px edge and back when the pointer reaches it; the install dialog opens and closes; a `.msstyles` dropped on it is installed.
- **Your settings are the defaults.** The build script now takes its defaults from the build that is switched on right now (the highest numbered enabled one) unless told otherwise, so each new build starts from what you use, button icons included.

## Research: can YASB themes be ported?

A YASB theme is a `styles.css` plus a `config.yaml` (which widgets go left, center and right, and their label formats). To see what a port would need, all 68 themes in [amnweb/yasb-themes](https://github.com/amnweb/yasb-themes) were read.

- **Variables are the norm:** 53 of 68 use `:root` custom properties and `var()`; 78% of themes.
- **Popups are most of the CSS.** 39% of rules are about the bar; the rest style popups (calendar, media, control centre) and Qt sub-controls (`::item`, `::groove`), which a bar doesn't need at first.
- **Rare:** gradients (2 themes), `box-shadow` (2), `transition` (11: `background-color` and `opacity`, 0.08 to 0.25 s).
- **They are status bars.** Clock is in all 68, volume in 65, media 58, power menu 56, workspaces 51, weather 48; only 20 show taskbar buttons.
- **Icons are font glyphs** (Nerd Font, Segoe Fluent Icons), so the look depends on the font installed.

Full numbers in `research/YASB-SURVEY.md` in the working folder.

## The CSS spike

A small CSS engine, a config reader and a software renderer, in the same style as the rest of the bar (no libraries, GDI only, x64 and x86):

- `css-engine.cpp`: parser, selectors (classes, descendants, `>`, `:hover` and friends), the cascade with specificity, `var()`, shorthands, gradients, transitions read.
- `css-draw.cpp`: rounded boxes with borders and gradients, text with icon-font fallback, icons.
- `yaml-lite.cpp`: `config.yaml` (block and flow lists and maps, `\uXXXX` escapes).
- `tools/css-spike`: builds YASB's element tree (`.yasb-bar`, `.container-left`, `.widget`, `.clock-widget`, `.widget-container`, `.icon`, `.label`), lays it out like a Qt row and draws it.

Across the 68 themes: all configs and layouts read, 6,551 of 6,908 rules kept (the rest are pseudo-element ones), 99% of 19,445 declarations recognised. Six themes drawn from their own files:

![Six YASB themes drawn by our engine](images/24-yasb-spike-themes.png)

Detail of one (Windows 11 theme by amnweb): the left of the bar, the right, and a hover on a workspace button:

![Detail](images/25-yasb-spike-detail.png)

Layout, colours, radii, borders and spacing follow the themes' own screenshots closely. What differs: fonts (icon fonts are different widths from the theme's own), the acrylic blur and rounded window, and real data for the widgets (the spike uses stand-in values). It runs beside the suite for now; it isn't in a build yet.

## b18: YASB themes in the bar (first stage)

The engine is in the bar. Pick a theme under **YASB theme** in the Taskbar settings and the bar is drawn from it.

![Two YASB themes drawn in the suite's own bar](images/26-yasb-in-the-bar.png)

- **Where themes live.** A folder under `%LOCALAPPDATA%\Tourne\Yasb\<name>` with `styles.css` and `config.yaml`. Drop a theme's folder or `.zip` on the bar (or its two files), or use *Install a theme or orb...*; the list fills itself. Saving a file reloads the bar.
- **What the theme decides.** The bar's size, position and padding (it floats off the screen's edge as it does in YASB, with the window as the bar's body, so blur and rounded corners match), the acrylic blur, the widgets in the left, center and right lists, and all their styling, `.dark` and `:hover` included.
- **What stays ours.** The Start button, task buttons (with jump lists, previews, drag), the tray and the clock keep working as they do; they are drawn in the boxes the theme gives them (`.app-container`, `.tray-icon`, `.clock-widget`). Also built: volume (the real level), active window, apps lists, power menu, custom buttons with `start_menu`.
- **Themes without buttons or a tray** (most of them: only 20 of 68 have task buttons) get ours: *Add buttons and tray to YASB themes without them*, on by default, adds task buttons and the Start button at the left and the tray at the right.
- **Commands.** A theme's buttons can name commands (`exec ...`). Those run only if *Let YASB themes run commands* is on (off by default: a theme is someone else's file). Start menu, power menu, notification centre and Windows settings pages always work.
- **Speed.** Building and styling the bar takes 0.2 to 0.5 ms even for the biggest themes (rules are looked up by class).
- **More screens.** Every bar on every screen uses the theme, each at its own scale.

Not there yet: transitions (`:hover` fades), gradients on the bar's window, box shadows, the data widgets (cpu, memory, weather, media, workspaces), popups (calendar, control centre), and `--yasb-*` colours from the system accent. Fonts matter: themes lean on Nerd Fonts, so install the one a theme names.

## b19: get YASB themes from the gallery

**Get a YASB theme...** in the bar's right-click menu opens a small window for the community themes of [yasb.dev/themes](https://yasb.dev/themes). They all live in [amnweb/yasb-themes](https://github.com/amnweb/yasb-themes) on GitHub (one folder per theme with `theme.json`, `styles.css`, `config.yaml` and a screenshot), so the window reads that repository directly.

![The gallery window](images/27-yasb-gallery.png)

- **Nothing is fetched until you ask.** The window opens empty; *Get the list* reads the folder listing and every theme's `theme.json` (name, author, description), 68 themes in a few seconds, six at a time. *Install* (or a double-click) downloads that one theme's `styles.css`, `config.yaml` and screenshot, and installs it like a dropped theme folder, under the theme's name. The bar picks it up at once; choose it under **YASB theme** in the Taskbar settings.
- **Paste a link instead.** The box takes a theme's gallery link, a GitHub folder or file link, a raw link, a `yasb-themes://` link or the bare id; the id in it is what counts.
- **Screenshot** opens the theme's picture in the browser before installing.
- **Only that repository.** File addresses named inside a `theme.json` are used only if they point into `amnweb/yasb-themes`.
- **Safe at unload.** A download in progress holds the module loaded until it ends. Tried in the test bar against the real repository: the list (68), an install from the list and one from a pasted GitHub link.

Because a theme is someone else's file, its commands still stay off unless *Let YASB themes run commands* is on.

## Small things

- Discord's taskbar button now uses the same pixel chat icon as its tray icon (`Discord.ico` in `%LOCALAPPDATA%\Tourne\Icons`, one more entry in *Button icons* for `Discord.exe`). Claude, GitHub Desktop, Settings and Snipping Tool already had theirs.
- Every build compiles for x64 and x86 (`__stdcall` callbacks fixed for x86).

## Not done yet

- **The other screens' bars were tried in the test bar** (three screens), not yet for long in Windhawk itself. A real 150% or 200% screen hasn't been seen, only a bar forced to 2x.
- The separator SAB draws before the last window's button: the rule isn't known.
- Progress bars on the buttons (parts 9 to 12).
- SVG and `.orb` orbs.
- Explorer restyling when SAB is off.

## b21: smoother dynamic shyness

- **Slides and fades instead of jumping.** The bar used to snap between shown and away. Now Windows' compositor moves and fades the finished bar (no redraw per frame), with frames timed to the screen's refresh: 6.9 ms apart on a 144 Hz screen in the test bar. When it turns back halfway, it continues from where it is.
- **New settings:** Hiding animation (Slide, Fade, Slide and fade, None); Smoothing (Smooth, Soft landing, Gentle, Off); Animation length; Hide delay; Return delay; While hidden (a sliver of the bar like StartAllBack, a soft glow in the accent colour, or nothing), with glow strength and depth.
- **Less flicker.** Windows have to stay clear for the return delay before the bar comes back, and nothing changes while a window is being dragged or resized. Moving the pointer to the screen's edge still brings it back at once.
- **Hidden means out of the way.** While away, the bar ignores clicks, and the blur is turned off under a fading bar or the glow.
- Blurring the bar as it moves (motion blur) was left out. It needs the whole bar redrawn every frame, which costs CPU; moving and fading cost nothing.
- The animation ignores Windows' "Animation effects" setting, since many people turn that off for Windows' own animations. Use None to turn it off.

## b22: progress on buttons, a crash guard, tidier settings

- **Progress bars on task buttons.** Downloads, copies and installers now show their progress on their button, drawn with the style's own progress parts (normal, busy, error, paused). A probe of Windows' own code showed where apps send it: to Explorer's task switcher window, as a state and a value from 0 to 65535. The suite's Explorer side catches it there and passes it to our bar. Busy progress sweeps across the button; that animation runs only while something is busy. Without a style, the colour scheme's accent is used (warning colour for errors).
- **Not yet matched to StartAllBack.** SAB's styles hold thin 42x5 progress strips, and niivu's full Everforest style holds full-button 27x26 ones, so where SAB puts the strip on a button still has to be photographed. The tool for that is ready; it needs an unlocked screen.
- **Crash guard.** If Explorer ends three times within two minutes of starting, the suite stops loading its parts into Explorer and says so on the taskbar. Saving any suite setting lets them back in. Folder windows running in their own Explorer process don't count. It costs one registry read and write when Explorer starts.
- **Safe Mode.** The suite doesn't load at all in Safe Mode.
- **Settings of a part that's off are hidden** (Windhawk 2.0). Each part's options show only while its Enabled switch is on. Windhawk 1.7.3 still installs the build and shows everything.
- The Explorer side now also runs for the taskbar alone (for progress), without System Icons' Explorer work when System Icons is off.
- The test bar can now save the frame it drew without reading the screen, so tests work while the PC is locked.

## Research: what a visual style fills in, and what it leaves out

Every class, part, state and property of 14 styles was dumped with msstyleEditor's own library: Windows' aero and aerolite, Windows 7, Plain8, niivu's Bouquet family (Bouquet, Night, MAC Dark, MAC Night, Bouquet SAB), Everforest Night and Everforest SAB, and the one in use here. Each was compared with aero value by value, with images compared by their decoded pixels. The full map is `research/msstyles/MSSTYLES-MAP.md` in the working folder.

- **Missing usually means inherited.** uxtheme fills a missing value from the part, then the class, then the style's globals, and a missing `App::Class` falls back to the plain class. This was checked against uxtheme itself, not assumed. niivu leaves scroll arrows, combo box dropdown backgrounds and the navigation sizer blank on purpose.
- **What's really left behind** is Windows' accent blue in a few places even finished styles don't repaint: the calendar's cells, dark Explorer's property text and fills, the command bar's split buttons, and dark list expanders. Task Manager's colours are data colours and stay.
- **A style's system colours hold its whole palette.** Bouquet SAB is Plain8 repainted (it kept Plain8's preview and tray arrow images). Everforest SAB is a full style plus the SAB classes. Bouquet and Bouquet MAC differ only in the window frame.

## b24: the style's own tray parts, and its palette

(b23, a SecureUxTheme build, was skipped.)

![Tray end with Bouquet SAB: at rest, arrow hovered, clock hovered, show desktop hovered and pressed](images/28-sab-tray-parts.png)

- **The show-hidden-icons arrow** is the style's own (`TrayNotifyHoriz::Button` and its Vert and Open variants), lit when pointed at or pressed. It flips to the Open arrow while the overflow flyout is up.
- **Clock and tray icon hovers** use `TrayNotify::Clock` and `TrayNotify::Toolbar`, as SAB does. The clock's text colour comes from the style too.
- **Show desktop button** at the far end (`ShowDesktop::Button`), sized by the style (10 px in Bouquet SAB, 14 in Windows 7). Clicking it sends Win+D. New setting *Show desktop button*: as Windows is set (the default, Windows' own "show desktop" switch), always or never. **It will appear on this PC**, since that switch is on here, as it does in SAB.
- **Thumbnail previews** are drawn with `TaskbandExtendedUI`: the popup's background, the hovered and flashing window, and the close button. Which part and state is which was worked out from the images; SAB's own previews haven't been compared side by side yet.

  ![Preview popup with Bouquet SAB's parts](images/29-sab-preview.png)

- **Only when the style has them.** A class that only falls back to Windows' plain one (compared by its pixels) is ignored, so styles without SAB's classes keep the old look. Checked: the style in use here, aero and Bouquet MAC Night draw as before; Bouquet SAB, Everforest SAB and Windows 7 get every part.
- **Palette from the style's system colours.** The flat colour scheme taken from a style now starts from its own menu, text, border and grey text colours before the menu's pictures are measured. For the style in use here and every Night style, the palette is unchanged. The light-menu styles (Bouquet Dark and Medium, aero) get a readable faint-text colour, the style's own grey instead of a near-white mix.
- **Windows' blue left in the style** (Theme Gaps, off by default): swaps the accent blues listed in the research for the style's own highlight colour (a darker mix of it for fills). Off, it costs nothing: the hook isn't even put in.
- Nothing forces title bar colours or changes Windows' system colours.
- The test bar's saved frames were upside down (GetObject reports a top-down picture's height as positive), and its preview test never opened a preview. Both fixed.

## b25: Explorer in your colours, and the Windows logo from a folder

### Explorer Colours (new part, off until switched on)

The idea comes from VitalS's [Explorer Visual Tweaks Dark](https://windhawk.net/mods/explorer-visual-tweaks-dark): catch the few theme parts Explorer draws its drive bars, selections and preview pane with, and draw them in your colours. That mod replaces the drive bar with its own rounded gradient, which throws away a style's shape (like the pixel segments here). This part keeps it.

![Drive bars and selections from the style in use here: its own, recoloured, recoloured with a fade and a recoloured empty part, flat; then selections and the focus pill](images/31-explorer-colours.png)

- **Drive bars, three ways.** The style's own; *the style's shape in your colours*; or flat bars (rounding, border, fade). A recoloured bar is the style's own picture drawn aside and remapped by brightness, so its most common colour becomes exactly yours and every segment, gap and shade stays. Separate colours for the filled part, a nearly full drive and the empty part, each with an optional second colour to fade to, and each left as the style's own when empty.
- **Selections** in the file list and the navigation pane (either or both): pointed at, selected, and (new) selected in a window that isn't active, each with its own fill and border; border width 0 to 3 and corner rounding 0 to 6; the navigation pane's focus pill with its own colour and (new) width.
- **Preview pane** background, and plain-text previews, in your colour, with (new) a text colour of your own or one picked to suit.
- **Also in these programs:** their Open and Save dialogs get the same selections and bars.
- **Any colour can be `accent`**, the visual style's highlight.
- **Cost.** Nothing is hooked unless something is on, and each hook checks a part and state number first, then one flag, before it looks at the class. A recoloured bar is two small draws and a pass over its pixels (a drive bar is a few thousand), only when a drive bar is painted.
- Colours start as the ones set in Explorer Visual Tweaks Dark here. Switch the part and the blocks on to use them.
- Checked outside Explorer against the style in use here (`tools/compare/colortest.cpp`). Not yet seen inside Explorer itself.

### Windows branding pictures (Icon Redirect)

![The logo from BrandingLoadImage, redirected: the three WINDux sizes, then an .svg drawn at 573x90](images/30-branding.png)

- **Folders of pictures.** A redirect's replacement can now be a folder of pictures named by resource id (`123.png`, `1123.bmp`, `2123.svg`). They stand in for that file's IMAGE, PNG and BITMAP resources. PNGs are used as they are; anything else is converted once, the first time it's asked for, and an .svg is drawn by Windows' own SVG renderer at the size of the picture it replaces. Ids the folder doesn't have stay Windows' own.
- **Windows branding pictures**, one setting: a folder for `Branding\Basebrd\basebrd.dll`, whose 123, 1123 and 2123 are the logo in About Windows (winver), the shut down dialog and System at their three sizes, the ids niivu's packs use. No system file is changed.
- **How it was checked.** `tools/basebrd/brandtest.cpp` compiles Icon Redirect into a test program, points winbrand.dll's own resource calls at its hooks, and asks `BrandingLoadImage` for the logo, which is what winver does. PNG, BMP and SVG all came back at the right sizes.
- **Cost.** Nothing new is hooked (Icon Redirect already reads resources); WIC and Direct2D are loaded only if a .bmp or .svg has to be converted, and never while a DLL is loading.


## b26: a quicker start, and a bar that stays out of the way

Measured in the test bar (warm disk, 3 screens) with timings built into the taskbar; `%LOCALAPPDATA%\Tourne\startup.txt` now logs the same steps on every real start.

### Starting up

- **The rest of the suite no longer waits for the taskbar.** It waited for the whole first setup (650 to 1400 ms); now it waits for the bar's window only (about 12 ms), and the bar sets itself up once its hooks are in.
- **Icons are read once at startup**, not twice (the second read came from a message that arrived after setup).
- **The programs list is read once at startup**, not twice (about 190 ms of background work saved).
- **Not in the lock screen.** The suite no longer loads into LockApp and LogonUI at all.

### The Start menu

| | b25 | b26 |
|---|---|---|
| First open after starting | 190 to 216 ms | about 30 ms |
| Every later open | 21 to 26 ms | about 9 ms |
| First open after a screen change | as slow as the first | about 9 ms |

- **Warmed while closed.** The home list, places, their icons and the right-click menus are made on low-priority threads a few seconds after starting (and after any settings change), so the first open is as quick as the rest.
- **A programs list that didn't change no longer throws the home list away.**
- **Open on press** (on): opens the moment the Start button goes down, as a menu does, instead of when it's let go.

### Screens, sleep and wake

- **Screen changes are gathered up** (a second's wait for waking screens to settle) and redo only the bars' placement, keeping the fonts, style and Start caches: 26 ms instead of a full reload.
- **Theme and colour notices that change nothing are ignored.** Windows sends several on wake; each used to reload everything.
- **Choices are written once**, not 60 times per reload.

### Tooltips and the aura

- **Tooltips:** by the pointer (Windows' place) or beside the bar, centred on the button; moved away from the bar or along it by any number of pixels; their own delay; and closed the moment a preview opens so they never sit over it.
- **The dynamic aura follows at the screen's refresh rate** instead of 15 ms steps, with a *trail* (0 sticks to the pointer; more is a smooth catch-up, as a half-life in ms), plus *size* (40 to 300 %) and *strength* (0 to 300 %) for the Aura style.
- **Cost.** A redraw with the aura following is 0.5 ms, the same as without; the pacer stops when the aura has arrived.

### Window previews, your way

- **How they open:** on resting on a button (as before), on resting on groups only, or only on a click. Ctrl+click now opens any running button's previews, not only a group's. While they're up, moving to another button moves them there.
- **Where:** on the button (as before) or on the pointer, then moved away from the bar or along it by any number of pixels.
- **How long they stay:** until the pointer leaves (after a delay you set, 300 ms as before), or **until dismissed**, in the ways you pick: a click elsewhere, another window coming to the front (not one of theirs), Esc (taken, as a menu takes it), any key (passed on), or a time away. Each can be on or off.
- **After picking a window:** close them (as before), or keep them up to flip between windows.
- **Cost.** The click and key watchers are only in while previews kept until dismissed are up; otherwise nothing is added.
- Checked in the test bar: the offsets and pointer placement land where expected, and the watchers go in and out with the previews.

### Frames and stacks, as StartAllBack draws them

![Ours (top) and StartAllBack (bottom) with Bouquet SAB, 8x: square frames with their dark outer rows, and a stack's edges as straight lines](images/32-frames-and-stacks.png)

- **The dark outer layer is back.** The colour scheme swaps its tint in for whatever is still the bar's background, and it found that by colour alone. Bouquet's frame border is exactly the bar's colour there, so the frame's two dark outer rows were taken for background and tinted away, leaving the light line at the edge. Drawn pixels are now told apart by alpha as well.
- **Square frames.** Frames are 4 px in from both sides of the bar (30 px on a 38 px bar), as StartAllBack's are, not 28. With the dark rows back, a frame now matches StartAllBack's pixel for pixel.
- **Stacks.** A group's edge is the style's own picture at its own width, right against the frame, as tall as the frame, and one straight line: StartAllBack leaves out the picture's little top and bottom caps, which showed here as hooks. The next button starts right after it (5 px for two windows, 8 for more, read from the picture). Before, it was squeezed to 6 px, so a stack of three or more ran its two edges together.
- **Cost.** Nothing measurable: a redraw is still about 0.5 ms.
### Shadows

- **Under icons, under button frames, or both** (off by default), in any colour, with strength, softness (0 to 8 px) and offset. Each shape's shadow is blurred once and kept, found again by its pixels, so a changed icon gets its own.
- A shadow falling on the bar's background falls on your colour scheme's tint (it's kept aside and laid over the tint at the end), not on the style's own background.
- About 0.06 ms per redraw.

### Hiding and fading

- **Fade when idle** (off by default): after a set time away, the bar fades to a set opacity. Choose whether that happens always, only while nothing touches the bar, or only while a window does. The opacity rises as the pointer comes within a set distance, and anything that keeps the bar busy wakes it at once: a menu, its previews, a press, the Start menu, or (as set) a window wanting attention. Fade-out and wake times are separate. Only the compositor's opacity changes; the bar isn't drawn again for it.
- **Automatic hiding:**
  - A *reveal zone*: the bar comes back before the pointer reaches the very edge.
  - It can come back for a window wanting attention.
  - It can come back while the Start menu is open.

### Tray

- **Overflow arrow:** System Icons' green pixel arrow again, or the visual style's (b24 always took the style's).
- **Tray icons, one click** (right-click the clock; they're first there, or right-click the bar):
  - A running icon goes into the tray or back to the overflow with one click.
  - *Not running now* lists every icon Windows has seen (the list its own settings page uses). Ticking one puts it in the tray the next time its program starts, without going through Windows' settings.
  - The old per-icon choices are under *Where each goes*.

### Previews your way, jump lists out of the way

![Previews in Bouquet SAB's own look (top) and in colours set by hand, with a 2 px border, bigger thumbnails and more padding (bottom)](images/33-preview-looks.png)

- **Preview look:** the visual style's (as before), or your own colours: background, titles, the pointed-at window and its border, the window in front, a window wanting attention, and the close button pointed at and its cross. Empty colours are the scheme's.
- **Either look:**
  - thumbnail width and height
  - padding in each window
  - padding to the border (also between windows)
  - border colour (or none), and in your own look its width
  - corners: square, slightly rounded or rounded
  - a drop shadow
  - a backdrop (blur, acrylic or Mica) behind a background of any opacity. A backdrop takes the place of the style's background picture.
- **Jump lists and window menus:** at the pointer (as before), or beside the bar, lined up with the button's start or centre, moved away from the bar or along it by any number of pixels. The button's previews and tooltip close first either way.
- **Show desktop with segments.** The button was only laid out on a bar without segments, so with segments on it was missing. It now sits at the end of the tray's segment, inside its island (with no tray shown, in an island of its own). Bouquet SAB draws it empty until it's pointed at, as StartAllBack does.
- **The notification centre** stays on the taskbar's edge when it shrinks (after *Clear all*). A resizing flyout used to guess which edge to keep from whether its top was in the top half of the screen, and the notification centre is tall enough that it always is. It now remembers the edge it was placed against.
### Explorer: navigation pane lines and gaps (Explorer Style)

- **Hide the navigation pane's separator lines** and **Close the navigation pane's gaps**, both off by default. The idea is Languster's Explorer TreeLine Killer.
  - Lines are found by their pixels: after the pane paints, any flat row across a group's row that isn't the background is painted over. So this doesn't depend on where Windows puts the line.
  - Gaps close by making the groups' double-height rows single height; they're given back when switched off.
  - Nothing is subclassed while both are off.
- Not yet seen with Home, Gallery or pinned folders showing (the pane here shows only This PC).

### Windows 11 26H2, and running without StartAllBack

- 26H2 is an enablement package on the same files (this PC's 26200.9550 already has them). Nothing in the suite checks the build number, and its hooks match files that don't change. The only risk is StartAllBack needing an update for build 26300.
- Research for replacing Windows' taskbar and Start without StartAllBack is in `research/disable-w11-shell.md`. It weighs m417z's Taskbar auto-hide fine tuning and Exiled Eye's Block Start Menu and Hosts. The plan: cloak the taskbar from inside Explorer, and take every Start opening through `XamlLauncher::ShowStartView`, which also retires the keyboard hook.
### Settings folded at first

Windhawk 2.0 alpha 6 has no way for a mod to ask for folded groups. It remembers what you fold per mod id, and each build here has its own id, so every new build opens unfolded.
### Does switching a part off unload it?

- A part that's off has no hooks: they're removed when the suite reloads. Where no part is on at all, the suite asks Windhawk to unload it from that program.
- Where it is loaded, it costs about 75 KB of private memory per program, plus about 350 KB shared by all of them (the 3.3 MB DLL is mapped once).
- Programs that can't be unloaded into (sandboxed browsers, suspended apps) keep an old build until they close.

## b27: the Wi-Fi list comes back by itself

- **Nearby networks were missing from the network flyout.** System Icons opened its connection to Windows' Wi-Fi service once, at start. After the service restarts (a Windows update, like the 26H2 one here, or waking from sleep), that connection goes dead, and every scan and list after it failed quietly. The flyout then showed only the current connection, which comes from somewhere else. It now opens the connection again whenever a call fails, and retries.
- **Checked** with a new test program (`tools/sysicons/nettest.cpp`) that runs System Icons' own network code outside Windhawk, started as a plain desktop app: it lists the nearby networks (6 to 8 here).
- **When Windows really does withhold the list:** since Windows 11 24H2, Windows may keep nearby networks from apps without location access. In that case the flyout now says so, with a link to Location settings, instead of *Looking for networks...*. It shows only when Windows actually refuses, or gives back nothing but the connected network while location is off. Here Windows gave the list without location access, so this is a fallback.

## b28: the location question, explained and optional

- **New setting in System Icons: Nearby Wi-Fi networks.** Its description explains why Windows may ask about location access (or list Windhawk under *Recently used* in its location settings) when the network flyout lists nearby networks. Windows counts that list as location data. The flyout itself only gets network names and signal strength.
- **List them** is the default and works as before.
- **List them, without the location note** keeps the list but never shows the "location access" message, for people who keep location off on purpose.
- **Off** never asks Windows for nearby networks, so there's no scan and no location question. The Wi-Fi section then shows your current connection and a *Show available networks* link to Windows' own list.

## b29: Windows' own taskbar and Start, switched off from inside Explorer; All Programs opens at once

### Windows' taskbar and Start

The suite's Explorer relay now turns Windows' own taskbar and Start away itself, the way the best Windhawk mods do it (m417z's Taskbar auto-hide fine tuning, whose symbol names this follows), but only for the suite's own bar. Every symbol was checked against this PC's Windows files (26100.9278, the 26H2 binaries) before building.

- **Taskbar.** With "Hide StartAllBack's taskbar" on, Explorer's taskbars are **cloaked**: DWM draws nothing and they take no input, so there's no flash for a notification and no sliver at the screen's edge. Explorer uncloaks its taskbar by itself as it redraws, so that one call is turned away (`DwmSetWindowAttribute`, only for the taskbar windows). Its unhide (`TrayUI::Unhide`, `CSecondaryTray::_Unhide`) is dropped too, so it doesn't even animate. A new screen's taskbar is cloaked a moment after the screen appears.
- **The old 500 ms check** that re-hid the taskbar now looks in every 5 seconds while the taskbar is cloaked, just to catch an Explorer restart.
- **Safety.** If the suite's bar ends for any reason, a crash included, Explorer's relay notices at once (it waits on the bar's process) and puts Windows' taskbar back, so there's always a taskbar.
- **Start.** Without StartAllBack, Explorer hands every opening of Windows' Start (`XamlLauncher::ShowStartView`) to the suite's Start: the Windows key, Ctrl+Esc, or anything else. Then the low-level keyboard hook switches itself off, so no key press on the system passes through the bar any more. With StartAllBack loaded nothing changes: it keeps the key, and the keyboard hook stays.
- **Cost:** nothing between events. The hooks only run when Explorer tries to show its taskbar or Start.
- **Not tried live yet:** it needs StartAllBack switched off and Explorer restarted.

### All Programs

- **Measured** with a new test-bar check (`TB_perf=all`): pressing All programs cost about 340 ms, and 700 ms the first time. Two causes:
  - The program list was thrown away on every Start open and read again, asking the shell for each entry's display name (about 1.3 ms each, 81 entries here).
  - The icons were made on the menu's own thread as it painted.
- **Now** the idle warm-up that already fetched the home list's icons also fetches All Programs' top-level names and icons, and the apps' icons, on its own thread while the menu is closed. The names are kept from one open to the next.
- **Result:** pressing All programs takes **1.6 ms** after the warm-up, and 0.8 ms on later opens. Start's own open is unchanged (29.8 ms first, 7.2 ms after).

## b30: no more "Initializing..." when Windhawk opens

- **What you saw:** every time Windhawk was opened, a window listed the suite as *Initializing...* in a windhawk.exe process.
- **Why:** each windhawk.exe loads the suite's launcher, both Windhawk's tray process and the one started when you open Windhawk. Each launcher started the suite's tool process (the taskbar and System Icons). With one already running, the second sat in its start-up for up to 5 seconds, waiting for the first to go, and then gave up. Windhawk showed that wait as a mod still initializing.
- **Now** the launcher only starts a tool process when none is running. It waits for one on a thread of its own, since after a settings change the old one is on its way out and gone within moments. One still there after a few seconds is running for good, so it's left alone. The tool process itself now gives up after 1 second instead of 5, for the rare race.

## b31: Wi-Fi connects again; tray menus close on a click away and can open clear of the taskbar

- **CONNECT and DISCONNECT did nothing in the network flyout.** The button sits inside its network's row, and a click went to whatever was listed first under the pointer, which was the row. So each click just folded the row up again. A click now goes to the smallest target under the pointer, so the button wins over its row, whichever order a flyout lists them in. Bluetooth's CONNECT had the same fault and is fixed with it.
- **A tray icon's right-click menu wouldn't close on a click elsewhere.** Windows closes a menu on an outside click only when the menu's window is in front, which a click on the Tourne taskbar (which never takes the focus) doesn't always leave it. While a menu is up, a mouse hook now closes it on any click outside it, and swallows that click as Windows' own menus do, so a click on the same icon doesn't reopen it. The hook exists only while a menu is open.
- **New System Icons settings for where things open:**
  - *Where the icons' right-click menus open*: at the pointer (as before), or clear of the taskbar, next to the icon, on whichever edge the taskbar is.
  - *Menu lined up with the icon*: starting at it, centred on it, or ending at it.
  - Distance from the taskbar and shift along it, in pixels, for both the menus and the flyouts. The flyouts' distance is added to their usual 8 px.
  - All default to how things were.

## b32: flyouts and menus stay on their own screen

- **Flyouts** are kept inside the work area of the screen their icon is on, both when they open and when they grow while open (the notification centre, a network list). They're moved back in, and are never taller or wider than that screen.
- **Every menu in the suite** (the taskbar's, Start's, System Icons') goes through one place that keeps it on one screen. That's the screen of whatever opened it, or else the screen under its point. A point on or past the screen's edge is brought back inside.
- **A menu doesn't cover what opened it.** Windows is told what to keep clear of, and when there's no room on one side it shows the menu on the other:
  - A menu opened from inside a flyout (an icon in the hidden icons flyout) opens beside the flyout, on the side toward the middle of the screen, and on the other side if there's no room.
  - Tray menus set to open clear of the taskbar, and jump lists placed by their button, stay off the taskbar.
- **Other programs' own menus** (an app's tray icon in the hidden icons flyout) are drawn by those programs, so they open where the program puts them.

## b33: the hidden icons flyout's own menu

A right click on the open part of the hidden icons flyout (off its icons) now opens its own menu, beside the flyout:
- **Show on the taskbar**: pick an icon and it moves into the taskbar's tray.
- **Take out of the taskbar and here**: pick an icon and it shows in neither the taskbar nor this flyout.
- **Show them all on the taskbar**: every icon in the flyout moves at once.
- **Bring back here**: icons taken out, listed with *not running* for programs that are closed. Pick one and it's back in the flyout.

Icons are listed by their tooltip and program. The choices are kept where the taskbar reads them too, so it updates at once. The flyout stays open under the menu and takes the focus back afterwards, so a click away still closes it. A right click on an icon still opens that program's own menu.

## b34: new calendar events, through Google Calendar

**Right-click a day to add an event (Google Calendar, online)** is a new System Icons setting, off by default. A right click on a day in the notification centre's calendar opens a small window:
- Title, the day, all day or a time from and to, where, importance (Low, Normal, High) and notes.
- Times can be typed as `9`, `9:30`, `0930`, `9pm`, `9:30 am` or `21:15`. An end before the start runs past midnight.
- Tab, Enter and Escape work as in a dialog.

![The new event window](images/34-new-event.png)

**Add to Google Calendar** opens Google Calendar's own new-event page in the browser with everything filled in, and one click there saves it. The event then reaches every device, and this agenda the next time it reads the calendar.
- The times are sent as UTC, so the event lands at the right time whatever time zone Google's calendar is set to.
- Google has no importance field: High puts "!" before the title, and Low or High is written in the notes. Tasks can't be made this way, because Google Tasks has no such page.
- **Why it's online, as the setting says:** Google Calendar itself saves the event, in a browser signed in to your account. The suite only opens that page. It sends nothing itself and keeps no account, key or password.

Checked outside Windhawk (`tools/sysicons/evtest.cpp`): the time parsing, 14:30 local on 3 October 2026 becoming `20261003T183000Z`, the link's encoding (`Dentist & café` becomes `Dentist%20%26%20caf%C3%A9`), and the window as drawn.

## b35: testing the preview pane's sizer

With StartAllBack off, Explorer's side pane could be dragged wider but never back narrower than the width it opened at. b35 added test-only layouts behind a hidden switch, to rule the layout in or out, and `tools/navpane/Test-PreviewPane.ps1` to drag the pane in a folder window of its own (it posts mouse messages, so the pointer isn't touched, and Explorer's saved pane sizes are put back after).
- Every layout tried behaved the same, ours and StartAllBack's own: StartAllBack's DLL still loads into Explorer while it's switched off, and its details-on-the-bottom layout wins over ours.
- With StartAllBack running in the test window, the pane shrank normally. So the limit isn't the layout but Windows' sizer code in shell32, which StartAllBack patches.

Also from b35: the user's b33 settings were saved as `settings/tourne-suite-b33-settings.reg` for future builds.

## b36: choosing the Google account for new events

**Google accounts for new events** (System Icons) takes your calendar's email, or several separated by commas. The event window gets an Account row, which cycles through them on a click, and Google's page opens as that account (`authuser=`). This matters when the browser's default Google account isn't the calendar's. That account has to be signed in to the browser.

b36 also tried two fixes for the pane, still as tests. Neither worked live, which b37 explains.

## b37: side panes shrink again, and resized windows stay where you put them

**The pane fix.** A small test DLL, loaded only into a test folder window, logged every call to shell32's `CDUISizerElement::_ComputeBoundedSize` (its disassembly came from Windhawk's own LLVM DLL, as this PC has no disassembler). It showed:
- b36's hook was installed and running, but only looked for the old preview pane (ReadingPane). The pane being dragged is Windows 11's combined details and preview pane, DetailsContainer.
- That sizer's MinSize is set to the width the pane opened at (277 px here), and the clamp held every smaller size at 277.
- Letting that floor go in the test DLL let the pane shrink the full 100 px, twice.

Explorer Style now does the same for both panes, only when the minimum is what holds the pane, and never below 100 px. The test layouts are gone. `tools/navpane/Probe-Sizer.ps1` keeps the probe for next time.

**The window manager.** A tiled window you resize by hand now leaves the tiling instead of snapping back, and stays floating where you put it.
- Its place is remembered for that program (file name and window class) and saved, so the program's next window opens there, also after a restart. One window at a time holds a program's place, so a second window is tiled as usual.
- Moving a kept window updates its place. Shift+W+Space puts it back in the tiling and forgets the place.
- It works with automatic tiling off too, so Shift+W+Tab doesn't pull it back either. Dragging a tiled window without resizing it still swaps it with the one it's dropped on.
- New setting under Window tiling: **Leave a window you resize where you put it**, on.

## b38: window layouts, every window's place remembered, and the pane goes narrower

**The pane.** b37's fix works, but it stopped at 100 px. The probe showed one long drag reaching that floor straight away, after which the pane wouldn't go narrower and seemed broken. The floor is now 24 px. Repeated drags in a window of its own process, in one the shell opens, with a picture previewing, and across three windows in a row (each opening at the width the last left) all shrank.

**Remember where every window closed** (Window tiling, on). Every window with a title bar or a resizing border is followed while it's open: programs' windows, dialogs and tool windows. Its place is saved when it closes, or hides, as programs that close to the tray do.
- The next window of its kind opens there, and maximised again if it closed maximised. A kind is the program and window class, and for dialogs the title too, as one program has many.
- The tiling still places the windows it tiles, and a place kept by resizing a window by hand comes first.
- Places are written two seconds after a window closes, once for a burst, and only the 256 most recent are kept.

**Window layouts** (Shift+W+M). A layout is each screen's tiling layout, plus each window's place and whether it's tiled, saved under a name.
- **Save current as new** takes the windows open now. **Update from windows** retakes the chosen one.
- **Apply** puts the open windows back: each line takes the first window of its program not taken yet, and the tiled ones go back in their order.
- A layout is plain text, one line per screen or window, and can be edited, renamed and saved in the window. **Delete** asks first.
- Shift+W and 1 to 9 apply the first nine layouts without opening the window.

![The Window layouts window](images/38-window-layouts.png)

Checked outside Windhawk (`tools/tiling/lytest.cpp`, with two see-through test windows in a separate process):
- A layout taken, the windows moved away and the layout applied: both went back.
- A window closed at one place and a new one of its kind opened elsewhere: it was moved to where the last one closed.
- Two layouts were saved and one renamed.

The first run used resizable test windows, so the live b37 tiler tiled them. The test windows are now fixed size, which it leaves alone.

## b39: one icon for a program, everywhere

The ask: Process Lasso, every part of NVIDIA, and Discord (launched through the Vencord shortcut, from a folder that changes with every update) each with one icon in every place Windows shows one. The NVIDIA Settings tray icon kept going back to NVIDIA's own.

**Why they slipped.**
- The Icon Redirect rows for Discord had a `*` in front of the replacement file, which turned it into a path that led nowhere. The same was true of the two InputSwitch rows.
- Discord's rows named `app-1.0.9260`, and NVIDIA's named one driver store folder. Discord's updates and NVIDIA's driver updates move both.
- The tray swaps other programs' icons as they arrive. Anything it doesn't recognise keeps the program's own icon.

**Use everywhere.** The taskbar's Button icons rows get a "Use everywhere" switch, and their program can hold `*` and `?`, like `*\NVIDIA Corporation\*.exe`. One marked row now drives everything:
- **The taskbar buttons**, as before.
- **The tray:** the row's icon wins over the tray's own designs.
- **Alt+Tab.**
- **Icon Redirect,** which puts the .ico wherever Windows reads the icon from the program's file: Explorer, Start, its windows. A plain file name matches in any folder, so updates can't lose it.

Rows that already existed stay off, so nothing else changes. b39's defaults add rows for:
- **Discord:** `Discord.exe`, `*\Discord\*.exe`, `*\Discord\app.ico` and its app id.
- **NVIDIA:** `*\NVIDIA Corporation\*.exe`, `*\Display.NvContainer\*.exe`, `nvcplui.exe`, and the NVIDIA App and Control Panel app ids.
- **Process Lasso:** `*\Process Lasso\*.exe`.

**The icons.** They're drawn by `Make-PixelIcons.ps1`, exactly as the tray draws its art: the ink green, coral accents, one-pixel shade, and whole-number scales from 16 to 256 px. Redrawing the Discord icon with it matched the original pixel for pixel. NVIDIA is an eye with a coral pupil, and Process Lasso is a lasso with a coral knot.

![NVIDIA and Process Lasso](images/39-program-icons.png)

**Known limits:**
- The Start menu's tile for the NVIDIA Control Panel is a packaged app's own picture, which nothing here replaces. Its taskbar button and Alt+Tab do change.
- Explorer may keep old icons in its cache until it restarts.

## b40: a desktops pager, and tray icons that stay themed

The ask: a way to see and reach the desktops Win+Tab makes, from the taskbar. It should move along the bar, come off it to sit anywhere on the screen, lock in place, and change its look from a right-click menu. Also, NVIDIA Settings was still showing its own tray icon.

**The pager.** It shows a mark for each desktop, and the one you're on is lit in the accent colour.
- **Click** a mark to go to that desktop. **Scroll** over it to step through them. It switches with Ctrl+Win+Left and Right, the same way Windows does.
- **Docking:** it can sit before the tray (the default), after Start, or after the clock. It takes its own room on the bar, so the buttons move over. With segments, it shares an island with the tray or with Start.
- **Floating:** drag it off the bar and it floats anywhere on the screen, on every desktop. While it's over the bar it turns see-through, and letting go there docks it at the nearest of its three places.
- **Right-click** it for:
  - Lock in place.
  - Its look: dots, numbers, or the names Task view gives them.
  - Where it goes, and whether a floating pager stays on top.
  - Hide it when there's only one desktop.
  - Task view, a new desktop, or closing the one you're on.

The desktops come from Explorer's own registry, which the bar already watched for YASB themes' workspace buttons, so changes show at once without polling. Your pager choices are kept beside the tray's, in `HKCU\Software\TourneTray`. It's in the taskbar settings as "Desktops pager", and it's on.

**NVIDIA Settings' tray icon.** The tray swaps a program's icon as it arrives. NVIDIA's arrived before anything could swap it, and nothing ever looked at it again. Now every icon the tray holds remembers whether it's themed. The relay's existing 10 s pass themes any that aren't. An icon with nothing to theme it is tried a few times, then left alone until its program sends a new one.

**Known limits:**
- Clicking a mark several desktops away steps through each one in between, as the keys do.
- On a vertical bar, the names look shows numbers.

## b41: scans from the security flyout, and a pager that really floats

The ask: run a scan from our Windows Security flyout. Also, let the desktops pager truly float, so it doesn't attach itself when it's over the taskbar, and give it options for size, shape, colours, fade and opacity.

**Scanning.** The flyout has **Quick scan** and **Full scan** under the last scan's time.
- It uses Defender's own command line (`MpCmdRun`), with no window. Defender's service does the scanning; we only wait for it to finish.
- The wait runs on its own thread, so you can close the flyout. Reopen it and you'll see how long the scan has been running.
- The tray icon gets a dot while a scan runs, and its tooltip says which kind of scan.
- When it's done, the flyout says "no threats" or "threats found", and the last-scan time updates.
- If Windows asks for an administrator, the flyout offers **Scan as admin**, which runs the same scan through the UAC prompt.
- Unloading the suite only stops the waiting. The scan itself carries on in Defender.

**Floating that stays floating.**
- A floating pager stays where you let go of it, over the taskbar too. It's never pushed off the bar's part of the screen any more.
- It docks only when you pick a place on the bar from its menu, or when you carry it off the bar and drop it back on.
- With "Keep on top" on, the bar owns it, so it always sits above the bar.

**Its look:** a new "Desktops pager look" group in the taskbar settings.
- **Marks:** size, spacing, and shape: pixel dot, square, circle, diamond, pill (the desktop showing is a wide pill) or bar.
- **Outline:** an outer colour ring around each mark, with the inner colour inside.
- **Colours:** from the colour scheme, or custom ones for the marks, the desktop showing, and the floating panel and its edge.
- **Floating only:** the panel's shape (rounded, square, pill or none), the panel's opacity, the whole pager's opacity, a fade when the pointer leaves it (how faint, and after how long), and running top to bottom.
- Shapes are smoothed. Text on a see-through panel keeps its own edges, so names stay crisp with no panel behind them.
- The right-click menu adds "Size, shape and colours...", which opens Windhawk.

The defaults draw it exactly as b40 did.

## b42: Explorer stops crashing, tooltips hold still, classic menus everywhere

The ask: find out why Explorer kept crashing and why the tooltips flickered (from a screen recording), and fix both. Also, a better way to turn off Windows' "immersive" menus.

**The crashes.** Explorer had crashed 11 times since 2026-09-29, and never before that. Every crash was in Windows 11's own taskbar, which is built with XAML.
- Turning StartAllBack off brings that taskbar back to life inside Explorer, and we were keeping it hidden in two ways it can't cope with:
  - We cloaked it, then answered "done" when Explorer tried to uncloak it.
  - Our taskbar process kept hiding its window from outside.
- The crash dumps fit: the failure always came inside the taskbar's own code, on the thread that runs it.

**How it's hidden now.** It's the approach of m417z's "Taskbar auto-hide fine tuning" mod, which hides that same taskbar without trouble:
- Windows' own auto-hide is on, so Explorer itself has the taskbar hidden away.
- Everything that would bring it back is turned away before it starts:
  - notifications and the Windows key asking it to unhide,
  - the timer that slides it in when the pointer rests at the edge,
  - Windows 11's "should it be expanded" check, and the pointer or a swipe at the edge.
- Only once Explorer has hidden it is it cloaked, so not even its sliver shows. That's done on the taskbar's own thread, and Explorer's own cloaking and uncloaking go through untouched.
- Its window is never hidden from outside any more. A taskbar an older build hid gets shown again.

**Tooltips.** Every second, a tray icon's tooltip changing (the CPU temperature, say) made our bar refresh whichever tooltip was showing. Each refresh put it on top of the bar for one frame before it jumped back.
- Now only the tooltip you're looking at refreshes, and only when its own text changes.
- The tooltip can no longer land anywhere but its place above the bar.

**Classic Menus (new part).** Windows draws some of its own menus as "immersive" ones. Each part of the shell carries its own copy of the code that does that.
- I looked through every Windows file for that code. It's in 12 64-bit and 6 32-bit files on this Windows.
- The usual mod's list predates Windows 11 24H2, so it misses:
  - the taskbar (moved into Taskbar.dll),
  - the keyboard language menu,
  - Windows Security's renamed tray module,
  - File Explorer's new parts,
  - Task Manager,
  - Store apps' title bars.
- Ours covers them all, and hooks each file as it loads instead of loading it into every program.
- On 32-bit programs it uses the same calling convention Windows does, which the usual mod gets wrong.
- Switches cover Explorer and file dialogs, the taskbar, tray icons, and Windows' programs. The "Taskbar settings" gear is left off.

## b43: desktop previews

**Previews of your virtual desktops.** Point at a pager mark, on the bar or floating, and after a moment a preview of that desktop opens beside it.
- It shows the desktop's own wallpaper (or the main one), its windows in their real places, and its name.
- The windows are live, as Task view shows them, straight from Windows' compositor. Windows on other desktops keep their pictures there even while hidden.
- Sliding along the marks swaps the preview at once. Clicking, dragging, the menu or leaving the pager closes it.
- Nothing runs while no preview is open.
- Two new settings under "Desktops pager look": the previews themselves (on) and their width (280 px at 100%). With previews on, the mark's tooltip steps aside, since the preview names the desktop.

## Tester kit

**A copy for anyone to try.** A clean build of the suite in its own folder, with nothing from my PC in it. On a tester's PC it fills itself in from their own setup, and it can put everything back.
- No paths, colours, devices or apps of mine. Colours follow the tester's own visual style. The parts that restyle Windows to one person's taste (green text, menus without icons, classic menus) start off for them to try.
- **Back up first:** records every Windows setting the suite is able to change before it's installed.
- **Fill in my settings:** reads the tester's tweaks, StartAllBack, taskbar layout, Resource Redirect icon theme and rules, and battery devices into the suite, and installs the pixel fonts.
- **Reset my PC to before:** puts it all back, takes away anything the suite added, and names the few boot and account tweaks it can't read without admin.
- A START HERE guide covers installing, applying, using and resetting, with a notes sheet for feedback.

## b44 to b47: hunting the extract crash

The ask: Explorer still crashed with StartAllBack off. It failed fast in Windows.UI.Xaml (0xc000027b), and it could be made to happen on demand by extracting an archive.

**Three wrong turns first.** Each was its own test build, and each was taken back out:
- **b44** kept StartAllBack loaded and hid its classic taskbar instead. Explorer stopped crashing, but there were two taskbars, and StartAllBack being off was the whole point.
- **b45** made one more of the relay's messages wait (`SMTO_BLOCK`). The crash didn't care.
- **b46** had test switches to turn the relay's hooks off one by one. With all of them off it still crashed, so the hooks weren't the cause.

**The crash dumps.** Read with Microsoft's symbols for Windows.UI.Xaml, Taskbar.View and WinUI 2:
- The hidden Windows 11 taskbar still builds a task button for every window. Extracting an archive reports progress on its window, so that button builds a WinUI 2 progress bar.
- The progress bar's template asks WinUI 2 for `ProgressBarTemplateSettings` by name. WinUI 2 doesn't make that class by name, so the request fails (`0x80040111`, "class not available").
- Its next animation then fails (`0x80070490`), and `VisualStateManager.GoToState` takes Explorer down.
- Nothing about the hiding causes it. Any progress shown on a hidden XAML taskbar does.

**b47: the fix.** Your idea: just don't let the progress through. The relay already listens to the task list window (`MSTaskSwWClass`) for app progress, to show it on our buttons. While Windows 11's taskbar is kept hidden, that progress now stops there instead of going on to Explorer. Our taskbar still shows it. Once hiding ends, the next update reaches Explorer as before.

Extracting, copying and downloads no longer crash Explorer with StartAllBack off.

## b48: a design for any program's tray icon

The ask: a program (Recordly) had lost the little screenshot icon the tray used to give it. Can that icon be used for other programs too?

- A Button icons row can now name one of System Icons' designs instead of an `.ico`: `tourne:screenshot`, `tourne:chat`, `tourne:cloud` and so on.
- In the tray, the program's own icon is replaced by that design, in the colour scheme, at every size.
- The names: screenshot, capture, chat, cloud, game, audio, claude, spotify, wallpaper, fan, tiling, gpu, sensor, bell, clipboard, bin, phone, printer, usb, keyboard, power, mic, vpn, update. They're listed in the Icon field's description too.

## b49: designs everywhere, and a kit that's anyone's

- **Designs on buttons and in Alt+Tab.** A `tourne:` row now also gives the program's taskbar buttons and its Alt+Tab entry the design, with no "Use everywhere" needed. The button needs the Taskbar part on, Alt+Tab the Switcher.
- **The kit's defaults are neutral.** A tester's build takes each part's own defaults, but three of those were one person's taste. Now a kit starts with:
  - the colour scheme following the taskbar's style,
  - no names in Menu No Duplicates,
  - Menu No Icons on `explorer.exe`, `dllhost.exe` and `rundll32.exe` only.
- The kit's notes sheet names the right step ("3. Fill in my settings").

## Off the suite: green arrows, and a restore

- **Green Start menu arrows.** The arrows beside the Start menu's places are images in the style (StartPanelPriv's FLYOUTARROW, which Bouquet SAB shares with PINICON). `tools/msstyles/Set-GreenStartArrows.ps1` makes a copy of the style with our pixel chevron drawn in at each image's size, in `%LOCALAPPDATA%\Tourne\Styles`.
- **Green menu arrows.** StartAllBack's flyouts and every classic menu draw Windows' own submenu arrow (Menu's POPUPSUBMENU, a normal and a disabled arrow in one image). `tools/msstyles/Set-GreenMenuArrows.ps1` makes a copy of the Windows style with the chevron in both, the disabled one faded, and a `.theme` to apply it.
- **A reset.** A crash outside the suite ended with a System Restore, which took the newest settings with it. The b48 settings in `builds\` were renamed for b50 and imported back. That's one more reason everything now lives in git.

## Research: how StartAllBack stays stable

With StartAllBack on, nothing crashes. Why?
- StartAllBack never lets Windows 11's XAML taskbar exist. The suite kept it alive and hidden, and every crash so far came from that hidden XAML running.
- ExplorerPatcher, which is open source, shows how: Explorer creates its taskbar through one component (`TrayUIComponent`). ExplorerPatcher answers that request with a classic taskbar of its own.
- Since 24H2 the classic taskbar isn't in Explorer any more. ExplorerPatcher ships its own rebuilt one, as a compiled DLL only.
- On 25H2, `Taskbar.dll` loads each taskbar's XAML content through two functions (`TaskbarHost::TryLoadTaskbarFrameFromController` and `TryLoadTaskbarFrameFromXamlExtension`). If neither loads anything, Explorer itself unsubscribes that taskbar's events and carries on with the bare window. That's a path Microsoft built, which gave b50 its idea.
- An app instead of a mod wouldn't help: whatever stops Windows' taskbar has to run inside Explorer either way.

## b50: Windows' taskbar without its content (a test)

**A new setting, "Don't load Windows' taskbar (test)", off by default.** With it on, together with "Hide StartAllBack's taskbar" and StartAllBack off:
- The relay makes both of those loaders report that nothing loaded, as Explorer starts.
- Windows 11's taskbar window is still made, and still hidden, but its buttons, tray and progress bars never exist, so none of them can fail hidden.
- It's hooked in init, before Explorer builds its taskbars, so a change takes effect when Explorer restarts. The crash guard still turns the suite off if Explorer loops.

The first b50 didn't compile: `Wh_Log` is a macro that pastes its message onto a string literal, so it can't take a choice between two strings. The syntax checks had been run in Windhawk's editor mode, where `Wh_Log` is an ordinary function. They now run in real compile mode, which shows that error.

## b51: renaming desktops, and your own desktop keys

The ask: change a desktop's name with a middle click on the pager, and choose the keys that switch desktops.

**Renaming.** A middle click on a mark, docked or floating, or "Rename" in its menu, opens a box with the desktop's name. Enter keeps it, Esc or clicking away leaves it, and an empty name goes back to "Desktop N".
- The name is set through Windows' own desktop manager, the one Task view uses (`IVirtualDesktopManagerInternal`), so Task view shows it at once.
- That interface is undocumented and changes between builds. Its id and the slots used (switch 9, find 14, set name 16) were read from 24H2/25H2's `twinui.pcshell.dll` and its symbols. On another build the id doesn't answer, and nothing is called.

**Desktop keys** (in the taskbar settings, empty by default):
- Next desktop and Previous desktop, with Windhawk's hotkey picker.
- Go to desktop 1 to 9: Alt, Ctrl+Alt, Shift+Alt or Win+Alt with a number key.
- They switch through the same manager, so the keys still held don't get mixed into sent ones. Where the manager doesn't answer, they fall back to Ctrl+Win+Left or Right, as a click does.
- Windows' own Ctrl+Win+arrows keep working.

The Hide/show hotkey now shares their reading. Each setting's name is still written out where it's read, because that's where the build adds the part's prefix.

**Known limits:** not tried on a real PC yet.

## The b51 kit

A tester got "Failed to parse settings: instance[0].trayIcons[1].icons has an unsupported value type".
- A tester's build has no tray icon or folder rows of its own, so those two lists were written with nothing under them. Windhawk refuses the whole settings block for that.
- With no rows, the build now writes one empty row, as the other lists already did. Both parts skip an empty row.
- Kits are now checked by parsing their settings block, not just by reading their defaults.

## Bonsai: a cbonsai screensaver

Not part of the suite, but kept with it in `bonsai/` after the reset lost the first copy. `build.bat` compiles both with the C# compiler built into Windows.

- **BonsaiSaver.scr** grows a tree on each screen (or the main one only), holds it, then grows the next. Scientifica is built in, drawn at its own 5 x 11 pixels and enlarged by whole pixels, so it stays sharp. Twelve trees are grown off screen and the one that fills the screen best is shown.
- **bonsai.exe** grows the same trees in a terminal, with cbonsai's options (`-l -i`, `-p`, `-m`, `-S classic`).
- **Classic** is cbonsai's growth ported step for step. **Detailed** adds more side branches, a trunk that tapers from a root flare, and shaded leaf clouds lit from above.
- Its colours default to the palette: trunk #a95d5d lit #ff8f8f, leaves #64aa89 lit #15ffa2 with #6ba6a0 undersides, background #121c21. Every colour, and the tree's style and size, are in its settings.

**Known limits:** at 1080p a big tree only enlarges 2x, so it fills the height but not the width; a lower Life lets it go to 3x.

## Window manager phase 1: fix the floor (for b52)

The ask: phase 1 of the window manager and workspaces plan, with the Hybrid model (Windows' own desktops stay; the suite builds on them), and Shift+W left up to users.

**The tiling key.** Shift+W is gone as a fixed key. The tiling's chords are now held with a key the user sets ("Tiling key", Windhawk's hotkey picker), none until then, with the same risk wording as every other hotkey. It must be pressed with exactly its modifiers. Pressed alone it still does what it did. With Win or Alt in it, a key nothing uses is pressed between, so letting go doesn't open Start or a menu bar. Anyone who had "Shift+W shortcuts" on sets the key once.

**Layouts kept per screen and desktop.** N, [ and ], and a layout applied are kept for that screen on that desktop, across restarts (`switcher.tiling.choices`, 64 at most). A screen is known by its monitor's device path, so rearranging screens keeps them. Changing the Layout or Main window's share settings lets the choices go, so the new settings show.

**Monocle.** H, J, K, L and the arrows go through the windows in turn, round and round, instead of doing nothing.

**Desktop keys** (taskbar, all empty by default): send the window in front to desktop 1 to 9, to the next or the one before, and "Go along with a window sent". They use the same undocumented manager as renaming (move view 4, switch and move the window in front 10) and `IApplicationViewCollection` (view for a window, 6). On another build nothing is called and the key beeps.

**Straight switching.** Pager clicks, the wheel and YASB workspace buttons switch through Windows' manager, like the keys already did, falling back to Ctrl+Win+Left or Right.

**Focus border** (off by default): the window in front draws its thin Windows 11 border in the accent colour or one of the user's, tiled windows only or every window, and gives it back when the focus moves.

**Idle cost.** The location-change hook is gone: it woke the switcher for every move any window made, the mouse pointer's included. Frames are now noted when a window opens, is left for another, minimises, is let go after a drag, or hides. The other hooks are only set while the tiling, the places or the border is on, so with all three off nothing is followed. "Remember where every window closed" is now off by default.

**YASB.** `{name}` shows the desktop's own name (its number while unnamed), and the tip shows the name.

**Known limits:** not compiled or tried on a real PC yet. A window snapped with Win+arrows and closed without ever losing the focus reopens where it was when it was last focused.

### Window borders, after Tacky Borders

The ask: something like Tacky Borders in the suite, with Tourne's own Tacky config as the defaults.

**What it draws.** A border round each app window (`borders.cpp`, in the Switcher, settings under "Window borders"): width, offset from the edge, square, rounded or the window's own corners, above or below the window, a colour or gradient for the window in front and another for the rest (hex with or without alpha, or `accent`; direction as `45deg` or start and end points as Tacky writes them), a clockwise or anticlockwise spin, a fade between the two looks, a glow and a shadow (Gaussian, as Tacky's), delays for a new or restored window, "only where Windows draws its own border", and programs, classes and titles left out.

**Defaults are Tourne's Tacky config**, off until turned on: width 1, offset -1, square, above, follow the native border, 200 and 150 ms delays, `#64aa89` to `#121c21` from bottom left to top right for the window in front and the reverse for the rest, a 2.5 s clockwise spin on the window in front, a 1.5 s eased fade, 60 fps, glow and shadow of 1 at full opacity on every window, and the same rules (CoreWindow, XamlExplorerHostIslandWindow, tacky-borders, zebar, komorebi-bar, keyviz, Picture-in-Picture).

**How it's drawn.** Four see-through layered strips per window (top, bottom, left, right), only as deep as the border, its corner curve and its glow and shadow need, instead of one window the size of the window it frames: a 1920x1080 window's border is about 50,000 pixels, not two million. Each pixel's coverage (border, glow, shadow) is worked out once per size and kept; a frame of the spin or the fade only looks colours up along the gradient. Drawn in software into a DIB and given to DWM with `UpdateLayeredWindow`, no Direct2D.

**Idle cost.** While the borders are off nothing is followed. While they're on, Windows' location-change event is followed again (as Tacky does) so the strips move with their window; a move moves the strips without drawing them again. The animation timer runs only while a border waits, fades or spins: with a spin set it runs at the chosen fps for the window in front, as Tacky's does.

**Tacky's "Access is denied".** Next to a program running as administrator Windows refuses to put the strips beside it in the z-order; the strips are then moved and shown without the z-order change, one by one, instead of failing.

**Checked:** the drawing code rendered on Linux to pictures (Tourne's look at 100% and 150%, turned, inactive; a wide rounded gradient with a deep glow and offset shadow, turned and mid-fade); the switcher compiles as before (x64 syntax check, the one error is the existing `part_system_icons` one). Not tried on Windows yet: turn Tacky Borders off first, or both draw.

## Tourne Desktop and the Tourne theme (auto theming, step 1)

**Tourne Desktop** (`desktop/`): the parts that need no hooks (the taskbar, the switcher with tiling and borders) built into one program of their own, from their unchanged source, with a Hyprland-style text config (`%APPDATA%\Tourne\Desktop\tourne.conf`) that applies when it's saved. See `desktop/README.md`.

**The Tourne theme.** Tourne Desktop can make colours from the wallpaper (or one seed colour) and keep them in one record, `HKCU\Software\Tourne\Theme`: each of the suite's roles as `0xRRGGBB`, what they were made from, and `Revision`, written last. It then posts `TourneThemeChanged` to every top-level window. The taskbar, System Icons and the Switcher have a new colour scheme, "Tourne theme", that reads the record (`theme-palette.cpp`) and follows it when that message comes.

**How a palette is made** (`desktop/host/palette-gen.cpp`): the picture is read at most 128 px across through WIC, its colours are grouped (k-means, started the same way every time) in OKLab, and each role is given a lightness of its own: dark grounds tinted a little by the picture's main colour, the accent from its most colourful part (or a small strong colour in a grey picture), a second accent of another hue, and fixed hues for warn, bad and good. Every text colour is then moved until it meets the contrast asked for (WCAG, 4.5 by default; text itself at least 7). Profiles: adaptive, monochrome, muted, vivid, pastel; light or dark follows Windows unless set.

**Idle cost.** None while nothing changes: the Themes folder (where Windows rewrites its copy of the wallpaper on every change, slideshows and Spotlight included) is watched on a thread that sleeps until it changes, a picture is read once on its own thread, and the record is only written when the colours differ. Off by default (`theme.source = off`).

**Checked:** the palette maker on Linux with nine wallpapers and every profile (all text passes); Tourne Desktop builds with MinGW. Not tried on Windows yet.

### CSS injector

Tourne Desktop can put your own style sheets into apps made of web pages (`desktop/host/css-injector.cpp`): it starts the app with Chromium's DevTools port on a random local port, attaches to every page (and every page opened later) and adds a `<style>` kept last in `<head>`, through `Page.addScriptToEvaluateOnNewDocument` and `Runtime.evaluate`. Saving a style sheet restyles the app at once. Known apps are found on their own (Discord and its PTB and Canary, Claude, Spotify, Slack, Obsidian, VS Code; Squirrel apps from their newest `app-*` folder); any other is given by its program.

Tourne writes a starter style for each (`css\tourne\`) from the theme's colours, again whenever the theme changes: the colours as `--tourne-*` variables, scrollbars and selection, and the colour variables of Discord, Claude and Spotify. Those app variables are a starting point and unverified.

Nothing is applied until the user says yes to a warning (the app doesn't support it; an update can undo it; the DevTools port can be used by other programs on the PC while the app runs). An app opened without its styles is noticed when it comes to the front (an out-of-context foreground event, nothing else) and the user is asked to restart it styled.

**Checked:** the injection script and the protocol steps on Chromium (styles applied, kept after a reload and after the page adds its own, updated live, removed); the JSON reader and the CSS templates on Linux; Tourne Desktop builds with MinGW. Not tried on Windows yet, nor in a real Electron app.

### Tourne Studio (step 1 of the Studio, glass and sets plan)

One window for every Tourne Desktop setting, the way HyprMod shows Hyprland's (ideas only; HyprMod is GPL and none of its code is used). Open it from the tray menu ("Open Tourne Studio") or with `TourneDesktop.exe --studio`, which also starts Tourne Desktop if it isn't running. It runs nothing until it's opened and nothing while it sits idle.

![Tourne Studio under Wine, wearing a theme made from one colour](images/40-tourne-studio.png)

- **Pages from the schemas.** A page per part (Taskbar; Switcher, tiling and borders), plus Tourne theme, Style apps and Tourne Desktop. Each setting shows with its name, description, choices and limits, read from the part's own Windhawk settings schema (`ParseSchemaDefaults` now keeps them), in the schema's order and grouped as it groups them. Switches, choice menus, sliders (for settings with a minimum and maximum), numbers, text and colours.
- **Search** across every page: names, descriptions, choices and keys.
- **Live, then kept when you save.** A change shows at once (the part is told, as on a config save) but is only kept when you save. Until then the bar at the bottom counts what isn't saved, with Undo, Redo (Ctrl+Z, Ctrl+Y), Discard and Save (Ctrl+S); "Changes not saved" lists them on one page. Closing or quitting with changes asks first.
- **Your file stays yours.** The Studio writes only `tourne-gui.conf`, read after `tourne.conf`, so its value wins for the same setting. Each row says where its value comes from (tourne.conf, Studio, Studio over tourne.conf, not saved); Reset takes a setting out of the Studio's file, and Revert drops a change not saved. Editing either file by hand still applies at once, and the Studio picks it up.
- **Theme roles for any colour.** A colour can be one of the theme's roles (`@accent`, `@ground`, `@text` and the rest). The host puts in the theme's colour and puts it in again on `TourneThemeChanged`, so the setting follows the wallpaper. Before there's a theme, the default shows. Colours can also be picked (Windows' colour picker, with the theme's colours as its custom colours) or typed.
- **It wears the theme.** The Studio uses the Tourne theme's colours when there is one (title bar included), else Windows' light or dark.
- Tourne Desktop now has a manifest (current common controls, per-screen DPI), built in when a resource compiler sits beside the C++ one.

**Checked:** the schema reader against the old one on both parts' schemas (same defaults, minus two junk keys the old one made from folded option labels); Tourne Desktop builds with MinGW; under Wine (Xvfb, driven with xdotool): pages, search, switches, choice menus, the colour menu with theme roles, typing a value, dragging a slider (one undo step), undo, save to `tourne-gui.conf`, a hand edit of that file picked up, the theme made live from the Studio and the Studio recoloured by it, and the ask-to-save on quit. Not tried on Windows yet, nor with the parts running.

### Plans and research (2026-10-04)

Kept in `research/plans/` so the reasoning lives next to the code:

- `handoff-2026-10-04.md`: where things stood at the start of this round.
- `hyprland-wiki-parity-map.md`: every Hyprland wiki variable, dispatcher, rule and animation (878 rows), what Tourne has today, the Windows route, whether it's possible, and its build phase (P1 control to P7 pieces and sets).
- `wallpaper-manager-proposal.md`: the wallpaper manager (library, per-screen apply, recolourer, palettes, theme hand-off).
- `studio-glass-and-sets-proposal.md`: Tourne Studio (HyprMod's feature set on Windows), system-wide glass, one-click sets, auto palettes, PrinceWM, komorebi and AltSnap presets.

Decided by Tourne ("go with your recommendeds"): Studio core first; see-through glass per app only when turned on; Material You as the default palette maker; the PrinceWM canvas as a Switcher style; the wallpaper manager drawn with Direct2D, phases 1 and 2 together. Build order from here: Studio core (done above), glass, palettes, sets, window manager presets, pieces, with the wallpaper manager beside palettes and sets.

### Glass (step 2: the engine, the Studio, rims and app glass)

Liquid glass for Tourne Desktop (`desktop/host/glass-engine.cpp`, `glass.cpp`), off until `glass.enabled` is on. Tourne picked "Studio and rims first": this round is the engine, the Studio's Glass page (the Tuner), a test window, the Studio wearing glass, glass rims round the window in front, and see-through glass inside chosen apps. The taskbar and the switcher get it in a later round.

![The seven looks, drawn by the engine on a test picture](images/41-glass-looks.jpg)

- **The engine.** A Direct3D 11 pixel shader, compiled when first needed (`d3dcompiler_47.dll`, part of Windows). Over a rounded shape it bends what's behind at the edge (refraction along the edge's normal), splits the colours there (fringe), blurs (12 taps at a mip level), can magnify the middle like a dome (lens), tints, sets saturation and brightness, glows at the rim (fresnel), takes light from above (highlight, plus a thin bright line round it), casts a soft shadow, and can carry a moving oil film. The result is read back as premultiplied pixels for a layered window or the Studio's own drawing.
- **What's behind it is the wallpaper.** The backdrop is the desktop as Windows draws its wallpaper, composed once from each screen's picture and how it's placed (fill, fit, stretch, tile, centre, span), and again only when Windows rewrites its wallpaper copy. It is not the windows in between: Windows gives one program no cheap way to see another's pixels, so this is the honest version, and it costs nothing per frame.
- **Looks.** Clear, Liquid (Apple-ish), Frosted, Aero, Smoke, Prism and Oil film. A look is a starting point for every glass setting; a setting you set yourself wins over it, and choosing a look in the Studio puts back the ones you changed there (one undo step).
- **The Tuner** is the Studio's new Glass page: the look, every setting as a slider or colour (theme roles work, so the tint can follow the wallpaper), an "Open test window" button (a glass card you drag over the wallpaper, which works even with glass off) and "Hold to compare" (glass off everywhere while held).
- **The Studio wears it.** With glass on, the Studio's background is glass with its panels laid over it, see-through enough to show it. `glass.studio` turns that off.
- **Rims.** `glass.rims.on`: a glass rim (width and corner settable) round the window in front, drawn as four thin layered windows so only the rim is redrawn. It follows the window through per-process window events (moved, minimised, hidden, closed) and a foreground event, and glows briefly when a window comes to the front.
- **Glass inside apps.** `glass.app.<name>.on`, per app: Windows 11's own Mica, Acrylic or tabbed backdrop, or "frosted" (acrylic blur behind, for Windows 10 too), and an opacity. Six apps are listed (Terminal, File Explorer, Notepad, VS Code, Discord, Spotify), all off; any program can be added by its exe. Turned off, or Tourne Desktop closed, and it's put back as it was. An app that draws its own background will hide the glass; that's the app, not something Tourne can change from outside.
- **Idle cost: none.** Nothing is drawn unless something just moved, changed or came to the front, or the oil film is showing (then for a few seconds after). With nothing using glass, the engine is stopped and its GPU memory freed.
- **Bugs found while testing:** the glass's window messages and timer shared numbers with the CSS injector's, so the injector swallowed the "backdrop ready" message and the Studio never got its glass. Glass now has its own range. The test window's label went straight onto the glass through a render target that may not keep what's already there; it's now drawn on its own and laid over.

![Tourne Studio with glass on (Oil film look) and the test window, under Wine](images/42-studio-glass.png)

**Checked:** Tourne Desktop builds with MinGW; under Wine (Xvfb, driven with xdotool): every look rendered by `--glass-preview` (shader compiled by Wine's d3dcompiler, run on its software Direct3D), the Studio wearing glass, the Glass page, choosing a look (sliders follow it, one undo step), the test window, hold to compare, and app glass applied to Notepad (logged; Wine has no compositor to show it). Under Xvfb there's no compositor, so the test window's transparent margin shows black there; on Windows it's see-through. Rims need a window manager to fire foreground events, so they weren't seen. Not tried on Windows yet.

Try the looks on your own wallpaper without starting anything:

    TourneDesktop.exe --glass-preview C:\looks.png --wallpaper C:\path\to\picture.jpg --look all

### Presets: 824 palettes and looks, one click each

Tourne asked for every preset that could be found. A research pass gathered 1,322 (base16 and base24 schemes, terminal themes, Omarchy, HyDE, hyprdots, end-4, ML4W, JaKooLit, gh0stzk, polybar themes, HyprPanel, YASB, windots, Windhawk's taskbar and Start styles, niivu); 824 of them have a full set of colours and ship built in. See `desktop/presets/README.md` for where each came from and what was left out.

![The Presets page, Kanagawa in use, under Wine](images/43-studio-presets.png)

- **The Presets page** opens first in the Studio: a grid of cards, each drawn in its own colours (ground, a panel with its accent, name, swatches), with filters (All, Picks, Dark, Light, Full looks) and the search box filtering them when you search from there.
- **One click.** Choosing a preset makes it the Tourne theme (`theme.source = preset`, `theme.preset = <id>`), sets the taskbar and the switcher to follow the theme, and, for a full look, sets the gaps, borders and corners it states. It's one step to undo, shows at once, and is kept when you save. Trying another preset takes the last one's look settings back out, so they don't pile up.
- **The theme engine** takes a preset's colours as they are (its own dark or light), writes them to the theme record and tells every piece, as it does for the wallpaper.
- **The switcher's "Colour scheme" style** reads System Icons' scheme, which had no page in Tourne Desktop. It now shows on the Switcher page next to the style.
- **Data:** facts only (colours, sizes, names, sources), in `desktop/presets/catalogue.json`, turned into `desktop/host/presets-data.inc` by `desktop/tools/make-presets.py`. Same colours from several places are kept once.

**Checked:** under Wine: the grid, filters, search, choosing a preset (the theme record made from it, the Studio recoloured by it), the unsaved list (theme, preset, taskbar scheme, switcher style and scheme), undo. Not tried on Windows yet.

### Windows' taskbar and Start off, with Tourne Desktop's taskbar

Tourne wants StartAllBack and Windhawk both off, Windows' taskbar and Start gone, and Windows back only after a crash. Turning Windows' taskbar off properly has to run inside Explorer, and the only part of Tourne there is the Windhawk relay. Until Tourne decides between keeping the relay or Tourne Desktop loading its own Explorer part (the Shell guard), the relay now serves Tourne Desktop's bar too:

- **It finds Tourne Desktop's bar.** It looked for the bar by a class name ending in its own mod id, so a bar from Tourne Desktop (`tourne-desktop`) was never found and Windows' taskbar was never hidden for it. Bars are now found by the class's prefix, for progress, hiding and Start.
- **The bar says what it wants.** The taskbar puts its wishes on its own window (`TourneShell_HideNative`, `TourneShell_StartKey`) and, from Tourne Desktop, in `HKCU\Software\Tourne\Desktop\Shell` (`HideTaskbar`, `StartKey`), which the relay reads as Explorer starts. So the relay works with the suite's own Taskbar part off; only the Explorer relay part needs to be on in Windhawk.
- **Start from anywhere.** The relay's "Start comes here" signal now also has a name without a mod id, so Tourne Desktop's Start drops its keyboard hook when Explorer sends it every opening of Start.
- **After a crash, Windows' taskbar comes back as it was.** When the bar's process ends without putting Windows' taskbar back, the relay restores the auto-hide setting the taskbar saved, as well as uncloaking it as before.

**Checked:** Tourne Desktop builds with MinGW. The relay change needs the Windhawk suite rebuilt on Windows (`Build-TourneSuiteRedirect.ps1`); it hasn't been compiled here.

### When Tourne Desktop crashes at start

Tourne's copy showed its tray icon for a second and closed. It runs under Wine here, so the cause has to come from Tourne's PC. Tourne Desktop now catches its own crashes:

- **crash.txt.** Any crash writes `%LOCALAPPDATA%\Tourne\Desktop\crash.txt`: the build, what was starting, the error and where it happened (each frame as module and offset), and the last 60 log lines, logging on or not. A message says where the file is and offers to open it. An exception nobody caught is reported the same way.
- **The culprit stays off.** Parts now start one at a time, a moment apart: the theme, app styles, glass, then each module. If the program dies while one is starting, the next start leaves that part off, says so, and runs the rest. Saving any setting tries it again.
- **Naming the address.** The build keeps `TourneDesktop.symbols.exe`, the same program with its function names left in, so an offset in crash.txt can be named.

**Checked:** under Wine, with a crash put in on purpose while the switcher starts: crash.txt named the switcher and the function, the next start left the switcher off with a notice and ran the taskbar, and saving `tourne.conf` started the switcher again.

**The crash on Tourne's PC, found.** Tourne sent ten crash dumps from Windows. All ten were the same fault. When a window came to the front, the app styler checked whether it was one of the apps to style. It split that app's program path at its last backslash, and a path with no backslash (a bare program name, or one typed with forward slashes) threw an error nobody caught. Forward slashes are now read as backslashes, and a bare name no longer throws. The crash report itself had a flaw too: its message box let the window that had just failed run again, which failed again inside the report and ended the program before the report finished. The message now waits on a thread of its own, and crash.txt also says what the uncaught error said.

### Style apps: programs found wherever they're installed

Tourne asked why the app styler couldn't find a program itself. It only looked in each app's usual folder. Now, when the program isn't there, or only its file name was given, it's looked for in order: the app if it's running, Windows' App Paths, the installed programs list (an entry whose icon is the program, or whose folder holds it), Start menu shortcuts (yours and everyone's, including a Squirrel app's `Update.exe --processStart` shortcut), and last the Windows search index. A Squirrel app found in any of these is started from its newest `app-x.y.z` folder. The search runs off the host's thread, once per app per run, and the log says where each was found. The path box in the Studio is now only for a different copy.

**Checked:** under Wine, with apps installed in odd places: one found running, one through App Paths, two through the installed programs list (one a Squirrel app with two versions, the newer picked), two through Start menu shortcuts (one a Squirrel `Update.exe` shortcut), and one not found, logged. The search index step can't run under Wine (it has no Windows Search); it's untested.

### Identity: one look for every piece

Tourne's design scope asks for one identity that every surface reads: change it once and the taskbar, switcher, borders, glass and apps all follow, like a Hyprland setup with hyprmod. The Tourne theme already did this for colour. The identity now does it for shape, material and motion: six `identity.*` settings (shape, frame, material, opacity, motion, speed), all `inherit` by default. When set, they work out the pieces' own settings (corner radii, border width, backdrops, opacity, hide and fade timings) as a layer under yours: defaults, then identity, then tourne.conf, then the Studio. A glass look gives way to the identity too. The Studio has an Identity page next to Tourne theme. It costs nothing while running: it's worked out when settings are read.

**Checked:** under Wine, with shape pill, material acrylic, motion snap and speed 200 set: the log lists the fourteen settings it derived, a border width set by hand stayed as set, and the Studio shows the Identity page with the four values marked as from tourne.conf.

### Identities: Wise Wolf, Everforest, Ghost Shell

The design scope's first test is Wise Wolf: every surface derived from one seed colour. The palette maker already had a seed source and a monochrome profile, but monochrome threw the seed's colour away (pure greys). It now keeps one faint tint of the seed through every role: a grey seed still gives greys, a blue-grey one a cool monochrome. Four identity packs come first on the Presets page, with an Identities filter: Wise Wolf and Wise Wolf Snow (seed #9AA3AD, monochrome, subtle, 1 px frames, acrylic, smooth and slower), Everforest (its fixed palette, rounded, 2 px, Mica, smooth) and Ghost Shell (the gits palette, MIT, square, 1 px, solid, snap). They live in `desktop/presets/identities.json`; their preview colours are the palette maker's own output for the seed. A pack's theme source replaces the preset's, and the tile shows "In use" when its source and preset are both current.

From the first test on Tourne's PC: the build script now passes `--target=x86_64-w64-mingw32` to clang++ (Windhawk's builds 32-bit by default, which fails on the switcher's monitor callbacks), and without a resource compiler it writes the manifest beside the exe instead of leaving it out. `theme.source = on` now means the wallpaper (an older tourne.conf had it, and every reload logged it as wrong).

**Checked:** under Wine: the Identities filter shows the four, picking Wise Wolf retints the Studio grey and marks it in use, picking Ghost Shell after it replaces Wise Wolf's settings rather than piling them up (9 pending changes). On Tourne's PC (built with Windhawk's clang++ before this fix, by hand with the flag): runs, the identity block applied and logged, no crash.

### The killswitch: Windows' taskbar and Start off without Windhawk

Tourne wants Tourne Desktop apart from Windhawk, and Windows' taskbar and Start gone. Everything that truly removes the taskbar runs inside Explorer (the Windhawk relay did it), and hiding its window from outside breaks Windows 11. So the killswitch does what can be done from outside, as a new host part (`killswitch.*`, off by default, a Studio page "Windows' taskbar off"):

- Windows' taskbar is set to auto-hide; the state before is kept in `HKCU\Software\Tourne\Desktop\Killswitch` and put back when it's turned off, when Tourne Desktop quits (after the taskbar part's own, so what was there first goes back last), at the next start if it's off, or with `--restore`.
- An edge guard: a topmost, all-but-invisible window 3 px deep over the edge each Windows taskbar waits at, so pointing there doesn't bring it up. Where Tourne's own bar lies on that edge the guard leaves a hole (a window region), so the bar keeps its clicks. It's raised again when another window comes to the front (a WinEvent, out of context).
- A low-level keyboard hook catches Ctrl+Esc, Win+T, Win+B and Win+1..0 (with the usual unassigned-key mask so Windows doesn't then open Start), and a lone Windows key unless Tourne's Start menu has that key.
- Explorer restarting (TaskbarCreated) puts it all back.

What it can't do from outside: an app flashing for attention can still make Windows' taskbar peek. The full kill would be Tourne Desktop as the shell (Explorer's taskbar and Start never start), asked of Tourne as a decision.

**Checked:** under Wine: on, its log line and saved state; the guard is a 1600x3 window at the edge with Tourne's bar cut out of it (the bar draws through and its Start still opens); Ctrl+Esc, Win+T and a lone Win open nothing; turned off live, the guard goes and the saved state is cleared. Not yet run on Windows 11.

### Shell mode, step 1: the switch, the watchdog and startup apps

Tourne chose shell mode as the killswitch: Tourne Desktop as the account's shell, so Explorer's taskbar and Start never start (plan: `tourne-research/reports/TourneSuite shell mode, views, effects, extensions and dots plan.md`). This first step is everything around it:

- **The switch** (`shell.enabled`, Studio page "Shell mode", off by default) keeps a copy of the exe in `%LOCALAPPDATA%\Tourne\Desktop\bin` (moving the old one aside when it's running) and sets `HKCU\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell` to it with `--shell`. Off, or `--restore`, deletes the value if it's Tourne's. Each start with it on refreshes the kept copy, so a newer build takes over at the next sign-in.
- **The watchdog** (`--shell`) runs Tourne Desktop (`--shell-child`) and waits. Exit 0 (you quit) starts Explorer for the session; a crash restarts it once; a second crash within two minutes starts Explorer and marks shell mode off (`ShellTurnedOff`) until a setting is saved, the way the crash guard leaves a part off. Shift held at sign-in gives Explorer once. Explorer is made the shell for the session by taking Tourne's value out while it starts and putting it back after. A copy already running answers with exit code 2 and is watched instead. Everything it does goes in `shell.log`.
- **As the shell**, Tourne Desktop tells Windows the shell is ready (`msgina: ShellReadyEvent`, `ShellDesktopSwitchEvent`) and, once per sign-in (a volatile registry key), starts the startup apps as Explorer would: HKCU RunOnce (deleted as run), HKLM Run (both views), HKCU Run and both Startup folders, skipping what Task Manager's Startup page has off and Tourne's own entry.

Next: the tray host (step 2, below).

**Checked:** under Wine: the switch sets the shell value and the kept copy, and a newer build replaces it; the watchdog starts Tourne Desktop, which starts a Run-key app and skips one Task Manager has off; quitting starts Explorer with the shell value kept; killing the child twice restarts it once, then starts Explorer and leaves shell mode off, and the next start doesn't set it again. Not yet signed in with it on Windows.


### Shell mode, step 2: the tray host

Without Explorer there's no tray for apps' icons, so Tourne Desktop now hosts one when it's the shell (`desktop/host/tray-host.cpp`):

- **A tray window apps can find.** An invisible `Shell_TrayWnd` with a `TrayNotifyWnd` inside, created only when no other tray exists. It takes `Shell_NotifyIcon`'s messages (add, modify, delete, set version, focus) and announces itself with `TaskbarCreated`, so apps that started first add their icons again.
- **Icons in Tourne's bar.** The bar already asks the suite's Explorer relay for every tray icon; the tray host answers the same question in the same packet, so the bar's tray, its places (tray, overflow, hidden), its order and drag and drop all work unchanged. A change tells the bar at once instead of waiting for its next check. Clicks go straight to the app, as before.
- **Each icon kept as a copy**, so an app destroying its own handle doesn't blank it; when an icon can't be copied, the app's program icon stands in. Icons of apps that closed without taking them out are dropped.
- Balloon tips are written to the log for now; they become Tourne notifications later.

Next: app bars and the work area (step 3, below).

**Checked:** under Wine, with a test app sending `Shell_NotifyIcon`'s Windows-format message: the icon shows in the bar's tray, the bar makes room for it and gives it back when it's removed, and mouse moves, left and right clicks reach the app. Not yet on Windows. (`TOURNE_TEST_TRAY=1` lets the tray host start beside another tray, for testing only.)

### Shell mode, step 3: app bars and the work area

Bars that keep an edge of the screen (Tourne's own bar, and other programs' docks and bars) ask Explorer for their place with `SHAppBarMessage`. In shell mode those requests reach the tray host, which now answers them (`desktop/host/app-bars.cpp`):

- **Places.** A bar asks for an edge; it's moved past the bars before it on that edge, and inside the ones on the edges either side, then kept. The others hear `ABN_POSCHANGED` when one moves, and bars whose windows are gone are dropped.
- **The work area.** Each screen's work area is the screen less its bars, so maximised windows stop at Tourne's bar. Maximised windows fitted to the old work area move with it; on quitting, screens are whole again.
- **The rest of the protocol:** where the taskbar is (`ABM_GETTASKBARPOS` gives Tourne's bar on the main screen), auto-hide bars per edge and screen, the taskbar state, and `ABN_FULLSCREENAPP` when a program fills its screen, so bars keep out of the way of games and videos. Windows hands the answers back in shared memory (`SHLockShared`), as it does with Explorer.

Next: the desktop (step 4, below).

**Checked:** under Wine, with a test program sending Windows-format requests: a bar is placed and answered, a second bar on the same edge stacks below it, a left bar fits under the top ones, the work area shrinks and grows back on removal, the first bar hears when the second moves, `ABM_GETTASKBARPOS` returns Tourne's bar, and only one auto-hide bar per edge is allowed. Not checked here: work areas as other programs see them, and the full-screen notice (Wine keeps work areas per program and sends no foreground events). Not yet on Windows.

### Shell mode, step 4: the desktop

Without Explorer there's no desktop window: no icons, no right-click menu, and programs that look for the desktop find nothing. Tourne Desktop now makes one in shell mode (`desktop/host/desktop-host.cpp`, `shell.desktop`, on with shell mode):

- **A window under every other one**, named as Explorer's is (`Progman`, "Program Manager"), across all screens, kept at the bottom and following screen changes. It shows the wallpaper Windows has set, as Windows paints it.
- **The icons are Windows' own folder view** of the Desktop folder, the same one Explorer hosts, so icons, their right-click menus, renaming, dragging, the desktop's own menu (View, Sort by, New, Display settings) and "Show desktop icons" behave as with Explorer. Folders open in File Explorer windows; files open in their apps.
- Tourne's wallpaper manager will draw here when it's built; this step only gives shell mode a desktop.

Next: quick toggles (step 5, below).

**Checked:** under Wine: the desktop window and the icon view come up, Desktop folder items show (a folder and a text file added for the test), the right-click menu opens, and a file opens in its app. Not checked here: the wallpaper (Wine paints it only in its own desktop process), icons keeping clear of the bar, and opening folders (Wine's folder view didn't open them). Not yet on Windows.

### Shell mode, step 5: quick toggles (the system icons, built in)

Windows' network, volume and battery icons are Explorer's own, so in shell mode they're gone with it, along with the Quick Settings panel (Win+A). The suite already has the answer: its System Icons part, pixel icons with flyouts for network, volume, Bluetooth, battery, brightness, night light, notifications, privacy and more. It runs in a tool process and only needs a tray to put its icons in, so it now builds into Tourne Desktop as a third module (`systemIcons`, off by default; skipped while it's on in the Windhawk suite):

- **The source is the suite's, unchanged.** It used three Windhawk helpers; `module-shim.inl` now stands in for them: string settings, and subclassing a window that belongs to another thread of the same program (set on that thread through a message hook, as Windhawk's helper does). Hooking other programs is still refused by the build.
- **The build** moves `ws2ipdef.h` up with `winsock2.h`, so `iphlpapi.h` declares its newer tables for every module.
- In shell mode its icons go to Tourne's own tray (step 2) and show in the bar.
- The tray now answers the Windhawk suite's bar too (it runs in Windhawk's tool process), so with the suite's taskbar in charge, shell mode still has tray icons. The right-click menus that open the original Windows icon's menu have nothing to open there.

**Checked:** under Wine, Tourne Desktop builds with all three modules and starts the system icons beside the taskbar and switcher without trouble. Their icons don't reach the bar under Wine (Wine's `Shell_NotifyIcon` speaks its own format, not Windows'), so the icons and flyouts in Tourne's tray are for the first Windows test.

Shell mode's five steps are now in. Next in the plan: the motion engine.

### Motion engine, round one

Tourne's animations now speak Hyprland (`motion.cpp`, shared by the suite's parts and Tourne Desktop):

- **Hyprland's lines, as they are.** An `animations { }` block from a Hyprland config pastes into `tourne.conf`: `animation = LEAF, ON, SPEED, CURVE[, STYLE]`, `bezier = NAME, X0, Y0, X1, Y1`, and the newer Lua-style `hl.animation({...})` and `hl.curve(...)`. They're stored as `motion.LEAF`, `motion.bezier.NAME` and `motion.spring.NAME`.
- **Hyprland's tree.** A leaf with no line takes its parent's (`windowsIn` under `windows` under `global`, and so on), with Tourne's `bar`, `start`, `popups` and `switcher` under `layers`. `motion.enabled = 0` stops everything that follows it. With nothing set, each part keeps the animation it had.
- **Curves and springs.** Cubic beziers (solved exactly, not sampled), the CSS and easings.net sets, Hyprland's `default` and `overshot`, and springs (mass, stiffness, damping; under-, critically and over-damped) whose length is worked out from when they settle: `rubber`, `gentle`, `snappy`, `bouncy`, `wobbly`, or your own.
- **What follows it today:** the bar's auto-hide (`bar`: speed, curve, and `slide`, `fade` or `slidefade`), CSS bar transitions (a `transition` naming a motion curve), and window borders (`border` fades, `borderangle` spins). The Identity's motion presets (snap, smooth, lively) write motion lines as well.
- **Studio › Motion:** every leaf as a line, with its own small curve playing beside it; pointing at a line draws its curve large with a dot running along it and a block moving in its style (slide, fade, popin with its size). The previews run only while the page is shown.

**Checked:** a unit test of the engine (inheritance, Hyprland and Lua lines, CSS `ease` at 0.5 = 0.802, the `rubber` spring settles in 1.22 s overshooting to 1.096), and under Wine: Tourne Desktop builds and runs with it, a Hyprland block in `tourne.conf` shows on the Motion page, and the previews play and follow the pointer. Not yet on Windows.

Next in the plan: switcher views (Flip 3D, Cover Flow, Expo).

### Switcher views: Flip 3D and Cover Flow

Alt+Tab can now show windows in 3D (`switcher-views.cpp`, inlined into the switcher; `switcher.view.style`, the list by default):

- **Flip 3D:** a slanted stack going back and up to the right, the chosen window in front. Stepping sends the front window flying past you while the next comes forward, and it goes round (the shortest way to the new one).
- **Cover Flow:** the chosen window flat in the middle, the rest turned about 60 degrees to the sides and set back, standing on a floor that reflects them.
- **Both** open by flying each window from where it is on the screen into its place, over the screen dimmed and blurred (`view.dim`, `view.blur`). The chosen window's title shows below. They move as the motion settings' `switcher` line says (else 320 ms on Hyprland's default curve), so a spring there makes the stack bounce.
- **The scene** is Direct3D 11. Each picture is one quad with perspective, corners rounded by a distance field (the switcher's radius, else 8 px) and a soft shadow; reflections and the blurred backdrop come from the same shader. Titles use DirectWrite on the same device.
- **Pictures:** live through Windows' own capture API (Windows.Graphics.Capture; nothing injected, no yellow border on Windows 11, Windows 10 1903 and later), else a still from `PrintWindow` as it opens. Minimised windows, and windows that draw nothing for a still, show their icon and title on a card the window's shape.
- **Input:** as the list: Tab, Shift+Tab, the arrows and the wheel, Esc and Ctrl to drill into an app. A click switches (Cover Flow brings a side window to the middle first), a middle click closes, a click on the background goes back.
- **Idle cost:** none. Captures and the frame timer run only while the view is open; the device and window are made once a view is chosen and kept hidden so Alt+Tab opens at once.

**Checked:** under Wine: both views open, step with their animations (Flip 3D round the stack, Cover Flow sliding), draw the blurred backdrop, shadows, reflections and titles, and switch on commit. Wine has no screen capture API and its `PrintWindow` gives blank pictures here, so every window showed as its card: the live pictures are for the first Windows test. Not yet on Windows.

Next: more views (Expo grid, Mission Control, Ring, Shelf), then a Studio Views page with moving previews.


### First shell mode sign-in on Tourne's PC: fixes

Tourne's first real sign-in in shell mode came up black, with two screens flashing. Two causes, both fixed:

- **No taskbar:** a broken Windhawk entry left behind by a removed suite build (no Version, no Disabled) still had the taskbar, switcher and system icons on in its saved settings, so Tourne Desktop thought the suite was running them and skipped its own. Entries with no version are now ignored (ae2dc3a).
- **Black desktop:** with Explorer not running, nothing hands Windows' painter the wallpaper, so `PaintDesktop` paints black. The desktop now hands the wallpaper over again and draws it itself, as Windows places it on each screen, and logs what it did (300fec1).
- **Older copies:** a newer Tourne Desktop started while an older build runs now takes over from it and moves "start with Windows" to itself (not the shell-mode copy, which would end the shell) (ae2dc3a). The Studio's title bar shows the build (12f56a3).

The flashing stopped once the old build was gone; it was most likely the old build fighting Explorer's taskbar.


### Shell mode: Windows apps after a full restart

Tourne turned on the Windows apps switch and did a full restart. Windows' taskbar showed and Tourne's bar had no tray icons. Two fixes:

- **A restart isn't two crashes** (47477d8). Windows ended Tourne Desktop while shutting down (0x40010004), the watchdog started it again, and Windows ended that too (0xC000026B). The watchdog took the pair for two crashes at sign-in, turned shell mode off and gave the next sign-in to Explorer. With the session ending, or with those two codes, the watchdog now just stops.
- **Explorer slow to start.** After a cold boot Explorer can take longer than the watchdog's 20 seconds to show its taskbar, and Tourne Desktop then started without it, leaving Explorer's taskbar on screen when it came. Now, with Windows apps on and Explorer not up yet, Tourne Desktop waits for it for two minutes: its own desktop and tray are made as usual, but its desktop isn't made Windows' shell window yet (Explorer would see one and not start as the shell), and startup apps wait. When Explorer's taskbar appears it is hidden and taken on as if it had been first (desktop hidden, auto-hide, Tourne's tray first in line and the icons asked for again, its keys caught), and Explorer starts the startup apps. If it never comes, Tourne's desktop becomes the shell window and Tourne starts the startup apps. `shell.log` says which happened.

**Checked:** builds with llvm-mingw. Not yet on Windows.


### Switcher views: Grid, Mission Control, Ring and Shelf

Four more views for Alt+Tab (`switcher.view.style`), from the plan's section 4 (`research/plans/shell-mode-views-effects-extensions-dots-plan.md`):

- **Grid** (`grid`): every window at once in a grid that fills the screen, the last row centred. The chosen one is lifted toward you and glows in the highlight's colour; Tab moves the light.
- **Mission Control** (`mission`): the same grid, read the way your windows sit on the screen (in bands from the top, then left to right), so each lands near where it was. Minimised ones go last.
- **Ring** (`ring`): the windows stand in a circle on a reflecting floor, facing out; Tab turns the ring the shorter way round to bring the next one to the front. The far side dims and fades.
- **Shelf** (`shelf`): one row like a dock, every window in it, magnified under the pointer with the neighbours moving aside; from the keys the magnifier follows the chosen one.
- In the grids and Shelf a click switches straight away; in Cover Flow and Ring a side window comes to the front first.

The plan's Expo (each virtual desktop as a tile), Spaces, Cube, Easymotion and the desktops strip over Mission Control are still to come.

### Import from the Windhawk suite, and the peek

- **Import** (tray menu "Import my Windhawk suite settings", or `--import`, `--import b53`) takes the suite's settings for the taskbar, switcher and system icons, running or switched off in Windhawk, and puts what differs from the defaults in `tourne-gui.conf`, so they win over `tourne.conf` and the Studio shows them (Reset takes one back). Which parts run is left as it is; the old file is kept as `tourne-gui.conf.before-import`. Before, it only read a running build and, with a `tourne.conf` there, left an `imported.conf` to copy by hand.
- **Peek:** hovering a taskbar preview faded Tourne's own desktop with every other window, leaving grey. It's now excluded from peek, as Explorer's desktop is.

**Checked:** builds with llvm-mingw. Not yet on Windows.

### Shell mode: icons moved to the overflow came back gone

Moving a tray icon into the overflow made it disappear: it left the bar, and the overflow flyout showed nothing. In shell mode Tourne Desktop hosts the tray itself, and its tray answered the icon request only from the bars (window class `TourneTaskbarTest_*`). The system icons ask from their own window (`TourneTraySystemIcons_*`) to fill the overflow flyout, so they were turned away and the flyout stayed empty. The tray now answers both (`IsTourneAsker` in `desktop/host/tray-host.cpp`). Not related to Windhawk: the placement (`HKCU\Software\TourneTray\TrayPlaces`) was saved all along.

**Checked:** builds with Windhawk's compiler. Not yet tried on Tourne's PC.

### Shell mode: a swap's restart isn't a crash; app styles' search index one at a time

Swapping in the overflow fix (2026-10-06 15:43) cost Tourne the shell for the rest of the session. The swap ended the shell child with `taskkill` (exit code 1), which the watchdog counted as a crash and answered by starting the new build. That build crashed a second later in app styles' startup ("It tried to run 0x0", no stack), the second crash inside two minutes, so the watchdog brought Explorer back and turned shell mode off. Two fixes:

- **Planned restarts** (`kPlannedRestart`, exit code 3). The watchdog starts the new build again and counts no crash. It marks itself in the volatile key `HKCU\Software\Tourne\Desktop\Watchdog` (`PlannedRestart`), and `Swap-TourneDesktop.ps1` now ends the child with code 3, refuses under an older watchdog unless `-Force`, and reports a crash report written right after the swap. A watchdog only changes at sign-in, so this protects swaps from the next sign-in on.
- **App styles' program search.** At startup every app not found yet searches on its own thread, and the three left (Slack, Discord PTB, Discord Canary) reached the Windows search index together, where the crash came. That query now runs one at a time. No dump was written (Tourne's own handler caught it), so the cause is likely, not proven. It wasn't the overflow fix: the crash came 2.5 s before the taskbar and the system icons started.

**Checked:** builds with Windhawk's compiler. Not yet run on Tourne's PC.

### Plugins: a Plugins section in the Studio, from the Noctalia catalogues

First step of plugins as Tourne decided them. `tools/plugins/Make-PluginIndex.ps1` scans Noctalia's two plugin catalogues and keeps the 30 that can run on Windows (the rest call Linux tools), with what each needs, in `desktop/plugins/index.json`. The build puts that index into the exe. The Studio's new Extensions page (after Presets) shows them as cards: name, author, licence, what it does, where it shows (Bar, Panel, Desktop, Launcher, Quick toggle, Background) and what it needs, with filters. Install first lists everything the plugin will be allowed (internet and which hosts, its own files, clipboard, notifications, programs it runs), then downloads it from GitHub at the scanned commit into `%LOCALAPPDATA%\Tourne\Plugins\<author>\<name>` and records the grant in `tourne-plugin.json`. Remove asks too. Code: `desktop/host/plugins.cpp`.

The plugins don't run yet: the Luau runtime comes next, then placing them on the bar, panel and desktop.

**Checked:** Tourne Desktop builds with Windhawk's compiler. `tools/plugins/plugintest.cpp` (build line at its top) reads all 30 cards, then installs World Clock (15 files from noctalia-dev/official-plugins at 2337829) and removes it. The Studio page itself is not yet tried on Tourne's PC.

### Plugins run: Luau, and their widgets on the bar

Installed plugins now run. Tourne Desktop has the Luau VM built in (`desktop/third_party/luau`, release 0.741, MIT) and a runtime on a thread of its own (`desktop/host/plugin-runtime.cpp`). Each of a plugin's services and bar widgets runs in a VM of its own, as in Noctalia, sharing only `noctalia.state`. Panels, desktop widgets, quick toggles and launcher providers don't run yet: a plugin that asks for one says so once in `desktop.log`.

- **What a plugin may do** is what it was granted on install (`tourne-plugin.json`): it reads its own folder and its data folder (`%LOCALAPPDATA%\Tourne\PluginData\<author>\<name>`, kept apart so a reinstall keeps it; Remove deletes it too, as the Studio says), writes only the data folder and only with "files", reaches the internet only with "internet" and only the hosts the index lists for it, and shows notifications or touches the clipboard only when granted. No `io`, no `os.execute`, no loading code. A call into a plugin is stopped after 2 s and a VM at 64 MB, and the plugin keeps running.
- **The API so far:** settings (`plugin.toml` defaults, read by the new `toml-lite.cpp`; then `plugins.<id>.<key>` or `plugins.<id>.<entry>.<key>` in the config, e.g. `plugins.noctalia/world_clock.show_clocks = 1`), `state`, `json`, `tr`/`trp` (the user's language, then English), files, `formatTime` with IANA time zones (Windows' own ICU), `timeFormat` from Windows' clock setting, `http` (WinHTTP, off the thread), notifications (a balloon from the runtime's own tray icon), the clipboard, colour roles from the Windows accent, and `barWidget.*` with `ui.*` trees flattened to a glyph and text. Commands (`runAsync` and friends) answer "no".
- **On the bar** (`plugin-bar-shared.cpp`, inlined into the taskbar): widgets show on the main bar just before the tray, glyph then text, in the clock's colour, with their tooltip; clicks, right and middle clicks and the wheel reach the plugin. Glyphs are Tabler icons (MIT), whose font is built into the exe and written once to `%LOCALAPPDATA%\Tourne\Fonts` (`tools/plugins/Make-GlyphMap.ps1` makes the name map). The bar only ever posts to the runtime, so a plugin that hangs can't hold the taskbar up. YASB themes don't place them yet.
- Installing, removing and changing a `plugins.` setting restart the runtime. The exe grew from 4.2 to 7.5 MB (Luau and the font).

**Checked:** builds with Windhawk's compiler. `tools/plugins/Run-PluginTest.ps1` (outside Tourne Desktop, a scratch folder, a stand-in bar): World Clock shows its globe, and with `show_clocks` the four clocks with the right times (daylight saving included); the Example's widgets show "Hello", the plural tooltip, its data file and the service's tick, and a click notifies. With `-Sandbox`, a plugin granted nothing is refused writing outside its data folder, reading a neighbour and the internet, can't replace the API, and is stopped in an endless loop and at 64 MB, then keeps running. Not yet run in Tourne Desktop itself or seen on the bar. The Windhawk suite build (which inlines the same taskbar code) wasn't rebuilt.

### Shell mode: other programs' tray icons in the Tourne design again

Tourne's tray looked unlike the Windhawk suite's in shell mode: HWiNFO's temperatures sat on grey squares, and programs kept their own icons (b53 showed the pixel designs and the "use everywhere" icons). In the suite, System Icons swaps them inside Explorer as they reach its tray. In shell mode the tray is Tourne Desktop's own (`tray-host.cpp`), so nothing swapped them; `tourne:` designs on task buttons and in Alt+Tab were also stubbed out (`standalone.h`).

Now the tray host asks System Icons, which runs in the same process, for each icon's design as it packs the icons for the bars and the overflow flyout (`TrayDesignIcon`: the "use everywhere" icon, else the design for its program and tooltip, sensor readings included). The taskbar and switcher get its `DesignIconAt`. Both go through the build's `modules.cpp` and only while System Icons runs. Its settings are now replaced under a lock, since these calls come from other threads.

**Checked:** on Tourne's PC (84f1603): the temperatures are green numbers on the bar, as on 2026-10-03. Icons inside Explorer windows, Start and programs' own windows come from Icon Redirect, which hooks other programs: that stays a Windhawk part.

### Plugins: panels

A plugin's `[[panel]]` now runs (`desktop/host/plugin-panel.cpp`). `noctalia.togglePanel` opens it near the click ("attached"), or where its manifest says ("floating", `height = "fill"` for the full height). `panel.render` draws its `ui.*` tree in System Icons' flyout look: its pixel fonts (ProggyClean, scientifica, Cozette) and its palette, square corners and an edge line. Rows and columns lay out with gaps, padding, `flexGrow`, `align` and `justify`; `scroll` scrolls with the wheel; labels, glyphs, buttons in their variants, inputs (Windows edit controls, kept by `key`, so typing survives a render), toggles, selects (a menu), sliders, progress bars, separators and graphs draw and answer. Esc or a click elsewhere closes it, and its second ticks run only while it's open. Not yet: images, tooltips, context menus, drag and drop (drawn as plain containers).

Events never call the plugin from inside a window's message. Each is queued with its node's identity (its keys, or type and place, from the root) and the handler is looked up again when it runs, so a handler that re-renders can't pull a control away mid-message, and an event on a node that's gone does nothing.

Also: Noctalia's glyph aliases (67 names like `close`, read from its `glyph_registry.cpp` by `Make-GlyphMap.ps1`); `noctalia.expandPath`; a folder named by a plugin's `folder` setting counts as its own (Notes keeps notes in `~/Documents/Notes`); colour roles from the flyouts' palette, `role/0.6` included; bar items in the flyouts' ink unless the plugin picks a colour.

**Checked:** `Run-PluginTest.ps1` (a scratch folder, pictures of the panel window): Notes' panel opens, and "+" makes a note and opens its editor; World Clock's panel lists its zones with their times and buttons. Swapped in on Tourne's PC (22347a2), clean start. Not yet clicked through by Tourne.

### Studio: a Plugins page

Each installed plugin's settings (its `plugin.toml`'s and its entries') join the defaults and the schema as `plugins.<id>.<key>`, named and described from the plugin's translations: switches, choices with their labels, numbers, text, plus a "Run it" switch per plugin. The new Plugins page shows them; they're read again after an install or removal, and a saved change restarts the plugins.

**Checked:** the rows for Example, Notes and World Clock come out as switches, choices and text with their English names (Run-PluginTest). Swapped in (d406daa). Not yet opened by Tourne.

### Shell mode: tray tooltips, the clock and switcher from b53

The hidden icons flyout now tells an icon's program when the pointer is over it, as the taskbar's tray does, and reads the tooltip again a moment later. AlienFan writes its tooltip only then, so in shell mode it had none. Settings that Windhawk fills in as it runs (dynamic selects, such as the switcher's style) are dropdowns in the Studio, read from the part's own values. `--import` now also brings the clock, tray folders and icons, menus and text colour.

**Checked:** builds and swaps in clean (1807619). Tourne's clock is set to red over green from their palette, and their switcher style is Tourne'Style again. Not yet seen by Tourne. The system-wide text colour is still a Windhawk hook part.

### Tray folders by hand

The hidden icons flyout edits its folders: drag an icon onto a folder to put it in, onto the open folder's name to take it out, or onto another icon to make a new folder of the two and name it there. Right-clicking a folder renames it, sets its glyph or deletes it. Changes are kept beside the tray's places, so Tourne Desktop and the Windhawk build share them; "Folders as in the settings" goes back.

**Checked:** builds and swaps in clean (4099eb0). Not yet tried by hand.

### Icon Redirect in Tourne Desktop

Since b53 stays out of TourneDesktop.exe (so its menus aren't styled twice), nothing themed the icons Tourne Desktop draws: the desktop, Start's places, menus and dialogs showed Windows' own icons rather than the Bouquet fixed-alpha theme. The suite's Icon Redirect source now builds into Tourne Desktop as a part "beside the suite" (`BesideSuite` in Build-TourneDesktop.ps1, `besideSuite` in td::Module). It hooks Tourne Desktop's own process only (`desktop/host/hooks.cpp`, MinHook 1.3.4 vendored in `desktop/third_party/minhook`) and reads b53's `iconRedirect.*` settings as they are in Windhawk, plus the themes Windhawk downloaded (its mod-storage folder). It runs while b53 has it on, starts first (before the desktop and bars load icons), has no Studio page, and stands down if a suite build is loaded in the process. Changed rules take effect when Tourne Desktop restarts.

**Checked:** swapped in (693afc8): "106 rules, 47 files" in desktop.log, and FindResourceExW, LoadResource, SizeofResource, PrivateExtractIconsW and LoadImageW start with a jump in the shell child. Not yet seen on screen by Tourne.

### Start and phone sizing, phone docking, desktop selection colours, settings items that open

- **Start menu** resizes by its edges away from the bar (the programs list takes the room; HKCU\Software\Tourne\StartMenu StartExtraWidth/Height; "Size as it comes" in its empty-space menu). Place menus open beside the places, over the phone pane, instead of past it.
- **Phone apart**: any edge resizes it (a taller window shows more notifications); dragged to a side of a bar, the desktops pager or Start it snaps and docks, and follows that window from then on (WinEvent location changes, this process only). Shift while dragging: no dock. Its menu: Undock, Size as it comes, Phone settings...
- **Desktop icons** (`shell.iconLook = tourne`): hover, picked and the keyboard outline in hex or Tourne theme colour names. The view draws them with the visual style's ListView item (states 2, 3, 5, 6) and DrawFocusRect, not custom draw (which never reports picked here; `tools/desktop/theme-probe.cpp`), so those two calls are hooked in Tourne Desktop and answered only while the icons' view paints.
- **Dead items fixed**: the parts once ran inside windhawk.exe and opened their settings by starting it again; in Tourne Desktop that started a second copy, which exits. Tourne taskbar settings, Start's Properties (both menus), the pager's look item and tourne:studio now open Tourne Studio filtered to the right settings (`td::OpenStudio`, `StudioOpenAt`). The Tourne Menu's LiteMenu settings item copies LiteMenu's menu into `taskbar.menu.items` once (`td::WriteStudioSettings`, backup `.before-litemenu-copy`) and opens Studio there; its exit item closes LiteMenu. Win+X's Run, Search and Desktop, System Icons' "Open notification center", and the clock's/YASB notifications use Tourne's own in Tourne Desktop.

**Checked:** builds, swapped in (dd425ed), DrawThemeBackground and DrawFocusRect hooked in the shell child, `shell.iconLook = tourne` set in Tourne's conf (backup `.before-icon-look`). Not yet seen or clicked through by Tourne.

### Package logos, suite b54 installed, Studio pickers

- **Icon Redirect reaches packaged apps' logos.** Task Manager, Start and the switcher show a packaged app's logo PNG (resolved through resources.pri and opened with CreateFileW), which no icon rule touched; Claude's group row in Task Manager was the stock logo. CreateFileW and CreateFile2 are hooked for .png/.jpg reads under a package's folder: the package's program (AppxManifest `Executable=`) or a direct rule picks the icon, it's drawn at the size the file name asks for (targetsize-N, scale-N, NxN, StoreLogo 50) with WIC, kept in `<mod storage>\package-logos\<hash>-<size>.png`, and the read goes there instead. In suite b54 and in Tourne Desktop.
- **Suite b54** (template gains the D3D11/WinRT Graphics headers it needed, and `@exclude TourneDesktop.exe`). `tools/windhawk/Install-SuiteBuild.ps1 -Build b54 -Replace b53` compiles it with windhawk-cli, imports the settings the build script wrote, carries the old build's exclusions over (excludeCustom), turns b53 off and b54 on, with the Mods key exported first.
- **Studio: pickers instead of text boxes.** Settings that take programs, window titles or classes, an icon, a folder, a file, a hotkey, a font, style sheets, a command, a motion line or a gradient now show what's set in a dropdown-style box, and clicking it offers the real choices: running programs, the icon picker, file and folder dialogs, a key-capture box, the font dialog, Tourne's actions, motion style/curve/speed, two colours. "Type it out..." is still there in each. Bounded numbers are sliders. Text boxes went from 126 to 11, bare number boxes from 39 to 0.
- **Studio search** typed on the Presets page falls through to settings when no preset matches.

**Checked:** b54 compiles (x64 and x86) and is the one Explorer loads; its settings match b53's (calendarIcs empty, unused: System Icons is off in the suite). Logos for Claude cached at 16, 32 and 44 px; Task Manager's Claude row shows the asterisk. Tourne Desktop swapped in (df43154); Studio pictures show the pickers on "leave out", "motion" and "program". Not yet clicked through by Tourne.

### Phone texts: Tourne holds them over Bluetooth MAP

Phone Link has no way in for another program, so Tourne talks to the iPhone's Message Access Server itself, as a car kit does (`phone-map.cpp`, inlined after phone-link.cpp, Tourne Desktop only; the suite's build gets stand-ins). One thread holds the session (RFCOMM to MAS 0x1132, OBEX CONNECT with MAP's target, SETPATH telecom/msg), registers for notices, reads each new inbox text (GET x-bt/message, UTF-8) and sends queued replies (PUT x-bt/message into outbox, split to the phone's packet size). A second thread is MAP's notification server: an RFCOMM listener with its own SDP record (MNS 0x1133, MAP 1.0) that the phone connects back to with event reports. If notices can't be set up, the inbox listing is read every 20 seconds instead. Only one program can hold the channel: while PhoneExperienceHost.exe runs, Tourne waits on its process. Phone away: tried again after 1, 2, then every 5 minutes, and at once when Start or the phone opens; connecting is non-blocking so ending is quick.

An iPhone shares only texts that come while connected, so what came is kept in `%APPDATA%\Tourne\Desktop\phone-messages.tsv` (latest 500). Each new one is also a notification of its own (AUMID `Tourne.PhoneTexts`, named "Messages"; its key under Notifications\Settings is made first so Tourne's pop-ups setting turns Windows' banner off before the first).

In the phone (Start's pane, its own window, the tray flyout): `startMenu.phoneMessages` (3, 0 to 8; 0 gives Phone Link's link back) conversations under a Messages heading, with the link state on the right ("Phone Link has them", "Not connected"). A click opens one: bubbles (yours in the accent's tint, "Sending..." / "Not sent"), a reply box (an edit control; Enter sends, Esc goes back) and Send. The phone's own window takes the keyboard for the box and Start stays open while it does.

**Checked:** `tools/phone/map-offline-test` (33 checks against a fake phone on loopback: bMessage parsing with a quoted-printable name and an emoji, LENGTH, listing over three packets, a PUT split to 64-byte packets with the handle back, the notification server taking a two-part event report) all pass. Swapped in (7cd95ca), starts clean; the log shows "phone texts: no session (10060)": the phone was out of reach all afternoon, so nothing has run against the real phone yet (`tools/phone/map-session-probe` is for that). The pane not seen on screen (the PC was locked). The suite's build of start-menu.cpp (the stand-ins) not compiled yet.

### Phone notifications: Tourne reads the iPhone's itself (ANCS); Phone Link off

`phone-ancs.cpp` (Tourne Desktop only, inlined after phone-map.cpp): Apple's Notification Center Service over Bluetooth LE, with C++/WinRT GATT on a thread of its own (multithreaded apartment). It finds the phone among paired LE devices by name, holds a GattSession with MaintainConnection (Windows reconnects when the phone is back), and subscribes to the Data Source, then the Notification Source (turned off first: the phone sends what it already has only when notices are turned on, and Phone Link may have left them on). For each notification it asks the Control Point for its app, title, subtitle, message, date and action labels, and the app's name once per app; the answers can come in parts and are gathered. Each becomes a Windows notification as its phone app (AUMID `Tourne.Phone.<bundle id>`, DisplayName as the phone names the app, its Notifications\Settings key made first so Tourne's pop-ups setting turns Windows' banner off before the first one). Ones already on the phone, and changes, come in quietly (SuppressPopup isn't kept in wpndatabase, so they carry `launch="tourne-phone-quiet"` and System Icons skips their pop-up). A notification gone from the phone is taken away (tag = its id, group `ancs`); on each connect the earlier ones are cleared and the phone's current set comes in again. Messages' notifications are left to phone-map.cpp while it holds texts. Both phone threads stand aside while PhoneExperienceHost.exe runs and log each change of link ("phone: texts: connected", "phone: notifications: 4 shown, 0 taken away").

The phone pane reads Tourne's notifications too (`phone-link.cpp`: handlers `Tourne.Phone.%`, named from the registration; unread is their count without a Phone Link badge). With Tourne holding the phone, Notifications opens Tourne's notification centre and the phone and Phone settings open Bluetooth settings; Send a file still opens Phone Link.

Phone Link off: its startup task was already off (State=1, real hive); Windows started it at 15:20 when the phone came near because of Settings, Mobile devices, Phone Link (`HKCU\Software\Microsoft\Windows\CurrentVersion\Mobility` `PhoneLinkEnabled`), set to 0 through Invoke-Real (was 1), and PhoneExperienceHost ended.

**Checked:** `tools/phone/ancs-probe test` (14 parsing checks) passes; `ancs-probe live` (dry run) read the phone's notifications with their app names. Swapped in (4b4f630): texts connected with the phone's notices on, notifications connected, "4 shown, 0 taken away", Windows' banners off for the three new apps first, "0 new to pop up". Not yet seen: a new notification popping, a text coming in, a reply going out, the pane on screen (the PC was locked).

### Phone: call buttons, Send a file over LocalSend, Windows calling probed

- **Answer / Decline (and Clear):** each phone notification carries the phone's own actions as buttons (ANCS positive/negative labels; arguments `tourne-phone:<0|1>:<id>`). System Icons' pop-ups and centre hand a click to the Start window (registered message `TournePhoneAction`), and `phoneancs::Perform` does it on the phone over the Control Point. A ringing call is marked `launch="tourne-phone-call"`, so its pop-up stays up 30 seconds.
- **Send a file:** `phone-files.cpp` sends to the phone's LocalSend app (open source, https://localsend.org; protocol v2) on the same network, with no Phone Link and no cloud. It finds the phone with one non-blocking connect per address of each /24 for about a second (WSAPoll), then asks the listeners for LocalSend's info, trying the last address first. All connections are outgoing, so there's no firewall prompt. Then prepare-upload (the phone asks you to accept, waited up to 2 minutes) and each file streamed over WinINet (https with LocalSend's self-signed certificate, else http; WinHTTP's header clashes with WinINet's in the taskbar part). The result comes back as a notification ("Phone files"). The phone pane's Send a file opens a file picker and sends.
- **Windows calling** (`tools/phone/calls-probe`, read-only): the phone is a phone-line transport (Bluetooth, audio routing possible, no app registered), but Tourne's access is "denied by system", and the call store times out. Phone Link declares `phoneLineTransportManagement` and `phoneCallSystem` (restricted). A packaged helper (a local package registered with Developer Mode on) is the way to try; Developer Mode is off. The hands-free speaker and mic endpoints appear only during a call; whether Windows or the app bridges them to the PC's own devices is to be tested in a call (Tourne wants the call on the PC's default mic and output).

**Checked:** `tools/phone/files-test` (a fake LocalSend phone on 127.0.0.1, 10 checks, both files byte for byte, an accented name in UTF-8) passes; `files-probe` finds no LocalSend on the network yet (1.2 s for 253 addresses). Swapped in (becc3b0): texts and notifications connected again within 8 s. Not yet tried: a real call's buttons, a real send to the phone (LocalSend isn't on it yet).

### Tourne Calls: the phone's calls through Windows, the sound on the PC's own devices

`desktop/calls/` is a helper with a local package identity (`Register-TourneCalls.ps1`, Developer Mode, external location `%LOCALAPPDATA%\Tourne\Desktop\calls`). Its manifest declares `phoneCall`, `phoneLineTransportManagement`, `phoneCallSystem` and `windows.phoneCallActivation`, as Phone Link's does; without the last two, RegisterApp silently did nothing. Windows allows it the phone's hands-free transport: registered, connected, with the hands-free Speakers/Microphone endpoints up. The call store needs the Phone Service (PhoneSvc), which was Disabled; Tourne set it to Manual and started it.

`TourneCalls.exe run` (started by Tourne Desktop, one at a time) follows the line's calls: status, sound device and mute are logged to `calls.log`. Commands come to its message window ("TourneCallsCommand": 1 answer with the sound on the PC, 2 decline or hang up, 3 mute, 4 and 5 sound here or there; WM_COPYDATA 6 dials), also from its command line (answer, hangup, mute, here, there). While a call talks with its sound on the PC, two WASAPI pumps run as communications streams: the hands-free Microphone (the caller) to the PC's own output for calls (default communications, else default, never a hands-free one), and the PC's own mic to the hands-free Speakers. A ringing call's Answer and Decline in Tourne's pop-up go to it (`phoneancs::Perform`), else to the phone over ANCS. For this first test it also looks at the calls every second, to learn whether Windows' events say when a call comes.

Also: Phone Link was started again at 16:17 (its Windows toggle still off; possibly an old Phone Link notification clicked) and closed again. Launching a program through Invoke-Real's `start` leaves it holding Invoke-Real's log open, which blocks the next Invoke-Real: let Tourne Desktop start Tourne Calls.

**Checked:** status from the helper: access allowed, line found ("can dial"), no active calls; run mode started by Tourne Desktop (16:47), registered and connected; texts and notifications back at 16:48. A Clear button on a phone notification worked (16:45, "action 1 done"). Not yet: a real call (ringing, Answer, the sound both ways).

**Calls' devices (ddb52a8):** "Calls: sound out" and "Calls: microphone" (`startMenu.phoneCallOutput`, `phoneCallMic`) are dynamic selects of the PC's own audio devices by name (a phone's hands-free ones left out) plus Windows' default for calls. Tourne Desktop hands the choice to Tourne Calls in `HKCU\Software\Tourne\Calls` (Output, Mic: endpoint ids) and sends it command 9. Tourne Calls uses the chosen device while it's there and working, else the default, and moves a call's sound at once when the choice changes. Checked: the choices list Tourne's two headset outputs (Chat, Game) and two mics; Tourne Calls running and the phone links up after the swap (16:53). Not yet seen in Studio on screen. Cross Device's "Mobile devices" startup isn't needed: Tourne reads only its phone file, already on disk.

**The centre's phone (67c3d8d):** clicking the notification centre's phone opened Phone Link, which took the phone back. With Tourne holding the phone (its Start window carries the property "TourneHoldsPhone"), the phone and Messages open Tourne's phone as a flyout by the pointer, Send a file sends over LocalSend, and the third link is Bluetooth's settings. Phone Link's old notifications only clear on a click. Phone Link closed again (it was started by that click at about 17:00).

**Desktops' previews show the wallpaper:** the hover preview of a desktop showed a flat colour. Its picture loader used WIC on the pager's thread, which had no COM, so the factory failed with CO_E_NOTINITIALIZED (reproduced with the same steps in a test: all succeed with COM, the factory fails without). `LoadCoverPicture` now sets up COM itself and logs `pager:` when a picture can't be read.

**First real calls (Tourne Calls fix):** at 18:38 an incoming call was answered from Tourne's pop-up and ended 1.5 s later without ever talking; a second call, made from the phone, ran 1.5 minutes and ended with its sound on the PC, but the helper never saw it talk there, so the bridge (the chosen mic and output) never ran. Cause: each look at the calls kept a fresh copy of each call and let go of the copy its events were tied to, so no change came. Now the copies with events are kept, the 1 s look also acts when a call's state changed, and Answer moves the sound to the PC only once the call talks (moved while still ringing, the first call dropped). Not yet checked in a call.

**The call's pop-up (Tourne Calls):** while a call talks, is held or is dialing, Tourne Calls shows its own pop-up at the bottom right (over everything, never taking the focus, rounded, light or dark with Windows): who, how long, where the sound is and, on the PC, the mic and output by name, each with a live level, so you can see the caller hears this mic. Buttons: Mute (the bridge sends quiet, and the call's own mute), To the phone / To this PC, Hang up. Ringing calls keep Tourne's notification (Answer, Decline). `TourneCalls.exe preview <folder>` draws it for a made-up call into BMPs. Checked: the preview (both states). Not yet: a real call.

**The call window in Tourne's look (c78cce6):** Tourne Desktop now draws the call itself, as a pane of the Start menu: the style's right pane and frame, its font (and capitals when set), Shut down's buttons, the accent for the levels; Proper 11's round corners when on. Tourne Calls sends the call's state to the Start window (WM_COPYDATA 'TCAL', `CallData` in both) and, while the sound is on the PC, the levels every 50 ms (`TourneCallsLevels`); the buttons go back as its commands; command 10 asks it to say again (sent when Tourne Desktop starts or its theme changes). Tourne Calls' own pop-up is left for when Tourne Desktop isn't running. `tools/phone/call-window-test.cpp` shows the window for a made-up call and captures it. Checked: shown, captured (both states, tools/phone/call-window-preview.png) and hidden on the running Tourne Desktop. Not yet: a real call.

**Desktops' previews, really fixed (b63aaa3):** the COM fix was right but not the whole story: the wallpaper was read (logged `pager: ... read`) and then covered. Windows' touch keyboard host ("Windows Input Experience", a full-screen CoreWindow, cloaked, on no desktop) was taken as a window shown on every desktop, and its teal surface was drawn over the wallpaper in every preview (its keyboard icon showed in a corner). Windows that are cloaked and on no desktop are now left out. `tools/desktop/pager-preview-test.cpp` points at a pager mark, captures the preview and puts the pointer back. Checked: both desktops' previews show their wallpaper (tools/desktop/pager-preview-after.png).

**Handy's tray icon did nothing on a click (suite b55):** Tourne's tray forwards clicks correctly (logged now: `tray: click ... for handy.exe`, id 2, message 0x1772, version 0, the old format Handy registered). But Tauri's tray (tray-icon crate, so Handy) asks `Shell_NotifyIconGetRect` where its icon is before acting on any click, and drops the click when that fails. That call goes to Explorer's Shell_TrayWnd as WM_COPYDATA dwData 3, and with Tourne's tray in front Explorer's is hidden and answers 0 (E_FAIL, for every icon, Tourne's own too). The suite's relay in Explorer (`TraySubclassProc`, suite-redirect.template.cpp) now answers when Explorer can't: sub-message 1 gets left and right, 2 top and bottom (MAKELONG), a 24 px box at the pointer, where the click just was (the protocol as systray-util answers it). Checked: b55 syntax-checks in real compile mode (x64). Not yet: installed (needs administrator: `Install-SuiteBuild.ps1 -Build b55 -Replace b54`), and Handy clicked after.

**Handy, the real fix (ae3fd31, a763ac8):** in shell mode the first Shell_TrayWnd is Tourne Desktop's own (tray-host.cpp), so `Shell_NotifyIconGetRect` asks it, not Explorer, and it answered 0 ("apps fall back to the cursor"; Tauri's tray doesn't, it drops the click). It now answers a 24 px box at the pointer. Measured: shell32 reads sub-message 1 as left and top and 2 as width and height (it adds them), not left/right and top/bottom. The suite relay's answer (b55, installed through UAC) is fixed the same way in the template for the next build; it only matters when Explorer's tray comes first. Checked: the lookup returns S_OK with the box at the pointer, and a right-click posted to Handy's tray window opens its menu (a #32768 window of handy.exe).

**Windows tiling into the taskbar:** dynamic shyness is on (`taskbar.dynamicShyness = 1`), and a shy bar reserves no space by design (work area = whole screen; it slides away while a window touches it), so the tiler lays windows over its edge. Tiling here is the suite's (Windhawk) switcher part: Tourne Desktop logs that it's on there and stays out. Asked the user which way to go.

**Switcher, tiling and borders moved into Tourne Desktop (2026-10-08 22:39):** the user wants everything that can run in Tourne Desktop to run there, looks unchanged. Of the suite parts on in Windhawk, only the switcher is hook-free; the rest (text colour, menus, Explorer style and colours, theme gaps, classic menus, tweaks, Icon Redirect for other programs) stay. Studio's switcher settings differed from the suite's in 29 places (Studio edits that never applied, since Tourne Desktop's switcher stood down while the suite's ran). The suite's visible values went into tourne-gui.conf (backup beside it, `.before-switcher-move-20261008-223837`): borders off, auto-tiling on, focus border none, gaps 3 bottom and 6 inner, switcher colours default; Studio's own additions kept (more float apps, dialogs (#32770) and two titles floating, leader key 93) and its borders settings kept for when they're turned on. Then `switcher.enabled=0` in b55 through windhawk-cli (UAC) and a restart: Tourne Desktop logs "starting Switcher, tiling and borders". This also fixes tiling under the shy bar: TileArea reserves the taskbar from `taskbar.*` settings, which in the suite read its own (off) taskbar and in Tourne Desktop read the real one. Checked: tiled windows end at y 1041 (1080 less the 36 px bar and the 3 px gap). Correction to earlier today: no Tourne border or border shadow was drawing (suite borders off, Tourne Desktop's switcher not running), so the shadows were Windows' own only.

**Alt+Tab peek, Win+Tab, menu edges (4421a79):**
- *Why windows could still be moved, and their outlines showed, while peeking (Alt+Tab and the taskbar's previews):* the peek is Windows' own (DwmpActivateLivePreview, live-peek.cpp). It fades the other windows' pictures but leaves them under the mouse, and it doesn't fade Windows 11's accent-coloured window borders (the outline measured 77,134,113, the AccentColorInactive). Now, while the switcher peeks, an unseen sheet (alpha 1) covers every screen just under the switcher and takes the clicks (a click puts the switcher away). The outlines stay while Windows draws accent borders (Settings, Colours, "Show accent colour on title bars and window borders"): asked the user.
- *Win+Tab opens Mission Control* (`switcher.keys.winTab`, on for the user): the switcher's hook takes Win+Tab when nothing is open, presses the unassigned key so Win let go isn't a lone tap, and opens sticky with the view for this opening only (`g_viewFor` = 4, `ViewKind()`); Esc, a pick or a click closes it. Tourne's Start key hook counts that injected key as Win used. Checked: Win+Tab sent by SendInput opened Mission Control over the screen, Esc closed it, Start didn't open, Task view didn't open.
- *Tourne's menus 1 px thinner at the edge:* menu-style.cpp set Windows' border colour on each menu window as well as drawing its own 1 px frame; Windows' border is now none. Not yet seen on screen.

**Menu frame settings, and why Windows' borders stay for now (6f02e4c):**
- *Menu frame:* StartAllBack's menu frame (1 px line, 6 px band, 1 px line, measured from its Bouquet menu) is now set by three taskbar settings: `menuFrameGap` (default 6), `menuFrameColors` (one colour, or two or three for a gradient from the top left to the bottom right; empty: the style's line) and `menuFrameGlow` (0 to 100, a 2 px soft light each side). `FrameLine` in menu-style.cpp draws every line, glows first then lines so a glow never covers a corner; with neither colours nor glow set it fills as before. The user's: gap 3, `#1e3632 #152026 #121c21`, glow 40. Not yet seen on screen.
- *Tourne's borders in place of Windows' accent borders, tried and put back:* with `switcher.borders` set to match (1 px, offset -1, accent and accentInactive, no glow, shadow or fade, follow native) and DWM ColorPrevalence 0, title bars didn't change and the lines landed on the same pixels, but their colour came out 89,120,109 instead of 77,134,113: with the accent off, Windows 11 still draws a 1 px grey border of its own, which shows through Tourne's line, and which would also stay through a peek. Windows only removes it per window (DWMWA_BORDER_COLOR = none, from inside each program), so the clean way is a suite part (the suite runs in every program) that sets it on each window, then Tourne's borders alone. Put back to the user's look (ColorPrevalence 1, Tourne's borders off; checked: 77,134,113 again). Tourne's border strips are no longer kept out of a peek, so they fade with their windows when used.

**Menu frame slimmer (c655512):** the gap can go down to 1 px: an inner line closer than the menu window's own 3 px border is drawn there with the outer one (StyledMenuWindowProc, the items' ground inside it), and the items fill to their edges (only the line's glow reaches in, when it sits right beside them). The glow is 1 px each side up to strength 50, 2 px beyond. The user's: gap 1, `#305245 #E57474` (the drive bars' gradient), glow 40. Checked on the desktop's menu: line, 1 px gap, line, 1 px glow (7 px before).
### Tourne'Icons: one Windhawk mod to change any icon everywhere, with a picker

Tourne (2026-10-11): changing an icon should be simple: pick it from what's running in the tray, on the taskbar or the desktop, pick an .ico, .svg or other picture, and it's converted to the right sizes and swapped in everywhere that icon shows, once.

New standalone mod `tourne-icons.wh.cpp` (id `tourne-icons`, version 2.0.0), built on Icon Redirect's engine (`tourne-icon-redirect.wh.cpp`, which stays for the suite):

- **The picker.** A window in the mod's own tool process (`windhawk.exe -tool-mod tourne-icons`), opened from its tray icon. On the left: On the taskbar (running windows' programs), In the tray (programs that added a tray icon while the mod ran), On the desktop, In Start (shortcuts, followed to their program or icon), Windows icons (named ones like Folders, This PC, the Recycle Bin, then all of imageres.dll), File types, Your changes. Search box; typing `.ext` adds a file type. "Pick from any file" picks any icon in any file. "Choose new icon" takes .ico, .png, .svg, .jpg, .bmp, .gif, .tif, .webp, or an icon inside a program; "Icon from a program" opens Windows' icon chooser. The new icon is previewed at 16, 24, 32 and 48 before "Apply everywhere". "Put back original" undoes. Store apps are listed and marked as out of reach.
- **Making icons.** Every picture becomes an .ico with 256, 128, 96, 64, 48, 40, 32, 24, 20 and 16 (PNG above 64, DIB at and below). A size the picture has is kept as it is; others are scaled from the nearest larger frame (Fant), or blown up pixel for pixel when the size divides evenly (pixel art stays crisp). SVGs are drawn by Direct2D at each size.
- **Your changes** live in the mod's storage folder (`icons.txt`, tab-separated: kind, original, icon, name, ProgID, previous) with the made icons in `icons\`. No admin. Kinds: `program` (main icon and tray icon), `icon` (`file,index` or `file,-id`, or a loose picture), `file` (whole-file redirects), `filetype` (Windows' own per-user setting, `HKCU\Software\Classes\<ProgID>\DefaultIcon`, with the old value kept to put back), `theme`.
- **Live.** Every program with the mod waits on the storage folder with a thread pool wait (no polling) and takes in a new list as a new rule set; module lookups are cached per rule set generation. Windhawk settings changes rebuild live too, no reload.
- **One icon, not only a program's main one.** Rules can now name an icon group by id or position, so Folders (`imageres.dll,-3`) or This PC (`imageres.dll,-109`) can be swapped for an .ico.
- **Tray icons.** `Shell_NotifyIconW` is hooked: a program you gave a new icon has its tray icon swapped before Windows sees it, and swapped (or put back) straight away when the list changes. Not for Explorer and the hosts (rundll32, svchost, dllhost, sihost, taskhostw). Each program that adds a tray icon is noted once in `HKCU\Software\Tourne\Icons\Tray` for the picker. Setting "Change programs' tray icons too", on by default.
- **Taking in what was set before**, once: Icon Redirect's rows and theme from the newest enabled Tourne'Suite build and from the older standalone mod (read from HKLM, read only), and Tourne Desktop's `taskbar.buttonIcons` rows from `tourne.conf` and `tourne-gui.conf` (app ids skipped). Pictures are made into icons on the way. A balloon says how many.
- The mod links only what its hooks need (user32, shell32, ole32, gdi32, advapi32); the picker's other functions are looked up in the tool process only.

**Checked (under Wine, with a test harness calling the hooks directly):** 15 of 15 engine checks (one icon by id redirected and its images served from the .ico, others left alone, program rows take the main icon, index rows, live rebuild on a list change and no rebuild without one, removal goes back); pictures made into 10-size icons that load at 16, 32, 48, 256, with a 16 px pixel-art picture exactly 3x at 48; a file type applied and put back in HKCU; Tourne Desktop rows and suite rows taken in. The picker window drawn and used. Not checked: SVGs (Wine has no Direct2D SVG), and nothing yet on Windows: the tool process, the tray icon, Explorer and Start refreshing, and whether the mod storage folder is readable from Store apps' sandboxes.

### Tourne'Icons 2.1: Store apps too

Tourne (2026-10-11): Store apps can and should be changed, as b55 and Tourne Desktop already do. b54 and b55 are only on Tourne's PC (`X:\Documents\Claude Workspace\tourne-suite-sab-clone\builds`), not on GitHub. b55 does it by app id in the taskbar's Button icons, and Tourne's own taskbar, Start, Alt+Tab and tray draw the icon themselves; Windows' own places keep the old logo.

Tourne'Icons now goes one level lower, so it covers both:

- **Pictures.** A Store app's logos are the files its `AppxManifest.xml` names (Square44x44Logo, the tiles, the package's Logo), each as several qualified files (`AppList.targetsize-24_altform-unplated.png`, `AppList.scale-200.png`, `contrast-black\AppList.png`), some in the app's resource packages. At Apply the tool finds every one (its own app's, when the package holds several), reads each PNG's size, and makes a picture of that size from the new icon (tiles get it at two thirds, centred), in the storage folder's `packages\<package>-<mark>\` by the same relative path. A new icon goes in a new folder, so programs holding the old pictures don't block it.
- **Opening them.** `NtCreateFile` and `NtOpenFile` are hooked: a read-only open of a `.png` under `\WindowsApps\`, `\SystemApps\` or `\ImmersiveControlPanel\` whose relative path we have a picture for opens ours instead (the original when ours can't be opened). Anything not ending in `.png` passes after a few compares. Rows match the package folder as `name_*_publisher`, so an app update and its resource packages still match.
- **The list.** Kind `package`: original is that pattern, the ProgID field holds the logo stems (`assets\applist;+assets\medtile`, + for a tile), the last field names the pictures' folder.
- **The picker.** On the taskbar finds a Store app's own process behind ApplicationFrameHost and its app id; In Start adds Start's apps folder's Store apps (by app id, to their package with `FindPackagesByPackageFamily` and `GetPackagePathByFullName`); a desktop shortcut to a Store app is followed by its app id. Each shows its own logo.
- **Taking in**, again once (marker `taken-in-2.txt`): Button icons rows marked Use everywhere from the suite build in Windhawk as well as Tourne Desktop, Store app ids included (Claude, Settings, NVIDIA Control Panel). Rows with a command line (Chrome.exe --app=...) and rows not marked Use everywhere are left out.
- Start's host is ended once after a Store app change (it starts again by itself) so it lets go of logos it drew; the storage folder is opened to app packages for reading (S-1-15-2-1 and -2), since Start and Store apps run walled off.

**Checked (Wine, harness calling the hooks with ntdll's own underneath):** 20 of 20: the pattern and family, the right app's logos from a two-app manifest, 8 pictures at their own sizes (310x150 wide tile, a resource package's scale-125, a qualifier folder), `AppListX.png` left out, `NtCreateFile` and `NtOpenFile` opening ours in any letter case, other files untouched, a new version's folder still matching, falling back when ours is gone, put back deleting the pictures. Through the picker: a Store item's logo shown, Apply making a package row and its pictures, listed in Your changes, put back. Engine checks still 15 of 15; take-in skips the command-line and not-everywhere rows. Not checked: anything on Windows (Wine has no Store packages), so whether Explorer, Start and Settings read the logos through these opens without a cache in the way.

### Tourne'Icons Lasting, and "That couldn't be saved"

Tourne (2026-10-11): build a separate version whose changes stick with the mod off.

`tourne-icons-lasting.wh.cpp` (id `tourne-icons-lasting`, 1.0.0) is the same code as `tourne-icons.wh.cpp` built with `TOURNE_ICONS_LASTING 1` and its own header; `tools/icons/make-lasting.py` makes it (run it after changing the main file). It includes only windhawk.exe and hooks nothing: the picker writes what Windows keeps by itself.

- **Programs:** every `.lnk` to the program (Start's two folders, both desktops, Quick Launch with the taskbar's pinned buttons) gets `IconLocation` = the new .ico. Shortcuts made later get it when the tool next starts.
- **Windows icons:** `HKCU\Software\Classes\CLSID\{...}\DefaultIcon` for This PC, the Recycle Bin (`empty`/`full`, plus the default for its current state), Network, Control Panel, the user folder and the six user folders (both their This PC and their own class ids); Folders (imageres 3 and 4) through `HKLM\...\Explorer\Shell Icons` 3 and 4 (admin). Any other icon is shown with a note: only the live version can change it.
- **File types:** as in the live version.
- Store apps, tray icons, windows, Alt+Tab: notes saying the live version does those.
- Everything a change replaced is kept in `lasting.txt` (shortcut path or registry value, and what it was), and Put back original restores it, deleting our own registry keys once empty.

**"That couldn't be saved":** Tourne saw it sometimes. The list is written as `icons.txt.new` and moved over `icons.txt`; a move fails while another program (a virus scanner, the search indexer) holds either file without sharing delete. Each step now retries for up to two seconds on a sharing or access error, and the message now gives Windows' reason and code when it still fails.

**Checked (Wine):** Lasting: 15 of 15 (two shortcuts to a program changed and another program's left alone, a later shortcut taken on the next start, This PC and the Recycle Bin set under HKCU, put back restoring the shortcuts and removing our keys, the list saved while another handle holds it for 400 ms). Live version: engine 15, Store apps 20, picker Store item 6, all passing. Not checked on Windows.

### Tourne'Icons Lasting 1.1: the live version's settings, made lasting

Tourne (2026-10-11): give Lasting a place to paste the same settings as the live version, and make them work there too.

- **Settings:** `make-lasting.py` now copies the live version's whole settings block into the lasting file, rewording the ones that work differently (OVERRIDES in the script), so one YAML block pastes into both. Tray icons, branding and All resource types say they do nothing in Lasting.
- **What the settings do in Lasting:** the icon theme (downloaded first, as live), the theme paths' theme.ini rows and the Redirects rows are read in live's order (later wins). Each run works out what every place should show, from what it showed before the settings:
  - shortcuts (the same folders as program rows): their own icon location, or their program's first icon;
  - every class's `DefaultIcon` in the merged classes view (file types' ProgIDs and every class id), set again under `HKCU\Software\Classes`, with the key's other values copied along since a user key hides the machine one;
  - Windows' stock icons 0 to 32 (folders, drives, the shortcut arrow, the Recycle Bin) through `Shell Icons`, from `SHGetStockIconInfo` (admin; values someone else set are left alone).
- **How a row maps:** a theme DLL or EXE gives the same group id (or the same place when the group has a name, never an id); an .ico or picture stands in for a program's first icon or for a loose icon file; a folder of pictures gives `<id>.png` (or .ico, .svg, .bmp, .jpg) made into an .ico. Imageres-style files under System32 are read from `SystemResources\<name>.mun` when the file itself has no icons.
- **Undo and the picker:** settings rows are kept in `lasting.txt` with kind `setting` and what showed before; places no row wants any more are put back each run, so clearing a setting undoes it. A picker change takes over a place the settings changed (keeping the original before), the settings leave picker places alone, and removing the picker change runs the settings again. Runs happen on their own thread at start, on a settings change and after a put back; one lock covers lasting.txt. Put back now deletes our HKCU keys only when they have no values and no subkeys left.
- **Can't last (said in the readme):** tray icons Windows draws from its own files (SndVolSSO, stobject, bthprops rows), programs' windows, branding, menus and dialogs. Those need the live version.

**Checked (Wine):** new settings test 24 of 24: .ico row on a program's shortcut, theme file row giving the same id, a shortcut whose icon is in a matched file, a file type under HKCU with its other value copied, the stock folder icon in Shell Icons, a second run changing nothing, the picker taking over and handing back, a folder row making an icon, a removed row putting its shortcut back, clearing everything (shortcuts, our key, Shell Icons, lasting.txt), and theme.ini rows from a theme path. Lasting 15 of 15; live engine 15, Store 20, picker Store 6. Not checked on Windows: how long the class scan takes, and whether Explorer honours every HKCU DefaultIcon and Shell Icons value straight away.

### Tourne Drop Install: drop a .wh.cpp, it's installed

Tourne (2026-10-11): a mod to drag a `.wh.cpp` into Windhawk that does Create a new mod, Enable mod off, paste, Compile mod, close the editor.

Tourne's PC runs Windhawk 2.0 (`C:\Program Files\Windhawk` has `windhawk-cli.exe`, `windhawk-core.dll`, the `windhawk-mod*.exe` hosts; `windhawk.ini` has `EnginePath=Engine\2.0.0`, registry `HKLM\SOFTWARE\Windhawk`). Its `windhawk-cli.exe mod install --file <path> [--disabled]` does what the editor's Compile mod does (source stored as `local@<id>`, compiled locally, config and initial settings written; without `--disabled` a reinstall keeps the mod's state). So `tourne-drop-install.wh.cpp` (1.0.0) is a small front for it:

- **Process:** a Windhawk 2 tool mod: `@include windhawk-mod-elevated.exe` and `windhawk-mod.exe`, so the engine starts a host for it (elevated when it can). The mod stops the host's entry point and runs its window thread there; it loads nowhere else.
- **Window:** a drop zone, Choose files, and a log. Drops from Explorer reach the elevated window through `ChangeWindowMessageFilterEx` (WM_DROPFILES, WM_COPYDATA, 0x49). A folder drops its `.wh.cpp` files. One install at a time on a worker thread.
- **Each file:** reads `// @id`, asks `mod show local@<id>` whether it's installed, then `mod install --file` with `--disabled` only for a new mod (setting Turn new mods on, default off). The log shows the tool's output, compiler errors included. Not elevated: the tool runs through the administrator prompt via `cmd /c ... > temp 2>&1`.
- Settings: tray icon (on; off opens the window at start), Turn new mods on (off), Keep on top (off).

**Checked (Wine, with a stand-in windhawk-cli.exe recording its arguments):** 12 of 12: @id parsing, window, a new mod installed with `--disabled`, an installed one without it, a folder drop, compile errors shown, a .cpp with no @id refused. Not checked on Windows: the real CLI, the host launch, drops from Explorer into the elevated window.

### Plugin panels: word wrap, resizing, moving, a pin to stay open

Tourne (2026-10-11), about the Notes plugin: word wrap and line breaks, resizable, and a toggle to stay open after a click elsewhere. All of it is in the panel runtime (`desktop/host/plugin-panel.cpp`), so every plugin panel gets it.

- **Word wrap:** multiline inputs no longer have `ES_AUTOHSCROLL`, so lines wrap at the input's width and re-wrap when the panel is resized. Enter already made a new line (Ctrl+Enter submits, as before); the test checks both.
- **Resizing and moving:** the panel's edges and corners resize it (hit-tested 6 px borders, min 220 by 160), and a 24 px strip along the top moves it. The content relays out on every size change.
- **Pin:** a pin at the strip's right. Pinned, a click elsewhere leaves the panel open (Esc and its bar item still close it). Right-click the pin to forget the kept size and place.
- **Kept per panel** in Tourne Desktop's values (`HKCU\Software\Tourne\Desktop\Values\plugins`, named `<plugin>:<panel>.panelWidth` and so on): the size (96-dpi pixels), the place once it's been moved (clamped onto its screen), and the pin. The manifest's size and placement are the default.

**Checked (Wine, a test program around plugin-panel.cpp):** 29 of 29: no ES_AUTOHSCROLL and a long line wrapping to 3 lines, real Enter key making a new line without submitting, edge/corner/strip/pin hit tests, the editor following a resize, size and place kept and reopened the same, pinned staying open while unpinned closes, right-click forgetting, a panel with no name. Not checked on Windows.

### Mod Manager Tray 1.1: the list scrolls with the wheel

Tourne (2026-10-11) runs Salyts' Mod Manager Tray (MIT, `local@mod-manager-tray`) and asked for its list to scroll: with about 60 mods the menu is taller than the screen and only its hover arrows moved it. 1.1 (`tools/windhawk/mod-manager-tray.wh.cpp`, from the copy in Tourne's Downloads) adds a `WH_MSGFILTER` hook while the menu is open: each wheel notch (touchpad deltas add up) becomes Up/Down presses that select the new bottom or top item, so the menu scrolls 3 mods a notch, never wrapping. Nothing else changed.

**Checked (Wine, a 101-item menu):** a real wheel notch down scrolls 3, three notches 9, up 6, far down stops at the end, far up at the top, half notches add up, Esc still closes it. Not checked on Windows.

**Later:** Tourne runs it as their own `tourne-mod-tray` (Tourne Mod Tray 1.0.0, `tools/windhawk/tourne-mod-tray.wh.cpp`), and the wheel didn't scroll on the PC. Likely cause: the tray click reaches the mod through the taskbar, so its thread isn't in front and the wheel messages never reach the menu's loop. 1.0.1 adds a `WH_MOUSE_LL` hook while the menu is open that takes wheel turns over the menu window wherever they'd go (the message filter hook stays as a fallback), logs what it did (`wheel: ...`), and credits Salyts in the readme. Wine: the same 9 checks pass with the low-level hook alone and another window in front.

### Tourne Notes: names kept, pictures, preview, search, trash (a plugin built into Tourne Desktop)

Tourne (2026-10-11): Notes reset note names to the date, needs picture pasting, and a better feature list (Tourne picked all 11 proposed). The name bug: Noctalia's Notes renames only on Enter in the name box (`onSubmit`), so a name typed and then left (Back, clicking the note, closing the panel) was dropped.

Noctalia's Notes is fetched from GitHub, so a fix there would be replaced on update. **Tourne Notes** (`tourne/notes` 2.0.0, MIT, from Noctalia's) is the first of Tourne's own plugins built into the program: `desktop/plugins/bundled/` (its own `index.json` and the plugin folders) is embedded by the build (`plugins-bundled.inc`), shows first in the Studio's Extensions, installs from the program with no download, and a newer build brings an installed copy up to its version by itself (`PluginRefreshBundled`, keeping what was agreed to). It keeps notes the same way (plain files in `~/Documents/Notes`), so Tourne's notes carry over.

- **Names:** what's typed in the name box is saved on leaving the note (Back, another note, a new one, closing the panel) as well as on Enter; `\ / : * ? " < > |` become `-`, no trailing dots or device names. A new note takes its first line as its name until it's named by hand (`.tourne-notes.json` marks those).
- **Pictures and files:** Ctrl+V with a picture and no text copied saves a PNG in `images\` and links it at the caret on a line of its own. Files copied in Explorer and pasted, or dropped on the panel, are copied into `images\` or `files\` and linked; note files dropped on the list become notes.
- **Preview** (eye button): headings, lists, quotes, code, rules, pictures (click to open), links as buttons, and `- [ ]` lines that tick off when clicked.
- **Search** over names and text (the matching line under each name), a **trash** (`.trash`, restore or delete for good, empty), **open in Notepad** and **open the notes folder**, settings for the **editor font** (pixel, Consolas, Segoe UI) and **size**, and a **hotkey** (none by default).

The runtime gained what that needs, for any plugin: `noctalia.saveClipboardImage(path)` (PNG, DIB(V5) or bitmap on the clipboard, needs clipboardRead), `noctalia.copyFile(from, to)` and `noctalia.openPath(path [, "notepad"])` (needs open; files the user dropped or pasted on the panel count as the plugin's), `panel.insertText(key, text [, ownLine])`, `onPasteImage` / `onPasteFiles` on inputs, `onDrop` on any node, image nodes that draw their `path` (WIC, scaled down to fit, `maxWidth`), `fontFamily` (mono, smooth) on labels and inputs, a multiline input's caret starting at the end, and a `hotkey` setting on any panel (`Ctrl+Alt+N`, `Win+Shift+N`, `F9`; RegisterHotKey on the runtime's window). The runtime thread now has COM. `tools/plugins/pluginrun.cpp` got the three panel-value functions it was missing since the panel pin (it didn't link).

**Checked (Wine, a test program driving the real runtime and panel):** install from the program, the hotkey registering and reopening the panel, typing, a pasted picture saved and linked, preview with picture, heading and checklist, ticking a box saved to the file, the first-line auto name on Back, a typed name (`Groceries: week 42?` to `Groceries- week 42-`) saved on closing the panel, Enter renaming, search with the matched line, delete to trash, restore, a .txt dropped on the list becoming a note, a picture dropped and a file pasted into a note on lines of their own, a 1.9 install brought up to 2.0.0. The whole program builds (on Linux with pwsh and MinGW, `#embed` stood in for). Not checked on Windows; fonts other than the fallback weren't visible in Wine.

## Phone pane: right-click Clear, themed notification icons, no Phone Link item (2026-10-11, cloud)

Tourne asked for three things in the Start menu's phone pane: clear a notification by right-clicking it, as in the notification centre; show its icons in their own style; and drop Phone Link from the right-click menu.

- **Clear.** A right-click on a notification row now offers:
  - Clear;
  - "Clear all from <app>", when that app has more than one;
  - Clear all.

  This works the way the notification centre's x and Clear all do: through Windows' toast history (`phoneancs::Clear`/`ClearApp`/`HideAll`; Tourne Desktop only, the suite build has stubs). `PhoneNote` now carries the toast's AUMID, tag and group (phone-link.cpp). The row and its "N new" go at once. The pane isn't re-read right away, since wpndatabase.db can lag behind.
- **Icons.** `startMenu.phoneAppIcons` now defaults to off: notifications show the tray design for their app, in the theme's colours.
  - New designs in system-icons-host.cpp: mail, reddit, video, shop. They join the existing chat, capture, spotify and audio designs, and the phone design is the fallback.
  - The notification centre's `DesignForApp` gives phone apps (Tourne.Phone.*) the same designs. `PhoneAppDesign` in start-menu.cpp keeps the same list.
  - On: the apps' full-colour logos, as before.
- **Phone Link.** "Open Phone Link" is left out of the right-click menu while Tourne holds the phone (`PhoneTexting()`).

A review of the phone code found 15 more issues: /mnt/project-files/tourne-desktop/phone-pane-review-2026-10-11.md in the project. None are fixed yet, waiting on Tourne's pick.

**Checked:** Tourne Desktop builds with the change (MinGW, cloud; all four modules). The four new 12x12 grids were checked by printing them. Not checked anywhere: the menu and the clearing at run time, because they need the phone linked over Bluetooth, which Wine can't do.

## Phone pane: the 15 review findings fixed (2026-10-11, cloud)

Tourne asked for all 15 from the review (/mnt/project-files/tourne-desktop/phone-pane-review-2026-10-11.md) to be fixed.

1. **The pane coming back.**
   - `PhoneLinked()` looks at the link record again after 30 s; it no longer caches the answer forever.
   - Start reads the phone whenever its pane is on, even if no phone was linked last time.
2. **Phone Link unlinked or gone.**
   - `ReadPhoneIdentity` keeps the linked phone's name, system and model in HKCU\Software\Tourne\StartMenu (PhoneName, PhoneOs, PhoneModel).
   - It falls back to them while Windows still has a Bluetooth device by that name.
   - The pane, texts (phone-map) and notifications (phone-ancs) all use it.
3. **Tourne's own count.**
   - `ReadPhoneNotes(own)`: while Tourne holds the phone, only Tourne.Phone.* toasts are read, and Phone Link's badge is ignored.
   - Both the pane (`PhoneReadAgain`) and the notification centre's phone header (`TourneHoldsPhone()`) pass it.
4. **Phone Link opened later.** Each wake of ANCS's connected loop checks for Phone Link. If it's running, Tourne's copies are hidden and Tourne waits, as before connecting.
5. **Stale dot.** `PhoneSeen()` marks the open conversation read when the phone comes into sight: Start, its own window, or the tray flyout.
6. **Rows the phone cleared.**
   - phone-ancs posts `WM_APP_PHONE_NOTES` to the Start window when notifications come or go.
   - While the phone is in sight, it is read again 1.5 s later, once wpndatabase.db has the change.
   - This is a message, not a timer, so there is no polling.
7. **Connected.** `PhoneBluetoothDevice` looks at every node with the phone's name. It takes the battery from whichever has one, and counts the phone as connected if any node is.
8. **Texts with no number or email** are kept under the sender's name ("Unknown sender" without one), so their conversation opens.
9. **Call ids.** A removed notification leaves `g.calls`. Calls and queued actions are dropped when the connection ends.
10. **Second send.** A send while one is running now says "Already sending to your phone".
11. **Over 4 GB.** Such files are left out, with a notification naming them.
12. **Logo cache.** Phone app logos sit in `g_phoneAppIcons`, freed with the rest (`ForgetPhoneAppIcons`). A missing one is asked for again after a minute.
13. **Texts in memory.** They are trimmed to the 500 the file keeps, once there are 100 over. The outbox and sentAt places move with them. There is no trim while a text is being handed to the phone.
14. **Reader thread.** The last read's thread handle is kept, and `PhoneReadWait()` waits for it in `DestroyStartMenu`.
15. **SQL and lock.**
    - The logo lookup binds the package (`?1`, `phonedb::Query`'s new `bind`).
    - `PhoneNow()` copies `g_phoneRead` under the lock into `PhoneInfo::read`, which the paint uses.

**Checked:** Tourne Desktop builds with all modules (MinGW, cloud). Nothing was run: all of these need the phone over Bluetooth.

## Phone notifications open their app on the PC (2026-10-11, cloud)

Tourne clicked a Discord notification in the phone pane and expected to reply there. The click opened the notification centre instead.

Replying isn't possible this way. ANCS, Apple's notification service, lets a PC only clear a notification or press its positive or negative action. Replies work only for texts, which go over MAP.

So `PhoneAppOnPc(package, name)` (phone-link.cpp) picks where to answer one on the PC:
- The app's own link when its scheme is registered: Discord, Slack, WhatsApp, Telegram, Spotify, Teams, Outlook, Steam.
- Otherwise its website: Gmail, Reddit, Instagram, Messenger, X, GitHub, YouTube, Twitch.
- Anything else falls back to the centre.

The pane's notification rows use it on click (`PhoneRun`, while Tourne holds the phone), and so does a click on a phone notification's body in the notification centre (`ActivateNotification`).

Seen in the same video: the pixel font draws boxes in place of emoji in notifications. Asked Tourne whether to strip them.

**Checked:** builds (MinGW, cloud). Not run.

## Phone pane: an open conversation scrolls (2026-10-11, cloud)

The open conversation drew only its newest texts, and the wheel did nothing over it.

The fix:
- `PaintPhoneThread` now measures every text in the conversation and draws only the ones in sight.
- It is scrolled by `g_phoneThreadScroll` (pixels up from the newest text). How far it can go is kept in `g_phoneThreadMax`.
- `PhoneThreadWheel` turns the wheel over the conversation into scrolling: from the Start window (`WM_MOUSEWHEEL`, after cascades), and from the phone's own window.
- Opening a conversation starts at its newest text.

**Checked:** builds (MinGW, cloud). Not run.

## Emoji as plain symbols in notifications and texts (2026-10-11, cloud)

The pixel fonts draw boxes in place of emoji. Tourne chose "swapped" (a plain symbol) over "stripped".

`PlainSymbols` (phone-link.cpp) does the swap:
- **Faces and hearts** become text: ":)", "xD", "<3", "(y)", "(fire)" and so on.
- **Fancy letters** become plain ones: maths bold, italic and script letters, and fullwidth.
- **Arrows and quotes** become "->", "'" and '"'.
- **Joiners, colour marks and skin tones** are dropped, along with flags.
- **Any other emoji** becomes "*".

It is applied to:
- every notification's text and button labels in the notification centre (`ParseToast`);
- the phone pane's notifications (`ReadPhoneNotes`);
- texts: conversation names, the last line and the bubbles.

Saved texts keep their emoji.

**Checked:** `PlainSymbols` run under Wine on 8 samples (faces, hearts with FE0F, bold and fullwidth letters, a skin tone, a ZWJ family, a flag, accents, curly quotes); all came out as expected. Tourne Desktop builds.

## Tourne'Icons 2.2 / Lasting 1.2: Save lists and Apply a pack (2026-10-11, cloud)

Two buttons under the picker's lists, for changing many icons at once:
- **Save lists** gathers every list (taskbar, tray, desktop, Start, Windows icons, file types, your changes) and writes a new folder in Downloads, `Tourne'Icons lists <date time>`: a 64 px PNG of each entry's icon as it shows now (one subfolder per list) and `pack.tsv`. The folder opens when done.
- **Apply a pack…** opens a `pack.tsv`, counts the rows whose picture exists, asks once, then applies them all the way Apply everywhere does. Store app rows make their logo pictures as usual; Start restarts once at the end, not per app (`SaveChange` takes `restartStart`).

`pack.tsv` (UTF-16 or UTF-8): `picture  kind  original  name  progid  list  shows-from  detail`, one row per entry; `#` lines are notes. The picture is a path from the pack's folder or a full path. Rows with no picture, or a kind that can't be changed (theme, file), are skipped.

Icons are turned into pixels by drawing them on black and on white (`IconPixels`), so old mask-only icons keep their see-through parts.

**Checked:** both builds compile (MinGW). Under Wine, Save lists wrote 24 rows and the PNGs that Wine could draw, with correct see-through edges; Apply a pack on a test pack applied a program, a Windows icon and a file type (from a PNG and an .ico), skipped the missing picture, the theme row and the row with no picture, and said "Applied 3 of 3." The picker's layout with the two buttons was checked in a screenshot.

## Tourne'Icons Kit 1.0 (and Tourne'Icons 2.3 / Lasting 1.3) (2026-10-11, cloud)

Tourne asked for something you open, run once and close, with nothing at startup, and changes that stay (open it again to put back or change more). **Tourne'Icons Kit** (`tools/icons/kit/`) is the lasting picker as a program of its own, no Windhawk:
- `kit.cpp` builds `tourne-icons.wh.cpp` with `TOURNE_ICONS_KIT 1` (which needs `TOURNE_ICONS_LASTING 1`) and stands in for Windhawk's functions (`windhawk_utils.h` there): storage in `%LOCALAPPDATA%\Tourne\Icons Kit`, no settings, no hooks, theme downloads through `URLDownloadToFileW`.
- It opens the picker at once; a thread runs `lasting::ApplyAll` (icons back on shortcuts made or remade since) and the picker says how many it put back. Closing the window ends the program (`PostQuitMessage` in the picker's `WM_DESTROY` when `kKit`), after any write in progress finishes. One copy at a time.
- Manifest: highestAvailable (stock icons are for every account), common controls 6, PerMonitorV2. Icon `kit.ico` from the pack-maker. Build: `tools/icons/kit/build.sh`.

In the shared code (so Lasting 1.3 gets them too):
- **Shortcuts are matched like Tourne's rules match:** by target, by the program an updater starts (Squirrel's `Update.exe --processStart Discord.exe` stands for `Discord.exe` beside it), and by the icon file (`*\discord\app.ico`). On Tourne's PC, Discord's Start shortcut targets Update.exe and takes its icon from `%LOCALAPPDATA%\Discord\app.ico`; the shortcut was last written in April (updates leave it alone), while app.ico was rewritten by the 8 October update. So the icon written into the shortcut survives updates, and swapping app.ico in place would not.
- **One pass over shortcuts:** `ApplyPrograms` opens each shortcut once for all program rows (newest matching row wins). A pack's shortcuts are done in one pass at the end (`g_holdLasting`).
- **Drives, discs and Windows' other stock icons:** `StockNumbers` reads which imageres id each Shell Icons number shows (0-17, 22, 23, 28, 29), and `DrivesShowing` which id each fixed drive shows (the Windows drive has its own), both before any change; an icon row sets Shell Icons `<n>` and `DriveIcons\<letter>\DefaultIcon` (HKLM). Put back removes the drive keys again when empty. A row changed in an earlier run still counts as lasting through lasting.txt.
- **Apply a pack** skips rows that can't last in the lasting builds (Store apps, Windows icons with no setting) and says how many.
- Texts say "after you close the Kit" / "the Kit must be opened as admin" in the Kit.

**Checked:** both mods and the Kit build (MinGW). Under Wine with a fake Discord (Update.exe + app.ico shortcut): a pack row `*\discord.exe` changed the shortcut; reopening the Kit put back nothing; after the shortcut was remade with app.ico, reopening said "Put your icon back on 1 shortcut"; Put back original restored `%LOCALAPPDATA%\Discord\app.ico,0` and removed the folder Shell Icons values. Tourne's full pack applied 534 of 534 lasting rows in 12 s (415 skipped as can't-last under Wine, where no stock icon comes from imageres). The window opens and closing it ends the program (exit 0). Not yet run on real Windows: stock and drive icons can't be checked in Wine.

## Tourne'Icons 2.4 / Lasting 1.4 / Kit 1.1: packs keep your icons, stubborn redirects (2026-10-11, cloud)

Tourne applied the generated pack and it replaced icons they had set themselves (their b55 Icon Redirect rows and their own Tourne'Icons rows). They want packs to change only Windows' default icons, and the Kit's lasting work brought into the Lasting mod with settings for stubborn redirects.

**Packs keep your icons** (`picker::ApplyPack`, `ApplyPackRows`, `FindYours`):
- Before applying, the picker finds what's yours: the list rows a pack didn't make (pack-made rows are listed in `pack-rows.txt` in storage), the live mod's list when this is Lasting, the suite's and older mods' Icon Redirect rows, and Tourne Desktop's and the suite's Button icons (`ReadEarlier`, shared with taking in earlier changes).
- An entry counts as yours on the same kind and original, a shows-from file in Tourne'Icons or Kit storage, a shows-from file equal to a row's icon, a program pattern matching the shows-from file or the original, or a whole-file stand-in (ink-flat style) that holds the entry's icon id.
- It asks: Yes keeps yours and changes the rest, No replaces yours too. The status says how many it kept.
- New pack rows: a picture of `-` puts that entry back; a row in the list "Your changes" is always applied and stays yours. Rows whose entry already shows that picture are only marked.

**Pack maker** (`/mnt/project-files/tourne-icons/pack-maker/build.py`, not in the repo): skips the "Your changes" list and anything already showing an icon from Tourne'Icons or Kit storage, and writes a shows-from column.

**Put-back pack for Tourne**: 73 of Tourne's icons the pack replaced (their old pictures were still in storage) come back as "Your changes" rows, 31 pack rows that covered Tourne's redirects are `-`, and 833 rows only mark what the pack made. Needs 2.4.

**Stubborn redirects (Lasting only)**: setting `stubbornRedirects` (target: a program, shortcut or icon file pattern, like `*\discord.exe` or `*\discord.lnk`; icon: picture). Matching shortcuts get the icon, and while the mod runs a watcher (`FindFirstChangeNotificationW` on the shortcut folders, 3 s settle, no polling) puts it back when an app remakes a shortcut. Rows removed from the setting are put back. `watchShortcuts` (default off) watches for every picker change too. The Kit has no settings, so it has neither; it puts icons back each time it opens.

**Checked:** both mods and the Kit build (MinGW). Under Wine: keeping yours kept 4 of 5 rows in a test pack (same key, program pattern, file type by shows-from, list icon) and applied the fifth; a `-` row put back regedit; a pack-made row was replaced by the next pack. Lasting: a stubborn `*\discord.lnk` row set the icon, a remade shortcut got it back within 6 s, removing the row put back app.ico, stopping the mod stopped the watcher. Not yet run on real Windows.

## Tourne'Icons 2.4.1 / Lasting 1.4.1: builds for 32-bit programs (2026-10-11, cloud)

Windhawk compiles mods for 32-bit programs too, and 2.4.0 failed there: the keep-yours check passed a lambda to `EnumResourceNamesW`, which a 32-bit build can't turn into a stdcall callback. It now uses `CollectGroups`.

**Checked:** clang with `--target=i686-w64-mingw32` reproduced Tourne's error on 2.4.0 and builds 2.4.1 and Lasting 1.4.1 clean; the 64-bit builds and the Kit build too.

## Tourne'Icons 2.4.2 / Lasting 1.4.2: tray icons of programs with several (2026-10-11, cloud)

After the packs, every tray icon Tourne Desktop shows of its own (system icons, the overflow's layout buttons, temperatures) became the T. Cause: the pack's `*\tournedesktop.exe` program row, and the live mod's rule that a program's new icon stands in for its tray icon (`RuleSet::trayIcon`), which swapped every tray icon the program put up.

Now the swap happens only while a program has one tray icon (`SwapsTrayIcons`); when a second one appears, the first gets its own icon back. TourneDesktop.exe also counts as a host (like Explorer), so its tray icons are never swapped. Turning the mod off or updating it puts the tray icons it changed back (`Wh_ModUninit`).

**Checked:** both mods build for 64-bit and 32-bit (clang i686) and the Kit builds. Under Wine with a stand-in for Shell_NotifyIcon: one icon was swapped; adding a second sent both with their own icons; unloading sent both back.

## Tourne'Icons 2.5 / Lasting 1.5 / Kit 1.2: live changes, file types of their own, Clear icon cache, an ink pack (2026-10-12, cloud)

Tourne found some Windows icons broken against their Bouquet theme, wants changes live (no Explorer restart, no sign out; admin is fine), Tourne Desktop's taskbar buttons to follow like the running Tourne Desktop does, their "Clear Icon Cache.ps1" inside Tourne'Icons, and every pack icon redrawn in the ink colours, each one unique.

**File types with no program** all share Windows' "Unknown" type, so one change showed on every one of them (the XVS icon on all). Each such file type now gets its own type, `HKCU\Software\Classes\TourneIcons.<ext>` with its icon and a name, and `.<ext>` points at it. The row remembers what `.<ext>` held before (`+-` none, `+=value`); putting back restores that and removes the type. On start, rows made the old way are moved over and our icon is taken off Unknown (`SeparateUnknownFileTypes`; the Kit does it when it opens).

**Live:** every change ends with `SHChangeNotify`, a nudge to Explorer's icon size (Shell Icon Size one smaller, broadcast, back, broadcast) so it redraws icons it already drew, and a message to Tourne Desktop's bars (`WM_APP + 4` to `TourneTaskbarTest_*` windows, the message the running build already reloads its button icons on). Tray changes reach Tourne Desktop's bar the same way.

**Clear icon cache** (picker button, under Apply a pack): asks, ends Explorer, deletes `iconcache_*.db` and `thumbcache_*.db` in `%LOCALAPPDATA%\Microsoft\Windows\Explorer` (retries while files are held), and starts Explorer again if Windows doesn't. This is what Tourne's script did.

**Video thumbnails** wear Windows icon 23 (imageres) as their frame; the pack had drawn a film there. The new pack puts it back.

**Pack maker** (`/mnt/project-files/tourne-icons/pack-maker`, not in the repo): everything in the ink colours (#66ea97 line, #3c865c shade, #ff6f77 red). Every entry gets its own drawing: entries sharing a picture get a small red letter (an app's own letters) or number in the corner, and Windows' page icons a number in the label (DOC, DO2, DO3) while file types keep their own. Steam is drawn after Tourne's STEAM.ico, ShareX is new. Tourne's tray-*.ico icons are no longer used as stand-ins, and programs whose tray icons Tourne set up in Tourne Desktop are left alone (all but ShareX and Explorer). 850 rows, 844 drawings.

**Checked:** both mods build for 64-bit and 32-bit (clang i686, MinGW `-Wall` clean) and the Kit builds. Under Wine: two Unknown rows moved to `TourneIcons.aaa` / `.bbb` with their names and `+-`, Unknown left clean, a second run moved nothing, put back removed both. Explorer's redraw, Tourne Desktop's bar refresh and Clear icon cache need real Windows: not yet run there.

**Later the same night: look-alikes drawn for their use.** Tourne turned down the letter and number badges ("lazy") and asked that look-alike icons differ by what calls them. Windows' own imageres icons were read from Tourne's PC (`C:\Windows\SystemResources\imageres.dll.mun`, read only), and each Windows icon now has a drawing for what Windows shows there (`pack-maker/variants.py`, with the corner details in `details.py`). Discs carry Windows' own labels (DVD-R, CD-RW, BD), and audio and video discs get a note or film. BitLocker drives get a lock or an open lock, plus a warning or the Windows mark; BitLocker To Go uses the USB stick. Printers show default, network, to file and fax. Command Prompt and PowerShell have an administrator version. Libraries sit on a shelf. Dark and light pairs are outlined and filled. Office pages are dashed for templates and get a "!" for macros. Apps that shared a picture get a detail of their own, such as Python's snake, IDLE's pencil, Riot Client's play mark and the EA logo. Where a Start shortcut and a Windows icon are the same thing, they share the picture (Command Prompt, Defragment, File Explorer). Some groups are drawn alike by Windows itself, and those get details by best guess: pictures 70, 71, 72 and 83; folders 115, 185 and 1020; fonts 124, 128, 158, 159 and 160. Windows icon 14 is blank and is left alone. The build stops if two entries would still look the same. The rebuilt pack has 849 rows and 830 drawings.

## Tourne'Icons 2.5.1 / Lasting 1.5.1 / Kit 1.2.1: Copilot's logo, folder pictures in the open dialog (2026-10-12, cloud)

**Copilot** gave "This app's logo pictures couldn't be found or made." Copilot is a "sparse" package: its WindowsApps folder holds only the manifest (checked on Tourne's PC: AppxManifest.xml, the block map, the signature and two json files), and its files, logos included, are in its outside folder. Tourne'Icons now asks Windows for that folder (`GetPackagePathByFullName2`, effective, then machine, then user external) and looks for logos there too (`store::OutsidePathOf`, used by `FoldersOf`). The package rule gets a second rule for the outside folder, so Windows' reads of the logos there are redirected as well (`AddPackageRule`, `store::OutsideFoldersOf`). Every other app gets an empty outside folder and works as before.

**Folder pictures in the open dialog** broke with the ink pack. Windows draws a folder that shows its contents from two icons (imageres 5, the back with the tab, and 6, the front flap) with the files' pictures between them, so both have to line up. The pack had drawn them as two unrelated icons (a folder with a plus, an open folder with dots). They are now one folder split in two on the same grid: the back is the outline with the tab, and the front is the filled lower part, with Windows' step on its top edge (`pack-maker/variants.py`, `FOLDER_BACK` / `FOLDER_FRONT`; a `,` in a corner keeps each layer in place since it isn't drawn). 849 rows, 830 drawings.

**Checked:** both mods build for 64-bit and 32-bit (clang i686, MinGW `-Wall` clean) and the Kit builds. The two folder layers were stacked at 256 px with a page between them and line up. The Copilot change needs real Windows: not yet run there.

### Tray folders: one icon at a time, folders made on purpose, drops into folders shown at once (2026-10-12, cloud)

Tourne found the hidden icons flyout's folders odd: moving one icon into a folder, or stacking two, moved more icons than that, and some changes showed only after closing and opening the flyout again.

- **More than one moved:** folders kept what was dragged into or out of them by program, so every icon of that program went along (all of Tourne Desktop's own tray icons are one program, as are Explorer's). They now keep the icon itself (`program|id`, the key the tray's places use). What was kept the old way still works for the icons not moved since.
- **Folders made by accident:** passing over the middle of an icon while moving another made a folder of the two, and the icon it picked could be the wrong one while the others were making room. Now an icon has to be held over another for about half a second (`kTimerStackWait`, one timer per hold, none while idle), the icon picked is the one shown there, and the others stay put meanwhile.
- **Shown late:** an icon dragged from the taskbar's tray onto a folder tile, or into an open folder, only went into the overflow; it now goes into that folder, shown at once.

The Windhawk suite build reads the same folders but not the new per-icon entries; with Tourne Desktop as the shell it isn't the one showing them.

**Checked:** the System icons module compiles (MinGW g++). Not yet tried on the PC.

**Later: undo for "Folders as in the settings", Customize opens Tourne Studio.** Tourne lost their games/audio folder to "Folders as in the settings", which deleted the folders made in the flyout with no way back. It now keeps them (`TrayFoldersBefore` beside `TrayFolders`), and the same menu then offers "Folders as I had them". The flyout's "Customize..." link opened StartAllBack's settings when StartAllBack is installed; in Tourne Desktop it now opens Tourne Studio searched to the tray folders (the Windhawk suite keeps the old behaviour). Tourne's settings still hold `{.GAME | .SOCIAL | .AUDIO}`, but its list is a copy of the TOOL folder's, so only Docker and Discord can land in it, and it hides while neither runs. The folders made by hand were deleted before this change and couldn't be brought back.

### Start crashed after a screen change (2026-10-12, cloud)

Tourne's c2a6816 crashed opening Start: an access violation in UxTheme under `StartBackground` < `StartShape` < `ShowStart` < `ToggleStartMenu` (crash.txt, named with the build's symbols copy). Start's look can use the bar's own visual style file. When the screens change (here, Explorer underneath restarting), `ApplySettings(wnd, screensOnly)` loads that file again but skipped Start's before/after steps, so Start kept theme handles into the closed file, and the next Start opened read freed memory. A screens-only apply now lets Start's look go before the file is replaced and loads it again after.

**Checked:** the taskbar module compiles (MinGW g++). Not yet run on the PC.

### Tourne'Icons 2.5.2 / Lasting 1.5.2 / Kit 1.2.2: no Store lookups as programs start (2026-10-12, cloud)

After 2.5.1, Windhawk's own window failed to start ("The WebView2 webview it hosts could not be created", timeout). 2.5.1 asked Windows for every Store row's outside folder (`FindPackagesByPackageFamily`, `GetPackagePathByFullName2`) while loading its rules, which happens in every program as it starts, msedgewebview2.exe included, and Tourne has many Store rows. Most likely that held WebView2 up. The outside folders are now written down once, when a row's pictures are made (`.outside` in the row's pictures folder, skipped as a picture), and the rules only read that file. Tourne Mod Tray isn't involved: it runs in its own windhawk.exe process, and the list length doesn't reach Windhawk's window.

**Checked:** both mods build for 64-bit and 32-bit (clang i686, MinGW `-Wall` clean) and the Kit builds. Not yet run on Windows.

**Later: "Reopen Programs at Sign-in" checked.** Read from Tourne's `C:\ProgramData\Windhawk\ModsSource` (2.0.0, by lima26x): it runs as a hidden `explorer.exe -tool-mod` copy (its `@include` is explorer.exe), notes open program windows every 20 s, reopens them once per sign-in within 10 minutes of signing in, and skips programs running as admin (Windhawk among them). Not the cause. Its hidden explorer.exe loads every Explorer mod too, which costs memory only.

### Tourne'Icons 2.5.3 / Lasting 1.5.3 / Kit 1.2.3: more than 128 changed icons per program (2026-10-12, cloud)

After applying the ink pack, about half of the Windows icons (imageres 125 and up, and most of the 53xx group) still showed Windows' own pictures in the picker and elsewhere, although Tourne'Icons listed them as changed. Each replacement a program loads is recorded so `LoadResource` can tell whose resource it was given, and that record held 128 entries; the 129th replacement onward was skipped. The pack sets several hundred. The record now grows in blocks of 256 as needed (up to 65,536), still append-only and read without a lock.

The ink pack also left 12 imageres icons as Windows draws them; they're drawn now (pack-maker `variants.py`): the shortcut arrow and shared overlays, download, in-sync and speaker badges (163, 164, 169, 157, 176, drawn in the corner Windows lays them on), the cloud warning and syncing icons (1041, 1042), and the failed task, restore point, media server and signed file icons (5318, 5319, 5368, 5378, 5383). Still left as they are: blanks and shadows (17, 119, 121, 191, 192, 1015, 1017, 1018, 1301 to 1303, 5365, 5369), the video thumbnail frame (23), and the ones already showing Tourne's own pictures (81, 84, 86 to 88, 97, 98, 105 to 107, 197).

**Checked:** both mods build for 64-bit and 32-bit (clang i686, MinGW `-Wall` clean), the Kit builds, and the new range record was run with 3,000 entries from 4 threads under ThreadSanitizer (all found, no races). The full pack builds with every drawing unique (861 rows). Not yet run on Windows.

### Discord: Tourne Big Files (Vencord plugin) and the server plan (2026-10-12, cloud)

Tourne asked for a Vencord/Testcord plugin that sends big files without Nitro by cutting them into pieces, and for a server plan another thread made to be fitted to our projects (their showcase thread in the Windhawk server was the starting point).

- **`tools/vencord/tourneBigFiles.desktop`**: right-click the upload (+) button, Send a big file (in pieces). Pieces under 10 MB go as silent messages (in a thread in servers), then a card with `name.tbf.json` (SHA-256 per piece and whole). With the plugin the card plays videos, music and pictures in chat or saves the file whole, piece by piece to disk; without it people get `name.001...` and a 7-Zip "Combine files" line. Piece attachments are hidden for plugin users. Optional "Send as a link" (Catbox, Litterbox, own ShareX `.sxcu`), off by default. Ideas from Testcord's BigFileUploadEnhanced (host list, .sxcu import), no code copied. `Install-TourneBigFiles.ps1` builds Vencord or Testcord from source with it and runs their installer.
- **`community/discord/`**: `tourne-server-blueprint.md` (roles per project, Tourne'Table presets and icon-pack forums, help forums asking for the right logs, FAQ from the showcase thread, templates, Big Files channel) and `build_server.py` (bot-based builder, now with `--dry-run` and a layout check). A bot, not a client plugin, builds the server: automating a user account breaks Discord's rules.

**Checked:** the plugin type-checks and builds in Vencord and Testcord; `native.ts` run under Node 22 with a stubbed Electron (pieces join to the same bytes, hashes match, a changed piece and other hosts are refused, unsafe save names are cleaned). `build_server.py --dry-run` prints the plan and its permissions build with discord.py 2.7.1. Not yet run inside Discord.

### Menus: the wheel, rules, opening animation, made ahead, and Tourne Menus for Explorer (2026-10-12, cloud)

Tourne looked at a cached context menu mod (Bolt-Scripts' Custom Cached Context Menu, MIT) and asked for its good parts in ours: build the menu ahead, hide and More options rules, an optional opening animation on our motion engine (default none, for speed), our menus in File Explorer windows, the file dialogs and Windows' immersive menus (each with a switch), and the wheel scrolling long menus (only Tourne Mod Tray had it).

- **The wheel** (`menu-style.cpp`, every menu through TrackStyledMenu: the bar's, Start's, the desktop's, System Icons'): as Tourne Mod Tray 1.0.1, a low-level mouse hook while a menu is open turns each notch over a menu window into Up/Down presses (3 items a notch). New: it works on the menu under the pointer (a submenu over it closes first, Esc), and fast turns go on from where the last one's presses will leave it while those are still queued (they used to stack up and overshoot). Logged as `wheel ...`.
- **Rules** (`ApplyMenuRules`/`UndoMenuRules`): `taskbar.menuHide` and `taskbar.menuMore` (labels or verbs, commas; case, `&`, a trailing "..." and accelerator text don't matter), `taskbar.menuMoreLabel` (More options). Top-level items only; separators left doubled, first or last go too; everything is put back after (Explorer keeps its menus). Applied to the desktop's menu and the bar's shell menus (jump list app menu), where verbs are known too.
- **Opening animation** (`g_menuMotion`): `taskbar.menuAnimation` none (default), fade, slide (10 px and a fade, from the side away from where it opens), unfold (a growing region). Length and curve from the motion engine's `popups` line (`motion.popups`, or its parents; off with `motion.enabled = false`), else 0.15 s easing out. Windows' own menu fade stays off. System Icons' menus follow the taskbar's setting.
- **Made ahead** (`desktop-menu.cpp`, `taskbar.desktopMenuReady`, off by default): the empty space's menu is kept built (again a second after each use and 0.3 s after the clipboard changes, so Paste is right); a file's starts building as the right button goes down (`prepare=1` from desktop-host.cpp) and is used if it comes up on the same items within 10 s; 5 s after start a thread builds and throws away one menu per kind of item on the desktop (40 at most) so their extensions are loaded. Logged as `desktop menus made ahead: N kinds in M ms`.
- **Published look**: the main bar writes its menu look, the animation and the rules to `HKCU\Software\Tourne\Menus` whenever its settings apply (`PublishMenuLook`).
- **Tourne Menus** (`tourne-menus.wh.cpp`, Windhawk mod 1.0.0, made from `tools/menus/tourne-menus.cpp` by `tools/menus/make-menus.py`, which puts in menu-style.cpp and motion.cpp): hooks TrackPopupMenu(Ex) and draws the menu with menu-style.cpp in the published look, with the wheel, the rules (labels only) and the animation, subclassing the owner for the drawing and for submenus filling in. Switches, all off by default: File Explorer windows (root CabinetWClass in explorer.exe; Windows 11's new menu skipped as m417z's Classic context menu mod does, IUnknown_QueryService for the presenter and CNscTree::ShouldShowMiniMenu), Open and Save dialogs (a #32770 with a folder view, any program), Windows' own menus (shown from the modules Classic Menus lists, made plain by the same CanApplyOwnerDrawToMenu answer). Shift + right-click gives Windows' own. Per-thread menu state (`MENU_STYLE_STORAGE thread_local`). Excludes TourneDesktop.exe.
- **Desktop icons stay where dropped** (`desktop/host/desktop-host.cpp`): Tourne reported Null.bin (the Recycle Bin) can't be moved. Since Windows 7 a hosted folder view keeps icons in arranged places unless its browser allows free placing (`IFolderViewOptions`, `FVO_CUSTOMPOSITION`), and our Browser never did. It now answers it (and sets it on the view), and the desktop window answers WM_GETISHELLBROWSER (WM_USER+7) as Explorer's do. Desktop.log says when the view asks. Not done: icon positions aren't saved yet (`GetViewStateStream` is E_NOTIMPL), so they'd go back at the next start; nothing found yet for drops onto Null.bin; a right-drag can't start while `shell.desktopMenu` takes the right button.

**Checked (Wine):** rules apply and undo to the same menu (by label and by verb, a submenu item kept); a 121-item menu scrolls 3, then 12 after three quick notches, then 6 after two up; fade, slide and unfold end opaque with no region; Tourne Menus (hooks stood in) reads the published look, curve and rules, draws a File Explorer window's menu with Hide and More applied and puts it back, leaves other windows' menus alone. Tourne Menus compiles for 64 and 32 bit (clang) and with gcc -Wall; Tourne Desktop's taskbar and System Icons parts and desktop-host.cpp compile (the whole exe doesn't build here: plugin-runtime.cpp's `#embed` needs a newer compiler). Not run on Windows.

### Overflow folders: New folder, Auto sort, and names for icons with no tooltip (2026-10-12, cloud)

Tourne confirmed Tourne Menus installs and works (run as admin), then asked the hidden icons flyout's menu for a new folder with a picker of icons to fill it (also ones in other folders, which move) and a rename, and an Auto sort that groups icons by what their programs do or how they relate. Also, Claude's icon showed "(no tooltip)" in the flyout.

- **New folder in the overflow** (`system-icons-host.cpp`, `ShowOverflowMenu`): "Pick its icons", or "Start with" any overflow icon (labelled "(from Folder)" when it's in one). The folder opens in the flyout, then `PickIntoFolder` shows every overflow icon, ticked when in the folder; each pick moves it in (or a ticked one back out, loose) and the menu comes back until Done or a click away. "Rename" there names it in the flyout's name box. A folder's own right-click menu has "Pick its icons" too. Empty folders made here (no patterns of their own) now show, so they can be filled; delete them from their menu.
- **Auto sort**: "Loose icons (your folders stay)" or "All icons (folders made again)". Kinds come from whole words in the program's file name, its file description, its maker and its tooltip (`ProgramKinds`: Chat, AI, Recording, Media, Audio, Games, Downloads, Cloud, Security, Network, Dev, Hardware, Tools, in that order, so NVIDIA Share is Recording before NVIDIA is Hardware). Icons with no kind group by maker (`IconMaker`, "Corporation", "Inc." and the like cut) when two or more share one. A kind or maker with one icon makes no folder. "Folders as before auto sort" puts them back (`TrayFoldersBeforeSort`).
- **No tooltip** (`desktop/host/tray-host.cpp`): in shell mode our own tray passed an empty tooltip through (Explorer's side already fell back to the program's name). An icon with none now gets the program's file description, else its file name, so Claude's shows "Claude".

**Checked:** the System Icons part and tray-host.cpp compile (MinGW, no new warnings); kind and maker matching run natively on sample programs (Discord Chat, Claude AI, NVIDIA Share Recording, SteelSeries Sonar Audio, AlienFan Hardware, Steam Games; "unicode" matches nothing). Not run on Windows; the whole exe doesn't build here (`#embed`).

### Tourne Menus 1.0.1, Auto sort by folder names, a log for the bar's place (2026-10-12, cloud)

- **Tourne Menus 1.0.1**: the Open dialog Tourne tried is Tourne'Icons' image picker, which runs in windhawk.exe, a process the mod skipped (and Windhawk loads mods there only when they name it). It now names it (`@include windhawk.exe`) and takes the file dialogs there (only those). The mod also stays loaded with every switch off (no hooks), so a switch turned on later reaches programs already running.
- **Auto sort** (`system-icons-host.cpp`): a folder whose name asks for a kind takes it, from its words or its letters run together (`{.GAME | .SOCIAL | .AUDIO}` takes Games, Chat, Audio and Media; `// W I N D.u x | 95` Windows; `.//SYS [ TOOL ]` Tools; `.//SYS [ DEVICE ]` Hardware). Else the folder most of that kind's icons are in now. A kind with neither goes where its broader kind goes (Recording to Media, Media to Audio, Security to Windows; AI, Downloads, Cloud, Network, Dev to Tools), else to a folder of its own. New Windows kind (Explorer's icons). "All icons" keeps the folders that take a kind (emptied and filled again) and drops the rest. Words may end before digits ("hwinfo64").
- **The bar's place is logged** (`taskbar-host.cpp`, PlaceBar) when it changes: Tourne finds the bar's hit spots a pixel off during and just after a Windows screen snip. desktop.log shows only "SnippingTool.exe fills its screen" then "no program fills a screen" 14 ms later, nothing of the bar moving.

**Checked:** Tourne Menus compiles for 64 and 32 bit (clang) and gcc -Wall; the System Icons and taskbar parts compile; folder-name matching run natively against Tourne's folder names and 19 sample programs. Not run on Windows.

## Tourne'Icons 2.6: Installed apps list; the ink pack's video frames and program icon (2026-10-12)

**Video thumbnails** were covered by a green film: Explorer lays Windows icons **193 and 194** over a video's thumbnail (film edges with a see-through middle, 4:3 and 16:9, from the top left), and the ink pack had drawn whole films there. The earlier fix put back **23**, which is only Windows' film icon (wrong guess). The pack maker now makes 193 and 194 from Windows' own pictures, recoloured (film ink dark, holes ink green, middle still clear, every size keeping Windows' outline: `frames.py`), and draws 23 as a film again. Pictures' thumbnails use 191 (a faint border), still Windows' own.

**The grid icon** in Task Manager (System, Desktop Window Manager) and Open with (Command line RAR, Claude) is Windows icon **15**, the icon a program with none of its own wears. It was drawn as the app grid with a dot, which reads as "all apps"; it's a program's window now (title bar buttons in red, a side list, a pane). Why Claude's Open with entry shows it was not checked (its registered icon is likely missing).

**Settings' Installed apps** shows each program's uninstall entry's `DisplayIcon`, which is often not the program (its uninstaller, a copy under `C:\Windows\Installer`, an .ico), so nothing in the pack covered it. Tourne'Icons has a new list, **Installed apps** (HKLM 64 and 32 bit, HKCU; system components and updates left out). An entry whose icon is an uninstaller or Windows Installer's copy changes only that file, never every `unins000.exe`. In Lasting and the Kit the list shows but can't change them (Settings reads the file itself). Store apps in Settings did change; they look small because the ink drawings keep their margin at 24 px.

**Control Panel** (Large icons) showed Windows 7 era icons: each item names its own, mostly in its .cpl or .dll, so no list held them. A **Control Panel** list now reads them from the All Control Panel Items folder. Opening in Large icons is Windows' own setting (`HKCU\...\Explorer\ControlPanel`, `StartupPage` 1, `AllItemsIconView` 0). The lighter band behind "Adjust your computer's settings" is the third-party mod Control Panel Color Fix (chip33) on Tourne's PC: it paints that band the body colour plus 22; its setting with the offset off (or a manual 18,28,33) makes it #121c21.

Read-only flags don't matter: Tourne'Icons changes icons as programs read them, files stay as they are.

Add-on pack 2 (193, 194, 23, 15): `/mnt/project-files/tourne-icons/tourne-ink-addon-2.zip`, in Tourne's Downloads as `Tourne ink pack add-on 2.zip`. Versions: Tourne'Icons 2.6, Lasting 1.6, Kit 1.3, checked with clang i686/x86_64 and gcc -Wall; Kit built. The Installed apps and Control Panel lists are untested on Windows; next ink pack needs a Save lists after 2.6 so it can draw them.

Add-on pack 3 (91 rows, from Tourne's lists 2026-10-12 090931): Installed apps (57) and Control Panel (35, Tourne's own Programs and Features kept). Installed apps entries match by their plain name (no version, edition or bitness) so they share their Start drawing; Control Panel items are drawn for what they open (pack-maker `variants.py`). Some Control Panel items read their icon from the Bouquet theme's `imageres.dll` in Downloads (their DefaultIcon points there), so those rows name that file and get the matching Windows icon's ink drawing. In Downloads as `Tourne ink pack add-on 3.zip`.

**Ink pack 5 (every entry).** Tourne: "make an ico FOR EVERY SINGLE ICON, including chrome, including settings, everything" and drop the lighter fill inside drawings. pack-maker now has `EVERYTHING = True` (Tourne's changes, tray programs and entries showing an earlier pack's icon all get drawn; only Tourne's own drawings, Windows' blank and shadow layers stay), and a drawing's fill (`g`) is Tourne's dark panel green #1e3632 instead of #64aa89. 1125 rows, 925 drawings, 32 of Tourne's own (Settings.ico now for Settings and ms-settings shortcuts too). Every look-alike got its own drawing or joins its program (variants.py SPEC and SAME). Built from "Tourne'Icons lists 2026-10-12 090931"; in Downloads as `Tourne ink pack 5.zip`. Apply a pack asks about icons Tourne set: No replaces them too. Tourne Desktop's own button icons (`taskbar.buttonIcons` in tourne-gui.conf) still win on its taskbar.


## Tourne'Icons 2.7: Tourne Desktop's button icons move in (2026-10-12)

Tourne asked for Tourne Desktop's button icon swaps to live in Tourne'Icons, so its pack (and everything else) reaches Tourne Desktop's bar, which until now showed its own Button icons rows over everything.

- **Taskbar buttons list** (new, second in the picker): each open window's button and the pinned ones, told apart as Tourne Desktop does: by the app id the program chose for its windows (Windows' app resolver: web apps, Discord's `com.squirrel.Discord.Discord`, Store apps), else by the program's file name. A change there is a `button` row: Tourne Desktop's buttons, tray and Alt+Tab only (On the taskbar still changes a program everywhere).
- **Handed over** (`HKCU\Software\Tourne\Icons`, REG_MULTI_SZ of `program<tab>icon`, newest first): `Buttons` from the Windhawk mod's tool process, written with the list and at start, removed when the mod stops; `Buttons Lasting` and `Buttons Kit` from the other two (removed when empty). The tool posts WM_APP+4 to the bars.
- **Taken in once** (`taken-in-3.txt`): every `taskbar.buttonIcons` row in tourne.conf / tourne-gui.conf becomes a button row, Use everywhere or not; designs (`tourne:app-tiling`) stay as they are; a picture taken in earlier as a program row is used again. Tourne Desktop's first row still wins among them.
- **Tourne Desktop** (`program-icons.cpp` ReadTourneIconsButtons/WithTourneIcons, read again at most once a second or after WM_APP+4): its taskbar (`CustomIcon`), Alt+Tab (`EverywhereIcon`) and tray (`ProgramTrayIcon`) take Tourne'Icons' rows first. While the Windhawk mod's value is there, Tourne Desktop's own Button icons rows step aside (they were taken in); without it they work as before. The built-in Icon Redirect doesn't read them.
- **Packs:** a pack row for a program also moves that program's button rows (by file name or the same path; not app ids or patterns like `*\NVIDIA Corporation\*.exe`) to its new picture, and a button of a program counts as one of Tourne's icons when the pack asks. pack.tsv accepts `button` rows.

**Checked:** both mods compile for 32 and 64 bit (clang) and with gcc -Wall; Kit 1.4 built; Wine unit test of the button matching and the hand-over (newest first, the live value holding with an empty list, the Kit's not holding). Not run on Windows.

### Tourne'Icons 2.7.1: a live tray, and the ink pack draws Taskbar buttons (2026-10-12)

- **Live tray** (Tourne asked): Tourne Desktop reads Tourne'Icons' Taskbar buttons again whenever `HKCU\Software\Tourne\Icons` was written since its last look (the key's write time, `program-icons.cpp`), not once a second. When Tourne'Icons nudges the bars (WM_APP+4), the bar also asks its tray for the icons again, so in shell mode the tray's icons change at once. On Explorer's tray, System Icons' watch thread (`AppStateThread`) now also wakes on that key and themes the icons already there.
- **pack-maker** (`/mnt/project-files/tourne-icons/pack-maker/build.py`, not in the repo): `button` rows get the picture of the program or app they stand for (by file name, path or folder pattern, a Store app's package, or a word of the app id, like Discord's `com.squirrel.Discord.Discord`), else one drawn for their own name (YouTube as a web app: a video frame with a globe). A third argument reads the button rows from a live `icons.txt`. Add-on "Tourne ink pack buttons" (20 rows from the 20 rows Tourne'Icons 2.7 took in on Tourne's PC); Apply a pack, answer No to replace them.
- Taken-in pattern rows get a readable name (`*\Process Lasso\*.exe` is "Process Lasso", was "*").

**Checked:** both mods for 32 and 64 bit (clang) and gcc -Wall; Kit 1.4.1 built; Tourne Desktop's four parts compile; the reader re-tested under Wine. Not run on Windows.


## Tourne Menu settings laid out like LiteMenu's (2026-10-12)

Tourne: the LiteMenu port's settings were "super hard to understand" with submenus in submenus, since the Studio listed its rows flat (Items 1, Items 2..., each a depth, name, command and arguments). Asked: make them mimic LiteMenu's configurator window near 1 to 1.

LiteMenu's config is `%LOCALAPPDATA%\Packages\RedBaronDevelopment.PopEx_36fx7gy0frrbc\LocalState\config.xml` (MenuItem type 0 submenu, 1 item, 2 separator; DisplayName, Command, Parameters, IconNumber, IsShellApp, UseThumbnail; ActivationButton "Middle Button", UseMouse, UseHotkey, DesktopOnly, Theme and five colours). Its program folder under WindowsApps can't be read, and its window didn't open from the remote session, so the layout follows its config's fields.

- **New window** `desktop/host/menu-editor.cpp` ("Tourne Menu settings", native controls, its own thread so Tab works):
  - **Menu tab:** the menu as a tree (submenus bold, separators as a line), with Add item, Add submenu, Add separator, Remove, Move up and Move down under it. Drag an item onto a submenu's middle to put it in, or onto an item's top or bottom half to put it before or after. F2 or a slow click renames in place, Delete removes. Move up on a submenu's first item takes it out above the submenu (Move down likewise below). Beside the tree, **Properties**: Type, Display name, Command with a "..." (a program or file, a folder, Tourne's actions, a submenu that fills itself, nothing) and Parameters, with a line saying what the command does.
  - **Settings tab:** Activation (turn on, Mouse button Middle/Right/None, also on the bar's empty space, hotkey as Ctrl/Alt/Shift/Win and a key) and Look (a note that Tourne's menu look replaces LiteMenu's theme and colours).
  - **Bottom:** Copy LiteMenu's menu, OK, Cancel, Apply. OK and Apply write every `taskbar.menu.*` setting to tourne-gui.conf (`WriteStudioSettings`, backup `.before-menu-settings`), plus an empty row after the last so rows left in tourne.conf don't trail on.
  - With no rows yet it opens on LiteMenu's menu (its own commands turned into `tourne:` ones, its app ids into `shell:AppsFolder\`), which OK copies in.
- **Opening it:** the menu's own "settings" item (`tourne:menu-settings`, LiteMenu's `<setup-litemenu>`) now opens this window instead of the Studio (so the old copy-on-first-open moved here and `TmFlatten` went). In Tourne Studio the item rows are one row, "Menu items: Edit the menu (N rows)", that opens it. `TourneDesktop.exe --menu-settings` opens it in the running copy, or on its own.
- Not built: LiteMenu's per-item icon number and thumbnail (Tourne's menus follow Menu No Icons), its own theme and colours.

**Checked:** the host files compile with -Wall; the window itself under Wine (a small harness with the real menu-editor.cpp): Tourne's LiteMenu config.xml loads as the tree, the Settings tab reads alt+win+77 as Alt+Win+M, a drag puts an item into a submenu, and OK writes the rows (LiteMenu's commands as tourne: ones). Not run on Windows.

### Every motion line moves something (2026-10-12)

Asked: "do our tourne desktop animations even function?" Partly: only 5 of the 38 lines were read (bar, popups, switcher, border, borderangle), each off by default. Tourne: fix all five problems found, and wire the rest in.

**Fixed**
- A Motion preset or an animations block edit now reaches running parts at once: `SettingsOf` (desktop/host/host.cpp) also compares `motion.*` and the identity's values, so the part's settingsChanged runs.
- `popups` style picks the menu effect (popin and gnomed unfold, slide and slidefade slide, fade fades; menu-style.cpp `MenuMotionStyleWord`), in the bar's and System Icons' menus.
- Animations off now stops the border spin (borders.cpp) and the CSS bar's fades (css-bar.cpp).
- The bar with motion on but no style word slides instead of waiting and jumping (taskbar-host.cpp).

**Wired** (each: no line set, as before; the line or Animations off, at once)
- `start`: Start opens and closes faded and/or slid in from the bar's edge (layered only while it moves; popin/gnomed = fade and a short slide, no scaling). start-menu.cpp `StartMotion*`. Cascades not yet.
- `popups` / `fadePopupsIn` / `fadePopupsOut`: System Icons' flyouts open and close (close plays on a snapshot, so every close path is unchanged); the calendar's event window fades in; the bar's previews fade in (thumbnail opacity; see-through previews fade only their thumbnails).
- `layersIn` / `layersOut` / `fadeLayers*`: notifications slide in from the screen edge and out; plugin panels fade or slide in from their bar's side and out (desktop/host/plugin-panel.cpp); the floating pager's fade takes fadeLayersIn/Out.
- `workspaces` / `workspacesIn` / `workspacesOut`: the desktops pager's lit mark slides to the new desktop (fade style: fades across).
- `monitorAdded`: a bar made for a newly added screen slides (or fades) in; auto-hiding bars only fade.
- `windowsMove`: tiling moves windows to their tiles over the line (all of one layout change together; admin windows stay instant; the last step is the old exact placement).
- `fadeSwitch` / `fadeGlow` / `fadeShadow`: borders' colour change on the active window changing, with the line's curve; glow and shadow can have their own timing. `glowangle`: the glow turns on its own.
- `windowsIn` / `windowsOut` / `fadeIn` / `fadeOut` (and `windows`, `global` above them): Tourne doesn't draw other programs' windows, so these turn Windows' own window animation on or off (SPI_SETANIMATION, not saved, put back when Tourne Desktop quits). Speed and curve stay Windows'.
- Not used yet, said so in the Studio: specialWorkspace* (no scratchpad), fadeDim (no dimming), fadeDpms, zoomFactor, shadowangle (the shadow has no gradient).

**Checked:** the three parts compile as the build makes them (taskbar, switcher, systemIcons) and the host files with -Wall. The exe doesn't link here (plugin-runtime.cpp's #embed needs a newer compiler than this server's), so nothing of this has run; not on Windows either.

## komorebi motion set (2026-10-12)

Studio's Motion page has a "komorebi" chip beside Subtle..Hyprland, from Tourne's komorebi.json `animation` block (enabled, duration 200, EaseOutBack): global, windowsMove (tiling's moves), bar (slide) and border are `1, 2, easeOutBack`; borderangle off (komorebi's borders don't turn). komorebi's `fps: 200` has no match: tiling steps on a 10 ms timer, which Windows rounds to its own tick (about 15.6 ms unless something raised the timer rate). Its paddings and layout rules are tiling settings, not motion, and aren't part of the set. Compiles; not run.

## Tiling sets and layout by window count (2026-10-12)

Tourne asked for whole presets for how tiling behaves, like their komorebi.json. Studio now gives tiling its own page ("Tiling", after the switcher; `switcher.tiling.*` keys moved there by the page match now taking the longest prefix) with set chips at the top, the same mechanism as Motion's (generalised: `ChipSet` with whole keys, `PageSets()`, `Part::ChipSet`). Each set writes auto (on), layout, layoutRules, mainRatio, the five gaps, focusBorder and `motion.windowsMove` (plus `motion.enabled` when it moves):

- komorebi (Tourne's): bsp, `1: bsp, 4: grid`, 16 px edges and between (komorebi pads 8 for the workspace plus 8 per container side), no border, `1, 2, easeOutBack`.
- Hyprland: bsp (dwindle), 10 between (gaps_in 5 a side), 20 edges, border, `1, 4.79, easeOutQuint`.
- GlazeWM: columns, 20/20, border, no animation. dwm: verticalStack at 55%, no gaps. i3: columns, no gaps. Monocle.

New setting `switcher.tiling.layoutRules` (tiling.cpp `ParseLayoutRules`, `LayoutFor(monitor, n)`): komorebi's layout_rules, "count: layout" pairs, names in any case. Applies on screens tiled from the settings; a layout picked with the Tiling key's N stays. Unit-tested the parser under Wine; switcher module and studio.cpp compile; not run on Windows.

Not mapped from komorebi.json: window_hiding_behaviour (Tourne doesn't hide windows between desktops itself), cross_monitor_move_behaviour (no move-to-screen command yet), mouse_follows_focus (false anyway), per-workspace names.

The videos (2026-10-12): the "visualizer" window is EasyEffects (a Linux PipeWire audio effects app) on niri with waybar and a conky side panel; the second is komorebi + YASB on Windows with a theme switcher menu and kageiro templates. Ideas only.

## DankLinux plugins and Tourne'Table as a panel (2026-10-12)

Asked: port the DankLinux plugins Tourne picked (motor-dev/wallpaperCarousel, hthienloc/dms-plugins, xantrk/dgpu-sleep-monitor, Mithgroth/dms-process-list, psyreactor/dms-awcc, YoungJurry/dms-screen-recorder), then make Tourne'Table 2.0 a portable panel, on a branch of its own (`claude/project-thread-7f685b`, made from `claude/project-thread-0rkfwp`) so the other session keeps a clean place to go back to.

**Plugin runtime** (desktop/host/plugin-system.inl, new, included by plugin-runtime.cpp): the Noctalia calls a Linux shell answers from the system, now answered from Windows.
- Programs: `noctalia.runAsync` (argv or a line; exit code, stdout, stderr, time-out), `noctalia.runStream` (a line at a time, \r ends one too; Tourne's onExit, `tourne.write`, `tourne.stop`), `commandExists`. Only programs named in the plugin's catalogue entry (`{"kind": "program", "label": "ffmpeg"}`) run, in a job object that ends them with the plugin.
- System: `systemStats`, `diskStats`, `outputs`, `focusedOutputName`, `processMatches`; Tourne's `processes`, `endProcess` (new grant "processes"; never core processes, Tourne Desktop or Windhawk), `gpus` (D0..D3 from SetupAPI), `battery`, `networkInterfaces`, `idleSeconds`, `busy`, `clipboardSequence`, `clipboardHasImage`, `snip`.
- Power (new grant "power"): `powerMode`/`setPowerMode` (Windows' overlay modes), `powerPlans`/`setPowerPlan`, `turbo`/`setTurbo`, `systemAction` (lock, sleep, hibernate, sign out, restart, shut down).
- Wallpaper: `wallpaperPath`, `wallpaperDirectory`, `setWallpaper` (IDesktopWallpaper), `tourne.thumbnail` (WIC, cached PNGs); plugins allowed "wallpaper" may read the wallpaper folder.
- Sound: `noctalia.sound.load/play` plus Tourne's `loop`, `stop`, `volume` (MCI, mpegvideo for every type so loop and volume work); C:\Windows\Media is readable for sounds.
- `tourne.tablePanel(what)`: shows, hides or pins Tourne'Table's panel (below).
- Settings in plugin.toml can carry `label`/`description` without translation files.

**15 plugins + 1** in desktop/plugins/bundled/tourne, listed in bundled/index.json with their needs (all off until installed): Wallpaper Carousel, Process List, dGPU Monitor, Power Modes (from AWCC; fans and lighting left out), Screen Recorder (ffmpeg gdigrab, NVENC/AMF/QSV), Timer, Hydrate, Take a Break, Breathing, Kaomoji (3,200), Emoji, IP Indicator (ipwho.is), Media Downloader (yt-dlp queue with progress, SponsorBlock), Ambient Sounds (white, pink and brown noise made by the plugin; your own files; mixes; sleep timer), OCR Scanner (Windows' snip + Tesseract), and Tourne'Table Panel (the bar button). GPL or unlicensed sources were ideas only (AWCC, dgpu-sleep-monitor, OCR, Ambient Sounds' sounds and code); hthienloc's MIT ones were ported with credit. Left out: activateLinux and desktopWidgetToggle (no desktop widgets yet), handMirror (camera), bongoCat, typingSounds, screenkey (a global keyboard hook), hiddenBar, lutrisLauncher, niriDS (Linux only), quickCapture (later).

**Tourne'Table**
- Place In = Panel, made in TOURNE-TABLE's parts (`BETA_BUILD_2.0/dev/src/splice2.py`, branch `claude/project-thread-7f685b` from `new-styles`): a window of its own the size of the visualizer. Drag to move, drag an edge or corner to scale (a zoom over the monitor's DPI, shape kept), Keep Panel on Top and Hide Panel in its right-click menu, Panel Hotkey, Acrylic or Mica behind it on Windows 11. Other programs use the registered message "TourneTable.Panel" (0 toggle, 1 show, 2 hide, 3 pin). One mod value keeps where it is, size, zoom, pin and hidden; hidden stops rendering.
- tourne-table.wh.cpp here is that build with `@id tourne-table-v2` (so it is new-styles' Place In = Taskbar too).
- Built into TourneDesktop.exe as the `table` module (off until turned on): Build-TourneDesktop.ps1 knows tool mods (`Tool = 'tourne-table'`): the launcher at the end stays out, and `TD_ToolInit` calls WhTool_ModInit once per run and never while a Windhawk mod with "tourne-table" in its id is on (`td::WindhawkModOn`, host.cpp).

**Checked:** plugin-runtime.cpp and host.cpp compile (mingw g++ 13 headers 13.0.0), every .luau compiles, every ui prop and glyph name is a real one. Table compiles as Windhawk's file and as the module the build makes; the other modules weren't rechecked with Table's headers in the shared prelude (this server has no C++/WinRT headers). Nothing has run: the exe doesn't link here and none of it is tested on Windows. Untested guesses to watch: the panel's Acrylic/Mica behind a DirectComposition window, MCI looping of .wav, and yt-dlp's progress template on Windows.

**After the first build on the PC (same day):**
- Linking failed: Tourne'Table's `@compilerOptions` ask for `-lwindowsapp`, and with it the compiler leaves kernel32 out (Toolhelp, QueryDosDevice, IsBadReadPtr, GetPrivateProfileSection went missing). Build-TourneDesktop.ps1 now skips `-lwindowsapp`; `-lruntimeobject` covers C++/WinRT (30eace9).
- Screen Recorder 1.1.0: "PC sound" is a Studio dropdown (None, Default output, each output by name; `options_from = "sound-outputs"` in plugin.toml, read when the Studio opens). `tourne.systemSound(output)` (new grant "systemSound") records the output with Windows' loopback into a named pipe ffmpeg reads (`-f f32le -ar -ac -i \\.\pipe\...`); silence is played to that output meanwhile so the sound has no gaps. Microphone (the old "Sound from") mixes in with amix.
- It couldn't keep 120 fps: gdigrab copies the screen with GDI on the CPU. Capture now defaults to ffmpeg's ddagrab (Desktop Duplication, ffmpeg 6+; `tourne.screenIndex(name)` gives its output_idx), with GDI as a choice and the fallback when the screen isn't on the first graphics card. NVENC takes ddagrab's frames as they are; other encoders and scaling get `hwdownload,format=bgra` first. 144 fps added.
- The Extensions page no longer says plugins run "once the runtime is in".
Untested on Windows, like the rest: the loopback pipe with ffmpeg, ddagrab with AMF/QSV after hwdownload.
- Tourne'Table setups can be shared (TOURNE-TABLE 2a2ce8b, copied into tourne-table.wh.cpp): the Style Editor's Share row (Copy code / Paste code as one `TT2:` line packed with Windows' XPRESS compression, Save file / Open file as `.ttstyle` text, Community: a list read from TOURNE-TABLE's `community/setups/index.txt` on `main` only when pressed, and Share Mine opens a ready GitHub issue). Parts: Look, Motion, Extras, Place (Place off at first); audio, interaction, hardware, validation, hotkeys and paths never go in. An import plays at once and asks Keep or No; it lives in the mod value `sharedSetup`, a layer every Wh_Get*Setting read goes through (macros after p4_share_layer.cpp), under quick settings; Reset Quick Settings clears it. Table now links `-lcabinet -lurlmon`. The community list is empty for everyone until `community/` reaches TOURNE-TABLE's `main`.
- Plugin hotkeys are set like the built-in ones: any plugin setting named `hotkey` (or with `format = "hotkey"`) gets the Studio's key picker (Press the keys, None, Type it out) instead of a text box. The picker writes key codes ("alt+45"), so the plugin runtime's ParseHotkey now takes those as well as typed names, and the row shows either as "Alt+Insert".
- Tourne'Table setups from Windhawk come into the Table built in here (host.cpp `ImportTable`): on the first start with the Table here empty, from the tray menu ("Bring in my Tourne'Table setups from Windhawk"), or `--import-table` (`--import` does it too). Every Windhawk mod with "tourne-table" in its id is read (settings from `HKLM\SOFTWARE\Windhawk\Engine\Mods\<id>\Settings` or its `Users\<SID>` copy, values from `...\ModsWritable\<id>\LocalStorage`, only read). The main copy (on, else changed last) gives its settings (tourne-gui.conf `table.*`, only those off the defaults, the old file kept as `.before-table-import`), quick settings, shared setup, place and panel; every copy's saved styles join My Styles (a name already here keeps its own); and each copy becomes `Documents\Tourne'Table setups\<id> (Windhawk).ttstyle` for the Share row's Open file. Untested on the PC.

## Claude Cockpit plugin (2026-10-13)

nightwatch75's Claude Cockpit for Noctalia (MIT) ported to Tourne Desktop as `desktop/plugins/bundled/tourne/claude-cockpit`, listed in bundled/index.json (off until installed; LICENSE kept, author credited). Its Bash helpers are Luau now: `usage.luau` is get-claude-usage (`--json`, same shape; LiteLLM prices, Frankfurter rates, the usage query with the login token sent only to api.anthropic.com, never logged or stored, caches in the plugin's data folder, no token refresh as in the original), `live.luau` is live-claude-sessions (`~\.claude\sessions\<pid>.json`, pid alive and not reused via `tourne.processInfo`, transcript stats kept in memory per size and mtime), `sessions.luau` is list-claude-sessions and find-claude-md, focus-claude-session is `tourne.focusProcess`, deletes go to the Recycle Bin, resume opens Windows Terminal (or a console window) in the project folder, and the editor is VS Code or Zed from their install folders.

**Runtime** (plugin-extras.inl, plugin-runtime.cpp, plugins.cpp):
- `tourne.jsonLines(spec, cb)`: JSON lines files read a line at a time on a thread (match, dotted fields, first, last, tail, maxRows).
- `tourne.findFiles(root, spec, cb)`: find below a readable folder (name patterns, prune, maxDepth, newerThan, max; junctions not followed; one minute at most).
- `tourne.trash(paths, cb)`: the Recycle Bin, only inside a new "deletes" need's paths and never those folders themselves; a link is removed itself.
- A "reads" need path ending in `*` covers names that start with it (`%USERPROFILE%\.claude*`).
- Also in, for the two Claude ports still to come (Claude Sessions, Claude Code Usage): `runInTerminal`'s env table, `fileInfo().created`, and bar tooltips as `{left, right}` rows.

**Checked:** every cockpit `.luau` compiles (vendored Luau), plugin.toml, en.json and index.json parse, plugin-runtime.cpp and plugins.cpp pass mingw `g++ -fsyntax-only` (only the known `#embed` errors). Nothing has run on Windows yet. To watch: the pid-reuse check (Claude Code's `startedAt` against the process start time, 10 s slack), `claude.exe` only (an npm install's `claude.cmd` isn't started), and the "reads" grant on the whole user folder that the CLAUDE.md scan needs.

## Plugin items as tray icons: overflow, folders, own icons (2026-10-13)

Plugin bar items now place like tray icons, under the key `plugin:<key>` in the tray's own `HKCU\Software\TourneTray` values. Nothing moves until the user moves one: an item with no place is in the tray (TrayPlaces::Set keeps no "tray" place for a plugin item, so putting one back leaves nothing behind).

- **Bar** (taskbar-host.cpp): Shift+right-click on an item gives Show in the tray, Move to the overflow, Change icon... and Put back original icon (a plain right-click is still the plugin's own). Dragging one onto System Icons' overflow arrow (resting there opens the flyout) or onto the open flyout puts it in the overflow; the bar menu's Tray icons list has the items too. Items placed in the overflow or hidden take no room on the bar (`g_pluginAway`, read with the tray's places).
- **Overflow flyout** (system-icons-host.cpp, now inlining plugin-bar-shared.cpp): `RequestHiddenIcons(all)` adds the items as HiddenIcons with `plugin` set (tray-shared.cpp; HiddenIconKey gives `plugin:<key>`), asked of the plugin runtime with the bar's ask message (SendMessageTimeout, at most once a second; the runtime now answers `TourneTraySystemIcons_` windows too). So they show with the hidden icons, drag into and out of folders, stack into new folders, sit in the overflow menu's lists and drag back onto the bar. Their picture is the item's picture fitted in, else its glyph drawn at the icon size (PluginItemPixels). Left, right and middle clicks go back through the bar's event message, as the bar sends them.
- **Own icons**: `TourneTray\PluginIcons` (REG_MULTI_SZ, `plugin:<key><tab>file` or `file,index`). The picker takes .ico, .png, .svg, .jpg, .bmp, .gif, and for .exe/.dll/.icl/.cpl Windows' icon picker after it. The runtime (plugin-extras.inl: ReadIconSwaps, RenderIconSwap) draws the file at the size of what it replaces (the glyph's 16, or the picture's size) into the item's picture, so the bar and the flyout both show it; the bar's ask with lParam `kPluginAskIconsChanged` makes it read them again.

**Checked:** modcheck taskbar and systemIcons pass; plugin-runtime.cpp passes mingw `g++ -fsyntax-only` (only the known `#embed` errors). Nothing has run on Windows yet. To watch: a glyph drawn by GDI into the flyout (the tabler font is the runtime's private font, same process), and the flyout's ask waiting up to half a second while a plugin script is busy.

## Key binds and tournectl (Hyprland parity P1, start), lighter borders (2026-10-14)

From the parity map (`research/plans/hyprland-wiki-parity-map.md`), phase P1: Hyprland's binds and dispatchers, plus a hyprctl and event socket of our own. All in the Switcher part (`binds.cpp`, inlined after the borders, sharing its keyboard hook), so it runs in Tourne Desktop and the suite alike.

- **Binds** (`binds.enabled`, off; `binds.lines`, empty; `binds.file`, a hyprland.conf-style file read with the settings): `bind[e|r|l|n] = MODS, key, command, argument`, `unbind`, `$var = value`, `submap = name` / `reset`, `catchall`. Keys by xkb name or `code:NN`. Left alone in fullscreen programs unless `bindl`. Win or Alt binds press the unused key so Start and menu bars don't open. The hook only does a table lookup; the command runs on the switcher's thread.
- **Commands (39)**: exec, execr, killactive, forcekillactive, closewindow, killwindow, togglefloating, setfloating, settiled, fullscreen (maximise), minimize, pin (topmost), movefocus (any window in view that way, across screens; Monocle falls back to the tiling's), movewindow (tiled: swap; floating: to the screen that way, or mon:N), swapwindow, resizeactive, moveactive, resizewindowpixel, movewindowpixel (px, %, exact), centerwindow, cyclenext (prev, tiled, floating), focuswindow, focusmonitor, focuscurrentorlast, focusurgentorlast (same), workspace / movetoworkspace / movetoworkspacesilent / renameworkspace (Windows' desktops through Task view's manager; N, +N, -N, previous; no new desktops made), layoutmsg (cycle, tile, auto, layouts, apply N, mfact N), splitratio, submap, sendshortcut, alterzorder, bringactivetotop, movecursor, movecursortocorner, dpms, lockscreen. Windows by class:, title:, exe:, pid:, address:, with * and ?.
- **tournectl** (`binds.control`, off): pipe `\\.\pipe\tourne-ctl-<session>`, local clients only. Requests: dispatch, activewindow, clients, workspaces, activeworkspace, monitors, binds, submap, reload, version, listen. `listen` streams activewindow, activewindowv2, openwindow, closewindow, workspace (from the registry's change notice, also for Windows' own switches) and submap; their hooks exist only while someone listens, and a listener that doesn't read loses lines rather than holding Tourne up. `desktop/tools/tournectl.cpp` builds beside the exe (Build-TourneDesktop.ps1; Swap-TourneDesktop.ps1 copies it).
- **Shared**: Task view's desktop manager moved out of desktops-pager.cpp into `virtual-desktops.cpp` (with the registry's desktop list), inlined by the pager and the binds.

**Borders, idle cost** (borders.cpp): `borders.spinRepeat` (once, new default; always is the old way): the gradient turns once as a window comes to the front or leaves it, then stands still and nothing is drawn. `borders.followMoves` (front, new default; all is the old way): the window-move hook follows only the front window's thread (moved with the front) plus move-size ends, instead of every move on the PC including the mouse pointer's; the tiling's moves and window reorders refresh the borders.

**Checked:** mingw `g++ -fsyntax-only -Wall` on the generated switcher and taskbar modules (winrt headers stubbed: only the known winrt errors, nothing in the changed code); tournectl.exe links with mingw. Nothing has run on Windows yet. To watch: bind keys with Win held (Start shouldn't open), workspace switching on 24H2/25H2 only (the manager's slots), and the front-only move hook when an app moves its window from another thread.

## Window rules (Hyprland parity P1/P2) (2026-10-14)

`window-rules.cpp` (in the Switcher after binds.cpp): `windowrule` / `windowrulev2` lines in the same Binds lines or binds file, under Key binds' switch. `windowrule = RULES..., MATCHES...`: matches are class:, title:, exe: (initialClass/initialTitle read as class/title), all must match, * and ?; Hyprland's simple regexes are translated (`^(a|b)$` to "a, b", `.*` to *). Rules, each once as a window opens: float, tile, center, size W H, move X Y (px or % of the work area), workspace N [silent], monitor N, pin, maximize/fullscreen, minimize, opacity A, noborder (borders.cpp `g_noBorder`). exec's `[rule; rule]` now apply to the first window of the started process within 20 s (was dropped). Hooks (show, uncloak, destroy) exist only while there are rules or an exec waits.

**Checked:** mingw `g++ -fsyntax-only -Wall` on the generated switcher module (winrt stubbed), nothing in the changed code. Not run on Windows.

## Window groups (Hyprland parity P3, start) (2026-10-14)

`tiling-groups.cpp` (Switcher, after the window rules): windows stacked in one place with a bar of tabs over them (a small no-activate window of our own, painted with GDI, the shown tab in the accent colour, click a tab to show it). Commands: togglegroup, changegroupactive f|b|N, moveintogroup l|r|u|d, moveoutofgroup. The tiling gives a group one tile (`WindowsOn` folds a group to its shown window; `TileMonitor` hands the tile to `PlaceGroupTile`, which takes the bar's height off its top and stacks every member there). A floating group keeps the window's place with the bar over it. One foreground hook exists only while groups do (a member brought to the front takes its tab along; the bar is raised with it). Not yet: lockgroups, group-on-move rules, the bar following a floating group dragged by hand.

**Checked:** mingw `g++ -fsyntax-only -Wall` on the generated switcher module (winrt stubbed). Not run on Windows.

**Open bug (Tourne, 2026-10-14):** sometimes the mouse doesn't register on the tray and clicks don't land after a rebuild, and sometimes while another window is focused; not reliably reproducible. Tourne suspects taskbar size, hiding or dynamic shyness. Worth checking first against 1ec26ed's resting bars (pointer checks stop after 2 s idle) and the bar's hit-test after a size change. Not looked at yet.

## Scratchpads (Hyprland's special workspaces) (2026-10-14)

binds.cpp: `movetoworkspace, special` (or `special:name`, `movetoworkspacesilent` the same) puts a window away: set floating (the tiling closes up behind it) and minimised. `togglespecialworkspace [name]` brings that scratchpad's windows to the current Windows desktop (Task view's manager), restores them floating at 80% of the active screen's work area, centred and cascaded, and focuses the last; again puts them away. Moving a window to a numbered desktop takes it out of its scratchpad. Nothing is watched; it's all done on the command. Syntax-checked only.

Tray click bug, a lead: a hidden (shy) bar is made click-through, and a show or hide cut short (a rebuild, or focus jumping to another window) could leave it click-through while it shows. Check that path first.

## overview and pickcolor commands (2026-10-14)

binds.cpp: `overview` (also `hyprexpo:expo`) opens the switcher's Mission Control, every window on every screen, as Win+Tab does when set; again closes it. `pickcolor` copies the colour under the pointer as #rrggbb (and sends a `colorpicked>>` event). Syntax-checked only.

## Idle listeners (hypridle) (2026-10-14)

binds.cpp: `idle = SECONDS, command, argument` lines run any bind command once you've been away that long (GetLastInputInfo), e.g. `idle = 300, dpms, off`, `idle = 600, lockscreen`. One timer set for the next due; after one has run, a look every 2 s while you're away to set them up again on your return; nothing while a fullscreen program is in front; no timer without idle lines. Syntax-checked only.

## Mouse binds and desktops made on demand (2026-10-14)

binds.cpp: `bindm = MODS, mouse:272|273|274, movewindow|resizewindow` (a modifier required). The switcher's existing mouse hook is now also there while a mouse bind is (`WatchWheel` ORs `g_mouseBindsOn`); a press with exactly those modifiers is kept back, the window under the pointer is floated (the tiling closes up), brought to the front (restored first if maximised, keeping its place under the pointer) and follows the pointer, one move handled at a time (`g_mouseDragPosted`). resizewindow moves the corner nearest where the drag started (120 px least). Injected input and fullscreen programs are left alone; Win or Alt held gets the dummy key so Start or a menu bar doesn't open.

virtual-desktops.cpp: `VdmCreate()` (IVirtualDesktopManagerInternal slot 11, IVirtualDesktop::GetID slot 4 on 24H2/25H2). `workspace N` / `movetoworkspace N` to a number past the last desktop make the missing ones first (up to 20), as Hyprland makes a workspace on first visit. The starter binds file has both mouse binds on WIN+ALT.

**Checked:** mingw `g++ -fsyntax-only -Wall` on the generated switcher module (winrt stubbed). Not run on Windows. To watch: the slot-11 create on Tourne's build, and that a held WIN+ALT drag doesn't leave Alt's menu bar open.

Being built on the PC at the same time (not in this section): keybind guide, keystroke overlay, Studio pages for binds/tiling/rules, the rest of hthienloc's dms-plugins.

## Layers of three tiling layout (2026-10-14)

Asked by Tourne (screenshots: one window full, two at half, three as main plus two stacked, taskbar left free). tiling.cpp: `TileLayout::Layers` (setting value `layers`, in both Layout lists and the layouts file names; also in the layout cycle). The first three windows go as the vertical stack grows (1 fills the work area, 2 halves, 3 main left and two stacked right, main share = Main window's share); windows 4 to 6 are a layer over them in the same three places (4th in the main even alone), 7 to 9 the next, and so on. Moving and focusing by direction stay within the window's own layer. Drag-to-swap (`DropTiled`) now picks, where places lie over each other, the window seen under the pointer (z-order). Win+arrows: bind lines `bind = WIN, left, movewindow, l` (r, u, d), which take Win+arrow from Windows' snap. Syntax-checked only.

## Window rule keep, and Where it is now (2026-10-14)

`keep [S]` (window-rules.cpp): for S seconds (15) after a window opens, its monitor/move/size/center rules are put back whenever its own program moves or sizes it (a LOCATIONCHANGE hook for that process only, ended by a timer, by a user drag via MOVESIZESTART, or when it closes). For Discord with Vencord, which restores its own size after showing. Keys and rules > Window rules gains "Keep it there" and "Where it is now" (fills screen N, place and size from the picked window's current frame, floating). Tourne's own: Discord and Spotify on screen 3 (the right one; screens count left to right), lines 63 to 65 of switcher.binds.lines. Not yet seen on a real Discord start.

**Tray click bug, more (Tourne, 2026-10-14 07:12):** the same bug also stops dragging things on the desktop (clip: /mnt/project-files/uploads/hearth/, 2026-10-14_03-08-52.mp4). Points again at a bar or overlay window left over the desktop that takes the drag. Queued after the animations work.

**Tiling flicker (Tourne, 2026-10-14 07:13):** a console window that opened for a second (likely a build or git step started on the PC) was tiled, shuffling the other windows, then everything moved back when it closed. To do: wait a short moment (e.g. 300 ms) before tiling a new window, and float console windows (ConsoleWindowClass, CASCADIA_HOSTING_WINDOW_CLASS) by default. Queued after the animations work.
## Desktop switches and windows opening, animated (2026-10-14)

Asked by Tourne: animated desktop switches and more window animations (first two of: switches, window animations, Studio Views page, shaders, extensions grid, dots).

- `desktop-slide.cpp` (inlined at the end of virtual-desktops.cpp, so every `VdmSwitchTo` caller gets it: binds' workspace commands, the pager and the YASB bar's workspaces): with a `workspaces`/`workspacesIn`/`workspacesOut` line (or global) set, a tool window (no desktop of its own, so the switch doesn't hide it) covers all screens with DWM thumbnails: what's on every desktop stays still (wallpaper = Tourne's Progman, bars, pinned), the old desktop's windows slide or fade out and the new one's (cloaked, their thumbnails still draw) in, each clipped to its own screen. Styles slide, slidevert, fade, slidefade N% (20%), slidefadevert. The real switch (`VdmSwitchNow`, the old function) runs under the cover once it's on screen. Own thread; a new switch ends a running one. Click-through or alpha-0 overlays (NVIDIA's, the visualiser) are left out: their thumbnails come out solid black.
- `window-motion.cpp` (Switcher, after tiling-groups): `windowsIn` (with fadeIn) animates windows as they show: popin N% (default 85%), slide [edge], gnomed, fade. Windows refuses DWMWA_CLOAK on another process's window (0x80070005, probed), but allows DWMWA_TRANSITIONS_FORCEDISABLED, and a layered alpha-0 window's thumbnail still draws (probed on Claude's Electron window). So: transitions off, layered alpha 0 in the SHOW event itself, a see-through cover (frame extended over it, black) moves the thumbnail, then the window's own style back and the cover gone after two DWM frames. Skips already-layered, topmost, tool, UWP frames, consoles, full-screen and small windows; at most 6 at once, 1.5 s. Close, minimise and maximise stay Windows' own.
- Harnesses: tools/desktop/desktop-slide-test.cpp (switches to the next desktop and back for real, shots partway), tools/desktop/window-motion-test.cpp (opens Character Map). Both run on Tourne's PC from a WMI-started process (from Claude's packaged shell the desktop switch fails): the slide and the popin look right in the shots.
- Risk to know: if Tourne Desktop dies mid-open, that window stays see-through until it's restyled (window rule opacity 1 or reopening it).

## Studio Views page, and shaders (2026-10-14)

- Studio: a **Views** page (`switcher.view.` prefix, after the Switcher's), Alt+Tab's seven views as cards with moving D2D previews (studio.cpp `DrawViewPreview`), a click writes `switcher.view.style`; the view's rows follow. Checked on the PC (window capture of the Studio).
- **Effects** (desktop/host/screen-effects.h, written for Tourne): crt, vhs, grain, chroma, pixelate, dither, halftone, glitch, vignette, duotone, bloom, invert, dark, as HLSL `EffectWarp` (where to sample) and `EffectColour` (after), needing `EffTake`/`EffBloom` from the shader they go in.
- **Glass slot**: `glass.effect` / `glass.effectAmount` run them after the glass pass (glass-engine.cpp inserts them before PS); vhs, grain and glitch keep the glass animating like the oil film. `--glass-preview out.png --effect all` draws each.
- **Screen shader** (desktop/host/screen-shader.cpp, Studio page "Screen shader", `shader.screen`, `shader.amount`, `shader.screens`, `shader.pauseFullscreen`): per screen a thread with Desktop Duplication on that screen's adapter, the effect into a composition swap chain on a topmost WS_EX_LAYERED|TRANSPARENT|NOREDIRECTIONBITMAP window (DirectComposition), `SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE)` so duplication (and recordings) never see it. Rotated screens: DXGI rotation 2 samples (y, w - x), 4 (h - y, x) (checked on Tourne's two portrait screens). Steps aside while a caption-less full-screen window is in front. Off by default.
- Checked on the PC: `--shader-shot EFFECT N out.bmp` pictures of all three screens; tools/desktop/screen-shader-test.cpp ran the overlay live for 7 s: shown, the window under the screen's middle is the app below (clicks go through), about 144 frames a second while the screen moves, gone after stop. Not yet looked at by eye.

## Extensions grid and dots (2026-10-14)

- **Extensions** (desktop/host/extensions.cpp): looks as data. Ten built in (Arcade CRT, Lo-fi VHS, Newsprint, Film grain, Pocket, Material motion, Bouncy, Exposé, Vista Flip, Glass tube); others are a folder's extension.json, copied into %APPDATA%\Tourne\Desktop\extensions\<id>. `ExtensionMaySet` allows motion., glass., shader., switcher.view./tiling./borders./theme., theme., identity. and refuses .exe/.command/.args/.path and glass.app. Cards on the Extensions page before the plugins (g_tiles holds -1 - n for them), "Turn on" writes the settings like a preset, "Turn off" takes the Studio's own lines out again. Examples in desktop/extensions (one refused); tools/desktop/extensions-test.cpp.
- **Dots** (desktop/host/dots.cpp): Presets page "Export my look…" writes a .tourne (tourne-gui.conf's look settings through WriteConfig, nothing with a path, the added extensions as dots.extension.N); "Import a look…" reads a .tourne or a hyprland.conf, shows what changes by page and what's left out, copies tourne-gui.conf to tourne-gui.conf.before-import, adds the extensions, and records the settings as Studio changes (Undo, Save). From Hyprland: animations and beziers (ParseConfig already reads them), gaps_in (x2 to Tourne's inner gap), gaps_out (1, 2 or 4 numbers), layout (master: verticalStack, else bsp), border_size, col.active_border and col.inactive_border (rgba()/rgb()/0xAARRGGBB and Ndeg to Tourne's gradient), rounding, shadow on/off, blur size; binds and exec are left out (said so). tools/desktop/dots-test.cpp: the sample in tools/desktop/dots-sample maps as expected; Tourne's real settings export to 380 lines and read back with none left out.
