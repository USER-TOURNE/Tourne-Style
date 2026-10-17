# Puts back the looks other programs lost on 2026-10-08 (Explorer's right-click went to Windows 11's style,
# Task Manager lost its icons): Set-NoMenuOverlaps.ps1 turned classic-menus, dark-menus and
# eradicate-immersive-menus off everywhere, when only Tourne Desktop needed to be left out of them; and
# b53's Icon Redirect was off. Needs administrator rights (Windhawk keeps its settings under HKLM).
# Every mod's settings as they were are exported beside this script first (*.reg here is ignored by git).
#
#   .\tools\windhawk\Restore-AppLooks.ps1 -WhatIf     # say what would change
#   .\tools\windhawk\Restore-AppLooks.ps1 -Log <file> # do it, writing what changed to a file too
[CmdletBinding(SupportsShouldProcess)]
param([string]$Build = 'b53', [string]$Log = '', [string]$Conf = '')
$ErrorActionPreference = 'Stop'
if ($Log) { Start-Transcript -Path $Log -Force | Out-Null }
try {
    $mods = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
    $principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    if (-not $WhatIfPreference -and -not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Run this as administrator (Windhawk keeps its settings under HKLM).'
    }
    if (-not $WhatIfPreference) {
        $backup = Join-Path $PSScriptRoot ('mods-before-restore-app-looks-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.reg')
        & reg.exe export 'HKLM\SOFTWARE\Windhawk\Engine\Mods' $backup /y | Out-Null
        Write-Output "Saved every mod's settings as they were: $backup"
    }
    $changed = 0
    function Set-Value([string]$Key, [string]$Name, $Value, [string]$Type) {
        if (-not (Test-Path $Key)) { Write-Output "  (not installed: $Key)"; return }
        $now = (Get-ItemProperty $Key -Name $Name -ErrorAction SilentlyContinue).$Name
        if ("$now" -eq "$Value") { Write-Output "  $(Split-Path $Key -Leaf)\$Name already '$Value'"; return }
        if ($PSCmdlet.ShouldProcess("$Key\$Name", "set to '$Value' (was '$now')")) {
            Set-ItemProperty $Key -Name $Name -Value $Value -Type $Type
            $mod = if ((Split-Path $Key -Leaf) -eq 'Settings') { Split-Path $Key -Parent } else { $Key }
            Set-ItemProperty $mod -Name SettingsChangeTime -Value ([int][DateTimeOffset]::UtcNow.ToUnixTimeSeconds()) -Type DWord
        }
        Write-Output "  $(Split-Path $Key -Leaf)\$Name = $Value (was '$now')"
        $script:changed++
    }

    # The menu mods back on everywhere but Tourne Desktop (which draws its own menus).
    foreach ($mod in 'classic-menus', 'dark-menus', 'eradicate-immersive-menus') {
        $key = "$mods\$mod"
        if (-not (Test-Path $key)) { Write-Output "  (not installed: $mod)"; continue }
        $exclude = "$((Get-ItemProperty $key -Name Exclude -ErrorAction SilentlyContinue).Exclude)"
        $entries = @($exclude -split '\|' | ForEach-Object { $_.Trim() } | Where-Object { $_ })
        if ($entries -notcontains 'TourneDesktop.exe') { $entries += 'TourneDesktop.exe' }
        Set-Value $key 'Exclude' ($entries -join '|') 'String'
        Set-Value $key 'Disabled' 0 'DWord'
    }

    # b53's Icon Redirect on, with Tourne Desktop's "Use everywhere" Button icons (.ico files) as redirects,
    # so Task Manager and Explorer show the same icons as Tourne's taskbar.
    $settings = "$mods\local@tourne-suite-$Build\Settings"
    Set-Value $settings 'iconRedirect.enabled' 1 'DWord'
    if (-not $Conf) { $Conf = Join-Path ([Environment]::GetFolderPath('ApplicationData')) 'Tourne\Desktop\tourne-gui.conf' }
    if (Test-Path $Conf) {
        $lines = Get-Content $Conf
        $value = { param($name) $l = $lines | Where-Object { $_.StartsWith("$name = ") } | Select-Object -First 1; if ($l) { $l.Substring($name.Length + 3).Trim('"') } }
        $props = Get-ItemProperty $settings
        $originals = @{}
        $next = 0
        for ($i = 0; $i -lt 1024; $i++) {
            $o = $props."iconRedirect.redirectionResourcePaths[$i].original"
            if ($o) { $originals[$o.ToLowerInvariant()] = $true; $next = $i + 1 }
        }
        $localAppData = [Environment]::GetFolderPath('LocalApplicationData')
        foreach ($line in $lines | Where-Object { $_ -match '^taskbar\.buttonIcons\[(\d+)\]\.everywhere = (1|true)$' }) {
            $i = [regex]::Match($line, '\[(\d+)\]').Groups[1].Value
            $program = & $value "taskbar.buttonIcons[$i].program"
            $icon = & $value "taskbar.buttonIcons[$i].icon"
            if (-not $program -or -not $icon -or $icon -notmatch '\.ico$') { continue }
            if ($program -notmatch '\.exe$') { continue }  # (app ids and window classes: Icon Redirect matches files)
            $icon = $icon -replace '%LOCALAPPDATA%', $localAppData
            $original = if ($program -match '[\\/]') { $program } else { "*\$program" }
            if ($originals.ContainsKey($original.ToLowerInvariant())) { Write-Output "  redirect for $original already there"; continue }
            Set-Value $settings "iconRedirect.redirectionResourcePaths[$next].original" $original 'String'
            Set-Value $settings "iconRedirect.redirectionResourcePaths[$next].redirect" $icon 'String'
            $originals[$original.ToLowerInvariant()] = $true
            $next++
        }
    } else {
        Write-Output "  (no Tourne Desktop config at ${Conf}: icon rows not copied)"
    }

    if ($changed -and $PSCmdlet.ShouldProcess('Windhawk service', 'restart')) {
        Restart-Service Windhawk
        Write-Output 'Windhawk restarted: every program takes the changes in (Explorer windows and Task Manager opened from now on).'
    }
    Write-Output "Done: $changed change(s)."
} finally {
    if ($Log) { Stop-Transcript | Out-Null }
}
