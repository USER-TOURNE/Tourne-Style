#include "plugins.h"

#include <shlobj.h>
#include <winhttp.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <set>

#include "config.h"
#include "json-lite.h"
#include "td.h"
#include "toml-lite.h"

namespace td {

namespace {

// desktop/plugins/index.json, as UTF-8 bytes (the build writes it).
#include "plugins-index.inc"
// Tourne's own plugins: desktop/plugins/bundled/index.json and each one's files (the build writes it).
struct BundledFile {
    const char* plugin;  // its id
    const char* path;    // in its folder, with "/"
    const unsigned char* bytes;
    size_t size;
};
#include "plugins-bundled.inc"

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"plugins", format, args);
    va_end(args);
}

std::wstring W(const json::Value& v) { return Utf8ToWide(v.text); }

std::vector<std::wstring> Strings(const json::Value& v) {
    std::vector<std::wstring> out;
    for (auto const& item : v.items) out.push_back(W(item));
    return out;
}

struct Catalogue {
    std::vector<PluginInfo> plugins;
    int leftOut = 0;
};

const Catalogue& Read() {
    static const Catalogue catalogue = [] {
        Catalogue c;
        json::Value index, bundled;
        if (!json::Parse((const char*)kPluginIndex, &index)) {
            Log(L"the plugin index didn't read");
            return c;
        }
        if (!json::Parse((const char*)kBundledIndex, &bundled)) Log(L"the index of Tourne's own plugins didn't read");
        std::vector<const json::Value*> all;  // (Tourne's own first)
        for (auto const& v : bundled["plugins"].items) all.push_back(&v);
        for (auto const& v : index["plugins"].items) all.push_back(&v);
        for (const json::Value* item : all) {
            const json::Value& v = *item;
            PluginInfo p;
            p.id = W(v["id"]), p.name = W(v["name"]), p.version = W(v["version"]), p.author = W(v["author"]);
            p.license = W(v["license"]), p.description = W(v["description"]), p.icon = W(v["icon"]);
            p.repo = W(v["repo"]), p.rev = W(v["rev"]), p.path = W(v["path"]);
            p.api = (int)v["api"].number;
            p.tags = Strings(v["tags"]), p.entries = Strings(v["entries"]), p.files = Strings(v["files"]);
            for (auto const& n : v["needs"].items) {
                PluginNeed need{W(n["kind"]), L""};
                if (need.kind == L"internet")
                    for (auto const& h : n["hosts"].items) need.detail += (need.detail.empty() ? L"" : L", ") + W(h);
                if (need.kind == L"program" || need.kind == L"reads" || need.kind == L"deletes") need.detail = W(n["label"]);
                for (auto const& path : n["paths"].items) need.paths.push_back(W(path));
                p.needs.push_back(need);
            }
            if (p.name.empty()) p.name = p.id;
            p.bundled = v["bundled"].kind == json::Value::Bool && v["bundled"].boolean;
            if (p.bundled) {
                p.files.clear();
                for (size_t i = 0; i < kBundledFileCount; i++)
                    if (p.id == Utf8ToWide(kBundledFiles[i].plugin)) p.files.push_back(Utf8ToWide(kBundledFiles[i].path));
            }
            c.plugins.push_back(std::move(p));
        }
        c.leftOut = (int)index["excluded"].items.size();
        return c;
    }();
    return catalogue;
}

std::mutex g_lock;
std::map<std::wstring, PluginState> g_busy;  // installing, or failed (installed is read from the folder)
std::map<std::wstring, std::wstring> g_errors;

std::wstring PluginsRoot() {
    wchar_t test[MAX_PATH];  // (tools\plugins\plugintest.cpp installs somewhere else)
    if (GetEnvironmentVariableW(L"TOURNE_PLUGINS_DIR", test, MAX_PATH)) return test;
    PWSTR local = nullptr;
    std::wstring root;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) root = std::wstring(local) + L"\\Tourne\\Plugins";
    CoTaskMemFree(local);
    return root;
}

