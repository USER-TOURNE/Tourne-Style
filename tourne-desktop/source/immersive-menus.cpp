// ==WindhawkMod==
// @id              local@tourne-classic-menus
// @name            Tourne Classic Menus
// @description     Windows' own menus drawn as classic menus everywhere, instead of "immersive" ones
// @version         1.0.0
// @author          Tourne
// @include         *
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- explorer: true
  $name: Explorer and file dialogs
  $description: >-
    Right-click menus on files, folders and the desktop, in Explorer and in
    every program's Open and Save dialogs.
- taskbar: true
  $name: Taskbar
  $description: >-
    Windows' own taskbar and its buttons, the Start button, and the desktop's
    menus Explorer itself shows.
- tray: true
  $name: Tray icons
  $description: Volume, keyboard language, Windows Security and network icons' menus.
- apps: true
  $name: Windows' programs
  $description: >-
    Task Manager, Narrator, and the title bar menu of Store apps' windows.
- noSettingsIcon: true
  $name: No icon on "Taskbar settings"
  $description: The taskbar's right-click menu without the gear Windows puts on that item.
- excludeApps: ''
  $name: Leave these programs alone
  $description: Program file names, separated by commas, like game.exe.
*/
// ==/WindhawkModSettings==

// Windows draws some of its own menus as "immersive" menus: ordinary menus that
// a helper compiled into each part of the shell takes over and owner-draws, with
// Windows 11's padding and rounding. That helper asks first whether it may
// (ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu); here the answer is no,
// so the menu stays a classic one, drawn by the visual style like any other.
//
// The helper is a copy in each module that has it, so each is hooked as it
// loads (never loaded here just to be hooked). Which modules have it was read
// from Windows itself, by the window property the helper keeps its data in
// (ImmersiveContextMenuArray_), on 24H2 / 26100 and 26300: Windows 11 moved
// the taskbar's menus into Taskbar.dll and the language menu into
// InputSwitch.dll, and renamed Windows Security's tray module, which the
// familiar "Eradicate Immersive Menus" mod's list predates. The older names
// stay listed for older builds. 32-bit programs get the same, with the
// helper's own calling convention.

#include <windows.h>

#include <atomic>
#include <string>
#include <vector>

struct MenuModule {
    PCWSTR name;  // a DLL, or a program (the helper in its own image)
    PCWSTR setting;
};

constexpr MenuModule kMenuModules[] = {
    {L"shell32.dll", L"explorer"},
    {L"ExplorerFrame.dll", L"explorer"},
    {L"Windows.UI.FileExplorer.dll", L"explorer"},
    {L"Taskbar.dll", L"taskbar"},
    {L"twinui.pcshell.dll", L"taskbar"},
    {L"twinui.dll", L"taskbar"},       // (older builds)
    {L"explorer.exe", L"taskbar"},     // (older builds)
    {L"SndVolSSO.dll", L"tray"},
    {L"InputSwitch.dll", L"tray"},
    {L"SecurityHealthSsoUdk.dll", L"tray"},
    {L"SecurityHealthSSO.dll", L"tray"},  // (older builds)
    {L"pnidui.dll", L"tray"},             // (older builds)
    {L"Taskmgr.exe", L"apps"},
    {L"Narrator.exe", L"apps"},
    {L"ApplicationFrame.dll", L"apps"},
    {L"museuxdocked.dll", L"apps"},
};

// Whether a category is on (read once; a change reloads the part, see NeedsReload).
bool g_categories[4];
bool g_noSettingsIcon;
SRWLOCK g_hookedLock = SRWLOCK_INIT;
std::vector<HMODULE> g_hooked;  // modules whose helper is hooked (guarded by g_hookedLock)
std::atomic<bool> g_initDone;

int CategoryIndex(PCWSTR setting) {
    static constexpr PCWSTR kNames[] = {L"explorer", L"taskbar", L"tray", L"apps"};
    for (int i = 0; i < 4; i++)
        if (!wcscmp(kNames[i], setting)) return i;
    return 0;
}

void ReadCategories(bool out[4], bool* noSettingsIcon) {
    out[0] = Wh_GetIntSetting(L"explorer") != 0;
    out[1] = Wh_GetIntSetting(L"taskbar") != 0;
    out[2] = Wh_GetIntSetting(L"tray") != 0;
    out[3] = Wh_GetIntSetting(L"apps") != 0;
    *noSettingsIcon = Wh_GetIntSetting(L"noSettingsIcon") != 0;
}

// The helper's answer, whoever's copy it is: no.
using CanApplyOwnerDrawToMenu_t = bool(WINAPI*)(HMENU, HWND);
bool WINAPI CanApplyOwnerDrawToMenu_Hook(HMENU, HWND) {
    return false;
}

// The module's file name, for matching the list.
std::wstring ModuleName(HMODULE module) {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(module, path, ARRAYSIZE(path));
    if (!length || length == ARRAYSIZE(path)) return L"";
    PCWSTR name = wcsrchr(path, L'\\');
    return name ? name + 1 : path;
}

