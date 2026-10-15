# `Tourne'Tray` **[Classic Tray Folders]**

Adds folders to the **classic** tray overflow, the "^" popup of the Win32 taskbar
that StartAllBack (and the Windows 10 taskbar) uses. Each folder is a tile in the
popup with a preview of the icons inside. Clicking it shows only that folder's
icons, and the tile turns into a back arrow.

This is the classic-taskbar counterpart of **System Tray Folders** by Ris Peng,
which only works with the Windows 11 taskbar. If `SystemTray.dll` is not loaded
in Explorer, that mod has nothing to attach to and this one is what you want.

## How it works

* **Folders are real tray icons.** The mod adds one tray icon per folder, so
  Explorer treats it like any other icon: it has a tooltip, it can be dragged,
  and nothing foreign is inserted into Explorer's own toolbars.
* **Icons in a folder are hidden in the popup only.** Their place on the taskbar,
  and Explorer's saved "show on taskbar" choices, are never touched. An icon you
  have dragged onto the taskbar stays there even if it matches a folder.
* **Icons inside a folder are the real icons**, so clicking, right-clicking and
  tooltips behave exactly as they do outside a folder.
* **The popup is trimmed to fit** whatever is currently shown.

Closing the popup always returns it to the top level. Turning the mod off puts
every icon back where Explorer had it.

## Choosing icons

Each pattern is compared, ignoring case, with these names of every icon:

* **App name**: the file description of the program, e.g. `Discord` or
  `NVIDIA Container`.
* **Executable**: the file name, with and without `.exe`, e.g. `sizer.exe` or
  `sizer`.
* **Tooltip**: the text you see when hovering the icon.

A pattern without wildcards must match a whole name. Use `*` for any text and
`?` for a single character, e.g. `*NVIDIA*`. An icon goes into the first folder
with a matching pattern.

To see every icon's names, turn on logging in the mod's **Advanced** tab, open
the popup once, then click **Show log output**. Each icon is listed as
`Icon: exe="..." app="..." tooltip="..." folder=N`, where `N` counts from 0 and
-1 means no folder.

## Tooltips

Tooltips are learned as programs send them to the tray. Icons that were already
in the tray when the mod loaded, and have not changed since, fall back to the
first tooltip Windows remembers for them, which can be empty or out of date.
Restart Explorer once after installing and every tooltip is picked up from the
start.

## Limits

* Folder tiles sit wherever Explorer places new icons, usually at the end.
  Explorer keeps the popup and the taskbar in the same order and tracks icons by
  position, so the mod does not reorder anything.
* Built for the classic tray. With the Windows 11 taskbar the mod finds no
  classic popup and does nothing.
