# Package logos, suite b54 installed, Studio pickers

When: 2026-10-08, about 09:00 to 10:30, Claude Code on the desktop PC, branch `claude/project-thread-0rkfwp`.

Why (Tourne): Icon Redirect should reach packaged apps' logos too (Task Manager's Claude row); Studio settings should be easy to grasp, with choices shown instead of text boxes; apply everything so only a restart is needed.

What changed: d24a17a, f8418e4, 2e72d34, df43154 (details in DEVLOG, last section). Suite b54 installed and on, b53 off but kept. Tourne Desktop swapped to df43154.

Found on the way:
- windhawk-cli can install (`mod install --file --disabled --yes`), enable, disable and `config set excludeCustom`, but not `exclude`: that comes from the source's `@exclude` at every compile.
- In an Install script, don't name a helper `Cli` (Clear-Item's alias).
- MinGW lacks WICBitmapInterpolationModeHighQualityCubic; Fant is used.

Not verified on screen: pickers clicked through, logos in Start/switcher. To go back: enable b53 in Windhawk, disable b54.
