<#
Builds tourne-suite-redirect-branch-wintweaks.wh.cpp (the wintweaks branch of
the redirect test build): the split test
build (System Icons in its own process, from system-icons-host.cpp) plus Icon
Redirect (tourne-icon-redirect.wh.cpp), which stands in for the Resource
Redirect mod. The suite code comes from suite-redirect.template.cpp.

Each mod's code goes into its own namespace with four mechanical changes:
lifecycle functions become Part*, settings keys get the part's group prefix,
hook registration goes through the suite's chaining helper, and includes move
to the suite's shared list. The settings schema is each mod's own, nested
under a group with an Enabled switch, with defaults taken from the installed suite's
current settings.

Re-run after changing one of the source mods to rebuild the suite.
#>
param(
    # The parts that came from installed mods, kept here since those mods were removed.
    [string]$ModsSource = '%USERPROFILE%\Downloads\tourne-suite\parts',
    # Whose settings become the defaults: by default the suite build that is switched on right now (the
    # highest-numbered enabled tourne-suite-b<N>, a suffix allowed: b20glowrevise), so each build starts from the settings in use.
    [string]$SettingsSource = '',
    [string]$ClockSource = '%USERPROFILE%\Downloads\tourne-tray-clock.wh.cpp',
    [string]$TrayFoldersSource = '%USERPROFILE%\Downloads\tourne-classic-tray-folders.wh.cpp',
    [string]$SystemIconsSource = (Join-Path $PSScriptRoot 'system-icons-host.cpp'),
    [string]$IconRedirectSource = (Join-Path $PSScriptRoot 'tourne-icon-redirect.wh.cpp'),
    # Generated from tweaks.def by Build-Tweaks.ps1, which runs first.
    [string]$TweaksSource = (Join-Path $PSScriptRoot 'tourne-tweaks.wh.cpp'),
    # Icon Redirect's defaults: Resource Redirect's settings, until this build
    # has settings of its own.
    [string]$RedirectSettingsSource = 'icon-resource-redirect',
    # Each build gets the next number and its own mod id (tourne-suite-b<N>), written under builds\, so any one can be installed beside the others and older ones stay for going back.
    [int]$BuildNumber = 0,
    # Settings to change on the way: a JSON file of full setting names and values (text, or numbers for
    # switches and numbers), applied over the settings source, in the build's defaults and both .reg files.
    [string]$SettingsChanges = '',
    # A build for someone else's PC: nothing from this one (no installed settings, StartAllBack's, Resource
    # Redirect's or this PC's tweaks), only each part's own defaults. Written to -TesterOutput as
    # tourne-suite-test.wh.cpp; build-number.txt and builds\ are left alone. The tester's own settings are
    # filled in on their PC by the kit's Fill-MySettings.ps1.
    [switch]$Tester,
    [string]$TesterOutput = ''
)
$script:changes = [ordered]@{}
if ($SettingsChanges) {
    foreach ($p in (Get-Content $SettingsChanges -Raw | ConvertFrom-Json).PSObject.Properties) {
        $script:changes[$p.Name] = if ($p.Value -is [string]) { $p.Value } else { [int]$p.Value }
    }
    "Settings changes: $($script:changes.Count) from $SettingsChanges"
}

$ErrorActionPreference = 'Stop'

