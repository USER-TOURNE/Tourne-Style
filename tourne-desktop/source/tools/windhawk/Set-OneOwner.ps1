# One owner per job (knowledge/consistency-audit-2026-10-07.md): the Windhawk side of it, in one go.
# Needs administrator rights (Windhawk keeps its settings under HKLM). Every mod's settings as they
# were are exported beside this script first (*.reg here is ignored by git).
#
#   .\tools\windhawk\Set-OneOwner.ps1 -WhatIf            # say what would change
#   .\tools\windhawk\Set-OneOwner.ps1                    # items 4, 7, 9, 10 of the audit
#   .\tools\windhawk\Set-OneOwner.ps1 -DesktopInTourne   # and decision A (b53 inside TourneDesktop.exe)
#
#  - b53's Tweaks part off (it moves to its own mod, tourne-tweaks; every group is use = 0 now).
#  - Tourne Desktop's icon rows (taskbar.buttonIcons, from tourne-gui.conf) copied into b53, so Icon
#    Redirect gives Task Manager, Explorer and every other program the same icons.
#  - win11-accent-border off (Tourne Desktop's borders take accent / accentInactive now) and
#    local@taskbar-thumbnail-hide-icon off (Explorer's previews aren't shown in shell mode).
#  - With -DesktopInTourne: b53 no longer excluded from TourneDesktop.exe (Theme Gaps runs there too: it only
#    swaps #383838 drawn by Windows' dialog code), which is added to Menu No Icons' and Menu No Duplicates' processes; remove-context-menu-items
#    also runs there (Tourne Desktop fades the desktop's selection box itself: shell.marquee); eradicate-immersive-menus off (b53's Classic Menus now
#    covers the desktop too).
# Each changed mod's SettingsChangeTime is bumped, then the Windhawk service restarts so every
# program takes the changes in.
[CmdletBinding(SupportsShouldProcess)]
param(
    [string]$Build = 'b53',
    [switch]$DesktopInTourne
)
$ErrorActionPreference = 'Stop'
$mods = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
$suite = "$mods\local@tourne-suite-$Build"
if (-not (Test-Path $suite)) { throw "local@tourne-suite-$Build isn't in Windhawk." }
$principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (-not $WhatIfPreference -and -not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw 'Run this as administrator (Windhawk keeps its settings under HKLM).'
}

if (-not $WhatIfPreference) {
    $backup = Join-Path $PSScriptRoot ('mods-before-one-owner-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
    & reg.exe export 'HKLM\SOFTWARE\Windhawk\Engine\Mods' $backup /y | Out-Null
    Write-Output "Saved every mod's settings as they were: $backup"
}

$changed = [System.Collections.Generic.HashSet[string]]::new()
function Set-Value([string]$Key, [string]$Name, $Value, [string]$Type) {
    $now = (Get-ItemProperty $Key -Name $Name -ErrorAction SilentlyContinue).$Name
    if ("$now" -eq "$Value") { return }
    if ($PSCmdlet.ShouldProcess("$Key\$Name", "set to '$Value' (was '$now')")) {
        Set-ItemProperty $Key -Name $Name -Value $Value -Type $Type
    }
    Write-Output "  $(Split-Path $Key -Leaf)\$Name = $Value (was '$now')"
    [void]$changed.Add(($Key -replace '\\Settings$', ''))
}
# A comma- or bar-separated list with one more entry (kept as it's written).
function With-Entry([string]$List, [string]$Entry, [string]$Separator) {
    $items = @($List -split [regex]::Escape($Separator.Trim()) | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    if ($items -contains $Entry) { return $List }
    (@($items) + $Entry) -join $Separator
}
function Without-Entry([string]$List, [string]$Entry) {
    (@($List -split '\|' | Where-Object { $_ -and $_ -ne $Entry })) -join '|'
}

Write-Output "Tourne'Suite ${Build}:"
Set-Value "$suite\Settings" 'tweaks.enabled' 0 DWord

# Tourne Desktop's icon rows into b53 (Icon Redirect reads taskbar.buttonIcons[...] marked everywhere).
$conf = Join-Path $env:APPDATA 'Tourne\Desktop\tourne-gui.conf'
$rows = @{}
if (Test-Path $conf) {
    foreach ($line in Get-Content $conf) {
        if ($line -match '^taskbar\.buttonIcons\[(\d+)\]\.(program|icon|everywhere) = (.*)$') {
            $i = [int]$matches[1]
            if (-not $rows.ContainsKey($i)) { $rows[$i] = @{} }
            $rows[$i][$matches[2]] = $matches[3].Trim().Trim('"')
        }
    }
}
$n = 0
foreach ($i in ($rows.Keys | Sort-Object)) {
    $row = $rows[$i]
    if (-not $row.program -or -not $row.icon) { continue }
    Set-Value "$suite\Settings" "taskbar.buttonIcons[$n].program" $row.program String
    Set-Value "$suite\Settings" "taskbar.buttonIcons[$n].icon" $row.icon String
    Set-Value "$suite\Settings" "taskbar.buttonIcons[$n].everywhere" ([int]($row.everywhere -in '1', 'true')) DWord
    $n++
}
Write-Output "  $n icon rows from Tourne Desktop"

foreach ($off in 'win11-accent-border', 'local@taskbar-thumbnail-hide-icon') {
    if (Test-Path "$mods\$off") { Write-Output "${off}:"; Set-Value "$mods\$off" 'Disabled' 1 DWord }
}

if ($DesktopInTourne) {
    Write-Output "Tourne'Suite ${Build} inside TourneDesktop.exe:"
    Set-Value $suite 'Exclude' (Without-Entry (Get-ItemProperty $suite).Exclude 'TourneDesktop.exe') String
    $s = Get-ItemProperty "$suite\Settings"
    Set-Value "$suite\Settings" 'menuNoIcons.processes' (With-Entry $s.'menuNoIcons.processes' 'TourneDesktop.exe' ', ') String
    Set-Value "$suite\Settings" 'menuNoDuplicates.processes' (With-Entry $s.'menuNoDuplicates.processes' 'TourneDesktop.exe' ', ') String
    foreach ($mod in 'remove-context-menu-items') {
        if (-not (Test-Path "$mods\$mod")) { continue }
        Write-Output "${mod}:"
        Set-Value "$mods\$mod" 'IncludeCustom' (With-Entry (Get-ItemProperty "$mods\$mod").IncludeCustom 'TourneDesktop.exe' '|') String
    }
    if (Test-Path "$mods\eradicate-immersive-menus") { Write-Output 'eradicate-immersive-menus:'; Set-Value "$mods\eradicate-immersive-menus" 'Disabled' 1 DWord }
}

if (-not $changed.Count) { Write-Output 'Nothing to change.'; return }
$stamp = [DateTimeOffset]::UtcNow.ToUnixTimeSeconds()
foreach ($key in $changed) {
    if ($PSCmdlet.ShouldProcess($key, 'bump SettingsChangeTime')) { Set-ItemProperty $key -Name SettingsChangeTime -Value $stamp -Type DWord }
}
if ($PSCmdlet.ShouldProcess('Windhawk service', 'restart')) {
    Restart-Service Windhawk
    Write-Output 'Windhawk restarted: every program takes the changes in.'
}
