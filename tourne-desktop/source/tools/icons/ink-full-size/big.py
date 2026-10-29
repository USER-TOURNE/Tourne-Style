"""Tourne ink, full size: 24 px app drawings in the ink palette (line, shadow one pixel down and right, red accents).

Each drawing is made on a 22x22 grid with a few primitives; the renderer adds the shadow, centres it in 24 px and
writes a multi-size .ico (24, 48, 72, 96 and 240-in-256 at whole-pixel scales; 16 and 32 resampled)."""
import math, os, sys
from PIL import Image

G, R = (0x66, 0xEA, 0x97), (0xFF, 0x6F, 0x77)
SHADOW = {G: (0x3C, 0x86, 0x5C), R: (0xA9, 0x5D, 0x5D)}
N = 22


class Canvas:
    def __init__(self):
        self.px = {}

    def dot(self, x, y, c=G):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < N and 0 <= y < N:
            self.px[(x, y)] = c

    def clear(self, x, y):
        self.px.pop((int(x), int(y)), None)

    def rect(self, x0, y0, x1, y1, c=G, fill=False, t=1):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                edge = min(x - x0, x1 - x, y - y0, y1 - y) < t
                if fill or edge:
                    self.dot(x, y, c)

    def rrect(self, x0, y0, x1, y1, c=G, t=2, r=2):
        self.rect(x0, y0, x1, y1, c, t=t)
        for (cx, cy) in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
            for i in range(r - 1):
                for j in range(r - 1 - i):
                    self.clear(cx + (i if cx == x0 else -i), cy + (j if cy == y0 else -j))

    def ring(self, cx, cy, r, t=2, c=G, a0=0, a1=360):
        for y in range(N):
            for x in range(N):
                d = math.hypot(x - cx, y - cy)
                if r - t < d <= r:
                    a = math.degrees(math.atan2(y - cy, x - cx)) % 360
                    if (a0 <= a <= a1) if a0 <= a1 else (a >= a0 or a <= a1):
                        self.dot(x, y, c)

    def disc(self, cx, cy, r, c=G):
        self.ring(cx, cy, r, t=r + 1, c=c)

    def line(self, x0, y0, x1, y1, c=G, t=1):
        steps = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
        for i in range(steps + 1):
            x = x0 + (x1 - x0) * i / steps
            y = y0 + (y1 - y0) * i / steps
            for dx in range(t):
                for dy in range(t):
                    self.dot(x + dx - (t - 1) / 2, y + dy - (t - 1) / 2, c)

    def shape(self, rects, c=G, t=2):
        """The outline (t pixels) of a union of filled rectangles."""
        S = {(x, y) for (x0, y0, x1, y1) in rects for x in range(x0, x1 + 1) for y in range(y0, y1 + 1)}
        for (x, y) in S:
            if any((x + dx, y + dy) not in S for dx in range(-t, t + 1) for dy in range(-t, t + 1) if max(abs(dx), abs(dy)) < t + 0):
                self.dot(x, y, c)

    def text(self, rows, x0, y0, c=G):
        for j, row in enumerate(rows):
            for i, ch in enumerate(row):
                if ch != ".":
                    self.dot(x0 + i, y0 + j, R if ch == "r" else c)


def render(cv):
    im = Image.new("RGBA", (24, 24), (0, 0, 0, 0))
    xs = [p[0] for p in cv.px] or [0]
    ys = [p[1] for p in cv.px] or [0]
    ox = (23 - (max(xs) - min(xs) + 1)) // 2 - min(xs)  # (23: room for the shadow)
    oy = (23 - (max(ys) - min(ys) + 1)) // 2 - min(ys)
    for (x, y), c in cv.px.items():
        if (x + 1, y + 1) not in cv.px:
            im.putpixel((x + ox + 1, y + oy + 1), SHADOW[c] + (255,))
    for (x, y), c in cv.px.items():
        im.putpixel((x + ox, y + oy), c + (255,))
    return im


def snap(cv, size):
    """The drawing at a size that isn't a multiple of 24, kept pixel-sharp: each target pixel is lit when enough of
    the 24 px drawing under it is lit (in the colour most of it is), and the shadow is added again at that size."""
    base = render_lines(cv)  # 24x24, lines only, no shadow
    s = size / 24.0
    out = Canvas()
    lit = {}
    for ty in range(size):
        for tx in range(size):
            x0, x1, y0, y1 = tx / s, (tx + 1) / s, ty / s, (ty + 1) / s
            cover, colours = 0.0, {}
            for sy in range(int(y0), min(24, int(math.ceil(y1)))):
                for sx in range(int(x0), min(24, int(math.ceil(x1)))):
                    c = base.get((sx, sy))
                    if not c:
                        continue
                    a = (min(x1, sx + 1) - max(x0, sx)) * (min(y1, sy + 1) - max(y0, sy))
                    cover += a
                    colours[c] = colours.get(c, 0) + a
            if cover * s * s >= 0.4:
                lit[(tx, ty)] = max(colours, key=colours.get)
    im = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    for (x, y), c in lit.items():
        if (x + 1, y + 1) not in lit and x + 1 < size and y + 1 < size:
            im.putpixel((x + 1, y + 1), SHADOW[c] + (255,))
    for (x, y), c in lit.items():
        im.putpixel((x, y), c + (255,))
    return im


