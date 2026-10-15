// Tray icons, shared by System Icons (its hidden icons flyout) and the
// taskbar (its tray zone): the icon records the Explorer relay sends, clicks
// replayed for their programs, and where the user put each icon. Placed into
// both parts by the build ("// @inline tray-shared.cpp").

[[maybe_unused]] constexpr ULONG_PTR kHiddenIconsCopyId = 0x54484931;  // "THI1"

// What to ask the relay for (its lParam): the icons in the overflow, or all.
[[maybe_unused]] constexpr LPARAM kRelayOverflowIcons = 0;
[[maybe_unused]] constexpr LPARAM kRelayAllIcons = 1;

// The taskbar asks the relay to pin or unpin an app, which Windows lets only
// Explorer do: WM_COPYDATA with this id, '1' (pin) or '0' (unpin), then the
// shortcut's or program's path.
[[maybe_unused]] constexpr ULONG_PTR kRelayPinCopyId = 0x5450494E;  // "TPIN"

struct HiddenIcon {
    HWND owner = nullptr;
    UINT id = 0;
    UINT callback = 0;
    UINT version = 0;
    bool overflow = false;  // In the overflow, as Explorer's tray has it.
    std::wstring tip;
    int size = 0;
    std::vector<uint32_t> pixels;  // Premultiplied BGRA, size x size.
    std::wstring plugin;  // A Tourne Desktop plugin item standing in as an icon (its key; no owner then).
};

std::wstring LowerFileName(std::wstring_view path) {
    size_t slash = path.find_last_of(L"\\/");
    std::wstring name(slash == std::wstring_view::npos ? path : path.substr(slash + 1));
    CharLowerBuffW(name.data(), (DWORD)name.size());
    return name;
}

// A process's image path without opening it, for processes this one can't
// open (elevated ones).
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

// The owner's program as a lower-case path (a device path for processes this
// one can't open).
std::wstring OwnerPath(HWND owner) {
    DWORD pid = 0;
    GetWindowThreadProcessId(owner, &pid);
    std::wstring path;
    if (HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        WCHAR buffer[MAX_PATH * 2];
        DWORD size = ARRAYSIZE(buffer);
        if (QueryFullProcessImageNameW(process, 0, buffer, &size)) {
            path.assign(buffer, size);
        }
        CloseHandle(process);
    }
    if (path.empty()) {
        path = ImageNameOfProcessId(pid);
    }
    CharLowerBuffW(path.data(), (DWORD)path.size());
    return path;
}

// The relay's packet: a count, then per icon the owner window (8 bytes), id,
// callback message, NOTIFYICON version, flags (1: in the overflow), icon
// size and tooltip length, the tooltip, and the icon's pixels.
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
        UINT flags = 0, iconSize = 0, chars = 0;
        if (!take(&owner64, 8) || !take(&icon.id, 4) || !take(&icon.callback, 4) ||
            !take(&icon.version, 4) || !take(&flags, 4) || !take(&iconSize, 4) || !take(&chars, 4) ||
            iconSize > 256 || chars > 4096) {
            break;
        }
        icon.owner = (HWND)(ULONG_PTR)owner64;
        icon.overflow = flags & 1;
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

// The suite's Explorer relay subclasses every tray toolbar, so any one of
// them reaches it, and the taskbar itself for when there's no classic tray.
HWND RelayToolbar() {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    HWND notify = tray ? FindWindowExW(tray, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
    HWND found = nullptr;
    if (notify) {
        EnumChildWindows(
            notify,
            [](HWND hWnd, LPARAM lParam) __stdcall -> BOOL {
                WCHAR className[32];
                if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) &&
                    _wcsicmp(className, TOOLBARCLASSNAMEW) == 0) {
                    *(HWND*)lParam = hWnd;
                    return FALSE;
                }
                return TRUE;
            },
            (LPARAM)&found);
    }
    return found ? found : tray;
}

enum HiddenClick { kClickLeft, kClickRight, kClickMiddle, kClickDouble, kClickSecondOfDouble };

