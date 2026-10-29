// Tiling layouts people write themselves: a .luau file in
// %LOCALAPPDATA%\Tourne\Layouts that is handed the screen and the windows and
// answers with where each window goes.
//
// A layout is the one part of the tiling that is pure arithmetic, so it is the
// one part worth opening up: nothing it is given can reach a window, a file or
// the internet, and nothing it answers with can do more than move a window
// inside the screen it was given. The plugins (plugin-runtime.cpp) get a VM
// each and a long list of what they are allowed; a layout gets a VM with
// Luau's own sandbox over it and nothing else at all.
//
// The file defines a function called layout, or returns one:
//
//     function layout(ask)
//         local out = {}
//         for i = 1, ask.count do
//             out[i] = {x = ..., y = ..., w = ..., h = ...}
//         end
//         return out
//     end
//
// It is called on the thread doing the tiling, so it has a few milliseconds
// and a memory limit, and a script that runs long is stopped and said so in
// the log. One that stops, or answers with anything but a rectangle for every
// window, is ignored for that call: the tiling lays the windows out in columns
// instead, so a broken layout never leaves windows in a heap.
#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace td {

struct LayoutScriptWindow {
    RECT rect{};  // where it is now
    std::wstring program, cls, title;
    bool focused = false;
};

struct LayoutScriptAsk {
    std::wstring script;   // the file, with or without .luau
    std::wstring options;  // the rest of the setting's line, the script's own to read
    RECT screen{};         // the room to lay out in, the gaps around it already taken off
    int gap = 0;           // the gap between windows, in pixels for this screen
    int ratio = 50;        // the main window's share, in percent
    std::vector<LayoutScriptWindow> windows;
};

// A rectangle for every window, inside the screen with room in it. Empty when
// there is no such script, when it stopped, or when it answered with something
// else: the caller lays the windows out its own way then.
std::vector<RECT> LayoutScriptRects(LayoutScriptAsk const& ask);

// The folder, made with the examples in it the first time anything asks.
std::wstring LayoutScriptsFolder();
// The .luau files in it, without the extension.
std::vector<std::wstring> LayoutScriptNames();
// Why that script last failed, for Studio and the log. Empty when it didn't.
std::wstring LayoutScriptProblem(std::wstring const& script);

}  // namespace td
