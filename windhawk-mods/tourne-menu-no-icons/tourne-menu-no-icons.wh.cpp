// ==WindhawkMod==
// @id                  tourne-menu-no-icons
// @name                Tourne'Menu [No Icons]
// @description         Strips the icons from context menu items, leaving the text alone
// @version             1.0.0
// @author              USER-TOURNE
// @github              https://github.com/USER-TOURNE
// @license             MIT
// @include             *
// @architecture        x86-64
// @compilerOptions     -luser32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Menu` **[No Icons]**

Removes the icons from context menu items, so a menu is text and nothing else.

An icon on a menu item is a bitmap hung off the item itself: `MIIM_BITMAP` in the mask
and an `HBITMAP` in `hbmpItem`. Whoever builds the menu sets it, and there is no system
switch to turn it off. This mod clears it as the item goes in.

The two entry points that matter are `InsertMenuItemW`, which adds an item, and
`SetMenuItemInfoW`, which changes one already added. Both live in user32, and anything
building a menu from outside user32 has to call them to do it, which is what makes this
reachable at all. That covers the shell's own menus, the `New` submenu among them, and
any application or shell extension that adds items the ordinary way.

`SetMenuItemBitmaps` is covered too. It is the older call, and while it is meant for the
check mark rather than an icon, some applications use it as one.

## What it leaves alone

**System glyphs.** A handful of reserved `HBMMENU_` values ask Windows to draw a control
rather than an icon: the close and minimise buttons on a menu bar, for one. Those are not
decoration, and clearing them leaves a blank, dead control, so they are kept.

**Owner-drawn items.** An item marked `HBMMENU_CALLBACK`, or `MFT_OWNERDRAW`, paints
itself in response to `WM_DRAWITEM`. There is no bitmap to remove, because the icon is
drawn later by whoever owns the menu. Those are reported under **Verbose logging** rather
than touched, so a menu that does not respond to this mod can be identified as such
instead of looking like a failure.

**Anything not drawn with a Win32 menu at all.** The Windows 11 modern context menu is
XAML and never calls these functions. Use the classic menu, which this machine already
does, or a Windhawk styler.

## Scope

**Processes** decides where the mod loads, as a list of executable names. `*` means
everywhere, which is a big hammer. The default list is the shell and the surfaces that
host shell menus.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- processes: explorer.exe, litemenu.exe, dllhost.exe, rundll32.exe
  $name: Processes
  $description: 'Comma separated executable names to load in. Use * for every process.'
- logging: false
  $name: Verbose logging
  $description: Log each item whose icon is removed, and each one left alone because it draws itself.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>

bool g_logging = false;

// A menu item's text, for logging only. Present only when the caller passed a string,
// which it often has not.
static PCWSTR ItemText(const MENUITEMINFOW* info) {
    if (info && (info->fMask & MIIM_STRING) && info->dwTypeData) {
        return info->dwTypeData;
    }
    return L"(no text)";
}

// Reserved values that ask Windows to draw a control rather than show a picture. They run
// from HBMMENU_CALLBACK, which is -1, through the small integers up to
// HBMMENU_POPUP_MINIMIZE. Clearing one leaves a blank control where a button should be.
static bool IsReservedBitmap(HBITMAP bitmap) {
    if (bitmap == HBMMENU_CALLBACK) {
        return true;
    }
    ULONG_PTR value = reinterpret_cast<ULONG_PTR>(bitmap);
    return value >= 1 && value <= reinterpret_cast<ULONG_PTR>(HBMMENU_POPUP_MINIMIZE);
}

// Fills a copy with the icon removed. Returns false when there is nothing to do, in which
// case the caller passes the original through untouched.
static bool StripIcon(const MENUITEMINFOW* source, MENUITEMINFOW* copy) {
    if (!source || source->cbSize < sizeof(MENUITEMINFOW)) {
        return false;
    }
    if (!(source->fMask & MIIM_BITMAP) || !source->hbmpItem) {
        return false;
    }

    if (IsReservedBitmap(source->hbmpItem)) {
        if (g_logging) {
            Wh_Log(L"left alone, draws itself: \"%s\"", ItemText(source));
        }
        return false;
    }

    *copy = *source;
    copy->fMask &= ~MIIM_BITMAP;
    copy->hbmpItem = nullptr;

    if (g_logging) {
        Wh_Log(L"removed icon from \"%s\"", ItemText(source));
    }
    return true;
}

using InsertMenuItemW_t = BOOL(WINAPI*)(HMENU, UINT, BOOL, LPCMENUITEMINFOW);
InsertMenuItemW_t InsertMenuItemW_Original;

