<#
The tester kit's backup and reset: a snapshot of everything the suite can change on this PC, taken
before it's installed, and putting it all back.

  -Backup   Records it in ..\Backup (once: a second run keeps the first, which is the real "before").
  -Reset    Puts it back. Run without admin; it asks once for admin for the machine-wide part.

What's recorded: every registry value and key any of the Tweaks can write (read from tweaks.def), the
folders Tweaks can add to This PC and the desktop, drive and shell icon overrides, accent colours,
Control Panel\Desktop and Mouse (window metrics, fonts, menu delay, focus follows mouse and the other
live settings), StartAllBack's on/off switch, Windows' taskbar auto-hide, the theme in use, and which
Tourne fonts and Tourne settings keys already existed.
#>
param([switch]$Backup, [switch]$Reset, [switch]$MachinePart, [switch]$RestorePoint, [string]$Folder = '')

$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
if (-not $Folder) { $Folder = Join-Path (Split-Path $here) 'Backup' }
$snapshotFile = Join-Path $Folder 'snapshot.json'
$modKey = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods\local@tourne-suite-test'

Add-Type -Namespace TourneKit -Name Shell -MemberDefinition @'
[StructLayout(LayoutKind.Sequential)] public struct APPBARDATA {
    public int cbSize; public System.IntPtr hWnd; public uint uCallbackMessage; public uint uEdge;
    public int left, top, right, bottom; public System.IntPtr lParam; }
[DllImport("shell32.dll")] public static extern System.UIntPtr SHAppBarMessage(uint msg, ref APPBARDATA data);
[DllImport("gdi32.dll", CharSet = CharSet.Unicode)] public static extern bool RemoveFontResourceW(string file);
'@ -ErrorAction SilentlyContinue

function Say($text) { Write-Host $text }

# "HKCU\..." -> (hive, subkey) in the 64-bit view.
function Open-Base([string]$hive) {
    $h = if ($hive -eq 'HKLM') { [Microsoft.Win32.RegistryHive]::LocalMachine } else { [Microsoft.Win32.RegistryHive]::CurrentUser }
    [Microsoft.Win32.RegistryKey]::OpenBaseKey($h, [Microsoft.Win32.RegistryView]::Registry64)
}
function Split-RegPath([string]$path) { $hive, $sub = $path -split '\\', 2; @($hive, $sub) }
function Test-Key([string]$path) {
    $hive, $sub = Split-RegPath $path
    $k = (Open-Base $hive).OpenSubKey($sub)
    if ($k) { $k.Close(); $true } else { $false }
}

function Read-Value([string]$path, [string]$name) {
    $hive, $sub = Split-RegPath $path
    $k = (Open-Base $hive).OpenSubKey($sub)
    $r = [ordered]@{ path = $path; name = $name; keyExisted = [bool]$k; exists = $false }
    if ($k) {
        $n = if ($name -eq '@') { '' } else { $name }
        if ($k.GetValueNames() -contains $n) {
            $kind = $k.GetValueKind($n)
            $data = $k.GetValue($n, $null, 'DoNotExpandEnvironmentNames')
            $r.exists = $true
            $r.kind = [string]$kind
            $r.data = switch ($kind) {
                'Binary' { [Convert]::ToBase64String([byte[]]$data) }
                'MultiString' { @($data) }
                default { $data }
            }
        }
        $k.Close()
    } else {
        # The nearest key above it that's here, so a reset removes only what was made below it.
        $p = Split-Path $path
        while ($p -match '\\' -and -not (Test-Key $p)) { $p = Split-Path $p }
        $r.root = $p
    }
    $r
}

function Write-Value($v) {
    $hive, $sub = Split-RegPath $v.path
    $base = Open-Base $hive
    $n = if ($v.name -eq '@') { '' } else { $v.name }
    if ($v.exists) {
        $k = $base.CreateSubKey($sub)
        $data = switch ($v.kind) {
            'Binary' { [Convert]::FromBase64String($v.data) }
            'MultiString' { [string[]]@($v.data) }
            'DWord' { [int]$v.data }
            'QWord' { [long]$v.data }
            default { [string]$v.data }
        }
        $k.SetValue($n, $data, [Microsoft.Win32.RegistryValueKind]$v.kind)
        $k.Close()
    } else {
        $k = $base.OpenSubKey($sub, $true)
        if ($k) { if ($k.GetValueNames() -contains $n) { $k.DeleteValue($n) }; $k.Close() }
    }
}

