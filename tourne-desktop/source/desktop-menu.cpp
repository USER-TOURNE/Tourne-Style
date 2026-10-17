////////////////////////////////////////////////////////////////////////////////
// The desktop's right-click menu in Tourne's menu look ("// @inline desktop-menu.cpp", in taskbar-host.cpp
// after tourne-menu.cpp). Tourne Desktop's desktop (desktop/host/desktop-host.cpp, shell.desktopMenu)
// posts DesktopMenuMessage with what was clicked (its selected items, or the empty space) and its view's
// state, as "key=value" lines. The menu is the shell's own for those (Paste, New, Open in Terminal, the
// extensions), with the view's View, Sort by and Refresh first, styled and run here like a button's shell
// menu. What changes the view goes back to the desktop as DesktopViewMessage, posted, so neither thread
// waits on the other. Shift + right-click keeps Windows' own menu.
//
// Made ahead (taskbar.desktopMenuReady): the empty space's menu is kept built between right-clicks (made
// again a second after each one, and when the clipboard changes, for Paste); a file's starts building as
// the button goes down (the desktop posts its request with "prepare=1" then) and is used if the same
// items are asked for as it comes up; and a few seconds after the bar starts, a thread of its own builds
// and throws away one menu for each kind of item on the desktop, so their extensions are loaded before
// the first right-click. The menus shown are made here, on the bar's thread: a shell menu's objects
// belong to the thread that made them.

UINT DesktopMenuMessage() {
    static UINT msg = RegisterWindowMessageW(L"TourneDesktop.DesktopMenu");
    return msg;
}

UINT DesktopViewMessage() {
    static UINT msg = RegisterWindowMessageW(L"TourneDesktop.DesktopView");
    return msg;
}

// The view's commands (the same numbers in desktop-host.cpp).
enum : int {
    kDvLarge = 1, kDvMedium, kDvSmall,
    kDvAutoArrange = 10, kDvGrid, kDvShowIcons,
    kDvSortName = 20, kDvSortSize, kDvSortType, kDvSortDate,
    kDvRefresh = 30, kDvRename = 40,
    kDvWallChoose = 60, kDvWallNext, kDvWallRecent = 70, kDvWallPlace = 80,  // (recent: + its place; place: + the position)
};
constexpr UINT kDvFirstId = 0x8000;  // our items' ids, above the shell's (1 to 0x7FFF)

struct DesktopRequest {
    std::vector<std::wstring> items;  // parsing names; none for the empty space
    int iconSize = 48;
    bool autoArrange = false, grid = true, icons = true, shift = false;
    std::wstring sort = L"name";
    std::wstring wallpaper;              // the picture showing (the empty space's menu only)
    int placement = -1;                  // how (DESKTOP_WALLPAPER_POSITION)
    std::vector<std::wstring> recent;    // the last few others, newest first
    bool prepare = false;                // the button went down: only build it, for when it comes up
};

DesktopRequest ParseDesktopRequest(std::wstring const& text) {
    DesktopRequest r;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? text.size() : end + 1;
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring key = line.substr(0, eq), value = line.substr(eq + 1);
        if (key == L"item") r.items.push_back(value);
        else if (key == L"iconSize") r.iconSize = _wtoi(value.c_str());
        else if (key == L"autoArrange") r.autoArrange = value == L"1";
        else if (key == L"grid") r.grid = value == L"1";
        else if (key == L"icons") r.icons = value == L"1";
        else if (key == L"shift") r.shift = value == L"1";
        else if (key == L"sort") r.sort = value;
        else if (key == L"wallpaper") r.wallpaper = value;
        else if (key == L"placement") r.placement = _wtoi(value.c_str());
        else if (key == L"recent") r.recent.push_back(value);
        else if (key == L"prepare") r.prepare = value == L"1";
    }
    return r;
}

