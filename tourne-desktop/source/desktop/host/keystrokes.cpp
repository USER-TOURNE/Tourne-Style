// Keystrokes (keystrokes.h): keycaps on screen for the keys pressed and the clicks made.
//
//   Rows        a combination (Ctrl+Shift+S, a click with Ctrl held) is a row of its own; keys typed one after
//               another flow along one row; the same again counts up (a "x5" badge) rather than repeating.
//               The newest row is nearest the screen's edge; older ones move away and go after a while.
//   Looks       Keyviz's four keycaps (Tz-slayer's port): minimal (flat), laptop (flat, a fine edge), low
//               profile (a key on its base, pressed down as it comes) and PBT (a tall key with a dished top).
//               Colours from the Tourne theme, or your own.
//   Motion      fade, zoom, float (rises in), slide (from the side) or none, coming and going.
//   Where       nine places on the screen in front, the one with the pointer or the main one, with margins.
//   Privacy     nothing is shown while a password box has the keys, or a program named (password managers)
//               is in front; nothing is kept or written anywhere.
//
// The hooks and the window live on a thread of their own, made when it's on and gone when it's off. A frame is
// drawn (Direct2D into the window's own picture, UpdateLayeredWindow) only while something moves; the next time
// a row is due to go is one timer; with nothing pressed, nothing runs.

#include "keystrokes.h"

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <string>
#include <vector>

#include "standalone.h"
#include "td.h"

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"keystrokes", format, args);
    va_end(args);
}

constexpr wchar_t kClass[] = L"TourneKeystrokes";
constexpr wchar_t kHotClass[] = L"TourneKeystrokesKey";
constexpr UINT WM_KS_RELOAD = WM_APP + 1;
constexpr UINT WM_KS_KEY = WM_APP + 2;    // wParam: vk | flags << 16; lParam: modifiers held
constexpr UINT WM_KS_MOUSE = WM_APP + 3;  // wParam: what (below); lParam: modifiers held
constexpr UINT_PTR kTimerFrame = 1, kTimerExpire = 2;

enum MouseWhat { kLeft, kRight, kMiddle, kBack, kForward, kWheelUp, kWheelDown, kWheelLeft, kWheelRight, kDrag };
enum Mods : UINT { kCtrl = 1, kAlt = 2, kShift = 4, kWin = 8 };

////////////////////////////////////////////////////////////////////////////////
// Settings

struct Config {
    bool showAll = true;  // false: only combinations (a modifier with a key), F keys and the like
    bool mouse = true, wheel = true, counts = true, passwordPause = true;
    int style = 2;        // 0 minimal, 1 laptop, 2 low profile, 3 PBT
    int size = 36;        // a key's height at 100%
    int place = 7;        // 0..8: top left, top, top right, left, centre, right, bottom left, bottom, bottom right
    int screen = 0;       // 0 the window in front's, 1 the pointer's, 2 the main one
    int marginX = 24, marginY = 72;
    int rows = 3;
    int linger = 2000;    // ms a row stays after its last key
    int motion = 1;       // 0 none, 1 fade, 2 zoom, 3 float, 4 slide
    int motionMs = 180;
    bool themeColors = true;
    COLORREF key = RGB(0x1B, 0x2A, 0x31), mod = RGB(0x1F, 0x3D, 0x34), text = RGB(0xE6, 0xF2, 0xEC), accent = RGB(0x15, 0xFF, 0xA2);
    int opacity = 100;
    std::wstring font = L"Segoe UI";
    std::vector<std::wstring> pausePrograms;
};

std::wstring Str(const wchar_t* key) {
    const wchar_t* v = GetString(L"keystrokes", key);
    std::wstring s = v ? v : L"";
    FreeString(v);
    return s;
}
int Int(const wchar_t* key) { return GetInt(L"keystrokes", key); }

