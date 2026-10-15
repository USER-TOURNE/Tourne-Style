# 02. Small mods and pixel icons

*September 20 to 25, 2026. The week the desktop started looking like mine.*

Once the visualizer was out, I started on everything else on screen that didn't match. None of these were meant to be big projects. Most of them didn't stay small.

## The little ones

- **Tourne'Text [System Text Color]** recolours the white system text Windows draws everywhere. Every Tourne colour setting reads `#AARRGGBB` with **alpha first**, and that convention stuck for everything after. Later it learned to recolour Chrome's menus too, which was its own adventure: Chrome draws menus with Skia, so none of the usual text hooks see them. It hooks two functions inside `chrome.dll` found through Chrome's public symbols, and the symbol file for one Chrome version is 5.6 GB.
- **Tourne'Menu [No Icons]** strips icons out of context menus, and **[No Duplicates]** removes the second copy of an entry that shows up twice.
- **Tourne'Tray [Change Tray Icons]** swaps any app's tray icon for your own, including apps that still use the old ANSI calls.
- **Hide the app icon in taskbar thumbnail previews** does exactly what it says.

## A pixel tray, generated

I wanted the tray in a pixel style that matched everything else. Drawing 91 icons by hand three times over wasn't going to happen, so the icons are **generated**: each is a 12x12 grid on a 13x13 canvas with a one-pixel drop shade, only ever scaled by whole numbers so it stays crisp, in three tones (sage, ink, alt). The output came in three forms: PNGs and ICOs, resource-only DLLs for Resource Redirect with one-click Apply and Revert, and a mod.

The mod exists because redirecting resources couldn't reach everything. StartAllBack draws its network icon from a **font glyph**, and the battery comes from sprite strips, so there's nothing to redirect. **Tourne'Tray [System Icons]** draws its own icons with live state instead, and forwards clicks to the real ones underneath.

Then it grew flyouts. Every icon got a Windows 7-style flyout in pixel fonts:

- **Volume:** a mixer of every app's sessions, the wheel changes volume, and a middle click switches the output device.
- **Network:** a Wi-Fi list with connect and disconnect.
- **Brightness:** DDC/CI on a worker thread, because talking to monitors is slow.
- **Security:** Defender's state, last scan, Windows Update, and the system drive.
- **VPN:** driven through the VPN client's own command line.
- **Bluetooth:** connected devices and their batteries.
- **Notifications and calendar:** a notification centre that reads Windows' own notification database read-only, dismisses through Windows' own toast history (so it stays in sync with Windows), and a calendar that reads ICS feeds with proper time zones.

### Things that bit

- **The location prompt.** Windows 11 24H2 counts asking for the *current* Wi-Fi connection's details as location use, so just drawing the network icon put Windhawk in the location privacy list. I tested probe programs against Windows' consent store until I found which calls count. Signal strength and the network's name come from places that don't. Only scanning for nearby networks still does, and only when the flyout opens. Later that became a setting with an honest description of why.
- **The hidden icons arrow.** With StartAllBack's "^" turned off, the real overflow window doesn't even exist, so clicking it did nothing. System Icons got its own overflow flyout and reads the hidden icons straight out of the tray's toolbar.
- **Steam's tray icon** is a version-0 icon: a single click opens its menu and a double click its window. That needed a setting to turn a click into a double click.
- **HWiNFO's sensor icons** became tiny readouts drawn in 4x7 digits, green for the GPU and red at 85 °C.
- **NVIDIA's container denies opening its process**, so working out which program owns an icon needed a fallback through a lower-level system call.
- **Two copies of every tray icon after a reboot.** The single-instance mutex was created owned, and the thread that owned it exited, which abandons the mutex. The second launch seven seconds later saw "abandoned" and ran anyway. Fixed with an unowned mutex held as a handle.

## Tray folders and the clock

- **Tourne'Tray [Classic Tray Folders]** groups tray icons into folders in StartAllBack's classic tray.
- **Tourne'Tray [Clock]** restyles the classic clock: lines, fonts, colours, positions.

Both depend on StartAllBack's classic tray existing, which matters a lot in the next chapter.

## Start orbs

Four orb designs (bar, stack, level, split) as StartAllBack orb strips. StartAllBack shows orbs at their real pixel size, so a 3x orb overflowed the 38 px taskbar. The fix was to redraw the marks on a 16 px grid and pad them into 54 px frames, which is the exact size StartAllBack expects at 96 DPI. Anything else gets resampled and blurry.

**Where they are:** each mod in [`windhawk-mods/`](../windhawk-mods/), the art in [`icon-packs/tourne-tray-icons/`](../icon-packs/tourne-tray-icons/) and [`icon-packs/tourne-start-orbs/`](../icon-packs/tourne-start-orbs/).
