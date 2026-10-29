// The launcher: see launcher.h.
//
// One thread of its own holds the hotkey and the window: a search line (an edit control) over a list drawn here,
// in the theme's colours. Apps come from the shell's Apps folder (what Start lists), read on a worker thread with
// their icons; providers answer through the plugin runtime (plugin-runtime.h, Launcher*), which posts kMsgResults
// when an answer comes, and the list is put together again.

#include "launcher.h"

#include <dwmapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <windowsx.h>

#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "launcher-math.h"
#include "plugin-runtime.h"
#include "ui-sounds.h"
#include "studio.h"
#include "td.h"

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"launcher", format, args);
    va_end(args);
}

constexpr UINT kMsgResults = WM_APP + 1, kMsgApps = WM_APP + 2, kMsgApply = WM_APP + 3, kMsgToggle = WM_APP + 4;
constexpr int kHotkey = 1;
constexpr int kMaxRows = 8;

struct Config {
    bool on = true;
    std::wstring hotkey = L"alt+space";
    std::wstring web = L"https://duckduckgo.com/?q=%s";  // "? " searches here (%s: the words); its bangs work as typed
};

Config ReadConfig() {
    Config c;
    c.on = GetInt(L"launcher", L"enabled") != 0;
    if (const wchar_t* v = GetString(L"launcher", L"hotkey")) {
        if (*v) c.hotkey = v;
        FreeString(v);
    }
    if (const wchar_t* v = GetString(L"launcher", L"webSearch")) {
        if (wcsstr(v, L"%s")) c.web = v;
        FreeString(v);
    }
    return c;
}

// "alt+space", "ctrl+shift+k", "win+f1"...
bool ParseKeys(std::wstring text, UINT* mods, UINT* vk) {
    for (auto& ch : text) ch = (wchar_t)towlower(ch);
    *mods = MOD_NOREPEAT;
    *vk = 0;
    size_t at = 0;
    while (at <= text.size()) {
        size_t plus = text.find(L'+', at);
        std::wstring part = text.substr(at, plus == std::wstring::npos ? std::wstring::npos : plus - at);
        part.erase(0, part.find_first_not_of(L" "));
        part.erase(part.find_last_not_of(L" ") + 1);
        if (part == L"alt") *mods |= MOD_ALT;
        else if (part == L"ctrl" || part == L"control") *mods |= MOD_CONTROL;
        else if (part == L"shift") *mods |= MOD_SHIFT;
        else if (part == L"win" || part == L"super" || part == L"mod") *mods |= MOD_WIN;
        else if (part == L"space") *vk = VK_SPACE;
        else if (part == L"enter" || part == L"return") *vk = VK_RETURN;
        else if (part == L"tab") *vk = VK_TAB;
        else if (part.size() == 1 && iswalnum(part[0])) *vk = (UINT)towupper(part[0]);
        else if (part.size() >= 2 && part[0] == L'f' && _wtoi(part.c_str() + 1) >= 1 && _wtoi(part.c_str() + 1) <= 24)
            *vk = VK_F1 + _wtoi(part.c_str() + 1) - 1;
        else if (!part.empty()) return false;
        if (plus == std::wstring::npos) break;
        at = plus + 1;
    }
    return *vk != 0;
}

////////////////////////////////////////////////////////////////////////////////
// Apps: what the shell's Apps folder lists (Start's own list), with a 32 px picture each.

struct App {
    std::wstring name, target;  // target: the Apps folder's parsing name (a path, or a Store app's id)
    HBITMAP icon = nullptr;     // 32 bpp, premultiplied
    int iconSize = 0;
};

std::mutex g_appsLock;
std::shared_ptr<std::vector<App>> g_apps;  // swapped whole when read again
std::atomic<bool> g_reading{false};
ULONGLONG g_readAt = 0;

std::shared_ptr<std::vector<App>> ReadApps(int iconPx) {
    // (the pictures go with the list, once nothing shows it)
    std::shared_ptr<std::vector<App>> out(new std::vector<App>, [](std::vector<App>* list) {
        for (auto& a : *list)
            if (a.icon) DeleteObject(a.icon);
        delete list;
    });
    IShellItem* folder = nullptr;
    if (FAILED(SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(&folder)))) return out;
    IEnumShellItems* items = nullptr;
    if (SUCCEEDED(folder->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&items)))) {
        IShellItem* item = nullptr;
        while (items->Next(1, &item, nullptr) == S_OK) {
            PWSTR name = nullptr, parse = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)) && SUCCEEDED(item->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &parse))) {
                App a{name, parse};
                IShellItemImageFactory* images = nullptr;
                if (SUCCEEDED(item->QueryInterface(IID_PPV_ARGS(&images)))) {
                    HBITMAP bmp = nullptr;
                    if (SUCCEEDED(images->GetImage({iconPx, iconPx}, SIIGBF_ICONONLY | SIIGBF_BIGGERSIZEOK, &bmp))) a.icon = bmp, a.iconSize = iconPx;
                    images->Release();
                }
                out->push_back(std::move(a));
            }
            CoTaskMemFree(name);
            CoTaskMemFree(parse);
            item->Release();
        }
        items->Release();
    }
    folder->Release();
    std::sort(out->begin(), out->end(), [](App const& a, App const& b) { return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0; });
    return out;
}

