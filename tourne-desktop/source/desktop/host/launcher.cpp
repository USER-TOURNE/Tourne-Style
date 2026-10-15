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
};

Config ReadConfig() {
    Config c;
    c.on = GetInt(L"launcher", L"enabled") != 0;
    if (const wchar_t* v = GetString(L"launcher", L"hotkey")) {
        if (*v) c.hotkey = v;
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

// The list again, from the apps and every answer that's in for the line as it is now.
void Rebuild() {
    std::wstring text = g_query;
    auto providers = LauncherProviders();
    std::vector<Row> rows;
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
    SendMessageW(g_edit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Search apps, or a prefix (g for games)");
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
        L"Game Launcher's \"g\" and a name, Display Profiles, Syncthing. Arrows pick, Enter starts, Esc closes.");
    add(L"launcher.enabled", L"Launcher", L"Off: the hotkey is left alone.").isSwitch = true;
    add(L"launcher.hotkey", L"Hotkey",
        L"Keys joined with +, for example alt+space, ctrl+shift+space, win+f1. Alt+Space usually opens a window's "
        L"system menu; the launcher takes it over while it's on.");
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
