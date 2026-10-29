# Tourne'Icons Kit

Tourne'Icons Lasting as a program of its own, without Windhawk.

- Open `TourneIconsKit.exe`, change icons (or **Apply a pack**), close it.
  Nothing of it runs after that and nothing starts with Windows.
- The changes are written where Windows keeps them: shortcuts (Start, the
  desktop, pinned taskbar buttons), file types, and Windows' own icon
  settings (folders, drives, discs, This PC, the Recycle Bin, user folders
  and the other stock icons). So they stay.
- Opening it again puts your icons back on shortcuts made or remade since
  (an app installed, pinned or updated), and **Your changes** → **Put back
  original** undoes any change.
- Folders', drives' and the other stock icons are set for every account, so
  Windows asks to let it run as admin when it opens.
- Store apps' logos, programs' own windows and tray icons, and icons with no
  Windows setting need Tourne'Icons (the Windhawk mod) running instead; the
  picker says so on each.

Its list, icons and what each change replaced (`lasting.txt`) are kept in
`%LOCALAPPDATA%\Tourne\Icons Kit`.

## Building

`kit.cpp` is `tourne-icons.wh.cpp` built with `TOURNE_ICONS_KIT 1`, plus
stand-ins for Windhawk's few functions. Edit `tourne-icons.wh.cpp`, not a
copy. With MinGW-w64 (Linux or MSYS2):

    tools/icons/kit/build.sh [output folder]

`kit.ico` is drawn by Tourne's pack-maker (bright palette).
