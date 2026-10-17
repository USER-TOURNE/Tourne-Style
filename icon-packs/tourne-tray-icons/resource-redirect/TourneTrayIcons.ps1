<#
Tourne Tray Icons: applies the Tourne pixel tray icons through Windhawk's
Resource Redirect mod, and takes them back off.

  -Action Apply    Adds the icon packages for -Tone (sage, ink, alt, or ink-flat:
                   ink without the pixel shade) as the
                   last, highest-priority entries of Resource Redirect's
                   "Redirection resource paths". Running it again with another
                   tone swaps the tone.
  -Action Revert   Removes exactly the entries Apply added. Everything else in
                   the mod's settings is left as it was.
  -Action Restore  Puts the whole Resource Redirect settings key back to the
                   backup taken before the very first Apply.

Every run first exports the current settings to backups\. Windhawk keeps mod
settings under HKLM, so the registry step asks for administrator rights once.
Explorer and the Windows Security tray icon are restarted afterwards so the new
icons are loaded; pass -NoRestart to skip that.
#>
param(
    [ValidateSet('Apply', 'Revert', 'Restore')]
    [string]$Action = 'Apply',
    [ValidateSet('sage', 'ink', 'alt', 'ink-flat')]
    [string]$Tone = 'sage',
    [switch]$NoRestart,
    [switch]$RegistryOnly
)

$ErrorActionPreference = 'Stop'

$hive = [Microsoft.Win32.Registry]::LocalMachine
$hiveName = 'HKLM'
$modSubKey = 'SOFTWARE\Windhawk\Engine\Mods\icon-resource-redirect'
$settingsKeyReg = "$hiveName\$modSubKey\Settings"
$packageRoot = Join-Path $PSScriptRoot 'packages'
$backupDir = Join-Path $PSScriptRoot 'backups'
$firstBackup = Join-Path $backupDir 'before-first-apply.reg'

function Test-Admin {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $identity).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
}

# The .NET registry API is used throughout because the registry cmdlets read
# the brackets in "redirectionResourcePaths[0].original" as a wildcard.
function Open-Key([string]$subKey, [bool]$writable) {
    $key = $hive.OpenSubKey($subKey, $writable)
    if (-not $key) { throw "Resource Redirect (icon-resource-redirect) isn't installed in Windhawk." }
    $key
}

