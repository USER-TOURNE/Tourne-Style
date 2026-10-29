// ==WindhawkMod==
// @id              tourne-mod-tray
// @name            Tourne Mod Tray
// @description     Tray icon with a right-click menu listing all installed Windhawk mods. Click a mod to enable or disable it (a check mark shows enabled mods).
// @version         1.0.1
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @donateUrl       https://ko-fi.com/tourne
// @license         GPL-3.0
// @include         windhawk.exe
// @compilerOptions -lshell32 -lgdi32 -luser32 -lole32 -lshlwapi -luxtheme -ladvapi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tourne Mod Tray

Based on [Mod Manager Tray](https://github.com/Salyts) by Salyts (MIT).

Adds a tray icon (Segoe Fluent Icons glyph `E71D`). **Right-click** it to see
every installed Windhawk mod. A check mark (`E73E`) next to a mod means it is
enabled. Click a mod to toggle it.

A list taller than the screen scrolls with the mouse wheel (3 mods a notch)
while the pointer is over it, as well as with the arrows at its ends.

## How it works

The mod uses `windhawk-cli.exe`:

* `windhawk-cli.exe mod list` - get the list of mods (id, version, state, name)
* `windhawk-cli.exe mod enable <id>` / `mod disable <id>` - toggle a mod

Enabling / disabling requires administrator rights, so a UAC prompt appears
for every toggle (unless the Windhawk process itself is already elevated).

If `mod list` cannot be read without elevation, the list is fetched through an
elevated helper and cached; a **Refresh list** item then appears at the bottom
of the menu.

## Settings

* **Icon color** - auto / white / black
* **Menu color** - auto / light / dark
* **Show mod IDs** - append the mod id to the name in the menu
* **Tray icon** - hex code of a Segoe Fluent Icons glyph (default `E71D`)
* **Whitelist** - mod IDs to show; if empty, all mods are shown
* **Blacklist** - mod IDs to hide (takes priority over the whitelist)
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- icon: E71D
  $name: Tray icon
  $description: Hex code of a Segoe Fluent Icons glyph (4 digits, e.g. E71D).
- icon_color: auto
  $name: Tray icon color
  $options:
  - auto: Auto
  - white: White
  - black: Black
- menu_color: auto
  $name: Context menu color
  $options:
  - auto: Auto
  - light: Light
  - dark: Dark
- show_ids: false
  $name: Show mod IDs
  $description: Append the mod id to the mod name in the menu.
- whitelist: [""]
  $name: Whitelist (mod IDs)
  $description: If not empty, only mods with these IDs are shown in the menu. Leave empty to show all mods.
- blacklist: [""]
  $name: Blacklist (mod IDs)
  $description: Mods with these IDs are hidden from the menu. The blacklist takes priority over the whitelist.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

struct ModInfo {
    std::wstring id;
    std::wstring version;
    std::wstring name;
    bool         enabled = false;
};

static const UINT WM_TRAY_CB = WM_USER + 100;
static const UINT WM_RELOAD  = WM_USER + 101;
static const UINT TRAY_UID   = 1;
static const UINT ID_BASE    = 1000;
static const UINT ID_REFRESH = 999;

static HINSTANCE g_hInstance       = nullptr;
static HWND      g_trayHwnd        = nullptr;
static UINT      g_taskbarCreated  = 0;
static HICON     g_iconWhite       = nullptr;
static HICON     g_iconBlack       = nullptr;
static DWORD     g_lastMenuClose   = 0;

static std::wstring g_iconColorMode = L"auto";
static std::wstring g_menuColorMode = L"auto";
static bool         g_showIds       = false;
static WCHAR        g_iconGlyph     = L'\uE71D';
static std::vector<std::wstring> g_whitelist;
static std::vector<std::wstring> g_blacklist;

static std::wstring        g_cliPath;
static std::wstring        g_cliDir;
static bool                g_isElevated   = false;
static std::mutex          g_modsMutex;
static std::vector<ModInfo> g_mods;
static bool                g_listNeedsAdmin = false;

static bool IsSystemDarkMode() {
    DWORD value = 1, size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER,
                 L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"AppsUseLightTheme", RRF_RT_DWORD, nullptr, &value, &size);
    return value == 0;
}

static bool TrayIconUseWhite() {
    if (g_iconColorMode == L"white") return true;
    if (g_iconColorMode == L"black") return false;
    return IsSystemDarkMode();
}

static bool MenuUseDark() {
    if (g_menuColorMode == L"dark")  return true;
    if (g_menuColorMode == L"light") return false;
    return IsSystemDarkMode();
}

static bool IsProcessElevated() {
    bool result = false;
    HANDLE tok = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tok)) {
        TOKEN_ELEVATION te = {};
        DWORD sz = 0;
        if (GetTokenInformation(tok, TokenElevation, &te, sizeof(te), &sz))
            result = te.TokenIsElevated != 0;
        CloseHandle(tok);
    }
    return result;
}

