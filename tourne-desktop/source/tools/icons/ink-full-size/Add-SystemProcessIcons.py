"""Full-size ink icons for Windows' own background processes (what Task Manager lists): program rows added to the live
Tourne'Icons list, written beside and swapped in like the mod does. A row with the same original is replaced."""
import os, shutil, sys, time

sys.path.insert(0, r"%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\302c218a-4e92-4a78-bd95-d817c5319397\scratchpad\ink")
from big import DRAWINGS, Canvas, render, save_ico, file_name  # noqa: E402

ROOT = r"C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-icons"
LIST = ROOT + r"\icons.txt"
ROWS = [  # (original, drawing, name)
    (r"*\svchost.exe", "Service Host", "Service Host"),
    (r"*\MsMpEng.exe", "Microsoft Defender", "Microsoft Defender Antivirus Service"),
    (r"*\MpDefenderCoreService.exe", "Microsoft Defender", "Microsoft Defender Core Service"),
    (r"*\DefenderSessionHelper.exe", "Microsoft Defender", "Microsoft Defender Session Helper"),
    (r"*\SearchHost.exe", "Search", "Search"),
    (r"*\SearchProtocolHost.exe", "Search Indexer", "Microsoft Windows Search Protocol Host"),
    (r"*\SearchFilterHost.exe", "Search Indexer", "Microsoft Windows Search Filter Host"),
    (r"*\gamingservices.exe", "Gaming Services", "Gaming Services"),
    (r"*\gamingservicesnet.exe", "Gaming Services", "Gaming Services"),
    (r"*\sihost.exe", "Shell Infrastructure Host", "Shell Infrastructure Host"),
]
made = {}
for _, drawing, _ in ROWS:
    if drawing in made:
        continue
    cv = Canvas()
    DRAWINGS[drawing](cv)
    path = os.path.join(ROOT, "icons", "Ink2-" + file_name(drawing))
    save_ico(render(cv), path, cv)
    made[drawing] = path
lines = open(LIST, encoding="utf-16").read().splitlines()
shutil.copy2(LIST, LIST + ".before-system-processes")
originals = {o.lower() for o, _, _ in ROWS}
lines = [l for l in lines if not (l.startswith("program\t") and l.split("\t")[1].lower() in originals)]
for original, drawing, name in ROWS:
    lines.append("\t".join(["program", original, made[drawing], name, "", ""]))
new = LIST + ".new"
with open(new, "w", encoding="utf-16", newline="\r\n") as f:
    f.write("\n".join(lines) + "\n")
for _ in range(40):
    try:
        os.replace(new, LIST)
        break
    except PermissionError:
        time.sleep(0.05)
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Add-SystemProcessIcons.log"), "w").write(f"{len(ROWS)} rows, {len(made)} icons\n")
