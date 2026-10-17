# Tourne Menu: a full LiteMenu replacement

Asked by Tourne on 2026-10-08: build LiteMenu (PopEx) into Tourne as a full replacement, and widen it with Tourne Desktop's own functions.

## Why

LiteMenu only opens over Explorer's desktop, which shell mode hides. Step 1 (0fb7727) showed LiteMenu's menu on Tourne's desktop from its own config.xml, drawn by host code. The full replacement moves the menu into the taskbar module, where Tourne's styled menus, palette, Start menu data, desktops pager and actions already live.

## What LiteMenu does, and where it goes

| LiteMenu | Tourne Menu |
| --- | --- |
| Menu tree (submenus, items, separators) in config.xml | `taskbar.menu.items[i]` rows: depth, name, command, args, so the Studio edits it like Button icons. With no rows, LiteMenu's config.xml is read live (nothing to import by hand). |
| Item command + parameters, shell apps (`IsShellApp`) | The same; shell apps open as `shell:AppsFolder\<id>`. |
| Built-in commands: lock, restart, shut down, settings, close | `tourne:` actions (below); LiteMenu's own tags map onto them. |
| Mouse button (middle, right), desktop only | `taskbar.menu.button` (middle, right, none); on the desktop's empty space, and on the bar's empty space with `taskbar.menu.onBar`. Shift + right-click keeps Windows' own desktop menu. |
| Hotkey | `taskbar.menu.hotkey` (like `Win+Alt+M`), none by default. |
| Its colours (ground, text, highlight, frame) | Tourne's menu look (the palette, frames, pixel font, Menu No Icons). |
| Icons on or off | Menu No Icons decides, as for every Tourne menu. |

## Tourne's own (the widening)

Actions, as an item's command `tourne:<name>`: `show-desktop`, `run`, `start`, `task-manager`, `windhawk`, `studio`, `settings`, `lock`, `sleep`, `sign-out`, `restart`, `shutdown`, `desktop-next`, `desktop-previous`, `desktop-new`, `desktop-close`, `desktop:N`.

Live submenus, as a submenu row's command: `tourne:windows` (open windows; a click brings one up), `tourne:desktops` (virtual desktops; a click goes there), `tourne:programs` (Start's All Programs tree), `tourne:recent` (recent items), `tourne:folder:<path>` (a folder's contents).

## Stages

1. Done (0fb7727): LiteMenu's menu on Tourne's desktop, host code.
2. Done (tourne-menu.cpp): the module menu: rows or LiteMenu's file, Tourne's look, `tourne:` actions and live submenus, desktop and bar activation, hotkey. Host code only forwards the click. LiteMenu itself can then be closed or uninstalled (Tourne's call).
3. Done (2026-10-12): not a Studio page but its own window laid out like LiteMenu's configurator (desktop/host/menu-editor.cpp): the rows as a tree (add, remove, move, drag into submenus), Properties, Activation, "Copy LiteMenu's menu", pickers for `tourne:` actions. The Studio shows the rows as one row that opens it.
4. Later: more than one menu (per place: desktop, bar, a screen corner), switcher views as actions (they're in the switcher module).
