<#
Real-registry helpers. This shell runs inside the packaged Claude app, whose HKCU\Software writes are
virtualized: my own reg/Set-ItemProperty calls never reach Explorer or StartAllBack. Anything that must
be seen by them goes through a process started outside the package (WMI), with its output in a file on X:.
#>
$script:realLog = Join-Path $PSScriptRoot 'out\real-reg.log'

function Invoke-Real([string]$CommandLine) {
    New-Item -ItemType Directory -Force (Split-Path $script:realLog) | Out-Null
    Remove-Item $script:realLog -ErrorAction SilentlyContinue
    $r = Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments @{ CommandLine = "cmd.exe /c ($CommandLine) > `"$script:realLog`" 2>&1" }
    if ($r.ReturnValue -ne 0) { throw "could not start: $($r.ReturnValue)" }
    $deadline = (Get-Date).AddSeconds(30)
    while ((Get-Date) -lt $deadline -and (Get-Process -Id $r.ProcessId -ErrorAction SilentlyContinue)) { Start-Sleep -Milliseconds 100 }
    if (Test-Path $script:realLog) { Get-Content $script:realLog -Raw } else { '' }
}

$script:sabKey = 'HKCU\Software\StartIsBack'
function Export-RealSab([string]$File)  { Invoke-Real "reg export $script:sabKey `"$File`" /y" | Out-Null }
function Import-RealSab([string]$File)  { Invoke-Real "reg import `"$File`"" | Out-Null }
function Remove-RealSabValue([string[]]$Names) {
    Invoke-Real (($Names | ForEach-Object { "reg delete $script:sabKey /v $_ /f" }) -join ' & ') | Out-Null
}
function Set-RealSabString([string]$Name, [string]$Value) { Invoke-Real "reg add $script:sabKey /v $Name /t REG_SZ /d `"$Value`" /f" | Out-Null }

# StartAllBack's real values as a hashtable (strings and numbers).
function Get-RealSab {
    $file = Join-Path $PSScriptRoot 'out\sab-real.reg'
    Export-RealSab $file
    $values = @{}
    $inKey = $false
    foreach ($line in Get-Content $file -Encoding Unicode) {
        if ($line -match '^\[') { $inKey = $line -eq '[HKEY_CURRENT_USER\Software\StartIsBack]'; continue }
        if (-not $inKey) { continue }
        if ($line -match '^"([^"]+)"=dword:([0-9a-f]{8})$') { $values[$matches[1]] = [long][uint32]::Parse($matches[2], 'HexNumber') }
        elseif ($line -match '^"([^"]+)"="(.*)"$') { $values[$matches[1]] = $matches[2].Replace('\\', '\') }
    }
    $values
}

function Set-RealSabDword([string]$Name, [long]$Value) { Invoke-Real "reg add $script:sabKey /v $Name /t REG_DWORD /d $Value /f" | Out-Null }
