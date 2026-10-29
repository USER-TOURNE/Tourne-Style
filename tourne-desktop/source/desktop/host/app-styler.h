// App Styler (app-styler.cpp): Windows' own XAML apps (Settings, Windows Security, Calculator...) in the Tourne
// theme's colours, Tourne's font and square corners. As one of the listed programs opens its first window,
// tourne-appstyler-attach.exe (beside Tourne Desktop) puts TourneAppStyler.dll into it through Windows' XAML
// diagnostics, the way Visual Studio's live inspector does: no hooks, nothing in any other program, and the app's
// colours are set once (tools/appstyler/tap.cpp), so it runs as fast as it did. Closing the app puts it back.
//
// Idle cost: one window event watcher (out of context) that looks at a new window's program only once per
// process.
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

void AppStylerDefaults(Settings& out);
void AppStylerDescribe(SchemaInfo& info);
void AppStylerStart(HWND host);
void AppStylerApply();  // the settings changed (the next app opened takes them)
void AppStylerStop();

}  // namespace td
