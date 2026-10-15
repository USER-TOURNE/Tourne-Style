@echo off
rem Builds BonsaiSaver.scr (the screensaver) and bonsai.exe (the terminal version) with the C# compiler
rem built into Windows (.NET Framework 4), so there's nothing to install. Output goes to build.log too.
rem "build.bat quiet" skips the pause at the end.
setlocal
cd /d "%~dp0"
set CSC=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe
if not exist "%CSC%" set CSC=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\csc.exe
if not exist "%CSC%" (
  echo The C# compiler that comes with Windows wasn't found at %CSC%.> build.log
  type build.log
  goto end
)

echo Building BonsaiSaver.scr...> build.log
"%CSC%" /nologo /target:winexe /optimize+ /out:BonsaiSaver.scr /resource:scientifica.ttf,scientifica.ttf /r:System.Windows.Forms.dll /r:System.Drawing.dll BonsaiCore.cs BonsaiSaver.cs >> build.log 2>&1
if errorlevel 1 goto failed
echo Building bonsai.exe...>> build.log
"%CSC%" /nologo /target:exe /optimize+ /out:bonsai.exe /r:System.Drawing.dll BonsaiCore.cs BonsaiTui.cs >> build.log 2>&1
if errorlevel 1 goto failed
echo BUILD OK>> build.log
rem "bonsai" in any terminal: a copy in the user's WindowsApps folder, which Windows keeps on the PATH.
set APPS=%LOCALAPPDATA%\Microsoft\WindowsApps
if exist "%APPS%" copy /y bonsai.exe "%APPS%\bonsai.exe" >nul && echo Copied bonsai.exe to %APPS% so "bonsai" works in any terminal.>> build.log
type build.log
echo.
echo To try the screensaver: right-click BonsaiSaver.scr and choose Test (Install sets it as yours).
echo In any terminal: bonsai   (bonsai -h for everything)
goto end

:failed
echo BUILD FAILED>> build.log
type build.log

:end
if /i not "%~1"=="quiet" pause
