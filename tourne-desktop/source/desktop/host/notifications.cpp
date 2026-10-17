// Notifications from Tourne Studio: see notifications.h.
//
// Windows keeps one key per app that has notified (its AppUserModelID, or a path for some desktop apps),
// with values only where they differ from its defaults: Enabled (0: off), ShowBanner (0: no pop-ups),
// ShowInActionCenter (0: not in the notification centre), Rank (0 normal, 1 high, 99 top). The global ones
// sit beside them: NOC_GLOBAL_SETTING_TOASTS_ENABLED, _ALLOW_NOTIFICATION_SOUND, _ALLOW_TOASTS_ABOVE_LOCK.
//
// The Studio's defaults are what Windows has when Tourne Desktop starts, and only what you set (IsSet) is
// written back, so a change made in Windows' Settings later isn't undone.
#include "notifications.h"

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include "td.h"

namespace td {

namespace {

constexpr wchar_t kKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings";
constexpr wchar_t kOurKey[] = L"Software\\Tourne\\Desktop";
constexpr wchar_t kQuietValue[] = L"QuietHoursPaused";  // 1 while quiet hours turned notifications off
constexpr wchar_t kBannersOffValue[] = L"BannersTurnedOff";  // the apps whose Windows pop-ups Tourne turned off

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"notify", format, args);
    va_end(args);
}

struct App {
    std::wstring id;    // Windows' key name
    std::wstring key;   // ours: notify.apps.<key>
    std::wstring name;  // as people read it
};

DWORD ReadDword(const std::wstring& key, const wchar_t* value, DWORD fallback) {
    DWORD v = fallback, size = sizeof(v);
    return RegGetValueW(HKEY_CURRENT_USER, key.c_str(), value, RRF_RT_REG_DWORD, nullptr, &v, &size) == ERROR_SUCCESS ? v : fallback;
}

bool WriteDword(const std::wstring& key, const wchar_t* value, DWORD v) {
    return RegSetKeyValueW(HKEY_CURRENT_USER, key.c_str(), value, REG_DWORD, &v, sizeof(v)) == ERROR_SUCCESS;
}

std::wstring KeyFor(const std::wstring& id) {
    std::wstring k;
    for (wchar_t c : id) k += iswalnum(c) ? (wchar_t)towlower(c) : L'_';
    return k;
}

// The name the Start menu shows for it, else a readable part of its id.
std::wstring NameFor(const std::wstring& id) {
    IShellItem* item = nullptr;
    std::wstring name;
    if (SUCCEEDED(SHCreateItemFromParsingName((L"shell:AppsFolder\\" + id).c_str(), nullptr, IID_PPV_ARGS(&item)))) {
        PWSTR text = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &text)) && text) name = text;
        CoTaskMemFree(text);
        item->Release();
    }
    if (!name.empty() && name != id) return name;
    constexpr wchar_t kSystem[] = L"Windows.SystemToast.";
    if (id.rfind(kSystem, 0) == 0) return L"Windows: " + id.substr(wcslen(kSystem));
    size_t slash = id.find_last_of(L"\\/");
    if (slash != std::wstring::npos) return id.substr(slash + 1);
    size_t bang = id.find(L'!');
    std::wstring s = bang != std::wstring::npos ? id.substr(bang + 1) : id;
    size_t dot = s.find_last_of(L'.');
    return dot != std::wstring::npos && dot + 1 < s.size() ? s.substr(dot + 1) : s;
}

// Every app Windows keeps notification settings for, by name.
std::vector<App> ReadApps() {
    std::vector<App> apps;
    HKEY root;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kKey, 0, KEY_READ, &root) != ERROR_SUCCESS) return apps;
    wchar_t name[512];
    for (DWORD i = 0;; i++) {
        DWORD length = ARRAYSIZE(name);
        if (RegEnumKeyExW(root, i, name, &length, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        apps.push_back({name, KeyFor(name), L""});
    }
    RegCloseKey(root);
    HRESULT co = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    for (auto& a : apps) a.name = NameFor(a.id);
    if (SUCCEEDED(co)) CoUninitialize();
    std::sort(apps.begin(), apps.end(), [](const App& a, const App& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
    return apps;
}

std::mutex g_appsLock;
std::vector<App> g_apps;  // as read for the defaults

////////////////////////////////////////////////////////////////////////////////
// Tourne's own pop-ups (notify.popups = tourne): System Icons draws them, so Windows' are turned off app by
// app (ShowBanner = 0; still in the notification centre), and those apps are kept in a list to put back
// as they were. Each app's own "Pop-ups" setting then decides Tourne's.

std::vector<std::wstring> ReadList(const wchar_t* value) {
    std::vector<std::wstring> out;
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kOurKey, value, RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &bytes) != ERROR_SUCCESS || !bytes)
        return out;
    std::vector<wchar_t> list(bytes / sizeof(wchar_t) + 2, 0);
    bytes = (DWORD)(list.size() * sizeof(wchar_t));
    if (RegGetValueW(HKEY_CURRENT_USER, kOurKey, value, RRF_RT_REG_MULTI_SZ, nullptr, list.data(), &bytes) != ERROR_SUCCESS) return out;
    for (const wchar_t* p = list.data(); *p; p += wcslen(p) + 1) out.push_back(p);
    return out;
}

void WriteList(const wchar_t* value, const std::vector<std::wstring>& list) {
    if (list.empty()) {
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kOurKey, value);
        return;
    }
    std::wstring multi;
    for (auto const& s : list) multi += s + L'\0';
    multi += L'\0';
    RegSetKeyValueW(HKEY_CURRENT_USER, kOurKey, value, REG_MULTI_SZ, multi.data(), (DWORD)(multi.size() * sizeof(wchar_t)));
}

