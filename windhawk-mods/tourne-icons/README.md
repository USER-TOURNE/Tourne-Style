# Tourne'Icons

Change any icon in Windows once, and it changes everywhere that icon shows.
It needs nothing but Windhawk.

## How to change an icon

1. Click **Tourne'Icons** in the tray (its picture icon). It opens the
   picker.
2. Pick what to change. The lists show what's on your **taskbar**, in your
   **tray**, on your **desktop**, in **Start**, in Settings' **Installed
   apps** and in **Control Panel**, plus **Windows icons** (folders, This PC, the Recycle Bin and
   every icon Windows ships) and **file types**. Type in the search box to find something quickly. Nothing
   needs a path.
3. Click **Choose new icon** and pick a picture: `.ico`, `.png`, `.svg`,
   `.jpg`, `.bmp` or `.gif`, or an icon inside another program. It's made
   into a proper icon at every size Windows uses (16 to 256 pixels).
4. Click **Apply everywhere**.

A program's new icon shows on the taskbar, in Start, in the tray and the
tray's overflow, on its windows, on the desktop, in Alt+Tab and in File
Explorer, because every one of those reads it from the program's file, and
this mod answers for that file in every program. A file type's new icon
shows wherever files of that type are listed.

**Your changes** lists everything you've changed; pick one and click **Put
back original** to undo it.

## Tourne Desktop's taskbar buttons

**Taskbar buttons** lists each open window's button and the pinned ones, the
way Tourne Desktop tells them apart: by the app id a program gives its
windows (a Chrome or Edge web app, Discord, a Store app), else by its file
name. A new icon there shows on that button on Tourne Desktop's taskbar, and
in its tray and Alt+Tab; nothing else changes (pick the program under **On
the taskbar** to change it everywhere). This is Tourne Desktop's Button icons
moved here: the first run takes in its rows (designs like `tourne:screenshot`
stay as they are), and while this mod runs, Tourne Desktop uses these and
leaves its own rows aside. A pack that changes a program changes its buttons
too.

## Many at once: Save lists and Apply a pack

**Save lists** (under the lists) writes every entry of every list, with its
icon as it shows now, to a new folder in Downloads: a picture per entry and
`pack.tsv`, which says what each picture is for. To make a set, draw new
pictures, point the first column of `pack.tsv` at them (or swap the pictures
in place), and click **Apply a pack** to put them all on at once. Rows with no
picture are skipped; Your changes lists everything it changed.

A pack keeps the icons you set already (in Your changes, the suite's Icon
Redirect or Tourne Desktop's button icons, by entry or by the file the icon
comes from) unless you say to replace them too; it asks first. Entries a pack
set aren't counted as yours, so a newer pack replaces them. A picture of `-`
puts that entry back, and rows saved from Your changes go back on as they
were.

## Good to know

- No admin is needed, and no Windows file is touched. Your choices and the
  icons made from your pictures are kept in this mod's own folder.
- Tray icons change straight away, and Explorer is told to refresh its icons
  straight away. A program that's already open shows its new icon on its own
  windows (and their taskbar buttons) once it's opened again. **Refresh
  icons** (in the picker, or on the tray icon's right-click menu) asks
  Windows again if one stays old.
- **Store apps** (Settings, Calculator, Photos, Claude and the like) change
  too: their logos are pictures in their packages, so every one of them
  (each size and variant the app ships) gets a picture made from your icon,
  and any program that opens the logo gets yours. That's the taskbar, Start,
  Alt+Tab, Settings' app list and File Explorer. Start is restarted once
  (it starts again by itself) so it lets go of the logos it had. A Store app
  update keeps your icon.
- Your tray list fills in as programs add their tray icons while this mod is
  running.
- If you used Tourne'Suite or Tourne Desktop before, the first run takes in
  the icons you set there (Button icons, and Icon Redirect's rows); turn the
  suite's Icon Redirect off afterwards so the two don't both answer for the
  same files. Without them there's nothing to take
  in, and nothing else changes.

## Icon themes and whole files (optional)

You don't need these to change icons; the picker does that. The settings below are for whole icon themes and for replacing whole files
(a theme DLL with the same resource ids as the file it stands in for, or a
folder of pictures named by resource id). Changes made in the picker win
over them.
