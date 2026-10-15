<#
Builds Tourne Desktop's plugin index (desktop\plugins\index.json) from Noctalia's plugin catalogues.

    .\tools\plugins\Make-PluginIndex.ps1              # both catalogues at their newest commit
    .\tools\plugins\Make-PluginIndex.ps1 -Keep        # keep the downloaded repos in the scratch folder

Each plugin's scripts are scanned, not trusted from its tags:
- Kept: it runs no programs, or only ones Tourne answers itself (xdg-open, sleep, true, noctalia msg) or
  ones that also exist on Windows (curl, git, gh; the card says it needs them).
- Excluded, with the reason: it runs a Linux program (hyprctl, nmcli, playerctl...), builds a command line
  the scan can't read, asks for flatpak or portals, reads Linux paths (/proc, ~/.config...), or needs a
  newer plugin API than Tourne answers.
For the kept ones it records what the plugin needs (internet and which hosts, the clipboard, notifications,
programs it runs...), which the Studio shows before install, and every file with the commit it came from,
so an install fetches exactly what was scanned.
#>
param(
    [string]$Out = (Join-Path (Split-Path (Split-Path $PSScriptRoot)) 'desktop\plugins\index.json'),
    [int]$MaxApi = 32,  # the newest plugin_api Tourne answers (noctalia.d.luau's newest calls, 2026-10-06)
    [switch]$Keep
)
$ErrorActionPreference = 'Stop'
$work = Join-Path ([IO.Path]::GetTempPath()) 'tourne-plugin-index'
New-Item -ItemType Directory -Force $work | Out-Null

# Programs a plugin may run: answered by Tourne itself, or found on Windows too.
$answered = @{ 'xdg-open' = 'Opens links and files'; 'sleep' = ''; 'true' = ''; 'noctalia' = '' }
$onWindows = @{ 'curl' = 'curl (built into Windows)'; 'git' = 'Git'; 'gh' = 'GitHub CLI (gh)' }

function Get-Repo([string]$name) {
    $sha = (Invoke-RestMethod "https://api.github.com/repos/noctalia-dev/$name/commits/main").sha
    $dir = Join-Path $work $name
    if (Test-Path $dir) { Remove-Item -Recurse -Force $dir }
    New-Item -ItemType Directory $dir | Out-Null
    $tgz = Join-Path $work "$name.tgz"
    Invoke-WebRequest "https://codeload.github.com/noctalia-dev/$name/tar.gz/$sha" -OutFile $tgz
    tar -xzf $tgz -C $dir --strip-components=1
    if ($LASTEXITCODE) { throw "couldn't unpack $name" }
    @{ Name = $name; Sha = $sha; Dir = $dir }
}

# Just enough TOML for plugin.toml: top-level keys, and the [[table]] headers (entries) in order.
function Read-Manifest([string]$path) {
    $top = [ordered]@{}
    $entries = New-Object System.Collections.Generic.List[object]
    $current = $null
    foreach ($raw in [IO.File]::ReadAllLines($path)) {
        $line = $raw.Trim()
        if ($line -eq '' -or $line.StartsWith('#')) { continue }
        if ($line -match '^\[\[([a-z_]+)\]\]$') {
            $current = [ordered]@{ kind = $Matches[1] }
            $entries.Add($current)
            continue
        }
        if ($line -match '^\[\[([a-z_]+)\.') { $current = $null; continue }  # (an entry's setting: read at install)
        if ($line -match '^\[') { $current = $null; continue }
        if ($line -notmatch '^([A-Za-z_]+)\s*=\s*(.+)$') { continue }
        $key = $Matches[1]; $value = $Matches[2].Trim()
        # (a value can carry a trailing "# comment")
        if ($value -notmatch '^["\[]') { $value = $value -replace '\s*#.*$', '' }
        $parsed = if ($value -match '^"((?:[^"\\]|\\.)*)"') { $Matches[1] -replace '\\"', '"' }
                  elseif ($value -match '^\[(.*?)\]') { @([regex]::Matches($Matches[1], '"([^"]*)"') | ForEach-Object { $_.Groups[1].Value }) }
                  elseif ($value -match '^-?\d+$') { [int]$value }
                  elseif ($value -eq 'true' -or $value -eq 'false') { $value -eq 'true' }
                  else { $value }
        if ($current) { if ($entries[-1] -eq $current) { $current[$key] = $parsed } }
        elseif ($entries.Count -eq 0) { $top[$key] = $parsed }
    }
    @{ Top = $top; Entries = $entries }
}

