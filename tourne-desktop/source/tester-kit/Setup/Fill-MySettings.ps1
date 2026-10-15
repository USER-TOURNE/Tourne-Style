<#
Tourne'Suite tester kit: fills the suite's settings in from this PC, so the suite starts out matching
how this PC is already set up, then installs the Tourne pixel fonts for this account.

  * Tweaks: each of the Winaero-style tweaks is read from this PC as it is now. (The suite also
    never changes a tweak until you switch its group on and change it in its settings.)
  * StartAllBack: if it's installed, its Start menu and taskbar choices become the suite taskbar's.
  * Device batteries: only the readers for devices plugged in or paired here are turned on.
  * Fonts: CozetteVector and scientifica, installed for this account only (no admin needed).

Run it after the suite is installed in Windhawk ("2. Fill in my settings.cmd" does). It writes
my-settings.reg next to this script, shows what it found, and asks before importing it (one admin
prompt: Windhawk keeps mod settings in HKLM). -DryRun writes the .reg and stops there.
#>
param([switch]$DryRun, [switch]$NoFonts)

$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$kit = Split-Path $here
$modId = 'local@tourne-suite-test'
$settingsKey = "HKEY_LOCAL_MACHINE\SOFTWARE\Windhawk\Engine\Mods\$modId\Settings"
$values = [ordered]@{}

function Say($text) { Write-Host $text }

Say "Tourne'Suite tester kit: reading this PC"
Say ''

# Windhawk and the suite.
$windhawk = Test-Path 'HKLM:\SOFTWARE\Windhawk'
$installed = Test-Path "HKLM:\SOFTWARE\Windhawk\Engine\Mods\$modId"
if (-not $windhawk) { Say '  Windhawk: not found. Install it from https://windhawk.net first.' }
elseif (-not $installed) { Say "  Windhawk: found, but the suite isn't installed yet (step 2 in START HERE.txt). The settings are written anyway." }
else { Say '  Windhawk: found, suite installed.' }

# The backup comes first (step 1); made now if it was skipped.
if (-not $DryRun -and -not (Test-Path (Join-Path $kit 'Backup\snapshot.json'))) {
    Say '  No backup yet (step 1), so making one now:'
    & (Join-Path $here 'Snapshot.ps1') -Backup
    Say ''
}