bool Has(const std::vector<std::wstring>& list, const std::wstring& s) {
    for (auto const& x : list)
        if (!_wcsicmp(x.c_str(), s.c_str())) return true;
    return false;
}

bool TournePopups() {
    const wchar_t* s = GetString(L"notify", L"popups");
    bool tourne = s && !wcscmp(s, L"tourne");
    FreeString(s);
    return tourne;
}

std::mutex g_bannersLock;

// Every app's Windows pop-ups off (those that had them on remembered), or the remembered ones back on.
void ApplyBanners() {
    std::lock_guard<std::mutex> lock(g_bannersLock);
    std::vector<std::wstring> off = ReadList(kBannersOffValue);
    std::wstring k = kKey;
    if (TournePopups()) {
        HKEY root;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kKey, 0, KEY_READ, &root) != ERROR_SUCCESS) return;
        wchar_t name[512];
        bool changed = false;
        for (DWORD i = 0;; i++) {
            DWORD length = ARRAYSIZE(name);
            if (RegEnumKeyExW(root, i, name, &length, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
            std::wstring at = k + L"\\" + name;
            if (ReadDword(at, L"ShowBanner", 1) == 0) continue;  // (off already: yours, or Tourne's)
            WriteDword(at, L"ShowBanner", 0);
            if (!Has(off, name)) off.push_back(name);
            changed = true;
        }
        RegCloseKey(root);
        if (changed) {
            WriteList(kBannersOffValue, off);
            Log(L"Windows' pop-ups off for %zu apps (Tourne draws them)", off.size());
        }
    } else if (!off.empty()) {
        for (auto const& id : off) {
            std::wstring at = k + L"\\" + id;
            // (as you set it in the Studio, else as it was: on)
            bool want = !IsSet(L"notify", L"apps." + KeyFor(id) + L".banners") || GetInt(L"notify", L"apps." + KeyFor(id) + L".banners");
            if (want) RegDeleteKeyValueW(HKEY_CURRENT_USER, at.c_str(), L"ShowBanner");
        }
        Log(L"Windows' pop-ups back for %zu apps", off.size());
        WriteList(kBannersOffValue, {});
    }
}

// While Tourne draws the pop-ups: an app notifying for the first time gets its key then, with Windows'
// pop-ups on; turned off as it appears. (Its very first one may still show Windows' pop-up.)
std::once_flag g_newAppsStarted;

DWORD WINAPI NewAppsThread(LPVOID) {
    HKEY root;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kKey, 0, KEY_NOTIFY | KEY_READ, &root) != ERROR_SUCCESS) return 0;
    HANDLE changed = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    for (;;) {
        if (RegNotifyChangeKeyValue(root, FALSE, REG_NOTIFY_CHANGE_NAME, changed, TRUE) != ERROR_SUCCESS) break;
        WaitForSingleObject(changed, INFINITE);
        if (TournePopups()) ApplyBanners();
    }
    CloseHandle(changed);
    RegCloseKey(root);
    return 0;
}

const wchar_t* RankWord(DWORD rank) { return rank >= 99 ? L"top" : rank >= 1 ? L"high" : L"normal"; }
DWORD RankOf(const std::wstring& word) { return word == L"top" ? 99 : word == L"high" ? 1 : 0; }

std::wstring Text(const std::wstring& key) {
    const wchar_t* s = GetString(L"notify", key);
    std::wstring out = s ? s : L"";
    FreeString(s);
    return out;
}

