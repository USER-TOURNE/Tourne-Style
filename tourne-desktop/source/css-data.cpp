// What a YASB theme's widgets show, read from Windows: the virtual desktops of Win+Tab (for the workspace widgets),
// processor and memory use, battery, network traffic and Wi-Fi, the weather (only when allowed), what is playing, and
// the output of a custom widget's command (only when allowed). Inlined into taskbar-host.cpp before css-bar.cpp.

void SendKeys(std::initializer_list<WORD> keys);
std::wstring Title(HWND h);
bool IsTaskWindow(HWND h, bool anyDesktop);

// ---------------------------------------------------------------------------------------------------- background work

// A worker that keeps this module loaded until it has ended, so a download still running at unload can't come back to
// unmapped code.
struct BgStart {
    void (*fn)(void*);
    void* arg;
    HMODULE self;
};

DWORD WINAPI BgProc(LPVOID param) {
    std::unique_ptr<BgStart> start((BgStart*)param);
    HMODULE self = start->self;
    start->fn(start->arg);
    start.reset();
    if (self) FreeLibraryAndExitThread(self, 0);
    return 0;
}

bool BgWork(void (*fn)(void*), void* arg, HANDLE* handle = nullptr) {
    auto* start = new BgStart{fn, arg, nullptr};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&BgProc, &start->self);
    HANDLE thread = CreateThread(nullptr, 0, BgProc, start, 0, nullptr);
    if (!thread) {
        if (start->self) FreeLibrary(start->self);
        delete start;
        return false;
    }
    if (handle) *handle = thread;
    else CloseHandle(thread);
    return true;
}

// ---------------------------------------------------------------------------------------------------- JSON, enough for a weather report

// The value of the first "key": a string (unescaped enough for names) or a number, true or false as written.
static std::string CssJsonValue(std::string const& text, std::string const& key) {
    std::string needle = "\"" + key + "\"";
    size_t at = text.find(needle);
    while (at != std::string::npos) {
        size_t i = at + needle.size();
        while (i < text.size() && isspace((unsigned char)text[i])) i++;
        if (i < text.size() && text[i] == ':') {
            i++;
            while (i < text.size() && isspace((unsigned char)text[i])) i++;
            if (i >= text.size()) return {};
            std::string out;
            if (text[i] == '"') {
                for (i++; i < text.size() && text[i] != '"'; i++) {
                    if (text[i] == '\\' && i + 1 < text.size()) i++;
                    out += text[i];
                }
            } else {
                while (i < text.size() && text[i] != ',' && text[i] != '}' && text[i] != ']' && !isspace((unsigned char)text[i])) out += text[i++];
            }
            return out;
        }
        at = text.find(needle, at + 1);
    }
    return {};
}

static std::wstring CssFromUtf8(std::string const& text) {
    if (text.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0);
    std::wstring out(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), out.data(), n);
    return out;
}

// ---------------------------------------------------------------------------------------------------- virtual desktops (Win+Tab)

struct CssDesktopInfo {
    int count = 1, current = 0;
    std::vector<int> windows;          // task windows on each
    std::vector<std::wstring> names;   // as named in Task view; empty for "Desktop N"
};

static ULONGLONG g_desktopsAt = 0;
static CssDesktopInfo g_desktopsCache;

// The registry says a desktop was added, removed or switched to: read again at the next look.
void CssDesktopsChanged() { g_desktopsAt = 0; }

static std::vector<GUID> CssReadGuids(wchar_t const* value) {
    std::vector<GUID> ids;
    BYTE data[16 * 64];
    DWORD size = sizeof(data);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops", value, RRF_RT_REG_BINARY,
                     nullptr, data, &size) != ERROR_SUCCESS)
        return ids;
    for (DWORD at = 0; at + 16 <= size; at += 16) {
        GUID g;
        memcpy(&g, data + at, 16);
        ids.push_back(g);
    }
    return ids;
}

