# 2026-10-11 11:35 Claude Code on the web: Tourne'Icons 2.1, Store apps

**Asked:** Store apps can and should be changed too (Tourne: "I've already done it with Tourne desktop and b55").

**Found:** b54 and b55 are only on Tourne's PC, in `X:\Documents\Claude Workspace\tourne-suite-sab-clone\builds` (not on GitHub). b55 changes Store apps by app id in `taskbar.buttonIcons`, drawn by Tourne's own taskbar, Start, Alt+Tab and tray; Windows' own places keep the old logo.

**Did:** Tourne'Icons 2.1.0: Store apps' logo pictures are made at every size the app ships and served in place of the originals by an `NtCreateFile`/`NtOpenFile` hook; the picker lists Store apps from the taskbar, Start and shortcuts; take-in also reads the suite's Button icons (app ids included). Details and checks: DEVLOG, "Tourne'Icons 2.1: Store apps too".

**State:** Wine-tested only. Not on Tourne's PC yet. 2.1.1: readme written for a new user (needs only Windhawk; take-in only when Tourne'Suite or Tourne Desktop were used).

**Next:** Tourne installs it in Windhawk, turns b53/b55's Icon Redirect off, and checks Claude's and Settings' logos on the taskbar, in Start and in Settings' app list. If Start keeps an old tile, "Refresh icons" in the tray menu.

**Later (11:55):** Tourne asked for a separate version that sticks with the mod off: `tourne-icons-lasting.wh.cpp` (generated, see CLAUDE.md). Tourne also saw "That couldn't be saved" sometimes: the list's move-over failed while something held the file; now retried, and the message says why. Both Wine-tested only.

## Later (12:50)

- Tourne pasted b55's redirects into Tourne'Icons' settings and got YAML errors: the list items were at mixed indents and `allResourceRedirect` was indented under the list. A cleaned block is in `/mnt/project-files/tourne-icons/tourne-icons-my-settings.yaml` and Tourne's Downloads (Tourne's own, not defaults; 5 rows dropped: a duplicate, three command lines, a window title).
- Lasting 1.1: same settings block as live (generated), and they're written lastingly (shortcuts, HKCU DefaultIcon for every class, Shell Icons). See DEVLOG "Tourne'Icons Lasting 1.1". Wine-tested only.
- Next: Tourne pastes the settings into both, checks file type and folder icons with Lasting on and then off.

## Later (13:20)

- Tourne asked for a drag-and-drop installer for .wh.cpp files. Their PC runs Windhawk 2.0 (has windhawk-cli.exe), so `tourne-drop-install.wh.cpp` hands each dropped file to `windhawk-cli mod install --file`, new mods turned off. Wine-tested with a stand-in CLI only. See DEVLOG "Tourne Drop Install".

## Later (13:40)

- Tourne asked for Notes (the plugin panel in Tourne Desktop) to wrap words, take line breaks, resize, and have a stay-open toggle. Done in the panel runtime for every plugin panel: wrap, edge resize, top strip to move, pin (kept per panel with the size and place). Enter already made new lines in the Wine test; if it doesn't on the PC, that's a separate cause to look for. See DEVLOG "Plugin panels: word wrap...". Wine-tested only; Tourne swaps it in with `Swap-TourneDesktop.ps1 -Commit <hash>`.
