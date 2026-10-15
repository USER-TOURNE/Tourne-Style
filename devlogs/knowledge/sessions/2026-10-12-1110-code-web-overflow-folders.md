# 2026-10-12 11:10, Claude Code (cloud): overflow folders and auto sort

- Tourne'Menus installed on the PC once `windhawk-cli mod install` ran in an admin PowerShell (a normal window installed nothing, silently). Tourne confirmed it works.
- Big Files installer fixed (bfa3780): pnpm through `npm install -g --allow-scripts=pnpm pnpm` (corepack needs admin), Vencord copy in `%LOCALAPPDATA%\Tourne\<client> source` (Tourne's Documents points at a missing OneDrive folder). Tourne's LOCALAPPDATA is `%USERPROFILE%\AppData\Local`; their Discord is at `%USERPROFILE%\AppData\Local\Discord` (picked via Custom Location).
- Hidden icons flyout menu: New folder in the overflow (picker that comes back after each pick, Rename, Done), folder menu "Pick its icons", Auto sort (kind table, then maker), undo. Empty folders made here now show.
- Shell mode tray: icons with no tooltip get the program's name (Claude's had none).
- Next: Tourne swaps in this commit and tries it; Null.bin moving still untested.