static IVirtualDesktopManager* CssVdm() {
    static IVirtualDesktopManager* manager = nullptr;
    static bool tried = false;
    if (!tried) {
        tried = true;
        if (FAILED(CoCreateInstance(CLSID_VirtualDesktopManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&manager)))) manager = nullptr;
    }
    return manager;
}

static CssDesktopInfo CssDesktops() {
    ULONGLONG now = GetTickCount64();
    if (g_desktopsAt && now - g_desktopsAt < 400) return g_desktopsCache;
    g_desktopsAt = now ? now : 1;
    CssDesktopInfo info;
    std::vector<GUID> ids = CssReadGuids(L"VirtualDesktopIDs");
    std::vector<GUID> current = CssReadGuids(L"CurrentVirtualDesktop");
    info.count = std::max<int>(1, (int)ids.size());
    if (!ids.empty() && !current.empty())
        for (size_t i = 0; i < ids.size(); i++)
            if (!memcmp(&ids[i], &current[0], sizeof(GUID))) info.current = (int)i;
    info.windows.assign(info.count, 0);
    for (GUID const& id : ids) {
        WCHAR guid[40] = L"";
        StringFromGUID2(id, guid, ARRAYSIZE(guid));
        std::wstring key = std::wstring(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops\\Desktops\\") + guid;
        WCHAR name[256] = L"";
        DWORD size = sizeof(name);
        if (RegGetValueW(HKEY_CURRENT_USER, key.c_str(), L"Name", RRF_RT_REG_SZ, nullptr, name, &size) != ERROR_SUCCESS) name[0] = 0;
        info.names.push_back(name);
    }
    info.names.resize(info.count);
    IVirtualDesktopManager* vdm = ids.size() > 1 ? CssVdm() : nullptr;
    struct Ctx {
        std::vector<GUID>* ids;
        CssDesktopInfo* info;
        IVirtualDesktopManager* vdm;
    } ctx{&ids, &info, vdm};
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            auto* c = (Ctx*)param;
            if (!IsTaskWindow(h, true)) return TRUE;
            int index = c->info->current;
            GUID id;
            if (c->vdm && SUCCEEDED(c->vdm->GetWindowDesktopId(h, &id))) {
                for (size_t i = 0; i < c->ids->size(); i++)
                    if (!memcmp(&(*c->ids)[i], &id, sizeof(GUID))) index = (int)i;
            }
            if (index >= 0 && index < (int)c->info->windows.size()) c->info->windows[index]++;
            return TRUE;
        },
        (LPARAM)&ctx);
    g_desktopsCache = info;
    return info;
}

// Straight there through Windows' own desktop manager, as Task view goes (desktops-pager.cpp), where it answers.
bool VdmSwitchTo(GUID const& id);

// Otherwise Ctrl+Win+Left or Right, once for each desktop between here and there: the same as Win+Tab's own desktop
// switching.
void CssGoToDesktop(int target) {
    std::vector<GUID> ids = CssReadGuids(L"VirtualDesktopIDs");
    if (target >= 0 && target < (int)ids.size() && VdmSwitchTo(ids[target])) {
        CssDesktopsChanged();
        return;
    }
    CssDesktopInfo d = CssDesktops();
    int steps = target - d.current;
    WORD key = steps < 0 ? VK_LEFT : VK_RIGHT;
    for (int i = 0; i < std::abs(steps) && i < 20; i++) {
        SendKeys({VK_LCONTROL, VK_LWIN, key});
        Sleep(90);
    }
    CssDesktopsChanged();
}

// ---------------------------------------------------------------------------------------------------- processor, memory, battery

static std::vector<int> g_cpuHistory;

