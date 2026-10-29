"""Package rows for app logos the list has no row for (Search and Windows Backup inside Windows' CBS package, Resume),
pictures made the way Tourne'Icons' picker makes them; Get Started (CBS's existing row) back to its own drawing;
Microsoft Defender's Network Inspection Service as a program row. Backup: icons.txt.before-package-logos."""
import glob, os, re, shutil, sys, time
from PIL import Image

sys.path.insert(0, r"%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\302c218a-4e92-4a78-bd95-d817c5319397\scratchpad\ink")
from big import DRAWINGS, Canvas, render, snap, save_ico, file_name  # noqa: E402

ROOT = r"C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-icons"
LIST = ROOT + r"\icons.txt"
CBS = r"c:\windows\systemapps\microsoftwindows.client.cbs_cw5n1h2txyewy"
CROSS = r"c:\program files\windowsapps\microsoftwindows.crossdevice_*_cw5n1h2txyewy"
NEW = [  # (original, name, drawing, logos; + marks a tile)
    (CBS, "Search", "Search", ["cortana.ui/assets/app/square44x44logo", "+cortana.ui/assets/app/square150x150logo",
                               "+cortana.ui/assets/app/wide310x150logo"]),
    (CBS, "Windows Backup", "Windows Backup", ["windowsbackup/assets/applist", "+windowsbackup/assets/medtile",
                                               "+windowsbackup/assets/widetile", "+windowsbackup/assets/largetile",
                                               "+windowsbackup/assets/smalltile"]),
    (CROSS, "Resume", "Resume", ["assets/apptiles/applist", "+assets/apptiles/medtile", "+assets/apptiles/widetile",
                                 "assets/streamingicon44x44logo", "+assets/streamingicon150x150logo",
                                 "+assets/streamingicon310x150logo"]),
]
NEW = [(o, t, d, [l.replace("/", "\\") for l in logos]) for o, t, d, logos in NEW]


def pictures_name(original, ico):
    name = original[original.rfind("\\") + 1:].replace("*", "#")
    h = 2166136261
    for ch in ico.lower():
        h = ((h ^ ord(ch)) * 16777619) & 0xFFFFFFFF
    return "%s-%08x" % (name, h)


def drawn(cv, side):
    if side < 24:
        return snap(cv, side)
    k = side // 24
    im = render(cv).resize((24 * k, 24 * k), Image.NEAREST)
    out = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    out.paste(im, ((side - 24 * k) // 2, (side - 24 * k) // 2))
    return out


def ink_ico(drawing):
    cv = Canvas()
    DRAWINGS[drawing](cv)
    ico = os.path.join(ROOT, "icons", "Ink2-" + file_name(drawing))
    if not os.path.exists(ico):
        save_ico(render(cv), ico, cv)
    return cv, ico


def make(original, logos, cv, ico, sources):
    """sources: (folder, relative) of every logo file to stand in for."""
    name = pictures_name(original, ico)
    out = os.path.join(ROOT, "packages", name)
    tiles = {l[1:] for l in logos if l.startswith("+")}
    made = 0
    for folder, rel in sources:
        dst = os.path.join(out, rel)
        if os.path.exists(dst):
            continue
        try:
            w, h = Image.open(os.path.join(folder, rel)).size
        except Exception:
            continue
        base = os.path.join(os.path.dirname(rel), os.path.basename(rel).split(".")[0]).lower()
        side = min(w, h)
        if base in tiles:
            side = max(1, side * 2 // 3)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        pic = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        pic.paste(drawn(cv, side), ((w - side) // 2, (h - side) // 2))
        pic.save(dst)
        made += 1
    return name, made


def logo_files(original, logos):
    folders = glob.glob(original.replace("*", "*"), recursive=False) if "*" in original else [original]
    found = []
    for folder in folders:
        for logo in logos:
            base = logo.lstrip("+")
            d, b = os.path.split(base)
            for f in glob.glob(os.path.join(folder, d, "**", b + ".*png"), recursive=True) + \
                     glob.glob(os.path.join(folder, d, "**", b + ".png"), recursive=True):
                found.append((folder, os.path.relpath(f, folder).lower()))
    return sorted(set(found))


lines = open(LIST, encoding="utf-16").read().splitlines()
if not os.path.exists(LIST + ".before-package-logos"):
    shutil.copy2(LIST, LIST + ".before-package-logos")
log = []
# Get Started keeps its own logos, with its own drawing.
for i, l in enumerate(lines):
    c = l.split("\t")
    if len(c) >= 6 and c[0] == "package" and c[3] == "Get Started":
        cv, ico = ink_ico("Get Started")
        logos = [e for e in c[4].split(";") if e]
        name, made = make(c[1], logos, cv, ico, logo_files(c[1], logos))
        c[2], c[5] = ico, name
        lines[i] = "\t".join(c)
        log.append(f"Get Started: {made} pictures")
for original, title, drawing, logos in NEW:
    lines = [l for l in lines if not (l.startswith("package\t") and l.split("\t")[3:4] == [title])]
    cv, ico = ink_ico(drawing)
    name, made = make(original, logos, cv, ico, logo_files(original, logos))
    lines.append("\t".join(["package", original, ico, title, ";".join(logos), name]))
    log.append(f"{title}: {made} pictures")
# Network Inspection Service: an exe of its own, in Defender's platform folder.
NISSRV = "*" + chr(92) + "nissrv.exe"
lines = [l for l in lines if l != "program\t*" and not l.startswith("issrv.exe\t")]  # (an earlier slip's rows)
lines = [l for l in lines if not l.lower().startswith("program\t" + NISSRV + "\t")]
lines.append("\t".join(["program", NISSRV, ink_ico("Microsoft Defender")[1], "Microsoft Defender Network Inspection Service", "", ""]))
new = LIST + ".new"
with open(new, "w", encoding="utf-16", newline="\r\n") as f:
    f.write("\n".join(lines) + "\n")
for _ in range(40):
    try:
        os.replace(new, LIST)
        break
    except PermissionError:
        time.sleep(0.05)
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Add-PackageLogos.log"), "w").write("\n".join(log) + "\n")
