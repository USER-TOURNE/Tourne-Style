# 03. The suite: one mod made of parts

*September 24 to 29, 2026. Builds 1 to 7, and the decision to rebuild StartAllBack.*

By the 24th I had a pile of my own mods plus a pile of other people's theming mods, and I wanted one thing to rule them all.

## What got merged, and what didn't

The first idea was to merge *every* theming mod I ran into one. I looked at what that would actually cost before doing it:

- mods hooking the same functions would fight each other;
- a mod that loads into every process (`@include *`) drags everything else in with it;
- I'd lose every upstream update from the original authors;
- several are GPL, which would infect the whole thing.

So **Tourne'Suite** became my own six mods merged into one, plus a **theme controller** that saves and restores snapshots of the other theming mods' settings. That second piece meant I could switch the whole look without owning anyone else's code.

## How a suite build is made

The suite is **generated**, never hand-edited. A build script takes each part's source and:

- wraps it in a namespace of its own;
- renames `Wh_ModInit` and friends to `PartInit`;
- prefixes every setting with the part's name (`trayIcons.`, `textColor.` and so on);
- routes hooks through one function that **chains** them, so two parts can hook the same function (Text Color and Tray Folders both hook `CreateWindowExW`);
- writes my current settings in as the defaults.

### Things that bit

- **The prefix only reaches literals.** The script prefixes setting names only where they're passed as a literal string to one of a known list of functions. A helper I added later (`IntSetting`) wasn't on the list, so every number setting in the switcher quietly read 0 in the suite while working fine in my test harness, which ignores prefixes.
- **A PowerShell here-string drops its last newline.** Inserted YAML glued the next setting onto the previous line (`...uses.- logging: false`), and Windhawk refused the whole settings block with "duplicated key". Every build now gets checked for that pattern.

## The split: out of Explorer where possible

The same instinct as the visualizer: don't live in Explorer unless you have to. The **split build** runs System Icons in its own tool process, at the same integrity as Explorer, and leaves a small **relay** inside Explorer for the few jobs that genuinely need to happen there (forwarding clicks, catching tray messages). Each part says when and why it needs Explorer.

## The redirect branch: my own Icon Redirect

Resource Redirect is the standard way to theme icons, and it's GPL. I wanted MIT, so **Icon Redirect** is written from scratch. Only RR's setting names and its public theme list (data) were reused. It hooks the four kernelbase resource functions, builds a synthetic icon group when you point a program at an `.ico`, and also catches loose `.ico` and `.png` loads.

Measured per call: a file with no rule costs **+1.4 µs**, and replacing a program's main icon with an `.ico` is **1.2 µs** against 118 µs for Windows doing it alone. (An old note of mine compared that 118 µs against Resource Redirect. It's Windows without any redirect; I never ran a real head-to-head.)

The same branch added:

- **Bluetooth** in the tray, connecting and disconnecting audio devices through a Bluetooth audio property set only one of my devices actually supported.
- **Tweaks:** every Winaero Tweaker option I could find, as switches. 312 settings in 18 groups. Winaero's licence forbids reverse engineering, so the list came from reading its *window* with a read-only UI walker, and its checkbox states from counting pixels. The first time Tweaks sees a setting it only remembers it; after that only your changes apply. Machine-wide changes are batched behind one UAC prompt.
- **Device batteries:** a port of HaloBattery (MIT) for headsets, mice and controllers, plus reading Alienware devices over raw HID.
- **Spatial sound** in the volume menu, using the same calls Windows' own volume flyout uses (found through its public symbols).

## Then: rebuild StartAllBack

By the 26th the plan was a list: Winaero options as toggles (done), **StartAllBack rebuilt from scratch**, then every dark mode fix as a toggle.

There were two ways to rebuild StartAllBack:

- **A:** skin Windows 11's XAML taskbar. Rejected: the Windows 11 layout stays, Start can't be themed, and my tray parts would break.
- **B:** my own classic Win32 taskbar and Windows 7 style Start, drawn straight from StartAllBack's `.msstyles` themes, so every existing SAB theme (DeviantArt, VSThemes, mine) works unchanged.

I picked B. Step one was proving I could read a visual style from a file at all, through two undocumented uxtheme entry points. Then:

1. A **test bar** at the top of the screen while StartAllBack kept the real taskbar.
2. StartAllBack's Taskbar page as settings: location, combining, centring, icon sizes, auto-hide, **segments** (islands with rounded corners, the bar a layered window with holes cut out of it).
3. 27 colour schemes, then schemes worked out from the visual style itself.
4. Button icons resolved the way Explorer does it. That needed Windows' undocumented app resolver to group windows by app id exactly as StartAllBack does, and to find the implicit shortcuts where my themed icons actually lived.
5. Pins read from the taskbar's binary pin list, jump lists through another undocumented interface, pin and unpin through the relay (the verb only works inside Explorer).
6. Tray hosting, previews, dragging icons between the bar and the overflow, themed menus.

And around the bar:

- **The switcher:** my own Alt+Tab, written from scratch to replace Simple Window Switcher (GPL).
- **Tiling:** komorebi-style, held on a leader key.
- **Theme Gaps:** Windows' fixed dark greys swapped for the style's own colours. It fixed the Open and Save dialogs' footer.
- **Dark Magic:** StartAllBack's dark-mode-for-classic-controls, rebuilt.
- **Explorer Layout:** the bottom details pane. On my Windows build the folder layouts live in **shell32**, not explorerframe where everyone says they are, so it never applied until both were handled.
- **A Windows 7 Start menu**, drawn from the style's Start panel classes.

### Things that bit

- **The switcher's hook ran on its drawing thread.** When drawing took too long, Windows timed the keyboard hook out and passed Alt+Tab to Windows' own switcher, so a held Tab sometimes showed both. Rule ever since: **never put a low-level hook on a thread that does UI work.**
- **Spatial sound.** Setting the format that was already active cut the music. Rule ever since: no live audio probes, only reads.

## One build, one id

On the 29th builds got numbers. Each build is its own mod (`tourne-suite-b1`, `-b2`...) with its own id, so a new build installs next to the old one and going back is one switch. Settings are copied from whichever build is running, so nothing has to be set up twice.

That's where the next chapter starts.
