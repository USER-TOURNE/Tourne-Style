# Builds the icon pack previews for the Tourne'Style page (docs/icons.html):
# one sprite sheet per pack in docs/icons/ plus packs.json with each icon's name.
# Run from the repo root after changing a pack: python icon-packs/make-previews.py
import glob, json, math, os
from PIL import Image

ROOT = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(ROOT, '..', 'docs', 'icons')

def ico(path, cell):
    im = Image.open(path)
    sizes = im.info.get('sizes') or {im.size}
    best = min((s for s in sizes if s[0] >= cell), default=max(sizes), key=lambda s: s[0])
    im.size = best
    im = im.convert('RGBA')
    return im if im.size == (cell, cell) else im.resize((cell, cell), Image.NEAREST)

def from_tsv(folder):
    rows = [r.split('\t') for r in open(os.path.join(ROOT, folder, 'pack.tsv'), encoding='utf-16').read().splitlines()
            if r and not r.startswith('#')]
    seen = {}
    for r in rows:
        pic, kind, name, where = r[0], r[1], r[3] or os.path.basename(r[0])[:-4], r[5] if len(r) > 5 else ''
        if pic in seen: continue
        seen[pic] = {'n': name, 'k': kind, 'w': where, 'f': pic}
    return list(seen.values())

def sheet(pid, items, cell, load):
    cols = min(32, len(items)); rows = math.ceil(len(items) / cols)
    img = Image.new('RGBA', (cols * cell, rows * cell))
    for i, it in enumerate(items):
        img.paste(load(it), ((i % cols) * cell, (i // cols) * cell))
    img.save(os.path.join(OUT, pid + '.png'), optimize=True)
    for it in items: it.pop('f', None)
    return cols

packs = []
def add(pid, folder, name, about, items, cell, show, load, pixel=False):
    cols = sheet(pid, items, cell, load)
    packs.append({'id': pid, 'folder': folder, 'name': name, 'about': about, 'cell': cell, 'show': show,
                  'cols': cols, 'pixel': pixel, 'icons': items})

for pid, folder, name, about in [
    ('ink', 'tourne-ink-pack', "Tourne ink pack", "The complete ink set in my dark green: programs, Store apps, Windows' own icons, file types and Tourne Desktop's taskbar buttons."),
    ('red-green', 'tourne-red-green-pack', "Tourne red and green pack", "The earlier red and green pixel set."),
]:
    items = from_tsv(folder)
    add(pid, folder, name, about, items, 64, 48, lambda it, f=folder: ico(os.path.join(ROOT, f, it['f']), 64), pixel=True)

tray = sorted(glob.glob(os.path.join(ROOT, 'tourne-tray-icons', 'art', 'png', '4x', '*.png')))
add('tray', 'tourne-tray-icons', "Tourne tray icons", "The pixel tray icons: Wi-Fi, sound, battery and the rest, as drawn live by Tourne'Tray and the suite.",
    [{'n': os.path.basename(p)[5:-4].replace('-', ' '), 'k': 'tray', 'w': '', 'f': p} for p in tray], 52, 52,
    lambda it: Image.open(it['f']).convert('RGBA'), pixel=True)

orbs = sorted(glob.glob(os.path.join(ROOT, 'tourne-start-orbs', 'Tourne Start * 2x.png'))) + [os.path.join(ROOT, 'tourne-start-orbs', 'tourne-null-orb-2x.png')]
def orb(it):
    im = Image.open(it['f']).convert('RGBA'); w = im.size[0]
    return im.crop((0, 0, w, w)).resize((48, 48), Image.NEAREST) if w != 48 else im.crop((0, 0, 48, 48))
add('orbs', 'tourne-start-orbs', "Tourne Start orbs", "Start buttons in the Start Mark designs for StartAllBack or Tourne Desktop's taskbar.",
    [{'n': os.path.basename(p)[:-7].replace('Tourne ', '').replace('tourne-', '').replace('-', ' '), 'k': 'orb', 'w': '', 'f': p} for p in orbs], 48, 48, orb, pixel=True)

json.dump(packs, open(os.path.join(OUT, 'packs.json'), 'w'), separators=(',', ':'))
print('\n'.join(f"{p['name']}: {len(p['icons'])}" for p in packs))
