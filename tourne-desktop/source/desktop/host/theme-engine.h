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

#include <cstdint>

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

// Windows' accent set to one colour (0xRRGGBB) as theme.windows sets it, and every window told (Designer Mode).
void WriteWindowsAccent(uint32_t rgb);
// Windows' colours (its accent, and its dark or light mode) as they were before Tourne first changed them, kept in
// HKCU\Software\Tourne\AccentBefore the first time Designer Mode or theme.windows writes them: whether there is
// a copy, and putting it back (the copy is then dropped; wait: until every window has been told, for an exit).
// Also put back on its own when theme.windows goes from on to off, and by --uninstall.
bool WindowsAccentSaved();
bool RestoreWindowsAccent(bool wait = false);
bool WindowsColoursOn();      // theme.windows is accent or both
void KeepWindowsColoursOnce();  // the next switch-off of theme.windows doesn't put them back (Designer's own accent)

}  // namespace td