$buildsDir = Join-Path $PSScriptRoot 'builds'
New-Item -ItemType Directory -Force $buildsDir | Out-Null
$counter = Join-Path $PSScriptRoot 'build-number.txt'
if ($Tester) {
    # Named after the build it matches, without taking a number of its own.
    if ($BuildNumber -le 0) { $BuildNumber = [int](Get-Content $counter -Raw).Trim() }
    if (-not $TesterOutput) { throw '-Tester needs -TesterOutput (the kit folder)' }
    New-Item -ItemType Directory -Force $TesterOutput | Out-Null
    $SuiteName = 'tourne-suite-test'
    $SuiteId = "local@$SuiteName"
    $Output = Join-Path $TesterOutput "$SuiteName.wh.cpp"
    $buildsDir = $TesterOutput
    $SettingsSource = 'none'  # (no such mod: every part keeps its own defaults)
    $TweaksSource = Join-Path ([IO.Path]::GetTempPath()) 'tourne-tweaks-tester.wh.cpp'
    "Building $SuiteName (tester build of b$BuildNumber)"
    & (Join-Path $PSScriptRoot 'Build-Tweaks.ps1') -Output $TweaksSource -SuiteId $SuiteId -WindowsDefaults `
        -RegOutput (Join-Path ([IO.Path]::GetTempPath()) 'tourne-tweaks-tester.reg')
} else {
    if ($BuildNumber -le 0) {
        $last = if (Test-Path $counter) { [int](Get-Content $counter -Raw).Trim() } else { 0 }
        $BuildNumber = $last + 1
    }
    Set-Content $counter $BuildNumber -NoNewline
    $SuiteName = "tourne-suite-b$BuildNumber"
    $SuiteId = "local@$SuiteName"
    $Output = Join-Path $buildsDir "$SuiteName.wh.cpp"
    "Building $SuiteName"

    # The Tweaks part is generated first, with its defaults read from this PC.
    & (Join-Path $PSScriptRoot 'Build-Tweaks.ps1') -Output $TweaksSource -SuiteId $SuiteId
}

$modsKey = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'

if (-not $SettingsSource) {
    $enabled = Get-ChildItem $modsKey -ErrorAction SilentlyContinue |
        Where-Object { $_.PSChildName -match '^local@tourne-suite-b(\d+)[a-z0-9-]*$' -and (Get-ItemProperty $_.PSPath).Disabled -eq 0 } |
        Sort-Object { [int][regex]::Match($_.PSChildName, 'b(\d+)').Groups[1].Value } -Descending
    $SettingsSource = if ($enabled) { $enabled[0].PSChildName } else { 'local@tourne-suite-taskbar-test' }
    "Settings from $SettingsSource (the build in use)"
}

$parts = @(
    @{ Id = 'local@tourne-change-tray-icons';    Ns = 'part_tray_icons';          Prefix = 'trayIcons';        Title = 'Change Tray Icons';    Win64 = $false; Source = $null }
    @{ Id = 'local@tourne-classic-tray-folders'; Ns = 'part_tray_folders';        Prefix = 'trayFolders';      Title = 'Classic Tray Folders'; Win64 = $true;  Source = $TrayFoldersSource }
    @{ Id = 'local@tourne-tray-icons';           Ns = 'part_system_icons';        Prefix = 'systemIcons';      Title = 'System Icons';         Win64 = $true;  Source = $SystemIconsSource }
    @{ Id = 'local@tourne-system-text-color';    Ns = 'part_text_color';          Prefix = 'textColor';        Title = 'System Text Color';    Win64 = $false; Source = $null }
    @{ Id = 'local@tourne-menu-no-icons';        Ns = 'part_menu_no_icons';       Prefix = 'menuNoIcons';      Title = 'Menu No Icons';        Win64 = $false; Source = $null }
    @{ Id = 'local@tourne-menu-no-duplicates';   Ns = 'part_menu_no_duplicates';  Prefix = 'menuNoDuplicates'; Title = 'Menu No Duplicates';   Win64 = $false; Source = $null }
    @{ Id = 'local@tourne-tray-clock';           Ns = 'part_clock';               Prefix = 'clock';            Title = 'Clock';                Win64 = $true;  Source = $ClockSource }
    @{ Id = 'local@tourne-icon-redirect';        Ns = 'part_icon_redirect';       Prefix = 'iconRedirect';     Title = 'Icon Redirect';        Win64 = $false; Source = $IconRedirectSource }
    @{ Id = 'local@tourne-tweaks';               Ns = 'part_tweaks';              Prefix = 'tweaks';           Title = 'Tweaks';               Win64 = $false; Source = $TweaksSource }
    @{ Id = 'local@tourne-taskbar';              Ns = 'part_taskbar';             Prefix = 'taskbar';          Title = 'Taskbar (test)';       Win64 = $true;  Source = (Join-Path $PSScriptRoot 'taskbar-host.cpp') }
    @{ Id = 'local@tourne-switcher';             Ns = 'part_switcher';            Prefix = 'switcher';         Title = "Switcher (Alt+Tab)";   Win64 = $true;  Source = (Join-Path $PSScriptRoot 'switcher-host.cpp') }
    @{ Id = 'local@tourne-explorer-style';       Ns = 'part_explorer_style';      Prefix = 'explorerStyle';    Title = 'Explorer Style';       Win64 = $true;  Source = (Join-Path $PSScriptRoot 'explorer-style.cpp') }
    @{ Id = 'local@tourne-explorer-colors';      Ns = 'part_explorer_colors';     Prefix = 'explorerColors';   Title = 'Explorer Colours';     Win64 = $true;  Source = (Join-Path $PSScriptRoot 'explorer-colors.cpp') }
    @{ Id = 'local@tourne-theme-gaps';           Ns = 'part_theme_gaps';          Prefix = 'themeGaps';        Title = 'Theme Gaps';           Win64 = $false; Source = (Join-Path $PSScriptRoot 'theme-gaps.cpp') }
    @{ Id = 'local@tourne-dark-magic';           Ns = 'part_dark_magic';          Prefix = 'darkMagic';        Title = 'Dark Magic';           Win64 = $false; Source = (Join-Path $PSScriptRoot 'dark-magic.cpp') }
    @{ Id = 'local@tourne-classic-menus';        Ns = 'part_classic_menus';       Prefix = 'classicMenus';     Title = 'Classic Menus';        Win64 = $false; Source = (Join-Path $PSScriptRoot 'immersive-menus.cpp') }
)

# Parts that start switched off: the test taskbar, and the switcher, which
# would fight another Alt+Tab mod until that one is turned off, and the
# newest parts, so a new build changes nothing until they're switched on.
$startOff = @('taskbar', 'switcher', 'darkMagic', 'explorerColors')

# "// @inline <file>" lines are replaced by that file, from the source's folder
# (System Icons' device batteries live in device-batteries.cpp).
function Read-Source($part) {
    $path = if ($part.Source) { $part.Source } else { Join-Path $ModsSource "$($part.Id).wh.cpp" }
    $text = [IO.File]::ReadAllText($path) -replace "`r`n", "`n"
    $dir = Split-Path $path
    [regex]::Replace($text, '(?m)^// @inline (\S+)$', {
        param($m)
        ([IO.File]::ReadAllText((Join-Path $dir $m.Groups[1].Value)) -replace "`r`n", "`n").TrimEnd("`n")
    })
}

