// Shell mode: Tourne Desktop as this account's shell, in place of Explorer.
//
// Windows starts the program named by HKCU\Software\Microsoft\Windows NT\
// CurrentVersion\Winlogon\Shell at sign-in. With shell.enabled on, that's
// Tourne Desktop (a copy kept in %LOCALAPPDATA%\Tourne\Desktop\bin, so moving
// or deleting the one you downloaded can't leave you without a shell), so
// Explorer's taskbar, Start and desktop never start. File Explorer windows
// still open as usual: explorer.exe runs as a file window when it isn't the
// shell.
//
// Never stuck:
//   - TourneDesktop.exe --shell is a small watchdog that runs Tourne Desktop
//     and waits. If you quit Tourne Desktop, Explorer starts for the rest of
//     the session. If it crashes, it's started again once; a second crash
//     soon after starts Explorer and turns shell mode off.
//   - Hold Shift while signing in: Explorer, this time only.
//   - --restore (and turning the switch off) puts Explorer back as the shell.
//   - --panic does the same and starts Explorer straight away, for when
//     there's no working desktop left to click in (run it from Task
//     Manager's "Run new task").
//
// In shell mode Tourne Desktop also starts your startup apps (the Run keys
// and Startup folders, as Task Manager's Startup page has them), once per
// sign-in.
//
// Windows apps (shell.windowsApps): Settings, the Store's apps and the like
// only get a window from the Explorer that runs as the shell. With this on,
// the watchdog starts Explorer first (explorer.exe /NoShellRegistrationCheck,
// as Windows' own kiosk shell host does), and Tourne Desktop keeps its
// taskbars and desktop hidden underneath, takes the tray icons over (its tray
// window first in line, where apps look) and starts Explorer again if it ends.
// Tourne Desktop still starts the startup apps (Explorer started this way
// doesn't count itself the shell, so it skips them). Quitting shows Explorer's taskbar.
// When Explorer is slow (after a restart) and comes up after Tourne Desktop,
// it is taken on then, the same way, for up to two minutes.
#pragma once

#include <windows.h>

#include <string>

#include "config.h"

namespace td {

constexpr int kAlreadyRunning = 2;  // Tourne Desktop's exit code when another copy already runs
constexpr int kPlannedRestart = 3;  // ended on purpose to start a new build (Swap-TourneDesktop.ps1): not a crash

void ShellDefaults(Settings& out);
void ShellDescribe(SchemaInfo& info);
void ShellApply();  // the settings changed: the shell set (or not) for the next sign-in
void ShellTryAgain();  // a setting was saved: shell mode turned off by crashes may be set again

int ShellWatchdog();                    // TourneDesktop.exe --shell
bool StartedAsShell(const std::wstring& args);  // --shell-child: run by the watchdog
void ShellSessionStart();               // Tourne Desktop is up as the shell: ready, startup apps
void ShellRestore();                    // --restore: Explorer the shell again
int ShellPanic();                       // --panic: Explorer the shell again and running, now
void ShellSessionStop();                // Tourne Desktop quitting: Explorer underneath shown again

bool ShellUnderneath();                    // Explorer runs hidden under Tourne (Windows apps)
bool ShellExplorerExpected();              // ... or will: it's still coming up (a slow start)
HWND ExplorerWindow(const wchar_t* cls);   // Explorer's own top-level window of that class
void ShellRelaunchExplorer();              // Explorer underneath ended: started again, Tourne's tray aside meanwhile (main thread)
void ShellExplorerRestarted();            // Explorer said "TaskbarCreated": hidden again, tray taken back
bool ShellShowExplorerBar(bool show);      // Explorer's own taskbars shown in Tourne's place (or hidden again); false without Explorer underneath

}  // namespace td
