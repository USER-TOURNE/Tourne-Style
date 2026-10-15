param([Parameter(Mandatory)][string]$Dir, [int]$BarWidth = 460)
# Makes bars.png (ours idle, SAB idle, ours hover, SAB hover; 4x) and start-pair.png (SAB left, ours right; 2x) from a Capture-Style folder.
Add-Type -AssemblyName System.Drawing
$Dir = (Resolve-Path $Dir).Path
function Load($n) { [Drawing.Bitmap]::FromFile("$Dir\$n") }
$idle = Load '1-idle.png'; $hs = Load '2-hover-sab.png'; $ho = Load '3-hover-ours.png'
$h = 38; $s = 4
$o = New-Object Drawing.Bitmap ($BarWidth * $s), (($h * $s + 6) * 4)
$g = [Drawing.Graphics]::FromImage($o); $g.Clear([Drawing.Color]::Magenta); $g.InterpolationMode = 'NearestNeighbor'; $g.PixelOffsetMode = 'Half'
$i = 0
foreach ($r in @(@($idle, 0), @($idle, 1042), @($ho, 0), @($hs, 1042))) {
    $g.DrawImage($r[0], [Drawing.Rectangle]::new(0, $i * ($h * $s + 6), $BarWidth * $s, $h * $s), 0, $r[1], $BarWidth, $h, 'Pixel'); $i++
}
$o.Save("$Dir\bars.png", [Drawing.Imaging.ImageFormat]::Png)
foreach ($pair in @(@('4-start-sab.png', '6-start-ours.png', 'start-pair.png'), @('5-start-sab-hover.png', '7-start-ours-hover.png', 'start-hover-pair.png'))) {
    $a = Load $pair[0]; $b = Load $pair[1]
    $w = 400; $hh = 425; $sc = 2
    $p = New-Object Drawing.Bitmap ($w * $sc * 2 + 8), ($hh * $sc)
    $gp = [Drawing.Graphics]::FromImage($p); $gp.Clear([Drawing.Color]::Magenta); $gp.InterpolationMode = 'NearestNeighbor'; $gp.PixelOffsetMode = 'Half'
    $gp.DrawImage($a, [Drawing.Rectangle]::new(0, 0, $w * $sc, $hh * $sc), 15, 618, $w, $hh, 'Pixel')
    $gp.DrawImage($b, [Drawing.Rectangle]::new($w * $sc + 8, 0, $w * $sc, $hh * $sc), 15, 42, $w, $hh, 'Pixel')
    $p.Save("$Dir\$($pair[2])", [Drawing.Imaging.ImageFormat]::Png)
}
"ok"