COLORREF Hex(std::wstring s, COLORREF fallback) {
    s.erase(0, s.find_first_not_of(L" #"));
    if (s.size() < 6) return fallback;
    unsigned v = wcstoul(s.substr(0, 6).c_str(), nullptr, 16);
    return RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

int Pick(std::wstring const& v, std::initializer_list<const wchar_t*> names, int fallback) {
    int i = 0;
    for (const wchar_t* n : names) {
        if (v == n) return i;
        i++;
    }
    return fallback;
}

Config ReadConfig() {
    Config c;
    c.showAll = Str(L"filter") != L"shortcuts";
    c.mouse = Int(L"mouse") != 0;
    c.wheel = Int(L"wheel") != 0;
    c.counts = Int(L"counts") != 0;
    c.passwordPause = Int(L"passwordPause") != 0;
    c.style = Pick(Str(L"style"), {L"minimal", L"laptop", L"lowprofile", L"pbt"}, 2);
    c.size = std::clamp(Int(L"size"), 20, 96);
    c.place = Pick(Str(L"position"), {L"topleft", L"top", L"topright", L"left", L"center", L"right", L"bottomleft", L"bottom", L"bottomright"}, 7);
    c.screen = Pick(Str(L"screen"), {L"front", L"pointer", L"main"}, 0);
    c.marginX = std::clamp(Int(L"marginX"), 0, 2000);
    c.marginY = std::clamp(Int(L"marginY"), 0, 2000);
    c.rows = std::clamp(Int(L"rows"), 1, 8);
    c.linger = std::clamp(Int(L"linger"), 300, 20000);
    c.motion = Pick(Str(L"animation"), {L"none", L"fade", L"zoom", L"float", L"slide"}, 1);
    c.motionMs = std::clamp(Int(L"animationTime"), 0, 2000);
    c.themeColors = Str(L"colors") != L"custom";
    c.opacity = std::clamp(Int(L"opacity"), 10, 100);
    if (std::wstring f = Str(L"font"); !f.empty()) c.font = f;
    uint32_t p[11];
    if (c.themeColors && FlyoutPalette(p)) {
        auto rgb = [](uint32_t v) { return RGB((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF); };
        c.key = rgb(p[1]), c.mod = rgb(p[10]), c.text = rgb(p[4]), c.accent = rgb(p[5]);
    } else if (!c.themeColors) {
        c.key = Hex(Str(L"keyColor"), c.key);
        c.mod = Hex(Str(L"modifierColor"), c.mod);
        c.text = Hex(Str(L"textColor"), c.text);
        c.accent = Hex(Str(L"badgeColor"), c.accent);
    }
    std::wstring list = Str(L"pausePrograms");
    for (size_t at = 0; at <= list.size();) {
        size_t comma = list.find(L',', at);
        std::wstring one = list.substr(at, comma == std::wstring::npos ? std::wstring::npos : comma - at);
        one.erase(0, one.find_first_not_of(L" \t"));
        one.erase(one.find_last_not_of(L" \t") + 1);
        for (auto& ch : one) ch = towlower(ch);
        if (!one.empty()) c.pausePrograms.push_back(one);
        if (comma == std::wstring::npos) break;
        at = comma + 1;
    }
    return c;
}

////////////////////////////////////////////////////////////////////////////////
// What's shown

struct Cap {
    std::wstring label;
    bool modifier = false;
    int count = 1;     // the same key pressed again in a row
    DWORD id = 0;      // the key (vk) or the button (0x1000 | what): repeats of the same count up
};

struct Row {
    std::vector<Cap> caps;
    bool combo = false;  // a combination: nothing typed joins it
    UINT mods = 0;
    int count = 1;       // the combination again
    double born = 0, touched = 0, leaving = 0;  // ms; leaving: when it started to go (0: staying)
    double pressed = 0;  // when its last key went down (the low profile key's press)
};

Config g_cfg;
std::vector<Row> g_rows;
HWND g_wnd, g_hotWnd, g_host;
HHOOK g_keyHook, g_mouseHook;
HANDLE g_thread;
DWORD g_threadId;
std::atomic<bool> g_running{false};
std::atomic<int> g_forced{-1};  // toggled: 1 on, 0 off, -1 as the setting says
bool g_settingOn;
UINT g_heldMods;            // (the hook thread's own count of what's held)
bool g_soloModifier;        // a modifier went down and nothing else since
DWORD g_soloVk;
POINT g_pressAt;
bool g_leftDown, g_dragging;
HMONITOR g_monitor;

double NowMs() {
    static LARGE_INTEGER f = [] {
        LARGE_INTEGER x;
        QueryPerformanceFrequency(&x);
        return x;
    }();
    LARGE_INTEGER n;
    QueryPerformanceCounter(&n);
    return n.QuadPart * 1000.0 / f.QuadPart;
}

std::wstring ModName(UINT m) {
    switch (m) {
        case kWin: return L"Win";
        case kCtrl: return L"Ctrl";
        case kAlt: return L"Alt";
        default: return L"Shift";
    }
}

std::wstring KeyLabel(DWORD vk, DWORD scan, bool extended) {
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) return std::wstring(1, (wchar_t)vk);
    if (vk >= VK_F1 && vk <= VK_F24) return L"F" + std::to_wstring(vk - VK_F1 + 1);
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) return L"Num " + std::to_wstring(vk - VK_NUMPAD0);
    switch (vk) {
        case VK_SPACE: return L"Space";
        case VK_RETURN: return extended ? L"Num Enter" : L"Enter";
        case VK_TAB: return L"Tab";
        case VK_BACK: return L"⌫";
        case VK_ESCAPE: return L"Esc";
        case VK_DELETE: return L"Del";
        case VK_INSERT: return L"Ins";
        case VK_HOME: return L"Home";
        case VK_END: return L"End";
        case VK_PRIOR: return L"PgUp";
        case VK_NEXT: return L"PgDn";
        case VK_LEFT: return L"←";
        case VK_RIGHT: return L"→";
        case VK_UP: return L"↑";
        case VK_DOWN: return L"↓";
        case VK_CAPITAL: return L"Caps";
        case VK_SNAPSHOT: return L"PrtSc";
        case VK_PAUSE: return L"Pause";
        case VK_APPS: return L"Menu";
        case VK_NUMLOCK: return L"NumLk";
        case VK_SCROLL: return L"ScrLk";
        case VK_MULTIPLY: return L"Num *";
        case VK_ADD: return L"Num +";
        case VK_SUBTRACT: return L"Num -";
        case VK_DIVIDE: return L"Num /";
        case VK_DECIMAL: return L"Num .";
        case VK_VOLUME_UP: return L"Vol +";
        case VK_VOLUME_DOWN: return L"Vol -";
        case VK_VOLUME_MUTE: return L"Mute";
        case VK_MEDIA_PLAY_PAUSE: return L"⏯";
        case VK_MEDIA_NEXT_TRACK: return L"⏭";
        case VK_MEDIA_PREV_TRACK: return L"⏮";
        case VK_MEDIA_STOP: return L"⏹";
    }
    // Punctuation as the keyboard layout prints it.
    if (UINT ch = MapVirtualKeyW(vk, MAPVK_VK_TO_CHAR) & 0x7FFF; ch >= 0x21) return std::wstring(1, (wchar_t)towupper((wchar_t)ch));
    WCHAR name[64] = L"";
    if (scan && GetKeyNameTextW((LONG)((scan << 16) | (extended ? 1 << 24 : 0)), name, ARRAYSIZE(name))) return name;
    return L"key " + std::to_wstring(vk);
}

bool IsModifier(DWORD vk) {
    switch (vk) {
        case VK_LCONTROL: case VK_RCONTROL: case VK_CONTROL: case VK_LMENU: case VK_RMENU: case VK_MENU:
        case VK_LSHIFT: case VK_RSHIFT: case VK_SHIFT: case VK_LWIN: case VK_RWIN:
            return true;
    }
    return false;
}

UINT ModOf(DWORD vk) {
    switch (vk) {
        case VK_LCONTROL: case VK_RCONTROL: case VK_CONTROL: return kCtrl;
        case VK_LMENU: case VK_RMENU: case VK_MENU: return kAlt;
        case VK_LSHIFT: case VK_RSHIFT: case VK_SHIFT: return kShift;
        case VK_LWIN: case VK_RWIN: return kWin;
    }
    return 0;
}

