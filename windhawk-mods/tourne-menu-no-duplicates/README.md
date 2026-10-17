# `Tourne'Menu` **[No Duplicates]**

Removes the extra copy of a menu item that appears twice, so each entry shows once.

Some applications register their context menu through two mechanisms at once. Convertify
is the case this was written for: it installs classic registry verbs and also a packaged
shell extension, and on this machine Explorer draws both, giving two identical
`Convertify` submenus. Hiding the registry verb with `ProgrammaticAccessOnly` or
`LegacyDisable` does not help here, because the menu Explorer shows ignores both.

## How

Every popup menu in a process is shown through `NtUserTrackPopupMenuEx` in win32u, which
is where user32's `TrackPopupMenu` and `TrackPopupMenuEx` end up. This mod looks at the
menu just before it opens, when every top-level item is in place, and deletes the extra
copies. If that export is missing it hooks the two user32 calls instead.

Doing it at that moment, rather than as items are inserted, is deliberate. The item
inserted second is not necessarily the one lower down the menu, because the shell inserts
by position, so only a finished menu can say which copy is first and which is last.

## What counts as a duplicate

Two **top-level** items whose text reads the same: compared without the `&` accelerator
marker, without the shortcut column after a tab, without zero-width characters, and
ignoring case. They must also agree on whether they open a submenu, so a plain command
and a submenu that happen to share a name are left alone.

**Items** limits this to the names listed. `*` applies it to any item that appears twice,
which is the broader hammer.

**Keep** chooses the survivor. `last` keeps the lower copy, which for Convertify is the
packaged shell extension; the upper one is the registry verb.

## What it leaves alone

Submenus, separators, owner-drawn items and bitmap items are never touched. The Windows
11 modern context menu is XAML and never goes through these functions, so this only
affects the classic menu, which this machine already uses.

## Scope

**Processes** decides where the mod loads, as a list of executable names. Explorer covers
the desktop and folder windows. Add an application's executable to cover the file browser
inside its Open and Save dialogs.
