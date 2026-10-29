// ==WindhawkMod==
// @id              tourne-classic-tray-folders
// @name            Tourne'Tray [Classic Tray Folders]
// @description     Group icons in the classic taskbar's tray overflow (StartAllBack and similar) into folders
// @version         1.0.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lcomctl32 -lgdi32 -lshell32 -lversion -ladvapi32
// @license         GPL-3.0
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# `Tourne'Tray` **[Classic Tray Folders]**

Adds folders to the **classic** tray overflow, the "^" popup of the Win32 taskbar
that StartAllBack (and the Windows 10 taskbar) uses. Each folder is a tile in the
popup with a preview of the icons inside. Clicking it shows only that folder's
icons, and the tile turns into a back arrow.

This is the classic-taskbar counterpart of **System Tray Folders** by Ris Peng,
which only works with the Windows 11 taskbar. If `SystemTray.dll` is not loaded
in Explorer, that mod has nothing to attach to and this one is what you want.

## How it works

* **Folders are real tray icons.** The mod adds one tray icon per folder, so
  Explorer treats it like any other icon: it has a tooltip, it can be dragged,
  and nothing foreign is inserted into Explorer's own toolbars.
* **Icons in a folder are hidden in the popup only.** Their place on the taskbar,
  and Explorer's saved "show on taskbar" choices, are never touched. An icon you
  have dragged onto the taskbar stays there even if it matches a folder.
* **Icons inside a folder are the real icons**, so clicking, right-clicking and
  tooltips behave exactly as they do outside a folder.
* **The popup is trimmed to fit** whatever is currently shown.

Closing the popup always returns it to the top level. Turning the mod off puts
every icon back where Explorer had it.

## Choosing icons

Each pattern is compared, ignoring case, with these names of every icon:

* **App name**: the file description of the program, e.g. `Discord` or
  `NVIDIA Container`.
* **Executable**: the file name, with and without `.exe`, e.g. `sizer.exe` or
  `sizer`.
* **Tooltip**: the text you see when hovering the icon.

A pattern without wildcards must match a whole name. Use `*` for any text and
`?` for a single character, e.g. `*NVIDIA*`. An icon goes into the first folder
with a matching pattern.

To see every icon's names, turn on logging in the mod's **Advanced** tab, open
the popup once, then click **Show log output**. Each icon is listed as
`Icon: exe="..." app="..." tooltip="..." folder=N`, where `N` counts from 0 and
-1 means no folder.

## Tooltips

Tooltips are learned as programs send them to the tray. Icons that were already
in the tray when the mod loaded, and have not changed since, fall back to the
first tooltip Windows remembers for them, which can be empty or out of date.
Restart Explorer once after installing and every tooltip is picked up from the
start.

## Limits

* Folder tiles sit wherever Explorer places new icons, usually at the end.
  Explorer keeps the popup and the taskbar in the same order and tracks icons by
  position, so the mod does not reorder anything.
* Built for the classic tray. With the Windows 11 taskbar the mod finds no
  classic popup and does nothing.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- folders:
  - - name: Folder
      $name: Folder name
    - glyph: ""
      $name: Folder glyph
      $description: >-
        Optional. A Segoe Fluent Icons code point such as U+E8B7, or any short
        text. Leave empty to show a preview of the first four icons inside.
    - icons: [""]
      $name: Icons
      $description: >-
        Patterns matched against each icon's app name, executable name and
        tooltip, ignoring case. Use * for any text and ? for one character,
        e.g. Discord or *NVIDIA*.
  $name: Folders
- showEmptyFolders: false
  $name: Show empty folders
  $description: >-
    Keep a folder's tile in the popup even when none of its icons are running.
- glyphColor: ""
  $name: Glyph color
  $description: >-
    Color of folder glyphs, the back arrow and the preview plate, as AARRGGBB
    or RRGGBB. Leave empty to follow the Windows light or dark mode.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cwchar>
#include <cwctype>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Constants

// Folder icons are tray icons owned by the mod's own window. Their ids must fit
// in 16 bits, because NOTIFYICON_VERSION_4 callbacks carry the id in
// HIWORD(lParam).
constexpr UINT kFolderIconIdBase = 0xC100;
constexpr int kMaxFolders = 32;
constexpr int kMaxPatterns = 128;

// A blank row is skipped, but this many in a row end the list, so a stray empty
// entry does not cost the rows after it.
constexpr int kBlankRowsEndList = 3;

// Shell_NotifyIcon reaches the tray as WM_COPYDATA to Shell_TrayWnd with this
// id, carrying a signature, the NIM_* message and a NOTIFYICONDATA with 32-bit
// handles. The tooltip sits at offset 24 of that struct.
constexpr ULONG_PTR kTrayCopyDataId = 1;
constexpr DWORD kTraySignature = 0x34753423;
constexpr size_t kTrayHeaderSize = 8;
constexpr size_t kTrayTipOffset = kTrayHeaderSize + 24;
constexpr size_t kTrayTipChars = 128;

// If the popup closes this soon after a folder click, the click closed it, and
// the popup is reopened with the folder still open.
constexpr DWORD kReopenWindowMs = 500;

// Caps the number of resizes per show or folder change, so the mod and Explorer
// can never take turns resizing the popup.
constexpr int kFitsPerEvent = 4;

constexpr DWORD kRegistryTipsMaxAgeMs = 30000;

constexpr WCHAR kBackGlyph[] = L"";
constexpr WCHAR kFolderGlyph[] = L"";

constexpr UINT WM_APP_TRAY_CALLBACK = WM_APP + 1;
constexpr UINT WM_APP_SNAPSHOT = WM_APP + 2;
constexpr UINT WM_APP_EXIT = WM_APP + 3;

UINT g_msgAttach;
UINT g_msgReevaluate;
UINT g_msgToggle;
UINT g_msgRestore;
UINT g_msgFit;
UINT g_msgTaskbarCreated;

std::atomic<bool> g_unloading;

////////////////////////////////////////////////////////////////////////////////
// Strings

std::wstring Trim(std::wstring_view s) {
    size_t begin = 0;
    size_t end = s.size();
    while (begin < end && iswspace(s[begin])) {
        begin++;
    }
    while (end > begin && iswspace(s[end - 1])) {
        end--;
    }
    return std::wstring(s.substr(begin, end - begin));
}

std::wstring ToLower(std::wstring s) {
    if (!s.empty()) {
        CharLowerBuffW(s.data(), (DWORD)s.size());
    }
    return s;
}

// Tooltips can hold line breaks and runs of spaces. Collapsing them lets a
// pattern be written on one line.
std::wstring CollapseSpaces(std::wstring_view s) {
    std::wstring result;
    result.reserve(s.size());
    bool lastWasSpace = false;
    for (WCHAR c : s) {
        if (iswspace(c)) {
            if (!lastWasSpace) {
                result.push_back(L' ');
            }
            lastWasSpace = true;
        } else {
            result.push_back(c);
            lastWasSpace = false;
        }
    }
    return Trim(result);
}

std::wstring FileNameOf(std::wstring_view path) {
    size_t slash = path.find_last_of(L"\\/");
    return std::wstring(slash == std::wstring_view::npos
                            ? path
                            : path.substr(slash + 1));
}

