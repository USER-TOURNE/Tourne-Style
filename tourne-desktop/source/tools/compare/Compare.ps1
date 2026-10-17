<#
Compares our taskbar with StartAllBack's, for the theme StartAllBack is set to.
  1. photographs StartAllBack's real bar (its window is shown for a moment),
  2. runs our bar outside Windhawk (bartest.cpp) with the same settings, at the top of the screen (StartAllBack stays at the bottom),
  3. writes sab.png, ours.png, stack.png (sab / ours / difference at 2x) and prints the difference.
Output goes to .\out. StartAllBack's style and orb are followed; set those first.
#>
param([string]$Combine = 'always', [string]$Segments = 'three', [string]$Out = "$PSScriptRoot\out")
$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force $Out | Out-Null
$null = [Drawing.Image]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime\.dll|System\.Collections\.dll|System\.Threading|System\.Memory|netstandard|System\.Private\.CoreLib' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Grab.cs"))

. "$PSScriptRoot\RealReg.ps1"
$sab = [pscustomobject](Get-RealSab)  # the real values: my own registry view is virtualized
"SAB: style=$($sab.TaskbarStyle) orb=$($sab.OrbBitmap) oneSegment=$($sab.TaskbarOneSegment) larger=$($sab.TaskbarLargerIcons) spacing=$($sab.TaskbarSpacierIcons) group=$($sab.TaskbarGrouping) effect=$($sab.TaskbarTranslucentEffect)"
[Grab]::Tray("$Out\sab.png")

# Our bar, built from the current source (the "// @inline" files put in, as the suite build does).
$branch = '%USERPROFILE%\Downloads\tourne-suite-sab-clone'
$hostSrc = ([IO.File]::ReadAllText("$branch\taskbar-host.cpp") -replace "`r`n", "`n")
$hostSrc = [regex]::Replace($hostSrc, '(?m)^// @inline (\S+)$', { param($m) ([IO.File]::ReadAllText("$branch\$($m.Groups[1].Value)") -replace "`r`n", "`n").TrimEnd("`n") })
[IO.File]::WriteAllText("$Out\taskbar-inlined.cpp", $hostSrc, (New-Object Text.UTF8Encoding $false))
$exe = "$Out\bartest.exe"
& 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' --target=x86_64-w64-mingw32 -std=c++23 -municode -O1 -DUNICODE -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008 -I'C:\Program Files\Windhawk\Compiler\include' -I $Out "$PSScriptRoot\bartest.cpp" -o $exe -static -luxtheme -ldwmapi -lole32 -lshell32 -lgdi32 -lmsimg32 -luser32 -luuid -lwindowscodecs -lcomctl32 -lpropsys -lshlwapi -loleaut32 -ladvapi32 -lwinmm -lversion -lwtsapi32 -lpsapi -lnetapi32 -limm32 2>&1 | Where-Object { $_ -match 'error' } | Select-Object -First 15
$img = [Drawing.Image]::FromFile("$Out\sab.png"); $thick = $img.Height; $img.Dispose()
$vars = @{ TB_style = $(if ($sab.TaskbarStyle) { $sab.TaskbarStyle } else { 'none' }); TB_orb = $(if ($sab.OrbBitmap) { $sab.OrbBitmap } else { 'none' }); TB_location = 'top'; TB_dynamicTransparency = "$([int]([long]$sab.TaskbarTranslucentEffect -band 1))"; TB_dynamicSegments = "$([int]([long]$sab.TaskbarTranslucentEffect -band 2) / 2)"; TB_thickness = "$thick"; TB_combine = $Combine
           TB_segments = $Segments; TB_scheme = 'tourne'; TB_tray = 'hidden'; TB_clock = 'hidden' }
foreach ($k in $vars.Keys) { Set-Item "env:$k" $vars[$k] }
Push-Location $Out; [Environment]::CurrentDirectory = $Out
$p = Start-Process -FilePath $exe -ArgumentList "`"$Out\ours.bmp`"" -NoNewWindow -Wait -PassThru -RedirectStandardOutput "$Out\ours.txt"
Pop-Location; [Environment]::CurrentDirectory = (Get-Location).Path
foreach ($k in $vars.Keys) { Remove-Item "env:$k" -ErrorAction SilentlyContinue }
Get-Content "$Out\ours.txt" | Where-Object { $_ -match '^(bar|settings|GEOM)' }
$b = [Drawing.Image]::FromFile("$Out\ours.bmp"); $b.Save("$Out\ours.png", [Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
[Grab]::Diff("$Out\sab.png", "$Out\ours.png", "$Out\stack.png", "$Out\diff.png")

# Per element: how many of its pixels differ clearly (over 48 of 765), from our geometry.
$a = [Drawing.Bitmap]::FromFile("$Out\sab.png"); $b = [Drawing.Bitmap]::FromFile("$Out\ours.png")
$n = @{}
foreach ($line in Get-Content "$Out\ours.txt" | Where-Object { $_ -match '^GEOM (orb|button|island|tray|clock) ' }) {
    $f = $line -split ' '; $kind = $f[1]; $n[$kind] = 1 + [int]$n[$kind]
    $x0 = [math]::Max(0, [int]$f[2]); $y0 = [math]::Max(0, [int]$f[3]); $x1 = [math]::Min($a.Width, [int]$f[4]); $y1 = [math]::Min($a.Height, [int]$f[5])
    if ($x1 -le $x0 -or $y1 -le $y0) { continue }
    $bad = 0
    for ($y = $y0; $y -lt $y1; $y++) { for ($x = $x0; $x -lt $x1; $x++) {
        $p = $a.GetPixel($x, $y); $q = $b.GetPixel($x, $y)
        if ([math]::Abs($p.R - $q.R) + [math]::Abs($p.G - $q.G) + [math]::Abs($p.B - $q.B) -gt 48) { $bad++ } } }
    '{0,-7} #{1,-2} x {2,4}..{3,4}: {4,5:P1} differ' -f $kind, $n[$kind], $x0, $x1, ($bad / (($x1 - $x0) * ($y1 - $y0)))
}
$a.Dispose(); $b.Dispose()