// A path's parts as a URL wants them (UTF-8, percent-escaped; "/" kept).
std::wstring UrlPath(const std::wstring& path) {
    std::wstring out;
    for (unsigned char c : WideToUtf8(path)) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '/') {
            out += (wchar_t)c;
        } else {
            wchar_t hex[4];
            swprintf(hex, 4, L"%%%02X", c);
            out += hex;
        }
    }
    return out;
}

// One file from raw.githubusercontent.com. False (and why) if it didn't come.
bool Fetch(HINTERNET connect, const std::wstring& path, std::string* body, std::wstring* why) {
    HINTERNET request = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                           WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    bool ok = request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
              WinHttpReceiveResponse(request, nullptr);
    DWORD status = 0, size = sizeof(status);
    if (ok) WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size, nullptr);
    if (ok && status == 200) {
        for (;;) {
            DWORD available = 0;
            if (!WinHttpQueryDataAvailable(request, &available) || !available) break;
            size_t at = body->size();
            body->resize(at + available);
            DWORD got = 0;
            if (!WinHttpReadData(request, body->data() + at, available, &got)) {
                ok = false;
                break;
            }
            body->resize(at + got);
        }
    }
    if (!ok) *why = L"no connection to GitHub (error " + std::to_wstring(GetLastError()) + L")";
    else if (status != 200) *why = L"GitHub answered " + std::to_wstring(status) + L" for " + path;
    if (request) WinHttpCloseHandle(request);
    return ok && status == 200;
}

bool WriteAll(const std::filesystem::path& path, const std::string& bytes) {
    std::error_code e;
    std::filesystem::create_directories(path.parent_path(), e);
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    bool ok = WriteFile(f, bytes.data(), (DWORD)bytes.size(), &written, nullptr) && written == bytes.size();
    CloseHandle(f);
    return ok;
}

