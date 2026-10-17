<#
Generates the Tweaks part (tourne-tweaks.wh.cpp) from tweaks.def and
tourne-tweaks.engine.cpp. Each setting's default is read from this PC, so
installing the part changes nothing, and tourne-tweaks-settings.reg holds the
same values for importing into the suite's settings.

Build-TourneSuiteRedirect.ps1 runs this first; it can also be run alone.
#>
param(
    [string]$Definitions = (Join-Path $PSScriptRoot 'tweaks.def'),
    [string]$Engine = (Join-Path $PSScriptRoot 'tourne-tweaks.engine.cpp'),
    [string]$Output = (Join-Path $PSScriptRoot 'tourne-tweaks.wh.cpp'),
    [string]$RegOutput = (Join-Path $PSScriptRoot 'tourne-tweaks-settings.reg'),
    [string]$SuiteId = 'local@tourne-suite-redirect-branch-wintweaks',
    # Defaults as Windows ships them, not read from this PC (the tester build).
    [switch]$WindowsDefaults,
    # Only the .reg, read from this PC (the tester kit runs this on the tester's PC).
    [switch]$RegOnly,
    # Every op any tweak can run, macros expanded, one per line (the tester kit's backup reads it), and nothing else.
    [switch]$ListOps
)

$ErrorActionPreference = 'Stop'

##################################################################### parse

function Expand-Macros([string]$line, $macros) {
    # Longest names first, so $PSYS wins over $PS; repeated for nested defines.
    for ($pass = 0; $pass -lt 5; $pass++) {
        $before = $line
        foreach ($name in $macros.Keys | Sort-Object Length -Descending) {
            $line = $line.Replace('$' + $name, $macros[$name])
        }
        if ($line -eq $before) { break }
    }
    $line
}

function Read-Definitions([string]$path) {
    $macros = [ordered]@{}
    $groups = New-Object System.Collections.Generic.List[object]
    $group = $null; $tweak = $null; $block = $null
    foreach ($raw in [IO.File]::ReadAllLines($path)) {
        $line = $raw.TrimEnd()
        if ($line -eq '' -or $line -match '^#') { continue }
        if ($line -match '^define (\w+) (.*)$') { $macros[$matches[1]] = $matches[2]; continue }
        $line = Expand-Macros $line $macros

        if ($line -match '^group (\w+) "(.*)"$') {
            $group = [pscustomobject]@{ Id = $matches[1]; Title = $matches[2]; Description = @(); Tweaks = New-Object System.Collections.Generic.List[object] }
            $groups.Add($group); $tweak = $null; $block = $null
            continue
        }
        if ($line -match '^(toggle|number|text|choice) (\w+) "((?:[^"\\]|\\.)*)"\s*(.*)$') {
            $attrs = @{}
            foreach ($m in [regex]::Matches($matches[4], '(\w[\w-]*)(?:=("(?:[^"\\]|\\.)*"|\S+))?')) {
                $v = $m.Groups[2].Value
                if ($v.StartsWith('"')) { $v = $v.Substring(1, $v.Length - 2).Replace('\"', '"') }
                $attrs[$m.Groups[1].Value] = if ($m.Groups[2].Success) { $v } else { $true }
            }
            $tweak = [pscustomobject]@{
                Kind = $matches[1]; Id = $matches[2]; Key = "$($group.Id).$($matches[2])"
                Title = $matches[3].Replace('\"', '"'); Attrs = $attrs; Description = @(); Uses = @()
                On = @(); Off = @(); Set = @(); Unset = @(); Detect = New-Object System.Collections.Generic.List[object]
                Options = New-Object System.Collections.Generic.List[object]; Read = $null
                DetectChoice = $null; ChoiceMap = New-Object System.Collections.Generic.List[object]; ChoiceDefault = $null
            }
            $group.Tweaks.Add($tweak); $block = $null
            continue
        }
        if ($line -match '^\| ?(.*)$') {
            if ($tweak) { $tweak.Description += $matches[1] } else { $group.Description += $matches[1] }
            continue
        }
        if ($line -match '^uses: (.*)$') { $tweak.Uses = $matches[1] -split '\s+'; continue }
        if ($line -match '^read: (.*)$') { $tweak.Read = $matches[1]; continue }
        if ($line -match '^default: (\S+)$') { $tweak.ChoiceDefault = $matches[1]; continue }
        if ($line -match '^detect-choice: (.*)$') { $tweak.DetectChoice = $matches[1]; $block = 'map'; continue }
        if ($line -match '^(on|off|set|unset):$') { $block = $matches[1]; continue }
        if ($line -eq 'detect:') { $tweak.Detect.Add((New-Object System.Collections.Generic.List[string])); $block = 'detect'; continue }
        if ($line -match '^option (\S+) "((?:[^"\\]|\\.)*)":$') {
            $tweak.Options.Add([pscustomobject]@{ Key = $matches[1]; Label = $matches[2].Replace('\"', '"'); Ops = New-Object System.Collections.Generic.List[string] })
            $block = 'option'
            continue
        }
        if ($line -match '^\s+(.*)$') {
            $op = $matches[1]
            switch ($block) {
                'on' { $tweak.On += $op }
                'off' { $tweak.Off += $op }
                'set' { $tweak.Set += $op }
                'unset' { $tweak.Unset += $op }
                'detect' { $tweak.Detect[$tweak.Detect.Count - 1].Add($op) }
                'option' { $tweak.Options[$tweak.Options.Count - 1].Ops.Add($op) }
                'map' {
                    if ($op -match '^(.+?) = (\S+)$') { $tweak.ChoiceMap.Add([pscustomobject]@{ From = $matches[1]; To = $matches[2] }) }
                }
                default { throw "Op outside a block in $($tweak.Key): $op" }
            }
            continue
        }
        throw "Can't read line: $line"
    }
    $groups
}