static int CssCpuPercent() {
    static ULONGLONG idle0 = 0, total0 = 0, at = 0;
    static int last = 0;
    ULONGLONG now = GetTickCount64();
    if (at && now - at < 900) return last;
    FILETIME idle, kernel, user;
    if (!GetSystemTimes(&idle, &kernel, &user)) return last;
    auto u64 = [](FILETIME const& f) { return ((ULONGLONG)f.dwHighDateTime << 32) | f.dwLowDateTime; };
    ULONGLONG i = u64(idle), total = u64(kernel) + u64(user);  // the kernel's time includes the idle time
    if (at && total > total0) last = (int)std::clamp<double>(100.0 * (double)((total - total0) - (i - idle0)) / (double)(total - total0), 0, 100);
    idle0 = i;
    total0 = total;
    at = now;
    g_cpuHistory.push_back(last);
    if (g_cpuHistory.size() > 64) g_cpuHistory.erase(g_cpuHistory.begin());
    return last;
}

static std::wstring CssBytes(double bytes) {
    wchar_t text[32];
    if (bytes >= 1024.0 * 1024 * 1024) swprintf(text, 32, L"%.1fG", bytes / (1024.0 * 1024 * 1024));
    else swprintf(text, 32, L"%.0fM", bytes / (1024.0 * 1024));
    return text;
}

// {virtual_mem_free}, {swap_mem_percent}... (psutil's names, as YASB's memory widget has them).
static std::wstring CssMemoryField(std::wstring const& key) {
    MEMORYSTATUSEX m{sizeof(m)};
    GlobalMemoryStatusEx(&m);
    bool swap = key.rfind(L"swap_mem_", 0) == 0;
    double total = swap ? (double)(m.ullTotalPageFile - m.ullTotalPhys) : (double)m.ullTotalPhys;
    double avail = swap ? (double)(m.ullAvailPageFile - std::min(m.ullAvailPhys, m.ullAvailPageFile)) : (double)m.ullAvailPhys;
    if (swap && total <= 0) total = 1;
    double used = std::max(0.0, total - avail);
    std::wstring what = key.substr(swap ? 9 : 12);
    if (what == L"total") return CssBytes(total);
    if (what == L"free" || what == L"available") return CssBytes(avail);
    if (what == L"used") return CssBytes(used);
    if (what == L"percent") return std::to_wstring((int)std::lround(100.0 * used / total));
    return L"";
}

struct CssBattery {
    bool present = false, charging = false;
    int percent = 0;
    int secondsLeft = -1;
};

static CssBattery CssBatteryNow() {
    CssBattery b;
    SYSTEM_POWER_STATUS s{};
    if (!GetSystemPowerStatus(&s) || s.BatteryFlag == 255 || (s.BatteryFlag & 128)) return b;
    b.present = true;
    b.percent = s.BatteryLifePercent == 255 ? 0 : s.BatteryLifePercent;
    b.charging = (s.BatteryFlag & 8) != 0 || (s.ACLineStatus == 1 && s.BatteryLifePercent < 100);
    b.secondsLeft = s.BatteryLifeTime == (DWORD)-1 ? -1 : (int)s.BatteryLifeTime;
    return b;
}

// ---------------------------------------------------------------------------------------------------- traffic and Wi-Fi

static void CssTraffic(double* down, double* up) {
    static ULONGLONG at = 0, in0 = 0, out0 = 0;
    static double lastDown = 0, lastUp = 0;
    ULONGLONG now = GetTickCount64();
    if (!at || now - at >= 900) {
        ULONG size = 0;
        GetIfTable(nullptr, &size, FALSE);
        std::vector<BYTE> buffer(size + 1024);
        auto* table = (MIB_IFTABLE*)buffer.data();
        size = (ULONG)buffer.size();
        if (GetIfTable(table, &size, FALSE) == NO_ERROR) {
            ULONGLONG in = 0, out = 0;
            for (DWORD i = 0; i < table->dwNumEntries; i++) {
                MIB_IFROW const& row = table->table[i];
                if (row.dwType == IF_TYPE_SOFTWARE_LOOPBACK || row.dwOperStatus < IF_OPER_STATUS_CONNECTED) continue;
                in += row.dwInOctets;
                out += row.dwOutOctets;
            }
            if (at && now > at) {
                double seconds = (now - at) / 1000.0;
                lastDown = in >= in0 ? (in - in0) / seconds : 0;
                lastUp = out >= out0 ? (out - out0) / seconds : 0;
            }
            in0 = in;
            out0 = out;
            at = now;
        }
    }
    *down = lastDown;
    *up = lastUp;
}

