# Tourne Desktop

The suite's parts that need no hooks (the taskbar, the switcher with tiling
and borders, and the system icons with their flyouts), run as one small
program of their own instead of inside Windhawk. Windhawk stays for the parts that must hook Explorer (Start, the
tray icons, the clock).

Status: runs on Windows as the shell on my PC since 2026-10-05 (shell mode); the newest state is at the bottom of DEVLOG.md.

## Build

    .\desktop\Build-TourneDesktop.ps1                    # every module
    .\desktop\Build-TourneDesktop.ps1 -Modules switcher  # only some
    .\desktop\Build-TourneDesktop.ps1 -Debug             # with symbols

It uses Windhawk's own compiler when Windhawk is installed, otherwise clang++
or g++ (MinGW) on the PATH, or `-Compiler <path>`. The result is
`desktop\build\TourneDesktop.exe`. The system icons need the C++/WinRT
headers, which Windhawk's compiler has; with a compiler that lacks them, leave
that module out (`-Modules taskbar, switcher`).

Each module is the suite's own source, unchanged. The build puts in its
`// @inline` files, wraps it in a namespace of its own, and answers Windhawk's
API from the host (`host\module-shim.inl`), including the two Windhawk helpers
the system icons use (string settings, and subclassing a window of another
thread in the same program). A module that hooks other programs is refused:
it belongs in the Windhawk suite.

## Run

    TourneDesktop.exe              # start (a tray icon shows its menu)
    TourneDesktop.exe --studio     # open Tourne Studio (starts it if needed)
    TourneDesktop.exe --import     # take the settings of the installed suite
    TourneDesktop.exe --install    # start with Windows
    TourneDesktop.exe --uninstall  # stop starting with Windows
    TourneDesktop.exe --stop       # close a running copy

A module whose part is turned on in the Windhawk suite is skipped, so the two
never draw the same bar twice.

If Tourne Desktop crashes, it writes `%LOCALAPPDATA%\Tourne\Desktop\crash.txt`
(send that file in). If it crashed while a part was starting, the next start
leaves that part off and says so; saving any setting tries it again.

## Settings

Your settings live in `%APPDATA%\Tourne\Desktop\tourne.conf`, written the way
Hyprland writes its config:

    taskbar {
        autoHide = true
        previewThumbHeight = 160
    }
    switcher.borders.enabled = true
    switcher.borders.width = 2

Saving the file applies it at once; only the modules whose settings changed
are told. `defaults.conf` next to it lists every setting with its default,
taken from each module's own settings schema. Lines it can't read are logged
and skipped. Set `desktop.log = 1` to keep a log file.

## Tourne Studio

Every setting in one window: the tray menu's "Open Tourne Studio", or
`TourneDesktop.exe --studio`. A page per part, plus the theme, app styles and
Tourne Desktop itself, with each setting's name, description, choices and
limits taken from the part's own schema, and a search over all of them.

A change shows at once but is kept only when you save (Ctrl+S); until then
you can undo, redo or discard it. Saving writes `tourne-gui.conf` next to
`tourne.conf`. Your `tourne.conf` is never written; `tourne-gui.conf` is read
after it, so for the same setting the Studio's value wins. Each row says
where its value comes from, and Reset takes it back out of the Studio's file.

Any colour can be one of the theme's roles, followed as the theme changes:

    taskbar.desktopsLook.markColor = @accent
    taskbar.desktopsLook.panelColor = @ground

The roles are `@ground`, `@raised`, `@line`, `@edge`, `@text`, `@muted`,
`@dim`, `@accent`, `@accent2`, `@good`, `@warn` and `@bad`. With no theme
made yet, the setting's default is used.

## Presets

The Studio opens on Presets: 824 palettes and looks from the ricing world
(base16, Catppuccin, Tokyo Night, Rosé Pine, Omarchy, HyDE, YASB, HyprPanel and
more). Choose one and it becomes the Tourne theme, the taskbar and switcher
follow it, and a full look also sets its gaps, borders and corners. In a file:

    theme {
        source = preset
        preset = pick-tokyo-night
    }

The ids are in `desktop/presets/catalogue.json`.

## Glass

Liquid glass over your wallpaper, off until you turn it on (`glass.enabled`).
The Studio's Glass page has every setting, a test window to drag about and
"Hold to compare".

    glass {
        enabled = true
        look = liquid        # clear, liquid, frosted, aero, smoke, prism, oil
        blur = 14            # anything you set wins over the look
        tint = @accent       # theme roles work here too
        studio = true        # the Studio wears it
        rims {
            on = true        # a glass rim round the window in front
            width = 6
            radius = 8
        }
        app.terminal.on = true
        app.terminal.backdrop = acrylic   # mica, acrylic, tabbed (Windows 11) or frosted
        app.terminal.opacity = 100        # 30 to 100
    }

