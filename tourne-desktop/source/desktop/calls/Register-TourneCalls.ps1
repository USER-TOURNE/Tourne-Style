# Builds Tourne Calls, puts it in %LOCALAPPDATA%\Tourne\Desktop\calls and registers it as a local package there
# (Developer Mode must be on), so it has the identity Windows asks of a program driving a phone's calls.
# Run as yourself, not elevated. -Remove takes the package away again (the files stay).
#
#   .\desktop\calls\Register-TourneCalls.ps1 [-Remove]
[CmdletBinding()]
param([switch]$Remove)
$ErrorActionPreference = 'Stop'
$name = 'Tourne.Calls'
if ($Remove) {
    Get-AppxPackage $name | Remove-AppxPackage
    "Removed $name."
    return
}
$unlock = Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock' -ErrorAction SilentlyContinue
if ($unlock.AllowDevelopmentWithoutDevLicense -ne 1) { throw 'Developer Mode is off (Settings, System, For developers).' }

$source = $PSScriptRoot
$target = Join-Path $env:LOCALAPPDATA 'Tourne\Desktop\calls'
New-Item -ItemType Directory -Force (Join-Path $target 'Assets') | Out-Null
$cc = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
$exe = Join-Path $target 'TourneCalls.exe'
Get-Process TourneCalls -ErrorAction SilentlyContinue | Stop-Process -Force
& $cc --target=x86_64-w64-mingw32 -std=c++20 -municode -mwindows -static -O2 (Join-Path $source 'TourneCalls.cpp') -o $exe `
    -lole32 -loleaut32 -lruntimeobject -lshell32 -luuid -ldwmapi
if ($LASTEXITCODE) { throw 'Compiling TourneCalls.cpp failed' }
Copy-Item (Join-Path $source 'TourneCalls.exe.manifest'), (Join-Path $source 'AppxManifest.xml') $target -Force

# Its logo (a package must have one): a 150 px square in Tourne's accent.
Add-Type -AssemblyName System.Drawing
$logo = New-Object Drawing.Bitmap 150, 150
$g = [Drawing.Graphics]::FromImage($logo)
$g.Clear([Drawing.Color]::FromArgb(0, 0, 0, 0))
$g.FillEllipse((New-Object Drawing.SolidBrush ([Drawing.Color]::FromArgb(255, 232, 128, 136))), 15, 15, 120, 120)
$logo.Save((Join-Path $target 'Assets\logo.png'), [Drawing.Imaging.ImageFormat]::Png)

Get-AppxPackage $name | Remove-AppxPackage
Add-AppxPackage -Register (Join-Path $target 'AppxManifest.xml') -ExternalLocation $target
Get-AppxPackage $name | Select-Object Name, PackageFullName, InstallLocation | Format-List
