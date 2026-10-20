# Icon packs

See every icon in every pack on the [icon packs page](https://user-tourne.github.io/Tourne-Style/icons.html). After changing a pack, run `python icon-packs/make-previews.py` to rebuild its previews.

| Pack | What's in it | How to use it |
|---|---|---|
| [Tourne ink pack](tourne-ink-pack/) | The complete ink set: 1,078 entries (programs, Store apps, Windows' own icons, file types, and Tourne Desktop's taskbar buttons), filled in my dark green. Pack 5 with every add-on and the Taskbar buttons set folded in, so there's nothing extra to apply. 50 apps (Terminal, Chrome, Edge, VS Code, Discord, Windhawk and more) get full-size drawings like the Snipping Tool scissors, in `icons/full-size/`. | Tourne'Icons 2.7+: **Apply a pack**, open `pack.tsv`. Say yes when it asks about icons you set yourself to keep them. |
| [Tourne red and green pack](tourne-red-green-pack/) | The earlier red and green pixel set: 915 entries. | The same. `pack (keeps yours).tsv` isn't needed: Tourne'Icons asks. |
| [Tourne tray icons](tourne-tray-icons/) | The pixel tray icons: 91 designs in sage, ink and alt tones, as PNG (1x to 8x) and 8-size ICO (`art/`), plus resource-only DLL packages for Resource Redirect with one-click Apply and Revert (`resource-redirect/`). | Tourne'Tray [System Icons] and the suite draw them live; the packages are for Resource Redirect users. |
| [Tourne Start orbs](tourne-start-orbs/) | Start buttons in the Start Mark designs (bar, stack, level, split) as StartAllBack orb strips, 54 x 162 at 96 DPI, at 16 px, 1x, 2x and 3x, plus a null orb. | StartAllBack's Start button picker, or copy them into %LOCALAPPDATA%\Tourne\Orbs for Tourne Desktop's taskbar. |
| [Tourne button icons](tourne-button-icons/) | Taskbar button icons in the Bouquet taskbar set's style: Claude, GitHub Desktop, Settings, Snipping Tool. | Tourne Desktop's Button icons, or any icon picker. |

## A pack file

`pack.tsv` is a tab-separated list (UTF-16), one icon per row: `picture`, `kind` (program, package, icon, filetype, button), `original` (what it replaces: a program, a Store app's folder, `file,index`, a file type), `name`, `progid`, `list` and `shows-from`. Pictures sit beside it in `icons/`. Tourne'Icons can write one too (**Save lists**), so you can make and share your own.

## Left out

Rows that only pointed at files on my PC (Tourne'Icons' own cache, Downloads) can't match on anyone else's, so they were dropped: 85 from the ink pack and 34 from the red and green pack.
