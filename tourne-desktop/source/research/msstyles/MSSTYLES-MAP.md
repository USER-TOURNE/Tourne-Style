# What a visual style themes, where it lands, and what's missing

Research for b24 (2026-09-29). Every class, part, state and property of eleven styles was read with msstyleEditor's own library, and every image was compared pixel by pixel with Windows' stock aero. Windows' own theme engine (uxtheme) settled how missing values are filled in.

## How it was done

- **Reading the styles.** `tools/msstyles/Dump-Msstyles.ps1` drives `libmsstyle`, the library inside msstyleEditor 2.1.2 (`.theme files\Claude Files to View for reference\msstyleEditor.exe`), from Windows PowerShell. It writes one row per property (`research/msstyles/<style>.tsv`) and exports every image (`<style>/img/`, kept out of git).
- **Comparing.** `tools/msstyles/Compare-Msstyles.ps1` compares a style with a base. Values are compared as text. Images are compared by their decoded pixels, since a PNG that was saved again has different bytes. Each class is tagged with the part of Windows it paints (`Msstyles-Surfaces.ps1`). Results are in `research/msstyles/compare/` and `compare-summary.txt`.
- **Asking Windows.** `tools/msstyles/resolve.cpp` loads a style file the way the suite does and asks uxtheme for the value it really gives and where it came from (`GetThemePropertyOrigin`).

| Style | Made by | Classes | Properties | What it is |
|---|---|---:|---:|---|
| aero | Microsoft (26200) | 374 | 11,096 | the base everything is compared with |
| Bouquet MAC - Night | niivu, 2025 | 371 | 11,132 | your Windows style |
| Bouquet - Night | niivu | 371 | 11,132 | same, Windows caption buttons |
| Bouquet MAC - Dark | niivu | 374 | 11,160 | the lighter dark variant |
| Bouquet | niivu | 345 | 9,380 | the older Windows 10 based build |
| mine | you | 371 | 11,144 | Bouquet MAC - Night plus your tooltip |
| Everforest Night | niivu, 2022 | 335 | 8,420 | a full Windows style |
| Everforest SAB | niivu, 2022 | 344 | 8,458 | a full Windows style used as a SAB style |
| Bouquet SAB | niivu, 2025 | 17 | 1,095 | a SAB-only style |
| Plain8, Windows 7 | Tihiy (SAB) | 17, 21 | 1,076, 780 | SAB's own styles |

## What a style holds

Every value has a type. Across Bouquet MAC - Night:

| Type | Count | What it does |
|---|---:|---|
| ENUM | 2,660 | how to draw: image or flat fill, stretch or tile, glyph type |
| INT | 2,503 | counts and sizes: images per strip, DPI thresholds, opacity |
| FILENAME | 2,134 | an image (one PNG per DPI step, states stacked in a strip) |
| COLOR | 1,473 | text, fill, border, glow colours |
| MARGINS | 1,187 | which edges of an image stay fixed when it stretches, and content padding |
| BOOL | 688 | transparent or not, mirror, and similar switches |
| FONT | 145 | a font by name and size |
| RECT | 136 | where each frame piece sits in the title bar atlas |
| POSITION, INTLIST, SIZE | 193 | offsets, animation lists, fixed sizes |
| DISKSTREAM | 3 | the title bar atlas (all window frame pieces in one image) |

Where it goes (Bouquet MAC - Night; `[dark]` marks the classes programs switch to in dark mode):

