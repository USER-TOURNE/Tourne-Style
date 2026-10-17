# Tourne Desktop

My own desktop for Windows, as one program: a classic taskbar drawn from StartAllBack themes, a Windows 7 style Start menu, Alt+Tab with switcher views and komorebi-style tiling, borders and window motion, the system icons with their flyouts (network, volume and mixer, Bluetooth, batteries, notifications and calendar, security), Tourne Studio for every setting, Keys and rules (hyprland-style binds), capture, phone texts and notifications, Luau plugins with a bar and panels, UI sounds, and Tourne'Table built in. Nothing phones home.

> **Work in progress.** Tourne Desktop is my daily desktop, shared as it stands. It runs every day on my PC, but it has been tested on very few other PCs, it changes often, and some parts are unfinished (listed below). Back up first, read "What can go wrong" before turning on shell mode or the killswitch, and expect rough edges.

On my PC it runs as the Windows shell (shell mode), with Explorer kept underneath for Windows apps. It runs just as well as a normal program on top of Explorer.

## Get it

- **Ready to run:** [`release/TourneDesktop-77a6918.zip`](release/) holds `TourneDesktop.exe`, its manifest, `tournectl.exe` (control it from scripts) and the README. Built from commit `77a6918` (2026-10-15), the newest build published here. My own PC already runs a few later fixes that aren't in this zip yet: startup apps in shell mode, tray icons that come back after a restart, and plugins that apply their settings at once.
- **Build it yourself:** the whole source is in [`source/`](source/). From PowerShell:

      .\source\desktop\Build-TourneDesktop.ps1                    # every module
      .\source\desktop\Build-TourneDesktop.ps1 -Modules switcher  # only some

  It uses Windhawk's own compiler when Windhawk is installed, otherwise clang++ or MinGW g++ on the PATH. The result is `source\desktop\build\TourneDesktop.exe`. Details in [`source/desktop/README.md`](source/desktop/README.md).

## Before you install

**What you need**

