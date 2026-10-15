# Tourne'Icons Kit: run once, changes stay

When: 2026-10-11 22:30 UTC, code-web, branch claude/project-thread-0rkfwp.

Why:
- Tourne: "something that opens, someone runs it once and then closes it out completely, it doesn't start on startup, and all the icons changed persist permanently (then if they open it again, they can return things to how they were before, or change as desired)".
- Tourne asked how the live swap keeps Discord's icon through updates, and to work it out from their PC.

What changed:
- Tourne'Icons Kit 1.0 (tools/icons/kit/, TourneIconsKit.exe) and Tourne'Icons 2.3 / Lasting 1.3; details in DEVLOG.
- The exe was copied to %USERPROFILE%\Downloads\TourneIconsKit.exe.

Found on Tourne's PC (read only):
- Start Menu\Programs\Discord Inc\Discord.lnk: target %LOCALAPPDATA%\Discord\Update.exe, args --processStart Discord.exe, icon %USERPROFILE%\AppData\Local\Discord\app.ico. Written 2026-04-17; untouched by updates since.
- %LOCALAPPDATA%\Discord\app.ico is Discord's stock icon, rewritten at the 2026-10-08 update (app-1.0.9261). There's also a 630 KB Rust Discord.exe launcher in that folder (from Tourne's own tweaks).
- Tourne's live Tourne'Icons list has Discord rows for *\discord.exe, *\discord\discord.exe, *\discord\*.exe, *\discord\app.ico and com.squirrel.Discord.Discord; the app.ico row is what keeps Start's icon in the live mod.
- The pinned taskbar folder has no Discord shortcut.

State:
- Wine-tested only (see DEVLOG). Stock and drive icons need a real Windows check.
- The Kit keeps its own list; the Lasting mod's list isn't carried over. Use one or the other.

Next:
- Tourne opens the Kit (it asks for admin), clicks Apply a pack… and picks Downloads\Tourne icon pack\pack.tsv, then checks Discord in Start, a drive icon and a folder icon.