// Keys shown even when only shortcuts are: they aren't typing.
bool NotTyping(DWORD vk) {
    return (vk >= VK_F1 && vk <= VK_F24) || vk == VK_ESCAPE || vk == VK_SNAPSHOT || vk == VK_PAUSE || vk == VK_APPS ||
           (vk >= VK_VOLUME_MUTE && vk <= VK_MEDIA_PLAY_PAUSE);
}

// A password box has the keys, or a program named is in front: nothing is shown.
bool Paused() {
    HWND front = GetForegroundWindow();
    if (!front) return false;
    if (!g_cfg.pausePrograms.empty()) {
        DWORD pid = 0;
        GetWindowThreadProcessId(front, &pid);
        static DWORD lastPid;
        static bool lastPaused;
        if (pid != lastPid) {
            lastPid = pid;
            lastPaused = false;
            if (HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
                WCHAR path[MAX_PATH];
                DWORD n = ARRAYSIZE(path);
                if (QueryFullProcessImageNameW(h, 0, path, &n)) {
                    std::wstring exe = path;
                    exe = exe.substr(exe.find_last_of(L"\\/") + 1);
                    for (auto& ch : exe) ch = towlower(ch);
                    for (auto const& p : g_cfg.pausePrograms) lastPaused |= exe == p;
                }
                CloseHandle(h);
            }
        }
        if (lastPaused) return true;
    }
    if (g_cfg.passwordPause) {
        GUITHREADINFO gti{sizeof(gti)};
        if (GetGUIThreadInfo(GetWindowThreadProcessId(front, nullptr), &gti) && gti.hwndFocus) {
            if (GetWindowLongW(gti.hwndFocus, GWL_STYLE) & ES_PASSWORD) {
                WCHAR cls[32] = L"";
                GetClassNameW(gti.hwndFocus, cls, ARRAYSIZE(cls));
                if (!_wcsicmp(cls, L"Edit") || wcsstr(cls, L"EDIT") || wcsstr(cls, L"Edit")) return true;
            }
        }
    }
    return false;
}

void Frame();
void Schedule();

Row& NewRow(bool combo, UINT mods) {
    double now = NowMs();
    // The oldest go once there are more than asked for.
    int staying = 0;
    for (auto const& r : g_rows) staying += r.leaving == 0;
    for (auto& r : g_rows)
        if (r.leaving == 0 && staying >= g_cfg.rows) r.leaving = now, staying--;
    Row r;
    r.combo = combo;
    r.mods = mods;
    r.born = r.touched = r.pressed = now;
    g_rows.push_back(r);
    return g_rows.back();
}

Row* LiveRow() {
    for (auto it = g_rows.rbegin(); it != g_rows.rend(); ++it)
        if (it->leaving == 0) return &*it;
    return nullptr;
}

void PickScreen() {
    POINT pt;
    GetCursorPos(&pt);
    HWND front = GetForegroundWindow();
    g_monitor = g_cfg.screen == 1 ? MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY)
              : g_cfg.screen == 2 || !front ? MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY)
                                           : MonitorFromWindow(front, MONITOR_DEFAULTTOPRIMARY);
}

// A key or a click: onto the live row, or a new one.
void Add(Cap cap, UINT mods) {
    if (Paused()) return;
    double now = NowMs();
    bool combo = mods != 0;
    if (!g_rows.empty() && LiveRow() == nullptr) g_rows.erase(std::remove_if(g_rows.begin(), g_rows.end(), [](Row const& r) { return r.leaving; }), g_rows.end());
    if (g_rows.empty()) PickScreen();
    Row* live = LiveRow();
    if (combo) {
        std::vector<Cap> caps;
        for (UINT m : {kWin, kCtrl, kAlt, kShift})
            if (mods & m) caps.push_back({ModName(m), true, 1, 0x2000 | m});
        caps.push_back(cap);
        // The same combination again: counted.
        if (live && live->combo && live->mods == mods && live->caps.back().id == cap.id && now - live->touched < g_cfg.linger && g_cfg.counts) {
            live->count++;
            live->touched = live->pressed = now;
        } else {
            NewRow(true, mods).caps = caps;
        }
    } else {
        bool joins = live && !live->combo && now - live->touched < g_cfg.linger && live->caps.size() < 14;
        if (joins && g_cfg.counts && live->caps.back().id == cap.id) {
            live->caps.back().count++;
        } else if (joins) {
            live->caps.push_back(cap);
        } else {
            NewRow(false, 0).caps.push_back(cap);
        }
        if (Row* r = LiveRow()) r->touched = r->pressed = now;
    }
    Frame();
}

void OnKey(DWORD vk, DWORD scan, bool extended, bool down) {
    if (vk == 0xE8 || vk == 0xFF || vk == 0) return;  // (keys sent so a lone Win doesn't open Start)
    if (IsModifier(vk)) {
        UINT m = ModOf(vk);
        if (down) {
            if (!(g_heldMods & m)) g_soloModifier = g_heldMods == 0, g_soloVk = vk;
            g_heldMods |= m;
        } else {
            g_heldMods &= ~m;
            // A modifier pressed and let go on its own (Win for Start): shown as itself.
            if (g_soloModifier && g_soloVk == vk && g_cfg.showAll) Add({ModName(m), true, 1, 0x2000 | m}, 0);
            g_soloModifier = false;
        }
        return;
    }
    g_soloModifier = false;
    if (!down) return;
    UINT mods = g_heldMods;
    bool shortcut = (mods & ~kShift) != 0 || NotTyping(vk);
    if (!g_cfg.showAll && !shortcut) return;
    // (Shift with a letter is typing: the letter alone)
    Add({KeyLabel(vk, scan, extended), false, 1, vk}, (mods & ~kShift) || NotTyping(vk) ? mods : 0);
}

