// A program's icon everywhere ("// @inline program-icons.cpp", in Icon Redirect, System Icons and the
// Switcher): the taskbar's Button icons rows marked "Use everywhere", read by each part that shows
// programs' icons, so one row gives a program the same icon on the taskbar, in the tray, in Alt+Tab and,
// through Icon Redirect, wherever Windows takes the icon from its file. Or one of System Icons' designs
// (tourne:screenshot), on its buttons, in the tray and in Alt+Tab. A row's program is a file name,
// a full path or an app id, and may hold * and ? (like *\NVIDIA Corporation\*.exe).

struct ProgramIcon {
    std::wstring program;  // lower case
    std::wstring icon;     // a file, or "file,index"; variables expanded; or tourne:design
};

// One of System Icons' own designs, by name (tourne:screenshot), in place of a file: drawn by System Icons
// for the tray, the taskbar's buttons and Alt+Tab; Icon Redirect, which needs an .ico, passes over it.
bool IsTourneDesign(std::wstring const& icon) {
    return _wcsnicmp(icon.c_str(), L"tourne:", 7) == 0;
}

// * matches any run of characters, ? any one. Both sides lower case.
bool ProgramWildcard(std::wstring_view pattern, std::wstring_view text) {
    size_t p = 0, t = 0, star = std::wstring_view::npos, resume = 0;
    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == L'?' || pattern[p] == text[t])) {
            p++;
            t++;
        } else if (p < pattern.size() && pattern[p] == L'*') {
            star = p++;
            resume = t;
        } else if (star != std::wstring_view::npos) {
            p = star + 1;
            t = ++resume;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == L'*') {
        p++;
    }
    return p == pattern.size();
}

// A path from a setting: trimmed, without outer quotes or a stray leading *, variables expanded.
std::wstring ProgramIconPath(std::wstring s) {
    auto trim = [&] {
        while (!s.empty() && iswspace(s.front())) s.erase(0, 1);
        while (!s.empty() && iswspace(s.back())) s.pop_back();
    };
    trim();
    if (s.size() >= 2 && s.front() == L'"' && s.back() == L'"') s = s.substr(1, s.size() - 2);
    while (!s.empty() && s.front() == L'*') s.erase(0, 1);
    trim();
    if (s.find(L'%') == std::wstring::npos) return s;
    WCHAR expanded[MAX_PATH * 2];
    DWORD length = ExpandEnvironmentStringsW(s.c_str(), expanded, ARRAYSIZE(expanded));
    return length && length <= ARRAYSIZE(expanded) ? std::wstring(expanded) : s;
}

std::vector<ProgramIcon> EverywhereIcons() {
    // The taskbar part's settings, by their full names (through variables, so the build doesn't file
    // them under the part reading them).
    constexpr PCWSTR kProgram = L"taskbar.buttonIcons[%d].program";
    constexpr PCWSTR kIcon = L"taskbar.buttonIcons[%d].icon";
    constexpr PCWSTR kEverywhere = L"taskbar.buttonIcons[%d].everywhere";
    std::vector<ProgramIcon> rows;
    for (int i = 0; i < 64; i++) {
        PCWSTR program = Wh_GetStringSetting(kProgram, i);
        std::wstring match = program ? program : L"";
        Wh_FreeStringSetting(program);
        while (!match.empty() && iswspace(match.front())) match.erase(0, 1);
        while (!match.empty() && iswspace(match.back())) match.pop_back();
        if (match.empty()) {
            if (i > 0) break;
            continue;
        }
        PCWSTR icon = Wh_GetStringSetting(kIcon, i);
        std::wstring file = ProgramIconPath(icon ? icon : L"");
        Wh_FreeStringSetting(icon);
        // (A design goes on the program's buttons, tray icon and Alt+Tab alike, without "Use everywhere".)
        if (!Wh_GetIntSetting(kEverywhere, i) && !IsTourneDesign(file)) continue;
        CharLowerBuffW(match.data(), (DWORD)match.size());
        if (!file.empty()) rows.push_back({match, file});
    }
    return rows;
}

