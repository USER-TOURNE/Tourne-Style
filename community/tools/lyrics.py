#!/usr/bin/env python3
"""Tourne'Table community lyrics animations: checks them and takes them in from GitHub issues.

    python community/tools/lyrics.py check            # every animation in community/lyrics is valid
    python community/tools/lyrics.py issue  OUT.json  # read the "Lyrics: ..." issue in $GITHUB_EVENT_PATH, say what it holds
    python community/tools/lyrics.py add    OUT.json  # add the issue's animation to lyrics/ and the index

An animation is the text the Animation editor writes (.ttlyrics): key = value lines (the keys the Table reads,
nothing else is kept), comments, and for a web one its CSS in "#| " lines with a "# css source:" line. Scripts and
anything that reaches online (url(), @import, expression(), javascript:) never come in; CSS from Animate.css (whose
licence doesn't allow it here) is refused.
"""
import json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
COMMUNITY = os.path.dirname(HERE)
LYRICS = os.path.join(COMMUNITY, "lyrics")
INDEX = os.path.join(LYRICS, "index.txt")

# The keys the Table's VizParseLyricAnim reads: number ranges, or the words allowed.
NUMBERS = {"in.ms": (1, 5000), "in.stagger": (0, 2000), "in.opacity": (0, 1), "in.x": (-2000, 2000), "in.y": (-2000, 2000),
           "in.scale": (0, 10), "out.ms": (1, 5000), "out.opacity": (0, 1), "out.x": (-2000, 2000), "out.y": (-2000, 2000),
           "out.scale": (0, 10), "react.beat": (0, 1), "react.wave": (0, 100), "glow": (0, 1), "glyph.speed": (0.1, 10)}
WORDS = {"unit": {"line", "word", "letter"}, "in.ease": {"linear", "out", "inout", "back"}, "out.ease": {"linear", "out", "inout", "back"},
         "karaoke": {"on", "off"}, "engine": {"ours", "web"},
         "glyph": {"none", "flip", "decode", "bonsaidrop", "ripple", "split", "type", "focus", "flap", "prompt"}}
REACHES_OUT = re.compile(r"url\(|@import|expression\(|javascript:|<\s*/?\s*script|</", re.I)


def clean(text):
    """The animation as the community keeps it, and notes on what was left out."""
    keys, css, source, notes, name = [], [], "", [], ""
    for raw in text.replace("\r", "").split("\n"):
        if raw.startswith("#|"):
            line = raw[3:] if raw.startswith("#| ") else ""
            if REACHES_OUT.search(line):
                notes.append("a CSS line that could reach online or run code")
                continue
            css.append(line)
            continue
        if raw.lower().startswith("# css source:"):
            source = raw.split(":", 1)[1].strip()[:200]
            continue
        line = raw.split("#", 1)[0]
        if "=" not in line:
            continue
        k, v = (x.strip() for x in line.split("=", 1))
        k = k.lower()
        if k == "name":
            name = v[:60]
            continue
        if k in NUMBERS:
            try:
                f = float(v)
            except ValueError:
                notes.append(f"`{k}` (not a number)")
                continue
            lo, hi = NUMBERS[k]
            f = min(hi, max(lo, f))
            keys.append(f"{k} = {f:g}")
        elif k in WORDS:
            if v.lower() in WORDS[k]:
                keys.append(f"{k} = {v.lower()}")
            else:
                notes.append(f"`{k} = {v[:20]}` (not one the Table knows)")
        else:
            notes.append(f"`{k[:30]}` (not a key the Table reads)")
    css_text = "\n".join(css).strip("\n")
    refused = None
    if re.search(r"animate\.css|animate\.style", text, re.I):
        refused = "Its CSS came from Animate.css, whose licence doesn't allow sharing it here."
    elif css_text and not source:
        notes.append("its CSS has no `# css source:` line (where it came from and its licence); added as unknown")
        source = "not given"
    out = (f"name = {name}\n" if name else "") + "\n".join(keys) + "\n"
    if css_text:
        out += f"# css source: {source}\n" + "".join(f"#| {l}\n" if l else "#|\n" for l in css_text.split("\n"))
    return out, notes, refused, bool(keys or css_text)


def slug(name):
    s = re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")
    return (s or "animation")[:40]


def read_index():
    rows = []
    for line in open(INDEX, encoding="utf-8").read().splitlines():
        if not line or line.startswith("#"):
            continue
        f = line.split("|")
        rows.append({"file": f[0], "name": f[1] if len(f) > 1 else "", "author": f[2] if len(f) > 2 else "", "about": "|".join(f[3:])})
    return rows


