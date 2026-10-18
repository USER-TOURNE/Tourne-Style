# 05. Tourne Desktop becomes the shell

*October 3 to 8, 2026.*

By build 51 I wanted one running program for the whole look, not a Windhawk mod with fifteen parts. A lot of the suite didn't need to hook anything at all: the taskbar, the switcher and the system icons only draw their own windows. So they moved out.

## One program, the same source

**Tourne Desktop** builds those parts from their **unchanged** source. A small shim answers Windhawk's API from the host (settings, logging, subclassing a window on another thread), and the build refuses any part that would hook other programs: those stay in Windhawk. Settings live in a Hyprland-style text config that applies the moment you save it.

Around the parts, it grew everything a desktop needs:

- **The Tourne theme:** colours made from the wallpaper (k-means in OKLab, every text colour pushed until it meets the contrast asked for), written to one record that the taskbar, switcher and icons all follow. Nothing runs while nothing changes.
- **Tourne Studio:** every setting in one window, pages built from each part's own settings schema, live while you edit, kept when you save, with undo and redo. It writes only its own file, so a hand-written config stays yours.
- **Glass:** a Direct3D shader for liquid glass (refraction at the edge, fringe, blur, lens, fresnel, oil film) behind the Studio, around the window in front, or inside chosen apps.
- **824 presets:** I went looking for every palette and look I could find. 1,322 turned up (base16, terminal themes, Omarchy, HyDE, end-4, YASB, niivu...), and 824 had a full set of colours. One click makes one the theme.
- **Identities:** one switch for shape, frame, material and motion across every piece (Wise Wolf, Everforest, Ghost Shell).
- **A CSS injector** for apps made of web pages, behind a warning, because it needs the app started with a debugging port.

### The first crash on my PC

Tourne Desktop showed its tray icon for a second and closed. Ten crash dumps later, it was the app styler: when a window came to the front, it split the program's path at its last backslash, and a path with none threw an error nobody caught. The crash report had a bug too: its message box let the failing code run again inside the report. Now Tourne Desktop writes `crash.txt` with every frame named, and if it dies while a part is starting, the next start leaves that part off and says so.

## Killing Windows' taskbar from outside

I wanted StartAllBack and Windhawk both gone and Windows' own taskbar gone with them. Everything that truly removes the taskbar runs *inside* Explorer, and hiding its window from outside breaks Windows 11 (see the crash in chapter 04). The **killswitch** does what can be done from outside: auto-hide, a 3 px invisible guard over the edge where it waits, and a keyboard hook for Win+T and friends. An app flashing for attention could still make it peek. The only real fix was not running Explorer's taskbar at all.

## Shell mode

So Tourne Desktop became **the Windows shell**, in five steps:

1. **The switch and a watchdog.** Tourne Desktop sets itself as the account's shell. A watchdog runs the real copy and waits: exit 0 (you quit) hands the session to Explorer, a crash restarts it, two crashes in two minutes give you Explorer and turn shell mode off until you save a setting. Holding Shift at sign-in gives you Explorer once. It also starts your startup apps the way Explorer would, skipping what Task Manager has off.
2. **A tray.** An invisible `Shell_TrayWnd` that apps can find, taking every `Shell_NotifyIcon` message and announcing itself so apps that started first add their icons again.
3. **App bars and the work area.** Other programs' docks and bars ask Explorer for an edge of the screen. Now they ask me, and maximised windows stop at my bar.
4. **A desktop.** A window named the way Explorer names its desktop, hosting Windows' own folder view of the Desktop folder, so icons, right-click menus and renaming all behave as usual.
5. **The system icons, built in,** because Windows' network, volume and battery icons are Explorer's and vanish with it.

### The first real sign-in

It came up **black**, with two screens flashing.

- **No taskbar:** a broken Windhawk entry left behind by a removed suite build still said the taskbar was on in the suite, so Tourne Desktop thought someone else was drawing it and stepped aside.
- **Black desktop:** with no Explorer, nothing hands Windows' painter the wallpaper. The desktop now does it and draws it itself.

### Then a full restart

Windows' taskbar showed and my tray was empty. Windows ends the shell while shutting down, the watchdog restarted it, and Windows ended that too. The watchdog counted those as **two crashes at sign-in**, turned shell mode off and gave the next sign-in to Explorer. The shutdown codes now just stop the watchdog. And after a cold boot Explorer could take longer than 20 seconds, so now Tourne Desktop waits for it, then hides it and takes its tray over.

### Swapping builds without losing the shell

Swapping a new build in meant killing the running copy, which the watchdog counted as a crash. Once, the new build crashed a second later, which made two, and the shell was gone for the rest of the day. Now a swap ends the old copy with a special exit code (3, a planned restart), and the watchdog starts the new one without counting anything. **Never quit the shell copy cleanly**: exit 0 means "give the session back to Explorer".

## The hooks question

I asked to move off Windhawk entirely: Tourne Desktop loading its own hooks into other programs. A plan was written, and it was shelved. Loading code into other programs is exactly the job Windhawk already does well, so the arrangement stands: **Tourne Desktop draws the shell; Windhawk runs the suite with only the hook parts on** (Explorer style and colours, Theme Gaps, classic menus, text colour, menu cleanup, Icon Redirect, Tweaks). One owner per option, never two things drawing the same menu. I'm still a bit annoyed about needing two tools.

The one exception: **Icon Redirect builds into Tourne Desktop too**, hooking only Tourne Desktop's own process (with MinHook), because the suite is kept out of it and its icons had gone back to Windows' own.

## Plugins

Plugins came from **Noctalia**'s catalogues: 30 of them run on Windows, listed in the Studio with exactly what each will be allowed (internet hosts, files, clipboard, programs) before you install it. They run in **Luau** (built in), each entry in its own sandboxed VM with a 2-second call limit and a 64 MB cap, so a plugin that hangs can't hold the taskbar up. Bar widgets came first, then panels drawn in the flyouts' pixel look.

Lesson from building it: a same-process `WM_COPYDATA` with a timeout is unsafe (if the sender gives up, the data is gone), and any send to a busy thread stalls the sender. The bar only ever **posts** to the plugin runtime.

## The phone

Phone Link has no way in for another program, so Tourne talks to the iPhone itself:

- **Texts** over Bluetooth MAP, the way a car kit does: a session to the phone's message server, notices when texts arrive, replies sent back, and a history kept locally, because an iPhone only shares texts that arrive while connected.
- **Notifications** over Apple's notification service (ANCS) on Bluetooth LE: each one becomes a Windows notification as its phone app, with its own buttons (Answer, Decline, Clear).
- **Calls** through a small helper with a package identity of its own, because Windows only lets apps with the right declared capabilities handle the phone's line. While a call talks, two audio pumps bridge the phone's hands-free audio to the PC's own mic and speakers. The first real calls showed the bridge never ran: each check made a fresh copy of each call and dropped the one its events were tied to.
- **Send a file** to the phone's LocalSend app over the local network, no cloud.

Phone Link itself went off once Tourne held the phone, since only one program can hold that Bluetooth channel at a time.

**Where it is:** [`tourne-desktop/`](../tourne-desktop/), ready to run in `release/`, the source in `source/`.
