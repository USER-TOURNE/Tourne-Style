@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0TourneTrayIcons.ps1" -Action Apply -Tone ink-flat
if errorlevel 1 pause
