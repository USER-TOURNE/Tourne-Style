// Tourne Desktop: runs the suite's modules that need no hooks (the taskbar,
// the switcher with its tiling and borders) as one small program of its own,
// with a text config that applies the moment it's saved.
//
//   TourneDesktop.exe              start (one copy at a time)
//   TourneDesktop.exe --studio     open Tourne Studio (starting it if it isn't running)
//   TourneDesktop.exe --menu-settings  the Tourne Menu's settings (in the running copy, else on their own)
//   TourneDesktop.exe --binds-settings key binds, window rules and idle actions (the same way)
//   TourneDesktop.exe --glass-preview out.png [--wallpaper picture] [--look id|all] [--effect name|all]
//                                  the glass looks drawn on a picture, to a PNG
//   TourneDesktop.exe --shader-shot crt 2 out.bmp   screen 2 (left to right) through a screen-shader effect, to a picture
//   TourneDesktop.exe --import     your Windhawk suite settings into tourne-gui.conf (--import b53: that build),
//                                  and your Windhawk Tourne'Table setups into the Table here (--import-table: only those)
//   TourneDesktop.exe --install    start with Windows      --uninstall  stop that
//   TourneDesktop.exe --msg plugin author/plugin:entry all event [payload]
//                                  onIpc(event, payload) in a running plugin, as Noctalia's `noctalia msg`
//                                  (--msg panel-toggle author/plugin:panel opens or closes a panel)
//   TourneDesktop.exe --stop       stop the running copy
//   TourneDesktop.exe --restore    Windows' taskbar back as it was (after a crash left it hidden)
//
// Idle cost: none of its own. The config is watched by Windows
// (ReadDirectoryChangesW) on a thread that sleeps until the file changes; the
// tray icon is a window that only answers messages. The modules keep their
// own cost, as in the suite.

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sddl.h>
#include <tlhelp32.h>
#include <mmdeviceapi.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "config.h"
#include "td.h"
#include "theme-engine.h"
#include "css-injector.h"
#include "plugin-runtime.h"
#include "plugins.h"
#include "studio.h"
#include "glass.h"
#include "crash.h"
#if __has_include("build-stamp.h")
#include "build-stamp.h"
#endif
#ifndef TD_BUILD
#define TD_BUILD L"(not stamped)"
#endif
#include "identity.h"
#include "killswitch.h"
#include "keystrokes.h"
#include "screen-shader.h"
#include "capture.h"
#include "launcher.h"
#include "flyouts.h"
#include "app-styler.h"
#include "designer.h"
#include "dots.h"
#include "ui-sounds.h"
#include "shell-mode.h"
#include "notifications.h"
#include "presets.h"
#include "desktop-host.h"
#include "motion-settings.h"
#include "tray-host.h"
#include "menu-editor.h"
#include "binds-editor.h"

namespace td {

////////////////////////////////////////////////////////////////////////////////
// Where things live

static std::wstring KnownFolder(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_CREATE, nullptr, &path))) out = path;
    CoTaskMemFree(path);
    return out;
}

static std::wstring ConfigDir() {
    static std::wstring dir = [] {
        std::wstring d = KnownFolder(FOLDERID_RoamingAppData) + L"\\Tourne\\Desktop";
        SHCreateDirectoryExW(nullptr, d.c_str(), nullptr);
        return d;
    }();
    return dir;
}

static std::wstring DataDir() {
    static std::wstring dir = [] {
        std::wstring d = KnownFolder(FOLDERID_LocalAppData) + L"\\Tourne\\Desktop";
        SHCreateDirectoryExW(nullptr, d.c_str(), nullptr);
        return d;
    }();
    return dir;
}

static std::wstring ConfigPath() { return ConfigDir() + L"\\tourne.conf"; }
static std::wstring DefaultsPath() { return ConfigDir() + L"\\defaults.conf"; }
static std::wstring GuiPath() { return ConfigDir() + L"\\tourne-gui.conf"; }  // Tourne Studio's
std::wstring ConfigFolder() { return ConfigDir(); }

static bool ReadText(const std::wstring& path, std::wstring* out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                           OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    std::string bytes;
    LARGE_INTEGER size{};
    GetFileSizeEx(f, &size);
    bytes.resize((size_t)size.QuadPart);
    DWORD read = 0;
    bool ok = !size.QuadPart || ReadFile(f, bytes.data(), (DWORD)bytes.size(), &read, nullptr);
    CloseHandle(f);
    if (!ok) return false;
    bytes.resize(read);
    if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF)
        bytes.erase(0, 3);
    *out = Utf8ToWide(bytes);
    return true;
}

static bool WriteText(const std::wstring& path, const std::wstring& text) {
    std::string bytes = WideToUtf8(text);
    std::wstring current;
    if (ReadText(path, &current) && current == text) return true;  // unchanged: not written (no reload, no churn)
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    bool ok = WriteFile(f, bytes.data(), (DWORD)bytes.size(), &written, nullptr) && written == bytes.size();
    CloseHandle(f);
    return ok;
}

////////////////////////////////////////////////////////////////////////////////
// Log

static std::mutex g_logLock;
static bool g_logToFile = false;

void LogV(const wchar_t* prefix, const wchar_t* format, va_list args) {
    wchar_t text[2048];
    int n = _snwprintf(text, ARRAYSIZE(text) - 1, L"[%s] ", prefix);
    if (n < 0) n = 0;
    _vsnwprintf(text + n, ARRAYSIZE(text) - 1 - n, format, args);
    text[ARRAYSIZE(text) - 1] = 0;
    std::wstring line = text;
    for (auto& c : line)  // (one line each: a balloon's text can have several)
        if (c == L'\r' || c == L'\n') c = L' ';
    CrashNote(line);  // (the last lines go in a crash report, logging on or not)
    line += L"\n";
    OutputDebugStringW(line.c_str());
    if (!g_logToFile) return;
    std::lock_guard<std::mutex> lock(g_logLock);
    std::wstring path = DataDir() + L"\\desktop.log";
    HANDLE f = CreateFileW(path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t stamp[32];
    _snwprintf(stamp, ARRAYSIZE(stamp), L"%02d:%02d:%02d.%03d ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    std::string bytes = WideToUtf8(stamp + line);
    DWORD written;
    WriteFile(f, bytes.data(), (DWORD)bytes.size(), &written, nullptr);
    CloseHandle(f);
}

static void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"desktop", format, args);
    va_end(args);
}

////////////////////////////////////////////////////////////////////////////////
// Settings: every module's defaults, with the user's config over them, read
// by the modules' threads from a snapshot that's swapped whole on a reload.
// The config is layers, each over the one before: tourne.conf (yours),
// tourne-gui.conf (Tourne Studio's), and the Studio's changes not yet saved
// (the live layer, never written). Theme roles ("@accent") in a value are
// put in last, as the colours the theme has now.

struct Store {
    Settings values;  // what the modules read: the layers merged, roles put in
    Settings user, gui;
    Settings suite;     // the parts that stay in Windhawk, as set there (ReadSuiteLayer): over the rest
    Settings identity;  // what the identity (identity.h) asks of the pieces: under yours, over the defaults
    std::vector<std::wstring> errors;
};

static std::shared_ptr<const Store> g_store;
static std::mutex g_storeLock;
static Settings g_defaults;
static SchemaInfo g_info;
static Settings g_live;                   // (the host's thread only)
static std::set<std::wstring> g_liveRemoved;
static bool g_usesRoles;                  // some value names a theme role

static std::shared_ptr<const Store> Current() {
    std::lock_guard<std::mutex> lock(g_storeLock);
    return g_store;
}

// The suite's part names, so a module reading another's setting by its full
// name ("systemIcons.doubleClickApps") isn't given its own prefix too.
static bool IsFullName(const std::wstring& key) {
    static const wchar_t* const kParts[] = {L"trayIcons", L"trayFolders", L"systemIcons", L"textColor", L"menuNoIcons",
                                            L"menuNoDuplicates", L"clock", L"iconRedirect", L"tweaks", L"taskbar",
                                            L"switcher", L"explorerStyle", L"explorerColors", L"themeGaps", L"darkMagic",
                                            L"classicMenus", L"desktop", L"shell", L"motion", L"notify"};
    size_t dot = key.find(L'.');
    if (dot == std::wstring::npos) return false;
    std::wstring first = key.substr(0, dot);
    for (auto p : kParts)
        if (first == p) return true;
    return false;
}

static std::wstring Lookup(const wchar_t* prefix, const std::wstring& key, bool* found) {
    std::wstring full = IsFullName(key) ? key : std::wstring(prefix) + L"." + key;
    auto store = Current();
    if (store) {
        auto it = store->values.find(full);
        if (it != store->values.end()) return *found = true, it->second;
        auto id = store->identity.find(full);
        if (id != store->identity.end()) return *found = true, id->second;
    }
    auto it = g_defaults.find(full);
    if (it != g_defaults.end()) return *found = true, it->second;
    *found = false;
    return L"";
}

std::vector<std::pair<std::wstring, std::wstring>> SettingsWithPrefix(const std::wstring& prefix) {
    Settings all;
    for (auto it = g_defaults.lower_bound(prefix); it != g_defaults.end() && it->first.rfind(prefix, 0) == 0; ++it) all[it->first] = it->second;
    if (auto store = Current()) {
        for (auto it = store->identity.lower_bound(prefix); it != store->identity.end() && it->first.rfind(prefix, 0) == 0; ++it)
            all[it->first] = it->second;
        for (auto it = store->values.lower_bound(prefix); it != store->values.end() && it->first.rfind(prefix, 0) == 0; ++it)
            all[it->first] = it->second;
    }
    return {all.begin(), all.end()};
}

std::wstring FormatKey(const wchar_t* format, va_list args) {
    if (!wcschr(format, L'%')) return format;
    wchar_t text[512];
    _vsnwprintf(text, ARRAYSIZE(text) - 1, format, args);
    text[ARRAYSIZE(text) - 1] = 0;
    return text;
}

int GetInt(const wchar_t* prefix, const std::wstring& key) {
    bool found;
    return ToInt(Lookup(prefix, key, &found));
}

const wchar_t* GetString(const wchar_t* prefix, const std::wstring& key) {
    bool found;
    std::wstring v = Lookup(prefix, key, &found);
    wchar_t* copy = new wchar_t[v.size() + 1];
    memcpy(copy, v.c_str(), (v.size() + 1) * sizeof(wchar_t));
    return copy;
}

void FreeString(const wchar_t* text) { delete[] text; }

bool IsSet(const wchar_t* prefix, const std::wstring& key) {
    std::wstring full = IsFullName(key) ? key : std::wstring(prefix) + L"." + key;
    auto store = Current();
    return store && store->values.count(full);
}

bool IsSetOrIdentity(const wchar_t* prefix, const std::wstring& key) {
    std::wstring full = IsFullName(key) ? key : std::wstring(prefix) + L"." + key;
    auto store = Current();
    return store && (store->values.count(full) || store->identity.count(full));
}

const ThemeRole kThemeRoles[] = {
    {L"@ground", L"Ground", L"Ground"},   {L"@raised", L"Raised", L"Raised"}, {L"@line", L"Line", L"Line"},
    {L"@edge", L"Edge", L"Edge"},         {L"@text", L"Text", L"Text"},       {L"@muted", L"Muted", L"Muted"},
    {L"@dim", L"Dim", L"Dim"},            {L"@accent", L"Accent", L"Accent"}, {L"@accent2", L"Accent2", L"Second accent"},
    {L"@good", L"Good", L"Good"},         {L"@warn", L"Warn", L"Warning"},    {L"@bad", L"Bad", L"Bad"},
};
const int kThemeRoleCount = ARRAYSIZE(kThemeRoles);

bool ThemeRoleColor(const ThemeRole& role, uint32_t* out) {
    DWORD v = 0, size = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Theme", role.value, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS)
        return false;
    *out = v & 0xFFFFFF;
    return true;
}

// "@accent" (and the others) in a value as the theme's colour: "#RRGGBB", or
// "RRGGBB" where the setting's default is written without the #. A value
// that is only a role, with no theme yet, is left to its default.
static bool PutInRoles(const std::wstring& key, std::wstring* value) {
    if (value->find(L'@') == std::wstring::npos) return true;
    value->erase(0, value->find_first_not_of(L" \t"));
    value->erase(value->find_last_not_of(L" \t") + 1);
    auto def = g_defaults.find(key);
    bool bare = def != g_defaults.end() && def->second.size() == 6 &&
                def->second.find_first_not_of(L"0123456789abcdefABCDEF") == std::wstring::npos;
    std::wstring out;
    bool onlyRole = value->front() == L'@', any = false;
    for (size_t i = 0; i < value->size();) {
        if ((*value)[i] != L'@') {
            out += (*value)[i++];
            continue;
        }
        size_t end = i + 1;
        while (end < value->size() && iswalnum((*value)[end])) end++;
        std::wstring token = value->substr(i, end - i);
        if (end < value->size()) onlyRole = false;
        uint32_t color;
        bool put = false;
        for (auto const& r : kThemeRoles)
            if (!_wcsicmp(r.token, token.c_str()) && ThemeRoleColor(r, &color)) {
                wchar_t hex[8];
                _snwprintf(hex, 8, bare ? L"%06X" : L"#%06X", color);
                out += hex;
                put = any = true;
            }
        if (!put) out += token;
        i = end;
    }
    if (onlyRole && !any) return false;
    *value = out;
    return true;
}