// Read again on a worker (at most once a minute); kMsgApps to the window when done.
void RefreshApps(HWND wnd, int iconPx) {
    if (g_reading.exchange(true)) return;
    if (g_apps && GetTickCount64() - g_readAt < 60000) {
        g_reading = false;
        return;
    }
    std::thread([wnd, iconPx] {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        auto apps = ReadApps(iconPx);
        CoUninitialize();
        {
            std::lock_guard lock(g_appsLock);
            g_apps = apps;  // (the old list's pictures go with its last user)
        }
        g_readAt = GetTickCount64();
        g_reading = false;
        PostMessageW(wnd, kMsgApps, 0, 0);
    }).detach();
}

// A pattern's letters in order in the text; more for runs, a word's start and the text's start. -1: not there.
double Fuzzy(std::wstring const& pattern, std::wstring const& text) {
    if (pattern.empty()) return 0;
    size_t at = 0;
    double score = 0, run = 0;
    for (wchar_t c : pattern) {
        if (c == L' ') continue;
        size_t found = std::wstring::npos;
        for (size_t i = at; i < text.size(); i++)
            if (towlower(text[i]) == towlower(c)) {
                found = i;
                break;
            }
        if (found == std::wstring::npos) return -1;
        run = found == at ? run + 1 : 0;
        bool wordStart = found == 0 || text[found - 1] == L' ' || text[found - 1] == L'-' || text[found - 1] == L'_';
        score += 1 + run * 2 + (wordStart ? 3 : 0) - (double)(found - at) * 0.1;
        at = found + 1;
    }
    if (_wcsnicmp(text.c_str(), pattern.c_str(), pattern.size()) == 0) score += 10;
    return score - text.size() * 0.02;  // (shorter names first, all else equal)
}

////////////////////////////////////////////////////////////////////////////////
// The window.

struct Row {
    std::wstring title, subtitle, glyph, provider, id;  // provider empty: an app (id: its target); "@prefix": fill it in
    HBITMAP icon = nullptr;
    int iconSize = 0;
    double score = 0;
};

Config g_cfg;
HWND g_host = nullptr, g_wnd = nullptr, g_edit = nullptr;
DWORD g_threadId = 0;
std::thread g_thread;
std::vector<Row> g_rows;
int g_sel = 0, g_hot = -1;
std::wstring g_query;
std::shared_ptr<std::vector<App>> g_shownApps;  // the list the rows' pictures come from (kept while shown)
HFONT g_font = nullptr, g_small = nullptr, g_glyphs = nullptr;
int g_dpi = 96;
WNDPROC g_editProc = nullptr;
HBRUSH g_editBrush = nullptr;
bool g_registered = false;

int S(int v) { return MulDiv(v, g_dpi, 96); }

struct Look {
    COLORREF ground, raised, line, text, muted, accent;
};

Look Themed() {
    auto c = [](uint32_t v) { return RGB(v >> 16 & 255, v >> 8 & 255, v & 255); };
    uint32_t p[11];
    if (FlyoutPalette(p)) return {c(p[0]), c(p[10]), c(p[2]), c(p[4]), c(p[6]), c(p[5])};
    Look look{RGB(32, 32, 32), RGB(52, 52, 52), RGB(64, 64, 64), RGB(235, 235, 235), RGB(160, 160, 160), RGB(79, 195, 247)};
    uint32_t v;
    for (int i = 0; i < kThemeRoleCount; i++) {
        if (!ThemeRoleColor(kThemeRoles[i], &v)) continue;
        std::wstring t = kThemeRoles[i].token;
        if (t == L"@ground") look.ground = c(v);
        else if (t == L"@raised") look.raised = c(v);
        else if (t == L"@line") look.line = c(v);
        else if (t == L"@text") look.text = c(v);
        else if (t == L"@muted") look.muted = c(v);
        else if (t == L"@accent") look.accent = c(v);
    }
    return look;
}

int InputHeight() { return S(52); }
int RowHeight() { return S(44); }