################################################################# registry

Add-Type -Namespace TourneTweaks -Name Native -MemberDefinition @'
[DllImport("user32.dll")] public static extern bool SystemParametersInfoW(uint action, uint ui, out uint pv, uint flags);
'@

function Open-Key([string]$path) {
    $root, $sub = $path -split '\\', 2
    $hive = switch ($root) {
        'HKCU' { [Microsoft.Win32.RegistryHive]::CurrentUser }
        'HKLM' { [Microsoft.Win32.RegistryHive]::LocalMachine }
        'HKU' { [Microsoft.Win32.RegistryHive]::Users }
        default { return $null }
    }
    $base = [Microsoft.Win32.RegistryKey]::OpenBaseKey($hive, [Microsoft.Win32.RegistryView]::Registry64)
    if (-not $sub) { return $base }
    try { $base.OpenSubKey($sub) } catch { $null }
}

# @{ Exists; Value } for a value; name @ is the default value.
function Get-RegValue([string]$path, [string]$name) {
    $key = Open-Key $path
    if (-not $key) { return @{ Exists = $false; KeyExists = $false } }
    $n = if ($name -eq '@') { '' } else { $name }
    $exists = $key.GetValueNames() -contains $n
    $value = if ($exists) { $key.GetValue($n, $null, 'DoNotExpandEnvironmentNames') } else { $null }
    $key.Close()
    @{ Exists = $exists; KeyExists = $true; Value = $value }
}

function Test-KeyExists([string]$path) {
    $key = Open-Key $path
    if ($key) { $key.Close(); return $true }
    $false
}

function ConvertTo-UInt32($v) {
    if ($v -is [int]) { return [uint32]([BitConverter]::ToUInt32([BitConverter]::GetBytes([int]$v), 0)) }
    if ($v -is [long]) { return [uint32]($v -band 0xFFFFFFFF) }
    if ($v -is [string]) {
        $s = $v.Trim()
        if ($s -match '^0x([0-9a-f]+)$') { return [uint32][Convert]::ToUInt32($matches[1], 16) }
        $n = 0L
        if ([long]::TryParse($s, [ref]$n)) { return [uint32]($n -band 0xFFFFFFFF) }
    }
    $null
}

