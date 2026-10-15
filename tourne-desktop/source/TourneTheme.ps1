<#
Tourne theme controller: one place for every theming mod's settings and
on/off state.

  -Action Save    -Profile <name>   Snapshot the mods in theme-mods.txt.
  -Action Apply   -Profile <name>   Put a snapshot back. The state just
                                    before is saved as "_before-last-apply".
  -Action Undo                      Apply "_before-last-apply".
  -Action Switch                    Turn the six separate Tourne mods off and
                                    Tourne'Suite on (saved first as
                                    "_before-switch").
  -Action Unswitch                  Apply "_before-switch".
  -Action List                      Show the saved profiles.

Windhawk keeps mod settings under HKLM, so Apply, Undo, Switch and Unswitch
ask for administrator rights once. Close the Windhawk window first, or it can
write its own copy of a mod's settings back over the change.
#>
param(
    [ValidateSet('Save', 'Apply', 'Undo', 'Switch', 'Unswitch', 'List')]
    [string]$Action = 'List',
    [string]$Profile,
    [switch]$Elevated
)

$ErrorActionPreference = 'Stop'

$hive = [Microsoft.Win32.Registry]::LocalMachine
$modsSubKey = 'SOFTWARE\Windhawk\Engine\Mods'
$profilesDir = Join-Path $PSScriptRoot 'profiles'
$logFile = Join-Path $profilesDir 'last-run.log'
$suiteId = 'local@tourne-suite-redirect-branch-wintweaks'
# Everything the redirect test build replaces: both other suite builds, the
# separate Tourne mods and Resource Redirect.
$separateTourneMods = @(
    'local@tourne-suite', 'local@tourne-suite-split', 'local@tourne-suite-redirect', 'icon-resource-redirect',
    'local@tourne-change-tray-icons', 'local@tourne-classic-tray-folders',
    'local@tourne-tray-system-icons', 'local@tourne-tray-icons',
    'local@tourne-system-text-color',
    'local@tourne-menu-no-icons', 'local@tourne-menu-no-duplicates',
    'local@tourne-tray-clock'
)

function Get-ThemeMods {
    Get-Content (Join-Path $PSScriptRoot 'theme-mods.txt') |
        ForEach-Object { $_.Trim() } |
        Where-Object { $_ -and -not $_.StartsWith('#') }
}