// What was agreed to, kept beside the plugin: what the runtime allows it.
std::string Record(const PluginInfo& p) {
    SYSTEMTIME t;
    GetSystemTime(&t);
    char when[32];
    snprintf(when, sizeof(when), "%04d-%02d-%02dT%02d:%02d:%02dZ", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    std::string granted;
    for (auto const& n : p.needs) granted += (granted.empty() ? "" : ", ") + json::Quote(WideToUtf8(n.kind));
    std::wstring repo = p.bundled ? L"bundled with Tourne Desktop" : p.repo;
    return "{\n  \"id\": " + json::Quote(WideToUtf8(p.id)) + ",\n  \"version\": " + json::Quote(WideToUtf8(p.version)) +
           ",\n  \"repo\": " + json::Quote(WideToUtf8(repo)) + ",\n  \"rev\": " + json::Quote(WideToUtf8(p.rev)) +
           ",\n  \"installed\": " + json::Quote(when) + ",\n  \"granted\": [" + granted + "]\n}\n";
}

struct Job {
    PluginInfo plugin;
    HWND notify;
    UINT msg;
};

// One of Tourne's own plugins' files written into a folder from the program.
bool WriteBundled(const PluginInfo& p, const std::filesystem::path& folder, std::wstring* why) {
    for (size_t i = 0; i < kBundledFileCount; i++) {
        BundledFile const& f = kBundledFiles[i];
        if (p.id != Utf8ToWide(f.plugin)) continue;
        if (!WriteAll(folder / std::filesystem::path(Utf8ToWide(f.path)).make_preferred(), std::string((const char*)f.bytes, f.size))) {
            *why = L"couldn't write " + Utf8ToWide(f.path);
            return false;
        }
    }
    return true;
}

DWORD WINAPI InstallThread(LPVOID param) {
    std::unique_ptr<Job> job((Job*)param);
    PluginInfo const& p = job->plugin;
    std::filesystem::path final = PluginFolder(p), temp = final;
    temp += L".installing";
    std::error_code e;
    std::filesystem::remove_all(temp, e);
    std::wstring why;
    HINTERNET session = p.bundled ? nullptr : WinHttpOpen(L"Tourne Desktop", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
                                                          WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET connect = session ? WinHttpConnect(session, L"raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0) : nullptr;
    bool ok = connect != nullptr;
    if (!ok) why = L"no connection to GitHub";
    if (p.bundled) ok = WriteBundled(p, temp, &why);
    for (size_t i = 0; ok && !p.bundled && i < p.files.size(); i++) {
        std::string body;
        std::wstring url = L"/" + UrlPath(p.repo + L"/" + p.rev + L"/" + p.path + L"/" + p.files[i]);
        ok = Fetch(connect, url, &body, &why);
        if (ok && !WriteAll(temp / std::filesystem::path(p.files[i]).make_preferred(), body)) {
            ok = false;
            why = L"couldn't write " + p.files[i];
        }
    }
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
    if (ok && !WriteAll(temp / L"tourne-plugin.json", Record(p))) ok = false, why = L"couldn't write tourne-plugin.json";
    if (ok) {
        std::filesystem::remove_all(final, e);
        std::filesystem::rename(temp, final, e);
        if (e) ok = false, why = L"couldn't move it into place (" + Utf8ToWide(e.message()) + L")";
    }
    if (!ok) std::filesystem::remove_all(temp, e);
    {
        std::lock_guard lock(g_lock);
        if (ok) g_busy.erase(p.id), g_errors.erase(p.id);
        else g_busy[p.id] = PluginState::Failed, g_errors[p.id] = why;
    }
    if (ok) Log(L"installed %s %s (%zu files) in %s", p.id.c_str(), p.version.c_str(), p.files.size(), final.c_str());
    else Log(L"%s didn't install: %s", p.id.c_str(), why.c_str());
    if (IsWindow(job->notify)) PostMessageW(job->notify, job->msg, 0, 0);
    return 0;
}

}  // namespace

const std::vector<PluginInfo>& PluginCatalogue() { return Read().plugins; }
int PluginsLeftOut() { return Read().leftOut; }

std::wstring PluginFolder(const PluginInfo& p) {
    std::wstring id = p.id;
    for (auto& c : id)
        if (c == L'/') c = L'\\';
        else if (wcschr(L"<>:\"|?*", c)) c = L'_';
    return PluginsRoot() + L"\\" + id;
}

PluginState PluginStateOf(const PluginInfo& p) {
    {
        std::lock_guard lock(g_lock);
        if (auto it = g_busy.find(p.id); it != g_busy.end()) return it->second;
    }
    DWORD a = GetFileAttributesW((PluginFolder(p) + L"\\tourne-plugin.json").c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY) ? PluginState::Installed : PluginState::NotInstalled;
}

std::wstring PluginError(const PluginInfo& p) {
    std::lock_guard lock(g_lock);
    auto it = g_errors.find(p.id);
    return it != g_errors.end() ? it->second : L"";
}

void PluginInstall(const PluginInfo& p, HWND notify, UINT msg) {
    {
        std::lock_guard lock(g_lock);
        if (g_busy.count(p.id) && g_busy[p.id] == PluginState::Installing) return;
        g_busy[p.id] = PluginState::Installing;
        g_errors.erase(p.id);
    }
    Log(L"installing %s %s from %s at %.7s", p.id.c_str(), p.version.c_str(), p.repo.c_str(), p.rev.c_str());
    if (HANDLE t = CreateThread(nullptr, 0, InstallThread, new Job{p, notify, msg}, 0, nullptr)) {
        CloseHandle(t);
    } else {
        std::lock_guard lock(g_lock);
        g_busy[p.id] = PluginState::Failed;
        g_errors[p.id] = L"couldn't start the download";
    }
}

namespace {

std::string ReadText(const std::filesystem::path& path) {
    std::string out;
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return out;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(f, &size) && size.QuadPart < (4 << 20)) {
        out.resize((size_t)size.QuadPart);
        DWORD got = 0;
        if (!ReadFile(f, out.data(), (DWORD)out.size(), &got, nullptr)) got = 0;
        out.resize(got);
    }
    CloseHandle(f);
    return out;
}

// A translation by its dotted key ("settings.show_clocks.label"), in the user's language, then English.
std::wstring Translated(const json::Value& mine, const json::Value& english, const std::string& key) {
    for (const json::Value* root : {&mine, &english}) {
        const json::Value* at = root;
        size_t start = 0;
        while (at && at->kind == json::Value::Object) {
            size_t dot = key.find('.', start);
            const json::Value& next = (*at)[key.substr(start, dot - start).c_str()];
            at = next.IsNull() ? nullptr : &next;
            if (dot == std::string::npos) break;
            start = dot + 1;
        }
        if (at && at->kind == json::Value::String) return Utf8ToWide(at->text);
    }
    return {};
}

// Settings whose choices are the sound outputs (options_from = "sound-outputs"), filled when the Studio opens.
std::set<std::wstring> g_soundOutputRows;

// None, Windows' default output, then each output Windows has now by name (its id the value, as
// tourne.systemSound takes it).
std::vector<std::pair<std::wstring, std::wstring>> SoundOutputChoices() {
    using Microsoft::WRL::ComPtr;
    std::vector<std::pair<std::wstring, std::wstring>> out = {{L"none", L"None"}, {L"default", L"Default output (follows Windows)"}};
    static const PROPERTYKEY kFriendlyName = {{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    {
        ComPtr<IMMDeviceEnumerator> devices;
        ComPtr<IMMDeviceCollection> list;
        UINT count = 0;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&devices))) &&
            SUCCEEDED(devices->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &list)) && SUCCEEDED(list->GetCount(&count)))
            for (UINT i = 0; i < count; i++) {
                ComPtr<IMMDevice> d;
                ComPtr<IPropertyStore> props;
                LPWSTR id = nullptr;
                if (FAILED(list->Item(i, &d)) || FAILED(d->GetId(&id)) || !id) continue;
                std::wstring name = id;
                PROPVARIANT v;
                PropVariantInit(&v);
                if (SUCCEEDED(d->OpenPropertyStore(STGM_READ, &props)) && SUCCEEDED(props->GetValue(kFriendlyName, &v)) &&
                    v.vt == VT_LPWSTR && v.pwszVal)
                    name = v.pwszVal;
                PropVariantClear(&v);
                out.push_back({id, name});
                CoTaskMemFree(id);
            }
    }
    if (SUCCEEDED(com)) CoUninitialize();
    return out;
}

}  // namespace

