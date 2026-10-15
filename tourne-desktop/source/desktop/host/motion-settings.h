// The motion tree's settings (motion.cpp at the repository's top): their
// defaults (unset, so every part keeps its own animation) and what the
// Studio's Motion page says about each.
#pragma once

#include "config.h"

namespace td {

void MotionDefaults(Settings& out);
void MotionDescribe(SchemaInfo& info);
// Windows' own window animations (opening, closing, minimizing) on or off as windowsIn, windowsOut, fadeIn and
// fadeOut say, while one of them (or a line above them) is set; for this sign-in only. Restore puts back what
// Windows had (when Tourne Desktop quits).
void ApplyWindowsMotion();
void RestoreWindowsMotion();

}  // namespace td
