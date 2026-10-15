# 2026-10-14 03:00, Claude Code on the PC: window rule "keep" for Discord and Spotify

- Asked: Discord (Vencord resets its size) and Spotify should always open at their current place and size on the 2nd monitor.
- Done: `keep [S]` window rule (window-rules.cpp), editor "Keep it there" and "Where it is now" (desktop/host/binds-editor.cpp). Rules written to the user's tourne-gui.conf (lines 63 to 65, backup `.before-keep-rules`): `float, monitor 3, move 4 450, size 1071 639, keep, exe:Discord.exe, title:*Discord` (title keeps the updater splash out) and `float, monitor 3, move 6 1094, size 1068 785, keep, exe:Spotify.exe`. Frames are DWM frames in physical px, move is from screen 3's work area (1920,-691, 1080x1920).
- Screens are numbered left to right by the Switcher: 1 is the left portrait one (-860), 2 the main, 3 the right portrait.
- Not verified yet: a real Discord/Spotify start with the swapped build (look for "rules: kept in place" in desktop.log).
- Still open from before: tray-click bug (parked), Table D3D/mouse-hook proposals (need approval), remaining hthienloc plugins.
