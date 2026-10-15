$o = '%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\759a88eb-f697-4524-8dab-30747c3ec8f0\scratchpad\showcase'
Add-Type -Path "$o\Seek.cs"
$ff = '%USERPROFILE%\AppData\Local\Programs\Convertify\resources\tools\ffmpeg\bin\ffmpeg.exe'
$src = 'G:\Nvidia HQ Recordings\Desktop\Screen Recording 2026-09-27 000706.mp4'
$out = '%USERPROFILE%\Downloads\Tourne Showcase\Done So Far\video\tourne-suite-settings-tour.mp4'
$crop = 'crop=944:1080:976:0'
# The ICO path box at 0:10 (frame 300): its text band is rows 652..670.
$x = [Seek]::TextStart($ff, $src, $crop, 300, 652, 18)
"template: text starts at x=$x"
$tw = 88; $th = 16
$t = [Seek]::Template($ff, $src, $crop, 300, $x - 1, 653, $tw, $th)
$sw = [Diagnostics.Stopwatch]::StartNew()
"=== original video (must find it):"
[Seek]::Scan($ff, $src, $crop, $t, $tw, $th, 0.80)
"took $([math]::Round($sw.Elapsed.TotalSeconds)) s"
"=== redacted video (must not):"
[Seek]::Scan($ff, $out, $null, $t, $tw, $th, 0.80)
