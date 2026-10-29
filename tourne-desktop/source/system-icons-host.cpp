// ==WindhawkMod==
// @id              tourne-tray-system-icons
// @name            Tourne'Tray [System Icons]
// @description     Pixel-art tray icons for network, volume, battery, notifications, privacy and more, with live state and Windows 7 style flyouts
// @version         1.1.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lwlanapi -liphlpapi -lpowrprof -lshell32 -lgdi32 -ladvapi32 -lcomctl32 -luxtheme -luuid -lmsimg32 -ldxva2 -lwscapi -lcfgmgr32 -ldwmapi -lversion -lshlwapi -lruntimeobject -lurlmon -lwininet -lhid
// @license         GPL-3.0
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
| Bluetooth | On, off, or connected with the device names | Bluetooth flyout: on or off, paired devices, connect or disconnect audio devices | Windows' own Bluetooth menu |
| Device batteries | One per wireless device: its level as a halo, charging, asleep, low | Devices flyout | Check now, Bluetooth settings |

## Device batteries

Headsets, mice, keyboards and controllers are read straight from the device,
the way HaloBattery (MIT) does it: Alienware, Razer, Logitech, SteelSeries,
HyperX, Audeze, MCHOSE, WLmouse, PlayStation and Xbox controllers, plus any
Bluetooth device that reports to Windows. Pick the brands you own under
**Devices you have**. Each can show as its own tray icon, inside the mixer
and Bluetooth flyouts, or both. Only read commands are ever sent.

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
- scheme: tourne
  $name: Colour scheme
  $description: >-
    The colours of the flyouts, the tray icons, the hidden icons flyout and the
    Tourne tooltips. Tourne is the original; the rest are the palettes of the
    ricing community, to match a system themed with one of them. The first two
    take the colours of a theme's menus: the taskbar's theme (or the one
    StartAllBack is set to), or Windows' own visual style, and follow it when it
    changes. A taskbar theme without menus of its own uses Windows' visual
    style. Tourne theme takes the colours Tourne Desktop makes from your
    wallpaper (or one seed colour) and follows them as they change.
  $options:
  - tourne: Tourne
  - taskbarStyle: From the taskbar's visual style
  - visualStyle: From Windows' visual style
  - tourneTheme: Tourne theme (Tourne Desktop's, from your wallpaper)
  - gruvboxDarkHard: Gruvbox dark (hard)
  - gruvboxDark: Gruvbox dark
  - gruvboxDarkSoft: Gruvbox dark (soft)
  - gruvboxLight: Gruvbox light
  - monokai: Monokai
  - catppuccinMocha: Catppuccin Mocha
  - catppuccinMacchiato: Catppuccin Macchiato
  - catppuccinFrappe: Catppuccin Frappé
  - catppuccinLatte: Catppuccin Latte
  - everforestDarkHard: Everforest dark (hard)
  - everforestDark: Everforest dark
  - everforestDarkSoft: Everforest dark (soft)
  - everforestLightHard: Everforest light (hard)
  - everforestLight: Everforest light
  - everforestLightSoft: Everforest light (soft)
  - nord: Nord
  - dracula: Dracula
  - tokyoNight: Tokyo Night
  - tokyoNightStorm: Tokyo Night Storm
  - rosePine: Rosé Pine
  - rosePineMoon: Rosé Pine Moon
  - rosePineDawn: Rosé Pine Dawn
  - kanagawa: Kanagawa
  - oneDark: One Dark
  - solarizedDark: Solarized dark
  - solarizedLight: Solarized light
- tone: ink
  $name: Icon tone
  $description: >-
    Which of the colour scheme's colours the icons are drawn in; they change
    with the scheme.
  $options:
  - ink: Text (Ink)
  - sage: Secondary text (Sage)
  - alt: Faint text (Alt)
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
- calendarAdd: false
  $name: Right-click a day to add an event (Google Calendar, online)
  $description: >-
    A right click on a day in the notification centre's calendar opens a
    small window for a new event: its title, day, all day or a time, place,
    importance and notes. "Add to Google Calendar" then opens Google
    Calendar's own new-event page in your browser with all of it filled in,
    and one click there saves it. It then reaches your other devices, and
    this agenda the next time it reads your calendar (Google can take a
    while to update its .ics address). This part is online because Google
    Calendar itself saves the event, in a browser signed in to your Google
    account: the suite only opens that page, and sends nothing anywhere
    itself or keeps any account or password. Google Calendar has no
    importance of its own, so High puts "!" before the title and the
    importance is written in the notes. Tasks can't be made this way:
    Google Tasks has no such page.
- calendarAccounts: ""
  $name: Google accounts for new events
  $description: >-
    The email of the Google account whose calendar new events go to, when
    it isn't your browser's default Google account (several, separated by
    commas, to pick one in the event window; the first is chosen to begin
    with). The page then opens as that account, which has to be signed in
    in the browser. Empty uses the browser's default account.
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
- flyoutWidth: auto
  $name: Flyout width
  $description: >-
    Auto widens a flyout until its longest line fits, up to the maximum
    below. Fixed keeps each flyout at its standard width and shortens long
    lines with "...".
  $options:
  - auto: Auto, fit the text
  - fixed: Fixed
- flyoutMaxWidth: 480
  $name: Flyout maximum width (px)
  $description: >-
    How wide Auto may make a flyout, at 100% display scaling. Lines longer
    than this are still shortened with "...".
  #! $min: 300
  #! $max: 1200
- notificationPopups: auto
  $name: Pop-ups for new notifications
  $description: >-
    A pop-up in the flyouts' style when a notification arrives, with its
    buttons. Auto shows them only where Windows' own pop-ups don't run (in
    Tourne Desktop's shell mode, without Explorer).
  $options:
  - auto: Auto, where Windows' own don't run
  - never: Never
- notificationPopupSeconds: 6
  $name: How long a notification pop-up stays (seconds)
  $description: Hovering it keeps it open.
- notificationPhone: false
  $name: Your phone in the notification centre
  $description: >-
    With a phone linked in Phone Link: its name, battery and connection
    at the top of the notification centre, with Messages, Send a file and
    Phone Link. Its notifications are in the list under each phone app's
    own name.
- flyoutAway: 0
  $name: Flyout extra distance from the taskbar (px)
  $description: >-
    Added to the usual 8 px between a flyout and the taskbar's edge, at 100%
    display scaling; negative brings it closer.
- flyoutAlong: 0
  $name: Flyout shift along the taskbar (px)
  $description: >-
    Moves every flyout sideways from its icon (up or down on a taskbar at the
    side); negative goes left (or up).
- menuPlace: pointer
  $name: Where the icons' right-click menus open
  $description: >-
    At the pointer, as Windows opens them, or clear of the taskbar next to
    the icon, so the menu covers neither the taskbar nor its tooltips.
  $options:
  - pointer: At the pointer
  - taskbar: Clear of the taskbar, by the icon
- menuAlign: centre
  $name: Menu lined up with the icon
  $description: >-
    With "Clear of the taskbar": which way the menu reaches from its icon.
  $options:
  - after: Starting at the icon (reaching right, or down)
  - centre: Centred on the icon
  - before: Ending at the icon (reaching left, or up)
- menuAway: 0
  $name: Menu distance from the taskbar (px)
  $description: >-
    With "Clear of the taskbar": the gap between the taskbar and the menu, at
    100% display scaling.
- menuAlong: 0
  $name: Menu shift along the taskbar (px)
  $description: >-
    With "Clear of the taskbar": moves the menu sideways from its icon;
    negative goes left (or up).
- overflowFolders: true
  $name: Folders in the hidden icons flyout
  $description: >-
    Groups the hidden icons into the folders set up under Classic Tray
    Folders (whether or not that part is on), as tiles that open to their
    icons. Works with or without StartAllBack.
- nearbyWifi: list
  $name: Nearby Wi-Fi networks
  $description: >-
    Windows treats the list of nearby Wi-Fi networks as location data. So
    when the network flyout lists them, Windows may ask whether to allow
    location access, or show Windhawk under "Recently used" in its location
    settings. Nothing about your location is read or sent anywhere; the
    flyout only gets network names and signal strength. If Windows withholds
    the list, the flyout says so, with a link to location settings. Without
    the note keeps the list but drops that message (for location turned off
    on purpose). Off never asks Windows for nearby networks: the flyout shows
    only your current connection, plus a link to Windows' own list.
  $options:
  - list: List them
  - quiet: List them, without the location note
  - off: "Off"
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
  - bluetooth: true
    $name: Bluetooth
    $description: >-
      On, off or connected, with a flyout to turn Bluetooth on or off and to
      connect or disconnect paired audio devices.
  $name: Icons
  $description: >-
    Icons to add. Which ones show on the taskbar is then chosen in Windows
    Settings, like any other tray icon.
- batteries:
  - trayIcons: true
    $name: Tray icon per device
    $description: >-
      A tray icon for each wireless device with a battery reading, its level
      drawn as a halo around the device's pictogram. Click it for the devices
      flyout.
  - inFlyouts: true
    $name: In the flyouts
    $description: >-
      Headset batteries in the volume mixer, and the other devices in the
      Bluetooth flyout.
  - pollInterval: 1min
    $name: Check every
    $description: >-
      How often the devices are asked for their level: a number followed by
      hr for hours, min for minutes or sec for seconds, e.g. 9hr, 9min, 1.5hr
      or 30sec. A number on its own counts as minutes. The longest allowed is
      24 hours (24hr) between checks, and the shortest 5 seconds. A device
      being plugged in or removed is checked at once, whatever this is.
  - lowAlert: 20
    $name: Low battery alert (%)
    $description: >-
      A notification once when a device drops to this level, again only after
      it was charged. 0 turns it off.
    #! $min: 0
    #! $max: 50
  - pictogram: true
    $name: Device pictogram
    $description: The headset, mouse, keyboard or gamepad inside the halo.
  - chargingAnimation: true
    $name: Charging animation
    $description: The halo breathes while a device charges.
  - vendors:
    - alienware: true
      $name: Alienware
      $description: Pro Wireless headset, AW720M mouse
    - razer: false
      $name: Razer
    - logitech: false
      $name: Logitech
    - steelseries: false
      $name: SteelSeries
    - hyperx: false
      $name: HyperX
    - audeze: false
      $name: Audeze
    - mchose: false
      $name: MCHOSE
    - wlmouse: false
      $name: WLmouse
    - playstation: false
      $name: PlayStation controllers
    - xbox: true
      $name: Xbox controllers
      $description: And other controllers that speak the Xbox protocol.
    - bluetooth: true
      $name: Windows Bluetooth devices
      $description: Any Bluetooth device that reports its battery to Windows.
    $name: Devices you have
    $description: >-
      Only the readers turned on here run, so devices you don't own cost
      nothing.
  $name: Device batteries
  $description: >-
    Battery levels of wireless headsets, mice, keyboards and controllers, read
    straight from the device (ported from HaloBattery, MIT).
*/
// ==/WindhawkModSettings==

#include <winsock2.h>
#include <ws2ipdef.h>

#include <windhawk_utils.h>

#include <windows.h>
#include <cfgmgr32.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <audiopolicy.h>
#include <devicetopology.h>
#include <endpointvolume.h>
#include <hidusage.h>
#include <hidpi.h>
#include <hidsdi.h>
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
#include <uxtheme.h>
#include <vssym32.h>
#include <wininet.h>
#include <windowsx.h>
#include <wincodec.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Power.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.System.Power.h>
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
    kBluetooth,
    kIconCount
};

constexpr PCWSTR kIconKeys[kIconCount] = {
    L"network",       L"vpn",        L"battery",       L"notifications",
    L"focus",         L"nightLight", L"brightness",    L"microphone",
    L"camera",        L"location",   L"accessibility", L"power",
    L"input",         L"recycleBin", L"clipboard",     L"capture",
    L"volume",        L"overflow",   L"security",      L"bluetooth",
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
constexpr UINT WM_APP_SCAN_DONE = WM_APP + 14;  // wParam: the scan type; lParam: MpCmdRun's exit code
constexpr UINT WM_APP_FLYOUT_RENAME_DONE = WM_APP + 15;  // a tray folder's name box lost the focus: what's typed is kept

constexpr UINT_PTR kTimerPeriodic = 1;
constexpr UINT_PTR kTimerFindTray = 2;
constexpr UINT_PTR kTimerNotifications = 4;
constexpr UINT_PTR kTimerNotifyCounter = 8;  // the notification database's change counter, compared each second
constexpr UINT_PTR kTimerStackWait = 9;      // a dragged hidden icon held over another: then they'd make a folder
constexpr UINT_PTR kTimerMotion = 10;        // a flyout, pop-up or the event window opening (the motion lines)
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

// @inline palettes.cpp
// @inline theme-palette.cpp

Palette g_palette = SchemePalette(L"tourne");
void ApplyPaletteToUi();

// Which colours an icon was drawn in: a change of scheme redraws every icon.
uint64_t PaletteKey() {
    auto const& p = g_palette;
    uint64_t hash = 14695981039346656037ull;
    for (uint32_t v : {p.ground, p.raised, p.line, p.edge, p.text, p.accent, p.muted, p.dim, p.warn,
                       p.bad, p.hover}) {
        hash = (hash ^ v) * 1099511628211ull;
    }
    return hash;
}

// @inline program-icons.cpp

struct Settings {
    std::wstring scheme = L"tourne";
    std::string tone = "sage";
    bool shade = true;
    std::array<bool, kIconCount> enabled{};
    bool trayTheme = true;
    bool tourneTooltips = true;
    std::wstring calendarIcs;
    bool calendarAdd = false;  // a right click on a day: a new event, through Google Calendar's page
    std::vector<std::wstring> calendarAccounts;  // Google accounts to open that page as (first: the default)
    std::wstring doubleClickApps;
    bool flyoutAutoWidth = true;
    int flyoutMaxWidth = 480;  // At 100% scale.
    bool overflowFolders = true;
    bool allAppIcons = false;  // nothing starts in the hidden icons flyout
    int flyoutAway = 0, flyoutAlong = 0;  // At 100% scale; away is added to the usual 8 px.
    bool notificationPopups = true;       // where Windows' own don't run
    int notificationPopupSeconds = 6;
    bool notificationPhone = false;       // the linked phone at the top of the notification centre
    bool menuByTaskbar = false;           // the icons' menus clear of the taskbar, not at the pointer
    int menuAlign = 0;                    // -1 ending at the icon, 0 centred, 1 starting at it
    int menuAway = 0, menuAlong = 0;      // At 100% scale.
    bool nearbyWifi = true;    // list nearby networks (Windows counts that as location data)
    bool locationNote = true;  // say so when Windows withholds the list
    std::vector<ProgramIcon> programIcons;  // the taskbar's Button icons used everywhere
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
    {
        WindhawkUtils::StringSetting scheme = WindhawkUtils::StringSetting::make(L"scheme");
        settings.scheme = scheme.get();
        g_palette = SuiteSchemePalette(settings.scheme);
        ApplyPaletteToUi();
    }
    settings.trayTheme = Wh_GetIntSetting(L"trayTheme");
    settings.tourneTooltips = Wh_GetIntSetting(L"tourneTooltips");
    settings.calendarIcs = WindhawkUtils::StringSetting::make(L"calendarIcs").get();
    settings.calendarAdd = Wh_GetIntSetting(L"calendarAdd") != 0;
    std::wstring accounts = WindhawkUtils::StringSetting::make(L"calendarAccounts").get();
    for (size_t start = 0; start < accounts.size();) {
        size_t end = accounts.find_first_of(L",; ", start);
        if (end == std::wstring::npos) end = accounts.size();
        if (end > start && accounts.find(L'@', start) < end) settings.calendarAccounts.push_back(accounts.substr(start, end - start));
        start = end + 1;
    }
    settings.doubleClickApps = WindhawkUtils::StringSetting::make(L"doubleClickApps").get();
    settings.programIcons = EverywhereIcons();
    // Unset (an installed mod that predates these) means the defaults.
    settings.flyoutAutoWidth = wcscmp(WindhawkUtils::StringSetting::make(L"flyoutWidth").get(), L"fixed") != 0;
    int maxWidth = Wh_GetIntSetting(L"flyoutMaxWidth");
    settings.flyoutMaxWidth = maxWidth <= 0 ? 480 : std::clamp(maxWidth, 300, 1200);
    settings.overflowFolders = Wh_GetIntSetting(L"overflowFolders") != 0;
    // Unset (an installed mod that predates these): as before, menus at the pointer.
    settings.flyoutAway = std::clamp(Wh_GetIntSetting(L"flyoutAway"), -8, 400);
    settings.notificationPopups = wcscmp(WindhawkUtils::StringSetting::make(L"notificationPopups").get(), L"never") != 0;
    int seconds = Wh_GetIntSetting(L"notificationPopupSeconds");
    settings.notificationPopupSeconds = seconds <= 0 ? 6 : std::clamp(seconds, 2, 60);
    settings.notificationPhone = Wh_GetIntSetting(L"notificationPhone") != 0;
    settings.flyoutAlong = std::clamp(Wh_GetIntSetting(L"flyoutAlong"), -2000, 2000);
    settings.menuByTaskbar = !wcscmp(WindhawkUtils::StringSetting::make(L"menuPlace").get(), L"taskbar");
    auto menuAlign = WindhawkUtils::StringSetting::make(L"menuAlign");
    settings.menuAlign = !wcscmp(menuAlign.get(), L"after") ? 1 : !wcscmp(menuAlign.get(), L"before") ? -1 : 0;
    settings.menuAway = std::clamp(Wh_GetIntSetting(L"menuAway"), -200, 400);
    settings.menuAlong = std::clamp(Wh_GetIntSetting(L"menuAlong"), -2000, 2000);
    auto nearbyWifi = WindhawkUtils::StringSetting::make(L"nearbyWifi");
    settings.nearbyWifi = wcscmp(nearbyWifi.get(), L"off") != 0;  // (unset: list)
    settings.locationNote = settings.nearbyWifi && wcscmp(nearbyWifi.get(), L"quiet") != 0;
    constexpr PCWSTR kAllAppIcons = L"taskbar.allAppIcons";  // the Taskbar part's setting, by full name
    settings.allAppIcons = Wh_GetIntSetting(kAllAppIcons) != 0;
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
// The media keys, for Tourne Desktop's flyouts (the same pixel look as the tray's).
const Grid kArtMediaPlay = P({"", "", "...#........", "...##.......", "...###......", "...####.....", "...#####....",
                              "...####.....", "...###......", "...##.......", "...#........"});
const Grid kArtMediaPlayPause = P({"", "", "", ".#.....##.##", ".##....##.##", ".###...##.##", ".####..##.##",
                                   ".###...##.##", ".##....##.##", ".#.....##.##"});
const Grid kArtMediaNext = P({"", "", "", "#....#....##", "##...##...##", "###..###..##", "####.####.##",
                              "###..###..##", "##...##...##", "#....#....##"});
const Grid kArtMediaPrev = P({"", "", "", "##....#....#", "##...##...##", "##..###..###", "##.####.####",
                              "##..###..###", "##...##...##", "##....#....#"});
const Grid kArtMediaStop = P({"", "", "..########..", "..########..", "..########..", "..########..",
                              "..########..", "..########..", "..########..", "..########.."});
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
// Phone apps' (the Start menu's phone pane and the notification centre): mail, Reddit, video, shopping.
const Grid kArtMail = P({"", "", "############", "##........##", "#.#......#.#", "#..#....#..#", "#...#..#...#",
                         "#....##....#", "#..........#", "############"});
const Grid kArtReddit = P({"........###.", "........#.#.", ".......####.", "......#.....", "..########..",
                           ".#........#.", "#..oo..oo..#", "#..oo..oo..#", "#..........#", "#..#....#..#",
                           ".#..####..#.", "..########.."});
const Grid kArtPlay = P({"", "############", "#..........#", "#...#......#", "#...##.....#", "#...###....#",
                         "#...####...#", "#...###....#", "#...##.....#", "#...#......#", "#..........#", "############"});
const Grid kArtBag = P({"", "....####....", "...#....#...", "...#....#...", "############", "#..........#",
                        "#..#....#..#", "#..........#", "#..........#", "#..........#", "#..........#", "############"});
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
// Device batteries: a halo around the device's pictogram. The canvas edge
// fills clockwise from the top with the level, and the pictogram sits inside.
// Ids are "devbat:<kind>:<level>:<state>[:nopict]"; the state is normal,
// warn, low, charge, charge2 (the other breathing frame), asleep or unknown.
const std::map<std::string_view, std::array<std::string_view, 8>> kDevicePictograms = {
    {"headset", {"..####..", ".#....#.", "#......#", "#......#", "##....##", "##....##", "##....##", "........"}},
    {"mouse", {"..####..", ".#.##.#.", ".#.##.#.", ".######.", ".#....#.", ".#....#.", ".#....#.", "..####.."}},
    {"keyboard", {"........", "........", "########", "#.#.#.##", "##.#.#.#", "#.####.#", "########", "........"}},
    {"gamepad", {"........", ".######.", "#......#", "#.#..#.#", "###.#.##", "#.#..#.#", "#.####.#", "##....##"}},
    {"bluetooth", {"...#....", "...##...", ".#.#.#..", "..###...", "..###...", ".#.#.#..", "...##...", "...#...."}},
};

// The 44 edge cells, clockwise from the middle of the top.
std::array<std::pair<int, int>, 44> HaloCells() {
    std::array<std::pair<int, int>, 44> cells;
    int n = 0;
    for (int x = 6; x <= 11; x++) cells[n++] = {x, 0};
    for (int y = 1; y <= 11; y++) cells[n++] = {11, y};
    for (int x = 10; x >= 0; x--) cells[n++] = {x, 11};
    for (int y = 10; y >= 0; y--) cells[n++] = {0, y};
    for (int x = 1; x <= 5; x++) cells[n++] = {x, 0};
    return cells;
}

const char* BaseToneName();

Art DeviceBatteryArt(std::string_view id) {
    std::vector<std::string_view> parts;
    for (size_t start = 0;;) {
        size_t colon = id.find(':', start);
        parts.push_back(id.substr(start, colon == std::string_view::npos ? std::string_view::npos : colon - start));
        if (colon == std::string_view::npos) {
            break;
        }
        start = colon + 1;
    }
    if (parts.size() < 4) {
        return {P({})};
    }
    std::string_view kind = parts[1];
    int level = atoi(std::string(parts[2]).c_str());
    std::string_view state = parts[3];
    bool pictogram = !(parts.size() > 4 && parts[4] == "nopict");

    Grid g = P({});
    static const auto cells = HaloCells();
    int filled = level < 0 ? 0 : (level * 44 + 50) / 100;
    if (level > 0 && filled == 0) {
        filled = 1;
    }
    for (int i = 0; i < 44; i++) {
        g[cells[i].second][cells[i].first] = i < filled ? 'o' : 'd';
    }
    if (pictogram) {
        auto it = kDevicePictograms.find(kind);
        auto const& rows = it != kDevicePictograms.end() ? it->second : kDevicePictograms.at("bluetooth");
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (rows[y][x] == '#') {
                    g[y + 2][x + 2] = '#';
                }
            }
        }
    }
    const char* accent = state == "warn"      ? "coral"
                         : state == "low"     ? "red"
                         : state == "charge"  ? "green"
                         : state == "charge2" ? "alt"
                         : state == "asleep"  ? "alt"
                                              : BaseToneName();
    return {g, state == "asleep" ? "alt" : nullptr, accent};
}

Art ArtFor(std::string_view id) {
    if (id.starts_with("devbat:")) return DeviceBatteryArt(id);
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
    if (id == "media-play") return {kArtMediaPlay};
    if (id == "media-playpause") return {kArtMediaPlayPause};
    if (id == "media-next") return {kArtMediaNext};
    if (id == "media-prev") return {kArtMediaPrev};
    if (id == "media-stop") return {kArtMediaStop};
    if (id == "app-bin") return {kArtTrash};
    if (id == "app-clipboard") return {kArtClipboard};
    if (id == "app-capture") return {kArtCamera};
    if (id == "overflow-up") return {kArtChevronUp, "ink"};
    if (id == "overflow-down") return {kArtChevronDown, "ink"};
    if (id == "security-ok") return {Badge(kArtShield, "ok"), nullptr, nullptr, "ok"};
    if (id == "security-warn") return {Badge(kArtShield, "warn"), nullptr, nullptr, "warn"};
    if (id == "security-alert") return {Badge(kArtShield, "err"), nullptr, nullptr, "err"};
    if (id == "security-off") return {Badge(kArtShield, "off"), "alt", nullptr, "off"};
    if (id == "security-scan") return {Badge(kArtShield, "dot"), nullptr, nullptr, "dot"};
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
    if (id == "app-mail") return {kArtMail};
    if (id == "app-reddit") return {kArtReddit, nullptr, "coral"};
    if (id == "app-video") return {kArtPlay};
    if (id == "app-shop") return {kArtBag};
    return {P({})};
}

const char* BaseToneName() {
    return g_settings.tone == "ink" ? "ink" : g_settings.tone == "alt" ? "alt" : "sage";
}

// Tones by role, from the colour scheme (Tourne's are the original tones).
uint32_t ToneColor(std::string_view tone) {
    auto const& p = g_palette;
    if (tone == "ink") return p.text;
    if (tone == "green") return p.accent;
    if (tone == "red") return p.bad;
    if (tone == "coral") return p.warn;
    if (tone == "alt") return p.dim;
    if (tone == "dim") return p.line;
    return p.muted;  // sage
}

uint32_t ShadeOf(uint32_t color) {
    return PaletteShade(g_palette, color);
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

std::vector<std::pair<std::wstring, int>> ExtraDeviceBatteries();

IconState ComputeDeviceBatteries() {
    auto devices = BluetoothBatteries();
    // Headsets, mice and controllers read directly, when Device batteries
    // shows them in the flyouts.
    for (auto const& [name, percent] : ExtraDeviceBatteries()) {
        devices.push_back({name, percent});
    }
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

////////////////////////////////////////////////////////////////////////////////
// Bluetooth: paired devices from their device nodes, the radio from
// Windows.Devices.Radios, and audio devices connected or disconnected with
// the Bluetooth audio driver's own requests (KSPROPSETID_BtAudio), which
// reconnect a paired speaker or headset the way Settings does.

struct BluetoothDevice {
    std::wstring name;
    GUID container{};
    bool connected = false;
    int battery = -1;
    // The Bluetooth audio drivers behind its speakers and microphones, by
    // their device topology ids. Empty for a device that isn't audio.
    std::vector<std::wstring> audioFilters;
};

constexpr DEVPROPKEY kDevpkeyBluetoothConnected = {
    {0x83DA6326, 0x97A6, 0x4088, {0x94, 0x53, 0xA1, 0x92, 0x3F, 0x57, 0x3B, 0x29}}, 15};
constexpr DEVPROPKEY kDevpkeyBluetoothBattery = {
    {0x104EA319, 0x6EE2, 0x4701, {0xBD, 0x47, 0x8D, 0xDB, 0xF4, 0x25, 0xBB, 0xE5}}, 2};
constexpr DEVPROPKEY kDevpkeyName = {
    {0xB725F130, 0x47EF, 0x101A, {0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC}}, 10};
constexpr DEVPROPKEY kDevpkeyContainerId = {
    {0x8C7ED206, 0x3F8A, 0x4827, {0xB3, 0xAB, 0xAE, 0x9E, 0x1F, 0xAE, 0xFC, 0x6C}}, 2};
constexpr PROPERTYKEY kPkeyDeviceContainerId = {
    {0x8C7ED206, 0x3F8A, 0x4827, {0xB3, 0xAB, 0xAE, 0x9E, 0x1F, 0xAE, 0xFC, 0x6C}}, 2};

// Paired devices, connected ones first. A device paired over both Bluetooth
// and Bluetooth LE has a node for each, grouped under one container.
std::vector<BluetoothDevice> PairedBluetoothDevices() {
    std::vector<BluetoothDevice> devices;
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
            // The devices themselves ("BTHENUM\Dev_..."), not their services.
            PCWSTR slash = wcschr(id, L'\\');
            DEVINST node;
            if (!slash || _wcsnicmp(slash + 1, L"DEV_", 4) != 0 ||
                CM_Locate_DevNodeW(&node, (DEVINSTID_W)id, CM_LOCATE_DEVNODE_NORMAL) != CR_SUCCESS) {
                continue;
            }
            BluetoothDevice device;
            DEVPROPTYPE type = 0;
            WCHAR name[128] = L"";
            ULONG length = sizeof(name);
            if (CM_Get_DevNode_PropertyW(node, &kDevpkeyName, &type, (PBYTE)name, &length, 0) != CR_SUCCESS) {
                name[0] = L'\0';
            }
            length = sizeof(device.container);
            CM_Get_DevNode_PropertyW(node, &kDevpkeyContainerId, &type, (PBYTE)&device.container, &length, 0);
            DEVPROP_BOOLEAN connected = DEVPROP_FALSE;
            length = sizeof(connected);
            device.connected =
                CM_Get_DevNode_PropertyW(node, &kDevpkeyBluetoothConnected, &type, (PBYTE)&connected, &length, 0) ==
                    CR_SUCCESS &&
                type == DEVPROP_TYPE_BOOLEAN && connected == DEVPROP_TRUE;
            BYTE battery = 0;
            length = sizeof(battery);
            if (CM_Get_DevNode_PropertyW(node, &kDevpkeyBluetoothBattery, &type, &battery, &length, 0) ==
                    CR_SUCCESS &&
                type == DEVPROP_TYPE_BYTE && battery <= 100) {
                device.battery = battery;
            }

            auto same = std::find_if(devices.begin(), devices.end(), [&](BluetoothDevice const& d) {
                return d.container != GUID_NULL && IsEqualGUID(d.container, device.container);
            });
            if (same != devices.end()) {
                same->connected = same->connected || device.connected;
                if (device.battery >= 0) {
                    same->battery = device.battery;
                }
                continue;
            }
            device.name = *name ? name : L"Bluetooth device";
            devices.push_back(std::move(device));
        }
    }
    std::stable_sort(devices.begin(), devices.end(), [](BluetoothDevice const& a, BluetoothDevice const& b) {
        if (a.connected != b.connected) {
            return a.connected;
        }
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
    });
    return devices;
}

// The device topology id of the audio driver an endpoint plays through.
std::wstring AudioFilterOf(IMMDevice* endpoint) {
    std::wstring filter;
    IDeviceTopology* topology = nullptr;
    if (FAILED(endpoint->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, nullptr, (void**)&topology))) {
        return filter;
    }
    IConnector* connector = nullptr;
    IConnector* other = nullptr;
    IPart* part = nullptr;
    IDeviceTopology* adapter = nullptr;
    LPWSTR id = nullptr;
    if (SUCCEEDED(topology->GetConnector(0, &connector)) && SUCCEEDED(connector->GetConnectedTo(&other)) &&
        SUCCEEDED(other->QueryInterface(__uuidof(IPart), (void**)&part)) &&
        SUCCEEDED(part->GetTopologyObjects(&adapter)) && SUCCEEDED(adapter->GetDeviceId(&id))) {
        filter = id;
    }
    CoTaskMemFree(id);
    for (IUnknown* object : {(IUnknown*)adapter, (IUnknown*)part, (IUnknown*)other, (IUnknown*)connector,
                             (IUnknown*)topology}) {
        if (object) {
            object->Release();
        }
    }
    return filter;
}

// Which paired devices are audio devices, and their drivers: Windows files a
// device's speakers and microphones under the device's own container. A
// disconnected speaker's endpoints are still there, unplugged.
void FindBluetoothAudio(IMMDeviceEnumerator* enumerator, std::vector<BluetoothDevice>& devices) {
    IMMDeviceCollection* endpoints = nullptr;
    if (!enumerator || devices.empty() ||
        FAILED(enumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED, &endpoints))) {
        return;
    }
    UINT count = 0;
    endpoints->GetCount(&count);
    for (UINT i = 0; i < count; i++) {
        IMMDevice* endpoint = nullptr;
        IPropertyStore* store = nullptr;
        if (FAILED(endpoints->Item(i, &endpoint)) || FAILED(endpoint->OpenPropertyStore(STGM_READ, &store))) {
            if (endpoint) {
                endpoint->Release();
            }
            continue;
        }
        PROPVARIANT value;
        PropVariantInit(&value);
        BluetoothDevice* owner = nullptr;
        if (SUCCEEDED(store->GetValue(kPkeyDeviceContainerId, &value)) && value.vt == VT_CLSID && value.puuid) {
            for (auto& device : devices) {
                if (IsEqualGUID(device.container, *value.puuid)) {
                    owner = &device;
                }
            }
        }
        PropVariantClear(&value);
        store->Release();
        if (owner) {
            std::wstring filter = AudioFilterOf(endpoint);
            if (!filter.empty() &&
                std::find(owner->audioFilters.begin(), owner->audioFilters.end(), filter) == owner->audioFilters.end()) {
                owner->audioFilters.push_back(filter);
            }
        }
        endpoint->Release();
    }
    endpoints->Release();
}

struct IKsControlLite : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE KsProperty(void* property, ULONG propertyLength, void* data,
                                                 ULONG dataLength, ULONG* returned) = 0;
    virtual HRESULT STDMETHODCALLTYPE KsMethod(void*, ULONG, void*, ULONG, ULONG*) = 0;
    virtual HRESULT STDMETHODCALLTYPE KsEvent(void*, ULONG, void*, ULONG, ULONG*) = 0;
};
constexpr GUID kIidKsControl = {0x28F54685, 0x06FD, 0x11D2, {0xB2, 0x7A, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}};
constexpr GUID kKsPropSetBtAudio = {0x7FA06C40, 0xB8F6, 0x4C7E, {0x85, 0x56, 0xE8, 0xC3, 0x3A, 0x12, 0xE5, 0x4D}};
constexpr ULONG kKsBtAudioReconnect = 0;
constexpr ULONG kKsBtAudioDisconnect = 1;
constexpr ULONG kKsPropertyTypeGet = 1;

// Asks each of a device's Bluetooth audio drivers to connect or disconnect.
// Answers at once; the connection itself follows within a few seconds. Runs
// on a job thread, with its own enumerator.
bool RequestBluetoothAudio(std::vector<std::wstring> const& filters, bool connect) {
    IMMDeviceEnumerator* enumerator = nullptr;
    if (FAILED(CoCreateInstance(kClsidMMDeviceEnumerator, nullptr, CLSCTX_ALL, kIidMMDeviceEnumerator,
                                (void**)&enumerator))) {
        return false;
    }
    bool sent = false;
    for (auto const& filter : filters) {
        IMMDevice* device = nullptr;
        IKsControlLite* control = nullptr;
        if (SUCCEEDED(enumerator->GetDevice(filter.c_str(), &device)) &&
            SUCCEEDED(device->Activate(kIidKsControl, CLSCTX_ALL, nullptr, (void**)&control))) {
            struct {
                GUID set;
                ULONG id;
                ULONG flags;
            } property{kKsPropSetBtAudio, connect ? kKsBtAudioReconnect : kKsBtAudioDisconnect, kKsPropertyTypeGet};
            ULONG returned = 0;
            sent = SUCCEEDED(control->KsProperty(&property, sizeof(property), nullptr, 0, &returned)) || sent;
        }
        if (control) {
            control->Release();
        }
        if (device) {
            device->Release();
        }
    }
    enumerator->Release();
    return sent;
}

// The Bluetooth radio: 1 on, 0 off, -1 not known yet or no radio. Watched on
// a thread of its own, since the radio API is asynchronous.
std::atomic<int> g_bluetoothRadio{-1};
HANDLE g_radioThread;
HANDLE g_radioStop;

DWORD WINAPI RadioWatchProc(LPVOID) {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
    try {
        using namespace winrt::Windows::Devices::Radios;
        Radio bluetooth{nullptr};
        for (auto const& radio : Radio::GetRadiosAsync().get()) {
            if (radio.Kind() == RadioKind::Bluetooth) {
                bluetooth = radio;
                break;
            }
        }
        if (bluetooth) {
            auto update = [](Radio const& radio) {
                g_bluetoothRadio = radio.State() == RadioState::On ? 1 : 0;
                RequestRefresh(1u << kBluetooth);
            };
            update(bluetooth);
            auto token = bluetooth.StateChanged(
                [update](Radio const& sender, winrt::Windows::Foundation::IInspectable const&) { update(sender); });
            WaitForSingleObject(g_radioStop, INFINITE);
            bluetooth.StateChanged(token);
        } else {
            WaitForSingleObject(g_radioStop, INFINITE);
        }
    } catch (...) {
        Wh_Log(L"Bluetooth radio unavailable");
    }
    winrt::uninit_apartment();
    return 0;
}

void StartRadioWatch() {
    g_radioStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_radioThread = CreateThread(nullptr, 0, RadioWatchProc, nullptr, 0, nullptr);
}

void StopRadioWatch() {
    if (g_radioThread) {
        SetEvent(g_radioStop);
        WaitForSingleObject(g_radioThread, 5000);
        CloseHandle(g_radioThread);
        g_radioThread = nullptr;
    }
    if (g_radioStop) {
        CloseHandle(g_radioStop);
        g_radioStop = nullptr;
    }
}

// Turns the radio on or off. Runs on a job thread.
std::wstring SetBluetoothRadio(bool on) {
    try {
        using namespace winrt::Windows::Devices::Radios;
        if (Radio::RequestAccessAsync().get() != RadioAccessStatus::Allowed) {
            return L"denied";
        }
        for (auto const& radio : Radio::GetRadiosAsync().get()) {
            if (radio.Kind() == RadioKind::Bluetooth) {
                auto status = radio.SetStateAsync(on ? RadioState::On : RadioState::Off).get();
                return status == RadioAccessStatus::Allowed ? L"ok" : L"denied";
            }
        }
    } catch (...) {
    }
    return L"failed";
}

// Devices connecting or disconnecting add and remove device interfaces;
// any of them refreshes the icon (refreshes run once per burst).
HCMNOTIFICATION g_deviceNotify;

void WakeBatteries();

