# Tourne'Icons [Icon Redirect]

Loads icons (and, if you like, other resources) from theme files instead of
the Windows files they replace, so nothing under C:\Windows is touched.

* **Theme files.** A resource-only DLL or EXE carrying the same resource ids
  as the file it stands in for. Only the resources it has are replaced; the
  rest still come from the original.
* **.ico files.** Replace a program's main icon, the one Explorer and the
  taskbar show.
* **Packaged apps** (from the Store or an MSIX installer, like Claude). Windows
  shows their package's logo pictures instead of the program's icon, so a row
  for the program (`*\Claude.exe`) with a picture (.ico, .png, .svg) stands in
  for its logos too, in Task Manager, Start and the taskbar. A row can also name
  a logo picture itself. The picture is drawn once at each size Windows asks
  for and kept in this mod's storage folder.
* **Folders of pictures.** Pictures named by resource id (`123.png`,
  `1123.bmp`, `2123.svg`) stand in for the file's IMAGE, PNG and BITMAP
  resources with those ids. PNGs are used as they are; anything else is
  converted once, the first time it's asked for (an .svg is drawn at the size
  of the picture it replaces).
* **Windows branding.** The logo in About Windows (winver), the shut down
  dialog and System comes from `Branding\Basebrd\basebrd.dll`. Point *Windows
  branding pictures* at a folder of pictures named 123, 1123 and 2123 (the
  logo at its three sizes, as niivu's theme packs name them).
* **theme.ini.** A folder with a `[redirections]` section of
  `original=replacement` lines, the replacement relative to the folder.
* **Icon themes.** The community themes collection. A theme is downloaded
  once, by Windhawk's own process, the first time you pick it, and loaded from
  disk after that. Nothing else in this part uses the network.

Later rows win over earlier ones, and rows win over themes.

Explorer keeps an icon cache, so icons it already showed can stay as they were
until it restarts.