void PluginDynamicOptions(SchemaInfo& info) {
    if (g_soundOutputRows.empty()) return;
    auto outputs = SoundOutputChoices();
    for (auto const& row : g_soundOutputRows)
        if (info.count(row)) info[row].options = outputs;
}

void PluginSettings(Settings& defaults, SchemaInfo& info) {
    for (auto it = defaults.lower_bound(L"plugins."); it != defaults.end() && it->first.rfind(L"plugins.", 0) == 0;) it = defaults.erase(it);
    for (auto it = info.lower_bound(L"plugins"); it != info.end() && it->first.rfind(L"plugins", 0) == 0;) it = info.erase(it);
    g_soundOutputRows.clear();
    info[L"plugins"] = {L"Plugins", L"The settings of the plugins you installed (Extensions). A change restarts them."};
    std::error_code e;
    std::vector<std::filesystem::path> folders;
    for (auto const& author : std::filesystem::directory_iterator(PluginsRoot(), e))
        if (author.is_directory(e) && author.path().filename().wstring()[0] != L'.')
            for (auto const& plugin : std::filesystem::directory_iterator(author.path(), e))
                if (std::filesystem::exists(plugin.path() / L"tourne-plugin.json", e)) folders.push_back(plugin.path());
    std::sort(folders.begin(), folders.end());
    wchar_t locale[LOCALE_NAME_MAX_LENGTH] = L"en";
    GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH);
    std::wstring lang = locale, base = lang.substr(0, lang.find(L'-'));
    for (auto const& folder : folders) {
        json::Value manifest, mine, english;
        if (!toml::Parse(ReadText(folder / L"plugin.toml"), &manifest)) continue;
        std::wstring id = Utf8ToWide(manifest["id"].text);
        if (id.empty()) continue;
        for (auto const& name : {lang, base})
            if (mine.IsNull()) json::Parse(ReadText(folder / L"translations" / (name + L".json")), &mine);
        json::Parse(ReadText(folder / L"translations" / L"en.json"), &english);
        auto order = [&] { return (int)info.size() + 100000; };  // (after every other page's)
        std::wstring group = L"plugins." + id;
        info[group] = {Utf8ToWide(manifest["name"].text.empty() ? manifest["id"].text : manifest["name"].text),
                       Utf8ToWide(manifest["description"].text)};
        info[group].order = order();
        defaults[group + L".enabled"] = L"1";
        SettingInfo& run = info[group + L".enabled"];
        run = {L"Run it", L"Off: installed, but not running."};
        run.isSwitch = true, run.order = order();
        auto add = [&](const json::Value& list) {
            for (auto const& s : list.items) {
                std::string key = s["key"].text, type = s["type"].text;
                if (key.empty() || key.find('.') != std::string::npos) continue;
                std::wstring full = group + L"." + Utf8ToWide(key);
                if (defaults.count(full)) continue;  // (declared by another entry already)
                const json::Value& d = s["default"];
                std::wstring value;
                if (d.kind == json::Value::Bool) value = d.boolean ? L"1" : L"0";
                else if (d.kind == json::Value::Number) {
                    wchar_t b[32];
                    swprintf(b, 32, L"%.15g", d.number);
                    value = b;
                } else if (d.kind == json::Value::String) value = Utf8ToWide(d.text);
                bool list = type == "string_list" || d.kind == json::Value::Array;
                if (d.kind == json::Value::Array) {  // (a list: a row of text, its items between commas)
                    for (auto const& item : d.items) {
                        if (item.kind != json::Value::String && item.kind != json::Value::Number) list = false;
                        wchar_t b[32];
                        if (item.kind == json::Value::Number) swprintf(b, 32, L"%.15g", item.number);
                        value += (value.empty() ? L"" : L", ") + (item.kind == json::Value::Number ? std::wstring(b) : Utf8ToWide(item.text));
                    }
                    if (!list) continue;  // (a list of tables: not a row the Studio has)
                } else if (d.kind != json::Value::Null && d.kind != json::Value::Bool && d.kind != json::Value::Number &&
                           d.kind != json::Value::String)
                    continue;  // (tables: not a row the Studio has)
                defaults[full] = value;
                SettingInfo& si = info[full];
                si.name = Translated(mine, english, s["label_key"].text);
                if (si.name.empty()) si.name = Utf8ToWide(s["label"].text);  // (Tourne's own plugins say it in place)
                if (si.name.empty()) si.name = Utf8ToWide(key);
                si.description = Translated(mine, english, s["description_key"].text);
                if (si.description.empty()) si.description = Utf8ToWide(s["description"].text);
                if (list) si.description += (si.description.empty() ? L"" : L" ") + std::wstring(L"Several: put commas between them.");
                si.order = order();
                if (type == "bool") si.isSwitch = true;
                if (key == "hotkey" || s["format"].text == "hotkey") si.format = L"hotkey";  // (the Studio asks for the keys)
                if (type == "select")
                    for (auto const& o : s["options"].items) {
                        std::wstring label = Translated(mine, english, o["label_key"].text);
                        if (label.empty()) label = Utf8ToWide(o["label"].text);
                        si.options.push_back({Utf8ToWide(o["value"].text), label.empty() ? Utf8ToWide(o["value"].text) : label});
                    }
                if (type == "select" && s["options_from"].text == "sound-outputs") g_soundOutputRows.insert(full);
                if (s["min"].kind == json::Value::Number) si.hasMin = true, si.min = s["min"].number;
                if (s["max"].kind == json::Value::Number) si.hasMax = true, si.max = s["max"].number;
            }
        };
        // Each desktop widget: shown on the desktop only when turned on here (plugin-runtime.cpp reads it).
        for (auto const& entry : manifest["desktop_widget"].items) {
            std::wstring entryId = Utf8ToWide(entry["id"].text);
            if (entryId.empty() || entryId.find(L'.') != std::wstring::npos) continue;
            std::wstring full = group + L".desktop_" + entryId;
            defaults[full] = L"0";
            SettingInfo& si = info[full];
            std::wstring name = Translated(mine, english, entry["name_key"].text);
            if (name.empty()) name = Utf8ToWide(entry["name"].text);
            si.name = L"On the desktop" + (manifest["desktop_widget"].items.size() > 1 ? L": " + (name.empty() ? entryId : name) : L"");
            si.description = L"Its desktop widget, on your desktop. Drag it by any empty part to move it; it stays where you leave it.";
            si.isSwitch = true, si.order = order();
        }
        add(manifest["setting"]);
        for (const char* kind : {"widget", "service", "panel", "desktop_widget", "shortcut", "launcher_provider"})
            for (auto const& entry : manifest[kind].items) add(entry["setting"]);
    }
}

