# 2026-10-12 13:45, Claude Code (web), Tourne'Icons 2.7: button icons

**Done**
- Tourne'Icons 2.7 / Lasting 1.7 / Kit 1.4: Tourne Desktop's Button icons now live in Tourne'Icons. New picker list "Taskbar buttons" (open windows by the app id the program chose, else file name; pinned ones). Rows of kind `button` go to HKCU\Software\Tourne\Icons (`Buttons` from the Windhawk mod while it runs, `Buttons Lasting`, `Buttons Kit`), newest first.
- First run takes in every taskbar.buttonIcons row from tourne.conf/tourne-gui.conf as button rows (taken-in-3.txt), designs (tourne:...) kept.
- Tourne Desktop (taskbar-host.cpp CustomIcon, switcher-host.cpp EverywhereIcon, system-icons-host.cpp ProgramTrayIcon, program-icons.cpp reader): Tourne'Icons' rows first; while the Windhawk mod's `Buttons` value exists its own Button icons rows step aside.
- Packs: a program row moves that program's button rows too (file name or same path only).
- Checked: clang i686/x86_64, gcc -Wall, Kit built, Wine unit test of matching and hand-over; Tourne Desktop's four parts compile (the exe doesn't link here: no Luau).

**Waiting on Tourne**
- Install 2.7 (Downloads), swap Tourne Desktop to this commit, then Apply a pack (ink pack 5, answer No) so the bar takes the pack.

**Notes**
- pack-maker doesn't draw `button` rows yet; app-id buttons (Discord's squirrel id, Security, web apps) keep Tourne's own icons until a Save lists from 2.7 is drawn.

**14:05 follow-up (2.7.1)**
- Tray is live: Tourne Desktop re-reads the button rows on the key's write time; WM_APP+4 also refetches the shell tray; Explorer-side tray rethemes on the key changing.
- pack-maker draws `button` rows (build.py 3rd arg = live icons.txt). Add-on "Tourne ink pack buttons.zip" in Downloads (20 rows). Tourne: Apply a pack, answer No; swap Tourne Desktop to the new commit.
