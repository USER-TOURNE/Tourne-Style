#!/usr/bin/env python3
"""Tourne'Table community setups: checks them, takes them in from GitHub issues, and writes the gallery.

    python community/tools/community.py check            # every setup in community/setups is valid
    python community/tools/community.py gallery          # rewrite the gallery in community/README.md
    python community/tools/community.py issue  OUT.json  # read the issue in $GITHUB_EVENT_PATH, say what it holds
    python community/tools/community.py add     OUT.json  # add the issue's setup to setups/ and the index

A setup is the text Tourne'Table's Style Editor writes (Save file / Share Mine): a first line
"Tourne'Table setup 1", then name=, author=, parts=, then setting=value lines. Only settings the mod
itself shares are kept (its kSetupKeys and quick settings, read from the mod's source), so audio
devices, hotkeys, paths and hardware choices can never come in through here.
"""
import base64, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
COMMUNITY = os.path.dirname(HERE)
ROOT = os.path.dirname(COMMUNITY)
SETUPS = os.path.join(COMMUNITY, "setups")
INDEX = os.path.join(SETUPS, "index.txt")
MOD = os.path.join(ROOT, "windhawk-mods", "tourne-table", "tourne-table.wh.cpp")
README = os.path.join(COMMUNITY, "README.md")
PARTS = {"look": 1, "motion": 2, "extras": 4, "place": 8}
PART_NAMES = {v: k for k, v in PARTS.items()}
PART_CONST = {"kPartLook": 1, "kPartMotion": 2, "kPartExtras": 4, "kPartPlace": 8}


def load_keys():
    src = open(MOD, encoding="utf-8").read()
    table = src[src.index("static const VizSetupKey kSetupKeys[]"):]
    table = table[:table.index("};")]
    keys = {m.group(1): (m.group(2), int(m.group(3)))
            for m in re.finditer(r'\{L"([^"]+)", L"([^"]*)", (?:true|false), (\d+)\}', table)}
    quick_t = src[src.index("static const VizQuickPart kQuickParts[]"):]
    quick_t = quick_t[:quick_t.index("};")]
    quick = {m.group(1): PART_CONST[m.group(2)] for m in re.finditer(r'\{L"([^"]+)", (kPart\w+)\}', quick_t)}
    if len(keys) < 100 or len(quick) < 10:
        sys.exit("couldn't read the setting list from the mod's source")
    return keys, quick


def parse(text):
    """Mirrors the mod's VizParseSetup: None when it isn't a setup."""
    text = text.strip()
    for prefix in ("TT2T:",):
        if text.startswith(prefix):
            try:
                text = base64.b64decode(text[len(prefix):]).decode("utf-8")
            except Exception:
                return None
    started, s = False, {"name": "", "author": "", "parts": 0, "values": []}
    for line in text.splitlines():
        line = line.strip()
        if not started:
            started = line.startswith("Tourne'Table setup")
            continue
        if not line or line.startswith("#") or line.startswith("```"):
            continue
        if "=" not in line or line.index("=") == 0:
            continue
        k, v = line.split("=", 1)
        v = v[:1024]
        if k == "name":
            s["name"] = v[:60]
        elif k == "author":
            s["author"] = v[:40]
        elif k == "parts":
            s["parts"] = sum(PARTS.get(p.strip().lower(), 0) for p in v.split(","))
        elif len(s["values"]) < 2000:
            s["values"].append((k, v))
    if not started:
        return None
    if not s["parts"]:
        s["parts"] = PARTS["look"]
    return s


PATHISH = re.compile(r"(^[a-zA-Z]:[\\/])|(\\\\)|(^/)|(%[A-Za-z]+%)")


def clean(s, keys, quick):
    """Keeps what the mod would share and drops the rest; returns (setup, notes)."""
    notes, values, seen = [], [], set()
    for k, v in s["values"]:
        part = 0
        if k.startswith("quick."):
            part = quick.get(k[6:], 0)
        elif k in keys:
            part = keys[k][1]
        if not part:
            notes.append(f"`{k}` isn't a shared setting, left out")
            continue
        if not part & s["parts"]:
            continue
        if PATHISH.search(v) or "\n" in v:
            notes.append(f"`{k}` looks like a file path, left out")
            continue
        if k in seen:
            continue
        seen.add(k)
        values.append((k, v))
    name = re.sub(r"[|\r\n]", " ", s["name"]).strip()
    author = re.sub(r"[|\r\n]", " ", s["author"]).strip()
    return {"name": name, "author": author, "parts": s["parts"], "values": values}, notes


