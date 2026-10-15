# Puts the suite's code on the clipboard, for pasting into Windhawk's mod editor.
$file = Join-Path (Split-Path $PSScriptRoot) 'tourne-suite-test.wh.cpp'
Get-Content -Raw -LiteralPath $file | Set-Clipboard
Write-Host "Copied. In Windhawk's editor: Ctrl+A, Ctrl+V, then Compile Mod."
