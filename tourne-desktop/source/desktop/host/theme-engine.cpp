#include "theme-engine.h"

#include <shlobj.h>
#include <shobjidl.h>
#include <wincodec.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "css-injector.h"
#include "palette-gen.h"
#include "presets.h"
#include "td.h"

namespace td {

namespace {

constexpr wchar_t kThemeKey[] = L"Software\\Tourne\\Theme";
constexpr UINT WM_TD_WALLPAPER = WM_APP + 20;  // the wallpaper folder changed
constexpr UINT WM_TD_THEME_DONE = WM_APP + 21; // a picture was read: wp = its generation, lp = Made*
constexpr UINT_PTR kTimerTheme = 20;

struct Wanted {
    std::wstring source = L"off", profileName = L"adaptive", mode = L"auto", seedText, preset;
    uint32_t seed = 0x64AA89;
    PaletteRequest request;
};

struct Made {
    Roles roles;
    std::wstring image, key;
    bool ok = false;
};

HWND g_wnd;
Wanted g_wanted;
std::wstring g_madeKey;           // what the record was last made from (so the same input isn't made twice)
std::atomic<unsigned> g_generation;
HANDLE g_folderThread, g_folderStop;

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"theme", format, args);
    va_end(args);
}

std::wstring Setting(const wchar_t* key) {
    const wchar_t* v = GetString(L"theme", key);
    std::wstring out = v ? v : L"";
    FreeString(v);
    return out;
}

bool ParseHex(std::wstring text, uint32_t* out) {
    text.erase(0, text.find_first_not_of(L" \t#"));
    text.erase(text.find_last_not_of(L" \t") + 1);
    wchar_t* end = nullptr;
    unsigned long v = wcstoul(text.c_str(), &end, 16);
    if (text.size() != 6 || !end || *end) return false;
    *out = (uint32_t)v;
    return true;
}

bool WindowsIsLight() {
    DWORD value = 0, size = sizeof(value);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                 L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &value, &size);
    return value != 0;
}

Wanted ReadWanted() {
    Wanted w;
    w.source = Setting(L"source");
    if (w.source == L"on") w.source = L"wallpaper";  // (on: the usual source)
    w.profileName = Setting(L"profile");
    if (!ParseProfile(w.profileName, &w.request.profile)) {
        if (!w.profileName.empty()) Log(L"theme.profile: \"%s\" isn't one (adaptive, monochrome, muted, vivid, pastel)", w.profileName.c_str());
        w.profileName = L"adaptive";
    }
    w.mode = Setting(L"mode");
    w.request.light = w.mode == L"light" || (w.mode != L"dark" && WindowsIsLight());
    w.preset = Setting(L"preset");
    w.seedText = Setting(L"seed");
    if (!w.seedText.empty() && !ParseHex(w.seedText, &w.seed)) Log(L"theme.seed: \"%s\" isn't a #RRGGBB colour", w.seedText.c_str());
    double contrast = _wtof(Setting(L"contrast").c_str());
    w.request.textContrast = std::clamp(contrast > 0 ? contrast : 4.5, 3.0, 21.0);
    return w;
}

// What a palette is made from, as one line: the same line, the same palette.
std::wstring RequestKey(Wanted const& w, std::wstring const& input) {
    wchar_t text[64];
    swprintf(text, 64, L"|%s|%.1f|", w.request.light ? L"light" : L"dark", w.request.textContrast);
    return w.source + L"|" + w.profileName + text + input;
}

////////////////////////////////////////////////////////////////////////////////
// The wallpaper: the main screen's picture, read small.

std::wstring FileStamp(std::wstring const& path) {
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &a)) return L"";
    wchar_t text[64];
    swprintf(text, 64, L"%08lX%08lX:%lu", a.ftLastWriteTime.dwHighDateTime, a.ftLastWriteTime.dwLowDateTime, a.nFileSizeLow);
    return text;
}

std::wstring ThemesFolder() {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path))) out = std::wstring(path) + L"\\Microsoft\\Windows\\Themes";
    CoTaskMemFree(path);
    return out;
}