# A part's current settings, from the installed suite, where each is stored as
# "<prefix>.<name>". The calendar link is private, so it never becomes a
# default in a file that might be shared.
# Whether a part starts on: as the settings source has it, else on unless it's
# one of the parts that start off.
$script:sourceKey = Get-Item "$modsKey\$SettingsSource\Settings" -ErrorAction SilentlyContinue
function Get-Enabled($part) {
    # A tester's build also starts the parts that restyle Windows to one person's taste off (green text,
    # menus without icons, classic menus): theirs to try, not to find.
    if ($Tester -and $part.Prefix -in 'textColor', 'menuNoIcons', 'classicMenus') { return 0 }
    if ($script:sourceKey) {
        $v = $script:sourceKey.GetValue("$($part.Prefix).enabled")
        if ($null -ne $v) { return [int]([int]$v -ne 0) }
    }
    [int]($part.Prefix -notin $startOff)
}

# A part's settings with the requested changes applied (-SettingsChanges).
function Get-Settings($part) {
    $values = Get-SourceSettings $part
    # A tester starts from neutral choices where a part's own default is one person's taste: colours from
    # their own visual style (as START HERE says), and no menu items or programs picked for this PC. (Each
    # -SettingsChanges value still wins.)
    if ($Tester) {
        switch ($part.Prefix) {
            'systemIcons' { $values['scheme'] = 'taskbarStyle' }
            'menuNoDuplicates' { $values['names'] = '' }
            'menuNoIcons' { $values['processes'] = 'explorer.exe, dllhost.exe, rundll32.exe' }
        }
    }
    $prefix = "$($part.Prefix)."
    foreach ($name in $script:changes.Keys) {
        if ($name.StartsWith($prefix) -and $name -ne "$($prefix)enabled") { $values[$name.Substring($prefix.Length)] = $script:changes[$name] }
    }
    $values
}

# Tweaks were regrouped (b52): a tweak keeps its id, but its setting is now "<new group>.<id>". Each
# tweak's group as tweaks.def has it now, so a value saved under its old group carries over.
$script:tweakGroups = @{}
$group = $null
foreach ($line in [IO.File]::ReadAllLines((Join-Path $PSScriptRoot 'tweaks.def'))) {
    if ($line -match '^group (\w+)') { $group = $matches[1] }
    elseif ($line -match '^(toggle|number|text|choice) (\w+) ') { $script:tweakGroups[$matches[2]] = $group }
}

# A Tweaks setting's name ("<group>.<id>") as it is now; other names as they are.
function Get-TweakName([string]$short) {
    if ($short -match '^(\w+)\.(\w+)$' -and $matches[2] -ne 'use') {
        $now = $script:tweakGroups[$matches[2]]
        if ($now -and $now -ne $matches[1]) { return "$now.$($matches[2])" }
    }
    $short
}

