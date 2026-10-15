# Tourne

Linux-level customizing for Windows, with no phoning home. Everything Tourne has made, in one place: **Tourne Desktop**, every **Windhawk mod** (latest versions only), the **icon packs**, the **Tourne'Table community styles**, and the devlogs that explain how it all got here.

| | What it is | Where |
|---|---|---|
| **Tourne Desktop** | The taskbar, Start menu, switcher with tiling, system icons and flyouts, Studio, plugins and the built-in Tourne'Table, as one program. On Tourne's PC it runs as the Windows shell. | [`tourne-desktop/`](tourne-desktop/) |
| **Windhawk mods** | 17 mods: the Tourne'Suite (all hook parts in one), Tourne'Table 2.1, Tourne'Icons, Tourne Menus, and the standalones. | [`windhawk-mods/`](windhawk-mods/) |
| **Icon packs** | The complete ink pack (add-ons folded in), the red and green pack, the pixel tray icons, Start orbs. | [`icon-packs/`](icon-packs/) |
| **Community styles** | Tourne'Table styles people share, with a gallery and a bot that takes them in. | [`community/`](community/) |
| **Devlogs** | The full devlog, the handoffs and every session note. | [`devlogs/`](devlogs/) |

## Start here

1. **Want the whole look?** Download `tourne-desktop/release/TourneDesktop-<commit>.zip`, unzip it anywhere, run `TourneDesktop.exe`. Its tray icon opens Tourne Studio, where every part is a switch. Then add the Windhawk mods in the "Use together" list below for the parts that have to live inside Explorer.
2. **Want one piece?** Each folder in `windhawk-mods/` is one mod with its own README. In Windhawk, choose **Create a new mod**, paste the `.wh.cpp`, then **Compile mod** (Windhawk 2 also takes `windhawk-cli mod install --file <path>`).
3. **Want icons?** Install Tourne'Icons, press **Apply a pack**, and open a pack's `pack.tsv`.

## Use together

These are built to run side by side with Tourne Desktop:

| Mod | Why it's separate |
|---|---|
| [Tourne'Suite](windhawk-mods/tourne-suite/) | The parts that must hook Explorer and other programs. Tourne Desktop skips any part that's switched on in the suite, so nothing is drawn twice. |
| [Tourne Menus](windhawk-mods/tourne-menus/) | Draws File Explorer's, the file dialogs' and Windows' own menus in Tourne Desktop's look (Desktop publishes the look to `HKCU\Software\Tourne\Menus`). |
| [Tourne'Icons](windhawk-mods/tourne-icons/) | Changes any icon everywhere, picked from a tray icon. With it on, turn the suite's Icon Redirect part off. Tourne'Icons Lasting is its hook-free twin whose changes stay with the mod off, and Tourne'Icons Kit the same as a plain program. |
| [Tourne'Table](windhawk-mods/tourne-table/) | Built into Tourne Desktop already. Use the mod when you don't run Desktop's copy: Desktop leaves its own off while a Tourne'Table mod is on. |

## Standalones

Each of these also lives inside the suite as a part. Run the standalone **or** the suite's part, not both: Tourne'Tray [System Icons], Tourne'Text [System Text Color], Tourne'Tray [Change Tray Icons], Tourne'Menu [No Icons], Tourne'Menu [No Duplicates], Tourne'Tray [Classic Tray Folders], Tourne'Tray [Clock], Tourne'Icons [Icon Redirect] and Tourne'Suite Tweaks. Fully on their own: Tourne Drop Install, Tourne Mod Tray, and Hide the app icon in taskbar thumbnail previews. The full table with versions is in [`windhawk-mods/README.md`](windhawk-mods/README.md).

## Share your Tourne'Table style

Right-click the visualizer, then **My Styles > Style Editor > Community > Share Mine to the Community**. It opens an issue here with your style in it; a bot checks it and replies with a code anyone can paste to try it. Browse what's there in [`community/`](community/) or on the [gallery page](https://user-tourne.github.io/Tourne/).

## What was left out on purpose

- **Older builds.** Only the newest version of each mod is here: suite b55, Tourne'Table 2.1, Tourne'Icons 2.7.1, and so on.
- **Personal settings.** The suite and Tweaks bake their author's settings in as defaults; paths into Tourne's own folders and e-mail addresses were blanked, and the `.reg` exports of Tourne's own settings aren't included. Icon pack rows that only pointed at files on Tourne's PC were dropped (85 in the ink pack, 34 in the red and green pack).
- **Third-party theme files** (Bouquet, Everforest and others) aren't redistributed; the research notes that describe them are.

## Licenses

Tourne's own code, mods, icons and docs are MIT ([`LICENSE`](LICENSE)). Third-party pieces keep their own licenses, listed in [`CREDITS.md`](CREDITS.md).