void Layout() {
    RECT rc;
    GetClientRect(g_wnd, &rc);
    int rows = std::min<int>((int)g_rows.size(), kMaxRows);
    int h = InputHeight() + (rows ? rows * RowHeight() + S(8) : 0);
    RECT wr;
    GetWindowRect(g_wnd, &wr);
    SetWindowPos(g_wnd, nullptr, 0, 0, wr.right - wr.left, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    MoveWindow(g_edit, S(48), (InputHeight() - S(24)) / 2, rc.right - S(64), S(26), TRUE);
    InvalidateRect(g_wnd, nullptr, FALSE);
}

////////////////////////////////////////////////////////////////////////////////
// Tourne's own answers (no plugin): a sum or a unit conversion on any line, and three prefixes, each with or
// without a space after it: "? " the web, "@ " the open windows, "$ " snippets. Their rows' provider starts
// with '#'; Activate does what each says.

std::wstring Trimmed(const std::wstring& s) {
    size_t a = s.find_first_not_of(L" \t"), b = s.find_last_not_of(L" \t");
    return a == std::wstring::npos ? L"" : s.substr(a, b - a + 1);
}

std::wstring SnippetsPath() {
    PWSTR roaming = nullptr;
    std::wstring path;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming))) path = std::wstring(roaming) + L"\\Tourne\\Desktop\\snippets.txt";
    CoTaskMemFree(roaming);
    return path;
}

// snippets.txt: "name = text" a line (\n in the text for a new line, # for a note).
std::vector<std::pair<std::wstring, std::wstring>> ReadSnippets() {
    std::vector<std::pair<std::wstring, std::wstring>> out;
    HANDLE f = CreateFileW(SnippetsPath().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return out;
    std::string bytes;
    char buf[4096];
    DWORD got;
    while (ReadFile(f, buf, sizeof(buf), &got, nullptr) && got) bytes.append(buf, got);
    CloseHandle(f);
    if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF) bytes.erase(0, 3);  // (a BOM)
    int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
    std::wstring all(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), all.data(), n);
    size_t at = 0;
    while (at < all.size()) {
        size_t end = all.find(L'\n', at);
        std::wstring line = all.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        at = end == std::wstring::npos ? all.size() : end + 1;
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        line = Trimmed(line);
        size_t eq = line.find(L'=');
        if (line.empty() || line[0] == L'#' || eq == std::wstring::npos) continue;
        std::wstring name = Trimmed(line.substr(0, eq)), text = Trimmed(line.substr(eq + 1));
        for (size_t i; (i = text.find(L"\\n")) != std::wstring::npos;) text.replace(i, 2, L"\r\n");
        if (!name.empty()) out.push_back({name, text});
    }
    return out;
}