# Whether one op already holds on this PC: $true, $false, or $null when it
# can't be told from the registry (commands, live calls).
function Test-Op([string]$op) {
    if ($op -match '^spiget (0x[0-9a-fA-F]+)$') {
        $v = [uint32]0
        [void][TourneTweaks.Native]::SystemParametersInfoW([Convert]::ToUInt32($matches[1], 16), 0, [ref]$v, 0)
        return $v -ne 0
    }
    if ($op -match '^localuser (\S+)$') {
        $user = Get-LocalUser -Name $matches[1] -ErrorAction SilentlyContinue
        return [bool]($user -and $user.Enabled)
    }
    if ($op -notmatch '^(HK\w+\\.*?) \| (.*)$') { return $null }
    $path = $matches[1]; $spec = $matches[2]
    if ($spec -eq '-') { return -not (Test-KeyExists $path) }
    if ($spec -eq '+') { return Test-KeyExists $path }
    if ($spec -match '^(.*) = (\w+):(.*)$') {
        $name = $matches[1]; $type = $matches[2]; $data = $matches[3]
        # Placeholders ({}, {@setting}, {twips} ...) depend on the setting itself.
        if ($data -match '\{(\}|@|twips\}|ms\}|drives\}|abgr\}|cmd\})') { return $null }
        $r = Get-RegValue $path $name
        if (-not $r.Exists) { return $false }
        switch ($type) {
            'any' { return $true }
            'dword' { return (ConvertTo-UInt32 $r.Value) -eq (ConvertTo-UInt32 $data) }
            'qword' { return [uint64]$r.Value -eq [uint64]$data }
            default { return [string]::Equals([string]$r.Value, $data, [StringComparison]::OrdinalIgnoreCase) }
        }
    }
    if ($spec -match '^(.*) -$') { return -not (Get-RegValue $path $matches[1]).Exists }
    $null
}

# All ops hold ($true), one doesn't ($false), or none could be checked ($null).
function Test-Ops($ops) {
    $checked = 0
    foreach ($op in $ops) {
        $r = Test-Op $op
        if ($null -eq $r) { continue }
        if (-not $r) { return $false }
        $checked++
    }
    if ($checked) { $true } else { $null }
}

function Read-LogFont([byte[]]$b) {
    if (-not $b -or $b.Length -lt 92) { return '' }
    $height = [BitConverter]::ToInt32($b, 0)
    $weight = [BitConverter]::ToInt32($b, 16)
    $italic = $b[20]
    $face = [Text.Encoding]::Unicode.GetString($b, 28, 64).Split([char]0)[0]
    $parts = @($face, [string][math]::Abs($height))
    if ($weight -ge 600) { $parts += 'bold' }
    if ($italic) { $parts += 'italic' }
    $parts -join ', '
}

function Read-Folders([string]$parent) {
    $ns = "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\$parent\NameSpace"
    $key = Open-Key $ns
    if (-not $key) { return '' }
    $rows = foreach ($guid in $key.GetSubKeyNames()) {
        $cls = "HKCU\Software\Classes\CLSID\$guid"
        $by = (Get-RegValue "$ns\$guid" 'CreatedBy').Value
        $classBy = (Get-RegValue $cls 'CreatedBy').Value
        if (@($by, $classBy) -notcontains 'Winaero Tweaker' -and @($by, $classBy) -notcontains "Tourne'Suite") { continue }
        $name = (Get-RegValue $cls '@').Value
        $target = (Get-RegValue "$cls\Instance\InitPropertyBag" 'TargetFolderPath').Value
        if (-not $target) { $target = (Get-RegValue "$cls\Instance\InitPropertyBag" 'Target').Value }
        $icon = (Get-RegValue "$cls\DefaultIcon" '@').Value
        if ($name -and $target) { (@($name, $target) + @($icon | Where-Object { $_ })) -join ' > ' }
    }
    $key.Close()
    @($rows) -join ' | '
}