def cmd_check():
    bad = 0
    for r in read_index():
        p = os.path.join(LYRICS, r["file"])
        if not re.fullmatch(r"[A-Za-z0-9_.-]+\.ttlyrics", r["file"]) or not os.path.exists(p):
            print(f"{r['file']}: missing or a bad name"); bad += 1; continue
        text = open(p, encoding="utf-8").read()
        out, notes, refused, any_ = clean(text)
        if refused or not any_ or REACHES_OUT.search(text):
            print(f"{r['file']}: {refused or 'nothing the Table reads, or a way online'}"); bad += 1
        elif notes:
            print(f"{r['file']}: would lose {', '.join(notes)}"); bad += 1
    print("ok" if not bad else f"{bad} to fix")
    return 1 if bad else 0


def issue_text():
    ev = json.load(open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8"))
    issue = ev["issue"]
    body = issue.get("body") or ""
    m = re.search(r"```(?:ttlyrics)?\s*\n(.*?)```", body, re.S)
    text = m.group(1) if m else None
    about = ""
    am = re.search(r"(?im)^about:\s*(.+)$", body)
    if am and "one line about" not in am.group(1):
        about = am.group(1).replace("|", "/")[:200]
    title = re.sub(r"^lyrics:\s*", "", (issue.get("title") or "").strip(), flags=re.I)[:60].strip()
    return issue, title, issue["user"]["login"], text, about


def cmd_issue(out):
    issue, title, login, text, about = issue_text()
    result = {"ok": False}
    if not text:
        result["comment"] = ("I couldn't find an animation in this issue. Paste the text of your `.ttlyrics` file between ``` lines, or use "
                             "**Community > Share mine** in Tourne'Table's Animation editor.")
    else:
        c, notes, refused, any_ = clean(text)
        if refused:
            result["comment"] = f"Thanks, @{login}! {refused} Use **Convert to ours** and take its `#|` lines out, then share it again."
        elif not any_:
            result["comment"] = "That text has none of the keys a Tourne'Table lyrics animation uses."
        else:
            result.update(ok=True)
            msg = [f"Thanks, @{login}! **{title or 'Untitled'}** reads fine" + (" (a web animation: its CSS runs in the Table's web engine)." if "engine = web" in c else ".")]
            if notes:
                msg.append("\nLeft out:\n" + "\n".join(f"- {n}" for n in notes[:20]))
            msg.append("\nTry it now: copy the text below, then **Community > Take the one I copied there** in the Animation editor.\n\n```\n" + c + "```")
            msg.append("\nA maintainer adds it to the gallery's Lyrics page with the `approved` label. An `about: one line` line in this issue becomes its description.")
            result["comment"] = "\n".join(msg)
    json.dump(result, open(out, "w", encoding="utf-8"))
    print(result["comment"])
    return 0


def cmd_add(out):
    issue, title, login, text, about = issue_text()
    c, notes, refused, any_ = clean(text or "")
    if refused or not any_:
        json.dump({"ok": False, "comment": "Nothing to add: " + (refused or "this issue has no readable animation.")}, open(out, "w", encoding="utf-8"))
        return 0
    name = title or "Untitled"
    c = re.sub(r"(?m)^name = .*\n", "", c)
    c = f"name = {name}\n" + c
    taken = {r["file"] for r in read_index()} | set(os.listdir(LYRICS))
    base, n = slug(name), 1
    file = base + ".ttlyrics"
    while file in taken:
        n += 1
        file = f"{base}-{n}.ttlyrics"
    open(os.path.join(LYRICS, file), "w", encoding="utf-8", newline="\n").write(c)
    with open(INDEX, "a", encoding="utf-8", newline="\n") as f:
        f.write(f"{file}|{name.replace('|', '/')}|{login}|{about or 'Shared in #' + str(issue['number'])}\n")
    repo = os.environ.get("GITHUB_REPOSITORY", "USER-TOURNE/Tourne-Style")
    json.dump({"ok": True, "file": file,
               "comment": f"Added as [`{file}`](https://github.com/{repo}/blob/main/community/lyrics/{file}). It's on the gallery's Lyrics page now. Thank you!"},
              open(out, "w", encoding="utf-8"))
    return 0


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    if cmd == "check":
        sys.exit(cmd_check())
    if cmd == "issue":
        sys.exit(cmd_issue(sys.argv[2]))
    if cmd == "add":
        sys.exit(cmd_add(sys.argv[2]))
    sys.exit(__doc__)