# Luau without its comments (strings kept), so a URL or a command named in a comment isn't counted.
function Remove-Comments([string]$code) {
    $pattern = '("(?:[^"\\\n]|\\.)*"|''(?:[^''\\\n]|\\.)*''|`(?:[^`\\]|\\.)*`)|--\[(=*)\[[\s\S]*?\]\2\]|--[^\n]*'
    [regex]::Replace($code, $pattern, { param($m) if ($m.Groups[1].Success) { $m.Value } else { '' } })
}

# The first word of a command a call runs: { "prog", ... }, "prog args", `prog {x}`, or sh -c "prog ...".
function Get-Commands([string]$code) {
    $found = New-Object System.Collections.Generic.List[string]
    $dynamic = 0
    foreach ($m in [regex]::Matches($code, 'noctalia\.(runAsync|runStream|runInTerminal)\(\s*')) {
        $rest = $code.Substring($m.Index + $m.Length, [Math]::Min(400, $code.Length - $m.Index - $m.Length))
        if ($rest -match '^\{\s*["''`]([^"''`\s]+)["''`]\s*(?:,\s*["''`]-c["''`]\s*,\s*["''`]\s*([^\s"''`]+))?') {
            $prog = $Matches[1]
            if (($prog -match '(^|/)(ba|z|da)?sh$') -and $Matches[2]) { $prog = $Matches[2] }
        } elseif ($rest -match '^["''`]\s*([^\s"''`]+)') {
            $prog = $Matches[1]
            if ($prog -match '(^|/)(ba|z|da)?sh$' -and $rest -match '^["''`]\s*\S+\s+-c\s+[''"]\s*([^\s''"]+)') { $prog = $Matches[1] }
        } else { $dynamic++; continue }
        if ($prog -match '^[{$]') { $dynamic++; continue }
        $found.Add(($prog -replace '.*/', ''))
    }
    foreach ($m in [regex]::Matches($code, 'noctalia\.commandExists\(\s*["''`]([^"''`]+)["''`]')) { $found.Add($m.Groups[1].Value) }
    if ([regex]::IsMatch($code, 'noctalia\.commandExists\(\s*[^"''`\s]')) { $dynamic++ }
    @{ Commands = @($found | Sort-Object -Unique); Dynamic = $dynamic }
}

$repos = @((Get-Repo 'official-plugins'), (Get-Repo 'community-plugins'))
$kept = New-Object System.Collections.Generic.List[object]
$excluded = New-Object System.Collections.Generic.List[object]

