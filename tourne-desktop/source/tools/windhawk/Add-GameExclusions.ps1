# Adds Riot's client and game programs to Windhawk's own exclusions (Settings, Advanced, Process exclusion
# list), beside Vanguard (vgc.exe) that's there already, so no mod loads into League or Valorant. Tourne
# Desktop reads this list too (shell.excludeWindhawk). Needs administrator rights (Windhawk keeps its
# settings under HKLM); the settings as they were are exported beside this script first (*.reg here is
# ignored by git).
#
#   .\tools\windhawk\Add-GameExclusions.ps1 -WhatIf      # say what would change
#   .\tools\windhawk\Add-GameExclusions.ps1 -Log <file>  # do it, writing what changed to a file too
[CmdletBinding(SupportsShouldProcess)]
param([string]$Log = '', [string[]]$Programs = @(
    'League of Legends.exe', 'LeagueClient.exe', 'LeagueClientUx.exe', 'LeagueClientUxRender.exe',
    'RiotClientServices.exe', 'RiotClientUx.exe', 'RiotClientUxRender.exe', 'RiotClientCrashHandler.exe',
    'VALORANT.exe', 'VALORANT-Win64-Shipping.exe', 'vgtray.exe'))
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    $key = 'HKLM:\SOFTWARE\Windhawk\Engine\Settings'
    $principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    if (-not $WhatIfPreference -and -not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Run this as administrator (Windhawk keeps its settings under HKLM).'
    }
    $now = "$((Get-ItemProperty $key -Name Exclude -ErrorAction SilentlyContinue).Exclude)"
    $entries = [System.Collections.Generic.List[string]]::new()
    foreach ($e in ($now -split '\|')) { if ($e.Trim()) { $entries.Add($e.Trim()) } }
    $added = @()
    foreach ($p in $Programs) {
        if (-not ($entries | Where-Object { $_ -ieq $p -or $_ -like "*\$p" })) { $entries.Add($p); $added += $p }
    }
    if (-not $added) { Write-Output 'Nothing to add: every one is excluded already.'; return }
    Write-Output "Adding to Windhawk's exclusions: $($added -join ', ')"
    if ($PSCmdlet.ShouldProcess("$key\Exclude", 'add the programs above')) {
        $backup = Join-Path $PSScriptRoot ('engine-settings-before-game-exclusions-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
        & reg.exe export 'HKLM\SOFTWARE\Windhawk\Engine\Settings' $backup /y | Out-Null
        Write-Output "Saved Windhawk's settings as they were: $backup"
        Set-ItemProperty $key -Name Exclude -Value ($entries -join '|') -Type String
        Restart-Service Windhawk
        Write-Output 'Windhawk restarted with the new exclusions.'
    }
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
