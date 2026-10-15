# Discord Voice

Monitor and control a local Discord desktop voice session from Tourne Desktop's bar. The bar widget and panel expose voice state and controls through Discord's local RPC interface.

Ported to Tourne Desktop (Windows) from raycursive's Discord Voice for Noctalia (`raycursive/discord-voice`, MIT). The panel, the bar widget, the settings and the Python bridge are the original's; what changed for Windows is listed under **Windows**.

## Plugin

| Field | Value |
| --- | --- |
| ID | `tourne/discord-voice` |
| Entries | Bar widget: `bar`; panel: `panel`; service: `bridge` |

## Requirements

- Python 3.10 or newer, as `python` on the `PATH` (python.org's installer, or `winget install Python.Python.3.12`), or its full path in the **Python** setting. Windows' own `python` stand-in that only offers the Microsoft Store is reported as missing Python.
- The Discord desktop app. The default is Discord's own launcher, `%LOCALAPPDATA%\Discord\Update.exe`; change **Discord executable** if yours is elsewhere (PTB and Canary have their own `Update.exe`, and `Discord.exe` itself works too).

Discord must be running locally; the Discord web app alone is not sufficient.

## Usage

Turn the plugin on in Tourne Studio's Extensions page and add its `bar` widget to the bar.

1. Click the bar widget to open the `panel` entry. If Discord is not running, the first click starts the configured Discord executable.
2. Select **Authorize Discord** and accept Discord's local consent prompt.
3. Use the panel to mute, deafen, hang up, adjust your input volume from 0-100%, or adjust another participant's listening volume from 0-200%.
4. Select saved recent or favorite channels to rejoin them in Discord.

The `bridge` service starts automatically when the plugin is enabled and keeps the bar widget and panel synchronized.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| `discord_binary` | `file` | `%LOCALAPPDATA%\Discord\Update.exe` | Executable used to launch Discord and open channel deep links. |
| `python` | `string` | `python` | Python's name on the `PATH`, or the full path to `python.exe`. |
| `glyph` | `glyph` | `brand-discord` | Glyph used by this bar-widget instance when no participant avatar is shown. |
| `show_channel_name` | `bool` | `true` | Shows the active voice-channel name beside the glyph. |
| `hide_when_disconnected` | `bool` | `false` | Hides the bar widget while no voice channel is connected. |

## Commands

The bridge takes these commands (the panel runs them itself):

```
python discord_bridge.py command authorize
python discord_bridge.py command refresh
python discord_bridge.py command hang-up
python discord_bridge.py command status
```

Noctalia's `noctalia msg plugin ...` events have no Tourne Desktop counterpart.

## Windows

- Discord's local RPC is the named pipe `\\.\pipe\discord-ipc-0` (to `-9`) instead of a Unix socket, with the same framing. The bridge's own control channel is the named pipe `\\.\pipe\noctalia-discord-voice-<user>`.
- In place of the Unix peer-credential check, the bridge accepts a pipe only when its server process is Discord's own executable (`Discord.exe`, `DiscordPTB.exe`, `DiscordCanary.exe`, `DiscordDevelopment.exe`) running as the same Windows user.
- Discord starts through its launcher: `Update.exe --processStart Discord.exe` (the app named after the launcher's folder), and a channel link goes to it as `--process-start-args "--url -- discord://..."`. A `Discord.exe` path is run as `Discord.exe --url -- discord://...`, as on Linux.
- Participant avatars are downloaded by the plugin's service (Tourne Desktop shows pictures only from the plugin's own folders) into its data folder, `avatars\`.
- The access token and saved channels are in `%LOCALAPPDATA%\noctalia\discord-voice\` (`token.json`, `channels.json`), unless `NOCTALIA_STATE_HOME` or `XDG_STATE_HOME` is set.
- Discord started from the bar or panel runs among Tourne Desktop's plugin programs, so it closes when Tourne Desktop closes. Discord started any other way is not affected.

## Notes

- The plugin requests only the Discord OAuth scopes `rpc`, `rpc.voice.read`, and `rpc.voice.write`. It does not request message, notification, relationship, or guild-member scopes.
- Authorization sends the code to `https://streamkit.discord.com/overlay/token`. Participant avatars are downloaded from `https://cdn.discordapp.com`; no downloaded content is executed.
- Discord's StreamKit token exchange does not provide a refresh token. Authorize again after the access token expires.
- Voice levels changed through Discord RPC belong to the active integration; Discord may restore previous levels when the bridge disconnects.
- The bridge reconnects after Discord restarts. The integration depends on Discord's public StreamKit identity and token broker, which Discord may change.

The authorization design was from [`PandorasFox/dms-discord-widget`](https://github.com/PandorasFox/dms-discord-widget).