// On a thread with COM. The main screen's wallpaper file; else the copy
// Windows keeps of what it shows (Spotlight, slideshows); empty for a plain
// colour, given in *solid then.
std::wstring MainWallpaper(uint32_t* solid, bool* isSolid) {
    *isSolid = false;
    std::wstring path;
    IDesktopWallpaper* dw = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) {
        UINT count = 0;
        dw->GetMonitorDevicePathCount(&count);
        for (UINT i = 0; i < count && path.empty(); i++) {
            LPWSTR id = nullptr;
            RECT rc{};
            if (FAILED(dw->GetMonitorDevicePathAt(i, &id))) continue;
            if (SUCCEEDED(dw->GetMonitorRECT(id, &rc)) && rc.left <= 0 && rc.top <= 0 && rc.right > 0 && rc.bottom > 0) {
                LPWSTR file = nullptr;
                if (SUCCEEDED(dw->GetWallpaper(id, &file)) && file) path = file;
                CoTaskMemFree(file);
            }
            CoTaskMemFree(id);
        }
        if (!path.empty() && GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) path.clear();
        // No picture set at all (not a slideshow or Spotlight, which leave
        // this empty too but have one): the plain background colour.
        wchar_t set[MAX_PATH] = L"";
        DWORD size = sizeof(set);
        RegGetValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper", RRF_RT_REG_SZ, nullptr, set, &size);
        COLORREF color = 0;
        if (path.empty() && !*set && SUCCEEDED(dw->GetBackgroundColor(&color))) {
            *isSolid = true;
            *solid = GetRValue(color) << 16 | GetGValue(color) << 8 | GetBValue(color);
        }
        dw->Release();
    }
    if (path.empty() && !*isSolid) {
        std::wstring copy = ThemesFolder() + L"\\TranscodedWallpaper";
        if (GetFileAttributesW(copy.c_str()) != INVALID_FILE_ATTRIBUTES) path = copy;
    }
    return path;
}

// A picture's pixels at most 128 across (WIC reads JPEGs that small
// directly), as 0xRRGGBB; see-through pixels left out.
bool ReadPicture(std::wstring const& path, std::vector<uint32_t>* out) {
    IWICImagingFactory* f = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f)))) return false;
    bool ok = false;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    IWICFormatConverter* converter = nullptr;
    UINT w = 0, h = 0;
    if (SUCCEEDED(f->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
        SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(frame->GetSize(&w, &h)) && w && h) {
        UINT side = 128, sw = w >= h ? std::min(w, side) : std::max(1u, w * std::min(h, side) / h);
        UINT sh = w >= h ? std::max(1u, h * sw / w) : std::min(h, side);
        if (SUCCEEDED(f->CreateBitmapScaler(&scaler)) && SUCCEEDED(scaler->Initialize(frame, sw, sh, WICBitmapInterpolationModeFant)) &&
            SUCCEEDED(f->CreateFormatConverter(&converter)) &&
            SUCCEEDED(converter->Initialize(scaler, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0,
                                            WICBitmapPaletteTypeCustom))) {
            std::vector<uint32_t> pixels(sw * sh);
            if (SUCCEEDED(converter->CopyPixels(nullptr, sw * 4, (UINT)(pixels.size() * 4), (BYTE*)pixels.data()))) {
                out->clear();
                for (uint32_t p : pixels)
                    if ((p >> 24) >= 128) out->push_back(p & 0xFFFFFF);
                ok = !out->empty();
            }
        }
    }
    if (converter) converter->Release();
    if (scaler) scaler->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    f->Release();
    return ok;
}

struct Job {
    Wanted wanted;
    std::wstring lastKey;
    unsigned generation;
};

