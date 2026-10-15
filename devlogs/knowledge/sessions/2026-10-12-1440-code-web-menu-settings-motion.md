# 2026-10-12 14:40, Claude Code (web), Tourne Menu settings window and motion wiring

**Done**
- Tourne Menu settings window, laid out like LiteMenu's configurator (desktop/host/menu-editor.cpp, 391329b): Menu tab (the rows as a tree; Add item / submenu / separator, Remove, Move up / down, drag into submenus, F2 rename; Properties: type, display name, command with "..." pickers, parameters), Settings tab (Activation: on, mouse button, bar's empty space, hotkey as Ctrl/Alt/Shift/Win + key; Look note), Copy LiteMenu's menu, OK/Cancel/Apply. Writes taskbar.menu.* to tourne-gui.conf. Opens from the menu's settings item, Studio's one "Menu items" row, and `TourneDesktop.exe --menu-settings`. Wine-tested with a harness on Tourne's real config.xml.
- LiteMenu's config: %LOCALAPPDATA%\Packages\RedBaronDevelopment.PopEx_36fx7gy0frrbc\LocalState\config.xml (copy staged; its WindowsApps folder can't be read and its window didn't open remotely, so the layout follows the config's fields).
- Animations: audit (only bar, popups, switcher, border, borderangle were read) then Tourne said fix all and wire the rest. Fixed live reload of motion lines, popin style for menus, Off stopping spin/CSS fades, the bar's pause-then-jump. Wired start, flyouts, previews, notifications, plugin panels, floating pager, pager mark, new-screen bars, tiling moves, border fade/glow/shadow timing, glow turn, Windows' own window animation on/off. Unused leaves are labelled in Studio. DEVLOG has the list.

**Waiting on Tourne**
- Swap Tourne Desktop to the pushed commit (it builds on the PC; this server can't link the exe: #embed). Then open the menu's settings, check the tree, press OK once to copy LiteMenu's menu in.
- Try a Motion preset (Hyprland, Lively) and say what looks wrong.

**Notes**
- Build here: modules check with a scratch script (expands @inline like Build-TourneDesktop.ps1 and runs g++ -fsyntax-only); plugin-runtime.cpp uses #embed, which this server's mingw g++ 13 / clang 18 can't compile, so no exe here.
- Not done: Start's cascading submenus and preview fade-out don't animate yet; shadows on fading flyouts may not follow the alpha.
