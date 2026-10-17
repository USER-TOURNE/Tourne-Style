////////////////////////////////////////////////////////////////////////////////
// Your phone's notifications, over Apple's Notification Center Service ("// @inline phone-ancs.cpp" in
// taskbar-host.cpp after phone-map.cpp, Tourne Desktop only): what Phone Link shows from an iPhone, read by Tourne
// itself over Bluetooth LE, so Phone Link can go.
//
// Each of the phone's notifications becomes one of Windows' own, as its phone app ("Tourne.Phone.<its bundle id>",
// named as the phone names the app), so the notification centre, Tourne's pop-ups and the phone pane show them as
// they show any app's. Gone from the phone, gone here; the ones already on the phone when it connects come in
// quietly (no pop-up). Texts are left to phone-map.cpp while it holds them. While Phone Link runs it does all this,
// and Tourne waits for it to end.
//
// One thread (a multithreaded apartment) keeps the connection (Windows reconnects when the phone comes back:
// GattSession.MaintainConnection) and asks the phone for each notification's text; the phone's answers come on
// Windows' threads and are handed over. Idle cost: waits on events.

#if defined(TD_PREFIX) || defined(PHONE_ANCS_PROBE)
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Storage.Streams.h>

namespace phoneancs {

constexpr GUID kService = {0x7905F431, 0xB5CE, 0x4E99, {0xA4, 0x0F, 0x4B, 0x1E, 0x12, 0x2D, 0x00, 0xD0}};
constexpr GUID kNotificationSource = {0x9FBF120D, 0x6301, 0x42D9, {0x8C, 0x58, 0x25, 0xE6, 0x99, 0xA2, 0x1D, 0xBD}};
constexpr GUID kControlPoint = {0x69D1D8F3, 0x45E1, 0x49A8, {0x98, 0x21, 0x9B, 0xBD, 0xFD, 0xAA, 0xD9, 0xD9}};
constexpr GUID kDataSource = {0x22EAC6E9, 0x24D6, 0x4BB5, {0xBE, 0x44, 0xB3, 0x6A, 0xCE, 0x7C, 0x7B, 0xFB}};

// The Notification Source's events, their flags, and what can be asked of a notification.
enum : uint8_t { kAdded = 0, kModified = 1, kRemoved = 2 };
enum : uint8_t { kSilent = 1, kImportant = 2, kPreExisting = 4, kPositiveAction = 8, kNegativeAction = 16 };
enum : uint8_t { kApp = 0, kTitle = 1, kSubtitle = 2, kMessage = 3, kDate = 5, kPositiveLabel = 6, kNegativeLabel = 7 };
enum : uint8_t { kIncomingCall = 1, kMissedCall = 2 };

struct Event {
    uint8_t id = 0, flags = 0, category = 0;
    uint32_t uid = 0;
};

// An event as the phone sends it: id, flags, category, count, then the notification's id (little-endian).
bool ParseEvent(uint8_t const* d, size_t n, Event& e) {
    if (n < 8) return false;
    e.id = d[0];
    e.flags = d[1];
    e.category = d[2];
    e.uid = d[4] | (d[5] << 8) | (d[6] << 16) | ((uint32_t)d[7] << 24);
    return true;
}

// What's asked of each notification, in this order (Title, Subtitle and Message with a most length).
constexpr uint8_t kAsked[] = {kApp, kTitle, kSubtitle, kMessage, kDate, kPositiveLabel, kNegativeLabel};

std::vector<uint8_t> AskNotification(uint32_t uid) {
    return {0, (uint8_t)uid, (uint8_t)(uid >> 8), (uint8_t)(uid >> 16), (uint8_t)(uid >> 24),
            kApp, kTitle, 128, 0, kSubtitle, 128, 0, kMessage, 0x00, 0x04 /* 1024 */, kDate, kPositiveLabel, kNegativeLabel};
}

std::vector<uint8_t> AskApp(std::string const& app) {
    std::vector<uint8_t> c{1};
    c.insert(c.end(), app.begin(), app.end());
    c.push_back(0);
    c.push_back(0);  // DisplayName
    return c;
}

// An action on a notification: 0 its positive one (answer a call), 1 its negative one (decline, clear).
std::vector<uint8_t> Act(uint32_t uid, uint8_t action) {
    return {2, (uint8_t)uid, (uint8_t)(uid >> 8), (uint8_t)(uid >> 16), (uint8_t)(uid >> 24), action};
}

// A notification's answer, which can come in several parts: 1 whole (values filled), 0 more to come, -1 not
// the answer for uid.
int ParseNotification(std::vector<uint8_t> const& b, uint32_t uid, std::map<uint8_t, std::string>& values) {
    if (b.size() < 5) return b.empty() || b[0] == 0 ? 0 : -1;
    if (b[0] != 0 || (b[1] | (b[2] << 8) | (b[3] << 16) | ((uint32_t)b[4] << 24)) != uid) return -1;
    values.clear();
    size_t at = 5;
    for (uint8_t asked : kAsked) {
        if (at + 3 > b.size()) return 0;
        if (b[at] != asked) return -1;
        size_t length = b[at + 1] | (b[at + 2] << 8);
        if (at + 3 + length > b.size()) return 0;
        values[asked] = std::string((char const*)b.data() + at + 3, length);
        at += 3 + length;
    }
    return 1;
}

// An app's answer (its name): as above.
int ParseApp(std::vector<uint8_t> const& b, std::string const& app, std::string& name) {
    if (b.empty()) return 0;
    if (b[0] != 1) return -1;
    auto nul = std::find(b.begin() + 1, b.end(), 0);
    if (nul == b.end()) return 0;
    if (std::string(b.begin() + 1, nul) != app) return -1;
    size_t at = nul - b.begin() + 1;
    if (at + 3 > b.size()) return 0;
    if (b[at] != 0) return -1;
    size_t length = b[at + 1] | (b[at + 2] << 8);
    if (at + 3 + length > b.size()) return 0;
    name.assign((char const*)b.data() + at + 3, length);
    return 1;
}

// The phone's date ("yyyyMMddTHHmmSS", its local time) in UTC for a notification's timestamp; empty if unreadable.
std::wstring Timestamp(std::string const& date) {
    SYSTEMTIME local{}, utc{};
    if (date.size() < 15 || sscanf(date.c_str(), "%4hu%2hu%2huT%2hu%2hu%2hu", &local.wYear, &local.wMonth, &local.wDay, &local.wHour,
                                   &local.wMinute, &local.wSecond) != 6)
        return {};
    if (!TzSpecificLocalTimeToSystemTime(nullptr, &local, &utc)) return {};
    WCHAR out[32];
    swprintf(out, 32, L"%04u-%02u-%02uT%02u:%02u:%02uZ", utc.wYear, utc.wMonth, utc.wDay, utc.wHour, utc.wMinute, utc.wSecond);
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Notifications made from them.

constexpr wchar_t kPrefix[] = L"Tourne.Phone.";
constexpr wchar_t kGroup[] = L"ancs";

bool g_dryRun = false;  // (the probe: say what would be shown, show nothing)

std::wstring AumidFor(std::string const& app) { return kPrefix + phonemap::FromUtf8(app); }

std::wstring Escape(std::wstring const& s) { return phonemap::XmlEscape(s); }

// The app's name for its notifications (as the phone names it), and its place in Windows' notification settings
// made first, so a Tourne that draws the pop-ups turns Windows' off for it before one shows twice.
void NameApp(std::wstring const& aumid, std::wstring const& name) {
    static std::set<std::wstring> named;
    if (g_dryRun || !named.insert(aumid + L"|" + name).second) return;
    std::wstring key = L"Software\\Classes\\AppUserModelId\\" + aumid;
    RegSetKeyValueW(HKEY_CURRENT_USER, key.c_str(), L"DisplayName", REG_SZ, name.c_str(), (DWORD)((name.size() + 1) * sizeof(WCHAR)));
    HKEY settings;
    std::wstring at = L"Software\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings\\" + aumid;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, at.c_str(), 0, nullptr, 0, KEY_READ, nullptr, &settings, nullptr) == ERROR_SUCCESS)
        RegCloseKey(settings);
}