# A number or text setting's current value, or $null for "not set here".
function Read-Current($tweak) {
    $spec = $tweak.Read
    if (-not $spec) { return $null }
    if ($spec -eq 'accents') {
        $colors = foreach ($i in 0..7) {
            $v = (Get-RegValue "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Accents\0\Theme$i" 'Color').Value
            if ($null -ne $v) { $u = ConvertTo-UInt32 $v; '#{0:X2}{1:X2}{2:X2}' -f ($u -band 0xFF), (($u -shr 8) -band 0xFF), (($u -shr 16) -band 0xFF) }
        }
        return (@($colors) -join ', ')
    }
    if ($spec -eq 'driveicons') {
        $key = Open-Key 'HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons'
        if (-not $key) { return '' }
        $pairs = foreach ($letter in $key.GetSubKeyNames() | Sort-Object) {
            $icon = (Get-RegValue "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons\$letter\DefaultIcon" '@').Value
            if ($icon) { "$letter=$icon" }
        }
        return (@($pairs) -join '|')
    }
    if ($spec -match '^folders (\w+)$') { return Read-Folders $matches[1] }
    if ($spec -match '^spiget (0x[0-9a-fA-F]+)$') {
        $v = [uint32]0
        [void][TourneTweaks.Native]::SystemParametersInfoW([Convert]::ToUInt32($matches[1], 16), 0, [ref]$v, 0)
        return [string]$v
    }
    if ($spec -notmatch '^(HK\w+\\.*?) \| (.*?)(?: as=(\w+))?$') { throw "Bad read: $spec" }
    $r = Get-RegValue $matches[1] $matches[2]
    $as = $matches[3]
    if (-not $r.Exists) { return $null }
    $v = $r.Value
    switch ($as) {
        'twips' { return [string][math]::Round(-[double][string]$v / 15) }
        'ms' { return [string][math]::Round([double](ConvertTo-UInt32 $v) / 1000) }
        'abgr' { $u = ConvertTo-UInt32 $v; return '{0:X2}{1:X2}{2:X2}' -f ($u -band 0xFF), (($u -shr 8) -band 0xFF), (($u -shr 16) -band 0xFF) }
        'drives' { $u = ConvertTo-UInt32 $v; return ((0..25 | Where-Object { $u -band (1 -shl $_) } | ForEach-Object { [string][char](65 + $_) }) -join ',') }
        'logfont' { return Read-LogFont $v }
        'shellicon' {
            if ([string]$v -match 'blank\.ico$') { return 'none' }
            if ([string]$v -match 'shell32\.dll,-30$') { return 'classic' }
            return [string]$v
        }
    }
    if ($v -is [byte[]]) { return $null }
    [string]$v
}

# The setting's value on this PC, in the form the settings store it.
function Get-Default($tweak) {
    if ($WindowsDefaults) {
        switch ($tweak.Kind) {
            'toggle' { return $tweak.Attrs['default'] -eq 'true' }
            'number' {
                $n = [int]$tweak.Attrs['default']
                if ($tweak.Attrs.ContainsKey('min') -and $n -lt [int]$tweak.Attrs['min']) { $n = [int]$tweak.Attrs['min'] }
                if ($tweak.Attrs.ContainsKey('max') -and $n -gt [int]$tweak.Attrs['max']) { $n = [int]$tweak.Attrs['max'] }
                return $n
            }
            'text' { return $(if ($tweak.Attrs.ContainsKey('default')) { [string]$tweak.Attrs['default'] } else { '' }) }
            'choice' { return $(if ($tweak.ChoiceDefault) { $tweak.ChoiceDefault } else { $tweak.Options[0].Key }) }
        }
    }
    switch ($tweak.Kind) {
        'toggle' {
            $alternatives = New-Object System.Collections.Generic.List[object]
            if ($tweak.Detect.Count) { foreach ($d in $tweak.Detect) { $alternatives.Add([string[]]$d) } }
            else { $alternatives.Add([string[]]$tweak.On) }
            $unknown = $true
            foreach ($ops in $alternatives) {
                $r = Test-Ops $ops
                if ($r) { return $true }
                if ($null -ne $r) { $unknown = $false }
            }
            if ($unknown -and $tweak.Attrs.ContainsKey('default')) { return $tweak.Attrs['default'] -eq 'true' }
            return $false
        }
        'number' {
            $v = Read-Current $tweak
            $n = 0
            if ($null -eq $v -or -not [int]::TryParse(([string]$v).Trim(), [ref]$n)) { $n = [int]$tweak.Attrs['default'] }
            # Windhawk refuses a default outside $min/$max; a value this PC
            # reports outside it (0% hibernation file with hibernation off)
            # starts at the nearest end.
            if ($tweak.Attrs.ContainsKey('min') -and $n -lt [int]$tweak.Attrs['min']) { $n = [int]$tweak.Attrs['min'] }
            if ($tweak.Attrs.ContainsKey('max') -and $n -gt [int]$tweak.Attrs['max']) { $n = [int]$tweak.Attrs['max'] }
            return $n
        }
        'text' {
            if ($tweak.Attrs.ContainsKey('private')) { return '' }
            $v = Read-Current $tweak
            if ($null -ne $v) { return [string]$v }
            if ($tweak.Attrs.ContainsKey('default')) { return [string]$tweak.Attrs['default'] }
            return ''
        }
        'choice' {
            if ($tweak.DetectChoice) {
                if ($tweak.DetectChoice -notmatch '^(HK\w+\\.*?) \| (.*)$') { throw "Bad detect-choice in $($tweak.Key)" }
                $path = $matches[1]; $name = $matches[2]
                $r = Get-RegValue $path $name
                foreach ($m in $tweak.ChoiceMap) {
                    if ($m.From -eq 'exists') { if ($r.Exists) { return $m.To } else { continue } }
                    if ($m.From -eq 'key') { if ($r.KeyExists) { return $m.To } else { continue } }
                    if (-not $r.Exists) { continue }
                    $u = ConvertTo-UInt32 $r.Value
                    if ($m.From -match '^\d+$' -and $null -ne $u -and $u -eq [uint32]$m.From) { return $m.To }
                    if ([string]$r.Value -like "*$($m.From)") { return $m.To }
                }
            } else {
                foreach ($option in $tweak.Options) {
                    if (Test-Ops $option.Ops) { return $option.Key }
                }
            }
            if ($tweak.ChoiceDefault) { return $tweak.ChoiceDefault }
            return $tweak.Options[0].Key
        }
    }
}

