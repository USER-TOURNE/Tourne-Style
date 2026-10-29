// Designer Mode (designer.cpp): pick a colour off the screen, then click a part of Tourne (the bar, a menu, the
// switcher, a flyout, Tourne'Table, an app the CSS injector styles) and choose which of its colour settings takes it;
// it's written to tourne-gui.conf (a copy kept first) and shows at once. Any part also offers the Tourne theme made
// from that colour, Windows' accent, and copying it.
// From the tray menu, a bind's "designer" or tournectl dispatch designer.
#pragma once

#include <windows.h>

namespace td {

void DesignerStart(HWND host);  // (the host's thread: it runs until the colour is used or Esc)

}  // namespace td