static std::wstring Trim(std::wstring s) {
    while (!s.empty() && iswspace(s.front())) s.erase(s.begin());
    while (!s.empty() && iswspace(s.back()))  s.pop_back();
    return s;
}

static std::wstring Decode(const std::string& s) {
    if (s.empty()) return L"";
    UINT cp = CP_UTF8;
    DWORD flags = MB_ERR_INVALID_CHARS;
    int n = MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), nullptr, 0);
    if (n <= 0) {
        cp = CP_ACP;
        flags = 0;
        n = MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), nullptr, 0);
    }
    if (n <= 0) return L"";
    std::wstring w(n, L'\0');
    MultiByteToWideChar(cp, flags, s.data(), (int)s.size(), &w[0], n);
    return w;
}

static bool FileExists(const std::wstring& p) {
    return GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static void ResolveCliPath() {
    g_cliPath.clear();
    g_cliDir.clear();

    WCHAR exe[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exe, MAX_PATH)) {
        PathRemoveFileSpecW(exe);
        std::wstring cand = std::wstring(exe) + L"\\windhawk-cli.exe";
        if (FileExists(cand)) {
            g_cliDir  = exe;
            g_cliPath = cand;
            return;
        }
    }

    WCHAR pf[MAX_PATH] = {};
    if (ExpandEnvironmentStringsW(L"%ProgramFiles%\\Windhawk", pf, MAX_PATH)) {
        std::wstring cand = std::wstring(pf) + L"\\windhawk-cli.exe";
        if (FileExists(cand)) {
            g_cliDir  = pf;
            g_cliPath = cand;
            return;
        }
    }

    g_cliDir  = L"C:\\Program Files\\Windhawk";
    g_cliPath = g_cliDir + L"\\windhawk-cli.exe";
}

static bool RunCliCapture(const std::wstring& args, std::string& out,
                          DWORD timeoutMs, DWORD* exitCode = nullptr) {
    out.clear();

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength        = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) return false;
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {};
    si.cb          = sizeof(si);
    si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput  = wr;
    si.hStdError   = wr;
    si.hStdInput   = GetStdHandle(STD_INPUT_HANDLE);

    std::wstring cmd = L"\"" + g_cliPath + L"\" " + args;
    std::vector<wchar_t> buf(cmd.begin(), cmd.end());
    buf.push_back(L'\0');

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessW(nullptr, buf.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, g_cliDir.c_str(),
                             &si, &pi);
    CloseHandle(wr);
    if (!ok) {
        Wh_Log(L"CreateProcess failed for CLI (%lu)", GetLastError());
        CloseHandle(rd);
        return false;
    }

    DWORD start = GetTickCount();
    bool finished = false;
    char tmp[4096];
    while (true) {
        DWORD avail = 0;
        if (PeekNamedPipe(rd, nullptr, 0, nullptr, &avail, nullptr) && avail) {
            DWORD got = 0;
            if (ReadFile(rd, tmp, sizeof(tmp), &got, nullptr) && got)
                out.append(tmp, got);
            continue;
        }
        if (WaitForSingleObject(pi.hProcess, 20) == WAIT_OBJECT_0) {
            while (PeekNamedPipe(rd, nullptr, 0, nullptr, &avail, nullptr) && avail) {
                DWORD got = 0;
                if (!ReadFile(rd, tmp, sizeof(tmp), &got, nullptr) || !got) break;
                out.append(tmp, got);
            }
            finished = true;
            break;
        }
        if (GetTickCount() - start > timeoutMs) {
            TerminateProcess(pi.hProcess, 1);
            break;
        }
    }

    DWORD code = 1;
    GetExitCodeProcess(pi.hProcess, &code);
    if (exitCode) *exitCode = code;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(rd);
    return finished;
}

static bool RunCliElevated(const std::wstring& args, DWORD* exitCode,
                           DWORD timeoutMs = 60000) {
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize       = sizeof(sei);
    sei.fMask        = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb       = g_isElevated ? L"open" : L"runas";
    sei.lpFile       = g_cliPath.c_str();
    sei.lpParameters = args.c_str();
    sei.lpDirectory  = g_cliDir.c_str();
    sei.nShow        = SW_HIDE;

    if (!ShellExecuteExW(&sei) || !sei.hProcess) {
        Wh_Log(L"ShellExecuteEx failed (%lu)", GetLastError());
        return false;
    }

    DWORD w = WaitForSingleObject(sei.hProcess, timeoutMs);
    DWORD code = 1;
    if (w == WAIT_OBJECT_0)
        GetExitCodeProcess(sei.hProcess, &code);
    CloseHandle(sei.hProcess);
    if (exitCode) *exitCode = code;
    return w == WAIT_OBJECT_0;
}

