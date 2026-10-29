"""Tourne ink pack add-on 6: the full-size ink drawings (big.py), for every row in the live Tourne'Icons list whose
app had only a stand-in. Rows are matched by name; the pack keeps each row's kind, original and progid."""
import os, shutil, sys
from big import DRAWINGS, Canvas, render, save_ico, file_name

LIST = r"C:\ProgramData\Windhawk\Engine\ModsWritable\mod-storage\local@tourne-icons\icons.txt"
OUT = sys.argv[1]

RULES = [  # (drawing, test on the lowercased name without a leading *)
    ("Terminal", lambda n: n in ("terminal", "windowsterminal", "wt", "openconsole", "elevate-shim")),
    ("Google Chrome", lambda n: n in ("chrome", "google chrome")),
    ("Microsoft Edge", lambda n: n in ("microsoft edge", "msedgewebview2")),
    ("VSCodium", lambda n: n == "vscodium"),
    ("VS Code", lambda n: n.startswith("microsoft visual studio code")),
    # Windhawk keeps the ink pack's chevron (Tourne's choice), so it has no row here.
    ("Discord", lambda n: n in ("discord", "discordsystemhelper") or n.startswith("com.squirrel.discord")),
    ("Task Manager", lambda n: n in ("task manager", "taskmgr")),
    ("Everything", lambda n: n.startswith("everything")),
    ("HWiNFO", lambda n: n.startswith("hwinfo")),
    ("Notepad++", lambda n: n == "notepad++"),
    ("Python", lambda n: n.startswith("python 3") or n.startswith("idle (python")),
    ("StartAllBack", lambda n: n == "startallback"),
    ("Rainmeter", lambda n: n == "rainmeter"),
    ("REDlauncher", lambda n: n == "redlauncher"),
    ("Intel NPU", lambda n: n.startswith("intel(r) npu")),
    ("Mouzi", lambda n: n == "mouzi"),
    ("Mozilla Maintenance", lambda n: n.startswith("mozilla maintenance")),
    ("NET Runtime", lambda n: n.startswith("microsoft windows desktop runtime")),
    ("ASP.NET", lambda n: n.startswith("microsoft asp.net")),
    ("Visual C++", lambda n: n.startswith("microsoft visual c++")),
    ("Windows SDK", lambda n: n.startswith("windows software development kit")),
    ("Zen Browser", lambda n: n.startswith("zen browser") or n.startswith("zen private")),
    ("Sizer", lambda n: n.startswith("sizer")),
    ("tacky-borders", lambda n: n == "tacky-borders"),
    ("Tourne terminal menu", lambda n: n == "tourne terminal menu"),
    ("Node.js", lambda n: n in ("node.js", "uninstall node.js", "install additional tools for node.js")),
    ("MSI Afterburner", lambda n: n.startswith("msi afterburner") or n == "msiafterburner"),
    ("Developer PowerShell", lambda n: n.startswith("developer powershell")),
    ("App Cert Kit", lambda n: n == "windows app cert kit"),
    ("App Verifier", lambda n: n.startswith("application verifier")),
    ("Flow Launcher", lambda n: n == "flow.launcher"),
    ("Magnify", lambda n: n == "magnify"),
    ("Search Indexer", lambda n: n.startswith("searchindexer")),
    ("Cinebench", lambda n: n == "cinebench"),
    ("LatencyMon", lambda n: n.startswith("latencymon")),
    ("EA", lambda n: n in ("ea", "ea app", "ea app updater", "ea error reporter")),
    ("LTK Manager", lambda n: n == "ltk manager"),
    ("Soulframe", lambda n: n == "soulframe"),
    ("Controller Companion", lambda n: n == "controller companion"),
    ("Photos", lambda n: n == "photos"),
    ("Eagle", lambda n: n == "eagle"),
    ("JPEGView", lambda n: n == "jpegview"),
    ("Topaz Video", lambda n: n == "topaz video"),
    ("Kontext", lambda n: n == "kontext"),
    ("File History", lambda n: n == "file history"),
    ("Peace Equalizer", lambda n: n.startswith("peter's equalizer")),
    ("Power Automate", lambda n: n == "power automate"),
    ("Convertify", lambda n: n.startswith("convertify")),
    ("File Converter", lambda n: n.startswith("file converter")),
]

os.makedirs(os.path.join(OUT, "icons"), exist_ok=True)
for name, fn in DRAWINGS.items():
    cv = Canvas()
    fn(cv)
    save_ico(render(cv), os.path.join(OUT, "icons", file_name(name)), cv)
shutil.copy2(os.path.join(os.path.dirname(os.path.abspath(__file__)), "big", "preview.png"), os.path.join(OUT, "preview.png"))

lines = [
    "# Tourne's ink icon pack add-on 6 (2026-10-20): full-size ink drawings for the apps that had a stand-in "
    "(Terminal, Chrome, Edge, VS Code, Windhawk, Discord and more). Open this file with Apply a pack in Tourne'Icons.",
    "# picture\tkind\toriginal\tname\tprogid\tlist\tshows-from",
]
used = {}
for l in open(LIST, encoding="utf-16").read().splitlines():
    if not l or l.startswith("#"):
        continue
    c = l.split("\t") + [""] * 6
    kind, original, name, progid = c[0], c[1], c[3], c[4]
    if kind not in ("program", "package", "button", "icon"):
        continue
    key = name.lstrip("*").lower()
    for drawing, test in RULES:
        if test(key):
            lines.append(f"icons/{file_name(drawing)}\t{kind}\t{original}\t{name}\t{progid}\t\t")
            used[drawing] = used.get(drawing, 0) + 1
            break
with open(os.path.join(OUT, "pack.tsv"), "w", encoding="utf-16", newline="\r\n") as f:
    f.write("\n".join(lines) + "\n")
print(len(lines) - 2, "rows")
for d in DRAWINGS:
    if d not in used:
        print("no rows for", d)
