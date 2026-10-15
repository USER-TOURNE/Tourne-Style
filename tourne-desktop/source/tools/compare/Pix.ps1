param([string]$Png, [int]$X0, [int]$Y0, [int]$X1, [int]$Y1, [switch]$Delta, [string]$Other)
# Prints the pixels of a rectangle as RRGGBB, one row per line (with -Delta and -Other: the difference to another image).
Add-Type -AssemblyName System.Drawing
$Png = (Resolve-Path $Png).Path; if ($Other) { $Other = (Resolve-Path $Other).Path }
$a = [Drawing.Bitmap]::FromFile($Png)
$b = if ($Other) { [Drawing.Bitmap]::FromFile($Other) } else { $null }
for ($y = $Y0; $y -le $Y1; $y++) {
    $cells = for ($x = $X0; $x -le $X1; $x++) {
        $p = $a.GetPixel($x, $y)
        if ($Delta -and $b) { $q = $b.GetPixel($x, $y); '{0:X2}{1:X2}{2:X2}' -f [math]::Abs($p.R - $q.R), [math]::Abs($p.G - $q.G), [math]::Abs($p.B - $q.B) }
        else { '{0:X2}{1:X2}{2:X2}' -f $p.R, $p.G, $p.B }
    }
    '{0,2}: {1}' -f $y, ($cells -join ' ')
}
$a.Dispose(); if ($b) { $b.Dispose() }
