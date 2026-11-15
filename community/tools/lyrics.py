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
           "out.scale": (0, 10), "react.beat": (0, 1), "react.wave": (0, 100), "glow": (0, 1), "glyph.speed": (0.1, 10),
           "in.turn": (-3600, 3600), "out.turn": (-3600, 3600), "in.sx": (0, 10), "in.sy": (0, 10), "out.sx": (0, 10), "out.sy": (0, 10)}
WORDS = {"unit": {"line", "word", "letter"}, "in.ease": {"linear", "out", "inout", "back"}, "out.ease": {"linear", "out", "inout", "back"},
         "karaoke": {"on", "off"}, "engine": {"ours", "web"}, "karaoke.step": {"sweep", "word", "current"},
         "glyph": {"none", "flip", "decode", "bonsaidrop", "ripple", "split", "type", "focus", "flap", "prompt"}}
REACHES_OUT = re.compile(r"url\(|@import|expression\(|javascript:|<\s*/?\s*script|</", re.I)
# A pen's markup kept for the web engine ("#< " lines): plain structure only, as the Table's cleaner keeps it.
TPL_BAD = re.compile(r"<\s*(script|style|iframe|frame|object|embed|svg|math|img|picture|video|audio|source|link|meta|base|form|input|"
                     r"button|select|textarea|canvas|template|portal|dialog)\b|\s(src|href|action|formaction|srcset|poster|xlink:href)\s*=|"
                     r"\son[a-z]+\s*=|javascript:|url\(|@import|expression\(", re.I)
TPL_KEYS = {"tpl.holder": re.compile(r"[0-9]+(/[0-9]+){0,30}"), "tpl.unit": re.compile(r"text|word|letter"),
            "tpl.attrs": re.compile(r"data-[a-z0-9-]{1,30}(,\s*data-[a-z0-9-]{1,30}){0,5}")}
# Keys with a shape of their own: in-between stops (at x y width height turn opacity), and colours (hex only here).
_NUM = r"-?[0-9]+(\.[0-9]+)?"
_HEX = r"#([0-9a-f]{6}|[0-9a-f]{8})"
SHAPED = {"in.stop": re.compile(rf"{_NUM}(\s+{_NUM}){{0,6}}"), "out.stop": re.compile(rf"{_NUM}(\s+{_NUM}){{0,6}}"),
          "karaoke.colors": re.compile(rf"{_HEX}(\s*,\s*{_HEX}){{0,11}}"), "karaoke.base": re.compile(_HEX)}
# A comment starts at the first # that isn't a colour (#rrggbb or #rrggbbaa), as the Table reads it.
COMMENT = re.compile(r"#(?![0-9a-fA-F]{6}(?:[0-9a-fA-F]{2})?(?:$|[\s,]))")
# The sharer's Lyrics look (look.<key> lines, the Table's lyrics.<key> settings): how the lines look, nothing tied to
# a PC. The same list is in the Table's editor (kLeLookKeys) and the gallery's page. Each value is checked by its kind
# in the one settings list.
LOOK_KEYS = ["color", "colorWaiting", "colorOther", "font", "size", "align", "before", "after", "shadow", "shadowColor", "shadowX",
             "shadowY", "shadowSoftness", "glow", "glowRadius", "bloom", "bloomRadius", "outline", "outlineColor", "highlight",
             "highlightColor", "background", "backgroundColor", "backgroundPadding", "backgroundCornerRadius", "borderSize", "borderColor"]
LOOK_NOTE = "# The sharer's Lyrics look (the gallery's preview uses it; Take its look too puts it on yours)"
_LIST = os.path.join(os.path.dirname(COMMUNITY), "docs", "settings-list.json")


def look_kinds():
    """{key: (kind, options)} for the look keys, from the settings list."""
    rows = {r["key"]: r for r in json.load(open(_LIST, encoding="utf-8"))["settings"]}
    out = {}
    for k in LOOK_KEYS:
        r = rows.get("lyrics." + k)
        if r and r.get("shared"):
            out[k.lower()] = (k, r["kind"], [o[0] for o in r.get("options", [])])
    return out


def look_value(kind, options, v):
    """The value as the Table takes it, or None."""
    if kind == "colour":
        return v.upper() if re.fullmatch(r"#[0-9A-Fa-f]{8}|#[0-9A-Fa-f]{6}", v) else None
    if kind == "number":
        try:
            f = float(v)
        except ValueError:
            return None
        return f"{min(1000.0, max(-1000.0, f)):g}"
    if kind == "switch":
        return v if v in ("0", "1") else None
    if kind == "choice":
        return v if v in options else None
    if kind == "text":
        return v if re.fullmatch(r"[^\\/:%<>\r\n]{1,64}", v) else None
    return None


# Scripts and the libraries they load stay on the PC they were imported on: never shared.
SCRIPTED = re.compile(r"(?m)^(#\$|\s*lib\s*=|\s*script\s*=)")


def clean(text):
    """The animation as the community keeps it, and notes on what was left out."""
    keys, css, tpl, source, notes, name = [], [], [], "", [], ""
    look, kinds = [], look_kinds()
    for raw in text.replace("\r", "").split("\n"):
        if raw.startswith("#< "):
            if TPL_BAD.search(raw[3:]):
                notes.append("a markup line that could reach online or run code")
                continue
            tpl.append(raw[3:])
            continue
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
        line = COMMENT.split(raw, 1)[0]
        if "=" not in line:
            continue
        k, v = (x.strip() for x in line.split("=", 1))
        k = k.lower()
        if k == "name":
            name = v[:60]
            continue
        if k.startswith("look."):
            spec = kinds.get(k[5:])
            good = spec and look_value(spec[1], spec[2], v)
            if good:
                look.append(f"look.{spec[0]} = {good}")
            else:
                notes.append(f"`{k[:30]}` (not part of the Lyrics look the Table shares)" if not spec else f"`{k} = {v[:30]}` (not a value the Table takes)")
            continue
        if k == "karaoke.colours":
            k = "karaoke.colors"
        if k in SHAPED:
            if SHAPED[k].fullmatch(v.lower()):
                keys.append(f"{k} = {v.lower() if k.startswith('karaoke') else v}")
            else:
                notes.append(f"`{k} = {v[:30]}` (not in the shape the Table reads)")
            continue
        if k in TPL_KEYS:
            if TPL_KEYS[k].fullmatch(v):
                keys.append(f"{k} = {v}")
            else:
                notes.append(f"`{k} = {v[:20]}` (not one the Table knows)")
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
    elif SCRIPTED.search(text):
        refused = "It carries scripts or the libraries they load, and those stay on your own PC: they're never shared."
    elif css_text and not source:
        notes.append("its CSS has no `# css source:` line (where it came from and its licence); added as unknown")
        source = "not given"
    out = (f"name = {name}\n" if name else "") + "\n".join(keys) + "\n"
    if look:
        out += LOOK_NOTE + "\n" + "\n".join(look) + "\n"
    if css_text:
        out += f"# css source: {source}\n" + "".join(f"#| {l}\n" if l else "#|\n" for l in css_text.split("\n"))
        out += "".join(f"#< {l}\n" for l in tpl if l.strip())
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
        plain = "\n".join(l for l in text.split("\n") if not l.startswith("#< "))
        if refused or not any_ or REACHES_OUT.search(plain):
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
