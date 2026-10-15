# Programs left alone (Tourne's exclusions, as Windhawk's)

When: 2026-10-08, about 00:45 to 01:00, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne asked whether Tourne Desktop would flag games' anti-cheat. It never loads into other programs, but it draws around their windows, tiles them, and keeps keyboard and mouse hooks system-wide. Tourne agreed to exclusions the way Windhawk has them.

What changed:
- `desktop/host/exclusions.cpp` (new, in the build list): `shell.exclude` (names or paths, * and ?, | between), `shell.excludeGames` (on: Steam, Epic, Riot, Xbox, EA, Battle.net, Blizzard, Ubisoft, GOG game folders; Vanguard, EasyAntiCheat, BattlEye, EA AntiCheat, FACEIT), `shell.excludeWindhawk` (on: Windhawk's own Process exclusion list, read from HKLM\SOFTWARE\Windhawk\Engine\Settings\Exclude). Lists re-read when 3 s old, on demand; a per-process cache. A WinEvent watcher (EVENT_SYSTEM_FOREGROUND, out of context) tells registered callbacks when a left-alone program comes to the front or goes.
- `desktop/host/standalone.h`: td::ProgramExcluded, ProgramPathExcluded, ExcludedInFront, WatchExcludedFront for modules. Module code uses `LeftAlone()` / `LeftAloneInFront()`, which are false in the Windhawk suite build (#ifdef TD_PREFIX).
- Switcher: its hook thread lets go of the Alt+Tab keyboard hook and the wheel mouse hook while one is in front, and puts them back (WM_APP_LEFTALONE); no border (borders.cpp BorderWanted) or tiling (tiling.cpp) for their windows.
- Taskbar: the bar keeps away as from a full-screen program; the Start key hook is let go (LeftAloneMessage to the main bar).
- Killswitch: its key hook let go (WM_APP + 51 on the host window).
- shell-mode.cpp: defaults and Studio entries for the three settings and `shell.desktopMenu`.
- `tools/windhawk/Add-GameExclusions.ps1`: adds Riot's client and game programs to Windhawk's Process exclusion list (needs admin).

Not covered: the bar's previews and System Icons' menus hook only while they're open, so not during a game. Log lines start `[exclude]` in desktop.log.

Changed right after (Tourne: "alt tab and win key should still work inside games unless specified"):
- The hooks are no longer let go for every left-alone program. Two lists now: `shell.exclude`, games and Windhawk's list only stop the window effects (borders, tiling, the bar keeping away); `shell.excludeKeys` (new, empty by default) names the programs where the Alt+Tab, Start key and killswitch hooks are let go while one is in front. So Alt+Tab and the Windows key work in games unless a program is named there.
- exclusions.cpp keeps a verdict per process (alone, keys); ExcludedInFront and the watchers follow the keys list only.