BOOL WINAPI InsertMenuItemW_Hook(HMENU hMenu, UINT item, BOOL byPosition,
                                 LPCMENUITEMINFOW lpmi) {
    MENUITEMINFOW copy;
    if (StripIcon(lpmi, &copy)) {
        return InsertMenuItemW_Original(hMenu, item, byPosition, &copy);
    }
    return InsertMenuItemW_Original(hMenu, item, byPosition, lpmi);
}

using SetMenuItemInfoW_t = BOOL(WINAPI*)(HMENU, UINT, BOOL, LPCMENUITEMINFOW);
SetMenuItemInfoW_t SetMenuItemInfoW_Original;

BOOL WINAPI SetMenuItemInfoW_Hook(HMENU hMenu, UINT item, BOOL byPosition,
                                  LPCMENUITEMINFOW lpmi) {
    MENUITEMINFOW copy;
    if (StripIcon(lpmi, &copy)) {
        return SetMenuItemInfoW_Original(hMenu, item, byPosition, &copy);
    }
    return SetMenuItemInfoW_Original(hMenu, item, byPosition, lpmi);
}

// The older call. It sets the check mark rather than an icon, but it is close enough to
// one on screen that some applications use it that way.
using SetMenuItemBitmaps_t = BOOL(WINAPI*)(HMENU, UINT, UINT, HBITMAP, HBITMAP);
SetMenuItemBitmaps_t SetMenuItemBitmaps_Original;

BOOL WINAPI SetMenuItemBitmaps_Hook(HMENU hMenu, UINT position, UINT flags,
                                    HBITMAP unchecked, HBITMAP checked) {
    if (g_logging && (unchecked || checked)) {
        Wh_Log(L"removed check-mark bitmaps at position %u", position);
    }
    return SetMenuItemBitmaps_Original(hMenu, position, flags, nullptr, nullptr);
}

// --- settings and lifecycle ----------------------------------------------

static void LoadSettings() {
    g_logging = Wh_GetIntSetting(L"logging") != 0;
}

// Compares one comma separated entry against this process, ignoring surrounding space.
static bool EntryMatchesProcess(PCWSTR begin, PCWSTR end, PCWSTR processName) {
    while (begin < end && (*begin == L' ' || *begin == L'\t')) {
        begin++;
    }
    while (end > begin && (end[-1] == L' ' || end[-1] == L'\t')) {
        end--;
    }

    if (begin == end) {
        return false;
    }
    if (end - begin == 1 && *begin == L'*') {
        return true;
    }

    size_t length = (size_t)(end - begin);
    return wcslen(processName) == length && _wcsnicmp(begin, processName, length) == 0;
}

static bool ShouldLoadInThisProcess() {
    WCHAR path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, ARRAYSIZE(path))) {
        return false;
    }

    PCWSTR processName = wcsrchr(path, L'\\');
    processName = processName ? processName + 1 : path;

    PCWSTR processes = Wh_GetStringSetting(L"processes");
    bool matched = false;

    PCWSTR entry = processes;
    for (PCWSTR p = processes;; p++) {
        if (*p == L',' || !*p) {
            if (EntryMatchesProcess(entry, p, processName)) {
                matched = true;
                break;
            }
            if (!*p) {
                break;
            }
            entry = p + 1;
        }
    }

    Wh_FreeStringSetting(processes);

    if (!matched) {
        Wh_Log(L"%s is not in the process list, not loading", processName);
    }

    return matched;
}

static bool HookExport(HMODULE module, const char* name, void* hook, void** original) {
    void* target = (void*)GetProcAddress(module, name);
    if (!target) {
        Wh_Log(L"missing export %S", name);
        return false;
    }
    return Wh_SetFunctionHook(target, hook, original);
}

BOOL Wh_ModInit() {
    if (!ShouldLoadInThisProcess()) {
        return FALSE;
    }

    LoadSettings();

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) {
        Wh_Log(L"failed to resolve user32");
        return FALSE;
    }

    HookExport(user32, "InsertMenuItemW", (void*)InsertMenuItemW_Hook,
               (void**)&InsertMenuItemW_Original);
    HookExport(user32, "SetMenuItemInfoW", (void*)SetMenuItemInfoW_Hook,
               (void**)&SetMenuItemInfoW_Original);
    HookExport(user32, "SetMenuItemBitmaps", (void*)SetMenuItemBitmaps_Hook,
               (void**)&SetMenuItemBitmaps_Original);

    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
