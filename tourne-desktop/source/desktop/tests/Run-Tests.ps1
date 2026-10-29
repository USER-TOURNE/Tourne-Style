# Builds and runs Tourne Desktop's console tests (tests.cpp, doctest) with the same compiler as the program.
#   .\desktop\tests\Run-Tests.ps1            # all of them
#   .\desktop\tests\Run-Tests.ps1 -Pass '-tc=css*'   # doctest's own switches pass through
param([string]$Compiler = '', [string]$Pass = '')
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$hostDir = Join-Path $here '..\host'
if (-not $Compiler) {
    $Compiler = @("$env:ProgramFiles\Windhawk\Compiler\bin\clang++.exe", 'clang++', 'g++') |
        Where-Object { (Get-Command $_ -ErrorAction SilentlyContinue) } | Select-Object -First 1
    if (-not $Compiler) { throw 'No compiler: install Windhawk or pass -Compiler.' }
}
$target = @()  # (an if's one-item array comes back as a string, which splats as letters)
if ([IO.Path]::GetFileNameWithoutExtension($Compiler) -match 'clang') { $target = @('--target=x86_64-w64-mingw32') }
$out = Join-Path $env:TEMP 'tourne-tests'
New-Item -ItemType Directory -Force $out | Out-Null
$exe = Join-Path $out 'tourne-tests.exe'
$sources = @(Join-Path $here 'tests.cpp')
foreach ($name in 'json-lite.cpp', 'toml-lite.cpp', 'palette-gen.cpp', 'css-gen.cpp', 'launcher-math.cpp') { $sources += Join-Path $hostDir $name }
& $Compiler @target -std=c++20 -O1 -static -DUNICODE -D_UNICODE @sources -o $exe
if ($LASTEXITCODE) { throw "build failed ($LASTEXITCODE)" }
& $exe $Pass.Split(' ', [StringSplitOptions]::RemoveEmptyEntries)
exit $LASTEXITCODE
