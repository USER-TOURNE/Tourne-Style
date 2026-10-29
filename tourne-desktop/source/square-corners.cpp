// ==WindhawkMod==
// @id              local@tourne-square-corners
// @name            Tourne Square Corners
// @description     Windows 11's rounded window corners made square again, in every program the suite runs in
// @version         1.0.0
// @author          Tourne
// @include         *
// @compilerOptions -ldwmapi -ladvapi32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- excludeApps: ''
  $name: Leave these programs rounded
  $description: Program file names, separated by commas, like game.exe.
*/
// ==/WindhawkModSettings==

// Windows 11 rounds a window's corners unless the window asks it not to
// (DWMWA_WINDOW_CORNER_PREFERENCE, which only the window's own program may
// set). The suite already runs in every program, so each asks for square
// corners on its own top-level windows: the ones there when it starts, each
// one made after (tooltips too), each menu as it opens, and any program's own
// corner choice turned to square. Nothing runs inside dwm.exe. Rounding drawn
// inside a window (XAML apps' own popups and flyouts) isn't DWM's, so it stays.
//
// The switch is Studio's ("Square window corners", Taskbar page): Tourne
// Desktop writes it to HKCU\Software\Tourne\Corners, Square, and each program
// waits on that key (a thread-pool wait, no thread of its own, no polling), so
// turning it on or off squares or rounds every open window at once.

#include <windows.h>
#include <dwmapi.h>

#include <atomic>
#include <string>

namespace {

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
constexpr DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33;
#endif
constexpr DWORD kSquare = 1, kWindowsChoice = 0;  // DWMWCP_DONOTROUND, DWMWCP_DEFAULT
constexpr wchar_t kKey[] = L"Software\\Tourne\\Corners";

using DwmSetWindowAttribute_t = decltype(&DwmSetWindowAttribute);
using CreateWindowExW_t = decltype(&CreateWindowExW);
using CreateWindowExA_t = decltype(&CreateWindowExA);
DwmSetWindowAttribute_t DwmSetWindowAttribute_Original;
CreateWindowExW_t CreateWindowExW_Original;
CreateWindowExA_t CreateWindowExA_Original;
std::atomic<bool> g_square{false};
HKEY g_key = nullptr;
HANDLE g_changed = nullptr, g_wait = nullptr;

void SetCorners(HWND wnd, DWORD preference) {
    auto set = DwmSetWindowAttribute_Original ? DwmSetWindowAttribute_Original : DwmSetWindowAttribute;
    set(wnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}

// A top-level window of its own (not a child, not message-only).
bool TopLevel(HWND parent, DWORD style) { return parent != HWND_MESSAGE && !(style & WS_CHILD); }

HRESULT WINAPI DwmSetWindowAttribute_Hook(HWND wnd, DWORD attribute, LPCVOID value, DWORD size) {
    if (g_square && attribute == DWMWA_WINDOW_CORNER_PREFERENCE && value && size == sizeof(DWORD)) {
        DWORD square = kSquare;
        return DwmSetWindowAttribute_Original(wnd, attribute, &square, size);
    }
    return DwmSetWindowAttribute_Original(wnd, attribute, value, size);
}

HWND WINAPI CreateWindowExW_Hook(DWORD exStyle, LPCWSTR className, LPCWSTR name, DWORD style, int x, int y, int w, int h,
                                 HWND parent, HMENU menu, HINSTANCE instance, LPVOID param) {
    HWND wnd = CreateWindowExW_Original(exStyle, className, name, style, x, y, w, h, parent, menu, instance, param);
    if (wnd && g_square && TopLevel(parent, style)) SetCorners(wnd, kSquare);
    return wnd;
}

HWND WINAPI CreateWindowExA_Hook(DWORD exStyle, LPCSTR className, LPCSTR name, DWORD style, int x, int y, int w, int h,
                                 HWND parent, HMENU menu, HINSTANCE instance, LPVOID param) {
    HWND wnd = CreateWindowExA_Original(exStyle, className, name, style, x, y, w, h, parent, menu, instance, param);
    if (wnd && g_square && TopLevel(parent, style)) SetCorners(wnd, kSquare);
    return wnd;
}

// Menus are made by Windows itself (not through CreateWindowEx), so each is caught as it opens: an in-context
// event hook for this program's own menus only, called on the menu's own thread, one event a menu.
HWINEVENTHOOK g_menuHook = nullptr;

void CALLBACK MenuOpened(HWINEVENTHOOK, DWORD, HWND wnd, LONG, LONG, DWORD, DWORD) {
    if (wnd && g_square) SetCorners(wnd, kSquare);
}

void WatchMenus() {
    HMODULE self = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&MenuOpened, &self);
    if (self)
        g_menuHook = SetWinEventHook(EVENT_SYSTEM_MENUPOPUPSTART, EVENT_SYSTEM_MENUPOPUPSTART, self, MenuOpened,
                                     GetCurrentProcessId(), 0, WINEVENT_INCONTEXT);
}

BOOL CALLBACK SetOwnCorners(HWND wnd, LPARAM preference) {
    DWORD pid = 0;
    GetWindowThreadProcessId(wnd, &pid);
    if (pid == GetCurrentProcessId()) SetCorners(wnd, (DWORD)preference);
    return TRUE;
}

// This program's top-level windows, each given the preference.
void SetAllCorners(DWORD preference) { EnumWindows(SetOwnCorners, preference); }

bool SquareWanted() {
    DWORD value = 0, size = sizeof(value);
    return RegGetValueW(HKEY_CURRENT_USER, kKey, L"Square", RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS &&
           value != 0;
}

// The key changed: watched again, and every window squared or rounded if the switch moved.
void CALLBACK KeyChanged(PVOID, BOOLEAN) {
    if (g_key) RegNotifyChangeKeyValue(g_key, FALSE, REG_NOTIFY_CHANGE_LAST_SET, g_changed, TRUE);
    bool square = SquareWanted();
    if (square != g_square.exchange(square)) SetAllCorners(square ? kSquare : kWindowsChoice);
}

void StartWatching() {
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kKey, 0, nullptr, 0, KEY_NOTIFY | KEY_QUERY_VALUE, nullptr, &g_key, nullptr)) return;
    g_changed = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_changed || RegNotifyChangeKeyValue(g_key, FALSE, REG_NOTIFY_CHANGE_LAST_SET, g_changed, TRUE)) return;
    RegisterWaitForSingleObject(&g_wait, g_changed, KeyChanged, nullptr, INFINITE, WT_EXECUTEDEFAULT);
}