function Get-SourceSettings($part) {
    if ($part.Prefix -eq 'iconRedirect') { return Get-RedirectSettings }
    # Tweaks too take the installed suite's settings; Build-Tweaks.ps1's (read from this PC's state) are
    # left only for the ones it doesn't have.
    $key = Get-Item "$modsKey\$SettingsSource\Settings" -ErrorAction SilentlyContinue
    $values = @{}
    if ($key) {
        $prefix = "$($part.Prefix)."
        foreach ($name in $key.GetValueNames()) {
            if (-not $name.StartsWith($prefix)) { continue }
            $short = $name.Substring($prefix.Length)
            if ($short -in 'enabled', 'calendarIcs') { continue }
            if ($part.Prefix -eq 'tweaks') {
                $now = Get-TweakName $short
                if ($now -ne $short) {
                    # (one saved under its new name already wins)
                    if (-not $key.GetValueNames().Contains("$prefix$now")) { $values[$now] = $key.GetValue($name) }
                    continue
                }
            }
            $values[$short] = $key.GetValue($name)
        }
    }
    # Button icons are a list the user builds up, and a build installed without its settings comes up with an
    # empty one; that empty list then became the next build's "current" settings. With none set, take the list
    # from the earlier build that had the most.
    if ($part.Prefix -eq 'taskbar' -and -not $Tester) {
        $mine = @($values.Keys | Where-Object { $_ -match '^buttonIcons\[\d+\]\.program$' -and $values[$_] })
        if (-not $mine.Count) {
            $best = $null; $bestCount = 0
            $builds = Get-ChildItem $modsKey -ErrorAction SilentlyContinue | Where-Object { $_.PSChildName -match '^local@tourne-suite-' }
            foreach ($other in ($builds | Sort-Object { [int]([regex]::Match($_.PSChildName, '\d+$').Value -replace '^$', '0') } -Descending)) {
                $sk = Get-Item "$($other.PSPath)\Settings" -ErrorAction SilentlyContinue
                if (-not $sk) { continue }
                $rows = @($sk.GetValueNames() | Where-Object { $_ -match '^taskbar\.buttonIcons\[\d+\]\.program$' -and $sk.GetValue($_) })
                if ($rows.Count -gt $bestCount) { $best = $sk; $bestCount = $rows.Count }
            }
            if ($best) {
                foreach ($name in $best.GetValueNames()) {
                    if ($name -match '^taskbar\.(buttonIcons\[\d+\]\.(program|icon))$') { $values[$matches[1]] = $best.GetValue($name) }
                }
                Write-Host "Button icons: none in $SettingsSource; took the $bestCount from $(($best.Name -split '\\')[-2])"
            }
        }
    }
    # The battery check interval was a number of seconds; it's now text with
    # a unit ("9min", "9hr"). Carry an old value over as the new default.
    if ($part.Prefix -eq 'systemIcons' -and $values.ContainsKey('batteries.pollSeconds')) {
        $old = [int]$values['batteries.pollSeconds']
        if (-not $values.ContainsKey('batteries.pollInterval') -and $old -gt 0) {
            $values['batteries.pollInterval'] = if ($old % 3600 -eq 0) { "$($old / 3600)hr" }
                                                elseif ($old % 60 -eq 0) { "$($old / 60)min" }
                                                else { "${old}sec" }
        }
        $values.Remove('batteries.pollSeconds')
    }
    # The taskbar's Start menu settings follow StartAllBack's page: what isn't
    # set here yet starts as StartAllBack is set (sab-settings-map.md). (A tester's StartAllBack is read
    # on their PC, by the kit.)
    if ($part.Prefix -eq 'taskbar' -and -not $Tester) {
        $sab = Get-SabDefaults
        foreach ($name in $sab.Keys) { if (-not $values.ContainsKey($name)) { $values[$name] = $sab[$name] } }
    }
    $values
}

# StartAllBack's settings as the taskbar's (Get-SabDefaults).
. (Join-Path $PSScriptRoot 'sab-defaults.ps1')

# Icon Redirect's current settings: this build's own once it has any,
# otherwise Resource Redirect's, whose names it shares. Resource Redirect's
# settings Icon Redirect has no use for (themeFolder, disableThumbnails) are
# left out.
function Get-RedirectSettings {
    $values = @{}
    if ($Tester) { return $values }
    $pattern = '^(iconTheme|allResourceRedirect|brandingImages|themePaths\[\d+\]|redirectionResourcePaths\[\d+\]\.(original|redirect))$'
    foreach ($source in @(@{ Id = $SettingsSource; Prefix = 'iconRedirect.' }, @{ Id = $RedirectSettingsSource; Prefix = '' })) {
        $key = Get-Item "$modsKey\$($source.Id)\Settings" -ErrorAction SilentlyContinue
        if (-not $key) { continue }
        foreach ($name in $key.GetValueNames()) {
            if (-not $name.StartsWith($source.Prefix)) { continue }
            $short = $name.Substring($source.Prefix.Length)
            if ($short -match $pattern) { $values[$short] = $key.GetValue($name) }
        }
        if ($values.Count) { break }
    }
    $values
}

function Q([string]$s) { "'" + $s.Replace("'", "''") + "'" }

# Array index order, not string order: icons[10] after icons[9].
function Sort-Indexed($names, $pattern) {
    $names | Where-Object { $_ -match $pattern } |
        Sort-Object { [int]([regex]::Match($_, $pattern).Groups[1].Value) }
}

####################################################################### code

