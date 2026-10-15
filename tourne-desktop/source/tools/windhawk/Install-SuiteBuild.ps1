# Installs a suite build into Windhawk in place of the one running, with nothing left to do by hand: it compiles
# builds\tourne-suite-<Build>.wh.cpp with Windhawk's own command line (windhawk-cli), gives it the settings the
# build script wrote beside it (the running build's), the running build's process exclusions (Tourne Desktop
# stays out), then turns the old build off and the new one on. Needs administrator rights (Windhawk keeps its
# mods under HKLM and ProgramData). Every mod's settings as they were are exported beside this script first.
#
#   .\tools\windhawk\Install-SuiteBuild.ps1 -Build b54 -Replace b53 -Log <file>
[CmdletBinding()]
param([Parameter(Mandatory)] [string]$Build, [Parameter(Mandatory)] [string]$Replace, [string]$Log = '')
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    $principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { throw 'Run this as administrator.' }
    $repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
    $source = Join-Path $repo "builds\tourne-suite-$Build.wh.cpp"
    $settings = Join-Path $repo "builds\tourne-suite-$Build-settings.reg"
    $cli = Join-Path $env:ProgramFiles 'Windhawk\windhawk-cli.exe'
    $new = "local@tourne-suite-$Build"
    $old = "local@tourne-suite-$Replace"
    foreach ($f in $source, $settings, $cli) { if (-not (Test-Path $f)) { throw "Missing: $f" } }
    function Invoke-WindhawkCli {  # (not "Cli": that is Clear-Item's alias)
    & $cli @args; if ($LASTEXITCODE) { throw "windhawk-cli $($args -join ' ') failed ($LASTEXITCODE)" } }

    $backup = Join-Path $PSScriptRoot ("mods-before-$Build-" + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
    & reg.exe export 'HKLM\SOFTWARE\Windhawk\Engine\Mods' $backup /y | Out-Null
    "Saved every mod's settings as they were: $backup"

    # The running build's exclusions, carried over (Tourne Desktop and Riot Vanguard stay out).
    $oldKey = "HKLM:\SOFTWARE\Windhawk\Engine\Mods\$old"
    $exclude = @(("$((Get-ItemProperty $oldKey -Name Exclude -ErrorAction SilentlyContinue).Exclude)" -split '\|') | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    if ($exclude -notcontains 'TourneDesktop.exe') { $exclude += 'TourneDesktop.exe' }
    "Exclusions: $($exclude -join ', ')"

    "Compiling and installing $new (off until its settings are in)..."
    Invoke-WindhawkCli mod install --file $source --disabled --yes
    "Settings from $Replace..."
    & reg.exe import $settings
    if ($LASTEXITCODE) { throw "reg import $settings failed" }
    # (a mod's own Exclude is its source's @exclude, rewritten at every compile: the rest go in its custom list)
    $own = @((Select-String -Path $source -Pattern '^// @exclude\s+(\S+)' | ForEach-Object { $_.Matches[0].Groups[1].Value }))
    $custom = @($exclude | Where-Object { $own -notcontains $_ })
    if ($custom.Count) { Invoke-WindhawkCli mod config set $new excludeCustom @custom }
    "Switching $Replace off and $Build on..."
    Invoke-WindhawkCli mod disable $old
    Invoke-WindhawkCli mod enable $new
    Invoke-WindhawkCli mod config get $new
    "Done: $Build is on, $Replace is off (kept installed, to go back: enable it and disable $Build)."
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
