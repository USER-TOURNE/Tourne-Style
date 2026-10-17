<#
Shell32's preview pane sizer clamp, logged from inside a test folder window: sizerprobe.dll (loaded into
that one window's process only) patches CDUISizerElement::_ComputeBoundedSize and notes every call in
sizerprobe.txt. -Relax lets the reading pane's lower bound go (what Explorer Style's hook does).
Explorer's saved pane sizes are exported first and put back at the end, as in Test-PreviewPane.ps1.
  .\Probe-Sizer.ps1 [-Relax]
#>
param([switch]$Relax, [string]$Folder = 'C:\Windows\Web')
$here = $PSScriptRoot
$settings = 'HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Modules\GlobalSettings'
function Run-Outside([string]$commandLine) {
    Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments @{ CommandLine = $commandLine } | Out-Null
    Start-Sleep -Milliseconds 1500
}
$backup = "$here\globalsettings-before-probe.reg"
Run-Outside "reg.exe export `"$settings`" `"$backup`" /y"
if (-not (Test-Path $backup)) { 'backup failed: stopping'; return }
$log = "$here\sizerprobe.txt"
$relaxFile = "$here\sizerprobe-relax.txt"
if (Test-Path $log) { Clear-Content $log }
Set-Content $relaxFile ($Relax ? '1' : '0')
$before = @(Get-Process explorer | Select-Object -ExpandProperty Id)
Start-Process explorer.exe $Folder
$proc = $null
for ($i = 0; $i -lt 30 -and -not $proc; $i++) {
    Start-Sleep -Milliseconds 300
    $proc = Get-Process explorer | Where-Object { $before -notcontains $_.Id -and $_.MainWindowHandle -ne 0 } | Select-Object -First 1
}
if (-not $proc) { 'no new folder window process'; return }
Start-Sleep 2
& "$here\inject.exe" $proc.Id "$here\sizerprobe.dll"
Start-Sleep 1
$env:PANE_HWND = '{0:X}' -f $proc.MainWindowHandle.ToInt64()
& "$here\panedrag.exe" auto "$here\probe" | Select-String 'view |no sizer'
Add-Type -Namespace Probe -Name U -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessageW(IntPtr h, uint m, IntPtr w, IntPtr l);'
[void][Probe.U]::PostMessageW($proc.MainWindowHandle, 0x10, [IntPtr]::Zero, [IntPtr]::Zero)
Start-Sleep 2
Stop-Process -Id $proc.Id -ErrorAction SilentlyContinue
Run-Outside "reg.exe import `"$backup`""
'--- probe log (repeats counted):'
Get-Content $log -ErrorAction SilentlyContinue | Group-Object -NoElement | ForEach-Object { '{0,4}x {1}' -f $_.Count, $_.Name } | Select-Object -First 40
'pane sizes put back'