static std::wstring CssSpeed(double bytesPerSecond) {
    wchar_t text[32];
    if (bytesPerSecond >= 1024.0 * 1024) swprintf(text, 32, L"%.1f MB/s", bytesPerSecond / (1024.0 * 1024));
    else swprintf(text, 32, L"%.0f KB/s", bytesPerSecond / 1024);
    return text;
}

struct CssWifi {
    bool connected = false;
    int strength = 0;
    std::wstring name;
};

static CssWifi CssWifiNow() {
    static CssWifi cache;
    static ULONGLONG at = 0;
    ULONGLONG now = GetTickCount64();
    if (at && now - at < 3000) return cache;
    at = now;
    CssWifi w;
    HANDLE client = nullptr;
    DWORD version = 0;
    if (WlanOpenHandle(2, nullptr, &version, &client) == ERROR_SUCCESS) {
        WLAN_INTERFACE_INFO_LIST* list = nullptr;
        if (WlanEnumInterfaces(client, nullptr, &list) == ERROR_SUCCESS) {
            for (DWORD i = 0; i < list->dwNumberOfItems && !w.connected; i++) {
                WLAN_CONNECTION_ATTRIBUTES* attrs = nullptr;
                DWORD size = 0;
                if (WlanQueryInterface(client, &list->InterfaceInfo[i].InterfaceGuid, wlan_intf_opcode_current_connection, nullptr, &size,
                                       (PVOID*)&attrs, nullptr) == ERROR_SUCCESS) {
                    if (attrs->isState == wlan_interface_state_connected) {
                        w.connected = true;
                        w.strength = (int)attrs->wlanAssociationAttributes.wlanSignalQuality;
                        auto const& ssid = attrs->wlanAssociationAttributes.dot11Ssid;
                        w.name = CssFromUtf8(std::string((char const*)ssid.ucSSID, ssid.uSSIDLength));
                    }
                    WlanFreeMemory(attrs);
                }
            }
            WlanFreeMemory(list);
        }
        WlanCloseHandle(client, nullptr);
    }
    cache = w;
    return w;
}

// ---------------------------------------------------------------------------------------------------- weather (weatherapi.com, when allowed)

struct CssWeatherData {
    bool ok = false, fetching = false, isDay = true;
    double tempC = 0, tempF = 0, minC = 0, maxC = 0, minF = 0, maxF = 0;
    int humidity = 0, code = 1000;
    std::wstring location, conditions;
    ULONGLONG at = 0;
};

static SRWLOCK g_weatherLock = SRWLOCK_INIT;
static CssWeatherData g_weather;

struct CssWeatherJob {
    std::string url;
};

static std::string CssUrlEscape(std::string const& s) {
    std::string out;
    for (unsigned char c : s) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') out += (char)c;
        else {
            char hex[4];
            snprintf(hex, sizeof(hex), "%%%02X", c);
            out += hex;
        }
    }
    return out;
}

static std::string CssNarrow(std::wstring const& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), out.data(), n, nullptr, nullptr);
    return out;
}