// Replays a click on a tray icon for its program, the way the tray reports
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
    Wh_Log(L"tray: click %d for %s (window %p%s), id %u, message 0x%X, version %u", click, OwnerFileName(icon.owner).c_str(), icon.owner,
           IsWindow(icon.owner) ? L"" : L", gone", icon.id, icon.callback, icon.version);
    for (UINT event : events) {
        if (icon.version >= NOTIFYICON_VERSION_4) {
            PostMessageW(icon.owner, icon.callback, MAKEWPARAM(pt.x, pt.y), MAKELPARAM(event, icon.id));
        } else if ((event != NIN_SELECT && event != WM_CONTEXTMENU) || icon.version == NOTIFYICON_VERSION) {
            PostMessageW(icon.owner, icon.callback, icon.id, event);
        }
    }
}

// Tells a tray icon's program the pointer is over it, as the tray does: some
// (AlienFan) set their tooltip only then.
void HoverHiddenIcon(HiddenIcon const& icon) {
    POINT pt;
    GetCursorPos(&pt);
    if (icon.version >= NOTIFYICON_VERSION_4) {
        PostMessageW(icon.owner, icon.callback, MAKEWPARAM(pt.x, pt.y), MAKELPARAM(WM_MOUSEMOVE, icon.id));
    } else {
        PostMessageW(icon.owner, icon.callback, icon.id, WM_MOUSEMOVE);
    }
}

// Programs that open their window on a double click of their tray icon, from
// a comma or semicolon separated list of program names.
bool OpensOnDoubleClickIn(HWND owner, std::wstring list) {
    std::wstring name = OwnerFileName(owner);
    if (name.empty()) {
        return false;
    }
    std::wstring bare = name.ends_with(L".exe") ? name.substr(0, name.size() - 4) : name;
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

// An icon by program and icon id, which stays the same across restarts.
std::wstring HiddenIconKey(HiddenIcon const& icon) {
    if (!icon.plugin.empty()) {
        return L"plugin:" + icon.plugin;
    }
    return OwnerFileName(icon.owner) + L"|" + std::to_wstring(icon.id);
}

////////////////////////////////////////////////////////////////////////////////
// Where each icon goes, kept in the registry so System Icons and the taskbar
// (and Explorer's side) all read the same: shown in the tray, in the overflow
// flyout, or hidden from both. Icons without a place follow Explorer's tray.

enum TrayPlace : int { kPlaceDefault, kPlaceTray, kPlaceOverflow, kPlaceHidden };

constexpr WCHAR kTrayStoreKey[] = L"Software\\TourneTray";
[[maybe_unused]] constexpr WCHAR kTrayPlacesValue[] = L"TrayPlaces";  // "tray:key", "overflow:key", "hidden:key"
[[maybe_unused]] constexpr WCHAR kTrayOrderValue[] = L"TrayOrder";    // keys, in the tray's order

std::vector<std::wstring> LoadTrayList(PCWSTR value) {
    std::vector<std::wstring> items;
    DWORD size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kTrayStoreKey, value, RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &size) !=
            ERROR_SUCCESS ||
        size < sizeof(WCHAR)) {
        return items;
    }
    std::wstring buffer(size / sizeof(WCHAR) + 2, L'\0');
    size = (DWORD)(buffer.size() * sizeof(WCHAR));
    if (RegGetValueW(HKEY_CURRENT_USER, kTrayStoreKey, value, RRF_RT_REG_MULTI_SZ, nullptr, buffer.data(), &size) !=
        ERROR_SUCCESS) {
        return items;
    }
    const WCHAR* end = buffer.data() + buffer.size();
    for (const WCHAR* p = buffer.data(); p < end && *p; p += wcslen(p) + 1) {
        items.push_back(p);
    }
    return items;
}

