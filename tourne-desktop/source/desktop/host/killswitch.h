// Windows' taskbar and Start kept away while Tourne Desktop runs, from
// outside Explorer (nothing of Tourne loads into it):
//
//   - Windows' taskbar is set to auto-hide (what it was is kept, and put back
//     when this is turned off, Tourne Desktop quits, or with --restore).
//   - A sliver of a window, invisible, lies over the screen edge where it
//     waits, so pointing there doesn't bring it up.
//   - The keys that open it are caught: Ctrl+Esc, Win+T, Win+B, Win+1..0, and
//     a lone Windows key (unless Tourne's Start menu has that one).
//   - All of it again when Explorer restarts.
//
// What it can't stop from outside: an app flashing for attention can make the
// taskbar peek. Off by default. Idle cost: none (it wakes on keys while the
// Windows key is held, a window coming to the front, or Explorer restarting).
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

void KillswitchDefaults(Settings& out);
void KillswitchDescribe(SchemaInfo& info);
void KillswitchStart(HWND host);
void KillswitchApply();  // the settings changed
void KillswitchStop();   // Windows' taskbar back as it was
bool KillswitchMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

// Puts back the taskbar state a killswitch saved (after a crash: --restore).
void KillswitchRestoreSaved();

}  // namespace td
