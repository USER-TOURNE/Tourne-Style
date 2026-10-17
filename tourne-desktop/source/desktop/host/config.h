// Tourne Desktop's settings, read without Windows (so they can be tested
// anywhere): each module's defaults from its own Windhawk settings schema, and
// the user's config file over them.
#pragma once

#include <map>
#include <string>
#include <vector>

namespace td {

using Settings = std::map<std::wstring, std::wstring>;  // "taskbar.autoHide" -> "1"

std::wstring Utf8ToWide(const std::string& text);
std::string WideToUtf8(const std::wstring& text);

// What the schema says about a setting or a group, for Tourne Studio: its
// name and description as the user reads them, its choices (value, label),
// and limits from "#! $min" / "#! $max".
struct SettingInfo {
    std::wstring name, description;
    std::vector<std::pair<std::wstring, std::wstring>> options;
    bool isSwitch = false;  // written true or false in the schema
    std::wstring format;    // a "#! $format" hint ("hotkey"): how the Studio asks for it
    bool hasMin = false, hasMax = false;
    double min = 0, max = 0;
    int order = 0;  // where the schema has it (so pages read in its order)
};
using SchemaInfo = std::map<std::wstring, SettingInfo>;  // full name (settings and groups) -> info

// A module's settings schema (the YAML between Windhawk's settings markers) as
// "prefix.group.key" -> default. Switches become "1" or "0"; lists become
// "key[0]", "key[1]" and "key[0].field". With info, what the schema says
// about each setting and group as well.
void ParseSchemaDefaults(const std::wstring& prefix, const std::string& yaml, Settings& out, SchemaInfo* info = nullptr);

// The user's config: "key = value" lines, groups as "name { ... }" (as
// Hyprland writes them) or dotted names, "#" comments. Lines it can't read are
// reported in errors (with their line number) and skipped.
void ParseConfig(const std::wstring& text, Settings& out, std::vector<std::wstring>* errors);

// A setting as a number: decimal, 0x hex, or true/false/on/off/yes/no.
int ToInt(const std::wstring& value);

// Writes settings back as config lines (for the defaults list and imports).
std::wstring WriteConfig(const Settings& settings, const std::wstring& header);

}  // namespace td
