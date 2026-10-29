# Noctalia plugins in Tourne Desktop

Written 2026-10-06. Research and a proposal: no code until Tourne says go.

Tourne's ask: "look into noctalia plugins and how we can get a section in tourne desktop there."

**Short answer:** it fits unusually well. Noctalia v5 is a native C++ shell (no Qt) with TOML config, GUI-managed overrides and theme roles, like Tourne Desktop. Its plugins are small **Luau** scripts against a documented API, listed in two catalogues (249 plugins, 96% MIT). Tourne Desktop could embed the Luau VM, answer that API the way `host\module-shim.inl` answers Windhawk's, and show the catalogues as a "Noctalia" source in the Studio's Extensions page (plan section 7). Plugins that only use the API (clocks, timers, translators, GitHub, AI usage meters, notes) would run as they are. Ones that shell out to Linux tools (`hyprctl`, `nmcli`, `playerctl`) won't, and can be detected and marked.

## 1. What Noctalia's plugins are (checked 2026-10-06)

| Thing | What it is |
| --- | --- |
| The shell | [noctalia-dev/noctalia](https://github.com/noctalia-dev/noctalia): Wayland desktop shell, C++ on Wayland and OpenGL ES, no Qt or GTK. MIT. About 11,100 stars, pushed daily. Older v4 was QML on Quickshell; its plugins live in [legacy-v4-plugins](https://github.com/noctalia-dev/legacy-v4-plugins) and are out of scope. |
| Plugin language | Luau (Roblox's typed Lua; MIT; a C++ VM built to be embedded and sandboxed). Every entry file starts `--!nonstrict`. |
| API | One file, [`noctalia.d.luau`](https://github.com/noctalia-dev/official-plugins/blob/main/noctalia.d.luau) (796 lines), declares everything. Versioned by `plugin_api` (the example is on 9; the newest calls are marked API 28). |
| Manifest | `plugin.toml`: `id` ("author/name"), `name`, `version`, `plugin_api`, `author`, `license`, `dependencies`, `tags`, `icon`, `description`, then one table per entry: `[[widget]]`, `[[service]]`, `[[shortcut]]`, `[[launcher_provider]]` (`prefix`, `glyph`), `[[panel]]` (`width`, `height`, `placement`, `position`), `[[desktop_widget]]`. Settings are declared per entry (`[[widget.setting]]`: `key`, `type` string/glyph/bool/..., `label_key`, `default`). Also `thumbnail.webp` and `translations/<lang>.json`. |
| Catalogues | `catalog.toml` at the root of [official-plugins](https://github.com/noctalia-dev/official-plugins) (13 plugins) and [community-plugins](https://github.com/noctalia-dev/community-plugins) (236). CI-generated: one `[[plugin]]` row per plugin, plus `[[plugin.release]]` rows that pin an older `rev` for an older `plugin_api`. |
| Licences | Community: 227 MIT, 9 GPL or AGPL (each plugin declares its own; no repo-wide licence). Official: MIT. |

### The API, by part

- **Entries (surfaces):** bar widget, background service, quick-toggle tile ("shortcut"), launcher provider, panel, desktop widget.
- **Callbacks the host calls:** `update`, `onClick`, `onRightClick`, `onMiddleClick`, `onHover`, `onScroll`, `onIpc`, `onQuery`, `onActivate`, `onOpen`, `onClose`, `onKey`, `onFrameTick`, `onAudioSpectrum`, `onConfigChanged`, `onEnable`, `onOutputsChanged`, `onExit`.
- **Drawing:** `barWidget.setText/setGlyph/setImage/setTooltip/setColor/render`; a declarative `ui.*` tree for panels, desktop widgets and the bar: `column`, `row`, `scroll`, `box`, `label`, `markdown`, `glyph`, `image`, `separator`, `spacer`, `progress`, `button`, `graph`, `input`, `select`, `slider`, `toggle`, `dragSource`, `dropZone`.
- **Services (`noctalia.*`):** `http`, `httpStream`, `download`; files (`readFile`, `writeFile`, `listDir`, `pluginDataDir`...); `runAsync`, `runStream`, `runInTerminal`, `commandExists`, `processMatches`; `systemStats`, `diskStats`, `cpuCores`; `getColor(role)`, `isDarkMode`, `getSetting`; wallpaper calls; `notify`, `copyToClipboard`, `openColorPicker`; `state.set/get/watch` (shared between a plugin's entries); `json`, `tr`/`trp` translations, `fuzzyScore`, `sound`.
- **Glyphs** are icon names like `puzzle` and `list-search` (Tabler icons, MIT).

## 2. How each part would land in Tourne Desktop

| Noctalia | Tourne Desktop | Work |
| --- | --- | --- |
| Luau VM | Embedded in TourneDesktop.exe (Luau's VM and compiler, MIT, plain C++) | Small: add the sources to `Build-TourneDesktop.ps1` |
| `[[widget]]` + `barWidget.*` | A taskbar item, placed like the bar's other items; text, glyph, tooltip, colour role | Medium |
| `ui.*` tree | One renderer for the tree, drawn with the taskbar's own drawing code and themed by the Tourne theme | The biggest single piece |
| `[[panel]]` | A popup from the bar, sized from the manifest | Small once `ui.*` exists |
| `[[desktop_widget]]` | A layer on Tourne's desktop (`desktop-host.cpp`) | Small once `ui.*` exists |
| `[[shortcut]]` | A tile in the system icons' quick toggles | Small |
| `[[launcher_provider]]` | A prefix in Start's search (`/ex ...`) | Medium |
| `[[service]]` + `state.*` | Background script; state shared by key | Small |
| `getColor(role)` | Tourne theme roles: Noctalia's colour roles mapped to `@accent`, `@ground`, `@text` and the rest | Small |
| `http`, files, `json`, `tr` | WinHTTP, Win32 files under `%LOCALAPPDATA%\Tourne\Plugins\<id>` | Small |
| `runAsync` and friends | Run through `cmd`/`pwsh`; a Linux command line won't exist | See section 3 |
| `systemStats`, `diskStats` | PDH and `GetDiskFreeSpaceEx` | Small |
| `onIpc` | `TourneDesktop.exe --msg plugin <id>:<entry> <event> <payload>` | Small |
| Settings | Each `[[*.setting]]` becomes a Studio row, saved in `tourne-gui.conf` under `plugins.<id>` | Small: the Studio already builds rows from schemas |
| Glyphs | Bundle the Tabler icon font (MIT) | Small |

## 3. What will and won't run

- **Run as they are:** plugins that only draw and use `http`, files, `state`, `systemStats`, clipboard and `notify`. From the catalogue that includes world clocks, timers, translators, kaomoji and emoji pickers, notes, calculators, GitHub feeds, AI usage meters (Claude, Codex, DeepSeek) and Home Assistant.
- **Won't, as they are:** ones built on Linux tools or Wayland: Hyprland and Niri controls, NetworkManager, `playerctl`, ASUS and IdeaPad hardware, `mpvpaper`, package updaters.
- **Tell them apart before install:** scan the entry scripts for `runAsync`, `runStream`, `runInTerminal`, `commandExists` and `processMatches`, and read the `tags`. The card says "Works on Windows", "Needs <command>" or "Linux only". A later step could map common commands (media keys, volume, brightness) to Tourne's own services.

## 4. The decision this needs from Tourne

Plan section 7 says an extension "changes how the desktop looks, never what it runs". Noctalia plugins **run code**: Luau is sandboxed (no `os` or `io`), but the API itself offers HTTP, file writes and command execution. Three ways to square it:

1. **Permissions per plugin (recommended).** Every plugin runs; HTTP, writing files outside its own folder, and running commands each need a yes on install, shown on its card. Runs inside TourneDesktop.exe, at almost no cost while idle.
2. **Pure plugins only.** Only plugins that need no commands are installable. Simplest and safest, but hides much of the catalogue.
3. **Their own process.** Plugins run in a helper process, so a bad one can't take the shell down. Safest, at the cost of one process and some delay per draw.

Plan section 7 also says later widgets would use WebView2. A Luau host is far lighter than WebView2 (no browser process), which fits the "near-zero cost" rule better.

## 5. The section in Tourne Desktop

**Studio › Extensions › Noctalia** (a source filter on the Extensions grid):

- Cards from both catalogues, read only when the page opens: thumbnail (`thumbnail.webp`), name, the 120-character description, author, licence, tags, and the Windows verdict from section 3.
- **Install** fetches only that plugin's folder at the catalogue's `rev`, into `%LOCALAPPDATA%\Tourne\Plugins\<author>\<name>`. **Enable** adds its entries (a bar item, a panel, a tile...). Updates are compared by `version`.
- Each installed plugin gets its own Studio page from its manifest settings, with `@role` colours like every other setting.
- GPL and AGPL plugins are fetched by the user and never bundled (same rule as shaders); their licence is shown on the card.

## 6. Order

1. Luau in the build, a `[[service]]` and `[[widget]]` with `setText`/`setGlyph`/`setTooltip`, and `http`, `state`, `json`, `tr`: enough to run the official Example and World Clock.
2. The `ui.*` renderer, then panels and desktop widgets.
3. The Extensions page with the Noctalia source, install, enable and settings.
4. Shortcut tiles and launcher providers.
5. Permissions (section 4), and the command scan with Windows mappings.

## Sources

- [noctalia-dev/noctalia](https://github.com/noctalia-dev/noctalia) (README, licence, languages)
- [noctalia-dev/official-plugins](https://github.com/noctalia-dev/official-plugins): README, `noctalia.d.luau`, `example/plugin.toml`, `example/hello.luau`, `catalog.toml`
- [noctalia-dev/community-plugins](https://github.com/noctalia-dev/community-plugins): README (tags, submitting, licences), `catalog.toml`
- [Luau](https://github.com/luau-lang/luau)