void OnMouse(int what) {
    static const wchar_t* const kNames[] = {L"Left click", L"Right click", L"Middle click", L"Back", L"Forward",
                                            L"Scroll ↑", L"Scroll ↓", L"Scroll ←", L"Scroll →", L"Drag"};
    if (what == kDrag) {  // (the click just shown becomes a drag)
        if (Row* r = LiveRow(); r && !r->caps.empty() && r->caps.back().id == (0x1000 | kLeft)) {
            r->caps.back().label = kNames[kDrag];
            r->caps.back().id = 0x1000 | kDrag;
            r->touched = NowMs();
            Frame();
        }
        return;
    }
    Add({kNames[what], false, 1, (DWORD)(0x1000 | what)}, g_heldMods);
}

// The hooks live on a thread of their own that only pumps: on the drawing thread, every key and mouse move in
// Windows waited for each frame's present while the badges animated (the pointer hitched after a click).
std::atomic<bool> g_hookWheel{false};
HANDLE g_hookThread = nullptr;
DWORD g_hookThreadId = 0;

LRESULT CALLBACK KeyProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        auto* k = (KBDLLHOOKSTRUCT*)lp;
        bool down = wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN;
        // (handled after the hook returns: the key isn't held up)
        WPARAM packed = (k->vkCode & 0xFF) | ((k->flags & LLKHF_EXTENDED) ? 1u << 30 : 0) | (down ? 1u << 31 : 0);
        PostMessageW(g_wnd, WM_KS_KEY, packed, k->scanCode);
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

LRESULT CALLBACK MouseProc(int code, WPARAM wp, LPARAM lp) {
    if (code == HC_ACTION) {
        auto* m = (MSLLHOOKSTRUCT*)lp;
        int what = -1;
        switch (wp) {
            case WM_LBUTTONDOWN:
                what = kLeft;
                g_leftDown = true, g_dragging = false, g_pressAt = m->pt;
                break;
            case WM_LBUTTONUP: g_leftDown = false; break;
            case WM_RBUTTONDOWN: what = kRight; break;
            case WM_MBUTTONDOWN: what = kMiddle; break;
            case WM_XBUTTONDOWN: what = HIWORD(m->mouseData) == XBUTTON1 ? kBack : kForward; break;
            case WM_MOUSEWHEEL:
                if (g_hookWheel) what = (short)HIWORD(m->mouseData) > 0 ? kWheelUp : kWheelDown;
                break;
            case WM_MOUSEHWHEEL:
                if (g_hookWheel) what = (short)HIWORD(m->mouseData) > 0 ? kWheelRight : kWheelLeft;
                break;
            case WM_MOUSEMOVE:
                if (g_leftDown && !g_dragging && (std::abs(m->pt.x - g_pressAt.x) > 12 || std::abs(m->pt.y - g_pressAt.y) > 12))
                    g_dragging = true, what = kDrag;
                break;
        }
        if (what >= 0) PostMessageW(g_wnd, WM_KS_MOUSE, what, 0);
    }
    return CallNextHookEx(nullptr, code, wp, lp);
}

////////////////////////////////////////////////////////////////////////////////
// Drawing

ID2D1Factory* g_d2d;
IDWriteFactory* g_dw;
ID2D1DCRenderTarget* g_rt;
IDWriteTextFormat* g_textFormat;
IDWriteTextFormat* g_badgeFormat;
float g_textFormatSize;
HDC g_memDC;
HBITMAP g_bitmap;
HGDIOBJ g_oldBitmap;
int g_bitmapW, g_bitmapH;

template <typename T>
void Release(T*& p) {
    if (p) p->Release();
    p = nullptr;
}

UINT DpiOf(HMONITOR m) {
    using Fn = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    static Fn fn = (Fn)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor");
    UINT x = 96, y = 96;
    if (fn && m) fn(m, 0, &x, &y);
    return x;
}

D2D1_COLOR_F Color(COLORREF c, float a = 1) { return {GetRValue(c) / 255.f, GetGValue(c) / 255.f, GetBValue(c) / 255.f, a}; }
COLORREF Shade(COLORREF c, float f) {
    auto s = [&](int v) { return (BYTE)std::clamp((int)(v * f), 0, 255); };
    return RGB(s(GetRValue(c)), s(GetGValue(c)), s(GetBValue(c)));
}
// A colour's lightness (0..1): dark keys get lighter edges, light keys darker ones.
float Lightness(COLORREF c) { return (0.299f * GetRValue(c) + 0.587f * GetGValue(c) + 0.114f * GetBValue(c)) / 255.f; }

void EnsureFormats(float px) {
    if (g_textFormat && g_textFormatSize == px) return;
    Release(g_textFormat);
    Release(g_badgeFormat);
    g_textFormatSize = px;
    g_dw->CreateTextFormat(g_cfg.font.c_str(), nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, px,
                           L"", &g_textFormat);
    g_dw->CreateTextFormat(g_cfg.font.c_str(), nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, px * 0.62f,
                           L"", &g_badgeFormat);
    for (IDWriteTextFormat* f : {g_textFormat, g_badgeFormat})
        if (f) {
            f->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            f->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            f->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        }
}

float TextWidth(std::wstring const& s, IDWriteTextFormat* f) {
    IDWriteTextLayout* layout = nullptr;
    float w = 0;
    if (f && SUCCEEDED(g_dw->CreateTextLayout(s.c_str(), (UINT32)s.size(), f, 4000, 400, &layout))) {
        DWRITE_TEXT_METRICS m{};
        layout->GetMetrics(&m);
        w = m.widthIncludingTrailingWhitespace;
        layout->Release();
    }
    return w;
}

struct Geometry {
    float scale, h, base, gap, rowGap, pad, radius;
};

Geometry Geo(UINT dpi) {
    Geometry g;
    g.scale = dpi / 96.f;
    g.h = g_cfg.size * g.scale;
    g.base = g_cfg.style == 2 ? g.h * 0.12f : g_cfg.style == 3 ? g.h * 0.16f : 0;  // the low profile and PBT keys' base
    g.gap = g.h * 0.16f;
    g.rowGap = g.h * 0.28f;
    g.pad = g.h * 0.34f;
    g.radius = g_cfg.style == 0 ? g.h * 0.22f : g_cfg.style == 1 ? g.h * 0.12f : g.h * 0.16f;
    return g;
}

