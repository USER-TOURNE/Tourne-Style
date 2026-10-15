# Tourne Drop Install

Install a mod from its `.wh.cpp` file by dropping it on a window, instead of
Create a new mod, turning off Enable mod, pasting the code in, Compile mod and
closing the editor.

## How to use it

1. Click **Tourne Drop Install** in the tray (a green arrow). A small window
   opens.
2. Drag one or more `.wh.cpp` files (or a folder holding them) onto it, or
   click **Choose files**.
3. Each one is compiled and installed, one after another, and the window
   says how it went (with the compiler's messages when one doesn't compile).

A mod you didn't have before is installed **turned off**, as Enable mod off
in the editor does; turn it on in Windhawk when you're ready (or turn on
**Turn new mods on** in this mod's settings). A mod you already had is
updated in place and keeps its settings and its on or off state.

They're installed as your own mods (as Create a new mod makes them), so
they're under Installed mods with the rest, and you can open them in the
editor as usual.

## Good to know

- It needs Windhawk 2, which comes with the `windhawk-cli.exe` it uses to do
  the work (the same steps the editor's Compile mod takes).
- Windhawk runs it in a process of its own, as administrator when it can,
  since installing a mod writes to Windhawk's own folders. If it can't, each
  install asks for administrator once (the usual Windows prompt).
- Nothing runs while the window is closed and nothing is dropped.
