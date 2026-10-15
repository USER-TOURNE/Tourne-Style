# AlienFan tooltip, clock colours, switcher style dropdown, system text colour

When: 2026-10-06, about 20:45 to 21:00, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at f206e63).

Why: Tourne reported four things in shell mode: no tooltip on AlienFan's tray icon; the clock not in their colours (line 1 should be the palette's red, line 2 its green); not all system text the right colour; their old switcher setup gone, and its style should be a dropdown, not a text field.

What changed:
- 1807619: the hidden icons flyout posts WM_MOUSEMOVE to an icon's program when the pointer moves onto it (as the taskbar's tray does), then reads the tooltips again 250 ms later. alienfan-gui sets its tip only on WM_MOUSEMOVE (its tip was empty: `tools/tray/trayprobe.cpp` showed `alienfan-gui.exe id 107 v0 tip []`). `HoverHiddenIcon` is now in tray-shared.cpp.
- 1807619: the Studio reads choices parts list as they run (Windhawk dynamic selects, stored under `HKCU\Software\Tourne\Desktop\Values\<prefix>` as `::wh_select_option::<setting>::<value>` = label) and shows those settings as dropdowns. `switcher.theme.style` is one.
- 1807619: `--import` also takes clock, trayFolders, trayIcons, textColor and menu settings (everything `IsFullName` knows).
- Tourne's `%APPDATA%\Tourne\Desktop\tourne-gui.conf`: added the b53 values it was missing (clock, tray, menus, text colour, `switcher.theme.style = tourneStyle`), keys already there left alone; `clock.color.primary = #a95d5d` (red), `clock.color.secondary = #64aa89` (green), swapped from b53 as asked. Backup: `tourne-gui.conf.before-b53-fill`.
- Swapped in 1807619 (old exe `TourneDesktop.exe.prev-f206e63`); log clean.

State: not yet seen by Tourne. System-wide text colour is b53's textColor part: it hooks into explorer, chrome and the rest, so it only runs in Windhawk, and b53 is disabled. The values are now in tourne-gui.conf, but nothing in Tourne Desktop applies them. Icon Redirect is in the same situation.

Next:
- Tourne: hover AlienFan in the overflow, check the clock, open Studio > Switcher > style.
- Decide on the hook parts (text colour, icon redirect, menu tweaks): a hooks-only suite build in Windhawk beside Tourne Desktop (its own taskbar, switcher, system icons and clock off), or Tourne Desktop injecting them itself (a bigger job).

Learned:
- Some tray apps fill in their tooltip only when told of the pointer; any tray of ours must send WM_MOUSEMOVE on hover, including the overflow.
- Windhawk dynamic select options live as values in the part's own store, so the Studio can read them from the registry.