float CapWidth(Cap const& c, Geometry const& g) {
    return std::max(g.h, TextWidth(c.label, g_textFormat) + 2 * g.pad);
}

float RowWidth(Row const& r, Geometry const& g) {
    float w = 0;
    for (auto const& c : r.caps) w += CapWidth(c, g) + g.gap;
    return std::max(0.f, w - g.gap);
}

void DrawCap(Cap const& c, D2D1_RECT_F box, Geometry const& g, float alpha, float press, ID2D1SolidColorBrush* brush) {
    COLORREF fill = c.modifier ? g_cfg.mod : g_cfg.key;
    bool dark = Lightness(fill) < 0.5f;
    float r = g.radius;
    auto rr = [&](D2D1_RECT_F rc, COLORREF col, float a) {
        brush->SetColor(Color(col, a * alpha));
        g_rt->FillRoundedRectangle({rc, r, r}, brush);
    };
    float sink = press * g.base * 0.6f;  // (pressed: the top comes down onto its base)
    D2D1_RECT_F top = box;
    switch (g_cfg.style) {
        case 0:  // minimal
            rr(box, fill, 0.92f);
            break;
        case 1:  // laptop
            rr(box, fill, 1);
            brush->SetColor(Color(dark ? Shade(fill, 1.6f) : Shade(fill, 0.75f), alpha));
            g_rt->DrawRoundedRectangle({{box.left + 0.5f, box.top + 0.5f, box.right - 0.5f, box.bottom - 0.5f}, r, r}, brush, std::max(1.f, g.scale));
            break;
        case 2: {  // low profile: the key on its base
            D2D1_RECT_F base = {box.left, box.top + g.base, box.right, box.bottom + g.base};
            rr(base, Shade(fill, dark ? 0.6f : 0.72f), 1);
            top = {box.left, box.top + sink, box.right, box.bottom + sink};
            rr(top, fill, 1);
            break;
        }
        case 3: {  // PBT: a tall key, its top dished in
            D2D1_RECT_F base = {box.left, box.top, box.right, box.bottom + g.base};
            rr(base, Shade(fill, dark ? 0.65f : 0.75f), 1);
            float in = g.h * 0.09f;
            top = {box.left + in, box.top + in * 0.5f + sink, box.right - in, box.bottom - in + sink};
            rr(top, dark ? Shade(fill, 1.25f) : Shade(fill, 1.06f), 1);
            break;
        }
    }
    brush->SetColor(Color(g_cfg.text, alpha));
    g_rt->DrawTextW(c.label.c_str(), (UINT32)c.label.size(), g_textFormat, top, brush, D2D1_DRAW_TEXT_OPTIONS_NONE);
    if (c.count > 1) {
        std::wstring n = L"×" + std::to_wstring(c.count);
        float bw = std::max(g.h * 0.55f, TextWidth(n, g_badgeFormat) + g.h * 0.24f), bh = g.h * 0.42f;
        D2D1_RECT_F b = {top.right - bw * 0.7f, top.top - bh * 0.45f, top.right - bw * 0.7f + bw, top.top - bh * 0.45f + bh};
        brush->SetColor(Color(g_cfg.accent, alpha));
        g_rt->FillRoundedRectangle({b, bh / 2, bh / 2}, brush);
        brush->SetColor(Color(Lightness(g_cfg.accent) > 0.55f ? RGB(0, 0, 0) : RGB(255, 255, 255), alpha));
        g_rt->DrawTextW(n.c_str(), (UINT32)n.size(), g_badgeFormat, b, brush, D2D1_DRAW_TEXT_OPTIONS_NONE);
    }
}

double Ease(double t) { return 1 - std::pow(1 - std::clamp(t, 0.0, 1.0), 3); }

