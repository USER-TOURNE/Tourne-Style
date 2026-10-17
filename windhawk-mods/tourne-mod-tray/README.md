# Tourne Mod Tray

Based on [Mod Manager Tray](https://github.com/Salyts) by Salyts (MIT).

Adds a tray icon (Segoe Fluent Icons glyph `E71D`). **Right-click** it to see
every installed Windhawk mod. A check mark (`E73E`) next to a mod means it is
enabled. Click a mod to toggle it.

A list taller than the screen scrolls with the mouse wheel (3 mods a notch)
while the pointer is over it, as well as with the arrows at its ends.

## How it works

The mod uses `windhawk-cli.exe`:

* `windhawk-cli.exe mod list` - get the list of mods (id, version, state, name)
* `windhawk-cli.exe mod enable <id>` / `mod disable <id>` - toggle a mod

Enabling / disabling requires administrator rights, so a UAC prompt appears
for every toggle (unless the Windhawk process itself is already elevated).

If `mod list` cannot be read without elevation, the list is fetched through an
elevated helper and cached; a **Refresh list** item then appears at the bottom
of the menu.

## Settings

* **Icon color** - auto / white / black
* **Menu color** - auto / light / dark
* **Show mod IDs** - append the mod id to the name in the menu
* **Tray icon** - hex code of a Segoe Fluent Icons glyph (default `E71D`)
* **Whitelist** - mod IDs to show; if empty, all mods are shown
* **Blacklist** - mod IDs to hide (takes priority over the whitelist)
