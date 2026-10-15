// Tourne Studio: every Tourne Desktop setting in one window, the way HyprMod
// shows Hyprland's. Opened from the tray menu or with --studio; nothing of it
// runs until then.
//
// A change shows at once (it goes to the host's live layer, so the module
// redraws), but is only kept when you save: then it's written to
// tourne-gui.conf next to tourne.conf. Your own tourne.conf is never written;
// what it sets shows as yours, and a Studio value for the same setting wins
// over it (tourne-gui.conf is read after it). Undo and redo go back and
// forward through the changes not yet saved.
//
// Any colour can be one of the theme's roles ("@accent", "@ground"...): the
// host puts in the colour the theme has now, and again when it changes.
#pragma once

#include <windows.h>

#include <cstdint>
#include <set>
#include <string>

#include "config.h"

namespace td {

// What the host gives the Studio (host.cpp).
const Settings& DefaultSettings();
const SchemaInfo& SettingsSchema();
void RefreshPluginSettings();  // the installed plugins' settings read again (after an install or removal)
void RefreshDynamicOptions();  // the choices parts list as they run (the switcher's styles) read again
void RefreshPluginChoices();   // plugins' choices Windows gives (the sound outputs) read again
void ConfigLayers(Settings* user, Settings* gui);  // tourne.conf and tourne-gui.conf as read last
// Shown now, not kept: set over everything, removed taken out of tourne-gui.conf's.
void SetLiveLayer(const Settings& set, const std::set<std::wstring>& removed);
// Writes tourne-gui.conf (and clears the live layer). False if it couldn't.
bool SaveGuiLayer(const Settings& gui);
std::wstring ConfigFolder();

// The theme's roles as the Studio and the host name them, "@" and all.
struct ThemeRole {
    const wchar_t* token;  // "@accent"
    const wchar_t* value;  // its value name in HKCU\Software\Tourne\Theme
    const wchar_t* label;  // "Accent"
};
extern const ThemeRole kThemeRoles[];
extern const int kThemeRoleCount;
// The theme's colour for a role now (0xRRGGBB); false if there's no theme yet.
bool ThemeRoleColor(const ThemeRole& role, uint32_t* out);

// What the host calls.
void StudioOpen(HWND host);
bool StudioWriteKinds(const std::wstring& path);  // --studio-kinds: every setting and its control, for checking
void StudioOpenAt(HWND host, const std::wstring& page, const std::wstring& query);  // a page ("taskbar."), or a search
void StudioConfigReloaded();  // the files were read again: what's saved may differ
bool StudioClose();           // the host is quitting: false if the user kept it open

}  // namespace td
