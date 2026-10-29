// ==WindhawkMod==
// @id                  tourne-change-tray-icons
// @name                Tourne'Tray [Change Tray Icons]
// @description         Change all tray icons for an application to another icon, ANSI callers included
// @version             1.1.0
// @author              USER-TOURNE
// @github              https://github.com/USER-TOURNE
// @license             GPL-3.0
// @compilerOptions     -lshell32 -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Tray` **[Change Tray Icons]**

Replaces every tray icon an application shows with one of your own.

A fork of **Change Tray Icons** by aubymori, with three changes.

## Applications that use the ANSI API

`Shell_NotifyIcon` has an ANSI form and a wide form, and the original hooked only the
wide one. An application built against the ANSI API was therefore invisible to it, with
no error and nothing in the log to say so.

Steam is one of these. Its tray icon is not created by `steam.exe` at all but by
`steamwebhelper.exe`, and that binary imports `Shell_NotifyIconA`. Both forms are hooked
here, so it no longer matters which one an application reaches for.

## Blank rows no longer truncate the list

The original stopped scanning at the first row with an empty application field. A blank
row is easy to leave behind when removing an entry, and every row after it was then
ignored, silently, with the settings page still showing them all. One stray blank near
the top of a long list could disable almost all of it.

This version scans the whole list and skips blanks, so a gap costs nothing. It also stops
at the first row that matches rather than the last, which only matters if you have two
rows for the same application, and first is the one you would expect to win.

## How to use

The mod only loads into applications you name, so each one has to be added to the
**Custom process inclusion list** on the **Advanced** tab, as well as getting a row in
the settings below. Both are required, and missing either one looks identical from the
outside: nothing happens.

Either a full path or a bare executable name will match. Restart the application
afterwards, since a tray icon already on screen is not redrawn until the application
sends it again.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- icons:
  - - path: Discord.exe
      $name: Application name or path
    - icon: C:\Classic\mIRC.ico
      $name: ICO file path
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <shellapi.h>

#include <stddef.h>
#include <wchar.h>

HICON g_hIcon = NULL;

// cbSize has to reach past the start of hIcon, not merely up to it, or writing the field
// runs off the end of a struct the caller sized smaller.
#define CAN_SET_ICON(lpData, type) \
    ((lpData) && (lpData)->cbSize >= offsetof(type, hIcon) + sizeof(HICON) && \
     ((lpData)->uFlags & NIF_ICON))

using Shell_NotifyIconW_t = decltype(&Shell_NotifyIconW);
Shell_NotifyIconW_t Shell_NotifyIconW_orig;

BOOL WINAPI Shell_NotifyIconW_hook(DWORD dwMessage, PNOTIFYICONDATAW lpData)
{
    if (g_hIcon && CAN_SET_ICON(lpData, NOTIFYICONDATAW))
    {
        lpData->hIcon = g_hIcon;
    }
    return Shell_NotifyIconW_orig(dwMessage, lpData);
}

// The half the original was missing. Steam reaches the tray through this one.
using Shell_NotifyIconA_t = decltype(&Shell_NotifyIconA);
Shell_NotifyIconA_t Shell_NotifyIconA_orig;

BOOL WINAPI Shell_NotifyIconA_hook(DWORD dwMessage, PNOTIFYICONDATAA lpData)
{
    if (g_hIcon && CAN_SET_ICON(lpData, NOTIFYICONDATAA))
    {
        lpData->hIcon = g_hIcon;
    }
    return Shell_NotifyIconA_orig(dwMessage, lpData);
}

// A blank row reads as an empty string, and is indistinguishable from the end of the
// list, so the list is scanned to a fixed ceiling instead of stopping at the first gap.
constexpr int kMaxIcons = 128;

static void LoadIconForSize(LPCWSTR pszIconPath)
{
    int cxIcon, cyIcon;

    typedef UINT (WINAPI *GetSystemMetricsForDpi_t)(int nIndex, UINT dpi);
    static GetSystemMetricsForDpi_t pfnGetSystemMetricsForDpi =
        (GetSystemMetricsForDpi_t)GetProcAddress(GetModuleHandleW(L"user32.dll"),
                                                 "GetSystemMetricsForDpi");
    if (pfnGetSystemMetricsForDpi)
    {
        HDC hdc = GetDC(NULL);
        int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
        ReleaseDC(NULL, hdc);

        cxIcon = pfnGetSystemMetricsForDpi(SM_CXSMICON, dpi);
        cyIcon = pfnGetSystemMetricsForDpi(SM_CYSMICON, dpi);
    }
    else
    {
        cxIcon = GetSystemMetrics(SM_CXSMICON);
        cyIcon = GetSystemMetrics(SM_CYSMICON);
    }

    g_hIcon = (HICON)LoadImageW(NULL, pszIconPath, IMAGE_ICON, cxIcon, cyIcon,
                                LR_LOADFROMFILE | LR_DEFAULTCOLOR);
    if (!g_hIcon)
    {
        Wh_Log(L"could not load icon: %s", pszIconPath);
    }
}

void Wh_ModSettingsChanged(void)
{
    if (g_hIcon)
    {
        DestroyIcon(g_hIcon);
        g_hIcon = NULL;
    }

    WCHAR szAppPath[MAX_PATH];
    if (!GetModuleFileNameW(GetModuleHandleW(NULL), szAppPath, MAX_PATH))
    {
        return;
    }
    WCHAR *pBackslash = wcsrchr(szAppPath, L'\\');

    for (int i = 0; i < kMaxIcons; i++)
    {
        LPCWSTR pszPath = Wh_GetStringSetting(L"icons[%d].path", i);

        // Either the full path or the bare executable name is accepted.
        bool matched = *pszPath &&
                       (0 == _wcsicmp(szAppPath, pszPath) ||
                        (pBackslash && 0 == _wcsicmp(pBackslash + 1, pszPath)));
        Wh_FreeStringSetting(pszPath);

        if (!matched)
        {
            continue;
        }

        LPCWSTR pszIconPath = Wh_GetStringSetting(L"icons[%d].icon", i);
        LoadIconForSize(pszIconPath);
        Wh_FreeStringSetting(pszIconPath);

        Wh_Log(L"matched row %d for %s", i, szAppPath);
        break;
    }
}

BOOL Wh_ModInit(void)
{
    Wh_ModSettingsChanged();

    BOOL wide = Wh_SetFunctionHook((void *)Shell_NotifyIconW,
                                   (void *)Shell_NotifyIconW_hook,
                                   (void **)&Shell_NotifyIconW_orig);

    BOOL ansi = Wh_SetFunctionHook((void *)Shell_NotifyIconA,
                                   (void *)Shell_NotifyIconA_hook,
                                   (void **)&Shell_NotifyIconA_orig);

    // Either one alone is still worth having, since an application uses one or the other.
    return wide || ansi;
}

void Wh_ModUninit(void)
{
    if (g_hIcon)
    {
        DestroyIcon(g_hIcon);
    }
}