static void CssWeatherWorker(void* param) {
    std::unique_ptr<CssWeatherJob> job((CssWeatherJob*)param);
    WCHAR temp[MAX_PATH];
    DWORD n = GetTempPathW(ARRAYSIZE(temp), temp);
    std::wstring file = std::wstring(temp, n) + L"TourneWeather.json";
    std::wstring url = CssFromUtf8(job->url);
    DeleteUrlCacheEntryW(url.c_str());
    CssWeatherData w;
    if (SUCCEEDED(URLDownloadToFileW(nullptr, url.c_str(), file.c_str(), 0, nullptr))) {
        HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        std::string text;
        if (h != INVALID_HANDLE_VALUE) {
            DWORD size = GetFileSize(h, nullptr), got = 0;
            if (size != INVALID_FILE_SIZE && size < 2 * 1024 * 1024) {
                text.resize(size);
                if (!ReadFile(h, text.data(), size, &got, nullptr)) got = 0;
                text.resize(got);
            }
            CloseHandle(h);
        }
        DeleteFileW(file.c_str());
        std::string tc = CssJsonValue(text, "temp_c");
        if (!tc.empty()) {
            w.ok = true;
            w.tempC = atof(tc.c_str());
            w.tempF = atof(CssJsonValue(text, "temp_f").c_str());
            w.minC = atof(CssJsonValue(text, "mintemp_c").c_str());
            w.maxC = atof(CssJsonValue(text, "maxtemp_c").c_str());
            w.minF = atof(CssJsonValue(text, "mintemp_f").c_str());
            w.maxF = atof(CssJsonValue(text, "maxtemp_f").c_str());
            w.humidity = atoi(CssJsonValue(text, "humidity").c_str());
            w.code = atoi(CssJsonValue(text, "code").c_str());
            w.isDay = CssJsonValue(text, "is_day") != "0";
            w.location = CssFromUtf8(CssJsonValue(text, "name"));
            w.conditions = CssFromUtf8(CssJsonValue(text, "text"));
        }
    }
    w.at = GetTickCount64();
    AcquireSRWLockExclusive(&g_weatherLock);
    // A failed read keeps the last good report (and is tried again after the interval).
    if (w.ok || !g_weather.ok) g_weather = w;
    else g_weather.at = w.at;
    g_weather.fetching = false;
    ReleaseSRWLockExclusive(&g_weatherLock);
}

// The report as last read; a new one is asked for when the interval has passed.
static CssWeatherData CssWeatherNow(yaml::Node const& opt) {
    ULONGLONG interval = (ULONGLONG)std::max(600.0, opt.Num(L"update_interval", 600)) * 1000;
    std::string key = CssNarrow(opt.Str(L"api_key")), place = CssNarrow(opt.Str(L"location"));
    AcquireSRWLockExclusive(&g_weatherLock);
    CssWeatherData copy = g_weather;
    bool start = g_settings.yasbNetwork && !key.empty() && !place.empty() && !g_weather.fetching &&
                 (!g_weather.at || GetTickCount64() - g_weather.at >= interval);
    if (start) g_weather.fetching = true;
    ReleaseSRWLockExclusive(&g_weatherLock);
    if (start) {
        auto* job = new CssWeatherJob{"https://api.weatherapi.com/v1/forecast.json?key=" + CssUrlEscape(key) + "&q=" + CssUrlEscape(place) +
                                      "&days=1&aqi=no&alerts=no"};
        if (!BgWork(CssWeatherWorker, job)) {
            delete job;
            AcquireSRWLockExclusive(&g_weatherLock);
            g_weather.fetching = false;
            ReleaseSRWLockExclusive(&g_weatherLock);
        }
    }
    return copy;
}