##################################################################### write

function Q([string]$s) { "'" + $s.Replace("'", "''") + "'" }

# Whether any of the tweak's ops needs admin (machine-wide keys and commands).
function Test-Admin($tweak) {
    $ops = @($tweak.On) + @($tweak.Off) + @($tweak.Set) + @($tweak.Unset) + @($tweak.Options | ForEach-Object { $_.Ops })
    [bool]($ops | Where-Object { $_ -match '^(HKLM|HKU)\\|^admin |^(shellicon|driveicons|accents) ' } | Select-Object -First 1)
}

function Get-Note($tweak) {
    $text = ($tweak.Description -join ' ').Trim()
    $after = switch ($tweak.Attrs['after']) {
        'explorer' { 'Shows after Explorer restarts.' }
        'signout' { 'Takes effect after you sign out and back in.' }
        'restart' { 'Takes effect after a restart.' }
    }
    $parts = @($text, $after, $(if (Test-Admin $tweak) { 'Asks for admin.' })) | Where-Object { $_ }
    $parts -join ' '
}

# Every group but General opens with its own switch, off: nothing in a group
# is applied until it's on (tourne-tweaks.engine.cpp, GroupOn).
function Test-HasUseSwitch($group) { $group.Id -ne 'general' }
$UseTitle = 'Use these tweaks'
$UseNote = 'Off: this group changes nothing in Windows, and its tweaks stay hidden. ' +
    'Switching it on shows them and applies nothing by itself; change a tweak and Save to apply it.'

