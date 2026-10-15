# Plugins: index and the Studio's Extensions page

When: 2026-10-06, about 15:50 to 16:40, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at 699c870).

Why: Tourne's plugin decision (see the 03:30 note): a Plugins section on the Studio's Extensions page, cards with Install, Linux-only plugins left out, each plugin's needs shown on install with permission per plugin. Then Tourne said "continue" in a fresh session, which found the work committed but not pushed and with no note.

What changed:
- 348883c: `tools/plugins/Make-PluginIndex.ps1` and `desktop/plugins/index.json` (30 of Noctalia's plugins run on Windows).
- fd8f969, 502af7a: `desktop/host/plugins.cpp/.h`, the Extensions page in `desktop/host/studio.cpp`, the index built into the exe, and `tools/plugins/plugintest.cpp`.
- This commit: the test's build line (needed `-static -luuid`), the DEVLOG section "Plugins: a Plugins section in the Studio", this note. Branch pushed.

State: Tourne Desktop builds; plugintest reads all 30 cards and installs and removes World Clock. Nothing swapped in this session. Tourne was last on Explorer with shell mode off (15:43 swap, see the 03:30 note) unless they turned it back on since.

Next:
- Tourne: get back into shell mode with the fixed build (start it now, or turn shell mode on and sign out and in so the watchdog is new too), then try moving a tray icon to the overflow and open the Studio's Extensions page.
- Luau runtime for plugins, then bar, panel and desktop placement.
- The waiting items from the 03:30 note (switcher views to try, the pull request into `main`, sharing the claude.ai page, the Projects on the app logins).

Learned:
- Small test programs built with Windhawk's clang need `-static`, like the main build: without it they exit at once (0xC0000135, `libc++.dll` not found). `FOLDERID_*` needs `-luuid`.
