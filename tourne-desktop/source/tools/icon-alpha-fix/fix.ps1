Add-Type -Path '%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\759a88eb-f697-4524-8dab-30747c3ec8f0\scratchpad\icons\Scan.cs','%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\759a88eb-f697-4524-8dab-30747c3ec8f0\scratchpad\icons\ResFile.cs','%USERPROFILE%\AppData\Local\Temp\claude\X--Documents-Claude-Workspace\759a88eb-f697-4524-8dab-30747c3ec8f0\scratchpad\icons\Fix.cs' -ReferencedAssemblies System.Drawing
$src='%USERPROFILE%\Downloads\bouquet_for_windows_10_by_niivu_dc3uqld\Bouquet  for Windows 10 by niivu\Icon Theme\Resources'
$out='%USERPROFILE%\Downloads\Bouquet Icon Theme (fixed alpha)'
foreach($f in 'imageres.dll','zipfldr.dll'){
  [Fix]::Run((Join-Path $src "Windhawk Resources\$f"), (Join-Path $src "$f.mun.res"), (Join-Path $out $f))
}
Copy-Item (Join-Path $src 'Windhawk Resources\imagesp1.dll') $out -Force
"--- verify"
foreach($f in 'imageres.dll','imagesp1.dll','zipfldr.dll'){ [Scan]::Report((Join-Path $out $f)) }