# Tweaks: read from this PC by the same reader the suite's own build uses.
$tweaksReg = Join-Path ([IO.Path]::GetTempPath()) 'tourne-tester-tweaks.reg'
& (Join-Path $here 'Build-Tweaks.ps1') -RegOnly -SuiteId $modId -Definitions (Join-Path $here 'tweaks.def') -RegOutput $tweaksReg | Out-Null
$tweakCount = 0
foreach ($line in [IO.File]::ReadAllLines($tweaksReg)) {
    if ($line -match '^"(tweaks\.[^"]+)"=dword:([0-9a-f]{8})$') {
        if ($matches[1] -eq 'tweaks.enabled') { continue }  # (on or off is the tester's choice in Windhawk)
        if ($matches[1] -like 'tweaks.*.use') { continue }  # (each group's switch is the tester's to turn on)
        $values[$matches[1]] = [int][Convert]::ToUInt32($matches[2], 16); $tweakCount++
    } elseif ($line -match '^"(tweaks\.[^"]+)"="(.*)"$') {
        $values[$matches[1]] = $matches[2].Replace('\"', '"').Replace('\\', '\'); $tweakCount++
    }
}
Remove-Item $tweaksReg -ErrorAction SilentlyContinue
Say "  Tweaks: $tweakCount read as this PC has them."

# StartAllBack, if it's here.
. (Join-Path $here 'sab-defaults.ps1')
$sab = Get-SabDefaults
foreach ($name in $sab.Keys) { $values["taskbar.$name"] = $sab[$name] }
if ($sab.Count) { Say "  StartAllBack: found; $($sab.Count) of its choices carried over to the suite's taskbar." }
else { Say '  StartAllBack: not installed (fine: the suite works without it, see START HERE.txt).' }

# Windows' own taskbar: where it sits, how its buttons line up and combine (the suite's taskbar starts
# the same way when it's switched on).
$stuck = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\StuckRects3' -ErrorAction SilentlyContinue).Settings
if ($stuck -and $stuck.Length -gt 12) { $values['taskbar.location'] = @('left', 'top', 'right', 'bottom')[[Math]::Min([int]$stuck[12], 3)] }
$advanced = Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced' -ErrorAction SilentlyContinue
if (-not $advanced) { $advanced = [pscustomobject]@{} }
$align = $advanced.PSObject.Properties['TaskbarAl']
$values['taskbar.center'] = if ($align -and [int]$align.Value -eq 0) { 'edge' } else { 'centered' }  # (Windows 11 centres unless told not to)
$glom = $advanced.PSObject.Properties['TaskbarGlomLevel']
if ($glom) { $values['taskbar.combine'] = @('always', 'whenFull', 'never')[[Math]::Min([int]$glom.Value, 2)] }
Say "  Windows' taskbar: $(if ($values['taskbar.location']) { "$($values['taskbar.location']), " })buttons $($values['taskbar.center'])."

# Resource Redirect's icon theme and rules, if it's installed: Icon Redirect (the suite's part that does the
# same job) takes them over as they are.
$rrKey = Get-Item 'HKLM:\SOFTWARE\Windhawk\Engine\Mods\icon-resource-redirect\Settings' -ErrorAction SilentlyContinue
if ($rrKey) {
    $paths = New-Object System.Collections.Generic.List[string]
    $rows = 0
    foreach ($name in $rrKey.GetValueNames()) {
        $v = $rrKey.GetValue($name)
        if ($name -match '^themePaths\[(\d+)\]$') { if ([string]$v -and $paths -notcontains [string]$v) { $paths.Add([string]$v) } }
        elseif ($name -match '^redirectionResourcePaths\[\d+\]\.(original|redirect)$') {
            $values["iconRedirect.$name"] = [string]$v
            if ($matches[1] -eq 'original' -and [string]$v) { $rows++ }
        }
        elseif ($name -eq 'iconTheme') { $values['iconRedirect.iconTheme'] = [string]$v }
        elseif ($name -eq 'allResourceRedirect') { $values['iconRedirect.allResourceRedirect'] = [int]$v }
    }
    # Its older single theme folder joins the list of theme paths.
    $folder = [string]$rrKey.GetValue('themeFolder')
    if ($folder -and $paths -notcontains $folder) { $paths.Add($folder) }
    for ($i = 0; $i -lt $paths.Count; $i++) { $values["iconRedirect.themePaths[$i]"] = $paths[$i] }
    $rrOff = (Get-ItemProperty 'HKLM:\SOFTWARE\Windhawk\Engine\Mods\icon-resource-redirect' -ErrorAction SilentlyContinue).Disabled -eq 1
    $theme = $values['iconRedirect.iconTheme']
    Say "  Resource Redirect: found$(if ($rrOff) { ' (turned off)' }); Icon Redirect takes over $(if ($theme) { "the theme '$(($theme -split '\|')[0])', " })$($paths.Count) theme path(s) and $rows rule(s)."
    if (-not $rrOff) { Say '    Turn Resource Redirect off in Windhawk once your icons show, so the two don''t both do the job.' }
} else {
    Say '  Resource Redirect: not installed.'
}

# Devices with a battery reader: by the vendor ids of what's plugged in or paired.
$ids = @(Get-PnpDevice -PresentOnly -ErrorAction SilentlyContinue | ForEach-Object { $_.InstanceId })
function Has-Vid([string[]]$vids) {
    foreach ($id in $ids) {
        foreach ($m in [regex]::Matches($id, 'VID[_&]([0-9A-Fa-f]{4,8})')) {
            $v = $m.Groups[1].Value; $v = $v.Substring($v.Length - 4)
            if ($vids -contains $v.ToUpper()) { return $true }
        }
    }
    $false
}
$vendors = [ordered]@{
    alienware   = Has-Vid '413C'
    razer       = Has-Vid '1532'
    logitech    = Has-Vid '046D'
    steelseries = Has-Vid '1038'
    hyperx      = Has-Vid '03F0', '0951'
    audeze      = Has-Vid '3329'
    mchose      = Has-Vid '5253', '3837', 'A8A5'
    wlmouse     = Has-Vid '36A7'
    playstation = Has-Vid '054C'
    # Xbox-protocol controllers show up as XInput ("IG_") devices, whoever made them.
    xbox        = [bool]($ids | Where-Object { $_ -match 'IG_[0-9A-F]{2}' })
    bluetooth   = [bool](Get-PnpDevice -Class Bluetooth -PresentOnly -ErrorAction SilentlyContinue)
}
$found = @()
foreach ($name in $vendors.Keys) {
    $values["systemIcons.batteries.vendors.$name"] = [int]$vendors[$name]
    if ($vendors[$name]) { $found += $name }
}
Say "  Device batteries: readers on for $(if ($found) { $found -join ', ' } else { 'none' })."

# The .reg.
$reg = New-Object System.Collections.Generic.List[string]
$reg.Add('Windows Registry Editor Version 5.00')
$reg.Add('')
$reg.Add("[$settingsKey]")
foreach ($name in $values.Keys) {
    $v = $values[$name]
    $regName = $name.Replace('\', '\\').Replace('"', '\"')
    if ($v -is [int] -or $v -is [bool]) { $reg.Add(('"{0}"=dword:{1:x8}' -f $regName, [int]$v)) }
    else { $reg.Add(('"{0}"="{1}"' -f $regName, ([string]$v).Replace('\', '\\').Replace('"', '\"'))) }
}
$regPath = Join-Path $kit 'my-settings.reg'
[IO.File]::WriteAllText($regPath, ($reg -join "`r`n") + "`r`n", [Text.Encoding]::Unicode)
Say ''
Say "Wrote $regPath ($($values.Count) settings)."

# Fonts, for this account.
if (-not $NoFonts -and -not $DryRun) {
    $fontDir = Join-Path $env:LOCALAPPDATA 'Microsoft\Windows\Fonts'
    New-Item -ItemType Directory -Force $fontDir | Out-Null
    Add-Type -Namespace TourneKit -Name Fonts -MemberDefinition @'
[DllImport("gdi32.dll", CharSet = CharSet.Unicode)] public static extern int AddFontResourceW(string file);
[DllImport("user32.dll")] public static extern System.IntPtr SendMessageTimeoutW(System.IntPtr wnd, uint msg, System.IntPtr w, System.IntPtr l, uint flags, uint timeout, out System.IntPtr result);
'@
    $names = @{
        'CozetteVector.ttf' = 'CozetteVector (TrueType)'; 'CozetteVectorBold.ttf' = 'CozetteVector Bold (TrueType)'
        'scientifica.ttf' = 'scientifica (TrueType)'; 'scientificaBold.ttf' = 'scientifica Bold (TrueType)'
        'scientificaItalic.ttf' = 'scientifica Italic (TrueType)'
    }
    $count = 0
    foreach ($file in Get-ChildItem (Join-Path $kit 'Fonts') -Filter *.ttf) {
        $target = Join-Path $fontDir $file.Name
        if (-not (Test-Path $target)) { Copy-Item $file.FullName $target }
        New-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Fonts' -Name $names[$file.Name] -Value $target -PropertyType String -Force | Out-Null
        [void][TourneKit.Fonts]::AddFontResourceW($target)
        $count++
    }
    $r = [IntPtr]::Zero
    [void][TourneKit.Fonts]::SendMessageTimeoutW([IntPtr]0xffff, 0x001D, [IntPtr]::Zero, [IntPtr]::Zero, 2, 1000, [ref]$r)  # WM_FONTCHANGE
    Say "Installed $count fonts for this account."
}

if ($DryRun) { return }

Say ''
$answer = Read-Host 'Import these settings into the suite now? An admin prompt follows. (Y/N)'
if ($answer -notmatch '^[Yy]') {
    Say 'Not imported. Double-click my-settings.reg later to import it.'
    return
}
$p = Start-Process reg.exe -ArgumentList 'import', "`"$regPath`"" -Verb RunAs -Wait -PassThru
if ($p.ExitCode -eq 0) {
    Say ''
    Say 'Imported. In Windhawk, turn the suite off and on again (or restart Windhawk) so it reads them.'
} else {
    Say "The import didn't go through (code $($p.ExitCode)). Double-click my-settings.reg to try again."
}
