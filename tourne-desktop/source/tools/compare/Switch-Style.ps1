<#
Sets StartAllBack's taskbar style in the real registry ('Default' removes it), kills every Explorer, waits until
the taskbar is back and steady. Backs up the real key once to out\startisback-before-switch.reg first (only if
that file doesn't exist yet). -Restore puts that backup back and restarts Explorer.
#>
param([string]$Style = 'Default', [switch]$Restore)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\RealReg.ps1"
$null = [Drawing.Image]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime\.dll|System\.Collections\.dll|System\.Threading|System\.Memory|netstandard|System\.Private\.CoreLib' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Grab.cs"))

function Restart-ExplorerFull {
    $old = @(Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object Id)
    for ($try = 0; $try -lt 10; $try++) {
        Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 700
        if (-not @(Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $old -contains $_.Id })) { break }
    }
    Start-Sleep -Seconds 3
    $deadline = (Get-Date).AddSeconds(60); $steady = 0; $last = ''
    while ((Get-Date) -lt $deadline -and $steady -lt 5) {
        Start-Sleep -Seconds 1
        $p = [Grab]::Probe()
        if ($p -match 'task=1' -and $p -match 'vis=1' -and $p -notmatch 'tray=0' -and $p -eq $last) { $steady++ } else { $steady = 0 }
        $last = $p
    }
    if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe; Start-Sleep -Seconds 10 }
    Start-Sleep -Seconds 6
    "explorer ready ($last)"
}

New-Item -ItemType Directory -Force "$PSScriptRoot\out" | Out-Null
$backup = "$PSScriptRoot\out\startisback-before-switch.reg"
if ($Restore) {
    Import-RealSab $backup
    Restart-ExplorerFull
    'restored your StartAllBack settings'
    return
}
if (-not (Test-Path $backup)) { Export-RealSab $backup }
if ($Style -eq 'Default') { Remove-RealSabValue @('TaskbarStyle') } else { Set-RealSabString 'TaskbarStyle' $Style }
Restart-ExplorerFull
$v = Get-RealSab
"TaskbarStyle now: '$($v.TaskbarStyle)'"