void SaveTrayList(PCWSTR value, std::vector<std::wstring> const& items) {
    std::wstring data;
    for (auto const& item : items) {
        data += item;
        data.push_back(L'\0');
    }
    data.push_back(L'\0');
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kTrayStoreKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr) ==
        ERROR_SUCCESS) {
        RegSetValueExW(key, value, 0, REG_MULTI_SZ, (const BYTE*)data.data(), (DWORD)(data.size() * sizeof(WCHAR)));
        RegCloseKey(key);
    }
}

constexpr PCWSTR kPlaceNames[] = {L"", L"tray", L"overflow", L"hidden"};

struct TrayPlaces {
    std::vector<std::pair<std::wstring, int>> places;  // key, TrayPlace

    static TrayPlaces Load() {
        TrayPlaces result;
        for (auto const& item : LoadTrayList(kTrayPlacesValue)) {
            size_t colon = item.find(L':');
            if (colon == std::wstring::npos) {
                continue;
            }
            std::wstring name = item.substr(0, colon);
            for (int place = kPlaceTray; place <= kPlaceHidden; place++) {
                if (name == kPlaceNames[place]) {
                    result.places.push_back({item.substr(colon + 1), place});
                }
            }
        }
        return result;
    }

    int Of(std::wstring const& key) const {
        for (auto const& [k, place] : places) {
            if (k == key) {
                return place;
            }
        }
        return kPlaceDefault;
    }

    // Where an icon goes: its own place, else where Explorer's tray has it.
    int For(HiddenIcon const& icon, std::wstring const& key) const {
        int place = Of(key);
        return place != kPlaceDefault ? place : icon.overflow ? kPlaceOverflow : kPlaceTray;
    }

    void Set(std::wstring const& key, int place) {
        // A plugin item's own place is the tray: back there, it keeps no place.
        if (place == kPlaceTray && key.starts_with(L"plugin:")) {
            place = kPlaceDefault;
        }
        std::erase_if(places, [&](auto const& entry) { return entry.first == key; });
        if (place != kPlaceDefault) {
            places.push_back({key, place});
        }
        std::vector<std::wstring> items;
        for (auto const& [k, p] : places) {
            items.push_back(std::wstring(kPlaceNames[p]) + L":" + k);
        }
        SaveTrayList(kTrayPlacesValue, items);
    }
};

////////////////////////////////////////////////////////////////////////////////
// Dragging an icon between the taskbar's tray and System Icons' hidden icons
// flyout (both run in the suite's tool process). The side it's dragged from
// tells the window under the pointer where it is (kTrayDragOverMessage:
// wParam 1 with the screen point in lParam, 0 when the pointer leaves), then
// hands the icon over with WM_COPYDATA (kTrayDropCopyId: the drop point, then
// the icon's key). The side dropped on places it and answers TRUE.

[[maybe_unused]] constexpr ULONG_PTR kTrayDropCopyId = 0x54445250;  // "TDRP"
[[maybe_unused]] constexpr WCHAR kTrayDragOverMessage[] = L"TourneSuite_TrayDragOver";
[[maybe_unused]] constexpr WCHAR kTaskbarClassPrefix[] = L"TourneTaskbar";
[[maybe_unused]] constexpr WCHAR kFlyoutClassPrefix[] = L"TourneTrayFlyout_";

// The top-level window under a screen point, if its class starts with the
// prefix.
HWND DropTargetAt(POINT pt, PCWSTR prefix) {
    HWND hit = WindowFromPoint(pt);
    HWND root = hit ? GetAncestor(hit, GA_ROOT) : nullptr;
    WCHAR cls[64];
    if (!root || !GetClassNameW(root, cls, ARRAYSIZE(cls)) || wcsncmp(cls, prefix, wcslen(prefix)) != 0) {
        return nullptr;
    }
    return root;
}

