# Handoff: Tourne'Suite (as of b26, 2026-09-29)

> **Out of date for current work.** Since b53 the work is Tourne Desktop running as the shell. Read `CLAUDE.md` and the end of `DEVLOG.md` first; the build notes, gotchas and quirks below still hold.

Start here, then read the memory files `project_tourne_suite_sab_clone_goal.md` (history b1 to b26) and `reference_msstyles_loading.md` in `%USERPROFILE%\.claude\projects\X--Documents-Claude-Workspace\memory\` (on the desktop PC only); Windows branding pictures are in DEVLOG.md, "Windows branding pictures (Icon Redirect)".

## Where things stand

- The user runs **`local@tourne-suite-b25`** (enabled). Builds take their defaults from it.
- **b26 is built, compiles clean (x64 and x86, 0 warnings) and is not installed.** The next build is **b27**.
- Git: `USER-TOURNE/tourne-suite-sab-clone` (private). 5 local commits are ahead of origin; the user pushes from GitHub Desktop. The docs repo `Tourne-Suite-GitHub` (DEVLOG with b24 and b25 sections, images 28 to 31) is committed by the user, not by us.

## Next: see b26 live

1. **Install b26 and reboot**, then read `%LOCALAPPDATA%\Tourne\startup.txt` (each startup step in ms from the taskbar process's start). Open Start straight away: it should feel as quick as later opens once "icons fetched (ready)" is logged.
2. Wake from monitor sleep: the bars should re-place once, about a second after the screens settle, with no full reload.
3. Tooltips (*Tooltips*, *further from the bar*, *along the bar*, *delay*) and the aura (*trail*, *size*, *strength*) are checked in the test bar only.
4. Window previews: *Previews open*, *placed*, *further/along*, *stay up* (until dismissed: click elsewhere, focus change, Esc, any key, time away) and *After picking*. Only placement was checked (test bar); the dismiss watchers need real input.
5. Frames and stacks with Bouquet SAB (square 30 px frames, dark outer rows kept from the tint, straight stack edges): pixel-checked against the old SAB capture; look at a real group of 2 and of 3+ windows live.
6. Folding: Windhawk keeps folded groups per mod id in its webview localStorage (`windhawk-modSettingsCollapsed`), with no annotation for a default. Open question for the user: keep one mod id across builds so folds stick.

## Still to see from b24 and b25

Everything below was checked only outside its real process (test bar, test programs).

1. **b24 taskbar** (in the test bar with Bouquet SAB): the style's tray arrow (Open variant while the flyout is up), clock and tray icon hovers, the show desktop button (it will appear here: Explorer's TaskbarSd is 1), themed thumbnail previews. The preview part mapping (part 1 ground, part 3 state 1 hot and 5 flashing, part 12 close) is a guess; compare with SAB's own previews side by side.
2. **b25 Explorer Colours** (new part, starts off): switch it on, set *Drive bars* to "The style's shape, in your colours", open This PC. Then selections in the file list and navigation pane, the focus pill, and the preview pane (needs an Explorer restart).
3. **b25 Windows branding pictures** (Icon Redirect): point it at `%USERPROFILE%\Downloads\WINDux.95 basebrd images\<variant>` and open winver and the Alt+F4 shut down dialog.
4. Still open from b22: photograph SAB's own progress strip on a task button (scratch tool `progress\progsab.cpp` grabs without moving the mouse; needs an unlocked screen).
5. Not exercised by any installed style: `Theme::OpenOwn` dropping a themed style's missing tray classes.

## What each recent build did

- **b22:** task button progress (the Explorer relay subclasses `MSTaskSwWClass` for WM_USER+0x40/0x41 and posts `TourneSuite_Progress` to the bar), a crash guard (`HKCU\Software\Tourne\Suite`, 4 Explorer starts in 120 s trips it), no loading in Safe Mode, and a part's settings hidden while it's off (the build adds `#! $showIf: {enabled: true}`).
- **b24:** SAB's tray parts in the bar (`TrayNotifyHoriz/Vert(Open)::Button`, `TrayNotify::Clock`, `TrayNotify::Toolbar`, `ShowDesktop::Button`, `TaskbandExtendedUI`), each only when the style really has the class. The palette is seeded from the style's system colours (unchanged for the user's style). Theme Gaps `accentLeftovers` (off) swaps Windows' accent blues left in a style. Research: `research/msstyles/MSSTYLES-MAP.md`.
- **b25:**
  - **Explorer Colours** (`explorer-colors.cpp`, prefix `explorerColors`, x64 only, explorer.exe plus prevhost.exe plus `otherPrograms`).
    - Drive bars: Progress part 5 state 4 (filled), state 2 (nearly full), part 11 state 2 (empty).
    - Modes: the style's own, recolour (the style's picture drawn over black and over white to recover alpha, then remapped by brightness so its main colour becomes the chosen one), or flat.
    - Selections: TreeView and ListView parts, ItemsView part 3, and dui70's `PaintBackground` for the stale selection a click leaves.
    - Preview pane: a class brush, ReadingPane FILLCOLOR, and prevhost's RichEdit colours.
    - Ideas and part numbers come from VitalS's Explorer Visual Tweaks Dark (installed and disabled here; a copy is in `%USERPROFILE%\Downloads\windhawk-mod-sources\`). The colour defaults are the user's settings for that mod.
  - **Icon Redirect:**
    - A folder of `<id>.png/.bmp/.svg` can be a replacement.
    - *Windows branding pictures* points `Branding\Basebrd\basebrd.dll` at one; ids 123, 1123 and 2123 are the logo at its three sizes.
    - An SVG is rewritten to fill the picture it replaces (`FitSvg`), then drawn by Direct2D.
    - WIC and Direct2D load only when a conversion is needed, and never under the loader lock.

