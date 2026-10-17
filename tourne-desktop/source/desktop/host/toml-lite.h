// Just enough TOML for a Noctalia plugin's plugin.toml: keys, strings, numbers,
// booleans, arrays, inline tables, [table] and [[array of tables]] headers
// (nested ones like [[widget.setting]] go into the last [[widget]]). Read into
// the same tree as json-lite, so the plugin runtime handles one kind of value.
#pragma once

#include <string>

#include "json-lite.h"

namespace td::toml {

// False (and where it stopped) if the text isn't TOML this reads.
bool Parse(const std::string& text, json::Value* out, std::string* error = nullptr);

}  // namespace td::toml
