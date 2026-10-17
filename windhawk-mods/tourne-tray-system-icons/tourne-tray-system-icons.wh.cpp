// ==WindhawkMod==
// @id              tourne-tray-system-icons
// @name            Tourne'Tray [System Icons]
// @description     Pixel-art tray icons for network, volume, battery, notifications, privacy and more, with live state and Windows 7 style flyouts
// @version         1.8.3
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lwlanapi -liphlpapi -lpowrprof -lshell32 -lgdi32 -ladvapi32 -lcomctl32 -luuid -lmsimg32 -ldxva2 -lwscapi -lcfgmgr32 -ldwmapi -lversion -lshlwapi -lruntimeobject -lurlmon -lwininet
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Tray` **[System Icons]**

Adds the Tourne pixel tray icons as real tray icons, for the system indicators
that no icon theme can reach: StartAllBack draws its network icon from a font,
the battery meter is built from sprite strips, and Windows 11 has no tray icon
at all for notifications, focus, night light or brightness.

Each icon is a normal tray icon, so it shows up in **Settings > Personalization
> Taskbar > Other system tray icons** (listed as Windows Explorer, told apart by
its picture) and can be turned on there or dragged onto the taskbar.

## Icons

| Icon | Shows | Left click | Right click |
|---|---|---|---|
| Network | Wi-Fi bars, no internet, off, not connected, Ethernet, unplugged, airplane mode | Network flyout | The real network icon's menu |
| Volume | Level, muted, no device | Volume Mixer | The real volume icon's menu |
| VPN | Connected or not | VPN settings | Network settings |
| Battery | Level, charging, battery saver, no battery | Power flyout | The real power icon's menu |
| Notifications | Bell, or Do not disturb | Notification center | Settings |
| Focus | Moon | Notification center | Focus settings |
| Night light | On or off | Night light settings | Display settings |
| Brightness | Sun | Brightness flyout | Display settings |
| Microphone, Camera, Location | Only while an app uses them, with the app names | Privacy settings | Privacy settings |
| Accessibility | Figure | Accessibility settings | Settings |
| Power | Power symbol | Lock, sign out, sleep, restart, shut down | Same |
| Input | Keyboard, with the current language | Input switcher | Language settings |
| Recycle Bin | Item count | Opens the Recycle Bin | Open, empty |
| Clipboard | Clipboard | Clipboard history | Settings |
| Screen capture | Camera | Snipping | Capture now |
| Show hidden icons | Up arrow, down while open | Hidden icons flyout: click, right-click or middle-click an icon as in the tray | Taskbar settings |

On the volume icon, **middle click** switches to the next output device and
**scrolling** changes the volume in steps of 2. Scrolling needs Windows'
"Scroll inactive windows when hovering over them" setting, which is on by
default.

## Flyouts

The network, power and brightness flyouts follow the Windows 7 layouts in the
Tourne style:

* **Network**: what you're connected to, then every Wi-Fi network with its
  signal. Click a network for Connect or Disconnect. A secured network you
  haven't joined before opens Windows' own sign-in, since that's where the
  password is entered.
* **Power**: battery status and the power plans, one click to switch.
* **Brightness**: a slider per monitor, over DDC/CI. Monitors that don't
  answer DDC/CI say so.

## Forwarded clicks

Right-clicking network, volume or battery opens the menu of the system icon
it stands in for, so the original has to stay enabled. It can live in the "^"
popup, or in a Classic Tray Folders folder, out of sight. If it's gone, a
short menu of the matching settings pages is shown instead.

## Not covered

The unread notification count has no public source, so the bell shows Do not
disturb but not unread.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- tone: ink
  $name: Icon tone
  $options:
  - sage: Sage
  - ink: Ink
  - alt: Alt
- shade: true
  $name: Pixel shade
  $description: The one-pixel darker step under each icon.
- trayTheme: true
  $name: Other apps in the Tourne design
  $description: >-
    Tray icons of Discord, OneDrive, Steam, Windscribe, Windows Security and
    others are drawn in the design, following their state where their
    tooltip tells it.
- tourneTooltips: true
  $name: Tourne tooltips
  $description: The tray's tooltips in the Tourne style.
- calendarIcs: ""
  $name: Calendar (.ics)
  $description: >-
    A local file or web address of an .ics calendar for the notification
    centre's agenda. Empty uses the Agenda in Calendar View mod's.
- doubleClickApps: steam.exe
  $name: Open with a double click
  $description: >-
    Programs, by executable name and separated by commas, that open their
    window on a double click of their tray icon. In the hidden icons flyout a
    left click on one sends it a double click.
- icons:
  - network: true
    $name: Network
  - volume: true
    $name: Volume
  - vpn: true
    $name: VPN
  - battery: true
    $name: Battery
  - notifications: true
    $name: Notifications
  - focus: true
    $name: Focus
  - nightLight: true
    $name: Night light
  - brightness: true
    $name: Brightness
  - microphone: true
    $name: Microphone in use
  - camera: true
    $name: Camera in use
  - location: true
    $name: Location in use
  - accessibility: true
    $name: Accessibility
  - power: true
    $name: Power
  - input: true
    $name: Input language
  - recycleBin: true
    $name: Recycle Bin
  - clipboard: true
    $name: Clipboard history
  - capture: true
    $name: Screen capture
  - overflow: true
    $name: Show hidden icons
    $description: >-
      A flyout of the icons hidden from the taskbar, standing in for the "^"
      button. StartAllBack's own "^" can then be turned off.
  - security: true
    $name: Windows Security
  $name: Icons
  $description: >-
    Icons to add. Which ones show on the taskbar is then chosen in Windows
    Settings, like any other tray icon.
*/
// ==/WindhawkModSettings==

// Windhawk includes windows.h before the mod's code, so winsock2.h can't come
// first; its "include me before windows.h" warning is expected here.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#include <winsock2.h>
#pragma clang diagnostic pop
#include <ws2ipdef.h>

#include <windhawk_utils.h>

#include <windows.h>
#include <cfgmgr32.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <highlevelmonitorconfigurationapi.h>
#include <iphlpapi.h>
#include <mmdeviceapi.h>
#include <netioapi.h>
#include <netlistmgr.h>
#include <physicalmonitorenumerationapi.h>
#include <powrprof.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <wlanapi.h>
#include <wscapi.h>
#include <wuapi.h>
#include <tlhelp32.h>
#include <urlmon.h>
#include <wininet.h>
#include <windowsx.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <iterator>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Notifications.h>

////////////////////////////////////////////////////////////////////////////////
// Icons, ids and messages

// New kinds go at the end: the tray id is kIconIdBase + kind, and Windows
// remembers each icon's taskbar placement by that id.
enum IconKind : int {
    kNetwork,
    kVpn,
    kBattery,
    kNotifications,
    kFocus,
    kNightLight,
    kBrightness,
    kMicrophone,
    kCamera,
    kLocation,
    kAccessibility,
    kPower,
    kInput,
    kRecycleBin,
    kClipboard,
    kCapture,
    kVolume,
    kOverflow,
    kSecurity,
    kIconCount
};

constexpr PCWSTR kIconKeys[kIconCount] = {
    L"network",       L"vpn",        L"battery",       L"notifications",
    L"focus",         L"nightLight", L"brightness",    L"microphone",
    L"camera",        L"location",   L"accessibility", L"power",
    L"input",         L"recycleBin", L"clipboard",     L"capture",
    L"volume",        L"overflow",      L"security",
};

// Tray ids fit in 16 bits, since NOTIFYICON_VERSION_4 callbacks carry the id in
// HIWORD(lParam). Chosen away from Explorer's own icons and Classic Tray
// Folders (0xC100+).
constexpr UINT kIconIdBase = 0x7E00;

constexpr UINT kNetworkBits = (1u << kNetwork) | (1u << kVpn);
constexpr UINT kAllBits = (1u << kIconCount) - 1;

constexpr UINT WM_APP_TRAY = WM_APP + 1;
constexpr UINT WM_APP_REFRESH = WM_APP + 2;
constexpr UINT WM_APP_SETTINGS = WM_APP + 3;
constexpr UINT WM_APP_EXIT = WM_APP + 4;
constexpr UINT WM_APP_BIN = WM_APP + 5;
constexpr UINT WM_APP_WHEEL = WM_APP + 6;
constexpr UINT WM_APP_MIDDLE = WM_APP + 7;
constexpr UINT WM_APP_AUDIO_DEVICE = WM_APP + 8;
constexpr UINT WM_APP_FLYOUT_RELOAD = WM_APP + 9;
constexpr UINT WM_APP_BRIGHTNESS_READY = WM_APP + 10;
constexpr UINT WM_APP_JOB_DONE = WM_APP + 11;
constexpr UINT WM_APP_CALENDAR_READY = WM_APP + 12;

constexpr UINT_PTR kTimerPeriodic = 1;
constexpr UINT_PTR kTimerFindTray = 2;
constexpr UINT_PTR kTimerNotifications = 4;
constexpr UINT kPeriodicMs = 30000;

// Volume changes per wheel notch, in percent.
constexpr int kVolumeStep = 2;

// Power setting GUIDs, spelled out so no import library is needed.
constexpr GUID kGuidBatteryPercentage = {
    0xA7AD8041, 0xB45A, 0x4CAE, {0x87, 0xA3, 0xEE, 0xCB, 0xB4, 0x68, 0xA9, 0xE1}};
constexpr GUID kGuidAcDcPowerSource = {
    0x5D3E9A59, 0xE9D5, 0x4B00, {0xA6, 0xBD, 0xFF, 0x34, 0xFF, 0x51, 0x65, 0x48}};
constexpr GUID kGuidPowerSavingStatus = {
    0xE00958C0, 0xC213, 0x4ACE, {0xAC, 0x77, 0xFE, 0xCC, 0xED, 0x2E, 0xEE, 0xA5}};

constexpr GUID kClsidNetworkListManager = {
    0xDCB00C01, 0x570F, 0x4A9B, {0x8D, 0x69, 0x19, 0x9F, 0xDB, 0xA5, 0x72, 0x3B}};
constexpr GUID kIidNetworkListManager = {
    0xDCB00000, 0x570F, 0x4A9B, {0x8D, 0x69, 0x19, 0x9F, 0xDB, 0xA5, 0x72, 0x3B}};
constexpr GUID kIidNetworkListManagerEvents = {
    0xDCB00001, 0x570F, 0x4A9B, {0x8D, 0x69, 0x19, 0x9F, 0xDB, 0xA5, 0x72, 0x3B}};

constexpr GUID kClsidMMDeviceEnumerator = {
    0xBCDE0395, 0xE52F, 0x467C, {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
constexpr GUID kIidMMDeviceEnumerator = {
    0xA95664D2, 0x9614, 0x4F35, {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
constexpr GUID kIidAudioEndpointVolume = {
    0x5CDF2C82, 0x841E, 0x4546, {0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A}};
constexpr GUID kIidAudioEndpointVolumeCallback = {
    0x657804FA, 0xD6AD, 0x4496, {0x8A, 0x60, 0x35, 0x27, 0x52, 0xAF, 0x4F, 0x89}};
constexpr GUID kIidMMNotificationClient = {
    0x7991EEC9, 0x7E89, 0x4D85, {0x83, 0x90, 0x6C, 0x70, 0x3C, 0xEC, 0x60, 0xC0}};
constexpr PROPERTYKEY kPkeyDeviceFriendlyName = {
    {0xA45C254E, 0xDF1C, 0x4EFD, {0x80, 0x20, 0x67, 0xD1, 0x46, 0xA8, 0x50, 0xE0}}, 14};

// Undocumented, but stable since Windows 7 and the only way to change the
// default audio device. Same definition as the AudioSwap mod uses.
constexpr GUID kClsidPolicyConfigClient = {
    0x870AF99C, 0x171D, 0x4F9E, {0xAF, 0x0D, 0xE6, 0x3D, 0xF4, 0x0C, 0x2B, 0xC9}};
constexpr GUID kIidPolicyConfig = {
    0xF8679F50, 0x850A, 0x41CF, {0x9C, 0x72, 0x43, 0x0F, 0x29, 0x02, 0x90, 0xC8}};

struct IPolicyConfig : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetMixFormat(PCWSTR, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(PCWSTR, INT, void**) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(PCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(PCWSTR, void*, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(PCWSTR, INT, PINT, PINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(PCWSTR, PINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(PCWSTR, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(PCWSTR, void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(PCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(PCWSTR deviceId, ERole role) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(PCWSTR, INT) = 0;
};

std::atomic<HWND> g_wnd;
std::atomic<UINT> g_pendingBits;
std::atomic<bool> g_refreshPosted;

void RequestRefresh(UINT bits) {
    g_pendingBits |= bits;
    HWND wnd = g_wnd;
    if (wnd && !g_refreshPosted.exchange(true)) {
        PostMessageW(wnd, WM_APP_REFRESH, 0, 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Settings

struct Settings {
    std::string tone = "sage";
    bool shade = true;
    std::array<bool, kIconCount> enabled{};
    bool trayTheme = true;
    bool tourneTooltips = true;
    std::wstring calendarIcs;
    std::wstring doubleClickApps;
};

Settings LoadSettings() {
    Settings settings;
    WindhawkUtils::StringSetting tone =
        WindhawkUtils::StringSetting::make(L"tone");
    if (wcscmp(tone.get(), L"ink") == 0) {
        settings.tone = "ink";
    } else if (wcscmp(tone.get(), L"alt") == 0) {
        settings.tone = "alt";
    }
    settings.shade = Wh_GetIntSetting(L"shade");
    settings.trayTheme = Wh_GetIntSetting(L"trayTheme");
    settings.tourneTooltips = Wh_GetIntSetting(L"tourneTooltips");
    settings.calendarIcs = WindhawkUtils::StringSetting::make(L"calendarIcs").get();
    settings.doubleClickApps = WindhawkUtils::StringSetting::make(L"doubleClickApps").get();
    for (int k = 0; k < kIconCount; k++) {
        settings.enabled[k] = Wh_GetIntSetting(L"icons.%s", kIconKeys[k]);
    }
    return settings;
}

Settings g_settings;  // Mod thread only.

////////////////////////////////////////////////////////////////////////////////
// Pixel art, ported from the design's generator: 12x12 grids drawn on a 13x13
// canvas with a one-pixel drop shade, at whole-number scales only.

using Grid = std::array<std::array<char, 12>, 12>;

Grid P(std::initializer_list<std::string_view> rows) {
    Grid g;
    for (auto& row : g) {
        row.fill('.');
    }
    int y = 0;
    for (auto row : rows) {
        for (int x = 0; x < 12 && x < (int)row.size(); x++) {
            g[y][x] = row[x];
        }
        if (++y == 12) {
            break;
        }
    }
    return g;
}

void Put(Grid& g, std::initializer_list<std::pair<int, int>> pts, char ch) {
    for (auto [x, y] : pts) {
        if (x >= 0 && x < 12 && y >= 0 && y < 12) {
            g[y][x] = ch;
        }
    }
}

const std::map<std::string_view, std::array<std::string_view, 5>> kBadges = {
    {"ok", {".....", "....b", "...b.", "b.b..", ".b..."}},
    {"warn", {"..b..", "..b..", "..b..", ".....", "..b.."}},
    {"err", {"b...b", ".b.b.", "..b..", ".b.b.", "b...b"}},
    {"off", {"b...b", ".b.b.", "..b..", ".b.b.", "b...b"}},
    {"dot", {".....", ".bbb.", ".bbb.", ".bbb.", "....."}},
    {"down", {"..b..", "..b..", "b.b.b", ".bbb.", "..b.."}},
};

Grid Badge(Grid g, std::string_view name) {
    auto const& rows = kBadges.at(name);
    std::vector<std::pair<int, int>> pts;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (rows[y][x] != '.') {
                pts.push_back({x + 7, y + 7});
            }
        }
    }
    for (auto [x, y] : pts) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int X = x + dx, Y = y + dy;
                if (X >= 0 && X < 12 && Y >= 0 && Y < 12 && g[Y][X] != 'b') {
                    g[Y][X] = '.';
                }
            }
        }
    }
    for (auto [x, y] : pts) {
        g[y][x] = 'b';
    }
    return g;
}

Grid Wifi(int n) {
    Grid g = P({});
    Put(g, {{3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {1, 2}, {2, 2}, {9, 2}, {10, 2}, {0, 3}, {11, 3}},
        n >= 4 ? '#' : 'd');
    Put(g, {{4, 4}, {5, 4}, {6, 4}, {7, 4}, {2, 5}, {3, 5}, {8, 5}, {9, 5}}, n >= 3 ? '#' : 'd');
    Put(g, {{5, 7}, {6, 7}, {4, 8}, {7, 8}}, n >= 2 ? '#' : 'd');
    Put(g, {{5, 10}, {6, 10}, {5, 11}, {6, 11}}, n >= 1 ? '#' : 'd');
    return g;
}

Grid Battery(int level, bool charging = false) {
    Grid g = P({"", "", "", "##########..", "#........#..", "#........##.",
                "#........##.", "#........#..", "##########.."});
    if (charging) {
        Put(g, {{5, 4}, {6, 4}, {3, 5}, {4, 5}, {5, 5}, {4, 6}, {5, 6}, {6, 6}, {3, 7}, {4, 7}}, 'o');
    } else {
        for (int i = 0; i < level; i++) {
            Put(g, {{2 + i, 5}, {2 + i, 6}}, 'o');
        }
    }
    return g;
}

// level: 0 to 3, or -1 for muted.
Grid Speaker(int level) {
    Grid g = P({"", ".....#......", "....##......", ".###.#......", ".#...#......",
                ".#...#......", ".#...#......", ".###.#......", "....##......", ".....#......"});
    if (level < 0) {
        auto const& rows = kBadges.at("err");
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                if (rows[y][x] != '.') {
                    g[y + 3][x + 7] = 'b';
                }
            }
        }
        return g;
    }
    if (level >= 1) Put(g, {{7, 4}, {7, 5}, {7, 6}}, '#');
    if (level >= 2) Put(g, {{9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}}, '#');
    if (level >= 3) Put(g, {{11, 2}, {11, 3}, {11, 4}, {11, 5}, {11, 6}, {11, 7}, {11, 8}}, '#');
    return g;
}

const Grid kArtMonitor = P({"", ".##########.", ".#........#.", ".#........#.", ".#........#.",
                            ".#........#.", ".##########.", ".....##.....", "...######..."});
const Grid kArtPlane = P({".....##.....", ".....##.....", "....####....", "..########..",
                          "############", ".....##.....", ".....##.....", "....####....",
                          "...######...", "...##..##..."});
const Grid kArtLock = P({"", "....####....", "...#....#...", "...#....#...", "..########..",
                         "..#......#..", "..#..##..#..", "..#..##..#..", "..#......#..", "..########.."});
const Grid kArtBell = P({".....##.....", "....####....", "...#....#...", "...#....#...", "...#....#...",
                         "..#......#..", "..#......#..", ".#........#.", ".##########.", "", ".....##....."});
const Grid kArtMoon = P({"", "....####....", "...###......", "..###.......", "..##........",
                         "..##........", "..##........", "..###....##.", "...#######..", "....#####..."});
const Grid kArtSun = P({".....##.....", ".#...##...#.", "..#......#..", "...######...", "...#....#...",
                        "##.#....#.##", "##.#....#.##", "...#....#...", "...######...", "..#......#..",
                        ".#...##...#.", ".....##....."});
const Grid kArtAccess = P({".....##.....", ".....##.....", "", ".##########.", ".....##.....",
                           ".....##.....", ".....##.....", "....#..#....", "...#....#...", "..#......#.."});
const Grid kArtKeyboard = P({"", "", "", "############", "#..........#", "#.#.#.#.#..#",
                             "#..........#", "#.#......#.#", "#..........#", "############"});
const Grid kArtPower = P({".....##.....", ".....##.....", "..#..##..#..", ".#...##...#.", "#....##....#",
                          "#..........#", "#..........#", "#..........#", ".#........#.", "..########.."});
const Grid kArtMic = P({"....####....", "....#..#....", "....#..#....", "....#..#....", "..#.#..#.#..",
                        "..#.####.#..", "..#......#..", "...######...", ".....##.....", "...######..."});
const Grid kArtCamera = P({"", "", "", "########....", "#......#..#.", "#......####.",
                           "#......####.", "#......#..#.", "########...."});
const Grid kArtLocationPin = P({"....####....", "...#....#...", "..#..##..#..", "..#.#..#.#..", "..#..##..#..",
                                "...#....#...", "...#....#...", "....#..#....", "....#..#....", ".....##....."});
const Grid kArtTrash = P({"....####....", ".##########.", "..#......#..", "..#.#..#.#..", "..#.#..#.#..",
                          "..#.#..#.#..", "..#.#..#.#..", "..#.#..#.#..", "..#......#..", "...######..."});
const Grid kArtClipboard = P({"....####....", ".###.##.###.", ".#........#.", ".#.######.#.", ".#........#.",
                              ".#.######.#.", ".#........#.", ".#.####...#.", ".#........#.", ".##########."});
// The design's overflow chevron; "down" is the same rows upside down, which
// also moves it to the top of the canvas, as in the design.
const Grid kArtChevronUp = P({"", "", "", "", ".....##.....", "....####....", "...##..##...", "..##....##.."});
const Grid kArtChevronDown = P({"..##....##..", "...##..##...", "....####....", ".....##....."});

// Shapes for the icons that stand in for other programs' tray icons, and for
// the Windows Security flyout, from the design.
const Grid kArtShield = P({".....##.....", "...##..##...", ".##......##.", ".#........#.", ".#........#.", ".#........#.",
                           ".#........#.", "..#......#..", "..#......#..", "...#....#...", "....#..#....", ".....##....."});
const Grid kArtHeart = P({"", "", ".###....###.", "#...#..#...#", "#....##....#", "#..........#", ".#........#.",
                          "..#......#..", "...#....#...", "....#..#....", ".....##....."});
const Grid kArtHeadphones = P({"", "...######...", "..#......#..", ".#........#.", ".#........#.", ".#........#.",
                               "###......###", "###......###", "###......###", "###......###"});
const Grid kArtBluetooth = P({"......#.....", "......##....", "......#.#...", "...#..#..#..", "....#.#.#...", ".....##.....",
                              ".....##.....", "....#.#.#...", "...#..#..#..", "......#.#...", "......##....", "......#....."});
const Grid kArtUsb = P({"....####....", "....#..#....", "....####....", "...######...", "...#....#...", "...#....#...",
                        "...#....#...", "...#....#...", "...#....#...", "...######..."});
const Grid kArtPrinter = P({"", "...######...", "...#....#...", "############", "#..........#", "#.......o..#",
                            "#..........#", "#..######..#", "...#....#...", "...######..."});
const Grid kArtPhone = P({"...######...", "...#....#...", "...#....#...", "...#....#...", "...#....#...", "...#....#...",
                          "...#....#...", "...#....#...", "...#.##.#...", "...######..."});
const Grid kArtCloud = P({"", "", "....####....", "...#....#...", ".###.....##.", "#..........#", "#..........#",
                          "#..........#", ".##########."});
const Grid kArtChat = P({"", ".##########.", ".#........#.", ".#.######.#.", ".#........#.", ".#.####...#.",
                         ".#........#.", ".#..#######.", ".#.#........", ".##........."});
const Grid kArtPad = P({"", "", "", "..########..", ".#........#.", "#..#....o..#", "#.###..o.o.#", "#..#....o..#",
                        "#...####...#", "#..#....#..#", ".##......##."});
const Grid kArtUpdate = P({"", "...######...", "..#......#.#", ".#........##", ".#.......###", ".#..........",
                           "..........#.", "###.......#.", "##........#.", "#.#......#..", "...######..."});

const Grid kArtGpu = P({"", "", "##########..", "#........###", "#.###.###..#", "#.#.#.#.#..#",
                        "#.###.###..#", "#........###", "##########..", "..#.#.#.#..."});
const Grid kArtThermometer = P({"....##......", "...#..#.##..", "...#..#.....", "...#..#.##..",
                                "...#oo#.....", "...#oo#.##..", "...#oo#.....", "..#oooo#....",
                                ".#oooooo#...", ".#oooooo#...", "..#oooo#....", "...####....."});

const Grid kArtGear = P({"....####....", ".##.####.##.", ".##########.", "..###..###..",
                         "####....####", "###......###", "###......###", "####....####",
                         "..###..###..", ".##########.", ".##.####.##.", "....####...."});
const Grid kArtSpark = P({".....##.....", ".#...##...#.", "..#..##..#..", "...#.##.#...",
                          "....####....", "############", "############", "....####....",
                          "...#.##.#...", "..#..##..#..", ".#...##...#.", ".....##....."});
const Grid kArtSpotify = P({"...######...", ".##......##.", ".#.######.#.", "#.#......#.#",
                            "#...####...#", "#..#....#..#", "#....##....#", "#...#..#...#",
                            "#..........#", ".#........#.", ".##......##.", "...######..."});
const Grid kArtTiles = P({"", "############", "#....#.....#", "#....#.....#", "#....#.....#",
                          "#....#######", "#....#.....#", "#....#.....#", "#....#.....#",
                          "############"});

const Grid kArtNamedShot = P({"", "###......###", "#..........#", "#..........#", "",
                              "...oooooo...", "", "...oooo.....", "#..........#",
                              "#..........#", "###......###"});
const Grid kArtWallpaper = P({"", "############", "#..........#", "#.......oo.#", "#.......oo.#",
                              "#...#......#", "#..###..#..#", "#.#####.##.#", "#.########.#",
                              "############"});
// A fan: one blade, turned a quarter at a time around the hub.
Grid Pinwheel() {
    Grid g = P({});
    std::vector<std::pair<int, int>> blade = {{5, 4}, {5, 3}, {4, 2}, {5, 2}, {6, 2}, {3, 1},
                                              {4, 1}, {5, 1}, {6, 1}, {3, 0}, {4, 0}, {5, 0}};
    for (int turn = 0; turn < 4; turn++) {
        for (auto& [x, y] : blade) {
            g[y][x] = '#';
            int turnedX = 11 - y;
            y = x;
            x = turnedX;
        }
    }
    Put(g, {{5, 5}, {6, 5}, {5, 6}, {6, 6}}, 'o');
    return g;
}
const Grid kArtFan = Pinwheel();
// Pixel digits for sensor readings: 4x7 for up to two digits, 3x5 for three.
constexpr std::array<std::array<std::string_view, 7>, 10> kDigitsLarge = {{
    {".##.", "#..#", "#..#", "#..#", "#..#", "#..#", ".##."},
    {"..#.", ".##.", "..#.", "..#.", "..#.", "..#.", ".###"},
    {".##.", "#..#", "...#", "..#.", ".#..", "#...", "####"},
    {"###.", "...#", "...#", ".##.", "...#", "...#", "###."},
    {"#..#", "#..#", "#..#", "####", "...#", "...#", "...#"},
    {"####", "#...", "###.", "...#", "...#", "#..#", ".##."},
    {".##.", "#...", "###.", "#..#", "#..#", "#..#", ".##."},
    {"####", "...#", "..#.", "..#.", ".#..", ".#..", ".#.."},
    {".##.", "#..#", "#..#", ".##.", "#..#", "#..#", ".##."},
    {".##.", "#..#", "#..#", ".###", "...#", "...#", ".##."},
}};
constexpr std::array<std::array<std::string_view, 5>, 10> kDigitsSmall = {{
    {"###", "#.#", "#.#", "#.#", "###"},
    {".#.", "##.", ".#.", ".#.", "###"},
    {"###", "..#", "###", "#..", "###"},
    {"###", "..#", "###", "..#", "###"},
    {"#.#", "#.#", "###", "..#", "..#"},
    {"###", "#..", "###", "..#", "###"},
    {"###", "#..", "###", "#.#", "###"},
    {"###", "..#", "..#", ".#.", ".#."},
    {"###", "#.#", "###", "#.#", "###"},
    {"###", "#.#", "###", "..#", "###"},
}};
struct Art {
    Grid grid;
    const char* tone;    // Overrides the base tone.
    const char* accent;  // 'o' pixels.
    const char* badge;   // 'b' pixels.

    Art(Grid grid,
        const char* tone = nullptr,
        const char* accent = nullptr,
        const char* badge = nullptr)
        : grid(grid), tone(tone), accent(accent), badge(badge) {}
};

// A sensor reading, from a "readout:<value>[:hot]" design. Hot readings carry
// the notification bell's dot, with the digits moved up to make room.
Art Readout(std::string_view id) {
    std::string_view rest = id.substr(8);
    int value = std::clamp(atoi(std::string(rest.substr(0, rest.find(':'))).c_str()), 0, 999);
    std::string text = std::to_string(value);
    Grid g = P({});
    bool large = text.size() <= 2;
    int width = large ? 4 : 3;
    int height = large ? 7 : 5;
    int x0 = (12 - ((int)text.size() * (width + 1) - 1)) / 2;
    bool hot = rest.find(":hot") != std::string_view::npos;
    int y0 = hot ? 0 : (12 - height) / 2;
    for (size_t n = 0; n < text.size(); n++) {
        int digit = text[n] - '0';
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                char ch = large ? kDigitsLarge[digit][y][x] : kDigitsSmall[digit][y][x];
                if (ch == '#') {
                    g[y0 + y][x0 + (int)n * (width + 1) + x] = '#';
                }
            }
        }
    }
    if (hot) {
        return {Badge(g, "dot"), nullptr, nullptr, "dot"};
    }
    return {g};
}
Art ArtFor(std::string_view id) {
    if (id == "wifi-4") return {Wifi(4)};
    if (id == "wifi-3") return {Wifi(3)};
    if (id == "wifi-2") return {Wifi(2)};
    if (id == "wifi-1") return {Wifi(1)};
    if (id == "wifi-no-internet") return {Badge(Wifi(4), "warn"), nullptr, nullptr, "warn"};
    if (id == "wifi-off") return {Badge(Wifi(0), "off"), nullptr, nullptr, "off"};
    if (id == "wifi-disconnected") return {Badge(Wifi(0), "dot"), nullptr, nullptr, "dot"};
    if (id == "ethernet") return {kArtMonitor};
    if (id == "ethernet-no-internet") return {Badge(kArtMonitor, "warn"), nullptr, nullptr, "warn"};
    if (id == "ethernet-unplugged") return {Badge(kArtMonitor, "err"), nullptr, nullptr, "err"};
    if (id == "airplane") return {kArtPlane};
    if (id == "vpn") return {kArtLock};
    if (id == "vpn-on") return {Badge(kArtLock, "ok"), nullptr, nullptr, "ok"};
    if (id == "battery-full") return {Battery(6), nullptr, "green"};
    if (id == "battery-high") return {Battery(4), nullptr, "green"};
    if (id == "battery-mid") return {Battery(3), nullptr, "sage"};
    if (id == "battery-low") return {Badge(Battery(1), "warn"), nullptr, "red", "warn"};
    if (id == "battery-charging") return {Battery(0, true), nullptr, "green"};
    if (id == "battery-saver") return {Badge(Battery(2), "dot"), nullptr, "coral", "dot"};
    if (id == "battery-none") return {Badge(Battery(0), "off"), nullptr, nullptr, "off"};
    if (id == "audio-mute") return {Speaker(-1), nullptr, nullptr, "err"};
    if (id == "audio-0") return {Speaker(0)};
    if (id == "audio-low") return {Speaker(1)};
    if (id == "audio-mid") return {Speaker(2)};
    if (id == "audio-high") return {Speaker(3)};
    if (id == "audio-no-device") return {Badge(Speaker(0), "warn"), nullptr, nullptr, "warn"};
    if (id == "bell") return {kArtBell};
    if (id == "bell-dnd") return {Badge(kArtBell, "off"), nullptr, nullptr, "off"};
    if (id == "bell-unread") return {Badge(kArtBell, "dot"), nullptr, nullptr, "dot"};
    if (id == "focus") return {kArtMoon};
    if (id == "night-light") return {kArtSun, "coral"};
    if (id == "night-light-off") return {kArtSun, "alt"};
    if (id == "brightness") return {kArtSun};
    if (id == "mic-in-use") return {Badge(kArtMic, "dot"), nullptr, nullptr, "dot"};
    if (id == "camera-in-use") return {Badge(kArtCamera, "dot"), nullptr, nullptr, "dot"};
    if (id == "location-in-use") return {Badge(kArtLocationPin, "dot"), nullptr, nullptr, "dot"};
    if (id == "accessibility") return {kArtAccess};
    if (id == "power") return {kArtPower};
    if (id == "keyboard") return {kArtKeyboard};
    if (id == "app-bin") return {kArtTrash};
    if (id == "app-clipboard") return {kArtClipboard};
    if (id == "app-capture") return {kArtCamera};
    if (id == "overflow-up") return {kArtChevronUp, "ink"};
    if (id == "overflow-down") return {kArtChevronDown, "ink"};
    if (id == "security-ok") return {Badge(kArtShield, "ok"), nullptr, nullptr, "ok"};
    if (id == "security-warn") return {Badge(kArtShield, "warn"), nullptr, nullptr, "warn"};
    if (id == "security-alert") return {Badge(kArtShield, "err"), nullptr, nullptr, "err"};
    if (id == "security-off") return {Badge(kArtShield, "off"), "alt", nullptr, "off"};
    if (id == "firewall-on") return {Badge(kArtLock, "ok"), nullptr, nullptr, "ok"};
    if (id == "firewall-off") return {Badge(kArtLock, "err"), nullptr, nullptr, "err"};
    if (id == "health-ok") return {Badge(kArtHeart, "ok"), nullptr, nullptr, "ok"};
    if (id == "health-warn") return {Badge(kArtHeart, "warn"), nullptr, nullptr, "warn"};
    if (id == "health-alert") return {Badge(kArtHeart, "err"), nullptr, nullptr, "err"};
    if (id == "update") return {kArtUpdate};
    if (id == "update-available") return {Badge(kArtUpdate, "dot"), nullptr, nullptr, "dot"};
    if (id == "restart-required") return {Badge(kArtPower, "warn"), nullptr, nullptr, "warn"};
    if (id == "bluetooth") return {kArtBluetooth};
    if (id == "bluetooth-connected") return {Badge(kArtBluetooth, "ok"), nullptr, nullptr, "ok"};
    if (id == "bluetooth-off") return {Badge(kArtBluetooth, "off"), "alt", nullptr, "off"};
    if (id == "usb") return {kArtUsb};
    if (id == "printer") return {kArtPrinter, nullptr, "green"};
    if (id == "phone") return {Badge(kArtPhone, "ok"), nullptr, nullptr, "ok"};
    if (id == "mic") return {kArtMic};
    if (id == "app-audio") return {kArtHeadphones, "coral"};
    if (id == "app-chat") return {kArtChat};
    if (id == "app-chat-unread") return {Badge(kArtChat, "dot"), nullptr, nullptr, "dot"};
    if (id == "app-cloud") return {Badge(kArtCloud, "ok"), nullptr, nullptr, "ok"};
    if (id == "app-cloud-sync") return {Badge(kArtCloud, "down"), nullptr, nullptr, "down"};
    if (id == "app-cloud-error") return {Badge(kArtCloud, "err"), nullptr, nullptr, "err"};
    if (id == "app-system") return {kArtGear};
    if (id == "app-claude") return {kArtSpark, "coral"};
    if (id == "app-spotify") return {kArtSpotify, "green"};
    if (id == "app-fan") return {kArtFan};
    if (id == "app-tiling") return {kArtTiles};
    if (id == "app-screenshot") return {kArtNamedShot};
    if (id == "app-screenshot-paused") return {Badge(kArtNamedShot, "off"), "alt", nullptr, "off"};
    if (id == "app-wallpaper") return {kArtWallpaper};
    if (id == "app-wallpaper-paused") return {Badge(kArtWallpaper, "off"), "alt", nullptr, "off"};
    if (id == "gpu") return {kArtGpu};
    if (id == "sensor") return {kArtThermometer};
    if (id.starts_with("readout:")) return Readout(id);
    if (id == "app-game") return {kArtPad, nullptr, "coral"};
    return {P({})};
}

uint32_t ToneColor(std::string_view tone) {
    if (tone == "ink") return 0x66EA97;
    if (tone == "green") return 0x15FFA2;
    if (tone == "red") return 0xFF8F8F;
    if (tone == "coral") return 0xFF6F77;
    if (tone == "alt") return 0x5D8D6F;
    if (tone == "dim") return 0x1E2F36;
    return 0x64AA89;  // sage
}

uint32_t ShadeOf(uint32_t color) {
    switch (color) {
        case 0x64AA89: return 0x3D6657;
        case 0x66EA97: return 0x3C865C;
        case 0x15FFA2: return 0x1B9650;
        case 0xFF8F8F: return 0x8D585A;
        case 0xFF6F77: return 0xA95D5D;
        case 0x5D8D6F: return 0x366E53;
    }
    return color;
}

std::string_view BadgeTone(const char* badge) {
    std::string_view b = badge ? badge : "";
    if (b == "warn" || b == "dot") return "coral";
    if (b == "err") return "red";
    if (b == "off") return "alt";
    return "green";
}

// 0xAARRGGBB pixels, straight alpha, size x size.
std::vector<uint32_t> RenderArt(Art const& art,
                                std::string_view baseTone,
                                bool shade,
                                int size) {
    auto colorOf = [&](char ch) -> uint32_t {
        switch (ch) {
            case '#': return ToneColor(art.tone ? art.tone : baseTone);
            case 'o': return ToneColor(art.accent ? art.accent : "coral");
            case 'b': return ToneColor(BadgeTone(art.badge));
            case 'd': return ToneColor("dim");
        }
        return 0;
    };

    uint32_t source[13][13] = {};
    auto const& g = art.grid;
    if (shade) {
        for (int y = 0; y < 12; y++) {
            for (int x = 0; x < 12; x++) {
                char ch = g[y][x];
                if (ch == '.' || ch == 'd') {
                    continue;
                }
                bool below = y + 1 < 12 && x + 1 < 12 && g[y + 1][x + 1] != '.';
                if (!below) {
                    source[y + 1][x + 1] = 0xFF000000 | ShadeOf(colorOf(ch));
                }
            }
        }
    }
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (g[y][x] != '.') {
                source[y][x] = 0xFF000000 | colorOf(g[y][x]);
            }
        }
    }

    std::vector<uint32_t> pixels((size_t)size * size, 0);
    int scale = std::max(1, size / 13);
    int offset = (size - 13 * scale) / 2;
    for (int y = 0; y < 13 * scale; y++) {
        for (int x = 0; x < 13 * scale; x++) {
            int X = x + offset, Y = y + offset;
            if (X >= 0 && X < size && Y >= 0 && Y < size) {
                pixels[(size_t)Y * size + X] = source[y / scale][x / scale];
            }
        }
    }
    return pixels;
}

HBITMAP CreateDib(int width, int height, void** bits) {
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    return CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, bits, nullptr, 0);
}

HICON CreateIconFromPixels(std::vector<uint32_t> const& pixels, int size) {
    void* bits = nullptr;
    HBITMAP color = CreateDib(size, size, &bits);
    if (!color || !bits) {
        return nullptr;
    }
    memcpy(bits, pixels.data(), pixels.size() * 4);

    std::vector<BYTE> maskBits((size_t)((size + 15) / 16) * 2 * size, 0);
    HBITMAP mask = CreateBitmap(size, size, 1, 1, maskBits.data());

    ICONINFO iconInfo{};
    iconInfo.fIcon = TRUE;
    iconInfo.hbmMask = mask;
    iconInfo.hbmColor = color;
    HICON icon = CreateIconIndirect(&iconInfo);

    DeleteObject(color);
    DeleteObject(mask);
    return icon;
}

////////////////////////////////////////////////////////////////////////////////
// Registry helpers

bool ReadDword(HKEY root, PCWSTR path, PCWSTR name, DWORD* value) {
    DWORD size = sizeof(*value);
    return RegGetValueW(root, path, name, RRF_RT_REG_DWORD, nullptr, value,
                        &size) == ERROR_SUCCESS;
}

bool ReadQword(HKEY key, PCWSTR name, ULONGLONG* value) {
    DWORD size = sizeof(*value);
    return RegGetValueW(key, nullptr, name, RRF_RT_REG_QWORD, nullptr, value,
                        &size) == ERROR_SUCCESS;
}

constexpr WCHAR kRadioStatePath[] =
    L"SYSTEM\\CurrentControlSet\\Control\\RadioManagement\\SystemRadioState";
constexpr WCHAR kNotificationsPath[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings";
constexpr WCHAR kNightLightPath[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\CloudStore\\Store\\"
    L"DefaultAccount\\Current\\default$windows.data.bluelightreduction."
    L"bluelightreductionstate\\windows.data.bluelightreduction."
    L"bluelightreductionstate";
constexpr WCHAR kConsentStorePath[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\"
    L"ConsentStore\\";

////////////////////////////////////////////////////////////////////////////////
// State of each icon

struct IconState {
    std::string art;
    std::wstring tip;
    bool hidden = false;
};

std::wstring Utf8ToWide(std::string_view s) {
    if (s.empty()) {
        return {};
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

bool ContainsAny(std::wstring const& lower,
                 std::initializer_list<std::wstring_view> words) {
    return std::any_of(words.begin(), words.end(), [&](std::wstring_view w) {
        return lower.find(w) != std::wstring::npos;
    });
}

// Network (mod thread only).
INetworkListManager* g_networkListManager;
HANDLE g_wlan;

struct NetworkInfo {
    bool airplane = false;
    bool wifiAdapter = false;
    bool wifiRadioOff = false;
    bool wifiConnected = false;
    bool wifiInternet = false;
    int wifiQuality = 0;
    std::wstring ssid;
    bool ethernetAdapter = false;
    bool ethernetConnected = false;
    bool ethernetInternet = false;
    bool vpnConnected = false;
    std::wstring vpnName;
};

NetworkInfo QueryNetwork() {
    NetworkInfo info;

    DWORD radio = 0;
    info.airplane =
        ReadDword(HKEY_LOCAL_MACHINE, kRadioStatePath, nullptr, &radio) && radio;

    // Internet reachability and network name per adapter, as the Network List
    // Manager sees it.
    struct AdapterNetwork {
        GUID adapter;
        NLM_CONNECTIVITY flags;
        std::wstring name;
    };
    std::vector<AdapterNetwork> connectivity;
    if (g_networkListManager) {
        IEnumNetworkConnections* connections = nullptr;
        if (SUCCEEDED(g_networkListManager->GetNetworkConnections(&connections)) &&
            connections) {
            INetworkConnection* connection = nullptr;
            ULONG fetched = 0;
            while (connections->Next(1, &connection, &fetched) == S_OK &&
                   fetched) {
                GUID adapter{};
                NLM_CONNECTIVITY flags = NLM_CONNECTIVITY_DISCONNECTED;
                if (SUCCEEDED(connection->GetAdapterId(&adapter)) &&
                    SUCCEEDED(connection->GetConnectivity(&flags))) {
                    std::wstring name;
                    INetwork* network = nullptr;
                    if (SUCCEEDED(connection->GetNetwork(&network)) && network) {
                        BSTR text = nullptr;
                        if (SUCCEEDED(network->GetName(&text)) && text) {
                            name = text;
                            SysFreeString(text);
                        }
                        network->Release();
                    }
                    connectivity.push_back({adapter, flags, std::move(name)});
                }
                connection->Release();
            }
            connections->Release();
        }
    }
    auto hasInternet = [&](GUID const& adapter) {
        for (auto const& [id, flags, name] : connectivity) {
            if (IsEqualGUID(id, adapter) &&
                (flags & (NLM_CONNECTIVITY_IPV4_INTERNET |
                          NLM_CONNECTIVITY_IPV6_INTERNET))) {
                return true;
            }
        }
        return false;
    };

    MIB_IF_TABLE2* table = nullptr;
    if (GetIfTable2(&table) == NO_ERROR) {
        for (ULONG i = 0; i < table->NumEntries; i++) {
            MIB_IF_ROW2 const& row = table->Table[i];
            bool hardware = row.InterfaceAndOperStatusFlags.HardwareInterface;
            bool up = row.OperStatus == IfOperStatusUp;
            bool media = row.MediaConnectState == MediaConnectStateConnected;

            if (row.Type == IF_TYPE_IEEE80211 && hardware) {
                info.wifiAdapter = true;
                if (up && media && hasInternet(row.InterfaceGuid)) {
                    info.wifiInternet = true;
                }
            } else if (row.Type == IF_TYPE_ETHERNET_CSMACD && hardware &&
                       row.InterfaceAndOperStatusFlags.ConnectorPresent) {
                info.ethernetAdapter = true;
                if (up && media) {
                    info.ethernetConnected = true;
                    if (hasInternet(row.InterfaceGuid)) {
                        info.ethernetInternet = true;
                    }
                }
            } else if (up && !hardware) {
                std::wstring text = std::wstring(row.Description) + L" " + row.Alias;
                CharLowerBuffW(text.data(), (DWORD)text.size());
                if (ContainsAny(text, {L"teredo", L"6to4", L"isatap", L"ip-https",
                                       L"loopback", L"hyper-v", L"vethernet",
                                       L"virtualbox", L"vmware"})) {
                    continue;
                }
                bool vpn = row.Type == IF_TYPE_PPP ||
                           ContainsAny(text, {L"vpn", L"wireguard", L"wintun",
                                              L"openvpn", L"tap-windows",
                                              L"tailscale", L"zerotier",
                                              L"nordlynx", L"windscribe",
                                              L"proton", L"mullvad",
                                              L"anyconnect", L"forti",
                                              L"globalprotect", L"pangp"});
                if (vpn && !info.vpnConnected) {
                    info.vpnConnected = true;
                    info.vpnName = row.Alias;
                }
            }
        }
        FreeMibTable(table);
    }

    if (g_wlan) {
        WLAN_INTERFACE_INFO_LIST* list = nullptr;
        if (WlanEnumInterfaces(g_wlan, nullptr, &list) == ERROR_SUCCESS) {
            bool anyRadioOn = false;
            bool anyRadioKnown = false;
            for (DWORD i = 0; i < list->dwNumberOfItems; i++) {
                auto const& iface = list->InterfaceInfo[i];

                WLAN_RADIO_STATE* radioState = nullptr;
                DWORD size = 0;
                if (WlanQueryInterface(g_wlan, &iface.InterfaceGuid,
                                       wlan_intf_opcode_radio_state, nullptr,
                                       &size, (PVOID*)&radioState,
                                       nullptr) == ERROR_SUCCESS) {
                    for (DWORD p = 0; p < radioState->dwNumberOfPhys; p++) {
                        anyRadioKnown = true;
                        auto const& phy = radioState->PhyRadioState[p];
                        if (phy.dot11SoftwareRadioState == dot11_radio_state_on &&
                            phy.dot11HardwareRadioState == dot11_radio_state_on) {
                            anyRadioOn = true;
                        }
                    }
                    WlanFreeMemory(radioState);
                }

                if (iface.isState != wlan_interface_state_connected) {
                    continue;
                }
                // Not wlan_intf_opcode_current_connection: since Windows 11
                // 24H2 that counts as location use, and this runs on every
                // network refresh. The signal comes from rssi (dBm, mapped
                // the way Windows does: -100 is 0%, -50 and up is 100%) and the
                // name from the Network List Manager.
                info.wifiConnected = true;
                LONG* rssi = nullptr;
                size = 0;
                if (WlanQueryInterface(g_wlan, &iface.InterfaceGuid,
                                       wlan_intf_opcode_rssi, nullptr, &size,
                                       (PVOID*)&rssi, nullptr) == ERROR_SUCCESS) {
                    info.wifiQuality = std::clamp((int)(2 * (*rssi + 100)), 0, 100);
                    WlanFreeMemory(rssi);
                }
                for (auto const& entry : connectivity) {
                    if (IsEqualGUID(entry.adapter, iface.InterfaceGuid)) {
                        info.ssid = entry.name;
                        break;
                    }
                }
            }
            info.wifiRadioOff = anyRadioKnown && !anyRadioOn;
            WlanFreeMemory(list);
        }
    }

    return info;
}

int WifiBars(int quality) {
    return quality >= 75 ? 4 : quality >= 50 ? 3 : quality >= 25 ? 2 : 1;
}

void ComputeNetwork(IconState* network, IconState* vpn) {
    NetworkInfo n = QueryNetwork();

    if (n.airplane) {
        *network = {"airplane", L"Airplane mode ... ON"};
    } else if (n.ethernetConnected && (n.ethernetInternet || !n.wifiConnected)) {
        *network = n.ethernetInternet
                       ? IconState{"ethernet", L"Ethernet ... Internet access"}
                       : IconState{"ethernet-no-internet", L"Ethernet ... No internet"};
    } else if (n.wifiConnected) {
        std::wstring name = n.ssid.empty() ? L"Wi-Fi" : n.ssid;
        if (!n.wifiInternet) {
            *network = {"wifi-no-internet", name + L" ... No internet"};
        } else {
            *network = {"wifi-" + std::to_string(WifiBars(n.wifiQuality)),
                        name + L" ... Internet access"};
        }
    } else if (n.wifiAdapter) {
        *network = n.wifiRadioOff
                       ? IconState{"wifi-off", L"Wi-Fi ... OFF"}
                       : IconState{"wifi-disconnected", L"Not connected ... Networks available"};
    } else if (n.ethernetAdapter) {
        *network = {"ethernet-unplugged", L"Network cable unplugged"};
    } else {
        *network = {"wifi-disconnected", L"Not connected"};
    }

    *vpn = n.vpnConnected ? IconState{"vpn-on", L"VPN ... " + n.vpnName}
                          : IconState{"vpn", L"VPN ... Not connected"};
}

IconState ComputeDeviceBatteries();

IconState ComputeBattery() {
    SYSTEM_POWER_STATUS status{};
    if (!GetSystemPowerStatus(&status) || (status.BatteryFlag & 128) ||
        status.BatteryLifePercent > 100) {
        return ComputeDeviceBatteries();
    }

    int percent = status.BatteryLifePercent;
    bool charging = status.BatteryFlag & 8;
    bool pluggedIn = status.ACLineStatus == 1;
    bool saver = status.SystemStatusFlag == 1;

    std::wstring detail;
    if (charging) {
        detail = L"Charging";
    } else if (pluggedIn) {
        detail = percent >= 100 ? L"Fully charged" : L"Plugged in";
    } else if (status.BatteryLifeTime != (DWORD)-1) {
        DWORD minutes = status.BatteryLifeTime / 60;
        WCHAR text[64];
        swprintf_s(text, L"%u h %02u m left", minutes / 60, minutes % 60);
        detail = text;
    } else {
        detail = L"On battery";
    }
    if (saver) {
        detail += L" ... Battery saver ON";
    }

    std::string art = charging        ? "battery-charging"
                      : saver         ? "battery-saver"
                      : percent >= 95 ? "battery-full"
                      : percent >= 60 ? "battery-high"
                      : percent >= 20 ? "battery-mid"
                                      : "battery-low";
    return {art, std::to_wstring(percent) + L"% ... " + detail};
}

int CountNotifications();

IconState ComputeNotifications() {
    DWORD toasts = 1;
    bool dnd = ReadDword(HKEY_CURRENT_USER, kNotificationsPath,
                         L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", &toasts) &&
               !toasts;
    if (dnd) {
        return {"bell-dnd", L"Do not disturb ... ON"};
    }
    int count = CountNotifications();
    return count ? IconState{"bell-unread",
                             std::to_wstring(count) + (count == 1 ? L" notification" : L" notifications")}
                 : IconState{"bell", L"No new notifications"};
}

IconState ComputeNightLight() {
    BYTE data[256];
    DWORD size = sizeof(data);
    bool on = RegGetValueW(HKEY_CURRENT_USER, kNightLightPath, L"Data",
                           RRF_RT_REG_BINARY, nullptr, data,
                           &size) == ERROR_SUCCESS &&
              size > 18 && data[18] == 0x15;
    return on ? IconState{"night-light", L"Night light ... ON"}
              : IconState{"night-light-off", L"Night light ... OFF"};
}

// A capability is in use by an app while its LastUsedTimeStop is 0.
std::vector<std::wstring> AppsUsing(PCWSTR capability) {
    std::vector<std::wstring> apps;
    std::wstring base = std::wstring(kConsentStorePath) + capability;

    auto scan = [&](std::wstring const& path, bool nonPackaged) {
        HKEY key;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &key) !=
            ERROR_SUCCESS) {
            return;
        }
        WCHAR name[512];
        for (DWORD i = 0;; i++) {
            DWORD length = ARRAYSIZE(name);
            if (RegEnumKeyExW(key, i, name, &length, nullptr, nullptr, nullptr,
                              nullptr) != ERROR_SUCCESS) {
                break;
            }
            HKEY app;
            if (RegOpenKeyExW(key, name, 0, KEY_READ, &app) != ERROR_SUCCESS) {
                continue;
            }
            ULONGLONG start = 0, stop = 1;
            if (ReadQword(app, L"LastUsedTimeStart", &start) &&
                ReadQword(app, L"LastUsedTimeStop", &stop) && start && !stop) {
                std::wstring label = name;
                if (nonPackaged) {
                    // C:#Program Files#App#App.exe
                    label = label.substr(label.find_last_of(L'#') + 1);
                    if (label.size() > 4 &&
                        _wcsicmp(label.c_str() + label.size() - 4, L".exe") == 0) {
                        label.resize(label.size() - 4);
                    }
                } else {
                    // Publisher.App_hash
                    label = label.substr(0, label.find(L'_'));
                    label = label.substr(label.find_last_of(L'.') + 1);
                }
                apps.push_back(label);
            }
            RegCloseKey(app);
        }
        RegCloseKey(key);
    };

    scan(base, false);
    scan(base + L"\\NonPackaged", true);
    return apps;
}

IconState ComputePrivacy(PCWSTR capability, const char* art, PCWSTR label) {
    auto apps = AppsUsing(capability);
    if (apps.empty()) {
        return {art, std::wstring(label) + L" in use", true};
    }
    std::wstring tip = std::wstring(label) + L" in use ... ";
    for (size_t i = 0; i < apps.size(); i++) {
        tip += (i ? L", " : L"") + apps[i];
    }
    return {art, tip};
}

IconState ComputeInput() {
    HWND foreground = GetForegroundWindow();
    HKL layout = GetKeyboardLayout(
        foreground ? GetWindowThreadProcessId(foreground, nullptr) : 0);
    WCHAR locale[LOCALE_NAME_MAX_LENGTH] = L"";
    LCIDToLocaleName(MAKELCID(LOWORD((ULONG_PTR)layout), SORT_DEFAULT), locale,
                     ARRAYSIZE(locale), 0);
    WCHAR language[16] = L"";
    WCHAR country[16] = L"";
    GetLocaleInfoEx(locale, LOCALE_SISO639LANGNAME2, language, ARRAYSIZE(language));
    GetLocaleInfoEx(locale, LOCALE_SISO3166CTRYNAME, country, ARRAYSIZE(country));
    std::wstring text = std::wstring(language) + L" " + country;
    CharUpperBuffW(text.data(), (DWORD)text.size());
    return {"keyboard", L"Input ... " + text};
}

IconState ComputeRecycleBin() {
    SHQUERYRBINFO info{};
    info.cbSize = sizeof(info);
    if (FAILED(SHQueryRecycleBinW(nullptr, &info)) || !info.i64NumItems) {
        return {"app-bin", L"Recycle Bin ... Empty"};
    }
    return {"app-bin", L"Recycle Bin ... " + std::to_wstring(info.i64NumItems) +
                           (info.i64NumItems == 1 ? L" item" : L" items")};
}

////////////////////////////////////////////////////////////////////////////////
// Audio (mod thread; the callbacks only request refreshes)

IMMDeviceEnumerator* g_audioEnumerator;
IAudioEndpointVolume* g_endpointVolume;
std::wstring g_audioDeviceName;

struct VolumeCallback final : IAudioEndpointVolumeCallback {
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, kIidAudioEndpointVolumeCallback)) {
            *out = static_cast<IAudioEndpointVolumeCallback*>(this);
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    // Lives for the whole mod lifetime, so reference counting is a no-op.
    ULONG STDMETHODCALLTYPE AddRef() override { return 2; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA) override {
        RequestRefresh(1u << kVolume);
        return S_OK;
    }
};

struct DeviceCallback final : IMMNotificationClient {
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, kIidMMNotificationClient)) {
            *out = static_cast<IMMNotificationClient*>(this);
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return 2; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    void Changed() {
        if (HWND wnd = g_wnd) {
            PostMessageW(wnd, WM_APP_AUDIO_DEVICE, 0, 0);
        }
    }
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) override {
        Changed();
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) override { Changed(); return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) override { Changed(); return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) override {
        if (flow == eRender && role == eConsole) {
            Changed();
        }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override {
        return S_OK;
    }
};

VolumeCallback g_volumeCallback;
DeviceCallback g_deviceCallback;
bool g_deviceCallbackRegistered;

std::wstring DeviceName(IMMDevice* device) {
    std::wstring name;
    IPropertyStore* store = nullptr;
    if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &store))) {
        PROPVARIANT value;
        PropVariantInit(&value);
        if (SUCCEEDED(store->GetValue(kPkeyDeviceFriendlyName, &value)) &&
            value.vt == VT_LPWSTR && value.pwszVal) {
            name = value.pwszVal;
        }
        PropVariantClear(&value);
        store->Release();
    }
    return name;
}

// Follows the default output device, re-registering for its volume changes.
void BindAudioEndpoint() {
    if (g_endpointVolume) {
        g_endpointVolume->UnregisterControlChangeNotify(&g_volumeCallback);
        g_endpointVolume->Release();
        g_endpointVolume = nullptr;
    }
    g_audioDeviceName.clear();
    if (!g_audioEnumerator) {
        return;
    }
    IMMDevice* device = nullptr;
    if (FAILED(g_audioEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        return;
    }
    g_audioDeviceName = DeviceName(device);
    if (SUCCEEDED(device->Activate(kIidAudioEndpointVolume, CLSCTX_ALL, nullptr,
                                   (void**)&g_endpointVolume))) {
        g_endpointVolume->RegisterControlChangeNotify(&g_volumeCallback);
    }
    device->Release();
}

void StartAudio() {
    if (FAILED(CoCreateInstance(kClsidMMDeviceEnumerator, nullptr, CLSCTX_ALL,
                                kIidMMDeviceEnumerator, (void**)&g_audioEnumerator))) {
        g_audioEnumerator = nullptr;
        return;
    }
    g_deviceCallbackRegistered = SUCCEEDED(
        g_audioEnumerator->RegisterEndpointNotificationCallback(&g_deviceCallback));
    BindAudioEndpoint();
}

void StopAudio() {
    if (g_endpointVolume) {
        g_endpointVolume->UnregisterControlChangeNotify(&g_volumeCallback);
        g_endpointVolume->Release();
        g_endpointVolume = nullptr;
    }
    if (g_audioEnumerator) {
        if (g_deviceCallbackRegistered) {
            g_audioEnumerator->UnregisterEndpointNotificationCallback(&g_deviceCallback);
            g_deviceCallbackRegistered = false;
        }
        g_audioEnumerator->Release();
        g_audioEnumerator = nullptr;
    }
}

IconState ComputeVolume() {
    float level = 0;
    BOOL muted = FALSE;
    if (!g_endpointVolume || FAILED(g_endpointVolume->GetMasterVolumeLevelScalar(&level))) {
        return {"audio-no-device", L"No audio output device"};
    }
    g_endpointVolume->GetMute(&muted);
    int percent = (int)(level * 100 + 0.5f);
    std::wstring name = g_audioDeviceName.empty() ? L"Speakers" : g_audioDeviceName;
    if (muted) {
        return {"audio-mute", name + L" ... Muted"};
    }
    std::string art = percent == 0  ? "audio-0"
                      : percent < 34 ? "audio-low"
                      : percent < 67 ? "audio-mid"
                                     : "audio-high";
    return {art, name + L" ... " + std::to_wstring(percent) + L"%"};
}

void ChangeVolume(int notches) {
    if (!g_endpointVolume || !notches) {
        return;
    }
    float level = 0;
    if (FAILED(g_endpointVolume->GetMasterVolumeLevelScalar(&level))) {
        return;
    }
    int percent = std::clamp((int)(level * 100 + 0.5f) + notches * kVolumeStep, 0, 100);
    g_endpointVolume->SetMasterVolumeLevelScalar(percent / 100.0f, nullptr);
    if (notches > 0) {
        // Like the volume slider: turning it up unmutes.
        g_endpointVolume->SetMute(FALSE, nullptr);
    }
}

// Makes the next active output device the default for every role.
void CycleOutputDevice() {
    if (!g_audioEnumerator) {
        return;
    }
    IMMDeviceCollection* devices = nullptr;
    if (FAILED(g_audioEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices))) {
        return;
    }
    std::vector<std::wstring> ids;
    UINT count = 0;
    devices->GetCount(&count);
    for (UINT i = 0; i < count; i++) {
        IMMDevice* device = nullptr;
        LPWSTR id = nullptr;
        if (SUCCEEDED(devices->Item(i, &device)) && SUCCEEDED(device->GetId(&id))) {
            ids.push_back(id);
            CoTaskMemFree(id);
        }
        if (device) {
            device->Release();
        }
    }
    devices->Release();
    if (ids.size() < 2) {
        return;
    }

    std::wstring current;
    IMMDevice* device = nullptr;
    if (SUCCEEDED(g_audioEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        LPWSTR id = nullptr;
        if (SUCCEEDED(device->GetId(&id))) {
            current = id;
            CoTaskMemFree(id);
        }
        device->Release();
    }
    auto it = std::find(ids.begin(), ids.end(), current);
    size_t next = it == ids.end() ? 0 : (size_t)(it - ids.begin() + 1) % ids.size();

    IPolicyConfig* policy = nullptr;
    if (SUCCEEDED(CoCreateInstance(kClsidPolicyConfigClient, nullptr, CLSCTX_ALL,
                                   kIidPolicyConfig, (void**)&policy))) {
        for (ERole role : {eConsole, eMultimedia, eCommunications}) {
            policy->SetDefaultEndpoint(ids[next].c_str(), role);
        }
        policy->Release();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Show hidden icons: a stand-in for the "^" button, with a flyout of its own.
// StartAllBack removes the tray's own popup along with its "^" button, so the
// flyout lists the hidden icons itself.

bool g_overflowOpen;  // The hidden icons flyout is showing.

IconState ComputeOverflow() {
    return g_overflowOpen ? IconState{"overflow-down", L"Hide icons"}
                          : IconState{"overflow-up", L"Show hidden icons"};
}

// Bluetooth devices' batteries, for desktops without one of their own: the
// battery icon shows the lowest connected device and lists them all.
struct DeviceBattery {
    std::wstring name;
    int percent = 0;
};

std::vector<DeviceBattery> BluetoothBatteries() {
    static const DEVPROPKEY kConnected = {
        {0x83DA6326, 0x97A6, 0x4088, {0x94, 0x53, 0xA1, 0x92, 0x3F, 0x57, 0x3B, 0x29}}, 15};
    static const DEVPROPKEY kBattery = {
        {0x104EA319, 0x6EE2, 0x4701, {0xBD, 0x47, 0x8D, 0xDB, 0xF4, 0x25, 0xBB, 0xE5}}, 2};
    static const DEVPROPKEY kFriendlyName = {
        {0xA45C254E, 0xDF1C, 0x4EFD, {0x80, 0x20, 0x67, 0xD1, 0x46, 0xA8, 0x50, 0xE0}}, 14};

    std::vector<DeviceBattery> devices;
    for (PCWSTR enumerator : {L"BTHLE", L"BTHENUM"}) {
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
            // The devices themselves ("BTHLE\Dev_...", in any case), not their
            // services.
            PCWSTR slash = wcschr(id, L'\\');
            if (!slash || _wcsnicmp(slash + 1, L"DEV_", 4) != 0) {
                continue;
            }
            DEVINST node;
            if (CM_Locate_DevNodeW(&node, (DEVINSTID_W)id, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) {
                continue;
            }
            DEVPROPTYPE type = 0;
            DEVPROP_BOOLEAN connected = DEVPROP_FALSE;
            ULONG length = sizeof(connected);
            if (CM_Get_DevNode_PropertyW(node, &kConnected, &type, (PBYTE)&connected, &length, 0) !=
                    CR_SUCCESS ||
                type != DEVPROP_TYPE_BOOLEAN || connected != DEVPROP_TRUE) {
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
            CM_Get_DevNode_PropertyW(node, &kFriendlyName, &type, (PBYTE)name, &length, 0);
            devices.push_back({*name ? name : L"Bluetooth device", battery});
        }
    }
    return devices;
}

IconState ComputeDeviceBatteries() {
    auto devices = BluetoothBatteries();
    if (devices.empty()) {
        return {"battery-none", L"No battery detected"};
    }
    int lowest = 100;
    std::wstring tip;
    for (auto const& device : devices) {
        lowest = std::min(lowest, device.percent);
        if (!tip.empty()) {
            tip += L"\n";
        }
        tip += device.name + L" ... " + std::to_wstring(device.percent) + L"%";
    }
    std::string art = lowest >= 95   ? "battery-full"
                      : lowest >= 60 ? "battery-high"
                      : lowest >= 20 ? "battery-mid"
                                     : "battery-low";
    return {art, tip};
}

// Windows Security, from the Security Center's own health summary.
WSC_SECURITY_PROVIDER_HEALTH ProviderHealth(DWORD provider) {
    WSC_SECURITY_PROVIDER_HEALTH health = WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED;
    if (FAILED(WscGetSecurityProviderHealth(provider, &health))) {
        return WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED;
    }
    return health;
}

IconState ComputeSecurity() {
    auto antivirus = ProviderHealth(WSC_SECURITY_PROVIDER_ANTIVIRUS);
    auto firewall = ProviderHealth(WSC_SECURITY_PROVIDER_FIREWALL);
    auto updates = ProviderHealth(WSC_SECURITY_PROVIDER_AUTOUPDATE_SETTINGS);
    if (antivirus == WSC_SECURITY_PROVIDER_HEALTH_POOR) {
        return {"security-alert", L"Windows Security ... Action needed"};
    }
    if (antivirus == WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED) {
        return {"security-off", L"Real-time protection ... OFF"};
    }
    if (firewall == WSC_SECURITY_PROVIDER_HEALTH_POOR || updates == WSC_SECURITY_PROVIDER_HEALTH_POOR ||
        antivirus == WSC_SECURITY_PROVIDER_HEALTH_SNOOZE ||
        firewall == WSC_SECURITY_PROVIDER_HEALTH_SNOOZE) {
        return {"security-warn", L"Windows Security ... Action recommended"};
    }
    return {"security-ok", L"Windows Security ... No actions needed"};
}

HANDLE g_securityWatch;

DWORD WINAPI OnSecurityChange(LPVOID) {
    RequestRefresh(1u << kSecurity);
    return 0;
}

IconState ComputeIcon(int kind) {
    switch (kind) {
        case kBattery: return ComputeBattery();
        case kNotifications: return ComputeNotifications();
        case kFocus: return {"focus", L"Focus"};
        case kNightLight: return ComputeNightLight();
        case kBrightness: return {"brightness", L"Brightness"};
        case kMicrophone: return ComputePrivacy(L"microphone", "mic-in-use", L"Microphone");
        case kCamera: return ComputePrivacy(L"webcam", "camera-in-use", L"Camera");
        case kLocation: return ComputePrivacy(L"location", "location-in-use", L"Location");
        case kAccessibility: return {"accessibility", L"Accessibility"};
        case kPower: return {"power", L"Power ... Sleep, shut down, restart"};
        case kInput: return ComputeInput();
        case kRecycleBin: return ComputeRecycleBin();
        case kClipboard: return {"app-clipboard", L"Clipboard history"};
        case kCapture: return {"app-capture", L"Screen capture"};
        case kVolume: return ComputeVolume();
        case kOverflow: return ComputeOverflow();
        case kSecurity: return ComputeSecurity();
    }
    return {};
}

////////////////////////////////////////////////////////////////////////////////
// Tray icons (mod thread)

struct AppliedIcon {
    bool added = false;
    IconState state;
    int size = 0;
    std::string tone;
    bool shade = true;
};

std::array<AppliedIcon, kIconCount> g_applied;
std::array<IconState, kIconCount> g_states;

int TrayIconSize() {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    UINT dpi = tray ? GetDpiForWindow(tray) : 0;
    return GetSystemMetricsForDpi(SM_CXSMICON, dpi ? dpi : USER_DEFAULT_SCREEN_DPI);
}

void FillIconData(NOTIFYICONDATAW* data, int kind) {
    *data = {};
    data->cbSize = sizeof(*data);
    data->hWnd = g_wnd;
    data->uID = kIconIdBase + kind;
}

void RemoveIcon(int kind) {
    if (!g_applied[kind].added) {
        return;
    }
    NOTIFYICONDATAW data;
    FillIconData(&data, kind);
    Shell_NotifyIconW(NIM_DELETE, &data);
    g_applied[kind] = {};
}

void ApplyIcon(int kind, int size) {
    IconState const& state = g_states[kind];
    AppliedIcon& applied = g_applied[kind];
    if (applied.added && applied.state.art == state.art &&
        applied.state.tip == state.tip &&
        applied.state.hidden == state.hidden && applied.size == size &&
        applied.tone == g_settings.tone && applied.shade == g_settings.shade) {
        return;
    }

    HICON icon = CreateIconFromPixels(
        RenderArt(ArtFor(state.art), g_settings.tone, g_settings.shade, size),
        size);

    NOTIFYICONDATAW data;
    FillIconData(&data, kind);
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_STATE;
    data.uCallbackMessage = WM_APP_TRAY;
    data.hIcon = icon;
    wcsncpy_s(data.szTip, state.tip.c_str(), _TRUNCATE);
    data.dwStateMask = NIS_HIDDEN;
    data.dwState = state.hidden ? NIS_HIDDEN : 0;

    bool ok;
    if (!applied.added) {
        ok = Shell_NotifyIconW(NIM_ADD, &data);
        if (ok) {
            data.uVersion = NOTIFYICON_VERSION_4;
            Shell_NotifyIconW(NIM_SETVERSION, &data);
        }
    } else {
        ok = Shell_NotifyIconW(NIM_MODIFY, &data);
    }
    if (icon) {
        DestroyIcon(icon);
    }

    if (ok) {
        applied = {true, state, size, g_settings.tone, g_settings.shade};
    } else {
        // Most likely the tray isn't up yet; TaskbarCreated and the periodic
        // timer retry.
        applied.added = false;
    }
}

void ReloadFlyout();
bool IsFlyoutOpen(int flyoutKind);

void Refresh(UINT bits) {
    int size = TrayIconSize();
    if (bits & kNetworkBits) {
        if (g_settings.enabled[kNetwork] || g_settings.enabled[kVpn]) {
            ComputeNetwork(&g_states[kNetwork], &g_states[kVpn]);
        }
    }
    for (int k = 0; k < kIconCount; k++) {
        if (!g_settings.enabled[k]) {
            RemoveIcon(k);
            continue;
        }
        if ((bits & (1u << k)) && !(kNetworkBits & (1u << k))) {
            g_states[k] = ComputeIcon(k);
        }
        if (!g_states[k].art.empty()) {
            ApplyIcon(k, size);
        }
    }
    if (bits & (kNetworkBits | (1u << kBattery) | (1u << kSecurity) | (1u << kVolume) |
                (1u << kNotifications))) {
        ReloadFlyout();
    }
}

////////////////////////////////////////////////////////////////////////////////
// The tray's toolbars. Each button's data holds the owner window at +0, the
// id at +8, the callback message at +12 and the NOTIFYICON version at +20.

std::vector<HWND> TrayToolbars() {
    std::vector<HWND> toolbars;
    if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr)) {
        if (HWND notify = FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr)) {
            EnumChildWindows(
                notify,
                [](HWND hWnd, LPARAM lParam) -> BOOL {
                    WCHAR className[32];
                    if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
                        _wcsicmp(className, TOOLBARCLASSNAMEW) == 0) {
                        ((std::vector<HWND>*)lParam)->push_back(hWnd);
                    }
                    return TRUE;
                },
                (LPARAM)&toolbars);
        }
    }
    if (HWND overflow = FindWindowW(L"NotifyIconOverflowWindow", nullptr)) {
        if (HWND toolbar = FindWindowExW(overflow, nullptr, TOOLBARCLASSNAMEW, nullptr)) {
            toolbars.push_back(toolbar);
        }
    }
    return toolbars;
}

struct TrayTarget {
    HWND hwnd = nullptr;
    UINT id = 0;
    UINT callback = 0;
    UINT version = 0;
};

using TargetMatch = std::function<bool(HWND owner, UINT id, PCWSTR className)>;

bool FindTrayTarget(TargetMatch const& match, TrayTarget* target) {
    for (HWND toolbar : TrayToolbars()) {
        int count = (int)SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            TBBUTTON button{};
            if (!SendMessageW(toolbar, TB_GETBUTTON, i, (LPARAM)&button) ||
                !button.dwData) {
                continue;
            }
            auto* data = (const BYTE*)button.dwData;
            HWND owner = *(const HWND*)data;
            UINT ownerId = *(const UINT*)(data + 8);
            WCHAR className[64];
            if (!IsWindow(owner) ||
                !GetClassNameW(owner, className, ARRAYSIZE(className)) ||
                !match(owner, ownerId, className)) {
                continue;
            }
            *target = {owner, ownerId, *(const UINT*)(data + 12),
                       *(const UINT*)(data + 20)};
            return true;
        }
    }
    return false;
}

bool OwnedByThisProcess(HWND hWnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    return pid == GetCurrentProcessId();
}

std::wstring OwnerFileName(HWND owner);

// The system icon each stand-in's right click goes to.
bool ForwardTargetFor(int kind, TrayTarget* target) {
    switch (kind) {
        case kNetwork:
            return FindTrayTarget(
                [](HWND, UINT, PCWSTR cls) {
                    return _wcsicmp(cls, L"SIBNetworkIcon") == 0 ||
                           _wcsicmp(cls, L"NetworkIconWindow") == 0;
                },
                target);
        case kBattery:
            return FindTrayTarget(
                [](HWND, UINT id, PCWSTR cls) {
                    return id == 1226 && _wcsicmp(cls, L"SystemTray_Main") == 0;
                },
                target);
        case kVolume:
            // Explorer's own volume icon: an ATL window with id 100. Windows
            // Security has the same shape, but lives in its own process.
            return FindTrayTarget(
                [](HWND owner, UINT id, PCWSTR cls) {
                    return id == 100 && wcsncmp(cls, L"ATL:", 4) == 0 &&
                           OwnedByThisProcess(owner);
                },
                target);
    }
    return false;
}

// Replays a tray event on another icon, in that icon's NOTIFYICON version.
void ForwardEvent(TrayTarget const& target, UINT event, WPARAM anchor) {
    if (target.version >= NOTIFYICON_VERSION_4) {
        PostMessageW(target.hwnd, target.callback, anchor,
                     MAKELPARAM(event, target.id));
        return;
    }
    bool mouse = event == WM_LBUTTONDOWN || event == WM_LBUTTONUP ||
                 event == WM_RBUTTONDOWN || event == WM_RBUTTONUP ||
                 event == WM_LBUTTONDBLCLK || event == WM_MOUSEMOVE;
    bool v3 = target.version == NOTIFYICON_VERSION &&
              (event == NIN_SELECT || event == NIN_KEYSELECT ||
               event == WM_CONTEXTMENU);
    if (mouse || v3) {
        PostMessageW(target.hwnd, target.callback, target.id, event);
    }
}

// Scrolling and middle clicks never reach a tray icon's callback reliably,
// so the toolbars are watched for them over the volume icon.
int KindUnderPoint(HWND toolbar, POINT clientPoint) {
    int index = (int)SendMessageW(toolbar, TB_HITTEST, 0, (LPARAM)&clientPoint);
    if (index < 0) {
        return -1;
    }
    TBBUTTON button{};
    if (!SendMessageW(toolbar, TB_GETBUTTON, index, (LPARAM)&button) ||
        !button.dwData) {
        return -1;
    }
    auto* data = (const BYTE*)button.dwData;
    HWND owner = *(const HWND*)data;
    UINT id = *(const UINT*)(data + 8);
    if (!owner || owner != g_wnd || id < kIconIdBase || id >= kIconIdBase + kIconCount) {
        return -1;
    }
    return (int)(id - kIconIdBase);
}

LRESULT CALLBACK ToolbarSubclassProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     DWORD_PTR) {
    if (uMsg == WM_MOUSEWHEEL || uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP) {
        POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (uMsg == WM_MOUSEWHEEL) {
            ScreenToClient(hWnd, &pt);
        }
        if (KindUnderPoint(hWnd, pt) == kVolume) {
            if (HWND wnd = g_wnd) {
                if (uMsg == WM_MOUSEWHEEL) {
                    PostMessageW(wnd, WM_APP_WHEEL, (WPARAM)GET_WHEEL_DELTA_WPARAM(wParam), 0);
                } else if (uMsg == WM_MBUTTONUP) {
                    PostMessageW(wnd, WM_APP_MIDDLE, 0, 0);
                }
            }
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

std::vector<HWND> g_hookedToolbars;  // Mod thread only.

void HookToolbars() {
    std::erase_if(g_hookedToolbars, [](HWND hWnd) { return !IsWindow(hWnd); });
    for (HWND toolbar : TrayToolbars()) {
        if (std::find(g_hookedToolbars.begin(), g_hookedToolbars.end(), toolbar) !=
            g_hookedToolbars.end()) {
            continue;
        }
        if (WindhawkUtils::SetWindowSubclassFromAnyThread(toolbar, ToolbarSubclassProc, 0)) {
            g_hookedToolbars.push_back(toolbar);
        }
    }
}

void UnhookToolbars() {
    for (HWND toolbar : g_hookedToolbars) {
        if (IsWindow(toolbar)) {
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(toolbar, ToolbarSubclassProc);
        }
    }
    g_hookedToolbars.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Actions

void OpenUri(PCWSTR uri) {
    ShellExecuteW(nullptr, L"open", uri, nullptr, nullptr, SW_SHOWNORMAL);
}

void Run(PCWSTR file, PCWSTR parameters = nullptr) {
    ShellExecuteW(nullptr, L"open", file, parameters, nullptr, SW_SHOWNORMAL);
}

void SendHotkey(std::initializer_list<WORD> keys) {
    std::vector<INPUT> inputs;
    for (WORD key : keys) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        inputs.push_back(input);
    }
    for (auto it = std::rbegin(keys); it != std::rend(keys); ++it) {
        INPUT input{};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = *it;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(input);
    }
    SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
}

bool EnableShutdownPrivilege() {
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }
    TOKEN_PRIVILEGES privileges{};
    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    bool ok = LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME,
                                    &privileges.Privileges[0].Luid) &&
              AdjustTokenPrivileges(token, FALSE, &privileges, 0, nullptr,
                                    nullptr);
    CloseHandle(token);
    return ok;
}

void RequestExitWindows(UINT flags) {
    EnableShutdownPrivilege();
    ExitWindowsEx(flags, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
}

struct MenuItem {
    PCWSTR label;
    std::function<void()> action;
};

void ShowMenu(std::vector<MenuItem> const& items, WPARAM anchor) {
    HMENU menu = CreatePopupMenu();
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].label) {
            AppendMenuW(menu, MF_STRING, i + 1, items[i].label);
        } else {
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        }
    }

    POINT pt;
    GetCursorPos(&pt);
    if (anchor) {
        pt = {GET_X_LPARAM(anchor), GET_Y_LPARAM(anchor)};
    }

    // Required for the menu to close when clicking elsewhere.
    HWND wnd = g_wnd;
    SetForegroundWindow(wnd);
    UINT chosen = TrackPopupMenuEx(
        menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y,
        wnd, nullptr);
    PostMessageW(wnd, WM_NULL, 0, 0);
    DestroyMenu(menu);

    if (chosen && chosen <= items.size() && items[chosen - 1].action) {
        items[chosen - 1].action();
    }
}

std::vector<MenuItem> PowerMenu() {
    return {
        {L"Lock", [] { LockWorkStation(); }},
        {L"Sign out", [] { RequestExitWindows(EWX_LOGOFF); }},
        {nullptr, nullptr},
        {L"Sleep", [] { SetSuspendState(FALSE, FALSE, FALSE); }},
        {L"Restart", [] { RequestExitWindows(EWX_REBOOT); }},
        {L"Shut down", [] { RequestExitWindows(EWX_SHUTDOWN | EWX_POWEROFF); }},
    };
}

auto Uri(PCWSTR uri) {
    return [uri] { OpenUri(uri); };
}

////////////////////////////////////////////////////////////////////////////////
// Hidden icons. The tray keeps every icon as a button of its toolbar, the ones
// the user hid included (marked hidden), and only Explorer can read them. The
// functions marked "In Explorer" run there, and pack the hidden icons with
// their pictures and tooltips for the flyout.

// Only the split suite, where the icons cross processes, uses it.
[[maybe_unused]] constexpr ULONG_PTR kHiddenIconsCopyId = 0x54484931;  // "THI1"

struct HiddenIcon {
    HWND owner = nullptr;
    UINT id = 0;
    UINT callback = 0;
    UINT version = 0;
    std::wstring tip;
    int size = 0;
    std::vector<uint32_t> pixels;  // Premultiplied BGRA, size x size.
};

std::wstring CollapseSpaces(std::wstring_view text) {
    std::wstring out;
    for (WCHAR ch : text) {
        bool space = ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n';
        if (space) {
            if (!out.empty() && out.back() != L' ') {
                out += L' ';
            }
        } else {
            out += ch;
        }
    }
    while (!out.empty() && out.back() == L' ') {
        out.pop_back();
    }
    return out;
}

std::wstring LowerFileName(std::wstring_view path) {
    size_t slash = path.find_last_of(L"\\/");
    std::wstring name(slash == std::wstring_view::npos ? path : path.substr(slash + 1));
    CharLowerBuffW(name.data(), (DWORD)name.size());
    return name;
}

// In Explorer: tooltips as programs send them. Shell_NotifyIcon reaches the
// tray as WM_COPYDATA to Shell_TrayWnd: a signature, the NIM_* message, then a
// NOTIFYICONDATA with 32-bit handles, whose tooltip is at offset 24.
constexpr ULONG_PTR kTrayCopyDataId = 1;
constexpr DWORD kTraySignature = 0x34753423;
constexpr size_t kTrayHeaderSize = 8;
constexpr size_t kTrayTipOffset = kTrayHeaderSize + 24;
constexpr size_t kTrayTipChars = 128;

SRWLOCK g_tipsLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::unordered_map<uint64_t, std::wstring> g_tips;
std::atomic<HWND> g_tipsTrayWnd;

uint64_t TipKey(HWND hwnd, UINT id) {
    // The tray only ever sees 32-bit window handles.
    return ((uint64_t)(uint32_t)(ULONG_PTR)hwnd << 32) | id;
}

void RecordTrayCopyData(const COPYDATASTRUCT* copyData) {
    if (!copyData || copyData->dwData != kTrayCopyDataId || !copyData->lpData ||
        copyData->cbData < kTrayTipOffset) {
        return;
    }
    const BYTE* data = (const BYTE*)copyData->lpData;
    DWORD signature, message, hwnd32;
    UINT id, flags;
    memcpy(&signature, data, 4);
    memcpy(&message, data + 4, 4);
    memcpy(&hwnd32, data + kTrayHeaderSize + 4, 4);
    memcpy(&id, data + kTrayHeaderSize + 8, 4);
    memcpy(&flags, data + kTrayHeaderSize + 12, 4);
    if (signature != kTraySignature) {
        return;
    }
    uint64_t key = TipKey((HWND)(ULONG_PTR)hwnd32, id);
    if (message == NIM_DELETE) {
        AcquireSRWLockExclusive(&g_tipsLock);
        g_tips.erase(key);
        ReleaseSRWLockExclusive(&g_tipsLock);
        return;
    }
    if ((message != NIM_ADD && message != NIM_MODIFY) || !(flags & NIF_TIP) ||
        copyData->cbData < kTrayTipOffset + kTrayTipChars * sizeof(WCHAR)) {
        return;
    }
    WCHAR tip[kTrayTipChars + 1];
    memcpy(tip, data + kTrayTipOffset, kTrayTipChars * sizeof(WCHAR));
    tip[kTrayTipChars] = L'\0';
    AcquireSRWLockExclusive(&g_tipsLock);
    g_tips[key] = CollapseSpaces(tip);
    ReleaseSRWLockExclusive(&g_tipsLock);
}

// In Explorer: other programs' tray icons in the Tourne design. Their icons
// are swapped as they reach the tray, so the taskbar and the hidden icons
// flyout both show the design, and states follow what their tooltips say.

std::wstring OwnerFileName(HWND owner);
std::unordered_map<std::wstring, std::wstring> RegistryTips();

bool TipHas(std::wstring const& tip, std::initializer_list<PCWSTR> words) {
    std::wstring lower = tip;
    CharLowerBuffW(lower.data(), (DWORD)lower.size());
    for (PCWSTR word : words) {
        if (lower.find(word) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

std::string CloudDesign(std::wstring const& tip) {
    if (TipHas(tip, {L"pause", L"error", L"problem", L"sign in", L"signed out", L"not signed"})) {
        return "app-cloud-error";
    }
    if (!TipHas(tip, {L"up to date"}) &&
        TipHas(tip, {L"sync", L"upload", L"download", L"processing", L"updating"})) {
        return "app-cloud-sync";
    }
    return "app-cloud";
}

// HWiNFO's sensor icons show a reading, which the tooltip carries as
// "<sensor> <value> <unit> Min: ... Max: ..."; its own icon has none.
std::string SensorDesign(std::wstring const& tip) {
    size_t min = tip.find(L"Min:");
    if (min == std::wstring::npos) {
        return "sensor";
    }
    std::wstring head = tip.substr(0, min);
    double value = 0;
    bool found = false;
    for (size_t i = 0; i < head.size();) {
        if (!iswdigit(head[i])) {
            i++;
            continue;
        }
        size_t end = i;
        while (end < head.size() && (iswdigit(head[end]) || head[end] == L'.' || head[end] == L',')) {
            end++;
        }
        std::wstring number = head.substr(i, end - i);
        std::replace(number.begin(), number.end(), L',', L'.');
        value = _wtof(number.c_str());
        found = true;
        i = end;
    }
    if (!found) {
        return "sensor";
    }
    long reading = std::lround(value);
    bool hot = TipHas(head, {L"°c"}) && reading > 65;
    return "readout:" + std::to_string(reading) + (hot ? ":hot" : "");
}
// Tamer keeps its pause in its settings file only; its icon and tooltip stay
// the same. Read again only when the file changes.
std::wstring TamerSettingsFolder() {
    WCHAR path[MAX_PATH];
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        return {};
    }
    return std::wstring(path) + L"\\app.tamer.windows";
}

SRWLOCK g_tamerLock = SRWLOCK_INIT;

bool TamerPaused() {
    static FILETIME lastWrite{};
    static bool paused = false;
    std::wstring file = TamerSettingsFolder() + L"\\settings.json";
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &info) ||
        info.nFileSizeLow > 256 * 1024) {
        return false;
    }
    AcquireSRWLockExclusive(&g_tamerLock);
    if (CompareFileTime(&info.ftLastWriteTime, &lastWrite) != 0) {
        lastWrite = info.ftLastWriteTime;
        std::string text;
        HANDLE handle = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    nullptr, OPEN_EXISTING, 0, nullptr);
        if (handle != INVALID_HANDLE_VALUE) {
            text.resize(info.nFileSizeLow);
            DWORD read = 0;
            if (!ReadFile(handle, text.data(), (DWORD)text.size(), &read, nullptr)) {
                read = 0;
            }
            text.resize(read);
            CloseHandle(handle);
        }
        // "paused": true, spaced however the file happens to be.
        size_t key = text.find("\"paused\"");
        size_t value = key == std::string::npos ? key : text.find_first_not_of(" \t\r\n:", key + 8);
        paused = value != std::string::npos && text.compare(value, 4, "true") == 0;
    }
    bool result = paused;
    ReleaseSRWLockExclusive(&g_tamerLock);
    return result;
}
// The design that stands for an icon, by program and tooltip; empty for none.
std::string DesignFor(std::wstring const& exe, std::wstring const& windowClass,
                      std::wstring const& tip) {
    auto is = [&](std::initializer_list<PCWSTR> names) {
        for (PCWSTR name : names) {
            if (exe == name) {
                return true;
            }
        }
        return false;
    };
    if (is({L"discord.exe", L"discordptb.exe", L"discordcanary.exe", L"vesktop.exe"})) {
        return "app-chat";
    }
    if (is({L"onedrive.exe", L"icloudhome.exe", L"iclouddrive.exe", L"googledrivefs.exe",
            L"dropbox.exe"})) {
        return CloudDesign(tip);
    }
    if (is({L"steam.exe", L"eadesktop.exe", L"epicgameslauncher.exe", L"riotclientservices.exe",
            L"playnite.desktopapp.exe", L"blitz.exe", L"battle.net.exe", L"galaxyclient.exe",
            L"ubisoftconnect.exe"})) {
        return "app-game";
    }
    if (exe == L"windscribe.exe") {
        return TipHas(tip, {L"connected"}) && !TipHas(tip, {L"disconnected", L"not connected"})
                   ? "vpn-on"
                   : "vpn";
    }
    if (exe == L"securityhealthsystray.exe") {
        if (TipHas(tip, {L"threat", L"action needed", L"attention"})) {
            return "security-alert";
        }
        return TipHas(tip, {L"recommended"}) ? "security-warn" : "security-ok";
    }
    if (exe == L"explorer.exe" && TipHas(tip, {L"safely remove", L"eject"})) {
        return "usb";
    }
    if (exe == L"windhawk-mod.exe" && windowClass == L"BTBatTrayWindow") {
        return "bluetooth";
    }
    if (exe == L"windhawk-mod.exe" && windowClass == L"AudioSwitcherWindowClass") {
        return "app-audio";
    }
    if (exe == L"tamer.exe") {
        return TamerPaused() || TipHas(tip, {L"paused"}) ? "app-screenshot-paused" : "app-screenshot";
    }
    if (is({L"wallpaper64.exe", L"wallpaper32.exe"})) {
        return TipHas(tip, {L"paus"}) ? "app-wallpaper-paused" : "app-wallpaper";
    }
    if (exe == L"claude.exe") {
        return "app-claude";
    }
    if (exe == L"spotify.exe") {
        return "app-spotify";
    }
    if (is({L"alienfan-gui.exe", L"fancontrol.exe"})) {
        return "app-fan";
    }
    if (is({L"sizer.exe", L"glazewm.exe", L"komorebi.exe"})) {
        return "app-tiling";
    }
    if (exe == L"windhawk-mod.exe" && windowClass == L"WCT_TrayWindow_v21") {
        return "app-system";
    }    if (is({L"hwinfo64.exe", L"hwinfo32.exe"})) {
        return SensorDesign(tip);
    }
    if (is({L"nvdisplay.container.exe", L"nvcontainer.exe"}) ||
        (exe.empty() && TipHas(tip, {L"nvidia"}))) {
        return "gpu";
    }    if (exe == L"handy.exe") {
        return "mic";
    }
    if (exe == L"phoneexperiencehost.exe") {
        return "phone";
    }
    if (is({L"musnotifyicon.exe", L"monotificationux.exe"})) {
        return TipHas(tip, {L"restart"}) ? "restart-required" : "update-available";
    }
    return {};
}

std::wstring WindowClassOf(HWND hwnd) {
    WCHAR name[128];
    return GetClassNameW(hwnd, name, ARRAYSIZE(name)) ? name : L"";
}

bool IsStandInWindow(HWND hwnd) {
    return WindowClassOf(hwnd).starts_with(L"TourneTraySystemIcons_");
}

std::string DesignForOwner(HWND owner, std::wstring const& tip) {
    if (!IsWindow(owner) || IsStandInWindow(owner)) {
        return {};
    }
    return DesignFor(OwnerFileName(owner), WindowClassOf(owner), tip);
}

std::atomic<bool> g_themeEnabled;
SRWLOCK g_themeLock = SRWLOCK_INIT;
// Kept for Explorer's lifetime: the tray may hold on to the handle it's given.
[[clang::no_destroy]] std::map<std::string, HICON> g_themeIcons;

HICON ThemeIcon(std::string const& design) {
    int size = TrayIconSize();
    std::string key = design + "|" + std::to_string(size) + "|" + g_settings.tone +
                      (g_settings.shade ? "|shade" : "");
    AcquireSRWLockExclusive(&g_themeLock);
    HICON& icon = g_themeIcons[key];
    if (!icon) {
        icon = CreateIconFromPixels(RenderArt(ArtFor(design), g_settings.tone, g_settings.shade, size),
                                    size);
    }
    HICON result = icon;
    ReleaseSRWLockExclusive(&g_themeLock);
    return result;
}

bool IsThemeIcon(DWORD handle32) {
    AcquireSRWLockShared(&g_themeLock);
    bool found = false;
    for (auto const& [key, icon] : g_themeIcons) {
        if ((DWORD)(ULONG_PTR)icon == handle32) {
            found = true;
            break;
        }
    }
    ReleaseSRWLockShared(&g_themeLock);
    return found;
}

// A Shell_NotifyIcon call rewritten to carry the design's icon; false when
// the icon isn't one the design covers.
bool ThemeTrayCopyData(const COPYDATASTRUCT* copyData, COPYDATASTRUCT* themed,
                       std::vector<BYTE>* buffer) {
    constexpr size_t kFlagsOffset = kTrayHeaderSize + 12;
    constexpr size_t kIconOffset = kTrayHeaderSize + 20;
    if (!g_themeEnabled || !copyData || copyData->dwData != kTrayCopyDataId ||
        !copyData->lpData || copyData->cbData < kTrayTipOffset) {
        return false;
    }
    const BYTE* data = (const BYTE*)copyData->lpData;
    DWORD signature, message, hwnd32, flags, icon32;
    UINT id;
    memcpy(&signature, data, 4);
    memcpy(&message, data + 4, 4);
    memcpy(&hwnd32, data + kTrayHeaderSize + 4, 4);
    memcpy(&id, data + kTrayHeaderSize + 8, 4);
    memcpy(&flags, data + kFlagsOffset, 4);
    memcpy(&icon32, data + kIconOffset, 4);
    if (signature != kTraySignature || (message != NIM_ADD && message != NIM_MODIFY) ||
        ((flags & NIF_ICON) && IsThemeIcon(icon32))) {
        return false;
    }

    HWND owner = (HWND)(ULONG_PTR)hwnd32;
    std::wstring tip;
    if ((flags & NIF_TIP) && copyData->cbData >= kTrayTipOffset + kTrayTipChars * sizeof(WCHAR)) {
        WCHAR text[kTrayTipChars + 1];
        memcpy(text, data + kTrayTipOffset, kTrayTipChars * sizeof(WCHAR));
        text[kTrayTipChars] = L'\0';
        tip = CollapseSpaces(text);
    } else {
        AcquireSRWLockShared(&g_tipsLock);
        auto it = g_tips.find(TipKey(owner, id));
        if (it != g_tips.end()) {
            tip = it->second;
        }
        ReleaseSRWLockShared(&g_tipsLock);
    }
    std::string design = DesignForOwner(owner, tip);
    HICON icon = design.empty() ? nullptr : ThemeIcon(design);
    if (!icon) {
        return false;
    }
    buffer->assign(data, data + copyData->cbData);
    DWORD newFlags = flags | NIF_ICON;
    DWORD newIcon = (DWORD)(ULONG_PTR)icon;
    memcpy(buffer->data() + kFlagsOffset, &newFlags, 4);
    memcpy(buffer->data() + kIconOffset, &newIcon, 4);
    *themed = *copyData;
    themed->lpData = buffer->data();
    return true;
}

struct TrayButtonOwner {
    HWND owner;
    UINT id;
};

std::vector<TrayButtonOwner> TrayButtonOwners() {
    std::vector<TrayButtonOwner> owners;
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    HWND notify = tray ? FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
    HWND toolbar = notify ? FindWindowExW(notify, nullptr, TOOLBARCLASSNAMEW, nullptr) : nullptr;
    int count = toolbar ? (int)SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0) : 0;
    for (int i = 0; i < count; i++) {
        TBBUTTON button{};
        if (SendMessageW(toolbar, TB_GETBUTTON, i, (LPARAM)&button) && button.dwData) {
            auto* data = (const BYTE*)button.dwData;
            owners.push_back({*(const HWND*)data, *(const UINT*)(data + 8)});
        }
    }
    return owners;
}

// In Explorer: icons already in the tray get the design too.
void ThemeExistingIcons() {
    auto registryTips = RegistryTips();
    for (auto const& [owner, id] : TrayButtonOwners()) {
        AcquireSRWLockShared(&g_tipsLock);
        auto live = g_tips.find(TipKey(owner, id));
        std::wstring tip = live != g_tips.end() ? live->second : std::wstring();
        ReleaseSRWLockShared(&g_tipsLock);
        if (tip.empty()) {
            if (auto saved = registryTips.find(OwnerFileName(owner) + L"|" + std::to_wstring(id));
                saved != registryTips.end()) {
                tip = saved->second;
            }
        }
        std::string design = DesignForOwner(owner, tip);
        if (design.empty()) {
            continue;
        }
        NOTIFYICONDATAW data{sizeof(data)};
        data.hWnd = owner;
        data.uID = id;
        data.uFlags = NIF_ICON;
        data.hIcon = ThemeIcon(design);
        Shell_NotifyIconW(NIM_MODIFY, &data);
    }
}

// Programs whose tooltip hasn't come by since capture started, and that
// Windows has none saved for, are asked to add their icons again, as they do
// when Explorer restarts. Their icon is already there, so the tray keeps it,
// but the capture gets the tooltip.
void PokeForTooltips() {
    UINT taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    auto registryTips = RegistryTips();
    std::vector<HWND> poked;
    for (auto const& [owner, id] : TrayButtonOwners()) {
        DWORD pid = 0;
        GetWindowThreadProcessId(owner, &pid);
        if (!IsWindow(owner) || pid == GetCurrentProcessId() || IsStandInWindow(owner) ||
            std::find(poked.begin(), poked.end(), owner) != poked.end()) {
            continue;
        }
        AcquireSRWLockShared(&g_tipsLock);
        bool known = g_tips.contains(TipKey(owner, id));
        ReleaseSRWLockShared(&g_tipsLock);
        if (known || registryTips.contains(OwnerFileName(owner) + L"|" + std::to_wstring(id))) {
            continue;
        }
        PostMessageW(owner, taskbarCreated, 0, 0);
        poked.push_back(owner);
    }
}

// The system icons the stand-ins replace. They stay in the tray, hidden, for
// the stand-ins' right clicks, but they don't belong in the hidden icons
// flyout.
bool IsReplacedByStandIn(HWND owner, UINT id) {
    std::wstring windowClass = WindowClassOf(owner);
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    if (windowClass == L"SIBNetworkIcon" || windowClass == L"NetworkIconWindow") {
        return g_settings.enabled[kNetwork];
    }
    if (windowClass == L"SystemTray_Main" && id == 1226) {
        return g_settings.enabled[kBattery];
    }
    if (id == 100 && windowClass.starts_with(L"ATL:")) {
        if (pid == GetCurrentProcessId()) {
            return g_settings.enabled[kVolume];
        }
        return OwnerFileName(owner) == L"securityhealthsystray.exe" && g_settings.enabled[kSecurity];
    }
    return false;
}
LRESULT CALLBACK TipsTrayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                 DWORD_PTR) {
    if (msg == WM_COPYDATA) {
        auto* copyData = (const COPYDATASTRUCT*)lParam;
        RecordTrayCopyData(copyData);
        COPYDATASTRUCT themed;
        std::vector<BYTE> buffer;
        if (ThemeTrayCopyData(copyData, &themed, &buffer)) {
            return DefSubclassProc(hWnd, msg, wParam, (LPARAM)&themed);
        }
    } else if (msg == WM_NCDESTROY) {
        HWND expected = hWnd;
        g_tipsTrayWnd.compare_exchange_strong(expected, nullptr);
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}

// In Explorer. Safe to call repeatedly; follows the taskbar when Explorer
// recreates it.
void EnsureTipCapture() {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD pid = 0;
    if (!tray || tray == g_tipsTrayWnd || !GetWindowThreadProcessId(tray, &pid) ||
        pid != GetCurrentProcessId()) {
        return;
    }
    if (WindhawkUtils::SetWindowSubclassFromAnyThread(tray, TipsTrayWndProc, 0)) {
        g_tipsTrayWnd = tray;
    }
}

void StopTipCapture() {
    if (HWND tray = g_tipsTrayWnd.exchange(nullptr); tray && IsWindow(tray)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(tray, TipsTrayWndProc);
    }
}

// The first tooltip Windows saw for each icon, by executable name and id: the
// fallback for icons that haven't sent one since capture started.
std::unordered_map<std::wstring, std::wstring> RegistryTips() {
    std::unordered_map<std::wstring, std::wstring> tips;
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\NotifyIconSettings", 0,
                      KEY_READ, &key) != ERROR_SUCCESS) {
        return tips;
    }
    WCHAR subkey[256];
    for (DWORD i = 0;; i++) {
        DWORD length = ARRAYSIZE(subkey);
        LONG result =
            RegEnumKeyExW(key, i, subkey, &length, nullptr, nullptr, nullptr, nullptr);
        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (result != ERROR_SUCCESS) {
            continue;
        }
        WCHAR path[MAX_PATH * 2];
        WCHAR tip[512];
        DWORD id = 0;
        DWORD size = sizeof(path);
        if (RegGetValueW(key, subkey, L"ExecutablePath", RRF_RT_REG_SZ, nullptr, path,
                         &size) != ERROR_SUCCESS) {
            continue;
        }
        size = sizeof(tip);
        if (RegGetValueW(key, subkey, L"InitialTooltip", RRF_RT_REG_SZ, nullptr, tip,
                         &size) != ERROR_SUCCESS ||
            !*tip) {
            continue;
        }
        size = sizeof(id);
        RegGetValueW(key, subkey, L"UID", RRF_RT_REG_DWORD, nullptr, &id, &size);
        tips.try_emplace(LowerFileName(path) + L"|" + std::to_wstring(id),
                         CollapseSpaces(tip));
    }
    RegCloseKey(key);
    return tips;
}

// Any process's image path by id, even one this process may not open (NVIDIA's
// display container is one): SystemProcessIdInformation, as Task Manager uses.
std::wstring ImageNameOfProcessId(DWORD pid) {
    using NtQuerySystemInformation_t = LONG(NTAPI*)(ULONG, PVOID, ULONG, PULONG);
    static auto query = (NtQuerySystemInformation_t)GetProcAddress(
        GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");
    if (!query || !pid) {
        return {};
    }
    WCHAR buffer[MAX_PATH * 2];
    struct {
        HANDLE processId;
        USHORT length;
        USHORT maximumLength;
        PWSTR buffer;
    } info{(HANDLE)(ULONG_PTR)pid, 0, (USHORT)sizeof(buffer), buffer};
    constexpr ULONG kSystemProcessIdInformation = 88;
    if (query(kSystemProcessIdInformation, &info, sizeof(info), nullptr) < 0) {
        return {};
    }
    return std::wstring(buffer, info.length / sizeof(WCHAR));
}
std::wstring OwnerFileName(HWND owner) {
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        return LowerFileName(ImageNameOfProcessId(pid));
    }
    WCHAR path[MAX_PATH * 2];
    DWORD size = ARRAYSIZE(path);
    std::wstring name;
    if (QueryFullProcessImageNameW(process, 0, path, &size)) {
        name = LowerFileName(path);
    }
    CloseHandle(process);
    return name;
}

// The icon as premultiplied pixels. Drawn once on black and once on white, so
// icons without an alpha channel come out right too: the difference between
// the two is how much background shows through.
std::vector<uint32_t> IconPixels(HICON icon, int size) {
    void* black = nullptr;
    void* white = nullptr;
    HBITMAP onBlack = CreateDib(size, size, &black);
    HBITMAP onWhite = CreateDib(size, size, &white);
    std::vector<uint32_t> out;
    if (onBlack && onWhite) {
        memset(black, 0x00, size * size * 4);
        memset(white, 0xFF, size * size * 4);
        HDC dc = CreateCompatibleDC(nullptr);
        HGDIOBJ old = SelectObject(dc, onBlack);
        DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        SelectObject(dc, onWhite);
        DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL);
        SelectObject(dc, old);
        DeleteDC(dc);
        GdiFlush();
        auto* b = (const uint32_t*)black;
        auto* w = (const uint32_t*)white;
        out.resize(size * size);
        for (int i = 0; i < size * size; i++) {
            int seen = 0;
            for (int shift = 0; shift < 24; shift += 8) {
                seen = std::max(seen, (int)((w[i] >> shift) & 0xFF) - (int)((b[i] >> shift) & 0xFF));
            }
            uint32_t alpha = 255 - std::clamp(seen, 0, 255);
            out[i] = alpha ? (alpha << 24) | (b[i] & 0xFFFFFF) : 0;
        }
    }
    if (onBlack) {
        DeleteObject(onBlack);
    }
    if (onWhite) {
        DeleteObject(onWhite);
    }
    return out;
}

// In Explorer: the icons the user hid, in tray order.
std::vector<BYTE> BuildHiddenIconsPacket() {
    std::vector<BYTE> packet;
    auto put = [&](const void* data, size_t size) {
        auto* bytes = (const BYTE*)data;
        packet.insert(packet.end(), bytes, bytes + size);
    };
    UINT count = 0;
    put(&count, 4);

    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    HWND notify = tray ? FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
    HWND toolbar = notify ? FindWindowExW(notify, nullptr, TOOLBARCLASSNAMEW, nullptr) : nullptr;
    if (!toolbar) {
        return packet;
    }

    // The toolbar's own comctl32, so the image list functions match it.
    using GetIcon_t = HICON(WINAPI*)(HIMAGELIST, int, UINT);
    using GetIconSize_t = BOOL(WINAPI*)(HIMAGELIST, int*, int*);
    HMODULE comctl = (HMODULE)GetClassLongPtrW(toolbar, GCLP_HMODULE);
    auto getIcon = comctl ? (GetIcon_t)GetProcAddress(comctl, "ImageList_GetIcon") : nullptr;
    auto getIconSize =
        comctl ? (GetIconSize_t)GetProcAddress(comctl, "ImageList_GetIconSize") : nullptr;
    auto images = (HIMAGELIST)SendMessageW(toolbar, TB_GETIMAGELIST, 0, 0);
    int width = 16, height = 16;
    if (images && getIconSize) {
        getIconSize(images, &width, &height);
    }

    auto registryTips = RegistryTips();
    int buttons = (int)SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < buttons; i++) {
        TBBUTTON button{};
        if (!SendMessageW(toolbar, TB_GETBUTTON, i, (LPARAM)&button) || !button.dwData ||
            !(button.fsState & TBSTATE_HIDDEN)) {
            continue;
        }
        // The button data: owner window at +0, id at +8, callback message at
        // +12, the icon's own NIS_* state at +16, NOTIFYICON version at +20.
        // NIS_HIDDEN there means its program hid it, not the user.
        auto* data = (const BYTE*)button.dwData;
        HWND owner = *(const HWND*)data;
        if (!IsWindow(owner) || (*(const DWORD*)(data + 16) & NIS_HIDDEN)) {
            continue;
        }
        if (IsReplacedByStandIn(owner, *(const UINT*)(data + 8))) {
            continue;
        }
        UINT id = *(const UINT*)(data + 8);
        UINT callback = *(const UINT*)(data + 12);
        UINT version = *(const UINT*)(data + 20);

        AcquireSRWLockShared(&g_tipsLock);
        auto live = g_tips.find(TipKey(owner, id));
        std::wstring tip = live != g_tips.end() ? live->second : std::wstring();
        ReleaseSRWLockShared(&g_tipsLock);
        if (tip.empty()) {
            if (auto saved = registryTips.find(OwnerFileName(owner) + L"|" + std::to_wstring(id));
                saved != registryTips.end()) {
                tip = saved->second;
            }
        }

        if (tip.empty()) {
            // Nothing to go by but the program's name.
            tip = OwnerFileName(owner);
            if (tip.ends_with(L".exe")) {
                tip.resize(tip.size() - 4);
            }
        }
        std::vector<uint32_t> pixels;
        if (images && getIcon) {
            if (HICON icon = getIcon(images, button.iBitmap, ILD_NORMAL)) {
                pixels = IconPixels(icon, width);
                DestroyIcon(icon);
            }
        }

        uint64_t owner64 = (uint64_t)(ULONG_PTR)owner;
        UINT size = pixels.empty() ? 0 : width;
        UINT chars = (UINT)tip.size();
        put(&owner64, 8);
        put(&id, 4);
        put(&callback, 4);
        put(&version, 4);
        put(&size, 4);
        put(&chars, 4);
        put(tip.data(), chars * sizeof(WCHAR));
        put(pixels.data(), pixels.size() * 4);
        count++;
    }
    memcpy(packet.data(), &count, 4);
    return packet;
}

std::vector<HiddenIcon> UnpackHiddenIcons(const BYTE* data, size_t size) {
    std::vector<HiddenIcon> icons;
    size_t at = 0;
    auto take = [&](void* out, size_t length) {
        if (at + length > size) {
            return false;
        }
        memcpy(out, data + at, length);
        at += length;
        return true;
    };
    UINT count = 0;
    if (!take(&count, 4)) {
        return icons;
    }
    for (UINT i = 0; i < count; i++) {
        HiddenIcon icon;
        uint64_t owner64 = 0;
        UINT iconSize = 0, chars = 0;
        if (!take(&owner64, 8) || !take(&icon.id, 4) || !take(&icon.callback, 4) ||
            !take(&icon.version, 4) || !take(&iconSize, 4) || !take(&chars, 4) ||
            iconSize > 256 || chars > 4096) {
            break;
        }
        icon.owner = (HWND)(ULONG_PTR)owner64;
        icon.tip.resize(chars);
        icon.size = (int)iconSize;
        icon.pixels.resize((size_t)iconSize * iconSize);
        if (!take(icon.tip.data(), chars * sizeof(WCHAR)) ||
            !take(icon.pixels.data(), icon.pixels.size() * 4)) {
            break;
        }
        icons.push_back(std::move(icon));
    }
    return icons;
}

enum HiddenClick { kClickLeft, kClickRight, kClickMiddle, kClickDouble, kClickSecondOfDouble };

// Replays a click on a hidden icon for its program, the way the tray reports
// clicks in that icon's NOTIFYICON version.
void ForwardHiddenClick(HiddenIcon const& icon, int click) {
    DWORD pid = 0;
    GetWindowThreadProcessId(icon.owner, &pid);
    // Lets the program bring its window or menu to the front, also from a
    // helper process (Steam's come from steamwebhelper).
    AllowSetForegroundWindow(pid);
    AllowSetForegroundWindow(ASFW_ANY);

    std::vector<UINT> events;
    switch (click) {
        case kClickRight: events = {WM_RBUTTONDOWN, WM_RBUTTONUP, WM_CONTEXTMENU}; break;
        case kClickMiddle: events = {WM_MBUTTONDOWN, WM_MBUTTONUP}; break;
        case kClickDouble:
            events = {WM_LBUTTONDOWN, WM_LBUTTONUP, NIN_SELECT, WM_LBUTTONDBLCLK, WM_LBUTTONUP};
            break;
        case kClickSecondOfDouble: events = {WM_LBUTTONDBLCLK, WM_LBUTTONUP}; break;
        default: events = {WM_LBUTTONDOWN, WM_LBUTTONUP, NIN_SELECT}; break;
    }

    POINT pt;
    GetCursorPos(&pt);
    for (UINT event : events) {
        if (icon.version >= NOTIFYICON_VERSION_4) {
            PostMessageW(icon.owner, icon.callback, MAKEWPARAM(pt.x, pt.y), MAKELPARAM(event, icon.id));
        } else if ((event != NIN_SELECT && event != WM_CONTEXTMENU) || icon.version == NOTIFYICON_VERSION) {
            PostMessageW(icon.owner, icon.callback, icon.id, event);
        }
    }
}

// This copy runs inside Explorer, so it reads the tray directly.
bool RequestHiddenIcons(std::vector<HiddenIcon>* icons) {
    std::vector<BYTE> packet = BuildHiddenIconsPacket();
    *icons = UnpackHiddenIcons(packet.data(), packet.size());
    return true;
}
////////////////////////////////////////////////////////////////////////////////
// Flyouts: Windows 7 layouts in the Tourne design system. One at a time, on
// the mod thread.

enum FlyoutKind : int { kFlyoutNone, kFlyoutNetwork, kFlyoutPower, kFlyoutBrightness, kFlyoutOverflow, kFlyoutVpn, kFlyoutSecurity, kFlyoutMixer, kFlyoutNotify };

namespace ui {
constexpr COLORREF Rgb(uint32_t rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}
constexpr COLORREF kGround = Rgb(0x121C21);
constexpr COLORREF kRaised = Rgb(0x1B2A31);
constexpr COLORREF kLine = Rgb(0x1E2F36);
constexpr COLORREF kEdge = Rgb(0x4F8F6A);
constexpr COLORREF kInk = Rgb(0x66EA97);
constexpr COLORREF kGreen = Rgb(0x15FFA2);
constexpr COLORREF kSage = Rgb(0x64AA89);
constexpr COLORREF kSageAlt = Rgb(0x5D8D6F);
constexpr COLORREF kCoral = Rgb(0xFF6F77);
// ink at 16% over ground, the design's hover fill.
constexpr COLORREF kHover = Rgb(0x1F3D34);
}  // namespace ui

enum HitType : int {
    kHitWifiRow = 1,
    kHitWifiAction,
    kHitLink,
    kHitPlan,
    kHitSlider,
    kHitHidden,
    kHitVpnAction,
    kHitVpnRow,
    kHitSecurityRow,
    kHitMixerSlider,
    kHitMixerMute,
    kHitNotifyItem,
    kHitNotifyDismiss,
    kHitNotifyClearApp,
    kHitNotifyAction,
    kHitNotifyClearAll,
    kHitCalPrev,
    kHitCalNext,
    kHitCalDay,
};

constexpr int MakeHit(int type, int index) {
    return (type << 16) | (index & 0xFFFF);
}

struct Hit {
    RECT rc;
    int id;
};

struct WifiNetwork {
    std::wstring ssid;
    int quality = 0;
    bool secured = false;
    bool connected = false;
    std::wstring profile;
    GUID iface{};
    DOT11_SSID dot11{};
    DOT11_BSS_TYPE bssType = dot11_BSS_type_infrastructure;
};

struct Connection {
    std::string art;
    std::wstring name;
    std::wstring status;
};

struct PowerPlan {
    GUID guid{};
    std::wstring name;
    bool active = false;
};

struct MonitorLevel {
    std::wstring name;
    int value = -1;  // Percent, or -1 when the monitor doesn't answer.
};

// DDC/CI calls take tens of milliseconds each, so they run on a worker that
// outlives nothing but itself: the flyout can close while a write is in
// flight, and the shared state is freed by whichever side lets go last.
struct BrightnessShared {
    CRITICAL_SECTION lock;
    HANDLE wake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    HWND notify = nullptr;
    bool stop = false;
    bool loaded = false;
    std::vector<MonitorLevel> levels;
    std::vector<int> pending;
    LONG refs = 2;

    BrightnessShared() { InitializeCriticalSection(&lock); }
    ~BrightnessShared() {
        DeleteCriticalSection(&lock);
        CloseHandle(wake);
    }
    void Release() {
        if (InterlockedDecrement(&refs) == 0) {
            delete this;
        }
    }
};

std::vector<HANDLE> g_brightnessThreads;

// The levels last read, so the flyout opens with them while the monitors are
// asked again. Also read once at start.
std::vector<MonitorLevel> g_brightnessCache;
// The monitor's model name from its EDID ("PHL 242G5"), where the physical
// monitor API only knows "Generic PnP Monitor".
std::wstring MonitorModelName(HMONITOR display) {
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(display, &info)) {
        return {};
    }
    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        return {};
    }
    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths.data(), &modeCount,
                           modes.data(), nullptr) != ERROR_SUCCESS) {
        return {};
    }
    for (UINT32 i = 0; i < pathCount; i++) {
        DISPLAYCONFIG_SOURCE_DEVICE_NAME source{};
        source.header = {DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME, sizeof(source),
                         paths[i].sourceInfo.adapterId, paths[i].sourceInfo.id};
        if (DisplayConfigGetDeviceInfo(&source.header) != ERROR_SUCCESS ||
            wcscmp(source.viewGdiDeviceName, info.szDevice) != 0) {
            continue;
        }
        DISPLAYCONFIG_TARGET_DEVICE_NAME target{};
        target.header = {DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME, sizeof(target),
                         paths[i].targetInfo.adapterId, paths[i].targetInfo.id};
        if (DisplayConfigGetDeviceInfo(&target.header) == ERROR_SUCCESS) {
            return target.monitorFriendlyDeviceName;
        }
    }
    return {};
}

DWORD WINAPI BrightnessThreadProc(LPVOID parameter) {
    auto* shared = (BrightnessShared*)parameter;

    std::vector<HMONITOR> displays;
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
            ((std::vector<HMONITOR>*)lParam)->push_back(monitor);
            return TRUE;
        },
        (LPARAM)&displays);

    struct Physical {
        PHYSICAL_MONITOR monitor;
        DWORD min = 0, max = 100;
        bool ok = false;
    };
    std::vector<Physical> physical;
    std::vector<MonitorLevel> levels;
    int displayNumber = 0;
    for (HMONITOR display : displays) {
        displayNumber++;
        DWORD count = 0;
        if (!GetNumberOfPhysicalMonitorsFromHMONITOR(display, &count) || !count) {
            continue;
        }
        std::vector<PHYSICAL_MONITOR> monitors(count);
        if (!GetPhysicalMonitorsFromHMONITOR(display, count, monitors.data())) {
            continue;
        }
        std::wstring model = MonitorModelName(display);
        for (auto const& monitor : monitors) {
            Physical p{monitor};
            DWORD current = 0;
            p.ok = GetMonitorBrightness(monitor.hPhysicalMonitor, &p.min, &current, &p.max) &&
                   p.max > p.min;
            MonitorLevel level;
            level.name = std::to_wstring(displayNumber) + L" ... " +
                         (!model.empty() ? model
                          : *monitor.szPhysicalMonitorDescription
                              ? std::wstring(monitor.szPhysicalMonitorDescription)
                              : L"Display");
            if (p.ok) {
                level.value = (int)((current - p.min) * 100 / (p.max - p.min));
            }
            physical.push_back(p);
            levels.push_back(level);
        }
    }

    EnterCriticalSection(&shared->lock);
    shared->levels = levels;
    shared->pending.assign(levels.size(), -1);
    shared->loaded = true;
    HWND notify = shared->notify;
    LeaveCriticalSection(&shared->lock);
    if (notify) {
        PostMessageW(notify, WM_APP_BRIGHTNESS_READY, 0, 0);
    }

    // Latest value wins; writes are spaced so the I2C bus keeps up.
    while (true) {
        WaitForSingleObject(shared->wake, INFINITE);
        std::vector<int> work;
        EnterCriticalSection(&shared->lock);
        bool stop = shared->stop;
        work.swap(shared->pending);
        shared->pending.assign(physical.size(), -1);
        LeaveCriticalSection(&shared->lock);
        if (stop) {
            break;
        }
        for (size_t i = 0; i < work.size() && i < physical.size(); i++) {
            if (work[i] >= 0 && physical[i].ok) {
                DWORD value = physical[i].min +
                              (DWORD)work[i] * (physical[i].max - physical[i].min) / 100;
                SetMonitorBrightness(physical[i].monitor.hPhysicalMonitor, value);
                Sleep(60);
            }
        }
    }

    for (auto& p : physical) {
        DestroyPhysicalMonitor(p.monitor.hPhysicalMonitor);
    }
    shared->Release();
    return 0;
}

// Flyout data for Windows Security and the VPN, and the slow-work job both use.

struct FlyoutJob;

struct SecurityRow {
    std::string art;
    std::wstring label;
    std::wstring value;
    PCWSTR uri;
};

struct VpnCity {
    std::wstring city;
    std::wstring nick;
};

struct VpnRegion {
    std::wstring name;
    std::wstring country;
    std::vector<VpnCity> cities;
};

struct MixerSession {
    DWORD pid = 0;
    bool system = false;
    std::wstring path;
    std::wstring name;
    int volume = 0;
    bool muted = false;
    std::vector<ISimpleAudioVolume*> controls;  // Released by ReleaseMixer.
};

// Notification centre and calendar data.

struct NotifyAction {
    std::wstring label;
    std::wstring arguments;
    std::wstring activationType;
};

struct NotifyItem {
    std::wstring aumid;
    std::wstring tag;
    std::wstring group;
    std::wstring title;
    std::wstring body;
    std::wstring launch;
    std::wstring activationType;
    std::vector<NotifyAction> actions;
    ULONGLONG arrival = 0;  // UTC FILETIME.
};

struct NotifyApp {
    std::wstring aumid;
    std::wstring name;
    std::string design;
    std::vector<NotifyItem> items;
};

// A calendar event, in local time as minutes since 1601.
struct CalEvent {
    LONGLONG start = 0;
    LONGLONG end = 0;
    bool allDay = false;
    std::wstring title;
    // Recurrence, from RRULE.
    int freq = 0;  // 0 none, 1 daily, 2 weekly, 3 monthly, 4 yearly.
    int interval = 1;
    int count = 0;         // 0 for no limit.
    LONGLONG until = 0;    // 0 for no limit.
    int byDayMask = 0;     // Bit per weekday, Sunday first.
    std::vector<LONGLONG> exdates;
};

struct CalOccurrence {
    LONGLONG start;
    LONGLONG end;
    bool allDay;
    std::wstring title;
};

////////////////////////////////////////////////////////////////////////////////
// Notifications, read from Windows' own notification database with the SQLite
// that ships with Windows, read-only while Windows keeps writing it. Removing
// and opening them goes through the same calls Windows' centre uses.

// Dates and times in the user's region formats.
std::wstring LocaleString(LCTYPE type) {
    WCHAR buffer[128];
    return GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, type, buffer, ARRAYSIZE(buffer)) ? buffer : L"";
}

std::wstring FormatDate(SYSTEMTIME const& time, PCWSTR picture, DWORD flags = 0) {
    WCHAR buffer[128];
    return GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &time, picture, buffer, ARRAYSIZE(buffer), nullptr)
               ? buffer
               : L"";
}

std::wstring FormatTime(SYSTEMTIME const& time, PCWSTR picture, DWORD flags = 0) {
    WCHAR buffer[128];
    return GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, flags, &time, picture, buffer, ARRAYSIZE(buffer)) ? buffer : L"";
}

namespace sqlite {
struct Db;
struct Stmt;
using Open = int (*)(const char*, Db**, int, const char*);
using Prepare = int (*)(Db*, const char*, int, Stmt**, const char**);
using Step = int (*)(Stmt*);
using ColumnText = const unsigned char* (*)(Stmt*, int);
using ColumnInt64 = long long (*)(Stmt*, int);
using ColumnBlob = const void* (*)(Stmt*, int);
using ColumnBytes = int (*)(Stmt*, int);
using BindInt64 = int (*)(Stmt*, int, long long);
using Finalize = int (*)(Stmt*);
using Close = int (*)(Db*);

constexpr int kReadOnly = 0x01;
constexpr int kRow = 100;

struct Api {
    Open open = nullptr;
    Prepare prepare = nullptr;
    Step step = nullptr;
    ColumnText text = nullptr;
    ColumnInt64 int64 = nullptr;
    ColumnBlob blob = nullptr;
    ColumnBytes bytes = nullptr;
    BindInt64 bindInt64 = nullptr;
    Finalize finalize = nullptr;
    Close close = nullptr;
};

Api const* Load() {
    static Api api = [] {
        Api a;
        if (HMODULE lib = LoadLibraryExW(L"winsqlite3.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)) {
            a.open = (Open)GetProcAddress(lib, "sqlite3_open_v2");
            a.prepare = (Prepare)GetProcAddress(lib, "sqlite3_prepare_v2");
            a.step = (Step)GetProcAddress(lib, "sqlite3_step");
            a.text = (ColumnText)GetProcAddress(lib, "sqlite3_column_text");
            a.int64 = (ColumnInt64)GetProcAddress(lib, "sqlite3_column_int64");
            a.blob = (ColumnBlob)GetProcAddress(lib, "sqlite3_column_blob");
            a.bytes = (ColumnBytes)GetProcAddress(lib, "sqlite3_column_bytes");
            a.bindInt64 = (BindInt64)GetProcAddress(lib, "sqlite3_bind_int64");
            a.finalize = (Finalize)GetProcAddress(lib, "sqlite3_finalize");
            a.close = (Close)GetProcAddress(lib, "sqlite3_close");
        }
        return a;
    }();
    return api.open && api.prepare && api.step && api.text && api.int64 && api.blob && api.bytes &&
                   api.bindInt64 && api.finalize && api.close
               ? &api
               : nullptr;
}
}  // namespace sqlite

std::wstring NotificationsFolder() {
    WCHAR path[MAX_PATH];
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        return {};
    }
    return std::wstring(path) + L"\\Microsoft\\Windows\\Notifications";
}

std::string DatabasePath() {
    std::wstring path = NotificationsFolder() + L"\\wpndatabase.db";
    int size = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8(size > 0 ? size - 1 : 0, '\0');
    WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, utf8.data(), size, nullptr, nullptr);
    return utf8;
}
ULONGLONG NowFileTime() {
    FILETIME now;
    GetSystemTimeAsFileTime(&now);
    return ((ULONGLONG)now.dwHighDateTime << 32) | now.dwLowDateTime;
}

// Runs a query over the unexpired toasts; the callback gets each row.
template <typename Row>
bool QueryToasts(char const* columns, char const* tail, Row row) {
    auto* api = sqlite::Load();
    if (!api) {
        return false;
    }
    sqlite::Db* db = nullptr;
    if (api->open(DatabasePath().c_str(), &db, sqlite::kReadOnly, nullptr)) {
        if (db) {
            api->close(db);
        }
        return false;
    }
    std::string sql = std::string("SELECT ") + columns +
                      " FROM Notification n JOIN NotificationHandler h ON h.RecordId = n.HandlerId"
                      " WHERE n.Type = 'toast' AND (n.ExpiryTime = 0 OR n.ExpiryTime > ?1) " +
                      tail;
    sqlite::Stmt* stmt = nullptr;
    bool ok = api->prepare(db, sql.c_str(), -1, &stmt, nullptr) == 0;
    if (ok) {
        api->bindInt64(stmt, 1, (long long)NowFileTime());
        while (api->step(stmt) == sqlite::kRow) {
            row(api, stmt);
        }
        api->finalize(stmt);
    }
    api->close(db);
    return ok;
}

int CountNotifications() {
    int count = 0;
    QueryToasts("COUNT(*)", "", [&](sqlite::Api const* api, sqlite::Stmt* stmt) {
        count = (int)api->int64(stmt, 0);
    });
    return count;
}

// Just enough XML for toast payloads.
std::wstring XmlUnescape(std::wstring_view text) {
    std::wstring out;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] != L'&') {
            out += text[i];
            continue;
        }
        size_t end = text.find(L';', i);
        if (end == std::wstring_view::npos || end - i > 10) {
            out += text[i];
            continue;
        }
        std::wstring_view entity = text.substr(i + 1, end - i - 1);
        if (entity == L"amp") out += L'&';
        else if (entity == L"lt") out += L'<';
        else if (entity == L"gt") out += L'>';
        else if (entity == L"quot") out += L'"';
        else if (entity == L"apos") out += L'\'';
        else if (entity.starts_with(L"#x")) out += (WCHAR)wcstoul(std::wstring(entity.substr(2)).c_str(), nullptr, 16);
        else if (entity.starts_with(L"#")) out += (WCHAR)wcstoul(std::wstring(entity.substr(1)).c_str(), nullptr, 10);
        else {
            out += text.substr(i, end - i + 1);
        }
        i = end;
    }
    return out;
}

std::wstring XmlAttribute(std::wstring_view tag, std::wstring_view name) {
    for (size_t at = 0; (at = tag.find(name, at)) != std::wstring_view::npos; at += name.size()) {
        bool boundary = at > 0 && iswspace(tag[at - 1]);
        size_t eq = at + name.size();
        while (eq < tag.size() && iswspace(tag[eq])) {
            eq++;
        }
        if (!boundary || eq >= tag.size() || tag[eq] != L'=') {
            continue;
        }
        size_t quote = eq + 1;
        while (quote < tag.size() && iswspace(tag[quote])) {
            quote++;
        }
        if (quote >= tag.size() || (tag[quote] != L'"' && tag[quote] != L'\'')) {
            continue;
        }
        size_t close = tag.find(tag[quote], quote + 1);
        if (close == std::wstring_view::npos) {
            return {};
        }
        return XmlUnescape(tag.substr(quote + 1, close - quote - 1));
    }
    return {};
}

// Every start tag of an element, as its text between < and >.
std::vector<std::wstring_view> XmlStartTags(std::wstring_view xml, std::wstring_view element) {
    std::vector<std::wstring_view> tags;
    std::wstring open = L"<" + std::wstring(element);
    for (size_t at = 0; (at = xml.find(open, at)) != std::wstring_view::npos; at += open.size()) {
        wchar_t next = at + open.size() < xml.size() ? xml[at + open.size()] : L'>';
        if (!iswspace(next) && next != L'>' && next != L'/') {
            continue;
        }
        size_t close = xml.find(L'>', at);
        if (close == std::wstring_view::npos) {
            break;
        }
        tags.push_back(xml.substr(at, close - at + 1));
    }
    return tags;
}

void ParseToast(std::wstring_view xml, NotifyItem* item) {
    auto roots = XmlStartTags(xml, L"toast");
    if (!roots.empty()) {
        item->launch = XmlAttribute(roots[0], L"launch");
        item->activationType = XmlAttribute(roots[0], L"activationType");
    }
    std::vector<std::wstring> texts;
    for (size_t at = 0; (at = xml.find(L"<text", at)) != std::wstring_view::npos;) {
        size_t tagEnd = xml.find(L'>', at);
        if (tagEnd == std::wstring_view::npos) {
            break;
        }
        std::wstring_view tag = xml.substr(at, tagEnd - at + 1);
        at = tagEnd + 1;
        if (tag.ends_with(L"/>")) {
            continue;
        }
        size_t close = xml.find(L"</text>", at);
        if (close == std::wstring_view::npos) {
            break;
        }
        std::wstring text = XmlUnescape(xml.substr(at, close - at));
        at = close + 7;
        if (!text.empty() && XmlAttribute(tag, L"placement") != L"attribution") {
            texts.push_back(text);
        }
    }
    if (!texts.empty()) {
        item->title = texts[0];
        for (size_t i = 1; i < texts.size(); i++) {
            item->body += (i > 1 ? L"\n" : L"") + texts[i];
        }
    }
    for (auto tag : XmlStartTags(xml, L"action")) {
        std::wstring label = XmlAttribute(tag, L"content");
        if (label.empty() || XmlAttribute(tag, L"placement") == L"contextMenu") {
            continue;
        }
        item->actions.push_back(
            {label, XmlAttribute(tag, L"arguments"), XmlAttribute(tag, L"activationType")});
    }
}

// An app's name as the Start menu shows it, from its AppUserModelID.
std::wstring AppNameFor(std::wstring const& aumid) {
    static std::map<std::wstring, std::wstring> cache;
    if (auto it = cache.find(aumid); it != cache.end()) {
        return it->second;
    }
    std::wstring name;
    // Tray balloons ("NotifyIconGeneratedAumid_...") and other registered
    // ids carry their display name in the registry.
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, (L"Software\\Classes\\AppUserModelId\\" + aumid).c_str(), 0,
                      KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
        WCHAR buffer[256];
        DWORD size = sizeof(buffer);
        if (RegGetValueW(key, nullptr, L"DisplayName", RRF_RT_REG_SZ, nullptr, buffer, &size) == ERROR_SUCCESS &&
            buffer[0] && buffer[0] != L'@') {
            name = buffer;
        }
        RegCloseKey(key);
    }
    if (!name.empty()) {
        cache[aumid] = name;
        return name;
    }
    constexpr GUID kFolderAppsFolder = {
        0x1E87508D, 0x89C2, 0x42F0, {0x8A, 0x7E, 0x64, 0x5A, 0x0F, 0x50, 0xCA, 0x58}};
    IShellItem* item = nullptr;
    if (SUCCEEDED(SHCreateItemInKnownFolder(kFolderAppsFolder, KF_FLAG_DONT_VERIFY, aumid.c_str(),
                                            IID_IShellItem, (void**)&item))) {
        LPWSTR display = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &display)) && display) {
            name = display;
            CoTaskMemFree(display);
        }
        item->Release();
    }
    if (name.empty() || name == aumid) {
        // "Windows.SystemToast.SecurityAndMaintenance": the last part, spaced.
        std::wstring last = aumid.substr(aumid.find_last_of(L".!\\") + 1);
        for (size_t i = 0; i < last.size(); i++) {
            if (i && iswupper(last[i]) && iswlower(last[i - 1])) {
                name += L' ';
            }
            name += last[i];
        }
    }
    cache[aumid] = name;
    return name;
}

// The design that stands for an app's notifications.
std::string DesignForApp(std::wstring const& aumid, std::wstring const& name) {
    std::wstring text = aumid + L" " + name;
    auto has = [&](std::initializer_list<PCWSTR> words) { return TipHas(text, words); };
    if (has({L"claude"})) return "app-claude";
    if (has({L"tamer"})) return "app-screenshot";
    if (has({L"wallpaper engine", L"wallpaper64"})) return "app-wallpaper";
    if (has({L"spotify"})) return "app-spotify";
    if (has({L"discord", L"vesktop", L"teams", L"slack", L"telegram", L"whatsapp", L"messenger", L"signal",
             L"claude", L"chat"})) {
        return "app-chat";
    }
    if (has({L"onedrive", L"icloud", L"dropbox", L"google drive"})) return "app-cloud";
    if (has({L"steam", L"xbox", L"gamingapp", L"epic", L"riot", L"battle.net", L"playnite", L"ea desktop"})) {
        return "app-game";
    }
    if (has({L"windscribe", L"vpn"})) return "vpn";
    if (has({L"securityhealth", L"defender", L"securityandmaintenance", L"windows security"})) {
        return "security-ok";
    }
    if (has({L"windowsupdate", L"musnotify", L"update"})) return "update-available";
    if (has({L"bluetooth"})) return "bluetooth";
    if (has({L"autoplay", L"hotplug", L"usb"})) return "usb";
    if (has({L"phone"})) return "phone";
    if (has({L"spotify", L"music", L"audio", L"sound"})) return "app-audio";
    if (has({L"snipping", L"screensketch", L"capture", L"obs"})) return "app-capture";
    if (has({L"clipboard"})) return "app-clipboard";
    if (has({L"battery", L"power options", L"power plan"})) return "battery-mid";
    return "bell";
}

std::vector<NotifyApp> LoadNotifications() {
    std::vector<NotifyApp> apps;
    QueryToasts("h.PrimaryId, n.Tag, n.[Group], n.ArrivalTime, n.Payload", "ORDER BY n.ArrivalTime DESC",
                [&](sqlite::Api const* api, sqlite::Stmt* stmt) {
                    auto text = [&](int column) {
                        auto* value = api->text(stmt, column);
                        return value ? Utf8ToWide(std::string_view((const char*)value)) : std::wstring();
                    };
                    NotifyItem item;
                    item.aumid = text(0);
                    item.tag = text(1);
                    item.group = text(2);
                    item.arrival = (ULONGLONG)api->int64(stmt, 3);
                    int size = api->bytes(stmt, 4);
                    auto* payload = (const char*)api->blob(stmt, 4);
                    if (payload && size > 0) {
                        ParseToast(Utf8ToWide(std::string_view(payload, size)), &item);
                    }
                    auto app = std::find_if(apps.begin(), apps.end(),
                                            [&](NotifyApp const& a) { return a.aumid == item.aumid; });
                    if (app == apps.end()) {
                        std::wstring name = AppNameFor(item.aumid);
                        apps.push_back({item.aumid, name, DesignForApp(item.aumid, name), {}});
                        app = apps.end() - 1;
                    }
                    app->items.push_back(std::move(item));
                });
    return apps;
}

// Removing: the toast history, which is what Windows' centre uses too.
void RemoveNotification(NotifyItem const& item) {
    try {
        auto history = winrt::Windows::UI::Notifications::ToastNotificationManager::History();
        history.Remove(item.tag, item.group, item.aumid);
    } catch (...) {
    }
}

void ClearAppNotifications(std::wstring const& aumid) {
    try {
        winrt::Windows::UI::Notifications::ToastNotificationManager::History().Clear(aumid);
    } catch (...) {
    }
}

// A desktop app's notification activator, the COM class Windows calls when
// one of its notifications is clicked.
struct IToastActivator : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Activate(LPCWSTR appUserModelId, LPCWSTR invokedArgs,
                                               const void* data, ULONG count) = 0;
};
constexpr GUID kIidToastActivator = {
    0x53E31837, 0x6600, 0x4A81, {0x93, 0x95, 0x75, 0xCF, 0xFE, 0x74, 0x6F, 0x94}};
constexpr GUID kClsidApplicationActivationManager = {
    0x45BA127D, 0x10A8, 0x46EA, {0x8A, 0xB7, 0x56, 0xEA, 0x90, 0x78, 0x94, 0x3C}};
constexpr GUID kIidApplicationActivationManager = {
    0x2E941141, 0x7F97, 0x4756, {0xBA, 0x1D, 0x9D, 0xEC, 0xDE, 0x89, 0x4A, 0x3D}};

bool ActivateWithActivator(std::wstring const& aumid, std::wstring const& arguments) {
    WCHAR clsidText[64];
    DWORD size = sizeof(clsidText);
    std::wstring key = L"Software\\Classes\\AppUserModelId\\" + aumid;
    CLSID clsid;
    if (RegGetValueW(HKEY_CURRENT_USER, key.c_str(), L"CustomActivator", RRF_RT_REG_SZ, nullptr, clsidText,
                     &size) != ERROR_SUCCESS ||
        FAILED(CLSIDFromString(clsidText, &clsid))) {
        return false;
    }
    IToastActivator* activator = nullptr;
    if (FAILED(CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, kIidToastActivator, (void**)&activator))) {
        return false;
    }
    CoAllowSetForegroundWindow(activator, nullptr);
    HRESULT hr = activator->Activate(aumid.c_str(), arguments.c_str(), nullptr, 0);
    activator->Release();
    return SUCCEEDED(hr);
}

void LaunchApp(std::wstring const& aumid, std::wstring const& arguments) {
    if (aumid.find(L'!') != std::wstring::npos) {
        IApplicationActivationManager* manager = nullptr;
        if (SUCCEEDED(CoCreateInstance(kClsidApplicationActivationManager, nullptr, CLSCTX_LOCAL_SERVER,
                                       kIidApplicationActivationManager, (void**)&manager))) {
            DWORD pid = 0;
            HRESULT hr = manager->ActivateApplication(aumid.c_str(), arguments.empty() ? nullptr : arguments.c_str(),
                                                      AO_NONE, &pid);
            manager->Release();
            if (SUCCEEDED(hr)) {
                return;
            }
        }
    }
    std::wstring target = L"shell:AppsFolder\\" + aumid;
    ShellExecuteW(nullptr, L"open", target.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

// Opens a notification (or one of its buttons) the way Windows would.
void ActivateNotification(NotifyItem const& item, std::wstring const& arguments,
                          std::wstring const& activationType) {
    if (activationType == L"protocol" && !arguments.empty()) {
        ShellExecuteW(nullptr, L"open", arguments.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    } else if (activationType == L"system") {
        // Snooze and dismiss buttons: dismissing is what's left without a
        // snooze time.
    } else if (!ActivateWithActivator(item.aumid, arguments)) {
        LaunchApp(item.aumid, arguments);
    }
    RemoveNotification(item);
}

std::wstring FormatDateShort(SYSTEMTIME const& time);

std::wstring RelativeTime(ULONGLONG arrival) {
    ULONGLONG now = NowFileTime();
    LONGLONG minutes = now > arrival ? (LONGLONG)((now - arrival) / 600000000ULL) : 0;
    if (minutes < 1) return L"now";
    if (minutes < 60) return std::to_wstring(minutes) + L"m";
    if (minutes < 24 * 60) return std::to_wstring(minutes / 60) + L"h";
    FILETIME utc{(DWORD)arrival, (DWORD)(arrival >> 32)}, local;
    SYSTEMTIME time;
    FileTimeToLocalFileTime(&utc, &local);
    FileTimeToSystemTime(&local, &time);
    return FormatDateShort(time);
}

// Windows writes the database as notifications come and go; its folder is
// watched while System Icons runs.
HANDLE g_notificationsWatch = INVALID_HANDLE_VALUE;

////////////////////////////////////////////////////////////////////////////////
// Calendar: the .ics calendar the Agenda in Calendar View mod shows (or this
// mod's own), fetched on the same schedule and kept, so the flyout opens with
// it at once.

LONGLONG MinutesOf(SYSTEMTIME const& local) {
    FILETIME ft;
    SystemTimeToFileTime(&local, &ft);
    return (LONGLONG)((((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime) / 600000000ULL);
}

SYSTEMTIME TimeOfMinutes(LONGLONG minutes) {
    ULONGLONG ticks = (ULONGLONG)minutes * 600000000ULL;
    FILETIME ft{(DWORD)ticks, (DWORD)(ticks >> 32)};
    SYSTEMTIME time;
    FileTimeToSystemTime(&ft, &time);
    return time;
}

LONGLONG DayStart(LONGLONG minutes) {
    return minutes - minutes % (24 * 60);
}

// IANA time zones ("America/New_York") through the ICU that ships with
// Windows; times in them come out as local time.
LONGLONG ZonedToLocal(SYSTEMTIME const& wall, std::wstring const& zone) {
    using Open = void* (*)(const wchar_t*, int32_t, const char*, int, int*);
    using SetDateTime = void (*)(void*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int*);
    using Set = void (*)(void*, int, int32_t);
    using GetMillis = double (*)(const void*, int*);
    using CloseCal = void (*)(void*);
    static HMODULE icu = LoadLibraryExW(L"icu.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    static auto open = icu ? (Open)GetProcAddress(icu, "ucal_open") : nullptr;
    static auto setDateTime = icu ? (SetDateTime)GetProcAddress(icu, "ucal_setDateTime") : nullptr;
    static auto set = icu ? (Set)GetProcAddress(icu, "ucal_set") : nullptr;
    static auto getMillis = icu ? (GetMillis)GetProcAddress(icu, "ucal_getMillis") : nullptr;
    static auto close = icu ? (CloseCal)GetProcAddress(icu, "ucal_close") : nullptr;
    if (!open || !setDateTime || !set || !getMillis || !close) {
        return MinutesOf(wall);
    }
    int status = 0;
    void* calendar = open(zone.c_str(), (int32_t)zone.size(), "", 1 /* UCAL_GREGORIAN */, &status);
    if (!calendar || status > 0) {
        return MinutesOf(wall);
    }
    setDateTime(calendar, wall.wYear, wall.wMonth - 1, wall.wDay, wall.wHour, wall.wMinute, wall.wSecond, &status);
    set(calendar, 14 /* UCAL_MILLISECOND */, 0);
    double millis = getMillis(calendar, &status);
    close(calendar);
    if (status > 0) {
        return MinutesOf(wall);
    }
    // Milliseconds since 1970 in UTC, then to local time.
    ULONGLONG ticks = (ULONGLONG)(millis * 10000.0) + 116444736000000000ULL;
    FILETIME utc{(DWORD)ticks, (DWORD)(ticks >> 32)}, local;
    FileTimeToLocalFileTime(&utc, &local);
    return (LONGLONG)((((ULONGLONG)local.dwHighDateTime << 32) | local.dwLowDateTime) / 600000000ULL);
}

// "20260925", "20260925T090000", "20260925T090000Z", with the TZID parameter.
bool ParseIcsTime(std::wstring const& value, std::wstring const& zone, LONGLONG* minutes, bool* allDay) {
    SYSTEMTIME time{};
    if (value.size() < 8 || swscanf_s(value.c_str(), L"%4hu%2hu%2hu", &time.wYear, &time.wMonth, &time.wDay) != 3) {
        return false;
    }
    *allDay = value.size() == 8;
    if (!*allDay) {
        if (value.size() < 15 || value[8] != L'T' ||
            swscanf_s(value.c_str() + 9, L"%2hu%2hu%2hu", &time.wHour, &time.wMinute, &time.wSecond) != 3) {
            return false;
        }
        if (value.back() == L'Z') {
            SYSTEMTIME local;
            if (SystemTimeToTzSpecificLocalTime(nullptr, &time, &local)) {
                time = local;
            }
        } else if (!zone.empty()) {
            *minutes = ZonedToLocal(time, zone);
            return true;
        }
    }
    *minutes = MinutesOf(time);
    return true;
}

std::vector<CalEvent> ParseIcs(std::wstring const& text) {
    // Unfold continuation lines first.
    std::vector<std::wstring> lines;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        if (!line.empty() && line.back() == L'\r') {
            line.pop_back();
        }
        if (!line.empty() && (line[0] == L' ' || line[0] == L'\t') && !lines.empty()) {
            lines.back() += line.substr(1);
        } else {
            lines.push_back(line);
        }
        if (end == std::wstring::npos) {
            break;
        }
        at = end + 1;
    }

    std::vector<CalEvent> events;
    CalEvent event;
    bool inEvent = false, cancelled = false, hasEnd = false;
    for (auto const& line : lines) {
        if (line == L"BEGIN:VEVENT") {
            event = {};
            inEvent = true;
            cancelled = hasEnd = false;
            continue;
        }
        if (line == L"END:VEVENT") {
            if (inEvent && !cancelled && event.start) {
                if (!hasEnd) {
                    event.end = event.start + (event.allDay ? 24 * 60 : 0);
                }
                events.push_back(event);
            }
            inEvent = false;
            continue;
        }
        if (!inEvent) {
            continue;
        }
        size_t colon = line.find(L':');
        if (colon == std::wstring::npos) {
            continue;
        }
        std::wstring head = line.substr(0, colon);
        std::wstring value = line.substr(colon + 1);
        std::wstring name = head.substr(0, head.find(L';'));
        std::wstring zone;
        if (size_t tz = head.find(L";TZID="); tz != std::wstring::npos) {
            zone = head.substr(tz + 6);
            zone = zone.substr(0, zone.find(L';'));
            if (zone.size() > 1 && zone.front() == L'"') {
                zone = zone.substr(1, zone.size() - 2);
            }
        }
        if (name == L"SUMMARY") {
            for (size_t i = 0; i < value.size(); i++) {
                if (value[i] == L'\\' && i + 1 < value.size()) {
                    wchar_t next = value[i + 1];
                    event.title += next == L'n' || next == L'N' ? L' ' : next;
                    i++;
                } else {
                    event.title += value[i];
                }
            }
        } else if (name == L"DTSTART") {
            ParseIcsTime(value, zone, &event.start, &event.allDay);
        } else if (name == L"DTEND") {
            bool allDay = false;
            hasEnd = ParseIcsTime(value, zone, &event.end, &allDay);
        } else if (name == L"STATUS") {
            cancelled = value == L"CANCELLED";
        } else if (name == L"EXDATE") {
            for (size_t start = 0; start < value.size();) {
                size_t comma = value.find(L',', start);
                LONGLONG minutes = 0;
                bool allDay = false;
                if (ParseIcsTime(value.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start),
                                 zone, &minutes, &allDay)) {
                    event.exdates.push_back(minutes);
                }
                if (comma == std::wstring::npos) {
                    break;
                }
                start = comma + 1;
            }
        } else if (name == L"RRULE") {
            for (size_t start = 0; start < value.size();) {
                size_t semi = value.find(L';', start);
                std::wstring part = value.substr(start, semi == std::wstring::npos ? std::wstring::npos : semi - start);
                size_t eq = part.find(L'=');
                std::wstring key = part.substr(0, eq), val = eq == std::wstring::npos ? L"" : part.substr(eq + 1);
                if (key == L"FREQ") {
                    event.freq = val == L"DAILY" ? 1 : val == L"WEEKLY" ? 2 : val == L"MONTHLY" ? 3 : val == L"YEARLY" ? 4 : 0;
                } else if (key == L"INTERVAL") {
                    event.interval = std::max(1, _wtoi(val.c_str()));
                } else if (key == L"COUNT") {
                    event.count = _wtoi(val.c_str());
                } else if (key == L"UNTIL") {
                    bool allDay = false;
                    ParseIcsTime(val, L"", &event.until, &allDay);
                    if (allDay) {
                        event.until += 24 * 60 - 1;
                    }
                } else if (key == L"BYDAY") {
                    static const PCWSTR days[] = {L"SU", L"MO", L"TU", L"WE", L"TH", L"FR", L"SA"};
                    for (int d = 0; d < 7; d++) {
                        if (val.find(days[d]) != std::wstring::npos) {
                            event.byDayMask |= 1 << d;
                        }
                    }
                }
                if (semi == std::wstring::npos) {
                    break;
                }
                start = semi + 1;
            }
        }
    }
    return events;
}

// Every occurrence overlapping [from, to), in local minutes.
std::vector<CalOccurrence> Occurrences(std::vector<CalEvent> const& events, LONGLONG from, LONGLONG to) {
    std::vector<CalOccurrence> out;
    for (auto const& event : events) {
        LONGLONG length = std::max<LONGLONG>(0, event.end - event.start);
        auto add = [&](LONGLONG start) {
            if (start < to && start + std::max<LONGLONG>(length, 1) > from &&
                std::find(event.exdates.begin(), event.exdates.end(), start) == event.exdates.end()) {
                out.push_back({start, start + length, event.allDay, event.title});
            }
        };
        if (!event.freq) {
            add(event.start);
            continue;
        }
        SYSTEMTIME first = TimeOfMinutes(event.start);
        LONGLONG timeOfDay = event.start - DayStart(event.start);
        int produced = 0;
        auto allowed = [&](LONGLONG start) {
            return (!event.until || start <= event.until) && (!event.count || produced < event.count);
        };
        if (event.freq == 1 || (event.freq == 2 && !event.byDayMask)) {
            LONGLONG step = (event.freq == 1 ? 1 : 7) * event.interval * 24 * 60;
            LONGLONG start = event.start;
            // Skip straight to the range when there's no count to keep.
            if (!event.count && start + length < from) {
                start += (from - length - start) / step * step;
            }
            for (int guard = 0; guard < 5000 && start < to && allowed(start); guard++, start += step) {
                add(start);
                produced++;
            }
        } else if (event.freq == 2) {
            // Weekly on chosen days: walk the weeks from the first one.
            LONGLONG weekStart = DayStart(event.start) - first.wDayOfWeek * 24 * 60;
            LONGLONG step = 7 * event.interval * 24 * 60;
            if (!event.count && weekStart + step < from) {
                weekStart += (from - weekStart) / step * step - step;
            }
            for (int guard = 0; guard < 2000 && weekStart < to; guard++, weekStart += step) {
                for (int d = 0; d < 7; d++) {
                    LONGLONG start = weekStart + d * 24 * 60 + timeOfDay;
                    if (!(event.byDayMask & (1 << d)) || start < event.start) {
                        continue;
                    }
                    if (!allowed(start)) {
                        guard = 2000;
                        break;
                    }
                    add(start);
                    produced++;
                }
            }
        } else {
            // Monthly and yearly on the first occurrence's day of the month.
            SYSTEMTIME at = first;
            for (int guard = 0; guard < 3000; guard++) {
                // Days a month doesn't have (the 31st, February 29th) are
                // skipped, as the calendar standard says.
                FILETIME check;
                bool valid = SystemTimeToFileTime(&at, &check);
                LONGLONG start = valid ? MinutesOf(at) : 0;
                if (valid && start >= to) {
                    break;
                }
                if (valid) {
                    if (!allowed(start)) {
                        break;
                    }
                    add(start);
                    produced++;
                }
                if (event.freq == 3) {
                    int month = at.wMonth - 1 + event.interval;
                    at.wYear += month / 12;
                    at.wMonth = month % 12 + 1;
                } else {
                    at.wYear += event.interval;
                }
            }
        }
    }
    std::sort(out.begin(), out.end(), [](CalOccurrence const& a, CalOccurrence const& b) {
        return a.allDay != b.allDay ? a.allDay : a.start < b.start;
    });
    return out;
}

SRWLOCK g_calendarLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::vector<CalEvent> g_calendar;
DWORD g_calendarFetchTick;
bool g_calendarFetching;

std::wstring CalendarSource(DWORD* intervalMinutes) {
    *intervalMinutes = 60;
    std::wstring own = g_settings.calendarIcs;
    if (!own.empty()) {
        return own;
    }
    // The Agenda in Calendar View mod's calendar and schedule.
    WCHAR path[2048];
    DWORD size = sizeof(path);
    PCWSTR key = L"SOFTWARE\\Windhawk\\Engine\\Mods\\agenda-in-calendar-view\\Settings";
    if (RegGetValueW(HKEY_LOCAL_MACHINE, key, L"icsPath", RRF_RT_REG_SZ, nullptr, path, &size) != ERROR_SUCCESS) {
        return {};
    }
    DWORD interval = 0;
    size = sizeof(interval);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, key, L"minFetchInterval", RRF_RT_REG_DWORD, nullptr, &interval, &size) ==
        ERROR_SUCCESS) {
        *intervalMinutes = interval;
    }
    return path;
}

std::wstring CalendarCachePath() {
    WCHAR path[MAX_PATH];
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        return {};
    }
    std::wstring folder = std::wstring(path) + L"\\TourneTray";
    CreateDirectoryW(folder.c_str(), nullptr);
    return folder + L"\\calendar.ics";
}

std::wstring ReadTextFile(std::wstring const& path) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return {};
    }
    std::string data;
    char buffer[65536];
    DWORD read = 0;
    while (ReadFile(file, buffer, sizeof(buffer), &read, nullptr) && read) {
        data.append(buffer, read);
    }
    CloseHandle(file);
    return Utf8ToWide(data);
}

void SetCalendar(std::vector<CalEvent> events) {
    AcquireSRWLockExclusive(&g_calendarLock);
    g_calendar = std::move(events);
    ReleaseSRWLockExclusive(&g_calendarLock);
}

struct CalendarFetch {
    std::wstring source;
    std::wstring cache;
    HWND notify;
};

DWORD WINAPI CalendarFetchProc(LPVOID parameter) {
    auto* fetch = (CalendarFetch*)parameter;
    std::wstring source = fetch->source;
    if (source.starts_with(L"webcal://")) {
        source = L"https://" + source.substr(9);
    }
    bool remote = source.starts_with(L"http://") || source.starts_with(L"https://");
    if (remote) {
        DeleteUrlCacheEntryW(source.c_str());
        std::wstring partial = fetch->cache + L".part";
        if (SUCCEEDED(URLDownloadToFileW(nullptr, source.c_str(), partial.c_str(), 0, nullptr))) {
            MoveFileExW(partial.c_str(), fetch->cache.c_str(), MOVEFILE_REPLACE_EXISTING);
        } else {
            DeleteFileW(partial.c_str());
        }
    }
    auto events = ParseIcs(ReadTextFile(remote ? fetch->cache : source));
    SetCalendar(std::move(events));
    PostMessageW(fetch->notify, WM_APP_CALENDAR_READY, 0, 0);
    delete fetch;
    return 0;
}

// Loads the kept copy at once, then fetches a fresh one when it's due.
void RefreshCalendar(bool force) {
    DWORD interval = 60;
    std::wstring source = CalendarSource(&interval);
    if (source.empty() || g_calendarFetching || !g_wnd) {
        return;
    }
    bool remote = source.starts_with(L"http") || source.starts_with(L"webcal");
    DWORD age = GetTickCount() - g_calendarFetchTick;
    if (!force && g_calendarFetchTick && remote && age < interval * 60000ULL) {
        return;
    }
    auto* fetch = new CalendarFetch{source, CalendarCachePath(), g_wnd};
    if (!g_calendarFetchTick && remote) {
        SetCalendar(ParseIcs(ReadTextFile(fetch->cache)));
    }
    g_calendarFetchTick = GetTickCount();
    g_calendarFetching = true;
    HANDLE thread = CreateThread(nullptr, 0, CalendarFetchProc, fetch, 0, nullptr);
    if (!thread) {
        delete fetch;
        g_calendarFetching = false;
        return;
    }
    g_brightnessThreads.push_back(thread);
}

std::vector<CalOccurrence> CalendarBetween(LONGLONG from, LONGLONG to) {
    AcquireSRWLockShared(&g_calendarLock);
    auto result = Occurrences(g_calendar, from, to);
    ReleaseSRWLockShared(&g_calendarLock);
    return result;
}

struct FlyoutState {
    HWND hwnd = nullptr;
    int kind = kFlyoutNone;
    int scale = 1;
    int width = 0;
    int hover = 0;
    int dragSlider = -1;
    std::vector<Hit> hits;

    // Network
    std::vector<Connection> connections;
    std::vector<WifiNetwork> networks;
    bool wifiAdapter = false;
    bool wifiOff = false;
    int selectedNetwork = -1;
    std::wstring selectedSsid;  // Survives the list re-sorting on updates.
    int networkScroll = 0;

    // Power
    IconState battery;
    std::vector<PowerPlan> plans;

    // Brightness
    BrightnessShared* brightness = nullptr;
    std::vector<MonitorLevel> monitors;
    bool monitorsLoaded = false;

    // Hidden icons
    std::vector<HiddenIcon> hidden;

    // Windows Security
    std::vector<SecurityRow> securityRows;
    std::wstring securityScan;
    std::wstring securityUpdates;  // From the job: a count, "restart" or "unknown".

    // VPN
    std::vector<VpnRegion> vpnRegions;
    int vpnOpenRegion = -1;
    int vpnScroll = 0;
    std::wstring vpnStatus;
    bool vpnConnected = false;
    bool vpnBusy = false;
    bool vpnChecking = false;

    FlyoutJob* job = nullptr;

    // Volume mixer
    std::vector<MixerSession> mixer;
    std::wstring mixerDevice;
    int mixerMaster = -1;
    bool mixerMasterMuted = false;
    int dragMixer = -1;
    bool swallowUp = false;  // The button up that ends a forwarded double click.

    // Rearranging the hidden icons by dragging.
    int dragHidden = -1;
    POINT dragStart{};
    bool dragMoved = false;
    int dragTarget = -1;
    int overflowLeft = 0;
    int overflowTop = 0;
    int overflowCell = 0;
    // Notifications and calendar
    std::vector<NotifyApp> notifyApps;
    std::vector<std::array<int, 3>> notifyTargets;  // App, notification, button (-1 for none).
    int notifyScroll = 0;
    int notifyEntries = 0;
    bool notifyDnd = false;
    int calYear = 0;
    int calMonth = 0;
    LONGLONG calSelected = 0;
    std::vector<CalOccurrence> calMonthEvents;
    // Fonts
    HFONT label = nullptr;
    HFONT body = nullptr;
    HFONT caption = nullptr;
};

FlyoutState g_flyout;
DWORD g_flyoutClosedTick;
int g_flyoutClosedKind;
bool g_flyoutClosing;

// After a click on a hidden icon the flyout stays open while the program
// answers: its menu can take the foreground and be used. The next click
// anywhere else, or a switch to another program, closes it.
DWORD g_stickyPid;
std::unordered_map<DWORD, bool> g_stickyRelated;
HHOOK g_stickyMouse;
HWINEVENTHOOK g_stickyForeground;

// The program itself or one of its helpers (Steam's menu and window come
// from steamwebhelper, a child and grandchild of steam.exe).
bool IsStickyProgram(DWORD pid) {
    if (!g_stickyPid || !pid) {
        return false;
    }
    if (pid == g_stickyPid) {
        return true;
    }
    if (auto it = g_stickyRelated.find(pid); it != g_stickyRelated.end()) {
        return it->second;
    }
    std::unordered_map<DWORD, DWORD> parents;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry{sizeof(entry)};
        for (BOOL more = Process32FirstW(snapshot, &entry); more;
             more = Process32NextW(snapshot, &entry)) {
            parents[entry.th32ProcessID] = entry.th32ParentProcessID;
        }
        CloseHandle(snapshot);
    }
    bool related = false;
    DWORD at = pid;
    for (int depth = 0; depth < 4 && !related; depth++) {
        auto it = parents.find(at);
        if (it == parents.end() || !it->second || it->second == at) {
            break;
        }
        at = it->second;
        related = at == g_stickyPid;
    }
    g_stickyRelated[pid] = related;
    return related;
}

// The desktop and taskbars, which belong to Explorer whatever it owns.
bool IsShellSurface(HWND hwnd) {
    std::wstring windowClass = WindowClassOf(hwnd);
    return windowClass == L"Shell_TrayWnd" || windowClass == L"Shell_SecondaryTrayWnd" ||
           windowClass == L"Progman" || windowClass == L"WorkerW";
}

// A menu or popup of the program, as opposed to its main window.
bool IsStickyMenu(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (!hwnd || IsShellSurface(hwnd) || !IsStickyProgram(pid)) {
        return false;
    }
    return WindowClassOf(hwnd) == L"#32768" ||
           (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & (WS_EX_TOOLWINDOW | WS_EX_TOPMOST));
}

void CloseStickyFlyout() {
    if (g_flyout.hwnd) {
        // Counts as a close for the icon's toggle, like losing activation.
        g_flyoutClosedTick = GetTickCount();
        g_flyoutClosedKind = g_flyout.kind;
        PostMessageW(g_flyout.hwnd, WM_CLOSE, 0, 0);
    }
}

LRESULT CALLBACK StickyMouseProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION && g_flyout.hwnd &&
        (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN)) {
        HWND target = GetAncestor(WindowFromPoint(((MSLLHOOKSTRUCT*)lParam)->pt), GA_ROOT);
        if (target != g_flyout.hwnd && !IsStickyMenu(target)) {
            CloseStickyFlyout();
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

void CALLBACK StickyForegroundProc(HWINEVENTHOOK, DWORD, HWND hwnd, LONG idObject, LONG, DWORD,
                                   DWORD) {
    if (idObject != OBJID_WINDOW || !hwnd || !g_flyout.hwnd || hwnd == g_flyout.hwnd) {
        return;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (IsShellSurface(hwnd) || !IsStickyProgram(pid)) {
        CloseStickyFlyout();
    }
}

void StartSticky(HWND owner) {
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    if (pid != g_stickyPid) {
        g_stickyRelated.clear();
    }
    g_stickyPid = pid;
    HMODULE module = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&StickyMouseProc, &module);
    if (!g_stickyMouse) {
        g_stickyMouse = SetWindowsHookExW(WH_MOUSE_LL, StickyMouseProc, module, 0);
    }
    if (!g_stickyForeground) {
        g_stickyForeground =
            SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr,
                            StickyForegroundProc, 0, 0,
                            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    }
}

void StopSticky() {
    if (g_stickyMouse) {
        UnhookWindowsHookEx(g_stickyMouse);
        g_stickyMouse = nullptr;
    }
    if (g_stickyForeground) {
        UnhookWinEvent(g_stickyForeground);
        g_stickyForeground = nullptr;
    }
    g_stickyPid = 0;
    g_stickyRelated.clear();
}

// Programs that open their window on a double click of their tray icon, from
// the setting.
bool OpensOnDoubleClick(HWND owner) {
    std::wstring name = OwnerFileName(owner);
    if (name.empty()) {
        return false;
    }
    std::wstring bare = name.ends_with(L".exe") ? name.substr(0, name.size() - 4) : name;
    std::wstring list = g_settings.doubleClickApps;
    for (auto& ch : list) {
        ch = towlower(ch);
    }
    size_t start = 0;
    while (start < list.size()) {
        size_t end = list.find_first_of(L",;", start);
        if (end == std::wstring::npos) {
            end = list.size();
        }
        std::wstring item = list.substr(start, end - start);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty() && (item == name || item == bare)) {
            return true;
        }
        start = end + 1;
    }
    return false;
}
constexpr WCHAR kFlyoutClass[] = L"TourneTrayFlyout_" WH_MOD_ID;
constexpr int kMaxVisibleNetworks = 7;

bool FontInstalled(PCWSTR face) {
    LOGFONTW lf{};
    lf.lfCharSet = DEFAULT_CHARSET;
    wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
    HDC dc = GetDC(nullptr);
    bool found = !EnumFontFamiliesExW(
        dc, &lf, [](const LOGFONTW*, const TEXTMETRICW*, DWORD, LPARAM) -> int { return 0; },
        0, 0);
    ReleaseDC(nullptr, dc);
    return found;
}

// Pixel fonts first, drawn without smoothing at whole multiples of their size.
HFONT MakeFont(std::initializer_list<PCWSTR> faces, int pixels) {
    for (PCWSTR face : faces) {
        if (FontInstalled(face)) {
            LOGFONTW lf{};
            lf.lfHeight = -pixels;
            lf.lfWeight = FW_NORMAL;
            lf.lfCharSet = DEFAULT_CHARSET;
            lf.lfQuality = NONANTIALIASED_QUALITY;
            wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
            return CreateFontIndirectW(&lf);
        }
    }
    LOGFONTW lf{};
    lf.lfHeight = -pixels;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, L"Consolas");
    return CreateFontIndirectW(&lf);
}

void DestroyFonts() {
    for (HFONT* font : {&g_flyout.label, &g_flyout.body, &g_flyout.caption}) {
        if (*font) {
            DeleteObject(*font);
            *font = nullptr;
        }
    }
}

void CreateFonts(int scale) {
    DestroyFonts();
    g_flyout.label = MakeFont({L"scientifica", L"CozetteVector"}, 11 * scale);
    g_flyout.body = MakeFont({L"ProggyClean Nerd Font Mono", L"ProggyCleanTT", L"CozetteVector"},
                             16 * scale);
    g_flyout.caption = MakeFont({L"CozetteVector", L"scientifica"}, 13 * scale);
}

// Measures and draws in one pass; with paint off it only lays out.
struct Canvas {
    HDC dc;
    bool paint;
    int s;
    int width;
    int y = 0;

    int Px(int v) const { return v * s; }

    void Fill(RECT rc, COLORREF color) const {
        if (!paint) {
            return;
        }
        SetDCBrushColor(dc, color);
        FillRect(dc, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
    }

    void Frame(RECT rc, COLORREF color) const {
        if (!paint) {
            return;
        }
        SetDCBrushColor(dc, color);
        FrameRect(dc, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
    }

    SIZE Measure(HFONT font, std::wstring const& text, int spacing = 0) const {
        HGDIOBJ old = SelectObject(dc, font);
        SetTextCharacterExtra(dc, spacing);
        SIZE size{};
        GetTextExtentPoint32W(dc, text.c_str(), (int)text.size(), &size);
        SelectObject(dc, old);
        return size;
    }

    // Ellipsis is only for free text; tracked-out labels are sized to fit, and
    // DrawText's ellipsis logic doesn't account for the extra spacing.
    void Text(HFONT font, COLORREF color, RECT rc, std::wstring const& text,
              UINT format = DT_LEFT, int spacing = 0) const {
        if (!paint) {
            return;
        }
        HGDIOBJ old = SelectObject(dc, font);
        SetTextCharacterExtra(dc, spacing);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, color);
        DrawTextW(dc, text.c_str(), (int)text.size(), &rc,
                  format | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX |
                      (spacing ? DT_NOCLIP : DT_END_ELLIPSIS));
        SelectObject(dc, old);
    }

    // Centers tracked-out text itself: DrawText centers without counting the
    // extra spacing, which pushes the text right.
    void CenteredText(HFONT font, COLORREF color, RECT rc, std::wstring const& text,
                      int spacing) const {
        if (!paint) {
            return;
        }
        SIZE size = Measure(font, text, spacing);
        HGDIOBJ old = SelectObject(dc, font);
        SetTextCharacterExtra(dc, spacing);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, color);
        // The last character's spacing trails it; leave it out of the centering.
        int width = size.cx - spacing;
        TextOutW(dc, rc.left + (rc.right - rc.left - width) / 2,
                 rc.top + (rc.bottom - rc.top - size.cy) / 2, text.c_str(), (int)text.size());
        SelectObject(dc, old);
    }

    void Art(std::string_view id, int x, int top, int size) const {
        if (!paint) {
            return;
        }
        auto pixels = RenderArt(ArtFor(id), g_settings.tone, g_settings.shade, size);
        void* bits = nullptr;
        HBITMAP bitmap = CreateDib(size, size, &bits);
        if (!bitmap) {
            return;
        }
        // Every pixel is fully opaque or fully clear, so straight alpha is
        // already premultiplied.
        memcpy(bits, pixels.data(), pixels.size() * 4);
        HDC mem = CreateCompatibleDC(dc);
        HGDIOBJ old = SelectObject(mem, bitmap);
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        AlphaBlend(dc, x, top, size, size, mem, 0, 0, size, size, blend);
        SelectObject(mem, old);
        DeleteDC(mem);
        DeleteObject(bitmap);
    }

    // Premultiplied pixels at their own size, never scaled.
    void Pixels(std::vector<uint32_t> const& pixels, int size, int x, int top) const {
        if (!paint || !size || pixels.size() < (size_t)size * size) {
            return;
        }
        void* bits = nullptr;
        HBITMAP bitmap = CreateDib(size, size, &bits);
        if (!bitmap) {
            return;
        }
        memcpy(bits, pixels.data(), (size_t)size * size * 4);
        HDC mem = CreateCompatibleDC(dc);
        HGDIOBJ old = SelectObject(mem, bitmap);
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        AlphaBlend(dc, x, top, size, size, mem, 0, 0, size, size, blend);
        SelectObject(mem, old);
        DeleteDC(mem);
        DeleteObject(bitmap);
    }

    // Section label: uppercase, tracked out, in the given color.
    void Label(std::wstring text, COLORREF color) {
        CharUpperBuffW(text.data(), (DWORD)text.size());
        RECT rc{Px(15), y, width - Px(15), y + Px(14)};
        Text(g_flyout.label, color, rc, text, DT_LEFT, Px(2));
        y += Px(14);
    }

    void Divider() {
        y += Px(8);
        Fill({Px(15), y, width - Px(15), y + s}, ui::kLine);
        y += s + Px(8);
    }

    void Link(std::wstring const& text, int index, std::vector<Hit>& hits, int hover) {
        RECT rc{Px(15), y, width - Px(15), y + Px(20)};
        int id = MakeHit(kHitLink, index);
        Text(g_flyout.caption, hover == id ? ui::kCoral : ui::kGreen, rc, text);
        SIZE size = Measure(g_flyout.caption, text);
        hits.push_back({{rc.left, rc.top, rc.left + size.cx, rc.bottom}, id});
        y += Px(20);
    }
};

void Execute(std::function<void()> action);
void CloseFlyout();
void ShowFlyout(int flyoutKind, int iconKind);

// Network flyout

void LoadNetworkFlyout() {
    NetworkInfo info = QueryNetwork();
    g_flyout.connections.clear();
    g_flyout.networks.clear();
    g_flyout.wifiAdapter = info.wifiAdapter;
    g_flyout.wifiOff = info.wifiRadioOff;

    if (g_networkListManager) {
        IEnumNetworks* networks = nullptr;
        if (SUCCEEDED(g_networkListManager->GetNetworks(NLM_ENUM_NETWORK_CONNECTED, &networks)) &&
            networks) {
            INetwork* network = nullptr;
            ULONG fetched = 0;
            while (networks->Next(1, &network, &fetched) == S_OK && fetched) {
                BSTR name = nullptr;
                NLM_CONNECTIVITY flags = NLM_CONNECTIVITY_DISCONNECTED;
                network->GetName(&name);
                network->GetConnectivity(&flags);

                // Wi-Fi, wired or VPN, from the adapter behind the connection.
                bool wireless = false;
                bool virtualAdapter = false;
                IEnumNetworkConnections* connections = nullptr;
                if (SUCCEEDED(network->GetNetworkConnections(&connections)) && connections) {
                    INetworkConnection* connection = nullptr;
                    ULONG got = 0;
                    if (connections->Next(1, &connection, &got) == S_OK && got) {
                        GUID adapter{};
                        MIB_IF_ROW2 row{};
                        if (SUCCEEDED(connection->GetAdapterId(&adapter)) &&
                            ConvertInterfaceGuidToLuid(&adapter, &row.InterfaceLuid) == NO_ERROR &&
                            GetIfEntry2(&row) == NO_ERROR) {
                            wireless = row.Type == IF_TYPE_IEEE80211;
                            virtualAdapter = !row.InterfaceAndOperStatusFlags.HardwareInterface;
                        }
                        connection->Release();
                    }
                    connections->Release();
                }

                bool internet = flags & (NLM_CONNECTIVITY_IPV4_INTERNET |
                                         NLM_CONNECTIVITY_IPV6_INTERNET);
                Connection c;
                c.name = name ? name : L"Network";
                c.status = internet ? L"Internet access" : L"No internet access";
                c.art = virtualAdapter ? (internet ? "vpn-on" : "vpn")
                        : wireless     ? (internet ? "wifi-" + std::to_string(WifiBars(info.wifiQuality))
                                                   : "wifi-no-internet")
                                       : (internet ? "ethernet" : "ethernet-no-internet");
                g_flyout.connections.push_back(c);
                SysFreeString(name);
                network->Release();
            }
            networks->Release();
        }
    }

    if (!g_wlan) {
        return;
    }
    WLAN_INTERFACE_INFO_LIST* interfaces = nullptr;
    if (WlanEnumInterfaces(g_wlan, nullptr, &interfaces) != ERROR_SUCCESS) {
        return;
    }
    for (DWORD i = 0; i < interfaces->dwNumberOfItems; i++) {
        GUID iface = interfaces->InterfaceInfo[i].InterfaceGuid;
        WLAN_AVAILABLE_NETWORK_LIST* list = nullptr;
        if (WlanGetAvailableNetworkList(g_wlan, &iface, 0, nullptr, &list) != ERROR_SUCCESS) {
            continue;
        }
        for (DWORD n = 0; n < list->dwNumberOfItems; n++) {
            auto const& available = list->Network[n];
            if (!available.dot11Ssid.uSSIDLength) {
                continue;
            }
            WifiNetwork network;
            network.ssid = Utf8ToWide(std::string_view(
                (const char*)available.dot11Ssid.ucSSID, available.dot11Ssid.uSSIDLength));
            network.quality = (int)available.wlanSignalQuality;
            network.secured = available.bSecurityEnabled;
            network.connected = available.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED;
            network.profile = available.strProfileName;
            network.iface = iface;
            network.dot11 = available.dot11Ssid;
            network.bssType = available.dot11BssType;

            // One row per name, keeping the strongest or connected entry.
            auto existing = std::find_if(
                g_flyout.networks.begin(), g_flyout.networks.end(),
                [&](WifiNetwork const& w) { return w.ssid == network.ssid; });
            if (existing == g_flyout.networks.end()) {
                g_flyout.networks.push_back(network);
            } else if (network.connected ||
                       (!existing->connected && network.quality > existing->quality)) {
                bool keepProfile = existing->profile.size() && network.profile.empty();
                std::wstring profile = existing->profile;
                *existing = network;
                if (keepProfile) {
                    existing->profile = profile;
                }
            }
        }
        WlanFreeMemory(list);
    }
    WlanFreeMemory(interfaces);

    std::stable_sort(g_flyout.networks.begin(), g_flyout.networks.end(),
                     [](WifiNetwork const& a, WifiNetwork const& b) {
                         if (a.connected != b.connected) {
                             return a.connected;
                         }
                         return a.quality > b.quality;
                     });
    int maxScroll = std::max(0, (int)g_flyout.networks.size() - kMaxVisibleNetworks);
    g_flyout.networkScroll = std::min(g_flyout.networkScroll, maxScroll);
    g_flyout.selectedNetwork = -1;
    for (size_t i = 0; i < g_flyout.networks.size(); i++) {
        if (!g_flyout.selectedSsid.empty() && g_flyout.networks[i].ssid == g_flyout.selectedSsid) {
            g_flyout.selectedNetwork = (int)i;
        }
    }
}

void ScanWifi() {
    if (!g_wlan) {
        return;
    }
    WLAN_INTERFACE_INFO_LIST* interfaces = nullptr;
    if (WlanEnumInterfaces(g_wlan, nullptr, &interfaces) == ERROR_SUCCESS) {
        for (DWORD i = 0; i < interfaces->dwNumberOfItems; i++) {
            WlanScan(g_wlan, &interfaces->InterfaceInfo[i].InterfaceGuid, nullptr, nullptr, nullptr);
        }
        WlanFreeMemory(interfaces);
    }
}

void ConnectOrDisconnect(WifiNetwork const& network) {
    if (network.connected) {
        WlanDisconnect(g_wlan, &network.iface, nullptr);
        return;
    }
    WLAN_CONNECTION_PARAMETERS parameters{};
    DOT11_SSID ssid = network.dot11;
    parameters.dot11BssType = network.bssType;
    if (!network.profile.empty()) {
        parameters.wlanConnectionMode = wlan_connection_mode_profile;
        parameters.strProfile = network.profile.c_str();
    } else if (!network.secured) {
        parameters.wlanConnectionMode = wlan_connection_mode_discovery_unsecure;
        parameters.pDot11Ssid = &ssid;
    } else {
        // A new secured network needs a password, which Windows' own sign-in
        // asks for.
        CloseFlyout();
        OpenUri(L"ms-availablenetworks:");
        return;
    }
    WlanConnect(g_wlan, &network.iface, &parameters, nullptr);
}

void RenderNetwork(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    int s = c.s;

    c.y = c.Px(15);
    c.Label(L"Currently connected to:", ui::kSage);
    c.y += c.Px(6);
    if (f.connections.empty()) {
        c.Text(f.body, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"Not connected");
        c.y += c.Px(24);
    }
    for (auto const& connection : f.connections) {
        c.Art(connection.art, c.Px(15), c.y + c.Px(3), c.Px(26));
        int x = c.Px(15 + 26 + 10);
        c.Text(f.body, ui::kInk, {x, c.y, c.width - c.Px(15), c.y + c.Px(18)}, connection.name);
        c.Text(f.caption, ui::kSage, {x, c.y + c.Px(18), c.width - c.Px(15), c.y + c.Px(32)},
               connection.status);
        c.y += c.Px(36);
    }

    c.Divider();
    c.Label(L"Wi-Fi", ui::kGreen);
    c.y += c.Px(4);

    auto note = [&](PCWSTR text) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)}, text);
        c.y += c.Px(24);
    };
    if (!f.wifiAdapter) {
        note(L"No Wi-Fi adapter");
    } else if (f.wifiOff) {
        note(L"Wi-Fi is off");
    } else if (f.networks.empty()) {
        note(L"Looking for networks...");
    }

    int last = std::min((int)f.networks.size(), f.networkScroll + kMaxVisibleNetworks);
    for (int i = f.networkScroll; i < last; i++) {
        auto const& network = f.networks[i];
        bool selected = f.selectedNetwork == i;
        int rowHeight = c.Px(selected ? 66 : 38);
        RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + rowHeight};
        int rowId = MakeHit(kHitWifiRow, i);
        if (selected) {
            c.Fill(row, ui::kRaised);
            c.Frame(row, ui::kEdge);
        } else if (hover == rowId) {
            c.Fill(row, ui::kHover);
        }
        hits.push_back({row, rowId});

        int artX = c.width - c.Px(15 + 26);
        c.Art("wifi-" + std::to_string(WifiBars(network.quality)), artX, c.y + c.Px(6), c.Px(26));
        c.Text(f.body, ui::kInk, {c.Px(15), c.y + c.Px(2), artX - c.Px(8), c.y + c.Px(20)},
               network.ssid);
        std::wstring detail = network.connected ? L"Connected"
                              : network.secured ? L"Secured"
                                                : L"Open";
        c.Text(f.caption, network.connected ? ui::kCoral : ui::kSage,
               {c.Px(15), c.y + c.Px(20), artX - c.Px(8), c.y + c.Px(34)}, detail);

        if (selected) {
            std::wstring action = network.connected ? L"DISCONNECT" : L"CONNECT";
            SIZE size = c.Measure(f.label, action, c.Px(2));
            RECT button{c.width - c.Px(15) - size.cx - c.Px(24), c.y + c.Px(38),
                        c.width - c.Px(15), c.y + c.Px(60)};
            int buttonId = MakeHit(kHitWifiAction, i);
            c.Fill(button, hover == buttonId ? ui::kHover : ui::kGround);
            c.Frame(button, hover == buttonId ? ui::kGreen : ui::kEdge);
            c.CenteredText(f.label, hover == buttonId ? ui::kGreen : ui::kSage, button, action,
                           c.Px(2));
            hits.push_back({button, buttonId});
        }
        c.y += rowHeight + s;
    }
    if ((int)f.networks.size() > kMaxVisibleNetworks) {
        std::wstring more = std::to_wstring(f.networkScroll + 1) + L"-" + std::to_wstring(last) +
                            L" of " + std::to_wstring(f.networks.size()) + L", scroll for more";
        note(more.c_str());
    }

    c.Divider();
    c.Link(L"Open Network and Sharing Center", 0, hits, hover);
    c.Link(L"Network & internet settings", 1, hits, hover);
    c.y += c.Px(10);
}

// Power flyout

void LoadPowerFlyout() {
    g_flyout.battery = ComputeBattery();
    g_flyout.plans.clear();

    GUID* active = nullptr;
    PowerGetActiveScheme(nullptr, &active);
    for (ULONG index = 0;; index++) {
        GUID guid;
        DWORD size = sizeof(guid);
        if (PowerEnumerate(nullptr, nullptr, nullptr, ACCESS_SCHEME, index,
                           (UCHAR*)&guid, &size) != ERROR_SUCCESS) {
            break;
        }
        WCHAR name[256] = L"";
        DWORD nameSize = sizeof(name);
        PowerReadFriendlyName(nullptr, &guid, nullptr, nullptr, (UCHAR*)name, &nameSize);
        g_flyout.plans.push_back({guid, name, active && IsEqualGUID(guid, *active)});
    }
    if (active) {
        LocalFree(active);
    }
}

void RenderPower(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    c.Label(L"Battery", ui::kSage);
    c.y += c.Px(6);
    c.Art(f.battery.art, c.Px(15), c.y, c.Px(26));
    // One line per device when the batteries are Bluetooth devices'.
    int top = c.y;
    std::wstring_view tip = f.battery.tip;
    for (size_t at = 0;;) {
        size_t end = tip.find(L'\n', at);
        std::wstring line(tip.substr(at, end == std::wstring_view::npos ? std::wstring_view::npos : end - at));
        c.Text(f.body, ui::kInk, {c.Px(15 + 26 + 10), c.y, c.width - c.Px(15), c.y + c.Px(26)}, line);
        c.y += c.Px(26);
        if (end == std::wstring_view::npos) {
            break;
        }
        at = end + 1;
    }
    c.y = std::max(c.y, top + c.Px(26)) + c.Px(6);

    c.Divider();
    c.Label(L"Select a power plan:", ui::kGreen);
    c.y += c.Px(6);
    for (size_t i = 0; i < f.plans.size(); i++) {
        auto const& plan = f.plans[i];
        RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(26)};
        int id = MakeHit(kHitPlan, (int)i);
        if (hover == id) {
            c.Fill(row, ui::kHover);
        }
        RECT box{c.Px(15), c.y + c.Px(7), c.Px(27), c.y + c.Px(19)};
        c.Frame(box, plan.active ? ui::kGreen : ui::kEdge);
        if (plan.active) {
            c.Fill({box.left + c.Px(3), box.top + c.Px(3), box.right - c.Px(3), box.bottom - c.Px(3)},
                   ui::kGreen);
        }
        c.Text(f.body, plan.active ? ui::kInk : ui::kSage,
               {c.Px(35), c.y, c.width - c.Px(15), c.y + c.Px(26)}, plan.name);
        hits.push_back({row, id});
        c.y += c.Px(28);
    }

    c.Divider();
    c.Link(L"Adjust screen brightness", 0, hits, hover);
    c.Link(L"More power options", 1, hits, hover);
    c.y += c.Px(10);
}

// Brightness flyout

void StartBrightness() {
    std::erase_if(g_brightnessThreads, [](HANDLE thread) {
        if (WaitForSingleObject(thread, 0) != WAIT_OBJECT_0) {
            return false;
        }
        CloseHandle(thread);
        return true;
    });

    auto* shared = new BrightnessShared();
    shared->notify = g_flyout.hwnd;
    HANDLE thread = CreateThread(nullptr, 0, BrightnessThreadProc, shared, 0, nullptr);
    if (!thread) {
        delete shared;
        return;
    }
    g_brightnessThreads.push_back(thread);
    g_flyout.brightness = shared;
    g_flyout.monitorsLoaded = false;
    g_flyout.monitors.clear();
}

void StopBrightness() {
    auto* shared = g_flyout.brightness;
    if (!shared) {
        return;
    }
    EnterCriticalSection(&shared->lock);
    shared->stop = true;
    shared->notify = nullptr;
    LeaveCriticalSection(&shared->lock);
    SetEvent(shared->wake);
    shared->Release();
    g_flyout.brightness = nullptr;
}

// Reads the levels once at start, so even the first flyout opens with them.
BrightnessShared* g_brightnessWarm;

void WarmBrightnessCache(HWND notify) {
    auto* shared = new BrightnessShared();
    shared->notify = notify;
    HANDLE thread = CreateThread(nullptr, 0, BrightnessThreadProc, shared, 0, nullptr);
    if (!thread) {
        delete shared;
        return;
    }
    g_brightnessThreads.push_back(thread);
    g_brightnessWarm = shared;
}

void FinishBrightnessWarm() {
    auto* shared = g_brightnessWarm;
    if (!shared) {
        return;
    }
    EnterCriticalSection(&shared->lock);
    if (shared->loaded) {
        g_brightnessCache = shared->levels;
    }
    shared->stop = true;
    shared->notify = nullptr;
    LeaveCriticalSection(&shared->lock);
    SetEvent(shared->wake);
    shared->Release();
    g_brightnessWarm = nullptr;
}

void SetBrightness(int monitor, int value) {
    auto* shared = g_flyout.brightness;
    if (!shared || monitor < 0 || monitor >= (int)g_flyout.monitors.size() ||
        g_flyout.monitors[monitor].value < 0) {
        return;
    }
    value = std::clamp(value, 0, 100);
    g_flyout.monitors[monitor].value = value;
    if (monitor < (int)g_brightnessCache.size()) {
        g_brightnessCache[monitor].value = value;
    }
    EnterCriticalSection(&shared->lock);
    if (monitor < (int)shared->pending.size()) {
        shared->pending[monitor] = value;
    }
    LeaveCriticalSection(&shared->lock);
    SetEvent(shared->wake);
}

RECT SliderTrack(Hit const& hit, int s) {
    return {hit.rc.left + 10 * s, hit.rc.bottom - 16 * s, hit.rc.right - 10 * s,
            hit.rc.bottom - 8 * s};
}

void RenderBrightness(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    c.Label(L"Brightness", ui::kSage);
    c.y += c.Px(6);
    if (!f.monitorsLoaded) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"Asking the monitors...");
        c.y += c.Px(24);
    } else if (f.monitors.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"No monitors found");
        c.y += c.Px(24);
    }
    for (size_t i = 0; i < f.monitors.size(); i++) {
        auto const& monitor = f.monitors[i];
        RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(48)};
        int id = MakeHit(kHitSlider, (int)i);
        if (hover == id || f.dragSlider == (int)i) {
            c.Fill(row, ui::kHover);
        }
        std::wstring value = monitor.value >= 0 ? std::to_wstring(monitor.value) + L"%" : L"";
        c.Text(f.body, ui::kInk, {c.Px(15), c.y + c.Px(2), c.width - c.Px(60), c.y + c.Px(20)},
               monitor.name);
        c.Text(f.label, ui::kCoral, {c.width - c.Px(60), c.y + c.Px(2), c.width - c.Px(15), c.y + c.Px(20)},
               value, DT_RIGHT);
        Hit hit{row, id};
        RECT track = SliderTrack(hit, c.s);
        if (monitor.value >= 0) {
            c.Fill(track, ui::kRaised);
            c.Frame(track, ui::kEdge);
            int filled = track.left + (track.right - track.left) * monitor.value / 100;
            c.Fill({track.left + c.s, track.top + c.s, std::max<LONG>(track.left + c.s, filled),
                    track.bottom - c.s},
                   ui::kGreen);
            c.Fill({filled - c.Px(2), track.top - c.Px(4), filled + c.Px(2), track.bottom + c.Px(4)},
                   ui::kInk);
            hits.push_back(hit);
        } else {
            c.Text(f.caption, ui::kSageAlt, {track.left, track.top - c.Px(6), track.right, track.bottom + c.Px(2)},
                   L"Doesn't answer DDC/CI brightness");
        }
        c.y += c.Px(50);
    }

    c.Divider();
    c.Link(L"Night light settings", 0, hits, hover);
    c.Link(L"Display settings", 1, hits, hover);
    c.y += c.Px(10);
}

// Hidden icons flyout: the icons the user hid, as a grid like the Windows 7
// overflow, with the hovered icon's tooltip under it.

constexpr int kOverflowColumns = 6;

int OverflowCell() {
    int icon = 16 * g_flyout.scale;
    for (auto const& hidden : g_flyout.hidden) {
        icon = std::max(icon, hidden.size);
    }
    return std::max(30 * g_flyout.scale, icon + 12 * g_flyout.scale);
}

int OverflowColumns() {
    return std::clamp((int)g_flyout.hidden.size(), 1, kOverflowColumns);
}

// The order the hidden icons were dragged into, by program and icon id.
constexpr WCHAR kOrderKey[] = L"Software\\TourneTray";
constexpr WCHAR kOrderValue[] = L"OverflowOrder";

std::wstring HiddenIconKey(HiddenIcon const& icon) {
    return OwnerFileName(icon.owner) + L"|" + std::to_wstring(icon.id);
}

std::vector<std::wstring> LoadOverflowOrder() {
    std::vector<std::wstring> keys;
    DWORD size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kOrderKey, kOrderValue, RRF_RT_REG_MULTI_SZ, nullptr,
                     nullptr, &size) != ERROR_SUCCESS ||
        size < sizeof(WCHAR)) {
        return keys;
    }
    std::wstring buffer(size / sizeof(WCHAR) + 2, L'\0');
    size = (DWORD)(buffer.size() * sizeof(WCHAR));
    if (RegGetValueW(HKEY_CURRENT_USER, kOrderKey, kOrderValue, RRF_RT_REG_MULTI_SZ, nullptr,
                     buffer.data(), &size) != ERROR_SUCCESS) {
        return keys;
    }
    const WCHAR* end = buffer.data() + buffer.size();
    for (const WCHAR* p = buffer.data(); p < end && *p; p += wcslen(p) + 1) {
        keys.push_back(p);
    }
    return keys;
}

void SaveOverflowOrder() {
    std::vector<std::wstring> keys;
    for (auto const& icon : g_flyout.hidden) {
        keys.push_back(HiddenIconKey(icon));
    }
    // Icons that aren't hidden right now keep their place for when they are.
    for (auto const& key : LoadOverflowOrder()) {
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
            keys.push_back(key);
        }
    }
    std::wstring data;
    for (auto const& key : keys) {
        data += key;
        data.push_back(L'\0');
    }
    data.push_back(L'\0');
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kOrderKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key,
                        nullptr) == ERROR_SUCCESS) {
        RegSetValueExW(key, kOrderValue, 0, REG_MULTI_SZ, (const BYTE*)data.data(),
                       (DWORD)(data.size() * sizeof(WCHAR)));
        RegCloseKey(key);
    }
}

// Dragged icons first, in their order; the rest after them in tray order.
void ApplyOverflowOrder() {
    auto order = LoadOverflowOrder();
    if (order.empty()) {
        return;
    }
    auto& hidden = g_flyout.hidden;
    std::vector<std::pair<size_t, size_t>> ranks;  // Place in the order, index.
    for (size_t i = 0; i < hidden.size(); i++) {
        auto it = std::find(order.begin(), order.end(), HiddenIconKey(hidden[i]));
        ranks.push_back({(size_t)(it - order.begin()), i});
    }
    std::stable_sort(ranks.begin(), ranks.end(),
                     [](auto const& a, auto const& b) { return a.first < b.first; });
    std::vector<HiddenIcon> sorted;
    for (auto const& [rank, i] : ranks) {
        sorted.push_back(std::move(hidden[i]));
    }
    hidden = std::move(sorted);
}
void LoadOverflowFlyout() {
    g_flyout.hidden.clear();
    RequestHiddenIcons(&g_flyout.hidden);
    // The stand-in itself never belongs in its own flyout.
    std::erase_if(g_flyout.hidden, [](HiddenIcon const& icon) {
        return icon.owner == g_wnd && icon.id == kIconIdBase + kOverflow;
    });
    ApplyOverflowOrder();
    int s = g_flyout.scale;
    g_flyout.width = std::max(220 * s, 20 * s + OverflowColumns() * OverflowCell());
}

void RenderOverflow(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    c.Label(L"Hidden icons", ui::kSage);
    c.y += c.Px(8);

    if (f.hidden.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"No hidden icons");
        c.y += c.Px(24);
    } else {
        int cell = OverflowCell();
        int columns = OverflowColumns();
        int rows = ((int)f.hidden.size() + columns - 1) / columns;
        int left = (c.width - columns * cell) / 2;
        int hovered = -1;
        f.overflowLeft = left;
        f.overflowTop = c.y;
        f.overflowCell = cell;
        // While an icon is dragged, the others make room where it would land.
        std::vector<int> order;
        for (int i = 0; i < (int)f.hidden.size(); i++) {
            order.push_back(i);
        }
        bool dragging = f.dragHidden >= 0 && f.dragMoved;
        if (dragging && f.dragTarget >= 0) {
            order.erase(order.begin() + f.dragHidden);
            order.insert(order.begin() + f.dragTarget, f.dragHidden);
        }
        for (int slot = 0; slot < (int)order.size(); slot++) {
            int i = order[slot];
            auto const& icon = f.hidden[i];
            RECT rc{left + (slot % columns) * cell, c.y + (slot / columns) * cell,
                    left + (slot % columns + 1) * cell, c.y + (slot / columns + 1) * cell};
            int id = MakeHit(kHitHidden, i);
            if (dragging ? i == f.dragHidden : hover == id) {
                hovered = i;
                c.Fill(rc, ui::kHover);
                c.Frame(rc, ui::kEdge);
            }
            if (icon.size) {
                c.Pixels(icon.pixels, icon.size, rc.left + (cell - icon.size) / 2,
                         rc.top + (cell - icon.size) / 2);
            }
            hits.push_back({rc, id});
        }
        c.y += rows * cell + c.Px(6);

        std::wstring line;
        if (hovered >= 0) {
            line = f.hidden[hovered].tip.empty() ? L"(no tooltip)" : f.hidden[hovered].tip;
        } else {
            size_t count = f.hidden.size();
            line = std::to_wstring(count) + (count == 1 ? L" hidden icon" : L" hidden icons");
        }
        c.Text(f.caption, hovered >= 0 ? ui::kInk : ui::kSageAlt,
               {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)}, line);
        c.y += c.Px(20);
    }

    c.Divider();
    c.Link(L"Customize...", 0, hits, hover);
    c.y += c.Px(10);
}

// StartAllBack's own list of tray icons when it's installed, else Windows'.
void OpenTrayCustomization() {
    WCHAR path[MAX_PATH];
    if (ExpandEnvironmentStringsW(L"%ProgramFiles%\\StartAllBack\\StartAllBackCfg.exe", path,
                                  ARRAYSIZE(path)) &&
        GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
        Run(path);
    } else {
        OpenUri(L"ms-settings:taskbar");
    }
}

// The slot under a point in the hidden icons grid, or -1 outside it.
int HiddenSlotAt(POINT pt) {
    auto const& f = g_flyout;
    int count = (int)f.hidden.size();
    if (!count || f.overflowCell <= 0 || pt.x < f.overflowLeft || pt.y < f.overflowTop) {
        return -1;
    }
    int columns = OverflowColumns();
    int rows = (count + columns - 1) / columns;
    int column = (pt.x - f.overflowLeft) / f.overflowCell;
    int row = (pt.y - f.overflowTop) / f.overflowCell;
    if (column >= columns || row >= rows) {
        return -1;
    }
    return std::min(row * columns + column, count - 1);
}

void MoveHiddenIcon(int from, int to) {
    auto& hidden = g_flyout.hidden;
    HiddenIcon icon = std::move(hidden[from]);
    hidden.erase(hidden.begin() + from);
    hidden.insert(hidden.begin() + to, std::move(icon));
    SaveOverflowOrder();
}
void ClickHiddenIcon(int index, int button) {
    if (index < 0 || index >= (int)g_flyout.hidden.size()) {
        return;
    }
    HiddenIcon icon = g_flyout.hidden[index];
    if (button == kClickLeft && OpensOnDoubleClick(icon.owner)) {
        button = kClickDouble;
    }
    StartSticky(icon.owner);
    ForwardHiddenClick(icon, button);
}

// Slow work for a flyout (a command line, a Windows Update search) runs on a
// thread of its own; the result is posted back if the flyout is still open.

struct FlyoutJob {
    std::function<std::wstring()> work;
    CRITICAL_SECTION lock;
    HWND notify = nullptr;  // Guarded by lock.
    std::wstring result;    // Guarded by lock.
    bool done = false;      // Guarded by lock.
    LONG refs = 2;

    FlyoutJob() { InitializeCriticalSection(&lock); }
    ~FlyoutJob() { DeleteCriticalSection(&lock); }
    void Release() {
        if (InterlockedDecrement(&refs) == 0) {
            delete this;
        }
    }
};

DWORD WINAPI FlyoutJobProc(LPVOID parameter) {
    auto* job = (FlyoutJob*)parameter;
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    std::wstring result = job->work();
    CoUninitialize();
    EnterCriticalSection(&job->lock);
    job->result = std::move(result);
    job->done = true;
    HWND notify = job->notify;
    LeaveCriticalSection(&job->lock);
    if (notify) {
        PostMessageW(notify, WM_APP_JOB_DONE, 0, 0);
    }
    job->Release();
    return 0;
}

FlyoutJob* StartJob(std::function<std::wstring()> work) {
    auto* job = new FlyoutJob();
    job->work = std::move(work);
    job->notify = g_flyout.hwnd;
    HANDLE thread = CreateThread(nullptr, 0, FlyoutJobProc, job, 0, nullptr);
    if (!thread) {
        delete job;
        return nullptr;
    }
    // Waited for when the mod unloads, like the brightness workers.
    g_brightnessThreads.push_back(thread);
    return job;
}

void CancelJob(FlyoutJob*& job) {
    if (!job) {
        return;
    }
    EnterCriticalSection(&job->lock);
    job->notify = nullptr;
    LeaveCriticalSection(&job->lock);
    job->Release();
    job = nullptr;
}

// The job's result once it's done, and lets go of it.
bool TakeJobResult(FlyoutJob*& job, std::wstring* result) {
    if (!job) {
        return false;
    }
    EnterCriticalSection(&job->lock);
    bool done = job->done;
    if (done) {
        *result = job->result;
    }
    LeaveCriticalSection(&job->lock);
    if (done) {
        CancelJob(job);
    }
    return done;
}

// Runs a command line without a window and returns what it printed.
std::wstring RunCapture(std::wstring commandLine) {
    SECURITY_ATTRIBUTES attributes{sizeof(attributes), nullptr, TRUE};
    HANDLE read = nullptr, write = nullptr;
    if (!CreatePipe(&read, &write, &attributes, 0)) {
        return {};
    }
    SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW startup{sizeof(startup)};
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = write;
    startup.hStdError = write;
    PROCESS_INFORMATION process;
    if (!CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &startup, &process)) {
        CloseHandle(read);
        CloseHandle(write);
        return {};
    }
    CloseHandle(write);
    std::string output;
    char buffer[4096];
    DWORD count = 0;
    // Ends when the program exits and the pipe closes.
    while (ReadFile(read, buffer, sizeof(buffer), &count, nullptr) && count) {
        output.append(buffer, count);
    }
    WaitForSingleObject(process.hProcess, 5000);
    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);
    CloseHandle(read);
    return Utf8ToWide(output);
}

// Windows Security flyout

std::wstring FormatDateShort(SYSTEMTIME const& time) {
    WCHAR text[64];
    return GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &time, L"MMM d", text, ARRAYSIZE(text), nullptr)
               ? text
               : L"";
}

std::wstring LastScanText() {
    BYTE data[8];
    DWORD size = sizeof(data);
    if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows Defender\\Scan",
                     L"LastScanRun", RRF_RT_REG_BINARY, nullptr, data, &size) != ERROR_SUCCESS ||
        size != 8) {
        return L"No scan yet";
    }
    FILETIME utc, local;
    memcpy(&utc, data, 8);
    SYSTEMTIME scan, now;
    FileTimeToLocalFileTime(&utc, &local);
    FileTimeToSystemTime(&local, &scan);
    GetLocalTime(&now);
    WCHAR text[64];
    bool today = scan.wYear == now.wYear && scan.wMonth == now.wMonth && scan.wDay == now.wDay;
    std::wstring date = today ? L"" : FormatDateShort(scan) + L" ";
    swprintf_s(text, L"Last scan %ls%02u:%02u", date.c_str(), scan.wHour, scan.wMinute);
    return text;
}

// Pending updates, from Windows Update's cached results (no online search).
std::wstring SearchPendingUpdates() {
    DWORD ignored = 0;
    if (RegGetValueW(HKEY_LOCAL_MACHINE,
                     L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired",
                     nullptr, RRF_RT_ANY, nullptr, nullptr, &ignored) == ERROR_SUCCESS) {
        return L"restart";
    }
    constexpr GUID kClsidUpdateSession = {
        0x4CB43D7F, 0x7EEE, 0x4906, {0x86, 0x98, 0x60, 0xDA, 0x1C, 0x38, 0xF2, 0xFE}};
    constexpr GUID kIidUpdateSession = {
        0x816858A4, 0x260D, 0x4260, {0x93, 0x3A, 0x25, 0x85, 0xF1, 0xAB, 0xC7, 0x6B}};
    IUpdateSession* session = nullptr;
    if (FAILED(CoCreateInstance(kClsidUpdateSession, nullptr, CLSCTX_INPROC_SERVER, kIidUpdateSession,
                                (void**)&session))) {
        return {};
    }
    std::wstring result;
    IUpdateSearcher* searcher = nullptr;
    if (SUCCEEDED(session->CreateUpdateSearcher(&searcher))) {
        searcher->put_Online(VARIANT_FALSE);
        ISearchResult* search = nullptr;
        BSTR criteria = SysAllocString(L"IsInstalled=0 and IsHidden=0");
        if (SUCCEEDED(searcher->Search(criteria, &search))) {
            IUpdateCollection* updates = nullptr;
            LONG count = 0;
            if (SUCCEEDED(search->get_Updates(&updates))) {
                updates->get_Count(&count);
                updates->Release();
                result = std::to_wstring(count);
            }
            search->Release();
        }
        SysFreeString(criteria);
        searcher->Release();
    }
    session->Release();
    return result;
}

void FillUpdatesRow(SecurityRow& row, std::wstring const& result) {
    if (result == L"restart") {
        row.art = "restart-required";
        row.value = L"Restart";
    } else if (result.empty()) {
        row.art = "update";
        row.value = L"...";  // Still searching.
    } else if (result == L"unknown") {
        row.art = "update";
        row.value = L"Unknown";
    } else if (result == L"0") {
        row.art = "update";
        row.value = L"Up to date";
    } else {
        row.art = "update-available";
        row.value = result + L" pending";
    }
}

void LoadSecurityFlyout() {
    auto& f = g_flyout;
    auto antivirus = ProviderHealth(WSC_SECURITY_PROVIDER_ANTIVIRUS);
    auto firewall = ProviderHealth(WSC_SECURITY_PROVIDER_FIREWALL);
    auto service = ProviderHealth(WSC_SECURITY_PROVIDER_SERVICE);

    f.securityScan = LastScanText();
    f.securityRows.clear();
    f.securityRows.push_back(
        {antivirus == WSC_SECURITY_PROVIDER_HEALTH_GOOD     ? "security-ok"
         : antivirus == WSC_SECURITY_PROVIDER_HEALTH_POOR   ? "security-alert"
         : antivirus == WSC_SECURITY_PROVIDER_HEALTH_SNOOZE ? "security-warn"
                                                            : "security-off",
         L"Virus & threat",
         antivirus == WSC_SECURITY_PROVIDER_HEALTH_GOOD     ? L"OK"
         : antivirus == WSC_SECURITY_PROVIDER_HEALTH_POOR   ? L"Action needed"
         : antivirus == WSC_SECURITY_PROVIDER_HEALTH_SNOOZE ? L"Snoozed"
                                                            : L"OFF",
         L"windowsdefender://threat"});
    bool firewallOn = firewall == WSC_SECURITY_PROVIDER_HEALTH_GOOD;
    f.securityRows.push_back({firewallOn ? "firewall-on" : "firewall-off", L"Firewall",
                              firewallOn ? L"ON" : L"OFF", L"windowsdefender://network"});
    SecurityRow updates{"update", L"Updates", L"...", L"ms-settings:windowsupdate"};
    FillUpdatesRow(updates, f.securityUpdates);
    f.securityRows.push_back(updates);

    ULARGE_INTEGER free{}, total{};
    WCHAR system[MAX_PATH] = L"C:\\";
    if (GetWindowsDirectoryW(system, ARRAYSIZE(system)) >= 3) {
        system[3] = L'\0';
    }
    int used = GetDiskFreeSpaceExW(system, nullptr, &total, &free) && total.QuadPart
                   ? (int)(100 - free.QuadPart * 100 / total.QuadPart)
                   : -1;
    f.securityRows.push_back({used >= 90 ? "health-warn" : "health-ok", L"Storage",
                              used >= 0 ? std::to_wstring(used) + L"%" : L"...",
                              L"ms-settings:storagesense"});
    bool healthy = service == WSC_SECURITY_PROVIDER_HEALTH_GOOD;
    f.securityRows.push_back({healthy ? "health-ok" : "health-alert", L"Device health",
                              healthy ? L"OK" : L"Check", L"windowsdefender://perfcenter"});
}

void RenderSecurity(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    RECT title{c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(22)};
    c.Text(f.body, ui::kInk, title, L"WINDOWS SECURITY", DT_LEFT, c.Px(2));
    c.y += c.Px(24);
    c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)}, f.securityScan);
    c.y += c.Px(26);

    for (size_t i = 0; i < f.securityRows.size(); i++) {
        auto const& row = f.securityRows[i];
        RECT rc{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(36)};
        int id = MakeHit(kHitSecurityRow, (int)i);
        if (hover == id) {
            c.Fill(rc, ui::kHover);
        }
        c.Art(row.art, c.Px(15), c.y + c.Px(6), c.Px(24));
        std::wstring label = row.label;
        CharUpperBuffW(label.data(), (DWORD)label.size());
        c.Text(f.label, ui::kSage, {c.Px(52), c.y, c.width - c.Px(110), c.y + c.Px(36)}, label, DT_LEFT,
               c.Px(2));
        c.Text(f.caption, ui::kCoral, {c.width - c.Px(110), c.y, c.width - c.Px(15), c.y + c.Px(36)},
               row.value, DT_RIGHT);
        hits.push_back({rc, id});
        c.y += c.Px(36);
    }

    c.Divider();
    c.Link(L"Open Windows Security", 0, hits, hover);
    c.y += c.Px(10);
}

// VPN flyout: Windscribe's locations, connected through its command line so
// its own window stays closed.

// Just enough JSON for Windscribe's location list.
struct Json {
    enum Type { kNull, kBool, kNumber, kString, kArray, kObject } type = kNull;
    double number = 0;
    std::wstring text;
    std::vector<Json> items;
    std::vector<std::pair<std::wstring, Json>> members;

    Json const* Get(std::wstring_view key) const {
        for (auto const& [name, value] : members) {
            if (name == key) {
                return &value;
            }
        }
        return nullptr;
    }
    std::wstring String(std::wstring_view key) const {
        auto* value = Get(key);
        return value && value->type == kString ? value->text : L"";
    }
    double Number(std::wstring_view key) const {
        auto* value = Get(key);
        return value && value->type == kNumber ? value->number : 0;
    }
};

struct JsonParser {
    std::wstring_view s;
    size_t at = 0;
    int depth = 0;

    void Space() {
        while (at < s.size() && iswspace(s[at])) {
            at++;
        }
    }

    bool Parse(Json& out) {
        Space();
        if (at >= s.size() || depth > 64) {
            return false;
        }
        depth++;
        bool ok = Value(out);
        depth--;
        return ok;
    }

    bool Value(Json& out) {
        wchar_t c = s[at];
        if (c == L'{' || c == L'[') {
            bool object = c == L'{';
            out.type = object ? Json::kObject : Json::kArray;
            at++;
            Space();
            if (at < s.size() && s[at] == (object ? L'}' : L']')) {
                at++;
                return true;
            }
            while (true) {
                std::wstring key;
                if (object) {
                    Space();
                    if (at >= s.size() || s[at] != L'"' || !String(key)) {
                        return false;
                    }
                    Space();
                    if (at >= s.size() || s[at] != L':') {
                        return false;
                    }
                    at++;
                }
                Json value;
                if (!Parse(value)) {
                    return false;
                }
                if (object) {
                    out.members.emplace_back(std::move(key), std::move(value));
                } else {
                    out.items.push_back(std::move(value));
                }
                Space();
                if (at < s.size() && s[at] == L',') {
                    at++;
                    continue;
                }
                if (at < s.size() && s[at] == (object ? L'}' : L']')) {
                    at++;
                    return true;
                }
                return false;
            }
        }
        if (c == L'"') {
            out.type = Json::kString;
            return String(out.text);
        }
        for (auto [word, type] : {std::pair{L"true", Json::kBool}, std::pair{L"false", Json::kBool},
                                  std::pair{L"null", Json::kNull}}) {
            std::wstring_view literal = word;
            if (s.substr(at, literal.size()) == literal) {
                out.type = type;
                out.number = literal == L"true";
                at += literal.size();
                return true;
            }
        }
        size_t start = at;
        while (at < s.size() && (iswdigit(s[at]) || s[at] == L'-' || s[at] == L'+' ||
                                 s[at] == L'.' || s[at] == L'e' || s[at] == L'E')) {
            at++;
        }
        if (at == start) {
            return false;
        }
        out.type = Json::kNumber;
        out.number = wcstod(std::wstring(s.substr(start, at - start)).c_str(), nullptr);
        return true;
    }

    bool String(std::wstring& out) {
        at++;  // The opening quote.
        while (at < s.size()) {
            wchar_t c = s[at++];
            if (c == L'"') {
                return true;
            }
            if (c != L'\\') {
                out += c;
                continue;
            }
            if (at >= s.size()) {
                return false;
            }
            wchar_t escaped = s[at++];
            switch (escaped) {
                case L'n': out += L'\n'; break;
                case L't': out += L'\t'; break;
                case L'r': out += L'\r'; break;
                case L'b':
                case L'f': break;
                case L'u':
                    if (at + 4 > s.size()) {
                        return false;
                    }
                    out += (wchar_t)wcstoul(std::wstring(s.substr(at, 4)).c_str(), nullptr, 16);
                    at += 4;
                    break;
                default: out += escaped;
            }
        }
        return false;
    }
};

std::wstring WindscribeFile(PCWSTR name) {
    WCHAR path[MAX_PATH];
    std::wstring pattern = std::wstring(L"%ProgramFiles%\\Windscribe\\") + name;
    if (ExpandEnvironmentStringsW(pattern.c_str(), path, ARRAYSIZE(path)) &&
        GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
        return path;
    }
    return {};
}

// Windscribe keeps its location list as JSON in its settings.
std::vector<VpnRegion> LoadWindscribeLocations() {
    std::vector<VpnRegion> regions;
    PCWSTR key = L"Software\\Windscribe\\Windscribe2";
    DWORD size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, key, L"wsnetSettings", RRF_RT_REG_SZ, nullptr, nullptr,
                     &size) != ERROR_SUCCESS ||
        !size) {
        return regions;
    }
    std::wstring text(size / sizeof(WCHAR), L'\0');
    if (RegGetValueW(HKEY_CURRENT_USER, key, L"wsnetSettings", RRF_RT_REG_SZ, nullptr, text.data(),
                     &size) != ERROR_SUCCESS) {
        return regions;
    }
    text.resize(wcsnlen(text.c_str(), text.size()));

    Json settings;
    JsonParser outer{text};
    if (!outer.Parse(settings)) {
        return regions;
    }
    std::wstring inventory = settings.String(L"invLocations");
    Json locations;
    JsonParser inner{inventory};
    if (!inner.Parse(locations)) {
        return regions;
    }
    auto* data = locations.Get(L"data");
    auto* list = data ? data->Get(L"locations") : nullptr;
    if (!list || list->type != Json::kArray) {
        return regions;
    }
    for (auto const& location : list->items) {
        VpnRegion region{location.String(L"name"), location.String(L"country_code"), {}};
        if (auto* datacenters = location.Get(L"datacenters");
            datacenters && datacenters->type == Json::kArray) {
            for (auto const& datacenter : datacenters->items) {
                if (datacenter.Number(L"status") == 1) {
                    region.cities.push_back({datacenter.String(L"city"), datacenter.String(L"nick")});
                }
            }
        }
        if (!region.name.empty() && !region.cities.empty()) {
            regions.push_back(std::move(region));
        }
    }
    return regions;
}

constexpr int kMaxVisibleVpnRows = 9;

// The list as shown: each region, and under the open one its cities.
struct VpnRow {
    int region;
    int city;  // -1 for the region itself.
};

std::vector<VpnRow> VpnRows() {
    std::vector<VpnRow> rows;
    for (int r = 0; r < (int)g_flyout.vpnRegions.size(); r++) {
        rows.push_back({r, -1});
        if (r == g_flyout.vpnOpenRegion) {
            for (int c = 0; c < (int)g_flyout.vpnRegions[r].cities.size(); c++) {
                rows.push_back({r, c});
            }
        }
    }
    return rows;
}

// The last status Windscribe gave, so the flyout opens with it.
std::wstring g_vpnLastStatus;
bool g_vpnLastConnected;

void ApplyVpnStatus(std::wstring const& output) {
    auto& f = g_flyout;
    f.vpnBusy = false;
    f.vpnChecking = false;
    f.vpnConnected = false;
    f.vpnStatus = L"Windscribe isn't answering";
    size_t at = output.find(L"Connect state:");
    if (at != std::wstring::npos) {
        size_t start = output.find_first_not_of(L" \t", at + 14);
        size_t end = output.find_first_of(L"\r\n", start);
        f.vpnStatus = output.substr(start, end == std::wstring::npos ? std::wstring::npos : end - start);
        f.vpnConnected = f.vpnStatus.starts_with(L"Connected");
    }
    g_vpnLastStatus = f.vpnStatus;
    g_vpnLastConnected = f.vpnConnected;
}

void RunVpnCommand(std::wstring const& arguments, std::wstring const& busyText) {
    auto& f = g_flyout;
    std::wstring cli = WindscribeFile(L"windscribe-cli.exe");
    if (cli.empty()) {
        return;
    }
    CancelJob(f.job);
    // A plain status check keeps showing the last status while it runs.
    if (arguments.empty()) {
        f.vpnChecking = true;
    } else {
        f.vpnBusy = true;
        f.vpnStatus = busyText;
    }
    std::wstring quoted = L"\"" + cli + L"\"";
    f.job = StartJob([quoted, arguments] {
        if (!arguments.empty()) {
            RunCapture(quoted + L" " + arguments);
        }
        return RunCapture(quoted + L" status");
    });
}

void LoadVpnFlyout() {
    auto& f = g_flyout;
    if (f.vpnRegions.empty()) {
        f.vpnRegions = LoadWindscribeLocations();
    }
}

void RenderVpn(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    c.Label(L"Windscribe", ui::kSage);
    c.y += c.Px(6);
    c.Art(f.vpnConnected ? "vpn-on" : "vpn", c.Px(15), c.y, c.Px(26));
    c.Text(f.body, ui::kInk, {c.Px(51), c.y, c.width - c.Px(15), c.y + c.Px(26)}, f.vpnStatus);
    c.y += c.Px(32);

    RECT action{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(26)};
    int actionId = MakeHit(kHitVpnAction, 0);
    // Always there, so the list never jumps; dimmed while Windscribe works.
    if (!f.vpnBusy && hover == actionId) {
        c.Fill(action, ui::kHover);
    }
    c.Text(f.body, f.vpnBusy ? ui::kSageAlt : hover == actionId ? ui::kCoral : ui::kGreen,
           {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(26)},
           f.vpnConnected ? L"Disconnect" : L"Connect to the best location");
    if (!f.vpnBusy) {
        hits.push_back({action, actionId});
    }
    c.y += c.Px(26);

    c.Divider();
    c.Label(L"Locations", ui::kGreen);
    c.y += c.Px(4);
    auto rows = VpnRows();
    if (rows.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"No locations saved by Windscribe yet");
        c.y += c.Px(24);
    }
    int first = std::clamp(f.vpnScroll, 0, std::max(0, (int)rows.size() - kMaxVisibleVpnRows));
    int last = std::min((int)rows.size(), first + kMaxVisibleVpnRows);
    for (int i = first; i < last; i++) {
        auto const& row = rows[i];
        auto const& region = f.vpnRegions[row.region];
        RECT rc{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(24)};
        int id = MakeHit(kHitVpnRow, i);
        if (hover == id) {
            c.Fill(rc, ui::kHover);
        }
        if (row.city < 0) {
            bool open = row.region == f.vpnOpenRegion;
            c.Text(f.body, open ? ui::kGreen : ui::kInk,
                   {c.Px(15), c.y, c.width - c.Px(50), c.y + c.Px(24)}, region.name);
            c.Text(f.label, ui::kSageAlt, {c.width - c.Px(50), c.y, c.width - c.Px(15), c.y + c.Px(24)},
                   region.country, DT_RIGHT, c.Px(1));
        } else {
            auto const& city = region.cities[row.city];
            c.Text(f.body, ui::kSage, {c.Px(30), c.y, c.width - c.Px(15), c.y + c.Px(24)}, city.city);
            SIZE size = c.Measure(f.body, city.city);
            c.Text(f.caption, ui::kCoral,
                   {c.Px(30) + size.cx + c.Px(10), c.y, c.width - c.Px(15), c.y + c.Px(24)}, city.nick);
        }
        hits.push_back({rc, id});
        c.y += c.Px(24);
    }
    if ((int)rows.size() > kMaxVisibleVpnRows) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               std::to_wstring(first + 1) + L"-" + std::to_wstring(last) + L" of " +
                   std::to_wstring(rows.size()) + L", scroll for more");
        c.y += c.Px(20);
    }

    c.Divider();
    c.Link(L"Open Windscribe", 0, hits, hover);
    c.Link(L"VPN settings", 1, hits, hover);
    c.y += c.Px(10);
}

void OnVpnRowClick(int index) {
    auto& f = g_flyout;
    auto rows = VpnRows();
    if (index < 0 || index >= (int)rows.size() || f.vpnBusy) {
        return;
    }
    auto const& row = rows[index];
    if (row.city < 0) {
        f.vpnOpenRegion = f.vpnOpenRegion == row.region ? -1 : row.region;
        return;
    }
    auto const& city = f.vpnRegions[row.region].cities[row.city];
    std::wstring target = city.nick.empty() ? city.city : city.nick;
    RunVpnCommand(L"connect \"" + target + L"\"", L"Connecting to " + city.city + L"...");
}

// Volume mixer flyout: the output device and every program playing through
// it, each with a slider and a mute button, like the Windows 7 mixer.

constexpr GUID kIidAudioSessionManager2 = {
    0x77AA99A0, 0x1BD6, 0x484F, {0x8B, 0xC7, 0x2C, 0x65, 0x4C, 0x9A, 0x9B, 0x6F}};
constexpr GUID kIidAudioSessionControl2 = {
    0xBFB7FF88, 0x7239, 0x4FC9, {0x8F, 0xA2, 0x07, 0xC9, 0x50, 0xBE, 0x9C, 0x6D}};
constexpr GUID kIidSimpleAudioVolume = {
    0x87CE5498, 0x68D6, 0x44E5, {0x92, 0x15, 0x6D, 0xA4, 0x7E, 0xF8, 0x83, 0xD8}};

constexpr UINT_PTR kTimerMixer = 3;
constexpr UINT kMixerRefreshMs = 1500;

std::wstring ProcessPath(DWORD pid) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        return {};
    }
    WCHAR path[MAX_PATH * 2];
    DWORD size = ARRAYSIZE(path);
    std::wstring result = QueryFullProcessImageNameW(process, 0, path, &size) ? path : L"";
    CloseHandle(process);
    return result;
}

// The name a program gives itself: its file description, like the mixer.
std::wstring FileDescription(std::wstring const& path) {
    DWORD ignored = 0;
    DWORD size = GetFileVersionInfoSizeW(path.c_str(), &ignored);
    if (size) {
        std::vector<BYTE> data(size);
        struct Translation {
            WORD language;
            WORD codePage;
        }* translations = nullptr;
        UINT length = 0;
        if (GetFileVersionInfoW(path.c_str(), 0, size, data.data()) &&
            VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (void**)&translations,
                           &length) &&
            length >= sizeof(Translation)) {
            WCHAR query[64];
            swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\FileDescription",
                       translations[0].language, translations[0].codePage);
            WCHAR* description = nullptr;
            UINT chars = 0;
            if (VerQueryValueW(data.data(), query, (void**)&description, &chars) && chars > 1) {
                return description;
            }
        }
    }
    std::wstring name = path.substr(path.find_last_of(L'\\') + 1);
    if (name.size() > 4 && _wcsicmp(name.c_str() + name.size() - 4, L".exe") == 0) {
        name.resize(name.size() - 4);
    }
    return name;
}

std::wstring SessionName(IAudioSessionControl2* control, std::wstring const& path) {
    LPWSTR display = nullptr;
    std::wstring name;
    if (SUCCEEDED(control->GetDisplayName(&display)) && display && *display) {
        if (display[0] == L'@') {
            WCHAR resolved[256];
            if (SUCCEEDED(SHLoadIndirectString(display, resolved, ARRAYSIZE(resolved), nullptr))) {
                name = resolved;
            }
        } else {
            name = display;
        }
    }
    CoTaskMemFree(display);
    if (name.empty() && !path.empty()) {
        name = FileDescription(path);
    }
    return name.empty() ? L"Unknown app" : name;
}

// Programs' icons, loaded once each.
std::map<std::wstring, HICON> g_appIcons;

HICON AppIcon(std::wstring const& path, int size) {
    std::wstring key = path + L"|" + std::to_wstring(size);
    auto it = g_appIcons.find(key);
    if (it != g_appIcons.end()) {
        return it->second;
    }
    HICON icon = nullptr;
    if (!path.empty()) {
        SHDefExtractIconW(path.c_str(), 0, 0, &icon, nullptr, MAKELONG(size, size));
    }
    g_appIcons[key] = icon;
    return icon;
}

void ReleaseMixer() {
    for (auto& session : g_flyout.mixer) {
        for (auto* control : session.controls) {
            control->Release();
        }
    }
    g_flyout.mixer.clear();
}

void LoadMixerFlyout() {
    auto& f = g_flyout;
    ReleaseMixer();
    f.mixerDevice = g_audioDeviceName.empty() ? L"No output device" : g_audioDeviceName;
    f.mixerMaster = -1;
    f.mixerMasterMuted = false;
    float level = 0;
    BOOL muted = FALSE;
    if (g_endpointVolume && SUCCEEDED(g_endpointVolume->GetMasterVolumeLevelScalar(&level))) {
        f.mixerMaster = (int)(level * 100 + 0.5f);
        g_endpointVolume->GetMute(&muted);
        f.mixerMasterMuted = muted;
    }

    IMMDevice* device = nullptr;
    if (!g_audioEnumerator ||
        FAILED(g_audioEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        return;
    }
    IAudioSessionManager2* manager = nullptr;
    if (SUCCEEDED(device->Activate(kIidAudioSessionManager2, CLSCTX_ALL, nullptr, (void**)&manager))) {
        IAudioSessionEnumerator* sessions = nullptr;
        if (SUCCEEDED(manager->GetSessionEnumerator(&sessions))) {
            int count = 0;
            sessions->GetCount(&count);
            for (int i = 0; i < count; i++) {
                IAudioSessionControl* control = nullptr;
                if (FAILED(sessions->GetSession(i, &control))) {
                    continue;
                }
                AudioSessionState state = AudioSessionStateExpired;
                IAudioSessionControl2* control2 = nullptr;
                ISimpleAudioVolume* volume = nullptr;
                if (SUCCEEDED(control->GetState(&state)) && state != AudioSessionStateExpired &&
                    SUCCEEDED(control->QueryInterface(kIidAudioSessionControl2, (void**)&control2)) &&
                    SUCCEEDED(control->QueryInterface(kIidSimpleAudioVolume, (void**)&volume))) {
                    DWORD pid = 0;
                    control2->GetProcessId(&pid);
                    bool system = control2->IsSystemSoundsSession() == S_OK;
                    std::wstring path = system ? L"" : ProcessPath(pid);
                    float value = 0;
                    BOOL sessionMuted = FALSE;
                    volume->GetMasterVolume(&value);
                    volume->GetMute(&sessionMuted);
                    // One row per program: a browser plays through several
                    // sessions, some programs through several processes, and
                    // they all follow the program's row.
                    auto existing = std::find_if(f.mixer.begin(), f.mixer.end(), [&](MixerSession const& s) {
                        return !system && !s.system &&
                               (s.pid == pid || (!path.empty() && _wcsicmp(s.path.c_str(), path.c_str()) == 0));
                    });
                    if (existing != f.mixer.end()) {
                        existing->controls.push_back(volume);
                    } else {
                        MixerSession session;
                        session.pid = pid;
                        session.system = system;
                        session.path = path;
                        session.name = system ? L"System sounds" : SessionName(control2, session.path);
                        session.volume = (int)(value * 100 + 0.5f);
                        session.muted = sessionMuted;
                        session.controls.push_back(volume);
                        f.mixer.push_back(std::move(session));
                    }
                    volume = nullptr;
                }
                if (volume) {
                    volume->Release();
                }
                if (control2) {
                    control2->Release();
                }
                control->Release();
            }
            sessions->Release();
        }
        manager->Release();
    }
    device->Release();

    // System sounds first, then the programs by name.
    std::stable_sort(f.mixer.begin(), f.mixer.end(), [](MixerSession const& a, MixerSession const& b) {
        if (a.system != b.system) {
            return a.system;
        }
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
    });
}

// Slider 0 is the device, then one per program.
void SetMixerVolume(int slider, int value) {
    auto& f = g_flyout;
    value = std::clamp(value, 0, 100);
    if (slider == 0) {
        if (g_endpointVolume) {
            g_endpointVolume->SetMasterVolumeLevelScalar(value / 100.0f, nullptr);
            f.mixerMaster = value;
        }
        return;
    }
    if (slider - 1 < (int)f.mixer.size()) {
        auto& session = f.mixer[slider - 1];
        for (auto* control : session.controls) {
            control->SetMasterVolume(value / 100.0f, nullptr);
        }
        session.volume = value;
    }
}

void ToggleMixerMute(int slider) {
    auto& f = g_flyout;
    if (slider == 0) {
        if (g_endpointVolume) {
            f.mixerMasterMuted = !f.mixerMasterMuted;
            g_endpointVolume->SetMute(f.mixerMasterMuted, nullptr);
        }
        return;
    }
    if (slider - 1 < (int)f.mixer.size()) {
        auto& session = f.mixer[slider - 1];
        session.muted = !session.muted;
        for (auto* control : session.controls) {
            control->SetMute(session.muted, nullptr);
        }
    }
}

std::string SpeakerArt(int volume, bool muted) {
    if (muted) {
        return "audio-mute";
    }
    return volume <= 0 ? "audio-0" : volume < 34 ? "audio-low" : volume < 67 ? "audio-mid" : "audio-high";
}

// One row: icon (a mute button), name, level, slider. The mute hit goes in
// first so it wins over the row's slider.
void MixerRow(Canvas& c, std::vector<Hit>& hits, int hover, int slider, std::wstring const& name,
              int volume, bool muted, HICON icon) {
    auto& f = g_flyout;
    RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(48)};
    int sliderId = MakeHit(kHitMixerSlider, slider);
    int muteId = MakeHit(kHitMixerMute, slider);
    if (hover == sliderId || hover == muteId || f.dragMixer == slider) {
        c.Fill(row, ui::kHover);
    }
    RECT mute{c.Px(10), c.y + c.Px(2), c.Px(36), c.y + c.Px(22)};
    if (hover == muteId) {
        c.Frame(mute, ui::kEdge);
    }
    if (icon && c.paint && !muted) {
        int size = c.Px(16);
        DrawIconEx(c.dc, c.Px(15), c.y + c.Px(4), icon, size, size, 0, nullptr, DI_NORMAL);
    } else {
        c.Art(muted ? "audio-mute" : SpeakerArt(volume, false), c.Px(15), c.y + c.Px(4), c.Px(16));
    }
    hits.push_back({mute, muteId});

    c.Text(f.body, slider == 0 ? ui::kInk : ui::kSage,
           {c.Px(42), c.y + c.Px(2), c.width - c.Px(70), c.y + c.Px(22)}, name);
    std::wstring value = volume < 0 ? L"" : muted ? L"Muted" : std::to_wstring(volume) + L"%";
    c.Text(f.caption, muted ? ui::kSageAlt : ui::kCoral,
           {c.width - c.Px(70), c.y + c.Px(2), c.width - c.Px(15), c.y + c.Px(22)}, value, DT_RIGHT);

    Hit hit{row, sliderId};
    RECT track = SliderTrack(hit, c.s);
    if (volume >= 0) {
        c.Fill(track, ui::kRaised);
        c.Frame(track, ui::kEdge);
        int filled = track.left + (track.right - track.left) * volume / 100;
        c.Fill({track.left + c.s, track.top + c.s, std::max<LONG>(track.left + c.s, filled), track.bottom - c.s},
               muted ? ui::kSageAlt : ui::kGreen);
        c.Fill({filled - c.Px(2), track.top - c.Px(4), filled + c.Px(2), track.bottom + c.Px(4)}, ui::kInk);
        hits.push_back(hit);
    }
    c.y += c.Px(50);
}

void RenderMixer(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    c.Label(L"Device", ui::kSage);
    c.y += c.Px(4);
    MixerRow(c, hits, hover, 0, f.mixerDevice, f.mixerMaster, f.mixerMasterMuted, nullptr);

    c.Divider();
    c.Label(L"Applications", ui::kGreen);
    c.y += c.Px(4);
    if (f.mixer.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"Nothing is playing sound");
        c.y += c.Px(24);
    }
    for (size_t i = 0; i < f.mixer.size(); i++) {
        auto const& session = f.mixer[i];
        MixerRow(c, hits, hover, (int)i + 1, session.name, session.volume, session.muted,
                 session.system ? nullptr : AppIcon(session.path, c.Px(16)));
    }

    c.Divider();
    c.Link(L"Open Volume mixer", 0, hits, hover);
    c.Link(L"Sound settings", 1, hits, hover);
    c.y += c.Px(10);
}

void DragMixer(int slider, int x) {
    for (auto const& hit : g_flyout.hits) {
        if (hit.id == MakeHit(kHitMixerSlider, slider)) {
            RECT track = SliderTrack(hit, g_flyout.scale);
            SetMixerVolume(slider, (x - track.left) * 100 / std::max(1, (int)(track.right - track.left)));
            InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
            return;
        }
    }
}

int MixerLevel(int slider) {
    if (slider == 0) {
        return g_flyout.mixerMaster;
    }
    return slider - 1 < (int)g_flyout.mixer.size() ? g_flyout.mixer[slider - 1].volume : 0;
}

void LayoutFlyout(int iconKind);

// Notification centre and calendar flyout: notifications grouped by app in
// the design's icons, then the month and the picked day's agenda.

constexpr int kNotifyListHeight = 330;  // At 100%; the list scrolls past it.
constexpr int kMaxAgendaRows = 6;

int LocaleFirstDayOfWeek() {
    WCHAR text[4];
    // The locale says 0 for Monday; counted from Sunday here, as SYSTEMTIME does.
    int monday = GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, text, ARRAYSIZE(text))
                     ? _wtoi(text)
                     : 6;
    return (monday + 1) % 7;
}

LONGLONG CalendarGridStart(int year, int month) {
    SYSTEMTIME first{};
    first.wYear = (WORD)year;
    first.wMonth = (WORD)month;
    first.wDay = 1;
    LONGLONG day = MinutesOf(first);
    SYSTEMTIME withWeekday = TimeOfMinutes(day);
    int offset = (withWeekday.wDayOfWeek - LocaleFirstDayOfWeek() + 7) % 7;
    return day - offset * 24 * 60;
}

void LoadCalendarMonth() {
    auto& f = g_flyout;
    LONGLONG start = CalendarGridStart(f.calYear, f.calMonth);
    f.calMonthEvents = CalendarBetween(start, start + 42 * 24 * 60);
}

void LoadNotifyFlyout() {
    auto& f = g_flyout;
    f.notifyApps = LoadNotifications();
    DWORD toasts = 1;
    f.notifyDnd = ReadDword(HKEY_CURRENT_USER, kNotificationsPath, L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", &toasts) &&
                  !toasts;
    int entries = 0;
    for (auto const& app : f.notifyApps) {
        entries += 1 + (int)app.items.size();
    }
    f.notifyEntries = entries;
    f.notifyScroll = std::clamp(f.notifyScroll, 0, std::max(0, entries - 1));
    LoadCalendarMonth();
    f.width = 340 * f.scale;
}

int BlockLines(Canvas& c, HFONT font, int width, std::wstring const& text, int maxLines) {
    if (text.empty()) {
        return 0;
    }
    RECT rc{0, 0, width, 0};
    HGDIOBJ old = SelectObject(c.dc, font);
    SetTextCharacterExtra(c.dc, 0);
    DrawTextW(c.dc, text.c_str(), -1, &rc, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
    TEXTMETRICW metrics{};
    GetTextMetricsW(c.dc, &metrics);
    SelectObject(c.dc, old);
    int line = std::max(1, (int)metrics.tmHeight);
    return std::clamp(((int)rc.bottom + line - 1) / line, 1, maxLines);
}

void TextBlock(Canvas& c, HFONT font, COLORREF color, RECT rc, std::wstring const& text) {
    if (!c.paint || text.empty()) {
        return;
    }
    HGDIOBJ old = SelectObject(c.dc, font);
    SetTextCharacterExtra(c.dc, 0);
    SetBkMode(c.dc, TRANSPARENT);
    SetTextColor(c.dc, color);
    DrawTextW(c.dc, text.c_str(), -1, &rc, DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL | DT_END_ELLIPSIS);
    SelectObject(c.dc, old);
}

void RenderCalendar(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    SYSTEMTIME picked = TimeOfMinutes(f.calSelected);
    c.Text(f.body, ui::kInk, {c.Px(15), c.y, c.width - c.Px(70), c.y + c.Px(20)},
           FormatDate(picked, nullptr, DATE_LONGDATE));
    RECT prev{c.width - c.Px(62), c.y, c.width - c.Px(40), c.y + c.Px(20)};
    RECT next{c.width - c.Px(37), c.y, c.width - c.Px(15), c.y + c.Px(20)};
    auto arrow = [&](RECT rc, int type, PCWSTR glyph) {
        int id = MakeHit(type, 0);
        if (hover == id) {
            c.Fill(rc, ui::kHover);
        }
        c.Frame(rc, ui::kEdge);
        c.CenteredText(f.body, hover == id ? ui::kCoral : ui::kGreen, rc, glyph, 0);
        hits.push_back({rc, id});
    };
    arrow(prev, kHitCalPrev, L"<");
    arrow(next, kHitCalNext, L">");
    c.y += c.Px(26);

    SYSTEMTIME month{};
    month.wYear = (WORD)f.calYear;
    month.wMonth = (WORD)f.calMonth;
    month.wDay = 1;
    std::wstring monthName = FormatDate(month, L"MMMM yyyy");
    CharUpperBuffW(monthName.data(), (DWORD)monthName.size());
    c.Text(f.label, ui::kSage, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(14)}, monthName, DT_LEFT, c.Px(2));
    c.y += c.Px(18);

    int cellWidth = (c.width - c.Px(30)) / 7;
    int cellHeight = c.Px(22);
    int firstDay = LocaleFirstDayOfWeek();
    for (int col = 0; col < 7; col++) {
        int weekday = (firstDay + col) % 7;
        std::wstring name = LocaleString(weekday == 0 ? LOCALE_SSHORTESTDAYNAME7 : LOCALE_SSHORTESTDAYNAME1 + weekday - 1);
        RECT rc{c.Px(15) + col * cellWidth, c.y, c.Px(15) + (col + 1) * cellWidth, c.y + c.Px(14)};
        c.CenteredText(f.label, ui::kSageAlt, rc, name, 0);
    }
    c.y += c.Px(18);

    SYSTEMTIME now;
    GetLocalTime(&now);
    LONGLONG today = DayStart(MinutesOf(now));
    LONGLONG start = CalendarGridStart(f.calYear, f.calMonth);
    for (int cell = 0; cell < 42; cell++) {
        LONGLONG day = start + cell * 24 * 60;
        SYSTEMTIME date = TimeOfMinutes(day);
        RECT rc{c.Px(15) + (cell % 7) * cellWidth, c.y + (cell / 7) * cellHeight,
                c.Px(15) + (cell % 7 + 1) * cellWidth, c.y + (cell / 7 + 1) * cellHeight};
        int id = MakeHit(kHitCalDay, cell);
        bool inMonth = date.wMonth == f.calMonth;
        if (day == today || hover == id) {
            c.Fill(rc, ui::kHover);
        }
        if (day == f.calSelected) {
            c.Frame(rc, ui::kGreen);
        } else if (day == today) {
            c.Frame(rc, ui::kEdge);
        }
        c.CenteredText(f.caption, day == today ? ui::kInk : inMonth ? ui::kSage : ui::kLine, rc,
                       std::to_wstring(date.wDay), 0);
        bool busy = std::any_of(f.calMonthEvents.begin(), f.calMonthEvents.end(), [&](CalOccurrence const& o) {
            return o.start < day + 24 * 60 && std::max(o.end, o.start + 1) > day;
        });
        if (busy) {
            int cx = (rc.left + rc.right) / 2;
            c.Fill({cx - c.s, rc.bottom - c.Px(4), cx + c.s, rc.bottom - c.Px(4) + 2 * c.s}, ui::kCoral);
        }
        hits.push_back({rc, id});
    }
    c.y += 6 * cellHeight + c.Px(8);

    DWORD interval = 0;
    if (CalendarSource(&interval).empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)},
               L"No calendar connected");
        c.y += c.Px(20);
        return;
    }
    std::vector<CalOccurrence> agenda;
    for (auto const& occurrence : f.calMonthEvents) {
        if (occurrence.start < f.calSelected + 24 * 60 && std::max(occurrence.end, occurrence.start + 1) > f.calSelected) {
            agenda.push_back(occurrence);
        }
    }
    if (agenda.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)},
               L"No events scheduled");
        c.y += c.Px(20);
    }
    for (size_t i = 0; i < agenda.size() && i < (size_t)kMaxAgendaRows; i++) {
        auto const& occurrence = agenda[i];
        std::wstring when = occurrence.allDay ? L"All day" : FormatTime(TimeOfMinutes(occurrence.start), nullptr, TIME_NOSECONDS);
        c.Text(f.caption, ui::kCoral, {c.Px(15), c.y, c.Px(85), c.y + c.Px(18)}, when);
        c.Text(f.caption, ui::kInk, {c.Px(90), c.y, c.width - c.Px(15), c.y + c.Px(18)},
               occurrence.title.empty() ? L"(no title)" : occurrence.title);
        c.y += c.Px(18);
    }
    if (agenda.size() > (size_t)kMaxAgendaRows) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)},
               L"+" + std::to_wstring(agenda.size() - kMaxAgendaRows) + L" more");
        c.y += c.Px(18);
    }
}

void RenderNotify(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    f.notifyTargets.clear();
    c.y = c.Px(15);
    int headerTop = c.y;
    c.Label(L"Notifications", ui::kSage);
    if (!f.notifyApps.empty()) {
        RECT clear{c.width - c.Px(100), headerTop - c.Px(2), c.width - c.Px(15), headerTop + c.Px(16)};
        int id = MakeHit(kHitNotifyClearAll, 0);
        c.Text(f.caption, hover == id ? ui::kCoral : ui::kGreen, clear, L"Clear all", DT_RIGHT);
        hits.push_back({clear, id});
    }
    c.y += c.Px(6);
    if (f.notifyDnd) {
        c.Text(f.caption, ui::kCoral, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)},
               L"Do not disturb is on");
        c.y += c.Px(20);
    }

    int listBottom = c.y + c.Px(kNotifyListHeight);
    int textLeft = c.Px(40);
    int entry = 0;
    bool clipped = false;
    for (int a = 0; a < (int)f.notifyApps.size() && !clipped; a++) {
        auto const& app = f.notifyApps[a];
        if (entry++ >= f.notifyScroll) {
            if (c.y + c.Px(26) > listBottom) {
                clipped = true;
                break;
            }
            c.Art(app.design, c.Px(15), c.y + c.Px(5), c.Px(16));
            std::wstring name = app.name;
            CharUpperBuffW(name.data(), (DWORD)name.size());
            c.Text(f.label, ui::kSage, {textLeft, c.y, c.width - c.Px(40), c.y + c.Px(26)}, name, DT_LEFT, c.Px(2));
            RECT clear{c.width - c.Px(32), c.y + c.Px(4), c.width - c.Px(12), c.y + c.Px(22)};
            int clearId = MakeHit(kHitNotifyClearApp, a);
            c.CenteredText(f.body, hover == clearId ? ui::kCoral : ui::kSageAlt, clear, L"x", 0);
            hits.push_back({clear, clearId});
            c.y += c.Px(26);
        }
        for (int i = 0; i < (int)app.items.size(); i++) {
            if (entry++ < f.notifyScroll) {
                continue;
            }
            auto const& item = app.items[i];
            int bodyWidth = c.width - textLeft - c.Px(15);
            int lines = BlockLines(c, f.caption, bodyWidth, item.body, 2);
            int lineHeight = c.Px(16);
            int height = c.Px(20) + lines * lineHeight + (item.actions.empty() ? 0 : c.Px(20)) + c.Px(8);
            if (c.y + height > listBottom) {
                clipped = true;
                break;
            }
            int target = (int)f.notifyTargets.size();
            f.notifyTargets.push_back({a, i, -1});
            RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + height - c.Px(4)};
            int itemId = MakeHit(kHitNotifyItem, target);
            int dismissId = MakeHit(kHitNotifyDismiss, target);
            bool hot = hover == itemId || hover == dismissId;
            if (hot) {
                c.Fill(row, ui::kHover);
            }
            c.Text(f.body, ui::kInk, {textLeft, c.y, c.width - c.Px(72), c.y + c.Px(20)},
                   item.title.empty() ? app.name : item.title);
            c.Text(f.caption, ui::kSageAlt, {c.width - c.Px(72), c.y, c.width - c.Px(34), c.y + c.Px(20)},
                   RelativeTime(item.arrival), DT_RIGHT);
            RECT dismiss{c.width - c.Px(32), c.y + c.Px(1), c.width - c.Px(12), c.y + c.Px(19)};
            c.CenteredText(f.body, hover == dismissId ? ui::kCoral : hot ? ui::kSage : ui::kLine, dismiss, L"x", 0);
            hits.push_back({dismiss, dismissId});
            int y = c.y + c.Px(20);
            TextBlock(c, f.caption, ui::kSage, {textLeft, y, textLeft + bodyWidth, y + lines * lineHeight}, item.body);
            y += lines * lineHeight;
            int x = textLeft;
            for (int k = 0; k < (int)item.actions.size(); k++) {
                auto const& action = item.actions[k];
                SIZE size = c.Measure(f.caption, action.label);
                RECT button{x, y + c.Px(2), x + size.cx + c.Px(12), y + c.Px(18)};
                if (button.right > c.width - c.Px(15)) {
                    break;
                }
                int actionTarget = (int)f.notifyTargets.size();
                f.notifyTargets.push_back({a, i, k});
                int actionId = MakeHit(kHitNotifyAction, actionTarget);
                c.Frame(button, hover == actionId ? ui::kCoral : ui::kEdge);
                c.CenteredText(f.caption, hover == actionId ? ui::kCoral : ui::kGreen, button, action.label, 0);
                hits.push_back({button, actionId});
                x = button.right + c.Px(8);
            }
            hits.push_back({row, itemId});
            c.y += height;
        }
    }
    if (f.notifyApps.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"No new notifications");
        c.y += c.Px(24);
    } else if (clipped || f.notifyScroll > 0) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)},
               clipped ? L"Scroll for more" : L"Scroll up for newer");
        c.y += c.Px(18);
    }

    c.Divider();
    RenderCalendar(c, hits, hover);
    c.Divider();
    c.Link(L"Notification settings", 0, hits, hover);
    c.Link(L"Date and time settings", 1, hits, hover);
    c.y += c.Px(10);
}

NotifyItem const* NotifyTargetItem(int target, int* action) {
    auto& f = g_flyout;
    if (target < 0 || target >= (int)f.notifyTargets.size()) {
        return nullptr;
    }
    auto [a, i, k] = f.notifyTargets[target];
    if (a >= (int)f.notifyApps.size() || i >= (int)f.notifyApps[a].items.size()) {
        return nullptr;
    }
    *action = k;
    return &f.notifyApps[a].items[i];
}

// Takes a notification off the list at once; Windows' database follows.
void DropNotifyItem(NotifyItem const* item) {
    auto& f = g_flyout;
    for (auto app = f.notifyApps.begin(); app != f.notifyApps.end(); ++app) {
        auto it = std::find_if(app->items.begin(), app->items.end(),
                               [&](NotifyItem const& other) { return &other == item; });
        if (it != app->items.end()) {
            app->items.erase(it);
            if (app->items.empty()) {
                f.notifyApps.erase(app);
            }
            return;
        }
    }
}

void OnNotifyClick(int type, int index) {
    auto& f = g_flyout;
    switch (type) {
        case kHitNotifyItem:
        case kHitNotifyAction: {
            int action = -1;
            NotifyItem const* item = NotifyTargetItem(index, &action);
            if (!item) {
                return;
            }
            NotifyItem copy = *item;
            CloseFlyout();
            if (action >= 0 && action < (int)copy.actions.size()) {
                auto const& chosen = copy.actions[action];
                ActivateNotification(copy, chosen.arguments, chosen.activationType);
            } else {
                ActivateNotification(copy, copy.launch, copy.activationType);
            }
            return;
        }
        case kHitNotifyDismiss: {
            int action = -1;
            if (NotifyItem const* item = NotifyTargetItem(index, &action)) {
                RemoveNotification(*item);
                DropNotifyItem(item);
            }
            break;
        }
        case kHitNotifyClearApp:
            if (index < (int)f.notifyApps.size()) {
                ClearAppNotifications(f.notifyApps[index].aumid);
                f.notifyApps.erase(f.notifyApps.begin() + index);
            }
            break;
        case kHitNotifyClearAll:
            for (auto const& app : f.notifyApps) {
                ClearAppNotifications(app.aumid);
            }
            f.notifyApps.clear();
            f.notifyScroll = 0;
            break;
        case kHitCalPrev:
        case kHitCalNext: {
            int month = f.calYear * 12 + (f.calMonth - 1) + (type == kHitCalNext ? 1 : -1);
            f.calYear = month / 12;
            f.calMonth = month % 12 + 1;
            LoadCalendarMonth();
            break;
        }
        case kHitCalDay: {
            f.calSelected = CalendarGridStart(f.calYear, f.calMonth) + index * 24 * 60;
            SYSTEMTIME picked = TimeOfMinutes(f.calSelected);
            if (picked.wMonth != f.calMonth) {
                f.calYear = picked.wYear;
                f.calMonth = picked.wMonth;
                LoadCalendarMonth();
            }
            break;
        }
    }
    LayoutFlyout(-1);
}

// Window

int Render(HDC dc, bool paint, std::vector<Hit>& hits) {
    Canvas c{dc, paint, g_flyout.scale, g_flyout.width};
    hits.clear();
    switch (g_flyout.kind) {
        case kFlyoutNetwork: RenderNetwork(c, hits, g_flyout.hover); break;
        case kFlyoutPower: RenderPower(c, hits, g_flyout.hover); break;
        case kFlyoutBrightness: RenderBrightness(c, hits, g_flyout.hover); break;
        case kFlyoutOverflow: RenderOverflow(c, hits, g_flyout.hover); break;
        case kFlyoutVpn: RenderVpn(c, hits, g_flyout.hover); break;
        case kFlyoutSecurity: RenderSecurity(c, hits, g_flyout.hover); break;
        case kFlyoutMixer: RenderMixer(c, hits, g_flyout.hover); break;
        case kFlyoutNotify: RenderNotify(c, hits, g_flyout.hover); break;
    }
    return c.y;
}

// Sizes the window to its content and keeps it next to the tray.
void LayoutFlyout(int iconKind) {
    HWND hwnd = g_flyout.hwnd;
    HDC dc = GetDC(hwnd);
    int height = Render(dc, false, g_flyout.hits);
    ReleaseDC(hwnd, dc);
    int width = g_flyout.width;

    RECT anchor{};
    NOTIFYICONIDENTIFIER identifier{sizeof(identifier)};
    identifier.hWnd = g_wnd;
    identifier.uID = kIconIdBase + iconKind;
    if (iconKind < 0 || FAILED(Shell_NotifyIconGetRect(&identifier, &anchor))) {
        RECT current;
        if (GetWindowRect(hwnd, &current) && IsWindowVisible(hwnd)) {
            // Keep the bottom edge while resizing.
            SetWindowPos(hwnd, nullptr, current.left, current.bottom - height, width, height,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            return;
        }
        POINT pt;
        GetCursorPos(&pt);
        anchor = {pt.x, pt.y, pt.x, pt.y};
    }

    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(MonitorFromRect(&anchor, MONITOR_DEFAULTTONEAREST), &monitor);
    RECT work = monitor.rcWork;
    int gap = 8 * g_flyout.scale;
    int x = std::clamp((int)(anchor.left + anchor.right) / 2 - width / 2,
                       (int)work.left + gap, (int)work.right - width - gap);
    int y;
    if (anchor.top >= work.bottom - gap) {
        y = work.bottom - height - gap;  // Taskbar at the bottom.
    } else if (anchor.bottom <= work.top + gap) {
        y = work.top + gap;  // At the top.
    } else {
        y = std::clamp((int)anchor.top - height - gap, (int)work.top + gap,
                       (int)work.bottom - height - gap);
    }
    SetWindowPos(hwnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE);
}

void LoadFlyout() {
    switch (g_flyout.kind) {
        case kFlyoutNetwork: LoadNetworkFlyout(); break;
        case kFlyoutPower: LoadPowerFlyout(); break;
        case kFlyoutBrightness: break;  // Filled in by the worker.
        case kFlyoutOverflow: LoadOverflowFlyout(); break;
        case kFlyoutVpn: LoadVpnFlyout(); break;
        case kFlyoutSecurity: LoadSecurityFlyout(); break;
        case kFlyoutMixer: LoadMixerFlyout(); break;
        case kFlyoutNotify: LoadNotifyFlyout(); break;
    }
}

bool IsFlyoutOpen(int flyoutKind) {
    return g_flyout.hwnd && g_flyout.kind == flyoutKind;
}

void ReloadFlyout() {
    if (g_flyout.hwnd &&
        (g_flyout.kind == kFlyoutNetwork || g_flyout.kind == kFlyoutPower ||
         g_flyout.kind == kFlyoutVpn || g_flyout.kind == kFlyoutSecurity ||
         g_flyout.kind == kFlyoutMixer || g_flyout.kind == kFlyoutNotify)) {
        PostMessageW(g_flyout.hwnd, WM_APP_FLYOUT_RELOAD, 0, 0);
    }
}

void CloseFlyout() {
    // Destroying the active window deactivates it, which would close it again.
    if (g_flyout.hwnd && !g_flyoutClosing) {
        g_flyoutClosing = true;
        DestroyWindow(g_flyout.hwnd);
        g_flyoutClosing = false;
    }
}

void Execute(std::function<void()> action) {
    if (action) {
        action();
    }
}

void OnFlyoutClick(int id) {
    int type = id >> 16;
    int index = id & 0xFFFF;
    auto& f = g_flyout;
    switch (type) {
        case kHitWifiRow:
            if (f.selectedNetwork == index) {
                f.selectedNetwork = -1;
                f.selectedSsid.clear();
            } else if (index < (int)f.networks.size()) {
                f.selectedNetwork = index;
                f.selectedSsid = f.networks[index].ssid;
            }
            LayoutFlyout(-1);
            break;
        case kHitWifiAction:
            if (index < (int)f.networks.size()) {
                ConnectOrDisconnect(f.networks[index]);
            }
            break;
        case kHitPlan:
            if (index < (int)f.plans.size()) {
                PowerSetActiveScheme(nullptr, &f.plans[index].guid);
                LoadPowerFlyout();
            }
            break;
        case kHitVpnAction:
            if (f.vpnConnected) {
                RunVpnCommand(L"disconnect", L"Disconnecting...");
            } else {
                RunVpnCommand(L"connect best", L"Connecting to the best location...");
            }
            LayoutFlyout(-1);
            break;
        case kHitVpnRow:
            OnVpnRowClick(index);
            LayoutFlyout(-1);
            break;
        case kHitSecurityRow:
            if (index < (int)f.securityRows.size()) {
                PCWSTR uri = f.securityRows[index].uri;
                CloseFlyout();
                OpenUri(uri);
            }
            return;
        case kHitMixerMute:
            ToggleMixerMute(index);
            break;
        case kHitNotifyItem:
        case kHitNotifyAction:
        case kHitNotifyDismiss:
        case kHitNotifyClearApp:
        case kHitNotifyClearAll:
        case kHitCalPrev:
        case kHitCalNext:
        case kHitCalDay:
            OnNotifyClick(type, index);
            break;
        case kHitHidden:
            ClickHiddenIcon(index, kClickLeft);
            return;
        case kHitLink:
            if (f.kind == kFlyoutNetwork) {
                CloseFlyout();
                if (index == 0) {
                    Run(L"control.exe", L"/name Microsoft.NetworkAndSharingCenter");
                } else {
                    OpenUri(L"ms-settings:network");
                }
            } else if (f.kind == kFlyoutPower) {
                if (index == 0) {
                    CloseFlyout();
                    ShowFlyout(kFlyoutBrightness, kBattery);
                } else {
                    CloseFlyout();
                    Run(L"control.exe", L"/name Microsoft.PowerOptions");
                }
            } else if (f.kind == kFlyoutBrightness) {
                CloseFlyout();
                OpenUri(index == 0 ? L"ms-settings:nightlight" : L"ms-settings:display");
            } else if (f.kind == kFlyoutOverflow) {
                CloseFlyout();
                OpenTrayCustomization();
            } else if (f.kind == kFlyoutVpn) {
                CloseFlyout();
                if (index == 0) {
                    std::wstring app = WindscribeFile(L"Windscribe.exe");
                    if (!app.empty()) {
                        Run(app.c_str());
                    }
                } else {
                    OpenUri(L"ms-settings:network-vpn");
                }
            } else if (f.kind == kFlyoutSecurity) {
                CloseFlyout();
                OpenUri(L"windowsdefender:");
            } else if (f.kind == kFlyoutMixer) {
                CloseFlyout();
                if (index == 0) {
                    Run(L"SndVol.exe");
                } else {
                    OpenUri(L"ms-settings:sound");
                }
            } else if (f.kind == kFlyoutNotify) {
                CloseFlyout();
                OpenUri(index == 0 ? L"ms-settings:notifications" : L"ms-settings:dateandtime");
            }
            return;
    }
    if (g_flyout.hwnd) {
        InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
    }
}

int HitAt(POINT pt) {
    for (auto const& hit : g_flyout.hits) {
        if (PtInRect(&hit.rc, pt)) {
            return hit.id;
        }
    }
    return 0;
}

void DragSlider(int monitor, int x) {
    for (auto const& hit : g_flyout.hits) {
        if (hit.id == MakeHit(kHitSlider, monitor)) {
            RECT track = SliderTrack(hit, g_flyout.scale);
            int value = (x - track.left) * 100 / std::max(1, (int)(track.right - track.left));
            SetBrightness(monitor, value);
            InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
            return;
        }
    }
}

LRESULT CALLBACK FlyoutProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hWnd, &ps);
            RECT client;
            GetClientRect(hWnd, &client);
            HDC mem = CreateCompatibleDC(dc);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
            HGDIOBJ old = SelectObject(mem, bitmap);
            SetDCBrushColor(mem, ui::kGround);
            FillRect(mem, &client, (HBRUSH)GetStockObject(DC_BRUSH));
            Render(mem, true, g_flyout.hits);
            SetDCBrushColor(mem, ui::kEdge);
            FrameRect(mem, &client, (HBRUSH)GetStockObject(DC_BRUSH));
            BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, old);
            DeleteObject(bitmap);
            DeleteDC(mem);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_MOUSEMOVE: {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_flyout.dragHidden >= 0) {
                if (!g_flyout.dragMoved &&
                    (abs(pt.x - g_flyout.dragStart.x) > GetSystemMetrics(SM_CXDRAG) ||
                     abs(pt.y - g_flyout.dragStart.y) > GetSystemMetrics(SM_CYDRAG))) {
                    g_flyout.dragMoved = true;
                }
                if (g_flyout.dragMoved) {
                    int target = HiddenSlotAt(pt);
                    if (target != g_flyout.dragTarget) {
                        g_flyout.dragTarget = target;
                        InvalidateRect(hWnd, nullptr, FALSE);
                    }
                    return 0;
                }
            }
            if (g_flyout.dragSlider >= 0) {
                DragSlider(g_flyout.dragSlider, pt.x);
                return 0;
            }
            if (g_flyout.dragMixer >= 0) {
                DragMixer(g_flyout.dragMixer, pt.x);
                return 0;
            }
            int hover = HitAt(pt);
            if (hover != g_flyout.hover) {
                g_flyout.hover = hover;
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hWnd};
            TrackMouseEvent(&track);
            return 0;
        }

        case WM_CAPTURECHANGED:
            if (g_flyout.dragHidden >= 0 && (HWND)lParam != hWnd) {
                g_flyout.dragHidden = -1;
                g_flyout.dragMoved = false;
                g_flyout.dragTarget = -1;
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;
        case WM_MOUSELEAVE:
            if (g_flyout.hover) {
                g_flyout.hover = 0;
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;

        case WM_LBUTTONDBLCLK: {
            // A double click on a hidden icon reaches its program as one; the
            // first click already went through as a click.
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int id = HitAt(pt);
            if ((id >> 16) == kHitHidden) {
                int index = id & 0xFFFF;
                g_flyout.swallowUp = true;
                if (index < (int)g_flyout.hidden.size() &&
                    !OpensOnDoubleClick(g_flyout.hidden[index].owner)) {
                    ForwardHiddenClick(g_flyout.hidden[index], kClickSecondOfDouble);
                }
                return 0;
            }
            // Anywhere else it's a second press like the first.
            [[fallthrough]];
        }

        case WM_LBUTTONDOWN: {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int id = HitAt(pt);
            if ((id >> 16) == kHitHidden && msg == WM_LBUTTONDOWN) {
                // A press on a hidden icon may turn into a drag to move it.
                g_flyout.dragHidden = id & 0xFFFF;
                g_flyout.dragStart = pt;
                g_flyout.dragMoved = false;
                g_flyout.dragTarget = -1;
                SetCapture(hWnd);
            }
            if ((id >> 16) == kHitSlider) {
                g_flyout.dragSlider = id & 0xFFFF;
                SetCapture(hWnd);
                DragSlider(g_flyout.dragSlider, pt.x);
            }
            if ((id >> 16) == kHitMixerSlider) {
                g_flyout.dragMixer = id & 0xFFFF;
                SetCapture(hWnd);
                DragMixer(g_flyout.dragMixer, pt.x);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            if (g_flyout.dragHidden >= 0) {
                int from = g_flyout.dragHidden;
                int target = g_flyout.dragTarget;
                bool moved = g_flyout.dragMoved;
                g_flyout.dragHidden = -1;
                g_flyout.dragMoved = false;
                g_flyout.dragTarget = -1;
                ReleaseCapture();
                if (moved) {
                    // A drop outside the grid puts it back where it was.
                    if (target >= 0 && target != from) {
                        MoveHiddenIcon(from, target);
                    }
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
            }
            if (g_flyout.dragSlider >= 0) {
                g_flyout.dragSlider = -1;
                ReleaseCapture();
                InvalidateRect(hWnd, nullptr, FALSE);
                return 0;
            }
            if (g_flyout.dragMixer >= 0) {
                g_flyout.dragMixer = -1;
                ReleaseCapture();
                InvalidateRect(hWnd, nullptr, FALSE);
                return 0;
            }
            if (g_flyout.swallowUp) {
                g_flyout.swallowUp = false;
                return 0;
            }
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (int id = HitAt(pt)) {
                OnFlyoutClick(id);
            }
            return 0;
        }

        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int id = HitAt(pt);
            if ((id >> 16) == kHitHidden) {
                ClickHiddenIcon(id & 0xFFFF, msg == WM_RBUTTONUP ? kClickRight : kClickMiddle);
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            int notches = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            if (g_flyout.kind == kFlyoutNetwork) {
                int maxScroll =
                    std::max(0, (int)g_flyout.networks.size() - kMaxVisibleNetworks);
                g_flyout.networkScroll =
                    std::clamp(g_flyout.networkScroll - notches, 0, maxScroll);
                g_flyout.selectedNetwork = -1;
                g_flyout.selectedSsid.clear();
                LayoutFlyout(-1);
                InvalidateRect(hWnd, nullptr, FALSE);
            } else if (g_flyout.kind == kFlyoutNotify) {
                // Over the calendar the wheel turns months; elsewhere it
                // scrolls the notifications.
                int type = g_flyout.hover >> 16;
                if (type == kHitCalDay || type == kHitCalPrev || type == kHitCalNext) {
                    OnNotifyClick(notches > 0 ? kHitCalPrev : kHitCalNext, 0);
                } else {
                    g_flyout.notifyScroll = std::clamp(g_flyout.notifyScroll - notches, 0,
                                                       std::max(0, g_flyout.notifyEntries - 1));
                    LayoutFlyout(-1);
                }
                InvalidateRect(hWnd, nullptr, FALSE);
            } else if (g_flyout.kind == kFlyoutMixer && (g_flyout.hover >> 16) == kHitMixerSlider) {
                int slider = g_flyout.hover & 0xFFFF;
                SetMixerVolume(slider, MixerLevel(slider) + notches * 2);
                InvalidateRect(hWnd, nullptr, FALSE);
            } else if (g_flyout.kind == kFlyoutVpn) {
                int rows = (int)VpnRows().size();
                g_flyout.vpnScroll = std::clamp(g_flyout.vpnScroll - notches, 0,
                                                std::max(0, rows - kMaxVisibleVpnRows));
                LayoutFlyout(-1);
                InvalidateRect(hWnd, nullptr, FALSE);            } else if (g_flyout.kind == kFlyoutBrightness && (g_flyout.hover >> 16) == kHitSlider) {
                int monitor = g_flyout.hover & 0xFFFF;
                if (monitor < (int)g_flyout.monitors.size()) {
                    SetBrightness(monitor, g_flyout.monitors[monitor].value + notches * 2);
                    InvalidateRect(hWnd, nullptr, FALSE);
                }
            }
            return 0;
        }

        case WM_TIMER:
            // Programs start and stop playing while the mixer is open.
            if (wParam == kTimerMixer && g_flyout.kind == kFlyoutMixer && g_flyout.dragMixer < 0) {
                LoadMixerFlyout();
                LayoutFlyout(-1);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                CloseFlyout();
            }
            return 0;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE && !g_flyoutClosing) {
                if (g_stickyPid) {
                    return 0;  // After an icon click the sticky hooks decide.
                }
                g_flyoutClosedTick = GetTickCount();
                g_flyoutClosedKind = g_flyout.kind;
                CloseFlyout();
            }
            return 0;

        case WM_APP_FLYOUT_RELOAD:
            if (g_flyout.dragMixer >= 0) {
                return 0;  // Mid-drag: the next refresh catches up.
            }
            LoadFlyout();
            LayoutFlyout(-1);
            InvalidateRect(hWnd, nullptr, FALSE);
            return 0;

        case WM_APP_JOB_DONE: {
            std::wstring result;
            if (TakeJobResult(g_flyout.job, &result)) {
                if (g_flyout.kind == kFlyoutVpn) {
                    // After a connect or disconnect the VPN icon catches up;
                    // a plain status check changes nothing, so it doesn't.
                    bool command = g_flyout.vpnBusy;
                    ApplyVpnStatus(result);
                    if (command) {
                        RequestRefresh(kNetworkBits);
                    }
                } else if (g_flyout.kind == kFlyoutSecurity) {
                    g_flyout.securityUpdates = result.empty() ? L"unknown" : result;
                    LoadSecurityFlyout();
                }
                LayoutFlyout(-1);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_APP_BRIGHTNESS_READY:
            if (auto* shared = g_flyout.brightness) {
                EnterCriticalSection(&shared->lock);
                g_flyout.monitors = shared->levels;
                g_flyout.monitorsLoaded = shared->loaded;
                LeaveCriticalSection(&shared->lock);
                g_brightnessCache = g_flyout.monitors;
                LayoutFlyout(-1);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, kTimerMixer);
            StopSticky();
            ReleaseMixer();
            g_flyout.dragMixer = -1;
            CancelJob(g_flyout.job);
            StopBrightness();
            if (g_flyout.hwnd == hWnd && g_flyout.kind == kFlyoutOverflow) {
                g_overflowOpen = false;
                RequestRefresh(1u << kOverflow);
            }
            if (g_flyout.hwnd == hWnd) {
                g_flyout.hwnd = nullptr;
                g_flyout.kind = kFlyoutNone;
                g_flyout.hits.clear();
            }
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void ShowFlyout(int flyoutKind, int iconKind) {
    // Clicking the icon of an open flyout deactivates the flyout first, which
    // closes it; that click should leave it closed, like a toggle.
    if (g_flyoutClosedKind == flyoutKind && GetTickCount() - g_flyoutClosedTick < 300) {
        g_flyoutClosedKind = kFlyoutNone;
        return;
    }
    CloseFlyout();

    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&FlyoutProc, &instance);

    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kFlyoutClass, L"",
                                WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
    if (!hwnd) {
        return;
    }

    // Pixel fonts and art only stay sharp at whole-number scales.
    RECT anchor{};
    NOTIFYICONIDENTIFIER identifier{sizeof(identifier)};
    identifier.hWnd = g_wnd;
    identifier.uID = kIconIdBase + iconKind;
    POINT pt;
    GetCursorPos(&pt);
    if (SUCCEEDED(Shell_NotifyIconGetRect(&identifier, &anchor))) {
        pt = {(anchor.left + anchor.right) / 2, (anchor.top + anchor.bottom) / 2};
    }
    UINT dpiX = USER_DEFAULT_SCREEN_DPI, dpiY = USER_DEFAULT_SCREEN_DPI;
    using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    static auto getDpiForMonitor = (GetDpiForMonitor_t)GetProcAddress(
        LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
    if (getDpiForMonitor) {
        getDpiForMonitor(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), 0, &dpiX, &dpiY);
    }

    g_flyout.hwnd = hwnd;
    g_flyout.kind = flyoutKind;
    g_flyout.scale = std::max(1, (int)(dpiX + USER_DEFAULT_SCREEN_DPI / 2) / USER_DEFAULT_SCREEN_DPI);
    g_flyout.width = 300 * g_flyout.scale;
    g_flyout.hover = 0;
    g_flyout.dragSlider = -1;
    g_flyout.swallowUp = false;
    g_flyout.dragHidden = -1;
    g_flyout.dragMoved = false;
    g_flyout.selectedNetwork = -1;
    g_flyout.selectedSsid.clear();
    g_flyout.networkScroll = 0;
    if (flyoutKind == kFlyoutOverflow) {
        g_overflowOpen = true;
        RequestRefresh(1u << kOverflow);
    }
    CreateFonts(g_flyout.scale);

    if (flyoutKind == kFlyoutNetwork) {
        ScanWifi();
    } else if (flyoutKind == kFlyoutBrightness) {
        StartBrightness();
        if (!g_brightnessCache.empty()) {
            g_flyout.monitors = g_brightnessCache;
            g_flyout.monitorsLoaded = true;
        }
    } else if (flyoutKind == kFlyoutSecurity) {
        g_flyout.securityUpdates.clear();
        g_flyout.job = StartJob(SearchPendingUpdates);
    } else if (flyoutKind == kFlyoutNotify) {
        SYSTEMTIME today;
        GetLocalTime(&today);
        g_flyout.calYear = today.wYear;
        g_flyout.calMonth = today.wMonth;
        g_flyout.calSelected = DayStart(MinutesOf(today));
        g_flyout.notifyScroll = 0;
        RefreshCalendar(false);
    } else if (flyoutKind == kFlyoutMixer) {
        g_flyout.dragMixer = -1;
        SetTimer(hwnd, kTimerMixer, kMixerRefreshMs, nullptr);
    } else if (flyoutKind == kFlyoutVpn) {
        g_flyout.vpnOpenRegion = -1;
        g_flyout.vpnScroll = 0;
        g_flyout.vpnBusy = false;
        g_flyout.vpnChecking = false;
        g_flyout.vpnStatus = g_vpnLastStatus.empty() ? L"Checking..." : g_vpnLastStatus;
        g_flyout.vpnConnected = g_vpnLastConnected;
        RunVpnCommand(L"", L"");
    }
    LoadFlyout();
    LayoutFlyout(iconKind);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetForegroundWindow(hwnd);
}

////////////////////////////////////////////////////////////////////////////////
// Tray tooltips in the Tourne style (in Explorer): the tray's own tooltip
// windows, given the flyouts' font and drawn like them. The tooltip still
// sizes itself, with the new font, so the text always fits.

constexpr DWORD kWindowCornerPreference = 33;  // DWMWA_WINDOW_CORNER_PREFERENCE
constexpr DWORD kCornerDoNotRound = 1;         // DWMWCP_DONOTROUND

struct StyledTooltip {
    HWND hwnd;
    HFONT oldFont;
    RECT oldMargin;
    LONG_PTR oldStyle;
};

std::vector<StyledTooltip> g_styledTooltips;  // Explorer side thread only.
std::map<int, HFONT> g_tooltipFonts;           // By scale.

void PaintTooltip(HWND hwnd, HDC dc) {
    RECT client;
    GetClientRect(hwnd, &client);
    SetDCBrushColor(dc, ui::kGround);
    FillRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, ui::kEdge);
    FrameRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));

    WCHAR text[1024];
    GetWindowTextW(hwnd, text, ARRAYSIZE(text));
    RECT margin{};
    SendMessageW(hwnd, TTM_GETMARGIN, 0, (LPARAM)&margin);
    RECT area{client.left + margin.left, client.top + margin.top,
              client.right - margin.right, client.bottom - margin.bottom};
    HGDIOBJ old = SelectObject(dc, (HFONT)SendMessageW(hwnd, WM_GETFONT, 0, 0));
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, ui::kInk);
    DrawTextW(dc, text, -1, &area, DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_EXPANDTABS);
    SelectObject(dc, old);
}

LRESULT CALLBACK TooltipStyleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, DWORD_PTR) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            PaintTooltip(hwnd, dc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_PRINTCLIENT:
            // How the tooltip is drawn while it fades in.
            PaintTooltip(hwnd, (HDC)wParam);
            return 0;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

std::vector<HWND> AllTrayToolbars() {
    std::vector<HWND> toolbars;
    for (PCWSTR taskbarClass : {L"Shell_TrayWnd", L"Shell_SecondaryTrayWnd"}) {
        for (HWND taskbar = FindWindowExW(nullptr, nullptr, taskbarClass, nullptr); taskbar;
             taskbar = FindWindowExW(nullptr, taskbar, taskbarClass, nullptr)) {
            HWND notify = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
            if (HWND toolbar = notify ? FindWindowExW(notify, nullptr, TOOLBARCLASSNAMEW, nullptr) : nullptr) {
                toolbars.push_back(toolbar);
            }
        }
    }
    if (HWND overflow = FindWindowW(L"NotifyIconOverflowWindow", nullptr)) {
        if (HWND toolbar = FindWindowExW(overflow, nullptr, TOOLBARCLASSNAMEW, nullptr)) {
            toolbars.push_back(toolbar);
        }
    }
    return toolbars;
}

void EnsureTooltipStyle() {
    std::erase_if(g_styledTooltips, [](StyledTooltip const& tip) { return !IsWindow(tip.hwnd); });
    for (HWND toolbar : AllTrayToolbars()) {
        DWORD pid = 0;
        GetWindowThreadProcessId(toolbar, &pid);
        HWND tooltip = pid == GetCurrentProcessId()
                           ? (HWND)SendMessageW(toolbar, TB_GETTOOLTIPS, 0, 0)
                           : nullptr;
        if (!tooltip || std::any_of(g_styledTooltips.begin(), g_styledTooltips.end(),
                                    [&](StyledTooltip const& tip) { return tip.hwnd == tooltip; })) {
            continue;
        }
        int scale = std::max(1, (int)(GetDpiForWindow(toolbar) + USER_DEFAULT_SCREEN_DPI / 2) /
                                    USER_DEFAULT_SCREEN_DPI);
        HFONT& font = g_tooltipFonts[scale];
        if (!font) {
            font = MakeFont({L"CozetteVector", L"scientifica"}, 13 * scale);
        }
        StyledTooltip styled{tooltip, (HFONT)SendMessageW(tooltip, WM_GETFONT, 0, 0), {},
                             GetWindowLongPtrW(tooltip, GWL_STYLE)};
        SendMessageW(tooltip, TTM_GETMARGIN, 0, (LPARAM)&styled.oldMargin);
        if (!WindhawkUtils::SetWindowSubclassFromAnyThread(tooltip, TooltipStyleProc, 0)) {
            continue;
        }
        SendMessageW(tooltip, WM_SETFONT, (WPARAM)font, FALSE);
        // No fade: it's drawn from the system's own look before the first paint.
        SetWindowLongPtrW(tooltip, GWL_STYLE, styled.oldStyle | TTS_NOANIMATE | TTS_NOFADE);
        RECT margin{6 * scale, 4 * scale, 6 * scale, 4 * scale};
        SendMessageW(tooltip, TTM_SETMARGIN, 0, (LPARAM)&margin);
        DwmSetWindowAttribute(tooltip, kWindowCornerPreference, &kCornerDoNotRound,
                              sizeof(kCornerDoNotRound));
        g_styledTooltips.push_back(styled);
    }
}

void StopTooltipStyle() {
    for (auto& tip : g_styledTooltips) {
        if (!IsWindow(tip.hwnd)) {
            continue;
        }
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(tip.hwnd, TooltipStyleProc);
        SendMessageW(tip.hwnd, WM_SETFONT, (WPARAM)tip.oldFont, FALSE);
        SendMessageW(tip.hwnd, TTM_SETMARGIN, 0, (LPARAM)&tip.oldMargin);
        SetWindowLongPtrW(tip.hwnd, GWL_STYLE, tip.oldStyle);
    }
    g_styledTooltips.clear();
    for (auto& [scale, font] : g_tooltipFonts) {
        DeleteObject(font);
    }
    g_tooltipFonts.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Everything that runs inside Explorer, in one place: tooltip capture, the
// design on other programs' icons, and the tooltip style. Called every few
// seconds (it only acts on what's new), from the Explorer relay in the split
// suite and from the mod thread when this runs inside Explorer itself.

HWND g_explorerSideTray;  // The taskbar whose existing icons were handled.

void ExplorerSideSettingsChanged(bool loadSettings) {
    if (loadSettings) {
        g_settings = LoadSettings();
    }
    g_themeEnabled = g_settings.trayTheme;
    g_explorerSideTray = nullptr;  // Theme the existing icons again.
    if (!g_settings.tourneTooltips) {
        StopTooltipStyle();
    }
}

// Programs whose state lives in a settings file get their icon redrawn when
// the file changes. One thread, asleep until then.
HANDLE g_appStateThread;
HANDLE g_appStateStop;

DWORD WINAPI AppStateThread(LPVOID) {
    HANDLE watch = FindFirstChangeNotificationW(TamerSettingsFolder().c_str(), FALSE,
                                                FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (watch == INVALID_HANDLE_VALUE) {
        return 0;
    }
    HANDLE events[] = {g_appStateStop, watch};
    while (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1) {
        // Programs save in more than one write; redraw once it has settled.
        if (WaitForSingleObject(g_appStateStop, 200) == WAIT_OBJECT_0) {
            break;
        }
        if (g_themeEnabled) {
            ThemeExistingIcons();
        }
        FindNextChangeNotification(watch);
    }
    FindCloseChangeNotification(watch);
    return 0;
}

void StartAppStateWatch() {
    if (g_appStateThread) {
        return;
    }
    g_appStateStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (g_appStateStop) {
        g_appStateThread = CreateThread(nullptr, 0, AppStateThread, nullptr, 0, nullptr);
    }
}

void StopAppStateWatch() {
    if (g_appStateThread) {
        SetEvent(g_appStateStop);
        WaitForSingleObject(g_appStateThread, 5000);
        CloseHandle(g_appStateThread);
        g_appStateThread = nullptr;
    }
    if (g_appStateStop) {
        CloseHandle(g_appStateStop);
        g_appStateStop = nullptr;
    }
}
void EnsureExplorerSide() {
    EnsureTipCapture();
    HWND tray = g_tipsTrayWnd;
    if (tray && tray != g_explorerSideTray) {
        g_explorerSideTray = tray;
        if (g_themeEnabled) {
            ThemeExistingIcons();
        }
        PokeForTooltips();
    }
    if (g_settings.tourneTooltips) {
        EnsureTooltipStyle();
    }
    StartAppStateWatch();
}

void StopExplorerSide() {
    StopAppStateWatch();
    StopTooltipStyle();
    StopTipCapture();
    g_explorerSideTray = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Click behavior

void OnPrimary(int kind, WPARAM anchor) {
    switch (kind) {
        case kNetwork: ShowFlyout(kFlyoutNetwork, kNetwork); break;
        case kVolume: ShowFlyout(kFlyoutMixer, kVolume); break;
        case kVpn:
            if (!WindscribeFile(L"windscribe-cli.exe").empty()) {
                ShowFlyout(kFlyoutVpn, kVpn);
            } else {
                OpenUri(L"ms-settings:network-vpn");
            }
            break;
        case kBattery: ShowFlyout(kFlyoutPower, kBattery); break;
        case kNotifications:
        case kFocus: ShowFlyout(kFlyoutNotify, kind); break;
        case kNightLight: OpenUri(L"ms-settings:nightlight"); break;
        case kBrightness: ShowFlyout(kFlyoutBrightness, kBrightness); break;
        case kMicrophone: OpenUri(L"ms-settings:privacy-microphone"); break;
        case kCamera: OpenUri(L"ms-settings:privacy-webcam"); break;
        case kLocation: OpenUri(L"ms-settings:privacy-location"); break;
        case kAccessibility: SendHotkey({VK_LWIN, 'U'}); break;
        case kPower: ShowMenu(PowerMenu(), anchor); break;
        case kInput: SendHotkey({VK_LWIN, VK_SPACE}); break;
        case kRecycleBin: OpenUri(L"shell:RecycleBinFolder"); break;
        case kClipboard: SendHotkey({VK_LWIN, 'V'}); break;
        case kCapture: SendHotkey({VK_LWIN, VK_SHIFT, 'S'}); break;
        case kOverflow: ShowFlyout(kFlyoutOverflow, kOverflow); break;
        case kSecurity: ShowFlyout(kFlyoutSecurity, kSecurity); break;
    }
}

void OnContextMenu(int kind, WPARAM anchor) {
    switch (kind) {
        case kNetwork:
        case kVpn:
            ShowMenu({{L"Network && internet settings", Uri(L"ms-settings:network")},
                      {L"Wi-Fi settings", Uri(L"ms-settings:network-wifi")},
                      {L"VPN settings", Uri(L"ms-settings:network-vpn")}},
                     anchor);
            break;
        case kVolume:
            ShowMenu({{L"Open Volume mixer", [] { Run(L"SndVol.exe"); }},
                      {L"Sound settings", Uri(L"ms-settings:sound")},
                      {nullptr, nullptr},
                      {L"Playback devices", [] { Run(L"control.exe", L"mmsys.cpl,,0"); }},
                      {L"Recording devices", [] { Run(L"control.exe", L"mmsys.cpl,,1"); }},
                      {L"Sounds", [] { Run(L"control.exe", L"mmsys.cpl,,2"); }}},
                     anchor);
            break;
        case kBattery:
            ShowMenu({{L"Power && battery settings", Uri(L"ms-settings:powersleep")},
                      {L"Battery saver", Uri(L"ms-settings:batterysaver")}},
                     anchor);
            break;
        case kNotifications:
            ShowMenu({{L"Open notification center", [] { SendHotkey({VK_LWIN, 'N'}); }},
                      {L"Notification settings", Uri(L"ms-settings:notifications")}},
                     anchor);
            break;
        case kFocus:
            ShowMenu({{L"Open notification center", [] { SendHotkey({VK_LWIN, 'N'}); }},
                      {L"Focus settings", Uri(L"ms-settings:quiethours")}},
                     anchor);
            break;
        case kNightLight:
        case kBrightness:
            ShowMenu({{L"Night light settings", Uri(L"ms-settings:nightlight")},
                      {L"Display settings", Uri(L"ms-settings:display")}},
                     anchor);
            break;
        case kMicrophone:
        case kCamera:
        case kLocation:
            OnPrimary(kind, anchor);
            break;
        case kAccessibility:
            ShowMenu({{L"Accessibility settings", Uri(L"ms-settings:easeofaccess")}}, anchor);
            break;
        case kPower:
            ShowMenu(PowerMenu(), anchor);
            break;
        case kInput:
            ShowMenu({{L"Language settings", Uri(L"ms-settings:regionlanguage")},
                      {L"Typing settings", Uri(L"ms-settings:typing")}},
                     anchor);
            break;
        case kRecycleBin:
            ShowMenu({{L"Open", Uri(L"shell:RecycleBinFolder")},
                      {L"Empty Recycle Bin",
                       [] {
                           // Windows asks for confirmation itself.
                           SHEmptyRecycleBinW(g_wnd, nullptr, 0);
                           RequestRefresh(1u << kRecycleBin);
                       }}},
                     anchor);
            break;
        case kClipboard:
            ShowMenu({{L"Clipboard settings", Uri(L"ms-settings:clipboard")}}, anchor);
            break;
        case kCapture:
            ShowMenu({{L"Capture now", [] { SendHotkey({VK_LWIN, VK_SHIFT, 'S'}); }}},
                     anchor);
            break;
        case kOverflow:
            ShowMenu({{L"Taskbar settings", Uri(L"ms-settings:taskbar")}}, anchor);
            break;
        case kSecurity:
            ShowMenu({{L"Open Windows Security", Uri(L"windowsdefender:")},
                      {L"Virus && threat protection", Uri(L"windowsdefender://threat")},
                      {L"Firewall && network protection", Uri(L"windowsdefender://network")}},
                     anchor);
            break;
    }
}

DWORD g_lastMiddleTick;
int g_wheelRemainder;

// The toolbar watcher and the tray callback can both report one middle click.
void OnMiddleClick() {
    DWORD now = GetTickCount();
    if (now - g_lastMiddleTick < 400) {
        return;
    }
    g_lastMiddleTick = now;
    CycleOutputDevice();
}

void OnTrayEvent(int kind, UINT event, WPARAM anchor) {
    bool primary = event == NIN_SELECT || event == NIN_KEYSELECT;
    bool secondary = event == WM_RBUTTONDOWN || event == WM_RBUTTONUP ||
                     event == WM_CONTEXTMENU;

    if (kind == kVolume && event == WM_MBUTTONUP) {
        OnMiddleClick();
        return;
    }

    // Right clicks on network, volume and battery go to the system icon, so
    // its own menu opens; left clicks are the mod's.
    if (secondary && (kind == kNetwork || kind == kVolume || kind == kBattery ||
                      kind == kSecurity)) {
        TrayTarget target;
        if (ForwardTargetFor(kind, &target)) {
            ForwardEvent(target, event, anchor);
            return;
        }
    }

    if (primary) {
        OnPrimary(kind, anchor);
    } else if (event == WM_CONTEXTMENU) {
        OnContextMenu(kind, anchor);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Change notifications

struct NlmSink final : INetworkListManagerEvents {
    LONG refs = 1;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, kIidNetworkListManagerEvents)) {
            *out = static_cast<INetworkListManagerEvents*>(this);
            AddRef();
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&refs); }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG count = InterlockedDecrement(&refs);
        if (!count) {
            delete this;
        }
        return count;
    }
    HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY) override {
        RequestRefresh(kNetworkBits);
        return S_OK;
    }
};

IConnectionPoint* g_nlmConnectionPoint;
DWORD g_nlmCookie;
HANDLE g_ipNotify;
std::array<HPOWERNOTIFY, 3> g_powerNotify;
ULONG g_binNotify;
UINT g_shellHookMessage;
UINT g_taskbarCreatedMessage;

VOID WINAPI OnIpInterfaceChange(PVOID, PMIB_IPINTERFACE_ROW, MIB_NOTIFICATION_TYPE) {
    RequestRefresh(kNetworkBits);
}

VOID WINAPI OnWlanNotification(PWLAN_NOTIFICATION_DATA, PVOID) {
    RequestRefresh(kNetworkBits);
}

void StartNetworkWatch() {
    if (SUCCEEDED(CoCreateInstance(kClsidNetworkListManager, nullptr, CLSCTX_ALL,
                                   kIidNetworkListManager,
                                   (void**)&g_networkListManager))) {
        IConnectionPointContainer* container = nullptr;
        if (SUCCEEDED(g_networkListManager->QueryInterface(
                IID_IConnectionPointContainer, (void**)&container))) {
            if (SUCCEEDED(container->FindConnectionPoint(
                    kIidNetworkListManagerEvents, &g_nlmConnectionPoint))) {
                auto* sink = new NlmSink();
                g_nlmConnectionPoint->Advise(sink, &g_nlmCookie);
                sink->Release();
            }
            container->Release();
        }
    } else {
        Wh_Log(L"Network List Manager unavailable, internet state is unknown");
    }

    NotifyIpInterfaceChange(AF_UNSPEC, OnIpInterfaceChange, nullptr, FALSE,
                            &g_ipNotify);

    DWORD version;
    if (WlanOpenHandle(2, nullptr, &version, &g_wlan) == ERROR_SUCCESS) {
        WlanRegisterNotification(
            g_wlan, WLAN_NOTIFICATION_SOURCE_ACM | WLAN_NOTIFICATION_SOURCE_MSM,
            TRUE, OnWlanNotification, nullptr, nullptr, nullptr);
    } else {
        g_wlan = nullptr;
    }
}

void StopNetworkWatch() {
    if (g_ipNotify) {
        CancelMibChangeNotify2(g_ipNotify);
        g_ipNotify = nullptr;
    }
    if (g_wlan) {
        WlanRegisterNotification(g_wlan, WLAN_NOTIFICATION_SOURCE_NONE, TRUE,
                                 nullptr, nullptr, nullptr, nullptr);
        WlanCloseHandle(g_wlan, nullptr);
        g_wlan = nullptr;
    }
    if (g_nlmConnectionPoint) {
        g_nlmConnectionPoint->Unadvise(g_nlmCookie);
        g_nlmConnectionPoint->Release();
        g_nlmConnectionPoint = nullptr;
    }
    if (g_networkListManager) {
        g_networkListManager->Release();
        g_networkListManager = nullptr;
    }
}

// Registry keys whose changes refresh some icons.
struct RegistryWatch {
    HKEY root;
    std::wstring path;
    UINT bits;
    HKEY key = nullptr;
    HANDLE event = nullptr;
};

std::vector<RegistryWatch> g_watches;

bool ArmWatch(RegistryWatch& watch) {
    if (!watch.event) {
        watch.event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    }
    if (!watch.key &&
        RegOpenKeyExW(watch.root, watch.path.c_str(), 0, KEY_NOTIFY | KEY_READ,
                      &watch.key) != ERROR_SUCCESS) {
        watch.key = nullptr;
        return false;
    }
    if (RegNotifyChangeKeyValue(watch.key, TRUE,
                                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                                watch.event, TRUE) != ERROR_SUCCESS) {
        // The key went away; reopen it next time.
        RegCloseKey(watch.key);
        watch.key = nullptr;
        return false;
    }
    return true;
}

void StartRegistryWatches() {
    std::wstring consent = kConsentStorePath;
    g_watches = {
        {HKEY_LOCAL_MACHINE, kRadioStatePath, kNetworkBits},
        {HKEY_CURRENT_USER, kNotificationsPath, 1u << kNotifications},
        {HKEY_CURRENT_USER, kNightLightPath, 1u << kNightLight},
        {HKEY_CURRENT_USER, consent + L"microphone", 1u << kMicrophone},
        {HKEY_CURRENT_USER, consent + L"webcam", 1u << kCamera},
        {HKEY_CURRENT_USER, consent + L"location", 1u << kLocation},
    };
    for (auto& watch : g_watches) {
        ArmWatch(watch);
    }
}

void StopRegistryWatches() {
    for (auto& watch : g_watches) {
        if (watch.key) {
            RegCloseKey(watch.key);
        }
        if (watch.event) {
            CloseHandle(watch.event);
        }
    }
    g_watches.clear();
}

void StartWindowWatches(HWND hWnd) {
    g_powerNotify[0] = RegisterPowerSettingNotification(
        hWnd, &kGuidBatteryPercentage, DEVICE_NOTIFY_WINDOW_HANDLE);
    g_powerNotify[1] = RegisterPowerSettingNotification(
        hWnd, &kGuidAcDcPowerSource, DEVICE_NOTIFY_WINDOW_HANDLE);
    g_powerNotify[2] = RegisterPowerSettingNotification(
        hWnd, &kGuidPowerSavingStatus, DEVICE_NOTIFY_WINDOW_HANDLE);

    PIDLIST_ABSOLUTE bin = nullptr;
    if (SUCCEEDED(SHGetSpecialFolderLocation(nullptr, CSIDL_BITBUCKET, &bin))) {
        SHChangeNotifyEntry entry{bin, TRUE};
        g_binNotify = SHChangeNotifyRegister(
            hWnd, SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
            SHCNE_CREATE | SHCNE_DELETE | SHCNE_MKDIR | SHCNE_RMDIR |
                SHCNE_RENAMEITEM | SHCNE_RENAMEFOLDER | SHCNE_UPDATEDIR |
                SHCNE_UPDATEITEM,
            WM_APP_BIN, 1, &entry);
        CoTaskMemFree(bin);
    }

    g_shellHookMessage = RegisterWindowMessageW(L"SHELLHOOK");
    RegisterShellHookWindow(hWnd);
}

void StopWindowWatches(HWND hWnd) {
    for (auto& notify : g_powerNotify) {
        if (notify) {
            UnregisterPowerSettingNotification(notify);
            notify = nullptr;
        }
    }
    if (g_binNotify) {
        SHChangeNotifyDeregister(g_binNotify);
        g_binNotify = 0;
    }
    DeregisterShellHookWindow(hWnd);
}

////////////////////////////////////////////////////////////////////////////////
// Mod window and thread

// Explorer can run folder windows in extra processes; only the one hosting
// the taskbar gets the icons.
bool IsTaskbarProcess() {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD pid = 0;
    return tray && GetWindowThreadProcessId(tray, &pid) &&
           pid == GetCurrentProcessId();
}

bool g_started;

void Start(HWND hWnd) {
    g_started = true;
    StartNetworkWatch();
    StartAudio();
    StartRegistryWatches();
    StartWindowWatches(hWnd);
    WscRegisterForChanges(nullptr, &g_securityWatch, OnSecurityChange, nullptr);
    WarmBrightnessCache(hWnd);
    g_notificationsWatch = FindFirstChangeNotificationW(
        NotificationsFolder().c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE);
    RefreshCalendar(false);
    ExplorerSideSettingsChanged(false);
    EnsureExplorerSide();
    HookToolbars();
    SetTimer(hWnd, kTimerPeriodic, kPeriodicMs, nullptr);
    RequestRefresh(kAllBits);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static const UINT notificationCentre = RegisterWindowMessageW(L"TourneTray_NotificationCenter");
    if (msg == notificationCentre) {
        // The Tourne clock was clicked: a toggle, like the tray icons. A click
        // on the taskbar doesn't take the focus, so an open flyout is still
        // open here and closes; one the click just closed stays closed.
        if (IsFlyoutOpen(kFlyoutNotify)) {
            CloseFlyout();
        } else {
            ShowFlyout(kFlyoutNotify, -1);
        }
        return 0;
    }

    if (msg == WM_APP_TRAY) {
        UINT event = LOWORD(lParam);
        int kind = (int)HIWORD(lParam) - (int)kIconIdBase;
        if (kind >= 0 && kind < kIconCount) {
            OnTrayEvent(kind, event, wParam);
        }
        return 0;
    }

    if (g_taskbarCreatedMessage && msg == g_taskbarCreatedMessage) {
        for (auto& applied : g_applied) {
            applied = {};
        }
        HookToolbars();
        RequestRefresh(kAllBits);
        return 0;
    }

    if (g_shellHookMessage && msg == g_shellHookMessage) {
        if (wParam == HSHELL_LANGUAGE || wParam == HSHELL_WINDOWACTIVATED ||
            wParam == HSHELL_RUDEAPPACTIVATED) {
            RequestRefresh(1u << kInput);
        }
        return 0;
    }

    switch (msg) {
        case WM_APP_REFRESH:
            g_refreshPosted = false;
            if (g_started) {
                Refresh(g_pendingBits.exchange(0));
            }
            return 0;

        case WM_APP_SETTINGS:
            g_settings = LoadSettings();
            ExplorerSideSettingsChanged(false);
            RequestRefresh(kAllBits);
            return 0;

        case WM_APP_CALENDAR_READY:
            g_calendarFetching = false;
            if (IsFlyoutOpen(kFlyoutNotify)) {
                PostMessageW(g_flyout.hwnd, WM_APP_FLYOUT_RELOAD, 0, 0);
            }
            return 0;

        case WM_APP_BRIGHTNESS_READY:
            FinishBrightnessWarm();
            return 0;

        case WM_APP_BIN:
            RequestRefresh(1u << kRecycleBin);
            return 0;

        case WM_APP_WHEEL: {
            g_wheelRemainder += (int)(short)wParam;
            int notches = g_wheelRemainder / WHEEL_DELTA;
            g_wheelRemainder -= notches * WHEEL_DELTA;
            ChangeVolume(notches);
            return 0;
        }

        case WM_APP_MIDDLE:
            OnMiddleClick();
            return 0;

        case WM_APP_AUDIO_DEVICE:
            BindAudioEndpoint();
            RequestRefresh(1u << kVolume);
            return 0;

        case WM_POWERBROADCAST:
            if (wParam == PBT_POWERSETTINGCHANGE || wParam == PBT_APMPOWERSTATUSCHANGE) {
                RequestRefresh(1u << kBattery);
            } else if (wParam == PBT_APMRESUMEAUTOMATIC) {
                RequestRefresh(kAllBits);
            }
            return TRUE;

        case WM_SETTINGCHANGE:
        case WM_DISPLAYCHANGE:
            // Re-renders only if the tray icon size changed with the DPI.
            RequestRefresh(0);
            return 0;

        case WM_TIMER:
            if (wParam == kTimerNotifications) {
                KillTimer(hWnd, kTimerNotifications);
                RequestRefresh(1u << kNotifications);
                return 0;
            }
            if (wParam == kTimerFindTray) {
                if (FindWindowW(L"Shell_TrayWnd", nullptr)) {
                    KillTimer(hWnd, kTimerFindTray);
                    if (IsTaskbarProcess()) {
                        Start(hWnd);
                    } else {
                        Wh_Log(L"Not the taskbar process, staying idle");
                    }
                }
            } else if (wParam == kTimerPeriodic) {
                // Battery time left, Recycle Bin, retries for icons the tray
                // refused, toolbars that were recreated, and registry keys
                // that didn't exist yet (night light's is only created once
                // it's first used).
                for (auto& watch : g_watches) {
                    if (!watch.key && ArmWatch(watch)) {
                        RequestRefresh(watch.bits);
                    }
                }
                HookToolbars();
                EnsureExplorerSide();
                RequestRefresh((1u << kBattery) | (1u << kRecycleBin) |
                               kNetworkBits);
            }
            return 0;

        case WM_APP_EXIT:
            CloseFlyout();
            StopExplorerSide();
            UnhookToolbars();
            for (int k = 0; k < kIconCount; k++) {
                RemoveIcon(k);
            }
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, kTimerPeriodic);
            KillTimer(hWnd, kTimerFindTray);
            if (g_started) {
                StopWindowWatches(hWnd);
                if (g_securityWatch) {
                    WscUnRegisterChanges(g_securityWatch);
                    g_securityWatch = nullptr;
                }
                FinishBrightnessWarm();
                if (g_notificationsWatch != INVALID_HANDLE_VALUE) {
                    FindCloseChangeNotification(g_notificationsWatch);
                    g_notificationsWatch = INVALID_HANDLE_VALUE;
                }
            }
            g_wnd = nullptr;
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

constexpr WCHAR kWindowClass[] = L"TourneTraySystemIcons_" WH_MOD_ID;

HANDLE g_thread;

DWORD WINAPI ThreadProc(LPVOID parameter) {
    HANDLE ready = (HANDLE)parameter;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&WndProc, &instance);

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = WndProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kWindowClass;
    RegisterClassW(&windowClass);

    WNDCLASSW flyoutClass{};
    flyoutClass.style = CS_DROPSHADOW | CS_DBLCLKS;
    flyoutClass.lpfnWndProc = FlyoutProc;
    flyoutClass.hInstance = instance;
    flyoutClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    flyoutClass.lpszClassName = kFlyoutClass;
    RegisterClassW(&flyoutClass);

    g_settings = LoadSettings();
    g_taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");

    // A hidden top-level window: TaskbarCreated, WM_SETTINGCHANGE and power
    // notifications only reach top-level windows.
    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW, kWindowClass, L"", WS_POPUP, 0,
                                0, 0, 0, nullptr, nullptr, instance, nullptr);
    g_wnd = hWnd;
    SetEvent(ready);

    if (hWnd) {
        if (FindWindowW(L"Shell_TrayWnd", nullptr)) {
            if (IsTaskbarProcess()) {
                Start(hWnd);
            } else {
                Wh_Log(L"Not the taskbar process, staying idle");
            }
        } else {
            // Explorer is still starting; wait for the taskbar.
            SetTimer(hWnd, kTimerFindTray, 500, nullptr);
        }

        bool running = true;
        while (running) {
            std::vector<HANDLE> events;
            for (auto& watch : g_watches) {
                if (watch.key && watch.event) {
                    events.push_back(watch.event);
                }
            }
            if (g_notificationsWatch != INVALID_HANDLE_VALUE) {
                events.push_back(g_notificationsWatch);
            }
            DWORD result = MsgWaitForMultipleObjectsEx(
                (DWORD)events.size(), events.data(), INFINITE, QS_ALLINPUT,
                MWMO_INPUTAVAILABLE);
            if (result < WAIT_OBJECT_0 + events.size()) {
                if (events[result - WAIT_OBJECT_0] == g_notificationsWatch) {
                    // Windows writes the database in bursts; one refresh
                    // after it settles.
                    FindNextChangeNotification(g_notificationsWatch);
                    SetTimer(g_wnd, kTimerNotifications, 250, nullptr);
                    continue;
                }
                for (auto& watch : g_watches) {
                    if (watch.event == events[result - WAIT_OBJECT_0]) {
                        RequestRefresh(watch.bits);
                        ArmWatch(watch);
                        break;
                    }
                }
                continue;
            }
            MSG msg;
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }

    StopRegistryWatches();
    StopAudio();
    StopNetworkWatch();
    DestroyFonts();
    UnregisterClassW(kFlyoutClass, instance);
    UnregisterClassW(kWindowClass, instance);
    CoUninitialize();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Mod lifecycle

BOOL Wh_ModInit() {
    Wh_Log(L">");

    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, ThreadProc, ready, 0, nullptr);
    if (!g_thread) {
        CloseHandle(ready);
        return FALSE;
    }
    WaitForSingleObject(ready, 5000);
    CloseHandle(ready);
    return TRUE;
}

void Wh_ModUninit() {
    Wh_Log(L">");

    if (HWND wnd = g_wnd) {
        PostMessageW(wnd, WM_APP_EXIT, 0, 0);
    }
    if (g_thread) {
        WaitForSingleObject(g_thread, 10000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
    // A brightness worker can be mid DDC/CI write; its code must stay loaded
    // until it returns.
    for (HANDLE thread : g_brightnessThreads) {
        WaitForSingleObject(thread, 5000);
        CloseHandle(thread);
    }
    g_brightnessThreads.clear();
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");

    if (HWND wnd = g_wnd) {
        PostMessageW(wnd, WM_APP_SETTINGS, 0, 0);
    }
}
