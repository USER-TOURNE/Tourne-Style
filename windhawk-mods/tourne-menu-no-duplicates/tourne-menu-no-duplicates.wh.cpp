// ==WindhawkMod==
// @id                  tourne-menu-no-duplicates
// @name                Tourne'Menu [No Duplicates]
// @description         Removes the extra copy of a context menu item that shows up twice
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
# `Tourne'Menu` **[No Duplicates]**

Removes the extra copy of a menu item that appears twice, so each entry shows once.

Some applications register their context menu through two mechanisms at once. Convertify
is the case this was written for: it installs classic registry verbs and also a packaged
shell extension, and on this machine Explorer draws both, giving two identical
`Convertify` submenus. Hiding the registry verb with `ProgrammaticAccessOnly` or
`LegacyDisable` does not help here, because the menu Explorer shows ignores both.

## How

Every popup menu in a process is shown through `NtUserTrackPopupMenuEx` in win32u, which
is where user32's `TrackPopupMenu` and `TrackPopupMenuEx` end up. This mod looks at the
menu just before it opens, when every top-level item is in place, and deletes the extra
copies. If that export is missing it hooks the two user32 calls instead.

Doing it at that moment, rather than as items are inserted, is deliberate. The item
inserted second is not necessarily the one lower down the menu, because the shell inserts
by position, so only a finished menu can say which copy is first and which is last.

## What counts as a duplicate

Two **top-level** items whose text reads the same: compared without the `&` accelerator
marker, without the shortcut column after a tab, without zero-width characters, and
ignoring case. They must also agree on whether they open a submenu, so a plain command
and a submenu that happen to share a name are left alone.

**Items** limits this to the names listed. `*` applies it to any item that appears twice,
which is the broader hammer.

**Keep** chooses the survivor. `last` keeps the lower copy, which for Convertify is the
packaged shell extension; the upper one is the registry verb.

## What it leaves alone

Submenus, separators, owner-drawn items and bitmap items are never touched. The Windows
11 modern context menu is XAML and never goes through these functions, so this only
affects the classic menu, which this machine already uses.

## Scope

**Processes** decides where the mod loads, as a list of executable names. Explorer covers
the desktop and folder windows. Add an application's executable to cover the file browser
inside its Open and Save dialogs.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- names: Convertify
  $name: Items
  $description: 'Comma separated item names to de-duplicate, matched without the & accelerator and ignoring case. Use * for any item that appears twice.'
- keep: last
  $name: Keep
  $description: Which copy survives when an item appears more than once.
  $options:
  - first: The first, higher up the menu
  - last: The last, lower down the menu
- processes: explorer.exe
  $name: Processes
  $description: 'Comma separated executable names to load in. Use * for every process.'
- logging: false
  $name: Verbose logging
  $description: Log the top-level items of every menu as it opens, and each copy removed.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

struct Settings {
    std::vector<std::wstring> names;  // already normalized
    bool anyName = false;
    bool keepFirst = false;
    bool logging = false;
};

SRWLOCK g_settingsLock = SRWLOCK_INIT;
Settings g_settings;

static Settings SnapshotSettings() {
    AcquireSRWLockShared(&g_settingsLock);
    Settings copy = g_settings;
    ReleaseSRWLockShared(&g_settingsLock);
    return copy;
}

// Reduces an item's text to what a person reads: accelerator markers dropped (a doubled
// && is a literal ampersand and stays as one), the shortcut column after a tab cut off,
// zero-width characters removed, surrounding space trimmed, and lower-cased.
static std::wstring Normalize(PCWSTR text) {
    std::wstring out;
    for (PCWSTR p = text; *p; p++) {
        WCHAR c = *p;
        if (c == L'\t') {
            break;
        }
        if (c == L'&') {
            if (p[1] == L'&') {
                out += L'&';
                p++;
            }
            continue;
        }
        if (c == 0x200B || c == 0x200C || c == 0x200D || c == 0x2060 || c == 0xFEFF) {
            continue;
        }
        out += c;
    }

    size_t begin = out.find_first_not_of(L"  ");
    if (begin == std::wstring::npos) {
        return std::wstring();
    }
    size_t end = out.find_last_not_of(L"  ");
    out = out.substr(begin, end - begin + 1);

    CharLowerBuffW(out.data(), (DWORD)out.size());
    return out;
}