struct Note {
    uint32_t uid = 0;
    uint8_t flags = 0, category = 0;
    std::string app;
    std::wstring appName, title, subtitle, message, when, positive, negative;
    std::wstring key;  // PhoneNoteKey (phone-link.cpp): its toast's tag, the same each time the phone sends it
};

// Each notification shown, by the phone's number for it: its toast's tag (on the connection's thread only).
std::map<uint32_t, std::wstring> g_tags;

void Show(Note const& n, bool quiet) {
    std::wstring aumid = AumidFor(n.app);
    std::wstring first = n.title.empty() ? n.appName : n.title;
    std::wstring second = n.subtitle.empty() ? n.message : n.message.empty() ? n.subtitle : n.subtitle + L": " + n.message;
    if (g_dryRun) {
        wprintf(L"  would show: %ls (%ls), category %u, flags 0x%02X%ls; title %zu chars, text %zu chars\n", n.appName.c_str(),
                aumid.c_str(), n.category, n.flags, quiet ? L", quietly" : L"", first.size(), second.size());
        return;
    }
    NameApp(aumid, n.appName);
    try {
        std::wstring xml = L"<toast";
        if (!n.when.empty()) xml += L" displayTimestamp=\"" + n.when + L"\"";
        // (Tourne's pop-ups read these: SuppressPopup isn't kept, and a call's stays up while it rings)
        if (quiet) xml += L" launch=\"tourne-phone-quiet\"";
        else if (n.category == kIncomingCall) xml += L" launch=\"tourne-phone-call\"";
        xml += L"><visual><binding template=\"ToastGeneric\"><text>" + Escape(first) + L"</text>";
        if (!second.empty()) xml += L"<text>" + Escape(second) + L"</text>";
        xml += L"</binding></visual>";
        // The phone's own actions as buttons (Answer and Decline for a call, Clear...): done on the phone
        // (System Icons hands a click to the taskbar part's Start window, "TournePhoneAction").
        std::wstring buttons;
        if ((n.flags & kPositiveAction) && !n.positive.empty())
            buttons += L"<action content=\"" + Escape(n.positive) + L"\" arguments=\"tourne-phone:0:" + std::to_wstring(n.uid) + L"\"/>";
        if ((n.flags & kNegativeAction) && !n.negative.empty())
            buttons += L"<action content=\"" + Escape(n.negative) + L"\" arguments=\"tourne-phone:1:" + std::to_wstring(n.uid) + L"\"/>";
        if (!buttons.empty()) xml += L"<actions>" + buttons + L"</actions>";
        if (quiet || (n.flags & kSilent)) xml += L"<audio silent=\"true\"/>";
        xml += L"</toast>";
        winrt::Windows::Data::Xml::Dom::XmlDocument doc;
        doc.LoadXml(xml);
        winrt::Windows::UI::Notifications::ToastNotification toast(doc);
        toast.Tag(n.key);
        toast.Group(kGroup);
        if (quiet) toast.SuppressPopup(true);
        winrt::Windows::UI::Notifications::ToastNotificationManager::CreateToastNotifier(aumid).Show(toast);
    } catch (...) {
        phonemap::Log(L"couldn't show a phone notification");
    }
}

