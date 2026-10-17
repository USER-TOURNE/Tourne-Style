// Dots: your whole look as one .tourne file, and a look read back from one, or from a Hyprland config
// (dots.cpp). A .tourne file is Tourne's own settings lines (tourne-gui.conf's, without anything tied to this
// PC: no paths, no programs) and the extensions you added, inline. From a hyprland.conf: its animations and
// curves, gaps, borders and their gradients, rounding, layout and shadows, onto Tourne's.
//
// Like extensions, a look never runs anything: what reads in passes ExtensionMaySet (extensions.h) or a short
// list of look settings, and the Studio shows what it changes before anything does; a copy of tourne-gui.conf is
// kept first, and Undo (before Save) or that copy puts the old look back.
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace td {

struct DotsLook {
    std::wstring name, author, from;  // from: "Tourne" or "Hyprland"
    std::vector<std::pair<std::wstring, std::wstring>> settings;
    std::vector<std::string> extensions;  // extension.json texts (UTF-8)
    std::vector<std::wstring> skipped;    // what wasn't taken, and why
};

// Your look, written to path. False with why.
bool DotsExport(const std::wstring& path, const std::wstring& name, std::wstring* error);
// A .tourne file or a Hyprland config read. False with why.
bool DotsRead(const std::wstring& path, DotsLook* out, std::wstring* error);

}  // namespace td
