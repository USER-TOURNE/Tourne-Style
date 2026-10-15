# Six more 16 x 16 directions for Tourne Studio's icon, from the desktop they live on: the torii and its pixel
# rain, "every machine has a spirit", the ./ROOT prompt, a shrine lantern, the windows' dots with the drive bars,
# and a brush laying down the menus' gradient. Drawn from character maps; shown large and at real sizes.
param([string]$Out)
Add-Type -AssemblyName System.Drawing
function Rgb([int]$hex) { [Drawing.Color]::FromArgb(255, ($hex -shr 16) -band 255, ($hex -shr 8) -band 255, $hex -band 255) }
$P = @{
    'k' = Rgb 0x121c21; 'b' = Rgb 0x152026; 'e' = Rgb 0x1e3632; 't' = Rgb 0x305245; 'g' = Rgb 0x66EA97; 'L' = Rgb 0x15FFA2
    'r' = Rgb 0xE57474; 'd' = Rgb 0xA95D5D; 'w' = Rgb 0xD9C7A0; 'p' = Rgb 0x4F8F6A
}
function Mix($a, $b, [double]$t) { [Drawing.Color]::FromArgb(255, [int]($a.R + ($b.R - $a.R) * $t), [int]($a.G + ($b.G - $a.G) * $t), [int]($a.B + ($b.B - $a.B) * $t)) }
# A map onto 16 x 16: '.' is the ground (a tile: dark, its edge in the edge colour, corners cut) or nothing.
function FromMap([string[]]$rows, [bool]$tile) {
    $bm = New-Object Drawing.Bitmap 16, 16
    for ($y = 0; $y -lt 16; $y++) { for ($x = 0; $x -lt 16; $x++) {
        $ch = $rows[$y][$x]
        if ($ch -ne '.') { $bm.SetPixel($x, $y, $P[[string]$ch]); continue }
        if (-not $tile -or (($x -in 0, 15) -and ($y -in 0, 15))) { $bm.SetPixel($x, $y, [Drawing.Color]::Transparent); continue }
        $bm.SetPixel($x, $y, $(if ($x -in 0, 15 -or $y -in 0, 15) { $P['e'] } else { $P['k'] })) } }
    $bm
}

$torii = FromMap @(
    '................',
    '.rr..........rr.',
    '..rrrrrrrrrrrr..',
    '...dddddddddd...',
    '....rr.dd.rr....',
    '..rrrrrrrrrrrr..',
    '....rr....rr....',
    '....rr.L..rr....',
    '....rr.g..rr....',
    '....rr....rr....',
    '....rr..g.rr....',
    '....rr..g.rr....',
    '....rr....rr....',
    '....rrg...rr....',
    '...dddd..dddd...',
    '................') $true

$spirit = FromMap @(
    '................',
    '................',
    '.tttttttttttttt.',
    '.tkkkkkkkkkkkkt.',
    '.tkkLkkkkkkLkkt.',
    '.tkLkLkkkkLkLkt.',
    '.tkkkkkkkkkkkkt.',
    '.tkrkkkkkkkkrkt.',
    '.tkkkkkgkgkkkkt.',
    '.tkkkkkkgkkkkkt.',
    '.tkkkkkkkkkkkkt.',
    '.tttttttttttttt.',
    '......tttt......',
    '....eeeeeeee....',
    '................',
    '................') $false

$prompt = FromMap @(
    '................',
    '.kkkkkkkkkkkkkk.',
    '.kkkkkkkkkkkkkk.',
    '.kkkkkkggkkkkkk.',
    '.kkkkkkggkkkkkk.',
    '.kkkkkggkkkrrrk.',
    '.kkkkkggkkkrrrk.',
    '.kkkkggkkkkrrrk.',
    '.kkkkggkkkkrrrk.',
    '.kkkggkkkkkrrrk.',
    '.kggkggkkkkrrrk.',
    '.kggkkkkkkkkkkk.',
    '.kkkkkkkkkkkkkk.',
    '.kkkkkkkkkkkkkk.',
    '.kkkkkkkkkkkkkk.',
    '................') $true

