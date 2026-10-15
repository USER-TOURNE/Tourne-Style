#include "plugin-runtime.h"
#include "ui-sounds.h"
#include "binds-editor.h"
#include "capture.h"
#include "keystrokes.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wincodec.h>
#include <windowsx.h>
#include <winhttp.h>
// (plugin-system.inl's)
#include <batclass.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <dxgi.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <powrprof.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <winternl.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

#include "config.h"
#include "json-lite.h"
#include "lua.h"
#include "luacode.h"
#include "lualib.h"
#include "plugin-panel.h"
#include "plugins.h"
#include "standalone.h"
#include "td.h"
#include "toml-lite.h"

namespace td {

namespace {

#include "plugin-bar-shared.cpp"

// The glyphs plugins name (Tabler icons, tools\plugins\Make-GlyphMap.ps1), and their font.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc23-extensions"
const char kGlyphMap[] = {
#embed "../plugins/tabler-glyphs.txt"
    , 0};
const unsigned char kGlyphFont[] = {
#embed "../plugins/tabler-icons.ttf"
};
#pragma clang diagnostic pop

constexpr size_t kMemoryLimit = 64 << 20;  // a plugin entry's VM, at most
constexpr ULONGLONG kCallLimit = 2000;     // ms one call into a plugin may run
constexpr int kMinInterval = 100;          // update() at most this often (ms)
constexpr UINT kMsgWake = WM_APP + 1;      // work queued for the runtime thread
constexpr UINT kMsgReload = WM_APP + 2;
constexpr UINT kMsgSettings = WM_APP + 3;  // plugins' settings changed

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"plugins", format, args);
    va_end(args);
}

std::wstring W(const std::string& s) { return Utf8ToWide(s); }
std::string U(const std::wstring& s) { return WideToUtf8(s); }

struct Plugin;

// One entry of a plugin ([[service]], [[widget]], [[panel]] or [[desktop_widget]]) in its own VM.
struct Entry {
    Plugin* plugin = nullptr;
    uint64_t serial = 0;  // (results arriving after a reload find no entry by it)
    std::string kind, id, file;
    json::Value settings;
    lua_State* L = nullptr;   // the VM
    lua_State* T = nullptr;   // the thread its script runs in, with globals of its own
    int threadRef = LUA_NOREF;
    int interval = 0;         // update() every this many ms
    bool paced = false;       // the script set its own pace (0: only when something asks)
    ULONGLONG due = 0;
    std::map<std::string, std::vector<int>> watchers;  // state key: functions to tell
    size_t memory = 0;
    ULONGLONG deadline = 0;
    int errors = 0;
    std::wstring lastError;
    // A bar widget's look: its glyph or picture (setImage: its file, the size asked for, the pixels the bar reads).
    std::wstring glyph, text, tip;
    std::string glyphName, colorRole;  // (as the plugin named them, for tourne.barItems)
    COLORREF color = CLR_INVALID, glyphColor = CLR_INVALID;
    bool visible = true;
    std::wstring imageSource, imageShown;
    int imageW = 0, imageH = 0;
    bool imageWatch = false;
    FILETIME imageWritten{};
    // onFrameTick (setNeedsFrameTick): an open panel's or a desktop widget's, at most 60 times a second.
    bool frameTicks = false;
    ULONGLONG lastFrame = 0;
    // require(): the modules it loaded (their full paths), and what each returned (a ref; LUA_NOREF while loading).
    std::map<std::wstring, int> modules;
    // A panel's (or desktop widget's): its manifest table, its window while open, and whether it wants update()
    // each second.
    json::Value manifest;
    std::unique_ptr<panel::Window> panel;
    bool wantsTicks = false;
    std::set<std::wstring> dropped;  // files the user dropped on (or pasted into) its panel: it may copy or open them
};

struct Plugin {
    std::wstring id, folder, dataDir;
    std::set<std::wstring> granted;
    std::vector<std::wstring> hosts;  // the internet hosts it may reach (empty: any, a server set in its settings)
    json::Value manifest, strings, english;
    std::map<std::string, json::Value> state;
    std::vector<std::unique_ptr<Entry>> entries;
    std::set<std::string> told;  // things said once in the log ("panels don't run yet")
};

// The runtime thread's own.
std::vector<std::unique_ptr<Plugin>> g_plugins;
std::map<uint64_t, Entry*> g_bySerial;
uint64_t g_nextSerial = 1;
std::deque<std::function<void()>> g_later;  // to run after the current call (state changes reaching watchers)
HWND g_window = nullptr;
UINT g_msgBarChanged = 0;
std::wstring g_lastPacked;          // the items as the bars last heard of them
std::vector<std::wstring> g_barKeys;  // (their keys, in order, for events by index)
unsigned g_barVersion = 1;           // which list that is
bool g_barDirty = false;

// Shared with other threads.
std::mutex g_lock;
std::deque<std::function<void()>> g_inbox;  // results from worker threads (HTTP), run on the runtime thread
std::vector<std::wstring> g_logLines;       // the last few things plugins logged, for PluginRuntimeReport
std::vector<std::wstring> g_reportLines;    // what each entry shows, rebuilt after each round
HANDLE g_thread = nullptr;
DWORD g_threadId = 0;

void Remember(const std::wstring& line) {
    std::lock_guard lock(g_lock);
    g_logLines.push_back(line);
    if (g_logLines.size() > 40) g_logLines.erase(g_logLines.begin());
}

// The launcher (launcher.cpp): providers, their last results, and who to tell when results come.
std::vector<LauncherProvider> g_launcherProviders;  // under g_lock
struct LauncherAnswer {
    std::wstring text;
    std::vector<LauncherItem> items;
};
std::map<std::wstring, LauncherAnswer> g_launcherAnswers;  // provider key: its last answer, under g_lock
HWND g_launcherWnd = nullptr;
UINT g_launcherMsg = 0;

void Post(std::function<void()> work) {
    {
        std::lock_guard lock(g_lock);
        g_inbox.push_back(std::move(work));
    }
    if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
}

Entry& E(lua_State* L) { return *(Entry*)lua_callbacks(L)->userdata; }

std::wstring Name(const Entry& e) { return e.plugin->id + L":" + W(e.id); }

void Problem(Entry& e, const std::wstring& what) {
    e.lastError = what;
    if (++e.errors <= 5) Log(L"%ls: %ls", Name(e).c_str(), what.c_str());
    if (e.errors == 5) Log(L"%ls: more errors aren't logged", Name(e).c_str());
    Remember(Name(e) + L" error: " + what);
}

void TellOnce(Entry& e, const std::string& what) {
    if (e.plugin->told.insert(what).second) {
        Log(L"%ls: %hs", Name(e).c_str(), what.c_str());
        Remember(Name(e) + L": " + W(what));
    }
}

bool Granted(const Entry& e, const wchar_t* kind) { return e.plugin->granted.count(kind) > 0; }

////////////////////////////////////////////////////////////////////////////////
// Values between Lua and JSON (noctalia.state, json, settings)

void PushJson(lua_State* L, const json::Value& v, int depth = 0) {
    lua_checkstack(L, 4);
    switch (v.kind) {
        case json::Value::Bool: lua_pushboolean(L, v.boolean); return;
        case json::Value::Number: lua_pushnumber(L, v.number); return;
        case json::Value::String: lua_pushlstring(L, v.text.data(), v.text.size()); return;
        case json::Value::Array:
            lua_createtable(L, (int)v.items.size(), 0);
            if (depth < 64)
                for (size_t i = 0; i < v.items.size(); i++) PushJson(L, v.items[i], depth + 1), lua_rawseti(L, -2, (int)i + 1);
            return;
        case json::Value::Object:
            lua_createtable(L, 0, (int)v.fields.size());
            if (depth < 64)
                for (auto const& [k, f] : v.fields) {
                    lua_pushlstring(L, k.data(), k.size());
                    PushJson(L, f, depth + 1);
                    lua_rawset(L, -3);
                }
            return;
        default: lua_pushnil(L);
    }
}

json::Value ToJson(lua_State* L, int idx, int depth = 0) {
    json::Value v;
    idx = lua_absindex(L, idx);
    switch (lua_type(L, idx)) {
        case LUA_TBOOLEAN: v.kind = json::Value::Bool, v.boolean = lua_toboolean(L, idx); break;
        case LUA_TNUMBER: v.kind = json::Value::Number, v.number = lua_tonumber(L, idx); break;
        case LUA_TSTRING: {
            size_t n = 0;
            const char* s = lua_tolstring(L, idx, &n);
            v.kind = json::Value::String, v.text.assign(s, n);
            break;
        }
        case LUA_TTABLE: {
            if (depth > 64) break;
            lua_checkstack(L, 4);
            int n = lua_objlen(L, idx);
            // A list when its keys are exactly 1..n.
            int keys = 0;
            bool list = true;
            lua_pushnil(L);
            while (lua_next(L, idx)) {
                keys++;
                if (lua_type(L, -2) != LUA_TNUMBER) list = false;
                lua_pop(L, 1);
            }
            if (list && keys == n && n > 0) {
                v.kind = json::Value::Array;
                for (int i = 1; i <= n; i++) {
                    lua_rawgeti(L, idx, i);
                    v.items.push_back(ToJson(L, -1, depth + 1));
                    lua_pop(L, 1);
                }
                break;
            }
            v.kind = json::Value::Object;
            lua_pushnil(L);
            while (lua_next(L, idx)) {
                std::string key;
                if (lua_type(L, -2) == LUA_TSTRING) key = lua_tostring(L, -2);
                else if (lua_type(L, -2) == LUA_TNUMBER) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%.17g", lua_tonumber(L, -2));
                    key = buf;
                }
                if (!key.empty()) v.fields.push_back({key, ToJson(L, -1, depth + 1)});
                lua_pop(L, 1);
            }
            break;
        }
        default: break;
    }
    return v;
}

void WriteJson(std::string& out, const json::Value& v, bool pretty, int depth) {
    auto line = [&](int d) {
        if (!pretty) return;
        out += '\n';
        out.append(d * 2, ' ');
    };
    switch (v.kind) {
        case json::Value::Bool: out += v.boolean ? "true" : "false"; return;
        case json::Value::Number: {
            char buf[32];
            if (v.number == (double)(long long)v.number && v.number > -1e15 && v.number < 1e15) snprintf(buf, sizeof(buf), "%lld", (long long)v.number);
            else snprintf(buf, sizeof(buf), "%.17g", v.number);
            out += buf;
            return;
        }
        case json::Value::String: out += json::Quote(v.text); return;
        case json::Value::Array:
            out += '[';
            for (size_t i = 0; i < v.items.size(); i++) {
                if (i) out += ',';
                line(depth + 1);
                WriteJson(out, v.items[i], pretty, depth + 1);
            }
            if (!v.items.empty()) line(depth);
            out += ']';
            return;
        case json::Value::Object:
            out += '{';
            for (size_t i = 0; i < v.fields.size(); i++) {
                if (i) out += ',';
                line(depth + 1);
                out += json::Quote(v.fields[i].first) + (pretty ? ": " : ":");
                WriteJson(out, v.fields[i].second, pretty, depth + 1);
            }
            if (!v.fields.empty()) line(depth);
            out += '}';
            return;
        default: out += "null";
    }
}