// weatherapi.com's condition code as the theme's icon names have it (sunnyDay, clearNight, cloudyDay...).
static std::wstring CssWeatherIconKey(CssWeatherData const& w) {
    int c = w.code;
    std::wstring base;
    if (c == 1000) base = w.isDay ? L"sunny" : L"clear";
    else if (c == 1003 || c == 1006 || c == 1009 || c == 1030 || c == 1135 || c == 1147) base = L"cloudy";
    else if (c == 1117 || c == 1282 || c == 1225) base = L"blizzard";
    else if ((c >= 1066 && c <= 1072 && c != 1069) || c == 1114 || (c >= 1210 && c <= 1237) || (c >= 1255 && c <= 1264) || c == 1069) base = L"snowy";
    else if (c == 1063 || c == 1087 || (c >= 1150 && c <= 1207) || (c >= 1240 && c <= 1252) || c == 1273 || c == 1276) base = L"rainy";
    if (base.empty()) return L"default";
    if (base == L"blizzard") return L"blizzard" + std::wstring(w.isDay ? L"Day" : L"Night");
    if (base == L"sunny") return L"sunnyDay";
    if (base == L"clear") return L"clearNight";
    return base + (w.isDay ? L"Day" : L"Night");
}

// ---------------------------------------------------------------------------------------------------- what is playing

struct CssMedia {
    bool found = false, playing = false;
    std::wstring title, artist;
};

// Windows keeps "what is playing" in a WinRT interface this build has no headers for; the players that put it in their
// window title (Spotify, VLC, foobar2000, MusicBee and YouTube in a browser) are read from there. The buttons send the
// media keys, which work for any player.
static CssMedia CssMediaNow() {
    static CssMedia cache;
    static ULONGLONG at = 0;
    ULONGLONG now = GetTickCount64();
    if (at && now - at < 1000) return cache;
    at = now;
    CssMedia m;
    struct Ctx {
        CssMedia* m;
    } ctx{&m};
    EnumWindows(
        [](HWND h, LPARAM param) __stdcall -> BOOL {
            auto* m = ((Ctx*)param)->m;
            if (m->found || !IsWindowVisible(h)) return TRUE;
            int len = GetWindowTextLengthW(h);
            if (len <= 0) return TRUE;
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            std::wstring exe;
            if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
                WCHAR path[MAX_PATH];
                DWORD size = ARRAYSIZE(path);
                if (QueryFullProcessImageNameW(p, 0, path, &size)) exe = path;
                CloseHandle(p);
            }
            exe = exe.substr(exe.find_last_of(L'\\') + 1);
            for (auto& c : exe) c = (wchar_t)towlower(c);
            std::wstring title = Title(h);
            auto cut = [&](std::wstring const& suffix) {
                if (title.size() > suffix.size() && title.compare(title.size() - suffix.size(), suffix.size(), suffix) == 0)
                    title.resize(title.size() - suffix.size());
            };
            auto split = [&](std::wstring const& t) {
                size_t dash = t.find(L" - ");
                if (dash == std::wstring::npos) m->title = t;
                else {
                    m->artist = t.substr(0, dash);
                    m->title = t.substr(dash + 3);
                }
            };
            if (exe == L"spotify.exe") {
                if (title == L"Spotify" || title == L"Spotify Free" || title == L"Spotify Premium" || title.find(L" - ") == std::wstring::npos) return TRUE;
                split(title);
            } else if (exe == L"vlc.exe") {
                cut(L" - VLC media player");
                if (title == L"VLC media player") return TRUE;
                split(title);
            } else if (exe == L"foobar2000.exe") {
                cut(L" [foobar2000]");
                split(title);
            } else if (exe == L"musicbee.exe") {
                if (title == L"MusicBee") return TRUE;
                split(title);
            } else if (exe == L"chrome.exe" || exe == L"msedge.exe" || exe == L"firefox.exe" || exe == L"brave.exe" || exe == L"opera.exe") {
                for (wchar_t const* browser : {L" - Google Chrome", L" - Microsoft Edge", L" - Mozilla Firefox", L" - Brave", L" - Opera", L" \u2014 Mozilla Firefox"}) cut(browser);
                if (title.size() > 10 && title.compare(title.size() - 10, 10, L" - YouTube") == 0) {
                    title.resize(title.size() - 10);
                    m->title = title;
                } else if (title.size() > 16 && title.compare(title.size() - 16, 16, L" - YouTube Music") == 0) {
                    title.resize(title.size() - 16);
                    split(title);
                } else {
                    return TRUE;
                }
            } else {
                return TRUE;
            }
            m->found = !m->title.empty();
            m->playing = m->found;
            return TRUE;
        },
        (LPARAM)&ctx);
    cache = m;
    return m;
}

