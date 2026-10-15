<#
Builds Tourne Desktop (desktop\build\TourneDesktop.exe): the suite's modules
that need no hooks, as one program of its own, outside Windhawk.

Each module is the suite's own source, unchanged: its "// @inline" files are
put in, its includes move to one shared list, its code goes into a namespace
of its own with Windhawk's API answered by the host (host\module-shim.inl),
and its settings schema is kept as the source of its defaults.

It uses Windhawk's own compiler when Windhawk is installed, so nothing else
needs installing; otherwise clang++ or g++ (MinGW) on the PATH, or -Compiler.

    .\desktop\Build-TourneDesktop.ps1                    # every module
    .\desktop\Build-TourneDesktop.ps1 -Modules switcher  # only some
    .\desktop\Build-TourneDesktop.ps1 -Debug             # with symbols, no optimisation
#>
param(
    [string[]]$Modules = @('taskbar', 'switcher', 'systemIcons', 'iconRedirect', 'table'),
    [string]$Compiler = '',
    [switch]$Debug,
    [string]$Out = (Join-Path $PSScriptRoot 'build')
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$hostDir = Join-Path $PSScriptRoot 'host'
$gen = Join-Path $Out 'gen'
New-Item -ItemType Directory -Force $gen | Out-Null

# The modules that can run here: their suite source, the prefix their settings
# have in the suite (so imported settings fit), and their name.
$known = [ordered]@{
    taskbar  = @{ Prefix = 'taskbar';  Title = 'Taskbar';                                  Source = 'taskbar-host.cpp' }
    switcher = @{ Prefix = 'switcher'; Title = 'Switcher, tiling and borders';            Source = 'switcher-host.cpp' }
    systemIcons = @{ Prefix = 'systemIcons'; Title = 'System icons and flyouts';       Source = 'system-icons-host.cpp' }
    # Beside the suite: the suite's Icon Redirect does every other program, this copy Tourne Desktop's
    # own icons (the suite stays out of it), with the suite's settings. It hooks this process only.
    iconRedirect = @{ Prefix = 'iconRedirect'; Title = "Icon Redirect (Tourne Desktop's own icons)"; Source = 'tourne-icon-redirect.wh.cpp'; BesideSuite = $true }
    # A Windhawk tool mod (its own process there): built from WhTool_ModInit and the rest, without the
    # launcher at its end. Tool names the Windhawk mod it is, so it waits while that one is on there.
    table = @{ Prefix = 'table'; Title = "Tourne'Table visualizer"; Source = 'tourne-table.wh.cpp'; Tool = 'tourne-table' }
}

function Find-Compiler {
    if ($Compiler) { return $Compiler }
    $candidates = @()
    foreach ($root in @($env:ProgramFiles, ${env:ProgramFiles(x86)}, $env:LOCALAPPDATA) | Where-Object { $_ }) {
        $candidates += Join-Path $root 'Windhawk\Compiler\bin\clang++.exe'
        $candidates += Join-Path $root 'Windhawk\Compiler\bin\g++.exe'
    }
    foreach ($c in $candidates) { if (Test-Path -LiteralPath $c) { return $c } }
    foreach ($name in 'clang++', 'g++', 'x86_64-w64-mingw32-g++') {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }
    throw 'No compiler found. Install Windhawk (its compiler is used), or pass -Compiler with a MinGW clang++ or g++.'
}

# A source with its "// @inline file" lines replaced by those files (and theirs).
function Read-Expanded([string]$path, [System.Collections.Generic.HashSet[string]]$seen) {
    $full = (Resolve-Path -LiteralPath $path).Path
    [void]$seen.Add($full)
    $text = [IO.File]::ReadAllText($full) -replace "`r`n", "`n"
    $dir = Split-Path $full
    [regex]::Replace($text, '(?m)^// @inline (\S+)$', {
        param($m)
        $inner = Join-Path $dir $m.Groups[1].Value
        (Read-Expanded $inner $seen).TrimEnd("`n")
    })
}

$includes = New-Object System.Collections.Generic.List[string]
$libs = New-Object System.Collections.Generic.List[string]
# The suite's own list (the code inlined into a module can need more than the
# module's header names), so every module links.
foreach ($lib in '-lshell32', '-lgdi32', '-lcomctl32', '-lversion', '-ladvapi32', '-lole32', '-loleaut32',
                 '-lwlanapi', '-liphlpapi', '-lpowrprof', '-luuid', '-lmsimg32', '-ldxva2', '-luser32',
                 '-luxtheme', '-lwscapi', '-lcfgmgr32', '-ldwmapi', '-lshlwapi', '-lruntimeobject',
                 '-lurlmon', '-lwininet', '-lhid', '-lwindowscodecs', '-lwinhttp', '-lws2_32',
                 '-ld2d1', '-ldwrite', '-lcomdlg32', '-ld3d11', '-ldxgi', '-ldcomp', '-lsetupapi', '-lwinmm') { $libs.Add($lib) }
$table = New-Object System.Collections.Generic.List[string]
$decls = New-Object System.Collections.Generic.List[string]
$units = New-Object System.Collections.Generic.List[string]

foreach ($name in $Modules) {
    if (-not $known.Contains($name)) { throw "Unknown module '$name'. Known: $($known.Keys -join ', ')" }
    $m = $known[$name]
    $text = Read-Expanded (Join-Path $repo $m.Source) (New-Object System.Collections.Generic.HashSet[string])

    # Not -lwindowsapp (Tourne'Table asks for it): with it the compiler leaves kernel32 out, and the
    # parts that call kernel32 functions windowsapp lacks (Toolhelp, QueryDosDevice) stop linking.
    # -lruntimeobject gives C++/WinRT what it needs.
    $opts = [regex]::Match($text, '(?m)^// @compilerOptions\s+(.+)$')
    if ($opts.Success) { foreach ($o in ($opts.Groups[1].Value -split '\s+')) { if ($o -and $o -ne '-lwindowsapp' -and -not $libs.Contains($o)) { $libs.Add($o) } } }

    $schema = [regex]::Match($text, '// ==WindhawkModSettings==\s*/\*\n?(.*?)\*/\s*// ==/WindhawkModSettings==', 'Singleline')
    $yaml = if ($schema.Success) { $schema.Groups[1].Value } else { '' }
    $end = $text.IndexOf('// ==/WindhawkModSettings==')
    $body = if ($end -ge 0) { $text.Substring($end + '// ==/WindhawkModSettings=='.Length) } else { $text }
    # A tool mod: Windhawk's launcher (it starts windhawk.exe -tool-mod and hooks its entry point) stays out.
    $tool = [string]$m.Tool
    if ($tool) {
        $launcher = [regex]::Match($body, '(?m)^/{20,}\n// Windhawk tool mod implementation')
        if (-not $launcher.Success) { throw "$name is a tool mod but its launcher wasn't found" }
        $body = $body.Substring(0, $launcher.Index)
    }

    # Includes move to the shared list, ahead of every module's namespace.
    $lines = $body -split "`n"
    $kept = New-Object System.Collections.Generic.List[string]
    foreach ($line in $lines) {
        if ($line -match '^\s*#\s*include\s*([<"][^>"]+[>"])') {
            $inc = $matches[1]
            if ($inc -notmatch 'windhawk_(api|utils)\.h' -and -not $includes.Contains($inc)) { $includes.Add($inc) }
            $kept.Add('')  # (keeps line numbers)
        } else {
            $kept.Add($line)
        }
    }
    # (WindhawkUtils' string settings and any-thread subclassing are stood in for by module-shim.inl.)
    # (a part beside the suite may hook, in this process only: host\hooks.cpp)
    $beside = [bool]$m.BesideSuite
    $helpers = 'StringSetting\b|SetWindowSubclassFromAnyThread\b|RemoveWindowSubclassFromAnyThread\b'
    $hooking = if ($beside) { "\bWindhawkUtils::(?!$helpers|SetFunctionHook\b)" } else { "\b(Wh_SetFunctionHook|WindhawkUtils::(?!$helpers))" }
    if ($body -match $hooking) {
        throw "$name hooks other programs or uses Windhawk's helpers: it belongs in the Windhawk suite."
    }
    # Windhawk's other form, which asks for a reload: rules a part reads once (Icon Redirect's) are
    # read again when Tourne Desktop restarts, and the log says so.
    $reloadForm = $body -match '\bBOOL\s+Wh_ModSettingsChanged\s*\(\s*BOOL\s*\*'
    if ($tool) {
        # Starts once per run (its uninit is written for a process that ends), and never beside
        # Windhawk's copy, which would draw a second one.
        $kept.Add('BOOL TD_ToolInit() {')
        $kept.Add('    static bool started = false;')
        $kept.Add("    if (td::WindhawkModOn(L`"$tool`")) {")
        $kept.Add('        Wh_Log(L"it is on in Windhawk: turn it off there first, so there is only one");')
        $kept.Add('        return FALSE;')
        $kept.Add('    }')
        $kept.Add('    if (started) {')
        $kept.Add('        Wh_Log(L"turned back on: it starts again when Tourne Desktop restarts");')
        $kept.Add('        return FALSE;')
        $kept.Add('    }')
        $kept.Add('    started = true;')
        $kept.Add('    return WhTool_ModInit();')
        $kept.Add('}')
    } elseif ($reloadForm) {
        $kept.Add('void TD_SettingsChanged() {')
        $kept.Add('    BOOL reload = FALSE;')
        $kept.Add('    Wh_ModSettingsChanged(&reload);')
        $kept.Add('    if (reload) Wh_Log(L"changed settings take effect here when Tourne Desktop restarts");')
        $kept.Add('}')
    }

    $unit = @"
// Generated by Build-TourneDesktop.ps1 from $($m.Source): don't edit.
#include "prelude.h"
namespace mod_$name {
#define TD_PREFIX L"$($m.Prefix)"
#include "module-shim.inl"
$($kept -join "`n")
#undef TD_PREFIX
}  // namespace mod_$name
"@
    $unitPath = Join-Path $gen "mod_$name.cpp"
    [IO.File]::WriteAllText($unitPath, $unit)
    $units.Add($unitPath)

    $hasAfter = -not $tool -and $body -match '\bvoid\s+Wh_ModAfterInit\s*\('
    $hasChanged = $body -match '\bvoid\s+Wh_ModSettingsChanged\s*\(\s*\)'
    $changedName = if ($tool) { 'WhTool_ModSettingsChanged' } elseif ($hasChanged) { 'Wh_ModSettingsChanged' } elseif ($reloadForm) { 'TD_SettingsChanged' } else { '' }
    $initName = if ($tool) { 'TD_ToolInit' } else { 'Wh_ModInit' }
    $uninitName = if ($tool) { 'WhTool_ModUninit' } else { 'Wh_ModUninit' }
    $decls.Add("namespace mod_$name { BOOL $initName(); void $uninitName();$(if ($hasAfter) { ' void Wh_ModAfterInit();' })$(if ($changedName) { " void $changedName();" }) }")
    # The schema as UTF-8 bytes, split so no compiler meets a literal too long.
    $bytes = [Text.Encoding]::UTF8.GetBytes($yaml)
    $chunks = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -lt $bytes.Length; $i += 2000) {
        $n = [Math]::Min(2000, $bytes.Length - $i)
        $chunks.Add(($bytes[$i..($i + $n - 1)] | ForEach-Object { $_.ToString() }) -join ',')
    }
    $decls.Add("static const unsigned char kSchema_$name[] = {$(($chunks -join ",`n")), 0};")
    $after = if ($hasAfter) { "mod_$name::Wh_ModAfterInit" } else { 'nullptr' }
    $changed = if ($changedName) { "mod_$name::$changedName" } else { 'nullptr' }
    $besideText = if ($beside) { 'true' } else { 'false' }
    $table.Add("    {L`"$($m.Prefix)`", L`"$($m.Title)`", (const char*)kSchema_$name, mod_$name::$initName, $after, mod_$name::$uninitName, $changed, $besideText},")
}

# Winsock (and its IP definitions, which iphlpapi.h needs first for its
# newer tables) must come before windows.h when a module uses it.
$early = '^<(winsock2|ws2ipdef)\.h>$'
$ordered = @($includes | Where-Object { $_ -match 'winsock2' }) + @($includes | Where-Object { $_ -match 'ws2ipdef' }) +
           @($includes | Where-Object { $_ -notmatch $early })
$prelude = @"
// Generated by Build-TourneDesktop.ps1: every module's includes, once.
#pragma once
#define WH_MOD_ID L"tourne-desktop"
$(($ordered | ForEach-Object { "#include $_" }) -join "`n")
#include <cstdarg>
#include "td.h"
#include "standalone.h"
"@
[IO.File]::WriteAllText((Join-Path $gen 'prelude.h'), $prelude)

# What one module gives the others and the host (standalone.h, tray-host.cpp): System Icons'
# designs, while it runs; nothing when it isn't built in.
$systemIcons = $Modules -contains 'systemIcons'
$designs = if ($systemIcons) { @"
namespace mod_systemIcons {
HICON DesignIconAt(std::wstring const& icon, int size);
HICON TrayDesignIcon(HWND owner, std::wstring const& tip);
void FlyoutPalette(uint32_t* out);
}
namespace part_system_icons {
HICON DesignIconAt(std::wstring const& icon, int size) {
    return td::ModuleRunning(L"systemIcons") ? mod_systemIcons::DesignIconAt(icon, size) : nullptr;
}
}
namespace td {
HICON TrayDesign(HWND owner, std::wstring const& tip) {
    return ModuleRunning(L"systemIcons") ? mod_systemIcons::TrayDesignIcon(owner, tip) : nullptr;
}
bool FlyoutPalette(uint32_t* out) {
    if (!ModuleRunning(L"systemIcons")) return false;
    mod_systemIcons::FlyoutPalette(out);
    return true;
}
}
"@ } else { @"
namespace part_system_icons {
HICON DesignIconAt(std::wstring const&, int) { return nullptr; }
}
namespace td {
HICON TrayDesign(HWND, std::wstring const&) { return nullptr; }
bool FlyoutPalette(uint32_t*) { return false; }
}
"@ }
$modulesCpp = @"
// Generated by Build-TourneDesktop.ps1: the modules built in.
#include <windows.h>
#include <string>
#include "td.h"
$($decls -join "`n")
namespace td {
const Module kModules[] = {
$($table -join "`n")
};
const int kModuleCount = $($Modules.Count);
}  // namespace td
$designs
"@
$modulesPath = Join-Path $gen 'modules.cpp'
[IO.File]::WriteAllText($modulesPath, $modulesCpp)

# Which build this is, for crash.txt: the commit and the time it was built.
$commit = (& git -C $hostDir rev-parse --short HEAD 2>$null)
if (-not $commit) { $commit = 'no git' }
$stamp = "$commit, built $((Get-Date).ToString('yyyy-MM-dd HH:mm'))"
[IO.File]::WriteAllText((Join-Path $gen 'build-stamp.h'), "#define TD_BUILD L`"$stamp`"`n")

# The plugin index (tools\plugins\Make-PluginIndex.ps1 makes it), as UTF-8 bytes for plugins.cpp.
$indexBytes = [IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'plugins\index.json'))
$indexChunks = New-Object System.Collections.Generic.List[string]
for ($i = 0; $i -lt $indexBytes.Length; $i += 2000) {
    $n = [Math]::Min(2000, $indexBytes.Length - $i)
    $indexChunks.Add(($indexBytes[$i..($i + $n - 1)] | ForEach-Object { $_.ToString() }) -join ',')
}
[IO.File]::WriteAllText((Join-Path $gen 'plugins-index.inc'),
    "// Generated by Build-TourneDesktop.ps1 from desktop\plugins\index.json.`nconst unsigned char kPluginIndex[] = {$(($indexChunks -join ",`n")), 0};`n")

# Tourne's own plugins (desktop\plugins\bundled): their index and every file of theirs, built in.
function ConvertTo-ByteList([byte[]]$bytes) {
    $chunks = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -lt $bytes.Length; $i += 2000) {
        $n = [Math]::Min(2000, $bytes.Length - $i)
        $chunks.Add(($bytes[$i..($i + $n - 1)] | ForEach-Object { $_.ToString() }) -join ',')
    }
    $chunks -join ",`n"
}
$bundledRoot = Join-Path $PSScriptRoot 'plugins\bundled'
$bundledIndex = Get-Content -Raw -LiteralPath (Join-Path $bundledRoot 'index.json') | ConvertFrom-Json
$bundled = New-Object System.Text.StringBuilder
[void]$bundled.Append("// Generated by Build-TourneDesktop.ps1 from desktop\plugins\bundled.`n")
[void]$bundled.Append("const unsigned char kBundledIndex[] = {$(ConvertTo-ByteList ([IO.File]::ReadAllBytes((Join-Path $bundledRoot 'index.json')))), 0};`n")
$entries = New-Object System.Collections.Generic.List[string]
$n = 0
foreach ($plugin in $bundledIndex.plugins) {
    $folder = (Get-Item -LiteralPath (Join-Path $bundledRoot ($plugin.id -replace '/', '\'))).FullName
    foreach ($file in Get-ChildItem -LiteralPath $folder -Recurse -File | Sort-Object FullName) {
        $rel = $file.FullName.Substring($folder.Length + 1) -replace '\\', '/'
        $bytes = [IO.File]::ReadAllBytes($file.FullName)
        $list = if ($bytes.Length) { ConvertTo-ByteList $bytes } else { '0' }
        [void]$bundled.Append("static const unsigned char kBundled$n[] = {$list};`n")
        $entries.Add("    {`"$($plugin.id)`", `"$rel`", kBundled$n, $($bytes.Length)},")
        $n++
    }
}
[void]$bundled.Append("const BundledFile kBundledFiles[] = {`n$($entries -join "`n")`n};`nconst size_t kBundledFileCount = $n;`n")
[IO.File]::WriteAllText((Join-Path $gen 'plugins-bundled.inc'), $bundled.ToString())

# The UI sounds (desktop\sounds\foley\*.wav), built in: ui-sounds.cpp plays them by name.
$sounds = New-Object System.Text.StringBuilder
[void]$sounds.Append("// Generated by Build-TourneDesktop.ps1 from desktop\sounds\foley.`n")
$soundEntries = New-Object System.Collections.Generic.List[string]
$n = 0
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'sounds\foley') -Filter *.wav | Sort-Object Name) {
    $bytes = [IO.File]::ReadAllBytes($file.FullName)
    [void]$sounds.Append("static const unsigned char kUiSound$n[] = {$(ConvertTo-ByteList $bytes)};`n")
    $soundEntries.Add("    {`"$($file.BaseName)`", kUiSound$n, $($bytes.Length)},")
    $n++
}
[void]$sounds.Append("const BuiltInSound kUiSounds[] = {`n$($soundEntries -join "`n")`n};`n")
[IO.File]::WriteAllText((Join-Path $gen 'ui-sounds.inc'), $sounds.ToString())

$cxx = Find-Compiler
"Compiler: $cxx"
$flags = @('-std=c++23', '-DUNICODE', '-D_UNICODE', '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00', '-municode',
           "-I$hostDir", "-I$gen", "-I$repo", '-Wno-attributes', '-Wno-unknown-pragmas')
$flags += if ($Debug) { @('-O0', '-g') } else { @('-O2') }
# Windhawk's clang++ makes 32-bit programs unless told otherwise; Tourne Desktop is 64-bit.
$target = @()
if ([IO.Path]::GetFileNameWithoutExtension($cxx) -match 'clang') { $target = @('--target=x86_64-w64-mingw32') }
$flags += $target
# The Luau VM and compiler, for the plugin runtime (compiled once and kept: Build-Luau.ps1).
$luau = Join-Path $PSScriptRoot 'third_party\luau'
$flags += @("-I$luau\VM\include", "-I$luau\Compiler\include", "-I$luau\Common\include")
$luauObjects = & (Join-Path $PSScriptRoot 'Build-Luau.ps1') -Compiler $cxx -Out $gen -Target $target -Symbols:$Debug
# MinHook, for hooks.cpp: C, compiled as C by the same compiler, and kept while newer than its source.
$minhook = Join-Path $PSScriptRoot 'third_party\minhook'
$flags += "-I$minhook\include"
$cFlags = @('-x', 'c', '-std=c11', '-w') + $target + $(if ($Debug) { @('-O0', '-g') } else { @('-O2') })
$minhookObjects = foreach ($c in Get-ChildItem -Recurse -Filter *.c -Path (Join-Path $minhook 'src')) {
    $obj = Join-Path $gen "minhook-$($c.BaseName).o"
    if (-not (Test-Path $obj) -or (Get-Item $obj).LastWriteTime -lt $c.LastWriteTime) {
        & $cxx @cFlags -c $c.FullName -o $obj
        if ($LASTEXITCODE) { throw "Compiling $($c.Name) failed" }
    }
    $obj
}

$objects = New-Object System.Collections.Generic.List[string]
foreach ($o in $luauObjects) { $objects.Add($o) }
foreach ($o in $minhookObjects) { $objects.Add($o) }
$sources = @($units) + @($modulesPath, (Join-Path $hostDir 'host.cpp'), (Join-Path $hostDir 'config.cpp'),
                               (Join-Path $hostDir 'plugin-runtime.cpp'), (Join-Path $hostDir 'plugin-panel.cpp'), (Join-Path $hostDir 'toml-lite.cpp'),
                               (Join-Path $hostDir 'theme-engine.cpp'), (Join-Path $hostDir 'palette-gen.cpp'),
                               (Join-Path $hostDir 'css-injector.cpp'), (Join-Path $hostDir 'css-gen.cpp'), (Join-Path $hostDir 'json-lite.cpp'),
                               (Join-Path $hostDir 'studio.cpp'), (Join-Path $hostDir 'menu-editor.cpp'), (Join-Path $hostDir 'binds-editor.cpp'), (Join-Path $hostDir 'glass.cpp'), (Join-Path $hostDir 'glass-engine.cpp'), (Join-Path $hostDir 'presets.cpp'),
                               (Join-Path $hostDir 'crash.cpp'), (Join-Path $hostDir 'identity.cpp'), (Join-Path $hostDir 'killswitch.cpp'), (Join-Path $hostDir 'keystrokes.cpp'), (Join-Path $hostDir 'screen-shader.cpp'), (Join-Path $hostDir 'capture.cpp'), (Join-Path $hostDir 'launcher.cpp'), (Join-Path $hostDir 'ui-sounds.cpp'), (Join-Path $hostDir 'extensions.cpp'), (Join-Path $hostDir 'dots.cpp'), (Join-Path $hostDir 'shell-mode.cpp'), (Join-Path $hostDir 'tray-host.cpp'), (Join-Path $hostDir 'app-bars.cpp'), (Join-Path $hostDir 'desktop-host.cpp'), (Join-Path $hostDir 'motion-settings.cpp'), (Join-Path $hostDir 'plugins.cpp'), (Join-Path $hostDir 'exclusions.cpp'), (Join-Path $hostDir 'notifications.cpp'), (Join-Path $hostDir 'hooks.cpp'))
foreach ($src in $sources) {
    $obj = Join-Path $gen ([IO.Path]::GetFileNameWithoutExtension($src) + '.o')
    "Compiling $([IO.Path]::GetFileName($src))"
    & $cxx @flags -c $src -o $obj
    if ($LASTEXITCODE) { throw "Compiling $src failed" }
    $objects.Add($obj)
}
# Its manifest (Windows' current controls, per-screen DPI), with the resource
# compiler that sits beside the C++ one; without one it still builds.
$windres = $null
$cxxDir = Split-Path $cxx
foreach ($name in @('windres.exe', 'llvm-windres.exe', 'windres', 'x86_64-w64-mingw32-windres')) {
    $candidate = if ($cxxDir) { Join-Path $cxxDir $name } else { $name }
    if (Test-Path -LiteralPath $candidate) { $windres = $candidate; break }
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { $windres = $cmd.Source; break }
}
if ($windres) {
    $res = Join-Path $gen 'resources.o'
    "Compiling tourne-desktop.rc"
    Push-Location $hostDir
    & $windres 'tourne-desktop.rc' -O coff -o $res
    $failed = $LASTEXITCODE
    Pop-Location
    if ($failed) { throw 'Compiling tourne-desktop.rc failed' }
    $objects.Add($res)
} else {
    'No resource compiler found: its manifest goes beside it as TourneDesktop.exe.manifest (keep the two together).'
}

$exe = Join-Path $Out 'TourneDesktop.exe'
$link = @('-mwindows', '-municode', '-static') + $target
"Linking"
if ($Debug) {
    & $cxx @objects -o $exe @link @libs
    if ($LASTEXITCODE) { throw 'Linking failed' }
} else {
    # TourneDesktop.symbols.exe is the same program with its function names left in, kept beside
    # it so a crash.txt address can be named. The exe itself has them stripped.
    $symbols = Join-Path $Out 'TourneDesktop.symbols.exe'
    & $cxx @objects -o $symbols @link @libs
    if ($LASTEXITCODE) { throw 'Linking failed' }
    $strip = $null
    foreach ($name in @('strip.exe', 'llvm-strip.exe', 'x86_64-w64-mingw32-strip', 'llvm-strip', 'strip')) {
        $candidate = if ($cxxDir) { Join-Path $cxxDir $name } else { $name }
        if (Test-Path -LiteralPath $candidate) { $strip = $candidate; break }
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) { $strip = $cmd.Source; break }
    }
    if ($strip) { & $strip -o $exe $symbols }
    if (-not $strip -or $LASTEXITCODE) { & $cxx @objects -o $exe @link '-s' @libs }
    if ($LASTEXITCODE) { throw 'Linking failed' }
}
$sideManifest = "$exe.manifest"
if ($windres) { Remove-Item -LiteralPath $sideManifest -ErrorAction SilentlyContinue }
else { Copy-Item -LiteralPath (Join-Path $hostDir 'tourne-desktop.manifest') -Destination $sideManifest -Force }
# tournectl (desktop\tools\tournectl.cpp), Tourne's hyprctl, beside it: small, and standalone.
$ctl = Join-Path $Out 'tournectl.exe'
& $cxx -std=c++20 -O2 -s -municode -DUNICODE -D_UNICODE -static @target (Join-Path $PSScriptRoot 'tools\tournectl.cpp') -o $ctl
if ($LASTEXITCODE) { Write-Warning 'tournectl.exe did not build (Tourne Desktop itself did)' }
"Built $exe ($([Math]::Round((Get-Item $exe).Length / 1MB, 1)) MB) with: $($Modules -join ', ')"