// One frame: every row where it is now. True while anything still moves.
bool Draw() {
    double now = NowMs();
    // Rows whose time is up start to go; gone ones are dropped.
    for (auto& r : g_rows)
        if (r.leaving == 0 && now - r.touched >= g_cfg.linger) r.leaving = now;
    double span = std::max(1, g_cfg.motionMs);
    g_rows.erase(std::remove_if(g_rows.begin(), g_rows.end(),
                                [&](Row const& r) { return r.leaving && (g_cfg.motion == 0 || now - r.leaving >= span); }),
                 g_rows.end());
    if (g_rows.empty()) {
        ShowWindow(g_wnd, SW_HIDE);
        return false;
    }
    if (!g_monitor) PickScreen();
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(g_monitor, &mi);
    UINT dpi = DpiOf(g_monitor);
    Geometry g = Geo(dpi);
    EnsureFormats(g.h * 0.42f);
    // The window: room for every row, plus some for the motion and the badges.
    float rowH = g.h + g.base, widest = 0;
    for (auto const& r : g_rows)
        if (r.caps.size()) widest = std::max(widest, RowWidth(r, g));
    float margin = g.h * 0.9f;
    int w = (int)std::ceil(widest + 2 * margin), h = (int)std::ceil(g_rows.size() * (rowH + g.rowGap) + 2 * margin);
    int col = g_cfg.place % 3, line = g_cfg.place / 3;
    RECT work = mi.rcWork;
    int mx = (int)(g_cfg.marginX * g.scale), my = (int)(g_cfg.marginY * g.scale);
    int x = col == 0 ? work.left + mx - (int)margin : col == 2 ? work.right - mx - w + (int)margin : (work.left + work.right - w) / 2;
    int y = line == 0 ? work.top + my - (int)margin : line == 2 ? work.bottom - my - h + (int)margin : (work.top + work.bottom - h) / 2;
    if (w != g_bitmapW || h != g_bitmapH) {
        if (g_bitmap) SelectObject(g_memDC, g_oldBitmap), DeleteObject(g_bitmap);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
        void* bits = nullptr;
        g_bitmap = CreateDIBSection(g_memDC, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        g_oldBitmap = SelectObject(g_memDC, g_bitmap);
        g_bitmapW = w, g_bitmapH = h;
    }
    RECT bind{0, 0, w, h};
    g_rt->BindDC(g_memDC, &bind);
    g_rt->BeginDraw();
    g_rt->Clear({0, 0, 0, 0});
    ID2D1SolidColorBrush* brush = nullptr;
    g_rt->CreateSolidColorBrush(Color(g_cfg.text), &brush);
    bool moving = false;
    // The newest row nearest the screen's edge it's placed at (the bottom, unless it's placed at the top).
    bool fromTop = line == 0;
    int n = (int)g_rows.size();
    for (int i = 0; i < n; i++) {
        Row const& r = g_rows[i];
        int slot = n - 1 - i;  // 0: the newest
        float rw = RowWidth(r, g);
        float ry = fromTop ? margin + slot * (rowH + g.rowGap) : h - margin - rowH - slot * (rowH + g.rowGap);
        float rx = col == 0 ? margin : col == 2 ? w - margin - rw : (w - rw) / 2;
        double in = g_cfg.motion ? std::min(1.0, (now - r.born) / span) : 1;
        double out = r.leaving && g_cfg.motion ? std::min(1.0, (now - r.leaving) / span) : 0;
        moving |= in < 1 || out > 0;
        float a = (float)(Ease(in) * (1 - Ease(out)));
        D2D1_MATRIX_3X2_F m = D2D1::Matrix3x2F::Identity();
        float alpha = g_cfg.motion == 0 ? 1.f : a;
        D2D1_POINT_2F centre = {rx + rw / 2, ry + rowH / 2};
        if (g_cfg.motion == 2) {  // zoom
            float s = 0.6f + 0.4f * (float)Ease(in) - 0.25f * (float)Ease(out);
            m = D2D1::Matrix3x2F::Scale(s, s, centre);
        } else if (g_cfg.motion == 3) {  // float: up from below, and on up as it goes
            float dy = (1 - (float)Ease(in)) * g.h * 0.6f - (float)Ease(out) * g.h * 0.6f;
            m = D2D1::Matrix3x2F::Translation(0, fromTop ? -dy : dy);
        } else if (g_cfg.motion == 4) {  // slide in from the side it's placed at (the left for the middle)
            float dx = (1 - (float)Ease(in)) * g.h * 2.2f + (float)Ease(out) * g.h * 2.2f;
            m = D2D1::Matrix3x2F::Translation(col == 2 ? dx : -dx, 0);
        }
        g_rt->SetTransform(m);
        float press = (float)std::max(0.0, 1 - (now - r.pressed) / 140.0);  // (the low profile key, pressed and back up)
        moving |= press > 0;
        float cx = rx;
        for (size_t c = 0; c < r.caps.size(); c++) {
            Cap cap = r.caps[c];
            if (c + 1 == r.caps.size() && r.count > 1) cap.count = r.count;  // (a combination counted on its last key)
            float cw = CapWidth(cap, g);
            bool last = c + 1 == r.caps.size();
            DrawCap(cap, {cx, ry, cx + cw, ry + g.h}, g, alpha * g_cfg.opacity / 100.f, last ? press : 0, brush);
            cx += cw + g.gap;
        }
    }
    g_rt->SetTransform(D2D1::Matrix3x2F::Identity());
    brush->Release();
    if (g_rt->EndDraw() == (HRESULT)D2DERR_RECREATE_TARGET) {
        Release(g_rt);
        D2D1_RENDER_TARGET_PROPERTIES p = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        g_d2d->CreateDCRenderTarget(&p, &g_rt);
    }
    POINT at{x, y}, zero{0, 0};
    SIZE size{w, h};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(g_wnd, nullptr, &at, &size, g_memDC, &zero, 0, &blend, ULW_ALPHA);
    if (!IsWindowVisible(g_wnd)) ShowWindow(g_wnd, SW_SHOWNOACTIVATE);
    SetWindowPos(g_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return moving;
}

// Frames while something moves; then one timer, for the next row to go.
void Schedule() {
    double now = NowMs(), next = 0;
    for (auto const& r : g_rows)
        if (r.leaving == 0) next = next ? std::min(next, r.touched + g_cfg.linger) : r.touched + g_cfg.linger;
    if (next) SetTimer(g_wnd, kTimerExpire, (UINT)std::max(10.0, next - now + 5), nullptr);
    else KillTimer(g_wnd, kTimerExpire);
}

void Frame() {
    bool moving = Draw();
    if (moving) SetTimer(g_wnd, kTimerFrame, 15, nullptr);
    else KillTimer(g_wnd, kTimerFrame);
    Schedule();
}

DWORD WINAPI HookThread(LPVOID mouse) {
    g_keyHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyProc, GetModuleHandleW(nullptr), 0);
    if (mouse) g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, MouseProc, GetModuleHandleW(nullptr), 0);
    MSG m;
    while (GetMessageW(&m, nullptr, 0, 0) > 0) DispatchMessageW(&m);
    if (g_keyHook) UnhookWindowsHookEx(g_keyHook), g_keyHook = nullptr;
    if (g_mouseHook) UnhookWindowsHookEx(g_mouseHook), g_mouseHook = nullptr;
    return 0;
}

// (again with each settings change: the mouse hook only while clicks are shown)
void Hooks(bool on) {
    if (g_hookThread) {
        // (again until it's gone: it may not have a message queue yet)
        for (int i = 0; i < 100 && WaitForSingleObject(g_hookThread, 0) == WAIT_TIMEOUT; i++) {
            PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
            WaitForSingleObject(g_hookThread, 20);
        }
        CloseHandle(g_hookThread);
        g_hookThread = nullptr;
    }
    if (!on) return;
    g_hookWheel = g_cfg.wheel;
    g_hookThread = CreateThread(nullptr, 0, HookThread, (LPVOID)(INT_PTR)(g_cfg.mouse ? 1 : 0), 0, &g_hookThreadId);
}

