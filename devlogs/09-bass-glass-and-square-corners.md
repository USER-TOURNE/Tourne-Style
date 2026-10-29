# 09. Bass, glass, and square corners

*October 15 to 29, 2026.*

Two weeks after Tourne'Style went up. Most of it went into Tourne'Table and into making Tourne Desktop feel like one thing instead of many. Along the way the Windhawk suite went from b55 to b65.

## Tourne'Table learns to hear the bass

What people asked for most was the low end. A bar visualizer spreads the bass over a few thin bars on the left, and a kick drum gets lost next to everything else. So 2.3 has three answers:

- **Bass**, a group that sets where the bass starts and ends, how hard it hits and how fast it lets go. Oscilloscope Bass Focus filters the scope down to the bass and triggers on it, so a bass note holds still as a clean shape instead of shaking.
- **Bass Line**, a small lane of its own that follows only the bass on its own auto-range. Kicks and 808s pop up in one colour, snares and claps in another, and hi-hats stay out. Telling a kick from a snare from a hat took the longest: it's all about where in the spectrum the energy jumps and how fast it dies.
- **Depth**, which puts the visualizer behind your wallpaper's subject, like a phone lock screen tucks the clock behind a person. It needs a cut-out of the subject. You can give it one, or it can make one with rembg, which it installs only for your user and only after telling you exactly what it will download. The mod itself still never goes online.

Then **Glass Bars**: every bar is a lens over your wallpaper, bent, tinted and lit along its edge. It reads the wallpaper file once and never captures the screen, which is why it costs almost nothing.

## A settings panel instead of a settings page

Windhawk's settings page has every setting, but you can't see the visualizer while you're on it. So right-click now opens a quick settings panel over the visualizer: search, a page per group, edit in place, scroll over a number to change it live. Every one of the 270-odd settings has a hover tip with what it does and a value to try. To keep the panel, the shared styles and the settings themselves from drifting apart, one script builds the settings list from the settings block, and everything else reads that list.

The Style Editor got redrawn the same way: tiles of styles playing on a test beat, mine and the community's, click one to try it.

The first tester report came in within a day: the Bass Line sat on top of the scope, and the colour picker couldn't do see-through colours. Both are fixed in the beta. The lane has a Layer setting now, and colours are typed as `#AARRGGBB`.

## The catalog version

Tourne Desktop's built-in Table also had my frame pacing experiments: Frame Sync, Low Latency and Frame Generation. They're fun, but they aren't ready for the catalog. So I made a script that builds the Windhawk version from the Desktop one, takes those three out and puts 2.1's Smooth Mode back. Everything else from 2.3 stays, Engine Cores included. One source, two builds, and they can't fall out of step.

## Tourne Desktop pulls together

- **Canvas**, a switcher view ported from PrinceWM (MIT): every window is a live tile on an infinite canvas that you pan, zoom and arrange, and it remembers where you put things. Win+Tab opens it now.
- **One command language** for key binds, menu items and scripts, plus tap, double press and hold on one key. Tapping Win on its own can run a bind without opening Start, which took a surprising amount of convincing.
- **Layouts you write yourself**, as a small Luau file the tiling asks where each window goes. It runs sandboxed: no files, no internet, 40 ms and it's out.
- **Look packs and a gallery**: a look, its wallpaper and its plugin list in one file, with a screenshot beside each one.
- **Designer mode**: freeze the screen, pick a colour off it, click any part of Tourne, and choose which of that part's colours takes it.
- **Capture** got markup (pen, arrows, boxes, numbered markers, redact) and recordings with the PC's sound and a microphone.

## The cream row

Task Manager in my colours took eight suite builds, b56 to b63, and most of them were for one row. The cells, the lists and the headers came quickly. But one row, the group heading next to "Apps", kept its cream usage colour every time. Each build caught it one more way: from any caller, as a text background, in gradient fills, and finally in DirectUI's GDI+ fills, where the cream is drawn see-through. Lesson: when a colour won't die, it's being painted by a path you haven't looked at yet, so list every path before you write the next fix.

## Taking things over instead of hiding them

Two fixes taught the same thing:

- **The volume flyout.** I hid Windows' own volume pop-up when it appeared, and it still flashed for a frame on every key press. The fix was to take the volume keys myself and step the volume the way Windows does, so its pop-up never opens. Media knobs too.
- **Print Screen** kept going back to Windows' tool. Windows quietly drops a keyboard hook that answers too slowly, so the hook got a thread of its own and is set again every minute.

## Square corners

Windows 11 rounds every window, and I wanted all of them square, menus too, unless a program is on an exclude list (anti-cheat games). The catch: only the program that owns a window may change its corners. So Square Corners is a suite part that runs inside every program and asks for square corners on that program's own windows, and catches each menu as it opens. Tourne Desktop has the switch, the suite (b65) does the work.

## Dropping windows on the pager

Last one: drag a window by its title bar onto a desktop's mark on the pager, and it's sent to that desktop. The first version guessed moves from resizes by whether the window's size changed, which broke on snapped windows. Now it asks the window what's under the pointer when the drag starts, so only a real title bar drag counts, and the nearest mark within a few pixels wins.

Next: the lock screen clock, more of Windows' own apps through the App Styler, and whatever the testers break first.
