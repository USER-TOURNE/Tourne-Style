# Show desktop works in shell mode; desktop menu without icons

When: 2026-10-08, about 00:30, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: "Show the desktop" (the bar's menu and its far-end button) sent Win+D, which nothing answers in shell mode. The desktop's right-click menu kept its icons although Menu No Icons is on.

What changed:
- taskbar-host.cpp: `ToggleShowDesktop` minimizes this desktop's task windows (frontmost first, remembered) and brings them back on the next press, frontmost ending in front. Used by the menu item and the show desktop button.
- desktop/host/desktop-host.cpp: the desktop's view (SHELLDLL_DefView) is subclassed; on WM_INITMENUPOPUP, after the view fills the menu, `StripMenuIcons` clears item bitmaps and checkmark bitmaps when `menuNoIcons.enabled` is set. That menu is the shell's own, shown by the view in TourneDesktop.exe, which b53's Exclude list keeps Menu No Icons out of, and menu-style.cpp leaves foreign menus' icons alone.

Found:
- Tourne turned `switcher.enabled` and `systemIcons.enabled` off in the Studio, and the empty boxes next to the Null orb went away. Which of the two makes them isn't known yet. No plugins are installed, so they aren't plugin bar widgets; the taskbar lists only titled, non-tool windows.
- LiteMenu (PopEx) in shell mode: its DesktopOnly check doesn't see Tourne's desktop as the desktop (found 2026-10-07). Options: its "all desktops" setting, or the LiteMenu port proposed before (Tourne's own menu built from PopEx's config.xml). Waiting on Tourne.

Check: right-click the desktop (no icons); Show the desktop from the clock's menu and the bar's end button, twice.