# reg.exe reports success on stderr, which Windows PowerShell treats as an error.
function Invoke-Reg([string[]]$arguments) {
    $out = [IO.Path]::GetTempFileName()
    $err = [IO.Path]::GetTempFileName()
    try {
        $p = Start-Process -FilePath reg.exe -ArgumentList $arguments -NoNewWindow -Wait -PassThru `
            -RedirectStandardOutput $out -RedirectStandardError $err
        $p.ExitCode
    } finally {
        Remove-Item -LiteralPath $out, $err -ErrorAction SilentlyContinue
    }
}

function Get-RedirectEntries {
    $key = Open-Key "$modSubKey\Settings" $false
    $entries = @{}
    foreach ($name in $key.GetValueNames()) {
        $m = [regex]::Match($name, '^redirectionResourcePaths\[(\d+)\]\.(original|redirect)$')
        if (-not $m.Success) { continue }
        $index = [int]$m.Groups[1].Value
        if (-not $entries.ContainsKey($index)) {
            $entries[$index] = [pscustomobject]@{ original = ''; redirect = '' }
        }
        $entries[$index].($m.Groups[2].Value) = [string]$key.GetValue($name)
    }
    $key.Close()
    $entries.Keys | Sort-Object | ForEach-Object { $entries[$_] }
}

function Set-RedirectEntries($entries) {
    $key = Open-Key "$modSubKey\Settings" $true
    foreach ($name in $key.GetValueNames()) {
        if ($name.StartsWith('redirectionResourcePaths[')) {
            $key.DeleteValue($name)
        }
    }
    $i = 0
    foreach ($e in $entries) {
        $key.SetValue("redirectionResourcePaths[$i].original", [string]$e.original, [Microsoft.Win32.RegistryValueKind]::String)
        $key.SetValue("redirectionResourcePaths[$i].redirect", [string]$e.redirect, [Microsoft.Win32.RegistryValueKind]::String)
        $i++
    }
    $key.Close()
}

function Test-OurEntry($entry) {
    $entry.redirect -and $entry.redirect.StartsWith($packageRoot, [StringComparison]::OrdinalIgnoreCase)
}

# Tells running Windhawk engines to reload the mod's settings.
function Update-ChangeTime {
    $now = [int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds()
    $key = Open-Key $modSubKey $true
    $key.SetValue('SettingsChangeTime', $now, [Microsoft.Win32.RegistryValueKind]::DWord)
    $key.Close()
}

function Invoke-RegistryStep {
    (Open-Key "$modSubKey\Settings" $false).Close()

    New-Item -ItemType Directory -Force $backupDir | Out-Null
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    if ((Invoke-Reg @('export', "`"$settingsKeyReg`"", "`"$(Join-Path $backupDir "settings-$stamp.reg")`"", '/y')) -ne 0) {
        throw 'Backing up the current settings failed, so nothing was changed.'
    }
    if (-not (Test-Path $firstBackup)) {
        Invoke-Reg @('export', "`"$settingsKeyReg`"", "`"$firstBackup`"", '/y') | Out-Null
    }

    switch ($Action) {
        'Apply' {
            $toneDir = Join-Path $packageRoot $Tone
            $kept = @(Get-RedirectEntries | Where-Object { -not (Test-OurEntry $_) })
            $ours = @()
            $modules = Import-Csv (Join-Path $PSScriptRoot 'mapping.csv') | Select-Object -ExpandProperty module -Unique
            foreach ($module in $modules) {
                $package = Join-Path $toneDir $module
                if (-not (Test-Path $package)) { throw "Missing package $package" }
                foreach ($dir in 'System32', 'SysWOW64') {
                    $original = Join-Path $env:SystemRoot "$dir\$module"
                    if (Test-Path $original) {
                        $ours += [pscustomobject]@{ original = $original; redirect = $package }
                    }
                }
            }
            # Last entries win, so the icons override theme packs and the
            # existing per-file redirects for the same modules.
            Set-RedirectEntries ($kept + $ours)
            Update-ChangeTime
            Write-Host "Applied the $Tone tray icons: $($ours.Count) redirects added after your $($kept.Count) existing ones."
        }
        'Revert' {
            $all = @(Get-RedirectEntries)
            $kept = @($all | Where-Object { -not (Test-OurEntry $_) })
            Set-RedirectEntries $kept
            Update-ChangeTime
            Write-Host "Reverted: removed $($all.Count - $kept.Count) Tourne redirects, kept your $($kept.Count) others."
        }
        'Restore' {
            if (-not (Test-Path $firstBackup)) { throw "No backup from before the first Apply was found." }
            $hive.DeleteSubKeyTree("$modSubKey\Settings")
            if ((Invoke-Reg @('import', "`"$firstBackup`"")) -ne 0) {
                throw "reg import failed; the settings export from this run is in $backupDir"
            }
            Update-ChangeTime
            Write-Host "Restored the Resource Redirect settings from $firstBackup."
        }
    }

    $modKey = Open-Key $modSubKey $false
    $disabled = $modKey.GetValue('Disabled', 0)
    $modKey.Close()
    if ($disabled) {
        Write-Warning "Resource Redirect is disabled in Windhawk, so nothing will change until it's enabled."
    }
}

function Restart-IconHosts {
    Write-Host 'Restarting Explorer and the Windows Security tray icon to load the icons...'
    $systray = Join-Path $env:SystemRoot 'System32\SecurityHealthSystray.exe'
    # Windows can refuse this one; its icon then updates at the next sign-in.
    try {
        Get-Process SecurityHealthSystray -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction Stop
    } catch {
        Write-Host "Windows Security's tray icon couldn't be restarted; it updates at the next sign-in."
    }
    Get-Process explorer -ErrorAction SilentlyContinue | Stop-Process -Force
    # Windows restarts the shell on its own; start it only if it didn't.
    Start-Sleep -Seconds 3
    if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe }
    if (Test-Path $systray) { Start-Process $systray }
}

$logFile = Join-Path $backupDir 'last-run.log'

if ($RegistryOnly) {
    # Runs in its own elevated window, which closes at once, so the outcome
    # goes to a log the unelevated side reads back.
    New-Item -ItemType Directory -Force $backupDir | Out-Null
    try {
        Invoke-RegistryStep *>&1 | Out-File -FilePath $logFile -Encoding utf8
        exit 0
    } catch {
        "ERROR: $($_.Exception.Message)" | Out-File -FilePath $logFile -Encoding utf8 -Append
        exit 1
    }
}

if (Test-Admin) {
    Invoke-RegistryStep
} else {
    # Only the registry step runs elevated, so Explorer comes back unelevated.
    $childArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"",
                   '-Action', $Action, '-Tone', $Tone, '-RegistryOnly')
    try {
        $child = Start-Process -FilePath (Get-Process -Id $PID).Path -ArgumentList $childArgs -Verb RunAs -Wait -PassThru
    } catch {
        Write-Error 'Administrator rights were declined. Nothing was changed.'
        exit 1
    }
    if (Test-Path $logFile) { Get-Content $logFile | Write-Host }
    if ($child.ExitCode -ne 0) {
        Write-Error "The registry step failed. Nothing else was changed."
        exit 1
    }
}

if (-not $NoRestart) {
    Restart-IconHosts
}
