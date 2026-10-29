// ==WindhawkMod==
// @id              tourne-icon-redirect
// @name            Tourne'Icons [Icon Redirect]
// @description     Themes icons by loading them from theme files instead of the Windows files they stand in for. No system file is changed
// @version         1.0.1
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @donateUrl       https://ko-fi.com/tourne
// @include         *
// @exclude         vgc.exe
// @compilerOptions -luser32 -lshell32 -lole32
// @license         GPL-3.0
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
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
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
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
  $description: Later rows win over earlier rows and over themes.
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
#include <objbase.h>
#include <wincodec.h>
#include <d2d1_3.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cwctype>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
// whose HRSRC it was given. Append only, and read without a lock.
struct Range {
    std::atomic<uintptr_t> begin;
    uintptr_t end;
    HMODULE module;
    IconFile* icon;
    ImageFolder* folder;
};
constexpr int kMaxRanges = 128;
Range g_ranges[kMaxRanges];
std::atomic<int> g_rangeCount;

bool AddRange(uintptr_t begin, uintptr_t end, HMODULE module, IconFile* icon, ImageFolder* folder = nullptr) {
    int i = g_rangeCount.fetch_add(1);
    if (i >= kMaxRanges) {
        return false;
    }
    g_ranges[i].end = end;
    g_ranges[i].module = module;
    g_ranges[i].icon = icon;
    g_ranges[i].folder = folder;
    g_ranges[i].begin.store(begin, std::memory_order_release);
    return true;
}