const json::Value* Lookup(const json::Value& root, const std::string& dotted) {
    const json::Value* at = &root;
    size_t start = 0;
    while (true) {
        size_t dot = dotted.find('.', start);
        std::string part = dotted.substr(start, dot - start);
        const json::Value* next = nullptr;
        for (auto const& [k, f] : at->fields)
            if (k == part) next = &f;
        if (!next) return nullptr;
        at = next;
        if (dot == std::string::npos) return at;
        start = dot + 1;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Glyphs

std::wstring GlyphChar(const std::string& name) {
    static const std::unordered_map<std::string, unsigned> map = [] {
        std::unordered_map<std::string, unsigned> m;
        const char* p = kGlyphMap;
        while (*p) {
            const char* end = strchr(p, '\n');
            std::string line(p, end ? end - p : strlen(p));
            p = end ? end + 1 : p + line.size();
            size_t space = line.find(' ');
            if (line.empty() || line[0] == '#' || space == std::string::npos) continue;
            m[line.substr(0, space)] = (unsigned)strtoul(line.c_str() + space + 1, nullptr, 16);
        }
        return m;
    }();
    auto it = map.find(name);
    if (it == map.end()) return L"";
    unsigned cp = it->second;
    if (cp < 0x10000) return std::wstring(1, (wchar_t)cp);
    cp -= 0x10000;
    return {(wchar_t)(0xD800 + (cp >> 10)), (wchar_t)(0xDC00 + (cp & 0x3FF))};
}

// The glyph font, written out once to %LOCALAPPDATA%\Tourne\Fonts and loaded for this process.
void LoadGlyphFont() {
    PWSTR local = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) return;
    std::wstring folder = std::wstring(local) + L"\\Tourne\\Fonts";
    CoTaskMemFree(local);
    std::wstring path = folder + L"\\tabler-icons.ttf";
    WIN32_FILE_ATTRIBUTE_DATA a{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &a) || a.nFileSizeLow != sizeof(kGlyphFont)) {
        SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr);
        HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
        if (f == INVALID_HANDLE_VALUE) return Log(L"couldn't write the glyph font to %ls", path.c_str());
        DWORD written = 0;
        WriteFile(f, kGlyphFont, sizeof(kGlyphFont), &written, nullptr);
        CloseHandle(f);
    }
    if (AddFontResourceExW(path.c_str(), FR_PRIVATE, nullptr) <= 0) Log(L"couldn't load the glyph font %ls", path.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Time and time zones (Windows' own ICU, icu.dll, for IANA zone names)

struct Icu {
    void* (*open)(const wchar_t*, int32_t, const char*, int, int*) = nullptr;
    void (*close)(void*) = nullptr;
    void (*setMillis)(void*, double, int*) = nullptr;
    int32_t (*get)(const void*, int, int*) = nullptr;
    int32_t (*canonical)(const wchar_t*, int32_t, wchar_t*, int32_t, int8_t*, int*) = nullptr;
    bool ok = false;
};

const Icu& TheIcu() {
    static const Icu icu = [] {
        Icu i;
        HMODULE m = LoadLibraryExW(L"icu.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!m) return i;
        i.open = (decltype(i.open))GetProcAddress(m, "ucal_open");
        i.close = (decltype(i.close))GetProcAddress(m, "ucal_close");
        i.setMillis = (decltype(i.setMillis))GetProcAddress(m, "ucal_setMillis");
        i.get = (decltype(i.get))GetProcAddress(m, "ucal_get");
        i.canonical = (decltype(i.canonical))GetProcAddress(m, "ucal_getCanonicalTimeZoneID");
        i.ok = i.open && i.close && i.setMillis && i.get && i.canonical;
        return i;
    }();
    return icu;
}

bool ValidZone(const std::string& zone) {
    const Icu& icu = TheIcu();
    if (!icu.ok || zone.empty() || zone.size() > 64) return false;
    std::wstring w = W(zone);
    wchar_t out[128];
    int8_t system = 0;
    int status = 0;
    icu.canonical(w.c_str(), (int32_t)w.size(), out, 128, &system, &status);
    return status <= 0 && system;
}

// Seconds east of UTC at unix time t: in that zone, or here when it's empty.
bool ZoneOffset(const std::string& zone, long long t, int* offset) {
    if (zone.empty()) {
        ULARGE_INTEGER u;
        u.QuadPart = (ULONGLONG)(t + 11644473600LL) * 10000000ULL;
        FILETIME ft{u.LowPart, u.HighPart};
        SYSTEMTIME utc, local;
        if (!FileTimeToSystemTime(&ft, &utc) || !SystemTimeToTzSpecificLocalTime(nullptr, &utc, &local)) return false;
        FILETIME lf;
        SystemTimeToFileTime(&local, &lf);
        ULARGE_INTEGER l{{lf.dwLowDateTime, lf.dwHighDateTime}};
        *offset = (int)(((long long)l.QuadPart - (long long)u.QuadPart) / 10000000LL);
        return true;
    }
    if (!ValidZone(zone)) return false;
    const Icu& icu = TheIcu();
    std::wstring w = W(zone);
    int status = 0;
    void* cal = icu.open(w.c_str(), (int32_t)w.size(), nullptr, 1 /* UCAL_GREGORIAN */, &status);
    if (!cal || status > 0) return false;
    icu.setMillis(cal, (double)t * 1000.0, &status);
    int zoneMs = icu.get(cal, 15 /* UCAL_ZONE_OFFSET */, &status);
    int dstMs = icu.get(cal, 16 /* UCAL_DST_OFFSET */, &status);
    icu.close(cal);
    if (status > 0) return false;
    *offset = (zoneMs + dstMs) / 1000;
    return true;
}

std::wstring DateText(const SYSTEMTIME& st, const wchar_t* format, DWORD flags = 0) {
    wchar_t buf[128];
    return GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &st, format, buf, 128, nullptr) ? buf : L"";
}
std::wstring TimeText(const SYSTEMTIME& st, const wchar_t* format, DWORD flags = 0) {
    wchar_t buf[128];
    return GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &st, format, buf, 128) ? buf : L"";
}

int IsoWeek(const SYSTEMTIME& st, int yday /* 1-based */) {
    auto weekday = [](int y, int m, int d) {  // Monday 1 ... Sunday 7
        static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
        if (m < 3) y--;
        int w = (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;  // 0 = Sunday
        return w ? w : 7;
    };
    auto weeks = [&](int y) {
        int jan1 = weekday(y, 1, 1);
        bool leap = (y % 4 == 0 && y % 100) || y % 400 == 0;
        return jan1 == 4 || (leap && jan1 == 3) ? 53 : 52;
    };
    int w = (yday - weekday(st.wYear, st.wMonth, st.wDay) + 10) / 7;
    if (w < 1) return weeks(st.wYear - 1);
    if (w > weeks(st.wYear)) return 1;
    return w;
}

// strftime's codes (and std::format's "{:%H:%M}" around them), without the C library's,
// which ends the program on a code it doesn't know.
std::string FormatTime(const std::string& pattern, long long t, const std::string& zone) {
    int offset = 0;
    if (!ZoneOffset(zone, t, &offset)) ZoneOffset("", t, &offset);
    ULARGE_INTEGER u;
    u.QuadPart = (ULONGLONG)(t + offset + 11644473600LL) * 10000000ULL;
    FILETIME ft{u.LowPart, u.HighPart};
    SYSTEMTIME st{};
    FileTimeToSystemTime(&ft, &st);
    static const int before[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    bool leap = (st.wYear % 4 == 0 && st.wYear % 100) || st.wYear % 400 == 0;
    int yday = before[st.wMonth - 1] + st.wDay + (leap && st.wMonth > 2 ? 1 : 0);
    int hour12 = st.wHour % 12 ? st.wHour % 12 : 12;
    auto two = [](int n) {
        char b[8];
        snprintf(b, sizeof(b), "%02d", n);
        return std::string(b);
    };
    std::function<std::string(const std::string&)> expand = [&](const std::string& f) {
        std::string out;
        for (size_t i = 0; i < f.size(); i++) {
            if (f[i] != '%' || i + 1 >= f.size()) {
                out += f[i];
                continue;
            }
            char c = f[++i];
            if (c == 'E' || c == 'O') {  // (std::format's alternative forms: as the plain ones)
                if (i + 1 >= f.size()) break;
                c = f[++i];
            }
            switch (c) {
                case 'a': out += U(DateText(st, L"ddd")); break;
                case 'A': out += U(DateText(st, L"dddd")); break;
                case 'b':
                case 'h': out += U(DateText(st, L"MMM")); break;
                case 'B': out += U(DateText(st, L"MMMM")); break;
                case 'c': out += U(DateText(st, nullptr, DATE_SHORTDATE)) + " " + U(TimeText(st, nullptr)); break;
                case 'C': out += two(st.wYear / 100); break;
                case 'd': out += two(st.wDay); break;
                case 'e': out += (st.wDay < 10 ? " " : "") + std::to_string(st.wDay); break;
                case 'D': out += expand("%m/%d/%y"); break;
                case 'F': out += expand("%Y-%m-%d"); break;
                case 'H': out += two(st.wHour); break;
                case 'I': out += two(hour12); break;
                case 'j': {
                    char b[8];
                    snprintf(b, sizeof(b), "%03d", yday);
                    out += b;
                    break;
                }
                case 'm': out += two(st.wMonth); break;
                case 'M': out += two(st.wMinute); break;
                case 'n': out += '\n'; break;
                case 'p': out += st.wHour < 12 ? "AM" : "PM"; break;
                case 'r': out += expand("%I:%M:%S %p"); break;
                case 'R': out += expand("%H:%M"); break;
                case 'S': out += two(st.wSecond); break;
                case 's': out += std::to_string(t); break;  // (seconds since 1970, as GNU's strftime has it)
                case 't': out += '\t'; break;
                case 'T': out += expand("%H:%M:%S"); break;
                case 'u': out += std::to_string(st.wDayOfWeek ? st.wDayOfWeek : 7); break;
                case 'w': out += std::to_string(st.wDayOfWeek); break;
                case 'V': out += two(IsoWeek(st, yday)); break;
                case 'x': out += U(DateText(st, nullptr, DATE_SHORTDATE)); break;
                case 'X': out += U(TimeText(st, nullptr)); break;
                case 'y': out += two(st.wYear % 100); break;
                case 'Y': out += std::to_string(st.wYear); break;
                case 'z':
                case 'Z': {
                    if (c == 'Z' && !zone.empty()) {
                        out += zone;
                        break;
                    }
                    int a = offset < 0 ? -offset : offset;
                    char b[16];
                    snprintf(b, sizeof(b), "%c%02d%02d", offset < 0 ? '-' : '+', a / 3600, a / 60 % 60);
                    out += b;
                    break;
                }
                case '%': out += '%'; break;
                default: out += '%', out += c;
            }
        }
        return out;
    };
    // "{:%H:%M}": each {:...} is a pattern; text around them is kept.
    if (pattern.find("{:") == std::string::npos) return expand(pattern);
    std::string out;
    size_t at = 0;
    while (at < pattern.size()) {
        size_t open = pattern.find("{:", at);
        if (open == std::string::npos) {
            out += pattern.substr(at);
            break;
        }
        size_t close = pattern.find('}', open);
        if (close == std::string::npos) {
            out += pattern.substr(at);
            break;
        }
        out += pattern.substr(at, open - at) + expand(pattern.substr(open + 2, close - open - 2));
        at = close + 1;
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Files: a plugin reads its own folder, its data folder and the folders its "reads" need names, and writes
// only the data folder (and folders its settings name)

std::wstring Full(const std::wstring& path) {
    wchar_t buf[MAX_PATH * 2];
    DWORD n = GetFullPathNameW(path.c_str(), MAX_PATH * 2, buf, nullptr);
    return n && n < MAX_PATH * 2 ? std::wstring(buf, n) : L"";
}

bool Inside(const std::wstring& path, const std::wstring& folder) {
    if (folder.empty() || path.size() < folder.size()) return false;
    if (CompareStringOrdinal(path.c_str(), (int)folder.size(), folder.c_str(), (int)folder.size(), TRUE) != CSTR_EQUAL) return false;
    return path.size() == folder.size() || path[folder.size()] == L'\\';
}

// A path inside a folder a plugin's need names. One ending in "*" names everything beside it whose name starts so
// ("%USERPROFILE%\.claude*": .claude, .claude.json, .claude-work), and what's inside those.
bool InsideNeed(const std::wstring& path, const std::wstring& folder) {
    if (folder.empty() || folder.back() != L'*') return Inside(path, folder);
    std::wstring stem = folder.substr(0, folder.size() - 1);
    size_t slash = stem.rfind(L'\\');
    if (slash == std::wstring::npos || slash < 3) return false;
    std::wstring parent = stem.substr(0, slash), prefix = stem.substr(slash + 1);
    if (prefix.empty() || !Inside(path, parent) || path.size() < parent.size() + 1 + prefix.size()) return false;
    return CompareStringOrdinal(path.c_str() + parent.size() + 1, (int)prefix.size(), prefix.c_str(), (int)prefix.size(), TRUE) == CSTR_EQUAL;
}

std::wstring Full(const std::wstring& path);
bool Inside(const std::wstring& path, const std::wstring& folder);

// "~/Documents/Notes", "$HOME/x", "%APPDATA%\x": made a Windows path.
std::wstring ExpandPath(const std::string& given) {
    std::wstring p = W(given);
    wchar_t home[MAX_PATH] = L"";
    GetEnvironmentVariableW(L"USERPROFILE", home, MAX_PATH);
    if (p == L"~" || p.rfind(L"~/", 0) == 0 || p.rfind(L"~\\", 0) == 0) p = home + p.substr(1);
    else if (p.rfind(L"$HOME", 0) == 0) p = home + p.substr(5);
    wchar_t expanded[MAX_PATH * 2];
    if (ExpandEnvironmentStringsW(p.c_str(), expanded, MAX_PATH * 2)) p = expanded;
    for (auto& c : p)
        if (c == L'/') c = L'\\';
    return p;
}

// A folder a "folder" setting may hand a plugin: one of its own, like Documents\Notes, never your
// whole profile (or anything above it), other programs' data (AppData), Windows or Program Files,
// whatever the plugin's manifest gives as the default.
bool UsableFolder(const std::wstring& f) {
    if (f.size() <= 3) return false;  // (a whole drive)
    auto env = [](const wchar_t* name) {
        wchar_t v[MAX_PATH] = L"";
        return GetEnvironmentVariableW(name, v, MAX_PATH) ? Full(v) : std::wstring();
    };
    std::wstring profile = env(L"USERPROFILE");
    if (!profile.empty() && (Inside(profile, f) || Inside(f, profile + L"\\AppData"))) return false;
    for (const wchar_t* system : {L"WINDIR", L"ProgramFiles", L"ProgramFiles(x86)", L"ProgramData"})
        if (std::wstring s = env(system); !s.empty() && (Inside(f, s) || Inside(s, f))) return false;
    return true;
}

// The folders the plugin's settings of type "folder" name now (the user chose them, or kept the default).
std::vector<std::wstring> SettingFolders(Entry& e) {
    std::vector<std::wstring> out;
    auto add = [&](const json::Value& list) {
        for (auto const& s : list.items) {
            if (s["type"].text != "folder") continue;
            for (auto const& [k, v] : e.settings.fields)
                if (k == s["key"].text && v.kind == json::Value::String && !v.text.empty()) {
                    std::wstring f = Full(ExpandPath(v.text));
                    if (UsableFolder(f)) out.push_back(f);
                }
        }
    };
    add(e.plugin->manifest["setting"]);
    for (const char* kind : {"widget", "panel", "service", "launcher_provider", "desktop_widget", "shortcut"})
        for (auto const& entry : e.plugin->manifest[kind].items) add(entry["setting"]);
    return out;
}

std::wstring WallpaperFolderNow();  // (plugin-system.inl)
bool ShowingWallpaper(const std::wstring& full);  // (plugin-system.inl)
bool Granted(const Entry& e, const wchar_t* kind);

// The folders of other programs a plugin may read (its "reads" need, from the index: "~/.claude",
// "%LOCALAPPDATA%\Syncthing"), when it was allowed them; or (kind "deletes") move to the Recycle Bin.
std::vector<std::wstring> ReadsFolders(const Entry& e, const wchar_t* kind = L"reads") {
    std::vector<std::wstring> out;
    if (!Granted(e, kind)) return out;
    for (auto const& info : PluginCatalogue())
        if (info.id == e.plugin->id)
            for (auto const& n : info.needs)
                if (n.kind == kind)
                    for (auto const& path : n.paths)
                        if (std::wstring f = Full(ExpandPath(U(path))); f.size() > 3) out.push_back(f);
    return out;
}

// The path a plugin named, made whole (relative ones are in its own folder), if it may use it.
// A plugin allowed the wallpaper also reads the wallpaper folder.
bool Resolve(Entry& e, const std::string& given, bool write, std::wstring* out, std::string* why) {
    std::wstring p = W(given);
    for (auto& c : p)
        if (c == L'/') c = L'\\';
    bool absolute = p.size() > 2 && (p[1] == L':' || (p[0] == L'\\' && p[1] == L'\\'));
    std::wstring full = Full(absolute ? p : e.plugin->folder + L"\\" + p);
    if (full.empty() || full.find(L"\\\\?\\") == 0 || full.find(L':', 2) != std::wstring::npos) {
        *why = "not a usable path";
        return false;
    }
    bool data = Inside(full, e.plugin->dataDir);
    for (auto const& folder : SettingFolders(e))
        if (Inside(full, folder)) data = true;
    bool own = data || Inside(full, e.plugin->folder);
    bool reads = false;
    if (!write && !own)
        for (auto const& folder : ReadsFolders(e)) reads = reads || InsideNeed(full, folder);
    bool wallpapers = !write && !own && !reads && Granted(e, L"wallpaper") &&
                      ((!WallpaperFolderNow().empty() && Inside(full, WallpaperFolderNow())) || ShowingWallpaper(full));
    if (write ? !data : !(data || wallpapers || reads || Inside(full, e.plugin->folder))) {
        *why = write ? "a plugin writes only in its data folder (noctalia.pluginDataDir()) or a folder its settings name"
                     : "outside the plugin's own folders";
        return false;
    }
    if (write && !Granted(e, L"files")) {
        *why = "it wasn't allowed to save files when installed";
        return false;
    }
    *out = full;
    return true;
}

bool ReadAll(const std::wstring& path, std::string* out, size_t limit = 16 << 20) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    bool ok = GetFileSizeEx(f, &size) && (size_t)size.QuadPart <= limit;
    if (ok) {
        out->resize((size_t)size.QuadPart);
        DWORD got = 0;
        ok = out->empty() || (ReadFile(f, out->data(), (DWORD)out->size(), &got, nullptr) && got == out->size());
    }
    CloseHandle(f);
    return ok;
}

bool WriteAll(const std::wstring& path, const std::string& bytes) {
    std::wstring temp = path + L".writing";
    HANDLE f = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    bool ok = bytes.empty() || (WriteFile(f, bytes.data(), (DWORD)bytes.size(), &written, nullptr) && written == bytes.size());
    CloseHandle(f);
    ok = ok && MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
    if (!ok) DeleteFileW(temp.c_str());
    return ok;
}

////////////////////////////////////////////////////////////////////////////////
// Calling into a plugin

void Interrupt(lua_State* L, int gc) {
    if (gc >= 0) return;  // (never stop a script in the middle of collecting)
    Entry& e = E(L);
    if (e.deadline && GetTickCount64() > e.deadline) {
        e.deadline = 0;
        luaL_error(L, "ran longer than %d seconds", (int)(kCallLimit / 1000));
    }
}

// The function and its arguments are on e.T.
bool Call(Entry& e, int args) {
    if (!e.T) return false;
    e.deadline = GetTickCount64() + kCallLimit;
    int status = lua_pcall(e.T, args, 0, 0);
    e.deadline = 0;
    if (status == LUA_OK) return true;
    const char* why = lua_tostring(e.T, -1);
    Problem(e, W(why ? why : status == LUA_YIELD ? "yielded where it can't" : "failed"));
    lua_settop(e.T, 0);
    return false;
}

// Work on an entry's VM outside a protected call (pushing a callback and its
// arguments): Luau raises an error there (out of its memory) as a C++
// exception, which must stop at the entry, not end the program.
bool Guarded(Entry& e, const std::function<bool()>& work) {
    if (!e.T) return false;
    try {
        return work();
    } catch (...) {
        Problem(e, L"ran out of its memory (" + std::to_wstring(kMemoryLimit >> 20) + L" MB)");
        e.deadline = 0;
        try {
            lua_settop(e.T, 0);
        } catch (...) {
        }
        return false;
    }
}

// A callback the plugin defines (update, onClick...), if it does.
bool CallGlobal(Entry& e, const char* name, std::function<int(lua_State*)> args = nullptr) {
    return Guarded(e, [&] {
        lua_settop(e.T, 0);
        lua_getglobal(e.T, name);
        if (!lua_isfunction(e.T, -1)) {
            lua_settop(e.T, 0);
            return false;
        }
        return Call(e, args ? args(e.T) : 0);
    });
}

void Later(std::function<void()> work) { g_later.push_back(std::move(work)); }

void BarChanged() { g_barDirty = true; }

////////////////////////////////////////////////////////////////////////////////
// The API: noctalia.*

std::string Arg(lua_State* L, int i) {
    size_t n = 0;
    const char* s = luaL_checklstring(L, i, &n);
    return std::string(s, n);
}
std::string OptArg(lua_State* L, int i) {
    if (lua_isnoneornil(L, i)) return {};
    size_t n = 0;
    const char* s = lua_tolstring(L, i, &n);
    return s ? std::string(s, n) : std::string();
}
int Fail(lua_State* L, const std::string& why) {
    lua_pushnil(L);
    lua_pushlstring(L, why.data(), why.size());
    return 2;
}
int FailBool(lua_State* L, const std::string& why) {
    lua_pushboolean(L, false);
    lua_pushlstring(L, why.data(), why.size());
    return 2;
}

int ApiLog(lua_State* L) {
    Entry& e = E(L);
    std::string text;
    for (int i = 1, n = lua_gettop(L); i <= n; i++) {
        size_t len = 0;
        const char* s = luaL_tolstring(L, i, &len);
        text += (i > 1 ? " " : "") + std::string(s, len);
        lua_pop(L, 1);
    }
    Log(L"%ls: %ls", Name(e).c_str(), W(text).c_str());
    Remember(Name(e) + L": " + W(text));
    return 0;
}

int ApiGetConfig(lua_State* L) {
    Entry& e = E(L);
    std::string key = Arg(L, 1);
    for (auto const& [k, v] : e.settings.fields)
        if (k == key) return PushJson(L, v), 1;
    lua_pushnil(L);
    return 1;
}

int ApiSetUpdateInterval(lua_State* L) {
    Entry& e = E(L);
    // 0 (or less) stops update() until the script asks again: nothing wakes for it meanwhile.
    int ms = (int)luaL_checknumber(L, 1);
    e.interval = ms > 0 ? std::max(kMinInterval, ms) : 0;
    e.paced = true;
    e.due = GetTickCount64() + e.interval;
    return 0;
}

int ApiStateSet(lua_State* L) {
    Entry& e = E(L);
    std::string key = Arg(L, 1);
    json::Value value = ToJson(L, 2);
    Plugin* p = e.plugin;
    p->state[key] = value;
    for (auto& other : p->entries) {
        auto it = other->watchers.find(key);
        if (it == other->watchers.end()) continue;
        for (int ref : it->second) {
            uint64_t serial = other->serial;
            Later([serial, ref, value] {
                auto found = g_bySerial.find(serial);
                if (found == g_bySerial.end()) return;
                Entry& to = *found->second;
                Guarded(to, [&] {
                    lua_settop(to.T, 0);
                    lua_getref(to.T, ref);
                    PushJson(to.T, value);
                    return Call(to, 1);
                });
            });
        }
    }
    return 0;
}

int ApiStateGet(lua_State* L) {
    Entry& e = E(L);
    auto it = e.plugin->state.find(Arg(L, 1));
    if (it == e.plugin->state.end()) lua_pushnil(L);
    else PushJson(L, it->second);
    return 1;
}

int ApiStateWatch(lua_State* L) {
    Entry& e = E(L);
    std::string key = Arg(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    e.watchers[key].push_back(lua_ref(L, 2));
    return 0;
}

int ApiJsonEncode(lua_State* L) {
    std::string out;
    WriteJson(out, ToJson(L, 1), lua_toboolean(L, 2), 0);
    lua_pushlstring(L, out.data(), out.size());
    return 1;
}

int ApiJsonDecode(lua_State* L) {
    json::Value v;
    if (!json::Parse(Arg(L, 1), &v)) return Fail(L, "not JSON");
    PushJson(L, v);
    return 1;
}

int ApiTrim(lua_State* L) {
    std::string s = Arg(L, 1);
    size_t a = s.find_first_not_of(" \t\r\n\v\f"), b = s.find_last_not_of(" \t\r\n\v\f");
    s = a == std::string::npos ? "" : s.substr(a, b - a + 1);
    lua_pushlstring(L, s.data(), s.size());
    return 1;
}

int ApiUrlEncode(lua_State* L) {
    std::string out;
    for (unsigned char c : Arg(L, 1)) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out += (char)c;
        else {
            char b[4];
            snprintf(b, sizeof(b), "%%%02X", c);
            out += b;
        }
    }
    lua_pushlstring(L, out.data(), out.size());
    return 1;
}

int ApiUrlDecode(lua_State* L) {
    std::string s = Arg(L, 1), out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size() && isxdigit((unsigned char)s[i + 1]) && isxdigit((unsigned char)s[i + 2])) {
            out += (char)strtoul(s.substr(i + 1, 2).c_str(), nullptr, 16);
            i += 2;
        } else out += s[i] == '+' ? ' ' : s[i];
    }
    lua_pushlstring(L, out.data(), out.size());
    return 1;
}

// "{name}" in a translation, from the table at idx.
std::string Substitute(lua_State* L, std::string text, int idx) {
    if (!lua_istable(L, idx)) return text;
    idx = lua_absindex(L, idx);
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        if (lua_type(L, -2) == LUA_TSTRING) {
            std::string key = "{" + std::string(lua_tostring(L, -2)) + "}";
            size_t len = 0;
            const char* v = luaL_tolstring(L, -1, &len);
            std::string value(v, len);
            lua_pop(L, 1);
            for (size_t at = 0; (at = text.find(key, at)) != std::string::npos; at += value.size()) text.replace(at, key.size(), value);
        }
        lua_pop(L, 1);
    }
    return text;
}

const json::Value* Translation(Entry& e, const std::string& key) {
    if (auto v = Lookup(e.plugin->strings, key)) return v;
    return Lookup(e.plugin->english, key);
}

int ApiTr(lua_State* L) {
    Entry& e = E(L);
    std::string key = Arg(L, 1);
    const json::Value* v = Translation(e, key);
    std::string text = v && v->kind == json::Value::String ? Substitute(L, v->text, 2) : key;
    lua_pushlstring(L, text.data(), text.size());
    return 1;
}

int ApiTrp(lua_State* L) {
    Entry& e = E(L);
    std::string key = Arg(L, 1);
    double count = luaL_checknumber(L, 2);
    const json::Value* v = Translation(e, key);
    std::string text = key;
    if (v && v->kind == json::Value::Object) {
        const json::Value& form = (*v)[count == 1 ? "one" : "other"];
        const json::Value& chosen = form.kind == json::Value::String ? form : (*v)["other"];
        if (chosen.kind == json::Value::String) text = chosen.text;
    } else if (v && v->kind == json::Value::String) text = v->text;
    char n[32];
    snprintf(n, sizeof(n), "%.15g", count);
    for (size_t at = 0; (at = text.find("{count}", at)) != std::string::npos; at += strlen(n)) text.replace(at, 7, n);
    text = Substitute(L, text, 3);
    lua_pushlstring(L, text.data(), text.size());
    return 1;
}

int ApiReadFile(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why, body;
    if (!Resolve(e, Arg(L, 1), false, &path, &why)) return Fail(L, why);
    if (!ReadAll(path, &body)) return Fail(L, "couldn't read it");
    lua_pushlstring(L, body.data(), body.size());
    return 1;
}

int ApiWriteFile(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why;
    if (!Resolve(e, Arg(L, 1), true, &path, &why)) return FailBool(L, why);
    if (!WriteAll(path, Arg(L, 2))) return FailBool(L, "couldn't write it");
    lua_pushboolean(L, true);
    return 1;
}

int ApiMkdirAll(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why;
    if (!Resolve(e, Arg(L, 1), true, &path, &why)) return FailBool(L, why);
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) return FailBool(L, ec.message());
    lua_pushboolean(L, true);
    return 1;
}

