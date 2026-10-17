<#
Renders our bar (bartest.cpp, the suite's own code) for one style, idle and with a button hovered, next to the
StartAllBack strips from a Capture-Style folder, and writes compare.png: ours idle / SAB idle / ours hover / SAB hover (4x).
#>
param([Parameter(Mandatory)][string]$Style, [Parameter(Mandatory)][string]$Dir, [string]$Spot = 'open207', [int]$Hover = -1, [int]$BarWidth = 460, [switch]$Compile)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\RealReg.ps1"
Add-Type -AssemblyName System.Drawing
$Dir = (Resolve-Path $Dir).Path
$work = "$PSScriptRoot\out-ours"; New-Item -ItemType Directory -Force $work | Out-Null
$branch = '%USERPROFILE%\Downloads\tourne-suite-sab-clone'
$exe = "$work\bartest.exe"
if ($Compile -or -not (Test-Path $exe)) {
    $src = ([IO.File]::ReadAllText("$branch\taskbar-host.cpp") -replace "`r`n", "`n")
    $src = [regex]::Replace($src, '(?m)^// @inline (\S+)$', { param($m) ([IO.File]::ReadAllText("$branch\$($m.Groups[1].Value)") -replace "`r`n", "`n").TrimEnd("`n") })
    [IO.File]::WriteAllText("$work\taskbar-inlined.cpp", $src, (New-Object Text.UTF8Encoding $false))
    & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' --target=x86_64-w64-mingw32 -std=c++23 -municode -O1 -DUNICODE -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008 -I'C:\Program Files\Windhawk\Compiler\include' -I $work "$PSScriptRoot\bartest.cpp" -o $exe -static -luxtheme -ldwmapi -lole32 -lshell32 -lgdi32 -lmsimg32 -luser32 -luuid -lwindowscodecs -lcomctl32 -lpropsys -lshlwapi -loleaut32 -ladvapi32 -lwinmm -lversion -lwtsapi32 -lpsapi -lnetapi32 -limm32 -lurlmon -lwininet -liphlpapi -lwlanapi -lpowrprof 2>&1 | Where-Object { $_ -match 'error' } | Select-Object -First 15
}
$sab = Get-RealSab
function Run([string]$name, [int]$hoverIndex) {
    $vars = @{
        TB_style = $(if ($Style -eq 'Default') { 'none' } else { $Style }); TB_orb = $(if ($sab.OrbBitmap) { $sab.OrbBitmap } else { 'none' })
        TB_location = 'bottom'; TB_thickness = '38'; TB_combine = 'always'; TB_iconSize = 'small'; TB_iconMargins = 'small'
        TB_center = 'edge'; TB_startPosition = 'edge'; TB_plumpness = 'aura'; TB_segments = 'none'; TB_scheme = 'tourne'
        TB_tray = 'hidden'; TB_clock = 'hidden'; TB_idle = '1'
    }
    if ($hoverIndex -ge 0) { $vars.TB_hover = "$hoverIndex" }
    foreach ($k in $vars.Keys) { Set-Item "env:$k" $vars[$k] }
    Push-Location $work; [Environment]::CurrentDirectory = $work
    $null = Start-Process -FilePath $exe -ArgumentList "`"$work\$name.bmp`"" -NoNewWindow -Wait -PassThru -RedirectStandardOutput "$work\$name.txt"
    Pop-Location; [Environment]::CurrentDirectory = (Get-Location).Path
    foreach ($k in $vars.Keys) { Remove-Item "env:$k" -ErrorAction SilentlyContinue }
    $b = [Drawing.Image]::FromFile("$work\$name.bmp"); $b.Save("$work\$name.png", [Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
}
Run 'idle' -1
if ($Hover -lt 0) { $Hover = [math]::Max(0, [int][math]::Round(([int]($Spot -replace '\D', '') - 59 - 16) / 33)) }
Run 'hover' $Hover
$oi = [Drawing.Bitmap]::FromFile("$work\idle.png"); $oh = [Drawing.Bitmap]::FromFile("$work\hover.png")
$si = [Drawing.Bitmap]::FromFile("$Dir\1-idle.png"); $sh = [Drawing.Bitmap]::FromFile("$Dir\2-hover-sab-$Spot.png")
$h = 38; $s = 4
$o = New-Object Drawing.Bitmap ($BarWidth * $s), (($h * $s + 6) * 4)
$g = [Drawing.Graphics]::FromImage($o); $g.Clear([Drawing.Color]::Magenta); $g.InterpolationMode = 'NearestNeighbor'; $g.PixelOffsetMode = 'Half'
$i = 0
foreach ($r in @(@($oi, 0), @($si, 1042), @($oh, 0), @($sh, 1042))) {
    $g.DrawImage($r[0], [Drawing.Rectangle]::new(0, $i * ($h * $s + 6), $BarWidth * $s, $h * $s), 0, $r[1], $BarWidth, $h, 'Pixel'); $i++
}
$o.Save("$Dir\compare-$Spot.png", [Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $o.Dispose(); $oi.Dispose(); $oh.Dispose(); $si.Dispose(); $sh.Dispose()
"wrote $Dir\compare-$Spot.png"