Range const* RangeOf(const void* p) {
    int count = std::min(g_rangeCount.load(std::memory_order_acquire), kMaxRanges);
    for (int i = 0; i < count; i++) {
        uintptr_t begin = g_ranges[i].begin.load(std::memory_order_acquire);
        if (begin && (uintptr_t)p >= begin && (uintptr_t)p < g_ranges[i].end) {
            return &g_ranges[i];
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

struct Rule {
    std::wstring pattern;  // Lower case.
    Target* target;
};

// Settings as read, and what they compare by to tell whether a reload is due.
struct RuleSettings {
    std::wstring theme;
    std::vector<std::wstring> themePaths;
    std::vector<std::pair<std::wstring, std::wstring>> redirects;
    std::wstring branding;

    std::wstring Signature() const {
        std::wstring s = theme + L'\n' + branding;
        for (auto const& path : themePaths) {
            s += L'\n' + path;
        }
        s += L"\n\n";
        for (auto const& [original, redirect] : redirects) {
            s += original + L'\t' + redirect + L'\n';
        }
        return s;
    }
};

std::wstring RedirectSetting(PCWSTR name, int index = -1) {
    PCWSTR value = index < 0 ? Wh_GetStringSetting(name) : Wh_GetStringSetting(name, index);
    std::wstring s = value ? value : L"";
    Wh_FreeStringSetting(value);
    return s;
}

// @inline program-icons.cpp

// Rows end at the first run of empty ones, so a blank row in the middle
// doesn't cut the list short.
RuleSettings ReadRuleSettings() {
    RuleSettings settings;
    settings.theme = RedirectSetting(L"iconTheme");
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
    // The taskbar's Button icons marked "Use everywhere", after these so they
    // win: a program's own icon, replaced by the row's .ico wherever it's read
    // from the program's file. A file name matches it in any folder.
    for (auto const& row : EverywhereIcons()) {
        if (!EndsWithI(row.icon, L".ico")) {
            continue;
        }
        bool path = row.program.find_first_of(L"\\/") != std::wstring::npos;
        settings.redirects.push_back({path ? row.program : L"*\\" + row.program, row.icon});
    }
    return settings;
}

std::vector<Rule> g_rules;       // Built once, then read only.
std::vector<Target*> g_targets;  // Never freed; see the top of the file.
std::wstring g_rulesSignature;
std::atomic<bool> g_rulesBuilt;
INIT_ONCE g_rulesOnce = INIT_ONCE_STATIC_INIT;
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

void AddRule(std::wstring const& original, std::wstring const& redirect) {
    std::wstring pattern = CleanPath(original);
    std::wstring file = CleanPath(redirect);
    // A * typed before the replacement (as in the original's box) would make
    // it a relative path that leads nowhere.
    while (!file.empty() && file.front() == L'*') {
        file.erase(0, 1);
    }
    if (pattern.empty() || file.empty()) {
        return;
    }
    g_rules.push_back({Lower(pattern), TargetFor(FullPath(file))});
}

// A theme.ini's [redirections]: original=replacement, relative to its folder.
void AddThemeIni(std::wstring const& ini) {
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
        AddRule(std::wstring(entry.substr(0, equals)), file);
    }
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
    WCHAR storage[MAX_PATH];
    if (!Wh_GetModStoragePath(storage, ARRAYSIZE(storage))) {
        return {};
    }
    return std::wstring(storage) + L"\\icon-themes";
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

// Order sets priority: the theme, then theme paths, then rows, and within
// each the later one. Lookups walk the list from the end.
BOOL CALLBACK BuildRules(PINIT_ONCE, PVOID, PVOID*) {
    RuleSettings settings = ReadRuleSettings();

    ThemeChoice theme = ParseTheme(settings.theme);
    std::wstring root = ThemesRoot();
    if (!theme.name.empty() && !root.empty()) {
        std::wstring ini = ThemeIniIn(root + L"\\" + theme.name);
        if (!ini.empty()) {
            AddThemeIni(ini);
        }
    }

    std::vector<std::wstring> seen;
    for (auto const& setting : settings.themePaths) {
        std::wstring path = FullPath(CleanPath(setting));
        std::wstring ini = EndsWithI(path, L".ini") ? path : path + L"\\theme.ini";
        bool duplicate = false;
        for (auto const& s : seen) {
            duplicate = duplicate || _wcsicmp(s.c_str(), ini.c_str()) == 0;
        }
        if (!duplicate && FileExists(ini)) {
            seen.push_back(ini);
            AddThemeIni(ini);
        }
    }

    // The Windows logo's pictures, after the rows so they win.
    if (!settings.branding.empty()) {
        AddRule(L"*\\branding\\basebrd\\basebrd.dll", settings.branding);
    }
    for (auto const& [original, redirect] : settings.redirects) {
        AddRule(original, redirect);
    }

    g_rulesSignature = settings.Signature();
    g_rulesBuilt = true;
    Wh_Log(L"%zu rules, %zu files", g_rules.size(), g_targets.size());
    return TRUE;
}

bool EnsureRules() {
    InitOnceExecuteOnce(&g_rulesOnce, BuildRules, nullptr, nullptr);
    return !g_rules.empty();
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

struct ModuleInfo {
    Fingerprint fingerprint;
    std::vector<Target*> targets;  // Highest priority first.
    // The program's main icon group, which an .ico file stands in for. When
    // the module lists no icon group at all, every group counts.
    bool hasMainIcon = false;
    WORD mainIconId = 0;
    std::wstring mainIconName;  // Lower case; empty when it's an id.

    bool IsMainIcon(LPCWSTR name) const {
        if (!hasMainIcon) {
            return true;
        }
        if (IS_INTRESOURCE(name)) {
            return mainIconName.empty() && LOWORD(name) == mainIconId;
        }
        if (name[0] == L'#') {
            return mainIconName.empty() && _wtoi(name + 1) == mainIconId;
        }
        return !mainIconName.empty() && _wcsicmp(name, mainIconName.c_str()) == 0;
    }
};

// Set while a hook is at work, so lookups Windows makes on our behalf go
// straight through.
thread_local int t_busy = 0;

BOOL CALLBACK FirstIconGroup(HMODULE, LPCWSTR, LPWSTR name, LONG_PTR param) {
    auto* info = (ModuleInfo*)param;
    if (IS_INTRESOURCE(name)) {
        info->mainIconId = LOWORD(name);
    } else {
        info->mainIconName = Lower(name);
    }
    info->hasMainIcon = true;
    return FALSE;
}

std::shared_ptr<ModuleInfo> BuildModuleInfo(HMODULE module, Fingerprint const& fingerprint) {
    auto info = std::make_shared<ModuleInfo>();
    info->fingerprint = fingerprint;
    std::wstring path = ModulePath(module);
    if (path.empty()) {
        return info;
    }
    std::vector<std::wstring> names = PathNames(Lower(path));
    bool hasIco = false;
    for (size_t i = g_rules.size(); i-- > 0;) {
        Rule const& rule = g_rules[i];
        if (std::find(info->targets.begin(), info->targets.end(), rule.target) != info->targets.end()) {
            continue;
        }
        for (auto const& name : names) {
            if (WildcardMatch(rule.pattern, name)) {
                info->targets.push_back(rule.target);
                hasIco = hasIco || rule.target->icoFile;
                break;
            }
        }
    }
    if (hasIco) {
        EnumResourceNamesW(module, RT_GROUP_ICON, FirstIconGroup, (LONG_PTR)info.get());
    }
    return info;
}

SRWLOCK g_modulesLock = SRWLOCK_INIT;
std::unordered_map<uintptr_t, std::shared_ptr<ModuleInfo>> g_modules;

std::shared_ptr<ModuleInfo> InfoFor(HMODULE module) {
    uintptr_t base = (uintptr_t)module & ~(uintptr_t)3;
    Fingerprint fingerprint;
    if (!ReadFingerprint(base, &fingerprint)) {
        return nullptr;
    }

    std::shared_ptr<ModuleInfo> info;
    AcquireSRWLockShared(&g_modulesLock);
    auto it = g_modules.find(base);
    if (it != g_modules.end() && it->second->fingerprint == fingerprint) {
        info = it->second;
    }
    ReleaseSRWLockShared(&g_modulesLock);
    if (info) {
        return info;
    }

    info = BuildModuleInfo(module, fingerprint);
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

HRSRC IconResource(IconFile* icon, ModuleInfo const& info, LPCWSTR type, LPCWSTR name) {
    if (type == RT_GROUP_ICON) {
        return info.IsMainIcon(name) ? (HRSRC)&icon->entries[0] : nullptr;
    }
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
        return IconResource(icon, info, type, name);
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
    auto info = EnsureRules() ? InfoFor(module) : nullptr;
    if (info && !info->targets.empty()) {
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
            for (Target* target : info->targets) {
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
    if (EnsureRules()) {
        std::vector<std::wstring> names = PathNames(Lower(FullPath(file)));
        for (size_t i = g_rules.size(); i-- > 0 && !found;) {
            for (auto const& name : names) {
                if (WildcardMatch(g_rules[i].pattern, name)) {
                    // The newest matching rule decides, even when it doesn't apply.
                    std::wstring const& path = g_rules[i].target->path;
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

////////////////////////////////////////////////////////////////////////////////
// Packaged apps' logos
//
// A packaged app (MSIX: Claude, the Store's apps) shows its package's logo
// pictures, not its program's icon: the shell and Task Manager open
// Assets\Square44x44Logo.targetsize-24.png and the like with CreateFile (picked
// through the package's resources.pri; tools/icons/package-logo-probe.cpp). So
// a rule for a package's program (*\Claude.exe) stands in for its logos too,
// and a rule can name a logo picture itself. The replacement is drawn as a PNG
// at the logo's size (from its name: targetsize-24, scale-200), kept in the
// storage folder, and that file is opened instead.

using CreateFileW_t = decltype(&CreateFileW);
CreateFileW_t CreateFileW_Original;
using CreateFile2_t = HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD, DWORD, void*);
CreateFile2_t CreateFile2_Original;

bool IsPicturePath(std::wstring_view path) {
    for (PCWSTR type : {L".ico", L".png", L".bmp", L".jpg", L".jpeg", L".svg"}) {
        if (EndsWithI(path, type)) {
            return true;
        }
    }
    return false;
}

// ...\WindowsApps\<package>, lower case; empty when the path isn't in a package.
std::wstring PackageFolderOf(std::wstring const& lower) {
    size_t at = lower.find(L"\\windowsapps\\");
    size_t end = at == std::wstring::npos ? at : lower.find(L'\\', at + 13);
    return end == std::wstring::npos ? L"" : lower.substr(0, end);
}

SRWLOCK g_packagesLock = SRWLOCK_INIT;
std::unordered_map<std::wstring, std::vector<std::wstring>> g_packagePrograms;

// A package's programs (Executable="app\Claude.exe" in its AppxManifest.xml), as full lower-case paths.
std::vector<std::wstring> PackagePrograms(std::wstring const& folder) {
    AcquireSRWLockShared(&g_packagesLock);
    auto it = g_packagePrograms.find(folder);
    bool known = it != g_packagePrograms.end();
    std::vector<std::wstring> programs = known ? it->second : std::vector<std::wstring>{};
    ReleaseSRWLockShared(&g_packagesLock);
    if (known) {
        return programs;
    }
    std::vector<BYTE> manifest = ReadWholeFile(folder + L"\\appxmanifest.xml");
    std::string text(manifest.begin(), manifest.end());
    for (size_t at = 0; (at = text.find("Executable=\"", at)) != std::string::npos;) {
        at += 12;
        size_t end = text.find('"', at);
        if (end == std::string::npos) {
            break;
        }
        std::string value = text.substr(at, end - at);
        int n = MultiByteToWideChar(CP_UTF8, 0, value.data(), (int)value.size(), nullptr, 0);
        std::wstring program(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, value.data(), (int)value.size(), program.data(), n);
        for (WCHAR& c : program) {
            if (c == L'/') c = L'\\';
        }
        programs.push_back(Lower(folder + L"\\" + program));
        at = end;
    }
    AcquireSRWLockExclusive(&g_packagesLock);
    if (g_packagePrograms.size() >= 512) {
        g_packagePrograms.clear();
    }
    g_packagePrograms[folder] = programs;
    ReleaseSRWLockExclusive(&g_packagesLock);
    return programs;
}

// The newest rule naming one of these paths, when its replacement is a picture; else null.
Target* PictureRuleFor(std::wstring const& lowerPath) {
    std::vector<std::wstring> names = PathNames(lowerPath);
    for (size_t i = g_rules.size(); i-- > 0;) {
        for (auto const& name : names) {
            if (WildcardMatch(g_rules[i].pattern, name)) {
                // The newest matching rule decides, even when it doesn't apply.
                return IsPicturePath(g_rules[i].target->path) ? g_rules[i].target : nullptr;
            }
        }
    }
    return nullptr;
}

// What stands in for a logo picture: a rule naming it, else one naming its package's program.
Target* LogoTarget(std::wstring const& lowerPath) {
    if (Target* direct = PictureRuleFor(lowerPath)) {
        return direct;
    }
    std::wstring folder = PackageFolderOf(lowerPath);
    if (folder.empty()) {
        return nullptr;
    }
    for (auto const& program : PackagePrograms(folder)) {
        if (Target* target = PictureRuleFor(program)) {
            return target;
        }
    }
    return nullptr;
}

// The size a logo picture is drawn at, from its name (Square44x44Logo.targetsize-24.png: 24;
// Square150x150Logo.scale-200.png: 300; StoreLogo.png: 50).
UINT LogoSize(std::wstring const& lowerPath) {
    std::wstring name = lowerPath.substr(lowerPath.find_last_of(L'\\') + 1);
    auto numberAfter = [&](PCWSTR key) -> int {
        size_t at = name.find(key);
        return at == std::wstring::npos ? 0 : _wtoi(name.c_str() + at + wcslen(key));
    };
    if (int target = numberAfter(L"targetsize-")) {
        return (UINT)std::clamp(target, 8, 1024);
    }
    int base = 0;
    for (size_t i = 0; i < name.size() && !base; i++) {
        if (iswdigit(name[i]) && (i == 0 || !iswdigit(name[i - 1]))) {
            size_t x = name.find(L'x', i);
            if (x != std::wstring::npos && x + 1 < name.size() && iswdigit(name[x + 1])) {
                base = _wtoi(name.c_str() + x + 1);  // (the height: Wide310x150Logo is 150 tall)
            }
        }
    }
    if (!base) {
        base = name.find(L"storelogo") != std::wstring::npos ? 50 : 256;
    }
    int scale = numberAfter(L"scale-");
    return (UINT)std::clamp(scale ? base * scale / 100 : base, 8, 1024);
}

// A picture at size x size (fitted, centred): an .svg drawn at it, else the
// file's nearest frame (an .ico's smallest one at least that big) scaled to it.
bool PictureAtSize(IWICImagingFactory* wic, std::wstring const& path, std::vector<BYTE> const& file, UINT size, Pixels* out) {
    if (EndsWithI(path, L".svg")) {
        return RenderSvg(wic, file, size, size, out);
    }
    Com<IStream> stream;
    stream.p = StreamOn(file);
    Com<IWICBitmapDecoder> decoder;
    UINT frames = 0;
    if (!stream || FAILED(wic->CreateDecoderFromStream(stream.p, nullptr, WICDecodeMetadataCacheOnDemand, &decoder)) ||
        FAILED(decoder->GetFrameCount(&frames)) || !frames) {
        return false;
    }
    UINT best = 0, bestSide = 0;
    for (UINT i = 0; i < frames; i++) {
        Com<IWICBitmapFrameDecode> frame;
        UINT w = 0, h = 0;
        if (FAILED(decoder->GetFrame(i, &frame)) || FAILED(frame->GetSize(&w, &h))) {
            continue;
        }
        UINT side = std::max(w, h);
        bool better = !bestSide || (side >= size ? bestSide < size || side < bestSide : bestSide < size && side > bestSide);
        if (better) {
            best = i;
            bestSide = side;
        }
    }
    Com<IWICBitmapFrameDecode> frame;
    Com<IWICFormatConverter> converter;
    Com<IWICBitmapScaler> scaler;
    UINT w = 0, h = 0;
    if (FAILED(decoder->GetFrame(best, &frame)) || FAILED(frame->GetSize(&w, &h)) || !w || !h ||
        FAILED(wic->CreateFormatConverter(&converter)) ||
        FAILED(converter->Initialize(frame.p, kWicFormatPbgra, WICBitmapDitherTypeNone, nullptr, 0,
                                     WICBitmapPaletteTypeCustom))) {
        return false;
    }
    UINT fitW = std::max(1u, w >= h ? size : w * size / h), fitH = std::max(1u, h >= w ? size : h * size / w);
    if (FAILED(wic->CreateBitmapScaler(&scaler)) ||
        FAILED(scaler->Initialize(converter.p, fitW, fitH, WICBitmapInterpolationModeFant))) {
        return false;
    }
    std::vector<BYTE> fitted((size_t)fitW * fitH * 4);
    if (FAILED(scaler->CopyPixels(nullptr, fitW * 4, (UINT)fitted.size(), fitted.data()))) {
        return false;
    }
    out->width = out->height = size;
    out->bgra.assign((size_t)size * size * 4, 0);
    UINT left = (size - fitW) / 2, top = (size - fitH) / 2;
    for (UINT y = 0; y < fitH; y++) {
        memcpy(&out->bgra[((size_t)(top + y) * size + left) * 4], &fitted[(size_t)y * fitW * 4], (size_t)fitW * 4);
    }
    return true;
}

// The replacement drawn at size, as a PNG file kept in the storage folder (made once); empty when it can't be.
std::wstring LogoFile(Target* target, UINT size) {
    WCHAR storage[MAX_PATH];
    WIN32_FILE_ATTRIBUTE_DATA source{};
    if (!Wh_GetModStoragePath(storage, ARRAYSIZE(storage)) ||
        !GetFileAttributesExW(target->path.c_str(), GetFileExInfoStandard, &source)) {
        return L"";
    }
    // Named by the replacement and its time, so a changed picture is made again.
    uint64_t hash = 1469598103934665603ull;
    for (WCHAR c : Lower(target->path)) {
        hash = (hash ^ c) * 1099511628211ull;
    }
    hash ^= ((uint64_t)source.ftLastWriteTime.dwHighDateTime << 32) | source.ftLastWriteTime.dwLowDateTime;
    WCHAR leaf[64];
    swprintf(leaf, ARRAYSIZE(leaf), L"%016llx-%u.png", (unsigned long long)hash, size);
    std::wstring folder = std::wstring(storage) + L"\\package-logos";
    std::wstring file = folder + L"\\" + leaf;
    if (FileExists(file)) {
        return file;
    }
    if (LoaderLockHeld()) {
        return L"";
    }
    std::vector<BYTE> png;
    HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    {
        Com<IWICImagingFactory> wic;
        Pixels pixels;
        std::vector<BYTE> bytes = ReadWholeFile(target->path);
        if (!bytes.empty() &&
            SUCCEEDED(CoCreateInstance(kClsidWicImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory),
                                       (void**)&wic)) &&
            PictureAtSize(wic.p, target->path, bytes, size, &pixels)) {
            EncodePng(wic.p, pixels, &png);
        }
    }
    if (SUCCEEDED(com)) {
        CoUninitialize();
    }
    if (png.empty()) {
        Wh_Log(L"Couldn't draw %s at %u for a package logo", target->path.c_str(), size);
        return L"";
    }
    CreateDirectoryW(folder.c_str(), nullptr);
    // Written beside it and moved in place, so another program reading it never sees half a file.
    std::wstring partial = file + L"." + std::to_wstring(GetCurrentProcessId());
    HANDLE out = CreateFileW(partial.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);  // (t_busy: not ours)
    if (out == INVALID_HANDLE_VALUE) {
        return L"";
    }
    DWORD written = 0;
    bool ok = WriteFile(out, png.data(), (DWORD)png.size(), &written, nullptr) && written == png.size();
    CloseHandle(out);
    if (!ok || (!MoveFileExW(partial.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING) && !FileExists(file))) {
        DeleteFileW(partial.c_str());
        return L"";
    }
    return file;
}

SRWLOCK g_logosLock = SRWLOCK_INIT;
std::unordered_map<std::wstring, std::wstring> g_logos;  // a logo's lower-case path: what's opened instead ("" none)

// A read of a picture: the logo's stand-in to open instead, when a rule has one.
bool LogoRedirect(LPCWSTR name, DWORD access, DWORD disposition, std::wstring* replacement) {
    constexpr DWORD kWrites = GENERIC_WRITE | GENERIC_ALL | FILE_WRITE_DATA | FILE_APPEND_DATA | DELETE | WRITE_DAC | WRITE_OWNER;
    if (t_busy || !name || disposition != OPEN_EXISTING || (access & kWrites)) {
        return false;
    }
    size_t length = wcslen(name);  // (most opens end here: only pictures go on)
    if (length < 5 || (_wcsicmp(name + length - 4, L".png") != 0 && _wcsicmp(name + length - 4, L".jpg") != 0)) {
        return false;
    }
    t_busy++;
    std::wstring lower = Lower(FullPath(name));
    AcquireSRWLockShared(&g_logosLock);
    auto it = g_logos.find(lower);
    bool known = it != g_logos.end();
    if (known) {
        *replacement = it->second;
    }
    ReleaseSRWLockShared(&g_logosLock);
    if (!known) {
        Target* target = EnsureRules() ? LogoTarget(lower) : nullptr;
        *replacement = target ? LogoFile(target, LogoSize(lower)) : L"";
        if (!target || !replacement->empty()) {  // (one that couldn't be drawn yet is tried again)
            AcquireSRWLockExclusive(&g_logosLock);
            if (g_logos.size() >= 4096) {
                g_logos.clear();
            }
            g_logos[lower] = *replacement;
            ReleaseSRWLockExclusive(&g_logosLock);
        }
    }
    t_busy--;
    return !replacement->empty();
}

HANDLE WINAPI CreateFileW_Hook(LPCWSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES security, DWORD disposition,
                               DWORD flags, HANDLE templateFile) {
    std::wstring replacement;
    if (LogoRedirect(name, access, disposition, &replacement)) {
        HANDLE file = CreateFileW_Original(replacement.c_str(), access, share, security, disposition, flags, templateFile);
        if (file != INVALID_HANDLE_VALUE) {
            return file;
        }
    }
    return CreateFileW_Original(name, access, share, security, disposition, flags, templateFile);
}

HANDLE WINAPI CreateFile2_Hook(LPCWSTR name, DWORD access, DWORD share, DWORD disposition, void* parameters) {
    std::wstring replacement;
    if (LogoRedirect(name, access, disposition, &replacement)) {
        HANDLE file = CreateFile2_Original(replacement.c_str(), access, share, disposition, parameters);
        if (file != INVALID_HANDLE_VALUE) {
            return file;
        }
    }
    return CreateFile2_Original(name, access, share, disposition, parameters);
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
    HANDLE mutex = CreateMutexW(nullptr, FALSE, L"TourneIconRedirect_ThemeFetch");
    std::wstring failed;
    while (!g_fetchStop) {
        ThemeChoice theme = ParseTheme(RedirectSetting(L"iconTheme"));
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
        if (ParseTheme(RedirectSetting(L"iconTheme")).name == theme.name) {
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
    ThemeChoice theme = ParseTheme(RedirectSetting(L"iconTheme"));
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
// Lifecycle

// Whether there's anything to redirect, without reading every row.
bool ShouldLoadInThisProcess() {
    return !RedirectSetting(L"iconTheme").empty() || !RedirectSetting(L"brandingImages").empty() ||
           !RedirectSetting(L"themePaths[%d]", 0).empty() ||
           !RedirectSetting(L"themePaths[%d]", 1).empty() ||
           !RedirectSetting(L"redirectionResourcePaths[%d].original", 0).empty() ||
           !RedirectSetting(L"redirectionResourcePaths[%d].original", 1).empty();
}

// Changed rules need a reload. The rules are read on first use, so a process
// that hasn't used them yet will read the new ones anyway.
bool NeedsReload() {
    g_allTypes = Wh_GetIntSetting(L"allResourceRedirect") != 0;
    return g_rulesBuilt && ReadRuleSettings().Signature() != g_rulesSignature;
}

BOOL Wh_ModInit() {
    Wh_Log(L">");

    WCHAR root[MAX_PATH];
    if (GetSystemWindowsDirectoryW(root, ARRAYSIZE(root))) {
        g_systemRoot = Lower(root);
    }
    g_allTypes = Wh_GetIntSetting(L"allResourceRedirect") != 0;

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
        return FALSE;
    }
    // Packaged apps' logos (opened as files): without these, only they stay as they were.
    if (!hook("CreateFileW", (void*)CreateFileW_Hook, (void**)&CreateFileW_Original) ||
        !hook("CreateFile2", (void*)CreateFile2_Hook, (void**)&CreateFile2_Original)) {
        Wh_Log(L"Couldn't hook the file functions: packaged apps' logos stay as they are");
    }
    WindhawkUtils::SetFunctionHook(PrivateExtractIconsW, PrivateExtractIconsW_Hook,
                                   &PrivateExtractIconsW_Original);
    WindhawkUtils::SetFunctionHook(LoadImageW, LoadImageW_Hook, &LoadImageW_Original);
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L">");
    StopFetch();
}

// The suite calls NeedsReload itself rather than this.
BOOL Wh_ModSettingsChanged(BOOL* reload) {
    Wh_Log(L">");
    *reload = NeedsReload();
    return TRUE;
}
