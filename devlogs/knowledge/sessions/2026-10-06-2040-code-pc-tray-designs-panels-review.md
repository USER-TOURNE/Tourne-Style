# Tray designs in shell mode, plugin panels, Studio Plugins page, review

When: 2026-10-06, about 18:10 to 20:40, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at d687478). A usage-limit pause around 19:00 to 20:30.

Why: after signing in again Tourne installed Notes (its icon showed, clicks did nothing, as expected) and asked: finish panels; the b53 icons are missing in Tourne Desktop ("I really don't want to have to use both"); the temperatures in the tray have a white background; look at older screenshots of the tray; keep doing the task list unprompted, then review it all. Tourne sent their 2026-10-03 screenshot as the target look.

What changed:
- 84f1603: other programs' tray icons in System Icons' designs in shell mode (the tray host asks System Icons, same process); `tourne:` designs for task buttons and Alt+Tab no longer stubbed; System Icons' settings replaced under a lock.
- 22347a2: plugin panels (`desktop/host/plugin-panel.cpp`), Noctalia glyph aliases, `expandPath`, folder settings, the flyouts' palette for plugin colours.
- d406daa: Studio Plugins page (each installed plugin's settings, from its manifest and translations).
- acf4744: DEVLOG sections; stale lines in HANDOFF.md and desktop/README.md fixed.
- f206e63: review fixes (below).
- Each swapped in with the script (planned restarts; no crash since 15:43). Running now: f206e63; the old exe is `TourneDesktop.exe.prev-d406daa`.

Review (done inline: the background /code-review agent stopped at the usage limit): fixed a use-after-free on every re-render of a panel with a scroll list (Relayout walked the old layout after the tree was replaced), a select reading the tree after its menu loop, and a sandbox hole (a "folder" setting's manifest default of "~" handed a plugin the whole profile; now refused for the profile and above, AppData, Windows, Program Files, ProgramData; the sandbox test tries it). Not reviewed in depth: the taskbar's plugin item drawing and the Studio page beyond building.

State: on Tourne's PC the temperatures are green numbers again (screenshot matched the 2026-10-03 look). Panels and the Plugins page are checked with `Run-PluginTest.ps1` (pictures of the panel window: Notes' list, "+" makes a note and opens the editor; World Clock's zones) but not yet clicked through by Tourne. Icons inside Explorer windows, Start and programs' own windows (Icon Redirect) need hooks: still a Windhawk part.

Next:
- Tourne: click Notes on the bar (panel on the right), World Clock (attached panel), and open Studio > Plugins.
- Plugins not done: images, tooltips, context menus and drag and drop in panels; quick toggles and launcher providers; YASB bars don't place plugin items; a single-line input's placeholder only shows with Windows' newer controls.
- The old waiting items: switcher views to try, the pull request into `main`, sharing the claude.ai page, the Projects on the app logins.

Learned:
- In shell mode anything the suite did inside Explorer (tray icon swaps, tooltip capture) has to be redone in Tourne Desktop's own tray host.
- A tree of layout records pointing into a tree of nodes must never be walked after the nodes are replaced; keep what survives a render by id.
- A plugin manifest supplies setting defaults, so anything a setting unlocks must be checked as if the plugin chose it.
- Screen captures fail with "handle is invalid" while the session is locked; PrintWindow of the window itself works anyway.