LRESULT CALLBACK Proc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_KS_KEY:
            OnKey(wp & 0xFF, (DWORD)lp, (wp >> 30) & 1, (wp >> 31) & 1);
            return 0;
        case WM_KS_MOUSE:
            OnMouse((int)wp);
            return 0;
        case WM_KS_RELOAD:
            g_cfg = ReadConfig();
            Release(g_textFormat);
            Release(g_badgeFormat);
            Hooks(true);
            if (!g_rows.empty()) Frame();
            return 0;
        case WM_TIMER:
            if (wp == kTimerFrame || wp == kTimerExpire) Frame();
            return 0;
        case WM_NCHITTEST: return HTTRANSPARENT;
    }
    return DefWindowProcW(w, msg, wp, lp);
}

DWORD WINAPI Thread(LPVOID started) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    g_cfg = ReadConfig();
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_d2d);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&g_dw);
    D2D1_RENDER_TARGET_PROPERTIES p =
        D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
    if (g_d2d) g_d2d->CreateDCRenderTarget(&p, &g_rt);
    g_memDC = CreateCompatibleDC(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = Proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kClass;
    RegisterClassW(&wc);
    g_wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kClass, L"Tourne keystrokes",
                            WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
    if (g_wnd && g_rt && g_dw) Hooks(true);
    g_running = g_wnd && g_rt && g_dw;
    SetEvent((HANDLE)started);
    if (g_running) {
        Log(L"on");
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) DispatchMessageW(&m);
    }
    Hooks(false);
    if (g_wnd) DestroyWindow(g_wnd);
    g_wnd = nullptr;
    g_rows.clear();
    g_heldMods = 0;
    Release(g_textFormat);
    Release(g_badgeFormat);
    Release(g_rt);
    Release(g_dw);
    Release(g_d2d);
    if (g_bitmap) SelectObject(g_memDC, g_oldBitmap), DeleteObject(g_bitmap), g_bitmap = nullptr;
    g_bitmapW = g_bitmapH = 0;
    if (g_memDC) DeleteDC(g_memDC), g_memDC = nullptr;
    UnregisterClassW(kClass, GetModuleHandleW(nullptr));
    g_running = false;
    CoUninitialize();
    return 0;
}

void StartThread() {
    if (g_thread) return;
    HANDLE started = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_thread = CreateThread(nullptr, 0, Thread, started, 0, &g_threadId);
    if (g_thread) WaitForSingleObject(started, 3000);
    CloseHandle(started);
}

void StopThread() {
    if (!g_thread) return;
    PostThreadMessageW(g_threadId, WM_QUIT, 0, 0);
    if (g_wnd) PostMessageW(g_wnd, WM_QUIT, 0, 0);
    WaitForSingleObject(g_thread, 3000);
    CloseHandle(g_thread);
    g_thread = nullptr;
    Log(L"off");
}

bool Wanted() { return g_forced >= 0 ? g_forced == 1 : g_settingOn; }

void Follow() {
    if (Wanted()) {
        if (!g_thread) StartThread();
        else if (g_wnd) PostMessageW(g_wnd, WM_KS_RELOAD, 0, 0);
    } else {
        StopThread();
    }
}

// The hotkey: "ctrl+alt+75" as the Studio writes it (or "Ctrl+Alt+K").
bool ParseKeys(std::wstring const& text, UINT* mods, UINT* vk) {
    *mods = *vk = 0;
    std::wstring part;
    for (wchar_t c : text + L"+") {
        if (c != L'+') {
            if (!iswspace(c)) part += (wchar_t)towlower(c);
            continue;
        }
        if (part == L"ctrl" || part == L"control") *mods |= MOD_CONTROL;
        else if (part == L"alt") *mods |= MOD_ALT;
        else if (part == L"shift") *mods |= MOD_SHIFT;
        else if (part == L"win") *mods |= MOD_WIN;
        else if (!part.empty() && iswdigit(part[0])) *vk = (UINT)_wtoi(part.c_str());
        else if (part.size() == 1) *vk = (UINT)towupper(part[0]);
        else if (part.size() >= 2 && part[0] == L'f' && iswdigit(part[1])) *vk = VK_F1 + _wtoi(part.c_str() + 1) - 1;
        part.clear();
    }
    return *vk != 0;
}

LRESULT CALLBACK HotProc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_HOTKEY) {
        KeystrokesShow(-1);
        return 0;
    }
    if (msg == WM_APP + 9) {  // (shown or hidden from another thread)
        Follow();
        return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}

void RegisterToggleKey() {
    if (!g_hotWnd) return;
    UnregisterHotKey(g_hotWnd, 1);
    UINT mods, vk;
    if (ParseKeys(Str(L"hotkey"), &mods, &vk) && !RegisterHotKey(g_hotWnd, 1, mods | MOD_NOREPEAT, vk))
        Log(L"its hotkey is taken by another program");
}

}  // namespace

void KeystrokesDefaults(Settings& out) {
    out[L"keystrokes.enabled"] = L"0";
    out[L"keystrokes.hotkey"] = L"";
    out[L"keystrokes.filter"] = L"all";
    out[L"keystrokes.mouse"] = L"1";
    out[L"keystrokes.wheel"] = L"1";
    out[L"keystrokes.counts"] = L"1";
    out[L"keystrokes.style"] = L"lowprofile";
    out[L"keystrokes.size"] = L"36";
    out[L"keystrokes.position"] = L"bottom";
    out[L"keystrokes.screen"] = L"front";
    out[L"keystrokes.marginX"] = L"24";
    out[L"keystrokes.marginY"] = L"72";
    out[L"keystrokes.rows"] = L"3";
    out[L"keystrokes.linger"] = L"2000";
    out[L"keystrokes.animation"] = L"float";
    out[L"keystrokes.animationTime"] = L"180";
    out[L"keystrokes.colors"] = L"theme";
    out[L"keystrokes.keyColor"] = L"1B2A31";
    out[L"keystrokes.modifierColor"] = L"1F3D34";
    out[L"keystrokes.textColor"] = L"E6F2EC";
    out[L"keystrokes.badgeColor"] = L"15FFA2";
    out[L"keystrokes.opacity"] = L"100";
    out[L"keystrokes.font"] = L"Segoe UI";
    out[L"keystrokes.passwordPause"] = L"1";
    out[L"keystrokes.pausePrograms"] = L"KeePassXC.exe, KeePass.exe, 1Password.exe, Bitwarden.exe, Dashlane.exe";
}

void KeystrokesDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"keystrokes", L"Keystrokes",
        L"The keys you press and the clicks you make, as keycaps on screen, for screencasts, tutorials and demos (Keyviz's "
        L"keycaps, from Tz-slayer's Keystrokes and hthienloc's Screenkey). Nothing runs while it's off; on, nothing runs "
        L"while nothing is pressed. Never shown in password boxes or the programs named below.");
    add(L"keystrokes.enabled", L"Show keystrokes", L"").isSwitch = true;
    add(L"keystrokes.hotkey", L"Hotkey to show or hide them",
        L"Shows or hides them for now (the switch above says how Tourne starts). None until you set one.")
        .format = L"hotkey";
    add(L"keystrokes.filter", L"Which keys", L"").options = {{L"all", L"Every key (typing too)"},
                                                            {L"shortcuts", L"Only shortcuts (with Ctrl, Alt or Win, and F keys)"}};
    add(L"keystrokes.mouse", L"Clicks too", L"Left, right and middle clicks, the side buttons, and drags.").isSwitch = true;
    add(L"keystrokes.wheel", L"The scroll wheel too", L"").isSwitch = true;
    add(L"keystrokes.counts", L"Count repeats", L"The same key or combination again shows a count (x3) instead of another keycap.").isSwitch = true;
    add(L"keystrokes.style", L"Keycaps", L"Keyviz's four.").options = {
        {L"minimal", L"Minimal (flat)"}, {L"laptop", L"Laptop (flat, a fine edge)"}, {L"lowprofile", L"Low profile (on a base, pressed down)"},
        {L"pbt", L"PBT (tall, a dished top)"}};
    SettingInfo& size = add(L"keystrokes.size", L"Size", L"A key's height in pixels at 100% scale.");
    size.hasMin = size.hasMax = true, size.min = 20, size.max = 96;
    add(L"keystrokes.position", L"Where", L"").options = {
        {L"topleft", L"Top left"}, {L"top", L"Top"}, {L"topright", L"Top right"}, {L"left", L"Left"}, {L"center", L"Middle"},
        {L"right", L"Right"}, {L"bottomleft", L"Bottom left"}, {L"bottom", L"Bottom"}, {L"bottomright", L"Bottom right"}};
    add(L"keystrokes.screen", L"Which screen", L"").options = {
        {L"front", L"The one with the window in front"}, {L"pointer", L"The one with the pointer"}, {L"main", L"The main one"}};
    add(L"keystrokes.marginX", L"Distance from the side", L"Pixels at 100% scale.");
    add(L"keystrokes.marginY", L"Distance from the top or bottom", L"Pixels at 100% scale.");
    SettingInfo& rows = add(L"keystrokes.rows", L"Rows at once", L"Older rows move up and go.");
    rows.hasMin = rows.hasMax = true, rows.min = 1, rows.max = 8;
    add(L"keystrokes.linger", L"How long a row stays (ms)", L"After its last key.");
    add(L"keystrokes.animation", L"Coming and going", L"").options = {
        {L"none", L"At once"}, {L"fade", L"Fade"}, {L"zoom", L"Zoom"}, {L"float", L"Float (rise in)"}, {L"slide", L"Slide from the side"}};
    add(L"keystrokes.animationTime", L"Its time (ms)", L"");
    add(L"keystrokes.colors", L"Colours", L"").options = {{L"theme", L"The Tourne theme's"}, {L"custom", L"My own (below)"}};
    add(L"keystrokes.keyColor", L"Key colour", L"With My own colours. RRGGBB.");
    add(L"keystrokes.modifierColor", L"Ctrl, Alt, Shift and Win's colour", L"With My own colours.");
    add(L"keystrokes.textColor", L"Label colour", L"With My own colours.");
    add(L"keystrokes.badgeColor", L"Count colour", L"With My own colours.");
    SettingInfo& op = add(L"keystrokes.opacity", L"Opacity (%)", L"");
    op.hasMin = op.hasMax = true, op.min = 10, op.max = 100;
    add(L"keystrokes.font", L"Font", L"");
    add(L"keystrokes.passwordPause", L"Hide in password boxes", L"Nothing is shown while a classic password box has the keys.").isSwitch = true;
    add(L"keystrokes.pausePrograms", L"Hide in these programs", L"File names, separated by commas: password managers and the like.");
}

void KeystrokesStart(HWND host) {
    g_host = host;
    if (!g_hotWnd) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = HotProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = kHotClass;
        RegisterClassW(&wc);
        g_hotWnd = CreateWindowExW(0, kHotClass, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
    }
    KeystrokesApply();
}

void KeystrokesApply() {
    bool on = GetInt(L"keystrokes", L"enabled") != 0;
    if (on != g_settingOn) g_forced = -1;  // (the switch changed: it says again)
    g_settingOn = on;
    RegisterToggleKey();
    Follow();
}

void KeystrokesStop() {
    StopThread();
    if (g_hotWnd) UnregisterHotKey(g_hotWnd, 1), DestroyWindow(g_hotWnd), g_hotWnd = nullptr;
}

bool KeystrokesShow(int how) {
    bool now = Wanted();
    bool to = how < 0 ? !now : how != 0;
    g_forced = to ? 1 : 0;
    if (g_hotWnd) PostMessageW(g_hotWnd, WM_APP + 9, 0, 0);  // (started and stopped on the host's thread)
    return to;
}

bool KeystrokesShowing() { return g_running; }

}  // namespace td
