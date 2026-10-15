# Tourne Notes

Markdown notes in a side panel, for Tourne Desktop. Built into Tourne Desktop:
install it from the Studio's Extensions page like any plugin.

Started from Noctalia's Notes (noctalia-dev/official-plugins, `notes`, MIT
licence, by the Noctalia authors), and it keeps its notes the same way: plain
files in the notes folder (`~/Documents/Notes` unless you change it), so notes
made with Noctalia's Notes show here too.

What it adds:

- A name typed in the name box is kept when you leave the note (Back, another
  note, a new one, closing the panel), not only when you press Enter. Characters
  Windows doesn't allow in file names are swapped for `-`.
- A new note takes its first line as its name until you name it yourself.
- Ctrl+V with a picture copied saves it as a PNG in `images\` beside the notes
  and puts it in the note where the caret is. Files copied in File Explorer and
  pasted, or dropped on the panel, are copied into `images\` (pictures) or
  `files\` (anything else) and linked. Dropping note files (`.md`, `.txt`) on
  the list adds them as notes.
- The eye button shows the note formatted: headings, lists, quotes, code,
  pictures, links (click one to open it), and checklist lines (`- [ ]`) that
  tick off when clicked.
- Search across every note's name and text, from the list.
- Deleting moves a note to `.trash` in the notes folder; the trash button in
  the list shows it, where a note can be put back or deleted for good.
- Settings: the editor's font (pixel, Consolas or Segoe UI) and text size, and
  a hotkey that opens and closes the panel (none until you set one).
- Buttons to open the note in Notepad and to open the notes folder.