function Convert-Part($part) {
    $text = Read-Source $part
    $end = $text.IndexOf('// ==/WindhawkModSettings==')
    $body = $text.Substring($end + '// ==/WindhawkModSettings=='.Length)

    $body = ($body -split "`n" | Where-Object { $_ -notmatch '^\s*#\s*include\b' }) -join "`n"
    $body = [regex]::Replace($body, '\bWh_Mod(Init|AfterInit|BeforeUninit|Uninit|SettingsChanged)\b', 'Part$1')
    $body = [regex]::Replace($body, '\bWindhawkUtils::SetFunctionHook\(', 'SuiteSetFunctionHookT(')
    $body = [regex]::Replace($body, '\bWh_SetFunctionHook\(', 'SuiteSetFunctionHook(')
    $body = [regex]::Replace($body, '\b(Wh_GetStringSetting|Wh_GetIntSetting|StringSetting::make|RedirectSetting|TextSetting|IntSetting)\(\s*L"',
        "`$1(L`"$($part.Prefix).")

    $open = "////////////////////////////////////////////////////////////////////////////////`n" +
            "// $($part.Title) (from $($part.Id))`n`n"
    if ($part.Win64) { $open += "#ifdef _WIN64`n" }
    $open += "namespace $($part.Ns) {`n"
    $close = "`n}  // namespace $($part.Ns)`n"
    if ($part.Win64) { $close += "#endif  // _WIN64`n" }
    $open + $body.Trim("`n") + "`n" + $close
}

################################################################### settings

# Rewrites one mod's settings block under a group, with registry values as
# the defaults. Scalars are replaced in place; the two object arrays are
# written out in full, since they have one element per row in use.
function Convert-Settings($part) {
    $text = Read-Source $part
    $m = [regex]::Match($text, '// ==WindhawkModSettings==\s*/\*\n?(.*?)\*/\s*// ==/WindhawkModSettings==', 'Singleline')
    $lines = $m.Groups[1].Value.TrimEnd("`n") -split "`n"
    $values = Get-Settings $part

    $out = New-Object System.Collections.Generic.List[string]
    # Groups the current line is inside, as (indent, key): paths go any depth.
    $parents = New-Object System.Collections.Generic.List[object]
    $skipArray = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Object arrays: replaced wholesale below.
        if ($line -match '^- (icons|folders):\s*$' -and $part.Prefix -in 'trayIcons', 'trayFolders') {
            $out.AddRange([string[]](Write-ObjectArray $part $matches[1] $values))
            $skipArray = $true
            continue
        }
        if ($skipArray) {
            if ($line -match '^\S') { $skipArray = $false } else { continue }
        }

        # The taskbar's button icons: one row for each in the settings, so the defaults of a new build have them.
        if ($part.Prefix -eq 'taskbar' -and $line -match '^- buttonIcons:\s*$') {
            $block = New-Object System.Collections.Generic.List[string]
            for ($j = $i + 1; $j -lt $lines.Count -and $lines[$j] -notmatch '^\S'; $j++) { $block.Add($lines[$j]) }
            $out.AddRange([string[]](Write-FieldRows $block $values 'buttonIcons' 'program|icon|everywhere'))
            $skipArray = $true
            continue
        }

        # Icon Redirect's lists.
        if ($part.Prefix -eq 'iconRedirect' -and $line -match '^- themePaths:') {
            $paths = Sort-Indexed $values.Keys '^themePaths\[(\d+)\]$' | ForEach-Object { Q $values[$_] }
            if ($paths) { $line = "- themePaths: [$($paths -join ', ')]" }
            $out.Add($line)
            continue
        }
        if ($part.Prefix -eq 'iconRedirect' -and $line -match '^- redirectionResourcePaths:\s*$') {
            $block = New-Object System.Collections.Generic.List[string]
            for ($j = $i + 1; $j -lt $lines.Count -and $lines[$j] -notmatch '^\S'; $j++) { $block.Add($lines[$j]) }
            $out.AddRange([string[]](Write-RedirectRows $block $values))
            $skipArray = $true
            continue
        }

        if ($line -match '^(\s*)- (\w+):\s*(.*)$') {
            # Copied out first: the type checks below overwrite $matches.
            $lead = $matches[1]; $indent = $lead.Length; $key = $matches[2]; $default = $matches[3].TrimEnd()
            while ($parents.Count -and $parents[$parents.Count - 1].Indent -ge $indent) { $parents.RemoveAt($parents.Count - 1) }
            $path = (@($parents | ForEach-Object { $_.Key }) + $key) -join '.'
            if ($default -eq '') { $parents.Add([pscustomobject]@{ Indent = $indent; Key = $key }) }
            if ($default -ne '' -and $values.ContainsKey($path)) {
                $value = $values[$path]
                $default = if ($default -match '^(true|false)$') { if ([int]$value) { 'true' } else { 'false' } }
                           elseif ($default -match '^-?\d+$') { "$value" }
                           else { Q "$value" }
            }
            $line = "$lead- ${key}:" + $(if ($default -ne '') { " $default" } else { '' })
        }
        $out.Add($line)
    }

    $out = Add-EnabledCondition $out

    $group = New-Object System.Collections.Generic.List[string]
    $group.Add("- $($part.Prefix):")
    $group.Add("  - enabled: $(if (Get-Enabled $part) { 'true' } else { 'false' })")
    $group.Add("    `$name: Enabled")
    foreach ($line in $out) { $group.Add("  $line") }
    $group.Add("  `$name: $($part.Title)")
    $group
}

