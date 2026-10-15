// Keystrokes: the keys you press and the clicks you make, shown on screen as keycaps, for screencasts and
// demos (keystrokes.cpp). The best of two DankMaterialShell plugins: Tz-slayer's Keystrokes (Keyviz's keycaps:
// combinations grouped on one row, press counts, four keycap styles, five animations, nine places) and
// hthienloc's Screenkey (clicks, a quick on and off).
//
// Off by default. On, it costs a low-level keyboard hook (and a mouse hook, when clicks are shown) and a window
// drawn only while keycaps come, go or show; with nothing pressed nothing runs. Off, or toggled away, nothing
// of it is there. Paused in programs named (password managers) and in password boxes.
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

void KeystrokesDefaults(Settings& out);
void KeystrokesDescribe(SchemaInfo& info);
void KeystrokesStart(HWND host);
void KeystrokesApply();  // the settings changed
void KeystrokesStop();
// Shown or not for now, whatever the setting says until it changes: 1 on, 0 off, -1 the other way round.
// Returns whether it shows afterwards. Any thread.
bool KeystrokesShow(int how);
bool KeystrokesShowing();

}  // namespace td
