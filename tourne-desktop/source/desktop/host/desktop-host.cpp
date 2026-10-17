#include "desktop-host.h"

#include <commctrl.h>
#include <shlobj.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <uxtheme.h>
#include <windowsx.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <atomic>
#include <memory>

#include <dwmapi.h>

#include "glass-engine.h"
#include "td.h"
#include "shell-mode.h"

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"desktop", format, args);
    va_end(args);
}

// SID_STopLevelBrowser, which the folder view asks its browser for.
constexpr GUID kTopLevelBrowser = {0x4C96BE40, 0x915C, 0x11CF, {0x99, 0xD3, 0x00, 0xAA, 0x00, 0x4A, 0xE8, 0x37}};

std::wstring KnownPath(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path))) out = path;
    CoTaskMemFree(path);
    return out;
}

// Whether an icon belongs on the desktop, as Explorer's desktop decides: files and folders that are
// in a Desktop folder (yours or the public one); Windows' own items (This PC, Network, the Recycle Bin,
// Home, Gallery, the user and OneDrive folders...) only where the desktop icon settings show them
// (HideDesktopIcons\NewStartPanel = 0, as the Recycle Bin has by default). Without this the view shows
// every item the Desktop namespace has (tools/desktop/desktop-view-test.cpp: 15 icons, Explorer 1).
bool DesktopShows(PCUITEMID_CHILD item) {
    IShellFolder* desktop = nullptr;
    if (FAILED(SHGetDesktopFolder(&desktop))) return true;
    STRRET ret;
    wchar_t* text = nullptr;
    std::wstring parsing;
    if (SUCCEEDED(desktop->GetDisplayNameOf(item, SHGDN_FORPARSING, &ret)) && SUCCEEDED(StrRetToStrW(&ret, item, &text))) {
        parsing = text;
        CoTaskMemFree(text);
    }
    desktop->Release();
    if (parsing.empty()) return true;
    if (parsing.rfind(L"::{", 0) == 0) {
        std::wstring clsid = parsing.substr(2, parsing.find(L'}') - 1);
        DWORD value = 1, size = sizeof(value);
        bool set = RegGetValueW(HKEY_CURRENT_USER,
                                L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel",
                                clsid.c_str(), RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS;
        return set && value == 0;
    }
    std::wstring folder = parsing.substr(0, parsing.find_last_of(L'\\'));
    return !_wcsicmp(folder.c_str(), KnownPath(FOLDERID_Desktop).c_str()) ||
           !_wcsicmp(folder.c_str(), KnownPath(FOLDERID_PublicDesktop).c_str());
}

// What the folder view needs from the window holding it: little, since the
// desktop has no menus, toolbar or status bar. Folders opened from it open
// in File Explorer. As a file dialog's browser (ICommDlgBrowser) it picks
// which icons show (DesktopShows). Its layout options (IFolderViewOptions) let
// icons stay where they're dropped: since Windows 7 the view keeps each icon in
// its arranged place, and auto arrange off only counts with FVO_CUSTOMPOSITION
// (Windows' notes on IFolderView2::SetCurrentFolderFlags); without it an icon
// dragged anywhere went back.
class Browser : public IShellBrowser, public IServiceProvider, public ICommDlgBrowser, public IFolderViewOptions {
   public:
    explicit Browser(HWND wnd) : wnd_(wnd) {}
    IShellView* view = nullptr;

    STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleWindow) || IsEqualIID(riid, IID_IShellBrowser))
            *out = static_cast<IShellBrowser*>(this);
        else if (IsEqualIID(riid, IID_IServiceProvider))
            *out = static_cast<IServiceProvider*>(this);
        else if (IsEqualIID(riid, IID_ICommDlgBrowser))
            *out = static_cast<ICommDlgBrowser*>(this);
        else if (IsEqualIID(riid, IID_IFolderViewOptions))
            *out = static_cast<IFolderViewOptions*>(this);
        else {
            *out = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&refs_); }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG left = InterlockedDecrement(&refs_);
        if (!left) delete this;
        return left;
    }

    STDMETHODIMP GetWindow(HWND* out) override {
        *out = wnd_;
        return S_OK;
    }
    STDMETHODIMP ContextSensitiveHelp(BOOL) override { return E_NOTIMPL; }

    STDMETHODIMP InsertMenusSB(HMENU, LPOLEMENUGROUPWIDTHS) override { return E_NOTIMPL; }
    STDMETHODIMP SetMenuSB(HMENU, HOLEMENU, HWND) override { return E_NOTIMPL; }
    STDMETHODIMP RemoveMenusSB(HMENU) override { return E_NOTIMPL; }
    STDMETHODIMP SetStatusTextSB(LPCWSTR) override { return S_OK; }
    STDMETHODIMP EnableModelessSB(BOOL) override { return S_OK; }
    STDMETHODIMP TranslateAcceleratorSB(MSG*, WORD) override { return S_FALSE; }
    STDMETHODIMP BrowseObject(PCUIDLIST_RELATIVE item, UINT) override {
        // (the desktop is the root, so an item below it names itself in full)
        SHELLEXECUTEINFOW run{sizeof(run)};
        run.fMask = SEE_MASK_IDLIST;
        run.lpIDList = (void*)item;
        run.nShow = SW_SHOWNORMAL;
        if (ShellExecuteExW(&run)) return S_OK;
        Log(L"couldn't open a folder from the desktop (error %lu)", GetLastError());
        return E_FAIL;
    }
    STDMETHODIMP GetViewStateStream(DWORD, IStream** out) override {
        *out = nullptr;
        return E_NOTIMPL;
    }
    STDMETHODIMP GetControlWindow(UINT, HWND* out) override {
        *out = nullptr;
        return E_NOTIMPL;
    }
    STDMETHODIMP SendControlMsg(UINT, UINT, WPARAM, LPARAM, LRESULT* result) override {
        if (result) *result = 0;
        return E_NOTIMPL;
    }
    STDMETHODIMP QueryActiveShellView(IShellView** out) override {
        *out = view;
        if (!view) return E_FAIL;
        view->AddRef();
        return S_OK;
    }
    STDMETHODIMP OnViewWindowActive(IShellView*) override { return S_OK; }
    STDMETHODIMP SetToolbarItems(LPTBBUTTONSB, UINT, UINT) override { return S_OK; }

    STDMETHODIMP QueryService(REFGUID service, REFIID riid, void** out) override {
        if (IsEqualGUID(service, kTopLevelBrowser) || IsEqualGUID(service, IID_IShellBrowser) ||
            IsEqualGUID(service, IID_ICommDlgBrowser) || IsEqualIID(riid, IID_IFolderViewOptions))
            return QueryInterface(riid, out);
        *out = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP OnDefaultCommand(IShellView*) override { return S_FALSE; }  // (the view opens it, as before)
    STDMETHODIMP OnStateChange(IShellView*, ULONG) override { return S_OK; }
    STDMETHODIMP IncludeObject(IShellView*, PCUITEMID_CHILD item) override { return DesktopShows(item) ? S_OK : S_FALSE; }

    STDMETHODIMP SetFolderViewOptions(FOLDERVIEWOPTIONS mask, FOLDERVIEWOPTIONS flags) override {
        options_ = (options_ & ~mask) | (flags & mask);
        return S_OK;
    }
    STDMETHODIMP GetFolderViewOptions(FOLDERVIEWOPTIONS* out) override {
        if (!out) return E_POINTER;
        *out = options_;
        static bool logged;
        if (!logged) Log(L"the icons' view asked for its layout options (icons placed freely)");
        logged = true;
        return S_OK;
    }

   private:
    HWND wnd_;
    LONG refs_ = 1;
    FOLDERVIEWOPTIONS options_ = FVO_CUSTOMPOSITION;
};

HWND g_desktop, g_viewWnd;
Browser* g_browser;
bool g_ole;

RECT AllScreens() {
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN), y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    return {x, y, x + GetSystemMetrics(SM_CXVIRTUALSCREEN), y + GetSystemMetrics(SM_CYVIRTUALSCREEN)};
}

void Fit() {
    RECT rc = AllScreens();
    SetWindowPos(g_desktop, HWND_BOTTOM, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE);
}

// The wallpaper. Windows' own painter (PaintDesktop) only has a picture
// once something hands it one, and with Explorer not running nothing has, so
// it paints black. So the picture is handed over again (for the icons' view,
// which paints with it) and drawn here too, as Windows places it on each
// screen.
HBITMAP g_wall = nullptr;
int g_wallW = 0, g_wallH = 0;
DWORD g_wallLoaded = 0;  // GetTickCount when last loaded

