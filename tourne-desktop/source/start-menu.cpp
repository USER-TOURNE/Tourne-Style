////////////////////////////////////////////////////////////////////////////////
// The Start menu ("// @inline start-menu.cpp", in taskbar-host.cpp before
// WndProc, so it shares the bar's theme, colours, fonts and menus, and runs
// on the bar's thread).
//
// The Windows 7 layout, as StartAllBack draws it: programs on the left (your
// pins, then the ones you use most; All Programs; search), places on the
// right, Shut down at the bottom. Drawn from the visual style's own Start
// menu parts (StartPanelPriv) when the taskbar's style has them, else from
// the colour scheme. Each place on the right is a link, a menu with flyouts
// for what's inside, or the program's jump list, as set.

// StartPanelPriv parts, as StartAllBack's styles draw them.
constexpr int kSpList = 2;           // the program list, without the top strip
constexpr int kSpAllArrow = 4;       // All Programs' arrow
constexpr int kSpPlaceArrow = 5;     // a place with a menu
constexpr int kSpLeft = 6;           // the left pane: top strip, then the list
constexpr int kSpSeparator = 8;      // across the left pane
constexpr int kSpRight = 9;          // the right pane: top strip, then the places
constexpr int kSpPlaceSeparator = 11;
constexpr int kSpBottom = 12;        // the bar along the bottom
constexpr int kSpSearch = 14;
constexpr int kSpItem = 16;          // a hot (2) or pressed (3) item
constexpr int kSpBackArrow = 21;
constexpr int kSpShutdownButton = 23;  // a grey split button with an arrow
constexpr int kSpAccent = 22;        // its text is the style's accent

constexpr WCHAR kStartClass[] = L"TourneStartMenu_" WH_MOD_ID;
constexpr WCHAR kStartKey[] = L"Software\\Tourne\\StartMenu";
constexpr UINT WM_APP_START = WM_APP + 5;  // the Windows key: open or close it
constexpr UINT_PTR kTimerFlyout = 1, kTimerApps = 2;
constexpr UINT_PTR kTimerPhoneNotes = 4;  // the phone's notifications changed: read once Windows has them

////////////////////////////////////////////////////////////////////////////////
// Settings

enum StartKind { kKindLink, kKindMenu, kKindJump };

uint32_t RefRgb(COLORREF c) {
    return (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
}

struct StartPlaceSetting {
    std::wstring item, kind, path, name;
};

// What the power button and menu do.
enum : int { kPwSwitch = 1, kPwSignOut, kPwLock, kPwSleep, kPwRestart, kPwShutdown, kPwHibernate };

PCWSTR PowerLabel(int action) {
    switch (action) {
        case kPwSwitch: return L"Switch user";
        case kPwSignOut: return L"Sign out";
        case kPwLock: return L"Lock";
        case kPwSleep: return L"Sleep";
        case kPwRestart: return L"Restart";
        case kPwHibernate: return L"Hibernate";
    }
    return L"Shut down";
}

// Laid out as StartAllBack's Start Menu page (see sab-settings-map.md), and
// its Welcome page's presets.
struct StartConfig {
    bool enabled = true;
    std::wstring stylePath;       // the visual style it's drawn from, or none
    bool round = false;           // Proper 11: rounded, as Windows 11's
    int iconSize = 16;            // the lists' icons, at 100%: S, M, L
    int iconCount = 10;           // recently opened programs under the pins
    bool myLarge = false;         // My Programs: large icons
    bool allLarge = false;        // All Programs: large icons
    bool allFlyout = false;       // All Programs as a flyout menu (Windows XP style)
    bool appsInFolder = true;     // Modern apps in a folder of their own
    int sortOrder = 0;            // 0 items first, 1 folders first, 2 no preference
    bool searchLarge = false;
    bool searchSettings = true;   // search programs and settings
    bool searchPublic = false;    // search public folders
    bool searchFlyout = false;    // more results in a flyout, not a window
    bool highlightNew = false;
    bool autoCascade = false;     // menus open on a pause over them
    bool userPicture = false;
    bool glyphIcons = false;
    bool multiColumn = false;
    bool largeFlyoutIcons = false;
    int powerButton = 0;          // kPw*, set below
    std::vector<StartPlaceSetting> places;
    std::wstring appsFolderIcon;
    std::wstring folderIcon;  // All Programs' other folders ("file,index"; empty: Windows' own)
    // Our own additions.
    bool winKey = false;
    bool onPress = true;
    bool caps = false;
    std::wstring colors = L"auto";
    bool textColorSet = false;  // yours, else System Text Color's colour, as it gives StartAllBack's text
    uint32_t textColor = 0;
    bool hotTextSet = false;    // yours for pointed-at text, else the text colour above when it's yours
    uint32_t hotText = 0;
    std::wstring searchIcon;    // the search box's magnifier ("file,index"; empty: StartAllBack's)
    bool phone = false;         // the phone pane, with a phone linked in Phone Link (phone-link.cpp)
    int phoneNotes = 4;         // its recent notifications
    std::wstring phoneIcon;     // the phone's icon ("file,index"; empty: Phone Link's, through Button icons)
    std::wstring phoneActionIcons[4];  // Messages, Notifications, Send a file, Phone settings
    bool phoneAppIcons = false;  // notifications with their phone apps' own logos (else their designs, themed)
    bool phoneTray = false;     // the tray's phone (Phone Link's icon) opens it as a flyout
    int phoneTexts = 3;         // its conversations, Tourne holding its texts itself (phone-map.cpp); 0: Phone Link's link
};
StartConfig g_startConfig;

// StartAllBack's custom folders for the right side, as it keeps them
// (StartPage2's CustomFolders): per folder its item list's length, the list,
// then how it's shown (0 not, 1 a link, 2 a menu) and 2 bytes more.
std::vector<std::pair<std::wstring, DWORD>> ReadSabCustomFolders() {
    std::vector<std::pair<std::wstring, DWORD>> folders;
    constexpr WCHAR kStartPage2[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage2";
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kStartPage2, L"CustomFolders", RRF_RT_REG_BINARY, nullptr, nullptr, &bytes) !=
            ERROR_SUCCESS ||
        !bytes)
        return folders;
    std::vector<BYTE> data(bytes);
    if (RegGetValueW(HKEY_CURRENT_USER, kStartPage2, L"CustomFolders", RRF_RT_REG_BINARY, nullptr, data.data(),
                     &bytes) != ERROR_SUCCESS)
        return folders;
    size_t at = 0;
    while (at + 2 <= data.size()) {
        WORD length = 0;
        memcpy(&length, &data[at], 2);
        at += 2;
        if (!length || at + length + 4 > data.size()) break;
        std::vector<BYTE> list(data.begin() + at, data.begin() + at + length);
        list.insert(list.end(), 4, 0);
        at += length;
        WORD mode = 0;
        memcpy(&mode, &data[at], 2);
        at += 4;
        PWSTR name = nullptr;
        if (SUCCEEDED(SHGetNameFromIDList((PCIDLIST_ABSOLUTE)list.data(), SIGDN_DESKTOPABSOLUTEPARSING, &name)) && name)
            folders.push_back({name, mode});
        CoTaskMemFree(name);
    }
    return folders;
}

// The Welcome page's preset, when one is chosen: "proper11", "kinda10",
// "remastered7", else empty. The taskbar and Explorer Style read it too.
std::wstring StartPreset() {
    std::wstring preset = TextSetting(L"preset");
    return preset == L"proper11" || preset == L"kinda10" || preset == L"remastered7" ? preset : L"";
}

void ForgetPhoneIcons();  // (the phone's, below)
void LoadPhoneState();
void LoadStartSize();

void LoadStartSettings() {
    StartConfig c;
    c.enabled = Wh_GetIntSetting(L"startMenu.enabled") != 0;
    // "startallback" (or unset): StartAllBack's Start menu style (its Start
    // Menu page), else the taskbar's; a preset decides it while chosen.
    std::wstring preset = StartPreset();
    std::wstring style = TextSetting(L"startMenu.visualStyle");
    if (preset == L"proper11") style = L"none";
    else if (preset == L"kinda10") style = L"Plain8.msstyles";
    else if (preset == L"remastered7") style = L"Windows 7.msstyles";
    c.round = preset == L"proper11";
    if (style == L"taskbar") {
        c.stylePath = g_settings.style;
    } else if (style != L"none") {
        if (style.empty() || style == L"startallback") style = ReadStartIsBack(L"AlterStyle");
        c.stylePath = style.empty() ? g_settings.style : FindStyle(style);
    }
    std::wstring size = TextSetting(L"startMenu.iconSize");
    c.iconSize = size == L"large" ? 32 : size == L"medium" ? 24 : 16;
    c.iconCount = std::clamp(Wh_GetIntSetting(L"startMenu.iconCount"), 0, 60);
    c.myLarge = Wh_GetIntSetting(L"startMenu.myPrograms.largeIcons") != 0;
    c.allLarge = Wh_GetIntSetting(L"startMenu.allPrograms.largeIcons") != 0;
    c.allFlyout = Wh_GetIntSetting(L"startMenu.allPrograms.flyout") != 0;
    {
        // All Programs' own "Display as a menu", kept until the setting
        // itself changes.
        DWORD chosen = 0, setting = 0, size = sizeof(DWORD);
        if (RegGetValueW(HKEY_CURRENT_USER, kStartKey, L"AllProgramsFlyout", RRF_RT_REG_DWORD, nullptr, &chosen, &size) ==
            ERROR_SUCCESS) {
            size = sizeof(DWORD);
            RegGetValueW(HKEY_CURRENT_USER, kStartKey, L"AllProgramsFlyoutSetting", RRF_RT_REG_DWORD, nullptr, &setting,
                         &size);
            if (setting == (DWORD)c.allFlyout) c.allFlyout = chosen != 0;
            else RegDeleteKeyValueW(HKEY_CURRENT_USER, kStartKey, L"AllProgramsFlyout");
        }
    }
    c.appsInFolder = Wh_GetIntSetting(L"startMenu.allPrograms.modernAppsFolder") != 0;
    std::wstring sort = TextSetting(L"startMenu.allPrograms.sortOrder");
    c.sortOrder = sort == L"foldersFirst" ? 1 : sort == L"noPreference" ? 2 : 0;
    c.searchLarge = Wh_GetIntSetting(L"startMenu.search.largeIcons") != 0;
    c.searchSettings = Wh_GetIntSetting(L"startMenu.search.programsAndSettings") != 0;
    c.searchPublic = Wh_GetIntSetting(L"startMenu.search.publicFolders") != 0;
    c.searchFlyout = Wh_GetIntSetting(L"startMenu.search.flyout") != 0;
    c.highlightNew = Wh_GetIntSetting(L"startMenu.general.highlightNew") != 0;
    c.autoCascade = Wh_GetIntSetting(L"startMenu.general.autoCascade") != 0;
    c.userPicture = Wh_GetIntSetting(L"startMenu.rightSide.userPicture") != 0;
    c.glyphIcons = Wh_GetIntSetting(L"startMenu.rightSide.glyphIcons") != 0;
    c.multiColumn = Wh_GetIntSetting(L"startMenu.rightSide.multiColumn") != 0;
    c.largeFlyoutIcons = Wh_GetIntSetting(L"startMenu.rightSide.largeFlyoutIcons") != 0;
    std::wstring power = TextSetting(L"startMenu.powerButton");
    c.powerButton = power == L"switchUser" ? kPwSwitch : power == L"signOut" ? kPwSignOut : power == L"lock" ? kPwLock
                  : power == L"restart" ? kPwRestart : power == L"sleep" ? kPwSleep
                  : power == L"hibernate" ? kPwHibernate : kPwShutdown;
    c.winKey = Wh_GetIntSetting(L"startMenu.winKey") != 0;
    c.onPress = Wh_GetIntSetting(L"startMenu.onPress") != 0;
    c.caps = Wh_GetIntSetting(L"startMenu.capitals") != 0;
    c.colors = TextSetting(L"startMenu.menuColors");
    // System Text Color's own settings (through variables, so the build
    // doesn't file them under this part).
    constexpr PCWSTR kTextColorOn = L"textColor.enabled", kTextColor = L"textColor.textColor";
    if (Wh_GetIntSetting(kTextColorOn)) {
        PCWSTR value = Wh_GetStringSetting(kTextColor);
        COLORREF color;
        if (value && ParseHexColor(value, &color)) {
            c.textColorSet = true;
            c.textColor = RefRgb(color);
        }
        Wh_FreeStringSetting(value);
    }
    // Your own text colours, over the style's and System Text Color's; pointed-at text follows yours
    // when only that's set.
    {
        COLORREF color;
        PCWSTR own = Wh_GetStringSetting(L"startMenu.textColor");
        bool ownText = own && *own && ParseHexColor(own, &color);
        Wh_FreeStringSetting(own);
        if (ownText) {
            c.textColorSet = true;
            c.textColor = RefRgb(color);
        }
        PCWSTR hot = Wh_GetStringSetting(L"startMenu.hotTextColor");
        if (hot && *hot && ParseHexColor(hot, &color)) {
            c.hotTextSet = true;
            c.hotText = RefRgb(color);
        } else if (ownText) {
            c.hotTextSet = true;
            c.hotText = c.textColor;
        }
        Wh_FreeStringSetting(hot);
        PCWSTR icon = Wh_GetStringSetting(L"startMenu.searchIcon");
        c.searchIcon = icon ? icon : L"";
        Wh_FreeStringSetting(icon);
    }
    c.phone = Wh_GetIntSetting(L"startMenu.phone") != 0;
    c.phoneNotes = std::clamp(Wh_GetIntSetting(L"startMenu.phoneNotifications"), 0, 8);
    {
        auto text = [](PCWSTR value) {
            std::wstring s = value ? value : L"";
            Wh_FreeStringSetting(value);
            return s;
        };
        c.phoneIcon = text(Wh_GetStringSetting(L"startMenu.phoneIcon"));
        c.phoneActionIcons[0] = text(Wh_GetStringSetting(L"startMenu.phoneMessagesIcon"));
        c.phoneActionIcons[1] = text(Wh_GetStringSetting(L"startMenu.phoneNotificationsIcon"));
        c.phoneActionIcons[2] = text(Wh_GetStringSetting(L"startMenu.phoneSendIcon"));
        c.phoneActionIcons[3] = text(Wh_GetStringSetting(L"startMenu.phoneSettingsIcon"));
        c.phoneAppIcons = Wh_GetIntSetting(L"startMenu.phoneAppIcons") != 0;
        c.phoneTray = Wh_GetIntSetting(L"startMenu.phoneTray") != 0;
#ifdef TD_PREFIX
        c.phoneTexts = std::clamp(Wh_GetIntSetting(L"startMenu.phoneMessages"), 0, 8);
        // A call's sound devices, for Tourne Calls (desktop/calls), which reads them at each call's sound and on
        // command 9 (moved at once during a call). "default" or empty: Windows' default for calls.
        for (auto [setting, value] : {std::pair{L"startMenu.phoneCallOutput", L"Output"}, std::pair{L"startMenu.phoneCallMic", L"Mic"}}) {
            std::wstring id = text(Wh_GetStringSetting(setting));
            if (id == L"default") id.clear();
            RegSetKeyValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Calls", value, REG_SZ, id.c_str(), (DWORD)((id.size() + 1) * sizeof(WCHAR)));
        }
        if (HWND calls = FindWindowExW(HWND_MESSAGE, nullptr, L"TourneCalls_Window", nullptr))
            PostMessageW(calls, RegisterWindowMessageW(L"TourneCallsCommand"), 9, 0);
#else
        c.phoneTexts = 0;  // (only Tourne Desktop holds the phone's texts)
#endif
    }
    ForgetPhoneIcons();  // (the colours may have changed too)
    LoadPhoneState();
    LoadStartSize();
    // The apps' folder's icon: yours, else StartAllBack's (from before it was uninstalled); and every
    // other All Programs folder's (our own addition).
    auto own = [](PCWSTR name) {
        PCWSTR value = Wh_GetStringSetting(name);
        std::wstring s = value ? value : L"";
        Wh_FreeStringSetting(value);
        return s;
    };
    c.appsFolderIcon = own(L"startMenu.allPrograms.appsFolderIcon");
    c.folderIcon = own(L"startMenu.allPrograms.folderIcon");
    WCHAR icon[MAX_PATH * 2] = L"";
    DWORD iconBytes = sizeof(icon);
    if (c.appsFolderIcon.empty() && RegGetValueW(HKEY_CURRENT_USER, L"Software\\StartIsBack", L"AppsFolderIcon", RRF_RT_REG_SZ,
                                                 nullptr, icon, &iconBytes) == ERROR_SUCCESS)
        c.appsFolderIcon = icon;

    // The right side, in StartAllBack's order: custom folders, your folders,
    // the computer's, then the settings and tools.
    auto take = [](PCWSTR value) {
        std::wstring s = value ? value : L"";
        Wh_FreeStringSetting(value);
        return s;
    };
    bool ownFolders = false;
    for (int i = 0; i < 32; i++) {
        StartPlaceSetting p{L"custom"};
        p.path = take(Wh_GetStringSetting(L"startMenu.rightSide.customFolders[%d].path", i));
        p.kind = take(Wh_GetStringSetting(L"startMenu.rightSide.customFolders[%d].show", i));
        p.name = take(Wh_GetStringSetting(L"startMenu.rightSide.customFolders[%d].name", i));
        if (p.path.empty()) {
            if (i > 0) break;
            continue;
        }
        ownFolders = true;
        if (p.kind != L"dontDisplay") c.places.push_back(p);
    }
    if (!ownFolders)  // StartAllBack's own: 0 not shown, 1 a link, 2 a menu
        for (auto const& [path, mode] : ReadSabCustomFolders())
            if (mode == 1 || mode == 2) c.places.push_back({L"custom", mode == 2 ? L"menu" : L"link", path, L""});
    // Each setting read where its name is written: the build prefixes only
    // literal setting names.
    auto item = [&](PCWSTR name, std::wstring const& show) {
        if (!show.empty() && show != L"dontDisplay") c.places.push_back({name, show, L"", L""});
    };
    item(L"user", TextSetting(L"startMenu.rightSide.user"));
    item(L"documents", TextSetting(L"startMenu.rightSide.documents"));
    item(L"pictures", TextSetting(L"startMenu.rightSide.pictures"));
    item(L"music", TextSetting(L"startMenu.rightSide.music"));
    item(L"videos", TextSetting(L"startMenu.rightSide.videos"));
    item(L"downloads", TextSetting(L"startMenu.rightSide.downloads"));
    item(L"oneDrive", TextSetting(L"startMenu.rightSide.oneDrive"));
    item(L"favorites", TextSetting(L"startMenu.rightSide.favorites"));
    item(L"recentItems", TextSetting(L"startMenu.rightSide.recentItems"));
    item(L"network", TextSetting(L"startMenu.rightSide.network"));
    item(L"networkConnections", TextSetting(L"startMenu.rightSide.networkConnections"));
    item(L"computer", TextSetting(L"startMenu.rightSide.computer"));
    c.places.push_back({L"separator", L"link", L"", L""});
    item(L"controlPanel", TextSetting(L"startMenu.rightSide.controlPanel"));
    item(L"settings", TextSetting(L"startMenu.rightSide.settings"));
    item(L"windowsTools", TextSetting(L"startMenu.rightSide.windowsTools"));
    item(L"devicesAndPrinters", TextSetting(L"startMenu.rightSide.devicesAndPrinters"));
    item(L"defaultPrograms", TextSetting(L"startMenu.rightSide.defaultPrograms"));
    item(L"terminal", TextSetting(L"startMenu.rightSide.terminal"));
    item(L"commandPrompt", TextSetting(L"startMenu.rightSide.commandPrompt"));
    item(L"run", TextSetting(L"startMenu.rightSide.run"));

    // What the right-click menu changed is kept until the list is set again.
    std::wstring signature;
    for (auto const& p : c.places) signature += p.item + L"|" + p.kind + L"|" + p.path + L"|" + p.name + L"\n";
    WCHAR old[4096] = L"";
    DWORD size2 = sizeof(old);
    RegGetValueW(HKEY_CURRENT_USER, kStartKey, L"PlacesSignature", RRF_RT_REG_SZ, nullptr, old, &size2);
    if (signature != old) {
        RegDeleteTreeW(HKEY_CURRENT_USER, (std::wstring(kStartKey) + L"\\Places").c_str());
        RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, L"PlacesSignature", REG_SZ, signature.c_str(),
                        (DWORD)((signature.size() + 1) * sizeof(WCHAR)));
    }
    g_startConfig = c;
}

////////////////////////////////////////////////////////////////////////////////
// The look: the style's Start menu parts, or the colour scheme

struct StartLook {
    HWND host = nullptr;
    HTHEME panel = nullptr;   // StartPanelPriv
    HTHEME scroll = nullptr;  // the scroll bar: the style's, else Windows' visual style's
    HTHEME menu = nullptr;    // Windows' visual style's menus
    HTHEME searchBox = nullptr;  // and its search box, as StartAllBack's is drawn
    HBRUSH searchBrush = nullptr;  // the search box's ground, under the typing
    int searchBrushState = 0;
    uint32_t button = 0, buttonHot = 0, buttonHotEdge = 0, buttonPressed = 0;  // Shut down, from part 23
    HFONT font = nullptr;     // the style's Start menu font
    HFONT menuFont = nullptr; // the system's menu font, for classic menus
    uint32_t text = 0, hotText = 0, accent = 0, ground = 0, box = 0, dim = 0, hover = 0, edge = 0;
    // The panes' frame: their see-through margins, and the border inside them.
    int frameLeft = 0, frameTop = 0, frameBorder = 0;
    uint32_t border = 0;
};
StartLook g_sl;
ThemeFileData g_startFile{};  // the Start menu's own style, when it isn't the taskbar's
bool g_startFileLoaded = false;

uint32_t StartMix(uint32_t a, uint32_t b, double t) {
    auto channel = [&](int shift) {
        int x = (a >> shift) & 0xFF, y = (b >> shift) & 0xFF;
        return (uint32_t)((int)(x + (y - x) * t + 0.5) & 0xFF) << shift;
    };
    return channel(0) | channel(8) | channel(16);
}

double StartLuma(uint32_t c) {
    return (0.299 * ((c >> 16) & 0xFF) + 0.587 * ((c >> 8) & 0xFF) + 0.114 * (c & 0xFF)) / 255.0;
}

// A panel part's colour at a point, drawn at 24 x 24.
uint32_t SpPixel(int part, int state, int x, int y, uint32_t fallback) {
    if (!g_sl.panel || !IsThemePartDefined(g_sl.panel, part, 0)) return fallback;
    constexpr int kSize = 24;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), kSize, -kSize, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    uint32_t result = fallback;
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        memset(bits, 0, kSize * kSize * 4);
        RECT r{0, 0, kSize, kSize};
        if (SUCCEEDED(DrawThemeBackground(g_sl.panel, dc, part, state, &r, nullptr))) {
            GdiFlush();
            uint32_t pixel = ((uint32_t*)bits)[std::clamp(y, 0, kSize - 1) * kSize + std::clamp(x, 0, kSize - 1)];
            if (pixel >> 24) result = pixel & 0xFFFFFF;
        }
        SelectObject(dc, old);
        DeleteObject(bitmap);
    }
    DeleteDC(dc);
    return result;
}

constexpr uint32_t kStartKeyColor = 0xFF00FF;  // where nothing is drawn

// Draws into a fresh 32-bit bitmap filled with the key colour; fn draws.
template <class F>
std::vector<uint32_t> StartKeyedRender(int w, int h, F&& fn) {
    std::vector<uint32_t> out;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap) {
        HGDIOBJ old = SelectObject(dc, bitmap);
        for (int i = 0; i < w * h; i++) ((uint32_t*)bits)[i] = kStartKeyColor;
        fn(dc);
        GdiFlush();
        out.assign((uint32_t*)bits, (uint32_t*)bits + w * h);
        for (auto& pixel : out) pixel &= 0xFFFFFF;
        SelectObject(dc, old);
        DeleteObject(bitmap);
    }
    DeleteDC(dc);
    return out;
}

// The left pane's see-through margins and the border inside them, as the
// window's outline and everything's place follow them.
void MeasureStartFrame() {
    g_sl.frameLeft = g_sl.frameTop = g_sl.frameBorder = 0;
    if (!g_sl.panel) return;
    constexpr int kSize = 100;
    auto pixels = StartKeyedRender(kSize, kSize, [](HDC dc) {
        RECT r{0, 0, kSize, kSize};
        DrawThemeBackground(g_sl.panel, dc, kSpLeft, 1, &r, nullptr);
    });
    if (pixels.empty()) return;
    auto at = [&](int x, int y) { return pixels[y * kSize + x]; };
    int x = 0, y = 0;
    while (x < kSize / 2 && at(x, kSize / 2) == kStartKeyColor) x++;
    while (y < kSize / 2 && at(kSize / 2, y) == kStartKeyColor) y++;
    int b = 0;
    while (x + b < kSize / 2 && at(x + b, kSize / 2) == at(x, kSize / 2)) b++;
    g_sl.frameLeft = x;
    g_sl.frameTop = y;
    g_sl.frameBorder = std::min(b, 6);
    g_sl.border = at(x, kSize / 2);
}

void CloseStartLook() {
    if (g_sl.panel) CloseThemeData(g_sl.panel);
    if (g_sl.scroll) CloseThemeData(g_sl.scroll);
    if (g_sl.menu) CloseThemeData(g_sl.menu);
    if (g_sl.searchBox) CloseThemeData(g_sl.searchBox);
    if (g_sl.searchBrush) DeleteObject(g_sl.searchBrush);
    if (g_sl.font) DeleteObject(g_sl.font);
    if (g_sl.menuFont) DeleteObject(g_sl.menuFont);
    if (g_sl.host) DestroyWindow(g_sl.host);
    g_sl = {};
    if (g_startFileLoaded) CloseThemeFile(&g_startFile);
    g_startFileLoaded = false;
}

// With the taskbar's theme loaded (and closed before it's replaced: the
// handles live in its file).
void LoadStartLook() {
    CloseStartLook();
    auto const& p = g_settings.palette;
    g_sl.text = g_sl.hotText = p.text;
    g_sl.accent = p.accent;
    g_sl.ground = p.ground;
    g_sl.box = p.raised;
    g_sl.hover = p.hover;
    g_sl.edge = p.edge;
    auto open = ThemeFileOpener();
    bool dark = StartLuma(p.ground) < 0.5;
    // The taskbar's file when it's the same style, else its own.
    ThemeFileData* file = nullptr;
    std::wstring const& path = g_startConfig.stylePath;
    if (!path.empty() && open) {
        if (g_themed && !_wcsicmp(path.c_str(), g_settings.style.c_str())) {
            file = &g_theme.file;
        } else if (LoadThemeFile(path, &g_startFile)) {
            g_startFileLoaded = true;
            file = &g_startFile;
        }
    }
    if (file) {
        g_sl.host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
        // The dark variant for a dark scheme, as StartAllBack picks it.
        for (PCWSTR app : {dark ? L"StartPanelDark" : L"StartPanelComposited", L"StartPanelComposited", L""}) {
            SetWindowTheme(g_sl.host, app, nullptr);
            HTHEME theme = open(file, g_sl.host, L"StartPanelPriv", 0);
            if (theme && IsThemePartDefined(theme, kSpLeft, 0)) {
                g_sl.panel = theme;
                break;
            }
            if (theme) CloseThemeData(theme);
        }
        for (PCWSTR app : {dark ? L"DarkMode_Explorer" : L"", L""}) {
            SetWindowTheme(g_sl.host, app, nullptr);
            HTHEME theme = open(file, g_sl.host, L"ScrollBar", 0);
            if (theme && IsThemePartDefined(theme, SBP_THUMBBTNVERT, 0)) {
                g_sl.scroll = theme;
                break;
            }
            if (theme) CloseThemeData(theme);
        }
    }
    // StartAllBack's styles only style its own parts: the scroll bar and
    // menus come from Windows' visual style, as StartAllBack's do.
    if (!g_sl.host) g_sl.host = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    for (PCWSTR app : {dark ? L"DarkMode_Explorer" : L"Explorer", L""}) {
        if (g_sl.scroll) break;
        SetWindowTheme(g_sl.host, app, nullptr);
        HTHEME theme = OpenThemeData(g_sl.host, L"ScrollBar");
        if (theme && IsThemePartDefined(theme, SBP_THUMBBTNVERT, 0)) g_sl.scroll = theme;
        else if (theme) CloseThemeData(theme);
    }
    SetWindowTheme(g_sl.host, L"", nullptr);
    if (HTHEME theme = OpenThemeData(g_sl.host, L"Menu")) {
        if (IsThemePartDefined(theme, MENU_POPUPITEM, 0)) g_sl.menu = theme;
        else CloseThemeData(theme);
    }
    if (HTHEME theme = OpenThemeData(g_sl.host, L"SearchBox")) {
        if (IsThemePartDefined(theme, 1 /* SBP_SEARCHBOX */, 0)) g_sl.searchBox = theme;
        else CloseThemeData(theme);
    }
    MeasureStartFrame();
    // Shut down's colours, from the style's split button (part 23), sampled
    // clear of the arrow it carries.
    g_sl.button = SpPixel(kSpShutdownButton, 1, 4, 12, p.raised);
    g_sl.buttonHot = SpPixel(kSpShutdownButton, 2, 4, 12, p.hover);
    g_sl.buttonHotEdge = SpPixel(kSpShutdownButton, 2, 1, 12, p.edge);
    g_sl.buttonPressed = SpPixel(kSpShutdownButton, 3, 4, 12, p.ground);
    if (g_sl.panel) {
        COLORREF c;
        if (SUCCEEDED(GetThemeColor(g_sl.panel, kSpList, 0, TMT_TEXTCOLOR, &c)) ||
            SUCCEEDED(GetThemeColor(g_sl.panel, kSpLeft, 0, TMT_TEXTCOLOR, &c)))
            g_sl.text = g_sl.hotText = RefRgb(c);
        if (SUCCEEDED(GetThemeColor(g_sl.panel, kSpItem, 2, TMT_TEXTCOLOR, &c))) g_sl.hotText = RefRgb(c);
        if (SUCCEEDED(GetThemeColor(g_sl.panel, kSpAccent, 0, TMT_TEXTCOLOR, &c)) ||
            SUCCEEDED(GetThemeColor(g_sl.panel, kSpAccent, 1, TMT_TEXTCOLOR, &c)))
            g_sl.accent = RefRgb(c);
        g_sl.ground = SpPixel(kSpBottom, 1, 12, 12, p.ground);
        g_sl.box = SpPixel(kSpSearch, 1, 12, 12, g_sl.ground);
        g_sl.hover = SpPixel(kSpItem, 2, 12, 12, p.hover);
        g_sl.edge = SpPixel(kSpSearch, 1, 0, 12, StartMix(g_sl.ground, g_sl.text, 0.35));
        // (The style's own font part is not used: StartAllBack draws the lists in the
        // icon title font whatever the style says; a side-by-side capture showed it.)
    }
    // Otherwise the icon title font, as Windows 7's Start menu and
    // StartAllBack's draw their lists, rather than the taskbar's.
    if (!g_sl.font) {
        LOGFONTW lf;
        if (SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, 96 * g_scale))
            g_sl.font = CreateFontIndirectW(&lf);
    }    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, 96 * g_scale))
        g_sl.menuFont = CreateFontIndirectW(&ncm.lfMenuFont);

    // System Text Color recolours Explorer's text, StartAllBack's among it.
    if (g_startConfig.textColorSet) g_sl.text = g_startConfig.textColor;
    if (g_startConfig.hotTextSet) g_sl.hotText = g_startConfig.hotText;
    g_sl.dim = StartMix(g_sl.box, g_sl.text, 0.55);
}

HICON StartIconSpec(std::wstring spec, int size);

// Your search icon (startMenu.searchIcon) at a size, kept for the next paint.
HICON StartSearchIcon(int size) {
    static HICON icon;
    static int iconSize;
    static std::wstring iconSpec;
    if (g_startConfig.searchIcon.empty()) return nullptr;
    if (!icon || iconSize != size || iconSpec != g_startConfig.searchIcon) {
        if (icon) DestroyIcon(icon);
        icon = StartIconSpec(g_startConfig.searchIcon, size);
        iconSize = size;
        iconSpec = g_startConfig.searchIcon;
    }
    return icon;
}

HFONT StartFont() {
    return g_sl.font ? g_sl.font : g_font;
}

// The flyouts' and menus' look, as set: by default made from the Start
// menu's own colours (so from the theme), else the style's menus, the
// colour scheme, or the bar's own menus (Tourne Menus' look: its frame,
// gradient and glow).
MenuLook StartMenuLook() {
    if (g_startConfig.colors == L"tourne") {
        MenuLook look = BarMenuLook();
        if (MenusLikeWindows()) LookLikeWindowsMenus(look);
        return look;
    }
    auto const& p = g_settings.palette;
    MenuLook look{p.ground, p.line, p.edge, p.text, p.accent, p.dim, p.hover, g_tipFont, nullptr, g_scale,
                  !g_settings.menuIcons};
    std::wstring const& colors = g_startConfig.colors;
    if (colors == L"visualStyle") {
        // Windows' classic menus, as StartAllBack shows them: the visual
        // style's parts, in the system's menu font, laid out as Windows lays
        // them. Drawn here rather than by Windows so System Text Color and
        // Menu No Icons still reach them.
        look.theme = g_themed && g_theme.menu ? g_theme.menu : g_sl.menu;
        look.classic = true;
        if (g_sl.menuFont) look.font = g_sl.menuFont;
        if (g_startConfig.textColorSet) {
            look.text = g_startConfig.textColor;
            look.ownText = true;
        }
    } else if (colors != L"scheme") {
        if (g_sl.panel) {
            look.ground = g_sl.ground;
            look.text = g_sl.text;
            look.accent = g_sl.accent;
            look.hover = g_sl.hover;
            look.edge = g_sl.edge;
            look.line = StartMix(g_sl.ground, g_sl.text, 0.2);
            look.dim = StartMix(g_sl.ground, g_sl.text, 0.55);
            if (g_sl.font) look.font = g_sl.font;
            if (g_settings.sabBackdrop && g_themed) {  // StartAllBack's menu look: framed, in capitals
                look.frame = true;
                look.caps = g_startConfig.caps;
            }
        } else if (g_themed && g_theme.menu) {
            look.theme = g_theme.menu;
        }
    }
    return look;
}

////////////////////////////////////////////////////////////////////////////////
// What's in it

struct StartApp {
    std::wstring name, parsing;
};
std::vector<StartApp> g_startApps;                         // Start's own list (shell:AppsFolder)
std::unordered_map<std::wstring, size_t> g_startAppIds;    // lower-case app id: its place in the list
std::vector<StartApp> g_startControlPanel;
bool g_startControlPanelRead = false;
std::unordered_map<std::wstring, HICON> g_startIcons;

template <class F>
void ForEachShellChild(std::wstring const& folder, F&& fn) {
    IShellItem* parent = nullptr;
    if (FAILED(SHCreateItemFromParsingName(folder.c_str(), nullptr, IID_PPV_ARGS(&parent)))) return;
    IEnumShellItems* items = nullptr;
    if (SUCCEEDED(parent->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&items)))) {
        IShellItem* item = nullptr;
        while (items->Next(1, &item, nullptr) == S_OK) {
            bool more = fn(item);
            item->Release();
            if (!more) break;
        }
        items->Release();
    }
    parent->Release();
}

std::wstring ItemName(IShellItem* item, SIGDN form) {
    LPWSTR name = nullptr;
    std::wstring s;
    if (SUCCEEDED(item->GetDisplayName(form, &name)) && name) s = name;
    CoTaskMemFree(name);
    return s;
}

void ReadStartApps() {
    g_startApps.clear();
    g_startAppIds.clear();
    ForEachShellChild(L"shell:AppsFolder", [](IShellItem* item) {
        std::wstring name = ItemName(item, SIGDN_NORMALDISPLAY), id = ItemName(item, SIGDN_PARENTRELATIVEPARSING);
        if (!name.empty() && !id.empty()) {
            g_startAppIds[Lower(id)] = g_startApps.size();
            g_startApps.push_back({name, L"shell:AppsFolder\\" + id});
        }
        return true;
    });
}

// What opening the menu used to redo every time is kept until something changes it: the
// places (until the settings change), and the home list's rows (until a pin or the list of
// programs changes; the frequent programs are read again in the idle time after it closes).
bool g_homeValid = false;
bool g_placesValid = false;

// The list of programs, read on a thread of its own and handed over whole: the shell takes a
// while to enumerate its apps folder, and that used to be paid on the menu opening.
struct AppsRead {
    std::vector<StartApp> apps;
    std::unordered_map<std::wstring, size_t> ids;
};
AppsRead* g_appsRead = nullptr;  // finished, waiting to be taken
LONG g_appsThreads = 0;

DWORD WINAPI ReadAppsThread(LPVOID) {
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        auto* read = new AppsRead;
        ForEachShellChild(L"shell:AppsFolder", [read](IShellItem* item) {
            std::wstring name = ItemName(item, SIGDN_NORMALDISPLAY), id = ItemName(item, SIGDN_PARENTRELATIVEPARSING);
            if (!name.empty() && !id.empty()) {
                read->ids[Lower(id)] = read->apps.size();
                read->apps.push_back({name, L"shell:AppsFolder\\" + id});
            }
            return true;
        });
        delete (AppsRead*)InterlockedExchangePointer((PVOID*)&g_appsRead, read);
        CoUninitialize();
    }
    BootMark(L"start menu: programs read");
    InterlockedDecrement(&g_appsThreads);
    return 0;
}

bool StartAppsReading() { return g_appsThreads > 0; }
bool g_appsJustRead = false;  // the settings' own read is the idle time's

void StartRefreshApps() {
    if (InterlockedCompareExchange(&g_appsThreads, 1, 0) != 0) return;
    HANDLE thread = CreateThread(nullptr, 0, ReadAppsThread, nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
    else InterlockedDecrement(&g_appsThreads);
}

// Takes what the thread read, if it's done: the names in the home list may change (only if the
// list did, so a read that lands just before an open doesn't make it build the list again).
void StartTakeApps() {
    auto* read = (AppsRead*)InterlockedExchangePointer((PVOID*)&g_appsRead, nullptr);
    if (!read) return;
    bool same = read->apps.size() == g_startApps.size() &&
                std::equal(read->apps.begin(), read->apps.end(), g_startApps.begin(),
                           [](StartApp const& a, StartApp const& b) { return a.name == b.name && a.parsing == b.parsing; });
    g_startApps = std::move(read->apps);
    g_startAppIds = std::move(read->ids);
    if (!same) g_homeValid = false;
    delete read;
}

void FreeStartApps() {
    for (int wait = 0; g_appsThreads > 0 && wait < 100; wait++) Sleep(50);
    delete (AppsRead*)InterlockedExchangePointer((PVOID*)&g_appsRead, nullptr);
}

void ReadStartControlPanel() {
    if (g_startControlPanelRead) return;
    g_startControlPanelRead = true;
    ForEachShellChild(L"shell:::{21EC2020-3AEA-1069-A2DD-08002B30309D}", [](IShellItem* item) {
        std::wstring name = ItemName(item, SIGDN_NORMALDISPLAY), parsing = ItemName(item, SIGDN_DESKTOPABSOLUTEPARSING);
        if (!name.empty() && !parsing.empty()) g_startControlPanel.push_back({name, parsing});
        return true;
    });
}

// Something's icon from the shell's system image lists, the icon's own frame
// at 16, 32 or 48 px as Explorer and StartAllBack draw them (sharp, where a
// rendered image is scaled from another size), or null at other sizes.
HICON SystemListIcon(std::wstring const& parsing, int size) {
    int list = size == 16 ? SHIL_SMALL : size == 32 ? SHIL_LARGE : size == 48 ? SHIL_EXTRALARGE : -1;
    if (list < 0) return nullptr;
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (FAILED(SHParseDisplayName(parsing.c_str(), nullptr, &pidl, 0, nullptr)) || !pidl) return nullptr;
    SHFILEINFOW info{};
    DWORD_PTR ok = SHGetFileInfoW((PCWSTR)pidl, 0, &info, sizeof(info), SHGFI_PIDL | SHGFI_SYSICONINDEX);
    CoTaskMemFree(pidl);
    if (!ok) return nullptr;
    // IID_IImageList: the list is also usable as an HIMAGELIST.
    static constexpr GUID kImageList = {0x46EB5926, 0x582E, 0x4017, {0x9F, 0xDF, 0xE8, 0x99, 0x8D, 0xAA, 0x09, 0x50}};
    IUnknown* images = nullptr;
    if (FAILED(SHGetImageList(list, kImageList, (void**)&images)) || !images) return nullptr;
    HICON icon = ImageList_GetIcon((HIMAGELIST)images, info.iIcon, ILD_TRANSPARENT);
    images->Release();
    return icon;
}

// The icon itself, from the shell; safe on any thread with COM started.
// An icon given as "file" or "file,index".
HICON StartIconSpec(std::wstring spec, int size) {
    int index = 0;
    size_t comma = spec.rfind(L',');
    if (comma != std::wstring::npos && comma + 1 < spec.size() && (iswdigit(spec[comma + 1]) || spec[comma + 1] == L'-')) {
        index = _wtoi(spec.c_str() + comma + 1);
        spec.resize(comma);
    }
    return ExtractAt(Expand(spec.c_str()), index, size);
}

// The icon of the taskbar's own pin for the same program, so a pinned program's Start icon is its taskbar
// icon with no row to set. Read from the pinned shortcuts on the icon thread; a pin with no icon of its
// own gives nothing (Windows' icon).
HICON StartPinnedIcon(std::wstring const& target, int size) {
    if (target.empty()) return nullptr;
    std::wstring folder = Expand(L"%APPDATA%\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar\\");
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"*.lnk").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return nullptr;
    HICON icon = nullptr;
    do {
        Shortcut pin;
        if (!ParseShortcut(folder + fd.cFileName, true, &pin) || pin.target != target || pin.iconPath.empty()) continue;
        icon = ExtractAt(pin.iconPath, pin.iconIndex, size);
        break;
    } while (FindNextFileW(find, &fd));
    FindClose(find);
    return icon;
}

// A Start menu item's icon from the taskbar's Button icons: your rows, all of them (not only "Use everywhere"),
// matched by app id or shortcut target as the taskbar matches a button; else the taskbar's pin for the same
// program. Runs on the icon thread, so it reads no shared state but the settings and the .lnk files. System
// Icons' designs (tourne:) are left to the Windows icon: they're drawn on the UI thread.
HICON StartButtonIcon(std::wstring const& parsing, int size) {
    Shortcut s;
    std::wstring id;
    if (parsing.starts_with(L"shell:AppsFolder\\")) {
        id = Lower(parsing.substr(wcslen(L"shell:AppsFolder\\")));
    } else if (parsing.size() > 4 && _wcsicmp(parsing.c_str() + parsing.size() - 4, L".lnk") == 0 && ParseShortcut(parsing, false, &s)) {
        id = s.appId;
    } else {
        return nullptr;
    }
    for (auto const& [match, file] : g_settings.buttonIcons) {
        if (!ProgramIconMatches(match, id, s.target)) continue;
        if (IsTourneDesign(file)) return nullptr;
        std::wstring source = file;
        int index = 0;
        size_t comma = source.rfind(L',');
        if (comma != std::wstring::npos && comma + 1 < source.size() && iswdigit(source.back())) {
            index = _wtoi(source.c_str() + comma + 1);
            source.resize(comma);
        }
        return ExtractAt(source, index, size);
    }
    return StartPinnedIcon(s.target, size);
}

HICON MakeStartIcon(std::wstring const& parsing, int size, std::wstring const& custom, std::wstring const& folder) {
    HICON icon = StartButtonIcon(parsing, size);
    // The apps' folder: your icon for it, or StartAllBack's.
    if (!icon && parsing == L"shell:AppsFolder" && !custom.empty()) icon = StartIconSpec(custom, size);
    // All Programs' other folders: your icon for them.
    else if (!icon && !folder.empty() && PathIsDirectoryW(parsing.c_str())) icon = StartIconSpec(folder, size);
    if (!icon) icon = SystemListIcon(parsing, size);
    if (!icon) icon = ShellIcon(parsing, size);
    return icon;
}

std::wstring StartIconKey(std::wstring const& parsing, int size) {
    return std::to_wstring(size) + L"|" + parsing;
}

void StartTakeIcons();

HICON StartIcon(std::wstring const& parsing, int size) {
    if (parsing.empty()) return nullptr;
    StartTakeIcons();
    std::wstring key = StartIconKey(parsing, size);
    auto it = g_startIcons.find(key);
    if (it != g_startIcons.end()) return it->second;
    HICON icon = MakeStartIcon(parsing, size, g_startConfig.appsFolderIcon, g_startConfig.folderIcon);
    g_startIcons[key] = icon;
    return icon;
}

// The icons the menu's home list and places will draw, fetched on a thread of their own while the
// menu is closed, so its first open (after starting, or after a change) doesn't wait on the shell.
// All Programs' top level too: its entries' names (the shell's display names, over a millisecond
// each) and icons, so pressing it doesn't wait on the shell.
struct IconsWarm {
    std::wstring custom;
    std::vector<std::pair<std::wstring, int>> wanted;  // parsing name, size
    std::vector<std::pair<std::wstring, HICON>> made;  // key, icon
    std::vector<std::wstring> progDirs;                // All Programs' folders
    int progIcon = 0;                                  // its icons' size
    std::unordered_set<std::wstring> haveNames, haveIcons;
    std::vector<std::pair<std::wstring, std::wstring>> named;  // path, display name
    std::wstring folder;  // All Programs' folders' icon, as set
};
IconsWarm* g_iconsWarm = nullptr;  // finished, waiting to be taken
LONG g_iconThreads = 0;
std::unordered_map<std::wstring, std::wstring> g_progNames;  // path -> display name, kept

std::wstring FileDisplayName(std::wstring const& path);

// The entries of one of All Programs' folders that it lists.
template <typename F>
void ForEachProgEntry(std::wstring const& dir, F&& f) {
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..") || (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
            !_wcsicmp(fd.cFileName, L"desktop.ini"))
            continue;
        f(fd);
    } while (FindNextFileW(find, &fd));
    FindClose(find);
}

DWORD WINAPI WarmIconsThread(LPVOID param) {
    auto* job = (IconsWarm*)param;
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        for (auto const& dir : job->progDirs) {
            ForEachProgEntry(dir, [&](WIN32_FIND_DATAW const& fd) {
                std::wstring path = dir + L"\\" + fd.cFileName;
                if (!job->haveNames.count(path)) job->named.push_back({path, FileDisplayName(path)});
                if (!job->haveIcons.count(StartIconKey(path, job->progIcon))) job->wanted.push_back({path, job->progIcon});
            });
        }
        for (auto const& [parsing, size] : job->wanted)
            job->made.push_back({StartIconKey(parsing, size), MakeStartIcon(parsing, size, job->custom, job->folder)});
        CoUninitialize();
    }
    delete (IconsWarm*)InterlockedExchangePointer((PVOID*)&g_iconsWarm, job);
    BootMark(L"start menu: icons fetched (ready)");
    WriteBootLog();
    InterlockedDecrement(&g_iconThreads);
    return 0;
}

// Takes what the thread made, if it's done; an icon the menu made itself meanwhile is kept.
void StartTakeIcons() {
    auto* job = (IconsWarm*)InterlockedExchangePointer((PVOID*)&g_iconsWarm, nullptr);
    if (!job) return;
    for (auto& [key, icon] : job->made) {
        if (g_startIcons.count(key)) {
            if (icon) DestroyIcon(icon);
        } else {
            g_startIcons[key] = icon;
        }
    }
    for (auto& [path, name] : job->named) g_progNames.try_emplace(std::move(path), std::move(name));
    delete job;
}

std::wstring KnownFolderPath(REFKNOWNFOLDERID id);
int AllProgramsIconPx();

// progs: All Programs' top level as well.
void WarmStartIcons(std::vector<std::pair<std::wstring, int>> wanted, bool progs = false) {
    std::erase_if(wanted, [](auto const& w) { return w.first.empty() || g_startIcons.count(StartIconKey(w.first, w.second)); });
    if ((wanted.empty() && !progs) || InterlockedCompareExchange(&g_iconThreads, 1, 0) != 0) return;
    auto* job = new IconsWarm{g_startConfig.appsFolderIcon, std::move(wanted), {}};
    job->folder = g_startConfig.folderIcon;
    if (progs) {
        job->progDirs = {KnownFolderPath(FOLDERID_Programs), KnownFolderPath(FOLDERID_CommonPrograms)};
        job->progIcon = AllProgramsIconPx();
        for (auto const& [path, name] : g_progNames) job->haveNames.insert(path);
        for (auto const& [key, icon] : g_startIcons) job->haveIcons.insert(key);
    }
    HANDLE thread = CreateThread(nullptr, 0, WarmIconsThread, job, 0, nullptr);
    if (thread) {
        SetThreadPriority(thread, THREAD_PRIORITY_BELOW_NORMAL);
        CloseHandle(thread);
    } else {
        delete job;
        InterlockedDecrement(&g_iconThreads);
    }
}

void FreeWarmIcons() {
    for (int wait = 0; g_iconThreads > 0 && wait < 100; wait++) Sleep(50);
    if (auto* job = (IconsWarm*)InterlockedExchangePointer((PVOID*)&g_iconsWarm, nullptr)) {
        for (auto& [key, icon] : job->made)
            if (icon) DestroyIcon(icon);
        delete job;
    }
}

void ClearStartIcons() {
    for (auto& [parsing, icon] : g_startIcons)
        if (icon) DestroyIcon(icon);
    g_startIcons.clear();
    // A warm-up finished before the change: made for what was, so it goes too.
    if (auto* job = (IconsWarm*)InterlockedExchangePointer((PVOID*)&g_iconsWarm, nullptr)) {
        for (auto& [key, icon] : job->made)
            if (icon) DestroyIcon(icon);
        delete job;
    }
}

std::wstring KnownFolderPath(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring s;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path)) && path) s = path;
    CoTaskMemFree(path);
    return s;
}

// "{known folder}\rest", as Start and UserAssist name programs, to a path.
std::wstring ExpandKnownFolder(std::wstring const& s) {
    if (s.size() < 38 || s[0] != L'{' || s[37] != L'}') return s;
    GUID id;
    if (FAILED(CLSIDFromString(s.substr(0, 38).c_str(), &id))) return s;
    std::wstring folder = KnownFolderPath(id);
    return folder.empty() ? s : folder + s.substr(38);
}

std::wstring FileDisplayName(std::wstring const& path) {
    SHFILEINFOW info{};
    if (SHGetFileInfoW(path.c_str(), 0, &info, sizeof(info), SHGFI_DISPLAYNAME) && info.szDisplayName[0])
        return info.szDisplayName;
    return path.substr(path.find_last_of(L'\\') + 1);
}

std::wstring ShellDisplayName(std::wstring const& parsing) {
    IShellItem* item = nullptr;
    std::wstring name;
    if (SUCCEEDED(SHCreateItemFromParsingName(parsing.c_str(), nullptr, IID_PPV_ARGS(&item)))) {
        name = ItemName(item, SIGDN_NORMALDISPLAY);
        item->Release();
    }
    return name;
}

// The file behind something in the list, if there is one.
std::wstring StartFilePath(std::wstring const& parsing) {
    std::wstring rest = parsing;
    constexpr std::wstring_view kApps = L"shell:AppsFolder\\";
    if (Lower(parsing).starts_with(Lower(std::wstring(kApps)))) rest = parsing.substr(kApps.size());
    rest = ExpandKnownFolder(rest);
    if (rest.size() > 2 && ((rest[1] == L':' && rest[2] == L'\\') || rest.starts_with(L"\\\\")) &&
        GetFileAttributesW(rest.c_str()) != INVALID_FILE_ATTRIBUTES)
        return rest;
    return {};
}

std::wstring StartName(std::wstring const& parsing) {
    constexpr std::wstring_view kApps = L"shell:AppsFolder\\";
    if (Lower(parsing).starts_with(Lower(std::wstring(kApps)))) {
        auto it = g_startAppIds.find(Lower(parsing.substr(kApps.size())));
        if (it != g_startAppIds.end()) return g_startApps[it->second].name;
    }
    std::wstring name = ShellDisplayName(parsing);
    return name.empty() ? FileDisplayName(parsing) : name;
}

// Opens anything the shell can name, or a program or link it can't.
void StartOpen(std::wstring const& parsing, PCWSTR verb = nullptr) {
    if (parsing.empty()) return;
    if (!verb && ShellLaunchAppsFolder(parsing)) return;  // (an app's entry: works without Explorer too)
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (SUCCEEDED(SHParseDisplayName(parsing.c_str(), nullptr, &pidl, 0, nullptr)) && pidl) {
        SHELLEXECUTEINFOW info{sizeof(info)};
        info.fMask = SEE_MASK_IDLIST | SEE_MASK_FLAG_NO_UI;  // (no error box: tried below another way)
        info.lpIDList = pidl;
        info.lpVerb = verb;
        info.nShow = SW_SHOWNORMAL;
        BOOL ok = ShellExecuteExW(&info);
        CoTaskMemFree(pidl);
        if (ok) return;
    }
    ShellExecuteW(nullptr, verb, parsing.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

// Your pins, and what you've taken off the list, kept as the shell's names.
std::vector<std::wstring> ReadStartList(PCWSTR value) {
    std::vector<std::wstring> list;
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kStartKey, value, RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &bytes) != ERROR_SUCCESS ||
        !bytes)
        return list;
    std::vector<WCHAR> buffer(bytes / sizeof(WCHAR) + 2, 0);
    if (RegGetValueW(HKEY_CURRENT_USER, kStartKey, value, RRF_RT_REG_MULTI_SZ, nullptr, buffer.data(), &bytes) !=
        ERROR_SUCCESS)
        return list;
    for (WCHAR const* s = buffer.data(); *s; s += wcslen(s) + 1) list.push_back(s);
    return list;
}

void WriteStartList(PCWSTR value, std::vector<std::wstring> const& list) {
    std::wstring data;
    for (auto const& s : list) data += s + L'\0';
    data += L'\0';
    RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, value, REG_MULTI_SZ, data.c_str(),
                    (DWORD)(data.size() * sizeof(WCHAR)));
}

// Windows 7's Start menu pins, in their order, which StartAllBack keeps too
// (StartPage2's Favorites: the same layout as the taskbar's).
std::vector<std::wstring> ReadWindowsStartPins() {
    std::vector<std::wstring> pins;
    constexpr WCHAR kStartPage2[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage2";
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kStartPage2, L"Favorites", RRF_RT_REG_BINARY, nullptr, nullptr, &bytes) !=
            ERROR_SUCCESS ||
        !bytes)
        return pins;
    std::vector<BYTE> data(bytes);
    if (RegGetValueW(HKEY_CURRENT_USER, kStartPage2, L"Favorites", RRF_RT_REG_BINARY, nullptr, data.data(), &bytes) !=
        ERROR_SUCCESS)
        return pins;
    size_t at = 1;
    while (at + 4 <= data.size()) {
        DWORD length = 0;
        memcpy(&length, &data[at], 4);
        at += 4;
        if (!length || at + length > data.size()) break;
        std::vector<BYTE> list(data.begin() + at, data.begin() + at + length);
        list.insert(list.end(), 4, 0);
        at += length + 1;
        PWSTR name = nullptr;
        if (SUCCEEDED(SHGetNameFromIDList((PCIDLIST_ABSOLUTE)list.data(), SIGDN_DESKTOPABSOLUTEPARSING, &name)) && name)
            pins.push_back(name);
        CoTaskMemFree(name);
    }
    return pins;
}

bool ListHas(std::vector<std::wstring> const& list, std::wstring const& s) {
    std::wstring lower = Lower(s);
    for (auto const& each : list)
        if (Lower(each) == lower) return true;
    return false;
}

void ListToggle(PCWSTR value, std::wstring const& s, bool add) {
    g_homeValid = false;
    auto list = ReadStartList(value);
    std::wstring lower = Lower(s);
    std::erase_if(list, [&](std::wstring const& each) { return Lower(each) == lower; });
    if (add) list.push_back(s);
    WriteStartList(value, list);
}

// The programs you use most, from Windows' own count (UserAssist), matched to
// Start's list so only programs Start knows show.
struct StartUse {
    std::wstring parsing, name;
    DWORD count = 0;
    ULONGLONG last = 0;
};

std::vector<StartUse> ReadStartUse() {
    std::vector<StartUse> out;
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist\\"
                      L"{CEBFF5CD-ACE2-4F4F-9178-9926F41749EA}\\Count",
                      0, KEY_READ, &key) != ERROR_SUCCESS)
        return out;
    for (DWORD i = 0;; i++) {
        WCHAR name[1024];
        DWORD nameLen = ARRAYSIZE(name), type = 0;
        BYTE data[128];
        DWORD dataLen = sizeof(data);
        LONG r = RegEnumValueW(key, i, name, &nameLen, nullptr, &type, data, &dataLen);
        if (r == ERROR_NO_MORE_ITEMS) break;
        if (r != ERROR_SUCCESS || type != REG_BINARY || dataLen < 16) continue;
        for (WCHAR* c = name; *c; c++) {  // the names are ROT13
            if (*c >= L'a' && *c <= L'z') *c = L'a' + (*c - L'a' + 13) % 26;
            else if (*c >= L'A' && *c <= L'Z') *c = L'A' + (*c - L'A' + 13) % 26;
        }
        StartUse use;
        memcpy(&use.count, data + 4, sizeof(DWORD));
        if (dataLen >= 68) memcpy(&use.last, data + 60, sizeof(ULONGLONG));
        if (!use.count) continue;
        std::wstring id = name, lower = Lower(id);
        if (lower.find(L"uninstall") != std::wstring::npos || lower.starts_with(L"ueme_")) continue;
        auto it = g_startAppIds.find(lower);
        if (it != g_startAppIds.end()) {
            use.parsing = g_startApps[it->second].parsing;
            use.name = g_startApps[it->second].name;
        } else {
            std::wstring path = ExpandKnownFolder(id);
            if (!Lower(path).ends_with(L".lnk") || !FileExists(path)) continue;
            use.parsing = path;
            use.name = FileDisplayName(path);
        }
        if (Lower(use.name).find(L"uninstall") != std::wstring::npos) continue;
        out.push_back(use);
    }
    RegCloseKey(key);
    std::stable_sort(out.begin(), out.end(), [](StartUse const& a, StartUse const& b) {
        return a.count != b.count ? a.count > b.count : a.last > b.last;
    });
    return out;
}

////////////////////////////////////////////////////////////////////////////////
// Search: programs from Start's list at once; Settings (Windows' tasks) and
// files (from Windows' index, by folder, as StartAllBack groups them) from a
// thread of their own, so typing never waits.

enum SearchGroup { kGroupSettings, kGroupDocuments, kGroupPictures, kGroupMusic, kGroupVideos, kGroupDownloads,
                   kGroupDesktop, kGroupCount };
constexpr PCWSTR kGroupNames[kGroupCount] = {L"Settings", L"Documents", L"Pictures", L"Music", L"Videos", L"Downloads",
                                             L"Desktop"};

struct SearchHit {
    int group;
    std::wstring name, parsing;
};

struct SearchJob {
    HWND wnd;
    LONG generation;
    std::wstring query;                    // lower case
    std::wstring folders[kGroupCount];     // the files' folders (Settings has none)
};

struct SearchResult {
    LONG generation;
    std::vector<SearchHit> hits;
};

constexpr UINT WM_APP_SEARCHED = WM_APP + 6;
std::atomic<LONG> g_searchGeneration{0};
std::vector<SearchHit> g_searchHits;  // the latest search's, on the menu's thread
bool g_searchPending = false;

// Windows' settings tasks (Control Panel's "All Tasks"), read once.
std::mutex g_tasksLock;
std::vector<StartApp> g_tasks;
bool g_tasksRead = false;

// A late-bound call on a COM object: ADO needs no headers this way.
HRESULT DispCall(IDispatch* object, PCWSTR name, WORD flags, VARIANT* result, std::initializer_list<VARIANT> args = {}) {
    DISPID id;
    LPOLESTR names[] = {(LPOLESTR)name};
    HRESULT hr = object->GetIDsOfNames(IID_NULL, names, 1, LOCALE_USER_DEFAULT, &id);
    if (FAILED(hr)) return hr;
    std::vector<VARIANT> reversed(args.begin(), args.end());
    std::reverse(reversed.begin(), reversed.end());
    DISPPARAMS params{reversed.data(), nullptr, (UINT)reversed.size(), 0};
    if (result) VariantInit(result);
    return object->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, flags, &params, result, nullptr, nullptr);
}

std::wstring FieldText(IDispatch* recordset, PCWSTR field) {
    std::wstring text;
    VARIANT fields, item, value;
    if (FAILED(DispCall(recordset, L"Fields", DISPATCH_PROPERTYGET, &fields)) || fields.vt != VT_DISPATCH) return text;
    VARIANT name;
    name.vt = VT_BSTR;
    name.bstrVal = SysAllocString(field);
    if (SUCCEEDED(DispCall(fields.pdispVal, L"Item", DISPATCH_PROPERTYGET | DISPATCH_METHOD, &item, {name})) &&
        item.vt == VT_DISPATCH) {
        if (SUCCEEDED(DispCall(item.pdispVal, L"Value", DISPATCH_PROPERTYGET, &value)) && value.vt == VT_BSTR &&
            value.bstrVal)
            text = value.bstrVal;
        VariantClear(&value);
        VariantClear(&item);
    }
    VariantClear(&name);
    VariantClear(&fields);
    return text;
}

// Files in a folder whose names hold the query, from Windows' index.
void IndexSearch(IDispatch* connection, std::wstring const& folder, std::wstring const& query, int group, int limit,
                 std::vector<SearchHit>& hits) {
    std::wstring scope = folder, safe;
    std::replace(scope.begin(), scope.end(), L'\\', L'/');
    for (WCHAR c : query)
        if (c != L'\'' && c != L'%' && c != L'_' && c != L'[' && c != L'"') safe += c;
    if (safe.empty()) return;
    std::wstring sql = L"SELECT TOP " + std::to_wstring(limit) +
                       L" System.ItemPathDisplay, System.ItemNameDisplay FROM SystemIndex WHERE SCOPE='file:" + scope +
                       L"' AND System.FileName LIKE '%" + safe + L"%' ORDER BY System.DateModified DESC";
    VARIANT text, recordset;
    text.vt = VT_BSTR;
    text.bstrVal = SysAllocString(sql.c_str());
    HRESULT hr = DispCall(connection, L"Execute", DISPATCH_METHOD, &recordset, {text});
    VariantClear(&text);
    if (FAILED(hr) || recordset.vt != VT_DISPATCH) return;
    for (int n = 0; n < limit; n++) {
        VARIANT eof;
        if (FAILED(DispCall(recordset.pdispVal, L"EOF", DISPATCH_PROPERTYGET, &eof)) || (eof.vt == VT_BOOL && eof.boolVal))
            break;
        std::wstring path = FieldText(recordset.pdispVal, L"System.ItemPathDisplay");
        std::wstring name = FieldText(recordset.pdispVal, L"System.ItemNameDisplay");
        if (!path.empty()) hits.push_back({group, name.empty() ? path.substr(path.rfind(L'\\') + 1) : name, path});
        DispCall(recordset.pdispVal, L"MoveNext", DISPATCH_METHOD, nullptr);
    }
    DispCall(recordset.pdispVal, L"Close", DISPATCH_METHOD, nullptr);
    VariantClear(&recordset);
}

DWORD WINAPI SearchThread(LPVOID parameter) {
    auto* job = (SearchJob*)parameter;
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    auto* result = new SearchResult{job->generation, {}};
    // Settings: the tasks whose names hold the query anywhere, as StartAllBack
    // matches them.
    {
        std::lock_guard lock(g_tasksLock);
        if (!g_tasksRead) {
            g_tasksRead = true;
            ForEachShellChild(L"shell:::{ED7BA470-8E54-465E-825C-99712043E01C}", [](IShellItem* item) {
                std::wstring name = ItemName(item, SIGDN_NORMALDISPLAY), parsing = ItemName(item, SIGDN_DESKTOPABSOLUTEPARSING);
                if (!name.empty() && !parsing.empty()) g_tasks.push_back({name, parsing});
                return true;
            });
        }
        int n = 0;
        for (auto const& task : g_tasks) {
            if (job->generation != g_searchGeneration) break;  // typed on: not wanted
            if (Lower(task.name).find(job->query) == std::wstring::npos) continue;
            result->hits.push_back({kGroupSettings, task.name, task.parsing});
            if (++n >= 3) break;
        }
    }
    CLSID clsid;
    IDispatch* connection = nullptr;
    if (job->generation == g_searchGeneration && SUCCEEDED(CLSIDFromProgID(L"ADODB.Connection", &clsid)) &&
        SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&connection)))) {
        VARIANT open;
        open.vt = VT_BSTR;
        open.bstrVal = SysAllocString(L"Provider=Search.CollatorDSO;Extended Properties='Application=Windows';");
        if (SUCCEEDED(DispCall(connection, L"Open", DISPATCH_METHOD, nullptr, {open}))) {
            for (int group = kGroupDocuments; group < kGroupCount; group++) {
                if (job->generation != g_searchGeneration) break;
                if (!job->folders[group].empty())
                    IndexSearch(connection, job->folders[group], job->query, group, 3, result->hits);
            }
            DispCall(connection, L"Close", DISPATCH_METHOD, nullptr);
        }
        VariantClear(&open);
        connection->Release();
    }
    if (!IsWindow(job->wnd) || !PostMessageW(job->wnd, WM_APP_SEARCHED, 0, (LPARAM)result)) delete result;
    delete job;
    CoUninitialize();
    return 0;
}

void StartSearch(HWND wnd, std::wstring const& query) {
    auto* job = new SearchJob{wnd, InterlockedIncrement((LONG*)&g_searchGeneration), Lower(query), {}};
    job->folders[kGroupDocuments] = KnownFolderPath(FOLDERID_Documents);
    job->folders[kGroupPictures] = KnownFolderPath(FOLDERID_Pictures);
    job->folders[kGroupMusic] = KnownFolderPath(FOLDERID_Music);
    job->folders[kGroupVideos] = KnownFolderPath(FOLDERID_Videos);
    job->folders[kGroupDownloads] = KnownFolderPath(FOLDERID_Downloads);
    job->folders[kGroupDesktop] = KnownFolderPath(FOLDERID_Desktop);
    g_searchHits.clear();
    g_searchPending = true;
    HANDLE thread = CreateThread(nullptr, 0, SearchThread, job, 0, nullptr);
    if (thread) CloseHandle(thread);
    else {
        delete job;
        g_searchPending = false;
    }
}

int StartCompare(std::wstring const& a, std::wstring const& b) {
    return CompareStringEx(LOCALE_NAME_USER_DEFAULT, NORM_IGNORECASE | SORT_DIGITSASNUMBERS, a.c_str(), -1, b.c_str(),
                           -1, nullptr, nullptr, 0) - 2;
}

// All Programs: the Start Menu's Programs folders (yours and everyone's,
// merged), and the packaged apps, read a folder at a time as it opens.
struct ProgNode {
    std::wstring name, parsing;
    std::vector<std::wstring> dirs;
    std::vector<ProgNode> kids;
    bool folder = false, open = false, read = false;
};
ProgNode g_allRoot;

// Names are kept (by path) from one open to the next, and the warm-up fetches the top level's.
std::wstring ProgName(std::wstring const& path) {
    auto it = g_progNames.find(path);
    if (it != g_progNames.end()) return it->second;
    return g_progNames[path] = FileDisplayName(path);
}

void ReadProgKids(ProgNode& node) {
    if (node.read) return;
    node.read = true;
    StartTakeIcons();  // (with the warm-up's names)
    for (auto const& dir : node.dirs) {
        ForEachProgEntry(dir, [&](WIN32_FIND_DATAW const& fd) {
            std::wstring path = dir + L"\\" + fd.cFileName;
            bool folder = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            std::wstring name = ProgName(path);
            auto same = std::find_if(node.kids.begin(), node.kids.end(), [&](ProgNode const& k) {
                return k.folder == folder && !_wcsicmp(k.name.c_str(), name.c_str());
            });
            if (same != node.kids.end()) {
                if (folder) same->dirs.push_back(path);
                return;
            }
            ProgNode kid;
            kid.name = name;
            kid.parsing = path;
            kid.folder = folder;
            if (folder) kid.dirs.push_back(path);
            node.kids.push_back(std::move(kid));
        });
    }
    // Packaged apps have no shortcuts: they go at the top level.
    // Or in a folder of their own, as StartAllBack can put them.
    if (&node == &g_allRoot) {
        ProgNode apps;
        apps.name = L"Apps";
        apps.parsing = L"shell:AppsFolder";
        apps.folder = apps.read = true;
        // A Start Menu folder of your own called Apps (shortcuts for the packaged
        // apps, with your icons) takes them in, rather than a second Apps folder
        // listing each one twice. Your shortcut wins over the app of the same name.
        ProgNode* own = nullptr;
        if (g_startConfig.appsInFolder) {
            for (auto& k : node.kids)
                if (k.folder && !_wcsicmp(k.name.c_str(), L"Apps")) own = &k;
            if (own) ReadProgKids(*own);
        }
        ProgNode& into = !g_startConfig.appsInFolder ? node : own ? *own : apps;
        auto named = [](std::vector<ProgNode> const& kids, std::wstring const& name) {
            return std::any_of(kids.begin(), kids.end(),
                               [&](ProgNode const& k) { return !k.folder && !_wcsicmp(k.name.c_str(), name.c_str()); });
        };
        for (auto const& app : g_startApps) {
            if (app.parsing.find(L'!') == std::wstring::npos) continue;
            if (named(node.kids, app.name) || (own && named(own->kids, app.name))) continue;
            ProgNode kid;
            kid.name = app.name;
            kid.parsing = app.parsing;
            into.kids.push_back(std::move(kid));
        }
        if (own) {
            int order = g_startConfig.sortOrder;
            std::sort(own->kids.begin(), own->kids.end(), [order](ProgNode const& a, ProgNode const& b) {
                if (a.folder != b.folder && order != 2) return order == 1 ? a.folder : !a.folder;
                return StartCompare(a.name, b.name) < 0;
            });
        } else if (g_startConfig.appsInFolder && !apps.kids.empty()) {
            std::sort(apps.kids.begin(), apps.kids.end(),
                      [](ProgNode const& a, ProgNode const& b) { return StartCompare(a.name, b.name) < 0; });
            node.kids.push_back(std::move(apps));
        }
    }
    // Programs first, then folders, as Windows 7 lists them, unless set
    // otherwise ("Override sort order").
    int order = g_startConfig.sortOrder;
    std::sort(node.kids.begin(), node.kids.end(), [order](ProgNode const& a, ProgNode const& b) {
        if (a.folder != b.folder && order != 2) return order == 1 ? a.folder : !a.folder;
        return StartCompare(a.name, b.name) < 0;
    });
}

////////////////////////////////////////////////////////////////////////////////
// The right column's places

struct StartPlace {
    std::wstring name, target, folder, appId;
    std::wstring key;  // for what its right-click menu changes
    int kind = kKindLink;
    int special = 0;  // 1 the Settings pages, 2 Run, 3 a separator
    bool hidden = false;
};
std::vector<StartPlace> g_places;

// What a place's small icon ("Display glyph icons") comes from.
std::wstring PlaceIconSource(StartPlace const& place) {
    std::wstring source = !place.folder.empty() ? place.folder
                        : !place.appId.empty() ? L"shell:AppsFolder\\" + place.appId
                        : place.special == 1 ? L"shell:AppsFolder\\windows.immersivecontrolpanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel"
                                             : place.target;
    if (!source.empty() && source.find(L'\\') == std::wstring::npos && source.find(L':') == std::wstring::npos) {
        WCHAR found[MAX_PATH];
        if (SearchPathW(nullptr, source.c_str(), nullptr, MAX_PATH, found, nullptr)) source = found;
    }
    return source;
}

constexpr struct {
    PCWSTR name, uri;
} kSettingsPages[] = {
    {L"System", L"ms-settings:display"},
    {L"Bluetooth & devices", L"ms-settings:bluetooth"},
    {L"Network & internet", L"ms-settings:network"},
    {L"Personalization", L"ms-settings:personalization"},
    {L"Apps", L"ms-settings:appsfeatures"},
    {L"Accounts", L"ms-settings:yourinfo"},
    {L"Time & language", L"ms-settings:dateandtime"},
    {L"Gaming", L"ms-settings:gaming"},
    {L"Accessibility", L"ms-settings:easeofaccess"},
    {L"Privacy & security", L"ms-settings:privacy"},
    {L"Windows Update", L"ms-settings:windowsupdate"},
};

StartPlace ResolvePlace(StartPlaceSetting const& s) {
    StartPlace p;
    p.kind = s.kind == L"menu" ? kKindMenu : s.kind == L"jumpList" ? kKindJump : kKindLink;
    auto folder = [&](REFKNOWNFOLDERID id) { p.target = p.folder = KnownFolderPath(id); };
    auto shell = [&](PCWSTR parsing) { p.target = p.folder = parsing; };
    if (s.item == L"separator") p.special = 3;
    else if (s.item == L"user") folder(FOLDERID_Profile);
    else if (s.item == L"oneDrive") folder(FOLDERID_SkyDrive);
    else if (s.item == L"favorites") folder(FOLDERID_Favorites);
    else if (s.item == L"recentItems") folder(FOLDERID_Recent);
    else if (s.item == L"networkConnections") shell(L"shell:ConnectionsFolder");
    else if (s.item == L"windowsTools") shell(L"shell:::{D20EA4E1-3957-11d2-A40B-0C5020524153}");
    else if (s.item == L"devicesAndPrinters") shell(L"shell:::{A8A91A66-3A7D-4424-8D24-04E180695C7A}");
    else if (s.item == L"defaultPrograms") shell(L"shell:::{17cd9488-1228-4b2f-88ce-4298e93e0966}");
    else if (s.item == L"commandPrompt") {
        p.target = L"cmd.exe";
        p.name = L"Command Prompt";
    }
    else if (s.item == L"documents") folder(FOLDERID_Documents);
    else if (s.item == L"pictures") folder(FOLDERID_Pictures);
    else if (s.item == L"music") folder(FOLDERID_Music);
    else if (s.item == L"videos") folder(FOLDERID_Videos);
    else if (s.item == L"downloads") folder(FOLDERID_Downloads);
    else if (s.item == L"computer") shell(L"shell:MyComputerFolder");
    else if (s.item == L"network") shell(L"shell:NetworkPlacesFolder");
    else if (s.item == L"recycleBin") shell(L"shell:RecycleBinFolder");
    else if (s.item == L"controlPanel") {
        p.target = L"shell:ControlPanelFolder";
        p.folder = L"shell:::{21EC2020-3AEA-1069-A2DD-08002B30309D}";  // all its items
    } else if (s.item == L"settings") {
        p.target = L"ms-settings:";
        p.name = L"Settings";
        p.special = 1;
    } else if (s.item == L"terminal") {
        p.target = L"wt.exe";
        p.appId = L"Microsoft.WindowsTerminal_8wekyb3d8bbwe!App";
        p.name = L"Terminal";
    } else if (s.item == L"run") {
        p.name = L"Run...";
        p.special = 2;
    }
    if (s.item == L"controlPanel") p.name = L"Control Panel";
    // Your own, or a path given for one of the above: that instead.
    bool own = s.item == L"custom";
    if (own || (!s.path.empty() && p.special < 2)) {
        p.target = Expand(s.path.c_str());
        p.folder.clear();
        p.appId.clear();
        p.special = 0;
        if (own) p.name.clear();
        DWORD attributes = GetFileAttributesW(p.target.c_str());
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) p.folder = p.target;
        std::wstring lower = Lower(p.target);
        if (lower.ends_with(L".lnk")) p.appId = AppIdForShortcut(p.target);
        if (lower.starts_with(L"shell:appsfolder\\")) p.appId = p.target.substr(17);
        if (lower.starts_with(L"shell:") && p.folder.empty() && p.appId.empty()) p.folder = p.target;
    }
    if (!s.name.empty()) p.name = s.name;
    // Changed from its right-click menu: shown otherwise, or not at all.
    p.key = s.item + L"|" + Lower(s.path);
    WCHAR change[16] = L"";
    DWORD size = sizeof(change);
    if (RegGetValueW(HKEY_CURRENT_USER, (std::wstring(kStartKey) + L"\\Places").c_str(), p.key.c_str(), RRF_RT_REG_SZ,
                     nullptr, change, &size) == ERROR_SUCCESS) {
        if (!wcscmp(change, L"hidden")) p.hidden = true;
        else if (!wcscmp(change, L"menu")) p.kind = kKindMenu;
        else if (!wcscmp(change, L"link")) p.kind = kKindLink;
    }
    if (p.name.empty() && !p.target.empty()) p.name = StartName(p.target);
    if (p.name.empty()) p.name = s.item;
    // What each can show: a menu needs a folder (or the Settings pages), a
    // jump list an app.
    if (p.special == 1 && p.kind == kKindJump) p.kind = kKindMenu;
    if (p.kind == kKindMenu && p.folder.empty() && p.special != 1) p.kind = p.appId.empty() ? kKindLink : kKindJump;
    if (p.kind == kKindJump && p.appId.empty()) p.kind = p.folder.empty() ? kKindLink : kKindMenu;
    if (p.special >= 2) p.kind = kKindLink;
    return p;
}

////////////////////////////////////////////////////////////////////////////////
// The window's state and layout

enum StartView { kViewHome, kViewAll, kViewSearch };
enum RowKind { kRowItem, kRowFolder, kRowSeparator, kRowHeader };

struct StartRow {
    int kind = kRowItem;
    std::wstring name, parsing;
    int depth = 0;
    ProgNode* node = nullptr;
    bool pinned = false, frequent = false;
    int top = 0, height = 0;
};

HWND g_startWnd;
HWND g_startEdit;
HWND g_startTip;  // the tooltips of what's lit
bool g_startClass = false;
StartView g_startView = kViewHome;
std::vector<StartRow> g_rows;
std::vector<StartRow> g_homeRows;  // the home list as last built, for g_homeValid
int g_rowsHeight = 0;
int g_scrollPx = 0;
int g_smHot = -1;
int g_smPressed = -1;
int g_flyPlace = -1;          // the place whose flyout is open
bool g_flyFooter = false;     // All Programs' menu is open
bool g_startInMenu = false;   // a menu of ours is open: deactivation doesn't close it
bool g_startTracking = false;
bool g_startResetting = false;
bool g_scrollDrag = false;
int g_scrollGrabY = 0, g_scrollGrabPx = 0;
DWORD g_startHiddenAt = 0;
HHOOK g_startKeyHook;
bool g_winHeld = false, g_winUsed = false;

constexpr int kSmFooter = 10000, kSmPlace = 20000, kSmShutdown = 30000, kSmShutdownArrow = 30001, kSmScroll = 30002,
              kSmClear = 30003,  // the search box's X
              // The phone's, in the Start menu's pane or its own window: the same codes in both.
              kSmPhoneHead = 40000, kSmPhoneClose = 40001, kSmPhoneDetach = 40002, kSmPhonePin = 40003, kSmPhoneStrip = 40004,
              kSmPhoneTexts = 40005,  // the texts' heading: back to the list from a conversation
              kSmPhoneSend = 40006, kSmPhoneAction = 40010, kSmPhoneNote = 40100, kSmPhoneConvo = 40200;

constexpr UINT WM_APP_PHONE = WM_APP + 7;        // phone-link.cpp's read is back
constexpr UINT WM_APP_PHONE_TEXTS = WM_APP + 8;  // phone-map.cpp: a text came or went, or the link changed
constexpr UINT WM_APP_PHONE_NOTES = WM_APP + 9;  // phone-ancs.cpp: the phone's notifications came or went

// The phone's links, into Phone Link's own pages. Their icons: yours (startMenu.phone*Icon), else the
// phone app's (a package), else Phone Link's.
struct PhoneAction {
    PCWSTR name, target, package;
};
constexpr PhoneAction kPhoneActions[] = {
    {L"Messages", L"ms-phone:messages", L"com.apple.mobilesms"},
    {L"Notifications", L"ms-phone:phonenotifications", L""},
    {L"Send a file", L"ms-phone:filetransfer", L""},
    {L"Phone settings", L"ms-phone:settings?target=DeviceList", L""},
};

// Where everything of the phone's goes, in the Start menu or in its own window.
struct PhoneLayout {
    RECT strip{};                    // its own window's title strip (empty in the Start menu)
    RECT head{}, heading{}, close{}, detach{}, pin{};
    std::vector<RECT> actions, notes, seps;
    // Its texts (Tourne holding them): the heading, then the conversations; or one open, its texts, the reply box
    // (an edit control over it) and Send.
    RECT textsHead{}, thread{}, reply{}, send{};
    std::vector<RECT> convos;
    int bottom = 0;
};

struct StartLayout {
    int width = 0, height = 0, left = 0, body = 0, strip = 0, row = 0, sep = 0, placeRow = 0, placeSep = 0;
    RECT list{}, footerSep{}, footer{}, search{}, shutdownFrame{}, shutdown{}, shutdownArrow{}, scroll{};
    std::vector<RECT> places;
    int placesRight = 0;  // where the places end: the window's edge, or the phone pane's start
    bool phone = false;   // the phone pane is shown (in the Start menu, not apart)
    PhoneLayout ply;
};
PhoneInfo g_phoneView;  // what the phone shows (the bar's copy of phone-link.cpp's latest read)

// The phone's texts, when Tourne holds them (startMenu.phoneMessages, phone-map.cpp): as last told, its
// conversations newest first, and the one open in the pane with its reply box, if any.
struct PhoneConvo {
    std::wstring key, address, name, last;
    long long when = 0;
    wchar_t kind = L'i';  // the last text's (phonemap::Text)
    int unread = 0;
};
std::vector<phonemap::Text> g_phoneTexts;
std::vector<PhoneConvo> g_phoneConvos;
phonemap::Link g_phoneLink = phonemap::Link::Off;
std::wstring g_phoneChat;  // the open conversation's key; empty: the list
HWND g_phoneReply;         // its reply box, a child of the window the phone shows in

bool PhoneTexting();

// The phone kept apart or out of sight (from its own buttons and menu; HKCU\Software\Tourne\StartMenu).
struct PhoneState {
    bool hidden = false;    // closed: not shown until asked for again
    bool apart = false;     // in a window of its own, beside the Start menu
    bool pinned = false;    // that window stays open when the Start menu closes
    bool placed = false;    // it has been moved: x, y
    int x = 0, y = 0;
    int w = 0, h = 0;       // its own window's size, as you pulled it (at 96 dpi; 0: as it comes)
    // Docked to one of Tourne's windows (a bar, the pager, the Start menu): it follows that window. dockClass is its
    // class, dockSide the side the phone sits on (0 left of it, 1 above, 2 right, 3 below), dockAlong where along that
    // side (at 96 dpi), dockX/Y the window's middle when docked (to pick the right bar of several).
    std::wstring dockClass;
    int dockSide = 0, dockAlong = 0, dockX = 0, dockY = 0;
};
PhoneState g_phoneState;
HWND g_phoneWnd;           // its own window
PhoneLayout g_phoneWly;    // laid out in it
int g_phoneHot = -1, g_phonePressed = -1;
bool g_phoneDragging = false;
POINT g_phoneGrab{};       // where it was taken, from its top left
POINT g_phoneDown{};       // where the press on the pane's head began (the Start menu's client)

void LoadPhoneState() {
    auto get = [](PCWSTR name, DWORD fallback) {
        DWORD v = fallback, size = sizeof(v);
        RegGetValueW(HKEY_CURRENT_USER, kStartKey, name, RRF_RT_REG_DWORD, nullptr, &v, &size);
        return v;
    };
    g_phoneState.hidden = get(L"PhoneHidden", 0) != 0;
    g_phoneState.apart = get(L"PhoneApart", 0) != 0;
    g_phoneState.pinned = get(L"PhonePinned", 0) != 0;
    g_phoneState.placed = get(L"PhonePlaced", 0) != 0;
    g_phoneState.x = (int)get(L"PhoneX", 0);
    g_phoneState.y = (int)get(L"PhoneY", 0);
    g_phoneState.w = (int)get(L"PhoneWidth", 0);
    g_phoneState.h = (int)get(L"PhoneHeight", 0);
    g_phoneState.dockSide = (int)get(L"PhoneDockSide", 0);
    g_phoneState.dockAlong = (int)get(L"PhoneDockAlong", 0);
    g_phoneState.dockX = (int)get(L"PhoneDockX", 0);
    g_phoneState.dockY = (int)get(L"PhoneDockY", 0);
    WCHAR dock[128] = L"";
    DWORD size = sizeof(dock);
    if (RegGetValueW(HKEY_CURRENT_USER, kStartKey, L"PhoneDock", RRF_RT_REG_SZ, nullptr, dock, &size) != ERROR_SUCCESS) dock[0] = 0;
    g_phoneState.dockClass = dock;
}

void SavePhoneState() {
    auto set = [](PCWSTR name, DWORD v) { RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, name, REG_DWORD, &v, sizeof(v)); };
    set(L"PhoneHidden", g_phoneState.hidden);
    set(L"PhoneApart", g_phoneState.apart);
    set(L"PhonePinned", g_phoneState.pinned);
    set(L"PhonePlaced", g_phoneState.placed);
    set(L"PhoneX", (DWORD)g_phoneState.x);
    set(L"PhoneY", (DWORD)g_phoneState.y);
    set(L"PhoneWidth", (DWORD)g_phoneState.w);
    set(L"PhoneHeight", (DWORD)g_phoneState.h);
    set(L"PhoneDockSide", (DWORD)g_phoneState.dockSide);
    set(L"PhoneDockAlong", (DWORD)g_phoneState.dockAlong);
    set(L"PhoneDockX", (DWORD)g_phoneState.dockX);
    set(L"PhoneDockY", (DWORD)g_phoneState.dockY);
    std::wstring const& dock = g_phoneState.dockClass;
    RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, L"PhoneDock", REG_SZ, dock.c_str(), (DWORD)((dock.size() + 1) * sizeof(WCHAR)));
}

// The Start menu's own size, as you pulled its edges (at 96 dpi, over what it comes to): StartExtraWidth goes to the
// programs list, StartExtraHeight to its height. HKCU\Software\Tourne\StartMenu.
int g_startExtraW = 0, g_startExtraH = 0;
int g_startSizing = 0, g_startFromW = 0, g_startFromH = 0;  // while an edge is pulled: which, and the size it began at
POINT g_startGrab{};

void LoadStartSize() {
    auto get = [](PCWSTR name) {
        DWORD v = 0, size = sizeof(v);
        RegGetValueW(HKEY_CURRENT_USER, kStartKey, name, RRF_RT_REG_DWORD, nullptr, &v, &size);
        return (int)v;
    };
    g_startExtraW = std::clamp(get(L"StartExtraWidth"), 0, 4000);
    g_startExtraH = std::clamp(get(L"StartExtraHeight"), 0, 4000);
}

void SaveStartSize() {
    DWORD w = (DWORD)g_startExtraW, h = (DWORD)g_startExtraH;
    RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, L"StartExtraWidth", REG_DWORD, &w, sizeof(w));
    RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, L"StartExtraHeight", REG_DWORD, &h, sizeof(h));
}

// The phone is wanted at all: set on, a phone linked, and not closed.
bool PhoneWanted() {
    return g_startConfig.phone && !g_phoneState.hidden && PhoneLinked();
}

// Tourne holds the phone's texts (Tourne Desktop, startMenu.phoneMessages above 0): its own Messages section
// takes the place of Phone Link's Messages link.
bool PhoneTexting() { return g_startConfig.phoneTexts > 0; }
StartLayout g_sly;

std::wstring StartCase(std::wstring s);
void BuildStartRows();

// The phone laid out from (x0, y0), x1 wide: in its own window a title strip first (to move it by, with
// Keep open, Back into Start and Close), in the Start menu Apart and Close at its head's corner. Then its
// name, system and battery; a line; its links; a line; its latest notifications.
// fillTo (its own window, sized by you): as many notifications as fit above it, instead of startMenu.phoneNotifications.
PhoneLayout PhoneComputeLayout(int x0, int y0, int x1, bool apart, int fillTo = -1) {
    PhoneLayout P;
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = SelectObject(dc, StartFont());
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    int row = std::max(Px(22), (int)tm.tmHeight + Px(6)), placeRow = std::max(Px(29), (int)tm.tmHeight + Px(12)), sep = Px(4) * 2;
    int button = Px(20), py = y0;
    if (apart) {
        P.strip = {x0, py, x1, py + Px(26)};
        int bx = x1 - Px(4) - button, by = py + (Px(26) - button) / 2;
        P.close = {bx, by, bx + button, by + button};
        P.detach = {bx - button - Px(2), by, bx - Px(2), by + button};
        P.pin = {P.detach.left - button - Px(2), by, P.detach.left - Px(2), by + button};
        py = P.strip.bottom + Px(4);
    }
    P.head = {x0, py, x1, py + std::max(Px(58), (int)tm.tmHeight * 3 + Px(10))};
    if (!apart) {
        int bx = x1 - Px(6) - button;
        P.close = {bx, py, bx + button, py + button};
        P.detach = {bx - button - Px(2), py, bx - Px(2), py + button};
    }
    py = P.head.bottom;
    P.seps.push_back({x0, py, x1, py + sep});
    py += sep;
    for (size_t i = 0; i < std::size(kPhoneActions); i++) {
        if (i == 0 && PhoneTexting()) {
            P.actions.push_back(RECT{});  // (Messages: Tourne's own, below)
            continue;
        }
        P.actions.push_back({x0, py, x1, py + placeRow});
        py += placeRow;
    }
    int noteRow = std::max(Px(38), (int)tm.tmHeight * 2 + Px(10));
    if (PhoneTexting()) {
        P.seps.push_back({x0, py, x1, py + sep});
        py += sep;
        P.textsHead = {x0, py, x1, py + row};
        py += row;
        int area = g_startConfig.phoneTexts * noteRow;
        if (!g_phoneChat.empty()) {
            // A conversation open: its texts, and under them the reply box with Send.
            area = std::max(area, 4 * noteRow);
            int box = row + Px(12);
            P.thread = {x0, py, x1, py + area - box};
            P.reply = {x0 + Px(12), py + area - box + Px(4), x1 - Px(10), py + area - Px(4)};
            P.send = {P.reply.right - Px(52), P.reply.top, P.reply.right, P.reply.bottom};
            py += area;
        } else {
            for (int i = 0; i < g_startConfig.phoneTexts; i++, py += noteRow) P.convos.push_back({x0, py, x1, py + noteRow});
        }
    }
    int notes = fillTo < 0 ? g_startConfig.phoneNotes : std::clamp((fillTo - py - sep - row) / noteRow, 0, 30);
    if (notes > 0) {
        P.seps.push_back({x0, py, x1, py + sep});
        py += sep;
        P.heading = {x0, py, x1, py + row};
        py += row;
        for (int i = 0; i < notes; i++, py += noteRow) P.notes.push_back({x0, py, x1, py + noteRow});
    }
    P.bottom = py;
    return P;
}

// How many notifications to read: as many as Start's pane or the phone's own window (sized by you) shows.
int PhoneNotesWanted() { return std::max(g_startConfig.phoneNotes, (int)g_phoneWly.notes.size()); }

// The phone read again in the background (back as WM_APP_PHONE); with Tourne holding it, its own notifications only.
void PhoneReadAgain() {
    if (g_startWnd) ReadPhoneSoon(g_startWnd, WM_APP_PHONE, PhoneNotesWanted(), PhoneTexting());
}

// The phone just came into sight: the open conversation (kept from last time) has been seen now.
void PhoneSeen() {
    if (!g_phoneChat.empty()) phonemap::MarkRead(g_phoneChat);
}

// What's under pt in the phone's layout: one of the kSmPhone codes, or -1.
int PhoneHitTest(PhoneLayout const& P, POINT pt) {
    if (PtInRect(&P.close, pt)) return kSmPhoneClose;
    if (PtInRect(&P.detach, pt)) return kSmPhoneDetach;
    if (PtInRect(&P.pin, pt)) return kSmPhonePin;
    if (PtInRect(&P.strip, pt)) return kSmPhoneStrip;
    if (PtInRect(&P.head, pt)) return kSmPhoneHead;
    for (int i = 0; i < (int)P.actions.size(); i++)
        if (PtInRect(&P.actions[i], pt)) return kSmPhoneAction + i;
    if (PtInRect(&P.textsHead, pt)) return g_phoneChat.empty() ? -1 : kSmPhoneTexts;
    if (PtInRect(&P.send, pt)) return kSmPhoneSend;
    if (PtInRect(&P.thread, pt) || PtInRect(&P.reply, pt)) return -1;
    for (int i = 0; i < (int)P.convos.size(); i++)
        if (PtInRect(&P.convos[i], pt)) return i < (int)g_phoneConvos.size() ? kSmPhoneConvo + i : -1;
    for (int i = 0; i < (int)P.notes.size(); i++)
        if (PtInRect(&P.notes[i], pt)) return i < (int)g_phoneView.notes.size() ? kSmPhoneNote + i : -1;
    return -1;
}

RECT PhoneHitRect(PhoneLayout const& P, int hit) {
    switch (hit) {
        case kSmPhoneClose: return P.close;
        case kSmPhoneDetach: return P.detach;
        case kSmPhonePin: return P.pin;
        case kSmPhoneStrip: return P.strip;
        case kSmPhoneHead: return P.head;  // (its corner buttons are inside it)
        case kSmPhoneTexts: return P.textsHead;
        case kSmPhoneSend: return P.send;
    }
    if (hit >= kSmPhoneConvo && hit - kSmPhoneConvo < (int)P.convos.size()) return P.convos[hit - kSmPhoneConvo];
    if (hit >= kSmPhoneAction && hit - kSmPhoneAction < (int)P.actions.size()) return P.actions[hit - kSmPhoneAction];
    if (hit >= kSmPhoneNote && hit - kSmPhoneNote < (int)P.notes.size()) return P.notes[hit - kSmPhoneNote];
    return RECT{};
}

bool IsPhoneHit(int hit) {
    return hit >= kSmPhoneHead && hit < kSmPhoneConvo + 100;
}

// The lists' icons, as set for each view (at the screen's scale).
int StartIconPx() {
    auto const& c = g_startConfig;
    int size = g_startView == kViewHome ? (c.myLarge ? 32 : c.iconSize)
             : g_startView == kViewAll  ? (c.allLarge ? 32 : 16)
                                        : (c.searchLarge ? 32 : 16);
    return Px(size);
}

int AllProgramsIconPx() {
    return Px(g_startConfig.allLarge ? 32 : 16);
}

void StartComputeLayout() {
    auto& L = g_sly;
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = SelectObject(dc, StartFont());
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    // Measured from StartAllBack's Windows 7 menu at 100%, from the style's
    // frame: the panes' see-through margin (F left, T top) and their border.
    int F = g_sl.frameLeft, T = g_sl.frameTop, B = g_sl.frameBorder;
    L.row = std::max(Px(22), (int)tm.tmHeight + Px(6));
    L.placeRow = std::max(Px(29), (int)tm.tmHeight + Px(12));  // 29 px apart in StartAllBack (frame-by-frame capture)
    L.sep = Px(9);
    L.placeSep = Px(4);
    L.strip = T + Px(24);
    L.left = F + Px(244) + Px(g_startExtraW);  // (wider as you pulled it: the programs list takes it)
    L.placesRight = L.left + Px(150);
    // The phone pane, beside the places, as Windows 11 has its phone beside Start (unless it's apart).
    L.phone = PhoneWanted() && !g_phoneState.apart;
    L.width = L.placesRight + (L.phone ? Px(250) : 0);
    int top = T + Px(26);
    // The list is as tall as the pins and recent programs, as StartAllBack's.
    StartView view = g_startView;
    g_startView = kViewHome;
    BuildStartRows();
    int listHeight = std::max(g_rowsHeight, 4 * L.row) + Px(g_startExtraH);  // (and taller)
    g_startView = view;
    int leftBottom = top + listHeight + Px(7) + L.row + Px(6);
    int y = T + Px(33);
    L.places.clear();
    for (auto const& p : g_places) {
        int h = p.special == 3 ? L.placeSep : L.placeRow;
        L.places.push_back({L.left, y, L.placesRight, y + h});
        y += h;
    }
    L.ply = {};
    if (L.phone) {
        L.ply = PhoneComputeLayout(L.placesRight, T + Px(33), L.width - B, false);
        y = std::max(y, L.ply.bottom);
    }
    L.body = std::max(leftBottom, y + Px(6));
    listHeight += L.body - leftBottom;  // any extra height goes to the list
    L.list = {F, top, L.left, top + listHeight};
    L.footerSep = {F, L.list.bottom, L.left, L.list.bottom + Px(7)};
    L.footer = {F, L.footerSep.bottom, L.left, L.footerSep.bottom + L.row};
    // Under the panes, their border, then the 45 px bottom bar.
    L.height = L.body + B + Px(45) + B;
    int boxHeight = Px(22), boxTop = L.body + B + (Px(45) - boxHeight) / 2;
    L.search = {F + Px(14), boxTop, L.left - Px(18), boxTop + boxHeight};
    // Shut down: as wide as its text.
    HDC measure = GetDC(nullptr);
    HGDIOBJ previous = SelectObject(measure, StartFont());
    std::wstring label = StartCase(PowerLabel(g_startConfig.powerButton));
    SIZE text{};
    GetTextExtentPoint32W(measure, label.c_str(), (int)label.size(), &text);
    SelectObject(measure, previous);
    ReleaseDC(nullptr, measure);
    // A dark frame holding the button and its arrow box, 3 px apart, as
    // StartAllBack's.
    int pad = Px(3);
    L.shutdownFrame = {L.left + Px(7), boxTop, L.left + Px(7) + pad + text.cx + Px(8) + pad + Px(18) + pad,
                       boxTop + boxHeight};
    L.shutdown = {L.shutdownFrame.left + pad, boxTop + pad, L.shutdownFrame.left + pad + text.cx + Px(8),
                  boxTop + boxHeight - pad};
    L.shutdownArrow = {L.shutdown.right + pad, boxTop + pad, L.shutdown.right + pad + Px(18), boxTop + boxHeight - pad};
    int bar = GetSystemMetricsForDpi(SM_CXVSCROLL, 96 * g_scale);
    L.scroll = {L.list.right - Px(2) - bar, L.list.top, L.list.right - Px(2), L.list.bottom};
}

// Search results fill the whole panel, as StartAllBack shows them; the list
// is the left pane otherwise.
void StartViewWidth() {
    auto& L = g_sly;
    int right = g_startView == kViewSearch ? L.width - g_sl.frameBorder : L.left;
    int shift = right - L.list.right;
    L.list.right = L.footerSep.right = L.footer.right = right;
    OffsetRect(&L.scroll, shift, 0);
}

bool StartScrolls() {
    return g_startView != kViewHome && g_rowsHeight > g_sly.list.bottom - g_sly.list.top;
}

void ClampScroll() {
    int room = g_sly.list.bottom - g_sly.list.top;
    g_scrollPx = std::clamp(g_scrollPx, 0, std::max(0, g_rowsHeight - room));
}

void AddProgRows(ProgNode& node, int depth) {
    for (auto& kid : node.kids) {
        StartRow row;
        row.kind = kid.folder ? kRowFolder : kRowItem;
        row.name = kid.name;
        row.parsing = kid.parsing;
        row.depth = depth;
        row.node = &kid;
        g_rows.push_back(row);
        if (kid.folder && kid.open) {
            ReadProgKids(kid);
            AddProgRows(kid, depth + 1);
        }
    }
}

std::wstring StartSearchText() {
    if (!g_startEdit) return {};
    int n = GetWindowTextLengthW(g_startEdit);
    std::wstring s(n, L'\0');
    if (n) GetWindowTextW(g_startEdit, s.data(), n + 1);
    return s;
}

void AddMatches(std::vector<StartApp> const& list, std::wstring const& query, PCWSTR header, size_t limit) {
    std::vector<std::pair<int, StartApp const*>> found;
    for (auto const& app : list) {
        std::wstring name = Lower(app.name);
        size_t at = name.find(query);
        if (at == std::wstring::npos) continue;
        found.push_back({at == 0 ? 0 : name[at - 1] == L' ' ? 1 : 2, &app});
    }
    if (found.empty()) return;
    std::stable_sort(found.begin(), found.end(), [](auto const& a, auto const& b) { return a.first < b.first; });
    StartRow title;
    title.kind = kRowHeader;
    title.name = header;
    g_rows.push_back(title);
    for (size_t i = 0; i < found.size() && i < limit; i++) {
        StartRow row;
        row.name = found[i].second->name;
        row.parsing = found[i].second->parsing;
        row.depth = 2;  // under its heading
        g_rows.push_back(row);
    }
}

void BuildStartRows() {
    g_rows.clear();
    if (g_startView == kViewHome && g_homeValid) {
        g_rows = g_homeRows;  // (their positions are worked out again below)
    } else if (g_startView == kViewHome) {
        // Windows' (and StartAllBack's) pins, then ones pinned here; taken
        // off here: hidden.
        auto hidden = ReadStartList(L"Hidden");
        std::vector<std::wstring> pins;
        for (auto const& pin : ReadWindowsStartPins())
            if (!ListHas(hidden, pin)) pins.push_back(pin);
        for (auto const& pin : ReadStartList(L"Pins"))
            if (!ListHas(pins, pin) && !ListHas(hidden, pin)) pins.push_back(pin);
        std::vector<std::wstring> seen;
        for (auto const& pin : pins) {
            StartRow row;
            row.parsing = pin;
            row.name = Lower(pin).ends_with(L".lnk") ? FileDisplayName(pin) : StartName(pin);
            row.pinned = true;
            seen.push_back(Lower(pin));
            seen.push_back(Lower(row.name));
            g_rows.push_back(row);
        }
        if (!g_rows.empty()) g_rows.push_back({kRowSeparator});
        int room = g_startConfig.iconCount;
        for (auto const& use : ReadStartUse()) {
            if (room <= 0) break;
            if (ListHas(seen, use.parsing) || ListHas(seen, use.name) || ListHas(hidden, use.parsing)) continue;
            seen.push_back(Lower(use.parsing));
            seen.push_back(Lower(use.name));
            StartRow row;
            row.parsing = use.parsing;
            row.name = use.name;
            row.frequent = true;
            g_rows.push_back(row);
            room--;
        }
        if (!g_rows.empty() && g_rows.back().kind == kRowSeparator) g_rows.pop_back();
        g_homeRows = g_rows;
        g_homeValid = true;
    } else if (g_startView == kViewAll) {
        if (g_allRoot.dirs.empty()) {
            g_allRoot = {};
            g_allRoot.dirs = {KnownFolderPath(FOLDERID_Programs), KnownFolderPath(FOLDERID_CommonPrograms)};
        }
        ReadProgKids(g_allRoot);
        AddProgRows(g_allRoot, 0);
    } else {
        std::wstring query = Lower(StartSearchText());
        AddMatches(g_startApps, query, L"Programs", 5);
        // Settings and files, as the search thread finds them.
        for (int group = 0; group < kGroupCount; group++) {
            bool titled = false;
            for (auto const& hit : g_searchHits) {
                if (hit.group != group) continue;
                if (!titled) {
                    StartRow title;
                    title.kind = kRowHeader;
                    title.name = kGroupNames[group];
                    g_rows.push_back(title);
                    titled = true;
                }
                StartRow row;
                row.name = hit.name;
                row.parsing = hit.parsing;
                row.depth = 2;
                g_rows.push_back(row);
            }
        }
        if (g_rows.empty() && !g_searchPending) {
            StartRow none;
            none.kind = kRowHeader;
            none.name = L"No items match your search.";
            g_rows.push_back(none);
        }
    }
    int y = 0;
    for (auto& row : g_rows) {
        row.top = y;
        row.height = row.kind == kRowSeparator ? g_sly.sep
                   : row.kind == kRowHeader    ? g_sly.row
                                               : std::max(g_sly.row, StartIconPx() + Px(6));
        y += row.height;
    }
    g_rowsHeight = y;
    ClampScroll();
}

RECT StartRowRect(StartRow const& row) {
    auto const& L = g_sly;
    int right = StartScrolls() ? L.scroll.left - Px(2) : L.list.right;
    int top = L.list.top + row.top - g_scrollPx;
    return {L.list.left, top, right, top + row.height};
}

// The scroll bar's pieces: its arrows, the track between them, the thumb.
struct StartScrollBar {
    RECT up, down, track, thumb;
    int travel;  // how far the thumb moves
};

StartScrollBar StartScrollGeometry() {
    auto const& L = g_sly;
    RECT s = L.scroll;
    int w = s.right - s.left;
    StartScrollBar b;
    b.up = {s.left, s.top, s.right, s.top + w};
    b.down = {s.left, s.bottom - w, s.right, s.bottom};
    b.track = {s.left, b.up.bottom, s.right, b.down.top};
    int room = L.list.bottom - L.list.top, trackHeight = std::max(0, (int)(b.track.bottom - b.track.top));
    int h = std::min(trackHeight, std::max(Px(12), trackHeight * room / std::max(1, g_rowsHeight)));
    b.travel = std::max(1, trackHeight - h);
    int top = b.track.top + b.travel * g_scrollPx / std::max(1, g_rowsHeight - room);
    b.thumb = {s.left, top, s.right, top + h};
    return b;
}

bool Selectable(int hit) {
    if (hit >= 0 && hit < (int)g_rows.size()) return g_rows[hit].kind == kRowItem || g_rows[hit].kind == kRowFolder;
    if (IsPhoneHit(hit)) return g_sly.phone && hit != kSmPhoneStrip;
    return hit == kSmFooter || hit == kSmShutdown || hit == kSmShutdownArrow ||
           (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size() && g_places[hit - kSmPlace].special != 3);
}

int StartHitTest(POINT pt) {
    auto const& L = g_sly;
    if (PtInRect(&L.list, pt)) {
        if (StartScrolls() && PtInRect(&L.scroll, pt)) return kSmScroll;
        for (int i = 0; i < (int)g_rows.size(); i++) {
            RECT r = StartRowRect(g_rows[i]);
            if (PtInRect(&r, pt)) return Selectable(i) ? i : -1;
        }
        return -1;
    }
    if (PtInRect(&L.footer, pt)) return kSmFooter;
    for (int i = 0; i < (int)L.places.size() && g_startView != kViewSearch; i++)
        if (PtInRect(&L.places[i], pt)) return g_places[i].special == 3 ? -1 : kSmPlace + i;
    if (L.phone && g_startView != kViewSearch) {
        int hit = PhoneHitTest(L.ply, pt);
        if (hit != -1) return hit;
    }
    RECT clear{L.search.right - Px(22), L.search.top, L.search.right, L.search.bottom};
    if (g_startView == kViewSearch && PtInRect(&clear, pt)) return kSmClear;
    if (PtInRect(&L.shutdown, pt)) return kSmShutdown;
    if (PtInRect(&L.shutdownArrow, pt)) return kSmShutdownArrow;
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing

bool SpDraw(HDC dc, int part, int state, RECT const& r) {
    if (!g_sl.panel || !IsThemePartDefined(g_sl.panel, part, 0)) return false;
    return SUCCEEDED(DrawThemeBackground(g_sl.panel, dc, part, state, &r, nullptr));
}

void StartFrame(HDC dc, RECT const& r, uint32_t rgb) {
    SetDCBrushColor(dc, PaletteRef(rgb));
    FrameRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
}

// dir: 0 right, 1 left, 2 down, 3 up.
void StartTriangle(HDC dc, RECT const& r, int dir, uint32_t rgb) {
    int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2, s = Px(3);
    POINT pts[3];
    if (dir == 0) pts[0] = {cx - s / 2, cy - s}, pts[1] = {cx - s / 2, cy + s}, pts[2] = {cx + s / 2 + 1, cy};
    else if (dir == 1) pts[0] = {cx + s / 2, cy - s}, pts[1] = {cx + s / 2, cy + s}, pts[2] = {cx - s / 2 - 1, cy};
    else if (dir == 2) pts[0] = {cx - s, cy - s / 2}, pts[1] = {cx + s, cy - s / 2}, pts[2] = {cx, cy + s / 2 + 1};
    else pts[0] = {cx - s, cy + s / 2}, pts[1] = {cx + s, cy + s / 2}, pts[2] = {cx, cy - s / 2 - 1};
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(DC_BRUSH)), oldPen = SelectObject(dc, GetStockObject(DC_PEN));
    SetDCBrushColor(dc, PaletteRef(rgb));
    SetDCPenColor(dc, PaletteRef(rgb));
    Polygon(dc, pts, 3);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
}

// A part's arrow, centred in r; the fallback's direction as StartTriangle's.
void StartArrow(HDC dc, int part, RECT const& r, int dir, uint32_t rgb) {
    SIZE size{};
    if (g_sl.panel && IsThemePartDefined(g_sl.panel, part, 0) &&
        SUCCEEDED(GetThemePartSize(g_sl.panel, dc, part, 1, nullptr, TS_TRUE, &size)) && size.cx > 0 && size.cy > 0) {
        int w = size.cx * g_scale, h = size.cy * g_scale;
        RECT a{(r.left + r.right - w) / 2, (r.top + r.bottom - h) / 2, 0, 0};
        a.right = a.left + w;
        a.bottom = a.top + h;
        DrawThemeBackground(g_sl.panel, dc, part, 1, &a, nullptr);
        return;
    }
    StartTriangle(dc, r, dir, rgb);
}

void StartSeparator(HDC dc, int part, RECT const& r) {
    int inset = part == kSpPlaceSeparator ? Px(10) : Px(6);
    RECT line{r.left + inset, (r.top + r.bottom) / 2, r.right - inset, (r.top + r.bottom) / 2 + 1};
    SIZE size{};
    if (g_sl.panel && IsThemePartDefined(g_sl.panel, part, 0) &&
        SUCCEEDED(GetThemePartSize(g_sl.panel, dc, part, 1, nullptr, TS_TRUE, &size)) && size.cy > 0) {
        line.bottom = line.top + std::min((int)size.cy, Px(4));
        if (SpDraw(dc, part, 1, line)) return;
    }
    Fill(dc, line, StartMix(g_settings.palette.ground, g_sl.text, 0.25));
}

// Inset as StartAllBack's: on the right, clear of the frame too.
void StartHighlight(HDC dc, RECT r, bool pressed) {
    bool place = r.left >= g_sly.left;
    r.left += place ? Px(6) : Px(7);
    r.right -= place ? g_sl.frameBorder + Px(7) : Px(4);
    if (!SpDraw(dc, kSpItem, pressed ? 3 : 2, r)) Fill(dc, r, g_sl.hover);
}

bool StartCapsSetting() { return g_startConfig.caps; }

std::wstring StartCase(std::wstring s) {
    if (g_startConfig.caps) CharUpperBuffW(s.data(), (DWORD)s.size());
    return s;
}

void StartText(HDC dc, std::wstring const& text, RECT r, uint32_t rgb, UINT align = DT_LEFT) {
    std::wstring shown = StartCase(text);
    SetTextColor(dc, PaletteRef(rgb));
    DrawTextW(dc, shown.c_str(), -1, &r, align | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
}

// The panes and the bottom bar. With the style's parts, as StartAllBack lays
// them: both panes the full height (their frame goes all the way round), the
// bottom bar inside the frame, and the frame closed along the bottom.
void StartBackground(HDC dc) {
    auto const& L = g_sly;
    auto const& p = g_settings.palette;
    bool searching = g_startView == kViewSearch;
    if (!g_sl.panel) {
        RECT all{0, 0, L.width, L.height};
        Fill(dc, all, p.ground);
        if (!searching) Fill(dc, RECT{0, L.strip, L.left, L.body}, p.raised);
        StartFrame(dc, all, p.edge);
        return;
    }
    int F = g_sl.frameLeft, B = g_sl.frameBorder;
    SpDraw(dc, kSpLeft, 1, RECT{0, 0, L.left, L.height});
    // Searching, the results have the places' ground across the panel.
    SpDraw(dc, kSpRight, 1, RECT{searching ? F + B - 1 : L.left, 0, L.width, L.height});
    Fill(dc, RECT{F, L.body, L.width, L.body + B}, g_sl.border);
    SpDraw(dc, kSpBottom, 1, RECT{F + B, L.body + B, L.width - B, L.height - B});
    Fill(dc, RECT{F, L.height - B, L.width, L.height}, g_sl.border);
}

// The window cut to the style's outline, so its see-through margins show
// what's behind, as StartAllBack's does.
void StartShape(HWND wnd) {
    auto const& L = g_sly;
    if (!g_sl.panel || L.width <= 0 || L.height <= 0) {
        SetWindowRgn(wnd, nullptr, TRUE);
        return;
    }
    auto pixels = StartKeyedRender(L.width, L.height, [](HDC dc) { StartBackground(dc); });
    HRGN shape = CreateRectRgn(0, 0, 0, 0);
    for (int y = 0; y < L.height && !pixels.empty(); y++) {
        for (int x = 0; x < L.width;) {
            while (x < L.width && pixels[y * L.width + x] == kStartKeyColor) x++;
            int start = x;
            while (x < L.width && pixels[y * L.width + x] != kStartKeyColor) x++;
            if (x > start) {
                HRGN run = CreateRectRgn(start, y, x, y + 1);
                CombineRgn(shape, shape, run, RGN_OR);
                DeleteObject(run);
            }
        }
    }
    SetWindowRgn(wnd, shape, TRUE);  // the window owns it now
}

// The search box's state: focused while typing is possible (it always is
// while the menu's open), hot under the pointer, else normal.
int StartSearchState() {
    // StartAllBack's shows its box while it has the focus, and nothing otherwise.
    // (Its own menu is always focused there, caret and all: also while ours is the front window.)
    if (g_startEdit && (GetFocus() == g_startEdit || (g_startWnd && GetForegroundWindow() == g_startWnd))) return 4;  // SBB_FOCUSED
    POINT pt;
    GetCursorPos(&pt);
    if (g_startWnd) ScreenToClient(g_startWnd, &pt);
    return PtInRect(&g_sly.search, pt) ? 2 : 1;  // SBB_HOT, else SBB_NORMAL (nothing drawn)
}

// The search box's ground as a brush, for the typing to sit on.
HBRUSH StartSearchBrush(int state) {
    auto const& L = g_sly;
    int w = L.search.right - L.search.left, h = L.search.bottom - L.search.top;
    static SIZE made;
    if (g_sl.searchBrush && g_sl.searchBrushState == state && made.cx == w && made.cy == h) return g_sl.searchBrush;
    if (g_sl.searchBrush) DeleteObject(g_sl.searchBrush);
    g_sl.searchBrush = nullptr;
    made = {w, h};
    if (w <= 0 || h <= 0) return nullptr;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, w, h);
    HGDIOBJ old = SelectObject(dc, bitmap);
    RECT r{0, 0, w, h};
    Fill(dc, r, g_sl.ground);
    if (g_sl.searchBox) DrawThemeBackground(g_sl.searchBox, dc, 1, state, &r, nullptr);
    else Fill(dc, r, g_sl.box);
    SelectObject(dc, old);
    g_sl.searchBrush = CreatePatternBrush(bitmap);
    g_sl.searchBrushState = state;
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    return g_sl.searchBrush;
}

HBITMAP g_paintBitmap = nullptr;  // the Start menu's back buffer, kept between paints
SIZE g_paintSize{};

// The area an item is painted in (nothing for no item, the whole menu for what isn't a row or place).
RECT StartHitRect(HWND wnd, int hit) {
    if (hit < 0) return RECT{};
    if (hit < (int)g_rows.size()) return StartRowRect(g_rows[hit]);
    if (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size() && hit - kSmPlace < (int)g_sly.places.size())
        return g_sly.places[hit - kSmPlace];
    if (IsPhoneHit(hit)) {
        RECT r = PhoneHitRect(g_sly.ply, hit);
        if (hit == kSmPhoneClose || hit == kSmPhoneDetach) r = g_sly.ply.head;  // (lit with their head)
        return r;
    }
    RECT all;
    GetClientRect(wnd, &all);
    return all;
}

// A small battery, filled to its level: the text's colour, or red when low.
void PhoneBattery(HDC dc, RECT r, int percent) {
    uint32_t edge = g_sl.text, fill = percent >= 0 && percent <= 20 ? 0xD13438 : g_sl.text;
    RECT body{r.left, r.top, r.right - Px(2), r.bottom};
    StartFrame(dc, body, edge);
    Fill(dc, RECT{body.right, r.top + (r.bottom - r.top) / 3, r.right, r.bottom - (r.bottom - r.top) / 3}, edge);
    if (percent > 0) {
        RECT inside{body.left + Px(2), body.top + Px(2), body.right - Px(2), body.bottom - Px(2)};
        inside.right = inside.left + std::max(Px(1), (int)((inside.right - inside.left) * std::min(percent, 100) / 100));
        Fill(dc, inside, fill);
    }
}

// An icon by its setting's form: one of System Icons' designs (tourne:name, in the tray's colours), a file,
// or "file,index"; at a size, kept until the settings change (ForgetPhoneIcons).
std::map<std::wstring, HICON> g_phoneIcons;

void ForgetPhoneIcons() {
    for (auto& [key, icon] : g_phoneIcons)
        if (icon) DestroyIcon(icon);
    g_phoneIcons.clear();
    ForgetPhoneAppIcons();
}

HICON PhoneSpecIcon(std::wstring const& spec, int size) {
    if (spec.empty()) return nullptr;
    std::wstring key = spec + L"|" + std::to_wstring(size);
    auto it = g_phoneIcons.find(key);
    if (it != g_phoneIcons.end()) return it->second;
    return g_phoneIcons[key] = IsTourneDesign(spec) ? part_system_icons::DesignIconAt(spec, size) : StartIconSpec(spec, size);
}

// The tray's own designs for each, so the phone is themed as the tray is (System Icons draws them).
constexpr PCWSTR kPhoneDesign = L"tourne:phone";
constexpr PCWSTR kPhoneActionDesigns[] = {L"tourne:chat", L"tourne:bell", L"tourne:cloud-sync", L"tourne:system"};

// The phone's icon: yours, else the tray's phone, else Phone Link's.
HICON PhoneOwnIcon(int size) {
    if (HICON own = PhoneSpecIcon(g_startConfig.phoneIcon, size)) return own;
    if (HICON design = PhoneSpecIcon(kPhoneDesign, size)) return design;
    return StartIcon(kPhoneLinkApp, size);
}

// A link's icon: yours, else the tray's design for it (Notifications' lit while there are unread), else its
// phone app's, else the phone's.
HICON PhoneActionIcon(int i, int size) {
    if (HICON own = PhoneSpecIcon(g_startConfig.phoneActionIcons[i], size)) return own;
    PCWSTR design = i == 1 && g_phoneView.unread > 0 ? L"tourne:bell-unread" : kPhoneActionDesigns[i];
    if (HICON themed = PhoneSpecIcon(design, size)) return themed;
    if (*kPhoneActions[i].package)
        if (HICON app = PhoneAppIcon(kPhoneActions[i].package, size)) return app;
    return PhoneOwnIcon(size);
}

// The tray's battery design for a level; null without System Icons (the drawn one is used then).
HICON PhoneBatteryIcon(int percent, int size) {
    PCWSTR design = percent >= 90 ? L"tourne:battery-full" : percent >= 60 ? L"tourne:battery-high"
                  : percent >= 30 ? L"tourne:battery-mid" : L"tourne:battery-low";
    return PhoneSpecIcon(design, size);
}

// The tray's design for a phone app, drawn in the theme's colours: the same one the notification centre gives
// its notifications (DesignForApp in system-icons-host.cpp; keep the two lists in step), else the phone's.
PCWSTR PhoneAppDesign(PhoneNote const& note) {
    std::wstring text = PhoneLower(note.package + L" " + note.app);
    auto has = [&](std::initializer_list<PCWSTR> words) {
        for (PCWSTR w : words)
            if (text.find(w) != std::wstring::npos) return true;
        return false;
    };
    if (has({L"mail", L"outlook", L"proton"})) return L"tourne:app-mail";
    if (has({L"reddit"})) return L"tourne:app-reddit";
    if (has({L"youtube", L"tiktok", L"twitch", L"netflix", L"video"})) return L"tourne:app-video";
    if (has({L"amazon", L"ebay", L"etsy", L"shop", L"walmart", L"costco", L"target"})) return L"tourne:app-shop";
    if (has({L"instagram", L"snapchat", L"camera", L"photos"})) return L"tourne:app-capture";
    if (has({L"spotify"})) return L"tourne:app-spotify";
    if (has({L"music", L"podcast", L"audible"})) return L"tourne:app-audio";
    if (has({L"mobilesms", L"messages", L"whatsapp", L"discord", L"messenger", L"telegram", L"signal", L"slack",
             L"teams", L"chat"})) {
        return L"tourne:app-chat";
    }
    return kPhoneDesign;
}

// A notification's: its phone app's own logo (startMenu.phoneAppIcons), else its design in the theme's colours,
// as the notification centre shows it, else Notifications' icon.
HICON PhoneNoteIcon(PhoneNote const& note, int size) {
    if (g_startConfig.phoneAppIcons && !note.package.empty())
        if (HICON app = PhoneAppIcon(note.package, size)) return app;
    if (HICON themed = PhoneSpecIcon(PhoneAppDesign(note), size)) return themed;
    return PhoneActionIcon(1, size);
}

// The corner buttons, drawn in the text's colours: Close (an X), Apart / Back into Start (two panes), Keep
// open (a pin, filled while on).
void PhoneButton(HDC dc, RECT r, int kind, bool hot, bool pressed, bool on) {
    if (IsRectEmpty(&r)) return;
    if (hot && !SpDraw(dc, kSpItem, pressed ? 3 : 2, r)) Fill(dc, r, g_sl.hover);
    uint32_t c = hot ? g_sl.hotText : g_sl.dim;
    int cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2, s = Px(4);
    HPEN pen = CreatePen(PS_SOLID, std::max(1, Px(1)), PaletteRef(c));
    HGDIOBJ oldPen = SelectObject(dc, pen), oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    if (kind == kSmPhoneClose) {
        MoveToEx(dc, cx - s, cy - s, nullptr);
        LineTo(dc, cx + s + 1, cy + s + 1);
        MoveToEx(dc, cx + s, cy - s, nullptr);
        LineTo(dc, cx - s - 1, cy + s + 1);
    } else if (kind == kSmPhoneDetach) {
        StartFrame(dc, RECT{cx - s - Px(1), cy - s + Px(2), cx + s - Px(2), cy + s + Px(1)}, c);
        StartFrame(dc, RECT{cx - s + Px(2), cy - s - Px(1), cx + s + Px(1), cy + s - Px(2)}, c);
    } else if (kind == kSmPhonePin) {
        if (on) {
            SelectObject(dc, GetStockObject(DC_BRUSH));
            SetDCBrushColor(dc, PaletteRef(c));
        }
        Ellipse(dc, cx - s + Px(1), cy - s, cx + s, cy + Px(1));
        MoveToEx(dc, cx, cy + Px(1), nullptr);
        LineTo(dc, cx, cy + s + Px(2));
    }
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
}

////////////////////////////////////////////////////////////////////////////////
// The phone's texts, when Tourne holds them (phone-map.cpp): the conversations, and one open with a reply box.

// A text's time: today's as the clock has it, older ones by day.
std::wstring PhoneWhen(long long when) {
    FILETIME utc{(DWORD)when, (DWORD)(when >> 32)}, local;
    SYSTEMTIME t, now;
    if (!when || !FileTimeToLocalFileTime(&utc, &local) || !FileTimeToSystemTime(&local, &t)) return {};
    GetLocalTime(&now);
    WCHAR out[64] = L"";
    if (t.wYear == now.wYear && t.wMonth == now.wMonth && t.wDay == now.wDay)
        GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, &t, nullptr, out, ARRAYSIZE(out));
    else
        GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &t, L"MMM d", out, ARRAYSIZE(out), nullptr);
    return out;
}

// The texts by who they're with, newest first: each with its last text and how many are new.
void PhoneBuildConvos() {
    g_phoneConvos.clear();
    std::map<std::wstring, size_t> at;
    for (auto const& t : g_phoneTexts) {  // (oldest first)
        std::wstring key = phonemap::Key(t.address);
        auto it = at.find(key);
        if (it == at.end()) {
            it = at.emplace(key, g_phoneConvos.size()).first;
            g_phoneConvos.push_back({key});
        }
        PhoneConvo& c = g_phoneConvos[it->second];
        if (t.kind == L'i' || c.address.empty()) c.address = t.address;  // (answered as they wrote from)
        if (!t.name.empty() && (t.kind == L'i' || c.name.empty())) c.name = t.name;
        c.last = t.body;
        for (auto& ch : c.last)
            if (ch == L'\n' || ch == L'\r') ch = L' ';
        c.when = t.when;
        c.kind = t.kind;
        if (t.kind == L'i' && !t.read) c.unread++;
    }
    for (auto& c : g_phoneConvos)
        if (c.name.empty()) c.name = c.address;
    // Cleared ones (a middle click, or Clear) stay out until a newer text comes in them.
    auto cleared = PhoneClearedConvos();
    std::erase_if(g_phoneConvos, [&](PhoneConvo const& c) {
        auto it = cleared.find(c.key);
        return it != cleared.end() && c.when <= it->second && c.key != g_phoneChat;
    });
    std::stable_sort(g_phoneConvos.begin(), g_phoneConvos.end(), [](auto const& a, auto const& b) { return a.when > b.when; });
}

void PhoneRepaint();

// A conversation cleared from the list (it comes back with its next text).
void PhoneClearConvoAt(int index) {
    if (index < 0 || index >= (int)g_phoneConvos.size()) return;
    PhoneClearConvo(g_phoneConvos[index].key, g_phoneConvos[index].when);
    g_phoneConvos.erase(g_phoneConvos.begin() + index);
    PhoneRepaint();
}

PhoneConvo const* PhoneChatConvo() {
    for (auto const& c : g_phoneConvos)
        if (c.key == g_phoneChat) return &c;
    return nullptr;
}

// What the heading says on its right: why there are none, or what's under way.
std::wstring PhoneLinkWords() {
    switch (g_phoneLink) {
        case phonemap::Link::PhoneLink: return L"Phone Link has them";
        case phonemap::Link::Away: return L"Not connected";
        case phonemap::Link::Connecting: return L"Connecting...";
        case phonemap::Link::NoPhone: return L"Not paired";
        default: return {};
    }
}

// The open conversation's latest texts, newest at the bottom, as bubbles: theirs on the left, yours on the right
// in the accent's tint, with "Sending..." or "Not sent" under one that hasn't gone.
// How far the open conversation is scrolled up from its newest text (the wheel over it), and how far it can go.
int g_phoneThreadScroll = 0, g_phoneThreadMax = 0;

void PaintPhoneThread(HDC dc, RECT area, int lineH) {
    int saved = SaveDC(dc);
    IntersectClipRect(dc, area.left, area.top, area.right, area.bottom);
    int pad = Px(6), left = area.left + Px(12), right = area.right - g_sl.frameBorder - Px(10);
    int widest = std::max(Px(60), (right - left) * 4 / 5);
    g_phoneThreadScroll = std::clamp(g_phoneThreadScroll, 0, g_phoneThreadMax);
    int bottom = area.bottom - Px(4), y = bottom + g_phoneThreadScroll;
    bool any = false;
    // Newest at the bottom, upward from there; all of them measured (for how far it scrolls), the ones in sight drawn.
    for (int i = (int)g_phoneTexts.size() - 1; i >= 0; i--) {
        auto const& t = g_phoneTexts[i];
        if (phonemap::Key(t.address) != g_phoneChat) continue;
        any = true;
        bool mine = t.kind != L'i';
        PCWSTR note = t.kind == L'f' ? L"Not sent" : t.kind == L'p' ? L"Sending..." : nullptr;
        if (note) {
            y -= lineH;
            StartText(dc, note, RECT{left, y, right, y + lineH}, t.kind == L'f' ? 0xD13438 : g_sl.dim, mine ? DT_RIGHT : DT_LEFT);
        }
        std::wstring body = PlainSymbols(t.body);
        RECT size{0, 0, widest - pad * 2, 0};
        DrawTextW(dc, body.c_str(), -1, &size, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL | DT_NOPREFIX);
        int w = size.right + pad * 2, h = size.bottom + pad;
        y -= h;
        RECT bubble{mine ? right - w : left, y, mine ? right : left + w, y + h};
        if (bubble.bottom > area.top && bubble.top < area.bottom) {
            Fill(dc, bubble, mine ? StartMix(g_sl.ground, g_sl.accent, 0.3) : StartMix(g_sl.ground, g_sl.text, 0.1));
            RECT inside{bubble.left + pad, bubble.top + pad / 2, bubble.right - pad, bubble.bottom};
            SetTextColor(dc, PaletteRef(g_sl.text));
            DrawTextW(dc, body.c_str(), -1, &inside, DT_WORDBREAK | DT_EDITCONTROL | DT_NOPREFIX);
        }
        y -= Px(4);
    }
    g_phoneThreadMax = std::max(0, (int)(area.top + Px(4)) - (y - g_phoneThreadScroll));
    if (!any) StartText(dc, L"Nothing yet", RECT{left, area.top, right, area.top + lineH + Px(8)}, g_sl.dim);
    RestoreDC(dc, saved);
}

// Why the list is empty, in its first row.
std::wstring PhoneTextsEmptyWords() {
    switch (g_phoneLink) {
        case phonemap::Link::PhoneLink: return L"Close Phone Link to have them here";
        case phonemap::Link::Ready: return L"Texts that come show here";
        case phonemap::Link::NoPhone: return L"Pair your phone in Bluetooth settings";
        default: return L"They show here once your phone connects";
    }
}

void PaintPhoneTexts(HDC dc, PhoneLayout const& P, int hot, int pressed, int lineH) {
    RECT h = P.textsHead;
    int right = h.right - g_sl.frameBorder - Px(8);
    std::wstring words = PhoneLinkWords();
    if (g_phoneChat.empty()) {
        int unread = 0;
        for (auto const& c : g_phoneConvos) unread += c.unread;
        std::wstring heading = L"Messages";
        if (unread > 0) heading += L"  ·  " + std::to_wstring(unread) + L" new";
        StartText(dc, heading, RECT{h.left + Px(12), h.top, right, h.bottom}, g_sl.accent);
        if (!words.empty()) StartText(dc, words, RECT{h.left + Px(12), h.top, right, h.bottom}, g_sl.dim, DT_RIGHT);
        int small = Px(24);
        for (int i = 0; i < (int)P.convos.size(); i++) {
            RECT r = P.convos[i];
            if (i >= (int)g_phoneConvos.size()) {
                if (i == 0) StartText(dc, PhoneTextsEmptyWords(), RECT{r.left + Px(12), r.top, r.right - Px(8), r.top + lineH + Px(8)}, g_sl.dim);
                continue;
            }
            auto const& c = g_phoneConvos[i];
            bool on = hot == kSmPhoneConvo + i;
            if (on) StartHighlight(dc, r, pressed == kSmPhoneConvo + i);
            if (HICON icon = PhoneActionIcon(0, small)) DrawIconEx(dc, r.left + Px(12), r.top + Px(5), icon, small, small, 0, nullptr, DI_NORMAL);
            int nx = r.left + Px(12) + small + Px(8), nr = r.right - g_sl.frameBorder - Px(8);
            int ny = r.top + (r.bottom - r.top - lineH * 2) / 2;
            std::wstring when = StartCase(PhoneWhen(c.when));
            SIZE ws{};
            GetTextExtentPoint32W(dc, when.c_str(), (int)when.size(), &ws);
            StartText(dc, PlainSymbols(c.name), RECT{nx, ny, nr - ws.cx - Px(6), ny + lineH}, g_sl.accent);
            StartText(dc, when, RECT{nx, ny, nr, ny + lineH}, g_sl.dim, DT_RIGHT);
            int lineRight = nr;
            if (c.unread) {
                // A dot for new ones, at the line's end.
                int d = Px(7), dx = nr - d, dy = ny + lineH + (lineH - d) / 2;
                HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(DC_BRUSH)), oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
                SetDCBrushColor(dc, PaletteRef(g_sl.accent));
                Ellipse(dc, dx, dy, dx + d + 1, dy + d + 1);
                SelectObject(dc, oldBrush);
                SelectObject(dc, oldPen);
                lineRight -= d + Px(6);
            }
            std::wstring line = (c.kind == L'f' ? L"Not sent: " : c.kind != L'i' ? L"You: " : L"") + PlainSymbols(c.last);
            StartText(dc, line, RECT{nx, ny + lineH, lineRight, ny + lineH * 2}, on ? g_sl.hotText : c.unread ? g_sl.text : g_sl.dim);
        }
        return;
    }
    // A conversation: its name (a click goes back to the list), its texts, the reply box and Send.
    bool on = hot == kSmPhoneTexts;
    if (on) StartHighlight(dc, h, pressed == kSmPhoneTexts);
    PhoneConvo const* c = PhoneChatConvo();
    SIZE ws{};
    std::wstring shownWords = StartCase(words);
    GetTextExtentPoint32W(dc, shownWords.c_str(), (int)shownWords.size(), &ws);
    StartText(dc, L"‹  " + PlainSymbols(c ? c->name : g_phoneChat), RECT{h.left + Px(12), h.top, right - ws.cx - Px(6), h.bottom},
              on ? g_sl.hotText : g_sl.accent);
    if (!words.empty()) StartText(dc, words, RECT{h.left + Px(12), h.top, right, h.bottom}, g_sl.dim, DT_RIGHT);
    PaintPhoneThread(dc, P.thread, lineH);
    RECT box{P.reply.left, P.reply.top, P.send.left - Px(4), P.reply.bottom};
    Fill(dc, box, g_sl.box);
    StartFrame(dc, box, g_sl.edge);
    bool ready = g_phoneLink == phonemap::Link::Ready, sendOn = hot == kSmPhoneSend && ready;
    if (sendOn) StartHighlight(dc, P.send, pressed == kSmPhoneSend);
    StartText(dc, L"Send", P.send, !ready ? g_sl.dim : sendOn ? g_sl.hotText : g_sl.accent, DT_CENTER);
}

void PhoneSendReply();
void PhoneCloseChat();

LRESULT CALLBACK PhoneReplyProc(HWND edit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
                PhoneSendReply();
                return 0;
            }
            if (wParam == VK_ESCAPE) {
                PhoneCloseChat();
                return 0;
            }
            break;
        case WM_CHAR:
            if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == L'\t') return 0;
            break;
        case WM_PAINT: {
            LRESULT result = DefSubclassProc(edit, msg, wParam, lParam);
            if (!GetWindowTextLengthW(edit)) {
                // Its prompt, dim, while nothing's typed.
                HDC dc = GetDC(edit);
                HGDIOBJ old = SelectObject(dc, (HFONT)SendMessageW(edit, WM_GETFONT, 0, 0));
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, PaletteRef(g_sl.dim));
                RECT r;
                GetClientRect(edit, &r);
                r.left += 2;
                bool ready = g_phoneLink == phonemap::Link::Ready;
                std::wstring cue = StartCase(ready ? L"Reply" : L"Reply when your phone connects");
                DrawTextW(dc, cue.c_str(), -1, &r, DT_SINGLELINE | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS);
                SelectObject(dc, old);
                ReleaseDC(edit, dc);
            }
            return result;
        }
        case WM_NCDESTROY:
            RemoveWindowSubclass(edit, PhoneReplyProc, 0);
            if (g_phoneReply == edit) g_phoneReply = nullptr;
            break;
    }
    return DefSubclassProc(edit, msg, wParam, lParam);
}

// The reply box's colours: the typing on the box's ground.
LRESULT PhoneReplyColors(HDC dc) {
    SetTextColor(dc, PaletteRef(g_sl.text));
    SetBkColor(dc, PaletteRef(g_sl.box));
    SetDCBrushColor(dc, PaletteRef(g_sl.box));
    return (LRESULT)GetStockObject(DC_BRUSH);
}

// The reply box where the layout has it, in host (the window the phone is painted in), or out of sight when
// there's no conversation open there. What's typed goes with it from window to window.
void PhoneSyncReply(HWND host, PhoneLayout const& P) {
    bool want = !g_phoneChat.empty() && !IsRectEmpty(&P.reply);
    if (!want) {
        if (g_phoneReply && GetParent(g_phoneReply) == host && IsWindowVisible(g_phoneReply)) ShowWindow(g_phoneReply, SW_HIDE);
        return;
    }
    std::wstring typed;
    if (g_phoneReply && GetParent(g_phoneReply) != host) {
        int n = GetWindowTextLengthW(g_phoneReply);
        typed.resize(n);
        if (n) GetWindowTextW(g_phoneReply, typed.data(), n + 1);
        DestroyWindow(g_phoneReply);
        g_phoneReply = nullptr;
    }
    if (!g_phoneReply) {
        g_phoneReply = CreateWindowExW(0, L"EDIT", typed.c_str(), WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, host, nullptr, g_instance, nullptr);
        if (!g_phoneReply) return;
        SetWindowSubclass(g_phoneReply, PhoneReplyProc, 0, 0);
        SendMessageW(g_phoneReply, EM_LIMITTEXT, 1000, 0);
    }
    SendMessageW(g_phoneReply, WM_SETFONT, (WPARAM)StartFont(), FALSE);
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = SelectObject(dc, StartFont());
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    RECT box{P.reply.left + Px(5), P.reply.top, P.send.left - Px(9), P.reply.bottom};
    int h = tm.tmHeight, y = (box.top + box.bottom - h) / 2;
    RECT now{};
    GetWindowRect(g_phoneReply, &now);
    MapWindowPoints(nullptr, host, (POINT*)&now, 2);
    if (now.left != box.left || now.top != y || now.right - now.left != box.right - box.left || now.bottom - now.top != h)
        SetWindowPos(g_phoneReply, HWND_TOP, box.left, y, box.right - box.left, h, SWP_NOACTIVATE);
    if (!IsWindowVisible(g_phoneReply)) ShowWindow(g_phoneReply, SW_SHOWNA);
}

// The phone: itself (Phone Link opens on a click), its links, and its latest notifications, drawn as the
// places are, in the Start menu's own parts, colours and font; in the Start menu's pane or its own window.
void PaintPhone(HDC dc, PhoneLayout const& P, int hot, int pressed, bool apart) {
    auto const& phone = g_phoneView;
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    int lineH = tm.tmHeight;
    if (apart) {
        // The title strip it's moved by.
        StartText(dc, L"Phone", RECT{P.strip.left + Px(12), P.strip.top, P.pin.left - Px(4), P.strip.bottom}, g_sl.dim);
        PhoneButton(dc, P.pin, kSmPhonePin, hot == kSmPhonePin, pressed == kSmPhonePin, g_phoneState.pinned);
    } else {
        // A line down between the places and the phone.
        Fill(dc, RECT{P.head.left, P.head.top, P.head.left + 1, P.bottom}, StartMix(g_sl.ground, g_sl.text, 0.25));
    }

    // The phone itself.
    RECT h = P.head;
    bool headHot = hot == kSmPhoneHead || hot == kSmPhoneClose || hot == kSmPhoneDetach;
    if (headHot) StartHighlight(dc, h, pressed == kSmPhoneHead);
    int big = Px(32), x = h.left + Px(12);
    if (HICON icon = PhoneOwnIcon(big)) DrawIconEx(dc, x, h.top + Px(6), icon, big, big, 0, nullptr, DI_NORMAL);
    int tx = x + big + Px(10), right = h.right - g_sl.frameBorder - Px(8);
    int nameRight = apart ? right : P.detach.left - Px(4);
    int y = h.top + Px(4);
    std::wstring name = phone.name.empty() ? L"Your phone" : phone.name;
    StartText(dc, name, RECT{tx, y, nameRight, y + lineH}, headHot ? g_sl.hotText : g_sl.text);
    y += lineH + Px(1);
    StartText(dc, phone.os.empty() ? L"Phone Link" : phone.os, RECT{tx, y, right, y + lineH}, g_sl.dim);
    y += lineH + Px(2);
    int bx = tx;
    if (phone.battery >= 0) {
        int b = Px(16);
        if (HICON battery = PhoneBatteryIcon(phone.battery, b)) {
            DrawIconEx(dc, bx, y + (lineH - b) / 2, battery, b, b, 0, nullptr, DI_NORMAL);
            bx += b + Px(4);
        } else {
            int bh = std::max(Px(9), lineH * 2 / 3);
            PhoneBattery(dc, RECT{bx, y + (lineH - bh) / 2, bx + bh * 2 + Px(2), y + (lineH - bh) / 2 + bh}, phone.battery);
            bx += bh * 2 + Px(8);
        }
        std::wstring pct = std::to_wstring(phone.battery) + L"%";
        StartText(dc, pct, RECT{bx, y, right, y + lineH}, g_sl.text);
        SIZE size{};
        std::wstring shown = StartCase(pct);
        GetTextExtentPoint32W(dc, shown.c_str(), (int)shown.size(), &size);
        bx += size.cx + Px(10);
    }
    StartText(dc, phone.connected ? L"Connected" : phone.read ? L"Not connected" : L"", RECT{bx, y, right, y + lineH},
              phone.connected ? g_sl.accent : g_sl.dim);
    PhoneButton(dc, P.detach, kSmPhoneDetach, hot == kSmPhoneDetach, pressed == kSmPhoneDetach, false);
    PhoneButton(dc, P.close, kSmPhoneClose, hot == kSmPhoneClose, pressed == kSmPhoneClose, false);

    for (auto const& sep : P.seps) StartSeparator(dc, kSpPlaceSeparator, RECT{sep.left - Px(4), sep.top, sep.right - Px(4), sep.bottom});

    // Its links, as places with their icons.
    int glyph = Px(16);
    for (int i = 0; i < (int)P.actions.size(); i++) {
        RECT r = P.actions[i];
        if (IsRectEmpty(&r)) continue;
        bool on = hot == kSmPhoneAction + i;
        if (on) StartHighlight(dc, r, pressed == kSmPhoneAction + i);
        if (HICON icon = PhoneActionIcon(i, glyph)) DrawIconEx(dc, r.left + Px(12), (r.top + r.bottom - glyph) / 2, icon, glyph, glyph, 0, nullptr, DI_NORMAL);
        StartText(dc, kPhoneActions[i].name, RECT{r.left + Px(12) + glyph + Px(8), r.top, r.right - Px(8), r.bottom},
                  on ? g_sl.hotText : g_sl.text);
    }

    if (!IsRectEmpty(&P.textsHead)) PaintPhoneTexts(dc, P, hot, pressed, lineH);

    // The latest notifications, each as its phone app with its own icon.
    if (P.notes.empty()) return;
    std::wstring heading = L"Notifications";
    if (phone.unread > 0) heading += L"  ·  " + std::to_wstring(phone.unread) + L" new";
    StartText(dc, heading, RECT{P.heading.left + Px(12), P.heading.top, P.heading.right - Px(8), P.heading.bottom}, g_sl.accent);
    int small = Px(24);
    for (int i = 0; i < (int)P.notes.size(); i++) {
        RECT r = P.notes[i];
        if (i >= (int)phone.notes.size()) {
            // None stored: Phone Link keeps them in its own window until it posts them.
            if (i == 0 && phone.read)
                StartText(dc, phone.unread > 0 ? L"Open Notifications to see them" : L"Nothing new from your phone",
                          RECT{r.left + Px(12), r.top, r.right - Px(8), r.top + lineH + Px(8)}, g_sl.dim);
            continue;
        }
        auto const& note = phone.notes[i];
        bool on = hot == kSmPhoneNote + i;
        if (on) StartHighlight(dc, r, pressed == kSmPhoneNote + i);
        if (HICON icon = PhoneNoteIcon(note, small)) DrawIconEx(dc, r.left + Px(12), r.top + Px(5), icon, small, small, 0, nullptr, DI_NORMAL);
        int nx = r.left + Px(12) + small + Px(8), nr = r.right - g_sl.frameBorder - Px(8);
        int ny = r.top + (r.bottom - r.top - lineH * 2) / 2;
        StartText(dc, note.app, RECT{nx, ny, nr, ny + lineH}, g_sl.accent);
        std::wstring line = note.title;
        if (!note.body.empty()) line += (line.empty() ? L"" : L": ") + note.body;
        StartText(dc, line, RECT{nx, ny + lineH, nr, ny + lineH * 2}, on ? g_sl.hotText : g_sl.text);
    }
}

void PaintStart(HWND wnd, HDC dc) {
    auto const& L = g_sly;
    auto const& p = g_settings.palette;
    Fill(dc, RECT{0, 0, L.width, L.height}, p.ground);
    StartBackground(dc);
    bool searching = g_startView == kViewSearch;

    SetBkMode(dc, TRANSPARENT);
    HGDIOBJ oldFont = SelectObject(dc, StartFont());
    int icon = StartIconPx();

    // The list, clipped to its area.
    int saved = SaveDC(dc);
    IntersectClipRect(dc, L.list.left, L.list.top, L.list.right, L.list.bottom);
    for (int i = 0; i < (int)g_rows.size(); i++) {
        auto const& row = g_rows[i];
        RECT r = StartRowRect(row);
        if (r.bottom <= L.list.top || r.top >= L.list.bottom) continue;
        if (row.kind == kRowSeparator) {
            StartSeparator(dc, kSpSeparator, r);
            continue;
        }
        if (row.kind == kRowHeader) {
            StartText(dc, row.name, RECT{r.left + Px(24), r.top, r.right - Px(8), r.bottom},
                      row.parsing.empty() && g_startView == kViewSearch ? g_sl.text : g_sl.accent);
            continue;
        }
        bool hot = g_smHot == i;
        if (hot) StartHighlight(dc, r, g_smPressed == i);
        int x = r.left + Px(12) + row.depth * Px(13);
        std::wstring const& iconOf = row.kind == kRowFolder && row.node && !row.node->dirs.empty() ? row.node->dirs[0]
                                                                                                   : row.parsing;
        if (HICON h = StartIcon(iconOf, icon)) DrawIconEx(dc, x, (r.top + r.bottom - icon) / 2, h, icon, icon, 0, nullptr, DI_NORMAL);
        StartText(dc, row.name, RECT{x + icon + Px(3), r.top, r.right - Px(8), r.bottom}, hot ? g_sl.hotText : g_sl.text);
    }
    if (StartScrolls()) {
        StartScrollBar b = StartScrollGeometry();
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(wnd, &pt);
        bool overBar = g_smHot == kSmScroll || g_scrollDrag;
        bool pressed = g_smPressed == kSmScroll && (GetKeyState(VK_LBUTTON) < 0);
        auto state = [&](RECT const& r, int normal) {
            bool on = PtInRect(&r, pt) != FALSE;
            return normal + (on && pressed ? 2 : on ? 1 : overBar ? 3 : 0);
        };
        if (g_sl.scroll) {
            RECT above{b.track.left, b.track.top, b.track.right, b.thumb.top};
            RECT below{b.track.left, b.thumb.bottom, b.track.right, b.track.bottom};
            DrawThemeBackground(g_sl.scroll, dc, SBP_UPPERTRACKVERT, 1, &above, nullptr);
            DrawThemeBackground(g_sl.scroll, dc, SBP_LOWERTRACKVERT, 1, &below, nullptr);
            // Arrow states: 1-4 up (normal, hot, pressed, disabled), 5-8
            // down; the pointer over the bar ("hover", 17 and 18) where defined.
            auto arrow = [&](RECT const& r, int normal, int hover) {
                int s = state(r, normal);
                if (s == normal + 3) s = IsThemePartDefined(g_sl.scroll, SBP_ARROWBTN, hover) ? hover : normal;
                DrawThemeBackground(g_sl.scroll, dc, SBP_ARROWBTN, s, &r, nullptr);
            };
            arrow(b.up, ABS_UPNORMAL, ABS_UPHOVER);
            arrow(b.down, ABS_DOWNNORMAL, ABS_DOWNHOVER);
            int thumb = g_scrollDrag ? SCRBS_PRESSED : PtInRect(&b.thumb, pt) ? SCRBS_HOT : overBar ? SCRBS_HOVER : SCRBS_NORMAL;
            DrawThemeBackground(g_sl.scroll, dc, SBP_THUMBBTNVERT, thumb, &b.thumb, nullptr);
            DrawThemeBackground(g_sl.scroll, dc, SBP_GRIPPERVERT, thumb, &b.thumb, nullptr);
        } else {
            Fill(dc, b.thumb, overBar ? p.accent : p.dim);
            StartTriangle(dc, b.up, 3, p.dim);
            StartTriangle(dc, b.down, 2, p.dim);
        }
    }
    RestoreDC(dc, saved);

    // All Programs, Back, or more results; the arrow where an icon would be.
    RECT footer = L.footer;
    bool footerHot = g_smHot == kSmFooter || g_flyFooter;
    if (footerHot) StartHighlight(dc, footer, g_smPressed == kSmFooter);
    uint32_t footerText = footerHot ? g_sl.hotText : g_sl.text;
    RECT arrowSlot{footer.left + Px(12), footer.top, footer.left + Px(12) + icon, footer.bottom};
    int textLeft = arrowSlot.right + Px(3);
    if (g_startView == kViewHome) {
        StartArrow(dc, kSpAllArrow, arrowSlot, 0, g_sl.accent);
        StartText(dc, L"All Programs", RECT{textLeft, footer.top, footer.right - Px(8), footer.bottom}, footerText);
    } else if (g_startView == kViewAll) {
        StartArrow(dc, kSpBackArrow, arrowSlot, 1, g_sl.accent);
        StartText(dc, L"Back", RECT{textLeft, footer.top, footer.right - Px(8), footer.bottom}, footerText);
    } else {
        // Windows' own search icon beside it.
        static HICON find;
        static int findSize;
        if (!find || findSize != icon) {
            if (find) DestroyIcon(find);
            SHSTOCKICONINFO info{sizeof(info)};
            find = SUCCEEDED(SHGetStockIconInfo(SIID_FIND, SHGSI_ICONLOCATION, &info))
                       ? ExtractAt(info.szPath, info.iIcon, icon)
                       : nullptr;
            findSize = icon;
        }
        HICON shown = StartSearchIcon(icon);  // yours, when set
        if (!shown) shown = find;
        if (shown) DrawIconEx(dc, arrowSlot.left, (footer.top + footer.bottom - icon) / 2, shown, icon, icon, 0, nullptr, DI_NORMAL);
        StartText(dc, L"See more results", RECT{textLeft, footer.top, footer.right - Px(8), footer.bottom}, footerText);
    }

    // The places (not while searching: the results have the panel).
    for (int i = 0; i < (int)g_places.size() && !searching; i++) {
        auto const& place = g_places[i];
        RECT r = L.places[i];
        if (place.special == 3) {
            StartSeparator(dc, kSpPlaceSeparator, RECT{r.left - Px(4), r.top, r.right - Px(4), r.bottom});  // StartAllBack's line starts 4 px further left
            continue;
        }
        bool hot = g_smHot == kSmPlace + i || g_flyPlace == i;
        if (hot) StartHighlight(dc, r, g_smPressed == kSmPlace + i);
        int arrow = place.kind == kKindLink ? 0 : Px(16);
        int textLeft = r.left + Px(12);
        if (g_startConfig.glyphIcons) {
            // "Display glyph icons": each place's own small icon.
            int glyph = Px(16);
            if (HICON h = StartIcon(PlaceIconSource(place), glyph))
                DrawIconEx(dc, textLeft, (r.top + r.bottom - glyph) / 2, h, glyph, glyph, 0, nullptr, DI_NORMAL);
            textLeft += glyph + Px(6);
        }
        StartText(dc, place.name, RECT{textLeft, r.top, r.right - Px(8) - arrow, r.bottom},
                  hot ? g_sl.hotText : g_sl.text);
        int arrowRight = r.right - g_sl.frameBorder - Px(6);
        if (arrow) StartArrow(dc, kSpPlaceArrow, RECT{arrowRight - arrow, r.top, arrowRight, r.bottom}, 0, g_sl.text);
    }
    if (L.phone && !searching) PaintPhone(dc, L.ply, g_smHot, g_smPressed, false);
    if (!g_phoneWnd || !IsWindowVisible(g_phoneWnd)) PhoneSyncReply(wnd, L.phone && !searching ? L.ply : PhoneLayout{});

    // Search: Windows' visual style's search box, as StartAllBack's is (its
    // focused state underlined in the accent), and a magnifier.
    int searchState = StartSearchState();
    if (g_sl.searchBox) {
        DrawThemeBackground(g_sl.searchBox, dc, 1 /* SBP_SEARCHBOX */, searchState, &L.search, nullptr);
    } else {
        Fill(dc, L.search, g_sl.box);
        StartFrame(dc, L.search, g_sl.edge);
        if (searchState == 4)
            Fill(dc, RECT{L.search.left, L.search.bottom - Px(2), L.search.right, L.search.bottom}, g_sl.accent);
    }
    {
        // A magnifier; while searching, an X that clears it.
        int r = Px(4), cx = L.search.right - Px(14), cy = (L.search.top + L.search.bottom) / 2 - Px(1);
        HPEN pen = CreatePen(PS_SOLID, Px(2), PaletteRef(searching && g_smHot == kSmClear ? g_sl.hotText : g_sl.accent));
        HGDIOBJ oldPen = SelectObject(dc, pen), oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        if (searching) {
            MoveToEx(dc, cx - r, cy - r, nullptr);
            LineTo(dc, cx + r + 1, cy + r + 1);
            MoveToEx(dc, cx + r, cy - r, nullptr);
            LineTo(dc, cx - r - 1, cy + r + 1);
        } else if (HICON own = StartSearchIcon(Px(16))) {
            // Yours (startMenu.searchIcon), in the magnifier's place.
            DrawIconEx(dc, cx - Px(8), cy - Px(7), own, Px(16), Px(16), 0, nullptr, DI_NORMAL);
        } else {
            // StartAllBack's magnifier, pixel for pixel: a 1 px ring and a
            // handle down to the right.
            static constexpr char const* kGlass[] = {
                ".####....",
                "#....#...",
                "#.....#..",
                "#.....#..",
                "#.....#..",
                ".#...##..",
                "..####.#.",
                ".......#.",
                "........#",
            };
            int s = Px(1), left = cx - Px(4), top = cy - Px(4);
            for (int y = 0; y < 9; y++)
                for (int x = 0; x < 9; x++)
                    if (kGlass[y][x] == '#')
                        Fill(dc, RECT{left + x * s, top + y * s, left + (x + 1) * s, top + (y + 1) * s},
                             searching && g_smHot == kSmClear ? g_sl.hotText : g_sl.accent);
        }
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        DeleteObject(pen);
    }
    // Shut down: a dark frame holding the button and its arrow box, in the
    // style's split button colours (grey; hot, the accent with a light edge;
    // pressed, dark), as StartAllBack draws them.
    Fill(dc, L.shutdownFrame, g_sl.border ? g_sl.border : g_sl.edge);
    auto button = [&](RECT const& r, int hit) {
        bool hot = g_smHot == hit, pressed = hot && g_smPressed == hit;
        Fill(dc, r, pressed ? g_sl.buttonPressed : hot ? g_sl.buttonHot : g_sl.button);
        if (hot && !pressed) StartFrame(dc, RECT{r.left + Px(1), r.top + Px(1), r.right - Px(1), r.bottom - Px(1)}, g_sl.buttonHotEdge);
        return hot;
    };
    bool shutdownHot = button(L.shutdown, kSmShutdown);
    StartText(dc, PowerLabel(g_startConfig.powerButton), L.shutdown, shutdownHot ? g_sl.hotText : g_sl.text, DT_CENTER);
    // The arrow box: the button's own fill and, centred in it, the style's
    // arrow at its true size, as StartAllBack draws it. (Parts 23 and 24 are
    // 21x24 images with 5 px stretch margins: squeezed into the 18x16 box,
    // their arrow was pushed into a corner and cut off.) Lit, the arrow goes
    // light on the accent, as the style's own hot state has it.
    bool arrowHot = button(L.shutdownArrow, kSmShutdownArrow) && g_smPressed != kSmShutdownArrow;
    if (arrowHot) StartTriangle(dc, L.shutdownArrow, 0, g_sl.buttonHotEdge);
    else StartArrow(dc, kSpAllArrow, L.shutdownArrow, 0, g_sl.accent);
    SelectObject(dc, oldFont);
}

////////////////////////////////////////////////////////////////////////////////
// Menus: the places' flyouts, a program's commands, and the power menu

struct StartFlyFolder {
    HMENU menu;
    std::wstring target;
    bool filled;
};

struct StartFlyout {
    std::vector<std::wstring> targets;  // command - 1
    std::vector<HICON> icons;
    std::vector<StartFlyFolder> folders;  // submenus, filled as they open
    std::vector<JumpCategory> jump;
    std::vector<JumpItem const*> jumpItems;

    void Clear() {
        for (HICON icon : icons) DestroyIcon(icon);
        FreeJumpList(jump);
        targets.clear();
        icons.clear();
        folders.clear();
        jumpItems.clear();
    }
};
StartFlyout g_fly;
constexpr int kFlyJump = 0x4000;
constexpr size_t kFlyMaxItems = 400;

void FillFolderMenu(HMENU menu, std::wstring const& folder) {
    struct Entry {
        std::wstring name, parsing;
        bool folder;
    };
    std::vector<Entry> entries;
    ForEachShellChild(folder, [&](IShellItem* item) {
        SFGAOF attributes = 0;
        item->GetAttributes(SFGAO_FOLDER | SFGAO_STREAM | SFGAO_HIDDEN, &attributes);
        if (!(attributes & SFGAO_HIDDEN)) {
            std::wstring name = ItemName(item, SIGDN_NORMALDISPLAY), parsing = ItemName(item, SIGDN_DESKTOPABSOLUTEPARSING);
            if (!name.empty() && !parsing.empty())
                entries.push_back({name, parsing, (attributes & SFGAO_FOLDER) && !(attributes & SFGAO_STREAM)});
        }
        return entries.size() < kFlyMaxItems;
    });
    std::sort(entries.begin(), entries.end(), [](Entry const& a, Entry const& b) {
        if (a.folder != b.folder) return a.folder;
        return StartCompare(a.name, b.name) < 0;
    });
    int size = 16 * g_scale;
    // Multi-column flyouts: a new column where the screen runs out, not a
    // scrolling menu.
    int perColumn = std::max(8, (GetSystemMetrics(SM_CYMAXIMIZED) - Px(40)) / Px(24));
    int n = 0;
    for (auto const& e : entries) {
        HICON icon = g_menuLook.noIcons ? nullptr : ShellIcon(e.parsing, size);
        if (icon) g_fly.icons.push_back(icon);
        UINT column = g_startConfig.multiColumn && n && n % perColumn == 0 ? MF_MENUBARBREAK : 0;
        n++;
        if (e.folder) {
            HMENU sub = CreatePopupMenu();
            g_fly.folders.push_back({sub, e.parsing, false});
            AppendMenuIcon(menu, MF_POPUP | MF_STRING | column, (UINT_PTR)sub, e.name.c_str(), icon);
        } else {
            g_fly.targets.push_back(e.parsing);
            AppendMenuIcon(menu, MF_STRING | column, g_fly.targets.size(), e.name.c_str(), icon);
        }
    }
    if (entries.empty()) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, L"(Empty)");
}

void FillJumpMenu(HMENU menu, StartPlace const& place) {
    g_fly.targets.push_back(place.target);
    AppendMenuW(menu, MF_STRING, g_fly.targets.size(), place.name.c_str());
    g_fly.jump = ReadJumpList(place.appId, 16 * g_scale);
    for (auto const& category : g_fly.jump) {
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuHeader(menu, category.name.c_str());
        for (auto const& item : category.items) {
            if (item.separator) {
                AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
                continue;
            }
            AppendMenuIcon(menu, MF_STRING, kFlyJump + g_fly.jumpItems.size(), item.title.c_str(), item.icon);
            g_fly.jumpItems.push_back(&item);
        }
    }
}

int RunStartMenu(HWND wnd, HMENU menu, UINT flags, int x, int y) {
    g_menuLook = StartMenuLook();
    g_startInMenu = true;
    int cmd = (int)TrackStyledMenu(menu, TPM_RETURNCMD | flags, x, y, wnd);
    g_startInMenu = false;
    DestroyMenu(menu);
    return cmd;
}

void HideStart();

// The shell's own menu for something; what's chosen runs once it has closed.
void StartShellMenu(HWND wnd, std::wstring const& target) {
    if (target.empty()) return;
    g_menuLook = StartMenuLook();
    g_startInMenu = true;
    OpenShellMenu(wnd, target);
    g_startInMenu = false;
    if (g_shellMenu.command > 0) {
        HideStart();
        RunShellCommand(wnd);
    } else {
        CloseShellMenu();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Cascading menus: the places' menus and jump lists, and All Programs as a
// menu, as Windows 7's Start menu showed them (its menu bands, which
// StartAllBack keeps). Each is a window of its own chained off the Start
// menu that never takes the focus, so the Start menu stays open and keeps the
// keyboard; folders open theirs on a pause, and it's drawn with the visual
// style's Menu parts, as StartAllBack's are.

constexpr WCHAR kCascadeClass[] = L"TourneStartCascade_" WH_MOD_ID;
bool g_cascadeClass = false;

struct CascadeItem {
    std::wstring text, target;  // a parsing name, or a settings page
    HICON icon = nullptr;       // not owned: cached, or the jump list's
    ProgNode* node = nullptr;   // All Programs: its program or folder
    JumpItem const* jump = nullptr;
    bool folder = false, separator = false, header = false, disabled = false, iconTried = false;
    RECT rc{};  // in the window, before scrolling
};

struct Cascade {
    HWND wnd = nullptr;
    std::vector<CascadeItem> items;
    std::vector<JumpCategory> jump;
    int hot = -1, open = -1;
    bool scrolls = false, tracking = false;
    int scroll = 0, room = 0, content = 0, scrollDir = 0;
    RECT up{}, down{};  // the scroll arrows, when it scrolls
    HBITMAP back = nullptr;  // its back buffer, kept between paints
    SIZE backSize{};
    size_t iconsDone = 0;  // items whose icons are filled in so far (they follow the text, in the background)
};
std::vector<std::unique_ptr<Cascade>> g_cascades;
constexpr UINT_PTR kTimerCascadeOpen = 1, kTimerCascadeScroll = 2, kTimerCascadeIcons = 3;

int CascadeIconPx() {
    return g_menuLook.noIcons ? 0 : Px(g_startConfig.largeFlyoutIcons ? 32 : 16);
}

void CloseCascades(size_t from) {
    while (g_cascades.size() > from) {
        std::unique_ptr<Cascade> c = std::move(g_cascades.back());
        g_cascades.pop_back();
        if (c->wnd) {
            SetWindowLongPtrW(c->wnd, GWLP_USERDATA, 0);
            DestroyWindow(c->wnd);
        }
        if (c->back) DeleteObject(c->back);
        FreeJumpList(c->jump);
    }
    if (!g_cascades.empty()) {
        Cascade& last = *g_cascades.back();
        last.open = -1;
        InvalidateRect(last.wnd, nullptr, FALSE);
    } else if (g_flyPlace >= 0 || g_flyFooter) {
        g_flyPlace = -1;
        g_flyFooter = false;
        if (g_startWnd) InvalidateRect(g_startWnd, nullptr, FALSE);
    }
}

int CascadeLevel(Cascade const* c) {
    for (size_t i = 0; i < g_cascades.size(); i++)
        if (g_cascades[i].get() == c) return (int)i;
    return -1;
}

// A folder's contents: folders first, then by name.
std::vector<CascadeItem> FolderCascadeItems(std::wstring const& folder) {
    std::vector<CascadeItem> items;
    ForEachShellChild(folder, [&](IShellItem* item) {
        SFGAOF attributes = 0;
        item->GetAttributes(SFGAO_FOLDER | SFGAO_STREAM | SFGAO_HIDDEN, &attributes);
        if (!(attributes & SFGAO_HIDDEN)) {
            CascadeItem e;
            e.text = ItemName(item, SIGDN_NORMALDISPLAY);
            e.target = ItemName(item, SIGDN_DESKTOPABSOLUTEPARSING);
            e.folder = (attributes & SFGAO_FOLDER) && !(attributes & SFGAO_STREAM);
            if (!e.text.empty() && !e.target.empty()) items.push_back(std::move(e));
        }
        return items.size() < kFlyMaxItems;
    });
    std::sort(items.begin(), items.end(), [](CascadeItem const& a, CascadeItem const& b) {
        if (a.folder != b.folder) return a.folder;
        return StartCompare(a.text, b.text) < 0;
    });
    return items;
}

// A folder's contents are read once and kept a few seconds, and read ahead in the background as
// the pointer comes to a folder (a place, or a folder in a flyout), so its flyout opens without
// waiting on the shell.
struct FolderCacheEntry {
    std::vector<CascadeItem> items;
    DWORD when = 0;
};
std::unordered_map<std::wstring, FolderCacheEntry> g_folderCache;
std::vector<std::wstring> g_folderPending;
SRWLOCK g_folderLock = SRWLOCK_INIT;
LONG g_folderThreads = 0;
constexpr DWORD kFolderKeepMs = 15000;

void StoreFolderItems(std::wstring const& folder, std::vector<CascadeItem> const& items) {
    AcquireSRWLockExclusive(&g_folderLock);
    g_folderCache[folder] = {items, GetTickCount()};
    ReleaseSRWLockExclusive(&g_folderLock);
}

bool FreshFolderItems(std::wstring const& folder, std::vector<CascadeItem>* out) {
    bool fresh = false;
    AcquireSRWLockShared(&g_folderLock);
    auto it = g_folderCache.find(folder);
    if (it != g_folderCache.end() && GetTickCount() - it->second.when < kFolderKeepMs) {
        fresh = true;
        if (out) *out = it->second.items;
    }
    ReleaseSRWLockShared(&g_folderLock);
    return fresh;
}

std::vector<CascadeItem> CachedFolderItems(std::wstring const& folder) {
    std::vector<CascadeItem> items;
    if (FreshFolderItems(folder, &items)) return items;
    items = FolderCascadeItems(folder);
    StoreFolderItems(folder, items);
    return items;
}

DWORD WINAPI PrefetchFolderThread(LPVOID param) {
    std::unique_ptr<std::wstring> folder((std::wstring*)param);
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        StoreFolderItems(*folder, FolderCascadeItems(*folder));
        CoUninitialize();
    }
    AcquireSRWLockExclusive(&g_folderLock);
    std::erase(g_folderPending, *folder);
    ReleaseSRWLockExclusive(&g_folderLock);
    InterlockedDecrement(&g_folderThreads);
    return 0;
}

void PrefetchFolder(std::wstring const& folder) {
    if (folder.empty() || folder.starts_with(L"ms-settings:") || FreshFolderItems(folder, nullptr)) return;
    AcquireSRWLockExclusive(&g_folderLock);
    bool started = std::find(g_folderPending.begin(), g_folderPending.end(), folder) == g_folderPending.end();
    if (started) g_folderPending.push_back(folder);
    ReleaseSRWLockExclusive(&g_folderLock);
    if (!started) return;
    InterlockedIncrement(&g_folderThreads);
    auto* job = new std::wstring(folder);
    HANDLE thread = CreateThread(nullptr, 0, PrefetchFolderThread, job, 0, nullptr);
    if (thread) {
        CloseHandle(thread);
    } else {
        delete job;
        InterlockedDecrement(&g_folderThreads);
        AcquireSRWLockExclusive(&g_folderLock);
        std::erase(g_folderPending, folder);
        ReleaseSRWLockExclusive(&g_folderLock);
    }
}

void FreeFolderCache() {
    for (int wait = 0; g_folderThreads > 0 && wait < 60; wait++) Sleep(50);
    AcquireSRWLockExclusive(&g_folderLock);
    g_folderCache.clear();
    ReleaseSRWLockExclusive(&g_folderLock);
}

// All Programs' folders, a level at a time, in the list's own order.
std::vector<CascadeItem> ProgCascadeItems(ProgNode& node) {
    ReadProgKids(node);
    std::vector<CascadeItem> items;
    for (auto& kid : node.kids) {
        CascadeItem e;
        e.text = kid.name;
        e.target = kid.folder && !kid.dirs.empty() ? kid.dirs[0] : kid.parsing;
        e.node = &kid;
        e.folder = kid.folder;
        items.push_back(std::move(e));
    }
    return items;
}

void FillCascadeIcons(Cascade& c) {
    int size = CascadeIconPx();
    if (!size) return;
    for (auto& item : c.items)
        if (!item.icon && !item.separator && !item.header && !item.target.empty() && !item.target.starts_with(L"ms-settings:"))
            item.icon = StartIcon(item.node && !item.node->folder ? item.node->parsing : item.target, size);
}

bool CascadeSelectable(Cascade const& c, int i) {
    if (i < 0 || i >= (int)c.items.size()) return false;
    auto const& item = c.items[i];
    return !item.separator && !item.header && !item.disabled;
}

// Repaints one item's row (nothing for none).
void InvalidateCascadeItem(Cascade& c, int i) {
    if (i < 0 || i >= (int)c.items.size()) return;
    RECT r = c.items[i].rc;
    if (c.scrolls) OffsetRect(&r, 0, -c.scroll);
    InvalidateRect(c.wnd, &r, FALSE);
}

// Sizes and places a cascade beside what opened it (a screen rectangle),
// on its screen: a new column where the screen runs out with Multi-column
// flyouts, else scroll arrows.
// Where a cascade's items start in from its edge (inside Tourne Menus' frame when it has one), and their text.
int CascadePad() {
    return g_menuLook.frame && !g_menuLook.theme ? FrameInnerAt(std::max(1, g_menuLook.scale)) + 1 + Px(2) : Px(3);
}
int CascadeTextLeft(int icon) {
    if (g_menuLook.frame && !g_menuLook.theme) return icon ? Px(11) + icon + Px(8) : Px(31);
    return icon ? Px(6) + icon + Px(8) : Px(22);
}

void LayoutCascade(Cascade& c, RECT const& from, int parentLeft, int& x, int& y, int& w, int& h) {
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = g_menuLook.font ? SelectObject(dc, g_menuLook.font) : nullptr;
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    int widest = 0;
    for (auto const& item : c.items) {
        if (item.separator) continue;
        SIZE size{};
        GetTextExtentPoint32W(dc, item.text.c_str(), (int)item.text.size(), &size);
        widest = std::max(widest, (int)size.cx);
    }
    if (old) SelectObject(dc, old);
    ReleaseDC(nullptr, dc);

    int icon = CascadeIconPx();
    bool framed = g_menuLook.frame && !g_menuLook.theme;
    int row = framed ? std::max(Px(22), std::max(icon + Px(6), (int)tm.tmHeight + Px(8)))
                     : std::max(Px(18), std::max(icon + Px(6), (int)tm.tmHeight + Px(5)));
    int textLeft = CascadeTextLeft(icon);
    int itemWidth = std::clamp(textLeft + widest + Px(30), Px(140), Px(420));
    int pad = CascadePad();
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&from, MONITOR_DEFAULTTONEAREST), &mi);
    RECT work = mi.rcWork;
    int maxHeight = work.bottom - work.top;

    auto heightOf = [&](CascadeItem const& item) {
        return item.separator ? Px(9) : item.header ? (int)tm.tmHeight + Px(8) : row;
    };
    int total = 0;
    for (auto const& item : c.items) total += heightOf(item);
    int columns = 1;
    c.scrolls = false;
    int yy = pad, column = 0;
    if (total + 2 * pad > maxHeight && g_startConfig.multiColumn) {
        for (auto& item : c.items) {
            int ih = heightOf(item);
            if (yy + ih > maxHeight - pad && yy > pad) {
                column++;
                yy = pad;
            }
            item.rc = {pad + column * itemWidth, yy, pad + (column + 1) * itemWidth, yy + ih};
            yy += ih;
        }
        columns = column + 1;
        h = maxHeight;
    } else {
        int arrows = total + 2 * pad > maxHeight ? Px(16) : 0;
        yy = pad + arrows;
        for (auto& item : c.items) {
            int ih = heightOf(item);
            item.rc = {pad, yy, pad + itemWidth, yy + ih};
            yy += ih;
        }
        h = std::min(maxHeight, total + 2 * pad + 2 * arrows);
        if (arrows) {
            c.scrolls = true;
            c.room = h - 2 * pad - 2 * arrows;
            c.content = total;
            c.up = {pad, pad, pad + itemWidth, pad + arrows};
            c.down = {pad, h - pad - arrows, pad + itemWidth, h - pad};
        }
    }
    w = columns * itemWidth + 2 * pad;
    // Beside it, its first item level with it; else on the other side.
    x = from.right;
    if (x + w > work.right) x = std::max((int)work.left, parentLeft - w);
    y = std::clamp((int)from.top - pad, (int)work.top, std::max((int)work.top, (int)work.bottom - h));
}

LRESULT CALLBACK CascadeProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Opens a cascade beside a screen rectangle (a place, All Programs, or the
// item of the cascade before it).
Cascade* OpenCascade(std::unique_ptr<Cascade> c, RECT const& from, int parentLeft) {
    if (!g_cascadeClass) {
        WNDCLASSW wc{};
        wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = CascadeProc;
        wc.hInstance = g_instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kCascadeClass;
        g_cascadeClass = RegisterClassW(&wc) != 0;
    }
    if (c->items.empty()) {
        CascadeItem empty;
        empty.text = L"(Empty)";
        empty.disabled = true;
        c->items.push_back(empty);
    }
    int x, y, w, h;
    LayoutCascade(*c, from, parentLeft, x, y, w, h);
    Cascade* raw = c.get();
    g_cascades.push_back(std::move(c));
    raw->wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kCascadeClass, L"", WS_POPUP, x, y,
                               w, h, g_startWnd, nullptr, g_instance, nullptr);
    if (!raw->wnd) {
        g_cascades.pop_back();
        return nullptr;
    }
    SetWindowLongPtrW(raw->wnd, GWLP_USERDATA, (LONG_PTR)raw);
    DWORD square = 1;  // DWMWCP_DONOTROUND
    DwmSetWindowAttribute(raw->wnd, 33, &square, sizeof(square));
    BOOL noFade = TRUE;  // DWMWA_TRANSITIONS_FORCEDISABLED: DWM's ~200 ms fade-in is what made it feel slow
    DwmSetWindowAttribute(raw->wnd, 3, &noFade, sizeof(noFade));
    ShowWindow(raw->wnd, SW_SHOWNOACTIVATE);
    SetTimer(raw->wnd, kTimerCascadeIcons, USER_TIMER_MINIMUM, nullptr);  // the icons follow, a few at a time
    return raw;
}

// An item's rectangle on screen, scrolled.
RECT CascadeItemScreen(Cascade const& c, int i) {
    RECT r = c.items[i].rc;
    if (c.scrolls) OffsetRect(&r, 0, -c.scroll);
    MapWindowPoints(c.wnd, nullptr, (POINT*)&r, 2);
    return r;
}

// Opens a folder item's cascade.
void OpenCascadeChild(int level, int index) {
    if (level < 0 || level >= (int)g_cascades.size()) return;
    CloseCascades(level + 1);
    Cascade& parent = *g_cascades[level];
    if (!CascadeSelectable(parent, index) || !parent.items[index].folder) return;
    auto child = std::make_unique<Cascade>();
    CascadeItem const& item = parent.items[index];
    child->items = item.node ? ProgCascadeItems(*item.node) : CachedFolderItems(item.target);
    RECT from = CascadeItemScreen(parent, index);
    RECT window;
    GetWindowRect(parent.wnd, &window);
    parent.open = index;
    parent.hot = index;
    InvalidateRect(parent.wnd, nullptr, FALSE);
    OpenCascade(std::move(child), from, window.left);
}

// Runs an item: the Start menu closes first (and every cascade with it).
void RunCascadeItem(Cascade& c, int index) {
    if (!CascadeSelectable(c, index)) return;
    CascadeItem const& item = c.items[index];
    if (item.folder) {
        OpenCascadeChild(CascadeLevel(&c), index);
        return;
    }
    JumpItem const* jump = item.jump;
    std::wstring target = item.node ? item.node->parsing : item.target;
    if (jump) {
        JumpItem copy = *jump;  // the list goes with the cascade
        copy.target = copy.target ? ILCloneFull(copy.target) : nullptr;
        copy.icon = nullptr;
        HideStart();
        RunJumpItem(copy);
        if (copy.target) CoTaskMemFree(copy.target);
        return;
    }
    HideStart();
    StartOpen(target);
}

int CascadeHit(Cascade const& c, POINT pt) {
    if (c.scrolls) {
        if (PtInRect(&c.up, pt)) return -2;
        if (PtInRect(&c.down, pt)) return -3;
        pt.y += c.scroll;
    }
    for (int i = 0; i < (int)c.items.size(); i++)
        if (PtInRect(&c.items[i].rc, pt)) return i;
    return -1;
}

void ScrollCascade(Cascade& c, int delta) {
    if (!c.scrolls) return;
    int before = c.scroll;
    c.scroll = std::clamp(c.scroll + delta, 0, std::max(0, c.content - c.room));
    if (c.scroll != before) {
        InvalidateRect(c.wnd, nullptr, FALSE);
        SetTimer(c.wnd, kTimerCascadeIcons, USER_TIMER_MINIMUM, nullptr);  // rows scrolled into view get their icons
    }
}

void EnsureCascadeVisible(Cascade& c, int i) {
    if (!c.scrolls || i < 0) return;
    int top = c.items[i].rc.top - c.up.bottom, bottom = c.items[i].rc.bottom - c.up.bottom;
    if (top < c.scroll) c.scroll = top;
    else if (bottom > c.scroll + c.room) c.scroll = bottom - c.room;
    c.scroll = std::clamp(c.scroll, 0, std::max(0, c.content - c.room));
}

void PaintCascade(Cascade& c, HDC dc) {
    auto const& look = g_menuLook;
    HTHEME theme = look.theme;
    bool framed = look.frame && !theme;
    RECT client;
    GetClientRect(c.wnd, &client);
    auto fill = [&](RECT const& r, uint32_t rgb) {
        SetDCBrushColor(dc, MenuRgb(rgb));
        FillRect(dc, &r, (HBRUSH)GetStockObject(DC_BRUSH));
    };
    if (framed) {
        // Tourne Menus' frame, as StyledMenuWindowProc draws it round a menu: the band, the outer line, the
        // items' ground inside the inner line, both lines with the gradient and glow as set.
        fill(client, look.band);
        int q = FrameInnerAt(std::max(1, look.scale));
        fill(RECT{q + 1, q + 1, client.right - q - 1, client.bottom - q - 1}, look.ground);
        POINT at{0, 0};
        SIZE size{client.right, client.bottom};
        uint32_t band = look.band, ground = look.ground;
        int w = client.right - 2 * q, h = client.bottom - 2 * q;
        for (int pass : {1, 2}) {
            FrameLine(dc, 0, 0, client.right, true, at, size, band, band, false, true, pass);
            FrameLine(dc, 0, client.bottom - 1, client.right, true, at, size, band, band, true, false, pass);
            FrameLine(dc, 0, 0, client.bottom, false, at, size, band, band, false, true, pass);
            FrameLine(dc, client.right - 1, 0, client.bottom, false, at, size, band, band, true, false, pass);
            FrameLine(dc, q, q, w, true, at, size, band, ground, true, true, pass);
            FrameLine(dc, q, client.bottom - 1 - q, w, true, at, size, ground, band, true, true, pass);
            FrameLine(dc, q, q, h, false, at, size, band, ground, true, true, pass);
            FrameLine(dc, client.right - 1 - q, q, h, false, at, size, ground, band, true, true, pass);
        }
    } else {
        fill(client, look.ground);
        SetDCBrushColor(dc, MenuRgb(look.edge));
        FrameRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
    }
    HGDIOBJ oldFont = look.font ? SelectObject(dc, look.font) : nullptr;
    SetBkMode(dc, TRANSPARENT);
    int icon = CascadeIconPx();
    int textLeft = CascadeTextLeft(icon);
    // Items scroll between the arrows.
    RECT view = client;
    if (c.scrolls) {
        view.top = c.up.bottom;
        view.bottom = c.down.top;
    }
    int saved = SaveDC(dc);
    IntersectClipRect(dc, view.left, view.top, view.right, view.bottom);
    for (int i = 0; i < (int)c.items.size(); i++) {
        auto const& item = c.items[i];
        RECT r = item.rc;
        if (c.scrolls) OffsetRect(&r, 0, -c.scroll);
        if (r.bottom < view.top || r.top > view.bottom) continue;
        bool hot = (i == c.hot || i == c.open) && CascadeSelectable(c, i);
        if (item.separator) {
            RECT line{r.left + Px(6), (r.top + r.bottom) / 2, r.right - Px(2), (r.top + r.bottom) / 2 + Px(1)};
            if (look.classic && look.ownText) fill(line, look.text);
            else if (theme) DrawThemeBackground(theme, dc, MENU_POPUPSEPARATOR, 0, &r, nullptr);
            else fill(line, look.line);
            continue;
        }
        if (hot) {
            RECT h{r.left + Px(2), r.top, r.right - Px(2), r.bottom};
            if (theme) DrawThemeBackground(theme, dc, MENU_POPUPITEM, MPI_HOT, &h, nullptr);
            else fill(h, look.hover);
        }
        bool hoverText = hot && !item.disabled && !item.header && look.hoverText != 0xFFFFFFFF;
        COLORREF color = MenuRgb(item.disabled || item.header ? look.dim : hoverText ? look.hoverText : look.text);
        if (theme && (!look.ownText || item.disabled))
            GetThemeColor(theme, MENU_POPUPITEM, item.disabled ? MPI_DISABLED : hot ? MPI_HOT : MPI_NORMAL, TMT_TEXTCOLOR,
                          &color);
        SetTextColor(dc, color);
        if (item.header) {
            RECT t{r.left + Px(8), r.top, r.right - Px(8), r.bottom};
            DrawTextW(dc, item.text.c_str(), -1, &t, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
            continue;
        }
        if (item.icon && icon)
            DrawIconEx(dc, r.left + (framed ? Px(11) : Px(6)), (r.top + r.bottom - icon) / 2, item.icon, icon, icon, 0, nullptr, DI_NORMAL);
        RECT t{r.left + textLeft, r.top, r.right - Px(22), r.bottom};
        std::wstring text = item.text;
        if (look.caps) CharUpperBuffW(text.data(), (DWORD)text.size());
        DrawTextW(dc, text.c_str(), -1, &t, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
        if (item.folder) {
            int size = Px(16);
            RECT a{r.right - Px(4) - size, (r.top + r.bottom - size) / 2, r.right - Px(4), (r.top + r.bottom + size) / 2};
            if (framed) {
                // Tourne Menus' chevron (MenuDrawItem's), 4 x 8 pixels, in the arrow colour when one is set.
                static constexpr const char* kChevron[] = {"#...", "##..", ".##.", "..##", "..##", ".##.", "##..", "#..."};
                int x = r.right - Px(12), y = (r.top + r.bottom) / 2 - Px(4);
                COLORREF arrow = look.arrow != 0xFFFFFFFF && !item.disabled ? MenuRgb(look.arrow) : color;
                SetDCBrushColor(dc, arrow);
                for (int row = 0; row < 8; row++)
                    for (int col = 0; col < 4; col++)
                        if (kChevron[row][col] == '#') {
                            RECT dot{x + col * Px(1), y + row * Px(1), x + (col + 1) * Px(1), y + (row + 1) * Px(1)};
                            FillRect(dc, &dot, (HBRUSH)GetStockObject(DC_BRUSH));
                        }
            } else if (theme) {
                DrawThemeBackground(theme, dc, MENU_POPUPSUBMENU, MSM_NORMAL, &a, nullptr);
            } else {
                int ah = Px(4), ax = a.left + Px(6), ay = (a.top + a.bottom) / 2;
                for (int k = 0; k < ah; k++)
                    fill(RECT{ax + k * Px(1), ay - (ah - k) * Px(1) + Px(1), ax + (k + 1) * Px(1), ay + (ah - k) * Px(1)},
                         look.text);
            }
        }
    }
    RestoreDC(dc, saved);
    // The scroll arrows: a triangle each, faint where it can't go further.
    if (c.scrolls) {
        auto arrow = [&](RECT const& r, bool upward, bool live) {
            int ah = Px(4), cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
            uint32_t rgb = live ? look.text : look.dim;
            for (int k = 0; k < ah; k++) {
                int row = upward ? cy - ah / 2 + k : cy + ah / 2 - k;
                fill(RECT{cx - k * Px(1), row, cx + (k + 1) * Px(1), row + Px(1)}, rgb);
            }
        };
        arrow(c.up, true, c.scroll > 0);
        arrow(c.down, false, c.scroll < c.content - c.room);
    }
    if (oldFont) SelectObject(dc, oldFont);
}

LRESULT CALLBACK CascadeProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* c = (Cascade*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            if (c) {
                RECT r;
                GetClientRect(wnd, &r);
                if (!c->back || c->backSize.cx != r.right || c->backSize.cy != r.bottom) {
                    if (c->back) DeleteObject(c->back);
                    c->back = CreateCompatibleBitmap(dc, r.right, r.bottom);
                    c->backSize = {r.right, r.bottom};
                    ps.rcPaint = r;
                }
                HDC mem = CreateCompatibleDC(dc);
                HGDIOBJ old = SelectObject(mem, c->back);
                IntersectClipRect(mem, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
                PaintCascade(*c, mem);
                BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
                       ps.rcPaint.bottom - ps.rcPaint.top, mem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
                SelectObject(mem, old);
                DeleteDC(mem);
            }
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (!c) break;
            if (!c->tracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                c->tracking = true;
            }
            int level = CascadeLevel(c);
            // Back in a cascade whose child is open: its opener stays lit.
            if (level > 0) {
                Cascade& parent = *g_cascades[level - 1];
                if (parent.hot != parent.open) {
                    int was = parent.hot;
                    parent.hot = parent.open;
                    InvalidateCascadeItem(parent, was);
                    InvalidateCascadeItem(parent, parent.open);
                }
                KillTimer(parent.wnd, kTimerCascadeOpen);
            }
            int hit = CascadeHit(*c, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (hit == -2 || hit == -3) {
                c->scrollDir = hit == -2 ? -1 : 1;
                SetTimer(wnd, kTimerCascadeScroll, 40, nullptr);
                return 0;
            }
            KillTimer(wnd, kTimerCascadeScroll);
            if (!CascadeSelectable(*c, hit)) hit = -1;
            if (hit != c->hot) {
                int was = c->hot;
                c->hot = hit;
                InvalidateCascadeItem(*c, was);
                InvalidateCascadeItem(*c, hit);
                KillTimer(wnd, kTimerCascadeOpen);
                // A folder opens its cascade, or another item closes the
                // open one, after Windows' menu delay.
                if (hit >= 0 && c->items[hit].folder && !c->items[hit].node) PrefetchFolder(c->items[hit].target);
                bool opens = hit >= 0 && c->items[hit].folder && hit != c->open;
                bool closes = c->open >= 0 && hit >= 0 && hit != c->open;
                if (opens || closes) {
                    UINT delay = 150;
                    SystemParametersInfoW(SPI_GETMENUSHOWDELAY, 0, &delay, 0);
                    SetTimer(wnd, kTimerCascadeOpen, std::clamp(delay, 1u, 150u), nullptr);
                }
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            if (!c) break;
            c->tracking = false;
            KillTimer(wnd, kTimerCascadeScroll);
            if (c->hot != c->open) {
                int was = c->hot;
                c->hot = c->open;
                InvalidateCascadeItem(*c, was);
                InvalidateCascadeItem(*c, c->open);
            }
            return 0;
        case WM_TIMER:
            if (!c) break;
            if (wParam == kTimerCascadeIcons) {
                // A handful of icons a tick, the rows in view first, each row repainted as its icon arrives.
                int size = CascadeIconPx();
                int done = 0;
                RECT client;
                GetClientRect(wnd, &client);
                for (size_t i = 0; i < c->items.size() && done < 6; i++) {
                    auto& item = c->items[i];
                    if (item.icon || item.separator || item.header || item.target.empty() || !size ||
                        item.target.starts_with(L"ms-settings:") || item.iconTried)
                        continue;
                    RECT r = item.rc;
                    if (c->scrolls) OffsetRect(&r, 0, -c->scroll);
                    if (r.bottom < 0 || r.top > client.bottom) continue;  // out of view: later
                    item.iconTried = true;
                    item.icon = StartIcon(item.node && !item.node->folder ? item.node->parsing : item.target, size);
                    InvalidateCascadeItem(*c, (int)i);
                    done++;
                }
                if (!done) {
                    KillTimer(wnd, kTimerCascadeIcons);  // (scrolling in more rows starts it again)
                }
                return 0;
            }
            if (wParam == kTimerCascadeScroll) {
                ScrollCascade(*c, c->scrollDir * Px(12));
            } else if (wParam == kTimerCascadeOpen) {
                KillTimer(wnd, kTimerCascadeOpen);
                int level = CascadeLevel(c);
                if (c->hot >= 0 && c->items[c->hot].folder) OpenCascadeChild(level, c->hot);
                else if (c->hot >= 0) CloseCascades(level + 1);
            }
            return 0;
        case WM_LBUTTONUP: {
            if (!c) break;
            int hit = CascadeHit(*c, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            KillTimer(wnd, kTimerCascadeOpen);
            RunCascadeItem(*c, hit);  // may close this window: nothing after it
            return 0;
        }
        case WM_RBUTTONUP: {
            if (!c) break;
            int hit = CascadeHit(*c, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (!CascadeSelectable(*c, hit) || c->items[hit].jump || c->items[hit].target.starts_with(L"ms-settings:"))
                return 0;
            std::wstring target = c->items[hit].target;
            c->hot = hit;
            InvalidateRect(wnd, nullptr, FALSE);
            StartShellMenu(g_startWnd, target);  // may close this window: nothing after it
            return 0;
        }
        case WM_MOUSEWHEEL:
            if (c) ScrollCascade(*c, -GET_WHEEL_DELTA_WPARAM(wParam) * Px(60) / WHEEL_DELTA);
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// The keyboard, from the Start menu while cascades are open. True when used.
bool CascadeKey(WPARAM key) {
    if (g_cascades.empty()) return false;
    int level = (int)g_cascades.size() - 1;
    Cascade& c = *g_cascades[level];
    int n = (int)c.items.size();
    switch (key) {
        case VK_DOWN:
        case VK_UP: {
            int step = key == VK_DOWN ? 1 : -1;
            int i = c.hot;
            for (int k = 0; k < n; k++) {
                i = i < 0 ? (step > 0 ? 0 : n - 1) : (i + step + n) % n;
                if (CascadeSelectable(c, i)) break;
            }
            if (CascadeSelectable(c, i)) {
                c.hot = i;
                EnsureCascadeVisible(c, i);
                InvalidateRect(c.wnd, nullptr, FALSE);
            }
            return true;
        }
        case VK_RIGHT:
            if (c.hot >= 0 && c.items[c.hot].folder) {
                OpenCascadeChild(level, c.hot);
                if ((int)g_cascades.size() > level + 1) CascadeKey(VK_DOWN);
            }
            return true;
        case VK_LEFT:
        case VK_ESCAPE:
            CloseCascades(level);
            return true;
        case VK_RETURN:
            RunCascadeItem(c, c.hot);
            return true;
    }
    return false;
}

void OpenPlaceFlyout(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_places.size()) return;
    KillTimer(wnd, kTimerFlyout);
    if (g_flyPlace == index && !g_cascades.empty()) return;  // open already
    CloseCascades(0);
    StartPlace const place = g_places[index];
    RECT r = g_sly.places[index];
    MapWindowPoints(wnd, nullptr, (POINT*)&r, 2);
    RECT window;
    GetWindowRect(wnd, &window);
    // With the phone beside the places, the flyout opens right by the places, over the phone pane: the pointer goes
    // straight from the place into it, as it does without the pane (it used to open past the pane, out of reach).
    g_menuLook = StartMenuLook();
    if (g_menuLook.theme) {  // the style's own menu ground and border
        g_menuLook.ground = ThemePartPixel(g_menuLook.theme, MENU_POPUPBACKGROUND, 4, 4, g_menuLook.ground);
        g_menuLook.edge = ThemePartPixel(g_menuLook.theme, MENU_POPUPBORDERS, 0, 0, g_menuLook.edge);
    }
    auto c = std::make_unique<Cascade>();
    if (place.kind == kKindJump) {
        CascadeItem app;
        app.text = place.name;
        app.target = place.target;
        c->items.push_back(app);
        c->jump = ReadJumpList(place.appId, std::max(CascadeIconPx(), Px(16)));
        for (auto const& category : c->jump) {
            CascadeItem separator, header;
            separator.separator = true;
            header.header = true;
            header.text = category.name;
            c->items.push_back(separator);
            if (!category.name.empty()) c->items.push_back(header);
            for (auto const& item : category.items) {
                CascadeItem e;
                e.separator = item.separator;
                e.text = item.title;
                e.icon = item.icon;
                e.jump = item.separator ? nullptr : &item;
                c->items.push_back(e);
            }
        }
    } else if (place.special == 1) {
        for (auto const& page : kSettingsPages) {
            CascadeItem e;
            e.text = page.name;
            e.target = page.uri;
            c->items.push_back(e);
        }
    } else {
        c->items = CachedFolderItems(place.folder);
    }
    g_flyPlace = index;
    InvalidateRect(wnd, nullptr, FALSE);
    if (Cascade* opened = OpenCascade(std::move(c), r, window.left); !opened) CloseCascades(0);
}

// All Programs as a menu (Windows XP's way, "Display as a menu").
void OpenAllProgramsFlyout(HWND wnd) {
    CloseCascades(0);
    if (g_allRoot.dirs.empty()) {
        g_allRoot = {};
        g_allRoot.dirs = {KnownFolderPath(FOLDERID_Programs), KnownFolderPath(FOLDERID_CommonPrograms)};
    }
    RECT r = g_sly.footer;
    MapWindowPoints(wnd, nullptr, (POINT*)&r, 2);
    RECT window;
    GetWindowRect(wnd, &window);
    g_menuLook = StartMenuLook();
    if (g_menuLook.theme) {
        g_menuLook.ground = ThemePartPixel(g_menuLook.theme, MENU_POPUPBACKGROUND, 4, 4, g_menuLook.ground);
        g_menuLook.edge = ThemePartPixel(g_menuLook.theme, MENU_POPUPBORDERS, 0, 0, g_menuLook.edge);
    }
    auto c = std::make_unique<Cascade>();
    c->items = ProgCascadeItems(g_allRoot);
    g_flyFooter = true;
    InvalidateRect(wnd, nullptr, FALSE);
    if (!OpenCascade(std::move(c), r, window.left)) CloseCascades(0);
}

// Properties: the Start menu's own settings (Tourne Studio in Tourne Desktop, Windhawk's window in the suite).
void OpenStartProperties() { OpenOwnSettings(L"startmenu."); }

// Right-click on All Programs (Open, Open All Users, Display as a menu), or
// on the menu's empty space (Properties alone), as StartAllBack has them.
void PhoneShowAgain();
void RelayoutStart();

void StartBlankMenu(HWND wnd, bool allPrograms) {
    enum : int { kOpen = 1, kOpenAll, kAsMenu, kProperties, kPhone, kResetSize };
    HMENU menu = CreatePopupMenu();
    if (allPrograms) {
        AppendMenuW(menu, MF_STRING, kOpen, L"&Open");
        AppendMenuW(menu, MF_STRING, kOpenAll, L"Open All &Users");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING | (g_startConfig.allFlyout ? MF_CHECKED : 0), kAsMenu, L"Display as a menu");
    }
    // The phone, once closed, comes back from here.
    bool phoneClosed = !allPrograms && g_startConfig.phone && g_phoneState.hidden && PhoneLinked();
    if (phoneClosed) AppendMenuW(menu, MF_STRING, kPhone, L"Show my phone");
    // Pulled bigger by its edges: back to as it comes.
    if (!allPrograms && (g_startExtraW || g_startExtraH)) AppendMenuW(menu, MF_STRING, kResetSize, L"Size as it comes");
    AppendMenuW(menu, MF_STRING, kProperties, L"P&roperties");
    POINT pt;
    GetCursorPos(&pt);
    int cmd = RunStartMenu(wnd, menu, TPM_RIGHTBUTTON, pt.x, pt.y);
    if (cmd == kPhone) {
        PhoneShowAgain();
        return;
    }
    if (cmd == kResetSize) {
        g_startExtraW = g_startExtraH = 0;
        SaveStartSize();
        RelayoutStart();
        return;
    }
    if (cmd == kAsMenu) {
        // Kept with the setting it overrides, so changing that setting wins.
        DWORD chosen = !g_startConfig.allFlyout;
        DWORD setting = Wh_GetIntSetting(L"startMenu.allPrograms.flyout") != 0;
        RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, L"AllProgramsFlyout", REG_DWORD, &chosen, sizeof(chosen));
        RegSetKeyValueW(HKEY_CURRENT_USER, kStartKey, L"AllProgramsFlyoutSetting", REG_DWORD, &setting, sizeof(setting));
        g_startConfig.allFlyout = chosen != 0;
        return;
    }
    if (!cmd) return;
    HideStart();
    switch (cmd) {
        case kOpen: StartOpen(L"shell:Programs"); break;
        case kOpenAll: StartOpen(L"shell:Common Programs"); break;
        case kProperties: OpenStartProperties(); break;
    }
}

enum : int { kSmOpen = 1, kSmAdmin, kSmLocation, kSmPinTaskbar, kSmPinStart, kSmUnpinStart, kSmRemove, kSmProperties };

void ProgramMenu(HWND wnd, StartRow const row) {
    if (row.kind == kRowFolder) {
        if (row.node && !row.node->dirs.empty()) StartShellMenu(wnd, row.node->dirs[0]);
        return;
    }
    std::wstring file = StartFilePath(row.parsing);
    bool pinned = row.pinned;
    HMENU menu = CreatePopupMenu();
    // As StartAllBack has it.
    AppendMenuW(menu, MF_STRING, kSmOpen, L"Open");
    AppendMenuW(menu, MF_STRING, kSmAdmin, L"Run as administrator");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kSmPinTaskbar, L"Pin to taskbar");
    AppendMenuW(menu, MF_STRING, pinned ? kSmUnpinStart : kSmPinStart, pinned ? L"Unpin from Start Menu" : L"Pin to Start Menu");
    if (g_startView == kViewHome) {
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(menu, MF_STRING, kSmRemove, L"Remove from this list");
    }
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING | (file.empty() ? MF_GRAYED : 0), kSmProperties, L"Properties");
    POINT pt;
    GetCursorPos(&pt);
    switch (RunStartMenu(wnd, menu, 0, pt.x, pt.y)) {
        case kSmOpen: HideStart(); StartOpen(row.parsing); break;
        case kSmAdmin: HideStart(); StartOpen(row.parsing, L"runas"); break;
        case kSmLocation: {
            HideStart();
            std::wstring args = L"/select,\"" + file + L"\"";
            ShellExecuteW(nullptr, nullptr, L"explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL);
            break;
        }
        case kSmPinTaskbar: RelayPin(g_wnd, row.parsing, true); break;
        case kSmPinStart:
            ListToggle(L"Hidden", row.parsing, false);
            if (!ListHas(ReadWindowsStartPins(), row.parsing)) ListToggle(L"Pins", row.parsing, true);
            break;
        case kSmUnpinStart:
            // Windows' own pins are left as they are, and hidden here.
            ListToggle(L"Pins", row.parsing, false);
            if (ListHas(ReadWindowsStartPins(), row.parsing)) ListToggle(L"Hidden", row.parsing, true);
            break;
        case kSmRemove: ListToggle(L"Hidden", row.parsing, true); break;
        case kSmProperties: HideStart(); SHObjectProperties(nullptr, SHOP_FILEPATH, file.c_str(), nullptr); return;
        default: return;
    }
    // Pinned, unpinned or taken off: the list again.
    if (g_startView == kViewHome && IsWindowVisible(wnd)) {
        BuildStartRows();
        InvalidateRect(wnd, nullptr, FALSE);
    }
}


bool ShutdownPrivilege() {
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) return false;
    TOKEN_PRIVILEGES tp{1};
    bool ok = LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid);
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    ok = ok && AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr);
    CloseHandle(token);
    return ok;
}

void StartPower(int what) {
    constexpr DWORD kReason = SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED;
    switch (what) {
        case kPwSwitch: {
            using Disconnect_t = BOOL(WINAPI*)(HANDLE, DWORD, BOOL);
            HMODULE wts = LoadLibraryW(L"wtsapi32.dll");
            auto disconnect = wts ? (Disconnect_t)GetProcAddress(wts, "WTSDisconnectSession") : nullptr;
            if (!disconnect || !disconnect(nullptr, (DWORD)-1, FALSE)) LockWorkStation();
            break;
        }
        case kPwSignOut: ExitWindowsEx(EWX_LOGOFF, kReason); break;
        case kPwLock: LockWorkStation(); break;
        case kPwSleep: {
            using Suspend_t = BOOLEAN(WINAPI*)(BOOLEAN, BOOLEAN, BOOLEAN);
            HMODULE power = LoadLibraryW(L"powrprof.dll");
            if (auto suspend = power ? (Suspend_t)GetProcAddress(power, "SetSuspendState") : nullptr)
                suspend(FALSE, FALSE, FALSE);
            break;
        }
        case kPwRestart:
            ShutdownPrivilege();
            ExitWindowsEx(EWX_REBOOT, kReason);
            break;
        case kPwShutdown:
            ShutdownPrivilege();
            ExitWindowsEx(EWX_SHUTDOWN | EWX_POWEROFF | EWX_HYBRID_SHUTDOWN, kReason);
            break;
        case kPwHibernate: {
            using Suspend_t = BOOLEAN(WINAPI*)(BOOLEAN, BOOLEAN, BOOLEAN);
            HMODULE power = LoadLibraryW(L"powrprof.dll");
            if (auto suspend = power ? (Suspend_t)GetProcAddress(power, "SetSuspendState") : nullptr)
                suspend(TRUE, FALSE, FALSE);
            break;
        }
    }
}

void PowerMenu(HWND wnd) {
    // Windows 7's, less what the button itself does.
    HMENU menu = CreatePopupMenu();
    for (int action : std::initializer_list<int>{kPwSwitch, kPwSignOut, kPwLock, 0, kPwRestart, kPwSleep, kPwHibernate,
                                                  kPwShutdown}) {
        if (!action) AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
        else if (action != g_startConfig.powerButton) AppendMenuW(menu, MF_STRING, action, PowerLabel(action));
    }
    RECT r = g_sly.shutdownArrow;
    MapWindowPoints(wnd, nullptr, (POINT*)&r, 2);
    int cmd = RunStartMenu(wnd, menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, r.left, r.top);
    if (cmd) {
        HideStart();
        StartPower(cmd);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Opening and closing

void SetStartView(HWND wnd, StartView view) {
    g_startView = view;
    g_scrollPx = 0;
    g_smHot = -1;
    StartViewWidth();
    BuildStartRows();
    InvalidateRect(wnd, nullptr, FALSE);
}

void ScrollToRow(int index) {
    if (index < 0 || index >= (int)g_rows.size()) return;
    int room = g_sly.list.bottom - g_sly.list.top;
    auto const& row = g_rows[index];
    if (row.top < g_scrollPx) g_scrollPx = row.top;
    else if (row.top + row.height > g_scrollPx + room) g_scrollPx = row.top + row.height - room;
    ClampScroll();
}

void HidePhoneWindow();
void PhoneActivate(int hit);

////////////////////////////////////////////////////////////////////////////////
// Opening and closing on the motion engine's "start" line (motion.cpp): faded, slid a short way in from
// the bar's edge, or both. Layered only while it moves, so it's drawn as ever once it's still. popin and
// gnomed have no scaling here: a fade and a shorter slide. No line: shown and hidden at once, as ever.

constexpr UINT_PTR kTimerStartMotion = 5;
double NowMs();

struct StartMotion {
    HWND wnd = nullptr;    // the window it's moving (none: still)
    bool closing = false;
    bool disabled = false;  // turned off for input while it goes
    double level = 1;      // 0 away, 1 in place
    double from = 1, start = 0;
    POINT home{};          // its place
};
StartMotion g_startMotion;

bool StartMotionOn() {
    auto const& a = g_settings.motionStart;
    return a.set && a.on && motion::Ms(a) > 0;
}

double StartMotionMs() { return std::min(motion::Ms(g_settings.motionStart), 1000.0); }

// Faded or not, and how far it slides.
void StartMotionStyle(bool* fade, int* slide) {
    std::wstring w = motion::StyleWord(g_settings.motionStart);
    bool slides = w == L"slide" || w == L"slidevert" || w == L"slidefade" || w == L"slidefadevert";
    *fade = w != L"slide" && w != L"slidevert";
    *slide = slides ? Px(32) : w == L"popin" || w == L"gnomed" ? Px(12) : 0;
}

void StartMotionLayer(HWND wnd, bool on, bool clickThrough) {
    LONG_PTR ex = GetWindowLongPtrW(wnd, GWL_EXSTYLE);
    LONG_PTR want = ex & ~(LONG_PTR)(WS_EX_LAYERED | WS_EX_TRANSPARENT);
    if (on) want |= WS_EX_LAYERED | (clickThrough ? WS_EX_TRANSPARENT : 0);
    if (want != ex) SetWindowLongPtrW(wnd, GWL_EXSTYLE, want);
}

// The window where the motion has it: its opacity, and its place (nearer the bar while it's away).
void StartMotionApply() {
    auto const& m = g_startMotion;
    bool fade;
    int slide;
    StartMotionStyle(&fade, &slide);
    BYTE alpha = fade ? (BYTE)std::lround(255 * std::clamp(m.level, 0.0, 1.0)) : 255;
    SetLayeredWindowAttributes(m.wnd, 0, alpha, LWA_ALPHA);
    if (!slide) return;
    int d = (int)std::lround(slide * (1 - m.level));
    POINT at = m.home;
    switch (g_settings.edge) {
        case Edge::Top: at.y -= d; break;
        case Edge::Bottom: at.y += d; break;
        case Edge::Left: at.x -= d; break;
        case Edge::Right: at.x += d; break;
    }
    SetWindowPos(m.wnd, nullptr, at.x, at.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// Still again: hidden if it was closing, back in its place, opaque and no longer layered, taking input.
void StartMotionEnd() {
    StartMotion m = g_startMotion;
    if (!m.wnd) return;
    KillTimer(m.wnd, kTimerStartMotion);
    bool alive = IsWindow(m.wnd);
    if (alive && m.closing) ShowWindow(m.wnd, SW_HIDE);  // (still closing meanwhile: losing the focus doesn't close it again)
    g_startMotion = {};
    if (!alive) return;
    SetWindowPos(m.wnd, nullptr, m.home.x, m.home.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    StartMotionLayer(m.wnd, false, false);
    if (m.disabled) EnableWindow(m.wnd, TRUE);
    // (Drawn again once it's no longer layered: now if it's showing, else when it's next shown.)
    RedrawWindow(m.wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | (m.closing ? 0 : RDW_UPDATENOW));
}

// Sets off from where it is now towards in place (opening) or away (closing).
void StartMotionGo(bool closing) {
    auto& m = g_startMotion;
    if (!m.wnd) {
        RECT r;
        GetWindowRect(g_startWnd, &r);
        m.home = {r.left, r.top};
        m.level = closing ? 1 : 0;
    }
    m.wnd = g_startWnd;
    m.closing = closing;
    m.from = m.level;
    m.start = NowMs();
    // While it goes, clicks pass through it and keys don't reach it.
    StartMotionLayer(m.wnd, true, closing);
    if (closing && !m.disabled && IsWindowEnabled(m.wnd)) {
        EnableWindow(m.wnd, FALSE);
        m.disabled = true;
    } else if (!closing && m.disabled) {
        EnableWindow(m.wnd, TRUE);
        m.disabled = false;
    }
    StartMotionApply();
    if (!SetTimer(m.wnd, kTimerStartMotion, USER_TIMER_MINIMUM, nullptr)) StartMotionEnd();  // (no frames: there at once)
}

// Before it's shown (placed already): true when it opens moving, from away or from where a close left it.
bool StartMotionOpen() {
    auto& m = g_startMotion;
    if (m.wnd && m.wnd != g_startWnd) m = {};  // (a window since closed)
    bool was = m.wnd != nullptr;
    if (was) {
        // Placed again: its place is where it is now.
        RECT r;
        GetWindowRect(g_startWnd, &r);
        m.home = {r.left, r.top};
        m.closing = false;  // (ending here leaves it shown)
    }
    if (!StartMotionOn() || (!was && IsWindowVisible(g_startWnd))) {
        StartMotionEnd();
        return false;
    }
    StartMotionGo(false);
    return true;
}

// Closing: true when it goes moving (hidden at the end), false to hide it now.
bool StartMotionClose() {
    if (g_startMotion.wnd && g_startMotion.wnd != g_startWnd) g_startMotion = {};
    if (!StartMotionOn()) {
        StartMotionEnd();
        return false;
    }
    StartMotionGo(true);
    return true;
}

bool StartClosing() { return g_startMotion.wnd && g_startMotion.wnd == g_startWnd && g_startMotion.closing; }

void StartMotionFrame() {
    auto& m = g_startMotion;
    if (!m.wnd) return;
    double to = m.closing ? 0 : 1;
    double span = StartMotionMs() * std::abs(to - m.from);
    double t = span > 0 ? (NowMs() - m.start) / span : 1;
    if (t >= 1 || !StartMotionOn()) {
        m.level = to;
        StartMotionEnd();
        return;
    }
    m.level = m.from + (to - m.from) * motion::Ease(g_settings.motionStart, t);
    StartMotionApply();
}

void HideStart() {
    CloseCascades(0);
    if (g_startTip) SendMessageW(g_startTip, TTM_POP, 0, 0);
    if (!g_phoneState.pinned) HidePhoneWindow();  // (apart, it goes with the Start menu unless kept open)
    if (!g_startWnd || !IsWindowVisible(g_startWnd) || StartClosing()) return;
    KillTimer(g_startWnd, kTimerFlyout);
    if (!StartMotionClose()) ShowWindow(g_startWnd, SW_HIDE);
    g_startHiddenAt = GetTickCount();
    g_inMenu = false;
    // Start's list read again for next time, once whatever was opened is up.
    SetTimer(g_startWnd, kTimerApps, 1500, nullptr);
}

void StartActivate(HWND wnd, int hit) {
    if (hit == kSmClear) {
        SetWindowTextW(g_startEdit, L"");
        SetFocus(g_startEdit);
        return;
    }
    if (hit >= 0 && hit < (int)g_rows.size()) {
        StartRow const row = g_rows[hit];
        if (row.kind == kRowFolder && row.node) {
            row.node->open = !row.node->open;
            BuildStartRows();
            ScrollToRow(hit);
            InvalidateRect(wnd, nullptr, FALSE);
        } else if (row.kind == kRowItem) {
            HideStart();
            StartOpen(row.parsing);
        }
        return;
    }
    if (hit == kSmFooter) {
        if (g_startView == kViewHome && g_startConfig.allFlyout) {
            if (g_flyFooter) CloseCascades(0);
            else OpenAllProgramsFlyout(wnd);
        } else if (g_startView == kViewHome) {
            SetStartView(wnd, kViewAll);
        } else if (g_startView == kViewAll) {
            SetStartView(wnd, kViewHome);
        } else {
            std::wstring query;
            for (WCHAR c : StartSearchText()) {
                if (iswalnum(c)) {
                    query += c;
                } else {
                    WCHAR hex[8];
                    swprintf_s(hex, L"%%%02X", (unsigned)c & 0xFF);
                    query += c > 0xFF ? std::wstring(1, c) : std::wstring(hex);
                }
            }
            HideStart();
            ShellExecuteW(nullptr, nullptr, (L"search-ms:query=" + query).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }
        return;
    }
    if (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size()) {
        StartPlace const place = g_places[hit - kSmPlace];
        if (place.kind != kKindLink) {
            // A click toggles its menu.
            if (g_flyPlace == hit - kSmPlace && !g_cascades.empty()) CloseCascades(0);
            else OpenPlaceFlyout(wnd, hit - kSmPlace);
            return;
        }
        HideStart();
        if (place.special == 2) ShowRunDialog();
        else StartOpen(place.target);
        return;
    }
    if (IsPhoneHit(hit)) {
        PhoneActivate(hit);
        return;
    }
    if (hit == kSmShutdown) {
        HideStart();
        StartPower(g_startConfig.powerButton);
    } else if (hit == kSmShutdownArrow) {
        PowerMenu(wnd);
    }
}

// Up and down through the list and the footer, as the keyboard moves.
void StartMove(HWND wnd, int step) {
    std::vector<int> order;
    for (int i = 0; i < (int)g_rows.size(); i++)
        if (Selectable(i)) order.push_back(i);
    order.push_back(kSmFooter);
    auto at = std::find(order.begin(), order.end(), g_smHot);
    int index = at == order.end() ? (step > 0 ? 0 : (int)order.size() - 1)
                                  : std::clamp((int)(at - order.begin()) + step, 0, (int)order.size() - 1);
    g_smHot = order[index];
    ScrollToRow(g_smHot);
    InvalidateRect(wnd, nullptr, FALSE);
}

void PlaceStartWindow(HWND wnd) {
    auto const& L = g_sly;
    HWND bar = g_wnd;
    RECT barRect{}, orb = g_orbRect;
    GetWindowRect(bar, &barRect);
    MapWindowPoints(bar, nullptr, (POINT*)&orb, 2);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&barRect, MONITOR_DEFAULTTONEAREST), &mi);
    RECT const& m = mi.rcMonitor;
    // The panes line up with the orb, their see-through margin before it,
    // kept on the screen: so at the screen's edge for an orb there, as
    // StartAllBack's.
    int x = orb.left - g_sl.frameLeft, y = barRect.bottom;
    switch (g_settings.edge) {
        case Edge::Top: y = barRect.bottom; break;
        case Edge::Bottom: y = barRect.top - L.height; break;
        case Edge::Left: x = barRect.right; y = orb.top; break;
        case Edge::Right: x = barRect.left - L.width; y = orb.top; break;
    }
    x = std::clamp(x, (int)m.left, std::max((int)m.left, (int)m.right - L.width));
    y = std::clamp(y, (int)m.top, std::max((int)m.top, (int)m.bottom - L.height));
    SetWindowPos(wnd, HWND_TOPMOST, x, y, L.width, L.height, SWP_NOACTIVATE);
    if (wnd == g_startMotion.wnd) {
        // Placed again while it moves: the motion goes on to (and ends at) the new place.
        g_startMotion.home = {x, y};
        StartMotionApply();
    }
}

////////////////////////////////////////////////////////////////////////////////
// The phone apart: its own window, moved by its title strip (or torn off the Start menu by its head), shown
// with the Start menu, or kept open on its own (Keep open). Never takes the focus from the Start menu.

void StartCorners(HWND wnd);
bool EnsureStartWindow();
void StartForeground(HWND wnd);

constexpr WCHAR kPhoneClass[] = L"TourneStartPhone_" WH_MOD_ID;
constexpr UINT_PTR kTimerPhoneRead = 3;  // while it's open on its own: read again every minute
bool g_phoneClass = false;
bool g_phoneTracking = false;

#ifdef TD_PREFIX
// A call's state as Tourne Calls sends it (WM_COPYDATA kCallData to the Start window); its window: CallChanged.
constexpr ULONG_PTR kCallData = 0x4C414354;  // 'TCAL'
struct CallData {
    uint32_t on, status, here, muted, sound, seconds;  // status: Windows' (2 dialing, 3 talking, 4 held); seconds talking, or ~0
    wchar_t name[128], mic[128], out[128];
};
float g_callMic = 0, g_callCaller = 0;  // the levels as last heard ("TourneCallsLevels"), falling back
void CallChanged(CallData const& data);
#endif

// The Start menu laid out again, after the phone came, went, or moved out.
void RelayoutStart() {
    if (!g_startWnd || !IsWindowVisible(g_startWnd)) return;
    CloseCascades(0);
    StartComputeLayout();
    StartViewWidth();
    BuildStartRows();
    PlaceStartWindow(g_startWnd);
    StartShape(g_startWnd);
    InvalidateRect(g_startWnd, nullptr, FALSE);
}

// Its own window's size: as you pulled it (more notifications fill a taller one), else as it comes.
SIZE PhoneWindowSize() {
    int w = g_phoneState.w > 0 ? std::max(Px(g_phoneState.w), Px(200)) : Px(250);
    if (g_phoneState.h > 0) {
        int h = Px(g_phoneState.h);
        g_phoneWly = PhoneComputeLayout(0, 0, w, true, h - Px(8));
        return {w, std::max(h, (int)g_phoneWly.bottom + Px(8))};
    }
    g_phoneWly = PhoneComputeLayout(0, 0, w, true);
    return {w, g_phoneWly.bottom + Px(8)};
}

////////////////////////////////////////////////////////////////////////////////
// Pulling a window's edges to resize it: the Start menu (the edges away from the bar) and the phone apart (any).

constexpr int kEdgeLeft = 1, kEdgeTop = 2, kEdgeRight = 4, kEdgeBottom = 8;

int EdgesAt(HWND wnd, POINT client, int allowed) {
    RECT r;
    GetClientRect(wnd, &r);
    int grip = Px(6), e = 0;
    if (client.x < r.left + grip) e |= kEdgeLeft;
    if (client.x >= r.right - grip) e |= kEdgeRight;
    if (client.y < r.top + grip) e |= kEdgeTop;
    if (client.y >= r.bottom - grip) e |= kEdgeBottom;
    return e & allowed;
}

LPCWSTR EdgeCursor(int e) {
    bool across = e & (kEdgeLeft | kEdgeRight), down = e & (kEdgeTop | kEdgeBottom);
    if (across && down) return ((e & kEdgeLeft) && (e & kEdgeTop)) || ((e & kEdgeRight) && (e & kEdgeBottom)) ? IDC_SIZENWSE : IDC_SIZENESW;
    return across ? IDC_SIZEWE : down ? IDC_SIZENS : IDC_ARROW;
}

// The Start menu's: the side away from the bar, and the one away from the orb's screen edge.
int StartSizingEdges() {
    switch (g_settings.edge) {
        case Edge::Top: return kEdgeBottom | kEdgeRight;
        case Edge::Bottom: return kEdgeTop | kEdgeRight;
        case Edge::Left: return kEdgeRight | kEdgeBottom;
        case Edge::Right: return kEdgeLeft | kEdgeBottom;
    }
    return kEdgeRight;
}

////////////////////////////////////////////////////////////////////////////////
// The phone docked to one of Tourne's windows (a bar, the desktops pager, the Start menu): dragged near one of their
// sides it snaps there, and after that it follows the window wherever it goes.

bool IsDockClass(std::wstring const& cls) {
    return cls == kStartClass || cls == kPagerClass || cls == kWindowClass;
}

// The window of that class (this program's, showing) nearest to a point: which of several bars.
HWND FindDockTarget(std::wstring const& cls, POINT around) {
    struct Find {
        std::wstring const* cls;
        POINT around;
        HWND best;
        long long bestDistance;
    } find{&cls, around, nullptr, LLONG_MAX};
    EnumWindows(
        [](HWND h, LPARAM param) -> BOOL {
            auto* f = (Find*)param;
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            WCHAR name[128] = L"";
            if (pid != GetCurrentProcessId() || !IsWindowVisible(h) || !GetClassNameW(h, name, ARRAYSIZE(name)) || *f->cls != name)
                return TRUE;
            RECT r;
            GetWindowRect(h, &r);
            long long dx = (r.left + r.right) / 2 - f->around.x, dy = (r.top + r.bottom) / 2 - f->around.y;
            if (dx * dx + dy * dy < f->bestDistance) f->bestDistance = dx * dx + dy * dy, f->best = h;
            return TRUE;
        },
        (LPARAM)&find);
    return find.best;
}

// Where the phone goes on a side of the window at target (screen), dockAlong along it.
POINT DockedAt(RECT target, SIZE size, int side, int along) {
    switch (side) {
        case 0: return {target.left - size.cx, target.top + along};
        case 1: return {target.left + along, target.top - size.cy};
        case 2: return {target.right, target.top + along};
        default: return {target.left + along, target.bottom};
    }
}

// A drag of the phone near a dock: where it snaps to (want moves there) and the dock it would take; false when none.
struct DockPick {
    std::wstring cls;
    int side = 0, along = 0;
    POINT middle{};
};
bool PickDock(HWND phone, RECT& want, DockPick& pick) {
    struct Find {
        HWND phone;
        RECT want;
        int snap;
        bool found = false;
        DockPick pick;
        RECT snapped{};
    } find{phone, want, Px(18)};
    EnumWindows(
        [](HWND h, LPARAM param) -> BOOL {
            auto* f = (Find*)param;
            if (h == f->phone || !IsWindowVisible(h)) return TRUE;
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            WCHAR name[128] = L"";
            if (pid != GetCurrentProcessId() || !GetClassNameW(h, name, ARRAYSIZE(name)) || !IsDockClass(name)) return TRUE;
            RECT c;
            GetWindowRect(h, &c);
            RECT w = f->want;
            int width = w.right - w.left, height = w.bottom - w.top, snap = f->snap;
            bool overlapsY = w.bottom > c.top && w.top < c.bottom, overlapsX = w.right > c.left && w.left < c.right;
            int side = -1;
            if (overlapsY && abs(w.right - c.left) < snap) side = 0;
            else if (overlapsX && abs(w.bottom - c.top) < snap) side = 1;
            else if (overlapsY && abs(w.left - c.right) < snap) side = 2;
            else if (overlapsX && abs(w.top - c.bottom) < snap) side = 3;
            if (side < 0) return TRUE;
            int along = side == 0 || side == 2 ? w.top - c.top : w.left - c.left;
            int length = side == 0 || side == 2 ? c.bottom - c.top : c.right - c.left;
            int own = side == 0 || side == 2 ? height : width;
            if (abs(along) < snap) along = 0;                                  // lined up with its start
            else if (abs(along + own - length) < snap) along = length - own;  // or its end
            POINT at = DockedAt(c, {width, height}, side, along);
            f->snapped = {at.x, at.y, at.x + width, at.y + height};
            f->pick = {name, side, along, {(c.left + c.right) / 2, (c.top + c.bottom) / 2}};
            f->found = true;
            return FALSE;
        },
        (LPARAM)&find);
    if (!find.found) return false;
    want = find.snapped;
    pick = find.pick;
    return true;
}

// Its own window pulled by an edge (g_phoneSizing: the edges), or dragged by its strip near a dock.
int g_phoneSizing = 0;
RECT g_phoneSizeFrom{};
POINT g_phoneGrabScreen{};
bool g_phoneDockPicked = false;
DockPick g_phoneDockPick;

void PlacePhoneWindow();

// The docked window moved or came back: the phone goes with it (while it shows).
void CALLBACK PhoneDockEvent(HWINEVENTHOOK, DWORD, HWND wnd, LONG object, LONG, DWORD, DWORD) {
    if (object != OBJID_WINDOW || !wnd || !g_phoneWnd || !IsWindowVisible(g_phoneWnd) || g_phoneState.dockClass.empty()) return;
    WCHAR name[128] = L"";
    if (!GetClassNameW(wnd, name, ARRAYSIZE(name)) || g_phoneState.dockClass != name) return;
    PlacePhoneWindow();
}

void WatchPhoneDock() {
    static HWINEVENTHOOK hook;
    if (!hook)
        hook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_LOCATIONCHANGE, nullptr, PhoneDockEvent, GetCurrentProcessId(), 0,
                               WINEVENT_OUTOFCONTEXT);
}

// Where it goes: docked, by the window it's docked to; else where it was put, else beside the Start menu; on a screen
// either way.
void PlacePhoneWindow() {
    SIZE size = PhoneWindowSize();
    POINT at{g_phoneState.x, g_phoneState.y};
    HWND dock = g_phoneState.dockClass.empty() ? nullptr : FindDockTarget(g_phoneState.dockClass, {g_phoneState.dockX, g_phoneState.dockY});
    if (dock) {
        RECT c;
        GetWindowRect(dock, &c);
        at = DockedAt(c, size, g_phoneState.dockSide, Px(g_phoneState.dockAlong));
        SetWindowPos(g_phoneWnd, HWND_TOPMOST, at.x, at.y, size.cx, size.cy, SWP_NOACTIVATE);
        return;
    }
    if (!g_phoneState.placed) {
        RECT start{};
        if (g_startWnd) GetWindowRect(g_startWnd, &start);
        at = {start.right + Px(6), start.bottom - size.cy};
    }
    MONITORINFO mi{sizeof(mi)};
    RECT want{at.x, at.y, at.x + size.cx, at.y + size.cy};
    GetMonitorInfoW(MonitorFromRect(&want, MONITOR_DEFAULTTONEAREST), &mi);
    at.x = std::clamp((int)at.x, (int)mi.rcWork.left, std::max((int)mi.rcWork.left, (int)mi.rcWork.right - (int)size.cx));
    at.y = std::clamp((int)at.y, (int)mi.rcWork.top, std::max((int)mi.rcWork.top, (int)mi.rcWork.bottom - (int)size.cy));
    SetWindowPos(g_phoneWnd, HWND_TOPMOST, at.x, at.y, size.cx, size.cy, SWP_NOACTIVATE);
}

LRESULT CALLBACK PhoneWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool EnsurePhoneWindow() {
    if (g_phoneWnd) return true;
    if (!g_phoneClass) {
        WNDCLASSW wc{};
        wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = PhoneWndProc;
        wc.hInstance = g_instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kPhoneClass;
        g_phoneClass = RegisterClassW(&wc) != 0;
    }
    g_phoneWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kPhoneClass, L"Phone", WS_POPUP, 0, 0, 1,
                                 1, nullptr, nullptr, g_instance, nullptr);
    if (!g_phoneWnd) return false;
    KeepOutOfPeek(g_phoneWnd);
    BOOL noFade = TRUE;
    DwmSetWindowAttribute(g_phoneWnd, 3 /* DWMWA_TRANSITIONS_FORCEDISABLED */, &noFade, sizeof(noFade));
    return true;
}

void ShowPhoneWindow() {
    if (!EnsurePhoneWindow()) return;
    if (phonemap::LinkNow() == phonemap::Link::Away) phonemap::Wake();  // (the phone may be back: try now)
    if (!g_phoneState.dockClass.empty()) WatchPhoneDock();
    PlacePhoneWindow();
    StartCorners(g_phoneWnd);
    ShowWindow(g_phoneWnd, SW_SHOWNOACTIVATE);
    InvalidateRect(g_phoneWnd, nullptr, FALSE);
    if (g_phoneState.pinned) SetTimer(g_phoneWnd, kTimerPhoneRead, 60000, nullptr);
    PhoneSeen();
}

// As the tray's flyout it takes the focus (and closes when it goes); otherwise never, so the Start menu
// keeps it.
bool g_phoneFlyout = false;

void SetPhoneActivates(bool activates) {
    if (!g_phoneWnd) return;
    LONG_PTR ex = GetWindowLongPtrW(g_phoneWnd, GWL_EXSTYLE);
    SetWindowLongPtrW(g_phoneWnd, GWL_EXSTYLE, activates ? ex & ~WS_EX_NOACTIVATE : ex | WS_EX_NOACTIVATE);
}

void HidePhoneWindow() {
    if (!g_phoneWnd) return;
    KillTimer(g_phoneWnd, kTimerPhoneRead);
    g_phoneHot = g_phonePressed = -1;
    if (g_phoneFlyout) {
        g_phoneFlyout = false;
        SetPhoneActivates(false);
    }
    ShowWindow(g_phoneWnd, SW_HIDE);
}

// The tray's phone clicked (startMenu.phoneTray): the phone as its flyout, above it as System Icons' flyouts
// are placed, or closed again. False: Phone Link opens as usual.
// asked: opened from elsewhere (the notification centre's phone), whatever startMenu.phoneTray says.
bool PhoneFromTray(RECT const& cell, bool asked) {
    if (!g_startConfig.enabled || !g_startConfig.phone || (!g_startConfig.phoneTray && !asked) || !PhoneLinked()) return false;
    if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) {
        if (g_phoneFlyout) HidePhoneWindow();  // (a second click closes it)
        else SetWindowPos(g_phoneWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        return true;
    }
    if (!EnsureStartWindow() || !EnsurePhoneWindow()) return false;
    if (g_phoneState.hidden) {
        g_phoneState.hidden = false;  // (asked for from the tray: it's wanted after all)
        SavePhoneState();
    }
    g_phoneView = PhoneNow();
    PhoneReadAgain();
    SIZE size = PhoneWindowSize();
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromRect(&cell, MONITOR_DEFAULTTONEAREST), &mi);
    RECT const& m = mi.rcMonitor;
    int gap = Px(8);
    int x = (cell.left + cell.right) / 2 - size.cx / 2;
    int y = cell.top - gap - size.cy;                                  // above a bar at the bottom
    if (cell.top < (m.top + m.bottom) / 2) y = cell.bottom + gap;      // below one at the top
    x = std::clamp(x, (int)m.left + gap, std::max((int)m.left + gap, (int)m.right - (int)size.cx - gap));
    y = std::clamp(y, (int)m.top, std::max((int)m.top, (int)m.bottom - (int)size.cy));
    g_phoneFlyout = true;
    SetPhoneActivates(true);
    SetWindowPos(g_phoneWnd, HWND_TOPMOST, x, y, size.cx, size.cy, SWP_SHOWWINDOW);
    StartCorners(g_phoneWnd);
    InvalidateRect(g_phoneWnd, nullptr, FALSE);
    StartForeground(g_phoneWnd);
    PhoneSeen();
    return true;
}

// The flyout kept: a window of its own where it is, kept open (by its pin or title strip).
void PhoneKeepFlyout() {
    if (!g_phoneFlyout) return;
    g_phoneFlyout = false;
    SetPhoneActivates(false);
    RECT r;
    GetWindowRect(g_phoneWnd, &r);
    g_phoneState.apart = g_phoneState.pinned = g_phoneState.placed = true;
    g_phoneState.x = r.left;
    g_phoneState.y = r.top;
    SavePhoneState();
    RelayoutStart();
    SetTimer(g_phoneWnd, kTimerPhoneRead, 60000, nullptr);
    InvalidateRect(g_phoneWnd, nullptr, FALSE);
}

// Out of the Start menu into its own window, or back.
void PhoneSetApart(bool apart) {
    g_phoneState.apart = apart;
    SavePhoneState();
    RelayoutStart();
    if (apart && PhoneWanted()) ShowPhoneWindow();
    else HidePhoneWindow();
}

void PhoneClose() {
    g_phoneState.hidden = true;
    SavePhoneState();
    HidePhoneWindow();
    RelayoutStart();
}

void PhoneShowAgain() {
    g_phoneState.hidden = false;
    SavePhoneState();
    RelayoutStart();
    if (g_phoneState.apart && PhoneWanted() && (g_phoneState.pinned || (g_startWnd && IsWindowVisible(g_startWnd)))) ShowPhoneWindow();
}

void PhoneSetPinned(bool pinned) {
    g_phoneState.pinned = pinned;
    SavePhoneState();
    if (g_phoneWnd) {
        if (pinned) SetTimer(g_phoneWnd, kTimerPhoneRead, 60000, nullptr);
        else KillTimer(g_phoneWnd, kTimerPhoneRead);
        InvalidateRect(g_phoneWnd, nullptr, FALSE);
    }
}

// Files picked to send to the phone, over the network as LocalSend does (phone-files.cpp).
void PhoneSendFiles() {
    std::vector<std::wstring> files;
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) return;
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
    dialog->SetTitle(L"Send to your phone (LocalSend open on it)");
    dialog->SetOkButtonLabel(L"Send");
    IShellItemArray* picked = nullptr;
    if (SUCCEEDED(dialog->Show(nullptr)) && SUCCEEDED(dialog->GetResults(&picked))) {
        DWORD count = 0;
        picked->GetCount(&count);
        for (DWORD i = 0; i < count; i++) {
            IShellItem* item = nullptr;
            PWSTR path = nullptr;
            if (SUCCEEDED(picked->GetItemAt(i, &item)) && SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                files.push_back(path);
                CoTaskMemFree(path);
            }
            if (item) item->Release();
        }
        picked->Release();
    }
    dialog->Release();
    phonefiles::SendFiles(files, nullptr);
}

// Into Phone Link: the phone, one of its pages, or its notifications. With Tourne holding the phone instead,
// its notifications are in Tourne's notification centre, the phone's settings in Bluetooth's, and files go
// over the network as LocalSend sends them (an iPhone takes none over Bluetooth).
void PhoneRun(int hit) {
    if (PhoneTexting()) {
        if (hit == kSmPhoneAction + 2) {
            HideStart();
            PhoneSendFiles();
            return;
        }
        // A notification: its app on this PC to answer it in (Discord...), when it has one there; else the centre.
        if (hit >= kSmPhoneNote && hit - kSmPhoneNote < (int)g_phoneView.notes.size()) {
            auto const& note = g_phoneView.notes[hit - kSmPhoneNote];
            std::wstring open = PhoneAppOnPc(note.package, note.app);
            if (!open.empty()) {
                HideStart();
                ShellExecuteW(nullptr, nullptr, open.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                return;
            }
        }
        bool notes = hit >= kSmPhoneNote || hit == kSmPhoneAction + 1;
        if (notes || hit == kSmPhoneHead || hit == kSmPhoneAction + 3) {
            HideStart();
            if (notes) OpenNotificationCentre(g_clockRect);
            else ShellExecuteW(nullptr, nullptr, L"ms-settings:bluetooth", nullptr, nullptr, SW_SHOWNORMAL);
            return;
        }
    }
    PCWSTR target = hit == kSmPhoneHead ? kPhoneLinkApp
                    : hit >= kSmPhoneNote ? L"ms-phone:phonenotifications"
                    : hit >= kSmPhoneAction && hit < kSmPhoneAction + (int)std::size(kPhoneActions) ? kPhoneActions[hit - kSmPhoneAction].target
                                                                                                   : nullptr;
    if (!target) return;
    HideStart();
    ShellExecuteW(nullptr, nullptr, target, nullptr, nullptr, SW_SHOWNORMAL);
}

////////////////////////////////////////////////////////////////////////////////
// A conversation opened in the phone (wherever it shows), read and replied to.

// The phone laid out again where it shows (its window or the Start menu), its reply box with it.
void PhoneRelayout() {
    if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) {
        RECT was;
        GetWindowRect(g_phoneWnd, &was);
        if (g_phoneFlyout) {
            // (the tray's flyout grows or shrinks away from the bar it's on)
            SIZE size = PhoneWindowSize();
            MONITORINFO mi{sizeof(mi)};
            GetMonitorInfoW(MonitorFromRect(&was, MONITOR_DEFAULTTONEAREST), &mi);
            bool upward = was.top > (mi.rcMonitor.top + mi.rcMonitor.bottom) / 2;
            int y = upward ? was.bottom - size.cy : was.top;
            SetWindowPos(g_phoneWnd, nullptr, was.left, std::max((int)mi.rcMonitor.top, y), size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
        } else {
            PlacePhoneWindow();
        }
        StartCorners(g_phoneWnd);
        InvalidateRect(g_phoneWnd, nullptr, FALSE);
        PhoneSyncReply(g_phoneWnd, g_phoneWly);
    } else if (g_startWnd && IsWindowVisible(g_startWnd)) {
        RelayoutStart();
        PhoneSyncReply(g_startWnd, g_sly.phone ? g_sly.ply : PhoneLayout{});
    }
}

// The keyboard to the reply box: its window brought forward first when it's the phone's own (which never takes
// the focus on a click otherwise; the Start menu stays open for it).
void PhoneFocusReply() {
    if (!g_phoneReply || !IsWindowVisible(g_phoneReply)) return;
    HWND host = GetParent(g_phoneReply);
    if (GetForegroundWindow() != host) StartForeground(host);
    SetFocus(g_phoneReply);
}

// The wheel over the open conversation (pt on the screen): older texts up, newer down. False: not over it.
bool PhoneThreadWheel(HWND wnd, PhoneLayout const& P, POINT pt, int delta) {
    ScreenToClient(wnd, &pt);
    if (g_phoneChat.empty() || !PtInRect(&P.thread, pt)) return false;
    int was = g_phoneThreadScroll;
    g_phoneThreadScroll = std::clamp(g_phoneThreadScroll + delta * Px(60) / WHEEL_DELTA, 0, g_phoneThreadMax);
    if (g_phoneThreadScroll != was) InvalidateRect(wnd, &P.thread, FALSE);
    return true;
}

void PhoneOpenChat(std::wstring const& key) {
    g_phoneThreadScroll = 0;  // (from its newest)
    g_phoneChat = key;
    phonemap::MarkRead(key);
    phonemap::Wake();
    PhoneRelayout();
    PhoneFocusReply();
}

void PhoneCloseChat() {
    bool hadFocus = g_phoneReply && GetFocus() == g_phoneReply;
    g_phoneChat.clear();
    if (g_phoneReply) SetWindowTextW(g_phoneReply, L"");
    PhoneRelayout();
    if (hadFocus && g_startWnd && GetForegroundWindow() == g_startWnd) SetFocus(g_startEdit);
}

// What's typed, sent to the open conversation (only while the phone is connected: else it stays typed).
void PhoneSendReply() {
    if (!g_phoneReply) return;
    int n = GetWindowTextLengthW(g_phoneReply);
    std::wstring text(n, L'\0');
    if (n) GetWindowTextW(g_phoneReply, text.data(), n + 1);
    size_t from = text.find_first_not_of(L" \t"), to = text.find_last_not_of(L" \t");
    text = from == std::wstring::npos ? L"" : text.substr(from, to - from + 1);
    PhoneConvo const* c = PhoneChatConvo();
    if (text.empty() || !c) return;
    if (g_phoneLink != phonemap::Link::Ready) {
        MessageBeep(MB_ICONWARNING);
        phonemap::Wake();
        return;
    }
    if (phonemap::Send(c->address, c->name == c->address ? L"" : c->name, text)) SetWindowTextW(g_phoneReply, L"");
}

// The phone's texts changed (phone-map.cpp tells): shown again, and a pop-up for each new one that isn't
// already in sight (its conversation open with the phone showing).
size_t g_phoneTextsSeen = SIZE_MAX;

void PhoneTextsChanged() {
    g_phoneTexts = phonemap::TextsNow();
    g_phoneLink = phonemap::LinkNow();
    bool first = g_phoneTextsSeen == SIZE_MAX;
    size_t from = first ? g_phoneTexts.size() : std::min(g_phoneTextsSeen, g_phoneTexts.size());
    g_phoneTextsSeen = g_phoneTexts.size();
    bool phoneShows = (g_phoneWnd && IsWindowVisible(g_phoneWnd)) || (g_startWnd && IsWindowVisible(g_startWnd) && g_sly.phone);
    for (size_t i = from; i < g_phoneTexts.size(); i++) {
        auto const& t = g_phoneTexts[i];
        if (t.kind != L'i') continue;
        std::wstring key = phonemap::Key(t.address);
        if (phoneShows && key == g_phoneChat) phonemap::MarkRead(key);  // (posts again, read)
        else phonemap::Toast(t.name.empty() ? t.address : t.name, t.body);
    }
    PhoneBuildConvos();
    if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) InvalidateRect(g_phoneWnd, nullptr, FALSE);
    if (g_startWnd && IsWindowVisible(g_startWnd) && g_sly.phone) {
        RECT pane{g_sly.placesRight, 0, g_sly.width, g_sly.body};
        InvalidateRect(g_startWnd, &pane, FALSE);
    }
    if (g_phoneReply) InvalidateRect(g_phoneReply, nullptr, TRUE);  // (its prompt says whether it can send)
}

// A click on one of the phone's: its buttons, a conversation, else into Phone Link.
void PhoneActivate(int hit) {
    if (hit >= kSmPhoneConvo && hit < kSmPhoneConvo + 100) {
        if (hit - kSmPhoneConvo < (int)g_phoneConvos.size()) PhoneOpenChat(g_phoneConvos[hit - kSmPhoneConvo].key);
        return;
    }
    if (hit == kSmPhoneTexts) return PhoneCloseChat();
    if (hit == kSmPhoneSend) {
        PhoneSendReply();
        PhoneFocusReply();
        return;
    }
    if (g_phoneFlyout) {
        // As the tray's flyout: X closes it; Keep open or Apart keeps it where it is, as its own window.
        if (hit == kSmPhoneClose) return HidePhoneWindow();
        if (hit == kSmPhonePin || hit == kSmPhoneDetach) return PhoneKeepFlyout();
    }
    if (hit == kSmPhoneClose) PhoneClose();
    else if (hit == kSmPhoneDetach) PhoneSetApart(!g_phoneState.apart);
    else if (hit == kSmPhonePin) PhoneSetPinned(!g_phoneState.pinned);
    else if (hit != kSmPhoneStrip) PhoneRun(hit);
}

// Notifications cleared from the pane, as the notification centre clears them: gone from Windows' list and from
// the pane at once (the count with them). Not read again now: Windows' database can lag behind the clear, and
// the next read (the next time the phone shows) has it right.
void PhoneRepaint() {
    if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) InvalidateRect(g_phoneWnd, nullptr, FALSE);
    if (g_startWnd && IsWindowVisible(g_startWnd) && g_sly.phone) {
        RECT pane{g_sly.placesRight, 0, g_sly.width, g_sly.body};
        InvalidateRect(g_startWnd, &pane, FALSE);
    }
}

void PhoneClearNotes(std::vector<PhoneNote> const& gone);

// A middle click on the phone: a notification or a conversation cleared, as from their menu.
void PhoneMiddleClick(int hit) {
    if (hit >= kSmPhoneNote && hit < kSmPhoneConvo && hit - kSmPhoneNote < (int)g_phoneView.notes.size()) {
        PhoneNote note = g_phoneView.notes[hit - kSmPhoneNote];
        if (phoneancs::kCanClear && !note.aumid.empty()) PhoneClearNotes({note});
    } else if (hit >= kSmPhoneConvo && hit < kSmPhoneConvo + 100) {
        PhoneClearConvoAt(hit - kSmPhoneConvo);
    }
}

void PhoneClearNotes(std::vector<PhoneNote> const& gone) {
    for (auto const& note : gone) {
        if (note.aumid.empty()) continue;
        phoneancs::Clear(note.tag, note.group, note.aumid);
        auto& notes = g_phoneView.notes;
        for (auto it = notes.begin(); it != notes.end(); ++it)
            if (it->aumid == note.aumid && it->tag == note.tag && it->group == note.group) {
                notes.erase(it);
                if (g_phoneView.unread > 0) g_phoneView.unread--;
                break;
            }
    }
    if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) InvalidateRect(g_phoneWnd, nullptr, FALSE);
    if (g_startWnd && IsWindowVisible(g_startWnd) && g_sly.phone) {
        RECT pane{g_sly.placesRight, 0, g_sly.width, g_sly.body};
        InvalidateRect(g_startWnd, &pane, FALSE);
    }
}

// Right-click on the phone: a notification's Clear (on one), where it's shown, and closing it; Phone Link only
// while it's what holds the phone.
void PhoneMenu(HWND owner, int hit) {
    enum : int { kOpen = 1, kApart, kPinned, kClose, kUndock, kResetSize, kSettings, kClearNote, kClearApp, kClearAll, kClearConvo };
    HMENU menu = CreatePopupMenu();
    int noteAt = hit - kSmPhoneNote;
    bool onNote = hit >= kSmPhoneNote && hit < kSmPhoneConvo && noteAt < (int)g_phoneView.notes.size();
    PhoneNote note = onNote ? g_phoneView.notes[noteAt] : PhoneNote{};
    int convoAt = hit - kSmPhoneConvo;
    bool onConvo = hit >= kSmPhoneConvo && hit < kSmPhoneConvo + 100 && convoAt < (int)g_phoneConvos.size();
    if (onConvo) {
        AppendMenuW(menu, MF_STRING, kClearConvo, L"Clear (back with its next text)");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    if (phoneancs::kCanClear && onNote && !note.aumid.empty()) {
        AppendMenuW(menu, MF_STRING, kClearNote, L"Clear");
        int fromApp = 0;
        for (auto const& n : g_phoneView.notes) fromApp += n.aumid == note.aumid;
        if (fromApp > 1) AppendMenuW(menu, MF_STRING, kClearApp, (L"Clear all from " + note.app).c_str());
        if ((int)g_phoneView.notes.size() > fromApp || g_phoneView.unread > fromApp)
            AppendMenuW(menu, MF_STRING, kClearAll, L"Clear all");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    if (!PhoneTexting()) {
        AppendMenuW(menu, MF_STRING, kOpen, L"Open Phone Link");
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, kApart, g_phoneState.apart ? L"Back into Start" : L"Move out of Start");
    if (g_phoneState.apart) {
        AppendMenuW(menu, MF_STRING | (g_phoneState.pinned ? MF_CHECKED : 0), kPinned, L"Keep open");
        // Docked by dragging it to a side of a bar, the pager or Start (Shift while dragging: not docked).
        if (!g_phoneState.dockClass.empty()) AppendMenuW(menu, MF_STRING, kUndock, L"Undock (stay where it is)");
        if (g_phoneState.w || g_phoneState.h) AppendMenuW(menu, MF_STRING, kResetSize, L"Size as it comes");
    }
    AppendMenuW(menu, MF_STRING, kClose, L"Close");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kSettings, L"Phone settings...");
    POINT pt;
    GetCursorPos(&pt);
    int cmd = RunStartMenu(owner, menu, TPM_RIGHTBUTTON, pt.x, pt.y);
    if (cmd == kClearNote) return PhoneClearNotes({note});
    if (cmd == kClearConvo) return PhoneClearConvoAt(convoAt);
    if (cmd == kClearApp || cmd == kClearAll) {
        // Every one Windows has from that app (or all of the phone's), not only the ones the pane shows.
        std::vector<PhoneNote> gone;
        for (auto const& n : g_phoneView.notes)
            if (cmd == kClearAll || n.aumid == note.aumid) gone.push_back(n);
        if (cmd == kClearAll) {
            // (each app's first, which notes them as cleared; HideAll after takes the rest)
            for (auto const& n : gone) phoneancs::ClearApp(n.aumid);
            phoneancs::HideAll();
        } else {
            phoneancs::ClearApp(note.aumid);
        }
        return PhoneClearNotes(gone);
    }
    if (cmd == kUndock) {
        g_phoneState.dockClass.clear();
        SavePhoneState();
        return;
    }
    if (cmd == kResetSize) {
        g_phoneState.w = g_phoneState.h = 0;
        SavePhoneState();
        if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) {
            PlacePhoneWindow();
            InvalidateRect(g_phoneWnd, nullptr, FALSE);
        }
        return;
    }
    if (cmd == kSettings) {
        HideStart();
        OpenOwnSettings(L"startmenu.phone");
        return;
    }
    if (g_phoneFlyout && (cmd == kApart || cmd == kPinned)) return PhoneKeepFlyout();
    if (g_phoneFlyout && cmd == kClose) return HidePhoneWindow();
    if (cmd == kOpen) PhoneRun(kSmPhoneHead);
    else if (cmd == kApart) PhoneSetApart(!g_phoneState.apart);
    else if (cmd == kPinned) PhoneSetPinned(!g_phoneState.pinned);
    else if (cmd == kClose) PhoneClose();
}

// Pulled off the Start menu by its head: it's apart from now on, and follows the pointer until let go.
void PhoneTearOff(HWND start, POINT pt) {
    RECT window;
    GetWindowRect(start, &window);
    SIZE size = PhoneWindowSize();
    // Held at the same spot of its head, which sits under the title strip in its own window.
    POINT grab{pt.x - g_sly.ply.head.left, pt.y - g_sly.ply.head.top + g_phoneWly.head.top};
    grab.x = std::clamp((int)grab.x, 0, (int)size.cx - 1);
    POINT screen = pt;
    ClientToScreen(start, &screen);
    g_phoneState.apart = true;
    g_phoneState.placed = true;
    g_phoneState.x = screen.x - grab.x;
    g_phoneState.y = screen.y - grab.y;
    g_phoneState.dockClass.clear();  // (it follows the pointer now; let go by a window's side, it docks there)
    SavePhoneState();
    g_smPressed = g_smHot = -1;
    ReleaseCapture();
    RelayoutStart();
    ShowPhoneWindow();
    g_phoneDragging = true;
    g_phoneDockPicked = false;
    g_phoneGrab = grab;
    SetCapture(g_phoneWnd);
}

LRESULT CALLBACK PhoneWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEACTIVATE: {
            // (else the Start menu keeps the focus, and stays open; a click in the reply box takes it)
            if (g_phoneFlyout) return MA_ACTIVATE;
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(wnd, &pt);
            return !g_phoneChat.empty() && PtInRect(&g_phoneWly.reply, pt) ? MA_ACTIVATE : MA_NOACTIVATE;
        }
        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE || g_startInMenu) return 0;
            // The tray's flyout closes when something else is clicked, as a flyout does; so does the Start menu
            // left open while a reply was typed here.
            if (g_phoneFlyout) HidePhoneWindow();
            else if (g_startWnd && IsWindowVisible(g_startWnd) && (HWND)lParam != g_startWnd) HideStart();
            return 0;
        case WM_CTLCOLOREDIT:
            if ((HWND)lParam == g_phoneReply) return PhoneReplyColors((HDC)wParam);
            break;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            RECT client;
            GetClientRect(wnd, &client);
            HDC mem = CreateCompatibleDC(dc);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
            HGDIOBJ oldBitmap = SelectObject(mem, bitmap);
            // The places' ground and the style's frame, as a pane of the Start menu.
            Fill(mem, client, g_sl.ground);
            if (!SpDraw(mem, kSpRight, 1, client)) Fill(mem, client, g_settings.palette.ground);
            StartFrame(mem, client, g_sl.border ? g_sl.border : g_sl.edge);
            SetBkMode(mem, TRANSPARENT);
            HGDIOBJ oldFont = SelectObject(mem, StartFont());
            PaintPhone(mem, g_phoneWly, g_phoneHot, g_phonePressed, true);
            PhoneSyncReply(wnd, g_phoneWly);
            SelectObject(mem, oldFont);
            BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, oldBitmap);
            DeleteObject(bitmap);
            DeleteDC(mem);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT && !g_phoneDragging) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(wnd, &pt);
                int edges = g_phoneSizing ? g_phoneSizing : EdgesAt(wnd, pt, kEdgeLeft | kEdgeTop | kEdgeRight | kEdgeBottom);
                if (edges) {
                    SetCursor(LoadCursorW(nullptr, EdgeCursor(edges)));
                    return TRUE;
                }
            }
            break;
        case WM_MOUSEWHEEL:
            // Over its open conversation: it scrolls.
            PhoneThreadWheel(wnd, g_phoneWly, POINT{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}, GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
        case WM_MOUSEMOVE: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_phoneSizing) {
                // Pulled by an edge: the opposite edges stay where they were.
                POINT screen = pt;
                ClientToScreen(wnd, &screen);
                int dx = screen.x - g_phoneGrabScreen.x, dy = screen.y - g_phoneGrabScreen.y;
                RECT r = g_phoneSizeFrom;
                int minW = Px(200), minH = Px(120);
                if (g_phoneSizing & kEdgeLeft) r.left = std::min(r.left + dx, r.right - minW);
                if (g_phoneSizing & kEdgeRight) r.right = std::max(r.right + dx, r.left + minW);
                if (g_phoneSizing & kEdgeTop) r.top = std::min(r.top + dy, r.bottom - minH);
                if (g_phoneSizing & kEdgeBottom) r.bottom = std::max(r.bottom + dy, r.top + minH);
                g_phoneState.w = (int)std::lround((r.right - r.left) / g_scale);
                g_phoneState.h = (int)std::lround((r.bottom - r.top) / g_scale);
                SIZE size = PhoneWindowSize();
                if (g_phoneSizing & kEdgeTop) r.top = r.bottom - size.cy;  // (no shorter than its contents)
                SetWindowPos(wnd, nullptr, r.left, r.top, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            if (g_phoneDragging) {
                POINT screen = pt;
                ClientToScreen(wnd, &screen);
                RECT r;
                GetWindowRect(wnd, &r);
                RECT want{screen.x - g_phoneGrab.x, screen.y - g_phoneGrab.y, 0, 0};
                want.right = want.left + (r.right - r.left);
                want.bottom = want.top + (r.bottom - r.top);
                // Near a side of a bar, the pager or the Start menu: it snaps on (Shift: never).
                g_phoneDockPicked = !(GetKeyState(VK_SHIFT) & 0x8000) && PickDock(wnd, want, g_phoneDockPick);
                SetWindowPos(wnd, nullptr, want.left, want.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                return 0;
            }
            if (!g_phoneTracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                g_phoneTracking = true;
            }
            int hit = PhoneHitTest(g_phoneWly, pt);
            if (hit != g_phoneHot) {
                g_phoneHot = hit;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            g_phoneTracking = false;
            if (g_phoneHot != -1) {
                g_phoneHot = -1;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;
        case WM_LBUTTONDOWN: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (int edges = EdgesAt(wnd, pt, kEdgeLeft | kEdgeTop | kEdgeRight | kEdgeBottom)) {
                PhoneKeepFlyout();  // (a flyout resized stays, open)
                g_phoneSizing = edges;
                GetWindowRect(wnd, &g_phoneSizeFrom);
                g_phoneGrabScreen = pt;
                ClientToScreen(wnd, &g_phoneGrabScreen);
                SetCapture(wnd);
                return 0;
            }
            g_phonePressed = PhoneHitTest(g_phoneWly, pt);
            if (g_phonePressed == kSmPhoneStrip) {
                PhoneKeepFlyout();  // (a flyout moved stays where it's put, open)
                g_phoneDragging = true;
                g_phoneDockPicked = false;
                g_phoneGrab = pt;
            }
            SetCapture(wnd);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }
        case WM_LBUTTONUP: {
            int pressed = g_phonePressed;
            g_phonePressed = -1;
            bool dragged = g_phoneDragging, sized = g_phoneSizing != 0;
            g_phoneDragging = false;
            g_phoneSizing = 0;
            ReleaseCapture();
            if (dragged || sized) {
                // Kept where it was put, at its size; docked when it was let go snapped to a window's side.
                RECT r;
                GetWindowRect(wnd, &r);
                g_phoneState.placed = true;
                g_phoneState.x = r.left;
                g_phoneState.y = r.top;
                if (dragged) {
                    g_phoneState.dockClass = g_phoneDockPicked ? g_phoneDockPick.cls : L"";
                    g_phoneState.dockSide = g_phoneDockPick.side;
                    g_phoneState.dockAlong = (int)std::lround(g_phoneDockPick.along / g_scale);
                    g_phoneState.dockX = g_phoneDockPick.middle.x;
                    g_phoneState.dockY = g_phoneDockPick.middle.y;
                    if (g_phoneDockPicked) WatchPhoneDock();
                } else if (!g_phoneState.dockClass.empty()) {
                    PlacePhoneWindow();  // (resized while docked: it stays on its side)
                }
                SavePhoneState();
                return 0;
            }
            int hit = PhoneHitTest(g_phoneWly, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            InvalidateRect(wnd, nullptr, FALSE);
            if (hit == pressed && hit != -1) PhoneActivate(hit);
            return 0;
        }
        case WM_CAPTURECHANGED:
            if (g_phoneSizing && (HWND)lParam != wnd) {
                g_phoneSizing = 0;
                SavePhoneState();
            }
            if (g_phoneDragging && (HWND)lParam != wnd) {
                g_phoneDragging = false;
                RECT r;
                GetWindowRect(wnd, &r);
                g_phoneState.placed = true;
                g_phoneState.x = r.left;
                g_phoneState.y = r.top;
                SavePhoneState();
            }
            return 0;
        case WM_RBUTTONUP: {
            bool startOpen = g_startWnd && IsWindowVisible(g_startWnd);
            if (!startOpen) SetForegroundWindow(wnd);
            PhoneMenu(startOpen ? g_startWnd : wnd, PhoneHitTest(g_phoneWly, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}));
            return 0;
        }
        case WM_MBUTTONUP:
            PhoneMiddleClick(PhoneHitTest(g_phoneWly, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}));
            return 0;
        case WM_TIMER:
            if (wParam == kTimerPhoneRead && IsWindowVisible(wnd) && g_startWnd)
                PhoneReadAgain();
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// The places, less those taken off, without a separator at either end or two
// together.
void LoadPlaces() {
    g_placesValid = true;
    g_places.clear();
    for (auto const& s : g_startConfig.places) {
        StartPlace p = ResolvePlace(s);
        if (p.hidden) continue;
        if (p.special == 3 && (g_places.empty() || g_places.back().special == 3)) continue;
        g_places.push_back(p);
    }
    while (!g_places.empty() && g_places.back().special == 3) g_places.pop_back();
}

// Windhawk isn't the foreground program, so its window may be refused focus.
void StartForeground(HWND wnd) {
    SetForegroundWindow(wnd);
    if (GetForegroundWindow() == wnd) return;
    HWND fg = GetForegroundWindow();
    DWORD fgThread = fg ? GetWindowThreadProcessId(fg, nullptr) : 0, me = GetCurrentThreadId();
    bool attached = fgThread && fgThread != me && AttachThreadInput(me, fgThread, TRUE);
    SetForegroundWindow(wnd);
    BringWindowToTop(wnd);
    if (attached) AttachThreadInput(me, fgThread, FALSE);
}

void SetPlaceChange(std::wstring const& key, PCWSTR change) {
    RegSetKeyValueW(HKEY_CURRENT_USER, (std::wstring(kStartKey) + L"\\Places").c_str(), key.c_str(), REG_SZ, change,
                    (DWORD)((wcslen(change) + 1) * sizeof(WCHAR)));
}

// Right-click on a place, as StartAllBack has it: the shell's own menu for
// it, then whether it's a menu, and taking it off the list.
void PlaceMenu(HWND wnd, int index) {
    if (index < 0 || index >= (int)g_places.size()) return;
    StartPlace const place = g_places[index];
    constexpr int kAsMenu = 0x7F01, kRemove = 0x7F02;
    CloseShellMenu();
    std::wstring target = place.folder.empty() ? place.target : place.folder;
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (!target.empty() && SUCCEEDED(SHParseDisplayName(target.c_str(), nullptr, &pidl, 0, nullptr)) && pidl) {
        IShellItem* item = nullptr;
        if (SUCCEEDED(SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&item)))) {
            item->BindToHandler(nullptr, BHID_SFUIObject, IID_PPV_ARGS(&g_shellMenu.menu));
            item->Release();
        }
        CoTaskMemFree(pidl);
    }
    g_shellMenu.popup = CreatePopupMenu();
    if (g_shellMenu.menu) {
        g_shellMenu.menu->QueryInterface(IID_PPV_ARGS(&g_shellMenu.menu2));
        g_shellMenu.menu->QueryInterface(IID_PPV_ARGS(&g_shellMenu.menu3));
        UINT query = CMF_NORMAL | (GetKeyState(VK_SHIFT) < 0 ? CMF_EXTENDEDVERBS : 0);
        if (SUCCEEDED(g_shellMenu.menu->QueryContextMenu(g_shellMenu.popup, 0, 1, 0x7EFF, query)) &&
            GetMenuItemCount(g_shellMenu.popup))
            AppendMenuW(g_shellMenu.popup, MF_SEPARATOR, 0, nullptr);
    }
    if (!place.folder.empty() || place.special == 1)
        AppendMenuW(g_shellMenu.popup, MF_STRING | (place.kind == kKindMenu ? MF_CHECKED : 0), kAsMenu,
                    L"Display as a menu");
    AppendMenuW(g_shellMenu.popup, MF_STRING, kRemove, L"Remove from this list");
    GetCursorPos(&g_shellMenu.pt);
    g_menuLook = StartMenuLook();
    g_startInMenu = true;
    g_shellMenu.open = true;
    int cmd = (int)TrackStyledMenu(g_shellMenu.popup, TPM_RETURNCMD | TPM_RIGHTBUTTON, g_shellMenu.pt.x,
                                   g_shellMenu.pt.y, wnd, true);
    g_shellMenu.open = false;
    g_startInMenu = false;
    if (cmd == kAsMenu || cmd == kRemove) {
        CloseShellMenu();
        SetPlaceChange(place.key, cmd == kRemove ? L"hidden" : place.kind == kKindMenu ? L"link" : L"menu");
        LoadPlaces();
        StartComputeLayout();
        StartViewWidth();
        BuildStartRows();
        PlaceStartWindow(wnd);
        StartShape(wnd);
        InvalidateRect(wnd, nullptr, FALSE);
    } else if (cmd > 0 && g_shellMenu.menu) {
        g_shellMenu.command = cmd;
        HideStart();
        RunShellCommand(wnd);
    } else {
        CloseShellMenu();
    }
}

LRESULT CALLBACK StartWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool EnsureStartWindow() {
    if (g_startWnd) return true;
    if (!g_startClass) {
        WNDCLASSW wc{};
        wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = StartWndProc;
        wc.hInstance = g_instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kStartClass;
        g_startClass = RegisterClassW(&wc) != 0;
    }
    g_startWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kStartClass, L"Start", WS_POPUP | WS_CLIPCHILDREN,
                                 0, 0, 1, 1, nullptr, nullptr, g_instance, nullptr);
    if (!g_startWnd) return false;
    KeepOutOfPeek(g_startWnd);
    BOOL noFade = TRUE;  // no DWM fade on showing and hiding it
    DwmSetWindowAttribute(g_startWnd, 3 /* DWMWA_TRANSITIONS_FORCEDISABLED */, &noFade, sizeof(noFade));
    return true;
}

// Square, or round for Proper 11, as Windows 11's own.
void StartCorners(HWND wnd) {
    DWORD corners = g_startConfig.round ? 2 /* DWMWCP_ROUND */ : 1 /* DWMWCP_DONOTROUND */;
    DwmSetWindowAttribute(wnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
}

// Shell extensions load their DLLs the first time a context menu is asked for, which is
// what makes the first right-click on a place hang. Ask once, in the background, for each
// place, so that work is done before anyone right-clicks.
HANDLE g_prewarmThread = nullptr;
DWORD WINAPI PrewarmShellMenus(LPVOID param) {
    std::unique_ptr<std::vector<std::wstring>> targets((std::vector<std::wstring>*)param);
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        for (auto const& target : *targets) {
            PIDLIST_ABSOLUTE pidl = nullptr;
            if (FAILED(SHParseDisplayName(target.c_str(), nullptr, &pidl, 0, nullptr)) || !pidl) continue;
            IShellItem* item = nullptr;
            IContextMenu* menu = nullptr;
            if (SUCCEEDED(SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&item)))) {
                item->BindToHandler(nullptr, BHID_SFUIObject, IID_PPV_ARGS(&menu));
                item->Release();
            }
            CoTaskMemFree(pidl);
            if (!menu) continue;
            if (HMENU popup = CreatePopupMenu()) {
                menu->QueryContextMenu(popup, 0, 1, 0x7EFF, CMF_NORMAL);
                DestroyMenu(popup);
            }
            menu->Release();
        }
        CoUninitialize();
    }
    return 0;
}

void PrewarmMenus() {
    static bool started = false;
    if (started) return;
    started = true;
    auto* targets = new std::vector<std::wstring>;
    for (auto const& place : g_places) {
        std::wstring target = place.folder.empty() ? place.target : place.folder;
        if (!target.empty() && !target.starts_with(L"ms-settings:") && target != L"wt.exe") targets->push_back(target);
    }
    g_prewarmThread = CreateThread(nullptr, 0, PrewarmShellMenus, targets, 0, nullptr);
    if (g_prewarmThread) SetThreadPriority(g_prewarmThread, THREAD_PRIORITY_BELOW_NORMAL);
    else delete targets;
}

// The icons the home list and the places will draw, as PaintStart asks for them.
std::vector<std::pair<std::wstring, int>> WantedStartIcons() {
    std::vector<std::pair<std::wstring, int>> wanted;
    int icon = StartIconPx();
    for (auto const& row : g_rows) {
        if (row.kind == kRowSeparator || row.kind == kRowHeader) continue;
        wanted.push_back({row.kind == kRowFolder && row.node && !row.node->dirs.empty() ? row.node->dirs[0] : row.parsing, icon});
    }
    if (g_startConfig.glyphIcons)
        for (auto const& place : g_places) wanted.push_back({PlaceIconSource(place), Px(16)});
    return wanted;
}
void ShowStart() {
    BootMark(L"start menu opened");
    if (!EnsureStartWindow()) return;
    HidePreview();
    StartTakeApps();  // the read that ran in the background, if it has finished
    if (g_startApps.empty() && !StartAppsReading()) ReadStartApps();
    if (!g_placesValid) LoadPlaces();
    if (g_startConfig.phone && !g_phoneState.hidden) {
        // The phone as last read now, and read again in the background (it comes back as WM_APP_PHONE). Read
        // even when none was linked last time: one linked since makes its pane come.
        g_phoneView = PhoneNow();
        PhoneReadAgain();
        if (PhoneWanted() && phonemap::LinkNow() == phonemap::Link::Away) phonemap::Wake();  // (the phone may be back: try now)
        if (PhoneWanted() && !g_phoneState.apart) PhoneSeen();
    }
    StartComputeLayout();
    g_startResetting = true;
    SetWindowTextW(g_startEdit, L"");
    g_startResetting = false;
    g_allRoot = {};
    g_startView = kViewHome;
    g_scrollPx = 0;
    g_smHot = g_smPressed = -1;
    BuildStartRows();

    HDC dc = GetDC(nullptr);
    HGDIOBJ old = SelectObject(dc, StartFont());
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    RECT s = g_sly.search;
    int h = tm.tmHeight;
    SendMessageW(g_startEdit, WM_SETFONT, (WPARAM)StartFont(), FALSE);
    SetWindowPos(g_startEdit, nullptr, s.left + Px(10), s.top + (s.bottom - s.top - h) / 2, s.right - s.left - Px(34), h,
                 SWP_NOZORDER | SWP_NOACTIVATE);

    PlaceStartWindow(g_startWnd);
    StartShape(g_startWnd);
    StartCorners(g_startWnd);
    g_inMenu = true;  // the bar stays out while it's open
    bool moving = StartMotionOpen();  // (layered at its first frame before it shows)
    ShowWindow(g_startWnd, SW_SHOW);
    StartForeground(g_startWnd);
    SetFocus(g_startEdit);
    InvalidateRect(g_startWnd, nullptr, FALSE);
    if (moving) UpdateWindow(g_startWnd);  // drawn before its first frame
    if (PhoneWanted() && g_phoneState.apart) ShowPhoneWindow();  // (beside it, or where it was put)
    PrewarmMenus();
}

////////////////////////////////////////////////////////////////////////////////
// Tooltips: a program's own description (its shortcut's comment), and
// Windows 7's words for its places, as StartAllBack shows them.

std::wstring g_startTipText;
std::unordered_map<std::wstring, std::wstring> g_startInfoTips;

std::wstring ShellInfoTip(std::wstring const& parsing) {
    if (parsing.empty() || parsing.starts_with(L"ms-settings:")) return {};
    auto it = g_startInfoTips.find(parsing);
    if (it != g_startInfoTips.end()) return it->second;
    std::wstring text;
    PIDLIST_ABSOLUTE pidl = nullptr;
    if (SUCCEEDED(SHParseDisplayName(parsing.c_str(), nullptr, &pidl, 0, nullptr)) && pidl) {
        IShellFolder* parent = nullptr;
        PCUITEMID_CHILD child = nullptr;
        if (SUCCEEDED(SHBindToParent(pidl, IID_PPV_ARGS(&parent), &child))) {
            IQueryInfo* info = nullptr;
            if (SUCCEEDED(parent->GetUIObjectOf(nullptr, 1, &child, IID_IQueryInfo, nullptr, (void**)&info))) {
                PWSTR tip = nullptr;
                if (SUCCEEDED(info->GetInfoTip(QITIPF_DEFAULT, &tip)) && tip) {
                    text = tip;
                    CoTaskMemFree(tip);
                }
                info->Release();
            }
            parent->Release();
        }
        CoTaskMemFree(pidl);
    }
    g_startInfoTips[parsing] = text;
    return text;
}

std::wstring StartTipText(int hit) {
    if (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size()) {
        StartPlace const& place = g_places[hit - kSmPlace];
        std::wstring item = place.key.substr(0, place.key.find(L'|'));
        static constexpr struct {
            PCWSTR item, tip;
        } kTips[] = {
            {L"user", L"Open your personal folder."},
            {L"documents", L"Access letters, reports, notes, and other kinds of documents."},
            {L"pictures", L"View and organize digital pictures."},
            {L"music", L"Play music and other audio files."},
            {L"videos", L"Watch home movies and other digital videos."},
            {L"computer", L"See the disk drives and other hardware connected to this computer."},
            {L"network", L"Access computers and devices that are on your network."},
            {L"controlPanel", L"Change settings and customize the functionality of your computer."},
            {L"devicesAndPrinters", L"View and manage devices, printers, and print jobs."},
            {L"defaultPrograms",
             L"Choose default programs for web browsing, e-mail, playing music, and other activities."},
            {L"run", L"Opens a program, folder, document, or web site."},
        };
        for (auto const& t : kTips)
            if (item == t.item) return t.tip;
        return {};
    }
    if (hit >= 0 && hit < (int)g_rows.size() && g_rows[hit].kind == kRowItem) {
        std::wstring tip = ShellInfoTip(g_rows[hit].parsing);
        return tip == g_rows[hit].name ? std::wstring() : tip;
    }
    return {};
}

// The tip follows what's lit: its rectangle and text, shown again after the
// usual pause.
void UpdateStartTip(HWND wnd, int hit) {
    if (!g_startTip) return;
    g_startTipText = StartTipText(hit);
    RECT r{};
    if (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size()) {
        r = g_sly.places[hit - kSmPlace];
    } else if (hit >= 0 && hit < (int)g_rows.size()) {
        auto const& row = g_rows[hit];
        int top = g_sly.list.top + row.top - g_scrollPx;
        r = {g_sly.list.left, top, g_sly.list.right, top + row.height};
    }
    TTTOOLINFOW ti{(UINT)TTTOOLINFOW_V2_SIZE};
    ti.hwnd = wnd;
    ti.uId = 1;
    ti.rect = r;
    SendMessageW(g_startTip, TTM_POP, 0, 0);
    SendMessageW(g_startTip, TTM_NEWTOOLRECTW, 0, (LPARAM)&ti);
}

// The Start button: ours if it's on (true), else Windows' opens.
bool ToggleStartMenu() {
    if (!g_startConfig.enabled) return false;
    if (g_startWnd && IsWindowVisible(g_startWnd) && !StartClosing()) HideStart();
    else if (GetTickCount() - g_startHiddenAt > 200) ShowStart();  // not the click that just closed it (closing, it comes back)
    return true;
}

LRESULT CALLBACK StartEditProc(HWND edit, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    HWND wnd = GetParent(edit);
    switch (msg) {
        case WM_KEYDOWN:
            // An open cascade has the arrows, Enter and Esc.
            if (CascadeKey(wParam)) return 0;
            // Right on a place with a menu opens it, its first item lit.
            if (wParam == VK_RIGHT && g_smHot >= kSmPlace && g_smHot < kSmPlace + (int)g_places.size() &&
                g_places[g_smHot - kSmPlace].kind != kKindLink) {
                OpenPlaceFlyout(wnd, g_smHot - kSmPlace);
                CascadeKey(VK_DOWN);
                return 0;
            }
            switch (wParam) {
                case VK_DOWN: StartMove(wnd, 1); return 0;
                case VK_UP: StartMove(wnd, -1); return 0;
                case VK_RETURN: {
                    int hit = g_smHot;
                    if (!Selectable(hit))
                        for (int i = 0; i < (int)g_rows.size() && hit < 0; i++)
                            if (g_rows[i].kind == kRowItem) hit = i;
                    StartActivate(wnd, hit);
                    return 0;
                }
                case VK_ESCAPE:
                    if (GetWindowTextLengthW(edit)) SetWindowTextW(edit, L"");
                    else if (g_startView == kViewAll) SetStartView(wnd, kViewHome);
                    else HideStart();
                    return 0;
            }
            break;
        case WM_CHAR:
            if (wParam == VK_RETURN || wParam == VK_ESCAPE || wParam == L'\t') return 0;
            break;
        case WM_PAINT: {
            LRESULT result = DefSubclassProc(edit, msg, wParam, lParam);
            if (!GetWindowTextLengthW(edit)) {
                HDC dc = GetDC(edit);
                HGDIOBJ old = SelectObject(dc, (HFONT)SendMessageW(edit, WM_GETFONT, 0, 0));
                SetBkMode(dc, TRANSPARENT);
                // StartAllBack's prompt is a light grey (#96A1A2 measured in Bouquet), not the style's dim green.
                SetTextColor(dc, PaletteRef(StartMix(g_sl.ground, 0xFFFFFF, 0.55)));
                RECT r;
                GetClientRect(edit, &r);
                r.left += 2;
                std::wstring cue = StartCase(L"Search programs and files");
                DrawTextW(dc, cue.c_str(), -1, &r, DT_SINGLELINE | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS);
                SelectObject(dc, old);
                ReleaseDC(edit, dc);
            }
            return result;
        }
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            InvalidateRect(wnd, &g_sly.search, FALSE);
            InvalidateRect(edit, nullptr, TRUE);
            break;
        case WM_NCDESTROY:
            RemoveWindowSubclass(edit, StartEditProc, 0);
            break;
    }
    return DefSubclassProc(edit, msg, wParam, lParam);
}

LRESULT CALLBACK StartWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            g_startEdit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, wnd,
                                          (HMENU)1, g_instance, nullptr);
            if (g_startEdit) SetWindowSubclass(g_startEdit, StartEditProc, 0, 0);
            g_startTip = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, TOOLTIPS_CLASSW, nullptr,
                                         WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 0, 0, 0, 0, wnd, nullptr, g_instance,
                                         nullptr);
            if (g_startTip) {
                TTTOOLINFOW ti{(UINT)TTTOOLINFOW_V2_SIZE};
                ti.uFlags = TTF_SUBCLASS;
                ti.hwnd = wnd;
                ti.uId = 1;
                ti.lpszText = LPSTR_TEXTCALLBACKW;
                SendMessageW(g_startTip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
                SendMessageW(g_startTip, TTM_SETMAXTIPWIDTH, 0, Px(420));
            }
            return 0;

        case WM_NOTIFY: {
            auto* header = (NMHDR*)lParam;
            if (header->hwndFrom == g_startTip && header->code == TTN_GETDISPINFOW) {
                auto* info = (NMTTDISPINFOW*)lParam;
                info->lpszText = (LPWSTR)g_startTipText.c_str();
                if (g_sl.menuFont) SendMessageW(g_startTip, WM_SETFONT, (WPARAM)g_sl.menuFont, FALSE);
                return 0;
            }
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            RECT client;
            GetClientRect(wnd, &client);
            // One back buffer kept between paints, and only the invalid part drawn and copied:
            // pointing at an item repaints two rows, not the whole menu.
            if (!g_paintBitmap || g_paintSize.cx != client.right || g_paintSize.cy != client.bottom) {
                if (g_paintBitmap) DeleteObject(g_paintBitmap);
                g_paintBitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
                g_paintSize = {client.right, client.bottom};
                ps.rcPaint = client;
            }
            HDC mem = CreateCompatibleDC(dc);
            HGDIOBJ old = SelectObject(mem, g_paintBitmap);
            IntersectClipRect(mem, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
            PaintStart(wnd, mem);
            BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
                   mem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
            SelectObject(mem, old);
            DeleteDC(mem);
            EndPaint(wnd, &ps);
            return 0;
        }

        case WM_CTLCOLOREDIT: {
            if ((HWND)lParam == g_phoneReply) return PhoneReplyColors((HDC)wParam);
            // The typing sits on the search box's own ground (hatched, in
            // StartAllBack's look), lined up with the box drawn behind it.
            HDC dc = (HDC)wParam;
            HBRUSH brush = StartSearchBrush(StartSearchState());
            RECT edit;
            GetWindowRect((HWND)lParam, &edit);
            MapWindowPoints(nullptr, wnd, (POINT*)&edit, 2);
            SetBrushOrgEx(dc, g_sly.search.left - edit.left, g_sly.search.top - edit.top, nullptr);
            SetTextColor(dc, PaletteRef(g_sl.text));
            SetBkMode(dc, TRANSPARENT);
            if (brush) return (LRESULT)brush;
            SetDCBrushColor(dc, PaletteRef(g_sl.box));
            return (LRESULT)GetStockObject(DC_BRUSH);
        }

        case WM_APP_SEARCHED: {
            // The search thread's settings and files, if still wanted.
            auto* result = (SearchResult*)lParam;
            if (result->generation == g_searchGeneration && g_startView == kViewSearch) {
                g_searchHits = std::move(result->hits);
                g_searchPending = false;
                int hot = g_smHot;
                BuildStartRows();
                g_smHot = Selectable(hot) ? hot : -1;
                for (int i = 0; i < (int)g_rows.size() && g_smHot < 0; i++)
                    if (g_rows[i].kind == kRowItem) g_smHot = i;
                InvalidateRect(wnd, nullptr, FALSE);
            }
            delete result;
            return 0;
        }

        case WM_APP_PHONE_TEXTS:
            PhoneTextsChanged();
            return 0;

        case WM_APP_PHONE_NOTES:
            // Read again only while the phone is in sight (else the next time it shows reads it), a moment later:
            // Windows' notification database takes a little while to have the change.
            if ((g_phoneWnd && IsWindowVisible(g_phoneWnd)) || (IsWindowVisible(wnd) && g_sly.phone))
                SetTimer(wnd, kTimerPhoneNotes, 1500, nullptr);
            return 0;

#ifdef TD_PREFIX
        case WM_COPYDATA: {  // Tourne Calls: a call's state
            auto* data = (COPYDATASTRUCT*)lParam;
            if (data->dwData != kCallData || data->cbData < sizeof(CallData)) break;
            CallData call;
            memcpy(&call, data->lpData, sizeof(call));
            call.name[127] = call.mic[127] = call.out[127] = 0;
            CallChanged(call);
            return TRUE;
        }
#endif

        default: {
            // A phone notification's button clicked in System Icons' pop-up or centre: its action on the phone.
            static UINT const phoneAction = RegisterWindowMessageW(L"TournePhoneAction");
            // The notification centre's phone clicked, with Tourne holding the phone: 0 the phone, 1 its texts (both:
            // the phone as a flyout by the pointer), 2 Send a file, 3 Bluetooth's settings.
            static UINT const phoneOpen = RegisterWindowMessageW(L"TournePhoneOpen");
            if (msg == phoneOpen) {
                if (wParam == 2) {
                    PhoneSendFiles();
                } else if (wParam == 3) {
                    ShellExecuteW(nullptr, nullptr, L"ms-settings:bluetooth", nullptr, nullptr, SW_SHOWNORMAL);
                } else {
                    POINT pt;
                    GetCursorPos(&pt);
                    PhoneFromTray(RECT{pt.x - 1, pt.y - 1, pt.x + 1, pt.y + 1}, true);
                }
                return 0;
            }
            if (msg == phoneAction) {
                phoneancs::Perform((uint32_t)wParam, (uint8_t)lParam);
                return 0;
            }
#ifdef TD_PREFIX
            static UINT const callLevels = RegisterWindowMessageW(L"TourneCallsLevels");
            if (msg == callLevels) {
                g_callMic = std::max(g_callMic, std::sqrt((float)wParam / 1000.f));
                g_callCaller = std::max(g_callCaller, std::sqrt((float)lParam / 1000.f));
                return 0;
            }
#endif
            break;
        }

        case WM_APP_PHONE: {
            g_phoneView = PhoneNow();
            if (g_phoneWnd && IsWindowVisible(g_phoneWnd)) InvalidateRect(g_phoneWnd, nullptr, FALSE);
            if (!IsWindowVisible(wnd)) return 0;
            if (g_sly.phone != (PhoneWanted() && !g_phoneState.apart)) {
                RelayoutStart();  // linked or unlinked since the layout: the pane comes or goes
            } else if (g_sly.phone) {
                RECT pane{g_sly.placesRight, 0, g_sly.width, g_sly.body};
                InvalidateRect(wnd, &pane, FALSE);
            }
            return 0;
        }

        case WM_COMMAND:
            // The typing sits on a patterned ground: redrawn whole, so a
            // deleted letter leaves no trace.
            if ((HWND)lParam == g_startEdit && HIWORD(wParam) == EN_UPDATE) InvalidateRect(g_startEdit, nullptr, TRUE);
            if ((HWND)lParam == g_startEdit && HIWORD(wParam) == EN_CHANGE && !g_startResetting) {
                bool searching = GetWindowTextLengthW(g_startEdit) > 0;
                if (searching) {
                    StartSearch(wnd, StartSearchText());
                } else {
                    InterlockedIncrement((LONG*)&g_searchGeneration);  // any search running is dropped
                    g_searchHits.clear();
                    g_searchPending = false;
                }
                SetStartView(wnd, searching ? kViewSearch : kViewHome);
                if (searching)  // the first result, for Enter
                    for (int i = 0; i < (int)g_rows.size(); i++)
                        if (g_rows[i].kind == kRowItem) {
                            g_smHot = i;
                            break;
                        }
            }
            return 0;

        case WM_SETFOCUS:
            if (g_startEdit) SetFocus(g_startEdit);
            return 0;

        case WM_ACTIVATE:
            // (the phone beside it taking the keyboard for a reply keeps it open)
            if (LOWORD(wParam) == WA_INACTIVE && !g_startInMenu && !(g_phoneWnd && (HWND)lParam == g_phoneWnd)) HideStart();
            return 0;

        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT && wnd == g_startWnd) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(wnd, &pt);
                int edges = g_startSizing ? g_startSizing : EdgesAt(wnd, pt, StartSizingEdges());
                if (edges) {
                    SetCursor(LoadCursorW(nullptr, EdgeCursor(edges)));
                    return TRUE;
                }
            }
            break;

        case WM_MOUSEMOVE: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_startSizing) {
                // Pulled by an edge: the list takes the room (the window stays by the bar, so the edge follows).
                POINT screen = pt;
                ClientToScreen(wnd, &screen);
                int dx = screen.x - g_startGrab.x, dy = screen.y - g_startGrab.y;
                int w = g_startFromW + (int)std::lround(((g_startSizing & kEdgeRight) ? dx : (g_startSizing & kEdgeLeft) ? -dx : 0) / g_scale);
                int h = g_startFromH + (int)std::lround(((g_startSizing & kEdgeBottom) ? dy : (g_startSizing & kEdgeTop) ? -dy : 0) / g_scale);
                w = std::clamp(w, 0, 2000);
                h = std::clamp(h, 0, 2000);
                if (w != g_startExtraW || h != g_startExtraH) {
                    g_startExtraW = w;
                    g_startExtraH = h;
                    RelayoutStart();
                }
                return 0;
            }
            if (g_scrollDrag) {
                int room = g_sly.list.bottom - g_sly.list.top;
                g_scrollPx = g_scrollGrabPx + (pt.y - g_scrollGrabY) * (g_rowsHeight - room) / StartScrollGeometry().travel;
                ClampScroll();
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            if (!g_startTracking) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
                TrackMouseEvent(&tme);
                g_startTracking = true;
            }
            // The phone's head pulled away: it comes off the Start menu into its own window.
            if (g_smPressed == kSmPhoneHead && (wParam & MK_LBUTTON) &&
                (abs(pt.x - g_phoneDown.x) > GetSystemMetrics(SM_CXDRAG) * 2 || abs(pt.y - g_phoneDown.y) > GetSystemMetrics(SM_CYDRAG) * 2)) {
                PhoneTearOff(wnd, pt);
                return 0;
            }
            int hit = StartHitTest(pt);
            if (hit == kSmScroll) InvalidateRect(wnd, &g_sly.scroll, FALSE);  // its arrows and thumb light up
            if (hit != g_smHot) {
                int oldHot = g_smHot;
                size_t cascadeCount = g_cascades.size();
                int flyPlace = g_flyPlace;
                bool flyFooter = g_flyFooter;
                g_smHot = hit;
                KillTimer(wnd, kTimerFlyout);
                UpdateStartTip(wnd, hit);
                // Onto anything else in the Start menu: an open cascade closes
                // (empty space leaves it be, on the way over to it).
                bool owner = (g_flyPlace >= 0 && hit == kSmPlace + g_flyPlace) || (g_flyFooter && hit == kSmFooter);
                if (!g_cascades.empty() && hit != -1 && !owner) CloseCascades(0);
                // Menus open on a pause over them only as StartAllBack is set.
                if (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size() && g_places[hit - kSmPlace].kind != kKindLink)
                    PrefetchFolder(g_places[hit - kSmPlace].folder);  // read ahead, in the background
                if (g_startConfig.autoCascade && hit >= kSmPlace && hit < kSmPlace + (int)g_places.size() &&
                    g_places[hit - kSmPlace].kind != kKindLink)
                    SetTimer(wnd, kTimerFlyout, 150, nullptr);  // Windows' 400 ms feels sluggish
                if (cascadeCount != g_cascades.size() || flyPlace != g_flyPlace || flyFooter != g_flyFooter) {
                    InvalidateRect(wnd, nullptr, FALSE);
                } else {
                    RECT before = StartHitRect(wnd, oldHot), after = StartHitRect(wnd, hit), both;
                    UnionRect(&both, &before, &after);
                    if (!IsRectEmpty(&both)) InvalidateRect(wnd, &both, FALSE);
                }
            }
            return 0;
        }

        case WM_MOUSELEAVE:
            g_startTracking = false;
            KillTimer(wnd, kTimerFlyout);
            if (g_smHot != -1) {
                RECT was = StartHitRect(wnd, g_smHot);
                g_smHot = -1;
                if (!IsRectEmpty(&was)) InvalidateRect(wnd, &was, FALSE);
            }
            return 0;

        case WM_MOUSEWHEEL: {
            // Over a cascade: it scrolls (the wheel comes here, with the focus).
            POINT screen{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            HWND under = WindowFromPoint(screen);
            for (auto const& c : g_cascades)
                if (c->wnd == under) {
                    ScrollCascade(*c, -GET_WHEEL_DELTA_WPARAM(wParam) * Px(60) / WHEEL_DELTA);
                    return 0;
                }
            // Over the phone's open conversation: it scrolls.
            if (g_sly.phone && PhoneThreadWheel(wnd, g_sly.ply, screen, GET_WHEEL_DELTA_WPARAM(wParam))) return 0;
        }
            if (g_startView != kViewHome) {
                g_scrollPx -= GET_WHEEL_DELTA_WPARAM(wParam) * 3 * g_sly.row / WHEEL_DELTA;
                ClampScroll();
                POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                ScreenToClient(wnd, &pt);
                g_smHot = StartHitTest(pt);
                InvalidateRect(wnd, nullptr, FALSE);
            }
            return 0;

        case WM_LBUTTONDOWN: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (int edges = EdgesAt(wnd, pt, StartSizingEdges())) {
                CloseCascades(0);
                g_startSizing = edges;
                g_startGrab = pt;
                ClientToScreen(wnd, &g_startGrab);
                g_startFromW = g_startExtraW;
                g_startFromH = g_startExtraH;
                SetCapture(wnd);
                return 0;
            }
            g_smPressed = StartHitTest(pt);
            g_phoneDown = pt;
            // A click elsewhere closes a cascade; its own place toggles it.
            bool owner = (g_flyPlace >= 0 && g_smPressed == kSmPlace + g_flyPlace) || (g_flyFooter && g_smPressed == kSmFooter);
            if (!owner) CloseCascades(0);
            if (g_smPressed == kSmScroll) {
                int room = g_sly.list.bottom - g_sly.list.top;
                StartScrollBar b = StartScrollGeometry();
                if (PtInRect(&b.up, pt) || PtInRect(&b.down, pt)) {  // the arrows: a row at a time
                    g_scrollPx += (PtInRect(&b.up, pt) ? -1 : 1) * g_sly.row;
                    ClampScroll();
                } else if (!PtInRect(&b.thumb, pt)) {  // the track: a page at a time
                    g_scrollPx += (pt.y < b.thumb.top ? -1 : 1) * (room - g_sly.row);
                    ClampScroll();
                } else {
                    g_scrollDrag = true;
                    g_scrollGrabY = pt.y;
                    g_scrollGrabPx = g_scrollPx;
                }
            }
            SetCapture(wnd);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }

        case WM_LBUTTONUP: {
            if (g_startSizing) {
                g_startSizing = 0;
                ReleaseCapture();
                SaveStartSize();
                return 0;
            }
            int pressed = g_smPressed;
            g_smPressed = -1;
            g_scrollDrag = false;
            ReleaseCapture();
            int hit = StartHitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            InvalidateRect(wnd, nullptr, FALSE);
            if (hit == pressed && hit != kSmScroll && hit != -1) StartActivate(wnd, hit);
            return 0;
        }

        case WM_CAPTURECHANGED:
            g_scrollDrag = false;
            if (g_startSizing && (HWND)lParam != wnd) {
                g_startSizing = 0;
                SaveStartSize();
            }
            return 0;

        case WM_MBUTTONUP: {
            // On the phone's notifications and conversations: cleared.
            int hit = StartHitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (IsPhoneHit(hit)) PhoneMiddleClick(hit);
            return 0;
        }
        case WM_RBUTTONUP: {
            int hit = StartHitTest({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (hit >= 0 && hit < (int)g_rows.size()) {
                ProgramMenu(wnd, g_rows[hit]);
            } else if (hit >= kSmPlace && hit < kSmPlace + (int)g_places.size()) {
                PlaceMenu(wnd, hit - kSmPlace);
            } else if (IsPhoneHit(hit)) {
                PhoneMenu(wnd, hit);
            } else if (hit == kSmFooter && g_startView == kViewHome) {
                StartBlankMenu(wnd, true);
            } else if (hit == -1) {
                StartBlankMenu(wnd, false);
            }
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }

        case WM_TIMER:
            if (wParam == kTimerStartMotion) {
                StartMotionFrame();
            } else if (wParam == kTimerPhoneNotes) {
                KillTimer(wnd, kTimerPhoneNotes);
                PhoneReadAgain();
            } else if (wParam == kTimerFlyout) {
                KillTimer(wnd, kTimerFlyout);
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(wnd, &pt);
                int hit = StartHitTest(pt);
                if (IsWindowVisible(wnd) && hit == g_smHot && hit >= kSmPlace && hit < kSmPlace + (int)g_places.size())
                    OpenPlaceFlyout(wnd, hit - kSmPlace);
            } else if (wParam == kTimerApps) {
                KillTimer(wnd, kTimerApps);
                if (!IsWindowVisible(wnd)) {
                    // Idle: the list of programs read again in the background, and the home list
                    // (whatever was launched changes its frequent programs) built ready for the next open,
                    // with the icons it and the places will draw fetched in the background too.
                    // (Not when the settings were just applied: those read the list already.)
                    StartTakeApps();
                    if (!g_appsJustRead) StartRefreshApps();
                    g_appsJustRead = false;
                    g_homeValid = false;
                    StartView view = g_startView;
                    g_startView = kViewHome;
                    BuildStartRows();
                    g_startView = view;
                    if (!g_placesValid) LoadPlaces();
                    // And All Programs' top level: its folders' names and icons (on the thread), and its apps'
                    // icons (at the top, or in their folder).
                    auto wanted = WantedStartIcons();
                    int all = AllProgramsIconPx();
                    if (g_startConfig.appsInFolder) wanted.push_back({L"shell:AppsFolder", all});
                    for (auto const& app : g_startApps)
                        if (app.parsing.find(L'!') != std::wstring::npos) wanted.push_back({app.parsing, all});
                    WarmStartIcons(std::move(wanted), true);
                    PrewarmMenus();
                }
            }
            return 0;

        case WM_MEASUREITEM:
            if (MenuMeasureItem((MEASUREITEMSTRUCT*)lParam)) return TRUE;
            if (LRESULT result; g_shellMenu.menu && ShellMenuMessage(msg, wParam, lParam, &result)) return TRUE;
            break;

        case WM_DRAWITEM:
            if (MenuDrawItem((DRAWITEMSTRUCT*)lParam)) return TRUE;
            if (LRESULT result; g_shellMenu.menu && ShellMenuMessage(msg, wParam, lParam, &result)) return TRUE;
            break;

        case WM_INITMENUPOPUP: {
            if (g_shellMenu.open) {
                LRESULT result;
                ShellMenuMessage(msg, wParam, lParam, &result);
                StyleMenuItems((HMENU)wParam, true);
                return 0;
            }
            // A folder in a flyout: what's inside, as it opens.
            HMENU menu = (HMENU)wParam;
            std::wstring target;
            for (auto& folder : g_fly.folders) {
                if (folder.menu == menu && !folder.filled) {
                    folder.filled = true;
                    target = folder.target;
                    break;
                }
            }
            if (!target.empty()) {
                FillFolderMenu(menu, target);
                StyleMenuItems(menu, false);
            }
            return 0;
        }

        case WM_MENUCHAR:
            if (LRESULT result; g_shellMenu.open && ShellMenuMessage(msg, wParam, lParam, &result)) return result;
            break;

        case WM_MENURBUTTONUP: {
            // Something in a flyout: its own shell menu.
            if (g_shellMenu.menu) return 0;
            MENUITEMINFOW info{sizeof(info)};
            info.fMask = MIIM_ID | MIIM_SUBMENU;
            std::wstring target;
            if (GetMenuItemInfoW((HMENU)lParam, (UINT)wParam, TRUE, &info)) {
                if (info.hSubMenu) {
                    for (auto const& folder : g_fly.folders)
                        if (folder.menu == info.hSubMenu) target = folder.target;
                } else if (info.wID > 0 && info.wID <= g_fly.targets.size()) {
                    target = g_fly.targets[info.wID - 1];
                }
            }
            if (!target.empty() && !target.starts_with(L"ms-settings:") && target != L"wt.exe") OpenShellMenu(wnd, target);
            return 0;
        }

        case WM_DESTROY:
            g_startTip = nullptr;
            g_startEdit = nullptr;
            return 0;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
// Right-click on the Start button: the power user menu (Windows+X's), in the
// Start menu's look, as StartAllBack has it, with Properties for these
// settings.

enum : int { kWxApps = 1, kWxPower, kWxEvents, kWxSystem, kWxDevices, kWxNetwork, kWxDisks, kWxComputer,
             kWxTerminal, kWxTerminalAdmin, kWxTaskManager, kWxSettings, kWxExplorer, kWxSearch, kWxRun, kWxDesktop,
             kWxProperties };
constexpr int kWxShutdown = 100;  // + the power command

bool PowerUserMenu(HWND bar) {
    if (!g_startConfig.enabled) return false;
    HideStart();
    struct Item {
        int id;
        PCWSTR name;
    };
    constexpr Item kTop[] = {
        {kWxApps, L"Apps and &Features"}, {kWxPower, L"Power &Options"}, {kWxEvents, L"Event &Viewer"},
        {kWxSystem, L"S&ystem"}, {kWxDevices, L"Device &Manager"}, {kWxNetwork, L"Net&work Connections"},
        {kWxDisks, L"Dis&k Management"}, {kWxComputer, L"Computer Mana&gement"},
        {kWxTerminal, L"W&indows Terminal"}, {kWxTerminalAdmin, L"Windows Terminal (&Admin)"},
    };
    constexpr Item kMiddle[] = {
        {kWxTaskManager, L"&Task Manager"}, {kWxSettings, L"Setti&ngs"}, {kWxExplorer, L"File &Explorer"},
        {kWxSearch, L"&Search"}, {kWxRun, L"&Run"},
    };
    HMENU menu = CreatePopupMenu();
    for (auto const& item : kTop) AppendMenuW(menu, MF_STRING, item.id, item.name);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    for (auto const& item : kMiddle) AppendMenuW(menu, MF_STRING, item.id, item.name);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    HMENU power = CreatePopupMenu();
    AppendMenuW(power, MF_STRING, kWxShutdown + kPwSignOut, L"Sign &out");
    AppendMenuW(power, MF_STRING, kWxShutdown + kPwSleep, L"S&leep");
    AppendMenuW(power, MF_STRING, kWxShutdown + kPwShutdown, L"Sh&ut down");
    AppendMenuW(power, MF_STRING, kWxShutdown + kPwRestart, L"&Restart");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)power, L"Sh&ut down or sign out");
    AppendMenuW(menu, MF_STRING, kWxDesktop, L"&Desktop");
    AppendMenuW(menu, MF_STRING, kWxProperties, L"P&roperties");

    POINT pt;
    GetCursorPos(&pt);
    HidePreview();
    g_inMenu = true;
    SetForegroundWindow(bar);
    g_menuLook = StartMenuLook();
    int cmd = (int)TrackStyledMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, bar);
    PostMessageW(bar, WM_NULL, 0, 0);
    g_inMenu = false;
    DestroyMenu(menu);

    auto open = [](PCWSTR what, PCWSTR verb = nullptr) {
        ShellExecuteW(nullptr, verb, what, nullptr, nullptr, SW_SHOWNORMAL);
    };
    switch (cmd) {
        case kWxApps: open(L"ms-settings:appsfeatures"); break;
        case kWxPower: open(L"ms-settings:powersleep"); break;
        case kWxEvents: open(L"eventvwr.msc"); break;
        case kWxSystem: open(L"ms-settings:about"); break;
        case kWxDevices: open(L"devmgmt.msc"); break;
        case kWxNetwork: open(L"ms-settings:network"); break;
        case kWxDisks: open(L"diskmgmt.msc"); break;
        case kWxComputer: open(L"compmgmt.msc"); break;
        case kWxTerminal: open(L"wt.exe"); break;
        case kWxTerminalAdmin: open(L"wt.exe", L"runas"); break;
        case kWxTaskManager: open(L"taskmgr.exe"); break;
        case kWxSettings: open(L"ms-settings:"); break;
        case kWxExplorer: open(L"explorer.exe"); break;
        case kWxSearch: ShowStart(); break;  // (Start's own search: type to search)
        case kWxRun: ShowRunDialog(); break;  // (Win+R needs Explorer's taskbar, which shell mode hides)
        case kWxDesktop: ToggleShowDesktop(); break;
        case kWxProperties: OpenStartProperties(); break;
        default:
            if (cmd > kWxShutdown) StartPower(cmd - kWxShutdown);
            break;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// The Windows key

// A lone press opens this Start menu. Windows opens its own when the key comes
// up with nothing pressed in between, so an unused key goes in first (as
// AutoHotkey masks it), and its own release after.
LRESULT CALLBACK StartKeyProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        auto* key = (KBDLLHOOKSTRUCT*)lParam;
        // The unassigned key another part sends while Win is held (the switcher, taking Win+Tab): Win was used.
        if ((key->flags & LLKHF_INJECTED) && key->vkCode == 0xE8 && g_winHeld) g_winUsed = true;
        if (!(key->flags & LLKHF_INJECTED)) {
            bool win = key->vkCode == VK_LWIN || key->vkCode == VK_RWIN;
            bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
            if (win && down) {
                if (!g_winHeld) g_winUsed = false;
                g_winHeld = true;
            } else if (win) {
                bool alone = g_winHeld && !g_winUsed;
                g_winHeld = false;
                if (alone) {
                    INPUT in[3]{};
                    in[0].type = in[1].type = in[2].type = INPUT_KEYBOARD;
                    in[0].ki.wVk = in[1].ki.wVk = 0xE8;  // unassigned
                    in[1].ki.dwFlags = KEYEVENTF_KEYUP;
                    in[2].ki.wVk = (WORD)key->vkCode;
                    in[2].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY;
                    SendInput(3, in, sizeof(INPUT));
                    if (HWND bar = g_wnd) PostMessageW(bar, WM_APP_START, 0, 0);
                    return 1;
                }
            } else if (down && g_winHeld) {
                g_winUsed = true;
            }
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

// Explorer's relay sends every opening of Windows' Start here (without
// StartAllBack), which leaves the keyboard hook nothing to do: no key press
// then passes through this process.
bool ShellSendsStart() {
    HANDLE signal = OpenEventW(SYNCHRONIZE, FALSE, L"TourneSuite_ShellStart_" WH_MOD_ID);
    if (!signal) signal = OpenEventW(SYNCHRONIZE, FALSE, L"TourneSuite_ShellStart");  // (a relay of another build)
    if (signal) CloseHandle(signal);
    return signal != nullptr;
}

void StartKeyHook(bool on) {
    on = on && !ShellSendsStart() && !LeftAloneInFront();  // (let go while a game is in front)
    if (on && !g_startKeyHook) {
        g_startKeyHook = SetWindowsHookExW(WH_KEYBOARD_LL, StartKeyProc, g_instance, 0);
    } else if (!on && g_startKeyHook) {
        UnhookWindowsHookEx(g_startKeyHook);
        g_startKeyHook = nullptr;
    }
    g_winHeld = g_winUsed = false;
}

// With the bar's settings: before the theme is replaced (closing), and after.
void StartBeforeTheme() {
    g_homeValid = g_placesValid = false;
    HideStart();
    StartMotionEnd();  // (hidden now: its look is about to go)
    CloseStartLook();
    ClearStartIcons();
}

#ifdef TD_PREFIX
////////////////////////////////////////////////////////////////////////////////
// A call on: Tourne Calls (desktop/calls) tells its state (WM_COPYDATA kCallData) and, while its sound is on the
// PC, the levels of the mic and the caller ("TourneCallsLevels", 0 to 1000). Shown in its own small window at the
// bottom right, in the Start menu's look (the style's pane and frame, its font, Shut down's buttons): who, how
// long, where the sound is, the mic and output by name with their levels, and Mute, To the phone / To this PC,
// Hang up, which go back to Tourne Calls as its commands (3, 5 / 4, 2).

constexpr WCHAR kCallClass[] = L"TourneStartCall_" WH_MOD_ID;
constexpr UINT_PTR kTimerCall = 1;  // the meters falling back, and the clock

CallData g_call{};
ULONGLONG g_callSince = 0;
HWND g_callWnd = nullptr;
int g_callHot = -1, g_callPressed = -1;

struct CallLayout {
    int line = 0;
    RECT name{}, state{}, mic{}, out{}, buttons[3]{};
};

HFONT CallBoldFont() {
    static HFONT bold, from;
    if (from != StartFont()) {
        if (bold) DeleteObject(bold);
        LOGFONTW lf{};
        GetObjectW(StartFont(), sizeof(lf), &lf);
        lf.lfWeight = FW_SEMIBOLD;
        bold = CreateFontIndirectW(&lf);
        from = StartFont();
    }
    return bold;
}

CallLayout CallLay(int width) {
    CallLayout L;
    HDC dc = GetDC(nullptr);
    HGDIOBJ old = SelectObject(dc, StartFont());
    TEXTMETRICW tm{};
    GetTextMetricsW(dc, &tm);
    SelectObject(dc, old);
    ReleaseDC(nullptr, dc);
    L.line = tm.tmHeight + Px(4);
    int pad = Px(12) + g_sl.frameBorder, right = width - pad, y = pad;
    L.name = {pad, y, right, y + L.line + Px(2)};
    y = L.name.bottom;
    L.state = {pad, y, right, y + L.line};
    y = L.state.bottom + Px(6);
    L.mic = {pad, y, right, y + L.line};
    L.out = {pad, L.mic.bottom, right, L.mic.bottom + L.line};
    y = L.out.bottom + Px(10);
    int gap = Px(6), w = (right - pad - 2 * gap) / 3, h = L.line + Px(10);
    for (int i = 0; i < 3; i++) L.buttons[i] = {pad + i * (w + gap), y, i == 2 ? right : pad + i * (w + gap) + w, y + h};
    return L;
}

SIZE CallWindowSize() {
    int width = Px(320);
    CallLayout L = CallLay(width);
    return {width, L.buttons[0].bottom + Px(12) + g_sl.frameBorder};
}

int CallHit(POINT pt) {
    RECT client;
    GetClientRect(g_callWnd, &client);
    CallLayout L = CallLay(client.right);
    for (int i = 0; i < 3; i++)
        if (PtInRect(&L.buttons[i], pt)) return i;
    return -1;
}

void PaintCall(HDC dc, RECT client) {
    CallLayout L = CallLay(client.right);
    Fill(dc, client, g_sl.ground);
    if (!SpDraw(dc, kSpRight, 1, client)) Fill(dc, client, g_settings.palette.ground);
    StartFrame(dc, client, g_sl.border ? g_sl.border : g_sl.edge);
    SetBkMode(dc, TRANSPARENT);
    HGDIOBJ oldFont = SelectObject(dc, CallBoldFont());

    // Who, and how long.
    std::wstring time;
    if (g_callSince && g_call.status != 2) {
        ULONGLONG s = (GetTickCount64() - g_callSince) / 1000;
        WCHAR t[32];
        if (s >= 3600) swprintf(t, ARRAYSIZE(t), L"%llu:%02llu:%02llu", s / 3600, s / 60 % 60, s % 60);
        else swprintf(t, ARRAYSIZE(t), L"%llu:%02llu", s / 60, s % 60);
        time = t;
    }
    SIZE tw{};
    GetTextExtentPoint32W(dc, time.c_str(), (int)time.size(), &tw);
    RECT name = L.name;
    name.right -= tw.cx + Px(8);
    StartText(dc, g_call.name[0] ? g_call.name : L"Call", name, g_sl.text);
    SelectObject(dc, StartFont());
    StartText(dc, time, L.name, g_sl.dim, DT_RIGHT);
    bool micOff = g_call.muted && g_call.here;
    std::wstring state = g_call.status == 2 ? L"Calling" : g_call.status == 4 ? L"On hold" : g_call.here ? L"On this PC" : L"On the phone";
    if (micOff) state += L", mic off";
    StartText(dc, state, L.state, micOff ? g_sl.accent : g_sl.dim);

    // Where the sound is.
    if (g_call.here && g_call.sound) {
        auto row = [&](RECT r, wchar_t const* label, wchar_t const* device, float level) {
            int labelW = Px(34), barW = Px(64);
            StartText(dc, label, RECT{r.left, r.top, r.left + labelW, r.bottom}, g_sl.dim);
            StartText(dc, device, RECT{r.left + labelW, r.top, r.right - barW - Px(10), r.bottom}, g_sl.text);
            int mid = (r.top + r.bottom) / 2;
            RECT bar{r.right - barW, mid - Px(2), r.right, mid + Px(2)};
            Fill(dc, bar, StartMix(g_sl.ground, g_sl.text, 0.2));
            if (level > 0.02f) Fill(dc, RECT{bar.left, bar.top, bar.left + std::max(Px(3), (int)((bar.right - bar.left) * std::min(level, 1.f))), bar.bottom}, g_sl.accent);
        };
        row(L.mic, L"Mic", g_call.mic, micOff ? 0.f : g_callMic);
        row(L.out, L"Out", g_call.out, g_callCaller);
    } else {
        StartText(dc, !g_call.here ? L"To this PC: talk with its mic and speakers"
                      : g_call.status == 3 ? L"The sound couldn't start here (see calls.log)"
                                           : L"Its sound comes here when it starts",
                  L.mic, g_sl.dim);
    }

    // The buttons, as Shut down's.
    wchar_t const* labels[3] = {g_call.muted ? L"Unmute" : L"Mute", g_call.here ? L"To the phone" : L"To this PC", L"Hang up"};
    for (int i = 0; i < 3; i++) {
        RECT r = L.buttons[i];
        bool hot = g_callHot == i, pressed = hot && g_callPressed == i, lit = i == 0 && g_call.muted;
        Fill(dc, r, pressed || lit ? g_sl.buttonPressed : hot ? g_sl.buttonHot : g_sl.button);
        if (hot && !pressed) StartFrame(dc, RECT{r.left + Px(1), r.top + Px(1), r.right - Px(1), r.bottom - Px(1)}, g_sl.buttonHotEdge);
        StartText(dc, labels[i], r, lit ? g_sl.accent : hot ? g_sl.hotText : g_sl.text, DT_CENTER);
    }
    SelectObject(dc, oldFont);
}

void CallCommand(int what) {
    if (HWND calls = FindWindowExW(HWND_MESSAGE, nullptr, L"TourneCalls_Window", nullptr))
        PostMessageW(calls, RegisterWindowMessageW(L"TourneCallsCommand"), what, 0);
}

LRESULT CALLBACK CallWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            RECT client;
            GetClientRect(wnd, &client);
            HDC mem = CreateCompatibleDC(dc);
            HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
            HGDIOBJ oldBitmap = SelectObject(mem, bitmap);
            PaintCall(mem, client);
            BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
            SelectObject(mem, oldBitmap);
            DeleteObject(bitmap);
            DeleteDC(mem);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_TIMER:
            g_callMic *= 0.8f;
            g_callCaller *= 0.8f;
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_MOUSEMOVE: {
            int hot = CallHit({(short)LOWORD(lParam), (short)HIWORD(lParam)});
            if (hot != g_callHot) g_callHot = hot, InvalidateRect(wnd, nullptr, FALSE);
            TRACKMOUSEEVENT t{sizeof(t), TME_LEAVE, wnd, 0};
            TrackMouseEvent(&t);
            return 0;
        }
        case WM_MOUSELEAVE:
            g_callHot = -1;
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONDOWN:
            g_callPressed = CallHit({(short)LOWORD(lParam), (short)HIWORD(lParam)});
            if (g_callPressed >= 0) SetCapture(wnd);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONUP: {
            int was = g_callPressed;
            g_callPressed = -1;
            ReleaseCapture();
            if (was >= 0 && was == CallHit({(short)LOWORD(lParam), (short)HIWORD(lParam)}))
                CallCommand(was == 0 ? 3 : was == 1 ? (g_call.here ? 5 : 4) : 2);
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// Bottom right of the main screen's work area, clear of the edge as the tray's pop-ups are.
void PlaceCallWindow() {
    SIZE size = CallWindowSize();
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &mi);
    SetWindowPos(g_callWnd, HWND_TOPMOST, mi.rcWork.right - size.cx - Px(12), mi.rcWork.bottom - size.cy - Px(12), size.cx, size.cy,
                 SWP_NOACTIVATE);
}

// Tourne Calls' news: the window shown, changed or hidden.
void CallChanged(CallData const& data) {
    bool was = g_call.on;
    g_call = data;
    if (g_call.on && g_call.seconds != ~0u) {
        ULONGLONG since = GetTickCount64() - (ULONGLONG)g_call.seconds * 1000;
        if (!g_callSince || (since > g_callSince ? since - g_callSince : g_callSince - since) > 1500) g_callSince = since;
    } else {
        g_callSince = 0;
    }
    if (!g_call.on) {
        if (g_callWnd) {
            KillTimer(g_callWnd, kTimerCall);
            ShowWindow(g_callWnd, SW_HIDE);
        }
        g_callMic = g_callCaller = 0;
        return;
    }
    if (!g_callWnd) {
        WNDCLASSW wc{};
        wc.style = CS_DROPSHADOW;
        wc.lpfnWndProc = CallWndProc;
        wc.hInstance = g_instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kCallClass;
        RegisterClassW(&wc);
        g_callWnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kCallClass, L"Call", WS_POPUP, 0, 0, 1, 1, nullptr,
                                    nullptr, g_instance, nullptr);
        if (!g_callWnd) return;
        KeepOutOfPeek(g_callWnd);
        BOOL noFade = TRUE;
        DwmSetWindowAttribute(g_callWnd, 3 /* DWMWA_TRANSITIONS_FORCEDISABLED */, &noFade, sizeof(noFade));
    }
    if (!was || !IsWindowVisible(g_callWnd)) {
        PlaceCallWindow();
        StartCorners(g_callWnd);
        ShowWindow(g_callWnd, SW_SHOWNOACTIVATE);
        SetTimer(g_callWnd, kTimerCall, 50, nullptr);
    }
    InvalidateRect(g_callWnd, nullptr, FALSE);
}

// The theme changed (or Tourne Desktop just started): the window in the new look, and Tourne Calls asked to say
// again what's on (command 10).
void CallsAfterTheme() {
    if (g_callWnd && IsWindowVisible(g_callWnd)) {
        PlaceCallWindow();
        StartCorners(g_callWnd);
        InvalidateRect(g_callWnd, nullptr, FALSE);
    }
    CallCommand(10);
}
#endif

void StartAfterTheme() {
    LoadStartSettings();
    LoadStartLook();
    ShellAsk(g_wnd, kShellStartProp, L"StartKey", g_startConfig.enabled && g_startConfig.winKey);
    StartKeyHook(g_startConfig.enabled && g_startConfig.winKey);
#ifdef TD_PREFIX
    // A program left alone (a game) coming to the front or going: the main bar lets the hook go or puts it back.
    static bool watching = false;
    if (!watching) {
        watching = true;
        td::WatchExcludedFront([](bool inFront) {
            if (HWND bar = g_wnd.load()) PostMessageW(bar, LeftAloneMessage(), inFront, 0);
        });
    }
#endif
    // Warm up: the programs are read in the background now, and the menu's window and lists are
    // built a little later, in idle time, so the first open is as quick as the rest.
    StartRefreshApps();
    g_appsJustRead = true;
    if (g_startConfig.enabled && EnsureStartWindow()) SetTimer(g_startWnd, kTimerApps, 2500, nullptr);
    // The phone's texts held by Tourne while its pane is on (startMenu.phoneMessages): from now on, as they come.
    // So are its notifications (phone-ancs.cpp), made Windows' own, as Phone Link would.
    if (g_startConfig.enabled && g_startConfig.phone && PhoneTexting() && EnsureStartWindow()) {
        phonemap::Begin(g_startWnd, WM_APP_PHONE_TEXTS);
        SetPropW(g_startWnd, L"TourneHoldsPhone", (HANDLE)1);  // (System Icons' centre opens Tourne's phone then)
        phoneancs::Begin(g_startWnd, WM_APP_PHONE_NOTES);
        phoneancs::StartCallsHelper();
#ifdef TD_PREFIX
        CallsAfterTheme();
#endif
        PhoneTextsChanged();
    } else {
        phoneancs::End();
        phonemap::End();
        if (g_startWnd) RemovePropW(g_startWnd, L"TourneHoldsPhone");
    }
    // The phone kept open on its own: up from the start, read now and every minute after.
    if (g_startConfig.enabled && g_phoneState.apart && g_phoneState.pinned && PhoneWanted() && EnsureStartWindow()) {
        g_phoneView = PhoneNow();
        PhoneReadAgain();
        ShowPhoneWindow();
    } else {
        HidePhoneWindow();
    }
}

void DestroyStartMenu() {
    if (g_paintBitmap) DeleteObject(g_paintBitmap);
    g_paintBitmap = nullptr;
    g_paintSize = {};
    FreeFolderCache();
    FreeStartApps();
    FreeWarmIcons();
    if (g_prewarmThread) {  // it must be gone before this code is
        WaitForSingleObject(g_prewarmThread, 3000);
        CloseHandle(g_prewarmThread);
        g_prewarmThread = nullptr;
    }
    StartKeyHook(false);
    phoneancs::End();
    phonemap::End();
    PhoneReadWait();  // (a read under way finished before this code goes)
    CloseCascades(0);
    if (g_cascadeClass) UnregisterClassW(kCascadeClass, g_instance);
    g_cascadeClass = false;
    StartMotionEnd();
    if (g_startWnd) DestroyWindow(g_startWnd);
    g_startWnd = nullptr;
    if (g_phoneWnd) DestroyWindow(g_phoneWnd);
    g_phoneWnd = nullptr;
    g_phoneReply = nullptr;  // (gone with its window)
    if (g_phoneClass) UnregisterClassW(kPhoneClass, g_instance);
    g_phoneClass = false;
    ForgetPhoneIcons();
    g_fly.Clear();
    ClearStartIcons();
    g_startInfoTips.clear();
    CloseStartLook();
    g_allRoot = {};
    g_rows.clear();
    g_startApps.clear();
    g_startAppIds.clear();
    g_startControlPanel.clear();
    g_startControlPanelRead = false;
    if (g_startClass) UnregisterClassW(kStartClass, g_instance);
    g_startClass = false;
}
