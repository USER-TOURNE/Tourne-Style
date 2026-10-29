// Designer Mode: see designer.h.
//
// The screen is frozen into a picture under a window that covers every screen; a loupe follows the pointer with
// the colour under it. A click takes that colour; then the window goes and the next click names a part: the
// window under the pointer, by its class, gives a menu of the settings that colour it (in Tourne's menu look).

#include "designer.h"

#include <windowsx.h>

#include <algorithm>
#include <cstdarg>
#include <string>
#include <vector>

#include "standalone.h"
#include "td.h"
#include "theme-engine.h"

#include <cwctype>

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"designer", format, args);
    va_end(args);
}

struct State {
    HWND wnd = nullptr;
    HDC shot = nullptr;  // the frozen screen
    HBITMAP shotBmp = nullptr;
    RECT screen{};       // the virtual screen
    int step = 0;        // 0 picking the colour, 1 aiming at a part, 2 done
    COLORREF picked = 0;
    POINT at{};          // where the part was clicked
    bool cancelled = false;
};
State* g_s = nullptr;

COLORREF ColourAt(State& s, POINT pt) { return GetPixel(s.shot, pt.x - s.screen.left, pt.y - s.screen.top); }

std::wstring Hex(COLORREF c, bool hash) {
    wchar_t b[16];
    swprintf(b, 16, hash ? L"#%02X%02X%02X" : L"%02X%02X%02X", GetRValue(c), GetGValue(c), GetBValue(c));
    return b;
}

void Paint(State& s, HDC dc) {
    RECT rc;
    GetClientRect(s.wnd, &rc);
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
    HGDIOBJ old = SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, rc.right, rc.bottom, s.shot, 0, 0, SRCCOPY);
    POINT pt;
    GetCursorPos(&pt);
    int x = pt.x - s.screen.left, y = pt.y - s.screen.top;
    // The loupe: 15 by 15 pixels made 9 times bigger, the middle one framed, the colour's hex under it.
    const int n = 15, z = 9, size = n * z;
    int lx = x + 24, ly = y + 24;
    if (lx + size + 8 > rc.right) lx = x - 24 - size;
    if (ly + size + 40 > rc.bottom) ly = y - 24 - size - 32;
    StretchBlt(mem, lx, ly, size, size, s.shot, x - n / 2, y - n / 2, n, n, SRCCOPY);
    HBRUSH edge = CreateSolidBrush(RGB(0x15, 0xFF, 0xA2));
    RECT frame{lx - 1, ly - 1, lx + size + 1, ly + size + 1};
    FrameRect(mem, &frame, edge);
    RECT mid{lx + (n / 2) * z, ly + (n / 2) * z, lx + (n / 2 + 1) * z, ly + (n / 2 + 1) * z};
    FrameRect(mem, &mid, edge);
    DeleteObject(edge);
    COLORREF c = ColourAt(s, pt);
    RECT label{lx, ly + size + 4, lx + size, ly + size + 30};
    HBRUSH back = CreateSolidBrush(RGB(0x12, 0x1C, 0x21));
    FillRect(mem, &label, back);
    DeleteObject(back);
    RECT swatch{label.left + 4, label.top + 4, label.left + 22, label.bottom - 4};
    HBRUSH sw = CreateSolidBrush(c);
    FillRect(mem, &swatch, sw);
    DeleteObject(sw);
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(0x66, 0xEA, 0x97));
    HGDIOBJ oldFont = SelectObject(mem, GetStockObject(DEFAULT_GUI_FONT));
    RECT text{swatch.right + 6, label.top, label.right, label.bottom};
    std::wstring hex = Hex(c, true);
    DrawTextW(mem, hex.c_str(), -1, &text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    // What to do, along the top.
    RECT tip{0, 8, rc.right, 36};
    SetTextColor(mem, RGB(0xE5, 0x74, 0x74));
    std::wstring what = s.step == 0 ? L"Designer: click a colour to take it (Esc stops)"
                                    : L"Designer: " + Hex(s.picked, true) + L" taken. Now click a part of Tourne to colour (Esc stops)";
    DrawTextW(mem, what.c_str(), -1, &tip, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, oldFont);
    BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
}