def render_lines(cv):
    """The drawing's line pixels as placed in the 24 px icon (same centring as render), without the shadow."""
    xs = [p[0] for p in cv.px] or [0]
    ys = [p[1] for p in cv.px] or [0]
    ox = (23 - (max(xs) - min(xs) + 1)) // 2 - min(xs)
    oy = (23 - (max(ys) - min(ys) + 1)) // 2 - min(ys)
    return {(x + ox, y + oy): c for (x, y), c in cv.px.items()}


def save_ico(im, path, cv=None):
    frames = []
    for s in (16, 20, 24, 32, 40, 48, 64, 72, 96, 128):
        if s % 24 == 0:
            frames.append(im.resize((s, s), Image.NEAREST))
        elif cv is not None:
            frames.append(snap(cv, s))
        else:
            frames.append(im.resize((s, s), Image.LANCZOS))
    big = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    big.paste(im.resize((240, 240), Image.NEAREST), (8, 8))
    allf = frames + [big]
    allf[-1].save(path, format="ICO", sizes=[f.size for f in allf], append_images=allf[:-1])


# ---------------------------------------------------------------------------------------------- drawings

def terminal(c):
    c.rrect(0, 2, 21, 19)
    c.rect(0, 2, 21, 5, fill=True)
    for x in (3, 6, 9):
        c.dot(x, 3, R), c.dot(x + 1, 3, R)
    c.line(4, 8, 8, 11, t=2), c.line(8, 11, 4, 14, t=2)
    c.rect(10, 15, 16, 16, R, fill=True)


def chrome(c):
    c.ring(10.5, 10.5, 11, 2)
    c.disc(10.5, 10.5, 3.2, R)
    c.ring(10.5, 10.5, 5.5, 1.6)
    for a in (270, 30, 150):
        r0, r1 = 5.5, 10.5
        x0, y0 = 10.5 + r0 * math.cos(math.radians(a + 60)), 10.5 + r0 * math.sin(math.radians(a + 60))
        x1, y1 = 10.5 + r1 * math.cos(math.radians(a)), 10.5 + r1 * math.sin(math.radians(a))
        c.line(x0, y0, x1, y1, t=2)


def edge(c):
    c.ring(10.5, 10.5, 11, 2, a0=75, a1=15)   # the outer swirl, open on the lower right
    c.ring(11, 12, 5.5, 2, a0=180, a1=360)    # the "e"'s bowl
    c.line(5, 12, 16, 12, t=2)
    c.ring(13, 12, 8, 2, R, a0=25, a1=115)    # the wave underneath


def vscode(c):
    c.rect(15, 0, 19, 21, fill=True)
    c.rect(16, 2, 18, 19, R, fill=True)
    c.line(15, 2, 3, 14, t=2)
    c.line(15, 19, 3, 7, t=2)
    c.rect(1, 6, 2, 15, fill=True)


def windhawk(c):
    c.ring(12.5, 10, 8.5, 2)                  # the head
    c.line(5, 6, 0, 10, t=2), c.line(0, 10, 5, 14, t=2)  # the hooked beak
    c.line(1, 10, 5, 10, R, t=2)
    c.disc(11, 7, 1.6, R)                     # the eye
    c.line(14, 18, 17, 21, t=2), c.line(19, 16, 21, 20, t=2)  # the neck


def discord(c):
    c.line(4, 3, 9, 4, t=2), c.line(17, 3, 12, 4, t=2)
    c.line(4, 3, 0, 14, t=2), c.line(17, 3, 21, 14, t=2)
    c.line(0, 14, 5, 18, t=2), c.line(21, 14, 16, 18, t=2)
    c.line(5, 18, 7, 16, t=2), c.line(16, 18, 14, 16, t=2)
    c.line(7, 16, 14, 16, t=2)
    c.line(9, 4, 12, 4, t=2)
    c.disc(7, 10, 1.8, R), c.disc(14, 10, 1.8, R)