def text_of(s):
    parts = ",".join(PART_NAMES[b] for b in (1, 2, 4, 8) if s["parts"] & b)
    lines = ["Tourne'Table setup 1", f"name={s['name']}", f"author={s['author']}", f"parts={parts}"]
    lines += [f"{k}={v}" for k, v in s["values"]]
    return "\n".join(lines) + "\n"


def code_of(text):
    return "TT2T:" + base64.b64encode(text.encode("utf-8")).decode("ascii")


def slug(name):
    s = re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")[:40]
    return s or "setup"


def read_index():
    rows = []
    if os.path.exists(INDEX):
        for line in open(INDEX, encoding="utf-8"):
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            f = line.split("|")
            if len(f) >= 4:
                rows.append({"file": f[0], "name": f[1], "author": f[2], "about": "|".join(f[3:])})
    return rows


def write_index_line(file, name, author, about):
    about = re.sub(r"[|\r\n]", " ", about).strip()[:140]
    with open(INDEX, "a", encoding="utf-8", newline="\n") as f:
        f.write(f"{file}|{name}|{author}|{about}\n")


def cmd_check():
    keys, quick = load_keys()
    bad = 0
    rows = read_index()
    for r in rows:
        p = os.path.join(SETUPS, r["file"])
        if not re.fullmatch(r"[A-Za-z0-9_.-]+\.ttstyle", r["file"]) or not os.path.exists(p):
            print(f"{r['file']}: missing or badly named"); bad += 1; continue
        s = parse(open(p, encoding="utf-8").read())
        if not s:
            print(f"{r['file']}: not a Tourne'Table setup"); bad += 1; continue
        c, notes = clean(s, keys, quick)
        if notes:
            print(f"{r['file']}: " + "; ".join(n.replace("`", "") for n in notes)); bad += 1
    listed = {r["file"] for r in rows}
    for f in os.listdir(SETUPS):
        if f.endswith(".ttstyle") and f not in listed:
            print(f"{f}: not in index.txt"); bad += 1
    print(f"{len(rows)} setups, {bad} problems")
    return 1 if bad else 0


GALLERY_START, GALLERY_END = "<!-- gallery:start -->", "<!-- gallery:end -->"


def colours_of(s):
    out = []
    for k, v in s["values"]:
        if k in ("appearance.color", "appearance.gradientColor1", "appearance.gradientColor2", "quick.color",
                 "quick.grad1", "quick.grad2") and v not in out:
            out.append(v)
    return out


def cmd_gallery():
    keys, quick = load_keys()
    blocks = []
    for r in read_index():
        p = os.path.join(SETUPS, r["file"])
        s = parse(open(p, encoding="utf-8").read())
        if not s:
            continue
        vals = dict(s["values"])
        shape = vals.get("quick.shape") or vals.get("appearance.shape", keys["appearance.shape"][0])
        parts = ", ".join(PART_NAMES[b].capitalize() for b in (1, 2, 4, 8) if s["parts"] & b)
        colours = colours_of(s)
        blocks.append(
            f"### {r['name']}\n\n"
            f"by **{r['author']}** · {r['about']}\n\n"
            f"Shape `{shape}` · Parts: {parts}" + (f" · Colours: {', '.join('`' + c + '`' for c in colours)}" if colours else "") + "\n\n"
            f"[Open the file](setups/{r['file']}) · copy this code, then **Paste code** in the Style Editor:\n\n"
            f"```\n{code_of(open(p, encoding='utf-8').read())}\n```\n")
    body = "\n".join(blocks) if blocks else "_No setups yet: be the first._\n"
    text = open(README, encoding="utf-8").read()
    a, b = text.index(GALLERY_START) + len(GALLERY_START), text.index(GALLERY_END)
    text = text[:a] + "\n\n" + body + "\n" + text[b:]
    open(README, "w", encoding="utf-8", newline="\n").write(text)
    print(f"gallery: {len(blocks)} setups")
    return 0


