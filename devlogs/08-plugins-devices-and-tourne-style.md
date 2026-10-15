# 08. Plugins, devices, and Tourne'Style

*October 12 to 15, 2026.*

## A plugin for everything

The plugin runtime got the system calls a Linux shell would answer: running programs (only the ones a plugin lists, in a job object that dies with it), system stats, GPUs, battery, power modes and plans, wallpapers, sounds, the clipboard. Then a batch of ports from DankLinux and Noctalia, MIT ones ported with credit and GPL ones used as ideas only: Wallpaper Carousel, Process List, dGPU Monitor, Power Modes, Screen Recorder, Timer, Hydrate, Take a Break, Breathing, Kaomoji, Emoji, IP Indicator, Media Downloader, Ambient Sounds, OCR Scanner, Tourne Notes, and more.

- **Screen Recorder** couldn't hold 120 FPS: ffmpeg's GDI capture copies the screen on the CPU. It defaults to Desktop Duplication now, records the PC's own sound through a loopback pipe, and follows a single window when you record one.
- **Tourne Notes** was Noctalia's Notes, which reset a note's name to the date unless you pressed Enter in the name box. Mine keeps names, pastes pictures, previews Markdown, searches and has a trash.
- **The launcher**, plugin hotkeys through the same bind engine as tiling, and **every plugin's bar item as a tray icon** you can move to the overflow, put in a folder, give your own icon, or hide.
- **Caffeine** (keep the PC or the screen awake), a full-screen **Power Menu** over a blurred desktop, and a **Control Center** of every plugin's tile in one panel.
- **UI sounds** from Foley (MIT), off by default: a pop for menus, a swoosh for the launcher, and cues plugins can play.
- **Panels scroll** when they're smaller than what they show, and Start's flyouts can wear the Tourne Menus look.

### The test toasts

For a while I kept getting test notifications from a plugin that was switched off. It wasn't the plugin. The test runner's scratch folder kept every plugin it had ever been asked to run, so an old test-notifications plugin kept starting there. The runner now removes anything it wasn't asked to run.

## Capture

Capture got a picker and a record bar. Then two complaints:

- **Capturing closed the menu I wanted to capture.** The picker took the focus. Now the picker and the bar never take the focus at all: the pointer and keys reach them through hooks, so open menus and flyouts stay put to be captured.
- **Then the bar vanished the instant I clicked it** while holding the capture key. The hook's idea of where the pointer was didn't match the screen's, so the click landed "outside". It asks Windows where the cursor is on its own thread now. The capture key became a setting too (empty means Print Screen).

## Alienware Command Center

Alienware's thermal modes and fans are behind a firmware interface that needs administrator rights for every call. I looked at how T-Troll's AlienFan GUI deals with that: it runs from a scheduled task set to run with the highest rights. So the plugin does the same: one UAC prompt sets up a helper task, and after that the panel switches modes (Quiet, Balanced, Cool, Performance, Full Speed, G-Mode) and fan boosts without asking again. Opening the panel only ever *peeks*, so it never pops a UAC prompt on its own. The lights go through T-Troll's AlienFX Tools, which need no admin at all.

## The GMK87 keyboard

My GMK87 runs on a 2.4 GHz dongle, and I wanted its battery, its lighting, the picture on its little screen and its settings in Tourne. What I found:

- The open-source tools for it (gmk87-node, MIT, and its reference script, BSD) speak a vendor protocol over 64-byte reports: lighting, screen pictures in RGB565 at 240x135, the clock, 48 bytes of settings.
- **The dongle doesn't pass that protocol on.** Over 2.4 GHz only the keyboard's VIA interface answers.
- So the backlight (effect, brightness, speed, colour) works over the dongle through VIA. The screen, its clock and the settings need the USB cable, and the panel says so instead of failing.
- **The battery** isn't in any public protocol. That one is parked until I capture what the vendor's own tool sends.

Every first test was **read-only**. I never send an unknown command to a device to see what happens.

## Tourne'Style

And then this repo. Everything in one place, newest versions only, with the add-on icon packs folded into one, Tourne'Table ported back out of Tourne Desktop as a standalone mod, and a community page for its styles with a bot that checks them.

Two last lessons from the day it went up:

- **32-bit again.** Tourne'Table 2.1 had kept 2.0's 64-bit-only line. The catalog maintainer had asked for that line to go back in September, because Windhawk 1.x runs tool mods as 32-bit. It's gone, and 2.1 now compiles and links for both.
- **The first shared style went to the old repo**, because the copy of Tourne'Table built into Tourne Desktop still pointed there. Moved over, and the bot answered it on the first try.

Next: the battery, hover tooltips in plugin panels, and whatever breaks first.