function Test-Admin {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    (New-Object Security.Principal.WindowsPrincipal $identity).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
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

function Test-ModInstalled([string]$id) {
    $key = $hive.OpenSubKey("$modsSubKey\$id")
    if ($key) { $key.Close(); return $true }
    $false
}

function Save-Profile([string]$name) {
    $dir = Join-Path $profilesDir $name
    if (Test-Path $dir) { Remove-Item -LiteralPath $dir -Recurse -Force }
    New-Item -ItemType Directory -Force $dir | Out-Null

    $state = @()
    foreach ($id in Get-ThemeMods) {
        $key = $hive.OpenSubKey("$modsSubKey\$id")
        if (-not $key) { continue }
        # Null when the mod has no Disabled value at all, so a restore can put
        # it back exactly as it was.
        $disabled = $key.GetValue('Disabled')
        $settings = $key.OpenSubKey('Settings')
        $key.Close()
        $file = $null
        if ($settings) {
            $settings.Close()
            $file = "$id.reg"
            $code = Invoke-Reg @('export', "`"HKLM\$modsSubKey\$id\Settings`"", "`"$(Join-Path $dir $file)`"", '/y')
            if ($code -ne 0) { throw "Exporting the settings of $id failed." }
        }
        $state += [pscustomobject]@{ id = $id; disabled = $disabled; settings = $file }
    }
    $state | ConvertTo-Json | Set-Content -Path (Join-Path $dir 'state.json') -Encoding UTF8
    Write-Host "Saved profile '$name' ($($state.Count) mods)."
}

function Set-ModEnabled([string]$id, [bool]$enabled) {
    $key = $hive.OpenSubKey("$modsSubKey\$id", $true)
    if (-not $key) { return }
    $key.SetValue('Disabled', [int](-not $enabled), [Microsoft.Win32.RegistryValueKind]::DWord)
    $key.Close()
}

# Tells running Windhawk engines to reload the mod's settings.
function Update-ChangeTime([string]$id) {
    $key = $hive.OpenSubKey("$modsSubKey\$id", $true)
    if (-not $key) { return }
    $key.SetValue('SettingsChangeTime', [int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds(),
        [Microsoft.Win32.RegistryValueKind]::DWord)
    $key.Close()
}

function Apply-Profile([string]$name, [string]$backupName) {
    $dir = Join-Path $profilesDir $name
    $statePath = Join-Path $dir 'state.json'
    if (-not (Test-Path $statePath)) { throw "There's no profile named '$name'." }
    # Windows PowerShell returns a JSON array as one object; piping it through
    # ForEach-Object unrolls it into its entries.
    $state = @((Get-Content $statePath -Raw | ConvertFrom-Json) | ForEach-Object { $_ })

    Save-Profile $backupName

    $applied = 0
    foreach ($entry in $state) {
        if (-not (Test-ModInstalled $entry.id)) {
            Write-Warning "$($entry.id) isn't installed, skipped."
            continue
        }
        if ($entry.settings) {
            $hive.DeleteSubKeyTree("$modsSubKey\$($entry.id)\Settings", $false)
            if ((Invoke-Reg @('import', "`"$(Join-Path $dir $entry.settings)`"")) -ne 0) {
                throw "Importing the settings of $($entry.id) failed; the state before is in '$backupName'."
            }
        }
        if ($null -eq $entry.disabled) {
            $key = $hive.OpenSubKey("$modsSubKey\$($entry.id)", $true)
            $key.DeleteValue('Disabled', $false)
            $key.Close()
        } else {
            Set-ModEnabled $entry.id ($entry.disabled -eq 0)
        }
        Update-ChangeTime $entry.id
        $applied++
    }
    Write-Host "Applied profile '$name' to $applied mods. The state before is saved as '$backupName'."
}

function Switch-ToSuite {
    if (-not (Test-ModInstalled $suiteId)) {
        throw "The redirect test build isn't installed yet. Paste tourne-suite-redirect-branch-wintweaks.wh.cpp into a new Windhawk mod and compile it first."
    }
    Save-Profile '_before-switch'

    # Windhawk should have taken every row of the multi-row defaults; if it
    # kept only the first, bring the rest in from the generated settings file.
    $settings = $hive.OpenSubKey("$modsSubKey\$suiteId\Settings")
    $complete = $settings -and $settings.GetValue('trayIcons.icons[1].path') -and
        $settings.GetValue('iconRedirect.redirectionResourcePaths[1].original')
    if ($settings) { $settings.Close() }
    if (-not $complete) {
        $reg = Join-Path $PSScriptRoot 'tourne-suite-redirect-branch-wintweaks-settings.reg'
        if ((Invoke-Reg @('import', "`"$reg`"")) -ne 0) { throw "Importing $reg failed." }
        Write-Host 'Filled in the suite settings from tourne-suite-redirect-branch-wintweaks-settings.reg.'
    }

    foreach ($id in $separateTourneMods) { Set-ModEnabled $id $false }
    Set-ModEnabled $suiteId $true
    Update-ChangeTime $suiteId
    Write-Host "Switched: the other suite builds, the separate Tourne mods and Resource Redirect are off, and the redirect test build is on."
}

function Invoke-Elevated {
    New-Item -ItemType Directory -Force $profilesDir | Out-Null
    try {
        & {
            switch ($Action) {
                'Apply' { Apply-Profile $Profile '_before-last-apply' }
                'Undo' { Apply-Profile '_before-last-apply' '_before-undo' }
                'Switch' { Switch-ToSuite }
                'Unswitch' {
                    Apply-Profile '_before-switch' '_before-unswitch'
                    # Windhawk turns a new mod on as it compiles it, so the
                    # snapshot can have this build on too.
                    Set-ModEnabled $suiteId $false
                    Update-ChangeTime $suiteId
                }
            }
        } *>&1 | Out-File -FilePath $logFile -Encoding utf8
        return 0
    } catch {
        "ERROR: $($_.Exception.Message)" | Out-File -FilePath $logFile -Encoding utf8 -Append
        return 1
    }
}

New-Item -ItemType Directory -Force $profilesDir | Out-Null

switch ($Action) {
    'List' {
        Get-ChildItem $profilesDir -Directory | Sort-Object Name | ForEach-Object { $_.Name }
        exit 0
    }
    'Save' {
        if (-not $Profile) { $Profile = Read-Host 'Name for this theme' }
        if (-not $Profile) { exit 1 }
        Save-Profile $Profile
        exit 0
    }
}

if ($Action -eq 'Apply' -and -not $Profile) {
    $names = @(Get-ChildItem $profilesDir -Directory | Where-Object { -not $_.Name.StartsWith('_') } |
        Sort-Object Name | ForEach-Object { $_.Name })
    if (-not $names) { Write-Host 'No saved themes yet. Run Save Theme first.'; exit 1 }
    for ($i = 0; $i -lt $names.Count; $i++) { Write-Host "  $($i + 1). $($names[$i])" }
    $choice = Read-Host 'Theme to apply (number)'
    if ($choice -notmatch '^\d+$' -or [int]$choice -lt 1 -or [int]$choice -gt $names.Count) { exit 1 }
    $Profile = $names[[int]$choice - 1]
}

if ($Elevated) {
    exit (Invoke-Elevated)
}

if (Test-Admin) {
    $code = Invoke-Elevated
} else {
    $childArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`"",
                   '-Action', $Action, '-Elevated')
    if ($Profile) { $childArgs += @('-Profile', "`"$Profile`"") }
    try {
        $child = Start-Process -FilePath (Get-Process -Id $PID).Path -ArgumentList $childArgs -Verb RunAs -Wait -PassThru
    } catch {
        Write-Error 'Administrator rights were declined. Nothing was changed.'
        exit 1
    }
    $code = $child.ExitCode
}
if (Test-Path $logFile) { Get-Content $logFile | Write-Host }
exit $code