void PluginRefreshBundled() {
    for (auto const& p : PluginCatalogue()) {
        if (!p.bundled || PluginStateOf(p) != PluginState::Installed) continue;
        std::filesystem::path final = PluginFolder(p), temp = final;
        json::Value record;
        if (!json::Parse(ReadText(final / L"tourne-plugin.json"), &record) || Utf8ToWide(record["version"].text) == p.version) continue;
        // A newer build: its files replace the old ones; what was agreed to stays, with what this version needs.
        temp += L".installing";
        std::error_code e;
        std::filesystem::remove_all(temp, e);
        std::wstring why;
        PluginInfo now = p;
        for (auto const& g : record["granted"].items) {
            bool have = false;
            for (auto const& n : now.needs) have |= n.kind == Utf8ToWide(g.text);
            if (!have) now.needs.push_back({Utf8ToWide(g.text), L""});
        }
        bool ok = WriteBundled(now, temp, &why) && WriteAll(temp / L"tourne-plugin.json", Record(now));
        if (ok) {
            std::filesystem::remove_all(final, e);
            std::filesystem::rename(temp, final, e);
            ok = !e;
        }
        if (!ok) std::filesystem::remove_all(temp, e);
        if (ok) Log(L"%ls brought up to %ls", p.id.c_str(), p.version.c_str());
        else Log(L"%ls couldn't be brought up to %ls (%ls)", p.id.c_str(), p.version.c_str(), why.c_str());
    }
}

