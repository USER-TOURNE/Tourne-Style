// The desktop, kept by Tourne Desktop when it is the shell (shell-mode.h): a
// window under every other one, named as Explorer's is ("Progman", "Program
// Manager") so programs that look for the desktop find it. It shows the
// wallpaper Windows has and, through Windows' own folder view (the one
// Explorer uses), the desktop's icons, so the icons, their menus, renaming,
// dragging, "Show desktop icons" and the desktop's right-click menu work as
// they do with Explorer. Tourne's wallpaper manager will draw here later.
//
// Idle cost: none.
#pragma once

#include <windows.h>

namespace td {

void DesktopHostStart();
void DesktopHostClaimShell();  // Windows' shell window (GetShellWindow), when nobody has it
void DesktopHostStop();
void DesktopHostSettingsChanged();  // (the wallpaper slideshow follows its settings)

}  // namespace td
