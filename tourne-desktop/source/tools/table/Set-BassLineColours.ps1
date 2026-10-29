# Sets the bass line's colours in Tourne Desktop's built-in Table to the style's own three colours: Body = Color,
# Kick = Gradient Color 1 (opaque), Snare = Gradient Color 2. They go in the applied style (the "sharedSetup"
# layer, which wins over Studio's values), and the Table reads them at its next start. Run it in a real
# process (Invoke-Real): HKCU writes from a packaged app's shell are redirected. Logs beside it.
$key = 'HKCU:\Software\Tourne\Desktop\Values\table'
$log = Join-Path $PSScriptRoot 'set-bassline-colours.log'
$layer = (Get-ItemProperty -Path $key -Name sharedSetup).sharedSetup
$lines = $layer -split "`n" | Where-Object { $_ -ne '' }
$get = { param($name) ($lines | Where-Object { $_ -like "$name=*" } | Select-Object -First 1) -replace '^[^=]*=', '' }
$body = & $get 'appearance.color'
$g1 = & $get 'appearance.gradientColor1'
$g2 = & $get 'appearance.gradientColor2'
# rgba(r, g, b, a) made opaque, so the kick flash is as bright as the other two
if ($g1 -match '^rgba?\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)') { $g1 = '#{0:x2}{1:x2}{2:x2}' -f [int]$Matches[1], [int]$Matches[2], [int]$Matches[3] }
$want = @{ 'bassLine.bodyColor' = $body; 'bassLine.kickColor' = $g1; 'bassLine.snareColor' = $g2 }
$out = foreach ($l in $lines) {
    $k = $l -replace '=.*$', ''
    if ($want.ContainsKey($k)) { continue }
    $l
}
$out += $want.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Name)=$($_.Value)" }
Set-ItemProperty -Path $key -Name sharedSetup -Value (($out -join "`n") + "`n")
"body $body, kick $g1, snare $g2" | Set-Content $log
