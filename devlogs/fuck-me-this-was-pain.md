# Fuck me, this was pain

The TL;DR of everything that went wrong. The bugs that cost the most, in one line each, and what actually fixed them.

| What happened | What it actually was | The fix |
|---|---|---|
| Target FPS 144 ran at 64. So did 90, 120 and 240. | `GetTickCount64` moves every 15.625 ms. 1 / 15.625 ms = 64. | `QueryPerformanceCounter` and floating-point intervals. 141.7 FPS. |
| Every colour setting's help said "Format is" and stopped. For months. | YAML ends an unquoted value at ` #`. | Quote anything with a `#` in it. |
| Custom media icons never worked, in any version. | The UI thread never initialised COM, so WIC never loaded. | `CoInitializeEx` on every thread that touches COM. (Bit me again later: the pager's previews showed a flat colour for the same reason.) |
| Dragging the visualizer felt like dragging through mud. Four releases. | Cursor-chasing a renderer is always going to lag. | Stopped chasing: Keyboard Move, one exact step per key. |
| Every number setting in the suite's switcher read 0. | The build only prefixes setting names passed as literals to known functions. A new helper wasn't on the list. | Add every helper to the list. The test harness hid it because it ignores prefixes. |
| Windhawk refused a whole settings block: "duplicated key". | A PowerShell here-string dropped its last newline and glued two settings together. | End inserted YAML with an empty line, and scan builds for it. |
| Two copies of every tray icon after a reboot. | The single-instance mutex was abandoned by a thread that exited, and the second launch ran anyway. | An unowned mutex held as a handle. |
| A held Tab showed my Alt+Tab *and* Windows'. | The keyboard hook lived on the drawing thread and timed out. | Low-level hooks get a thread of their own. Learned it again with menus. |
| Changing spatial sound cut the music. | Setting the format that was already active. | Audio probes are read-only, forever. |
| My screenshots showed StartAllBack's acrylic as copper. | My own capture path, not the screen. | Only real screenshots count as ground truth. |
| Explorer crashed whenever I extracted an archive. A week. Four test builds. | The hidden Windows 11 XAML taskbar built a progress bar whose template asks for a class WinUI 2 can't make by name. | Don't let app progress reach the hidden taskbar. My bar still shows it. |
| A System Restore took the newest settings with it. | Something unrelated crashed, and restore rolled everything back. | Everything lives in git now. |
| A build didn't compile that had passed every check. | `Wh_Log` is a function in the editor's check mode and a macro in a real build. | Syntax-check in real compile mode. |
| A tester's build: "Failed to parse settings". | An empty list in the settings block. Windhawk refuses the whole block. | Every list gets one empty example row. |
| First sign-in as the shell: a black screen and two screens flashing. | A dead Windhawk entry claimed the taskbar was on elsewhere, and nothing handed Windows the wallpaper. | Ignore entries with no version; the desktop paints the wallpaper itself. |
| A full restart turned shell mode off. | Windows ends the shell twice while shutting down. The watchdog saw two crashes. | Shutdown codes stop the watchdog. |
| Swapping in a build killed the shell for the day. | The swap's kill counted as a crash, and the new build crashed once. Two crashes. | Planned restarts use exit code 3, which counts as nothing. |
| Tourne Desktop died a second after starting. | A program path with no backslash in it. | Read forward slashes as backslashes, and catch it. Plus `crash.txt` with every frame named. |
| Handy's tray icon ignored clicks. | Tauri asks where its icon is before every click and drops the click if that fails. Behind my tray it always failed. | My tray answers with a box at the pointer. |
| Windhawk's own window stopped opening. | Store app lookups in every program as it starts, WebView2 included. | Do the lookups once, when the pictures are made. |
| Half the ink pack didn't apply. | A fixed list of 128 replacements per program. The pack sets hundreds. | A list that grows. |
| Video thumbnails covered in a green film. | Windows lays a see-through film frame over videos, and the pack drew a solid one. | Recolour Windows' own frames and keep the middle clear. |
| Tourne Desktop wouldn't link with the visualizer in it. | `-lwindowsapp` made the linker leave kernel32 out. | Skip it; `-lruntimeobject` covers the rest. |
| The first 2.0 beta's shaders failed on every Windows PC. | My test compiler accepts `pass` as a name; Windows' doesn't. | A lint pass for fxc's reserved words before every build. |
| Test notifications kept coming from a plugin that was off. | The test runner's scratch folder kept every plugin it had ever run. | It removes what it wasn't asked to run. |
| The capture bar vanished the moment it was clicked. | The hook's idea of the pointer didn't match the screen's. | Ask Windows where the cursor is, on the capture thread. |
| The GMK87's screen and settings did nothing over 2.4 GHz. | The dongle only passes VIA. | Lighting over VIA, the rest by cable, and the panel says so. |
| Tourne'Table 2.1 was 64-bit only. Again. | 2.0 kept the line the catalog maintainer asked me to remove in September. | Removed. Compiles for both. |