void Hide(uint32_t uid, std::wstring const& aumid) {
    if (g_dryRun) {
        wprintf(L"  would take away %u\n", uid);
        return;
    }
    auto tag = g_tags.find(uid);
    if (tag == g_tags.end()) return;
    try {
        winrt::Windows::UI::Notifications::ToastNotificationManager::History().Remove(tag->second, kGroup, aumid);
    } catch (...) {
    }
    g_tags.erase(tag);
}

// Every phone notification shown before (from the apps named so far): the phone sends the ones it still has
// again when it connects, so the centre matches it.
void HideAll() {
    if (g_dryRun) return;
    g_tags.clear();
    HKEY root;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\AppUserModelId", 0, KEY_READ, &root) != ERROR_SUCCESS) return;
    WCHAR name[256];
    std::vector<std::wstring> ours;
    for (DWORD i = 0;; i++) {
        DWORD length = ARRAYSIZE(name);
        if (RegEnumKeyExW(root, i, name, &length, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) break;
        if (!_wcsnicmp(name, kPrefix, wcslen(kPrefix))) ours.push_back(name);
    }
    RegCloseKey(root);
    for (auto const& aumid : ours) {
        try {
            winrt::Windows::UI::Notifications::ToastNotificationManager::History().Clear(aumid);
        } catch (...) {
        }
    }
}

// Cleared from the phone pane (its right-click), as the notification centre clears one: from Windows' list only;
// the phone keeps its own, and sends the ones it still has again when it connects next.
constexpr bool kCanClear = true;

void Clear(std::wstring const& tag, std::wstring const& group, std::wstring const& aumid) {
    PhoneNoteClear(aumid, tag);  // (so it doesn't come back when the phone connects again)
    try {
        winrt::Windows::UI::Notifications::ToastNotificationManager::History().Remove(tag, group, aumid);
    } catch (...) {
    }
}

void ClearApp(std::wstring const& aumid) {
    try {
        for (auto const& toast : winrt::Windows::UI::Notifications::ToastNotificationManager::History().GetHistory(aumid))
            PhoneNoteClear(aumid, std::wstring(toast.Tag()));
        winrt::Windows::UI::Notifications::ToastNotificationManager::History().Clear(aumid);
    } catch (...) {
    }
}

////////////////////////////////////////////////////////////////////////////////
// The connection.

enum class Link { Off, NoPhone, PhoneLink, Away, Ready, Refused };

struct State {
    std::mutex lock;
    Link link = Link::Off;
    std::deque<Event> events;      // from the Notification Source, to be handled in order
    std::deque<std::pair<uint32_t, uint8_t>> actions;  // clicked: a notification's action, to do on the phone
    std::set<uint32_t> calls;      // the notifications that are a ringing call
    std::vector<uint8_t> answer;   // the Data Source's, gathered
};
State g;
HANDLE g_stopEvent, g_eventsEvent, g_answerEvent, g_linkEvent;
HANDLE g_thread;
std::atomic<int> g_shown{0};  // (for the probe)
HWND g_tellWnd = nullptr;  // told (g_tellMsg) when notifications came or went, so the phone pane reads them again
UINT g_tellMsg = 0;

void SetLink(Link link) {
    {
        std::lock_guard<std::mutex> lock(g.lock);
        if (g.link == link) return;
        g.link = link;
    }
    static PCWSTR const names[] = {L"off", L"no paired phone", L"Phone Link has them", L"not connected", L"connected", L"the phone said no"};
    phonemap::Log(std::wstring(L"notifications: ") + names[(int)link]);
}

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
using winrt::Windows::Storage::Streams::Buffer;
using winrt::Windows::Storage::Streams::IBuffer;

std::vector<uint8_t> Bytes(IBuffer const& b) { return std::vector<uint8_t>(b.data(), b.data() + b.Length()); }

// One question to the Control Point, and its whole answer from the Data Source (as parse decides), or false.
template <typename Parse>
bool Ask(GattCharacteristic const& point, std::vector<uint8_t> const& command, Parse parse) {
    {
        std::lock_guard<std::mutex> lock(g.lock);
        g.answer.clear();
    }
    ResetEvent(g_answerEvent);
    Buffer buffer((uint32_t)command.size());
    memcpy(buffer.data(), command.data(), command.size());
    buffer.Length((uint32_t)command.size());
    try {
        auto result = point.WriteValueWithResultAsync(buffer, GattWriteOption::WriteWithResponse).get();
        if (result.Status() != GattCommunicationStatus::Success) return false;  // (gone already: ANCS's 0xA2, 0xA3)
    } catch (...) {
        return false;
    }
    for (DWORD until = GetTickCount() + 5000;;) {
        int left = (int)(until - GetTickCount());
        HANDLE w[2] = {g_stopEvent, g_answerEvent};
        if (left <= 0 || WaitForMultipleObjects(2, w, FALSE, (DWORD)left) != WAIT_OBJECT_0 + 1) return false;
        std::lock_guard<std::mutex> lock(g.lock);
        int r = parse(g.answer);
        if (r != 0) return r > 0;
    }
    return false;
}

// One notification come or changed: its text and app asked for and shown.
void Handle(GattCharacteristic const& point, Event const& e, std::map<std::string, std::wstring>& apps,
            std::map<uint32_t, std::wstring>& shown) {
    std::map<uint8_t, std::string> v;
    if (!Ask(point, AskNotification(e.uid), [&](std::vector<uint8_t> const& b) { return ParseNotification(b, e.uid, v); })) return;
    Note n;
    n.uid = e.uid;
    n.flags = e.flags;
    n.category = e.category;
    n.app = v[kApp];
    if (n.app.empty()) return;
    // Texts: phone-map.cpp's while it holds them (no second notification for one).
    if (!_stricmp(n.app.c_str(), "com.apple.MobileSMS") && phonemap::LinkNow() == phonemap::Link::Ready) return;
    auto known = apps.find(n.app);
    if (known == apps.end()) {
        std::string name;
        Ask(point, AskApp(n.app), [&](std::vector<uint8_t> const& b) { return ParseApp(b, n.app, name); });
        known = apps.emplace(n.app, phonemap::FromUtf8(name)).first;
    }
    n.appName = known->second;
    if (n.appName.empty()) {
        std::wstring id = phonemap::FromUtf8(n.app);
        n.appName = PhoneAppName(PhoneLower(id), PhoneApps());
    }
    n.title = phonemap::FromUtf8(v[kTitle]);
    n.subtitle = phonemap::FromUtf8(v[kSubtitle]);
    n.message = phonemap::FromUtf8(v[kMessage]);
    n.when = Timestamp(v[kDate]);
    n.positive = phonemap::FromUtf8(v[kPositiveLabel]);
    n.negative = phonemap::FromUtf8(v[kNegativeLabel]);
    n.key = PhoneNoteKey(phonemap::FromUtf8(n.app), n.title, n.subtitle, n.message, phonemap::FromUtf8(v[kDate]));
    // Changed: the one it was goes (its words, so its key, may be new).
    if (auto old = g_tags.find(e.uid); old != g_tags.end() && old->second != n.key) Hide(e.uid, AumidFor(n.app));
    // Cleared here before: left cleared, however often the phone sends it again.
    if (PhoneNoteCleared(n.key)) return;
    g_tags[e.uid] = n.key;
    Show(n, (e.flags & kPreExisting) != 0 || e.id == kModified);  // (one already there, or changed: no pop-up)
    if (n.category == kIncomingCall) {
        std::lock_guard<std::mutex> lock(g.lock);
        g.calls.insert(e.uid);
    }
    shown[e.uid] = AumidFor(n.app);
    g_shown++;
}

// The phone's LE device, by name, among the paired ones; empty if it isn't.
std::wstring FindPhone(std::wstring const& name) {
    using namespace winrt::Windows::Devices::Enumeration;
    try {
        for (auto const& d : DeviceInformation::FindAllAsync(BluetoothLEDevice::GetDeviceSelectorFromPairingState(true)).get())
            if (d.Name() == name) return std::wstring(d.Id());
    } catch (...) {
    }
    return {};
}

// Waits for stop (true), or also, or ms.
bool Pause(DWORD ms, HANDLE also = nullptr) {
    HANDLE w[2] = {g_stopEvent, also};
    return WaitForMultipleObjects(also ? 2 : 1, w, FALSE, ms) == WAIT_OBJECT_0;
}

DWORD WINAPI Run(LPVOID) {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
    while (WaitForSingleObject(g_stopEvent, 0) != WAIT_OBJECT_0) {
        if (HANDLE phoneLink = phonemap::PhoneLinkRunning()) {
            SetLink(Link::PhoneLink);
            HANDLE w[2] = {g_stopEvent, phoneLink};
            DWORD r = WaitForMultipleObjects(2, w, FALSE, 10 * 60000);
            CloseHandle(phoneLink);
            if (r == WAIT_OBJECT_0 || Pause(3000)) break;
            continue;
        }
        PhoneInfo who;
        ReadPhoneIdentity(who);
        std::wstring id = who.linked ? FindPhone(who.name) : L"";
        if (id.empty()) {
            SetLink(Link::NoPhone);
            if (Pause(5 * 60000)) break;
            continue;
        }
        try {
            BluetoothLEDevice device = BluetoothLEDevice::FromIdAsync(id).get();
            if (!device) throw winrt::hresult_error();
            // Windows keeps the phone connected (and connects again when it's back) while this session is held.
            GattSession session = GattSession::FromDeviceIdAsync(device.BluetoothDeviceId()).get();
            session.MaintainConnection(true);
            auto statusToken = device.ConnectionStatusChanged([](auto&&, auto&&) { SetEvent(g_linkEvent); });
            bool stopping = false;
            while (!(stopping = WaitForSingleObject(g_stopEvent, 0) == WAIT_OBJECT_0)) {
                if (HANDLE phoneLink = phonemap::PhoneLinkRunning()) {
                    CloseHandle(phoneLink);
                    break;  // (Phone Link came: it has them)
                }
                if (device.ConnectionStatus() != BluetoothConnectionStatus::Connected) {
                    SetLink(Link::Away);
                    if ((stopping = Pause(5 * 60000, g_linkEvent))) break;
                    continue;
                }
                auto services = device.GetGattServicesForUuidAsync(winrt::guid(kService), BluetoothCacheMode::Uncached).get();
                if (services.Status() != GattCommunicationStatus::Success || services.Services().Size() == 0) {
                    // Connected but no ANCS: the phone shares notifications only with what it's allowed to (its
                    // Bluetooth settings for this PC: Share System Notifications).
                    SetLink(services.Status() == GattCommunicationStatus::AccessDenied ? Link::Refused : Link::Away);
                    phonemap::Log(L"notifications: the phone offers none (" + std::to_wstring((int)services.Status()) + L")");
                    if ((stopping = Pause(5 * 60000, g_linkEvent))) break;
                    continue;
                }
                GattDeviceService service = services.Services().GetAt(0);
                auto characteristic = [&](GUID const& uuid) {
                    auto r = service.GetCharacteristicsForUuidAsync(winrt::guid(uuid), BluetoothCacheMode::Uncached).get();
                    return r.Status() == GattCommunicationStatus::Success && r.Characteristics().Size() ? r.Characteristics().GetAt(0)
                                                                                                     : GattCharacteristic(nullptr);
                };
                GattCharacteristic source = characteristic(kNotificationSource), point = characteristic(kControlPoint),
                                   data = characteristic(kDataSource);
                if (!source || !point || !data) {
                    phonemap::Log(L"notifications: the phone's service is incomplete");
                    if ((stopping = Pause(5 * 60000, g_linkEvent))) break;
                    continue;
                }
                {
                    std::lock_guard<std::mutex> lock(g.lock);
                    g.events.clear();
                }
                auto dataToken = data.ValueChanged([](auto&&, GattValueChangedEventArgs const& a) {
                    auto bytes = Bytes(a.CharacteristicValue());
                    {
                        std::lock_guard<std::mutex> lock(g.lock);
                        g.answer.insert(g.answer.end(), bytes.begin(), bytes.end());
                    }
                    SetEvent(g_answerEvent);
                });
                auto sourceToken = source.ValueChanged([](auto&&, GattValueChangedEventArgs const& a) {
                    auto bytes = Bytes(a.CharacteristicValue());
                    Event e;
                    if (!ParseEvent(bytes.data(), bytes.size(), e)) return;
                    {
                        std::lock_guard<std::mutex> lock(g.lock);
                        g.events.push_back(e);
                    }
                    SetEvent(g_eventsEvent);
                });
                // The Data Source first, so no answer is missed; the Notification Source then sends what's on the phone.
                HideAll();
                // (off first: the phone sends what it has only when its notices are turned on, and another program
                // that had them on, Phone Link, may have left them on)
                auto notify = GattClientCharacteristicConfigurationDescriptorValue::Notify;
                source.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None).get();
                auto d = data.WriteClientCharacteristicConfigurationDescriptorAsync(notify).get();
                auto s = d == GattCommunicationStatus::Success ? source.WriteClientCharacteristicConfigurationDescriptorAsync(notify).get() : d;
                if (s != GattCommunicationStatus::Success) {
                    SetLink(s == GattCommunicationStatus::AccessDenied ? Link::Refused : Link::Away);
                    phonemap::Log(L"notifications: the phone said no (" + std::to_wstring((int)s) + L")");
                    data.ValueChanged(dataToken);
                    source.ValueChanged(sourceToken);
                    if ((stopping = Pause(5 * 60000, g_linkEvent))) break;
                    continue;
                }
                SetLink(Link::Ready);
                std::map<std::string, std::wstring> apps;
                std::map<uint32_t, std::wstring> shown;
                while (device.ConnectionStatus() == BluetoothConnectionStatus::Connected) {
                    HANDLE w[3] = {g_stopEvent, g_eventsEvent, g_linkEvent};
                    DWORD r = WaitForMultipleObjects(3, w, FALSE, 60000);
                    if (r == WAIT_OBJECT_0) {
                        stopping = true;
                        break;
                    }
                    // Phone Link opened since: it shows the phone's notifications now, so Tourne's go (else each
                    // shows twice) and Tourne waits for it to end, as before connecting.
                    if (HANDLE phoneLink = phonemap::PhoneLinkRunning()) {
                        CloseHandle(phoneLink);
                        HideAll();
                        shown.clear();
                        if (g_tellWnd) PostMessageW(g_tellWnd, g_tellMsg, 0, 0);
                        break;
                    }
                    // Buttons clicked: their actions, done on the phone (no answer comes but the notification's going).
                    for (;;) {
                        std::pair<uint32_t, uint8_t> a;
                        {
                            std::lock_guard<std::mutex> lock(g.lock);
                            if (g.actions.empty()) break;
                            a = g.actions.front();
                            g.actions.pop_front();
                        }
                        auto command = Act(a.first, a.second);
                        Buffer buffer((uint32_t)command.size());
                        memcpy(buffer.data(), command.data(), command.size());
                        buffer.Length((uint32_t)command.size());
                        try {
                            auto r = point.WriteValueWithResultAsync(buffer, GattWriteOption::WriteWithResponse).get();
                            phonemap::Log(L"notifications: action " + std::to_wstring(a.second) +
                                          (r.Status() == GattCommunicationStatus::Success ? L" done" : L" refused"));
                        } catch (...) {
                        }
                    }
                    int before = g_shown, gone = 0;
                    for (;;) {
                        Event e;
                        {
                            std::lock_guard<std::mutex> lock(g.lock);
                            if (g.events.empty()) break;
                            e = g.events.front();
                            g.events.pop_front();
                        }
                        if (e.id == kRemoved) {
                            {
                                std::lock_guard<std::mutex> lock(g.lock);
                                g.calls.erase(e.uid);
                            }
                            if (auto it = shown.find(e.uid); it != shown.end()) {
                                Hide(e.uid, it->second);
                                shown.erase(it);
                                gone++;
                            }
                        } else {
                            Handle(point, e, apps, shown);
                        }
                    }
                    if (g_shown != before || gone) {
                        phonemap::Log(L"notifications: " + std::to_wstring(g_shown - before) + L" shown, " + std::to_wstring(gone) + L" taken away");
                        if (g_tellWnd) PostMessageW(g_tellWnd, g_tellMsg, 0, 0);
                    }
                }
                {
                    // The phone's ids start again when it does: nothing kept for them carries over.
                    std::lock_guard<std::mutex> lock(g.lock);
                    g.calls.clear();
                    g.actions.clear();
                }
                data.ValueChanged(dataToken);
                source.ValueChanged(sourceToken);
                if (!stopping) SetLink(Link::Away);
                if (stopping) {
                    try {
                        source.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None).get();
                        data.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None).get();
                    } catch (...) {
                    }
                }
            }
            device.ConnectionStatusChanged(statusToken);
            session.MaintainConnection(false);
            session.Close();
            device.Close();
            if (stopping) break;
        } catch (...) {
            SetLink(Link::Away);
            if (Pause(60000)) break;
        }
    }
    SetLink(Link::Off);
    winrt::uninit_apartment();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// For the phone pane.

