// Runs desktop/host/plugin-runtime.cpp outside Tourne Desktop: installs the
// plugins named (into TOURNE_PLUGINS_DIR, as plugintest does), runs them for a
// few seconds as a bar would see them (asking for the items, clicking each),
// and prints what each showed, logged and failed at.
//
//   .\tools\plugins\Run-PluginTest.ps1 noctalia/world_clock noctalia/example
//
// TOURNE_PLUGIN_SETTINGS="plugins.<id>.<key>=<value>;..." stands in for the user's settings.
#include <windows.h>

#include <cstdio>
#include <string>
#include <vector>

#include "plugin-runtime.h"
#include "plugins.h"

namespace td {
std::wstring Utf8ToWide(const std::string& s) {
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}
std::string WideToUtf8(const std::wstring& w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}
void LogV(const wchar_t* prefix, const wchar_t* format, va_list args) {
    wchar_t text[2048];
    _vsnwprintf(text, 2047, format, args);  // (as the host does: %s is a wide string)
    text[2047] = 0;
    printf("  [%ls] %s\n", prefix, WideToUtf8(text).c_str());
}
bool FlyoutPalette(uint32_t*) { return false; }  // (no System Icons here: the Tourne scheme)
// (no Studio here: said, not opened or written)
void OpenStudio(std::wstring const& page, std::wstring const& query) { printf("  [studio] would open %ls at %ls\n", page.c_str(), query.c_str()); }
bool WriteStudioSettings(std::vector<std::pair<std::wstring, std::wstring>> const& set, std::wstring const&, std::wstring const& why) {
    for (auto const& [k, v] : set) printf("  [studio] would set %ls = %ls (%ls)\n", k.c_str(), v.c_str(), why.c_str());
    return true;
}
void OpenBindsEditor() { printf("  [binds] would open Keys and rules\n"); }
void CaptureToolkit(const std::wstring&, const std::wstring&) {}
static bool g_keystrokes;
bool KeystrokesShow(int how) { return g_keystrokes = how < 0 ? !g_keystrokes : how != 0; }
bool KeystrokesShowing() { return g_keystrokes; }
const wchar_t* GetString(const wchar_t*, const std::wstring&) { return nullptr; }
void FreeString(const wchar_t*) {}
int GetInt(const wchar_t*, const std::wstring&) { return 0; }  // (Keys and rules off here: Windows' own hotkeys)
// A panel's kept size, place and pin (config.cpp's values, in the same key).
static std::wstring ValuesKey(const wchar_t* prefix) { return std::wstring(L"Software\\Tourne\\Desktop\\Values\\") + prefix; }
int GetIntValue(const wchar_t* prefix, const wchar_t* name, int fallback) {
    DWORD v = 0, size = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS) return fallback;
    return (int)v;
}
BOOL SetIntValue(const wchar_t* prefix, const wchar_t* name, int value) {
    DWORD v = (DWORD)value;
    return RegSetKeyValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name, REG_DWORD, &v, sizeof(v)) == ERROR_SUCCESS;
}
BOOL DeleteValue(const wchar_t* prefix, const wchar_t* name) {
    return RegDeleteKeyValueW(HKEY_CURRENT_USER, ValuesKey(prefix).c_str(), name) == ERROR_SUCCESS;
}
std::vector<std::pair<std::wstring, std::wstring>> SettingsWithPrefix(const std::wstring& prefix) {
    std::vector<std::pair<std::wstring, std::wstring>> out;
    wchar_t buf[4096];
    if (!GetEnvironmentVariableW(L"TOURNE_PLUGIN_SETTINGS", buf, 4096)) return out;
    std::wstring all = buf;
    for (size_t at = 0; at < all.size();) {
        size_t end = all.find(L';', at);
        std::wstring one = all.substr(at, end - at);
        at = end == std::wstring::npos ? all.size() : end + 1;
        size_t eq = one.find(L'=');
        if (eq != std::wstring::npos && one.rfind(prefix, 0) == 0) out.push_back({one.substr(0, eq), one.substr(eq + 1)});
    }
    return out;
}
}  // namespace td