- **b26:** load time and feel. The suite waits for the bar's window only (12 ms, was 650 to 1400); icons and programs read once at startup; screen changes debounced and screens-only; theme notices that change nothing ignored; choices written once; no loading in LockApp or LogonUI. Start warmed while closed (first open about 30 ms, was 190 to 216) and *Open on press* (on). Tooltip placement and delay; aura trail, size and strength; window preview opening, placement and dismissal options; SAB-exact frames and stack edges. Boot log in `startup.txt`. Test bar: `TB_perf=1`, `TB_perfwait=ms`.
  - Ideas not done: Icon Redirect's per-call cost (a VirtualQuery plus a map lookup on every resource lookup in every program while *all resources* is on); first bar placement (165 ms) and app bar registration (up to 93 ms) are Explorer round trips.

## Where things are

- Working folder: `X:\Documents\Claude Workspace\tourne-suite-sab-clone` (was `%USERPROFILE%\Downloads\tourne-suite-sab-clone`).
- **Suite sources:**
  - The suite is generated from `suite-redirect.template.cpp` plus one source per part, listed in `Build-TourneSuiteRedirect.ps1` (`$parts`, `$startOff`).
  - Main taskbar source: `taskbar-host.cpp` (CRLF). Its `// @inline <file>` lines pull in the css-*.cpp files, start-menu, palettes, theme-palette, tray-shared and menu-style.
- **Build:** `./Build-TourneSuiteRedirect.ps1 -BuildNumber N` writes `builds\tourne-suite-bN.wh.cpp` and `-settings.reg`.
- **Compile check** (both architectures must be clean): `& 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' -target x86_64-w64-mingw32` (and `i686-w64-mingw32`) `-std=c++23 -fsyntax-only -Wall -DUNICODE -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -include windhawk_api.h -DWH_MOD -DWH_EDITING <build>`. Also check the settings YAML for glued lines (`\S- \w+: `).
- **Test bar:** `tools\compare\Test-Css.ps1 -Vars @{ TB_<setting>='value' } [-Filter regex] [-NoBuild]`, with output in `tools\compare\out-css`. Unset int settings read 0, so pass what you depend on. Flags:
  - `TB_hoverat='x,y'` (negative x counts from the right) and `TB_pressat`: capture to `hoverat.bmp` (zoom 2). Arrow 1715,19, clock 1880,19, show desktop 1915,19 on a 1920 bar.
  - `TB_previewtest=N` with `TB_previews='thumbnails'`: preview-hot.bmp and preview-close.bmp.
  - `TB_progress=1`, `TB_bars=1` (needed for TB_hover, TB_csshover, TB_popup and TB_cssclick), `TB_shytest=2`, `TB_shyshot=1`, `TB_gallery`, `TB_btntest`, `TB_dragtest`.
  - Frames are saved from what the bar hands UpdateLayeredWindow, so tests work while the PC is locked.
