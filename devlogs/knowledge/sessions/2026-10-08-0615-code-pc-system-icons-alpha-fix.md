# Why the system icons aren't Bouquet's fixed-alpha ones (read only)

When: 2026-10-08, about 06:00 to 06:15, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne: "my system icons aren't how they used to be"; they should be the ones in `%USERPROFILE%\Downloads\Bouquet Icon Theme (fixed alpha)` (the repaired imageres/zipfldr frames).

Found:
- b53's Icon Redirect is on and its `themePaths[0]` is still the fixed-alpha theme.ini (the folder and its theme.ini are there). Explorer (all three explorer.exe) loads b53, so Explorer's own windows still get them.
- Tourne Desktop doesn't: `Set-NoMenuOverlaps.ps1` (00:21 today) put `TourneDesktop.exe` in b53's Exclude (`vgc.exe|TourneDesktop.exe`) to stop its menus being styled twice. Both TourneDesktop.exe processes load only windhawk.dll now. Everything Tourne Desktop draws with the shell's icons (the desktop's hosted view, Start's places, the desktop menu, file dialogs) is stock Windows.
- Tourne Desktop has no Icon Redirect of its own: host.cpp reads b53's `iconRedirect.*` into its settings (ReadSuiteLayer) but no module uses the rows or theme paths. It relied on b53 being loaded into it.

Options put to Tourne (nothing changed):
1. A suite build where, inside TourneDesktop.exe, only Icon Redirect runs (Exclude drops TourneDesktop.exe). Keeps the menus fixed.
2. Take TourneDesktop.exe out of the Exclude again: icons back at once, double-framed menus back too.
3. Icon Redirect built into Tourne Desktop (reads the same theme paths). Most work.