namespace {

#include "plugin-bar-shared.cpp"

std::vector<PluginBarItem> g_items;
int g_answers = 0;
unsigned g_version = 0;

LRESULT CALLBACK BarProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_COPYDATA) {
        auto* cds = (const COPYDATASTRUCT*)lp;
        if (cds->dwData == kPluginBarItemsId && cds->lpData) {
            g_items = UnpackPluginBarItems(std::wstring_view((const wchar_t*)cds->lpData, cds->cbData / sizeof(wchar_t) - 1), &g_version);
            g_answers++;
        }
        return TRUE;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

std::string Show(const PluginBarItem& item) {
    char g[16] = "";
    if (!item.glyph.empty()) snprintf(g, sizeof(g), "U+%04X ", (unsigned)item.glyph[0]);
    return td::WideToUtf8(item.key) + "  [" + g + td::WideToUtf8(item.text) + "]" +
           (item.tip.empty() ? "" : "  tip [" + td::WideToUtf8(item.tip) + "]");
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    // Install what's named (and not there yet), waiting for each.
    HWND notify = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    for (int i = 1; i < argc; i++) {
        for (auto const& p : td::PluginCatalogue()) {
            if (p.id != argv[i] || td::PluginStateOf(p) == td::PluginState::Installed) continue;
            td::PluginInstall(p, notify, WM_APP);
            MSG m;
            while (GetMessageW(&m, nullptr, 0, 0) > 0 && m.message != WM_APP) DispatchMessageW(&m);
            printf("install %ls: %s\n", argv[i], td::PluginStateOf(p) == td::PluginState::Installed ? "ok" : "FAILED");
        }
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = BarProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneTaskbarTest_pluginrun";
    RegisterClassW(&wc);
    HWND bar = CreateWindowExW(0, wc.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    UINT changed = RegisterWindowMessageW(kPluginBarChangedMessage);
    UINT ask = RegisterWindowMessageW(kPluginBarAskMessage);
    UINT event = RegisterWindowMessageW(kPluginBarEventMessage);

    td::PluginRuntimeStart();
    HWND host = FindWindowExW(HWND_MESSAGE, nullptr, kPluginHostClass, nullptr);
    printf("runtime window: %s\n", host ? "found" : "MISSING");
    int changes = 0;
    bool clicked = false;
    ULONGLONG start = GetTickCount64();
    wchar_t ms[16] = L"";  // (TOURNE_PLUGINRUN_MS: longer, to look at a panel a click opened)
    ULONGLONG end = GetTickCount64() + (GetEnvironmentVariableW(L"TOURNE_PLUGINRUN_MS", ms, 16) ? _wtoi(ms) : 7000);
    while (GetTickCount64() < end) {
        MsgWaitForMultipleObjects(0, nullptr, FALSE, 100, QS_ALLINPUT);
        MSG m;
        while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) {
            if (m.message == changed) {
                changes++;
                PostMessageW(host, ask, (WPARAM)bar, 0);
            }
            DispatchMessageW(&m);
        }
        // After 3 s, a click on every item (or only TOURNE_PLUGINRUN_CLICK's).
        if (!clicked && GetTickCount64() > start + 3000 && !g_items.empty()) {
            clicked = true;
            wchar_t only[256] = L"";
            GetEnvironmentVariableW(L"TOURNE_PLUGINRUN_CLICK", only, 256);
            for (size_t i = 0; i < g_items.size(); i++) {
                if (*only && g_items[i].key != only) continue;
                PostMessageW(host, event, g_version, MakePluginBarEvent((int)i, kPluginClick));
                printf("clicked %s\n", td::WideToUtf8(g_items[i].key).c_str());
            }
        }
        // A second later, any panel it opened, saved as a picture (TOURNE_PLUGINRUN_SHOT: a .bmp path); then a click
        // in it at TOURNE_PLUGINRUN_PANELCLICK ("x,y", its own pixels) and, after that, TOURNE_PLUGINRUN_SHOT2.
        static int stage = 0;
        wchar_t path[MAX_PATH] = L"", at[32] = L"";
        if (clicked && stage == 1 && GetTickCount64() > start + 5500 && GetEnvironmentVariableW(L"TOURNE_PLUGINRUN_PANELCLICK", at, 32)) {
            stage = 2;
            if (HWND panel = FindWindowW(L"TourneDesktopPluginPanel", nullptr)) {
                int x = _wtoi(at), y = wcschr(at, L',') ? _wtoi(wcschr(at, L',') + 1) : 0;
                SendMessageW(panel, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
                SendMessageW(panel, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
                printf("clicked the panel at %d,%d\n", x, y);
            }
        }
        const wchar_t* which = stage == 0 ? L"TOURNE_PLUGINRUN_SHOT" : L"TOURNE_PLUGINRUN_SHOT2";
        if (clicked && (stage == 0 || stage == 2) && GetTickCount64() > start + (stage ? 7000 : 4500)) {
            stage++;
            HWND panel = GetEnvironmentVariableW(which, path, MAX_PATH) ? FindWindowW(L"TourneDesktopPluginPanel", nullptr) : nullptr;
            if (*path) printf("panel window: %s\n", panel ? "open" : "NONE");
            RECT r{};
            if (panel && GetWindowRect(panel, &r)) {
                int w = r.right - r.left, h = r.bottom - r.top;
                BITMAPINFO bi{};
                bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, h, 1, 32, BI_RGB};
                void* bits = nullptr;
                HDC dc = CreateCompatibleDC(nullptr);
                HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
                HGDIOBJ old = SelectObject(dc, bmp);
                PrintWindow(panel, dc, 2);  // (PW_RENDERFULLCONTENT)
                GdiFlush();
                BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader) + w * h * 4), 0, 0, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader))};
                if (FILE* f = _wfopen(path, L"wb")) {
                    fwrite(&fh, sizeof(fh), 1, f);
                    fwrite(&bi.bmiHeader, sizeof(bi.bmiHeader), 1, f);
                    fwrite(bits, 4, (size_t)w * h, f);
                    fclose(f);
                    printf("panel %dx%d saved\n", w, h);
                }
                SelectObject(dc, old);
                DeleteObject(bmp);
                DeleteDC(dc);
            }
        }
    }
    printf("\nbar told of changes %d times, answered %d times; items now:\n", changes, g_answers);
    for (auto const& item : g_items) printf("  %s\n", Show(item).c_str());
    // The Studio's Plugins page: each setting, its default and what it is.
    {
        td::Settings defaults;
        td::SchemaInfo info;
        td::PluginSettings(defaults, info);
        printf("\nsettings for the Studio:\n");
        for (auto const& [key, value] : defaults) {
            auto const& si = info[key];
            std::string kind = si.isSwitch ? "switch" : !si.options.empty() ? "choice of " + std::to_string(si.options.size()) : "text/number";
            printf("  %s = %s  (%s: \"%s\")\n", td::WideToUtf8(key).c_str(), td::WideToUtf8(value).c_str(), kind.c_str(),
                   td::WideToUtf8(si.name).c_str());
        }
    }
    printf("\nentries:\n");
    for (auto const& line : td::PluginRuntimeReport()) printf("  %s\n", td::WideToUtf8(line).c_str());
    td::PluginRuntimeStop();
    return 0;
}
