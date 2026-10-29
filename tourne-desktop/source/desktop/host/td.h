// Tourne Desktop: what the host gives each module (in place of Windhawk's API)
// and what each module gives the host.
#pragma once

#include <windows.h>

#include <cstdarg>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace td {

// One module, as the build writes it (gen/modules.cpp): its settings'
// prefix, its own settings schema (Windhawk's YAML, for the defaults), and
// its lifecycle. afterInit and settingsChanged may be null. besideSuite: a part the Windhawk suite runs
// in other programs and this one runs here, in Tourne Desktop only (the suite stays out of it), with
// the suite's settings (Icon Redirect): it runs while the suite has it on, and has no Studio page.
struct Module {
    const wchar_t* prefix;
    const wchar_t* title;
    const char* settingsYaml;
    BOOL (*init)();
    void (*afterInit)();
    void (*uninit)();
    void (*settingsChanged)();
    bool besideSuite;
};

extern const Module kModules[];
extern const int kModuleCount;

// Settings: "key" is the module's own (prefixed with the module's name) or
// another module's full name ("systemIcons.doubleClickApps").
std::wstring FormatKey(const wchar_t* format, va_list args);
int GetInt(const wchar_t* prefix, const std::wstring& key);
const wchar_t* GetString(const wchar_t* prefix, const std::wstring& key);  // freed with FreeString
bool IsSet(const wchar_t* prefix, const std::wstring& key);  // set by you or the Studio (not just its default)
bool IsSetOrIdentity(const wchar_t* prefix, const std::wstring& key);  // ... or asked for by the identity (identity.h)
void FreeString(const wchar_t* text);
// Writes one of the module's settings into tourne-gui.conf as the Studio would; the config watch then
// reads it back and the module's settingsChanged runs (Tourne'Table's quick-settings panel).
bool WriteSetting(const wchar_t* prefix, const std::wstring& key, const std::wstring& value);

// What a module keeps for itself between runs (Windhawk's "values").
size_t GetStringValue(const wchar_t* prefix, const wchar_t* name, wchar_t* buffer, size_t chars);
BOOL SetStringValue(const wchar_t* prefix, const wchar_t* name, const wchar_t* value);
int GetIntValue(const wchar_t* prefix, const wchar_t* name, int fallback);
BOOL SetIntValue(const wchar_t* prefix, const wchar_t* name, int value);
size_t GetBinaryValue(const wchar_t* prefix, const wchar_t* name, void* buffer, size_t size);
BOOL SetBinaryValue(const wchar_t* prefix, const wchar_t* name, const void* buffer, size_t size);
BOOL DeleteValue(const wchar_t* prefix, const wchar_t* name);
size_t GetStoragePath(const wchar_t* prefix, wchar_t* buffer, size_t chars);

void LogV(const wchar_t* prefix, const wchar_t* format, va_list args);

// Hooks in this process only (hooks.cpp), for besideSuite modules: set while the module starts, put
// in by ApplyHooks once it has, turned off by DisableHooks when it stops (or failed to start).
BOOL SetFunctionHook(const wchar_t* owner, void* target, void* hook, void** original);
void ApplyHooks();
void DisableHooks(const wchar_t* owner);

// Tourne Desktop's own Open and Save dialogs in the shell's look: their footer and right-click menus
// (dialog-look.cpp), what Theme Gaps and Tourne Menus do in other programs.
void DialogLookStart();

// Every setting (defaults and the user's) whose full name starts with prefix.
std::vector<std::pair<std::wstring, std::wstring>> SettingsWithPrefix(const std::wstring& prefix);

// Whether a module ("systemIcons") is running now.
bool ModuleRunning(const wchar_t* prefix);

// Whether a Windhawk mod whose id holds idPart is on in Windhawk now.
bool WindhawkModOn(const wchar_t* idPart);

// Another program's tray icon in System Icons' design (its pixel art, a sensor's reading...), or
// null to show its own (System Icons not running, or no design for it). The caller doesn't destroy it.
HICON TrayDesign(HWND owner, const std::wstring& tip);

// System Icons' flyout colours (0xRRGGBB: ground, raised, line, edge, text, accent, muted, dim, warn,
// bad, hover) into out[11]; false when it isn't running.
bool FlyoutPalette(uint32_t* out);

}  // namespace td
