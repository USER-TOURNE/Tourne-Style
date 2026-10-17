# Tourne Desktop's own Alienware Command Center, after psyreactor's DMS plugin.
#
# Talks to the firmware interface Alienware (and Dell G-series) laptops expose to Windows:
# the WMI class AWCCWmiMethodFunction in root\WMI. The same interface is what Linux's
# alienware-wmi driver calls "AWCC"; each method takes one 32-bit argument packed from four
# bytes (operation, then up to three operands) and returns one 32-bit value.
#
#   Thermal_Information  0x02 system description (fans, sensors, other, profiles counts)
#                        0x03 resource id at an index (fans first, profiles last)
#                        0x0B current thermal profile
#                        0x0C fan boost of a fan (0..255)
#   Thermal_Control      0x01 activate a thermal profile
#                        0x02 set a fan's boost (fan id, 0..255)
#   GameShiftStatus      0x01 toggle G-Mode, 0x02 read G-Mode
#
# Usage (prints one JSON line):
#   awcc.ps1 -Op info|current|mode|boost|install|uninstall [-Arg x] [-Arg2 y] [-Out file -Nonce n]
#   awcc.ps1 -Request <file>   reads {"nonce","op","arg","arg2"} from a JSON file and writes
#                              result.json next to it (used when it runs as administrator).

param(
  [string]$Op = 'info',
  [string]$Arg = '',
  [string]$Arg2 = '',
  [string]$Request = '',
  [string]$Out = '',
  [string]$Nonce = ''
)

$ErrorActionPreference = 'Stop'
$TaskPath = '\Tourne\'
$TaskName = 'AlienwareCommandCenter'

$Names = @{
  0xA0 = 'Balanced'; 0xA1 = 'Performance'; 0xA2 = 'Cool'; 0xA3 = 'Quiet'; 0xA4 = 'Full Speed'
  0xA5 = 'Battery Saving'; 0x96 = 'Quiet'; 0x97 = 'Balanced'; 0x98 = 'Performance'; 0x99 = 'Full Speed'
  0xAB = 'G-Mode'; 0x00 = 'Manual'
}
$Keys = @{
  0xA0 = 'balanced'; 0xA1 = 'performance'; 0xA2 = 'cool'; 0xA3 = 'quiet'; 0xA4 = 'fullspeed'
  0xA5 = 'battery'; 0x96 = 'quiet'; 0x97 = 'balanced'; 0x98 = 'performance'; 0x99 = 'fullspeed'
  0xAB = 'gmode'; 0x00 = 'manual'
}

$script:obj = $null
$script:result = [ordered]@{ ok = $true }

