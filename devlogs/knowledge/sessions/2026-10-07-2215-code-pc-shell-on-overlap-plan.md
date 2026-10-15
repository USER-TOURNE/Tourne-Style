# Shell mode on again; plan to stop mods overlapping

When: 2026-10-07, about 22:15, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne asked for shell mode back on while Explorer's taskbar crashes, and for one owner per option: Explorer theming from b53 in Windhawk, everything else from Tourne Desktop, Tweaks as its own Windhawk mod, StartAllBack gone.

What changed:
- `%APPDATA%\Tourne\Desktop\tourne-gui.conf`: shell.enabled, shell.windowsApps, shell.desktop, switcher.enabled, systemIcons.enabled set true (they had been set false at 21:02). Backup: `tourne-gui.conf.before-shell-on`.
- Through `Invoke-Real`: HKCU Winlogon `Shell` = `"%LOCALAPPDATA%\Tourne\Desktop\bin\TourneDesktop.exe" --shell`, `HKCU\Software\Tourne\Desktop\ShellWindowsApps` = 1. ShellTurnedOff wasn't set. Takes effect at the next sign-in.

What was found:
- StartAllBack 3.9.27 is already `Disabled=1` for the user but its DLL still loads into Explorer. Tourne Desktop has its own Start menu (`taskbar.startMenu.enabled = 1`). `Bouquet SAB.msstyles` is already in `%LOCALAPPDATA%\Tourne\Styles`. Tourne Desktop's `taskbar.orb` points into `%LOCALAPPDATA%\StartAllBack\Orbs`.
- The Tweaks part already exists as a standalone mod (`tourne-tweaks.wh.cpp`, id `tourne-tweaks`). Every Tweaks group in b53 is `use = 0`.
- Overlaps: b53 classicMenus = eradicate-immersive-menus; dark-menus and classic-menus fight b53's themed menus; win11-accent-border vs Tourne Desktop's borders; local@taskbar-thumbnail-hide-icon only matters for Explorer's taskbar.

Next (proposed to Tourne, waiting for "go" and their UAC approval): turn off eradicate-immersive-menus, dark-menus, classic-menus, win11-accent-border and local@taskbar-thumbnail-hide-icon; install tourne-tweaks and turn b53's tweaks part off; copy the orbs into Tourne's folder and repoint `taskbar.orb`; export `HKCU\Software\StartIsBack`, then uninstall StartAllBack.