$lantern = FromMap @(
    '................',
    '.......tt.......',
    '.....tttttt.....',
    '...tttttttttt...',
    '..eeeeeeeeeeee..',
    '....tkkkkkkt....',
    '....tkLLLLkt....',
    '....tkLggLkt....',
    '....tkLLLLkt....',
    '....tttttttt....',
    '.....eeeeee.....',
    '......tttt......',
    '......tttt......',
    '.....eeeeee.....',
    '...tttttttttt...',
    '................') $false

# The window: its title bar with the three dots, and three drive bars filled teal to red.
$window = FromMap @(
    '................',
    '.eeeeeeeeeeeeee.',
    '.eeeeeeeetewere.',
    '.eeeeeeeeeeeeee.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.ekkkkkkkkkkkke.',
    '.eeeeeeeeeeeeee.',
    '................',
    '................') $false
foreach ($bar in @(@(6, 9), @(8, 5), @(10, 11))) {
    for ($x = 3; $x -le 12; $x++) {
        $c = if ($x - 3 -lt $bar[1]) { Mix $P['t'] $P['r'] (($x - 3) / 9.0) } else { $P['e'] }
        $window.SetPixel($x, $bar[0], $c) } }

$brush = FromMap @(
    '................',
    '.............tt.',
    '............ttt.',
    '...........ttt..',
    '..........ttt...',
    '.........ttt....',
    '........ww......',
    '.......www......',
    '......rrr.......',
    '.....rrr........',
    '.....rr.........',
    '................',
    '................',
    '................',
    '................',
    '................') $false
for ($x = 1; $x -le 11; $x++) { $t = ($x - 1) / 10.0; $c = if ($t -lt 0.5) { Mix $P['t'] $P['g'] ($t * 2) } else { Mix $P['g'] $P['r'] (($t - 0.5) * 2) }
    $brush.SetPixel($x, 12, $c); if ($x -ge 3 -and $x -le 10) { $brush.SetPixel($x, 13, $c) } }

$icons = @(@('D  Torii', $torii), @('E  Machine spirit', $spirit), @('F  ./ prompt', $prompt),
           @('G  Lantern', $lantern), @('H  Window + drive bars', $window), @('I  Brush', $brush))
$cols = 3; $rows = 2; $cw = 300; $ch = 420
$sheet = New-Object Drawing.Bitmap ($cols * $cw + 40), ($rows * $ch + 10)
$g = [Drawing.Graphics]::FromImage($sheet); $g.Clear([Drawing.Color]::FromArgb(255, 24, 26, 28)); $g.InterpolationMode = 'NearestNeighbor'; $g.PixelOffsetMode = 'Half'
$font = New-Object Drawing.Font 'Consolas', 13; $text = New-Object Drawing.SolidBrush ($P['g'])
$dark = New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(255, 18, 28, 33)); $light = New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(255, 235, 235, 235))
for ($i = 0; $i -lt $icons.Count; $i++) {
    $x0 = 20 + ($i % $cols) * $cw; $y0 = [Math]::Floor($i / $cols) * $ch
    $g.DrawString($icons[$i][0], $font, $text, $x0, $y0 + 6)
    $g.FillRectangle($dark, $x0, $y0 + 34, 256, 256)
    $g.DrawImage($icons[$i][1], (New-Object Drawing.Rectangle $x0, ($y0 + 34), 256, 256))
    $g.FillRectangle($dark, $x0, $y0 + 304, 256, 52); $g.FillRectangle($light, $x0, $y0 + 360, 256, 52)
    $x = $x0 + 8
    foreach ($s in 16, 32, 48) { foreach ($row in 304, 360) { $g.DrawImage($icons[$i][1], (New-Object Drawing.Rectangle $x, ($y0 + $row + (52 - [Math]::Min($s, 48)) / 2), $s, $s)) }; $x += $s + 16 }
}
$sheet.Save($Out)
$dir = Split-Path $Out
foreach ($pair in @(@('torii', $torii), @('spirit', $spirit), @('prompt', $prompt), @('lantern', $lantern), @('window', $window), @('brush', $brush))) {
    $pair[1].Save((Join-Path $dir "icon-$($pair[0])-16.png")) }
