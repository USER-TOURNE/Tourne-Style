# Why doesn't Explorer look like it should? Reads (never writes) every piece that shapes
# Explorer's look and prints what it finds, with a "!" in front of anything that would change it.
# No administrator rights needed. Run it as yourself (not from Claude Code, whose HKCU and
# %LOCALAPPDATA% reads are virtualized):
#
#   powershell -ExecutionPolicy Bypass -File .\tools\windhawk\Check-ExplorerLook.ps1
#
# The report is also saved to Explorer-look-check.txt on the desktop, to send back.
#
# Who draws what (2026-10-08): the Windows 7 command bar, classic address bar, no tabs, drive
# groups, dark Properties dialogs and the visual style inside Explorer are b53's Explorer Style;
# drive bars and the palette's colours are Explorer Colours and Theme Gaps; green text is Text
# Color; menus are Classic Menus. Tourne Desktop only touches Explorer's windows through glass
# (glass.app.explorer.*) and the window borders it draws over them.
[CmdletBinding()]
param([string]$Build = 'b53')

$lines = [System.Collections.Generic.List[string]]::new()
function Say([string]$Text) { $lines.Add($Text); Write-Output $Text }
function Bad([string]$Text) { Say "  ! $Text" }
function Ok([string]$Text) { Say "    $Text" }
function Prop($Key, [string]$Name) { (Get-ItemProperty -LiteralPath $Key -Name $Name -ErrorAction SilentlyContinue).$Name }

Say "Explorer look check, $(Get-Date -Format 'yyyy-MM-dd HH:mm')"

# Windhawk and the suite build
Say ''
Say 'Windhawk'
$svc = Get-Service Windhawk -ErrorAction SilentlyContinue
if (-not $svc) { Bad 'the Windhawk service is not installed' }
elseif ($svc.Status -ne 'Running') { Bad "the Windhawk service is $($svc.Status)" }
else { Ok 'service running' }
$mods = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
$suite = "$mods\local@tourne-suite-$Build"
if (-not (Test-Path $suite)) {
    Bad "local@tourne-suite-$Build is not in Windhawk"
} else {
    if ((Prop $suite 'Disabled') -eq 1) { Bad "$Build is switched off in Windhawk (Disabled = 1)" } else { Ok "$Build switched on" }
    if (-not (Prop $suite 'Version')) { Bad "$Build has no Version (not compiled, or a leftover key)" } else { Ok "$Build version $(Prop $suite 'Version')" }
    $exclude = "$(Prop $suite 'Exclude')"
    if ($exclude -match '(^|\|)\s*explorer\.exe\s*(\||$)') { Bad "explorer.exe is in $Build's exclude list: $exclude" }
    else { Ok "exclude list: $(if ($exclude) { $exclude } else { '(empty)' })" }
    $include = "$(Prop $suite 'Include')"
    if ($include) { Ok "include list: $include" }

    $s = "$suite\Settings"
    foreach ($part in 'explorerStyle', 'explorerColors', 'themeGaps', 'textColor', 'classicMenus', 'darkMagic', 'iconRedirect', 'menuNoIcons') {
        $on = Prop $s "$part.enabled"
        if ($on -ne 1) { Bad "$part is off ($part.enabled = $on)" } else { Ok "$part on" }
    }
    $style = "$(Prop $s 'explorerStyle.style')"
    if (-not $style) { Bad 'explorerStyle.style is empty: Explorer keeps Windows'' own look' }
    else {
        $full = if ([IO.Path]::IsPathRooted($style)) { $style } else { Join-Path $env:LOCALAPPDATA "Tourne\Styles\$style" }
        if (Test-Path -LiteralPath $full) { Ok "visual style: $full" }
        else { Bad "visual style not found: $full" }
    }
    foreach ($name in 'explorerStyle.commandBar', 'explorerStyle.detailsPane', 'explorerStyle.driveGrouping', 'explorerStyle.darkDialogs',
                      'explorerColors.driveBars', 'textColor.color') {
        $v = Prop $s $name
        if ($null -ne $v) { Ok "$name = $v" }
    }
    if ((Prop $s 'explorerStyle.commandBar') -eq 'windows11') { Bad 'commandBar = windows11: Windows 11''s header, tabs and command bar come back' }
}

Say ''
Say 'Other Windhawk mods (on ones first)'
if (Test-Path $mods) {
    $all = Get-ChildItem $mods | ForEach-Object {
        [pscustomobject]@{ Name = $_.PSChildName; Off = ((Prop $_.PSPath 'Disabled') -eq 1); Version = (Prop $_.PSPath 'Version') }
    }
    $suites = @($all | Where-Object { $_.Name -like 'local@tourne-suite-*' -and -not $_.Off -and $_.Version })
    if ($suites.Count -gt 1) { Bad "more than one suite build is on: $(($suites.Name) -join ', ') (they fight over the same windows)" }
    foreach ($m in ($all | Sort-Object Off, Name)) {
        if ($m.Name -eq "local@tourne-suite-$Build") { continue }
        Ok ("{0,-4} {1}" -f $(if ($m.Off) { 'off' } else { 'ON' }), $m.Name)
    }
}