bool PluginRemove(const PluginInfo& p) {
    std::error_code e;
    std::filesystem::remove_all(PluginFolder(p), e);
    // What it saved, kept apart (plugin-runtime.cpp: Tourne\PluginData, or .data beside a test's plugins).
    std::wstring rel = PluginFolder(p).substr(PluginsRoot().size());
    std::error_code ignored;
    std::filesystem::remove_all(PluginsRoot() + (GetEnvironmentVariableW(L"TOURNE_PLUGINS_DIR", nullptr, 0) ? L"\\.data" : L"\\..\\PluginData") + rel, ignored);
    {
        std::lock_guard lock(g_lock);
        g_busy.erase(p.id), g_errors.erase(p.id);
    }
    if (e) Log(L"%s: couldn't remove it (%hs)", p.id.c_str(), e.message().c_str());
    else Log(L"removed %s", p.id.c_str());
    return !e;
}

std::vector<std::wstring> PluginNeedsList(const PluginInfo& p) {
    std::vector<std::wstring> out;
    for (auto const& n : p.needs) {
        if (n.kind == L"internet") out.push_back(n.detail.empty() ? L"Connects to the internet (a server you set in its settings)"
                                                                  : L"Connects to the internet: " + n.detail);
        else if (n.kind == L"program") out.push_back(L"Runs a program you install yourself: " + n.detail);
        else if (n.kind == L"open") out.push_back(L"Opens links and files in their usual apps");
        else if (n.kind == L"files") out.push_back(L"Saves files in its own folder (or a folder you choose in its settings)");
        else if (n.kind == L"clipboardRead") out.push_back(L"Reads the clipboard");
        else if (n.kind == L"clipboardWrite") out.push_back(L"Copies to the clipboard");
        else if (n.kind == L"notify") out.push_back(L"Shows notifications");
        else if (n.kind == L"systemInfo") out.push_back(L"Reads CPU, memory and disk use");
        else if (n.kind == L"wallpaper") out.push_back(L"Reads your wallpaper and may change it");
        else if (n.kind == L"sound") out.push_back(L"Plays sounds");
        else if (n.kind == L"processes") out.push_back(L"Ends programs you pick in it");
        else if (n.kind == L"power") out.push_back(L"Changes power mode, plan and turbo, and can lock, sleep or shut down");
        else if (n.kind == L"systemSound") out.push_back(L"Records the sound your PC plays, while you record with it");
        else if (n.kind == L"reads") {
            std::wstring where;
            for (auto const& path : n.paths) where += (where.empty() ? L"" : L", ") + path;
            out.push_back(L"Reads " + (n.detail.empty() ? std::wstring(L"another program's files") : n.detail) +
                          (where.empty() ? L"" : L" (" + where + L")") + L", without changing them");
        } else if (n.kind == L"deletes") {
            std::wstring where;
            for (auto const& path : n.paths) where += (where.empty() ? L"" : L", ") + path;
            out.push_back(L"Moves " + (n.detail.empty() ? std::wstring(L"files") : n.detail) + L" to the Recycle Bin when you ask it to" +
                          (where.empty() ? L"" : L" (" + where + L")"));
        } else if (n.kind == L"screen") out.push_back(L"Takes pictures of your screen when you pick a region, window or colour in it");
    }
    return out;
}