bool SendTrayDrop(HWND target, HWND from, POINT pt, std::wstring const& key) {
    std::vector<BYTE> data(sizeof(POINT) + (key.size() + 1) * sizeof(WCHAR));
    memcpy(data.data(), &pt, sizeof(pt));
    memcpy(data.data() + sizeof(pt), key.c_str(), (key.size() + 1) * sizeof(WCHAR));
    COPYDATASTRUCT copyData{kTrayDropCopyId, (DWORD)data.size(), data.data()};
    DWORD_PTR result = 0;
    return SendMessageTimeoutW(target, WM_COPYDATA, (WPARAM)from, (LPARAM)&copyData, SMTO_ABORTIFHUNG, 2000,
                               &result) &&
           result == TRUE;
}

bool ReadTrayDrop(COPYDATASTRUCT const* copyData, POINT* pt, std::wstring* key) {
    if (!copyData || copyData->dwData != kTrayDropCopyId || copyData->cbData < sizeof(POINT) + sizeof(WCHAR)) {
        return false;
    }
    auto* bytes = (const BYTE*)copyData->lpData;
    memcpy(pt, bytes, sizeof(POINT));
    key->assign((PCWSTR)(bytes + sizeof(POINT)), (copyData->cbData - sizeof(POINT)) / sizeof(WCHAR));
    key->erase(std::find(key->begin(), key->end(), L'\0'), key->end());
    return !key->empty();
}

// The dragged icon following the pointer once it's outside its own window.
// See-through to the pointer, so the window under it is still found.
struct DragGhost {
    HWND wnd = nullptr;
    int size = 0;

    void Move(HiddenIcon const& icon, POINT pt) {
        if (!icon.size || icon.pixels.size() < (size_t)icon.size * icon.size) {
            return;
        }
        if (wnd) {
            SetWindowPos(wnd, HWND_TOPMOST, pt.x - size / 2, pt.y - size / 2, 0, 0,
                         SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
            return;
        }
        constexpr WCHAR kClass[] = L"TourneTrayDragGhost";
        WNDCLASSW wc{};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kClass;
        RegisterClassW(&wc);  // Already there when the other part made one: fine.
        wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
                              kClass, L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
        if (!wnd) {
            return;
        }
        size = icon.size;
        HDC screen = GetDC(nullptr);
        HDC mem = CreateCompatibleDC(screen);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), size, -size, 1, 32, BI_RGB};
        void* bits = nullptr;
        HBITMAP bitmap = CreateDIBSection(mem, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (bitmap) {
            memcpy(bits, icon.pixels.data(), (size_t)size * size * 4);
            HGDIOBJ old = SelectObject(mem, bitmap);
            POINT dst{pt.x - size / 2, pt.y - size / 2}, src{0, 0};
            SIZE extent{size, size};
            BLENDFUNCTION blend{AC_SRC_OVER, 0, 210, AC_SRC_ALPHA};
            UpdateLayeredWindow(wnd, screen, &dst, &extent, mem, &src, 0, &blend, ULW_ALPHA);
            SelectObject(mem, old);
            DeleteObject(bitmap);
        }
        DeleteDC(mem);
        ReleaseDC(nullptr, screen);
        ShowWindow(wnd, SW_SHOWNOACTIVATE);
    }

    void Hide() {
        if (wnd) {
            DestroyWindow(wnd);
        }
        wnd = nullptr;
        size = 0;
    }
};

// Sorts icons into a saved order: ordered ones first, in their order; the
// rest after them, as they came.
template <typename T, typename KeyOf>
void ApplyTrayOrder(std::vector<T>& items, std::vector<std::wstring> const& order, KeyOf keyOf) {
    if (order.empty()) {
        return;
    }
    std::vector<std::pair<size_t, size_t>> ranks;  // Place in the order, index.
    for (size_t i = 0; i < items.size(); i++) {
        auto it = std::find(order.begin(), order.end(), keyOf(items[i]));
        ranks.push_back({(size_t)(it - order.begin()), i});
    }
    std::stable_sort(ranks.begin(), ranks.end(), [](auto const& a, auto const& b) { return a.first < b.first; });
    std::vector<T> sorted;
    for (auto const& [rank, i] : ranks) {
        sorted.push_back(std::move(items[i]));
    }
    items = std::move(sorted);
}