# A part's settings show only while the part is on (Windhawk 2.0's $showIf, behind the "#! " marker that
# Windhawk 1.7.3 reads as a comment): each of its top-level settings gets {enabled: true}, the part's own
# switch beside it, merged into a condition it already has. Nested settings follow their group.
function Add-EnabledCondition($lines) {
    $result = New-Object System.Collections.Generic.List[string]
    $block = New-Object System.Collections.Generic.List[string]
    $flush = {
        if (-not $block.Count) { return }
        if ($block[0] -notmatch '^- \w+:') { $result.AddRange([string[]]$block); $block.Clear(); return }
        $merged = $false
        for ($k = 0; $k -lt $block.Count; $k++) {
            if ($block[$k] -match '^  #! \$showIf: \{(.*)\}\s*$') {
                $block[$k] = "  #! `$showIf: {enabled: true, $($matches[1])}"
                $merged = $true
            }
        }
        $last = $block.Count - 1
        while ($last -gt 0 -and $block[$last].Trim() -eq '') { $last-- }
        if (-not $merged) { $block.Insert($last + 1, '  #! $showIf: {enabled: true}') }
        $result.AddRange([string[]]$block)
        $block.Clear()
    }
    foreach ($line in $lines) {
        if ($line -match '^- \w+:') { & $flush }
        $block.Add($line)
    }
    & $flush
    $result
}

# A list of rows (name[N].field) written out in full, one per row in the settings, laid out like the first row of the part's own
# schema (only the first keeps the names and descriptions).
function Write-FieldRows($block, $values, $arrayName, $fields) {
    $tail = 0
    while ($tail -lt $block.Count -and $block[$tail] -notmatch '^  \$') { $tail++ }
    $row = @($block[0..($tail - 1)])
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add("- ${arrayName}:")
    $indexes = @(Sort-Indexed $values.Keys "^$arrayName\[(\d+)\]\.program$" | Where-Object { $values[$_] } |
        ForEach-Object { [regex]::Match($_, '\[(\d+)\]').Groups[1].Value })
    if (-not $indexes) { $indexes = @($null) }
    $first = $true
    foreach ($index in $indexes) {
        foreach ($line in $row) {
            if ($line -match "^(\s*(?:- )+)($fields):\s*(.*)$") {
                $lead = $matches[1]; $field = $matches[2]; $default = $matches[3].Trim()
                $raw = if ($null -ne $index) { $values["$arrayName[$index].$field"] } else { $null }
                # A switch keeps true/false (unset: its default); text is quoted.
                if ($default -match '^(true|false)$') {
                    $value = if ($null -ne $raw) { if ([int]$raw) { 'true' } else { 'false' } } else { $default }
                    $lines.Add("${lead}${field}: $value")
                } else {
                    $lines.Add("${lead}${field}: $(Q ([string]$raw))")
                }
            } elseif ($first) {
                $lines.Add($line)
            }
        }
        $first = $false
    }
    for ($k = $tail; $k -lt $block.Count; $k++) { $lines.Add($block[$k]) }
    $lines
}

# Icon Redirect's rows, one per registry row, laid out like the first row of
# the part's own schema; only the first row keeps the names and descriptions.
function Write-RedirectRows($block, $values) {
    $tail = 0
    while ($tail -lt $block.Count -and $block[$tail] -notmatch '^  \$') { $tail++ }
    $row = @($block[0..($tail - 1)])
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add('- redirectionResourcePaths:')
    $indexes = @(Sort-Indexed $values.Keys '^redirectionResourcePaths\[(\d+)\]\.original$' |
        ForEach-Object { [regex]::Match($_, '\[(\d+)\]').Groups[1].Value })
    if (-not $indexes) { $indexes = @($null) }
    $first = $true
    foreach ($index in $indexes) {
        foreach ($line in $row) {
            if ($line -match '^(\s*(?:- )+)(original|redirect):') {
                $value = if ($null -ne $index) { [string]$values["redirectionResourcePaths[$index].$($matches[2])"] } else { '' }
                $lines.Add("$($matches[1])$($matches[2]): $(Q $value)")
            } elseif ($first) {
                $lines.Add($line)
            }
        }
        $first = $false
    }
    for ($k = $tail; $k -lt $block.Count; $k++) { $lines.Add($block[$k]) }
    $lines
}

