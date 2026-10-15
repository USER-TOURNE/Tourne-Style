# Tourne Desktop

Tourne's own desktop for Windows, as one program: a classic taskbar drawn from StartAllBack themes, a Windows 7 style Start menu, Alt+Tab with switcher views and komorebi-style tiling, borders and window motion, the system icons with their flyouts (network, volume and mixer, Bluetooth, batteries, notifications and calendar, security), Tourne Studio for every setting, Keys and rules (hyprland-style binds), capture, phone texts and notifications, Luau plugins with a bar and panels, UI sounds, and Tourne'Table built in. Nothing phones home.

On Tourne's PC it runs as the Windows shell (shell mode), with Explorer kept underneath for Windows apps. It runs just as well as a normal program on top of Explorer.

## Get it

- **Ready to run:** [`release/TourneDesktop-77a6918.zip`](release/) holds `TourneDesktop.exe`, its manifest, `tournectl.exe` (control it from scripts) and the README. Built from commit `77a6918` (2026-10-15), the newest build, the one running on Tourne's PC.
- **Build it yourself:** the whole source is in [`source/`](source/). From PowerShell:

      .\source\desktop\Build-TourneDesktop.ps1                    # every module
      .\source\desktop\Build-TourneDesktop.ps1 -Modules switcher  # only some

  It uses Windhawk's own compiler when Windhawk is installed, otherwise clang++ or MinGW g++ on the PATH. The result is `source\desktop\build\TourneDesktop.exe`. Details in [`source/desktop/README.md`](source/desktop/README.md).

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
