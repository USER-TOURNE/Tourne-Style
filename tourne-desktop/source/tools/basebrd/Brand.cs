using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;

// The basebrd art for "// WINDux.95" in the Tourne style: one pixel-art
// master drawn at 1x, then scaled by whole numbers (6x, 5x, 4x) to the
// three sizes Windows uses, so every pixel stays a hard square.
public static class Brand {
    static readonly Color Ground = Color.FromArgb(0x12, 0x1C, 0x21);
    static readonly Color SageAlt = Color.FromArgb(0x5D, 0x8D, 0x6F);
    static readonly Color Ink = Color.FromArgb(0x66, 0xEA, 0x97);
    static readonly Color Coral = Color.FromArgb(0xFF, 0x6F, 0x77);

    // The design's "pixel shade": the darker step under each lit pixel.
    static Color ShadeOf(Color c) {
        if (c.ToArgb() == Ink.ToArgb()) return Color.FromArgb(0x3C, 0x86, 0x5C);
        if (c.ToArgb() == Coral.ToArgb()) return Color.FromArgb(0xA9, 0x5D, 0x5D);
        if (c.ToArgb() == SageAlt.ToArgb()) return Color.FromArgb(0x36, 0x6E, 0x53);
        return c;
    }

    // Text drawn one-bit (no smoothing) into a mask, trimmed to its pixels.
    static bool[,] TextMask(string text, Font font, out int w, out int h) {
        var tmp = new Bitmap(400, 60);
        using (var g = Graphics.FromImage(tmp)) {
            g.Clear(Color.Black);
            g.TextRenderingHint = TextRenderingHint.SingleBitPerPixelGridFit;
            g.DrawString(text, font, Brushes.White, 2, 2, StringFormat.GenericTypographic);
        }
        int minX = int.MaxValue, minY = int.MaxValue, maxX = -1, maxY = -1;
        for (int y = 0; y < tmp.Height; y++)
            for (int x = 0; x < tmp.Width; x++)
                if (tmp.GetPixel(x, y).R > 127) {
                    minX = Math.Min(minX, x); maxX = Math.Max(maxX, x);
                    minY = Math.Min(minY, y); maxY = Math.Max(maxY, y);
                }
        w = maxX - minX + 1; h = maxY - minY + 1;
        var m = new bool[w, h];
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++) m[x, y] = tmp.GetPixel(minX + x, minY + y).R > 127;
        tmp.Dispose();
        return m;
    }

    // Baseline offset of a piece of text, so pieces with and without
    // descenders line up: rows from the top of "H" to the piece's top.
    static int TopOffset(string text, Font font) {
        var tmp = new Bitmap(400, 60);
        int top = -1, refTop = -1;
        using (var g = Graphics.FromImage(tmp)) {
            g.Clear(Color.Black);
            g.TextRenderingHint = TextRenderingHint.SingleBitPerPixelGridFit;
            g.DrawString(text, font, Brushes.White, 2, 2, StringFormat.GenericTypographic);
        }
        for (int y = 0; y < tmp.Height && top < 0; y++)
            for (int x = 0; x < tmp.Width; x++) if (tmp.GetPixel(x, y).R > 127) { top = y; break; }
        using (var g = Graphics.FromImage(tmp)) {
            g.Clear(Color.Black);
            g.TextRenderingHint = TextRenderingHint.SingleBitPerPixelGridFit;
            g.DrawString("H", font, Brushes.White, 2, 2, StringFormat.GenericTypographic);
        }
        for (int y = 0; y < tmp.Height && refTop < 0; y++)
            for (int x = 0; x < tmp.Width; x++) if (tmp.GetPixel(x, y).R > 127) { refTop = y; break; }
        tmp.Dispose();
        return top - refTop;
    }

    public static Bitmap Master(string fontFile, float fontPx, int W, int H, out string report) {
        var pfc = new PrivateFontCollection();
        pfc.AddFontFile(fontFile);
        var font = new Font(pfc.Families[0], fontPx, FontStyle.Regular, GraphicsUnit.Pixel);

        // Pieces of the wordmark and their colours.
        var pieces = new[] {
            Tuple.Create("//", SageAlt),
            Tuple.Create("WINDux", Ink),
            Tuple.Create(".95", Coral),
        };
        const int gapAfterSlashes = 3, gapWord = 1;
        var masks = new List<bool[,]>();
        var widths = new List<int>(); var heights = new List<int>(); var tops = new List<int>();
        int textW = 0, capH = 0;
        for (int i = 0; i < pieces.Length; i++) {
            int w, h;
            masks.Add(TextMask(pieces[i].Item1, font, out w, out h));
            widths.Add(w); heights.Add(h); tops.Add(TopOffset(pieces[i].Item1, font));
            textW += w + (i == 0 ? gapAfterSlashes : i < pieces.Length - 1 ? gapWord : 0);
        }
        int dummyW;
        TextMask("H", font, out dummyW, out capH);

        // The mark: a 2x2 window of coral panes, as tall as the capitals.
        int pane = Math.Max(4, (capH + 1) / 2);
        int markW = pane * 2 + 1, markH = pane * 2 + 1;
        const int gapMark = 5;

        int totalW = markW + gapMark + textW;
        int left = (W - totalW) / 2;
        int capTop = (H - capH) / 2;   // capitals centred vertically

        var bmp = new Bitmap(W, H, PixelFormat.Format32bppArgb);
        var lit = new Color?[W, H];
        // Mark, top aligned with the capitals.
        int my = capTop + (capH - markH) / 2;
        for (int py = 0; py < 2; py++)
            for (int px = 0; px < 2; px++)
                for (int y = 0; y < pane; y++)
                    for (int x = 0; x < pane; x++)
                        lit[left + px * (pane + 1) + x, my + py * (pane + 1) + y] = Coral;
        // Text.
        int cx = left + markW + gapMark;
        for (int i = 0; i < pieces.Length; i++) {
            var m = masks[i];
            for (int y = 0; y < heights[i]; y++)
                for (int x = 0; x < widths[i]; x++)
                    if (m[x, y]) {
                        int X = cx + x, Y = capTop + tops[i] + y;
                        if (X >= 0 && X < W && Y >= 0 && Y < H) lit[X, Y] = pieces[i].Item2;
                    }
            cx += widths[i] + (i == 0 ? gapAfterSlashes : gapWord);
        }
        // Paint: ground, then the shade step under each lit pixel, then the lit pixels.
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++) bmp.SetPixel(x, y, Ground);
        // The mark's own gaps stay open: no shade inside its square.
        for (int y = 0; y < H - 1; y++)
            for (int x = 0; x < W; x++) {
                bool inMark = x >= left && x < left + markW && y + 1 >= my && y + 1 < my + markH;
                if (lit[x, y].HasValue && !lit[x, y + 1].HasValue && !inMark) bmp.SetPixel(x, y + 1, ShadeOf(lit[x, y].Value));
            }
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (lit[x, y].HasValue) bmp.SetPixel(x, y, lit[x, y].Value);

        report = string.Format("font '{0}' {1}px: caps {2}px, text {3}px, mark {4}px, total {5}px of {6}",
                               pfc.Families[0].Name, fontPx, capH, textW, markW, totalW, W);
        return bmp;
    }

    // Whole-number scale onto the exact output size, centred, ground around it.
    public static void SaveScaled(Bitmap master, int scale, int outW, int outH, string path) {
        var outBmp = new Bitmap(outW, outH, PixelFormat.Format32bppArgb);
        using (var g = Graphics.FromImage(outBmp)) {
            g.Clear(Ground);
            g.InterpolationMode = InterpolationMode.NearestNeighbor;
            g.PixelOffsetMode = PixelOffsetMode.Half;
            int w = master.Width * scale, h = master.Height * scale;
            g.DrawImage(master, new Rectangle((outW - w) / 2, (outH - h) / 2, w, h));
        }
        outBmp.Save(path, ImageFormat.Png);
        outBmp.Dispose();
    }
}