DWORD CALLBACK OnDeviceInterfaceChange(HCMNOTIFICATION, PVOID, CM_NOTIFY_ACTION action, PCM_NOTIFY_EVENT_DATA,
                                       DWORD) {
    if (action == CM_NOTIFY_ACTION_DEVICEINTERFACEARRIVAL || action == CM_NOTIFY_ACTION_DEVICEINTERFACEREMOVAL) {
        RequestRefresh(1u << kBluetooth);
        // A receiver or cable plugged in or out: device batteries poll now.
        WakeBatteries();
    }
    return ERROR_SUCCESS;
}

void StartDeviceWatch() {
    CM_NOTIFY_FILTER filter{};
    filter.cbSize = sizeof(filter);
    filter.Flags = CM_NOTIFY_FILTER_FLAG_ALL_INTERFACE_CLASSES;
    filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
    if (CM_Register_Notification(&filter, nullptr, OnDeviceInterfaceChange, &g_deviceNotify) != CR_SUCCESS) {
        g_deviceNotify = nullptr;
    }
}

void StopDeviceWatch() {
    if (g_deviceNotify) {
        CM_Unregister_Notification(g_deviceNotify);
        g_deviceNotify = nullptr;
    }
}

IconState ComputeBluetooth() {
    if (g_bluetoothRadio == 0) {
        return {"bluetooth-off", L"Bluetooth ... OFF"};
    }
    std::wstring names;
    for (auto const& device : PairedBluetoothDevices()) {
        if (device.connected) {
            names += (names.empty() ? L"" : L", ") + device.name;
        }
    }
    return names.empty() ? IconState{"bluetooth", L"Bluetooth ... ON"}
                         : IconState{"bluetooth-connected", L"Bluetooth ... " + names};
}

// Windows Security, from the Security Center's own health summary.
WSC_SECURITY_PROVIDER_HEALTH ProviderHealth(DWORD provider) {
    WSC_SECURITY_PROVIDER_HEALTH health = WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED;
    if (FAILED(WscGetSecurityProviderHealth(provider, &health))) {
        return WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED;
    }
    return health;
}

// A Microsoft Defender scan started from the flyout (see StartDefenderScan). Touched on this part's
// own thread only.
struct DefenderScan {
    int running = 0;          // 1 a quick scan, 2 a full one, 0 none
    int lastType = 1;
    ULONGLONG started = 0;
    std::wstring result;      // how the last one ended, for the flyout
};
DefenderScan g_scan;

IconState ComputeSecurity() {
    if (g_scan.running) {
        return {"security-scan", g_scan.running == 2 ? L"Windows Security ... Full scan running"
                                                     : L"Windows Security ... Quick scan running"};
    }
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
        case kBluetooth: return ComputeBluetooth();
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
    uint64_t palette = 0;
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

// Set when an icon was added or changed, so the suite's taskbar can be told.
bool g_iconsChanged = false;

// The suite's own taskbar reads the tray on a timer; this makes it read it
// now, so the volume and brightness it shows keep up with the wheel.
void TellTaskbars() {
    static UINT msg = RegisterWindowMessageW(L"TourneSuite_TrayChanged");
    EnumWindows(
        [](HWND hwnd, LPARAM lParam) __stdcall -> BOOL {
            WCHAR cls[64];
            if (GetClassNameW(hwnd, cls, ARRAYSIZE(cls)) && !wcsncmp(cls, L"TourneTaskbar", 13)) {
                PostMessageW(hwnd, (UINT)lParam, 0, 0);
            }
            return TRUE;
        },
        msg);
}

void ApplyIcon(int kind, int size) {
    IconState const& state = g_states[kind];
    AppliedIcon& applied = g_applied[kind];
    if (applied.added && applied.state.art == state.art &&
        applied.state.tip == state.tip &&
        applied.state.hidden == state.hidden && applied.size == size &&
        applied.tone == g_settings.tone && applied.shade == g_settings.shade && applied.palette == PaletteKey()) {
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
        applied = {true, state, size, g_settings.tone, g_settings.shade, PaletteKey()};
        g_iconsChanged = true;
    } else {
        // Most likely the tray isn't up yet; TaskbarCreated and the periodic
        // timer retry.
        applied.added = false;
    }
}

void ReloadFlyout();
bool IsFlyoutOpen(int flyoutKind);
void ApplyBatteryIcons();

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
    // Device batteries follow the tray size and theme, and retry like the rest.
    ApplyBatteryIcons();
    if (g_iconsChanged) {
        g_iconsChanged = false;
        TellTaskbars();
    }
    if (bits & (kNetworkBits | (1u << kBattery) | (1u << kSecurity) | (1u << kVolume) |
                (1u << kNotifications) | (1u << kBluetooth))) {
        ReloadFlyout();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Device batteries: read on a thread of their own, from device-batteries.cpp
// (placed here by the build), and shown as a tray icon per device and in the
// flyouts, each switchable.

// @inline device-batteries.cpp

constexpr UINT WM_APP_BATTERIES = WM_APP + 13;
constexpr UINT_PTR kTimerBreath = 5;
// Tray ids kIconIdBase + kBatteryIdBase + slot; a device keeps its slot, so
// Windows remembers where it was placed.
constexpr UINT kBatteryIdBase = 0x40;
constexpr int kBatterySlots = 32;

struct BatterySettings {
    bool trayIcons = false;
    bool inFlyouts = false;
    bool pictogram = true;
    bool animation = true;
    int pollSeconds = 60;
    int lowAlert = 20;
    std::array<bool, bat::kVendorCount> vendors{};

    bool operator==(BatterySettings const&) const = default;
};

constexpr int kPollMinSeconds = 5;
constexpr int kPollMaxSeconds = 24 * 60 * 60;

// "9hr", "9min", "30sec", "1.5hr" or a bare number (minutes) to seconds,
// clamped to 5 s .. 24 hr; 0 when it doesn't parse.
int ParsePollInterval(std::wstring text) {
    std::transform(text.begin(), text.end(), text.begin(), towlower);
    std::erase_if(text, iswspace);
    wchar_t* end = nullptr;
    double value = wcstod(text.c_str(), &end);
    if (end == text.c_str() || !(value > 0)) {
        return 0;
    }
    std::wstring unit = end;
    double seconds;
    if (unit.empty() || unit == L"m" || unit == L"min" || unit == L"mins" || unit == L"minute" ||
        unit == L"minutes") {
        seconds = value * 60;
    } else if (unit == L"h" || unit == L"hr" || unit == L"hrs" || unit == L"hour" || unit == L"hours") {
        seconds = value * 3600;
    } else if (unit == L"s" || unit == L"sec" || unit == L"secs" || unit == L"second" || unit == L"seconds") {
        seconds = value;
    } else {
        return 0;
    }
    return (int)std::clamp(std::lround(seconds), (long)kPollMinSeconds, (long)kPollMaxSeconds);
}

BatterySettings LoadBatterySettings() {
    BatterySettings s;
    s.trayIcons = Wh_GetIntSetting(L"batteries.trayIcons");
    s.inFlyouts = Wh_GetIntSetting(L"batteries.inFlyouts");
    s.pictogram = Wh_GetIntSetting(L"batteries.pictogram");
    s.animation = Wh_GetIntSetting(L"batteries.chargingAnimation");
    s.pollSeconds = ParsePollInterval(WindhawkUtils::StringSetting::make(L"batteries.pollInterval").get());
    if (!s.pollSeconds) {
        // Unset or unreadable: the old seconds setting, if there is one, or a minute.
        int old = Wh_GetIntSetting(L"batteries.pollSeconds");
        s.pollSeconds = old > 0 ? std::clamp(old, kPollMinSeconds, kPollMaxSeconds) : 60;
    }
    s.lowAlert = std::clamp(Wh_GetIntSetting(L"batteries.lowAlert"), 0, 50);
    for (int v = 0; v < bat::kVendorCount; v++) {
        s.vendors[v] = Wh_GetIntSetting(L"batteries.vendors.%s", bat::kVendorKeys[v]);
    }
    return s;
}

BatterySettings g_batterySettings;          // Mod thread.
std::vector<bat::Device> g_batteries;        // Mod thread: what's shown.

std::mutex g_batteryLock;                    // Guards the two below.
BatterySettings g_batteryOptions;            // What the poll thread reads.
std::vector<bat::Device> g_batteryResults;   // What it last found.

HANDLE g_batteryThread;
HANDLE g_batteryWake;
std::atomic<bool> g_batteryStop;

void WakeBatteries() {
    if (g_batteryWake) {
        SetEvent(g_batteryWake);
    }
}

DWORD WINAPI BatteryThread(LPVOID) {
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    } catch (...) {
    }
    bat::Poller poller;
    while (!g_batteryStop) {
        BatterySettings options;
        {
            std::lock_guard<std::mutex> guard(g_batteryLock);
            options = g_batteryOptions;
        }
        poller.vendors = options.vendors;
        std::wstring signature = poller.Signature();
        bool any = std::any_of(options.vendors.begin(), options.vendors.end(), [](bool on) { return on; });
        std::vector<bat::Device> devices;
        if (any) {
            devices = poller.PollOnce();
        }
        {
            std::lock_guard<std::mutex> guard(g_batteryLock);
            g_batteryResults = devices;
        }
        if (HWND wnd = g_wnd) {
            PostMessageW(wnd, WM_APP_BATTERIES, 0, 0);
        }

        // The next poll: the interval from the settings, or 3 s while a device
        // has only just appeared or gone missing. A device plugged in or
        // removed, or a wake from the settings, polls at once.
        ULONGLONG interval = (ULONGLONG)options.pollSeconds * 1000;
        if (poller.Pending()) {
            interval = std::min<ULONGLONG>(interval, 3000);
        }
        ULONGLONG deadline = GetTickCount64() + interval;
        while (!g_batteryStop) {
            ULONGLONG now = GetTickCount64();
            if (now >= deadline) {
                break;
            }
            DWORD wait = (DWORD)std::min<ULONGLONG>(2500, deadline - now);
            if (WaitForSingleObject(g_batteryWake, wait) == WAIT_OBJECT_0) {
                Sleep(1000);  // let Windows finish setting the device up
                break;
            }
            if (any && poller.Signature() != signature) {
                Sleep(1000);
                break;
            }
        }
    }
    try {
        winrt::uninit_apartment();
    } catch (...) {
    }
    return 0;
}

void StartBatteries() {
    g_batterySettings = LoadBatterySettings();
    {
        std::lock_guard<std::mutex> guard(g_batteryLock);
        g_batteryOptions = g_batterySettings;
    }
    g_batteryStop = false;
    g_batteryWake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_batteryThread = CreateThread(nullptr, 0, BatteryThread, nullptr, 0, nullptr);
}

void StopBatteries() {
    g_batteryStop = true;
    WakeBatteries();
    if (g_batteryThread) {
        // A poll in progress finishes its current device first.
        WaitForSingleObject(g_batteryThread, 15000);
        CloseHandle(g_batteryThread);
        g_batteryThread = nullptr;
    }
    if (g_batteryWake) {
        CloseHandle(g_batteryWake);
        g_batteryWake = nullptr;
    }
}

// Tray icons

struct BatteryIcon {
    std::wstring key;
    bool added = false;
    std::string art;
    std::wstring tip;
    int size = 0;
    std::string tone;
    bool shade = true;
    uint64_t palette = 0;
};

std::array<BatteryIcon, kBatterySlots> g_batteryIcons;
std::map<std::wstring, int> g_batterySlotOf;
std::map<std::wstring, bool> g_batteryAlerted;
int g_breathFrame;

void LoadBatterySlots() {
    WCHAR text[4096] = L"";
    Wh_GetStringValue(L"systemIcons.batterySlots", text, ARRAYSIZE(text));
    g_batterySlotOf.clear();
    std::wstring_view rest = text;
    while (!rest.empty()) {
        size_t end = rest.find(L';');
        std::wstring_view item = rest.substr(0, end);
        size_t eq = item.rfind(L'=');
        if (eq != std::wstring_view::npos) {
            int slot = _wtoi(std::wstring(item.substr(eq + 1)).c_str());
            if (slot >= 0 && slot < kBatterySlots) {
                g_batterySlotOf[std::wstring(item.substr(0, eq))] = slot;
            }
        }
        rest = end == std::wstring_view::npos ? std::wstring_view() : rest.substr(end + 1);
    }
}

void SaveBatterySlots() {
    std::wstring text;
    for (auto const& [key, slot] : g_batterySlotOf) {
        text += key + L"=" + std::to_wstring(slot) + L";";
    }
    Wh_SetStringValue(L"systemIcons.batterySlots", text.c_str());
}

int BatterySlotFor(std::wstring const& key) {
    auto it = g_batterySlotOf.find(key);
    if (it != g_batterySlotOf.end()) {
        return it->second;
    }
    std::array<bool, kBatterySlots> taken{};
    for (auto const& [k, slot] : g_batterySlotOf) {
        taken[slot] = true;
    }
    int slot = -1;
    for (int i = 0; i < kBatterySlots && slot < 0; i++) {
        if (!taken[i]) {
            slot = i;
        }
    }
    if (slot < 0) {
        // All remembered: take the slot of a device that isn't here now.
        for (auto it2 = g_batterySlotOf.begin(); it2 != g_batterySlotOf.end(); ++it2) {
            bool present = std::any_of(g_batteries.begin(), g_batteries.end(),
                                       [&](bat::Device const& d) { return d.key == it2->first; });
            if (!present) {
                slot = it2->second;
                g_batterySlotOf.erase(it2);
                break;
            }
        }
    }
    if (slot >= 0) {
        g_batterySlotOf[key] = slot;
        SaveBatterySlots();
    }
    return slot;
}

std::string BatteryKind(bat::Device const& d) {
    if (!d.kind.empty()) {
        return d.kind;
    }
    return d.source == "bluetooth" ? "bluetooth" : "mouse";
}

std::string BatteryArtId(bat::Device const& d) {
    int low = g_batterySettings.lowAlert;
    std::string state = !d.online                        ? "asleep"
                        : d.level < 0                    ? "unknown"
                        : d.charging                     ? (g_batterySettings.animation && g_breathFrame ? "charge2"
                                                                                                           : "charge")
                        : low > 0 && d.level <= low      ? "low"
                        : low > 0 && d.level <= low + 10 ? "warn"
                                                         : "normal";
    return "devbat:" + BatteryKind(d) + ":" + std::to_string(d.level) + ":" + state +
           (g_batterySettings.pictogram ? "" : ":nopict");
}

std::wstring BatteryDetail(bat::Device const& d) {
    std::wstring text = !d.approx.empty() ? d.approx
                        : d.level >= 0     ? std::to_wstring(d.level) + L"%"
                                           : L"no reading";
    if (d.charging && d.approx.empty()) {
        text += L", charging";
    }
    if (!d.online) {
        text += L", asleep";
    }
    return text;
}

void FillBatteryIconData(NOTIFYICONDATAW* data, int slot) {
    *data = {};
    data->cbSize = sizeof(*data);
    data->hWnd = g_wnd;
    data->uID = kIconIdBase + kBatteryIdBase + slot;
}

void RemoveBatteryIcon(int slot) {
    if (!g_batteryIcons[slot].added) {
        g_batteryIcons[slot] = {};
        return;
    }
    NOTIFYICONDATAW data;
    FillBatteryIconData(&data, slot);
    Shell_NotifyIconW(NIM_DELETE, &data);
    g_batteryIcons[slot] = {};
}

void RemoveBatteryIcons() {
    for (int slot = 0; slot < kBatterySlots; slot++) {
        RemoveBatteryIcon(slot);
    }
}

void ApplyBatteryIcons() {
    if (!g_batterySettings.trayIcons) {
        RemoveBatteryIcons();
        return;
    }
    int size = TrayIconSize();
    std::array<bool, kBatterySlots> used{};
    for (auto const& d : g_batteries) {
        int slot = BatterySlotFor(d.key);
        if (slot < 0) {
            continue;
        }
        used[slot] = true;
        BatteryIcon& icon = g_batteryIcons[slot];
        std::string art = BatteryArtId(d);
        std::wstring tip = d.name + L" ... " + BatteryDetail(d);
        if (icon.added && icon.key == d.key && icon.art == art && icon.tip == tip && icon.size == size &&
            icon.tone == g_settings.tone && icon.shade == g_settings.shade && icon.palette == PaletteKey()) {
            continue;
        }
        HICON hicon = CreateIconFromPixels(RenderArt(ArtFor(art), g_settings.tone, g_settings.shade, size), size);
        NOTIFYICONDATAW data;
        FillBatteryIconData(&data, slot);
        data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
        data.uCallbackMessage = WM_APP_TRAY;
        data.hIcon = hicon;
        wcsncpy_s(data.szTip, tip.c_str(), _TRUNCATE);
        bool ok;
        if (!icon.added || icon.key != d.key) {
            if (icon.added) {
                Shell_NotifyIconW(NIM_DELETE, &data);
            }
            ok = Shell_NotifyIconW(NIM_ADD, &data);
            if (ok) {
                data.uVersion = NOTIFYICON_VERSION_4;
                Shell_NotifyIconW(NIM_SETVERSION, &data);
            }
        } else {
            ok = Shell_NotifyIconW(NIM_MODIFY, &data);
        }
        if (hicon) {
            DestroyIcon(hicon);
        }
        if (ok) {
            icon = {d.key, true, art, tip, size, g_settings.tone, g_settings.shade, PaletteKey()};
        } else {
            icon.added = false;
        }
    }
    for (int slot = 0; slot < kBatterySlots; slot++) {
        if (!used[slot]) {
            RemoveBatteryIcon(slot);
        }
    }
}

// Once per discharge: again only after the device was charged.
void CheckBatteryAlerts() {
    int low = g_batterySettings.lowAlert;
    for (auto const& d : g_batteries) {
        if (!low || d.level < 0 || !d.online) {
            continue;
        }
        if (d.charging || d.level > low + 5) {
            g_batteryAlerted[d.key] = false;
            continue;
        }
        if (d.level > low || g_batteryAlerted[d.key]) {
            continue;
        }
        g_batteryAlerted[d.key] = true;
        auto slot = g_batterySlotOf.find(d.key);
        if (!g_batterySettings.trayIcons || slot == g_batterySlotOf.end() || !g_batteryIcons[slot->second].added) {
            continue;
        }
        NOTIFYICONDATAW data;
        FillBatteryIconData(&data, slot->second);
        data.uFlags = NIF_INFO;
        data.dwInfoFlags = NIIF_WARNING;
        wcsncpy_s(data.szInfoTitle, L"Low battery", _TRUNCATE);
        std::wstring text = d.name + L": " + (d.approx.empty() ? std::to_wstring(d.level) + L"% left" : L"battery is low") +
                            L". Time to charge.";
        wcsncpy_s(data.szInfo, text.c_str(), _TRUNCATE);
        Shell_NotifyIconW(NIM_MODIFY, &data);
    }
}

void UpdateBreathing() {
    bool charging = std::any_of(g_batteries.begin(), g_batteries.end(),
                                [](bat::Device const& d) { return d.charging && d.online && d.level >= 0; });
    if (charging && g_batterySettings.animation && g_batterySettings.trayIcons) {
        SetTimer(g_wnd, kTimerBreath, 700, nullptr);
    } else {
        KillTimer(g_wnd, kTimerBreath);
        g_breathFrame = 0;
    }
}

bool IsFlyoutOpen(int flyoutKind);

void OnBatteriesFound() {
    {
        std::lock_guard<std::mutex> guard(g_batteryLock);
        g_batteries = g_batteryResults;
    }
    ApplyBatteryIcons();
    CheckBatteryAlerts();
    UpdateBreathing();
    // The battery icon's list of devices on a desktop, and open flyouts.
    RequestRefresh(1u << kBattery);
}

void OnBatterySettings() {
    BatterySettings next = LoadBatterySettings();
    if (next == g_batterySettings) {
        return;
    }
    bool vendorsChanged = next.vendors != g_batterySettings.vendors || next.pollSeconds != g_batterySettings.pollSeconds;
    g_batterySettings = next;
    {
        std::lock_guard<std::mutex> guard(g_batteryLock);
        g_batteryOptions = next;
    }
    if (vendorsChanged) {
        WakeBatteries();
    }
    ApplyBatteryIcons();
    UpdateBreathing();
}

// Devices with a reading, for the battery icon's tooltip on a desktop.
std::vector<std::pair<std::wstring, int>> ExtraDeviceBatteries() {
    std::vector<std::pair<std::wstring, int>> list;
    if (!g_batterySettings.inFlyouts) {
        return list;
    }
    for (auto const& d : g_batteries) {
        if (d.level >= 0 && d.source != "bluetooth") {
            list.push_back({d.name, d.level});
        }
    }
    return list;
}

////////////////////////////////////////////////////////////////////////////////
// Talking to the Explorer relay
//
// This copy runs in its own process, so it can't reach into the tray's
// toolbars. The suite's Explorer relay does that part: it replays right
// clicks on the system icons (which live in Explorer), and sends scroll and
// middle clicks over the volume icon here. The message names are shared.

UINT g_msgRelayForward;  // To the relay: wParam = MAKEWPARAM(kind, event), lParam = anchor.
UINT g_msgRelayWheel;    // From the relay: wParam = wheel delta.
UINT g_msgRelayMiddle;   // From the relay.
UINT g_msgRelayOverflow; // To the relay: wParam = this window; answered with WM_COPYDATA.

void RegisterRelayMessages() {
    g_msgRelayForward = RegisterWindowMessageW(L"TourneSuite_RelayForward");
    g_msgRelayWheel = RegisterWindowMessageW(L"TourneSuite_RelayWheel");
    g_msgRelayMiddle = RegisterWindowMessageW(L"TourneSuite_RelayMiddle");
    g_msgRelayOverflow = RegisterWindowMessageW(L"TourneSuite_RelayOverflow");
}

// @inline tray-shared.cpp
// @inline plugin-bar-shared.cpp
// @inline shell-core.cpp
// @inline menu-style.cpp
// @inline motion.cpp
// @inline phone-link.cpp

// Asks the relay to replay a tray event on the system icon this stand-in
// replaces. False when there's no relay or no such icon, so the caller can
// fall back to its own menu.
bool RelayForward(int kind, UINT event, WPARAM anchor) {
    HWND toolbar = RelayToolbar();
    DWORD_PTR result = 0;
    return toolbar && g_msgRelayForward &&
           SendMessageTimeoutW(toolbar, g_msgRelayForward, MAKEWPARAM(kind, event),
                               (LPARAM)anchor, SMTO_ABORTIFHUNG | SMTO_BLOCK, 500,
                               &result) &&
           result == 1;
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

HFONT MakeFont(std::initializer_list<PCWSTR> faces, int pixels);

struct MenuItem {
    std::wstring label;  // empty: a separator
    std::function<void()> action;
    std::vector<MenuItem> submenu;
    bool checked = false;
};

// Items numbered in order, submenus included, so a chosen id finds its item.
HMENU BuildMenu(std::vector<MenuItem> const& items, std::vector<MenuItem const*>* ids) {
    HMENU menu = CreatePopupMenu();
    for (auto const& item : items) {
        if (item.label.empty()) {
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        } else if (!item.submenu.empty()) {
            AppendMenuW(menu, MF_POPUP, (UINT_PTR)BuildMenu(item.submenu, ids), item.label.c_str());
        } else {
            ids->push_back(&item);
            AppendMenuW(menu, MF_STRING | (item.checked ? MF_CHECKED : 0), ids->size(), item.label.c_str());
        }
    }
    return menu;
}

bool TakeMenuAnchor(RECT* icon);
bool FlyoutUnder(POINT pt, RECT* rc);

// A click anywhere but the menu closes it (and goes no further, as Windows' own menus take it). Windows
// does this itself only when the menu's window is in front, which a click on the Tourne taskbar (which
// never takes the focus) doesn't always leave it; so a mouse hook watches, for as long as the menu is up. The hook
// runs on a thread of its own: on the menu's, every mouse move anywhere waited for the menu's painting (the pointer
// hitched over items). It closes the menu by telling its owner to cancel (EndMenu works only on the menu's thread).
bool g_menuEatUp = false;
HWND g_menuOwner = nullptr;

LRESULT CALLBACK MenuMouseProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        bool down = wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN;
        bool up = wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP;
        if (up && g_menuEatUp) {
            g_menuEatUp = false;
            return 1;
        }
        if (down) {
            HWND at = WindowFromPoint(((MSLLHOOKSTRUCT*)lParam)->pt);
            WCHAR name[16];
            if (!at || !GetClassNameW(at, name, ARRAYSIZE(name)) || wcscmp(name, L"#32768") != 0) {
                PostMessageW(g_menuOwner, WM_CANCELMODE, 0, 0);
                g_menuEatUp = true;
                return 1;
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

// The taskbar's menu animation (its settings, read by their full names through variables, so the build
// doesn't file them under this part) and the motion engine's popups line.
motion::Anim g_popupsMotion;
double PopupsMotionEase(double t) { return motion::Ease(g_popupsMotion, t); }

void SetMenuMotionLikeTaskbar() {
    constexpr PCWSTR kMenuAnimation = L"taskbar.menuAnimation";
    PCWSTR value = Wh_GetStringSetting(kMenuAnimation);
    g_menuMotion = {};
    g_menuMotion.style = MenuMotionStyle(value ? value : L"");
    Wh_FreeStringSetting(value);
    g_popupsMotion = motion::Get(L"popups");
    if (g_popupsMotion.set)
        g_menuMotion.style = g_popupsMotion.on ? MenuMotionStyleWord(motion::StyleWord(g_popupsMotion), g_menuMotion.style) : 0;
    if (g_popupsMotion.set && g_popupsMotion.on) {
        g_menuMotion.ms = std::clamp(motion::Ms(g_popupsMotion), 0.0, 1500.0);
        g_menuMotion.ease = PopupsMotionEase;
    }
}

void ShowMenu(std::vector<MenuItem> const& items, WPARAM anchor) {
    std::vector<MenuItem const*> ids;
    HMENU menu = BuildMenu(items, &ids);

    POINT pt;
    GetCursorPos(&pt);
    if (anchor) {
        pt = {GET_X_LPARAM(anchor), GET_Y_LPARAM(anchor)};
    }
    RECT icon{pt.x, pt.y, pt.x, pt.y};
    bool haveIcon = TakeMenuAnchor(&icon);

    // Required for the menu to close when clicking elsewhere.
    HWND wnd = g_wnd;
    SetForegroundWindow(wnd);
    // In the colour scheme and the Tourne font, like the flyouts.
    int scale = std::max(1, (int)(GetDpiForWindow(wnd) + USER_DEFAULT_SCREEN_DPI / 2) / USER_DEFAULT_SCREEN_DPI);
    static std::map<int, HFONT> fonts;
    HFONT& font = fonts[scale];
    if (!font) {
        font = MakeFont({L"CozetteVector", L"scientifica"}, 13 * scale);
    }
    auto const& p = g_palette;
    constexpr PCWSTR kMenuNoIcons = L"menuNoIcons.enabled";  // (through a variable: another part's setting)
    g_menuLook = {p.ground, p.line, p.edge, p.text, p.accent, p.dim, p.hover, font, nullptr, scale,
                  Wh_GetIntSetting(kMenuNoIcons) != 0};
    // Framed as the taskbar's are with a StartAllBack style, so every menu from the bar looks the same.
    if (TaskbarFramesMenus()) {
        static std::map<int, HFONT> titleFonts;
        HFONT& title = titleFonts[scale];
        LOGFONTW lf;
        if (!title && SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, 96 * scale)) {
            title = CreateFontIndirectW(&lf);
        }
        FrameLikeStartAllBack(g_menuLook, title, StartMenuCapitals());
    }
    // It opens as the taskbar's menus do (taskbar.menuAnimation, on the motion engine's popups line).
    SetMenuMotionLikeTaskbar();
    // Above the icon on a bottom taskbar, below it on a top one.
    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &monitor);
    UINT place = pt.y < (monitor.rcMonitor.top + monitor.rcMonitor.bottom) / 2 ? TPM_TOPALIGN : TPM_BOTTOMALIGN;
    // What it mustn't cover, and whose screen it stays on: the flyout it was opened from (an icon in the
    // hidden icons flyout), or the taskbar when set to open clear of it.
    RECT from{};
    bool haveFrom = false;
    RECT flyout;
    if (FlyoutUnder(pt, &flyout)) {
        // Beside the flyout, on its side toward the screen's middle first; the other side if there's no room.
        bool right = (flyout.left + flyout.right) / 2 < (monitor.rcMonitor.left + monitor.rcMonitor.right) / 2;
        pt.x = right ? flyout.right : flyout.left;
        place = (right ? TPM_LEFTALIGN : TPM_RIGHTALIGN) | (place & TPM_BOTTOMALIGN);
        from = flyout;
        haveFrom = true;
    } else if (g_settings.menuByTaskbar) {
        // Clear of the taskbar the icon is on, lined up with the icon as set.
        POINT middle{(icon.left + icon.right) / 2, (icon.top + icon.bottom) / 2};
        RECT bar = icon;
        HWND under = haveIcon ? WindowFromPoint(middle) : WindowFromPoint(pt);
        if (HWND root = under ? GetAncestor(under, GA_ROOT) : nullptr) GetWindowRect(root, &bar);
        int away = g_settings.menuAway * scale, along = g_settings.menuAlong * scale;
        int align = g_settings.menuAlign;
        bool across = bar.right - bar.left >= bar.bottom - bar.top;  // a taskbar along the top or bottom
        RECT const& m = monitor.rcMonitor;
        if (across) {
            bool top = bar.top + bar.bottom < m.top + m.bottom;
            pt.x = (align > 0 ? icon.left : align < 0 ? icon.right : middle.x) + along;
            pt.y = top ? bar.bottom + away : bar.top - away;
            place = (top ? TPM_TOPALIGN : TPM_BOTTOMALIGN) |
                    (align > 0 ? TPM_LEFTALIGN : align < 0 ? TPM_RIGHTALIGN : TPM_CENTERALIGN);
        } else {
            bool left = bar.left + bar.right < m.left + m.right;
            pt.x = left ? bar.right + away : bar.left - away;
            pt.y = (align > 0 ? icon.top : align < 0 ? icon.bottom : middle.y) + along;
            place = (left ? TPM_LEFTALIGN : TPM_RIGHTALIGN) |
                    (align > 0 ? TPM_TOPALIGN : align < 0 ? TPM_BOTTOMALIGN : TPM_VCENTERALIGN);
        }
        if (across) place |= TPM_VERTICAL;  // (kept off the taskbar first, then fitted along it)
        from = bar;
        haveFrom = true;
    }
    // (At the pointer, it stays on the pointer's screen.)
    g_menuEatUp = false;
    g_menuOwner = wnd;
    HANDLE hookStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    HANDLE hookThread = hookStop ? CreateThread(nullptr, 0, [](void* stop) -> DWORD {
        HHOOK mouse = SetWindowsHookExW(WH_MOUSE_LL, MenuMouseProc, GetModuleHandleW(nullptr), 0);
        while (MsgWaitForMultipleObjects(1, (HANDLE*)&stop, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0) {
            MSG m;
            while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
        }
        if (mouse) UnhookWindowsHookEx(mouse);
        return 0;
    }, hookStop, 0, nullptr) : nullptr;
    UINT chosen = TrackStyledMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | place, pt.x, pt.y, wnd, false,
                                  haveFrom ? &from : nullptr);
    if (hookStop) SetEvent(hookStop);
    if (hookThread) WaitForSingleObject(hookThread, 500), CloseHandle(hookThread);
    if (hookStop) CloseHandle(hookStop);
    PostMessageW(wnd, WM_NULL, 0, 0);
    DestroyMenu(menu);

    if (chosen && chosen <= ids.size() && ids[chosen - 1]->action) {
        ids[chosen - 1]->action();
    }
}

std::vector<MenuItem> PowerMenu() {
    return {
        {L"Lock", [] { LockWorkStation(); }},
        {L"Sign out", [] { RequestExitWindows(EWX_LOGOFF); }},
        {},
        {L"Sleep", [] { SetSuspendState(FALSE, FALSE, FALSE); }},
        {L"Restart", [] { RequestExitWindows(EWX_REBOOT); }},
        {L"Shut down", [] { RequestExitWindows(EWX_SHUTDOWN | EWX_POWEROFF); }},
    };
}

auto Uri(PCWSTR uri) {
    return [uri] { OpenUri(uri); };
}

// Spatial sound for the default output, set the way Windows' own volume menu
// (SndVolSSO) sets it: through the policy config's spatial interface, which
// Windows' public API doesn't offer ("Off" isn't a format it can set). Read
// from SndVolSSO's public symbols; the layouts below are what it uses.
//   method 4   GetDeviceFormat(device, 0, &format)
//   method 34  GetSpatialSettings(device, 0, &format, &settings, &count,
//                                 &formats)
//   method 35  SetSpatialSettings(device, settings, format or null)
// Settings are 72 bytes: +0 on, +4 and +8 flags, +0x0C and +0x1C the format's
// GUID twice, +0x3C a flag, +0x44 the format's second flag. Each format
// Windows offers is 0x342 bytes: its name at +0, GUID at +0x300, licensed at
// +0x310, and the two flags copied into the settings at +0x314 and +0x318.
constexpr GUID kIidPolicyConfigSpatial = {
    0xe8478600, 0xa74b, 0x4b3a, {0xa9, 0x6b, 0x1f, 0xc3, 0xe7, 0x96, 0xfc, 0x46}};
constexpr size_t kSpatialSettingsSize = 0x48;
constexpr size_t kSpatialFormatSize = 0x342;

using SpatialGetDeviceFormat_t = HRESULT(STDMETHODCALLTYPE*)(void*, PCWSTR, int, WAVEFORMATEX**);
using SpatialGet_t = HRESULT(STDMETHODCALLTYPE*)(void*, PCWSTR, int, WAVEFORMATEX**, BYTE**, DWORD*, BYTE**);
using SpatialSet_t = HRESULT(STDMETHODCALLTYPE*)(void*, PCWSTR, BYTE*, WAVEFORMATEX*);

template <typename T>
T SpatialMethod(IUnknown* policy, int slot) {
    return (T)(*(void***)policy)[slot];
}

struct SpatialFormat {
    std::wstring name;
    GUID guid;
    bool licensed;
    DWORD flag1, flag2;
};

struct SpatialState {
    std::wstring device;
    std::array<BYTE, kSpatialSettingsSize> settings{};
    std::vector<SpatialFormat> formats;
    bool on = false;
    GUID current{};
};

// The default output's spatial sound, as Windows' menu reads it. False when
// this Windows has no such interface or the output can't do spatial sound.
bool ReadSpatialState(SpatialState* state) {
    IMMDeviceEnumerator* enumerator = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                                (void**)&enumerator))) {
        return false;
    }
    IMMDevice* device = nullptr;
    PWSTR id = nullptr;
    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device))) {
        device->GetId(&id);
        device->Release();
    }
    enumerator->Release();
    if (!id) {
        return false;
    }
    state->device = id;
    CoTaskMemFree(id);

    IUnknown* policy = nullptr;
    if (FAILED(CoCreateInstance(kClsidPolicyConfigClient, nullptr, CLSCTX_ALL, kIidPolicyConfigSpatial,
                                (void**)&policy))) {
        return false;
    }
    WAVEFORMATEX* format = nullptr;
    BYTE* settings = nullptr;
    DWORD count = 0;
    BYTE* formats = nullptr;
    HRESULT hr = SpatialMethod<SpatialGet_t>(policy, 34)(policy, state->device.c_str(), 0, &format, &settings,
                                                         &count, &formats);
    policy->Release();
    bool ok = SUCCEEDED(hr) && settings;
    if (ok) {
        memcpy(state->settings.data(), settings, kSpatialSettingsSize);
        state->on = *(DWORD*)settings != 0;
        state->current = *(GUID*)(settings + 0x0C);
        for (DWORD i = 0; formats && i < count && i < 16; i++) {
            BYTE* f = formats + i * kSpatialFormatSize;
            state->formats.push_back({std::wstring((PCWSTR)f, wcsnlen((PCWSTR)f, 0x300 / sizeof(WCHAR))),
                                      *(GUID*)(f + 0x300), *(DWORD*)(f + 0x310) != 0, *(DWORD*)(f + 0x314),
                                      *(DWORD*)(f + 0x318)});
        }
    }
    CoTaskMemFree(format);
    CoTaskMemFree(settings);
    CoTaskMemFree(formats);
    return ok && !state->formats.empty();
}

struct SpatialRequest {
    std::wstring device;
    std::array<BYTE, kSpatialSettingsSize> settings;
    bool off;
};

DWORD WINAPI SetSpatialProc(LPVOID param) {
    std::unique_ptr<SpatialRequest> request((SpatialRequest*)param);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    HRESULT hr = E_FAIL;
    IUnknown* policy = nullptr;
    if (SUCCEEDED(CoCreateInstance(kClsidPolicyConfigClient, nullptr, CLSCTX_ALL, kIidPolicyConfigSpatial,
                                   (void**)&policy))) {
        PCWSTR device = request->device.c_str();
        WAVEFORMATEX* format = nullptr;
        if (request->off) {
            // Off is plain stereo, as Windows' menu sets it.
            if (SUCCEEDED(SpatialMethod<SpatialGetDeviceFormat_t>(policy, 4)(policy, device, 0, &format)) && format) {
                format->nChannels = 2;
                if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE && format->cbSize >= 22) {
                    ((WAVEFORMATEXTENSIBLE*)format)->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
                }
            }
        }
        hr = SpatialMethod<SpatialSet_t>(policy, 35)(policy, device, request->settings.data(), format);
        CoTaskMemFree(format);
        policy->Release();
    }
    Wh_Log(L"Spatial sound %s: 0x%08X", request->off ? L"off" : L"format", hr);
    CoUninitialize();
    if (FAILED(hr)) {
        OpenUri(L"ms-settings:sound");
    }
    return 0;
}

