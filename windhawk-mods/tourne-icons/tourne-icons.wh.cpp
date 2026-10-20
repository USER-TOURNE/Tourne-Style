// ==WindhawkMod==
// @id              tourne-icons
// @name            Tourne'Icons
// @description     Change any icon once, by picking it from your taskbar, tray, desktop or Start, and it changes everywhere. No system file is changed
// @version         2.7.3
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @donateUrl       https://ko-fi.com/tourne
// @include         *
// @include         windhawk.exe
// @exclude         vgc.exe
// @compilerOptions -luser32 -lshell32 -lole32 -lgdi32 -ladvapi32 -luuid
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tourne'Icons

Change any icon in Windows once, and it changes everywhere that icon shows.
It needs nothing but Windhawk.

## How to change an icon

1. Click **Tourne'Icons** in the tray (its picture icon). It opens the
   picker.
2. Pick what to change. The lists show what's on your **taskbar**, in your
   **tray**, on your **desktop**, in **Start**, in Settings' **Installed
   apps** and in **Control Panel**, plus **Windows icons** (folders, This PC, the Recycle Bin and
   every icon Windows ships) and **file types**. Type in the search box to find something quickly. Nothing
   needs a path.
3. Click **Choose new icon** and pick a picture: `.ico`, `.png`, `.svg`,
   `.jpg`, `.bmp` or `.gif`, or an icon inside another program. It's made
   into a proper icon at every size Windows uses (16 to 256 pixels).
4. Click **Apply everywhere**.

A program's new icon shows on the taskbar, in Start, in the tray and the
tray's overflow, on its windows, on the desktop, in Alt+Tab and in File
Explorer, because every one of those reads it from the program's file, and
this mod answers for that file in every program. A file type's new icon
shows wherever files of that type are listed.

**Your changes** lists everything you've changed; pick one and click **Put
back original** to undo it.

## Tourne Desktop's taskbar buttons

**Taskbar buttons** lists each open window's button and the pinned ones, the
way Tourne Desktop tells them apart: by the app id a program gives its
windows (a Chrome or Edge web app, Discord, a Store app), else by its file
name. A new icon there shows on that button on Tourne Desktop's taskbar, and
in its tray and Alt+Tab; nothing else changes (pick the program under **On
the taskbar** to change it everywhere). This is Tourne Desktop's Button icons
moved here: the first run takes in its rows (designs like `tourne:screenshot`
stay as they are), and while this mod runs, Tourne Desktop uses these and
leaves its own rows aside. A pack that changes a program changes its buttons
too.

## Many at once: Save lists and Apply a pack

**Save lists** (under the lists) writes every entry of every list, with its
icon as it shows now, to a new folder in Downloads: a picture per entry and
`pack.tsv`, which says what each picture is for. To make a set, draw new
pictures, point the first column of `pack.tsv` at them (or swap the pictures
in place), and click **Apply a pack** to put them all on at once. Rows with no
picture are skipped; Your changes lists everything it changed.

A pack keeps the icons you set already (in Your changes, the suite's Icon
Redirect or Tourne Desktop's button icons, by entry or by the file the icon
comes from) unless you say to replace them too; it asks first. Entries a pack
set aren't counted as yours, so a newer pack replaces them. A picture of `-`
puts that entry back, and rows saved from Your changes go back on as they
were.

## Good to know

- No admin is needed, and no Windows file is touched. Your choices and the
  icons made from your pictures are kept in this mod's own folder.
- Tray icons change straight away, and Explorer is told to refresh its icons
  straight away. A program that's already open shows its new icon on its own
  windows (and their taskbar buttons) once it's opened again. **Refresh
  icons** (in the picker, or on the tray icon's right-click menu) asks
  Windows again if one stays old.
- **Store apps** (Settings, Calculator, Photos and the like) change
  too: their logos are pictures in their packages, so every one of them
  (each size and variant the app ships) gets a picture made from your icon,
  and any program that opens the logo gets yours. That's the taskbar, Start,
  Alt+Tab, Settings' app list and File Explorer. Start is restarted once
  (it starts again by itself) so it lets go of the logos it had. A Store app
  update keeps your icon.
- Your tray list fills in as programs add their tray icons while this mod is
  running.
- If you used Tourne'Suite or Tourne Desktop before, the first run takes in
  the icons you set there (Button icons, and Icon Redirect's rows); turn the
  suite's Icon Redirect off afterwards so the two don't both answer for the
  same files. Without them there's nothing to take
  in, and nothing else changes.

## Icon themes and whole files (optional)

You don't need these to change icons; the picker does that. The settings below are for whole icon themes and for replacing whole files
(a theme DLL with the same resource ids as the file it stands in for, or a
folder of pictures named by resource id). Changes made in the picker win
over them.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- trayIcon: true
  $name: Tourne'Icons in the tray
  $description: >-
    Click it to open the icon picker; right-click it for Refresh icons. Your
    changes keep working with it off, but the picker can't be opened then.
- swapTrayIcons: true
  $name: Change programs' tray icons too
  $description: >-
    A program you give a new icon shows it in the tray as well. Off: tray
    icons stay as the programs draw them (some draw a different icon for each
    state, like an unread badge).
- iconTheme: ""
  $name: Icon theme
  $description: >-
    A theme from the community icon theme collection. It's downloaded once,
    the first time you pick it; programs that are already open pick it up
    when they restart.
  $options:
  - "": None
  - All White Icons|themes/icons/niivu/All%20White%20Icons.zip: All White Icons (by niivu)
  - Antu alt|themes/icons/niivu/antu%20alt.zip: Antu alt (by niivu)
  - Antu|themes/icons/niivu/antu.zip: Antu (by niivu)
  - Kuyen Alt|themes/icons/niivu/kuyen%20alt.zip: Kuyen Alt (by niivu)
  - Kuyen|themes/icons/niivu/kuyen.zip: Kuyen (by niivu)
  - ARC Symbolic|themes/icons/niivu/ARC%20Symbolic.zip: ARC Symbolic (by niivu)
  - ARC|themes/icons/niivu/ARC.zip: ARC (by niivu)
  - Arc Neutral Brown|themes/icons/niivu/arc-neutral%20brown.zip: Arc Neutral Brown (by niivu)
  - Arc Neutral Grey|themes/icons/niivu/arc-neutral%20grey.zip: Arc Neutral Grey (by niivu)
  - BananaOneUI|themes/icons/niivu/BANAANA%20OneUI.zip: BananaOneUI (by niivu)
  - Big Sur DarkMode|themes/icons/niivu/Big%20Sur%20DarkMode.zip: Big Sur DarkMode (by niivu)
  - Big Sur LightMode|themes/icons/niivu/Big%20Sur%20LightMode.zip: Big Sur LightMode (by niivu)
  - Blanked DarkMode|themes/icons/niivu/blanked%20dark%20mode.zip: Blanked DarkMode (by niivu)
  - Blanked LightMode|themes/icons/niivu/blanked%20light%20mode.zip: Blanked LightMode (by niivu)
  - Bonny|themes/icons/niivu/bonny%20by%20niivu.zip: Bonny (by niivu)
  - Bouquet|themes/icons/niivu/bouquet.zip: Bouquet (by niivu)
  - Buuf|themes/icons/niivu/buuf.zip: Buuf (by niivu)
  - CakeOS 2.0|themes/icons/niivu/cakeOS%202.0.zip: CakeOS 2.0 (by niivu)
  - CakeOS Blue|themes/icons/niivu/Cake%20OS%20Blue.zip: CakeOS Blue (by niivu)
  - CakeOS Green|themes/icons/niivu/Cake%20OS%20Green.zip: CakeOS Green (by niivu)
  - CakeOS Orange|themes/icons/niivu/Cake%20OS%20Orange.zip: CakeOS Orange (by niivu)
  - CakeOS Purple|themes/icons/niivu/Cake%20OS%20Purple.zip: CakeOS Purple (by niivu)
  - CakeOS Red|themes/icons/niivu/Cake%20OS%20Red.zip: CakeOS Red (by niivu)
  - Candy Original|themes/icons/niivu/candy%20original%20folders.zip: Candy Original (by niivu)
  - Candy Outlined|themes/icons/niivu/candy%20outlined%20folders.zip: Candy Outlined (by niivu)
  - Catppuccin|themes/icons/niivu/Catppuccin.zip: Catppuccin (by niivu)
  - Catppuccin Blue|themes/icons/niivu/Catppuccin%20blue.zip: Catppuccin Blue (by niivu)
  - Catppuccin Flamingo|themes/icons/niivu/Catppuccin%20flamingo.zip: Catppuccin Flamingo (by niivu)
  - Catppuccin Green|themes/icons/niivu/Catppuccin%20green.zip: Catppuccin Green (by niivu)
  - Catppuccin Latte|themes/icons/niivu/Catppuccin%20Latte.zip: Catppuccin Latte (by niivu)
  - Catppuccin Lavender|themes/icons/niivu/Catppuccin%20lavender.zip: Catppuccin Lavender (by niivu)
  - Catppuccin Maroon|themes/icons/niivu/Catppuccin%20maroon.zip: Catppuccin Maroon (by niivu)
  - Catppuccin Mauve|themes/icons/niivu/Catppuccin%20mauve.zip: Catppuccin Mauve (by niivu)
  - Catppuccin Mocha|themes/icons/niivu/Catppuccin%20Mocha.zip: Catppuccin Mocha (by niivu)
  - Catppuccin Peach|themes/icons/niivu/Catppuccin%20peach.zip: Catppuccin Peach (by niivu)
  - Catppuccin Pink|themes/icons/niivu/Catppuccin%20pink.zip: Catppuccin Pink (by niivu)
  - Catppuccin Red|themes/icons/niivu/Catppuccin%20red.zip: Catppuccin Red (by niivu)
  - Catppuccin Sky|themes/icons/niivu/Catppuccin%20sky.zip: Catppuccin Sky (by niivu)
  - Catppuccin Teal|themes/icons/niivu/Catppuccin%20teal.zip: Catppuccin Teal (by niivu)
  - Catppuccin Yellow|themes/icons/niivu/Catppuccin%20yellow.zip: Catppuccin Yellow (by niivu)
  - Deepin Blue DarkMode|themes/icons/niivu/Deepin%20Blue%20-%20for%20dark%20themes.zip: Deepin Blue DarkMode (by niivu)
  - Deepin Blue LightMode|themes/icons/niivu/Deepin%20Blue%20-%20for%20light%20themes.zip: Deepin Blue LightMode (by niivu)
  - Deepin Brown DarkMode|themes/icons/niivu/Deepin%20Brown%20-%20for%20dark%20themes.zip: Deepin Brown DarkMode (by niivu)
  - Deepin Brown LightMode|themes/icons/niivu/Deepin%20Brown%20-%20for%20light%20themes.zip: Deepin Brown LightMode (by niivu)
  - Deepin Green DarkMode|themes/icons/niivu/Deepin%20Green%20-%20for%20dark%20themes.zip: Deepin Green DarkMode (by niivu)
  - Deepin Green LightMode|themes/icons/niivu/Deepin%20Green%20-%20for%20light%20themes.zip: Deepin Green LightMode (by niivu)
  - Deepin Slate DarkMode|themes/icons/niivu/Deepin%20Slate%20-%20for%20dark%20themes.zip: Deepin Slate DarkMode (by niivu)
  - Deepin Slate LightMode|themes/icons/niivu/Deepin%20Slate%20-%20for%20light%20themes.zip: Deepin Slate LightMode (by niivu)
  - Deepo|themes/icons/niivu/Deepo%20Icon%20pack.zip: Deepo (by niivu)
  - Tango|themes/icons/niivu/Tango.zip: Tango (by niivu)
  - Tangerine|themes/icons/niivu/Tangerine.zip: Tangerine (by niivu)
  - Gnome|themes/icons/niivu/Gnome.zip: Gnome (by niivu)
  - Cheser|themes/icons/niivu/Cheser.zip: Cheser (by niivu)
  - Gnome Brave|themes/icons/niivu/Gnome%20Brave.zip: Gnome Brave (by niivu)
  - Gnome Human|themes/icons/niivu/Gnome%20Human.zip: Gnome Human (by niivu)
  - Gnome Noble|themes/icons/niivu/Gnome%20Noble.zip: Gnome Noble (by niivu)
  - Gnome Wine|themes/icons/niivu/Gnome%20Wine.zip: Gnome Wine (by niivu)
  - Gnome Wise|themes/icons/niivu/Gnome%20Wise.zip: Gnome Wise (by niivu)
  - Elementary|themes/icons/niivu/Elementary.zip: Elementary (by niivu)
  - Elementary New|themes/icons/niivu/Elementary%20NEW.zip: Elementary New (by niivu)
  - Humanity|themes/icons/niivu/Humanity.zip: Humanity (by niivu)
  - Everblush|themes/icons/niivu/Everblush.zip: Everblush (by niivu)
  - Everforest|themes/icons/niivu/everforest.zip: Everforest (by niivu)
  - Everforest Blank|themes/icons/niivu/everforest%20blank.zip: Everforest Blank (by niivu)
  - Eyecandy|themes/icons/niivu/Eyecandy.zip: Eyecandy (by niivu)
  - Faba|themes/icons/niivu/FABA.zip: Faba (by niivu)
  - Faba Symbolic|themes/icons/niivu/FABA%20Symbolic.zip: Faba Symbolic (by niivu)
  - Slate|themes/icons/niivu/SLATE.zip: Slate (by niivu)
  - Slate Symbolic|themes/icons/niivu/SLATE%20Symbolic.zip: Slate Symbolic (by niivu)
  - Fetch|themes/icons/niivu/Fetch.zip: Fetch (by niivu)
  - Fluent|themes/icons/niivu/Fluent.zip: Fluent (by niivu)
  - Fluent Keys Night|themes/icons/niivu/Fluent%20Keys%20Night.zip: Fluent Keys Night (by niivu)
  - Fluent Keys Day|themes/icons/niivu/Fluent%20Keys%20Day.zip: Fluent Keys Day (by niivu)
  - Flurry|themes/icons/niivu/FLURRY.zip: Flurry (by niivu)
  - Gruvbox|themes/icons/niivu/Gruvbox.zip: Gruvbox (by niivu)
  - Gruvbox Plus Olive|themes/icons/niivu/gruvbox%20plus%20-%20Olive.zip: Gruvbox Plus Olive (by niivu)
  - Gruvbox Numix|themes/icons/niivu/Gruvbox%20numix.zip: Gruvbox Numix (by niivu)
  - Haiku BeOS|themes/icons/niivu/Haiku%20BeOS.zip: Haiku BeOS (by niivu)
  - Janguru Blue|themes/icons/niivu/janguru%20blue.zip: Janguru Blue (by niivu)
  - Janguru BlueGrey|themes/icons/niivu/janguru%20bluegrey.zip: Janguru BlueGrey (by niivu)
  - Janguru Brown|themes/icons/niivu/janguru%20brown.zip: Janguru Brown (by niivu)
  - Janguru Green|themes/icons/niivu/janguru%20green.zip: Janguru Green (by niivu)
  - Janguru Grey|themes/icons/niivu/janguru%20grey.zip: Janguru Grey (by niivu)
  - Janguru Orange|themes/icons/niivu/janguru%20orange.zip: Janguru Orange (by niivu)
  - koZ|themes/icons/niivu/koZ.zip: koZ (by niivu)
  - Kripton Flatery|themes/icons/niivu/Kripton%20Flatery.zip: Kripton Flatery (by niivu)
  - Linuxfx 11 AIO|themes/icons/niivu/Linuxfx-11-AIO.zip: Linuxfx 11 AIO (by niivu)
  - Linuxfx 11 Lite|themes/icons/niivu/Linuxfx-11-lite.zip: Linuxfx 11 Lite (by niivu)
  - Lol|themes/icons/niivu/lol.zip: Lol (by niivu)
  - Lumicons Folders|themes/icons/niivu/Lumicons%20Folders.zip: Lumicons Folders (by niivu)
  - Lumicons Symbols|themes/icons/niivu/Lumicons%20Symbols.zip: Lumicons Symbols (by niivu)
  - macOSx|themes/icons/niivu/mac%20osx.zip: macOSx (by niivu)
  - macOS Regular|themes/icons/niivu/macOS%20Regular.zip: macOS Regular (by niivu)
  - macOS Blue|themes/icons/niivu/macOS%20blue.zip: macOS Blue (by niivu)
  - macOS Yellow|themes/icons/niivu/macOS%20Yellow%20Folders.zip: macOS Yellow (by niivu)
  - macOS DarkMode|themes/icons/niivu/macOS%20Dark%20Mode.zip: macOS DarkMode (by niivu)
  - macOS LightMode|themes/icons/niivu/macOS%20Light%20Mode.zip: macOS LightMode (by niivu)
  - macPac DarkMode|themes/icons/niivu/macpac%20darkmode.zip: macPac DarkMode (by niivu)
  - macPac LightMode|themes/icons/niivu/macpac%20lightmode.zip: macPac LightMode (by niivu)
  - Mechanical|themes/icons/niivu/mechanical.zip: Mechanical (by niivu)
  - Minium2|themes/icons/niivu/MINIUM2.zip: Minium2 (by niivu)
  - Nord Papirus|themes/icons/niivu/Nord%20Papirus.zip: Nord Papirus (by niivu)
  - Nord Papirus NovaGalactic|themes/icons/niivu/Nord-Papirus-Nova-galactic.zip: Nord Papirus NovaGalactic (by niivu)
  - Numix|themes/icons/niivu/numix.zip: Numix (by niivu)
  - Numix Blue|themes/icons/niivu/numix-remix-blue.zip: Numix Blue (by niivu)
  - Numix Green|themes/icons/niivu/numix-remix-green.zip: Numix Green (by niivu)
  - Numix macOS|themes/icons/niivu/numix-remix-macos.zip: Numix macOS (by niivu)
  - Numix Slate|themes/icons/niivu/numix-remix-slate.zip: Numix Slate (by niivu)
  - Numix Windows|themes/icons/niivu/numix-remix-windows.zip: Numix Windows (by niivu)
  - NUX|themes/icons/niivu/NUX.zip: NUX (by niivu)
  - One Dark Pro|themes/icons/niivu/One%20Dark%20Pro.zip: One Dark Pro (by niivu)
  - One Dark Pro Alt|themes/icons/niivu/One%20Dark%20Pro%20alt.zip: One Dark Pro Alt (by niivu)
  - One UI4|themes/icons/niivu/OneUI4.zip: One UI4 (by niivu)
  - OS X Minimalism|themes/icons/niivu/OS%20X%20Minimalism.zip: OS X Minimalism (by niivu)
  - OS X Minimalism Symbolic|themes/icons/niivu/OS%20X%20Minimalism%20Symbolic.zip: OS X Minimalism Symbolic (by niivu)
  - Paper|themes/icons/niivu/Paper.zip: Paper (by niivu)
  - Papirus Black|themes/icons/niivu/Papirus%20Black.zip: Papirus Black (by niivu)
  - Papirus BlueGrey|themes/icons/niivu/Papirus%20Blue%20Grey.zip: Papirus BlueGrey (by niivu)
  - Papirus Blue|themes/icons/niivu/Papirus%20Blue.zip: Papirus Blue (by niivu)
  - Papirus Brown|themes/icons/niivu/Papirus%20Brown.zip: Papirus Brown (by niivu)
  - Papirus Deep Orange|themes/icons/niivu/Papirus%20Deep%20Orange.zip: Papirus Deep Orange (by niivu)
  - Papirus Dracula|themes/icons/niivu/Papirus%20Dracula.zip: Papirus Dracula (by niivu)
  - Papirus Grey|themes/icons/niivu/Papirus%20Grey.zip: Papirus Grey (by niivu)
  - Papirus Magenta|themes/icons/niivu/Papirus%20Magenta.zip: Papirus Magenta (by niivu)
  - Papirus Pink|themes/icons/niivu/Papirus%20Pink.zip: Papirus Pink (by niivu)
  - Papirus Red|themes/icons/niivu/Papirus%20Red.zip: Papirus Red (by niivu)
  - Papirus Solarized|themes/icons/niivu/Papirus%20Solarized.zip: Papirus Solarized (by niivu)
  - Papirus Teal|themes/icons/niivu/Papirus%20Teal.zip: Papirus Teal (by niivu)
  - Papirus Violet|themes/icons/niivu/Papirus%20Violet.zip: Papirus Violet (by niivu)
  - Tokyo Night|themes/icons/niivu/Tokyo%20Night%20blank.zip: Tokyo Night (by niivu)
  - Tokyo Night Papirus|themes/icons/niivu/Tokyo%20Night%20Papirus.zip: Tokyo Night Papirus (by niivu)
  - Tokyo Night SE Papirus|themes/icons/niivu/Tokyo%20Night%20SE%20Papirus.zip: Tokyo Night SE Papirus (by niivu)
  - Pink Folders|themes/icons/niivu/pink%20folders.zip: Pink Folders (by niivu)
  - Post|themes/icons/niivu/post.zip: Post (by niivu)
  - Pure Dark|themes/icons/niivu/Pure%20for%20dark%20or%20dark%20side%20panel%20themes.zip: Pure Dark (by niivu)
  - Pure Light|themes/icons/niivu/Pure%20for%20light%20themes.zip: Pure Light (by niivu)
  - Quixotic Day|themes/icons/niivu/Quixotic-SE%20Day%20AIO.zip: Quixotic Day (by niivu)
  - Quixotic Dark|themes/icons/niivu/Quixotic-SE%20Dark%20AIO.zip: Quixotic Dark (by niivu)
  - Quixotic Night|themes/icons/niivu/Quixotic-SE%20Night%20AIO.zip: Quixotic Night (by niivu)
  - Rose Pine|themes/icons/niivu/Rose%20Pine.zip: Rose Pine (by niivu)
  - Solarized Day AIO|themes/icons/niivu/solarized%20Day%20AIO.zip: Solarized Day AIO (by niivu)
  - Solarized Night AIO|themes/icons/niivu/solarized%20Night%20AIO.zip: Solarized Night AIO (by niivu)
  - Solus|themes/icons/niivu/Solus.zip: Solus (by niivu)
  - Somatic Rebirth|themes/icons/niivu/Somatic%20Rebirth.zip: Somatic Rebirth (by niivu)
  - Spaceshrooms Blue|themes/icons/niivu/space-shrooms-blue.zip: Spaceshrooms Blue (by niivu)
  - Spaceshrooms Green|themes/icons/niivu/space-shrooms-green.zip: Spaceshrooms Green (by niivu)
  - Spaceshrooms Yellow|themes/icons/niivu/space-shrooms-yellow.zip: Spaceshrooms Yellow (by niivu)
  - Super Remix Blue|themes/icons/niivu/Super%20Remix%20Blue.zip: Super Remix Blue (by niivu)
  - Super Remix Green|themes/icons/niivu/Super%20Remix%20Green.zip: Super Remix Green (by niivu)
  - Super Remix Slate|themes/icons/niivu/Super%20Remix%20Slate.zip: Super Remix Slate (by niivu)
  - Sweet Awesomeness|themes/icons/niivu/Sweet%20Awesomeness.zip: Sweet Awesomeness (by niivu)
  - Sweetness Blue|themes/icons/niivu/Sweetness%20Blue%20folders.zip: Sweetness Blue (by niivu)
  - Sweetness Neutral|themes/icons/niivu/Sweetness%20Neutral.zip: Sweetness Neutral (by niivu)
  - Sweetness Original|themes/icons/niivu/Sweetness%20Original.zip: Sweetness Original (by niivu)
  - Sweetness Pink|themes/icons/niivu/Sweetness%20Pink%20folders.zip: Sweetness Pink (by niivu)
  - Sweetness Purple|themes/icons/niivu/Sweetness%20Purple%20folders.zip: Sweetness Purple (by niivu)
  - Sweet Rainbow|themes/icons/niivu/Sweet-Rainbow.zip: Sweet Rainbow (by niivu)
  - UOS|themes/icons/niivu/Uos%20Icon%20pack.zip: UOS (by niivu)
  - Windows 11 New (default)|themes/icons/niivu/Windows%2011%20New%20%28default%29.zip: Windows 11 New (default) (by niivu)
  - Windows 11 New Folders Blue|themes/icons/niivu/Windows%2011%20New%20Folders%20Blue.zip: Windows 11 New Folders Blue (by niivu)
  - Windows 11 New Folders Green|themes/icons/niivu/Windows%2011%20New%20Folders%20Green.zip: Windows 11 New Folders Green (by niivu)
  - Windows 11 New Folders Purple|themes/icons/niivu/Windows%2011%20New%20Folders%20Purple.zip: Windows 11 New Folders Purple (by niivu)
  - Windows 11 New Folders Slate|themes/icons/niivu/Windows%2011%20New%20Folders%20Slate.zip: Windows 11 New Folders Slate (by niivu)
  - Windows 11 New Folders Yellow|themes/icons/niivu/Windows%2011%20New%20Folders%20Yellow.zip: Windows 11 New Folders Yellow (by niivu)
  - PaneVista|themes/icons/SoftwareType/PaneVista.zip: 'PaneVista (by SoftwareType, credit: ImSwordQueen)'
  - Pane7|themes/icons/ImSwordQueen/Pane7.zip: Pane7 (by ImSwordQueen)
  - Pane8.1|themes/icons/NicSonic/Pane8.1.zip: Pane8.1 (by NicSonic)
  - Minecraft Axolotle|themes/icons/Wasiabbas4pk/Minecraft%20Axolotle.zip: 'Minecraft Axolotle (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft Fox|themes/icons/Wasiabbas4pk/Minecraft%20Fox.zip: 'Minecraft Fox (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft TNT|themes/icons/Wasiabbas4pk/Minecraft%20TNT.zip: 'Minecraft TNT (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft Panda|themes/icons/Wasiabbas4pk/Minecraft%20Panda.zip: 'Minecraft Panda (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft Creeper|themes/icons/Wasiabbas4pk/Minecraft%20Creeper.zip: 'Minecraft Creeper (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft Ender Dragon|themes/icons/Wasiabbas4pk/Minecraft%20Ender%20Dragon.zip: 'Minecraft Ender Dragon (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft Shulker (Blue)|themes/icons/Wasiabbas4pk/Minecraft%20Shulker%28Blue%29.zip: 'Minecraft Shulker (Blue) (by WasiXGamer, credit: dalps/minecraft11)'
  - Minecraft Shulker (Black)|themes/icons/Wasiabbas4pk/Minecraft%20Shulker%28Black%29.zip: 'Minecraft Shulker (Black) (by WasiXGamer, credit: dalps/minecraft11)'
- themePaths: [""]
  $name: Theme paths
  $description: >-
    Themes on disk: a folder holding a theme.ini file, or the .ini file
    itself. Later paths win over earlier ones.
- brandingImages: ""
  $name: Windows branding pictures
  $description: >-
    A folder of pictures for the Windows logo in About Windows (winver), the
    shut down dialog and System, named by Windows' own ids: 123, 1123 and 2123
    are the logo at its three sizes (458x72, 573x90, 687x108). .png, .bmp or
    .svg. Open windows pick it up when they're opened again.
  #! $format: folderPath
- redirectionResourcePaths:
  - - original: ""
      $name: Original file
      $description: >-
        The file icons normally come from. * matches any text and ? any one
        character, e.g. *\notepad++.exe.
    - redirect: ""
      $name: Replacement file
      $description: >-
        A theme DLL or EXE with the same resource ids, an .ico file to
        replace the program's main icon, or a folder of pictures named by
        resource id (123.png, 124.svg, 131.bmp).
  $name: Redirects
  $description: >-
    Later rows win over earlier rows and over themes. Changes made in the
    picker win over these.
- stubbornRedirects:
  - - target: ""
      $name: Program, shortcut or icon file
      $description: >-
        What to catch: a program (*\discord.exe), a shortcut (*\Discord.lnk)
        or the icon file shortcuts take their icon from (*\discord\app.ico).
        * matches any text.
    - icon: ""
      $name: New icon
      $description: >-
        A picture (.ico, .png, .svg, .jpg, .bmp) or an icon in a program
        (file,number).
  $name: Stubborn redirects
  $description: >-
    Only Tourne'Icons Lasting uses these; here they do nothing. They're kept
    so the same settings paste into both.
- watchShortcuts: false
  $name: Put icons back when a shortcut changes
  $description: >-
    Only Tourne'Icons Lasting uses this; here it does nothing.
- allResourceRedirect: true
  $name: All resource types
  $description: >-
    Off: only icons, cursors and images are replaced. On: menus, dialogs and
    anything else a theme file carries are replaced too.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <psapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <commctrl.h>
#include <commdlg.h>
#include <objbase.h>
#include <wincodec.h>
#include <d2d1_3.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <winternl.h>
#include <aclapi.h>
#include <sddl.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <cwctype>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Tourne'Icons Lasting (tourne-icons-lasting.wh.cpp) is this file made with
// TOURNE_ICONS_LASTING 1: the picker only, writing what Windows keeps by
// itself (shortcuts' icons, Windows' own icon settings), so the changes stay
// with the mod off. It hooks nothing.
//
// Tourne'Icons Kit (tools/icons/kit) is the lasting one built as a program of
// its own with TOURNE_ICONS_KIT 1, without Windhawk: it opens, puts back the
// icons on shortcuts an app's update remade, and ends when its window closes.
#ifndef TOURNE_ICONS_LASTING
#define TOURNE_ICONS_LASTING 0
#endif
#ifndef TOURNE_ICONS_KIT
#define TOURNE_ICONS_KIT 0
#endif
constexpr bool kLasting = TOURNE_ICONS_LASTING;
constexpr bool kKit = TOURNE_ICONS_KIT;
static_assert(!kKit || kLasting, "The Kit is the lasting one");
constexpr const WCHAR* kModName = kKit ? L"Tourne'Icons Kit" : kLasting ? L"Tourne'Icons Lasting" : L"Tourne'Icons";
constexpr const WCHAR* kModTitle = kKit       ? L"Tourne'Icons Kit: change any icon, for good"
                                   : kLasting ? L"Tourne'Icons Lasting: change any icon, for good"
                                              : L"Tourne'Icons: change any icon";
// How the lasting one's texts say where changes stay, and what changes the rest.
constexpr const WCHAR* kStays = kKit ? L"after you close the Kit" : L"with the mod off";
constexpr const WCHAR* kLiveOne = kKit ? L"Tourne'Icons (the Windhawk mod)" : L"Tourne'Icons (the live one)";

// How it works: Windows' own icon code (user32, shell32, comctl32, shcore)
// reads resources through FindResourceExW, FindResourceW, LoadResource and
// SizeofResource. A lookup in a file that has a redirect is tried in the
// replacement files first, newest rule first, then in the file itself. The
// replacement's HRSRC is then handed back to LoadResource with the original
// file's handle, so LoadResource and SizeofResource send any HRSRC that lies
// inside a replacement to that replacement.
//
// An .ico file has no resources to look up, so its images are laid out as an
// icon group and icon entries of our own, and the program's main icon group
// resolves to them.
//
// Replacement files stay loaded for the life of the process, even across a
// settings reload: a program can hold on to what LoadResource gave it.

constexpr WCHAR kThemeBaseUrl[] = L"https://ramensoftware.github.io/resource-redirect-themes/";

// Ids of the icon entries made from an .ico file's images.
constexpr WORD kIconIdBase = 0xE100;

////////////////////////////////////////////////////////////////////////////////
// Paths

std::wstring Lower(std::wstring s) {
    if (!s.empty()) {
        CharLowerBuffW(s.data(), (DWORD)s.size());
    }
    return s;
}

bool EndsWithI(std::wstring_view s, std::wstring_view suffix) {
    return s.size() >= suffix.size() &&
           _wcsnicmp(s.data() + s.size() - suffix.size(), suffix.data(), suffix.size()) == 0;
}

bool FileExists(std::wstring const& path) {
    DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool FolderExists(std::wstring const& path) {
    DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// A setting's path: trimmed, outer quotes dropped, variables expanded.
std::wstring CleanPath(std::wstring_view s) {
    while (!s.empty() && iswspace(s.front())) {
        s.remove_prefix(1);
    }
    while (!s.empty() && iswspace(s.back())) {
        s.remove_suffix(1);
    }
    if (s.size() >= 2 && s.front() == L'"' && s.back() == L'"') {
        s = s.substr(1, s.size() - 2);
    }
    std::wstring path(s);
    if (path.find(L'%') == std::wstring::npos) {
        return path;
    }
    DWORD length = ExpandEnvironmentStringsW(path.c_str(), nullptr, 0);
    if (!length) {
        return path;
    }
    std::wstring expanded(length, L'\0');
    length = ExpandEnvironmentStringsW(path.c_str(), expanded.data(), length);
    expanded.resize(length ? length - 1 : 0);
    return expanded;
}

std::wstring FullPath(std::wstring const& path) {
    WCHAR buffer[MAX_PATH * 2];
    DWORD length = GetFullPathNameW(path.c_str(), ARRAYSIZE(buffer), buffer, nullptr);
    return length && length < ARRAYSIZE(buffer) ? std::wstring(buffer, length) : path;
}

// "\Device\HarddiskVolume3" and the like, to drive letters.
struct DeviceName {
    std::wstring device;  // Lower case.
    std::wstring drive;
};
std::vector<DeviceName> g_deviceNames;
INIT_ONCE g_deviceNamesOnce = INIT_ONCE_STATIC_INIT;

BOOL CALLBACK ReadDeviceNames(PINIT_ONCE, PVOID, PVOID*) {
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (!(drives & (1u << i))) {
            continue;
        }
        WCHAR drive[] = {WCHAR(L'A' + i), L':', L'\0'};
        WCHAR device[MAX_PATH];
        if (QueryDosDeviceW(drive, device, ARRAYSIZE(device))) {
            g_deviceNames.push_back({Lower(device), drive});
        }
    }
    return TRUE;
}

std::wstring DosPath(std::wstring const& ntPath) {
    InitOnceExecuteOnce(&g_deviceNamesOnce, ReadDeviceNames, nullptr, nullptr);
    std::wstring lower = Lower(ntPath);
    for (auto const& name : g_deviceNames) {
        if (lower.size() > name.device.size() && lower[name.device.size()] == L'\\' &&
            lower.starts_with(name.device)) {
            return name.drive + ntPath.substr(name.device.size());
        }
    }
    constexpr std::wstring_view kNetwork = L"\\device\\mup\\";
    if (lower.starts_with(kNetwork)) {
        return L"\\\\" + ntPath.substr(kNetwork.size());
    }
    return {};
}

// The file behind a module handle. Handles from LoadLibraryEx's data file
// flags carry flag bits and aren't in the loader's list, so those are looked
// up by the file mapped there.
std::wstring ModulePath(HMODULE module) {
    WCHAR buffer[1024];
    uintptr_t flags = (uintptr_t)module & 3;
    if (!flags) {
        DWORD length = GetModuleFileNameW(module, buffer, ARRAYSIZE(buffer));
        if (length && length < ARRAYSIZE(buffer)) {
            return std::wstring(buffer, length);
        }
    }
    DWORD length = K32GetMappedFileNameW(GetCurrentProcess(), (void*)((uintptr_t)module - flags),
                                         buffer, ARRAYSIZE(buffer));
    if (!length || length >= ARRAYSIZE(buffer)) {
        return {};
    }
    return DosPath(std::wstring(buffer, length));
}

std::wstring g_systemRoot;  // Lower case, e.g. c:\windows.

// The names a file answers to. Rules are written for System32, and a 32-bit
// program sees the same files under SysWOW64; icons that Windows keeps in
// SystemResources\<name>.mun belong to System32\<name>.
std::vector<std::wstring> PathNames(std::wstring lower) {
    std::vector<std::wstring> names{lower};
    if (g_systemRoot.empty()) {
        return names;
    }
    std::wstring wow = g_systemRoot + L"\\syswow64\\";
    std::wstring resources = g_systemRoot + L"\\systemresources\\";
    if (lower.starts_with(wow)) {
        names.push_back(g_systemRoot + L"\\system32\\" + lower.substr(wow.size()));
    } else if (lower.starts_with(resources) && lower.ends_with(L".mun")) {
        names.push_back(g_systemRoot + L"\\system32\\" +
                        lower.substr(resources.size(), lower.size() - resources.size() - 4));
    }
    return names;
}

// * matches any run of characters, ? any one. Both sides lower case.
bool WildcardMatch(std::wstring_view pattern, std::wstring_view text) {
    size_t p = 0, t = 0, star = std::wstring_view::npos, resume = 0;
    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == L'?' || pattern[p] == text[t])) {
            p++;
            t++;
        } else if (p < pattern.size() && pattern[p] == L'*') {
            star = p++;
            resume = t;
        } else if (star != std::wstring_view::npos) {
            p = star + 1;
            t = ++resume;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == L'*') {
        p++;
    }
    return p == pattern.size();
}

////////////////////////////////////////////////////////////////////////////////
// Replacement files

#pragma pack(push, 2)
struct IcoEntry {
    BYTE width, height, colors, reserved;
    WORD planes, bitCount;
    DWORD bytes, offset;
};
struct GroupEntry {
    BYTE width, height, colors, reserved;
    WORD planes, bitCount;
    DWORD bytes;
    WORD id;
};
#pragma pack(pop)

// An .ico file laid out as resources: entries[0] is the icon group, and
// entries[1 + i] image i, with id kIconIdBase + i.
struct IconFile {
    std::vector<BYTE> file;
    std::vector<BYTE> group;
    std::vector<IMAGE_RESOURCE_DATA_ENTRY> entries;
    std::vector<DWORD> offsets;
};

IconFile* ReadIconFile(std::wstring const& path) {
    HANDLE handle = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE,
                                nullptr, OPEN_EXISTING, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    std::vector<BYTE> data;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(handle, &size) && size.QuadPart > 6 && size.QuadPart <= 16 * 1024 * 1024) {
        data.resize((size_t)size.QuadPart);
        DWORD read = 0;
        if (!ReadFile(handle, data.data(), (DWORD)data.size(), &read, nullptr) || read != data.size()) {
            data.clear();
        }
    }
    CloseHandle(handle);
    if (data.size() < 6) {
        return nullptr;
    }

    WORD reserved = *(WORD*)&data[0], type = *(WORD*)&data[2], count = *(WORD*)&data[4];
    if (reserved != 0 || type != 1 || count == 0 ||
        6 + (size_t)count * sizeof(IcoEntry) > data.size()) {
        return nullptr;
    }

    auto icon = std::make_unique<IconFile>();
    icon->group.resize(6 + count * sizeof(GroupEntry));
    memcpy(icon->group.data(), data.data(), 6);
    icon->entries.resize(count + 1);
    for (WORD i = 0; i < count; i++) {
        IcoEntry entry;
        memcpy(&entry, &data[6 + i * sizeof(IcoEntry)], sizeof(entry));
        if (!entry.bytes || entry.offset > data.size() || entry.bytes > data.size() - entry.offset) {
            return nullptr;
        }
        // Some .ico writers leave the bit depth out, and Windows picks an
        // image by it.
        if (!entry.bitCount) {
            const BYTE* image = &data[entry.offset];
            bool png = entry.bytes >= 8 && memcmp(image, "\x89PNG", 4) == 0;
            entry.bitCount = png ? 32 : entry.bytes >= 16 ? *(const WORD*)(image + 14) : 32;
            entry.planes = 1;
        }
        GroupEntry groupEntry;
        memcpy(&groupEntry, &entry, 12);
        groupEntry.id = kIconIdBase + i;
        memcpy(&icon->group[6 + i * sizeof(GroupEntry)], &groupEntry, sizeof(groupEntry));
        icon->offsets.push_back(entry.offset);
        icon->entries[1 + i].Size = entry.bytes;
    }
    icon->entries[0].Size = (DWORD)icon->group.size();
    icon->file = std::move(data);
    return icon.release();
}

std::vector<BYTE> ReadWholeFile(std::wstring const& path) {
    std::vector<BYTE> data;
    HANDLE handle = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr,
                                OPEN_EXISTING, 0, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return data;
    }
    LARGE_INTEGER size{};
    if (GetFileSizeEx(handle, &size) && size.QuadPart > 0 && size.QuadPart <= 16 * 1024 * 1024) {
        data.resize((size_t)size.QuadPart);
        DWORD read = 0;
        if (!ReadFile(handle, data.data(), (DWORD)data.size(), &read, nullptr) || read != data.size()) {
            data.clear();
        }
    }
    CloseHandle(handle);
    return data;
}

////////////////////////////////////////////////////////////////////////////////
// Folders of pictures

// A folder of pictures named by resource id (123.png, 1123.bmp, 2123.svg)
// stands in for a file's IMAGE and PNG resources (as PNG data) and its
// BITMAP resources (as a DIB). Each picture has two resource entries, [2i]
// for the first kind and [2i + 1] for the second, filled in when first asked
// for: a PNG as it is, a .bmp as a BITMAP as it is, anything else converted
// once and kept.
enum class PictureKind { Png, Bmp, Svg };

struct FolderPicture {
    WORD id;
    PictureKind kind;
    std::vector<BYTE> file;
};

struct ImageFolder {
    std::vector<FolderPicture> pictures;
    std::vector<IMAGE_RESOURCE_DATA_ENTRY> entries;
    std::unique_ptr<std::atomic<const BYTE*>[]> data;  // Published after the entry's size.
    std::vector<std::vector<BYTE>> made;               // Converted forms, never freed.
    SRWLOCK lock = SRWLOCK_INIT;
};

ImageFolder* ReadImageFolder(std::wstring const& path) {
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((path + L"\\*").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    auto folder = std::make_unique<ImageFolder>();
    do {
        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        std::wstring_view name(find.cFileName);
        size_t dot = name.rfind(L'.');
        if (dot == 0 || dot == std::wstring_view::npos || dot > 5) {
            continue;
        }
        std::wstring_view stem = name.substr(0, dot), ext = name.substr(dot);
        PictureKind kind;
        if (_wcsicmp(ext.data(), L".png") == 0) {
            kind = PictureKind::Png;
        } else if (_wcsicmp(ext.data(), L".bmp") == 0) {
            kind = PictureKind::Bmp;
        } else if (_wcsicmp(ext.data(), L".svg") == 0) {
            kind = PictureKind::Svg;
        } else {
            continue;
        }
        unsigned id = 0;
        bool number = true;
        for (WCHAR c : stem) {
            number = number && c >= L'0' && c <= L'9';
            id = id * 10 + (c - L'0');
        }
        if (!number || !id || id > 0xFFFF) {
            continue;
        }
        // One picture per id: a PNG before a .bmp before an .svg.
        auto same = std::find_if(folder->pictures.begin(), folder->pictures.end(),
                                 [&](FolderPicture const& p) { return p.id == id; });
        if (same != folder->pictures.end() && same->kind <= kind) {
            continue;
        }
        std::vector<BYTE> file = ReadWholeFile(path + L"\\" + find.cFileName);
        bool valid = kind == PictureKind::Png   ? file.size() > 24 && memcmp(file.data(), "\x89PNG", 4) == 0
                     : kind == PictureKind::Bmp ? file.size() > 54 && file[0] == 'B' && file[1] == 'M'
                                                : !file.empty();
        if (!valid) {
            Wh_Log(L"Not a picture: %s\\%s", path.c_str(), find.cFileName);
            continue;
        }
        if (same != folder->pictures.end()) {
            *same = {(WORD)id, kind, std::move(file)};
        } else {
            folder->pictures.push_back({(WORD)id, kind, std::move(file)});
        }
    } while (FindNextFileW(search, &find));
    FindClose(search);
    if (folder->pictures.empty()) {
        return nullptr;
    }
    size_t count = folder->pictures.size() * 2;
    folder->entries.resize(count);
    folder->data.reset(new std::atomic<const BYTE*>[count]);
    folder->made.resize(count);
    for (size_t i = 0; i < folder->pictures.size(); i++) {
        auto const& file = folder->pictures[i].file;
        folder->data[2 * i] = nullptr;
        folder->data[2 * i + 1] = nullptr;
        if (folder->pictures[i].kind == PictureKind::Png) {
            folder->entries[2 * i].Size = (DWORD)file.size();
            folder->data[2 * i] = file.data();
        } else if (folder->pictures[i].kind == PictureKind::Bmp) {
            // A BITMAP resource is the file without its 14 byte header.
            folder->entries[2 * i + 1].Size = (DWORD)file.size() - 14;
            folder->data[2 * i + 1] = file.data() + 14;
        }
    }
    return folder.release();
}

// Whether this thread holds the loader lock, when COM and Direct2D mustn't be
// started: a lookup made while a DLL loads keeps the original picture.
bool LoaderLockHeld() {
#ifdef _WIN64
    auto* peb = (BYTE*)__readgsqword(0x60);
    auto* lock = *(RTL_CRITICAL_SECTION**)(peb + 0x110);
#else
    auto* peb = (BYTE*)__readfsdword(0x30);
    auto* lock = *(RTL_CRITICAL_SECTION**)(peb + 0xA0);
#endif
    return lock && (DWORD)(ULONG_PTR)lock->OwningThread == GetCurrentThreadId();
}

// WIC's and Direct2D's ids, kept here so no import library is needed; the
// DLLs are loaded only when a picture has to be converted.
constexpr GUID kClsidWicImagingFactory = {0xcacaf262, 0x9370, 0x4615, {0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a}};
constexpr GUID kWicFormatBgra = {0x6fddc324, 0x4e03, 0x4bfe, {0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0f}};
constexpr GUID kWicFormatPbgra = {0x6fddc324, 0x4e03, 0x4bfe, {0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10}};
constexpr GUID kWicContainerPng = {0x1b7cfaf4, 0x713f, 0x473c, {0xbb, 0xcd, 0x61, 0x37, 0x42, 0x5f, 0xae, 0xaf}};

template <typename T>
struct Com {
    T* p = nullptr;
    ~Com() {
        if (p) p->Release();
    }
    T** operator&() { return &p; }
    T* operator->() const { return p; }
    explicit operator bool() const { return p != nullptr; }
};

IStream* StreamOn(std::vector<BYTE> const& bytes) {
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes.size());
    if (!memory) return nullptr;
    memcpy(GlobalLock(memory), bytes.data(), bytes.size());
    GlobalUnlock(memory);
    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(memory, TRUE, &stream))) {
        GlobalFree(memory);
        return nullptr;
    }
    return stream;
}

// Premultiplied BGRA, top-down.
struct Pixels {
    UINT width = 0, height = 0;
    std::vector<BYTE> bgra;
};

bool DecodePicture(IWICImagingFactory* wic, std::vector<BYTE> const& file, Pixels* out) {
    Com<IStream> stream;
    stream.p = StreamOn(file);
    Com<IWICBitmapDecoder> decoder;
    Com<IWICBitmapFrameDecode> frame;
    Com<IWICFormatConverter> converter;
    if (!stream || FAILED(wic->CreateDecoderFromStream(stream.p, nullptr, WICDecodeMetadataCacheOnDemand, &decoder)) ||
        FAILED(decoder->GetFrame(0, &frame)) || FAILED(wic->CreateFormatConverter(&converter)) ||
        FAILED(converter->Initialize(frame.p, kWicFormatPbgra, WICBitmapDitherTypeNone, nullptr, 0,
                                     WICBitmapPaletteTypeCustom)) ||
        FAILED(converter->GetSize(&out->width, &out->height)) || !out->width || !out->height) {
        return false;
    }
    out->bgra.resize((size_t)out->width * out->height * 4);
    return SUCCEEDED(converter->CopyPixels(nullptr, out->width * 4, (UINT)out->bgra.size(), out->bgra.data()));
}

// An .svg keeps its own width and height, so its root tag is rewritten to
// fill the picture it replaces (centred): width and height become 100%, with
// a viewBox made from them when it has none. Done on the text because
// MinGW's ID2D1SvgElement doesn't match Windows' own for its overloaded
// attribute methods.
std::vector<BYTE> FitSvg(std::vector<BYTE> const& file) {
    std::string text(file.begin(), file.end());
    size_t open = text.find("<svg");
    size_t close = open == std::string::npos ? std::string::npos : text.find('>', open);
    if (close == std::string::npos) {
        return file;
    }
    std::string tag = text.substr(open, close - open);
    // One attribute's value, removed from the tag.
    auto take = [&](const char* name) {
        std::string value;
        for (size_t at = 0; (at = tag.find(name, at)) != std::string::npos; at++) {
            size_t end = at + strlen(name);
            if (!isspace((unsigned char)tag[at - 1]) || tag.compare(end, 1, "=") != 0 || end + 1 >= tag.size()) {
                continue;
            }
            char quote = tag[end + 1];
            size_t last = quote == '"' || quote == '\'' ? tag.find(quote, end + 2) : std::string::npos;
            if (last == std::string::npos) {
                break;
            }
            value = tag.substr(end + 2, last - end - 2);
            tag.erase(at, last + 1 - at);
            break;
        }
        return value;
    };
    std::string w = take("width"), h = take("height");
    bool hasViewBox = tag.find("viewBox") != std::string::npos;
    std::string extra = " width=\"100%\" height=\"100%\"";
    double wv = atof(w.c_str()), hv = atof(h.c_str());
    if (!hasViewBox && wv > 0 && hv > 0 && w.find('%') == std::string::npos && h.find('%') == std::string::npos) {
        char box[96];
        snprintf(box, sizeof(box), " viewBox=\"0 0 %g %g\"", wv, hv);
        extra += box;
    }
    text = text.substr(0, open) + tag + extra + text.substr(close);
    return std::vector<BYTE>(text.begin(), text.end());
}

// An .svg drawn by Windows' own SVG renderer (Direct2D) at the given size.
bool RenderSvg(IWICImagingFactory* wic, std::vector<BYTE> const& original, UINT width, UINT height, Pixels* out) {
    std::vector<BYTE> file = FitSvg(original);
    static auto create = (HRESULT(WINAPI*)(D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS*, void**))
        GetProcAddress(LoadLibraryExW(L"d2d1.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32), "D2D1CreateFactory");
    Com<ID2D1Factory> factory;
    Com<IWICBitmap> bitmap;
    Com<ID2D1RenderTarget> target;
    Com<ID2D1DeviceContext5> context;
    Com<IStream> stream;
    Com<ID2D1SvgDocument> document;
    stream.p = StreamOn(file);
    D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_SOFTWARE, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    if (!create || !stream ||
        FAILED(create(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, (void**)&factory)) ||
        FAILED(wic->CreateBitmap(width, height, kWicFormatPbgra, WICBitmapCacheOnLoad, &bitmap)) ||
        FAILED(factory->CreateWicBitmapRenderTarget(bitmap.p, properties, &target)) ||
        FAILED(target->QueryInterface(__uuidof(ID2D1DeviceContext5), (void**)&context)) ||
        FAILED(context->CreateSvgDocument(stream.p, D2D1::SizeF((float)width, (float)height), &document))) {
        return false;
    }
    target->BeginDraw();
    target->Clear(D2D1::ColorF(0, 0, 0, 0));
    context->DrawSvgDocument(document.p);
    if (FAILED(target->EndDraw())) {
        return false;
    }
    out->width = width;
    out->height = height;
    out->bgra.resize((size_t)width * height * 4);
    return SUCCEEDED(bitmap->CopyPixels(nullptr, width * 4, (UINT)out->bgra.size(), out->bgra.data()));
}

bool EncodePng(IWICImagingFactory* wic, Pixels const& pixels, std::vector<BYTE>* png) {
    // PNG keeps straight alpha.
    std::vector<BYTE> straight = pixels.bgra;
    for (size_t i = 0; i < straight.size(); i += 4) {
        if (BYTE a = straight[i + 3]; a && a < 255) {
            for (int c = 0; c < 3; c++) straight[i + c] = (BYTE)std::min(255, straight[i + c] * 255 / a);
        }
    }
    Com<IWICBitmap> bitmap;
    Com<IStream> stream;
    Com<IWICBitmapEncoder> encoder;
    Com<IWICBitmapFrameEncode> frame;
    WICPixelFormatGUID format = kWicFormatBgra;
    if (FAILED(wic->CreateBitmapFromMemory(pixels.width, pixels.height, kWicFormatBgra, pixels.width * 4,
                                           (UINT)straight.size(), straight.data(), &bitmap)) ||
        FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)) ||
        FAILED(wic->CreateEncoder(kWicContainerPng, nullptr, &encoder)) ||
        FAILED(encoder->Initialize(stream.p, WICBitmapEncoderNoCache)) ||
        FAILED(encoder->CreateNewFrame(&frame, nullptr)) || FAILED(frame->Initialize(nullptr)) ||
        FAILED(frame->SetSize(pixels.width, pixels.height)) || FAILED(frame->SetPixelFormat(&format)) ||
        FAILED(frame->WriteSource(bitmap.p, nullptr)) || FAILED(frame->Commit()) || FAILED(encoder->Commit())) {
        return false;
    }
    HGLOBAL memory = nullptr;
    STATSTG stat{};
    if (FAILED(GetHGlobalFromStream(stream.p, &memory)) || FAILED(stream->Stat(&stat, STATFLAG_NONAME))) {
        return false;
    }
    png->resize((size_t)stat.cbSize.QuadPart);
    memcpy(png->data(), GlobalLock(memory), png->size());
    GlobalUnlock(memory);
    return true;
}

// A 32-bit bottom-up DIB, as a BITMAP resource holds it.
std::vector<BYTE> MakeDib(Pixels const& pixels) {
    std::vector<BYTE> dib(sizeof(BITMAPINFOHEADER) + pixels.bgra.size());
    BITMAPINFOHEADER header{sizeof(header), (LONG)pixels.width, (LONG)pixels.height, 1, 32, BI_RGB};
    memcpy(dib.data(), &header, sizeof(header));
    size_t row = (size_t)pixels.width * 4;
    for (UINT y = 0; y < pixels.height; y++) {
        memcpy(dib.data() + sizeof(header) + (pixels.height - 1 - y) * row, pixels.bgra.data() + y * row, row);
    }
    return dib;
}

// The size of the picture a resource holds (PNG, BMP file or DIB), for
// drawing an .svg in its place.
bool PictureSize(const BYTE* data, DWORD size, UINT* width, UINT* height) {
    auto big = [&](size_t at) {
        return (UINT)data[at] << 24 | (UINT)data[at + 1] << 16 | (UINT)data[at + 2] << 8 | data[at + 3];
    };
    if (size > 24 && memcmp(data, "\x89PNG", 4) == 0) {
        *width = big(16);
        *height = big(20);
    } else {
        if (size > 14 && data[0] == 'B' && data[1] == 'M') {
            data += 14;
            size -= 14;
        }
        if (size < sizeof(BITMAPINFOHEADER) || *(const DWORD*)data < sizeof(BITMAPINFOHEADER)) {
            return false;
        }
        auto const* header = (const BITMAPINFOHEADER*)data;
        *width = (UINT)abs(header->biWidth);
        *height = (UINT)abs(header->biHeight);
    }
    return *width && *height && *width <= 8192 && *height <= 8192;
}

// Where every loaded replacement lies in memory, so LoadResource can tell
// whose HRSRC it was given. Append only, and read without a lock. Kept in
// blocks made as they're needed: a pack changes hundreds of icons, and one
// program (Explorer, the picker) can load them all.
struct Range {
    std::atomic<uintptr_t> begin;
    uintptr_t end;
    HMODULE module;
    IconFile* icon;
    ImageFolder* folder;
};
constexpr int kRangesPerBlock = 256;
constexpr int kMaxRangeBlocks = 256;
std::atomic<Range*> g_rangeBlocks[kMaxRangeBlocks];
std::atomic<int> g_rangeCount;

bool AddRange(uintptr_t begin, uintptr_t end, HMODULE module, IconFile* icon, ImageFolder* folder = nullptr) {
    int i = g_rangeCount.fetch_add(1);
    if (i >= kRangesPerBlock * kMaxRangeBlocks) {
        return false;
    }
    std::atomic<Range*>& slot = g_rangeBlocks[i / kRangesPerBlock];
    Range* block = slot.load(std::memory_order_acquire);
    if (!block) {
        Range* made = new Range[kRangesPerBlock]();
        if (slot.compare_exchange_strong(block, made, std::memory_order_acq_rel)) {
            block = made;
        } else {
            delete[] made;
        }
    }
    Range& range = block[i % kRangesPerBlock];
    range.end = end;
    range.module = module;
    range.icon = icon;
    range.folder = folder;
    range.begin.store(begin, std::memory_order_release);
    return true;
}

Range const* RangeOf(const void* p) {
    int count = std::min(g_rangeCount.load(std::memory_order_acquire), kRangesPerBlock * kMaxRangeBlocks);
    for (int i = 0; i < count; i++) {
        Range* block = g_rangeBlocks[i / kRangesPerBlock].load(std::memory_order_acquire);
        if (!block) {
            i += kRangesPerBlock - 1 - i % kRangesPerBlock;  // Still being made: nothing in it yet.
            continue;
        }
        Range const& range = block[i % kRangesPerBlock];
        uintptr_t begin = range.begin.load(std::memory_order_acquire);
        if (begin && (uintptr_t)p >= begin && (uintptr_t)p < range.end) {
            return &range;
        }
    }
    return nullptr;
}

struct Target {
    std::wstring path;
    bool icoFile = false;
    bool imageFolder = false;
    std::atomic<bool> tried{false};
    std::atomic<HMODULE> module{nullptr};
    std::atomic<IconFile*> icon{nullptr};
    std::atomic<ImageFolder*> folder{nullptr};
};

// Loaded on first use. Two threads can race here; each loads its own copy
// and registers its range before publishing, and the one that loses stays
// loaded, since its range is already live.
bool EnsureLoaded(Target* target) {
    if (!target->tried.load(std::memory_order_acquire)) {
        if (target->imageFolder) {
            if (ImageFolder* folder = ReadImageFolder(target->path)) {
                auto* entries = folder->entries.data();
                if (AddRange((uintptr_t)entries, (uintptr_t)(entries + folder->entries.size()), nullptr, nullptr,
                             folder)) {
                    ImageFolder* expected = nullptr;
                    target->folder.compare_exchange_strong(expected, folder);
                }
            } else {
                Wh_Log(L"No pictures in %s", target->path.c_str());
            }
        } else if (target->icoFile) {
            if (IconFile* icon = ReadIconFile(target->path)) {
                auto* entries = icon->entries.data();
                if (AddRange((uintptr_t)entries, (uintptr_t)(entries + icon->entries.size()), nullptr,
                             icon)) {
                    IconFile* expected = nullptr;
                    target->icon.compare_exchange_strong(expected, icon);
                }
            } else {
                Wh_Log(L"Can't read %s", target->path.c_str());
            }
        } else {
            HMODULE module = LoadLibraryExW(target->path.c_str(), nullptr,
                                            LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
            if (!module) {
                module = LoadLibraryExW(target->path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
            }
            if (module) {
                uintptr_t base = (uintptr_t)module & ~(uintptr_t)3, end = base;
                MEMORY_BASIC_INFORMATION info;
                while (VirtualQuery((void*)end, &info, sizeof(info)) &&
                       (uintptr_t)info.AllocationBase == base) {
                    end = (uintptr_t)info.BaseAddress + info.RegionSize;
                }
                if (AddRange(base, end, module, nullptr)) {
                    HMODULE expected = nullptr;
                    target->module.compare_exchange_strong(expected, module);
                }
            } else {
                Wh_Log(L"Can't load %s (%u)", target->path.c_str(), GetLastError());
            }
        }
        target->tried.store(true, std::memory_order_release);
    }
    return target->module.load(std::memory_order_acquire) ||
           target->icon.load(std::memory_order_acquire) || target->folder.load(std::memory_order_acquire);
}

////////////////////////////////////////////////////////////////////////////////
// Rules

// Set while a hook is at work, so lookups Windows makes on our behalf go
// straight through.
thread_local int t_busy = 0;

// Which icon group of a file a rule's .ico stands in for. A theme file, a
// folder of pictures or a loose picture stands in for the whole file.
enum class Pick { File, Main, Id, Index };

struct Rule {
    std::wstring pattern;  // Lower case.
    Target* target;
    Pick pick = Pick::File;
    int number = 0;        // The id for Pick::Id, the position for Pick::Index.
    bool program = false;  // A program picked in the picker: its tray icon too.
};

// A Store app's logos. They're pictures in its package folder (Windows
// reads them for the taskbar, Start, Alt+Tab and Settings' app list), so
// each one is swapped for a picture made from your new icon at the same size,
// kept in the storage folder under packages\<name>\ by the same relative path.
struct PackageRule {
    std::wstring parent;           // Lower case, ending in \: c:\program files\windowsapps\.
    std::wstring folder;           // The package's folder, lower case; may hold *.
    std::wstring stored;           // Ours.
    std::set<std::wstring> files;  // Relative paths (lower case) we have pictures for.
};

// A set of rules, built whole and then only read. A new set replaces the old
// one when the picker saves; the old one stays in memory, since a lookup on
// another thread can still be reading it.
struct RuleSet {
    std::vector<Rule> rules;
    std::vector<PackageRule> packages;
    unsigned generation = 0;
    std::wstring signature;
    std::wstring trayIcon;  // This program's new icon (.ico), for its tray icon.
};

std::atomic<RuleSet*> g_ruleSet;
std::atomic<unsigned> g_generation;
SRWLOCK g_buildLock = SRWLOCK_INIT;  // Builders only: TargetFor adds to g_targets.
std::wstring g_selfPath;             // This program's file, lower case.

std::wstring RedirectSetting(PCWSTR name, int index = -1) {
    PCWSTR value = index < 0 ? Wh_GetStringSetting(name) : Wh_GetStringSetting(name, index);
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

////////////////////////////////////////////////////////////////////////////////
// Your changes: the picker's list
//
// One file in the mod's storage folder, which every program can read and
// nobody needs admin for: icons.txt, a line per change, its fields split by
// tabs: kind, what was changed, the new icon, a name to show, and for a file
// type its ProgID and the icon it had before. Kinds:
//   program   a program's file (or a pattern): its main icon, and its tray icon
//   icon      one icon: file,index (file,-id for a resource id), or a loose
//             picture file
//   file      a whole file: a theme DLL, or a folder of pictures
//   filetype  .ext: set in Windows' own file type icon (HKCU), so not a rule
//   theme     an icon theme, as the Icon theme setting stores it
// The icons made from your pictures sit beside it, in icons\.

struct ListRow {
    std::wstring kind, original, icon, label, progId, previous;
};

std::wstring StorageRoot() {
    WCHAR storage[MAX_PATH];
    if (!Wh_GetModStoragePath(storage, ARRAYSIZE(storage))) {
        return {};
    }
    return storage;
}

std::wstring ListPath() {
    std::wstring root = StorageRoot();
    return root.empty() ? root : root + L"\\icons.txt";
}

std::vector<std::wstring> SplitTabs(std::wstring_view line) {
    std::vector<std::wstring> fields;
    size_t at = 0;
    while (true) {
        size_t tab = line.find(L'\t', at);
        fields.emplace_back(line.substr(at, tab == std::wstring_view::npos ? std::wstring_view::npos : tab - at));
        if (tab == std::wstring_view::npos) {
            break;
        }
        at = tab + 1;
    }
    return fields;
}

std::vector<ListRow> ReadList(std::wstring const& path = ListPath()) {
    std::vector<ListRow> rows;
    if (path.empty()) {
        return rows;
    }
    std::vector<BYTE> data = ReadWholeFile(path);
    if (data.size() < 2 || data[0] != 0xFF || data[1] != 0xFE) {
        return rows;
    }
    std::wstring_view text((const WCHAR*)(data.data() + 2), (data.size() - 2) / sizeof(WCHAR));
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring_view line = text.substr(at, end == std::wstring_view::npos ? std::wstring_view::npos : end - at);
        at = end == std::wstring_view::npos ? text.size() : end + 1;
        if (!line.empty() && line.back() == L'\r') {
            line.remove_suffix(1);
        }
        if (line.empty() || line[0] == L'#') {
            continue;
        }
        std::vector<std::wstring> f = SplitTabs(line);
        f.resize(6);
        if (!f[0].empty() && !f[1].empty()) {
            rows.push_back({f[0], f[1], f[2], f[3], f[4], f[5]});
        }
    }
    return rows;
}

DWORD g_saveError = 0;  // Why the last write of a list failed.

// Taskbar buttons rows (kind "button") are Tourne Desktop's: its taskbar
// buttons, tray and Alt+Tab, matched by file name, path, app id or a pattern,
// like its own Button icons. The tool process (and the Kit) hands them over as
// "program<tab>icon" lines in HKCU\Software\Tourne\Icons whenever the list is
// written. The live one's value says it holds the button icons (it took in
// Tourne Desktop's rows), so those step aside while it runs; it's taken away
// when the mod stops. The lasting ones' values only come first.
constexpr WCHAR kButtonsKey[] = L"Software\\Tourne\\Icons";
constexpr const WCHAR* kButtonsValue = kKit ? L"Buttons Kit" : kLasting ? L"Buttons Lasting" : L"Buttons";
bool g_publishButtons = false;  // Set in the tool process and the Kit.

void PublishButtons(std::vector<ListRow> const& rows) {
    std::wstring text;
    // The newest first: Tourne Desktop takes the first that matches.
    for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
        ListRow const& row = *it;
        if (row.kind == L"button" && !row.original.empty() && !row.icon.empty()) {
            text += row.original + L'\t' + row.icon + L'\0';
        }
    }
    if (kLasting && text.empty()) {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kButtonsKey, kButtonsValue);
        return;
    }
    text += text.empty() ? std::wstring(2, L'\0') : std::wstring(1, L'\0');
    RegSetKeyValueW(HKEY_CURRENT_USER, kButtonsKey, kButtonsValue, REG_MULTI_SZ, text.data(),
                    (DWORD)(text.size() * sizeof(WCHAR)));
}

void UnpublishButtons() {
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kButtonsKey, kButtonsValue);
}

// Written beside the old list and moved over it, so a program reading it
// never sees half a list.
bool WriteList(std::vector<ListRow> const& rows, std::wstring const& path = ListPath()) {
    if (path.empty()) {
        return false;
    }
    std::wstring text = L"# Tourne'Icons: your changes. kind\toriginal\ticon\tname\tprogid\tprevious\r\n";
    for (auto const& row : rows) {
        text += row.kind + L'\t' + row.original + L'\t' + row.icon + L'\t' + row.label + L'\t' + row.progId + L'\t' +
                row.previous + L"\r\n";
    }
    std::wstring temporary = path + L".new";
    // A virus scanner or the search indexer can hold either file for a
    // moment after it's written, so each step is tried for up to two seconds.
    auto retry = [](auto step) {
        for (int i = 0; i < 40; i++) {
            if (step()) return true;
            DWORD error = GetLastError();
            if (error != ERROR_SHARING_VIOLATION && error != ERROR_ACCESS_DENIED && error != ERROR_LOCK_VIOLATION) {
                return false;
            }
            Sleep(50);
        }
        return false;
    };
    HANDLE file = INVALID_HANDLE_VALUE;
    if (!retry([&] {
            file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            return file != INVALID_HANDLE_VALUE;
        })) {
        g_saveError = GetLastError();
        return false;
    }
    const WORD bom = 0xFEFF;
    DWORD written = 0;
    bool ok = WriteFile(file, &bom, 2, &written, nullptr) &&
              WriteFile(file, text.data(), (DWORD)(text.size() * sizeof(WCHAR)), &written, nullptr);
    g_saveError = ok ? 0 : GetLastError();
    CloseHandle(file);
    if (ok && !retry([&] { return MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE; })) {
        g_saveError = GetLastError();
        Wh_Log(L"Couldn't save %s: %u", path.c_str(), g_saveError);
        return false;
    }
    if (ok && g_publishButtons && _wcsicmp(path.c_str(), ListPath().c_str()) == 0) PublishButtons(rows);
    return ok;
}

// "file,index" into the file and the number after its last comma.
bool SplitIconLocation(std::wstring const& location, std::wstring* file, int* number) {
    size_t comma = location.rfind(L',');
    if (comma == std::wstring::npos || comma + 1 >= location.size()) {
        return false;
    }
    std::wstring tail = location.substr(comma + 1);
    size_t start = tail[0] == L'-' ? 1 : 0;
    if (start >= tail.size() || tail.find_first_not_of(L"0123456789", start) != std::wstring::npos) {
        return false;
    }
    *file = location.substr(0, comma);
    *number = _wtoi(tail.c_str());
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Rules

// Settings as read, and what they compare by to tell whether a reload is due.
struct RuleSettings {
    std::wstring theme;
    std::vector<std::wstring> themePaths;
    std::vector<std::pair<std::wstring, std::wstring>> redirects;
    std::wstring branding;
    std::vector<ListRow> list;

    std::wstring Signature() const {
        std::wstring s = theme + L'\n' + branding;
        for (auto const& path : themePaths) {
            s += L'\n' + path;
        }
        s += L"\n\n";
        for (auto const& [original, redirect] : redirects) {
            s += original + L'\t' + redirect + L'\n';
        }
        s += L"\n";
        for (auto const& row : list) {
            s += row.kind + L'\t' + row.original + L'\t' + row.icon + L'\n';
        }
        return s;
    }
};

// The icon theme: the setting's, or else the one taken in from before.
std::wstring ThemeSetting(std::vector<ListRow> const& list) {
    std::wstring theme = RedirectSetting(L"iconTheme");
    for (auto const& row : list) {
        if (theme.empty() && row.kind == L"theme") {
            theme = row.original;
        }
    }
    return theme;
}

// Rows end at the first run of empty ones, so a blank row in the middle
// doesn't cut the list short.
RuleSettings ReadRuleSettings() {
    RuleSettings settings;
    settings.list = ReadList();
    settings.theme = ThemeSetting(settings.list);
    settings.branding = RedirectSetting(L"brandingImages");
    for (int i = 0, empty = 0; i < 256 && empty < 8; i++) {
        std::wstring path = RedirectSetting(L"themePaths[%d]", i);
        empty = path.empty() ? empty + 1 : 0;
        if (!path.empty()) {
            settings.themePaths.push_back(path);
        }
    }
    for (int i = 0, empty = 0; i < 1024 && empty < 8; i++) {
        std::wstring original = RedirectSetting(L"redirectionResourcePaths[%d].original", i);
        std::wstring redirect = RedirectSetting(L"redirectionResourcePaths[%d].redirect", i);
        empty = original.empty() && redirect.empty() ? empty + 1 : 0;
        if (!original.empty() && !redirect.empty()) {
            settings.redirects.push_back({original, redirect});
        }
    }
    return settings;
}

std::vector<Target*> g_targets;  // Never freed; see the top of the file.
std::atomic<bool> g_allTypes;

Target* TargetFor(std::wstring const& path) {
    for (Target* target : g_targets) {
        if (_wcsicmp(target->path.c_str(), path.c_str()) == 0) {
            return target;
        }
    }
    auto* target = new Target;
    target->path = path;
    target->icoFile = EndsWithI(path, L".ico");
    DWORD attributes = GetFileAttributesW(path.c_str());
    target->imageFolder = attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
    g_targets.push_back(target);
    return target;
}

// An .ico in a row of the Redirects setting stands in for the program's main
// icon, as it always has.
Rule* AddRule(RuleSet& set, std::wstring const& original, std::wstring const& redirect) {
    std::wstring pattern = CleanPath(original);
    std::wstring file = CleanPath(redirect);
    // A * typed before the replacement (as in the original's box) would make
    // it a relative path that leads nowhere.
    while (!file.empty() && file.front() == L'*') {
        file.erase(0, 1);
    }
    if (pattern.empty() || file.empty()) {
        return nullptr;
    }
    Target* target = TargetFor(FullPath(file));
    set.rules.push_back({Lower(pattern), target, target->icoFile ? Pick::Main : Pick::File});
    return &set.rules.back();
}

// A theme.ini's [redirections]: original=replacement, relative to its folder.
std::vector<std::pair<std::wstring, std::wstring>> ThemeIniRows(std::wstring const& ini) {
    std::vector<std::pair<std::wstring, std::wstring>> rows;
    std::vector<WCHAR> buffer(16384);
    DWORD length;
    while ((length = GetPrivateProfileSectionW(L"redirections", buffer.data(), (DWORD)buffer.size(),
                                               ini.c_str())) == buffer.size() - 2 &&
           buffer.size() < 1024 * 1024) {
        buffer.resize(buffer.size() * 4);
    }
    std::wstring folder = ini.substr(0, ini.find_last_of(L'\\'));
    for (PCWSTR line = buffer.data(); *line; line += wcslen(line) + 1) {
        std::wstring_view entry(line);
        size_t equals = entry.find(L'=');
        if (equals == std::wstring_view::npos) {
            continue;
        }
        std::wstring file = CleanPath(entry.substr(equals + 1));
        if (file.empty()) {
            continue;
        }
        if (!(file.size() > 1 && file[1] == L':') && !file.starts_with(L"\\\\")) {
            file = folder + L"\\" + file;
        }
        rows.push_back({std::wstring(entry.substr(0, equals)), file});
    }
    return rows;
}

void AddThemeIni(RuleSet& set, std::wstring const& ini) {
    for (auto const& [original, file] : ThemeIniRows(ini)) {
        AddRule(set, original, file);
    }
}

// A row of the picker's list, as a rule.
void AddListRule(RuleSet& set, ListRow const& row) {
    if (row.icon.empty()) {
        return;
    }
    if (row.kind == L"file") {
        AddRule(set, row.original, row.icon);
    } else if (row.kind == L"program") {
        if (Rule* rule = AddRule(set, row.original, row.icon)) {
            rule->program = true;
        }
    } else if (row.kind == L"icon") {
        std::wstring file;
        int number = 0;
        if (!SplitIconLocation(row.original, &file, &number)) {
            AddRule(set, row.original, row.icon);  // A loose picture file.
        } else if (Rule* rule = AddRule(set, file, row.icon); rule && rule->target->icoFile) {
            rule->pick = number < 0 ? Pick::Id : Pick::Index;
            rule->number = number < 0 ? -number : number;
        }
    }
}

void AddStoredFiles(std::wstring const& folder, std::wstring const& relative, std::set<std::wstring>& files, int depth) {
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((folder + L"\\" + relative + L"*").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        std::wstring name = find.cFileName;
        if (name.empty() || name[0] == L'.') {  // (".", "..", and our own notes, like .outside)
            continue;
        }
        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (depth < 4) {
                AddStoredFiles(folder, relative + name + L"\\", files, depth + 1);
            }
        } else {
            files.insert(Lower(relative + name));
        }
    } while (FindNextFileW(search, &find));
    FindClose(search);
}

constexpr WCHAR kOutsideNote[] = L".outside";  // in a package row's pictures folder: the app's outside folders

// A package row: its folder (as a pattern, so an update that moves it to a
// new version's folder still matches) and the pictures made for it, in the
// storage folder's packages\<previous>. An app installed outside WindowsApps
// gets a rule for its outside folder too, where its logos are: written down
// when the pictures were made, since asking Windows for it in every program
// as it starts held them up (WebView2 timed out).
void AddPackageRule(RuleSet& set, ListRow const& row) {
    std::wstring original = Lower(CleanPath(row.original));
    size_t slash = original.find_last_of(L'\\');
    std::wstring root = StorageRoot();
    if (slash == std::wstring::npos || slash + 1 >= original.size() || row.previous.empty() || root.empty() ||
        row.previous.find_first_of(L"\\/:") != std::wstring::npos) {
        return;
    }
    PackageRule rule{original.substr(0, slash + 1), original.substr(slash + 1), root + L"\\packages\\" + row.previous};
    AddStoredFiles(rule.stored, L"", rule.files, 0);
    if (rule.files.empty()) {
        return;
    }
    std::vector<BYTE> bytes = ReadWholeFile(rule.stored + L"\\" + kOutsideNote);
    std::string note(bytes.begin(), bytes.end());
    for (size_t at = 0; at < note.size();) {
        size_t end = note.find('\n', at);
        std::string line = note.substr(at, end == std::string::npos ? std::string::npos : end - at);
        at = end == std::string::npos ? note.size() : end + 1;
        if (line.empty()) continue;
        int length = MultiByteToWideChar(CP_UTF8, 0, line.data(), (int)line.size(), nullptr, 0);
        std::wstring outside(length, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, line.data(), (int)line.size(), outside.data(), length);
        std::wstring lower = Lower(outside);
        size_t cut = lower.find_last_of(L'\\');
        if (cut == std::wstring::npos || cut + 1 >= lower.size()) continue;
        set.packages.push_back({lower.substr(0, cut + 1), lower.substr(cut + 1), rule.stored, rule.files});
    }
    set.packages.push_back(std::move(rule));
}

// A theme folder's theme.ini: in the folder, or one level down, where a
// zipped theme often keeps it.
std::wstring ThemeIniIn(std::wstring const& folder) {
    std::wstring ini = folder + L"\\theme.ini";
    if (FileExists(ini)) {
        return ini;
    }
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((folder + L"\\*").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) {
        return {};
    }
    std::wstring found;
    do {
        if ((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && wcscmp(find.cFileName, L".") != 0 &&
            wcscmp(find.cFileName, L"..") != 0) {
            ini = folder + L"\\" + find.cFileName + L"\\theme.ini";
            if (FileExists(ini)) {
                found = ini;
            }
        }
    } while (found.empty() && FindNextFileW(search, &find));
    FindClose(search);
    return found;
}

std::wstring ThemesRoot() {
    std::wstring root = StorageRoot();
    return root.empty() ? root : root + L"\\icon-themes";
}

// "Name|themes/icons/author/name.zip", as the icon theme setting stores it.
struct ThemeChoice {
    std::wstring name;  // Safe as a folder name.
    std::wstring url;
};

ThemeChoice ParseTheme(std::wstring const& value) {
    size_t bar = value.find(L'|');
    if (bar == std::wstring::npos || bar == 0 || bar + 1 == value.size()) {
        return {};
    }
    std::wstring name = value.substr(0, bar);
    for (WCHAR& c : name) {
        if (wcschr(L"<>:\"/\\|?*", c) || c < 32) {
            c = L'_';
        }
    }
    return {name, kThemeBaseUrl + value.substr(bar + 1)};
}

std::vector<std::wstring> PathNames(std::wstring lower);
bool WildcardMatch(std::wstring_view pattern, std::wstring_view text);

// The theme's theme.ini, then the theme paths', each once.
std::vector<std::wstring> ThemeInis(RuleSettings const& settings) {
    std::vector<std::wstring> inis;
    ThemeChoice theme = ParseTheme(settings.theme);
    std::wstring root = ThemesRoot();
    if (!theme.name.empty() && !root.empty()) {
        std::wstring ini = ThemeIniIn(root + L"\\" + theme.name);
        if (!ini.empty()) {
            inis.push_back(ini);
        }
    }
    for (auto const& setting : settings.themePaths) {
        std::wstring path = FullPath(CleanPath(setting));
        std::wstring ini = EndsWithI(path, L".ini") ? path : path + L"\\theme.ini";
        bool duplicate = false;
        for (auto const& s : inis) {
            duplicate = duplicate || _wcsicmp(s.c_str(), ini.c_str()) == 0;
        }
        if (!duplicate && FileExists(ini)) {
            inis.push_back(ini);
        }
    }
    return inis;
}

// Order sets priority: the theme, then theme paths, then the Redirects rows,
// then your changes in the picker, and within each the later one. Lookups
// walk the list from the end.
RuleSet* BuildRuleSet(RuleSettings const& settings) {
    auto* set = new RuleSet;
    AcquireSRWLockExclusive(&g_buildLock);
    // The lock is let go even if a row throws, so the next reload doesn't wait forever.
    struct Unlock {
        ~Unlock() { ReleaseSRWLockExclusive(&g_buildLock); }
    };
    std::unique_ptr<Unlock> unlock(new Unlock);
    std::unique_ptr<RuleSet> owned(set);

    for (auto const& ini : ThemeInis(settings)) {
        AddThemeIni(*set, ini);
    }

    // The Windows logo's pictures, after the rows so they win.
    if (!settings.branding.empty()) {
        AddRule(*set, L"*\\branding\\basebrd\\basebrd.dll", settings.branding);
    }
    for (auto const& [original, redirect] : settings.redirects) {
        AddRule(*set, original, redirect);
    }
    for (auto const& row : settings.list) {
        AddListRule(*set, row);
        if (row.kind == L"package") {
            AddPackageRule(*set, row);
        }
    }

    // This program's own new icon, for its tray icon: the newest program row
    // that names it. Not for the programs that put up tray icons on others'
    // behalf (Explorer's own, and the hosts that run many things).
    bool host = false;
    for (PCWSTR name : {L"\\explorer.exe", L"\\rundll32.exe", L"\\svchost.exe", L"\\dllhost.exe",
                        L"\\sihost.exe", L"\\taskhostw.exe", L"\\tournedesktop.exe"}) {
        host = host || EndsWithI(g_selfPath, name);
    }
    if (!g_selfPath.empty() && !host) {
        std::vector<std::wstring> names = PathNames(g_selfPath);
        for (size_t i = set->rules.size(); i-- > 0 && set->trayIcon.empty();) {
            Rule const& rule = set->rules[i];
            if (!rule.program || !rule.target->icoFile) {
                continue;
            }
            for (auto const& name : names) {
                if (WildcardMatch(rule.pattern, name)) {
                    set->trayIcon = rule.target->path;
                    break;
                }
            }
        }
    }

    unlock.reset();
    set->signature = settings.Signature();
    set->generation = ++g_generation;
    return owned.release();
}

INIT_ONCE g_rulesOnce = INIT_ONCE_STATIC_INIT;

BOOL CALLBACK BuildFirstRules(PINIT_ONCE, PVOID, PVOID*) {
    t_busy++;
    RuleSet* set = nullptr;
    try {
        set = BuildRuleSet(ReadRuleSettings());
    } catch (...) {
        set = new RuleSet;  // No changes rather than a crash in the program that asked.
    }
    t_busy--;
    Wh_Log(L"%zu rules", set->rules.size());
    g_ruleSet.store(set, std::memory_order_release);
    return TRUE;
}

// The rules, read the first time they're needed; null when there are none.
RuleSet* EnsureRules() {
    InitOnceExecuteOnce(&g_rulesOnce, BuildFirstRules, nullptr, nullptr);
    RuleSet* set = g_ruleSet.load(std::memory_order_acquire);
    return set && (!set->rules.empty() || !set->packages.empty()) ? set : nullptr;
}

// After the list or the settings changed: a new set, if anything differs.
// Returns whether it did.
bool RebuildRules() {
    RuleSet* old = g_ruleSet.load(std::memory_order_acquire);
    if (!old) {
        return false;  // Not read yet: the first use reads the new ones.
    }
    t_busy++;
    RuleSettings settings = ReadRuleSettings();
    RuleSet* set = settings.Signature() == old->signature ? nullptr : BuildRuleSet(settings);
    t_busy--;
    if (!set) {
        return false;
    }
    Wh_Log(L"%zu rules (changed)", set->rules.size());
    g_ruleSet.store(set, std::memory_order_release);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Modules

struct Fingerprint {
    DWORD stamp = 0, imageSize = 0, checkSum = 0;
    bool operator==(Fingerprint const&) const = default;
};

// What a module handle points at now. Handles get reused once a module is
// freed, so a cached answer only counts while the headers still match.
bool ReadFingerprint(uintptr_t base, Fingerprint* fingerprint) {
    MEMORY_BASIC_INFORMATION info;
    if (!VirtualQuery((void*)base, &info, sizeof(info)) || info.State != MEM_COMMIT ||
        (info.Protect & (PAGE_NOACCESS | PAGE_GUARD)) || info.RegionSize < 0x200) {
        return false;
    }
    auto* dos = (const IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 ||
        (size_t)dos->e_lfanew > info.RegionSize - sizeof(IMAGE_NT_HEADERS)) {
        return false;
    }
    auto* nt = (const IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }
    // SizeOfImage and CheckSum sit at the same offsets in 32 and 64-bit
    // headers, so either kind of file reads right here.
    *fingerprint = {nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage,
                    nt->OptionalHeader.CheckSum};
    return true;
}

// One icon group of a module that an .ico stands in for.
struct IconSwap {
    bool any = false;   // The module lists no icon group at all: every group.
    WORD id = 0;
    std::wstring name;  // Lower case; empty when it's an id.
    Target* target;

    bool Matches(LPCWSTR group) const {
        if (any) {
            return true;
        }
        if (IS_INTRESOURCE(group)) {
            return name.empty() && LOWORD(group) == id;
        }
        if (group[0] == L'#') {
            return name.empty() && _wtoi(group + 1) == id;
        }
        return !name.empty() && _wcsicmp(group, name.c_str()) == 0;
    }
};

struct ModuleInfo {
    Fingerprint fingerprint;
    unsigned generation = 0;
    std::vector<Target*> targets;  // Whole files, highest priority first.
    std::vector<IconSwap> swaps;   // One icon each, highest priority first.
};


// The icon group at a position (0 is the program's main icon), as Windows
// counts them for "file,index".
struct GroupSearch {
    int wanted, seen = 0;
    bool found = false;
    WORD id = 0;
    std::wstring name;
};

BOOL CALLBACK FindIconGroup(HMODULE, LPCWSTR, LPWSTR name, LONG_PTR param) {
    auto* search = (GroupSearch*)param;
    if (search->seen++ < search->wanted) {
        return TRUE;
    }
    if (IS_INTRESOURCE(name)) {
        search->id = LOWORD(name);
    } else {
        search->name = Lower(name);
    }
    search->found = true;
    return FALSE;
}

std::shared_ptr<ModuleInfo> BuildModuleInfo(HMODULE module, Fingerprint const& fingerprint, RuleSet const& set) {
    auto info = std::make_shared<ModuleInfo>();
    info->fingerprint = fingerprint;
    info->generation = set.generation;
    std::wstring path = ModulePath(module);
    if (path.empty()) {
        return info;
    }
    std::vector<std::wstring> names = PathNames(Lower(path));
    for (size_t i = set.rules.size(); i-- > 0;) {
        Rule const& rule = set.rules[i];
        bool matches = false;
        for (auto const& name : names) {
            if (WildcardMatch(rule.pattern, name)) {
                matches = true;
                break;
            }
        }
        if (!matches) {
            continue;
        }
        if (rule.pick == Pick::File) {
            if (std::find(info->targets.begin(), info->targets.end(), rule.target) == info->targets.end()) {
                info->targets.push_back(rule.target);
            }
            continue;
        }
        IconSwap swap{.target = rule.target};
        if (rule.pick == Pick::Id) {
            swap.id = (WORD)rule.number;
        } else {
            GroupSearch search{rule.pick == Pick::Index ? rule.number : 0};
            EnumResourceNamesW(module, RT_GROUP_ICON, FindIconGroup, (LONG_PTR)&search);
            if (!search.found && (rule.pick == Pick::Index || search.seen)) {
                continue;  // No such icon in this file.
            }
            swap.any = !search.found;
            swap.id = search.id;
            swap.name = search.name;
        }
        info->swaps.push_back(swap);
    }
    return info;
}

SRWLOCK g_modulesLock = SRWLOCK_INIT;
std::unordered_map<uintptr_t, std::shared_ptr<ModuleInfo>> g_modules;

std::shared_ptr<ModuleInfo> InfoFor(HMODULE module, RuleSet const& set) {
    uintptr_t base = (uintptr_t)module & ~(uintptr_t)3;
    Fingerprint fingerprint;
    if (!ReadFingerprint(base, &fingerprint)) {
        return nullptr;
    }

    std::shared_ptr<ModuleInfo> info;
    AcquireSRWLockShared(&g_modulesLock);
    auto it = g_modules.find(base);
    if (it != g_modules.end() && it->second->fingerprint == fingerprint &&
        it->second->generation == set.generation) {
        info = it->second;
    }
    ReleaseSRWLockShared(&g_modulesLock);
    if (info) {
        return info;
    }

    info = BuildModuleInfo(module, fingerprint, set);
    AcquireSRWLockExclusive(&g_modulesLock);
    // Explorer opens thousands of programs for their icons over a session.
    if (g_modules.size() >= 4096) {
        g_modules.clear();
    }
    g_modules[base] = info;
    ReleaseSRWLockExclusive(&g_modulesLock);
    return info;
}

////////////////////////////////////////////////////////////////////////////////
// Lookups

bool TypeWanted(LPCWSTR type) {
    if (g_allTypes.load(std::memory_order_relaxed)) {
        return true;
    }
    if (IS_INTRESOURCE(type)) {
        // RT_CURSOR, RT_BITMAP, RT_ICON, RT_GROUP_CURSOR, RT_GROUP_ICON,
        // RT_ANICURSOR and RT_ANIICON.
        switch ((ULONG_PTR)type) {
            case 1:
            case 2:
            case 3:
            case 12:
            case 14:
            case 21:
            case 22:
                return true;
        }
        return false;
    }
    return _wcsicmp(type, L"PNG") == 0 || _wcsicmp(type, L"IMAGE") == 0;
}

using FindResourceExW_t = HRSRC(WINAPI*)(HMODULE, LPCWSTR, LPCWSTR, WORD);
using FindResourceW_t = HRSRC(WINAPI*)(HMODULE, LPCWSTR, LPCWSTR);
using LoadResource_t = HGLOBAL(WINAPI*)(HMODULE, HRSRC);
using SizeofResource_t = DWORD(WINAPI*)(HMODULE, HRSRC);
FindResourceExW_t FindResourceExW_Original;
FindResourceW_t FindResourceW_Original;
LoadResource_t LoadResource_Original;
SizeofResource_t SizeofResource_Original;

// An .ico's image, by the id its group gave it. (Its group is handed out by
// Redirect, for the group it stands in for.)
HRSRC IconResource(IconFile* icon, LPCWSTR type, LPCWSTR name) {
    if (type == RT_ICON && IS_INTRESOURCE(name)) {
        WORD id = LOWORD(name);
        if (id >= kIconIdBase && id - kIconIdBase < (int)icon->offsets.size()) {
            return (HRSRC)&icon->entries[1 + id - kIconIdBase];
        }
    }
    return nullptr;
}

// A folder's picture for an IMAGE, PNG or BITMAP resource, converted the
// first time it's asked for in that form.
HRSRC FolderResource(ImageFolder* folder, HMODULE module, LPCWSTR type, LPCWSTR name, WORD language) {
    int form = IS_INTRESOURCE(type) ? (type == RT_BITMAP ? 1 : -1)
               : _wcsicmp(type, L"IMAGE") == 0 || _wcsicmp(type, L"PNG") == 0 ? 0
                                                                             : -1;
    unsigned id = IS_INTRESOURCE(name) ? LOWORD(name) : name[0] == L'#' ? (unsigned)_wtoi(name + 1) : 0;
    if (form < 0 || !id) {
        return nullptr;
    }
    size_t index = 0;
    while (index < folder->pictures.size() && folder->pictures[index].id != id) {
        index++;
    }
    if (index == folder->pictures.size()) {
        return nullptr;
    }
    size_t entry = 2 * index + form;
    if (folder->data[entry].load(std::memory_order_acquire)) {
        return (HRSRC)&folder->entries[entry];
    }
    if (LoaderLockHeld()) {
        return nullptr;
    }

    AcquireSRWLockExclusive(&folder->lock);
    if (!folder->data[entry].load(std::memory_order_acquire)) {
        FolderPicture const& picture = folder->pictures[index];
        // An .svg takes the size of the picture it replaces.
        UINT width = 0, height = 0;
        if (picture.kind == PictureKind::Svg) {
            HRSRC original = FindResourceExW_Original(module, type, name, language);
            if (!original) {
                original = FindResourceExW_Original(module, type, name, 0);
            }
            HGLOBAL loaded = original ? LoadResource_Original(module, original) : nullptr;
            if (!loaded || !PictureSize((const BYTE*)loaded, SizeofResource_Original(module, original), &width, &height)) {
                width = height = 256;
            }
        }
        HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        {
            Com<IWICImagingFactory> wic;
            Pixels pixels;
            std::vector<BYTE> made;
            if (SUCCEEDED(CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                           __uuidof(IWICImagingFactory), (void**)&wic)) &&
                (picture.kind == PictureKind::Svg ? RenderSvg(wic.p, picture.file, width, height, &pixels)
                                                  : DecodePicture(wic.p, picture.file, &pixels))) {
                if (form == 0) {
                    EncodePng(wic.p, pixels, &made);
                } else {
                    made = MakeDib(pixels);
                }
            }
            if (!made.empty()) {
                folder->made[entry] = std::move(made);
                folder->entries[entry].Size = (DWORD)folder->made[entry].size();
                folder->data[entry].store(folder->made[entry].data(), std::memory_order_release);
            } else {
                Wh_Log(L"Couldn't convert picture %u", id);
            }
        }
        if (SUCCEEDED(com)) {
            CoUninitialize();
        }
    }
    ReleaseSRWLockExclusive(&folder->lock);
    return folder->data[entry].load(std::memory_order_acquire) ? (HRSRC)&folder->entries[entry] : nullptr;
}

// One replacement's copy of a resource, or null.
HRSRC FindIn(Target* target, HMODULE module, ModuleInfo const& info, LPCWSTR type, LPCWSTR name, WORD language) {
    if (ImageFolder* folder = target->folder.load(std::memory_order_acquire)) {
        return FolderResource(folder, module, type, name, language);
    }
    if (IconFile* icon = target->icon.load(std::memory_order_acquire)) {
        return IconResource(icon, type, name);
    }
    HMODULE replacement = target->module.load(std::memory_order_acquire);
    HRSRC found = FindResourceExW_Original(replacement, type, name, language);
    // A theme file usually carries one language only.
    if (!found && language) {
        found = FindResourceExW_Original(replacement, type, name, 0);
    }
    // A copy of a Windows file can still point at the original's .mui or
    // .mun, which would hand back the stock resource.
    Range const* range = found ? RangeOf(found) : nullptr;
    return range && range->module == replacement ? found : nullptr;
}

// Where this thread's latest icon or cursor group in each module came from:
// a replacement, or null for the module's own. Windows reads a group and then
// its images in one go on one thread, so a few recent modules are enough.
struct GroupSource {
    uintptr_t module;
    Target* target;
};
thread_local GroupSource t_groupSources[8];
thread_local unsigned t_groupSourceNext;

void RememberGroupSource(uintptr_t module, Target* target) {
    for (auto& source : t_groupSources) {
        if (source.module == module) {
            source.target = target;
            return;
        }
    }
    t_groupSources[t_groupSourceNext++ % ARRAYSIZE(t_groupSources)] = {module, target};
}

Target* GroupSourceFor(uintptr_t module) {
    for (auto const& source : t_groupSources) {
        if (source.module == module) {
            return source.target;
        }
    }
    return nullptr;
}

// The resource from the first replacement that has it, or null to use the
// module's own.
HRSRC Redirect(HMODULE module, LPCWSTR type, LPCWSTR name, WORD language) {
    if (t_busy || !type || !name || !TypeWanted(type)) {
        return nullptr;
    }
    if (!module) {
        module = GetModuleHandleW(nullptr);
    }
    // Held through the first rule load too: a lookup made from inside it
    // would otherwise wait on itself.
    t_busy++;
    HRSRC found = nullptr;
    RuleSet* set = EnsureRules();
    auto info = set ? InfoFor(module, *set) : nullptr;
    if (info && (!info->targets.empty() || !info->swaps.empty())) {
        // An icon or cursor image is found by a number that only means
        // something in the file its group came from, so images are taken
        // from that file alone, and from the module itself when the group
        // was the module's own.
        ULONG_PTR kind = IS_INTRESOURCE(type) ? (ULONG_PTR)type : 0;
        uintptr_t base = (uintptr_t)module & ~(uintptr_t)3;
        if (kind == (ULONG_PTR)RT_ICON || kind == (ULONG_PTR)RT_CURSOR) {
            Target* source = GroupSourceFor(base);
            if (source && EnsureLoaded(source)) {
                found = FindIn(source, module, *info, type, name, language);
            }
        } else {
            Target* source = nullptr;
            if (kind == (ULONG_PTR)RT_GROUP_ICON) {
                for (auto const& swap : info->swaps) {
                    IconFile* icon;
                    if (swap.Matches(name) && EnsureLoaded(swap.target) &&
                        (icon = swap.target->icon.load(std::memory_order_acquire))) {
                        found = (HRSRC)&icon->entries[0];
                        source = swap.target;
                        break;
                    }
                }
            }
            for (Target* target : info->targets) {
                if (found) {
                    break;
                }
                if (EnsureLoaded(target) && (found = FindIn(target, module, *info, type, name, language))) {
                    source = target;
                    break;
                }
            }
            if (kind == (ULONG_PTR)RT_GROUP_ICON || kind == (ULONG_PTR)RT_GROUP_CURSOR) {
                RememberGroupSource(base, source);
            }
        }
    }
    t_busy--;
    return found;
}

HRSRC WINAPI FindResourceExW_Hook(HMODULE module, LPCWSTR type, LPCWSTR name, WORD language) {
    if (HRSRC found = Redirect(module, type, name, language)) {
        return found;
    }
    return FindResourceExW_Original(module, type, name, language);
}

HRSRC WINAPI FindResourceW_Hook(HMODULE module, LPCWSTR name, LPCWSTR type) {
    if (HRSRC found = Redirect(module, type, name, 0)) {
        return found;
    }
    // FindResourceW can go through FindResourceExW, which needn't look again.
    t_busy++;
    HRSRC found = FindResourceW_Original(module, name, type);
    t_busy--;
    return found;
}

HGLOBAL WINAPI LoadResource_Hook(HMODULE module, HRSRC resource) {
    if (Range const* range = resource ? RangeOf(resource) : nullptr) {
        if (ImageFolder* folder = range->folder) {
            size_t index = (IMAGE_RESOURCE_DATA_ENTRY*)resource - folder->entries.data();
            return (HGLOBAL)folder->data[index].load(std::memory_order_acquire);
        }
        if (IconFile* icon = range->icon) {
            size_t index = (IMAGE_RESOURCE_DATA_ENTRY*)resource - icon->entries.data();
            return index == 0 ? (HGLOBAL)icon->group.data()
                              : (HGLOBAL)(icon->file.data() + icon->offsets[index - 1]);
        }
        return LoadResource_Original(range->module, resource);
    }
    return LoadResource_Original(module, resource);
}

DWORD WINAPI SizeofResource_Hook(HMODULE module, HRSRC resource) {
    if (Range const* range = resource ? RangeOf(resource) : nullptr) {
        if (range->icon || range->folder) {
            return ((IMAGE_RESOURCE_DATA_ENTRY*)resource)->Size;
        }
        return SizeofResource_Original(range->module, resource);
    }
    return SizeofResource_Original(module, resource);
}

// Loose image files (a program's own .ico, say) are opened by path instead.
// The replacement is used when it's a loose image file too.
bool LooseFileRedirect(LPCWSTR file, std::wstring* replacement) {
    constexpr PCWSTR kImageTypes[] = {L".ico", L".cur", L".ani", L".bmp", L".png"};
    auto isImage = [&](std::wstring_view path) {
        for (PCWSTR type : kImageTypes) {
            if (EndsWithI(path, type)) {
                return true;
            }
        }
        return false;
    };
    if (t_busy || !isImage(file)) {
        return false;
    }
    t_busy++;
    bool found = false;
    if (RuleSet* set = EnsureRules()) {
        std::vector<std::wstring> names = PathNames(Lower(FullPath(file)));
        for (size_t i = set->rules.size(); i-- > 0 && !found;) {
            for (auto const& name : names) {
                if (WildcardMatch(set->rules[i].pattern, name)) {
                    // The newest matching rule decides, even when it doesn't apply.
                    std::wstring const& path = set->rules[i].target->path;
                    if (isImage(path) && FileExists(path)) {
                        *replacement = path;
                    }
                    found = true;
                    break;
                }
            }
        }
    }
    t_busy--;
    return !replacement->empty();
}

using PrivateExtractIconsW_t = decltype(&PrivateExtractIconsW);
PrivateExtractIconsW_t PrivateExtractIconsW_Original;
UINT WINAPI PrivateExtractIconsW_Hook(LPCWSTR file,
                                      int index,
                                      int cx,
                                      int cy,
                                      HICON* icons,
                                      UINT* ids,
                                      UINT count,
                                      UINT flags) {
    std::wstring replacement;
    if (file && LooseFileRedirect(file, &replacement)) {
        return PrivateExtractIconsW_Original(replacement.c_str(), index, cx, cy, icons, ids, count,
                                             flags);
    }
    return PrivateExtractIconsW_Original(file, index, cx, cy, icons, ids, count, flags);
}

using LoadImageW_t = decltype(&LoadImageW);
LoadImageW_t LoadImageW_Original;
HANDLE WINAPI LoadImageW_Hook(HINSTANCE instance, LPCWSTR name, UINT type, int cx, int cy, UINT flags) {
    std::wstring replacement;
    if ((flags & LR_LOADFROMFILE) && name && !IS_INTRESOURCE(name) &&
        LooseFileRedirect(name, &replacement)) {
        return LoadImageW_Original(instance, replacement.c_str(), type, cx, cy, flags);
    }
    return LoadImageW_Original(instance, name, type, cx, cy, flags);
}

////////////////////////////////////////////////////////////////////////////////
// Store apps' logos
//
// A Store app's logo is a picture file in its package, opened by whatever
// shows it (Explorer for the taskbar, Start, Alt+Tab, Settings). Opening one
// we have a picture for opens ours instead. Checked at the lowest file open,
// so every way of opening a file goes through it; anything not ending in .png
// is passed on after a few compares.

#ifndef NT_SUCCESS
#define NT_SUCCESS(status) (((NTSTATUS)(status)) >= 0)
#endif

using NtCreateFile_t = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER,
                                        ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
using NtOpenFile_t = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, ULONG, ULONG);
NtCreateFile_t NtCreateFile_Original;
NtOpenFile_t NtOpenFile_Original;

constexpr ACCESS_MASK kWriteAccess = GENERIC_WRITE | GENERIC_ALL | FILE_WRITE_DATA | FILE_APPEND_DATA |
                                     FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | DELETE | WRITE_DAC | WRITE_OWNER;
constexpr ULONG kFileOpen = 1;  // FILE_OPEN: an existing file, nothing made.

bool HasPart(std::wstring_view lower, std::wstring_view part) {
    return lower.find(part) != std::wstring_view::npos;
}

// "\??\C:\Program Files\WindowsApps\...\Assets\Logo.scale-200.png": the
// picture we have for it, as a name of the same form; empty when none.
std::wstring PackagePicture(POBJECT_ATTRIBUTES attributes) {
    if (t_busy || !attributes || attributes->RootDirectory || !attributes->ObjectName ||
        !attributes->ObjectName->Buffer) {
        return {};
    }
    std::wstring_view name(attributes->ObjectName->Buffer, attributes->ObjectName->Length / sizeof(WCHAR));
    if (name.size() < 12 || !EndsWithI(name, L".png") || name.substr(0, 4) != L"\\??\\") {
        return {};
    }
    std::wstring lower = Lower(std::wstring(name.substr(4)));
    if (!HasPart(lower, L"\\windowsapps\\") && !HasPart(lower, L"\\systemapps\\") &&
        !HasPart(lower, L"\\immersivecontrolpanel\\")) {
        return {};
    }
    t_busy++;
    std::wstring found;
    if (RuleSet* set = EnsureRules()) {
        for (auto const& rule : set->packages) {
            if (lower.size() <= rule.parent.size() || lower.compare(0, rule.parent.size(), rule.parent) != 0) {
                continue;
            }
            size_t slash = lower.find(L'\\', rule.parent.size());
            if (slash == std::wstring::npos ||
                !WildcardMatch(rule.folder, std::wstring_view(lower).substr(rule.parent.size(), slash - rule.parent.size()))) {
                continue;
            }
            std::wstring relative = lower.substr(slash + 1);
            if (rule.files.count(relative)) {
                found = L"\\??\\" + rule.stored + L"\\" + relative;
                break;
            }
        }
    }
    t_busy--;
    return found;
}

NTSTATUS NTAPI NtCreateFile_Hook(PHANDLE handle, ACCESS_MASK access, POBJECT_ATTRIBUTES attributes,
                                 PIO_STATUS_BLOCK status, PLARGE_INTEGER allocation, ULONG fileAttributes,
                                 ULONG share, ULONG disposition, ULONG options, PVOID ea, ULONG eaLength) {
    if (!(access & kWriteAccess) && disposition == kFileOpen) {
        std::wstring picture = PackagePicture(attributes);
        if (!picture.empty()) {
            UNICODE_STRING name{(USHORT)(picture.size() * sizeof(WCHAR)), (USHORT)(picture.size() * sizeof(WCHAR)),
                                picture.data()};
            OBJECT_ATTRIBUTES ours = *attributes;
            ours.ObjectName = &name;
            NTSTATUS result = NtCreateFile_Original(handle, access, &ours, status, allocation, fileAttributes, share,
                                                    disposition, options, ea, eaLength);
            if (NT_SUCCESS(result)) {
                return result;
            }
        }
    }
    return NtCreateFile_Original(handle, access, attributes, status, allocation, fileAttributes, share, disposition,
                                 options, ea, eaLength);
}

NTSTATUS NTAPI NtOpenFile_Hook(PHANDLE handle, ACCESS_MASK access, POBJECT_ATTRIBUTES attributes,
                               PIO_STATUS_BLOCK status, ULONG share, ULONG options) {
    if (!(access & kWriteAccess)) {
        std::wstring picture = PackagePicture(attributes);
        if (!picture.empty()) {
            UNICODE_STRING name{(USHORT)(picture.size() * sizeof(WCHAR)), (USHORT)(picture.size() * sizeof(WCHAR)),
                                picture.data()};
            OBJECT_ATTRIBUTES ours = *attributes;
            ours.ObjectName = &name;
            NTSTATUS result = NtOpenFile_Original(handle, access, &ours, status, share, options);
            if (NT_SUCCESS(result)) {
                return result;
            }
        }
    }
    return NtOpenFile_Original(handle, access, attributes, status, share, options);
}

////////////////////////////////////////////////////////////////////////////////
// Icon theme downloads
//
// Done by Windhawk's own process, not by every program at once. A theme that
// Resource Redirect already downloaded is copied from its folder instead.

HANDLE g_fetchThread;
std::atomic<bool> g_fetchStop;

void DeleteTree(std::wstring const& path) {
    std::wstring from = path + L'\0';
    SHFILEOPSTRUCTW operation{.wFunc = FO_DELETE,
                              .pFrom = from.c_str(),
                              .fFlags = FOF_NO_UI};
    SHFileOperationW(&operation);
}

bool CopyTree(std::wstring const& from, std::wstring const& to) {
    if (!CreateDirectoryW(to.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((from + L"\\*").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) {
        return false;
    }
    bool ok = true;
    do {
        if (wcscmp(find.cFileName, L".") == 0 || wcscmp(find.cFileName, L"..") == 0 ||
            (find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            continue;
        }
        std::wstring source = from + L"\\" + find.cFileName;
        std::wstring target = to + L"\\" + find.cFileName;
        ok = (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? CopyTree(source, target)
                                                                  : CopyFileW(source.c_str(), target.c_str(), FALSE);
    } while (ok && !g_fetchStop && FindNextFileW(search, &find));
    FindClose(search);
    return ok && !g_fetchStop;
}

bool ExtractZip(std::wstring const& zip, std::wstring const& folder) {
    WCHAR system[MAX_PATH];
    if (!GetSystemDirectoryW(system, ARRAYSIZE(system))) {
        return false;
    }
    std::wstring tar = std::wstring(system) + L"\\tar.exe";
    std::wstring commandLine = L"\"" + tar + L"\" -xf \"" + zip + L"\" -C \"" + folder + L"\"";
    STARTUPINFOW startup{.cb = sizeof(startup)};
    PROCESS_INFORMATION process;
    if (!CreateProcessW(tar.c_str(), commandLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &startup, &process)) {
        return false;
    }
    DWORD code = 1;
    if (WaitForSingleObject(process.hProcess, 5 * 60 * 1000) == WAIT_OBJECT_0) {
        GetExitCodeProcess(process.hProcess, &code);
    } else {
        TerminateProcess(process.hProcess, 1);
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return code == 0;
}

bool FetchOne(ThemeChoice const& theme, std::wstring const& root) {
    std::wstring folder = root + L"\\" + theme.name;
    std::wstring partial = folder + L".partial";
    if (FolderExists(partial)) {
        DeleteTree(partial);
    }

    // Resource Redirect keeps its downloads beside this mod's storage.
    std::wstring old = root.substr(0, root.find_last_of(L'\\'));
    old = old.substr(0, old.find_last_of(L'\\')) + L"\\icon-resource-redirect\\" + theme.name;
    bool ok = false;
    if (!ThemeIniIn(old).empty()) {
        Wh_Log(L"Copying %s from Resource Redirect", theme.name.c_str());
        ok = CopyTree(old, partial);
    } else {
        Wh_Log(L"Downloading %s", theme.url.c_str());
        std::wstring zip = folder + L".zip";
        WH_GET_URL_CONTENT_OPTIONS options{.optionsSize = sizeof(options), .targetFilePath = zip.c_str()};
        const WH_URL_CONTENT* content = Wh_GetUrlContent(theme.url.c_str(), &options);
        int status = content ? content->statusCode : 0;
        Wh_FreeUrlContent(content);
        if (status == 200 && !g_fetchStop && CreateDirectoryW(partial.c_str(), nullptr)) {
            ok = ExtractZip(zip, partial);
        } else {
            Wh_Log(L"Download failed (%d)", status);
        }
        DeleteFileW(zip.c_str());
    }

    ok = ok && !ThemeIniIn(partial).empty() && MoveFileExW(partial.c_str(), folder.c_str(), 0);
    if (!ok && FolderExists(partial)) {
        DeleteTree(partial);
    }
    return ok;
}

DWORD WINAPI FetchThreadProc(LPVOID) {
    std::wstring root = ThemesRoot();
    if (root.empty()) {
        return 0;
    }
    CreateDirectoryW(root.substr(0, root.find_last_of(L'\\')).c_str(), nullptr);
    CreateDirectoryW(root.c_str(), nullptr);

    // Windhawk's tray and its window can both be running this.
    HANDLE mutex = CreateMutexW(nullptr, FALSE, L"TourneIcons_ThemeFetch");
    std::wstring failed;
    while (!g_fetchStop) {
        ThemeChoice theme = ParseTheme(ThemeSetting(ReadList()));
        if (theme.name.empty() || theme.name == failed) {
            break;
        }
        DWORD wait = WAIT_TIMEOUT;
        while (mutex && wait == WAIT_TIMEOUT && !g_fetchStop) {
            wait = WaitForSingleObject(mutex, 500);
        }
        if (g_fetchStop) {
            if (wait == WAIT_OBJECT_0 || wait == WAIT_ABANDONED) {
                ReleaseMutex(mutex);
            }
            break;
        }
        bool ok = !ThemeIniIn(root + L"\\" + theme.name).empty() || FetchOne(theme, root);
        if (wait == WAIT_OBJECT_0 || wait == WAIT_ABANDONED) {
            ReleaseMutex(mutex);
        }
        if (!ok) {
            failed = theme.name;
            continue;
        }
        // Picked another theme while this one was on its way: go again.
        if (ParseTheme(ThemeSetting(ReadList())).name == theme.name) {
            break;
        }
    }
    if (mutex) {
        CloseHandle(mutex);
    }
    return 0;
}

// Called in Windhawk's own process when it starts and when settings change.
void FetchTheme() {
    if (g_fetchThread) {
        if (WaitForSingleObject(g_fetchThread, 0) != WAIT_OBJECT_0) {
            return;  // It checks the setting again when it's done.
        }
        CloseHandle(g_fetchThread);
        g_fetchThread = nullptr;
    }
    ThemeChoice theme = ParseTheme(ThemeSetting(ReadList()));
    std::wstring root = ThemesRoot();
    if (theme.name.empty() || root.empty() || !ThemeIniIn(root + L"\\" + theme.name).empty()) {
        return;
    }
    g_fetchStop = false;
    g_fetchThread = CreateThread(nullptr, 0, FetchThreadProc, nullptr, 0, nullptr);
}

void StopFetch() {
    if (g_fetchThread) {
        g_fetchStop = true;
        WaitForSingleObject(g_fetchThread, INFINITE);
        CloseHandle(g_fetchThread);
        g_fetchThread = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Tray icons
//
// A program's tray icon is a handle it makes itself and hands to
// Shell_NotifyIconW, often from a picture of its own rather than from its
// main icon. So a program you gave a new icon has its tray icon swapped here,
// in that program, before Windows sees it. The programs that add tray icons
// are noted too (under HKCU\Software\Tourne\Icons\Tray), for the picker's
// tray list.

using Shell_NotifyIconW_t = decltype(&Shell_NotifyIconW);
Shell_NotifyIconW_t Shell_NotifyIconW_Original;
std::atomic<bool> g_swapTrayIcons{true};
std::atomic<bool> g_trayNoted;

constexpr WCHAR kTrayKey[] = L"Software\\Tourne\\Icons\\Tray";

// This program's tray icons, as it last set them, so a new icon (or the old
// one back) can be shown without waiting for the program.
struct TrayIconState {
    HWND window;
    UINT id;
    bool byGuid;
    GUID guid;
    HICON own;  // The program's own handle.
};
SRWLOCK g_trayLock = SRWLOCK_INIT;
std::vector<TrayIconState> g_trayIcons;
std::map<std::pair<std::wstring, int>, HICON> g_trayMade;  // Never destroyed: Explorer may still be copying one.

int IconSize(HICON icon) {
    ICONINFO info{};
    if (!icon || !GetIconInfo(icon, &info)) {
        return 0;
    }
    BITMAP bitmap{};
    int size = 0;
    if (info.hbmColor && GetObjectW(info.hbmColor, sizeof(bitmap), &bitmap)) {
        size = bitmap.bmWidth;
    } else if (info.hbmMask && GetObjectW(info.hbmMask, sizeof(bitmap), &bitmap)) {
        size = bitmap.bmWidth;
    }
    if (info.hbmColor) DeleteObject(info.hbmColor);
    if (info.hbmMask) DeleteObject(info.hbmMask);
    return size;
}

// The program's new icon stands in for its tray icon only while it has one:
// a program with several (volume, network, a set of buttons) shows each for
// what it means, so they stay its own. Called with g_trayLock held.
bool SwapsTrayIcons(RuleSet const* set) {
    return set && !set->trayIcon.empty() && g_trayIcons.size() <= 1;
}

// Called with g_trayLock held.
HICON MadeTrayIcon(std::wstring const& path, int size) {
    auto key = std::make_pair(path, size);
    auto it = g_trayMade.find(key);
    if (it != g_trayMade.end()) {
        return it->second;
    }
    t_busy++;
    HICON icon = (HICON)LoadImageW(nullptr, path.c_str(), IMAGE_ICON, size, size, LR_LOADFROMFILE);
    t_busy--;
    if (icon) {
        g_trayMade[key] = icon;
    }
    return icon;
}

void NoteTrayProgram(NOTIFYICONDATAW const* data) {
    if (g_trayNoted.exchange(true)) {
        return;
    }
    WCHAR path[MAX_PATH * 2];
    DWORD length = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) {
        return;
    }
    WCHAR tip[128] = L"";
    if ((data->uFlags & NIF_TIP) && data->cbSize >= offsetof(NOTIFYICONDATAW, szTip) + sizeof(WCHAR) * 64) {
        wcsncpy_s(tip, data->szTip, _TRUNCATE);
    }
    RegSetKeyValueW(HKEY_CURRENT_USER, kTrayKey, path, REG_SZ, tip, (DWORD)((wcslen(tip) + 1) * sizeof(WCHAR)));
}

void RefreshTrayIcons();
BOOL CALLBACK TellBarIcons(HWND window, LPARAM anyProgram);

BOOL WINAPI Shell_NotifyIconW_Hook(DWORD message, PNOTIFYICONDATAW data) {
    if (!data || t_busy || (message != NIM_ADD && message != NIM_MODIFY && message != NIM_DELETE)) {
        return Shell_NotifyIconW_Original(message, data);
    }
    bool byGuid = data->cbSize >= sizeof(NOTIFYICONDATAW) && (data->uFlags & NIF_GUID);
    auto same = [&](TrayIconState const& s) {
        return byGuid ? s.byGuid && IsEqualGUID(s.guid, data->guidItem) : !s.byGuid && s.window == data->hWnd && s.id == data->uID;
    };
    if (message == NIM_DELETE) {
        AcquireSRWLockExclusive(&g_trayLock);
        std::erase_if(g_trayIcons, same);
        ReleaseSRWLockExclusive(&g_trayLock);
        return Shell_NotifyIconW_Original(message, data);
    }
    if (!(data->uFlags & NIF_ICON)) {
        return Shell_NotifyIconW_Original(message, data);
    }
    NoteTrayProgram(data);

    HICON swap = nullptr;
    bool second = false;
    RuleSet* set = g_swapTrayIcons.load(std::memory_order_relaxed) ? EnsureRules() : nullptr;
    AcquireSRWLockExclusive(&g_trayLock);
    auto it = std::find_if(g_trayIcons.begin(), g_trayIcons.end(), same);
    if (it == g_trayIcons.end()) {
        g_trayIcons.push_back({data->hWnd, data->uID, byGuid, byGuid ? data->guidItem : GUID{}, data->hIcon});
        second = g_trayIcons.size() == 2 && set && !set->trayIcon.empty();
    } else {
        it->own = data->hIcon;
    }
    if (SwapsTrayIcons(set)) {
        int size = IconSize(data->hIcon);
        swap = MadeTrayIcon(set->trayIcon, size > 0 ? size : GetSystemMetrics(SM_CXSMICON));
    }
    ReleaseSRWLockExclusive(&g_trayLock);

    if (!swap) {
        BOOL result = Shell_NotifyIconW_Original(message, data);
        if (second) {
            RefreshTrayIcons();  // The first one gets its own icon back.
        }
        return result;
    }
    NOTIFYICONDATAW copy{};
    memcpy(&copy, data, std::min<size_t>(data->cbSize, sizeof(copy)));
    copy.hIcon = swap;
    BOOL result = Shell_NotifyIconW_Original(message, &copy);
    if (!result && message == NIM_ADD) {
        // Already shown (it was put back after TaskbarCreated): change it in place.
        result = Shell_NotifyIconW_Original(NIM_MODIFY, &copy);
    }
    return result;
}

// When the mod loads into a program that already shows a tray icon (after an
// update, or turning the mod back on), that icon was set before the hook, so
// it's the program's own. Once, a few seconds after loading, a program that has
// a new icon is told the taskbar was made again, and it puts its icons back
// through the hook. Programs without a top-level window, or without a new
// icon, are left alone and don't read the rules for it.
HANDLE g_trayNudgeTimer;

BOOL CALLBACK NudgeOwnWindow(HWND window, LPARAM message) {
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (pid == GetCurrentProcessId()) {
        PostMessageW(window, (UINT)message, 0, 0);
    }
    return TRUE;
}

BOOL CALLBACK FindOwnWindow(HWND window, LPARAM found) {  // (a named function: 32-bit callbacks are __stdcall)
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (pid == GetCurrentProcessId()) {
        *(bool*)found = true;
        return FALSE;
    }
    return TRUE;
}

void CALLBACK NudgeTrayIcons(PVOID, BOOLEAN) {
    bool hasWindow = false;
    EnumWindows(FindOwnWindow, (LPARAM)&hasWindow);
    if (!hasWindow || !g_swapTrayIcons) {
        return;
    }
    try {
        RuleSet* set = EnsureRules();
        if (!set || set->trayIcon.empty()) {
            return;
        }
    } catch (...) {
        return;
    }
    UINT message = RegisterWindowMessageW(L"TaskbarCreated");
    EnumWindows(NudgeOwnWindow, message);
    for (HWND w = nullptr; (w = FindWindowExW(HWND_MESSAGE, w, nullptr, nullptr));) {
        NudgeOwnWindow(w, message);
    }
}

void StartTrayNudge() {
    CreateTimerQueueTimer(&g_trayNudgeTimer, nullptr, NudgeTrayIcons, nullptr, 3000, 0, WT_EXECUTEONLYONCE);
}

void StopTrayNudge() {
    if (g_trayNudgeTimer) {
        DeleteTimerQueueTimer(nullptr, g_trayNudgeTimer, INVALID_HANDLE_VALUE);
        g_trayNudgeTimer = nullptr;
    }
}

// After the rules changed: this program's tray icons shown again, with the
// new icon or the program's own.
void RefreshTrayIcons() {
    if (!Shell_NotifyIconW_Original) {
        return;
    }
    RuleSet* set = g_swapTrayIcons.load(std::memory_order_relaxed) ? g_ruleSet.load(std::memory_order_acquire) : nullptr;
    std::vector<std::pair<TrayIconState, HICON>> updates;
    AcquireSRWLockExclusive(&g_trayLock);
    bool swaps = SwapsTrayIcons(set);
    for (auto const& state : g_trayIcons) {
        HICON icon = state.own;
        if (swaps) {
            int size = IconSize(state.own);
            icon = MadeTrayIcon(set->trayIcon, size > 0 ? size : GetSystemMetrics(SM_CXSMICON));
        }
        if (icon && IconSize(icon) > 0) {
            updates.push_back({state, icon});
        }
    }
    ReleaseSRWLockExclusive(&g_trayLock);
    for (auto const& [state, icon] : updates) {
        NOTIFYICONDATAW data{.cbSize = sizeof(data), .hWnd = state.window, .uID = state.id, .uFlags = NIF_ICON};
        if (state.byGuid) {
            data.uFlags |= NIF_GUID;
            data.guidItem = state.guid;
        }
        data.hIcon = icon;
        Shell_NotifyIconW_Original(NIM_MODIFY, &data);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Watching your changes
//
// Every program with the mod waits on the storage folder (a thread pool wait,
// so nothing runs until the folder changes) and takes in the new list when
// the picker saves it.

HANDLE g_listChange = INVALID_HANDLE_VALUE;
HANDLE g_listWait;

void OnRulesChanged() {
    RefreshTrayIcons();
    EnumWindows(TellBarIcons, 0);
}

// Runs on a thread pool thread inside every program the mod is in, so it must
// never let anything escape: one save fires it twice (the new file, then the
// swap), and the two used to run at once. Now one runs at a time and a change
// that lands meanwhile makes it go round once more.
SRWLOCK g_listChangeLock = SRWLOCK_INIT;
std::atomic<bool> g_listChangeAgain{false};

void CALLBACK ListChanged(PVOID, BOOLEAN) {
    FindNextChangeNotification(g_listChange);
    g_listChangeAgain = true;
    if (!TryAcquireSRWLockExclusive(&g_listChangeLock)) {
        return;  // The one running sees the flag.
    }
    while (g_listChangeAgain.exchange(false)) {
        try {
            if (RebuildRules()) {
                OnRulesChanged();
            }
        } catch (...) {
            Wh_Log(L"Couldn't read the changed list; keeping the icons as they were");
        }
    }
    ReleaseSRWLockExclusive(&g_listChangeLock);
}

void WatchList() {
    std::wstring root = StorageRoot();
    if (root.empty()) {
        return;
    }
    CreateDirectoryW(root.c_str(), nullptr);
    g_listChange = FindFirstChangeNotificationW(root.c_str(), FALSE,
                                                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);
    if (g_listChange == INVALID_HANDLE_VALUE) {
        return;
    }
    if (!RegisterWaitForSingleObject(&g_listWait, g_listChange, ListChanged, nullptr, INFINITE, WT_EXECUTEDEFAULT)) {
        g_listWait = nullptr;
        FindCloseChangeNotification(g_listChange);
        g_listChange = INVALID_HANDLE_VALUE;
    }
}

void StopWatchingList() {
    if (g_listWait) {
        UnregisterWaitEx(g_listWait, INVALID_HANDLE_VALUE);
        g_listWait = nullptr;
    }
    if (g_listChange != INVALID_HANDLE_VALUE) {
        FindCloseChangeNotification(g_listChange);
        g_listChange = INVALID_HANDLE_VALUE;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Making icons
//
// Any picture becomes an .ico with every size Windows asks for. A size the
// picture already has is used as it is (so a pixel-art icon's own 16 stays
// crisp); the others are scaled from the nearest larger one, or blown up
// whole when the picture is smaller (pixel for pixel when it divides evenly).

constexpr UINT kIconSizes[] = {256, 128, 96, 64, 48, 40, 32, 24, 20, 16};


bool DecodeFrames(IWICImagingFactory* wic, std::vector<BYTE> const& file, std::vector<Pixels>* frames) {
    Com<IStream> stream;
    stream.p = StreamOn(file);
    Com<IWICBitmapDecoder> decoder;
    UINT count = 0;
    if (!stream || FAILED(wic->CreateDecoderFromStream(stream.p, nullptr, WICDecodeMetadataCacheOnDemand, &decoder)) ||
        FAILED(decoder->GetFrameCount(&count))) {
        return false;
    }
    for (UINT i = 0; i < count && i < 64; i++) {
        Com<IWICBitmapFrameDecode> frame;
        Com<IWICFormatConverter> converter;
        Pixels pixels;
        if (FAILED(decoder->GetFrame(i, &frame)) || FAILED(wic->CreateFormatConverter(&converter)) ||
            FAILED(converter->Initialize(frame.p, kWicFormatPbgra, WICBitmapDitherTypeNone, nullptr, 0,
                                         WICBitmapPaletteTypeCustom)) ||
            FAILED(converter->GetSize(&pixels.width, &pixels.height)) || !pixels.width || !pixels.height ||
            pixels.width > 4096 || pixels.height > 4096) {
            continue;
        }
        pixels.bgra.resize((size_t)pixels.width * pixels.height * 4);
        if (SUCCEEDED(converter->CopyPixels(nullptr, pixels.width * 4, (UINT)pixels.bgra.size(), pixels.bgra.data()))) {
            frames->push_back(std::move(pixels));
        }
        // A GIF's later frames are its animation.
        if (count > 1 && frames->size() == 1 && file.size() > 3 && memcmp(file.data(), "GIF", 3) == 0) {
            break;
        }
    }
    return !frames->empty();
}

// An icon inside a program or library, as the bytes of an .ico file. A
// negative number is a resource id, as in "shell32.dll,-16".
std::vector<BYTE> IcoFromModule(std::wstring const& path, int number) {
    std::vector<BYTE> ico;
    HMODULE module = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (!module) {
        return ico;
    }
    LPCWSTR group = nullptr;
    GroupSearch search{number >= 0 ? number : 0};
    if (number < 0) {
        group = MAKEINTRESOURCEW(-number);
    } else {
        EnumResourceNamesW(module, RT_GROUP_ICON, FindIconGroup, (LONG_PTR)&search);
        if (search.found) {
            group = search.name.empty() ? MAKEINTRESOURCEW(search.id) : search.name.c_str();
        }
    }
    HRSRC found = group ? FindResourceW(module, group, RT_GROUP_ICON) : nullptr;
    HGLOBAL loaded = found ? LoadResource(module, found) : nullptr;
    DWORD size = found ? SizeofResource(module, found) : 0;
    auto const* dir = (const BYTE*)loaded;
    if (dir && size >= 6) {
        WORD count = *(const WORD*)(dir + 4);
        if (6 + (size_t)count * sizeof(GroupEntry) <= size) {
            std::vector<std::pair<IcoEntry, std::vector<BYTE>>> images;
            for (WORD i = 0; i < count; i++) {
                GroupEntry entry;
                memcpy(&entry, dir + 6 + i * sizeof(GroupEntry), sizeof(entry));
                HRSRC image = FindResourceW(module, MAKEINTRESOURCEW(entry.id), RT_ICON);
                HGLOBAL data = image ? LoadResource(module, image) : nullptr;
                DWORD bytes = image ? SizeofResource(module, image) : 0;
                if (!data || !bytes) {
                    continue;
                }
                IcoEntry out;
                memcpy(&out, &entry, 12);
                out.bytes = bytes;
                images.push_back({out, std::vector<BYTE>((const BYTE*)data, (const BYTE*)data + bytes)});
            }
            if (!images.empty()) {
                ico.resize(6 + images.size() * sizeof(IcoEntry));
                WORD header[3] = {0, 1, (WORD)images.size()};
                memcpy(ico.data(), header, 6);
                for (size_t i = 0; i < images.size(); i++) {
                    images[i].first.offset = (DWORD)ico.size();
                    memcpy(ico.data() + 6 + i * sizeof(IcoEntry), &images[i].first, sizeof(IcoEntry));
                    ico.insert(ico.end(), images[i].second.begin(), images[i].second.end());
                }
            }
        }
    }
    FreeLibrary(module);
    return ico;
}

bool IsModuleFile(std::wstring const& path) {
    for (PCWSTR type : {L".exe", L".dll", L".mun", L".icl", L".cpl", L".ocx", L".scr"}) {
        if (EndsWithI(path, type)) {
            return true;
        }
    }
    return false;
}

// One size, from the frames: the frame of that size when there is one, else
// the nearest larger frame scaled down, else the largest scaled up.
bool FrameAt(IWICImagingFactory* wic, std::vector<Pixels> const& frames, UINT size, Pixels* out) {
    Pixels const* exact = nullptr;
    Pixels const* larger = nullptr;
    Pixels const* largest = nullptr;
    for (auto const& f : frames) {
        UINT side = std::max(f.width, f.height);
        if (f.width == size && f.height == size && (!exact || f.bgra.size() >= exact->bgra.size())) {
            exact = &f;
        }
        if (side >= size && (!larger || side < std::max(larger->width, larger->height))) {
            larger = &f;
        }
        if (!largest || side > std::max(largest->width, largest->height)) {
            largest = &f;
        }
    }
    if (exact) {
        *out = *exact;
        return true;
    }
    Pixels const* from = larger ? larger : largest;
    if (!from) {
        return false;
    }
    UINT side = std::max(from->width, from->height);
    UINT width = std::max(1u, from->width * size / side), height = std::max(1u, from->height * size / side);
    WICBitmapInterpolationMode mode = side > size ? WICBitmapInterpolationModeFant
                                      : size % side == 0 ? WICBitmapInterpolationModeNearestNeighbor
                                                         : (WICBitmapInterpolationMode)4;  // High quality cubic.
    Com<IWICBitmap> source;
    Com<IWICBitmapScaler> scaler;
    if (FAILED(wic->CreateBitmapFromMemory(from->width, from->height, kWicFormatPbgra, from->width * 4,
                                           (UINT)from->bgra.size(), (BYTE*)from->bgra.data(), &source)) ||
        FAILED(wic->CreateBitmapScaler(&scaler))) {
        return false;
    }
    if (FAILED(scaler->Initialize(source.p, width, height, mode)) &&
        FAILED(scaler->Initialize(source.p, width, height, WICBitmapInterpolationModeCubic))) {
        return false;
    }
    std::vector<BYTE> scaled((size_t)width * height * 4);
    if (FAILED(scaler->CopyPixels(nullptr, width * 4, (UINT)scaled.size(), scaled.data()))) {
        return false;
    }
    // Centred on a clear square.
    out->width = out->height = size;
    out->bgra.assign((size_t)size * size * 4, 0);
    UINT left = (size - width) / 2, top = (size - height) / 2;
    for (UINT y = 0; y < height; y++) {
        memcpy(&out->bgra[((size_t)(top + y) * size + left) * 4], &scaled[(size_t)y * width * 4], (size_t)width * 4);
    }
    return true;
}

// An icon image as an .ico holds it: a DIB of twice the height (colour, then
// a mask of the clear pixels), with straight alpha.
std::vector<BYTE> IconDib(Pixels const& pixels) {
    UINT w = pixels.width, h = pixels.height;
    size_t maskRow = ((w + 31) / 32) * 4;
    std::vector<BYTE> dib(sizeof(BITMAPINFOHEADER) + (size_t)w * h * 4 + maskRow * h, 0);
    BITMAPINFOHEADER header{sizeof(header), (LONG)w, (LONG)(h * 2), 1, 32, BI_RGB,
                            (DWORD)(dib.size() - sizeof(BITMAPINFOHEADER))};
    memcpy(dib.data(), &header, sizeof(header));
    BYTE* color = dib.data() + sizeof(header);
    BYTE* mask = color + (size_t)w * h * 4;
    for (UINT y = 0; y < h; y++) {
        const BYTE* in = &pixels.bgra[(size_t)y * w * 4];
        BYTE* row = color + (size_t)(h - 1 - y) * w * 4;
        BYTE* maskLine = mask + (h - 1 - y) * maskRow;
        for (UINT x = 0; x < w; x++) {
            BYTE a = in[x * 4 + 3];
            for (int c = 0; c < 3; c++) {
                row[x * 4 + c] = a ? (BYTE)std::min(255, in[x * 4 + c] * 255 / a) : 0;
            }
            row[x * 4 + 3] = a;
            if (!a) {
                maskLine[x / 8] |= (BYTE)(0x80 >> (x % 8));
            }
        }
    }
    return dib;
}

// A picture (or an icon in a program, with number >= 0 or a resource id
// below 0) as a complete .ico. Empty when it can't be read.
std::vector<BYTE> MakeIco(std::wstring const& path, int number) {
    std::vector<BYTE> ico;
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    {
        Com<IWICImagingFactory> wic;
        if (SUCCEEDED(CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                       __uuidof(IWICImagingFactory), (void**)&wic))) {
            bool svg = EndsWithI(path, L".svg");
            std::vector<BYTE> file = IsModuleFile(path) ? IcoFromModule(path, number) : ReadWholeFile(path);
            std::vector<Pixels> frames;
            if (!file.empty() && (svg || DecodeFrames(wic.p, file, &frames))) {
                std::vector<std::pair<UINT, std::vector<BYTE>>> images;
                for (UINT size : kIconSizes) {
                    Pixels pixels;
                    bool ok = svg ? RenderSvg(wic.p, file, size, size, &pixels) : FrameAt(wic.p, frames, size, &pixels);
                    std::vector<BYTE> data;
                    if (ok) {
                        if (size > 64) {
                            EncodePng(wic.p, pixels, &data);
                        } else {
                            data = IconDib(pixels);
                        }
                    }
                    if (!data.empty()) {
                        images.push_back({size, std::move(data)});
                    }
                }
                if (!images.empty()) {
                    ico.resize(6 + images.size() * sizeof(IcoEntry));
                    WORD header[3] = {0, 1, (WORD)images.size()};
                    memcpy(ico.data(), header, 6);
                    for (size_t i = 0; i < images.size(); i++) {
                        IcoEntry entry{(BYTE)(images[i].first >= 256 ? 0 : images[i].first),
                                       (BYTE)(images[i].first >= 256 ? 0 : images[i].first), 0, 0, 1, 32,
                                       (DWORD)images[i].second.size(), (DWORD)ico.size()};
                        memcpy(ico.data() + 6 + i * sizeof(IcoEntry), &entry, sizeof(entry));
                        ico.insert(ico.end(), images[i].second.begin(), images[i].second.end());
                    }
                }
            }
        }
    }
    if (SUCCEEDED(com)) {
        CoUninitialize();
    }
    return ico;
}

// Saved in the storage folder's icons\ under a name made from what it's for,
// so the folder reads sensibly. Returns the path.
std::wstring SaveIco(std::vector<BYTE> const& ico, std::wstring const& label) {
    std::wstring root = StorageRoot();
    if (root.empty() || ico.empty()) {
        return {};
    }
    std::wstring folder = root + L"\\icons";
    CreateDirectoryW(folder.c_str(), nullptr);
    std::wstring name;
    for (WCHAR c : label) {
        if (iswalnum(c)) {
            name += c;
        } else if (!name.empty() && name.back() != L'-') {
            name += L'-';
        }
        if (name.size() >= 40) {
            break;
        }
    }
    while (!name.empty() && name.back() == L'-') {
        name.pop_back();
    }
    DWORD hash = 2166136261u;
    for (BYTE b : ico) {
        hash = (hash ^ b) * 16777619u;
    }
    WCHAR suffix[16];
    swprintf_s(suffix, L"%08x", hash);
    std::wstring path = folder + L"\\" + (name.empty() ? L"icon" : name) + L"-" + suffix + L".ico";
    if (FileExists(path)) {
        return path;
    }
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return {};
    }
    DWORD written = 0;
    bool ok = WriteFile(file, ico.data(), (DWORD)ico.size(), &written, nullptr) && written == ico.size();
    CloseHandle(file);
    if (!ok) {
        DeleteFileW(path.c_str());
        return {};
    }
    return path;
}

////////////////////////////////////////////////////////////////////////////////
// Changing your list (the picker, in the tool process)

// What a program row matches: the program's file name in any folder, so it
// still matches after an update moves it (app-1.2.3\Discord.exe), unless the
// name is one many programs share.
std::wstring ProgramPattern(std::wstring const& path) {
    std::wstring lower = Lower(path);
    size_t slash = lower.find_last_of(L"\\/");
    std::wstring name = slash == std::wstring::npos ? lower : lower.substr(slash + 1);
    for (PCWSTR common : {L"launcher.exe", L"update.exe", L"updater.exe", L"setup.exe", L"app.exe", L"main.exe",
                          L"client.exe", L"game.exe", L"start.exe", L"run.exe", L"helper.exe", L"service.exe"}) {
        if (name == common) {
            return lower;
        }
    }
    return L"*\\" + name;
}

std::wstring ReadDefaultValue(HKEY root, std::wstring const& key, bool* exists) {
    WCHAR buffer[2048];
    DWORD size = sizeof(buffer);
    LSTATUS status = RegGetValueW(root, key.c_str(), nullptr, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                  nullptr, buffer, &size);
    *exists = status == ERROR_SUCCESS;
    return *exists ? std::wstring(buffer) : std::wstring();
}

// A file type's icon is Windows' own setting, under your account: the
// program that opens it (its ProgID) gets a DefaultIcon of yours, and the
// one it had there before is kept to put back.
//
// A file type nothing opens has no ProgID of its own: Windows shows it with
// "Unknown"'s icon, which every such type shares, so a DefaultIcon there
// would change them all at once. Such a type gets a ProgID of its own
// instead, holding only its icon and name ("previous" then starts with +).
bool OwnFileType(ListRow const& row) {
    return row.progId.empty() || _wcsicmp(row.progId.c_str(), L"Unknown") == 0;
}

std::wstring OwnProgId(ListRow const& row) {
    return L"TourneIcons" + Lower(row.original);
}

bool SetRegText(std::wstring const& key, PCWSTR name, std::wstring const& value, DWORD type = REG_SZ) {
    return RegSetKeyValueW(HKEY_CURRENT_USER, key.c_str(), name, type, value.c_str(),
                           (DWORD)((value.size() + 1) * sizeof(WCHAR))) == ERROR_SUCCESS;
}

bool SetFileTypeIcon(ListRow* row) {
    std::wstring value = row->icon + L",0";
    if (OwnFileType(*row)) {
        if (row->original.size() < 2 || row->original[0] != L'.') {
            return false;
        }
        std::wstring extKey = L"Software\\Classes\\" + row->original;
        if (row->previous.empty() || row->previous[0] != L'+') {
            bool exists = false;
            std::wstring before = ReadDefaultValue(HKEY_CURRENT_USER, extKey, &exists);
            row->previous = exists && _wcsnicmp(before.c_str(), L"TourneIcons.", 12) != 0 ? L"+=" + before : L"+-";
        }
        std::wstring own = OwnProgId(*row);
        std::wstring name = row->label;
        if (size_t paren = name.rfind(L" (."); paren != std::wstring::npos) name.resize(paren);
        return SetRegText(L"Software\\Classes\\" + own, nullptr, name) &&
               SetRegText(L"Software\\Classes\\" + own + L"\\DefaultIcon", nullptr, value, REG_EXPAND_SZ) &&
               SetRegText(extKey, nullptr, own);
    }
    std::wstring key = L"Software\\Classes\\" + row->progId + L"\\DefaultIcon";
    if (row->previous.empty()) {
        bool exists = false;
        std::wstring before = ReadDefaultValue(HKEY_CURRENT_USER, key, &exists);
        row->previous = exists ? L"=" + before : L"-";
    }
    return SetRegText(key, nullptr, value, REG_EXPAND_SZ);
}

void PutBackFileTypeIcon(ListRow const& row) {
    if (!row.previous.empty() && row.previous[0] == L'+') {
        std::wstring extKey = L"Software\\Classes\\" + row.original;
        if (row.previous.size() > 2 && row.previous[1] == L'=') {
            SetRegText(extKey, nullptr, row.previous.substr(2));
        } else {
            RegDeleteKeyValueW(HKEY_CURRENT_USER, extKey.c_str(), nullptr);
        }
        RegDeleteTreeW(HKEY_CURRENT_USER, (L"Software\\Classes\\" + OwnProgId(row)).c_str());
        return;
    }
    if (row.progId.empty()) {
        return;
    }
    std::wstring key = L"Software\\Classes\\" + row.progId + L"\\DefaultIcon";
    if (row.previous.size() > 1 && row.previous[0] == L'=') {
        SetRegText(key, nullptr, row.previous.substr(1), REG_EXPAND_SZ);
    } else {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, key.c_str(), nullptr);
        RegDeleteKeyW(HKEY_CURRENT_USER, key.c_str());  // Only goes when it's empty.
    }
}

// Earlier versions gave file types nothing opens "Unknown"'s own icon, so
// they all showed the last one changed. Puts Unknown's icon back as it was
// and gives each of those types its own. Returns how many it moved.
int SeparateUnknownFileTypes() {
    std::vector<ListRow> rows = ReadList();
    std::set<std::wstring> ours;
    for (auto const& row : rows) ours.insert(Lower(row.icon + L",0"));
    std::wstring const unknownKey = L"Software\\Classes\\Unknown\\DefaultIcon";
    std::wstring first;  // What Unknown had before any of them.
    int moved = 0;
    for (auto& row : rows) {
        if (row.kind != L"filetype" || !OwnFileType(row) || (!row.previous.empty() && row.previous[0] == L'+')) {
            continue;
        }
        if (first.empty() && !(row.previous.size() > 1 && ours.count(Lower(row.previous.substr(1))))) {
            first = row.previous.empty() ? L"-" : row.previous;
        }
        row.previous.clear();
        if (SetFileTypeIcon(&row)) moved++;
    }
    if (!moved) {
        return 0;
    }
    bool exists = false;
    std::wstring now = ReadDefaultValue(HKEY_CURRENT_USER, unknownKey, &exists);
    if (exists && ours.count(Lower(now))) {
        if (first.size() > 1 && first[0] == L'=') {
            SetRegText(unknownKey, nullptr, first.substr(1), REG_EXPAND_SZ);
        } else {
            RegDeleteKeyValueW(HKEY_CURRENT_USER, unknownKey.c_str(), nullptr);
            RegDeleteKeyW(HKEY_CURRENT_USER, unknownKey.c_str());
        }
    }
    WriteList(rows);
    return moved;
}

// Tourne Desktop's taskbar (and the suite's test taskbar) keeps the icons
// it read; WM_APP + 4 has it read them again, through the new rules, so its
// buttons change at once like its own Button icons do. A program whose rules
// just changed tells its own bars; the picker tells every bar, after the save.
constexpr UINT kTourneBarIcons = WM_APP + 4;

BOOL CALLBACK TellBarIcons(HWND window, LPARAM anyProgram) {
    DWORD process = 0;
    GetWindowThreadProcessId(window, &process);
    WCHAR name[64];
    if ((anyProgram || process == GetCurrentProcessId()) && GetClassNameW(window, name, ARRAYSIZE(name)) &&
        wcsncmp(name, L"TourneTaskbarTest_", 18) == 0) {
        PostMessageW(window, kTourneBarIcons, 0, 0);
    }
    return TRUE;
}

// Explorer keeps the icons it has drawn in a list of its own, and that alone
// doesn't empty it. Changing the size of the shell's icons does: Explorer
// throws the list away and draws every icon again (through the new
// redirects), at once and without restarting. So the size is set one pixel
// smaller and straight back.
void RedrawShellIcons() {
    constexpr WCHAR kMetrics[] = L"Control Panel\\Desktop\\WindowMetrics";
    constexpr WCHAR kSize[] = L"Shell Icon Size";
    WCHAR before[16] = L"";
    DWORD size = sizeof(before);
    bool had = RegGetValueW(HKEY_CURRENT_USER, kMetrics, kSize, RRF_RT_REG_SZ, nullptr, before, &size) == ERROR_SUCCESS;
    int pixels = had ? _wtoi(before) : 0;
    if (pixels < 16 || pixels > 256) pixels = 32;
    auto tell = [] {
        DWORD_PTR result;
        SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, SPI_SETNONCLIENTMETRICS, (LPARAM)L"WindowMetrics",
                            SMTO_ABORTIFHUNG, 2000, &result);
    };
    std::wstring smaller = std::to_wstring(pixels - 1);
    if (RegSetKeyValueW(HKEY_CURRENT_USER, kMetrics, kSize, REG_SZ, smaller.c_str(),
                        (DWORD)((smaller.size() + 1) * sizeof(WCHAR))) != ERROR_SUCCESS) {
        return;
    }
    tell();
    if (had) {
        RegSetKeyValueW(HKEY_CURRENT_USER, kMetrics, kSize, REG_SZ, before, (DWORD)((wcslen(before) + 1) * sizeof(WCHAR)));
    } else {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kMetrics, kSize);
    }
    tell();
}

// Explorer and the rest are told to read icons again. Later than the save,
// so every program has taken in the new list by then.
void TellWindowsIconsChanged() {
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, nullptr, nullptr);
    RedrawShellIcons();
    EnumWindows(TellBarIcons, TRUE);  // (shortcuts the lasting version changed, too)
}

void EndExplorers() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;
    PROCESSENTRY32W entry{.dwSize = sizeof(entry)};
    for (BOOL more = Process32FirstW(snapshot, &entry); more; more = Process32NextW(snapshot, &entry)) {
        if (_wcsicmp(entry.szExeFile, L"explorer.exe") != 0) continue;
        if (HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID)) {
            TerminateProcess(process, 1);
            CloseHandle(process);
        }
    }
    CloseHandle(snapshot);
}

bool ExplorerRunning() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return true;
    PROCESSENTRY32W entry{.dwSize = sizeof(entry)};
    bool found = false;
    for (BOOL more = Process32FirstW(snapshot, &entry); more && !found; more = Process32NextW(snapshot, &entry)) {
        found = _wcsicmp(entry.szExeFile, L"explorer.exe") == 0;
    }
    CloseHandle(snapshot);
    return found;
}

// Explorer's icon and thumbnail caches deleted (what Clear Icon Cache.ps1
// does), for icons that still show old pictures after Refresh icons. Explorer
// holds them open and starts again the moment it's closed, so it's closed and
// they're deleted until none is left; then it's started again if Windows (or
// Tourne Desktop) didn't. Returns how many files couldn't be deleted.
int ClearExplorerCaches() {
    PWSTR local = nullptr;
    std::wstring folder;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local)) && local) {
        folder = std::wstring(local) + L"\\Microsoft\\Windows\\Explorer";
    }
    CoTaskMemFree(local);
    if (folder.empty()) return -1;
    int left = 0;
    for (int attempt = 0; attempt < 20; attempt++) {
        EndExplorers();
        Sleep(400);
        left = 0;
        for (PCWSTR pattern : {L"\\thumbcache_*.db", L"\\iconcache_*.db"}) {
            WIN32_FIND_DATAW find;
            HANDLE search = FindFirstFileW((folder + pattern).c_str(), &find);
            if (search == INVALID_HANDLE_VALUE) continue;
            do {
                if (!DeleteFileW((folder + L"\\" + find.cFileName).c_str())) left++;
            } while (FindNextFileW(search, &find));
            FindClose(search);
        }
        if (!left) break;
    }
    // Windows (or Tourne Desktop, as the shell) starts Explorer again by
    // itself; only when neither did is it started here.
    for (int wait = 0; wait < 20 && !ExplorerRunning(); wait++) Sleep(200);
    if (!ExplorerRunning()) {
        WCHAR windows[MAX_PATH];
        if (GetWindowsDirectoryW(windows, ARRAYSIZE(windows))) {
            std::wstring program = std::wstring(windows) + L"\\explorer.exe";
            ShellExecuteW(nullptr, nullptr, program.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
    }
    return left;
}

// Windows' icon cache, refreshed: what "Refresh icons" does.
void RefreshIconCache() {
    TellWindowsIconsChanged();
    WCHAR system[MAX_PATH];
    if (GetSystemDirectoryW(system, ARRAYSIZE(system))) {
        std::wstring program = std::wstring(system) + L"\\ie4uinit.exe";
        std::wstring commandLine = L"\"" + program + L"\" -show";
        STARTUPINFOW startup{.cb = sizeof(startup)};
        PROCESS_INFORMATION process;
        if (CreateProcessW(program.c_str(), commandLine.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                           nullptr, &startup, &process)) {
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Store apps (the tool makes their pictures)
//
// A Store app's logos are named in its AppxManifest.xml (Square44x44Logo for
// the taskbar, Start's list and Alt+Tab; the tiles; the package's own Logo),
// and each comes as several files: Assets\AppList.png can be
// AppList.targetsize-24_altform-unplated.png, AppList.scale-200.png and so on,
// some in the app's resource packages (folders of their own beside it). Every
// one of them gets a picture made from your new icon, at its own size.

namespace store {

using FindPackagesByPackageFamily_t = LONG(WINAPI*)(PCWSTR, UINT32, UINT32*, PWSTR*, UINT32*, WCHAR*, UINT32*);
using GetPackagePathByFullName_t = LONG(WINAPI*)(PCWSTR, UINT32*, PWSTR);
using GetPackagePathByFullName2_t = LONG(WINAPI*)(PCWSTR, int, UINT32*, PWSTR);
using ProcessString_t = LONG(WINAPI*)(HANDLE, UINT32*, PWSTR);

constexpr UINT32 kFilterHead = 0x10, kFilterDirect = 0x20, kFilterResource = 0x40;

struct Api {
    FindPackagesByPackageFamily_t findPackages = nullptr;
    GetPackagePathByFullName_t packagePath = nullptr;
    GetPackagePathByFullName2_t packagePath2 = nullptr;  // Windows 11: an app's outside folder
    ProcessString_t packageFullName = nullptr;  // GetPackageFullName
    ProcessString_t appId = nullptr;            // GetApplicationUserModelId
};

Api const& A() {
    static Api api = [] {
        Api a;
        HMODULE kernel = GetModuleHandleW(L"kernelbase.dll");
        if (!kernel) kernel = GetModuleHandleW(L"kernel32.dll");
        if (kernel) {
            a.findPackages = (FindPackagesByPackageFamily_t)GetProcAddress(kernel, "FindPackagesByPackageFamily");
            a.packagePath = (GetPackagePathByFullName_t)GetProcAddress(kernel, "GetPackagePathByFullName");
            a.packagePath2 = (GetPackagePathByFullName2_t)GetProcAddress(kernel, "GetPackagePathByFullName2");
            a.packageFullName = (ProcessString_t)GetProcAddress(kernel, "GetPackageFullName");
            a.appId = (ProcessString_t)GetProcAddress(kernel, "GetApplicationUserModelId");
        }
        return a;
    }();
    return api;
}

std::wstring PathOf(std::wstring const& fullName) {
    UINT32 length = MAX_PATH * 2;
    WCHAR path[MAX_PATH * 2];
    if (!A().packagePath || A().packagePath(fullName.c_str(), &length, path) != ERROR_SUCCESS) {
        return {};
    }
    return path;
}

// An app installed outside WindowsApps (a "sparse" package, like the Copilot
// app built on Edge) keeps only its manifest there; its files, logos too, are
// in its outside folder. Empty for every other app.
std::wstring OutsidePathOf(std::wstring const& fullName) {
    constexpr int kEffectiveExternal = 5, kMachineExternal = 3, kUserExternal = 4;  // PackagePathType
    if (!A().packagePath2) return {};
    for (int type : {kEffectiveExternal, kMachineExternal, kUserExternal}) {
        UINT32 length = MAX_PATH * 2;
        WCHAR path[MAX_PATH * 2];
        if (A().packagePath2(fullName.c_str(), type, &length, path) == ERROR_SUCCESS && path[0]) {
            std::wstring outside = path;
            while (!outside.empty() && outside.back() == L'\\') outside.pop_back();
            if (_wcsicmp(outside.c_str(), PathOf(fullName).c_str()) != 0) return outside;
        }
    }
    return {};
}

// The family's packages ("Microsoft.WindowsCalculator_8wekyb3d8bbwe"): the
// app's own first, then its resource packages.
std::vector<std::wstring> PackagesOf(std::wstring const& family, UINT32 filter) {
    std::vector<std::wstring> names;
    if (!A().findPackages) return names;
    UINT32 count = 0, length = 0;
    LONG result = A().findPackages(family.c_str(), filter, &count, nullptr, &length, nullptr, nullptr);
    if (result != ERROR_INSUFFICIENT_BUFFER || !count || !length) return names;
    std::vector<PWSTR> pointers(count);
    std::vector<WCHAR> buffer(length);
    if (A().findPackages(family.c_str(), filter, &count, pointers.data(), &length, buffer.data(), nullptr) !=
        ERROR_SUCCESS) {
        return names;
    }
    for (UINT32 i = 0; i < count && i < pointers.size(); i++) {
        if (pointers[i]) names.push_back(pointers[i]);
    }
    return names;
}

std::wstring MainFolderOf(std::wstring const& family) {
    for (auto const& name : PackagesOf(family, kFilterHead | kFilterDirect)) {
        std::wstring path = PathOf(name);
        if (!path.empty() && FileExists(path + L"\\AppxManifest.xml")) return path;
    }
    return {};
}

// "microsoft.windowscalculator_11.2405.2.0_x64__8wekyb3d8bbwe" as
// "microsoft.windowscalculator_*_8wekyb3d8bbwe", so it still matches once an
// update installs a new version (and matches the app's resource packages).
// A system app's folder (c:\windows\immersivecontrolpanel) stays as it is.
std::wstring Pattern(std::wstring const& folder) {
    std::wstring lower = Lower(CleanPath(folder));
    while (!lower.empty() && lower.back() == L'\\') lower.pop_back();
    size_t slash = lower.find_last_of(L'\\');
    if (slash == std::wstring::npos) return lower;
    std::wstring name = lower.substr(slash + 1);
    size_t first = name.find(L'_'), last = name.rfind(L'_');
    if (first == std::wstring::npos || first == 0 || last == first || last + 1 >= name.size()) return lower;
    return lower.substr(0, slash + 1) + name.substr(0, first) + L"_*_" + name.substr(last + 1);
}

// The family from a pattern made above, empty for a system app's folder.
std::wstring FamilyOf(std::wstring const& pattern) {
    size_t slash = pattern.find_last_of(L'\\');
    std::wstring name = slash == std::wstring::npos ? pattern : pattern.substr(slash + 1);
    size_t star = name.find(L"_*_");
    return star == std::wstring::npos ? std::wstring() : name.substr(0, star) + name.substr(star + 2);
}

// Every folder a row's pictures come from.
std::vector<std::wstring> FoldersOf(std::wstring const& pattern) {
    std::vector<std::wstring> folders;
    std::wstring family = FamilyOf(pattern);
    if (family.empty()) {
        if (FolderExists(pattern)) folders.push_back(pattern);
        return folders;
    }
    for (auto const& name : PackagesOf(family, kFilterHead | kFilterDirect | kFilterResource)) {
        std::wstring path = PathOf(name);
        if (path.empty() || !WildcardMatch(pattern, Lower(path))) continue;
        folders.push_back(path);
        std::wstring outside = OutsidePathOf(name);
        if (!outside.empty()) folders.push_back(outside);
    }
    return folders;
}

// The outside folders of a pattern's packages (see OutsidePathOf).
std::vector<std::wstring> OutsideFoldersOf(std::wstring const& pattern) {
    std::vector<std::wstring> folders;
    std::wstring family = FamilyOf(pattern);
    if (family.empty()) return folders;
    for (auto const& name : PackagesOf(family, kFilterHead | kFilterDirect)) {
        std::wstring outside = OutsidePathOf(name);
        if (!outside.empty()) folders.push_back(outside);
    }
    return folders;
}

std::wstring Utf8ToWide(std::string_view text) {
    int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0);
    std::wstring wide(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), wide.data(), length);
    return wide;
}

// The value of name="..." in a piece of the manifest, or empty.
std::wstring Attribute(std::wstring_view xml, std::wstring_view name) {
    size_t at = 0;
    while ((at = xml.find(name, at)) != std::wstring_view::npos) {
        size_t after = at + name.size();
        bool starts = at == 0 || iswspace(xml[at - 1]);
        if (starts && after + 1 < xml.size() && xml[after] == L'=' && (xml[after + 1] == L'"' || xml[after + 1] == L'\'')) {
            WCHAR quote = xml[after + 1];
            size_t end = xml.find(quote, after + 2);
            if (end != std::wstring_view::npos) return std::wstring(xml.substr(after + 2, end - after - 2));
        }
        at = after;
    }
    return {};
}

// "Assets\AppList.png" as the stem "assets\applist"; empty for anything else.
std::wstring Stem(std::wstring value) {
    for (auto& c : value) {
        if (c == L'/') c = L'\\';
    }
    while (!value.empty() && value.front() == L'\\') value.erase(0, 1);
    if (value.empty() || value.find(L':') != std::wstring::npos || value.find(L"..") != std::wstring::npos ||
        !EndsWithI(value, L".png")) {
        return {};
    }
    return Lower(value.substr(0, value.size() - 4));
}

// The logos an app's manifest names, as stems; a tile's (drawn with room
// around it) starts with +. Its own app's, when the package holds several.
std::wstring Logos(std::wstring const& folder, std::wstring const& appId) {
    std::vector<BYTE> bytes = ReadWholeFile(folder + L"\\AppxManifest.xml");
    if (bytes.empty()) return {};
    std::string_view text((const char*)bytes.data(), bytes.size());
    if (text.size() >= 3 && (BYTE)text[0] == 0xEF) text.remove_prefix(3);
    std::wstring xml = Utf8ToWide(text);
    std::wstring_view app = xml;
    // The <Application Id="..."> element, up to its end.
    for (size_t at = 0; !appId.empty() && (at = xml.find(L"<Application ", at)) != std::wstring::npos; at++) {
        size_t end = xml.find(L"</Application>", at);
        size_t close = xml.find(L'>', at);
        if (close == std::wstring::npos) break;
        if (_wcsicmp(Attribute(std::wstring_view(xml).substr(at, close - at), L"Id").c_str(), appId.c_str()) == 0) {
            app = std::wstring_view(xml).substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
            break;
        }
    }
    std::vector<std::wstring> stems;
    auto add = [&](std::wstring stem, bool tile) {
        if (stem.empty()) return;
        for (auto const& s : stems) {
            if (s == stem || s == L"+" + stem) return;
        }
        stems.push_back(tile ? L"+" + stem : stem);
    };
    add(Stem(Attribute(app, L"Square44x44Logo")), false);
    add(Stem(Attribute(app, L"Square30x30Logo")), false);  // Windows 8.1's.
    add(Stem(Attribute(app, L"SmallLogo")), false);
    add(Stem(Attribute(app, L"Square150x150Logo")), true);
    add(Stem(Attribute(app, L"Logo")), true);
    add(Stem(Attribute(app, L"Wide310x150Logo")), true);
    add(Stem(Attribute(app, L"Square71x71Logo")), true);
    add(Stem(Attribute(app, L"Square310x310Logo")), true);
    add(Stem(Attribute(app, L"Square70x70Logo")), true);
    // The package's own logo (Settings' app list, the Store).
    size_t logo = xml.find(L"<Logo>");
    size_t logoEnd = logo == std::wstring::npos ? logo : xml.find(L"</Logo>", logo);
    if (logoEnd != std::wstring::npos) add(Stem(xml.substr(logo + 6, logoEnd - logo - 6)), false);
    std::wstring joined;
    for (auto const& s : stems) joined += (joined.empty() ? L"" : L";") + s;
    return joined;
}

bool PngSize(std::wstring const& path, UINT* width, UINT* height) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    BYTE head[24];
    DWORD read = 0;
    bool ok = ReadFile(file, head, sizeof(head), &read, nullptr) && read == sizeof(head) &&
              memcmp(head, "\x89PNG", 4) == 0 && memcmp(head + 12, "IHDR", 4) == 0;
    CloseHandle(file);
    if (!ok) return false;
    auto big = [](const BYTE* b) { return (UINT)b[0] << 24 | (UINT)b[1] << 16 | (UINT)b[2] << 8 | b[3]; };
    *width = big(head + 16);
    *height = big(head + 20);
    return *width && *height && *width <= 4096 && *height <= 4096;
}

struct LogoFile {
    std::wstring relative;  // Lower case, from the package folder.
    std::wstring path;
    bool tile = false;
};

// AppList.png, AppList.<qualifiers>.png, and the same in qualifier folders
// (scale-200\AppList.png, contrast-high\...).
void FindLogoFiles(std::wstring const& folder, std::wstring const& stemWithMark, std::vector<LogoFile>& out) {
    bool tile = !stemWithMark.empty() && stemWithMark[0] == L'+';
    std::wstring stem = tile ? stemWithMark.substr(1) : stemWithMark;
    size_t slash = stem.find_last_of(L'\\');
    std::wstring dir = slash == std::wstring::npos ? L"" : stem.substr(0, slash + 1);
    std::wstring name = slash == std::wstring::npos ? stem : stem.substr(slash + 1);
    auto matches = [&](std::wstring const& file) {
        std::wstring lower = Lower(file);
        return lower == name + L".png" ||
               (lower.size() > name.size() + 5 && lower.compare(0, name.size() + 1, name + L".") == 0 &&
                EndsWithI(lower, L".png"));
    };
    auto scan = [&](std::wstring const& relative, bool descend, auto& self) -> void {
        WIN32_FIND_DATAW find;
        HANDLE search = FindFirstFileW((folder + L"\\" + relative + L"*").c_str(), &find);
        if (search == INVALID_HANDLE_VALUE) return;
        do {
            std::wstring file = find.cFileName;
            if (file == L"." || file == L"..") continue;
            if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (descend && file.find(L'-') != std::wstring::npos) self(relative + file + L"\\", false, self);
            } else if (matches(file)) {
                out.push_back({Lower(relative + file), folder + L"\\" + relative + file, tile});
            }
        } while (FindNextFileW(search, &find));
        FindClose(search);
    };
    scan(dir, true, scan);
}

bool WriteWholeFile(std::wstring const& path, std::vector<BYTE> const& data) {
    for (size_t at = path.find(L'\\', 3); at != std::wstring::npos; at = path.find(L'\\', at + 1)) {
        CreateDirectoryW(path.substr(0, at).c_str(), nullptr);
    }
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    bool ok = WriteFile(file, data.data(), (DWORD)data.size(), &written, nullptr) && written == data.size();
    CloseHandle(file);
    return ok;
}

void DeleteTree(std::wstring const& path) {
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((path + L"\\*").c_str(), &find);
    if (search != INVALID_HANDLE_VALUE) {
        do {
            std::wstring name = find.cFileName;
            if (name == L"." || name == L"..") continue;
            std::wstring child = path + L"\\" + name;
            if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                DeleteTree(child);
            } else {
                DeleteFileW(child.c_str());
            }
        } while (FindNextFileW(search, &find));
        FindClose(search);
    }
    RemoveDirectoryW(path.c_str());
}

// Start and other Store apps run walled off: they may read the pictures (and
// the list) only once the storage folder lets every app package read it.
void LetAppsRead(std::wstring const& folder) {
    PACL old = nullptr;
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if (GetNamedSecurityInfoW(folder.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &old,
                              nullptr, &descriptor) != ERROR_SUCCESS) {
        return;
    }
    PSID sids[2] = {};
    EXPLICIT_ACCESSW access[2] = {};
    int count = 0;
    for (PCWSTR text : {L"S-1-15-2-1", L"S-1-15-2-2"}) {  // All (restricted) app packages.
        if (!ConvertStringSidToSidW(text, &sids[count])) continue;
        access[count].grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
        access[count].grfAccessMode = GRANT_ACCESS;
        access[count].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        access[count].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        access[count].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        access[count].Trustee.ptstrName = (LPWSTR)sids[count];
        count++;
    }
    PACL updated = nullptr;
    if (count && SetEntriesInAclW(count, access, old, &updated) == ERROR_SUCCESS) {
        DWORD result = SetNamedSecurityInfoW((LPWSTR)folder.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr,
                                             nullptr, updated, nullptr);
        if (result != ERROR_SUCCESS) Wh_Log(L"Couldn't let apps read the storage folder: %u", result);
        LocalFree(updated);
    }
    for (int i = 0; i < count; i++) LocalFree(sids[i]);
    LocalFree(descriptor);
}

// Pictures for every logo file of the app, from an .ico, in the storage
// folder's packages\<name>. Returns how many were made.
int MakePictures(std::vector<std::wstring> const& folders, std::wstring const& stems, std::wstring const& ico,
                 std::wstring const& name) {
    std::wstring root = StorageRoot();
    if (root.empty() || name.empty()) return 0;
    std::vector<LogoFile> files;
    for (auto const& folder : folders) {
        for (size_t at = 0; at <= stems.size();) {
            size_t semicolon = stems.find(L';', at);
            std::wstring stem = stems.substr(at, semicolon == std::wstring::npos ? std::wstring::npos : semicolon - at);
            if (!stem.empty()) FindLogoFiles(folder, stem, files);
            if (semicolon == std::wstring::npos) break;
            at = semicolon + 1;
        }
    }
    if (files.empty()) return 0;
    std::wstring packages = root + L"\\packages";
    CreateDirectoryW(packages.c_str(), nullptr);
    LetAppsRead(root);
    std::wstring target = packages + L"\\" + name;
    DeleteTree(target);
    int made = 0;
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    {
        Com<IWICImagingFactory> wic;
        std::vector<Pixels> frames;
        if (SUCCEEDED(CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                       __uuidof(IWICImagingFactory), (void**)&wic)) &&
            DecodeFrames(wic.p, ReadWholeFile(ico), &frames)) {
            std::set<std::wstring> done;
            for (auto const& file : files) {
                UINT width = 0, height = 0;
                if (!done.insert(file.relative).second || !PngSize(file.path, &width, &height)) continue;
                // A tile has room around its logo, as Windows' own do.
                UINT side = std::min(width, height);
                if (file.tile) side = std::max(1u, side * 2 / 3);
                Pixels square, picture;
                if (!FrameAt(wic.p, frames, side, &square)) continue;
                picture.width = width;
                picture.height = height;
                picture.bgra.assign((size_t)width * height * 4, 0);
                UINT left = (width - side) / 2, top = (height - side) / 2;
                for (UINT y = 0; y < side; y++) {
                    memcpy(&picture.bgra[((size_t)(top + y) * width + left) * 4], &square.bgra[(size_t)y * side * 4],
                           (size_t)side * 4);
                }
                std::vector<BYTE> png;
                if (EncodePng(wic.p, picture, &png) && WriteWholeFile(target + L"\\" + file.relative, png)) made++;
            }
        }
    }
    if (SUCCEEDED(com)) CoUninitialize();
    return made;
}

// The folder name for a row's pictures: the package's, with the icon's mark,
// so a new icon goes in a new folder while programs may still hold the old.
std::wstring PicturesName(std::wstring const& pattern, std::wstring const& ico) {
    size_t slash = pattern.find_last_of(L'\\');
    std::wstring name = slash == std::wstring::npos ? pattern : pattern.substr(slash + 1);
    for (auto& c : name) {
        if (c == L'*') c = L'#';
    }
    DWORD hash = 2166136261u;
    for (WCHAR c : Lower(ico)) hash = (hash ^ c) * 16777619u;
    WCHAR suffix[16];
    swprintf_s(suffix, L"-%08x", hash);
    return name + suffix;
}

// A Store app's row, its pictures made: the app by its package folder (or a
// pattern of it) and the logos its manifest names.
bool MakeRow(ListRow* row) {
    if (row->progId.empty() || row->icon.empty()) return false;
    row->previous = PicturesName(row->original, row->icon);
    if (MakePictures(FoldersOf(row->original), row->progId, row->icon, row->previous) <= 0) return false;
    // The app's outside folders, for the rules (see AddPackageRule).
    std::string note;
    for (auto const& outside : OutsideFoldersOf(row->original)) {
        int length = WideCharToMultiByte(CP_UTF8, 0, outside.c_str(), (int)outside.size(), nullptr, 0, nullptr, nullptr);
        std::string line(length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, outside.c_str(), (int)outside.size(), line.data(), length, nullptr, nullptr);
        note += line + "\n";
    }
    if (!note.empty()) {
        WriteWholeFile(StorageRoot() + L"\\packages\\" + row->previous + L"\\" + kOutsideNote,
                       std::vector<BYTE>(note.begin(), note.end()));
    }
    return true;
}

// The process's app ("Microsoft.WindowsCalculator_8wekyb3d8bbwe!App"), if it
// is one.
std::wstring AppIdOf(HANDLE process) {
    if (!A().appId) return {};
    UINT32 length = 256;
    WCHAR id[256];
    return A().appId(process, &length, id) == ERROR_SUCCESS ? std::wstring(id) : std::wstring();
}

// Start's own host keeps the logos it has drawn; it starts again by itself.
void RestartStart() {
    DWORD session = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &session);
    DWORD ids[2048], bytes = 0;
    if (!EnumProcesses(ids, sizeof(ids), &bytes)) return;
    for (DWORD i = 0; i < bytes / sizeof(DWORD); i++) {
        DWORD theirs = 0;
        if (!ProcessIdToSessionId(ids[i], &theirs) || theirs != session) continue;
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE, FALSE, ids[i]);
        if (!process) continue;
        WCHAR path[MAX_PATH * 2];
        DWORD length = ARRAYSIZE(path);
        if (QueryFullProcessImageNameW(process, 0, path, &length) &&
            EndsWithI(path, L"\\StartMenuExperienceHost.exe")) {
            TerminateProcess(process, 1);
        }
        CloseHandle(process);
    }
}

}  // namespace store

////////////////////////////////////////////////////////////////////////////////
// Lasting changes (Tourne'Icons Lasting)
//
// What Windows keeps by itself, so it stays with the mod off: a program's
// shortcuts (Start, the desktop, pinned taskbar buttons) get the new icon,
// Windows' own icons (This PC, the Recycle Bin, user folders, folders) get
// it through Windows' icon settings, and file types as in the live version.
// What each had before is kept in lasting.txt, to put back.

std::wstring FileTitle(std::wstring const& path);

namespace lasting {

// lasting.txt rows: kind and original of the change, then what was changed
// (lnk or reg), where, and what it was ("-" for nothing).
std::wstring SavedPath() {
    std::wstring root = StorageRoot();
    return root.empty() ? root : root + L"\\lasting.txt";
}

struct Saved {
    std::wstring kind, original, type, where, before;
};

std::vector<Saved> ReadSaved() {
    std::vector<Saved> saved;
    for (auto const& r : ReadList(SavedPath())) saved.push_back({r.kind, r.original, r.icon, r.label, r.progId});
    return saved;
}

bool WriteSaved(std::vector<Saved> const& saved) {
    std::vector<ListRow> rows;
    for (auto const& s : saved) rows.push_back({s.kind, s.original, s.type, s.where, s.before, L""});
    return WriteList(rows, SavedPath());
}

// Rows the settings made have the kind "setting" and, as their original,
// the icon that was showing there before (what the settings work from).
constexpr WCHAR kSettingKind[] = L"setting";

Saved* FindSaved(std::vector<Saved>& saved, std::wstring const& type, std::wstring const& where) {
    for (auto& s : saved) {
        if (s.type == type && _wcsicmp(s.where.c_str(), where.c_str()) == 0) return &s;
    }
    return nullptr;
}

// Whether a place is saved already. A change made in the picker takes over
// a place the settings changed, keeping what it was before them all.
bool IsSaved(std::vector<Saved>& saved, ListRow const& row, std::wstring const& type, std::wstring const& where) {
    Saved* s = FindSaved(saved, type, where);
    if (s && s->kind == kSettingKind) {
        s->kind = row.kind;
        s->original = row.original;
    }
    return s != nullptr;
}

// One lock for lasting.txt and the places it lists: the picker and the
// settings run on different threads.
SRWLOCK g_lock = SRWLOCK_INIT;

struct Locked {
    Locked() { AcquireSRWLockExclusive(&g_lock); }
    ~Locked() { ReleaseSRWLockExclusive(&g_lock); }
};

std::wstring Folder(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring result;
    if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_DONT_VERIFY, nullptr, &path)) && path) result = path;
    CoTaskMemFree(path);
    return result;
}

// Start's folders, the desktops, and the taskbar's pinned buttons (with the
// rest of Quick Launch).
std::vector<std::wstring> ShortcutFolders() {
    std::vector<std::wstring> folders = {Folder(FOLDERID_Programs), Folder(FOLDERID_CommonPrograms),
                                         Folder(FOLDERID_Desktop), Folder(FOLDERID_PublicDesktop)};
    std::wstring roaming = Folder(FOLDERID_RoamingAppData);
    if (!roaming.empty()) folders.push_back(roaming + L"\\Microsoft\\Internet Explorer\\Quick Launch");
    return folders;
}

void FindShortcuts(std::wstring const& folder, int depth, std::vector<std::wstring>& out) {
    if (folder.empty() || depth > 5) return;
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((folder + L"\\*").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) return;
    do {
        std::wstring name = find.cFileName;
        if (name == L"." || name == L"..") continue;
        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!(find.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) FindShortcuts(folder + L"\\" + name, depth + 1, out);
        } else if (EndsWithI(name, L".lnk")) {
            out.push_back(folder + L"\\" + name);
        }
    } while (FindNextFileW(search, &find));
    FindClose(search);
}

struct Link {
    Com<IShellLinkW> link;
    Com<IPersistFile> file;
    std::wstring target;
};

bool OpenLink(std::wstring const& path, DWORD mode, Link* out) {
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&out->link)) ||
        FAILED(out->link->QueryInterface(IID_IPersistFile, (void**)&out->file)) ||
        FAILED(out->file->Load(path.c_str(), mode))) {
        return false;
    }
    WCHAR target[MAX_PATH * 2] = L"";
    if (SUCCEEDED(out->link->GetPath(target, ARRAYSIZE(target), nullptr, 0)) && target[0]) out->target = target;
    return true;
}

// "file,index", or "-" when the shortcut shows its target's own icon.
std::wstring IconOf(IShellLinkW* link) {
    WCHAR file[MAX_PATH * 2] = L"";
    int index = 0;
    if (FAILED(link->GetIconLocation(file, ARRAYSIZE(file), &index)) || !file[0]) return L"-";
    return std::wstring(file) + L"," + std::to_wstring(index);
}

bool SetIcon(Link& link, std::wstring const& path, std::wstring const& location) {
    std::wstring file;
    int index = 0;
    if (location != L"-" && !SplitIconLocation(location, &file, &index)) file = location;
    return SUCCEEDED(link.link->SetIconLocation(location == L"-" ? L"" : file.c_str(), index)) &&
           SUCCEEDED(link.file->Save(path.c_str(), TRUE));
}

// What a shortcut stands for, as a program row can name it (lower case): its
// target; the program an app's updater starts through it (Squirrel's
// "Update.exe --processStart Discord.exe" starts Discord.exe beside it); and
// the file its icon comes from (Discord's app.ico), as it was before any
// change here; and the shortcut itself.
std::vector<std::wstring> LinkNames(std::wstring const& path, Link const& link, Saved const* known) {
    std::vector<std::wstring> names;
    if (!link.target.empty()) names.push_back(Lower(CleanPath(link.target)));
    WCHAR arguments[1024] = L"";
    if (!link.target.empty() && SUCCEEDED(link.link->GetArguments(arguments, ARRAYSIZE(arguments)))) {
        std::wstring args = Lower(arguments);
        size_t at = args.find(L"--processstart");
        if (at != std::wstring::npos) {
            std::wstring exe = args.substr(at + 14);
            exe.erase(0, exe.find_first_not_of(L" =\t\""));
            exe = exe.substr(0, exe.find_first_of(L" \t\""));
            std::wstring target = names[0];
            if (!exe.empty() && exe.find(L'\\') == std::wstring::npos) {
                names.push_back(target.substr(0, target.find_last_of(L'\\') + 1) + exe);
            }
        }
    }
    std::wstring icon = known && known->before != L"-" ? known->before : IconOf(link.link.p), file;
    int number = 0;
    if (icon != L"-" && SplitIconLocation(icon, &file, &number)) {
        file = Lower(CleanPath(file));
        if (!file.empty()) names.push_back(file);
    }
    names.push_back(Lower(path));  // The shortcut itself, for stubborn redirects.
    return names;
}

// Every shortcut to a program one of the rows matches gets that row's icon
// (the newest row's, when several match). Returns how many show a row's icon
// now; *written counts those changed just now.
int ApplyPrograms(std::vector<ListRow> const& rows, std::vector<Saved>& saved, int* written) {
    int count = 0;
    if (rows.empty()) return 0;
    std::vector<std::wstring> links;
    for (auto const& folder : ShortcutFolders()) FindShortcuts(folder, 0, links);
    for (auto const& path : links) {
        Link link;
        if (!OpenLink(path, STGM_READWRITE, &link)) continue;
        Saved const* known = FindSaved(saved, L"lnk", path);
        std::vector<std::wstring> names = LinkNames(path, link, known);
        ListRow const* row = nullptr;
        for (auto const& candidate : rows) {
            bool matches = known && known->kind == candidate.kind &&
                           _wcsicmp(known->original.c_str(), candidate.original.c_str()) == 0;
            for (auto const& name : names) matches = matches || WildcardMatch(candidate.original, name);
            if (matches) row = &candidate;
        }
        if (!row) continue;
        std::wstring before = IconOf(link.link.p), icon = row->icon + L",0";
        bool isNew = !IsSaved(saved, *row, L"lnk", path);
        if (isNew) saved.push_back({row->kind, row->original, L"lnk", path, before});
        if (_wcsicmp(before.c_str(), icon.c_str()) == 0) {
            count++;
        } else if (SetIcon(link, path, icon)) {
            count++;
            (*written)++;
        } else if (isNew) {
            saved.pop_back();  // (Couldn't be changed: Start's shared folder needs admin.)
        }
    }
    return count;
}

// Windows' own icons that have a setting of their own: imageres.dll's id to
// the shell's items (by class id) that show it, or the folder icon.
struct WindowsIcon {
    int id;
    PCWSTR value;  // In the item's DefaultIcon: empty for its default.
    std::vector<PCWSTR> items;
};
const std::vector<WindowsIcon>& WindowsIcons() {
    static const std::vector<WindowsIcon> icons = {
        {109, L"", {L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}"}},  // This PC
        {55, L"empty", {L"{645FF040-5081-101B-9F08-00AA002F954E}"}},  // Recycle Bin
        {54, L"full", {L"{645FF040-5081-101B-9F08-00AA002F954E}"}},
        {25, L"", {L"{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}"}},  // Network
        {123, L"", {L"{59031a47-3f72-44a7-89c5-5595fe6b30ee}"}},  // Your user folder
        {112, L"", {L"{d3162b92-9365-467a-956b-92703aca08af}", L"{A8CDFF1C-4878-43be-B5FD-F8091C1C60D0}"}},  // Documents
        {113, L"", {L"{24ad3ad4-a569-4530-98e1-ab02f9417aa8}", L"{3ADD1653-EB32-4cb0-BBD7-DFA0ABB5ACCA}"}},  // Pictures
        {108, L"", {L"{3dfdf296-dbec-4fb4-81d1-6a3438bcf4de}", L"{1CF1260C-4DD0-4ebb-811F-33C572699FDE}"}},  // Music
        {189, L"", {L"{f86fa3ab-70d2-4fc7-9c99-fcbf05467f3a}", L"{A0953C92-50DC-43bf-BE83-3742FED03C9C}"}},  // Videos
        {184, L"", {L"{088e3905-0323-4b02-9826-5d99428e115f}", L"{374DE290-123F-4565-9164-39C4925E467B}"}},  // Downloads
        {183, L"", {L"{B4BFCC3A-DB2C-424C-B029-7FE99A87C641}"}},  // Desktop
        {27, L"", {L"{21EC2020-3AEA-1069-A2DD-08002B30309D}", L"{26EE0668-A00A-44D7-9371-BEB064C98683}"}},  // Control Panel
    };
    return icons;
}

constexpr WCHAR kShellIconsKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons";

// The imageres.dll id an icon row stands for, or 0.
int ImageresId(std::wstring const& original) {
    std::wstring file;
    int number = 0;
    if (!SplitIconLocation(original, &file, &number) || number >= 0 || !EndsWithI(file, L"\\imageres.dll")) return 0;
    return -number;
}

// Windows' stock icons that Shell Icons can set (for every account), by
// their number there, which is the stock icon's own: the imageres.dll id each
// shows, read from Windows when first asked (before any change here). The
// Recycle Bin goes by its own class instead.
std::map<int, std::vector<int>> const& StockNumbers() {
    static const std::map<int, std::vector<int>> numbers = [] {
        std::map<int, std::vector<int>> found;
        for (int n : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 22, 23, 28, 29}) {
            WCHAR data[2048];
            DWORD size = sizeof(data);
            if (RegGetValueW(HKEY_LOCAL_MACHINE, kShellIconsKey, std::to_wstring(n).c_str(),
                             RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, data, &size) == ERROR_SUCCESS) {
                continue;  // Set already (by you, or here before): not Windows' own any more.
            }
            SHSTOCKICONINFO info{sizeof(info)};
            if (SUCCEEDED(SHGetStockIconInfo((SHSTOCKICONID)n, SHGSI_ICONLOCATION, &info)) && info.iIcon < 0 &&
                EndsWithI(info.szPath, L"\\imageres.dll")) {
                found[-info.iIcon].push_back(n);
            }
        }
        // Folders, closed and open, go together.
        for (int folder : {3, 4}) {
            auto& list = found[3];
            if (std::find(list.begin(), list.end(), folder) == list.end()) list.push_back(folder);
        }
        found[4] = found[3];
        return found;
    }();
    return numbers;
}

constexpr WCHAR kDriveIconsKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DriveIcons";

// The fixed drives (C, D, ...) by the imageres.dll id each shows, read when
// first asked: the Windows drive has an icon of its own. Each drive is set by
// its letter in DriveIcons (for every account).
std::vector<WCHAR> DrivesShowing(int id) {
    static const std::map<int, std::vector<WCHAR>> drives = [] {
        std::map<int, std::vector<WCHAR>> found;
        DWORD present = GetLogicalDrives();
        for (int i = 2; i < 26; i++) {
            if (!(present & (1u << i))) continue;
            WCHAR root[] = {WCHAR(L'A' + i), L':', L'\\', 0};
            if (GetDriveTypeW(root) != DRIVE_FIXED) continue;
            PIDLIST_ABSOLUTE pidl = nullptr;
            if (FAILED(SHParseDisplayName(root, nullptr, &pidl, 0, nullptr)) || !pidl) continue;
            Com<IShellFolder> parent;
            PCUITEMID_CHILD child = nullptr;
            if (SUCCEEDED(SHBindToParent(pidl, IID_IShellFolder, (void**)&parent, &child))) {
                Com<IExtractIconW> extract;
                if (SUCCEEDED(parent->GetUIObjectOf(nullptr, 1, &child, IID_IExtractIconW, nullptr, (void**)&extract))) {
                    WCHAR file[MAX_PATH * 2] = L"";
                    int index = 0;
                    UINT flags = 0;
                    if (extract->GetIconLocation(GIL_FORSHELL, file, ARRAYSIZE(file), &index, &flags) == S_OK &&
                        !(flags & GIL_NOTFILENAME) && index < 0 && EndsWithI(CleanPath(file), L"\\imageres.dll")) {
                        found[-index].push_back(root[0]);
                    }
                }
            }
            CoTaskMemFree(pidl);
        }
        return found;
    }();
    auto found = drives.find(id);
    return found == drives.end() ? std::vector<WCHAR>{} : found->second;
}

// Whether a Windows icon can be changed lastingly. One changed here before
// can be again, though Windows now reports its new icon in those places.
bool CanLast(std::wstring const& original) {
    int id = ImageresId(original);
    if (!id) return false;
    if (StockNumbers().count(id) || !DrivesShowing(id).empty()) return true;
    for (auto const& icon : WindowsIcons()) {
        if (icon.id == id) return true;
    }
    static std::set<int> before = [] {
        std::set<int> ids;
        for (auto const& s : ReadSaved()) {
            if (s.kind == L"icon" && s.type == L"reg") ids.insert(ImageresId(s.original));
        }
        return ids;
    }();
    return before.count(id) != 0;
}

// "HKCU|key|value" or "HKLM|key|value".
std::wstring RegWhere(bool machine, std::wstring const& key, std::wstring const& value) {
    return (machine ? L"HKLM|" : L"HKCU|") + key + L"|" + value;
}

bool SetValue(std::vector<Saved>& saved, ListRow const& row, bool machine, std::wstring const& key,
              std::wstring const& value, std::wstring const& data) {
    HKEY root = machine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    std::wstring where = RegWhere(machine, key, value);
    if (!IsSaved(saved, row, L"reg", where)) {
        WCHAR buffer[2048];
        DWORD size = sizeof(buffer);
        bool exists = RegGetValueW(root, key.c_str(), value.empty() ? nullptr : value.c_str(),
                                   RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, buffer, &size) ==
                      ERROR_SUCCESS;
        saved.push_back({row.kind, row.original, L"reg", where, exists ? L"=" + std::wstring(buffer) : L"-"});
    }
    if (RegSetKeyValueW(root, key.c_str(), value.empty() ? nullptr : value.c_str(), REG_EXPAND_SZ, data.c_str(),
                        (DWORD)((data.size() + 1) * sizeof(WCHAR))) == ERROR_SUCCESS) {
        return true;
    }
    saved.pop_back();
    return false;
}

int ApplyWindowsIcon(ListRow const& row, std::vector<Saved>& saved) {
    int id = ImageresId(row.original), count = 0;
    std::wstring data = row.icon + L",0";
    // Places this row set before stay its own, though Windows now reports
    // them as showing the new icon.
    std::set<std::wstring> places;
    for (auto const& s : saved) {
        if (s.type == L"reg" && s.kind == row.kind && _wcsicmp(s.original.c_str(), row.original.c_str()) == 0) {
            places.insert(s.where);
        }
    }
    // Stock icons (folders, drives, discs and the like) through Shell Icons,
    // and fixed drives by their letter; for every account, so they need admin.
    auto stock = StockNumbers().find(id);
    if (stock != StockNumbers().end()) {
        for (int n : stock->second) places.insert(RegWhere(true, kShellIconsKey, std::to_wstring(n)));
    }
    for (WCHAR letter : DrivesShowing(id)) {
        places.insert(RegWhere(true, std::wstring(kDriveIconsKey) + L"\\" + letter + L"\\DefaultIcon", L""));
    }
    for (auto const& where : places) {
        size_t bar1 = where.find(L'|'), bar2 = where.rfind(L'|');
        if (where.compare(0, 4, L"HKLM") != 0 || bar1 == std::wstring::npos || bar2 == bar1) continue;
        count += SetValue(saved, row, true, where.substr(bar1 + 1, bar2 - bar1 - 1), where.substr(bar2 + 1), data);
    }
    for (auto const& icon : WindowsIcons()) {
        if (icon.id != id) continue;
        for (PCWSTR item : icon.items) {
            std::wstring key = std::wstring(L"Software\\Classes\\CLSID\\") + item + L"\\DefaultIcon";
            count += SetValue(saved, row, false, key, icon.value, data);
            // The Recycle Bin's own value follows whether it's empty; start it as it is now.
            if (*icon.value) {
                SHQUERYRBINFO bin{sizeof(bin)};
                bool full = SUCCEEDED(SHQueryRecycleBinW(nullptr, &bin)) && bin.i64NumItems > 0;
                if (full == (wcscmp(icon.value, L"full") == 0)) SetValue(saved, row, false, key, L"", data);
            }
        }
    }
    return count;
}

// No values and no keys under it.
bool KeyIsEmpty(HKEY root, std::wstring const& key) {
    HKEY handle;
    if (RegOpenKeyExW(root, key.c_str(), 0, KEY_READ, &handle) != ERROR_SUCCESS) return false;
    DWORD keys = 1, values = 1;
    bool empty = RegQueryInfoKeyW(handle, nullptr, nullptr, nullptr, &keys, nullptr, nullptr, &values, nullptr, nullptr,
                                  nullptr, nullptr) == ERROR_SUCCESS &&
                 !keys && !values;
    RegCloseKey(handle);
    return empty;
}

void PutBackOne(Saved const& s) {
    if (s.type == L"lnk") {
        Link link;
        if (OpenLink(s.where, STGM_READWRITE, &link)) SetIcon(link, s.where, s.before);
        return;
    }
    size_t bar1 = s.where.find(L'|'), bar2 = s.where.rfind(L'|');
    if (bar1 == std::wstring::npos || bar2 == bar1) return;
    HKEY root = s.where.compare(0, 4, L"HKLM") == 0 ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    std::wstring key = s.where.substr(bar1 + 1, bar2 - bar1 - 1), value = s.where.substr(bar2 + 1);
    PCWSTR name = value.empty() ? nullptr : value.c_str();
    if (s.before.size() > 1 && s.before[0] == L'=') {
        std::wstring data = s.before.substr(1);
        RegSetKeyValueW(root, key.c_str(), name, REG_EXPAND_SZ, data.c_str(), (DWORD)((data.size() + 1) * sizeof(WCHAR)));
    } else {
        RegDeleteKeyValueW(root, key.c_str(), name);
        // Our own keys go when they're empty again (Windows' own keep): a
        // class's DefaultIcon under your account, a drive's under DriveIcons.
        if (root == HKEY_CURRENT_USER || _wcsnicmp(key.c_str(), kDriveIconsKey, wcslen(kDriveIconsKey)) == 0) {
            std::wstring parent = key.substr(0, key.find_last_of(L'\\'));
            if (KeyIsEmpty(root, key)) RegDeleteKeyW(root, key.c_str());
            if (KeyIsEmpty(root, parent) && _wcsicmp(parent.c_str(), kDriveIconsKey) != 0) {
                RegDeleteKeyW(root, parent.c_str());
            }
        }
    }
}

// A change made lasting. Returns how many places took it.
int Apply(ListRow const& row) {
    Locked locked;
    std::vector<Saved> saved = ReadSaved();
    int written = 0;
    int count = row.kind == L"program" ? ApplyPrograms({row}, saved, &written)
                : row.kind == L"icon"  ? ApplyWindowsIcon(row, saved)
                                       : 0;
    WriteSaved(saved);
    return count;
}

// Everything a change did, put back as it was.
void PutBack(ListRow const& row) {
    Locked locked;
    std::vector<Saved> saved = ReadSaved(), kept;
    for (auto const& s : saved) {
        if (s.kind == row.kind && _wcsicmp(s.original.c_str(), row.original.c_str()) == 0) {
            PutBackOne(s);
        } else {
            kept.push_back(s);
        }
    }
    WriteSaved(kept);
}

// Stubborn redirects (the settings): shortcuts whose program, own file or
// icon file a row names get its icon, after the picker's changes (so they
// win), and are watched while the mod runs when that's on.
constexpr WCHAR kStubbornKind[] = L"stubborn";

std::vector<ListRow> StubbornRows() {
    std::vector<ListRow> rows;
    for (int i = 0; i < 256; i++) {
        std::wstring target = RedirectSetting(L"stubbornRedirects[%d].target", i);
        std::wstring icon = RedirectSetting(L"stubbornRedirects[%d].icon", i);
        if (target.empty() && icon.empty()) break;
        target = Lower(CleanPath(target));
        icon = CleanPath(icon);
        if (target.empty() || icon.empty()) continue;
        std::wstring file = icon;
        int number = 0;
        if (!SplitIconLocation(icon, &file, &number)) file = icon;
        file = FullPath(CleanPath(file));
        // Kept in the storage folder, so it stays when the picture moves.
        std::wstring made = SaveIco(MakeIco(file, number), L"stubborn " + FileTitle(file));
        if (!made.empty()) rows.push_back({kStubbornKind, target, made, FileTitle(target)});
    }
    return rows;
}

// Shortcuts made since (a program installed or pinned later, or one an
// update remade) get their icons too, each time the tool starts. Returns how
// many shortcuts were changed.
int ApplyAll() {
    std::vector<ListRow> programs, icons, stubborn = StubbornRows();
    for (auto const& row : ReadList()) {
        if (row.kind == L"program") programs.push_back(row);
        if (row.kind == L"icon") icons.push_back(row);
    }
    programs.insert(programs.end(), stubborn.begin(), stubborn.end());
    Locked locked;
    std::vector<Saved> saved = ReadSaved();
    // Stubborn rows taken out of the settings: their shortcuts go back.
    std::erase_if(saved, [&](Saved const& s) {
        if (s.kind != kStubbornKind) return false;
        for (auto const& row : stubborn) {
            if (_wcsicmp(row.original.c_str(), s.original.c_str()) == 0) return false;
        }
        PutBackOne(s);
        return true;
    });
    int written = 0;
    ApplyPrograms(programs, saved, &written);
    for (auto const& row : icons) ApplyWindowsIcon(row, saved);
    WriteSaved(saved);
    return written;
}

// Watching Start's folders, the desktops and the pinned buttons: when a
// shortcut is made or rewritten, once things settle (3 s with no more
// changes), what was changed is put back. Nothing runs between changes.
HANDLE g_watchThread, g_watchStop;

DWORD WINAPI WatchThread(LPVOID) {
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::vector<HANDLE> handles{g_watchStop};
    for (auto const& folder : ShortcutFolders()) {
        if (folder.empty() || !FolderExists(folder)) continue;
        HANDLE change = FindFirstChangeNotificationW(folder.c_str(), TRUE,
                                                     FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);
        if (change != INVALID_HANDLE_VALUE) handles.push_back(change);
    }
    bool pending = false;
    for (;;) {
        DWORD result = WaitForMultipleObjects((DWORD)handles.size(), handles.data(), FALSE, pending ? 3000 : INFINITE);
        if (result == WAIT_OBJECT_0 || result == WAIT_FAILED) break;
        if (result == WAIT_TIMEOUT) {
            pending = false;
            if (ApplyAll()) TellWindowsIconsChanged();
            continue;
        }
        FindNextChangeNotification(handles[result - WAIT_OBJECT_0]);
        pending = true;
    }
    for (size_t i = 1; i < handles.size(); i++) FindCloseChangeNotification(handles[i]);
    if (SUCCEEDED(com)) CoUninitialize();
    return 0;
}

void StopWatching() {
    if (!g_watchThread) return;
    SetEvent(g_watchStop);
    WaitForSingleObject(g_watchThread, 5000);
    CloseHandle(g_watchThread);
    CloseHandle(g_watchStop);
    g_watchThread = g_watchStop = nullptr;
}

// On when the setting is, or when there are stubborn redirects.
void Watch() {
    bool on = Wh_GetIntSetting(L"watchShortcuts") != 0 ||
              !RedirectSetting(L"stubbornRedirects[%d].target", 0).empty();
    if (on == (g_watchThread != nullptr)) return;
    if (!on) {
        StopWatching();
        return;
    }
    g_watchStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_watchThread = g_watchStop ? CreateThread(nullptr, 0, WatchThread, nullptr, 0, nullptr) : nullptr;
    if (!g_watchThread && g_watchStop) {
        CloseHandle(g_watchStop);
        g_watchStop = nullptr;
    }
}


// The settings (icon theme, theme paths, Redirects), written into the same
// places: every shortcut, file type and Windows icon whose icon comes from a
// file a row names gets the row's version of that icon. Each run works out
// what every place should show and puts back the places no row wants any
// more, so changing or clearing a setting undoes what it did. Places changed
// in the picker are left to the picker.

enum class Stand { None, Ico, Picture, Module, Folder };

struct Replacement {
    std::wstring pattern;  // Lower case.
    std::wstring file;
    Stand stand = Stand::None;
};

Stand StandOf(std::wstring const& file) {
    if (FolderExists(file)) return Stand::Folder;
    if (!FileExists(file)) return Stand::None;
    if (EndsWithI(file, L".ico")) return Stand::Ico;
    if (IsModuleFile(file)) return Stand::Module;
    for (PCWSTR type : {L".png", L".svg", L".bmp", L".jpg", L".jpeg", L".gif"}) {
        if (EndsWithI(file, type)) return Stand::Picture;
    }
    return Stand::None;
}

// In priority order, lowest first, as the live version reads them.
std::vector<Replacement> SettingRows(RuleSettings const& settings) {
    std::vector<std::pair<std::wstring, std::wstring>> pairs;
    for (auto const& ini : ThemeInis(settings)) {
        for (auto const& row : ThemeIniRows(ini)) pairs.push_back(row);
    }
    for (auto const& row : settings.redirects) pairs.push_back(row);
    std::vector<Replacement> rows;
    for (auto const& [original, redirect] : pairs) {
        std::wstring pattern = Lower(CleanPath(original)), file = CleanPath(redirect);
        while (!file.empty() && file.front() == L'*') file.erase(0, 1);
        if (pattern.empty() || file.empty()) continue;
        file = FullPath(file);
        Stand stand = StandOf(file);
        if (stand != Stand::None) rows.push_back({pattern, file, stand});
    }
    return rows;
}

// One run's look-ups, kept for the run.
struct Run {
    std::vector<Replacement> rows;
    std::map<std::wstring, std::vector<std::wstring>> groups;  // A file's icon groups: "#123" or a name.
    std::map<std::wstring, std::wstring> made;                // A picture, made into an .ico.
};

BOOL CALLBACK CollectGroup(HMODULE, LPCWSTR, LPWSTR name, LONG_PTR param) {
    auto* names = (std::vector<std::wstring>*)param;
    names->push_back(IS_INTRESOURCE(name) ? L"#" + std::to_wstring(LOWORD(name)) : Lower(name));
    return TRUE;
}

std::vector<std::wstring> const& GroupsOf(Run& run, std::wstring const& file) {
    std::wstring key = Lower(file);
    auto found = run.groups.find(key);
    if (found != run.groups.end()) return found->second;
    std::vector<std::wstring>& names = run.groups[key];
    std::vector<std::wstring> tries{file};
    // Windows keeps its own files' icons in SystemResources\<name>.mun.
    std::wstring system = g_systemRoot + L"\\system32\\";
    if (!g_systemRoot.empty() && key.starts_with(system) && key.find(L'\\', system.size()) == std::wstring::npos) {
        tries.push_back(g_systemRoot + L"\\SystemResources\\" + file.substr(system.size()) + L".mun");
    }
    for (auto const& path : tries) {
        HMODULE module = LoadLibraryExW(path.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
        if (!module) continue;
        EnumResourceNamesW(module, RT_GROUP_ICON, CollectGroup, (LONG_PTR)&names);
        FreeLibrary(module);
        if (!names.empty()) break;
    }
    return names;
}

// "file,number" for an icon in a module: by id when it has one, else by place.
std::wstring InModule(Run& run, std::wstring const& file, std::wstring const& group) {
    std::vector<std::wstring> const& names = GroupsOf(run, file);
    for (size_t i = 0; i < names.size(); i++) {
        if (names[i] != group) continue;
        return group[0] == L'#' ? file + L",-" + group.substr(1) : file + L"," + std::to_wstring(i);
    }
    return {};
}

// A picture, as an .ico in the storage folder.
std::wstring MadeIco(Run& run, std::wstring const& picture) {
    if (EndsWithI(picture, L".ico")) return picture;
    std::wstring key = Lower(picture);
    auto found = run.made.find(key);
    if (found != run.made.end()) return found->second;
    std::wstring name = picture.substr(picture.find_last_of(L'\\') + 1);
    return run.made[key] = SaveIco(MakeIco(picture, 0), L"setting " + name);
}

// What one row makes of an icon (file and number, as shortcuts and the
// registry name it), or nothing when it doesn't match or has no such icon.
std::wstring Replace(Run& run, Replacement const& row, std::wstring const& file, int number) {
    bool matches = false;
    for (auto const& name : PathNames(Lower(file))) matches = matches || WildcardMatch(row.pattern, name);
    if (!matches) return {};
    if (!IsModuleFile(file)) {
        // A loose icon or picture: a whole-file stand-in replaces it.
        if (row.stand == Stand::Ico || row.stand == Stand::Picture) {
            std::wstring ico = MadeIco(run, row.file);
            return ico.empty() ? ico : ico + L",0";
        }
        return {};
    }
    std::vector<std::wstring> const& groups = GroupsOf(run, file);
    std::wstring group = number < 0 ? L"#" + std::to_wstring(-number)
                         : (size_t)number < groups.size() ? groups[number]
                                                          : L"";
    if (group.empty()) return {};
    switch (row.stand) {
        case Stand::Ico:
        case Stand::Picture: {
            // An .ico stands in for the program's main icon (its first).
            if (groups.empty() || groups[0] != group) return {};
            std::wstring ico = MadeIco(run, row.file);
            return ico.empty() ? ico : ico + L",0";
        }
        case Stand::Module:
            return InModule(run, row.file, group);
        case Stand::Folder: {
            std::wstring name = group[0] == L'#' ? group.substr(1) : group;
            for (PCWSTR type : {L".ico", L".png", L".svg", L".bmp", L".jpg"}) {
                std::wstring picture = row.file + L"\\" + name + type;
                if (FileExists(picture)) {
                    std::wstring ico = MadeIco(run, picture);
                    return ico.empty() ? ico : ico + L",0";
                }
            }
            return {};
        }
        default:
            return {};
    }
}

// The newest row that has something for this icon. A shortcut's program
// counts too, for rows that give a program a new icon.
std::wstring ReplaceAny(Run& run, std::wstring const& location, std::wstring const& program = L"") {
    std::wstring file;
    int number = 0;
    if (!SplitIconLocation(location, &file, &number)) file = location;
    file = CleanPath(file);
    if (file.empty() || file.find(L'%') != std::wstring::npos) return {};
    for (size_t i = run.rows.size(); i-- > 0;) {
        std::wstring result = Replace(run, run.rows[i], file, number);
        if (result.empty() && !program.empty() && _wcsicmp(program.c_str(), file.c_str()) != 0 &&
            (run.rows[i].stand == Stand::Ico || run.rows[i].stand == Stand::Picture)) {
            result = Replace(run, run.rows[i], program, 0);
        }
        if (!result.empty()) return result;
    }
    return {};
}

// What every place should show: type and where, to what it shows now
// without the settings, and what it should show.
struct Want {
    std::wstring type, where, source, value;
};

// Shortcuts: their own icon, or their program's first.
void WantShortcuts(Run& run, std::vector<Saved>& saved, std::vector<Want>& wants) {
    std::vector<std::wstring> links;
    for (auto const& folder : ShortcutFolders()) FindShortcuts(folder, 0, links);
    for (auto const& path : links) {
        Saved* s = FindSaved(saved, L"lnk", path);
        if (s && s->kind != kSettingKind) continue;
        Link link;
        if (!OpenLink(path, STGM_READ, &link)) continue;
        std::wstring source = s ? s->original : IconOf(link.link.p);
        std::wstring location = source == L"-" ? link.target + L",0" : source;
        if (location == L",0") continue;
        std::wstring value = ReplaceAny(run, location, link.target);
        if (!value.empty()) wants.push_back({L"lnk", path, source, value});
    }
}

// Windows' icon for a class (a file type's ProgID, a shell item's class id):
// its DefaultIcon, set again under your account. All of the key's values go
// along, since your account's key hides the whole of Windows' one.
void WantClass(Run& run, std::vector<Saved>& saved, std::vector<Want>& wants, std::wstring const& name) {
    std::wstring key = L"Software\\Classes\\" + name + L"\\DefaultIcon";
    HKEY handle;
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, (name + L"\\DefaultIcon").c_str(), 0, KEY_READ, &handle) != ERROR_SUCCESS) {
        return;
    }
    std::vector<Want> found;
    bool any = false;
    for (DWORD i = 0;; i++) {
        WCHAR value[256], data[2048];
        DWORD valueSize = ARRAYSIZE(value), dataSize = sizeof(data) - sizeof(WCHAR), type = 0;
        LSTATUS status = RegEnumValueW(handle, i, value, &valueSize, nullptr, &type, (BYTE*)data, &dataSize);
        if (status == ERROR_NO_MORE_ITEMS) break;
        if (status != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ)) continue;
        data[dataSize / sizeof(WCHAR)] = L'\0';
        std::wstring where = RegWhere(false, key, value);
        Saved* s = FindSaved(saved, L"reg", where);
        if (s && s->kind != kSettingKind) {
            found.clear();
            any = false;
            break;  // The picker's.
        }
        std::wstring source = s ? s->original : data;
        std::wstring replaced = source.find(L"%1") == std::wstring::npos ? ReplaceAny(run, source) : L"";
        any = any || !replaced.empty();
        found.push_back({L"reg", where, source, replaced.empty() ? source : replaced});
    }
    RegCloseKey(handle);
    if (any) wants.insert(wants.end(), found.begin(), found.end());
}

void WantClasses(Run& run, std::vector<Saved>& saved, std::vector<Want>& wants, std::wstring const& under) {
    HKEY handle;
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, under.empty() ? nullptr : under.c_str(), 0, KEY_READ, &handle) !=
        ERROR_SUCCESS) {
        return;
    }
    std::vector<std::wstring> names;
    for (DWORD i = 0;; i++) {
        WCHAR name[256];
        DWORD size = ARRAYSIZE(name);
        LSTATUS status = RegEnumKeyExW(handle, i, name, &size, nullptr, nullptr, nullptr, nullptr);
        if (status == ERROR_NO_MORE_ITEMS) break;
        if (status == ERROR_SUCCESS) names.push_back(under.empty() ? name : under + L"\\" + name);
    }
    RegCloseKey(handle);
    for (auto const& name : names) WantClass(run, saved, wants, name);
}

// Windows' stock icons (folders, drives, the shortcut arrow and the like)
// through Shell Icons, by the stock icon's number. For every account, so it
// needs admin; one you set yourself is left alone.
void WantStockIcons(Run& run, std::vector<Saved>& saved, std::vector<Want>& wants) {
    for (int id : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 22, 23, 28, 29, 31, 32}) {
        std::wstring where = RegWhere(true, kShellIconsKey, std::to_wstring(id));
        Saved* s = FindSaved(saved, L"reg", where);
        if (s && s->kind != kSettingKind) continue;
        std::wstring source;
        if (s) {
            source = s->original;
        } else {
            WCHAR data[2048];
            DWORD size = sizeof(data);
            if (RegGetValueW(HKEY_LOCAL_MACHINE, kShellIconsKey, std::to_wstring(id).c_str(),
                             RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, data, &size) == ERROR_SUCCESS) {
                continue;  // Yours.
            }
            SHSTOCKICONINFO info{sizeof(info)};
            if (FAILED(SHGetStockIconInfo((SHSTOCKICONID)id, SHGSI_ICONLOCATION, &info)) || !info.szPath[0]) continue;
            source = std::wstring(info.szPath) + L"," + std::to_wstring(info.iIcon);
        }
        std::wstring value = ReplaceAny(run, source);
        if (!value.empty()) wants.push_back({L"reg", where, source, value});
    }
}

std::wstring ReadNow(Saved const& s) {
    if (s.type == L"lnk") {
        Link link;
        return OpenLink(s.where, STGM_READ, &link) ? IconOf(link.link.p) : L"";
    }
    size_t bar1 = s.where.find(L'|'), bar2 = s.where.rfind(L'|');
    HKEY root = s.where.compare(0, 4, L"HKLM") == 0 ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    std::wstring key = s.where.substr(bar1 + 1, bar2 - bar1 - 1), value = s.where.substr(bar2 + 1);
    WCHAR data[2048];
    DWORD size = sizeof(data);
    return RegGetValueW(root, key.c_str(), value.empty() ? nullptr : value.c_str(),
                        RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, data, &size) == ERROR_SUCCESS
               ? data
               : L"";
}

bool Write(Saved const& s, std::wstring const& value) {
    if (s.type == L"lnk") {
        Link link;
        return OpenLink(s.where, STGM_READ, &link) && SetIcon(link, s.where, value);
    }
    size_t bar1 = s.where.find(L'|'), bar2 = s.where.rfind(L'|');
    HKEY root = s.where.compare(0, 4, L"HKLM") == 0 ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    std::wstring key = s.where.substr(bar1 + 1, bar2 - bar1 - 1), name = s.where.substr(bar2 + 1);
    return RegSetKeyValueW(root, key.c_str(), name.empty() ? nullptr : name.c_str(), REG_EXPAND_SZ, value.c_str(),
                           (DWORD)((value.size() + 1) * sizeof(WCHAR))) == ERROR_SUCCESS;
}

// What was there before: a shortcut's icon as IconOf gives it, a value's
// data after "=", or "-" for nothing.
std::wstring Before(Want const& want) {
    if (want.type == L"lnk") return want.source;
    WCHAR data[2048];
    DWORD size = sizeof(data);
    size_t bar1 = want.where.find(L'|'), bar2 = want.where.rfind(L'|');
    HKEY root = want.where.compare(0, 4, L"HKLM") == 0 ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    std::wstring key = want.where.substr(bar1 + 1, bar2 - bar1 - 1), name = want.where.substr(bar2 + 1);
    return RegGetValueW(root, key.c_str(), name.empty() ? nullptr : name.c_str(),
                        RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, nullptr, data, &size) == ERROR_SUCCESS
               ? L"=" + std::wstring(data)
               : L"-";
}

// Returns how many places changed.
int ApplySettings() {
    Run run;
    run.rows = SettingRows(ReadRuleSettings());
    Locked locked;
    std::vector<Saved> saved = ReadSaved();
    std::vector<Want> wants;
    if (!run.rows.empty()) {
        WantShortcuts(run, saved, wants);
        WantClasses(run, saved, wants, L"");
        WantClasses(run, saved, wants, L"CLSID");
        WantStockIcons(run, saved, wants);
    }
    int changed = 0;
    std::vector<Saved> kept;
    for (auto const& s : saved) {
        bool wanted = false;
        for (auto const& want : wants) {
            wanted = wanted || (want.type == s.type && _wcsicmp(want.where.c_str(), s.where.c_str()) == 0);
        }
        if (s.kind == kSettingKind && !wanted) {
            PutBackOne(s);
            changed++;
        } else {
            kept.push_back(s);
        }
    }
    saved = kept;
    for (auto const& want : wants) {
        Saved* s = FindSaved(saved, want.type, want.where);
        Saved fresh{kSettingKind, want.source, want.type, want.where, s ? L"" : Before(want)};
        Saved const& place = s ? *s : fresh;
        if (_wcsicmp(ReadNow(place).c_str(), want.value.c_str()) == 0) {
            if (!s) saved.push_back(fresh);
            continue;
        }
        if (Write(place, want.value)) {
            if (!s) saved.push_back(fresh);
            changed++;
        }
    }
    WriteSaved(saved);
    Wh_Log(L"Settings: %zu rows, %zu places, %d changed", run.rows.size(), wants.size(), changed);
    return changed;
}

// The settings run on a thread of their own (they read every shortcut and
// class), once more if they change while it runs.
std::atomic<bool> g_settingsRunning, g_settingsAgain;

DWORD WINAPI SettingsThread(LPVOID) {
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    int changed = 0;
    do {
        while (g_settingsAgain.exchange(false)) {
            // An icon theme is downloaded first, the first time it's picked.
            ThemeChoice theme = ParseTheme(ThemeSetting(ReadList()));
            std::wstring root = ThemesRoot();
            if (!theme.name.empty() && !root.empty() && ThemeIniIn(root + L"\\" + theme.name).empty()) {
                g_fetchStop = false;
                FetchThreadProc(nullptr);
            }
            changed += ApplySettings();
            changed += ApplyAll();  // (Stubborn redirects.)
        }
        g_settingsRunning = false;
        // (A change that came in just as it finished.)
    } while (g_settingsAgain && !g_settingsRunning.exchange(true));
    if (changed) TellWindowsIconsChanged();
    if (SUCCEEDED(com)) CoUninitialize();
    return 0;
}

void StartSettings() {
    g_settingsAgain = true;
    if (g_settingsRunning.exchange(true)) return;
    HANDLE thread = CreateThread(nullptr, 0, SettingsThread, nullptr, 0, nullptr);
    if (thread) {
        CloseHandle(thread);
    } else {
        g_settingsRunning = false;
    }
}

}  // namespace lasting

int g_lastingPlaces = 0;  // How many places the last lasting change reached.
bool g_holdLasting = false;  // A pack is going in: its shortcuts are done at the end, in one pass.

// Adds (or replaces) a row; the file type's own setting is made here too.
// restartStart false leaves restarting Start to the caller (a pack applies
// many Store apps' logos and restarts it once).
bool SaveChange(ListRow row, bool restartStart = true) {
    std::vector<ListRow> rows = ReadList();
    std::vector<std::wstring> oldPictures;
    for (auto it = rows.begin(); it != rows.end();) {
        if (it->kind == row.kind && _wcsicmp(it->original.c_str(), row.original.c_str()) == 0) {
            if (row.kind == L"package" && !it->previous.empty()) {
                oldPictures.push_back(it->previous);
            }
            if (row.kind == L"filetype" && row.previous.empty()) {
                row.previous = it->previous;  // What it had before any of this.
            }
            it = rows.erase(it);
        } else {
            ++it;
        }
    }
    if (row.kind == L"filetype" && !SetFileTypeIcon(&row)) {
        return false;
    }
    rows.push_back(row);
    if (!WriteList(rows)) {
        return false;
    }
    for (auto const& name : oldPictures) {
        if (name != row.previous) store::DeleteTree(StorageRoot() + L"\\packages\\" + name);
    }
    if (row.kind == L"package" && restartStart) store::RestartStart();
    if (kLasting && !g_holdLasting && (row.kind == L"program" || row.kind == L"icon")) {
        g_lastingPlaces = lasting::Apply(row);
    }
    return true;
}

bool RemoveChange(ListRow const& row) {
    std::vector<ListRow> rows = ReadList();
    std::vector<std::wstring> pictures;
    bool removed = false;
    for (auto it = rows.begin(); it != rows.end();) {
        if (it->kind == row.kind && _wcsicmp(it->original.c_str(), row.original.c_str()) == 0) {
            if (it->kind == L"filetype") {
                PutBackFileTypeIcon(*it);
            }
            if (kLasting) {
                lasting::PutBack(*it);
            }
            if (it->kind == L"package" && !it->previous.empty()) {
                pictures.push_back(it->previous);
            }
            it = rows.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    if (!removed || !WriteList(rows)) {
        return false;
    }
    // (Any still open in a program stay until it lets go; nothing uses them.)
    for (auto const& name : pictures) store::DeleteTree(StorageRoot() + L"\\packages\\" + name);
    if (!pictures.empty()) store::RestartStart();
    if (kLasting) lasting::StartSettings();  // What the settings had there, back.
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Taking in what was set before (once, the first time the tool runs)
//
// Tourne Desktop's Button icons rows, and Icon Redirect's rows in the
// Tourne'Suite build (and in the older Tourne'Icons [Icon Redirect] mod), so
// this one list holds them all.

std::wstring TrimText(std::wstring s) {
    while (!s.empty() && iswspace(s.front())) s.erase(0, 1);
    while (!s.empty() && iswspace(s.back())) s.pop_back();
    if (s.size() >= 2 && (s.front() == L'"' || s.front() == L'\'') && s.back() == s.front()) {
        s = s.substr(1, s.size() - 2);
    }
    return s;
}

// Tourne Desktop's config (key = value, with name { ... } groups), flattened.
std::map<std::wstring, std::wstring> ReadTourneConfig(std::wstring const& path) {
    std::map<std::wstring, std::wstring> out;
    std::vector<BYTE> data = ReadWholeFile(path);
    if (data.empty()) {
        return out;
    }
    std::wstring text;
    if (data.size() >= 2 && data[0] == 0xFF && data[1] == 0xFE) {
        text.assign((const WCHAR*)(data.data() + 2), (data.size() - 2) / 2);
    } else {
        size_t skip = data.size() >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF ? 3 : 0;
        int length = MultiByteToWideChar(CP_UTF8, 0, (const char*)data.data() + skip, (int)(data.size() - skip), nullptr, 0);
        text.resize(length);
        MultiByteToWideChar(CP_UTF8, 0, (const char*)data.data() + skip, (int)(data.size() - skip), text.data(), length);
    }
    std::vector<std::wstring> groups;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = TrimText(text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at));
        at = end == std::wstring::npos ? text.size() : end + 1;
        if (line.empty() || line[0] == L'#') continue;
        if (line == L"}") {
            if (!groups.empty()) groups.pop_back();
            continue;
        }
        if (line.back() == L'{') {
            groups.push_back(TrimText(line.substr(0, line.size() - 1)));
            continue;
        }
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring key = TrimText(line.substr(0, eq)), value = line.substr(eq + 1);
        value = TrimText(value);
        if (!value.empty() && value[0] != L'"' && value[0] != L'\'') {
            size_t hash = value.find(L" #");
            if (hash != std::wstring::npos) value = TrimText(value.substr(0, hash));
        }
        std::wstring full;
        for (auto const& g : groups) full += g + L".";
        out[full + key] = value;
    }
    return out;
}

// An icon from an old row ("file.ico", "file,index", a picture) made into one
// of ours. Empty when it can't be.
std::wstring TakeInIcon(std::wstring const& value, std::wstring const& label) {
    std::wstring location = CleanPath(value);
    std::wstring file = location;
    int number = 0;
    if (SplitIconLocation(location, &file, &number)) {
        file = CleanPath(file);
    } else {
        number = 0;
    }
    if (file.empty() || _wcsnicmp(file.c_str(), L"tourne:", 7) == 0 || !FileExists(file)) {
        return {};
    }
    return SaveIco(MakeIco(file, number), label);
}

std::wstring FileTitle(std::wstring const& path) {
    size_t slash = path.find_last_of(L"\\/");
    std::wstring name = slash == std::wstring::npos ? path : path.substr(slash + 1);
    size_t dot = name.rfind(L'.');
    return dot == std::wstring::npos || dot == 0 ? name : name.substr(0, dot);
}

// A Windhawk mod's settings (read only, so no admin).
std::wstring ModSettingsKey(std::wstring const& mod) {
    return L"SOFTWARE\\Windhawk\\Engine\\Mods\\" + mod + L"\\Settings";
}

std::wstring ReadModString(std::wstring const& key, std::wstring const& name) {
    WCHAR buffer[2048];
    DWORD size = sizeof(buffer);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, key.c_str(), name.c_str(), RRF_RT_REG_SZ, nullptr, buffer, &size) != ERROR_SUCCESS) {
        return {};
    }
    return buffer;
}

// A number setting, or fallback when it isn't there.
DWORD ReadModNumber(std::wstring const& key, std::wstring const& name, DWORD fallback) {
    DWORD value = 0, size = sizeof(value);
    return RegGetValueW(HKEY_LOCAL_MACHINE, key.c_str(), name.c_str(), RRF_RT_REG_DWORD, nullptr, &value, &size) ==
                   ERROR_SUCCESS
               ? value
               : fallback;
}

// The newest Tourne'Suite build that's switched on.
std::wstring SuiteModName() {
    HKEY mods;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods", 0, KEY_READ, &mods) != ERROR_SUCCESS) {
        return {};
    }
    std::wstring best;
    int bestNumber = -1;
    WCHAR name[256];
    for (DWORD i = 0;; i++) {
        DWORD length = ARRAYSIZE(name);
        if (RegEnumKeyExW(mods, i, name, &length, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) {
            break;
        }
        const WCHAR* at = wcsstr(name, L"tourne-suite-b");
        if (!at) continue;
        DWORD disabled = 0, size = sizeof(disabled);
        RegGetValueW(mods, name, L"Disabled", RRF_RT_REG_DWORD, nullptr, &disabled, &size);
        int number = _wtoi(at + 14);
        if (!disabled && number > bestNumber) {
            bestNumber = number;
            best = name;
        }
    }
    RegCloseKey(mods);
    return best;
}

// Tourne Desktop's own Button icons rows (its settings and Studio's), in order.
struct DesktopButtonIcon {
    std::wstring program, icon;
    bool everywhere = true;  // Use everywhere: its program's icon wherever Windows shows it, too.
};

std::vector<DesktopButtonIcon> DesktopButtonIcons() {
    std::vector<DesktopButtonIcon> rows;
    WCHAR appData[MAX_PATH];
    if (!GetEnvironmentVariableW(L"APPDATA", appData, ARRAYSIZE(appData))) return rows;
    std::wstring folder = std::wstring(appData) + L"\\Tourne\\Desktop\\";
    auto config = ReadTourneConfig(folder + L"tourne.conf");
    for (auto const& [k, v] : ReadTourneConfig(folder + L"tourne-gui.conf")) config[k] = v;
    for (int i = 0; i < 256; i++) {
        WCHAR key[96];
        swprintf_s(key, L"taskbar.buttonIcons[%d].program", i);
        auto program = config.find(key);
        if (program == config.end()) continue;
        swprintf_s(key, L"taskbar.buttonIcons[%d].icon", i);
        auto iconValue = config.find(key);
        swprintf_s(key, L"taskbar.buttonIcons[%d].everywhere", i);
        auto everywhere = config.find(key);
        bool off = everywhere != config.end() && (TrimText(everywhere->second) == L"false" || TrimText(everywhere->second) == L"0");
        if (iconValue != config.end()) rows.push_back({program->second, iconValue->second, !off});
    }
    return rows;
}

// Icon changes made outside the list: Icon Redirect's rows (in the suite as
// iconRedirect.*, and in the older standalone mod) with the icon theme, and
// the Button icons rows marked Use everywhere (Tourne Desktop's, and the
// suite's taskbar part's).
struct EarlierSink {
    std::function<void(std::wstring const& original, std::wstring const& redirect)> redirect;
    std::function<void(std::wstring const& theme)> theme;
    std::function<void(std::wstring const& match, std::wstring const& icon)> buttonIcon;
};

void ReadEarlier(EarlierSink const& sink) {
    auto readRedirects = [&](std::wstring const& key, std::wstring const& prefix) {
        for (int i = 0, empty = 0; i < 1024 && empty < 8; i++) {
            WCHAR name[128];
            swprintf_s(name, L"%sredirectionResourcePaths[%d].original", prefix.c_str(), i);
            std::wstring original = ReadModString(key, name);
            swprintf_s(name, L"%sredirectionResourcePaths[%d].redirect", prefix.c_str(), i);
            std::wstring redirect = ReadModString(key, name);
            empty = original.empty() && redirect.empty() ? empty + 1 : 0;
            if (!original.empty() && !redirect.empty() && sink.redirect) sink.redirect(original, redirect);
        }
        std::wstring theme = ReadModString(key, prefix + L"iconTheme");
        if (!theme.empty() && sink.theme) sink.theme(theme);
    };
    readRedirects(ModSettingsKey(L"local@tourne-icon-redirect"), L"");
    std::wstring suite = SuiteModName();
    if (!suite.empty()) {
        readRedirects(ModSettingsKey(suite), L"iconRedirect.");
    }
    if (!sink.buttonIcon) return;
    for (auto const& row : DesktopButtonIcons()) {
        if (row.everywhere) sink.buttonIcon(row.program, row.icon);
    }
    if (!suite.empty()) {
        std::wstring key = ModSettingsKey(suite);
        for (int i = 0, empty = 0; i < 256 && empty < 8; i++) {
            WCHAR name[96];
            swprintf_s(name, L"taskbar.buttonIcons[%d].program", i);
            std::wstring program = ReadModString(key, name);
            swprintf_s(name, L"taskbar.buttonIcons[%d].icon", i);
            std::wstring icon = ReadModString(key, name);
            empty = program.empty() && icon.empty() ? empty + 1 : 0;
            swprintf_s(name, L"taskbar.buttonIcons[%d].everywhere", i);
            if (ReadModNumber(key, name, 1)) sink.buttonIcon(program, icon);
        }
    }
}

// A Button icons row's program as a list pattern: a path, or any folder's
// file of that name ("" for a web app's command line or a Store app).
std::wstring ButtonIconPattern(std::wstring match) {
    match = TrimText(match);
    if (match.empty() || match.find(L" --") != std::wstring::npos || match.find(L'!') != std::wstring::npos) return {};
    bool path = match.find_first_of(L"\\/") != std::wstring::npos;
    return Lower(path ? CleanPath(match) : L"*\\" + match);
}

// Whether a Taskbar buttons row stands for just the program a program row
// names (a path, or *\name.exe).
bool ButtonOfProgram(std::wstring const& button, std::wstring const& program) {
    std::wstring b = Lower(button), p = Lower(program);
    // An app id, a web app's command line, or a pattern for many programs (*\NVIDIA Corporation\*.exe): no.
    if (b.find_first_of(L"!*?") != std::wstring::npos || b.find(L" --") != std::wstring::npos) return false;
    std::wstring name = p.substr(p.find_last_of(L"\\/") + 1);
    if (name.empty() || name.find_first_of(L"*?") != std::wstring::npos) return false;
    bool path = b.find_first_of(L"\\/") != std::wstring::npos;
    // A path only for that same path, or for any folder's file of its name.
    if (path) return b == p || (p.find_first_of(L"*?") != std::wstring::npos && b.substr(b.find_last_of(L"\\/") + 1) == name);
    return b == name;
}

// Returns how many changes were taken in.
int TakeInEarlierChanges() {
    std::vector<ListRow> rows;
    auto add = [&](ListRow row) {
        for (auto const& r : rows) {
            if (r.kind == row.kind && _wcsicmp(r.original.c_str(), row.original.c_str()) == 0) return;
        }
        rows.push_back(std::move(row));
    };

    EarlierSink sink;
    // An .ico stands in for the program's main icon; anything else is a whole file.
    sink.redirect = [&](std::wstring const& original, std::wstring const& redirect) {
        std::wstring file = CleanPath(redirect);
        if (EndsWithI(file, L".ico")) {
            std::wstring icon = TakeInIcon(file, FileTitle(original));
            if (!icon.empty()) add({L"program", Lower(CleanPath(original)), icon, FileTitle(original)});
        } else {
            add({L"file", CleanPath(original), file, FileTitle(original)});
        }
    };
    sink.theme = [&](std::wstring const& theme) { add({L"theme", theme, L"", ParseTheme(theme).name}); };
    // A Store app goes by its app id (Microsoft.WindowsCalculator_8wekyb3d8bbwe!App). A row with
    // a command line (Chrome.exe --app=...) stands for one web app's buttons,
    // which no file holds.
    sink.buttonIcon = [&](std::wstring match, std::wstring const& iconValue) {
        match = TrimText(match);
        if (match.empty() || iconValue.empty() || match.find(L" --") != std::wstring::npos) {
            return;
        }
        if (match.find(L'!') != std::wstring::npos) {
            if (kLasting) return;  // (A Store app's logo can't last.)
            std::wstring family = match.substr(0, match.find(L'!'));
            std::wstring folder = store::MainFolderOf(family);
            if (folder.empty()) return;
            std::wstring label = family.substr(0, family.find(L'_'));
            label = label.substr(label.find_last_of(L'.') + 1);
            ListRow row{L"package", store::Pattern(folder), TakeInIcon(iconValue, label), label,
                        store::Logos(folder, match.substr(match.find(L'!') + 1)), L""};
            if (!row.icon.empty() && store::MakeRow(&row)) add(row);
            return;
        }
        std::wstring icon = TakeInIcon(iconValue, FileTitle(match));
        if (icon.empty()) return;
        add({L"program", ButtonIconPattern(match), icon, FileTitle(match)});
    };
    ReadEarlier(sink);

    if (kLasting) {
        // Only what can last: programs' shortcuts and Windows' own icons.
        std::erase_if(rows, [](ListRow const& r) { return r.kind != L"program" && r.kind != L"icon"; });
    }
    if (rows.empty()) {
        return 0;
    }
    // Ahead of anything already in the list, so what you pick now still wins.
    std::vector<ListRow> list = ReadList();
    int added = 0;
    for (auto& row : rows) {
        bool there = false;
        for (auto const& r : list) {
            there = there || (r.kind == row.kind && _wcsicmp(r.original.c_str(), row.original.c_str()) == 0);
        }
        if (!there) {
            list.insert(list.begin() + added, row);
            added++;
        }
    }
    return added && WriteList(list) ? added : 0;
}

// Tourne Desktop's Button icons rows as Taskbar buttons rows, so Tourne'Icons
// holds them all (Tourne Desktop's step aside while the live one runs). The
// icon is made into one of ours; a design of Tourne Desktop's (tourne:name)
// stays as it is. Returns how many were taken in.
int TakeInButtons() {
    std::vector<ListRow> list = ReadList(), rows;
    for (auto const& desktop : DesktopButtonIcons()) {
        std::wstring match = Lower(TrimText(desktop.program)), value = TrimText(desktop.icon);
        if (match.empty() || value.empty()) continue;
        bool there = false;
        for (auto const& r : list) there = there || (r.kind == L"button" && r.original == match);
        for (auto const& r : rows) there = there || r.original == match;
        if (there) continue;
        std::wstring label = match;
        if (size_t bang = match.find(L'!'); bang != std::wstring::npos) {
            label = match.substr(0, bang);
            label = label.substr(0, label.find(L'_'));
            label = label.substr(label.find_last_of(L'.') + 1);
        } else if (size_t dash = match.find(L" --"); dash != std::wstring::npos) {
            label = FileTitle(match.substr(0, dash)) + L" app";
        } else if (match.find_first_of(L"\\/*?") != std::wstring::npos) {
            // A path or pattern (*\Process Lasso\*.exe): its last part that names something.
            std::wstring part;
            for (size_t at = 0; at <= match.size();) {
                size_t end = match.find_first_of(L"\\/", at);
                std::wstring piece = match.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
                if (!piece.empty() && piece.find_first_of(L"*?") == std::wstring::npos) part = piece;
                if (end == std::wstring::npos) break;
                at = end + 1;
            }
            label = part.empty() ? match : EndsWithI(part, L".exe") || EndsWithI(part, L".ico") ? FileTitle(part) : part;
        } else if (EndsWithI(match, L".exe")) {
            label = FileTitle(match);
        }
        std::wstring icon;
        if (_wcsnicmp(value.c_str(), L"tourne:", 7) == 0) {
            icon = value;
        } else {
            // The same picture taken in before for the program (as a program row) is used again.
            std::wstring pattern = ButtonIconPattern(match);
            for (auto const& r : list) {
                if (icon.empty() && !pattern.empty() && r.kind == L"program" && r.original == pattern) icon = r.icon;
            }
            if (icon.empty() || !FileExists(icon)) icon = TakeInIcon(value, label);
        }
        if (!icon.empty()) rows.push_back({L"button", match, icon, label});
    }
    if (rows.empty()) return 0;
    // Ahead of the rest, the last first: newer rows come first when handed
    // over, and Tourne Desktop's first row wins as it did.
    list.insert(list.begin(), rows.rbegin(), rows.rend());
    return WriteList(list) ? (int)rows.size() : 0;
}

////////////////////////////////////////////////////////////////////////////////
// The tool process's own Windows functions
//
// Looked up when the tool starts, not linked, so the mod loads nothing more
// into every other program than its hooks need.

namespace api {

decltype(&::GetFileVersionInfoSizeW) GetFileVersionInfoSizeW;
decltype(&::GetFileVersionInfoW) GetFileVersionInfoW;
decltype(&::VerQueryValueW) VerQueryValueW;
decltype(&::AssocQueryStringW) AssocQueryStringW;
decltype(&::DwmGetWindowAttribute) DwmGetWindowAttribute;
decltype(&::SetWindowTheme) SetWindowTheme;
decltype(&::GetOpenFileNameW) GetOpenFileNameW;
decltype(&::InitCommonControlsEx) InitCommonControlsEx;
decltype(&::ImageList_Create) ImageList_Create;
decltype(&::ImageList_ReplaceIcon) ImageList_ReplaceIcon;
decltype(&::ImageList_Remove) ImageList_Remove;
decltype(&::ImageList_Destroy) ImageList_Destroy;

template <typename T>
bool Find(HMODULE module, PCSTR name, T* function) {
    *function = module ? (T)GetProcAddress(module, name) : nullptr;
    return *function != nullptr;
}

bool Load() {
    auto load = [](PCWSTR name) { return LoadLibraryExW(name, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); };
    HMODULE version = load(L"version.dll"), shlwapi = load(L"shlwapi.dll"), dwm = load(L"dwmapi.dll"),
            uxtheme = load(L"uxtheme.dll"), comdlg = load(L"comdlg32.dll"), comctl = load(L"comctl32.dll");
    return Find(version, "GetFileVersionInfoSizeW", &GetFileVersionInfoSizeW) &&
           Find(version, "GetFileVersionInfoW", &GetFileVersionInfoW) &&
           Find(version, "VerQueryValueW", &VerQueryValueW) &&
           Find(shlwapi, "AssocQueryStringW", &AssocQueryStringW) &&
           Find(dwm, "DwmGetWindowAttribute", &DwmGetWindowAttribute) &&
           Find(uxtheme, "SetWindowTheme", &SetWindowTheme) &&
           Find(comdlg, "GetOpenFileNameW", &GetOpenFileNameW) &&
           Find(comctl, "InitCommonControlsEx", &InitCommonControlsEx) &&
           Find(comctl, "ImageList_Create", &ImageList_Create) &&
           Find(comctl, "ImageList_ReplaceIcon", &ImageList_ReplaceIcon) &&
           Find(comctl, "ImageList_Remove", &ImageList_Remove) &&
           Find(comctl, "ImageList_Destroy", &ImageList_Destroy);
}

}  // namespace api

////////////////////////////////////////////////////////////////////////////////
// The picker
//
// A window of the tool process: on the left where to look, in the middle
// what's there (with the icons it shows now), and below the one you picked,
// its new icon at four sizes, and the buttons.

namespace picker {

struct Item {
    std::wstring name;
    std::wstring detail;    // What changes, in words.
    std::wstring kind;      // program, icon, filetype; empty when it can't be changed.
    std::wstring original;  // What the row matches.
    std::wstring iconFile;  // Where its icon shows from, to draw it.
    int iconIndex = 0;
    std::wstring progId;    // A file type's.
    std::wstring note;      // Why it can't be changed.
    bool change = false;    // One of your changes.
    ListRow row;            // That change.
};

enum Source { kTaskbar, kButtons, kTray, kDesktop, kStart, kInstalled, kControlPanel, kWindows, kFileTypes, kChanges, kSourceCount };
constexpr PCWSTR kSourceNames[] = {L"On the taskbar", L"Taskbar buttons", L"In the tray", L"On the desktop", L"In Start",
                                   L"Installed apps", L"Control Panel", L"Windows icons", L"File types",
                                   L"Your changes"};

enum : int {
    kIdSources = 100,
    kIdSearch,
    kIdItems,
    kIdChoose,
    kIdFromProgram,
    kIdApply,
    kIdPutBack,
    kIdAnyFile,
    kIdRefresh,
    kIdPreview,
    kIdTitle,
    kIdDetail,
    kIdStatus,
    kIdSaveLists,
    kIdApplyPack,
    kIdClearCache,
};

constexpr WCHAR kClass[] = L"TourneIcons_Picker";
constexpr UINT_PTR kNotifyTimer = 1;
// The Kit's start: wParam is how many shortcuts got their icon back.
constexpr UINT kPutBackMessage = WM_APP + 20;
// Clear icon cache finished: wParam is how many files were left (-1: no folder).
constexpr UINT kClearedMessage = WM_APP + 21;

HWND g_window, g_sources, g_search, g_items, g_title, g_detail, g_status, g_preview;
HWND g_choose, g_fromProgram, g_apply, g_putBack, g_anyFile, g_refresh, g_saveLists, g_applyPack, g_clearCache;
HFONT g_font, g_bold;
HIMAGELIST g_images;
int g_dpi = 96;
Source g_source = kTaskbar;
std::vector<Item> g_all;     // The source's items.
std::vector<int> g_shown;    // Indexes into g_all, after the search.
Item g_picked;               // Picked from the list or with "Pick from any file".
bool g_hasPicked = false;
std::vector<BYTE> g_newIco;  // The new icon, made but not yet saved.
HICON g_newIcons[4];         // It at 16, 24, 32 and 48, for the preview.
HICON g_nowIcon;             // The picked one as it shows now, at 48.

int Scale(int value) {
    return MulDiv(value, g_dpi, 96);
}

void SetStatus(std::wstring const& text) {
    SetWindowTextW(g_status, text.c_str());
}

std::wstring ExpandText(std::wstring const& s) {
    return CleanPath(s);
}

// "Microsoft Teams" from the program's own description, else its file name.
std::wstring ProgramName(std::wstring const& path) {
    DWORD ignored = 0, size = api::GetFileVersionInfoSizeW(path.c_str(), &ignored);
    if (size) {
        std::vector<BYTE> data(size);
        struct Translation {
            WORD language, codePage;
        }* translations = nullptr;
        UINT length = 0;
        if (api::GetFileVersionInfoW(path.c_str(), 0, size, data.data()) &&
            api::VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (void**)&translations, &length) &&
            length >= sizeof(Translation)) {
            WCHAR query[64];
            swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\FileDescription", translations[0].language,
                       translations[0].codePage);
            WCHAR* text = nullptr;
            if (api::VerQueryValueW(data.data(), query, (void**)&text, &length) && text && length > 1 && *text) {
                return TrimText(text);
            }
        }
    }
    return FileTitle(path);
}

// A Store app's package folder from a file in it, or empty.
std::wstring StoreFolderOf(std::wstring const& path) {
    std::wstring lower = Lower(path);
    for (PCWSTR part : {L"\\windowsapps\\", L"\\systemapps\\"}) {
        size_t at = lower.find(part);
        if (at != std::wstring::npos) {
            size_t end = lower.find(L'\\', at + wcslen(part));
            return end == std::wstring::npos ? std::wstring() : path.substr(0, end);
        }
    }
    size_t at = lower.find(L"\\immersivecontrolpanel\\");
    return at == std::wstring::npos ? std::wstring() : path.substr(0, at + 22);
}

// The logo file that shows it best in the list: about 48 pixels, unplated.
std::wstring ListLogo(std::wstring const& folder, std::wstring const& stems) {
    std::vector<store::LogoFile> files;
    std::wstring first = stems.substr(0, stems.find(L';'));
    store::FindLogoFiles(folder, first, files);
    std::wstring best;
    int bestScore = -1;
    for (auto const& file : files) {
        UINT width = 0, height = 0;
        if (!store::PngSize(file.path, &width, &height)) continue;
        int score = (width >= 48 ? 2000 - (int)width : (int)width) +
                    (file.relative.find(L"unplated") != std::wstring::npos ? 3000 : 0) +
                    (file.relative.find(L"contrast") != std::wstring::npos ? -5000 : 0);
        if (score > bestScore) {
            bestScore = score;
            best = file.path;
        }
    }
    return best;
}

// A Store app, by its package folder and its id in the package ("App").
Item StoreItem(std::wstring const& folder, std::wstring const& appId, std::wstring const& name) {
    Item item;
    item.name = name;
    item.detail = folder;
    if (kLasting) {
        item.note = std::wstring(L"A Store app: its logo can only change while ") + kLiveOne + L" runs.";
        item.iconFile = ListLogo(folder, store::Logos(folder, appId));
        return item;
    }
    std::wstring stems = store::Logos(folder, appId);
    if (stems.empty()) {
        item.note = L"A Store app whose logo pictures couldn't be found in its package (" + folder + L").";
        return item;
    }
    item.kind = L"package";
    item.original = store::Pattern(folder);
    item.progId = stems;
    item.iconFile = ListLogo(folder, stems);
    item.detail = L"A Store app: changes its logo on the taskbar, in Start, Alt+Tab, Settings and File Explorer. (" +
                  folder + L")";
    return item;
}

// By its full app id ("Microsoft.WindowsCalculator_8wekyb3d8bbwe!App").
Item StoreItemFor(std::wstring const& fullAppId, std::wstring const& name) {
    size_t bang = fullAppId.find(L'!');
    std::wstring folder = store::MainFolderOf(fullAppId.substr(0, bang));
    if (folder.empty()) {
        Item item;
        item.name = name;
        item.note = L"A Store app that couldn't be found on this PC (" + fullAppId + L").";
        return item;
    }
    return StoreItem(folder, bang == std::wstring::npos ? L"" : fullAppId.substr(bang + 1), name);
}

Item ProgramItem(std::wstring const& path, std::wstring name = L"", std::wstring const& fullAppId = L"") {
    Item item;
    item.name = name.empty() ? ProgramName(path) : name;
    item.iconFile = path;
    item.iconIndex = 0;
    if (!fullAppId.empty()) {
        return StoreItemFor(fullAppId, item.name);
    }
    std::wstring storeFolder = StoreFolderOf(path);
    if (!storeFolder.empty()) {
        return StoreItem(storeFolder, L"", item.name);
    }
    if (EndsWithI(path, L"\\applicationframehost.exe")) {
        item.note = L"A Store app's window; pick the app in In Start.";
        return item;
    }
    item.kind = L"program";
    item.original = ProgramPattern(path);
    item.detail = kLasting ? L"Changes the icon on " + FileTitle(path) +
                                 L"'s shortcuts: Start, the desktop and pinned taskbar buttons. Its windows and tray "
                                 L"icon keep their own (" + kLiveOne + L" changes those). (" +
                                 path + L")"
                           : L"Changes " + FileTitle(path) + L"'s icon everywhere: taskbar, Start, tray, its windows, File Explorer. (" +
                  path + L")";
    return item;
}

// One icon in a file ("file,index", or a picture file).
Item IconItem(std::wstring const& file, int index, std::wstring const& name) {
    std::wstring expanded = ExpandText(file);
    if (EndsWithI(expanded, L".exe") && index == 0) {
        return ProgramItem(expanded, name);
    }
    Item item;
    item.name = name;
    item.iconFile = expanded;
    item.iconIndex = index;
    item.kind = L"icon";
    bool module = IsModuleFile(expanded);
    item.original = module ? expanded + L"," + std::to_wstring(index) : expanded;
    item.detail = module ? L"Changes this icon wherever Windows shows it (" + FileTitle(expanded) + L" icon " +
                               std::to_wstring(index) + L")."
                         : L"Changes this picture wherever it's used (" + expanded + L").";
    if (kLasting && !lasting::CanLast(item.original)) {
        item.kind.clear();
        item.note = std::wstring(L"Windows keeps no setting of its own for this icon, so it can only change while ") +
                    kLiveOne + L" runs. Folders, drives, This PC, the Recycle Bin, Network, Control Panel and your "
                               L"user folders can change here.";
    } else if (kLasting) {
        item.detail = std::wstring(L"Changes this icon in Windows' own settings, so it stays ") + kStays + L".";
    }
    return item;
}

// A file type, by its extension (".txt").
Item FileTypeItem(std::wstring ext) {
    ext = Lower(ext);
    Item item;
    item.kind = L"filetype";
    item.original = ext;
    WCHAR buffer[1024];
    DWORD size = ARRAYSIZE(buffer);
    std::wstring friendly;
    if (SUCCEEDED(api::AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_FRIENDLYDOCNAME, ext.c_str(), nullptr, buffer, &size))) {
        friendly = buffer;
    }
    item.name = (friendly.empty() ? ext.substr(1) + L" file" : friendly) + L" (" + ext + L")";
    size = ARRAYSIZE(buffer);
    if (SUCCEEDED(api::AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_PROGID, ext.c_str(), nullptr, buffer, &size))) {
        item.progId = buffer;
    }
    // Older ways, where the shell's answer is missing: the extension's own
    // key names its ProgID, and the ProgID's key its icon.
    bool exists = false;
    if (item.progId.empty()) {
        item.progId = ReadDefaultValue(HKEY_CLASSES_ROOT, ext, &exists);
    }
    size = ARRAYSIZE(buffer);
    std::wstring icon;
    if (SUCCEEDED(api::AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_DEFAULTICON, ext.c_str(), nullptr, buffer, &size))) {
        icon = TrimText(buffer);
    }
    if (icon.empty() && !item.progId.empty()) {
        icon = TrimText(ReadDefaultValue(HKEY_CLASSES_ROOT, item.progId + L"\\DefaultIcon", &exists));
    }
    if (icon == L"%1" || icon.empty()) {
        item.kind.clear();
        item.note = L"Each of these files shows its own picture, so there's no one icon to change.";
    }
    std::wstring file;
    int number = 0;
    if (SplitIconLocation(icon, &file, &number)) {
        item.iconFile = ExpandText(file);
        item.iconIndex = number;
    } else {
        item.iconFile = ExpandText(icon);
    }
    item.detail = L"Changes the icon of every " + ext + L" file.";
    return item;
}

struct ShortcutInfo {
    std::wstring target, iconFile, appId;
    int iconIndex = 0;
    bool storeApp = false;
};

ShortcutInfo ReadShortcut(std::wstring const& path) {
    ShortcutInfo info;
    Com<IShellLinkW> link;
    Com<IPersistFile> file;
    if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&link)) ||
        FAILED(link->QueryInterface(IID_IPersistFile, (void**)&file)) || FAILED(file->Load(path.c_str(), STGM_READ))) {
        return info;
    }
    WCHAR buffer[MAX_PATH * 2] = L"";
    int index = 0;
    if (SUCCEEDED(link->GetIconLocation(buffer, ARRAYSIZE(buffer), &index)) && buffer[0]) {
        info.iconFile = ExpandText(buffer);
        info.iconIndex = index;
    }
    buffer[0] = 0;
    if (SUCCEEDED(link->GetPath(buffer, ARRAYSIZE(buffer), nullptr, SLGP_RAWPATH)) && buffer[0]) {
        info.target = ExpandText(buffer);
    } else {
        info.storeApp = true;  // A shortcut to an app with no file of its own.
        // System.AppUserModel.ID.
        constexpr PROPERTYKEY kAppIdKey = {{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 5};
        Com<IPropertyStore> properties;
        PROPVARIANT value;
        PropVariantInit(&value);
        if (SUCCEEDED(link->QueryInterface(__uuidof(IPropertyStore), (void**)&properties)) &&
            SUCCEEDED(properties->GetValue(kAppIdKey, &value)) && value.vt == VT_LPWSTR && value.pwszVal) {
            info.appId = value.pwszVal;
        }
        PropVariantClear(&value);
    }
    return info;
}

// What a file on the desktop or in Start stands for.
bool ItemForFile(std::wstring const& path, std::wstring const& name, Item* item) {
    std::wstring lower = Lower(path);
    if (EndsWithI(lower, L".lnk")) {
        ShortcutInfo link = ReadShortcut(path);
        if (!link.iconFile.empty() && FileExists(link.iconFile)) {
            *item = IconItem(link.iconFile, link.iconIndex, name);
            return true;
        }
        if (link.storeApp) {
            if (link.appId.find(L'!') != std::wstring::npos) {
                *item = StoreItemFor(link.appId, name);
            } else {
                item->name = name;
                item->note = L"A shortcut to an app with no file of its own; pick the app in In Start.";
            }
            return true;
        }
        if (FolderExists(link.target)) {
            *item = IconItem(L"%SystemRoot%\\System32\\imageres.dll", -3, name);
            item->detail = L"A folder: this changes the icon of every folder.";
            return true;
        }
        if (EndsWithI(link.target, L".exe") && FileExists(link.target)) {
            *item = ProgramItem(link.target, name);
            return true;
        }
        size_t dot = link.target.rfind(L'.');
        if (dot != std::wstring::npos && link.target.find(L'\\', dot) == std::wstring::npos) {
            *item = FileTypeItem(link.target.substr(dot));
            item->name = name;
            return true;
        }
        return false;
    }
    if (EndsWithI(lower, L".url")) {
        WCHAR icon[MAX_PATH * 2] = L"";
        GetPrivateProfileStringW(L"InternetShortcut", L"IconFile", L"", icon, ARRAYSIZE(icon), path.c_str());
        if (icon[0] && FileExists(ExpandText(icon))) {
            *item = IconItem(icon, GetPrivateProfileIntW(L"InternetShortcut", L"IconIndex", 0, path.c_str()), name);
            return true;
        }
    }
    if (EndsWithI(lower, L".exe")) {
        *item = ProgramItem(path, name);
        return true;
    }
    if (FolderExists(path)) {
        *item = IconItem(L"%SystemRoot%\\System32\\imageres.dll", -3, name);
        item->detail = L"A folder: this changes the icon of every folder.";
        return true;
    }
    size_t dot = path.rfind(L'.');
    if (dot != std::wstring::npos && path.find(L'\\', dot) == std::wstring::npos) {
        *item = FileTypeItem(path.substr(dot));
        item->name = name + L" (all " + Lower(path.substr(dot)) + L" files)";
        return true;
    }
    return false;
}

std::wstring KnownFolder(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path))) {
        out = path;
    }
    CoTaskMemFree(path);
    return out;
}

void AddUnique(std::vector<Item>& items, Item item) {
    if (!item.kind.empty()) {
        for (auto const& i : items) {
            if (i.kind == item.kind && _wcsicmp(i.original.c_str(), item.original.c_str()) == 0) {
                return;
            }
        }
    }
    items.push_back(std::move(item));
}

void AddFolderFiles(std::vector<Item>& items, std::wstring const& folder, int depth, bool shortcutsOnly) {
    if (folder.empty() || depth > 3) {
        return;
    }
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((folder + L"\\*").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        std::wstring name = find.cFileName;
        if (name == L"." || name == L".." || (find.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))) {
            continue;
        }
        std::wstring path = folder + L"\\" + name;
        if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (shortcutsOnly) {
                AddFolderFiles(items, path, depth + 1, true);
                continue;
            }
        } else if (shortcutsOnly && !EndsWithI(name, L".lnk")) {
            continue;
        }
        Item item;
        std::wstring shown = EndsWithI(name, L".lnk") || EndsWithI(name, L".url") ? FileTitle(name) : name;
        if (ItemForFile(path, shown, &item)) {
            AddUnique(items, std::move(item));
        }
    } while (FindNextFileW(search, &find));
    FindClose(search);
}

// Windows' own icons that people change most, by name.
struct NamedIcon {
    PCWSTR name;
    int id;
};
constexpr NamedIcon kNamedIcons[] = {
    {L"Folders", 3},         {L"This PC", 109},       {L"Recycle Bin (empty)", 55}, {L"Recycle Bin (full)", 54},
    {L"Network", 25},        {L"Your user folder", 123}, {L"Documents", 112},      {L"Pictures", 113},
    {L"Music", 108},         {L"Videos", 189},        {L"Downloads", 184},          {L"Desktop", 183},
    {L"Control Panel", 27},
};

BOOL CALLBACK CollectGroups(HMODULE, LPCWSTR, LPWSTR name, LONG_PTR param) {
    auto* ids = (std::vector<int>*)param;
    if (IS_INTRESOURCE(name)) {
        ids->push_back(LOWORD(name));
    }
    return TRUE;
}

void GatherWindowsIcons(std::vector<Item>& items) {
    for (auto const& named : kNamedIcons) {
        Item item = IconItem(L"%SystemRoot%\\System32\\imageres.dll", -named.id, named.name);
        AddUnique(items, item);
    }
    WCHAR system[MAX_PATH];
    GetSystemDirectoryW(system, ARRAYSIZE(system));
    std::wstring imageres = std::wstring(system) + L"\\imageres.dll";
    HMODULE module = LoadLibraryExW(imageres.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (module) {
        std::vector<int> ids;
        EnumResourceNamesW(module, RT_GROUP_ICON, CollectGroups, (LONG_PTR)&ids);
        FreeLibrary(module);
        for (int id : ids) {
            AddUnique(items, IconItem(L"%SystemRoot%\\System32\\imageres.dll", -id, L"Windows icon " + std::to_wstring(id)));
        }
    }
}

void GatherFileTypes(std::vector<Item>& items) {
    std::set<std::wstring> seen;
    auto add = [&](std::wstring ext) {
        ext = Lower(ext);
        if (ext.size() < 2 || ext[0] != L'.' || !seen.insert(ext).second) return;
        Item item = FileTypeItem(ext);
        if (!item.kind.empty()) items.push_back(item);
    };
    for (PCWSTR ext : {L".txt", L".pdf", L".png", L".jpg", L".gif", L".zip", L".mp3", L".mp4", L".html", L".json",
                       L".log", L".ini", L".md", L".docx", L".xlsx", L".pptx", L".7z", L".rar", L".svg", L".ico", L".py",
                       L".js", L".css", L".cpp", L".ps1", L".bat", L".cmd", L".reg", L".iso", L".wav", L".flac", L".mkv"}) {
        add(ext);
    }
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts", 0,
                      KEY_READ, &key) == ERROR_SUCCESS) {
        WCHAR name[256];
        for (DWORD i = 0; i < 2000; i++) {
            DWORD length = ARRAYSIZE(name);
            if (RegEnumKeyExW(key, i, name, &length, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
            add(name);
        }
        RegCloseKey(key);
    }
    std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return a.original < b.original; });
}

struct TaskbarProgram {
    std::wstring path, appId;
};

// A Store app's window is a frame of ApplicationFrameHost's, holding the
// app's own window from its own process.
BOOL CALLBACK AppInFrame(HWND child, LPARAM param) {
    auto* found = (std::pair<DWORD, DWORD>*)param;  // Frame's process, app's.
    DWORD pid = 0;
    GetWindowThreadProcessId(child, &pid);
    if (pid && pid != found->first) {
        found->second = pid;
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK TaskbarWindow(HWND window, LPARAM param) {
    auto* paths = (std::vector<TaskbarProgram>*)param;
    if (!IsWindowVisible(window) || !GetWindowTextLengthW(window)) {
        return TRUE;
    }
    LONG_PTR ex = GetWindowLongPtrW(window, GWL_EXSTYLE);
    if ((ex & WS_EX_TOOLWINDOW) || (GetWindow(window, GW_OWNER) && !(ex & WS_EX_APPWINDOW))) {
        return TRUE;
    }
    BOOL cloaked = FALSE;
    api::DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked) {
        return TRUE;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    for (int pass = 0; pass < 2 && pid; pass++) {
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!process) break;
        WCHAR path[MAX_PATH * 2];
        DWORD length = ARRAYSIZE(path);
        bool named = QueryFullProcessImageNameW(process, 0, path, &length);
        std::wstring appId = store::AppIdOf(process);
        CloseHandle(process);
        if (!named) break;
        if (pass == 0 && EndsWithI(path, L"\\applicationframehost.exe")) {
            std::pair<DWORD, DWORD> found{pid, 0};
            EnumChildWindows(window, AppInFrame, (LPARAM)&found);
            if (found.second) {
                pid = found.second;
                continue;
            }
        }
        paths->push_back({path, appId.find(L'!') != std::wstring::npos ? appId : L""});
        break;
    }
    return TRUE;
}

// Windows' app resolver (undocumented, stable since Windows 8): the app id a
// window's button is grouped under, as Tourne Desktop and Explorer ask it,
// and whether the program set that id itself.
struct IApplicationResolver8 : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcut(IShellItem*, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForShortcutObject(IShellLinkW*, IShellItem*, LPWSTR*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForWindow(HWND, LPWSTR*, BOOL*, BOOL*, BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppIDForProcess(DWORD, LPWSTR*, BOOL*, BOOL*, BOOL*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShortcutForProcess(DWORD, IShellItem**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetBestShortcutForAppID(LPCWSTR, IShellItem**) = 0;
};
constexpr CLSID kClsidAppResolver = {0x660b90c8, 0x73a9, 0x4b58, {0x8c, 0xae, 0x35, 0x5b, 0x7f, 0x55, 0x34, 0x1b}};
constexpr IID kIidAppResolver8 = {0xde25675a, 0x72de, 0x44b4, {0x93, 0x73, 0x05, 0x17, 0x04, 0x50, 0xc1, 0x40}};
// System.AppUserModel.ID.
constexpr PROPERTYKEY kAppUserModelId = {{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 5};

struct TaskbarButton {
    std::wstring path, appId, title;  // appId: only one the program set itself.
};

struct ButtonSearch {
    std::vector<TaskbarButton> buttons;
    Com<IApplicationResolver8> resolver;
};

// A window with a taskbar button: its program, and its app id when the
// program chose one (a web app, Discord, a Store app), since that's what tells
// its button apart from the program's others.
BOOL CALLBACK ButtonWindow(HWND window, LPARAM param) {
    auto* search = (ButtonSearch*)param;
    std::vector<TaskbarProgram> one;
    TaskbarWindow(window, (LPARAM)&one);
    if (one.empty()) return TRUE;
    TaskbarButton button{one[0].path, one[0].appId};
    if (button.appId.empty() && search->resolver) {
        LPWSTR id = nullptr;
        BOOL noPin = FALSE, chosen = FALSE, embedded = FALSE;
        if (SUCCEEDED(search->resolver->GetAppIDForWindow(window, &id, &noPin, &chosen, &embedded)) && id) {
            if (chosen) button.appId = id;
            CoTaskMemFree(id);
        }
    }
    if (button.appId.empty()) {
        Com<IPropertyStore> properties;
        PROPVARIANT value;
        PropVariantInit(&value);
        if (SUCCEEDED(SHGetPropertyStoreForWindow(window, __uuidof(IPropertyStore), (void**)&properties)) &&
            SUCCEEDED(properties->GetValue(kAppUserModelId, &value)) && value.vt == VT_LPWSTR && value.pwszVal) {
            button.appId = value.pwszVal;
        }
        PropVariantClear(&value);
    }
    WCHAR title[256] = L"";
    GetWindowTextW(window, title, ARRAYSIZE(title));
    button.title = TrimText(title);
    search->buttons.push_back(std::move(button));
    return TRUE;
}

// A taskbar button, matched as Tourne Desktop matches its Button icons: by
// the app id the program chose, else by its file name.
Item ButtonItem(std::wstring const& path, std::wstring const& appId, std::wstring const& title) {
    Item item;
    item.kind = L"button";
    item.iconFile = path;
    std::wstring file = path.substr(path.find_last_of(L"\\/") + 1);
    std::wstring tail = L" on Tourne Desktop's taskbar, in its tray and in Alt+Tab. File Explorer, Start and Windows' own "
                        L"taskbar keep theirs (On the taskbar changes the program everywhere). (";
    if (!appId.empty()) {
        item.original = Lower(appId);
        item.name = title.empty() ? ProgramName(path) : title;
        size_t bang = appId.find(L'!');
        if (bang != std::wstring::npos) {
            std::wstring folder = store::MainFolderOf(appId.substr(0, bang));
            std::wstring logo = folder.empty() ? L"" : ListLogo(folder, store::Logos(folder, appId.substr(bang + 1)));
            if (!logo.empty()) item.iconFile = logo;
        }
        item.detail = L"Changes the icon of this app's buttons" + tail + L"app id " + appId + L")";
    } else {
        item.original = Lower(file);
        item.name = ProgramName(path);
        item.detail = L"Changes the icon of " + file + L"'s buttons" + tail + path + L")";
    }
    if (kLasting) {
        item.detail += L" Tourne Desktop takes it from here, so it stays " + std::wstring(kStays) + L".";
    }
    return item;
}

// The app id a shortcut gives the program it starts, if it sets one.
std::wstring ShortcutAppId(std::wstring const& path) {
    std::wstring id;
    Com<IShellLinkW> link;
    Com<IPersistFile> file;
    Com<IPropertyStore> properties;
    PROPVARIANT value;
    PropVariantInit(&value);
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&link)) &&
        SUCCEEDED(link->QueryInterface(IID_IPersistFile, (void**)&file)) && SUCCEEDED(file->Load(path.c_str(), STGM_READ)) &&
        SUCCEEDED(link->QueryInterface(__uuidof(IPropertyStore), (void**)&properties)) &&
        SUCCEEDED(properties->GetValue(kAppUserModelId, &value)) && value.vt == VT_LPWSTR && value.pwszVal) {
        id = value.pwszVal;
    }
    PropVariantClear(&value);
    return id;
}

// What's on the taskbar: each open window's button, and the pinned ones.
void GatherButtons(std::vector<Item>& items) {
    ButtonSearch search;
    CoCreateInstance(kClsidAppResolver, nullptr, CLSCTX_INPROC_SERVER, kIidAppResolver8, (void**)&search.resolver);
    EnumWindows(ButtonWindow, (LPARAM)&search);
    for (auto const& button : search.buttons) AddUnique(items, ButtonItem(button.path, button.appId, button.title));
    std::wstring pinned = KnownFolder(FOLDERID_RoamingAppData);
    if (!pinned.empty()) {
        pinned += L"\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar";
        WIN32_FIND_DATAW find;
        HANDLE found = FindFirstFileW((pinned + L"\\*.lnk").c_str(), &find);
        if (found != INVALID_HANDLE_VALUE) {
            do {
                std::wstring path = pinned + L"\\" + find.cFileName;
                ShortcutInfo info = ReadShortcut(path);
                std::wstring appId = info.appId.empty() ? ShortcutAppId(path) : info.appId;
                if (info.target.empty() && appId.empty()) continue;
                Item item = ButtonItem(info.target.empty() ? info.iconFile : info.target, appId, FileTitle(find.cFileName));
                if (!info.iconFile.empty()) {
                    item.iconFile = info.iconFile;
                    item.iconIndex = info.iconIndex;
                }
                AddUnique(items, std::move(item));
            } while (FindNextFileW(found, &find));
            FindClose(found);
        }
    }
    std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
}

// Start's list of apps: the Store apps in it (the others are in its folders).
void GatherStoreApps(std::vector<Item>& items) {
    Com<IShellItem> apps;
    Com<IEnumShellItems> list;
    if (FAILED(SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, nullptr, IID_IShellItem, (void**)&apps)) ||
        FAILED(apps->BindToHandler(nullptr, BHID_EnumItems, IID_IEnumShellItems, (void**)&list))) {
        return;
    }
    std::map<std::wstring, std::wstring> folders;  // Family to folder, as looked up.
    while (true) {
        Com<IShellItem> app;
        if (list->Next(1, &app, nullptr) != S_OK) break;
        PWSTR parsing = nullptr, display = nullptr;
        if (SUCCEEDED(app->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &parsing)) && parsing &&
            wcschr(parsing, L'!') && SUCCEEDED(app->GetDisplayName(SIGDN_NORMALDISPLAY, &display)) && display) {
            std::wstring id = parsing;
            std::wstring family = id.substr(0, id.find(L'!'));
            auto known = folders.find(family);
            std::wstring folder = known != folders.end() ? known->second : (folders[family] = store::MainFolderOf(family));
            if (!folder.empty()) AddUnique(items, StoreItem(folder, id.substr(id.find(L'!') + 1), display));
        }
        CoTaskMemFree(parsing);
        CoTaskMemFree(display);
    }
}

// Settings' Installed apps (and Control Panel's Programs and Features): each
// program's uninstall entry names the icon it shows there (DisplayIcon), often
// not the program itself: its uninstaller, or a copy Windows Installer keeps.
// Entries with none show Windows icon 15 (a program without its own icon).
void GatherInstalled(std::vector<Item>& items) {
    struct Place {
        HKEY root;
        REGSAM view;
    };
    for (Place place : {Place{HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY}, Place{HKEY_LOCAL_MACHINE, KEY_WOW64_32KEY},
                        Place{HKEY_CURRENT_USER, 0}}) {
        HKEY uninstall;
        if (RegOpenKeyExW(place.root, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0,
                          KEY_READ | place.view, &uninstall) != ERROR_SUCCESS) {
            continue;
        }
        WCHAR sub[256];
        for (DWORD i = 0; i < 5000; i++) {
            DWORD subLength = ARRAYSIZE(sub);
            if (RegEnumKeyExW(uninstall, i, sub, &subLength, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
            auto text = [&](PCWSTR value) {
                WCHAR buffer[1024];
                DWORD size = sizeof(buffer);
                return RegGetValueW(uninstall, sub, value, RRF_RT_REG_SZ,  // (REG_EXPAND_SZ comes expanded)
                                    nullptr, buffer, &size) == ERROR_SUCCESS
                           ? TrimText(buffer)
                           : std::wstring();
            };
            DWORD hidden = 0, size = sizeof(hidden);
            RegGetValueW(uninstall, sub, L"SystemComponent", RRF_RT_REG_DWORD, nullptr, &hidden, &size);
            std::wstring name = text(L"DisplayName"), icon = text(L"DisplayIcon");
            if (hidden == 1 || name.empty() || icon.empty() || !text(L"ParentKeyName").empty()) continue;
            // "C:\\App\\app.exe",0 or C:\\App\\app.exe,0 or just a path.
            std::wstring file = icon;
            int index = 0;
            if (icon[0] == L'"') {
                size_t close = icon.find(L'"', 1);
                file = icon.substr(1, close == std::wstring::npos ? std::wstring::npos : close - 1);
                size_t comma = close == std::wstring::npos ? close : icon.find(L',', close);
                if (comma != std::wstring::npos) index = _wtoi(icon.c_str() + comma + 1);
            } else if (std::wstring split; SplitIconLocation(icon, &split, &index)) {
                file = split;
            }
            file = ExpandText(file);
            if (!FileExists(file)) continue;
            Item item = IconItem(file, index, name);
            // An uninstaller or Windows Installer's copy is that entry's own: change only that file,
            // not every program of the same name (unins000.exe, uninstall.exe).
            std::wstring lower = Lower(file);
            if (item.kind == L"program" &&
                (lower.find(L"unins") != std::wstring::npos || lower.find(L"\\windows\\installer\\") != std::wstring::npos ||
                 lower.find(L"\\package cache\\") != std::wstring::npos)) {
                item.original = lower;
                item.detail = L"Changes the icon " + name + L" shows in Installed apps (" + file + L").";
            }
            if (kLasting && !item.kind.empty()) {
                item.kind.clear();
                item.note = std::wstring(L"Installed apps reads this icon from the file itself, so it can only change while ") +
                            kLiveOne + L" runs.";
            }
            AddUnique(items, std::move(item));
        }
        RegCloseKey(uninstall);
    }
    std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
}

// Control Panel's items (its icon views): each names its icon itself, often in
// its own .cpl or .dll rather than imageres.dll.
void GatherControlPanel(std::vector<Item>& items) {
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (FAILED(SHParseDisplayName(L"::{21EC2020-3AEA-1069-A2DD-08002B30309D}", nullptr, &pidl, 0, nullptr)) || !pidl) {
        return;
    }
    Com<IShellFolder> folder;
    Com<IEnumIDList> list;
    if (SUCCEEDED(SHBindToObject(nullptr, pidl, nullptr, IID_IShellFolder, (void**)&folder)) &&
        folder->EnumObjects(nullptr, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &list) == S_OK && list) {
        PITEMID_CHILD child = nullptr;
        while (list->Next(1, &child, nullptr) == S_OK) {
            STRRET shown{};
            WCHAR name[256] = L"";
            if (SUCCEEDED(folder->GetDisplayNameOf(child, SHGDN_NORMAL, &shown))) {
                if (shown.uType == STRRET_WSTR && shown.pOleStr) {
                    wcsncpy_s(name, shown.pOleStr, _TRUNCATE);
                    CoTaskMemFree(shown.pOleStr);
                } else if (shown.uType == STRRET_CSTR) {
                    MultiByteToWideChar(CP_ACP, 0, shown.cStr, -1, name, ARRAYSIZE(name));
                }
            }
            PCUITEMID_CHILD one = child;
            Com<IExtractIconW> extract;
            if (name[0] &&
                SUCCEEDED(folder->GetUIObjectOf(nullptr, 1, &one, IID_IExtractIconW, nullptr, (void**)&extract))) {
                WCHAR file[MAX_PATH * 2] = L"";
                int index = 0;
                UINT flags = 0;
                if (extract->GetIconLocation(GIL_FORSHELL, file, ARRAYSIZE(file), &index, &flags) == S_OK &&
                    !(flags & GIL_NOTFILENAME) && file[0] && file[0] != L'*' && FileExists(ExpandText(file))) {
                    Item item = IconItem(file, index, name);
                    if (item.kind == L"icon") {
                        item.detail = L"Changes " + std::wstring(name) + L"'s icon in Control Panel, and wherever "
                                      L"else Windows shows that icon (" + item.original + L").";
                    }
                    AddUnique(items, std::move(item));
                }
            }
            CoTaskMemFree(child);
        }
    }
    CoTaskMemFree(pidl);
    std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
}

void GatherTray(std::vector<Item>& items) {
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kTrayKey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return;
    }
    WCHAR path[MAX_PATH * 2], tip[256];
    for (DWORD i = 0; i < 1000; i++) {
        DWORD pathLength = ARRAYSIZE(path), tipSize = sizeof(tip), type = 0;
        if (RegEnumValueW(key, i, path, &pathLength, nullptr, &type, (BYTE*)tip, &tipSize) != ERROR_SUCCESS) break;
        if (type != REG_SZ || !FileExists(path)) continue;
        Item item = ProgramItem(path);
        if (!item.kind.empty() && !g_swapTrayIcons) {
            item.detail += L" Its tray icon stays as it is while \"Change programs' tray icons too\" is off.";
        }
        AddUnique(items, item);
    }
    RegCloseKey(key);
    std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
}

void GatherChanges(std::vector<Item>& items) {
    for (auto const& row : ReadList()) {
        Item item;
        item.change = true;
        item.row = row;
        item.kind = row.kind;
        item.original = row.original;
        item.name = row.label.empty() ? row.original : row.label;
        item.iconFile = row.icon;
        item.progId = row.progId;
        if (row.kind == L"program") {
            item.detail = L"A program's icon: " + row.original + L"  →  " + row.icon;
        } else if (row.kind == L"icon") {
            item.detail = L"One icon: " + row.original + L"  →  " + row.icon;
        } else if (row.kind == L"filetype") {
            item.detail = L"Every " + row.original + L" file  →  " + row.icon;
        } else if (row.kind == L"package") {
            item.detail = L"A Store app's logo: " + row.original + L"  →  " + row.icon;
        } else if (row.kind == L"button") {
            item.detail = L"A taskbar button (Tourne Desktop's taskbar, tray and Alt+Tab): " + row.original + L"  →  " + row.icon;
            if (_wcsnicmp(row.icon.c_str(), L"tourne:", 7) == 0) {
                item.detail += L" (one of Tourne Desktop's own designs)";
                item.iconFile.clear();
            }
        } else if (row.kind == L"file") {
            item.detail = L"A whole file: " + row.original + L"  →  " + row.icon;
            item.iconFile = CleanPath(row.original);
            if (item.iconFile.find(L'*') != std::wstring::npos) item.iconFile.clear();
        } else if (row.kind == L"theme") {
            item.name = L"Icon theme: " + (row.label.empty() ? ParseTheme(row.original).name : row.label);
            item.detail = L"An icon theme, taken in from Icon Redirect.";
            item.iconFile.clear();
        }
        items.push_back(item);
    }
}

std::vector<Item> Gather(Source source) {
    std::vector<Item> items;
    switch (source) {
        case kTaskbar: {
            std::vector<TaskbarProgram> programs;
            EnumWindows(TaskbarWindow, (LPARAM)&programs);
            for (auto const& program : programs) AddUnique(items, ProgramItem(program.path, L"", program.appId));
            break;
        }
        case kButtons:
            GatherButtons(items);
            break;
        case kTray:
            GatherTray(items);
            break;
        case kDesktop:
            for (auto const& named : {kNamedIcons[1], kNamedIcons[2], kNamedIcons[3], kNamedIcons[4], kNamedIcons[5]}) {
                AddUnique(items, IconItem(L"%SystemRoot%\\System32\\imageres.dll", -named.id, named.name));
            }
            AddFolderFiles(items, KnownFolder(FOLDERID_Desktop), 3, false);
            AddFolderFiles(items, KnownFolder(FOLDERID_PublicDesktop), 3, false);
            break;
        case kStart:
            AddFolderFiles(items, KnownFolder(FOLDERID_Programs), 0, true);
            AddFolderFiles(items, KnownFolder(FOLDERID_CommonPrograms), 0, true);
            GatherStoreApps(items);
            std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
            break;
        case kInstalled:
            GatherInstalled(items);
            break;
        case kControlPanel:
            GatherControlPanel(items);
            break;
        case kWindows:
            GatherWindowsIcons(items);
            break;
        case kFileTypes:
            GatherFileTypes(items);
            break;
        case kChanges:
            GatherChanges(items);
            break;
        default:
            break;
    }
    return items;
}

// A picture (a Store app's logo) as an icon of one size.
HICON PictureIcon(std::wstring const& path, int size) {
    HICON icon = nullptr;
    Com<IWICImagingFactory> wic;
    std::vector<Pixels> frames;
    Pixels pixels;
    if (SUCCEEDED(CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                   __uuidof(IWICImagingFactory), (void**)&wic)) &&
        DecodeFrames(wic.p, ReadWholeFile(path), &frames) && FrameAt(wic.p, frames, size, &pixels)) {
        std::vector<BYTE> dib = IconDib(pixels);
        icon = CreateIconFromResourceEx(dib.data(), (DWORD)dib.size(), TRUE, 0x00030000, size, size, LR_DEFAULTCOLOR);
    }
    return icon;
}

HICON ItemIcon(Item const& item, int size) {
    HICON icon = nullptr;
    if (!item.iconFile.empty()) {
        if (EndsWithI(item.iconFile, L".ico") && FileExists(item.iconFile)) {
            icon = (HICON)LoadImageW(nullptr, item.iconFile.c_str(), IMAGE_ICON, size, size, LR_LOADFROMFILE);
        } else if (EndsWithI(item.iconFile, L".png")) {
            icon = PictureIcon(item.iconFile, size);
        } else if (FAILED(SHDefExtractIconW(item.iconFile.c_str(), item.iconIndex, 0, &icon, nullptr, size))) {
            icon = nullptr;
        }
    }
    if (!icon && item.kind == L"filetype") {
        SHFILEINFOW info{};
        if (SHGetFileInfoW((L"x" + item.original).c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info),
                           SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON)) {
            icon = info.hIcon;
        }
    }
    return icon;
}

bool Matches(Item const& item, std::wstring const& search) {
    if (search.empty()) return true;
    std::wstring lower = Lower(item.name + L" " + item.original + L" " + item.iconFile);
    return lower.find(Lower(search)) != std::wstring::npos;
}

void ShowItems() {
    WCHAR text[256] = L"";
    GetWindowTextW(g_search, text, ARRAYSIZE(text));
    std::wstring search = TrimText(text);
    SendMessageW(g_items, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(g_items);
    api::ImageList_Remove(g_images, -1);
    g_shown.clear();
    // A file type typed in that isn't listed yet.
    if (g_source == kFileTypes && search.size() > 1 && search[0] == L'.' &&
        search.find_first_of(L" \\/") == std::wstring::npos) {
        bool listed = false;
        for (auto const& item : g_all) listed = listed || _wcsicmp(item.original.c_str(), search.c_str()) == 0;
        if (!listed) g_all.insert(g_all.begin(), FileTypeItem(search));
    }
    int size = Scale(48);
    HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    for (int i = 0; i < (int)g_all.size(); i++) {
        if (!Matches(g_all[i], search)) continue;
        HICON icon = ItemIcon(g_all[i], size);
        int image = icon ? api::ImageList_ReplaceIcon(g_images, -1, icon) : -1;
        if (icon) DestroyIcon(icon);
        LVITEMW item{.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM, .iItem = (int)g_shown.size()};
        std::wstring name = g_all[i].name;
        item.pszText = name.data();
        item.iImage = image;
        item.lParam = i;
        ListView_InsertItem(g_items, &item);
        g_shown.push_back(i);
    }
    SetCursor(cursor);
    SendMessageW(g_items, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(g_items, nullptr, TRUE);
    if (g_shown.empty()) {
        std::wstring noTray = kLasting ? std::wstring(L"Nothing yet: programs show up here once they've added a tray "
                                                      L"icon while ") +
                                             kLiveOne + L" runs. Pick them On the taskbar or In Start instead."
                                       : L"Nothing yet: programs show up here as they add their tray icons while this mod runs.";
        SetStatus(g_source == kTray ? noTray
                  : g_source == kChanges ? L"You haven't changed any icons yet."
                                         : L"Nothing found.");
    } else {
        SetStatus(L"");
    }
}

void LoadSource(Source source) {
    g_source = source;
    HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    g_all = Gather(source);
    SetCursor(cursor);
    ShowItems();
}

void ClearNewIcon() {
    g_newIco.clear();
    for (HICON& icon : g_newIcons) {
        if (icon) DestroyIcon(icon);
        icon = nullptr;
    }
}

void UpdateButtons() {
    bool changeable = g_hasPicked && !g_picked.kind.empty() && g_picked.kind != L"theme" && g_picked.kind != L"file";
    EnableWindow(g_choose, changeable);
    EnableWindow(g_fromProgram, changeable);
    EnableWindow(g_apply, changeable && !g_newIco.empty());
    bool changed = false;
    if (g_hasPicked) {
        for (auto const& row : ReadList()) {
            changed = changed || (row.kind == g_picked.kind && _wcsicmp(row.original.c_str(), g_picked.original.c_str()) == 0);
        }
    }
    EnableWindow(g_putBack, changed);
    InvalidateRect(g_preview, nullptr, TRUE);
}

void Pick(Item const& item) {
    g_picked = item;
    g_hasPicked = true;
    ClearNewIcon();
    if (g_nowIcon) DestroyIcon(g_nowIcon);
    g_nowIcon = ItemIcon(item, Scale(48));
    SetWindowTextW(g_title, item.name.c_str());
    SetWindowTextW(g_detail, (item.note.empty() ? item.detail : item.note).c_str());
    UpdateButtons();
}

// The new icon from a picture or a program's icon, shown before it's saved.
void UseNewIcon(std::wstring const& path, int number) {
    HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    std::vector<BYTE> ico = MakeIco(path, number);
    SetCursor(cursor);
    if (ico.empty()) {
        SetStatus(L"That picture couldn't be read. Try an .ico, .png, .svg, .jpg or .bmp.");
        return;
    }
    ClearNewIcon();
    g_newIco = std::move(ico);
    const int sizes[4] = {16, 24, 32, 48};
    for (int i = 0; i < 4; i++) {
        int size = Scale(sizes[i]);
        // The image of that size, or the nearest larger one.
        WORD count = *(WORD*)&g_newIco[4];
        IcoEntry best{};
        int bestSide = 0;
        for (WORD e = 0; e < count; e++) {
            IcoEntry entry;
            memcpy(&entry, &g_newIco[6 + e * sizeof(IcoEntry)], sizeof(entry));
            int side = entry.width ? entry.width : 256;
            bool better = !bestSide ||
                          (side >= size ? bestSide < size || side < bestSide  // The smallest that's big enough,
                                        : bestSide < size && side > bestSide);  // else the largest.
            if (better) {
                best = entry;
                bestSide = side;
            }
        }
        if (bestSide && (size_t)best.offset + best.bytes <= g_newIco.size()) {
            g_newIcons[i] = CreateIconFromResourceEx(g_newIco.data() + best.offset, best.bytes, TRUE, 0x00030000, size,
                                                     size, LR_DEFAULTCOLOR);
        }
    }
    SetStatus(L"Looks right? Click Apply everywhere.");
    UpdateButtons();
}

void ChoosePicture() {
    WCHAR file[MAX_PATH * 2] = L"";
    OPENFILENAMEW dialog{.lStructSize = sizeof(dialog), .hwndOwner = g_window};
    dialog.lpstrFilter =
        L"Pictures and icons\0*.ico;*.png;*.svg;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff;*.webp\0"
        L"An icon inside a program or library\0*.exe;*.dll;*.icl;*.cpl;*.mun\0All files\0*.*\0";
    dialog.lpstrFile = file;
    dialog.nMaxFile = ARRAYSIZE(file);
    dialog.lpstrTitle = L"Choose the new icon";
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
    if (!api::GetOpenFileNameW(&dialog)) {
        return;
    }
    if (IsModuleFile(file)) {
        int index = 0;
        if (!PickIconDlg(g_window, file, ARRAYSIZE(file), &index)) {
            return;
        }
        UseNewIcon(ExpandText(file), index);
    } else {
        UseNewIcon(file, -1);
    }
}

void IconFromProgram() {
    WCHAR file[MAX_PATH * 2] = L"%SystemRoot%\\System32\\imageres.dll";
    int index = 0;
    if (PickIconDlg(g_window, file, ARRAYSIZE(file), &index)) {
        UseNewIcon(ExpandText(file), index);
    }
}

// Any icon in any file, as the one to change.
void PickFromAnyFile() {
    WCHAR file[MAX_PATH * 2] = L"%SystemRoot%\\System32\\shell32.dll";
    int index = 0;
    if (!PickIconDlg(g_window, file, ARRAYSIZE(file), &index)) {
        return;
    }
    std::wstring path = ExpandText(file);
    Pick(IconItem(path, index, FileTitle(path) + L" icon " + std::to_wstring(index)));
}

void Apply() {
    if (!g_hasPicked || g_newIco.empty() || g_picked.kind.empty()) {
        return;
    }
    std::wstring icon = SaveIco(g_newIco, g_picked.name);
    if (icon.empty()) {
        SetStatus(L"The new icon couldn't be saved in the mod's folder.");
        return;
    }
    ListRow row{g_picked.kind, g_picked.original, icon, g_picked.name, g_picked.progId, L""};
    if (row.kind == L"package") {
        HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
        bool made = store::MakeRow(&row);
        SetCursor(cursor);
        if (!made) {
            SetStatus(L"This app's logo pictures couldn't be found or made.");
            return;
        }
    }
    g_saveError = 0;
    if (!SaveChange(row)) {
        std::wstring why;
        if (row.kind == L"filetype" && !g_saveError) {
            why = L" Windows didn't take the file type's new icon (its setting under your account couldn't be written).";
        } else if (g_saveError) {
            WCHAR* text = nullptr;
            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr, g_saveError, 0, (LPWSTR)&text, 0, nullptr);
            why = L" Windows said: " + TrimText(text ? text : L"") + L" (" + std::to_wstring(g_saveError) +
                  L"). Try Apply again.";
            LocalFree(text);
        }
        SetStatus(L"That couldn't be saved." + why);
        return;
    }
    SetTimer(g_window, kNotifyTimer, 800, nullptr);
    if (row.kind == L"button") {
        SetStatus(L"Done. Tourne Desktop's taskbar shows it in a moment, and its tray and Alt+Tab with it.");
    } else if (!kLasting) {
        SetStatus(L"Done. It shows everywhere now; programs that are already open show it once they're opened again.");
    } else if (row.kind == L"program") {
        std::wstring later = kKit ? L"One made later (or remade by an update) gets it when you next open the Kit."
                                  : L"New ones get it when the mod next starts.";
        SetStatus(g_lastingPlaces ? L"Done: " + std::to_wstring(g_lastingPlaces) + L" shortcuts changed. They keep it " +
                                        kStays + L". " + later
                                  : L"Saved, but no shortcut to it was found (in Start, on the desktop or pinned to "
                                    L"the taskbar). " + later);
    } else if (row.kind == L"icon") {
        SetStatus(g_lastingPlaces ? std::wstring(L"Done. Windows keeps it ") + kStays + L"."
                                  : std::wstring(L"That couldn't be set. Folders', drives' and Windows' other stock icons "
                                                 L"are for every account, so ") +
                                        (kKit ? L"the Kit must be opened as admin (right-click it, Run as administrator)."
                                              : L"Windhawk must run as admin for them."));
    } else {
        SetStatus(std::wstring(L"Done. Windows keeps it ") + kStays + L".");
    }
    ClearNewIcon();
    if (g_nowIcon) DestroyIcon(g_nowIcon);
    g_nowIcon = (HICON)LoadImageW(nullptr, icon.c_str(), IMAGE_ICON, Scale(48), Scale(48), LR_LOADFROMFILE);
    UpdateButtons();
    if (g_source == kChanges) LoadSource(kChanges);
}

void PutBack() {
    if (!g_hasPicked) return;
    ListRow row{g_picked.kind, g_picked.original};
    if (!RemoveChange(row)) {
        SetStatus(L"There was nothing to put back.");
        return;
    }
    SetTimer(g_window, kNotifyTimer, 800, nullptr);
    SetStatus(L"Put back. Programs that are already open show it once they're opened again.");
    ClearNewIcon();
    if (g_source == kChanges) {
        g_hasPicked = false;
        SetWindowTextW(g_title, L"");
        SetWindowTextW(g_detail, L"");
        LoadSource(kChanges);
    }
    UpdateButtons();
}

////////////////////////////////////////////////////////////////////////////////
// Lists and packs
//
// Save lists writes every entry of every list, with its icon as it shows now,
// to a folder in Downloads: a picture per entry and pack.tsv, which says what
// each picture is for. Apply a pack reads a pack.tsv like it (with new
// pictures) and applies them all at once.
//
// pack.tsv: UTF-16 or UTF-8 text, a line per entry, tab-separated:
//   picture  kind  original  name  progid  list  shows-from  detail
// picture is a path from the pack's folder (or a full path); a row with no
// picture, or a kind that can't be changed, is skipped. Lines starting with #
// are notes.

// An icon as straight pixels: drawn on black and on white, the difference
// gives its alpha, so old icons with only a mask come out right too.
bool IconPixels(HICON icon, int size, Pixels* out) {
    BITMAPINFO info{};
    info.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    ReleaseDC(nullptr, screen);
    void* bits[2] = {};
    HBITMAP bitmaps[2] = {CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits[0], nullptr, 0),
                          CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits[1], nullptr, 0)};
    bool ok = bitmaps[0] && bitmaps[1];
    for (int i = 0; ok && i < 2; i++) {
        HGDIOBJ old = SelectObject(dc, bitmaps[i]);
        RECT all{0, 0, size, size};
        FillRect(dc, &all, (HBRUSH)GetStockObject(i ? WHITE_BRUSH : BLACK_BRUSH));
        ok = DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL) != FALSE;
        GdiFlush();
        SelectObject(dc, old);
    }
    if (ok) {
        out->width = out->height = size;
        out->bgra.resize((size_t)size * size * 4);
        auto* black = (const BYTE*)bits[0];
        auto* white = (const BYTE*)bits[1];
        for (size_t i = 0; i < out->bgra.size(); i += 4) {
            int a = 255 - std::clamp(white[i + 1] - black[i + 1], 0, 255);
            for (int c = 0; c < 3; c++) out->bgra[i + c] = (BYTE)std::min<int>(black[i + c], a);  // Premultiplied.
            out->bgra[i + 3] = (BYTE)a;
        }
    }
    for (HBITMAP bitmap : bitmaps) {
        if (bitmap) DeleteObject(bitmap);
    }
    DeleteDC(dc);
    return ok;
}

// A name that's safe as a file name.
std::wstring FileSafe(std::wstring const& name) {
    std::wstring out;
    for (WCHAR c : name) {
        if (c < 32 || wcschr(L"<>:\"/\\|?*", c)) {
            if (!out.empty() && out.back() != L' ') out += L' ';
        } else {
            out += c;
        }
        if (out.size() >= 60) break;
    }
    while (!out.empty() && (out.back() == L' ' || out.back() == L'.')) out.pop_back();
    return out.empty() ? L"icon" : out;
}

// Tabs and line breaks in a field would split it.
std::wstring OneField(std::wstring s) {
    for (WCHAR& c : s) {
        if (c == L'\t' || c == L'\r' || c == L'\n') c = L' ';
    }
    return s;
}

bool WriteUtf16(std::wstring const& path, std::wstring const& text) {
    std::vector<BYTE> data(2 + text.size() * sizeof(WCHAR));
    data[0] = 0xFF;
    data[1] = 0xFE;
    memcpy(data.data() + 2, text.data(), text.size() * sizeof(WCHAR));
    return store::WriteWholeFile(path, data);
}

void SaveLists() {
    std::wstring downloads = KnownFolder(FOLDERID_Downloads);
    if (downloads.empty()) {
        SetStatus(L"Your Downloads folder couldn't be found.");
        return;
    }
    SYSTEMTIME now;
    GetLocalTime(&now);
    WCHAR stamp[32];
    swprintf_s(stamp, L"%04u-%02u-%02u %02u%02u%02u", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute,
               now.wSecond);
    std::wstring folder = downloads + L"\\Tourne'Icons lists " + stamp;
    if (!CreateDirectoryW(folder.c_str(), nullptr)) {
        SetStatus(L"A folder couldn't be made in Downloads.");
        return;
    }
    HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    Com<IWICImagingFactory> wic;
    CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&wic);
    std::wstring tsv =
        L"# Tourne'Icons lists, saved " + std::wstring(stamp) +
        L". To make a pack: draw new pictures, point the first column at them, then Apply a pack.\r\n"
        L"# picture\tkind\toriginal\tname\tprogid\tlist\tshows-from\tdetail\r\n";
    int entries = 0, pictures = 0;
    const int size = 64;
    for (int source = 0; source < kSourceCount; source++) {
        std::vector<Item> items = Gather((Source)source);
        if (items.empty()) continue;
        std::wstring sub = std::to_wstring(source + 1) + L" " + kSourceNames[source];
        CreateDirectoryW((folder + L"\\" + sub).c_str(), nullptr);
        int number = 0;
        for (auto const& item : items) {
            WCHAR prefix[8];
            swprintf_s(prefix, L"%03d ", ++number);
            std::wstring picture = sub + L"\\" + prefix + FileSafe(item.name) + L".png";
            bool saved = false;
            if (HICON icon = ItemIcon(item, size)) {
                Pixels pixels;
                std::vector<BYTE> png;
                saved = wic && IconPixels(icon, size, &pixels) && EncodePng(wic.p, pixels, &png) &&
                        store::WriteWholeFile(folder + L"\\" + picture, png);
                DestroyIcon(icon);
            }
            pictures += saved;
            entries++;
            std::wstring from = item.iconFile.empty() ? L"" : item.iconFile + L"," + std::to_wstring(item.iconIndex);
            tsv += OneField(saved ? picture : L"") + L'\t' + OneField(item.kind) + L'\t' + OneField(item.original) +
                   L'\t' + OneField(item.name) + L'\t' + OneField(item.progId) + L'\t' + kSourceNames[source] + L'\t' +
                   OneField(from) + L'\t' + OneField(item.note.empty() ? item.detail : item.note) + L"\r\n";
        }
    }
    bool written = WriteUtf16(folder + L"\\pack.tsv", tsv);
    SetCursor(cursor);
    if (!written) {
        SetStatus(L"The list couldn't be written to " + folder + L".");
        return;
    }
    SetStatus(L"Saved " + std::to_wstring(entries) + L" entries (" + std::to_wstring(pictures) + L" pictures) to " +
              folder + L".");
    ShellExecuteW(g_window, L"open", folder.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

struct PackRow {
    std::wstring picture;  // "-": put the entry back as it was.
    ListRow row;
    std::wstring from;     // The icon it showed when the lists were saved ("file,number"), if given.
    bool mine = false;     // Saved from Your changes: your own icon, put back as it was.
    bool yours = false;    // Has an icon you set already (worked out when applying).
};

std::vector<PackRow> ReadPack(std::wstring const& path) {
    std::vector<PackRow> rows;
    std::vector<BYTE> data = ReadWholeFile(path);
    std::wstring text;
    if (data.size() >= 2 && data[0] == 0xFF && data[1] == 0xFE) {
        text.assign((const WCHAR*)(data.data() + 2), (data.size() - 2) / sizeof(WCHAR));
    } else {
        size_t skip = data.size() >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF ? 3 : 0;
        text = store::Utf8ToWide(std::string_view((const char*)data.data() + skip, data.size() - skip));
    }
    std::wstring folder = path.substr(0, path.find_last_of(L"\\/"));
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring_view line = std::wstring_view(text).substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? text.size() : end + 1;
        if (!line.empty() && line.back() == L'\r') line.remove_suffix(1);
        if (line.empty() || line[0] == L'#') continue;
        std::vector<std::wstring> f = SplitTabs(line);
        f.resize(7);
        if (f[0].empty() || f[1].empty() || f[2].empty()) continue;
        if (f[1] != L"program" && f[1] != L"icon" && f[1] != L"filetype" && f[1] != L"package" && f[1] != L"button") continue;
        std::wstring picture = f[0];
        for (WCHAR& c : picture) {
            if (c == L'/') c = L'\\';
        }
        bool full = picture.size() > 2 && (picture[1] == L':' || (picture[0] == L'\\' && picture[1] == L'\\'));
        if (picture != L"-" && !full) picture = folder + L"\\" + picture;
        rows.push_back({picture, {f[1], f[2], L"", f[3], f[4], L""}, f[6], f[5] == L"Your changes"});
    }
    return rows;
}

// Which entries already have an icon you set, so a pack leaves them be.
// Yours are: rows in Your changes that no pack made (pack-rows.txt lists
// the ones a pack did), the same in Tourne'Icons (the live one) for the
// lasting builds, Icon Redirect's rows in the suite and the older mod, and
// Tourne Desktop's and the suite's Button icons rows. A row counts when it
// is the same entry, or when it changes the icon the entry shows: the same
// program, or a whole-file stand-in that holds that icon.
std::wstring PackRowsPath(std::wstring const& root) {
    return root.empty() ? root : root + L"\\pack-rows.txt";
}

struct Yours {
    std::vector<ListRow> rows;  // kind, original; a file row's icon is its stand-in.
    std::map<std::wstring, std::set<int>> held;  // A stand-in file's icon ids.

    void AddList(std::wstring const& root) {
        if (root.empty()) return;
        std::vector<ListRow> made = ReadList(PackRowsPath(root));
        for (auto const& row : ReadList(root + L"\\icons.txt")) {
            bool byPack = false;
            for (auto const& m : made) {
                byPack = byPack || (m.kind == row.kind && _wcsicmp(m.original.c_str(), row.original.c_str()) == 0);
            }
            if (!byPack && row.kind != L"theme") rows.push_back(row);
            // A button of a program (not of an app id) holds that program's icon on Tourne Desktop's bar.
            std::wstring pattern = row.kind == L"button" ? ButtonIconPattern(row.original) : L"";
            if (!byPack && !pattern.empty() && pattern.find(L'.') != std::wstring::npos) {
                rows.push_back({L"program", pattern, row.icon});
            }
        }
    }

    void AddEarlier() {
        EarlierSink sink;
        sink.redirect = [this](std::wstring const& original, std::wstring const& redirect) {
            std::wstring file = CleanPath(redirect);
            if (EndsWithI(file, L".ico")) {
                rows.push_back({L"program", Lower(CleanPath(original)), file});
            } else {
                rows.push_back({L"file", CleanPath(original), file});
            }
        };
        sink.buttonIcon = [this](std::wstring const& match, std::wstring const& icon) {
            std::wstring pattern = ButtonIconPattern(match);
            if (!pattern.empty() && !icon.empty()) rows.push_back({L"program", pattern, icon});
        };
        ReadEarlier(sink);
    }

    bool Holds(std::wstring const& standIn, int id) {
        std::wstring key = Lower(standIn);
        auto found = held.find(key);
        if (found == held.end()) {
            std::set<int>& ids = held[key];
            if (HMODULE module = LoadLibraryExW(standIn.c_str(), nullptr,
                                                LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)) {
                std::vector<int> groups;
                EnumResourceNamesW(module, RT_GROUP_ICON, CollectGroups, (LONG_PTR)&groups);
                ids.insert(groups.begin(), groups.end());
                FreeLibrary(module);
            }
            found = held.find(key);
        }
        return found->second.count(id) != 0;
    }

    bool Has(PackRow const& entry) {
        ListRow const& row = entry.row;
        for (auto const& r : rows) {
            if (r.kind == row.kind && _wcsicmp(r.original.c_str(), row.original.c_str()) == 0) return true;
        }
        if (row.kind == L"package") return false;
        // The icon it shows: given by the pack, or the entry itself for a Windows icon.
        std::wstring shows = !entry.from.empty() ? entry.from : row.kind == L"icon" ? row.original : L"";
        std::wstring file;
        int number = 0;
        if (shows.empty() || !SplitIconLocation(shows, &file, &number)) file = shows;
        file = Lower(CleanPath(file));
        std::vector<std::wstring> names;
        if (!file.empty()) names = PathNames(file);
        if (row.kind == L"program") names.push_back(Lower(row.original));
        // An icon that's one of yours already: made by Tourne'Icons (any
        // version's folder, or the Kit's), or one a row of yours uses.
        if (!file.empty() && (file.find(L"\\mod-storage\\local@tourne-icons") != std::wstring::npos ||
                              file.find(L"\\tourne\\icons kit\\") != std::wstring::npos)) {
            return true;
        }
        for (auto const& r : rows) {
            if (!file.empty() && r.kind != L"file" && _wcsicmp(CleanPath(r.icon).c_str(), file.c_str()) == 0) return true;
        }
        for (auto const& r : rows) {
            if (r.kind == L"program" && (row.kind == L"program" || number == 0)) {
                for (auto const& name : names) {
                    if (WildcardMatch(r.original, name)) return true;
                }
            } else if (r.kind == L"file" && number < 0 && !file.empty()) {
                bool matches = false;
                for (auto const& name : names) matches = matches || WildcardMatch(Lower(r.original), name);
                if (matches && Holds(r.icon, -number)) return true;
            }
        }
        return false;
    }
};

Yours FindYours() {
    Yours yours;
    yours.AddList(StorageRoot());
    if (kLasting) {
        // Tourne'Icons' (the live one's) own folder.
        std::wstring data = KnownFolder(FOLDERID_ProgramData);
        if (!data.empty()) yours.AddList(data + L"\\Windhawk\\Engine\\ModsWritable\\mod-storage\\local@tourne-icons");
    }
    yours.AddEarlier();
    return yours;
}

// Whether an entry shows this pack row's picture already (a pack applied
// before, or one of yours saved and put back as it is).
bool ShowsAlready(std::vector<ListRow> const& list, PackRow const& entry) {
    for (auto const& r : list) {
        if (r.kind == entry.row.kind && _wcsicmp(r.original.c_str(), entry.row.original.c_str()) == 0) {
            return _wcsicmp(r.icon.c_str(), entry.picture.c_str()) == 0;
        }
    }
    return false;
}

void ApplyPackRows(std::vector<PackRow> const& found, bool keepYours = true);

void ApplyPack() {
    WCHAR file[MAX_PATH * 2] = L"pack.tsv";
    std::wstring downloads = KnownFolder(FOLDERID_Downloads);
    OPENFILENAMEW dialog{.lStructSize = sizeof(dialog), .hwndOwner = g_window};
    dialog.lpstrFilter = L"Tourne'Icons pack (pack.tsv)\0*.tsv\0All files\0*.*\0";
    dialog.lpstrFile = file;
    dialog.nMaxFile = ARRAYSIZE(file);
    dialog.lpstrInitialDir = downloads.empty() ? nullptr : downloads.c_str();
    dialog.lpstrTitle = L"Open the pack's pack.tsv";
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
    if (!api::GetOpenFileNameW(&dialog)) return;
    std::vector<PackRow> rows = ReadPack(file);
    std::vector<PackRow> found;
    int missing = 0, cantLast = 0, yoursCount = 0;
    HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    Yours yours = FindYours();
    std::vector<ListRow> list = ReadList();
    for (auto& row : rows) {
        if (kLasting && (row.row.kind == L"package" ||
                         (row.row.kind == L"icon" && !lasting::CanLast(row.row.original)))) {
            cantLast++;  // (Store apps' logos, and Windows icons with no setting of their own.)
        } else if (row.picture == L"-" || FileExists(row.picture)) {
            row.yours = !row.mine && row.picture != L"-" && !ShowsAlready(list, row) && yours.Has(row);
            yoursCount += row.yours;
            found.push_back(std::move(row));
        } else {
            missing++;
        }
    }
    SetCursor(cursor);
    if (found.empty()) {
        SetStatus(rows.empty() || cantLast == (int)rows.size()
                      ? L"That pack has no entries that can be changed here."
                      : L"None of that pack's pictures were found beside its pack.tsv.");
        return;
    }
    std::wstring ask = L"Apply " + std::to_wstring(found.size()) + L" icons from this pack?";
    if (missing) ask += L" (" + std::to_wstring(missing) + L" rows have no picture and are skipped.)";
    if (cantLast) {
        ask += L"\n\n" + std::to_wstring(cantLast) + L" more can only change while " + kLiveOne +
               L" runs (Store apps' logos and Windows icons that have no setting of their own), so they're skipped.";
    }
    ask += L"\n\nEach one replaces that entry's icon. Put back original undoes one; Your changes lists them all.";
    if (!yoursCount) {
        if (MessageBoxW(g_window, ask.c_str(), kModTitle, MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;
        ApplyPackRows(found);
        return;
    }
    ask += L"\n\n" + std::to_wstring(yoursCount) +
           L" of them already have an icon you set (in Your changes, the suite's Icon Redirect or Tourne "
           L"Desktop's button icons).\n\nYes: keep yours, change only the rest.\nNo: replace yours too.";
    int answer = MessageBoxW(g_window, ask.c_str(), kModTitle, MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1);
    if (answer == IDCANCEL) return;
    ApplyPackRows(found, answer == IDYES);
}

void ApplyPackRows(std::vector<PackRow> const& found, bool keepYours) {
    HCURSOR cursor = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    int done = 0, places = 0, kept = 0, putBack = 0;
    bool packages = false;
    std::wstring failed;
    std::wstring root = StorageRoot(), ours = root.empty() ? root : root + L"\\icons\\";
    // Which rows a pack made: a later pack may replace those.
    std::vector<ListRow> made = ReadList(PackRowsPath(root)), list = ReadList();
    auto mark = [&made](ListRow const& row, bool byPack) {
        std::erase_if(made, [&](ListRow const& m) {
            return m.kind == row.kind && _wcsicmp(m.original.c_str(), row.original.c_str()) == 0;
        });
        if (byPack) made.push_back({row.kind, row.original});
    };
    // Tourne Desktop's buttons of a program take its new icon too, or theirs
    // would still show on its bar.
    auto follow = [&](ListRow const& row, bool mine) {
        if (row.kind != L"program") return;
        for (auto const& button : list) {
            if (button.kind != L"button" || !ButtonOfProgram(button.original, row.original) ||
                _wcsicmp(button.icon.c_str(), row.icon.c_str()) == 0) {
                continue;
            }
            ListRow moved = button;
            moved.icon = row.icon;
            if (SaveChange(moved, false)) mark(moved, !mine);
        }
    };
    g_holdLasting = kLasting;
    size_t removes = 0;
    for (auto const& entry : found) {
        ListRow row = entry.row;
        if (entry.picture == L"-") {
            removes++;
            if (RemoveChange(row)) putBack++;
            mark(row, false);
            continue;
        }
        if (entry.yours && keepYours) {
            kept++;
            continue;
        }
        if (ShowsAlready(list, entry)) {
            mark(row, !entry.mine);  // Nothing to change; only whose it is.
            row.icon = entry.picture;
            follow(row, entry.mine);
            done++;
            continue;
        }
        // A picture already in the storage folder (one of yours) is used as it is.
        if (!ours.empty() && EndsWithI(entry.picture, L".ico") && _wcsnicmp(entry.picture.c_str(), ours.c_str(), ours.size()) == 0) {
            row.icon = entry.picture;
        } else {
            std::vector<BYTE> ico = MakeIco(entry.picture, -1);
            row.icon = ico.empty() ? L"" : SaveIco(ico, row.label.empty() ? row.original : row.label);
        }
        bool ok = !row.icon.empty() && (row.kind != L"package" || store::MakeRow(&row));
        g_lastingPlaces = 0;
        ok = ok && SaveChange(row, false);
        if (ok) follow(row, entry.mine);
        if (ok) {
            mark(row, !entry.mine);
            done++;
            places += g_lastingPlaces;
            packages = packages || row.kind == L"package";
        } else if (failed.size() < 200) {
            failed += (failed.empty() ? L"" : L", ") + (row.label.empty() ? row.original : row.label);
        }
    }
    if (!root.empty()) WriteList(made, PackRowsPath(root));
    if (kLasting) {
        g_holdLasting = false;
        places = lasting::ApplyAll();
    }
    if (packages) store::RestartStart();
    SetCursor(cursor);
    SetTimer(g_window, kNotifyTimer, 800, nullptr);
    std::wstring status = L"Applied " + std::to_wstring(done) + L" of " + std::to_wstring(found.size() - removes - kept) + L".";
    if (kept) status += L" Kept " + std::to_wstring(kept) + L" icons you'd set.";
    if (putBack) status += L" Put back " + std::to_wstring(putBack) + L".";
    if (kLasting && places) status += L" " + std::to_wstring(places) + L" shortcuts changed.";
    if (!failed.empty()) status += L" Not applied: " + failed + L".";
    SetStatus(status);
    if (g_source == kChanges) LoadSource(kChanges);
    UpdateButtons();
}

void DrawPreview(DRAWITEMSTRUCT const* draw) {
    HDC dc = draw->hDC;
    RECT r = draw->rcItem;
    FillRect(dc, &r, GetSysColorBrush(COLOR_WINDOW));
    int y = r.top + Scale(8), x = r.left + Scale(8);
    if (g_nowIcon) {
        DrawIconEx(dc, x, y, g_nowIcon, Scale(48), Scale(48), 0, nullptr, DI_NORMAL);
    }
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, GetSysColor(COLOR_GRAYTEXT));
    HGDIOBJ old = SelectObject(dc, g_font);
    RECT label{x, y + Scale(52), x + Scale(64), y + Scale(72)};
    if (g_hasPicked) DrawTextW(dc, L"Now", -1, &label, DT_LEFT | DT_SINGLELINE);
    x += Scale(76);
    if (g_newIcons[0] || g_newIcons[3]) {
        RECT arrow{x - Scale(14), y + Scale(14), x + Scale(6), y + Scale(34)};
        DrawTextW(dc, L"→", -1, &arrow, DT_CENTER | DT_SINGLELINE);
        const int sizes[4] = {16, 24, 32, 48};
        int left = x + Scale(10);
        for (int i = 0; i < 4; i++) {
            if (g_newIcons[i]) {
                DrawIconEx(dc, left, y + Scale(48) - Scale(sizes[i]), g_newIcons[i], Scale(sizes[i]), Scale(sizes[i]), 0,
                           nullptr, DI_NORMAL);
            }
            left += Scale(sizes[i]) + Scale(10);
        }
        RECT newLabel{x + Scale(10), y + Scale(52), left, y + Scale(72)};
        DrawTextW(dc, L"New, at 16, 24, 32 and 48", -1, &newLabel, DT_LEFT | DT_SINGLELINE);
    }
    SelectObject(dc, old);
}

DWORD WINAPI ClearCacheThread(LPVOID) {
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    int left = ClearExplorerCaches();
    TellWindowsIconsChanged();
    if (SUCCEEDED(com)) CoUninitialize();
    if (g_window) PostMessageW(g_window, kClearedMessage, (WPARAM)(INT_PTR)left, 0);
    return 0;
}

void ClearCache() {
    if (MessageBoxW(g_window,
                    L"Clear Windows' icon and thumbnail caches?\n\nExplorer closes for a moment (folder windows "
                    L"close too) and starts again, then every icon and thumbnail is drawn again.",
                    kModTitle, MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
        return;
    }
    EnableWindow(g_clearCache, FALSE);
    SetStatus(L"Clearing the icon cache…");
    if (HANDLE thread = CreateThread(nullptr, 0, ClearCacheThread, nullptr, 0, nullptr)) {
        CloseHandle(thread);
    } else {
        EnableWindow(g_clearCache, TRUE);
    }
}

void Layout() {
    RECT client;
    GetClientRect(g_window, &client);
    int w = client.right, h = client.bottom, m = Scale(12), row = Scale(28);
    int left = Scale(170), bottom = Scale(176);
    int sourcesH = h - 2 * m - Scale(24) - 3 * (row + Scale(6));
    MoveWindow(g_sources, m, m, left, sourcesH, TRUE);
    int ly = m + sourcesH + Scale(6);
    for (HWND button : {g_saveLists, g_applyPack, g_clearCache}) {
        MoveWindow(button, m, ly, left, row, TRUE);
        ly += row + Scale(6);
    }
    int x = m + left + m, cw = w - x - m;
    int buttonW = Scale(150);
    MoveWindow(g_search, x, m, cw - 2 * (buttonW + Scale(8)), row, TRUE);
    MoveWindow(g_anyFile, x + cw - 2 * buttonW - Scale(8), m, buttonW, row, TRUE);
    MoveWindow(g_refresh, x + cw - buttonW, m, buttonW, row, TRUE);
    int listTop = m + row + Scale(8);
    MoveWindow(g_items, x, listTop, cw, h - listTop - bottom - Scale(24), TRUE);
    int panel = h - bottom - Scale(16);
    MoveWindow(g_title, x, panel + Scale(8), cw - buttonW - m, Scale(22), TRUE);
    MoveWindow(g_detail, x, panel + Scale(32), cw - buttonW - m, Scale(40), TRUE);
    MoveWindow(g_preview, x, panel + Scale(76), cw - buttonW - m, Scale(88), TRUE);
    int bx = x + cw - buttonW, by = panel + Scale(8);
    for (HWND button : {g_choose, g_fromProgram, g_apply, g_putBack}) {
        MoveWindow(button, bx, by, buttonW, row, TRUE);
        by += row + Scale(6);
    }
    MoveWindow(g_status, m, h - Scale(26), w - 2 * m, Scale(20), TRUE);
}

void MakeFonts() {
    NONCLIENTMETRICSW metrics{.cbSize = sizeof(metrics)};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);
    LOGFONTW font = metrics.lfMessageFont;
    font.lfHeight = -MulDiv(10, g_dpi, 72);
    if (g_font) DeleteObject(g_font);
    if (g_bold) DeleteObject(g_bold);
    g_font = CreateFontIndirectW(&font);
    font.lfWeight = FW_SEMIBOLD;
    font.lfHeight = -MulDiv(12, g_dpi, 72);
    g_bold = CreateFontIndirectW(&font);
}

void ApplyFonts() {
    for (HWND control : {g_sources, g_search, g_items, g_detail, g_status, g_choose, g_fromProgram, g_apply, g_putBack,
                         g_anyFile, g_refresh, g_saveLists, g_applyPack, g_clearCache}) {
        SendMessageW(control, WM_SETFONT, (WPARAM)g_font, TRUE);
    }
    SendMessageW(g_title, WM_SETFONT, (WPARAM)g_bold, TRUE);
}

void MakeImageList() {
    if (g_images) api::ImageList_Destroy(g_images);
    int size = Scale(48);
    g_images = api::ImageList_Create(size, size, ILC_COLOR32, 64, 64);
    ListView_SetImageList(g_items, g_images, LVSIL_NORMAL);
    ListView_SetIconSpacing(g_items, Scale(104), Scale(96));
}

HWND Control(PCWSTR cls, PCWSTR text, DWORD style, int id, DWORD ex = 0) {
    return CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, g_window, (HMENU)(INT_PTR)id,
                           nullptr, nullptr);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            g_window = window;
            g_dpi = GetDpiForWindow(window);
            MakeFonts();
            g_sources = Control(WC_LISTBOXW, L"", WS_BORDER | LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS |
                                                      LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP, kIdSources);
            for (PCWSTR name : kSourceNames) SendMessageW(g_sources, LB_ADDSTRING, 0, (LPARAM)name);
            g_search = Control(WC_EDITW, L"", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, kIdSearch);
            SendMessageW(g_search, EM_SETCUEBANNER, TRUE, (LPARAM)L"Search (or type a file type, like .txt)");
            g_anyFile = Control(WC_BUTTONW, L"Pick from any file…", BS_PUSHBUTTON | WS_TABSTOP, kIdAnyFile);
            g_refresh = Control(WC_BUTTONW, L"Refresh icons", BS_PUSHBUTTON | WS_TABSTOP, kIdRefresh);
            g_items = Control(WC_LISTVIEWW, L"", WS_BORDER | LVS_ICON | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                                                     LVS_AUTOARRANGE | WS_TABSTOP, kIdItems);
            ListView_SetExtendedListViewStyle(g_items, LVS_EX_DOUBLEBUFFER);
            api::SetWindowTheme(g_items, L"Explorer", nullptr);
            g_title = Control(WC_STATICW, L"Pick something to change", SS_LEFT | SS_NOPREFIX | SS_ENDELLIPSIS, kIdTitle);
            g_detail = Control(WC_STATICW, L"Everything on the left is a place it shows. Nothing needs a path.",
                               SS_LEFT | SS_NOPREFIX, kIdDetail);
            g_preview = Control(WC_STATICW, L"", SS_OWNERDRAW, kIdPreview);
            g_choose = Control(WC_BUTTONW, L"Choose new icon…", BS_PUSHBUTTON | WS_TABSTOP, kIdChoose);
            g_fromProgram = Control(WC_BUTTONW, L"Icon from a program…", BS_PUSHBUTTON | WS_TABSTOP, kIdFromProgram);
            g_apply = Control(WC_BUTTONW, L"Apply everywhere", BS_DEFPUSHBUTTON | WS_TABSTOP, kIdApply);
            g_putBack = Control(WC_BUTTONW, L"Put back original", BS_PUSHBUTTON | WS_TABSTOP, kIdPutBack);
            g_status = Control(WC_STATICW, L"", SS_LEFT | SS_NOPREFIX | SS_ENDELLIPSIS, kIdStatus);
            g_saveLists = Control(WC_BUTTONW, L"Save lists", BS_PUSHBUTTON | WS_TABSTOP, kIdSaveLists);
            g_applyPack = Control(WC_BUTTONW, L"Apply a pack…", BS_PUSHBUTTON | WS_TABSTOP, kIdApplyPack);
            g_clearCache = Control(WC_BUTTONW, L"Clear icon cache…", BS_PUSHBUTTON | WS_TABSTOP, kIdClearCache);
            ApplyFonts();
            MakeImageList();
            Layout();
            UpdateButtons();
            SendMessageW(g_sources, LB_SETCURSEL, kTaskbar, 0);
            LoadSource(kTaskbar);
            return 0;
        }
        case WM_SIZE:
            Layout();
            return 0;
        case WM_GETMINMAXINFO: {
            auto* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize = {Scale(760), Scale(520)};
            return 0;
        }
        case WM_DPICHANGED: {
            g_dpi = HIWORD(wParam);
            MakeFonts();
            ApplyFonts();
            MakeImageList();
            auto* r = (RECT*)lParam;
            SetWindowPos(window, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER | SWP_NOACTIVATE);
            ShowItems();
            return 0;
        }
        case WM_CTLCOLORSTATIC:
            if ((HWND)lParam == g_detail || (HWND)lParam == g_status) {
                SetTextColor((HDC)wParam, GetSysColor(COLOR_GRAYTEXT));
                SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
                return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
            }
            SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        case WM_MEASUREITEM:
            if (wParam == kIdSources) {
                ((MEASUREITEMSTRUCT*)lParam)->itemHeight = Scale(30);
                return TRUE;
            }
            break;
        case WM_DRAWITEM:
            if (wParam == kIdPreview) {
                DrawPreview((DRAWITEMSTRUCT*)lParam);
                return TRUE;
            }
            if (wParam == kIdSources) {
                auto* draw = (DRAWITEMSTRUCT*)lParam;
                if (draw->itemID == (UINT)-1) return TRUE;
                bool selected = draw->itemState & ODS_SELECTED;
                FillRect(draw->hDC, &draw->rcItem, GetSysColorBrush(selected ? COLOR_HIGHLIGHT : COLOR_WINDOW));
                SetBkMode(draw->hDC, TRANSPARENT);
                SetTextColor(draw->hDC, GetSysColor(selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
                HGDIOBJ old = SelectObject(draw->hDC, g_font);
                RECT text = draw->rcItem;
                text.left += Scale(10);
                if (draw->itemID < kSourceCount) {
                    DrawTextW(draw->hDC, kSourceNames[draw->itemID], -1, &text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                }
                SelectObject(draw->hDC, old);
                return TRUE;
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case kIdSources:
                    if (HIWORD(wParam) == LBN_SELCHANGE) {
                        int selected = (int)SendMessageW(g_sources, LB_GETCURSEL, 0, 0);
                        if (selected >= 0 && selected < kSourceCount) LoadSource((Source)selected);
                    }
                    return 0;
                case kIdSearch:
                    if (HIWORD(wParam) == EN_CHANGE) ShowItems();
                    return 0;
                case kIdChoose: ChoosePicture(); return 0;
                case kIdFromProgram: IconFromProgram(); return 0;
                case kIdApply: Apply(); return 0;
                case kIdPutBack: PutBack(); return 0;
                case kIdAnyFile: PickFromAnyFile(); return 0;
                case kIdSaveLists: SaveLists(); return 0;
                case kIdApplyPack: ApplyPack(); return 0;
                case kIdRefresh:
                    RefreshIconCache();
                    SetStatus(L"Asked Windows to read its icons again.");
                    return 0;
                case kIdClearCache: ClearCache(); return 0;
            }
            break;
        case WM_NOTIFY: {
            auto* header = (NMHDR*)lParam;
            if (header->idFrom == kIdItems && header->code == LVN_ITEMCHANGED) {
                auto* change = (NMLISTVIEW*)lParam;
                if ((change->uNewState & LVIS_SELECTED) && !(change->uOldState & LVIS_SELECTED)) {
                    int index = (int)change->lParam;
                    if (index >= 0 && index < (int)g_all.size()) Pick(g_all[index]);
                }
            } else if (header->idFrom == kIdItems && header->code == NM_DBLCLK && g_hasPicked && !g_picked.kind.empty()) {
                ChoosePicture();
            }
            break;
        }
        case WM_TIMER:
            if (wParam == kNotifyTimer) {
                KillTimer(window, kNotifyTimer);
                TellWindowsIconsChanged();
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            ClearNewIcon();
            if (g_nowIcon) DestroyIcon(g_nowIcon);
            g_nowIcon = nullptr;
            if (g_images) api::ImageList_Destroy(g_images);
            g_images = nullptr;
            g_window = nullptr;
            g_hasPicked = false;
            g_all.clear();
            if (kKit) PostQuitMessage(0);  // The Kit ends with its window.
            return 0;
        case kClearedMessage:
            EnableWindow(g_clearCache, TRUE);
            SetStatus((int)wParam < 0 ? std::wstring(L"Couldn't find Windows' icon cache.")
                      : wParam    ? L"Cleared most of the icon cache; " + std::to_wstring(wParam) + L" files were still in use."
                                  : std::wstring(L"Cleared Windows' icon and thumbnail caches; every icon is drawn again."));
            return 0;
        case kPutBackMessage:
            if (wParam) {
                SetStatus(L"Put your icon back on " + std::to_wstring(wParam) +
                          (wParam == 1 ? L" shortcut" : L" shortcuts") + L" made or changed since you last opened the Kit.");
            }
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void Open() {
    if (g_window) {
        if (IsIconic(g_window)) ShowWindow(g_window, SW_RESTORE);
        SetForegroundWindow(g_window);
        return;
    }
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc{.cbSize = sizeof(wc), .lpfnWndProc = WindowProc, .hInstance = GetModuleHandleW(nullptr),
                       .hCursor = LoadCursorW(nullptr, IDC_ARROW), .hbrBackground = GetSysColorBrush(COLOR_WINDOW),
                       .lpszClassName = kClass};
        SHSTOCKICONINFO stock{.cbSize = sizeof(stock)};
        if (SUCCEEDED(SHGetStockIconInfo(SIID_IMAGEFILES, SHGSI_ICON | SHGSI_LARGEICON, &stock))) wc.hIcon = stock.hIcon;
        stock = {.cbSize = sizeof(stock)};
        if (SUCCEEDED(SHGetStockIconInfo(SIID_IMAGEFILES, SHGSI_ICON | SHGSI_SMALLICON, &stock))) wc.hIconSm = stock.hIcon;
        registered = RegisterClassExW(&wc) != 0;
    }
    // Sized for the monitor the mouse is on.
    POINT cursor;
    GetCursorPos(&cursor);
    HMONITOR monitor = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info{.cbSize = sizeof(info)};
    GetMonitorInfoW(monitor, &info);
    UINT dpiX = 96, dpiY = 96;
    using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    if (HMODULE shcore = LoadLibraryExW(L"shcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
        if (auto get = (GetDpiForMonitor_t)GetProcAddress(shcore, "GetDpiForMonitor")) get(monitor, 0, &dpiX, &dpiY);
    }
    int w = MulDiv(980, dpiX, 96), h = MulDiv(680, dpiX, 96);
    RECT work = info.rcWork;
    int x = work.left + (work.right - work.left - w) / 2, y = work.top + (work.bottom - work.top - h) / 2;
    HWND window = CreateWindowExW(0, kClass, kModTitle, WS_OVERLAPPEDWINDOW, x, y, w, h,
                                  nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (window) {
        ShowWindow(window, SW_SHOWNORMAL);
        SetForegroundWindow(window);
    }
}

bool IsDialogMessageHere(MSG* message) {
    return g_window && IsDialogMessageW(g_window, message);
}

}  // namespace picker

////////////////////////////////////////////////////////////////////////////////
// The tool process
//
// Windhawk's pattern for a mod with a process of its own: the windhawk.exe UI
// process launches "windhawk.exe -tool-mod <id>", and the mod stops the
// process's own entry point so only the mod runs there. See
// https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process
// Here it holds the tray icon and the picker, makes icons, and downloads
// icon themes.

enum Role : int {
    kRoleNormal,    // Any other program: the hooks.
    kRoleSkip,      // Windhawk's service, another mod's tool process, the lock screen.
    kRoleLauncher,  // A Windhawk process that starts the tool process.
    kRoleTool,      // This mod's own tool process.
};
Role g_role = kRoleNormal;

Role DetermineRole() {
    DWORD session;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &session) && session == 0) {
        return kRoleSkip;
    }
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return kRoleNormal;
    }
    PCWSTR name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    if (_wcsicmp(name, L"LockApp.exe") == 0 || _wcsicmp(name, L"LogonUI.exe") == 0) {
        return kRoleSkip;
    }
    if (_wcsicmp(name, L"windhawk.exe") != 0) {
        return kRoleNormal;
    }
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return kRoleSkip;
    }
    Role role = kRoleLauncher;
    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 || wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            role = kRoleSkip;
            break;
        }
        if (wcscmp(argv[i], L"-tool-mod") == 0 && i + 1 < argc) {
            role = wcscmp(argv[i + 1], WH_MOD_ID) == 0 ? kRoleTool : kRoleSkip;
            break;
        }
    }
    LocalFree(argv);
    return role;
}

void WINAPI EntryPointHook() {
    ExitThread(0);
}

HANDLE g_toolMutex;

// One tool process at a time (see the suite for why it's the handle, not
// mutex ownership, that claims it).
bool ClaimToolProcess() {
    for (int attempt = 0; attempt < 10; attempt++) {
        HANDLE mutex = CreateMutexW(nullptr, FALSE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!mutex) {
            return false;
        }
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            g_toolMutex = mutex;
            return true;
        }
        CloseHandle(mutex);
        Sleep(100);
    }
    return false;
}

void LaunchToolProcess() {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) {
        return;
    }
    std::wstring commandLine = L"\"" + std::wstring(path) + L"\" -tool-mod \"" WH_MOD_ID L"\"";
    HMODULE kernel = GetModuleHandleW(L"kernelbase.dll");
    if (!kernel) {
        kernel = GetModuleHandleW(L"kernel32.dll");
    }
    using CreateProcessInternalW_t = BOOL(WINAPI*)(HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
                                                   WINBOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION,
                                                   PHANDLE);
    auto create = kernel ? (CreateProcessInternalW_t)GetProcAddress(kernel, "CreateProcessInternalW") : nullptr;
    STARTUPINFOW startup{.cb = sizeof(startup), .dwFlags = STARTF_FORCEOFFFEEDBACK};
    PROCESS_INFORMATION process;
    if (create && create(nullptr, path, commandLine.data(), nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS, nullptr,
                         nullptr, &startup, &process, nullptr)) {
        CloseHandle(process.hProcess);
        CloseHandle(process.hThread);
    }
}

// Started only when none is running: after a settings change the old one is
// on its way out and gone within moments.
HANDLE g_launchThread;
HANDLE g_launchStop;

DWORD WINAPI LaunchWhenFreeThread(LPVOID) {
    for (int attempt = 0; attempt < 50; attempt++) {
        HANDLE mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!mutex) {
            LaunchToolProcess();
            return 0;
        }
        CloseHandle(mutex);
        if (WaitForSingleObject(g_launchStop, 100) == WAIT_OBJECT_0) {
            return 0;
        }
    }
    return 0;
}

void LaunchToolWhenFree() {
    g_launchStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_launchThread = g_launchStop ? CreateThread(nullptr, 0, LaunchWhenFreeThread, nullptr, 0, nullptr) : nullptr;
}

void StopLaunching() {
    if (g_launchThread) {
        SetEvent(g_launchStop);
        WaitForSingleObject(g_launchThread, INFINITE);
        CloseHandle(g_launchThread);
        g_launchThread = nullptr;
    }
    if (g_launchStop) {
        CloseHandle(g_launchStop);
        g_launchStop = nullptr;
    }
}

namespace tool {

constexpr WCHAR kClass[] = L"TourneIcons_Tool";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kSettingsMessage = WM_APP + 2;
constexpr UINT kTakenInMessage = WM_APP + 3;
constexpr UINT kTrayId = 1;
constexpr int kMenuOpen = 1, kMenuRefresh = 2;

HWND g_window;
HANDLE g_thread;
DWORD g_threadId;
UINT g_taskbarCreated;
bool g_trayShown;

void ShowTrayIcon(bool show) {
    NOTIFYICONDATAW data{.cbSize = sizeof(data), .hWnd = g_window, .uID = kTrayId};
    if (!show) {
        if (g_trayShown) Shell_NotifyIconW_Original ? Shell_NotifyIconW_Original(NIM_DELETE, &data) : Shell_NotifyIconW(NIM_DELETE, &data);
        g_trayShown = false;
        return;
    }
    data.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    data.uCallbackMessage = kTrayMessage;
    SHSTOCKICONINFO stock{.cbSize = sizeof(stock)};
    if (SUCCEEDED(SHGetStockIconInfo(SIID_IMAGEFILES, SHGSI_ICON | SHGSI_SMALLICON, &stock))) data.hIcon = stock.hIcon;
    wcscpy_s(data.szTip, kModTitle);
    auto notify = Shell_NotifyIconW_Original ? Shell_NotifyIconW_Original : Shell_NotifyIconW;
    g_trayShown = notify(g_trayShown ? NIM_MODIFY : NIM_ADD, &data) || notify(NIM_ADD, &data);
    data.uVersion = NOTIFYICON_VERSION_4;
    notify(NIM_SETVERSION, &data);
    if (stock.hIcon) DestroyIcon(stock.hIcon);
}

void Balloon(std::wstring const& text) {
    if (!g_trayShown) return;
    NOTIFYICONDATAW data{.cbSize = sizeof(data), .hWnd = g_window, .uID = kTrayId, .uFlags = NIF_INFO};
    wcsncpy_s(data.szInfo, text.c_str(), _TRUNCATE);
    wcscpy_s(data.szInfoTitle, kModName);
    data.dwInfoFlags = NIIF_INFO;
    (Shell_NotifyIconW_Original ? Shell_NotifyIconW_Original : Shell_NotifyIconW)(NIM_MODIFY, &data);
}

void ShowMenu() {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kMenuOpen, L"Change icons…");
    AppendMenuW(menu, MF_STRING, kMenuRefresh, L"Refresh icons");
    SetMenuDefaultItem(menu, kMenuOpen, FALSE);
    POINT point;
    GetCursorPos(&point);
    SetForegroundWindow(g_window);
    int chosen = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, g_window, nullptr);
    DestroyMenu(menu);
    if (chosen == kMenuOpen) picker::Open();
    if (chosen == kMenuRefresh) RefreshIconCache();
}

// Done once: a marker file says so.
int TakeInOnce(PCWSTR markerName, int (*takeIn)(), PCSTR what) {
    std::wstring root = StorageRoot();
    std::wstring marker = root + L"\\" + markerName;
    if (root.empty() || FileExists(marker)) return 0;
    int count = takeIn();
    HANDLE file = CreateFileW(marker.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        char text[96];
        int length = snprintf(text, sizeof(text), "%d %s taken in.\r\n", count, what);
        DWORD written;
        WriteFile(file, text, length, &written, nullptr);
        CloseHandle(file);
    }
    return count;
}

DWORD WINAPI TakeInThread(LPVOID) {
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    int count = TakeInOnce(L"taken-in-2.txt", TakeInEarlierChanges, "earlier icon changes");
    count += TakeInOnce(L"taken-in-3.txt", TakeInButtons, "Tourne Desktop button icons");
    if (count && g_window) PostMessageW(g_window, kTakenInMessage, count, 0);
    PublishButtons(ReadList());
    EnumWindows(TellBarIcons, TRUE);
    bool separated = SeparateUnknownFileTypes() > 0;
    if (kLasting) {
        lasting::ApplyAll();
        TellWindowsIconsChanged();
        lasting::StartSettings();
        lasting::Watch();
    } else {
        if (separated) TellWindowsIconsChanged();
        FetchTheme();
    }
    if (SUCCEEDED(com)) CoUninitialize();
    return 0;
}

LRESULT CALLBACK ToolProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == g_taskbarCreated && g_taskbarCreated) {
        g_trayShown = false;
        ShowTrayIcon(Wh_GetIntSetting(L"trayIcon") != 0);
        return 0;
    }
    switch (message) {
        case kTrayMessage:
            switch (LOWORD(lParam)) {
                case NIN_SELECT:
                case NIN_KEYSELECT:
                case WM_LBUTTONUP:
                    picker::Open();
                    return 0;
                case WM_CONTEXTMENU:
                case WM_RBUTTONUP:
                    ShowMenu();
                    return 0;
            }
            return 0;
        case kSettingsMessage:
            ShowTrayIcon(Wh_GetIntSetting(L"trayIcon") != 0);
            return 0;
        case kTakenInMessage:
            Balloon(L"Took in " + std::to_wstring(wParam) +
                    L" icon changes you made before (Tourne Desktop's button icons and Icon Redirect's). They're under "
                    L"Your changes and Taskbar buttons.");
            SetTimer(window, 1, 800, nullptr);
            return 0;
        case WM_TIMER:
            KillTimer(window, wParam);
            TellWindowsIconsChanged();
            return 0;
        case WM_DESTROY:
            ShowTrayIcon(false);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

DWORD WINAPI ToolThread(LPVOID) {
    if (!api::Load()) {
        Wh_Log(L"Missing a Windows function the picker needs");
        return 1;
    }
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX controls{.dwSize = sizeof(controls), .dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
    api::InitCommonControlsEx(&controls);
    g_publishButtons = true;
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    WNDCLASSEXW wc{.cbSize = sizeof(wc), .lpfnWndProc = ToolProc, .hInstance = GetModuleHandleW(nullptr), .lpszClassName = kClass};
    RegisterClassExW(&wc);
    g_window = CreateWindowExW(0, kClass, kModName, 0, 0, 0, 0, 0, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!g_window) {
        return 1;
    }
    // TaskbarCreated reaches us even when Windhawk runs as administrator.
    ChangeWindowMessageFilterEx(g_window, g_taskbarCreated, MSGFLT_ALLOW, nullptr);
    ShowTrayIcon(Wh_GetIntSetting(L"trayIcon") != 0);
    HANDLE takeIn = CreateThread(nullptr, 0, TakeInThread, nullptr, 0, nullptr);
    if (takeIn) CloseHandle(takeIn);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (picker::IsDialogMessageHere(&message)) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    if (SUCCEEDED(com)) CoUninitialize();
    return 0;
}

void Start() {
    g_thread = CreateThread(nullptr, 0, ToolThread, nullptr, 0, &g_threadId);
}

void Stop() {
    if (kLasting) lasting::StopWatching();
    if (g_window) PostMessageW(g_window, WM_CLOSE, 0, 0);
    if (g_thread) {
        WaitForSingleObject(g_thread, 3000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
    if (!kLasting && g_publishButtons) {
        // Tourne Desktop's own Button icons rows again, as it was without this mod.
        UnpublishButtons();
        EnumWindows(TellBarIcons, TRUE);
    }
}

}  // namespace tool

////////////////////////////////////////////////////////////////////////////////
// Lifecycle

void ReadFlags() {
    g_allTypes = Wh_GetIntSetting(L"allResourceRedirect") != 0;
    g_swapTrayIcons = Wh_GetIntSetting(L"swapTrayIcons") != 0;
}

BOOL Wh_ModInit() {
    Wh_Log(L">");

    g_role = DetermineRole();
    if (g_role == kRoleSkip || GetSystemMetrics(SM_CLEANBOOT)) {
        return FALSE;
    }
    WCHAR root[MAX_PATH];
    if (GetSystemWindowsDirectoryW(root, ARRAYSIZE(root))) {
        g_systemRoot = Lower(root);
    }
    WCHAR self[MAX_PATH * 2];
    DWORD selfLength = GetModuleFileNameW(nullptr, self, ARRAYSIZE(self));
    if (selfLength && selfLength < ARRAYSIZE(self)) {
        g_selfPath = Lower(self);
    }
    ReadFlags();

    if (g_role == kRoleTool) {
#ifdef _WIN64
        if (!ClaimToolProcess()) {
            ExitProcess(0);
        }
#else
        ExitProcess(0);
#endif
    }

    if (kLasting) {
        // The picker only; nothing in any other program.
        if (g_role == kRoleTool) {
            auto* dos = (IMAGE_DOS_HEADER*)GetModuleHandleW(nullptr);
            auto* nt = (IMAGE_NT_HEADERS*)((BYTE*)dos + dos->e_lfanew);
            Wh_SetFunctionHook((BYTE*)dos + nt->OptionalHeader.AddressOfEntryPoint, (void*)EntryPointHook, nullptr);
        }
        return TRUE;
    }

    HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
    if (!kernelBase) {
        kernelBase = GetModuleHandleW(L"kernel32.dll");
    }
    auto hook = [kernelBase](PCSTR name, void* hookFunction, void** original) {
        void* target = kernelBase ? (void*)GetProcAddress(kernelBase, name) : nullptr;
        return target && Wh_SetFunctionHook(target, hookFunction, original);
    };
    if (!hook("FindResourceExW", (void*)FindResourceExW_Hook, (void**)&FindResourceExW_Original) ||
        !hook("FindResourceW", (void*)FindResourceW_Hook, (void**)&FindResourceW_Original) ||
        !hook("LoadResource", (void*)LoadResource_Hook, (void**)&LoadResource_Original) ||
        !hook("SizeofResource", (void*)SizeofResource_Hook, (void**)&SizeofResource_Original)) {
        Wh_Log(L"Couldn't hook the resource functions");
        if (g_role == kRoleTool) ExitProcess(1);
        return FALSE;
    }
    WindhawkUtils::SetFunctionHook(PrivateExtractIconsW, PrivateExtractIconsW_Hook, &PrivateExtractIconsW_Original);
    WindhawkUtils::SetFunctionHook(LoadImageW, LoadImageW_Hook, &LoadImageW_Original);
    WindhawkUtils::SetFunctionHook(Shell_NotifyIconW, Shell_NotifyIconW_Hook, &Shell_NotifyIconW_Original);
    if (HMODULE ntdll = GetModuleHandleW(L"ntdll.dll")) {
        void* create = (void*)GetProcAddress(ntdll, "NtCreateFile");
        void* open = (void*)GetProcAddress(ntdll, "NtOpenFile");
        if (create && open) {
            Wh_SetFunctionHook(create, (void*)NtCreateFile_Hook, (void**)&NtCreateFile_Original);
            Wh_SetFunctionHook(open, (void*)NtOpenFile_Hook, (void**)&NtOpenFile_Original);
        }
    }

    WatchList();

    if (g_role == kRoleTool) {
        // Stop windhawk.exe's own main from running; the mod's threads keep
        // the process alive.
        auto* dos = (IMAGE_DOS_HEADER*)GetModuleHandleW(nullptr);
        auto* nt = (IMAGE_NT_HEADERS*)((BYTE*)dos + dos->e_lfanew);
        Wh_SetFunctionHook((BYTE*)dos + nt->OptionalHeader.AddressOfEntryPoint, (void*)EntryPointHook, nullptr);
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    if (g_role == kRoleLauncher) {
        LaunchToolWhenFree();
    } else if (g_role == kRoleTool) {
        tool::Start();
    }
    if (g_role != kRoleTool && !kLasting) {
        StartTrayNudge();
    }
}

void Wh_ModUninit() {
    Wh_Log(L">");
    if (g_role == kRoleTool) {
        tool::Stop();
        StopFetch();
        ExitProcess(0);
    }
    // Tray icons back as the program set them, so turning the mod off (or
    // updating it) doesn't leave them changed.
    StopTrayNudge();
    g_swapTrayIcons = false;
    RefreshTrayIcons();
    StopLaunching();
    StopWatchingList();
    StopFetch();
}

BOOL Wh_ModSettingsChanged(BOOL* reload) {
    Wh_Log(L">");
    bool wasSwapping = g_swapTrayIcons;
    ReadFlags();
    bool changed = RebuildRules();
    if (changed || wasSwapping != g_swapTrayIcons) {
        OnRulesChanged();
    }
    if (g_role == kRoleTool) {
        if (tool::g_window) PostMessageW(tool::g_window, tool::kSettingsMessage, 0, 0);
        if (kLasting) {
            lasting::StartSettings();
            lasting::Watch();
        } else {
            FetchTheme();
        }
        if (changed) TellWindowsIconsChanged();
    }
    *reload = FALSE;
    return TRUE;
}