# A key the suite made: gone once nothing is left in it, and its parents the same way.
function Remove-IfEmptyMade([string]$path, $madeKeys) {
    while ($path -and $madeKeys.Contains($path)) {
        $hive, $sub = Split-RegPath $path
        $base = Open-Base $hive
        $k = $base.OpenSubKey($sub)
        if (-not $k) { $path = Split-Path $path; continue }
        $empty = $k.ValueCount -eq 0 -and $k.SubKeyCount -eq 0
        $k.Close()
        if (-not $empty) { return }
        $base.DeleteSubKey($sub, $false)
        $path = Split-Path $path
    }
}

function Remove-KeyTree([string]$path) {
    $hive, $sub = Split-RegPath $path
    try { (Open-Base $hive).DeleteSubKeyTree($sub, $false) } catch { Say "    couldn't remove $path ($($_.Exception.Message))" }
}

function Export-Key([string]$path, [string]$file) {
    # (Through cmd: reg.exe reports success on stderr, which Windows PowerShell would take for an error.)
    & cmd.exe /c "reg.exe export `"$path`" `"$file`" /y /reg:64 >nul 2>&1"
    $LASTEXITCODE -eq 0
}
function Import-File([string]$file) {
    & cmd.exe /c "reg.exe import `"$file`" /reg:64 >nul 2>&1"
    $LASTEXITCODE -eq 0
}

$placeholder = '\{(\}|@|twips\}|ms\}|drives\}|abgr\}|cmd\})'
$fontNames = 'CozetteVector.ttf', 'CozetteVectorBold.ttf', 'scientifica.ttf', 'scientificaBold.ttf', 'scientificaItalic.ttf'
$wholeKeys = @(
    'HKCU\Control Panel\Desktop', 'HKCU\Control Panel\Mouse',
    'HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Accents',
    'HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons',
    'HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Icons'
)
$extraValues = @(@('HKCU\Software\StartIsBack', 'Disabled'))
$suiteKeys = 'HKCU\Software\TourneTray', 'HKCU\Software\Tourne'

function Get-NameSpace([string]$where) {
    $hive, $sub = Split-RegPath "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\$where\NameSpace"
    $k = (Open-Base $hive).OpenSubKey($sub)
    if (-not $k) { return @() }
    $names = $k.GetSubKeyNames(); $k.Close(); @($names)
}

function Get-AutoHide {
    $abd = New-Object TourneKit.Shell+APPBARDATA
    $abd.cbSize = [Runtime.InteropServices.Marshal]::SizeOf($abd)
    ([uint64][TourneKit.Shell]::SHAppBarMessage(4, [ref]$abd) -band 1) -ne 0  # ABM_GETSTATE, ABS_AUTOHIDE
}

############################################################ restore point

# Windows' own safety net beside the backup: a System Restore point, made with admin rights (this script
# again, elevated). Exit codes: 0 made, 2 System Protection couldn't be turned on, 3 Windows didn't make it.
$restorePointName = "Before Tourne'Suite"
if ($RestorePoint) {
    $ErrorActionPreference = 'Continue'
    $drive = "$env:SystemDrive\"
    try { Enable-ComputerRestore -Drive $drive -ErrorAction Stop } catch { exit 2 }
    # Windows makes at most one restore point a day for scripts; that limit lifted for this one, then put back.
    $srKey = 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\SystemRestore'
    $old = (Get-ItemProperty $srKey -ErrorAction SilentlyContinue).SystemRestorePointCreationFrequency
    New-ItemProperty $srKey -Name SystemRestorePointCreationFrequency -Value 0 -PropertyType DWord -Force | Out-Null
    try { Checkpoint-Computer -Description $restorePointName -RestorePointType MODIFY_SETTINGS -ErrorAction Stop } catch { }
    if ($null -eq $old) { Remove-ItemProperty $srKey -Name SystemRestorePointCreationFrequency -ErrorAction SilentlyContinue }
    else { Set-ItemProperty $srKey -Name SystemRestorePointCreationFrequency -Value $old }
    $made = Get-ComputerRestorePoint -ErrorAction SilentlyContinue | Where-Object { $_.Description -eq $restorePointName } |
        Where-Object { $_.ConvertToDateTime($_.CreationTime) -gt (Get-Date).AddMinutes(-15) }
    exit $(if ($made) { 0 } else { 3 })
}

function Offer-RestorePoint {
    Say ''
    Say 'Also make a Windows restore point? It can roll back the few things the backup can''t read (boot'
    Say 'options, hibernation, the built-in Administrator account), but it rolls back everything else on'
    Say 'the PC since then too, so the Reset is the first thing to use. Needs admin; takes a minute or two.'
    if ((Read-Host 'Make one? (Y/N)') -notmatch '^[Yy]') { Say 'No restore point made.'; return }
    Say 'Making it (System Protection is turned on for your system drive if it was off)...'
    try {
        $p = Start-Process powershell.exe -Verb RunAs -Wait -PassThru -WindowStyle Hidden -ArgumentList '-NoProfile',
            '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"", '-RestorePoint'
    } catch { Say 'Skipped (the admin prompt was declined).'; return }
    switch ($p.ExitCode) {
        0 { Say "Made: a restore point called `"$restorePointName`"." }
        2 { Say "System Protection couldn't be turned on here (a work PC may block it), so no restore point." }
        default { Say "Windows didn't make the restore point (code $($p.ExitCode)). The backup is still there." }
    }
}