# What actually loaded into Explorer
Say ''
Say 'Inside explorer.exe'
$explorers = @(Get-Process explorer -ErrorAction SilentlyContinue)
if (-not $explorers) { Bad 'no explorer.exe running (folder windows and Windows apps need it)' }
foreach ($p in $explorers) {
    $started = try { $p.StartTime.ToString('yyyy-MM-dd HH:mm') } catch { '?' }
    Say "    pid $($p.Id), started $started"
    $names = try { @($p.Modules | ForEach-Object { $_.ModuleName }) } catch { @() }
    if (-not $names) { Bad "can't read its modules (another user, or elevated)"; continue }
    $suiteDll = @($names | Where-Object { $_ -like "*tourne-suite-$Build*" })
    if ($suiteDll) { Ok "$Build loaded: $($suiteDll -join ', ')" } else { Bad "$Build is NOT loaded in this Explorer" }
    $otherSuite = @($names | Where-Object { $_ -like '*tourne-suite-*' -and $_ -notlike "*tourne-suite-$Build*" })
    if ($otherSuite) { Bad "another suite build loaded too: $($otherSuite -join ', ')" }
    $sab = @($names | Where-Object { $_ -match 'StartAllBack|StartIsBack' })
    if ($sab) { Ok "StartAllBack loaded: $($sab -join ', ')" }
    $other = @($names | Where-Object { $_ -match '_\d[\d.]*_\d+\.dll$' -and $_ -notlike '*tourne-suite-*' })
    if ($other) { Ok "other mods loaded: $($other.Count)" }
}

# StartAllBack (its Explorer options can stack on b53's)
Say ''
Say 'StartAllBack'
$sabDir = @("$env:ProgramFiles\StartAllBack", "$env:LOCALAPPDATA\StartAllBack") | Where-Object { Test-Path "$_\StartAllBackCfg.exe" }
if ($sabDir) { Ok "installed: $($sabDir -join ', ')" } else { Ok 'not installed' }
if (Test-Path 'HKCU:\Software\StartIsBack') {
    foreach ($name in 'Disabled', 'FrameStyle', 'ModernIconsColorized', 'NoXAMLPrelaunch') {
        $v = Prop 'HKCU:\Software\StartIsBack' $name
        if ($null -ne $v) { Ok "$name = $v" }
    }
}

# Windows' own visual style and fonts (title bars, Properties, Task Manager)
Say ''
Say 'Windows'
$tm = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\ThemeManager'
$dll = "$(Prop $tm 'DllName')"
if ($dll -match '\\aero\\aero\.msstyles$' -or -not $dll) { Bad "Windows' visual style is the stock one: $dll" } else { Ok "visual style: $dll" }
Ok "theme: $(Prop 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes' 'CurrentTheme')"
$sub = Prop 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\FontSubstitutes' 'Segoe UI'
if ($sub) { Ok "Segoe UI is replaced by: $sub" } else { Bad 'Segoe UI is not replaced (no pixel font in Windows'' own text)' }
$apps = Prop 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize' 'AppsUseLightTheme'
Ok "apps light theme: $apps (0 = dark)"
$prev = Prop 'HKCU:\Software\Microsoft\Windows\DWM' 'ColorPrevalence'
Ok "accent on title bars: $prev"

# Tourne Desktop's side
Say ''
Say 'Tourne Desktop'
$conf = Join-Path $env:APPDATA 'Tourne\Desktop\tourne-gui.conf'
if (-not (Test-Path $conf)) { Ok "no $conf" }
else {
    $values = @{}
    foreach ($line in Get-Content $conf) { if ($line -match '^\s*([^#=\s][^=]*?)\s*=\s*(.*?)\s*$') { $values[$matches[1]] = $matches[2] } }
    $glassOn = $values['glass.enabled'] -in '1', 'true'
    $explorerGlass = $values['glass.app.explorer.on'] -in '1', 'true'
    if ($glassOn -and $explorerGlass) { Bad "glass is on for Explorer ($($values['glass.app.explorer.backdrop'])): Windows' Mica/Acrylic drawn behind b53's style" }
    else { Ok "glass for Explorer: off (glass.enabled = $($values['glass.enabled']), glass.app.explorer.on = $($values['glass.app.explorer.on']))" }
    foreach ($k in 'shell.enabled', 'shell.windowsApps', 'switcher.borders.enabled', 'switcher.borders.activeColor', 'switcher.borders.shadow') {
        if ($values.ContainsKey($k)) { Ok "$k = $($values[$k])" }
    }
}
$running = @(Get-Process TourneDesktop -ErrorAction SilentlyContinue)
Ok "TourneDesktop.exe running: $($running.Count)"
foreach ($p in $running) {
    $names = try { @($p.Modules | ForEach-Object { $_.ModuleName }) } catch { @() }
    if ($names | Where-Object { $_ -like '*tourne-suite-*' }) { Ok "  pid $($p.Id) has the suite loaded (Set-OneOwner -DesktopInTourne)" }
}

$out = Join-Path ([Environment]::GetFolderPath('Desktop')) 'Explorer-look-check.txt'
$lines | Set-Content -LiteralPath $out -Encoding UTF8
Write-Output ''
Write-Output "Saved: $out"
