#!/usr/bin/env python3
"""Tourne Desktop's built-in presets, made from the research catalogue.

    python3 desktop/tools/make-presets.py [catalogue.json]

Reads the catalogue (by default desktop/presets/catalogue.json, which this
also writes when given the full research file) and writes
desktop/host/presets-data.inc.

What ships: every entry with all twelve colour roles, once per set of colours
(the same scheme from several places is kept once, the best-placed copy).
Full looks also carry the settings Tourne has a home for: tiling gaps, window
border width, colours and corners, glass corners. Only facts are kept
(colours, sizes, names, where they came from); no theme files.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
CATALOGUE = os.path.join(ROOT, "presets", "catalogue.json")
IDENTITIES = os.path.join(ROOT, "presets", "identities.json")  # Tourne's own: a palette with a whole identity, listed first
OUT = os.path.join(ROOT, "host", "presets-data.inc")

ROLES = ["ground", "raised", "line", "edge", "text", "accent", "muted", "dim", "warn", "bad", "accent2", "good"]  # Roles' order
HEX = re.compile(r"^#[0-9a-fA-F]{6}$")
SKIP = {
    # Written from memory of the Material spec, not read from it.
    "material-you-baseline-dark",
    "material-you-baseline-light",
}
KIND_RANK = {"full-look": 0, "palette": 1, "bar-style": 2}


def luminance(hex_colour):
    v = int(hex_colour[1:], 16)
    def channel(c):
        c /= 255
        return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    return 0.2126 * channel(v >> 16 & 255) + 0.7152 * channel(v >> 8 & 255) + 0.0722 * channel(v & 255)


def number(v):
    try:
        f = float(v)
    except (TypeError, ValueError):
        return None
    return int(round(f)) if f >= 0 else None


def colours(v):
    if isinstance(v, str):
        v = [v]
    if not isinstance(v, list):
        return []
    return [c.lower() for c in v if isinstance(c, str) and HEX.match(c)]


def look_settings(look):
    """The look's values Tourne has a setting for."""
    out = {}
    gaps = look.get("gaps")
    if isinstance(gaps, dict):
        inner, outer = number(gaps.get("inner")), number(gaps.get("outer"))
        if inner is not None and inner <= 64:
            out["switcher.tiling.gapInner"] = str(inner)
        if outer is not None and outer <= 64:
            for side in ("Top", "Bottom", "Left", "Right"):
                out["switcher.tiling.gap" + side] = str(outer)
    width = number(look.get("border_width"))
    if width is not None and 1 <= width <= 16:
        out["switcher.borders.width"] = str(width)
    radius = number(look.get("corner_radius"))
    if radius is not None and radius <= 40:
        if radius == 0:
            out["switcher.borders.radius"] = "square"
        else:
            out["switcher.borders.radius"] = "custom"
            out["switcher.borders.radiusCustom"] = str(radius)
        out["switcher.corners.radius"] = str(radius)
        out["glass.radius"] = str(radius)
    angle = number(look.get("active_border_gradient_angle"))
    for field, key in (("active_border_colours", "switcher.borders.activeColor"),
                       ("inactive_border_colours", "switcher.borders.inactiveColor")):
        cs = colours(look.get(field))
        if cs:
            value = " ".join(cs)
            if len(cs) > 1 and angle is not None:
                value += " %ddeg" % angle
            out[key] = value
    return out


def trim(p):
    """What the repo keeps of a catalogue entry."""
    keep = {k: p[k] for k in ("id", "name", "family", "source", "source_url", "licence", "kind", "variant",
                              "featured", "rank", "why") if p.get(k) not in (None, "")}
    if p["id"].startswith("pick-"):
        keep["family"] = "Tourne pick"
    keep["palette"] = {r: p["palette"][r].lower() for r in ROLES}
    settings = look_settings(p.get("look") or {})
    if settings:
        keep["settings"] = settings
    return keep


def wide(s):
    s = str(s).replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
    # (an escape ends its literal, so the next character can't be read as more hex)
    def escape(c):
        units = c.encode("utf-16-le")  # (a wchar_t is UTF-16 on Windows)
        return "".join('\\x%04X" L"' % int.from_bytes(units[i:i + 2], "little") for i in range(0, len(units), 2))
    s = "".join(c if 32 <= ord(c) < 127 else escape(c) for c in s)
    return 'L"' + s + '"'


def main():
    if len(sys.argv) > 1:
        raw = json.load(open(sys.argv[1], encoding="utf-8"))
        full = [p for p in raw
                if p.get("id") not in SKIP and p.get("kind") in KIND_RANK
                and all(HEX.match(str((p.get("palette") or {}).get(r, ""))) for r in ROLES)]
        # Picks first (by their rank), then featured, then full looks, palettes, bar styles.
        full.sort(key=lambda p: (0 if p["id"].startswith("pick-") else 1, p.get("rank") or 99,
                                 0 if p.get("featured") else 1, KIND_RANK[p["kind"]]))
        seen, kept = set(), []
        for p in full:
            key = tuple(p["palette"][r].lower() for r in ROLES)
            if key in seen:
                continue
            seen.add(key)
            kept.append(trim(p))
        with open(CATALOGUE, "w", encoding="utf-8") as f:
            json.dump(kept, f, indent=1, ensure_ascii=False)
            f.write("\n")
    presets = json.load(open(IDENTITIES, encoding="utf-8")) + json.load(open(CATALOGUE, encoding="utf-8"))

    ids = set()
    lines = ["// Made by desktop/tools/make-presets.py from desktop/presets/identities.json and catalogue.json. Don't edit.",
             "// id, name, family, source, light, pick (0: not one), roles (Roles' order), settings (key=value lines)"]
    for p in presets:
        assert p["id"] not in ids, p["id"]
        ids.add(p["id"])
        pal = p["palette"]
        light = p.get("variant") == "light" or (p.get("variant") is None and luminance(pal["ground"]) > 0.4)
        pick = p.get("rank") or 0 if p["id"].startswith("pick-") else 0
        roles = ", ".join("0x" + pal[r][1:].upper() for r in ROLES)
        settings = "".join("%s=%s\n" % kv for kv in sorted((p.get("settings") or {}).items()))
        lines.append("{%s, %s, %s, %s, %s, %d, {%s}, %s}," % (
            wide(p["id"]), wide(p["name"]), wide(p["family"]), wide(p["source"]),
            "true" if light else "false", pick, roles, wide(settings)))
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    looks = sum(1 for p in presets if p.get("settings"))
    print("%d presets (%d with look settings, %d light) -> %s" % (
        len(presets), looks, sum(1 for p in presets if p.get("variant") == "light"), os.path.relpath(OUT, ROOT)))


if __name__ == "__main__":
    main()
