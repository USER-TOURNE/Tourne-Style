# Three 16 x 16 pixel-art directions for Tourne Studio's icon, in Tourne's palette, shown large and at real sizes.
param([string]$Out)
Add-Type -AssemblyName System.Drawing
$C = @{
    ground = [Drawing.Color]::FromArgb(255, 0x12, 0x1c, 0x21); band = [Drawing.Color]::FromArgb(255, 0x15, 0x20, 0x26)
    edge = [Drawing.Color]::FromArgb(255, 0x1e, 0x36, 0x32); teal = [Drawing.Color]::FromArgb(255, 0x30, 0x52, 0x45)
    green = [Drawing.Color]::FromArgb(255, 0x66, 0xEA, 0x97); bright = [Drawing.Color]::FromArgb(255, 0x15, 0xFF, 0xA2)
    red = [Drawing.Color]::FromArgb(255, 0xE5, 0x74, 0x74); dred = [Drawing.Color]::FromArgb(255, 0xA9, 0x5D, 0x5D)
}
function Mix($a, $b, [double]$t) { [Drawing.Color]::FromArgb(255, [int]($a.R + ($b.R - $a.R) * $t), [int]($a.G + ($b.G - $a.G) * $t), [int]($a.B + ($b.B - $a.B) * $t)) }
function NewIcon { $b = New-Object Drawing.Bitmap 16, 16; for ($y = 0; $y -lt 16; $y++) { for ($x = 0; $x -lt 16; $x++) { $b.SetPixel($x, $y, [Drawing.Color]::Transparent) } }; $b }
function Tile($b) {  # a square tile, its corners cut by a pixel, framed in the menus' gradient
    for ($y = 0; $y -lt 16; $y++) { for ($x = 0; $x -lt 16; $x++) {
        $corner = ($x -in 0, 15) -and ($y -in 0, 15); if ($corner) { continue }
        $frame = $x -eq 0 -or $y -eq 0 -or $x -eq 15 -or $y -eq 15
        $b.SetPixel($x, $y, $(if ($frame) { Mix $C.teal $C.red (($x + $y) / 30.0) } else { $C.ground })) } }
}

# A: Mixer - three faders, their knobs at different heights.
$a = NewIcon; Tile $a
foreach ($col in 4, 8, 12) { for ($y = 3; $y -le 12; $y++) { $a.SetPixel($col - 1 + 0, $y, $C.edge) } }
foreach ($k in @(@(3, 9, 'red'), @(7, 4, 'bright'), @(11, 7, 'green'))) {
    for ($dx = 0; $dx -lt 3; $dx++) { for ($dy = 0; $dy -lt 2; $dy++) { $a.SetPixel($k[0] + $dx - 1 + 0, $k[1] + $dy, $C[$k[2]]) } } }

# B: Dial - a knob that turns ("Tourne"), its pointer in the red, ticks round it.
$b2 = NewIcon; Tile $b2
for ($y = 0; $y -lt 16; $y++) { for ($x = 0; $x -lt 16; $x++) {
    $d = [Math]::Sqrt(($x - 7.5) * ($x - 7.5) + ($y - 7.5) * ($y - 7.5))
    if ($d -le 4.6) { $b2.SetPixel($x, $y, $C.band) }
    if ($d -gt 4.0 -and $d -le 4.9) { $b2.SetPixel($x, $y, $C.teal) } } }
foreach ($p in @(@(9, 6), @(10, 5))) { $b2.SetPixel($p[0], $p[1], $C.red) }
$b2.SetPixel(8, 7, $C.dred); $b2.SetPixel(7, 8, $C.green)
foreach ($t in @(@(2, 12), @(2, 7), @(3, 3), @(7, 2), @(12, 3), @(13, 7), @(13, 12))) { $b2.SetPixel($t[0], $t[1], $(if ($t[0] -ge 12 -and $t[1] -le 7) { $C.red } else { $C.green })) }

# C: Monogram - a T inside the menus' double frame.
$c3 = NewIcon; Tile $c3
for ($y = 2; $y -le 13; $y++) { for ($x = 2; $x -le 13; $x++) {
    if ($x -eq 2 -or $y -eq 2 -or $x -eq 13 -or $y -eq 13) { $c3.SetPixel($x, $y, (Mix $C.teal $C.red (($x + $y) / 30.0))) } } }
for ($x = 4; $x -le 11; $x++) { $c3.SetPixel($x, 5, $C.green); $c3.SetPixel($x, 6, $C.green) }
for ($y = 7; $y -le 11; $y++) { $c3.SetPixel(7, $y, $C.green); $c3.SetPixel(8, $y, $C.green) }
$c3.SetPixel(11, 11, $C.red)

# The sheet: each large (x16) with its name, and at 16, 32 and 48 on a dark and a light ground.
$icons = @(@('A  Mixer', $a), @('B  Dial', $b2), @('C  Monogram', $c3))
$sheet = New-Object Drawing.Bitmap (3 * 300 + 40), 420
$g = [Drawing.Graphics]::FromImage($sheet); $g.Clear([Drawing.Color]::FromArgb(255, 24, 26, 28)); $g.InterpolationMode = 'NearestNeighbor'; $g.PixelOffsetMode = 'Half'
$font = New-Object Drawing.Font 'Consolas', 13; $brush = New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(255, 0x66, 0xEA, 0x97))
for ($i = 0; $i -lt 3; $i++) {
    $x0 = 20 + $i * 300
    $g.DrawString($icons[$i][0], $font, $brush, $x0, 6)
    $g.DrawImage($icons[$i][1], (New-Object Drawing.Rectangle $x0, 34, 256, 256))
    $g.FillRectangle((New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(255, 18, 28, 33))), $x0, 304, 256, 52)
    $g.FillRectangle((New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(255, 235, 235, 235))), $x0, 360, 256, 52)
    $x = $x0 + 8
    foreach ($s in 16, 32, 48) { foreach ($row in 304, 360) { $g.DrawImage($icons[$i][1], (New-Object Drawing.Rectangle $x, ($row + (52 - [Math]::Min($s, 48)) / 2), $s, $s)) }; $x += $s + 16 }
}
$sheet.Save($Out)
foreach ($i in 0..2) { $icons[$i][1].Save(($Out -replace '\.png$', "-$('abc'[$i]).png")) }
