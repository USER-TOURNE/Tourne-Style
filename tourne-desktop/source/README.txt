Tourne'Suite (redirect test, wintweaks branch)

This folder builds the wintweaks branch (mod id tourne-suite-redirect-branch-wintweaks),
the build in use. ..\tourne-suite-redirect is left as it was, matching the older
redirect mod, which stays installed but off.
============================

A separate test build: the split test build plus Icon Redirect, our own
replacement for the Resource Redirect mod, as a part in the suite's settings.
The split build in ..\tourne-suite-split and the mod I have installed from
it are untouched; this one has its own id (tourne-suite-redirect).

tourne-suite-redirect-branch-wintweaks.wh.cpp  The test mod. Generated; don't edit it.
tourne-icon-redirect.wh.cpp   Icon Redirect's source (MIT, written from
                              scratch). Also works as a mod on its own.
suite-redirect.template.cpp   The suite code, with Icon Redirect wired in.
Build-TourneSuiteRedirect.ps1 Regenerates the mod. Icon Redirect's defaults
                              are Resource Redirect's current settings (my
                              Bouquet theme, 4 theme paths and 71 rows), the
                              other parts' are the installed split build's.
tourne-suite-redirect-branch-wintweaks-settings.reg
                              The same defaults, imported by the switch
                              script if Windhawk kept only the first row of a
                              list.

What Icon Redirect does
-----------------------
Hooks FindResourceExW, FindResourceW, LoadResource and SizeofResource, which
is how Windows' own icon code reads icons. A lookup in a file with rules tries
the replacement files first (newest rule first), then the file itself.
  * Theme DLLs/EXEs: only the resources they carry are replaced.
  * .ico files: replace the program's main icon (its first icon group).
  * Loose .ico/.png/.cur files a program opens by path are swapped too.
  * "All resource types" off limits it to icons, cursors and images.
Icon themes picked from the list download once, in Windhawk's own process,
into the mod's storage. A theme Resource Redirect already downloaded is copied
from its folder instead, so my Bouquet theme needs no download.

Trying it
---------
1. Paste tourne-suite-redirect-branch-wintweaks.wh.cpp into a new Windhawk mod and compile it.
   Close the Windhawk window.
2. Double-click "Switch to redirect test build.cmd". It turns off the split
   build, Resource Redirect and the other Tourne mods, and turns this on.
3. Restart Explorer (or sign out and in) so it drops icons it cached.
4. "Switch back from redirect test build.cmd" puts back exactly what was on
   before step 2 and turns this build off.

What to check
-------------
* Tray icons: volume, Windows Security, USB, Bluetooth in the Tourne pack.
* Explorer: Bouquet folder, drive and zip icons.
* Taskbar and Start: Chrome, Steam, Notepad, Task Manager, HWiNFO and
  Windhawk with their Bouquet icons, both pinned and while running.
* A 32-bit program's file dialog shows the same Bouquet icons.
* Settings > Icon Redirect: pick a theme you haven't used before, and it
  downloads once; programs pick it up when they restart.

Tweaks (redirect.4)
-------------------
The Winaero Tweaker 1.65 options that are a setting, as the suite's Tweaks
group (276 settings in 22 groups). Pages that only run a tool, make a shortcut
or link to a download aren't included: Windhawk settings can't be buttons.
Options Windows already offers in Settings, Folder Options, Windows Security
or Edge's own settings are left out; each group's description says where.

* tweaks.def lists each tweak and the registry values or live calls behind
  it; Build-Tweaks.ps1 turns it into tourne-tweaks.wh.cpp, with each default
  read from this PC, and tourne-tweaks-settings.reg. The main build runs it.
* Tweaks runs in Windhawk's own process. Each group has a "Use these
  tweaks" switch, off to begin with; nothing in a group is applied while it's
  off. The first time it sees a tweak (or its group is switched on) it only
  records it; after that, each save applies what changed. A save changing
  more than 5 tweaks asks first, and so does anything changed while Windhawk
  wasn't running.
* Changes for your account are written straight away; window metrics, fonts,
  snapping, focus-follows-mouse, menu delay and similar apply live.
* Machine-wide changes go into one elevated step per save (one UAC prompt).
  Declined or failed ones are logged and left until changed again; at
  start, tweaks needing admin wait for a save.
* Context menu entries Winaero added (Restart Explorer, Task Manager, MSI
  Extract, Advanced system properties) are taken over: turning one off
  removes Winaero's copy too.

Turning it on: Windhawk doesn't fill in defaults for new settings of an
installed mod, so after compiling, double-click tourne-tweaks-settings.reg,
then turn the suite off and on. Nothing changes until you save a setting.
