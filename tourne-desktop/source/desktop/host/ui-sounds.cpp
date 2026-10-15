// UI sounds: see ui-sounds.h.
//
// The cues are WAV files built into the program (ui-sounds.inc, made by Build-TourneDesktop.ps1 from
// desktop/sounds/foley). Each is copied once at the volume set (16-bit samples scaled), kept for as long as it
// may play (PlaySound reads it while it plays), and played with PlaySound(SND_MEMORY | SND_ASYNC) on the sound
// thread, so the thread asking never waits. A new sound cuts the one before off, which suits UI cues.
//
// Menus are heard through Windows' accessibility events (SetWinEventHook, out of context, this process only): a
// menu popping up (EVENT_SYSTEM_MENUPOPUPSTART) and an item getting the focus as the pointer or arrows move over
// it (EVENT_OBJECT_FOCUS on a "#32768" menu window). These are posted events, not hooks in the input path, so they
// can't hold up the pointer.

#include "ui-sounds.h"

#include <mmsystem.h>

#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>

#include "td.h"

namespace td {

namespace {

struct BuiltInSound {
    const char* name;
    const unsigned char* data;
    size_t size;
};
#include "ui-sounds.inc"

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"sounds", format, args);
    va_end(args);
}

// The moments with a cue of their own: setting, name, default, description.
struct Moment {
    const wchar_t* key;
    const wchar_t* name;
    const wchar_t* fallback;
    const wchar_t* description;
};
constexpr Moment kMoments[] = {
    {L"menuOpen", L"A menu opens", L"pop", L"Tourne Desktop's own menus: the desktop's, the bar's, the tray's."},
    {L"menuHover", L"Moving over a menu item", L"none", L"As the pointer or the arrow keys move from item to item."},
    {L"launcher", L"The launcher opens", L"swoosh", L"The search box (Alt+Space unless you changed it)."},
};

struct Config {
    bool on = false;
    int volume = 40;
    bool plugins = true;
    std::map<std::wstring, std::wstring> cues;  // moment: cue name, or "none"
};

std::mutex g_lock;
Config g_cfg;  // under g_lock
std::map<std::string, std::vector<BYTE>> g_scaled;  // cue: its WAV at g_scaledAt's volume (sound thread only)
int g_scaledAt = -1;
std::thread g_thread;
std::atomic<DWORD> g_threadId{0};
HWINEVENTHOOK g_menuHook = nullptr, g_focusHook = nullptr;
ULONGLONG g_lastHover = 0;
constexpr UINT kMsgPlay = WM_APP + 1, kMsgApply = WM_APP + 2;

Config ReadConfig() {
    Config c;
    c.on = GetInt(L"sounds", L"enabled") != 0;
    c.volume = std::clamp(GetInt(L"sounds", L"volume"), 0, 100);
    c.plugins = GetInt(L"sounds", L"plugins") != 0;
    for (auto const& m : kMoments) {
        std::wstring v = m.fallback;
        if (const wchar_t* s = GetString(L"sounds", m.key)) {
            v = s;
            FreeString(s);
        }
        c.cues[m.key] = v;
    }
    return c;
}

BuiltInSound const* Find(const std::string& name) {
    for (auto const& s : kUiSounds)
        if (name == s.name) return &s;
    return nullptr;
}

// The cue at the volume set: its bytes with every 16-bit sample in its data chunk scaled.
std::vector<BYTE> const* Scaled(const std::string& name, int volume) {
    if (volume != g_scaledAt) g_scaled.clear(), g_scaledAt = volume;
    if (auto it = g_scaled.find(name); it != g_scaled.end()) return &it->second;
    BuiltInSound const* s = Find(name);
    if (!s) return nullptr;
    std::vector<BYTE> wav(s->data, s->data + s->size);
    // (find "fmt " for the sample size and "data" for the samples)
    int bits = 0;
    for (size_t at = 12; at + 8 <= wav.size();) {
        uint32_t size;
        memcpy(&size, &wav[at + 4], 4);
        size_t body = at + 8, end = std::min(wav.size(), body + (size_t)size);
        if (!memcmp(&wav[at], "fmt ", 4) && size >= 16) bits = wav[body + 14] | (wav[body + 15] << 8);
        if (!memcmp(&wav[at], "data", 4) && bits == 16) {
            for (size_t i = body; i + 1 < end; i += 2) {
                int16_t v;
                memcpy(&v, &wav[i], 2);
                v = (int16_t)std::clamp((int)v * volume / 100, -32768, 32767);
                memcpy(&wav[i], &v, 2);
            }
        }
        at = body + size + (size & 1);
    }
    return &(g_scaled[name] = std::move(wav));
}