void Begin(HWND tell, UINT message) {
    g_tellWnd = tell;
    g_tellMsg = message;
    if (g_thread) return;
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_eventsEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_answerEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_linkEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, Run, nullptr, 0, nullptr);
    if (g_thread) SetThreadPriority(g_thread, THREAD_PRIORITY_BELOW_NORMAL);
}

void End() {
    if (!g_thread) return;
    SetEvent(g_stopEvent);
    bool ended = WaitForSingleObject(g_thread, 10000) == WAIT_OBJECT_0;
    CloseHandle(g_thread);
    g_thread = nullptr;
    if (ended)  // (else it still has them: left open)
        for (HANDLE* h : {&g_stopEvent, &g_eventsEvent, &g_answerEvent, &g_linkEvent}) CloseHandle(*h), *h = nullptr;
}

Link LinkNow() {
    std::lock_guard<std::mutex> lock(g.lock);
    return g.link;
}

// Tourne Calls (desktop/calls): when it's installed, the phone's calls go through Windows' own calling with the
// sound on this PC. Started with Tourne Desktop (it stays on its own; one at a time).
std::wstring CallsHelper() {
    WCHAR path[MAX_PATH] = L"";
    SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path);
    std::wstring exe = std::wstring(path) + L"\\Tourne\\Desktop\\calls\\TourneCalls.exe";
    return GetFileAttributesW(exe.c_str()) == INVALID_FILE_ATTRIBUTES ? L"" : exe;
}

