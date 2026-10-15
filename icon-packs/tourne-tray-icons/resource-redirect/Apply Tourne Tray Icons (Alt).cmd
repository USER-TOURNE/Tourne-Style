@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0TourneTrayIcons.ps1" -Action Apply -Tone alt
if errorlevel 1 pause