The glass shows the wallpaper behind it, not the windows in between (Windows
gives no cheap way to see another program's pixels). It draws nothing while
nothing changes. App glass is put back as it was when you turn it off or close
Tourne Desktop. To see the looks on a picture without starting anything:

    TourneDesktop.exe --glass-preview looks.png --wallpaper picture.jpg --look all

## Tourne theme: colours from your wallpaper

    theme {
        source = wallpaper   # off, wallpaper or seed
        profile = adaptive   # monochrome, muted, vivid, pastel
        mode = auto          # dark, light
        seed = #64AA89       # for source = seed
        contrast = 4.5       # text on grounds (WCAG); 7 for AAA
    }

With a source set, Tourne Desktop makes a palette whenever the wallpaper (or
the seed) changes and keeps it in `HKCU\Software\Tourne\Theme`. A piece
retints with it when its colour scheme is "Tourne theme": in the Windhawk
suite, System Icons' Colour scheme (the taskbar follows it by default, the
Switcher with its Style set to the colour scheme); here, for example,
`taskbar.scheme = tourneTheme`.

## Identity: shape, material and motion for every piece

    identity {
        shape = rounded     # inherit, square, subtle, rounded, pill
        frame = 2           # border weight in px (0 leaves it)
        material = acrylic  # inherit, none, blur, acrylic, mica
        opacity = 85        # % solid (0 leaves it)
        motion = smooth     # inherit, off, snap, smooth, lively
        speed = 100         # % (200 is twice as fast)
    }

The Tourne theme is the desktop's colours; the identity is the rest of its
look, said once. The switcher's corners and borders, glass, and the
taskbar's previews, blur and hiding take their values from it. A piece's own
setting still wins when you set it yourself, and a glass look's values give
way to the identity's. Everything defaults to `inherit`, which changes
nothing. With `desktop.log = true` the log lists what the identity set.

The Presets page's **Identities** are whole looks in one click: Wise Wolf
(and Wise Wolf Snow, its light side) makes a monochrome palette from one
silver seed with subtle corners, hairline frames, see-through acrylic and
slow, smooth motion; Everforest keeps its fixed palette with rounded shapes
and Mica; Ghost Shell is CRT cyan on navy-black, square, solid and snappy.
Undo or Discard puts back what you had.

## Motion: Hyprland's animations

    animations {
        enabled = true
        bezier = myBezier, 0.05, 0.9, 0.1, 1.05
        spring = soft, 1, 90, 12          # mass, stiffness, damping
        animation = windows, 1, 7, myBezier
        animation = border, 1, 10, default
        animation = workspaces, 1, 6, soft, slide
    }

How things move, written as Hyprland writes it: a Hyprland `animations`
block pastes in as it is (the newer `hl.animation({...})` and `hl.curve(...)`
lines too). Each line is on/off, speed in tenths of a second, a curve and a
style. A part with no line takes its parent's, as in Hyprland's tree
(`windowsIn` under `windows` under `global`); Tourne adds `bar`, `start`,
`popups` and `switcher` under `layers`. Curves: `default`, `linear`, the CSS
set (`ease`, `ease-in-out` ...), `overshoot`, the easings.net set
(`easeOutQuint`, `easeInOutBack` ...) and the springs `rubber`, `gentle`,
`snappy`, `bouncy` and `wobbly`, plus your own. Nothing set: each part keeps
its own animation.

Today the bar's hiding (`bar`, with `slide`, `fade` or `slidefade`), CSS bar
transitions named by curve, and window borders (`border`, `borderangle`)
follow it; more parts join as they gain animations. The Identity's motion
presets write these lines too.

The Studio's **Motion** page draws the curve of the line you point at, large,
with a dot running along it and a block moved by it in its style; each line
has its own small curve playing beside it. The previews run only while the
page is open.

## Switcher views: Flip 3D and Cover Flow

    switcher.view.style = flip3d   # list, flip3d, coverflow
    switcher.view.live = true      # windows keep moving in the view
    switcher.view.dim = 55         # % darker behind
    switcher.view.blur = 6         # 0 to 10