// "22:00" as minutes after midnight; -1 when it isn't a time.
int Minutes(const std::wstring& t) {
    int h = 0, m = 0;
    if (swscanf_s(t.c_str(), L"%d:%d", &h, &m) < 1 || h < 0 || h > 23 || m < 0 || m > 59) return -1;
    return h * 60 + m;
}

////////////////////////////////////////////////////////////////////////////////
// Quiet hours: notifications off from notify.quietFrom to notify.quietTo, every day, and back on after
// (only if quiet hours turned them off: one you turned off yourself stays off).

HANDLE g_quietWake;  // set when the settings change
std::once_flag g_quietStarted;
std::atomic<bool> g_quietNow{false};

bool InQuietHours(int* untilChange) {
    *untilChange = -1;
    if (!GetInt(L"notify", L"quiet")) return false;
    int from = Minutes(Text(L"quietFrom")), to = Minutes(Text(L"quietTo"));
    if (from < 0 || to < 0 || from == to) return false;
    SYSTEMTIME now;
    GetLocalTime(&now);
    int m = now.wHour * 60 + now.wMinute;
    bool in = from < to ? (m >= from && m < to) : (m >= from || m < to);
    int next = in ? to : from;
    *untilChange = ((next - m + 1440) % 1440) * 60 - now.wSecond;
    if (*untilChange <= 0) *untilChange = 60;
    return in;
}