bool WildcardMatch(std::wstring_view pattern, std::wstring_view text) {
    size_t p = 0;
    size_t t = 0;
    size_t starP = std::wstring_view::npos;
    size_t starT = 0;
    while (t < text.size()) {
        if (p < pattern.size() && pattern[p] == L'*') {
            starP = p++;
            starT = t;
        } else if (p < pattern.size() &&
                   (pattern[p] == L'?' || pattern[p] == text[t])) {
            p++;
            t++;
        } else if (starP != std::wstring_view::npos) {
            p = starP + 1;
            t = ++starT;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == L'*') {
        p++;
    }
    return p == pattern.size();
}

////////////////////////////////////////////////////////////////////////////////
// Settings

struct FolderConfig {
    std::wstring name;
    std::wstring glyph;
    std::vector<std::wstring> patterns;  // Lowercase.
};

struct Settings {
    std::vector<FolderConfig> folders;
    bool showEmptyFolders = false;
    bool glyphColorAuto = true;
    uint32_t glyphColor = 0xFFFFFFFF;  // AARRGGBB.
};

SRWLOCK g_settingsLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::shared_ptr<const Settings> g_settings;

std::shared_ptr<const Settings> GetSettings() {
    AcquireSRWLockShared(&g_settingsLock);
    auto settings = g_settings;
    ReleaseSRWLockShared(&g_settingsLock);
    return settings;
}

// "U+E8B7" or "0xE8B7" becomes that character, anything else is used as text.
std::wstring ParseGlyph(std::wstring_view value) {
    std::wstring glyph = Trim(value);
    std::wstring_view hex = glyph;
    if (!hex.starts_with(L"U+") && !hex.starts_with(L"u+") &&
        !hex.starts_with(L"0x") && !hex.starts_with(L"0X")) {
        return glyph;
    }
    hex.remove_prefix(2);
    if (hex.size() < 4 || hex.size() > 5 ||
        !std::all_of(hex.begin(), hex.end(),
                     [](WCHAR c) { return !!iswxdigit(c); })) {
        return glyph;
    }
    UINT32 codePoint = wcstoul(std::wstring(hex).c_str(), nullptr, 16);
    if (codePoint <= 0xFFFF) {
        return std::wstring(1, (WCHAR)codePoint);
    }
    codePoint -= 0x10000;
    return {(WCHAR)(0xD800 + (codePoint >> 10)),
            (WCHAR)(0xDC00 + (codePoint & 0x3FF))};
}

// AARRGGBB, alpha first as in the other Tourne mods, or RRGGBB. A leading hash
// is accepted.
bool ParseColor(std::wstring_view value, uint32_t* color) {
    std::wstring text = Trim(value);
    std::wstring_view hex = text;
    if (hex.starts_with(L"#")) {
        hex.remove_prefix(1);
    }
    if ((hex.size() != 6 && hex.size() != 8) ||
        !std::all_of(hex.begin(), hex.end(),
                     [](WCHAR c) { return !!iswxdigit(c); })) {
        return false;
    }
    uint32_t parsed = wcstoul(std::wstring(hex).c_str(), nullptr, 16);
    *color = hex.size() == 6 ? (0xFF000000 | parsed) : parsed;
    return true;
}

Settings LoadSettings() {
    Settings settings;

    for (int i = 0, blankFolders = 0;
         i < kMaxFolders && blankFolders < kBlankRowsEndList; i++) {
        FolderConfig folder;
        folder.name = Trim(
            WindhawkUtils::StringSetting::make(L"folders[%d].name", i).get());
        folder.glyph = ParseGlyph(
            WindhawkUtils::StringSetting::make(L"folders[%d].glyph", i).get());

        for (int j = 0, blankIcons = 0;
             j < kMaxPatterns && blankIcons < kBlankRowsEndList; j++) {
            std::wstring pattern =
                CollapseSpaces(WindhawkUtils::StringSetting::make(
                                   L"folders[%d].icons[%d]", i, j)
                                   .get());
            if (pattern.empty()) {
                blankIcons++;
                continue;
            }
            blankIcons = 0;
            folder.patterns.push_back(ToLower(std::move(pattern)));
        }

        if (folder.name.empty() && folder.glyph.empty() &&
            folder.patterns.empty()) {
            blankFolders++;
            continue;
        }
        blankFolders = 0;

        if (folder.name.empty()) {
            folder.name = L"Folder";
        }
        settings.folders.push_back(std::move(folder));
    }

    settings.showEmptyFolders = Wh_GetIntSetting(L"showEmptyFolders");

    WindhawkUtils::StringSetting color =
        WindhawkUtils::StringSetting::make(L"glyphColor");
    uint32_t parsed;
    if (ParseColor(color.get(), &parsed)) {
        settings.glyphColorAuto = false;
        settings.glyphColor = parsed;
    } else if (*color.get()) {
        Wh_Log(L"Glyph color \"%s\" isn't AARRGGBB or RRGGBB, using auto",
               color.get());
    }

    return settings;
}

////////////////////////////////////////////////////////////////////////////////
// Icon names

// What each classic tray toolbar button's dwData points to. Only the leading
// fields are read; the owner window and id identify the icon.
struct TrayItemData {
    HWND hwnd;
    UINT uID;
    UINT uCallbackMessage;
};

uint64_t IconKey(HWND hwnd, UINT uID) {
    // The tray only ever sees 32-bit window handles.
    return ((uint64_t)(uint32_t)(ULONG_PTR)hwnd << 32) | uID;
}

// Tooltips as programs send them, filled in from Shell_TrayWnd's WM_COPYDATA.
SRWLOCK g_tipsLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::unordered_map<uint64_t, std::wstring> g_tips;

void RecordTrayCopyData(const COPYDATASTRUCT* copyData) {
    if (!copyData || copyData->dwData != kTrayCopyDataId || !copyData->lpData ||
        copyData->cbData < kTrayTipOffset) {
        return;
    }

    const BYTE* data = (const BYTE*)copyData->lpData;
    DWORD signature;
    DWORD message;
    DWORD hwnd32;
    UINT uID;
    UINT flags;
    memcpy(&signature, data, sizeof(signature));
    memcpy(&message, data + 4, sizeof(message));
    memcpy(&hwnd32, data + kTrayHeaderSize + 4, sizeof(hwnd32));
    memcpy(&uID, data + kTrayHeaderSize + 8, sizeof(uID));
    memcpy(&flags, data + kTrayHeaderSize + 12, sizeof(flags));
    if (signature != kTraySignature) {
        return;
    }

    uint64_t key = IconKey((HWND)(ULONG_PTR)hwnd32, uID);

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

// The first tooltip Windows saw for each icon, by lowercase executable name and
// id. The fallback for icons that haven't sent a tooltip since the mod loaded.
[[clang::no_destroy]] std::unordered_map<std::wstring, std::wstring>
    g_registryTips;
DWORD g_registryTipsTick;
bool g_registryTipsLoaded;

std::wstring RegistryTipKey(std::wstring_view exeLower, UINT uID) {
    return std::wstring(exeLower) + L"|" + std::to_wstring(uID);
}

void RefreshRegistryTips() {
    if (g_registryTipsLoaded &&
        GetTickCount() - g_registryTipsTick < kRegistryTipsMaxAgeMs) {
        return;
    }
    g_registryTipsLoaded = true;
    g_registryTipsTick = GetTickCount();
    g_registryTips.clear();

    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\NotifyIconSettings",
                      0, KEY_READ, &key) != ERROR_SUCCESS) {
        return;
    }

    WCHAR subkey[256];
    for (DWORD i = 0;; i++) {
        DWORD subkeyLength = ARRAYSIZE(subkey);
        LONG result = RegEnumKeyExW(key, i, subkey, &subkeyLength, nullptr,
                                    nullptr, nullptr, nullptr);
        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (result != ERROR_SUCCESS) {
            continue;
        }

        WCHAR path[MAX_PATH * 2];
        WCHAR tip[512];
        DWORD uID = 0;
        DWORD size = sizeof(path);
        if (RegGetValueW(key, subkey, L"ExecutablePath", RRF_RT_REG_SZ,
                         nullptr, path, &size) != ERROR_SUCCESS) {
            continue;
        }
        size = sizeof(tip);
        if (RegGetValueW(key, subkey, L"InitialTooltip", RRF_RT_REG_SZ,
                         nullptr, tip, &size) != ERROR_SUCCESS ||
            !*tip) {
            continue;
        }
        size = sizeof(uID);
        RegGetValueW(key, subkey, L"UID", RRF_RT_REG_DWORD, nullptr, &uID,
                     &size);

        // The path can start with a known folder GUID; only the name matters.
        g_registryTips.try_emplace(
            RegistryTipKey(ToLower(FileNameOf(path)), uID),
            CollapseSpaces(tip));
    }

    RegCloseKey(key);
}

std::wstring LookupTip(HWND owner, UINT uID, std::wstring_view exeLower) {
    AcquireSRWLockShared(&g_tipsLock);
    auto it = g_tips.find(IconKey(owner, uID));
    std::wstring tip = it != g_tips.end() ? it->second : std::wstring();
    ReleaseSRWLockShared(&g_tipsLock);

    if (tip.empty() && !exeLower.empty()) {
        if (auto registryTip =
                g_registryTips.find(RegistryTipKey(exeLower, uID));
            registryTip != g_registryTips.end()) {
            tip = registryTip->second;
        }
    }
    return tip;
}

