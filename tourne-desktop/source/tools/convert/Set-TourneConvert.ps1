<#
Tourne Convert: one "Convert" flyout in File Explorer's right-click menu that merges Convertify and File Converter,
made only of registry verbs (nothing loads into Explorer when the menu opens, so it can't slow the menu down).

Per file type (HKCU\Software\Classes\SystemFileAssociations\.<ext>\shell\TourneConvert):
  - every target format: File Converter's preset(s) where it has one (its quality settings, many files in one
    window), else Convertify's;
  - File Converter's preset folders (Scale 75%, Rotate left...) as flyouts;
  - Tools: Convertify's extras (4x Upscale, Remove Background, Compress...).
Then Convertify's own entries are hidden (LegacyDisable, kept) and File Converter's shell extension is blocked
(HKCU Shell Extensions\Blocked), so each shows once. -Undo puts everything back.

Run it again after changing File Converter's presets or updating Convertify. No icons (the user's menus have none).
Writes HKCU: run it where the real hive is written (on Tourne's PC, a process started through WMI).

    .\tools\convert\Set-TourneConvert.ps1            # build the menu
    .\tools\convert\Set-TourneConvert.ps1 -DryRun    # print what it would make
    .\tools\convert\Set-TourneConvert.ps1 -Undo      # remove it, show the originals again
#>
[CmdletBinding()]
param(
    [switch]$Undo,
    [switch]$DryRun,
    [string]$FileConverterDir = 'D:\Applications\File Converter',
    [string]$FileConverterSettings = "$env:LOCALAPPDATA\FileConverter\Settings.user.xml",
    [string]$Log = ''
)
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }

$hkcuSfa = 'HKCU:\Software\Classes\SystemFileAssociations'
$hkcrSfa = 'Registry::HKEY_CLASSES_ROOT\SystemFileAssociations'
$fcClsid = '{af9b72b5-f4e4-44b0-a3d9-b55b748efe90}'
$blocked = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Blocked'
$fcExe = Join-Path $FileConverterDir 'FileConverter.exe'
# Folders: Convertify only offers compressing them; that becomes a "Compress" flyout of the same kind.
$dirConvertify = 'HKCU:\Software\Classes\Directory\shell\Convertify'
$dirTourne = 'HKCU:\Software\Classes\Directory\shell\TourneCompress'

function Set-ConvertifyHidden([bool]$hide) {
    $n = 0
    foreach ($ext in Get-ChildItem -LiteralPath $hkcrSfa -ErrorAction SilentlyContinue) {
        $path = "$($ext.PSPath)\shell\Convertify"
        if (-not (Test-Path -LiteralPath $path)) { continue }
        # (where Convertify wrote it: HKCU for a per-user install)
        $own = "$hkcuSfa\$($ext.PSChildName)\shell\Convertify"
        if (-not (Test-Path -LiteralPath $own)) { continue }
        if ($hide) { New-ItemProperty -LiteralPath $own -Name LegacyDisable -Value '' -PropertyType String -Force | Out-Null }
        else { Remove-ItemProperty -LiteralPath $own -Name LegacyDisable -ErrorAction SilentlyContinue }
        $n++
    }
    $n
}

if ($Undo) {
    $removed = 0
    foreach ($ext in Get-ChildItem -LiteralPath $hkcuSfa -ErrorAction SilentlyContinue) {
        $path = "$($ext.PSPath)\shell\TourneConvert"
        if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Recurse -Force; $removed++ }
    }
    $shown = Set-ConvertifyHidden $false
    if (Test-Path -LiteralPath $dirTourne) { Remove-Item -LiteralPath $dirTourne -Recurse -Force }
    if (Test-Path -LiteralPath $dirConvertify) { Remove-ItemProperty -LiteralPath $dirConvertify -Name LegacyDisable -ErrorAction SilentlyContinue }
    if (Test-Path -LiteralPath $blocked) { Remove-ItemProperty -LiteralPath $blocked -Name $fcClsid -ErrorAction SilentlyContinue }
    "Removed Tourne Convert from $removed file types; Convertify shown again on $shown; File Converter's extension unblocked."
    if ($Log) { Stop-Transcript | Out-Null }
    return
}

# ---- What each offers, per extension ----------------------------------------------------------------------------

# Convertify: its verbs as it registered them (label, command).
$convertify = @{}
foreach ($ext in Get-ChildItem -LiteralPath $hkcrSfa -ErrorAction SilentlyContinue) {
    $shell = "$($ext.PSPath)\shell\Convertify\shell"
    if (-not (Test-Path -LiteralPath $shell)) { continue }
    $items = foreach ($verb in Get-ChildItem -LiteralPath $shell | Sort-Object PSChildName) {
        $label = (Get-ItemProperty -LiteralPath $verb.PSPath -ErrorAction SilentlyContinue).MUIVerb
        $cmd = (Get-ItemProperty -LiteralPath "$($verb.PSPath)\command" -ErrorAction SilentlyContinue).'(default)'
        if (-not $label -or -not $cmd) { continue }
        $format = if ($cmd -match '--format\s+(\S+)') { $Matches[1].ToLower() } else { $null }
        [pscustomobject]@{ Label = $label; Command = $cmd; Format = $format }
    }
    $convertify[$ext.PSChildName.ToLower()] = @($items)
}

