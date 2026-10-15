<#
Explorer's side pane sizer, tried in a real folder window: the pane grown, then shrunk past the width it
opened at (Explorer Style lets go of the minimum Explorer sets to that width). The window gets a process
of its own, which is closed afterwards. Explorer's saved pane sizes are exported first and put back at
the end, through processes started by WMI (outside Claude's package, so the real registry). The pointer
isn't touched: panedrag.exe posts the mouse messages. Probe-Sizer.ps1 logs the sizer's own numbers.
  .\Test-PreviewPane.ps1 [-WithoutSabLayout] [-Steps 40,40,-100]   (each a drag; positive makes the pane smaller)
#>
param([string]$Folder = 'C:\Windows\Web', [switch]$WithoutSabLayout, [int[]]$Steps, [switch]$InShell, [int]$Windows = 1, [switch]$Select)
$here = $PSScriptRoot
$settings = 'HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Modules\GlobalSettings'
function Run-Outside([string]$commandLine) {
    $r = Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments @{ CommandLine = $commandLine }
    Start-Sleep -Milliseconds 1500
    return $r.ReturnValue
}
$backup = "$here\globalsettings-before-test.reg"
Run-Outside "reg.exe export `"$settings`" `"$backup`" /y" | Out-Null
if (-not (Test-Path $backup)) { 'backup failed: stopping'; return }
# StartAllBack's own bottom-details layout applies even while it's disabled (its DLL still loads into
# Explorer) and wins over Explorer Style's: -WithoutSabLayout turns it off for the test, and back after.
if ($WithoutSabLayout) { Run-Outside 'reg.exe add HKCU\Software\StartIsBack /v BottomDetails /t REG_DWORD /d 0 /f' | Out-Null }
Add-Type -Namespace PaneTest -Name U -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessageW(IntPtr h, uint m, IntPtr w, IntPtr l);'
# -Windows N: the same done in N windows one after another, each opening with the width the last left.
for ($round = 1; $round -le $Windows; $round++) {
if ($Windows -gt 1) { "window $round" }
$before = @(Get-Process explorer | Select-Object -ExpandProperty Id)
$proc = $null
$window = [IntPtr]::Zero
if ($InShell) {
    # -InShell: opened by the shell itself, in its own process, as folder windows normally are here.
    $shell = New-Object -ComObject Shell.Application
    $known = @($shell.Windows() | ForEach-Object { $_.HWND })
    $shell.Explore($Folder)
    for ($i = 0; $i -lt 30 -and $window -eq [IntPtr]::Zero; $i++) {
        Start-Sleep -Milliseconds 300
        $new = $shell.Windows() | Where-Object { $known -notcontains $_.HWND } | Select-Object -First 1
        if ($new) { $window = [IntPtr]$new.HWND }
    }
    # -Select: the first file selected, so the preview pane shows it while the pane is dragged.
    if ($Select -and $new) {
        Start-Sleep 1
        $item = @($new.Document.Folder.Items() | Where-Object { -not $_.IsFolder })[0]
        if ($item) { $new.Document.SelectItem($item, 1 + 4 + 8 + 16); "selected $($item.Name)" }
    }
} else {
    Start-Process explorer.exe $Folder
    for ($i = 0; $i -lt 30 -and -not $proc; $i++) {
        Start-Sleep -Milliseconds 300
        $proc = Get-Process explorer | Where-Object { $before -notcontains $_.Id -and $_.MainWindowHandle -ne 0 } | Select-Object -First 1
    }
    if ($proc) { $window = $proc.MainWindowHandle }
}
if ($window -ne [IntPtr]::Zero) {
    Start-Sleep 2
    $env:PANE_HWND = '{0:X}' -f $window.ToInt64()
    if ($Steps) { & "$here\panedrag.exe" steps "$here\pane" @Steps | Select-String 'view |drag |no sizer' }
    else { & "$here\panedrag.exe" auto "$here\pane" | Select-String 'view |no sizer' }
    [void][PaneTest.U]::PostMessageW($window, 0x10, [IntPtr]::Zero, [IntPtr]::Zero)
    Start-Sleep 2
    if ($proc) { Stop-Process -Id $proc.Id -ErrorAction SilentlyContinue }
} else { 'no new folder window' }
}
Run-Outside "reg.exe import `"$backup`"" | Out-Null
if ($WithoutSabLayout) { Run-Outside 'reg.exe add HKCU\Software\StartIsBack /v BottomDetails /t REG_DWORD /d 1 /f' | Out-Null }
'pane sizes put back'