static void Publish(std::shared_ptr<Store> store) {
    store->values = store->user;
    for (auto const& [k, v] : store->gui)
        if (!g_liveRemoved.count(k)) store->values[k] = v;  // (taken out: yours again, if you set one)
    for (auto const& [k, v] : g_live) store->values[k] = v;
    for (auto const& [k, v] : store->suite) store->values[k] = v;
    g_usesRoles = false;
    for (auto it = store->values.begin(); it != store->values.end();) {
        if (it->second.find(L'@') != std::wstring::npos) g_usesRoles = true;
        if (PutInRoles(it->first, &it->second)) ++it;
        else it = store->values.erase(it);
    }
    store->identity = IdentityDerive(store->values, g_defaults);
    {
        std::lock_guard<std::mutex> lock(g_storeLock);
        g_store = store;
    }
    bool found;
    g_logToFile = ToInt(Lookup(L"desktop", L"log", &found)) != 0;
    for (auto const& [k, v] : store->identity)
        if (!store->values.count(k)) Log(L"identity: %s = %s", k.c_str(), v.c_str());
}

static Settings ReadSuiteLayer();

static bool LoadConfig() {
    auto store = std::make_shared<Store>();
    store->suite = ReadSuiteLayer();
    std::wstring text;
    std::vector<std::wstring> errors;
    if (ReadText(ConfigPath(), &text)) ParseConfig(text, store->user, &errors);
    for (auto const& e : errors) store->errors.push_back(L"tourne.conf: " + e);
    errors.clear();
    if (ReadText(GuiPath(), &text)) ParseConfig(text, store->gui, &errors);
    for (auto const& e : errors) store->errors.push_back(L"tourne-gui.conf: " + e);
    for (auto const& e : store->errors) Log(L"%s", e.c_str());
    Publish(store);
    return true;
}

// The same files over again (a layer or the theme changed).
static void Republish() {
    auto now = Current();
    auto store = std::make_shared<Store>();
    if (now) store->user = now->user, store->gui = now->gui, store->suite = now->suite, store->errors = now->errors;
    Publish(store);
}

const Settings& DefaultSettings() { return g_defaults; }
const SchemaInfo& SettingsSchema() { return g_info; }
void RefreshPluginSettings() {
    PluginSettings(g_defaults, g_info);
    PluginDynamicOptions(g_info);
}
void RefreshPluginChoices() { PluginDynamicOptions(g_info); }

static std::wstring ValuesKey(const wchar_t* prefix);

// Choices a part lists as it runs (Windhawk's dynamic selects: the switcher's
// styles), kept where it keeps its values as "::wh_select_option::<setting>::<value>" = label.
void RefreshDynamicOptions() {
    const std::wstring tag = L"::wh_select_option::";
    for (int m = 0; m < kModuleCount; m++) {
        HKEY k;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, ValuesKey(kModules[m].prefix).c_str(), 0, KEY_READ, &k) != ERROR_SUCCESS) continue;
        std::map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> found;
        wchar_t name[1024], label[1024];
        for (DWORD i = 0;; i++) {
            DWORD nameLen = ARRAYSIZE(name), type = 0, size = sizeof(label) - sizeof(wchar_t);
            LONG r = RegEnumValueW(k, i, name, &nameLen, nullptr, &type, (BYTE*)label, &size);
            if (r == ERROR_NO_MORE_ITEMS) break;
            if (r != ERROR_SUCCESS || type != REG_SZ) continue;
            label[size / sizeof(wchar_t)] = 0;
            std::wstring n = name;
            if (n.rfind(tag, 0) != 0) continue;
            size_t split = n.find(L"::", tag.size());
            if (split == std::wstring::npos) continue;
            std::wstring setting = n.substr(tag.size(), split - tag.size());
            if (!IsFullName(setting)) setting = std::wstring(kModules[m].prefix) + L"." + setting;
            found[setting].push_back({n.substr(split + 2), label});
        }
        RegCloseKey(k);
        for (auto& [setting, options] : found) {
            if (!g_defaults.count(setting)) continue;
            auto& info = g_info[setting];
            for (auto& o : options) {
                bool have = false;
                for (auto const& h : info.options) have = have || h.first == o.first;
                if (!have) info.options.push_back(std::move(o));
            }
        }
    }
}

void ConfigLayers(Settings* user, Settings* gui) {
    auto store = Current();
    if (user) *user = store ? store->user : Settings{};
    if (gui) *gui = store ? store->gui : Settings{};
}

////////////////////////////////////////////////////////////////////////////////
// Values a module keeps between runs: in the registry, per module.

static std::wstring ValuesKey(const wchar_t* prefix) { return std::wstring(L"Software\\Tourne\\Desktop\\Values\\") + prefix; }

size_t GetStringValue(const wchar_t* prefix, const wchar_t* name, wchar_t* buffer, size_t chars) {
    if (!chars) return 0;
    buffer[0] = 0;
    DWORD size = (DWORD)(chars * sizeof(wchar_t));
    if (RegGetValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, RRF_RT_REG_SZ, nullptr, buffer, &size) != ERROR_SUCCESS)
        return 0;
    return wcslen(buffer);
}

BOOL SetStringValue(const wchar_t* prefix, const wchar_t* name, const wchar_t* value) {
    return RegSetKeyValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, REG_SZ, value,
                           (DWORD)((wcslen(value) + 1) * sizeof(wchar_t))) == ERROR_SUCCESS;
}

int GetIntValue(const wchar_t* prefix, const wchar_t* name, int fallback) {
    DWORD v = 0, size = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS)
        return fallback;
    return (int)v;
}

BOOL SetIntValue(const wchar_t* prefix, const wchar_t* name, int value) {
    DWORD v = (DWORD)value;
    return RegSetKeyValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, REG_DWORD, &v, sizeof(v)) == ERROR_SUCCESS;
}

size_t GetBinaryValue(const wchar_t* prefix, const wchar_t* name, void* buffer, size_t size) {
    DWORD got = (DWORD)size;
    if (RegGetValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, RRF_RT_REG_BINARY, nullptr, buffer, &got) != ERROR_SUCCESS)
        return 0;
    return got;
}

BOOL SetBinaryValue(const wchar_t* prefix, const wchar_t* name, const void* buffer, size_t size) {
    return RegSetKeyValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, REG_BINARY, buffer, (DWORD)size) == ERROR_SUCCESS;
}

BOOL DeleteValue(const wchar_t* prefix, const wchar_t* name) {
    return RegDeleteKeyValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name) == ERROR_SUCCESS;
}

static std::wstring SuiteModKey();

// A part beside the suite reads what the suite's copy keeps (Icon Redirect: the themes Windhawk
// downloaded), from Windhawk's mod storage; its own folder here when there's none.
static std::wstring SuiteStorage(const wchar_t* prefix) {
    bool beside = false;
    for (int i = 0; i < kModuleCount; i++) beside = beside || (kModules[i].besideSuite && !wcscmp(kModules[i].prefix, prefix));
    std::wstring key = beside ? SuiteModKey() : L"";
    if (key.empty()) return L"";
    std::wstring id = key.substr(0, key.rfind(L'\\'));
    id = id.substr(id.rfind(L'\\') + 1);
    std::wstring dir = KnownFolder(FOLDERID_ProgramData) + L"\\Windhawk\\Engine\\ModsWritable\\mod-storage\\" + id;
    DWORD attributes = GetFileAttributesW(dir.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) ? dir : L"";
}

size_t GetStoragePath(const wchar_t* prefix, wchar_t* buffer, size_t chars) {
    std::wstring dir = SuiteStorage(prefix);
    if (dir.empty()) dir = DataDir() + L"\\" + prefix;
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
    if (!chars) return 0;
    wcsncpy(buffer, dir.c_str(), chars - 1);
    buffer[chars - 1] = 0;
    return wcslen(buffer);
}

////////////////////////////////////////////////////////////////////////////////
// The Windhawk suite, if it's installed: its settings to import, and whether
// it's already running a module (two taskbars would fight).