// The picture Windows has now, as it was set (IDesktopWallpaper: the registry's WallPaper lags behind a
// change made through it); empty when it can't say.
std::wstring DesktopWallpaperPath() {
    std::wstring out;
    IDesktopWallpaper* dw = nullptr;
    if (FAILED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) return out;
    LPWSTR file = nullptr;
    if (dw->GetWallpaper(nullptr, &file) != S_OK || !file || !*file) {  // (S_FALSE: the screens differ: the first one's)
        CoTaskMemFree(file);
        file = nullptr;
        LPWSTR id = nullptr;
        if (SUCCEEDED(dw->GetMonitorDevicePathAt(0, &id))) dw->GetWallpaper(id, &file);
        CoTaskMemFree(id);
    }
    if (file && *file && GetFileAttributesW(file) != INVALID_FILE_ATTRIBUTES) out = file;
    CoTaskMemFree(file);
    dw->Release();
    return out;
}

// hand: give Windows the picture too (at start-up and when the screens change: with Explorer not
// running, nothing else has); never after a change, which would put back what was there before.
// chosen: the picture just set, drawn as is (reading it back from Windows can still give the one before).
void LoadWallpaper(bool hand = false, std::wstring const& chosen = L"") {
    g_wallLoaded = GetTickCount();
    std::wstring path = chosen.empty() ? DesktopWallpaperPath() : chosen;
    wchar_t value[MAX_PATH * 2]{};
    DWORD size = sizeof(value);
    if (path.empty() &&
        RegGetValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper", RRF_RT_REG_SZ, nullptr, value, &size) == ERROR_SUCCESS)
        path = value;
    if (path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        PWSTR roaming = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming)))
            path = std::wstring(roaming) + L"\\Microsoft\\Windows\\Themes\\TranscodedWallpaper";
        CoTaskMemFree(roaming);
    }
    bool handed = hand && GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES &&
                  SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)path.c_str(), 0);
    Backdrop back;
    bool drawn = ComposeWallpaper(&back, chosen.empty() ? nullptr : chosen.c_str()) && back.width > 0 && back.height > 0 &&
                 back.pixels.size() == (size_t)back.width * back.height;
    if (g_wall) DeleteObject(g_wall);
    g_wall = nullptr;
    if (drawn) {
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), back.width, -back.height, 1, 32, BI_RGB};
        void* bits = nullptr;
        HDC screen = GetDC(nullptr);
        g_wall = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        ReleaseDC(nullptr, screen);
        if (g_wall) memcpy(bits, back.pixels.data(), back.pixels.size() * 4);
        g_wallW = back.width, g_wallH = back.height;
    }
    Log(L"wallpaper %s: %s to Windows, %s here", path.empty() ? L"(none)" : path.c_str(), handed ? L"handed" : L"not handed",
        g_wall ? L"drawn" : L"not drawn");
}

void PaintWallpaper(HWND wnd, HDC dc) {
    if (!g_wall) {
        PaintDesktop(dc);
        return;
    }
    RECT rc;
    GetClientRect(wnd, &rc);
    HDC mem = CreateCompatibleDC(dc);
    HGDIOBJ old = SelectObject(mem, g_wall);
    SetStretchBltMode(dc, HALFTONE);
    StretchBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, g_wallW, g_wallH, SRCCOPY);
    SelectObject(mem, old);
    DeleteDC(mem);
}

// Where the icons go: each screen's work area, the main screen's first (new
// icons land there), as Explorer has it. Without these the view lays them out
// from the top left of the box round every screen, where there may be none.
BOOL CALLBACK AddScreen(HMONITOR monitor, HDC, LPRECT, LPARAM out) {
    MONITORINFO mi{sizeof(mi)};
    if (GetMonitorInfoW(monitor, &mi)) {
        auto* areas = (std::vector<RECT>*)out;
        if (mi.dwFlags & MONITORINFOF_PRIMARY) areas->insert(areas->begin(), mi.rcWork);
        else areas->push_back(mi.rcWork);
    }
    return TRUE;
}

void IconAreas() {
    HWND list = g_viewWnd ? FindWindowExW(g_viewWnd, nullptr, L"SysListView32", nullptr) : nullptr;
    if (!list) return;
    std::vector<RECT> areas;
    EnumDisplayMonitors(nullptr, nullptr, AddScreen, (LPARAM)&areas);
    if (areas.size() > LV_MAX_WORKAREAS) areas.resize(LV_MAX_WORKAREAS);
    RECT all = AllScreens();
    for (auto& a : areas) OffsetRect(&a, -all.left, -all.top);  // (in the view's own terms)
    if (!areas.empty()) SendMessageW(list, LVM_SETWORKAREAS, areas.size(), (LPARAM)areas.data());
}

// The selection box fading out once the mouse is let go (shell.marquee), as the Desktop Marquee Fade
// mod gave Explorer's desktop: the box drawn once into a see-through window just above the desktop,
// which then fades. The drag is seen on the icons' own view, whose marquee runs inside its button-down
// handling, so nothing outside this process is watched (the mod needed a mouse hook for every program).
struct Fade {
    float alpha, step;
};
std::map<HWND, Fade> g_fades;
POINT g_dragFrom;
bool g_dragging;
constexpr UINT_PTR kFadeTimer = 1;

int MarqueeInt(const wchar_t* key, int low, int high) { return std::clamp(GetInt(L"shell", key), low, high); }

