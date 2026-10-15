# Tourne'Icons: packs keep your icons, stubborn redirects in Lasting

When: 2026-10-12 00:10 UTC, code-web, branch claude/project-thread-0rkfwp.

Why:
- Tourne: "port this into the lasting version of the windhawk mod we have, with settings for specific redirects that are stubborn".
- Tourne: the pack "changed some of my already set icons that I had redirects for already inside of b55 ... only the default windows ones".

What changed:
- Tourne'Icons 2.4, Lasting 1.4, Kit 1.1; details in DEVLOG.
- Pack maker skips Tourne's own icons; new pack in /mnt/project-files/tourne-icons/tourne-pack-new.
- Put-back pack: /mnt/project-files/tourne-icons/tourne-restore/pack.tsv (73 restores, 31 put-backs, 833 marks).

My reading of "stubborn" (a default, tell Tourne if wrong): a target plus icon list in Lasting's settings; those shortcuts are watched while the mod runs and get the icon back when an app remakes them.

Follow-ups the same night:
- 2.4.1 (eac68c9): Windhawk's 32-bit build rejected a lambda passed to EnumResourceNamesW. Check mods with clang --target=i686-w64-mingw32 before sending them.
- 2.4.2 (b8edfaf): the pack's *\tournedesktop.exe row made every Tourne Desktop tray icon the T. The tray swap now happens only for a program with one tray icon, never in TourneDesktop.exe, and unloading the mod puts them back.

State:
- Wine-tested only.
- Copies in %USERPROFILE%\Downloads: both mods, TourneIconsKit.exe, "Tourne icon restore\pack.tsv", "Tourne icon pack (keeps yours)".

Next:
- Tourne installs 2.4 (the live mod), then Apply a pack with Downloads\Tourne icon restore\pack.tsv and answers Yes.
- Then the new pack if wanted; it now asks before touching their icons.

## Later: 2.5 / Lasting 1.5 / Kit 1.2 and the ink pack (01:00-02:00)

- Unknown file types get their own `TourneIcons.<ext>` type (the "XVS on everything" break); fixed on start.
- Changes show at once: Explorer redraw (shell icon size nudge) plus `WM_APP + 4` to Tourne Desktop's bars.
- Clear icon cache button (Tourne's ps1, built in).
- New ink pack (all unique, 850 rows) in Downloads as `Tourne ink pack.zip`. Skips Tourne's Tourne Desktop tray programs except ShareX, puts Windows icon 23 (video thumbnail frame) back.
- Waiting on Tourne: install 2.5, unzip and Apply the ink pack, check unknown files, thumbnails, Steam.

## Later: 2.5.1 (5d6a991)

- Copilot's "logo pictures couldn't be found" fixed: it is a sparse package (only the manifest in WindowsApps), so Tourne'Icons now reads its outside folder (`GetPackagePathByFullName2`) and redirects there too. Needs real Windows.
- Folder pictures in the open dialog: imageres 5 (back) and 6 (front) are layers Windows stacks around the contents; the ink pack now draws them as one folder split in two on the same grid (`pack-maker/variants.py` FOLDER_BACK / FOLDER_FRONT).
- In Downloads: tourne-icons.wh.cpp 2.5.1, lasting 1.5.1, TourneIconsKit.exe 1.2.1, Tourne ink pack.zip (849 rows). Tourne's old extracted "Tourne ink pack" folder there is stale.
- Next for Tourne: install 2.5.1, Apply a pack from the new zip, Clear icon cache, try Copilot again.
