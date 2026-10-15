# The desktop's "Restart Explorer" (Winaero Tweaker's, in HKLM) as one command instead of a submenu.
# It only ends explorer.exe: Tourne Desktop starts Explorer underneath again by itself in shell mode, and
# Windows restarts its shell by itself otherwise (AutoRestartShell). Needs administrator rights.
# The key as it was is exported beside this script first (*.reg here is ignored by git).
param([string]$Log = '')
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    $key = 'HKLM\SOFTWARE\Classes\DesktopBackground\Shell\RestartExplorer'
    $backup = Join-Path $PSScriptRoot ('restart-explorer-before-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
    & reg.exe export $key $backup /y | Out-Null
    Write-Output "Saved as it was: $backup"
    & reg.exe delete "$key\shell" /f | Out-Null
    & reg.exe delete $key /v SubCommands /f | Out-Null
    & reg.exe add $key /v MUIVerb /t REG_SZ /d 'Restart Explorer' /f | Out-Null
    & reg.exe add "$key\command" /ve /t REG_SZ /d 'taskkill.exe /f /im explorer.exe' /f | Out-Null
    & reg.exe query $key /s
    Write-Output 'Done.'
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
