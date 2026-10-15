// Notifications from Tourne Studio (notify.*): Windows' own notification settings, globally and per app,
// written where Windows keeps them (HKCU\Software\Microsoft\Windows\CurrentVersion\Notifications\Settings),
// so Windows' Settings page agrees; and quiet hours of Tourne's own.
//
// Idle cost: none (quiet hours wait on a timer for their next start or end).
#pragma once

#include "config.h"

namespace td {

void NotifyDefaults(Settings& out);  // Windows' values now, as the defaults (the Studio shows what Windows has)
void NotifyDescribe(SchemaInfo& info);
void NotifyApply();                  // the settings changed: what you set written to Windows

// Whether quiet hours have notifications paused now (for Tourne's own pop-ups).
bool NotifyQuietNow();

}  // namespace td
