// Tourne theme: one record of colours every Tourne piece can follow, made
// here from the wallpaper or one seed colour.
//
// The record is HKCU\Software\Tourne\Theme: each colour role as a DWORD
// (0xRRGGBB), what it was made from, and Revision, written last. After a new
// record, "TourneThemeChanged" is posted to every top-level window; the
// suite's parts set to "Tourne theme" read it again then (theme-palette.cpp).
//
// Settings (tourne.conf):
//   theme.source   off | wallpaper | seed      (off: the record is left as it is)
//   theme.profile  adaptive | monochrome | muted | vivid | pastel
//   theme.mode     auto | dark | light          (auto: as Windows' own mode)
//   theme.seed     #RRGGBB                      (for source = seed)
//   theme.contrast 4.5                          (text on grounds, WCAG; 7 for AAA)
//
// Idle cost: none. The wallpaper folder is watched by Windows on a thread that
// sleeps until it changes, and a picture is read once, on its own thread.
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

// The theme settings and their defaults (for defaults.conf and lookups).
void ThemeDefaults(Settings& out);

void ThemeEngineStart(HWND host);  // after the config is loaded
void ThemeEngineApply();           // the config changed
void ThemeEngineStop();

// The host window's messages the engine wants (Windows' setting changes, its
// own timer and results). True if handled.
bool ThemeEngineMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

}  // namespace td