// Setting it waits on the audio service, so it runs off the menu's thread.
// Windows restarts what's playing when it changes, as its own switch does.
void SetSpatial(SpatialState const& state, SpatialFormat const* format) {
    auto* request = new SpatialRequest{state.device, state.settings, !format};
    BYTE* s = request->settings.data();
    if (format) {
        *(DWORD*)(s + 0x00) = 1;
        *(DWORD*)(s + 0x04) = format->flag1;
        *(DWORD*)(s + 0x08) = 1;
        *(GUID*)(s + 0x0C) = format->guid;
        *(GUID*)(s + 0x1C) = format->guid;
        *(DWORD*)(s + 0x3C) = 1;
        *(DWORD*)(s + 0x44) = format->flag2 != 0;
    } else {
        *(DWORD*)(s + 0x00) = 0;
        *(GUID*)(s + 0x0C) = GUID{};
        *(DWORD*)(s + 0x44) = 0;
    }
    if (HANDLE thread = CreateThread(nullptr, 0, SetSpatialProc, request, 0, nullptr)) {
        CloseHandle(thread);
    } else {
        delete request;
    }
}

// "Spatial sound (Dolby Atmos for Headphones)" with the formats Windows
// offers for the output that are licensed (their app installed), and the one
// on now; nothing when there's no spatial sound. Reading changes nothing,
// and the entry already on does nothing when picked.
std::vector<MenuItem> SpatialSoundMenu() {
    auto state = std::make_shared<SpatialState>();
    if (!ReadSpatialState(state.get())) {
        return {};
    }
    std::wstring currentName = L"Off";
    std::vector<MenuItem> choices;
    choices.push_back({L"Off", state->on ? std::function<void()>([state] { SetSpatial(*state, nullptr); }) : nullptr,
                       {}, !state->on});
    for (size_t i = 0; i < state->formats.size(); i++) {
        auto const& f = state->formats[i];
        bool on = state->on && IsEqualGUID(f.guid, state->current);
        if (on) {
            currentName = f.name;
        }
        if (on || f.licensed) {
            choices.push_back({f.name, on ? nullptr : std::function<void()>([state, i] {
                                                          SetSpatial(*state, &state->formats[i]);
                                                      }),
                               {}, on});
        }
    }
    choices.push_back({});
    choices.push_back({L"Spatial sound settings", Uri(L"ms-settings:sound"), {}, false});
    return {{L"Spatial sound (" + currentName + L")", nullptr, std::move(choices), false}};
}

////////////////////////////////////////////////////////////////////////////////
// Hidden icons. The tray keeps every icon as a button of its toolbar, the ones
// the user hid included (marked hidden), and only Explorer can read them. The
// functions marked "In Explorer" run there, and pack the hidden icons with
// their pictures and tooltips for the flyout.

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

// A tooltip's lines kept as its program wrote them (a sensor's: name, then a value a line), the
// spaces in each collapsed and the empty lines dropped. The taskbar's tooltip stacks the lines, as
// StartAllBack's does; lists that want one line take the first.
std::wstring TipLines(std::wstring_view text) {
    std::wstring out, line;
    auto flush = [&] {
        while (!line.empty() && line.back() == L' ') line.pop_back();
        if (!line.empty()) {
            if (!out.empty()) out += L'\n';
            out += line;
        }
        line.clear();
    };
    for (WCHAR ch : text) {
        if (ch == L'\n' || ch == L'\r') {
            flush();
        } else if (ch == L' ' || ch == L'\t') {
            if (!line.empty() && line.back() != L' ') line += L' ';
        } else {
            line += ch;
        }
    }
    flush();
    return out;
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
    g_tips[key] = TipLines(tip);
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
// Held to replace g_settings, and by what reads them from another thread: Tourne Desktop's own tray
// (TrayDesignIcon) and its taskbar and switcher (DesignIconAt), which run in the same process there.
SRWLOCK g_settingsLock = SRWLOCK_INIT;
SRWLOCK g_themeLock = SRWLOCK_INIT;
// Kept for Explorer's lifetime: the tray may hold on to the handle it's given.
[[clang::no_destroy]] std::map<std::string, HICON> g_themeIcons;

// A row's tourne:name as one of the designs: its full name (app-screenshot) or without app- (screenshot);
// empty when there's none by that name.
std::string TourneDesignId(std::wstring const& icon) {
    std::string name;
    for (wchar_t c : std::wstring_view(icon).substr(7)) {
        if (c < 0x80 && !iswspace(c)) {
            name += (char)towlower(c);
        }
    }
    if (name.empty() || name.find(':') != std::string::npos) {
        return {};
    }
    Art none{P({})};
    for (std::string id : {name, "app-" + name}) {
        if (ArtFor(id).grid != none.grid) {
            return id;
        }
    }
    return {};
}

// A row's tourne:name drawn at any size, in the colour scheme as the tray draws it, for the taskbar's
// buttons and Alt+Tab (in this same process); the caller destroys it. Null when the name isn't a design.
HICON DesignIconAt(std::wstring const& icon, int size) {
    std::string design = TourneDesignId(icon);
    if (design.empty() || size <= 0) {
        return nullptr;
    }
    AcquireSRWLockShared(&g_settingsLock);
    HICON made = CreateIconFromPixels(RenderArt(ArtFor(design), g_settings.tone, g_settings.shade, size), size);
    ReleaseSRWLockShared(&g_settingsLock);
    return made;
}

HICON ThemeIcon(std::string const& design) {
    int size = TrayIconSize();
    std::string key = design + "|" + std::to_string(size) + "|" + g_settings.tone +
                      (g_settings.shade ? "|shade" : "") + "|" + std::to_string(PaletteKey());
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

// A program's icon from Tourne'Icons' Taskbar buttons and the taskbar's Button
// icons marked "Use everywhere", which wins over the design: the same icon as
// on its buttons. Kept with the designs' icons, for as long.
HICON ProgramTrayIcon(HWND owner) {
    std::vector<ProgramIcon> rows = WithTourneIcons(g_settings.programIcons);
    if (rows.empty() || !IsWindow(owner) || IsStandInWindow(owner)) {
        return nullptr;
    }
    std::wstring path = OwnerPath(owner);
    for (auto const& row : rows) {
        if (!ProgramIconMatches(row.program, L"", path)) {
            continue;
        }
        if (IsTourneDesign(row.icon)) {
            std::string design = TourneDesignId(row.icon);
            if (design.empty()) {
                continue;  // (not one of the designs: the program's own icon, or its built-in design)
            }
            return ThemeIcon(design);
        }
        int size = TrayIconSize();
        int length = WideCharToMultiByte(CP_UTF8, 0, row.icon.c_str(), (int)row.icon.size(), nullptr, 0, nullptr, nullptr);
        std::string key(length, '\0');
        WideCharToMultiByte(CP_UTF8, 0, row.icon.c_str(), (int)row.icon.size(), key.data(), length, nullptr, nullptr);
        key = "file|" + std::to_string(size) + "|" + key;
        AcquireSRWLockExclusive(&g_themeLock);
        HICON& icon = g_themeIcons[key];
        if (!icon) {
            std::wstring file = row.icon;
            int index = 0;
            size_t comma = file.rfind(L',');
            if (comma != std::wstring::npos && comma + 1 < file.size() && iswdigit(file.back())) {
                index = _wtoi(file.c_str() + comma + 1);
                file.resize(comma);
            }
            UINT id = 0;
            if (PrivateExtractIconsW(file.c_str(), index, size, size, &icon, &id, 1, LR_DEFAULTCOLOR) != 1) {
                icon = nullptr;
            }
        }
        HICON result = icon;
        ReleaseSRWLockExclusive(&g_themeLock);
        return result;
    }
    return nullptr;
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

// For a tray outside Explorer (Tourne Desktop's own, in shell mode, which has
// no Explorer to swap icons in): what an icon shows in the design, as
// ThemeTrayCopyData would put in, or null for its own. Kept by the design's
// cache, so the caller doesn't destroy it. Safe from another thread.
HICON TrayDesignIcon(HWND owner, std::wstring const& tip) {
    AcquireSRWLockShared(&g_settingsLock);
    HICON icon = nullptr;
    if (g_settings.trayTheme) {
        icon = ProgramTrayIcon(owner);
        if (!icon) {
            std::string design = DesignForOwner(owner, TipLines(tip));
            icon = design.empty() ? nullptr : ThemeIcon(design);
        }
    }
    ReleaseSRWLockShared(&g_settingsLock);
    return icon;
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
        tip = TipLines(text);
    } else {
        AcquireSRWLockShared(&g_tipsLock);
        auto it = g_tips.find(TipKey(owner, id));
        if (it != g_tips.end()) {
            tip = it->second;
        }
        ReleaseSRWLockShared(&g_tipsLock);
    }
    HICON icon = ProgramTrayIcon(owner);
    if (!icon) {
        std::string design = DesignForOwner(owner, tip);
        icon = design.empty() ? nullptr : ThemeIcon(design);
    }
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

////////////////////////////////////////////////////////////////////////////////
// Tray hosting, in Explorer: every icon programs add, kept from the
// Shell_NotifyIcon messages Explorer's taskbar receives, so the tray doesn't
// depend on the classic one StartAllBack brings back (Windows 11's own
// taskbar gets the same messages). Each icon is copied as it arrives, since
// its program may destroy its own.

struct HostedIcon {
    HWND owner = nullptr;
    UINT id = 0;
    GUID guid{};
    bool hasGuid = false;
    UINT callback = 0;
    UINT version = 0;
    DWORD state = 0;       // NIS_*
    HICON icon = nullptr;  // this process's copy
    bool themed = false;   // showing a design's or a program icon
    int tries = 0;         // passes that found nothing to theme it with
};

SRWLOCK g_hostedLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::vector<HostedIcon> g_hosted;  // in the order they were added

// NOTIFYICONDATA as the tray receives it (32-bit handles), from its start.
constexpr size_t kNidCallback = 16;
constexpr size_t kNidIcon = 20;
constexpr size_t kNidState = 280;
constexpr size_t kNidStateMask = 284;
constexpr size_t kNidVersion = 800;
constexpr size_t kNidGuid = 936;

// A 32-bit handle from the tray's data, widened the way Windows does.
template <typename T>
T WideHandle(DWORD value) {
    return (T)(LONG_PTR)(LONG)value;
}

void RecordHostedIcon(const COPYDATASTRUCT* copyData) {
    if (!copyData || copyData->dwData != kTrayCopyDataId || !copyData->lpData ||
        copyData->cbData < kTrayHeaderSize + 24) {
        return;
    }
    const BYTE* data = (const BYTE*)copyData->lpData;
    size_t size = copyData->cbData;
    auto field = [&](size_t offset) -> DWORD {
        DWORD value = 0;
        if (kTrayHeaderSize + offset + 4 <= size) {
            memcpy(&value, data + kTrayHeaderSize + offset, 4);
        }
        return value;
    };
    DWORD signature, message;
    memcpy(&signature, data, 4);
    memcpy(&message, data + 4, 4);
    if (signature != kTraySignature) {
        return;
    }
    HWND owner = WideHandle<HWND>(field(4));
    UINT id = field(8);
    UINT flags = field(12);
    GUID guid{};
    bool hasGuid = (flags & NIF_GUID) && kTrayHeaderSize + kNidGuid + sizeof(GUID) <= size;
    if (hasGuid) {
        memcpy(&guid, data + kTrayHeaderSize + kNidGuid, sizeof(GUID));
    }
    bool themedIcon = (flags & NIF_ICON) && IsThemeIcon(field(kNidIcon));

    AcquireSRWLockExclusive(&g_hostedLock);
    auto it = std::find_if(g_hosted.begin(), g_hosted.end(), [&](HostedIcon const& h) {
        return hasGuid ? h.hasGuid && IsEqualGUID(h.guid, guid)
                       : !h.hasGuid && h.owner == owner && h.id == id;
    });
    if (message == NIM_DELETE) {
        if (it != g_hosted.end()) {
            if (it->icon) {
                DestroyIcon(it->icon);
            }
            g_hosted.erase(it);
        }
    } else if (message == NIM_ADD || message == NIM_MODIFY || message == NIM_SETVERSION) {
        // An icon from before this started is taken in once a change brings
        // what's needed to show and click it.
        if (it == g_hosted.end() &&
            (message == NIM_ADD || ((flags & NIF_MESSAGE) && (flags & NIF_ICON)))) {
            g_hosted.push_back({owner, id, guid, hasGuid});
            it = g_hosted.end() - 1;
        }
        if (it != g_hosted.end()) {
            it->owner = owner;
            it->id = id;
            if (flags & NIF_MESSAGE) {
                it->callback = field(kNidCallback);
            }
            if (flags & NIF_ICON) {
                HICON source = WideHandle<HICON>(field(kNidIcon));
                HICON copy = source ? CopyIcon(source) : nullptr;
                if (it->icon) {
                    DestroyIcon(it->icon);
                }
                it->icon = copy;
                it->themed = themedIcon;
                it->tries = 0;
            }
            if (flags & NIF_STATE) {
                DWORD mask = field(kNidStateMask);
                it->state = (it->state & ~mask) | (field(kNidState) & mask);
            }
            if (message == NIM_SETVERSION) {
                it->version = field(kNidVersion);
            }
        }
    }
    ReleaseSRWLockExclusive(&g_hostedLock);
}

// A copy of the hosted icons, with their own copies of the icons (the caller
// destroys them), dropping ones whose program has gone.
std::vector<HostedIcon> HostedIcons() {
    std::vector<HostedIcon> icons;
    AcquireSRWLockExclusive(&g_hostedLock);
    for (auto it = g_hosted.begin(); it != g_hosted.end();) {
        if (!IsWindow(it->owner)) {
            if (it->icon) {
                DestroyIcon(it->icon);
            }
            it = g_hosted.erase(it);
            continue;
        }
        HostedIcon copy = *it;
        copy.icon = it->icon ? CopyIcon(it->icon) : nullptr;
        icons.push_back(copy);
        ++it;
    }
    ReleaseSRWLockExclusive(&g_hostedLock);
    return icons;
}

void ClearHostedIcons() {
    AcquireSRWLockExclusive(&g_hostedLock);
    for (auto& h : g_hosted) {
        if (h.icon) {
            DestroyIcon(h.icon);
        }
    }
    g_hosted.clear();
    ReleaseSRWLockExclusive(&g_hostedLock);
}

// Programs asked to add their icons again, as when Explorer starts, so the
// ones from before hosting began are taken in too. Only where there's no
// classic tray to read them from.
void SeedHostedIcons() {
    UINT taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    EnumWindows(
        [](HWND hWnd, LPARAM param) __stdcall -> BOOL {
            DWORD pid = 0;
            GetWindowThreadProcessId(hWnd, &pid);
            if (pid != GetCurrentProcessId()) {
                PostMessageW(hWnd, (UINT)param, 0, 0);
            }
            return TRUE;
        },
        (LPARAM)taskbarCreated);
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
    AcquireSRWLockShared(&g_hostedLock);
    for (auto const& h : g_hosted) {
        if (std::none_of(owners.begin(), owners.end(),
                         [&](TrayButtonOwner const& o) { return o.owner == h.owner && o.id == h.id; })) {
            owners.push_back({h.owner, h.id});
        }
    }
    ReleaseSRWLockShared(&g_hostedLock);
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
        HICON icon = ProgramTrayIcon(owner);
        if (!icon) {
            std::string design = DesignForOwner(owner, tip);
            icon = design.empty() ? nullptr : ThemeIcon(design);
        }
        if (!icon) {
            continue;
        }
        NOTIFYICONDATAW data{sizeof(data)};
        data.hWnd = owner;
        data.uID = id;
        data.uFlags = NIF_ICON;
        data.hIcon = icon;
        Shell_NotifyIconW(NIM_MODIFY, &data);
    }
}

// In Explorer, each relay pass: hosted icons still showing their program's own
// art (one that came by before the settings or the icon file were ready, or
// that the swap missed) are themed now. One with nothing to theme it with is
// tried a few passes more, then left until its program sends a new icon.
void ThemeMissedIcons() {
    if (!g_themeEnabled) {
        return;
    }
    struct Missed {
        HWND owner;
        UINT id;
        GUID guid;
        bool hasGuid;
    };
    std::vector<Missed> missed;
    AcquireSRWLockExclusive(&g_hostedLock);
    for (auto& h : g_hosted) {
        if (!h.themed && h.tries < 6 && h.icon) {
            h.tries++;
            missed.push_back({h.owner, h.id, h.guid, h.hasGuid});
        }
    }
    ReleaseSRWLockExclusive(&g_hostedLock);
    for (auto const& m : missed) {
        if (!IsWindow(m.owner)) {
            continue;
        }
        HICON icon = ProgramTrayIcon(m.owner);
        if (!icon) {
            AcquireSRWLockShared(&g_tipsLock);
            auto live = g_tips.find(TipKey(m.owner, m.id));
            std::wstring tip = live != g_tips.end() ? live->second : std::wstring();
            ReleaseSRWLockShared(&g_tipsLock);
            std::string design = DesignForOwner(m.owner, tip);
            icon = design.empty() ? nullptr : ThemeIcon(design);
        }
        if (!icon) {
            continue;
        }
        NOTIFYICONDATAW data{sizeof(data)};
        data.hWnd = m.owner;
        data.uID = m.id;
        data.uFlags = NIF_ICON | (m.hasGuid ? NIF_GUID : 0);
        data.guidItem = m.guid;
        data.hIcon = icon;
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
    // Windows Security's own icon, by its program: its window class isn't always "ATL:..." (on 26100
    // it wasn't, and the hidden icons flyout showed it beside the stand-in, with its own menu).
    if (OwnerFileName(owner) == L"securityhealthsystray.exe") {
        return g_settings.enabled[kSecurity];
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
            RecordHostedIcon(&themed);
            return DefSubclassProc(hWnd, msg, wParam, (LPARAM)&themed);
        }
        RecordHostedIcon(copyData);
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
        // Without a classic tray to read them from, the icons from before
        // this started are asked for again.
        HWND notify = FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr);
        if (!notify || !FindWindowExW(notify, nullptr, TOOLBARCLASSNAMEW, nullptr)) {
            SeedHostedIcons();
        }
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
                         TipLines(tip));
    }
    RegCloseKey(key);
    return tips;
}

// Any process's image path by id, even one this process may not open (NVIDIA's
// display container is one): SystemProcessIdInformation, as Task Manager uses.

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

// Windows 11's own "show in the taskbar" for each icon, by executable name and
// id: where an icon goes when there's no classic tray to say.
std::unordered_map<std::wstring, bool> RegistryPromoted() {
    std::unordered_map<std::wstring, bool> promoted;
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\NotifyIconSettings", 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return promoted;
    }
    WCHAR subkey[256];
    for (DWORD i = 0;; i++) {
        DWORD length = ARRAYSIZE(subkey);
        LONG result = RegEnumKeyExW(key, i, subkey, &length, nullptr, nullptr, nullptr, nullptr);
        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (result != ERROR_SUCCESS) {
            continue;
        }
        WCHAR path[MAX_PATH * 2];
        DWORD size = sizeof(path);
        if (RegGetValueW(key, subkey, L"ExecutablePath", RRF_RT_REG_SZ, nullptr, path, &size) != ERROR_SUCCESS) {
            continue;
        }
        DWORD id = 0, isPromoted = 0;
        size = sizeof(id);
        RegGetValueW(key, subkey, L"UID", RRF_RT_REG_DWORD, nullptr, &id, &size);
        size = sizeof(isPromoted);
        RegGetValueW(key, subkey, L"IsPromoted", RRF_RT_REG_DWORD, nullptr, &isPromoted, &size);
        promoted[LowerFileName(path) + L"|" + std::to_wstring(id)] = isPromoted != 0;
    }
    RegCloseKey(key);
    return promoted;
}

// In Explorer: the icons in the overflow, or all the tray's icons. The
// hosted ones first (see above), then any only the classic tray has, which
// covers icons from before hosting began while StartAllBack's tray is there.
std::vector<BYTE> BuildHiddenIconsPacket(bool all) {
    std::vector<BYTE> packet;
    auto put = [&](const void* data, size_t size) {
        auto* bytes = (const BYTE*)data;
        packet.insert(packet.end(), bytes, bytes + size);
    };
    UINT count = 0;
    put(&count, 4);

    auto registryTips = RegistryTips();
    auto tipFor = [&](HWND owner, UINT id) {
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
        return tip;
    };
    auto putIcon = [&](HWND owner, UINT id, UINT callback, UINT version, bool overflow,
                       std::vector<uint32_t> const& pixels, int width) {
        std::wstring tip = tipFor(owner, id);
        uint64_t owner64 = (uint64_t)(ULONG_PTR)owner;
        UINT size = pixels.empty() ? 0 : width;
        UINT chars = (UINT)tip.size();
        UINT flags = overflow ? 1 : 0;
        put(&owner64, 8);
        put(&id, 4);
        put(&callback, 4);
        put(&version, 4);
        put(&flags, 4);
        put(&size, 4);
        put(&chars, 4);
        put(tip.data(), chars * sizeof(WCHAR));
        put(pixels.data(), pixels.size() * 4);
        count++;
    };

    // The classic tray (StartAllBack's), when it's there.
    struct ClassicIcon {
        HWND owner;
        UINT id, callback, version;
        bool overflow;
        int image;
    };
    std::vector<ClassicIcon> classic;
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    HWND notify = tray ? FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
    HWND toolbar = notify ? FindWindowExW(notify, nullptr, TOOLBARCLASSNAMEW, nullptr) : nullptr;
    // The toolbar's own comctl32, so the image list functions match it.
    using GetIcon_t = HICON(WINAPI*)(HIMAGELIST, int, UINT);
    using GetIconSize_t = BOOL(WINAPI*)(HIMAGELIST, int*, int*);
    GetIcon_t getIcon = nullptr;
    HIMAGELIST images = nullptr;
    int width = GetSystemMetrics(SM_CXSMICON), height = width;
    if (toolbar) {
        HMODULE comctl = (HMODULE)GetClassLongPtrW(toolbar, GCLP_HMODULE);
        getIcon = comctl ? (GetIcon_t)GetProcAddress(comctl, "ImageList_GetIcon") : nullptr;
        auto getIconSize = comctl ? (GetIconSize_t)GetProcAddress(comctl, "ImageList_GetIconSize") : nullptr;
        images = (HIMAGELIST)SendMessageW(toolbar, TB_GETIMAGELIST, 0, 0);
        if (images && getIconSize) {
            getIconSize(images, &width, &height);
        }
        int buttons = (int)SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0);
        for (int i = 0; i < buttons; i++) {
            TBBUTTON button{};
            if (!SendMessageW(toolbar, TB_GETBUTTON, i, (LPARAM)&button) || !button.dwData) {
                continue;
            }
            // The button data: owner window at +0, id at +8, callback message
            // at +12, the icon's own NIS_* state at +16, NOTIFYICON version at
            // +20. NIS_HIDDEN there means its program hid it, not the user.
            auto* data = (const BYTE*)button.dwData;
            HWND owner = *(const HWND*)data;
            UINT id = *(const UINT*)(data + 8);
            if (!IsWindow(owner) || (*(const DWORD*)(data + 16) & NIS_HIDDEN) || IsReplacedByStandIn(owner, id)) {
                continue;
            }
            classic.push_back({owner, id, *(const UINT*)(data + 12), *(const UINT*)(data + 20),
                               (button.fsState & TBSTATE_HIDDEN) != 0, button.iBitmap});
        }
    }
    // Classic tray order first, so nothing changes while it's there, then the
    // icons only we have.
    std::vector<std::pair<HWND, UINT>> sent;
    for (auto const& c : classic) {
        sent.push_back({c.owner, c.id});
        if (!c.overflow && !all) {
            continue;
        }
        std::vector<uint32_t> pixels;
        if (images && getIcon) {
            if (HICON icon = getIcon(images, c.image, ILD_NORMAL)) {
                pixels = IconPixels(icon, width);
                DestroyIcon(icon);
            }
        }
        putIcon(c.owner, c.id, c.callback, c.version, c.overflow && !g_settings.allAppIcons, pixels, width);
    }
    auto promoted = RegistryPromoted();
    for (auto& h : HostedIcons()) {
        bool known = std::find(sent.begin(), sent.end(), std::pair<HWND, UINT>{h.owner, h.id}) != sent.end();
        if (!known && h.icon && h.callback && !(h.state & NIS_HIDDEN) && !IsReplacedByStandIn(h.owner, h.id)) {
            // Where Windows' own settings put it: in the bar only if promoted.
            auto it = promoted.find(OwnerFileName(h.owner) + L"|" + std::to_wstring(h.id));
            bool overflow = !g_settings.allAppIcons && (it == promoted.end() || !it->second);
            if (overflow || all) {
                int size = GetSystemMetrics(SM_CXSMICON);
                putIcon(h.owner, h.id, h.callback, h.version, overflow, IconPixels(h.icon, size), size);
            }
        }
        if (h.icon) {
            DestroyIcon(h.icon);
        }
    }    memcpy(packet.data(), &count, 4);
    return packet;
}

// The relay packs the hidden icons in Explorer and sends them back as
// WM_COPYDATA while this waits; the window procedure stores them here.
std::vector<HiddenIcon> g_receivedHidden;
bool g_hiddenReceived;

void AddPluginIcons(std::vector<HiddenIcon>* icons);