// Whether a row stands for a program, by its app id, full path or file name (all lower case).
bool ProgramIconMatches(std::wstring const& row, std::wstring const& appId, std::wstring const& path) {
    std::wstring_view name = path;
    if (size_t slash = name.find_last_of(L"\\/"); slash != std::wstring_view::npos) name = name.substr(slash + 1);
    if (row.find_first_of(L"*?") != std::wstring::npos)
        return (!path.empty() && ProgramWildcard(row, path)) || (!name.empty() && ProgramWildcard(row, name)) ||
               (!appId.empty() && ProgramWildcard(row, appId));
    return (!path.empty() && row == path) || (!name.empty() && row == name) || (!appId.empty() && row == appId);
}

// Tourne'Icons' Taskbar buttons rows, which it hands over as "program<tab>icon" lines (newest first) in
// HKCU\Software\Tourne\Icons: Buttons from its Windhawk mod while that runs, Buttons Kit and Buttons Lasting
// from the other two. They come before these settings' rows. While the Windhawk mod runs it holds the button
// icons (it took these settings' rows in), so these settings' rows step aside. Read again whenever the key
// was written since the last look, so a change shows on the next look at once.
struct TourneIconsButtons {
    std::vector<ProgramIcon> rows;
    bool holds = false;
};

SRWLOCK g_tourneIconsLock = SRWLOCK_INIT;
TourneIconsButtons g_tourneIconsButtons;
FILETIME g_tourneIconsWritten;
bool g_tourneIconsRead;

void ForgetTourneIconsButtons() {
    AcquireSRWLockExclusive(&g_tourneIconsLock);
    g_tourneIconsRead = false;
    ReleaseSRWLockExclusive(&g_tourneIconsLock);
}

TourneIconsButtons ReadTourneIconsButtons() {
    HKEY key = nullptr;
    FILETIME written{};
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Tourne\\Icons", 0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryInfoKeyW(key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                         &written);
    }
    AcquireSRWLockExclusive(&g_tourneIconsLock);
    if (!g_tourneIconsRead || CompareFileTime(&written, &g_tourneIconsWritten) != 0) {
        g_tourneIconsRead = true;
        g_tourneIconsWritten = written;
        TourneIconsButtons fresh;
        for (PCWSTR value : {L"Buttons", L"Buttons Kit", L"Buttons Lasting"}) {
            DWORD size = 0;
            if (!key || RegGetValueW(key, nullptr, value, RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &size) != ERROR_SUCCESS) {
                continue;
            }
            std::vector<WCHAR> text(size / sizeof(WCHAR) + 2, L'\0');
            DWORD got = (DWORD)(text.size() * sizeof(WCHAR));
            if (RegGetValueW(key, nullptr, value, RRF_RT_REG_MULTI_SZ, nullptr, text.data(), &got) != ERROR_SUCCESS) {
                continue;
            }
            if (value[7] == L'\0') fresh.holds = true;  // (Buttons: the Windhawk mod's)
            for (PCWSTR line = text.data(); *line; line += wcslen(line) + 1) {
                std::wstring_view row = line;
                size_t tab = row.find(L'\t');
                if (tab == std::wstring_view::npos) continue;
                std::wstring program(row.substr(0, tab));
                CharLowerBuffW(program.data(), (DWORD)program.size());
                std::wstring icon = ProgramIconPath(std::wstring(row.substr(tab + 1)));
                if (!program.empty() && !icon.empty()) fresh.rows.push_back({program, icon});
            }
        }
        g_tourneIconsButtons = std::move(fresh);
    }
    TourneIconsButtons result = g_tourneIconsButtons;
    ReleaseSRWLockExclusive(&g_tourneIconsLock);
    if (key) RegCloseKey(key);
    return result;
}

// These settings' rows (own) with Tourne'Icons' ahead of them, or in their place while it holds them.
std::vector<ProgramIcon> WithTourneIcons(std::vector<ProgramIcon> const& own) {
    TourneIconsButtons published = ReadTourneIconsButtons();
    if (!published.holds) published.rows.insert(published.rows.end(), own.begin(), own.end());
    return std::move(published.rows);
}