// A colour setting as 0xRRGGBB; empty or unreadable: Windows' selection colour.
uint32_t MarqueeColor(const wchar_t* key) {
    const wchar_t* value = GetString(L"shell", key);
    std::wstring hex = value ? value : L"";
    FreeString(value);
    std::erase_if(hex, [](wchar_t c) { return c == L'#' || c == L'"' || iswspace(c); });
    if (hex.size() == 6 && hex.find_first_not_of(L"0123456789abcdefABCDEF") == std::wstring::npos)
        return (uint32_t)wcstoul(hex.c_str(), nullptr, 16);
    COLORREF c = GetSysColor(COLOR_HIGHLIGHT);
    return (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
}

void EndFade(HWND wnd) {
    KillTimer(wnd, kFadeTimer);
    g_fades.erase(wnd);
    DestroyWindow(wnd);
}

LRESULT CALLBACK FadeProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_TIMER && wp == kFadeTimer) {
        auto it = g_fades.find(wnd);
        if (it == g_fades.end()) return 0;
        Fade& f = it->second;
        // Another window in front: out of its way four times as fast (a fifth of a second at most).
        if (GetInt(L"shell", L"marquee.quickWhenCovered")) {
            HWND front = GetForegroundWindow();
            if (front && GetAncestor(front, GA_ROOT) != g_desktop) f.step = std::max(f.step, 255.0f / 5);
        }
        f.alpha -= f.step;
        if (f.alpha <= 0) {
            EndFade(wnd);
        } else {
            BLENDFUNCTION blend{AC_SRC_OVER, 0, (BYTE)f.alpha, AC_SRC_ALPHA};
            UpdateLayeredWindow(wnd, nullptr, nullptr, nullptr, nullptr, nullptr, 0, &blend, ULW_ALPHA);
        }
        return 0;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

void FadeMarquee(RECT box) {
    int speed = MarqueeInt(L"marquee.speed", 50, 500), duration = MarqueeInt(L"marquee.duration", 100, 2000);
    int shown = std::max(16, duration * 100 / speed);  // how long it stays, in ms
    // Automatic opacity: dimmer the longer it stays (about 90 at 300 ms), as the mod worked it out.
    int fill = GetInt(L"shell", L"marquee.autoOpacity") ? std::clamp((int)std::lround(90 * std::sqrt(300.0 / shown)), 30, 150)
                                                        : MarqueeInt(L"marquee.opacity", 1, 255);
    if (GetInt(L"shell", L"marquee.oneAtATime"))
        while (!g_fades.empty()) EndFade(g_fades.begin()->first);
    static bool registered;
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = FadeProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"TourneMarqueeFade";
        registered = RegisterClassW(&wc) != 0;
    }
    int w = box.right - box.left, h = box.bottom - box.top;
    HWND wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"TourneMarqueeFade", L"",
                               WS_POPUP, box.left, box.top, w, h, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!wnd) return;
    int edge = std::max(1, (int)std::lround(GetDpiForWindow(wnd) / 96.0));
    edge = std::clamp(edge, 1, std::max(1, std::min(w, h) / 2));
    uint32_t fillColor = MarqueeColor(L"marquee.fill"), edgeColor = MarqueeColor(L"marquee.border");
    // The edge starts a little under solid, as the live box's is softer than a flat line.
    constexpr int kEdgeAlpha = 190;
    auto premultiplied = [](uint32_t rgb, int alpha) {
        return ((uint32_t)alpha << 24) | ((((rgb >> 16) & 0xFF) * alpha / 255) << 16) | ((((rgb >> 8) & 0xFF) * alpha / 255) << 8) |
               ((rgb & 0xFF) * alpha / 255);
    };
    uint32_t fillPixel = premultiplied(fillColor, fill), edgePixel = premultiplied(edgeColor, kEdgeAlpha);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC screen = GetDC(nullptr);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateDIBSection(mem, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bitmap) {
        HGDIOBJ old = SelectObject(mem, bitmap);
        auto* px = (uint32_t*)bits;
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                px[y * w + x] = x < edge || x >= w - edge || y < edge || y >= h - edge ? edgePixel : fillPixel;
        POINT origin{0, 0};
        SIZE size{w, h};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        UpdateLayeredWindow(wnd, screen, nullptr, &size, mem, &origin, 0, &blend, ULW_ALPHA);
        SelectObject(mem, old);
        DeleteObject(bitmap);
    }
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
    g_fades[wnd] = {255.0f, 255.0f / (shown / 16.0f)};
    HWND above = GetWindow(g_desktop, GW_HWNDPREV);  // (just above the desktop, under every window)
    SetWindowPos(wnd, above ? above : HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SetTimer(wnd, kFadeTimer, 16, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// The icons' hover, selection and keyboard outline in your colours (shell.iconLook = tourne). The icons' view
// draws them with the visual style's ListView item (state 2 hover, 3 picked, 5 picked while the desktop isn't
// in front, 6 both) and DrawFocusRect for the keyboard's outline (tools/desktop/theme-probe.cpp), not through
// its custom draw, which never reports the picked state here. So while the view paints (t_iconsPainting)
// those two calls are answered here, in this process only (hooks.cpp); elsewhere they pass straight on.

thread_local int t_iconsPainting = 0;
using DrawThemeBackgroundFn = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
using DrawFocusRectFn = BOOL(WINAPI*)(HDC, const RECT*);
DrawThemeBackgroundFn g_drawThemeBackground;
DrawFocusRectFn g_drawFocusRect;

struct IconLook {
    bool on = false, outline = true;
    uint32_t pickFill = 0, pickEdge = 0, hotFill = 0, hotEdge = 0, outlineColor = 0;
    int pickAlpha = 90, hotAlpha = 45, edgeAlpha = 220;
};
IconLook g_iconLook;

// A Tourne theme role (accent, text, edge...) from its record, else -1.
int64_t ThemeRole(std::wstring role) {
    if (role.empty()) return -1;
    role[0] = towupper(role[0]);
    if (role == L"Accent2" || role == L"Accent 2") role = L"Accent2";
    DWORD v = 0, size = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Theme", role.c_str(), RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS)
        return -1;
    return v & 0xFFFFFF;
}

// A colour setting: #RRGGBB, a Tourne theme role's name (accent, text, edge, line, raised, muted, good...), or empty for
// the fallback role; without a Tourne theme, Windows' selection colour.
uint32_t IconLookColor(const wchar_t* key, const wchar_t* fallbackRole) {
    const wchar_t* value = GetString(L"shell", key);
    std::wstring s = value ? value : L"";
    FreeString(value);
    std::erase_if(s, [](wchar_t c) { return c == L'#' || c == L'"' || iswspace(c); });
    if (s.size() == 6 && s.find_first_not_of(L"0123456789abcdefABCDEF") == std::wstring::npos)
        return (uint32_t)wcstoul(s.c_str(), nullptr, 16);
    for (auto& c : s) c = towlower(c);
    int64_t role = ThemeRole(s.empty() ? fallbackRole : s);
    if (role < 0 && !s.empty()) role = ThemeRole(fallbackRole);
    if (role >= 0) return (uint32_t)role;
    COLORREF c = GetSysColor(COLOR_HIGHLIGHT);
    return (GetRValue(c) << 16) | (GetGValue(c) << 8) | GetBValue(c);
}

void ReadIconLook() {
    const wchar_t* look = GetString(L"shell", L"iconLook");
    g_iconLook.on = look && !wcscmp(look, L"tourne");
    FreeString(look);
    g_iconLook.pickFill = IconLookColor(L"iconLook.pickedFill", L"accent");
    g_iconLook.pickEdge = IconLookColor(L"iconLook.pickedEdge", L"accent");
    g_iconLook.hotFill = IconLookColor(L"iconLook.hoverFill", L"text");
    g_iconLook.hotEdge = IconLookColor(L"iconLook.hoverEdge", L"edge");
    g_iconLook.outlineColor = IconLookColor(L"iconLook.outlineColor", L"text");
    g_iconLook.outline = GetInt(L"shell", L"iconLook.outline") != 0;
    g_iconLook.pickAlpha = std::clamp(GetInt(L"shell", L"iconLook.pickedOpacity"), 0, 255);
    g_iconLook.hotAlpha = std::clamp(GetInt(L"shell", L"iconLook.hoverOpacity"), 0, 255);
    g_iconLook.edgeAlpha = std::clamp(GetInt(L"shell", L"iconLook.edgeOpacity"), 0, 255);
}

// A see-through rectangle: alpha over what's there (a 1x1 premultiplied pixel, stretched).
void BlendRect(HDC dc, RECT r, uint32_t rgb, int alpha) {
    if (alpha <= 0 || r.right <= r.left || r.bottom <= r.top) return;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), 1, -1, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC mem = CreateCompatibleDC(dc);
    HBITMAP pixel = CreateDIBSection(mem, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (pixel && bits) {
        *(uint32_t*)bits = ((uint32_t)alpha << 24) | ((((rgb >> 16) & 0xFF) * alpha / 255) << 16) |
                           ((((rgb >> 8) & 0xFF) * alpha / 255) << 8) | ((rgb & 0xFF) * alpha / 255);
        HGDIOBJ old = SelectObject(mem, pixel);
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        GdiAlphaBlend(dc, r.left, r.top, r.right - r.left, r.bottom - r.top, mem, 0, 0, 1, 1, blend);
        SelectObject(mem, old);
    }
    if (pixel) DeleteObject(pixel);
    DeleteDC(mem);
}

void BlendFrame(HDC dc, RECT r, uint32_t rgb, int alpha, int width) {
    BlendRect(dc, {r.left, r.top, r.right, r.top + width}, rgb, alpha);
    BlendRect(dc, {r.left, r.bottom - width, r.right, r.bottom}, rgb, alpha);
    BlendRect(dc, {r.left, r.top + width, r.left + width, r.bottom - width}, rgb, alpha);
    BlendRect(dc, {r.right - width, r.top + width, r.right, r.bottom - width}, rgb, alpha);
}

HRESULT WINAPI IconsDrawThemeBackground(HTHEME theme, HDC dc, int part, int state, LPCRECT rect, LPCRECT clip) {
    constexpr int kListItem = 1;  // LVP_LISTITEM
    if (!t_iconsPainting || !g_iconLook.on || part != kListItem || !rect || state < 2 || state > 6 || state == 4)
        return g_drawThemeBackground(theme, dc, part, state, rect, clip);
    int save = SaveDC(dc);
    if (clip) IntersectClipRect(dc, clip->left, clip->top, clip->right, clip->bottom);
    bool picked = state != 2;
    int edge = std::max(1, (int)std::lround(GetDeviceCaps(dc, LOGPIXELSX) / 96.0));
    BlendRect(dc, *rect, picked ? g_iconLook.pickFill : g_iconLook.hotFill,
              picked ? (state == 5 ? g_iconLook.pickAlpha * 2 / 3 : g_iconLook.pickAlpha) : g_iconLook.hotAlpha);
    if (state == 6) BlendRect(dc, *rect, g_iconLook.hotFill, g_iconLook.hotAlpha / 2);  // (picked and under the pointer)
    BlendFrame(dc, *rect, picked ? g_iconLook.pickEdge : g_iconLook.hotEdge, g_iconLook.edgeAlpha, edge);
    RestoreDC(dc, save);
    return S_OK;
}

BOOL WINAPI IconsDrawFocusRect(HDC dc, const RECT* rect) {
    if (!t_iconsPainting || !g_iconLook.on || !rect) return g_drawFocusRect(dc, rect);
    if (g_iconLook.outline) {
        // A dotted line in the outline's colour, as Windows' is dotted.
        int edge = std::max(1, (int)std::lround(GetDeviceCaps(dc, LOGPIXELSX) / 96.0));
        RECT r = *rect;
        for (int x = r.left; x < r.right; x += edge * 2) {
            BlendRect(dc, {x, r.top, std::min(x + edge, (int)r.right), r.top + edge}, g_iconLook.outlineColor, 255);
            BlendRect(dc, {x, r.bottom - edge, std::min(x + edge, (int)r.right), r.bottom}, g_iconLook.outlineColor, 255);
        }
        for (int y = r.top + edge * 2; y < r.bottom - edge; y += edge * 2) {
            BlendRect(dc, {r.left, y, r.left + edge, std::min(y + edge, (int)r.bottom)}, g_iconLook.outlineColor, 255);
            BlendRect(dc, {r.right - edge, y, r.right, std::min(y + edge, (int)r.bottom)}, g_iconLook.outlineColor, 255);
        }
    }
    return TRUE;
}

// The two calls answered while the view paints (once; they pass on unchanged outside its painting).
void HookIconLook() {
    static bool done;
    if (done) return;
    done = true;
    HMODULE ux = LoadLibraryW(L"uxtheme.dll");
    void* draw = ux ? (void*)GetProcAddress(ux, "DrawThemeBackground") : nullptr;
    bool ok = SetFunctionHook(L"desktopLook", draw, (void*)IconsDrawThemeBackground, (void**)&g_drawThemeBackground) &&
              SetFunctionHook(L"desktopLook", (void*)&DrawFocusRect, (void*)IconsDrawFocusRect, (void**)&g_drawFocusRect);
    ApplyHooks();
    if (!ok) Log(L"the icons' own look couldn't be set up (Windows' look stays)");
}

void DragEnded() {
    if (!g_dragging) return;
    g_dragging = false;
    POINT to;
    GetCursorPos(&to);
    int w = std::abs(to.x - g_dragFrom.x), h = std::abs(to.y - g_dragFrom.y);
    if (w <= 10 || h <= 10) return;  // (a click, or a box too small to see fade)
    LONG left = std::min(to.x, g_dragFrom.x), top = std::min(to.y, g_dragFrom.y);
    FadeMarquee(RECT{left, top, left + w, top + h});
}

// A menu without its items' icons, as Menu No Icons does in other programs: the bitmaps the shell and its
// extensions set (Open in Terminal, New's file types, ...). Its own submenus are done as each opens.
void StripMenuIcons(HMENU menu) {
    int count = GetMenuItemCount(menu);
    for (int i = 0; i < count; i++) {
        MENUITEMINFOW info{sizeof(info)};
        info.fMask = MIIM_BITMAP | MIIM_CHECKMARKS;
        if (!GetMenuItemInfoW(menu, i, TRUE, &info)) continue;
        if (!info.hbmpItem && !info.hbmpChecked && !info.hbmpUnchecked) continue;
        MENUITEMINFOW set{sizeof(set)};
        set.fMask = MIIM_BITMAP | MIIM_CHECKMARKS;
        SetMenuItemInfoW(menu, i, TRUE, &set);
    }
}

// The desktop's icon size. Windows keeps it for Explorer's desktop (Shell\Bags\1\Desktop, IconSize), which
// shell mode never shows, so each start brought back the default (medium). Tourne keeps its own copy,
// taken from Explorer's the first time, and saves it whenever it changes.
constexpr wchar_t kIconSizeKey[] = L"Software\\Tourne\\Desktop";
constexpr UINT_PTR kTimerSaveIconSize = 0x7D01;  // on the view, after its menu has done its command

int SavedIconSize() {
    DWORD size = 0, bytes = sizeof(size);
    if (RegGetValueW(HKEY_CURRENT_USER, kIconSizeKey, L"DesktopIconSize", RRF_RT_REG_DWORD, nullptr, &size, &bytes) == ERROR_SUCCESS && size)
        return (int)size;
    bytes = sizeof(size);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\Shell\\Bags\\1\\Desktop", L"IconSize", RRF_RT_REG_DWORD,
                     nullptr, &size, &bytes) == ERROR_SUCCESS && size)
        return (int)size;
    return 0;
}