static std::vector<std::wstring> SplitNonEmpty(const std::wstring& s, wchar_t sep) {
    std::vector<std::wstring> r;
    size_t start = 0;
    while (start <= s.size()) {
        size_t p = s.find(sep, start);
        std::wstring part = Trim(s.substr(start, p == std::wstring::npos ? p : p - start));
        if (!part.empty()) r.push_back(part);
        if (p == std::wstring::npos) break;
        start = p + 1;
    }
    return r;
}

static std::vector<std::wstring> SplitWhitespace(const std::wstring& s) {
    std::vector<std::wstring> r;
    size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && iswspace(s[i])) ++i;
        size_t j = i;
        while (j < s.size() && !iswspace(s[j])) ++j;
        if (j > i) r.push_back(s.substr(i, j - i));
        i = j;
    }
    return r;
}

static bool IsStateToken(const std::wstring& t) {
    return _wcsicmp(t.c_str(), L"enabled") == 0 ||
           _wcsicmp(t.c_str(), L"disabled") == 0;
}

static std::vector<ModInfo> ParseModList(const std::wstring& text) {
    std::vector<ModInfo> result;
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t nl = text.find(L'\n', pos);
        std::wstring line = text.substr(pos, nl == std::wstring::npos ? nl : nl - pos);
        pos = (nl == std::wstring::npos) ? text.size() + 1 : nl + 1;

        line = Trim(line);
        if (line.empty()) continue;

        std::vector<std::wstring> tok = SplitNonEmpty(line, L'\t');
        if (tok.size() < 3 || !IsStateToken(tok[2]))
            tok = SplitWhitespace(line);
        if (tok.size() < 3 || !IsStateToken(tok[2]))
            continue;

        ModInfo m;
        m.id      = tok[0];
        m.version = tok[1];
        m.enabled = _wcsicmp(tok[2].c_str(), L"enabled") == 0;
        for (size_t i = 3; i < tok.size(); ++i) {
            if (i > 3) m.name += L" ";
            m.name += tok[i];
        }
        if (m.name.empty()) m.name = m.id;
        result.push_back(std::move(m));
    }
    return result;
}

static bool FetchMods(bool forceAdmin) {
    std::vector<ModInfo> mods;

    if (!forceAdmin) {
        std::string out;
        RunCliCapture(L"mod list", out, 8000);
        mods = ParseModList(Decode(out));
    }

    if (mods.empty() && (forceAdmin || !g_isElevated)) {
        bool haveCache;
        {
            std::lock_guard<std::mutex> lk(g_modsMutex);
            haveCache = !g_mods.empty();
        }

        if (forceAdmin || !haveCache) {
            WCHAR tmpDir[MAX_PATH] = {}, tmpFile[MAX_PATH] = {};
            GetTempPathW(MAX_PATH, tmpDir);
            GetTempFileNameW(tmpDir, L"whm", 0, tmpFile);

            std::wstring args = L"/c \"\"" + g_cliPath + L"\" mod list > \"" +
                                tmpFile + L"\" 2>&1\"";

            SHELLEXECUTEINFOW sei = {};
            sei.cbSize       = sizeof(sei);
            sei.fMask        = SEE_MASK_NOCLOSEPROCESS;
            sei.lpVerb       = L"runas";
            sei.lpFile       = L"cmd.exe";
            sei.lpParameters = args.c_str();
            sei.lpDirectory  = g_cliDir.c_str();
            sei.nShow        = SW_HIDE;

            if (ShellExecuteExW(&sei) && sei.hProcess) {
                WaitForSingleObject(sei.hProcess, 30000);
                CloseHandle(sei.hProcess);

                HANDLE h = CreateFileW(tmpFile, GENERIC_READ, FILE_SHARE_READ,
                                       nullptr, OPEN_EXISTING, 0, nullptr);
                if (h != INVALID_HANDLE_VALUE) {
                    std::string data;
                    char b[4096];
                    DWORD got = 0;
                    while (ReadFile(h, b, sizeof(b), &got, nullptr) && got)
                        data.append(b, got);
                    CloseHandle(h);
                    mods = ParseModList(Decode(data));
                }
            }
            DeleteFileW(tmpFile);
            g_listNeedsAdmin = true;
        }
    } else if (!mods.empty()) {
        g_listNeedsAdmin = false;
    }

    if (mods.empty())
        return false;

    std::lock_guard<std::mutex> lk(g_modsMutex);
    g_mods = std::move(mods);
    return true;
}

static void ToggleModAsync(std::wstring id, bool enable) {
    std::thread([id, enable]() {
        std::wstring args = std::wstring(L"mod ") +
                            (enable ? L"enable" : L"disable") + L" \"" + id + L"\"";
        DWORD code = 1;
        bool ran = RunCliElevated(args, &code);
        Wh_Log(L"%s %s -> ran=%d code=%lu",
               enable ? L"enable" : L"disable", id.c_str(), (int)ran, code);
        if (!ran) return;

        if (!FetchMods(false) && code == 0) {
            std::lock_guard<std::mutex> lk(g_modsMutex);
            for (auto& m : g_mods)
                if (m.id == id) m.enabled = enable;
        }
    }).detach();
}


