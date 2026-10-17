@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Setup\Snapshot.ps1" -Reset
pause