LRESULT CALLBACK Proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    State* s = g_s;
    if (!s) return DefWindowProcW(wnd, msg, wp, lp);
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            Paint(*s, dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_MOUSEMOVE:
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        case WM_LBUTTONDOWN: {
            // The first click takes the colour; the second names the part (still on the frozen screen, so the
            // click never reaches what's under it).
            POINT pt;
            GetCursorPos(&pt);
            if (s->step == 0) s->picked = ColourAt(*s, pt), s->step = 1;
            else s->at = pt, s->step = 2;
            InvalidateRect(wnd, nullptr, FALSE);
            return 0;
        }
        case WM_KEYDOWN:
            if (wp == VK_ESCAPE) s->cancelled = true, s->step = 2;
            return 0;
        case WM_RBUTTONDOWN:
            s->cancelled = true, s->step = 2;
            return 0;
        case WM_SETCURSOR:
            SetCursor(LoadCursorW(nullptr, IDC_CROSS));
            return TRUE;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

// A part of Tourne by its top window's class, and the settings that colour it: {name, key, written with a #}.
struct Choice {
    std::wstring name, key;
    bool hash;
};

std::wstring ClassOf(HWND w) {
    wchar_t cls[128] = L"";
    GetClassNameW(w, cls, 128);
    return cls;
}

bool TableClass(std::wstring const& c) { return c.rfind(L"DesktopAudioVis", 0) == 0 || c.rfind(L"TourneTable", 0) == 0; }

// Tourne'Table's windows let clicks through (the visualizer is drawn over the desktop), so the window from the
// point is what's under them: the windows at the point top down, a Table window found before anything that
// takes clicks.
HWND TableWindowAt(POINT pt) {
    struct Find {
        POINT pt;
        HWND found = nullptr;
    } f{pt};
    EnumWindows(
        [](HWND w, LPARAM lp) -> BOOL {
            auto* f = (Find*)lp;
            RECT r;
            if (!IsWindowVisible(w) || !GetWindowRect(w, &r) || !PtInRect(&r, f->pt)) return TRUE;
            if (TableClass(ClassOf(w))) return f->found = w, FALSE;
            if (GetWindowLongW(w, GWL_EXSTYLE) & WS_EX_TRANSPARENT) return TRUE;
            return FALSE;  // (a window that takes clicks is over any Table window below it)
        },
        (LPARAM)&f);
    return f.found;
}

// A program's file name, lower case.
std::wstring ProgramOf(HWND w) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    std::wstring name;
    if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        wchar_t path[MAX_PATH];
        DWORD n = MAX_PATH;
        if (QueryFullProcessImageNameW(p, 0, path, &n)) name = wcsrchr(path, L'\\') ? wcsrchr(path, L'\\') + 1 : path;
        CloseHandle(p);
    }
    for (auto& c : name) c = (wchar_t)towlower(c);
    return name;
}

