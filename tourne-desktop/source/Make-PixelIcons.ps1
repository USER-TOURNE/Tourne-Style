<#
Tourne pixel icons as .ico files, drawn exactly as System Icons draws its tray art (RenderArt): a 12x12
grid on a 13x13 canvas with a one-pixel shade down and right, scaled by whole numbers and centred, in
the ink tone with coral accents. 16, 20, 24, 32, 40, 48, 64 and 256 px, 32-bit.
  .\Make-PixelIcons.ps1 -OutDir <folder> [-Preview sheet.png] [-Check existing.ico]
-Check redraws Discord's art and compares it with that file pixel for pixel. Runs in Windows PowerShell
(powershell.exe), whose System.Drawing needs no extra references.
#>
param([string]$OutDir = (Join-Path $PSScriptRoot 'icons'), [string]$Preview, [string]$Check)
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

public static class PixelIcons {
    // Colours by grid letter: '#' the ink tone, 'o' and 'b' coral; each with its shade.
    static readonly int Ink = 0x66EA97, InkShade = 0x3C865C, Coral = 0xFF6F77, CoralShade = 0xA95D5D;

    static int Lit(char c) { return c == '#' ? Ink : Coral; }
    static int Shade(char c) { return c == '#' ? InkShade : CoralShade; }

    public static Bitmap Render(string[] grid, int size) {
        var src = new int[13, 13];
        var lit = new bool[13, 13];
        for (int y = 0; y < 12; y++)
            for (int x = 0; x < 12; x++) {
                char c = grid[y][x];
                if (c == '.') continue;
                bool below = y + 1 < 12 && x + 1 < 12 && grid[y + 1][x + 1] != '.';
                if (!below) { src[y + 1, x + 1] = Shade(c); lit[y + 1, x + 1] = true; }
            }
        for (int y = 0; y < 12; y++)
            for (int x = 0; x < 12; x++)
                if (grid[y][x] != '.') { src[y, x] = Lit(grid[y][x]); lit[y, x] = true; }
        var bmp = new Bitmap(size, size, PixelFormat.Format32bppArgb);
        int scale = Math.Max(1, size / 13), offset = (size - 13 * scale) / 2;
        for (int y = 0; y < 13 * scale; y++)
            for (int x = 0; x < 13 * scale; x++) {
                int X = x + offset, Y = y + offset;
                if (X < 0 || X >= size || Y < 0 || Y >= size || !lit[y / scale, x / scale]) continue;
                int c = src[y / scale, x / scale];
                bmp.SetPixel(X, Y, Color.FromArgb(255, (c >> 16) & 255, (c >> 8) & 255, c & 255));
            }
        return bmp;
    }

    // A .ico with a 32-bit image (and its AND mask) at each size; 256 is stored as PNG, as Windows does.
    public static void WriteIco(string path, Bitmap[] images) {
        var blobs = new byte[images.Length][];
        for (int i = 0; i < images.Length; i++) {
            var bmp = images[i];
            int s = bmp.Width;
            if (s >= 256) {
                using (var png = new MemoryStream()) { bmp.Save(png, ImageFormat.Png); blobs[i] = png.ToArray(); }
                continue;
            }
            int maskStride = ((s + 31) / 32) * 4;
            var b = new MemoryStream();
            var w = new BinaryWriter(b);
            w.Write(40); w.Write(s); w.Write(s * 2); w.Write((ushort)1); w.Write((ushort)32);
            w.Write(0); w.Write(s * s * 4 + maskStride * s); w.Write(0); w.Write(0); w.Write(0); w.Write(0);
            for (int y = s - 1; y >= 0; y--)
                for (int x = 0; x < s; x++) { var c = bmp.GetPixel(x, y); w.Write(c.B); w.Write(c.G); w.Write(c.R); w.Write(c.A); }
            for (int y = s - 1; y >= 0; y--) {
                var row = new byte[maskStride];
                for (int x = 0; x < s; x++) if (bmp.GetPixel(x, y).A == 0) row[x / 8] |= (byte)(0x80 >> (x % 8));
                w.Write(row);
            }
            blobs[i] = b.ToArray();
        }
        using (var ms = new MemoryStream()) {
            var w = new BinaryWriter(ms);
            w.Write((ushort)0); w.Write((ushort)1); w.Write((ushort)images.Length);
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

$sizes = 16, 20, 24, 32, 40, 48, 64, 256
# Badge(): the bell's dot at 8..10, its neighbours cleared, as System Icons adds it.
function Add-Dot([string[]]$g) {
    $rows = @(foreach ($r in $g) { ,$r.ToCharArray() })
    foreach ($y in 7..11) { foreach ($x in 7..11) { if ($y -lt 12 -and $x -lt 12 -and $rows[$y][$x] -ne 'b') { $rows[$y][$x] = '.' } } }
    foreach ($y in 8..10) { foreach ($x in 8..10) { $rows[$y][$x] = 'b' } }
    $rows | ForEach-Object { -join $_ }
}
$chat = @('............', '.##########.', '.#........#.', '.#.######.#.', '.#........#.', '.#.####...#.',
          '.#........#.', '.#..#######.', '.#.#........', '.##.........', '............', '............')
$designs = [ordered]@{
    'NVIDIA'        = @('............', '............', '....####....', '..##....##..', '.#..####..#.', '#..#.oo.#..#',
                        '#..#.oo.#..#', '.#..####..#.', '..##....##..', '....####....', '............', '............')
    'Process Lasso' = @('....#####...', '..##.....##.', '.#.........#', '#..........#', '#..........#', '.#........#.',
                        '..#oo...##..', '...oo###....', '...#........', '....#.......', '...#........', '..#.........')
}
if ($Check) {
    $mine = Add-Dot $chat
    $bad = 0
    foreach ($s in 16, 32, 48) {
        $drawn = [PixelIcons]::Render($mine, $s)
        $ico = New-Object Drawing.Icon $Check, $s, $s
        $theirs = $ico.ToBitmap()
        for ($y = 0; $y -lt $s; $y++) { for ($x = 0; $x -lt $s; $x++) { if ($drawn.GetPixel($x, $y).ToArgb() -ne $theirs.GetPixel($x, $y).ToArgb()) { $bad++ } } }
    }
    "check against $(Split-Path $Check -Leaf): $bad pixels differ at 16, 32 and 48"
}
New-Item -ItemType Directory -Force $OutDir | Out-Null
$all = [ordered]@{}
foreach ($name in $designs.Keys) {
    [Drawing.Bitmap[]]$images = foreach ($s in $sizes) { [PixelIcons]::Render($designs[$name], $s) }
    [PixelIcons]::WriteIco((Join-Path $OutDir "$name.ico"), $images)
    $all[$name] = $images
    "wrote $(Join-Path $OutDir "$name.ico")"
}
if ($Preview) {
    $z = 4
    $sheet = New-Object Drawing.Bitmap 1500, ($all.Count * (64 * $z + 30) + 20)
    $g = [Drawing.Graphics]::FromImage($sheet)
    $g.Clear([Drawing.Color]::FromArgb(0x12, 0x1C, 0x21))
    $g.InterpolationMode = 'NearestNeighbor'; $g.PixelOffsetMode = 'Half'
    $y = 10
    foreach ($name in $all.Keys) {
        $x = 10
        foreach ($img in $all[$name] | Where-Object { $_.Width -le 64 }) { $g.DrawImage($img, $x, $y, $img.Width * $z, $img.Height * $z); $x += $img.Width * $z + 16 }
        $y += 64 * $z + 30
    }
    $g.Dispose(); $sheet.Save($Preview); $sheet.Dispose()
}
