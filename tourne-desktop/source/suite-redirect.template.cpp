// ==WindhawkMod==
// @id              tourne-suite-redirect-branch-wintweaks
// @name            Tourne'Suite (redirect test)
// @description     Test build of Tourne'Suite with Icon Redirect, which themes icons in place of the Resource Redirect mod
// @version         1.0.0-redirect.4
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         *
// @include         windhawk.exe
// @exclude         vgc.exe
// @exclude         TourneDesktop.exe
// @compilerOptions -lshell32 -lgdi32 -lcomctl32 -lversion -ladvapi32 -lole32 -loleaut32 -lwlanapi -liphlpapi -lpowrprof -luuid -lmsimg32 -ldxva2 -luser32 -luxtheme -lwscapi -lcfgmgr32 -ldwmapi -lshlwapi -lruntimeobject -lurlmon -lwininet -lhid -ld3d11 -ldxgi -ld2d1 -ldwrite
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Suite`

Every way to theme Windows in one place: one mod made of parts, each switched
on or off on its own.

## It starts from what you already have

- **Your settings are the defaults.** Loading the suite doesn't change your
  Windows settings. Tweaks starts from what's already set on your PC (by you,
  by Winaero Tweaker, or by Windows), the taskbar from StartAllBack's
  settings, and the switcher from Simple Window Switcher's. Where nothing was
  ever set, Windows' own value stays, or nothing is applied.
- **Only what you change is applied**, and only when you change it: each save
  applies the values that differ from before, nothing else.
- **Themed to your visual style.** The parts that draw their own look (the
  taskbar, its menus and tooltips, the switcher) follow the `.msstyles` you
  use and its colours, unless you already have colours of your own set
  somewhere (like Simple Window Switcher's).

## The parts

| Part | What it does | Runs in |
|---|---|---|
| System Icons | Pixel-art tray icons with live state, and a flyout for each | Its own process (`windhawk.exe -tool-mod`) |
| Taskbar (test) | Our own classic taskbar, drawn from StartAllBack themes | The same process, off until switched on |
| Switcher (Alt+Tab) | Our own Alt+Tab, in the suite's style | The same process, off until switched on |
| Explorer relay | The jobs that have to happen inside Explorer | Explorer, while System Icons is on |
| Classic Tray Folders, Clock | Tray folders; the classic clock restyled | Explorer |
| Change Tray Icons | Any app's tray icon swapped for a themed one | Apps that have a row in its list |
| System Text Color, Menu No Icons, Menu No Duplicates | Text colour; menus without icons or doubles | Their Processes lists |
| Icon Redirect | Icon themes and single-app icon swaps | Every program, while it has a theme or a row |
| Tweaks | Winaero Tweaker's options, 312 of them | Windhawk's own process; every program for the live font swap |

**Taskbar (test)** runs beside StartAllBack's taskbar while it's tested: pinned
apps, grouping in the same order as StartAllBack, jump lists (right-click; the
app's own Windows menu on right-clicking it there), live window previews, the
tray with every icon, the clock, and segments. Its visual style and orb follow
StartAllBack's unless you pick others.

**Switcher (Alt+Tab)** replaces Simple Window Switcher, with all its settings.
Its colours start as Simple Window Switcher's if you've used it, otherwise
your visual style's. Turn Simple Window Switcher off while it's on.

**Tweaks** starts from this PC's settings, so turning it on changes nothing.
Changes for your account apply straight away. Machine-wide ones go into one
step that asks for admin once per save; if you decline, they wait for the next
save.

Icon themes picked in Icon Redirect are downloaded by Windhawk's own process,
once each; its settings start as Resource Redirect's.

A problem in System Icons can no longer take Explorer down, and its icons come
back by themselves when Explorer restarts.

The **Explorer relay** does the two jobs that have to happen inside Explorer:
it passes right clicks on network, volume and battery to the system icons they
stand in for, and it sends scrolling and middle clicks over the volume icon to
the System Icons process.

Because the icons now belong to a different program, Windows treats them as new
tray icons: turn them on again in **Settings > Personalization > Taskbar >
Other system tray icons**, where they're listed as Windhawk.

Turn off Tourne'Suite, the split test build, the separate Tourne mods and
Resource Redirect while this build is on.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
{{SETTINGS}}
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
#include <commctrl.h>
#include <dwmapi.h>
#include <cfgmgr32.h>
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
#include <psapi.h>
#include <physicalmonitorenumerationapi.h>
#include <powrprof.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <wlanapi.h>
#include <wuapi.h>
#include <tlhelp32.h>
#include <urlmon.h>
#include <wininet.h>
#include <wscapi.h>
#include <windowsx.h>
#include <wincodec.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <dwrite.h>
#include <propsys.h>

#include <stddef.h>
#include <wchar.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cwchar>
#include <cwctype>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Power.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.System.Power.h>
#include <winrt/Windows.UI.Notifications.h>

////////////////////////////////////////////////////////////////////////////////
// Suite: shared hooks
//
// Parts register their hooks here rather than with Windhawk. One mod can't
// hook the same function twice, and two parts can want the same one (System
// Text Color and Classic Tray Folders both hook CreateWindowExW in Explorer).
// Hooks on one function are chained: the function calls the last part's hook,
// whose "original" is the previous part's hook, and so on down to Windhawk's
// trampoline, so every part runs just as it did as a separate mod.

struct SuiteHook {
    void* hook;
    void** original;
    int part;
};

struct SuiteHookChain {
    void* target;
    std::vector<SuiteHook> hooks;
};

[[clang::no_destroy]] std::vector<SuiteHookChain> g_suiteHookChains;
bool g_suiteCollectingHooks = true;
int g_suiteCurrentPart = -1;

BOOL SuiteSetFunctionHook(void* target, void* hook, void** original) {
    if (!target) {
        return FALSE;
    }
    if (!g_suiteCollectingHooks) {
        return Wh_SetFunctionHook(target, hook, original);
    }
    for (auto& chain : g_suiteHookChains) {
        if (chain.target == target) {
            chain.hooks.push_back({hook, original, g_suiteCurrentPart});
            return TRUE;
        }
    }
    g_suiteHookChains.push_back({target, {{hook, original, g_suiteCurrentPart}}});
    return TRUE;
}

template <typename Prototype>
BOOL SuiteSetFunctionHookT(Prototype* target, Prototype* hook, Prototype** original) {
    return SuiteSetFunctionHook((void*)target, (void*)hook, (void**)original);
}

// A part that failed to start leaves nothing hooked behind.
void SuiteDropHooks(int part) {
    for (auto& chain : g_suiteHookChains) {
        std::erase_if(chain.hooks, [part](SuiteHook const& h) { return h.part == part; });
    }
    std::erase_if(g_suiteHookChains, [](SuiteHookChain const& c) { return c.hooks.empty(); });
}

void SuiteCommitHooks() {
    for (auto& chain : g_suiteHookChains) {
        auto& hooks = chain.hooks;
        for (size_t k = 1; k < hooks.size(); k++) {
            *hooks[k].original = hooks[k - 1].hook;
        }
        if (!Wh_SetFunctionHook(chain.target, hooks.back().hook, hooks.front().original)) {
            Wh_Log(L"Hooking %p failed", chain.target);
        }
    }
    g_suiteHookChains.clear();
    g_suiteCollectingHooks = false;
}

{{PARTS}}

////////////////////////////////////////////////////////////////////////////////
// Explorer relay: the half of System Icons that has to live in Explorer
//
// System Icons runs in the suite's tool process and can't reach into the
// tray's toolbars from there. This part subclasses them, then:
//   * replays right clicks the host asks for on the system icon behind each
//     stand-in, which has to happen here because those icons live in Explorer;
//   * sends scrolling and middle clicks over the host's volume icon to the
//     host, since tray icons never receive those themselves.

#ifdef _WIN64
bool SuiteIsShellExplorer();  // (with the crash guard, below)

namespace part_explorer_relay {

// Must match part_system_icons.
constexpr UINT kIconIdBase = 0x7E00;
constexpr int kNetwork = 0;
constexpr int kBattery = 2;
constexpr int kVolume = 16;
constexpr int kSecurity = 18;
constexpr int kBluetooth = 19;
constexpr int kHostIconCount = 20;
constexpr WCHAR kHostClassPrefix[] = L"TourneTraySystemIcons_";
// The test taskbar's tray zone asks for the icons too.
constexpr WCHAR kTaskbarClassPrefix[] = L"TourneTaskbarTest_";

UINT g_msgForward;
UINT g_msgWheel;
UINT g_msgMiddle;
UINT g_msgOverflow;

std::vector<HWND> TrayToolbars() {
    std::vector<HWND> toolbars;
    if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr)) {
        if (HWND notify = FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr)) {
            EnumChildWindows(
                notify,
                [](HWND hWnd, LPARAM lParam) __stdcall -> BOOL {
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

// Each tray button's data holds the owner window at +0, the id at +8, the
// callback message at +12 and the NOTIFYICON version at +20.
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
            if (!SendMessageW(toolbar, TB_GETBUTTON, i, (LPARAM)&button) || !button.dwData) {
                continue;
            }
            auto* data = (const BYTE*)button.dwData;
            HWND owner = *(const HWND*)data;
            UINT ownerId = *(const UINT*)(data + 8);
            WCHAR className[64];
            if (!IsWindow(owner) || !GetClassNameW(owner, className, ARRAYSIZE(className)) ||
                !match(owner, ownerId, className)) {
                continue;
            }
            *target = {owner, ownerId, *(const UINT*)(data + 12), *(const UINT*)(data + 20)};
            return true;
        }
    }
    // Without a classic tray, the icons the suite hosts.
    bool found = false;
    for (auto& h : part_system_icons::HostedIcons()) {
        WCHAR className[64];
        if (!found && h.callback && GetClassNameW(h.owner, className, ARRAYSIZE(className)) &&
            match(h.owner, h.id, className)) {
            *target = {h.owner, h.id, h.callback, h.version};
            found = true;
        }
        if (h.icon) {
            DestroyIcon(h.icon);
        }
    }
    return found;
}

bool OwnedByThisProcess(HWND hWnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    return pid == GetCurrentProcessId();
}

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
        case kSecurity:
            // Windows Security's own icon: also an ATL window with id 100, in
            // its own process.
            return FindTrayTarget(
                [](HWND owner, UINT id, PCWSTR cls) {
                    return id == 100 && wcsncmp(cls, L"ATL:", 4) == 0 &&
                           part_system_icons::OwnerFileName(owner) == L"securityhealthsystray.exe";
                },
                target);
        case kBluetooth:
            // Windows' own Bluetooth icon, shown while "Show the Bluetooth
            // icon in the notification area" is on.
            return FindTrayTarget(
                [](HWND, UINT, PCWSTR cls) {
                    return _wcsicmp(cls, L"BluetoothNotificationAreaIconWindowClass") == 0;
                },
                target);
    }
    return false;
}

// Replays a tray event on another icon, in that icon's NOTIFYICON version.
void ForwardEvent(TrayTarget const& target, UINT event, WPARAM anchor) {
    if (target.version >= NOTIFYICON_VERSION_4) {
        PostMessageW(target.hwnd, target.callback, anchor, MAKELPARAM(event, target.id));
        return;
    }
    bool mouse = event == WM_LBUTTONDOWN || event == WM_LBUTTONUP ||
                 event == WM_RBUTTONDOWN || event == WM_RBUTTONUP ||
                 event == WM_LBUTTONDBLCLK || event == WM_MOUSEMOVE;
    bool v3 = target.version == NOTIFYICON_VERSION &&
              (event == NIN_SELECT || event == NIN_KEYSELECT || event == WM_CONTEXTMENU);
    if (mouse || v3) {
        PostMessageW(target.hwnd, target.callback, target.id, event);
    }
}

// The host's icon under a toolbar point, or null; its kind in *kind.
HWND HostIconAt(HWND toolbar, POINT clientPoint, int* kind) {
    int index = (int)SendMessageW(toolbar, TB_HITTEST, 0, (LPARAM)&clientPoint);
    if (index < 0) {
        return nullptr;
    }
    TBBUTTON button{};
    if (!SendMessageW(toolbar, TB_GETBUTTON, index, (LPARAM)&button) || !button.dwData) {
        return nullptr;
    }
    auto* data = (const BYTE*)button.dwData;
    HWND owner = *(const HWND*)data;
    UINT id = *(const UINT*)(data + 8);
    WCHAR className[64];
    if (id < kIconIdBase || id >= kIconIdBase + kHostIconCount || !IsWindow(owner) ||
        !GetClassNameW(owner, className, ARRAYSIZE(className)) ||
        wcsncmp(className, kHostClassPrefix, ARRAYSIZE(kHostClassPrefix) - 1) != 0) {
        return nullptr;
    }
    *kind = (int)(id - kIconIdBase);
    return owner;
}

// Pins or unpins through the item's own "Pin to taskbar" command, which works
// only in Explorer.
void PinToTaskbar(std::wstring const& path, bool pin) {
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (FAILED(SHParseDisplayName(path.c_str(), nullptr, &pidl, 0, nullptr))) {
        return;
    }
    IShellItem* item = nullptr;
    if (SUCCEEDED(SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&item)))) {
        IContextMenu* menu = nullptr;
        if (SUCCEEDED(item->BindToHandler(nullptr, BHID_SFUIObject, IID_PPV_ARGS(&menu)))) {
            HMENU popup = CreatePopupMenu();
            if (SUCCEEDED(menu->QueryContextMenu(popup, 0, 1, 0x7FFF, CMF_NORMAL))) {
                CMINVOKECOMMANDINFOEX info{sizeof(info)};
                info.fMask = CMIC_MASK_UNICODE | CMIC_MASK_FLAG_NO_UI;
                info.lpVerb = pin ? "taskbarpin" : "taskbarunpin";
                info.lpVerbW = pin ? L"taskbarpin" : L"taskbarunpin";
                info.nShow = SW_SHOWNORMAL;
                HRESULT hr = menu->InvokeCommand((CMINVOKECOMMANDINFO*)&info);
                Wh_Log(L"%ls %ls: %08X", pin ? L"Pin" : L"Unpin", path.c_str(), hr);
            }
            DestroyMenu(popup);
            menu->Release();
        }
        item->Release();
    }
    CoTaskMemFree(pidl);
}

// Progress on taskbar buttons (ITaskbarList3::SetProgressState/Value). Apps
// post it to Explorer's task switcher window (MSTaskSwWClass), as a probe of
// ExplorerFrame's TaskbarList showed: WM_USER+0x41 with the state (TBPF_*),
// WM_USER+0x40 with the value scaled to 0..65535, the app's window in wParam.
// It's kept here and passed on to the suite's taskbars, which ask for all of
// it when they start.
constexpr UINT kTaskSwProgressValue = WM_USER + 0x40;
constexpr UINT kTaskSwProgressState = WM_USER + 0x41;
// The suite's taskbars, this build's or Tourne Desktop's (whose class ends in
// its own id, not this mod's): found by the class's prefix.
bool IsSuiteBar(HWND wnd) {
    WCHAR className[64];
    return IsWindow(wnd) && GetClassNameW(wnd, className, ARRAYSIZE(className)) &&
           wcsncmp(className, kTaskbarClassPrefix, ARRAYSIZE(kTaskbarClassPrefix) - 1) == 0;
}

BOOL CALLBACK CollectSuiteBar(HWND wnd, LPARAM param) {
    if (IsSuiteBar(wnd)) {
        ((std::vector<HWND>*)param)->push_back(wnd);
    }
    return TRUE;
}

std::vector<HWND> SuiteBars() {
    std::vector<HWND> bars;
    EnumWindows(CollectSuiteBar, (LPARAM)&bars);
    return bars;
}

struct ProgressEntry {
    int state = 0;  // TBPF_*
    int value = 0;  // 0..65535
};
std::mutex g_progressMutex;
std::unordered_map<HWND, ProgressEntry> g_progressTable;
UINT g_msgProgress;
UINT g_msgProgressQuery;

void PostProgress(HWND bar, HWND window, ProgressEntry const& entry) {
    PostMessageW(bar, g_msgProgress, (WPARAM)window, MAKELPARAM(entry.value, entry.state));
}

void OnProgressMessage(UINT uMsg, HWND window, LPARAM lParam) {
    ProgressEntry entry;
    {
        std::lock_guard lock(g_progressMutex);
        std::erase_if(g_progressTable, [](auto const& e) { return !IsWindow(e.first); });
        entry = g_progressTable[window];
        if (uMsg == kTaskSwProgressState) {
            entry.state = (int)lParam & (TBPF_INDETERMINATE | TBPF_NORMAL | TBPF_ERROR | TBPF_PAUSED);
        } else {
            entry.value = (int)std::clamp<LPARAM>(lParam, 0, 0xFFFF);
            // A value shown while there was none, or while busy, is normal
            // progress (as ITaskbarList3 documents).
            if (entry.state == TBPF_NOPROGRESS || entry.state == TBPF_INDETERMINATE) {
                entry.state = TBPF_NORMAL;
            }
        }
        if (entry.state == TBPF_NOPROGRESS) {
            g_progressTable.erase(window);
        } else {
            g_progressTable[window] = entry;
        }
    }
    for (HWND bar : SuiteBars()) {
        PostProgress(bar, window, entry);
    }
}

extern std::atomic<bool> g_hiding;
extern std::atomic<bool> g_taskbarViewHooked;

// While Windows 11's XAML taskbar is kept hidden the progress stops here: its task button would build a
// progress bar for it, and that progress bar fails Explorer (its template asks WinUI 2 for
// ProgressBarTemplateSettings, which WinUI 2 doesn't make by name, and the next state change fails
// fast). The suite's taskbar shows it instead; once hiding ends, the next update reaches Explorer again.
LRESULT CALLBACK TaskSwSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD_PTR) {
    if ((uMsg == kTaskSwProgressState || uMsg == kTaskSwProgressValue) && wParam) {
        OnProgressMessage(uMsg, (HWND)wParam, lParam);
        if (g_hiding && g_taskbarViewHooked) {
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Pin requests wait here for the tray's thread, off the sender's call.
std::mutex g_pinMutex;
std::vector<std::wstring> g_pinRequests;
UINT g_msgPinNow;

// The requests from the suite's own windows, answered on whichever of the
// tray's windows they reach: a toolbar of the classic tray, or the taskbar
// itself when there's none.
bool HandleRelayRequest(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* result) {
    *result = 0;
    // The taskbar asks to pin or unpin an app. The shell is called once the
    // taskbar's call has returned, since it can't be called during one.
    if (uMsg == WM_COPYDATA) {
        auto* copyData = (const COPYDATASTRUCT*)lParam;
        HWND sender = (HWND)wParam;
        WCHAR className[64];
        if (copyData && copyData->dwData == part_system_icons::kRelayPinCopyId && copyData->lpData &&
            copyData->cbData >= 2 * sizeof(WCHAR) && IsWindow(sender) &&
            GetClassNameW(sender, className, ARRAYSIZE(className)) &&
            wcsncmp(className, kTaskbarClassPrefix, ARRAYSIZE(kTaskbarClassPrefix) - 1) == 0) {
            auto* text = (const WCHAR*)copyData->lpData;
            std::wstring request(text, wcsnlen(text, copyData->cbData / sizeof(WCHAR)));
            {
                std::lock_guard lock(g_pinMutex);
                g_pinRequests.push_back(std::move(request));
            }
            PostMessageW(hWnd, g_msgPinNow, 0, 0);
            *result = TRUE;
            return true;
        }
    }
    if (g_msgPinNow && uMsg == g_msgPinNow) {
        std::vector<std::wstring> requests;
        {
            std::lock_guard lock(g_pinMutex);
            requests.swap(g_pinRequests);
        }
        for (auto const& request : requests) {
            if (request.size() > 1) {
                PinToTaskbar(request.substr(1), request[0] == L'1');
            }
        }
        return true;
    }

    // A taskbar starting asks for the progress shown so far.
    if (g_msgProgressQuery && uMsg == g_msgProgressQuery) {
        HWND bar = (HWND)wParam;
        if (IsSuiteBar(bar)) {
            std::lock_guard lock(g_progressMutex);
            for (auto const& [window, entry] : g_progressTable) {
                if (IsWindow(window)) {
                    PostProgress(bar, window, entry);
                }
            }
        }
        return true;
    }

    if (g_msgForward && uMsg == g_msgForward) {
        int kind = LOWORD(wParam);
        TrayTarget target;
        if (kind >= 0 && kind < kHostIconCount && ForwardTargetFor(kind, &target)) {
            ForwardEvent(target, HIWORD(wParam), (WPARAM)lParam);
            *result = 1;
        }
        return true;
    }

    // The host's hidden icons flyout, or the taskbar's tray zone, asks for the
    // tray's icons (lParam: only the hidden ones, or all); they're packed here
    // and sent back while the asker waits.
    if (g_msgOverflow && uMsg == g_msgOverflow) {
        HWND host = (HWND)wParam;
        WCHAR className[64];
        if (!IsWindow(host) || !GetClassNameW(host, className, ARRAYSIZE(className)) ||
            (wcsncmp(className, kHostClassPrefix, ARRAYSIZE(kHostClassPrefix) - 1) != 0 &&
             wcsncmp(className, kTaskbarClassPrefix, ARRAYSIZE(kTaskbarClassPrefix) - 1) != 0)) {
            return true;
        }
        std::vector<BYTE> packet =
            part_system_icons::BuildHiddenIconsPacket(lParam == part_system_icons::kRelayAllIcons);
        COPYDATASTRUCT copyData{part_system_icons::kHiddenIconsCopyId, (DWORD)packet.size(),
                                packet.data()};
        DWORD_PTR sent = 0;
        SendMessageTimeoutW(host, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&copyData,
                            SMTO_ABORTIFHUNG, 2000, &sent);
        *result = 1;
        return true;
    }
    return false;
}

LRESULT CALLBACK ToolbarSubclassProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     DWORD_PTR) {
    if (LRESULT result; HandleRelayRequest(hWnd, uMsg, wParam, lParam, &result)) {
        return result;
    }
    if (uMsg == WM_MOUSEWHEEL || uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP) {
        POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (uMsg == WM_MOUSEWHEEL) {
            ScreenToClient(hWnd, &pt);
        }
        // The host decides what scrolling or a middle click does for each of
        // its icons (lParam: the icon's kind plus one).
        int kind = -1;
        if (HWND host = HostIconAt(hWnd, pt, &kind)) {
            if (uMsg == WM_MOUSEWHEEL) {
                PostMessageW(host, g_msgWheel, (WPARAM)(short)GET_WHEEL_DELTA_WPARAM(wParam), kind + 1);
            } else if (uMsg == WM_MBUTTONUP) {
                PostMessageW(host, g_msgMiddle, 0, kind + 1);
            }
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void ScreensChanged();

void ApplyHidingOnTaskbarThread();
extern UINT g_msgHideNow;

// Shell_NotifyIconGetRect reaches the tray as WM_COPYDATA (dwData 3): a signature, then 1 for an icon's left and
// top or 2 for its width and height (answered packed low and high, as MAKELONG; measured), then the icon (hwnd, id, guid).
// With a Tourne tray in front, Explorer's own tray is hidden and can't answer, and programs that ask before acting
// on a click ignore it (Tauri's tray, so Handy: its menu never opened). They get a small box at the pointer then,
// where the click just was.
LRESULT AnswerIconRect(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto* copyData = (const COPYDATASTRUCT*)lParam;
    LRESULT own = DefSubclassProc(hWnd, uMsg, wParam, lParam);
    if (own || copyData->cbData < 8) {
        return own;
    }
    DWORD which = 0;
    memcpy(&which, (const BYTE*)copyData->lpData + 4, 4);
    POINT pt;
    if (!GetCursorPos(&pt)) {
        return own;
    }
    constexpr int kHalf = 12;
    if (which == 1) {
        return (LRESULT)(DWORD)MAKELONG((WORD)(SHORT)(pt.x - kHalf), (WORD)(SHORT)(pt.y - kHalf));
    }
    if (which == 2) {
        return (LRESULT)(DWORD)MAKELONG(2 * kHalf, 2 * kHalf);
    }
    return own;
}

LRESULT CALLBACK TraySubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD_PTR) {
    if (LRESULT result; HandleRelayRequest(hWnd, uMsg, wParam, lParam, &result)) {
        return result;
    }
    if (uMsg == WM_COPYDATA && lParam) {
        auto* copyData = (const COPYDATASTRUCT*)lParam;
        if (copyData->dwData == 3 && copyData->lpData) {
            return AnswerIconRect(hWnd, uMsg, wParam, lParam);
        }
    }
    if (g_msgHideNow && uMsg == g_msgHideNow) {
        ApplyHidingOnTaskbarThread();
        return 0;
    }
    if (uMsg == WM_DISPLAYCHANGE) {
        ScreensChanged();
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// The toolbars exist only once Explorer's tray is up and are rebuilt when it
// restarts, so a small thread keeps them subclassed.
std::vector<HWND> g_hooked;  // Relay thread only.
HWND g_hookedTray;           // Relay thread only.
HWND g_hookedTaskSw;         // Relay thread only.
bool g_withSystemIcons;      // System Icons is on (the relay also runs for the taskbar alone).
HANDLE g_thread;
HANDLE g_stop;
HANDLE g_wake;                           // Settings changed.
std::atomic<bool> g_settingsChanged;

////////////////////////////////////////////////////////////////////////////////
// Windows' own taskbar and Start, turned away while the suite's taskbar
// replaces them, from in here where it costs nothing between events:
//   * with "Hide StartAllBack's taskbar", Explorer's taskbars are kept in
//     their own auto-hidden state (the suite's taskbar turns auto-hide on):
//     whatever would bring one back is turned away before it starts (an
//     unhide for a notification or the Windows key, the pointer at the edge,
//     the timer that slides it in, Windows 11's "should it be expanded"), and
//     only once one is hidden by Explorer itself is it cloaked as well, so
//     not even its sliver at the edge shows. Explorer is never told anything
//     that isn't so: the taskbar is never hidden or moved behind its back,
//     and its own cloaking and uncloaking go through. (Before, a taskbar
//     cloaked while Explorer believed it uncloaked, and hidden from another
//     process, made Windows 11's XAML taskbar fail Explorer.) All of it is
//     done on the taskbar's own thread;
//   * with the suite's Start on the Windows key and StartAllBack not loaded,
//     every opening of Windows' Start (the key, Ctrl+Esc, any caller) opens
//     the suite's instead, and the taskbar's keyboard hook stands down.
// When the suite's taskbar goes away (switched off, or its process ends),
// all of it is undone at once.

constexpr WCHAR kBarClass[] = L"TourneTaskbarTest_" WH_MOD_ID;
constexpr WCHAR kCloakedEvent[] = L"TourneSuite_TaskbarCloaked_" WH_MOD_ID;  // exists while hiding
constexpr WCHAR kShellStartEvent[] = L"TourneSuite_ShellStart_" WH_MOD_ID;   // exists while Start comes here
constexpr WCHAR kShellStartAnyEvent[] = L"TourneSuite_ShellStart";           // the same, for any build's bar
// What a bar asks of Explorer, on its own window (taskbar-host.cpp sets them), so a bar in
// Tourne Desktop, whose settings this mod can't read, is served too.
constexpr WCHAR kBarHideProp[] = L"TourneShell_HideNative";  // Windows' taskbars kept hidden
constexpr WCHAR kBarStartProp[] = L"TourneShell_StartKey";   // its Start takes the Windows key

// Tourne Desktop's taskbar asks for hiding or Start (it writes this as its settings apply): the relay
// then serves it as it serves the suite's own taskbar, for as long as its bar is there.
bool TourneDesktopAsks() {
    DWORD hide = 0, start = 0, size = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Desktop\\Shell", L"HideTaskbar", RRF_RT_REG_DWORD, nullptr, &hide, &size);
    size = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Desktop\\Shell", L"StartKey", RRF_RT_REG_DWORD, nullptr, &start, &size);
    return hide || start;
}
constexpr UINT kBarStart = WM_APP + 5;                                       // part_taskbar's WM_APP_START
constexpr UINT_PTR kTrayTimerUnhide = 3;  // TrayUI's timer that slides an auto-hidden taskbar in
constexpr int kExpandReasonPointerOver = 7, kExpandReasonEdgeStroke = 8;  // TaskbarExpandCollapseReason
bool g_withTaskbar;                    // The Taskbar part is on.
std::atomic<bool> g_hiding;            // Explorer's taskbars are kept hidden now.
std::atomic<bool> g_startHooked;       // ShowStartView is hooked.
std::atomic<int> g_quickPasses;        // Screens changed: look for new taskbars soon.
HANDLE g_barProcess;                   // Relay thread only: the suite's taskbar, while hiding.
HANDLE g_cloakedSignal;                // Relay thread only.
HANDLE g_shellStartSignal;             // Relay thread only.
UINT g_msgHideNow;                     // to Shell_TrayWnd: hiding (or not) brought up to date, on its thread
std::mutex g_cloakedMutex;
std::vector<HWND> g_cloaked;           // Taskbars cloaked here (guarded by g_cloakedMutex).
// Windows 11's taskbar view (Taskbar.View.dll, loaded after Explorer starts): each taskbar's
// ViewCoordinator, as its calls pass, to ask whether it's expanded. Taskbar thread only.
std::unordered_map<HWND, void*> g_viewCoordinators;
std::atomic<bool> g_taskbarViewHooked;
// With "Don't load Windows' taskbar", read once as Explorer starts: each of its taskbars is made without
// its XAML content (its task buttons, tray and progress bars), so none of it runs hidden.
bool g_noTaskbarContent;

bool IsSystemTaskbar(HWND wnd) {
    WCHAR name[32];
    return GetClassNameW(wnd, name, ARRAYSIZE(name)) &&
           (!wcscmp(name, L"Shell_TrayWnd") || !wcscmp(name, L"Shell_SecondaryTrayWnd"));
}

// Cloaks or uncloaks a taskbar, remembering which ones are cloaked here (only those are uncloaked here).
void SetTaskbarCloak(HWND wnd, bool cloak) {
    std::lock_guard lock(g_cloakedMutex);
    auto it = std::find(g_cloaked.begin(), g_cloaked.end(), wnd);
    if (cloak == (it != g_cloaked.end())) {
        return;
    }
    BOOL value = cloak;
    DwmSetWindowAttribute(wnd, DWMWA_CLOAK, &value, sizeof(value));
    if (cloak) {
        g_cloaked.push_back(wnd);
    } else {
        g_cloaked.erase(it);
    }
}

void UncloakAllTaskbars() {
    std::vector<HWND> cloaked;
    {
        std::lock_guard lock(g_cloakedMutex);
        cloaked.swap(g_cloaked);
    }
    for (HWND wnd : cloaked) {
        if (IsWindow(wnd)) {
            BOOL value = FALSE;
            DwmSetWindowAttribute(wnd, DWMWA_CLOAK, &value, sizeof(value));
        }
    }
}

// Whether Explorer has a taskbar hidden away now. Windows 11's view says so itself; otherwise
// (StartAllBack's taskbar, or before the view has been asked) an auto-hidden taskbar is down to a
// sliver at its screen's edge.
using ViewCoordinatorIsExpanded_t = bool(WINAPI*)(void* self, HWND taskbar);
ViewCoordinatorIsExpanded_t ViewCoordinatorIsExpanded_Original;
bool TaskbarHiddenAway(HWND wnd) {
    if (auto it = g_viewCoordinators.find(wnd); it != g_viewCoordinators.end() && ViewCoordinatorIsExpanded_Original) {
        return !ViewCoordinatorIsExpanded_Original(it->second, wnd);
    }
    RECT window, shown;
    MONITORINFO mi{sizeof(mi)};
    if (!GetWindowRect(wnd, &window) ||
        !GetMonitorInfoW(MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST), &mi)) {
        return false;
    }
    if (!IntersectRect(&shown, &window, &mi.rcMonitor)) {
        return true;
    }
    int sliver = MulDiv(4, GetDpiForWindow(wnd), USER_DEFAULT_SCREEN_DPI);
    return std::min(shown.right - shown.left, shown.bottom - shown.top) <= sliver;
}

// On the taskbar's thread: every taskbar cloaked once Explorer has it hidden away, while hiding;
// uncloaked otherwise.
void ApplyHidingOnTaskbarThread() {
    if (!g_hiding) {
        UncloakAllTaskbars();
        return;
    }
    {
        std::lock_guard lock(g_cloakedMutex);
        std::erase_if(g_cloaked, [](HWND wnd) { return !IsWindow(wnd); });
    }
    std::erase_if(g_viewCoordinators, [](auto const& entry) { return !IsWindow(entry.first); });
    auto apply = [](HWND wnd) {
        if (OwnedByThisProcess(wnd) && TaskbarHiddenAway(wnd)) {
            SetTaskbarCloak(wnd, true);
        }
    };
    if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr)) {
        apply(tray);
    }
    for (HWND wnd = nullptr; (wnd = FindWindowExW(nullptr, wnd, L"Shell_SecondaryTrayWnd", nullptr));) {
        apply(wnd);
    }
}

// Asks the taskbar's thread to bring the hiding up to date (posted: nothing waits on it).
void RequestHidingUpdate() {
    if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr); tray && g_msgHideNow) {
        PostMessageW(tray, g_msgHideNow, 0, 0);
    }
}

// Explorer's taskbar asked to come back (a notification, the Windows key, a flash): not while hiding.
using TrayUnhide_t = void(WINAPI*)(void* self, int flags, int request);
TrayUnhide_t TrayUnhide_Original;
TrayUnhide_t SecondaryUnhide_Original;
void WINAPI TrayUnhide_Hook(void* self, int flags, int request) {
    if (!g_hiding) {
        TrayUnhide_Original(self, flags, request);
    }
}
void WINAPI SecondaryUnhide_Hook(void* self, int flags, int request) {
    if (!g_hiding) {
        SecondaryUnhide_Original(self, flags, request);
    }
}

// Explorer sliding a taskbar in or out (the auto-hide of StartAllBack's taskbar and the older Windows
// 11 one): uncloaked before it slides in (only ever when not hiding), cloaked once it has slid away.
using TraySlideWindow_t = void(WINAPI*)(void* self, HWND wnd, const RECT* rc, HMONITOR monitor, bool show, bool flag);
TraySlideWindow_t TraySlideWindow_Original;
void WINAPI TraySlideWindow_Hook(void* self, HWND wnd, const RECT* rc, HMONITOR monitor, bool show, bool flag) {
    if (show) {
        SetTaskbarCloak(wnd, false);
    }
    TraySlideWindow_Original(self, wnd, rc, monitor, show, flag);
    if (!show && g_hiding) {
        SetTaskbarCloak(wnd, true);
    }
}

// The timer that slides an auto-hidden taskbar in when the pointer rests at the edge: not started while
// hiding.
using SetTimer_t = decltype(&SetTimer);
SetTimer_t SetTimer_Original;
UINT_PTR WINAPI SetTimer_Hook(HWND wnd, UINT_PTR id, UINT elapse, TIMERPROC proc) {
    if (id == kTrayTimerUnhide && wnd && g_hiding && IsSystemTaskbar(wnd)) {
        return 1;
    }
    return SetTimer_Original(wnd, id, elapse, proc);
}

// Windows 11's taskbar view deciding whether a taskbar should be expanded: not while hiding; the
// pointer over it or a swipe at the edge doesn't expand it; and once it has settled, it's cloaked
// if it's hidden away (as Explorer itself sees it).
using ShouldTaskbarBeExpanded_t = bool(WINAPI*)(void* self, HWND taskbar, bool expanded);
ShouldTaskbarBeExpanded_t ShouldTaskbarBeExpanded_Original;
bool WINAPI ShouldTaskbarBeExpanded_Hook(void* self, HWND taskbar, bool expanded) {
    g_viewCoordinators[taskbar] = self;
    if (g_hiding) {
        return false;
    }
    return ShouldTaskbarBeExpanded_Original(self, taskbar, expanded);
}

using UpdateIsExpanded_t = void(WINAPI*)(void* self, HWND taskbar, int reason);
UpdateIsExpanded_t UpdateIsExpanded_Original;
void WINAPI UpdateIsExpanded_Hook(void* self, HWND taskbar, int reason) {
    g_viewCoordinators[taskbar] = self;
    if (g_hiding && (reason == kExpandReasonPointerOver || reason == kExpandReasonEdgeStroke)) {
        return;
    }
    UpdateIsExpanded_Original(self, taskbar, reason);
    if (g_hiding && ViewCoordinatorIsExpanded_Original) {
        SetTaskbarCloak(taskbar, !ViewCoordinatorIsExpanded_Original(self, taskbar));
    }
}

// The suite's taskbar, when its Start takes the Windows key and StartAllBack
// (whose Start that would be) isn't loaded.
HWND StartGoesToBar() {
    constexpr PCWSTR kStartOn = L"taskbar.startMenu.enabled";  // (through variables: the Taskbar part's settings)
    constexpr PCWSTR kWinKey = L"taskbar.startMenu.winKey";
    if (GetModuleHandleW(L"StartAllBackX64.dll")) {
        return nullptr;
    }
    for (HWND bar : SuiteBars()) {
        if (GetPropW(bar, kBarStartProp)) {
            return bar;
        }
    }
    if (!g_withTaskbar || !Wh_GetIntSetting(kStartOn) || !Wh_GetIntSetting(kWinKey)) {
        return nullptr;
    }
    return FindWindowW(kBarClass, nullptr);
}

using ShowStartView_t = HRESULT(WINAPI*)(void* self, int method, int flags);
ShowStartView_t ShowStartView_Original;
HRESULT WINAPI ShowStartView_Hook(void* self, int method, int flags) {
    if (HWND bar = StartGoesToBar()) {
        DWORD pid = 0;
        GetWindowThreadProcessId(bar, &pid);
        AllowSetForegroundWindow(pid);  // (Explorer has the right, from the key press)
        PostMessageW(bar, kBarStart, 0, 0);
        return S_OK;
    }
    return ShowStartView_Original(self, method, flags);
}

// Hiding ends: the taskbar's thread uncloaks them (waited for, briefly; if it doesn't answer, done
// from here), and Explorer's own auto-hide takes over again.
void StopHiding() {
    g_hiding = false;
    DWORD_PTR ignored;
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!tray || !g_msgHideNow ||
        !SendMessageTimeoutW(tray, g_msgHideNow, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, &ignored)) {
        UncloakAllTaskbars();
    }
    if (g_barProcess) {
        // Its process ended without putting Windows' taskbar back (a crash): the auto-hide the taskbar
        // saved (as its ABM state + 1) is put back here, so Windows' taskbar returns as it was.
        constexpr PCWSTR kStore = L"Software\\TourneTray", kState = L"SystemTaskbarState";
        DWORD saved = 0, size = sizeof(saved);
        if (tray && WaitForSingleObject(g_barProcess, 0) == WAIT_OBJECT_0 &&
            RegGetValueW(HKEY_CURRENT_USER, kStore, kState, RRF_RT_REG_DWORD, nullptr, &saved, &size) == ERROR_SUCCESS &&
            saved) {
            APPBARDATA abd{sizeof(abd)};
            abd.hWnd = tray;
            abd.lParam = saved - 1;
            SHAppBarMessage(ABM_SETSTATE, &abd);
            RegDeleteKeyValueW(HKEY_CURRENT_USER, kStore, kState);
            Wh_Log(L"The suite's taskbar ended without a goodbye: Windows' taskbar is back as it was");
        }
        CloseHandle(g_barProcess);
        g_barProcess = nullptr;
    }
    if (g_cloakedSignal) {
        CloseHandle(g_cloakedSignal);
        g_cloakedSignal = nullptr;
    }
}

// On each pass of the relay thread: hiding while the setting and the suite's taskbar are both there,
// which also catches taskbars for new screens (the taskbar's thread does the cloaking).
void UpdateHiding() {
    constexpr PCWSTR kHide = L"taskbar.hideSystemTaskbar";
    HWND bar = g_withTaskbar && Wh_GetIntSetting(kHide) ? FindWindowW(kBarClass, nullptr) : nullptr;
    if (!bar) {
        for (HWND each : SuiteBars()) {
            if (GetPropW(each, kBarHideProp)) {
                bar = each;
                break;
            }
        }
    }
    if (!bar) {
        bool cloakedAny;
        {
            std::lock_guard lock(g_cloakedMutex);
            cloakedAny = !g_cloaked.empty();
        }
        if (g_hiding || cloakedAny) {
            StopHiding();
        }
        return;
    }
    if (!g_barProcess) {
        DWORD pid = 0;
        GetWindowThreadProcessId(bar, &pid);
        g_barProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
        if (!g_barProcess) {
            return;  // (no way to know when it ends: not hidden)
        }
    }
    g_hiding = true;
    if (!g_cloakedSignal) {
        g_cloakedSignal = CreateEventW(nullptr, TRUE, FALSE, kCloakedEvent);
    }
    RequestHidingUpdate();
}

// A screen came or went: its taskbar is made a moment later, so a few quick
// passes find it (only while hiding).
void ScreensChanged() {
    if (g_hiding && g_wake) {
        g_quickPasses = 8;
        SetEvent(g_wake);
    }
}

// Start comes here once the hook is in and Explorer's tray is up (StartAllBack,
// if it's there, is loaded by then). The suite's taskbar is told either way.
void UpdateShellStart() {
    bool want = g_startHooked && FindWindowW(L"Shell_TrayWnd", nullptr) && !GetModuleHandleW(L"StartAllBackX64.dll");
    if (want == (g_shellStartSignal != nullptr)) {
        return;
    }
    static HANDLE anySignal;
    if (want) {
        g_shellStartSignal = CreateEventW(nullptr, TRUE, FALSE, kShellStartEvent);
        anySignal = CreateEventW(nullptr, TRUE, FALSE, kShellStartAnyEvent);
    } else {
        CloseHandle(g_shellStartSignal);
        g_shellStartSignal = nullptr;
        if (anySignal) {
            CloseHandle(anySignal);
            anySignal = nullptr;
        }
    }
    UINT changed = RegisterWindowMessageW(L"TourneSuite_ShellStart");
    for (HWND bar : SuiteBars()) {
        PostMessageW(bar, changed, 0, 0);
    }
}

void HookToolbars() {
    // Tooltips for the hidden icons flyout are captured on the taskbar.
    if (g_withSystemIcons) {
        part_system_icons::EnsureExplorerSide();
    }
    std::erase_if(g_hooked, [](HWND hWnd) { return !IsWindow(hWnd); });
    for (HWND toolbar : TrayToolbars()) {
        if (std::find(g_hooked.begin(), g_hooked.end(), toolbar) == g_hooked.end() &&
            WindhawkUtils::SetWindowSubclassFromAnyThread(toolbar, ToolbarSubclassProc, 0)) {
            g_hooked.push_back(toolbar);
        }
    }
    // The taskbar answers too, for when there's no classic tray.
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (tray && tray != g_hookedTray && OwnedByThisProcess(tray) &&
        WindhawkUtils::SetWindowSubclassFromAnyThread(tray, TraySubclassProc, 0)) {
        g_hookedTray = tray;
    }
    // Where apps send their progress (it exists with StartAllBack's classic
    // taskbar, and with Windows 10's).
    if (g_hookedTaskSw && !IsWindow(g_hookedTaskSw)) {
        g_hookedTaskSw = nullptr;
    }
    if (tray && !g_hookedTaskSw) {
        HWND taskSw = nullptr;
        EnumChildWindows(
            tray,
            [](HWND hWnd, LPARAM lParam) __stdcall -> BOOL {
                WCHAR className[32];
                if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
                    wcscmp(className, L"MSTaskSwWClass") == 0) {
                    *(HWND*)lParam = hWnd;
                    return FALSE;
                }
                return TRUE;
            },
            (LPARAM)&taskSw);
        if (taskSw && OwnedByThisProcess(taskSw) &&
            WindhawkUtils::SetWindowSubclassFromAnyThread(taskSw, TaskSwSubclassProc, 0)) {
            g_hookedTaskSw = taskSw;
        }
    }
}

DWORD WINAPI ThreadProc(LPVOID) {
    // Quick while waiting for the tray, then an occasional check for toolbars
    // that were recreated. With StartAllBack's "^" off there's only the one.
    // The suite's taskbar ending (its process) wakes it too, to uncloak.
    DWORD wait = 1000;
    DWORD woke;
    do {
        if (g_settingsChanged.exchange(false) && g_withSystemIcons) {
            part_system_icons::ExplorerSideSettingsChanged(true);
        }
        HookToolbars();
        UpdateHiding();
        UpdateShellStart();
        wait = g_hooked.empty() && !g_hookedTray ? 1000 : 10000;
        if (g_quickPasses > 0) {
            g_quickPasses--;
            wait = 300;
        }
        HANDLE events[] = {g_stop, g_wake, g_barProcess};
        woke = WaitForMultipleObjects(g_barProcess ? 3 : 2, events, FALSE, wait);
        if (woke == WAIT_OBJECT_0 + 2) {
            // The suite's taskbar ended without putting Explorer's back (a crash): back to Explorer's own
            // auto-hide, so there's always a taskbar. Hidden again if a new one is up by the next pass.
            // (Shown too if an older build hid it outright.)
            StopHiding();
            if (HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr); tray && !IsWindowVisible(tray)) {
                ShowWindow(tray, SW_SHOWNA);
            }
            for (HWND wnd = nullptr; (wnd = FindWindowExW(nullptr, wnd, L"Shell_SecondaryTrayWnd", nullptr));) {
                if (!IsWindowVisible(wnd)) {
                    ShowWindow(wnd, SW_SHOWNA);
                }
            }
        }
    } while (woke != WAIT_OBJECT_0);

    StopHiding();
    if (g_shellStartSignal) {
        g_startHooked = false;
        UpdateShellStart();
    }

    for (HWND toolbar : g_hooked) {
        if (IsWindow(toolbar)) {
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(toolbar, ToolbarSubclassProc);
        }
    }
    g_hooked.clear();
    if (g_hookedTray && IsWindow(g_hookedTray)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_hookedTray, TraySubclassProc);
    }
    g_hookedTray = nullptr;
    if (g_hookedTaskSw && IsWindow(g_hookedTaskSw)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_hookedTaskSw, TaskSwSubclassProc);
    }
    g_hookedTaskSw = nullptr;
    if (g_withSystemIcons) {
        part_system_icons::StopExplorerSide();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Windows' own Alt+Tab, turned away while the suite's Switcher runs (as set):
// Explorer's view is asked to show and refuses, and the Switcher opens
// instead, so an Alt+Tab its keyboard hook missed still lands there. Without
// the Switcher's window, Windows' view shows as usual.

constexpr WCHAR kSwitcherClass[] = L"TourneSwitcher_" WH_MOD_ID;
constexpr UINT kSwitcherStep = WM_APP + 3;  // part_switcher's WM_APP_STEP
constexpr WORD kSwitcherSticky = 1, kSwitcherAllMonitors = 2;

using AltTabShow_t = HRESULT(WINAPI*)(void* self, void* monitor, int flags, void* view);
AltTabShow_t XamlAltTabShow_Original;
AltTabShow_t ClassicAltTabShow_Original;
HANDLE g_altTabThread;
std::atomic<bool> g_altTabUnloading;

bool SwitcherTakesAltTab() {
    // The Switcher's own setting (through a variable, so the build doesn't
    // file it under another part).
    constexpr PCWSTR kReplaceSetting = L"switcher.keys.replaceWindows";
    HWND switcher = FindWindowW(kSwitcherClass, nullptr);
    if (!switcher || !Wh_GetIntSetting(kReplaceSetting)) {
        return false;
    }
    bool back = GetAsyncKeyState(VK_SHIFT) < 0;
    WORD flags = (GetAsyncKeyState(VK_CONTROL) < 0 ? kSwitcherSticky : 0) |
                 (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0 ? kSwitcherAllMonitors : 0);
    PostMessageW(switcher, kSwitcherStep, back ? (WPARAM)-1 : 1, MAKELPARAM(0 /* from Tab */, flags));
    return true;
}

HRESULT WINAPI XamlAltTabShow_Hook(void* self, void* monitor, int flags, void* view) {
    if (SwitcherTakesAltTab()) {
        return E_FAIL;
    }
    return XamlAltTabShow_Original(self, monitor, flags, view);
}

HRESULT WINAPI ClassicAltTabShow_Hook(void* self, void* monitor, int flags, void* view) {
    if (SwitcherTakesAltTab()) {
        return E_FAIL;
    }
    return ClassicAltTabShow_Original(self, monitor, flags, view);
}

// Whether a Windows feature is switched on (its staged rollout), from ntdll's own record of it; nothing
// when the record can't be read or leaves it to the default.
std::optional<bool> OsFeatureEnabled(UINT32 featureId) {
    struct FeatureConfiguration {
        UINT32 featureId;
        UINT32 group : 4;
        UINT32 enabledState : 2;  // 0 default, 1 disabled, 2 enabled
        UINT32 isWexpConfiguration : 1;
        UINT32 hasSubscriptions : 1;
        UINT32 variant : 6;
        UINT32 variantPayloadKind : 2;
        UINT32 variantPayload;
    };
    using Query_t = LONG(NTAPI*)(UINT32, int, UINT64*, FeatureConfiguration*);
    static auto query = (Query_t)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlQueryFeatureConfiguration");
    FeatureConfiguration config{};
    UINT64 changeStamp = 0;
    if (!query || query(featureId, 1 /* runtime */, &changeStamp, &config) < 0 || config.enabledState == 0) {
        return std::nullopt;  // (left to the default: unknown)
    }
    return config.enabledState == 2;
}

DWORD WINAPI AltTabHookThread(LPVOID) {
    HMODULE twinui = GetModuleHandleW(L"twinui.pcshell.dll");
    if (!twinui) {
        twinui = LoadLibraryExW(L"twinui.pcshell.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    }
    if (!twinui) {
        return 0;
    }
    // Windows' Start is only turned away for the suite's taskbar.
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(?Show@XamlAltTabViewHost@@UEAAJPEAUIImmersiveMonitor@@W4ALT_TAB_VIEW_FLAGS@@PEAUIApplicationView@@@Z)"},
         &XamlAltTabShow_Original, XamlAltTabShow_Hook, true},
        {{LR"(?Show@CAltTabViewHost@@UEAAJPEAUIImmersiveMonitor@@W4ALT_TAB_VIEW_FLAGS@@PEAUIApplicationView@@@Z)"},
         &ClassicAltTabShow_Original, ClassicAltTabShow_Hook, true},
        {{LR"(?ShowStartView@XamlLauncher@@UEAAJW4IMMERSIVELAUNCHERSHOWMETHOD@@W4IMMERSIVELAUNCHERSHOWFLAGS@@@Z)"},
         &ShowStartView_Original, ShowStartView_Hook, true},
    };
    WH_HOOK_SYMBOLS_OPTIONS options = {};
    options.optionsSize = sizeof(options);
    options.noUndecoratedSymbols = TRUE;
    if (!WindhawkUtils::HookSymbols(twinui, hooks, g_withTaskbar ? ARRAYSIZE(hooks) : 2, &options)) {
        Wh_Log(L"Alt+Tab: Explorer's Alt+Tab view wasn't found");
        return 0;
    }
    // Explorer's taskbar coming back while it's kept hidden: its unhide, its slide, its unhide timer. (The
    // names as Taskbar auto-hide fine tuning by m417z hooks them, the same approach.)
    if (g_withTaskbar && !g_altTabUnloading) {
        WindhawkUtils::SYMBOL_HOOK trayHooks[] = {
            {{LR"(public: virtual void __cdecl TrayUI::Unhide(enum TrayCommon::TrayUnhideFlags,enum TrayCommon::UnhideRequest))"},
             &TrayUnhide_Original, TrayUnhide_Hook, true},
            {{LR"(private: void __cdecl CSecondaryTray::_Unhide(enum TrayCommon::TrayUnhideFlags,enum TrayCommon::UnhideRequest))"},
             &SecondaryUnhide_Original, SecondaryUnhide_Hook, true},
            {{LR"(public: virtual void __cdecl TrayUI::SlideWindow(struct HWND__ *,struct tagRECT const *,struct HMONITOR__ *,bool,bool))"},
             &TraySlideWindow_Original, TraySlideWindow_Hook, true},
        };
        HMODULE taskbar = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!taskbar || !WindhawkUtils::HookSymbols(taskbar, trayHooks, ARRAYSIZE(trayHooks))) {
            Wh_Log(L"Explorer's taskbar: its unhide wasn't found (it's only cloaked once hidden)");
        }
        WindhawkUtils::SetFunctionHook(SetTimer, SetTimer_Hook, &SetTimer_Original);
    }
    if (!g_altTabUnloading) {
        Wh_ApplyHookOperations();
        if (ShowStartView_Original) {
            g_startHooked = true;
        } else if (g_withTaskbar) {
            Wh_Log(L"Start: Windows' ShowStartView wasn't found; the keyboard hook stays");
        }
        if (g_wake) {
            SetEvent(g_wake);  // (the relay thread takes it from here)
        }
    }
    // Windows 11's taskbar view, once Explorer has loaded it (it does after starting), when its newer
    // auto-hide is the one in use (as the mod above decides).
    // (Without its content Explorer never loads the view, so nothing is waited for.)
    while (g_withTaskbar && !g_noTaskbarContent && !g_altTabUnloading && !g_taskbarViewHooked) {
        HMODULE view = GetModuleHandleW(L"Taskbar.View.dll");
        if (!view) {
            view = GetModuleHandleW(L"ExplorerExtensions.dll");
        }
        if (!view) {
            Sleep(1000);
            continue;
        }
        g_taskbarViewHooked = true;
        constexpr UINT32 kNewTaskbarAutoHideAnimation = 41356296;
        if (!OsFeatureEnabled(kNewTaskbarAutoHideAnimation).value_or(true)) {
            Wh_Log(L"Explorer's taskbar: the older auto-hide (slides only)");
            break;
        }
        WindhawkUtils::SYMBOL_HOOK viewHooks[] = {
            {{LR"(public: bool __cdecl winrt::Taskbar::implementation::ViewCoordinator::IsExpanded(unsigned __int64))"},
             &ViewCoordinatorIsExpanded_Original, nullptr, true},
            {{LR"(public: bool __cdecl winrt::Taskbar::implementation::ViewCoordinator::ShouldTaskbarBeExpanded(unsigned __int64,bool))"},
             &ShouldTaskbarBeExpanded_Original, ShouldTaskbarBeExpanded_Hook, true},
            {{LR"(public: void __cdecl winrt::Taskbar::implementation::ViewCoordinator::UpdateIsExpanded(unsigned __int64,enum TaskbarTipTest::TaskbarExpandCollapseReason))"},
             &UpdateIsExpanded_Original, UpdateIsExpanded_Hook, true},
        };
        if (WindhawkUtils::HookSymbols(view, viewHooks, ARRAYSIZE(viewHooks)) && !g_altTabUnloading) {
            Wh_ApplyHookOperations();
            if (g_wake) {
                SetEvent(g_wake);
            }
        } else {
            Wh_Log(L"Explorer's taskbar: its view's expanding wasn't found");
        }
    }
    return 0;
}

// Each taskbar loads its XAML content (Taskbar.View, from Windows' extension or its controller) as it
// starts; when neither loads, Explorer itself unsubscribes that taskbar's events and carries on with the
// window alone. Turned away here, so Windows 11's taskbar is never built at all, as with StartAllBack's.
using TryLoadTaskbarFrame_t = bool(WINAPI*)(void* self);
TryLoadTaskbarFrame_t TryLoadFrameFromXamlExtension_Original;
TryLoadTaskbarFrame_t TryLoadFrameFromController_Original;
bool WINAPI TryLoadFrameFromXamlExtension_Hook(void*) {
    return false;
}
bool WINAPI TryLoadFrameFromController_Hook(void*) {
    return false;
}

// In init, before Explorer makes its taskbars (Windhawk applies the hooks once init returns). Only while
// the suite's taskbar replaces Windows' (it and hiding both on); a change takes effect as Explorer restarts.
void HookTaskbarContent() {
    constexpr PCWSTR kHide = L"taskbar.hideSystemTaskbar";
    constexpr PCWSTR kNoContent = L"taskbar.noSystemTaskbarContent";
    if (!g_withTaskbar || !Wh_GetIntSetting(kHide) || !Wh_GetIntSetting(kNoContent) ||
        GetModuleHandleW(L"StartAllBackX64.dll")) {
        return;
    }
    HMODULE taskbar = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(private: bool __cdecl TaskbarHost::TryLoadTaskbarFrameFromXamlExtension(void))"},
         &TryLoadFrameFromXamlExtension_Original, TryLoadFrameFromXamlExtension_Hook},
        {{LR"(private: bool __cdecl TaskbarHost::TryLoadTaskbarFrameFromController(void))"},
         &TryLoadFrameFromController_Original, TryLoadFrameFromController_Hook},
    };
    g_noTaskbarContent = taskbar && WindhawkUtils::HookSymbols(taskbar, hooks, ARRAYSIZE(hooks));
    if (g_noTaskbarContent) {
        Wh_Log(L"Explorer's taskbar: made without its content");
    } else {
        Wh_Log(L"Explorer's taskbar: its content loader wasn't found (it loads as usual)");
    }
}

// After init, since symbols can take a while the first time (Windhawk fetches
// them) and the hooks are applied on their own.
void PartAfterInit() {
    g_altTabThread = CreateThread(nullptr, 0, AltTabHookThread, nullptr, 0, nullptr);
}

BOOL PartInit() {
    g_msgForward = RegisterWindowMessageW(L"TourneSuite_RelayForward");
    g_msgWheel = RegisterWindowMessageW(L"TourneSuite_RelayWheel");
    g_msgMiddle = RegisterWindowMessageW(L"TourneSuite_RelayMiddle");
    g_msgOverflow = RegisterWindowMessageW(L"TourneSuite_RelayOverflow");
    g_msgPinNow = RegisterWindowMessageW(L"TourneSuite_RelayPinNow");
    g_msgProgress = RegisterWindowMessageW(L"TourneSuite_Progress");
    g_msgProgressQuery = RegisterWindowMessageW(L"TourneSuite_ProgressQuery");
    g_msgHideNow = RegisterWindowMessageW(L"TourneSuite_TaskbarHideNow");
    g_stop =CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_wake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (SuiteIsShellExplorer()) {
        HookTaskbarContent();
    }
    // The Explorer side reads System Icons' settings in this process.
    if (g_withSystemIcons) {
        part_system_icons::ExplorerSideSettingsChanged(true);
    }
    g_thread = CreateThread(nullptr, 0, ThreadProc, nullptr, 0, nullptr);
    return g_thread != nullptr;
}

void PartUninit() {
    // The symbol thread must be done with this module's code before it unloads.
    g_altTabUnloading = true;
    if (g_altTabThread) {
        WaitForSingleObject(g_altTabThread, INFINITE);
        CloseHandle(g_altTabThread);
        g_altTabThread = nullptr;
    }
    SetEvent(g_stop);
    if (g_thread) {
        WaitForSingleObject(g_thread, 5000);
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
    CloseHandle(g_stop);
    g_stop = nullptr;
    CloseHandle(g_wake);
    g_wake = nullptr;
}

void PartSettingsChanged() {
    g_settingsChanged = true;
    SetEvent(g_wake);
}

}  // namespace part_explorer_relay
#endif  // _WIN64

////////////////////////////////////////////////////////////////////////////////
// Suite: the tool process
//
// Windhawk's pattern for mods that run in a process of their own: the
// windhawk.exe UI process launches "windhawk.exe -tool-mod <id>", the mod
// starts its work there, and the process's own entry point is stopped so only
// the mod runs. See
// https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process

enum SuiteRole : int {
    kRoleNormal,     // Any other process: the injected parts.
    kRoleSkip,       // Windhawk's service, another mod's tool process, or the lock screen.
    kRoleLauncher,   // The Windhawk process that starts the tool process.
    kRoleTool,       // The suite's own tool process: System Icons.
};

SuiteRole g_suiteRole = kRoleNormal;
HANDLE g_toolMutex;

SuiteRole SuiteDetermineRole() {
    DWORD sessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) && sessionId == 0) {
        return kRoleSkip;
    }

    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return kRoleNormal;
    }
    PCWSTR name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    // The lock screen and sign-in screen: nothing of the suite's belongs there, and a part
    // loaded into them can only slow them down as the screen wakes.
    if (_wcsicmp(name, L"LockApp.exe") == 0 || _wcsicmp(name, L"LogonUI.exe") == 0) {
        return kRoleSkip;
    }
    if (_wcsicmp(name, L"windhawk.exe") != 0 && _wcsicmp(name, L"windhawk-mod.exe") != 0) {
        return kRoleNormal;
    }

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return kRoleSkip;
    }
    SuiteRole role = kRoleLauncher;
    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 || wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            role = kRoleSkip;
            break;
        }
        if (wcscmp(argv[i], L"-tool-mod") == 0 && i + 1 < argc) {
            role = wcscmp(argv[i + 1], WH_MOD_ID) == 0 ? kRoleTool : kRoleSkip;
            break;
        }
    }
    LocalFree(argv);
    return role;
}

void WINAPI SuiteEntryPointHook() {
    ExitThread(0);
}

// One tool process at a time: whoever creates the named mutex runs, and it
// stays claimed while that process holds the handle. Not mutex ownership,
// which belongs to a thread: the entry point hook ends this process's main
// thread, abandoning an owned mutex, and a second launch (Windhawk's tray
// process and its UI both launch one) then took it over and doubled every
// tray icon. After a settings change the old process can still be on its way
// out, so a taken mutex is retried for a moment before giving up (the
// launcher waits for the old one to go before it starts a new one).
bool SuiteClaimToolProcess() {
    for (int attempt = 0; attempt < 10; attempt++) {
        HANDLE mutex = CreateMutexW(nullptr, FALSE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!mutex) {
            return false;
        }
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            g_toolMutex = mutex;
            return true;
        }
        CloseHandle(mutex);
        Sleep(100);
    }
    Wh_Log(L"Another tool process is still running");
    return false;
}

void SuiteLaunchToolProcess() {
    WCHAR path[MAX_PATH];
    switch (GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        case 0:
        case ARRAYSIZE(path):
            Wh_Log(L"GetModuleFileName failed");
            return;
    }

    WCHAR commandLine[MAX_PATH + 2 + (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", path, WH_MOD_ID);

    HMODULE kernel = GetModuleHandleW(L"kernelbase.dll");
    if (!kernel) {
        kernel = GetModuleHandleW(L"kernel32.dll");
    }
    using CreateProcessInternalW_t = BOOL(WINAPI*)(
        HANDLE, LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, WINBOOL, DWORD,
        LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION, PHANDLE);
    auto createProcessInternal =
        kernel ? (CreateProcessInternalW_t)GetProcAddress(kernel, "CreateProcessInternalW")
               : nullptr;
    if (!createProcessInternal) {
        Wh_Log(L"No CreateProcessInternalW");
        return;
    }

    STARTUPINFOW si{.cb = sizeof(STARTUPINFOW), .dwFlags = STARTF_FORCEOFFFEEDBACK};
    PROCESS_INFORMATION pi;
    if (!createProcessInternal(nullptr, path, commandLine, nullptr, nullptr, FALSE,
                               NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi, nullptr)) {
        Wh_Log(L"CreateProcess failed");
        return;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Every windhawk.exe loads the launcher: its tray process, and another each
// time Windhawk is opened. With a tool process already running, a second one
// only sat in its init waiting for the first, which Windhawk showed as the
// suite "Initializing..." every time it opened. So one is started only once
// none is running: after a settings change the old one is on its way out and
// gone within moments; one still there after a few seconds is running for
// good, and is left to it. Waited for on a thread, so the launcher's own init
// isn't held up either.
HANDLE g_launchThread;
HANDLE g_launchStop;

DWORD WINAPI SuiteLaunchWhenFreeThread(LPVOID) {
    for (int attempt = 0; attempt < 50; attempt++) {
        HANDLE mutex = OpenMutexW(SYNCHRONIZE, FALSE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!mutex) {
            SuiteLaunchToolProcess();
            return 0;
        }
        CloseHandle(mutex);
        if (WaitForSingleObject(g_launchStop, 100) == WAIT_OBJECT_0) {
            return 0;
        }
    }
    Wh_Log(L"The tool process is already running");
    return 0;
}

void SuiteLaunchToolProcessWhenFree() {
    g_launchStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_launchThread = g_launchStop ? CreateThread(nullptr, 0, SuiteLaunchWhenFreeThread, nullptr, 0, nullptr) : nullptr;
}

void SuiteStopLaunching() {
    if (g_launchThread) {
        SetEvent(g_launchStop);
        WaitForSingleObject(g_launchThread, INFINITE);
        CloseHandle(g_launchThread);
        g_launchThread = nullptr;
    }
    if (g_launchStop) {
        CloseHandle(g_launchStop);
        g_launchStop = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Suite: which parts run where

enum SuitePart : int {
    kPartTrayIcons,
    kPartTrayFolders,
    kPartSystemIcons,
    kPartTextColor,
    kPartMenuNoIcons,
    kPartMenuNoDuplicates,
    kPartExplorerRelay,
    kPartClock,
    kPartIconRedirect,
    kPartTweaks,
    kPartTaskbar,
    kPartSwitcher,
    kPartExplorerStyle,
    kPartThemeGaps,
    kPartDarkMagic,
    kPartExplorerColors,
    kPartClassicMenus,
    kPartCount
};

// The relay has no settings of its own: it follows System Icons.
constexpr PCWSTR kPartKeys[kPartCount] = {
    L"trayIcons", L"trayFolders", L"systemIcons", L"textColor",
    L"menuNoIcons", L"menuNoDuplicates", L"systemIcons", L"clock",
    L"iconRedirect", L"tweaks", L"taskbar", L"switcher",
    L"explorerStyle", L"themeGaps", L"darkMagic", L"explorerColors",
    L"classicMenus",
};

bool g_partEnabled[kPartCount];
bool g_partWanted[kPartCount];
bool g_partActive[kPartCount];

// The parts that run in the tool process.
bool SuiteToolPartEnabled(bool const* enabled) {
    return enabled[kPartSystemIcons] || enabled[kPartTaskbar] || enabled[kPartSwitcher];
}

bool SuitePartEnabled(int part) {
    return Wh_GetIntSetting(L"%s.enabled", kPartKeys[part]) != 0;
}

bool SuiteIsExplorer() {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return false;
    }
    PCWSTR name = wcsrchr(path, L'\\');
    return _wcsicmp(name ? name + 1 : path, L"explorer.exe") == 0;
}

// Change Tray Icons used to load only into apps on its inclusion list; a row
// naming this app now does the same job.
bool SuiteTrayIconsWanted() {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return false;
    }
    PCWSTR name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    for (int i = 0; i < 128; i++) {
        PCWSTR row = Wh_GetStringSetting(L"trayIcons.icons[%d].path", i);
        bool matched = *row && (_wcsicmp(row, path) == 0 || _wcsicmp(row, name) == 0);
        Wh_FreeStringSetting(row);
        if (matched) {
            return true;
        }
    }
    return false;
}

// Whether each part has work in this process with the current settings.
void SuiteEvaluate() {
    for (int p = 0; p < kPartCount; p++) {
        g_partEnabled[p] = SuitePartEnabled(p);
        g_partWanted[p] = false;
    }

    // The tool process runs System Icons and the test taskbar.
    if (g_suiteRole == kRoleTool) {
#ifdef _WIN64
        g_partWanted[kPartSystemIcons] = g_partEnabled[kPartSystemIcons];
        g_partWanted[kPartTaskbar] = g_partEnabled[kPartTaskbar];
        g_partWanted[kPartSwitcher] = g_partEnabled[kPartSwitcher];
        // The taskbar extracts app icons here, so the icon theme applies to
        // them as it does to Explorer's.
        g_partWanted[kPartIconRedirect] = g_partWanted[kPartTaskbar] && g_partEnabled[kPartIconRedirect] &&
                                          part_icon_redirect::ShouldLoadInThisProcess();
#endif
        return;
    }

    bool explorer = SuiteIsExplorer();
    g_partWanted[kPartTrayIcons] = g_partEnabled[kPartTrayIcons] && SuiteTrayIconsWanted();
#ifdef _WIN64
    g_partWanted[kPartTrayFolders] = g_partEnabled[kPartTrayFolders] && explorer;
    g_partWanted[kPartClock] = g_partEnabled[kPartClock] && explorer;
    // The relay serves System Icons, and passes app progress to the taskbar.
    g_partWanted[kPartExplorerRelay] =
        (g_partEnabled[kPartExplorerRelay] || g_partEnabled[kPartTaskbar]) && explorer;
    part_explorer_relay::g_withSystemIcons = g_partEnabled[kPartSystemIcons];
    part_explorer_relay::g_withTaskbar = g_partEnabled[kPartTaskbar] || part_explorer_relay::TourneDesktopAsks();
    g_partWanted[kPartExplorerStyle] = g_partEnabled[kPartExplorerStyle] && explorer;
    g_partWanted[kPartExplorerColors] =
        g_partEnabled[kPartExplorerColors] && part_explorer_colors::ShouldLoadInThisProcess();
#else
    (void)explorer;
#endif
    g_partWanted[kPartTextColor] =
        g_partEnabled[kPartTextColor] && part_text_color::ShouldLoadInThisProcess();
    g_partWanted[kPartThemeGaps] =
        g_partEnabled[kPartThemeGaps] && part_theme_gaps::ShouldLoadInThisProcess();
    g_partWanted[kPartDarkMagic] =
        g_partEnabled[kPartDarkMagic] && part_dark_magic::ShouldLoadInThisProcess();
    g_partWanted[kPartClassicMenus] =
        g_partEnabled[kPartClassicMenus] && part_classic_menus::ShouldLoadInThisProcess();
    g_partWanted[kPartMenuNoIcons] =
        g_partEnabled[kPartMenuNoIcons] && part_menu_no_icons::ShouldLoadInThisProcess();
    g_partWanted[kPartMenuNoDuplicates] =
        g_partEnabled[kPartMenuNoDuplicates] &&
        part_menu_no_duplicates::ShouldLoadInThisProcess();
    g_partWanted[kPartIconRedirect] =
        g_partEnabled[kPartIconRedirect] && part_icon_redirect::ShouldLoadInThisProcess();
    // In the launcher Tweaks applies settings instead; see Wh_ModAfterInit.
    g_partWanted[kPartTweaks] = g_partEnabled[kPartTweaks] && g_suiteRole != kRoleLauncher &&
                                part_tweaks::ShouldLoadInThisProcess();
}

bool SuiteStartPart(int part, BOOL (*init)()) {
    if (!g_partWanted[part]) {
        return false;
    }
    g_suiteCurrentPart = part;
    bool ok = init();
    g_suiteCurrentPart = -1;
    if (!ok) {
        SuiteDropHooks(part);
        Wh_Log(L"%s didn't start", kPartKeys[part]);
    }
    return ok;
}

////////////////////////////////////////////////////////////////////////////////
// Crash guard. A part that crashes Explorer would crash it again each time
// Windows restarts it, leaving no taskbar to fix anything from. Each start of
// the shell's Explorer is counted in HKCU\Software\Tourne\Suite; one that stays
// up for 20 seconds clears the count. The fourth start within two minutes
// (three quick ends in a row) trips the guard: the suite then stays loaded in
// Explorer but starts none of its parts there, and says so on the taskbar,
// until its settings are saved again. Costs one registry read and write when
// Explorer starts, and one more write 20 seconds later.

constexpr WCHAR kCrashGuardKey[] = L"Software\\Tourne\\Suite";
constexpr DWORD kCrashGuardStarts = 4;
constexpr ULONGLONG kCrashGuardWindowSeconds = 120;
constexpr DWORD kCrashGuardStableMs = 20000;
bool g_crashGuardTripped;
HANDLE g_crashGuardTimer;

ULONGLONG SuiteNowSeconds() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return (((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime) / 10000000ULL;
}

// The shell's own Explorer, not a folder window's process (those start with
// arguments such as /factory or -Embedding).
bool SuiteIsShellExplorer() {
    if (!SuiteIsExplorer()) {
        return false;
    }
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        LocalFree(argv);
    }
    return argc <= 1;
}

void CALLBACK SuiteCrashGuardStable(PVOID, BOOLEAN) {
    DWORD zero = 0;
    RegSetKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerStarts", REG_DWORD, &zero, sizeof(zero));
}

// True when the parts should stay out of this Explorer.
bool SuiteCrashGuardCheck() {
    DWORD tripped = 0, starts = 0, size = sizeof(DWORD);
    ULONGLONG first = 0;
    RegGetValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"CrashGuardTripped", RRF_RT_REG_DWORD, nullptr, &tripped, &size);
    if (tripped) {
        return true;
    }
    size = sizeof(DWORD);
    RegGetValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerStarts", RRF_RT_REG_DWORD, nullptr, &starts, &size);
    size = sizeof(ULONGLONG);
    RegGetValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerFirstStart", RRF_RT_REG_QWORD, nullptr, &first, &size);
    ULONGLONG now = SuiteNowSeconds();
    if (!starts || now < first || now - first > kCrashGuardWindowSeconds) {
        starts = 0;
        first = now;
        RegSetKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerFirstStart", REG_QWORD, &first, sizeof(first));
    }
    starts++;
    if (starts >= kCrashGuardStarts) {
        DWORD one = 1, zero = 0;
        RegSetKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"CrashGuardTripped", REG_DWORD, &one, sizeof(one));
        RegSetKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerStarts", REG_DWORD, &zero, sizeof(zero));
        Wh_Log(L"Explorer ended %u times within %u s of starting: the suite's parts stay out of it until its "
               L"settings are saved",
               kCrashGuardStarts - 1, (UINT)kCrashGuardWindowSeconds);
        return true;
    }
    RegSetKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerStarts", REG_DWORD, &starts, sizeof(starts));
    CreateTimerQueueTimer(&g_crashGuardTimer, nullptr, SuiteCrashGuardStable, nullptr, kCrashGuardStableMs, 0,
                          WT_EXECUTEONLYONCE);
    return false;
}

void SuiteCrashGuardStop() {
    if (g_crashGuardTimer) {
        DeleteTimerQueueTimer(nullptr, g_crashGuardTimer, INVALID_HANDLE_VALUE);
        g_crashGuardTimer = nullptr;
    }
}

// Saving the settings is the way back in.
void SuiteCrashGuardReset() {
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"CrashGuardTripped");
    RegDeleteKeyValueW(HKEY_CURRENT_USER, kCrashGuardKey, L"ExplorerStarts");
}

////////////////////////////////////////////////////////////////////////////////
// Mod lifecycle

////////////////////////////////////////////////////////////////////////////////
// Tiling's helper in programs running as administrator. The Switcher's tiling
// runs as the user, and Windows doesn't let it move an elevated program's
// windows; a program can always move its own, so in an elevated program in
// the user's session a hidden window takes the tiling's requests (only those
// let through from the user's level) and places the window itself.

namespace suite_tile_agent {

// Must match tiling.cpp.
constexpr WCHAR kAgentClass[] = L"TourneTileAgent_" WH_MOD_ID;
constexpr ULONG_PTR kPlaceCopyId = 0x54504C43;  // "TPLC"
struct PlaceRequest {
    uint64_t hwnd;
    RECT frame;
    int force;  // pushed onto a window that won't shrink to it
};

HANDLE g_thread;
DWORD g_threadId;

bool Elevated() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return false;
    BYTE buffer[64];
    DWORD size = 0;
    bool high = false;
    if (GetTokenInformation(token, TokenIntegrityLevel, buffer, sizeof(buffer), &size)) {
        PSID sid = ((TOKEN_MANDATORY_LABEL*)buffer)->Label.Sid;
        DWORD rid = *GetSidSubAuthority(sid, *GetSidSubAuthorityCount(sid) - 1);
        high = rid >= SECURITY_MANDATORY_HIGH_RID && rid < SECURITY_MANDATORY_SYSTEM_RID;
    }
    CloseHandle(token);
    return high;
}

RECT FrameOf(HWND h) {
    RECT r{};
    if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(h, &r);
    return r;
}

// 1: in place; 0: it wouldn't shrink to it (and wasn't forced).
LRESULT Place(HWND h, RECT const& want, bool force) {
    if (IsZoomed(h)) ShowWindow(h, SW_SHOWNOACTIVATE);
    RECT window{}, frame = FrameOf(h);
    GetWindowRect(h, &window);
    int il = frame.left - window.left, it = frame.top - window.top;
    int ir = window.right - frame.right, ib = window.bottom - frame.bottom;
    int x = want.left - il, y = want.top - it;
    int cx = (want.right - want.left) + il + ir, cy = (want.bottom - want.top) + it + ib;
    UINT flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
    SetWindowPos(h, nullptr, x, y, cx, cy, flags);
    RECT now = FrameOf(h);
    if (EqualRect(&now, &want)) return 1;
    bool bigger = now.right - now.left > want.right - want.left || now.bottom - now.top > want.bottom - want.top;
    if (bigger && !force) return 0;
    SetWindowPos(h, nullptr, x, y, cx, cy, flags | SWP_NOSENDCHANGING);
    return 1;
}

LRESULT CALLBACK AgentProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COPYDATA) {
        auto* data = (const COPYDATASTRUCT*)lParam;
        if (!data || data->dwData != kPlaceCopyId || data->cbData != sizeof(PlaceRequest)) return 0;
        PlaceRequest request;
        memcpy(&request, data->lpData, sizeof(request));
        HWND h = (HWND)(ULONG_PTR)request.hwnd;
        DWORD pid = 0;
        // Only this program's own windows.
        if (!IsWindow(h) || !GetWindowThreadProcessId(h, &pid) || pid != GetCurrentProcessId()) return 0;
        return Place(h, request.frame, request.force != 0) + 1;  // 0 stays "not handled"
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

DWORD WINAPI AgentThread(LPVOID ready) {
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&AgentProc, &instance);
    WNDCLASSW wc{};
    wc.lpfnWndProc = AgentProc;
    wc.hInstance = instance;
    wc.lpszClassName = kAgentClass;
    RegisterClassW(&wc);
    HWND wnd = CreateWindowExW(0, kAgentClass, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);
    if (wnd) {
        // Let the user-level tiling's requests through: WM_COPYDATA, and the
        // message that carries its data across.
        ChangeWindowMessageFilterEx(wnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
        ChangeWindowMessageFilterEx(wnd, 0x0049 /* WM_COPYGLOBALDATA */, MSGFLT_ALLOW, nullptr);
    }
    SetEvent(ready);
    MSG msg;
    while (wnd && GetMessageW(&msg, nullptr, 0, 0) > 0) DispatchMessageW(&msg);
    if (wnd) DestroyWindow(wnd);
    UnregisterClassW(kAgentClass, instance);
    return 0;
}

bool Start(bool switcherEnabled) {
    // The Switcher's own setting (through a variable, so the build doesn't
    // file it under a part).
    constexpr PCWSTR kElevatedSetting = L"switcher.tiling.tileElevated";
    if (!switcherEnabled || !Wh_GetIntSetting(kElevatedSetting) || !Elevated()) return false;
    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, AgentThread, ready, 0, &g_threadId);
    if (g_thread) WaitForSingleObject(ready, 2000);
    CloseHandle(ready);
    return g_thread != nullptr;
}

void Stop() {
    if (!g_thread) return;
    PostThreadMessageW(g_threadId, WM_QUIT, 0, 0);
    WaitForSingleObject(g_thread, 2000);
    CloseHandle(g_thread);
    g_thread = nullptr;
}

}  // namespace suite_tile_agent

BOOL Wh_ModInit() {
    Wh_Log(L">");

    g_suiteRole = SuiteDetermineRole();
    if (g_suiteRole == kRoleSkip) {
        return FALSE;
    }
    // Safe Mode is for fixing things: none of the suite loads there.
    if (GetSystemMetrics(SM_CLEANBOOT)) {
        return FALSE;
    }

    SuiteEvaluate();

    if (g_suiteRole == kRoleTool) {
#ifdef _WIN64
        if (!SuiteToolPartEnabled(g_partWanted) || !SuiteClaimToolProcess()) {
            ExitProcess(0);
        }
        g_partActive[kPartSystemIcons] =
            SuiteStartPart(kPartSystemIcons, part_system_icons::PartInit);
        g_partActive[kPartTaskbar] = SuiteStartPart(kPartTaskbar, part_taskbar::PartInit);
        g_partActive[kPartSwitcher] = SuiteStartPart(kPartSwitcher, part_switcher::PartInit);
        if (!SuiteToolPartEnabled(g_partActive)) {
            ExitProcess(1);
        }
        g_partActive[kPartIconRedirect] =
            SuiteStartPart(kPartIconRedirect, part_icon_redirect::PartInit);
        SuiteCommitHooks();
        // Stop windhawk.exe's own main from running; the mod's threads keep
        // the process alive.
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)GetModuleHandleW(nullptr);
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)dos + dos->e_lfanew);
        Wh_SetFunctionHook((BYTE*)dos + nt->OptionalHeader.AddressOfEntryPoint,
                           (void*)SuiteEntryPointHook, nullptr);
        return TRUE;
#else
        ExitProcess(0);
#endif
    }

    // A program running as administrator never hears Explorer's "TaskbarCreated"
    // (Windows keeps the user-level broadcast from it), so its tray icon is lost
    // whenever Explorer restarts. Let that one message through; it stays let
    // through for the program's life, whether or not the suite stays loaded.
    if (suite_tile_agent::Elevated()) {
        ChangeWindowMessageFilter(RegisterWindowMessageW(L"TaskbarCreated"), MSGFLT_ADD);
    }

    if (g_suiteRole == kRoleNormal && SuiteIsShellExplorer() && SuiteCrashGuardCheck()) {
        g_crashGuardTripped = true;
        // The taskbar (in the suite's own process) tells the user.
        UINT notice = RegisterWindowMessageW(L"TourneSuite_CrashGuard");
        for (HWND bar = nullptr;
             (bar = FindWindowExW(nullptr, bar, L"TourneTaskbarTest_" WH_MOD_ID, nullptr));) {
            PostMessageW(bar, notice, 0, 0);
        }
        // Stays loaded, doing nothing, so saving the settings brings the parts back.
        return TRUE;
    }

    g_partActive[kPartTrayIcons] = SuiteStartPart(kPartTrayIcons, part_tray_icons::PartInit);
#ifdef _WIN64
    g_partActive[kPartTrayFolders] =
        SuiteStartPart(kPartTrayFolders, part_tray_folders::PartInit);
    g_partActive[kPartClock] = SuiteStartPart(kPartClock, part_clock::PartInit);
    g_partActive[kPartExplorerRelay] =
        SuiteStartPart(kPartExplorerRelay, part_explorer_relay::PartInit);
    g_partActive[kPartExplorerStyle] =
        SuiteStartPart(kPartExplorerStyle, part_explorer_style::PartInit);
    g_partActive[kPartExplorerColors] =
        SuiteStartPart(kPartExplorerColors, part_explorer_colors::PartInit);
#endif
    g_partActive[kPartTextColor] = SuiteStartPart(kPartTextColor, part_text_color::PartInit);
    g_partActive[kPartThemeGaps] = SuiteStartPart(kPartThemeGaps, part_theme_gaps::PartInit);
    g_partActive[kPartDarkMagic] = SuiteStartPart(kPartDarkMagic, part_dark_magic::PartInit);
    g_partActive[kPartClassicMenus] = SuiteStartPart(kPartClassicMenus, part_classic_menus::PartInit);
    g_partActive[kPartMenuNoIcons] =
        SuiteStartPart(kPartMenuNoIcons, part_menu_no_icons::PartInit);
    g_partActive[kPartMenuNoDuplicates] =
        SuiteStartPart(kPartMenuNoDuplicates, part_menu_no_duplicates::PartInit);
    g_partActive[kPartIconRedirect] =
        SuiteStartPart(kPartIconRedirect, part_icon_redirect::PartInit);
    g_partActive[kPartTweaks] = SuiteStartPart(kPartTweaks, part_tweaks::PartInit);

    SuiteCommitHooks();

    // The launcher stays loaded to start the tool process; anywhere else with
    // nothing to do, let Windhawk unload the suite.
    if (g_suiteRole == kRoleLauncher) {
        return TRUE;
    }
    bool agent = suite_tile_agent::Start(g_partEnabled[kPartSwitcher]);
    return agent || std::any_of(std::begin(g_partActive), std::end(g_partActive),
                                [](bool active) { return active; });
}

void Wh_ModAfterInit() {
#ifdef _WIN64
    // The tool process's hooks (Icon Redirect's) are live now.
    if (g_suiteRole == kRoleTool) {
        if (g_partActive[kPartTaskbar]) {
            part_taskbar::PartAfterInit();
        }
        return;
    }
#endif
    if (g_suiteRole == kRoleLauncher) {
        if (SuiteToolPartEnabled(g_partEnabled)) {
            SuiteLaunchToolProcessWhenFree();
        }
        // Icon themes are downloaded here, once, rather than by every program.
        if (g_partEnabled[kPartIconRedirect]) {
            part_icon_redirect::FetchTheme();
        }
        // Settings changed while Windhawk wasn't running: offered now (the ones
        // needing admin wait for a Save).
        if (g_partEnabled[kPartTweaks]) {
            part_tweaks::Apply(true);
        }
        return;
    }
#ifdef _WIN64
    if (g_partActive[kPartTrayFolders]) {
        part_tray_folders::PartAfterInit();
    }
    if (g_partActive[kPartExplorerRelay]) {
        part_explorer_relay::PartAfterInit();
    }
    if (g_partActive[kPartExplorerStyle]) {
        part_explorer_style::PartAfterInit();
    }
#endif
    if (g_partActive[kPartTextColor]) {
        part_text_color::PartAfterInit();
    }
}

void Wh_ModBeforeUninit() {
#ifdef _WIN64
    if (g_partActive[kPartTrayFolders]) {
        part_tray_folders::PartBeforeUninit();
    }
#endif
}

void Wh_ModUninit() {
    Wh_Log(L">");

    if (g_suiteRole == kRoleTool) {
#ifdef _WIN64
        if (g_partActive[kPartSwitcher]) {
            part_switcher::PartUninit();
        }
        if (g_partActive[kPartTaskbar]) {
            part_taskbar::PartUninit();
        }
        if (g_partActive[kPartSystemIcons]) {
            part_system_icons::PartUninit();
        }
#endif
        ExitProcess(0);
    }

    SuiteStopLaunching();
    suite_tile_agent::Stop();
    SuiteCrashGuardStop();
    // Text Color goes last: it removes every window subclass the suite set.
#ifdef _WIN64
    if (g_partActive[kPartExplorerRelay]) {
        part_explorer_relay::PartUninit();
    }
    if (g_partActive[kPartTrayFolders]) {
        part_tray_folders::PartUninit();
    }
    if (g_partActive[kPartClock]) {
        part_clock::PartUninit();
    }
    if (g_partActive[kPartExplorerStyle]) {
        part_explorer_style::PartUninit();
    }
    if (g_partActive[kPartExplorerColors]) {
        part_explorer_colors::PartUninit();
    }
#endif
    if (g_partActive[kPartTrayIcons]) {
        part_tray_icons::PartUninit();
    }
    if (g_partActive[kPartTextColor]) {
        part_text_color::PartUninit();
    }
    if (g_partActive[kPartDarkMagic]) {
        part_dark_magic::PartUninit();
    }
    // Stops a theme download and a tweaks pass, which only the launcher starts.
    part_icon_redirect::StopFetch();
    part_tweaks::Stop();
}

BOOL Wh_ModSettingsChanged(BOOL* bReload) {
    Wh_Log(L">");

    // Explorer that the crash guard kept the parts out of: a save lets them back in.
    if (g_crashGuardTripped) {
        SuiteCrashGuardReset();
        *bReload = TRUE;
        return TRUE;
    }

    // A part switching on or off needs its hooks or process set up or taken
    // down, which a reload does cleanly: the tool process exits, and the
    // launcher starts a new one if System Icons is still on.
    bool enabled[kPartCount];
    bool wanted[kPartCount];
    std::copy(std::begin(g_partEnabled), std::end(g_partEnabled), enabled);
    std::copy(std::begin(g_partWanted), std::end(g_partWanted), wanted);
    SuiteEvaluate();
    // Icon Redirect builds its rules once, so new rules need a reload.
    if (!std::equal(std::begin(wanted), std::end(wanted), std::begin(g_partWanted)) ||
        (g_partActive[kPartExplorerRelay] && enabled[kPartSystemIcons] != g_partEnabled[kPartSystemIcons]) ||
        (g_partActive[kPartExplorerRelay] && enabled[kPartTaskbar] != g_partEnabled[kPartTaskbar]) ||
        (g_suiteRole == kRoleLauncher &&
         (enabled[kPartSystemIcons] != g_partEnabled[kPartSystemIcons] ||
          enabled[kPartTaskbar] != g_partEnabled[kPartTaskbar] ||
          enabled[kPartSwitcher] != g_partEnabled[kPartSwitcher])) ||
        (g_partActive[kPartIconRedirect] && part_icon_redirect::NeedsReload()) ||
        (g_partActive[kPartThemeGaps] && part_theme_gaps::NeedsReload()) ||
        (g_partActive[kPartClassicMenus] && part_classic_menus::NeedsReload()) ||
#ifdef _WIN64
        (g_partActive[kPartExplorerColors] && part_explorer_colors::NeedsReload()) ||
#endif
        (g_partActive[kPartTweaks] && part_tweaks::NeedsReload())) {
        *bReload = TRUE;
        return TRUE;
    }
    if (g_suiteRole == kRoleLauncher && g_partEnabled[kPartIconRedirect]) {
        part_icon_redirect::FetchTheme();
    }
    if (g_suiteRole == kRoleLauncher && g_partEnabled[kPartTweaks]) {
        part_tweaks::Apply(false);
    }

    if (g_partActive[kPartTrayIcons]) {
        part_tray_icons::PartSettingsChanged();
    }
    if (g_partActive[kPartThemeGaps]) {
        part_theme_gaps::PartSettingsChanged();
    }
    if (g_partActive[kPartDarkMagic]) {
        part_dark_magic::PartSettingsChanged();
    }
    if (g_partActive[kPartClassicMenus]) {
        part_classic_menus::PartSettingsChanged();
    }
#ifdef _WIN64
    if (g_partActive[kPartTrayFolders]) {
        part_tray_folders::PartSettingsChanged();
    }
    if (g_partActive[kPartClock]) {
        part_clock::PartSettingsChanged();
    }
    if (g_partActive[kPartExplorerStyle]) {
        part_explorer_style::PartSettingsChanged();
    }
    if (g_partActive[kPartExplorerColors]) {
        part_explorer_colors::PartSettingsChanged();
    }
    if (g_partActive[kPartSystemIcons]) {
        part_system_icons::PartSettingsChanged();
    }
    if (g_partActive[kPartTaskbar]) {
        part_taskbar::PartSettingsChanged();
    }
    if (g_partActive[kPartSwitcher]) {
        part_switcher::PartSettingsChanged();
    }
    if (g_partActive[kPartExplorerRelay]) {
        part_explorer_relay::PartSettingsChanged();
    }
#endif
    if (g_partActive[kPartTextColor]) {
        part_text_color::PartSettingsChanged();
    }
    if (g_partActive[kPartMenuNoIcons]) {
        part_menu_no_icons::PartSettingsChanged();
    }
    if (g_partActive[kPartMenuNoDuplicates]) {
        part_menu_no_duplicates::PartSettingsChanged();
    }
    return TRUE;
}