static HBITMAP CreateDib32(int size, void** bits) {
    HDC hdc = GetDC(nullptr);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = size;
    bmi.bmiHeader.biHeight      = -size;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    HBITMAP dib = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, bits, nullptr, 0);
    ReleaseDC(nullptr, hdc);
    return dib;
}

static HBITMAP RenderGlyphDib(WCHAR glyph, int size, bool white) {
    void* bits = nullptr;
    HBITMAP dib = CreateDib32(size, &bits);
    if (!dib) return nullptr;
    memset(bits, 0, (size_t)size * size * 4);

    HDC hdc   = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, dib);

    HFONT font = CreateFontW(-size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
    HFONT oldFont = (HFONT)SelectObject(memDC, font);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(255, 255, 255));
    RECT rc = { 0, 0, size, size };
    DrawTextW(memDC, &glyph, 1, &rc,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(memDC, oldFont);
    DeleteObject(font);
    SelectObject(memDC, oldBmp);
    DeleteDC(memDC);
    ReleaseDC(nullptr, hdc);

    DWORD* px = (DWORD*)bits;
    for (int i = 0; i < size * size; i++) {
        BYTE b = (BYTE)(px[i] & 0xFF);
        if (!b) { px[i] = 0; continue; }
        if (white) {
            px[i] = (b << 24) | (b << 16) | (b << 8) | b;
        } else {
            BYTE v = (BYTE)((40 * b) / 255);
            px[i] = (b << 24) | (v << 16) | (v << 8) | v;
        }
    }
    return dib;
}

static HICON CreateGlyphIcon(WCHAR glyph, int size, bool white) {
    HBITMAP dib = RenderGlyphDib(glyph, size, white);
    if (!dib) return nullptr;
    HBITMAP mask = CreateBitmap(size, size, 1, 1, nullptr);
    ICONINFO ii = { TRUE, 0, 0, mask, dib };
    HICON h = CreateIconIndirect(&ii);
    DeleteObject(mask);
    DeleteObject(dib);
    return h;
}

static HBITMAP MakeBlankBitmap(int size) {
    void* bits = nullptr;
    HBITMAP dib = CreateDib32(size, &bits);
    if (dib) memset(bits, 0, (size_t)size * size * 4);
    return dib;
}

static void DestroyIcons() {
    if (g_iconWhite) { DestroyIcon(g_iconWhite); g_iconWhite = nullptr; }
    if (g_iconBlack) { DestroyIcon(g_iconBlack); g_iconBlack = nullptr; }
}

static HICON CurrentTrayIcon() {
    return TrayIconUseWhite() ? g_iconWhite : g_iconBlack;
}

static void AddTrayIcon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize           = sizeof(nid);
    nid.hWnd             = g_trayHwnd;
    nid.uID              = TRAY_UID;
    nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY_CB;
    nid.hIcon            = CurrentTrayIcon();
    lstrcpynW(nid.szTip, L"Windhawk mods", ARRAYSIZE(nid.szTip));
    if (!Shell_NotifyIconW(NIM_MODIFY, &nid))
        Shell_NotifyIconW(NIM_ADD, &nid);
}

static void RefreshTrayIconTheme() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd   = g_trayHwnd;
    nid.uID    = TRAY_UID;
    nid.uFlags = NIF_ICON;
    nid.hIcon  = CurrentTrayIcon();
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