function Write-ObjectArray($part, $name, $values) {
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add("- ${name}:")
    if ($part.Prefix -eq 'trayIcons') {
        $rows = @(Sort-Indexed $values.Keys '^icons\[(\d+)\]\.path$')
        # None (a tester's build): one empty row, which Windhawk needs to know the list's shape (a list with
        # nothing under it fails the whole mod's settings). The part skips an empty row.
        if ($rows.Count -eq 0) {
            $lines.Add("  - - path: ''")
            $lines.Add("      `$name: Application name or path")
            $lines.Add("    - icon: ''")
            $lines.Add("      `$name: ICO file path")
        }
        $first = $true
        foreach ($row in $rows) {
            $index = [regex]::Match($row, '\[(\d+)\]').Groups[1].Value
            $lines.Add("  - - path: $(Q $values[$row])")
            if ($first) { $lines.Add("      `$name: Application name or path") }
            $lines.Add("    - icon: $(Q ([string]$values["icons[$index].icon"]))")
            if ($first) { $lines.Add("      `$name: ICO file path") }
            $first = $false
        }
        $lines.Add("  `$name: Icons")
    } else {
        $names = @(Sort-Indexed $values.Keys '^folders\[(\d+)\]\.name$')
        $first = $true
        if ($names.Count -eq 0) {  # (as above; an empty folder row is skipped too)
            $lines.Add("  - - name: ''")
            $lines.Add("      `$name: Folder name")
            $lines.Add("    - glyph: ''")
            $lines.Add("      `$name: Folder glyph")
            $lines.Add("      `$description: >-")
            $lines.Add("        Optional. A Segoe Fluent Icons code point such as U+E8B7, or any short")
            $lines.Add("        text. Leave empty to show a preview of the first four icons inside.")
            $lines.Add("    - icons: [`"`"]")
            $lines.Add("      `$name: Icons")
            $lines.Add("      `$description: >-")
            $lines.Add("        Patterns matched against each icon's app name, executable name and")
            $lines.Add("        tooltip, ignoring case. Use * for any text and ? for one character,")
            $lines.Add("        e.g. Discord or *NVIDIA*.")
        }
        foreach ($folderName in $names) {
            $index = [regex]::Match($folderName, '\[(\d+)\]').Groups[1].Value
            $icons = Sort-Indexed $values.Keys "^folders\[$index\]\.icons\[(\d+)\]$" | ForEach-Object { Q $values[$_] }
            $lines.Add("  - - name: $(Q $values[$folderName])")
            if ($first) { $lines.Add("      `$name: Folder name") }
            $lines.Add("    - glyph: $(Q ([string]$values["folders[$index].glyph"]))")
            if ($first) {
                $lines.Add("      `$name: Folder glyph")
                $lines.Add("      `$description: >-")
                $lines.Add("        Optional. A Segoe Fluent Icons code point such as U+E8B7, or any short")
                $lines.Add("        text. Leave empty to show a preview of the first four icons inside.")
            }
            $lines.Add("    - icons: [$($icons -join ', ')]")
            if ($first) {
                $lines.Add("      `$name: Icons")
                $lines.Add("      `$description: >-")
                $lines.Add("        Patterns matched against each icon's app name, executable name and")
                $lines.Add("        tooltip, ignoring case. Use * for any text and ? for one character,")
                $lines.Add("        e.g. Discord or *NVIDIA*.")
            }
            $first = $false
        }
        $lines.Add("  `$name: Folders")
    }
    $lines
}

###################################################################### build

$settings = New-Object System.Collections.Generic.List[string]
$code = New-Object System.Collections.Generic.List[string]
# Tweaks' settings go last on the page: the long list of Windows changes shouldn't stand between
# someone and the parts they came for. Only the page order changes; the code keeps the parts' order.
foreach ($part in @($parts | Where-Object { $_.Prefix -ne 'tweaks' }) + @($parts | Where-Object { $_.Prefix -eq 'tweaks' })) {
    $settings.AddRange([string[]](Convert-Settings $part))
}
foreach ($part in $parts) {
    $code.Add((Convert-Part $part))
}

$template = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'suite-redirect.template.cpp')) -replace "`r`n", "`n"
$suite = $template.Replace('{{SETTINGS}}', ($settings -join "`n")).Replace('{{PARTS}}', ($code -join "`n"))
# This build's own id, name and version.
$suite = [regex]::Replace($suite, '(?m)^// @id\s+.*$', "// @id              $SuiteName")
$suite = [regex]::Replace($suite, '(?m)^// @name\s+.*$', "// @name            Tourne'Suite $(if ($Tester) { "(test build, b$BuildNumber)" } else { "b$BuildNumber" })")
$suite = [regex]::Replace($suite, '(?m)^// @version\s+.*$', "// @version         1.0.0-b$BuildNumber")
[IO.File]::WriteAllText($Output, $suite, (New-Object Text.UTF8Encoding $false))
if ($Tester) {
    # The rest of the kit: its guide, launchers, backup/reset and fill-in scripts, fonts, and the readers
    # those scripts share with this build (tweaks.def, Build-Tweaks.ps1, sab-defaults.ps1).
    Copy-Item -Path (Join-Path $PSScriptRoot 'tester-kit\*') -Destination $TesterOutput -Recurse -Force
    foreach ($file in 'Build-Tweaks.ps1', 'sab-defaults.ps1', 'tweaks.def') {
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot $file) -Destination (Join-Path $TesterOutput 'Setup') -Force
    }
    "Wrote the tester kit to $TesterOutput ($([math]::Round((Get-Item $Output).Length / 1KB)) KB suite)"
    return
}