void ApplyIconSize(IShellView* view) {
    int size = SavedIconSize();
    IFolderView2* folder = nullptr;
    if (!size || !view || FAILED(view->QueryInterface(IID_PPV_ARGS(&folder)))) return;
    folder->SetViewModeAndIconSize(FVM_ICON, size);
    folder->Release();
}

void SaveIconSize(IShellView* view) {
    static int saved = 0;
    IFolderView2* folder = nullptr;
    if (!view || FAILED(view->QueryInterface(IID_PPV_ARGS(&folder)))) return;
    FOLDERVIEWMODE mode{};
    int size = 0;
    if (SUCCEEDED(folder->GetViewModeAndIconSize(&mode, &size)) && size > 0 && size != saved) {
        DWORD value = (DWORD)size;
        if (RegSetKeyValueW(HKEY_CURRENT_USER, kIconSizeKey, L"DesktopIconSize", REG_DWORD, &value, sizeof(value)) == ERROR_SUCCESS)
            saved = size;
    }
    folder->Release();
}

// The desktop's view: its right-click menu is the shell's own, shown by the view itself, so Menu No Icons
// (in Windhawk, which leaves this program alone) never reaches it. The view fills each menu as it opens,
// so the icons go after it has.
// Icons moved about the desktop. The view's own drop target, without the desktop Explorer's browser gives it,
// takes things dropped on it from elsewhere but not its own icons moved within it (it refused them: "not
// allowed"). So its drop target is wrapped: while a drag started on the icons themselves is over empty desktop,
// it's a move, done here (IFolderView::SelectAndPositionItems, each selected icon by how far the pointer went);
// anything else, and over an icon (a folder, the Recycle Bin), is the view's.
bool g_iconDrag = false;   // the drag now is the icons' own (set across the view's DoDragDrop)
POINT g_iconDragFrom{};

bool OverEmptyDesktop(POINTL pt) {
    HWND list = g_viewWnd ? FindWindowExW(g_viewWnd, nullptr, L"SysListView32", nullptr) : nullptr;
    if (!list) return false;
    LVHITTESTINFO hit{};
    hit.pt = {pt.x, pt.y};
    ScreenToClient(list, &hit.pt);
    return ListView_HitTest(list, &hit) == -1;
}

void MoveDraggedIcons(POINTL to) {
    IFolderView* folder = nullptr;
    if (!g_browser || !g_browser->view || FAILED(g_browser->view->QueryInterface(IID_PPV_ARGS(&folder)))) return;
    int dx = to.x - g_iconDragFrom.x, dy = to.y - g_iconDragFrom.y;
    IEnumIDList* items = nullptr;
    if (SUCCEEDED(folder->Items(SVGIO_SELECTION, IID_PPV_ARGS(&items))) && items) {
        std::vector<PITEMID_CHILD> pidls;
        std::vector<POINT> places;
        PITEMID_CHILD pidl;
        while (items->Next(1, &pidl, nullptr) == S_OK) {
            POINT p{};
            if (SUCCEEDED(folder->GetItemPosition(pidl, &p))) {
                pidls.push_back(pidl);
                places.push_back({p.x + dx, p.y + dy});
            } else {
                CoTaskMemFree(pidl);
            }
        }
        items->Release();
        if (!pidls.empty())
            folder->SelectAndPositionItems((UINT)pidls.size(), (PCUITEMID_CHILD_ARRAY)pidls.data(), places.data(), SVSI_POSITIONITEM);
        for (auto p : pidls) CoTaskMemFree(p);
    }
    folder->Release();
}

class IconDropTarget : public IDropTarget {
   public:
    explicit IconDropTarget(IDropTarget* inner) : inner_(inner) { inner_->AddRef(); }
    ~IconDropTarget() { inner_->Release(); }
    STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget)) {
            *out = static_cast<IDropTarget*>(this);
            AddRef();
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return ++refs_; }
    STDMETHODIMP_(ULONG) Release() override {
        ULONG left = --refs_;
        if (!left) delete this;
        return left;
    }
    STDMETHODIMP DragEnter(IDataObject* data, DWORD keys, POINTL pt, DWORD* effect) override {
        DWORD allowed = *effect;
        HRESULT r = inner_->DragEnter(data, keys, pt, effect);
        if (Moving(pt)) *effect = allowed & DROPEFFECT_MOVE ? DROPEFFECT_MOVE : *effect;
        return r;
    }
    STDMETHODIMP DragOver(DWORD keys, POINTL pt, DWORD* effect) override {
        DWORD allowed = *effect;
        HRESULT r = inner_->DragOver(keys, pt, effect);
        if (Moving(pt)) *effect = allowed & DROPEFFECT_MOVE ? DROPEFFECT_MOVE : *effect;
        return r;
    }
    STDMETHODIMP DragLeave() override { return inner_->DragLeave(); }
    STDMETHODIMP Drop(IDataObject* data, DWORD keys, POINTL pt, DWORD* effect) override {
        if (Moving(pt)) {
            inner_->DragLeave();
            MoveDraggedIcons(pt);
            *effect = DROPEFFECT_NONE;  // (placed, not moved on disk: the source mustn't delete anything)
            return S_OK;
        }
        return inner_->Drop(data, keys, pt, effect);
    }

   private:
    // The icons' own drag, over empty desktop, without Ctrl (a copy) or Alt (a shortcut) held.
    bool Moving(POINTL pt) const {
        return g_iconDrag && OverEmptyDesktop(pt) && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000);
    }
    IDropTarget* inner_;
    ULONG refs_ = 1;
};

LRESULT CALLBACK ViewProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR) {
    // The view starts its drag (and runs it to the drop) inside its handling of the icons' LVN_BEGINDRAG.
    if (msg == WM_NOTIFY && (((NMHDR*)lp)->code == LVN_BEGINDRAG || ((NMHDR*)lp)->code == LVN_BEGINRDRAG)) {
        GetCursorPos(&g_iconDragFrom);
        g_iconDrag = true;
        LRESULT result = DefSubclassProc(wnd, msg, wp, lp);
        g_iconDrag = false;
        return result;
    }
    if (msg == WM_INITMENUPOPUP) {
        LRESULT result = DefSubclassProc(wnd, msg, wp, lp);
        if (GetInt(L"menuNoIcons", L"menuNoIcons.enabled")) StripMenuIcons((HMENU)wp);
        return result;
    }
    // Its menu's command (View > Small icons, ...) runs after the menu closes: the size is saved just after.
    if (msg == WM_EXITMENULOOP) SetTimer(wnd, kTimerSaveIconSize, 500, nullptr);
    if (msg == WM_TIMER && wp == kTimerSaveIconSize) {
        KillTimer(wnd, kTimerSaveIconSize);
        if (g_browser) SaveIconSize(g_browser->view);
        return 0;
    }
    if (msg == WM_NCDESTROY) RemoveWindowSubclass(wnd, ViewProc, 0);
    return DefSubclassProc(wnd, msg, wp, lp);
}