int ApiRemoveFile(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why;
    if (!Resolve(e, Arg(L, 1), true, &path, &why)) return FailBool(L, why);
    if (!DeleteFileW(path.c_str())) return FailBool(L, "couldn't remove it");
    lua_pushboolean(L, true);
    return 1;
}

int ApiRenameFile(lua_State* L) {
    Entry& e = E(L);
    std::wstring from, to;
    std::string why;
    if (!Resolve(e, Arg(L, 1), true, &from, &why) || !Resolve(e, Arg(L, 2), true, &to, &why)) return FailBool(L, why);
    if (!MoveFileExW(from.c_str(), to.c_str(), MOVEFILE_REPLACE_EXISTING)) return FailBool(L, "couldn't rename it");
    lua_pushboolean(L, true);
    return 1;
}

int ApiFileExists(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why;
    lua_pushboolean(L, Resolve(e, Arg(L, 1), false, &path, &why) && GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES);
    return 1;
}

int ApiFileInfo(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why;
    if (!Resolve(e, Arg(L, 1), false, &path, &why)) return Fail(L, why);
    WIN32_FILE_ATTRIBUTE_DATA a{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &a)) return Fail(L, "not found");
    ULARGE_INTEGER t{{a.ftLastWriteTime.dwLowDateTime, a.ftLastWriteTime.dwHighDateTime}};
    ULARGE_INTEGER born{{a.ftCreationTime.dwLowDateTime, a.ftCreationTime.dwHighDateTime}};
    lua_createtable(L, 0, 4);
    lua_pushnumber(L, (double)(((ULONGLONG)a.nFileSizeHigh << 32) | a.nFileSizeLow));
    lua_setfield(L, -2, "size");
    lua_pushnumber(L, (double)((long long)(t.QuadPart / 10000000ULL) - 11644473600LL));
    lua_setfield(L, -2, "mtime");
    lua_pushnumber(L, (double)((long long)(born.QuadPart / 10000000ULL) - 11644473600LL));  // (Tourne's: when it was made)
    lua_setfield(L, -2, "created");
    lua_pushboolean(L, (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    lua_setfield(L, -2, "isDir");
    return 1;
}

int ApiListDir(lua_State* L) {
    Entry& e = E(L);
    std::wstring path;
    std::string why;
    if (!Resolve(e, Arg(L, 1), false, &path, &why)) return Fail(L, why);
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((path + L"\\*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return Fail(L, "not a folder");
    lua_newtable(L);
    int i = 1;
    do {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..")) continue;
        std::string name = U(fd.cFileName);
        lua_pushlstring(L, name.data(), name.size());
        lua_rawseti(L, -2, i++);
    } while (FindNextFileW(find, &fd));
    FindClose(find);
    return 1;
}

int ApiPluginDir(lua_State* L) {
    std::string s = U(E(L).plugin->folder);
    lua_pushlstring(L, s.data(), s.size());
    return 1;
}

int ApiPluginDataDir(lua_State* L) {
    Entry& e = E(L);
    std::error_code ec;
    std::filesystem::create_directories(e.plugin->dataDir, ec);
    if (ec) return Fail(L, ec.message());
    std::string s = U(e.plugin->dataDir);
    lua_pushlstring(L, s.data(), s.size());
    return 1;
}

int ApiFormatTime(lua_State* L) {
    long long t = lua_isnoneornil(L, 2) ? (long long)time(nullptr) : (long long)luaL_checknumber(L, 2);
    std::string s = FormatTime(Arg(L, 1), t, OptArg(L, 3));
    lua_pushlstring(L, s.data(), s.size());
    return 1;
}

int ApiTimeFormat(lua_State* L) {
    wchar_t f[80] = L"";
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SSHORTTIME, f, 80);
    lua_pushstring(L, wcschr(f, L'H') ? "{:%H:%M}" : "{:%I:%M %p}");
    return 1;
}

int ApiDateFormat(lua_State* L) {
    lua_pushstring(L, "%A, %x");
    return 1;
}

int ApiIsValidTimezone(lua_State* L) {
    lua_pushboolean(L, ValidZone(Arg(L, 1)));
    return 1;
}

int ApiNowMs(lua_State* L) {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    ULARGE_INTEGER u{{ft.dwLowDateTime, ft.dwHighDateTime}};
    lua_pushnumber(L, (double)(u.QuadPart / 10000ULL) - 11644473600000.0);
    return 1;
}

// A notification: a balloon from a tray icon of the runtime's own, which Windows shows as a notification.
void Notify(Entry& e, const std::string& title, const std::string& body, bool error) {
    if (!Granted(e, L"notify")) return TellOnce(e, "wasn't allowed to show notifications when installed");
    NOTIFYICONDATAW n{sizeof(n)};
    n.hWnd = g_window;
    n.uID = 1;
    n.uFlags = NIF_ICON | NIF_TIP | NIF_INFO;
    n.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wcsncpy(n.szTip, (e.plugin->id).c_str(), ARRAYSIZE(n.szTip) - 1);
    wcsncpy(n.szInfoTitle, W(title).c_str(), ARRAYSIZE(n.szInfoTitle) - 1);
    wcsncpy(n.szInfo, W(body.empty() ? " " : body).c_str(), ARRAYSIZE(n.szInfo) - 1);
    n.dwInfoFlags = error ? NIIF_ERROR : NIIF_INFO;
    if (!Shell_NotifyIconW(NIM_MODIFY, &n)) Shell_NotifyIconW(NIM_ADD, &n);
    SetTimer(g_window, 1, 15000, nullptr);  // the icon goes again
    Remember(Name(e) + L" notified: " + W(title) + L" / " + W(body));
}

int ApiNotify(lua_State* L) {
    Notify(E(L), Arg(L, 1), OptArg(L, 2), false);
    return 0;
}
int ApiNotifyError(lua_State* L) {
    Notify(E(L), Arg(L, 1), OptArg(L, 2), true);
    return 0;
}

std::wstring DroppedPath(Entry& e, const std::string& given);

// A "file://" address (or a plain path) as a Windows path: "file:///C:/a%20b.png" and "file://C:\a b.png" alike.
std::wstring FileFromUri(std::string uri) {
    if (uri.rfind("file://", 0) == 0) {
        uri = uri.substr(7);
        if (uri.rfind("localhost/", 0) == 0) uri = uri.substr(9);
        if (uri.size() > 2 && uri[0] == '/' && isalpha((unsigned char)uri[1]) && uri[2] == ':') uri = uri.substr(1);
        std::string plain;
        for (size_t i = 0; i < uri.size(); i++)
            if (uri[i] == '%' && i + 2 < uri.size() && isxdigit((unsigned char)uri[i + 1]) && isxdigit((unsigned char)uri[i + 2]))
                plain += (char)strtol(uri.substr(i + 1, 2).c_str(), nullptr, 16), i += 2;
            else plain += uri[i];
        uri = plain;
    }
    std::wstring path = W(uri);
    for (auto& c : path)
        if (c == L'/') c = L'\\';
    return path;
}

// noctalia.copyToClipboard(text, mimeType): text; with "text/uri-list", the files it names (one "file://" address
// a line, each one the plugin may read) are copied as files, as Explorer copies them, so they paste into a folder,
// a chat or a mail.
int ApiCopyToClipboard(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"clipboardWrite")) return FailBool(L, "it wasn't allowed to copy to the clipboard when installed");
    std::string given = Arg(L, 1), mime = OptArg(L, 2);
    std::wstring text = W(given);
    std::vector<std::wstring> files;
    if (mime.rfind("text/uri-list", 0) == 0) {
        text.clear();
        for (size_t at = 0; at < given.size();) {
            size_t end = given.find_first_of("\r\n", at);
            if (end == std::string::npos) end = given.size();
            std::string line = given.substr(at, end - at);
            at = end + 1;
            if (line.empty() || line[0] == '#') continue;
            std::wstring path = FileFromUri(line), full = DroppedPath(e, U(path));
            std::string why;
            if (full.empty() && !Resolve(e, U(path), false, &full, &why)) return FailBool(L, why);
            files.push_back(full);
            text += (text.empty() ? L"" : L"\r\n") + full;
        }
        if (files.empty()) return FailBool(L, "no files named");
    }
    bool ok = false;
    if (OpenClipboard(g_window)) {
        EmptyClipboard();
        if (HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t))) {
            memcpy(GlobalLock(h), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
            GlobalUnlock(h);
            ok = SetClipboardData(CF_UNICODETEXT, h) != nullptr;
            if (!ok) GlobalFree(h);
        }
        if (!files.empty()) {
            // DROPFILES, then each path and a final empty one.
            size_t chars = 1;
            for (auto const& f : files) chars += f.size() + 1;
            if (HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DROPFILES) + chars * sizeof(wchar_t))) {
                auto* drop = (DROPFILES*)GlobalLock(h);
                drop->pFiles = sizeof(DROPFILES);
                drop->fWide = TRUE;
                auto* at = (wchar_t*)(drop + 1);
                for (auto const& f : files) memcpy(at, f.c_str(), (f.size() + 1) * sizeof(wchar_t)), at += f.size() + 1;
                GlobalUnlock(h);
                bool set = SetClipboardData(CF_HDROP, h) != nullptr;
                if (!set) GlobalFree(h);
                ok = ok && set;
            }
            static const UINT effect = RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT);
            if (HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD))) {
                *(DWORD*)GlobalLock(h) = DROPEFFECT_COPY;
                GlobalUnlock(h);
                if (!SetClipboardData(effect, h)) GlobalFree(h);
            }
        }
        CloseClipboard();
    }
    lua_pushboolean(L, ok);
    return 1;
}

int ApiClipboardText(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"clipboardRead")) {
        TellOnce(e, "wasn't allowed to read the clipboard when installed");
        lua_pushnil(L);
        return 1;
    }
    std::string text;
    if (OpenClipboard(g_window)) {
        if (HANDLE h = GetClipboardData(CF_UNICODETEXT))
            if (auto* w = (const wchar_t*)GlobalLock(h)) {
                text = U(w);
                GlobalUnlock(h);
            }
        CloseClipboard();
    }
    if (text.empty()) lua_pushnil(L);
    else lua_pushlstring(L, text.data(), text.size());
    return 1;
}