Alt+Tab can show your windows in 3D instead of the list. **Flip 3D** stacks
them on a slant with the chosen one in front; Tab sends the front one flying
past you to the back. **Cover Flow** lines them up with the chosen one facing
you and the rest turned to the sides, standing on a reflecting floor. Both
open by flying your windows from where they are into place, over a dimmed,
blurred copy of the screen, and move as the motion settings' `switcher` line
says. Tab, the arrows and the wheel move through them; a click switches (in
Cover Flow a side window comes to the middle first), a middle click closes,
and a click on the background goes back. The pictures are live through
Windows' own screen capture (Windows 10 1903 and later), else stills taken as
the view opens; minimised windows show their icon on a card. The list stays
the default.

## Shell mode: Tourne Desktop in place of Explorer

    shell {
        enabled = true       # from the next sign-in
        startupApps = true   # the apps Task Manager's Startup page has on
        desktop = true       # wallpaper and desktop icons
    }

Windows starts the shell named in `HKCU\...\Winlogon\Shell` at sign-in; with
shell mode on, that's Tourne Desktop (a copy kept in
`%LOCALAPPDATA%\Tourne\Desktop\bin`), so Explorer's taskbar, Start and
desktop never start. File Explorer windows still open as usual.

- `--shell` is a small watchdog. Quit Tourne Desktop and Explorer starts for
  the rest of the session; if it crashes it starts again once, and a second
  crash soon after brings Explorer back and keeps shell mode off until you
  save a setting.
- Hold **Shift** while signing in for Explorer, that time only.
- `--restore`, or the switch off, makes Explorer the shell again.
- `shell.log` in `%LOCALAPPDATA%\Tourne\Desktop` says what happened at each
  sign-in.

- Apps' tray icons show in Tourne's bar: Tourne Desktop hosts the tray
  itself, so the bar's places, order and drag and drop work as before.

- App bars (Tourne's bar, other docks and bars) get their edges and the
  work area as with Explorer, so maximised windows stop at them, and they
  hear when a game or video fills the screen.

- The desktop: your wallpaper and desktop icons, with their menus, from
  Windows' own folder view (`shell.desktop`, on).
- Quick toggles: Windows' own network, volume and battery icons are
  Explorer's, so in shell mode turn on the system icons
  (`systemIcons.enabled`): network, volume, Bluetooth, battery, brightness,
  night light and the rest, each with its flyout, in Tourne's tray.

Not in shell mode: balloon tips (logged for now), Win+A and Win+N
(Explorer's own), and the system icons' right-click menus that open the
original Windows icon's menu (there's no original to open).

## Windows' taskbar off (the killswitch)

    killswitch {
        enabled = true   # Windows' taskbar and Start kept away
        keys = true      # Ctrl+Esc, Win+T, Win+B, Win+1..0, a lone Win key
        edge = true      # an invisible sliver over the edge it waits at
    }

Without anything of Tourne inside Explorer, Windows' taskbar is set to
auto-hide, the edge it waits at is covered (except where Tourne's own bar is),
and the keys that bring it or Windows' Start up are caught. A lone Windows key
is left to Tourne's Start menu when that has it. It all comes back after an
Explorer restart. Turning it off or quitting puts the taskbar back as it was;
after a crash, `TourneDesktop.exe --restore` does. An app flashing for
attention can still make the taskbar peek. Off by default.

## Style apps with your own CSS

Apps made of web pages (Discord, Claude, Spotify, Slack, Obsidian, VS Code,
or any Electron or Chromium app) can be given your own style sheets:

    inject {
        discord {
            on = true
            css = tourne/discord.css, my-discord.css
        }
        myapp.exe = C:\Path\To\App.exe   # any other app
    }

Tourne finds each app's program by itself, wherever it was installed: in
its usual folder, then from the app if it's running, Windows' App Paths,
the installed programs list, Start menu shortcuts, and last the Windows
search index. Give `exe` only for a different copy; a file name alone
(`myapp.exe = Notion`) is found the same way.

Style sheets live in `%APPDATA%\Tourne\Desktop\css\` (the tray menu's
"Style apps" opens it). Its `tourne` folder is made by Tourne from your
theme's colours and written again whenever the theme changes; copy one out
to change it. Saving a style sheet restyles the app at once.

Nothing is applied until you say yes. Turning an app on asks first, with a
warning: the app doesn't support this, so an update can undo or break the
styles, and Tourne has to start the app with a DevTools port on this PC that
other programs on the PC could also use while the app runs. When the app is
later opened without its styles, Tourne asks to restart it with them
(`inject.<app>.restart = ask`, `always` or `never`).

When nothing changes, the program does no work.