// The Tourne Menu (the taskbar's tourne-menu.cpp, a full LiteMenu replacement) on the desktop's empty space:
// the click goes to a bar, which owns the menu and its look. Its button is the taskbar's menu.button.
std::wstring ForwardTourneMenuButton() {
    const wchar_t* set = GetString(L"taskbar", L"menu.button");
    std::wstring chosen = set && *set ? set : L"middle";
    delete[] set;
    return chosen;
}

// A bar of Tourne's taskbar (any: it only owns the menus), in this process.
HWND FindTourneBar() {
    struct Find {
        DWORD pid;
        HWND bar;
    } find{GetCurrentProcessId(), nullptr};
    EnumWindows(
        [](HWND h, LPARAM param) -> BOOL {
            auto* f = (Find*)param;
            DWORD pid = 0;
            GetWindowThreadProcessId(h, &pid);
            WCHAR cls[64] = L"";
            GetClassNameW(h, cls, ARRAYSIZE(cls));
            if (pid != f->pid || wcsncmp(cls, L"TourneTaskbar", 13) != 0) return TRUE;
            f->bar = h;
            return FALSE;
        },
        (LPARAM)&find);
    return find.bar;
}

bool ForwardTourneMenu(const wchar_t* button) {
    if (!GetInt(L"taskbar", L"menu.enabled") || ForwardTourneMenuButton() != button) return false;
    HWND bar = FindTourneBar();
    if (!bar) return false;
    PostMessageW(bar, RegisterWindowMessageW(L"TourneDesktop.ShowMenu"), 0, 0);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Choosing the wallpaper (the desktop's menu, Wallpaper): a picture, the next one in its folder (or
// shell.wallpaper.folder), one of the last few, and how it's placed; with shell.wallpaper.minutes, the next one
// on its own every so often (shell.wallpaper.shuffle: any of them). Set as Windows sets it (IDesktopWallpaper,
// else SystemParametersInfo), so Settings and the lock screen agree; on a thread of its own, as a big picture
// takes Windows a moment to copy. Idle cost: none (the slideshow's timer, when on).

constexpr wchar_t kRecentWallpapers[] = L"RecentWallpapers";
constexpr wchar_t kChosenWallpaper[] = L"Wallpaper";  // the picture as chosen here (Windows keeps a copy of it)
constexpr UINT kWallpaperSet = WM_APP + 70;            // posted to the desktop once set (lParam: when it began)
constexpr UINT kWallpaperSettings = WM_APP + 71;       // the settings changed: the slideshow again
constexpr UINT_PTR kTimerSlideshow = 70;
constexpr size_t kRecentMax = 8;
const wchar_t* const kPictureTypes[] = {L".jpg", L".jpeg", L".jfif", L".png", L".bmp", L".gif", L".tif", L".tiff", L".webp", L".heic", L".avif", L".jxr"};

std::wstring RegText(HKEY root, const wchar_t* key, const wchar_t* name) {
    wchar_t value[MAX_PATH * 2]{};
    DWORD size = sizeof(value);
    return RegGetValueW(root, key, name, RRF_RT_REG_SZ, nullptr, value, &size) == ERROR_SUCCESS ? value : L"";
}

bool IsPicture(std::wstring const& path) {
    const wchar_t* ext = PathFindExtensionW(path.c_str());
    for (auto type : kPictureTypes)
        if (!_wcsicmp(ext, type)) return true;
    return false;
}

// The picture on the desktop, as chosen (not Windows' copy of it).
std::wstring CurrentWallpaper() {
    std::wstring path = DesktopWallpaperPath();
    if (path.empty()) path = RegText(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper");
    if (path.empty() || StrStrIW(path.c_str(), L"\\Themes\\TranscodedWallpaper") || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
        path = RegText(HKEY_CURRENT_USER, kIconSizeKey, kChosenWallpaper);
    return path;
}

// How it's placed (DESKTOP_WALLPAPER_POSITION), from Windows' WallpaperStyle and TileWallpaper.
int CurrentPlacement() {
    int style = _wtoi(RegText(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallpaperStyle").c_str());
    if (_wtoi(RegText(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"TileWallpaper").c_str())) return DWPOS_TILE;
    switch (style) {
        case 0: return DWPOS_CENTER;
        case 2: return DWPOS_STRETCH;
        case 6: return DWPOS_FIT;
        case 22: return DWPOS_SPAN;
        default: return DWPOS_FILL;
    }
}

std::vector<std::wstring> RecentWallpapers() {
    std::vector<std::wstring> out;
    DWORD bytes = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, kIconSizeKey, kRecentWallpapers, RRF_RT_REG_MULTI_SZ, nullptr, nullptr, &bytes) != ERROR_SUCCESS || !bytes)
        return out;
    std::vector<wchar_t> list(bytes / sizeof(wchar_t) + 2, 0);
    bytes = (DWORD)(list.size() * sizeof(wchar_t));
    if (RegGetValueW(HKEY_CURRENT_USER, kIconSizeKey, kRecentWallpapers, RRF_RT_REG_MULTI_SZ, nullptr, list.data(), &bytes) != ERROR_SUCCESS)
        return out;
    for (const wchar_t* p = list.data(); *p; p += wcslen(p) + 1) out.push_back(p);
    return out;
}

// The last few, newest first, without the one showing (as the menu lists them).
std::vector<std::wstring> OtherRecentWallpapers() {
    std::wstring now = CurrentWallpaper();
    std::vector<std::wstring> out;
    for (auto const& path : RecentWallpapers())
        if (_wcsicmp(path.c_str(), now.c_str()) && GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) out.push_back(path);
    return out;
}

void RememberWallpaper(std::wstring const& path) {
    std::vector<std::wstring> list{path};
    for (auto const& other : RecentWallpapers())
        if (_wcsicmp(other.c_str(), path.c_str()) && list.size() < kRecentMax) list.push_back(other);
    std::wstring multi;
    for (auto const& item : list) multi += item + L'\0';
    multi += L'\0';
    RegSetKeyValueW(HKEY_CURRENT_USER, kIconSizeKey, kRecentWallpapers, REG_MULTI_SZ, multi.data(), (DWORD)(multi.size() * sizeof(wchar_t)));
    RegSetKeyValueW(HKEY_CURRENT_USER, kIconSizeKey, kChosenWallpaper, REG_SZ, path.c_str(), (DWORD)((path.size() + 1) * sizeof(wchar_t)));
}

// The pictures in the slideshow's folder (shell.wallpaper.folder; empty: the one showing's), by name.
std::vector<std::wstring> FolderPictures() {
    std::wstring folder;
    if (const wchar_t* set = GetString(L"shell", L"wallpaper.folder")) {
        wchar_t expanded[MAX_PATH * 2]{};
        if (*set && ExpandEnvironmentStringsW(set, expanded, ARRAYSIZE(expanded))) folder = expanded;
        FreeString(set);
    }
    if (folder.empty()) {
        folder = CurrentWallpaper();
        folder = folder.substr(0, folder.find_last_of(L'\\'));
    }
    while (!folder.empty() && folder.back() == L'\\') folder.pop_back();
    std::vector<std::wstring> out;
    if (folder.empty()) return out;
    WIN32_FIND_DATAW found;
    HANDLE find = FindFirstFileExW((folder + L"\\*").c_str(), FindExInfoBasic, &found, FindExSearchNameMatch, nullptr, 0);
    if (find == INVALID_HANDLE_VALUE) return out;
    do {
        if (!(found.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)) && IsPicture(found.cFileName))
            out.push_back(folder + L"\\" + found.cFileName);
    } while (FindNextFileW(find, &found));
    FindClose(find);
    std::sort(out.begin(), out.end(), [](auto const& a, auto const& b) { return StrCmpLogicalW(a.c_str(), b.c_str()) < 0; });
    return out;
}

// The picture after the one showing (or any other, shuffled); empty when there's no other.
std::wstring NextWallpaper(bool shuffle) {
    std::vector<std::wstring> pictures = FolderPictures();
    std::wstring now = CurrentWallpaper();
    size_t at = pictures.size();
    for (size_t i = 0; i < pictures.size(); i++)
        if (!_wcsicmp(pictures[i].c_str(), now.c_str())) at = i;
    if (pictures.empty() || (pictures.size() == 1 && at == 0)) return L"";
    if (shuffle) {
        size_t pick = (size_t)(GetTickCount64() * 2654435761ull >> 7) % pictures.size();
        if (pick == at) pick = (pick + 1) % pictures.size();
        return pictures[pick];
    }
    return pictures[at >= pictures.size() ? 0 : (at + 1) % pictures.size()];
}

// A picture from a file dialog, starting where the one showing is.
std::wstring PickWallpaper() {
    std::wstring out;
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) return out;
    std::wstring types;
    for (auto type : kPictureTypes) types += std::wstring(types.empty() ? L"*" : L";*") + type;
    COMDLG_FILTERSPEC filters[] = {{L"Pictures", types.c_str()}, {L"All files", L"*.*"}};
    dialog->SetFileTypes(ARRAYSIZE(filters), filters);
    dialog->SetTitle(L"Choose a wallpaper");
    dialog->SetOkButtonLabel(L"Choose picture");
    DWORD options = 0;
    dialog->GetOptions(&options);
    dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
    std::wstring now = CurrentWallpaper();
    if (!now.empty()) {
        std::wstring folder = now.substr(0, now.find_last_of(L'\\'));
        IShellItem* item = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(folder.c_str(), nullptr, IID_PPV_ARGS(&item)))) {
            dialog->SetFolder(item);
            item->Release();
        }
    }
    IShellItem* result = nullptr;
    if (SUCCEEDED(dialog->Show(nullptr)) && SUCCEEDED(dialog->GetResult(&result))) {
        PWSTR path = nullptr;
        if (SUCCEEDED(result->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path) out = path;
        CoTaskMemFree(path);
        result->Release();
    }
    dialog->Release();
    return out;
}

std::atomic<int> g_wallJobs{0};  // pictures being set: Windows' change notices meanwhile are ours, drawn when each is done

struct WallpaperJob {
    enum Kind { kPick, kNext, kPicture, kPlace } kind;
    std::wstring path;  // kPicture
    int placement = -1;  // kPlace
    bool shuffle = false;  // kNext
};

DWORD WINAPI WallpaperThread(LPVOID param) {
    std::unique_ptr<WallpaperJob> job((WallpaperJob*)param);
    DWORD began = GetTickCount();
    HRESULT co = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    std::wstring path = job->kind == WallpaperJob::kPick ? PickWallpaper()
                        : job->kind == WallpaperJob::kNext ? NextWallpaper(job->shuffle)
                                                           : job->path;
    bool wanted = job->kind == WallpaperJob::kPlace || !path.empty();
    bool set = false;
    if (wanted) {
        IDesktopWallpaper* dw = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) {
            set = job->kind == WallpaperJob::kPlace ? SUCCEEDED(dw->SetPosition((DESKTOP_WALLPAPER_POSITION)job->placement))
                                                     : SUCCEEDED(dw->SetWallpaper(nullptr, path.c_str()));
            dw->Release();
        }
        if (!set) {
            // Without it: Windows' older way, the placement in the registry first.
            if (job->kind == WallpaperJob::kPlace) {
                int p = job->placement;
                const wchar_t* style = p == DWPOS_CENTER || p == DWPOS_TILE ? L"0" : p == DWPOS_STRETCH ? L"2" : p == DWPOS_FIT ? L"6" : p == DWPOS_SPAN ? L"22" : L"10";
                const wchar_t* tile = p == DWPOS_TILE ? L"1" : L"0";
                RegSetKeyValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallpaperStyle", REG_SZ, style, (DWORD)((wcslen(style) + 1) * sizeof(wchar_t)));
                RegSetKeyValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"TileWallpaper", REG_SZ, tile, (DWORD)((wcslen(tile) + 1) * sizeof(wchar_t)));
                path = CurrentWallpaper();
            }
            set = !path.empty() && SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        }
        if (set && job->kind != WallpaperJob::kPlace) RememberWallpaper(path);
        Log(L"wallpaper %s: %s", job->kind == WallpaperJob::kPlace ? L"placement" : path.c_str(), set ? L"set" : L"couldn't be set");
    }
    if (SUCCEEDED(co)) CoUninitialize();
    g_wallJobs--;
    // (the picture itself, so the desktop draws exactly it; a placement change reads Windows back)
    auto* drawn = new std::wstring(set && job->kind != WallpaperJob::kPlace ? path : L"");
    if (!g_desktop || !PostMessageW(g_desktop, kWallpaperSet, set, (LPARAM)drawn)) delete drawn;
    (void)began;
    return 0;
}

