Tourne Tray Icons for Windhawk Resource Redirect
================================================

Apply Tourne Tray Icons.cmd          Apply the sage icons (the design default).
Apply Tourne Tray Icons (Ink).cmd    Apply the ink tone instead.
Apply Tourne Tray Icons (Alt).cmd    Apply the alt tone instead.
Apply Tourne Tray Icons (Ink no shading).cmd
                                     Ink without the one-pixel shade under each icon.
Revert Tourne Tray Icons.cmd         Take them all back off.

Each one asks for administrator rights once (Windhawk keeps mod settings under
HKLM), then restarts Explorer and the Windows Security tray icon so the icons
load. Applying another tone swaps the tone; it never stacks.

How it works
------------
packages\<tone>\ holds resource-only DLLs named after the Windows files they
stand in for. Each contains only the icons in mapping.csv, under the same
resource ids Windows uses. Apply adds one Resource Redirect entry per file
(System32 and SysWOW64) at the end of "Redirection resource paths". Last
entries win, so these icons override your Bouquet theme and your existing
per-file redirects for the same files. Any icon not in mapping.csv still comes
from wherever it came from before.

Reverting
---------
Revert removes exactly the entries Apply added and leaves everything else
alone. Every run also exports the settings to backups\ first, and
backups\before-first-apply.reg is the state before the very first Apply.
To put that whole snapshot back:

  powershell -ExecutionPolicy Bypass -File TourneTrayIcons.ps1 -Action Restore

If the Windhawk settings page for Resource Redirect is open while you apply or
revert, close and reopen it before saving anything there, or it will write its
old copy back.

Not covered, and why
--------------------
Network: StartAllBack draws its own network icon from the Segoe Fluent Icons
  font, not from an icon resource, so no redirect can reach it.
Battery: batmeter.dll builds the battery meter at runtime from layered sprite
  strips, not per-state icons, so there is nothing to swap one-for-one.
Bell, focus, night light, brightness, pin, more, folder, download, the
  Hidden apps set and the other flyout glyphs: StartAllBack and Windows 11
  draw these from fonts or have no tray icon resource. The glyphs that do have
  a Windows counterpart (info, warning, error, close, accessibility) are
  included.