// The windows that can be switched to: shown, with a title, not tool windows or hidden (cloaked) ones.
std::vector<std::pair<HWND, std::wstring>> OpenWindows() {
    std::vector<std::pair<HWND, std::wstring>> out;
    EnumWindows(
        [](HWND w, LPARAM l) -> BOOL {
            auto* list = (std::vector<std::pair<HWND, std::wstring>>*)l;
            if (!IsWindowVisible(w) || w == g_wnd || GetWindow(w, GW_OWNER) || (GetWindowLongW(w, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)) return TRUE;
            DWORD cloaked = 0;
            if (SUCCEEDED(DwmGetWindowAttribute(w, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) return TRUE;
            wchar_t title[256];
            if (GetWindowTextW(w, title, 256) <= 0) return TRUE;
            list->push_back({w, title});
            return TRUE;
        },
        (LPARAM)&out);
    return out;
}

std::wstring ProgramOf(HWND w) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    std::wstring name;
    if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        wchar_t path[MAX_PATH];
        DWORD len = MAX_PATH;
        if (QueryFullProcessImageNameW(p, 0, path, &len)) name = wcsrchr(path, L'\\') ? wcsrchr(path, L'\\') + 1 : path;
        CloseHandle(p);
    }
    return name;
}

// A prefix's line ("?", "@", "$"): its rows (true), or false when the line isn't one.
bool PrefixRows(const std::wstring& text, std::vector<Row>* rows) {
    if (text.empty() || (text[0] != L'?' && text[0] != L'@' && text[0] != L'$')) return false;
    std::wstring rest = Trimmed(text.substr(1));
    if (text[0] == L'?') {
        if (rest.empty()) rows->push_back({L"Search the web", L"Type what to look for (!g, !w and other bangs work)", L"world-search", L"#hint", L"", nullptr, 0, 0});
        else rows->push_back({L"Search the web for \"" + rest + L"\"", L"Opens in your browser", L"world-search", L"#web", rest, nullptr, 0, 1});
        return true;
    }
    if (text[0] == L'@') {
        for (auto const& [w, title] : OpenWindows()) {
            std::wstring program = ProgramOf(w);
            double score = rest.empty() ? 0 : std::max(Fuzzy(rest, title), Fuzzy(rest, program));
            if (score < 0) continue;
            rows->push_back({title, program, L"app-window", L"#window", std::to_wstring((ULONG_PTR)w), nullptr, 0, score});
        }
        std::stable_sort(rows->begin(), rows->end(), [](Row const& a, Row const& b) { return a.score > b.score; });
        if (rows->empty()) rows->push_back({L"No open window by that name", L"", L"app-window", L"#hint", L"", nullptr, 0, 0});
        return true;
    }
    auto snippets = ReadSnippets();
    for (auto const& [name, body] : snippets) {
        double score = rest.empty() ? 0 : std::max(Fuzzy(rest, name), Fuzzy(rest, body) - 5);
        if (score < 0) continue;
        std::wstring one = body;
        for (auto& c : one)
            if (c == L'\r' || c == L'\n') c = L' ';
        rows->push_back({name, one, L"clipboard-text", L"#snippet", body, nullptr, 0, score});
    }
    std::stable_sort(rows->begin(), rows->end(), [](Row const& a, Row const& b) { return a.score > b.score; });
    rows->push_back({snippets.empty() ? L"No snippets yet: make some" : L"Edit snippets", L"snippets.txt, a line each: name = text",
                     L"note", L"#snippets", L"", nullptr, 0, -1});
    return true;
}

// A sum or a conversion: its row, to go first (true), or false.
bool AnswerRow(const std::wstring& text, Row* row) {
    std::wstring answer;
    double v;
    if (launchermath::Convert(text, &answer)) {
        *row = {answer, L"Enter copies it", L"ruler-2", L"#copy", answer.substr(0, answer.find(L' ')), nullptr, 0, 100};
        return true;
    }
    if (launchermath::Evaluate(text, &v)) {
        std::wstring n = launchermath::Number(v);
        *row = {L"= " + n, L"Enter copies it", L"calculator", L"#copy", n, nullptr, 0, 100};
        return true;
    }
    return false;
}

void CopyText(const std::wstring& text) {
    if (!OpenClipboard(g_wnd)) return;
    EmptyClipboard();
    if (HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t))) {
        memcpy(GlobalLock(mem), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
        GlobalUnlock(mem);
        if (!SetClipboardData(CF_UNICODETEXT, mem)) GlobalFree(mem);
    }
    CloseClipboard();
}

// The list again, from the apps and every answer that's in for the line as it is now.
void Rebuild() {
    std::wstring text = g_query;
    auto providers = LauncherProviders();
    std::vector<Row> rows;
    if (PrefixRows(text, &rows)) {
        g_rows = std::move(rows);
        g_sel = std::clamp(g_sel, 0, std::max(0, (int)g_rows.size() - 1));
        return Layout();
    }
    // Typed after a provider's prefix (and a space): only that one.
    for (auto const& p : providers) {
        if (p.prefix.empty() || text.size() < p.prefix.size() + 1 || _wcsnicmp(text.c_str(), p.prefix.c_str(), p.prefix.size()) ||
            text[p.prefix.size()] != L' ')
            continue;
        std::wstring rest = text.substr(p.prefix.size() + 1);
        for (auto const& item : LauncherResults(p.key, rest))
            rows.push_back({item.title, item.subtitle, item.glyph.empty() ? p.glyph : item.glyph, p.key, item.id, nullptr, 0, item.score});
        std::stable_sort(rows.begin(), rows.end(), [](Row const& a, Row const& b) { return a.score > b.score; });
        g_rows = std::move(rows);
        g_sel = std::clamp(g_sel, 0, std::max(0, (int)g_rows.size() - 1));
        return Layout();
    }
    if (text.empty()) {
        // Nothing typed: what each provider's prefix finds.
        for (auto const& p : providers)
            if (!p.prefix.empty()) rows.push_back({p.name, L"Type \"" + p.prefix + L" \" and a name", p.glyph, L"@" + p.prefix, L"", nullptr, 0, 0});
        rows.push_back({L"Web search", L"Type \"? \" and what to look for", L"world-search", L"@?", L"", nullptr, 0, 0});
        rows.push_back({L"Switch to a window", L"Type \"@ \" and its name", L"app-window", L"@@", L"", nullptr, 0, 0});
        rows.push_back({L"Snippets", L"Type \"$ \" and a snippet's name", L"clipboard-text", L"@$", L"", nullptr, 0, 0});
        rows.push_back({L"Sums and units", L"Just type them: 2+2, 10 km to mi", L"calculator", L"#hint", L"", nullptr, 0, 0});
    } else {
        {
            std::lock_guard lock(g_appsLock);
            g_shownApps = g_apps;
        }
        if (g_shownApps) {
            std::vector<Row> apps;
            for (auto const& a : *g_shownApps) {
                double score = Fuzzy(text, a.name);
                if (score >= 0) apps.push_back({a.name, L"App", L"", L"", a.target, a.icon, a.iconSize, score});
            }
            std::stable_sort(apps.begin(), apps.end(), [](Row const& a, Row const& b) { return a.score > b.score; });
            if (apps.size() > 6) apps.resize(6);
            rows = std::move(apps);
        }
        std::vector<Row> more;
        for (auto const& p : providers) {
            if (!p.global) continue;
            for (auto const& item : LauncherResults(p.key, text))
                if (item.id != L"none" && item.score > 0)
                    more.push_back({item.title, item.subtitle, item.glyph.empty() ? p.glyph : item.glyph, p.key, item.id, nullptr, 0, item.score});
        }
        std::stable_sort(more.begin(), more.end(), [](Row const& a, Row const& b) { return a.score > b.score; });
        for (auto& r : more) rows.push_back(std::move(r));
        Row answer;
        if (AnswerRow(text, &answer)) rows.insert(rows.begin(), answer);
    }
    g_rows = std::move(rows);
    g_sel = std::clamp(g_sel, 0, std::max(0, (int)g_rows.size() - 1));
    Layout();
}