def taskmanager(c):
    c.rrect(0, 2, 21, 19)
    c.rect(0, 2, 21, 4, fill=True)
    pts = [(2, 12), (6, 12), (8, 8), (10, 16), (12, 6), (14, 13), (16, 11), (19, 11)]
    for a, b in zip(pts, pts[1:]):
        c.line(*a, *b, R, t=2)


def everything(c):
    c.ring(8.5, 8.5, 8.5, 2)
    c.line(14, 14, 20, 20, t=3)
    c.text(["rrrr", "r...", "rrr.", "r...", "rrrr"], 6, 6)


def hwinfo(c):
    c.rrect(3, 3, 18, 18, t=2, r=2)
    for i in (6, 10, 14):
        c.line(i, 0, i, 2, t=1), c.line(i + 1, 0, i + 1, 2, t=1)
        c.line(i, 19, i, 21, t=1), c.line(i + 1, 19, i + 1, 21, t=1)
        c.line(0, i, 2, i, t=1), c.line(0, i + 1, 2, i + 1, t=1)
        c.line(19, i, 21, i, t=1), c.line(19, i + 1, 21, i + 1, t=1)
    c.rect(10, 6, 11, 7, R, fill=True)
    c.rect(10, 9, 11, 15, R, fill=True)


def notepadpp(c):
    c.rect(2, 1, 16, 21, t=2)
    c.rect(2, 1, 16, 3, fill=True)
    for y in (6, 9, 12):
        c.line(5, y, 13, y)
    c.rect(12, 13, 21, 14, R, fill=True), c.rect(16, 9, 17, 18, R, fill=True)  # "+"
    c.rect(5, 15, 9, 16, fill=True)


def python(c):
    c.shape([(6, 0, 15, 9), (1, 5, 10, 11)], G)
    c.shape([(6, 12, 15, 21), (11, 10, 20, 16)], R)
    c.rect(8, 2, 9, 3, R, fill=True)
    c.rect(12, 18, 13, 19, G, fill=True)


def startallback(c):
    c.ring(10.5, 10.5, 11, 2)
    c.rect(5, 5, 9, 9, G, fill=True)
    c.rect(12, 5, 16, 9, R, fill=True)
    c.rect(5, 12, 9, 16, R, fill=True)
    c.rect(12, 12, 16, 16, G, fill=True)


# A 3x5 font for the few drawings that carry letters.
FONT = {
    "A": ["GGG", "G.G", "GGG", "G.G", "G.G"], "C": ["GGG", "G..", "G..", "G..", "GGG"], "E": ["GGG", "G..", "GG.", "G..", "GGG"],
    "J": ["..G", "..G", "..G", "G.G", "GGG"], "N": ["G.G", "GGG", "GGG", "G.G", "G.G"], "S": ["GGG", "G..", "GGG", "..G", "GGG"],
    "T": ["GGG", ".G.", ".G.", ".G.", ".G."], "+": ["...", ".G.", "GGG", ".G.", "..."], ".": ["...", "...", "...", "...", ".G."],
}


def word(c, text, x, y, col=G):
    for ch in text:
        for j, row in enumerate(FONT[ch]):
            for i, p in enumerate(row):
                if p == "G":
                    c.dot(x + i, y + j, col)
        x += 4


def window(c, t=2):
    c.rrect(0, 2, 21, 19, t=t)
    c.rect(0, 2, 21, 4, fill=True)


def magnifier(c, cx=8.5, cy=8.5, r=8.5):
    c.ring(cx, cy, r, 2)
    c.line(cx + r * 0.66, cy + r * 0.66, 20.5, 20.5, t=3)


def gamepad(c):
    c.rrect(0, 5, 21, 16, t=2, r=4)
    c.rect(4, 8, 5, 13, fill=True), c.rect(2, 10, 7, 11, fill=True)


def rainmeter(c):
    pts = [(10.5, 0), (4, 10), (3, 14)]
    c.line(10.5, 0, 3.5, 11, t=2), c.line(10.5, 0, 17.5, 11, t=2)
    c.ring(10.5, 14, 7.5, 2, a0=350, a1=190)
    c.ring(10.5, 14, 4, 2, R, a0=20, a1=110)


def redlauncher(c):
    c.line(10.5, 0, 20, 5.5, t=2), c.line(20, 5.5, 20, 15.5, t=2), c.line(20, 15.5, 10.5, 21, t=2)
    c.line(10.5, 21, 1, 15.5, t=2), c.line(1, 15.5, 1, 5.5, t=2), c.line(1, 5.5, 10.5, 0, t=2)
    c.line(12, 4, 7, 11, R, t=2), c.line(7, 11, 14, 11, R, t=2), c.line(14, 11, 9, 18, R, t=2)