| Surface | Classes | Images | Colours | Who draws it |
|---|---:|---:|---:|---|
| Controls in programs | 35 | 513 | 153 | every Win32 program: buttons, edits, lists, tabs, scroll bars, progress, tooltips |
| Controls in programs [dark] | 22 | 312 | 154 | the same in dark mode (DarkMode_DarkTheme, DarkMode_Explorer) |
| Explorer, Open/Save, Control Panel | 132 | 304 | 270 | folder windows, address and search bars, nav pane, previews, Control Panel |
| the same [dark] | 65 | 534 | 212 | dark mode Explorer |
| Menus | 2 | 52 | 21 | every Win32 menu |
| Menus [dark] | 1 | 50 | 12 | DarkMode::Menu |
| Window frames: DWM | 3 | 0 (1 atlas) | 11 | every title bar and caption button |
| Window frames: classic | 3 | 60 | 27 | only with DWM off, MDI children and a few old apps |
| Taskbar and tray (classic) | 46 | 166 | 79 | Windows 10's taskbar; on Windows 11 nothing reads them from the Windows style |
| Start menu | 8 | 45 | 56 | Windows 7 Start classes; read by SAB and ours from the SAB style instead |
| Task Manager | 5 | 4 | 234 | the classic Task Manager's graphs |
| High contrast | 8 | 76 | 9 | only in high contrast mode |
| Theme-wide | 2 | 0 | 39 | system colours (these become Windows' own colours), fonts, sizes |
| Control animations (*Style) | 22 | 0 | 148 | the newer control looks and timings (ButtonStyle, TooltipStyle...) |

## How Windows fills in what a style leaves out (checked with uxtheme)

1. **A value missing from a state comes from its part, then its class, then the globals.**
2. **A derived class (`DarkMode_DarkTheme::Listbox`) missing a value takes it from its base class (`Listbox`) in the same style.** niivu deleted the dark listbox's hot fill; Windows then gives his own base fill #121C21, not Microsoft's #363636.
3. **A class missing altogether falls back to the plain class.** Everforest Night has no `DarkMode_DarkTheme::Button`; dark-mode programs get its plain `Button`, whose text is #D3C6AA. An app name the style has never heard of does the same.
4. **If the plain class is missing too, nothing opens.** Bouquet SAB has no plain `TaskBand2`, so asking for `TaskBand2CompositedSmallIcons::TaskBand2` gets nothing; only `Taskband2Composited::TaskBand2` works.
5. **A missing `StartPanelDark` falls back to plain `StartPanelPriv`,** which in the Windows 7 style is its light panel. Our Start menu checks that the panel part exists before taking a class, so Everforest SAB's one-property `StartPanelPriv` isn't taken by mistake.

So **"missing" usually means "inherited on purpose"**. It only goes wrong where the class it falls back to has the wrong look: a light base class, or Microsoft's stock look.

## How niivu builds his themes

- **Full Windows styles start from Microsoft's aero and repaint it.** In Bouquet MAC - Night about three in four colour values (1,113 of 1,473) and about two in three of the images Windows 11 actually shows were changed. No class Windows 11 uses is missing.
- **MAC and Windows variants differ only in the title bar.** Bouquet MAC - Night and Bouquet - Night are identical except 83 values of `DWMWindow` (the caption buttons).
- **He removes Microsoft's overrides so his base look applies.** 49 values are gone from derived classes (`Explorer::ItemsView`, `DarkMode_DarkTheme::Listbox` and `::Edit`, `AutoComplete::ComboBox`); by rule 2 they now take his base styling. About 90 values were added (coral and grey tab text, tiled toolbars, extra DPI images, a fill and text colour in the globals).
- **He blanks some images on purpose:** scroll bar arrow buttons (arrowless scroll bars), combo box dropdown backgrounds (flat), and the navigation pane's sizer. Never fill these.
- **He leaves Windows' own images where they already fit:** the grey glyphs of dark mode (dropdown, spin and sort arrows, expand buttons, submenu arrows, caption glyphs) look right on any dark background. That's most of the 111 stock images in dark controls, 220 in dark Explorer and 42 in dark menus; the few blue ones are listed below.
- **He doesn't touch what Windows 11 never shows from this file:** all 166 images of the classic taskbar and tray, 40 of the Start classes, the 60 classic frame images and high contrast are Microsoft's. The taskbar and Start come from the SAB style instead.
- **The palette is complete in `sysmetrics`.** These become Windows' own colours when the theme is applied, and yours match them except the two you changed:

| System colour | aero | Bouquet MAC - Night | Your registry now |
|---|---|---|---|
| Window, ButtonFace, Menu, MenuBar, Background | 255,255,255 / 240,240,240 | 18,28,33 (#121C21) | 18,28,33 |
| WindowText, ButtonText, MenuText, InfoText | 0,0,0 | 170,175,178 (#AAAFB2) | same |
| Highlight, MenuHilight, HotTracking | 0,120,212 | 240,105,111 (#F0696F) | same |
| ActiveBorder, 3D light and shadow, GrayText | greys | 56,73,80 (#384950) | same, except ButtonShadow 18,28,33 and ButtonHilight 63,106,90 |

- **Older builds miss Windows 11's new classes.** Everforest Night and the older Bouquet lack all 21 `DarkMode_DarkTheme::*` classes and 11 dark Explorer classes. By rule 3 dark programs get the plain classes. In these dark-only themes the plain classes are already dark, so nothing breaks. A light theme built the same way would show light controls in dark mode.

### SAB styles: two ways

- **Bouquet SAB is Tihiy's Plain8 repainted.** It has the same 17 classes as Plain8 (Windows 7 has 21, and no dark Start panel). niivu repainted the Start panel (128 images), the bar and the buttons. He kept Plain8's own images for the thumbnail previews (8) and the tray's show-hidden-icons arrow (20).
- **Everforest SAB is a whole Windows style** with the SAB classes added, using Windows 10's class names (`TaskBand2Composited::TaskBand2`, which uxtheme treats the same as `Taskband2Composited`). It has no dark Start panel.
- **A SAB style's `sysmetrics` are never applied to Windows.** Bouquet SAB's are Plain8's light values, which don't matter.
- **Menus, tooltips and the Start search box are never in a SAB style.** They come from the Windows style (already how ours works).

## Is anything unfinished in Bouquet MAC - Night, and should it be filled?

| What | Where you'd see it | Stock value | Fill it? |
|---|---|---|---|
| Removed overrides (49) | lists, edits, autocomplete | inherited on purpose | **No.** Rule 2 gives his own look. |
| Blank images (scroll arrows, combo buttons, sizer) | scroll bars, combo boxes | deliberately empty | **No.** That's the design. |
| Grey glyphs in dark classes (~370) | arrows, expanders, caption glyphs | Microsoft's greys | **No.** They fit any dark palette. |
| Missing Windows 11 dark classes (older themes only) | dark dialogs | falls back to the plain class | **Only for light themes.** Dark Magic already covers this at runtime. |
| Calendar selection cells (MonthCal) | date pickers, the clock's calendar in some apps | Windows blue #016BD4, #0063AF | **Yes, tint to the accent.** |
| Dark list expand buttons (`DarkMode_DarkTheme::ListView`) | grouped lists in dark dialogs | blue-grey #487F9B | **Yes, tint.** |
| Blue text and fills in dark Explorer (`DarkMode::ItemsView` PROPERTY text 35,151,201 and 58,153,232; `DarkMode_Explorer::ItemsView` fills 12,59,123 and 23,102,165; `DarkMode::CommandModule` split text 30,57,91) | property values in the details pane, command bar | Windows blues | **Yes, swap for the accent.** |
| Light leftovers in light classes (`ProperTree` header #CBE0EB, `Flyout` #F1F1F1, `Address::ComboBox` dropdown) | rare old surfaces | light | **Optional;** only where they show up. |
| Task Manager graph colours (CPU blue, memory purple, orange contention) | classic Task Manager | Microsoft's | **No.** They're data colours; changing them loses meaning. |
| Search hit yellow, warning colours | search results, dialogs | semantic | **No.** |
| Classic taskbar, tray, Start, frame, high contrast images | not shown on Windows 11 | Microsoft's | **No.** Nothing reads them there; the SAB style covers the taskbar. |

**In short: a finished niivu theme needs almost nothing filled.** Fill only where Windows' accent blue or a light background was left in a place that shows. Leave removed values, blank images, grey glyphs and data colours alone.

## What SAB styles define that our bar doesn't read yet

| Class | What it is | Ours |
|---|---|---|
| `TaskbandExtendedUI` (parts 1-3, 5, 8-12) | thumbnail preview popups: frame, hover, buttons, close | our own palette popup |
| `TrayNotifyHoriz/Vert(Open)::Button` | the show-hidden-icons arrow, per DPI, open and closed | System Icons' arrow |
| `TrayNotify::Clock`, `TrayNotify::Toolbar` | clock and tray hover background, clock text colour | clock.* settings |
| `ShowDesktop::Button`, `VerticalShowDesktop::Button` | the show desktop sliver at the end of the bar | none yet |
| `Taskbar2Composited::TaskBar` part 5 | a second bar background with 4 px content margins; probably the segment islands (unverified) | not used |
| `Taskband2Composited::TaskBand2` part 5 | holds 10 images (states 8 to 10 unknown); we use 7 | 7 states |

These matter more for the 1:1 StartAllBack goal than filling gaps in the Windows style.

## What this means for b24

A theme-wide palette layer that overwrites colours would fight niivu's work. On this machine, title bars (the DWM atlas), system colours and menus are already his. What's worth building:

1. **Palette tokens from the style's own system colours** (`GetThemeSysColor` on the style file), so every full theme gives a complete palette even without a Menu class. Cheap: read once per theme change.
2. **Leftover accent swap:** when a non-stock style is active, Windows' own accent blues listed above become the style's Highlight colour at the point they're read (`GetThemeColor`), and the calendar cells are tinted. Off by default, since it changes what you see.
3. **No forced title bar colours, no SetSysColors, no mode pinning by default.** The style already does all three. Keep them for YASB-only setups without a visual style.
4. Separately, and more useful for the SAB goal: read the SAB style classes in the table above.
