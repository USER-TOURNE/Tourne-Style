<#
Builds tools\plugins\pluginrun.cpp with the plugin runtime and runs the plugins
named, installed into a scratch folder (never Tourne's own), as a bar sees them.
Needs a build of Tourne Desktop first (for desktop\build\gen\plugins-index.inc).

    .\tools\plugins\Run-PluginTest.ps1 noctalia/world_clock noctalia/example
    .\tools\plugins\Run-PluginTest.ps1 noctalia/world_clock -Settings 'plugins.noctalia/world_clock.show_clocks=1'
    .\tools\plugins\Run-PluginTest.ps1 -Sandbox    # also sandbox-test: a plugin granted nothing that tries
                                                   # everything it mustn't (each should be refused or stopped)
#>
[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(ValueFromRemainingArguments)] [string[]]$Plugins = @('noctalia/world_clock'),
    [string]$Settings = '',
    [switch]$Sandbox,
    [string]$Scratch = (Join-Path ([IO.Path]::GetTempPath()) 'tourne-pluginrun')
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$desktop = Join-Path $repo 'desktop'
$gen = Join-Path $desktop 'build\gen'
if (-not (Test-Path (Join-Path $gen 'plugins-bundled.inc'))) { throw 'Build Tourne Desktop first (desktop\Build-TourneDesktop.ps1).' }
$cxx = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
if (-not (Test-Path $cxx)) { $cxx = (Get-Command clang++).Source }
$target = @('--target=x86_64-w64-mingw32')
$luauObjects = & (Join-Path $desktop 'Build-Luau.ps1') -Compiler $cxx -Out $gen -Target $target
$luau = Join-Path $desktop 'third_party\luau'
$flags = @('-std=c++23', '-O1', '-DUNICODE', '-D_UNICODE', '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00', '-municode', '-static', "-I$desktop\host", "-I$gen", "-I$repo",
           "-I$luau\VM\include", "-I$luau\Compiler\include", "-I$luau\Common\include") + $target
$exe = Join-Path $gen 'pluginrun.exe'
& $cxx @flags (Join-Path $PSScriptRoot 'pluginrun.cpp') "$desktop\host\plugin-runtime.cpp" "$desktop\host\plugins.cpp" `
    "$desktop\host\json-lite.cpp" "$desktop\host\toml-lite.cpp" "$desktop\host\plugin-panel.cpp" "$desktop\host\ui-sounds.cpp" @luauObjects `
    -lwinhttp -lole32 -lshell32 -luuid -lgdi32 -luser32 -ldwmapi -lcomctl32 -luxtheme -lwindowscodecs -lmsimg32 -lsetupapi -lwinmm -liphlpapi -lpowrprof -lws2_32 -lshlwapi -ld2d1 -ldxgi -lcfgmgr32 -o $exe
if ($LASTEXITCODE) { throw 'Building pluginrun failed' }
$sandboxDir = Join-Path $Scratch 'test\sandbox'
Remove-Item -Recurse -Force $sandboxDir -ErrorAction SilentlyContinue
if ($Sandbox) {
    New-Item -ItemType Directory -Force $sandboxDir | Out-Null
    Copy-Item (Join-Path $PSScriptRoot 'sandbox-test\*') $sandboxDir
}
# Only the plugins named run: ones installed by earlier runs are taken out of the scratch folder first (an old
# Test Notifications kept sending real toasts from every later run).
Get-ChildItem $Scratch -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -notin '.data', 'test' } | ForEach-Object {
    $author = $_.Name
    Get-ChildItem $_.FullName -Directory | Where-Object { "$author/$($_.Name)" -notin $Plugins } |
        ForEach-Object { Remove-Item -Recurse -Force $_.FullName }
}
$env:TOURNE_PLUGINS_DIR = $Scratch
$env:TOURNE_PLUGIN_SETTINGS = $Settings
& $exe @Plugins
