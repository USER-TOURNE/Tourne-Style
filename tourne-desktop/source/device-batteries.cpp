////////////////////////////////////////////////////////////////////////////////
// Device batteries: the battery of wireless headsets, mice, keyboards and
// controllers, read straight from the devices.
//
// A C++ port of HaloBattery's providers (github.com/HeyOkay/HaloBattery, MIT,
// Copyright (c) 2026 Halo Battery contributors), whose protocol notes come
// from OpenRazer, Solaar, HeadsetControl and the projects credited there, plus
// the Alienware Pro Wireless headset and AW720M mouse, learned from what
// Alienware Command Center sends them. Every request here is a read.
//
// Built into System Icons by Build-TourneSuiteRedirect.ps1, and included on
// its own by the test harness, so it only needs Windows headers and Wh_Log.

namespace bat {

struct Device {
    std::wstring key;       // one tray icon per key
    std::wstring name;
    std::string kind;       // headset, mouse, keyboard, gamepad, bluetooth
    std::string source;     // which reader found it
    int level = -1;         // 0 to 100; -1 when not reported
    bool charging = false;
    bool online = true;     // false: asleep, showing the last level it gave
    std::wstring approx;    // shown instead of "NN%" for coarse readings
    std::string via;        // "bluetooth": a controller connected over Bluetooth
    unsigned vid = 0;
    unsigned pid = 0;
};

enum Vendor : int {
    kAlienware,
    kRazer,
    kLogitech,
    kSteelSeries,
    kHyperX,
    kAudeze,
    kMchose,
    kWlmouse,
    kPlayStation,
    kXbox,
    kBluetooth,
    kVendorCount
};

constexpr PCWSTR kVendorKeys[kVendorCount] = {
    L"alienware", L"razer", L"logitech", L"steelseries", L"hyperx", L"audeze",
    L"mchose", L"wlmouse", L"playstation", L"xbox", L"bluetooth",
};

// How long a device that stopped answering keeps its last level, greyed out.
constexpr ULONGLONG kAsleepKeepMs = 300 * 1000;

std::wstring Lower(std::wstring s) {
    for (auto& c : s) {
        c = (wchar_t)towlower(c);
    }
    return s;
}

std::string LowerA(std::wstring const& s) {
    std::string out;
    for (wchar_t c : s) {
        out += c < 128 ? (char)tolower((int)c) : '?';
    }
    return out;
}

bool Contains(std::wstring const& lower, std::wstring_view word) {
    return lower.find(word) != std::wstring::npos;
}

////////////////////////////////////////////////////////////////////////////////
// The HID device list, without opening any device.

constexpr GUID kGuidHid = {0x4D1E55B2, 0xF16F, 0x11CF, {0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};

std::vector<std::wstring> InterfacePaths() {
    std::vector<std::wstring> paths;
    for (int attempt = 0; attempt < 4; attempt++) {
        ULONG size = 0;
        if (CM_Get_Device_Interface_List_SizeW(&size, (LPGUID)&kGuidHid, nullptr,
                                               CM_GET_DEVICE_INTERFACE_LIST_PRESENT) != CR_SUCCESS) {
            return paths;
        }
        std::vector<WCHAR> buffer(size + 1);
        CONFIGRET cr = CM_Get_Device_Interface_ListW((LPGUID)&kGuidHid, nullptr, buffer.data(), size + 1,
                                                     CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
        if (cr == CR_BUFFER_SMALL) {
            continue;
        }
        if (cr != CR_SUCCESS) {
            return paths;
        }
        for (PCWSTR p = buffer.data(); *p; p += wcslen(p) + 1) {
            paths.push_back(p);
        }
        return paths;
    }
    return paths;
}

// "vid_054c" on USB, "vid&0002054c" over Bluetooth: the last four digits.
unsigned HexAfter(std::wstring const& lower, std::wstring_view tag) {
    for (wchar_t sep : {L'_', L'&'}) {
        std::wstring key = std::wstring(tag) + sep;
        size_t at = lower.find(key);
        if (at == std::wstring::npos) {
            continue;
        }
        at += key.size();
        size_t end = at;
        while (end < lower.size() && iswxdigit(lower[end]) && end - at < 8) {
            end++;
        }
        if (end - at >= 4) {
            return wcstoul(lower.substr(end - 4, 4).c_str(), nullptr, 16);
        }
    }
    return 0;
}

struct HidInfo {
    std::wstring path;
    std::wstring lower;
    unsigned vid = 0;
    unsigned pid = 0;
    int iface = -1;
    unsigned usagePage = 0;
    unsigned usage = 0;
    std::wstring product;
    std::wstring serial;
    unsigned inLen = 0;
    unsigned outLen = 0;
    unsigned featLen = 0;
    bool bluetooth = false;
};

void ParsePath(HidInfo& info) {
    info.lower = Lower(info.path);
    info.vid = HexAfter(info.lower, L"vid");
    info.pid = HexAfter(info.lower, L"pid");
    size_t mi = info.lower.find(L"&mi_");
    if (mi != std::wstring::npos) {
        info.iface = (int)wcstoul(info.lower.substr(mi + 4, 2).c_str(), nullptr, 16);
    }
    info.bluetooth = Contains(info.lower, L"{00001124-0000-1000-8000-00805f9b34fb}") ||
                     Contains(info.lower, L"{00001812-0000-1000-8000-00805f9b34fb}") ||
                     Contains(info.lower, L"vid&");
}

// Attributes of the collections of one vendor. Each is opened once, with no
// access rights, which is enough for its usage, report sizes and strings and
// doesn't disturb the device; the result is kept until the path goes away.
std::map<std::wstring, HidInfo> g_infoCache;

std::vector<HidInfo> Enumerate(std::vector<std::wstring> const& paths, unsigned vid) {
    std::vector<HidInfo> out;
    for (auto const& path : paths) {
        HidInfo quick{path};
        ParsePath(quick);
        if (quick.vid != vid) {
            continue;
        }
        auto cached = g_infoCache.find(path);
        if (cached != g_infoCache.end()) {
            out.push_back(cached->second);
            continue;
        }
        HANDLE h = CreateFileW(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0,
                               nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            continue;
        }
        PHIDP_PREPARSED_DATA preparsed = nullptr;
        if (HidD_GetPreparsedData(h, &preparsed)) {
            HIDP_CAPS caps{};
            if (HidP_GetCaps(preparsed, &caps) == HIDP_STATUS_SUCCESS) {
                quick.usagePage = caps.UsagePage;
                quick.usage = caps.Usage;
                quick.inLen = caps.InputReportByteLength;
                quick.outLen = caps.OutputReportByteLength;
                quick.featLen = caps.FeatureReportByteLength;
            }
            HidD_FreePreparsedData(preparsed);
        }
        WCHAR text[128] = L"";
        if (HidD_GetProductString(h, text, sizeof(text))) {
            quick.product = text;
        }
        text[0] = 0;
        if (HidD_GetSerialNumberString(h, text, sizeof(text))) {
            quick.serial = text;
        }
        CloseHandle(h);
        g_infoCache[path] = quick;
        out.push_back(quick);
    }
    return out;
}

void ForgetMissing(std::vector<std::wstring> const& paths) {
    for (auto it = g_infoCache.begin(); it != g_infoCache.end();) {
        if (std::find(paths.begin(), paths.end(), it->first) == paths.end()) {
            it = g_infoCache.erase(it);
        } else {
            ++it;
        }
    }
}

bool VendorPresent(std::vector<std::wstring> const& paths, unsigned vid) {
    for (auto const& path : paths) {
        HidInfo quick{path};
        ParsePath(quick);
        if (quick.vid == vid) {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// One open collection, with hidapi's conventions, since the protocol notes are
// written against them: writes are padded to the report size, a read whose
// report id is 0 has that byte removed, feature and input reports keep their
// report id in front. Every request uses overlapped I/O with a timeout.

constexpr DWORD kIoctlSetFeature = 0xB0191;
constexpr DWORD kIoctlGetFeature = 0xB0192;
constexpr DWORD kIoctlSetOutput = 0xB0195;
constexpr DWORD kIoctlGetInput = 0xB01A2;

class HidDevice {
   public:
    HidInfo info;

    ~HidDevice() { Close(); }

    bool Open(HidInfo const& from) {
        info = from;
        h_ = CreateFileW(info.path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                         nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if (h_ == INVALID_HANDLE_VALUE) {
            h_ = nullptr;
            return false;
        }
        readEvent_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        ioEvent_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        readBuffer_.assign(std::max(info.inLen, 1u), 0);
        return readEvent_ && ioEvent_;
    }

    void Close() {
        if (h_) {
            if (readPending_) {
                CancelIoEx(h_, &readOv_);
                DWORD n;
                GetOverlappedResult(h_, &readOv_, &n, TRUE);
                readPending_ = false;
            }
            CloseHandle(h_);
            h_ = nullptr;
        }
        if (readEvent_) {
            CloseHandle(readEvent_);
            readEvent_ = nullptr;
        }
        if (ioEvent_) {
            CloseHandle(ioEvent_);
            ioEvent_ = nullptr;
        }
    }

    // Bytes written, or -1.
    int Write(std::vector<BYTE> data) {
        if (!h_) {
            return -1;
        }
        if (data.size() < info.outLen) {
            data.resize(info.outLen, 0);
        }
        OVERLAPPED ov{};
        ov.hEvent = ioEvent_;
        ResetEvent(ioEvent_);
        DWORD n = 0;
        if (!WriteFile(h_, data.data(), (DWORD)data.size(), nullptr, &ov) && GetLastError() != ERROR_IO_PENDING) {
            return -1;
        }
        if (WaitForSingleObject(ioEvent_, 1000) != WAIT_OBJECT_0) {
            CancelIoEx(h_, &ov);
            GetOverlappedResult(h_, &ov, &n, TRUE);
            return -1;
        }
        if (!GetOverlappedResult(h_, &ov, &n, FALSE)) {
            return -1;
        }
        return (int)n;
    }

    // Up to timeoutMs for the next input report. The read stays pending across
    // calls, like hidapi's, so nothing is lost between them. Bytes copied, 0 on
    // timeout, -1 on error.
    int Read(BYTE* out, int length, int timeoutMs) {
        if (!h_) {
            return -1;
        }
        if (!readPending_) {
            readOv_ = {};
            readOv_.hEvent = readEvent_;
            ResetEvent(readEvent_);
            std::fill(readBuffer_.begin(), readBuffer_.end(), 0);
            if (!ReadFile(h_, readBuffer_.data(), (DWORD)readBuffer_.size(), nullptr, &readOv_) &&
                GetLastError() != ERROR_IO_PENDING) {
                return -1;
            }
            readPending_ = true;
        }
        if (WaitForSingleObject(readEvent_, (DWORD)std::max(timeoutMs, 0)) != WAIT_OBJECT_0) {
            return 0;
        }
        readPending_ = false;
        DWORD n = 0;
        if (!GetOverlappedResult(h_, &readOv_, &n, FALSE)) {
            return -1;
        }
        int start = 0;
        if (n > 0 && readBuffer_[0] == 0) {
            start = 1;
        }
        int count = std::min(length, (int)n - start);
        if (count > 0) {
            memcpy(out, readBuffer_.data() + start, count);
        }
        return std::max(count, 0);
    }

    std::vector<BYTE> Read(int timeoutMs) {
        std::vector<BYTE> out(std::max(info.inLen, 1u));
        int n = Read(out.data(), (int)out.size(), timeoutMs);
        out.resize(n > 0 ? n : 0);
        return out;
    }

    bool SendFeature(std::vector<BYTE> data) {
        if (data.size() < info.featLen) {
            data.resize(info.featLen, 0);
        }
        return Ioctl(kIoctlSetFeature, data, false) >= 0;
    }

    // The report, id first; empty on failure.
    std::vector<BYTE> GetFeature(BYTE id) {
        std::vector<BYTE> data(std::max(info.featLen, 2u), 0);
        data[0] = id;
        int n = Ioctl(kIoctlGetFeature, data, true);
        if (n < 0) {
            return {};
        }
        return data;
    }

    bool SetOutput(std::vector<BYTE> data) {
        if (data.size() < info.outLen) {
            data.resize(info.outLen, 0);
        }
        return Ioctl(kIoctlSetOutput, data, false) >= 0;
    }

    std::vector<BYTE> GetInput(BYTE id) {
        std::vector<BYTE> data(std::max(info.inLen, 2u), 0);
        data[0] = id;
        int n = Ioctl(kIoctlGetInput, data, true);
        if (n < 0) {
            return {};
        }
        return data;
    }

   private:
    int Ioctl(DWORD code, std::vector<BYTE>& data, bool reply) {
        if (!h_) {
            return -1;
        }
        OVERLAPPED ov{};
        ov.hEvent = ioEvent_;
        ResetEvent(ioEvent_);
        DWORD n = 0;
        BOOL ok = reply ? DeviceIoControl(h_, code, data.data(), (DWORD)data.size(), data.data(),
                                          (DWORD)data.size(), nullptr, &ov)
                        : DeviceIoControl(h_, code, data.data(), (DWORD)data.size(), nullptr, 0, nullptr, &ov);
        if (!ok && GetLastError() != ERROR_IO_PENDING) {
            return -1;
        }
        if (WaitForSingleObject(ioEvent_, 1000) != WAIT_OBJECT_0) {
            CancelIoEx(h_, &ov);
            GetOverlappedResult(h_, &ov, &n, TRUE);
            return -1;
        }
        if (!GetOverlappedResult(h_, &ov, &n, FALSE)) {
            return -1;
        }
        return (int)n;
    }

    HANDLE h_ = nullptr;
    HANDLE readEvent_ = nullptr;
    HANDLE ioEvent_ = nullptr;
    OVERLAPPED readOv_{};
    bool readPending_ = false;
    std::vector<BYTE> readBuffer_;
};

// One provider's last good readings, for devices that fall asleep: the level
// stays, greyed out, for a while.
struct LastSeen {
    std::map<std::wstring, std::pair<Device, ULONGLONG>> last;

    void Remember(Device const& d) { last[d.key] = {d, GetTickCount64()}; }

    // Devices remembered but missing from found, still within the window.
    void KeepAsleep(std::vector<Device>& out, std::vector<std::wstring> const& found) {
        ULONGLONG now = GetTickCount64();
        for (auto it = last.begin(); it != last.end();) {
            bool present = std::find(found.begin(), found.end(), it->first) != found.end();
            if (present) {
                ++it;
                continue;
            }
            if (now - it->second.second < kAsleepKeepMs) {
                Device d = it->second.first;
                d.online = false;
                out.push_back(d);
                ++it;
            } else {
                it = last.erase(it);
            }
        }
    }
};

////////////////////////////////////////////////////////////////////////////////
// Alienware: the Pro Wireless headset's dongle (413C:A529) and the AW720M
// mouse (413C:3029). Worked out by watching Alienware Command Center's
// traffic, and verified on both devices.
//
// Headset, vendor collection 0xFF13: output report 06 "c0 0a", then input
// report 07 polled until it echoes "c0 0a" ("07 00 80" is busy). The answer
// is 07 c0 0a <len> 00 <data>: data[0] (byte 5) reads 0 on battery, data[1]
// (byte 6) is the percentage, bytes 9-10 the cell voltage in mV.
// Mouse, vendor collection 0xFF00 on interface 2: "00 c0 0a" written, a report
// echoing "c0 0a" read back, with the percentage after a flag byte. A reply
// three bytes long means the mouse isn't answering (asleep). "c0" is a read;
// commands starting "40" change settings and are never sent.

constexpr unsigned kDellVid = 0x413C;

std::vector<Device> PollAlienware(std::vector<std::wstring> const& paths, LastSeen& asleep) {
    std::vector<Device> out;
    std::vector<std::wstring> found;
    if (!VendorPresent(paths, kDellVid)) {
        asleep.KeepAsleep(out, found);
        return out;
    }
    for (auto const& info : Enumerate(paths, kDellVid)) {
        if (info.pid == 0xA529 && info.usagePage == 0xFF13) {
            HidDevice dev;
            if (!dev.Open(info) || !dev.SetOutput({0x06, 0xC0, 0x0A})) {
                continue;
            }
            for (int i = 0; i < 40; i++) {
                Sleep(16);
                auto in = dev.GetInput(0x07);
                if (in.size() >= 11 && in[1] == 0xC0 && in[2] == 0x0A && in[3] >= 2 && in[6] <= 100) {
                    Device d;
                    d.key = L"alienware:a529";
                    d.name = L"Alienware Pro Wireless Headset";
                    d.kind = "headset";
                    d.source = "alienware";
                    d.level = in[6];
                    d.charging = in[5] != 0;
                    d.vid = info.vid;
                    d.pid = info.pid;
                    out.push_back(d);
                    found.push_back(d.key);
                    break;
                }
            }
        } else if (info.pid == 0x3029 && info.usagePage == 0xFF00 && info.iface == 2) {
            HidDevice dev;
            if (!dev.Open(info) || dev.Write({0x00, 0xC0, 0x0A}) < 0) {
                continue;
            }
            for (int i = 0; i < 10; i++) {
                auto r = dev.Read(100);
                if (r.size() < 3 || r[0] != 0xC0 || r[1] != 0x0A) {
                    continue;
                }
                if (r[2] >= 4 && r.size() >= 6 && r[5] <= 100) {
                    Device d;
                    d.key = L"alienware:3029";
                    d.name = info.product.empty() ? L"Alienware AW720M" : info.product;
                    d.kind = "mouse";
                    d.source = "alienware";
                    d.level = r[5];
                    d.charging = r[4] == 2;
                    d.vid = info.vid;
                    d.pid = info.pid;
                    asleep.Remember(d);
                    out.push_back(d);
                    found.push_back(d.key);
                }
                break;  // a short reply: not answering right now
            }
        }
    }
    asleep.KeepAsleep(out, found);
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Razer: the 90-byte feature report Synapse and OpenRazer use (power class 07:
// 80 level, 84 charging), and the BlackShark V2 Pro 2023's own "PA" protocol.

constexpr unsigned kRazerVid = 0x1532;

struct RazerModel {
    unsigned pid;
    PCWSTR name;
    BYTE tid;
};

constexpr RazerModel kRazerKnown[] = {
    {0x0528, L"Razer BlackShark V2 Pro (2020)", 0x3F}, {0x0555, L"Razer BlackShark V2 Pro (2023)", 0x3F},
    {0x0556, L"Razer BlackShark V2 Pro (2023)", 0x3F}, {0x0557, L"Razer BlackShark V2 Pro (2023)", 0x3F},
    {0x00A4, L"Razer Mouse Dock Pro", 0x1F},           {0x00AA, L"Razer Basilisk V3 Pro", 0x1F},
    {0x00AB, L"Razer Basilisk V3 Pro", 0x1F},          {0x00B9, L"Razer Basilisk V3 X HyperSpeed", 0x1F},
    {0x007C, L"Razer DeathAdder V2 Pro", 0x3F},        {0x007D, L"Razer DeathAdder V2 Pro", 0x3F},
    {0x009C, L"Razer DeathAdder V2 X HyperSpeed", 0x1F}, {0x00B3, L"Razer HyperPolling Dongle", 0x1F},
    {0x00B6, L"Razer DeathAdder V3 Pro", 0x1F},        {0x00B7, L"Razer DeathAdder V3 Pro", 0x1F},
    {0x00BE, L"Razer DeathAdder V4 Pro", 0x1F},        {0x00BF, L"Razer DeathAdder V4 Pro", 0x1F},
    {0x0083, L"Razer Basilisk X HyperSpeed", 0x1F},    {0x0086, L"Razer Basilisk Ultimate", 0x1F},
    {0x0088, L"Razer Basilisk Ultimate", 0x1F},        {0x008F, L"Razer Naga V2 Pro", 0x1F},
    {0x0090, L"Razer Naga V2 Pro", 0x1F},              {0x00A5, L"Razer Viper V2 Pro", 0x1F},
    {0x00A6, L"Razer Viper V2 Pro", 0x1F},             {0x00C0, L"Razer Viper V3 Pro", 0x1F},
    {0x00C1, L"Razer Viper V3 Pro", 0x1F},             {0x007A, L"Razer Viper Ultimate", 0xFF},
    {0x007B, L"Razer Viper Ultimate", 0xFF},           {0x0078, L"Razer Viper Ultimate", 0xFF},
    {0x00AF, L"Razer Cobra Pro", 0x1F},                {0x00B0, L"Razer Cobra Pro", 0x1F},
    {0x00CC, L"Razer Basilisk V3 Pro 35K", 0x1F},      {0x00CD, L"Razer Basilisk V3 Pro 35K", 0x1F},
};

constexpr BYTE kRazerTids[] = {0x1F, 0x3F, 0xFF, 0x9F, 0x08};
constexpr unsigned kRazerPaPids[] = {0x0555, 0x0556};

constexpr BYTE kRazerOk = 0x02;
constexpr BYTE kRazerBusy = 0x01;
constexpr BYTE kRazerTimeout = 0x04;

RazerModel const* RazerKnown(unsigned pid) {
    for (auto const& m : kRazerKnown) {
        if (m.pid == pid) {
            return &m;
        }
    }
    return nullptr;
}

bool RazerIsPa(unsigned pid) {
    return std::find(std::begin(kRazerPaPids), std::end(kRazerPaPids), pid) != std::end(kRazerPaPids);
}

// Wired Razer devices answer the battery command with nonsense, so unknown
// ones are only asked when their name suggests a battery.
bool RazerMaybeWireless(unsigned pid, std::wstring const& name) {
    std::wstring n = Lower(name);
    if (RazerKnown(pid) || RazerIsPa(pid)) {
        return true;
    }
    for (PCWSTR w : {L"hyperspeed", L"wireless", L"receiver", L"dongle", L"dock", L"blackshark", L"barracuda",
                     L"nari"}) {
        if (Contains(n, w)) {
            return true;
        }
    }
    return false;
}

std::vector<BYTE> RazerRequest(BYTE tid, BYTE cmdClass, BYTE cmdId) {
    std::vector<BYTE> msg(91, 0);  // report id 0, then the 90-byte message
    BYTE* m = msg.data() + 1;
    m[1] = tid;
    m[5] = 0x02;
    m[6] = cmdClass;
    m[7] = cmdId;
    BYTE crc = 0;
    for (int i = 2; i < 88; i++) {
        crc ^= m[i];
    }
    m[88] = crc;
    return msg;
}

// (status, value); status -1 when the collection didn't take the request.
std::pair<int, int> RazerQuery(HidDevice& dev, BYTE tid, BYTE cmdClass, BYTE cmdId) {
    if (!dev.SendFeature(RazerRequest(tid, cmdClass, cmdId))) {
        return {-1, -1};
    }
    ULONGLONG deadline = GetTickCount64() + 1000;
    Sleep(60);
    while (true) {
        auto resp = dev.GetFeature(0x00);
        if (resp.size() < 91) {
            return {-1, -1};
        }
        BYTE const* data = resp.data() + 1;
        int status = data[0];
        int value = (data[6] == cmdClass && data[7] == cmdId) ? data[9] : -1;
        // Busy, or another program's reply (Synapse polls the same collection):
        // read on.
        if (GetTickCount64() < deadline && (status == kRazerBusy || (status == kRazerOk && value < 0))) {
            Sleep(80);
            continue;
        }
        return {status, value};
    }
}

struct RazerReading {
    int status = -1;
    int level = -1;
    bool charging = false;
};

RazerReading RazerRead(HidInfo const& info, BYTE tid) {
    HidDevice dev;
    if (!dev.Open(info)) {
        return {};
    }
    auto [status, raw] = RazerQuery(dev, tid, 0x07, 0x80);
    if (status != kRazerOk || raw < 0) {
        return {status, -1, false};
    }
    RazerReading r{status, (int)std::lround(raw / 255.0 * 100), false};
    auto [cst, craw] = RazerQuery(dev, tid, 0x07, 0x84);
    r.charging = cst == kRazerOk && craw > 0;
    return r;
}

// BlackShark V2 Pro 2023 ("PA"): 64-byte output reports with report id 2.
std::vector<BYTE> PaFrameQuery(BYTE cmd) {
    std::vector<BYTE> b(64, 0);
    b[0] = 0x02;
    b[1] = 0x80;
    b[2] = 8;
    b[5] = 'P';
    b[6] = 'A';
    b[7] = 0x08;
    b[9] = 0x03;
    b[10] = cmd;
    return b;
}

std::vector<BYTE> PaFrameRemote(bool on) {
    std::vector<BYTE> b(64, 0);
    b[0] = 0x02;
    b[1] = 0x80;
    b[2] = 0x07;
    b[5] = 'P';
    b[6] = 'A';
    b[7] = 0x0E;
    b[9] = 0x02;
    b[10] = 0xE1;
    b[11] = on ? 1 : 0;
    return b;
}

std::vector<BYTE> PaParse(std::vector<BYTE> const& d, BYTE cmd) {
    for (int off : {0, -1}) {
        int c = 12 + off;
        if ((int)d.size() > c + 3 && (off == -1 || d[0] == 0x02) && d[c] == cmd && d[c + 1] == 0x01) {
            int n = std::max((int)d[c + 2], 1);
            if ((int)d.size() >= c + 3 + n) {
                return std::vector<BYTE>(d.begin() + c + 3, d.begin() + c + 3 + n);
            }
        }
    }
    return {};
}

std::vector<BYTE> PaQuery(HidDevice& dev, BYTE cmd) {
    for (int i = 0; i < 32 && !dev.Read(5).empty(); i++) {
    }
    dev.Write(PaFrameRemote(true));  // wakes the radio link
    Sleep(35);
    std::vector<BYTE> result;
    for (int attempt = 0; attempt < 3 && result.empty(); attempt++) {
        dev.Write(PaFrameRemote(true));
        Sleep(35);
        if (dev.Write(PaFrameQuery(cmd)) < 0) {
            break;
        }
        ULONGLONG deadline = GetTickCount64() + 150;
        while (GetTickCount64() < deadline) {
            auto data = dev.Read(50);
            if (data.empty()) {
                continue;
            }
            result = PaParse(data, cmd);
            if (!result.empty()) {
                break;
            }
        }
    }
    dev.Write(PaFrameRemote(false));
    Sleep(35);
    return result;
}

// "ok", "offline" or "fail".
std::string PaRead(HidInfo const& info, int* level, bool* charging) {
    HidDevice dev;
    if (!dev.Open(info)) {
        return "fail";
    }
    bool woke = false;
    for (int attempt = 0; attempt < 4; attempt++) {
        if (dev.Write(PaFrameRemote(true)) >= 0) {
            woke = true;
            break;
        }
        Sleep(150 * (attempt + 1));
    }
    if (!woke) {
        return "offline";
    }
    Sleep(35);
    auto battery = PaQuery(dev, 0x21);
    if (battery.empty()) {
        return "offline";
    }
    *level = std::min((int)battery[0], 100);
    auto charge = PaQuery(dev, 0x2A);
    *charging = !charge.empty() && charge[0];
    return "ok";
}

struct RazerProvider {
    std::map<std::wstring, std::pair<std::wstring, BYTE>> cache;  // group -> working path, tid
    std::map<std::wstring, ULONGLONG> dead;                       // paths that never answer
    LastSeen asleep;

    RazerReading PollGroup(std::wstring const& group, std::vector<HidInfo> ifaces, BYTE preferred) {
        auto cached = cache.find(group);
        if (cached != cache.end()) {
            for (auto const& info : ifaces) {
                if (info.path == cached->second.first) {
                    auto r = RazerRead(info, cached->second.second);
                    if (r.status == kRazerOk || r.status == kRazerTimeout) {
                        return r;
                    }
                }
            }
            cache.erase(group);
        }
        std::stable_sort(ifaces.begin(), ifaces.end(), [](HidInfo const& a, HidInfo const& b) {
            auto rank = [](HidInfo const& d) {
                int iface = d.iface < 0 ? 99 : d.iface;
                return std::make_pair(d.usagePage == 0x0001 || d.usagePage == 0xFF00 ? 0 : 1, iface);
            };
            return rank(a) < rank(b);
        });
        std::vector<BYTE> tids;
        if (preferred) {
            tids.push_back(preferred);
        }
        for (BYTE t : kRazerTids) {
            if (std::find(tids.begin(), tids.end(), t) == tids.end()) {
                tids.push_back(t);
            }
        }
        ULONGLONG now = GetTickCount64();
        RazerReading timeoutHit;
        for (auto const& info : ifaces) {
            auto d = dead.find(info.path);
            if (d != dead.end() && d->second > now) {
                continue;
            }
            bool answered = false;
            for (BYTE tid : tids) {
                auto r = RazerRead(info, tid);
                if (r.status < 0) {
                    break;
                }
                answered = true;
                if (r.status == kRazerOk) {
                    cache[group] = {info.path, tid};
                    return r;
                }
                if (r.status == kRazerTimeout) {
                    timeoutHit = r;
                    cache[group] = {info.path, tid};
                    break;
                }
            }
            if (!answered) {
                dead[info.path] = now + 300 * 1000;
            }
        }
        return timeoutHit;
    }

    RazerReading PollPa(std::vector<HidInfo> const& ifaces) {
        for (auto const& info : ifaces) {
            if (info.usagePage != 0xFF00 && info.usagePage != 0xFF14) {
                continue;
            }
            int level = -1;
            bool charging = false;
            std::string res = PaRead(info, &level, &charging);
            if (res == "ok") {
                return {kRazerOk, level, charging};
            }
            if (res == "offline") {
                return {kRazerTimeout, -1, false};
            }
        }
        return {};
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        std::vector<std::wstring> found;
        if (!VendorPresent(paths, kRazerVid)) {
            asleep.KeepAsleep(out, found);
            return out;
        }
        std::map<std::pair<unsigned, std::wstring>, std::vector<HidInfo>> groups;
        for (auto const& info : Enumerate(paths, kRazerVid)) {
            groups[{info.pid, info.serial}].push_back(info);
        }
        // Collections Windows re-parents (Synapse's virtual children) have no
        // serial: they belong to the one real device of that product.
        for (auto it = groups.begin(); it != groups.end();) {
            if (it->first.second.empty()) {
                std::vector<std::wstring> serials;
                for (auto const& [k, v] : groups) {
                    if (k.first == it->first.first && !k.second.empty()) {
                        serials.push_back(k.second);
                    }
                }
                if (serials.size() == 1) {
                    auto& target = groups[{it->first.first, serials[0]}];
                    target.insert(target.end(), it->second.begin(), it->second.end());
                    it = groups.erase(it);
                    continue;
                }
            }
            ++it;
        }
        for (auto const& [id, ifaces] : groups) {
            unsigned pid = id.first;
            auto known = RazerKnown(pid);
            std::wstring name = known ? known->name : ifaces[0].product;
            if (name.empty()) {
                WCHAR buffer[32];
                swprintf_s(buffer, L"Razer %04x", pid);
                name = buffer;
            }
            if (!RazerMaybeWireless(pid, name)) {
                continue;
            }
            WCHAR keyBuffer[64];
            swprintf_s(keyBuffer, L"razer:%04x:", pid);
            std::wstring key = keyBuffer + id.second;
            bool headset = RazerIsPa(pid) || Contains(Lower(name), L"blackshark");
            RazerReading r;
            if (RazerIsPa(pid)) {
                r = PollPa(ifaces);
                if (r.status < 0) {
                    r = PollGroup(key, ifaces, known ? known->tid : 0);
                }
            } else {
                r = PollGroup(key, ifaces, known ? known->tid : 0);
                if (headset && r.status != kRazerOk) {
                    auto pa = PollPa(ifaces);
                    if (pa.status >= 0 && (r.status < 0 || pa.status == kRazerOk)) {
                        r = pa;
                    }
                }
            }
            if (r.status != kRazerOk) {
                continue;  // asleep mice keep their level through `asleep`; headsets go
            }
            Device d;
            d.key = key;
            d.name = name;
            d.kind = headset ? "headset" : "mouse";
            d.source = "razer";
            d.level = r.level;
            d.charging = r.charging;
            d.vid = kRazerVid;
            d.pid = pid;
            if (!headset) {
                asleep.Remember(d);
            }
            out.push_back(d);
            found.push_back(key);
        }
        asleep.KeepAsleep(out, found);
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Logitech: HID++ 2.0 on a Lightspeed or Unifying receiver's vendor
// collections (0xFF00, usages 1 short and 2 long), or on the device's cable.
// Battery from feature 0x1004 unified, 0x1000 status or 0x1001 voltage.

constexpr unsigned kLogitechVid = 0x046D;
constexpr BYTE kSwid = 0x0A;

int VoltageToPercent(int mv) {
    static constexpr int curve[][2] = {{4186, 100}, {4067, 90}, {3989, 80}, {3922, 70}, {3859, 60},
                                       {3811, 50},  {3778, 40}, {3751, 30}, {3717, 20}, {3671, 10},
                                       {3646, 5},   {3579, 2},  {3500, 0}};
    if (mv >= curve[0][0]) {
        return 100;
    }
    for (size_t i = 0; i + 1 < std::size(curve); i++) {
        auto hi = curve[i];
        auto lo = curve[i + 1];
        if (mv >= lo[0]) {
            return (int)std::lround(lo[1] + (mv - lo[0]) * (double)(hi[1] - lo[1]) / (hi[0] - lo[0]));
        }
    }
    return 0;
}

struct HidppChannel {
    HidDevice longDev;
    HidDevice shortDev;
    bool haveShort = false;
    bool error = false;

    bool Open(HidInfo const& longInfo, HidInfo const* shortInfo) {
        if (!longDev.Open(longInfo)) {
            return false;
        }
        if (shortInfo) {
            haveShort = shortDev.Open(*shortInfo);
        }
        return true;
    }

    // The reply's parameters (16 bytes and zero padding), or empty.
    std::vector<BYTE> Request(BYTE idx, BYTE feature, BYTE func, std::vector<BYTE> params = {},
                              DWORD timeoutMs = 600) {
        error = false;
        std::vector<BYTE> req = {0x11, idx, feature, (BYTE)((func << 4) | kSwid)};
        req.insert(req.end(), params.begin(), params.end());
        req.resize(20, 0);
        if (longDev.Write(req) < 0) {
            return {};
        }
        ULONGLONG end = GetTickCount64() + timeoutMs;
        while (GetTickCount64() < end) {
            for (HidDevice* d : {&longDev, &shortDev}) {
                if (d == &shortDev && !haveShort) {
                    continue;
                }
                auto r = d->Read(0);
                if (r.size() < 4 || r[1] != idx) {
                    continue;
                }
                if ((r[2] == 0x8F || r[2] == 0xFF) && r[3] == feature) {
                    error = true;
                    return {};
                }
                if (r[2] == feature && r[3] == (BYTE)((func << 4) | kSwid)) {
                    std::vector<BYTE> p(r.begin() + 4, r.end());
                    p.resize(p.size() + 16, 0);
                    return p;
                }
            }
            Sleep(5);
        }
        return {};
    }

    BYTE FeatureIndex(BYTE idx, unsigned featureId) {
        auto r = Request(idx, 0, 0, {(BYTE)(featureId >> 8), (BYTE)(featureId & 0xFF)});
        return r.empty() ? 0 : r[0];
    }
};

struct LogitechIdentity {
    std::wstring name;
    std::string kind;
    std::wstring unit;
};

struct LogitechProvider {
    std::map<std::pair<unsigned, int>, LogitechIdentity> ids;
    std::set<std::pair<unsigned, int>> sleeping;
    LastSeen asleep;

    LogitechIdentity Identity(HidppChannel& ch, BYTE idx) {
        LogitechIdentity id;
        BYTE fi = ch.FeatureIndex(idx, 0x0005);
        if (fi) {
            auto r = ch.Request(idx, fi, 0);
            int length = r.empty() ? 0 : r[0];
            std::string raw;
            while ((int)raw.size() < length) {
                auto part = ch.Request(idx, fi, 1, {(BYTE)raw.size()});
                if (part.empty()) {
                    break;
                }
                raw.append((char const*)part.data(), 16);
            }
            raw.resize(std::min((int)raw.size(), length));
            int n = MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), (int)raw.size(), nullptr, 0);
            id.name.resize(n);
            MultiByteToWideChar(CP_UTF8, 0, raw.c_str(), (int)raw.size(), id.name.data(), n);
            auto type = ch.Request(idx, fi, 2);
            if (!type.empty()) {
                switch (type[0]) {
                    case 0:
                    case 2: id.kind = "keyboard"; break;
                    case 3:
                    case 4:
                    case 5: id.kind = "mouse"; break;
                }
            }
        }
        fi = ch.FeatureIndex(idx, 0x0003);
        if (fi) {
            auto r = ch.Request(idx, fi, 0);
            if (r.size() >= 5 && (r[1] | r[2] | r[3] | r[4])) {
                WCHAR hex[16];
                swprintf_s(hex, L"%02X%02X%02X%02X", r[1], r[2], r[3], r[4]);
                id.unit = hex;
            }
        }
        return id;
    }

    std::optional<Device> ReadSlot(HidppChannel& ch, unsigned pid, BYTE idx) {
        auto slot = std::make_pair(pid, (int)idx);
        // A paired device that's asleep doesn't answer at all; once a slot has
        // gone quiet, ask it with the short timeout until it answers again.
        DWORD timeout = sleeping.count(slot) ? 600 : 2000;
        if (ch.Request(idx, 0x00, 1, {}, timeout).empty()) {
            if (ch.error) {
                sleeping.erase(slot);
            } else {
                sleeping.insert(slot);
            }
            return std::nullopt;
        }
        sleeping.erase(slot);
        if (!ids.count(slot)) {
            ids[slot] = Identity(ch, idx);
        }
        auto const& id = ids[slot];
        for (unsigned feature : {0x1004u, 0x1000u, 0x1001u}) {
            BYTE fi = ch.FeatureIndex(idx, feature);
            if (!fi) {
                continue;
            }
            auto p = ch.Request(idx, fi, feature == 0x1004 ? 1 : 0);
            if (p.size() < 3) {
                continue;
            }
            int level = -1;
            bool charging = false;
            if (feature == 0x1004) {
                level = p[0] <= 100 ? p[0] : -1;
                charging = p[2] >= 1 && p[2] <= 3;
            } else if (feature == 0x1000) {
                level = p[0] > 0 && p[0] <= 100 ? p[0] : -1;
                charging = p[2] >= 1 && p[2] <= 3;
            } else {
                int mv = (p[0] << 8) | p[1];
                if (mv >= 2500) {
                    level = VoltageToPercent(mv);
                    charging = (p[2] & 0x80) != 0;
                }
            }
            if (level < 0) {
                continue;
            }
            Device d;
            WCHAR key[48];
            if (!id.unit.empty()) {
                swprintf_s(key, L"logitech:%ls", id.unit.c_str());
            } else {
                swprintf_s(key, L"logitech:%04x:%d", pid, idx);
            }
            d.key = key;
            d.name = id.name.empty() ? L"Logitech device" : id.name;
            d.kind = id.kind;
            d.source = "logitech";
            d.level = level;
            d.charging = charging;
            d.vid = kLogitechVid;
            d.pid = pid;
            return d;
        }
        return std::nullopt;
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        std::vector<std::wstring> foundKeys;
        if (!VendorPresent(paths, kLogitechVid)) {
            asleep.KeepAsleep(out, foundKeys);
            return out;
        }
        auto infos = Enumerate(paths, kLogitechVid);
        std::map<unsigned, std::map<unsigned, HidInfo>> groups;  // pid -> usage -> info
        for (auto const& info : infos) {
            if (info.usagePage == 0xFF00 && (info.usage == 1 || info.usage == 2)) {
                groups[info.pid][info.usage] = info;
            }
        }
        std::map<std::wstring, Device> found;
        for (auto& [pid, byUsage] : groups) {
            if (!byUsage.count(2)) {
                continue;
            }
            std::wstring product;
            for (auto const& info : infos) {
                if (info.pid == pid && !info.product.empty()) {
                    product = info.product;
                    break;
                }
            }
            bool receiver = Contains(Lower(product), L"receiver");
            HidppChannel ch;
            if (!ch.Open(byUsage[2], byUsage.count(1) ? &byUsage[1] : nullptr)) {
                continue;
            }
            std::vector<BYTE> indexes;
            if (receiver) {
                indexes = {1, 2, 3, 4, 5, 6};
            } else {
                indexes = {0xFF};
            }
            for (BYTE idx : indexes) {
                auto d = ReadSlot(ch, pid, idx);
                // The same device on the cable and through the receiver: charging wins.
                if (d && (!found.count(d->key) || d->charging)) {
                    found[d->key] = *d;
                }
            }
        }
        for (auto& [key, d] : found) {
            asleep.Remember(d);
            out.push_back(d);
            foundKeys.push_back(key);
        }
        if (!groups.empty()) {
            asleep.KeepAsleep(out, foundKeys);
        }
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// SteelSeries: Arctis Nova headsets (output report "00 b0" on interface 3,
// usage page 0xFFC0) and Rival 3 Wireless mice ("00 aa 01", aa echo), as
// documented by HeadsetControl and steel-mouse.

constexpr unsigned kSteelSeriesVid = 0x1038;

enum class NovaLayout { k7, k7Discrete, k5 };

struct SteelSeriesModel {
    unsigned pid;
    PCWSTR name;
    NovaLayout layout;
};

constexpr SteelSeriesModel kNovaModels[] = {
    {0x22A1, L"Arctis Nova 7", NovaLayout::k7},        {0x2202, L"Arctis Nova 7", NovaLayout::k7Discrete},
    {0x227E, L"Arctis Nova 7 Gen 2", NovaLayout::k7},  {0x2206, L"Arctis Nova 7x", NovaLayout::k7Discrete},
    {0x2258, L"Arctis Nova 7x", NovaLayout::k7},       {0x229E, L"Arctis Nova 7x", NovaLayout::k7},
    {0x22AD, L"Arctis Nova 7x", NovaLayout::k7},       {0x22A4, L"Arctis Nova 7X", NovaLayout::k7Discrete},
    {0x22A5, L"Arctis Nova 7X", NovaLayout::k7},       {0x223A, L"Arctis Nova 7 Diablo IV", NovaLayout::k7Discrete},
    {0x22A9, L"Arctis Nova 7 Diablo IV", NovaLayout::k7}, {0x227A, L"Arctis Nova 7 WoW Edition", NovaLayout::k7Discrete},
    {0x2232, L"Arctis Nova 5", NovaLayout::k5},        {0x2253, L"Arctis Nova 5X", NovaLayout::k5},
};

constexpr std::pair<unsigned, PCWSTR> kRivalModels[] = {
    {0x1830, L"SteelSeries Rival 3 Wireless"},
    {0x1872, L"SteelSeries Rival 3 Wireless Gen 2"},
};

struct SteelSeriesProvider {
    std::vector<BYTE> ReadNova(HidInfo const& info) {
        HidDevice dev;
        if (!dev.Open(info) || dev.Write({0x00, 0xB0}) < 0) {
            return {};
        }
        std::vector<BYTE> first;
        ULONGLONG end = GetTickCount64() + 1000;
        while (GetTickCount64() < end) {
            auto r = dev.Read(100);
            if (r.empty()) {
                continue;
            }
            if (r[0] == 0xB0) {
                return r;
            }
            if (first.empty()) {
                first = r;
            }
        }
        return first;
    }

    std::vector<BYTE> ReadRival(HidInfo const& info) {
        HidDevice dev;
        if (!dev.Open(info)) {
            return {};
        }
        std::vector<BYTE> request = {0x00, 0xAA, 0x01};
        request.resize(64, 0);
        for (int attempt = 0; attempt < 3; attempt++) {
            if (dev.Write(request) < 0) {
                continue;
            }
            for (int i = 0; i < 6; i++) {
                auto r = dev.Read(100);
                if (r.empty()) {
                    continue;
                }
                if (r[0] == 0xAA || (r[0] == 0x00 && r.size() > 1 && r[1] == 0xAA)) {
                    return r;
                }
                break;
            }
        }
        return {};
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        if (!VendorPresent(paths, kSteelSeriesVid)) {
            return out;
        }
        std::set<unsigned> seen;
        for (auto const& info : Enumerate(paths, kSteelSeriesVid)) {
            if (seen.count(info.pid) || info.iface != 3) {
                continue;
            }
            for (auto const& [pid, name] : kRivalModels) {
                if (pid != info.pid) {
                    continue;
                }
                seen.insert(pid);
                auto r = ReadRival(info);
                size_t m = !r.empty() && r[0] == 0x00 && r.size() > 1 ? 1 : 0;
                if (r.size() >= m + 4 && r[m] == 0xAA && r[m + 1] <= 100) {
                    Device d;
                    WCHAR key[32];
                    swprintf_s(key, L"steelseries:%04x", pid);
                    d.key = key;
                    d.name = name;
                    d.kind = "mouse";
                    d.source = "steelseries";
                    d.level = r[m + 1];
                    d.charging = r[m + 3] != 0;
                    out.push_back(d);
                }
            }
            for (auto const& model : kNovaModels) {
                if (model.pid != info.pid) {
                    continue;
                }
                seen.insert(model.pid);
                auto r = ReadNova(info);
                int level = -1;
                bool charging = false;
                if (model.layout == NovaLayout::k5) {
                    if (r.size() >= 5 && r[0] == 0xB0 && r[1] != 0x02) {
                        level = std::min((int)r[3], 100);
                        charging = r[4] == 0x01;
                    }
                } else if (r.size() >= 4 && r[0] == 0xB0 && r[3] != 0x00) {
                    level = std::min((int)r[2], 100);
                    if (model.layout == NovaLayout::k7Discrete) {
                        level = std::min(level, 4) * 25;
                    }
                    charging = r[3] == 0x01 || r[3] == 0x02;
                }
                if (level >= 0) {
                    Device d;
                    WCHAR key[32];
                    swprintf_s(key, L"steelseries:%04x", model.pid);
                    d.key = key;
                    d.name = model.name;
                    d.kind = "headset";
                    d.source = "steelseries";
                    d.level = level;
                    d.charging = charging;
                    out.push_back(d);
                }
            }
        }
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// HyperX Cloud II Wireless: vendor collection 0xFF90:0x0303, request
// "06 ff bb <cmd>", reply echoing it (cmd 02: level at byte 7; 03: charging).

constexpr unsigned kHyperXVid = 0x03F0;

std::vector<Device> PollHyperX(std::vector<std::wstring> const& paths) {
    std::vector<Device> out;
    if (!VendorPresent(paths, kHyperXVid)) {
        return out;
    }
    std::set<std::wstring> seen;
    for (unsigned pid : {0x0696u, 0x018Bu}) {
        for (auto const& info : Enumerate(paths, kHyperXVid)) {
            if (info.pid != pid || seen.count(info.path) || info.usagePage != 0xFF90 || info.usage != 0x0303) {
                continue;
            }
            seen.insert(info.path);
            auto query = [&](BYTE cmd) -> std::vector<BYTE> {
                HidDevice dev;
                if (!dev.Open(info)) {
                    return {};
                }
                std::vector<BYTE> req = {0x06, 0xFF, 0xBB, cmd, 0x00};
                req.resize(52, 0);
                if (dev.Write(req) < 0) {
                    return {};
                }
                Sleep(100);
                return dev.Read(1000);
            };
            auto r = query(0x02);
            if (r.size() <= 7 || r[0] != 0x06 || r[1] != 0xFF || r[2] != 0xBB || r[3] != 0x02 || r[7] > 100) {
                continue;
            }
            Device d;
            WCHAR key[32];
            swprintf_s(key, L"hyperx:%04x", pid);
            d.key = key;
            d.name = L"HyperX Cloud II Wireless";
            d.kind = "headset";
            d.source = "hyperx";
            d.level = r[7];
            auto c = query(0x03);
            d.charging = c.size() > 4 && c[0] == 0x06 && c[1] == 0xFF && c[2] == 0xBB && c[3] == 0x03 && c[4] == 1;
            out.push_back(d);
        }
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Audeze Maxwell: vendor collection 0xFF13, output report 06, input report 07
// holding a rolling buffer; the battery is the byte after "d6 0c 00 00". The
// cable endpoint answering means the headset is charging.

constexpr unsigned kAudezeVid = 0x3329;

constexpr BYTE kAudezeInit[][10] = {
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x20}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x25},
    {0x06, 0x07, 0x80, 0x05, 0x5A, 0x03, 0x00, 0x07, 0x1C, 0x00}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x28},
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x83, 0x2C, 0x01}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x83, 0x2C, 0x07},
    {0x06, 0x07, 0x00, 0x05, 0x5A, 0x03, 0x00, 0x07, 0x1C, 0x00}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x2D},
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x2C}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x00},
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x83, 0x2C, 0x0B}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x24},
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x2F}, {0x06, 0x07, 0x80, 0x05, 0x5A, 0x03, 0x00, 0xD6, 0x0C, 0x00},
};
constexpr BYTE kAudezeStatus[][10] = {
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x22}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x00},
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x83, 0x2C, 0x0B}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x24},
    {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x01, 0x09, 0x2C}, {0x06, 0x08, 0x80, 0x05, 0x5A, 0x04, 0x00, 0x83, 0x2C, 0x07},
};