bool RequestHiddenIcons(std::vector<HiddenIcon>* icons, LPARAM which = kRelayOverflowIcons) {
    HWND toolbar = RelayToolbar();
    HWND wnd = g_wnd;
    if (!toolbar || !wnd || !g_msgRelayOverflow) {
        return false;
    }
    g_hiddenReceived = false;
    DWORD_PTR result = 0;
    // Not SMTO_BLOCK: the answer is a message sent to this thread while it
    // waits.
    SendMessageTimeoutW(toolbar, g_msgRelayOverflow, (WPARAM)wnd, which, SMTO_ABORTIFHUNG, 2000,
                        &result);
    if (!g_hiddenReceived) {
        return false;
    }
    *icons = std::move(g_receivedHidden);
    g_receivedHidden.clear();
    if (which == kRelayAllIcons) {
        AddPluginIcons(icons);  // (Tourne Desktop's plugin items too, placed as tray icons are)
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////////
// Flyouts: Windows 7 layouts in the Tourne design system. One at a time, on
// the mod thread.

enum FlyoutKind : int { kFlyoutNone, kFlyoutNetwork, kFlyoutPower, kFlyoutBrightness, kFlyoutOverflow, kFlyoutVpn, kFlyoutSecurity, kFlyoutMixer, kFlyoutNotify, kFlyoutBluetooth, kFlyoutDevices };

namespace ui {
constexpr COLORREF Rgb(uint32_t rgb) {
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}
// Set from the colour scheme (these are Tourne's): ground, raised row,
// divider, border, main text, accent, secondary and faint text, warning.
inline COLORREF kGround = Rgb(0x121C21);
inline COLORREF kRaised = Rgb(0x1B2A31);
inline COLORREF kLine = Rgb(0x1E2F36);
inline COLORREF kEdge = Rgb(0x4F8F6A);
inline COLORREF kInk = Rgb(0x66EA97);
inline COLORREF kGreen = Rgb(0x15FFA2);
inline COLORREF kSage = Rgb(0x64AA89);
inline COLORREF kSageAlt = Rgb(0x5D8D6F);
inline COLORREF kCoral = Rgb(0xFF6F77);
// text at 16% over ground, the design's hover fill.
inline COLORREF kHover = Rgb(0x1F3D34);
}  // namespace ui

void ApplyPaletteToUi() {
    auto const& p = g_palette;
    ui::kGround = ui::Rgb(p.ground);
    ui::kRaised = ui::Rgb(p.raised);
    ui::kLine = ui::Rgb(p.line);
    ui::kEdge = ui::Rgb(p.edge);
    ui::kInk = ui::Rgb(p.text);
    ui::kGreen = ui::Rgb(p.accent);
    ui::kSage = ui::Rgb(p.muted);
    ui::kSageAlt = ui::Rgb(p.dim);
    ui::kCoral = ui::Rgb(p.warn);
    ui::kHover = ui::Rgb(p.hover);
}

// Tourne Desktop's plugin items (plugin-bar-shared.cpp), as icons: the ones the user moved into the
// overflow show in its flyout with the hidden icons, go in and out of its folders, and pass their clicks
// to their plugin as the bar does. Asked of the plugin runtime as the icons are read (its answer comes
// while this waits); outside Tourne Desktop there's no runtime, so none.
std::vector<PluginBarItem> g_pluginItems;
unsigned g_pluginVersion = 0;  // that list's version, sent back with a click
ULONGLONG g_pluginItemsRead = 0;

HWND PluginHost() {
    return FindWindowExW(HWND_MESSAGE, nullptr, kPluginHostClass, nullptr);
}

void AddPluginIcons(std::vector<HiddenIcon>* icons) {
    static UINT ask = RegisterWindowMessageW(kPluginBarAskMessage);
    HWND host = PluginHost();
    HWND wnd = g_wnd;
    if (!host || !wnd) {
        return;
    }
    // At most once a second: a menu reads the icons a few times over.
    if (GetTickCount64() - g_pluginItemsRead > 1000) {
        DWORD_PTR result = 0;
        SendMessageTimeoutW(host, ask, (WPARAM)wnd, 0, SMTO_ABORTIFHUNG, 500, &result);
        g_pluginItemsRead = GetTickCount64();
    }
    int size = 0;
    for (auto const& icon : *icons) {
        size = std::max(size, icon.size);
    }
    if (!size) {
        size = GetSystemMetrics(SM_CXSMICON);
    }
    for (size_t i = 0; i < g_pluginItems.size(); i++) {
        auto const& item = g_pluginItems[i];
        HiddenIcon icon;
        icon.plugin = item.key;
        icon.id = (UINT)i;
        icon.tip = !item.tip.empty() ? item.tip : !item.text.empty() ? item.text : item.key;
        icon.size = size;
        icon.pixels = PluginItemPixels(item, size, ui::kInk);
        icons->push_back(std::move(icon));
    }
}

// A click on a plugin item's icon, posted to the runtime as the bar posts one.
void PluginIconClick(HiddenIcon const& icon, int click) {
    static UINT event = RegisterWindowMessageW(kPluginBarEventMessage);
    HWND host = PluginHost();
    for (size_t i = 0; host && i < g_pluginItems.size(); i++) {
        if (g_pluginItems[i].key == icon.plugin) {
            PluginBarEvent what = click == kClickRight ? kPluginRightClick : click == kClickMiddle ? kPluginMiddleClick : kPluginClick;
            PostMessageW(host, event, g_pluginVersion, MakePluginBarEvent((int)i, what));
            return;
        }
    }
}

// For Tourne Desktop's plugin panels and bar items, so they look like these flyouts: the colours as
// 0xRRGGBB, in order ground, raised, line, edge, text, accent, muted, dim, warn, bad, hover.
void FlyoutPalette(uint32_t* out) {
    Palette p = g_palette;
    uint32_t colors[11] = {p.ground, p.raised, p.line, p.edge, p.text, p.accent, p.muted, p.dim, p.warn, p.bad, p.hover};
    memcpy(out, colors, sizeof(colors));
}

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
    kHitPhoneLink,  // the notification centre's phone: 0 Phone Link, 1 Messages, 2 Send a file
    kHitCalPrev,
    kHitCalNext,
    kHitCalDay,
    kHitBtToggle,
    kHitBtRow,
    kHitBtAction,
    kHitMixerBt,
    kHitFolder,
    kHitFolderBack,
    kHitSecurityScan,  // index 1 quick, 2 full, 3 again as administrator
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
    bool primary = false;  // On the main screen.
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
        MONITORINFO displayInfo{sizeof(displayInfo)};
        bool primary = GetMonitorInfoW(display, &displayInfo) && (displayInfo.dwFlags & MONITORINFOF_PRIMARY);
        for (auto const& monitor : monitors) {
            Physical p{monitor};
            DWORD current = 0;
            p.ok = GetMonitorBrightness(monitor.hPhysicalMonitor, &p.min, &current, &p.max) &&
                   p.max > p.min;
            MonitorLevel level;
            level.primary = primary;
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
    // Shown without a pop-up (the database doesn't keep SuppressPopup): Tourne's own for a phone notification
    // that was already on the phone, marked launch="tourne-phone-quiet" (phone-ancs.cpp).
    bool quiet = false;
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
        // (emoji and fancy letters as the pixel font can draw them: PlainSymbols, phone-link.cpp)
        std::wstring text = PlainSymbols(XmlUnescape(xml.substr(at, close - at)));
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
        std::wstring label = PlainSymbols(XmlAttribute(tag, L"content"));
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
    // A phone app's, through Phone Link ("...!YourPhoneNotifications_com.google.Gmail"): the phone app's
    // own name, as Phone Link lists it.
    if (size_t at = aumid.find(L"!YourPhoneNotifications_"); at != std::wstring::npos) {
        std::wstring package = PhoneLower(aumid.substr(at + 24));
        if (package.find(L'.') != std::wstring::npos) {
            name = PhoneAppName(package, PhoneApps());
            cache[aumid] = name;
            return name;
        }
    }
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
    // A phone app's (Tourne.Phone.<its bundle id>, phone-ancs.cpp): the same design the Start menu's phone pane
    // gives it (PhoneAppDesign there), else the phone.
    if (has({L"tourne.phone."})) {
        if (has({L"mail", L"outlook", L"proton"})) return "app-mail";
        if (has({L"reddit"})) return "app-reddit";
        if (has({L"youtube", L"tiktok", L"twitch", L"netflix", L"video"})) return "app-video";
        if (has({L"amazon", L"ebay", L"etsy", L"shop", L"walmart", L"costco", L"target"})) return "app-shop";
        if (has({L"instagram", L"snapchat", L"camera", L"photos"})) return "app-capture";
        if (has({L"spotify"})) return "app-spotify";
        if (has({L"music", L"podcast", L"audible"})) return "app-audio";
        if (has({L"mobilesms", L"messages", L"whatsapp", L"discord", L"messenger", L"telegram", L"signal", L"slack",
                 L"teams", L"chat"})) {
            return "app-chat";
        }
        return "phone";
    }
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
                        item.quiet = std::string_view(payload, size).find("launch=\"tourne-phone-quiet\"") != std::string_view::npos;
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
    PhoneNoteClear(item.aumid, item.tag);  // (a phone's: kept cleared when the phone sends it again)
    try {
        auto history = winrt::Windows::UI::Notifications::ToastNotificationManager::History();
        history.Remove(item.tag, item.group, item.aumid);
    } catch (...) {
    }
}

void ClearAppNotifications(std::wstring const& aumid) {
    try {
        if (aumid.starts_with(L"Tourne.Phone."))
            for (auto const& toast : winrt::Windows::UI::Notifications::ToastNotificationManager::History().GetHistory(aumid))
                PhoneNoteClear(aumid, std::wstring(toast.Tag()));
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

// The app's own window, by its app ID: the one a window says it has, else its
// package's (packaged apps, such as Claude's).
HWND AppWindow(std::wstring const& aumid) {
    struct Search {
        std::wstring const& aumid;
        HWND found = nullptr;
    } search{aumid};
    EnumWindows(
        [](HWND w, LPARAM lp) -> BOOL {
            auto* s = (Search*)lp;
            if (!IsWindowVisible(w) || GetWindow(w, GW_OWNER) || !GetWindowTextLengthW(w)) return TRUE;
            std::wstring id;
            IPropertyStore* store = nullptr;
            if (SUCCEEDED(SHGetPropertyStoreForWindow(w, IID_PPV_ARGS(&store)))) {
                static const PROPERTYKEY kAppId{{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 5};
                PROPVARIANT v;
                PropVariantInit(&v);
                if (SUCCEEDED(store->GetValue(kAppId, &v)) && v.vt == VT_LPWSTR && v.pwszVal) id = v.pwszVal;
                PropVariantClear(&v);
                store->Release();
            }
            if (id.empty()) {
                using GetAppId = LONG(WINAPI*)(HANDLE, UINT32*, PWSTR);
                static auto getAppId =
                    (GetAppId)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "GetApplicationUserModelId");
                DWORD pid = 0;
                GetWindowThreadProcessId(w, &pid);
                if (HANDLE process = getAppId ? OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid) : nullptr) {
                    wchar_t buffer[256];
                    UINT32 size = ARRAYSIZE(buffer);
                    if (getAppId(process, &size, buffer) == ERROR_SUCCESS) id = buffer;
                    CloseHandle(process);
                }
            }
            if (!id.empty() && _wcsicmp(id.c_str(), s->aumid.c_str()) == 0) {
                s->found = w;
                return FALSE;
            }
            return TRUE;
        },
        (LPARAM)&search);
    return search.found;
}

bool TourneHoldsPhone();

// Opens a notification (or one of its buttons) the way Windows would.
void ActivateNotification(NotifyItem const& item, std::wstring const& arguments,
                          std::wstring const& activationType) {
    // Tourne's own for a phone's notification (phone-ancs.cpp): its buttons are the phone's actions ("tourne-phone:
    // <0 positive, 1 negative>:<its id>": answer or decline a call, clear), done on the phone by the taskbar part;
    // a click elsewhere on it opens nothing (it stays: the phone takes it away when it's dealt with there).
    if (item.aumid.starts_with(L"Tourne.Phone.")) {
        unsigned action = 0, uid = 0;
        if (swscanf(arguments.c_str(), L"tourne-phone:%u:%u", &action, &uid) == 2) {
            if (HWND start = FindWindowW(L"TourneStartMenu_" WH_MOD_ID, nullptr))
                PostMessageW(start, RegisterWindowMessageW(L"TournePhoneAction"), uid, action);
            RemoveNotification(item);
        } else {
            // Its body: the app on this PC to answer it in (Discord...), when it has one here (phone-link.cpp).
            std::wstring open = PhoneAppOnPc(item.aumid.substr(13), AppNameFor(item.aumid));
            if (!open.empty()) ShellExecuteW(nullptr, nullptr, open.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
        return;
    }
    // Phone Link's old ones, with Tourne holding the phone now: opening one would start Phone Link (which takes the
    // phone back), so a click only clears it.
    if (item.aumid.starts_with(L"Microsoft.YourPhone") && TourneHoldsPhone()) {
        RemoveNotification(item);
        return;
    }
    if (activationType == L"protocol" && !arguments.empty()) {
        ShellExecuteW(nullptr, L"open", arguments.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    } else if (activationType == L"system") {
        // Snooze and dismiss buttons: dismissing is what's left without a
        // snooze time.
    } else if (!ActivateWithActivator(item.aumid, arguments)) {
        // Apps with no handler of their own (Claude's among them) hear of a
        // click only through Windows' own notifications, which shell mode
        // doesn't run. So the app comes to the front instead, where it asks
        // the same thing, and the notification stays: the click didn't reach it.
        if (HWND w = AppWindow(item.aumid)) {
            ShellBringToFront(w);
            return;
        }
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

// The folder watch doesn't fire for a toast arriving (the database is written
// without the folder hearing of it), and no system event marks one either. So
// the database's shared-memory header (wpndatabase.db-shm, mapped read-only)
// is compared once a second: its change counter and last frame move with every
// write. A memory read, no disk. Mapped again when the folder watch sees the
// file made anew.
struct NotifyCounter {
    HANDLE file = INVALID_HANDLE_VALUE;
    HANDLE mapping = nullptr;
    const volatile DWORD* header = nullptr;
    DWORD change = 0, frame = 0;
} g_notifyCounter;

void CloseNotifyCounter() {
    auto& n = g_notifyCounter;
    if (n.header) UnmapViewOfFile((LPCVOID)n.header);
    if (n.mapping) CloseHandle(n.mapping);
    if (n.file != INVALID_HANDLE_VALUE) CloseHandle(n.file);
    n = {};
}

void OpenNotifyCounter() {
    CloseNotifyCounter();
    auto& n = g_notifyCounter;
    n.file = CreateFileW((NotificationsFolder() + L"\\wpndatabase.db-shm").c_str(), GENERIC_READ,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (n.file == INVALID_HANDLE_VALUE) return;
    n.mapping = CreateFileMappingW(n.file, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (n.mapping) n.header = (const volatile DWORD*)MapViewOfFile(n.mapping, FILE_MAP_READ, 0, 0, 48);
    if (!n.header) {
        CloseNotifyCounter();
        return;
    }
    n.change = n.header[2];  // (iChange)
    n.frame = n.header[4];   // (mxFrame)
}

// True once per change since the last call.
bool NotifyCounterMoved() {
    auto& n = g_notifyCounter;
    if (!n.header) return false;
    DWORD change = n.header[2], frame = n.header[4];
    if (change == n.change && frame == n.frame) return false;
    n.change = change;
    n.frame = frame;
    return true;
}

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
    int width = 0;      // The flyout's standard width.
    int fitWidth = 0;   // What it's drawn at: wider when Auto width fits the text.
    int needed = 0;     // The widest line wanted, from the last measure.
    int hover = 0;
    int dragSlider = -1;
    std::vector<Hit> hits;

    // Network
    std::vector<Connection> connections;
    std::vector<WifiNetwork> networks;
    bool wifiAdapter = false;
    bool wifiOff = false;
    bool wifiNeedsLocation = false;  // Windows withholds nearby networks: no location access for desktop apps
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

    // Bluetooth, in its own flyout and in the mixer's.
    std::vector<BluetoothDevice> btDevices;
    std::wstring btSelected;  // By name, since the list re-sorts.
    std::wstring btBusy;      // What a running request is doing.
    std::wstring btNote;      // How the last request went, when it didn't.
    bool btRadioOn = true;

    // Device batteries, in their own flyout and in the mixer's and Bluetooth's.
    std::vector<bat::Device> batteries;

    // Rearranging the hidden icons by dragging.
    int dragHidden = -1;
    POINT dragStart{};
    bool dragMoved = false;
    int dragTarget = -1;
    int dragFolder = -1;    // the folder tile under the dragged icon: it goes in there
    int dragStack = -1;     // the icon under it: the two make a new folder
    int stackWait = -1;     // the icon it's held over, until it has been there a moment (kTimerStackWait)
    bool dragBack = false;  // over the open folder's way back: it comes out of the folder
    HWND dragOver = nullptr;  // The taskbar under the pointer, once it's out of the flyout.
    DragGhost ghost;
    int overflowLeft = 0;
    int overflowTop = 0;
    int overflowCell = 0;
    // Tray folders in the hidden icons flyout: their tiles first in the grid,
    // and the one open (hidden then holds its icons), or -1.
    struct OverflowFolder {
        std::wstring name;
        std::wstring glyph;
        std::vector<HiddenIcon> icons;
        int setup = -1;  // its place in the folder layout (FolderLayout::folders)
    };
    std::vector<OverflowFolder> folders;
    int openFolder = -1;
    int folderTiles = 0;  // tiles before the icons in the grid shown now
    // A folder's name or glyph being typed, in a box over the flyout's title.
    HWND renameBox = nullptr;
    int renameSetup = -1;
    bool renameGlyph = false;
    // Notifications and calendar
    std::vector<NotifyApp> notifyApps;
    std::vector<std::array<int, 3>> notifyTargets;  // App, notification, button (-1 for none).
    int notifyScroll = 0;
    int notifyEntries = 0;
    bool notifyDnd = false;
    PhoneInfo phone;  // the linked phone, for the notification centre's phone (notificationPhone)
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
    return OpensOnDoubleClickIn(owner, g_settings.doubleClickApps);
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
    // The flyout width every line so far would need to show in full: the
    // line's own margins plus its text.
    mutable int needed = 0;

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
            if (!text.empty()) {
                int wanted = rc.left + Measure(font, text, spacing).cx + (width - rc.right);
                needed = std::max(needed, wanted);
            }
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
void RenderBluetoothBatteries(Canvas& c);

// Network flyout

bool LocationDeniedForDesktopApps();
WLAN_INTERFACE_INFO_LIST* WlanInterfaces();
void OpenWlan();

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

    g_flyout.wifiNeedsLocation = false;
    if (!g_settings.nearbyWifi) {
        return;  // (Windows counts the list as location data; the user said no)
    }
    WLAN_INTERFACE_INFO_LIST* interfaces = WlanInterfaces();
    if (!interfaces) {
        return;
    }
    bool refused = false;
    for (DWORD i = 0; i < interfaces->dwNumberOfItems; i++) {
        GUID iface = interfaces->InterfaceInfo[i].InterfaceGuid;
        WLAN_AVAILABLE_NETWORK_LIST* list = nullptr;
        DWORD result = WlanGetAvailableNetworkList(g_wlan, &iface, 0, nullptr, &list);
        if (result == ERROR_INVALID_HANDLE) {
            OpenWlan();  // (gone stale between the two calls)
            result = g_wlan ? WlanGetAvailableNetworkList(g_wlan, &iface, 0, nullptr, &list) : result;
        }
        if (result != ERROR_SUCCESS) {
            refused |= result == ERROR_ACCESS_DENIED;
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
    // Since Windows 11 24H2 the list of nearby networks is only for apps with location access (Windhawk
    // counts as a desktop app): refused outright, or only the connected network given back.
    bool onlyConnected = std::all_of(g_flyout.networks.begin(), g_flyout.networks.end(),
                                     [](WifiNetwork const& w) { return w.connected; });
    g_flyout.wifiNeedsLocation = g_settings.locationNote &&
                                 (refused || (onlyConnected && LocationDeniedForDesktopApps()));

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

// Location access for desktop apps is off: for the whole device, or for desktop apps.
bool LocationDeniedForDesktopApps() {
    constexpr WCHAR kStore[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\ConsentStore\\location";
    auto denied = [](HKEY root, std::wstring const& key) {
        WCHAR value[16] = L"";
        DWORD size = sizeof(value);
        return RegGetValueW(root, key.c_str(), L"Value", RRF_RT_REG_SZ, nullptr, value, &size) == ERROR_SUCCESS &&
               !_wcsicmp(value, L"Deny");
    };
    return denied(HKEY_LOCAL_MACHINE, kStore) || denied(HKEY_CURRENT_USER, kStore) ||
           denied(HKEY_CURRENT_USER, std::wstring(kStore) + L"\\NonPackaged");
}

void ScanWifi() {
    if (!g_settings.nearbyWifi) {
        return;
    }
    if (WLAN_INTERFACE_INFO_LIST* interfaces = WlanInterfaces()) {
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
    } else if (!g_settings.nearbyWifi) {
        // Turned off in settings: Windows' own list does the looking.
        c.Link(L"Show available networks", 3, hits, hover);
        c.y += c.Px(4);
    } else if (f.wifiNeedsLocation) {
        // Windows gives nearby networks only to apps allowed the location.
        note(L"Windows shows nearby networks only to");
        c.y -= c.Px(6);
        note(L"apps with location access.");
        c.Link(L"Location settings", 2, hits, hover);
        c.y += c.Px(4);
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

// Scrolling over the brightness icon steps the main screen's brightness, on a
// worker of its own: started by the first scroll, kept for the next ones.
constexpr int kBrightnessWheelStep = 5;
BrightnessShared* g_brightnessWheel;
int g_brightnessWheelPending;  // Notches scrolled before the levels were read.

void ApplyBrightnessWheel() {
    auto* shared = g_brightnessWheel;
    if (!shared || !g_brightnessWheelPending) {
        return;
    }
    EnterCriticalSection(&shared->lock);
    if (shared->loaded) {
        // The main screen's monitor, else the first that answers.
        int target = -1;
        for (size_t i = 0; i < shared->levels.size(); i++) {
            auto const& level = shared->levels[i];
            if (level.value >= 0 && (target < 0 || (level.primary && !shared->levels[target].primary))) {
                target = (int)i;
            }
        }
        if (target >= 0) {
            auto& level = shared->levels[target];
            // The flyout may have moved it since this worker read it.
            if (target < (int)g_brightnessCache.size() && g_brightnessCache[target].value >= 0) {
                level.value = g_brightnessCache[target].value;
            }
            level.value = std::clamp(level.value + g_brightnessWheelPending * kBrightnessWheelStep, 0, 100);
            shared->pending[target] = level.value;
            if (target < (int)g_brightnessCache.size()) {
                g_brightnessCache[target].value = level.value;
            }
        }
        g_brightnessWheelPending = 0;
    }
    LeaveCriticalSection(&shared->lock);
    SetEvent(shared->wake);
}

void WheelBrightness(int notches) {
    if (!notches) {
        return;
    }
    g_brightnessWheelPending += notches;
    if (!g_brightnessWheel) {
        auto* shared = new BrightnessShared();
        shared->notify = g_wnd;
        HANDLE thread = CreateThread(nullptr, 0, BrightnessThreadProc, shared, 0, nullptr);
        if (!thread) {
            delete shared;
            g_brightnessWheelPending = 0;
            return;
        }
        g_brightnessThreads.push_back(thread);
        g_brightnessWheel = shared;
        return;  // Applied once it has read the levels.
    }
    ApplyBrightnessWheel();
}

void StopBrightnessWheel() {
    auto* shared = g_brightnessWheel;
    if (!shared) {
        return;
    }
    EnterCriticalSection(&shared->lock);
    shared->stop = true;
    shared->notify = nullptr;
    LeaveCriticalSection(&shared->lock);
    SetEvent(shared->wake);
    shared->Release();
    g_brightnessWheel = nullptr;
    g_brightnessWheelPending = 0;
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
    return std::clamp((int)g_flyout.hidden.size() + g_flyout.folderTiles, 1, kOverflowColumns);
}

// The order the hidden icons were dragged into, by program and icon id.
constexpr WCHAR kOrderKey[] = L"Software\\TourneTray";
constexpr WCHAR kOrderValue[] = L"OverflowOrder";

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
// The folders set up under Classic Tray Folders (through variables, so the
// build doesn't file them under this part): a name, an optional glyph, and
// patterns for the icons that belong in it.
struct FolderSetup {
    std::wstring name, glyph;
    std::vector<std::wstring> patterns;
    std::vector<std::wstring> pins;  // icons (program|id) put in it by dragging: they win over patterns
                                     // (a bare program name, from before: all of its icons)
};

// The folders as they are now: the settings' until one is changed in the
// flyout (dragged into, made, renamed, deleted), then kept beside the tray's
// places, where every build reads them.
struct FolderLayout {
    std::vector<FolderSetup> folders;
    std::vector<std::wstring> loose;  // icons dragged out of their folder (program|id, or a program from
                                      // before): no pattern takes them back
};

constexpr WCHAR kTrayFoldersValue[] = L"TrayFolders";  // "v1", then "folder:name<tab>glyph", "match:", "pin:", "loose:"
constexpr WCHAR kTrayFoldersBeforeValue[] = L"TrayFoldersBefore";  // as they were before "Folders as in the settings"

std::vector<FolderSetup> LoadFolderSetups();

// Folders with the same name are one; a pattern an earlier folder has already takes nothing later.
void TidyFolders(FolderLayout& layout) {
    std::vector<FolderSetup> tidy;
    std::vector<std::wstring> seen;
    for (auto& setup : layout.folders) {
        auto same = std::find_if(tidy.begin(), tidy.end(), [&](FolderSetup const& t) { return t.name == setup.name; });
        FolderSetup& into = same != tidy.end() ? *same : tidy.emplace_back(FolderSetup{setup.name, setup.glyph, {}, {}});
        if (into.glyph.empty()) into.glyph = setup.glyph;
        for (auto& pattern : setup.patterns) {
            if (std::find(seen.begin(), seen.end(), pattern) != seen.end()) continue;
            seen.push_back(pattern);
            into.patterns.push_back(std::move(pattern));
        }
        for (auto& pin : setup.pins)
            if (std::find(into.pins.begin(), into.pins.end(), pin) == into.pins.end()) into.pins.push_back(std::move(pin));
    }
    layout.folders = std::move(tidy);
}

FolderLayout LoadFolderLayout() {
    FolderLayout layout;
    auto saved = LoadTrayList(kTrayFoldersValue);
    if (!saved.empty() && saved[0] == L"v1") {
        for (size_t i = 1; i < saved.size(); i++) {
            std::wstring const& item = saved[i];
            size_t colon = item.find(L':');
            if (colon == std::wstring::npos) continue;
            std::wstring kind = item.substr(0, colon), rest = item.substr(colon + 1);
            if (kind == L"folder") {
                size_t tab = rest.find(L'\t');
                layout.folders.push_back({rest.substr(0, tab), tab == std::wstring::npos ? L"" : rest.substr(tab + 1), {}, {}});
            } else if (kind == L"loose") {
                layout.loose.push_back(rest);
            } else if (!layout.folders.empty() && kind == L"match") {
                layout.folders.back().patterns.push_back(rest);
            } else if (!layout.folders.empty() && kind == L"pin") {
                layout.folders.back().pins.push_back(rest);
            }
        }
    } else {
        layout.folders = LoadFolderSetups();
    }
    TidyFolders(layout);
    return layout;
}

void SaveFolderLayout(FolderLayout const& layout) {
    std::vector<std::wstring> items{L"v1"};
    for (auto const& setup : layout.folders) {
        items.push_back(L"folder:" + setup.name + L"\t" + setup.glyph);
        for (auto const& pattern : setup.patterns) items.push_back(L"match:" + pattern);
        for (auto const& pin : setup.pins) items.push_back(L"pin:" + pin);
    }
    for (auto const& file : layout.loose) items.push_back(L"loose:" + file);
    SaveTrayList(kTrayFoldersValue, items);
}

// Back to the folders in the settings; the folders as they were are kept, so
// that can be undone.
void ForgetFolderLayout() {
    auto made = LoadTrayList(kTrayFoldersValue);
    if (!made.empty()) SaveTrayList(kTrayFoldersBeforeValue, made);
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kTrayFoldersValue);
}

// Undoes ForgetFolderLayout.
void BringBackFolderLayout() {
    auto before = LoadTrayList(kTrayFoldersBeforeValue);
    if (before.empty()) return;
    SaveTrayList(kTrayFoldersValue, before);
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kTrayFoldersBeforeValue);
}

std::vector<FolderSetup> LoadFolderSetups() {
    constexpr PCWSTR kName = L"trayFolders.folders[%d].name";
    constexpr PCWSTR kGlyph = L"trayFolders.folders[%d].glyph";
    constexpr PCWSTR kIcon = L"trayFolders.folders[%d].icons[%d]";
    auto text = [](PCWSTR value) {
        std::wstring s = value ? value : L"";
        Wh_FreeStringSetting(value);
        return s;
    };
    std::vector<FolderSetup> setups;
    for (int i = 0; i < 64; i++) {
        FolderSetup setup;
        setup.name = text(Wh_GetStringSetting(kName, i));
        setup.glyph = text(Wh_GetStringSetting(kGlyph, i));
        for (int k = 0; k < 256; k++) {
            std::wstring pattern = text(Wh_GetStringSetting(kIcon, i, k));
            if (pattern.empty()) break;
            CharLowerBuffW(pattern.data(), (DWORD)pattern.size());
            setup.patterns.push_back(pattern);
        }
        if (setup.name.empty() && setup.patterns.empty()) break;
        setups.push_back(std::move(setup));
    }
    return setups;
}

// * any text, ? one character; both sides already lowercase.
bool FolderPatternMatch(std::wstring_view pattern, std::wstring_view text) {
    size_t p = 0, t = 0, star = std::wstring_view::npos, mark = 0;
    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == L'?' || pattern[p] == text[t])) {
            p++, t++;
        } else if (p < pattern.size() && pattern[p] == L'*') {
            star = p++, mark = t;
        } else if (star != std::wstring_view::npos) {
            p = star + 1, t = ++mark;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == L'*') p++;
    return p == pattern.size();
}

// A line of the program's version info (FileDescription, CompanyName) as written, else "".
std::wstring IconProgramVersionText(HWND owner, PCWSTR field) {
    static std::unordered_map<std::wstring, std::wstring> cache;  // "pid|field"
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    std::wstring key = std::to_wstring(pid) + L"|" + field;
    if (auto it = cache.find(key); it != cache.end()) return it->second;
    std::wstring text;
    if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        WCHAR path[MAX_PATH];
        DWORD size = ARRAYSIZE(path);
        if (QueryFullProcessImageNameW(process, 0, path, &size)) {
            DWORD handle = 0, bytes = GetFileVersionInfoSizeW(path, &handle);
            std::vector<BYTE> data(bytes);
            struct Translation { WORD language, codePage; }* translations = nullptr;
            UINT length = 0;
            if (bytes && GetFileVersionInfoW(path, 0, bytes, data.data()) &&
                VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", (void**)&translations, &length) &&
                length) {
                WCHAR query[96];
                swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\%ls", translations[0].language,
                           translations[0].codePage, field);
                WCHAR* value = nullptr;
                if (VerQueryValueW(data.data(), query, (void**)&value, &length) && length > 1) text = value;
            }
        }
        CloseHandle(process);
    }
    while (!text.empty() && iswspace(text.back())) text.pop_back();
    cache[key] = text;
    return text;
}

// The program's own name for itself (its file description) in lower case, else "".
std::wstring IconProgramDescription(HWND owner) {
    std::wstring description = IconProgramVersionText(owner, L"FileDescription");
    CharLowerBuffW(description.data(), (DWORD)description.size());
    return description;
}

// The folder an icon is in (an index into the layout's folders), else -1: put somewhere by hand
// first (in a folder, or out of them all; the icon itself, else its whole program, as folders were
// kept before), then the first folder with a pattern matching the program's name, its file name or
// the tooltip.
int HomeFolder(HiddenIcon const& icon, FolderLayout const& layout) {
    auto const& setups = layout.folders;
    std::wstring file = OwnerFileName(icon.owner);
    for (std::wstring const& placed : {HiddenIconKey(icon), file}) {
        for (int i = 0; i < (int)setups.size(); i++)
            if (std::find(setups[i].pins.begin(), setups[i].pins.end(), placed) != setups[i].pins.end()) return i;
        if (std::find(layout.loose.begin(), layout.loose.end(), placed) != layout.loose.end()) return -1;
    }
    // Pinned under another id of its program (some take a new one each run: MSI Afterburner), when every such
    // pin is in the one folder and none of them was dragged out.
    if (!file.empty() && icon.plugin.empty()) {
        std::wstring program = file + L"|";
        auto ofProgram = [&](std::wstring const& k) { return k.starts_with(program) && k.find(L'|', program.size()) == std::wstring::npos; };
        int found = -1;
        bool split = std::any_of(layout.loose.begin(), layout.loose.end(), ofProgram);
        for (int i = 0; i < (int)setups.size() && !split; i++)
            if (std::any_of(setups[i].pins.begin(), setups[i].pins.end(), ofProgram)) {
                if (found >= 0) split = true;
                found = i;
            }
        if (!split && found >= 0) return found;
    }
    std::wstring stem = file.ends_with(L".exe") ? file.substr(0, file.size() - 4) : file;
    std::wstring tip = icon.tip;
    CharLowerBuffW(tip.data(), (DWORD)tip.size());
    std::wstring description = IconProgramDescription(icon.owner);
    for (int i = 0; i < (int)setups.size(); i++)
        for (auto const& pattern : setups[i].patterns)
            if (FolderPatternMatch(pattern, tip) || FolderPatternMatch(pattern, file) ||
                FolderPatternMatch(pattern, stem) || (!description.empty() && FolderPatternMatch(pattern, description)))
                return i;
    return -1;
}

// The hidden icons shared out into their folders (the first that matches by
// the program's name, its file name or the tooltip); the rest stay loose.
void SortIntoFolders() {
    auto& f = g_flyout;
    std::wstring openName = f.openFolder >= 0 && f.openFolder < (int)f.folders.size() ? f.folders[f.openFolder].name : L"";
    f.folders.clear();
    f.folderTiles = 0;
    if (!g_settings.overflowFolders) {
        f.openFolder = -1;
        return;
    }
    FolderLayout layout = LoadFolderLayout();
    auto const& setups = layout.folders;
    if (setups.empty()) {
        f.openFolder = -1;
        return;
    }
    constexpr PCWSTR kShowEmpty = L"trayFolders.showEmptyFolders";
    bool showEmpty = Wh_GetIntSetting(kShowEmpty) != 0;
    std::vector<FlyoutState::OverflowFolder> folders(setups.size());
    std::vector<HiddenIcon> loose;
    for (auto& icon : f.hidden) {
        int home = HomeFolder(icon, layout);
        if (home >= 0) folders[home].icons.push_back(std::move(icon));
        else loose.push_back(std::move(icon));
    }
    for (int i = 0; i < (int)setups.size(); i++) {
        // (an empty folder made here, with no patterns of its own, shows: it's waiting to be filled)
        if (folders[i].icons.empty() && !showEmpty && !setups[i].patterns.empty()) continue;
        folders[i].name = setups[i].name;
        folders[i].glyph = setups[i].glyph;
        folders[i].setup = i;
        f.folders.push_back(std::move(folders[i]));
    }
    // The folder that was open stays open, by name.
    f.openFolder = -1;
    for (int i = 0; i < (int)f.folders.size(); i++)
        if (!openName.empty() && f.folders[i].name == openName) f.openFolder = i;
    if (f.openFolder >= 0) {
        f.hidden = f.folders[f.openFolder].icons;
    } else {
        f.hidden = std::move(loose);
        f.folderTiles = (int)f.folders.size();
    }
}

// A folder tile's preview: four icons at half size, from full-size pixels.
std::vector<uint32_t> HalfPixels(std::vector<uint32_t> const& pixels, int size) {
    int half = size / 2;
    std::vector<uint32_t> out((size_t)half * half);
    for (int y = 0; y < half; y++)
        for (int x = 0; x < half; x++) {
            uint32_t sum[4] = {};
            for (int dy = 0; dy < 2; dy++)
                for (int dx = 0; dx < 2; dx++) {
                    uint32_t p = pixels[(size_t)(y * 2 + dy) * size + x * 2 + dx];
                    for (int c = 0; c < 4; c++) sum[c] += (p >> (c * 8)) & 0xFF;
                }
            uint32_t v = 0;
            for (int c = 0; c < 4; c++) v |= ((sum[c] / 4) & 0xFF) << (c * 8);
            out[(size_t)y * half + x] = v;
        }
    return out;
}

void LoadOverflowFlyout() {
    g_flyout.hidden.clear();
    // Every icon, then the ones whose place is the overflow: the user's
    // placements first, else where Explorer's tray has them.
    RequestHiddenIcons(&g_flyout.hidden, kRelayAllIcons);
    TrayPlaces places = TrayPlaces::Load();
    std::erase_if(g_flyout.hidden, [&](HiddenIcon const& icon) {
        return places.For(icon, HiddenIconKey(icon)) != kPlaceOverflow;
    });
    // The stand-in itself never belongs in its own flyout.
    std::erase_if(g_flyout.hidden, [](HiddenIcon const& icon) {
        return icon.owner == g_wnd && icon.id == kIconIdBase + kOverflow;
    });
    ApplyOverflowOrder();
    SortIntoFolders();
    int s = g_flyout.scale;
    g_flyout.width = std::max(220 * s, 20 * s + OverflowColumns() * OverflowCell());
}

void RenderOverflow(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    if (f.openFolder >= 0 && f.openFolder < (int)f.folders.size()) {
        // Inside a folder: its name, with the way back.
        int id = MakeHit(kHitFolderBack, 0);
        RECT back{c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(14)};
        c.Text(g_flyout.label, hover == id || f.dragBack ? ui::kCoral : ui::kSage, back,
               L"< " + f.folders[f.openFolder].name, DT_LEFT, c.Px(1));
        hits.push_back({back, id});
        c.y += c.Px(14);
    } else {
        c.Label(L"Hidden icons", ui::kSage);
    }
    c.y += c.Px(8);

    if (f.hidden.empty() && !f.folderTiles) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"No hidden icons");
        c.y += c.Px(24);
    } else {
        int cell = OverflowCell();
        int columns = OverflowColumns();
        int tiles = f.folderTiles;
        int rows = ((int)f.hidden.size() + tiles + columns - 1) / columns;
        int left = (c.width - columns * cell) / 2;
        int hovered = -1, hoveredFolder = -1;
        f.overflowLeft = left;
        f.overflowTop = c.y;
        f.overflowCell = cell;
        // The folders' tiles: four of their icons at half size, or the glyph.
        for (int k = 0; k < tiles; k++) {
            auto const& folder = f.folders[k];
            RECT rc{left + (k % columns) * cell, c.y + (k / columns) * cell,
                    left + (k % columns + 1) * cell, c.y + (k / columns + 1) * cell};
            int id = MakeHit(kHitFolder, k);
            c.Frame({rc.left + c.Px(2), rc.top + c.Px(2), rc.right - c.Px(2), rc.bottom - c.Px(2)}, ui::kLine);
            if (hover == id || f.dragFolder == k) {
                hoveredFolder = k;
                c.Fill(rc, ui::kHover);
                c.Frame(rc, ui::kEdge);
            }
            if (!folder.glyph.empty()) {
                c.Text(f.body, ui::kInk, rc, folder.glyph, DT_CENTER);
            } else {
                for (int n = 0; n < 4 && n < (int)folder.icons.size(); n++) {
                    auto const& icon = folder.icons[n];
                    if (icon.size < 2) continue;
                    int half = icon.size / 2;
                    int x0 = rc.left + (cell - 2 * half) / 2 + (n % 2) * half;
                    int y0 = rc.top + (cell - 2 * half) / 2 + (n / 2) * half;
                    c.Pixels(HalfPixels(icon.pixels, icon.size), half, x0, y0);
                }
            }
            hits.push_back({rc, id});
        }
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
        for (int place = 0; place < (int)order.size(); place++) {
            int i = order[place];
            int slot = place + tiles;
            auto const& icon = f.hidden[i];
            RECT rc{left + (slot % columns) * cell, c.y + (slot / columns) * cell,
                    left + (slot % columns + 1) * cell, c.y + (slot / columns + 1) * cell};
            int id = MakeHit(kHitHidden, i);
            if (dragging ? i == f.dragHidden : hover == id) {
                hovered = i;
                c.Fill(rc, ui::kHover);
                c.Frame(rc, ui::kEdge);
            } else if (dragging && i == f.dragStack) {
                // The icon it would stack onto, framed as a folder tile is.
                c.Fill(rc, ui::kHover);
                c.Frame({rc.left + c.Px(2), rc.top + c.Px(2), rc.right - c.Px(2), rc.bottom - c.Px(2)}, ui::kEdge);
            }
            if (icon.size) {
                c.Pixels(icon.pixels, icon.size, rc.left + (cell - icon.size) / 2,
                         rc.top + (cell - icon.size) / 2);
            }
            hits.push_back({rc, id});
        }
        c.y += rows * cell + c.Px(6);

        std::wstring line;
        if (dragging && f.dragFolder >= 0 && f.dragFolder < (int)f.folders.size()) {
            line = L"Into " + f.folders[f.dragFolder].name;
        } else if (dragging && f.dragStack >= 0) {
            line = L"A new folder with both";
        } else if (dragging && f.dragBack) {
            line = L"Out of " + f.folders[f.openFolder].name;
        } else if (hoveredFolder >= 0) {
            size_t count = f.folders[hoveredFolder].icons.size();
            line = f.folders[hoveredFolder].name + L" (" + std::to_wstring(count) + (count == 1 ? L" icon)" : L" icons)");
        } else if (hovered >= 0) {
            line = f.hidden[hovered].tip.empty() ? L"(no tooltip)" : f.hidden[hovered].tip;
            line.resize(std::min(line.size(), line.find(L'\n')));  // its first line: this is one line of caption
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

// In Tourne Desktop, Tourne Studio at the tray folders' settings; in the
// Windhawk suite, StartAllBack's own list of tray icons when it's installed,
// else Windows'.
void OpenTrayCustomization() {
#ifdef TD_PREFIX
    td::OpenStudio(L"", L"trayFolders");
#else
    WCHAR path[MAX_PATH];
    if (ExpandEnvironmentStringsW(L"%ProgramFiles%\\StartAllBack\\StartAllBackCfg.exe", path,
                                  ARRAYSIZE(path)) &&
        GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
        Run(path);
    } else {
        OpenUri(L"ms-settings:taskbar");
    }
#endif
}

// The slot under a point in the hidden icons grid, or -1 outside it.
int HiddenSlotAt(POINT pt) {
    auto const& f = g_flyout;
    int count = (int)f.hidden.size();
    if (!count || f.overflowCell <= 0 || pt.x < f.overflowLeft || pt.y < f.overflowTop) {
        return -1;
    }
    int columns = OverflowColumns();
    int rows = (count + f.folderTiles + columns - 1) / columns;
    int column = (pt.x - f.overflowLeft) / f.overflowCell;
    int row = (pt.y - f.overflowTop) / f.overflowCell;
    if (column >= columns || row >= rows) {
        return -1;
    }
    // The folder tiles come first: a drop on one lands at the start.
    return std::clamp(row * columns + column - f.folderTiles, 0, count - 1);
}

void MoveHiddenIcon(int from, int to) {
    auto& hidden = g_flyout.hidden;
    HiddenIcon icon = std::move(hidden[from]);
    hidden.erase(hidden.begin() + from);
    hidden.insert(hidden.begin() + to, std::move(icon));
    SaveOverflowOrder();
}

// A hidden icon dragged out of the flyout: it follows the pointer, and the
// taskbar under it is told, so it can show where the icon would land.
UINT TrayDragOverMessage() {
    static UINT message = RegisterWindowMessageW(kTrayDragOverMessage);
    return message;
}

void DragHiddenOutside(POINT screen) {
    auto& f = g_flyout;
    if (f.dragHidden < 0 || f.dragHidden >= (int)f.hidden.size()) {
        return;
    }
    f.ghost.Move(f.hidden[f.dragHidden], screen);
    HWND bar = DropTargetAt(screen, kTaskbarClassPrefix);
    if (bar != f.dragOver && f.dragOver) {
        PostMessageW(f.dragOver, TrayDragOverMessage(), 0, 0);
    }
    f.dragOver = bar;
    if (bar) {
        PostMessageW(bar, TrayDragOverMessage(), 1, MAKELPARAM(screen.x, screen.y));
    }
}

void EndHiddenDragOutside() {
    auto& f = g_flyout;
    f.ghost.Hide();
    if (f.dragOver) {
        PostMessageW(f.dragOver, TrayDragOverMessage(), 0, 0);
    }
    f.dragOver = nullptr;
}

void LayoutFlyout(int iconKind);

bool g_flyoutMenu = false;  // a menu opened from the flyout is up: the flyout stays open under it

// Folders changed in the flyout: the layout read, changed, kept, and shown.
void ChangeFolders(std::function<void(FolderLayout&)> const& change) {
    FolderLayout layout = LoadFolderLayout();
    change(layout);
    SaveFolderLayout(layout);
    LoadOverflowFlyout();
    LayoutFlyout(-1);
    if (g_flyout.hwnd) InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
}

void ForgetPlacement(FolderLayout& layout, std::wstring const& key) {
    for (auto& setup : layout.folders) std::erase(setup.pins, key);
    std::erase(layout.loose, key);
}

// Placed by hand: the icon itself (program|id), so a program's other icons stay where they are.
std::wstring PlacedKey(HiddenIcon const& icon) {
    if (!icon.plugin.empty()) return HiddenIconKey(icon);  // (a plugin item: "plugin:<key>")
    return OwnerFileName(icon.owner).empty() ? L"" : HiddenIconKey(icon);
}

void PutInFolder(HiddenIcon const& icon, int setup) {
    std::wstring key = PlacedKey(icon);
    if (key.empty()) return;
    ChangeFolders([&](FolderLayout& layout) {
        if (setup < 0 || setup >= (int)layout.folders.size()) return;
        ForgetPlacement(layout, key);
        layout.folders[setup].pins.push_back(key);
    });
}

void TakeOutOfFolder(HiddenIcon const& icon) {
    std::wstring key = PlacedKey(icon);
    if (key.empty()) return;
    ChangeFolders([&](FolderLayout& layout) {
        ForgetPlacement(layout, key);
        layout.loose.push_back(key);
    });
}

void StartFolderRename(int setup, bool glyph);

// "New folder", or "New folder 2" and on when that's taken.
std::wstring FreeFolderName(FolderLayout const& layout) {
    std::wstring name = L"New folder";
    for (int n = 2; std::any_of(layout.folders.begin(), layout.folders.end(),
                                [&](FolderSetup const& s) { return s.name == name; });
         n++)
        name = L"New folder " + std::to_wstring(n);
    return name;
}

// One icon dropped on another: a new folder of the two, named at once.
void StackIntoNewFolder(HiddenIcon const& dragged, HiddenIcon const& onto) {
    std::wstring a = PlacedKey(onto), b = PlacedKey(dragged);
    if (a.empty() || b.empty()) return;
    int made = -1;
    ChangeFolders([&](FolderLayout& layout) {
        std::wstring name = FreeFolderName(layout);
        ForgetPlacement(layout, a);
        ForgetPlacement(layout, b);
        FolderSetup setup{name, L"", {}, {a}};
        if (b != a) setup.pins.push_back(b);
        layout.folders.push_back(std::move(setup));
        made = (int)layout.folders.size() - 1;
    });
    if (made >= 0) StartFolderRename(made, false);
}

// The name box: Enter (or a click away) keeps what's typed, Escape leaves it as it was. As the flyout
// closes (show false) it's kept without showing it.
void EndFolderRename(bool keep, bool show = true) {
    auto& f = g_flyout;
    HWND box = f.renameBox;
    if (!box) return;
    f.renameBox = nullptr;
    int length = GetWindowTextLengthW(box);
    std::wstring text(length + 1, L'\0');
    GetWindowTextW(box, text.data(), length + 1);
    text.resize(length);
    text.erase(0, text.find_first_not_of(L" \t"));
    text.erase(text.find_last_not_of(L" \t") + 1);
    DestroyWindow(box);
    int setup = f.renameSetup;
    bool glyph = f.renameGlyph;
    if (keep && (glyph || !text.empty())) {
        auto rename = [&](FolderLayout& layout) {
            if (setup < 0 || setup >= (int)layout.folders.size()) return;
            if (glyph) {
                layout.folders[setup].glyph = text;
                return;
            }
            // (two folders with one name would be one)
            std::wstring name = text;
            for (int n = 2; std::any_of(layout.folders.begin(), layout.folders.end(),
                                        [&](FolderSetup const& s) { return &s != &layout.folders[setup] && s.name == name; });
                 n++)
                name = text + L" " + std::to_wstring(n);
            // The open folder stays open under its new name.
            if (f.openFolder >= 0 && f.openFolder < (int)f.folders.size() && f.folders[f.openFolder].setup == setup)
                f.folders[f.openFolder].name = name;
            layout.folders[setup].name = name;
        };
        if (show) {
            ChangeFolders(rename);
        } else {
            FolderLayout layout = LoadFolderLayout();
            rename(layout);
            SaveFolderLayout(layout);
        }
    } else if (show && f.hwnd) {
        InvalidateRect(f.hwnd, nullptr, FALSE);
    }
}

LRESULT CALLBACK RenameBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR) {
    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN || wParam == VK_ESCAPE) {
                HWND parent = GetParent(hWnd);
                EndFolderRename(wParam == VK_RETURN);
                if (parent) SetFocus(parent);
                return 0;
            }
            break;
        case WM_CHAR:
            if (wParam == L'\r' || wParam == 27) return 0;  // (no beep)
            break;
        case WM_KILLFOCUS:
            if (g_flyout.renameBox == hWnd) PostMessageW(GetParent(hWnd), WM_APP_FLYOUT_RENAME_DONE, 0, 0);
            break;
        case WM_NCDESTROY:
            RemoveWindowSubclass(hWnd, RenameBoxProc, id);
            break;
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}

void StartFolderRename(int setup, bool glyph) {
    auto& f = g_flyout;
    if (!f.hwnd || f.kind != kFlyoutOverflow) return;
    EndFolderRename(true);
    FolderLayout layout = LoadFolderLayout();
    if (setup < 0 || setup >= (int)layout.folders.size()) return;
    f.renameSetup = setup;
    f.renameGlyph = glyph;
    // Over the flyout's title line.
    RECT client;
    GetClientRect(f.hwnd, &client);
    int s = f.scale;
    SetWindowLongPtrW(f.hwnd, GWL_STYLE, GetWindowLongPtrW(f.hwnd, GWL_STYLE) | WS_CLIPCHILDREN);
    std::wstring const& text = glyph ? layout.folders[setup].glyph : layout.folders[setup].name;
    f.renameBox = CreateWindowExW(0, L"EDIT", text.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 12 * s, 9 * s,
                                  client.right - 24 * s, 20 * s, f.hwnd, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!f.renameBox) return;
    SendMessageW(f.renameBox, WM_SETFONT, (WPARAM)f.body, FALSE);
    SendMessageW(f.renameBox, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(4 * s, 4 * s));
    SendMessageW(f.renameBox, EM_SETSEL, 0, -1);
    SetWindowSubclass(f.renameBox, RenameBoxProc, 1, 0);
    SetForegroundWindow(f.hwnd);
    SetFocus(f.renameBox);
    InvalidateRect(f.hwnd, nullptr, FALSE);
}

std::wstring MenuEscape(std::wstring text);
std::wstring IconMenuName(HiddenIcon const* icon, std::wstring const& key);

// The running icons whose place is the overflow (the flyout's own stand-in aside).
std::vector<HiddenIcon> OverflowIcons() {
    std::vector<HiddenIcon> all;
    RequestHiddenIcons(&all, kRelayAllIcons);
    TrayPlaces places = TrayPlaces::Load();
    std::erase_if(all, [&](HiddenIcon const& icon) {
        return (icon.owner == g_wnd && icon.id == kIconIdBase + kOverflow) ||
               places.For(icon, HiddenIconKey(icon)) != kPlaceOverflow;
    });
    return all;
}

// The flyout opened on a folder (by its place in the layout).
void OpenFolderSetup(int setup) {
    auto& f = g_flyout;
    if (!f.hwnd) return;
    for (int i = 0; i < (int)f.folders.size(); i++) {
        if (f.folders[i].setup != setup) continue;
        f.openFolder = i;
        LoadOverflowFlyout();
        LayoutFlyout(-1);
        InvalidateRect(f.hwnd, nullptr, FALSE);
        return;
    }
}

// A folder's icons, picked from a menu that comes back after each pick: every icon in the overflow,
// ticked when it's in this folder. One from another folder moves here; a ticked one goes back out,
// loose. Rename names it in the flyout; Done (or a click away) ends it.
void PickIntoFolder(POINT screen, int setup) {
    auto& f = g_flyout;
    bool rename = false;
    for (bool again = true; again;) {
        again = false;
        FolderLayout layout = LoadFolderLayout();
        if (setup < 0 || setup >= (int)layout.folders.size()) return;
        std::vector<MenuItem> icons;
        for (auto const& icon : OverflowIcons()) {
            if (PlacedKey(icon).empty()) continue;
            int home = HomeFolder(icon, layout);
            std::wstring label = IconMenuName(&icon, HiddenIconKey(icon));
            if (home >= 0 && home != setup) label += L"  (from " + MenuEscape(layout.folders[home].name) + L")";
            MenuItem item{label, [&again, icon, home, setup] {
                              again = true;
                              if (home == setup) TakeOutOfFolder(icon);
                              else PutInFolder(icon, setup);
                          }};
            item.checked = home == setup;
            icons.push_back(std::move(item));
        }
        std::sort(icons.begin(), icons.end(),
                  [](MenuItem const& a, MenuItem const& b) { return _wcsicmp(a.label.c_str(), b.label.c_str()) < 0; });
        std::vector<MenuItem> items;
        items.push_back({L"Rename " + MenuEscape(layout.folders[setup].name), [&rename] { rename = true; }});
        items.push_back({L"Done", [] {}});
        items.push_back({});
        if (icons.empty()) items.push_back({L"No icons in the overflow", nullptr});
        for (auto& item : icons) items.push_back(std::move(item));
        g_flyoutMenu = true;
        ShowMenu(items, MAKELPARAM(screen.x, screen.y));
        g_flyoutMenu = false;
    }
    if (rename) StartFolderRename(setup, false);
    if (f.hwnd && IsWindow(f.hwnd) && !f.renameBox) SetForegroundWindow(f.hwnd);
}

// A new folder in the overflow, empty or with one icon (moved out of its folder if it's in one), opened
// in the flyout; then the picker for the rest.
void NewOverflowFolder(POINT screen, HiddenIcon const* first) {
    std::wstring key = first ? PlacedKey(*first) : L"";
    int made = -1;
    ChangeFolders([&](FolderLayout& layout) {
        FolderSetup setup{FreeFolderName(layout), L"", {}, {}};
        if (!key.empty()) {
            ForgetPlacement(layout, key);
            setup.pins.push_back(key);
        }
        layout.folders.push_back(std::move(setup));
        made = (int)layout.folders.size() - 1;
    });
    if (made < 0) return;
    OpenFolderSetup(made);
    PickIntoFolder(screen, made);
}

// Auto sort: what an icon's program is for, from whole words of its file name, its own name for itself,
// its maker and its tooltip; the first kind that has one. The order matters where words overlap (NVIDIA's
// recorder is Recording before NVIDIA is Hardware; SteelSeries Sonar is Audio; the Bluetooth icon Explorer
// shows is Hardware before Explorer's are Windows). A folder whose name asks for a kind takes it ("{.GAME |
// .SOCIAL}" takes Games and Chat, "W I N D.u x" Windows); a kind no folder asks for goes where its next
// broader kind goes (Media, then Audio; Security, then Windows; AI, Network, Dev and the like, then Tools).
struct ProgramKind {
    PCWSTR folder;
    std::vector<PCWSTR> words;  // in the program's names and tooltip
    std::vector<PCWSTR> names;  // in a folder's name
    PCWSTR broader;
};

std::vector<ProgramKind> const& ProgramKinds() {
    static std::vector<ProgramKind> const kinds = {
        {L"Chat",
         {L"discord", L"vesktop", L"vencord", L"equicord", L"testcord", L"telegram", L"slack", L"teams", L"whatsapp",
          L"signal", L"element", L"skype", L"zoom", L"messenger", L"thunderbird", L"outlook", L"mail", L"phone link",
          L"yourphone", L"kde connect"},
         {L"chat", L"social", L"messages", L"messaging", L"comms", L"talk"},
         nullptr},
        {L"AI", {L"claude", L"chatgpt", L"copilot", L"ollama", L"lm studio"}, {L"ai", L"assistant"}, L"Tools"},
        {L"Recording",
         {L"obs", L"sharex", L"shadowplay", L"nvidia share", L"streamlabs", L"greenshot", L"lightshot", L"snagit",
          L"capture", L"screenshot", L"recordly", L"screen recorder"},
         {L"recording", L"record", L"capture", L"stream", L"streaming"},
         L"Media"},
        {L"Media",
         {L"spotify", L"vlc", L"foobar", L"musicbee", L"aimp", L"itunes", L"winamp", L"mpc", L"plex", L"tidal",
          L"deezer", L"audacious", L"media player", L"now playing", L"kodi", L"jellyfin"},
         {L"media", L"music", L"video", L"player", L"audio"},
         L"Audio"},
        {L"Audio",
         {L"volume", L"speaker", L"speakers", L"headphones", L"headset", L"voicemeeter", L"eartrumpet", L"soundswitch",
          L"equalizer", L"realtek", L"nahimic", L"sonar", L"wave link", L"audio", L"sound", L"microphone"},
         {L"audio", L"sound", L"music", L"volume"},
         L"Media"},
        {L"Games",
         {L"steam", L"epic games", L"battle.net", L"gog", L"galaxy", L"ea app", L"origin", L"ubisoft", L"uplay",
          L"riot", L"vanguard", L"playnite", L"xbox", L"rockstar", L"curseforge", L"overwolf", L"league of legends"},
         {L"game", L"games", L"gaming", L"play"},
         nullptr},
        {L"Downloads",
         {L"qbittorrent", L"utorrent", L"bittorrent", L"transmission", L"internet download manager", L"idman",
          L"abdm", L"ab download manager", L"jdownloader", L"free download manager", L"motrix"},
         {L"download", L"downloads", L"torrent"},
         L"Tools"},
        {L"Cloud",
         {L"onedrive", L"dropbox", L"google drive", L"googledrivefs", L"megasync", L"syncthing", L"pcloud", L"icloud",
          L"nextcloud"},
         {L"cloud", L"sync", L"drive"},
         L"Tools"},
        {L"Security",
         {L"windows security", L"securityhealthsystray", L"defender", L"malwarebytes", L"bitdefender", L"avast",
          L"avg", L"kaspersky", L"norton", L"eset", L"vpn", L"nordvpn", L"protonvpn", L"mullvad", L"wireguard",
          L"openvpn", L"bitwarden", L"1password", L"keepass", L"keepassxc", L"lastpass", L"dashlane", L"glasswire",
          L"simplewall", L"portmaster"},
         {L"security", L"secure", L"safety", L"vpn"},
         L"Windows"},
        {L"Network",
         {L"network", L"wi-fi", L"wifi", L"ethernet", L"netlimiter", L"tailscale", L"zerotier", L"hamachi", L"parsec",
          L"anydesk", L"teamviewer", L"rustdesk", L"localsend"},
         {L"network", L"net", L"web", L"remote"},
         L"Tools"},
        {L"Dev",
         {L"docker", L"github", L"visual studio", L"vscode", L"wsl", L"postman", L"jetbrains", L"toolbox"},
         {L"dev", L"code", L"coding"},
         L"Tools"},
        {L"Hardware",
         {L"alienware", L"alienfx", L"alienfan", L"command center", L"afterburner", L"rtss", L"rivatuner", L"hwinfo",
          L"icue", L"corsair", L"logitech", L"g hub", L"lghub", L"razer", L"synapse", L"steelseries", L"armoury",
          L"msi", L"nvidia", L"geforce", L"amd", L"radeon", L"intel", L"dell", L"samsung", L"magician", L"epson",
          L"printer", L"bluetooth", L"battery", L"signalrgb", L"openrgb", L"safely remove", L"usb", L"eject",
          L"sensor", L"fan"},
         {L"hardware", L"device", L"devices", L"hw", L"gear", L"peripherals"},
         nullptr},
        {L"Windows",
         {L"explorer", L"windows"},
         {L"windows", L"wind", L"win", L"os", L"system", L"microsoft"},
         nullptr},
        {L"Tools",
         {L"powertoys", L"command palette", L"everything", L"autohotkey", L"ditto", L"f.lux", L"flux", L"caffeine",
          L"windhawk", L"translucenttb", L"rainmeter", L"wallpaper engine", L"lively", L"displayfusion", L"quicklook",
          L"clipboard", L"sizer", L"unigetui", L"tourne"},
         {L"tool", L"tools", L"utility", L"utilities", L"util", L"utils"},
         nullptr},
    };
    return kinds;
}

int ProgramKindIndex(PCWSTR folder) {
    auto const& kinds = ProgramKinds();
    for (int i = 0; folder && i < (int)kinds.size(); i++)
        if (!wcscmp(kinds[i].folder, folder)) return i;
    return -1;
}

// The word at a word's start in lower-case text (not inside another word).
bool HasWord(std::wstring const& text, PCWSTR word) {
    size_t length = wcslen(word);
    for (size_t at = text.find(word); at != std::wstring::npos; at = text.find(word, at + 1)) {
        bool startOk = at == 0 || !iswalnum(text[at - 1]);
        bool endOk = at + length >= text.size() || !iswalpha(text[at + length]);  // ("hwinfo64" too)
        if (startOk && endOk) return true;
    }
    return false;
}

// A folder's name asks for a kind: one of its words, or a longer one with the name's letters run together.
bool FolderNameAsksFor(std::wstring name, ProgramKind const& kind) {
    CharLowerBuffW(name.data(), (DWORD)name.size());
    std::wstring together;
    for (wchar_t c : name)
        if (iswalnum(c)) together += c;
    for (PCWSTR word : kind.names)
        if (HasWord(name, word) || (wcslen(word) >= 4 && together.find(word) != std::wstring::npos)) return true;
    return false;
}

// The program's maker, without "Corporation", "Inc." and the like ("" if it names none).
std::wstring IconMaker(HiddenIcon const& icon) {
    std::wstring maker = IconProgramVersionText(icon.owner, L"CompanyName");
    maker = maker.substr(0, maker.find(L','));
    for (PCWSTR mark : {L"(R)", L"(r)", L"(TM)", L"(tm)", L"®", L"™"})
        for (size_t at; (at = maker.find(mark)) != std::wstring::npos;) maker.erase(at, wcslen(mark));
    for (bool cut = true; cut;) {
        cut = false;
        while (!maker.empty() && (iswspace(maker.back()) || maker.back() == L'.')) maker.pop_back();
        size_t space = maker.find_last_of(L' ');
        if (space == std::wstring::npos) break;
        std::wstring last = maker.substr(space + 1);
        CharLowerBuffW(last.data(), (DWORD)last.size());
        for (PCWSTR suffix : {L"corporation", L"corp", L"inc", L"incorporated", L"ltd", L"limited", L"llc",
                              L"gmbh", L"aps", L"a/s", L"co", L"company", L"s.a", L"ag", L"b.v", L"pty", L"plc",
                              L"technologies", L"software"})
            if (last == suffix) {
                maker.resize(space);
                cut = true;
                break;
            }
    }
    return maker;
}

// The kind an icon's program is (an index into ProgramKinds), else -1.
int IconKindOf(HiddenIcon const& icon) {
    std::wstring file = OwnerFileName(icon.owner);
    std::wstring stem = file.ends_with(L".exe") ? file.substr(0, file.size() - 4) : file;
    std::wstring tip = icon.tip.substr(0, icon.tip.find_first_of(L"\r\n"));
    std::wstring text = stem + L"\n" + IconProgramDescription(icon.owner) + L"\n" +
                        IconProgramVersionText(icon.owner, L"CompanyName") + L"\n" + tip;
    CharLowerBuffW(text.data(), (DWORD)text.size());
    auto const& kinds = ProgramKinds();
    for (int i = 0; i < (int)kinds.size(); i++)
        for (PCWSTR word : kinds[i].words)
            if (HasWord(text, word)) return i;
    return -1;
}

constexpr WCHAR kTrayFoldersBeforeSortValue[] = L"TrayFoldersBeforeSort";  // "settings": they were the settings'

// Icons into folders by what their programs are for, and the rest by maker when two or more share one.
// Each kind goes to the folder whose name asks for it, else the folder most of its icons are in now
// (the first found), else its broader kind's, else a folder of its own named for it (Chat, Media, Games...;
// made only for two icons or more). Only loose icons (folders stay as they are), or every icon (the folders
// that take a kind are kept, emptied and filled again; the rest go). The folders as they were are kept, to
// undo.
void AutoSortFolders(bool everything) {
    auto before = LoadTrayList(kTrayFoldersValue);
    if (before.empty()) before = {L"settings"};
    SaveTrayList(kTrayFoldersBeforeSortValue, before);
    std::vector<HiddenIcon> icons = OverflowIcons();
    ChangeFolders([&](FolderLayout& layout) {
        auto const& kinds = ProgramKinds();
        std::vector<std::wstring> taker(kinds.size());
        for (size_t k = 0; k < kinds.size(); k++)
            for (auto const& folder : layout.folders)
                if (FolderNameAsksFor(folder.name, kinds[k])) {
                    taker[k] = folder.name;
                    break;
                }
        struct Found {
            std::wstring key;
            int kind;
            bool placed;
        };
        std::vector<Found> found;
        std::vector<std::vector<std::pair<std::wstring, int>>> where(kinds.size());  // folder, icons
        for (auto const& icon : icons) {
            std::wstring key = PlacedKey(icon);
            if (key.empty()) continue;
            int home = HomeFolder(icon, layout), kind = IconKindOf(icon);
            found.push_back({key, kind, home >= 0});
            if (home < 0 || kind < 0) continue;
            auto& counts = where[kind];
            auto same = std::find_if(counts.begin(), counts.end(),
                                     [&](auto const& c) { return c.first == layout.folders[home].name; });
            if (same == counts.end()) counts.push_back({layout.folders[home].name, 1});
            else same->second++;
        }
        for (size_t k = 0; k < kinds.size(); k++) {
            if (!taker[k].empty() || where[k].empty()) continue;
            taker[k] = std::max_element(where[k].begin(), where[k].end(), [](auto const& a, auto const& b) {
                           return a.second < b.second;
                       })->first;
        }
        auto folderFor = [&](int kind) {
            for (int at = kind, steps = 0; at >= 0 && steps < 8; at = ProgramKindIndex(kinds[at].broader), steps++)
                if (!taker[at].empty()) return taker[at];
            return std::wstring(kinds[kind].folder);
        };
        std::vector<std::wstring> kept;
        if (everything) {
            FolderLayout fresh;
            for (auto const& folder : layout.folders)
                if (std::find(taker.begin(), taker.end(), folder.name) != taker.end()) {
                    fresh.folders.push_back({folder.name, folder.glyph, {}, {}});
                    kept.push_back(folder.name);
                }
            layout = std::move(fresh);
        }
        struct Sorted {
            std::wstring key, folder;
        };
        std::vector<Sorted> sorted;
        std::vector<std::pair<std::wstring, std::vector<std::wstring>>> makers;  // maker, keys
        for (size_t i = 0; i < found.size(); i++) {
            if (found[i].placed && !everything) continue;
            if (found[i].kind >= 0) {
                sorted.push_back({found[i].key, folderFor(found[i].kind)});
                continue;
            }
            std::wstring maker;
            for (auto const& icon : icons)
                if (PlacedKey(icon) == found[i].key) maker = IconMaker(icon);
            if (maker.empty()) continue;
            auto same = std::find_if(makers.begin(), makers.end(),
                                     [&](auto const& m) { return !_wcsicmp(m.first.c_str(), maker.c_str()); });
            if (same == makers.end()) {
                makers.emplace_back(maker, std::vector<std::wstring>{});
                same = makers.end() - 1;
            }
            same->second.push_back(found[i].key);
        }
        for (auto const& [maker, keys] : makers)
            for (auto const& key : keys) sorted.push_back({key, maker});
        auto existing = [&](std::wstring const& name) {
            return std::find_if(layout.folders.begin(), layout.folders.end(),
                                [&](FolderSetup const& s) { return !_wcsicmp(s.name.c_str(), name.c_str()); });
        };
        for (auto const& item : sorted) {
            auto into = existing(item.folder);
            if (into == layout.folders.end()) {
                size_t count = std::count_if(sorted.begin(), sorted.end(),
                                             [&](Sorted const& s) { return s.folder == item.folder; });
                if (count < 2) continue;
                layout.folders.push_back({item.folder, L"", {}, {}});
                into = layout.folders.end() - 1;
            }
            ForgetPlacement(layout, item.key);
            into->pins.push_back(item.key);
        }
        // Kept folders nothing went into go too.
        std::erase_if(layout.folders, [&](FolderSetup const& s) {
            return s.pins.empty() && std::find(kept.begin(), kept.end(), s.name) != kept.end();
        });
    });
}

// Undoes AutoSortFolders.
void UndoAutoSort() {
    auto before = LoadTrayList(kTrayFoldersBeforeSortValue);
    if (before.empty()) return;
    if (before.size() == 1 && before[0] == L"settings") RegDeleteKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kTrayFoldersValue);
    else SaveTrayList(kTrayFoldersValue, before);
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kTrayStoreKey, kTrayFoldersBeforeSortValue);
}

void ShowFolderMenu(POINT screen, int folder) {
    auto& f = g_flyout;
    if (folder < 0 || folder >= (int)f.folders.size()) return;
    int setup = f.folders[folder].setup;
    std::vector<MenuItem> items;
    items.push_back({L"Rename", [setup] { StartFolderRename(setup, false); }});
    items.push_back({L"Glyph (empty: its icons)", [setup] { StartFolderRename(setup, true); }});
    items.push_back({L"Pick its icons", [screen, setup] { PickIntoFolder(screen, setup); }});
    items.push_back({});
    items.push_back({L"Delete the folder (its icons stay)", [setup] {
                         ChangeFolders([&](FolderLayout& layout) {
                             if (setup < 0 || setup >= (int)layout.folders.size()) return;
                             // What it had by hand goes loose; what its patterns took finds its own way.
                             for (auto const& pin : layout.folders[setup].pins) layout.loose.push_back(pin);
                             layout.folders.erase(layout.folders.begin() + setup);
                         });
                     }});
    g_flyoutMenu = true;
    ShowMenu(items, MAKELPARAM(screen.x, screen.y));
    g_flyoutMenu = false;
    if (f.hwnd && IsWindow(f.hwnd) && !f.renameBox) SetForegroundWindow(f.hwnd);
}

// The hidden icon shown at a place in the grid while one is dragged (the
// others make room for it, as RenderOverflow draws them).
int ShownHiddenAt(int place) {
    auto const& f = g_flyout;
    std::vector<int> order;
    for (int i = 0; i < (int)f.hidden.size(); i++) order.push_back(i);
    if (f.dragHidden >= 0 && f.dragHidden < (int)order.size() && f.dragTarget >= 0 && f.dragTarget < (int)order.size()) {
        order.erase(order.begin() + f.dragHidden);
        order.insert(order.begin() + f.dragTarget, f.dragHidden);
    }
    return place >= 0 && place < (int)order.size() ? order[place] : -1;
}

// Where a dragged hidden icon would land: a folder tile (into it), the middle
// of another icon held there a moment (a new folder of the two), the open
// folder's way back (out of it), else a slot in the grid, the others making
// room. True if that changed.
bool AimHiddenDrag(POINT pt) {
    auto& f = g_flyout;
    int folder = -1, stack = -1, target = -1, over = -1;
    bool back = false;
    int hit = -1;
    for (auto const& h : f.hits)
        if (PtInRect(&h.rc, pt) && ((h.id >> 16) == kHitFolder || (h.id >> 16) == kHitFolderBack)) hit = h.id;
    // Inside a folder, anywhere above its icons is the way out.
    if (hit < 0 && f.openFolder >= 0 && pt.y >= 0 && pt.y < f.overflowTop) hit = MakeHit(kHitFolderBack, 0);
    if (hit >= 0 && (hit >> 16) == kHitFolder) {
        folder = hit & 0xFFFF;
    } else if (hit >= 0) {
        back = true;
    } else {
        target = HiddenSlotAt(pt);
        int cell = f.overflowCell, columns = OverflowColumns();
        if (target >= 0 && cell > 0 && f.openFolder < 0 && g_settings.overflowFolders) {
            int column = (pt.x - f.overflowLeft) / cell, row = (pt.y - f.overflowTop) / cell;
            int x = (pt.x - f.overflowLeft) % cell, y = (pt.y - f.overflowTop) % cell;
            int under = ShownHiddenAt(row * columns + column - f.folderTiles);
            if (under >= 0 && under != f.dragHidden && x > cell / 4 && x < cell * 3 / 4 && y > cell / 4 &&
                y < cell * 3 / 4) {
                // Passing over icons while moving one makes no folders: only one held over another.
                // Meanwhile the others stay where they are.
                over = under;
                if (under == f.dragStack) {
                    stack = under;
                } else if (under != f.stackWait) {
                    f.stackWait = under;
                    if (f.hwnd) SetTimer(f.hwnd, kTimerStackWait, 450, nullptr);
                }
                target = f.dragTarget;
            }
        }
    }
    if (f.stackWait >= 0 && (over != f.stackWait || stack >= 0)) {
        f.stackWait = -1;
        if (f.hwnd) KillTimer(f.hwnd, kTimerStackWait);
    }
    bool changed = folder != f.dragFolder || stack != f.dragStack || back != f.dragBack || target != f.dragTarget;
    f.dragFolder = folder;
    f.dragStack = stack;
    f.dragBack = back;
    f.dragTarget = target;
    return changed;
}

// The hidden icons flyout's own menu, from a right click off its icons: an icon onto the taskbar, or out
// of both the taskbar and this flyout (and those taken out brought back). Every place is kept where the
// taskbar reads it too.

std::wstring MenuEscape(std::wstring text) {
    for (size_t at = 0; (at = text.find(L'&', at)) != std::wstring::npos; at += 2) text.insert(at, 1, L'&');
    return text;
}

// "Tooltip (program.exe)", or just the program when there's no tooltip.
std::wstring IconMenuName(HiddenIcon const* icon, std::wstring const& key) {
    std::wstring program = key.starts_with(L"plugin:") ? key.substr(7) : key.substr(0, key.find(L'|'));
    std::wstring tip = icon ? icon->tip.substr(0, icon->tip.find_first_of(L"\r\n")) : L"";
    if (tip.size() > 40) tip = tip.substr(0, 37) + L"...";
    std::wstring name = tip.empty() || !_wcsicmp(tip.c_str(), program.c_str()) ? program : tip + L"  (" + program + L")";
    return MenuEscape(icon ? name : name + L", not running");
}

void ShowOverflowMenu(POINT screen) {
    std::vector<HiddenIcon> all;
    RequestHiddenIcons(&all, kRelayAllIcons);
    TrayPlaces places = TrayPlaces::Load();
    auto placeIt = [](std::wstring key, int place) {
        return [key, place] {
            TrayPlaces::Load().Set(key, place);
            LoadOverflowFlyout();
            LayoutFlyout(-1);
            if (g_flyout.hwnd) InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
            TellTaskbars();
        };
    };
    std::vector<MenuItem> toTaskbar, takeOut, bringBack;
    std::vector<std::wstring> inFlyout;
    for (auto const& icon : all) {
        std::wstring key = HiddenIconKey(icon);
        if (icon.owner == g_wnd && icon.id == kIconIdBase + kOverflow) continue;  // (the flyout's own stand-in)
        if (places.For(icon, key) != kPlaceOverflow) continue;
        inFlyout.push_back(key);
        toTaskbar.push_back({IconMenuName(&icon, key), placeIt(key, kPlaceTray)});
        takeOut.push_back({IconMenuName(&icon, key), placeIt(key, kPlaceHidden)});
    }
    for (auto const& [key, place] : places.places) {
        if (place != kPlaceHidden) continue;
        auto running = std::find_if(all.begin(), all.end(), [&](HiddenIcon const& i) { return HiddenIconKey(i) == key; });
        bringBack.push_back({IconMenuName(running != all.end() ? &*running : nullptr, key), placeIt(key, kPlaceOverflow)});
    }
    auto byName = [](MenuItem const& a, MenuItem const& b) { return _wcsicmp(a.label.c_str(), b.label.c_str()) < 0; };
    std::sort(toTaskbar.begin(), toTaskbar.end(), byName);
    std::sort(takeOut.begin(), takeOut.end(), byName);
    std::sort(bringBack.begin(), bringBack.end(), byName);

    std::vector<MenuItem> items;
    if (toTaskbar.empty()) {
        items.push_back({L"No icons here to move", nullptr});
    } else {
        items.push_back({L"Show on the taskbar", nullptr, toTaskbar});
        items.push_back({L"Take out of the taskbar and here", nullptr, takeOut});
        items.push_back({L"Show them all on the taskbar", [inFlyout] {
                             TrayPlaces places = TrayPlaces::Load();
                             for (auto const& key : inFlyout) places.Set(key, kPlaceTray);
                             LoadOverflowFlyout();
                             LayoutFlyout(-1);
                             if (g_flyout.hwnd) InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
                             TellTaskbars();
                         }});
    }
    if (!bringBack.empty()) {
        items.push_back({});
        items.push_back({L"Bring back here", nullptr, bringBack});
    }
    if (g_settings.overflowFolders) {
        // A new folder: picked icons go in (from other folders too), then it can be renamed. Or every icon
        // sorted into folders by what its program is for.
        std::vector<MenuItem> newFolder{{L"Pick its icons", [screen] { NewOverflowFolder(screen, nullptr); }}, {}};
        FolderLayout layout = LoadFolderLayout();
        std::vector<MenuItem> startWith;
        for (auto const& icon : all) {
            std::wstring key = HiddenIconKey(icon);
            if (icon.owner == g_wnd && icon.id == kIconIdBase + kOverflow) continue;
            if (places.For(icon, key) != kPlaceOverflow || PlacedKey(icon).empty()) continue;
            int home = HomeFolder(icon, layout);
            std::wstring label = L"Start with " + IconMenuName(&icon, key);
            if (home >= 0) label += L"  (from " + MenuEscape(layout.folders[home].name) + L")";
            startWith.push_back({label, [screen, icon] { NewOverflowFolder(screen, &icon); }});
        }
        std::sort(startWith.begin(), startWith.end(), byName);
        for (auto& item : startWith) newFolder.push_back(std::move(item));
        if (startWith.empty()) newFolder.pop_back();
        std::vector<MenuItem> autoSort{
            {L"Loose icons (your folders stay)", [] { AutoSortFolders(false); }},
            {L"All icons (folders made again)", [] { AutoSortFolders(true); }},
        };
        if (!LoadTrayList(kTrayFoldersBeforeSortValue).empty()) {
            autoSort.push_back({});
            autoSort.push_back({L"Folders as before auto sort", [] {
                                    UndoAutoSort();
                                    LoadOverflowFlyout();
                                    LayoutFlyout(-1);
                                    if (g_flyout.hwnd) InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
                                }});
        }
        items.push_back({});
        items.push_back({L"New folder in the overflow", nullptr, newFolder});
        items.push_back({L"Auto sort", nullptr, autoSort});
    }
    bool changedHere = !LoadTrayList(kTrayFoldersValue).empty();
    bool keptBefore = !LoadTrayList(kTrayFoldersBeforeValue).empty();
    if (g_settings.overflowFolders && (changedHere || keptBefore)) {
        // Folders changed here (dragged into, made, renamed) back to the ones in the settings, and back again.
        items.push_back({});
        if (changedHere) {
            items.push_back({L"Folders as in the settings", [] {
                                 ForgetFolderLayout();
                                 LoadOverflowFlyout();
                                 LayoutFlyout(-1);
                                 if (g_flyout.hwnd) InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
                             }});
        } else {
            items.push_back({L"Folders as I had them", [] {
                                 BringBackFolderLayout();
                                 LoadOverflowFlyout();
                                 LayoutFlyout(-1);
                                 if (g_flyout.hwnd) InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
                             }});
        }
    }
    g_flyoutMenu = true;
    ShowMenu(items, MAKELPARAM(screen.x, screen.y));
    g_flyoutMenu = false;
    // The flyout stayed open under the menu: it takes the focus back, so a click away still closes it.
    if (g_flyout.hwnd && IsWindow(g_flyout.hwnd)) SetForegroundWindow(g_flyout.hwnd);
}

// An icon dragged from the taskbar's tray onto the flyout: into the overflow,
// in the slot it was dropped on (at the end, off the grid); dropped on a
// folder tile, or while a folder is open, into that folder too.
bool DropOnFlyout(HWND hWnd, POINT screen, std::wstring const& key) {
    auto& f = g_flyout;
    TrayPlaces::Load().Set(key, kPlaceOverflow);
    POINT client = screen;
    ScreenToClient(hWnd, &client);
    int slot = HiddenSlotAt(client);
    int setup = -1;
    for (auto const& h : f.hits)
        if (PtInRect(&h.rc, client) && (h.id >> 16) == kHitFolder && (h.id & 0xFFFF) < (int)f.folders.size())
            setup = f.folders[h.id & 0xFFFF].setup;
    if (setup < 0 && f.openFolder >= 0 && f.openFolder < (int)f.folders.size()) setup = f.folders[f.openFolder].setup;
    if (setup >= 0) {
        std::vector<HiddenIcon> all;
        RequestHiddenIcons(&all, kRelayAllIcons);
        for (auto const& icon : all) {
            if (HiddenIconKey(icon) == key) {
                PutInFolder(icon, setup);  // (shown at once)
                return true;
            }
        }
    }
    LoadOverflowFlyout();
    auto& hidden = g_flyout.hidden;
    for (int i = 0; i < (int)hidden.size(); i++) {
        if (HiddenIconKey(hidden[i]) == key) {
            MoveHiddenIcon(i, slot >= 0 && slot < (int)hidden.size() ? slot : (int)hidden.size() - 1);
            break;
        }
    }
    LayoutFlyout(-1);
    InvalidateRect(hWnd, nullptr, FALSE);
    return true;
}
void ClickHiddenIcon(int index, int button) {
    if (index < 0 || index >= (int)g_flyout.hidden.size()) {
        return;
    }
    HiddenIcon icon = g_flyout.hidden[index];
    if (!icon.plugin.empty()) {
        PluginIconClick(icon, button);
        // Its look and tooltip read again once the plugin has answered (kTimerHoverTip, 7).
        g_pluginItemsRead = 0;
        if (g_flyout.hwnd) SetTimer(g_flyout.hwnd, 7, 400, nullptr);
        return;
    }
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

// A Defender scan from the flyout: Defender's own command line (MpCmdRun) asks its service to scan, with no
// window. The scan is the service's work; this only waits for it, on a thread of its own, so it carries on
// with the flyout closed. Unloading lets go of the wait and leaves the scan to finish. When Windows wants an
// administrator for it, the flyout offers the same scan through the UAC prompt.
HANDLE g_scanThread = nullptr;
HANDLE g_scanStop = nullptr;
bool g_scanNeedsAdmin = false;  // the last try was refused for want of rights

struct ScanWork {
    int type;
    bool admin;
    HANDLE stop;
    HWND notify;
};

std::wstring MpCmdRunPath() {
    WCHAR folder[MAX_PATH] = L"C:\\Program Files";
    ExpandEnvironmentStringsW(L"%ProgramFiles%", folder, ARRAYSIZE(folder));
    return std::wstring(folder) + L"\\Windows Defender\\MpCmdRun.exe";
}

DWORD WINAPI DefenderScanProc(LPVOID parameter) {
    auto* work = (ScanWork*)parameter;
    std::wstring exe = MpCmdRunPath();
    std::wstring arguments = L"-Scan -ScanType " + std::to_wstring(work->type);
    HANDLE process = nullptr;
    if (work->admin) {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        SHELLEXECUTEINFOW info{sizeof(info)};
        info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
        info.lpVerb = L"runas";
        info.lpFile = exe.c_str();
        info.lpParameters = arguments.c_str();
        info.nShow = SW_HIDE;
        if (ShellExecuteExW(&info)) process = info.hProcess;
        CoUninitialize();
    } else {
        std::wstring command = L"\"" + exe + L"\" " + arguments;
        STARTUPINFOW startup{sizeof(startup)};
        PROCESS_INFORMATION info{};
        if (CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                           &startup, &info)) {
            CloseHandle(info.hThread);
            process = info.hProcess;
        }
    }
    DWORD code = (DWORD)-1;  // didn't start (or the UAC prompt was declined)
    bool stopped = false;
    if (process) {
        HANDLE waits[] = {process, work->stop};
        stopped = WaitForMultipleObjects(2, waits, FALSE, INFINITE) != WAIT_OBJECT_0;
        if (!stopped && !GetExitCodeProcess(process, &code)) code = (DWORD)-1;
        CloseHandle(process);
    }
    if (!stopped) PostMessageW(work->notify, WM_APP_SCAN_DONE, work->type, (LPARAM)code);
    delete work;
    return 0;
}

void StartDefenderScan(int type, bool admin) {
    if (g_scan.running || !g_wnd) return;
    if (g_scanThread) {
        WaitForSingleObject(g_scanThread, INFINITE);  // the last one has already said it's done
        CloseHandle(g_scanThread);
        g_scanThread = nullptr;
    }
    if (!g_scanStop) g_scanStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    ResetEvent(g_scanStop);
    auto* work = new ScanWork{type, admin, g_scanStop, g_wnd};
    g_scanThread = CreateThread(nullptr, 0, DefenderScanProc, work, 0, nullptr);
    if (!g_scanThread) {
        delete work;
        return;
    }
    g_scan.running = g_scan.lastType = type;
    g_scan.started = GetTickCount64();
    g_scan.result.clear();
    g_scanNeedsAdmin = false;
    RequestRefresh(1u << kSecurity);
}

// MpCmdRun's word on how it went: 0 nothing found, 2 something found (Windows Security has the details).
void FinishDefenderScan(int type, DWORD code) {
    g_scan.running = 0;
    std::wstring kind = type == 2 ? L"Full scan" : L"Quick scan";
    if (code == 0) {
        g_scan.result = kind + L": no threats";
    } else if (code == 2) {
        g_scan.result = kind + L": threats found";
    } else if (code == (DWORD)-1) {
        g_scan.result = kind + L" didn't start";
    } else if (code == 0x80070005 || code == 5) {
        g_scan.result = kind + L" needs an administrator";
        g_scanNeedsAdmin = true;
    } else {
        WCHAR text[64];
        swprintf_s(text, L"%ls stopped (0x%08X)", kind.c_str(), code);
        g_scan.result = text;
    }
    RequestRefresh(1u << kSecurity);
}

void StopDefenderScanWait() {
    if (g_scanStop) SetEvent(g_scanStop);
    if (g_scanThread) {
        WaitForSingleObject(g_scanThread, 5000);
        CloseHandle(g_scanThread);
        g_scanThread = nullptr;
    }
    if (g_scanStop) {
        CloseHandle(g_scanStop);
        g_scanStop = nullptr;
    }
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
    c.y += c.Px(22);

    // A scan from here: quick or full, or how far the one running has got; and how the last one ended.
    RECT line{c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)};
    if (g_scan.running) {
        ULONGLONG seconds = (GetTickCount64() - g_scan.started) / 1000;
        WCHAR text[64];
        swprintf_s(text, L"%ls SCAN RUNNING  %llu:%02llu", g_scan.running == 2 ? L"FULL" : L"QUICK", seconds / 60,
                   seconds % 60);
        c.Text(f.label, ui::kCoral, line, text, DT_LEFT, c.Px(2));
    } else {
        int x = line.left;
        auto button = [&](PCWSTR text, int index) {
            int id = MakeHit(kHitSecurityScan, index);
            SIZE size = c.Measure(f.label, text, c.Px(2));
            RECT rc{x, line.top, x + size.cx, line.bottom};
            c.Text(f.label, hover == id ? ui::kCoral : ui::kGreen, rc, text, DT_LEFT, c.Px(2));
            hits.push_back({rc, id});
            x = rc.right + c.Px(14);
        };
        if (g_scanNeedsAdmin) {
            button(L"SCAN AS ADMIN", 3);
        } else {
            button(L"QUICK SCAN", 1);
            button(L"FULL SCAN", 2);
        }
    }
    c.y += c.Px(20);
    if (!g_scan.result.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(18)}, g_scan.result);
        c.y += c.Px(18);
    }
    c.y += c.Px(6);

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

// Bluetooth flyout: the radio, with a switch, then the paired devices. An
// audio device can be connected or disconnected from here; anything else
// opens Settings, since Windows has no public way to connect it.

void LoadBluetoothDevices() {
    auto& f = g_flyout;
    f.btDevices = PairedBluetoothDevices();
    FindBluetoothAudio(g_audioEnumerator, f.btDevices);
    f.btRadioOn = g_bluetoothRadio != 0;
}

void ConnectBluetooth(BluetoothDevice const& device, bool connect) {
    auto& f = g_flyout;
    if (device.audioFilters.empty() || !f.btBusy.empty()) {
        return;
    }
    CancelJob(f.job);
    f.btNote.clear();
    f.btBusy = (connect ? L"Connecting to " : L"Disconnecting ") + device.name + L"...";
    std::vector<std::wstring> filters = device.audioFilters;
    GUID container = device.container;
    f.job = StartJob([filters, container, connect] {
        if (!RequestBluetoothAudio(filters, connect)) {
            return std::wstring(L"failed");
        }
        for (int i = 0; i < 40; i++) {
            Sleep(250);
            for (auto const& device : PairedBluetoothDevices()) {
                if (IsEqualGUID(device.container, container) && device.connected == connect) {
                    return std::wstring(L"ok");
                }
            }
        }
        return std::wstring(L"timeout");
    });
}

void ToggleBluetoothRadio() {
    auto& f = g_flyout;
    if (!f.btBusy.empty()) {
        return;
    }
    CancelJob(f.job);
    f.btNote.clear();
    bool on = !f.btRadioOn;
    f.btBusy = on ? L"Turning on..." : L"Turning off...";
    f.job = StartJob([on] { return SetBluetoothRadio(on); });
}

// A request finished: what it says, if it didn't work.
void FinishBluetoothJob(std::wstring const& result) {
    auto& f = g_flyout;
    bool connecting = f.btBusy.starts_with(L"Connecting");
    bool radio = f.btBusy.starts_with(L"Turning");
    f.btBusy.clear();
    if (result == L"denied") {
        f.btNote = L"Windows didn't allow the change";
    } else if (result == L"failed") {
        f.btNote = radio ? L"Couldn't switch Bluetooth" : L"The device didn't answer";
    } else if (result == L"timeout") {
        f.btNote = connecting ? L"Not connected yet, is it in range?" : L"Still connected";
    }
    LoadBluetoothDevices();
    RequestRefresh((1u << kBluetooth) | (1u << kVolume));
}

void RenderBluetooth(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    bool busy = !f.btBusy.empty();
    bool anyConnected = std::any_of(f.btDevices.begin(), f.btDevices.end(),
                                    [](BluetoothDevice const& d) { return d.connected; });

    c.y = c.Px(15);
    c.Label(L"Bluetooth", ui::kSage);
    c.y += c.Px(6);
    c.Art(!f.btRadioOn ? "bluetooth-off" : anyConnected ? "bluetooth-connected" : "bluetooth", c.Px(15), c.y,
          c.Px(26));
    std::wstring action = f.btRadioOn ? L"TURN OFF" : L"TURN ON";
    SIZE size = c.Measure(f.label, action, c.Px(2));
    RECT button{c.width - c.Px(15) - size.cx - c.Px(24), c.y + c.Px(2), c.width - c.Px(15), c.y + c.Px(24)};
    std::wstring status = busy ? f.btBusy : !f.btNote.empty() ? f.btNote : f.btRadioOn ? L"On" : L"Off";
    c.Text(f.body, !f.btNote.empty() && !busy ? ui::kCoral : ui::kInk,
           {c.Px(51), c.y, button.left - c.Px(8), c.y + c.Px(26)}, status);
    int toggleId = MakeHit(kHitBtToggle, 0);
    bool hot = !busy && hover == toggleId;
    c.Fill(button, hot ? ui::kHover : ui::kGround);
    c.Frame(button, hot ? ui::kGreen : ui::kEdge);
    c.CenteredText(f.label, busy ? ui::kSageAlt : hot ? ui::kGreen : ui::kSage, button, action, c.Px(2));
    if (!busy) {
        hits.push_back({button, toggleId});
    }
    c.y += c.Px(32);

    c.Divider();
    c.Label(L"Devices", ui::kGreen);
    c.y += c.Px(4);
    auto note = [&](PCWSTR text) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)}, text);
        c.y += c.Px(24);
    };
    if (!f.btRadioOn) {
        note(L"Bluetooth is off");
    } else if (f.btDevices.empty()) {
        note(L"No paired devices");
    }
    for (int i = 0; f.btRadioOn && i < (int)f.btDevices.size(); i++) {
        auto const& device = f.btDevices[i];
        bool selected = device.name == f.btSelected;
        int rowHeight = c.Px(selected ? 66 : 38);
        RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + rowHeight};
        int rowId = MakeHit(kHitBtRow, i);
        if (selected) {
            c.Fill(row, ui::kRaised);
            c.Frame(row, ui::kEdge);
        } else if (hover == rowId) {
            c.Fill(row, ui::kHover);
        }
        hits.push_back({row, rowId});

        int artX = c.width - c.Px(15 + 26);
        c.Art(device.connected ? "bluetooth-connected" : "bluetooth", artX, c.y + c.Px(6), c.Px(26));
        c.Text(f.body, ui::kInk, {c.Px(15), c.y + c.Px(2), artX - c.Px(8), c.y + c.Px(20)}, device.name);
        std::wstring detail = device.connected ? L"Connected" : L"Not connected";
        if (device.connected && device.battery >= 0) {
            detail += L" ... " + std::to_wstring(device.battery) + L"%";
        }
        c.Text(f.caption, device.connected ? ui::kCoral : ui::kSage,
               {c.Px(15), c.y + c.Px(20), artX - c.Px(8), c.y + c.Px(34)}, detail);

        if (selected) {
            bool audio = !device.audioFilters.empty();
            std::wstring label = !audio ? L"SETTINGS" : device.connected ? L"DISCONNECT" : L"CONNECT";
            SIZE labelSize = c.Measure(f.label, label, c.Px(2));
            RECT action{c.width - c.Px(15) - labelSize.cx - c.Px(24), c.y + c.Px(38), c.width - c.Px(15),
                        c.y + c.Px(60)};
            int actionId = MakeHit(kHitBtAction, i);
            bool actionHot = !busy && hover == actionId;
            c.Fill(action, actionHot ? ui::kHover : ui::kGround);
            c.Frame(action, actionHot ? ui::kGreen : ui::kEdge);
            c.CenteredText(f.label, busy ? ui::kSageAlt : actionHot ? ui::kGreen : ui::kSage, action, label,
                           c.Px(2));
            if (!busy) {
                hits.push_back({action, actionId});
            }
        }
        c.y += rowHeight + c.s;
    }

    RenderBluetoothBatteries(c);

    c.Divider();
    c.Link(L"Add a device", 0, hits, hover);
    c.Link(L"Bluetooth settings", 1, hits, hover);
    c.y += c.Px(10);
}

