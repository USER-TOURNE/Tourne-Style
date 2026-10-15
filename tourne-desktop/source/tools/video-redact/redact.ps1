$d = Split-Path $MyInvocation.MyCommand.Path
Add-Type -Path "$d\Redact.cs"
$ff = '%USERPROFILE%\AppData\Local\Programs\Convertify\resources\tools\ffmpeg\bin\ffmpeg.exe'
$in = 'G:\Nvidia HQ Recordings\Desktop\Screen Recording 2026-09-27 000706.mp4'
$out = '%USERPROFILE%\Downloads\Tourne Showcase\Done So Far\video\tourne-suite-settings-tour.mp4'
$sw = [Diagnostics.Stopwatch]::StartNew()
[Redact]::Run($ff, $in, $out, 'crop=944:1080:976:0')
"encode took $([math]::Round($sw.Elapsed.TotalSeconds)) s, $([math]::Round((Get-Item $out).Length / 1MB, 1)) MB"
Remove-Item "$d\leak_*.bmp" -ErrorAction SilentlyContinue
"--- leak check of the output:"
[Redact]::Check($ff, $out, $d)
