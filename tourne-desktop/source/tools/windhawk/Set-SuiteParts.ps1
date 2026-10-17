# Turns parts of a Tourne'Suite build in Windhawk on or off, for running it beside
# Tourne Desktop: Tourne Desktop draws the taskbar, tray, clock, switcher and
# flyouts, and the suite keeps only what has to run inside other programs.
# Needs administrator rights (Windhawk keeps its settings under HKLM). The
# settings as they were are exported beside this script first.
#
#   .\tools\windhawk\Set-SuiteParts.ps1 -HooksOnly           # b53: hook parts on, the rest off, mod enabled
#   .\tools\windhawk\Set-SuiteParts.ps1 -Off explorerStyle   # a part Tourne Desktop has taken over
[CmdletBinding(PositionalBinding = $false)]
param(
    [string]$Build = 'b53',
    [switch]$HooksOnly,
    [string[]]$Off = @(),
    [string[]]$On = @()
)
$ErrorActionPreference = 'Stop'
$name = "local@tourne-suite-$Build"
$key = "HKLM:\SOFTWARE\Windhawk\Engine\Mods\$name"
if (-not (Test-Path $key)) { throw "$name isn't in Windhawk." }
$principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw 'Run this as administrator (Windhawk keeps its settings under HKLM).'
}

$backup = Join-Path $PSScriptRoot ("$name-" + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
& reg.exe export ($key -replace '^HKLM:', 'HKLM') $backup /y | Out-Null
Write-Output "Saved the settings as they were: $backup"

# What Tourne Desktop draws itself, and what still has to run inside other programs.
$drawn = 'taskbar', 'taskbar.startMenu', 'switcher', 'systemIcons', 'trayIcons', 'trayFolders', 'clock'
$hooks = 'explorerStyle', 'explorerColors', 'themeGaps', 'classicMenus', 'textColor', 'menuNoIcons', 'menuNoDuplicates', 'iconRedirect', 'tweaks'
if ($HooksOnly) {
    $Off += $drawn
    $On += $hooks
}
foreach ($part in $Off) { Set-ItemProperty "$key\Settings" -Name "$part.enabled" -Value 0 -Type DWord; Write-Output "off: $part" }
foreach ($part in $On) { Set-ItemProperty "$key\Settings" -Name "$part.enabled" -Value 1 -Type DWord; Write-Output "on:  $part" }
if ($HooksOnly) {
    # Never inside Tourne Desktop itself: a crash there would count against shell mode.
    $exclude = "$((Get-ItemProperty $key).Exclude)"
    if ($exclude -notmatch '(^|\|)TourneDesktop\.exe(\||$)') {
        $exclude = (@($exclude -split '\|' | Where-Object { $_ }) + 'TourneDesktop.exe') -join '|'
        Set-ItemProperty $key -Name Exclude -Value $exclude -Type String
        Write-Output "excluded: $exclude"
    }
    Set-ItemProperty $key -Name Disabled -Value 0 -Type DWord
    Write-Output "$name is enabled again."
}
Write-Output 'Windhawk takes this in when the mod is next loaded: in Windhawk, open the mod and turn it off and on (or restart Windhawk).'