################################################################### backup

if ($Backup) {
    if (Test-Path $snapshotFile) {
        Say "A backup from $((Get-Item $snapshotFile).LastWriteTime) is already here; keeping it (it's the real 'before')."
        Offer-RestorePoint
        return
    }
    if (Test-Path $modKey) {
        Say 'Note: the suite is already installed. The backup still records everything, but anything changed'
        Say 'since it was installed counts as "before" now.'
    }
    New-Item -ItemType Directory -Force $Folder | Out-Null
    Say 'Backing up what the suite can change...'
    $ops = @(& (Join-Path $here 'Build-Tweaks.ps1') -ListOps -Definitions (Join-Path $here 'tweaks.def') | Where-Object { $_ }) | Sort-Object -Unique
    $values = New-Object System.Collections.Generic.List[object]
    $keys = New-Object System.Collections.Generic.List[object]
    $seenValues = @{}; $seenKeys = @{}
    $n = 0
    foreach ($op in $ops) {
        if ($op -notmatch '^(HK\w+\\.*?) \| (.*)$') { continue }
        $path = $matches[1]; $spec = $matches[2]
        if ($path -match $placeholder) { continue }
        if ($spec -eq '-' -or $spec -eq '+') {
            if ($seenKeys.ContainsKey($path)) { continue }
            $seenKeys[$path] = $true
            $entry = [ordered]@{ path = $path; existed = (Test-Key $path); export = $null }
            if ($entry.existed -and $spec -eq '-') {
                $file = "key{0:d4}.reg" -f (++$n)
                if (Export-Key $path (Join-Path $Folder $file)) { $entry.export = $file }
            }
            $keys.Add($entry)
            continue
        }
        $name = if ($spec -match '^(.*) = \w+:') { $matches[1] } elseif ($spec -match '^(.*) -$') { $matches[1] } else { $null }
        if ($null -eq $name -or $name -match $placeholder) { continue }
        $id = "$path|$name"
        if ($seenValues.ContainsKey($id)) { continue }
        $seenValues[$id] = $true
        $values.Add((Read-Value $path $name))
    }
    foreach ($pair in $extraValues) { $values.Add((Read-Value $pair[0] $pair[1])) }
    $whole = foreach ($path in $wholeKeys) {
        $entry = [ordered]@{ path = $path; existed = (Test-Key $path); export = $null }
        if ($entry.existed) {
            $file = "key{0:d4}.reg" -f (++$n)
            if (Export-Key $path (Join-Path $Folder $file)) { $entry.export = $file }
        }
        $entry
    }
    $fontDir = Join-Path $env:LOCALAPPDATA 'Microsoft\Windows\Fonts'
    $snapshot = [ordered]@{
        created = (Get-Date).ToString('s')
        windows = [Environment]::OSVersion.Version.ToString()
        values = $values
        keys = $keys
        wholeKeys = @($whole)
        nameSpace = [ordered]@{ Desktop = Get-NameSpace 'Desktop'; MyComputer = Get-NameSpace 'MyComputer' }
        theme = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes' -ErrorAction SilentlyContinue).CurrentTheme
        autoHide = Get-AutoHide
        fontsBefore = @($fontNames | Where-Object { Test-Path (Join-Path $fontDir $_) })
        suiteKeysBefore = @($suiteKeys | Where-Object { Test-Key $_ })
        tourneFolderBefore = Test-Path (Join-Path $env:LOCALAPPDATA 'Tourne')
    }
    $snapshot | ConvertTo-Json -Depth 6 | Set-Content -Path $snapshotFile -Encoding UTF8
    Say "Backed up $($values.Count) settings, $($keys.Count + $whole.Count) keys and your theme to:"
    Say "  $Folder"
    Say 'Keep this folder until you no longer need to go back.'
    Offer-RestorePoint
    return
}

