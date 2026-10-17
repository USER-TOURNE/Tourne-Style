<#
Compares our taskbar with StartAllBack's, style by style and state by state (idle and hovered), with StartAllBack
restarted (Explorer killed, then fully reloaded) before EVERY photograph of it.

StartAllBack's settings stay as they are in your registry, except TaskbarStyle, which is set per style and put back.
Our bar (bartest.cpp, the same code as the suite's bar) is given the matching settings from StartAllBack's real values.
Output: out-matrix\<style>\{sab,ours}-{idle,hover1,hover2}.png, stack-*.png (sab / ours / difference, 2x), report.txt.
#>
param([string[]]$Styles = @('Default', 'Bouquet SAB.msstyles', 'Plain8.msstyles', 'Windows 7.msstyles'),
      [int[]]$HoverButtons = @(1, 5), [string]$Out = "$PSScriptRoot\out-matrix", [switch]$SkipCompile, [switch]$ReuseSab, [string]$OurEdge = 'bottom', [switch]$Opaque)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\RealReg.ps1"
New-Item -ItemType Directory -Force $Out | Out-Null
$null = [Drawing.Image]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime\.dll|System\.Collections\.dll|System\.Threading|System\.Memory|netstandard|System\.Private\.CoreLib' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Grab.cs"))

# Kill Explorer, wait until it is back and the taskbar is fully populated and steady, then let it settle.
function Restart-ExplorerFull {
    # Kill every explorer.exe and make sure none of the old ones is left before waiting for the new one.
    $old = @(Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object Id)
    for ($try = 0; $try -lt 10; $try++) {
        Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 700
        if (-not @(Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $old -contains $_.Id })) { break }
    }
    Start-Sleep -Seconds 3
    $deadline = (Get-Date).AddSeconds(60)
    $steady = 0; $last = ''
    while ((Get-Date) -lt $deadline -and $steady -lt 5) {
        Start-Sleep -Seconds 1
        $p = [Grab]::Probe()
        if ($p -match 'task=1' -and $p -match 'vis=1' -and $p -notmatch 'tray=0' -and $p -eq $last) { $steady++ } else { $steady = 0 }
        $last = $p
    }
    if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe; Start-Sleep -Seconds 10 }
    Start-Sleep -Seconds 6
    "explorer ready ($last)"
}

$branch = '%USERPROFILE%\Downloads\tourne-suite-sab-clone'
$exe = "$Out\bartest.exe"
if (-not $SkipCompile) {
    $hostSrc = ([IO.File]::ReadAllText("$branch\taskbar-host.cpp") -replace "`r`n", "`n")
    $hostSrc = [regex]::Replace($hostSrc, '(?m)^// @inline (\S+)$', { param($m) ([IO.File]::ReadAllText("$branch\$($m.Groups[1].Value)") -replace "`r`n", "`n").TrimEnd("`n") })
    [IO.File]::WriteAllText("$Out\taskbar-inlined.cpp", $hostSrc, (New-Object Text.UTF8Encoding $false))
    & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' --target=x86_64-w64-mingw32 -std=c++23 -municode -O1 -DUNICODE -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008 -I'C:\Program Files\Windhawk\Compiler\include' -I $Out "$PSScriptRoot\bartest.cpp" -o $exe -static -luxtheme -ldwmapi -lole32 -lshell32 -lgdi32 -lmsimg32 -luser32 -luuid -lwindowscodecs -lcomctl32 -lpropsys -lshlwapi -loleaut32 -ladvapi32 -lwinmm -lversion -lwtsapi32 -lpsapi -lnetapi32 -limm32 2>&1 | Where-Object { $_ -match 'error' } | Select-Object -First 15
}

