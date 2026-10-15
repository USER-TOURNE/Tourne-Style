# Consistency audit, 2026-10-07

Tourne's goal: one look everywhere, and one place to control it (Tourne Desktop). The split that's decided:

- **Windhawk** runs only what has to live inside other programs: b53's Explorer parts (Explorer Style, Explorer Colours, Theme Gaps, Text Color, Menu No Icons, Menu No Duplicates, Classic Menus, Icon Redirect) and the third-party mods that do the same kind of job.
- **Tourne Desktop** draws and controls everything else: taskbar, Start, tray, clock, flyouts, its own menus, the desktop, window borders, animations.
- **Tweaks** moves out of b53 into its own Windhawk mod (`tourne-tweaks.wh.cpp` already exists).
- **StartAllBack** goes (it's already `Disabled=1` for the user; its DLL still loads into Explorer).

The reference looks are Tourne's screenshots of 2026-10-07: Explorer's right-click, LiteMenu's menu, a tray icon's menu (classic menus, `#121C21` ground, Scientifica, text `#64AA89`, separators `#3F6A5A`, no icons), Task Manager, File Explorer, and the StartAllBack Start menu with green text.

## Why things differ

The root cause behind most items: **in shell mode several things that used to run inside Explorer now run inside TourneDesktop.exe**, and b53 is kept out of TourneDesktop.exe (its Exclude list, so a hook crash can't take the shell down). The explorer-only mods (`remove-context-menu-items`, `desktop-marquee-fade`, `fix-strips`...) don't run there either.

| # | What Tourne sees | Where it runs | Cause | Fix | Status |
|---|---|---|---|---|---|
| 1 | Start menu text not green | Tourne Desktop | `--import` skips `.enabled` values, so Tourne Desktop never saw Text Color on | Suite layer: b53's `textColor`, `menuNoIcons`, `menuNoDuplicates`, `iconRedirect`, `classicMenus` settings read live from Windhawk (10f9886) | Done, live 21:50 |
| 2 | Taskbar, Start and tray menus keep icons | Tourne Desktop | Same: Menu No Icons looked off | Same (10f9886) | Done, live |
| 3 | Tourne's own menus don't look like other programs' | Tourne Desktop | Drawn from the palette or the visual style | `taskbar.menuLook = windows`: Windows' menu colours and font, Text Color's green, measured from the screenshots (10f9886) | Done, set in tourne-gui.conf |
| 4 | Window borders differ from win11-accent-border | Both | Tourne's borders used their own gradient | `accentInactive` colour word; `activeColor = accent`, `inactiveColor = accentInactive` (10f9886). Then win11-accent-border off | Tourne side done; mod off waits for admin |
| 5 | Desktop right-click keeps icons, Paste / Display settings / Personalize, grey text | TourneDesktop.exe (the desktop is a shell view hosted there) | b53's Menu No Icons and Duplicates, remove-context-menu-items and Text Color don't run in TourneDesktop.exe | See decision A | Open |
| 6 | Desktop icons, pinned and Start icons not themed (Icon Redirect) | TourneDesktop.exe | Icon Redirect doesn't run there | See decision A | Open |
| 7 | Task Manager shows Claude's own icon | taskmgr.exe | Tourne Desktop's icon rows (Claude, Discord, NVIDIA...) aren't in b53, so Icon Redirect doesn't know them | Copy Tourne Desktop's "use everywhere" rows into b53 once | Admin |
| 8 | Two tools set menus: eradicate-immersive-menus and b53 Classic Menus | Explorer | Same job twice | Keep eradicate-immersive-menus (it also covers TourneDesktop.exe, where b53 doesn't run) until decision A, then turn it off | Waits for A |
| 9 | local@taskbar-thumbnail-hide-icon | explorer.exe | Only touches Explorer's taskbar previews, hidden in shell mode | Off | Admin |
| 10 | Tweaks inside b53 | windhawk.exe | Tourne wants it separate | Install `tourne-tweaks` mod, b53 `tweaks.enabled = 0` (every group is `use = 0`, so nothing changes) | Admin |
| 11 | StartAllBack still installed | explorer.exe | Replaced by Tourne Desktop | Copy orbs from `%LOCALAPPDATA%\StartAllBack\Orbs` to Tourne's folder, repoint `taskbar.orb`, export `HKCU\Software\StartIsBack`, uninstall | Admin |
| 12 | Changing a Windhawk part means opening Windhawk | - | Settings live in HKLM | Studio writes them through one UAC prompt per save and bumps the mod's `SettingsChangeTime` (Windhawk applies it live) | To build |
| 13 | LiteMenu misbehaves in shell mode | LiteMenu.exe | Unknown, waiting for Tourne's description | - | Open |
| 14 | `ColorPrevalence = 1` (accent on title bars) | DWM | win11-accent-border's readme says it conflicts | Ask Tourne | Open |
| 15 | Extra desktop icons (Home, Gallery, Libraries, Network, Control Panel, user and OneDrive folders) | TourneDesktop.exe | The hosted desktop view showed every Desktop namespace item | desktop-host.cpp filters like Explorer's desktop (ICommDlgBrowser::IncludeObject + HideDesktopIcons) (385abf2) | Done, live 22:02 |
| 16 | Jump lists lost their border with the Windows menu look | Tourne Desktop | Windows' menu border colour is the ground here | Keep the caller's edge then (385abf2) | Done, live |
| 17 | Windows Security's taskbar button unthemed | Tourne Desktop | A Store app: its icon comes from its package, not a file | `taskbar.buttonIcons[18]`: `Microsoft.SecHealthUI_8wekyb3d8bbwe!SecHealthUI` -> SECURITY CENTER.ico, everywhere | Done (config) |
| 18 | Search magnifier green (Start menu, Explorer's address bar) where the style's is red | Both | The green one is an icon (Start: `imageres.dll,-177` = SIID_FIND; Explorer's crumb not pinned down yet); the red one is the Bouquet SAB style's glyph | Draw the style's search glyph in Start; for Explorer, find the crumb's icon and give it a red version | Open |
| 19 | Desktop right-click text grey | TourneDesktop.exe | classic-menus draws with COLOR_MENUTEXT (#AAAFB2); Text Color doesn't run there | `menu-colors.ps1 -Text "#64AA89"` (Windows' menu text green everywhere; arrows follow) | Done 22:05 |

Admin steps (Set-OneOwner.ps1 -DesktopInTourne, the StartAllBack uninstall) were refused by Claude Code's safety system on 2026-10-07 22:05, so they're Tourne's to run. Orbs and StartAllBack's styles are already copied into `%LOCALAPPDATA%\Tourne\Orbs` and `Styles`, and `taskbar.orb` points there.

## Decision A: the desktop Tourne Desktop hosts

The desktop (its icons and its right-click menu) is Explorer's own shell view, hosted inside TourneDesktop.exe. Two ways to give it the Explorer look:

1. **Let b53 into TourneDesktop.exe (recommended).** Remove it from b53's Exclude list; add `TourneDesktop.exe` to Theme Gaps' `excludeApps` (so it never repaints Tourne's own windows), and to Menu No Icons' and Menu No Duplicates' process lists. Text Color stays out (its list doesn't name TourneDesktop.exe; it would recolour all of Tourne's text). Add TourneDesktop.exe to remove-context-menu-items' and desktop-marquee-fade's includes. For the green text in that menu, set Windows' menu text colour to `#64AA89` (`menu-colors.ps1 -Text "#64AA89"`), which the classic-menus mod draws with; the submenu arrows then turn green too (they're `#AAAFB2` now). Exact match, no new code. Risk: a crash in a hook ends the shell child; the watchdog restarts it, and two crashes in 120 s turn shell mode off. classic-menus, dark-menus and eradicate-immersive-menus already run inside TourneDesktop.exe today.
2. **Tourne Desktop does the desktop's menu itself.** Subclass the shell view and restyle its menus as Tourne's own (strip icons, drop duplicates and listed items, the Windows look). Keeps hooks out of the shell, but desktop icons can't be themed this way (they come from the system image list inside the process), and it duplicates what b53 already does.

## Later the same evening (6d6a586, live 22:13)

| # | What | Fix | Status |
|---|---|---|---|
| 20 | Tray menus plain, jump lists framed | System icons' menus take the taskbar's framed StartAllBack look (`FrameLikeStartAllBack`, `TaskbarFramesMenus`); `taskbar.menuLook` back to `theme` | Done |
| 21 | Desktop selection fade (desktop-marquee-fade only runs in explorer.exe) | `shell.marquee.*` in Tourne Desktop, set to Tourne's mod values; Set-OneOwner no longer adds the mod to TourneDesktop.exe | Done |
| 22 | Start's folder icons; the APPS icon read from StartAllBack's registry | `startMenu.allPrograms.appsFolderIcon` and `folderIcon` (Studio > Start Menu > All Programs) | Done; Tourne sets the folder icon |
| 23 | File dialog footer grey in dialogs opened from Tourne Desktop | Theme Gaps runs in TourneDesktop.exe after Set-OneOwner -DesktopInTourne (no longer excluded) | Waits for the admin step |

## Icons everywhere (Tourne, 22:15): "if I set x icon to be x.ico, it should change it everywhere"

Icon Redirect (b53) is the mechanism that reaches every program: it hooks the resource functions, PrivateExtractIconsW and LoadImageW in each process, and Windhawk reloads it live when its settings change. What stops "one setting, everywhere" today:

1. It doesn't run inside TourneDesktop.exe (Set-OneOwner -DesktopInTourne fixes that: desktop, Start, pinned icons).
2. Two lists: b53's rows, and Tourne Desktop's Button icons rows marked "use everywhere" (Set-OneOwner copies the latter into b53 once).
3. A row can only replace a program's main icon (an .ico for `*\program.exe`) or a whole file (a theme DLL). A single Windows icon (the folder icon, the search magnifier, This PC) can't be swapped for an .ico.

Plan (b54 and a Tourne Desktop build):
- Icon Redirect also reads Tourne Desktop's rows from `%APPDATA%\Tourne\Desktop\tourne-gui.conf` (per user, no admin), checking the file's time at most every couple of seconds, so a row saved in the Studio reaches every program without Windhawk or admin.
- Rows for one icon: `imageres.dll,-3 = folder.ico` style originals, so any icon in any file can be swapped for an .ico.
- A Studio Icons page: what (a program, an app id, or a file and icon number, with a picker that shows the icons) and the .ico.
- Limits to say up front: Store apps' logos come from their package images, which these hooks don't reach everywhere (Tourne Desktop's own rows cover its taskbar, Start and Alt+Tab); Explorer's icon cache keeps old icons until it restarts.