# When each tweak shows (Windhawk 2.0's $showIf; a group's own switch can't hide the group, so each
# tweak in it names the switch): only while its group is in use, and a setting that only some
# choices read (through "uses:") only while one of those choices is picked. Conditions name
# siblings, so a setting read from another group shows whenever its own group is in use.
function Get-ShowIf($groups) {
    $conditions = @{}
    foreach ($group in $groups) {
        $ids = @($group.Tweaks | ForEach-Object { $_.Id })
        foreach ($tweak in $group.Tweaks) {
            $when = [ordered]@{}
            if (Test-HasUseSwitch $group) { $when['use'] = 'true' }
            $conditions[$tweak.Key] = $when
        }
        foreach ($tweak in $group.Tweaks) {
            foreach ($used in @($tweak.Uses | Where-Object { $_ })) {
                $id = $used.Split('.', 2)[1]
                if (-not $used.StartsWith("$($group.Id).") -or $ids -notcontains $id) { continue }
                $ref = [regex]::Escape("{@$used}")
                $value = $null
                if ($tweak.Kind -eq 'choice') {
                    $reading = @($tweak.Options | Where-Object { ($_.Ops -join "`n") -match $ref } | ForEach-Object { $_.Key })
                    if ($reading.Count -and $reading.Count -lt $tweak.Options.Count) { $value = '[' + ($reading -join ', ') + ']' }
                } elseif ($tweak.Kind -eq 'toggle') {
                    $on = (@($tweak.On) + @($tweak.Set)) -join "`n"
                    $off = (@($tweak.Off) + @($tweak.Unset)) -join "`n"
                    if ($on -match $ref -and $off -notmatch $ref) { $value = 'true' }
                }
                if ($value) { $conditions["$($group.Id).$id"][$tweak.Id] = $value }
            }
        }
    }
    $conditions
}

function Write-Yaml($groups, $defaults) {
    $out = New-Object System.Collections.Generic.List[string]
    $showIf = Get-ShowIf $groups
    foreach ($group in $groups) {
        $out.Add("- $($group.Id):")
        if (Test-HasUseSwitch $group) {
            $out.Add('  - use: false')
            $out.Add("    `$name: $(Q $UseTitle)")
            $out.Add("    `$description: $(Q $UseNote)")
        }
        foreach ($tweak in $group.Tweaks) {
            $d = $defaults[$tweak.Key]
            $value = switch ($tweak.Kind) {
                'toggle' { if ($d) { 'true' } else { 'false' } }
                'number' { [string]$d }
                default { Q ([string]$d) }
            }
            $out.Add("  - $($tweak.Id): $value")
            $out.Add("    `$name: $(Q $tweak.Title)")
            $note = Get-Note $tweak
            if ($note) { $out.Add("    `$description: $(Q $note)") }
            # Windhawk 1.7.3 refuses these annotations, so they go behind the
            # "#! " marker, which 1.7.3 reads as a comment.
            if ($tweak.Kind -eq 'number') {
                if ($tweak.Attrs.ContainsKey('min')) { $out.Add("    #! `$min: $($tweak.Attrs['min'])") }
                if ($tweak.Attrs.ContainsKey('max')) { $out.Add("    #! `$max: $($tweak.Attrs['max'])") }
            }
            if ($tweak.Attrs.ContainsKey('format')) { $out.Add("    #! `$format: $($tweak.Attrs['format'])") }
            if ($tweak.Kind -eq 'choice') {
                $out.Add('    $options:')
                foreach ($option in $tweak.Options) { $out.Add("    - $(Q $option.Key): $(Q $option.Label)") }
            }
            $when = $showIf[$tweak.Key]
            if ($when.Count) { $out.Add("    #! `$showIf: {$(($when.Keys | ForEach-Object { "${_}: $($when[$_])" }) -join ', ')}") }
        }
        $out.Add("  `$name: $(Q $group.Title)")
        if ($group.Description) { $out.Add("  `$description: $(Q (($group.Description -join ' ').Trim()))") }
    }
    $out
}

