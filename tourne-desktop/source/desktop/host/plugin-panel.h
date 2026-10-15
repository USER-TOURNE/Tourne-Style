// A plugin's panel: the ui.* tree a [[panel]] entry renders (panel.render),
// laid out and drawn in a popup in the style of System Icons' flyouts (its
// pixel fonts and colours), with buttons, inputs, toggles, selects, sliders,
// pictures (an image's path), scrolling and the rest. An input with an
// onPasteImage handler hears Ctrl+V with a picture and no text copied, one with
// onPasteFiles hears it with files copied (their paths); a node with onDrop
// hears files dropped on it (their paths). plugin-runtime.cpp turns the plugin's Lua tree into
// a Node tree and runs its handlers; this file knows nothing of Lua.
//
// Events never call into the plugin from inside a window's own message (a
// handler may re-render and take that window away): each is handed to the
// runtime with the node's identity (its keys and places from the root), which
// it looks up again in the tree as it is then, so an event on a node the tree
// no longer has does nothing.
#pragma once

#include <windows.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "json-lite.h"

namespace td::panel {

// A callback prop: a plugin global's name, or a function the runtime keeps (its ref).
struct Handler {
    std::string name;
    int ref = -1;
    bool Set() const { return !name.empty() || ref >= 0; }
};

struct Node {
    std::string type;   // column, row, label, button...
    json::Value props;  // everything but the handlers
    std::map<std::string, Handler> on;  // onClick, onHover, onChange...
    std::vector<Node> children;
};

// Where it opens: from the [[panel]] entry.
struct Placement {
    int width = 360, height = 400;
    bool fillHeight = false;            // height = "fill": the screen's full height
    // "attached": near what was clicked; "floating": at `position`; "fullscreen": centred over a dimmed, blurred
    // copy of the screen (a click on it or Esc closes it).
    std::string placement = "attached";
    std::string position;                // floating: "center_right", "top_left"...
    POINT anchor{};                      // the pointer when it was asked for
    std::wstring remember;               // a name to keep its size, place and pin under ("author/plugin:panel"); empty: none
    // A desktop widget ([[desktop_widget]]): on the desktop, under every window, as big as what it shows (width:
    // the most it takes); dragged by any part that isn't a control; never closes by itself. anchor: where it
    // goes the first time (its top right corner).
    bool desktop = false;
    // A panel's capture_keys ("Tab", "Shift+Tab", "Backtab", "Return", "Ctrl+K"): pressed and released, they go
    // to Host::key instead of what the panel would do with them. In a text box, only keys a text box doesn't
    // use itself (Escape, Tab, Up, Down, Page Up and Down, F keys, Ctrl or Alt chords other than editing's).
    std::vector<std::string> captureKeys;
};

// The runtime's side.
struct Host {
    // Runs a node's handler, found again by identity; args arrive as strings.
    std::function<void(const std::string& identity, const std::string& handler, std::vector<std::string> args)> event;
    // The user closed it (Esc, a click elsewhere unless it's pinned): the runtime calls onClose and drops it.
    // A desktop widget: the user took it off the desktop (its right-click menu).
    std::function<void()> closed;
    // A role ("primary", "on_surface/0.6") or "#rrggbb" as a colour; CLR_INVALID if it isn't one.
    std::function<COLORREF(const std::string&)> color;
    // A capture_keys chord, as the manifest spells it, pressed or released (Placement::captureKeys).
    std::function<void(const std::string& chord, bool pressed)> key;
};

class Window {
public:
    static std::unique_ptr<Window> Open(const Placement& where, Host host);
    virtual ~Window() = default;
    virtual void Render(Node tree) = 0;
    // The handler a node now has, by identity (null if the tree no longer has it).
    virtual const Handler* Find(const std::string& identity, const std::string& handler) const = 0;
    // Every function ref the current tree holds (the runtime releases them when it's replaced).
    virtual std::vector<int> Refs() const = 0;
    virtual HWND Hwnd() const = 0;
    // Puts text where the caret is in the input with this key, as if typed (its onChange follows); own line:
    // on a line of its own. False if no input has that key.
    virtual bool InsertText(const std::string& key, const std::string& text, bool ownLine) = 0;
};

// A picture file as premultiplied BGRA pixels, top-down: what WIC reads (PNG, JPEG, BMP, GIF, ICO, WebP with
// its codec) at most `longest` pixels on its longer side, or an SVG drawn by Direct2D (shapes, paths, strokes,
// fills, gradients, opacity; no <text>) at `longest` on its longer side. natural: its own size in 96-dpi pixels
// (an SVG's width and height, else its viewBox's). False if it doesn't read.
bool ReadPicturePixels(const std::wstring& path, int longest, std::vector<uint32_t>* pixels, UINT* w, UINT* h,
                       double* naturalW = nullptr, double* naturalH = nullptr);

// Wallpaper depth (noctalia.setWallpaperMask): on the screen named (as noctalia.outputs() names it, "DISPLAY1"),
// desktop widgets are drawn under the wallpaper's foreground: where the mask (a grey picture aligned with the
// wallpaper's own pixels) is white, the wallpaper shows over them. An empty mask clears it. Any thread.
void SetDepthMask(const std::wstring& screen, const std::wstring& maskPath, const std::wstring& wallpaperPath);

// How a glyph's name ("world") becomes its character in the glyph font (the runtime's map).
void SetGlyphLookup(std::function<std::wstring(const std::string&)> lookup);

// When the user last closed a panel by clicking elsewhere (so a click on its own bar item, which
// closed it, doesn't open it again at once).
ULONGLONG LastDismissed();

}  // namespace td::panel
