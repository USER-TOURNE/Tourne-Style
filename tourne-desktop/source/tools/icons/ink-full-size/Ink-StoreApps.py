"""Store apps in full-size ink: each package row's icon becomes its ink drawing and its logo pictures are made again
(as Tourne'Icons' picker makes them: same files and sizes, tiles with room around the logo, in a new folder named for
the icon), so Start, Task Manager and the rest show the ink ones. Backup: icons.txt.before-ink-store-apps."""
import os, shutil, sys, time
from PIL import Image

sys.path.insert(0, r"%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\302c218a-4e92-4a78-bd95-d817c5319397\scratchpad\ink")
from big import DRAWINGS, Canvas, render, snap, save_ico, file_name  # noqa: E402

ROOT = r"C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-icons"
LIST = ROOT + r"\icons.txt"
BY_NAME = {
    "copilot": "Copilot", "sharex": "ShareX", "settings": "Settings", "nvidia control panel": "NVIDIA Control Panel",
    "snipping tool": "Snipping Tool", "xbox tray": "Xbox", "calculator": "Calculator", "camera": "Camera",
    "click to do": "Click to Do", "clock": "Clock", "dolby access": "Dolby Access", "game bar": "Game Bar",
    "get help": "Get Help", "get started": "Windows Backup", "media player": "Media Player",
    "microsoft designer": "Microsoft Designer", "microsoft store": "Microsoft Store",
    "minecraft for windows": "Minecraft", "minecraft launcher": "Minecraft", "minecraft: java edition": "Minecraft",
    "notepad": "Notepad", "paint": "Paint", "phone link": "Phone Link", "realtek audio console": "Realtek Audio Console",
    "windows security": "Microsoft Defender", "wintoys": "Wintoys", "wireless display": "Wireless Display",
    "cinebench": "Cinebench", "photos": "Photos", "power automate": "Power Automate", "terminal": "Terminal",
}


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


lines = open(LIST, encoding="utf-16").read().splitlines()
if not os.path.exists(LIST + ".before-ink-store-apps"):
    shutil.copy2(LIST, LIST + ".before-ink-store-apps")
log = []
for i, l in enumerate(lines):
    c = l.split("\t")
    if len(c) < 6 or c[0] != "package" or c[3].lower() not in BY_NAME:
        continue
    drawing = BY_NAME[c[3].lower()]
    cv = Canvas()
    DRAWINGS[drawing](cv)
    ico = os.path.join(ROOT, "icons", "Ink2-" + file_name(drawing))
    if not os.path.exists(ico):
        save_ico(render(cv), ico, cv)
    old = os.path.join(ROOT, "packages", c[5])
    new_name = pictures_name(c[1], ico)
    new = os.path.join(ROOT, "packages", new_name)
    tiles = {e[1:].lower() for e in c[4].split(";") if e.startswith("+")}
    made = 0
    for folder, _, files in os.walk(old):
        for f in files:
            src = os.path.join(folder, f)
            rel = os.path.relpath(src, old)
            dst = os.path.join(new, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            if f.startswith("."):
                shutil.copy2(src, dst)
                continue
            try:
                w, h = Image.open(src).size
            except Exception:
                continue
            base = os.path.join(os.path.dirname(rel), f.split(".")[0]).lower()
            side = min(w, h)
            if base in tiles:
                side = max(1, side * 2 // 3)
            pic = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            pic.paste(drawn(cv, side), ((w - side) // 2, (h - side) // 2))
            pic.save(dst)
            made += 1
    if made:
        c[2], c[5] = ico, new_name
        lines[i] = "\t".join(c)
    log.append(f"{c[3]}: {drawing}, {made} pictures")
new_list = LIST + ".new"
with open(new_list, "w", encoding="utf-16", newline="\r\n") as f:
    f.write("\n".join(lines) + "\n")
for _ in range(40):
    try:
        os.replace(new_list, LIST)
        break
    except PermissionError:
        time.sleep(0.05)
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Ink-StoreApps.log"), "w").write("\n".join(log) + "\n")
