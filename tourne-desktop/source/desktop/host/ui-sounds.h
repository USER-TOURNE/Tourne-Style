// UI sounds (ui-sounds.cpp): short Foley cues (desktop/sounds/foley, built in) for Tourne Desktop's own moments:
// a menu opening, the pointer moving over its items, the launcher opening, and whatever plugins ask for with
// tourne.uiSound(name). Off by default (sounds.enabled).
//
// Idle cost: none while off. On: a thread that waits for messages and Windows' menu events from this process
// only; a sound is played with PlaySound from memory, so no audio stream stays open between sounds.
#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "config.h"

namespace td {

void UiSoundsDefaults(Settings& out);
void UiSoundsDescribe(SchemaInfo& info);
void UiSoundsStart(HWND host);
void UiSoundsApply();  // the settings changed
void UiSoundsStop();

// Plays a cue by its name ("pop", "tick"...) when sounds are on; any thread, never waits.
void UiSound(const std::wstring& name);
// A moment's cue as set ("launcher" -> sounds.launcher), played when sounds are on.
void UiSoundFor(const wchar_t* moment);
// Whether plugins' tourne.uiSound plays (sounds on, and allowed for plugins).
bool UiSoundsForPlugins();
// The cues built in, by name.
std::vector<std::wstring> UiSoundNames();

}  // namespace td