static void RemoveTrayIcon() {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd   = g_trayHwnd;
    nid.uID    = TRAY_UID;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

static void LoadSettings() {
    g_iconColorMode = L"auto";
    if (PCWSTR p = Wh_GetStringSetting(L"icon_color")) {
        if (wcscmp(p, L"white") == 0) g_iconColorMode = L"white";
        else if (wcscmp(p, L"black") == 0) g_iconColorMode = L"black";
        Wh_FreeStringSetting(p);
    }
    g_menuColorMode = L"auto";
    if (PCWSTR p = Wh_GetStringSetting(L"menu_color")) {
        if (wcscmp(p, L"dark") == 0) g_menuColorMode = L"dark";
        else if (wcscmp(p, L"light") == 0) g_menuColorMode = L"light";
        Wh_FreeStringSetting(p);
    }
    g_showIds = Wh_GetIntSetting(L"show_ids") != 0;

    g_iconGlyph = L'\uE71D';
    if (PCWSTR p = Wh_GetStringSetting(L"icon")) {
        std::wstring v = Trim(p);
        Wh_FreeStringSetting(p);
        if (v.size() > 2 && v[0] == L'\\' && (v[1] == L'u' || v[1] == L'U'))
            v = v.substr(2);
        WCHAR g = (WCHAR)wcstoul(v.c_str(), nullptr, 16);
        if (g) g_iconGlyph = g;
    }

    auto loadList = [](const wchar_t* name, std::vector<std::wstring>& out) {
        out.clear();
        for (int i = 0; i < 512; ++i) {
            WCHAR key[64];
            swprintf_s(key, L"%s[%d]", name, i);
            PCWSTR p = Wh_GetStringSetting(key);
            if (!p) break;
            std::wstring v = Trim(p);
            Wh_FreeStringSetting(p);
            if (!v.empty()) out.push_back(v);
        }
    };
    loadList(L"whitelist", g_whitelist);
    loadList(L"blacklist", g_blacklist);
}

static bool ListContainsId(const std::vector<std::wstring>& list,
                           const std::wstring& id) {
    for (auto& s : list)
        if (_wcsicmp(s.c_str(), id.c_str()) == 0) return true;
    return false;
}

static void CreateTrayIcons() {
    DestroyIcons();
    g_iconWhite = CreateGlyphIcon(g_iconGlyph, 32, true);
    g_iconBlack = CreateGlyphIcon(g_iconGlyph, 32, false);
}

static void ApplyMenuTheme(HWND hWnd, bool dark) {
    HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
    if (!ux) return;

    using fnSetPreferredAppMode = int(WINAPI*)(int);
    auto SetPreferredAppMode =
        (fnSetPreferredAppMode)GetProcAddress(ux, MAKEINTRESOURCEA(135));
    if (SetPreferredAppMode) SetPreferredAppMode(dark ? 2 : 0);

    using fnAllowDarkModeForWindow = bool(WINAPI*)(HWND, bool);
    auto AllowDarkModeForWindow =
        (fnAllowDarkModeForWindow)GetProcAddress(ux, MAKEINTRESOURCEA(133));
    if (AllowDarkModeForWindow) AllowDarkModeForWindow(hWnd, dark);

    using fnFlushMenuThemes = void(WINAPI*)();
    auto FlushMenuThemes =
        (fnFlushMenuThemes)GetProcAddress(ux, MAKEINTRESOURCEA(136));
    if (FlushMenuThemes) FlushMenuThemes();
}

static std::wstring EscapeAmp(const std::wstring& s) {
    std::wstring r;
    for (wchar_t c : s) {
        r += c;
        if (c == L'&') r += L'&';
    }
    return r;
}

// ---- Mouse wheel scrolling ----
// A menu taller than the screen shows scroll arrows that only move it while
// hovered. While the menu is open, a low-level mouse hook catches the wheel over
// it (wherever Windows would send it: the menu's thread often isn't in front,
// as the tray click came through the taskbar) and turns each notch into Up/Down
// key presses, which the menu follows by scrolling. A message filter hook does
// the same for wheel messages that reach the menu itself, should the low-level
// hook not be allowed.

static HMENU     g_wheelMenu  = nullptr;
static HHOOK     g_wheelHook  = nullptr;   // WH_MSGFILTER
static HHOOK     g_wheelLLHook = nullptr;  // WH_MOUSE_LL
static DWORD     g_wheelThread = 0;
static int       g_wheelDelta = 0;   // the part of a notch not used yet (touchpads)
static const int kWheelItems  = 3;   // mods per notch

static BOOL CALLBACK FindMenuWindow(HWND hwnd, LPARAM lp) {
    WCHAR cls[16];
    if (IsWindowVisible(hwnd) && GetClassNameW(hwnd, cls, 16) &&
        wcscmp(cls, L"#32768") == 0) {
        *(HWND*)lp = hwnd;
        return FALSE;
    }
    return TRUE;
}

// The first or last item showing (between the arrows), by position; -1 if none.
static int MenuItemAtEdge(HWND menuWnd, HMENU menu, bool top) {
    RECT rc;
    GetWindowRect(menuWnd, &rc);
    int x = (rc.left + rc.right) / 2;
    for (int d = 1; d < 80; d += 2) {
        POINT pt = {x, top ? rc.top + d : rc.bottom - 1 - d};
        int i = MenuItemFromPoint(nullptr, menu, pt);
        if (i >= 0) return i;
    }
    return -1;
}

static bool MenuItemSelectable(HMENU menu, int i) {
    UINT st = GetMenuState(menu, i, MF_BYPOSITION);
    return st != (UINT)-1 && !(st & MF_SEPARATOR);
}

// notches > 0: up (wheel away from you), < 0: down.
static void WheelScrollMenu(HWND menuWnd, HMENU menu, int notches) {
    int count = GetMenuItemCount(menu);
    int first = MenuItemAtEdge(menuWnd, menu, true);
    int last  = MenuItemAtEdge(menuWnd, menu, false);
    Wh_Log(L"wheel %d: %d items, showing %d..%d", notches, count, first, last);
    if (count <= 0 || first < 0 || last < first) return;
    if (first == 0 && last == count - 1) return;  // all of it shows
    int shown = last - first + 1;
    int top = first - notches * kWheelItems;      // the first item to show
    if (top > count - shown) top = count - shown;
    if (top < 0) top = 0;
    if (top == first) return;

    int hilite = -1;
    for (int i = 0; i < count; i++)
        if (GetMenuState(menu, i, MF_BYPOSITION) & MF_HILITE) { hilite = i; break; }

    // The menu scrolls to keep the selected item showing: select the new
    // bottom item going down, or the new top item going up.
    UINT key;
    int steps = 0;
    if (hilite < 0 || top > first) {
        key = VK_DOWN;
        int to = top + shown - 1;
        // (with nothing selected, the first Down selects the first item)
        for (int i = hilite < 0 ? 0 : hilite + 1; i <= to; i++)
            if (MenuItemSelectable(menu, i)) steps++;
    } else {
        key = VK_UP;
        for (int i = hilite - 1; i >= top; i--)
            if (MenuItemSelectable(menu, i)) steps++;
    }
    Wh_Log(L"wheel: selected %d, %d x %s", hilite, steps, key == VK_DOWN ? L"Down" : L"Up");
    for (int i = 0; i < steps; i++)
        PostMessageW(menuWnd, WM_KEYDOWN, key, 0);
}

static HWND OpenMenuWindow() {
    HWND menuWnd = nullptr;
    EnumThreadWindows(g_wheelThread, FindMenuWindow, (LPARAM)&menuWnd);
    return menuWnd;
}

static void WheelBy(HWND menuWnd, int delta) {
    g_wheelDelta += delta;
    int notches = g_wheelDelta / WHEEL_DELTA;
    g_wheelDelta -= notches * WHEEL_DELTA;
    if (notches) WheelScrollMenu(menuWnd, g_wheelMenu, notches);
}

static LRESULT CALLBACK MenuWheelLLHook(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION && wp == WM_MOUSEWHEEL && g_wheelMenu) {
        auto* ms = (MSLLHOOKSTRUCT*)lp;
        HWND menuWnd = OpenMenuWindow();
        RECT rc;
        if (menuWnd && GetWindowRect(menuWnd, &rc) && PtInRect(&rc, ms->pt)) {
            WheelBy(menuWnd, (short)HIWORD(ms->mouseData));
            return 1;  // (taken: nothing else scrolls)
        }
    }
    return CallNextHookEx(g_wheelLLHook, code, wp, lp);
}