def issue_setup():
    ev = json.load(open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8"))
    issue = ev["issue"]
    body = issue.get("body") or ""
    title = (issue.get("title") or "").strip()
    login = issue["user"]["login"]
    # The setup is the text from "Tourne'Table setup" on, or a TT2T: code.
    m = re.search(r"TT2T:[A-Za-z0-9+/=]+", body)
    text = None
    if "Tourne'Table setup" in body:
        text = body[body.index("Tourne'Table setup"):]
        text = text.split("```")[0]
    elif m:
        text = m.group(0)
    about = ""
    am = re.search(r"(?im)^about:\s*(.+)$", body)
    if am and "one line about" not in am.group(1):
        about = am.group(1)
    return issue, title, login, text, about


def cmd_issue(out):
    keys, quick = load_keys()
    issue, title, login, text, about = issue_setup()
    result = {"ok": False, "comment": ""}
    if text is None:
        if "TT2:" in (issue.get("body") or ""):
            result["comment"] = ("Thanks! That's a packed `TT2:` code, which only Tourne'Table can unpack. In the Style Editor press "
                                 "**Save file**, open the `.ttstyle` file in Notepad and paste its text here (or press **Share Mine "
                                 "to the Community**, which puts the text in for you).")
        else:
            result["comment"] = ("I couldn't find a setup in this issue. Paste the text of your `.ttstyle` file (it starts with "
                                 "`Tourne'Table setup 1`), or use **Share Mine to the Community** in Tourne'Table's Style Editor.")
    else:
        s = parse(text)
        if not s or not s["values"]:
            result["comment"] = "That text isn't a Tourne'Table setup I can read (it should start with `Tourne'Table setup 1`)."
        else:
            if not s["name"]:
                s["name"] = re.sub(r"^setup:\s*", "", title, flags=re.I)[:60] or "Untitled"
            if not s["author"]:
                s["author"] = login
            c, notes = clean(s, keys, quick)
            parts = ", ".join(PART_NAMES[b].capitalize() for b in (1, 2, 4, 8) if c["parts"] & b)
            result.update(ok=True, name=c["name"], author=c["author"], text=text_of(c), about=about)
            msg = [f"Thanks, @{login}! **{c['name']}** reads fine: {len(c['values'])} settings across {parts}."]
            if notes:
                msg.append("\nLeft out:\n" + "\n".join(f"- {n}" for n in notes[:20]))
            msg.append("\nTry it now: copy this code and press **Paste code** in the Style Editor.\n\n```\n"
                       + code_of(text_of(c)) + "\n```")
            msg.append("\nA maintainer adds it to the community list with the `approved` label. A screenshot helps people "
                       "pick it, and an `about: one line` line in this issue becomes its description.")
            result["comment"] = "\n".join(msg)
    json.dump(result, open(out, "w", encoding="utf-8"))
    print(result["comment"])
    return 0


def cmd_add(out):
    keys, quick = load_keys()
    issue, title, login, text, about = issue_setup()
    s = parse(text or "")
    if not s or not s["values"]:
        json.dump({"ok": False, "comment": "Nothing to add: this issue has no readable setup."}, open(out, "w"))
        return 0
    if not s["name"]:
        s["name"] = re.sub(r"^setup:\s*", "", title, flags=re.I)[:60] or "Untitled"
    if not s["author"]:
        s["author"] = login
    c, _ = clean(s, keys, quick)
    taken = {r["file"] for r in read_index()} | set(os.listdir(SETUPS))
    base, n = slug(c["name"]), 1
    file = base + ".ttstyle"
    while file in taken:
        n += 1
        file = f"{base}-{n}.ttstyle"
    open(os.path.join(SETUPS, file), "w", encoding="utf-8", newline="\n").write(text_of(c))
    write_index_line(file, c["name"], c["author"], about or f"Shared in #{issue['number']}")
    cmd_gallery()
    json.dump({"ok": True, "file": file, "name": c["name"],
               "comment": f"Added as [`{file}`](https://github.com/{os.environ.get('GITHUB_REPOSITORY', 'USER-TOURNE/Tourne-Style')}"
                          f"/blob/main/community/setups/{file}). It's in everyone's **Community** list now. Thank you!"},
              open(out, "w", encoding="utf-8"))
    return 0


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    if cmd == "check":
        sys.exit(cmd_check())
    if cmd == "gallery":
        sys.exit(cmd_gallery())
    if cmd == "issue":
        sys.exit(cmd_issue(sys.argv[2]))
    if cmd == "add":
        sys.exit(cmd_add(sys.argv[2]))
    sys.exit(__doc__)
