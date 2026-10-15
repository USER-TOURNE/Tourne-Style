# Why Explorer keeps crashing (read only, nothing changed)

When: 2026-10-07, about 21:30, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: Tourne asked why Explorer keeps crashing. They said it was stable around when the Notes plugin went on the bar.

What was found:
- 21 Explorer crashes in the last 30 days. 18 are the same one: `Windows.UI.Xaml.dll+0x90e603`, code `0xc000027b` (an unhandled WinRT error inside XAML). Tonight: 9 between 20:56 and 21:21. Before that: 10-04 (5), 09-29 (1). Odd ones: `Microsoft.UI.Xaml.dll` 09-28, `wpnapps.dll` 10-04 16:37, null jump x3 on 09-21 (older build), heap corruption 09-09.
- They start after Windows update KB5124009/KB5124010 (09-22), which moved Explorer from 26100.9444 to 26100.9549.
- Tonight's crashes all came after `--restore` at 20:56:07 (Explorer became the full shell again). shell.log only goes back to 10-05, so 10-04 can't be matched.
- I parsed the 9 dumps in `%LOCALAPPDATA%\CrashDumps` with a small C# reader (no debugger on the PC). Every one dies on Explorer's own taskbar thread: explorer.exe, then `Taskbar.dll`, then XAML. Windows' XAML taskbar (`Taskbar.View.dll`, `SystemTray.dll`) is loaded next to StartAllBack 3.9.27. No Windhawk mod DLL is on the crashing stack. The dumps are minidumps, so the error text itself isn't in them.
- Other DLLs in Explorer: 18 Windhawk mods incl. b53, StartAllBack, `D:\Applications\Theming Applications\hook64.dll` (Sizer, 2020).
- Windhawk mods: the backup `G:\Device Files\BACKUP OF WINDHAWK MODS AND THEIR SETTTINGS\2026-10-05-18h18m36-windhawk-backup.json` matches today, except native-shadow-tuner and shadowplay-single-folder were on then and are off now (and b48 exists only in the registry, empty).

Most likely cause: Windows' own XAML taskbar crashing on the new build with StartAllBack 3.9.27 on top. Not a Windhawk mod and not Tourne Desktop.

Next (proposed, not done): update StartAllBack, or test one session with it off; turn on full dumps (HKLM LocalDumps, needs admin) to read the actual error; or go back to shell mode, where Explorer's taskbar stays hidden.