static std::wstring SuiteModKey() {
    HKEY mods;
    std::wstring best;
    int bestNumber = -1;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods", 0, KEY_READ, &mods) != ERROR_SUCCESS)
        return L"";
    wchar_t name[256];
    for (DWORD i = 0;; i++) {
        DWORD len = ARRAYSIZE(name);
        if (RegEnumKeyExW(mods, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        const wchar_t* at = wcsstr(name, L"tourne-suite-b");
        if (!at) continue;
        DWORD disabled = 0, size = sizeof(disabled);
        RegGetValueW(mods, name, L"Disabled", RRF_RT_REG_DWORD, nullptr, &disabled, &size);
        if (disabled) continue;
        // (a build Windhawk no longer has can leave its key behind with no Version and no Disabled: not running)
        size = 0;
        if (RegGetValueW(mods, name, L"Version", RRF_RT_REG_SZ, nullptr, nullptr, &size) != ERROR_SUCCESS || size <= sizeof(wchar_t))
            continue;
        int number = _wtoi(at + 14);
        if (number > bestNumber) bestNumber = number, best = name;
    }
    RegCloseKey(mods);
    return best.empty() ? L"" : L"SOFTWARE\\Windhawk\\Engine\\Mods\\" + best + L"\\Settings";
}

// Whether a Windhawk mod whose id holds idPart ("tourne-table") is on there now (a tool mod built
// in here waits while it is, so it never runs twice).
bool WindhawkModOn(const wchar_t* idPart) {
    HKEY mods;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods", 0, KEY_READ, &mods) != ERROR_SUCCESS)
        return false;
    bool on = false;
    wchar_t name[256];
    for (DWORD i = 0; !on; i++) {
        DWORD len = ARRAYSIZE(name);
        if (RegEnumKeyExW(mods, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        if (!wcsstr(name, idPart)) continue;
        DWORD disabled = 0, size = sizeof(disabled);
        RegGetValueW(mods, name, L"Disabled", RRF_RT_REG_DWORD, nullptr, &disabled, &size);
        size = 0;
        on = !disabled && RegGetValueW(mods, name, L"Version", RRF_RT_REG_SZ, nullptr, nullptr, &size) == ERROR_SUCCESS &&
             size > sizeof(wchar_t);
    }
    RegCloseKey(mods);
    return on;
}

// The parts that stay in Windhawk and that pieces here read too (System Text Color's colour for the
// menus, Menu No Icons and Duplicates, Icon Redirect's rows, Classic Menus): their settings as the
// running suite build has them in Windhawk, read only, over the config's copies (an import's), so what's
// set in Windhawk is what shows here. Read again whenever Windhawk's settings change (WatchThread).
static Settings ReadSuiteLayer() {
    Settings out;
    std::wstring key = SuiteModKey();
    if (key.empty()) return out;
    HKEY k;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &k) != ERROR_SUCCESS) return out;
    static const wchar_t* const kStays[] = {L"textColor.", L"menuNoIcons.", L"menuNoDuplicates.", L"iconRedirect.",
                                            L"classicMenus."};
    wchar_t name[512];
    std::vector<BYTE> data(64 * 1024);
    for (DWORD i = 0;; i++) {
        DWORD nameLen = ARRAYSIZE(name), type = 0, size = (DWORD)data.size();
        LONG r = RegEnumValueW(k, i, name, &nameLen, nullptr, &type, data.data(), &size);
        if (r == ERROR_NO_MORE_ITEMS) break;
        if (r != ERROR_SUCCESS) continue;
        bool stays = false;
        for (auto p : kStays) stays = stays || !wcsncmp(name, p, wcslen(p));
        if (!stays) continue;
        if (type == REG_DWORD) out[name] = std::to_wstring((int)*(DWORD*)data.data());
        else if (type == REG_SZ) out[name] = std::wstring((wchar_t*)data.data(), wcsnlen((wchar_t*)data.data(), size / sizeof(wchar_t)));
    }
    RegCloseKey(k);
    return out;
}

static bool SuiteRuns(const wchar_t* prefix) {
    std::wstring key = SuiteModKey();
    if (key.empty()) return false;
    DWORD on = 0, size = sizeof(on);
    std::wstring name = std::wstring(prefix) + L".enabled";
    return RegGetValueW(HKEY_LOCAL_MACHINE, key.c_str(), name.c_str(), RRF_RT_REG_DWORD, nullptr, &on, &size) == ERROR_SUCCESS && on;
}

// A suite build to import from, running or not: the one asked for ("b53"),
// else the newest running, else the newest that has settings at all.
static std::wstring SuiteImportKey(int wanted, int* number) {
    HKEY mods;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods", 0, KEY_READ, &mods) != ERROR_SUCCESS) return L"";
    std::wstring running, any, exact;
    int runningN = -1, anyN = -1;
    wchar_t name[256];
    for (DWORD i = 0;; i++) {
        DWORD len = ARRAYSIZE(name);
        if (RegEnumKeyExW(mods, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        const wchar_t* at = wcsstr(name, L"tourne-suite-b");
        if (!at) continue;
        int n = _wtoi(at + 14);
        HKEY settings;
        DWORD values = 0;
        if (RegOpenKeyExW(mods, (std::wstring(name) + L"\\Settings").c_str(), 0, KEY_READ, &settings) == ERROR_SUCCESS) {
            RegQueryInfoKeyW(settings, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &values, nullptr, nullptr, nullptr, nullptr);
            RegCloseKey(settings);
        }
        if (!values) continue;
        if (n == wanted) exact = name;
        if (n > anyN) anyN = n, any = name;
        DWORD disabled = 0, size = sizeof(disabled);
        RegGetValueW(mods, name, L"Disabled", RRF_RT_REG_DWORD, nullptr, &disabled, &size);
        size = 0;
        bool versioned = RegGetValueW(mods, name, L"Version", RRF_RT_REG_SZ, nullptr, nullptr, &size) == ERROR_SUCCESS && size > sizeof(wchar_t);
        if (!disabled && versioned && n > runningN) runningN = n, running = name;
    }
    RegCloseKey(mods);
    std::wstring pick = !exact.empty() ? exact : !running.empty() ? running : any;
    if (pick.empty()) return L"";
    *number = _wtoi(wcsstr(pick.c_str(), L"tourne-suite-b") + 14);
    return L"SOFTWARE\\Windhawk\\Engine\\Mods\\" + pick + L"\\Settings";
}

// Your suite settings for the parts Tourne Desktop has (the taskbar, the
// switcher and the system icons, and the parts they read: the clock, tray
// folders and icons, menus, text colour), only those that differ from the
// defaults, put in tourne-gui.conf so they win over tourne.conf and the Studio
// shows them (Reset there takes one back). Which parts run is left as it is.
// The file as it was is kept beside it.
static bool ImportSuite(int wanted, std::wstring* report) {
    int number = 0;
    std::wstring key = SuiteImportKey(wanted, &number);
    if (key.empty()) {
        *report = wanted ? L"Tourne'Suite b" + std::to_wstring(wanted) + L" has no settings in Windhawk."
                         : L"No Tourne'Suite build with settings was found in Windhawk.";
        return false;
    }
    if (wanted && number != wanted) {
        *report = L"Tourne'Suite b" + std::to_wstring(wanted) + L" has no settings in Windhawk.";
        return false;
    }
    HKEY k;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &k) != ERROR_SUCCESS) {
        *report = L"Couldn't read " + key + L".";
        return false;
    }
    Settings imported;
    int other = 0;
    wchar_t name[512];
    std::vector<BYTE> data(64 * 1024);
    for (DWORD i = 0;; i++) {
        DWORD nameLen = ARRAYSIZE(name), type = 0, size = (DWORD)data.size();
        LONG r = RegEnumValueW(k, i, name, &nameLen, nullptr, &type, data.data(), &size);
        if (r == ERROR_NO_MORE_ITEMS) break;
        if (r != ERROR_SUCCESS) continue;
        std::wstring value;
        if (type == REG_DWORD) value = std::to_wstring((int)*(DWORD*)data.data());
        else if (type == REG_SZ) value = std::wstring((wchar_t*)data.data(), wcsnlen((wchar_t*)data.data(), size / sizeof(wchar_t)));
        else continue;
        std::wstring n = name;
        bool ours = IsFullName(n);
        for (int m = 0; m < kModuleCount && !ours; m++) {
            if (kModules[m].besideSuite) continue;  // (read from Windhawk as it is, never copied)
            std::wstring prefix = std::wstring(kModules[m].prefix) + L".";
            ours = n.compare(0, prefix.size(), prefix) == 0;
        }
        if (!ours) {
            other++;
            continue;
        }
        if (n.size() > 8 && n.compare(n.size() - 8, 8, L".enabled") == 0) continue;  // (which parts run: as you have it)
        auto def = g_defaults.find(n);
        if (def != g_defaults.end() && def->second == value) continue;
        imported[n] = value;
    }
    RegCloseKey(k);
    Settings gui;
    std::wstring text;
    if (ReadText(GuiPath(), &text)) {
        ParseConfig(text, gui, nullptr);
        CopyFileW(GuiPath().c_str(), (GuiPath() + L".before-import").c_str(), FALSE);
    }
    for (auto const& [n, v] : imported) gui[n] = v;
    std::wstring header =
        L"# Tourne Desktop: what Tourne Studio sets. It's read after tourne.conf, so a setting\n"
        L"# here wins over the same one there. The Studio rewrites this file when you save.\n"
        L"# Imported from Tourne'Suite b" + std::to_wstring(number) + L" in Windhawk.\n\n";
    if (!WriteText(GuiPath(), WriteConfig(gui, header))) {
        *report = L"Couldn't write " + GuiPath() + L".";
        return false;
    }
    *report = L"Imported " + std::to_wstring(imported.size()) + L" settings from Tourne'Suite b" + std::to_wstring(number) +
              L" (the taskbar, switcher, system icons, clock and tray).";
    if (other) *report += L"\n" + std::to_wstring(other) + L" more belong to parts that stay in Windhawk (Start and the rest).";
    *report += L"\n\nThey're in tourne-gui.conf; the one before is kept as tourne-gui.conf.before-import.";
    Log(L"import: %s", report->c_str());
    return true;
}

static bool ImportTable(std::wstring* report);

// --import [bNN]
static int Import(const std::wstring& args) {
    int wanted = 0;
    size_t at = args.find(L"--import");
    std::wstring rest = args.substr(at + 8);
    size_t b = rest.find_first_not_of(L" \t\"");
    if (b != std::wstring::npos) {
        if (rest[b] == L'b' || rest[b] == L'B') b++;
        if (b < rest.size() && iswdigit(rest[b])) wanted = _wtoi(rest.c_str() + b);
    }
    std::wstring report;
    bool ok = false;
    if (args.find(L"--import-table") == std::wstring::npos) ok = ImportSuite(wanted, &report);
    std::wstring table;
    bool tableOk = ImportTable(&table);
    if (args.find(L"--import-table") != std::wstring::npos || tableOk) {
        report += (report.empty() ? L"" : L"\n\n") + table;
        ok = ok || tableOk;
    }
    if (ok && FindWindowW(L"TourneDesktop_Host", nullptr)) report += L"\nTourne Desktop is taking them in now.";
    MessageBoxW(nullptr, report.c_str(), L"Tourne Desktop", ok ? MB_ICONINFORMATION : MB_ICONWARNING);
    return ok ? 0 : 1;
}

////////////////////////////////////////////////////////////////////////////////
// Tourne'Table from Windhawk: every copy there (a mod with "tourne-table" in its id) brought into the
// Table built in here. The main copy (the one that's on; else the one changed last) gives its settings
// (into tourne-gui.conf as table.*), its quick settings, the setup it shares, its place and its panel;
// every copy gives its saved styles (My Styles; a name the Table here already has keeps its own) and
// becomes a setup file in Documents\Tourne'Table setups, which the Style Editor's Share row opens.
// Windhawk keeps settings in HKLM\SOFTWARE\Windhawk\Engine\Mods\<id>\Settings (or under Users\<SID>
// when they're per user) and a mod's values in ...\ModsWritable\<id>\LocalStorage; both are only read.

struct WindhawkTable {
    std::wstring id;
    bool on = false;
    FILETIME changed{};
    std::vector<std::pair<std::wstring, std::wstring>> settings;  // as Windhawk has them
    std::map<std::wstring, std::wstring> values;                  // its LocalStorage strings
    std::map<std::wstring, DWORD> numbers;                        // and its whole numbers
};

static std::wstring MySid() {
    HANDLE token = nullptr;
    std::wstring out;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return out;
    BYTE buffer[256];
    DWORD size = 0;
    LPWSTR text = nullptr;
    if (GetTokenInformation(token, TokenUser, buffer, sizeof(buffer), &size) &&
        ConvertSidToStringSidW(((TOKEN_USER*)buffer)->User.Sid, &text)) {
        out = text;
        LocalFree(text);
    }
    CloseHandle(token);
    return out;
}

// Every value of a key: strings and whole numbers (as text in all), false if there's no such key.
static bool ReadKeyValues(const std::wstring& path, std::vector<std::pair<std::wstring, std::wstring>>* all,
                          std::map<std::wstring, DWORD>* numbers, FILETIME* changed) {
    HKEY k;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &k) != ERROR_SUCCESS) return false;
    if (changed) RegQueryInfoKeyW(k, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, changed);
    wchar_t name[512];
    std::vector<BYTE> data(256 * 1024);
    for (DWORD i = 0;; i++) {
        DWORD nameLen = ARRAYSIZE(name), type = 0, size = (DWORD)data.size();
        LONG r = RegEnumValueW(k, i, name, &nameLen, nullptr, &type, data.data(), &size);
        if (r == ERROR_NO_MORE_ITEMS) break;
        if (r != ERROR_SUCCESS) continue;
        if (type == REG_DWORD) {
            DWORD v = *(DWORD*)data.data();
            all->emplace_back(name, std::to_wstring((int)v));
            if (numbers) (*numbers)[name] = v;
        } else if (type == REG_SZ || type == REG_EXPAND_SZ) {
            all->emplace_back(name, std::wstring((wchar_t*)data.data(), wcsnlen((wchar_t*)data.data(), size / sizeof(wchar_t))));
        }
    }
    RegCloseKey(k);
    return true;
}

