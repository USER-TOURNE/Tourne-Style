# Desktop preview stays open and is clickable; Start icons follow the taskbar pins

When: 2026-10-07, about 23:55, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: the desktop pager's preview closed as soon as the pointer left its mark, and it was click-through, so you couldn't go to a desktop from it. Start menu icons also didn't match the taskbar where the taskbar's pin has its own icon (Chrome: Bouquet's CHROME.ico on the pin, Windows' icon on the Start shortcut).

What changed:
- desktops-pager.cpp: the preview isn't click-through any more (no WS_EX_TRANSPARENT, hit test is client). A click on it goes to that desktop (PagerGoTo). Moving onto it keeps it open; leaving it closes it. Leaving a mark, the bar or the floating pager starts a 250 ms pause (`HideDesktopPreviewSoon`), so the pointer can cross the gap onto it. A mark pointed at again cancels the pause.
- taskbar-host.cpp: the bar's mouse-leave uses the same pause.
- start-menu.cpp: `StartPinnedIcon` gives a Start item the icon of the taskbar's pin for the same program (matched by target), when no Button icons row matches. This covers every pinned program, not only Chrome. UWP apps (shell:AppsFolder) have no target to match, so they still need a row.
- Config: `taskbar.buttonIcons[19]` = chrome.exe, CHROME.ico, everywhere on (added in the real profile; the previous file is `tourne-gui.conf.before-chrome-row`).

Not done: the ghost frames around unfocused windows during previews. The border code hides frames on minimise and cloak; the cause isn't found yet.

Check: hover a desktop mark, move onto the preview, click it; check the Chrome icon in the Start menu against the taskbar.
