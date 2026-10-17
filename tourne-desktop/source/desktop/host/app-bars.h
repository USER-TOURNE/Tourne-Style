// App bars and the work area, kept by Tourne Desktop when it is the shell
// (shell-mode.h). Bars that reserve an edge of a screen (Tourne's own bar,
// and other programs' docks and bars) ask with SHAppBarMessage, which Windows
// sends to the "Shell_TrayWnd" window: with Explorer not the shell, that's
// the tray host's (tray-host.h), which hands those messages here. Each
// screen's work area is its whole area less the bars on its edges, so
// maximised windows stop at them, and bars hear when a program fills their
// screen (ABN_FULLSCREENAPP), as they would from Explorer.
//
// Idle cost: none. It works when a bar moves or the window in front changes.
#pragma once

#include <windows.h>

namespace td {

void AppBarsStart();
void AppBarsStop();
// SHAppBarMessage's request (WM_COPYDATA, dwData 0): what it answers.
LRESULT AppBarsMessage(const COPYDATASTRUCT* data);
// The screens changed (WM_DISPLAYCHANGE): work areas set again.
void AppBarsScreensChanged();

}  // namespace td