// PNG bytes for 32-bit BGRA pixels (top-down, straight alpha).
bool EncodePng(const std::vector<uint32_t>& pixels, UINT w, UINT h, std::string* out) {
    IWICImagingFactory* f = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) return false;
    IStream* stream = nullptr;
    IWICBitmapEncoder* encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    bool ok = SUCCEEDED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)) && SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)) &&
              SUCCEEDED(encoder->Initialize(stream, WICBitmapEncoderNoCache)) && SUCCEEDED(encoder->CreateNewFrame(&frame, nullptr)) &&
              SUCCEEDED(frame->Initialize(nullptr)) && SUCCEEDED(frame->SetSize(w, h)) && SUCCEEDED(frame->SetPixelFormat(&format)) &&
              IsEqualGUID(format, GUID_WICPixelFormat32bppBGRA) &&
              SUCCEEDED(frame->WritePixels(h, w * 4, w * h * 4, (BYTE*)pixels.data())) && SUCCEEDED(frame->Commit()) &&
              SUCCEEDED(encoder->Commit());
    STATSTG st{};
    HGLOBAL global = nullptr;
    if (ok) ok = SUCCEEDED(stream->Stat(&st, STATFLAG_NONAME)) && SUCCEEDED(GetHGlobalFromStream(stream, &global));
    if (ok) {
        if (void* bytes = GlobalLock(global)) {
            out->assign((const char*)bytes, (size_t)st.cbSize.QuadPart);
            GlobalUnlock(global);
        } else {
            ok = false;
        }
    }
    for (IUnknown* u : std::initializer_list<IUnknown*>{frame, encoder, stream, f})
        if (u) u->Release();
    return ok;
}

// A DIB on the clipboard (CF_DIB or CF_DIBV5, 24 or 32 bits) as BGRA pixels, top-down.
bool DibPixels(HANDLE handle, std::vector<uint32_t>* px, UINT* w, UINT* h) {
    auto* bih = (const BITMAPINFOHEADER*)GlobalLock(handle);
    if (!bih) return false;
    size_t size = GlobalSize(handle);
    bool ok = size >= sizeof(BITMAPINFOHEADER) && (bih->biBitCount == 32 || bih->biBitCount == 24) &&
              (bih->biCompression == BI_RGB || bih->biCompression == BI_BITFIELDS) && bih->biWidth > 0 && bih->biHeight != 0;
    if (ok) {
        *w = (UINT)bih->biWidth, *h = (UINT)std::abs(bih->biHeight);
        bool bottomUp = bih->biHeight > 0;
        size_t offset = bih->biSize + (bih->biCompression == BI_BITFIELDS && bih->biSize == sizeof(BITMAPINFOHEADER) ? 12 : 0) +
                        (size_t)bih->biClrUsed * 4;
        size_t stride = ((size_t)*w * bih->biBitCount + 31) / 32 * 4;
        ok = offset + stride * *h <= size;
        if (ok) {
            const BYTE* bits = (const BYTE*)bih + offset;
            px->assign((size_t)*w * *h, 0);
            bool anyAlpha = false;
            for (UINT y = 0; y < *h; y++) {
                const BYTE* row = bits + stride * (bottomUp ? *h - 1 - y : y);
                for (UINT x = 0; x < *w; x++) {
                    uint32_t v = bih->biBitCount == 32 ? *(const uint32_t*)(row + x * 4)
                                                       : (0xFF000000u | row[x * 3] | row[x * 3 + 1] << 8 | row[x * 3 + 2] << 16);
                    anyAlpha |= (v >> 24) != 0;
                    (*px)[(size_t)y * *w + x] = v;
                }
            }
            if (!anyAlpha)  // (a screenshot's 32 bits: its alpha unused)
                for (auto& v : *px) v |= 0xFF000000u;
        }
    }
    GlobalUnlock(handle);
    return ok;
}

// noctalia.saveClipboardImage(path): the picture on the clipboard saved as a PNG (Tourne Desktop's).
int ApiSaveClipboardImage(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"clipboardRead")) return FailBool(L, "it wasn't allowed to read the clipboard when installed");
    std::wstring path;
    std::string why;
    if (!Resolve(e, Arg(L, 1), true, &path, &why)) return FailBool(L, why);
    static const UINT pngFormat = RegisterClipboardFormatW(L"PNG");
    std::string png;
    std::vector<uint32_t> px;
    UINT w = 0, h = 0;
    if (!OpenClipboard(g_window)) return FailBool(L, "the clipboard is busy");
    if (HANDLE data = GetClipboardData(pngFormat)) {
        if (const void* bytes = GlobalLock(data)) {
            png.assign((const char*)bytes, GlobalSize(data));
            GlobalUnlock(data);
        }
    }
    if (png.empty()) {
        HANDLE dib = GetClipboardData(CF_DIBV5);
        if (!dib || !DibPixels(dib, &px, &w, &h)) {
            px.clear();
            if ((dib = GetClipboardData(CF_DIB)) && !DibPixels(dib, &px, &w, &h)) px.clear();
        }
    }
    if (png.empty() && px.empty()) {  // (only a bitmap handle: through GetDIBits)
        if (HBITMAP bitmap = (HBITMAP)GetClipboardData(CF_BITMAP)) {
            BITMAP b{};
            if (GetObjectW(bitmap, sizeof(b), &b) && b.bmWidth > 0 && b.bmHeight > 0) {
                w = b.bmWidth, h = b.bmHeight;
                px.assign((size_t)w * h, 0);
                BITMAPINFO bi{};
                bi.bmiHeader = {sizeof(BITMAPINFOHEADER), (LONG)w, -(LONG)h, 1, 32, BI_RGB};
                HDC dc = GetDC(nullptr);
                if (!GetDIBits(dc, bitmap, 0, h, px.data(), &bi, DIB_RGB_COLORS)) px.clear();
                ReleaseDC(nullptr, dc);
                for (auto& v : px) v |= 0xFF000000u;
            }
        }
    }
    CloseClipboard();
    if (png.empty() && (px.empty() || !EncodePng(px, w, h, &png))) return FailBool(L, "there's no picture on the clipboard");
    if (!WriteAll(path, png)) return FailBool(L, "couldn't write it");
    lua_pushboolean(L, true);
    return 1;
}

// A path the user dropped on (or pasted into) the plugin's panel, made whole; empty if it isn't one.
std::wstring DroppedPath(Entry& e, const std::string& given) {
    std::wstring p = W(given);
    for (auto& c : p)
        if (c == L'/') c = L'\\';
    std::wstring full = Full(p);
    for (auto const& d : e.dropped)
        if (CompareStringOrdinal(d.c_str(), -1, full.c_str(), -1, TRUE) == CSTR_EQUAL) return full;
    return {};
}

// noctalia.copyFile(from, to): from its own folders or a file the user dropped on its panel, to where it may write.
int ApiCopyFile(lua_State* L) {
    Entry& e = E(L);
    std::wstring from = DroppedPath(e, Arg(L, 1)), to;
    std::string why;
    if (from.empty() && !Resolve(e, Arg(L, 1), false, &from, &why)) return FailBool(L, why);
    if (!Resolve(e, Arg(L, 2), true, &to, &why)) return FailBool(L, why);
    if (!CopyFileW(from.c_str(), to.c_str(), TRUE))
        return FailBool(L, GetLastError() == ERROR_FILE_EXISTS ? "there's a file there already" : "couldn't copy it");
    lua_pushboolean(L, true);
    return 1;
}