- **Other test programs** (each includes the part's source and supplies the Windhawk API itself):
  - `tools\compare\colortest.cpp`: Explorer Colours' drive bars and selections from the live style.
  - `tools\basebrd\brandtest.cpp <folder> [ids]`: points winbrand's imports at Icon Redirect's hooks and calls `BrandingLoadImage`.
- **msstyles tools:** `tools\msstyles\Dump-Msstyles.ps1` and `Compare-Msstyles.ps1` (msstyleEditor 2.1.2's library, loaded with UnsafeLoadFrom in Windows PowerShell 5.1). Dumps are in `research\msstyles\*.tsv`.
- **basebrd art:** `tools\basebrd\make.ps1` builds the WINDux.95 images from a pixel font.
- **SAB comparison:** `tools\compare\Compare-Matrix` and `Compare-Themes.ps1`.
- Docs: `%USERPROFILE%\Downloads\Tourne-Suite-GitHub` (DEVLOG.md, README.md, images/).

## How the suite works (what trips people up)

- **Parts:**
  - Each part is wrapped in its own namespace. `Wh_ModInit` becomes `PartInit`, and so on.
  - The template decides where each part runs (`SuiteEvaluate`), and its start, uninit, settings-changed and reload calls must all be added for a new part.
  - Parts marked `Win64` exist only in the x64 build.
- **Hooks:** parts call `Wh_SetFunctionHook`, which becomes `SuiteSetFunctionHook`. Hooks from several parts on one function are chained, so two parts may hook the same function.
- **Includes:** the build strips a part's `#include` lines. Every header must be in the template's include list (b25 added `<d2d1_3.h>`).
- **Settings:**
  - Setting names get the part's prefix only when passed as literals to `Wh_GetStringSetting`, `Wh_GetIntSetting`, `StringSetting::make`, `RedirectSetting`, `TextSetting` or `IntSetting`.
  - A `#! $showIf` a part writes itself is merged with the build's `enabled: true`.
  - Settings annotations must use the `#!` marker.
- **Defaults:** "your settings are the defaults". New parts go in `$startOff`, and new features default off, so installing a build changes nothing visible.
- **Taskbar:**
  - One bar per screen. Per-bar globals go in both `BarState` and `SwapBarState`.
  - The bar is a layered window drawn into a premultiplied top-down DIB.
  - Timers 1 to 17 are in use. WM_APP ids in use: +1 to +6, +20, +71, +72.

## Gotchas found

- **GetObject:** a DIB section's `dsBmih.biHeight` is reported positive even for top-down DIBs.
- **Test bar timers:** the test bar's own test code runs on the bar's thread, so bar timers never fire during its Sleeps. Call the handler directly.
- **MinGW Direct2D:** `d2d1svg.h`'s overloaded virtual methods don't match Windows' vtable order. A typed set fails and the string form crashes, so edit the SVG text instead.
- **Plain GDI and alpha:** uxtheme parts drawn with plain GDI (BORDERFILL) leave alpha at 0 in a 32-bit DIB.
- **PowerShell `R`:** `R` is PowerShell's alias for Invoke-History; don't name helpers `R`.
- **.NET file calls:** `[IO.File]` resolves relative paths against the process folder, not the PowerShell location. Pass full paths.
- **uxtheme inheritance:** a missing value comes from the part, then the class, then globals. `App::Class` falls back to the base class, but an app class that exists with its own common properties (like `DarkMode_Explorer::Progress`) wins over the base. Explorer Style's *Progress bars from the visual style* swaps that one for plain `Progress`.
- **Locked screen:** while the screen is locked (LogonUI), screen captures read black.

## Also in b26 (late)

- Shadows, idle fade, reveal zone/attention/Start for auto-hide, the green overflow arrow back (trayArrow), one-click tray icons from the clock's menu (with icons not running), Explorer Style's navigation pane lines and gaps (unverified here: the pane shows only This PC).
- Next big item: running without StartAllBack, planned in `research/disable-w11-shell.md`.

## Older open items

- b21 shyness animation and the b20 YASB batch (popups, workspaces, fades, CSS clicks) are checked in the test bar only.
- Known YASB limits:
  - Media works from window titles.
  - Weather needs `yasbNetwork` and the theme's own key.
  - The power menu has no full-screen blur.
  - The calendar changes month with the wheel only.
  - Nerd Font glyphs need a font in `%LOCALAPPDATA%\Tourne\Fonts`.
- Still missing: svg and .orb orbs, Explorer restyling while SAB is off, and real multi-monitor testing inside Windhawk.

## Environment quirks

- My shell virtualizes `%LOCALAPPDATA%` writes. Real reads and writes go through `Invoke-Real` in `tools\compare\RealReg.ps1`.
- The Edit tool fails if the file changed since it was read. Big edits to taskbar-host.cpp are easier with PowerShell string replaces, keeping CRLF.
- The Write tool turns `\uXXXX` into literal characters. Use `\x` escapes.
- `C:\ProgramData\Windhawk\ModsSource` is admin-only; copy mod sources to Downloads to read them. Mod settings live in `HKLM\SOFTWARE\Windhawk\Engine\Mods\<id>\Settings`.
- Git is at `%LOCALAPPDATA%\GitHubDesktop\app-3.6.6\resources\app\git\cmd\git.exe`. Write commit messages to a file (`-F`).
- **Swapping a new Tourne Desktop build into the running shell.** Use `.\desktop\Swap-TourneDesktop.ps1 -Commit <hash>` (`-DryRun` to preview, `-NoBuild` to reuse the last build); it does all of this, waits out the crash window and records the swapped-in commit in `bin\TourneDesktop.commit.txt`. By hand (after `.\desktop\Build-TourneDesktop.ps1`): rename the running exe (Windows allows it), copy the build in, then end only the `--shell-child`. The `--shell` watchdog sees a non-zero exit and starts the new exe from the same path. Find the child's PID with `Get-CimInstance Win32_Process -Filter "name='TourneDesktop.exe'"`. `<old hash>` is the commit being replaced.

  ```powershell
  . "X:\Documents\Claude Workspace\tourne-suite-sab-clone\tools\compare\RealReg.ps1"
  $bin = "%USERPROFILE%\AppData\Local\Tourne\Desktop\bin"
  $src = "X:\Documents\Claude Workspace\tourne-suite-sab-clone\desktop\build"
  Invoke-Real "cd /d `"$bin`" && ren TourneDesktop.exe TourneDesktop.exe.prev-<old hash> && copy /y `"$src\TourneDesktop.exe`" . && copy /y `"$src\TourneDesktop.symbols.exe`" . && copy /y `"$src\TourneDesktop.exe.manifest`" . && taskkill /F /PID <shell-child pid>"
  ```

  - Never use `--stop` or a clean quit. Exit code 0 hands the session to Explorer.
  - The kill counts as one crash. A second crash within 120 s (`kQuickCrash`) turns shell mode off.
  - The watchdog itself stays the old build until the next sign-in.
  - To check: tail `%LOCALAPPDATA%\Tourne\Desktop\shell.log` through `Invoke-Real`. Expect "ended (code 0x00000001): starting it again", then "started as the shell".

## Facts about the user's machine

- Windows 11 26200, a 144 Hz main screen, several screens, 2 virtual desktops. Windows animation effects are off; the bar's own animations must not follow that setting.
- StartAllBack is installed. The visual style is `mine.msstyles`: Bouquet MAC Night plus the user's tooltip.
- The YASB theme Everforest is in `%LOCALAPPDATA%\Tourne\Yasb`. The accent is green.

## User rules

- No em dashes anywhere.
- Short replies: the outcome plus the next step.
- Discuss review findings before applying them.
- Keep performance and load-time cost near zero.
- Every build: check it in the test bar, compile it for x64 and x86, write a DEVLOG section and update memory.
- Never run live audio probes.
- Don't move the user's real mouse in tests.
- Never click power actions.
- Ask before sending Ctrl+Win+Arrow.
- Never write `$host` in PowerShell.
- Use the user's email only for git authorship.
