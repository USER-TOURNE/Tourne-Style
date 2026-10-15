<#
.SYNOPSIS
    Builds Vencord (or Testcord) from source with Tourne Big Files in it, then
    offers to put it into Discord.

.DESCRIPTION
    Vencord's own installer can't add plugins of your own: they only load in a
    copy built from source. This keeps that copy in one folder and rebuilds it.
    Run it again after editing the plugin or to update Vencord/Testcord.

    Needs Git and Node.js (LTS). pnpm is installed into your user folder with npm
    if it's missing (no admin). The copy lives in %LOCALAPPDATA%\Tourne\<Client> source
    unless -Folder says otherwise (Documents can point at a OneDrive folder that
    isn't there).

.EXAMPLE
    .\tools\vencord\Install-TourneBigFiles.ps1
    .\tools\vencord\Install-TourneBigFiles.ps1 -Client Testcord
    .\tools\vencord\Install-TourneBigFiles.ps1 -NoInject     # build only
#>
param(
    [ValidateSet("Vencord", "Testcord")]
    [string]$Client = "Vencord",
    [string]$Folder = "",
    [switch]$NoInject
)

$ErrorActionPreference = "Stop"
$plugin = Join-Path $PSScriptRoot "tourneBigFiles.desktop"
$repoUrl = @{ Vencord = "https://github.com/Vendicated/Vencord.git"; Testcord = "https://github.com/TestcordDev/TestCord.git" }[$Client]
if (-not $Folder) { $Folder = Join-Path $env:LOCALAPPDATA "Tourne\$Client source" }

function Assert-Tool([string]$name, [string]$hint) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) { throw "$name isn't installed. $hint" }
}

function Invoke-Step([string]$what, [scriptblock]$work) {
    Write-Host "== $what" -ForegroundColor Green
    & $work
    if ($LASTEXITCODE) { throw "$what failed (exit $LASTEXITCODE)." }
}

Assert-Tool git "Get it from https://git-scm.com/download/win"
Assert-Tool node "Get Node.js LTS from https://nodejs.org"
if (-not (Get-Command pnpm -ErrorAction SilentlyContinue)) {
    Write-Host "== Setting up pnpm" -ForegroundColor Green
    # Not corepack: it writes into Node's Program Files folder, which needs admin.
    # npm's own global folder is in %APPDATA%; pnpm's setup step must be allowed.
    npm install -g --allow-scripts=pnpm pnpm
    if (-not (Get-Command pnpm -ErrorAction SilentlyContinue)) { throw "pnpm couldn't be set up. Run: npm install -g --allow-scripts=pnpm pnpm, then open a new PowerShell." }
}
if (-not (Test-Path $plugin)) { throw "Can't find the plugin at $plugin" }

if (Test-Path (Join-Path $Folder ".git")) {
    Invoke-Step "Updating $Client in $Folder" { git -C $Folder pull --ff-only }
} else {
    New-Item -ItemType Directory -Force (Split-Path $Folder) | Out-Null
    Invoke-Step "Getting $Client into $Folder" { git clone --depth 1 $repoUrl $Folder }
}

$target = Join-Path $Folder "src\userplugins\tourneBigFiles.desktop"
New-Item -ItemType Directory -Force (Split-Path $target) | Out-Null
if (Test-Path $target) { Remove-Item -Recurse -Force $target }
Copy-Item -Recurse $plugin $target
Write-Host "== Plugin copied to $target" -ForegroundColor Green

Push-Location $Folder
try {
    Invoke-Step "Installing $Client's parts" { pnpm install --frozen-lockfile }
    Invoke-Step "Building $Client" { pnpm build }
    if (-not $NoInject) {
        Write-Host ""
        Write-Host "Next, the installer asks which Discord to put it in (pick yours, then Install / Repair)." -ForegroundColor Yellow
        Invoke-Step "Putting it into Discord" { pnpm inject }
    }
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "Done. Restart Discord fully (tray icon, Quit), then Settings, $Client, Plugins: turn on TourneBigFiles." -ForegroundColor Green
Write-Host "Use it: right-click the upload (+) button next to the message box."