int AudezeFindLevel(std::vector<BYTE> const& b) {
    for (size_t i = 0; i + 4 < b.size(); i++) {
        if (b[i] == 0xD6 && b[i + 1] == 0x0C && b[i + 2] == 0x00 && b[i + 3] == 0x00 && b[i + 4] <= 100) {
            return b[i + 4];
        }
    }
    return -1;
}

struct AudezeProvider {
    std::map<std::pair<unsigned, std::wstring>, bool> shortUseless;

    // Frames read after sending the packets; answered false when nothing came.
    std::vector<std::vector<BYTE>> Sequence(HidDevice& dev, std::vector<std::vector<BYTE>> const& requests,
                                            bool* answered) {
        std::vector<std::vector<BYTE>> frames;
        int unanswered = 0;
        *answered = true;
        for (auto const& req : requests) {
            std::vector<BYTE> packet = req;
            packet.resize(62, 0);
            if (dev.Write(packet) < 0) {
                break;
            }
            Sleep(60);
            auto frame = dev.GetInput(0x07);
            if (!frame.empty()) {
                frames.push_back(frame);
            } else if (frames.empty() && ++unanswered >= 3) {
                *answered = false;
                return frames;
            }
        }
        for (int i = 0; i < 2; i++) {
            Sleep(60);
            auto frame = dev.GetInput(0x07);
            if (!frame.empty()) {
                frames.push_back(frame);
            }
        }
        return frames;
    }

