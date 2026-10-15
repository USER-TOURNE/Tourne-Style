@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0TourneTrayIcons.ps1" -Action Revert
if errorlevel 1 pause