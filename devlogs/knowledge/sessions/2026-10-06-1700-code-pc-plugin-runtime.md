# Plugins run: Luau runtime and bar widgets

When: 2026-10-06, about 16:20 to 17:10, Claude Code in the terminal on the desktop PC, branch `claude/project-thread-0rkfwp` (started at ce804e8).

Why: Tourne signed out and in (shell mode back, on 502af7a, both processes since 16:21, no crash), said "it works, now check our task list and start on it". Next on the list was the plugins' Luau runtime.

What changed:
- 27487e7: Luau 0.741 vendored (`desktop/third_party/luau`), `desktop/Build-Luau.ps1` (compiled once, kept).
- 1e4ebdd: `desktop/host/plugin-runtime.cpp/.h`, `toml-lite.cpp/.h`, `plugin-bar-shared.cpp` (the bar protocol), the taskbar showing plugin items before the tray, the Tabler glyph font and map (`desktop/plugins/tabler-*`, `tools/plugins/Make-GlyphMap.ps1`), host start/stop and Studio reloads, Remove deleting saved data too, `tools/plugins/Run-PluginTest.ps1` + `pluginrun.cpp` + `sandbox-test/`.
- This commit: DEVLOG "Plugins run: Luau, and their widgets on the bar", this note.

State: builds (7.5 MB). Checked outside Tourne Desktop (Run-PluginTest, see DEVLOG). Swapped in at 18:02 (planned restart, shell child 6044, no crash, glyph font written; the old exe is `TourneDesktop.exe.prev-502af7a`). Tourne then signs out and in so the watchdog is 59579f3 too. Nothing is installed in Tourne's real plugin folder yet, so after a swap the bar looks the same until a plugin is installed from the Studio's Extensions page.

Next:
- Tourne: install World Clock from Studio > Extensions, check the globe appears before the tray, then try `plugins.noctalia/world_clock.show_clocks = 1` in tourne.conf.
- Then plan step 2: the `ui.*` renderer, so panels (World Clock's panel, opened by its click) and desktop widgets work.
- Still waiting from earlier notes: switcher views to try, the pull request into `main`, sharing the claude.ai page, the Projects on the app logins.

Learned:
- Luau reports errors as C++ exceptions: anything that touches a VM outside `lua_pcall` (pushing a callback's arguments) needs a catch, or a plugin over its memory cap would end the shell.
- Same-process `WM_COPYDATA` with a timeout is unsafe (the data is the sender's, gone if it gave up), and any send to a busy thread stalls the sender. Bars post to the runtime instead; indexes are checked against a list version that changes only when the items themselves do.
- clang 20 (Windhawk's) supports `#embed`: a 1.8 MB font builds in instantly.
- PowerShell: a script param named `-Debug` clashes with the common parameter once any `[Parameter()]` attribute is used; a positional `[string]` param can swallow the first of a `ValueFromRemainingArguments` list (use `PositionalBinding = $false`).
