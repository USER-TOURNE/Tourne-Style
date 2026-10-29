"""Makes a suite build safe to publish: in its settings block, any value naming my PC (my user folders, my other
drives, my e-mail) is blanked. A list row that names my PC goes, except a list's first row, which stays with those
values blanked, so the list keeps its shape. The code itself is left alone.

    python tools/publish/scrub-suite.py builds/tourne-suite-b65.wh.cpp <out.wh.cpp>
"""
import re
import sys

PERSONAL_VALUE = re.compile(r"[A-Za-z]:\\Users\\|\b[D-Zd-z]:\\|@gmail\.com")


class PERSONAL:
    """A setting's value naming my PC ($name, $description and the like are text, and stay)."""
    @staticmethod
    def search(line):
        return not re.match(r"^\s*(?:- )*\$", line) and PERSONAL_VALUE.search(line)
src, out = sys.argv[1], sys.argv[2]
lines = open(src, encoding="utf-8", newline="").read().split("\n")
begin = next(i for i, l in enumerate(lines) if "==WindhawkModSettings==" in l)
end = next(i for i, l in enumerate(lines) if "==/WindhawkModSettings==" in l)


def blank(line):
    # key: 'value' or key: ['a', 'b'] naming my PC: the value emptied
    m = re.match(r"^(\s*(?:- )*[\w.]+: )(.*?)(\r?)$", line)
    if m and PERSONAL.search(m.group(2)):
        return m.group(1) + ("['']" if m.group(2).startswith("[") else "''") + m.group(3)
    return line


body = lines[begin + 1:end]
kept = []
i = 0
while i < len(body):
    row = re.match(r"^(\s*)- - ", body[i])
    if not row:
        kept.append(blank(body[i]))
        i += 1
        continue
    # One list row: its first line and the lines indented as its other fields.
    ind = row.group(1)
    j = i + 1
    while j < len(body) and body[j].startswith(ind + "  - ") and not body[j].startswith(ind + "  - - "):
        j += 1
    rows = body[i:j]
    first = not kept or not (kept[-1].startswith(ind + "- - ") or kept[-1].startswith(ind + "  - "))
    if any(PERSONAL.search(r) for r in rows):
        if first:
            kept.extend(blank(r) for r in rows)
    else:
        kept.extend(rows)
    i = j

left = [l for l in kept if PERSONAL.search(l)]
if left:
    sys.exit("still personal:\n" + "\n".join(left[:10]))
open(out, "w", encoding="utf-8", newline="").write("\n".join(lines[:begin + 1] + kept + lines[end:]))
print(f"wrote {out}: settings {len(body)} -> {len(kept)} lines")