void Play(const std::string& name) {
    int volume;
    {
        std::lock_guard lock(g_lock);
        if (!g_cfg.on) return;
        volume = g_cfg.volume;
    }
    if (volume <= 0) return;
    if (auto const* wav = Scaled(name, volume)) PlaySoundW((LPCWSTR)wav->data(), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

std::wstring CueFor(const wchar_t* moment) {
    std::lock_guard lock(g_lock);
    if (!g_cfg.on) return L"";
    auto it = g_cfg.cues.find(moment);
    return it == g_cfg.cues.end() || it->second == L"none" ? L"" : it->second;
}

void CALLBACK MenuEvent(HWINEVENTHOOK, DWORD event, HWND wnd, LONG, LONG, DWORD, DWORD) {
    if (event == EVENT_SYSTEM_MENUPOPUPSTART) {
        if (std::wstring cue = CueFor(L"menuOpen"); !cue.empty()) Play(WideToUtf8(cue));
        g_lastHover = GetTickCount64();  // (the first item's focus comes with it)
        return;
    }
    wchar_t cls[16] = L"";
    if (!wnd || !GetClassNameW(wnd, cls, 16) || wcscmp(cls, L"#32768")) return;
    ULONGLONG now = GetTickCount64();
    if (now - g_lastHover < 40) return;  // (a quick sweep over the items isn't a drum roll)
    g_lastHover = now;
    if (std::wstring cue = CueFor(L"menuHover"); !cue.empty()) Play(WideToUtf8(cue));
}

void Hooks(bool on) {
    if (on && !g_menuHook) {
        DWORD pid = GetCurrentProcessId();
        g_menuHook = SetWinEventHook(EVENT_SYSTEM_MENUPOPUPSTART, EVENT_SYSTEM_MENUPOPUPSTART, nullptr, MenuEvent, pid, 0,
                                     WINEVENT_OUTOFCONTEXT);
        g_focusHook = SetWinEventHook(EVENT_OBJECT_FOCUS, EVENT_OBJECT_FOCUS, nullptr, MenuEvent, pid, 0, WINEVENT_OUTOFCONTEXT);
    } else if (!on && g_menuHook) {
        UnhookWinEvent(g_menuHook), g_menuHook = nullptr;
        if (g_focusHook) UnhookWinEvent(g_focusHook), g_focusHook = nullptr;
    }
}

void Apply() {
    Config c = ReadConfig();
    bool on = c.on;
    {
        std::lock_guard lock(g_lock);
        g_cfg = std::move(c);
    }
    Hooks(on);
    if (!on) PlaySoundW(nullptr, nullptr, 0);
    Log(on ? L"on" : L"off");
}

void Thread() {
    MSG msg;
    PeekMessageW(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);  // (its queue, before anyone posts to it)
    g_threadId = GetCurrentThreadId();
    Apply();
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (msg.message == kMsgPlay) {
            auto* name = (std::string*)msg.lParam;
            Play(*name);
            delete name;
        } else if (msg.message == kMsgApply) {
            Apply();
        } else {
            DispatchMessageW(&msg);
        }
    }
    Hooks(false);
    PlaySoundW(nullptr, nullptr, 0);
    g_threadId = 0;
}

}  // namespace

std::vector<std::wstring> UiSoundNames() {
    std::vector<std::wstring> names;
    for (auto const& s : kUiSounds) names.push_back(Utf8ToWide(s.name));
    return names;
}

void UiSound(const std::wstring& name) {
    DWORD id = g_threadId;
    if (!id || name.empty() || name == L"none") return;
    {
        std::lock_guard lock(g_lock);
        if (!g_cfg.on) return;
    }
    auto* s = new std::string(WideToUtf8(name));
    if (!PostThreadMessageW(id, kMsgPlay, 0, (LPARAM)s)) delete s;
}

void UiSoundFor(const wchar_t* moment) { UiSound(CueFor(moment)); }

bool UiSoundsForPlugins() {
    std::lock_guard lock(g_lock);
    return g_cfg.on && g_cfg.plugins;
}

void UiSoundsDefaults(Settings& out) {
    out[L"sounds.enabled"] = L"0";
    out[L"sounds.volume"] = L"40";
    out[L"sounds.plugins"] = L"1";
    for (auto const& m : kMoments) out[std::wstring(L"sounds.") + m.key] = m.fallback;
}

void UiSoundsDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"sounds", L"Sounds",
        L"Short sounds for Tourne Desktop's own moments: menus, the launcher, and plugins that ask (the power menu, "
        L"Caffeine). The cues are Foley's (usefoley.dev, MIT), built in; nothing is downloaded.");
    add(L"sounds.enabled", L"UI sounds", L"Off: Tourne Desktop makes no sounds of its own.").isSwitch = true;
    SettingInfo& volume = add(L"sounds.volume", L"Volume", L"0 to 100, on top of Windows' volume for Tourne Desktop.");
    volume.hasMin = volume.hasMax = true;
    volume.min = 0, volume.max = 100;
    add(L"sounds.plugins", L"Plugins may play them", L"Plugins' own cues (tourne.uiSound), as long as UI sounds are on.").isSwitch =
        true;
    std::vector<std::pair<std::wstring, std::wstring>> options = {{L"none", L"None"}};
    for (auto const& n : UiSoundNames()) {
        std::wstring label = n;
        if (!label.empty()) label[0] = (wchar_t)towupper(label[0]);
        options.push_back({n, label});
    }
    for (auto const& m : kMoments) add(std::wstring(L"sounds.") + m.key, m.name, m.description).options = options;
}

void UiSoundsStart(HWND) {
    if (g_thread.joinable()) return;
    g_thread = std::thread(Thread);
}

void UiSoundsApply() {
    if (DWORD id = g_threadId) PostThreadMessageW(id, kMsgApply, 0, 0);
}

void UiSoundsStop() {
    if (!g_thread.joinable()) return;
    for (int i = 0; i < 100 && !g_threadId; i++) Sleep(10);
    if (DWORD id = g_threadId) PostThreadMessageW(id, WM_QUIT, 0, 0);
    g_thread.join();
}

}  // namespace td