static std::vector<WindhawkTable> WindhawkTables() {
    std::vector<WindhawkTable> out;
    HKEY mods;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods", 0, KEY_READ, &mods) != ERROR_SUCCESS) return out;
    std::wstring sid = MySid();
    wchar_t name[256];
    for (DWORD i = 0;; i++) {
        DWORD len = ARRAYSIZE(name);
        if (RegEnumKeyExW(mods, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        if (!wcsstr(name, L"tourne-table")) continue;
        WindhawkTable t;
        t.id = name;
        DWORD disabled = 0, size = sizeof(disabled);
        RegGetValueW(mods, name, L"Disabled", RRF_RT_REG_DWORD, nullptr, &disabled, &size);
        size = 0;
        t.on = !disabled && RegGetValueW(mods, name, L"Version", RRF_RT_REG_SZ, nullptr, nullptr, &size) == ERROR_SUCCESS &&
               size > sizeof(wchar_t);
        std::wstring base = L"SOFTWARE\\Windhawk\\Engine\\Mods\\" + t.id;
        if (sid.empty() || !ReadKeyValues(base + L"\\Users\\" + sid + L"\\Settings", &t.settings, nullptr, &t.changed) ||
            t.settings.empty()) {
            t.settings.clear();
            ReadKeyValues(base + L"\\Settings", &t.settings, nullptr, &t.changed);
        }
        std::vector<std::pair<std::wstring, std::wstring>> stored;
        FILETIME storedChanged{};
        ReadKeyValues(L"SOFTWARE\\Windhawk\\Engine\\ModsWritable\\" + t.id + L"\\LocalStorage", &stored, &t.numbers, &storedChanged);
        for (auto& [n, v] : stored)
            if (!t.numbers.count(n)) t.values[n] = v;
        if (CompareFileTime(&storedChanged, &t.changed) > 0) t.changed = storedChanged;
        if (!t.settings.empty() || !stored.empty()) out.push_back(std::move(t));
    }
    RegCloseKey(mods);
    // The main copy first: the one that's on, else the one changed last.
    std::stable_sort(out.begin(), out.end(), [](const WindhawkTable& a, const WindhawkTable& b) {
        if (a.on != b.on) return a.on;
        return CompareFileTime(&a.changed, &b.changed) > 0;
    });
    return out;
}

static std::vector<std::wstring> TableLines(const std::wstring& text) {
    std::vector<std::wstring> out;
    for (size_t pos = 0; pos < text.size();) {
        size_t nl = text.find(L'\n', pos);
        if (nl == std::wstring::npos) nl = text.size();
        if (nl > pos) out.push_back(text.substr(pos, nl - pos));
        pos = nl + 1;
    }
    return out;
}

// One copy as a Tourne'Table setup file (the Style Editor's Share row reads it): its settings with the
// setup it took in over them, and its quick settings. Settings a setup never carries are ignored on
// opening, so all of them go in.
static std::wstring TableSetupText(const WindhawkTable& t) {
    std::map<std::wstring, std::wstring> settings(t.settings.begin(), t.settings.end());
    if (auto it = t.values.find(L"sharedSetup"); it != t.values.end())
        for (auto const& line : TableLines(it->second))
            if (size_t eq = line.find(L'='); eq != std::wstring::npos && eq) settings[line.substr(0, eq)] = line.substr(eq + 1);
    std::wstring text = L"Tourne'Table setup 1\nname=" + t.id + L" (Windhawk)\nparts=look,motion,extras,place\n";
    for (auto const& [n, v] : settings) text += n + L"=" + v + L"\n";
    if (auto it = t.values.find(L"menuOverrides"); it != t.values.end())
        for (auto const& line : TableLines(it->second)) text += L"quick." + line + L"\n";
    return text;
}

// Every Windhawk copy's saved styles into the Table here's My Styles; a name it already has keeps its
// own. Returns how many were added.
static int MergeTableStyles(const std::vector<WindhawkTable>& tables) {
    wchar_t buffer[8192] = L"";
    GetStringValue(L"table", L"stylePresets", buffer, ARRAYSIZE(buffer));
    std::vector<std::wstring> names = TableLines(buffer);
    int styles = 0;
    for (auto const& t : tables) {
        auto list = t.values.find(L"stylePresets");
        if (list == t.values.end()) continue;
        for (auto const& name : TableLines(list->second)) {
            auto body = t.values.find(L"stylePreset:" + name);
            if (body == t.values.end() || body->second.empty() || std::find(names.begin(), names.end(), name) != names.end()) continue;
            SetStringValue(L"table", (L"stylePreset:" + name).c_str(), body->second.c_str());
            names.push_back(name);
            styles++;
        }
    }
    if (!styles) return 0;
    std::wstring list;
    for (auto const& n : names) list += n + L"\n";
    SetStringValue(L"table", L"stylePresets", list.c_str());
    return styles;
}

static bool ImportTable(std::wstring* report) {
    std::vector<WindhawkTable> tables = WindhawkTables();
    if (tables.empty()) {
        *report = L"No Tourne'Table was found in Windhawk.";
        return false;
    }
    const WindhawkTable& main = tables.front();
    std::wstring values = L"Software\\Tourne\\Desktop\\Values\\table";

    // Its settings, those that differ from the defaults, in tourne-gui.conf.
    Settings gui;
    std::wstring text;
    if (ReadText(GuiPath(), &text)) {
        ParseConfig(text, gui, nullptr);
        CopyFileW(GuiPath().c_str(), (GuiPath() + L".before-table-import").c_str(), FALSE);
    }
    int settings = 0;
    for (auto const& [n, v] : main.settings) {
        std::wstring key = L"table." + n;
        auto def = g_defaults.find(key);
        if (def != g_defaults.end() && def->second == v) {
            gui.erase(key);
            continue;
        }
        gui[key] = v;
        settings++;
    }
    std::wstring header =
        L"# Tourne Desktop: what Tourne Studio sets. It's read after tourne.conf, so a setting\n"
        L"# here wins over the same one there. The Studio rewrites this file when you save.\n";
    if (!WriteText(GuiPath(), WriteConfig(gui, header))) {
        *report = L"Couldn't write " + GuiPath() + L".";
        return false;
    }

    // Its quick settings, shared setup, place, panel and the rest, as the Table here keeps them.
    for (auto const& [n, v] : main.values) {
        if (n == L"stylePresets" || n.rfind(L"stylePreset:", 0) == 0) continue;  // (below, with every copy's)
        RegSetKeyValueW(HKEY_CURRENT_USER, values.c_str(), n.c_str(), REG_SZ, v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
    }
    for (auto const& [n, v] : main.numbers) RegSetKeyValueW(HKEY_CURRENT_USER, values.c_str(), n.c_str(), REG_DWORD, &v, sizeof(v));

    // Every copy's styles; a name the Table here already has keeps its own.
    int styles = MergeTableStyles(tables);

    // Each copy as a setup file.
    std::wstring folder = KnownFolder(FOLDERID_Documents) + L"\\Tourne'Table setups";
    SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr);
    int files = 0;
    for (auto const& t : tables) {
        std::wstring file = t.id;
        for (auto& c : file)
            if (wcschr(L"<>:\"/\\|?*", c)) c = L'-';
        if (WriteText(folder + L"\\" + file + L" (Windhawk).ttstyle", TableSetupText(t))) files++;
    }

    *report = L"From " + main.id + L" in Windhawk: " + std::to_wstring(settings) + L" settings, its quick settings and place.\n" +
              std::to_wstring(styles) + L" saved styles (from " + std::to_wstring(tables.size()) +
              L" Tourne'Table copies) are in My Styles.\n" + std::to_wstring(files) + L" setups are files in " + folder +
              L": the Style Editor's Share row opens them (Open file...) and saves new ones (Save file...).\n\n"
              L"tourne-gui.conf as it was is kept as tourne-gui.conf.before-table-import. Tourne'Table here starts once "
              L"it's off in Windhawk.";
    Log(L"table import: %s", report->c_str());
    return true;
}

// At start, when the Table here has nothing of its own yet (no values, no settings in tourne-gui.conf):
// whatever Windhawk's Tourne'Table has, brought in once.
static void AutoImportTable() {
    bool built = false;
    for (int i = 0; i < kModuleCount; i++) built = built || !wcscmp(kModules[i].prefix, L"table");
    HKEY k;
    if (!built || RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Desktop\\Values\\table", 0, KEY_READ, &k) == ERROR_SUCCESS) {
        if (built) RegCloseKey(k);
        return;
    }
    Settings gui;
    std::wstring text;
    if (ReadText(GuiPath(), &text)) ParseConfig(text, gui, nullptr);
    for (auto const& [n, v] : gui)
        if (n.rfind(L"table.", 0) == 0) return;
    std::wstring report;
    if (!ImportTable(&report)) SetIntValue(L"table", L"importedFromWindhawk", 0);  // (looked once; the key now exists)
    else SetIntValue(L"table", L"importedFromWindhawk", 1);
}

// At every start: styles saved in Windhawk's Tourne'Table since last time join My Styles here too, so
// the two lists stay in step (nothing else is copied; a name already here keeps its own).
static void SyncTableStyles() {
    bool built = false;
    for (int i = 0; i < kModuleCount; i++) built = built || !wcscmp(kModules[i].prefix, L"table");
    if (!built) return;
    if (int added = MergeTableStyles(WindhawkTables())) Log(L"table: %d new styles from Windhawk", added);
}

////////////////////////////////////////////////////////////////////////////////
// Modules

struct Running {
    bool on = false;
    Settings seen;  // its settings when it last started or was told of a change
};
static std::vector<Running> g_running;
static std::atomic<bool> g_moduleOn[16];  // (the same "on", for other threads: ModuleRunning)

// What a part reads: its own settings, as set or as the identity asks (identity.cpp), and the motion lines
// every animating part reads (a Motion preset or an animations block changes only those).
static Settings SettingsOf(const wchar_t* prefix) {
    Settings out;
    std::wstring p = std::wstring(prefix) + L".";
    auto store = Current();
    if (store) {
        for (auto const* layer : {&store->identity, &store->values})
            for (auto const& [k, v] : *layer)
                if (k.compare(0, p.size(), p) == 0 || k.rfind(L"motion.", 0) == 0) out[k] = v;
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Starting up, one part at a time: each gets a moment on its own, so a crash
// is pinned on the part that was starting (crash.h), and the next start leaves
// that part off until a setting is saved.

static HWND g_host;
static std::set<std::wstring> g_started;    // the services started: "theme", "inject", "glass"
static std::vector<std::wstring> g_startup;  // parts still to start
static bool g_startingUp = false;
constexpr UINT_PTR kTimerStartup = 2;

static void StartServices();

static void ApplyServices() {
    StartServices();  // (one a crash left off, once a setting is saved)
    if (g_started.count(L"theme")) ThemeEngineApply();
    if (g_started.count(L"inject")) InjectorApply();
    if (g_started.count(L"glass")) GlassApply();
    if (g_started.count(L"killswitch")) KillswitchApply();
    if (g_started.count(L"keystrokes")) KeystrokesApply();
    if (g_started.count(L"shader")) ScreenShaderApply();
    if (g_started.count(L"capture")) CaptureApply();
    if (g_started.count(L"launcher")) LauncherApply();
    if (g_started.count(L"flyouts")) FlyoutsApply();
    if (g_started.count(L"appStyler")) AppStylerApply();
    if (g_started.count(L"sounds")) UiSoundsApply();
    ShellApply();
    NotifyApply();
    ApplyWindowsMotion();
}

// Whether a Windhawk suite build (or its Icon Redirect on its own) is loaded in this process, as it is
// when Tourne Desktop isn't on the suite's exclusion list: then it does the besideSuite parts here.
static bool SuiteInThisProcess() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W entry{sizeof(entry)};
    bool found = false;
    for (BOOL more = Module32FirstW(snap, &entry); more && !found; more = Module32NextW(snap, &entry)) {
        std::wstring name = entry.szModule;
        for (auto& c : name) c = towlower(c);
        found = name.find(L"tourne-suite") != std::wstring::npos || name.find(L"tourne-icon-redirect") != std::wstring::npos;
    }
    CloseHandle(snap);
    return found;
}

static bool g_explorerBar;  // Explorer's taskbar in place of Tourne's bar (the explorerbar command)

static bool Wanted(int i) {
    bool found;
    if (g_explorerBar && !wcscmp(kModules[i].prefix, L"taskbar")) return false;  // (until it's swapped back)
    return ToInt(Lookup(kModules[i].prefix, L"enabled", &found)) != 0;
}

static void StartModule(int i) {
    Module const& m = kModules[i];
    if (CrashLeavesOff(std::wstring(L"module:") + m.prefix)) {
        Log(L"%s is left off: Tourne Desktop crashed while starting it last time (save a setting to try again)", m.title);
        return;
    }
    if (m.besideSuite ? SuiteInThisProcess() : SuiteRuns(m.prefix)) {
        if (m.besideSuite) Log(L"%s: the Windhawk suite is loaded in Tourne Desktop and does it here already", m.title);
        else Log(L"%s is on in the Windhawk suite: turn it off there first, so there's only one", m.title);
        return;
    }
    Log(L"starting %s", m.title);
    if (!m.init()) {
        if (m.besideSuite) DisableHooks(m.prefix);
        Log(L"%s didn't start", m.title);
        return;
    }
    if (m.besideSuite) ApplyHooks();
    if (m.afterInit) m.afterInit();
    g_running[i].on = true;
    g_running[i].seen = SettingsOf(m.prefix);
    if (i < (int)std::size(g_moduleOn)) g_moduleOn[i] = true;
}

static void StopModule(int i) {
    if (!g_running[i].on) return;
    Log(L"stopping %s", kModules[i].title);
    if (i < (int)std::size(g_moduleOn)) g_moduleOn[i] = false;  // (others stop calling into it first)
    kModules[i].uninit();
    if (kModules[i].besideSuite) DisableHooks(kModules[i].prefix);
    g_running[i].on = false;
}

bool ModuleRunning(const wchar_t* prefix) {
    for (int i = 0; i < kModuleCount && i < (int)std::size(g_moduleOn); i++)
        if (!wcscmp(kModules[i].prefix, prefix)) return g_moduleOn[i];
    return false;
}

// After the config changed: modules turned on or off start or stop; those
// whose settings changed are told, as Windhawk tells them.
static void ApplyConfig() {
    for (int i = 0; i < kModuleCount; i++) {
        bool want = Wanted(i);
        if (want && !g_running[i].on) {
            if (!g_startingUp) StartModule(i);  // (while starting up, each in its turn)
        }
        else if (!want && g_running[i].on) StopModule(i);
        else if (g_running[i].on) {
            Settings now = SettingsOf(kModules[i].prefix);
            if (now != g_running[i].seen) {
                g_running[i].seen = now;
                if (kModules[i].settingsChanged) kModules[i].settingsChanged();
            }
        }
    }
}

// Plugins' settings as the runtime was last told them. One place compares, whatever changed them (the Studio's
// live layer, its Save, an editor): Save reloads before the watcher does, so a before/after there saw nothing.
static std::vector<std::pair<std::wstring, std::wstring>> g_pluginsSeen;  // (the host's thread only)

static void TellPlugins() {
    auto now = SettingsWithPrefix(L"plugins.");
    if (now == g_pluginsSeen) return;
    g_pluginsSeen = std::move(now);
    PluginRuntimeSettingsChanged();
}

void SetLiveLayer(const Settings& set, const std::set<std::wstring>& removed) {
    if (set == g_live && removed == g_liveRemoved) return;
    g_live = set;
    g_liveRemoved = removed;
    Republish();
    ApplyConfig();
    ApplyServices();
    TellPlugins();
}

static void StartServices() {
    if (g_startingUp) return;
    for (const wchar_t* part : {L"theme", L"inject", L"glass", L"killswitch", L"keystrokes", L"shader", L"capture", L"launcher", L"flyouts", L"appStyler", L"sounds"}) {
        if (g_started.count(part) || CrashLeavesOff(part)) continue;
        if (part == std::wstring(L"theme")) ThemeEngineStart(g_host);
        else if (part == std::wstring(L"inject")) InjectorStart(g_host);
        else if (part == std::wstring(L"glass")) GlassStart(g_host);
        else if (part == std::wstring(L"keystrokes")) KeystrokesStart(g_host);
        else if (part == std::wstring(L"shader")) ScreenShaderStart(g_host);
        else if (part == std::wstring(L"capture")) CaptureStart(g_host);
        else if (part == std::wstring(L"launcher")) LauncherStart(g_host);
        else if (part == std::wstring(L"flyouts")) FlyoutsStart(g_host);
        else if (part == std::wstring(L"appStyler")) AppStylerStart(g_host);
        else if (part == std::wstring(L"sounds")) UiSoundsStart(g_host);
        else KillswitchStart(g_host);
        g_started.insert(part);
    }
}

static std::wstring PartTitle(const std::wstring& part) {
    if (part == L"theme") return L"the Tourne theme";
    if (part == L"inject") return L"app styles";
    if (part == L"glass") return L"glass";
    if (part == L"killswitch") return L"the taskbar killswitch";
    if (part == L"keystrokes") return L"keystrokes";
    if (part == L"shader") return L"the screen shader";
    if (part == L"capture") return L"Print Screen capture";
    if (part == L"launcher") return L"the launcher";
    if (part == L"flyouts") return L"the flyouts";
    if (part == L"appStyler") return L"the app styler";
    if (part == L"sounds") return L"UI sounds";
    for (int i = 0; i < kModuleCount; i++)
        if (part == std::wstring(L"module:") + kModules[i].prefix) return kModules[i].title;
    return part;
}

static DWORD WINAPI TellLeftOff(LPVOID param) {
    std::unique_ptr<std::wstring> title((std::wstring*)param);
    std::wstring text = L"Last time Tourne Desktop crashed while starting " + *title +
                        L", so it's left off this time and the rest is running.\n\n"
                        L"Save any setting (in the Studio or tourne.conf) to try it again. What happened is in crash.txt, in "
                        L"%LOCALAPPDATA%\\Tourne\\Desktop.";
    MessageBoxW(nullptr, text.c_str(), L"Tourne Desktop", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND);
    return 0;
}

// The next part to start, a moment after the last (crash.h). Theme, app styles and glass first, then
// the modules turned on.
static void StartupStep() {
    CrashStarted();
    while (!g_startup.empty()) {
        std::wstring part = g_startup.front();
        g_startup.erase(g_startup.begin());
        if (CrashLeavesOff(part)) {  // (a module says so itself, in StartModule)
            if (part.rfind(L"module:", 0) != 0)
                Log(L"%s is left off: Tourne Desktop crashed while starting it last time (save a setting to try again)",
                    PartTitle(part).c_str());
            continue;
        }
        int module = -1;
        for (int i = 0; i < kModuleCount; i++)
            if (part == std::wstring(L"module:") + kModules[i].prefix) module = i;
        if (module >= 0 && (!Wanted(module) || g_running[module].on)) continue;
        CrashStarting(part, PartTitle(part));
        if (module >= 0) StartModule(module);
        else {
            if (part == L"theme") ThemeEngineStart(g_host);
            else if (part == L"inject") InjectorStart(g_host);
            else if (part == L"glass") GlassStart(g_host);
            else if (part == L"killswitch") KillswitchStart(g_host);
            else if (part == L"keystrokes") KeystrokesStart(g_host);
            else if (part == L"shader") ScreenShaderStart(g_host);
            else if (part == L"capture") CaptureStart(g_host);
            else if (part == L"launcher") LauncherStart(g_host);
            else if (part == L"flyouts") FlyoutsStart(g_host);
            else if (part == L"appStyler") AppStylerStart(g_host);
            else if (part == L"sounds") UiSoundsStart(g_host);
            g_started.insert(part);
        }
        // A moment between parts, so a crash is put down to the right one: longer after a start that crashed.
        SetTimer(g_host, kTimerStartup, CrashLeftOff().empty() ? 100 : 800, nullptr);
        return;
    }
    g_startingUp = false;
    ApplyConfig();  // (anything turned on meanwhile)
    if (!CrashLeftOff().empty()) {
        if (HANDLE t = CreateThread(nullptr, 0, TellLeftOff, new std::wstring(PartTitle(CrashLeftOff())), 0, nullptr))
            CloseHandle(t);
    }
}

bool SaveGuiLayer(const Settings& gui) {
    std::wstring header =
        L"# Tourne Desktop: what Tourne Studio sets. It's read after tourne.conf, so a setting\n"
        L"# here wins over the same one there. The Studio rewrites this file when you save.\n\n";
    bool ok = WriteText(GuiPath(), WriteConfig(gui, header));
    if (!ok) return false;
    g_live.clear();
    g_liveRemoved.clear();
    LoadConfig();  // (the watcher reads it again too; nothing changes then)
    CrashTryAgain();  // (a setting saved: what a crash left off is tried again)
    ShellTryAgain();
    ApplyConfig();
    ApplyServices();
    TellPlugins();
    return true;
}

// Every setting each module has, with its default: a reference to copy from
// (rewritten only when a module's settings changed).
static void WriteDefaults() {
    std::wstring header =
        L"# Tourne Desktop: every setting and its default, for reference. Don't edit this file;\n"
        L"# copy a line into tourne.conf and change it there. It's rewritten when the modules change.\n\n";
    Settings all = g_defaults;
    for (int i = 0; i < kModuleCount; i++) all[std::wstring(kModules[i].prefix) + L".enabled"] = L"false";
    all[L"desktop.log"] = L"false";
    all[L"desktop.trayIcon"] = L"true";
    ThemeDefaults(all);
    InjectorDefaults(all);
    WriteText(DefaultsPath(), WriteConfig(all, header));
}

static void WriteStarterConfig() {
    if (GetFileAttributesW(ConfigPath().c_str()) != INVALID_FILE_ATTRIBUTES) return;
    std::wstring text =
        L"# Tourne Desktop: your settings. Save and they apply at once.\n"
        L"#\n"
        L"#   key = value          switcher.borders.width = 2\n"
        L"#   group { ... }        groups, as Hyprland writes them\n"
        L"#   # comment\n"
        L"#\n"
        L"# Every setting and its default is in defaults.conf next to this file.\n"
        L"# Your Windhawk suite settings: run TourneDesktop.exe --import.\n"
        L"# Turn a module off in the Windhawk suite before you turn it on here.\n\n";
    for (int i = 0; i < kModuleCount; i++)
        text += std::wstring(L"# ") + kModules[i].title + L"\n" + kModules[i].prefix + L".enabled = false\n\n";
    text += L"desktop {\n    trayIcon = true\n    log = false\n}\n\n";
    text += L"# Tourne theme: colours made from your wallpaper (or one seed colour) for every\n"
            L"# Tourne piece set to \"Tourne theme\". source: off, wallpaper, seed or preset.\n"
            L"theme {\n    source = off\n    profile = adaptive   # monochrome, muted, vivid, pastel\n"
            L"    mode = auto          # dark, light\n    seed = #64AA89\n}\n";
    WriteText(ConfigPath(), text);
}

////////////////////////////////////////////////////////////////////////////////
// The config watched (a thread that sleeps until the folder changes)

constexpr UINT WM_TD_RELOAD = WM_APP + 1;
constexpr UINT WM_TD_TRAY = WM_APP + 2;
constexpr UINT WM_TD_STUDIO = WM_APP + 3;  // another copy was started with --studio
constexpr UINT WM_TD_MENU_SETTINGS = WM_APP + 9;  // ... with --menu-settings
constexpr UINT WM_TD_BINDS_SETTINGS = WM_APP + 10;  // ... with --binds-settings
constexpr UINT_PTR kTimerReload = 1;
static HANDLE g_watchStop;

static DWORD WINAPI WatchThread(LPVOID) {
    HANDLE dir = CreateFileW(ConfigDir().c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (dir == INVALID_HANDLE_VALUE) return 0;
    OVERLAPPED ov{};
    ov.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    alignas(DWORD) BYTE buffer[4096];
    // Windhawk's mod settings too (read only), for the suite layer: a reload only when what it reads changed.
    HKEY mods = nullptr;
    HANDLE modsChanged = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Windhawk\\Engine\\Mods", 0, KEY_NOTIFY, &mods) != ERROR_SUCCESS) mods = nullptr;
    auto watchMods = [&] {
        if (mods && RegNotifyChangeKeyValue(mods, TRUE, REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET, modsChanged, TRUE) !=
                        ERROR_SUCCESS) {
            RegCloseKey(mods);
            mods = nullptr;
        }
    };
    watchMods();
    bool reading = false;
    for (;;) {
        if (!reading) {
            ResetEvent(ov.hEvent);
            if (!ReadDirectoryChangesW(dir, buffer, sizeof(buffer), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
                                       nullptr, &ov, nullptr))
                break;
            reading = true;
        }
        HANDLE waits[3] = {g_watchStop, ov.hEvent, modsChanged};
        DWORD woke = WaitForMultipleObjects(mods ? 3 : 2, waits, FALSE, INFINITE);
        if (woke == WAIT_OBJECT_0 + 2) {
            watchMods();
            auto now = Current();
            if (g_host && (!now || ReadSuiteLayer() != now->suite)) PostMessageW(g_host, WM_TD_RELOAD, 0, 0);
            continue;
        }
        if (woke != WAIT_OBJECT_0 + 1) {
            CancelIo(dir);
            break;
        }
        reading = false;
        DWORD got = 0;
        if (!GetOverlappedResult(dir, &ov, &got, FALSE)) continue;
        bool ours = got == 0;  // (overflowed: look anyway)
        for (auto* info = (FILE_NOTIFY_INFORMATION*)buffer; got;) {
            std::wstring name(info->FileName, info->FileNameLength / sizeof(wchar_t));
            if (!_wcsicmp(name.c_str(), L"tourne.conf") || !_wcsicmp(name.c_str(), L"tourne-gui.conf")) ours = true;
            if (!info->NextEntryOffset) break;
            info = (FILE_NOTIFY_INFORMATION*)((BYTE*)info + info->NextEntryOffset);
        }
        if (ours && g_host) PostMessageW(g_host, WM_TD_RELOAD, 0, 0);
    }
    if (mods) RegCloseKey(mods);
    CloseHandle(modsChanged);
    CloseHandle(ov.hEvent);
    CloseHandle(dir);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// The host window: the tray icon, reloads and stopping

constexpr wchar_t kHostClass[] = L"TourneDesktop_Host";

static void ShowTray(HWND wnd, bool show) {
    NOTIFYICONDATAW nid{sizeof(nid)};
    nid.hWnd = wnd;
    nid.uID = 1;
    if (!show) {
        Shell_NotifyIconW(NIM_DELETE, &nid);
        return;
    }
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TD_TRAY;
    nid.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
    if (!nid.hIcon) nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcscpy(nid.szTip, L"Tourne Desktop");
    if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) Shell_NotifyIconW(NIM_ADD, &nid);
}

static void UpdateTray(HWND wnd) {
    bool found;
    ShowTray(wnd, ToInt(Lookup(L"desktop", L"trayIcon", &found)) != 0 || !found);
}

// "Copy diagnostics": what someone helping needs to know, as text to paste. The build, Windows, the screens,
// the graphics card and sound device, the parts running, and every setting that isn't its default. Nothing
// that points at the person: values that are paths, or that hold the account's name, are left out.
static std::wstring Diagnostics() {
    std::wstring out = L"Tourne Desktop diagnostics\r\nBuild: " TD_BUILD L"\r\n";
    wchar_t line[512];
    // Windows' own version (RtlGetVersion, as GetVersionEx is told a lie).
    using RtlGetVersion_t = LONG(WINAPI*)(OSVERSIONINFOW*);
    OSVERSIONINFOW v{sizeof(v)};
    if (auto get = (RtlGetVersion_t)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion")) get(&v);
    swprintf(line, 512, L"Windows: %lu.%lu build %lu\r\n", v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber);
    out += line;
    out += std::wstring(L"Shell mode: ") + (ShellUnderneath() ? L"on, Explorer underneath" : L"off") + L"\r\n";
    // The graphics cards, and each screen: size, scale, refresh.
    DISPLAY_DEVICEW dd{sizeof(dd)};
    std::set<std::wstring> cards;
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); i++, dd.cb = sizeof(dd))
        if (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) cards.insert(dd.DeviceString);
    for (auto const& c : cards) out += L"Graphics: " + c + L"\r\n";
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM l) -> BOOL {
            MONITORINFOEXW mi{};
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(m, &mi);
            DEVMODEW dm{};
            dm.dmSize = sizeof(dm);
            EnumDisplaySettingsW(mi.szDevice, ENUM_CURRENT_SETTINGS, &dm);
            UINT dpiX = 96, dpiY = 96;
            using GetDpi_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
            static auto getDpi = (GetDpi_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
            if (getDpi) getDpi(m, 0, &dpiX, &dpiY);
            wchar_t s[160];
            swprintf(s, 160, L"Screen: %ldx%ld at %ld,%ld, %u%%, %lu Hz%ls\r\n", mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top, mi.rcMonitor.left, mi.rcMonitor.top, dpiX * 100 / 96,
                     dm.dmDisplayFrequency, (mi.dwFlags & MONITORINFOF_PRIMARY) ? L" (main)" : L"");
            *(std::wstring*)l += s;
            return TRUE;
        },
        (LPARAM)&out);
    // The sound device apps play to.
    {
        static const PROPERTYKEY kFriendlyName = {{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};
        HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        IMMDeviceEnumerator* e = nullptr;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&e))) {
            IMMDevice* d = nullptr;
            if (SUCCEEDED(e->GetDefaultAudioEndpoint(eRender, eConsole, &d))) {
                IPropertyStore* props = nullptr;
                PROPVARIANT name;
                PropVariantInit(&name);
                if (SUCCEEDED(d->OpenPropertyStore(STGM_READ, &props)) && SUCCEEDED(props->GetValue(kFriendlyName, &name)) &&
                    name.vt == VT_LPWSTR)
                    out += std::wstring(L"Sound: ") + name.pwszVal + L"\r\n";
                PropVariantClear(&name);
                if (props) props->Release();
                d->Release();
            }
            e->Release();
        }
        if (SUCCEEDED(init)) CoUninitialize();
    }
    out += L"Running:";
    for (int i = 0; i < kModuleCount; i++)
        if (g_running[i].on) out += std::wstring(L" ") + kModules[i].prefix;
    for (auto const& part : g_started) out += L" " + part;
    out += L"\r\n\r\nSettings not at their default:\r\n";
    // Left out: a value that's a path, or holds the account's name.
    wchar_t user[256] = L"";
    DWORD userLen = 256;
    GetUserNameW(user, &userLen);
    std::wstring me = user;
    for (auto& c : me) c = towlower(c);
    int shown = 0, hidden = 0;
    if (auto store = Current()) {
        for (auto const& [key, value] : store->values) {
            auto def = g_defaults.find(key);
            if (def != g_defaults.end() && def->second == value) continue;
            std::wstring low = value;
            for (auto& c : low) c = towlower(c);
            bool personal = value.find(L":\\") != std::wstring::npos || value.find(L":/") != std::wstring::npos || value.find(L"\\\\") != std::wstring::npos ||
                            value.find(L"%") != std::wstring::npos || (me.size() > 2 && low.find(me) != std::wstring::npos);
            if (personal) {
                out += key + L" = (left out: a path or a name)\r\n";
                hidden++;
            } else {
                out += key + L" = " + value + L"\r\n";
            }
            shown++;
        }
    }
    swprintf(line, 512, L"(%d changed, %d of them left out)\r\n", shown, hidden);
    out += line;
    return out;
}

