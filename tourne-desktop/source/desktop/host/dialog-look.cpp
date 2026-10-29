// The Open and Save dialogs Tourne Desktop shows itself (picking a wallpaper, an icon, a file in Studio),
// in the shell's look: the Windhawk suite and Tourne Menus stay out of this program, so what Theme Gaps and
// Tourne Menus do in every other program is done here, for these dialogs only.
//
// - The footer (File name, the buttons): Windows paints it a fixed #383838 in dark mode, whatever the
//   visual style says. As Theme Gaps does, that colour is caught where the shell's and the dialog's own
//   code asks GDI for it, and the shell's ground put in its place.
// - Their right-click menus: in Tourne's menu look, drawn by the taskbar's menu code (taskbar-host.cpp,
//   TourneStyledShellMenu), with the menu rules Tourne Menus applies everywhere else.

#include <windows.h>

#include "td.h"

namespace td {

namespace {

constexpr COLORREF kWindowsFooter = RGB(0x38, 0x38, 0x38);
constexpr COLORREF kTourneGround = RGB(0x12, 0x1c, 0x21);  // (the tourne scheme's, until the bar publishes its own)

// The shell's ground: the menus' as the bar publishes them for Tourne Menus (HKCU\Software\Tourne\Menus),
// read again now and then since painting asks often.
COLORREF ShellGround() {
    static COLORREF ground = kTourneGround;
    static DWORD readAt = 0;
    static bool read = false;
    DWORD now = GetTickCount();
    if (read && now - readAt < 2000) return ground;
    DWORD value = 0, size = sizeof(value);
    if (!RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Menus", L"Ground", RRF_RT_REG_DWORD, nullptr, &value, &size))
        ground = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);  // (published as 0xRRGGBB)
    readAt = now;
    read = true;
    return ground;
}

// The shell's and dialogs' own code, where the footer's colour comes from (as Theme Gaps lists it).
bool FromDialogCode(void* returnAddress) {
    HMODULE caller = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCWSTR)returnAddress, &caller) ||
        !caller)
        return false;
    for (PCWSTR name : {L"dui70.dll", L"duser.dll", L"explorerframe.dll", L"shell32.dll", L"comdlg32.dll",
                        L"windows.storage.dll"}) {
        if (caller == GetModuleHandleW(name)) return true;
    }
    return false;
}

COLORREF Footer(COLORREF color, void* returnAddress) {
    if ((color & 0xFFFFFF) != kWindowsFooter || !FromDialogCode(returnAddress)) return color;
    return ShellGround();
}

using CreateSolidBrush_t = HBRUSH(WINAPI*)(COLORREF);
using SetDCBrushColor_t = COLORREF(WINAPI*)(HDC, COLORREF);
using SetBkColor_t = COLORREF(WINAPI*)(HDC, COLORREF);
CreateSolidBrush_t g_createSolidBrush;
SetDCBrushColor_t g_setDcBrushColor;
SetBkColor_t g_setBkColor;

HBRUSH WINAPI CreateSolidBrushHere(COLORREF color) {
    return g_createSolidBrush(Footer(color, __builtin_return_address(0)));
}
COLORREF WINAPI SetDCBrushColorHere(HDC dc, COLORREF color) {
    return g_setDcBrushColor(dc, Footer(color, __builtin_return_address(0)));
}
COLORREF WINAPI SetBkColorHere(HDC dc, COLORREF color) {
    return g_setBkColor(dc, Footer(color, __builtin_return_address(0)));
}

bool ClassIs(HWND wnd, PCWSTR name) {
    WCHAR cls[64];
    return wnd && GetClassNameW(wnd, cls, ARRAYSIZE(cls)) && !_wcsicmp(cls, name);
}

BOOL CALLBACK FindShellView(HWND child, LPARAM lp) {
    if (ClassIs(child, L"SHELLDLL_DefView") || ClassIs(child, L"DUIViewWndClassName")) {
        *(bool*)lp = true;
        return FALSE;
    }
    return TRUE;
}

