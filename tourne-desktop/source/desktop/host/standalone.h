// What modules reach for in another part, which the suite gives each part in
// one process: here the build's modules.cpp answers it (from System Icons when
// it's built in and running, else as the suite does when that part is off).
#pragma once

#include <windows.h>

#include <string>
#include <utility>
#include <vector>

namespace part_system_icons {
// A "tourne:" icon design, drawn by System Icons at that size (the caller
// destroys it). Null without it: the caller uses the program's own icon.
HICON DesignIconAt(std::wstring const& icon, int size);
}  // namespace part_system_icons

namespace td {
// Programs Tourne leaves alone (exclusions.cpp): no window effects for shell.exclude, games and Windhawk's
// exclusions; no hooks while one of shell.excludeKeys is in front.
bool ProgramExcluded(HWND window);                     // the program the window belongs to is one
bool ProgramPathExcluded(std::wstring const& path);    // by its full path
bool ExcludedInFront();                                // the window in front is one of shell.excludeKeys
void WatchExcludedFront(void (*callback)(bool inFront));  // called (on another thread) when that changes
// Tourne Studio (host.cpp): at a page ("taskbar."), or with its search box set to query. Any thread.
void OpenStudio(std::wstring const& page, std::wstring const& query = L"");
// Settings written as the Studio writes them (tourne-gui.conf; a backup .before-<why>); names starting with prefix
// that aren't in set are removed first. Read again by the config watch.
bool WriteStudioSettings(std::vector<std::pair<std::wstring, std::wstring>> const& set, std::wstring const& prefix,
                         std::wstring const& why);
// The Tourne Menu's settings window, laid out like LiteMenu's (menu-editor.cpp). Any thread.
void OpenMenuEditor();
}  // namespace td
