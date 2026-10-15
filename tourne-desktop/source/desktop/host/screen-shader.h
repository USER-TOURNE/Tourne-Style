// Screen shader: one of Tourne's effects (screen-effects.h: CRT, VHS, grain, dither...) over a whole screen,
// the ShaderGlass idea (screen-shader.cpp). Each screen is read with Windows' Desktop Duplication, run through
// the effect on the graphics card, and shown in a click-through window on top that Windows leaves out of every
// capture (so it never reads itself, and recordings and screenshots stay clean).
//
// Off by default. On, it draws whenever the screen changes (and every frame for the effects that move), so it
// costs some GPU time; it steps aside while a full-screen program (a game, a video) is in front, unless told
// not to.
#pragma once

#include <windows.h>

#include "config.h"

namespace td {

void ScreenShaderDefaults(Settings& out);
void ScreenShaderDescribe(SchemaInfo& info);
void ScreenShaderStart(HWND host);
void ScreenShaderApply();  // the settings changed
void ScreenShaderStop();

// For tests: one screen's next frame through the effect, saved as a 32-bit BMP instead of shown. 0 when done.
int ScreenShaderShot(const std::wstring& effect, int monitorIndex, const std::wstring& path);

}  // namespace td
