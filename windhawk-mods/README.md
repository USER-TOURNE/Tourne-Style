# Windhawk mods

Every Windhawk mod I've made, newest version only. Each folder holds the mod's `.wh.cpp` and its README (taken from the mod's own readme block).

**Install one:** in Windhawk, **Create a new mod**, paste the file, **Compile mod**. On Windhawk 2 you can also run `windhawk-cli mod install --file <path to .wh.cpp>`, or drop files on [Tourne Drop Install](tourne-drop-install/).

## Use together (with Tourne Desktop)

| Mod | Version | What it does |
|---|---|---|
| [Tourne'Suite](tourne-suite/) | b55 | Every Tourne part that hooks Windows, in one mod, each part a switch: System Icons and the Explorer relay, Change Tray Icons, Tray Folders, Clock, System Text Color, Menu No Icons and No Duplicates, Classic Menus, Icon Redirect, Explorer Style and Colors, Theme Gaps, Dark Magic, Tweaks, plus the older test Taskbar and Switcher (Tourne Desktop has the current ones). |
| [Tourne Menus](tourne-menus/) | 1.0.6 | Explorer's, the file dialogs' and Windows' menus in Tourne Desktop's look. |
| [Tourne'Icons](tourne-icons/) | 2.7.2 | Change any icon everywhere from a picker in the tray; applies icon packs. Comes with **Tourne'Icons Lasting** 1.7.1 (no hooks, changes stay with the mod off) and **Tourne'Icons Kit** (the same as a plain program, no Windhawk). |
| [Tourne'Table](tourne-table/) | 2.1.1 (2.1.0 in the catalog, 2.1.1 in review) | The desktop audio visualizer, ported out of Tourne Desktop as a mod of its own, with community styles. The catalog has 2.1.0; 2.1.1 is in review. Screenshots and GIFs are in [`media/`](tourne-table/media/). |

## Standalones that are also suite parts

Run either the standalone or the suite with that part on, not both.

| Mod | Version | What it does |
|---|---|---|
| [Tourne'Tray [System Icons]](tourne-tray-system-icons/) | 1.8.3 | Pixel-art tray icons with live state (network, volume, battery, Bluetooth, security, VPN), each with a Tourne flyout. |
| [Tourne'Text [System Text Color]](tourne-system-text-color/) | 1.8.1 | Recolours white system text, Chrome's menus included. |
| [Tourne'Tray [Change Tray Icons]](tourne-change-tray-icons/) | 1.1.0 | Swap any app's tray icon for your own. |
| [Tourne'Menu [No Icons]](tourne-menu-no-icons/) | 1.0.0 | Context menus without icons. |
| [Tourne'Menu [No Duplicates]](tourne-menu-no-duplicates/) | 1.0.0 | Removes doubled context menu entries. |
| [Tourne'Tray [Classic Tray Folders]](tourne-classic-tray-folders/) | 1.0.0 | Group tray icons into folders in StartAllBack's classic tray. |
| [Tourne'Tray [Clock]](tourne-tray-clock/) | 1.1.0 | Restyles StartAllBack's classic clock. |
| [Tourne'Icons [Icon Redirect]](tourne-icon-redirect/) | 1.0.1 | Icon themes by redirecting resources (an MIT rewrite that replaces Resource Redirect). Superseded by Tourne'Icons. |
| [Tourne'Suite Tweaks](tourne-tweaks/) | 1.0.0 | Winaero Tweaker's options as switches. |

## On their own

| Mod | Version | What it does |
|---|---|---|
| [Tourne Drop Install](tourne-drop-install/) | 1.0.0 | A small window: drop `.wh.cpp` files on it to install them. |
| [Tourne Mod Tray](tourne-mod-tray/) | 1.0.1 | A tray icon with a right-click list of your mods to switch on and off. |
| [Hide the app icon in taskbar thumbnail previews](taskbar-thumbnail-hide-icon/) | 1.4.0 | Removes the small app icon from taskbar previews. |

## Dev tools

| Mod | Version | What it does |
|---|---|---|
| [Tourne HID Watch](dev-tools/) | 1.5 | Read-only logger of what Alienware Command Center sends to its devices. Kept for reference; not for everyday use. |

## Notes

- The suite's file keeps its build id (`tourne-suite-b55`), so it installs next to older builds instead of over them. Run one suite at a time.
- Tourne'Table 2.1 is the copy under review for the Windhawk Explore catalog ([#6269](https://github.com/ramensoftware/windhawk-mods/pull/6269)); the catalog offers 1.x until it's merged. It takes the public id `tourne-table-desktop-audio-visualizer`, so it replaces the 1.x release. Turn off any 2.0 beta copies.
- Defaults that pointed into my own folders (icons, themes, pictures) were blanked for sharing; set your own in each mod's settings.