# File Converter: its presets (name, output, input types); "Folder/Name" presets go in a flyout.
$fc = @{}
if ((Test-Path -LiteralPath $fcExe) -and (Test-Path -LiteralPath $FileConverterSettings)) {
    $xml = [xml](Get-Content -LiteralPath $FileConverterSettings -Raw)
    foreach ($p in $xml.SelectNodes('//ConversionPreset')) {
        $name = $p.GetAttribute('Name')
        $folder, $label = if ($name -match '^(.+)/([^/]+)$') { $Matches[1], $Matches[2] } else { '', $name }
        $label = $label -replace '^To\s+', ''
        $preset = [pscustomobject]@{
            Name = $name; Folder = $folder; Label = $label; Format = $p.GetAttribute('OutputType').ToLower()
            Command = "`"$fcExe`" --conversion-preset `"$name`" `"%1`""
        }
        foreach ($in in $p.SelectNodes('InputTypes')) {
            $ext = '.' + $in.InnerText.Trim().ToLower()
            if (-not $fc[$ext]) { $fc[$ext] = [System.Collections.Generic.List[object]]::new() }
            $fc[$ext].Add($preset)
        }
    }
} else {
    Write-Warning "File Converter not found ($fcExe, $FileConverterSettings): Convertify only."
}

# ---- The menu, per extension ------------------------------------------------------------------------------------

# "mp4" -> "MP4", "Gif (low quality)" -> "GIF (low quality)".
function Upper([string]$s) { if ($s -match '^([A-Za-z0-9]{2,5})((?=\W|$).*)$') { $Matches[1].ToUpper() + $Matches[2] } else { $s } }

# A video's sound alone goes in a flyout of its own.
$audio = 'mp3 wav flac aac ogg opus m4a wma aiff ac3' -split ' '
$video = '3gp 3gpp apng avi bik flv m4v mkv mov mp4 mpeg mpg mxf ogv rm ts vob webm wmv' -split ' ' 

function New-Menu($ext) {
    $cv = @($convertify[$ext] | Where-Object { $_ }); $presets = @($fc[$ext] | Where-Object { $_ })
    $isVideo = $video -contains $ext.TrimStart('.')
    $sound = [System.Collections.Generic.List[object]]::new()
    $formats = [System.Collections.Generic.List[object]]::new()
    $done = @{}
    # Formats in Convertify's order, then File Converter's others; File Converter wins where both have one.
    $order = @($cv | Where-Object Format | ForEach-Object Format) + @($presets | Where-Object { -not $_.Folder } | ForEach-Object Format)
    foreach ($f in $order) {
        if (-not $f -or $done[$f] -or $f -eq $ext.TrimStart('.')) { continue }
        $done[$f] = $true
        $to = $formats
        if ($isVideo -and $audio -contains $f) { $to = $sound }
        $mine = @($presets | Where-Object { -not $_.Folder -and $_.Format -eq $f })
        if ($mine) { foreach ($p in $mine) { $to.Add([pscustomobject]@{ Label = (Upper $p.Label); Command = $p.Command }) } }
        else {
            $c = $cv | Where-Object Format -eq $f | Select-Object -First 1
            $to.Add([pscustomobject]@{ Label = (Upper $f); Command = $c.Command })
        }
    }
    $folders = [ordered]@{}
    if ($sound.Count) { $folders['Audio only'] = $sound }
    foreach ($p in $presets | Where-Object Folder) {
        if (-not $folders[$p.Folder]) { $folders[$p.Folder] = [System.Collections.Generic.List[object]]::new() }
        $folders[$p.Folder].Add([pscustomobject]@{ Label = (Upper $p.Label); Command = $p.Command })
    }
    $tools = @($cv | Where-Object { -not $_.Format } | ForEach-Object { [pscustomobject]@{ Label = $_.Label; Command = $_.Command } })
    [pscustomobject]@{ Formats = $formats; Folders = $folders; Tools = $tools }
}

