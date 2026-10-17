// Tourne's CSS injector: your own style sheets in apps made of web pages
// (Discord, Claude, Spotify, Slack, Obsidian, VS Code, or any Electron or
// Chromium app you point it at). Nothing is injected until you turn an app on
// and say yes to the warning.
//
// How: the app is started by Tourne with Chromium's DevTools port (a random
// port on 127.0.0.1), and Tourne adds a <style> to each of its pages, and to
// every page it opens later, through that port. Saving a style sheet restyles
// the app at once. Apps don't support this, so an update can undo or break
// it; and while the app runs, other programs on this PC could use the same
// port.
//
// Style sheets live in %APPDATA%\Tourne\Desktop\css\. Its "tourne" folder is
// Tourne's own: a style for each app made from your theme's colours, written
// again whenever the theme changes (copy one out of it to change it).
//
// Settings (tourne.conf), per app:
//   inject.discord.on = false          style it (asks first, once)
//   inject.discord.css = tourne/discord.css, mine.css
//   inject.discord.exe =               the app's program, if not found itself
//   inject.discord.restart = ask       opened without styles: ask | always | never
// Any other app: give it a name and its program,
//   inject.myapp.exe = C:\Path\App.exe
#pragma once

#include <windows.h>

#include "config.h"
#include "palette-gen.h"

namespace td {

void InjectorDefaults(Settings& out);
void InjectorStart(HWND host);
void InjectorApply();                          // the config changed
void InjectorThemeChanged(const Roles& roles); // the palette CSS is written again
void InjectorStop();
bool InjectorMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

// The tray menu's "Style apps" items (ids from firstId) and their commands.
void InjectorMenu(HMENU menu, UINT firstId);
bool InjectorMenuCommand(UINT id, UINT firstId);

}  // namespace td