#################################################################### reset

if (-not $Reset -and -not $MachinePart) { Say 'Use -Backup or -Reset.'; return }
if (-not (Test-Path $snapshotFile)) { Say "No backup found in $Folder, so there's nothing to go back to."; return }
$snap = Get-Content $snapshotFile -Raw | ConvertFrom-Json
$hive = if ($MachinePart) { 'HKLM' } else { 'HKCU' }

if (-not $MachinePart) {
    Say "Putting this PC back as it was on $($snap.created -replace 'T', ' at ')."
    if (Test-Path $modKey) {
        Say ''
        Say "The suite is still installed in Windhawk. Remove it first (Windhawk: open Tourne'Suite, Remove),"
        Say 'or it may put some things back again.'
        if ((Read-Host 'Go on anyway? (Y/N)') -notmatch '^[Yy]') { return }
    }
    Say ''
}

# Keys that didn't exist before: each value's own key, and its parents below the nearest one that did.
$made = New-Object 'System.Collections.Generic.HashSet[string]'
foreach ($v in $snap.values) {
    if ($v.keyExisted -or -not $v.root) { continue }
    $p = $v.path
    while ($p -and $p -ne $v.root -and $p -match '\\') { [void]$made.Add($p); $p = Split-Path $p }
}

$restored = 0
foreach ($v in ($snap.values | Where-Object { $_.path.StartsWith("$hive\") })) {
    try { Write-Value $v; $restored++ } catch { Say "    couldn't put back $($v.path) | $($v.name) ($($_.Exception.Message))" }
}
# Deepest first, so a made key's subkeys go before it.
foreach ($v in ($snap.values | Where-Object { $_.path.StartsWith("$hive\") -and -not $_.keyExisted } | Sort-Object { $_.path.Length } -Descending)) {
    Remove-IfEmptyMade $v.path $made
}
foreach ($k in ($snap.keys | Where-Object { $_.path.StartsWith("$hive\") } | Sort-Object { $_.path.Length } -Descending)) {
    if (-not $k.existed) { if (Test-Key $k.path) { Remove-KeyTree $k.path } }
    elseif ($k.export) { [void](Import-File (Join-Path $Folder $k.export)) }
}
foreach ($k in ($snap.wholeKeys | Where-Object { $_.path.StartsWith("$hive\") })) {
    if (-not $k.existed) { if (Test-Key $k.path) { Remove-KeyTree $k.path } }
    elseif ($k.export) { [void](Import-File (Join-Path $Folder $k.export)) }
}
Say "  $(if ($MachinePart) { 'Machine-wide' } else { 'Your account''s' }) settings: $restored put back."

if ($MachinePart) { return }

# Folders Tweaks added to This PC and the desktop.
foreach ($where in 'Desktop', 'MyComputer') {
    $before = @($snap.nameSpace.$where)
    foreach ($guid in (Get-NameSpace $where)) {
        if ($before -contains $guid) { continue }
        $by = (Get-ItemProperty "HKCU:\Software\Classes\CLSID\$guid" -ErrorAction SilentlyContinue).CreatedBy
        if ($by -ne "Tourne'Suite") { continue }
        Remove-KeyTree "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\$where\NameSpace\$guid"
        Remove-KeyTree "HKCU\Software\Classes\CLSID\$guid"
        Say "  Removed a folder the suite added ($guid)."
    }
}

# The suite's own keys.
foreach ($key in $suiteKeys) {
    if (@($snap.suiteKeysBefore) -notcontains $key -and (Test-Key $key)) { Remove-KeyTree $key }
}
Say "  The suite's own settings in your account: removed."

# Windows' taskbar auto-hide, as it was.
if ((Get-AutoHide) -ne [bool]$snap.autoHide) {
    $abd = New-Object TourneKit.Shell+APPBARDATA
    $abd.cbSize = [Runtime.InteropServices.Marshal]::SizeOf($abd)
    $abd.lParam = [IntPtr]$(if ($snap.autoHide) { 1 } else { 2 })  # ABS_AUTOHIDE, or ABS_ALWAYSONTOP
    [void][TourneKit.Shell]::SHAppBarMessage(10, [ref]$abd)  # ABM_SETSTATE
    Say "  Windows' taskbar auto-hide: back $(if ($snap.autoHide) { 'on' } else { 'off' })."
}

# The kit's fonts, unless they were here before.
$fontDir = Join-Path $env:LOCALAPPDATA 'Microsoft\Windows\Fonts'
$fontsKey = 'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Fonts'
$removedFonts = 0
foreach ($file in $fontNames) {
    if (@($snap.fontsBefore) -contains $file) { continue }
    $path = Join-Path $fontDir $file
    $props = Get-ItemProperty $fontsKey -ErrorAction SilentlyContinue
    if ($props) {
        foreach ($p in $props.PSObject.Properties) { if ([string]$p.Value -eq $path) { Remove-ItemProperty $fontsKey -Name $p.Name -ErrorAction SilentlyContinue } }
    }
    if (Test-Path $path) {
        [void][TourneKit.Shell]::RemoveFontResourceW($path)
        Remove-Item $path -ErrorAction SilentlyContinue
        if (Test-Path $path) { Say "    $file is in use; it goes after you sign out (run this again then)." } else { $removedFonts++ }
    }
}
if ($removedFonts) { Say "  Fonts the kit installed: $removedFonts removed." }

# The machine-wide part (policies, Explorer's machine settings, drive icons...), with admin rights.
if (@(@($snap.values) + @($snap.keys) + @($snap.wholeKeys) | Where-Object { $_.path -like 'HKLM\*' }).Count) {
    Say '  Machine-wide settings: an admin prompt follows.'
    try {
        $p = Start-Process powershell.exe -Verb RunAs -Wait -PassThru -ArgumentList '-NoProfile', '-ExecutionPolicy', 'Bypass',
            '-File', "`"$PSCommandPath`"", '-MachinePart', '-Folder', "`"$Folder`""
        if ($p.ExitCode -ne 0) { Say "    That part reported a problem (code $($p.ExitCode)); run the reset again to retry it." }
        else { Say '  Machine-wide settings: put back.' }
    } catch {
        Say "    Skipped (the admin prompt was declined). Run the reset again to do that part."
    }
}

# Tourne's folder in your AppData, if the suite made it (it may hold themes you added).
$tourneDir = Join-Path $env:LOCALAPPDATA 'Tourne'
if (-not $snap.tourneFolderBefore -and (Test-Path $tourneDir)) {
    if ((Read-Host "  Delete $tourneDir too (styles, orbs or themes you added there go with it)? (Y/N)") -match '^[Yy]') {
        Remove-Item $tourneDir -Recurse -Force -ErrorAction SilentlyContinue
        Say '  Removed.'
    }
}

# The theme, if it changed.
$themeNow = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes' -ErrorAction SilentlyContinue).CurrentTheme
if ($snap.theme -and $themeNow -ne $snap.theme -and (Test-Path $snap.theme)) {
    Start-Process $snap.theme
    Say '  Your theme: re-applied (Settings opens while it does; close it after).'
}

Say ''
Say 'Done. Sign out and back in (or restart) so Windows reloads everything.'
Say ''
Say "Not in the automatic backup (they need admin to even read): the Tweaks for boot options (F8 boot"
Say "menu, boot logo, automatic repair), hibernation and its file size, and the built-in Administrator"
Say "account. If you changed any of those, set them back in Windows (or in the suite before removing it)."
Say "If you made the restore point in step 1, it can do those too (rstrui.exe, `"$restorePointName`"), but"
Say "it also undoes everything else changed on the PC since."
Say "If you installed Windhawk only for this, remove it in Settings > Apps."
