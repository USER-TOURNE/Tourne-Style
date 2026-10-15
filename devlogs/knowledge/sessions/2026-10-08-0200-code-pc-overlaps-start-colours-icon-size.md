# Windhawk overlaps off; Start menu colours and search icon; desktop icon size kept

When: 2026-10-08, about 00:15 to 02:00, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Found:
- The Tourne Menu swap (04a70f6) crashed once at 00:17:01 (null call while "app styles (inject)" started). Not from the menu: the same crash is in desktop.log at 22:31 on 10-07. The crash guard left app styles off and the restart stayed up. The injector crash is still open.
- Menus had a double frame and opened out of place: b53's Exclude had lost TourneDesktop.exe (it was `vgc.exe` only), and classic-menus and dark-menus (Include `*`) loaded into Tourne Desktop too, so Tourne's own menus were styled three more times.
- Start text was white: b53's Text Color part is off (`textColor.enabled = 0`), and Start only took Text Color's green when it's on. Hot text never took it.
- Desktop icons went back to medium at every shell restart: the hosted view never kept its size.
- Screenshot notifications missing: System Icons (which draws notification pop-ups in shell mode) is off; Tourne turned it off with the switcher to lose the boxes next to the Null orb.

What changed:
- `tools/windhawk/Set-NoMenuOverlaps.ps1`, run elevated (Tourne approved UAC) at 00:21: b53 Exclude `vgc.exe|TourneDesktop.exe`; classic-menus, dark-menus, eradicate-immersive-menus, desktop-marquee-fade, local@taskbar-thumbnail-hide-icon off; Windhawk restarted. Backup `tools/windhawk/mods-before-no-menu-overlaps-20261008-002105.reg` (PC only). Tourne Desktop now loads only windhawk.dll.
- start-menu.cpp: `startMenu.textColor`, `startMenu.hotTextColor` (hot follows text when only that's set), `startMenu.searchIcon` (the box's magnifier and the "See more results" line). Set to #64aa89, #64aa89 and Bouquet 11's Taskbar Icons\SEARCH.ico. Backup `tourne-gui.conf.before-start-colours`.
- desktop/host/desktop-host.cpp: the icon size is kept in `HKCU\Software\Tourne\Desktop\DesktopIconSize` (first time from Explorer's Bags\1\Desktop IconSize), applied when the view is made, saved after its menu's command, after Ctrl + wheel, and at shutdown. Set to 32 (small).

Open: the boxes next to the Null orb (switcher or System Icons; with b53 out of Tourne Desktop they may be gone, to test by turning System Icons back on); the app styles injector crash; side-monitor bars 346 px below their screens; ghost frames in previews; menu placement to recheck now the overlaps are off.

Later (00:30 to 00:45): the desktop's right-click menu in Tourne's look (Tourne picked this over letting b53 back into Tourne Desktop).
- d646baa: Tourne's desktop window passes menu messages on to its view (the view's menu is owned by it).
- `desktop-menu.cpp` (inlined after tourne-menu.cpp) and desktop-host.cpp, `shell.desktopMenu` (on in Tourne's config, backup `tourne-gui.conf.before-desktop-menu`): a right-click on the desktop posts its state and the selected items (or the empty space) to a bar, which shows the shell's own menu for them with View (sizes, auto arrange, grid, show icons), Sort by and Refresh first, in RunMenu's look with Menu No Icons, and runs the command. View, sort, refresh and rename go back to the desktop window as posted commands. Shift + right-click keeps Windows' menu. "Show desktop icons" off is kept in HKCU\Software\Tourne\Desktop\HideDesktopIcons.
- Check: right-click empty space and an icon; View > sizes; New > Folder; Rename.