void StartWallpaperJob(WallpaperJob* job) {
    g_wallJobs++;
    if (HANDLE thread = CreateThread(nullptr, 0, WallpaperThread, job, 0, nullptr)) CloseHandle(thread);
    else delete job, g_wallJobs--;
}

// The slideshow's timer: on with shell.wallpaper.minutes, from now.
void ArmSlideshow() {
    if (!g_desktop) return;
    int minutes = GetInt(L"shell", L"wallpaper.minutes");
    if (minutes > 0) SetTimer(g_desktop, kTimerSlideshow, (UINT)std::min(minutes, 24 * 60) * 60000u, nullptr);
    else KillTimer(g_desktop, kTimerSlideshow);
}

////////////////////////////////////////////////////////////////////////////////
// The desktop's right-click menu in Tourne's look (shell.desktopMenu): the bar shows it (the taskbar's
// desktop-menu.cpp) from what's posted here, and sends back what changes the view. Same numbers there.

enum : int {
    kDvLarge = 1, kDvMedium, kDvSmall,
    kDvAutoArrange = 10, kDvGrid, kDvShowIcons,
    kDvSortName = 20, kDvSortSize, kDvSortType, kDvSortDate,
    kDvRefresh = 30, kDvRename = 40,
    kDvWallChoose = 60, kDvWallNext, kDvWallRecent = 70, kDvWallPlace = 80,  // (recent: + its place; place: + the position)
};

// The sort keys (System.ItemNameDisplay, Size, ItemTypeText, DateModified).
constexpr GUID kStorageProps = {0xB725F130, 0x47EF, 0x101A, {0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC}};
constexpr PROPERTYKEY kSortName{kStorageProps, 10}, kSortSize{kStorageProps, 12}, kSortType{kStorageProps, 4}, kSortDate{kStorageProps, 14};

UINT DesktopViewMsg() {
    static UINT msg = RegisterWindowMessageW(L"TourneDesktop.DesktopView");
    return msg;
}

HWND IconsList() {
    return g_viewWnd ? FindWindowExW(g_viewWnd, nullptr, L"SysListView32", nullptr) : nullptr;
}

// The view's state and what was clicked, as "key=value" lines, posted to a bar; false when there's none.
// prepare: the button only went down, so the bar builds the menu ahead (taskbar.desktopMenuReady).
bool PostDesktopMenu(bool onItems, bool shift, bool prepare = false) {
    HWND bar = FindTourneBar();
    IFolderView2* folder = nullptr;
    if (!bar || !g_browser || !g_browser->view || FAILED(g_browser->view->QueryInterface(IID_PPV_ARGS(&folder)))) return false;
    std::wstring text;
    FOLDERVIEWMODE mode{};
    int size = 48;
    folder->GetViewModeAndIconSize(&mode, &size);
    DWORD flags = 0;
    folder->GetCurrentFolderFlags(&flags);
    SORTCOLUMN column{};
    std::wstring sort = L"name";
    if (SUCCEEDED(folder->GetSortColumns(&column, 1)) && column.propkey.fmtid == kStorageProps)
        sort = column.propkey.pid == 12 ? L"size" : column.propkey.pid == 4 ? L"type" : column.propkey.pid == 14 ? L"date" : L"name";
    HWND list = IconsList();
    text += L"iconSize=" + std::to_wstring(size) + L"\n";
    text += std::wstring(L"autoArrange=") + ((flags & FWF_AUTOARRANGE) ? L"1" : L"0") + L"\n";
    text += std::wstring(L"grid=") + ((flags & FWF_SNAPTOGRID) ? L"1" : L"0") + L"\n";
    text += std::wstring(L"icons=") + (list && IsWindowVisible(list) ? L"1" : L"0") + L"\n";
    text += std::wstring(L"shift=") + (shift ? L"1" : L"0") + L"\n";
    text += L"sort=" + sort + L"\n";
    if (prepare) text += L"prepare=1\n";
    if (!onItems && !prepare) {
        text += L"wallpaper=" + CurrentWallpaper() + L"\n";
        text += L"placement=" + std::to_wstring(CurrentPlacement()) + L"\n";
        for (auto const& path : OtherRecentWallpapers()) text += L"recent=" + path + L"\n";
    }
    if (onItems) {
        IShellItemArray* selection = nullptr;
        if (SUCCEEDED(folder->GetSelection(FALSE, &selection)) && selection) {
            DWORD count = 0;
            selection->GetCount(&count);
            for (DWORD i = 0; i < count; i++) {
                IShellItem* item = nullptr;
                if (FAILED(selection->GetItemAt(i, &item))) continue;
                LPWSTR name = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &name)) && name) text += L"item=" + std::wstring(name) + L"\n";
                CoTaskMemFree(name);
                item->Release();
            }
            selection->Release();
        }
    }
    folder->Release();
    auto* request = new std::wstring(text);
    if (!PostMessageW(bar, RegisterWindowMessageW(L"TourneDesktop.DesktopMenu"), (WPARAM)g_desktop, (LPARAM)request)) {
        delete request;
        return false;
    }
    return true;
}