function Test-Admin {
  $id = [Security.Principal.WindowsIdentity]::GetCurrent()
  return ([Security.Principal.WindowsPrincipal]$id).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-Denied($err) {
  $text = "$err"
  return ($text -match 'Access (is )?denied' -or $text -match '0x80041003' -or $text -match '0x80070005' -or $text -match 'privilege')
}

function Get-Awcc {
  if (-not $script:obj) {
    $script:obj = Get-CimInstance -Namespace 'root/WMI' -ClassName 'AWCCWmiMethodFunction' | Select-Object -First 1
  }
  return $script:obj
}

# One call: packs the four bytes little-endian into arg2, returns the out value (or -1 on a firmware "no").
function Invoke-Awcc([string]$method, [int]$op, [int]$a1 = 0, [int]$a2 = 0, [int]$a3 = 0) {
  $packed = [uint32]([double]$op + [double]$a1 * 256 + [double]$a2 * 65536 + [double]$a3 * 16777216)
  $r = Invoke-CimMethod -InputObject (Get-Awcc) -MethodName $method -Arguments @{ arg2 = $packed }
  $value = $null
  if ($r.PSObject.Properties['argr']) { $value = $r.argr }
  if ($null -eq $value) {
    foreach ($p in $r.PSObject.Properties) {
      if ($p.Name -notin @('ReturnValue', 'PSComputerName', 'CimClass', 'CimInstanceProperties', 'CimSystemProperties') -and $p.Value -is [ValueType]) {
        $value = $p.Value; break
      }
    }
  }
  if ($null -eq $value) { return -1 }
  $u = [uint32]$value
  if ($u -ge 0xFFFFFFFE) { return -1 }
  return [int64]$u
}

function Get-Model {
  try {
    $cs = Get-CimInstance -ClassName Win32_ComputerSystem
    $script:result.maker = "$($cs.Manufacturer)".Trim()
    $script:result.model = "$($cs.Model)".Trim()
  } catch {
    $script:result.maker = ''; $script:result.model = ''
  }
}

function Read-State {
  $res = $script:result
  $res.admin = Test-Admin
  Get-Model
  try {
    $class = Get-CimClass -Namespace 'root/WMI' -ClassName 'AWCCWmiMethodFunction'
    $res.methods = @($class.CimClassMethods | ForEach-Object { $_.Name })
  } catch {
    $res.awcc = $false
    return
  }
  $res.awcc = $true
  try {
    $null = Get-Awcc
    if (-not $script:obj) { $res.awcc = $false; return }
    $desc = Invoke-Awcc 'Thermal_Information' 0x02
    if ($desc -lt 0) { $res.error = 'The firmware did not describe its fans and modes.'; return }
    $fanCount = [int]($desc -band 0xFF)
    $tempCount = [int](($desc -shr 8) -band 0xFF)
    $otherCount = [int](($desc -shr 16) -band 0xFF)
    $profileCount = [int](($desc -shr 24) -band 0xFF)

    $fans = @()
    for ($i = 0; $i -lt $fanCount; $i++) {
      $rid = Invoke-Awcc 'Thermal_Information' 0x03 $i
      if ($rid -lt 0) { continue }
      $fid = [int]($rid -band 0xFF)
      $boost = Invoke-Awcc 'Thermal_Information' 0x0C $fid
      $label = switch ($fans.Count) { 0 { 'CPU' } 1 { 'GPU' } default { "Fan $($fans.Count + 1)" } }
      $fans += [ordered]@{ id = $fid; name = $label; boost = [int]([math]::Max(0, $boost)) }
    }
    $res.fans = $fans

    $modes = @()
    $seen = @{}
    $offset = $fanCount + $tempCount + $otherCount
    for ($i = 0; $i -lt $profileCount; $i++) {
      $rid = Invoke-Awcc 'Thermal_Information' 0x03 ($offset + $i)
      if ($rid -lt 0) { continue }
      $code = [int]($rid -band 0xFF)
      if (-not $Keys.ContainsKey($code) -or $seen.ContainsKey($Keys[$code])) { continue }
      $seen[$Keys[$code]] = $true
      $modes += [ordered]@{ key = $Keys[$code]; code = $code; name = $Names[$code] }
    }

    $gmode = $null
    if ($res.methods -contains 'GameShiftStatus') {
      $g = Invoke-Awcc 'GameShiftStatus' 0x02
      if ($g -ge 0) { $gmode = [bool]($g -band 1) }
    }
    $res.gmode = $gmode
    if ($null -ne $gmode -and -not $seen.ContainsKey('gmode')) {
      $modes += [ordered]@{ key = 'gmode'; code = 0xAB; name = 'G-Mode' }
    }
    if (-not $seen.ContainsKey('manual')) {
      $modes += [ordered]@{ key = 'manual'; code = 0x00; name = 'Manual' }
    }
    $res.modes = $modes

    $cur = Invoke-Awcc 'Thermal_Information' 0x0B
    if ($cur -ge 0) {
      $code = [int]($cur -band 0xFF)
      $res.current = [ordered]@{ code = $code; key = $Keys[$code]; name = $(if ($Names.ContainsKey($code)) { $Names[$code] } else { ('Mode 0x{0:X2}' -f $code) }) }
    }
  } catch {
    if (Test-Denied $_) { $res.needsAdmin = $true } else { $res.error = "$($_.Exception.Message)".Trim() }
  }
}

function Set-Mode([string]$key) {
  # The newer table first (0xA0..0xA5); a machine on the older one (0x96..0x99) lists those, so
  # whatever the firmware lists for this mode wins.
  $code = -1
  foreach ($k in $Keys.Keys) { if ($Keys[$k] -eq $key -and ($k -lt 0x96 -or $k -gt 0x99)) { $code = $k } }
  $desc = Invoke-Awcc 'Thermal_Information' 0x02
  if ($desc -ge 0) {
    $offset = [int]($desc -band 0xFF) + [int](($desc -shr 8) -band 0xFF) + [int](($desc -shr 16) -band 0xFF)
    $profileCount = [int](($desc -shr 24) -band 0xFF)
    for ($j = 0; $j -lt $profileCount; $j++) {
      $rid = Invoke-Awcc 'Thermal_Information' 0x03 ($offset + $j)
      if ($rid -ge 0 -and $Keys[[int]($rid -band 0xFF)] -eq $key) { $code = [int]($rid -band 0xFF) }
    }
  }
  if ($code -lt 0) { throw "Unknown thermal mode '$key'." }
  $class = Get-CimClass -Namespace 'root/WMI' -ClassName 'AWCCWmiMethodFunction'
  if (@($class.CimClassMethods | ForEach-Object { $_.Name }) -contains 'GameShiftStatus') {
    $g = Invoke-Awcc 'GameShiftStatus' 0x02
    if ($g -ge 0) {
      $on = [bool]($g -band 1)
      if (($key -eq 'gmode') -ne $on) { $null = Invoke-Awcc 'GameShiftStatus' 0x01 }
    }
  }
  $r = Invoke-Awcc 'Thermal_Control' 0x01 $code
  if ($r -lt 0) { throw "The firmware refused $($Names[$code])." }
  $script:result.did = "Thermal mode: $($Names[$code])"
}

function Set-Boost([string]$fan, [string]$percent) {
  $fid = 0; $pct = 0
  if (-not [int]::TryParse($fan, [ref]$fid) -or $fid -lt 0 -or $fid -gt 255) { throw 'Bad fan id.' }
  if (-not [int]::TryParse($percent, [ref]$pct)) { throw 'Bad fan boost.' }
  $pct = [math]::Min(100, [math]::Max(0, $pct))
  $raw = [int][math]::Round($pct * 255 / 100)
  $r = Invoke-Awcc 'Thermal_Control' 0x02 $fid $raw
  if ($r -lt 0) { throw 'The firmware refused the fan boost.' }
  $script:result.did = "Fan boost $pct%"
}

function Install-Task([string]$requestFile, [string]$user) {
  if (-not (Test-Admin)) { throw 'Setting up needs administrator rights.' }
  if (-not $requestFile -or -not $user) { throw 'Missing request file or user.' }
  $dir = Join-Path $env:ProgramData 'Tourne\AlienwareCommandCenter'
  New-Item -ItemType Directory -Force -Path $dir | Out-Null
  $dest = Join-Path $dir 'awcc.ps1'
  Copy-Item -LiteralPath $PSCommandPath -Destination $dest -Force
  # Only administrators may change the copy the task runs; everyone else may read it.
  $acl = New-Object System.Security.AccessControl.DirectorySecurity
  $acl.SetAccessRuleProtection($true, $false)
  $inherit = [System.Security.AccessControl.InheritanceFlags]'ContainerInherit, ObjectInherit'
  $none = [System.Security.AccessControl.PropagationFlags]::None
  foreach ($sid in @('S-1-5-32-544', 'S-1-5-18')) {
    $acl.AddAccessRule((New-Object System.Security.AccessControl.FileSystemAccessRule((New-Object System.Security.Principal.SecurityIdentifier $sid), 'FullControl', $inherit, $none, 'Allow')))
  }
  $acl.AddAccessRule((New-Object System.Security.AccessControl.FileSystemAccessRule((New-Object System.Security.Principal.SecurityIdentifier 'S-1-5-32-545'), 'ReadAndExecute', $inherit, $none, 'Allow')))
  Set-Acl -LiteralPath $dir -AclObject $acl
  $argLine = '-NoProfile -NonInteractive -ExecutionPolicy Bypass -WindowStyle Hidden -File "' + $dest + '" -Request "' + $requestFile + '"'
  $action = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument $argLine
  $settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -ExecutionTimeLimit (New-TimeSpan -Minutes 2) -MultipleInstances Queue
  try {
    # S4U runs without a window; it needs the "log on as a batch job" right, which administrators have.
    $principal = New-ScheduledTaskPrincipal -UserId $user -LogonType S4U -RunLevel Highest
    Register-ScheduledTask -TaskPath $TaskPath -TaskName $TaskName -Action $action -Principal $principal -Settings $settings -Force | Out-Null
  } catch {
    $principal = New-ScheduledTaskPrincipal -UserId $user -LogonType Interactive -RunLevel Highest
    Register-ScheduledTask -TaskPath $TaskPath -TaskName $TaskName -Action $action -Principal $principal -Settings $settings -Force | Out-Null
  }
  $script:result.did = 'Alienware controls no longer ask for administrator rights.'
}

function Uninstall-Task {
  if (-not (Test-Admin)) { throw 'Removing it needs administrator rights.' }
  Unregister-ScheduledTask -TaskPath $TaskPath -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue
  Remove-Item -LiteralPath (Join-Path $env:ProgramData 'Tourne\AlienwareCommandCenter') -Recurse -Force -ErrorAction SilentlyContinue
  $script:result.did = 'Alienware controls will ask for administrator rights again.'
}

$outFile = $Out
$nonce = $Nonce
if ($Request) {
  try {
    $req = Get-Content -LiteralPath $Request -Raw | ConvertFrom-Json
    $Op = "$($req.op)"; $Arg = "$($req.arg)"; $Arg2 = "$($req.arg2)"; $nonce = "$($req.nonce)"
  } catch {
    $Op = 'info'
  }
  $outFile = Join-Path (Split-Path -Parent $Request) 'result.json'
}
$script:result.nonce = $nonce
$script:result.op = $Op

try {
  switch ($Op) {
    'info' { }
    'current' { }
    'mode' {
      if ($Arg -notmatch '^[a-z]+$') { throw 'Bad mode.' }
      Set-Mode $Arg
    }
    'boost' { Set-Boost $Arg $Arg2 }
    'install' {
      # The request file the task reads is fixed when it's set up: $Arg names it, $Arg2 the user.
      if ($Request) { throw 'Not from a request.' }
      Install-Task $Arg $Arg2
    }
    'uninstall' { if ($Request) { throw 'Not from a request.' }; Uninstall-Task }
    default { throw "Unknown operation '$Op'." }
  }
} catch {
  $script:result.ok = $false
  if (Test-Denied $_) { $script:result.needsAdmin = $true } else { $script:result.error = "$($_.Exception.Message)".Trim() }
}

if ($Op -eq 'current') {
  $script:result.admin = Test-Admin
  try {
    $null = Get-Awcc
    if (-not $script:obj) { $script:result.awcc = $false } else {
      $script:result.awcc = $true
      $cur = Invoke-Awcc 'Thermal_Information' 0x0B
      if ($cur -ge 0) {
        $code = [int]($cur -band 0xFF)
        $script:result.current = [ordered]@{ code = $code; key = $Keys[$code]; name = $(if ($Names.ContainsKey($code)) { $Names[$code] } else { ('Mode 0x{0:X2}' -f $code) }) }
      }
      try {
        $g = Invoke-Awcc 'GameShiftStatus' 0x02
        if ($g -ge 0) { $script:result.gmode = [bool]($g -band 1) }
      } catch { }
    }
  } catch {
    if (Test-Denied $_) { $script:result.needsAdmin = $true } else { $script:result.awcc = $false; $script:result.error = "$($_.Exception.Message)".Trim() }
  }
} elseif ($Op -ne 'uninstall') {
  $keepOk = $script:result.ok; $keepErr = $script:result.error; $keepAdmin = $script:result.needsAdmin
  Read-State
  if (-not $keepOk) { $script:result.ok = $false }
  if ($keepErr) { $script:result.error = $keepErr }
  if ($keepAdmin) { $script:result.needsAdmin = $true }
}

$json = ($script:result | ConvertTo-Json -Depth 5 -Compress)
if ($outFile) {
  try { [IO.File]::WriteAllText($outFile, $json, (New-Object System.Text.UTF8Encoding $false)) } catch { }
}
Write-Output $json
