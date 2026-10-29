// Tourne glass in Tourne Desktop: the engine (glass-engine.h) put to use.
//
//   Looks        Clear, Liquid, Frosted, Aero, Smoke, Prism, Oil: a starting
//                point for every setting; a setting you set yourself wins.
//   Test window  a glass card you can drag over the wallpaper (from the Studio).
//   The Studio   wears the glass itself.
//   Rims         a glass rim round the window in front: edge bend, fringe,
//                glow and highlight, a glow when it comes to the front.
//   Apps         see-through glass inside chosen apps: Windows' own Mica,
//                Acrylic or tabbed backdrop, or "frosted" (acrylic blur behind
//                and the window made see-through), each app on its own.
//
// All of it is off until glass.enabled is on (the test window and the Tuner
// work before that, to try a look). Idle cost: none. Nothing animates unless
// something just changed (a move, a focus) or the oil film is on, and then
// only for a few seconds after.
#pragma once

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

#include "config.h"

namespace td {

void GlassDefaults(Settings& out);
void GlassDescribe(SchemaInfo& info);  // names, choices and limits, for the Studio
void GlassStart(HWND host);
void GlassApply();  // the settings changed
void GlassStop();
bool GlassMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

// Glass in admin apps (glass.yours.admin): a helper with admin rights, started by a logon task.
void GlassAdminSync();                    // start, update or stop the helper to match the settings
int GlassAdminSetup();                    // --glass-admin-setup (elevated): make the logon task and start it
int GlassAdminRun(bool (*reload)());      // --glass-admin (elevated): the helper itself

// TourneDesktop.exe --glass-preview out.png [--wallpaper picture] [--look id|all]
int GlassPreview(const std::wstring& args);

// The looks, for the Studio: each sets these settings (names under "glass.").
struct GlassLookPreset {
    const wchar_t* id;
    const wchar_t* label;
    std::vector<std::pair<const wchar_t*, const wchar_t*>> values;
};
const std::vector<GlassLookPreset>& GlassLooks();
// A look's value for a setting ("glass.blur"); empty if the look doesn't set it.
std::wstring GlassLookValue(const std::wstring& look, const std::wstring& key);
bool GlassIsLookSetting(const std::wstring& key);

void GlassTestWindow(bool show);
bool GlassTestWindowShown();
void GlassCompare(bool holding);  // glass off everywhere while held

// The Studio's own glass: the window's client area as glass, if it's on.
bool GlassStudioOn();
bool GlassForWindow(HWND wnd, std::vector<uint32_t>* pixels, int* width, int* height);
void StudioGlassChanged();  // (studio.cpp) the glass changed: draw it again

}  // namespace td