constexpr wchar_t kHideIconsValue[] = L"HideDesktopIcons";

void DesktopViewCommand(int command) {
    // The wallpaper's, which need no view.
    if (command == kDvWallChoose || command == kDvWallNext) {
        StartWallpaperJob(new WallpaperJob{command == kDvWallChoose ? WallpaperJob::kPick : WallpaperJob::kNext, L"", -1,
                                           GetInt(L"shell", L"wallpaper.shuffle") != 0});
        return;
    }
    if (command >= kDvWallRecent && command < kDvWallRecent + (int)kRecentMax) {
        auto recent = OtherRecentWallpapers();
        if (command - kDvWallRecent < (int)recent.size())
            StartWallpaperJob(new WallpaperJob{WallpaperJob::kPicture, recent[command - kDvWallRecent]});
        return;
    }
    if (command >= kDvWallPlace + (int)DWPOS_CENTER && command <= kDvWallPlace + (int)DWPOS_SPAN) {
        StartWallpaperJob(new WallpaperJob{WallpaperJob::kPlace, L"", command - kDvWallPlace});
        return;
    }
    IFolderView2* folder = nullptr;
    if (!g_browser || !g_browser->view || FAILED(g_browser->view->QueryInterface(IID_PPV_ARGS(&folder)))) return;
    DWORD flags = 0;
    folder->GetCurrentFolderFlags(&flags);
    auto sortBy = [&](PROPERTYKEY const& key) {
        SORTCOLUMN column{key, SORT_ASCENDING};
        folder->SetSortColumns(&column, 1);
    };
    switch (command) {
        case kDvLarge: folder->SetViewModeAndIconSize(FVM_ICON, 96); break;
        case kDvMedium: folder->SetViewModeAndIconSize(FVM_ICON, 48); break;
        case kDvSmall: folder->SetViewModeAndIconSize(FVM_ICON, 32); break;
        case kDvAutoArrange: folder->SetCurrentFolderFlags(FWF_AUTOARRANGE, flags ^ FWF_AUTOARRANGE); break;
        case kDvGrid: folder->SetCurrentFolderFlags(FWF_SNAPTOGRID, flags ^ FWF_SNAPTOGRID); break;
        case kDvShowIcons:
            if (HWND list = IconsList()) {
                bool show = !IsWindowVisible(list);
                ShowWindow(list, show ? SW_SHOW : SW_HIDE);
                DWORD hide = show ? 0 : 1;
                RegSetKeyValueW(HKEY_CURRENT_USER, kIconSizeKey, kHideIconsValue, REG_DWORD, &hide, sizeof(hide));
            }
            break;
        case kDvSortName: sortBy(kSortName); break;
        case kDvSortSize: sortBy(kSortSize); break;
        case kDvSortType: sortBy(kSortType); break;
        case kDvSortDate: sortBy(kSortDate); break;
        case kDvRefresh: g_browser->view->Refresh(); break;
        case kDvRename: {
            int focused = -1;
            if (SUCCEEDED(folder->GetFocusedItem(&focused)) && focused >= 0)
                folder->SelectItem(focused, SVSI_EDIT | SVSI_SELECT | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE | SVSI_FOCUSED);
            break;
        }
    }
    if (command >= kDvLarge && command <= kDvSmall) SaveIconSize(g_browser->view);
    folder->Release();
}

LRESULT CALLBACK IconsProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR) {
    switch (msg) {
        case WM_PAINT:
        case WM_PRINTCLIENT: {
            t_iconsPainting++;  // (the hover, picked and keyboard looks are ours meanwhile: IconsDrawThemeBackground)
            LRESULT result = DefSubclassProc(wnd, msg, wp, lp);
            t_iconsPainting--;
            return result;
        }
        case WM_MOUSEWHEEL: {
            // Ctrl + the wheel resizes the icons: kept for the next start.
            if (!(GET_KEYSTATE_WPARAM(wp) & MK_CONTROL)) break;
            LRESULT result = DefSubclassProc(wnd, msg, wp, lp);
            if (g_browser) SaveIconSize(g_browser->view);
            return result;
        }
        case WM_MBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            // The Tourne Menu's button on the empty space; else, with the right button, the desktop's own menu in
            // Tourne's look. The press and the release are both taken, so Windows' menu doesn't follow;
            // Shift + right-click still gives it.
            bool right = msg != WM_MBUTTONUP;
            if (right && (GetKeyState(VK_SHIFT) & 0x8000)) break;
            LVHITTESTINFO hit{};
            hit.pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            int item = ListView_HitTest(wnd, &hit);
            bool tourneMenu = GetInt(L"taskbar", L"menu.enabled") && ForwardTourneMenuButton() == (right ? L"right" : L"middle");
            if (item == -1 && tourneMenu) {
                if (msg != WM_RBUTTONDOWN) ForwardTourneMenu(right ? L"right" : L"middle");
                return 0;
            }
            if (!right || !GetInt(L"shell", L"desktopMenu") || !FindTourneBar()) break;
            if (msg == WM_RBUTTONDOWN) {
                // As Explorer's desktop: a right-click on an icon not picked picks it alone.
                if (item != -1 && !(ListView_GetItemState(wnd, item, LVIS_SELECTED) & LVIS_SELECTED)) {
                    ListView_SetItemState(wnd, -1, 0, LVIS_SELECTED);
                    ListView_SetItemState(wnd, item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                }
                if (item == -1) ListView_SetItemState(wnd, -1, 0, LVIS_SELECTED);
                // The bar starts on the menu now, so it's ready as the button comes up.
                if (GetInt(L"taskbar", L"desktopMenuReady")) PostDesktopMenu(item != -1, false, true);
                return 0;
            }
            PostDesktopMenu(item != -1, false);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (!GetInt(L"shell", L"marquee.fade")) break;
            LVHITTESTINFO hit{};
            hit.pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            if (ListView_HitTest(wnd, &hit) != -1) break;  // (on an icon: that's moving it, not a box)
            g_dragFrom = hit.pt;
            ClientToScreen(wnd, &g_dragFrom);
            g_dragging = true;
            LRESULT result = DefSubclassProc(wnd, msg, wp, lp);
            // The view's own box runs until the button is let go, inside the line above.
            if (!(GetAsyncKeyState(GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON) & 0x8000)) DragEnded();
            return result;
        }
        case WM_LBUTTONUP: {
            LRESULT result = DefSubclassProc(wnd, msg, wp, lp);
            DragEnded();
            return result;
        }
        case WM_NCDESTROY:
            RemoveWindowSubclass(wnd, IconsProc, 0);
            break;
    }
    return DefSubclassProc(wnd, msg, wp, lp);
}

// The desktop's icons: Windows' own view of the Desktop folder.
void ShowIcons() {
    IShellFolder* desktop = nullptr;
    if (FAILED(SHGetDesktopFolder(&desktop))) return;
    IShellView* view = nullptr;
    HRESULT made = desktop->CreateViewObject(g_desktop, IID_IShellView, (void**)&view);
    desktop->Release();
    if (FAILED(made) || !view) {
        Log(L"no view of the desktop's icons (0x%08lx)", made);
        return;
    }
    g_browser = new Browser(g_desktop);
    // (and on the view itself, where it takes them: one of the two is where Windows looks)
    IFolderViewOptions* options = nullptr;
    if (SUCCEEDED(view->QueryInterface(IID_PPV_ARGS(&options)))) {
        options->SetFolderViewOptions(FVO_CUSTOMPOSITION, FVO_CUSTOMPOSITION);
        options->Release();
    }
    FOLDERSETTINGS settings{FVM_ICON, FWF_DESKTOP | FWF_TRANSPARENT | FWF_NOCLIENTEDGE | FWF_NOSCROLL | FWF_BESTFITWINDOW | FWF_SNAPTOGRID};
    RECT rc;
    GetClientRect(g_desktop, &rc);
    HRESULT shown = view->CreateViewWindow(nullptr, &settings, g_browser, &rc, &g_viewWnd);
    if (FAILED(shown) || !g_viewWnd) {
        Log(L"the desktop's icons couldn't be shown (0x%08lx)", shown);
        view->Release();
        g_browser->Release();
        g_browser = nullptr;
        g_viewWnd = nullptr;
        return;
    }
    g_browser->view = view;
    // Drops: a view made with FWF_DESKTOP leaves registering its drop target to the desktop that holds it (Explorer's
    // does it on its own window), so without this every drag onto the desktop, icons moved about included, met
    // nothing and showed "not allowed". The view is its own drop target; OLE finds it from the icons up.
    IDropTarget* drop = nullptr;
    if (SUCCEEDED(view->QueryInterface(IID_PPV_ARGS(&drop))) || SUCCEEDED(view->GetItemObject(SVGIO_BACKGROUND, IID_PPV_ARGS(&drop)))) {
        auto* wrapped = new IconDropTarget(drop);
        HRESULT r = RegisterDragDrop(g_viewWnd, wrapped);
        wrapped->Release();
        if (FAILED(r) && r != DRAGDROP_E_ALREADYREGISTERED) Log(L"the icons won't take drops (0x%08lx)", r);
        drop->Release();
    } else {
        Log(L"the icons' view has no drop target");
    }
    view->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
    ApplyIconSize(view);
    IconAreas();
    SetWindowSubclass(g_viewWnd, ViewProc, 0, 0);
    ReadIconLook();
    HookIconLook();
    if (HWND list = FindWindowExW(g_viewWnd, nullptr, L"SysListView32", nullptr)) {
        SetWindowSubclass(list, IconsProc, 0, 0);
        // "Show desktop icons" off in the desktop's menu: kept for the next start.
        DWORD hide = 0, bytes = sizeof(hide);
        if (RegGetValueW(HKEY_CURRENT_USER, kIconSizeKey, kHideIconsValue, RRF_RT_REG_DWORD, nullptr, &hide, &bytes) == ERROR_SUCCESS && hide)
            ShowWindow(list, SW_HIDE);
    }
}

