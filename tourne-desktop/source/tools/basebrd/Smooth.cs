using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;

// The same "// WINDux.95" design drawn natively at each size with smooth
// (anti-aliased) edges: a terminal font and a crisp square mark, with the
// Tourne shade as a solid darker step under everything.
public static class Smooth {
    static readonly Color Ground = Color.FromArgb(0x12, 0x1C, 0x21);
    static readonly Color SageAlt = Color.FromArgb(0x5D, 0x8D, 0x6F);
    static readonly Color Ink = Color.FromArgb(0x66, 0xEA, 0x97);
    static readonly Color Coral = Color.FromArgb(0xFF, 0x6F, 0x77);
    static readonly Color InkShade = Color.FromArgb(0x3C, 0x86, 0x5C);
    static readonly Color CoralShade = Color.FromArgb(0xA9, 0x5D, 0x5D);
    static readonly Color SageShade = Color.FromArgb(0x36, 0x6E, 0x53);

    public static string Render(string fontName, bool bold, int W, int H, string path) {
        var bmp = new Bitmap(W, H, PixelFormat.Format32bppArgb);
        using (var g = Graphics.FromImage(bmp)) {
            g.Clear(Ground);
            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.TextRenderingHint = TextRenderingHint.AntiAliasGridFit;
            g.PixelOffsetMode = PixelOffsetMode.HighQuality;

            // Sized from the height, like Bouquet: capitals about 38% of it.
            float em = H * 0.52f;
            var font = new Font(fontName, em, bold ? FontStyle.Bold : FontStyle.Regular, GraphicsUnit.Pixel);
            var fmt = StringFormat.GenericTypographic;
            fmt.FormatFlags |= StringFormatFlags.MeasureTrailingSpaces;
            string[] parts = { "//", " WINDux", ".95" };
            Color[] colors = { SageAlt, Ink, Coral };
            Color[] shades = { SageShade, InkShade, CoralShade };
            var sizes = new SizeF[parts.Length];
            float textW = 0;
            for (int i = 0; i < parts.Length; i++) { sizes[i] = g.MeasureString(parts[i], font, 10000, fmt); textW += sizes[i].Width; }

            // Cap height from the font metrics, to size and place the mark.
            var fam = font.FontFamily;
            var style = font.Style;
            float ascent = em * fam.GetCellAscent(style) / fam.GetEmHeight(style);
            float capH = em * 0.70f;   // close enough for these fonts; the mark is sized to it
            float mark = (float)Math.Round(capH * 1.08f);
            float gap = (float)Math.Round(mark * 0.12f);
            float pane = (mark - gap) / 2;
            float shade = Math.Max(2, (float)Math.Round(H / 27f));   // the darker step, in pixels
            float spacing = (float)Math.Round(mark * 0.55f);

            float total = mark + spacing + textW;
            float x = (float)Math.Round((W - total) / 2);
            float capTop = (float)Math.Round((H - capH) / 2);
            float baseline = capTop + capH;
            float textTop = baseline - ascent;
            float markTop = (float)Math.Round(capTop + (capH - mark) / 2);

            // Mark: shade step, then the four panes.
            using (var sb = new SolidBrush(CoralShade))
                g.FillRectangle(sb, x, markTop + mark - pane + shade - shade, mark, 0);   // (no-op, kept simple below)
            for (int py = 0; py < 2; py++)
                for (int px = 0; px < 2; px++) {
                    float rx = x + px * (pane + gap), ry = markTop + py * (pane + gap);
                    if (py == 1) using (var sb = new SolidBrush(CoralShade)) g.FillRectangle(sb, rx, ry + shade, pane, pane);
                    using (var b = new SolidBrush(Coral)) g.FillRectangle(b, rx, ry, pane, pane);
                }
            // Text: shade copy one step down, then the colour.
            float tx = x + mark + spacing;
            for (int i = 0; i < parts.Length; i++) {
                using (var sb = new SolidBrush(shades[i])) g.DrawString(parts[i], font, sb, tx, textTop + shade, fmt);
                using (var b = new SolidBrush(colors[i])) g.DrawString(parts[i], font, b, tx, textTop, fmt);
                tx += sizes[i].Width;
            }
            string report = string.Format("{0}{1} {2}x{3}: em {4:F0}px, mark {5}px, total {6:F0}px", fontName, bold ? " bold" : "",
                                          W, H, em, mark, total);
            bmp.Save(path, ImageFormat.Png);
            bmp.Dispose();
            return report;
        }
    }
}