std::wstring GetFileDescription(PCWSTR path) {
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeW(path, &handle);
    if (!size) {
        return {};
    }
    std::vector<BYTE> data(size);
    if (!GetFileVersionInfoW(path, 0, size, data.data())) {
        return {};
    }

    struct Translation {
        WORD language;
        WORD codePage;
    };
    std::vector<Translation> translations;
    Translation* found;
    UINT length;
    if (VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation",
                       (void**)&found, &length)) {
        size_t count = length / sizeof(Translation);
        translations.assign(found, found + count);
    }
    translations.push_back({0x0409, 0x04B0});
    translations.push_back({0x0409, 0x04E4});

    for (auto const& translation : translations) {
        WCHAR query[64];
        swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\FileDescription",
                   translation.language, translation.codePage);
        WCHAR* description;
        if (VerQueryValueW(data.data(), query, (void**)&description,
                           &length) &&
            length > 1) {
            return CollapseSpaces(description);
        }
    }
    return {};
}

struct ProcessNames {
    DWORD pid = 0;
    std::wstring exe;
    std::wstring description;
    std::wstring exeLower;
    std::wstring exeBaseLower;
    std::wstring descriptionLower;
};

// Tray thread only. Descriptions are kept per executable for the life of the
// mod, since reading version info is the slow part.
[[clang::no_destroy]] std::unordered_map<HWND, ProcessNames> g_processNames;
[[clang::no_destroy]] std::unordered_map<std::wstring, std::wstring>
    g_descriptions;

ProcessNames ReadProcessNames(DWORD pid) {
    ProcessNames names;
    names.pid = pid;

    HANDLE process =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        return names;
    }
    WCHAR path[MAX_PATH * 2];
    DWORD pathLength = ARRAYSIZE(path);
    BOOL gotPath = QueryFullProcessImageNameW(process, 0, path, &pathLength);
    CloseHandle(process);
    if (!gotPath) {
        return names;
    }

    std::wstring pathLower = ToLower(path);
    auto description = g_descriptions.find(pathLower);
    if (description == g_descriptions.end()) {
        description =
            g_descriptions.emplace(pathLower, GetFileDescription(path)).first;
    }

    names.exe = FileNameOf(path);
    names.description = description->second;
    names.exeLower = ToLower(names.exe);
    names.exeBaseLower = names.exeLower;
    if (names.exeBaseLower.ends_with(L".exe")) {
        names.exeBaseLower.resize(names.exeBaseLower.size() - 4);
    }
    names.descriptionLower = ToLower(names.description);
    return names;
}

