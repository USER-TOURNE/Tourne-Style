# Pull request: claude/project-thread-0rkfwp into main

Open it here: https://github.com/USER-TOURNE/tourne-suite-sab-clone/compare/main...claude/project-thread-0rkfwp?expand=1

Title: Tourne Desktop: shell mode, Studio, glass, motion, plugins and the window manager

Description (paste below the title):

---

Everything since b51 on the working branch (73 commits, 2026-10-03 to 2026-10-07).

**Tourne Desktop** (`desktop/`): the suite's hook-free parts as one program of its own.
- Shell mode: Tourne Desktop runs as the Windows shell with a sign-in watchdog, its own tray host, app bars and work area, the desktop and wallpaper, and Explorer hidden underneath for Windows apps. A swap's restart isn't counted as a crash; two crashes within 120 s turn shell mode off.
- Taskbar, switcher and System Icons built in, with b53's tray designs, the clock in the user's colours, and an importer for a Windhawk suite build's settings.
- Tourne Studio: every setting in one window, 824 built-in presets, dropdowns for Windhawk dynamic selects, Plugins and Extensions pages.
- Glass (engine, rims, app glass), identity packs, the Tourne theme from the wallpaper, the motion engine (Hyprland curves and springs), a CSS injector for web-based apps.
- Killswitch for Windows' taskbar and Start; `--restore` puts it back.

**Plugins**: Noctalia plugins on Windows. A Luau 0.741 runtime with one sandboxed VM per entry, permissions from each manifest, bar widgets on the taskbar, panels in the flyouts' look, and the install dialog.

**Tray**: overflow icons hear the pointer (AlienFan's tooltip), and tray folders are edited in the hidden icons flyout (drag in and out, stack two icons for a new folder, rename, glyph, delete).

**Window manager**: per-desktop tiling layouts, send-to-desktop keys, focus border, Tacky Borders style borders; switcher views Flip 3D, Cover Flow, Grid, Mission Control, Ring and Shelf.

**Tools and notes**: `desktop/Swap-TourneDesktop.ps1`, `tools/windhawk/Set-SuiteParts.ps1`, `tools/tray/trayprobe.cpp`, the plugin test runner, the shared knowledge base (`knowledge/sessions/`), DEVLOG sections, and research plans (the Tourne Hooks plan is marked not going ahead).

The hook parts (Explorer Style and Colours, text colour, classic menus, Icon Redirect, tweaks) stay in the Windhawk suite, which runs beside Tourne Desktop with only those parts on.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