void StartCallsHelper() {
    std::wstring exe = CallsHelper();
    if (exe.empty() || FindWindowExW(HWND_MESSAGE, nullptr, L"TourneCalls_Window", nullptr)) return;
    std::wstring line = L"\"" + exe + L"\" run";
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    if (CreateProcessW(exe.c_str(), line.data(), nullptr, nullptr, FALSE, DETACHED_PROCESS, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        phonemap::Log(L"calls: Tourne Calls started");
    }
}

// A notification's button clicked: its action (0 positive, 1 negative), done on the phone. A call's goes to Tourne
// Calls when it runs: answered there, its sound is on this PC.
void Perform(uint32_t uid, uint8_t action) {
    bool call;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        call = g.calls.count(uid) != 0;
    }
    if (call)
        if (HWND helper = FindWindowExW(HWND_MESSAGE, nullptr, L"TourneCalls_Window", nullptr)) {
            PostMessageW(helper, RegisterWindowMessageW(L"TourneCallsCommand"), action == 0 ? 1 : 2, 0);
            phonemap::Log(action == 0 ? L"calls: answer, on this PC" : L"calls: decline");
            return;
        }
    if (!g_thread) return;
    {
        std::lock_guard<std::mutex> lock(g.lock);
        g.actions.push_back({uid, action});
    }
    SetEvent(g_eventsEvent);
}

}  // namespace phoneancs

#else
namespace phoneancs {
enum class Link { Off, NoPhone, PhoneLink, Away, Ready, Refused };
inline void Begin(HWND, UINT) {}
inline void End() {}
inline Link LinkNow() { return Link::Off; }
inline void Perform(uint32_t, uint8_t) {}
inline void StartCallsHelper() {}
constexpr bool kCanClear = false;
inline void Clear(std::wstring const&, std::wstring const&, std::wstring const&) {}
inline void ClearApp(std::wstring const&) {}
inline void HideAll() {}
}  // namespace phoneancs
#endif
