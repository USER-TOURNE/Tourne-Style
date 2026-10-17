// Key binds, window rules and idle actions as lists with plain choices, no config file needed
// (binds-editor.cpp): the Switcher's switcher.binds.lines, written as Hyprland's.
#pragma once

namespace td {

// Opens it, or brings it to the front (any thread; it runs on its own).
void OpenBindsEditor();
// Opens it on this thread and returns when it closes (--binds-settings with no copy of Tourne Desktop running).
void RunBindsEditor();

}  // namespace td
