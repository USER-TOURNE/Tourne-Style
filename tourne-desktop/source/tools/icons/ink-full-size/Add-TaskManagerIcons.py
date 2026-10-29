"""Task Manager's own icons (Service Host groups, the services under them, System Interrupts) as full-size ink
drawings: icon rows on the live Tourne'Icons list for Taskmgr.exe's icons at positions 3, 2 and 1 (groups 30654, 30653, 30652;
the numbers PrivateExtractIcons gives back are image ids, not group ids)."""
import os, shutil, sys, time

sys.path.insert(0, r"%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\302c218a-4e92-4a78-bd95-d817c5319397\scratchpad\ink")
from big import DRAWINGS, Canvas, render, save_ico, file_name  # noqa: E402

ROOT = r"C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-icons"
LIST = ROOT + r"\icons.txt"
ROWS = [
    (r"C:\windows\System32\Taskmgr.exe,3", "Service Host", "Task Manager: Service Host"),
    (r"C:\windows\System32\Taskmgr.exe,2", "Service", "Task Manager: a service"),
    # processes with no icon of their own (sihost, svchost children): Task Manager's blank windows and Windows' default
    (r"C:\windows\System32\Taskmgr.exe,1", "Program", "Task Manager: an app whose own icon it can't read"),
    (r"C:\windows\System32\Taskmgr.exe,4", "Program", "Task Manager: a program with no icon"),
    (r"C:\windows\System32\Taskmgr.exe,15", "Program", "Task Manager: a program with no icon"),
    (r"C:\windows\System32\imageres.dll,-15", "Program", "A program with no icon"),
    (r"C:\windows\System32\shell32.dll,-3", "Program", "A program with no icon"),
]
made = {}
for _, drawing, _ in ROWS:
    if drawing not in made:
        cv = Canvas()
        DRAWINGS[drawing](cv)
        made[drawing] = os.path.join(ROOT, "icons", "Ink2-" + file_name(drawing))
        save_ico(render(cv), made[drawing], cv)
lines = open(LIST, encoding="utf-16").read().splitlines()
shutil.copy2(LIST, LIST + ".before-taskmgr-positions")
originals = {o.lower() for o, _, _ in ROWS} | {"c:\\windows\\system32\\taskmgr.exe,-%d" % n for n in (22, 16, 9, 31, 91)}
lines = [l for l in lines if not (l.startswith("icon\t") and l.split("\t")[1].lower() in originals)]
for original, drawing, name in ROWS:
    lines.append("\t".join(["icon", original, made[drawing], name, "", ""]))
new = LIST + ".new"
with open(new, "w", encoding="utf-16", newline="\r\n") as f:
    f.write("\n".join(lines) + "\n")
for _ in range(40):
    try:
        os.replace(new, LIST)
        break
    except PermissionError:
        time.sleep(0.05)
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "Add-TaskManagerIcons.log"), "w").write(f"{len(ROWS)} rows\n")