function C([string]$s) {
    if ($null -eq $s) { return 'nullptr' }
    $b = New-Object Text.StringBuilder
    [void]$b.Append('L"')
    # if/else, not switch: a PowerShell switch runs every case that matches.
    foreach ($ch in $s.ToCharArray()) {
        $code = [int]$ch
        if ($ch -ceq '\') { [void]$b.Append('\\') }
        elseif ($ch -ceq '"') { [void]$b.Append('\"') }
        elseif ($code -eq 10) { [void]$b.Append('\n') }
        elseif ($code -eq 0x1E) { [void]$b.Append('\036') }
        elseif ($code -eq 0x1F) { [void]$b.Append('\037') }
        elseif ($code -lt 32 -or $code -gt 126) { [void]$b.Append(('\u{0:x4}' -f $code)) }
        else { [void]$b.Append($ch) }
    }
    [void]$b.Append('"')
    $b.ToString()
}

function Write-Table($groups) {
    $rows = foreach ($group in $groups) {
        foreach ($tweak in $group.Tweaks) {
            $kind = switch ($tweak.Kind) { 'toggle' { 't' } 'number' { 'n' } 'text' { 's' } 'choice' { 'c' } }
            $after = switch ($tweak.Attrs['after']) { 'explorer' { "'e'" } 'signout' { "'s'" } 'restart' { "'r'" } default { '0' } }
            $keep = if ($tweak.Attrs.ContainsKey('keep-empty') -or $tweak.Attrs.ContainsKey('zero-off')) { 'true' } else { 'false' }
            $winDefault = if ($tweak.Kind -eq 'number') { C ([string]$tweak.Attrs['default']) } else { 'nullptr' }
            switch ($tweak.Kind) {
                'toggle' { $a = $tweak.On -join "`n"; $b = $tweak.Off -join "`n" }
                'choice' {
                    $a = ($tweak.Options | ForEach-Object { $_.Key + [char]0x1E + ($_.Ops -join "`n") }) -join [char]0x1F
                    $b = ''
                }
                default { $a = $tweak.Set -join "`n"; $b = $tweak.Unset -join "`n" }
            }
            $uses = ($tweak.Uses | Where-Object { $_ }) -join ' '
            $admin = if (Test-Admin $tweak) { 'true' } else { 'false' }
            "    {$(C $tweak.Key), $(C $tweak.Title), '$kind', $after, $admin, $keep, $winDefault,`n     $(C $a),`n     $(C $b),`n     $(C $uses)},"
        }
    }
    "constexpr Tweak kTweaks[] = {`n" + ($rows -join "`n") + "`n};"
}

###################################################################### build

$groups = Read-Definitions $Definitions
if ($ListOps) {
    foreach ($group in $groups) {
        foreach ($tweak in $group.Tweaks) {
            $tweak.On; $tweak.Off; $tweak.Set; $tweak.Unset
            foreach ($d in $tweak.Detect) { $d }
            foreach ($o in $tweak.Options) { $o.Ops }
        }
    }
    return
}
$defaults = [ordered]@{}
foreach ($group in $groups) {
    foreach ($tweak in $group.Tweaks) {
        $defaults[$tweak.Key] = Get-Default $tweak
    }
}

if (-not $RegOnly) {
    $yaml = Write-Yaml $groups $defaults
    $engine = [IO.File]::ReadAllText($Engine) -replace "`r`n", "`n"
    $part = $engine.Replace('{{TWEAKS_SETTINGS}}', ($yaml -join "`n")).Replace('{{TWEAKS_TABLE}}', (Write-Table $groups))
    [IO.File]::WriteAllText($Output, $part, (New-Object Text.UTF8Encoding $false))
}

# The same defaults as the suite's registry settings, for importing.
$reg = New-Object System.Collections.Generic.List[string]
$reg.Add('Windows Registry Editor Version 5.00')
$reg.Add('')
$reg.Add("[HKEY_LOCAL_MACHINE\SOFTWARE\Windhawk\Engine\Mods\$SuiteId\Settings]")
$reg.Add('"tweaks.enabled"=dword:00000001')
foreach ($group in $groups) {
    if (Test-HasUseSwitch $group) { $reg.Add(('"tweaks.{0}.use"=dword:00000000' -f $group.Id)) }
    foreach ($tweak in $group.Tweaks) {
        $name = "tweaks.$($tweak.Key)".Replace('\', '\\').Replace('"', '\"')
        $d = $defaults[$tweak.Key]
        switch ($tweak.Kind) {
            'toggle' { $reg.Add(('"{0}"=dword:{1:x8}' -f $name, [int][bool]$d)) }
            'number' { $reg.Add(('"{0}"=dword:{1:x8}' -f $name, [int]$d)) }
            default { $reg.Add(('"{0}"="{1}"' -f $name, ([string]$d).Replace('\', '\\').Replace('"', '\"'))) }
        }
    }
}
[IO.File]::WriteAllText($RegOutput, ($reg -join "`r`n") + "`r`n", [Text.Encoding]::Unicode)

$count = ($groups | ForEach-Object { $_.Tweaks.Count } | Measure-Object -Sum).Sum
"Tweaks: $count settings in $($groups.Count) groups -> $(if ($RegOnly) { $RegOutput } else { $Output })"