static bool IsListed(const Settings& settings, const std::wstring& key) {
    if (settings.anyName) {
        return true;
    }
    return std::find(settings.names.begin(), settings.names.end(), key) !=
           settings.names.end();
}

// Reads one item's text and whether it opens a submenu. Returns false for anything that
// is not a plain text item: separators, owner-drawn items, bitmaps, and empty labels.
static bool ReadItem(HMENU menu, int position, std::wstring* text, bool* hasSubmenu) {
    MENUITEMINFOW info = {sizeof(info)};
    info.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_STRING;
    if (!GetMenuItemInfoW(menu, position, TRUE, &info)) {
        return false;
    }
    if (info.fType & (MFT_SEPARATOR | MFT_OWNERDRAW | MFT_BITMAP)) {
        return false;
    }
    if (info.cch == 0) {
        return false;
    }

    std::wstring buffer(info.cch + 1, L'\0');
    info.dwTypeData = buffer.data();
    info.cch = (UINT)buffer.size();
    if (!GetMenuItemInfoW(menu, position, TRUE, &info)) {
        return false;
    }
    buffer.resize(info.cch);

    *text = std::move(buffer);
    *hasSubmenu = info.hSubMenu != nullptr;
    return true;
}

struct Entry {
    int position;
    std::wstring key;
    bool hasSubmenu;
};

static void RemoveDuplicates(HMENU menu, const Settings& settings) {
    int count = GetMenuItemCount(menu);
    if (count < 2) {
        return;
    }

    if (settings.logging) {
        Wh_Log(L"menu %p opening with %d items", menu, count);
    }

    std::vector<Entry> entries;
    std::wstring raw;
    for (int i = 0; i < count; i++) {
        bool hasSubmenu = false;
        if (!ReadItem(menu, i, &raw, &hasSubmenu)) {
            continue;
        }
        if (settings.logging) {
            Wh_Log(L"  [%d] %s%s", i, raw.c_str(), hasSubmenu ? L"  >" : L"");
        }

        std::wstring key = Normalize(raw.c_str());
        if (key.empty() || !IsListed(settings, key)) {
            continue;
        }
        entries.push_back({i, std::move(key), hasSubmenu});
    }

    // Group copies of the same item, then mark all but the chosen survivor.
    std::vector<int> doomed;
    std::vector<bool> grouped(entries.size(), false);
    for (size_t a = 0; a < entries.size(); a++) {
        if (grouped[a]) {
            continue;
        }
        std::vector<size_t> group{a};
        for (size_t b = a + 1; b < entries.size(); b++) {
            if (!grouped[b] && entries[b].key == entries[a].key &&
                entries[b].hasSubmenu == entries[a].hasSubmenu) {
                group.push_back(b);
                grouped[b] = true;
            }
        }
        if (group.size() < 2) {
            continue;
        }

        size_t survivor = settings.keepFirst ? 0 : group.size() - 1;
        for (size_t g = 0; g < group.size(); g++) {
            if (g != survivor) {
                doomed.push_back(entries[group[g]].position);
            }
        }
    }

    // Highest position first, so each deletion leaves the positions still to go intact.
    // DeleteMenu rather than RemoveMenu: it also destroys the copy's submenu, which is no
    // longer reachable from anywhere and would otherwise leak one menu per right-click.
    std::sort(doomed.begin(), doomed.end(), std::greater<int>());
    for (int position : doomed) {
        if (DeleteMenu(menu, position, MF_BYPOSITION)) {
            if (settings.logging) {
                Wh_Log(L"removed duplicate at position %d", position);
            }
        } else {
            Wh_Log(L"failed to remove duplicate at position %d, error %u", position,
                   GetLastError());
        }
    }
}

