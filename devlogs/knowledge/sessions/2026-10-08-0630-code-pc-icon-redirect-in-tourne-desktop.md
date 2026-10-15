# Icon Redirect built into Tourne Desktop

When: 2026-10-08, about 06:15 to 06:30, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why: follows the 06:15 note (the icons Tourne Desktop draws had lost the Bouquet fixed-alpha theme, because b53 excludes TourneDesktop.exe). Tourne picked option 3: Icon Redirect inside Tourne Desktop.

What changed (693afc8, swapped in at 06:27):
- `tourne-icon-redirect.wh.cpp` builds in unchanged as module `iconRedirect`, a part beside the suite: hooks in Tourne Desktop's own process only (`desktop/host/hooks.cpp`, MinHook 1.3.4 in `desktop/third_party/minhook`, BSD-2).
- It reads b53's `iconRedirect.*` settings live from Windhawk (the suite layer) and the themes in Windhawk's mod-storage for the suite build. It runs while b53's `iconRedirect.enabled` is on, starts first, has no Studio page, isn't imported, and stands down if a suite DLL is loaded in the process.
- Changed rules need a Tourne Desktop restart (logged).

Checked: desktop.log "106 rules, 47 files"; the five hooked functions start with a jump in the shell child. Not seen on screen yet: Tourne to check the desktop icons, Start's places and a file dialog.
