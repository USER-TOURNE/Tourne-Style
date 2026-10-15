// The plugin runtime: runs installed Noctalia plugins (plugins.h installs them)
// with the Luau VM (desktop/third_party/luau), on a thread of its own.
//
// Each entry of a plugin (its [[service]], each [[widget]]) gets a VM of its
// own, as in Noctalia; they share only noctalia.state. Bar widgets go to the
// taskbar through plugin-bar-shared.cpp. A plugin is allowed only what its
// tourne-plugin.json says the user agreed to on install: the internet (and to
// which hosts), saving files in its own data folder, the clipboard,
// notifications, and the rest of its needs. Bar widgets, panels and desktop
// widgets run; quick toggles and launcher providers don't yet.
#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace td {

void PluginRuntimeStart();
void PluginRuntimeStop();
// Installed plugins read again (after an install or a removal): every one starts again.
void PluginRuntimeReload();
// Plugins' settings changed: running entries with onConfigChanged are told, the others start again with
// them; a plugin turned on or off starts or stops. The rest keep running.
void PluginRuntimeSettingsChanged();

// A message to the running copy's plugins, as Noctalia's `noctalia msg` has them: {"plugin",
// "author/plugin:entry", "all", event [, payload...]} calls onIpc(event, payload) in that entry (every entry
// without ":entry"); {"panel-toggle", "author/plugin:panel"} opens or closes a panel. 0 when it was delivered.
int PluginMessage(const std::vector<std::wstring>& words);

// The launcher's providers ([[launcher_provider]] entries, running): onQuery(text) answered with
// launcher.setResults(text, results), onActivate(id) for the one picked. Any thread may call these; results come
// back as the listener's message (posted, no data), then LauncherResults reads them.
struct LauncherProvider {
    std::wstring key, name, prefix, glyph;  // key: "author/plugin:entry"
    bool global = false;                     // its results join a search without its prefix
};
struct LauncherItem {
    std::wstring provider, id, title, subtitle, glyph;
    double score = 0;
};
std::vector<LauncherProvider> LauncherProviders();
void LauncherQuery(const std::wstring& key, const std::wstring& text);
std::vector<LauncherItem> LauncherResults(const std::wstring& key, const std::wstring& text);
void LauncherActivate(const std::wstring& key, const std::wstring& id);
void LauncherSetListener(HWND wnd, UINT msg);
// Its glyph font's character for a glyph name ("device-gamepad-2"), empty if there's none; the font is
// "tabler-icons", loaded for this process.
std::wstring LauncherGlyph(const std::wstring& name);

// For tools/plugins/pluginrun.cpp: what each running entry last showed or
// logged, as one line each.
std::vector<std::wstring> PluginRuntimeReport();

}  // namespace td