// Its own thread: the picture is found and read here, so the host (and the
// modules on its thread) never wait on a decoder.
DWORD WINAPI WallpaperJob(LPVOID param) {
    std::unique_ptr<Job> job((Job*)param);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    auto made = std::make_unique<Made>();
    uint32_t solid = 0;
    bool isSolid = false;
    std::wstring path = MainWallpaper(&solid, &isSolid);
    wchar_t solidText[16];
    swprintf(solidText, 16, L"#%06X", solid);
    made->image = isSolid ? solidText : path;
    made->key = RequestKey(job->wanted, isSolid ? made->image : path + L"|" + FileStamp(path));
    if (made->key != job->lastKey) {
        std::vector<uint32_t> pixels;
        if (isSolid) {
            made->roles = PaletteFromSeed(solid, job->wanted.request);
            made->ok = true;
        } else if (!path.empty() && ReadPicture(path, &pixels)) {
            made->roles = PaletteFromPixels(pixels, job->wanted.request);
            made->ok = true;
        } else {
            Log(L"couldn't read the wallpaper \"%s\"", path.c_str());
        }
    }
    CoUninitialize();
    if (!made->ok || !PostMessageW(g_wnd, WM_TD_THEME_DONE, job->generation, (LPARAM)made.get())) return 0;
    made.release();
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// The record

bool SameRecord(HKEY key, Roles const& r, std::wstring const& made) {
    struct { const wchar_t* name; uint32_t value; } const roles[] = {
        {L"Ground", r.ground}, {L"Raised", r.raised}, {L"Line", r.line}, {L"Edge", r.edge}, {L"Text", r.text},
        {L"Accent", r.accent}, {L"Muted", r.muted}, {L"Dim", r.dim}, {L"Warn", r.warn}, {L"Bad", r.bad},
        {L"Accent2", r.accent2}, {L"Good", r.good}};
    for (auto const& role : roles) {
        DWORD v = 0, size = sizeof(v);
        if (RegGetValueW(key, nullptr, role.name, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS || v != role.value) return false;
    }
    wchar_t text[1024] = L"";
    DWORD size = sizeof(text);
    RegGetValueW(key, nullptr, L"MadeFrom", RRF_RT_REG_SZ, nullptr, text, &size);
    return made == text;
}

// Written in full, Revision last, then every window told. Nothing written if
// nothing changed.
void WriteRecord(Made const& made, Wanted const& w) {
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, kThemeKey, 0, nullptr, 0, KEY_READ | KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) return;
    Roles const& r = made.roles;
    if (SameRecord(key, r, made.key)) {
        RegCloseKey(key);
        g_madeKey = made.key;
        return;
    }
    auto dword = [&](const wchar_t* name, DWORD v) { RegSetValueExW(key, name, 0, REG_DWORD, (const BYTE*)&v, sizeof(v)); };
    auto text = [&](const wchar_t* name, std::wstring const& v) {
        RegSetValueExW(key, name, 0, REG_SZ, (const BYTE*)v.c_str(), (DWORD)((v.size() + 1) * sizeof(wchar_t)));
    };
    dword(L"Ground", r.ground), dword(L"Raised", r.raised), dword(L"Line", r.line), dword(L"Edge", r.edge);
    dword(L"Text", r.text), dword(L"Accent", r.accent), dword(L"Muted", r.muted), dword(L"Dim", r.dim);
    dword(L"Warn", r.warn), dword(L"Bad", r.bad), dword(L"Accent2", r.accent2), dword(L"Good", r.good);
    dword(L"Light", w.request.light);
    text(L"Source", w.source);
    text(L"Profile", w.profileName);
    text(L"Image", made.image);
    text(L"MadeFrom", made.key);
    DWORD revision = 0, size = sizeof(revision);
    RegGetValueW(key, nullptr, L"Revision", RRF_RT_REG_DWORD, nullptr, &revision, &size);
    dword(L"Revision", revision + 1);
    RegCloseKey(key);
    g_madeKey = made.key;
    Log(L"new theme %lu from %s (%s, %s): ground #%06X, text #%06X, accent #%06X", revision + 1, w.source.c_str(),
        w.profileName.c_str(), w.request.light ? L"light" : L"dark", r.ground, r.text, r.accent);
    PostMessageW(HWND_BROADCAST, RegisterWindowMessageW(L"TourneThemeChanged"), revision + 1, 0);
    InjectorThemeChanged(r);  // the apps' palette CSS
}

////////////////////////////////////////////////////////////////////////////////
// Watching the wallpaper: Windows rewrites its copy of the picture in the
// Themes folder on every change, slideshow steps and Spotlight included.

DWORD WINAPI FolderThread(LPVOID) {
    HANDLE change = FindFirstChangeNotificationW(ThemesFolder().c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);
    if (change == INVALID_HANDLE_VALUE) return 0;
    HANDLE waits[2] = {change, g_folderStop};
    while (WaitForMultipleObjects(2, waits, FALSE, INFINITE) == WAIT_OBJECT_0) {
        PostMessageW(g_wnd, WM_TD_WALLPAPER, 0, 0);
        if (!FindNextChangeNotification(change)) break;
    }
    FindCloseChangeNotification(change);
    return 0;
}

void WatchFolder(bool on) {
    if (on == (g_folderThread != nullptr)) return;
    if (on) {
        g_folderStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        g_folderThread = CreateThread(nullptr, 0, FolderThread, nullptr, 0, nullptr);
        return;
    }
    SetEvent(g_folderStop);
    WaitForSingleObject(g_folderThread, 2000);
    CloseHandle(g_folderThread);
    CloseHandle(g_folderStop);
    g_folderThread = g_folderStop = nullptr;
}

// Makes the record from what the settings ask for, if that changed.
void Make() {
    Wanted const& w = g_wanted;
    if (w.source == L"seed") {
        wchar_t seed[16];
        swprintf(seed, 16, L"#%06X", w.seed);
        Made made;
        made.key = RequestKey(w, seed);
        if (made.key == g_madeKey) return;
        made.roles = PaletteFromSeed(w.seed, w.request);
        made.image = seed;
        WriteRecord(made, w);
    } else if (w.source == L"preset") {
        // A preset is a finished palette: used as it is, its own dark or light.
        const Preset* p = FindPreset(w.preset);
        if (!p) {
            Log(L"theme.preset: there's no preset \"%s\"", w.preset.c_str());
            return;
        }
        Wanted shown = w;
        shown.request.light = p->light;
        shown.profileName = L"preset";
        Made made;
        made.key = L"preset|" + w.preset;
        if (made.key == g_madeKey) return;
        made.roles = p->AsRoles();
        made.image = p->name;
        WriteRecord(made, shown);
    } else if (w.source == L"wallpaper") {
        auto* job = new Job{w, g_madeKey, ++g_generation};
        HANDLE thread = CreateThread(nullptr, 0, WallpaperJob, job, 0, nullptr);
        if (thread) CloseHandle(thread);
        else delete job;
    }
}

void Soon(UINT ms) { SetTimer(g_wnd, kTimerTheme, ms, nullptr); }

}  // namespace

