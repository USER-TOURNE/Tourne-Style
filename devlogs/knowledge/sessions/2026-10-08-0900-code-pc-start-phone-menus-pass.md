# Start and phone resizing, phone docking, desktop selection colours, dead menu items

When: 2026-10-08, about 06:40 to 09:00, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why (Tourne): resize the Start menu; attach the phone to anything (pagers); the phone inside Start makes submenus hard to use; the detached phone must resize by drag; many menu items open nothing (LiteMenu settings, the tray menu's settings items, Start's Properties); the desktop icons' outline and hover/selection colours must be themeable.

What changed: dd425ed (details in DEVLOG, last section). Swapped in at 08:53; `shell.iconLook = tourne` set in Tourne's conf.

Found on the way:
- Every "settings" item that did nothing restarted the program: the parts were written to run inside windhawk.exe. In Tourne Desktop a second copy without --studio just exits.
- Desktop icon selection isn't reported by the list view's custom draw (uItemState only had 0x200, keyboard cues). It's drawn by DrawThemeBackground(ListView, LVP_LISTITEM, state) and DrawFocusRect; the white outline was the focus rectangle. An unfocused list draws no selection at all.
- Task Manager's Claude group row: Claude is an MSIX package, so the group shows the package logo PNG, not Claude.exe's icon; b53's `*\Claude.exe` row can't reach it. Not fixed (asked Tourne).
- sed -i in Git Bash rewrote CRLF files as LF here; .gitattributes is `* -text`, so check `git ls-files --eol` (index) after edits.

Not verified on screen: all of the above. Open: phone docking only to Tourne's own windows (bars, pager, Start), not other programs' windows.
