<#
Builds tools\layouts\layouttest.cpp with Luau and runs it: the layout sandbox
and the examples, checked without a screen. Needs no build of Tourne Desktop.

    .\tools\layouts\Run-LayoutTest.ps1                     # the checks
    .\tools\layouts\Run-LayoutTest.ps1 my-layout.luau       # that file, laid out and printed
    .\tools\layouts\Run-LayoutTest.ps1 my-layout.luau "3"   # with the options a setting would give it

A file name with no folder in it is looked for in %LOCALAPPDATA%\Tourne\Layouts.
#>
[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(Position = 0)] [string]$File = '',
    [Parameter(Position = 1)] [string]$Options = ''
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$desktop = Join-Path $repo 'desktop'
$gen = Join-Path $desktop 'build\gen'
New-Item -ItemType Directory -Force $gen | Out-Null
$cxx = 'C:\Program Files\Windhawk\Compiler\bin\clang++.exe'
if (-not (Test-Path $cxx)) { $cxx = (Get-Command clang++).Source }
$target = @('--target=x86_64-w64-mingw32')
$luauObjects = & (Join-Path $desktop 'Build-Luau.ps1') -Compiler $cxx -Out $gen -Target $target
$luau = Join-Path $desktop 'third_party\luau'
$flags = @('-std=c++23', '-O1', '-static', "-I$luau\VM\include", "-I$luau\Compiler\include",
           "-I$luau\Common\include") + $target
$exe = Join-Path $gen 'layouttest.exe'
& $cxx @flags (Join-Path $PSScriptRoot 'layouttest.cpp') @luauObjects -o $exe
if ($LASTEXITCODE) { throw 'Building layouttest failed' }

if (-not $File) {
    & $exe
    exit $LASTEXITCODE
}
$path = $File
if (-not (Split-Path $path -Parent)) {
    if (-not $path.EndsWith('.luau')) { $path += '.luau' }
    $path = Join-Path (Join-Path $env:LOCALAPPDATA 'Tourne\Layouts') $path
}
if (-not (Test-Path -LiteralPath $path)) { throw "No layout at $path" }
& $exe $path $Options
exit $LASTEXITCODE