foreach ($repo in $repos) {
    foreach ($dir in Get-ChildItem -Directory $repo.Dir | Where-Object { -not $_.Name.StartsWith('.') }) {
        $toml = Join-Path $dir.FullName 'plugin.toml'
        if (-not (Test-Path $toml)) { continue }
        $m = Read-Manifest $toml
        $t = $m.Top
        $files = @(Get-ChildItem -Recurse -File $dir.FullName | ForEach-Object { $_.FullName.Substring($dir.FullName.Length + 1).Replace('\', '/') })
        $code = (Get-ChildItem -Recurse -File -Filter *.luau $dir.FullName | ForEach-Object { Remove-Comments ([IO.File]::ReadAllText($_.FullName)) }) -join "`n"
        $id = if ($t.id) { $t.id } else { $dir.Name }
        $why = New-Object System.Collections.Generic.List[string]

        $api = if ($t.plugin_api) { [int]$t.plugin_api } else { 0 }
        if ($api -gt $MaxApi) { $why.Add("needs plugin API $api") }
        $c = Get-Commands $code
        $linux = @($c.Commands | Where-Object { -not $answered.ContainsKey($_) -and -not $onWindows.ContainsKey($_) })
        if ($linux.Count) { $why.Add('runs ' + ($linux -join ', ')) }
        if ($c.Dynamic) { $why.Add("builds $($c.Dynamic) command line(s) the scan can't read") }
        if ($code -match 'noctalia\.(processMatches|flatpakAppInstalled|portalAvailable)') { $why.Add('asks Linux for ' + $Matches[1]) }
        if ($code -match '["''`](/(proc|sys|dev|usr|etc|run|var)/|~/\.(config|local|cache))') { $why.Add('reads Linux paths (' + $Matches[1] + ')') }
        if ($code -match 'noctalia\.getenv\(\s*["''`](XDG_[A-Z_]+|WAYLAND_DISPLAY|HYPRLAND_INSTANCE_SIGNATURE|NIRI_SOCKET|SWAYSOCK)') { $why.Add('reads ' + $Matches[1]) }
        if ($code -match 'noctalia\.Mpris') { $why.Add('uses MPRIS (Linux media players)') }

        $entryKinds = @($m.Entries | ForEach-Object { $_.kind } | Where-Object { $_ -ne 'setting' } | Sort-Object -Unique)  # ([[setting]]: the plugin's own options, not a surface)
        if ($why.Count) {
            $excluded.Add([ordered]@{ id = $id; name = $t.name; why = ($why -join '; ') })
            continue
        }

        # What it needs, as the Studio says it before install.
        $needs = New-Object System.Collections.Generic.List[object]
        if ($code -match 'noctalia\.(http|httpStream|download)\b') {
            $hosts = @([regex]::Matches($code, 'https?://([A-Za-z0-9.-]+\.[A-Za-z]{2,})') | ForEach-Object { $_.Groups[1].Value.ToLower() } | Sort-Object -Unique)
            $needs.Add([ordered]@{ kind = 'internet'; hosts = $hosts })
        }
        foreach ($p in $c.Commands) {
            if ($onWindows.ContainsKey($p)) { $needs.Add([ordered]@{ kind = 'program'; program = $p; label = $onWindows[$p] }) }
            elseif ($answered[$p]) { $needs.Add([ordered]@{ kind = 'open' }) }
        }
        if ($code -match 'noctalia\.(writeFile|removeFile|renameFile|mkdirAll|download)\b') { $needs.Add([ordered]@{ kind = 'files' }) }
        if ($code -match 'noctalia\.clipboardText\b') { $needs.Add([ordered]@{ kind = 'clipboardRead' }) }
        if ($code -match 'noctalia\.copyToClipboard\b') { $needs.Add([ordered]@{ kind = 'clipboardWrite' }) }
        if ($code -match 'noctalia\.(notify|notifyError)\b') { $needs.Add([ordered]@{ kind = 'notify' }) }
        if ($code -match 'noctalia\.(systemStats|diskStats|cpuCores|diskMounts)\b') { $needs.Add([ordered]@{ kind = 'systemInfo' }) }
        if ($code -match 'noctalia\.(setWallpaper|setWallpaperEnabled|setWallpaperMask)\b') { $needs.Add([ordered]@{ kind = 'wallpaper' }) }
        if ($code -match 'noctalia\.sound\.') { $needs.Add([ordered]@{ kind = 'sound' }) }

        $kept.Add([ordered]@{
            id = $id; name = $t.name; version = $t.version; api = $api; author = $t.author; license = $t.license
            description = $t.description; tags = @($t.tags); icon = $t.icon
            entries = $entryKinds
            repo = "noctalia-dev/$($repo.Name)"; rev = $repo.Sha; path = $dir.Name; files = $files
            needs = $needs.ToArray()
        })
    }
}

$index = [ordered]@{
    made = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')
    sources = @($repos | ForEach-Object { [ordered]@{ repo = "noctalia-dev/$($_.Name)"; rev = $_.Sha } })
    plugins = @($kept.ToArray() | Sort-Object { $_.name })
    excluded = @($excluded.ToArray() | Sort-Object { $_.id })
}
New-Item -ItemType Directory -Force (Split-Path $Out) | Out-Null
[IO.File]::WriteAllText($Out, ($index | ConvertTo-Json -Depth 8), [Text.UTF8Encoding]::new($false))
"$($kept.Count) plugins kept, $($excluded.Count) excluded: $Out"
if (-not $Keep) { Remove-Item -Recurse -Force $work }
