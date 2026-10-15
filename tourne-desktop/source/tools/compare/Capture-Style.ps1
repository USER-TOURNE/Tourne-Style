<#
Switches StartAllBack's taskbar style (real registry), kills Explorer, clears the caches, waits for it to rebuild, and
photographs the whole screen (StartAllBack's bar at the bottom, ours at the top): idle, a hovered button on each bar, and each Start
menu open (idle and with an item hovered). No computer-use tools: plain input and a full-screen capture.
Show Desktop (Win+D) is used so no windows are in the way, and used again at the end to bring them back.
#>
param([Parameter(Mandatory)][string]$Style, [string]$Out = "$PSScriptRoot\out-styles", [int]$Wait = 15, [int]$HoverX = 108, [int[]]$OpenX = @(207, 273, 339), [switch]$NoDesktop)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\RealReg.ps1"
$dir = Join-Path $Out (($Style -replace '\.msstyles$', '') -replace '\W', '_'); New-Item -ItemType Directory -Force $dir | Out-Null
$null = [Drawing.Image]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime\.dll|System\.Collections\.dll|System\.Threading|System\.Memory|netstandard|System\.Private\.CoreLib' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Grab.cs"))
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Cap.cs"))
[Cap]::Init()

# 1. The style, in the real registry; the caches cleared while Explorer is stopped.
$backup = Join-Path $Out 'startisback-backup.reg'
if (-not (Test-Path $backup)) { New-Item -ItemType Directory -Force $Out | Out-Null; Export-RealSab $backup }
if ($Style -eq 'Default') { Remove-RealSabValue @('TaskbarStyle') } else { Set-RealSabString 'TaskbarStyle' $Style }
$old = @(Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object Id)
for ($try = 0; $try -lt 10; $try++) {
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 700
    if (-not @(Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $old -contains $_.Id })) { break }
}
Start-Sleep -Seconds 2
Invoke-Real 'reg delete HKCU\Software\StartIsBack\Cache /f & del /f /q "%LOCALAPPDATA%\Microsoft\Windows\Explorer\iconcache_*.db" "%LOCALAPPDATA%\Microsoft\Windows\Explorer\thumbcache_*.db" "%LOCALAPPDATA%\IconCache.db"' | Out-Null
# 2. Explorer comes back by itself (Windows restarts the shell); wait until the taskbar is up, then the asked time.
$deadline = (Get-Date).AddSeconds(60); $steady = 0; $last = ''
while ((Get-Date) -lt $deadline -and $steady -lt 5) {
    Start-Sleep -Seconds 1
    $p = [Grab]::Probe()
    if ($p -match 'task=1' -and $p -match 'vis=1' -and $p -notmatch 'tray=0' -and $p -eq $last) { $steady++ } else { $steady = 0 }
    $last = $p
}
if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe; Start-Sleep -Seconds 10 }
Start-Sleep -Seconds $Wait
[Cap]::ThemeChanged()   # our bar follows StartAllBack's style when it reloads
Start-Sleep -Seconds 4
"rebuilt ($last); real style '$((Get-RealSab).TaskbarStyle)'"

$w = 1920; $h = 1080
if (-not $NoDesktop) { [Cap]::Chord(0x5B, 0x44); [Cap]::Wait(1500) }   # Win+D: no windows in the way
[Cap]::Move(960, 500); [Cap]::Wait(900)
[Cap]::Full("$dir\1-idle.png", $w, $h)
# 3. Hot state on each bar: StartAllBack's (bottom), ours (top).
# Hovering the pinned app (HoverX) and each open-window button (the running apps start after the four pins).
$spots = [System.Collections.ArrayList]::new()
$null = $spots.Add(@('pin', $HoverX))
foreach ($x in $OpenX) { $null = $spots.Add(@("open$x", $x)) }
foreach ($spot in $spots) {
    [Cap]::HoverAt($spot[1], 1060); [Cap]::Wait(900); [Cap]::Full("$dir\2-hover-sab-$($spot[0]).png", $w, $h)
    [Cap]::HoverAt($spot[1], 19);   [Cap]::Wait(900); [Cap]::Full("$dir\3-hover-ours-$($spot[0]).png", $w, $h)
}
[Cap]::Move(960, 500); [Cap]::Wait(500)
# 4. StartAllBack's Start menu (Win key), idle and with an item hovered.
[Cap]::Key(0x5B); [Cap]::Wait(1800); [Cap]::Full("$dir\4-start-sab.png", $w, $h)
[Cap]::HoverAt(120, 700); [Cap]::Wait(900); [Cap]::Full("$dir\5-start-sab-hover.png", $w, $h)
[Cap]::Key(0x1B); [Cap]::Wait(600); [Cap]::Key(0x1B); [Cap]::Wait(600)
# 5. Ours: click its Start button.
[Cap]::Click(30, 19); [Cap]::Wait(1800); [Cap]::Full("$dir\6-start-ours.png", $w, $h)
[Cap]::HoverAt(120, 330); [Cap]::Wait(900); [Cap]::Full("$dir\7-start-ours-hover.png", $w, $h)
[Cap]::Key(0x1B); [Cap]::Wait(600); [Cap]::Move(960, 500)
if (-not $NoDesktop) { [Cap]::Wait(500); [Cap]::Chord(0x5B, 0x44) }   # windows back
"saved to $dir"
