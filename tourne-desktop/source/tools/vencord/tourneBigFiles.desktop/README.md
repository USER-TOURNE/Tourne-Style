# Tourne Big Files

A Vencord (and Testcord) plugin for sending files over Discord's upload limit without Nitro.

**Right-click the upload (+) button**, pick **Send a big file (in pieces)**, choose the file.

- The file is cut into pieces that fit (9.5 MB by default; Discord's limit without Nitro is 10 MB) and each piece goes as an ordinary, silent message. In a server they go in a thread, so the channel only gets the file's card. In DMs they go in the chat.
- The last message is the card: `name.tbf.json` holds the list of pieces with a SHA-256 for each piece and for the whole file.
- **With the plugin**, the card offers **Play here** (videos, music, pictures up to 300 MB by default), **Save...** (written straight to disk piece by piece, each piece checked, then the whole file), then **Open** and **Show in folder**. Programs and scripts (`.exe`, `.bat`, `.ps1`, ...) get Show in folder only. The pieces' attachments are hidden.
- **Without it**, people see `name.001`, `name.002`, ... and a line on the card: save every piece in one folder, open the `.001` in 7-Zip and pick **Combine files** (or `copy /b`).
- A send can be stopped from the window (Stop) or hidden and opened again from the + menu.
- Discord's file links expire after a while, so the card reads the pieces' messages again to get current links each time.

**Send as a link** (off by default): uploads to Catbox (up to 200 MB, kept), Litterbox (up to 1 GB, 1 to 72 hours) or your own ShareX uploader (import its `.sxcu` in the plugin settings), then puts the link in the message box or sends it. The file leaves Discord.

## Files

| File | What it does |
|---|---|
| `index.tsx` | Menu items, the send window, the card, settings, and the patch that hides piece attachments. |
| `native.ts` | Runs in Discord's main process: file picker, reading pieces, SHA-256, downloading from Discord's CDN only, saving, file hosts. |
| `styles.css` | The card and window. |

The folder name ends in `.desktop` so it only builds into the Discord desktop app (it needs `native.ts`).

## Install

Vencord's installer can't add your own plugins; they need a copy built from source. From the repo:

```powershell
.\tools\vencord\Install-TourneBigFiles.ps1                     # Vencord
.\tools\vencord\Install-TourneBigFiles.ps1 -Client Testcord    # Testcord
```

It clones the client into `%LOCALAPPDATA%\Tourne\<client> source` (or the `-Folder` you give) (or updates it), copies this folder into `src\userplugins\`, builds, and runs the installer so you can pick your Discord. Then restart Discord and turn on **TourneBigFiles** in the plugin list.

Checked: builds in Vencord (2026-10-09) and Testcord (main, 2026-10-12) with TypeScript clean; `native.ts` tested on Node 22 (pieces join to the same bytes, hashes, a bad piece is refused, other hosts refused, unsafe save names cleaned). Not yet run inside Discord.