static void CopyToClipboard(HWND wnd, const std::wstring& text) {
    if (!OpenClipboard(wnd)) return;
    EmptyClipboard();
    if (HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t))) {
        memcpy(GlobalLock(mem), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
        GlobalUnlock(mem);
        if (!SetClipboardData(CF_UNICODETEXT, mem)) GlobalFree(mem);
    }
    CloseClipboard();
}

static void TrayMenu(HWND wnd) {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 4, L"Open Tourne Studio");
    SetMenuDefaultItem(menu, 4, FALSE);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 1, L"Open my settings");
    AppendMenuW(menu, MF_STRING, 2, L"Open every setting and its default");
    AppendMenuW(menu, MF_STRING, 3, L"Open the settings folder");
    AppendMenuW(menu, MF_STRING, 5, L"Import my Windhawk suite settings");
    AppendMenuW(menu, MF_STRING, 6, L"Bring in my Tourne'Table setups from Windhawk");
    AppendMenuW(menu, MF_STRING, 7, L"Copy diagnostics (no paths or names)");
    AppendMenuW(menu, MF_STRING, 8, L"Designer mode: pick a colour, click a part");
    AppendMenuW(menu, MF_STRING, 10, L"Save my look to the gallery");
    AppendMenuW(menu, MF_STRING, 11, L"Open the look gallery");
    if (WindowsAccentSaved()) AppendMenuW(menu, MF_STRING, 12, L"Restore my Windows colours");
    InjectorMenu(menu, 100);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    for (int i = 0; i < kModuleCount; i++)
        AppendMenuW(menu, MF_STRING | MF_GRAYED | (g_running[i].on ? MF_CHECKED : 0), 0, kModules[i].title);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 9, L"Quit");
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(wnd);
    // In Tourne's menu look, as the bar draws its own (taskbar-host.cpp, TourneStyledMenu).
    using Styled = UINT (*)(HMENU, UINT, int, int, HWND);
    static Styled styled = (Styled)GetProcAddress(GetModuleHandleW(nullptr), "TourneStyledMenu");
    UINT flags = TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY;
    int id = styled ? (int)styled(menu, flags, pt.x, pt.y, wnd) : TrackPopupMenuEx(menu, flags, pt.x, pt.y, wnd, nullptr);
    PostMessageW(wnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
    if (id == 1) ShellExecuteW(nullptr, L"open", ConfigPath().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (id == 2) ShellExecuteW(nullptr, L"open", DefaultsPath().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (id == 3) ShellExecuteW(nullptr, L"open", ConfigDir().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (id == 4) StudioOpen(wnd);
    if (id == 5) {
        std::wstring report;
        bool ok = ImportSuite(0, &report);
        MessageBoxW(nullptr, report.c_str(), L"Tourne Desktop", ok ? MB_ICONINFORMATION : MB_ICONWARNING);
    }
    if (id == 6) {
        std::wstring report;
        bool ok = ImportTable(&report);
        MessageBoxW(nullptr, report.c_str(), L"Tourne Desktop", ok ? MB_ICONINFORMATION : MB_ICONWARNING);
    }
    if (id == 8) PostMessageW(wnd, RegisterWindowMessageW(L"TourneDesktop.Designer"), 0, 0);
    if (id == 10) PostMessageW(wnd, RegisterWindowMessageW(L"TourneDesktop.LookSave"), 0, 0);
    if (id == 11) ShellExecuteW(nullptr, L"open", DotsGalleryFolder().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (id == 12) {
        bool WriteStudioSettings(const std::vector<std::pair<std::wstring, std::wstring>>& set, const std::wstring& prefix,
                                 const std::wstring& why);  // (below)
        WriteStudioSettings({{L"theme.windows", L"off"}}, L"", L"accent-restore");  // (else the theme writes its accent again)
        RestoreWindowsAccent();
    }
    if (id == 7) {
        CopyToClipboard(wnd, Diagnostics());
        Log(L"diagnostics copied");
    }
    if (id == 9 && StudioClose()) DestroyWindow(wnd);
    if (id >= 100) InjectorMenuCommand(id, 100);
}

static UINT g_taskbarCreated, g_themeChanged;

// The explorerbar command: Tourne's bar and tray put away and Explorer's taskbar shown (its tray takes the icons,
// as apps are told a taskbar was made), or the other way round. to: 1 Explorer's, 2 Tourne's, 0 the other one.
static void SwapBar(WPARAM to) {
    bool explorer = to == 1 ? true : to == 2 ? false : !g_explorerBar;
    if (explorer == g_explorerBar) return;
    int bar = -1;
    for (int i = 0; i < kModuleCount; i++)
        if (!wcscmp(kModules[i].prefix, L"taskbar")) bar = i;
    if (explorer) {
        if (!ShellUnderneath()) {
            Log(L"explorerbar: there's no Explorer underneath to show (shell mode only)");
            return;
        }
        g_explorerBar = true;
        if (bar >= 0) StopModule(bar);
        TrayHostStop();
        ShellShowExplorerBar(true);
        PostMessageW(HWND_BROADCAST, RegisterWindowMessageW(L"TaskbarCreated"), 0, 0);  // (icons into Explorer's tray)
    } else {
        g_explorerBar = false;
        ShellShowExplorerBar(false);
        TrayHostStart();
        TrayHostRetake();
        if (bar >= 0 && Wanted(bar)) StartModule(bar);
    }
}

static LRESULT CALLBACK HostProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    static const UINT explorerBar = RegisterWindowMessageW(L"TourneDesktop.ExplorerBar");
    static const UINT designer = RegisterWindowMessageW(L"TourneDesktop.Designer");
    if (msg == designer && designer) {
        DesignerStart(wnd);
        return 0;
    }
    // The look as it is now into the gallery, its screenshot taken once the menu that asked has gone.
    static const UINT lookSave = RegisterWindowMessageW(L"TourneDesktop.LookSave");
    if (msg == lookSave && lookSave) {
        Sleep(250);
        std::wstring saved, error;
        Balloon b;
        b.app = L"Tourne Desktop";
        if (DotsSaveToGallery(L"", &saved, &error)) {
            Log(L"look saved: %s", saved.c_str());
            b.title = L"Look saved";
            b.body = saved.substr(saved.find_last_of(L'\\') + 1) + L", in the look gallery";
        } else {
            Log(L"look not saved: %s", error.c_str());
            b.title = L"Look not saved";
            b.body = error;
        }
        FlyoutsBalloon(b);
        return 0;
    }
    if (msg == explorerBar && explorerBar) {
        SwapBar(wp);
        return 0;
    }
    if (ThemeEngineMessage(wnd, msg, wp, lp) || InjectorMessage(wnd, msg, wp, lp) || GlassMessage(wnd, msg, wp, lp) ||
        KillswitchMessage(wnd, msg, wp, lp))
        return 0;
    if (msg == g_taskbarCreated && g_taskbarCreated) {
        UpdateTray(wnd);  // Explorer restarted: the icon again
        return 0;
    }
    if (msg == g_themeChanged && g_themeChanged) {
        if (g_usesRoles) {  // settings that are theme roles: their new colours
            Republish();
            ApplyConfig();
            if (g_started.count(L"glass")) GlassApply();
        }
        return 0;
    }
    switch (msg) {
        case WM_TD_RELOAD:
            // Editors save in bursts: once it settles.
            SetTimer(wnd, kTimerReload, 150, nullptr);
            return 0;
        case WM_TIMER:
            if (wp == kTimerStartup) {
                KillTimer(wnd, kTimerStartup);
                StartupStep();
                return 0;
            }
            if (wp == kTimerReload) {
                KillTimer(wnd, kTimerReload);
                auto before = Current();
                LoadConfig();
                auto after = Current();
                if (before && after && (before->user != after->user || before->gui != after->gui)) CrashTryAgain();
                ApplyConfig();
                ApplyServices();
                UpdateTray(wnd);
                StudioConfigReloaded();
                TellPlugins();
            }
            return 0;
        case WM_TD_STUDIO:
            if (auto* at = (std::pair<std::wstring, std::wstring>*)lp) {  // (OpenStudio: a page or a search)
                StudioOpenAt(wnd, at->first, at->second);
                delete at;
            } else {
                StudioOpen(wnd);
            }
            return 0;
        case WM_TD_MENU_SETTINGS:
            AllowSetForegroundWindow(ASFW_ANY);
            OpenMenuEditor();
            return 0;
        case WM_TD_BINDS_SETTINGS:
            AllowSetForegroundWindow(ASFW_ANY);
            OpenBindsEditor();
            return 0;
        case WM_TD_TRAY:
            if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_LBUTTONUP) TrayMenu(wnd);
            return 0;
        case WM_CLOSE:
            if (StudioClose()) DestroyWindow(wnd);
            return 0;
        case WM_DESTROY:
            // (closed on purpose, a newer build taking over or Quit, while a part
            // was still settling: that's no crash, so it isn't left off next time)
            CrashStarted();
            RestoreWindowsMotion();
            ShowTray(wnd, false);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////
// Start with Windows

static int Install(bool on) {
    HKEY run;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, nullptr, 0, KEY_SET_VALUE,
                        nullptr, &run, nullptr) != ERROR_SUCCESS)
        return 1;
    if (on) {
        wchar_t exe[MAX_PATH];
        GetModuleFileNameW(nullptr, exe, MAX_PATH);
        std::wstring cmd = L"\"" + std::wstring(exe) + L"\"";
        RegSetValueExW(run, L"TourneDesktop", 0, REG_SZ, (const BYTE*)cmd.c_str(), (DWORD)((cmd.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(run, L"TourneDesktop");
        RestoreWindowsAccent(true);  // Windows' colours as they were before Tourne, if it changed them
    }
    RegCloseKey(run);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// A newer build takes over

static std::wstring OwnExe() {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    return exe;
}

// Whether the exe at path a was written later than the one at b (a missing b counts as older).
static bool NewerThan(const std::wstring& a, const std::wstring& b) {
    WIN32_FILE_ATTRIBUTE_DATA x, y;
    if (!GetFileAttributesExW(a.c_str(), GetFileExInfoStandard, &x)) return false;
    if (!GetFileAttributesExW(b.c_str(), GetFileExInfoStandard, &y)) return true;
    return CompareFileTime(&x.ftLastWriteTime, &y.ftLastWriteTime) > 0;
}

// The running copy's window, process and exe, when it is an older build than this one.
static HANDLE OlderRunningCopy(HWND& window, std::wstring& path) {
    window = FindWindowW(kHostClass, nullptr);
    DWORD pid = 0;
    if (!window || !GetWindowThreadProcessId(window, &pid) || pid == GetCurrentProcessId()) return nullptr;
    HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return nullptr;
    wchar_t exe[MAX_PATH];
    DWORD size = MAX_PATH;
    // (not the copy kept for shell mode: closing that one would end the shell, and Explorer would start instead)
    bool got = QueryFullProcessImageNameW(process, 0, exe, &size);
    std::wstring lower = exe;
    for (auto& c : lower) c = towlower(c);
    if (got && _wcsicmp(exe, OwnExe().c_str()) != 0 && NewerThan(OwnExe(), exe) &&
        lower.find(L"\\tourne\\desktop\\bin\\") == std::wstring::npos) {
        path = exe;
        return process;
    }
    CloseHandle(process);
    return nullptr;
}

// Started with Windows from an older copy (or one since deleted): this build starts instead from now on.
static void TakeOverStartup() {
    wchar_t value[1024];
    DWORD size = sizeof(value);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"TourneDesktop", RRF_RT_REG_SZ,
                     nullptr, value, &size) != ERROR_SUCCESS)
        return;  // (not set to start with Windows: left that way)
    std::wstring listed = value;
    if (listed.size() >= 2 && listed.front() == L'"') listed = listed.substr(1, listed.find(L'"', 1) - 1);
    if (_wcsicmp(listed.c_str(), OwnExe().c_str()) == 0 || !NewerThan(OwnExe(), listed)) return;
    Install(true);
    Log(L"starts with Windows from here now (was %ls, an older build)", listed.c_str());
}

// An older build is running: it closes and this one carries on in its place (single is then this one's).
static bool TakeOverFromOlder(HANDLE& single, std::wstring& older) {
    HWND window;
    HANDLE process = OlderRunningCopy(window, older);
    if (!process) return false;
    PostMessageW(window, WM_CLOSE, 0, 0);
    bool closed = WaitForSingleObject(process, 8000) == WAIT_OBJECT_0;
    CloseHandle(process);
    if (!closed) return false;  // (it didn't close: it stays the one running)
    if (single) CloseHandle(single);
    single = CreateMutexW(nullptr, TRUE, L"Local\\TourneDesktop");
    return GetLastError() != ERROR_ALREADY_EXISTS;
}

// Settings a part writes as the Studio would (tourne-gui.conf, the one before kept as .before-<why>): rows it
// copies in (the Tourne Menu's from LiteMenu). Names that start with prefix and aren't in set are taken out.
// The config watch reads the file again.
bool WriteStudioSettings(const std::vector<std::pair<std::wstring, std::wstring>>& set, const std::wstring& prefix,
                         const std::wstring& why) {
    Settings gui;
    std::wstring text;
    if (ReadText(GuiPath(), &text)) {
        ParseConfig(text, gui, nullptr);
        CopyFileW(GuiPath().c_str(), (GuiPath() + L".before-" + why).c_str(), FALSE);
    }
    if (!prefix.empty())
        for (auto it = gui.begin(); it != gui.end();) it = it->first.rfind(prefix, 0) == 0 ? gui.erase(it) : std::next(it);
    for (auto const& [n, v] : set) gui[n] = v;
    std::wstring header =
        L"# Tourne Desktop: what Tourne Studio sets. It's read after tourne.conf, so a setting\n"
        L"# here wins over the same one there. The Studio rewrites this file when you save.\n\n";
    bool ok = WriteText(GuiPath(), WriteConfig(gui, header));
    Log(L"%s: %zu settings written to tourne-gui.conf%s", why.c_str(), set.size(), ok ? L"" : L" (failed)");
    return ok;
}

bool WriteSetting(const wchar_t* prefix, const std::wstring& key, const std::wstring& value) {
    std::wstring full = IsFullName(key) ? key : std::wstring(prefix) + L"." + key;
    return WriteStudioSettings({{full, value}}, L"", L"quick-settings");
}

// Tourne Studio from a part (any thread): at a page ("taskbar."), or with its search set (query).
void OpenStudio(const std::wstring& page, const std::wstring& query) {
    if (!g_host) return;
    auto* at = new std::pair<std::wstring, std::wstring>(page, query);
    if (!PostMessageW(g_host, WM_TD_STUDIO, 0, (LPARAM)at)) delete at;
}

static int Run(bool studio) {
    HANDLE single = CreateMutexW(nullptr, TRUE, L"Local\\TourneDesktop");
    std::wstring older;
    if (GetLastError() == ERROR_ALREADY_EXISTS && !TakeOverFromOlder(single, older)) {
        if (single) CloseHandle(single);
        if (studio)
            if (HWND w = FindWindowW(kHostClass, nullptr)) {
                AllowSetForegroundWindow(ASFW_ANY);
                PostMessageW(w, WM_TD_STUDIO, 0, 0);
            }
        return kAlreadyRunning;  // (the shell's watchdog waits on that one instead)
    }
    CrashGuardBegin();
    WriteStarterConfig();
    WriteDefaults();
    AutoImportTable();
    SyncTableStyles();
    LoadConfig();
    Log(L"Tourne Desktop starting, %d modules built in", kModuleCount);
    if (!older.empty()) Log(L"took over from an older build running at %ls", older.c_str());
    TakeOverStartup();

    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    g_themeChanged = RegisterWindowMessageW(L"TourneThemeChanged");
    WNDCLASSW wc{};
    wc.lpfnWndProc = HostProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kHostClass;
    RegisterClassW(&wc);
    g_host = CreateWindowExW(0, kHostClass, L"Tourne Desktop", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    ChangeWindowMessageFilterEx(g_host, g_taskbarCreated, MSGFLT_ALLOW, nullptr);
    UpdateTray(g_host);

    g_running.assign(kModuleCount, {});
    // (a part beside the suite first, started now: Icon Redirect before the desktop and bars load icons)
    // Then what's seen first: the theme, the taskbar, the switcher and system icons, and the services that
    // work out of sight after them (each part waits for the one before: crash.h).
    g_startup.clear();
    for (int i = 0; i < kModuleCount; i++)
        if (kModules[i].besideSuite) g_startup.push_back(std::wstring(L"module:") + kModules[i].prefix);
    for (auto part : {L"theme", L"module:taskbar", L"inject", L"glass", L"killswitch", L"module:switcher", L"module:systemIcons",
                      L"keystrokes", L"shader", L"capture", L"launcher", L"flyouts", L"appStyler", L"sounds"})
        g_startup.push_back(part);
    for (int i = 0; i < kModuleCount; i++) {
        std::wstring part = std::wstring(L"module:") + kModules[i].prefix;
        if (!kModules[i].besideSuite && std::find(g_startup.begin(), g_startup.end(), part) == g_startup.end())
            g_startup.push_back(part);
    }
    g_startingUp = true;
    StartupStep();
    ShellApply();         // (a newer build kept for the shell, if it's on)
    NotifyApply();        // (Windows' pop-ups off while Tourne draws them; quiet hours)
    ShellSessionStart();  // (as the shell: Windows told, startup apps)
    UpdateTray(g_host);   // (again: the first went to whatever tray was there before Tourne's own was up)
    g_pluginsSeen = SettingsWithPrefix(L"plugins.");  // (what the runtime starts with)
    PluginRuntimeStart();  // installed plugins, on their own thread (their bar items reach the taskbar when it's up)
    if (!SuiteInThisProcess()) DialogLookStart();  // (the suite's Theme Gaps would do the footer here itself)

    g_watchStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HANDLE watcher = CreateThread(nullptr, 0, WatchThread, nullptr, 0, nullptr);
    if (studio) StudioOpen(g_host);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    SetEvent(g_watchStop);
    if (watcher) {
        WaitForSingleObject(watcher, 2000);
        CloseHandle(watcher);
    }
    PluginRuntimeStop();
    if (g_started.count(L"glass")) GlassStop();
    if (g_started.count(L"inject")) InjectorStop();
    if (g_started.count(L"theme")) ThemeEngineStop();
    for (int i = kModuleCount - 1; i >= 0; i--) StopModule(i);
    if (g_started.count(L"keystrokes")) KeystrokesStop();
    if (g_started.count(L"shader")) ScreenShaderStop();
    if (g_started.count(L"capture")) CaptureStop();
    if (g_started.count(L"launcher")) LauncherStop();
    if (g_started.count(L"flyouts")) FlyoutsStop();
    if (g_started.count(L"appStyler")) AppStylerStop();
    if (g_started.count(L"sounds")) UiSoundsStop();
    if (g_started.count(L"killswitch")) KillswitchStop();  // (after the taskbar's own: what was there first goes back last)
    DesktopHostStop();
    TrayHostStop();
    ShellSessionStop();  // (Explorer underneath shown again, after Tourne's tray is gone)
    Log(L"Tourne Desktop stopped");
    if (single) CloseHandle(single);
    return 0;
}

// What the settings that aren't in a module's schema are, for the Studio:
// each module's switch, the host's own, the theme's and the app styles'.
static void HostSchema() {
    auto add = [](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = g_info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)g_info.size();
        return si;
    };
    for (int i = 0; i < kModuleCount; i++) {
        std::wstring key = std::wstring(kModules[i].prefix) + L".enabled";
        g_defaults[key] = L"0";
        add(key, L"Run it here", L"Turn it off in the Windhawk suite first, so there's only one.").isSwitch = true;
    }
    g_defaults[L"desktop.trayIcon"] = L"1";
    g_defaults[L"desktop.log"] = L"0";
    add(L"desktop", L"Tourne Desktop", L"The program itself.");
    add(L"desktop.trayIcon", L"Tray icon", L"Its icon and menu in the tray. Without it, run TourneDesktop.exe --studio to open the Studio.")
        .isSwitch = true;
    add(L"desktop.log", L"Keep a log file", L"desktop.log in %LOCALAPPDATA%\\Tourne\\Desktop, for when something goes wrong.").isSwitch = true;

    add(L"theme", L"Tourne theme", L"Colours made from your wallpaper or one colour, for every piece set to \"Tourne theme\" and every colour set to a theme role.");
    add(L"theme.source", L"Made from", L"").options = {{L"off", L"Nothing (off)"}, {L"wallpaper", L"Your wallpaper"}, {L"seed", L"One colour"}, {L"preset", L"A preset"}};
    add(L"theme.profile", L"Profile", L"How colourful it is.").options = {
        {L"adaptive", L"Adaptive"}, {L"monochrome", L"Monochrome"}, {L"muted", L"Muted"}, {L"vivid", L"Vivid"}, {L"pastel", L"Pastel"}};
    add(L"theme.mode", L"Dark or light", L"").options = {{L"auto", L"As Windows is"}, {L"dark", L"Dark"}, {L"light", L"Light"}};
    add(L"theme.seed", L"The one colour", L"Used when it's made from one colour.");
    {
        SettingInfo& preset = add(L"theme.preset", L"The preset", L"Used when it's made from a preset (above). The Presets page shows them.");
        preset.options = {{L"", L"None"}};
        for (auto const& p : Presets()) preset.options.push_back({p.id, p.name});
    }
    SettingInfo& contrast = add(L"theme.contrast", L"Text contrast", L"Against its ground, as WCAG measures it: 4.5 is AA, 7 is AAA.");
    contrast.hasMin = contrast.hasMax = true, contrast.min = 3, contrast.max = 21;
    add(L"theme.windows", L"Windows' own colours",
        L"Windows' accent colour (title bars, Start, Settings, every app that asks Windows) set from this theme's accent "
        L"each time it's made; \"both\" sets Windows' dark or light mode too, when the theme isn't following Windows' "
        L"(Dark or light, above). It changes how every app looks.")
        .options = {{L"off", L"Leave them (off)"}, {L"accent", L"The accent"}, {L"both", L"The accent and dark or light"}};

    add(L"inject", L"Style apps", L"Your own CSS in apps made of web pages. Nothing is injected until you turn an app on and say yes to the warning.");
    for (auto const& [key, value] : g_defaults) {
        if (key.rfind(L"inject.", 0) != 0 || key.rfind(L"inject.yours.", 0) == 0) continue;
        size_t dot = key.rfind(L'.');
        std::wstring app = key.substr(7, dot - 7), field = key.substr(dot + 1);
        std::wstring title = app;
        if (!title.empty()) title[0] = towupper(title[0]);
        add(L"inject." + app, title.c_str(), L"");
        if (field == L"on") add(key, L"Style it", L"Asks first, once.").isSwitch = true;
        if (field == L"css") add(key, L"Style sheets", L"In the css folder, comma between them. tourne\\ ones are made from your theme.");
        if (field == L"exe") add(key, L"Program", L"Found by itself wherever it's installed. Set this only for a different copy (a full path, or just its file name).");
        if (field == L"restart")
            add(key, L"Opened without styles", L"").options = {{L"ask", L"Ask to restart it"}, {L"always", L"Restart it"}, {L"never", L"Leave it"}};
    }
    add(L"inject.yours", L"Your apps",
        L"Style any program you add here, all with the same style sheets. Apps made of web pages (Chromium and Electron) "
        L"take styles; others are left alone. Each asks first, once.");
    add(L"inject.yours.programs", L"Programs", L"Add a running program, or find its file.");
    add(L"inject.yours.css", L"Style sheets", L"In the css folder, comma between them. tourne\\ ones are made from your theme.");
    add(L"inject.yours.restart", L"Opened without styles", L"").options = {
        {L"ask", L"Ask to restart it"}, {L"always", L"Restart it"}, {L"never", L"Leave it"}};
}

static void InstallCrashGuard() { CrashGuardInstall(DataDir()); }

static void LoadDefaults() {
    for (int i = 0; i < kModuleCount; i++)
        ParseSchemaDefaults(kModules[i].prefix, kModules[i].settingsYaml ? kModules[i].settingsYaml : "", g_defaults, &g_info);
    ThemeDefaults(g_defaults);
    InjectorDefaults(g_defaults);
    GlassDefaults(g_defaults);
    IdentityDefaults(g_defaults);
    KillswitchDefaults(g_defaults);
    KeystrokesDefaults(g_defaults);
    ScreenShaderDefaults(g_defaults);
    CaptureDefaults(g_defaults);
    LauncherDefaults(g_defaults);
    FlyoutsDefaults(g_defaults);
    AppStylerDefaults(g_defaults);
    UiSoundsDefaults(g_defaults);
    ShellDefaults(g_defaults);
    NotifyDefaults(g_defaults);
    MotionDefaults(g_defaults);
    HostSchema();
    GlassDescribe(g_info);
    IdentityDescribe(g_info);
    KillswitchDescribe(g_info);
    KeystrokesDescribe(g_info);
    ScreenShaderDescribe(g_info);
    CaptureDescribe(g_info);
    LauncherDescribe(g_info);
    FlyoutsDescribe(g_info);
    AppStylerDescribe(g_info);
    UiSoundsDescribe(g_info);
    ShellDescribe(g_info);
    NotifyDescribe(g_info);
    MotionDescribe(g_info);
    PluginSettings(g_defaults, g_info);
    RefreshDynamicOptions();
    // The switcher's "Colour scheme" style reads System Icons' scheme, which
    // has no part of its own here: given one, with the taskbar's choices.
    g_defaults[L"systemIcons.scheme"] = L"tourne";
    SettingInfo& scheme = g_info[L"systemIcons.scheme"];
    scheme.name = L"Colour scheme (for its \"Colour scheme\" style)";
    scheme.description = L"What the switcher uses when its style is the colour scheme. System Icons reads it too in the Windhawk suite.";
    if (auto it = g_info.find(L"taskbar.scheme"); it != g_info.end())
        for (auto const& o : it->second.options)
            if (o.first != L"matchIcons") scheme.options.push_back(o);
    if (auto it = g_info.find(L"switcher.theme.style"); it != g_info.end()) scheme.order = it->second.order;
    // Answers written as text that come from a short set: the Studio offers the set (a value typed into
    // tourne.conf that isn't in it still works, and shows as it is).
    if (auto it = g_info.find(L"systemIcons.batteries.pollInterval"); it != g_info.end() && it->second.options.empty())
        it->second.options = {{L"30sec", L"Every 30 seconds"}, {L"1min", L"Every minute"},   {L"5min", L"Every 5 minutes"},
                              {L"15min", L"Every 15 minutes"}, {L"30min", L"Every 30 minutes"}, {L"1hr", L"Every hour"},
                              {L"2hr", L"Every 2 hours"},     {L"6hr", L"Every 6 hours"},      {L"12hr", L"Every 12 hours"}};
    // Numbers with a sensible span: a slider in the Studio instead of a bare box (typed values outside still work).
    static const struct {
        const wchar_t* key;
        double min, max;
    } kSpans[] = {
        {L"switcher.badgeLayout.offsetX", -40, 40},       {L"switcher.badgeLayout.offsetY", -40, 40},
        {L"switcher.badgeLayout.padding", 0, 24},         {L"switcher.borders.fadeDuration", 0, 5000},
        {L"switcher.borders.fps", 10, 240},               {L"switcher.borders.glowBlur", 0, 40},
        {L"switcher.borders.glowOpacity", 0, 100},        {L"switcher.borders.glowSpread", 0, 40},
        {L"switcher.borders.initDelay", 0, 2000},         {L"switcher.borders.offset", -20, 20},
        {L"switcher.borders.radiusCustom", 0, 32},        {L"switcher.borders.shadowBlur", 0, 40},
        {L"switcher.borders.shadowOpacity", 0, 100},      {L"switcher.borders.shadowX", -40, 40},
        {L"switcher.borders.shadowY", -40, 40},           {L"switcher.borders.spinDuration", 250, 20000},
        {L"switcher.borders.unminimizeDelay", 0, 2000},   {L"switcher.borders.width", 1, 20},
        {L"switcher.colors.countOpacity", 0, 100},        {L"switcher.colors.iconBackgroundOpacity", 0, 100},
        {L"switcher.corners.radius", 0, 32},              {L"switcher.font.size", 6, 36},
        {L"switcher.keys.showDelay", 0, 1000},            {L"switcher.layout.maxHeight", 10, 100},
        {L"switcher.layout.maxWidth", 10, 100},           {L"switcher.layout.rowHeight", 60, 600},
        {L"switcher.layout.rowWidth", 0, 800},            {L"switcher.lightColors.countOpacity", 0, 100},
        {L"switcher.lightColors.iconBackgroundOpacity", 0, 100}, {L"switcher.view.blur", 0, 10},
        {L"switcher.view.dim", 0, 100},                   {L"systemIcons.flyoutAlong", -200, 200},
        {L"systemIcons.flyoutAway", -40, 100},            {L"systemIcons.menuAlong", -200, 200},
        {L"systemIcons.menuAway", -40, 100},              {L"systemIcons.notificationPopupSeconds", 1, 60},
        {L"taskbar.menu.items[0].depth", 0, 6},           {L"taskbar.startMenu.phoneNotifications", 0, 8}, {L"taskbar.startMenu.phoneMessages", 0, 8},
    };
    for (auto const& s : kSpans)
        if (auto it = g_info.find(s.key); it != g_info.end() && !it->second.hasMin && !it->second.hasMax) {
            it->second.hasMin = it->second.hasMax = true;
            it->second.min = s.min;
            it->second.max = s.max;
        }
    if (auto it = g_info.find(L"theme.contrast"); it != g_info.end())
        it->second.options = {{L"3", L"3 (low)"}, {L"4.5", L"4.5 (AA, the usual)"}, {L"7", L"7 (AAA, the most readable)"}};
    for (auto key : {L"notify.quietFrom", L"notify.quietTo"})
        if (auto it = g_info.find(key); it != g_info.end() && it->second.options.empty())
            for (int half = 0; half < 48; half++) {
                wchar_t t[8];
                swprintf(t, 8, L"%02d:%02d", half / 2, half % 2 * 30);
                it->second.options.push_back({t, t});
            }
}

// --restore: Windows' taskbar back as it was, by hand: after a crash that left it hidden, or any time. Closes a
// running copy first so it doesn't hide it again.
static int RestoreWindowsTaskbar() {
    if (HWND w = FindWindowW(kHostClass, nullptr)) {
        DWORD pid = 0;
        GetWindowThreadProcessId(w, &pid);
        HANDLE process = pid ? OpenProcess(SYNCHRONIZE, FALSE, pid) : nullptr;
        PostMessageW(w, WM_CLOSE, 0, 0);
        if (process) {
            WaitForSingleObject(process, 5000);
            CloseHandle(process);
        }
    }
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD saved = 0, size = sizeof(saved);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\TourneTray", L"SystemTaskbarState", RRF_RT_REG_DWORD, nullptr, &saved, &size) == ERROR_SUCCESS && saved && tray) {
        APPBARDATA abd{sizeof(abd)};
        abd.hWnd = tray;
        abd.lParam = saved - 1;
        SHAppBarMessage(ABM_SETSTATE, &abd);
        RegDeleteKeyValueW(HKEY_CURRENT_USER, L"Software\\TourneTray", L"SystemTaskbarState");
    }
    for (HWND h = tray; h; h = FindWindowExW(nullptr, h == tray ? nullptr : h, L"Shell_SecondaryTrayWnd", nullptr)) {
        if (!IsWindowVisible(h)) ShowWindow(h, SW_SHOWNA);
        if (h == tray && !FindWindowExW(nullptr, nullptr, L"Shell_SecondaryTrayWnd", nullptr)) break;
    }
    KillswitchRestoreSaved();  // (after the taskbar's own: what was there first goes back last)
    ShellRestore();
    return 0;
}

// --menu-settings: in the running copy, else on their own (with the settings read as that copy would).
int MenuSettingsOnly() {
    if (HWND w = FindWindowW(kHostClass, nullptr)) {
        AllowSetForegroundWindow(ASFW_ANY);
        PostMessageW(w, WM_TD_MENU_SETTINGS, 0, 0);
        return 0;
    }
    LoadConfig();
    RunMenuEditor();
    return 0;
}

// --binds-settings: key binds, window rules and idle actions (binds-editor.cpp), the same way.
int BindsSettingsOnly() {
    if (HWND w = FindWindowW(kHostClass, nullptr)) {
        AllowSetForegroundWindow(ASFW_ANY);
        PostMessageW(w, WM_TD_BINDS_SETTINGS, 0, 0);
        return 0;
    }
    LoadConfig();
    RunBindsEditor();
    return 0;
}

// --glass-admin: the glass for admin apps, from its logon task (glass.cpp).
int GlassAdminOnly() {
    LoadConfig();
    return GlassAdminRun(&LoadConfig);
}

}  // namespace td

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR cmd, int) {
    td::InstallCrashGuard();
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    td::LoadDefaults();
    std::wstring args = cmd ? cmd : L"";
    if (args.find(L"--import") != std::wstring::npos) return td::Import(args);
    if (size_t at = args.find(L"--studio-kinds"); at != std::wstring::npos) {
        std::wstring path = args.substr(at + 14);
        path.erase(0, path.find_first_not_of(L" \""));
        path.erase(path.find_last_not_of(L" \"") + 1);
        return td::StudioWriteKinds(path) ? 0 : 1;
    }
    if (args.find(L"--glass-preview") != std::wstring::npos) return td::GlassPreview(args);
    if (args.find(L"--glass-admin-setup") != std::wstring::npos) return td::GlassAdminSetup();
    if (args.find(L"--glass-admin") != std::wstring::npos) return td::GlassAdminOnly();
    if (size_t at = args.find(L"--shader-shot "); at != std::wstring::npos) {  // --shader-shot EFFECT SCREEN out.bmp
        std::vector<std::wstring> words;
        for (size_t i = at + 14; i < args.size() && words.size() < 3;) {
            while (i < args.size() && args[i] == L' ') i++;
            bool quoted = i < args.size() && args[i] == L'"';
            size_t end = args.find(quoted ? L'"' : L' ', i + quoted);
            if (end == std::wstring::npos) end = args.size();
            words.push_back(args.substr(i + quoted, end - i - quoted));
            i = end + quoted;
        }
        if (words.size() < 3) return 1;
        return td::ScreenShaderShot(words[0], _wtoi(words[1].c_str()), words[2]);
    }
    if (size_t at = args.find(L"--msg "); at != std::wstring::npos) {  // (to the running copy's plugins)
        std::vector<std::wstring> words;
        int n = 0;
        if (LPWSTR* parts = CommandLineToArgvW((L"x " + args.substr(at + 6)).c_str(), &n)) {
            for (int i = 1; i < n; i++) words.push_back(parts[i]);
            LocalFree(parts);
        }
        return td::PluginMessage(words);
    }
    if (args.find(L"--menu-settings") != std::wstring::npos) return td::MenuSettingsOnly();
    if (args.find(L"--binds-settings") != std::wstring::npos) return td::BindsSettingsOnly();
    if (args.find(L"--uninstall") != std::wstring::npos) return td::Install(false);
    if (args.find(L"--install") != std::wstring::npos) return td::Install(true);
    if (args.find(L"--restore") != std::wstring::npos) return td::RestoreWindowsTaskbar();
    if (args.find(L"--panic") != std::wstring::npos) return td::ShellPanic();
    if (args.find(L"--shell") != std::wstring::npos && !td::StartedAsShell(args)) return td::ShellWatchdog();
    if (args.find(L"--stop") != std::wstring::npos) {
        if (HWND w = FindWindowW(td::kHostClass, nullptr)) PostMessageW(w, WM_CLOSE, 0, 0);
        return 0;
    }
    return td::Run(args.find(L"--studio") != std::wstring::npos);
}
