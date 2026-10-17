$d = Split-Path $MyInvocation.MyCommand.Path
Add-Type -Path "$d\Brand.cs" -ReferencedAssemblies System.Drawing
$out = '%USERPROFILE%\Downloads\WINDux.95 basebrd images'
New-Item -ItemType Directory -Force $out | Out-Null
$font = "$env:LOCALAPPDATA\Microsoft\Windows\Fonts\scientifica.ttf"
$report = ''
$master = [Brand]::Master($font, 11, 114, 18, [ref]$report)
$report
$master.Save("$d\master.png", [Drawing.Imaging.ImageFormat]::Png)
[Brand]::SaveScaled($master, 6, 687, 108, "$out\2123.png")
[Brand]::SaveScaled($master, 5, 573, 90, "$out\1123.png")
[Brand]::SaveScaled($master, 4, 458, 72, "$out\123.png")
[Brand]::SaveScaled($master, 8, 912, 144, "$d\preview8x.png")
Get-ChildItem $out | Select-Object Name, Length
