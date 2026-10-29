// Dots: your whole look as one .tourne file, and a look read back from one, or from a Hyprland config
// (dots.cpp). A .tourne file is Tourne's own settings lines (tourne-gui.conf's, without anything tied to this
// PC: no paths, no programs) and the extensions you added, inline. From a hyprland.conf: its animations and
// curves, gaps, borders and their gradients, rounding, layout and shadows, onto Tourne's.
//
// Like extensions, a look never runs anything: what reads in passes ExtensionMaySet (extensions.h) or a short
// list of look settings, and the Studio shows what it changes before anything does; a copy of tourne-gui.conf is
// kept first, and Undo (before Save) or that copy puts the old look back.
//
// A look pack (.tournelook) is the same .tourne file with what it can't hold beside it, zipped (miniz): the
// wallpaper, and the list of plugins the look was made with. Only those three names are read out of one
// (look.tourne, wallpaper.<picture type>, plugins.txt, preview.png); anything else in the zip is left where it is.
//
// The look gallery is a folder of packs, %APPDATA%\Tourne\Desktop\looks: each NAME.tournelook with NAME.png
// beside it, the screen as it looked when saved (so Explorer's big icons show the gallery as it is). Saved from
// the tray menu, a bind's "looksave" or tournectl dispatch looksave; the Studio's Import and Export open there.
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
    // From a look pack (.tournelook) only:
    std::wstring wallpaper;               // its picture, unpacked to %LOCALAPPDATA%\Tourne\Looks\<name> ("" for none)
    std::vector<std::wstring> plugins;    // the ids of the plugins it was made with
    std::wstring preview;                 // its screenshot, unpacked beside the wallpaper ("" for none)
};

// Your look, written to path: a look pack when it ends in .tournelook, else a .tourne file. False with why.
bool DotsExport(const std::wstring& path, const std::wstring& name, std::wstring* error);
// A look pack, a .tourne file or a Hyprland config read. False with why.
bool DotsRead(const std::wstring& path, DotsLook* out, std::wstring* error);

// The gallery's folder (made if it isn't there).
std::wstring DotsGalleryFolder();
// Your look saved into the gallery as a pack, with the screen as it is now; name "" for one from the date and
// time. saved: the pack's path. False with why.
bool DotsSaveToGallery(std::wstring name, std::wstring* saved, std::wstring* error);

}  // namespace td
