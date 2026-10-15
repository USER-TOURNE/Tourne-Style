<#
Sets StartAllBack's taskbar style in the real registry (through a process outside the package), kills Explorer, and waits
until it has fully reloaded. -Style 'Default' clears it; -Restore puts back the backup taken on the first call.
#>
param([string]$Style = '', [switch]$Restore, [string]$Out = "$PSScriptRoot\out-shots")
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\RealReg.ps1"
New-Item -ItemType Directory -Force $Out | Out-Null
$null = [Drawing.Image]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime\.dll|System\.Collections\.dll|System\.Threading|System\.Memory|netstandard|System\.Private\.CoreLib' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Grab.cs"))

$backup = Join-Path $Out 'startisback-backup.reg'
if (-not (Test-Path $backup)) { Export-RealSab $backup; "Saved $backup" }
if ($Restore) { Import-RealSab $backup }
elseif ($Style -eq 'Default' -or $Style -eq '') { Remove-RealSabValue @('TaskbarStyle') }
else { Set-RealSabString 'TaskbarStyle' $Style }

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
$v = Get-RealSab
"ready ($last); real TaskbarStyle='$($v.TaskbarStyle)'"
