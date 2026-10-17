# Syncthing

Syncthing status and control for Tourne Desktop. Shows the sync state in the bar,
gives you a Syncthing Tray-style panel with folder and device lists, per-folder
and per-device pause/resume, rescan, sync progress, transfer rates, and desktop
notifications on sync events. Ported from Marco Ziliani's Syncthing plugin for
Noctalia (MIT), itself a port of the v4 `syncthing-status` plugin (by
Pir0c0pter0). It talks to the Syncthing REST API directly, with no helper
scripts.

## Plugin

| Field | Value |
| --- | --- |
| ID | `tourne/syncthing` |
| Entries | Bar widget: `bar`; panel: `panel`; service: `poller`; shortcut: `pause`; launcher: `folders`; desktop widget: `desktop` |
| Launcher Prefix | `/st` |

## Requirements

Install Syncthing (`winget install Syncthing.Syncthing`, SyncTrayzor, or the zip
from syncthing.net) and have it running for this user. The plugin autodetects
the GUI URL and API key from `%LOCALAPPDATA%\Syncthing\config.xml` (or the older
`%APPDATA%\Syncthing\config.xml`, or `%APPDATA%\SyncTrayzor\syncthing\config.xml`),
so it usually works with zero configuration. A GUI listening on every address
(`0.0.0.0:8384`) is reached on `127.0.0.1:8384`.

## Usage

- **Bar widget** (`bar`): add it from Studio. While everything is up to date it
  shows the Syncthing logo; in any other state (syncing, paused, error, offline,
  ...) it shows that state's glyph in its colour, plus the overall completion
  percentage while syncing. Left click opens the panel, right click forces a
  refresh. The tooltip shows devices, folders, pending items, transfer rates,
  and the last check time.
- **Panel** (`panel`): header with pause-all/resume-all, open web GUI (in your
  default browser), and refresh buttons; stat tiles (devices / folders /
  pending); **Folders | Devices** tabs. Folder rows show state, sync progress,
  and last activity, with per-folder pause/resume and rescan buttons. Device
  rows show connection state, address, and client version, with per-device
  pause/resume.
- **Desktop widget** (`desktop`): turn it on in Studio. A compact card with
  state, counters, sync progress, and transfer rates.
- **Launcher** (`/st`) and **Shortcut** (`pause`): kept from the original;
  Tourne Desktop doesn't run launcher providers or shortcut tiles yet.
- **Notifications**: by default the service notifies when a folder finishes
  syncing, reports an error, or a device connects/disconnects. Turn off with
  the **Event notifications** setting.

## Settings

| Setting | Type | Default | Description |
| --- | --- | --- | --- |
| `url` | `string` | `""` | Syncthing GUI base URL. Empty = autodetect from config.xml. |
| `api_key` | `string` | `""` | REST API key. Empty = autodetect from config.xml. |
| `config_path` | `file` | `""` | Explicit config.xml path for autodetection (advanced). |
| `poll_interval` | `int` | `10` | Status refresh interval in seconds (2 to 300). |
| `notify_events` | `bool` | `true` | Desktop notifications for folder done/error and device connect/disconnect. |
| `folders` | `string_list` | `[]` | Folder IDs to monitor. Empty = all folders (advanced). |
| `insecure_tls` | `bool` | `false` | Accept self-signed certificates on HTTPS GUI URLs (advanced). |
| `show_pending` | `bool` | `true` | Bar widget: show completion %/pending count while syncing. |

## Notes

- Requests to the Syncthing API go through a small queue (four at a time), so
  many folders never hit the host's limit on requests in flight.
- **Network**: all requests go to the local (or configured) Syncthing REST API
  via `noctalia.http`: `/rest/noauth/health`, `/rest/system/status`,
  `/rest/config`, `/rest/system/connections`, `/rest/system/error`,
  `/rest/stats/folder`, `/rest/db/status`, and the action endpoints
  (`/rest/config/folders|devices/<id>` PATCH, `/rest/db/scan` POST,
  `/rest/system/pause|resume` POST). No other network access.
- **Programs**: none. *Open web GUI* hands the URL to Windows, which opens it
  in the default browser.
- **Files**: reads Syncthing's `config.xml` for URL/API key autodetection.
  Nothing is written.
- The API key never leaves the machine: it is only sent as the `X-API-Key`
  header to the Syncthing GUI endpoint it belongs to.

## Credits

Marco Ziliani's Syncthing plugin for Noctalia, based on the v4
[syncthing-status](https://github.com/noctalia-dev/legacy-v4-plugins/tree/main/syncthing-status)
plugin by Pir0c0pter0. MIT license.
