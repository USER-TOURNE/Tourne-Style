# Turns off the Windhawk mods that overlap Tourne Desktop or b53 (2026-10-08): Tourne Desktop's menus were
# styled three times over (b53's Classic Menus and Text Color, classic-menus, dark-menus), giving double
# frames and menus opening out of place. Needs administrator rights (Windhawk keeps its settings under HKLM).
# Every mod's settings as they were are exported beside this script first (*.reg here is ignored by git).
#
#   .\tools\windhawk\Set-NoMenuOverlaps.ps1 -WhatIf     # say what would change
#   .\tools\windhawk\Set-NoMenuOverlaps.ps1 -Log <file> # do it, writing what changed to a file too
[CmdletBinding(SupportsShouldProcess)]
param([string]$Build = 'b53', [string]$Log = '')
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    $mods = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
    $principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    if (-not $WhatIfPreference -and -not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Run this as administrator (Windhawk keeps its settings under HKLM).'
    }
    if (-not $WhatIfPreference) {
        $backup = Join-Path $PSScriptRoot ('mods-before-no-menu-overlaps-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
        & reg.exe export 'HKLM\SOFTWARE\Windhawk\Engine\Mods' $backup /y | Out-Null
        Write-Output "Saved every mod's settings as they were: $backup"
    }
    $changed = 0
    function Set-Value([string]$Key, [string]$Name, $Value, [string]$Type) {
        if (-not (Test-Path $Key)) { Write-Output "  (not installed: $(Split-Path $Key -Leaf))"; return }
        $now = (Get-ItemProperty $Key -Name $Name -ErrorAction SilentlyContinue).$Name
        if ("$now" -eq "$Value") { Write-Output "  $(Split-Path $Key -Leaf)\$Name already '$Value'"; return }
        if ($PSCmdlet.ShouldProcess("$Key\$Name", "set to '$Value' (was '$now')")) {
            Set-ItemProperty $Key -Name $Name -Value $Value -Type $Type
            Set-ItemProperty $Key -Name SettingsChangeTime -Value ([int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds()) -Type DWord
        }
        Write-Output "  $(Split-Path $Key -Leaf)\$Name = $Value (was '$now')"
        $script:changed++
    }

    # b53 stays out of Tourne Desktop, which styles its own menus (and Riot Vanguard, as before).
    $suite = "$mods\local@tourne-suite-$Build"
    $exclude = "$((Get-ItemProperty $suite -Name Exclude -ErrorAction SilentlyContinue).Exclude)"
    $entries = @($exclude -split '\|' | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    if ($entries -notcontains 'TourneDesktop.exe') { $entries += 'TourneDesktop.exe' }
    Set-Value $suite 'Exclude' ($entries -join '|') 'String'

    # Mods whose job b53 or Tourne Desktop does now.
    foreach ($mod in 'classic-menus', 'dark-menus', 'eradicate-immersive-menus', 'desktop-marquee-fade',
                     'local@taskbar-thumbnail-hide-icon') {
        Set-Value "$mods\$mod" 'Disabled' 1 'DWord'
    }

    if ($changed -and $PSCmdlet.ShouldProcess('Windhawk service', 'restart')) {
        Restart-Service Windhawk
        Write-Output 'Windhawk restarted: every program takes the changes in.'
    }
    Write-Output "Done: $changed change(s)."
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
