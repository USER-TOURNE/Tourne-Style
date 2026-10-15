// Plugins: Noctalia's plugins that run on Windows. desktop/plugins/index.json
// (made by tools/plugins/Make-PluginIndex.ps1, built into the program) lists
// them, each with what it needs and its files at the commit that was scanned;
// plugins found to need Linux programs or paths are left out of it.
//
// Installing one fetches exactly those files from GitHub into
// %LOCALAPPDATA%\Tourne\Plugins\<author>\<name>, after the user agreed to what
// it needs; that agreement is kept beside it (tourne-plugin.json) and is what
// the plugin runtime will allow it.
//
// Tourne's own plugins (desktop/plugins/bundled, listed by its index.json) are
// built into the program instead: installing one writes its files from there,
// and a newer build brings the installed copy up to its version by itself.
#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "config.h"

namespace td {

struct PluginNeed {
    std::wstring kind;   // internet, program, open, files, clipboardRead, clipboardWrite, notify, systemInfo, wallpaper, sound, processes, power, systemSound, reads, deletes, screen
    std::wstring detail; // internet: its hosts (", "); program: its name; reads: what it reads, said for people
    std::vector<std::wstring> paths;  // reads: the folders it may read (as given: "~/.claude", "%LOCALAPPDATA%\\Syncthing")
};

struct PluginInfo {
    std::wstring id, name, version, author, license, description, icon;
    std::wstring repo, rev, path;  // where its files are: github.com/<repo> at <rev>, in <path>
    bool bundled = false;          // Tourne's own: its files are in the program
    int api = 0;
    std::vector<std::wstring> tags, entries, files;
    std::vector<PluginNeed> needs;
};

// The index, read once.
const std::vector<PluginInfo>& PluginCatalogue();
int PluginsLeftOut();  // Linux-only ones the index excluded

// What it needs, said for people: one short line for its card, or one line per need (for asking).
std::wstring PluginNeedsLine(const PluginInfo& p);
std::vector<std::wstring> PluginNeedsList(const PluginInfo& p);
// Where it shows: "Bar", "Panel", "Desktop", "Launcher", "Quick toggle", "Background".
std::vector<std::wstring> PluginSurfaces(const PluginInfo& p);

enum class PluginState { NotInstalled, Installing, Installed, Failed };
PluginState PluginStateOf(const PluginInfo& p);
std::wstring PluginFolder(const PluginInfo& p);
std::wstring PluginError(const PluginInfo& p);  // why the last install failed

// The installed plugins' settings, for the Studio: their defaults as plugins.<id>.<key> (and
// plugins.<id>.enabled, on), and what each is (its name and description in the user's language,
// switches, choices, numbers). Everything under "plugins." in both is replaced.
void PluginSettings(Settings& defaults, SchemaInfo& info);
// Choices read from Windows as they are now (options_from = "sound-outputs": the sound outputs), for the Studio.
void PluginDynamicOptions(SchemaInfo& info);

// Installs in the background; `notify` gets message `msg` when it's done (or failed).
void PluginInstall(const PluginInfo& p, HWND notify, UINT msg);
bool PluginRemove(const PluginInfo& p);

// Installed copies of Tourne's own plugins brought up to this build's version (before the runtime starts them).
void PluginRefreshBundled();

}  // namespace td
