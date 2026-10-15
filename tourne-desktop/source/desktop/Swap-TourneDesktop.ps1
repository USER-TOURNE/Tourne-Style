<#
Pull a commit, build Tourne Desktop and swap it into the copy running as the shell.

    .\desktop\Swap-TourneDesktop.ps1 -Commit c691bbf     # fetch, fast-forward, build, swap, restart
    .\desktop\Swap-TourneDesktop.ps1                     # build what's checked out, swap, restart
    .\desktop\Swap-TourneDesktop.ps1 -NoBuild            # swap the last build as it is
    .\desktop\Swap-TourneDesktop.ps1 -DryRun             # say what would happen, change nothing

The running exe in %LOCALAPPDATA%\Tourne\Desktop\bin is renamed (Windows allows that), the build
copied in, and only the --shell-child ended, with exit code 3 (kPlannedRestart): the --shell
watchdog starts the new exe from the same path and counts no crash. Never quit it cleanly (exit 0
hands the session to Explorer). Every write goes through Invoke-Real: Claude's shell has its
AppData writes redirected.

A watchdog from before kPlannedRestart (it marks itself in HKCU\Software\Tourne\Desktop\Watchdog)
counts the restart as a crash, so one crash of the new build within 2 minutes turns shell mode
off (2026-10-06). The script refuses then, unless -Force.
#>
param(
    [string]$Commit,
    [switch]$NoBuild,
    [switch]$DryRun,
    [switch]$Force  # swap even under a watchdog that counts the restart as a crash
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot
. (Join-Path $repo 'tools\compare\RealReg.ps1')
$bin = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Tourne\Desktop\bin'
$src = Join-Path $PSScriptRoot 'build'
$exe = Join-Path $bin 'TourneDesktop.exe'
$kQuickCrash = 125  # seconds; shell-mode.cpp's kQuickCrash is 120 s, plus a margin

function Step([string]$Text) { Write-Host "== $Text" }
# (not named Git: PowerShell names ignore case, so it would call itself)
function RepoGit { & git.exe -C $repo @args; if ($LASTEXITCODE) { throw "git $args failed" } }

# 1. The commit
if ($Commit) {
    Step "fetching"
    RepoGit fetch --all --prune --quiet
    $full = & git -C $repo rev-parse --verify --quiet "$Commit^{commit}"
    if (-not $full) { throw "$Commit isn't in any fetched branch" }
    & git -C $repo merge-base --is-ancestor $full HEAD
    if ($LASTEXITCODE -eq 0) {
        Step "$Commit is already checked out"
    } else {
        if (RepoGit status --porcelain --untracked-files=no) { throw 'tracked files have local changes: commit or stash them first' }
        $branch = RepoGit branch --show-current
        $upstream = & git -C $repo rev-parse --abbrev-ref '@{upstream}' 2>$null
        $onUpstream = $false
        if ($upstream) { & git -C $repo merge-base --is-ancestor $full $upstream; $onUpstream = $LASTEXITCODE -eq 0 }
        if (-not $onUpstream) {
            $where = (RepoGit branch -r --contains $full) -join ', '
            throw "$Commit isn't on $branch's upstream ($upstream). Branches that have it: $where. Check out the right one first."
        }
        Step "fast-forwarding $branch to $Commit"
        if (-not $DryRun) { RepoGit merge --ff-only --quiet $full }
    }
}
$head = (RepoGit rev-parse --short HEAD)

# 2. The build
if (-not $NoBuild) {
    Step "building $head"
    if (-not $DryRun) {
        $before = (Get-Item (Join-Path $src 'TourneDesktop.exe') -ErrorAction SilentlyContinue).LastWriteTime
        & (Join-Path $PSScriptRoot 'Build-TourneDesktop.ps1')
        $after = (Get-Item (Join-Path $src 'TourneDesktop.exe') -ErrorAction SilentlyContinue).LastWriteTime
        if (-not $after -or $after -eq $before) { throw 'the build left no new TourneDesktop.exe' }
    }
}

# 3. What's running
$procs = Get-CimInstance Win32_Process -Filter "name='TourneDesktop.exe'"
$child = $procs | Where-Object { $_.CommandLine -match '--shell-child' -and $_.ExecutablePath -ieq $exe }
$watchdog = $procs | Where-Object { $_.CommandLine -match '--shell(\s|"|$)' -and $_.CommandLine -notmatch '--shell-child' }
$old = (Invoke-Real "type `"$bin\TourneDesktop.commit.txt`"").Trim()
if (-not $old -or $old -match '\s') { $old = (Get-Date).ToString('yyyyMMdd-HHmmss') }
Step "running: watchdog $(if ($watchdog) { $watchdog.ProcessId } else { 'none' }), shell child $(if ($child) { $child.ProcessId } else { 'none' }), build $old"

$planned = (Invoke-Real 'reg query HKCU\Software\Tourne\Desktop\Watchdog /v PlannedRestart') -match 'PlannedRestart\s+REG_DWORD\s+0x1'
if ($child -and $watchdog -and -not $planned) {
    if (-not $Force) {
        throw ("the running watchdog is older than planned restarts: it would count this restart as a crash, and one " +
               "crash of the new build in the next 2 minutes would turn shell mode off. Sign out and in first (the " +
               "watchdog becomes the installed build), or pass -Force.")
    }
    Step 'watchdog is too old for planned restarts (-Force): this restart counts as a crash'
}
if ($child -and $watchdog -and -not $planned) {
    # A second end within kQuickCrash of the first turns shell mode off: wait it out.
    $age = ((Get-Date) - $child.CreationDate).TotalSeconds
    if ($age -lt $kQuickCrash) {
        $wait = [int][Math]::Ceiling($kQuickCrash - $age)
        Step "the shell child is $([int]$age) s old: waiting $wait s so the watchdog doesn't count two crashes"
        if (-not $DryRun) { Start-Sleep -Seconds $wait }
    }
}

# 4. The swap
$copy = "copy /y `"$src\TourneDesktop.exe`" . && copy /y `"$src\TourneDesktop.symbols.exe`" . && copy /y `"$src\TourneDesktop.exe.manifest`" . && (if exist `"$src\tournectl.exe`" copy /y `"$src\tournectl.exe`" .) && (echo $head)> TourneDesktop.commit.txt"
$line = "cd /d `"$bin`" && (if exist TourneDesktop.exe.prev-$old del TourneDesktop.exe.prev-$old) && (if exist TourneDesktop.exe ren TourneDesktop.exe TourneDesktop.exe.prev-$old) && $copy"
Step "swap: $line"
if ($child -and $watchdog) { Step "then end shell child $($child.ProcessId) with exit code 3 (planned restart)" }
if ($DryRun) { Step 'dry run: nothing changed'; return }
$crashFile = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Tourne\Desktop\crash.txt'
$crashBefore = Invoke-Real "dir `"$crashFile`" 2>nul | findstr /r `"[0-9]:[0-9]`""
Write-Host (Invoke-Real $line)
if ($child -and $watchdog) {
    Add-Type -Namespace TourneSwap -Name Native -MemberDefinition @"
[DllImport("kernel32.dll", SetLastError = true)] public static extern IntPtr OpenProcess(uint access, bool inherit, uint pid);
[DllImport("kernel32.dll", SetLastError = true)] public static extern bool TerminateProcess(IntPtr process, uint code);
[DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr handle);
"@
    $h = [TourneSwap.Native]::OpenProcess(0x0001, $false, [uint32]$child.ProcessId)  # PROCESS_TERMINATE
    if ($h -eq [IntPtr]::Zero) { throw "couldn't open shell child $($child.ProcessId) to restart it" }
    $ended = [TourneSwap.Native]::TerminateProcess($h, 3)
    [void][TourneSwap.Native]::CloseHandle($h)
    if (-not $ended) { throw "couldn't end shell child $($child.ProcessId)" }
}

# 5. Check
if (-not ($child -and $watchdog)) {
    Step "no shell-mode copy was running: the new build is in $bin, start it yourself"
    return
}
$deadline = (Get-Date).AddSeconds(20)
do {
    Start-Sleep -Milliseconds 500
    $new = Get-CimInstance Win32_Process -Filter "name='TourneDesktop.exe'" |
        Where-Object { $_.CommandLine -match '--shell-child' -and $_.ProcessId -ne $child.ProcessId }
} until ($new -or (Get-Date) -gt $deadline)
Start-Sleep -Seconds 3
Write-Host (Invoke-Real "powershell -NoProfile -Command `"Get-Content '$([Environment]::GetFolderPath('LocalApplicationData'))\Tourne\Desktop\shell.log' -Tail 4`"")
if (-not $new) { throw 'no new shell child after 20 s: read shell.log; Explorer may have taken over' }
# A crash right after starting (the new build's, not the restart's): say so, with the report.
Start-Sleep -Seconds 5
$crashAfter = Invoke-Real "dir `"$crashFile`" 2>nul | findstr /r `"[0-9]:[0-9]`""
if ($crashAfter -ne $crashBefore) {
    Write-Host (Invoke-Real "type `"$crashFile`"")
    throw "the new build crashed after starting: the report is above ($crashFile)"
}
Step "swapped: $old -> $head, shell child $($new.ProcessId) (the watchdog stays the build it signed in with)"
