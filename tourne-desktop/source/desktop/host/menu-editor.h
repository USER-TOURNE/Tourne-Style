// The Tourne Menu's settings window (menu-editor.cpp), laid out like LiteMenu's own configurator: the
// menu as a tree with its submenus, the picked item's name, command and parameters beside it, and how
// the menu opens on a second tab.
#pragma once

namespace td {

// Opens it, or brings it to the front (any thread; it runs on its own).
void OpenMenuEditor();
// Opens it on this thread and returns when it closes (--menu-settings with no copy of Tourne Desktop running).
void RunMenuEditor();

}  // namespace td