void ThemeDefaults(Settings& out) {
    out[L"theme.source"] = L"off";
    out[L"theme.profile"] = L"adaptive";
    out[L"theme.mode"] = L"auto";
    out[L"theme.seed"] = L"#64AA89";
    out[L"theme.preset"] = L"";
    out[L"theme.contrast"] = L"4.5";
}

void ThemeEngineStart(HWND host) {
    g_wnd = host;
    ThemeEngineApply();
}

void ThemeEngineApply() {
    g_wanted = ReadWanted();
    if (g_wanted.source != L"off" && g_wanted.source != L"wallpaper" && g_wanted.source != L"seed" && g_wanted.source != L"preset") {
        Log(L"theme.source: \"%s\" isn't one (off, wallpaper, seed, preset)", g_wanted.source.c_str());
        g_wanted.source = L"off";
    }
    WatchFolder(g_wanted.source == L"wallpaper");
    if (g_wanted.source != L"off") Soon(10);
}

void ThemeEngineStop() {
    WatchFolder(false);
    if (g_wnd) KillTimer(g_wnd, kTimerTheme);
    g_generation++;
}

bool ThemeEngineMessage(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_SETTINGCHANGE:
            if (g_wanted.source == L"off") return false;
            if (wp == SPI_SETDESKWALLPAPER && g_wanted.source == L"wallpaper") Soon(300);
            // Windows' light or dark mode changed: following it, so made again.
            if (lp && !wcscmp((const wchar_t*)lp, L"ImmersiveColorSet") && g_wanted.mode != L"dark" && g_wanted.mode != L"light") {
                bool light = WindowsIsLight();
                if (light != g_wanted.request.light) {
                    g_wanted.request.light = light;
                    Soon(300);
                }
            }
            return false;  // (others may want it too)
        case WM_TD_WALLPAPER:
            // Windows writes the picture in a few steps: once it settles.
            Soon(300);
            return true;
        case WM_TIMER:
            if (wp != kTimerTheme) return false;
            KillTimer(wnd, kTimerTheme);
            Make();
            return true;
        case WM_TD_THEME_DONE: {
            std::unique_ptr<Made> made((Made*)lp);
            if ((unsigned)wp == g_generation && made->ok && g_wanted.source == L"wallpaper") WriteRecord(*made, g_wanted);
            return true;
        }
    }
    return false;
}

}  // namespace td
