// The launcher (launcher.cpp): a search box on a hotkey (Alt+Space unless set), over the screen under the pointer.
// It finds apps (everything Start lists, desktop and Store alike) and asks the plugins' launcher providers
// ([[launcher_provider]]: Game Launcher's "g", Display Profiles, Syncthing...): typed after a provider's prefix and a
// space, only that one answers; without one, those that join every search answer under the apps. Enter starts the
// one picked, Esc or a click elsewhere closes it.
//
// Idle cost: a registered hotkey and a window that's hidden; the app list is read again when it opens, at most
// once a minute, on a thread of its own.
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

void LauncherDefaults(Settings& out);
void LauncherDescribe(SchemaInfo& info);
void LauncherStart(HWND host);
void LauncherApply();  // the settings changed
void LauncherStop();
void LauncherToggle();  // (tourne: launcher)

}  // namespace td