LRESULT CALLBACK DesktopProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    // A new Tourne theme: the icons' looks that follow it.
    static UINT themeChanged = RegisterWindowMessageW(L"TourneThemeChanged");
    if (msg == themeChanged && msg) {
        ReadIconLook();
        if (HWND list = IconsList()) InvalidateRect(list, nullptr, TRUE);
    }
    // A command from the desktop's menu, which the bar shows (View, Sort by, Refresh, Rename).
    // A recent wallpaper picked in the bar's desktop menu, by its path (a place in the list could have moved on).
    if (msg == WM_COPYDATA) {
        auto* cd = (COPYDATASTRUCT*)lp;
        if (cd && cd->dwData == 0x54445750 /* TDWP */ && cd->cbData >= sizeof(wchar_t)) {
            std::wstring path((const wchar_t*)cd->lpData, cd->cbData / sizeof(wchar_t));
            while (!path.empty() && !path.back()) path.pop_back();
            if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) StartWallpaperJob(new WallpaperJob{WallpaperJob::kPicture, path});
            return TRUE;
        }
    }
    if (msg == DesktopViewMsg() && msg) {
        DesktopViewCommand((int)wp);
        return 0;
    }
    switch (msg) {
        case WM_USER + 7:  // WM_GETISHELLBROWSER: shell code asks a view's window for its browser; Explorer's windows answer
            return (LRESULT)static_cast<IShellBrowser*>(g_browser);
        case kWallpaperSet:
            if (std::unique_ptr<std::wstring> chosen{(std::wstring*)lp}; wp) {
                LoadWallpaper(false, *chosen);
                InvalidateRect(wnd, nullptr, TRUE);
                if (g_viewWnd) RedrawWindow(g_viewWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            }
            ArmSlideshow();  // (the next one a full turn from this one)
            return 0;
        case kWallpaperSettings:
            ArmSlideshow();
            ReadIconLook();
            if (HWND list = IconsList()) InvalidateRect(list, nullptr, TRUE);
            return 0;
        case WM_TIMER:
            if (wp == kTimerSlideshow) {
                StartWallpaperJob(new WallpaperJob{WallpaperJob::kNext, L"", -1, GetInt(L"shell", L"wallpaper.shuffle") != 0});
                return 0;
            }
            break;
        case WM_ERASEBKGND:
            PaintWallpaper(wnd, (HDC)wp);
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(wnd, &ps);
            PaintWallpaper(wnd, dc);
            EndPaint(wnd, &ps);
            return 0;
        }
        case WM_WINDOWPOSCHANGING: {
            auto* pos = (WINDOWPOS*)lp;  // always under every other window
            pos->hwndInsertAfter = HWND_BOTTOM;
            pos->flags &= ~SWP_NOZORDER;
            break;
        }
        case WM_SIZE:
            if (g_viewWnd) MoveWindow(g_viewWnd, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
            return 0;
        case WM_DISPLAYCHANGE:
            Fit();
            IconAreas();
            LoadWallpaper(true);
            InvalidateRect(wnd, nullptr, TRUE);
            return 0;
        case WM_SETTINGCHANGE:
            if (wp == SPI_SETDESKWALLPAPER && !g_wallJobs) LoadWallpaper();
            if (wp == SPI_SETWORKAREA) IconAreas();
            if (wp == SPI_SETDESKWALLPAPER || wp == SPI_SETWORKAREA) {
                InvalidateRect(wnd, nullptr, TRUE);
                if (g_viewWnd) RedrawWindow(g_viewWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
            }
            break;
        case WM_CLOSE:  // (Alt+F4 on the desktop: it stays)
            return 0;
        case WM_INITMENU:
        case WM_INITMENUPOPUP:
        case WM_MEASUREITEM:
        case WM_DRAWITEM:
        case WM_MENUCHAR:
        case WM_MENUSELECT:
        case WM_EXITMENULOOP:
            // The view's right-click menu is owned by this window (the view's browser): its messages go on to
            // the view, as Explorer's desktop window passes them, so its own items draw (New, Send to) and
            // ViewProc sees the menu.
            if (g_viewWnd) return SendMessageW(g_viewWnd, msg, wp, lp);
            break;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

}  // namespace

void DesktopHostStart() {
    if (g_desktop || !GetInt(L"shell", L"desktop")) return;
    // (TOURNE_TEST_TRAY: made beside another, for testing under Wine)
    // (with Explorer underneath, its desktop is hidden and this one shows instead)
    // (and while it's still coming up: a slow start after a restart)
    if (FindWindowW(L"Progman", nullptr) && !ShellExplorerExpected() && !GetEnvironmentVariableW(L"TOURNE_TEST_TRAY", nullptr, 0)) {
        Log(L"there's a desktop already (Explorer's?): not making one");
        return;
    }
    g_ole = SUCCEEDED(OleInitialize(nullptr));  // (dragging to and from the icons)
    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = DesktopProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"Progman";
    RegisterClassW(&wc);
    LoadWallpaper(true);
    RECT rc = AllScreens();
    g_desktop = CreateWindowExW(WS_EX_TOOLWINDOW, L"Progman", L"Program Manager", WS_POPUP | WS_CLIPCHILDREN, rc.left, rc.top,
                                rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, instance, nullptr);
    if (!g_desktop) {
        Log(L"couldn't make the desktop window (error %lu)", GetLastError());
        return;
    }
    ShowIcons();
    ArmSlideshow();
    // Kept in view when a taskbar preview peeks at a window: DWM fades every other window
    // then, and only Explorer's desktop is spared on its own (else the wallpaper goes grey).
    BOOL keep = TRUE;
    DwmSetWindowAttribute(g_desktop, DWMWA_EXCLUDED_FROM_PEEK, &keep, sizeof(keep));
    // (Explorer still coming up underneath: it is told it's the shell only if no shell window
    // is there yet, so this one waits; shell-mode.cpp claims it if Explorer never comes.)
    if (!ShellExplorerExpected()) DesktopHostClaimShell();
    SetWindowPos(g_desktop, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    Log(g_viewWnd ? L"showing the desktop, with its icons" : L"showing the desktop");
}

void DesktopHostSettingsChanged() {
    if (g_desktop) PostMessageW(g_desktop, kWallpaperSettings, 0, 0);
}

// Tell Windows this is the shell's desktop window (GetShellWindow), as Explorer's
// is: apps and Windows itself look for it. Only works while no other shell has one.
void DesktopHostClaimShell() {
    if (!g_desktop || GetShellWindow() || GetEnvironmentVariableW(L"TOURNE_TEST_TRAY", nullptr, 0)) return;
    using SetShellWindowFn = BOOL(WINAPI*)(HWND);
    auto setShell = (SetShellWindowFn)(void*)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetShellWindow");
    if (!setShell || !setShell(g_desktop)) Log(L"couldn't make the desktop Windows' shell window (error %lu)", GetLastError());
}

void DesktopHostStop() {
    if (g_browser) {
        if (IShellView* view = g_browser->view) {
            SaveIconSize(view);
            if (g_viewWnd) RevokeDragDrop(g_viewWnd);
            view->UIActivate(SVUIA_DEACTIVATE);
            view->DestroyViewWindow();
            view->Release();
            g_browser->view = nullptr;
        }
        g_browser->Release();
        g_browser = nullptr;
    }
    g_viewWnd = nullptr;
    while (!g_fades.empty()) EndFade(g_fades.begin()->first);
    if (g_desktop) DestroyWindow(g_desktop);
    g_desktop = nullptr;
    if (g_wall) DeleteObject(g_wall);
    g_wall = nullptr;
    if (g_ole) OleUninitialize();
    g_ole = false;
}

}  // namespace td
