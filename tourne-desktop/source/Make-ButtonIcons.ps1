# Draws Tourne button icons in the Bouquet taskbar set's style (flat one-colour
# silhouettes, cut-out details, crisp edges) and writes multi-size .ico files.
param([string]$OutDir = "$env:LOCALAPPDATA\Tourne\Icons", [string]$Preview)
Add-Type -AssemblyName System.Drawing
$null = [Drawing.Bitmap]::new(1, 1)
$refs = [AppDomain]::CurrentDomain.GetAssemblies() | ? { $_.GetName().Name -match '^System\.(Drawing|Private\.Windows|Runtime|Collections|Private\.CoreLib|ComponentModel)' -and $_.Location } | % Location
$refs = @($refs) + [object].Assembly.Location + [System.Collections.Generic.List[int]].Assembly.Location | Select -Unique
Add-Type -ReferencedAssemblies $refs -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;

public static class TourneIcons {
    const int SS = 8;  // supersampling

    // Coverage 0..1 of a shape drawn by 'draw' into a 24-unit square, at 'size' pixels.
    static double[] Coverage(int size, Action<Graphics, float> draw) {
        int big = size * SS;
        using (var bmp = new Bitmap(big, big, PixelFormat.Format32bppArgb))
        using (var g = Graphics.FromImage(bmp)) {
            g.Clear(Color.Transparent);
            g.SmoothingMode = SmoothingMode.None;
            draw(g, big / 24f);
            var data = bmp.LockBits(new Rectangle(0, 0, big, big), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            var bytes = new byte[big * big * 4];
            System.Runtime.InteropServices.Marshal.Copy(data.Scan0, bytes, 0, bytes.Length);
            bmp.UnlockBits(data);
            var cov = new double[size * size];
            for (int y = 0; y < size; y++)
                for (int x = 0; x < size; x++) {
                    int n = 0;
                    for (int sy = 0; sy < SS; sy++)
                        for (int sx = 0; sx < SS; sx++)
                            if (bytes[((y * SS + sy) * big + x * SS + sx) * 4 + 3] > 127) n++;
                    cov[y * size + x] = n / (double)(SS * SS);
                }
            return cov;
        }
    }

    // Crisp like the set: solid inside, a faint step at the edge, nothing past it.
    static byte Snap(double a) {
        if (a >= 0.55) return 255;
        if (a >= 0.25) return 115;
        return 0;
    }

    static Bitmap Paint(int size, double[] cov, Color color) {
        var bmp = new Bitmap(size, size, PixelFormat.Format32bppArgb);
        for (int y = 0; y < size; y++)
            for (int x = 0; x < size; x++)
                bmp.SetPixel(x, y, Color.FromArgb(Snap(cov[y * size + x]), color));
        return bmp;
    }

    static readonly Brush White = Brushes.White;

    // Claude: a burst of rays around a round middle.
    public static Bitmap Claude(int size, Color color) {
        int rays = size <= 16 ? 8 : 10;
        var cov = Coverage(size, (g, u) => {
            float c = 12f * u;
            float core = (size <= 16 ? 3.6f : 4.2f) * u;
            g.FillEllipse(White, c - core, c - core, core * 2, core * 2);
            for (int i = 0; i < rays; i++) {
                double angle = Math.PI * 2 * i / rays - Math.PI / 2;
                float length = (i % 2 == 0 ? 11.4f : 9.2f) * u;
                float half = (size <= 16 ? 1.7f : 1.6f) * u;
                float dx = (float)Math.Cos(angle), dy = (float)Math.Sin(angle);
                float px = -dy * half, py = dx * half;
                var ray = new PointF[] {
                    new PointF(c + px, c + py), new PointF(c + dx * length + px * 0.75f, c + dy * length + py * 0.75f),
                    new PointF(c + dx * length - px * 0.75f, c + dy * length - py * 0.75f), new PointF(c - px, c - py),
                };
                g.FillPolygon(White, ray);
            }
        });
        return Paint(size, cov, color);
    }

    // GitHub: the octocat cut out of a round badge, as the mark is drawn.
    public static Bitmap GitHub(int size, Color color) {
        var badge = Coverage(size, (g, u) => g.FillEllipse(White, 0.6f * u, 0.6f * u, 22.8f * u, 22.8f * u));
        var cat = Coverage(size, (g, u) => {
            g.FillEllipse(White, 6.3f * u, 6.4f * u, 11.4f * u, 8.8f * u);                       // head
            g.FillPolygon(White, new[] { new PointF(6.6f * u, 9.5f * u), new PointF(7.1f * u, 4.3f * u), new PointF(10.6f * u, 7.0f * u) });
            g.FillPolygon(White, new[] { new PointF(17.4f * u, 9.5f * u), new PointF(16.9f * u, 4.3f * u), new PointF(13.4f * u, 7.0f * u) });
            g.FillRectangle(White, 9.7f * u, 13.5f * u, 4.6f * u, 9.9f * u);                     // body, to the badge's edge
            g.FillPolygon(White, new[] {                                                           // tail
                new PointF(9.9f * u, 17.2f * u), new PointF(7.4f * u, 17.0f * u), new PointF(5.4f * u, 15.0f * u),
                new PointF(4.6f * u, 16.0f * u), new PointF(6.8f * u, 18.6f * u), new PointF(9.9f * u, 19.0f * u) });
        });
        var cov = new double[size * size];
        for (int i = 0; i < cov.Length; i++) cov[i] = badge[i] * (1 - cat[i]);
        return Paint(size, cov, color);
    }

    // Settings: a gear with chunky teeth and a round hole.
    public static Bitmap Gear(int size, Color color) {
        int teeth = size <= 16 ? 6 : 8;
        var gear = Coverage(size, (g, u) => {
            float c = 12f * u;
            float body = 7.6f * u;
            g.FillEllipse(White, c - body, c - body, body * 2, body * 2);
            for (int i = 0; i < teeth; i++) {
                double angle = Math.PI * 2 * i / teeth;
                float dx = (float)Math.Cos(angle), dy = (float)Math.Sin(angle);
                float half = (size <= 16 ? 2.3f : 1.9f) * u, reach = 11.4f * u;
                float px = -dy * half, py = dx * half;
                g.FillPolygon(White, new PointF[] {
                    new PointF(c + px, c + py), new PointF(c + dx * reach + px, c + dy * reach + py),
                    new PointF(c + dx * reach - px, c + dy * reach - py), new PointF(c - px, c - py) });
            }
        });
        var hole = Coverage(size, (g, u) => {
            float c = 12f * u, r = (size <= 16 ? 3.4f : 3.6f) * u;
            g.FillEllipse(White, c - r, c - r, r * 2, r * 2);
        });
        var cov = new double[size * size];
        for (int i = 0; i < cov.Length; i++) cov[i] = gear[i] * (1 - hole[i]);
        return Paint(size, cov, color);
    }

    // Snipping Tool: open scissors, two ring handles below crossed blades.
    public static Bitmap Scissors(int size, Color color) {
        bool small = size <= 16;
        var solid = Coverage(size, (g, u) => {
            float ring = (small ? 5.0f : 4.8f) * u;
            g.FillEllipse(White, 6.2f * u - ring, 18.0f * u - ring, ring * 2, ring * 2);
            g.FillEllipse(White, 17.8f * u - ring, 18.0f * u - ring, ring * 2, ring * 2);
            float half = (small ? 2.4f : 2.1f) * u;
            // Each blade from its handle, through the pivot, to a point.
            foreach (var b in new[] { new[] { 8.6f, 13.6f, 18.2f, 0.8f }, new[] { 15.4f, 13.6f, 5.8f, 0.8f } }) {
                float x0 = b[0] * u, y0 = b[1] * u, x1 = b[2] * u, y1 = b[3] * u;
                float dx = x1 - x0, dy = y1 - y0, len = (float)Math.Sqrt(dx * dx + dy * dy);
                float px = -dy / len * half, py = dx / len * half;
                g.FillPolygon(White, new PointF[] {
                    new PointF(x0 + px, y0 + py), new PointF(x1 + px * 0.45f, y1 + py * 0.45f),
                    new PointF(x1 - px * 0.45f, y1 - py * 0.45f), new PointF(x0 - px, y0 - py) });
            }
        });
        var holes = Coverage(size, (g, u) => {
            float hole = (small ? 2.0f : 2.1f) * u;
            g.FillEllipse(White, 6.2f * u - hole, 18.0f * u - hole, hole * 2, hole * 2);
            g.FillEllipse(White, 17.8f * u - hole, 18.0f * u - hole, hole * 2, hole * 2);
        });
        var cov = new double[size * size];
        for (int i = 0; i < cov.Length; i++) cov[i] = solid[i] * (1 - holes[i]);
        return Paint(size, cov, color);
    }

    // A .ico with 32-bit images (plus their AND masks) at each size.
    public static void WriteIco(string path, Bitmap[] images) {
        using (var ms = new MemoryStream()) {
            var w = new BinaryWriter(ms);
            w.Write((ushort)0); w.Write((ushort)1); w.Write((ushort)images.Length);
            var blobs = new byte[images.Length][];
            int k = 0;
            foreach (var bmp in images) {
                int s = bmp.Width;
                int maskStride = ((s + 31) / 32) * 4;
                var b = new MemoryStream();
                var bw = new BinaryWriter(b);
                bw.Write(40); bw.Write(s); bw.Write(s * 2); bw.Write((ushort)1); bw.Write((ushort)32);
                bw.Write(0); bw.Write(s * s * 4 + maskStride * s); bw.Write(0); bw.Write(0); bw.Write(0); bw.Write(0);
                for (int y = s - 1; y >= 0; y--)
                    for (int x = 0; x < s; x++) {
                        var c = bmp.GetPixel(x, y);
                        bw.Write(c.B); bw.Write(c.G); bw.Write(c.R); bw.Write(c.A);
                    }
                for (int y = s - 1; y >= 0; y--) {
                    var row = new byte[maskStride];
                    for (int x = 0; x < s; x++)
                        if (bmp.GetPixel(x, y).A == 0) row[x / 8] |= (byte)(0x80 >> (x % 8));
                    bw.Write(row);
                }
                blobs[k++] = b.ToArray();
            }
            int offset = 6 + 16 * images.Length;
            for (int i = 0; i < images.Length; i++) {
                int s = images[i].Width;
                w.Write((byte)(s >= 256 ? 0 : s)); w.Write((byte)(s >= 256 ? 0 : s));
                w.Write((byte)0); w.Write((byte)0); w.Write((ushort)1); w.Write((ushort)32);
                w.Write(blobs[i].Length); w.Write(offset);
                offset += blobs[i].Length;
            }
            foreach (var blob in blobs) w.Write(blob);
            File.WriteAllBytes(path, ms.ToArray());
        }
    }
}
'@

$coral = [Drawing.Color]::FromArgb(0xF0, 0x69, 0x6F)
$slate = [Drawing.Color]::FromArgb(0x71, 0x8C, 0x97)
$teal = [Drawing.Color]::FromArgb(0x5F, 0x99, 0x93)
$mauve = [Drawing.Color]::FromArgb(0xB5, 0x86, 0xB0)
$sizes = 16, 24, 32, 48
New-Item -ItemType Directory -Force $OutDir | Out-Null
$sets = @(
    @{ Name = 'Claude'; Draw = { param($s) [TourneIcons]::Claude($s, $coral) } },
    @{ Name = 'GitHub Desktop'; Draw = { param($s) [TourneIcons]::GitHub($s, $slate) } },
    @{ Name = 'Settings'; Draw = { param($s) [TourneIcons]::Gear($s, $teal) } },
    @{ Name = 'Snipping Tool'; Draw = { param($s) [TourneIcons]::Scissors($s, $mauve) } }
)
$all = @{}
foreach ($set in $sets) {
    [System.Drawing.Bitmap[]]$images = foreach ($s in $sizes) { & $set.Draw $s }
    [TourneIcons]::WriteIco((Join-Path $OutDir "$($set.Name).ico"), $images)
    $all[$set.Name] = $images
    "wrote $(Join-Path $OutDir "$($set.Name).ico")"
}
if ($Preview) {
    # Each icon at every size, 6x, on the bar's ground, next to two of the set at 24 and 16.
    $z = 6
    $sheet = New-Object Drawing.Bitmap 1500, (4 * (48 * $z + 20) + 20)
    $g = [Drawing.Graphics]::FromImage($sheet)
    $g.Clear([Drawing.Color]::FromArgb(0x12, 0x1C, 0x21))
    $g.InterpolationMode = 'NearestNeighbor'; $g.PixelOffsetMode = 'Half'
    $y = 10
    foreach ($name in 'Claude', 'GitHub Desktop', 'Settings', 'Snipping Tool') {
        $x = 10
        foreach ($img in $all[$name]) { $g.DrawImage($img, $x, $y, $img.Width * $z, $img.Height * $z); $x += $img.Width * $z + 20 }
        $y += 48 * $z + 20
    }
    $src = '%USERPROFILE%\Downloads\Bouquet\Taskbar Icons\Bouquet Taskbar Icons'
    $y = 10
    foreach ($n in 'CHROME', 'STEAM', 'SETTINGS') {
        $x = 1100
        foreach ($s in 16, 24) {
            $ico = New-Object Drawing.Icon (Join-Path $src "$n.ico"), $s, $s
            $g.DrawImage($ico.ToBitmap(), $x, $y, $s * $z, $s * $z); $x += $s * $z + 20
        }
        $y += 48 * $z + 20
    }
    $g.Dispose(); $sheet.Save($Preview); $sheet.Dispose()
}