// Extensions: looks you add to Tourne as data, never programs (the rule from Omarchy: an extension changes how the
// desktop looks, not what it runs). Each is a set of Tourne settings (motion lines, a shader, a view, glass, tiling's
// look, the theme), shown as a card on the Studio's Extensions page and turned on like a preset (Save keeps it).
//
// Tourne's own few are built in. Others are a folder with an extension.json, added from the page (copied into
// %APPDATA%\Tourne\Desktop\extensions\<id>):
//
//     { "id": "arcade", "name": "Arcade", "author": "you", "licence": "MIT", "kind": "Shader", "version": "1.0",
//       "description": "A CRT over every screen.",
//       "settings": { "shader.screen": "crt", "shader.amount": "70" } }
//
// Settings that could run something (binds, menu commands, programs, app styles) are refused, so an extension
// can't, whatever it says.
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace td {

struct Extension {
    std::wstring id, name, author, licence, kind, version, description;
    std::vector<std::pair<std::wstring, std::wstring>> settings;  // (whole keys)
    bool builtIn = false;
    std::wstring folder;  // a user's: where it lives
};

// Tourne's own, then the ones added (read again with reload).
const std::vector<Extension>& ExtensionList(bool reload = false);
std::wstring ExtensionsFolder();
// A setting an extension may set.
bool ExtensionMaySet(const std::wstring& key);
// Copied in from a folder holding extension.json (or the file itself). False with why.
bool ExtensionAdd(const std::wstring& path, std::wstring* error);
// A user's taken out (to the Recycle Bin).
bool ExtensionRemove(const Extension& e);

}  // namespace td
