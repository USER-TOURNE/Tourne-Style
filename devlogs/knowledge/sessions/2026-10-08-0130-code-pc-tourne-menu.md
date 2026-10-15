# The Tourne Menu: LiteMenu built into Tourne

When: 2026-10-08, about 00:15 to 01:30, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: LiteMenu (PopEx) doesn't open over Tourne's desktop in shell mode. Tourne asked for it built into Tourne as a full replacement, widened with Tourne Desktop's own functions. Plan: `research/plans/tourne-menu-litemenu-replacement.md`.

What changed:
- 0fb7727 (step 1, live 00:07): host code showed LiteMenu's menu on the desktop from its config.xml (`shell.liteMenu`). Replaced by step 2 below.
- Step 2: `tourne-menu.cpp`, inlined in taskbar-host.cpp after start-menu.cpp. Settings `taskbar.menu.*`: enabled, button (middle, right, none), onBar, hotkey (alt+win+77 form), items rows (depth, name, command, args). With no rows, LiteMenu's config.xml is read at each opening. Drawn with RunMenu (Tourne's menu look, Menu No Icons). Commands: programs, files, folders, addresses, LiteMenu's shell apps and its tags (lock, restart, shut down, settings, close), and `tourne:` actions (show-desktop, run, start, task-manager, windhawk, studio, settings, lock, sleep, sign-out, restart, shutdown, desktop-next/previous/new/close, desktop:N). Live submenus: tourne:windows, tourne:desktops, tourne:programs, tourne:recent, tourne:folder:<path>.
- desktop/host/desktop-host.cpp: a middle click (or right click when menu.button = right; Shift keeps Windows' menu) on the desktop's empty space posts `TourneDesktop.ShowMenu` to a bar. taskbar-host.cpp handles it, the hotkey (kHotkeyMenu = 40) and the bar's empty-space middle click (menu.onBar).
- Config (real profile): `shell.liteMenu` removed, `taskbar.menu.enabled = true`. Backup `tourne-gui.conf.before-tourne-menu`.

Not done (stage 3 and 4 in the plan): a Studio page that edits the rows as a tree and copies LiteMenu's menu in; icons for program items; switcher views as actions; more than one menu. LiteMenu itself is still installed and running; it can go once Tourne is happy.

Check: middle-click the desktop's empty space (LiteMenu's menu in Tourne's look); its items open; .EXIT > lock.
