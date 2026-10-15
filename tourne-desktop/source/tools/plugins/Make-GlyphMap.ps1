<#
Fetches the Tabler icons font (MIT) that Noctalia's plugins name their glyphs
from ("world", "puzzle"...), and writes desktop\plugins\tabler-icons.ttf and
desktop\plugins\tabler-glyphs.txt (one "name hex-codepoint" per line). Both
are built into Tourne Desktop, so plugins' glyphs need no download.

Noctalia also answers names of its own ("close" is Tabler's "x", "add" its
"plus"): its alias map (src/render/text/glyph_registry.cpp, MIT) is read at the
commit below and its names added with their Tabler glyph's codepoint.

    .\tools\plugins\Make-GlyphMap.ps1                  # the versions below
    .\tools\plugins\Make-GlyphMap.ps1 -Version 3.50.0
#>
param(
    [string]$Version = '3.49.0',
    [string]$NoctaliaCommit = 'd698f30c5905ded5b060e819c13e891ae6dc6296'
)
$ErrorActionPreference = 'Stop'
$out = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) 'desktop\plugins'
$base = "https://cdn.jsdelivr.net/npm/@tabler/icons-webfont@$Version/dist"

Invoke-WebRequest "$base/fonts/tabler-icons.ttf" -OutFile (Join-Path $out 'tabler-icons.ttf') -UseBasicParsing
$css = (Invoke-WebRequest "$base/tabler-icons.css" -UseBasicParsing).Content
$codes = [ordered]@{}
foreach ($m in [regex]::Matches($css, '\.ti-([a-z0-9-]+):before\s*\{\s*content:\s*"\\([0-9a-f]+)"')) { $codes[$m.Groups[1].Value] = $m.Groups[2].Value }

$registry = (Invoke-WebRequest "https://raw.githubusercontent.com/noctalia-dev/noctalia/$NoctaliaCommit/src/render/text/glyph_registry.cpp" -UseBasicParsing).Content
$aliases = 0
$block = [regex]::Match($registry, 'kAliases\s*=\s*\{(.*?)\n\};', 'Singleline').Groups[1].Value
foreach ($m in [regex]::Matches($block, '\{"([a-z0-9-]+)",\s*"([a-z0-9-]+)"\}')) {
    $alias, $target = $m.Groups[1].Value, $m.Groups[2].Value
    if (-not $codes.Contains($alias) -and $codes.Contains($target)) { $codes[$alias] = $codes[$target]; $aliases++ }
}

$header = "# Tabler icons $Version (MIT, Pawel Kuna): glyph name, codepoint in tabler-icons.ttf; with Noctalia's aliases (MIT, at $($NoctaliaCommit.Substring(0, 7))). Made by tools\plugins\Make-GlyphMap.ps1."
$lines = foreach ($k in $codes.Keys) { "$k $($codes[$k])" }
[IO.File]::WriteAllText((Join-Path $out 'tabler-glyphs.txt'), (@($header) + ($lines | Sort-Object -Unique)) -join "`n")
"Wrote $($codes.Count) glyphs ($aliases of them Noctalia's aliases) and the font (Tabler icons $Version) to $out"
