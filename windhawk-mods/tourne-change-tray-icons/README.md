# `Tourne'Tray` **[Change Tray Icons]**

Replaces every tray icon an application shows with one of your own.

A fork of **Change Tray Icons** by aubymori, with three changes.

## Applications that use the ANSI API

`Shell_NotifyIcon` has an ANSI form and a wide form, and the original hooked only the
wide one. An application built against the ANSI API was therefore invisible to it, with
no error and nothing in the log to say so.

Steam is one of these. Its tray icon is not created by `steam.exe` at all but by
`steamwebhelper.exe`, and that binary imports `Shell_NotifyIconA`. Both forms are hooked
here, so it no longer matters which one an application reaches for.

## Blank rows no longer truncate the list

The original stopped scanning at the first row with an empty application field. A blank
row is easy to leave behind when removing an entry, and every row after it was then
ignored, silently, with the settings page still showing them all. One stray blank near
the top of a long list could disable almost all of it.

This version scans the whole list and skips blanks, so a gap costs nothing. It also stops
at the first row that matches rather than the last, which only matters if you have two
rows for the same application, and first is the one you would expect to win.

## How to use

The mod only loads into applications you name, so each one has to be added to the
**Custom process inclusion list** on the **Advanced** tab, as well as getting a row in
the settings below. Both are required, and missing either one looks identical from the
outside: nothing happens.

Either a full path or a bare executable name will match. Restart the application
afterwards, since a tray icon already on screen is not redrawn until the application
sends it again.
