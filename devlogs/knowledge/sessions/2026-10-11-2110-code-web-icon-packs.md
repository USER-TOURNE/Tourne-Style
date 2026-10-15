# Tourne'Icons packs, and Tourne's own icon set

When: 2026-10-11 21:10 UTC, code-web, branch claude/project-thread-0rkfwp at ad94e31.

Why:
- Queued from earlier: "Save lists" and "Apply a pack" in Tourne'Icons, then one custom icon per picker entry in Tourne's red and green pixel style (their Icons folder: %USERPROFILE%\Downloads\tourne-suite-sab-clone\Icons). The icons are for Tourne's PC only, never repo defaults.

What changed:
- Tourne'Icons 2.2 / Lasting 1.2: Save lists and Apply a pack: ad94e31 (DEVLOG has the pack.tsv format).
- Pack maker (not in the repo, project files): /mnt/project-files/tourne-icons/pack-maker/
  - render.py: 16x16 glyphs, 1 px line art; each lit pixel casts a shadow one pixel down and right. Multi-size .ico at whole-pixel scales (24 px = the glyph at 1x with 4 px padding, like Tourne's own).
  - glyphs.py: ~80 drawings. filetypes.py: a page with a 3x5 red label, and a lettered tile used for any app without a drawing.
  - build.py <saved lists folder> <out>: picks Tourne's own drawing (Claude, Discord, GitHub Desktop, NVIDIA, Process Lasso, Snipping Tool, Tamer), else imageres id, else name words, else the tile. Writes icons/, pack.tsv and preview.png.
  - Palette: soft green #64aa89 / shadow #3d6657 for Windows things, bright #66ea97 / #3c865c for apps, red #ff6f77 / #a95d5d accents.

State:
- Wine: Save lists and Apply a pack work; a built pack applied 17 of 17 and the saved icons came out pixel-identical at 24 and 256.
- Tourne installed 2.2 and ran Save lists (Downloads\Tourne'Icons lists 2026-10-11 173755: 1,049 entries).
- The pack is built and on the PC: %USERPROFILE%\Downloads\Tourne icon pack (pack.tsv, preview.png, 483 drawings for 949 entries). Windows icons were sorted by eye into build.py WINDOWS_ICONS; overlays, blanks and the ones already in Tourne's pixel style (81, 84, 86-88, 98, 105-107) are left alone. Under Wine the full pack applied 916 of 949 in 20 s (Store apps can't apply in Wine). Not yet applied on the PC.

Learned:
- device_commit_files right after writing a file in /mnt/project-files can copy the OLD version (sync lag). Re-stage the device file and check its size before telling Tourne it's there.
- In this sandbox, `pkill -f <pattern>` and `pgrep -f` match the bash running them; don't put the pattern on the same command line.
- Wine's ShellExecute of a folder keeps a pipe open, so pipe a Wine run to a file, not to `tail`.

Next:
- Tourne clicks Apply a pack and opens Downloads\Tourne icon pack\pack.tsv, then says which drawings to redo.
- To change one: edit glyphs.py or the WORDS/WINDOWS_ICONS maps, rerun build.py, copy the changed files over.
