# Tourne'Suite Tweaks

Winaero Tweaker's options as Windhawk switches: 312 settings in 18 groups, from the taskbar and File Explorer to context menus, the lock screen, privacy and boot.

- **Your settings are the defaults.** The first time it sees a tweak, it only remembers it; after that, only what you change is applied.
- **Applied live where Windows allows it**: per-user settings are written straight away, and system metrics and fonts update without signing out.
- **One prompt for system-wide changes.** Machine-wide values and admin commands are gathered and run in one go behind a single UAC prompt. Anything that failed is retried on the next save.
- The same tweaks are a part of the [Tourne'Suite](../tourne-suite/) (`tweaks.*`). Run this standalone or the suite's part, not both.

The option list was read from Winaero Tweaker's own window, not its code. The mod is generated: edit `tweaks.def` in `tourne-desktop/source/` and run `Build-Tweaks.ps1`.

A few mappings aren't verified on a real PC yet: colourising the taskbar only, custom accent colours, the folder thumbnail handler, Edge's Essentials button policy, and the Windows 10-only "notifications stay open" value.
