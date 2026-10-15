"""Makes tourne-icons-lasting.wh.cpp from tourne-icons.wh.cpp.

The two are one program: the lasting one is the same code with
TOURNE_ICONS_LASTING 1, its own header and readme
(tourne-icons-lasting.header.txt), and the live version's settings with the
wording changed where it works differently (OVERRIDES below), so the same
settings paste into both.
Run it after changing tourne-icons.wh.cpp:  python3 tools/icons/make-lasting.py
"""
import pathlib
import re

here = pathlib.Path(__file__).resolve().parent
root = here.parent.parent
source = (root / "tourne-icons.wh.cpp").read_text(encoding="utf-8")
header = (here / "tourne-icons-lasting.header.txt").read_text(encoding="utf-8")

# Setting key: (new $name or None, new $description or None).
OVERRIDES = {
    "trayIcon": (
        "Tourne'Icons Lasting in the tray",
        "Click it to open the icon picker; right-click it for Refresh icons. Your "
        "changes stay with it off, but the picker can't be opened then.",
    ),
    "swapTrayIcons": (
        None,
        "Only Tourne'Icons (the live version) can change tray icons, so this does "
        "nothing here. It's kept so the same settings paste into both.",
    ),
    "iconTheme": (
        None,
        "A theme from the community icon theme collection. It's downloaded once, "
        "the first time you pick it, and written into shortcuts, file types and "
        "Windows' icons, where it stays with the mod off.",
    ),
    "brandingImages": (
        None,
        "Only Tourne'Icons (the live version) can change the Windows logo, so this "
        "does nothing here. It's kept so the same settings paste into both.",
    ),
    "redirectionResourcePaths": (
        None,
        "Written into every shortcut, file type and Windows icon (folders, drives, "
        "This PC and the like) whose icon comes from the original file, so they "
        "stay with the mod off. What Windows draws straight from a file while it "
        "runs (the tray's volume and network icons, programs' own windows) needs "
        "Tourne'Icons (live). Later rows win; changes made in the picker win over "
        "these.",
    ),
    "stubbornRedirects": (
        None,
        "For icons that keep coming back, like a shortcut an app remakes each "
        "time it updates. A row catches every shortcut whose program, own file "
        "or icon file it names, and gives it the new icon; it wins over the "
        "picker. While the mod runs, those shortcuts are watched and fixed a few "
        "seconds after anything rewrites them. Take a row out to put its "
        "shortcuts back.",
    ),
    "watchShortcuts": (
        None,
        "Off: icons go back on remade shortcuts when the mod next starts (and "
        "stubborn redirects are watched anyway). On: every change you made in "
        "the picker is watched too, and put back a few seconds after an app "
        "remakes its shortcut. Nothing runs between changes.",
    ),
    "allResourceRedirect": (
        None,
        "Only Tourne'Icons (the live version) replaces menus and dialogs, so this "
        "does nothing here. It's kept so the same settings paste into both.",
    ),
}


def wrap(text, indent):
    lines, line = [], ""
    for word in text.split():
        if line and len(indent) + len(line) + 1 + len(word) > 78:
            lines.append(indent + line)
            line = word
        else:
            line = f"{line} {word}" if line else word
    lines.append(indent + line)
    return "\n".join(lines) + "\n"


start = "// ==WindhawkModSettings==\n"
end = "// ==/WindhawkModSettings==\n"
settings = source[source.index(start) : source.index(end) + len(end)]
items = re.split(r"(?m)^(?=- )", settings)
done = set()
for i, item in enumerate(items):
    m = re.match(r"- (\w+):", item)
    if not m or m.group(1) not in OVERRIDES:
        continue
    name, description = OVERRIDES[m.group(1)]
    if name:
        item, n = re.subn(r"(?m)^  \$name: .*\n", f"  $name: {name}\n", item, count=1)
        assert n == 1, m.group(1)
    if description:
        item, n = re.subn(
            r"(?m)^  \$description: >-\n(?:    .*\n)+",
            lambda _: "  $description: >-\n" + wrap(description, "    "),
            item,
            count=1,
        )
        assert n == 1, m.group(1)
    items[i] = item
    done.add(m.group(1))
assert done == set(OVERRIDES), set(OVERRIDES) - done
settings = "".join(items)

body = source[source.index(end) + len(end) :]
first_include = body.index("#include")
body = body[:first_include] + "#define TOURNE_ICONS_LASTING 1\n\n" + body[first_include:]
out = root / "tourne-icons-lasting.wh.cpp"
out.write_text(header + settings + body, encoding="utf-8", newline="\n")
print(f"wrote {out.name}")
