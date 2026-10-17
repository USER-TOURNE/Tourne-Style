# 04. Our own taskbar, build by build

*September 28 to October 8, 2026. Builds b8 to b55.*

The goal for this stretch was simple to say: a taskbar and Start menu that look **identical** to StartAllBack with any theme, side by side, pixel for pixel. Then switch StartAllBack off and keep going.

## How I checked anything

- **A test bar outside Windhawk.** The same taskbar code builds as a plain program driven by environment variables, so I could photograph a change without touching the live taskbar. A comparison script shot StartAllBack and mine across four styles, idle and hovered, and diffed them.
- **Only real screenshots count.** My own captures showed StartAllBack's acrylic areas as **copper**, which they aren't. Every colour was measured from real screenshots only.
- **Every build compiles for 64 and 32 bit.** 32-bit Windows wants `__stdcall` callbacks, and lambdas don't convert. That one kept coming back.

## b8 to b16: looking right, feeling fast

- Follows StartAllBack live: change SAB's style or orb and the bar reloads.
- The bar's 3 px frame, button frames 28 px high and 4 px in, the hover glow inside them, the Start menu's row heights and pitches measured off video frames.
- **Lightning fast** (b9 to b11): app lists, folder contents and jump lists read on background threads and cached, Start repaints only changed rows, Windows' menu animations off for our popups.
- Stacked windows drawn with the theme's own stack parts; StartAllBack's double frame on every menu.
- **A bar per screen** with its own DPI scale, hot reload of themes, drag-and-drop theme install, and a hover glow that fades like SAB's.

## b18 and b19: YASB themes

I read all 68 themes in the YASB gallery to see what porting them would take: 78% use CSS variables, most of the CSS is popups, only 20 even show task buttons. So I wrote a small CSS engine, a YAML reader and a software renderer (no libraries, GDI only). It kept 6,551 of 6,908 rules and recognised 99% of declarations, and the bar can now be drawn from a YASB theme, with a gallery window that installs them straight from the repo. Themes can name commands, which **stay off** unless you turn them on: a theme is someone else's file.

## b21 to b26: the details you notice

- Auto-hide slides and fades through the compositor, timed to the screen's refresh (6.9 ms frames at 144 Hz), with no redraw per frame.
- Progress bars on task buttons, caught where Windows actually sends them.
- **A crash guard:** if Explorer dies three times in two minutes of starting, the suite stops loading into it and says so.
- The style's own tray parts, show desktop button and preview parts, only when the style really has them (compared by pixels against Windows' fallback).
- **Explorer Colours** (drive bars recoloured but keeping the style's own shape), and the Windows logo in winver replaced from a folder of pictures, with no system file touched.
- **b26: speed.** The rest of the suite stopped waiting for the taskbar (650 to 1400 ms down to 12). Start's first open went from about 200 ms to about 30, later opens from 25 to 9. A screen change went from a full reload to 26 ms.

## b27 to b41: the long middle

- The Wi-Fi list vanished after Windows' Wi-Fi service restarted: the connection to it went dead and every call failed quietly. It reconnects now.
- **b29:** Windows' own taskbar cloaked from inside Explorer, and every opening of Windows' Start handed to mine, the way m417z's taskbar mod does it. If my bar dies, Explorer's relay notices at once and puts Windows' taskbar back, so there's always a taskbar.
- All Programs went from 340 ms to **1.6 ms** by warming it while closed.
- Wi-Fi CONNECT did nothing: the click went to the row the button sits in, not the button. Clicks now go to the smallest target under the pointer.
- Menus and flyouts kept on their own screen, a menu in the hidden icons flyout, new calendar events through Google Calendar's own page (it sends nothing itself and keeps no account).
- The side pane that could grow but never shrink. A probe DLL in a test folder window showed Windows' sizer clamping the pane at the width it opened at. Letting that floor go fixed it.
- Window layouts, every window's place remembered, a **virtual desktops pager** with previews, renaming and its own keys, scans from the security flyout.

## b42 to b47: the crash I chased for a week

Explorer had crashed 11 times since StartAllBack went off, and never before. Every crash was inside Windows 11's own XAML taskbar.

- **b42:** hide that taskbar the way that doesn't upset it (Windows' own auto-hide, then refuse every reason to unhide, then cloak it on its own thread). Better, not gone.
- Then it turned out it could be triggered **on demand by extracting an archive.**
- **b44** kept StartAllBack loaded and hid its taskbar instead: no crashes, two taskbars, and StartAllBack off was the whole point. Taken back out.
- **b45** made one relay message wait. Didn't matter.
- **b46** had switches to turn the relay's hooks off one by one. All off, it still crashed. So it wasn't my hooks.

Then the crash dumps, read with Microsoft's symbols, told the real story. The *hidden* XAML taskbar still builds a button for every window. Extracting an archive reports progress on its window, so that button builds a WinUI 2 progress bar, whose template asks for a class WinUI 2 can't make by name. The next animation fails, and XAML takes Explorer down. Any progress on a hidden XAML taskbar would do it.

**b47: the fix was to not let the progress through.** My relay already catches every app's progress to draw it on my buttons. While Windows' taskbar is hidden, it now stops there. Extracting, copying and downloading never crashed Explorer again.

## b48 to b51

- **`tourne:` designs:** any program's tray icon, taskbar button and Alt+Tab entry can wear one of the pixel designs (`tourne:screenshot`, `tourne:chat`...).
- **A tester kit:** a clean build with nothing from my PC in it, a backup of every setting it can touch, "fill in my settings" from the tester's own StartAllBack and tweaks, and a full reset. Its first version wrote empty lists, and Windhawk refuses a whole settings block over one empty list.
- **b50:** an option to stop Windows' taskbar from loading its content at all, through a path Microsoft built into `Taskbar.dll`. The first b50 didn't compile: `Wh_Log` is a macro in a real build and a function in the editor's checks, so I'd been syntax-checking in the wrong mode.

### The reset

Mid-way through, a crash that had nothing to do with the suite ended in a **System Restore**, which took the newest settings with it. That's the day everything moved into git.

## b52 to b55: the tail

Window manager phase 1 (your own tiling key, layouts per screen and desktop, send-to-desktop keys, a focus border, Tacky-style window borders drawn as four thin strips instead of one window the size of the screen), Icon Redirect reaching packaged apps' logo PNGs, and a fix for **Handy**'s tray icon. Handy is a Tauri app, and Tauri asks where its tray icon is before acting on any click, and drops the click if that fails. Behind my tray it always failed.

b55 is the last suite build. By then most of the bar had moved somewhere else, which is the next chapter.

**Where it is:** [`windhawk-mods/tourne-suite/`](../windhawk-mods/tourne-suite/) (b55), with the source in [`tourne-desktop/source/`](../tourne-desktop/source/).