// The mixer's Bluetooth section: each paired audio device, with Connect or
// Disconnect.
void RenderMixerBluetooth(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    bool any = std::any_of(f.btDevices.begin(), f.btDevices.end(),
                           [](BluetoothDevice const& d) { return !d.audioFilters.empty(); });
    if (!any || !f.btRadioOn) {
        return;
    }
    bool busy = !f.btBusy.empty();
    c.Divider();
    c.Label(L"Bluetooth", ui::kGreen);
    c.y += c.Px(4);
    if (busy || !f.btNote.empty()) {
        c.Text(f.caption, busy ? ui::kSageAlt : ui::kCoral, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               busy ? f.btBusy : f.btNote);
        c.y += c.Px(22);
    }
    for (int i = 0; i < (int)f.btDevices.size(); i++) {
        auto const& device = f.btDevices[i];
        if (device.audioFilters.empty()) {
            continue;
        }
        int id = MakeHit(kHitMixerBt, i);
        std::wstring action = device.connected ? L"Disconnect" : L"Connect";
        SIZE size = c.Measure(f.caption, action);
        RECT link{c.width - c.Px(15) - size.cx, c.y, c.width - c.Px(15), c.y + c.Px(26)};
        c.Art(device.connected ? "bluetooth-connected" : "bluetooth", c.Px(15), c.y + c.Px(5), c.Px(16));
        c.Text(f.body, device.connected ? ui::kInk : ui::kSage,
               {c.Px(42), c.y, link.left - c.Px(10), c.y + c.Px(26)}, device.name);
        c.Text(f.caption, busy ? ui::kSageAlt : hover == id ? ui::kCoral : ui::kGreen, link, action);
        if (!busy) {
            hits.push_back({link, id});
        }
        c.y += c.Px(26);
    }
}

