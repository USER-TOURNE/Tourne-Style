# `Tourne'Tray` **[Clock]**

Takes over the drawing, size and clicks of the classic taskbar clock, the one
StartAllBack shows on every taskbar. **Keep that clock turned on in StartAllBack**:
this mod redraws it, and StartAllBack removes the clock's window when it's off. The time itself still comes from Windows:
the clock reads the system time and your region's formats the same way the
built-in clock does, and follows time zone and format changes.

## Lines

Up to three lines, each a mix of text and these tokens:

| Token | Shows | Example |
|---|---|---|
| `{time}` | The short time, as the built-in clock shows it | 9:41 PM |
| `{time:long}` | The long time, with seconds | 9:41:07 PM |
| `{date}` | The short date | 9/24/2026 |
| `{date:long}` | The long date | Thursday, September 24, 2026 |
| `{weekday}` / `{weekday:short}` | The day of the week | Thursday / Thu |
| `{t:...}` | Any time picture: `h H hh HH m mm s ss t tt` | `{t:HH:mm}` 21:41 |
| `{d:...}` | Any date picture: `d dd ddd dddd M MM MMM MMMM yy yyyy` | `{d:dd MMM}` 24 Sep |

The clock ticks every second only when a line shows seconds.

## Clicks

Left, right and middle click each pick one action: the Tourne notification
centre and calendar (System Icons' flyout), what Windows does, nothing,
the clock's own flyout, the taskbar menu, notifications, date and time
settings, Alarms & Clock, copying the shown text, or a command of your own.

## Position

The native spot, after the tray icons, or left of the tray icons.
