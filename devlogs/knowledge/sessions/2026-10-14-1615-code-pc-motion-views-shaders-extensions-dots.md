# 2026-10-14 16:15, Claude Code on the PC: animated desktops and windows, Views page, shaders, extensions, dots

- Asked (Tourne): animated desktop switches, more window animations, then the plan's Studio Views page, shaders, extensions grid, dots; keep usage low; one Claude on this PC at a time.
- Done, all swapped in (last 2a51776): desktop-slide.cpp (workspaces line slides desktops; Ctrl+Win+Left/Right bound through Tourne), window-motion.cpp (windowsIn popin/slide/gnomed/fade), Studio Views page, screen-effects.h (13 effects) in the glass and as the Screen shader (Desktop Duplication overlay), extensions.cpp (10 built in + added ones), dots.cpp (.tourne export/import, hyprland.conf import). Details in DEVLOG's last three sections.
- Tourne's settings now: motion.workspaces = 1, 4, easeOutQuint, slide; motion.windowsIn = 1, 3, easeOutBack, popin 85%; binds lines 66-68 Ctrl+Win+arrows (backup tourne-gui.conf.before-anim). Screen shader off.
- Checked on the PC: shots partway through the slide and the popin; Studio pages captured; shader shots of all three screens (rotations right); live overlay 7 s (click-through, ~144 fps while moving); extension and dots harnesses. Not seen by eye yet: the screen shader live, a real import from the Studio dialog.
- Learned: run test programs from WMI with Win32_ProcessStartup ShowWindow=0 (visible consoles get tiled and shuffle Tourne's windows); DWMWA_CLOAK on another process's window is access denied but DWMWA_TRANSITIONS_FORCEDISABLED works, and a layered alpha-0 window's DWM thumbnail still draws; Desktop Duplication rotation 2 samples (y, w - x), 4 (h - y, x).
- Open: tray-click / desktop-drag bug (clip in DEVLOG), new windows tiled for a moment (wait ~300 ms and float consoles), Table D3D/mouse-hook (needs approval), remaining hthienloc plugins.

## Menus and wallpaper picker (80030b8, swapped in 18:04)

- menu-style.cpp: TrackMenuOnScreen sets SPI_SETMENUSHOWDELAY to 0 while our menu is open and puts it back after; only the top menu animates, so submenus open instantly.
- Wallpaper picker: recent wallpapers are sent to the desktop by path (WM_COPYDATA 'TDWP'), not by list index. Index lookup could hit a reordered list and set the wrong picture.
- Pointer Afterimage Motion Blur (Windhawk) runs in its own process with a click-through window and doesn't hook our menus, so it's unchanged.
- Waiting on the user: does any hot-state choppiness remain?

## Drag peek from edges and tile gaps (12d0945, swapped in 18:16)

- The bar-only drag peek never fired in the user's test: they held drags at screen edges and between tiles. Now an EVENT_SYSTEM_CAPTURESTART hook starts a 50 ms check while a button is held; if a SysDragImage window is visible and the cursor rests 500 ms at a screen edge or over our Progman, PeekDesktop runs for that screen. Log line: 'Drag held ... peeking the desktop'.
- Bar drop target now returns the source's effect in DragEnter/DragOver (Drop still returns none), so the cursor no longer swaps to the stock no-drop one.
- Unverified: whether Explorer drags on this PC show SysDragImage (if no log line appears, that check is the suspect).

## Wallpaper picks draw the chosen file (71e588c)

- After a pick the desktop read the picture back from Windows (ComposeWallpaper per screen); WM_SETTINGCHANGE could load first with the old picture and then kWallpaperSet skipped its reload. Now WallpaperThread posts the set path (heap wstring in lParam), LoadWallpaper(false, chosen) draws it on every screen, and change notices while g_wallJobs > 0 are ignored.
- User's tests at 18:13-18:14 were on the build before the drag watcher (12d0945, 18:16), so the drag step-aside is still untested.
- Open: menus still hitch slightly when the cursor moves fast across items (user, 18:13); not yet looked at.

## Drags make room by sliding (b4489b8)

- User wants windows to slide back 100-300 px to open a drop space, never minimise. MakeRoom in taskbar-host.cpp: edge = windows within 48 px of that work-area edge pull back by taskbar.dragRoom (default 240); desktop gap = nearest-edge axis decides, the two sides pull back room/2 each from the cursor. Eased 170 ms via a 15 ms thread timer (SetWindowPos async), back on button release. Min window size 160. Log: 'room made beside N window(s)'.
- The bar hover peek (over the bar / show-desktop corner) still minimises, like Windows; ask if that should slide too.

## Bar flashing over full-screen video (00ad920)

- Log at 18:24:56: 'Bar behind a full-screen program' then all three bars 'Bar placed' (shown, from the hidden spot) every ~300 ms. Only ApplyHiding resets g_shown/g_underFullscreen silently and places all bars, so something kept calling ApplySettings. ApplyHiding now keeps a hiding bar hidden (and under) while FullscreenInFront. ApplySettings logs 'Settings applied: <why>'; the cause of the repeats is still unknown, so check that line next time it happens.

## Gap detection by drawn edges, settings, double-click (0391998)

- Edges fired (log 18:51) but gaps never did: between tiles WindowFromPoint hits a window's invisible resize border, not Progman. InGap now uses DWMWA_EXTENDED_FRAME_BOUNDS: nearest facing edges each side of the pointer within taskbar.dragGapPad (default 4) px, gap <= 120 px wide; windows pull back from the seam middle by drawn-edge delta. Rest check: within 6 px for taskbar.dragHold ms (default 250).
- Double-click on the bar's empty space (hit -1) = ToggleShowDesktop.

## Tile bounce, overflow delay (c18365a)

- Bounce: windows whose min size exceeds their tile (fit overlap/force) were re-animated toward the same tile on every TileAll. tiling.cpp now keeps g_refusedTiles (hwnd, tile) from PlaceFrame and TilePlace skips that tile until it changes.
- Overflow not answering after a swap restart: undiagnosed. desktop.log mixes days with no dates, so old 'System icons is on in the Windhawk suite' lines misled me. A SendMessageTimeout probe of the bar windows for 70 s after the c18365a restart saw them answer in under 130 ms. Need the time of the next occurrence.
- 'Settings applied: scale changed' fires when Chrome goes fullscreen (WM_DPICHANGED to the bar); that was the reset behind the fullscreen flash.

## Tiling waits for drop room; edge zone; Float default (7d72859)

- Room snapped back: TileEventProc retiles 150 ms after any show/hide (the drag image too). Taskbar sets named event Local\TourneDropRoom while room is open/closing (HoldTiling); tiling.cpp TileTimer re-arms while it's set.
- Edges: zone = dragGapPad + 2 px in from the screen edge (the main screen's sides meet other screens).
- switcher tiling.tooSmall default now float (user found Force bounced Notepad; they set Float themselves).

## Binds over oversized windows (67d81f5)

- User: Notepad++ blocks keybinds. It floats at a min size taller than the screen, and ForegroundIsFullscreen (tiling.cpp, used by binds.cpp) counted any covering window as a film/game. Now: exact screen rect, or covering with no WS_CAPTION. Not elevated (process handle opens). Unconfirmed by the user.

- 19:23: user confirmed 'it works' (binds over Notepad++, 67d81f5).

## Notifications, Test Notifications plugin, segment gaps (a15addb)

- Tiling check after placing uses the window rect less measured borders (DWM frame bounds lag): Notepad++ takes any size (tested 900x520, 400x300) yet was floated. App bars/taskbar fullscreen tests skip captioned oversized windows (81401c3). Still open: after the swap Notepad++ was put back at -167,-72 1125x1112 by something; not in keptPlaces. Suspect Places (rememberAll, tiling-layouts.cpp:181) or Notepad++ itself.
- Notifications: middle click clears (centre and pop-up); right click menu Clear / Clear all from app / Copy text / Open app / Turn off from app / Notification settings (system-icons-host.cpp ShowNotifyMenu). Phone toasts tagged PhoneNoteKey (FNV of app, title, subtitle, message, phone date; phone-link.cpp), clears kept in HKCU\Software\Tourne\Phone ClearedNotes (400), phone-ancs skips them on resend.
- Test Notifications plugin (bundled tourne/test-notifications): PowerShell -File script in its data dir posts toasts as AUMID Tourne.TestNotifications and ntfy pushes; right-click panel. User's topic seeded in %LOCALAPPDATA%\Tourne\PluginData\tourne\test-notifications\choices.json.
- Segment gaps drawn alpha 1 so the bar takes clicks; taskbar.emptyDoubleClick (Tourne Menu actions via TmAction, default show-desktop).

## Discord/Spotify small squares (2f3dc06)

- Rules (float, monitor 3, move, size, keep) opened Discord and Spotify as tiny squares. Same DWM frame lag as Notepad++: each step read FrameOf right after the previous moved it. FrameOf (tiling.cpp) now checks the DWM frame against GetWindowRect; insets outside 0..40 px mean stale, so the last good insets per hwnd (or the DPI size frame) are used. Mutexed (switcher and binds threads). Unconfirmed; may also fix the Notepad++ oversize. If Discord still opens small, check whether the rule's title:*Discord matches at show time.

## Double-click fix (c8e868a)

- Two bugs: HitTest returns -2 for empty space (-1 is the orb), and the user's YASB theme draws through CssRedraw (css-bar.cpp), whose alpha-0 gaps passed clicks to Progman. CssRedraw now writes alpha 1 there. Verified with WindowFromPoint: the gaps at y 1062 hit TourneTaskbarTest_ now.

## Phone pane middle click (d705aa0)

- start-menu.cpp: WM_MBUTTONUP in the Start window and the apart phone window -> PhoneMiddleClick: notes cleared via PhoneClearNotes (remembered), conversations hidden via PhoneClearConvo (ClearedConvos key|lastWhen, back with a newer text). Convo right-click has Clear. Clear all now runs ClearApp before HideAll so tags are remembered.

## Middle click on empty space; Table strip clicks (bc930ac)

- taskbar.emptyMiddleClick / emptyMiddleDoubleClick (default none), TmAction list; single waits GetDoubleClickTime when both set (kTimerEmptyMiddle 63); WM_MBUTTONDBLCLK handled, g_swallowMiddleUp. Tourne Menu on middle click keeps priority.
- Regression fixed: alpha-1 gaps ate right clicks for Tourne'Table's DesktopAudioVisMedia strip (under the bar). BarPassThrough leaves own-process windows below the bar see-through. Verified via WindowFromPoint (1610,1062 -> DesktopAudioVisMedia; 1300 -> bar).
- User wish (not done): double-click variants for most hotkeys/binds.

## Table visualizer right click (7bff1ca)

- The visualizer is DesktopAudioVisOverlay (full-screen, WS_EX_TRANSPARENT); its menu comes from a WH_MOUSE_LL hook that checks VizDesktopUnderPoint, which only accepted Progman/WorkerW/Shell_TrayWnd. Added TourneTaskbar* (when hostTaskbar). Tourne'Table source change, made without prior discussion because it fixes my regression; told the user. CHANGELOG beta-fix line added.

## Typing Sounds settings (356acac)

- User: Table right click works now. Typing Sounds style made no difference: prepare() only ran on turn-on; added onConfigChanged. Styles made distinct (mechanical click+clack, typewriter strike+overtone, soft muffled noise), files renamed -b so old cache isn't reused. Not listened to (no live audio probes).

## Live plugin settings (ca9cc88)

- User wants all plugins' settings live. The runtime already does it (plugin-runtime.cpp ApplySettings: onConfigChanged entries told, the rest restarted). My earlier 'only on turn-on' explanation for Typing Sounds was likely wrong; the styles probably just sounded alike. Added a log line per entry told/restarted ('settings changed, ...') to verify next time.

## Capture: Print Screen (5dd5b02)

- New host part desktop/host/capture.cpp (Studio page Capture, capture.*): own thread with WH_KEYBOARD_LL taking plain VK_SNAPSHOT (modifiers pass). Tap = screen under pointer; double tap (300 ms) = window picker overlay (alpha-1 layer, outline, click/Esc); hold (600 ms) = TrackPopupMenu of screens / every screen / a window, then ffmpeg gdigrab region to MP4 (NVENC if listed, else x264), PrtSc or the red dot stops ('q' on stdin, finished on a side thread). Pictures: CF_DIB + PNG format + file in Pictures\Screenshots; recordings: CF_HDROP + Videos\Captures. Started OK (log 23:07:40). Untested by the user; no audio in recordings yet; window recordings are the window's rect at the start (don't follow moves).

## Print Screen through Screen Toolkit; record bar; toolkit panel (ac64dc0)

- User tested 5dd5b02 at 23:11: window picture OK; first native recording "didn't come out" (4 s), second OK.
- User's toolkit settings: G:\Nvidia HQ Recordings\Captures for both paths, sound + mic (fifine), HEVC NVENC, ddagrab, 144 fps, skip confirmation, copy to clipboard.
- Hold now opens a themed top-centre bar (capture.cpp RecordBar: screens, All screens, Window, close; keys 1-9/A/W/arrows/Enter/Esc) instead of TrackPopupMenu. Hint and red dot themed (FlyoutPalette, else Theme roles) and rounded.
- With Screen Toolkit's service running, StartRecording delegates: PluginMessage {"plugin","tourne/screen-toolkit:service","all","recordRect",x,y,w,h} on a side thread; service.luau recordRect -> startRecord(rect,"mp4"), so its settings apply and its finalizeRecord copies the file. New tourne.captureLink(state, screenshotDir) (only for tourne/screen-toolkit) -> CaptureToolkit: state drives the red dot and lets Print Screen / the dot send recordStop (also stops panel-started recordings). onExit sends "off" so the native ffmpeg path returns. Pictures use the toolkit's screenshot path when capture.pictures is empty.
- panel.luau standard mode: rows Tools / Markup / Record with short names; record row Area, Screen + MP4/GIF chip + sound + mic chips; footer hint for Print Screen. Bug fixed: onOpen sent setAudioIn false every open, overriding "Record Microphone".
- Untested by the user: the bar, delegation, the panel. Did not see screenshot 3 (the tray panel) myself; cleanup is my guess at "clean up a lot".

## Window recordings follow the window (9e81026)

- User's tap/double tap/record tests were on 5dd5b02 (log 23:10-23:11): working. Asked to "start" the remaining list; most of it was already in ac64dc0.
- Window recordings now use gdigrab `hwnd=0x...` (ffmpeg 9.0.2 at C:\Program Files\FFmpeg\bin, tested), capped at 60 fps, crop to even size; toolkit handler recordWindow, native path too. Caveats: client area only; GPU-drawn windows may come out black under GDI.
- Still open: "menu cursor hitch" (no details in my notes).

## Menu cursor hitch (c5d35e8)

- Cause: menu-style.cpp MenuWheelStart installed WH_MOUSE_LL (wheel scrolling) on the menu's own thread while a menu was open, so every mouse move waited for menu paints. Now a dedicated thread holds the LL hook (MenuWheelHookThread, stop event); a turn over a menu window is posted to it as WM_MOUSEWHEEL and MenuWheelFilterProc (MSGF_MENU) scrolls on the menu thread. Works with thread_local storage (tourne-menus): the hook thread sets its own g_menuWheelThread.
- Tourne Desktop swapped in. tourne-menus.wh.cpp regenerated as 1.0.2 but NOT installed: windhawk-cli mod install needs admin (compiled mods folder not writable). User to run it elevated.
- Not verified by feel; the user uses Pointer Afterimage Motion Blur.

## Tourne Convert (merged converter menu)

- User still felt slight hiccups after Tourne Menus 1.0.2; asked to merge Convertify + File Converter.
- Findings: File Converter = .NET SharpShell extension on * (mscoree, D:\Applications\File Converter\FileConverterExtension.DLL), loads the CLR into the menu host. Convertify = static verbs only, per-user in HKCU\Software\Classes\SystemFileAssociations\.<ext>\shell\Convertify. File Converter's CLI: FileConverter.exe --conversion-preset "<name>" <file> (seen in its Diagnostics log). Presets in %LOCALAPPDATA%\FileConverter\Settings.user.xml.
- tools/convert/Set-TourneConvert.ps1: per extension a static "Convert" flyout (TourneConvert): formats (File Converter preset where it has one, else Convertify), "Audio only" flyout for video, File Converter folders (Scale, Rotate, Tempo - Pitch), Tools (Convertify upscale/rembg/compress). Hides Convertify via LegacyDisable and blocks File Converter's CLSID in HKCU Shell Extensions\Blocked. -Undo, -DryRun. Applied via WMI pwsh: 114 types, Convertify hidden on 99, verified in the real hive.
- Engines are still Convertify.exe and FileConverter.exe (no own engine yet). Rerun after preset changes or Convertify updates.

## Input hitch hunt (34bb5f5)

- tools/menus/input-gap-probe.cpp (build static with -static; args: seconds log [csv]): raw-input sink logging gaps and every report. Real held-up input = gap then a burst of queued reports (slow hand movement also makes sparse reports; don't count those).
- Run 1 (before): raw input held up to 148 ms, 1,649 hits with a menu open vs 53 closed.
- Fixed: keystrokes overlay hooks lived on its D2D drawing thread (badge animation after a click held up all input) -> own pump-only thread; system-icons-host MenuMouseProc (tray/taskbar menus) on the menu thread -> own thread, closes via WM_CANCELMODE to the owner.
- Run 3 (after, csv analysis): 28 stalls of 15-56 ms in 88 s of menu sweeps, 2 in 20 s without. Remaining cause unknown; next: user disables Pointer Afterimage Motion Blur to test, else add owning process to the probe.
- Tourne Menus 1.0.2 installed by the user (admin). Tourne Convert applied (30a825b).

## Flyout slowness: per-pixel frame drawing (133bec9)

- User: hiccups also in Tourne Desktop's own menus; "it has to be the flyouts, noticeably slower than menus with none". Their menu look: Frame 1, FrameGlow 45, FrameStops gradient, Animation 0 (HKCU\Software\Tourne\Menus).
- menu-style.cpp FrameLine drew every frame/glow pixel with SetPixelV on the menu window DC (thousands of kernel calls per flyout, also per hovered item). Now each strip is computed in memory and drawn with one SetDIBitsToDevice. Tourne Menus 1.0.3 regenerated, needs the user's admin install; Tourne Desktop swapped.
- Also checked: taskbar RestInputProc (only while resting) and PreviewMouseProc (only with a preview up) are fine; switcher hook has its own thread.
- Seen in a screenshot: folders still show Convertify's own entry (Directory\shell, not covered by Set-TourneConvert).

## Flyouts filled ahead (0cacc78)

- User: hitch is ONLY on flyout items (video), still there with Pointer Afterimage off; pointed at "Desktop menu ready ahead". Shell submenus were filled by their extensions (HandleMenuMsg2 WM_INITMENUPOPUP) on every hover, on the menu thread.
- ShellMenu.filled set; FillShellSubmenus fills every submenu when the ready menu is made (desktop-menu.cpp MakeReadyMenu); WM_INITMENUPOPUP skips submenus already filled. Applies to Tourne Desktop's desktop menu only; Explorer's own menus still fill lazily.
- Open: File Converter's progress window too tall (user screenshot), not looked at yet. Folders still show Convertify.

## Submenus on click (e6ef4db), Capture settings (7b0255c)

- taskbar.menuFlyoutClick: while our menus are open the hover delay is 60 s (put back after), so flyouts open only on click/Right/Enter. Published as FlyoutClick to Tourne Menus 1.0.4 (needs the user's admin install). Off by default. User's call: flyouts "smoother but not smooth"; D3D drawing deferred.
- Capture page: encoder, codec (default HEVC), quality (CQ, default 23), resolution, method, fps to 240, sound + soundDevice (output endpoint ids), mic + micDevice (friendly names), after (none/av1/hevc/discord10/50/500/gif), afterCopy. Native Delegate sends recordWith with JSON; service.luau recOpts overrides cfg per recording (cleared when idle), afterRecording runs the extra ffmpeg (AV1/HEVC on NVENC, Discord fit by ffprobe duration, GIF) and copies it. RTX 5070: av1_nvenc tested OK.
- Defaults: mic off (user's toolkit had it on): user to switch on in Capture.

## Hover bug, desktop middle click, Remove Context Menu Items ported (20aa6c7)

- User: flyouts still open on hover with menuFlyoutClick on (tourne-gui.conf has it true, HKCU Menus FlyoutClick=1). A bare Win32 menu honours a 60 s MenuShowDelay (scratch hovertest: 1 menu at 60000, 2 at 0), so our path is missing it. Added Wh_Log "menu: hover delay %u while open" in TrackMenuOnScreen; waiting for a menu to be opened to read it.
- Desktop middle click switched desktops: bar's empty-space actions keyed on HitTest == -2, which is true for points outside the bar. OnBarItself (inside client + WindowFromPoint root is the bar) now gates middle, double middle and double left. Bars measured: 1920x38 main, fine.
- Remove Context Menu Items (Windhawk, settings in HKLM\SOFTWARE\Windhawk\Engine\Mods\remove-context-menu-items\Settings) ported to taskbar.menuHide (written via WMI to %APPDATA%\Tourne\Desktop\tourne-gui.conf, applied live): Move to OneDrive, Ask Copilot, Create with Designer, Edit with Clipchamp, Ask Microsoft 365 Copilot, Pin to Quick access, Include in library, Paste (greyed), Edit with Photos, Edit with Paint. New rule suffix " (greyed)". Display settings/Personalize left in the user's More list. Disabling the mod needs admin (user to run).

## Launcher (4867fb2)

- New part desktop/host/launcher.cpp (+ .h, Studio page "Launcher", launcher.enabled, launcher.hotkey default alt+space via RegisterHotKey). Own thread, themed popup (FlyoutPalette/theme roles, DWM round + border), EDIT for the line, rows drawn with GDI; apps from FOLDERID_AppsFolder (SIGDN_PARENTRELATIVEPARSING, launched as shell:AppsFolder\<that>), icons via IShellItemImageFactory, read on a worker at most once a minute.
- Plugin runtime now starts [[launcher_provider]] entries: global `launcher` (setResults, close, setQuery no-op); public LauncherProviders/Query/Results/Activate/SetListener/Glyph in plugin-runtime.h; answers stored per provider under g_lock, listener gets a posted message. Prefix "p " routes to one provider; include_in_global_search providers join plain searches under the apps (their "none" rows dropped). Empty line lists the providers' prefixes.
- Swapped; log "[launcher] on: alt+space opens it", game-launcher entry starts. UI untested by the user so far. Not done: a tourne:launcher action for binds/menus (LauncherToggle exists).

## Launcher actions, folder Compress (f8bc600); parked DMS branch reviewed

- tourne:launcher in binds, the Tourne Menu and the bar's empty-space click choices. Set-TourneConvert: folders get a Compress flyout (ZIP, 7Z, TAR.GZ) in place of Convertify's entry. Swapped in.
- Launcher indexes only the shell Apps folder (Start's list), no files.
- Reviewed origin/claude/parked-dms-ports (bee04fd) for Caffeine, Fullscreen Power Menu, Depthscape. Useful: plugin-more.inl ApiKeepAwake (SetThreadExecutionState per entry, small, lifts cleanly), plugin-panel placement "fullscreen" (blurred dimmed backdrop, for the power menu), brief-fullscreen-power-menu.txt (feature list). Caffeine and power menu were never started there; GPL/unlicensed so ideas only. Depthscape port is unchecked and overlaps our wallpaper-depth plugin (Noctalia port, same idea), so skip it. Nothing compiled on that branch.
- Tray: plugins with a [[widget]] already get a tray icon (taskbar-host PluginItemMenu, Shift+right-click: tray/overflow, change icon). Missing: a Hide choice (kPlaceHidden is already honoured by g_pluginAway) and icons for enabled plugins with no widget: activate-watermark, display-profiles, game-launcher.

## Tray icons for every plugin, Caffeine (a130a83, 783f045, 54b964c)

- Plugin tray icons: Shift+right-click menu gains Hide (kPlaceHidden; back via the bar menu's tray icons list).
- tourne.openLauncher(line): posts the "TourneDesktop.Launcher" message with a global atom of the line in wParam; launcher opens with it typed. Game Launcher ("g ") and Display Profiles ("dp ") got [[widget]] tray icons using it; Activate Watermark's tray icon hides/shows it (state "hidden", not kept over restarts). Versions bumped to 1.1.0 so PluginRefreshBundled rewrites installed copies (installed bundled ones only refresh when the version changes). Installed on the PC: game-launcher, keybind-guide, notes, screen-toolkit, typing-sounds.
- tourne.keepAwake lifted from the parked branch into plugin-system.inl (StopAwake in Stop). New bundled plugin tourne/caffeine (own code; DMS originals GPL): tray cup, click on/off, right-click panel 5 min to Infinite, settings screen/minutes/notify, IPC toggle/on/off. Not installed yet: the user installs it from Studio Extensions (it asks for power). pluginrun test: all new widgets start and click without errors.
- Next: Fullscreen Power Menu needs placement "fullscreen" from the parked plugin-panel.cpp.

## Power Menu (f51729b)

- plugin-panel: placement "fullscreen" lifted from the parked branch (BackdropProc + CreateFullscreen: a screenshot of the pointer's screen shrunk/grown for blur, dimmed, as a topmost backdrop; panel centred, no pin strip; a backdrop click or Esc closes). Overlay, camera, canvas and drag-out left on the parked branch.
- New bundled tourne/power-menu (own code; original unlicensed): tray icon, hotkey setting, six round tiles (hibernate off by default), arrows/Tab/Enter/1-6/Esc via capture_keys, confirm arms sign out/restart/shut down for 5 s, IPC toggle/open on the tray entry. pluginrun: starts clean; the full-screen look is untested until the user installs it from Studio Extensions.

## Start flyouts in the Tourne Menus look (e0656c1), toasts (27b723a), Control Center (4858923)

- startMenu.menuColors gains "tourne": StartMenuLook returns BarMenuLook (+ LookLikeWindowsMenus when menus look like Windows'). Start's own cascade windows (PaintCascade/LayoutCascade) now draw the frame (band, outer and inner FrameLine with gradient and glow), hover text, caps and the chevron when look.frame. Set to tourne in the user's conf via WMI (was visualStyle). Not seen on screen yet.
- Test toasts: Run-PluginTest's scratch folder (%TEMP%\tourne-pluginrun) kept an old test-notifications install that ran on every test; the script now removes plugins not named.
- Parked branch, whole-build ranking sent to the user: sample player (XAudio2) > dragSource files > overlay placement > camera/canvas > Alienware CC, Shelf, Dank Actions, Software Depot. Skip httpStream/Ephemera, Depthscape, parked typing-sounds (clashes; its packs maybe later).
- Foley sounds (user's Sounds.7z): foley library is MIT (usefoley.dev footer, github.com/eakbulut/foley), cues generated by it; miraclei files look like Pixabay, kept out of the repo.
- Control Center: tourne.barItems (other plugins' widgets: key, plugin, glyph name, text, tip, color role; Entry gains glyphName/colorRole) and tourne.barClick(key, left|right|middle). Bundled tourne/control-center: tray icon, attached panel of tiles (lit when the widget's colour is primary), hotkey, columns.

## UI sounds (f7f65e5)

- desktop/sounds/foley/*.wav (28 Foley cues, MIT, 16-bit 44.1 kHz stereo) built in as ui-sounds.inc by Build-TourneDesktop.ps1. New part desktop/host/ui-sounds.cpp (part "sounds", Studio page Sounds): sounds.enabled (off), volume (PCM scaled per volume, cached), plugins, menuOpen (pop), menuHover (none), launcher (swoosh). PlaySound SND_MEMORY|SND_ASYNC on its own thread (no stream open between sounds). Menus heard via SetWinEventHook out of context, own pid: MENUPOPUPSTART and OBJECT_FOCUS on #32768 (40 ms throttle). tourne.uiSound(name) for plugins; Caffeine (on/off), Power Menu (whoosh/tick/warning/press), Control Center (tap) use it. Log shows "[sounds] off". Not heard: user turns it on (no live audio probes from me). miraclei files not bundled (Pixabay licence unknown).

## Binds: combos, double press, hold; plugin hotkeys through Keys and rules (a894ee1, ceb1686)

- User: Esc+End couldn't be set or typed for Power Menu's hotkey; wanted plugin hotkeys on the tiling binds, plus double press and hold-for-time.
- binds.cpp: key field takes combos (escape&end; extra key names in the mods field count too), flags k (double press, binds.doubleMs default 300) and o / oNNN (held NNN ms, binds.holdMs default 500; let go sooner: the plain bind on that key, else the key replayed with SendKeys). Hook keeps g_keysDown; picks the binds naming the most held keys; hold timer kTimerBindHold on the switcher window (WM_APP_BINDHOLD/BINDTAP). New dispatcher "plugin" (atom of author/plugin:panel to TourneDesktopPlugins, registered "TourneDesktop.PluginPanel"). ReadBindLines appends HKCU\Software\Tourne\Desktop PluginBinds (REG_MULTI_SZ); "TourneDesktop.BindsReload" re-reads.
- plugin-runtime RegisterHotkeys: HotkeySpec parser ("esc+end", "ctrl+alt+78", suffix " x2" or " hold 800"); with switcher.binds.enabled every panel hotkey becomes a bind line (code:NN keys) in PluginBinds and the switchers are told; off, RegisterHotKey for simple ones only. Log: "power-menu:menu opens with Esc+End (through Keys and rules)", "binds: 82 read".
- Studio CaptureHotkey(what, rich): plugin hotkeys record keys held together (ends on the first key let go), then a menu: once, double press, held 0.3 to 2 s. HotkeyLabel shows ", double press" / ", held N s". Keys and rules editor: When-it-runs gains Double press and Held 0.3/0.5/0.8/1.2/2 s; Press keys records combos (escape&end).
- Not exercised by me (pressing keys would pop the Power Menu on the user's screen). The suite's .wh.cpp builds weren't regenerated.

## Panels scroll when they don't fit (1a6e65a)

- User: Caffeine's panel made small hid its buttons; "everything like this needs to be scrollable like Notes". plugin-panel Relayout: when the tree has no scroll node and Measure(tree).h is taller than the panel (not desktop widgets), it's placed inside a generated scroll node (PanelWindow::scrolled_), so the wheel scrolls the whole panel. Panels that fit lay out as before (flexGrow fills kept).

## Alienware Command Center (d14561b)

- User: Esc+End works; picked Alienware next. Caffeine "shows off even when on": asked where (unclear; Caffeine resets on every Tourne restart/swap).
- PC is an Alienware Aurora ACT1250; root\WMI AWCCWmiMethodFunction exists (Thermal_Information/Control, GameShiftStatus, GetFanSensors...). Reading an instance needs admin (Access denied as the user). AlienFan Control (alienfan-gui) runs elevated on the same interface.
- Bundled tourne/alienware-command-center from the parked branch's awcc.ps1 + service.luau (our code) plus new plugin.toml, widget.luau (mode glyph/tooltip, optional text), panel.luau (mode tiles, fan boost sliders, set-up/once buttons, AlienFan note, Lighting -> ms-settings:personalization-lighting). Admin route: one UAC to install a scheduled task \Tourne\AlienwareCommandCenter (S4U, Highest) running an admin-only copy in ProgramData\Tourne\AlienwareCommandCenter with request/result JSON in the plugin data folder. Panel open sends "peek" (never a UAC prompt). Poll default 60 s.
- Checked: awcc.ps1 -Op info as the user returns awcc:true, needsAdmin:true; pluginrun starts all 3 entries. Mode/fan changes untested: need the user's admin setup. Not installed for the user (Studio, Extensions).
- Bash tool heredocs here eat backslashes (\ -> \): write Python edits with the Write tool or use Edit.

## Capture keeps menus open, tray tip refresh, Alienware lighting (d8663cb, 018bcef); stopped

- Capture picker and record bar: WS_EX_NOACTIVATE, MA_NOACTIVATE, pointer and keys through a WH_MOUSE_LL (only while up) and the existing keyboard hook, so open menus/flyouts stay and can be picked. Not tried by the user yet.
- System Icons overflow: after a plugin icon click, plugin items re-read (g_pluginItemsRead = 0, timer 7) and its tip and pixels refreshed (the "Caffeine: off" while on).
- Alienware: AlienFan GUI runs from a Highest scheduled task too (same as our helper). Lighting via T-Troll's alienfx-cli (no admin; Aurora chassis device 0, APIv4, Front Ring + Alienhead): colours, Static/Breathe/Pulse/Spectrum/Rainbow, brightness (setdim). alienfxCli path written to the user's conf. Untested on the lights.
- Open: user asked for hover tooltips on buttons in plugin panels (icon-only buttons like Screen Toolkit's). User said stop at 16:16; nothing in progress.

## Capture key setting, bar fix attempt (aa0348b)

- User: holding the capture key then pressing the bar choice made the bar vanish at once (after the no-focus change); and no setting for the capture key. capture.hotkey (format hotkey, empty: Print Screen) parsed by ParseKey; hook matches g_keyVk/g_keyMods (atomics), release taken when the press was. kMsgPointer now uses GetCursorPos on the capture thread instead of the hook point (suspected mismatch making clicks land "outside" and close the bar). Unconfirmed: asked the user what exactly vanishes.
- 16:26 user confirmed: the bar no longer vanishes (the GetCursorPos fix was the cause).

## GMK87 keyboard (research only, nothing built)

- User wants the GMK87 (Zuoya, 2.4G) battery in device batteries, plus lighting, screen picture and settings from Tourne. Sent the vendor exes/DLLs (not run; static strings only: a generic Sonix/"GMMK" editor and Zuoya's Image Custom Tool, no battery command found) and the VIA JSONs.
- Dongle 320F:5088 (wired 5055). HID collections: mi_00 keyboard (feat 65), mi_01 VIA raw (FF60/61, 33), mi_02 col04 FF1C/92 (64 in/out: the gmk87 vendor protocol), col05 FFEF (64), col06 mouse, col02/03 system/consumer.
- VIA (2.4G V2 JSON): lighting channel 3, ids 1 brightness 0-9, 2 effect 0-18, 3 speed 0-4, 4 colour.
- gmk87-node (MIT, codedgar) + reference.py (BSD): 64-byte reports [0]=0x04, [1-2] checksum = sum(bytes 3..63) LE, [3] cmd, [4] len, [5-7] pos, [8..] data. 0x01/0x02 bracket, 0x03 unknown, 0x05 read config (12 x 4-byte pos), 0x06 write config, 0x21 image RGB565 240x135 (2 slots, 36 frames), 0x23 before upload. No battery command anywhere public.
- Read-only checks: no unsolicited input reports on mi_01/col04/col05 in 25 s; GetFeature on mi_00 refused. Asked the user: USB capture of the Zuoya tool (USBPcap) vs skip battery; and whether to start on lighting/picture/settings.

## GMK87 plugin (77a6918)

- User: leave battery, do the rest. Bundled tourne/gmk87: gmk87.ps1 (C# HID inside PowerShell) + panel/widget.
- Over the 2.4G dongle only VIA answers (mi_01 FF60): read brightness 9, effect 7, speed 1, hue 18 sat 240 (read-only check). The FF1C collection on the dongle (mi_02 col04) never answers 0x01, so the gmk87 protocol (pictures, clock, 48-byte settings) is cable-only (5055 interface 3); helper's Open() takes only pid_5055 for it and reports "wireless" otherwise.
- Ops: lighting (VIA get 0x08 ch3 ids 1-4), light (VIA set 0x07 + save 0x09), info/set/time (0x01, 0x03 x10, 0x02, 0x05 x12 read; 0x01, 0x06, 0x02 write; clock BCD at 35-41), picture (System.Drawing cover-resize to 240x135, RGB565 big-endian, 64 KiB per frame, 36 frames max, 0x23, 0x01, 0x21 x56-byte chunks, 0x02).
- Panel: effect select, brightness/speed sliders, colour swatches (hue/sat), changes coalesced one run at a time; cable section: clock/picture 1/2, drop zones per slot, frame time, send, set clock, key light mode/colour/saturation, Win lock. Not written to the keyboard by me (no VIA set, no config write); not installed (Studio, Extensions).