    int ReadBattery(HidInfo const& info, std::pair<unsigned, std::wstring> const& state) {
        HidDevice dev;
        if (!dev.Open(info)) {
            return -1;
        }
        auto newest = [](std::vector<std::vector<BYTE>> const& frames) {
            for (auto it = frames.rbegin(); it != frames.rend(); ++it) {
                int level = AudezeFindLevel(*it);
                if (level >= 0) {
                    return level;
                }
            }
            return -1;
        };
        bool shortFirst = !shortUseless[state];
        bool answered = true;
        int level = -1;
        if (shortFirst) {
            std::vector<std::vector<BYTE>> one = {{0x06, 0x07, 0x80, 0x05, 0x5A, 0x03, 0x00, 0xD6, 0x0C}};
            level = newest(Sequence(dev, one, &answered));
        }
        if (level < 0 && answered) {
            std::vector<std::vector<BYTE>> all;
            for (auto const& p : kAudezeInit) {
                all.push_back(std::vector<BYTE>(p, p + (p[1] == 0x07 ? 9 : 10)));
            }
            for (auto const& p : kAudezeStatus) {
                all.push_back(std::vector<BYTE>(p, p + 10));
            }
            // The two nine-byte packets in the table end at 0x09: trim them back.
            for (auto& p : all) {
                if (p.size() == 10 && p[7] == 0x01 && p[8] == 0x09 && p[9] == 0x00) {
                    p.resize(9);
                }
            }
            level = newest(Sequence(dev, all, &answered));
        }
        if (level >= 0) {
            shortUseless[state] = false;
        } else if (shortFirst) {
            shortUseless[state] = true;
        }
        return level;
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        if (!VendorPresent(paths, kAudezeVid)) {
            return out;
        }
        std::map<std::pair<unsigned, std::wstring>, std::vector<HidInfo>> groups;
        for (auto const& info : Enumerate(paths, kAudezeVid)) {
            groups[{info.pid, info.serial}].push_back(info);
        }
        auto isCable = [](unsigned pid) { return pid == 0x4B1A || pid == 0x4B1E; };
        std::vector<std::pair<std::pair<unsigned, std::wstring>, std::vector<HidInfo>>> order(groups.begin(),
                                                                                             groups.end());
        std::stable_sort(order.begin(), order.end(),
                         [&](auto const& a, auto const& b) { return isCable(a.first.first) > isCable(b.first.first); });
        for (auto const& [id, ifaces] : order) {
            bool off = false;
            if (!isCable(id.first)) {
                for (auto const& info : ifaces) {
                    if (Lower(info.product) == L"audeze maxwell dongle") {
                        off = true;
                    }
                }
            }
            if (off) {
                continue;
            }
            std::vector<HidInfo> cands;
            for (auto const& info : ifaces) {
                if (info.usagePage == 0xFF13 && info.usage == 0x0001) {
                    cands.push_back(info);
                }
            }
            if (cands.empty()) {
                for (auto const& info : ifaces) {
                    if (info.usagePage == 0xFF13) {
                        cands.push_back(info);
                    }
                }
            }
            if (cands.empty()) {
                cands = ifaces;
            }
            int level = -1;
            for (auto const& info : cands) {
                level = ReadBattery(info, id);
                if (level >= 0) {
                    break;
                }
            }
            if (level < 0) {
                continue;
            }
            Device d;
            d.key = L"audeze:" + id.second;
            d.name = L"Audeze Maxwell";
            d.kind = "headset";
            d.source = "audeze";
            d.level = level;
            d.charging = isCable(id.first);
            out.push_back(d);
            break;  // one icon per headset: the cable was read first
        }
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// MCHOSE mice: feature reports 0x11 or 0x12 on the 0xFF01 collection, every
// payload byte inverted, command 0x06 (vid, model, firmware, flags, level,
// charging). The G7 (0xA8A5:0x2255): "00 55 30 a5 0b 2e 01 01 01" out, a
// report starting "aa 30" with the level at byte 8 and charging at byte 9.

constexpr unsigned kMchoseVids[] = {0x5253, 0x3837};
constexpr unsigned kG7Vid = 0xA8A5;

struct MchoseProvider {
    LastSeen asleep;
    std::map<std::wstring, std::wstring> names;

    // (level, charging) or level -1.
    std::pair<int, bool> ReadCollection(HidInfo const& info) {
        HidDevice dev;
        if (!dev.Open(info)) {
            return {-1, false};
        }
        for (auto [report, length] : {std::pair<BYTE, int>{0x11, 20}, {0x12, 64}}) {
            std::vector<BYTE> req(1 + length, 0xFF);
            req[0] = report;
            req[1] = (BYTE)(0x06 ^ 0xFF);
            for (int attempt = 0; attempt < 4; attempt++) {
                if (!dev.SendFeature(req)) {
                    break;
                }
                Sleep(120);
                auto r = dev.GetFeature(report);
                if (r.size() < 13 || (r[0] != 0x11 && r[0] != 0x12) || r[1] != (0x06 ^ 0xFF)) {
                    continue;
                }
                std::vector<BYTE> pay;
                for (size_t i = 2; i < r.size(); i++) {
                    pay.push_back(r[i] ^ 0xFF);
                }
                if (pay.size() < 11) {
                    continue;
                }
                unsigned vid = pay[0] | (pay[1] << 8);
                if (vid != 0x5253 && vid != 0x3837) {
                    continue;
                }
                if (pay[9] > 100) {
                    continue;
                }
                return {pay[9], pay[10] != 0};
            }
        }
        return {-1, false};
    }

    std::pair<int, bool> ReadG7(HidInfo const& info) {
        HidDevice dev;
        if (!dev.Open(info)) {
            return {-1, false};
        }
        std::vector<BYTE> req = {0x00, 0x55, 0x30, 0xA5, 0x0B, 0x2E, 0x01, 0x01, 0x01};
        req.resize(65, 0);
        if (dev.Write(req) < 0) {
            return {-1, false};
        }
        for (int i = 0; i < 25; i++) {
            Sleep(20);
            auto r = dev.Read(0);
            if (r.size() > 9 && r[0] == 0xAA && r[1] == 0x30 && r[8] <= 100) {
                return {r[8], r[9] != 0};
            }
        }
        return {-1, false};
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        std::vector<std::wstring> foundKeys;
        std::vector<HidInfo> infos;
        for (unsigned vid : {kMchoseVids[0], kMchoseVids[1], kG7Vid}) {
            if (VendorPresent(paths, vid)) {
                auto more = Enumerate(paths, vid);
                infos.insert(infos.end(), more.begin(), more.end());
            }
        }
        std::map<std::pair<unsigned, unsigned>, std::vector<HidInfo>> groups;
        for (auto const& info : infos) {
            groups[{info.vid, info.pid}].push_back(info);
        }
        std::map<std::wstring, std::pair<int, bool>> found;
        for (auto& [id, ifaces] : groups) {
            std::wstring key = id.first == 0x5253 ? L"mchose" : L"mchose:" + std::to_wstring(id.first);
            if (!ifaces[0].product.empty()) {
                names[key] = ifaces[0].product;
            }
            std::vector<HidInfo> cols;
            for (auto const& info : ifaces) {
                if (info.usagePage >= 0xFF00 && (id.first != kG7Vid || info.usagePage == 0xFF01)) {
                    cols.push_back(info);
                }
            }
            std::stable_sort(cols.begin(), cols.end(), [](HidInfo const& a, HidInfo const& b) {
                return std::make_pair(a.usagePage != 0xFF01, a.usage != 1) <
                       std::make_pair(b.usagePage != 0xFF01, b.usage != 1);
            });
            for (auto const& info : cols) {
                auto r = id.first == kG7Vid ? ReadG7(info) : ReadCollection(info);
                if (r.first >= 0) {
                    // On the cable and on the dongle: charging wins.
                    if (!found.count(key) || r.second) {
                        found[key] = r;
                    }
                    break;
                }
            }
        }
        for (auto& [key, r] : found) {
            Device d;
            d.key = key;
            d.name = names.count(key) ? names[key] : L"MCHOSE mouse";
            d.kind = "mouse";
            d.source = "mchose";
            d.level = r.first;
            d.charging = r.second;
            asleep.Remember(d);
            out.push_back(d);
            foundKeys.push_back(key);
        }
        asleep.KeepAsleep(out, foundKeys);
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// WLmouse: feature report 0 "00 00 02 02 00 83", reply "a1|a2 00 02 02 00 83
// <charging> <level>"; failing that, the mouse's heartbeat "03 00 <level>
// <charging>". Receiver and cable share one icon, the cable winning.

constexpr unsigned kWlmouseVid = 0x36A7;

struct WlmouseProvider {
    LastSeen asleep;
    std::wstring name;

    static std::pair<int, bool> ParseFeature(std::vector<BYTE> const& r) {
        for (size_t i = 5; i + 2 < r.size(); i++) {
            if (r[i] == 0x83 && r[i - 1] == 0x00 && r[i - 2] == 0x02 && (r[i - 5] == 0xA1 || r[i - 5] == 0xA2) &&
                r[i + 2] <= 100) {
                return {r[i + 2], r[i + 1] != 0};
            }
        }
        return {-1, false};
    }

    static std::pair<int, bool> ParseHeartbeat(std::vector<BYTE> const& d) {
        for (size_t off : {0u, 1u}) {
            if (d.size() >= off + 4 && d[off] == 0x03 && d[off + 1] == 0x00 && d[off + 2] <= 100) {
                return {d[off + 2], d[off + 3] != 0};
            }
        }
        return {-1, false};
    }

    std::pair<int, bool> ReadGroup(std::vector<HidInfo> const& ifaces) {
        std::vector<HidInfo> vendor;
        for (auto const& info : ifaces) {
            if (info.usagePage == 0xFFFF) {
                vendor.push_back(info);
            }
        }
        std::stable_sort(vendor.begin(), vendor.end(),
                         [](HidInfo const& a, HidInfo const& b) { return (a.usage != 0) < (b.usage != 0); });
        for (auto const& info : vendor) {
            HidDevice dev;
            if (!dev.Open(info)) {
                continue;
            }
            std::vector<BYTE> report = {0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x83};
            report.resize(65, 0);
            dev.SendFeature(report);
            for (int i = 0; i < 15; i++) {
                Sleep(50);
                auto r = ParseFeature(dev.GetFeature(0x00));
                if (r.first >= 0) {
                    return r;
                }
            }
        }
        int listened = 0;
        for (auto const& info : ifaces) {
            if (info.usagePage < 0xFF00 || listened >= 2) {
                continue;
            }
            listened++;
            HidDevice dev;
            if (!dev.Open(info)) {
                continue;
            }
            ULONGLONG end = GetTickCount64() + 1000;
            while (GetTickCount64() < end) {
                auto r = ParseHeartbeat(dev.Read(20));
                if (r.first >= 0) {
                    return r;
                }
            }
        }
        return {-1, false};
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        std::vector<std::wstring> foundKeys;
        if (!VendorPresent(paths, kWlmouseVid)) {
            asleep.KeepAsleep(out, foundKeys);
            return out;
        }
        std::map<unsigned, std::vector<HidInfo>> groups;
        for (auto const& info : Enumerate(paths, kWlmouseVid)) {
            groups[info.pid].push_back(info);
        }
        auto isReceiver = [](unsigned pid, std::vector<HidInfo> const& ifaces) {
            std::wstring product = Lower(ifaces[0].product);
            return Contains(product, L"receiver") || pid == 0xA887 || pid == 0xA868 || pid == 0xA880;
        };
        std::vector<std::pair<unsigned, std::vector<HidInfo>>> order(groups.begin(), groups.end());
        std::stable_sort(order.begin(), order.end(), [&](auto const& a, auto const& b) {
            return isReceiver(a.first, a.second) < isReceiver(b.first, b.second);
        });
        std::vector<std::pair<int, bool>> readings;
        for (auto const& [pid, ifaces] : order) {
            if (!readings.empty() && readings[0].second && isReceiver(pid, ifaces)) {
                continue;
            }
            if (pid == 0xA887) {
                name = L"WLmouse Beast X";
            } else if (pid == 0xA868) {
                name = L"WLmouse Beast X Mini Pro";
            } else if (pid == 0xA880) {
                name = L"WLmouse Beast X Max";
            } else if (name.empty() && !ifaces[0].product.empty()) {
                name = ifaces[0].product;
            }
            auto r = ReadGroup(ifaces);
            if (r.first >= 0) {
                readings.push_back(r);
            }
        }
        if (!readings.empty()) {
            std::stable_sort(readings.begin(), readings.end(),
                             [](auto const& a, auto const& b) { return a.second > b.second; });
            Device d;
            d.key = L"wlmouse";
            d.name = name.empty() ? L"WLmouse" : name;
            d.kind = "mouse";
            d.source = "wlmouse";
            d.level = readings[0].first;
            d.charging = readings[0].second;
            asleep.Remember(d);
            out.push_back(d);
            foundKeys.push_back(d.key);
        }
        asleep.KeepAsleep(out, foundKeys);
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Sony DualShock 4 and DualSense, from their full input report. Over
// Bluetooth, reading feature report 02 (DS4) or 05 (DualSense) switches the
// controller to the full report first.

constexpr unsigned kSonyVid = 0x054C;

std::vector<Device> PollPlayStation(std::vector<std::wstring> const& paths) {
    std::vector<Device> out;
    if (!VendorPresent(paths, kSonyVid)) {
        return out;
    }
    struct Model {
        unsigned pid;
        PCWSTR name;
        bool dualsense;
    };
    static constexpr Model kModels[] = {
        {0x05C4, L"Sony DualShock 4", false}, {0x09CC, L"Sony DualShock 4", false},
        {0x05C5, L"Sony DualShock 4", false}, {0x0BA0, L"Sony DualShock 4", false},
        {0x0CE6, L"Sony DualSense", true},    {0x0DF2, L"Sony DualSense Edge", true},
    };
    struct Conn {
        unsigned pid;
        std::wstring name;
        bool bluetooth;
        std::wstring mac;
        int level = -1;
        bool charging = false;
        bool read = false;
    };
    std::map<std::pair<unsigned, std::wstring>, std::vector<HidInfo>> groups;
    for (auto const& info : Enumerate(paths, kSonyVid)) {
        groups[{info.pid, info.serial}].push_back(info);
    }
    std::vector<Conn> conns;
    for (auto const& [id, ifaces] : groups) {
        Model const* model = nullptr;
        for (auto const& m : kModels) {
            if (m.pid == id.first) {
                model = &m;
            }
        }
        if (!model) {
            continue;
        }
        Conn conn{id.first, model->name, ifaces[0].bluetooth, ifaces[0].bluetooth ? id.second : L""};
        for (auto const& info : ifaces) {
            HidDevice dev;
            if (!dev.Open(info)) {
                continue;
            }
            BYTE rid = model->dualsense ? (info.bluetooth ? 0x31 : 0x01) : (info.bluetooth ? 0x11 : 0x01);
            size_t off = model->dualsense ? (info.bluetooth ? 54 : 53) : (info.bluetooth ? 32 : 30);
            dev.GetFeature(model->dualsense ? 0x05 : 0x02);
            ULONGLONG end = GetTickCount64() + 1500;
            while (GetTickCount64() < end && !conn.read) {
                auto data = dev.Read(50);
                // Reports with id 0 lose that byte; these controllers number them.
                if (data.size() <= off || data[0] != rid) {
                    continue;
                }
                BYTE b = data[off];
                conn.level = std::min(b & 0x0F, 10) * 10;
                if (model->dualsense) {
                    int charge = (b >> 4) & 0x0F;
                    conn.charging = charge == 1 || charge == 2;
                } else {
                    conn.charging = (b & 0x10) != 0;
                }
                conn.read = true;
            }
            if (conn.read) {
                break;
            }
        }
        conns.push_back(conn);
    }
    // One controller on Bluetooth and the cable at once: one icon, the cable's reading.
    std::map<unsigned, std::vector<Conn>> byPid;
    for (auto const& c : conns) {
        byPid[c.pid].push_back(c);
    }
    for (auto& [pid, group] : byPid) {
        std::vector<Conn> wireless, wired;
        for (auto const& c : group) {
            (c.bluetooth ? wireless : wired).push_back(c);
        }
        std::vector<Conn> devices;
        if (wireless.size() == 1 && !wired.empty()) {
            Conn merged = wireless[0];
            if (wired[0].read && (wired[0].charging || !merged.read)) {
                merged.level = wired[0].level;
                merged.charging = wired[0].charging;
                merged.read = true;
            }
            devices.push_back(merged);
        } else {
            devices = group;
        }
        for (auto const& c : devices) {
            Device d;
            WCHAR key[64];
            swprintf_s(key, L"ps:%04x:%ls", c.pid, c.mac.c_str());
            d.key = key;
            d.name = c.name;
            d.kind = "gamepad";
            d.source = "playstation";
            d.vid = kSonyVid;
            d.pid = c.pid;
            if (c.read) {
                d.level = c.level;
                d.charging = c.charging;
            } else {
                d.approx = L"connected, battery not reported yet";
            }
            out.push_back(d);
        }
    }
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Xbox-protocol controllers: Windows.Gaming.Input's battery report (a real
// percentage and charging), with XInput's four coarse levels as the
// fallback. Over Bluetooth, Windows' own Bluetooth value is used instead.

using XInputGetState_t = DWORD(WINAPI*)(DWORD, void*);
using XInputGetBatteryInformation_t = DWORD(WINAPI*)(DWORD, BYTE, void*);

struct XboxProvider {
    HMODULE dll = nullptr;
    bool loaded = false;
    XInputGetState_t getState = nullptr;
    XInputGetBatteryInformation_t getBattery = nullptr;
    std::map<int, int> lastLevel;
    bool pending = false;

    bool Load() {
        if (!loaded) {
            loaded = true;
            for (PCWSTR name : {L"xinput1_4.dll", L"xinput1_3.dll"}) {
                dll = LoadLibraryW(name);
                if (dll) {
                    getState = (XInputGetState_t)GetProcAddress(dll, "XInputGetState");
                    getBattery = (XInputGetBatteryInformation_t)GetProcAddress(dll, "XInputGetBatteryInformation");
                    if (getState && getBattery) {
                        break;
                    }
                    FreeLibrary(dll);
                    dll = nullptr;
                }
            }
        }
        return getState && getBattery;
    }

    std::vector<int> ConnectedSlots() {
        std::vector<int> slots;
        if (!Load()) {
            return slots;
        }
        BYTE state[16];
        for (int i = 0; i < 4; i++) {
            if (getState(i, state) == ERROR_SUCCESS) {
                slots.push_back(i);
            }
        }
        return slots;
    }

    struct Wgi {
        std::wstring name;
        unsigned vid = 0;
        unsigned pid = 0;
        int level = -1;
        bool charging = false;
    };

    std::vector<Wgi> QueryWgi() {
        std::vector<Wgi> list;
        try {
            using namespace winrt::Windows::Gaming::Input;
            for (auto const& c : RawGameController::RawGameControllers()) {
                Wgi w;
                w.name = std::wstring(c.DisplayName());
                w.vid = c.HardwareVendorId();
                w.pid = c.HardwareProductId();
                if (auto report = c.TryGetBatteryReport()) {
                    auto remain = report.RemainingCapacityInMilliwattHours();
                    auto full = report.FullChargeCapacityInMilliwattHours();
                    if (remain && full && full.Value() > 0) {
                        w.level = std::clamp((int)std::lround(remain.Value() * 100.0 / full.Value()), 0, 100);
                    }
                    w.charging = report.Status() == winrt::Windows::System::Power::BatteryStatus::Charging;
                }
                std::wstring lower = Lower(w.name);
                if (Contains(lower, L"hid") || lower.empty() || lower == L"xbox controller" ||
                    lower == L"xbox one controller" || lower == L"controller (xbox one for windows)" ||
                    lower == L"xbox wireless controller") {
                    w.name = w.vid == 0x3537 ? L"GameSir controller" : w.vid == 0x045E ? L"Xbox controller" : L"";
                }
                list.push_back(w);
            }
        } catch (...) {
        }
        return list;
    }

    std::vector<Device> Poll(std::vector<std::wstring> const& paths) {
        std::vector<Device> out;
        pending = false;
        if (!Load()) {
            return out;
        }
        // Which (vid, pid) are connected over Bluetooth, from the HID paths.
        std::set<std::pair<unsigned, unsigned>> bluetoothIds;
        for (auto const& path : paths) {
            HidInfo quick{path};
            ParsePath(quick);
            if (quick.bluetooth) {
                bluetoothIds.insert({quick.vid, quick.pid});
            }
        }
        struct Slot {
            int slot;
            int level;
            bool charging;
            std::wstring approx;
            bool known;
        };
        std::vector<Slot> slots;
        for (int slot = 0; slot < 4; slot++) {
            BYTE state[16];
            if (getState(slot, state) != ERROR_SUCCESS) {
                continue;
            }
            BYTE info[2] = {};
            Slot s{slot, -1, false, L"", false};
            if (getBattery(slot, 0, info) == ERROR_SUCCESS) {
                BYTE type = info[0];
                BYTE level = info[1];
                static constexpr int kLevels[] = {5, 20, 55, 100};
                static constexpr PCWSTR kTexts[] = {L"empty", L"low", L"medium", L"full"};
                if (type == 0x01) {  // wired: charging
                    s.level = lastLevel.count(slot) ? lastLevel[slot] : (level < 4 ? kLevels[level] : 100);
                    s.charging = true;
                    s.approx = L"on cable, charging";
                    s.known = true;
                } else if (type != 0x00 && type != 0xFF && level < 4) {
                    s.level = kLevels[level];
                    s.approx = std::wstring(L"about ") + std::to_wstring(s.level) + L"% (" + kTexts[level] + L")";
                    s.known = true;
                }
            }
            slots.push_back(s);
        }
        if (slots.empty()) {
            return out;
        }
        std::vector<Wgi> reports;
        for (auto const& w : QueryWgi()) {
            if (w.level >= 0) {
                reports.push_back(w);
            }
        }
        for (size_t n = 0; n < slots.size(); n++) {
            auto& s = slots[n];
            Device d;
            d.key = L"xinput:" + std::to_wstring(s.slot);
            d.kind = "gamepad";
            d.source = "xinput";
            Wgi const* rep = n < reports.size() ? &reports[n] : nullptr;
            d.name = rep && !rep->name.empty() ? rep->name : L"Controller";
            if (rep && bluetoothIds.count({rep->vid, rep->pid})) {
                d.via = "bluetooth";
                d.approx = L"connected over Bluetooth";
            } else if (rep) {
                d.level = rep->level;
                d.charging = rep->charging;
                if (!rep->charging) {
                    lastLevel[s.slot] = rep->level;
                }
            } else if (s.known) {
                d.level = s.level;
                d.charging = s.charging;
                d.approx = s.approx;
                if (!s.charging) {
                    lastLevel[s.slot] = s.level;
                }
            } else {
                d.approx = L"connected, battery not reported yet";
                pending = true;
            }
            if (slots.size() > 1) {
                d.name += L" " + std::to_wstring(n + 1);
            }
            out.push_back(d);
        }
        return out;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Bluetooth devices whose battery Windows itself knows (Settings > Bluetooth
// & devices), connected ones only.

std::string KindFromName(std::wstring const& name) {
    std::wstring n = Lower(name);
    for (PCWSTR w : {L"controller", L"gamepad", L"joystick", L"joy-con", L"dualsense", L"dualshock"}) {
        if (Contains(n, w)) {
            return "gamepad";
        }
    }
    for (PCWSTR w : {L"headset", L"headphone", L"buds", L"earbud", L"airpods", L"ear", L"sonoflow", L"maxwell",
                     L"arctis", L"cloud", L"blackshark"}) {
        if (Contains(n, w)) {
            return "headset";
        }
    }
    if (Contains(n, L"mouse") || Contains(n, L"mx ")) {
        return "mouse";
    }
    if (Contains(n, L"keyboard") || Contains(n, L"keys")) {
        return "keyboard";
    }
    return "";
}

std::vector<Device> PollBluetooth() {
    static const DEVPROPKEY kConnected = {
        {0x83DA6326, 0x97A6, 0x4088, {0x94, 0x53, 0xA1, 0x92, 0x3F, 0x57, 0x3B, 0x29}}, 15};
    static const DEVPROPKEY kBattery = {
        {0x104EA319, 0x6EE2, 0x4701, {0xBD, 0x47, 0x8D, 0xDB, 0xF4, 0x25, 0xBB, 0xE5}}, 2};
    static const DEVPROPKEY kName = {
        {0xB725F130, 0x47EF, 0x101A, {0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC}}, 10};

    std::map<std::wstring, Device> byMac;
    for (PCWSTR enumerator : {L"BTHENUM", L"BTHLE"}) {
        ULONG flags = CM_GETIDLIST_FILTER_ENUMERATOR | CM_GETIDLIST_FILTER_PRESENT;
        ULONG size = 0;
        if (CM_Get_Device_ID_List_SizeW(&size, enumerator, flags) != CR_SUCCESS || size <= 1) {
            continue;
        }
        std::vector<WCHAR> ids(size);
        if (CM_Get_Device_ID_ListW(enumerator, ids.data(), size, flags) != CR_SUCCESS) {
            continue;
        }
        for (PCWSTR id = ids.data(); *id; id += wcslen(id) + 1) {
            PCWSTR slash = wcschr(id, L'\\');
            if (!slash || _wcsnicmp(slash + 1, L"DEV_", 4) != 0) {
                continue;
            }
            std::wstring mac(slash + 5, std::min<size_t>(12, wcslen(slash + 5)));
            DEVINST node;
            if (CM_Locate_DevNodeW(&node, (DEVINSTID_W)id, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) {
                continue;
            }
            DEVPROPTYPE type = 0;
            DEVPROP_BOOLEAN connected = DEVPROP_FALSE;
            ULONG length = sizeof(connected);
            if (CM_Get_DevNode_PropertyW(node, &kConnected, &type, (PBYTE)&connected, &length, 0) != CR_SUCCESS ||
                connected != DEVPROP_TRUE) {
                continue;
            }
            BYTE battery = 0;
            length = sizeof(battery);
            if (CM_Get_DevNode_PropertyW(node, &kBattery, &type, &battery, &length, 0) != CR_SUCCESS ||
                type != DEVPROP_TYPE_BYTE || battery > 100) {
                continue;
            }
            WCHAR name[128] = L"";
            length = sizeof(name);
            CM_Get_DevNode_PropertyW(node, &kName, &type, (PBYTE)name, &length, 0);
            Device d;
            d.key = L"bt:" + mac;
            d.name = *name ? name : L"Bluetooth device";
            d.kind = KindFromName(d.name);
            if (d.kind.empty()) {
                d.kind = "bluetooth";
            }
            d.source = "bluetooth";
            d.level = battery;
            byMac[mac] = d;
        }
    }
    std::vector<Device> out;
    for (auto& [mac, d] : byMac) {
        out.push_back(d);
    }
    return out;
}

// A name reduced to what identifies the device, not how it's connected, so
// "Audeze Maxwell" and "Audeze Maxwell BT" come out equal.
std::wstring Family(std::wstring const& name) {
    std::wstring clean;
    for (wchar_t c : Lower(name)) {
        clean += iswalnum(c) ? c : L' ';
    }
    std::wstring out, word;
    auto flush = [&] {
        static const std::set<std::wstring> kTransport = {L"bt", L"ble", L"bluetooth", L"wireless", L"dongle",
                                                          L"receiver", L"headset", L"usb"};
        if (!word.empty() && !kTransport.count(word)) {
            out += (out.empty() ? L"" : L" ") + word;
        }
        word.clear();
    };
    for (wchar_t c : clean) {
        if (c == L' ') {
            flush();
        } else {
            word += c;
        }
    }
    flush();
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Polling all of them

struct Poller {
    std::array<bool, kVendorCount> vendors{};
    LastSeen alienwareAsleep;
    RazerProvider razer;
    LogitechProvider logitech;
    SteelSeriesProvider steelseries;
    AudezeProvider audeze;
    MchoseProvider mchose;
    WlmouseProvider wlmouse;
    XboxProvider xbox;

    // Missing polls in a row, per device; icons go after two (one for
    // controllers and Bluetooth, which report their state reliably).
    std::map<std::wstring, Device> shown;
    std::map<std::wstring, int> missing;

    bool Pending() const { return xbox.pending || !missing.empty(); }

    std::vector<Device> PollOnce() {
        auto paths = InterfacePaths();
        ForgetMissing(paths);
        std::vector<Device> results;
        auto add = [&](std::vector<Device> more) { results.insert(results.end(), more.begin(), more.end()); };
        if (vendors[kAlienware]) add(PollAlienware(paths, alienwareAsleep));
        if (vendors[kRazer]) add(razer.Poll(paths));
        if (vendors[kLogitech]) add(logitech.Poll(paths));
        if (vendors[kSteelSeries]) add(steelseries.Poll(paths));
        if (vendors[kHyperX]) add(PollHyperX(paths));
        if (vendors[kAudeze]) add(audeze.Poll(paths));
        if (vendors[kMchose]) add(mchose.Poll(paths));
        if (vendors[kWlmouse]) add(wlmouse.Poll(paths));
        if (vendors[kPlayStation]) add(PollPlayStation(paths));
        if (vendors[kXbox]) add(xbox.Poll(paths));
        if (vendors[kBluetooth]) {
            auto bt = PollBluetooth();
            // A controller over Bluetooth: Windows' value wins.
            bool btPads = std::any_of(bt.begin(), bt.end(), [](Device const& d) { return d.kind == "gamepad"; });
            if (btPads) {
                results.erase(std::remove_if(results.begin(), results.end(),
                                             [](Device const& d) { return d.source == "xinput" && d.via == "bluetooth"; }),
                              results.end());
            }
            // A device also read over HID: the HID reading wins, it has charging.
            std::vector<std::wstring> hidFamilies;
            for (auto const& d : results) {
                if (d.source != "xinput") {
                    hidFamilies.push_back(Family(d.name));
                }
            }
            for (auto const& d : bt) {
                std::wstring fam = Family(d.name);
                bool duplicate = !fam.empty() && std::any_of(hidFamilies.begin(), hidFamilies.end(),
                                                             [&](std::wstring const& h) {
                                                                 return fam == h || (fam.size() >= 6 &&
                                                                                     (Contains(h, fam) || Contains(fam, h)));
                                                             });
                if (!duplicate) {
                    results.push_back(d);
                }
            }
        }
        return Smooth(results);
    }

    std::vector<Device> Smooth(std::vector<Device> const& fresh) {
        std::vector<Device> out = fresh;
        std::set<std::wstring> seen;
        for (auto const& d : fresh) {
            seen.insert(d.key);
            shown[d.key] = d;
            missing.erase(d.key);
        }
        for (auto it = shown.begin(); it != shown.end();) {
            if (seen.count(it->first)) {
                ++it;
                continue;
            }
            int limit = it->first.starts_with(L"xinput:") || it->first.starts_with(L"bt:") ||
                                it->first.starts_with(L"ps:")
                            ? 1
                            : 2;
            if (++missing[it->first] < limit) {
                out.push_back(it->second);
                ++it;
            } else {
                missing.erase(it->first);
                it = shown.erase(it);
            }
        }
        return out;
    }

    // What's connected, cheaply: the HID interfaces (receivers, cables) and
    // the XInput slots (a controller switched on behind a receiver).
    std::wstring Signature() {
        std::wstring sig;
        auto paths = InterfacePaths();
        std::sort(paths.begin(), paths.end());
        for (auto const& p : paths) {
            sig += p;
            sig += L'|';
        }
        if (vendors[kXbox]) {
            for (int slot : xbox.ConnectedSlots()) {
                sig += L"x" + std::to_wstring(slot);
            }
        }
        return sig;
    }
};

}  // namespace bat
