# Tourne Menus

Tourne Desktop draws its own menus (the bar's, Start's, the desktop's) in
Tourne's look. This mod draws the right-click menus of other places the same
way, each with its own switch:

- **File Explorer windows**: files, folders, the empty space, the navigation
  pane. Windows 11's new menu is skipped for them, so the full menu shows at
  once (hold Shift to get Windows' own menu instead).
- **Open and Save dialogs**, in every program.
- **Windows' own menus**: the ones Windows would draw as "immersive" menus
  (Explorer's taskbar and desktop when Explorer is the shell, the volume,
  network, language and Windows Security icons, Task Manager, Store apps'
  title bar menus). They're made plain menus first, as Classic Menus does.

All three start off: turn on the ones you want.

What the menus look like, how they open and what's left out of them all come
from Tourne Desktop: its Menus, Menu frame, Menus open, Leave out of
right-click menus and Move into More options settings (Studio, Taskbar). It
writes them to `HKCU\Software\Tourne\Menus` whenever its settings apply, and
this mod reads them as each menu opens, so a change shows on the next menu.
Without Tourne Desktop, menus get Tourne's green scheme.

In every menu drawn here, the mouse wheel scrolls a menu taller than the
screen (3 items a notch).

Not drawn here: menus whose items draw themselves (a few programs' own), and
the icons some shell extensions draw themselves (Open with, Send to); those
items show without them. Hold Shift while right-clicking for Windows' own
menu.

The way Windows 11's new File Explorer menu is skipped follows m417z's
"Classic context menu" Windhawk mod (MIT); the idea for this mod came from
Bolt-Scripts' Custom Cached Context Menu (MIT). No code was copied from either.