void ApplyQuiet() {
    int wait;
    bool quiet = InQuietHours(&wait);
    bool paused = ReadDword(kOurKey, kQuietValue, 0) != 0;
    if (quiet && !paused) {
        if (ReadDword(kKey, L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", 1)) {  // (off already: yours, left alone)
            WriteDword(kKey, L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", 0);
            WriteDword(kOurKey, kQuietValue, 1);
            Log(L"quiet hours: notifications paused");
        }
    } else if (!quiet && paused) {
        WriteDword(kKey, L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", 1);
        RegDeleteKeyValueW(HKEY_CURRENT_USER, kOurKey, kQuietValue);
        Log(L"quiet hours over: notifications back");
    }
    g_quietNow = quiet;
}

DWORD WINAPI QuietThread(LPVOID) {
    for (;;) {
        ApplyQuiet();
        int wait;
        InQuietHours(&wait);
        // Wakes at the next start or end (and at most hourly, as the clock may have moved).
        DWORD ms = wait > 0 ? (DWORD)std::min(wait, 3600) * 1000 : 3600 * 1000;
        WaitForSingleObject(g_quietWake, ms);
    }
}

}  // namespace

void NotifyDefaults(Settings& out) {
    std::wstring k = kKey;
    out[L"notify.enabled"] = ReadDword(k, L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", 1) ? L"1" : L"0";
    out[L"notify.sound"] = ReadDword(k, L"NOC_GLOBAL_SETTING_ALLOW_NOTIFICATION_SOUND", 1) ? L"1" : L"0";
    out[L"notify.lockScreen"] = ReadDword(k, L"NOC_GLOBAL_SETTING_ALLOW_TOASTS_ABOVE_LOCK", 1) ? L"1" : L"0";
    out[L"notify.popups"] = L"windows";
    out[L"notify.quiet"] = L"0";
    out[L"notify.quietFrom"] = L"22:00";
    out[L"notify.quietTo"] = L"07:00";
    std::vector<App> apps = ReadApps();
    std::vector<std::wstring> off = ReadList(kBannersOffValue);  // (off for Tourne's pop-ups: on, as they were)
    for (auto const& a : apps) {
        std::wstring at = k + L"\\" + a.id, base = L"notify.apps." + a.key + L".";
        out[base + L"allow"] = ReadDword(at, L"Enabled", 1) ? L"1" : L"0";
        out[base + L"banners"] = Has(off, a.id) || ReadDword(at, L"ShowBanner", 1) ? L"1" : L"0";
        out[base + L"centre"] = ReadDword(at, L"ShowInActionCenter", 1) ? L"1" : L"0";
        out[base + L"priority"] = RankWord(ReadDword(at, L"Rank", 0));
    }
    std::lock_guard<std::mutex> lock(g_appsLock);
    g_apps = std::move(apps);
}

void NotifyDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"notify", L"Notifications",
        L"Windows' notification settings, here: the same ones as Settings, System, Notifications (a change there shows here "
        L"from Tourne Desktop's next start). Only what you change here is written.");
    add(L"notify.enabled", L"Notifications", L"Pop-ups and the notification centre, from every app. Off is Do not disturb.").isSwitch = true;
    add(L"notify.sound", L"Play sounds", L"").isSwitch = true;
    add(L"notify.lockScreen", L"Show them on the lock screen", L"").isSwitch = true;
    add(L"notify.popups", L"Pop-ups",
        L"Whose pop-ups show when a notification arrives. Tourne's are drawn in your theme, stacked above the bar, with the "
        L"notification's buttons; Windows' own are turned off app by app then (each still lands in the notification centre), "
        L"and put back as they were when you switch back. Each app's Pop-ups setting below decides either way.")
        .options = {{L"windows", L"Windows' own"}, {L"tourne", L"Tourne's"}};
    add(L"notify.quiet", L"Quiet hours", L"Notifications off every day between these times, and back on after.").isSwitch = true;
    add(L"notify.quietFrom", L"From", L"24-hour, like 22:00.");
    add(L"notify.quietTo", L"To", L"24-hour, like 07:00.");
    add(L"notify.apps", L"Apps", L"Each app that has notified, as Windows keeps them.");
    std::lock_guard<std::mutex> lock(g_appsLock);
    for (auto const& a : g_apps) {
        std::wstring base = L"notify.apps." + a.key;
        add(base, a.name.c_str(), a.id.c_str());
        add(base + L".allow", L"Notifications", L"").isSwitch = true;
        add(base + L".banners", L"Pop-ups", L"Off: they go straight to the notification centre.").isSwitch = true;
        add(base + L".centre", L"In the notification centre", L"").isSwitch = true;
        add(base + L".priority", L"Priority", L"Where they sit in the notification centre.").options = {
            {L"normal", L"Normal"}, {L"high", L"High"}, {L"top", L"Top"}};
    }
}

void NotifyApply() {
    std::wstring k = kKey;
    auto global = [&](const wchar_t* setting, const wchar_t* value) {
        if (!IsSet(L"notify", setting)) return;
        DWORD want = GetInt(L"notify", setting) ? 1 : 0;
        if (ReadDword(k, value, 1) != want && WriteDword(k, value, want)) Log(L"%s = %lu", value, want);
    };
    if (!g_quietNow) global(L"enabled", L"NOC_GLOBAL_SETTING_TOASTS_ENABLED");  // (quiet hours have it now)
    global(L"sound", L"NOC_GLOBAL_SETTING_ALLOW_NOTIFICATION_SOUND");
    global(L"lockScreen", L"NOC_GLOBAL_SETTING_ALLOW_TOASTS_ABOVE_LOCK");
    std::vector<App> apps;
    {
        std::lock_guard<std::mutex> lock(g_appsLock);
        apps = g_apps;
    }
    bool tourne = TournePopups();
    for (auto const& a : apps) {
        std::wstring at = k + L"\\" + a.id, base = L"apps." + a.key + L".";
        auto flag = [&](const wchar_t* setting, const wchar_t* value) {
            if (!IsSet(L"notify", base + setting)) return;
            DWORD want = GetInt(L"notify", base + setting) ? 1 : 0;
            if (ReadDword(at, value, 1) != want && WriteDword(at, value, want)) Log(L"%s: %s = %lu", a.id.c_str(), value, want);
        };
        flag(L"allow", L"Enabled");
        if (!tourne) flag(L"banners", L"ShowBanner");  // (Tourne's pop-ups: Windows' stay off, Tourne reads this)
        flag(L"centre", L"ShowInActionCenter");
        if (IsSet(L"notify", base + L"priority")) {
            DWORD want = RankOf(Text(base + L"priority"));
            if (ReadDword(at, L"Rank", 0) != want && WriteDword(at, L"Rank", want)) Log(L"%s: Rank = %lu", a.id.c_str(), want);
        }
    }
    ApplyBanners();
    if (tourne)
        std::call_once(g_newAppsStarted, [] {
            if (HANDLE thread = CreateThread(nullptr, 0, NewAppsThread, nullptr, 0, nullptr)) CloseHandle(thread);
        });
    // Quiet hours: started the first time they're wanted (or have notifications paused), told of changes after.
    if (!GetInt(L"notify", L"quiet") && !ReadDword(kOurKey, kQuietValue, 0) && !g_quietWake) return;
    std::call_once(g_quietStarted, [] {
        g_quietWake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (HANDLE thread = CreateThread(nullptr, 0, QuietThread, nullptr, 0, nullptr)) {
            SetThreadPriority(thread, THREAD_PRIORITY_BELOW_NORMAL);
            CloseHandle(thread);
        }
    });
    if (g_quietWake) SetEvent(g_quietWake);
}

bool NotifyQuietNow() { return g_quietNow.load(); }

}  // namespace td