def intel_npu(c):
    c.rrect(3, 3, 18, 18, t=2, r=2)
    for i in (6, 10, 14):
        for (a, b) in ((0, 2), (19, 21)):
            c.rect(i, a, i + 1, b, fill=True), c.rect(a, i, b, i + 1, fill=True)
    nodes = [(7, 7), (14, 7), (7, 14), (14, 14), (10.5, 10.5)]
    for n in nodes[:4]:
        c.line(*n, *nodes[4])
    for n in nodes:
        c.rect(int(n[0]) - 1, int(n[1]) - 1, int(n[0]), int(n[1]), R, fill=True)


def mouzi(c):
    c.rrect(4, 0, 17, 21, t=2, r=4)
    c.rect(10, 0, 11, 8, fill=True), c.rect(4, 8, 17, 9, fill=True)
    c.rect(10, 3, 11, 5, R, fill=True)


def mozilla(c):
    c.line(3, 18, 13, 8, t=3)
    c.ring(15, 6, 5.5, 2, a0=0, a1=240)
    c.rect(1, 17, 4, 20, R, fill=True)


def badge(c, text, col=R):
    c.rrect(0, 3, 21, 18, t=2, r=3)
    word(c, text, 11 - (len(text) * 4 - 1) // 2, 8, col)


def dotnet(c): badge(c, ".NET")
def aspnet(c):
    badge(c, ".NET", G)
    c.rect(17, 5, 18, 6, R, fill=True)
def vcredist(c): badge(c, "C++")
def winsdk(c):
    c.line(6, 2, 3, 2, t=2), c.line(3, 2, 3, 19, t=2), c.line(3, 19, 6, 19, t=2), c.line(1, 10, 3, 10, t=2)
    c.line(15, 2, 18, 2, t=2), c.line(18, 2, 18, 19, t=2), c.line(18, 19, 15, 19, t=2), c.line(18, 10, 20, 10, t=2)
    c.rect(8, 8, 13, 13, R, fill=True)


def zen(c):
    c.ring(10.5, 10.5, 11, 2)
    c.ring(10.5, 10.5, 7, 2)
    c.disc(10.5, 10.5, 2.5, R)


def sizer(c):
    window(c)
    c.line(5, 16, 16, 7, R, t=2)
    c.line(5, 16, 9, 16, R, t=2), c.line(5, 16, 5, 12, R, t=2)
    c.line(16, 7, 12, 7, R, t=2), c.line(16, 7, 16, 11, R, t=2)


def tacky(c):
    c.rect(3, 4, 18, 17, t=2)
    for (x, y) in ((0, 1), (16, 1), (0, 15), (16, 15)):
        c.rect(x, y, x + 5, y + 5, R, t=2)


def tourne_menu(c):
    window(c)
    c.line(4, 8, 7, 10, t=2), c.line(7, 10, 4, 12, t=2)
    for y in (8, 12, 16):
        c.rect(10, y, 17, y, R if y == 12 else G, fill=True)


def node(c):
    c.line(10.5, 0, 20, 5.5, t=2), c.line(20, 5.5, 20, 15.5, t=2), c.line(20, 15.5, 10.5, 21, t=2)
    c.line(10.5, 21, 1, 15.5, t=2), c.line(1, 15.5, 1, 5.5, t=2), c.line(1, 5.5, 10.5, 0, t=2)
    word(c, "JS", 7, 8, R)


def afterburner(c):
    c.ring(10.5, 12, 10, 2, a0=150, a1=30)
    for a in (180, 225, 270, 315, 360):
        x, y = 10.5 + 7 * math.cos(math.radians(a)), 12 + 7 * math.sin(math.radians(a))
        c.dot(x, y)
    c.line(10.5, 12, 16, 6, R, t=2)
    c.disc(10.5, 12, 2, G)
    c.line(5, 19, 16, 19, t=2)


def devps(c):
    c.line(6, 3, 21, 3, t=2), c.line(21, 3, 15, 18, t=2), c.line(15, 18, 0, 18, t=2), c.line(0, 18, 6, 3, t=2)
    c.line(7, 7, 11, 10.5, R, t=2), c.line(11, 10.5, 7, 14, R, t=2), c.rect(11, 13, 14, 14, R, fill=True)


def certkit(c):
    c.ring(10.5, 8, 7.5, 2)
    c.disc(10.5, 8, 3, R)
    c.line(6, 13, 3, 21, t=2), c.line(3, 21, 7, 19, t=2), c.line(7, 19, 9, 21, t=2)
    c.line(15, 13, 18, 21, t=2), c.line(18, 21, 14, 19, t=2), c.line(14, 19, 12, 21, t=2)


def appverifier(c):
    c.shape([(3, 0, 18, 21)], G)
    for y in (4, 7):
        c.line(6, y, 15, y)
    c.line(6, 14, 9, 17, R, t=2), c.line(9, 17, 16, 10, R, t=2)


def flow(c):
    magnifier(c)
    c.line(5, 8, 12, 8, R, t=2), c.line(9, 5, 12, 8, R, t=2), c.line(9, 11, 12, 8, R, t=2)


def magnify(c):
    magnifier(c)
    c.rect(5, 8, 12, 9, R, fill=True), c.rect(8, 5, 9, 12, R, fill=True)


def searchindexer(c):
    for y in (2, 6, 10):
        c.rect(0, y, 13, y + 1, fill=True)
    magnifier(c, 12.5, 12.5, 6.5)
    c.disc(12.5, 12.5, 2, R)


def cinebench(c):
    c.line(10.5, 1, 20, 6, t=2), c.line(20, 6, 10.5, 11, t=2), c.line(10.5, 11, 1, 6, t=2), c.line(1, 6, 10.5, 1, t=2)
    c.line(1, 6, 1, 16, t=2), c.line(20, 6, 20, 16, t=2), c.line(10.5, 11, 10.5, 21, t=2)
    c.line(1, 16, 10.5, 21, t=2), c.line(20, 16, 10.5, 21, t=2)
    c.line(10.5, 4, 15, 6, R, t=2), c.line(6, 6, 10.5, 8, R, t=2)


def latencymon(c):
    c.ring(10.5, 12, 9.5, 2)
    c.rect(8, 0, 13, 1, fill=True), c.rect(10, 1, 11, 2, fill=True)
    c.line(10.5, 12, 10.5, 6, R, t=2), c.line(10.5, 12, 15, 14, R, t=2)


def ea(c):
    c.line(0, 4, 9, 4, t=2), c.line(0, 4, 0, 17, t=2), c.line(0, 17, 9, 17, t=2), c.line(0, 10.5, 7, 10.5, t=2)
    c.line(11, 17, 16, 4, R, t=2), c.line(16, 4, 21, 17, R, t=2), c.line(13, 12, 19, 12, R, t=2)


def ltk(c):
    c.line(3, 18, 18, 3, t=2)
    c.line(18, 3, 21, 0, t=2)
    c.line(3, 12, 9, 18, R, t=2)
    c.line(0, 21, 4, 17, t=3)


def soulframe(c):
    c.ring(10.5, 10.5, 10.5, 2, a0=60, a1=300)
    c.ring(15, 10.5, 8, 2, a0=110, a1=250)
    c.disc(7, 10.5, 2, R)


def controller(c):
    gamepad(c)
    c.rect(13, 8, 14, 9, R, fill=True), c.rect(16, 10, 17, 11, R, fill=True)


def ea_controller(c): controller(c)


def photos(c):
    c.rrect(0, 2, 21, 19, t=2)
    c.line(3, 16, 8, 9, t=2), c.line(8, 9, 12, 14, t=2), c.line(12, 14, 14, 12, t=2), c.line(14, 12, 18, 16, t=2)
    c.disc(15, 7, 2, R)


def eagle(c):
    c.line(0, 6, 10.5, 15, t=2), c.line(21, 6, 10.5, 15, t=2)
    c.line(0, 6, 5, 4, t=2), c.line(21, 6, 16, 4, t=2)
    c.line(5, 4, 10.5, 9, t=2), c.line(16, 4, 10.5, 9, t=2)
    c.rect(9, 15, 12, 18, R, fill=True)


def jpegview(c):
    photos(c)
    c.ring(5, 7, 3.5, 1, R)


def topaz(c):
    c.rect(0, 3, 21, 18, t=2)
    for x in range(3, 20, 4):
        c.rect(x, 5, x + 1, 6, fill=True), c.rect(x, 15, x + 1, 16, fill=True)
    c.line(10.5, 7, 10.5, 14, R, t=1), c.line(7, 10.5, 14, 10.5, R, t=1)
    c.line(8.5, 8.5, 12.5, 12.5, R), c.line(12.5, 8.5, 8.5, 12.5, R)


def kontext(c):
    c.line(19, 1, 9, 13, t=3)
    c.ring(6, 16, 5, 2, R, a0=90, a1=360)
    c.rect(1, 17, 5, 21, R, fill=True)


def filehistory(c):
    c.ring(10.5, 10.5, 10.5, 2, a0=200, a1=160)
    c.line(0, 6, 1, 11, t=2), c.line(1, 11, 5, 8, t=2)
    c.line(10.5, 10.5, 10.5, 5, R, t=2), c.line(10.5, 10.5, 15, 13, R, t=2)


def peace(c):
    for i, (x, k) in enumerate(((3, 13), (10, 6), (17, 10))):
        c.rect(x, 0, x + 1, 21, fill=True)
        c.rect(x - 2, k, x + 3, k + 3, R, fill=True)


def powerautomate(c):
    c.line(1, 2, 10, 10.5, t=3), c.line(10, 10.5, 1, 19, t=3)
    c.line(10, 2, 19, 10.5, R, t=3), c.line(19, 10.5, 10, 19, R, t=3)


def convertify(c):
    c.line(1, 6, 20, 6, t=2), c.line(20, 6, 15, 1, t=2)
    c.line(20, 15, 1, 15, R, t=2), c.line(1, 15, 6, 20, R, t=2)


def fileconverter(c):
    c.shape([(2, 0, 15, 21)], G)
    for y in (4, 7):
        c.line(5, y, 12, y)
    c.ring(14, 15, 6.5, 2, R, a0=200, a1=110)
    c.line(19, 9, 20, 13, R, t=2)


def vscodium(c):
    vscode(c)
    c.rect(16, 2, 18, 19, G, fill=True)
    c.rect(17, 2, 17, 19, R, fill=True)


def gear(c, cx, cy, r, col=G):
    c.ring(cx, cy, r, 1.6, col)
    for a in range(0, 360, 60):
        x, y = cx + (r + 1.3) * math.cos(math.radians(a)), cy + (r + 1.3) * math.sin(math.radians(a))
        c.dot(x, y, col)


def servicehost(c):
    window(c)
    gear(c, 10.5, 12, 4.2, R)
    c.disc(10.5, 12, 1.2, G)


def defender(c):
    c.line(10.5, 0, 20, 3, t=2), c.line(10.5, 0, 1, 3, t=2)
    c.line(20, 3, 19, 12, t=2), c.line(1, 3, 2, 12, t=2)
    c.line(19, 12, 10.5, 21, t=2), c.line(2, 12, 10.5, 21, t=2)
    c.line(6, 10, 9, 13, R, t=2), c.line(9, 13, 15, 7, R, t=2)


def search(c):
    magnifier(c)
    c.line(8.5, 4.5, 8.5, 12.5, R), c.line(4.5, 8.5, 12.5, 8.5, R)
    c.line(6, 6, 11, 11, R), c.line(11, 6, 6, 11, R)


def gamingservices(c):
    gamepad(c)
    c.ring(15.5, 10.5, 3.2, 1.4, R)
    c.dot(15.5, 10.5, R)


def shellhost(c):
    c.rrect(5, 0, 21, 13, t=2)
    c.rrect(0, 7, 16, 21, t=2)
    c.rect(2, 9, 14, 11, R, fill=True)


def service(c):
    gear(c, 10.5, 10.5, 7.5, G)
    c.disc(10.5, 10.5, 2.4, R)


def interrupts(c):
    c.rrect(3, 3, 18, 18, t=2, r=2)
    for i in (7, 13):
        for (a, b) in ((0, 2), (19, 21)):
            c.rect(i, a, i + 1, b, fill=True), c.rect(a, i, b, i + 1, fill=True)
    c.line(12, 6, 8, 11, R, t=2), c.line(8, 11, 13, 11, R, t=2), c.line(13, 11, 9, 16, R, t=2)


def program(c):
    window(c)
    c.rect(4, 8, 13, 9, R, fill=True), c.rect(4, 12, 16, 13, R, fill=True), c.rect(4, 15, 10, 16, R, fill=True)


# Store apps (package rows: their logos everywhere, Start's Apps folder too)
def settings(c):
    gear(c, 10.5, 10.5, 7.5, G)
    c.ring(10.5, 10.5, 3.5, 2, R)


def snipping(c):
    c.ring(5, 16, 4, 2, R), c.ring(16, 16, 4, 2, R)
    c.line(7, 13, 15, 1, t=2), c.line(14, 13, 6, 1, t=2)


def nvidia(c):
    c.ring(10.5, 17, 12, 2, a0=215, a1=325)
    c.ring(10.5, 4, 12, 2, a0=35, a1=145)
    c.disc(10.5, 10.5, 3, R)


def xbox(c):
    c.ring(10.5, 10.5, 11, 2)
    c.line(6, 6, 15, 15, R, t=2), c.line(15, 6, 6, 15, R, t=2)


def calculator(c):
    c.rrect(3, 0, 18, 21)
    c.rect(6, 3, 15, 6, R, fill=True)
    for y in (10, 14, 18):
        for x in (6, 10, 14):
            c.rect(x, y - 1, x + 1, y, fill=True)


def camera(c):
    c.rrect(0, 5, 21, 19)
    c.rect(6, 2, 13, 5, t=2)
    c.ring(10.5, 12, 5, 2)
    c.disc(10.5, 12, 1.8, R)
    c.rect(16, 8, 17, 9, R, fill=True)


def clicktodo(c):
    c.line(3, 3, 3, 18, t=2), c.line(3, 3, 14, 14, t=2), c.line(3, 18, 7, 14, t=2), c.line(7, 14, 14, 14, t=2)
    c.line(17, 1, 17, 9, R), c.line(13, 5, 21, 5, R)


def clock(c):
    c.ring(10.5, 10.5, 11, 2)
    c.line(10.5, 10.5, 10.5, 4, t=2), c.line(10.5, 10.5, 15, 13, R, t=2)


def dolby(c):
    c.line(1, 2, 1, 19, t=2), c.ring(1, 10.5, 9, 2, a0=270, a1=90)
    c.line(20, 2, 20, 19, R, t=2), c.ring(20, 10.5, 9, 2, R, a0=90, a1=270)


def gamebar(c):
    gamepad(c)
    c.rect(13, 8, 18, 9, R, fill=True), c.rect(13, 12, 16, 13, R, fill=True)


def gethelp(c):
    c.ring(10.5, 10.5, 11, 2)
    c.ring(10.5, 8, 4, 2, R, a0=180, a1=90)
    c.line(10.5, 12, 10.5, 13.5, R, t=2)
    c.rect(10, 16, 11, 17, R, fill=True)


def backup(c):
    c.shape([(1, 11, 20, 18), (3, 7, 11, 18), (8, 3, 17, 18)])
    c.line(10.5, 16, 10.5, 9, R, t=2), c.line(10.5, 8, 7, 11, R, t=2), c.line(10.5, 8, 14, 11, R, t=2)


def mediaplayer(c):
    c.ring(10.5, 10.5, 11, 2)
    c.line(8, 6, 8, 15, R, t=2), c.line(8, 6, 15, 10.5, R, t=2), c.line(8, 15, 15, 10.5, R, t=2)


def designer(c):
    pts = [(10.5, 0), (13, 8), (21, 10.5), (13, 13), (10.5, 21), (8, 13), (0, 10.5), (8, 8), (10.5, 0)]
    for a, b in zip(pts, pts[1:]):
        c.line(*a, *b, t=2)
    c.disc(10.5, 10.5, 1.6, R)


def copilot(c):
    c.rrect(1, 5, 13, 20, t=2, r=4)
    c.rrect(8, 1, 20, 16, R, t=2, r=4)


def store(c):
    c.rrect(1, 6, 20, 21)
    c.ring(10.5, 6, 4.5, 2, a0=180, a1=360)
    for (x, y) in ((6, 10), (12, 10), (6, 15), (12, 15)):
        c.rect(x, y, x + 3, y + 3, R, fill=True)


def minecraft(c):
    c.rect(1, 1, 20, 20, t=2)
    c.rect(3, 3, 18, 7, fill=True)
    c.rect(5, 8, 6, 10, fill=True), c.rect(11, 8, 12, 9, fill=True), c.rect(15, 8, 16, 11, fill=True)
    c.rect(8, 13, 9, 14, R, fill=True), c.rect(14, 15, 15, 16, R, fill=True), c.rect(5, 16, 6, 17, R, fill=True)


def notepad(c):
    c.rrect(3, 1, 18, 21)
    for y in (6, 10, 14):
        c.rect(6, y, 15, y + 1, fill=True)
    c.rect(6, 17, 11, 18, R, fill=True)
    for x in (6, 10, 14):
        c.rect(x, 0, x + 1, 3, R, fill=True)


def paint(c):
    c.ring(10.5, 10.5, 11, 2)
    c.disc(6, 8, 1.6, R), c.disc(11, 5, 1.6, R), c.disc(16, 8, 1.6, R)
    c.disc(14, 15, 2.2)


def phonelink(c):
    c.rrect(5, 0, 16, 21)
    c.rect(9, 3, 12, 3, fill=True)
    c.rect(10, 17, 11, 18, R, fill=True)


def realtek(c):
    c.rect(1, 7, 5, 14, t=2)
    c.line(5, 7, 11, 2, t=2), c.line(5, 14, 11, 19, t=2), c.line(11, 2, 11, 19, t=2)
    c.ring(11, 10.5, 5, 2, R, a0=300, a1=60), c.ring(11, 10.5, 9.5, 2, R, a0=310, a1=50)


def wintoys(c):
    c.rect(1, 1, 9, 9, t=2), c.rect(12, 1, 20, 9, t=2), c.rect(1, 12, 9, 20, t=2)
    c.rect(12, 12, 20, 20, R, t=2)


def wirelessdisplay(c):
    c.rrect(0, 1, 21, 16)
    c.rect(9, 17, 12, 18, fill=True), c.rect(6, 19, 15, 20, fill=True)
    c.ring(10.5, 13, 3, 2, R, a0=200, a1=340), c.ring(10.5, 13, 7, 2, R, a0=215, a1=325)


def sharex(c):
    c.ring(10.5, 10.5, 11, 2)
    c.ring(10.5, 10.5, 6, 2, R)
    c.disc(10.5, 10.5, 1.6)


def getstarted(c):
    c.line(4, 1, 4, 21, t=2)
    c.rect(6, 2, 18, 11, R, t=2)


def resume(c):
    c.rrect(0, 1, 15, 13)
    c.rect(6, 14, 9, 15, fill=True), c.rect(3, 16, 12, 17, fill=True)
    c.rrect(14, 7, 21, 21, R)
    c.rect(17, 18, 18, 18, R, fill=True)


DRAWINGS = {
    "Terminal": terminal, "Google Chrome": chrome, "Microsoft Edge": edge, "VS Code": vscode, "Windhawk": windhawk,
    "Discord": discord, "Task Manager": taskmanager, "Everything": everything, "HWiNFO": hwinfo,
    "Notepad++": notepadpp, "Python": python, "StartAllBack": startallback,
    "Rainmeter": rainmeter, "REDlauncher": redlauncher, "Intel NPU": intel_npu, "Mouzi": mouzi, "Mozilla Maintenance": mozilla,
    "NET Runtime": dotnet, "ASP.NET": aspnet, "Visual C++": vcredist, "Windows SDK": winsdk, "Zen Browser": zen,
    "Sizer": sizer, "tacky-borders": tacky, "Tourne terminal menu": tourne_menu, "Node.js": node,
    "MSI Afterburner": afterburner, "Developer PowerShell": devps, "App Cert Kit": certkit, "App Verifier": appverifier,
    "Flow Launcher": flow, "Magnify": magnify, "Search Indexer": searchindexer, "Cinebench": cinebench,
    "LatencyMon": latencymon, "EA": ea, "LTK Manager": ltk, "Soulframe": soulframe, "Controller Companion": controller,
    "Photos": photos, "Eagle": eagle, "JPEGView": jpegview, "Topaz Video": topaz, "Kontext": kontext,
    "File History": filehistory, "Peace Equalizer": peace, "Power Automate": powerautomate, "Convertify": convertify,
    "File Converter": fileconverter, "VSCodium": vscodium,
    "Service Host": servicehost, "Microsoft Defender": defender, "Search": search, "Gaming Services": gamingservices,
    "Shell Infrastructure Host": shellhost, "Service": service, "System Interrupts": interrupts,
    "Program": program,
    "Settings": settings, "Snipping Tool": snipping, "NVIDIA Control Panel": nvidia, "Xbox": xbox,
    "Calculator": calculator, "Camera": camera, "Click to Do": clicktodo, "Clock": clock, "Dolby Access": dolby,
    "Game Bar": gamebar, "Get Help": gethelp, "Windows Backup": backup, "Media Player": mediaplayer,
    "Microsoft Designer": designer, "Copilot": copilot, "Microsoft Store": store, "Minecraft": minecraft,
    "Notepad": notepad, "Paint": paint, "Phone Link": phonelink, "Realtek Audio Console": realtek,
    "Wintoys": wintoys, "Wireless Display": wirelessdisplay, "ShareX": sharex,
    "Get Started": getstarted, "Resume": resume,
}


def file_name(name):
    return "".join(ch if ch.isalnum() else "-" for ch in name.replace("+", "p")).strip("-") + ".ico"


if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "big")
    os.makedirs(out, exist_ok=True)
    from PIL import ImageDraw, ImageFont
    font = ImageFont.truetype("consola.ttf", 12)
    cols, cw, chh = 10, 120, 150
    rows = (len(DRAWINGS) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * cw, rows * chh), (22, 26, 30))
    d = ImageDraw.Draw(sheet)
    for i, (name, fn) in enumerate(DRAWINGS.items()):
        cv = Canvas()
        fn(cv)
        im = render(cv)
        save_ico(im, os.path.join(out, file_name(name)), cv)
        x, y = (i % cols) * cw, (i // cols) * chh
        big = im.resize((96, 96), Image.NEAREST)
        sheet.paste(big, (x + 12, y + 8), big)
        sheet.paste(im, (x + 48, y + 110), im)
        d.text((x + 4, y + 134), name[:16], fill=(200, 200, 200), font=font)
    sheet.save(os.path.join(out, "preview.png"))
    print(out)
