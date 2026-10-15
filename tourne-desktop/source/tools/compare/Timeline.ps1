<#
Kills Explorer, then photographs StartAllBack's bar at growing delays, to see whether its look settles late
(the bar background was copper in some photographs and dark navy in others).
#>
param([int[]]$Delays = @(0, 5, 10, 20, 40), [string]$Out = "$PSScriptRoot\out-timeline")
$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force $Out | Out-Null
$null = [Drawing.Image]
$refs = @([AppDomain]::CurrentDomain.GetAssemblies() | Where-Object { $_.Location -match 'System\.Drawing|GdiPlus|System\.Private\.Windows|System\.Runtime\.dll|System\.Collections\.dll|System\.Threading|System\.Memory|netstandard|System\.Private\.CoreLib' } | ForEach-Object Location)
Add-Type -ReferencedAssemblies $refs -TypeDefinition ([IO.File]::ReadAllText("$PSScriptRoot\Grab.cs"))
$old = @(Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object Id)
for ($try = 0; $try -lt 10; $try++) {
    Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
    Start-Sleep -Milliseconds 700
    if (-not @(Get-Process explorer -ErrorAction SilentlyContinue | Where-Object { $old -contains $_.Id })) { break }
}
"old explorer ids: $($old -join ',')"
$start = Get-Date
$i = 0
foreach ($d in $Delays) {
    $wait = $d - ((Get-Date) - $start).TotalSeconds
    if ($wait -gt 0) { Start-Sleep -Seconds $wait }
    $probe = [Grab]::Probe()
    if ($probe -match 'task=1') {
        $path = "$Out\t$('{0:D2}' -f $d)s.png"
        $info = [Grab]::Tray($path, -1)
        $bmp = [Drawing.Bitmap]::FromFile($path)
        $p = $bmp.GetPixel(1000, [math]::Min(30, $bmp.Height - 1)); $bmp.Dispose()
        "{0,3}s  explorer={1}  {2}  bar pixel(1000,30)={3:X2}{4:X2}{5:X2}" -f $d, ((Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object Id) -join ','), $probe, $p.R, $p.G, $p.B
    } else { "{0,3}s  not ready: {1}" -f $d, $probe }
}
