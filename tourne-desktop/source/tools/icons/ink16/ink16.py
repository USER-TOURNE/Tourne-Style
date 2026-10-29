"""Tourne ink at 16 px (the chevron, Steam and folder style): 1-pixel lines on a 16x16 grid, shadow one pixel down
and right, scaled by whole pixels into a multi-size .ico."""
import math
from PIL import Image

G, R = (0x66, 0xEA, 0x97), (0xFF, 0x6F, 0x77)
SHADOW = {G: (0x3C, 0x86, 0x5C), R: (0xA9, 0x5D, 0x5D)}


class C:
    def __init__(self): self.px = {}
    def dot(self, x, y, c=G):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < 15 and 0 <= y < 15: self.px[(x, y)] = c
    def line(self, x0, y0, x1, y1, c=G):
        n = int(max(abs(x1 - x0), abs(y1 - y0))) or 1
        for i in range(n + 1): self.dot(x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, c)
    def ring(self, cx, cy, r, c=G):
        for y in range(15):
            for x in range(15):
                if abs(math.hypot(x - cx, y - cy) - r) < 0.5: self.dot(x, y, c)


def render(cv):
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    for (x, y), c in cv.px.items():
        if (x + 1, y + 1) not in cv.px: im.putpixel((x + 1, y + 1), SHADOW[c] + (255,))
    for (x, y), c in cv.px.items(): im.putpixel((x, y), c + (255,))
    return im


def save(cv, path):
    im = render(cv)
    frames = [im.resize((s, s), Image.NEAREST) for s in (16, 20, 24, 32, 40, 48, 64, 96, 128, 256)]
    frames[-1].save(path, format="ICO", sizes=[f.size for f in frames], append_images=frames[:-1])


def github(c):
    c.ring(7, 7, 7)
    c.line(4, 4, 4, 9), c.line(10, 4, 10, 9)          # head sides, up into the ears
    c.line(5, 5, 6, 6), c.line(9, 5, 8, 6), c.line(6, 6, 8, 6)
    c.line(5, 10, 9, 10)                               # chin
    c.dot(6, 8, R), c.dot(8, 8, R)                     # eyes
    c.line(6, 11, 6, 13), c.line(8, 11, 8, 13)        # neck down to the ring


def snipping(c):
    c.line(4, 8, 10, 0), c.line(10, 8, 4, 0)          # blades
    for x in (2, 9):                                   # handles
        c.line(x + 1, 10, x + 2, 10, R), c.line(x + 1, 14, x + 2, 14, R)
        c.line(x, 11, x, 13, R), c.line(x + 3, 11, x + 3, 13, R)
    c.line(4, 8, 4, 9, R), c.line(10, 8, 10, 9, R)
    c.dot(7, 4, R)                                     # pivot