function Write-Verb([string]$parent, [int]$i, $item, [bool]$sepBefore) {
    $key = '{0}\{1:D2}' -f $parent, $i
    New-Item -Path $key -Force | Out-Null
    New-ItemProperty -LiteralPath $key -Name MUIVerb -Value $item.Label -PropertyType String -Force | Out-Null
    New-ItemProperty -LiteralPath $key -Name MultiSelectModel -Value 'Player' -PropertyType String -Force | Out-Null
    if ($sepBefore) { New-ItemProperty -LiteralPath $key -Name CommandFlags -Value 0x20 -PropertyType DWord -Force | Out-Null }
    New-Item -Path "$key\command" -Force | Out-Null
    Set-ItemProperty -LiteralPath "$key\command" -Name '(default)' -Value $item.Command
}

function Write-Flyout([string]$parent, [int]$i, [string]$label, $items, [bool]$sepBefore) {
    $key = '{0}\{1:D2}' -f $parent, $i
    New-Item -Path "$key\shell" -Force | Out-Null
    New-ItemProperty -LiteralPath $key -Name MUIVerb -Value $label -PropertyType String -Force | Out-Null
    New-ItemProperty -LiteralPath $key -Name SubCommands -Value '' -PropertyType String -Force | Out-Null
    if ($sepBefore) { New-ItemProperty -LiteralPath $key -Name CommandFlags -Value 0x20 -PropertyType DWord -Force | Out-Null }
    $j = 0
    foreach ($it in $items) { Write-Verb "$key\shell" ($j++) $it $false }
}

$exts = @($convertify.Keys) + @($fc.Keys) | Sort-Object -Unique
$made = 0
foreach ($ext in $exts) {
    $m = New-Menu $ext
    if (-not $m.Formats.Count -and -not $m.Folders.Count -and -not $m.Tools.Count) { continue }
    if ($DryRun) {
        "{0}: {1}{2}{3}" -f $ext, (($m.Formats | ForEach-Object Label) -join ', '),
            $(if ($m.Folders.Count) { ' | ' + (($m.Folders.Keys) -join ', ') }), $(if ($m.Tools) { ' | Tools: ' + (($m.Tools | ForEach-Object Label) -join ', ') })
        $made++
        continue
    }
    $root = "$hkcuSfa\$ext\shell\TourneConvert"
    if (Test-Path -LiteralPath $root) { Remove-Item -LiteralPath $root -Recurse -Force }
    New-Item -Path "$root\shell" -Force | Out-Null
    New-ItemProperty -LiteralPath $root -Name MUIVerb -Value 'Convert' -PropertyType String -Force | Out-Null
    New-ItemProperty -LiteralPath $root -Name SubCommands -Value '' -PropertyType String -Force | Out-Null
    $i = 0
    foreach ($it in $m.Formats) { Write-Verb "$root\shell" ($i++) $it $false }
    $first = $true
    foreach ($name in $m.Folders.Keys) { Write-Flyout "$root\shell" ($i++) $name $m.Folders[$name] $first; $first = $false }
    if ($m.Tools) { Write-Flyout "$root\shell" ($i++) 'Tools' $m.Tools $true }
    $made++
}

if ($DryRun) { "($made file types; nothing written)"; if ($Log) { Stop-Transcript | Out-Null }; return }

$hidden = Set-ConvertifyHidden $true
if (Test-Path -LiteralPath "$dirConvertify\shell") {
    if (Test-Path -LiteralPath $dirTourne) { Remove-Item -LiteralPath $dirTourne -Recurse -Force }
    New-Item -Path "$dirTourne\shell" -Force | Out-Null
    New-ItemProperty -LiteralPath $dirTourne -Name MUIVerb -Value 'Compress' -PropertyType String -Force | Out-Null
    New-ItemProperty -LiteralPath $dirTourne -Name SubCommands -Value '' -PropertyType String -Force | Out-Null
    $i = 0
    foreach ($verb in Get-ChildItem -LiteralPath "$dirConvertify\shell" | Sort-Object PSChildName) {
        $label = ((Get-ItemProperty -LiteralPath $verb.PSPath).MUIVerb -replace '^Compress \((.+)\)$', '$1')
        $cmd = (Get-ItemProperty -LiteralPath "$($verb.PSPath)\command" -ErrorAction SilentlyContinue).'(default)'
        if ($label -and $cmd) { Write-Verb "$dirTourne\shell" ($i++) ([pscustomobject]@{ Label = $label; Command = $cmd }) $false }
    }
    New-ItemProperty -LiteralPath $dirConvertify -Name LegacyDisable -Value '' -PropertyType String -Force | Out-Null
    "Folders: Compress (ZIP, 7Z, TAR.GZ) in place of Convertify's entry."
}
if ($fc.Count) {
    New-Item -Path $blocked -Force | Out-Null
    New-ItemProperty -LiteralPath $blocked -Name $fcClsid -Value 'File Converter (merged into Tourne Convert)' -PropertyType String -Force | Out-Null
}
"Tourne Convert on $made file types; Convertify's own entry hidden on $hidden; File Converter's extension blocked."
if ($Log) { Stop-Transcript | Out-Null }