std::wstring PluginNeedsLine(const PluginInfo& p) {
    std::wstring line;
    auto add = [&](const std::wstring& s) { line += (line.empty() ? L"" : L" · ") + s; };
    for (auto const& n : p.needs) {
        if (n.kind == L"internet") {
            std::wstring first = n.detail.substr(0, n.detail.find(L','));
            size_t more = 0;
            for (wchar_t c : n.detail) more += c == L',';
            add(n.detail.empty() ? L"Internet" : L"Internet (" + first + (more ? L" +" + std::to_wstring(more) : L"") + L")");
        } else if (n.kind == L"program") add(n.detail);
        else if (n.kind == L"open") add(L"Opens links");
        else if (n.kind == L"files") add(L"Own files");
        else if (n.kind == L"clipboardRead") add(L"Reads clipboard");
        else if (n.kind == L"clipboardWrite") add(L"Clipboard");
        else if (n.kind == L"notify") add(L"Notifications");
        else if (n.kind == L"systemInfo") add(L"System stats");
        else if (n.kind == L"wallpaper") add(L"Wallpaper");
        else if (n.kind == L"sound") add(L"Sound");
        else if (n.kind == L"processes") add(L"Ends programs");
        else if (n.kind == L"power") add(L"Power");
        else if (n.kind == L"systemSound") add(L"Records PC sound");
        else if (n.kind == L"reads") add(L"Reads " + (n.detail.empty() ? std::wstring(L"other files") : n.detail));
        else if (n.kind == L"screen") add(L"Screen pictures");
        else if (n.kind == L"deletes") add(L"Deletes " + (n.detail.empty() ? std::wstring(L"files") : n.detail) + L" (Recycle Bin)");
    }
    return line.empty() ? L"Needs nothing" : L"Needs: " + line;
}

std::vector<std::wstring> PluginSurfaces(const PluginInfo& p) {
    std::vector<std::wstring> out;
    for (auto const& [entry, label] : std::initializer_list<std::pair<const wchar_t*, const wchar_t*>>{
             {L"widget", L"Bar"}, {L"panel", L"Panel"}, {L"desktop_widget", L"Desktop"}, {L"launcher_provider", L"Launcher"},
             {L"shortcut", L"Quick toggle"}, {L"service", L"Background"}})
        for (auto const& e : p.entries)
            if (e == entry) out.push_back(label);
    return out;
}

}  // namespace td
