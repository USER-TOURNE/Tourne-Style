#!/usr/bin/env python3
"""Tourne'Table's one settings list: reads the settings block at the top of tourne-table.wh.cpp and
writes, between the "settings table" markers lower in the same file:

  kSetupKeys     every setting a style shares, with its default and the part of a style it belongs to
                 (Look, Motion, Extras, Place), so styles, the gallery's list and the quick-settings
                 panel can't drift from the settings themselves
  kSettingsInfo  every setting with its name, group, kind and choices, for the quick-settings panel,
                 and its hover tip (setting-tips.txt beside this script, else its settings-page description)

    python tools/table/make-settings-table.py           # rewrite the table in tourne-table.wh.cpp
    python tools/table/make-settings-table.py --check   # exit 1 when the file's table is out of date

Then copy the file to tourne-table-2.3.0-beta.wh.cpp (its own id and name) as usual. Which part a
setting belongs to, and which never go in a style, is set in PART_RULES and PRIVATE below; everything
else comes from the settings block.
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
MOD = os.path.join(os.path.dirname(os.path.dirname(HERE)), "tourne-table.wh.cpp")
TIPS = os.path.join(HERE, "setting-tips.txt")
BEGIN = "// ---- settings table (made by tools/table/make-settings-table.py, don't edit by hand): begin"
END = "// ---- settings table: end"
LOOK, MOTION, EXTRAS, PLACE = 1, 2, 4, 8

# Never in a style: what belongs to one PC (a device, a hotkey, a monitor, an app, file paths, the GPU
# and its cores) and the settings page's own error reporting.
PRIVATE = {
    "audio.source", "audio.deviceName", "depth.cutter", "position.panelHotkey", "position.monitor",
    "position.dockApp", "media_controls.iconPrevPath", "media_controls.iconPlayPath",
    "media_controls.iconPausePath", "media_controls.iconNextPath", "hardware.renderDevice",
    "hardware.npuRuntimePath", "hardware.engineCoreList", "interaction.menuFont", "interaction.menuFontSize",
}
PRIVATE_GROUPS = {"validation"}

# The part of a style a setting goes in: the first rule whose pattern matches the key, else its group's.
PART_RULES = [
    (r"appearance\.(sensitivity\w*|inputGain|autoGain\w*|smoothing|eqPreset|fftSize|freqScale|oscilloscope\w+)$", MOTION),
    (r"appearance\.(nowPlaying\w+|peakFreq\w+)$", EXTRAS),
    (r"analysis\.(readout|loudnessResetOnTrack)$", EXTRAS),
    (r"bassLine\.enabled$", EXTRAS),
    (r"bassLine\.offset[XY]$", PLACE),
    (r"bassLine\.(style|placement|layer|height|width|gap|thickness|\w+Color)$", LOOK),
    (r"bassLine\.", MOTION),
    (r"media_controls\.(stacking|hideWhenCovered|coveredThresholdPercent|horizontalPosition|verticalPosition|anchor\w*)$", PLACE),
]
GROUP_PART = {
    "appearance": LOOK, "terminal": LOOK, "glass": LOOK, "background": LOOK,
    "analysis": MOTION, "bass": MOTION, "performance": MOTION, "hardware": MOTION,
    "progress": EXTRAS, "depth": EXTRAS, "media_controls": EXTRAS,
    "position": PLACE, "interaction": PLACE,
}
# Sub-pages in the quick-settings panel, so a long group isn't one long list: the first rule whose pattern
# matches a key names its section; a key with none sits on the group's own page, under the sections.
SECTION_RULES = [
    (r"appearance\.(shape|orientation|bar\w+|verticalAnchor|reflection|afterimage\w*)$", "Shape and Bars"),
    (r"appearance\.(colorMode|color|gradientColor\d|rainbowSpeed|peakHold\w+|beatFlash\w+)$", "Colours"),
    (r"appearance\.fx\w+$", "Glow, Bloom, Outline and Shadow"),
    (r"appearance\.(sensitivity\w*|inputGain|autoGain\w*|smoothing|eqPreset|fftSize|freqScale)$", "Sensitivity"),
    (r"appearance\.oscilloscope\w+$", "Oscilloscope"),
    (r"appearance\.(nowPlaying\w+|textRendering)$", "Now Playing"),
    (r"appearance\.peakFreq\w+$", "Readout"),
    (r"analysis\.(engine|bandLayout|octaveFraction|minFrequency|maxFrequency|tuningA4)$", "Bands"),
    (r"analysis\.(weighting|tilt|detector|levelReference|window|bassDetail|channel|dbFloor|dbCeiling)$", "Levels"),
    (r"analysis\.(ballistics|attackMs|releaseDbPerSecond|peakHoldMs|peakFall)$", "Motion"),
    (r"bassLine\.(style|placement|layer|height|width|gap|thickness|offset[XY]|\w+Color)$", "Look"),
    (r"bassLine\.(window|lowHz|highHz|floorDb|depthDb|boostDb|smoothMs|kickSens|snareSens|cooldownMs)$", "Detection"),
    (r"depth\.(cutter\w*)$", "Cutter"),
    (r"depth\.(source|folder|file|fit|scale|offset[XY])$", "Picture"),
    (r"depth\.key\w+$", "Key"),
    (r"position\.(dock\w+)$", "Dock To App"),
    (r"position\.(split\w*)$", "Split"),
    (r"position\.panel\w+$", "Panel"),
    (r"interaction\.keyMove\w+$", "Keyboard Move"),
    (r"interaction\.drag\w+$", "Drag"),
    (r"media_controls\.(icon\w+|plate\w+)$", "Strip"),
    (r"media_controls\.(card\w+|backdrop)$", "Card"),
    (r"media_controls\.(stacking|hideWhenCovered|coveredThresholdPercent|horizontalPosition|verticalPosition|anchor\w*)$", "Place"),
    (r"terminal\.(\w+Color)$", "Colours"),
    (r"progress\.(clickToSeek|seekApps\w*)$", "Click to Seek"),
    (r"performance\.(autoHide\w+|pauseWhenObscured|obscuredThresholdPercent)$", "Hiding"),
]
KIND = {"switch": 0, "number": 1, "text": 2, "choice": 3, "colour": 4}


def unquote(v):
    v = v.strip()
    if len(v) >= 2 and v[0] == v[-1] and v[0] in "'\"":
        v = v[1:-1]
        if v.startswith("'"):
            pass
        v = v.replace("''", "'")
    return v


def read_settings(src):
    a = src.index("// ==WindhawkModSettings==")
    b = src.index("// ==/WindhawkModSettings==")
    lines = [l for l in src[a:b].splitlines()[1:] if l.strip() not in ("/*", "*/")]
    out, group, cur, in_opts = [], None, None, False
    for line in lines:
        m = re.match(r"^- (\w+):\s*$", line)
        if m:
            group, cur, in_opts = m.group(1), None, False
            continue
        m = re.match(r"^  \$name:\s*(.*)$", line)
        if m and group:
            for s in out:
                if s["group"] == group:
                    s["groupName"] = unquote(m.group(1))
            continue
        m = re.match(r"^    - (\w+):\s*(.*)$", line)
        if m and group:
            raw = m.group(2).strip()
            cur = {"key": f"{group}.{m.group(1)}", "group": group, "groupName": group, "raw": raw,
                   "default": unquote(raw), "name": m.group(1), "options": []}
            out.append(cur)
            in_opts = False
            continue
        if cur is None:
            continue
        m = re.match(r"^      \$name:\s*(.*)$", line)
        if m:
            cur["name"] = unquote(m.group(1))
            continue
        m = re.match(r"^      \$description:\s*(.*)$", line)
        if m:
            cur["description"] = unquote(m.group(1))
            in_opts = False
            continue
        if re.match(r"^      \$options:\s*$", line):
            in_opts = True
            continue
        m = re.match(r"^        - ('[^']*'|[\w.-]+):\s*(.*)$", line)
        if m and in_opts:
            cur["options"].append((unquote(m.group(1)), unquote(m.group(2))))
            continue
        if re.match(r"^      \$", line):
            in_opts = False
    return out


def kind_of(s):
    raw = s["raw"]
    if s["options"]:
        return "choice"
    if raw in ("true", "false"):
        return "switch"
    if re.fullmatch(r"-?\d+", raw):
        return "number"
    v = s["default"]
    if re.fullmatch(r"#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?|rgba?\(.*\)", v) or s["key"].lower().endswith("color"):
        return "colour"
    if re.fullmatch(r"-?\d+(\.\d+)?", v):
        return "number"
    return "text"


def section_of(key):
    for pat, name in SECTION_RULES:
        if re.match(pat, key):
            return name
    return ""


def part_of(key, group):
    for pat, part in PART_RULES:
        if re.match(pat, key):
            return part
    return GROUP_PART.get(group, 0)


def read_tips():
    """key = text | Try: ... lines; the tip is the text, a line break, then the Try part."""
    tips = {}
    if not os.path.exists(TIPS):
        return tips
    for line in open(TIPS, encoding="utf-8"):
        line = line.strip()
        if not line or line.startswith("#") or " = " not in line:
            continue
        key, text = line.split(" = ", 1)
        text = text.replace("\\n", "\n")  # (a \n in the file starts a new line in the tip: one per option)
        what, _, tryit = text.partition(" | ")
        tips[key.strip()] = what.strip() + ("\n" + tryit.strip() if tryit else "")
    return tips


def wstr(v):
    return 'L"' + v.replace("\\", "\\\\").replace('"', '\\"') + '"'


def setup_default(s):
    raw = s["raw"]
    if raw == "true":
        return "1", False
    if raw == "false":
        return "0", False
    if re.fullmatch(r"-?\d+", raw):
        return raw, False
    return s["default"], True


def table(settings, tips):
    out = [BEGIN,
           "// Every setting a style shares: its default (as text) and the part of a style it is in.",
           "static const VizSetupKey kSetupKeys[] = {"]
    for s in settings:
        if s["key"] in PRIVATE or s["group"] in PRIVATE_GROUPS:
            continue
        part = part_of(s["key"], s["group"])
        if not part:
            sys.exit(f"{s['key']}: no part for it (PART_RULES or GROUP_PART)")
        d, text = setup_default(s)
        out.append(f"    {{{wstr(s['key'])}, {wstr(d)}, {'true' if text else 'false'}, {part}}},")
    out += ["};", "",
            "// Every setting, for the quick-settings panel: its group, its name on the settings page, what kind of",
            "// control it is (0 switch, 1 number, 2 text, 3 choice, 4 colour), its default and, for a choice,",
            "// value=label lines.",
            "struct VizSettingInfo {",
            "    const wchar_t* key;",
            "    const wchar_t* group;",
            "    const wchar_t* section;  // its sub-page in the panel, or empty",
            "    const wchar_t* name;",
            "    unsigned char kind;",
            "    const wchar_t* def;",
            "    const wchar_t* options;",
            "    const wchar_t* tip;  // what it is and where it shows, then a line with a value to try",
            "};",
            "static const VizSettingInfo kSettingsInfo[] = {"]
    for s in settings:
        if s["group"] in PRIVATE_GROUPS:
            continue
        opts = "\\n".join(f"{v}={l}".replace("\\", "\\\\").replace('"', '\\"') for v, l in s["options"])
        tip = tips.get(s["key"], s.get("description", "")).replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
        out.append(f"    {{{wstr(s['key'])}, {wstr(s['groupName'])}, {wstr(section_of(s['key']))}, {wstr(s['name'])}, {KIND[kind_of(s)]}, {wstr(setup_default(s)[0])}, L\"{opts}\",")
        out.append(f"     L\"{tip}\"}},")
    out += ["};", END]
    return "\n".join(out)


def main():
    src = open(MOD, encoding="utf-8", newline="").read()
    crlf = "\r\n" in src
    src = src.replace("\r\n", "\n")
    settings = read_settings(src)
    if len(settings) < 200:
        sys.exit("couldn't read the settings block")
    tips = read_tips()
    unknown = set(tips) - {s["key"] for s in settings}
    if unknown:
        sys.exit("setting-tips.txt: no such setting: " + ", ".join(sorted(unknown)))
    new = table(settings, tips)
    if BEGIN in src:
        a, b = src.index(BEGIN), src.index(END) + len(END)
        result = src[:a] + new + src[b:]
    else:
        a = src.index("static const VizSetupKey kSetupKeys[]")
        b = src.index("};", a) + 2
        result = src[:a] + new + src[b:]
    if "--check" in sys.argv:
        if result != src:
            print("tourne-table.wh.cpp: the settings table is out of date; run tools/table/make-settings-table.py")
            return 1
        print("settings table up to date")
        return 0
    if crlf:
        result = result.replace("\n", "\r\n")
    open(MOD, "w", encoding="utf-8", newline="").write(result)
    shared = sum(1 for s in settings if s["key"] not in PRIVATE and s["group"] not in PRIVATE_GROUPS)
    print(f"{len(settings)} settings, {shared} of them shared in styles")
    return 0


if __name__ == "__main__":
    sys.exit(main())
