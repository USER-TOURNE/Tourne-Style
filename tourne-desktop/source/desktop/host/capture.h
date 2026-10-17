// Screen capture on the Print Screen key, in place of Snipping Tool (capture.cpp):
//   tap          the screen under the pointer, as a picture;
//   double tap   pick a window (click it; Esc gives up), as a picture;
//   hold         record: a bar at the top of the screen (each screen, all of them, a window), then the key again
//                (or the red dot) stops. With Screen Toolkit running, it records (its settings).
// Each lands on the clipboard at once for Ctrl+V (a picture as one, a recording as its file) and is kept in
// Pictures\Screenshots or Videos\Captures. Recording uses ffmpeg (installed, or the copy Convertify brings).
// Only a plain Print Screen is taken: with Ctrl, Alt, Shift or Windows held it goes on as before.
//
// Idle cost: a keyboard hook that looks at one key and lets every other through at once.
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

void CaptureDefaults(Settings& out);
void CaptureDescribe(SchemaInfo& info);
void CaptureStart(HWND host);
void CaptureApply();  // the settings changed
void CaptureStop();
// Screen Toolkit (tourne.captureLink): while it runs, a recording goes to it (its codec, sound, frame rate and
// folder, and its copy to the clipboard), and pictures go to its screenshot folder. state: "idle", "recording",
// "converting", "ready", or "off" when it stops.
void CaptureToolkit(const std::wstring& state, const std::wstring& pictures);

}  // namespace td