// Device batteries in the flyouts: their own flyout, opened from a device's
// icon, and sections in the mixer (headsets) and the Bluetooth flyout (what
// Bluetooth itself doesn't list).

void LoadBatteriesFlyout() {
    g_flyout.batteries = g_batteries;
}

// One device: its halo, name, and level. Large rows in the devices flyout,
// compact ones in the mixer and Bluetooth flyouts.
void BatteryRow(Canvas& c, bat::Device const& d, bool large) {
    auto& f = g_flyout;
    std::wstring const& name = d.name;
    std::wstring detail = BatteryDetail(d);
    COLORREF detailColor = d.charging ? ui::kGreen : d.online ? ui::kCoral : ui::kSageAlt;
    if (large) {
        int artX = c.width - c.Px(15 + 26);
        c.Art(BatteryArtId(d), artX, c.y + c.Px(6), c.Px(26));
        c.Text(f.body, d.online ? ui::kInk : ui::kSage, {c.Px(15), c.y + c.Px(2), artX - c.Px(8), c.y + c.Px(20)},
               name);
        c.Text(f.caption, detailColor, {c.Px(15), c.y + c.Px(20), artX - c.Px(8), c.y + c.Px(34)}, detail);
        c.y += c.Px(38) + c.s;
        return;
    }
    SIZE size = c.Measure(f.caption, detail);
    RECT right{c.width - c.Px(15) - size.cx, c.y, c.width - c.Px(15), c.y + c.Px(26)};
    c.Art(BatteryArtId(d), c.Px(15), c.y + c.Px(5), c.Px(16));
    c.Text(f.body, d.online ? ui::kInk : ui::kSage, {c.Px(42), c.y, right.left - c.Px(10), c.y + c.Px(26)}, name);
    c.Text(f.caption, detailColor, right, detail);
    c.y += c.Px(26);
}

void RenderDevices(Canvas& c, std::vector<Hit>& hits, int hover) {
    auto& f = g_flyout;
    c.y = c.Px(15);
    c.Label(L"Device batteries", ui::kSage);
    c.y += c.Px(6);
    if (f.batteries.empty()) {
        c.Text(f.caption, ui::kSageAlt, {c.Px(15), c.y, c.width - c.Px(15), c.y + c.Px(20)},
               L"No devices with a battery reading");
        c.y += c.Px(24);
    }
    for (auto const& d : f.batteries) {
        BatteryRow(c, d, true);
    }
    c.Divider();
    c.Link(L"Check now", 0, hits, hover);
    c.Link(L"Bluetooth settings", 1, hits, hover);
    c.y += c.Px(10);
}

void RenderBatterySection(Canvas& c, PCWSTR label, bool (*include)(bat::Device const&)) {
    if (!g_batterySettings.inFlyouts) {
        return;
    }
    auto& list = g_flyout.batteries;
    if (std::none_of(list.begin(), list.end(), include)) {
        return;
    }
    c.Divider();
    c.Label(label, ui::kGreen);
    c.y += c.Px(4);
    for (auto const& d : list) {
        if (include(d)) {
            BatteryRow(c, d, false);
        }
    }
}

// The mixer shows headsets; the Bluetooth flyout shows what its own list
// doesn't already carry a level for.
void RenderMixerBatteries(Canvas& c) {
    RenderBatterySection(c, L"Headset battery",
                         [](bat::Device const& d) { return BatteryKind(d) == "headset"; });
}

