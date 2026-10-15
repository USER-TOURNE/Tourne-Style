# Run works in shell mode; Windhawk in the bar's menu

When: 2026-10-08, about 00:10, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: in shell mode the Start menu's Run... did nothing, and Windhawk couldn't be opened from the clock's right-click any more.

What changed:
- start-menu.cpp: Run... calls `ShowRunDialog` instead of sending Win+R. Win+R needs Explorer's taskbar to answer it, which shell mode hides.
- taskbar-host.cpp: `ShowRunDialog` opens Windows' Run dialog through shell32 (RunFileDlg, ordinal 61) on its own thread, as the dialog is modal. `WindhawkExe` finds `%ProgramFiles%\Windhawk\windhawk.exe`; when it's there, the bar's menu (and the clock's right-click, which opens the same menu) has a "Windhawk" item under Task Manager.

Also this session (config, in the real profile): taskbar segments were set to none by mistake and put back from `tourne-gui.conf.before-segments-none`. Tourne wants the segments; the boxes they don't want are next to the Null orb (pager island and the empty-looking task buttons), still open.

Not done: "Show the desktop" in the same menu also sends a key (Win+D) and may have the same shell-mode problem. Side-monitor bars sit about 346 px below their screens. Ghost frames on unfocused windows during previews.

Check: Start > Run... opens the Run dialog; right-click the clock > Windhawk opens Windhawk.