// The shell's own menu for the items, or for the desktop's empty space.
IContextMenu* DesktopShellMenu(HWND wnd, std::vector<std::wstring> const& items) {
    IContextMenu* menu = nullptr;
    if (items.empty()) {
        IShellFolder* desktop = nullptr;
        if (SUCCEEDED(SHGetDesktopFolder(&desktop))) {
            desktop->CreateViewObject(wnd, IID_PPV_ARGS(&menu));
            desktop->Release();
        }
        return menu;
    }
    std::vector<PIDLIST_ABSOLUTE> pidls;
    for (auto const& item : items) {
        PIDLIST_ABSOLUTE pidl = nullptr;
        if (SUCCEEDED(SHParseDisplayName(item.c_str(), nullptr, &pidl, 0, nullptr)) && pidl) pidls.push_back(pidl);
    }
    IShellItemArray* array = nullptr;
    if (!pidls.empty() && SUCCEEDED(SHCreateShellItemArrayFromIDLists((UINT)pidls.size(), (PCIDLIST_ABSOLUTE_ARRAY)pidls.data(), &array))) {
        array->BindToHandler(nullptr, BHID_SFUIObject, IID_PPV_ARGS(&menu));
        array->Release();
    }
    for (auto pidl : pidls) CoTaskMemFree(pidl);
    return menu;
}

bool MenuHasText(HMENU menu, std::wstring const& want) {
    for (int i = 0, n = GetMenuItemCount(menu); i < n; i++) {
        WCHAR text[256] = L"";
        GetMenuStringW(menu, i, text, ARRAYSIZE(text), MF_BYPOSITION);
        std::wstring plain;
        for (WCHAR* c = text; *c; c++)
            if (*c != L'&') plain += *c;
        if (!_wcsicmp(plain.c_str(), want.c_str())) return true;
    }
    return false;
}

