# 06. Icons, menus and packs

*October 8 to 12, 2026.*

## Tourne'Icons: change any icon, once

Changing an icon on Windows means knowing whether it lives in a shortcut, a DLL, a registry key, a Store app's manifest or a tray call. I wanted: pick the thing you can see (in the tray, on the taskbar, on the desktop), pick a picture, done, everywhere.

**Tourne'Icons** is a Windhawk mod with a picker in the tray, built on Icon Redirect's engine:

- **Any picture in.** .ico, .png, .svg, .jpg, an icon inside a program. Each becomes an .ico with ten sizes, scaled from the nearest larger frame, or blown up pixel for pixel when the size divides evenly, so pixel art stays crisp.
- **Everywhere.** The program's own icon, its tray icon (swapped before Windows ever sees it), one icon inside a Windows file (Folders, This PC), file types.
- **Store apps.** Their logos are a pile of qualified PNGs named in the app's manifest, some inside resource packages. Tourne'Icons makes a picture of every one at its own size and answers the file opens for them. Copilot was the odd one out: a "sparse" package whose folder only holds the manifest, with the logos somewhere else.
- **Live, no admin.** Your list lives in the mod's own storage folder. Every program waits on that folder (no polling) and picks up a change as soon as it's saved.

Then it split into three, because people want different things:

- **Tourne'Icons** (live, hooks).
- **Tourne'Icons Lasting:** no hooks at all. It writes the changes Windows keeps by itself (shortcuts, Windows' icon settings, file types), so they stay with the mod off. Generated from the live mod's source, never edited by hand.
- **Tourne'Icons Kit:** the same as a plain program. Open it, change things, close it. Nothing at startup. Reopening it puts icons back on shortcuts an updater rewrote.

### Things that bit

- **"That couldn't be saved"**, sometimes. The list is written next to the old one and moved over it, and that fails while a virus scanner or the search indexer holds either file. It retries now and tells you Windows' actual reason when it still fails.
- **A pack replaced my own icons.** Packs now work out what's *yours* first (your own rows, the suite's redirects, Tourne Desktop's button icons) and ask whether to keep them.
- **Every tray icon of Tourne Desktop became the T**, because a pack row for its program swapped every tray icon it put up. A program's tray icon is only swapped while it has exactly one.
- **The 32-bit build failed:** a lambda passed where a 32-bit `stdcall` callback was needed. Again.
- **Windhawk's own window stopped opening.** Loading rules looked up every Store row's outside folder, in every program as it starts, Windhawk's WebView2 included, and that held it up. The lookups now happen once, when the pictures are made.
- **About half the pack didn't apply.** The record of loaded replacements held 128 entries per program; the pack sets several hundred. It grows now.

## The ink pack

Then I wanted every icon on the system redrawn in ink: a dark green fill, a bright green line and a red accent.

- The first version had look-alike icons told apart by **small letters and numbers in the corner**. I called that lazy, and it was. Every look-alike now gets a drawing for what it's actually used for: discs with their real labels (DVD-R, CD-RW, BD), BitLocker drives with an open or closed lock, printers as default, network, to file and fax, Command Prompt and PowerShell with an admin version, Office templates dashed, macro files with a "!". The build stops if two entries would still look the same.
- **Video thumbnails** were covered by a green film. Explorer lays icons 193 and 194 over a video thumbnail: film edges with a **see-through** middle. The pack had drawn whole films. They're now Windows' own frames, recoloured, with the middle still clear.
- **Folder previews in the Open dialog broke**, because Windows draws a folder showing its contents from two icons (the back with the tab, and the front flap) with the pictures between them. They have to line up on the same grid.
- **Pack 5** drew *everything*, Chrome and Settings included: 1,125 rows. Then three add-ons (Windows icons left out before, the video frames, Installed apps and Control Panel) and the taskbar buttons set.

In this repo it's **one pack**, with the add-ons folded in.

## Tourne Menus

Tourne Desktop draws its own menus in its own look. Explorer's right-click menus, the file dialogs and Windows' own menus are other programs, so **Tourne Menus** is a Windhawk mod that draws them in the look Tourne Desktop publishes: the same frame, the same rows, the same wheel scrolling, the same hide and "More options" rules, with an optional opening animation on the motion engine. Shift + right-click still gives you Windows' own.

Along the way:

- **The menu frame** became settings: StartAllBack's line, band, line, slimmed down to a 1 px gap and a gradient.
- **Menus felt laggy for the cursor.** The wheel-scrolling hook was a low-level mouse hook on the menu's own thread, so every mouse move waited for the menu to paint. Same rule as the switcher, learned twice: the hook lives on its own thread now.
- **A menu editor laid out like LiteMenu's:** a tree of items with drag and drop and a Properties panel, because a flat list of "Items 1, Items 2..." with depths was unreadable.

## Tourne Drop Install and Tourne Mod Tray

- **Tourne Drop Install:** drop `.wh.cpp` files on a small window and they're compiled and installed, using Windhawk 2's own command line. New ones arrive switched off.
- **Tourne Mod Tray:** a tray icon with every mod in a menu, click to switch one on or off. With about 60 mods the menu was taller than the screen, so the wheel scrolls it. It didn't, on my PC: the tray click came through the taskbar, so the menu's thread wasn't in front and the wheel never reached it. A low-level hook while the menu is open fixed it.

**Where they are:** [`windhawk-mods/tourne-icons/`](../windhawk-mods/tourne-icons/) (with the Kit), [`windhawk-mods/tourne-menus/`](../windhawk-mods/tourne-menus/), and the packs in [`icon-packs/`](../icon-packs/).