void RenderBluetoothBatteries(Canvas& c) {
    RenderBatterySection(c, L"Device batteries",
                         [](bat::Device const& d) { return d.source != "bluetooth"; });
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
constexpr UINT_PTR kTimerScanTick = 6;  // the security flyout's running-scan clock
constexpr UINT_PTR kTimerHoverTip = 7;  // a hidden icon's tooltip, read again after its program heard of the pointer
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
    RenderMixerBatteries(c);
    RenderMixerBluetooth(c, hits, hover);

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
    // The phone: its name, battery and connection (its notifications are in the list already).
    f.phone = g_settings.notificationPhone ? ReadPhoneInfo(0, TourneHoldsPhone()) : PhoneInfo{};
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

////////////////////////////////////////////////////////////////////////////////
// Flyouts and pop-ups opening and closing on the motion engine's lines: popups
// (timed by fadePopupsIn and fadePopupsOut for the fades) for the flyouts and
// the new event window, layersIn and layersOut (with fadeLayersIn and
// fadeLayersOut) for the notification pop-ups. Opening, the window itself is
// layered while it fades and slides in. Closing, a still of it fades and
// slides out on its own while the window is already gone, so every close path
// stays as it was. With none of these lines set, all of it is instant, as before.

struct MotionLines {
    motion::Anim popups, fadePopupsIn, fadePopupsOut, layersIn, layersOut, fadeLayersIn, fadeLayersOut;
};
MotionLines g_motion;  // (the flyouts' thread only)

// Read with the part's settings: as it starts and when they change.
void LoadMotion() {
    motion::Forget();
    g_motion.popups = motion::Get(L"popups");
    g_motion.fadePopupsIn = motion::Get(L"fadePopupsIn");
    g_motion.fadePopupsOut = motion::Get(L"fadePopupsOut");
    g_motion.layersIn = motion::Get(L"layersIn");
    g_motion.layersOut = motion::Get(L"layersOut");
    g_motion.fadeLayersIn = motion::Get(L"fadeLayersIn");
    g_motion.fadeLayersOut = motion::Get(L"fadeLayersOut");
}

constexpr UINT kMotionFrameMs = 10;
constexpr double kMotionMaxMs = 1500;  // (as the menus have it)

// How a window opens or closes: a slide between where it's placed and an offset from there, and a fade.
struct WindowMotion {
    motion::Anim slide, fade;  // (a part that doesn't run: on is false)
    POINT offset{};            // the slide's far end, from where the window is placed
    DWORD start = 0;
};

// The slide a style asks for: none, from the side, from the top or bottom, or a small one.
enum MotionSlide { kSlideNone, kSlideSide, kSlideVert, kSlideSmall };

// A window opening or closing as the lines have it: the style from the move leaf (popups, layersIn,
// layersOut), the fade timed by the fade leaf when that's set, else by the move leaf. "slide" and
// "slidevert" don't fade unless fadeAlways; a style not known is the fallback. False when nothing runs:
// neither leaf set, the move leaf off, or every part off.
bool PlanMotion(motion::Anim const& move, motion::Anim const& fade, PCWSTR fallback, bool fadeAlways, WindowMotion* out,
                MotionSlide* slide) {
    *out = {};
    *slide = kSlideNone;
    if ((!move.set && !fade.set) || (move.set && !move.on)) return false;
    std::wstring style = move.set ? motion::StyleWord(move) : L"fade";
    static constexpr PCWSTR kKnown[] = {L"slide", L"slidevert", L"fade", L"slidefade",
                                        L"slidefadevert", L"popin", L"gnomed"};
    if (std::none_of(std::begin(kKnown), std::end(kKnown), [&](PCWSTR known) { return style == known; }))
        style = fallback;
    if (move.set) {
        if (style == L"slide" || style == L"slidefade") *slide = kSlideSide;
        else if (style == L"slidevert" || style == L"slidefadevert") *slide = kSlideVert;
        else if (style == L"popin" || style == L"gnomed") *slide = kSlideSmall;
    }
    out->slide = move;
    out->slide.on = *slide != kSlideNone && motion::Ms(move) > 0;
    if (!out->slide.on) *slide = kSlideNone;
    out->fade = fade.set ? fade : move;
    bool fades = fadeAlways || (style != L"slide" && style != L"slidevert");
    out->fade.on = out->fade.on && fades && motion::Ms(out->fade) > 0;
    out->start = GetTickCount();
    return out->slide.on || out->fade.on;
}

// A part's eased progress (0 to 1, past 1 on an overshoot); clears *done while it still runs.
double MotionPart(motion::Anim const& a, DWORD start, bool* done) {
    double ms = a.on ? std::min(motion::Ms(a), kMotionMaxMs) : 0;
    double p = ms > 0 ? (GetTickCount() - start) / ms : 1;
    if (p >= 1) return 1;
    *done = false;
    return motion::Ease(a, p);
}

// Where a window opening (or closing) is now: its alpha, out of full, and its offset from where it's placed.
// True when it's over; the alpha and offset are then those of its end.
bool MotionAt(WindowMotion const& m, bool closing, BYTE full, BYTE* alpha, POINT* offset) {
    bool done = true;
    double s = MotionPart(m.slide, m.start, &done), f = MotionPart(m.fade, m.start, &done);
    double away = closing ? s : 1 - s;  // (of the offset)
    double shown = !m.fade.on ? 1 : closing ? 1 - f : f;
    *offset = {(LONG)std::lround(m.offset.x * away), (LONG)std::lround(m.offset.y * away)};
    *alpha = (BYTE)std::clamp((int)std::lround(full * shown), 0, 255);
    return done;
}

// A window about to show, made ready to open with m: layered, at its first alpha, its timer started.
void BeginOpening(HWND wnd, WindowMotion const& m) {
    SetWindowLongPtrW(wnd, GWL_EXSTYLE, GetWindowLongPtrW(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(wnd, 0, m.fade.on ? 0 : 255, LWA_ALPHA);
    SetTimer(wnd, kTimerMotion, kMotionFrameMs, nullptr);
}

// One frame of a window fading in (no slide); at the end it's fully shown and its timer stops.
void FadeInStep(HWND wnd, WindowMotion const& m) {
    BYTE alpha;
    POINT offset;
    bool done = MotionAt(m, false, 255, &alpha, &offset);
    SetLayeredWindowAttributes(wnd, 0, alpha, LWA_ALPHA);
    if (done) KillTimer(wnd, kTimerMotion);
}

// While a window slides in from past its screen's edge, only the part on that screen shows.
void ClipToScreen(HWND wnd, RECT rc, RECT const& screen) {
    RECT shown;
    if (!IntersectRect(&shown, &rc, &screen)) shown = {rc.left, rc.top, rc.left, rc.top};
    OffsetRect(&shown, -rc.left, -rc.top);
    SetWindowRgn(wnd, CreateRectRgnIndirect(&shown), TRUE);  // (the window owns the region now)
}

// A still of a window that just closed, fading and sliding out on its own. Clicks go through it.
constexpr WCHAR kStillClass[] = L"TourneMotionStill_" WH_MOD_ID;

struct MotionStill {
    HDC dc = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ old = nullptr;
    RECT from{};       // where the window was
    RECT screen{};     // what it's kept to, when clipped
    bool clipped = false;
    BYTE alpha = 255;  // the window's as it closed
    WindowMotion motion;
};
std::vector<HWND> g_stills;

void FreeStill(MotionStill* s) {
    if (s->dc && s->old) SelectObject(s->dc, s->old);
    if (s->dc) DeleteDC(s->dc);
    if (s->bitmap) DeleteObject(s->bitmap);
    delete s;
}

// One frame; false when it's over (gone, or slid past its screen's edge).
bool StillStep(HWND wnd, MotionStill const& s) {
    BYTE alpha;
    POINT offset;
    if (MotionAt(s.motion, true, s.alpha, &alpha, &offset)) return false;
    RECT rc = s.from;
    OffsetRect(&rc, offset.x, offset.y);
    RECT shown = rc;
    if (s.clipped && !IntersectRect(&shown, &rc, &s.screen)) return false;
    POINT at{shown.left, shown.top}, from{shown.left - rc.left, shown.top - rc.top};
    SIZE size{shown.right - shown.left, shown.bottom - shown.top};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, alpha, 0};
    return UpdateLayeredWindow(wnd, nullptr, &at, &size, s.dc, &from, 0, &blend, ULW_ALPHA);
}

LRESULT CALLBACK StillProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* s = (MotionStill*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
    switch (msg) {
        case WM_TIMER:
            if (wParam == kTimerMotion && (!s || !StillStep(wnd, *s))) DestroyWindow(wnd);
            return 0;
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_NCDESTROY:
            KillTimer(wnd, kTimerMotion);
            std::erase(g_stills, wnd);
            if (s) {
                SetWindowLongPtrW(wnd, GWLP_USERDATA, 0);
                FreeStill(s);
            }
            break;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// A still of the window "of", which is about to close, drawn by paint at its alpha now, playing m as it
// closes (kept to screen when that's given). Nothing when the window isn't showing.
void StartStill(HWND of, BYTE alpha, WindowMotion const& m, RECT const* screen,
                std::function<void(HDC, RECT const&)> const& paint) {
    RECT rc;
    if (!of || !IsWindowVisible(of) || !GetWindowRect(of, &rc) || rc.right <= rc.left || rc.bottom <= rc.top) return;
    int width = rc.right - rc.left, height = rc.bottom - rc.top;
    auto* s = new MotionStill{};
    HDC screenDc = GetDC(nullptr);
    s->dc = CreateCompatibleDC(screenDc);
    s->bitmap = CreateCompatibleBitmap(screenDc, width, height);
    ReleaseDC(nullptr, screenDc);
    if (!s->dc || !s->bitmap) {
        FreeStill(s);
        return;
    }
    s->old = SelectObject(s->dc, s->bitmap);
    paint(s->dc, RECT{0, 0, width, height});
    s->from = rc;
    s->clipped = screen != nullptr;
    if (screen) s->screen = *screen;
    s->alpha = alpha;
    s->motion = m;
    s->motion.start = GetTickCount();
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&StillProc, &instance);
    HWND wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
                               kStillClass, L"", WS_POPUP, rc.left, rc.top, width, height, nullptr, nullptr, instance,
                               nullptr);
    if (!wnd) {
        FreeStill(s);
        return;
    }
    SetWindowLongPtrW(wnd, GWLP_USERDATA, (LONG_PTR)s);
    g_stills.push_back(wnd);
    DWORD corner = 1;  // (square, as the windows it stands for)
    DwmSetWindowAttribute(wnd, 33, &corner, sizeof(corner));
    if (!StillStep(wnd, *s)) {
        DestroyWindow(wnd);
        return;
    }
    ShowWindow(wnd, SW_SHOWNOACTIVATE);
    SetTimer(wnd, kTimerMotion, kMotionFrameMs, nullptr);
}

// Every still gone at once (the part stopping).
void EndStills() {
    for (HWND wnd : std::vector<HWND>(g_stills)) DestroyWindow(wnd);
    g_stills.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Notification pop-ups: where Windows' own don't run (Explorer starts the host
// that draws them), one in the flyouts' style as each notification arrives,
// stacked over the corner by the taskbar. Its buttons do what the notification
// center's do; it goes after a few seconds, or stays while the pointer is on it.

constexpr WCHAR kPopupClass[] = L"TourneNotifyPopup_" WH_MOD_ID;
constexpr UINT_PTR kTimerPopupGone = 1;
constexpr int kMaxPopups = 3;
constexpr int kPopupClose = -2, kPopupBody = -1;  // (hit ids; 0 up are its buttons)

struct Popup {
    HWND wnd = nullptr;
    NotifyItem item;
    std::wstring appName;
    std::string design;
    int scale = 1;
    int hover = INT_MIN;
    bool tracking = false;
    std::vector<std::pair<RECT, int>> hits;
    POINT at{};          // where PlacePopups puts it
    SIZE size{};
    RECT screen{};       // the screen it's on
    bool top = false;    // stacked down from the top of it
    bool opening = false;  // sliding or fading in (its motion timer runs)
    WindowMotion in;
    MotionSlide slideIn = kSlideNone;
    BYTE alpha = 255;
};

std::vector<Popup*> g_popups;
ULONGLONG g_popupSeen = 0;  // the newest arrival already shown, or there when watching began
HFONT g_popupFonts[3] = {};  // title, body, label
int g_popupFontScale = 0;

// Windows' own pop-ups are drawn when Explorer is the shell: it has a taskbar then (hidden under
// Tourne's own tray, in shell mode with Windows apps on).
bool WindowsShowsPopups() {
    for (HWND tray = nullptr; (tray = FindWindowExW(nullptr, tray, L"Shell_TrayWnd", nullptr));) {
        DWORD pid = 0;
        if (!GetWindowThreadProcessId(tray, &pid)) continue;
        bool explorer = false;
        if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
            wchar_t path[MAX_PATH];
            DWORD size = MAX_PATH;
            if (QueryFullProcessImageNameW(process, 0, path, &size)) {
                PCWSTR name = wcsrchr(path, L'\\');
                explorer = !_wcsicmp(name ? name + 1 : path, L"explorer.exe");
            }
            CloseHandle(process);
        }
        if (explorer) return true;
    }
    return false;
}

void PopupFonts(int scale) {
    if (g_popupFontScale == scale) return;
    for (HFONT& f : g_popupFonts)
        if (f) DeleteObject(f), f = nullptr;
    g_popupFonts[0] = MakeFont({L"ProggyClean Nerd Font Mono", L"ProggyCleanTT", L"CozetteVector"}, 16 * scale);
    g_popupFonts[1] = MakeFont({L"CozetteVector", L"scientifica"}, 13 * scale);
    g_popupFonts[2] = MakeFont({L"scientifica", L"CozetteVector"}, 11 * scale);
    g_popupFontScale = scale;
}

// Lays out (paint off) or draws one pop-up; its height.
int RenderPopup(HDC dc, bool paint, Popup& p, int width) {
    Canvas c{dc, paint, p.scale, width};
    HFONT title = g_popupFonts[0], body = g_popupFonts[1], label = g_popupFonts[2];
    p.hits.clear();
    int left = c.Px(40), right = width - c.Px(12);
    c.y = c.Px(10);
    c.Art(p.design, c.Px(14), c.y + c.Px(1), c.Px(16));
    std::wstring name = p.appName;
    CharUpperBuffW(name.data(), (DWORD)name.size());
    c.Text(label, ui::kSage, {left, c.y, right - c.Px(24), c.y + c.Px(18)}, name, DT_LEFT, c.Px(2));
    RECT close{right - c.Px(20), c.y, right, c.y + c.Px(18)};
    c.CenteredText(body, p.hover == kPopupClose ? ui::kCoral : ui::kSageAlt, close, L"x", 0);
    p.hits.push_back({close, kPopupClose});
    c.y += c.Px(22);
    c.Text(title, ui::kInk, {left, c.y, right, c.y + c.Px(20)}, p.item.title.empty() ? p.appName : p.item.title);
    c.y += c.Px(20);
    int lines = BlockLines(c, body, right - left, p.item.body, 3);
    TextBlock(c, body, ui::kSage, {left, c.y, right, c.y + lines * c.Px(16)}, p.item.body);
    c.y += lines * c.Px(16);
    int x = left;
    bool buttons = false;
    for (int k = 0; k < (int)p.item.actions.size(); k++) {
        SIZE size = c.Measure(body, p.item.actions[k].label);
        RECT button{x, c.y + c.Px(6), x + size.cx + c.Px(14), c.y + c.Px(26)};
        if (button.right > right) break;
        c.Frame(button, p.hover == k ? ui::kCoral : ui::kEdge);
        c.CenteredText(body, p.hover == k ? ui::kCoral : ui::kGreen, button, p.item.actions[k].label, 0);
        p.hits.push_back({button, k});
        x = button.right + c.Px(8);
        buttons = true;
    }
    if (buttons) c.y += c.Px(26);
    c.y += c.Px(12);
    p.hits.push_back({{0, 0, width, c.y}, kPopupBody});
    return c.y;
}

// The Tourne taskbar's bar on a screen (in this process: Tourne Desktop, or Explorer with the suite).
bool OwnBarOn(HMONITOR monitor, RECT* out) {
    struct Find {
        DWORD pid;
        HMONITOR monitor;
        RECT rc;
        bool found;
    } find{GetCurrentProcessId(), monitor, {}, false};
    EnumWindows(
        [](HWND h, LPARAM param) -> BOOL {
            auto* f = (Find*)param;
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            WCHAR cls[64] = L"";
            if (pid != f->pid || !IsWindowVisible(h) || !GetClassNameW(h, cls, ARRAYSIZE(cls)) || wcsncmp(cls, L"TourneTaskbar", 13))
                return TRUE;
            RECT rc;
            if (!GetWindowRect(h, &rc) || MonitorFromRect(&rc, MONITOR_DEFAULTTONULL) != f->monitor) return TRUE;
            f->rc = rc;
            f->found = true;
            return FALSE;
        },
        (LPARAM)&find);
    if (find.found) *out = find.rc;
    return find.found;
}

// The pop-up's look, into dc (the size of the window).
void PaintPopup(HDC dc, RECT const& client, Popup& p) {
    SetDCBrushColor(dc, ui::kGround);
    FillRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
    if (p.hover == kPopupBody) {
        RECT inner{p.scale, p.scale, client.right - p.scale, client.bottom - p.scale};
        SetDCBrushColor(dc, ui::kHover);
        FillRect(dc, &inner, (HBRUSH)GetStockObject(DC_BRUSH));
    }
    RenderPopup(dc, true, p, client.right);
    SetDCBrushColor(dc, p.hover == INT_MIN ? ui::kEdge : ui::kGreen);
    FrameRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
}

// A pop-up's slide, from (or out to) the edge of the screen it sits at: past its right edge, or past its top
// or bottom (the stack's end); a small one from the right.
POINT PopupSlide(Popup const& p, MotionSlide slide, POINT from) {
    switch (slide) {
        case kSlideSide: return {p.screen.right - from.x, 0};
        case kSlideVert: return {0, p.top ? p.screen.top - (from.y + p.size.cy) : p.screen.bottom - from.y};
        case kSlideSmall: return {8 * p.scale, 0};
        default: return {0, 0};
    }
}

// Where a pop-up opening is now (pos), its alpha and what shows of it set; true when it's over.
bool PopupOpeningAt(Popup& p, POINT* pos) {
    p.in.offset = PopupSlide(p, p.slideIn, p.at);
    POINT offset;
    bool done = MotionAt(p.in, false, 255, &p.alpha, &offset);
    *pos = {p.at.x + offset.x, p.at.y + offset.y};
    SetLayeredWindowAttributes(p.wnd, 0, p.alpha, LWA_ALPHA);
    if (p.slideIn == kSlideSide || p.slideIn == kSlideVert) {
        if (done) SetWindowRgn(p.wnd, nullptr, TRUE);
        else ClipToScreen(p.wnd, {pos->x, pos->y, pos->x + p.size.cx, pos->y + p.size.cy}, p.screen);
    }
    return done;
}

// With the layersIn line (or fadeLayersIn), a new pop-up slides in from its screen's edge and fades in;
// set up before PlacePopups first shows it.
void StartPopupOpening(Popup& p) {
    if (!PlanMotion(g_motion.layersIn, g_motion.fadeLayersIn, L"slidefade", false, &p.in, &p.slideIn)) return;
    p.opening = true;
    BeginOpening(p.wnd, p.in);
}

// One frame of it (its motion timer).
void PopupOpeningStep(Popup& p) {
    POINT pos;
    bool done = PopupOpeningAt(p, &pos);
    SetWindowPos(p.wnd, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    if (done) {
        KillTimer(p.wnd, kTimerMotion);
        p.opening = false;
    }
}

// With the layersOut line (or fadeLayersOut), a closing pop-up's still slides out to its screen's edge and
// fades, from where it is now.
void PopupStill(Popup& p) {
    WindowMotion m;
    MotionSlide slide;
    RECT rc;
    if (!PlanMotion(g_motion.layersOut, g_motion.fadeLayersOut, L"slidefade", false, &m, &slide) ||
        !GetWindowRect(p.wnd, &rc))
        return;
    m.offset = PopupSlide(p, slide, {rc.left, rc.top});
    bool clipped = slide == kSlideSide || slide == kSlideVert;
    StartStill(p.wnd, p.alpha, m, clipped ? &p.screen : nullptr,
               [&p](HDC dc, RECT const& client) { PaintPopup(dc, client, p); });
}

// Stacked up from the corner by the taskbar, newest nearest it.
void PlacePopups() {
    MONITORINFO mi{sizeof(mi)};
    HMONITOR primary = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfoW(primary, &mi);
    RECT work = mi.rcWork;
    bool top = work.top > mi.rcMonitor.top && work.bottom == mi.rcMonitor.bottom;  // (a bar at the top)
    // Clear of Tourne's bar too, which doesn't take the work area from windows.
    if (RECT bar; OwnBarOn(primary, &bar) && bar.right - bar.left > bar.bottom - bar.top) {
        top = bar.top < (mi.rcMonitor.top + mi.rcMonitor.bottom) / 2;
        if (top) work.top = std::max(work.top, bar.bottom);
        else work.bottom = std::min(work.bottom, bar.top);
    }
    int y = top ? work.top : work.bottom;
    for (auto it = g_popups.rbegin(); it != g_popups.rend(); ++it) {
        Popup& p = **it;
        int gap = 12 * p.scale, width = 340 * p.scale;
        HDC dc = GetDC(p.wnd);
        int height = RenderPopup(dc, false, p, width);
        ReleaseDC(p.wnd, dc);
        int at = top ? y + gap : y - gap - height;
        p.at = {work.right - gap - width, at};
        p.size = {width, height};
        p.screen = mi.rcMonitor;
        p.top = top;
        POINT pos = p.at;
        if (p.opening) PopupOpeningAt(p, &pos);  // (still on its way in: there, its look set before it shows)
        SetWindowPos(p.wnd, HWND_TOPMOST, pos.x, pos.y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        y = top ? at + height : at;
        InvalidateRect(p.wnd, nullptr, FALSE);
    }
}

void ClosePopup(Popup* p) {
    std::erase(g_popups, p);
    if (p->wnd) {
        PopupStill(*p);  // (with the layersOut line, a still of it goes out; the window goes now, as before)
        SetWindowLongPtrW(p->wnd, GWLP_USERDATA, 0);
        DestroyWindow(p->wnd);
    }
    delete p;
    PlacePopups();
}

void DropNotifyItem(NotifyItem const* item);

// The same notification (in the centre's list, or a pop-up of it).
bool SameNotify(NotifyItem const& a, NotifyItem const& b) {
    return a.aumid == b.aumid && a.tag == b.tag && a.group == b.group && a.arrival == b.arrival;
}

// Gone from wherever it shows here: the centre's list (open or not) and its pop-up.
void ForgetNotify(NotifyItem const& item, bool wholeApp) {
    auto& f = g_flyout;
    for (auto app = f.notifyApps.begin(); app != f.notifyApps.end(); ++app) {
        if (app->aumid != item.aumid) continue;
        if (wholeApp) {
            f.notifyApps.erase(app);
            break;
        }
        for (auto const& other : app->items)
            if (SameNotify(other, item)) {
                DropNotifyItem(&other);
                break;
            }
        break;
    }
    std::vector<Popup*> closing;
    for (Popup* p : g_popups)
        if (wholeApp ? p->item.aumid == item.aumid : SameNotify(p->item, item)) closing.push_back(p);
    for (Popup* p : closing) ClosePopup(p);
    if (f.hwnd && IsWindow(f.hwnd) && f.kind == kFlyoutNotify) {
        LayoutFlyout(-1);
        InvalidateRect(f.hwnd, nullptr, FALSE);
    }
}

// Cleared (a middle click, or Clear): from Windows' list, and for a phone's, kept cleared (phone-link.cpp).
void ClearNotify(NotifyItem const& item) {
    RemoveNotification(item);
    ForgetNotify(item, false);
}

void CopyNotifyText(NotifyItem const& item) {
    std::wstring text = item.title;
    if (!item.body.empty()) text += (text.empty() ? L"" : L"\r\n") + item.body;
    if (text.empty() || !OpenClipboard(nullptr)) return;
    EmptyClipboard();
    if (HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t))) {
        memcpy(GlobalLock(mem), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
        GlobalUnlock(mem);
        if (!SetClipboardData(CF_UNICODETEXT, mem)) GlobalFree(mem);
    }
    CloseClipboard();
}

// Its app: a phone app's place on this PC (Discord...), else the app's window brought up, else the app started.
void OpenNotifyApp(NotifyItem const& item) {
    if (item.aumid.starts_with(L"Tourne.Phone.")) {
        std::wstring open = PhoneAppOnPc(item.aumid.substr(13), AppNameFor(item.aumid));
        if (!open.empty()) ShellExecuteW(nullptr, nullptr, open.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        else if (HWND start = FindWindowW(L"TourneStartMenu_" WH_MOD_ID, nullptr))
            PostMessageW(start, RegisterWindowMessageW(L"TournePhoneOpen"), 0, 0);
        return;
    }
    if (HWND w = AppWindow(item.aumid)) ShellBringToFront(w);
    else LaunchApp(item.aumid, L"");
}

// Off in Windows' own settings for it (Settings, Notifications agrees; Tourne Studio's list too).
void TurnOffNotifyApp(std::wstring const& aumid) {
    std::wstring key = L"Software\\Microsoft\\Windows\\CurrentVersion\\Notifications\\Settings\\" + aumid;
    DWORD off = 0;
    RegSetKeyValueW(HKEY_CURRENT_USER, key.c_str(), L"Enabled", REG_DWORD, &off, sizeof(off));
}

void OpenNotifySettings() {
#ifdef TD_PREFIX
    td::OpenStudio(L"notify", L"");
#else
    OpenUri(L"ms-settings:notifications");
#endif
}

// A notification's right-click menu (in the centre or on its pop-up).
void ShowNotifyMenu(POINT screen, NotifyItem const& item) {
    NotifyItem copy = item;
    std::wstring name = AppNameFor(item.aumid);
    if (name.empty()) name = L"this app";
    std::vector<MenuItem> items;
    items.push_back({L"Clear", [copy] { ClearNotify(copy); }});
    items.push_back({L"Clear all from " + name, [copy] {
                         ClearAppNotifications(copy.aumid);
                         ForgetNotify(copy, true);
                     }});
    items.push_back({L"Copy text", [copy] { CopyNotifyText(copy); }});
    items.push_back({});
    items.push_back({L"Open " + name, [copy] { OpenNotifyApp(copy); }});
    items.push_back({L"Turn off notifications from " + name, [copy] { TurnOffNotifyApp(copy.aumid); }});
    items.push_back({L"Notification settings", [] { OpenNotifySettings(); }});
    g_flyoutMenu = true;
    ShowMenu(items, MAKELPARAM(screen.x, screen.y));
    g_flyoutMenu = false;
}

LRESULT CALLBACK PopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* p = (Popup*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
    if (!p) return DefWindowProcW(wnd, msg, wParam, lParam);
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            RECT client;
            GetClientRect(wnd, &client);
            HDC mem = CreateCompatibleDC(dc);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
            HGDIOBJ old = SelectObject(mem, bitmap);
            PaintPopup(mem, client, *p);
            BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, old);
            DeleteObject(bitmap);
            DeleteDC(mem);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_MOUSEMOVE: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int hover = INT_MIN;
            for (auto const& [rc, id] : p->hits)
                if (PtInRect(&rc, pt)) {
                    hover = id;
                    break;
                }
            if (!p->tracking) {
                TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, wnd, 0};
                p->tracking = TrackMouseEvent(&track);
                KillTimer(wnd, kTimerPopupGone);  // (stays while the pointer is on it)
            }
            if (hover != p->hover) {
                p->hover = hover;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            p->tracking = false;
            p->hover = INT_MIN;
            InvalidateRect(wnd, nullptr, FALSE);
            SetTimer(wnd, kTimerPopupGone, 2000, nullptr);
            return 0;
        case WM_LBUTTONUP: {
            int id = p->hover;
            if (id == INT_MIN) return 0;
            NotifyItem item = p->item;
            ClosePopup(p);
            if (id == kPopupBody) ActivateNotification(item, item.launch, item.activationType);
            else if (id >= 0 && id < (int)item.actions.size())
                ActivateNotification(item, item.actions[id].arguments, item.actions[id].activationType);
            return 0;
        }
        case WM_MBUTTONUP: {  // cleared, from the centre too
            NotifyItem item = p->item;
            ClearNotify(item);
            return 0;
        }
        case WM_RBUTTONUP: {  // its menu; the pop-up waits while it's up (the menu may clear it: p is gone then)
            KillTimer(wnd, kTimerPopupGone);
            NotifyItem item = p->item;
            POINT screen{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ClientToScreen(wnd, &screen);
            ShowNotifyMenu(screen, item);
            if (IsWindow(wnd) && GetWindowLongPtrW(wnd, GWLP_USERDATA)) SetTimer(wnd, kTimerPopupGone, 2000, nullptr);
            return 0;
        }
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_TIMER:
            if (wParam == kTimerPopupGone) ClosePopup(p);
            else if (wParam == kTimerMotion && p->opening) PopupOpeningStep(*p);
            else if (wParam == kTimerMotion) KillTimer(wnd, kTimerMotion);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

void ShowPopup(NotifyApp const& app, NotifyItem const& item) {
    static bool registered = false;
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&PopupProc, &instance);
    if (!registered) {
        WNDCLASSW wc{};
        wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = PopupProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_HAND);
        wc.lpszClassName = kPopupClass;
        registered = RegisterClassW(&wc) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }
    while ((int)g_popups.size() >= kMaxPopups) ClosePopup(g_popups.front());
    auto* p = new Popup{};
    p->item = item;
    p->appName = app.name;
    p->design = app.design;
    UINT dpi = GetDpiForSystem();
    p->scale = std::max(1, (int)(dpi + USER_DEFAULT_SCREEN_DPI / 2) / USER_DEFAULT_SCREEN_DPI);
    PopupFonts(p->scale);
    p->wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kPopupClass, L"", WS_POPUP, 0, 0, 1, 1,
                             nullptr, nullptr, instance, nullptr);
    if (!p->wnd) {
        delete p;
        return;
    }
    SetWindowLongPtrW(p->wnd, GWLP_USERDATA, (LONG_PTR)p);
    DWORD corner = 1;  // (square, as the flyouts)
    DwmSetWindowAttribute(p->wnd, 33, &corner, sizeof(corner));
    g_popups.push_back(p);
    StartPopupOpening(*p);
    PlacePopups();
    // (a phone's ringing call stays up as long as a phone rings, to answer it here: phone-ancs.cpp)
    int seconds = item.launch == L"tourne-phone-call" ? 30 : g_settings.notificationPopupSeconds;
    SetTimer(p->wnd, kTimerPopupGone, seconds * 1000, nullptr);
}

// Tourne Desktop's Notifications page (notify.*): its pop-ups chosen over Windows' own (which it turns
// off then), and each app's Pop-ups setting. Not set for an app (one new since it started): yes.
bool TournePopupsChosen() {
#ifdef TD_PREFIX
    return !wcscmp(WindhawkUtils::StringSetting::make(L"notify.popups").get(), L"tourne");
#else
    return false;
#endif
}

bool AppWantsPopups(std::wstring const& aumid) {
#ifdef TD_PREFIX
    std::wstring key;
    for (wchar_t c : aumid) key += iswalnum(c) ? (wchar_t)towlower(c) : L'_';
    for (PCWSTR field : {L".allow", L".banners"}) {
        std::wstring name = L"notify.apps." + key + field;
        if (name.find(L'%') != std::wstring::npos) continue;
        auto value = WindhawkUtils::StringSetting::make(name.c_str());
        std::wstring v = value.get();
        if (v == L"0" || v == L"false" || v == L"off" || v == L"no") return false;
    }
#endif
    return true;
}

// Tourne holds the phone itself (its texts, notifications and calls; the taskbar part's start-menu.cpp marks its
// Start window so): the centre's phone opens Tourne's, not Phone Link.
bool TourneHoldsPhone() {
    HWND start = FindWindowW(L"TourneStartMenu_" WH_MOD_ID, nullptr);
    return start && GetPropW(start, L"TourneHoldsPhone");
}

// The notification database changed: a pop-up for each one newer than the last shown.
void CheckNewNotifications() {
    bool tourne = TournePopupsChosen();
    Wh_Log(L"notifications changed: Tourne's pop-ups %s, popups setting %d", tourne ? L"chosen" : L"not chosen",
           (int)g_settings.notificationPopups);
    if ((!g_settings.notificationPopups && !tourne) || (!tourne && WindowsShowsPopups())) {
        g_popupSeen = std::max(g_popupSeen, NowFileTime());
        return;
    }
    DWORD toasts = 1;
    bool dnd = ReadDword(HKEY_CURRENT_USER, kNotificationsPath, L"NOC_GLOBAL_SETTING_TOASTS_ENABLED", &toasts) && !toasts;
    ULONGLONG newest = g_popupSeen;
    std::vector<std::pair<NotifyApp const*, NotifyItem const*>> fresh;
    auto apps = LoadNotifications();
    for (auto const& app : apps)
        for (auto const& item : app.items)
            if (item.arrival > g_popupSeen) {
                if (!item.quiet && AppWantsPopups(app.aumid)) fresh.push_back({&app, &item});
                newest = std::max(newest, item.arrival);
            }
    g_popupSeen = newest;
    Wh_Log(L"notifications: %zu new to pop up%s", fresh.size(), dnd ? L" (do not disturb: none shown)" : L"");
    if (dnd) return;
    std::sort(fresh.begin(), fresh.end(), [](auto const& a, auto const& b) { return a.second->arrival < b.second->arrival; });
    for (auto const& [app, item] : fresh) ShowPopup(*app, *item);
}

void CloseAllPopups() {
    while (!g_popups.empty()) ClosePopup(g_popups.front());
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

    // The linked phone: the tray's phone design, its name, battery and connection; then its links.
    if (f.phone.linked) {
        int phoneLeft = c.Px(40);
        RECT row{c.Px(5), c.y, c.width - c.Px(5), c.y + c.Px(24)};
        int headId = MakeHit(kHitPhoneLink, 0);
        if (hover == headId) c.Fill(row, ui::kHover);
        c.Art("phone", c.Px(15), c.y + c.Px(4), c.Px(16));
        std::wstring status;
        if (f.phone.battery >= 0) status = std::to_wstring(f.phone.battery) + L"%";
        int right = c.width - c.Px(15);
        if (f.phone.battery >= 0) {
            const char* battery = f.phone.battery >= 90 ? "battery-full" : f.phone.battery >= 60 ? "battery-high"
                                : f.phone.battery >= 30 ? "battery-mid" : "battery-low";
            SIZE size = c.Measure(f.caption, status);
            c.Text(f.caption, ui::kSage, {right - size.cx, c.y, right, c.y + c.Px(24)}, status, DT_RIGHT);
            right -= size.cx + c.Px(4);
            c.Art(battery, right - c.Px(16), c.y + c.Px(4), c.Px(16));
            right -= c.Px(22);
        }
        c.Text(f.body, ui::kInk, {phoneLeft, c.y, right, c.y + c.Px(24)}, f.phone.name.empty() ? L"Your phone" : f.phone.name);
        hits.push_back({row, headId});
        c.y += c.Px(24);
        c.Text(f.caption, f.phone.connected ? ui::kGreen : ui::kSageAlt, {phoneLeft, c.y, c.width - c.Px(15), c.y + c.Px(16)},
               (f.phone.connected ? L"Connected" : L"Not connected") + (f.phone.unread > 0 ? L"  ·  " + std::to_wstring(f.phone.unread) + L" new" : L""));
        c.y += c.Px(18);
        int x = phoneLeft;
        static const wchar_t* const kLinks[] = {L"Messages", L"Send a file", L"Phone Link"};
        static const wchar_t* const kTourneLinks[] = {L"Messages", L"Send a file", L"Bluetooth"};
        bool tourne = TourneHoldsPhone();
        for (int k = 0; k < 3; k++) {
            wchar_t const* label = (tourne ? kTourneLinks : kLinks)[k];
            SIZE size = c.Measure(f.caption, label);
            RECT button{x, c.y + c.Px(2), x + size.cx + c.Px(12), c.y + c.Px(18)};
            if (button.right > c.width - c.Px(15)) break;
            int id = MakeHit(kHitPhoneLink, k + 1);
            c.Frame(button, hover == id ? ui::kCoral : ui::kEdge);
            c.CenteredText(f.caption, hover == id ? ui::kCoral : ui::kGreen, button, label, 0);
            hits.push_back({button, id});
            x = button.right + c.Px(8);
        }
        c.y += c.Px(26);
        c.Divider();
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

////////////////////////////////////////////////////////////////////////////////
// A new event from the calendar ("Right-click a day to add an event"): a small window for it, and "Add to
// Google Calendar" opens Google Calendar's own new-event page in the browser with it filled in, where one
// click saves it. Nothing is sent from here: it's a link opened in the browser.

constexpr WCHAR kEventClass[] = L"TourneTrayEvent_" WH_MOD_ID;
enum EventControl : int {
    kEvTitle = 101, kEvDate, kEvAllDay, kEvStart, kEvEnd, kEvWhere, kEvImportance, kEvNotes, kEvStatus, kEvAccount
};
constexpr int kEvLabel = -1;  // the labels

struct EventEditor {
    HWND wnd = nullptr;
    HFONT font = nullptr;
    HBRUSH ground = nullptr, field = nullptr;
    int scale = 1;
    bool allDay = false;
    int importance = 1;  // 0 low, 1 normal, 2 high
    int account = 0;     // in g_settings.calendarAccounts
    std::vector<RECT> frames;  // around the text boxes
    std::wstring error;
} g_event;

constexpr PCWSTR kImportanceNames[] = {L"Low", L"Normal", L"High"};
WindowMotion g_eventIn;  // the event window fading in, on the popups line (it doesn't slide)

std::wstring EventText(int id) {
    HWND control = GetDlgItem(g_event.wnd, id);
    int length = GetWindowTextLengthW(control);
    std::wstring text(length, L'\0');
    if (length) GetWindowTextW(control, text.data(), length + 1);
    return text;
}

// "9", "9:30", "0930", "9pm", "9:30 am", "21:15": minutes after midnight, or -1.
int ParseEventTime(std::wstring text) {
    std::wstring s;
    for (WCHAR ch : text)
        if (!iswspace(ch)) s += (WCHAR)towlower(ch);
    int half = 0;  // 1 am, 2 pm
    constexpr std::pair<PCWSTR, int> kSuffixes[] = {{L"am", 1}, {L"pm", 2}, {L"a", 1}, {L"p", 2}};
    for (auto [suffix, which] : kSuffixes) {
        std::wstring_view end(suffix);
        if (s.size() > end.size() && std::wstring_view(s).ends_with(end)) {
            s.resize(s.size() - end.size());
            half = which;
            break;
        }
    }
    int hour = 0, minute = 0;
    size_t colon = s.find_first_of(L":.");
    auto digits = [](std::wstring const& d) { return !d.empty() && d.size() <= 2 && std::all_of(d.begin(), d.end(), iswdigit); };
    if (colon != std::wstring::npos) {
        std::wstring h = s.substr(0, colon), m = s.substr(colon + 1);
        if (!digits(h) || !digits(m) || m.size() != 2) return -1;
        hour = _wtoi(h.c_str());
        minute = _wtoi(m.c_str());
    } else if (s.size() >= 3 && s.size() <= 4 && std::all_of(s.begin(), s.end(), iswdigit)) {
        hour = _wtoi(s.substr(0, s.size() - 2).c_str());
        minute = _wtoi(s.substr(s.size() - 2).c_str());
    } else if (digits(s)) {
        hour = _wtoi(s.c_str());
    } else {
        return -1;
    }
    if (half) {
        if (hour < 1 || hour > 12) return -1;
        hour = hour % 12 + (half == 2 ? 12 : 0);
    }
    if (hour > 23 || minute > 59) return -1;
    return hour * 60 + minute;
}

// "2026-10-03" (as the window fills it in): true with the day.
bool ParseEventDate(std::wstring const& text, SYSTEMTIME* day) {
    int y = 0, m = 0, d = 0;
    if (swscanf_s(text.c_str(), L"%d-%d-%d", &y, &m, &d) != 3) return false;
    SYSTEMTIME t{};
    t.wYear = (WORD)y;
    t.wMonth = (WORD)m;
    t.wDay = (WORD)d;
    FILETIME check;
    if (y < 1900 || y > 2200 || !SystemTimeToFileTime(&t, &check)) return false;
    *day = t;
    return true;
}

std::wstring UrlEncode(std::wstring const& text) {
    int length = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0, nullptr, nullptr);
    std::string utf8(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), (int)text.size(), utf8.data(), length, nullptr, nullptr);
    std::wstring out;
    for (unsigned char ch : utf8) {
        if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            out += (WCHAR)ch;
        } else {
            WCHAR hex[4];
            swprintf_s(hex, L"%%%02X", ch);
            out += hex;
        }
    }
    return out;
}

// A local time as Google wants it: in UTC, "20261003T143000Z".
std::wstring GoogleUtc(SYSTEMTIME const& local) {
    SYSTEMTIME utc;
    if (!TzSpecificLocalTimeToSystemTime(nullptr, &local, &utc)) utc = local;
    WCHAR out[20];
    swprintf_s(out, L"%04u%02u%02uT%02u%02u00Z", utc.wYear, utc.wMonth, utc.wDay, utc.wHour, utc.wMinute);
    return out;
}

std::wstring GoogleDay(SYSTEMTIME const& day) {
    WCHAR out[12];
    swprintf_s(out, L"%04u%02u%02u", day.wYear, day.wMonth, day.wDay);
    return out;
}

// Google Calendar's new-event page for the window's fields; false (with the reason in e.error) if one's wrong.
bool EventUrl(std::wstring* out) {
    auto& e = g_event;
    std::wstring title = EventText(kEvTitle);
    SYSTEMTIME day;
    if (!ParseEventDate(EventText(kEvDate), &day)) {
        e.error = L"The day is written as year-month-day, like 2026-10-03.";
        return false;
    }
    LONGLONG dayMinutes = MinutesOf(day);
    std::wstring dates;
    if (e.allDay) {
        dates = GoogleDay(day) + L"/" + GoogleDay(TimeOfMinutes(dayMinutes + 24 * 60));
    } else {
        int start = ParseEventTime(EventText(kEvStart)), end = ParseEventTime(EventText(kEvEnd));
        if (start < 0 || end < 0) {
            e.error = L"Times are like 9:30, 14:00 or 2pm.";
            return false;
        }
        LONGLONG from = dayMinutes + start, to = dayMinutes + end;
        if (to <= from) to += 24 * 60;  // past midnight
        dates = GoogleUtc(TimeOfMinutes(from)) + L"/" + GoogleUtc(TimeOfMinutes(to));
    }
    std::wstring notes = EventText(kEvNotes);
    if (e.importance != 1) notes += std::wstring(notes.empty() ? L"" : L"\n\n") + L"Importance: " + kImportanceNames[e.importance];
    if (e.importance == 2) title = L"! " + title;
    std::wstring url = L"https://calendar.google.com/calendar/render?action=TEMPLATE&text=" + UrlEncode(title) +
                       L"&dates=" + dates;
    if (!notes.empty()) url += L"&details=" + UrlEncode(notes);
    if (std::wstring where = EventText(kEvWhere); !where.empty()) url += L"&location=" + UrlEncode(where);
    // As the chosen account, not the browser's default one.
    auto const& accounts = g_settings.calendarAccounts;
    if (e.account >= 0 && e.account < (int)accounts.size()) url += L"&authuser=" + UrlEncode(accounts[e.account]);
    *out = url;
    return true;
}

// Opens that page in the browser.
bool AddEventToGoogle() {
    std::wstring url;
    if (!EventUrl(&url)) return false;
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return true;
}

LRESULT CALLBACK EventWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto& e = g_event;
    switch (msg) {
        case WM_CTLCOLOREDIT:
            SetTextColor((HDC)wParam, ui::kInk);
            SetBkColor((HDC)wParam, ui::kRaised);
            return (LRESULT)e.field;
        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, GetDlgCtrlID((HWND)lParam) == kEvStatus && !e.error.empty() ? ui::kCoral : ui::kSage);
            SetBkColor((HDC)wParam, ui::kGround);
            return (LRESULT)e.ground;
        case WM_ERASEBKGND: {
            RECT rc;
            GetClientRect(wnd, &rc);
            FillRect((HDC)wParam, &rc, e.ground);
            HBRUSH edge = CreateSolidBrush(ui::kEdge);
            for (RECT frame : e.frames) FrameRect((HDC)wParam, &frame, edge);
            DeleteObject(edge);
            return 1;
        }
        case WM_DRAWITEM: {
            auto* item = (DRAWITEMSTRUCT*)lParam;
            bool pressed = item->itemState & ODS_SELECTED, focused = item->itemState & ODS_FOCUS;
            bool on = (item->CtlID == kEvAllDay && e.allDay) || item->CtlID == IDOK;
            HBRUSH fill = CreateSolidBrush(pressed ? ui::kHover : on ? ui::kRaised : ui::kGround);
            FillRect(item->hDC, &item->rcItem, fill);
            DeleteObject(fill);
            HBRUSH frame = CreateSolidBrush(focused || on ? ui::kGreen : ui::kEdge);
            FrameRect(item->hDC, &item->rcItem, frame);
            DeleteObject(frame);
            std::wstring text(GetWindowTextLengthW(item->hwndItem), L'\0');
            GetWindowTextW(item->hwndItem, text.data(), (int)text.size() + 1);
            if (item->CtlID == kEvAllDay) text = e.allDay ? L"[x] All day" : L"[ ] All day";
            if (item->CtlID == kEvImportance) text = std::wstring(L"Importance: ") + kImportanceNames[e.importance];
            if (item->CtlID == kEvAccount && e.account < (int)g_settings.calendarAccounts.size()) {
                text = g_settings.calendarAccounts[e.account];
                if (g_settings.calendarAccounts.size() > 1) text += L"  >";  // (a click: the next one)
            }
            SetBkMode(item->hDC, TRANSPARENT);
            SetTextColor(item->hDC, focused || on ? ui::kGreen : ui::kSage);
            HGDIOBJ old = SelectObject(item->hDC, e.font);
            DrawTextW(item->hDC, text.c_str(), -1, &item->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(item->hDC, old);
            return TRUE;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case kEvAllDay:
                    e.allDay = !e.allDay;
                    EnableWindow(GetDlgItem(wnd, kEvStart), !e.allDay);
                    EnableWindow(GetDlgItem(wnd, kEvEnd), !e.allDay);
                    InvalidateRect(GetDlgItem(wnd, kEvAllDay), nullptr, FALSE);
                    return 0;
                case kEvImportance:
                    e.importance = (e.importance + 1) % 3;
                    InvalidateRect(GetDlgItem(wnd, kEvImportance), nullptr, FALSE);
                    return 0;
                case kEvAccount:
                    if (!g_settings.calendarAccounts.empty())
                        e.account = (e.account + 1) % (int)g_settings.calendarAccounts.size();
                    InvalidateRect(GetDlgItem(wnd, kEvAccount), nullptr, FALSE);
                    return 0;
                case IDOK:
                    if (AddEventToGoogle()) {
                        DestroyWindow(wnd);
                    } else {
                        SetDlgItemTextW(wnd, kEvStatus, e.error.c_str());
                    }
                    return 0;
                case IDCANCEL:
                    DestroyWindow(wnd);
                    return 0;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(wnd);
            return 0;
        case WM_TIMER:
            if (wParam != kTimerMotion) break;
            FadeInStep(wnd, g_eventIn);
            return 0;
        case WM_DESTROY:
            KillTimer(wnd, kTimerMotion);
            if (e.font) DeleteObject(e.font);
            if (e.ground) DeleteObject(e.ground);
            if (e.field) DeleteObject(e.field);
            e = {};
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// For the thread's message loop: Tab, Enter and Escape in the event window.
bool EventDialogMessage(MSG* msg) {
    return g_event.wnd && IsDialogMessageW(g_event.wnd, msg);
}

void OpenEventEditor(LONGLONG dayMinutes) {
    if (g_event.wnd) {
        SetForegroundWindow(g_event.wnd);
        return;
    }
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = EventWndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kEventClass;
        registered = RegisterClassW(&wc) != 0;
    }
    POINT pt;
    GetCursorPos(&pt);
    UINT dpi = GetDpiForSystem();
    int s = std::max(1, (int)(dpi + 48) / 96);
    auto& e = g_event;
    e.scale = s;
    e.font = MakeFont({L"CozetteVector", L"scientifica"}, 13 * s);
    e.ground = CreateSolidBrush(ui::kGround);
    e.field = CreateSolidBrush(ui::kRaised);
    int width = 380 * s, pad = 14 * s, row = 30 * s, labelWidth = 90 * s, fieldHeight = 22 * s;
    bool accounts = !g_settings.calendarAccounts.empty();  // a row to pick the Google account
    int height = pad + row * (accounts ? 7 : 6) + 70 * s + row + 22 * s + pad;
    RECT frame{0, 0, width, height};
    AdjustWindowRectExForDpi(&frame, WS_CAPTION | WS_SYSMENU, FALSE, WS_EX_TOOLWINDOW, dpi);
    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &monitor);
    int w = frame.right - frame.left, h = frame.bottom - frame.top;
    RECT const& work = monitor.rcWork;
    int x = std::clamp((int)pt.x - w / 2, (int)work.left, (int)work.right - w);
    int y = std::clamp((int)pt.y - h - 12 * s, (int)work.top, (int)work.bottom - h);
    SYSTEMTIME day = TimeOfMinutes(dayMinutes);
    std::wstring dayName = FormatDate(day, nullptr, DATE_LONGDATE);
    e.wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_CONTROLPARENT, kEventClass,
                            (L"New event, " + dayName).c_str(), WS_CAPTION | WS_SYSMENU | WS_POPUP, x, y, w, h,
                            nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!e.wnd) return;
    BOOL dark = TRUE;
    DwmSetWindowAttribute(e.wnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));
    COLORREF caption = ui::kGround, text = ui::kInk, border = ui::kEdge;
    DwmSetWindowAttribute(e.wnd, 35 /* DWMWA_CAPTION_COLOR */, &caption, sizeof(caption));
    DwmSetWindowAttribute(e.wnd, 36 /* DWMWA_TEXT_COLOR */, &text, sizeof(text));
    DwmSetWindowAttribute(e.wnd, 34 /* DWMWA_BORDER_COLOR */, &border, sizeof(border));

    auto add = [&](PCWSTR cls, PCWSTR text, DWORD style, int id, int left, int top, int right, int bottom) {
        HWND control = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, left, top, right - left,
                                       bottom - top, e.wnd, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
        SendMessageW(control, WM_SETFONT, (WPARAM)e.font, FALSE);
        return control;
    };
    auto label = [&](PCWSTR text, int top, int left = -1) {
        int l = left < 0 ? pad : left;
        add(L"STATIC", text, SS_LEFT | SS_CENTERIMAGE, kEvLabel, l, top, l + labelWidth, top + fieldHeight);
    };
    // A text box with a one-pixel frame drawn around it (in the window's colours, not the system's).
    auto box = [&](int id, PCWSTR text, DWORD style, int left, int top, int right, int bottom) {
        e.frames.push_back({left - 1, top - 1, right + 1, bottom + 1});
        int inset = 4 * s;
        return add(L"EDIT", text, WS_TABSTOP | ES_AUTOHSCROLL | style, id, left + inset, top + 3 * s, right - inset,
                   bottom - 3 * s);
    };
    int fieldLeft = pad + labelWidth, right = width - pad;
    int top = pad;
    label(L"Title", top);
    HWND title = box(kEvTitle, L"", 0, fieldLeft, top, right, top + fieldHeight);
    top += row;
    label(L"Day", top);
    WCHAR date[16];
    swprintf_s(date, L"%04u-%02u-%02u", day.wYear, day.wMonth, day.wDay);
    box(kEvDate, date, 0, fieldLeft, top, fieldLeft + 110 * s, top + fieldHeight);
    add(L"BUTTON", L"All day", WS_TABSTOP | BS_OWNERDRAW, kEvAllDay, right - 110 * s, top, right, top + fieldHeight);
    top += row;
    // A start on the next whole hour today, else at nine; an hour long.
    SYSTEMTIME now;
    GetLocalTime(&now);
    int startHour = DayStart(MinutesOf(now)) == DayStart(dayMinutes) ? std::min(23, now.wHour + 1) : 9;
    WCHAR from[8], to[8];
    swprintf_s(from, L"%02d:00", startHour);
    swprintf_s(to, L"%02d:00", (startHour + 1) % 24);
    label(L"From", top);
    box(kEvStart, from, 0, fieldLeft, top, fieldLeft + 80 * s, top + fieldHeight);
    label(L"to", top, fieldLeft + 96 * s);
    box(kEvEnd, to, 0, fieldLeft + 126 * s, top, fieldLeft + 206 * s, top + fieldHeight);
    top += row;
    label(L"Where", top);
    box(kEvWhere, L"", 0, fieldLeft, top, right, top + fieldHeight);
    top += row;
    label(L"", top);
    add(L"BUTTON", L"Importance", WS_TABSTOP | BS_OWNERDRAW, kEvImportance, fieldLeft, top, fieldLeft + 160 * s,
        top + fieldHeight);
    top += row;
    if (accounts) {
        label(L"Account", top);
        add(L"BUTTON", L"Account", WS_TABSTOP | BS_OWNERDRAW, kEvAccount, fieldLeft, top, right, top + fieldHeight);
        top += row;
    }
    label(L"Notes", top);
    box(kEvNotes, L"", ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL, fieldLeft, top, right, top + 70 * s);
    top += 70 * s + 10 * s;
    add(L"BUTTON", L"Add to Google Calendar", WS_TABSTOP | BS_OWNERDRAW | BS_DEFPUSHBUTTON, IDOK, width - pad - 300 * s,
        top, width - pad - 100 * s, top + 24 * s);
    add(L"BUTTON", L"Cancel", WS_TABSTOP | BS_OWNERDRAW, IDCANCEL, width - pad - 90 * s, top, width - pad, top + 24 * s);
    top += row;
    add(L"STATIC", L"Opens Google Calendar in your browser, where you save it.", SS_LEFT, kEvStatus, pad, top,
        right, top + 22 * s);

    // With the popups line (or fadePopupsIn), it fades in as the flyouts do.
    MotionSlide slide;
    if (PlanMotion(g_motion.popups, g_motion.fadePopupsIn, L"fade", false, &g_eventIn, &slide) && g_eventIn.fade.on) {
        g_eventIn.slide.on = false;
        BeginOpening(e.wnd, g_eventIn);
    }
    ShowWindow(e.wnd, SW_SHOW);
    SetForegroundWindow(e.wnd);
    SetFocus(title);
}

// Window

int Render(HDC dc, bool paint, std::vector<Hit>& hits) {
    Canvas c{dc, paint, g_flyout.scale, g_flyout.fitWidth ? g_flyout.fitWidth : g_flyout.width};
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
        case kFlyoutBluetooth: RenderBluetooth(c, hits, g_flyout.hover); break;
        case kFlyoutDevices: RenderDevices(c, hits, g_flyout.hover); break;
    }
    if (!paint) {
        g_flyout.needed = c.needed;
    }
    return c.y;
}

// Sizes the window to its content and keeps it next to the tray.
// A click in the Tourne taskbar's tray leaves the icon's rectangle on this
// window (Shell_NotifyIconGetRect only knows Explorer's tray): left, top,
// right and bottom as 16-bit values, and when it was left there.
constexpr WCHAR kTaskbarAnchorProp[] = L"TourneTaskbar_Anchor";
constexpr WCHAR kTaskbarAnchorTickProp[] = L"TourneTaskbar_AnchorTick";
bool g_anchorFromTaskbar;
RECT g_taskbarAnchor;
bool g_flyoutAtTop;  // placed against the top of the screen (a taskbar at the top), not the bottom
WindowMotion g_flyoutIn;  // the flyout opening, on the popups line (its motion timer runs while it does)
POINT g_flyoutShift{};    // how far its slide has it now from where it's laid out
BYTE g_flyoutAlpha = 255;

void TakeTaskbarAnchor() {
    g_anchorFromTaskbar = false;
    auto packed = (uint64_t)(ULONG_PTR)GetPropW(g_wnd, kTaskbarAnchorProp);
    auto tick = (DWORD)(ULONG_PTR)GetPropW(g_wnd, kTaskbarAnchorTickProp);
    RemovePropW(g_wnd, kTaskbarAnchorProp);
    RemovePropW(g_wnd, kTaskbarAnchorTickProp);
    if (!packed || GetTickCount() - tick > 1500) {
        return;
    }
    g_taskbarAnchor = {(short)(packed & 0xFFFF), (short)((packed >> 16) & 0xFFFF),
                       (short)((packed >> 32) & 0xFFFF), (short)((packed >> 48) & 0xFFFF)};
    g_anchorFromTaskbar = true;
}

// The open flyout's rectangle, if the point is on it (a menu opened from inside it).
bool FlyoutUnder(POINT pt, RECT* rc) {
    return g_flyout.hwnd && IsWindowVisible(g_flyout.hwnd) && GetWindowRect(g_flyout.hwnd, rc) && PtInRect(rc, pt);
}

// For a right-click menu: the icon's rectangle on the Tourne taskbar, if the click came from there.
bool TakeMenuAnchor(RECT* icon) {
    TakeTaskbarAnchor();
    if (!g_anchorFromTaskbar) return false;
    *icon = g_taskbarAnchor;
    g_anchorFromTaskbar = false;
    return true;
}

// A flyout never reaches onto another screen: inside the work area of the screen "on" is on (its icon,
// or where it already is), moved back in, and no bigger than that area.
void KeepOnScreen(RECT* rc, RECT const& on) {
    MONITORINFO monitor{sizeof(monitor)};
    GetMonitorInfoW(MonitorFromRect(&on, MONITOR_DEFAULTTONEAREST), &monitor);
    RECT const& work = monitor.rcWork;
    LONG width = std::min(rc->right - rc->left, work.right - work.left);
    LONG height = std::min(rc->bottom - rc->top, work.bottom - work.top);
    LONG left = std::clamp(rc->left, work.left, work.right - width);
    LONG top = std::clamp(rc->top, work.top, work.bottom - height);
    *rc = {left, top, left + width, top + height};
}