// The view's own part of the menu, first, as Explorer's desktop has it.
void AddDesktopViewItems(HMENU popup, DesktopRequest const& r) {
    auto radio = [](HMENU m, UINT id, bool on, const wchar_t* text) {
        AppendMenuW(m, MF_STRING | (on ? MF_CHECKED : 0), kDvFirstId + id, text);
        if (on) {
            MENUITEMINFOW info{sizeof(info)};
            info.fMask = MIIM_FTYPE;
            info.fType = MFT_RADIOCHECK;
            SetMenuItemInfoW(m, kDvFirstId + id, FALSE, &info);
        }
    };
    HMENU view = CreatePopupMenu();
    radio(view, kDvLarge, r.iconSize >= 96, L"Large icons");
    radio(view, kDvMedium, r.iconSize >= 48 && r.iconSize < 96, L"Medium icons");
    radio(view, kDvSmall, r.iconSize < 48, L"Small icons");
    AppendMenuW(view, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(view, MF_STRING | (r.autoArrange ? MF_CHECKED : 0), kDvFirstId + kDvAutoArrange, L"Auto arrange icons");
    AppendMenuW(view, MF_STRING | (r.grid ? MF_CHECKED : 0), kDvFirstId + kDvGrid, L"Align icons to grid");
    AppendMenuW(view, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(view, MF_STRING | (r.icons ? MF_CHECKED : 0), kDvFirstId + kDvShowIcons, L"Show desktop icons");
    HMENU sort = CreatePopupMenu();
    radio(sort, kDvSortName, r.sort == L"name", L"Name");
    radio(sort, kDvSortSize, r.sort == L"size", L"Size");
    radio(sort, kDvSortType, r.sort == L"type", L"Item type");
    radio(sort, kDvSortDate, r.sort == L"date", L"Date modified");
    // The wallpaper: another picture, one of the last few, and how it's placed.
    HMENU wall = CreatePopupMenu();
    AppendMenuW(wall, MF_STRING, kDvFirstId + kDvWallChoose, L"Choose a picture...");
    AppendMenuW(wall, MF_STRING, kDvFirstId + kDvWallNext, L"Next picture");
    if (!r.recent.empty()) AppendMenuW(wall, MF_SEPARATOR, 0, nullptr);
    for (size_t i = 0; i < r.recent.size() && i < 8; i++) {
        std::wstring name;
        for (const wchar_t* c = PathFindFileNameW(r.recent[i].c_str()); *c; c++) name += *c == L'&' ? std::wstring(L"&&") : std::wstring(1, *c);
        AppendMenuW(wall, MF_STRING, kDvFirstId + kDvWallRecent + (UINT)i, name.c_str());
    }
    AppendMenuW(wall, MF_SEPARATOR, 0, nullptr);
    HMENU place = CreatePopupMenu();
    radio(place, kDvWallPlace + (int)DWPOS_FILL, r.placement == DWPOS_FILL, L"Fill");
    radio(place, kDvWallPlace + (int)DWPOS_FIT, r.placement == DWPOS_FIT, L"Fit");
    radio(place, kDvWallPlace + (int)DWPOS_STRETCH, r.placement == DWPOS_STRETCH, L"Stretch");
    radio(place, kDvWallPlace + (int)DWPOS_TILE, r.placement == DWPOS_TILE, L"Tile");
    radio(place, kDvWallPlace + (int)DWPOS_CENTER, r.placement == DWPOS_CENTER, L"Center");
    radio(place, kDvWallPlace + (int)DWPOS_SPAN, r.placement == DWPOS_SPAN, L"Span");
    AppendMenuW(wall, MF_POPUP, (UINT_PTR)place, L"Placement");
    int at = 0;
    InsertMenuW(popup, at++, MF_BYPOSITION | MF_POPUP, (UINT_PTR)view, L"View");
    InsertMenuW(popup, at++, MF_BYPOSITION | MF_POPUP, (UINT_PTR)sort, L"Sort by");
    InsertMenuW(popup, at++, MF_BYPOSITION | MF_STRING, kDvFirstId + kDvRefresh, L"Refresh");
    InsertMenuW(popup, at++, MF_BYPOSITION | MF_POPUP, (UINT_PTR)wall, L"Wallpaper");
    InsertMenuW(popup, at++, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
    // Windows' own desktop entries, when the shell's background menu didn't bring them.
    bool display = MenuHasText(popup, L"Display settings"), personalize = MenuHasText(popup, L"Personalize");
    if (!display || !personalize) AppendMenuW(popup, MF_SEPARATOR, 0, nullptr);
    if (!display) AppendMenuW(popup, MF_STRING, kDvFirstId + 50, L"Display settings");
    if (!personalize) AppendMenuW(popup, MF_STRING, kDvFirstId + 51, L"Personalize");
}

// The shell's menu for the items (or the empty space), built: false when there's none.
bool BuildDesktopShellMenu(HWND wnd, std::vector<std::wstring> const& items, bool shift, ShellMenu& out) {
    ReleaseShellMenu(out);
    out.menu = DesktopShellMenu(wnd, items);
    if (!out.menu) return false;
    out.menu->QueryInterface(IID_PPV_ARGS(&out.menu2));
    out.menu->QueryInterface(IID_PPV_ARGS(&out.menu3));
    out.popup = CreatePopupMenu();
    UINT query = CMF_NORMAL | (shift ? CMF_EXTENDEDVERBS : 0) | (items.empty() ? 0 : CMF_CANRENAME);
    if (FAILED(out.menu->QueryContextMenu(out.popup, 0, 1, 0x7FFF, query))) {
        ReleaseShellMenu(out);
        return false;
    }
    return true;
}

// The one made ahead, and what it's for (the items one to a line; empty: the empty space).
ShellMenu g_readyMenu;
std::wstring g_readyKey;
bool g_readyMade = false;
DWORD g_readyAt = 0;  // when it was made: a file's is only used for a few seconds
constexpr UINT_PTR kTimerReadyMenu = 31;  // the empty space's menu, made again once things settle

std::wstring DesktopItemsKey(std::vector<std::wstring> const& items) {
    std::wstring key;
    for (auto const& item : items) key += item + L"\n";
    return key;
}

void DropReadyMenu() {
    ReleaseShellMenu(g_readyMenu);
    g_readyKey.clear();
    g_readyMade = false;
}

void MakeReadyMenu(HWND wnd, std::vector<std::wstring> const& items) {
    if (!g_settings.desktopMenuReady || g_shellMenu.open) return;
    std::wstring key = DesktopItemsKey(items);
    if (g_readyMade && g_readyKey == key && (key.empty() || GetTickCount() - g_readyAt < 10000)) return;
    DropReadyMenu();
    if (!BuildDesktopShellMenu(wnd, items, false, g_readyMenu)) return;
    FillShellSubmenus(g_readyMenu, g_readyMenu.popup);
    g_readyKey = key;
    g_readyMade = true;
    g_readyAt = GetTickCount();
}

// The empty space's, again in a moment (after a menu, the clipboard changing, settings applied).
void ReadyMenuSoon(HWND wnd, UINT ms) {
    if (!g_settings.desktopMenuReady) {
        KillTimer(wnd, kTimerReadyMenu);
        DropReadyMenu();
        return;
    }
    SetTimer(wnd, kTimerReadyMenu, ms, nullptr);
}

// The bar's timer (WM_TIMER, kTimerReadyMenu) and the clipboard changing (WM_CLIPBOARDUPDATE).
void OnReadyMenuTimer(HWND wnd) {
    KillTimer(wnd, kTimerReadyMenu);
    if (g_inMenu) {
        ReadyMenuSoon(wnd, 1000);  // (not while a menu is open)
        return;
    }
    MakeReadyMenu(wnd, {});
}

void OnClipboardChanged(HWND wnd) {
    if (!g_readyMade || !g_readyKey.empty()) return;
    DropReadyMenu();  // (its Paste is as the clipboard was)
    ReadyMenuSoon(wnd, 300);
}

// One menu for each kind of item on the desktop, built and thrown away on a thread of its own a few
// seconds after the bar starts, so the extensions they load are in before the first right-click.
DWORD WINAPI WarmDesktopMenus(LPVOID) {
    Sleep(5000);
    if (FAILED(OleInitialize(nullptr))) return 0;
    DWORD started = GetTickCount();
    int built = 0;
    IShellFolder* desktop = nullptr;
    IEnumIDList* list = nullptr;
    if (SUCCEEDED(SHGetDesktopFolder(&desktop)) &&
        desktop->EnumObjects(nullptr, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &list) == S_OK && list) {
        std::vector<std::wstring> kinds;
        PITEMID_CHILD child = nullptr;
        while (list->Next(1, &child, nullptr) == S_OK && built < 40) {
            // Its kind: a folder, a file type, or the item itself (This PC, the Recycle Bin, ...).
            std::wstring kind;
            SFGAOF attrs = SFGAO_FOLDER | SFGAO_FILESYSTEM;
            desktop->GetAttributesOf(1, (PCUITEMID_CHILD_ARRAY)&child, &attrs);
            STRRET name{};
            WCHAR path[MAX_PATH] = L"";
            if (SUCCEEDED(desktop->GetDisplayNameOf(child, SHGDN_FORPARSING, &name))) StrRetToBufW(&name, child, path, ARRAYSIZE(path));
            if (!(attrs & SFGAO_FILESYSTEM)) kind = path;
            else if (attrs & SFGAO_FOLDER) kind = L"<folder>";
            else kind = PathFindExtensionW(path);
            for (auto& c : kind) c = (wchar_t)towlower(c);
            if (std::find(kinds.begin(), kinds.end(), kind) == kinds.end()) {
                kinds.push_back(kind);
                IContextMenu* menu = nullptr;
                if (SUCCEEDED(desktop->GetUIObjectOf(nullptr, 1, (PCUITEMID_CHILD_ARRAY)&child, IID_IContextMenu, nullptr, (void**)&menu))) {
                    HMENU popup = CreatePopupMenu();
                    if (SUCCEEDED(menu->QueryContextMenu(popup, 0, 1, 0x7FFF, CMF_NORMAL | CMF_CANRENAME))) built++;
                    DestroyMenu(popup);
                    menu->Release();
                }
            }
            CoTaskMemFree(child);
        }
    }
    if (list) list->Release();
    if (desktop) desktop->Release();
    Wh_Log(L"desktop menus made ahead: %d kinds in %lu ms", built, GetTickCount() - started);
    OleUninitialize();
    return 0;
}

void WarmDesktopMenusOnce() {
    static bool started;
    if (started || !g_settings.desktopMenuReady) return;
    started = true;
    if (HANDLE thread = CreateThread(nullptr, 0, WarmDesktopMenus, nullptr, 0, nullptr)) CloseHandle(thread);
}

void ShowDesktopMenu(HWND wnd, HWND desktop, std::wstring const& text) {
    DesktopRequest r = ParseDesktopRequest(text);
    if (r.prepare) {
        MakeReadyMenu(wnd, r.items);
        return;
    }
    CloseShellMenu();
    // The one made ahead when it's for these items, else made now.
    if (g_readyMade && g_readyKey == DesktopItemsKey(r.items) && !r.shift &&
        (r.items.empty() || GetTickCount() - g_readyAt < 10000)) {
        g_shellMenu = g_readyMenu;
        g_readyMenu = {};
        g_readyKey.clear();
        g_readyMade = false;
    } else if (!BuildDesktopShellMenu(wnd, r.items, r.shift, g_shellMenu)) {
        return;
    }
    if (r.items.empty()) AddDesktopViewItems(g_shellMenu.popup, r);
    IContextMenu* shell = g_shellMenu.menu;
    MenuRulesUndo rules = ApplyMenuRules(g_shellMenu.popup, g_settings.menuRules, [&](UINT id) {
        return id < kDvFirstId ? ShellMenuVerb(shell, id) : std::wstring();
    });
    GetCursorPos(&g_shellMenu.pt);

    // The bar's menu look, as RunMenu sets it.
    HidePreview();
    if (g_tooltip) SendMessageW(g_tooltip, TTM_POP, 0, 0);
    g_inMenu = true;
    SetForegroundWindow(wnd);
    g_menuLook = BarMenuLook();
    g_shellMenu.open = true;
    int cmd = (int)TrackStyledMenu(g_shellMenu.popup, TPM_RETURNCMD | TPM_RIGHTBUTTON, g_shellMenu.pt.x, g_shellMenu.pt.y, wnd, true);
    g_shellMenu.open = false;
    UndoMenuRules(rules);
    PostMessageW(wnd, WM_NULL, 0, 0);
    g_inMenu = false;

    if (cmd >= (int)kDvFirstId) {
        int view = cmd - (int)kDvFirstId;
        if (view == 50) ShellExecuteW(nullptr, nullptr, L"ms-settings:display", nullptr, nullptr, SW_SHOWNORMAL);
        else if (view == 51) ShellExecuteW(nullptr, nullptr, L"ms-settings:personalization", nullptr, nullptr, SW_SHOWNORMAL);
        else if (view >= kDvWallRecent && view < kDvWallRecent + 8 && view - kDvWallRecent < (int)r.recent.size()) {
            // The picture itself, as listed: the desktop's own list could be in another order by now.
            std::wstring const& path = r.recent[view - kDvWallRecent];
            COPYDATASTRUCT cd{0x54445750 /* TDWP */, (DWORD)((path.size() + 1) * sizeof(wchar_t)), (void*)path.c_str()};
            SendMessageW(desktop, WM_COPYDATA, (WPARAM)wnd, (LPARAM)&cd);
        }
        else PostMessageW(desktop, DesktopViewMessage(), view, 0);
    } else if (cmd > 0) {
        // Rename is the view's (its label edit); everything else runs through the shell's menu.
        WCHAR verb[64] = L"";
        if (SUCCEEDED(g_shellMenu.menu->GetCommandString(cmd - 1, GCS_VERBW, nullptr, (LPSTR)verb, ARRAYSIZE(verb))) &&
            !_wcsicmp(verb, L"rename")) {
            PostMessageW(desktop, DesktopViewMessage(), kDvRename, 0);
        } else {
            CMINVOKECOMMANDINFOEX info{sizeof(info)};
            info.fMask = CMIC_MASK_UNICODE | CMIC_MASK_PTINVOKE;
            info.hwnd = desktop;
            info.lpVerb = MAKEINTRESOURCEA(cmd - 1);
            info.lpVerbW = MAKEINTRESOURCEW(cmd - 1);
            info.nShow = SW_SHOWNORMAL;
            info.ptInvoke = g_shellMenu.pt;
            g_shellMenu.menu->InvokeCommand((CMINVOKECOMMANDINFO*)&info);
        }
    }
    CloseShellMenu();
    ReadyMenuSoon(wnd, 1000);
}