# The same defaults as a .reg, in case Windhawk ever keeps only the first
# element of a multi-row default.
$reg = New-Object System.Collections.Generic.List[string]
$reg.Add('Windows Registry Editor Version 5.00')
$reg.Add('')
$reg.Add("[HKEY_LOCAL_MACHINE\SOFTWARE\Windhawk\Engine\Mods\$SuiteId\Settings]")
foreach ($part in $parts) {
    $reg.Add("`"$($part.Prefix).enabled`"=dword:0000000$(Get-Enabled $part)")
    $values = Get-Settings $part
    foreach ($name in ($values.Keys | Sort-Object)) {
        $v = $values[$name]
        $regName = "$($part.Prefix).$name".Replace('\', '\\').Replace('"', '\"')
        if ($v -is [int]) { $reg.Add(('"{0}"=dword:{1:x8}' -f $regName, $v)) }
        else { $reg.Add(('"{0}"="{1}"' -f $regName, ([string]$v).Replace('\', '\\').Replace('"', '\"'))) }
    }
}
$reg.Add('"systemIcons.icons.volume"=dword:00000001')
$reg.Add('"systemIcons.icons.bluetooth"=dword:00000001')
# Settings newer than the installed suite: Windhawk doesn't fill in defaults
# for a mod that's already installed. Only the ones not set yet.
$iconValues = Get-Settings ($parts | Where-Object { $_.Prefix -eq 'systemIcons' })
$newDefaults = [ordered]@{
    'batteries.trayIcons' = 1; 'batteries.inFlyouts' = 1; 'batteries.pollInterval' = '1min'; 'batteries.lowAlert' = 20
    'batteries.pictogram' = 1; 'batteries.chargingAnimation' = 1
    'batteries.vendors.alienware' = 1; 'batteries.vendors.razer' = 0; 'batteries.vendors.logitech' = 0
    'batteries.vendors.steelseries' = 0; 'batteries.vendors.hyperx' = 0; 'batteries.vendors.audeze' = 0
    'batteries.vendors.mchose' = 0; 'batteries.vendors.wlmouse' = 0; 'batteries.vendors.playstation' = 0
    'batteries.vendors.xbox' = 1; 'batteries.vendors.bluetooth' = 1
}
$newDefaults['flyoutWidth'] = 'auto'
$newDefaults['flyoutMaxWidth'] = 480
foreach ($name in $newDefaults.Keys) {
    if ($iconValues.ContainsKey($name)) { continue }
    $v = $newDefaults[$name]
    if ($v -is [string]) { $reg.Add(('"systemIcons.{0}"="{1}"' -f $name, $v)) }
    else { $reg.Add(('"systemIcons.{0}"=dword:{1:x8}' -f $name, $v)) }
}
[IO.File]::WriteAllText((Join-Path $buildsDir "$SuiteName-settings.reg"), ($reg -join "`r`n") + "`r`n", [Text.Encoding]::Unicode)

# Every installed setting copied one for one, the private calendar link too, for this PC only: in
# builds\private, which git ignores. Imported after installing the build, it makes it the one before.
if ($script:sourceKey) {
    $private = Join-Path $buildsDir 'private'
    New-Item -ItemType Directory -Force $private | Out-Null
    $mine = New-Object System.Collections.Generic.List[string]
    $mine.Add('Windows Registry Editor Version 5.00')
    $mine.Add('')
    $mine.Add("[HKEY_LOCAL_MACHINE\SOFTWARE\Windhawk\Engine\Mods\$SuiteId\Settings]")
    $all = [ordered]@{}
    $names = $script:sourceKey.GetValueNames()
    foreach ($name in ($names | Sort-Object)) {
        $v = $script:sourceKey.GetValue($name)
        $v = if ($script:sourceKey.GetValueKind($name) -eq 'DWord') { [int]$v } else { [string]$v }
        # A tweak saved under its old group goes in under its new one.
        if ($name.StartsWith('tweaks.')) {
            $now = 'tweaks.' + (Get-TweakName $name.Substring(7))
            if ($now -ne $name) {
                if ($names -notcontains $now) { $all[$now] = $v }
                continue
            }
        }
        $all[$name] = $v
    }
    foreach ($name in $script:changes.Keys) { $all[$name] = $script:changes[$name] }
    foreach ($name in $all.Keys) {
        $v = $all[$name]
        $regName = $name.Replace('\', '\\').Replace('"', '\"')
        if ($v -is [int]) { $mine.Add(('"{0}"=dword:{1:x8}' -f $regName, $v)) }
        else { $mine.Add(('"{0}"="{1}"' -f $regName, ([string]$v).Replace('\', '\\').Replace('"', '\"'))) }
    }
    [IO.File]::WriteAllText((Join-Path $private "$SuiteName-my-settings.reg"), ($mine -join "`r`n") + "`r`n", [Text.Encoding]::Unicode)
}

"Wrote $Output ($([math]::Round((Get-Item $Output).Length / 1KB)) KB)"