// Its helper hooked, once, when the module is one of the list and its category is on. Each module's
// copy gets its own original pointer (only kept so the hook has somewhere to put it: it never calls on).
bool HookModule(HMODULE module) {
    if (!module) return false;
    std::wstring name = ModuleName(module);
    MenuModule const* entry = nullptr;
    for (auto const& m : kMenuModules)
        if (!_wcsicmp(m.name, name.c_str())) entry = &m;
    if (!entry || !g_categories[CategoryIndex(entry->setting)]) return false;
    AcquireSRWLockExclusive(&g_hookedLock);
    bool already = std::find(g_hooked.begin(), g_hooked.end(), module) != g_hooked.end();
    if (!already) g_hooked.push_back(module);
    ReleaseSRWLockExclusive(&g_hookedLock);
    if (already) return false;

    static CanApplyOwnerDrawToMenu_t originals[ARRAYSIZE(kMenuModules)];
    CanApplyOwnerDrawToMenu_t* original = &originals[entry - kMenuModules];
    WindhawkUtils::SYMBOL_HOOK hook[] = {
        {{
#ifdef _WIN64
             LR"(bool __cdecl ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *))"
#else
             LR"(bool __stdcall ImmersiveContextMenuHelper::CanApplyOwnerDrawToMenu(struct HMENU__ *,struct HWND__ *))"
#endif
         },
         original,
         CanApplyOwnerDrawToMenu_Hook,
         true},
    };
    if (!WindhawkUtils::HookSymbols(module, hook, ARRAYSIZE(hook)) || !*original) {
        Wh_Log(L"%ls: no immersive menu helper found", name.c_str());
        return false;
    }
    return true;
}

// Modules loaded later (a tray icon's, the file dialog's) are hooked as they arrive.
using LoadLibraryExW_t = HMODULE(WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR fileName, HANDLE file, DWORD flags) {
    HMODULE module = LoadLibraryExW_Original(fileName, file, flags);
    // (Not data or image-resource loads: those aren't code that runs.)
    if (module && g_initDone && !((ULONG_PTR)module & 3) &&
        !(flags & (LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE | LOAD_LIBRARY_AS_IMAGE_RESOURCE))) {
        if (HookModule(module)) Wh_ApplyHookOperations();
    }
    return module;
}

// "Taskbar settings" (item 413 of the taskbar's menu) keeps its gear off: Windows sets it after building
// the menu.
using SetMenuItemInfoW_t = BOOL(WINAPI*)(HMENU, UINT, BOOL, LPCMENUITEMINFOW);
SetMenuItemInfoW_t SetMenuItemInfoW_Original;
BOOL WINAPI SetMenuItemInfoW_Hook(HMENU menu, UINT item, BOOL byPosition, LPCMENUITEMINFOW info) {
    if (g_noSettingsIcon && !byPosition && item == 413 && info && (info->fMask & MIIM_BITMAP)) {
        if (info->fMask == MIIM_BITMAP) return TRUE;  // only the gear: left out
        MENUITEMINFOW rest = *info;
        rest.fMask &= ~MIIM_BITMAP;
        return SetMenuItemInfoW_Original(menu, item, byPosition, &rest);
    }
    return SetMenuItemInfoW_Original(menu, item, byPosition, info);
}

// Programs Windows' own menus aren't in, or where hooking isn't wanted (as Eradicate Immersive Menus
// leaves them), and the ones set here.
bool ShouldLoadInThisProcess() {
    if (!GetModuleHandleW(L"user32.dll")) return false;
    std::wstring name = ModuleName(nullptr);
    for (PCWSTR skip : {L"windhawk.exe", L"dwm.exe", L"ShellExperienceHost.exe", L"wlanext.exe", L"svchost.exe",
                        L"SearchIndexer.exe", L"consent.exe", L"LogonUI.exe", L"winlogon.exe", L"csrss.exe"}) {
        if (!_wcsicmp(name.c_str(), skip)) return false;
    }
    PCWSTR list = Wh_GetStringSetting(L"excludeApps");
    std::wstring items = list ? list : L"";
    Wh_FreeStringSetting(list);
    size_t start = 0;
    while (start <= items.size()) {
        size_t comma = items.find(L',', start);
        std::wstring item = items.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty() && !_wcsicmp(item.c_str(), name.c_str())) return false;
        if (comma == std::wstring::npos) break;
        start = comma + 1;
    }
    return true;
}

// A category switched on or off needs its hooks put in or taken out.
bool NeedsReload() {
    bool now[4], icon;
    ReadCategories(now, &icon);
    for (int i = 0; i < 4; i++)
        if (now[i] != g_categories[i]) return true;
    return false;
}

BOOL Wh_ModInit() {
    ReadCategories(g_categories, &g_noSettingsIcon);
    // The ones already loaded (shell32 in nearly every program), and the program itself.
    HookModule(GetModuleHandleW(nullptr));
    for (auto const& m : kMenuModules) {
        if (HMODULE module = GetModuleHandleW(m.name)) HookModule(module);
    }
    HMODULE kernelBase = GetModuleHandleW(L"kernelbase.dll");
    if (auto loadLibraryExW = kernelBase ? (LoadLibraryExW_t)GetProcAddress(kernelBase, "LoadLibraryExW") : nullptr)
        WindhawkUtils::SetFunctionHook(loadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    // The taskbar's menu: Explorer only.
    if (!_wcsicmp(ModuleName(nullptr).c_str(), L"explorer.exe"))
        WindhawkUtils::SetFunctionHook(SetMenuItemInfoW, SetMenuItemInfoW_Hook, &SetMenuItemInfoW_Original);
    g_initDone = true;
    return TRUE;
}

void Wh_ModUninit() {}

void Wh_ModSettingsChanged() {
    bool categories[4];
    ReadCategories(categories, &g_noSettingsIcon);
}