// noctalia.openPath(path [, "notepad"]): a file or folder of its own (or one dropped on its panel) in its usual
// app (or Notepad), or a web link in the browser.
int ApiOpenPath(lua_State* L) {
    Entry& e = E(L);
    if (!Granted(e, L"open")) return FailBool(L, "it wasn't allowed to open files and links when installed");
    std::string given = Arg(L, 1), how = OptArg(L, 2);
    std::string lower = given;
    for (auto& c : lower) c = (char)tolower((unsigned char)c);
    bool web = lower.rfind("http://", 0) == 0 || lower.rfind("https://", 0) == 0 || lower.rfind("mailto:", 0) == 0;
    std::wstring target = web ? W(given) : DroppedPath(e, given);
    std::string why;
    if (target.empty() && !Resolve(e, given, false, &target, &why)) return FailBool(L, why);
    HINSTANCE done = how == "notepad" && !web
                         ? ShellExecuteW(nullptr, L"open", L"notepad.exe", (L"\"" + target + L"\"").c_str(), nullptr, SW_SHOWNORMAL)
                         : ShellExecuteW(nullptr, L"open", target.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if ((INT_PTR)done <= 32) return FailBool(L, "Windows couldn't open it");
    lua_pushboolean(L, true);
    return 1;
}

bool DarkMode() {
    DWORD light = 1, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme",
                 RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light == 0;
}

int ApiIsDarkMode(lua_State* L) {
    lua_pushboolean(L, DarkMode());
    return 1;
}

// Noctalia's colour roles in System Icons' flyout colours (the Tourne scheme when it isn't running):
// "on_surface", "primary/0.6" (over the ground), "#rrggbb" or "#rrggbbaa".
COLORREF RoleColor(const std::string& given) {
    enum { kGround, kRaised, kLine, kEdge, kText, kAccent, kMuted, kDim, kWarn, kBad, kHover };
    uint32_t p[11] = {0x121C21, 0x1B2A31, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x64AA89, 0x5D8D6F, 0xFF6F77, 0xFF6F77, 0x1F3D34};
    FlyoutPalette(p);
    auto rgb = [](uint32_t v) { return RGB(v >> 16 & 0xFF, v >> 8 & 0xFF, v & 0xFF); };
    std::string role = given;
    double alpha = 1;
    if (size_t slash = role.find('/'); slash != std::string::npos) {
        alpha = std::clamp(strtod(role.c_str() + slash + 1, nullptr), 0.0, 1.0);
        role.resize(slash);
    }
    COLORREF c = CLR_INVALID;
    if (role.size() >= 7 && role[0] == '#') {
        unsigned v = (unsigned)strtoul(role.substr(1, 6).c_str(), nullptr, 16);
        c = rgb(v);
        if (role.size() == 9) alpha *= strtoul(role.substr(7, 2).c_str(), nullptr, 16) / 255.0;
    } else {
        static const std::map<std::string, int> roles = {
            {"surface", kGround}, {"background", kGround}, {"surface_variant", kRaised}, {"surface_container", kRaised},
            {"surface_container_high", kRaised}, {"on_surface", kText}, {"on_background", kText},
            {"on_surface_variant", kMuted}, {"primary", kAccent}, {"accent", kAccent}, {"on_primary", kGround},
            {"secondary", kMuted}, {"on_secondary", kGround}, {"tertiary", kText}, {"on_tertiary", kGround},
            {"error", kWarn}, {"on_error", kGround}, {"outline", kEdge}, {"outline_variant", kLine}, {"shadow", kGround},
            {"hover", kHover}, {"on_hover", kText}};
        auto it = roles.find(role);
        if (it == roles.end()) return CLR_INVALID;
        c = rgb(p[it->second]);
    }
    if (alpha < 1) {  // (over the ground)
        COLORREF g = rgb(p[kGround]);
        auto mix = [&](int a, int b) { return (int)std::lround(b + (a - b) * alpha); };
        c = RGB(mix(GetRValue(c), GetRValue(g)), mix(GetGValue(c), GetGValue(g)), mix(GetBValue(c), GetBValue(g)));
    }
    return c;
}

int ApiGetColor(lua_State* L) {
    COLORREF c = RoleColor(Arg(L, 1));
    if (c == CLR_INVALID) return lua_pushnil(L), 1;
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02x%02x%02x", GetRValue(c), GetGValue(c), GetBValue(c));
    lua_pushstring(L, hex);
    return 1;
}

// HTTP, off this thread; the answer comes back as a call on it.
int ApiHttp(lua_State* L) {
    Entry& e = E(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_getfield(L, 1, "url");
    std::string url = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_getfield(L, 1, "method");
    std::string method = lua_isstring(L, -1) ? lua_tostring(L, -1) : "GET";
    lua_getfield(L, 1, "body");
    std::string body = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_getfield(L, 1, "basic_username");
    std::wstring user = lua_isstring(L, -1) ? W(lua_tostring(L, -1)) : L"";
    lua_getfield(L, 1, "basic_password");
    std::wstring password = lua_isstring(L, -1) ? W(lua_tostring(L, -1)) : L"";
    lua_getfield(L, 1, "follow_redirects");
    bool redirects = lua_isnil(L, -1) || lua_toboolean(L, -1);
    lua_getfield(L, 1, "allow_insecure_tls");  // (a self-signed certificate, on a machine of the user's own)
    bool insecure = lua_toboolean(L, -1);
    lua_pop(L, 7);
    std::wstring headers;
    lua_getfield(L, 1, "headers");
    if (lua_istable(L, -1))
        for (int i = 1, n = lua_objlen(L, -1); i <= n; i++) {
            lua_rawgeti(L, -1, i);
            if (lua_isstring(L, -1)) headers += W(lua_tostring(L, -1)) + L"\r\n";
            lua_pop(L, 1);
        }
    lua_pop(L, 1);
    int ref = lua_ref(L, 2);
    uint64_t serial = e.serial;
    auto answer = [serial, ref](bool ok, int status, std::string text) {
        Post([serial, ref, ok, status, text = std::move(text)] {
            auto found = g_bySerial.find(serial);
            if (found == g_bySerial.end()) return;
            Entry& to = *found->second;
            Guarded(to, [&] {
                lua_settop(to.T, 0);
                lua_getref(to.T, ref);
                lua_unref(to.T, ref);
                lua_createtable(to.T, 0, 3);
                lua_pushboolean(to.T, ok);
                lua_setfield(to.T, -2, "ok");
                lua_pushinteger(to.T, status);
                lua_setfield(to.T, -2, "status");
                lua_pushlstring(to.T, text.data(), text.size());
                lua_setfield(to.T, -2, "body");
                return Call(to, 1);
            });
        });
    };
    // Allowed: the internet at all, then (when the plugin names them) only its hosts.
    std::wstring wurl = W(url);
    URL_COMPONENTSW parts{sizeof(parts)};
    wchar_t host[256] = L"", path[2048] = L"";
    parts.lpszHostName = host, parts.dwHostNameLength = 256;
    parts.lpszUrlPath = path, parts.dwUrlPathLength = 2048;
    bool parsed = WinHttpCrackUrl(wurl.c_str(), 0, 0, &parts);
    std::string refused;
    if (!Granted(e, L"internet")) refused = "it wasn't allowed the internet when installed";
    else if (!parsed || (parts.nScheme != INTERNET_SCHEME_HTTPS && parts.nScheme != INTERNET_SCHEME_HTTP)) refused = "not a web address: " + url;
    else if (!e.plugin->hosts.empty()) {
        std::wstring h = host;
        bool listed = false;
        for (auto const& allowed : e.plugin->hosts)
            if (!_wcsicmp(h.c_str(), allowed.c_str()) ||
                (h.size() > allowed.size() && !_wcsicmp(h.c_str() + h.size() - allowed.size(), allowed.c_str()) && h[h.size() - allowed.size() - 1] == L'.'))
                listed = true;
        if (!listed) refused = "it may reach only the hosts it was installed with, not " + U(h);
    }
    if (!refused.empty()) {
        TellOnce(e, "an HTTP request was refused: " + refused);
        answer(false, 0, "");
        lua_pushboolean(L, false);
        return 1;
    }
    std::wstring wmethod = W(method), whost = host, wpath = path;
    INTERNET_PORT port = parts.nPort;
    bool secure = parts.nScheme == INTERNET_SCHEME_HTTPS;
    std::thread([=] {
        HINTERNET session = WinHttpOpen(L"Tourne Desktop", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        HINTERNET connect = session ? WinHttpConnect(session, whost.c_str(), port, 0) : nullptr;
        HINTERNET request = connect ? WinHttpOpenRequest(connect, wmethod.c_str(), wpath.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                         WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0)
                                    : nullptr;
        if (session) WinHttpSetTimeouts(session, 10000, 10000, 15000, 30000);
        if (request && insecure) {
            DWORD flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                          SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
            WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &flags, sizeof(flags));
        }
        if (request && !redirects) {
            DWORD off = WINHTTP_DISABLE_REDIRECTS;
            WinHttpSetOption(request, WINHTTP_OPTION_DISABLE_FEATURE, &off, sizeof(off));
        }
        if (request && !user.empty())
            WinHttpSetCredentials(request, WINHTTP_AUTH_TARGET_SERVER, WINHTTP_AUTH_SCHEME_BASIC, user.c_str(), password.c_str(), nullptr);
        bool ok = request && WinHttpSendRequest(request, headers.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : headers.c_str(), (DWORD)-1,
                                                body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.data(), (DWORD)body.size(),
                                                (DWORD)body.size(), 0) &&
                  WinHttpReceiveResponse(request, nullptr);
        DWORD status = 0, size = sizeof(status);
        std::string text;
        if (ok) {
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, nullptr, &status, &size, nullptr);
            for (;;) {
                DWORD available = 0;
                if (!WinHttpQueryDataAvailable(request, &available) || !available || text.size() > (8u << 20)) break;
                size_t at = text.size();
                text.resize(at + available);
                DWORD got = 0;
                if (!WinHttpReadData(request, text.data() + at, available, &got)) break;
                text.resize(at + got);
            }
        }
        if (request) WinHttpCloseHandle(request);
        if (connect) WinHttpCloseHandle(connect);
        if (session) WinHttpCloseHandle(session);
        answer(ok, (int)status, std::move(text));
    }).detach();
    lua_pushboolean(L, true);
    return 1;
}

// What doesn't run yet, or can't on Windows: said once, and answered as "no".
int NotYet(lua_State* L) {
    Entry& e = E(L);
    const char* what = lua_tostring(L, lua_upvalueindex(1));
    TellOnce(e, std::string(what) + " doesn't work in Tourne Desktop yet");
    lua_pushboolean(L, false);
    return 1;
}

int Nothing(lua_State* L) {
    lua_pushnil(L);
    return 1;
}

int EmptyList(lua_State* L) {
    lua_newtable(L);
    return 1;
}

// launcher.setResults(text, {{id, title, subtitle, glyph, score}...}): a provider's answer to onQuery(text).
int LauncherSetResults(lua_State* L) {
    Entry& e = E(L);
    LauncherAnswer answer;
    answer.text = W(OptArg(L, 1));
    if (lua_istable(L, 2)) {
        int n = (int)lua_objlen(L, 2);
        for (int i = 1; i <= n && i <= 200; i++) {
            lua_rawgeti(L, 2, i);
            if (lua_istable(L, -1)) {
                LauncherItem item;
                item.provider = Name(e);
                auto field = [&](const char* name) {
                    lua_getfield(L, -1, name);
                    std::wstring v;
                    if (lua_isstring(L, -1)) v = W(lua_tostring(L, -1));
                    lua_pop(L, 1);
                    return v;
                };
                item.id = field("id");
                item.title = field("title");
                item.subtitle = field("subtitle");
                item.glyph = field("glyph");
                lua_getfield(L, -1, "score");
                item.score = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
                lua_pop(L, 1);
                if (!item.title.empty()) answer.items.push_back(std::move(item));
            }
            lua_pop(L, 1);
        }
    }
    HWND wnd;
    UINT msg;
    {
        std::lock_guard lock(g_lock);
        g_launcherAnswers[Name(e)] = std::move(answer);
        wnd = g_launcherWnd;
        msg = g_launcherMsg;
    }
    if (wnd && msg) PostMessageW(wnd, msg, 0, 0);
    return 0;
}

int ApiFuzzyScore(lua_State* L) {
    std::string pattern = Arg(L, 1), text = Arg(L, 2);
    size_t at = 0;
    double score = 0, run = 0;
    for (char c : pattern) {
        size_t found = std::string::npos;
        for (size_t i = at; i < text.size(); i++)
            if (tolower((unsigned char)text[i]) == tolower((unsigned char)c)) {
                found = i;
                break;
            }
        if (found == std::string::npos) return lua_pushnil(L), 1;
        run = found == at ? run + 1 : 0;
        score += 1 + run * 2 - (double)(found - at) * 0.1;
        at = found + 1;
    }
    lua_pushnumber(L, score);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// barWidget.* and ui.*

int BarSetText(lua_State* L) {
    Entry& e = E(L);
    e.text = W(OptArg(L, 1));
    BarChanged();
    return 0;
}

int BarSetGlyph(lua_State* L) {
    Entry& e = E(L);
    e.glyphName = OptArg(L, 1);
    e.glyph = GlyphChar(e.glyphName);
    BarChanged();
    return 0;
}

int BarSetTooltip(lua_State* L) {
    Entry& e = E(L);
    std::wstring tip;
    auto row = [&](int idx) {
        lua_getfield(L, idx, "key");
        lua_getfield(L, idx, "value");
        if (lua_isnil(L, -2) && lua_isnil(L, -1)) {  // ({left, right}, as Noctalia also takes a row)
            lua_pop(L, 2);
            lua_rawgeti(L, idx, 1);
            lua_rawgeti(L, idx, 2);
        }
        std::string k = lua_isstring(L, -2) ? lua_tostring(L, -2) : "", v = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
        lua_pop(L, 2);
        if (!tip.empty()) tip += L"\n";
        tip += W(k.empty() ? v : v.empty() ? k : k + ": " + v);
    };
    if (lua_isstring(L, 1)) tip = W(lua_tostring(L, 1));
    else if (lua_istable(L, 1)) {
        if (lua_objlen(L, 1) > 0)
            for (int i = 1, n = lua_objlen(L, 1); i <= n; i++) {
                lua_rawgeti(L, 1, i);
                if (lua_istable(L, -1)) row(lua_gettop(L));
                lua_pop(L, 1);
            }
        else row(1);
    }
    e.tip = tip;
    BarChanged();
    return 0;
}

int BarClearTooltip(lua_State* L) {
    E(L).tip.clear();
    BarChanged();
    return 0;
}

int BarSetVisible(lua_State* L) {
    E(L).visible = lua_toboolean(L, 1);
    BarChanged();
    return 0;
}

int BarSetColor(lua_State* L) {
    E(L).color = lua_isstring(L, 1) ? RoleColor(lua_tostring(L, 1)) : CLR_INVALID;
    E(L).colorRole = lua_isstring(L, 1) ? lua_tostring(L, 1) : "";
    BarChanged();
    return 0;
}

int BarIsVertical(lua_State* L) {
    lua_pushboolean(L, false);
    return 1;
}

int Ignore(lua_State*) { return 0; }

// A ui.* tree, flattened for the bar: its first glyph, and its text in order.
void Flatten(lua_State* L, int idx, std::wstring* glyph, std::wstring* text, int depth) {
    if (depth > 32 || !lua_istable(L, idx)) return;
    idx = lua_absindex(L, idx);
    lua_checkstack(L, 6);
    lua_getfield(L, idx, "type");
    std::string type = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
    lua_pop(L, 1);
    lua_getfield(L, idx, "props");
    int props = lua_gettop(L);
    auto prop = [&](const char* name) {
        std::string v;
        if (lua_istable(L, props)) {
            lua_getfield(L, props, name);
            if (lua_isstring(L, -1)) v = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
        return v;
    };
    auto addText = [&](const std::string& s) {
        if (s.empty()) return;
        if (!text->empty()) *text += L"  ";
        *text += W(s);
    };
    if (type == "glyph") {
        std::wstring g = GlyphChar(prop("name").empty() ? prop("glyph") : prop("name"));
        if (glyph->empty()) *glyph = g;
        else if (!g.empty()) addText(U(g));  // (a later glyph: kept among the text)
    } else if (type == "label" || type == "markdown") {
        addText(prop("text"));
    } else if (type == "button") {
        if (glyph->empty()) *glyph = GlyphChar(prop("glyph"));
        addText(prop("text"));
    }
    lua_pop(L, 1);
    lua_getfield(L, idx, "children");
    if (lua_istable(L, -1)) {
        int children = lua_gettop(L);
        for (int i = 1, n = lua_objlen(L, children); i <= n; i++) {
            lua_rawgeti(L, children, i);
            Flatten(L, -1, glyph, text, depth + 1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

int BarRender(lua_State* L) {
    Entry& e = E(L);
    std::wstring glyph, text;
    Flatten(L, 1, &glyph, &text, 0);
    e.glyph = glyph, e.text = text;
    BarChanged();
    return 0;
}

// ui.row(props, children) and the rest: a plain table the host reads.
int UiNode(lua_State* L) {
    lua_createtable(L, 0, 3);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setfield(L, -2, "type");
    if (lua_istable(L, 1)) lua_pushvalue(L, 1);
    else lua_newtable(L);
    lua_setfield(L, -2, "props");
    if (lua_istable(L, 2)) lua_pushvalue(L, 2);
    else lua_newtable(L);
    lua_setfield(L, -2, "children");
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Panels: the plugin's ui.* tree (plugin-panel.cpp draws it), opened by togglePanel

// A ui.* table as a panel node: handlers (functions, or names of globals) apart from the other props.
panel::Node ToNode(lua_State* L, int idx, int depth) {
    panel::Node n;
    idx = lua_absindex(L, idx);
    if (depth > 64 || !lua_istable(L, idx)) return n;
    lua_checkstack(L, 6);
    lua_getfield(L, idx, "type");
    if (lua_isstring(L, -1)) n.type = lua_tostring(L, -1);
    lua_pop(L, 1);
    n.props.kind = json::Value::Object;
    lua_getfield(L, idx, "props");
    if (lua_istable(L, -1)) {
        int props = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, props)) {
            if (lua_type(L, -2) == LUA_TSTRING) {
                std::string key = lua_tostring(L, -2);
                bool handler = key.size() > 2 && key[0] == 'o' && key[1] == 'n' && isupper((unsigned char)key[2]);
                if (handler && lua_isfunction(L, -1)) n.on[key].ref = lua_ref(L, -1);
                else if (handler && lua_type(L, -1) == LUA_TSTRING) n.on[key].name = lua_tostring(L, -1);
                else n.props.fields.push_back({key, ToJson(L, -1)});
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    lua_getfield(L, idx, "children");
    if (lua_istable(L, -1)) {
        int children = lua_gettop(L);
        for (int i = 1, count = lua_objlen(L, children); i <= count; i++) {
            lua_rawgeti(L, children, i);
            if (lua_istable(L, -1)) n.children.push_back(ToNode(L, -1, depth + 1));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    return n;
}

void ReleaseRefs(Entry& e, const std::vector<int>& refs) {
    if (!e.L) return;
    for (int r : refs) lua_unref(e.L, r);
}

// An image node's path made whole, if the plugin may read it (else the picture shows as missing).
void ResolvePictures(Entry& e, panel::Node& n) {
    if (n.type == "image")
        for (auto& [k, v] : n.props.fields) {
            if ((k != "path" && k != "source") || v.kind != json::Value::String || v.text.empty()) continue;
            std::wstring full = DroppedPath(e, v.text);
            std::string why;
            if (full.empty() && !Resolve(e, v.text, false, &full, &why)) full = L"";
            v.text = U(full);
        }
    for (auto& c : n.children) ResolvePictures(e, c);
}

int PanelRender(lua_State* L) {
    Entry& e = E(L);
    panel::Node tree = ToNode(L, 1, 0);
    ResolvePictures(e, tree);
    if (!e.panel) {  // (not open: nothing to show it in)
        std::vector<int> refs;
        std::function<void(const panel::Node&)> walk = [&](const panel::Node& n) {
            for (auto const& [k, h] : n.on)
                if (h.ref >= 0) refs.push_back(h.ref);
            for (auto const& c : n.children) walk(c);
        };
        walk(tree);
        ReleaseRefs(e, refs);
        return 0;
    }
    std::vector<int> old = e.panel->Refs();
    e.panel->Render(std::move(tree));
    ReleaseRefs(e, old);
    return 0;
}

void ClosePanel(Entry& e) {
    if (!e.panel) return;
    CallGlobal(e, "onClose");
    if (!e.panel) return;  // (onClose closed it itself)
    std::vector<int> refs = e.panel->Refs();
    e.panel.reset();
    ReleaseRefs(e, refs);
    e.wantsTicks = false;
    e.frameTicks = false;
    e.interval = 0;
}

int PanelClose(lua_State* L) {
    uint64_t serial = E(L).serial;
    Later([serial] {  // (after the call that asked: it may be one of the panel's own handlers)
        if (auto it = g_bySerial.find(serial); it != g_bySerial.end()) ClosePanel(*it->second);
    });
    return 0;
}

int PanelWantsTicks(lua_State* L) {
    Entry& e = E(L);
    e.wantsTicks = lua_toboolean(L, 1);
    e.interval = e.wantsTicks && e.panel ? 1000 : 0;
    e.due = GetTickCount64() + 1000;
    return 0;
}

// desktopWidget.setWantsSecondTicks(true): update() on each second's turn (its own pace otherwise).
int DesktopWantsTicks(lua_State* L) {
    Entry& e = E(L);
    e.wantsTicks = lua_toboolean(L, 1);
    if (!e.wantsTicks) return 0;
    SYSTEMTIME st;
    GetSystemTime(&st);
    e.interval = 1000;
    e.due = GetTickCount64() + (1000 - st.wMilliseconds);
    return 0;
}

// panel/desktopWidget.setNeedsFrameTick(on): onFrameTick(deltaMs) about 60 times a second while it shows.
int WantsFrameTicks(lua_State* L) {
    Entry& e = E(L);
    e.frameTicks = lua_toboolean(L, 1);
    e.lastFrame = 0;
    return 0;
}

// panel.insertText(key, text [, ownLine]): text put where the caret is in the input with that key (Tourne Desktop's).
int PanelInsertText(lua_State* L) {
    Entry& e = E(L);
    lua_pushboolean(L, e.panel && e.panel->InsertText(Arg(L, 1), Arg(L, 2), lua_toboolean(L, 3)));
    return 1;
}

int PanelContextMenu(lua_State* L) {
    TellOnce(E(L), "panel.openContextMenu doesn't work in Tourne Desktop yet");
    lua_pushboolean(L, false);
    return 1;
}

// A handler of the open panel's, found again by the node's identity, called with its arguments.
void PanelEvent(uint64_t serial, const std::string& identity, const std::string& name, const std::vector<std::string>& args) {
    auto it = g_bySerial.find(serial);
    if (it == g_bySerial.end() || !it->second->panel) return;
    Entry& e = *it->second;
    const panel::Handler* found = e.panel->Find(identity, name);
    if (!found) return;
    panel::Handler h = *found;
    if (name == "onDrop" || name == "onPasteFiles")  // (the plugin may copy and open what the user gave it)
        for (auto const& a : args) e.dropped.insert(Full(W(a)));
    Guarded(e, [&] {
        lua_settop(e.T, 0);
        if (h.ref >= 0) lua_getref(e.T, h.ref);
        else lua_getglobal(e.T, h.name.c_str());
        if (!lua_isfunction(e.T, -1)) {
            lua_settop(e.T, 0);
            return false;
        }
        for (auto const& a : args) lua_pushlstring(e.T, a.data(), a.size());
        return Call(e, (int)args.size());
    });
}

void OpenPanel(Entry& e) {
    if (e.panel || !e.L) return;
    panel::Placement where;
    const json::Value& m = e.manifest;
    if (m["width"].kind == json::Value::Number) where.width = (int)m["width"].number;
    if (m["height"].kind == json::Value::Number) where.height = (int)m["height"].number;
    where.fillHeight = m["height"].text == "fill";
    if (!m["placement"].text.empty()) where.placement = m["placement"].text;
    where.position = m["position"].text;
    GetCursorPos(&where.anchor);
    where.remember = Name(e);
    uint64_t serial = e.serial;
    panel::Host host;
    host.event = [serial](const std::string& id, const std::string& name, std::vector<std::string> args) {
        Later([=] { PanelEvent(serial, id, name, args); });
        if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
    };
    host.closed = [serial] {
        Later([serial] {
            if (auto it = g_bySerial.find(serial); it != g_bySerial.end()) ClosePanel(*it->second);
        });
        if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
    };
    host.color = [](const std::string& role) { return RoleColor(role); };
    // The keys it asked for (capture_keys) go to its onKey(chord, pressed), when it has one.
    bool onKey = Guarded(e, [&] {
        lua_getglobal(e.T, "onKey");
        bool is = lua_isfunction(e.T, -1);
        lua_settop(e.T, 0);
        return is;
    });
    if (onKey)
        for (auto const& k : m["capture_keys"].items)
            if (k.kind == json::Value::String && !k.text.empty()) where.captureKeys.push_back(k.text);
    host.key = [serial](const std::string& chord, bool pressed) {
        Later([=] {
            if (auto it = g_bySerial.find(serial); it != g_bySerial.end() && it->second->panel)
                CallGlobal(*it->second, "onKey", [&](lua_State* T) {
                    lua_pushlstring(T, chord.data(), chord.size());
                    lua_pushboolean(T, pressed);
                    return 2;
                });
        });
        if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
    };
    e.panel = panel::Window::Open(where, std::move(host));
    if (!e.panel) return Problem(e, L"its panel window couldn't open");
    CallGlobal(e, "onOpen", [](lua_State* T) { return lua_pushnil(T), 1; });
}

// A panel ("author/plugin:panel") opened, or closed when it's open.
void TogglePanel(const std::wstring& id) {
    for (auto& p : g_plugins)
        for (auto& e : p->entries) {
            if (e->kind != "panel" || Name(*e) != id) continue;
            if (e->panel) ClosePanel(*e);
            else if (GetTickCount64() - panel::LastDismissed() > 300) OpenPanel(*e);  // (the click that just closed it)
            return;
        }
    Log(L"no panel %ls to open", id.c_str());
}

// A desktop widget's window, opened as it starts (so what its script renders at once has a place): its first
// place is the main screen's top right, a step lower for each desktop widget before it.
void OpenDesktopWidget(Entry& e) {
    if (e.panel) return;
    panel::Placement where;
    where.desktop = true;
    where.width = e.manifest["width"].kind == json::Value::Number ? (int)e.manifest["width"].number : 0;
    where.remember = Name(e);
    RECT work{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    HDC screen = GetDC(nullptr);
    double scale = GetDeviceCaps(screen, LOGPIXELSX) / 96.0;
    ReleaseDC(nullptr, screen);
    int before = 0;
    for (auto& p : g_plugins)
        for (auto& other : p->entries) before += other.get() != &e && other->kind == "desktop_widget" && other->panel ? 1 : 0;
    where.anchor = {work.right - (int)std::lround(24 * scale), work.top + (int)std::lround((24 + before * 180) * scale)};
    uint64_t serial = e.serial;
    panel::Host host;
    host.event = [serial](const std::string& id, const std::string& name, std::vector<std::string> args) {
        Later([=] { PanelEvent(serial, id, name, args); });
        if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
    };
    // Off the desktop (its menu): its switch in the Studio turned off, which starts the plugins again without it.
    std::wstring key = L"plugins." + e.plugin->id + L".desktop_" + W(e.id);
    host.closed = [key] {
        Later([key] { WriteStudioSettings({{key, L"0"}}, key, L"desktop widget off"); });
        if (g_window) PostMessageW(g_window, kMsgWake, 0, 0);
    };
    host.color = [](const std::string& role) { return RoleColor(role); };
    e.panel = panel::Window::Open(where, std::move(host));
    if (!e.panel) Problem(e, L"its desktop window couldn't open");
}

// noctalia.togglePanel("author/plugin:panel"): open, or close when it's open.
int ApiTogglePanel(lua_State* L) {
    std::wstring id = W(Arg(L, 1));
    Later([id] { TogglePanel(id); });
    return 0;
}

// Hotkeys: a panel whose settings have a "hotkey" ("Ctrl+Alt+N", "Win+Shift+N", "F9", or the Studio's "ctrl+alt+78") opens and closes with it.
std::map<int, std::wstring> g_hotkeys;  // RegisterHotKey's id: the panel

// A hotkey as the Studio writes it: modifiers and keys joined with + ("ctrl+alt+n", "esc+end", "ctrl+alt+78":
// a key code), the last key the one pressed and any other held with it, and how it's pressed after a space:
// "x2" (twice), or "hold" and a time ("hold 800", "hold 1.2s"; alone, half a second).
struct HotkeySpec {
    UINT mods = 0;
    std::vector<UINT> keys;  // the held ones, then the one pressed
    bool twice = false;
    int holdMs = 0;
    bool Simple() const { return keys.size() == 1 && !twice && !holdMs; }  // (Windows' RegisterHotKey takes it)
};

bool ParseHotkey(const std::string& text, HotkeySpec* out) {
    *out = {};
    std::string lower;
    for (char c : text) lower += (char)tolower((unsigned char)c);
    // how it's pressed
    std::string combo = lower;
    if (size_t sp = lower.find(' '); sp != std::string::npos) {
        combo = lower.substr(0, sp);
        std::string how = lower.substr(sp + 1);
        if (how.find("x2") != std::string::npos || how.find("double") != std::string::npos || how.find("twice") != std::string::npos) {
            out->twice = true;
        } else if (size_t h = how.find("hold"); h != std::string::npos) {
            const char* at = how.c_str() + h + 4;
            while (*at == ' ') at++;
            double n = atof(at);
            bool seconds = strstr(at, "ms") == nullptr && (strchr(at, 's') != nullptr || (n > 0 && n < 20));
            out->holdMs = n > 0 ? (int)(seconds ? n * 1000 : n) : 500;
            out->holdMs = std::clamp(out->holdMs, 100, 10000);
        }
    }
    std::vector<std::string> parts;
    std::string part;
    for (char c : combo + "+") {
        if (c != '+') {
            if (!isspace((unsigned char)c)) part += c;
            continue;
        }
        if (!part.empty()) parts.push_back(part);
        part.clear();
    }
    if (parts.empty()) return false;
    static const std::map<std::string, UINT> named = {
        {"space", VK_SPACE}, {"enter", VK_RETURN}, {"return", VK_RETURN}, {"tab", VK_TAB}, {"esc", VK_ESCAPE},
        {"escape", VK_ESCAPE}, {"insert", VK_INSERT}, {"ins", VK_INSERT}, {"delete", VK_DELETE}, {"del", VK_DELETE},
        {"home", VK_HOME}, {"end", VK_END}, {"pageup", VK_PRIOR}, {"pgup", VK_PRIOR}, {"pagedown", VK_NEXT},
        {"pgdn", VK_NEXT}, {"up", VK_UP}, {"down", VK_DOWN}, {"left", VK_LEFT}, {"right", VK_RIGHT},
        {"backspace", VK_BACK}, {"pause", VK_PAUSE}, {"printscreen", VK_SNAPSHOT}, {"prtsc", VK_SNAPSHOT},
        {"print", VK_SNAPSHOT}, {"capslock", VK_CAPITAL}, {"scrolllock", VK_SCROLL}, {"menu", VK_APPS}};
    for (const std::string& k : parts) {
        if (k == "ctrl" || k == "control") out->mods |= MOD_CONTROL;
        else if (k == "alt") out->mods |= MOD_ALT;
        else if (k == "shift") out->mods |= MOD_SHIFT;
        else if (k == "win" || k == "windows" || k == "super" || k == "meta") out->mods |= MOD_WIN;
        else {
            UINT vk = 0;
            if (auto it = named.find(k); it != named.end()) vk = it->second;
            else if (k.size() == 1 && isalnum((unsigned char)k[0])) vk = (UINT)toupper((unsigned char)k[0]);
            else if (k.size() >= 2 && k[0] == 'f' && isdigit((unsigned char)k[1])) {
                int n = atoi(k.c_str() + 1);
                if (n >= 1 && n <= 24) vk = VK_F1 + n - 1;
            } else if (k.size() == 1) {
                SHORT scan = VkKeyScanW((wchar_t)k[0]);
                if (scan != -1) vk = scan & 0xFF;
            } else if (k.find_first_not_of("0123456789") == std::string::npos) {  // (the Studio's "alt+45": a key code)
                int code = atoi(k.c_str());
                if (code > 0 && code < 255) vk = (UINT)code;
            }
            if (!vk) return false;
            out->keys.push_back(vk);
        }
    }
    if (out->keys.empty()) return false;
    // (a lone letter or digit, pressed once, would take it from every program)
    UINT last = out->keys.back();
    bool letter = (last >= 'A' && last <= 'Z') || (last >= '0' && last <= '9');
    return !(letter && !out->mods && out->keys.size() == 1 && !out->twice && !out->holdMs);
}

// A bind line for Keys and rules (binds.cpp): "bindk = CTRL, code:27&code:35, plugin, tourne/power-menu:menu".
std::wstring HotkeyBindLine(const HotkeySpec& h, const std::wstring& panel) {
    std::wstring flags = h.twice ? L"k" : h.holdMs ? L"o" + std::to_wstring(h.holdMs) : L"";
    std::wstring mods;
    if (h.mods & MOD_CONTROL) mods += L"CTRL ";
    if (h.mods & MOD_ALT) mods += L"ALT ";
    if (h.mods & MOD_SHIFT) mods += L"SHIFT ";
    if (h.mods & MOD_WIN) mods += L"WIN ";
    std::wstring keys;
    for (UINT vk : h.keys) keys += (keys.empty() ? L"" : L"&") + std::wstring(L"code:") + std::to_wstring(vk);
    return L"bind" + flags + L" = " + mods + L", " + keys + L", plugin, " + panel;
}

void UnregisterHotkeys() {
    for (auto const& [id, name] : g_hotkeys) UnregisterHotKey(g_window, id);
    g_hotkeys.clear();
}

// The panels' hotkeys: through Keys and rules (binds.cpp, the switcher's keyboard hook) while it's on, so any
// keys work, pressed twice or held; else Windows' own hotkeys, which take a modifier and one key.
void RegisterHotkeys() {
    UnregisterHotkeys();
    bool binds = GetInt(L"switcher", L"binds.enabled") != 0;
    std::wstring lines;  // REG_MULTI_SZ: each line, then an empty one
    int next = 1;
    for (auto& p : g_plugins)
        for (auto& e : p->entries) {
            if (e->kind != "panel" || !e->L) continue;
            std::string key = e->settings["hotkey"].text;
            if (key.find_first_not_of(" \t") == std::string::npos) continue;
            HotkeySpec h;
            if (!ParseHotkey(key, &h)) {
                Problem(*e, L"its hotkey \"" + W(key) + L"\" isn't one (like Ctrl+Alt+N, Esc+End, or F8 x2)");
                continue;
            }
            if (binds) {
                lines += HotkeyBindLine(h, Name(*e)) + L'\0';
                Log(L"%ls opens with %hs (through Keys and rules)", Name(*e).c_str(), key.c_str());
                continue;
            }
            if (!h.Simple()) {
                Problem(*e, L"its hotkey " + W(key) + L" needs Keys and rules on (Studio, Switcher, tiling and borders)");
                continue;
            }
            if (RegisterHotKey(g_window, next, h.mods | MOD_NOREPEAT, h.keys.back())) {
                g_hotkeys[next++] = Name(*e);
                Log(L"%ls opens with %hs", Name(*e).c_str(), key.c_str());
            } else {
                Problem(*e, L"its hotkey " + W(key) + L" is taken by another program");
            }
        }
    // Written for binds.cpp to read, and every switcher told (only when it changed).
    static std::wstring written = L"\x1";
    if (lines == written) return;
    written = lines;
    HKEY reg = nullptr;
    if (!RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Desktop", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &reg, nullptr)) {
        if (lines.empty()) RegDeleteValueW(reg, L"PluginBinds");
        else RegSetValueExW(reg, L"PluginBinds", 0, REG_MULTI_SZ, (const BYTE*)lines.c_str(), (DWORD)((lines.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(reg);
    }
    static UINT reload = RegisterWindowMessageW(L"TourneDesktop.BindsReload");
    EnumWindows([](HWND w, LPARAM msg) -> BOOL {
        DWORD pid = 0;
        GetWindowThreadProcessId(w, &pid);
        wchar_t name[64];
        if (pid == GetCurrentProcessId() && GetClassNameW(w, name, 64) && !wcsncmp(name, L"TourneSwitcher_", 15))
            PostMessageW(w, (UINT)msg, 0, 0);
        return TRUE;
    }, reload);
}

int ApiExpandPath(lua_State* L) {
    std::string s = U(ExpandPath(Arg(L, 1)));
    lua_pushlstring(L, s.data(), s.size());
    return 1;
}

// The shell's own settings a plugin may read: Tourne's square corners for the one plugins use.
int ApiGetSetting(lua_State* L) {
    std::string key = Arg(L, 1);
    if (key == "shell.corner_radius_scale") lua_pushnumber(L, 0);
    else lua_pushnil(L);
    return 1;
}

void SetFunctions(lua_State* L, std::initializer_list<std::pair<const char*, lua_CFunction>> fns) {
    for (auto const& [name, fn] : fns) {
        lua_pushcfunction(L, fn, name);
        lua_setfield(L, -2, name);
    }
}

void SetNotYet(lua_State* L, std::initializer_list<const char*> names, const char* prefix) {
    for (const char* name : names) {
        lua_pushstring(L, (std::string(prefix) + name).c_str());
        lua_pushcclosure(L, NotYet, name, 1);
        lua_setfield(L, -2, name);
    }
}

// require("./shared.luau"): another .luau file of the plugin's own, run once per entry (with its globals) and what
// it returns kept, as Noctalia's Luau require (API 22). The path is relative to the file that requires it.
int ApiRequire(lua_State* L) {
    Entry& e = E(L);
    std::string given = Arg(L, 1);
    std::string from = e.file;
    lua_Debug ar{};
    if (lua_getinfo(L, 1, "s", &ar) && ar.source && ar.source[0] == '=') from = ar.source + 1;
    std::wstring base = e.plugin->folder + L"\\" + W(from);
    for (auto& c : base)
        if (c == L'/') c = L'\\';
    base = base.substr(0, base.rfind(L'\\'));
    std::wstring rel = W(given);
    for (auto& c : rel)
        if (c == L'/') c = L'\\';
    if (rel.size() < 5 || _wcsicmp(rel.c_str() + rel.size() - 5, L".luau")) rel += L".luau";
    std::wstring full = Full(base + L"\\" + rel);
    if (full.empty() || !Inside(full, e.plugin->folder)) luaL_error(L, "require: %s isn't a .luau file of this plugin's", given.c_str());
    if (auto it = e.modules.find(full); it != e.modules.end()) {
        if (it->second == LUA_NOREF) luaL_error(L, "require: %s requires itself (through another module)", given.c_str());
        lua_getref(L, it->second);
        return 1;
    }
    std::string source;
    if (!ReadAll(full, &source)) luaL_error(L, "require: couldn't read %s", given.c_str());
    std::string chunk = "=" + U(full.substr(e.plugin->folder.size() + 1));
    for (auto& c : chunk)
        if (c == '\\') c = '/';
    size_t size = 0;
    char* bytecode = luau_compile(source.data(), source.size(), nullptr, &size);
    int loaded = luau_load(L, chunk.c_str(), bytecode, size, 0);
    free(bytecode);
    if (loaded != 0) lua_error(L);  // (its message is on the stack)
    e.modules[full] = LUA_NOREF;
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        e.modules.erase(full);
        lua_error(L);
    }
    if (lua_isnil(L, -1)) lua_pop(L, 1), lua_pushboolean(L, true);
    e.modules[full] = lua_ref(L, -1);
    return 1;
}

void Event(const std::wstring& key, PluginBarEvent event);
#include "plugin-system.inl"
#include "plugin-extras.inl"

void OpenApi(lua_State* L, Entry& e) {
    lua_newtable(L);  // noctalia
    SetFunctions(L, {{"log", ApiLog}, {"getConfig", ApiGetConfig}, {"setUpdateInterval", ApiSetUpdateInterval},
                     {"tr", ApiTr}, {"trp", ApiTrp}, {"readFile", ApiReadFile}, {"writeFile", ApiWriteFile},
                     {"mkdirAll", ApiMkdirAll}, {"removeFile", ApiRemoveFile}, {"renameFile", ApiRenameFile},
                     {"fileExists", ApiFileExists}, {"fileInfo", ApiFileInfo}, {"listDir", ApiListDir},
                     {"pluginDir", ApiPluginDir}, {"pluginDataDir", ApiPluginDataDir}, {"formatTime", ApiFormatTime},
                     {"timeFormat", ApiTimeFormat}, {"dateFormat", ApiDateFormat}, {"isValidTimezone", ApiIsValidTimezone},
                     {"nowMs", ApiNowMs}, {"notify", ApiNotify}, {"notifyError", ApiNotifyError},
                     {"copyToClipboard", ApiCopyToClipboard}, {"clipboardText", ApiClipboardText}, {"isDarkMode", ApiIsDarkMode},
                     {"saveClipboardImage", ApiSaveClipboardImage}, {"copyFile", ApiCopyFile}, {"openPath", ApiOpenPath},
                     {"getColor", ApiGetColor}, {"http", ApiHttp}, {"fuzzyScore", ApiFuzzyScore},
                     {"getSetting", ApiGetSetting}, {"expandPath", ApiExpandPath}, {"togglePanel", ApiTogglePanel},
                     {"cpuCores", Nothing}, {"appIconPath", Nothing}, {"diskMounts", EmptyList}});
    OpenSystemApi(L);  // runAsync, runStream, systemStats, outputs, the wallpaper, sound (plugin-system.inl)
    OpenExtrasApi(L);  // getenv, runInTerminal, download, readFileAsync, openSettings, setWallpaperMask (plugin-extras.inl)
    SetNotYet(L, {"flatpakAppInstalled", "portalAvailable", "setWallpaperEnabled", "openColorPicker", "httpStream", "loadFont"},
              "noctalia.");
    lua_newtable(L);
    SetFunctions(L, {{"set", ApiStateSet}, {"get", ApiStateGet}, {"watch", ApiStateWatch}});
    lua_setfield(L, -2, "state");
    lua_newtable(L);
    SetFunctions(L, {{"encode", ApiJsonEncode}, {"decode", ApiJsonDecode}});
    lua_setfield(L, -2, "json");
    lua_newtable(L);
    SetFunctions(L, {{"trim", ApiTrim}, {"urlEncode", ApiUrlEncode}, {"urlDecode", ApiUrlDecode}});
    lua_setfield(L, -2, "string");
    lua_setglobal(L, "noctalia");
    OpenTourneApi(L);  // tourne.*: Windows' own extras (plugin-system.inl)
    OpenTourneExtras(L);  // tourne.processInfo, focusProcess, screenPick (plugin-extras.inl)
    lua_pushcfunction(L, ApiRequire, "require");
    lua_setglobal(L, "require");

    lua_newtable(L);  // ui
    for (const char* type : {"column", "row", "scroll", "dragSource", "dropZone", "box", "label", "markdown", "glyph", "image",
                             "separator", "spacer", "progress", "button", "graph", "input", "select", "slider", "toggle"}) {
        lua_pushstring(L, type);
        lua_pushcclosure(L, UiNode, type, 1);
        lua_setfield(L, -2, type);
    }
    lua_setglobal(L, "ui");

    if (e.kind == "panel") {
        lua_newtable(L);
        SetFunctions(L, {{"render", PanelRender}, {"close", PanelClose}, {"setWantsSecondTicks", PanelWantsTicks},
                         {"setNeedsFrameTick", WantsFrameTicks}, {"openContextMenu", PanelContextMenu}, {"insertText", PanelInsertText}});
        lua_setglobal(L, "panel");
    }
    if (e.kind == "desktop_widget") {
        lua_newtable(L);
        SetFunctions(L, {{"render", PanelRender}, {"setWantsSecondTicks", DesktopWantsTicks}, {"setNeedsFrameTick", WantsFrameTicks}});
        lua_setglobal(L, "desktopWidget");
    }
    if (e.kind == "launcher_provider") {
        lua_newtable(L);
        SetFunctions(L, {{"setResults", LauncherSetResults}, {"close", Nothing}, {"setQuery", Ignore}});
        lua_setglobal(L, "launcher");
    }
    if (e.kind == "widget") {
        lua_newtable(L);
        SetFunctions(L, {{"setText", BarSetText}, {"setGlyph", BarSetGlyph}, {"setTooltip", BarSetTooltip},
                         {"clearTooltip", BarClearTooltip}, {"setVisible", BarSetVisible}, {"setColor", BarSetColor},
                         {"isVertical", BarIsVertical}, {"render", BarRender}, {"outputName", Nothing},
                         {"setGlyphColor", BarSetGlyphColor}, {"setFont", Ignore}, {"setImage", BarSetImage}});
        lua_setglobal(L, "barWidget");
    }
}

////////////////////////////////////////////////////////////////////////////////
// Starting and stopping plugins

void* Alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    Entry& e = *(Entry*)ud;
    if (!ptr) osize = 0;
    if (nsize == 0) {
        free(ptr);
        e.memory -= std::min(e.memory, osize);
        return nullptr;
    }
    if (nsize > osize && e.memory + (nsize - osize) > kMemoryLimit) return nullptr;
    void* p = realloc(ptr, nsize);
    if (p) e.memory = e.memory - std::min(e.memory, osize) + nsize;
    return p;
}

void StopStreams(const Entry& e);
void StopSounds(const Entry& e);
void StopCaptures(const Entry& e);

// reason, for its onExit: "reload", "disable" (turned off) or "shutdown".
void Stop(Entry& e, const char* reason = "reload") {
    StopStreams(e);
    StopSounds(e);
    StopCaptures(e);
    StopPick(e);
    StopAwake(e);
    if (!e.imageShown.empty()) DeleteFileW(e.imageShown.c_str()), e.imageShown.clear();
    if (e.panel) {
        std::vector<int> refs = e.panel->Refs();
        e.panel.reset();
        ReleaseRefs(e, refs);
    }
    if (e.L) {
        CallGlobal(e, "onExit", [reason](lua_State* T) {
            lua_pushinteger(T, 0);
            lua_pushstring(T, reason);
            return 2;
        });
        lua_close(e.L);
    }
    e.L = e.T = nullptr;
    e.modules.clear();
    g_bySerial.erase(e.serial);
}

bool Start(Entry& e) {
    std::string source;
    std::wstring file = e.plugin->folder + L"\\" + W(e.file);
    for (auto& c : file)
        if (c == L'/') c = L'\\';
    if (!Inside(Full(file), e.plugin->folder) || !ReadAll(Full(file), &source)) {
        Problem(e, L"couldn't read " + W(e.file));
        return false;
    }
    e.L = lua_newstate(Alloc, &e);
    if (!e.L) return Problem(e, L"no memory for its VM"), false;
    e.serial = g_nextSerial++;
    g_bySerial[e.serial] = &e;
    int loaded = -1;
    try {
        lua_callbacks(e.L)->userdata = &e;
        lua_callbacks(e.L)->interrupt = Interrupt;
        luaL_openlibs(e.L);
        OpenApi(e.L, e);
        luaL_sandbox(e.L);
        e.T = lua_newthread(e.L);
        e.threadRef = lua_ref(e.L, -1);
        lua_pop(e.L, 1);
        luaL_sandboxthread(e.T);
        size_t size = 0;
        char* bytecode = luau_compile(source.data(), source.size(), nullptr, &size);
        std::string chunk = "=" + e.file;
        loaded = luau_load(e.T, chunk.c_str(), bytecode, size, 0);
        free(bytecode);
        if (loaded != 0) {
            const char* why = lua_tostring(e.T, -1);
            Problem(e, W(why ? why : "didn't compile"));
        }
    } catch (...) {
        Problem(e, L"its VM couldn't start");
    }
    if (loaded != 0) {
        lua_close(e.L);
        e.L = e.T = nullptr;
        g_bySerial.erase(e.serial);
        return false;
    }
    if (e.kind == "desktop_widget") OpenDesktopWidget(e);  // (before its script: it may render at once)
    if (!Call(e, 0)) return false;  // (kept: its callbacks may still work)
    // A widget's update() runs every second unless it asks for another pace.
    Guarded(e, [&] {
        lua_getglobal(e.T, "update");
        if (lua_isfunction(e.T, -1) && !e.interval && !e.paced && e.kind != "panel") e.interval = 1000, e.due = GetTickCount64() + 1000;
        lua_settop(e.T, 0);
        return true;
    });
    return true;
}

// Settings: the manifest's defaults (the plugin's own, then the entry's), then what the
// user set: plugins.<id>.<key>, or plugins.<id>.<entry>.<key> for one entry.
json::Value Settings(const Plugin& p, const json::Value& entry, const std::string& entryId) {
    json::Value out;
    out.kind = json::Value::Object;
    auto add = [&](const json::Value& list) {
        for (auto const& s : list.items) {
            const json::Value& key = s["key"];
            if (key.kind != json::Value::String) continue;
            json::Value value = s["default"];
            if (value.IsNull() && s["type"].text == "string_list") value.kind = json::Value::Array;
            bool replaced = false;
            for (auto& [k, v] : out.fields)
                if (k == key.text) v = value, replaced = true;
            if (!replaced) out.fields.push_back({key.text, value});
        }
    };
    add(p.manifest["setting"]);
    add(entry["setting"]);
    std::wstring prefix = L"plugins." + p.id + L".";
    for (auto const& [name, value] : SettingsWithPrefix(prefix)) {
        std::string rest = U(name.substr(prefix.size()));
        std::string key = rest;
        if (rest.rfind(entryId + ".", 0) == 0) key = rest.substr(entryId.size() + 1);
        else if (rest.find('.') != std::string::npos) continue;  // another entry's
        for (auto& [k, v] : out.fields) {
            if (k != key) continue;
            std::string text = U(value);
            if (v.kind == json::Value::Bool) v.boolean = text == "1" || text == "true" || text == "on" || text == "yes";
            else if (v.kind == json::Value::Number) v.number = strtod(text.c_str(), nullptr);
            else if (v.kind == json::Value::Array) {  // (a list: the Studio's row has it with commas, or a line each)
                bool numbers = !v.items.empty() && v.items[0].kind == json::Value::Number;
                v.items.clear();
                for (size_t at = 0; at <= text.size();) {
                    size_t end = text.find_first_of(",\n", at);
                    if (end == std::string::npos) end = text.size();
                    std::string item = text.substr(at, end - at);
                    at = end + 1;
                    while (!item.empty() && (item.front() == ' ' || item.front() == '\t' || item.front() == '\r')) item.erase(0, 1);
                    while (!item.empty() && (item.back() == ' ' || item.back() == '\t' || item.back() == '\r')) item.pop_back();
                    if (item.empty()) continue;
                    json::Value one;
                    if (numbers) one.kind = json::Value::Number, one.number = strtod(item.c_str(), nullptr);
                    else one.kind = json::Value::String, one.text = item;
                    v.items.push_back(std::move(one));
                }
            } else v.kind = json::Value::String, v.text = text;
        }
    }
    return out;
}

std::unique_ptr<Plugin> Load(const std::wstring& folder) {
    std::string record, manifest;
    if (!ReadAll(folder + L"\\tourne-plugin.json", &record) || !ReadAll(folder + L"\\plugin.toml", &manifest)) return nullptr;
    auto p = std::make_unique<Plugin>();
    p->folder = Full(folder);
    json::Value r;
    std::string why;
    if (!json::Parse(record, &r) || !toml::Parse(manifest, &p->manifest, &why)) {
        Log(L"%ls: couldn't read its plugin.toml or tourne-plugin.json (%hs)", folder.c_str(), why.c_str());
        return nullptr;
    }
    p->id = W(r["id"].text);
    if (p->id.empty() || W(p->manifest["id"].text) != p->id) return Log(L"%ls: its id doesn't match", folder.c_str()), nullptr;
    for (auto const& g : r["granted"].items) p->granted.insert(W(g.text));
    for (auto const& info : PluginCatalogue())
        if (info.id == p->id)
            for (auto const& n : info.needs)
                if (n.kind == L"internet" && !n.detail.empty())
                    for (size_t at = 0; at < n.detail.size();) {
                        size_t comma = n.detail.find(L", ", at);
                        p->hosts.push_back(n.detail.substr(at, comma - at));
                        at = comma == std::wstring::npos ? n.detail.size() : comma + 2;
                    }
    // Its data folder, kept apart from its files, so a reinstall keeps what it saved.
    std::wstring root = Full(folder + L"\\..\\..");
    std::wstring rel = p->folder.substr(root.size());
    p->dataDir = Full(root + L"\\..\\PluginData" + rel);
    if (GetEnvironmentVariableW(L"TOURNE_PLUGINS_DIR", nullptr, 0)) p->dataDir = Full(root + L"\\.data" + rel);
    // Its words: the user's language, then English.
    wchar_t locale[LOCALE_NAME_MAX_LENGTH] = L"en";
    GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH);
    std::wstring lang = locale, base = lang.substr(0, lang.find(L'-'));
    std::string text;
    for (auto const& name : {lang, base})
        if (text.empty() && ReadAll(p->folder + L"\\translations\\" + name + L".json", &text)) json::Parse(text, &p->strings);
    if (ReadAll(p->folder + L"\\translations\\en.json", &text)) json::Parse(text, &p->english);
    // Its services first, then its bar widgets, panels, launcher providers and the desktop widgets turned on in the
    // Studio (quick toggles don't run yet).
    for (const char* kind : {"service", "widget", "panel", "launcher_provider", "desktop_widget"}) {
        for (auto const& entry : p->manifest[kind].items) {
            if (!strcmp(kind, "desktop_widget")) {
                std::wstring on = L"plugins." + p->id + L".desktop_" + W(entry["id"].text);
                bool shown = false;
                for (auto const& [k, v] : SettingsWithPrefix(on))
                    if (k == on) shown = v == L"1" || v == L"true";
                if (!shown) continue;
            }
            auto e = std::make_unique<Entry>();
            e->plugin = p.get();
            e->kind = kind;
            e->id = entry["id"].text;
            e->file = entry["entry"].text;
            if (e->id.empty() || e->file.empty()) continue;
            e->settings = Settings(*p, entry, e->id);
            e->manifest = entry;
            p->entries.push_back(std::move(e));
        }
    }
    for (const char* kind : {"shortcut"})
        if (!p->manifest[kind].items.empty()) p->told.insert(kind);
    return p;
}

std::wstring Root() {
    wchar_t test[MAX_PATH];
    if (GetEnvironmentVariableW(L"TOURNE_PLUGINS_DIR", test, MAX_PATH)) return test;
    PWSTR local = nullptr;
    std::wstring root;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) root = std::wstring(local) + L"\\Tourne\\Plugins";
    CoTaskMemFree(local);
    return root;
}

void StopAll(const char* reason = "reload") {
    UnregisterHotkeys();
    for (auto& p : g_plugins)
        for (auto& e : p->entries) Stop(*e, reason);
    g_plugins.clear();
    g_later.clear();
    BarChanged();
}

// The installed plugins' folders (Plugins\<author>\<name>), in order.
std::vector<std::wstring> PluginFolders() {
    std::wstring root = Root();
    std::vector<std::wstring> folders;
    WIN32_FIND_DATAW author, name;
    HANDLE a = FindFirstFileW((root + L"\\*").c_str(), &author);
    if (a != INVALID_HANDLE_VALUE) {
        do {
            if (!(author.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || author.cFileName[0] == L'.') continue;
            HANDLE n = FindFirstFileW((root + L"\\" + author.cFileName + L"\\*").c_str(), &name);
            if (n == INVALID_HANDLE_VALUE) continue;
            do {
                std::wstring f = name.cFileName;
                if ((name.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && f[0] != L'.' && f.find(L".installing") == std::wstring::npos)
                    folders.push_back(root + L"\\" + author.cFileName + L"\\" + f);
            } while (FindNextFileW(n, &name));
            FindClose(n);
        } while (FindNextFileW(a, &author));
        FindClose(a);
    }
    std::sort(folders.begin(), folders.end());
    return folders;
}

// Its Studio switch ("Run it").
bool Enabled(const Plugin& p) {
    std::wstring off = L"plugins." + p.id + L".enabled";
    bool enabled = true;
    for (auto const& [k, v] : SettingsWithPrefix(off))
        if (k == off) enabled = v != L"0" && v != L"false";
    return enabled;
}

void StartAll() {
    StopAll();
    for (auto const& folder : PluginFolders()) {
        auto p = Load(folder);
        if (!p || !Enabled(*p)) continue;
        Log(L"starting %ls (%zu entries)", p->id.c_str(), p->entries.size());
        g_plugins.push_back(std::move(p));
        for (auto& e : g_plugins.back()->entries) Start(*e);
    }
    RegisterHotkeys();
    // The user's own icons for bar items, drawn again (at the screen's scale now) as they're asked for.
    for (auto& [key, swap] : g_iconSwaps)
        if (!swap.shown.empty()) DeleteFileW(swap.shown.c_str());
    g_iconSwaps.clear();
    ReadIconSwaps();
    BarChanged();
}

bool SameJson(const json::Value& a, const json::Value& b) {
    if (a.kind != b.kind || a.boolean != b.boolean || a.number != b.number || a.text != b.text || a.items.size() != b.items.size() ||
        a.fields.size() != b.fields.size())
        return false;
    for (size_t i = 0; i < a.items.size(); i++)
        if (!SameJson(a.items[i], b.items[i])) return false;
    for (size_t i = 0; i < a.fields.size(); i++)
        if (a.fields[i].first != b.fields[i].first || !SameJson(a.fields[i].second, b.fields[i].second)) return false;
    return true;
}

// Plugins' settings changed (the Studio, tourne-gui.conf). As in Noctalia, a running entry with onConfigChanged
// keeps running (a recording, a download, a setup carry on) and is told, getConfig now answering the new
// values; an entry without it starts again with them. A plugin turned on or off starts or stops; a desktop
// widget switched on or off starts or stops alone. Everything else is left alone.
void ApplySettings() {
    UnregisterHotkeys();
    std::vector<std::unique_ptr<Plugin>> next;
    std::vector<Entry*> told, starting, enabled;
    for (auto const& folder : PluginFolders()) {
        auto fresh = Load(folder);
        if (!fresh || !Enabled(*fresh)) continue;
        auto old = std::find_if(g_plugins.begin(), g_plugins.end(),
                                [&](const std::unique_ptr<Plugin>& p) { return p && p->folder == fresh->folder; });
        if (old == g_plugins.end()) {  // (just turned on: its services hear onEnable)
            Log(L"starting %ls (%zu entries)", fresh->id.c_str(), fresh->entries.size());
            for (auto& e : fresh->entries) starting.push_back(e.get());
            for (auto& e : fresh->entries)
                if (e->kind == "service") enabled.push_back(e.get());
            next.push_back(std::move(fresh));
            continue;
        }
        Plugin& p = **old;
        std::vector<std::unique_ptr<Entry>> entries;  // (in the fresh order)
        for (auto& f : fresh->entries) {
            auto was = std::find_if(p.entries.begin(), p.entries.end(),
                                    [&](const std::unique_ptr<Entry>& e) { return e && e->kind == f->kind && e->id == f->id; });
            if (was == p.entries.end()) {  // (a desktop widget just switched on)
                f->plugin = &p;
                starting.push_back(f.get());
                entries.push_back(std::move(f));
                continue;
            }
            Entry& e = **was;
            if (SameJson(e.settings, f->settings)) {
                entries.push_back(std::move(*was));
                continue;
            }
            bool listens = e.L && Guarded(e, [&] {
                lua_getglobal(e.T, "onConfigChanged");
                bool is = lua_isfunction(e.T, -1);
                lua_settop(e.T, 0);
                return is;
            });
            if (listens) {
                Log(L"%ls:%hs: settings changed, told (onConfigChanged)", p.id.c_str(), e.id.c_str());
                e.settings = f->settings;
                told.push_back(&e);
                entries.push_back(std::move(*was));
                continue;
            }
            // Started again, as a new entry with the new settings (nothing of the old VM's kept).
            Log(L"%ls:%hs: settings changed, started again with them", p.id.c_str(), e.id.c_str());
            Stop(e);
            was->reset();
            f->plugin = &p;
            starting.push_back(f.get());
            entries.push_back(std::move(f));
        }
        for (auto& e : p.entries)  // (a desktop widget switched off)
            if (e) Stop(*e, "disable");
        p.entries = std::move(entries);
        next.push_back(std::move(*old));
    }
    for (auto& p : g_plugins)  // (turned off, or gone)
        if (p) {
            Log(L"stopping %ls", p->id.c_str());
            for (auto& e : p->entries) Stop(*e, "disable");
        }
    g_plugins = std::move(next);
    for (Entry* e : starting) Start(*e);
    for (Entry* e : enabled) CallGlobal(*e, "onEnable");
    for (Entry* e : told) CallGlobal(*e, "onConfigChanged");
    RegisterHotkeys();
    BarChanged();
}

////////////////////////////////////////////////////////////////////////////////
// The bar

std::vector<PluginBarItem> BarItems() {
    std::vector<PluginBarItem> items;
    for (auto const& p : g_plugins)
        for (auto const& e : p->entries)
            if (e->kind == "widget" && e->L && e->visible && (!e->glyph.empty() || !e->text.empty() || !e->imageShown.empty())) {
                // (the user's own icon, when they chose one, in place of its glyph or picture)
                std::wstring own = IconSwapFor(*e, Name(*e));
                items.push_back({Name(*e), e->glyph, e->text, e->tip, e->color != CLR_INVALID ? e->color : RoleColor("on_surface"),
                                 own.empty() ? e->imageShown : own, e->glyphColor});
            }
    return items;
}

BOOL CALLBACK TellBar(HWND w, LPARAM) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    wchar_t name[64];
    if (pid == GetCurrentProcessId() && GetClassNameW(w, name, 64) && !wcsncmp(name, L"TourneTaskbarTest_", 18))
        PostMessageW(w, g_msgBarChanged, 0, 0);
    return TRUE;
}

void Report() {
    std::vector<LauncherProvider> providers;
    for (auto const& p : g_plugins)
        for (auto const& e : p->entries) {
            if (e->kind != "launcher_provider" || !e->L) continue;
            LauncherProvider lp;
            lp.key = Name(*e);
            lp.name = W(p->manifest["name"].text);
            lp.prefix = W(e->manifest["prefix"].text);
            lp.glyph = W(e->manifest["glyph"].text);
            if (lp.glyph.empty()) lp.glyph = W(p->manifest["icon"].text);
            lp.global = e->manifest["include_in_global_search"].boolean;
            providers.push_back(lp);
        }
    {
        std::lock_guard lock(g_lock);
        g_launcherProviders = std::move(providers);
    }
    std::vector<std::wstring> lines;
    for (auto const& p : g_plugins)
        for (auto const& e : p->entries) {
            std::wstring line = Name(*e) + L" (" + W(e->kind) + L")";
            if (!e->L) line += L" not running";
            if (e->kind == "widget") {
                wchar_t g[16] = L"";
                if (!e->glyph.empty()) swprintf(g, 16, L"U+%04X ", (unsigned)e->glyph[0]);
                line += L" shows [" + std::wstring(g) + e->text + L"]";
                if (!e->tip.empty()) line += L" tip [" + e->tip + L"]";
            }
            if (!e->lastError.empty()) line += L" last error: " + e->lastError;
            lines.push_back(line);
        }
    std::lock_guard lock(g_lock);
    g_reportLines = std::move(lines);
}

void Event(const std::wstring& key, PluginBarEvent event) {
    for (auto& p : g_plugins)
        for (auto& e : p->entries) {
            if (Name(*e) != key || e->kind != "widget") continue;
            Entry* entry = e.get();
            if (event == kPluginClick) CallGlobal(*entry, "onClick");
            else if (event == kPluginRightClick) CallGlobal(*entry, "onRightClick");
            else if (event == kPluginMiddleClick) CallGlobal(*entry, "onMiddleClick");
            else if (event == kPluginEnter || event == kPluginLeave)
                CallGlobal(*entry, "onHover", [&](lua_State* T) { return lua_pushboolean(T, event == kPluginEnter), 1; });
            else if (event == kPluginScrollUp || event == kPluginScrollDown)
                CallGlobal(*entry, "onScroll", [&](lua_State* T) {
                    lua_pushstring(T, "vertical");
                    lua_pushinteger(T, event == kPluginScrollUp ? -1 : 1);
                    lua_pushboolean(T, true);
                    return 3;
                });
        }
}

// The items as packed for the bars. The version changes only when which items
// there are (or their order) does, so a click on one still finds it while its
// text ticks along.
std::wstring PackedItems(bool* changed = nullptr) {
    auto items = BarItems();
    std::vector<std::wstring> keys;
    for (auto const& item : items) keys.push_back(item.key);
    if (keys != g_barKeys) {
        g_barKeys = std::move(keys);
        g_barVersion++;
    }
    std::wstring packed = PackPluginBarItems(items, g_barVersion);
    if (changed) *changed = packed != g_lastPacked;
    g_lastPacked = packed;
    return packed;
}

UINT g_msgBarAsk = 0, g_msgBarEvent = 0;

// A message for plugins, as Noctalia's `noctalia msg` sends them (TourneDesktop.exe --msg ...): onIpc(event, payload)
// in the entries of "author/plugin:entry" (every entry of the plugin with no ":entry"), or a panel toggled.
constexpr ULONG_PTR kPluginMessageId = 0x54504D53;  // "TPMS"

void Ipc(const std::vector<std::wstring>& words) {
    if (words.size() >= 2 && words[0] == L"panel-toggle") return TogglePanel(words[1]);
    if (words.size() < 4 || words[0] != L"plugin") return Log(L"a message for plugins it doesn't know: %ls", words.empty() ? L"" : words[0].c_str());
    std::wstring target = words[1], plugin = target.substr(0, target.find(L':'));
    std::wstring entry = target.find(L':') == std::wstring::npos ? L"" : target.substr(target.find(L':') + 1);
    std::string event = U(words[3]), payload;
    for (size_t i = 4; i < words.size(); i++) payload += (i > 4 ? " " : "") + U(words[i]);
    bool any = false;
    for (auto& p : g_plugins)
        for (auto& e : p->entries) {
            if (p->id != plugin || (!entry.empty() && W(e->id) != entry) || !e->L) continue;
            any = true;
            CallGlobal(*e, "onIpc", [&](lua_State* T) {
                lua_pushlstring(T, event.data(), event.size());
                if (words.size() > 4) lua_pushlstring(T, payload.data(), payload.size());
                else lua_pushnil(T);
                return 2;
            });
        }
    if (!any) Log(L"no running plugin entry %ls for a message", target.c_str());
}

LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == g_msgBarAsk && g_msgBarAsk) {
        // A bar asks, or System Icons (the items in its overflow flyout); with kPluginAskIconsChanged, after the
        // user changed an item's icon.
        HWND asker = (HWND)wp;
        wchar_t name[64];
        if (!IsWindow(asker) || !GetClassNameW(asker, name, 64) ||
            (wcsncmp(name, L"TourneTaskbarTest_", 18) && wcsncmp(name, L"TourneTraySystemIcons_", 22)))
            return 0;
        if (lp == kPluginAskIconsChanged) ReadIconSwaps();
        std::wstring packed = PackedItems();
        COPYDATASTRUCT reply{kPluginBarItemsId, (DWORD)((packed.size() + 1) * sizeof(wchar_t)), (PVOID)packed.c_str()};
        DWORD_PTR result = 0;
        SendMessageTimeoutW(asker, WM_COPYDATA, (WPARAM)wnd, (LPARAM)&reply, SMTO_ABORTIFHUNG, 1000, &result);
        return 0;
    }
    if (msg == g_msgBarEvent && g_msgBarEvent) {
        size_t index = (size_t)(lp & 0xFFFF);
        int event = (int)(lp >> 16 & 0xFF);
        if ((unsigned)wp == g_barVersion && index < g_barKeys.size() && event <= kPluginLeave) {
            std::wstring key = g_barKeys[index];
            Later([key, event] { Event(key, (PluginBarEvent)event); });
        }
        return 0;
    }
    if (msg == WM_COPYDATA) {
        auto* data = (const COPYDATASTRUCT*)lp;
        if (!data || data->dwData != kPluginMessageId || !data->lpData) return FALSE;
        std::wstring text((const wchar_t*)data->lpData, wcsnlen((const wchar_t*)data->lpData, data->cbData / sizeof(wchar_t)));
        std::vector<std::wstring> words;
        for (size_t at = 0; at <= text.size();) {
            size_t cut = text.find(L'\x1F', at);
            if (cut == std::wstring::npos) cut = text.size();
            words.push_back(text.substr(at, cut - at));
            at = cut + 1;
        }
        Later([words] { Ipc(words); });
        return TRUE;
    }
    if (msg == WM_TIMER && wp == 1) {
        KillTimer(wnd, 1);
        NOTIFYICONDATAW n{sizeof(n)};
        n.hWnd = wnd;
        n.uID = 1;
        Shell_NotifyIconW(NIM_DELETE, &n);
        return 0;
    }
    if (msg == kMsgReload) {
        StartAll();
        return 0;
    }
    if (msg == kMsgSettings) {
        ApplySettings();
        return 0;
    }
    // A bind's "plugin" dispatcher (binds.cpp): a global atom of "author/plugin:panel" in wParam.
    static UINT panelAsked = RegisterWindowMessageW(L"TourneDesktop.PluginPanel");
    if (msg == panelAsked && panelAsked) {
        wchar_t id[256] = L"";
        GlobalGetAtomNameW((ATOM)wp, id, 256);
        GlobalDeleteAtom((ATOM)wp);
        std::wstring panel = id;
        if (!panel.empty()) Later([panel] { TogglePanel(panel); });
        return 0;
    }
    if (msg == WM_HOTKEY) {
        if (auto it = g_hotkeys.find((int)wp); it != g_hotkeys.end()) {
            std::wstring id = it->second;
            Later([id] { TogglePanel(id); });
        }
        return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

DWORD WINAPI RuntimeThread(LPVOID started) {
    // COM, for pictures (WIC) and opening files. (Never uninitialised: a panel keeps its picture reader.)
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kPluginHostClass;
    RegisterClassW(&wc);
    g_window = CreateWindowExW(0, kPluginHostClass, L"Tourne Desktop plugins", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
    g_msgBarChanged = RegisterWindowMessageW(kPluginBarChangedMessage);
    g_msgBarAsk = RegisterWindowMessageW(kPluginBarAskMessage);
    g_msgBarEvent = RegisterWindowMessageW(kPluginBarEventMessage);
    SetEvent((HANDLE)started);
    LoadGlyphFont();
    panel::SetGlyphLookup([](const std::string& name) { return GlyphChar(name); });
    PluginRefreshBundled();
    StartAll();
    for (;;) {
        // What came back from other threads, then whatever is due.
        std::deque<std::function<void()>> inbox;
        {
            std::lock_guard lock(g_lock);
            inbox.swap(g_inbox);
        }
        for (auto& work : inbox) work();
        ULONGLONG now = GetTickCount64();
        for (size_t pi = 0; pi < g_plugins.size(); pi++)
            for (size_t ei = 0; ei < g_plugins[pi]->entries.size(); ei++) {
                Entry& e = *g_plugins[pi]->entries[ei];
                if (!e.L || !e.interval || e.due > now) continue;
                e.due = now + e.interval;
                CallGlobal(e, "update");
            }
        // Frames, for those that asked (an open panel, a desktop widget).
        for (size_t pi = 0; pi < g_plugins.size(); pi++)
            for (size_t ei = 0; ei < g_plugins[pi]->entries.size(); ei++) {
                Entry& e = *g_plugins[pi]->entries[ei];
                if (!e.L || !e.frameTicks || !e.panel || now - e.lastFrame < 16) continue;
                double delta = e.lastFrame ? (double)(now - e.lastFrame) : 16.0;
                e.lastFrame = now;
                CallGlobal(e, "onFrameTick", [delta](lua_State* T) { return lua_pushnumber(T, delta), 1; });
            }
        CheckBarPictures();
        // State changes reach their watchers (which may change more, up to a point).
        for (int rounds = 0; !g_later.empty() && rounds < 1000; rounds++) {
            auto work = std::move(g_later.front());
            g_later.pop_front();
            work();
        }
        if (g_barDirty) {
            g_barDirty = false;
            bool changed = false;
            PackedItems(&changed);
            if (changed) EnumWindows(TellBar, 0);
        }
        Report();
        DWORD wait = INFINITE;
        now = GetTickCount64();
        for (auto const& p : g_plugins)
            for (auto const& e : p->entries)
                if (e->L && e->interval) wait = std::min<DWORD>(wait, e->due > now ? (DWORD)(e->due - now) : 0);
        for (auto const& p : g_plugins)
            for (auto const& e : p->entries)
                if (e->L && e->frameTicks && e->panel) wait = std::min<DWORD>(wait, 16);
        if (WatchingBarPictures()) wait = std::min<DWORD>(wait, 1000);
        if (!g_later.empty()) wait = 0;
        MsgWaitForMultipleObjectsEx(0, nullptr, wait, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        MSG m;
        while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) {
            if (m.message == WM_QUIT) {
                StopAll("shutdown");
                DestroyWindow(g_window);
                g_window = nullptr;
                return 0;
            }
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
    }
}

}  // namespace

void PluginRuntimeStart() {
    if (g_thread) return;
    HANDLE started = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, RuntimeThread, started, 0, &g_threadId);
    if (g_thread) WaitForSingleObject(started, 5000);
    CloseHandle(started);
}

void PluginRuntimeStop() {
    if (!g_thread) return;
    PostThreadMessageW(g_threadId, WM_QUIT, 0, 0);
    if (WaitForSingleObject(g_thread, 3000) == WAIT_TIMEOUT) Log(L"the plugin runtime didn't stop in time");
    CloseHandle(g_thread);
    g_thread = nullptr;
}

void PluginRuntimeSettingsChanged() {
    if (g_window) PostMessageW(g_window, kMsgSettings, 0, 0);
}

void PluginRuntimeReload() {
    if (g_window) PostMessageW(g_window, kMsgReload, 0, 0);
}

std::vector<LauncherProvider> LauncherProviders() {
    std::lock_guard lock(g_lock);
    return g_launcherProviders;
}

void LauncherSetListener(HWND wnd, UINT msg) {
    std::lock_guard lock(g_lock);
    g_launcherWnd = wnd;
    g_launcherMsg = msg;
}

void LauncherQuery(const std::wstring& key, const std::wstring& text) {
    Post([key, text] {
        for (auto& p : g_plugins)
            for (auto& e : p->entries)
                if (e->kind == "launcher_provider" && e->L && Name(*e) == key) {
                    std::string t = U(text);
                    CallGlobal(*e, "onQuery", [&](lua_State* T) { return lua_pushlstring(T, t.data(), t.size()), 1; });
                }
    });
}

std::vector<LauncherItem> LauncherResults(const std::wstring& key, const std::wstring& text) {
    std::lock_guard lock(g_lock);
    auto it = g_launcherAnswers.find(key);
    if (it == g_launcherAnswers.end() || it->second.text != text) return {};
    return it->second.items;
}

void LauncherActivate(const std::wstring& key, const std::wstring& id) {
    Post([key, id] {
        for (auto& p : g_plugins)
            for (auto& e : p->entries)
                if (e->kind == "launcher_provider" && e->L && Name(*e) == key) {
                    std::string i = U(id);
                    CallGlobal(*e, "onActivate", [&](lua_State* T) { return lua_pushlstring(T, i.data(), i.size()), 1; });
                }
    });
}

std::wstring LauncherGlyph(const std::wstring& name) { return GlyphChar(U(name)); }

int PluginMessage(const std::vector<std::wstring>& words) {
    HWND host = FindWindowExW(HWND_MESSAGE, nullptr, kPluginHostClass, nullptr);
    if (!host || words.empty()) return 1;
    std::wstring text;
    for (size_t i = 0; i < words.size(); i++) text += (i ? L"\x1F" : L"") + words[i];
    COPYDATASTRUCT data{kPluginMessageId, (DWORD)((text.size() + 1) * sizeof(wchar_t)), (PVOID)text.c_str()};
    DWORD_PTR result = 0;
    return SendMessageTimeoutW(host, WM_COPYDATA, 0, (LPARAM)&data, SMTO_ABORTIFHUNG, 3000, &result) && result ? 0 : 1;
}

std::vector<std::wstring> PluginRuntimeReport() {
    std::lock_guard lock(g_lock);
    std::vector<std::wstring> out = g_reportLines;
    for (auto const& l : g_logLines) out.push_back(L"  log: " + l);
    return out;
}

}  // namespace td
