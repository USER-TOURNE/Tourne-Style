# One owner per job: menus, b53 read live, borders, the audit

When: 2026-10-07, 21:30 to 22:00, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne wants one look everywhere and one place to control it: Explorer theming from b53 in Windhawk, everything else from Tourne Desktop, Tweaks as its own mod, StartAllBack gone. They sent screenshots of the target menus (Explorer, LiteMenu, a tray icon's), Task Manager, File Explorer, the old StartAllBack Start menu with green text, and Tourne Desktop's desktop right-click (icons kept, grey text).

What changed:
- 10f9886: `taskbar.menuLook = windows` (menu-style.cpp: Windows' menu colours and font, Text Color's green, measured from the screenshots); the suite layer in host.cpp (b53's textColor, menuNoIcons, menuNoDuplicates, iconRedirect, classicMenus read live from HKLM, read only, over the config); `accentInactive` border colour word (borders.cpp).
- Swapped into the running shell 21:50 (shell mode was already back on since 21:34): clean start.
- tourne-gui.conf: `taskbar.menuLook = windows`, `switcher.borders.activeColor = accent`, `inactiveColor = accentInactive` (backup `.before-menus-borders`). Reloaded live.
- `knowledge/consistency-audit-2026-10-07.md`: every inconsistency, its cause and fix.
- `tools/windhawk/Set-OneOwner.ps1` (admin; `-WhatIf` checked): b53 Tweaks off, Tourne Desktop's icon rows into b53, win11-accent-border and taskbar-thumbnail-hide-icon off; `-DesktopInTourne` lets b53 into TourneDesktop.exe (decision A). Not run yet.

Found:
- The import skips `.enabled`, so Tourne Desktop never knew Text Color and Menu No Icons were on: the Start menu text wasn't green and menus kept icons.
- In shell mode the desktop (icons and right-click) is a shell view inside TourneDesktop.exe, where b53 and the explorer-only mods don't run.
- Windhawk applies a mod's settings when its `SettingsChangeTime` changes, so the Studio can write Windhawk settings through one UAC prompt per save.
- `ColorPrevalence = 1` (accent on title bars), which win11-accent-border says conflicts with it.

Next: Tourne answers decision A and LiteMenu; then run Set-OneOwner.ps1 as admin, the StartAllBack uninstall (orbs first), tourne-tweaks installed in Windhawk, and the Studio writing Windhawk settings (audit item 12).

Later (22:00 to 22:10):
- 385abf2, live 22:02: the desktop shows only what Explorer's would (1 icon, Null.bin, instead of 15); the Windows menu look keeps a visible border.
- tourne-gui.conf: Windows Security button icon row; `taskbar.orb` now in `%LOCALAPPDATA%\Tourne\Orbs` (all orbs and StartAllBack's styles copied there).
- Windows' menu text set to `#64AA89` with menu-colors.ps1 (so classic menus inside TourneDesktop.exe, like the desktop's right-click, are green).
- Refused by Claude Code's safety system: running Set-OneOwner.ps1 elevated and the StartAllBack uninstall. One elevated pwsh was already requested (pid 868) pointing at a script that was never written; it couldn't be checked afterwards. Tourne runs those steps.
- Open: search magnifiers (audit 18), LiteMenu (13), Studio writing Windhawk settings (12), tourne-tweaks installed in Windhawk (10).

22:10 to 22:30:
- 6d6a586: tray menus framed like the taskbar's; `shell.marquee.*` (desktop selection fade done in Tourne Desktop, set to Tourne's mod values); Start's `appsFolderIcon` / `folderIcon`; Set-OneOwner keeps Theme Gaps in TourneDesktop.exe and no longer adds desktop-marquee-fade there.
- 10cdac7: menus taller than the screen split into framed columns (the every-tray-icon list scrolled, hiding Windows' arrows under the frame).
- 3c66040: submenu arrows are the tray overflow's chevron; Windows' arrow clipped over the whole item; Windows Security's own icon hidden by program name (it showed beside the stand-in).
- tourne-gui.conf: `taskbar.menuLook = theme` (Tourne wanted the framed look, without icons); borders matched to win11-accent-border (glow and shadow none, offset -1, width 1, no fade; backup `.before-accent-borders`).
- LiteMenu (PopEx): crashes on Explorer restart, doesn't show on Tourne's desktop unless "all desktops". Its menu is `%LOCALAPPDATA%\Packages\RedBaronDevelopment.PopEx_36fx7gy0frrbc\LocalState\config.xml` (MenuItem type 0 submenu, 1 item with Command/Parameters/IsShellApp, 2 separator; `<lock-computer>` etc.; ActivationButton Middle, DesktopOnly).
- Proposed to Tourne, waiting for the order: b54 (hook parts only, settings read from Tourne Desktop's config, Studio pages, single-icon swaps), the LiteMenu port, Studio pages for third-party Explorer mods (admin per save), a Tourne Desktop icon (stamped in after linking; design to pick).

22:30 to 22:40:
- dd31d0a, live 22:35: window shadows drawn as the window's shape blurred (Gaussian edge), a wide soft layer plus a tight one at half the offset, outside the border only, float coverage and a 4 x 4 dither on alpha (no rings on dark wallpapers). Not looked at on screen by me; Tourne to judge.
- Studio > Motion: chips under the curve (Off, Subtle, Smooth, Snappy, Lively, Springy, Hyprland) write a whole set of `motion.*` lines as pending changes. Each writes global, bar, border, borderangle (so it beats the identity's lines) and `motion.enabled = true` (Tourne's conf had it false: no animations at all).
- tourne-gui.conf: `switcher.borders.shadow = all`, `shadowY = 3` (blur 12, opacity 35 kept; backup `.before-soft-shadow`).