std::wstring PartOf(HWND w, std::vector<Choice>* out) {
    HWND top = GetAncestor(w, GA_ROOT);
    std::wstring c = ClassOf(top);
    if (!TableClass(c))
        if (HWND table = TableWindowAt(g_s->at)) c = ClassOf(table);
    auto starts = [&](const wchar_t* p) { return c.rfind(p, 0) == 0; };
    if (TableClass(c)) {
        *out = {{L"Bars", L"table.appearance.color", true}, {L"Gradient's second colour", L"table.appearance.gradientColor2", true},
                {L"Peak hold", L"table.appearance.peakHoldColor", true}, {L"Beat flash", L"table.appearance.beatFlashColor", true},
                {L"Outline", L"table.appearance.fxOutlineColor", true}, {L"Bass line", L"table.bassLine.bodyColor", true},
                {L"Now playing", L"table.appearance.nowPlayingColor", true}, {L"Now playing's artist", L"table.appearance.nowPlayingArtistColor", true},
                {L"Now playing's background", L"table.appearance.nowPlayingBgColor", true},
                {L"Borders", L"table.background.borderColor", true}, {L"Progress", L"table.progress.color", true},
                {L"Progress track", L"table.progress.trackColor", true}, {L"Media controls' icons", L"table.media_controls.iconColor", true},
                {L"Media controls' plate", L"table.media_controls.plateColor", true}};
        return L"Tourne'Table";
    }
    // An app the CSS injector styles: its sheet's own colours, over the Tourne theme's (css-injector.cpp).
    std::wstring program = ProgramOf(top), sheet;
    if (program.rfind(L"discord", 0) == 0) sheet = L"discord";
    else if (program == L"claude.exe") sheet = L"claude";
    else if (program == L"spotify.exe") sheet = L"spotify";
    else if (c == L"Chrome_WidgetWin_1" && program != L"chrome.exe" && program != L"msedge.exe") sheet = L"generic";
    if (!sheet.empty()) {
        std::wstring k = L"inject.sheet." + sheet + L".";
        *out = {{L"Background", k + L"ground", true}, {L"Cards and panels", k + L"raised", true}, {L"Text", k + L"text", true},
                {L"Accent", k + L"accent", true}, {L"Lines", k + L"line", true}};
        return sheet == L"generic" ? L"Styled apps (the shared sheet: Slack, Obsidian, VS Code...)" : L"Styled app (" + program + L")";
    }
    if (starts(L"TourneTaskbarTest_")) {
        *out = {{L"The bar's tint", L"taskbar.coloringColor", false}, {L"Shadows", L"taskbar.shadowColor", false},
                {L"Previews' background", L"taskbar.previewBackground", false}, {L"Menus' frame", L"taskbar.menuFrameColors", true}};
        return L"The bar";
    }
    if (c == L"#32768" || starts(L"TourneStartMenu_") || starts(L"TourneFlyout") || starts(L"TourneBalloon") ||
        starts(L"TourneTraySystemIcons_") || starts(L"TourneNotifyPopup_")) {
        *out = {{L"The frame", L"taskbar.menuFrameColors", true}};
        return L"Menus and flyouts";
    }
    if (starts(L"TourneSwitcher")) {
        *out = {{L"Background", L"switcher.colors.background", true}, {L"Selection", L"switcher.colors.selection", true},
                {L"Selection border", L"switcher.colors.selectionBorder", true}, {L"Text", L"switcher.colors.text", true},
                {L"Border", L"switcher.colors.border", true}};
        return L"The switcher";
    }
    out->clear();
    return L"";
}

}  // namespace

