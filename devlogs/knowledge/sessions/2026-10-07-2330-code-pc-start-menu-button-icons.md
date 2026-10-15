# Start menu uses the Button icons rows

When: 2026-10-07, about 23:30, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne's Start menu icons were still Windows' defaults although their Icon Redirect settings were set. The Start menu (start-menu.cpp, drawn by Tourne Desktop) only asked the shell for icons, so the redirects never reached it. Icon Redirect in Windhawk (b53) hooks Explorer, which is hidden underneath in shell mode, and the taskbar's Button icons rows only applied to the taskbar and tray.

What changed:
- start-menu.cpp: `StartButtonIcon` checks every `taskbar.buttonIcons` row (not only "Use everywhere") against the Start item's app id or its .lnk target, as the taskbar does. It runs first in `MakeStartIcon`, so a program's own icon wins over the apps folder rule and the Windows icon. It reads only settings and parses the .lnk, so it is safe on the icon thread.
- Not covered yet: rows with a `tourne:` design (drawn on the UI thread), so those Start items keep the Windows icon.

Cache: the Windows icon cache was rebuilt first (files moved to `%LOCALAPPDATA%\Tourne\icon-cache-backup-20261007-230239`). That did not change the Start menu, so the cache was not the cause.

Menu icons: the menus with icons are Tourne Desktop's own (taskbar, Start, desktop menu). Their switch is `menuNoIcons.enabled` in Tourne's config, which is not set (default: icons on). Windhawk's `menuNoIcons.enabled = 1` only affects Explorer's menus. Not changed here.

Check: after the swap, the Start menu shows each program's Button icon. Open the Start menu, compare with the taskbar's button for the same program.
