# Claude Cockpit

Watch your Claude Code sessions and plan limits from the bar. One panel
shows the running sessions, usage and cost, every past session, your
CLAUDE.md files, and the installed skills and mods.

Ported to Tourne Desktop (Windows) from nightwatch75's Claude Cockpit for
Noctalia (MIT, see `LICENSE`). What changed is listed under
[Windows](#windows).


## Plugin

| Field | Value |
| --- | --- |
| ID | `tourne/claude-cockpit` |
| Entries | Bar widget: `widget`; panel: `panel`; service: `service` |

The headless `service` fetches usage and polls the live sessions. The
widget and the Live and Usage tabs only show what it publishes.

## Requirements

- A logged-in Claude Code (`%USERPROFILE%\.claude\.credentials.json`) for
  the Usage tab.
- `claude.exe` on the `PATH` or in `%USERPROFILE%\.local\bin` (where Claude
  Code's own installer puts it) for the Skills/Mods tab and for resuming a
  session. An npm install's `claude.cmd` isn't run: a plugin starts programs,
  not scripts.
- Optional: VS Code or Zed to open a CLAUDE.md, or set `editor_command`.
  Without either it opens in the file's usual app.

Nothing else: no Bash, jq, curl or WSL.

## Usage

Add **Claude Cockpit** to a bar from the Add-widget picker. Click it to
open the panel, or:

```sh
TourneDesktop.exe --msg panel-toggle tourne/claude-cockpit:panel
```

### Bar widget


- The glyph turns into a red bell when a session waits for you.
- One dot per running session: red needs you, accent is working, grey is
  idle. After 8 dots it shows `+N`.
- Rings and percentages show the 5-hour session and the 7-day weekly
  window. They turn amber or red when usage runs ahead of the clock.

Set `display_mode` to `classic` for the glyph and percentages only.

### Live

Sessions are grouped into *Needs you*,
*Working* and *Idle*. Each card shows the title, the time in its state, the
path and git branch, what it waits for, the last prompt, the model, the
context size and the cost. Click a card to focus its terminal window. The globe opens the session on claude.ai while Remote Control is
on.

### Usage


Cost is an estimate from token counts and public prices, in the currency
you choose.

### Sessions


Click a session to resume it (`claude --resume <id>`) in a terminal, in its
project directory. Deleting a session moves its transcript to the Recycle Bin. The search box filters by title, prompt or path.

### CLAUDE.md


The list also has a row for each session project without a CLAUDE.md, so
you can create one. Noise folders (`.git`, `node_modules`, `.cache`,
`.venv`, and `AppData`) and linked folders are skipped.

### Skills/Mods


- Skills and mods are Claude Code plugins, so this tab drives
  `claude plugin`. Bare skill folders in `~/.claude/skills` are listed too.
  You can only remove them.
- **Check updates** fetches every marketplace and marks a plugin with a
  newer version as `old → new`. It installs nothing.
- Removing a marketplace also uninstalls its plugins.
- The install box takes `plugin@marketplace` to install a plugin, or
  `owner/repo`, a git URL or a path to add a marketplace.
- Restart Claude Code to apply plugin changes.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| `usage_refresh_interval` | `int` | `2` | Minutes between usage fetches (2 to 15). |
| `currency` | `select` | `auto` | Cost currency: `auto` follows the locale, or one of `usd`, `eur`, `gbp`, `jpy`, `cny`, `chf`, `aud`, `cad`, `inr`. |
| `currency_api_url` | `string` | `https://api.frankfurter.dev/v1/latest` | Exchange-rate API for non-USD currencies. |
| `terminal` | `string` | `""` | Terminal to resume a session in: `wt` with any options. Empty opens a Windows Terminal tab, or a console window without it. |
| `editor_command` | `string` | `""` | Command to open a CLAUDE.md: `code` or `zed`, with any options. Empty tries VS Code, then Zed, then the file's usual app. |
| `display_mode` | `select` | `activity` | Widget mode: `activity` (dots, rings, percentages) or `classic` (glyph and percentages). |
| `glyph` | `glyph` | `robot` | Widget glyph. |
| `usage_percent_display` | `select` | `both` | Windows the widget shows, in both modes: `session` (`sNN%`), `weekly` (`wNN%`), `both`, or `none`. |

## Notes

What the plugin touches:

- **Reads** `~/.claude/sessions/*.json` and the transcripts of running
  sessions, every 2 seconds, only while the widget is in activity mode or
  the Live tab is open. A transcript is parsed again only when it changes.
  The Usage and Sessions tabs read the other transcripts in small slices.
- **Reads** `~/.claude/.credentials.json` for the token of the usage query.
- **Writes** only in the plugin's data folder: `pricing-cache.json` and
  `usage-cache.json` (caches you can delete) and `rings/` (widget ring
  images). Live session stats are kept in memory.
- **Network**: the Anthropic usage API, the LiteLLM price table and the
  exchange-rate API, all over HTTPS. The Sessions and CLAUDE.md tabs make
  no network calls.
- **Runs** `claude --version` once (for the usage query's User-Agent, as the
  original), `claude plugin` on a click in the Skills/Mods tab, your
  terminal and your editor. Tourne Desktop focuses a window itself.
- **Deletes**, always after an inline confirm: a session transcript (trash
  glyph), a plugin or marketplace (through `claude plugin`), a local skill
  folder (a link loses the link only). Transcripts and skill folders go to
  the Recycle Bin.
- No rename: Claude Code cannot rename a session after it starts.

The Usage engine, `get-claude-usage`, was copied (MIT) from
[jrohland/claudecode](https://github.com/jrohland/noctalia-v5-claudecode),
reading only the default `~/.claude` account; here it is `usage.luau`, its
Luau port. See `LICENSE`.

Localized in English.

## Windows

- **Usage** (`usage.luau`): the same numbers and the same `--json` shape as
  `get-claude-usage`, made in Luau. Transcripts are read with
  `tourne.findFiles` and `tourne.jsonLines`, the web asked with
  `noctalia.http`. It sends the login token only to `api.anthropic.com`, as
  the original does, and never logs, stores or copies it. With no token the
  tab shows the original's fallback. The caches live in the plugin's data
  folder, not in `~/.claude`.
- **Live**: Claude Code keeps `~\.claude\sessions\<pid>.json` on Windows
  too. A session counts as running while Windows runs its pid and that
  process started no later than the session (`tourne.processInfo`, in place
  of `/proc/<pid>/stat`). Clicking a card brings its terminal window to the
  front (`tourne.focusProcess`) where Linux used niri or Umbriel.
- **Sessions** reads the transcripts with `tourne.findFiles` and
  `tourne.jsonLines` (find, grep, tac and jq on Linux); Claude Code names a
  project folder like `C--work-app` for `C:\work\app`.
- **Deletes** go to the Recycle Bin (`tourne.trash`, only inside
  `~\.claude\projects` and `~\.claude\skills`), where Linux ran `rm -rf`.
- **CLAUDE.md** scan: `tourne.findFiles` over the user folder; there is no
  `/proc/mounts`, and no network share sits under the user folder.
- **Resume** opens Windows Terminal (or a console window) in the project
  folder; links open in the default browser.
- Tourne's bar calls the widget's `onClick` for the `[widget.actions]` left
  click, which toggles the panel the same way.
- Tourne Desktop lets the plugin read `~\.claude`, `~\.agents` (the skills
  lock file) and the user folder (for the CLAUDE.md scan), and start only
  `claude`, `code`, `zed` and `wt`.

## License

[MIT](LICENSE)
