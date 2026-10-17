<#
Makes a copy of a Start menu visual style whose arrows (StartPanelPriv's FLYOUTARROW and PINICON) are
the suite's green pixel chevron, drawn at each image's own size. The original style is not changed.
The copy goes in %LOCALAPPDATA%\Tourne\Styles, where the suite's Start menu "Visual style" list finds it.
Uses msstyleEditor's own library (libmsstyle, inside msstyleEditor.exe 2.1.2), so it runs under
Windows PowerShell 5.1:
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File Set-GreenStartArrows.ps1
  -Style   the style to copy (a file name or full path). Default: StartAllBack's Start menu style.
  -Color   the chevron's colour. Default: #4BAA69.
#>
param(
    [string]$Style = '',
    [string]$Color = '#4BAA69',
    [string]$Editor = '%USERPROFILE%\Downloads\.theme files\Claude Files to View for reference\msstyleEditor.exe'
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$sabStyles = Join-Path $env:ProgramFiles 'StartAllBack\Styles'
$tourneStyles = Join-Path $env:LOCALAPPDATA 'Tourne\Styles'

if (-not $Style) {
    $Style = (Get-ItemProperty 'HKCU:\Software\StartIsBack' -Name AlterStyle -ErrorAction SilentlyContinue).AlterStyle
    if (-not $Style) { throw "StartAllBack has no Start menu style set. Run again with -Style 'name.msstyles'." }
}
$path = $Style
if (-not (Test-Path -LiteralPath $path)) {
    $path = @((Join-Path $sabStyles $Style), (Join-Path $tourneStyles $Style)) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (-not $path) { throw "Can't find the style '$Style' in $sabStyles or $tourneStyles." }
if (-not (Test-Path -LiteralPath $Editor)) { throw "Can't find msstyleEditor.exe at $Editor. Run again with -Editor 'path\to\msstyleEditor.exe'." }

$name = [IO.Path]::GetFileNameWithoutExtension($path)
New-Item -ItemType Directory -Force $tourneStyles | Out-Null
$out = Join-Path $tourneStyles "$name green arrows.msstyles"
"Style: $path"

$asm = [Reflection.Assembly]::UnsafeLoadFrom($Editor)
$vs = [Activator]::CreateInstance($asm.GetType('libmsstyle.VisualStyle'))
$imageType = [Enum]::Parse($asm.GetType('libmsstyle.StyleResourceType'), 'Image')
$vs.Load($path)

# Every image resource and who uses it, so one shared with another part is left alone.
$users = @{}
$arrows = @{}
foreach ($c in $vs.Classes.Values) {
    foreach ($p in $c.Parts.Values) {
        foreach ($s in $p.States.Values) {
            foreach ($prop in $s.Properties) {
                if (-not $prop.IsImageProperty()) { continue }
                $res = $null
                try { $res = $vs.GetResourceFromProperty($prop) } catch { continue }
                if (-not $res -or -not $res.Data -or $res.Type -ne $imageType) { continue }
                $id = $res.ResourceId
                # FLYOUTARROW (5), and PINICON (25): the same arrow beside a program with a jump list.
                $isArrow = $c.ClassName -match 'StartPanelPriv$' -and ($p.PartId -eq 5 -or $p.PartId -eq 25)
                if (-not $users.ContainsKey($id)) { $users[$id] = New-Object System.Collections.Generic.HashSet[string] }
                [void]$users[$id].Add($(if ($isArrow) { 'arrow' } else { "$($c.ClassName) part $($p.PartId)" }))
                if ($isArrow) { $arrows[$id] = $res.Data }
            }
        }
    }
}
if ($arrows.Count -eq 0) { $vs.Dispose(); throw "$name has no Start menu submenu arrows (StartPanelPriv FLYOUTARROW)." }

# The design's chevron (system-icons-host.cpp, kArtChevronUp) turned to point right: 4 wide, 8 tall.
$up = '.....##.....', '....####....', '...##..##...', '..##....##..'
$cells = for ($r = 0; $r -lt 4; $r++) { for ($c = 0; $c -lt 12; $c++) { if ($up[$r][$c] -eq '#') { ,@((3 - $r), ($c - 2)) } } }
$rgb = [Drawing.ColorTranslator]::FromHtml($Color)
$ink = [Drawing.Color]::FromArgb(255, $rgb.R, $rgb.G, $rgb.B)

$temp = Join-Path ([IO.Path]::GetTempPath()) "tourne-arrows-$PID"
New-Item -ItemType Directory -Force $temp | Out-Null
$done = 0
foreach ($id in $arrows.Keys) {
    if ($users[$id].Count -gt 1) { "  image $id skipped: also used by $(($users[$id] | Where-Object { $_ -ne 'arrow' }) -join ', ')"; continue }
    $d = $arrows[$id]
    if ($d.Length -lt 24 -or $d[0] -ne 0x89 -or $d[1] -ne 0x50) { "  image $id skipped: not a PNG"; continue }
    $w = ($d[16] -shl 24) -bor ($d[17] -shl 16) -bor ($d[18] -shl 8) -bor $d[19]
    $h = ($d[20] -shl 24) -bor ($d[21] -shl 16) -bor ($d[22] -shl 8) -bor $d[23]
    $bmp = New-Object Drawing.Bitmap $w, $h, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $step = [Math]::Min([Math]::Floor($w / 4), [Math]::Floor($h / 8))
    if ($step -ge 1) {
        if ($step -gt 1) { $step = [Math]::Max(1, [Math]::Min($step, [Math]::Floor([Math]::Max($w, $h) / 9))) }
        $ox = [Math]::Floor(($w - 4 * $step) / 2); $oy = [Math]::Floor(($h - 8 * $step) / 2)
        foreach ($cell in $cells) {
            for ($dx = 0; $dx -lt $step; $dx++) { for ($dy = 0; $dy -lt $step; $dy++) {
                $bmp.SetPixel($ox + $cell[0] * $step + $dx, $oy + $cell[1] * $step + $dy, $ink) } }
        }
    } else {
        # Too small for the chevron (Windows 7's 4x7): a solid arrowhead filling it.
        for ($y = 0; $y -lt $h; $y++) {
            $len = [Math]::Max(1, [Math]::Floor(($h / 2 - [Math]::Abs($y - ($h - 1) / 2)) * $w / ($h / 2)))
            for ($x = 0; $x -lt [Math]::Min($w, $len); $x++) { $bmp.SetPixel($x, $y, $ink) }
        }
    }
    $png = Join-Path $temp "$id.png"
    $bmp.Save($png, [Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    $vs.QueueResourceUpdate($id, $imageType, $png)
    "  image $id ($w x $h) replaced"
    $done++
}
if ($done -eq 0) { $vs.Dispose(); Remove-Item -Recurse -Force $temp; throw 'No arrow images could be replaced.' }
$vs.Save($out)
$vs.Dispose()
Remove-Item -Recurse -Force $temp
""
"Saved: $out"
"Now in Windhawk, Tourne'Suite settings, Taskbar, Start Menu, Visual style: pick '$name green arrows.msstyles' and Save."