static void BeforeMenuOpens(HMENU menu) {
    if (!menu || !IsMenu(menu)) {
        return;
    }
    RemoveDuplicates(menu, SnapshotSettings());
}

// The common path. Both user32 calls below land here.
using NtUserTrackPopupMenuEx_t = BOOL(WINAPI*)(HMENU, UINT, int, int, HWND, LPTPMPARAMS);
NtUserTrackPopupMenuEx_t NtUserTrackPopupMenuEx_Original;

BOOL WINAPI NtUserTrackPopupMenuEx_Hook(HMENU menu, UINT flags, int x, int y, HWND owner,
                                        LPTPMPARAMS params) {
    BeforeMenuOpens(menu);
    return NtUserTrackPopupMenuEx_Original(menu, flags, x, y, owner, params);
}

// Fallbacks, used only if win32u does not export the call above.
using TrackPopupMenuEx_t = BOOL(WINAPI*)(HMENU, UINT, int, int, HWND, LPTPMPARAMS);
TrackPopupMenuEx_t TrackPopupMenuEx_Original;

BOOL WINAPI TrackPopupMenuEx_Hook(HMENU menu, UINT flags, int x, int y, HWND owner,
                                  LPTPMPARAMS params) {
    BeforeMenuOpens(menu);
    return TrackPopupMenuEx_Original(menu, flags, x, y, owner, params);
}

using TrackPopupMenu_t = BOOL(WINAPI*)(HMENU, UINT, int, int, int, HWND, const RECT*);
TrackPopupMenu_t TrackPopupMenu_Original;

BOOL WINAPI TrackPopupMenu_Hook(HMENU menu, UINT flags, int x, int y, int reserved,
                                HWND owner, const RECT* rect) {
    BeforeMenuOpens(menu);
    return TrackPopupMenu_Original(menu, flags, x, y, reserved, owner, rect);
}

// --- settings and lifecycle ----------------------------------------------

static void LoadSettings() {
    Settings settings;

    PCWSTR names = Wh_GetStringSetting(L"names");
    PCWSTR entry = names;
    for (PCWSTR p = names;; p++) {
        if (*p == L',' || !*p) {
            std::wstring item(entry, p - entry);
            size_t begin = item.find_first_not_of(L" \t");
            size_t end = item.find_last_not_of(L" \t");
            if (begin != std::wstring::npos) {
                item = item.substr(begin, end - begin + 1);
                if (item == L"*") {
                    settings.anyName = true;
                } else {
                    std::wstring key = Normalize(item.c_str());
                    if (!key.empty()) {
                        settings.names.push_back(std::move(key));
                    }
                }
            }
            if (!*p) {
                break;
            }
            entry = p + 1;
        }
    }
    Wh_FreeStringSetting(names);

    PCWSTR keep = Wh_GetStringSetting(L"keep");
    settings.keepFirst = keep && _wcsicmp(keep, L"first") == 0;
    Wh_FreeStringSetting(keep);

    settings.logging = Wh_GetIntSetting(L"logging") != 0;

    AcquireSRWLockExclusive(&g_settingsLock);
    g_settings = std::move(settings);
    ReleaseSRWLockExclusive(&g_settingsLock);
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

    HMODULE win32u = GetModuleHandleW(L"win32u.dll");
    if (win32u && HookExport(win32u, "NtUserTrackPopupMenuEx",
                             (void*)NtUserTrackPopupMenuEx_Hook,
                             (void**)&NtUserTrackPopupMenuEx_Original)) {
        return TRUE;
    }

    Wh_Log(L"win32u hook unavailable, falling back to user32");

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) {
        Wh_Log(L"failed to resolve user32");
        return FALSE;
    }

    HookExport(user32, "TrackPopupMenuEx", (void*)TrackPopupMenuEx_Hook,
               (void**)&TrackPopupMenuEx_Original);
    HookExport(user32, "TrackPopupMenu", (void*)TrackPopupMenu_Hook,
               (void**)&TrackPopupMenu_Original);

    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
