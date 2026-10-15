#!/usr/bin/env python3
"""Makes tourne-menus.wh.cpp (the repo's root) from tools/menus/tourne-menus.cpp: each
"// @inline <file>" line is replaced by that file from the repo's root, as the suite's
build does. Edit tourne-menus.cpp and the shared files, then run this.

    python3 tools/menus/make-menus.py
"""
import pathlib
import re

here = pathlib.Path(__file__).resolve().parent
root = here.parent.parent
source = here / "tourne-menus.cpp"
out = root / "tourne-menus.wh.cpp"


def read(path):
    return path.read_text(encoding="utf-8").replace("\r\n", "\n")


def expand(match):
    name = match.group(1)
    return read(root / name).rstrip("\n")


text = re.sub(r"(?m)^// @inline (\S+)$", expand, read(source))
out.write_text(text, encoding="utf-8", newline="\n")
print(f"{out.name}: {len(text.splitlines())} lines")
