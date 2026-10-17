# Turns b53's Explorer style and Explorer colours parts back on (2026-10-08: both were found off, so Explorer
# lost its coral drive bars, Windows 7 drive groups and selection colours). Needs administrator rights
# (Windhawk keeps its settings under HKLM). b53's settings as they were are exported beside this script
# first (*.reg here is ignored by git). Explorer windows opened afterwards get the look.
#
#   .\tools\windhawk\Set-ExplorerLook.ps1 -WhatIf     # say what would change
#   .\tools\windhawk\Set-ExplorerLook.ps1 -Log <file> # do it, writing what changed to a file too
[CmdletBinding(SupportsShouldProcess)]
param([string]$Build = 'b53', [string]$Log = '')
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    $mod = "HKLM:\SOFTWARE\Windhawk\Engine\Mods\local@tourne-suite-$Build"
    $settings = "$mod\Settings"
    if (-not (Test-Path $settings)) { throw "Not installed: local@tourne-suite-$Build" }
    $principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    if (-not $WhatIfPreference -and -not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Run this as administrator (Windhawk keeps its settings under HKLM).'
    }
    if (-not $WhatIfPreference) {
        $backup = Join-Path $PSScriptRoot ("tourne-suite-$Build-before-explorer-look-" + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
        & reg.exe export ($mod -replace '^HKLM:', 'HKLM') $backup /y | Out-Null
        Write-Output "Saved b53's settings as they were: $backup"
    }
    $changed = 0
    foreach ($name in 'explorerStyle.enabled', 'explorerColors.enabled') {
        $now = (Get-ItemProperty $settings -Name $name -ErrorAction SilentlyContinue).$name
        if ("$now" -eq '1') { Write-Output "  $name already on"; continue }
        if ($PSCmdlet.ShouldProcess("$settings\$name", "set to 1 (was '$now')")) {
            Set-ItemProperty $settings -Name $name -Value 1 -Type DWord
            $changed++
        }
        Write-Output "  $name = 1 (was '$now')"
    }
    if ($changed) {
        # Windhawk hands the mod its new settings when this time moves.
        Set-ItemProperty $mod -Name SettingsChangeTime -Value ([int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds()) -Type DWord
    }
    Write-Output "Done: $changed change(s). Open a new Explorer window to see it."
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