| | |
|---|---|
| Windows | Windows 11, 64-bit. Built and used on 24H2 and later. Windows 10 is untested. |
| Windhawk | Not needed for Tourne Desktop itself. Needed for the [mods that pair with it](../README.md#use-together) (Start's Explorer parts, the tray icons inside Explorer, the clock, menus, icons): [windhawk.net](https://windhawk.net), 1.7.3 or newer. |
| StartAllBack | Optional. The taskbar reads StartAllBack themes (`.msstyles`) and Start orbs from StartAllBack's folders when it's installed. Without it, the taskbar uses its own looks. |
| Node.js | Never needed. |
| Python | Not needed to run Tourne Desktop. Only the Wallpaper Depth and Discord Voice plugins use it (Python 3.10 or newer on the PATH, or its path in the plugin's settings), plus the developer tools in `source/desktop/tools`. |
| Admin rights | Not needed to run it. A few steps ask once: the suite's system-wide parts, and Tourne'Icons Lasting's admin-only icon changes. |
| Building it | PowerShell plus Windhawk's compiler, or clang++ or MinGW g++ on the PATH. The system icons need the C++/WinRT headers, which Windhawk's compiler has. |

**Windows settings that matter**

- **Developer Mode** (Settings > System > For developers) is needed for **one thing only: phone calls** (Tourne Calls, below). The phone pane in the Start menu, your phone's notifications and texts, and sending files do **not** need it.
- **Bluetooth** must be on, and the phone paired with the PC, for the phone features.
- **Phone Link**: the Start menu's phone pane reads the phone Phone Link has linked. To have Tourne hold your texts itself, close Phone Link: only one program on the PC can hold the phone's message channel at a time.
- **Live window previews** in the switcher's 3D views need Windows' screen capture (Windows 10 1903 or later); without it they show still pictures.
- Nothing else needs to be switched on or off in Windows. Shell mode, the killswitch and app styles change Windows' own settings themselves, and put them back (see below).

**Plugins that need another program**

Most bundled plugins need nothing. These call a free tool you install yourself, and say so in their settings if it's missing:

| Plugin | Needs |
|---|---|
| Media Downloader | yt-dlp (`winget install yt-dlp.yt-dlp`), plus ffmpeg to join video, sound and cover art |
| Screen Recorder | ffmpeg (`winget install Gyan.FFmpeg`) |
| OCR | Tesseract (`winget install UB-Mannheim.TesseractOCR`) |
| Syncthing | Syncthing (`winget install Syncthing.Syncthing`) |
| Git Companion | the GitHub CLI `gh` (or your platform's CLI), signed in once |
| Wallpaper Depth | Python 3.11 to 3.14; its first setup needs the internet |
| Discord Voice | Python 3.10 or newer, and signing in to Discord through the plugin |
| Alienware Command Center, GMK87, dGPU Monitor | the matching hardware (an Alienware PC, a GMK87 keyboard, a laptop with a separate graphics card) |

**Phone features (iPhone and Android)**

| Feature | How it works | Needs |
|---|---|---|
| Phone pane in Start | Shows the phone Phone Link has linked, its pages and latest notifications | Phone Link set up with your phone |
| Texts | Tourne talks to the phone over Bluetooth (MAP), as a car kit does, and keeps them in `%APPDATA%\Tourne\Desktop\phone-messages.tsv` | Bluetooth pairing; Phone Link closed. An iPhone only shares texts that arrive while it's connected. |
| iPhone notifications | Read over Bluetooth LE (Apple's notification service) and shown as Windows notifications | Bluetooth pairing |
| Files to the phone | Sent over your own Wi-Fi, the LocalSend way | The free LocalSend app on the phone, same network |
| Calls | `Tourne Calls`, a small separate program, makes the PC your phone's hands-free | **Developer Mode on**, then run `source\desktop\calls\Register-TourneCalls.ps1` once (as yourself, not as admin). It builds with Windhawk's compiler and registers a local package. `-Remove` takes it away again. |

## What can go wrong, and how to undo it

- **Shell mode** (off by default) makes Tourne Desktop your Windows shell in place of Explorer, through `HKCU\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell`. If it fails you could sign in to a desktop with no taskbar. Safety nets: a watchdog brings Explorer back after a second crash; **hold Shift while signing in** for Explorer that one time; `TourneDesktop.exe --restore` (or turning the switch off) makes Explorer the shell again. If all else fails: Ctrl+Shift+Esc opens Task Manager, then File > Run new task > `explorer.exe`.
- **The killswitch** (off by default) hides Windows' own taskbar and catches the keys that open it. Quitting or turning it off puts it back; after a crash, run `TourneDesktop.exe --restore`.
- **App styles** (custom CSS for Discord, Spotify and other web-based apps) are unsupported by those apps. An app update can break them, and while a styled app runs it has a DevTools port open that other programs on your PC could use. Tourne asks before each app and warns you.
- **The Windhawk mods** run inside Explorer and other programs. A bad setting or a Windows update can crash Explorer; turn the mod off in Windhawk and Explorer comes back.
- **Unsigned program.** `TourneDesktop.exe` isn't code-signed, so SmartScreen or your antivirus may warn about it. The whole source is here to read and build yourself.
- **If it crashes**, it writes `%LOCALAPPDATA%\Tourne\Desktop\crash.txt`. A part that crashed while starting stays off at the next start until you save a setting.
- **Back up first.** `source/tester-kit` records your Windows settings before you start and puts them back afterwards, and offers a restore point.
- **Remove it completely:** `TourneDesktop.exe --restore`, then `--uninstall` (stops it starting with Windows), then `--stop`, then delete the folder, `%APPDATA%\Tourne\Desktop` and `%LOCALAPPDATA%\Tourne\Desktop`.

## What works, and what isn't finished

**Working, in daily use on my PC:** the taskbar, the Start menu, Alt+Tab with its list, Flip 3D and Cover Flow views, tiling and borders, the system icons and their flyouts, the tray (with folders), Tourne Studio, presets, the Tourne theme, identities, glass, shell mode with startup apps, keys and rules, Tourne Menu (the LiteMenu replacement, with its editor), UI sounds, Luau plugins with the bar and panels, and the built-in Tourne'Table.

**Working, but not finished:**
- Shell mode has no balloon tips yet, and Win+A, Win+N and the system icons' "original menu" don't work in it (they belong to Explorer).
- Motion (Hyprland animations) drives the bar's hiding and window borders so far; other parts join as they get animations.
- Glass shows the wallpaper behind it, not the windows in between.
- Tray icons that come back after a restart, and the stand-in tray at sign-in, are new and still being checked across sign-ins (not yet in the zip above).
- Phone calls need the manual Developer Mode setup above; texts and iPhone notifications over Bluetooth are new.
- Tourne Studio has been less steady in the newest builds; save often.
- The killswitch: an app flashing for attention can still make Windows' taskbar peek.

What comes next is in [Where we're going](../README.md#where-were-going).

## Run

    TourneDesktop.exe              # start (a tray icon shows its menu)
    TourneDesktop.exe --studio     # open Tourne Studio
    TourneDesktop.exe --import     # take the settings of an installed Tourne'Suite
    TourneDesktop.exe --install    # start with Windows
    TourneDesktop.exe --stop       # close it

Settings live in `%APPDATA%\Tourne\Desktop\` (a Hyprland-style config). Each part is a switch in Studio, and any part that's on in the Windhawk suite is skipped here so nothing draws twice. Pair it with the mods in [Use together](../README.md#use-together).

## What's in `source/`

| Path | What |
|---|---|
| `desktop/host/` | The program itself: shell mode, Studio, plugin runtime, capture, launcher, UI sounds, phone, menus |
| `desktop/plugins/bundled/` | The bundled plugins (Caffeine, Power Menu, Control Center, Alienware Command Center, GMK87, Wallpaper Carousel, Screen Recorder, Process List and more) |
| `*.cpp` at the top | The parts shared with the Windhawk suite: taskbar, Start menu, switcher, tiling, system icons, tray, Icon Redirect, Explorer style, Dark Magic, palettes |
| `Build-TourneSuiteRedirect.ps1` | Builds the Windhawk suite from those parts |
| `tools/` | Generators and test harnesses (icons, menus, plugins, Table, Windhawk installers) |
| `research/` | Maps of how StartAllBack themes and works, a YASB theme survey, plans |
| `tester-kit/` | Back up, install, fill in settings, and put a PC back, for testers |

The devlogs that explain all of it are in [`../devlogs/`](../devlogs/).
