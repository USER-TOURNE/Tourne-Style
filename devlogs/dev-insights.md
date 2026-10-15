# Dev insights

What a month of beating Windows into shape taught me. If you're building Windhawk mods or a shell of your own, steal these.

## Rules I don't break any more

- **Idle means no work.** No polling. Wait on a file change, a registry change, a window event or a timer that's only set while something's happening. Measure it: wakes per second per thread, not "feels fine".
- **New features ship off. Hotkeys default to none.** Installing a build should change nothing you can see.
- **Your settings are the defaults.** Read what the user already set (in StartAllBack, Winaero, Simple Window Switcher, Windows itself) and start from there.
- **Never put a low-level hook on a thread that draws.** Windows times the hook out and passes the key or click on. A hook gets its own thread and only posts.
- **Never do anything slow inside a hook.** No file writes, no sends. Defer it.
- **Post, don't send, between your own threads.** A send to a busy thread stalls the sender, and same-process `WM_COPYDATA` with a timeout can leave you reading freed memory.
- **Initialise COM on every thread that uses it.** WIC, Direct2D and the shell all fail quietly without it.
- **Compile for 32-bit too.** Windhawk 1.x runs tool mods as 32-bit, and 32-bit callbacks are `__stdcall`, which a lambda can't be.
- **Read-only first** on hardware and audio. Never send an unknown command to a device.
- **Measure from the real screen.** Your own capture code can lie.

## Windhawk

- Settings are YAML: quote anything with ` #` or `: ` in it, a setting with `$options` must be a string, and option keys can't be `on` or `off`.
- Mods can read settings but not write them. Use mod values for anything the mod remembers (positions, quick settings).
- A tool mod (`@include windhawk.exe`) runs in its own process. It survives Explorer restarts and can't take the shell down with it.
- Give each test build its own id. Going back is then one switch.
- Syntax-check in real compile mode, not the editor's, because some functions are macros there.

## Explorer and the shell

- Windows 11's XAML taskbar does not like being hidden from outside. Hide it from inside Explorer, the way Explorer hides it itself, and keep app progress away from it while it's hidden.
- The folder layouts live in shell32 on current builds, not explorerframe.
- uxtheme fills a missing value from the part, then the class, then the style's globals. "Missing" usually means "inherited".
- Being the shell means being the tray (`Shell_TrayWnd`), the app-bar host (`SHAppBarMessage`), the desktop (`Progman`) and the startup-app runner, and answering `Shell_NotifyIconGetRect`.
- Exit codes matter to a watchdog. A clean quit means "give the session back".

## Windows APIs that surprised me

- `GetTickCount64` moves every 15.625 ms.
- Asking for the current Wi-Fi connection's details counts as location use on 24H2.
- Windows won't let you cloak another program's window, but it will let you turn off its transitions, and a layered window at alpha 0 still has a live DWM thumbnail.
- Task view's desktop manager is undocumented and its slots change between builds. Check it answers before you call it.
- Store app logos are dozens of qualified PNGs, and some apps keep theirs outside their own folder.

## Process

- **Generate, don't hand-edit.** The suite, Tourne'Icons Lasting, Tourne Menus and Tourne'Table 2.0 are all built by scripts from their parts. Anchors that must match an exact number of times make a stale edit fail loudly.
- **Test outside the real process.** A test bar, a test runner for plugins, harnesses that call the hooks directly. You can't debug a shell from inside the shell.
- **Write it down.** Every build has a devlog entry saying what was checked and what wasn't. "Not tried on Windows yet" is a perfectly good sentence.
