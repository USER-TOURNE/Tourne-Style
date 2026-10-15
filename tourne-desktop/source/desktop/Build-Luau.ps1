<#
Compiles the Luau VM and compiler (desktop\third_party\luau) for Tourne
Desktop's plugin runtime, into <Out>\luau, and returns the object files.
An object newer than its source is kept, so after the first time this costs
nothing. Build-TourneDesktop.ps1 and tools\plugins\Run-PluginTest.ps1 call it.
#>
param(
    [Parameter(Mandatory)] [string]$Compiler,
    [Parameter(Mandatory)] [string]$Out,
    [string[]]$Target = @(),
    [switch]$Symbols
)
$ErrorActionPreference = 'Stop'
$luau = Join-Path $PSScriptRoot 'third_party\luau'
$objDir = Join-Path $Out 'luau'
New-Item -ItemType Directory -Force $objDir | Out-Null
$includes = foreach ($d in 'Common', 'Ast', 'Bytecode', 'Compiler', 'VM') { "-I$(Join-Path $luau "$d\include")" }
$includes += "-I$(Join-Path $luau 'VM\src')"
$flags = @('-std=c++17', '-w') + $Target + $(if ($Symbols) { @('-O0', '-g') } else { @('-O2') })

$todo = New-Object System.Collections.Generic.List[object]
$objects = foreach ($src in Get-ChildItem -Recurse -Filter *.cpp -Path (Join-Path $luau '*\src')) {
    $part = $src.Directory.Parent.Name
    $obj = Join-Path $objDir "$part-$($src.BaseName).o"
    if (-not (Test-Path $obj) -or (Get-Item $obj).LastWriteTime -lt $src.LastWriteTime) { $todo.Add(@{ Src = $src.FullName; Obj = $obj }) }
    $obj
}
if ($todo.Count) {
    "Compiling Luau ($($todo.Count) files, once)" | Write-Host
    $failed = if ($PSVersionTable.PSVersion.Major -ge 7) {
        $todo | ForEach-Object -ThrottleLimit 8 -Parallel {
            & $using:Compiler @using:flags @using:includes -c $_.Src -o $_.Obj 2>&1 | Out-Null
            if ($LASTEXITCODE) { $_.Src }
        }
    } else {  # (Windows PowerShell: one at a time)
        foreach ($t in $todo) {
            & $Compiler @flags @includes -c $t.Src -o $t.Obj 2>&1 | Out-Null
            if ($LASTEXITCODE) { $t.Src }
        }
    }
    if ($failed) { throw "Compiling Luau failed: $($failed -join ', ')" }
}
$objects