void DesignerStart(HWND host) {
    if (g_s) return;
    State s;
    g_s = &s;
    s.screen = {GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0, 0};
    s.screen.right = s.screen.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    s.screen.bottom = s.screen.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int w = s.screen.right - s.screen.left, h = s.screen.bottom - s.screen.top;
    HDC screen = GetDC(nullptr);
    s.shot = CreateCompatibleDC(screen);
    s.shotBmp = CreateCompatibleBitmap(screen, w, h);
    HGDIOBJ oldShot = SelectObject(s.shot, s.shotBmp);
    BitBlt(s.shot, 0, 0, w, h, screen, s.screen.left, s.screen.top, SRCCOPY | CAPTUREBLT);
    ReleaseDC(nullptr, screen);
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = Proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"TourneDesigner";
        registered = RegisterClassW(&wc) != 0;
    }
    s.wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"TourneDesigner", L"Tourne designer", WS_POPUP, s.screen.left, s.screen.top, w, h,
                            nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (s.wnd) {
        ShowWindow(s.wnd, SW_SHOW);
        SetForegroundWindow(s.wnd);
        SetFocus(s.wnd);
        MSG msg;
        BOOL got = 1;
        while (s.step < 2 && (got = GetMessageW(&msg, nullptr, 0, 0)) > 0) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        DestroyWindow(s.wnd);
        if (got == 0) PostQuitMessage((int)msg.wParam);
    }
    SelectObject(s.shot, oldShot);
    DeleteObject(s.shotBmp);
    DeleteDC(s.shot);
    if (s.cancelled || !s.wnd) {
        g_s = nullptr;
        return;
    }
    std::wstring hex = Hex(s.picked, true);
    POINT pt = s.at;
    std::vector<Choice> choices;
    std::wstring part = PartOf(WindowFromPoint(pt), &choices);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, ((part.empty() ? std::wstring(L"Not one of Tourne's parts") : part) + L": " + hex).c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    for (size_t i = 0; i < choices.size(); i++) AppendMenuW(menu, MF_STRING, 10 + i, choices[i].name.c_str());
    if (!choices.empty()) AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 2, L"Make the Tourne theme from it");
    AppendMenuW(menu, MF_STRING, 4, L"Use it as Windows' accent");
    if (WindowsAccentSaved()) AppendMenuW(menu, MF_STRING, 5, L"Restore my Windows colours (as they were before Tourne)");
    AppendMenuW(menu, MF_STRING, 3, L"Copy it");
    using Styled = UINT (*)(HMENU, UINT, int, int, HWND);
    static Styled styled = (Styled)GetProcAddress(GetModuleHandleW(nullptr), "TourneStyledMenu");
    SetForegroundWindow(host);
    UINT flags = TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY;
    int id = styled ? (int)styled(menu, flags, pt.x, pt.y, host) : TrackPopupMenuEx(menu, flags, pt.x, pt.y, host, nullptr);
    DestroyMenu(menu);
    if (id >= 10 && id < 10 + (int)choices.size()) {
        Choice const& c = choices[id - 10];
        std::vector<std::pair<std::wstring, std::wstring>> set{{c.key, Hex(s.picked, c.hash)}};
        if (c.key == L"taskbar.coloringColor") set.push_back({L"taskbar.coloring", L"custom"});  // (a tint only shows when it's on)
        WriteStudioSettings(set, L"", L"designer");
        Log(L"%ls set to %ls", c.key.c_str(), hex.c_str());
    } else if (id == 4) {
        // Windows' own accent (title bars, Start, Settings, every app that asks Windows). The theme's own writing of
        // it (theme.windows) goes off, or it would put the theme's accent back.
        WriteWindowsAccent(((DWORD)GetRValue(s.picked) << 16) | ((DWORD)GetGValue(s.picked) << 8) | GetBValue(s.picked));
        if (WindowsColoursOn()) KeepWindowsColoursOnce();  // (switching it off would put the old ones back)
        WriteStudioSettings({{L"theme.windows", L"off"}}, L"", L"designer");
        Log(L"Windows' accent set to %ls", hex.c_str());
    } else if (id == 5) {
        WriteStudioSettings({{L"theme.windows", L"off"}}, L"", L"designer");  // (first: else the theme writes its accent again)
        RestoreWindowsAccent();
    } else if (id == 2) {
        WriteStudioSettings({{L"theme.source", L"seed"}, {L"theme.seed", hex}}, L"", L"designer");
        Log(L"the Tourne theme made from %ls", hex.c_str());
    } else if (id == 3 && OpenClipboard(host)) {
        EmptyClipboard();
        if (HGLOBAL m = GlobalAlloc(GMEM_MOVEABLE, (hex.size() + 1) * sizeof(wchar_t))) {
            memcpy(GlobalLock(m), hex.c_str(), (hex.size() + 1) * sizeof(wchar_t));
            GlobalUnlock(m);
            if (!SetClipboardData(CF_UNICODETEXT, m)) GlobalFree(m);
        }
        CloseClipboard();
    }
    g_s = nullptr;
}

}  // namespace td
