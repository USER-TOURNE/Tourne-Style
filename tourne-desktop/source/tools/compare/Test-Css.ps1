<#
Builds the test bar from the current source (the "// @inline" files put in) and runs it with the given TB_* settings,
for the YASB theme work. Prints what the bar says and converts the captures it made to png.
  .\Test-Css.ps1 -Vars @{ TB_yasbTheme = 'Everforest'; TB_popup = 'calendar' }
Never sets the pointer: the bar is worked with messages.
#>
param(
    [hashtable]$Vars = @{},
    [switch]$NoBuild,
    [string]$Filter = ''
)
$branch = Split-Path (Split-Path $PSScriptRoot)
$Out = "$PSScriptRoot\out-css"
New-Item -ItemType Directory -Force $Out | Out-Null
if (-not $NoBuild) {
    $hostSrc = ([IO.File]::ReadAllText("$branch\taskbar-host.cpp") -replace "`r`n", "`n")
    $hostSrc = [regex]::Replace($hostSrc, '(?m)^// @inline (\S+)$', { param($m) ([IO.File]::ReadAllText("$branch\$($m.Groups[1].Value)") -replace "`r`n", "`n").TrimEnd("`n") })
    [IO.File]::WriteAllText("$Out\taskbar-inlined.cpp", $hostSrc, (New-Object Text.UTF8Encoding $false))
    $log = & 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe' --target=x86_64-w64-mingw32 -std=c++23 -municode -O1 -DUNICODE -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008 -I'C:\Program Files\Windhawk\Compiler\include' -I $Out "$PSScriptRoot\bartest.cpp" -o "$Out\bartest.exe" -static -luxtheme -ldwmapi -lole32 -lshell32 -lgdi32 -lmsimg32 -luser32 -luuid -lwindowscodecs -lcomctl32 -lpropsys -lshlwapi -loleaut32 -ladvapi32 -lwinmm -lversion -lwtsapi32 -lpsapi -lnetapi32 -limm32 -lurlmon -lwininet -liphlpapi -lwlanapi -lpowrprof 2>&1
    $log | Where-Object { $_ -match 'error|undefined' } | Select-Object -First 15
}
$defaults = @{ TB_yasbFill = '1'; TB_yasbFades = '1'; TB_location = 'top'; TB_style = 'none'; TB_orb = 'none'; TB_tray = 'hidden'; TB_scheme = 'tourne' }
foreach ($k in $defaults.Keys) { if (-not $Vars.ContainsKey($k)) { $Vars[$k] = $defaults[$k] } }
Push-Location $Out; [Environment]::CurrentDirectory = $Out
foreach ($k in $Vars.Keys) { Set-Item "env:$k" $Vars[$k] }
foreach ($f in Get-ChildItem $Out -Filter *.bmp -ErrorAction SilentlyContinue) { $f.Delete() }
Start-Process -FilePath "$Out\bartest.exe" -ArgumentList 'bar.bmp' -NoNewWindow -Wait -RedirectStandardOutput "$Out\run.txt" | Out-Null
foreach ($k in $Vars.Keys) { Set-Item "env:$k" $null }
Pop-Location; [Environment]::CurrentDirectory = (Get-Location).Path
Add-Type -AssemblyName System.Drawing
foreach ($f in Get-ChildItem $Out -Filter *.bmp) {
    try { $b = [Drawing.Image]::FromFile($f.FullName); $b.Save(($f.FullName -replace '\.bmp$', '.png'), [Drawing.Imaging.ImageFormat]::Png); $b.Dispose() } catch {}
}
$lines = Get-Content "$Out\run.txt"
if ($Filter) { $lines = $lines | Where-Object { $_ -match $Filter } } else { $lines = $lines | Where-Object { $_ -notmatch '^(GEOM|button|option|>)' } | Select-Object -Last 30 }
$lines