void StopWatching() {
    if (g_wait) UnregisterWaitEx(g_wait, INVALID_HANDLE_VALUE);  // (waits for a change being handled)
    g_wait = nullptr;
    if (g_key) RegCloseKey(g_key);
    g_key = nullptr;
    if (g_changed) CloseHandle(g_changed);
    g_changed = nullptr;
}

bool Excluded() {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) return false;
    PCWSTR slash = wcsrchr(path, L'\\');
    std::wstring name = slash ? slash + 1 : path;
    PCWSTR setting = Wh_GetStringSetting(L"excludeApps");
    std::wstring list = setting ? setting : L"";
    Wh_FreeStringSetting(setting);
    size_t start = 0;
    while (start <= list.size()) {
        size_t comma = list.find(L',', start);
        std::wstring item = list.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty() && _wcsicmp(item.c_str(), name.c_str()) == 0) return true;
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return false;
}

}  // namespace

// The suite asks before starting the part here: not in a program left rounded.
bool ShouldLoadInThisProcess() { return !Excluded(); }

BOOL Wh_ModInit() {
    if (Excluded()) return FALSE;
    HMODULE dwm = LoadLibraryExW(L"dwmapi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    HMODULE user = GetModuleHandleW(L"user32.dll");
    if (!dwm || !user) return FALSE;
    Wh_SetFunctionHook((void*)GetProcAddress(dwm, "DwmSetWindowAttribute"), (void*)DwmSetWindowAttribute_Hook,
                       (void**)&DwmSetWindowAttribute_Original);
    Wh_SetFunctionHook((void*)GetProcAddress(user, "CreateWindowExW"), (void*)CreateWindowExW_Hook,
                       (void**)&CreateWindowExW_Original);
    Wh_SetFunctionHook((void*)GetProcAddress(user, "CreateWindowExA"), (void*)CreateWindowExA_Hook,
                       (void**)&CreateWindowExA_Original);
    g_square = SquareWanted();
    if (g_square) SetAllCorners(kSquare);  // the windows already there
    StartWatching();
    WatchMenus();
    return TRUE;
}

void Wh_ModUninit() {
    if (g_menuHook) UnhookWinEvent(g_menuHook);
    g_menuHook = nullptr;
    StopWatching();
    if (g_square.exchange(false)) SetAllCorners(kWindowsChoice);
}

// Left rounded now: the suite loads again without the part here.
bool NeedsReload() { return Excluded(); }

void Wh_ModSettingsChanged() {}