void LayoutFlyout(int iconKind) {
    HWND hwnd = g_flyout.hwnd;
    HDC dc = GetDC(hwnd);
    // Measured at the standard width first; with Auto width, laid out again
    // as wide as the longest line wants, up to the maximum. The notification
    // centre wraps its own text and the hidden icons flyout is a grid, so
    // both keep their width.
    g_flyout.fitWidth = g_flyout.width;
    int height = Render(dc, false, g_flyout.hits);
    bool fits = g_flyout.kind != kFlyoutNotify && g_flyout.kind != kFlyoutOverflow;
    if (g_settings.flyoutAutoWidth && fits && g_flyout.needed > g_flyout.width) {
        int maxWidth = std::max(g_flyout.width, g_settings.flyoutMaxWidth * g_flyout.scale);
        g_flyout.fitWidth = std::min(g_flyout.needed, maxWidth);
        height = Render(dc, false, g_flyout.hits);
    }
    ReleaseDC(hwnd, dc);
    int width = g_flyout.fitWidth;

    RECT anchor{};
    NOTIFYICONIDENTIFIER identifier{sizeof(identifier)};
    identifier.hWnd = g_wnd;
    identifier.uID = kIconIdBase + iconKind;
    if (g_anchorFromTaskbar && (iconKind >= 0 || !IsWindowVisible(hwnd))) {
        anchor = g_taskbarAnchor;
    } else if (iconKind < 0 || FAILED(Shell_NotifyIconGetRect(&identifier, &anchor))) {
        RECT current;
        if (GetWindowRect(hwnd, &current) && IsWindowVisible(hwnd)) {
            // Keep the edge by the taskbar while resizing, as it was placed (a tall flyout, like the
            // notification centre, reaches the top half of the screen either way), on its own screen.
            // (Laid out where it ends up, while it still slides in.)
            OffsetRect(&current, -g_flyoutShift.x, -g_flyoutShift.y);
            RECT placed{current.left, g_flyoutAtTop ? current.top : current.bottom - height, 0, 0};
            placed.right = placed.left + width;
            placed.bottom = placed.top + height;
            KeepOnScreen(&placed, current);
            OffsetRect(&placed, g_flyoutShift.x, g_flyoutShift.y);
            SetWindowPos(hwnd, nullptr, placed.left, placed.top, placed.right - placed.left, placed.bottom - placed.top,
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
    int away = gap + g_settings.flyoutAway * g_flyout.scale;  // from the taskbar ("Flyout extra distance")
    int along = g_settings.flyoutAlong * g_flyout.scale;
    int x = std::clamp((int)(anchor.left + anchor.right) / 2 - width / 2 + along,
                       (int)work.left + gap, (int)work.right - width - gap);
    int y;
    g_flyoutAtTop = false;
    if (anchor.top >= work.bottom - gap) {
        y = work.bottom - height - away;  // Taskbar at the bottom.
    } else if (anchor.bottom <= work.top + gap) {
        y = work.top + away;  // At the top.
        g_flyoutAtTop = true;
    } else {
        y = std::clamp((int)anchor.top - height - away, (int)work.top + gap,
                       (int)work.bottom - height - gap);
        g_flyoutAtTop = y > anchor.bottom;  // (below its icon: grows down from there)
    }
    RECT placed{x, y, x + width, y + height};
    KeepOnScreen(&placed, anchor);
    OffsetRect(&placed, g_flyoutShift.x, g_flyoutShift.y);
    SetWindowPos(hwnd, HWND_TOPMOST, placed.left, placed.top, placed.right - placed.left, placed.bottom - placed.top,
                 SWP_NOACTIVATE);
}

void LoadFlyout() {
    switch (g_flyout.kind) {
        case kFlyoutNetwork: LoadNetworkFlyout(); break;
        case kFlyoutPower: LoadPowerFlyout(); break;
        case kFlyoutBrightness: break;  // Filled in by the worker.
        case kFlyoutOverflow: LoadOverflowFlyout(); break;
        case kFlyoutVpn: LoadVpnFlyout(); break;
        case kFlyoutSecurity: LoadSecurityFlyout(); break;
        // The mixer's timer reloads only its programs; Bluetooth devices are
        // read here, when it opens and when something changed.
        case kFlyoutMixer:
            LoadMixerFlyout();
            LoadBluetoothDevices();
            LoadBatteriesFlyout();
            break;
        case kFlyoutNotify: LoadNotifyFlyout(); break;
        case kFlyoutBluetooth:
            LoadBluetoothDevices();
            LoadBatteriesFlyout();
            break;
        case kFlyoutDevices: LoadBatteriesFlyout(); break;
    }
}

bool IsFlyoutOpen(int flyoutKind) {
    return g_flyout.hwnd && g_flyout.kind == flyoutKind;
}

void ReloadFlyout() {
    if (g_flyout.hwnd &&
        (g_flyout.kind == kFlyoutNetwork || g_flyout.kind == kFlyoutPower ||
         g_flyout.kind == kFlyoutVpn || g_flyout.kind == kFlyoutSecurity ||
         g_flyout.kind == kFlyoutMixer || g_flyout.kind == kFlyoutNotify ||
         g_flyout.kind == kFlyoutBluetooth || g_flyout.kind == kFlyoutDevices)) {
        PostMessageW(g_flyout.hwnd, WM_APP_FLYOUT_RELOAD, 0, 0);
    }
}

// The flyout's look, into dc (the size of the window).
void PaintFlyout(HDC dc, RECT const& client) {
    SetDCBrushColor(dc, ui::kGround);
    FillRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
    Render(dc, true, g_flyout.hits);
    SetDCBrushColor(dc, ui::kEdge);
    FrameRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
}

// One frame of the flyout opening (its motion timer); at the end it's fully shown where it's laid out.
void FlyoutOpeningStep(HWND hwnd) {
    POINT offset;
    bool done = MotionAt(g_flyoutIn, false, 255, &g_flyoutAlpha, &offset);
    SetLayeredWindowAttributes(hwnd, 0, g_flyoutAlpha, LWA_ALPHA);
    if (offset.x != g_flyoutShift.x || offset.y != g_flyoutShift.y) {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        SetWindowPos(hwnd, nullptr, rc.left + offset.x - g_flyoutShift.x, rc.top + offset.y - g_flyoutShift.y, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        g_flyoutShift = offset;
    }
    if (done) KillTimer(hwnd, kTimerMotion);
}

// With the popups line (or fadePopupsIn), the flyout about to show fades in in its style: slide, slidevert
// and slidefade come a short way from the bar's side, popin and gnomed a smaller way, as they fade.
void StartFlyoutOpening(HWND hwnd) {
    MotionSlide slide;
    if (!PlanMotion(g_motion.popups, g_motion.fadePopupsIn, L"fade", false, &g_flyoutIn, &slide)) return;
    int away = (slide == kSlideSmall ? 8 : 20) * g_flyout.scale;
    g_flyoutIn.offset = {0, slide == kSlideNone ? 0 : g_flyoutAtTop ? -away : away};
    BeginOpening(hwnd, g_flyoutIn);
    FlyoutOpeningStep(hwnd);  // (its first look, before it shows)
}

void CloseFlyout() {
    // Destroying the active window deactivates it, which would close it again.
    if (g_flyout.hwnd && !g_flyoutClosing) {
        g_flyoutClosing = true;
        // With the popups line (or fadePopupsOut), a still of it fades out; the window goes now, as before.
        WindowMotion out;
        MotionSlide slide;
        if (PlanMotion(g_motion.popups, g_motion.fadePopupsOut, L"fade", true, &out, &slide)) {
            out.slide.on = false;
            if (out.fade.on) StartStill(g_flyout.hwnd, g_flyoutAlpha, out, nullptr, PaintFlyout);
        }
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
        case kHitSecurityScan:
            // (As administrator: the same kind of scan again.)
            StartDefenderScan(index == 3 ? g_scan.lastType : index, index == 3);
            if (g_scan.running) SetTimer(f.hwnd, kTimerScanTick, 1000, nullptr);
            LayoutFlyout(-1);
            InvalidateRect(f.hwnd, nullptr, FALSE);
            return;
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
        case kHitBtToggle:
            ToggleBluetoothRadio();
            LayoutFlyout(-1);
            break;
        case kHitBtRow:
            if (index < (int)f.btDevices.size()) {
                f.btSelected = f.btSelected == f.btDevices[index].name ? L"" : f.btDevices[index].name;
            }
            LayoutFlyout(-1);
            break;
        case kHitBtAction:
        case kHitMixerBt:
            if (index < (int)f.btDevices.size()) {
                auto const& device = f.btDevices[index];
                if (device.audioFilters.empty()) {
                    CloseFlyout();
                    OpenUri(L"ms-settings:bluetooth");
                    return;
                }
                ConnectBluetooth(device, !device.connected);
            }
            LayoutFlyout(-1);
            break;
        case kHitPhoneLink: {
            // Into Phone Link: the app, its messages, or sending a file. With Tourne holding the phone, Tourne's own:
            // the phone with its texts, sending a file, Bluetooth's settings (the taskbar part's Start window does it).
            static const wchar_t* const kTargets[] = {kPhoneLinkApp, L"ms-phone:messages", L"ms-phone:filetransfer", kPhoneLinkApp};
            CloseFlyout();
            if (TourneHoldsPhone()) {
                PostMessageW(FindWindowW(L"TourneStartMenu_" WH_MOD_ID, nullptr), RegisterWindowMessageW(L"TournePhoneOpen"),
                             std::clamp(index, 0, 3), 0);
                break;
            }
            ShellExecuteW(nullptr, nullptr, kTargets[std::clamp(index, 0, 3)], nullptr, nullptr, SW_SHOWNORMAL);
            break;
        }
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
        case kHitFolder:
        case kHitFolderBack:
            // Into a folder, or back out to all the hidden icons.
            f.openFolder = type == kHitFolder ? index : -1;
            LoadOverflowFlyout();
            LayoutFlyout(-1);
            InvalidateRect(f.hwnd, nullptr, FALSE);
            return;
        case kHitLink:
            if (f.kind == kFlyoutNetwork) {
                CloseFlyout();
                if (index == 0) {
                    Run(L"control.exe", L"/name Microsoft.NetworkAndSharingCenter");
                } else if (index == 2) {
                    OpenUri(L"ms-settings:privacy-location");
                } else if (index == 3) {
                    OpenUri(L"ms-availablenetworks:");
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
            } else if (f.kind == kFlyoutBluetooth) {
                CloseFlyout();
                OpenUri(index == 0 ? L"ms-settings-connectabledevices:devicediscovery" : L"ms-settings:bluetooth");
            } else if (f.kind == kFlyoutDevices) {
                if (index == 0) {
                    WakeBatteries();
                } else {
                    CloseFlyout();
                    OpenUri(L"ms-settings:bluetooth");
                }
            }
            return;
    }
    if (g_flyout.hwnd) {
        InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
    }
}

// The smallest target under the point: a button inside a row (Wi-Fi's CONNECT, Bluetooth's) wins over
// the row, whichever was listed first.
int HitAt(POINT pt) {
    int found = 0;
    LONGLONG smallest = 0;
    for (auto const& hit : g_flyout.hits) {
        if (PtInRect(&hit.rc, pt)) {
            LONGLONG area = (LONGLONG)(hit.rc.right - hit.rc.left) * (hit.rc.bottom - hit.rc.top);
            if (!found || area < smallest) {
                found = hit.id;
                smallest = area;
            }
        }
    }
    return found;
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
            PaintFlyout(mem, client);
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
                    RECT client;
                    GetClientRect(hWnd, &client);
                    POINT screen = pt;
                    ClientToScreen(hWnd, &screen);
                    // Out of the flyout it follows the pointer, toward the taskbar.
                    if (!PtInRect(&client, pt)) {
                        DragHiddenOutside(screen);
                    } else if (g_flyout.ghost.wnd) {
                        EndHiddenDragOutside();
                    }
                    if (AimHiddenDrag(pt)) {
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
                // Its program hears of the pointer (some set their tooltip
                // only then), and the tooltip is read again a moment later.
                int index = hover & 0xFFFF;
                if ((hover >> 16) == kHitHidden && index < (int)g_flyout.hidden.size() && g_flyout.hidden[index].plugin.empty()) {
                    HoverHiddenIcon(g_flyout.hidden[index]);
                    SetTimer(hWnd, kTimerHoverTip, 250, nullptr);
                }
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, hWnd};
            TrackMouseEvent(&track);
            return 0;
        }

        case WM_CAPTURECHANGED:
            if (g_flyout.dragHidden >= 0 && (HWND)lParam != hWnd) {
                EndHiddenDragOutside();
                g_flyout.dragHidden = -1;
                g_flyout.dragMoved = false;
                g_flyout.dragTarget = -1;
                g_flyout.dragFolder = g_flyout.dragStack = g_flyout.stackWait = -1;
                KillTimer(hWnd, kTimerStackWait);
                g_flyout.dragBack = false;
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
                if (index < (int)g_flyout.hidden.size() && g_flyout.hidden[index].plugin.empty() &&
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
            if (g_flyout.renameBox) {
                // A click off the name box: the name is kept, and that's all the click does.
                EndFolderRename(true);
                SetFocus(hWnd);
                return 0;
            }
            int id = HitAt(pt);
            if ((id >> 16) == kHitHidden && msg == WM_LBUTTONDOWN) {
                // A press on a hidden icon may turn into a drag to move it.
                g_flyout.dragHidden = id & 0xFFFF;
                g_flyout.dragStart = pt;
                g_flyout.dragMoved = false;
                g_flyout.dragTarget = -1;
                g_flyout.dragFolder = g_flyout.dragStack = g_flyout.stackWait = -1;
                KillTimer(hWnd, kTimerStackWait);
                g_flyout.dragBack = false;
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
                int folder = g_flyout.dragFolder, stack = g_flyout.dragStack;
                bool back = g_flyout.dragBack;
                bool moved = g_flyout.dragMoved;
                HWND bar = g_flyout.dragOver;
                bool known = from < (int)g_flyout.hidden.size();
                std::wstring key = known ? HiddenIconKey(g_flyout.hidden[from]) : L"";
                g_flyout.dragOver = nullptr;  // Kept from EndHiddenDragOutside: the drop tells it.
                g_flyout.ghost.Hide();
                g_flyout.dragHidden = -1;
                g_flyout.dragMoved = false;
                g_flyout.dragTarget = -1;
                g_flyout.dragFolder = g_flyout.dragStack = g_flyout.stackWait = -1;
                KillTimer(hWnd, kTimerStackWait);
                g_flyout.dragBack = false;
                ReleaseCapture();
                if (moved) {
                    POINT screen = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                    ClientToScreen(hWnd, &screen);
                    if (known && !bar && folder >= 0 && folder < (int)g_flyout.folders.size()) {
                        // Onto a folder: in it from now on.
                        HiddenIcon icon = g_flyout.hidden[from];
                        PutInFolder(icon, g_flyout.folders[folder].setup);
                    } else if (known && !bar && stack >= 0 && stack < (int)g_flyout.hidden.size()) {
                        // Onto another icon: a new folder of the two.
                        HiddenIcon dragged = g_flyout.hidden[from], onto = g_flyout.hidden[stack];
                        StackIntoNewFolder(dragged, onto);
                    } else if (known && !bar && back) {
                        // Onto the open folder's way back: out of the folder.
                        HiddenIcon icon = g_flyout.hidden[from];
                        TakeOutOfFolder(icon);
                    } else if (bar && !key.empty()) {
                        // Onto the taskbar: it's in the tray now, off this list.
                        bool placed = SendTrayDrop(bar, hWnd, screen, key);
                        PostMessageW(bar, TrayDragOverMessage(), 0, 0);
                        if (placed) {
                            LoadOverflowFlyout();
                            LayoutFlyout(-1);
                        }
                    } else if (target >= 0 && target != from) {
                        // A drop anywhere else outside the grid puts it back.
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
            int hitType = id >> 16;
            int action = -1;
            NotifyItem const* note = hitType == kHitNotifyItem || hitType == kHitNotifyAction || hitType == kHitNotifyDismiss
                                         ? NotifyTargetItem(id & 0xFFFF, &action)
                                         : nullptr;
            if ((id >> 16) == kHitHidden) {
                ClickHiddenIcon(id & 0xFFFF, msg == WM_RBUTTONUP ? kClickRight : kClickMiddle);
            } else if (note && msg == WM_MBUTTONUP) {
                // A notification: a middle click clears it.
                NotifyItem copy = *note;
                ClearNotify(copy);
            } else if (note) {
                // Its menu: clear it or its app's, copy it, open its app, turn its app's off, the settings.
                NotifyItem copy = *note;
                POINT screen = pt;
                ClientToScreen(hWnd, &screen);
                ShowNotifyMenu(screen, copy);
                if (g_flyout.hwnd && IsWindow(g_flyout.hwnd)) SetForegroundWindow(g_flyout.hwnd);
            } else if (msg == WM_RBUTTONUP && (id >> 16) == kHitCalDay && g_settings.calendarAdd) {
                // A day in the calendar: a new event on it (the flyout closes as the window takes the focus).
                OpenEventEditor(CalendarGridStart(g_flyout.calYear, g_flyout.calMonth) + (id & 0xFFFF) * 24 * 60);
            } else if (msg == WM_RBUTTONUP && g_flyout.kind == kFlyoutOverflow &&
                       ((id >> 16) == kHitFolder || (id >> 16) == kHitFolderBack)) {
                // A folder (its tile, or its name while it's open): rename it, its glyph, delete it.
                POINT screen = pt;
                ClientToScreen(hWnd, &screen);
                ShowFolderMenu(screen, (id >> 16) == kHitFolder ? id & 0xFFFF : g_flyout.openFolder);
            } else if (msg == WM_RBUTTONUP && g_flyout.kind == kFlyoutOverflow) {
                // Off the icons: the flyout's own menu.
                POINT screen = pt;
                ClientToScreen(hWnd, &screen);
                ShowOverflowMenu(screen);
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
            if (wParam == kTimerMotion) {
                if (g_flyout.hwnd == hWnd) FlyoutOpeningStep(hWnd);
                else KillTimer(hWnd, kTimerMotion);
                return 0;
            }
            // Programs start and stop playing while the mixer is open.
            if (wParam == kTimerMixer && g_flyout.kind == kFlyoutMixer && g_flyout.dragMixer < 0) {
                LoadMixerFlyout();
                LayoutFlyout(-1);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            if (wParam == kTimerScanTick) {
                if (g_flyout.kind != kFlyoutSecurity || !g_scan.running) KillTimer(hWnd, kTimerScanTick);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            if (wParam == kTimerStackWait) {
                // Held over another icon a moment: dropped now, the two make a folder.
                KillTimer(hWnd, kTimerStackWait);
                if (g_flyout.dragHidden >= 0 && g_flyout.dragMoved && g_flyout.stackWait >= 0 &&
                    g_flyout.stackWait < (int)g_flyout.hidden.size()) {
                    g_flyout.dragStack = g_flyout.stackWait;
                    InvalidateRect(hWnd, nullptr, FALSE);
                }
                g_flyout.stackWait = -1;
            }
            if (wParam == kTimerHoverTip) {
                KillTimer(hWnd, kTimerHoverTip);
                std::vector<HiddenIcon> all;
                if (g_flyout.kind == kFlyoutOverflow && RequestHiddenIcons(&all, kRelayAllIcons)) {
                    for (auto& icon : g_flyout.hidden) {
                        for (auto const& now : all) {
                            if (!icon.plugin.empty() ? now.plugin == icon.plugin : now.owner == icon.owner && now.id == icon.id) {
                                icon.tip = now.tip;
                                if (!icon.plugin.empty()) icon.pixels = now.pixels;  // (a plugin's glyph changes with it)
                            }
                        }
                    }
                    InvalidateRect(hWnd, nullptr, FALSE);
                }
            }
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                CloseFlyout();
            }
            return 0;

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE && !g_flyoutClosing && !g_flyoutMenu) {
                if (g_stickyPid) {
                    return 0;  // After an icon click the sticky hooks decide.
                }
                g_flyoutClosedTick = GetTickCount();
                g_flyoutClosedKind = g_flyout.kind;
                CloseFlyout();
            }
            return 0;

        case WM_APP_FLYOUT_RENAME_DONE:
            // The name box lost the focus (a click elsewhere in the flyout): what's typed is kept.
            if (g_flyout.renameBox && GetFocus() != g_flyout.renameBox) EndFolderRename(true);
            return 0;

        case WM_CTLCOLOREDIT: {
            // The name box in the flyout's colours.
            static HBRUSH field = nullptr;
            static COLORREF fieldColour = 0;
            if (!field || fieldColour != ui::kRaised) {
                if (field) DeleteObject(field);
                fieldColour = ui::kRaised;
                field = CreateSolidBrush(fieldColour);
            }
            SetTextColor((HDC)wParam, ui::kInk);
            SetBkColor((HDC)wParam, ui::kRaised);
            return (LRESULT)field;
        }

        case WM_COPYDATA: {
            // An icon dragged here from the taskbar's tray.
            POINT screen;
            std::wstring key;
            if (g_flyout.kind == kFlyoutOverflow && ReadTrayDrop((COPYDATASTRUCT const*)lParam, &screen, &key)) {
                return DropOnFlyout(hWnd, screen, key);
            }
            return FALSE;
        }

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
                } else if (g_flyout.kind == kFlyoutBluetooth || g_flyout.kind == kFlyoutMixer) {
                    FinishBluetoothJob(result);
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
            KillTimer(hWnd, kTimerScanTick);
            KillTimer(hWnd, kTimerHoverTip);
            KillTimer(hWnd, kTimerMotion);
            if (g_flyout.hwnd == hWnd) EndFolderRename(true, false);  // (a name being typed is kept)
            StopSticky();
            ReleaseMixer();
            g_flyout.dragMixer = -1;
            EndHiddenDragOutside();
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
                g_flyoutShift = {};
                g_flyoutAlpha = 255;
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
    // The Tourne taskbar never takes the focus, so there the flyout is still
    // open when its icon is clicked again.
    TakeTaskbarAnchor();
    if (g_anchorFromTaskbar && g_flyout.hwnd && g_flyout.kind == flyoutKind) {
        CloseFlyout();
        return;
    }
    // Opened from an icon inside the hidden icons flyout (or one of its folders): placed by the bar as
    // that flyout was, not above the spot clicked in it, which floats off the bar.
    static RECT overflowAnchor;
    static bool overflowAnchored = false;
    if (!g_anchorFromTaskbar && overflowAnchored && g_flyout.hwnd && g_flyout.kind == kFlyoutOverflow &&
        flyoutKind != kFlyoutOverflow) {
        g_taskbarAnchor = overflowAnchor;
        g_anchorFromTaskbar = true;
    }
    if (flyoutKind == kFlyoutOverflow) {
        overflowAnchored = g_anchorFromTaskbar;
        if (g_anchorFromTaskbar) overflowAnchor = g_taskbarAnchor;
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
    if (g_anchorFromTaskbar) {
        anchor = g_taskbarAnchor;
        pt = {(anchor.left + anchor.right) / 2, (anchor.top + anchor.bottom) / 2};
    } else if (SUCCEEDED(Shell_NotifyIconGetRect(&identifier, &anchor))) {
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
    g_flyoutShift = {};
    g_flyoutAlpha = 255;
    g_flyout.scale = std::max(1, (int)(dpiX + USER_DEFAULT_SCREEN_DPI / 2) / USER_DEFAULT_SCREEN_DPI);
    g_flyout.width = 300 * g_flyout.scale;
    g_flyout.fitWidth = 0;
    g_flyout.hover = 0;
    g_flyout.dragSlider = -1;
    g_flyout.swallowUp = false;
    g_flyout.dragHidden = -1;
    g_flyout.dragMoved = false;
    g_flyout.dragFolder = g_flyout.dragStack = g_flyout.stackWait = -1;
    g_flyout.dragBack = false;
    g_flyout.renameBox = nullptr;
    g_flyout.openFolder = -1;  // opens on all the hidden icons
    g_flyout.selectedNetwork = -1;
    g_flyout.selectedSsid.clear();
    g_flyout.networkScroll = 0;
    g_flyout.btSelected.clear();
    g_flyout.btBusy.clear();
    g_flyout.btNote.clear();
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
        if (g_scan.running) SetTimer(hwnd, kTimerScanTick, 1000, nullptr);
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
    StartFlyoutOpening(hwnd);
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

// TODO: with a scheme taken from a theme (taskbarStyle, visualStyle), other
// apps' icons themed here only take new colours on a settings change, not
// when Windows' visual style, dark or light mode or accent colour changes.
// This side should react to WM_THEMECHANGED and "ImmersiveColorSet" too (or
// the tool process could tell it) and call ExplorerSideSettingsChanged(true).

void ExplorerSideSettingsChanged(bool loadSettings) {
    if (loadSettings) {
        auto loaded = LoadSettings();
        AcquireSRWLockExclusive(&g_settingsLock);
        g_settings = std::move(loaded);
        ReleaseSRWLockExclusive(&g_settingsLock);
    }
    g_themeEnabled = g_settings.trayTheme;
    g_explorerSideTray = nullptr;  // Theme the existing icons again.
    if (!g_settings.tourneTooltips) {
        StopTooltipStyle();
    }
}

// Programs whose state lives in a settings file get their icon redrawn when
// the file changes, and every icon when Tourne'Icons changes its Taskbar
// buttons (HKCU\Software\Tourne\Icons). One thread, asleep until then.
HANDLE g_appStateThread;
HANDLE g_appStateStop;

DWORD WINAPI AppStateThread(LPVOID) {
    HANDLE watch = FindFirstChangeNotificationW(TamerSettingsFolder().c_str(), FALSE,
                                                FILE_NOTIFY_CHANGE_LAST_WRITE);
    HKEY icons = nullptr;
    RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Icons", 0, nullptr, 0, KEY_NOTIFY, nullptr, &icons,
                    nullptr);
    HANDLE iconsChanged = icons ? CreateEventW(nullptr, FALSE, FALSE, nullptr) : nullptr;
    auto watchIcons = [&] {
        return iconsChanged && RegNotifyChangeKeyValue(icons, FALSE, REG_NOTIFY_CHANGE_LAST_SET, iconsChanged, TRUE) ==
                                   ERROR_SUCCESS;
    };
    bool watchingIcons = watchIcons();
    std::vector<HANDLE> events{g_appStateStop};
    if (watch != INVALID_HANDLE_VALUE) events.push_back(watch);
    if (watchingIcons) events.push_back(iconsChanged);
    while (events.size() > 1) {
        DWORD woke = WaitForMultipleObjects((DWORD)events.size(), events.data(), FALSE, INFINITE);
        if (woke == WAIT_OBJECT_0 || woke >= WAIT_OBJECT_0 + events.size()) {
            break;
        }
        // Programs save in more than one write; redraw once it has settled.
        if (WaitForSingleObject(g_appStateStop, 200) == WAIT_OBJECT_0) {
            break;
        }
        if (g_themeEnabled) {
            ThemeExistingIcons();
        }
        if (events[woke - WAIT_OBJECT_0] == iconsChanged) {
            watchIcons();
        } else {
            FindNextChangeNotification(watch);
        }
    }
    if (watch != INVALID_HANDLE_VALUE) FindCloseChangeNotification(watch);
    if (iconsChanged) CloseHandle(iconsChanged);
    if (icons) RegCloseKey(icons);
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
    } else if (tray) {
        ThemeMissedIcons();
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
    ClearHostedIcons();
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
        case kBluetooth: ShowFlyout(kFlyoutBluetooth, kBluetooth); break;
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
        case kVolume: {
            std::vector<MenuItem> items = {{L"Open Volume mixer", [] { Run(L"SndVol.exe"); }},
                                           {L"Sound settings", Uri(L"ms-settings:sound")}};
            for (auto& spatial : SpatialSoundMenu()) {
                items.push_back(std::move(spatial));
            }
            items.push_back({});
            items.push_back({L"Playback devices", [] { Run(L"control.exe", L"mmsys.cpl,,0"); }});
            items.push_back({L"Recording devices", [] { Run(L"control.exe", L"mmsys.cpl,,1"); }});
            items.push_back({L"Sounds", [] { Run(L"control.exe", L"mmsys.cpl,,2"); }});
            ShowMenu(items, anchor);
            break;
        }
        case kBattery:
            ShowMenu({{L"Power && battery settings", Uri(L"ms-settings:powersleep")},
                      {L"Battery saver", Uri(L"ms-settings:batterysaver")}},
                     anchor);
            break;
        case kNotifications:
            ShowMenu({{L"Open notification center", [] { ShowFlyout(kFlyoutNotify, -1); }},  // (Tourne's: Win+N's is hidden in shell mode)
                      {L"Notification settings", Uri(L"ms-settings:notifications")}},
                     anchor);
            break;
        case kFocus:
            ShowMenu({{L"Open notification center", [] { ShowFlyout(kFlyoutNotify, -1); }},  // (Tourne's: Win+N's is hidden in shell mode)
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
        case kBluetooth:
            // When Windows' own Bluetooth icon is gone and its menu with it.
            ShowMenu({{L"Bluetooth settings", Uri(L"ms-settings:bluetooth")},
                      {L"Add a device", Uri(L"ms-settings-connectabledevices:devicediscovery")},
                      {},
                      {L"Send a file", [] { Run(L"fsquirt.exe", L"-send"); }},
                      {L"Receive a file", [] { Run(L"fsquirt.exe", L"-receive"); }}},
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

// A click that came from the Tourne taskbar's tray (it left an anchor here
// just now). Windows' own icons live in Explorer's tray, so their menus would
// open there, not by the icon that was clicked.
bool ClickFromTaskbar() {
    auto tick = (DWORD)(ULONG_PTR)GetPropW(g_wnd, kTaskbarAnchorTickProp);
    return GetPropW(g_wnd, kTaskbarAnchorProp) && GetTickCount() - tick <= 1500;
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
    if (secondary && !ClickFromTaskbar() &&
        (kind == kNetwork || kind == kVolume || kind == kBattery || kind == kSecurity ||
         kind == kBluetooth)) {
        if (RelayForward(kind, event, anchor)) {
            return;
        }
    }

    if (primary) {
        OnPrimary(kind, anchor);
    } else if (event == WM_CONTEXTMENU) {
        OnContextMenu(kind, anchor);
    }
}

// A device battery icon: the devices flyout, or a menu.
void OnBatteryTrayEvent(int slot, UINT event, WPARAM anchor) {
    if (event == NIN_SELECT || event == NIN_KEYSELECT) {
        ShowFlyout(kFlyoutDevices, kBatteryIdBase + slot);
    } else if (event == WM_CONTEXTMENU) {
        ShowMenu({{L"Check now", [] { WakeBatteries(); }},
                  {L"Bluetooth settings", Uri(L"ms-settings:bluetooth")}},
                 anchor);
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

    OpenWlan();
}

// Windows' Wi-Fi service, and its notifications. Opened again when a call fails: a handle from before
// the service restarted (an update, waking from sleep) fails every call after it, which left the flyout
// with no networks but the one connected.
void OpenWlan() {
    if (g_wlan) {
        WlanRegisterNotification(g_wlan, WLAN_NOTIFICATION_SOURCE_NONE, TRUE, nullptr, nullptr, nullptr, nullptr);
        WlanCloseHandle(g_wlan, nullptr);
        g_wlan = nullptr;
    }
    DWORD version;
    if (WlanOpenHandle(2, nullptr, &version, &g_wlan) == ERROR_SUCCESS) {
        WlanRegisterNotification(
            g_wlan, WLAN_NOTIFICATION_SOURCE_ACM | WLAN_NOTIFICATION_SOURCE_MSM,
            TRUE, OnWlanNotification, nullptr, nullptr, nullptr);
    } else {
        g_wlan = nullptr;
    }
}

// The Wi-Fi interfaces, opening the service again once if the handle has gone stale.
WLAN_INTERFACE_INFO_LIST* WlanInterfaces() {
    WLAN_INTERFACE_INFO_LIST* interfaces = nullptr;
    if (g_wlan && WlanEnumInterfaces(g_wlan, nullptr, &interfaces) == ERROR_SUCCESS) {
        return interfaces;
    }
    OpenWlan();
    if (g_wlan && WlanEnumInterfaces(g_wlan, nullptr, &interfaces) == ERROR_SUCCESS) {
        return interfaces;
    }
    return nullptr;
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

// Runs in the suite's own tray host process, so any taskbar will do; the
// icons go to whichever Explorer is hosting it.
bool IsTaskbarProcess() {
    return FindWindowW(L"Shell_TrayWnd", nullptr) != nullptr;
}

bool g_started;

void Start(HWND hWnd) {
    g_started = true;
    StartNetworkWatch();
    StartAudio();
    StartRegistryWatches();
    StartWindowWatches(hWnd);
    WscRegisterForChanges(nullptr, &g_securityWatch, OnSecurityChange, nullptr);
    StartRadioWatch();
    StartDeviceWatch();
    WarmBrightnessCache(hWnd);
    g_popupSeen = NowFileTime();  // (pop-ups only for ones arriving from now on)
    g_notificationsWatch = FindFirstChangeNotificationW(
        NotificationsFolder().c_str(), FALSE,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_FILE_NAME);
    OpenNotifyCounter();
    SetTimer(hWnd, kTimerNotifyCounter, 1000, nullptr);
    RefreshCalendar(false);
    LoadBatterySlots();
    StartBatteries();
    SetTimer(hWnd, kTimerPeriodic, kPeriodicMs, nullptr);
    RequestRefresh(kAllBits);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Scroll and middle clicks arrive from the Explorer relay.
    if (g_msgRelayWheel && msg == g_msgRelayWheel) {
        msg = WM_APP_WHEEL;
    } else if (g_msgRelayMiddle && msg == g_msgRelayMiddle) {
        msg = WM_APP_MIDDLE;
    }

    // A bind's "flyout" (binds.cpp): one of these flyouts opened, or closed again if it's open. wParam: its kind.
    static const UINT openFlyout = RegisterWindowMessageW(L"TourneTray_Flyout");
    if (msg == openFlyout && openFlyout) {
        int kind = (int)wParam;
        if (kind > kFlyoutNone && kind <= kFlyoutDevices) {
            if (IsFlyoutOpen(kind)) CloseFlyout();
            else ShowFlyout(kind, kind == kFlyoutMixer ? kVolume : -1);
        }
        return 0;
    }

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
        } else if (kind >= (int)kBatteryIdBase && kind < (int)kBatteryIdBase + kBatterySlots) {
            OnBatteryTrayEvent(kind - (int)kBatteryIdBase, event, wParam);
        }
        return 0;
    }

    if (g_taskbarCreatedMessage && msg == g_taskbarCreatedMessage) {
        for (auto& applied : g_applied) {
            applied = {};
        }
        // The new tray has none of them; Refresh adds them back.
        for (auto& icon : g_batteryIcons) {
            icon = {};
        }
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

    // Tourne Desktop made new theme colours.
    if (msg == TourneThemeMessage() && msg) {
        if (g_followsTourneTheme) PostMessageW(hWnd, WM_APP_SETTINGS, 0, 0);
        return 0;
    }

    switch (msg) {
        // The Tourne-styled popup menus (ShowMenu).
        case WM_MEASUREITEM:
            if (MenuMeasureItem((MEASUREITEMSTRUCT*)lParam)) {
                return TRUE;
            }
            break;

        case WM_DRAWITEM:
            if (MenuDrawItem((DRAWITEMSTRUCT*)lParam)) {
                return TRUE;
            }
            break;

        case WM_APP_REFRESH:
            g_refreshPosted = false;
            if (g_started) {
                Refresh(g_pendingBits.exchange(0));
            }
            return 0;

        case WM_APP_SETTINGS: {
            auto loaded = LoadSettings();
            AcquireSRWLockExclusive(&g_settingsLock);
            g_settings = std::move(loaded);
            ReleaseSRWLockExclusive(&g_settingsLock);
            LoadMotion();
        }
            if (g_started) {
                OnBatterySettings();
            }
            RequestRefresh(kAllBits);
            return 0;

        case WM_APP_BATTERIES:
            if (g_started) {
                OnBatteriesFound();
            }
            return 0;

        case WM_COPYDATA: {
            auto* copyData = (const COPYDATASTRUCT*)lParam;
            if (copyData && copyData->dwData == kHiddenIconsCopyId) {
                g_receivedHidden =
                    UnpackHiddenIcons((const BYTE*)copyData->lpData, copyData->cbData);
                g_hiddenReceived = true;
                return TRUE;
            }
            // Tourne Desktop's plugin items, from its plugin runtime (AddPluginIcons asked).
            wchar_t from[64];
            if (copyData && copyData->dwData == kPluginBarItemsId && copyData->lpData &&
                GetClassNameW((HWND)wParam, from, 64) && !wcscmp(from, kPluginHostClass)) {
                auto const* text = (wchar_t const*)copyData->lpData;
                unsigned version = 0;
                g_pluginItems = UnpackPluginBarItems(std::wstring_view(text, wcsnlen(text, copyData->cbData / sizeof(wchar_t))), &version);
                g_pluginVersion = version;
                return TRUE;
            }
            break;
        }

        case WM_APP_CALENDAR_READY:
            g_calendarFetching = false;
            if (IsFlyoutOpen(kFlyoutNotify)) {
                PostMessageW(g_flyout.hwnd, WM_APP_FLYOUT_RELOAD, 0, 0);
            }
            return 0;

        case WM_APP_BRIGHTNESS_READY:
            FinishBrightnessWarm();
            ApplyBrightnessWheel();
            return 0;

        case WM_APP_BIN:
            RequestRefresh(1u << kRecycleBin);
            return 0;

        case WM_APP_WHEEL: {
            // lParam: the icon scrolled over, plus one (0, from older relays,
            // is the volume icon).
            g_wheelRemainder += (int)(short)wParam;
            int notches = g_wheelRemainder / WHEEL_DELTA;
            g_wheelRemainder -= notches * WHEEL_DELTA;
            int kind = lParam ? (int)lParam - 1 : kVolume;
            if (kind == kVolume) {
                ChangeVolume(notches);
            } else if (kind == kBrightness) {
                WheelBrightness(notches);
            }
            return 0;
        }

        case WM_APP_MIDDLE:
            if (!lParam || (int)lParam - 1 == kVolume) {
                OnMiddleClick();
            }
            return 0;

        case WM_APP_AUDIO_DEVICE:
            BindAudioEndpoint();
            // A Bluetooth speaker or headset connecting or disconnecting.
            RequestRefresh((1u << kVolume) | (1u << kBluetooth));
            return 0;

        case WM_POWERBROADCAST:
            if (wParam == PBT_POWERSETTINGCHANGE || wParam == PBT_APMPOWERSTATUSCHANGE) {
                RequestRefresh(1u << kBattery);
            } else if (wParam == PBT_APMRESUMEAUTOMATIC) {
                RequestRefresh(kAllBits);
            }
            return TRUE;

        case WM_THEMECHANGED:
            // Another visual style: its colours, for the schemes taken from it.
            if (IsThemeScheme(g_settings.scheme)) {
                PostMessageW(hWnd, WM_APP_SETTINGS, 0, 0);
            }
            return 0;

        case WM_SETTINGCHANGE:
            // Dark or light mode, or the accent colour.
            if (lParam && !wcscmp((PCWSTR)lParam, L"ImmersiveColorSet") && IsThemeScheme(g_settings.scheme)) {
                PostMessageW(hWnd, WM_APP_SETTINGS, 0, 0);
                return 0;
            }
            [[fallthrough]];
        case WM_DISPLAYCHANGE:
            // Re-renders only if the tray icon size changed with the DPI.
            RequestRefresh(0);
            return 0;

        case WM_TIMER:
            if (wParam == kTimerNotifications) {
                KillTimer(hWnd, kTimerNotifications);
                RequestRefresh(1u << kNotifications);
                CheckNewNotifications();
                return 0;
            }
            if (wParam == kTimerNotifyCounter) {
                if (NotifyCounterMoved()) SetTimer(hWnd, kTimerNotifications, 250, nullptr);
                return 0;
            }
            if (wParam == kTimerBreath) {
                g_breathFrame ^= 1;
                ApplyBatteryIcons();
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
                RequestRefresh((1u << kBattery) | (1u << kRecycleBin) |
                               kNetworkBits | (1u << kBluetooth));
            }
            return 0;

        case WM_APP_SCAN_DONE:
            FinishDefenderScan((int)wParam, (DWORD)lParam);
            // The flyout, if it's showing: the result, and Windows' new last-scan time.
            if (g_flyout.hwnd && g_flyout.kind == kFlyoutSecurity) {
                LoadSecurityFlyout();
                LayoutFlyout(-1);
                InvalidateRect(g_flyout.hwnd, nullptr, FALSE);
            }
            return 0;

        case WM_APP_EXIT:
            CloseFlyout();
            for (int k = 0; k < kIconCount; k++) {
                RemoveIcon(k);
            }
            RemoveBatteryIcons();
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, kTimerPeriodic);
            KillTimer(hWnd, kTimerFindTray);
            KillTimer(hWnd, kTimerBreath);
            if (g_started) {
                StopBatteries();
                StopWindowWatches(hWnd);
                if (g_securityWatch) {
                    WscUnRegisterChanges(g_securityWatch);
                    g_securityWatch = nullptr;
                }
                StopDeviceWatch();
                StopRadioWatch();
                FinishBrightnessWarm();
                StopBrightnessWheel();
                if (g_notificationsWatch != INVALID_HANDLE_VALUE) {
                    FindCloseChangeNotification(g_notificationsWatch);
                    g_notificationsWatch = INVALID_HANDLE_VALUE;
                }
                KillTimer(hWnd, kTimerNotifyCounter);
                CloseNotifyCounter();
            }
            CloseAllPopups();
            EndStills();
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

    // Explorer gave these windows per-monitor DPI for free; a process of its
    // own has to ask, or flyouts and icons are scaled blurry.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterRelayMessages();

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

    WNDCLASSW stillClass{};
    stillClass.lpfnWndProc = StillProc;
    stillClass.hInstance = instance;
    stillClass.lpszClassName = kStillClass;
    RegisterClassW(&stillClass);

    {
        auto loaded = LoadSettings();
        AcquireSRWLockExclusive(&g_settingsLock);
        g_settings = std::move(loaded);
        ReleaseSRWLockExclusive(&g_settingsLock);
    }
    LoadMotion();
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
                    OpenNotifyCounter();  // (the -shm file may have been made anew)
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
                if (EventDialogMessage(&msg)) {
                    continue;
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
    UnregisterClassW(kStillClass, instance);
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
    // A scan from the flyout carries on in Defender; only the wait for it ends.
    StopDefenderScanWait();
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