# Our bar with the settings StartAllBack really has, for one style; extra = hover index or -1 for idle.
function Run-Ours($base, [string]$style, [int]$thickness, [string]$dest, [int]$hover) {
    $effect = [long]$base.TaskbarTranslucentEffect
    $vars = @{
        TB_style = $(if ($style -eq 'Default') { 'none' } else { $style }); TB_orb = $(if ($base.OrbBitmap) { $base.OrbBitmap } else { 'none' })
        TB_location = $OurEdge; TB_thickness = "$thickness"; TB_combine = 'always'; TB_iconSize = 'small'; TB_iconMargins = 'small'
        TB_center = $(if ([long]$base.TaskbarCenterIcons -eq 1) { 'centered' } else { 'edge' }); TB_startPosition = 'edge'
        TB_plumpness = $(switch ([long]$base.FatTaskbar) { 1 { 'buffer' } 2 { 'aura' } default { 'none' } })
        TB_dynamicTransparency = "$([int]($effect -band 1))"; TB_dynamicSegments = "$([int](($effect -band 2) / 2))"
        TB_segments = 'none'; TB_scheme = 'tourne'; TB_tray = 'hidden'; TB_clock = 'hidden'; TB_idle = '1'
    }
    if ($Opaque) {
        $vars.TB_coloring = 'custom'; $vars.TB_coloringColor = '121C21'; $vars.TB_coloringAlpha = '255'; $vars.TB_coloringBlur = 'none'
    }
    if ($hover -ge 0) { $vars.TB_hover = "$hover" }
    foreach ($k in $vars.Keys) { Set-Item "env:$k" $vars[$k] }
    Push-Location $Out; [Environment]::CurrentDirectory = $Out
    $null = Start-Process -FilePath $exe -ArgumentList "`"$dest.bmp`"" -NoNewWindow -Wait -PassThru -RedirectStandardOutput "$dest.txt"
    Pop-Location; [Environment]::CurrentDirectory = (Get-Location).Path
    foreach ($k in $vars.Keys) { Remove-Item "env:$k" -ErrorAction SilentlyContinue }
    $b = [Drawing.Image]::FromFile("$dest.bmp"); $b.Save("$dest.png", [Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
}

function Report-Elements([string]$sabPng, [string]$oursPng, [string]$geomTxt) {
    $a = [Drawing.Bitmap]::FromFile($sabPng); $b = [Drawing.Bitmap]::FromFile($oursPng)
    $n = @{}
    foreach ($line in Get-Content $geomTxt | Where-Object { $_ -match '^GEOM (orb|button|island) ' }) {
        $f = $line -split ' '; $kind = $f[1]; $n[$kind] = 1 + [int]$n[$kind]
        $x0 = [math]::Max(0, [int]$f[2]); $y0 = [math]::Max(0, [int]$f[3]); $x1 = [math]::Min($a.Width, [int]$f[4]); $y1 = [math]::Min($a.Height, [int]$f[5])
        if ($x1 -le $x0 -or $y1 -le $y0) { continue }
        $bad = 0
        for ($y = $y0; $y -lt $y1; $y++) { for ($x = $x0; $x -lt $x1; $x++) {
            $p = $a.GetPixel($x, $y); $q = $b.GetPixel($x, $y)
            if ([math]::Abs($p.R - $q.R) + [math]::Abs($p.G - $q.G) + [math]::Abs($p.B - $q.B) -gt 48) { $bad++ } } }
        '  {0,-7} #{1,-2} x {2,4}..{3,4}: {4,5:P1} differ' -f $kind, $n[$kind], $x0, $x1, ($bad / (($x1 - $x0) * ($y1 - $y0)))
    }
    $a.Dispose(); $b.Dispose()
}

$backup = Join-Path $Out 'startisback-backup.reg'
Export-RealSab $backup
$base = [pscustomobject](Get-RealSab)
$report = New-Object System.Collections.Generic.List[string]
function Say($s) { $s; $report.Add($s) }
Say "StartAllBack now: style='$($base.TaskbarStyle)' orb='$($base.OrbBitmap)' center=$($base.TaskbarCenterIcons) plumpness(FatTaskbar)=$($base.FatTaskbar) effect=$($base.TaskbarTranslucentEffect) grouping=$($base.TaskbarGrouping)"
$needExplorer = -not $ReuseSab
if ($Opaque -and -not $ReuseSab) {
    # An opaque, unblurred bar colour, so the photographs show the bar as drawn (no acrylic to misjudge).
    Set-RealSabDword 'TaskbarColoring' 1; Set-RealSabDword 'TaskbarColor' 2169874; Set-RealSabDword 'TaskbarAlpha' 255; Set-RealSabDword 'TaskbarBlur' 0
}
try {
    foreach ($style in $Styles) {
        $safe = ($style -replace '\.msstyles$', '') -replace '\W', '_'
        $dir = "$Out\$safe"; New-Item -ItemType Directory -Force $dir | Out-Null
        Say "=== $style"
        $haveSab = $ReuseSab -and (Test-Path "$dir\sab-idle.png")
        if (-not $haveSab) { if ($style -eq 'Default') { Remove-RealSabValue @('TaskbarStyle') } else { Set-RealSabString 'TaskbarStyle' $style } }

        # 1. StartAllBack idle (fresh Explorer).
        if (-not $haveSab) { Say (Restart-ExplorerFull); Say ([Grab]::Tray("$dir\sab-idle.png", -1)) }
        $img = [Drawing.Image]::FromFile("$dir\sab-idle.png"); $thick = $img.Height; $img.Dispose()
        # 2. Ours idle, for its geometry (where to hover).
        Run-Ours $base $style $thick "$dir\ours-idle" -1
        $geom = Get-Content "$dir\ours-idle.txt"
        $buttons = @($geom | Where-Object { $_ -match '^GEOM button ' } | ForEach-Object { $f = $_ -split ' '; [pscustomobject]@{ x0 = [int]$f[2]; x1 = [int]$f[4] } })
        Say ($geom | Where-Object { $_ -match '^(bar|settings) ' } | Out-String).Trim()
        Say "state: ours has $($buttons.Count) buttons; SAB photo $($img.Width)x$thick"
        # 3. Hover states, each on a fresh Explorer.
        $shots = @('idle')
        foreach ($k in $HoverButtons) {
            if ($k -ge $buttons.Count) { continue }
            $name = "hover$k"; $shots += $name
            $x = [int](($buttons[$k].x0 + $buttons[$k].x1) / 2)
            if (-not ($ReuseSab -and (Test-Path "$dir\sab-$name.png"))) { Say (Restart-ExplorerFull); Say ([Grab]::Tray("$dir\sab-$name.png", $x)) }
            Run-Ours $base $style $thick "$dir\ours-$name" $k
        }
        # 4. Differences.
        foreach ($shot in $shots) {
            Say "-- $style / ${shot}: $([Grab]::Diff("$dir\sab-$shot.png", "$dir\ours-$shot.png", "$dir\stack-$shot.png", "$dir\diff-$shot.png"))"
            Say ((Report-Elements "$dir\sab-$shot.png" "$dir\ours-$shot.png" "$dir\ours-idle.txt") -join "`n")
        }
    }
} finally {
    if (-not $ReuseSab) { if ($Opaque) { Remove-RealSabValue @('TaskbarColoring') }; Import-RealSab $backup; Restart-ExplorerFull | Out-Null }
    Say "Restored your StartAllBack settings from $backup and restarted Explorer."
    Set-Content "$Out\report.txt" $report
}
