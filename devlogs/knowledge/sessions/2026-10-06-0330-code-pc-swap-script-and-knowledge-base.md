# Swap script, repo CLAUDE.md and this knowledge base

When: 2026-10-06, about 03:00 to 04:00, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at c691bbf).

Why: Tourne asked to "pull c691bbf, build, swap it in and restart", then to keep a memory of how, log the `Invoke-Real` line in the repo, give new sessions a way to do it without getting stuck, and make all sessions (phone app and desktop app on two accounts each, plus Claude Code) share knowledge so each picks up where the last left off.

What changed:
- Fast-forwarded to c691bbf (Switcher views Grid, Mission Control, Ring, Shelf; import from b53; peek keeps the desktop; Explorer starting late in shell mode). Built Tourne Desktop and swapped it into the running shell: the old exe is `%LOCALAPPDATA%\Tourne\Desktop\bin\TourneDesktop.exe.prev-5f6adb7`, the shell child restarted as PID 8544 at 03:17:39. `bin\TourneDesktop.commit.txt` now says which commit is installed (c691bbf).
- 5dcd29d: `HANDOFF.md` got the manual swap line (Invoke-Real).
- This commit: `desktop\Swap-TourneDesktop.ps1` (the whole swap in one command; tested with `-DryRun` only), `CLAUDE.md` at the repo root (new), `HANDOFF.md` marked out of date at the top with the working folder corrected, and this `knowledge/` folder.
- Made the claude.ai page "Tourne'Suite: where we left off" (https://claude.ai/code/artifact/dbb63574-12d7-494f-b141-cba652c118c3) and opened the pull request page for merging this branch into `main` (Tourne creates it).
- Outside the repo: Claude Code memory `reference_tourne_desktop_swap.md`, with pointers from both the home-folder and workspace memory indexes.

State: the swapped build runs as the shell with Explorer hidden underneath; `shell.log` was clean and no new `crash.txt`. The watchdog process is still the older build until the next sign-in. The new switcher views and the import are not yet tried by Tourne.

Known bug (reported by Tourne after the swap): tray icons can't be moved into the overflow tray; one sent there just disappears. Tourne's guess: it still thinks it is part of Windhawk. Seen on c691bbf in shell mode; **cause found and fixed, not yet tried live**: the Desktop tray host answered icon requests only from bars, so the system icons' overflow flyout got nothing (fix in `desktop/host/tray-host.cpp`, DEVLOG "icons moved to the overflow"). Not Windhawk: the placement was saved all along.

15:43: swapped in the overflow fix (5c5363c) with the script. The forced restart counted as a crash, the new build crashed in app styles' startup a second later, and the watchdog turned shell mode off; Tourne is on Explorer (`ShellTurnedOff` = 1, app styles marked to skip once). Fixed after: planned restarts (exit code 3) and one search-index query at a time; see DEVLOG. Lesson: under a watchdog older than planned restarts, a swap risks shell mode; the script now refuses without `-Force`.

Next:
- Get Tourne back into shell mode with the fixed build (Tourne chooses: start it now, or turn shell mode on again and sign out and in, which also makes the watchdog the new build), then have Tourne move an icon to the overflow.
- Plugins, decided by Tourne: a Plugins section on the Studio's Extensions page, cards with Install; Linux-tool plugins found and excluded; each plugin's needs shown on install, with permission per plugin. Started: `tools/plugins/Make-PluginIndex.ps1` (the scan); then the Studio page, the Luau runtime, then bar/panel/desktop placement.
- Tourne: try `switcher.view.style` = grid, mission, ring, shelf.
- Tourne: create and merge the pull request into `main` (46 commits behind; the page was opened in the browser) so new web sessions see this. The branch is pushed.
- Tourne: share the claude.ai page with the second account.
- Tourne: set up the Tourne Project on each of the 4 app logins (instructions text in `knowledge/README.md`).
- Open: `desktop/README.md` still says it "has not been run on Windows yet"; `HANDOFF.md` points at two memory files that don't exist (`reference_msstyles_inheritance.md`, `reference_windows_branding.md`); the 2026-10-05 plan says "proposal only" though much of it is built. Left for Tourne to decide.

Learned:
- Noctalia v5 is native C++ with Luau plugins (249 in two catalogues, 96% MIT); Tourne Desktop could host them. Full write-up: `research/plans/noctalia-plugins.md`.
- The project lived nowhere a home-folder session could find it; that cost the most time. The repo `CLAUDE.md` and the memory pointer fix it.
- A PowerShell helper named `Git` that calls `git` calls itself forever (names ignore case); it looked like a hung fetch.
- Killing the shell child with `taskkill /F` (exit 1) is the clean restart; exit 0 hands the session to Explorer.
