# `Tourne'Tray` **[System Icons]**

Adds the Tourne pixel tray icons as real tray icons, for the system indicators
that no icon theme can reach: StartAllBack draws its network icon from a font,
the battery meter is built from sprite strips, and Windows 11 has no tray icon
at all for notifications, focus, night light or brightness.

Each icon is a normal tray icon, so it shows up in **Settings > Personalization
> Taskbar > Other system tray icons** (listed as Windows Explorer, told apart by
its picture) and can be turned on there or dragged onto the taskbar.

## Icons

| Icon | Shows | Left click | Right click |
|---|---|---|---|
| Network | Wi-Fi bars, no internet, off, not connected, Ethernet, unplugged, airplane mode | Network flyout | The real network icon's menu |
| Volume | Level, muted, no device | Volume Mixer | The real volume icon's menu |
| VPN | Connected or not | VPN settings | Network settings |
| Battery | Level, charging, battery saver, no battery | Power flyout | The real power icon's menu |
| Notifications | Bell, or Do not disturb | Notification center | Settings |
| Focus | Moon | Notification center | Focus settings |
| Night light | On or off | Night light settings | Display settings |
| Brightness | Sun | Brightness flyout | Display settings |
| Microphone, Camera, Location | Only while an app uses them, with the app names | Privacy settings | Privacy settings |
| Accessibility | Figure | Accessibility settings | Settings |
| Power | Power symbol | Lock, sign out, sleep, restart, shut down | Same |
| Input | Keyboard, with the current language | Input switcher | Language settings |
| Recycle Bin | Item count | Opens the Recycle Bin | Open, empty |
| Clipboard | Clipboard | Clipboard history | Settings |
| Screen capture | Camera | Snipping | Capture now |
| Show hidden icons | Up arrow, down while open | Hidden icons flyout: click, right-click or middle-click an icon as in the tray | Taskbar settings |

On the volume icon, **middle click** switches to the next output device and
**scrolling** changes the volume in steps of 2. Scrolling needs Windows'
"Scroll inactive windows when hovering over them" setting, which is on by
default.

## Flyouts

The network, power and brightness flyouts follow the Windows 7 layouts in the
Tourne style:

* **Network**: what you're connected to, then every Wi-Fi network with its
  signal. Click a network for Connect or Disconnect. A secured network you
  haven't joined before opens Windows' own sign-in, since that's where the
  password is entered.
* **Power**: battery status and the power plans, one click to switch.
* **Brightness**: a slider per monitor, over DDC/CI. Monitors that don't
  answer DDC/CI say so.

## Forwarded clicks

Right-clicking network, volume or battery opens the menu of the system icon
it stands in for, so the original has to stay enabled. It can live in the "^"
popup, or in a Classic Tray Folders folder, out of sight. If it's gone, a
short menu of the matching settings pages is shown instead.

## Not covered

The unread notification count has no public source, so the bell shows Do not
disturb but not unread.
