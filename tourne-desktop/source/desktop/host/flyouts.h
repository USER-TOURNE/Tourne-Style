// The flyouts (flyouts.cpp): what Windows shows over the screen when the volume changes, a media key is
// pressed or Caps, Num or Scroll Lock is turned, drawn by Tourne in its menus' look. In shell mode Explorer is
// hidden, so without these nothing shows at all.
//
// Idle cost: one thread with a hidden window, waiting. The volume is heard through Windows' own notification
// (no polling); keys come as raw input, only the few that show something are looked at.
#pragma once

#include <windows.h>

#include <string>

#include "config.h"

namespace td {

void FlyoutsDefaults(Settings& out);
void FlyoutsDescribe(SchemaInfo& info);
void FlyoutsStart(HWND host);
void FlyoutsApply();  // the settings changed
void FlyoutsStop();

// A tray icon's balloon (NIF_INFO), shown as Tourne's: its app's icon, title and words, over the corner by the
// tray for a few seconds; a click on it is told to the app (NIN_BALLOONUSERCLICK), as Windows' would be.
// Any thread; the icon is copied.
struct Balloon {
    HWND owner = nullptr;
    UINT id = 0, callback = 0, version = 0;
    HICON icon = nullptr;
    std::wstring app, title, body;
    bool quiet = false;  // NIIF_NOSOUND (Tourne's pop-ups make none anyway)
};
void FlyoutsBalloon(const Balloon& b);

}  // namespace td
