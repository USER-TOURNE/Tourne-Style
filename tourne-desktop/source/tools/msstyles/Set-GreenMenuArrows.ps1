<#
Makes a copy of your Windows visual style whose menu submenu arrows (Menu's POPUPSUBMENU, which
StartAllBack's flyouts and every classic menu use) are the suite's green pixel chevron, and a .theme
that applies it. The original style is not changed. Each arrow image holds a normal and a disabled
arrow, one above the other; the disabled one is drawn faded.
The copy goes next to the original (in C:\Windows\Resources\Themes\...), so run it from an
administrator PowerShell. Uses msstyleEditor's own library, so it runs under Windows PowerShell 5.1:
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File Set-GreenMenuArrows.ps1
  -Style   the .msstyles to copy (full path). Default: the one Windows is using now.
  -Color   the chevron's colour. Default: #4BAA69.
#>
param(
    [string]$Style = '',
    [string]$Color = '#4BAA69',
    [string]$Editor = '%USERPROFILE%\Downloads\.theme files\Claude Files to View for reference\msstyleEditor.exe'
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$admin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $admin) { throw 'Run this from PowerShell opened as administrator (it writes next to the style in C:\Windows\Resources\Themes).' }

if (-not $Style) {
    $Style = [Environment]::ExpandEnvironmentVariables(
        (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\ThemeManager' -Name DllName).DllName)
}
if (-not (Test-Path -LiteralPath $Style)) { throw "Can't find the style $Style. Run again with -Style 'full\path\to\name.msstyles'." }
if (-not (Test-Path -LiteralPath $Editor)) { throw "Can't find msstyleEditor.exe at $Editor. Run again with -Editor 'path\to\msstyleEditor.exe'." }
if ($Style -match 'green arrows\.msstyles$') { throw "$Style is already a green-arrows copy. Run again with -Style pointing at the original." }

$dir = Split-Path -Parent $Style
$name = [IO.Path]::GetFileNameWithoutExtension($Style)
$newName = "$name green arrows"
$out = Join-Path $dir "$newName.msstyles"
"Style: $Style"

$asm = [Reflection.Assembly]::UnsafeLoadFrom($Editor)
$vs = [Activator]::CreateInstance($asm.GetType('libmsstyle.VisualStyle'))
$imageType = [Enum]::Parse($asm.GetType('libmsstyle.StyleResourceType'), 'Image')
$vs.Load($Style)

# Every image resource and who uses it, so one shared with anything but a menu's submenu arrow is left alone.
$users = @{}
$arrows = @{}
foreach ($c in $vs.Classes.Values) {
    foreach ($p in $c.Parts.Values) {
        $isArrow = $c.ClassName -match '(^|::)Menu$' -and $p.PartId -eq 16  # POPUPSUBMENU
        $count = 1
        $vertical = $true
        foreach ($s in $p.States.Values) {
            foreach ($prop in $s.Properties) {
                $value = ''
                try { $value = [string]$prop.GetValueAsString() } catch {}
                if ($prop.Header.nameID -eq 2401 -and $value -match '^\d+$') { $count = [int]$value }  # IMAGECOUNT
                if ($prop.Header.nameID -eq 4011 -and $value -match 'HORIZONTAL') { $vertical = $false }  # IMAGELAYOUT
            }
        }
        foreach ($s in $p.States.Values) {
            foreach ($prop in $s.Properties) {
                if (-not $prop.IsImageProperty()) { continue }
                $res = $null
                try { $res = $vs.GetResourceFromProperty($prop) } catch { continue }
                if (-not $res -or -not $res.Data -or $res.Type -ne $imageType) { continue }
                $id = $res.ResourceId
                if (-not $users.ContainsKey($id)) { $users[$id] = New-Object System.Collections.Generic.HashSet[string] }
                [void]$users[$id].Add($(if ($isArrow) { 'arrow' } else { "$($c.ClassName) part $($p.PartId)" }))
                if ($isArrow) { $arrows[$id] = @{ Data = $res.Data; Count = $count; Vertical = $vertical } }
            }
        }
    }
}
if ($arrows.Count -eq 0) { $vs.Dispose(); throw "$name has no menu submenu arrows (Menu POPUPSUBMENU images)." }

# The design's chevron (system-icons-host.cpp, kArtChevronUp) turned to point right: 4 wide, 8 tall.
$up = '.....##.....', '....####....', '...##..##...', '..##....##..'
$cells = for ($r = 0; $r -lt 4; $r++) { for ($c = 0; $c -lt 12; $c++) { if ($up[$r][$c] -eq '#') { ,@((3 - $r), ($c - 2)) } } }
$rgb = [Drawing.ColorTranslator]::FromHtml($Color)

function DrawChevron($bmp, [int]$left, [int]$top, [int]$w, [int]$h, [Drawing.Color]$ink) {
    $step = [Math]::Min([Math]::Floor($w / 4), [Math]::Floor($h / 8))
    if ($step -lt 1) {  # too small for the chevron: a solid arrowhead filling it
        for ($y = 0; $y -lt $h; $y++) {
            $len = [Math]::Max(1, [Math]::Floor(($h / 2 - [Math]::Abs($y - ($h - 1) / 2)) * $w / ($h / 2)))
            for ($x = 0; $x -lt [Math]::Min($w, $len); $x++) { $bmp.SetPixel($left + $x, $top + $y, $ink) }
        }
        return
    }
    if ($step -gt 1) { $step = [Math]::Max(1, [Math]::Min($step, [Math]::Floor([Math]::Max($w, $h) / 9))) }
    $ox = $left + [Math]::Floor(($w - 4 * $step) / 2); $oy = $top + [Math]::Floor(($h - 8 * $step) / 2)
    foreach ($cell in $cells) {
        for ($dx = 0; $dx -lt $step; $dx++) { for ($dy = 0; $dy -lt $step; $dy++) {
            $bmp.SetPixel($ox + $cell[0] * $step + $dx, $oy + $cell[1] * $step + $dy, $ink) } }
    }
}

$temp = Join-Path ([IO.Path]::GetTempPath()) "tourne-menu-arrows-$PID"
New-Item -ItemType Directory -Force $temp | Out-Null
$done = 0
foreach ($id in $arrows.Keys) {
    if ($users[$id].Count -gt 1) { "  image $id skipped: also used by $(($users[$id] | Where-Object { $_ -ne 'arrow' }) -join ', ')"; continue }
    $a = $arrows[$id]
    $d = $a.Data
    if ($d.Length -lt 24 -or $d[0] -ne 0x89 -or $d[1] -ne 0x50) { "  image $id skipped: not a PNG"; continue }
    $w = ($d[16] -shl 24) -bor ($d[17] -shl 16) -bor ($d[18] -shl 8) -bor $d[19]
    $h = ($d[20] -shl 24) -bor ($d[21] -shl 16) -bor ($d[22] -shl 8) -bor $d[23]
    $n = [Math]::Max(1, $a.Count)
    $bmp = New-Object Drawing.Bitmap $w, $h, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
    for ($i = 0; $i -lt $n; $i++) {
        # Frame 1 is the normal arrow; the rest (disabled) are faded.
        $ink = [Drawing.Color]::FromArgb($(if ($i -eq 0) { 255 } else { 100 }), $rgb.R, $rgb.G, $rgb.B)
        if ($a.Vertical) { $fh = [Math]::Floor($h / $n); DrawChevron $bmp 0 ($i * $fh) $w $fh $ink }
        else { $fw = [Math]::Floor($w / $n); DrawChevron $bmp ($i * $fw) 0 $fw $h $ink }
    }
    $png = Join-Path $temp "$id.png"
    $bmp.Save($png, [Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    $vs.QueueResourceUpdate($id, $imageType, $png)
    "  image $id ($w x $h, $n arrows) replaced"
    $done++
}
if ($done -eq 0) { $vs.Dispose(); Remove-Item -Recurse -Force $temp; throw 'No arrow images could be replaced.' }
$vs.Save($out)
$vs.Dispose()
Remove-Item -Recurse -Force $temp

# A style needs its language files (en-US\<name>.msstyles.mui and so on) under the same name.
foreach ($mui in Get-ChildItem -LiteralPath $dir -Recurse -Filter "$name.msstyles.mui") {
    Copy-Item -LiteralPath $mui.FullName -Destination (Join-Path $mui.DirectoryName "$newName.msstyles.mui") -Force
}

# A .theme like the one in use, with only the visual style and the name changed.
$current = [Environment]::ExpandEnvironmentVariables(
    (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes' -Name CurrentTheme -ErrorAction SilentlyContinue).CurrentTheme)
$themeDir = Join-Path $env:LOCALAPPDATA 'Microsoft\Windows\Themes'
New-Item -ItemType Directory -Force $themeDir | Out-Null
$themeOut = Join-Path $themeDir "$newName.theme"
if ($current -and (Test-Path -LiteralPath $current)) {
    $lines = [IO.File]::ReadAllLines($current)
    $section = ''
    $lines = foreach ($line in $lines) {
        if ($line -match '^\s*\[(.+)\]') { $section = $Matches[1] }
        if ($section -eq 'VisualStyles' -and $line -match '^\s*Path\s*=') { "Path=$out" }
        elseif ($section -eq 'Theme' -and $line -match '^\s*DisplayName\s*=') { "DisplayName=$newName" }
        else { $line }
    }
} else {
    $lines = '[Theme]', "DisplayName=$newName", '', '[VisualStyles]', "Path=$out", 'ColorStyle=NormalColor', 'Size=NormalSize'
}
[IO.File]::WriteAllLines($themeOut, $lines, [Text.Encoding]::Unicode)
""
"Saved: $out"
"Theme: $themeOut"
"Apply it the way you applied $name (SecureUxTheme's ThemeTool), or double-click the .theme above."
"To go back, apply your old theme again."
