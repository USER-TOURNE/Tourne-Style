# Tray folders edited in the hidden icons flyout

When: 2026-10-06, about 21:05 to 21:45, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at ab32b76).

Why: Tourne asked whether icons can be dragged into and out of tray folders, whether stacking one icon on another makes a folder, and whether folders can be renamed. None of it existed (folders came only from the `trayFolders.folders[...]` settings); Tourne said yes to building it.

What changed:
- 4099eb0 (system-icons-host.cpp): in the hidden icons flyout, drop an icon on a folder tile to put it in; drop it on the open folder's name (or anywhere above its icons) to take it out; drop it on the middle of another icon for a new folder of the two, named at once in a box over the title (Enter or a click away keeps it, Escape doesn't). Right click a folder tile, or the open folder's name: Rename, Glyph, Delete. The flyout's own menu has "Folders as in the settings".
- The layout is the settings' until changed, then `HKCU\Software\TourneTray\TrayFolders` (REG_MULTI_SZ: `v1`, `folder:name<TAB>glyph`, `match:`, `pin:`, `loose:`), which every build reads. Hand placements (`pin:` by program file name, `loose:`) win over patterns. Same-named folders merge and repeated patterns are dropped, which tidies Tourne's b53 list (folders 4 and 5 were one name; folder 4 repeated folder 2's patterns).
- Swapped in 4099eb0; clean start.

State: builds and runs; the drag, stack and rename paths are not yet tried by hand (nobody has clicked them).

Next:
- Tourne: try each gesture in the overflow; say if the stack zone (the middle half of an icon) feels too small or too big.
- Still open from before: the hook parts decision (text colour, Icon Redirect), checking AlienFan's tooltip, the clock and the switcher dropdown.

Learned:
- A folder placement by program name has to beat the patterns, or a broad pattern in an earlier folder takes the icon straight back.