static LRESULT CALLBACK MenuWheelHook(int code, WPARAM wp, LPARAM lp) {
    if (code == MSGF_MENU && g_wheelMenu) {
        MSG* m = (MSG*)lp;
        if (m->message == WM_MOUSEWHEEL) {
            HWND menuWnd = OpenMenuWindow();
            if (menuWnd) {
                WheelBy(menuWnd, GET_WHEEL_DELTA_WPARAM(m->wParam));
                return TRUE;
            }
        }
    }
    return CallNextHookEx(g_wheelHook, code, wp, lp);
}
// ---- end of mouse wheel scrolling ----

static void ShowModsMenu(HWND hWnd) {
    if (GetTickCount() - g_lastMenuClose < 300) return;

    if (g_cliPath.empty()) ResolveCliPath();
    FetchMods(false);

    std::vector<ModInfo> mods;
    {
        std::lock_guard<std::mutex> lk(g_modsMutex);
        mods = g_mods;
    }

    {
        std::vector<ModInfo> filtered;
        for (auto& m : mods) {
            if (!g_whitelist.empty() && !ListContainsId(g_whitelist, m.id))
                continue;
            if (ListContainsId(g_blacklist, m.id))
                continue;
            filtered.push_back(m);
        }
        mods = std::move(filtered);
    }

    const int sz = 16;
    bool dark = MenuUseDark();

    SetForegroundWindow(hWnd);
    ApplyMenuTheme(hWnd, dark);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    std::vector<HBITMAP> bitmaps;
    HBITMAP checkBmp = RenderGlyphDib(L'\uE73E', sz, dark);
    if (checkBmp) bitmaps.push_back(checkBmp);

    UINT pos = 0;
    if (mods.empty()) {
        MENUITEMINFOW mii = {};
        mii.cbSize     = sizeof(mii);
        mii.fMask      = MIIM_STRING | MIIM_STATE | MIIM_ID;
        mii.wID        = 998;
        mii.fState     = MFS_DISABLED;
        WCHAR txt[] = L"No mods to show";
        mii.dwTypeData = txt;
        InsertMenuItemW(hMenu, pos++, TRUE, &mii);
    }

    for (size_t i = 0; i < mods.size(); ++i) {
        const ModInfo& m = mods[i];
        std::wstring txt = EscapeAmp(m.name);
        if (g_showIds) txt += L"  (" + EscapeAmp(m.id) + L")";

        MENUITEMINFOW mii = {};
        mii.cbSize     = sizeof(mii);
        mii.fMask      = MIIM_ID | MIIM_STRING | MIIM_STATE;
        mii.wID        = ID_BASE + (UINT)i;
        mii.fState     = MFS_ENABLED;
        mii.dwTypeData = &txt[0];
        mii.cch        = (UINT)txt.size();

        HBITMAP bmp = m.enabled ? checkBmp : nullptr;
        if (!m.enabled) {
            bmp = MakeBlankBitmap(sz);
            if (bmp) bitmaps.push_back(bmp);
        }
        if (bmp) {
            mii.fMask   |= MIIM_BITMAP;
            mii.hbmpItem = bmp;
        }
        InsertMenuItemW(hMenu, pos++, TRUE, &mii);
    }

    if (g_listNeedsAdmin) {
        MENUITEMINFOW sep = {};
        sep.cbSize = sizeof(sep);
        sep.fMask  = MIIM_FTYPE;
        sep.fType  = MFT_SEPARATOR;
        InsertMenuItemW(hMenu, pos++, TRUE, &sep);

        MENUITEMINFOW mii = {};
        mii.cbSize     = sizeof(mii);
        mii.fMask      = MIIM_ID | MIIM_STRING | MIIM_STATE;
        mii.wID        = ID_REFRESH;
        mii.fState     = MFS_ENABLED;
        WCHAR txt[] = L"Refresh list (admin)";
        mii.dwTypeData = txt;
        InsertMenuItemW(hMenu, pos++, TRUE, &mii);
    }

    POINT pt;
    GetCursorPos(&pt);
    g_wheelMenu   = hMenu;
    g_wheelDelta  = 0;
    g_wheelThread = GetCurrentThreadId();
    HMODULE self  = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&MenuWheelLLHook, &self);
    g_wheelLLHook = SetWindowsHookExW(WH_MOUSE_LL, MenuWheelLLHook, self, 0);
    if (!g_wheelLLHook) Wh_Log(L"wheel: no low-level mouse hook (%lu)", GetLastError());
    g_wheelHook   = SetWindowsHookExW(WH_MSGFILTER, MenuWheelHook, nullptr,
                                      GetCurrentThreadId());
    int cmd = TrackPopupMenu(hMenu,
                             TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY |
                                 TPM_BOTTOMALIGN | TPM_RIGHTALIGN,
                             pt.x, pt.y, 0, hWnd, nullptr);
    if (g_wheelLLHook) UnhookWindowsHookEx(g_wheelLLHook);
    g_wheelLLHook = nullptr;
    if (g_wheelHook) UnhookWindowsHookEx(g_wheelHook);
    g_wheelHook = nullptr;
    g_wheelMenu = nullptr;
    PostMessageW(hWnd, WM_NULL, 0, 0);
    g_lastMenuClose = GetTickCount();

    DestroyMenu(hMenu);
    for (auto b : bitmaps) DeleteObject(b);

    if (cmd == (int)ID_REFRESH) {
        FetchMods(true);
    } else if (cmd >= (int)ID_BASE) {
        size_t idx = (size_t)(cmd - ID_BASE);
        if (idx < mods.size())
            ToggleModAsync(mods[idx].id, !mods[idx].enabled);
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAY_CB) {
        UINT event = (UINT)lParam;
        if (event == WM_RBUTTONUP) {
            if (g_iconColorMode == L"auto") RefreshTrayIconTheme();
            ShowModsMenu(hWnd);
        }
        return 0;
    }

    if (msg == WM_RELOAD) {
        LoadSettings();
        CreateTrayIcons();
        RefreshTrayIconTheme();
        return 0;
    }

    if (msg == WM_SETTINGCHANGE) {
        if (lParam && lstrcmpW((LPCWSTR)lParam, L"ImmersiveColorSet") == 0 &&
            g_iconColorMode == L"auto")
            RefreshTrayIconTheme();
        return 0;
    }

    if (g_taskbarCreated != 0 && msg == g_taskbarCreated) {
        AddTrayIcon();
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static DWORD WINAPI TrayThread(LPVOID) {
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

    WNDCLASSW wc     = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = g_hInstance;
    wc.lpszClassName = L"WHModManagerTray_Window";
    RegisterClassW(&wc);

    g_trayHwnd = CreateWindowExW(0, wc.lpszClassName, L"WHModManagerTray_Host",
                                 0, 0, 0, 0, 0, nullptr, nullptr, g_hInstance,
                                 nullptr);
    if (!g_trayHwnd) return 1;

    if (g_taskbarCreated != 0)
        ChangeWindowMessageFilterEx(g_trayHwnd, g_taskbarCreated,
                                    MSGFLT_ALLOW, nullptr);

    g_isElevated = IsProcessElevated();
    ResolveCliPath();
    LoadSettings();

    CreateTrayIcons();
    AddTrayIcon();

    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0))
        DispatchMessageW(&m);

    RemoveTrayIcon();
    DestroyIcons();
    return 0;
}