// The line changed: ask the providers that should answer it, and show what's in already.
void Search() {
    int n = GetWindowTextLengthW(g_edit);
    std::wstring text(n, L'\0');
    GetWindowTextW(g_edit, text.data(), n + 1);
    g_query = text;
    g_sel = 0;
    for (auto const& p : LauncherProviders()) {
        bool prefixed = !p.prefix.empty() && text.size() > p.prefix.size() && !_wcsnicmp(text.c_str(), p.prefix.c_str(), p.prefix.size()) &&
                        text[p.prefix.size()] == L' ';
        if (prefixed) LauncherQuery(p.key, text.substr(p.prefix.size() + 1));
        else if (p.global && !text.empty()) LauncherQuery(p.key, text);
    }
    Rebuild();
}

void Hide() {
    if (!IsWindowVisible(g_wnd)) return;
    ShowWindow(g_wnd, SW_HIDE);
    g_shownApps.reset();
    g_rows.clear();
}

void Show() {
    POINT pt;
    GetCursorPos(&pt);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    RECT const& work = mi.rcWork;
    // (moved there first, so its DPI is that screen's)
    SetWindowPos(g_wnd, nullptr, work.left + 10, work.top + 10, 10, 10, SWP_NOZORDER | SWP_NOACTIVATE);
    int dpi = GetDpiForWindow(g_wnd);
    if (dpi != g_dpi || !g_font) {
        g_dpi = dpi;
        for (HFONT* f : {&g_font, &g_small, &g_glyphs})
            if (*f) DeleteObject(*f);
        g_font = CreateFontW(-S(17), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI Variable Text");
        g_small = CreateFontW(-S(12), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI Variable Text");
        g_glyphs = CreateFontW(-S(20), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"tabler-icons");
        SendMessageW(g_edit, WM_SETFONT, (WPARAM)g_font, FALSE);
    }
    Look look = Themed();
    if (g_editBrush) DeleteObject(g_editBrush);
    g_editBrush = CreateSolidBrush(look.ground);
    DWORD corner = 2;  // DWMWCP_ROUND
    DwmSetWindowAttribute(g_wnd, 33, &corner, sizeof(corner));
    DwmSetWindowAttribute(g_wnd, 34, &look.line, sizeof(look.line));
    int w = std::min<int>(S(680), work.right - work.left - S(40));
    SetWindowPos(g_wnd, HWND_TOPMOST, (work.left + work.right - w) / 2, work.top + (work.bottom - work.top) / 5, w, InputHeight(),
                 SWP_SHOWWINDOW);
    SetWindowTextW(g_edit, L"");
    UiSoundFor(L"launcher");
    RefreshApps(g_wnd, S(32));
    Search();
    SetForegroundWindow(g_wnd);
    SetFocus(g_edit);
}

void Activate(int i) {
    if (i < 0 || i >= (int)g_rows.size()) return;
    Row r = g_rows[i];
    if (r.provider.size() > 1 && r.provider[0] == L'@') {  // a provider's hint: its prefix in the line
        std::wstring line = r.provider.substr(1) + L" ";
        SetWindowTextW(g_edit, line.c_str());
        SendMessageW(g_edit, EM_SETSEL, line.size(), line.size());
        return;
    }
    if (r.provider == L"#hint") return;
    if (r.provider.size() > 1 && r.provider[0] == L'#') {
        Hide();
        if (r.provider == L"#copy") {
            CopyText(r.id);
            Log(L"copied %ls", r.id.c_str());
        } else if (r.provider == L"#web") {
            std::wstring words;
            for (wchar_t c : r.id) {  // (the words for a URL: UTF-8, escaped)
                char u[8];
                int n = WideCharToMultiByte(CP_UTF8, 0, &c, 1, u, sizeof(u), nullptr, nullptr);
                for (int i = 0; i < n; i++) {
                    unsigned char b = (unsigned char)u[i];
                    wchar_t hex[4];
                    if (isalnum(b) || b == '-' || b == '_' || b == '.' || b == '~') words += (wchar_t)b;
                    else if (b == ' ') words += L'+';
                    else swprintf(hex, 4, L"%%%02X", b), words += hex;
                }
            }
            std::wstring url = g_cfg.web;
            url.replace(url.find(L"%s"), 2, words);
            ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        } else if (r.provider == L"#window") {
            HWND w = (HWND)(ULONG_PTR)_wcstoui64(r.id.c_str(), nullptr, 10);
            if (IsWindow(w)) {
                if (IsIconic(w)) ShowWindowAsync(w, SW_RESTORE);
                if (!SetForegroundWindow(w)) SwitchToThisWindow(w, TRUE);
            }
        } else if (r.provider == L"#snippet") {
            // Copied, then pasted into whatever had the keyboard before the launcher opened.
            CopyText(r.id);
            Sleep(60);
            INPUT in[4] = {};
            for (auto& i : in) i.type = INPUT_KEYBOARD;
            in[0].ki.wVk = in[3].ki.wVk = VK_CONTROL;
            in[1].ki.wVk = in[2].ki.wVk = 'V';
            in[2].ki.dwFlags = in[3].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(4, in, sizeof(INPUT));
        } else if (r.provider == L"#snippets") {
            std::wstring path = SnippetsPath();
            if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
                HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, 0, nullptr);
                if (f != INVALID_HANDLE_VALUE) {
                    const char start[] =
                        "# Snippets for the launcher: type $ and a name, Enter pastes it.\r\n"
                        "# A line each: name = text (\\n for a new line).\r\n"
                        "shrug = \xC2\xAF\\_(\xE3\x83\x84)_/\xC2\xAF\r\n";
                    DWORD wrote;
                    WriteFile(f, start, sizeof(start) - 1, &wrote, nullptr);
                    CloseHandle(f);
                }
            }
            ShellExecuteW(nullptr, L"open", L"notepad.exe", (L"\"" + path + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
        }
        return;
    }
    Hide();
    if (r.provider.empty()) {
        std::wstring target = L"shell:AppsFolder\\" + r.id;
        SHELLEXECUTEINFOW sei{sizeof(sei)};
        sei.fMask = SEE_MASK_FLAG_NO_UI;
        sei.lpFile = target.c_str();
        sei.nShow = SW_SHOWNORMAL;
        if (!ShellExecuteExW(&sei)) Log(L"couldn't start %ls (%lu)", r.title.c_str(), GetLastError());
        else Log(L"started %ls", r.title.c_str());
    } else {
        LauncherActivate(r.provider, r.id);
    }
}

void Paint(HDC dc) {
    RECT rc;
    GetClientRect(g_wnd, &rc);
    Look look = Themed();
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
    HGDIOBJ oldBmp = SelectObject(mem, bmp);
    HBRUSH ground = CreateSolidBrush(look.ground), raised = CreateSolidBrush(look.raised), line = CreateSolidBrush(look.line),
           accent = CreateSolidBrush(look.accent);
    FillRect(mem, &rc, ground);
    SetBkMode(mem, TRANSPARENT);
    // The search glyph, then a line under the input.
    HGDIOBJ oldFont = SelectObject(mem, g_glyphs);
    SetTextColor(mem, look.accent);
    RECT g{S(14), 0, S(44), InputHeight()};
    std::wstring search = LauncherGlyph(L"search");
    DrawTextW(mem, search.c_str(), -1, &g, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (!g_rows.empty()) {
        RECT sep{S(12), InputHeight() - 1, rc.right - S(12), InputHeight()};
        FillRect(mem, &sep, line);
    }
    int y = InputHeight() + S(4);
    HGDIOBJ oldPen = SelectObject(mem, GetStockObject(NULL_PEN));
    for (int i = 0; i < (int)g_rows.size() && i < kMaxRows; i++, y += RowHeight()) {
        Row const& r = g_rows[i];
        RECT row{S(6), y, rc.right - S(6), y + RowHeight()};
        if (i == g_sel || i == g_hot) {
            SelectObject(mem, raised);
            RoundRect(mem, row.left, row.top, row.right + 1, row.bottom + 1, S(8), S(8));
        }
        if (i == g_sel) {
            RECT bar{row.left + S(2), row.top + S(10), row.left + S(5), row.bottom - S(10)};
            FillRect(mem, &bar, accent);
        }
        RECT icon{row.left + S(10), row.top, row.left + S(42), row.bottom};
        if (r.icon) {
            HDC src = CreateCompatibleDC(mem);
            HGDIOBJ o = SelectObject(src, r.icon);
            BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
            int s = S(28);
            AlphaBlend(mem, icon.left + (S(32) - s) / 2, row.top + (RowHeight() - s) / 2, s, s, src, 0, 0, r.iconSize, r.iconSize, blend);
            SelectObject(src, o);
            DeleteDC(src);
        } else if (!r.glyph.empty()) {
            SelectObject(mem, g_glyphs);
            SetTextColor(mem, i == g_sel ? look.accent : look.muted);
            std::wstring ch = LauncherGlyph(r.glyph);
            DrawTextW(mem, ch.c_str(), -1, &icon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        RECT text{row.left + S(52), row.top + S(5), row.right - S(10), row.top + S(26)};
        SelectObject(mem, g_font);
        SetTextColor(mem, look.text);
        DrawTextW(mem, r.title.c_str(), -1, &text, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
        RECT sub{text.left, row.top + S(25), text.right, row.bottom - S(3)};
        SelectObject(mem, g_small);
        SetTextColor(mem, look.muted);
        DrawTextW(mem, r.subtitle.c_str(), -1, &sub, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }
    SelectObject(mem, oldPen);
    SelectObject(mem, oldFont);
    BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    for (HGDIOBJ o : {(HGDIOBJ)bmp, (HGDIOBJ)ground, (HGDIOBJ)raised, (HGDIOBJ)line, (HGDIOBJ)accent}) DeleteObject(o);
    DeleteDC(mem);
}

int RowAt(int y) {
    int i = (y - InputHeight() - S(4)) / RowHeight();
    return y >= InputHeight() && i >= 0 && i < (int)g_rows.size() && i < kMaxRows ? i : -1;
}

LRESULT CALLBACK EditProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_KEYDOWN) {
        int n = std::min<int>((int)g_rows.size(), kMaxRows);
        switch (wp) {
            case VK_ESCAPE:
                Hide();
                return 0;
            case VK_RETURN:
                Activate(g_sel);
                return 0;
            case VK_DOWN:
            case VK_TAB:
                if (n) g_sel = (g_sel + 1) % n;
                InvalidateRect(g_wnd, nullptr, FALSE);
                return 0;
            case VK_UP:
                if (n) g_sel = (g_sel + n - 1) % n;
                InvalidateRect(g_wnd, nullptr, FALSE);
                return 0;
        }
    }
    if (msg == WM_CHAR && (wp == VK_RETURN || wp == VK_ESCAPE || wp == VK_TAB)) return 0;  // (no beep)
    return CallWindowProcW(g_editProc, w, msg, wp, lp);
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    // A bind's "launcher", or the Tourne Menu's tourne:launcher; with a global atom in wParam (a plugin's
    // tourne.openLauncher), open with that line in it instead.
    static UINT asked = RegisterWindowMessageW(L"TourneDesktop.Launcher");
    if (msg == asked && asked && wp) {
        wchar_t line[256] = L"";
        GlobalGetAtomNameW((ATOM)wp, line, 256);
        GlobalDeleteAtom((ATOM)wp);
        if (!IsWindowVisible(wnd)) Show();
        SetWindowTextW(g_edit, line);
        SendMessageW(g_edit, EM_SETSEL, wcslen(line), wcslen(line));
        return 0;
    }
    if (msg == asked && asked) msg = kMsgToggle;
    switch (msg) {
        case WM_HOTKEY:
            if (wp == kHotkey) {
                if (IsWindowVisible(wnd)) Hide();
                else Show();
            }
            return 0;
        case kMsgToggle:
            if (IsWindowVisible(wnd)) Hide();
            else Show();
            return 0;
        case kMsgResults:
        case kMsgApps:
            if (IsWindowVisible(wnd)) Rebuild();
            return 0;
        case kMsgApply: {
            g_cfg = ReadConfig();
            if (g_registered) UnregisterHotKey(wnd, kHotkey), g_registered = false;
            UINT mods, vk;
            if (g_cfg.on && ParseKeys(g_cfg.hotkey, &mods, &vk)) {
                g_registered = RegisterHotKey(wnd, kHotkey, mods, vk);
                Log(g_registered ? L"on: %ls opens it" : L"couldn't take %ls (another program has it?)", g_cfg.hotkey.c_str());
            } else if (g_cfg.on) {
                Log(L"the hotkey \"%ls\" isn't one I can read", g_cfg.hotkey.c_str());
            }
            if (!g_cfg.on) Hide();
            return 0;
        }
        case WM_COMMAND:
            if ((HWND)lp == g_edit && HIWORD(wp) == EN_CHANGE) Search();
            return 0;
        case WM_CTLCOLOREDIT: {
            Look look = Themed();
            SetTextColor((HDC)wp, look.text);
            SetBkColor((HDC)wp, look.ground);
            return (LRESULT)g_editBrush;
        }
        case WM_ACTIVATE:
            if (LOWORD(wp) == WA_INACTIVE) Hide();
            return 0;
        case WM_MOUSEMOVE: {
            int hot = RowAt(GET_Y_LPARAM(lp));
            if (hot != g_hot) g_hot = hot, InvalidateRect(wnd, nullptr, FALSE);
            TRACKMOUSEEVENT t{sizeof(t), TME_LEAVE, wnd, 0};
            TrackMouseEvent(&t);
            return 0;
        }
        case WM_MOUSELEAVE:
            g_hot = -1;
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONUP: {
            int at = RowAt(GET_Y_LPARAM(lp));
            if (at >= 0) Activate(at);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            Paint(dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_CLOSE:
            Hide();
            return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void Thread() {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HINSTANCE inst = GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.lpszClassName = L"TourneLauncher";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    g_wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"TourneLauncher", L"Launcher", WS_POPUP, 0, 0, 10, 10, nullptr, nullptr, inst,
                            nullptr);
    g_edit = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 10, 10, g_wnd, nullptr, inst, nullptr);
    g_editProc = (WNDPROC)SetWindowLongPtrW(g_edit, GWLP_WNDPROC, (LONG_PTR)EditProc);
    SendMessageW(g_edit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Search apps, sums, ? web, @ windows, $ snippets");
    LauncherSetListener(g_wnd, kMsgResults);
    SendMessageW(g_wnd, kMsgApply, 0, 0);
    RefreshApps(g_wnd, 32);  // (read ahead, so the first opening has them)
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }
    LauncherSetListener(nullptr, 0);
    if (g_registered) UnregisterHotKey(g_wnd, kHotkey);
    g_registered = false;
    DestroyWindow(g_wnd);
    g_wnd = g_edit = nullptr;
    for (HFONT* f : {&g_font, &g_small, &g_glyphs})
        if (*f) DeleteObject(*f), *f = nullptr;
    if (g_editBrush) DeleteObject(g_editBrush), g_editBrush = nullptr;
    CoUninitialize();
}

}  // namespace

void LauncherDefaults(Settings& out) {
    out[L"launcher.enabled"] = L"1";
    out[L"launcher.hotkey"] = L"alt+space";
    out[L"launcher.webSearch"] = L"https://duckduckgo.com/?q=%s";
}

void LauncherDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"launcher", L"Launcher",
        L"A search box on a hotkey: your apps (everything Start lists), and the plugins' launchers by their prefix: "
        L"Game Launcher's \"g\" and a name, Display Profiles, Syncthing. Its own: sums and units on any line (2+2, "
        L"10 km to mi, 72 f in c; Enter copies), \"? \" searches the web, \"@ \" switches to an open window, \"$ \" pastes a "
        L"snippet. Arrows pick, Enter starts, Esc closes.");
    add(L"launcher.enabled", L"Launcher", L"Off: the hotkey is left alone.").isSwitch = true;
    add(L"launcher.hotkey", L"Hotkey",
        L"Keys joined with +, for example alt+space, ctrl+shift+space, win+f1. Alt+Space usually opens a window's "
        L"system menu; the launcher takes it over while it's on.");
    add(L"launcher.webSearch", L"Web search",
        L"Where \"? \" searches, with %s for the words. DuckDuckGo unless set, so its bangs (!g, !w, !yt) work as typed.");
}

void LauncherStart(HWND host) {
    g_host = host;
    if (g_thread.joinable()) return;
    g_thread = std::thread(Thread);
}

void LauncherApply() {
    if (g_wnd) PostMessageW(g_wnd, kMsgApply, 0, 0);
}

void LauncherToggle() {
    if (g_wnd) PostMessageW(g_wnd, kMsgToggle, 0, 0);
}

void LauncherStop() {
    if (!g_thread.joinable()) return;
    for (int i = 0; i < 100 && !g_wnd; i++) Sleep(10);
    if (g_wnd) PostMessageW(g_wnd, WM_QUIT, 0, 0);
    g_thread.join();
}

}  // namespace td
