// The notification area, hosted by Tourne Desktop when it is the shell
// (shell-mode.h). Apps put their tray icons in by sending Shell_NotifyIcon's
// message to the "Shell_TrayWnd" window; with Explorer not the shell, that
// window is this one. Tourne's bar reads the icons the way it reads them from
// the suite's Explorer relay (the same request and packet, tray-shared.cpp),
// so it needs no change, and clicks go straight to each app as before.
//
// With Explorer running underneath (shell-mode.h, Windows apps) there are two
// "Shell_TrayWnd" windows: apps send to the first one they find, so this one
// is kept first (topmost, raised again as windows come and go).
//
// Idle cost: none. It wakes when an app adds, changes or removes an icon.
#pragma once

#include <windows.h>

namespace td {

void TrayCollectorStart();  // the watchdog, at sign-in: a stand-in tray until TrayHostStart takes its icons on
void TrayHostStart();
void TrayHostStop();
void TrayHostRaise();   // with Explorer underneath: this tray first in line, where apps look for one
void TrayHostRetake();  // ... and apps asked to put their icons in again
void TrayHostExplorerEnded();  // (any thread) Explorer underneath ended: ShellRelaunchExplorer on the main thread

}  // namespace td