bool   g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

void WINAPI EntryPoint_Hook() {
    Wh_Log(L">");
    ExitThread(0);
}

BOOL WhTool_ModInit() {
    g_hInstance = (HINSTANCE)GetModuleHandleW(nullptr);
    HANDLE h = CreateThread(nullptr, 0, TrayThread, nullptr, 0, nullptr);
    if (!h) return FALSE;
    CloseHandle(h);
    return TRUE;
}

void WhTool_ModSettingsChanged() {
    if (g_trayHwnd)
        PostMessageW(g_trayHwnd, WM_RELOAD, 0, 0);
}

void WhTool_ModUninit() {
    if (g_trayHwnd) {
        RemoveTrayIcon();
        PostMessageW(g_trayHwnd, WM_QUIT, 0, 0);
        g_trayHwnd = nullptr;
    }
}

BOOL Wh_ModInit() {
    DWORD sessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) &&
        sessionId == 0) {
        return FALSE;
    }

    bool isExcluded              = false;
    bool isToolModProcess        = false;
    bool isCurrentToolModProcess = false;
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        Wh_Log(L"CommandLineToArgvW failed");
        return FALSE;
    }

    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 ||
            wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            isExcluded = true;
            break;
        }
    }

    for (int i = 1; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0) {
            isToolModProcess = true;
            if (wcscmp(argv[i + 1], WH_MOD_ID) == 0)
                isCurrentToolModProcess = true;
            break;
        }
    }

    LocalFree(argv);

    if (isExcluded)
        return FALSE;

    if (isCurrentToolModProcess) {
        g_toolModProcessMutex =
            CreateMutexW(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!g_toolModProcessMutex) {
            Wh_Log(L"CreateMutex failed");
            ExitProcess(1);
        }
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
            ExitProcess(1);
        }
        if (!WhTool_ModInit())
            ExitProcess(1);

        IMAGE_DOS_HEADER* dosHeader =
            (IMAGE_DOS_HEADER*)GetModuleHandleW(nullptr);
        IMAGE_NT_HEADERS* ntHeaders =
            (IMAGE_NT_HEADERS*)((BYTE*)dosHeader + dosHeader->e_lfanew);
        void* entryPoint =
            (BYTE*)dosHeader + ntHeaders->OptionalHeader.AddressOfEntryPoint;
        Wh_SetFunctionHook(entryPoint, (void*)EntryPoint_Hook, nullptr);
        return TRUE;
    }

    if (isToolModProcess)
        return FALSE;

    g_isToolModProcessLauncher = true;
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_isToolModProcessLauncher)
        return;

    WCHAR currentProcessPath[MAX_PATH];
    switch (GetModuleFileNameW(nullptr, currentProcessPath,
                               ARRAYSIZE(currentProcessPath))) {
        case 0:
        case ARRAYSIZE(currentProcessPath):
            Wh_Log(L"GetModuleFileName failed");
            return;
    }

    WCHAR commandLine[MAX_PATH + 2 +
                      (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"",
               currentProcessPath, WH_MOD_ID);

    HMODULE kernelModule = GetModuleHandleW(L"kernelbase.dll");
    if (!kernelModule) {
        kernelModule = GetModuleHandleW(L"kernel32.dll");
        if (!kernelModule) {
            Wh_Log(L"No kernelbase.dll/kernel32.dll");
            return;
        }
    }

    using CreateProcessInternalW_t = BOOL(WINAPI*)(
        HANDLE, LPCWSTR, LPWSTR,
        LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, WINBOOL,
        DWORD, LPVOID, LPCWSTR,
        LPSTARTUPINFOW, LPPROCESS_INFORMATION, PHANDLE);
    auto pCreateProcessInternalW =
        (CreateProcessInternalW_t)GetProcAddress(kernelModule,
                                                 "CreateProcessInternalW");
    if (!pCreateProcessInternalW) {
        Wh_Log(L"No CreateProcessInternalW");
        return;
    }

    STARTUPINFOW si{};
    si.cb      = sizeof(STARTUPINFOW);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    PROCESS_INFORMATION pi{};
    if (!pCreateProcessInternalW(nullptr, currentProcessPath, commandLine,
                                 nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS,
                                 nullptr, nullptr, &si, &pi, nullptr)) {
        Wh_Log(L"CreateProcess failed");
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Wh_ModSettingsChanged() {
    if (g_isToolModProcessLauncher)
        return;
    WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
    if (g_isToolModProcessLauncher)
        return;
    WhTool_ModUninit();
    ExitProcess(0);
}