void CssMediaKey(WORD key) { SendKeys({key}); }

// ---------------------------------------------------------------------------------------------------- a custom widget's command

struct CssExecResult {
    std::string text;
    ULONGLONG at = 0;
    bool running = false;
};

static SRWLOCK g_execLock = SRWLOCK_INIT;
static std::map<std::wstring, CssExecResult> g_exec;  // by the widget's name

struct CssExecJob {
    std::wstring name, command;
};

static void CssExecWorker(void* param) {
    std::unique_ptr<CssExecJob> job((CssExecJob*)param);
    std::string output;
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE read = nullptr, write = nullptr;
    if (CreatePipe(&read, &write, &sa, 0)) {
        SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOW si{sizeof(si)};
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdOutput = write;
        si.hStdError = write;
        si.wShowWindow = SW_HIDE;
        std::wstring cmd = L"cmd.exe /c " + job->command;
        PROCESS_INFORMATION pi{};
        if (CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            CloseHandle(write);
            write = nullptr;
            ULONGLONG start = GetTickCount64();
            char buffer[4096];
            for (;;) {
                DWORD avail = 0;
                if (!PeekNamedPipe(read, nullptr, 0, nullptr, &avail, nullptr)) break;  // the pipe closed: it ended
                if (avail) {
                    DWORD got = 0;
                    if (ReadFile(read, buffer, std::min<DWORD>(avail, sizeof(buffer)), &got, nullptr) && got && output.size() < 65536) output.append(buffer, got);
                } else if (WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0) {
                    break;
                }
                if (GetTickCount64() - start > 15000) {
                    TerminateProcess(pi.hProcess, 1);
                    break;
                }
            }
            DWORD avail = 0;
            while (PeekNamedPipe(read, nullptr, 0, nullptr, &avail, nullptr) && avail) {
                DWORD got = 0;
                if (!ReadFile(read, buffer, std::min<DWORD>(avail, sizeof(buffer)), &got, nullptr) || !got) break;
                if (output.size() < 65536) output.append(buffer, got);
            }
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        if (write) CloseHandle(write);
        CloseHandle(read);
    }
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r' || output.back() == ' ')) output.pop_back();
    AcquireSRWLockExclusive(&g_execLock);
    CssExecResult& r = g_exec[job->name];
    r.text = output;
    r.at = GetTickCount64();
    r.running = false;
    ReleaseSRWLockExclusive(&g_execLock);
}

// The last output of the widget's command; asked again when its interval has passed. Only with the setting that lets a theme
// run commands.
static std::string CssExecData(std::wstring const& name, yaml::Node const* exec) {
    if (!exec || !g_settings.yasbCommands) return {};
    std::wstring command = exec->Str(L"run_cmd");
    if (command.empty()) return {};
    ULONGLONG interval = (ULONGLONG)std::max(1000.0, exec->Num(L"run_interval", 60000));
    AcquireSRWLockExclusive(&g_execLock);
    CssExecResult& r = g_exec[name];
    std::string text = r.text;
    bool start = !r.running && (!r.at || GetTickCount64() - r.at >= interval);
    if (start) r.running = true;
    ReleaseSRWLockExclusive(&g_execLock);
    if (start) {
        auto* job = new CssExecJob{name, command};
        if (!BgWork(CssExecWorker, job)) {
            delete job;
            AcquireSRWLockExclusive(&g_execLock);
            g_exec[name].running = false;
            ReleaseSRWLockExclusive(&g_execLock);
        }
    }
    return text;
}
