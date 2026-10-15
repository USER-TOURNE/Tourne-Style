# Shell mode off, back to Explorer and the wallpaper

When: 2026-10-07, evening, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne asked why several Tourne Desktops were running and wanted the desktop back to how it looked before Tourne Desktop, wallpaper included.

What was found:
- Only one copy was running: the `--shell` watchdog and its `--shell-child`. The 16 `TourneDesktop.exe.prev-*` / `.old` files in `%LOCALAPPDATA%\Tourne\Desktop\bin` are backups from earlier swaps, not running.
- No explorer.exe was running at all, so nothing drew the wallpaper.

What changed:
- Ran `TourneDesktop.exe --restore` through `Invoke-Real`. Tourne Desktop closed, Explorer took over the session, the wallpaper shows again, and the real `HKCU\...\Winlogon\Shell` value is gone (Explorer is the shell at the next sign-in).
- Not done: putting b53 back to its full set of parts. Claude Code's safety system blocked the elevated `reg import`. The backup from before `-HooksOnly` is `tools/windhawk/local@tourne-suite-b53-20261007-161432.reg`; it has every part on but the mod itself `Disabled=1`, so after importing it b53 also has to be switched on in Windhawk.

Next: Tourne imports that backup as administrator and enables b53 in Windhawk (or picks another build), then says whether the look matches what they remember. To return to Tourne Desktop later: Studio > Shell mode.
