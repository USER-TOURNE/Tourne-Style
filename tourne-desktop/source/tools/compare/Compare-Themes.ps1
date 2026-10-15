<#
Compares our taskbar with StartAllBack's for each theme, with StartAllBack's
taskbar settings at THEIR DEFAULTS (the values are removed, so it uses its own
defaults) and ours at ours, so the result shows the themes and not your tweaks.

Your StartIsBack key is saved first (out\startisback-backup.reg) and put back
at the end, also when something fails. StartAllBack doesn't read them live, so Explorer is restarted (Windows starts it again by itself)
after each change, and once more at the end.
#>
param([string[]]$Themes = @('Bouquet SAB.msstyles', 'Plain8.msstyles', 'Windows 7.msstyles'), [string]$Out = "$PSScriptRoot\out-themes")
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\RealReg.ps1"
New-Item -ItemType Directory -Force $Out | Out-Null
function Restart-Explorer {
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 3
    $deadline = (Get-Date).AddSeconds(30)
    while ((Get-Date) -lt $deadline -and -not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Sleep -Milliseconds 500 }
    if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe }
    Start-Sleep -Seconds 8  # the taskbar, StartAllBack and the suite settle
}
$key = 'HKCU:\Software\StartIsBack'
$backup = Join-Path $Out 'startisback-backup.reg'
Export-RealSab $backup
"Saved $backup"
$defaults = 'TaskbarOneSegment', 'TaskbarTranslucentEffect', 'TaskbarCenterIcons', 'TaskbarSpacierIcons', 'TaskbarLargerIcons',
            'TaskbarGrouping', 'TaskbarBlur', 'TaskbarColor', 'TaskbarAlpha', 'TaskbarColoring', 'FatTaskbar', 'OrbBitmap',
            'TaskbarControlCenter', 'TaskbarPosition', 'TaskbarShy'
try {
    Remove-RealSabValue $defaults
    foreach ($theme in $Themes) {
        Set-RealSabString 'TaskbarStyle' $theme
        Restart-Explorer
        $safe = $theme -replace '\.msstyles$', '' -replace '\W', '_'
        "=== $theme"
        & "$PSScriptRoot\Compare.ps1" -Segments 'none' -Out "$Out\$safe" | Where-Object { $_ -notmatch '^GEOM' }
    }
} finally {
    Import-RealSab $backup
    Restart-Explorer
    "Restored your StartAllBack settings from the backup."
}
