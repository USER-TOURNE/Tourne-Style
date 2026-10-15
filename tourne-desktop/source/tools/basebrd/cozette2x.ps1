$d = Split-Path $MyInvocation.MyCommand.Path
Add-Type -Path "$d\Brand.cs" -ReferencedAssemblies System.Drawing
$font = "$env:LOCALAPPDATA\Microsoft\Windows\Fonts\CozetteVector.ttf"
$out = '%USERPROFILE%\Downloads\WINDux.95 basebrd images\pixel (Cozette 2x)'
New-Item -ItemType Directory -Force $out | Out-Null
foreach ($s in @(@('2123', 687, 108), @('1123', 573, 90), @('123', 458, 72))) {
    $r = ''
    # A master at half the size, scaled 2x: 2-pixel blocks at every size.
    $m = [Brand]::Master($font, 13, [math]::Floor($s[1] / 2), [math]::Floor($s[2] / 2), [ref]$r)
    [Brand]::SaveScaled($m, 2, $s[1], $s[2], "$out\$($s[0]).png")
    "$($s[0]).png  $($s[1])x$($s[2]): $r"
}
# Stacked preview of all three.
$files = '2123', '1123', '123' | ForEach-Object { "$out\$_.png" }
$sheet = New-Object Drawing.Bitmap 687, (108 + 90 + 72 + 20)
$g = [Drawing.Graphics]::FromImage($sheet); $g.Clear([Drawing.Color]::Black)
$y = 0
foreach ($f in $files) { $img = [Drawing.Image]::FromFile($f); $g.DrawImage($img, 0, $y, $img.Width, $img.Height); $y += $img.Height + 10; $img.Dispose() }
$sheet.Save("$d\cozette2x_all.png", [Drawing.Imaging.ImageFormat]::Png)