// A dialog with a folder view in it: the Open, Save and folder pickers (as Tourne Menus tells them).
bool IsFileDialog(HWND root) {
    if (!ClassIs(root, L"#32770")) return false;
    bool found = false;
    EnumChildWindows(root, FindShellView, (LPARAM)&found);
    return found;
}

using StyledShellMenu_t = UINT (*)(HMENU, UINT, int, int, HWND, RECT const*);
thread_local bool t_busy = false;  // a menu drawn here is open on this thread

// Whether this menu is one of a file dialog's: its owner there, on this thread; Shift held keeps Windows' own.
bool DialogMenu(HMENU menu, HWND owner) {
    if (t_busy || !menu || !IsMenu(menu) || !owner || GetWindowThreadProcessId(owner, nullptr) != GetCurrentThreadId())
        return false;
    if (GetKeyState(VK_SHIFT) < 0) return false;
    return IsFileDialog(GetAncestor(owner, GA_ROOT));
}

StyledShellMenu_t StyledShellMenu() {
    static StyledShellMenu_t styled = (StyledShellMenu_t)GetProcAddress(GetModuleHandleW(nullptr), "TourneStyledShellMenu");
    return styled;
}

using TrackPopupMenuEx_t = decltype(&TrackPopupMenuEx);
using TrackPopupMenu_t = decltype(&TrackPopupMenu);
TrackPopupMenuEx_t g_trackPopupMenuEx;
TrackPopupMenu_t g_trackPopupMenu;

BOOL WINAPI TrackPopupMenuExHere(HMENU menu, UINT flags, int x, int y, HWND owner, LPTPMPARAMS params) {
    StyledShellMenu_t styled = StyledShellMenu();
    if (!styled || !DialogMenu(menu, owner)) return g_trackPopupMenuEx(menu, flags, x, y, owner, params);
    t_busy = true;
    UINT result = styled(menu, flags, x, y, owner, params ? &params->rcExclude : nullptr);
    t_busy = false;
    return (BOOL)result;
}

BOOL WINAPI TrackPopupMenuHere(HMENU menu, UINT flags, int x, int y, int reserved, HWND owner, const RECT* rect) {
    StyledShellMenu_t styled = StyledShellMenu();
    if (!styled || !DialogMenu(menu, owner)) return g_trackPopupMenu(menu, flags, x, y, reserved, owner, rect);
    t_busy = true;
    UINT result = styled(menu, flags, x, y, owner, nullptr);
    t_busy = false;
    return (BOOL)result;
}

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"dialogs", format, args);
    va_end(args);
}

}  // namespace

// Once, as Tourne Desktop starts.
void DialogLookStart() {
    static bool done;
    if (done) return;
    done = true;
    // The implementations, so calls through gdi32's forwarders are caught too (as Theme Gaps hooks them).
    HMODULE gdi = GetModuleHandleW(L"gdi32full.dll");
    if (!gdi) gdi = LoadLibraryW(L"gdi32full.dll");
    if (!gdi) gdi = GetModuleHandleW(L"gdi32.dll");
    bool footer = gdi &&
                  SetFunctionHook(L"dialogs", (void*)GetProcAddress(gdi, "CreateSolidBrush"), (void*)CreateSolidBrushHere,
                                  (void**)&g_createSolidBrush) &&
                  SetFunctionHook(L"dialogs", (void*)GetProcAddress(gdi, "SetDCBrushColor"), (void*)SetDCBrushColorHere,
                                  (void**)&g_setDcBrushColor) &&
                  SetFunctionHook(L"dialogs", (void*)GetProcAddress(gdi, "SetBkColor"), (void*)SetBkColorHere,
                                  (void**)&g_setBkColor);
    bool menus = SetFunctionHook(L"dialogs", (void*)&TrackPopupMenuEx, (void*)TrackPopupMenuExHere, (void**)&g_trackPopupMenuEx) &&
                 SetFunctionHook(L"dialogs", (void*)&TrackPopupMenu, (void*)TrackPopupMenuHere, (void**)&g_trackPopupMenu);
    ApplyHooks();
    if (!footer) Log(L"the file dialogs' footer couldn't be set up (Windows' colour stays)");
    if (!menus) Log(L"the file dialogs' menus couldn't be set up (Windows' own stay)");
}

}  // namespace td
