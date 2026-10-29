"""GitHub Desktop and Snipping Tool in the 16 px ink style (like the Windhawk chevron, Steam and folders): new .ico
files, their rows pointed at them, and Snipping Tool's Store logos made again. Backup: icons.txt.before-ink16."""
import os, shutil, sys, time
from PIL import Image
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ink16 import C, render, save, github, snipping  # noqa: E402

B = chr(92)
L = r"C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-icons"
LIST = L + B + "icons.txt"
DRAW = {"8a62b74a": ("GitHub-Desktop", github), "6833952a": ("Snipping-Tool", snipping)}


def pictures_name(original, ico):
    name = original[original.rfind(B) + 1:].replace("*", "#")
    h = 2166136261
    for ch in ico.lower():
        h = ((h ^ ord(ch)) * 16777619) & 0xFFFFFFFF
    return "%s-%08x" % (name, h)


def drawn(im16, side):
    k = max(1, side // 16)
    out = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    big = im16.resize((16 * k, 16 * k), Image.NEAREST) if side >= 16 else im16.resize((side, side), Image.NEAREST)
    out.paste(big, ((side - big.width) // 2, (side - big.height) // 2))
    return out


icos = {}
for mark, (name, fn) in DRAW.items():
    cv = C(); fn(cv)
    path = L + B + "icons" + B + "Ink16-" + name + ".ico"
    save(cv, path)
    icos[mark] = (path, render(cv))
lines = open(LIST, encoding="utf-16").read().splitlines()
if not os.path.exists(LIST + ".before-ink16"):
    shutil.copy2(LIST, LIST + ".before-ink16")
log = []
for i, l in enumerate(lines):
    c = l.split("\t")
    if len(c) < 4:
        continue
    mark = next((m for m in DRAW if m in c[2] and ("github" in c[2].lower() or "snipping" in c[2].lower() or "screensketch" in c[2].lower())), None)
    if not mark:
        continue
    ico, im16 = icos[mark]
    if c[0] == "package" and len(c) > 5:
        old = L + B + "packages" + B + c[5]
        name = pictures_name(c[1], ico)
        tiles = {e[1:].lower() for e in c[4].split(";") if e.startswith("+")}
        made = 0
        for folder, _, files in os.walk(old):
            for f in files:
                src = os.path.join(folder, f); rel = os.path.relpath(src, old)
                dst = os.path.join(L, "packages", name, rel)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                if f.startswith("."):
                    shutil.copy2(src, dst); continue
                try:
                    w, h = Image.open(src).size
                except Exception:
                    continue
                base = os.path.join(os.path.dirname(rel), f.split(".")[0]).lower()
                side = min(w, h)
                if base in tiles:
                    side = max(1, side * 2 // 3)
                pic = Image.new("RGBA", (w, h), (0, 0, 0, 0))
                pic.paste(drawn(im16, side), ((w - side) // 2, (h - side) // 2))
                pic.save(dst); made += 1
        if made:
            c[5] = name
        log.append(f"package {c[3]}: {made} pictures")
    c[2] = ico
    lines[i] = "\t".join(c)
    log.append(f"{c[0]} {c[1]}")
new = LIST + ".new"
with open(new, "w", encoding="utf-16", newline="\r\n") as f:
    f.write("\n".join(lines) + "\n")
for _ in range(40):
    try:
        os.replace(new, LIST); break
    except PermissionError:
        time.sleep(0.05)
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Apply-Ink16.log"), "w").write("\n".join(log) + "\n")