// Builds the cache for this pass into `fresh`, reusing entries from the last
// pass whose window still belongs to the same process.
const ProcessNames& LookupProcessNames(
    HWND owner,
    std::unordered_map<HWND, ProcessNames>& fresh) {
    if (auto it = fresh.find(owner); it != fresh.end()) {
        return it->second;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    if (auto it = g_processNames.find(owner);
        it != g_processNames.end() && it->second.pid == pid && pid) {
        return fresh.emplace(owner, std::move(it->second)).first->second;
    }
    return fresh.emplace(owner, ReadProcessNames(pid)).first->second;
}

int MatchFolder(Settings const& settings,
                std::initializer_list<std::wstring_view> lowerNames) {
    for (size_t i = 0; i < settings.folders.size(); i++) {
        for (auto const& pattern : settings.folders[i].patterns) {
            for (auto name : lowerNames) {
                if (!name.empty() && WildcardMatch(pattern, name)) {
                    return (int)i;
                }
            }
        }
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Snapshots handed from the tray thread to the mod thread

struct FolderView {
    std::wstring name;
    std::wstring glyph;
    int count = 0;
    bool open = false;
    bool visible = false;
    std::vector<HICON> previews;
};

struct Snapshot {
    std::vector<FolderView> folders;
    int iconSize = 16;
    uint32_t glyphColor = 0xFFFFFFFF;

    Snapshot() = default;
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;
    ~Snapshot() {
        for (auto& folder : folders) {
            for (HICON icon : folder.previews) {
                DestroyIcon(icon);
            }
        }
    }
};

SRWLOCK g_snapshotLock = SRWLOCK_INIT;
[[clang::no_destroy]] std::unique_ptr<Snapshot> g_pendingSnapshot;

std::atomic<HWND> g_modWnd;
std::atomic<HWND> g_trayWnd;
std::atomic<HWND> g_toolbarWnd;
std::atomic<HWND> g_overflowWnd;

////////////////////////////////////////////////////////////////////////////////
// Overflow toolbar state (tray thread only)

using ImageList_GetIcon_t = HICON(WINAPI*)(HIMAGELIST, int, UINT);
ImageList_GetIcon_t g_imageListGetIcon;

struct TrayState {
    HWND toolbar = nullptr;
    HWND overflow = nullptr;

    // What Explorer wants each button's hidden flag to be, by command id. The
    // real flag is this or'ed with the mod's own hiding.
    std::unordered_map<int, bool> intentHidden;
    std::unordered_map<int, int> memberFolder;
    std::unordered_map<int, int> folderButton;
    std::vector<int> folderCounts;

    int openFolder = -1;
    int passThrough = 0;
    int fitsLeft = kFitsPerEvent;
    bool restoring = false;
    bool reevaluatePosted = false;
    bool fitPosted = false;
    bool fitting = false;
    bool keepFolderOnShow = false;
    bool overflowHooked = false;
    DWORD lastToggleTick = 0;
};

[[clang::no_destroy]] TrayState g_tray;

// Messages sent through here skip the subclass, so the mod sees the real state
// and its own changes aren't mistaken for Explorer's.
LRESULT ToolbarSend(UINT message, WPARAM wParam, LPARAM lParam) {
    g_tray.passThrough++;
    LRESULT result = SendMessageW(g_tray.toolbar, message, wParam, lParam);
    g_tray.passThrough--;
    return result;
}

bool GetButton(int index, TBBUTTON* button) {
    *button = {};
    return ToolbarSend(TB_GETBUTTON, index, (LPARAM)button) != 0;
}

int IdFromIndex(int index) {
    TBBUTTON button;
    return GetButton(index, &button) ? button.idCommand : -1;
}

BYTE WithHidden(BYTE state, bool hidden) {
    return hidden ? (state | TBSTATE_HIDDEN) : (state & ~TBSTATE_HIDDEN);
}

// Returns whether Explorer's wish for this button changed.
bool SetIntent(int id, bool hidden) {
    auto [it, inserted] = g_tray.intentHidden.try_emplace(id, hidden);
    if (inserted) {
        return true;
    }
    if (it->second != hidden) {
        it->second = hidden;
        return true;
    }
    return false;
}

bool IntentHidden(int id, bool actual) {
    return g_tray.intentHidden.try_emplace(id, actual).first->second;
}

bool OverlayHidden(int id) {
    if (g_tray.restoring) {
        return false;
    }
    if (auto it = g_tray.folderButton.find(id);
        it != g_tray.folderButton.end()) {
        return g_tray.openFolder >= 0 && it->second != g_tray.openFolder;
    }
    auto it = g_tray.memberFolder.find(id);
    int folder = it != g_tray.memberFolder.end() ? it->second : -1;
    if (g_tray.openFolder >= 0) {
        return folder != g_tray.openFolder;
    }
    return folder >= 0;
}

void ScheduleReevaluate() {
    if (!g_tray.reevaluatePosted && g_tray.toolbar) {
        g_tray.reevaluatePosted = true;
        PostMessageW(g_tray.toolbar, g_msgReevaluate, 0, 0);
    }
}

void PostFit() {
    if (!g_tray.fitPosted && g_tray.overflow && g_tray.overflowHooked) {
        g_tray.fitPosted = true;
        PostMessageW(g_tray.overflow, g_msgFit, 0, 0);
    }
}

void Enforce() {
    int count = (int)ToolbarSend(TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < count; i++) {
        TBBUTTON button;
        if (!GetButton(i, &button)) {
            continue;
        }
        bool actual = button.fsState & TBSTATE_HIDDEN;
        bool desired = IntentHidden(button.idCommand, actual) ||
                       OverlayHidden(button.idCommand);
        if (desired != actual) {
            ToolbarSend(TB_HIDEBUTTON, button.idCommand,
                        MAKELPARAM(desired, 0));
        }
    }
}

int CountVisibleButtons() {
    int visible = 0;
    int count = (int)ToolbarSend(TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < count; i++) {
        TBBUTTON button;
        if (GetButton(i, &button) && !(button.fsState & TBSTATE_HIDDEN)) {
            visible++;
        }
    }
    return visible;
}

void Reevaluate(bool logIcons) {
    auto settings = GetSettings();
    size_t folderCount = settings->folders.size();
    HWND modWnd = g_modWnd;

    if (logIcons) {
        RefreshRegistryTips();
    }

    std::unordered_map<HWND, ProcessNames> processNames;
    g_tray.memberFolder.clear();
    g_tray.folderButton.clear();
    g_tray.folderCounts.assign(folderCount, 0);

    int count = (int)ToolbarSend(TB_BUTTONCOUNT, 0, 0);
    for (int i = 0; i < count; i++) {
        TBBUTTON button;
        if (!GetButton(i, &button) || !button.dwData) {
            continue;
        }
        auto* item = (const TrayItemData*)button.dwData;
        HWND owner = item->hwnd;
        UINT uID = item->uID;
        bool intent = IntentHidden(button.idCommand,
                                   button.fsState & TBSTATE_HIDDEN);

        if (modWnd && owner == modWnd) {
            if (uID >= kFolderIconIdBase &&
                uID < kFolderIconIdBase + kMaxFolders) {
                g_tray.folderButton[button.idCommand] =
                    (int)(uID - kFolderIconIdBase);
            }
            continue;
        }

        const ProcessNames& names = LookupProcessNames(owner, processNames);
        std::wstring tip = LookupTip(owner, uID, names.exeLower);
        std::wstring tipLower = ToLower(tip);
        int folder = MatchFolder(*settings,
                                 {names.descriptionLower, names.exeLower,
                                  names.exeBaseLower, tipLower});
        g_tray.memberFolder[button.idCommand] = folder;
        if (folder >= 0 && !intent) {
            g_tray.folderCounts[folder]++;
        }

        if (logIcons) {
            Wh_Log(L"Icon: exe=\"%s\" app=\"%s\" tooltip=\"%s\" folder=%d%s",
                   names.exe.c_str(), names.description.c_str(), tip.c_str(),
                   folder, intent ? L" (on the taskbar)" : L"");
        }
    }

    g_processNames = std::move(processNames);

    if (g_tray.openFolder >= (int)folderCount ||
        (g_tray.openFolder >= 0 && g_tray.folderCounts[g_tray.openFolder] == 0 &&
         !settings->showEmptyFolders)) {
        g_tray.openFolder = -1;
    }
}

bool SystemUsesLightTheme() {
    DWORD value = 0;
    DWORD size = sizeof(value);
    return RegGetValueW(
               HKEY_CURRENT_USER,
               L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\"
               L"Personalize",
               L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &value,
               &size) == ERROR_SUCCESS &&
           value;
}

void PostSnapshot() {
    HWND modWnd = g_modWnd;
    if (!modWnd) {
        return;
    }

    auto settings = GetSettings();
    auto snapshot = std::make_unique<Snapshot>();

    UINT dpi = GetDpiForWindow(g_tray.toolbar);
    snapshot->iconSize =
        GetSystemMetricsForDpi(SM_CXSMICON, dpi ? dpi : USER_DEFAULT_SCREEN_DPI);
    snapshot->glyphColor = settings->glyphColorAuto
                               ? (SystemUsesLightTheme() ? 0xFF000000
                                                         : 0xFFFFFFFF)
                               : settings->glyphColor;

    snapshot->folders.resize(settings->folders.size());
    for (size_t k = 0; k < settings->folders.size(); k++) {
        auto& view = snapshot->folders[k];
        view.name = settings->folders[k].name;
        view.glyph = settings->folders[k].glyph;
        view.count = k < g_tray.folderCounts.size() ? g_tray.folderCounts[k] : 0;
        view.open = g_tray.openFolder == (int)k;
        view.visible = view.count > 0 || settings->showEmptyFolders;
    }

    // Previews come from the toolbar's own image list, in the order the icons
    // appear in the popup.
    HIMAGELIST imageList =
        (HIMAGELIST)ToolbarSend(TB_GETIMAGELIST, 0, 0);
    if (imageList && g_imageListGetIcon) {
        int count = (int)ToolbarSend(TB_BUTTONCOUNT, 0, 0);
        for (int i = 0; i < count; i++) {
            TBBUTTON button;
            if (!GetButton(i, &button) || button.iBitmap < 0 ||
                HIWORD(button.iBitmap)) {
                continue;
            }
            auto member = g_tray.memberFolder.find(button.idCommand);
            if (member == g_tray.memberFolder.end() || member->second < 0 ||
                member->second >= (int)snapshot->folders.size() ||
                IntentHidden(button.idCommand,
                             button.fsState & TBSTATE_HIDDEN)) {
                continue;
            }
            auto& view = snapshot->folders[member->second];
            if (!view.glyph.empty() || view.previews.size() >= 4) {
                continue;
            }
            if (HICON icon = g_imageListGetIcon(imageList, button.iBitmap,
                                                ILD_TRANSPARENT)) {
                view.previews.push_back(icon);
            }
        }
    }

    AcquireSRWLockExclusive(&g_snapshotLock);
    g_pendingSnapshot = std::move(snapshot);
    ReleaseSRWLockExclusive(&g_snapshotLock);
    PostMessageW(modWnd, WM_APP_SNAPSHOT, 0, 0);
}

// Shrinks or grows the popup so its grid has exactly the rows it needs.
// Explorer sizes the popup for every icon it put there, and hidden buttons
// take no space in a wrapping toolbar, so without this the popup keeps empty
// rows.
void Fit() {
    HWND overflow = g_tray.overflow;
    HWND toolbar = g_tray.toolbar;
    if (!overflow || !toolbar || !IsWindowVisible(overflow) ||
        g_tray.fitsLeft <= 0) {
        return;
    }

    RECT windowRect;
    RECT toolbarRect;
    if (!GetWindowRect(overflow, &windowRect) ||
        !GetWindowRect(toolbar, &toolbarRect)) {
        return;
    }

    DWORD buttonSize = (DWORD)ToolbarSend(TB_GETBUTTONSIZE, 0, 0);
    int buttonWidth = LOWORD(buttonSize);
    int buttonHeight = HIWORD(buttonSize);
    int toolbarWidth = toolbarRect.right - toolbarRect.left;
    int toolbarHeight = toolbarRect.bottom - toolbarRect.top;
    if (buttonWidth <= 0 || buttonHeight <= 0 || toolbarWidth < buttonWidth) {
        return;
    }

    int columns = toolbarWidth / buttonWidth;
    int visible = CountVisibleButtons();
    int rows = std::max(1, (visible + columns - 1) / columns);
    int delta = rows * buttonHeight - toolbarHeight;
    int windowHeight = windowRect.bottom - windowRect.top + delta;
    if (delta == 0 || windowHeight <= 0) {
        return;
    }
    g_tray.fitsLeft--;

    // Keep the edge next to the taskbar where it is.
    bool keepBottom = true;
    RECT trayRect;
    if (HWND tray = g_trayWnd; tray && GetWindowRect(tray, &trayRect)) {
        keepBottom = (windowRect.top + windowRect.bottom) / 2 <
                     (trayRect.top + trayRect.bottom) / 2;
    }
    int top = keepBottom ? windowRect.bottom - windowHeight : windowRect.top;

    // The window first and the toolbar last, so any resizing Explorer does in
    // response to the window is overridden.
    g_tray.fitting = true;
    SetWindowPos(overflow, nullptr, windowRect.left, top,
                 windowRect.right - windowRect.left, windowHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(toolbar, nullptr, 0, 0, toolbarWidth, rows * buttonHeight,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    g_tray.fitting = false;

    Wh_Log(L"Fit: %d visible in %d columns, %d rows, height %+d", visible,
           columns, rows, delta);
}

// Opens the popup the way a click on the "^" button does. Used when a folder is
// clicked while the popup is closed, which happens when a folder tile has been
// dragged onto the taskbar, or when the click itself closed the popup.
void ClickChevron() {
    HWND tray = g_trayWnd;
    HWND notify =
        tray ? FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
    if (!notify) {
        return;
    }

    for (HWND child = FindWindowExW(notify, nullptr, nullptr, nullptr); child;
         child = FindWindowExW(notify, child, nullptr, nullptr)) {
        WCHAR className[64];
        if (!GetClassNameW(child, className, ARRAYSIZE(className))) {
            continue;
        }
        if (_wcsicmp(className, L"Button") == 0) {
            PostMessageW(child, BM_CLICK, 0, 0);
            return;
        }
        if (_wcsicmp(className, L"SIBTrayButton") == 0) {
            RECT rect;
            GetClientRect(child, &rect);
            LPARAM point = MAKELPARAM(rect.right / 2, rect.bottom / 2);
            PostMessageW(child, WM_LBUTTONDOWN, MK_LBUTTON, point);
            PostMessageW(child, WM_LBUTTONUP, 0, point);
            return;
        }
    }
    Wh_Log(L"Couldn't find the overflow button");
}

void ToggleFolder(int folder) {
    auto settings = GetSettings();
    if (folder < 0 || folder >= (int)settings->folders.size()) {
        return;
    }

    g_tray.openFolder = g_tray.openFolder == folder ? -1 : folder;
    g_tray.lastToggleTick = GetTickCount();
    g_tray.fitsLeft = kFitsPerEvent;
    Wh_Log(L"Folder %d %s", folder,
           g_tray.openFolder >= 0 ? L"opened" : L"closed");

    Enforce();
    PostSnapshot();

    if (g_tray.overflow && IsWindowVisible(g_tray.overflow)) {
        Fit();
    } else if (g_tray.openFolder >= 0) {
        g_tray.keepFolderOnShow = true;
        ClickChevron();
    }
}

void OnOverflowShowing() {
    if (!g_tray.keepFolderOnShow) {
        g_tray.openFolder = -1;
    }
    g_tray.keepFolderOnShow = false;
    g_tray.fitsLeft = kFitsPerEvent;

    Reevaluate(/*logIcons=*/true);
    Enforce();
    PostSnapshot();
}

void OnOverflowHidden() {
    if (g_tray.openFolder < 0 || g_tray.restoring) {
        return;
    }

    if (GetTickCount() - g_tray.lastToggleTick < kReopenWindowMs) {
        Wh_Log(L"The popup closed right after a folder click, reopening");
        g_tray.lastToggleTick = 0;
        g_tray.keepFolderOnShow = true;
        ClickChevron();
        return;
    }

    g_tray.openFolder = -1;
    Enforce();
    PostSnapshot();
}

void Restore() {
    g_tray.restoring = true;
    g_tray.openFolder = -1;
    g_tray.fitsLeft = kFitsPerEvent;
    Enforce();
    Fit();
}

////////////////////////////////////////////////////////////////////////////////
// Subclasses

void RecordNewButtons(const TBBUTTON* buttons, int count) {
    for (int i = 0; i < count; i++) {
        SetIntent(buttons[i].idCommand, buttons[i].fsState & TBSTATE_HIDDEN);
    }
    ScheduleReevaluate();
}

LRESULT CALLBACK OverflowSubclassProc(HWND hWnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam,
                                      DWORD_PTR dwRefData);

void OnAttach(HWND toolbar) {
    g_tray = TrayState{};
    g_tray.toolbar = toolbar;
    g_tray.overflow = GetParent(toolbar);

    if (HMODULE comctl = (HMODULE)GetClassLongPtrW(toolbar, GCLP_HMODULE)) {
        g_imageListGetIcon =
            (ImageList_GetIcon_t)GetProcAddress(comctl, "ImageList_GetIcon");
    }
    if (!g_imageListGetIcon) {
        Wh_Log(L"ImageList_GetIcon unavailable, folders show a glyph instead "
               L"of a preview");
    }

    if (g_tray.overflow && GetWindowThreadProcessId(g_tray.overflow, nullptr) ==
                               GetCurrentThreadId()) {
        g_tray.overflowHooked = WindhawkUtils::SetWindowSubclassFromAnyThread(
            g_tray.overflow, OverflowSubclassProc, 0);
        g_overflowWnd = g_tray.overflow;
    }
    if (!g_tray.overflowHooked) {
        Wh_Log(L"Couldn't hook the popup window, it won't be resized or reset "
               L"on close");
    }

    g_toolbarWnd = toolbar;
    Wh_Log(L"Attached to the overflow toolbar %p",
           (void*)toolbar);
    ScheduleReevaluate();
}

LRESULT CALLBACK ToolbarSubclassProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     DWORD_PTR dwRefData) {
    if (uMsg == WM_NCDESTROY) {
        if (g_tray.toolbar == hWnd) {
            Wh_Log(L"Overflow toolbar destroyed");
            g_toolbarWnd = nullptr;
            g_overflowWnd = nullptr;
            g_tray = TrayState{};
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == g_msgAttach) {
        OnAttach(hWnd);
        return 0;
    }

    if (g_tray.toolbar != hWnd) {
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == g_msgReevaluate) {
        g_tray.reevaluatePosted = false;
        if (wParam) {
            // Settings changed: the folder list may no longer match.
            g_tray.openFolder = -1;
        }
        g_tray.fitsLeft = kFitsPerEvent;
        Reevaluate(/*logIcons=*/false);
        Enforce();
        Fit();
        PostSnapshot();
        return 0;
    }
    if (uMsg == g_msgToggle) {
        ToggleFolder((int)wParam);
        return 0;
    }
    if (uMsg == g_msgRestore) {
        Restore();
        return 0;
    }

    if (g_tray.passThrough > 0) {
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg) {
        // Changes to the hidden flag: remember what Explorer asked for, then
        // apply it together with the mod's own hiding.
        case TB_HIDEBUTTON: {
            int id = (int)wParam;
            bool hide = LOWORD(lParam) != 0;
            bool changed = SetIntent(id, hide);
            LRESULT result = DefSubclassProc(
                hWnd, uMsg, wParam,
                MAKELPARAM(hide || OverlayHidden(id), 0));
            if (changed) {
                ScheduleReevaluate();
            }
            return result;
        }

        case TB_SETSTATE: {
            int id = (int)wParam;
            BYTE state = (BYTE)LOWORD(lParam);
            bool hide = state & TBSTATE_HIDDEN;
            bool changed = SetIntent(id, hide);
            LRESULT result = DefSubclassProc(
                hWnd, uMsg, wParam,
                MAKELPARAM(WithHidden(state, hide || OverlayHidden(id)), 0));
            if (changed) {
                ScheduleReevaluate();
            }
            return result;
        }

        case TB_SETBUTTONINFOW:
        case TB_SETBUTTONINFOA: {
            // The A and W structs only differ in the type of a pointer.
            auto* info = (TBBUTTONINFOW*)lParam;
            if (!info || info->cbSize < sizeof(TBBUTTONINFOW) ||
                !(info->dwMask & TBIF_STATE)) {
                break;
            }
            int id = (info->dwMask & TBIF_BYINDEX) ? IdFromIndex((int)wParam)
                                                   : (int)wParam;
            bool hide = info->fsState & TBSTATE_HIDDEN;
            bool changed = SetIntent(id, hide);
            TBBUTTONINFOW copy = *info;
            copy.fsState = WithHidden(copy.fsState, hide || OverlayHidden(id));
            LRESULT result =
                DefSubclassProc(hWnd, uMsg, wParam, (LPARAM)&copy);
            if (changed) {
                ScheduleReevaluate();
            }
            return result;
        }

        case TB_INSERTBUTTONW:
        case TB_INSERTBUTTONA: {
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            if (result && lParam) {
                RecordNewButtons((const TBBUTTON*)lParam, 1);
            }
            return result;
        }

        case TB_ADDBUTTONSW:
        case TB_ADDBUTTONSA: {
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            if (result && lParam) {
                RecordNewButtons((const TBBUTTON*)lParam, (int)wParam);
            }
            return result;
        }

        case TB_DELETEBUTTON: {
            int id = IdFromIndex((int)wParam);
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            if (result) {
                g_tray.intentHidden.erase(id);
                g_tray.memberFolder.erase(id);
                g_tray.folderButton.erase(id);
                ScheduleReevaluate();
            }
            return result;
        }

        // Reads of the hidden flag report what Explorer asked for, so that a
        // read followed by a write can't turn the mod's hiding into Explorer's.
        case TB_ISBUTTONHIDDEN: {
            if (auto it = g_tray.intentHidden.find((int)wParam);
                it != g_tray.intentHidden.end()) {
                return it->second;
            }
            break;
        }

        case TB_GETSTATE: {
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            if (result != -1) {
                if (auto it = g_tray.intentHidden.find((int)wParam);
                    it != g_tray.intentHidden.end()) {
                    result = WithHidden((BYTE)result, it->second);
                }
            }
            return result;
        }

        case TB_GETBUTTON: {
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            auto* button = (TBBUTTON*)lParam;
            if (result && button) {
                if (auto it = g_tray.intentHidden.find(button->idCommand);
                    it != g_tray.intentHidden.end()) {
                    button->fsState = WithHidden(button->fsState, it->second);
                }
            }
            return result;
        }

        case TB_GETBUTTONINFOW:
        case TB_GETBUTTONINFOA: {
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            auto* info = (TBBUTTONINFOW*)lParam;
            if (result != -1 && info && info->cbSize >= sizeof(TBBUTTONINFOW) &&
                (info->dwMask & TBIF_STATE)) {
                int id = (info->dwMask & TBIF_BYINDEX)
                             ? IdFromIndex((int)wParam)
                             : (int)wParam;
                if (auto it = g_tray.intentHidden.find(id);
                    it != g_tray.intentHidden.end()) {
                    info->fsState = WithHidden(info->fsState, it->second);
                }
            }
            return result;
        }

        case WM_WINDOWPOSCHANGED: {
            auto* pos = (const WINDOWPOS*)lParam;
            if (!g_tray.fitting && pos && !(pos->flags & SWP_NOSIZE)) {
                PostFit();
            }
            break;
        }
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK OverflowSubclassProc(HWND hWnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam,
                                      DWORD_PTR dwRefData) {
    if (uMsg == WM_NCDESTROY) {
        if (g_tray.overflow == hWnd) {
            g_tray.overflow = nullptr;
            g_tray.overflowHooked = false;
            g_overflowWnd = nullptr;
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (g_tray.overflow != hWnd || !g_tray.toolbar) {
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    if (uMsg == g_msgFit) {
        g_tray.fitPosted = false;
        Fit();
        return 0;
    }

    switch (uMsg) {
        case WM_WINDOWPOSCHANGING: {
            auto* pos = (const WINDOWPOS*)lParam;
            if (pos && (pos->flags & SWP_SHOWWINDOW) && !g_tray.restoring) {
                OnOverflowShowing();
            }
            break;
        }

        case WM_WINDOWPOSCHANGED: {
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            auto* pos = (const WINDOWPOS*)lParam;
            if (!pos) {
                return result;
            }
            if (pos->flags & SWP_HIDEWINDOW) {
                OnOverflowHidden();
            } else if (!g_tray.fitting && IsWindowVisible(hWnd) &&
                       ((pos->flags & SWP_SHOWWINDOW) ||
                        !(pos->flags & SWP_NOSIZE))) {
                // After Explorer has finished laying the popup out.
                PostFit();
            }
            return result;
        }
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK TrayWndSubclassProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     DWORD_PTR dwRefData) {
    if (uMsg == WM_COPYDATA) {
        RecordTrayCopyData((const COPYDATASTRUCT*)lParam);
    } else if (uMsg == WM_NCDESTROY) {
        HWND expected = hWnd;
        g_trayWnd.compare_exchange_strong(expected, nullptr);
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
// Finding the tray windows

bool IsOwnProcessWindow(HWND hWnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    return pid == GetCurrentProcessId();
}

void AttachTrayWnd(HWND hWnd) {
    if (g_trayWnd == hWnd) {
        return;
    }
    if (WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd,
                                                      TrayWndSubclassProc, 0)) {
        g_trayWnd = hWnd;
    } else {
        Wh_Log(L"Couldn't hook Shell_TrayWnd, tooltips come from the registry "
               L"only");
    }
}

void AttachToolbar(HWND hWnd) {
    if (g_toolbarWnd == hWnd) {
        return;
    }
    if (!WindhawkUtils::SetWindowSubclassFromAnyThread(hWnd,
                                                       ToolbarSubclassProc, 0)) {
        Wh_Log(L"Couldn't hook the overflow toolbar");
        return;
    }
    SendMessageW(hWnd, g_msgAttach, 0, 0);
}

void OnWindowCreated(HWND hWnd) {
    WCHAR className[64];
    if (!GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
        return;
    }

    if (_wcsicmp(className, L"Shell_TrayWnd") == 0) {
        AttachTrayWnd(hWnd);
    } else if (_wcsicmp(className, TOOLBARCLASSNAMEW) == 0) {
        HWND parent = GetParent(hWnd);
        WCHAR parentClass[64];
        if (parent && GetClassNameW(parent, parentClass, ARRAYSIZE(parentClass)) &&
            _wcsicmp(parentClass, L"NotifyIconOverflowWindow") == 0) {
            AttachToolbar(hWnd);
        }
    }
}

// For when the mod is enabled while Explorer is already running.
void AttachToExistingWindows() {
    EnumWindows(
        [](HWND hWnd, LPARAM) -> BOOL {
            WCHAR className[64];
            if (!IsOwnProcessWindow(hWnd) ||
                !GetClassNameW(hWnd, className, ARRAYSIZE(className))) {
                return TRUE;
            }
            if (_wcsicmp(className, L"Shell_TrayWnd") == 0) {
                AttachTrayWnd(hWnd);
            } else if (_wcsicmp(className, L"NotifyIconOverflowWindow") == 0) {
                if (HWND toolbar = FindWindowExW(hWnd, nullptr,
                                                 TOOLBARCLASSNAMEW, nullptr)) {
                    AttachToolbar(toolbar);
                }
            }
            return TRUE;
        },
        0);

    if (!g_toolbarWnd) {
        Wh_Log(L"No classic tray overflow yet. With the Windows 11 taskbar "
               L"there is none and the mod stays idle");
    }
}

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;
HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle,
                                 LPCWSTR lpClassName,
                                 LPCWSTR lpWindowName,
                                 DWORD dwStyle,
                                 int X,
                                 int Y,
                                 int nWidth,
                                 int nHeight,
                                 HWND hWndParent,
                                 HMENU hMenu,
                                 HINSTANCE hInstance,
                                 LPVOID lpParam) {
    HWND hWnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName,
                                         dwStyle, X, Y, nWidth, nHeight,
                                         hWndParent, hMenu, hInstance, lpParam);
    if (hWnd && !g_unloading) {
        OnWindowCreated(hWnd);
    }
    return hWnd;
}

using CreateWindowInBand_t = HWND(WINAPI*)(DWORD dwExStyle,
                                           LPCWSTR lpClassName,
                                           LPCWSTR lpWindowName,
                                           DWORD dwStyle,
                                           int X,
                                           int Y,
                                           int nWidth,
                                           int nHeight,
                                           HWND hWndParent,
                                           HMENU hMenu,
                                           HINSTANCE hInstance,
                                           LPVOID lpParam,
                                           DWORD dwBand);
CreateWindowInBand_t CreateWindowInBand_Original;
HWND WINAPI CreateWindowInBand_Hook(DWORD dwExStyle,
                                    LPCWSTR lpClassName,
                                    LPCWSTR lpWindowName,
                                    DWORD dwStyle,
                                    int X,
                                    int Y,
                                    int nWidth,
                                    int nHeight,
                                    HWND hWndParent,
                                    HMENU hMenu,
                                    HINSTANCE hInstance,
                                    LPVOID lpParam,
                                    DWORD dwBand) {
    HWND hWnd = CreateWindowInBand_Original(
        dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight,
        hWndParent, hMenu, hInstance, lpParam, dwBand);
    if (hWnd && !g_unloading) {
        OnWindowCreated(hWnd);
    }
    return hWnd;
}

////////////////////////////////////////////////////////////////////////////////
// Folder icon images (mod thread)

// Pixels are 0xAARRGGBB with straight alpha, which is also the byte order of a
// 32-bit top-down DIB.
using Pixels = std::vector<uint32_t>;

void BlendOver(uint32_t& destination, uint32_t source) {
    uint32_t sourceAlpha = source >> 24;
    if (!sourceAlpha) {
        return;
    }
    uint32_t destinationAlpha = destination >> 24;
    float sa = sourceAlpha / 255.0f;
    float da = destinationAlpha / 255.0f * (1.0f - sa);
    float outAlpha = sa + da;
    uint32_t result = (uint32_t)(outAlpha * 255.0f + 0.5f) << 24;
    for (int shift = 0; shift < 24; shift += 8) {
        float s = ((source >> shift) & 0xFF) * sa;
        float d = ((destination >> shift) & 0xFF) * da;
        result |= (uint32_t)((s + d) / outAlpha + 0.5f) << shift;
    }
    destination = result;
}

bool IconToPixels(HICON icon, int* width, int* height, Pixels* pixels) {
    ICONINFO info;
    if (!GetIconInfo(icon, &info)) {
        return false;
    }

    bool ok = false;
    BITMAP bitmap;
    if (info.hbmColor &&
        GetObjectW(info.hbmColor, sizeof(bitmap), &bitmap) &&
        bitmap.bmWidth > 0 && bitmap.bmHeight > 0) {
        *width = bitmap.bmWidth;
        *height = bitmap.bmHeight;
        pixels->assign((size_t)*width * *height, 0);

        BITMAPINFO bitmapInfo{};
        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = *width;
        bitmapInfo.bmiHeader.biHeight = -*height;
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        HDC dc = GetDC(nullptr);
        ok = GetDIBits(dc, info.hbmColor, 0, *height, pixels->data(),
                       &bitmapInfo, DIB_RGB_COLORS) != 0;

        // Icons without an alpha channel carry transparency in the mask.
        if (ok && std::none_of(pixels->begin(), pixels->end(),
                               [](uint32_t p) { return p >> 24; })) {
            Pixels mask((size_t)*width * *height, 0);
            if (info.hbmMask &&
                GetDIBits(dc, info.hbmMask, 0, *height, mask.data(),
                          &bitmapInfo, DIB_RGB_COLORS)) {
                for (size_t i = 0; i < pixels->size(); i++) {
                    if (!(mask[i] & 0xFFFFFF)) {
                        (*pixels)[i] |= 0xFF000000;
                    }
                }
            }
        }
        ReleaseDC(nullptr, dc);
    }

    if (info.hbmColor) {
        DeleteObject(info.hbmColor);
    }
    if (info.hbmMask) {
        DeleteObject(info.hbmMask);
    }
    return ok;
}

// Area-averaged downscale of a source image into a square cell of the
// destination, blended over what is already there.
void DrawScaled(Pixels& destination,
                int size,
                int left,
                int top,
                int cell,
                Pixels const& source,
                int sourceWidth,
                int sourceHeight) {
    for (int y = 0; y < cell; y++) {
        int y0 = y * sourceHeight / cell;
        int y1 = std::max(y0 + 1, (y + 1) * sourceHeight / cell);
        for (int x = 0; x < cell; x++) {
            int x0 = x * sourceWidth / cell;
            int x1 = std::max(x0 + 1, (x + 1) * sourceWidth / cell);

            float alpha = 0;
            float channels[3] = {};
            int samples = 0;
            for (int sy = y0; sy < y1 && sy < sourceHeight; sy++) {
                for (int sx = x0; sx < x1 && sx < sourceWidth; sx++) {
                    uint32_t p = source[(size_t)sy * sourceWidth + sx];
                    float a = (p >> 24) / 255.0f;
                    alpha += a;
                    for (int c = 0; c < 3; c++) {
                        channels[c] += ((p >> (c * 8)) & 0xFF) * a;
                    }
                    samples++;
                }
            }
            if (!samples || alpha <= 0) {
                continue;
            }

            uint32_t pixel =
                (uint32_t)(alpha / samples * 255.0f + 0.5f) << 24;
            for (int c = 0; c < 3; c++) {
                pixel |= (uint32_t)(channels[c] / alpha + 0.5f) << (c * 8);
            }
            int dx = left + x;
            int dy = top + y;
            if (dx >= 0 && dx < size && dy >= 0 && dy < size) {
                BlendOver(destination[(size_t)dy * size + dx], pixel);
            }
        }
    }
}

uint32_t WithAlpha(uint32_t color, uint32_t alpha) {
    return (((color >> 24) * alpha / 255) << 24) | (color & 0xFFFFFF);
}

// A faint rounded plate with up to four icons on it, two by two.
Pixels ComposePreview(int size, uint32_t color, std::vector<HICON> const& icons) {
    Pixels pixels((size_t)size * size, 0);

    uint32_t plate = WithAlpha(color, 0x40);
    int radius = std::max(2, size / 6);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int cx = x < radius ? radius - x - 1
                     : x >= size - radius ? x - (size - radius)
                                          : -1;
            int cy = y < radius ? radius - y - 1
                     : y >= size - radius ? y - (size - radius)
                                          : -1;
            if (cx >= 0 && cy >= 0 &&
                (cx + 1) * (cx + 1) + (cy + 1) * (cy + 1) > radius * radius) {
                continue;
            }
            pixels[(size_t)y * size + x] = plate;
        }
    }

    int pad = std::max(1, size / 16);
    int cell = (size - 3 * pad) / 2;
    for (size_t i = 0; i < icons.size() && i < 4; i++) {
        int width;
        int height;
        Pixels source;
        if (!IconToPixels(icons[i], &width, &height, &source)) {
            continue;
        }
        int left = pad + (int)(i % 2) * (cell + pad);
        int top = pad + (int)(i / 2) * (cell + pad);
        DrawScaled(pixels, size, left, top, cell, source, width, height);
    }

    return pixels;
}

HFONT CreateGlyphFont(int size) {
    static const WCHAR* face = nullptr;

    LOGFONTW logFont{};
    logFont.lfHeight = -size;
    logFont.lfWeight = FW_NORMAL;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfQuality = ANTIALIASED_QUALITY;

    if (!face) {
        // Segoe Fluent Icons ships with Windows 11, MDL2 Assets with 10.
        face = L"Segoe MDL2 Assets";
        wcscpy_s(logFont.lfFaceName, L"Segoe Fluent Icons");
        HFONT probe = CreateFontIndirectW(&logFont);
        HDC dc = CreateCompatibleDC(nullptr);
        HGDIOBJ old = SelectObject(dc, probe);
        WCHAR selected[LF_FACESIZE];
        if (GetTextFaceW(dc, ARRAYSIZE(selected), selected) &&
            _wcsicmp(selected, L"Segoe Fluent Icons") == 0) {
            face = L"Segoe Fluent Icons";
        }
        SelectObject(dc, old);
        DeleteDC(dc);
        DeleteObject(probe);
    }

    wcscpy_s(logFont.lfFaceName, face);
    return CreateFontIndirectW(&logFont);
}

Pixels RenderGlyph(int size, uint32_t color, std::wstring const& glyph) {
    Pixels pixels((size_t)size * size, 0);

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = size;
    bitmapInfo.bmiHeader.biHeight = -size;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC dc = CreateCompatibleDC(nullptr);
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bitmapInfo, DIB_RGB_COLORS, &bits,
                                      nullptr, 0);
    if (!bitmap || !bits) {
        DeleteDC(dc);
        return pixels;
    }
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
    memset(bits, 0, (size_t)size * size * 4);

    // White on black with grayscale antialiasing, so any channel is coverage.
    HFONT font = CreateGlyphFont(size);
    HGDIOBJ oldFont = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));
    RECT rect{0, 0, size, size};
    DrawTextW(dc, glyph.c_str(), (int)glyph.size(), &rect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    GdiFlush();

    auto* source = (const uint32_t*)bits;
    for (size_t i = 0; i < pixels.size(); i++) {
        uint32_t p = source[i];
        uint32_t coverage =
            std::max({p & 0xFF, (p >> 8) & 0xFF, (p >> 16) & 0xFF});
        pixels[i] = WithAlpha(color, coverage);
    }

    SelectObject(dc, oldFont);
    DeleteObject(font);
    SelectObject(dc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
    return pixels;
}

HICON CreateIconFromPixels(Pixels const& pixels, int size) {
    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = size;
    bitmapInfo.bmiHeader.biHeight = -size;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP color = CreateDIBSection(nullptr, &bitmapInfo, DIB_RGB_COLORS,
                                     &bits, nullptr, 0);
    if (!color || !bits) {
        return nullptr;
    }
    memcpy(bits, pixels.data(), pixels.size() * 4);

    std::vector<BYTE> maskBits((size_t)((size + 15) / 16) * 2 * size, 0);
    HBITMAP mask = CreateBitmap(size, size, 1, 1, maskBits.data());

    ICONINFO info{};
    info.fIcon = TRUE;
    info.hbmMask = mask;
    info.hbmColor = color;
    HICON icon = CreateIconIndirect(&info);

    DeleteObject(color);
    DeleteObject(mask);
    return icon;
}

uint64_t HashPixels(Pixels const& pixels, int size) {
    uint64_t hash = 0xCBF29CE484222325ull ^ (uint64_t)size;
    for (uint32_t p : pixels) {
        hash = (hash ^ p) * 0x100000001B3ull;
    }
    return hash;
}

////////////////////////////////////////////////////////////////////////////////
// Folder tray icons (mod thread)

struct AppliedIcon {
    bool added = false;
    uint64_t hash = 0;
    std::wstring tip;
    bool hidden = false;
};

[[clang::no_destroy]] std::vector<AppliedIcon> g_applied;

void FillIconData(NOTIFYICONDATAW* data, HWND hWnd, int folder) {
    *data = {};
    data->cbSize = sizeof(*data);
    data->hWnd = hWnd;
    data->uID = kFolderIconIdBase + folder;
}

void DeleteFolderIcon(HWND hWnd, int folder) {
    NOTIFYICONDATAW data;
    FillIconData(&data, hWnd, folder);
    Shell_NotifyIconW(NIM_DELETE, &data);
}

void ApplySnapshot(HWND hWnd) {
    std::unique_ptr<Snapshot> snapshot;
    AcquireSRWLockExclusive(&g_snapshotLock);
    snapshot.swap(g_pendingSnapshot);
    ReleaseSRWLockExclusive(&g_snapshotLock);
    if (!snapshot || g_unloading) {
        return;
    }

    size_t count = snapshot->folders.size();
    for (size_t k = count; k < g_applied.size(); k++) {
        if (g_applied[k].added) {
            DeleteFolderIcon(hWnd, (int)k);
        }
    }
    g_applied.resize(count);

    int size = snapshot->iconSize;
    for (size_t k = 0; k < count; k++) {
        auto const& view = snapshot->folders[k];
        auto& applied = g_applied[k];

        std::wstring glyph = view.open ? kBackGlyph : view.glyph;
        if (glyph.empty() && view.previews.empty()) {
            glyph = kFolderGlyph;
        }
        Pixels pixels =
            glyph.empty()
                ? ComposePreview(size, snapshot->glyphColor, view.previews)
                : RenderGlyph(size, snapshot->glyphColor, glyph);
        uint64_t hash = HashPixels(pixels, size);
        std::wstring tip = view.open ? L"Back (" + view.name + L")"
                                     : view.name + L" (" +
                                           std::to_wstring(view.count) + L")";
        bool hidden = !view.visible;

        if (applied.added && applied.hash == hash && applied.tip == tip &&
            applied.hidden == hidden) {
            continue;
        }

        HICON icon = CreateIconFromPixels(pixels, size);

        NOTIFYICONDATAW data;
        FillIconData(&data, hWnd, (int)k);
        data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_STATE;
        data.uCallbackMessage = WM_APP_TRAY_CALLBACK;
        data.hIcon = icon;
        wcsncpy_s(data.szTip, tip.c_str(), _TRUNCATE);
        data.dwStateMask = NIS_HIDDEN;
        data.dwState = hidden ? NIS_HIDDEN : 0;

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

        if (ok) {
            applied.added = true;
            applied.hash = hash;
            applied.tip = std::move(tip);
            applied.hidden = hidden;
        } else {
            // Most likely the tray isn't up yet; TaskbarCreated retries.
            Wh_Log(L"Couldn't %s the icon of folder %d",
                   applied.added ? L"update" : L"add", (int)k);
            applied.added = false;
        }

        if (icon) {
            DestroyIcon(icon);
        }
    }
}

void RequestReevaluate() {
    if (HWND toolbar = g_toolbarWnd) {
        PostMessageW(toolbar, g_msgReevaluate, 0, 0);
    }
}

LRESULT CALLBACK ModWndProc(HWND hWnd,
                            UINT uMsg,
                            WPARAM wParam,
                            LPARAM lParam) {
    if (uMsg == WM_APP_TRAY_CALLBACK) {
        UINT event = LOWORD(lParam);
        UINT id = HIWORD(lParam);
        if ((event == NIN_SELECT || event == NIN_KEYSELECT) &&
            id >= kFolderIconIdBase && id < kFolderIconIdBase + kMaxFolders) {
            if (HWND toolbar = g_toolbarWnd) {
                PostMessageW(toolbar, g_msgToggle, id - kFolderIconIdBase, 0);
            }
        }
        return 0;
    }

    if (uMsg == WM_APP_SNAPSHOT) {
        ApplySnapshot(hWnd);
        return 0;
    }

    if (uMsg == g_msgTaskbarCreated) {
        // The tray was recreated and forgot every icon.
        for (auto& applied : g_applied) {
            applied = {};
        }
        RequestReevaluate();
        return 0;
    }

    if (uMsg == WM_SETTINGCHANGE && lParam &&
        wcscmp((PCWSTR)lParam, L"ImmersiveColorSet") == 0) {
        RequestReevaluate();
        return 0;
    }

    if (uMsg == WM_APP_EXIT) {
        for (size_t k = 0; k < g_applied.size(); k++) {
            if (g_applied[k].added) {
                DeleteFolderIcon(hWnd, (int)k);
            }
        }
        g_applied.clear();
        DestroyWindow(hWnd);
        return 0;
    }

    if (uMsg == WM_DESTROY) {
        g_modWnd = nullptr;
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

constexpr WCHAR kModWindowClass[] = L"TourneClassicTrayFolders_" WH_MOD_ID;

HANDLE g_modThread;

DWORD WINAPI ModThreadProc(LPVOID parameter) {
    HANDLE ready = (HANDLE)parameter;

    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&ModWndProc, &instance);

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = ModWndProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = kModWindowClass;
    RegisterClassW(&windowClass);

    // A hidden top-level window rather than a message-only one, because
    // TaskbarCreated and WM_SETTINGCHANGE are only broadcast to top-level
    // windows.
    HWND hWnd = CreateWindowExW(WS_EX_TOOLWINDOW, kModWindowClass, L"", WS_POPUP,
                                0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
    g_modWnd = hWnd;
    SetEvent(ready);

    if (hWnd) {
        MSG msg;
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    UnregisterClassW(kModWindowClass, instance);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Mod lifecycle

BOOL Wh_ModInit() {
    Wh_Log(L">");

    g_msgAttach = RegisterWindowMessageW(L"TourneClassicTrayFolders_Attach");
    g_msgReevaluate =
        RegisterWindowMessageW(L"TourneClassicTrayFolders_Reevaluate");
    g_msgToggle = RegisterWindowMessageW(L"TourneClassicTrayFolders_Toggle");
    g_msgRestore = RegisterWindowMessageW(L"TourneClassicTrayFolders_Restore");
    g_msgFit = RegisterWindowMessageW(L"TourneClassicTrayFolders_Fit");
    g_msgTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

    g_settings = std::make_shared<const Settings>(LoadSettings());

    WindhawkUtils::SetFunctionHook(CreateWindowExW, CreateWindowExW_Hook,
                                   &CreateWindowExW_Original);

    if (auto createWindowInBand = (CreateWindowInBand_t)GetProcAddress(
            GetModuleHandleW(L"user32.dll"), "CreateWindowInBand")) {
        WindhawkUtils::SetFunctionHook(createWindowInBand,
                                       CreateWindowInBand_Hook,
                                       &CreateWindowInBand_Original);
    }

    HANDLE ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_modThread = CreateThread(nullptr, 0, ModThreadProc, ready, 0, nullptr);
    if (!g_modThread) {
        CloseHandle(ready);
        Wh_Log(L"Couldn't start the mod thread");
        return FALSE;
    }
    WaitForSingleObject(ready, 5000);
    CloseHandle(ready);

    if (!g_modWnd) {
        Wh_Log(L"Couldn't create the mod window");
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L">");

    AttachToExistingWindows();
}

void Wh_ModBeforeUninit() {
    Wh_Log(L">");

    // Put every icon back where Explorer had it before anything is torn down.
    if (HWND toolbar = g_toolbarWnd) {
        SendMessageTimeoutW(toolbar, g_msgRestore, 0, 0, SMTO_NORMAL, 2000,
                            nullptr);
    }

    g_unloading = true;

    if (HWND modWnd = g_modWnd) {
        PostMessageW(modWnd, WM_APP_EXIT, 0, 0);
    }
    if (g_modThread) {
        WaitForSingleObject(g_modThread, 5000);
        CloseHandle(g_modThread);
        g_modThread = nullptr;
    }
}

void Wh_ModUninit() {
    Wh_Log(L">");

    if (HWND toolbar = g_toolbarWnd; toolbar && IsWindow(toolbar)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(toolbar,
                                                         ToolbarSubclassProc);
    }
    if (HWND overflow = g_overflowWnd; overflow && IsWindow(overflow)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(overflow,
                                                         OverflowSubclassProc);
    }
    if (HWND tray = g_trayWnd; tray && IsWindow(tray)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(tray,
                                                         TrayWndSubclassProc);
    }

    AcquireSRWLockExclusive(&g_snapshotLock);
    g_pendingSnapshot.reset();
    ReleaseSRWLockExclusive(&g_snapshotLock);
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");

    auto settings = std::make_shared<const Settings>(LoadSettings());
    AcquireSRWLockExclusive(&g_settingsLock);
    g_settings = std::move(settings);
    ReleaseSRWLockExclusive(&g_settingsLock);

    if (HWND toolbar = g_toolbarWnd) {
        PostMessageW(toolbar, g_msgReevaluate, 1, 0);
    }
}
