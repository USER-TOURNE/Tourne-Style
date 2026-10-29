#include "dots.h"

#include <windows.h>

#include <algorithm>
#include <cwctype>
#include <map>

#include <shlobj.h>
#include <wincodec.h>

#define MINIZ_NO_TIME
#include "miniz.h"

#include "config.h"
#include "extensions.h"
#include "plugins.h"
#include "studio.h"

namespace td {

namespace {

std::wstring Trimmed(std::wstring s) {
    size_t a = s.find_first_not_of(L" \t\r\n"), b = s.find_last_not_of(L" \t\r\n");
    return a == std::wstring::npos ? L"" : s.substr(a, b - a + 1);
}

std::wstring Lowered(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

bool ReadBytes(const std::wstring& path, std::string* out, LONGLONG most) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    GetFileSizeEx(f, &size);
    if (size.QuadPart > most) return CloseHandle(f), false;
    out->resize((size_t)size.QuadPart);
    DWORD got = 0;
    bool ok = ReadFile(f, out->data(), (DWORD)out->size(), &got, nullptr) && got == out->size();
    CloseHandle(f);
    return ok;
}

bool ReadUtf8(const std::wstring& path, std::string* out) {
    if (!ReadBytes(path, out, 8 << 20)) return false;
    if (out->size() >= 3 && (unsigned char)(*out)[0] == 0xEF) out->erase(0, 3);
    return true;
}

bool WriteBytes(const std::wstring& path, const void* data, size_t size) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD wrote = 0;
    bool ok = WriteFile(f, data, (DWORD)size, &wrote, nullptr) && wrote == size;
    CloseHandle(f);
    return ok;
}

bool EndsWith(const std::wstring& s, const wchar_t* tail) {
    size_t n = wcslen(tail);
    return s.size() >= n && Lowered(s.substr(s.size() - n)) == tail;
}

// The picture types a pack's wallpaper may be.
constexpr const wchar_t* kPictures[] = {L".jpg", L".jpeg", L".png", L".bmp", L".gif", L".webp", L".jxr"};
constexpr LONGLONG kMostWallpaper = 96LL << 20;

// Tied to this PC: a path, a drive, an environment variable.
bool Local(const std::wstring& value) {
    return value.find(L":\\") != std::wstring::npos || value.find(L":/") != std::wstring::npos || value.find(L"\\\\") != std::wstring::npos ||
           value.find(L'%') != std::wstring::npos;
}

// A look setting a .tourne file may carry: what an extension may set, and the bar's and Start's look.
bool LookSetting(const std::wstring& key) {
    if (ExtensionMaySet(key)) return true;
    std::wstring k = Lowered(key);
    for (auto p : {L"taskbar.", L"systemicons.", L"startmenu.", L"menus.", L"notify.", L"keystrokes."})
        if (k.rfind(p, 0) == 0) {
            for (auto bad : {L".command", L".args", L".exe", L".program", L".path", L".icon", L"hotkey"})
                if (k.find(bad) != std::wstring::npos) return false;
            return true;
        }
    return false;
}

// "rgba(33ccffee)", "rgb(33ccff)", "0xee33ccff" as Tourne's "33ccffee"; "45deg" kept; anything else "".
std::wstring HyprColour(std::wstring w) {
    w = Lowered(Trimmed(w));
    auto hex = [](std::wstring const& s) { return !s.empty() && s.find_first_not_of(L"0123456789abcdef") == std::wstring::npos; };
    if (w.rfind(L"rgba(", 0) == 0 || w.rfind(L"rgb(", 0) == 0) {
        std::wstring in = w.substr(w.find(L'(') + 1);
        in = in.substr(0, in.find(L')'));
        if (hex(in) && (in.size() == 6 || in.size() == 8)) return in;
        // rgba(r, g, b, a) in numbers
        int v[4] = {0, 0, 0, 255}, n = 0;
        for (size_t i = 0; i < in.size() && n < 4;) {
            size_t end = in.find(L',', i);
            std::wstring part = Trimmed(in.substr(i, end == std::wstring::npos ? std::wstring::npos : end - i));
            double d = _wtof(part.c_str());
            v[n++] = n == 4 && d <= 1 ? (int)(d * 255) : (int)d;
            if (end == std::wstring::npos) break;
            i = end + 1;
        }
        wchar_t out[12];
        swprintf(out, 12, L"%02x%02x%02x%02x", v[0] & 255, v[1] & 255, v[2] & 255, v[3] & 255);
        return out;
    }
    if (w.rfind(L"0x", 0) == 0 && w.size() == 10 && hex(w.substr(2))) return w.substr(4) + w.substr(2, 2);
    if (w.size() > 3 && w.substr(w.size() - 3) == L"deg") return w;
    return L"";
}

std::wstring HyprGradient(std::wstring const& value) {
    std::wstring out;
    size_t i = 0;
    while (i < value.size()) {
        while (i < value.size() && iswspace(value[i])) i++;
        size_t end = i;
        int depth = 0;
        while (end < value.size() && (depth || !iswspace(value[end]))) {
            if (value[end] == L'(') depth++;
            if (value[end] == L')') depth--;
            end++;
        }
        std::wstring c = HyprColour(value.substr(i, end - i));
        if (!c.empty()) out += (out.empty() ? L"" : L" ") + std::wstring(c.find(L"deg") == std::wstring::npos ? L"#" : L"") + c;
        i = end;
    }
    return out;
}

void FromHyprland(std::wstring const& text, DotsLook* out) {
    Settings read;
    std::vector<std::wstring> errors;
    ParseConfig(text, read, &errors);  // (animations and curves land as motion.* already)
    std::map<std::wstring, std::wstring> h;
    for (auto const& [key, value] : read) {
        std::wstring k = key;
        std::replace(k.begin(), k.end(), L':', L'.');
        h[Lowered(k)] = value;
    }
    auto set = [&](std::wstring const& key, std::wstring const& value) { out->settings.push_back({key, value}); };
    for (auto const& [key, value] : read)
        if (key.rfind(L"motion.", 0) == 0) set(key, value);
    auto num = [&](const wchar_t* key, int* v) {
        auto it = h.find(key);
        if (it == h.end()) return false;
        *v = _wtoi(it->second.c_str());
        return true;
    };
    int v;
    if (num(L"general.gaps_in", &v)) set(L"switcher.tiling.gapInner", std::to_wstring(v * 2));
    if (auto it = h.find(L"general.gaps_out"); it != h.end()) {
        // One number, or top right bottom left (as CSS).
        std::vector<int> n;
        for (std::wstring s = it->second; !s.empty();) {
            size_t end = s.find_first_of(L" ,");
            if (!Trimmed(s.substr(0, end)).empty()) n.push_back(_wtoi(s.substr(0, end).c_str()));
            s = end == std::wstring::npos ? L"" : s.substr(end + 1);
        }
        if (n.size() == 1) n = {n[0], n[0], n[0], n[0]};
        if (n.size() == 2) n = {n[0], n[1], n[0], n[1]};
        if (n.size() >= 4) {
            set(L"switcher.tiling.gapTop", std::to_wstring(n[0])), set(L"switcher.tiling.gapRight", std::to_wstring(n[1]));
            set(L"switcher.tiling.gapBottom", std::to_wstring(n[2])), set(L"switcher.tiling.gapLeft", std::to_wstring(n[3]));
        }
    }
    if (auto it = h.find(L"general.layout"); it != h.end())
        set(L"switcher.tiling.layout", Lowered(it->second) == L"master" ? L"verticalStack" : L"bsp");
    if (num(L"general.border_size", &v)) {
        set(L"switcher.borders.enabled", v > 0 ? L"true" : L"false");
        if (v > 0) set(L"switcher.borders.width", std::to_wstring(v));
    }
    for (auto [from, to] : {std::pair{L"general.col.active_border", L"switcher.borders.activeColor"},
                            std::pair{L"general.col.inactive_border", L"switcher.borders.inactiveColor"}})
        if (auto it = h.find(from); it != h.end()) {
            std::wstring g = HyprGradient(it->second);
            if (!g.empty()) set(to, g);
            else out->skipped.push_back(std::wstring(from) + L" (a colour Tourne doesn't read: " + it->second + L")");
        }
    if (num(L"decoration.rounding", &v)) set(L"switcher.borders.radius", v > 0 ? L"custom" : L"square"), set(L"switcher.borders.radiusCustom", std::to_wstring(v));
    if (auto it = h.find(L"decoration.shadow.enabled"); it != h.end()) {
        bool on = Lowered(it->second) == L"true" || it->second == L"1" || Lowered(it->second) == L"yes";
        set(L"switcher.borders.shadow", on ? L"all" : L"none");
    }
    if (num(L"decoration.blur.size", &v)) set(L"glass.blur", std::to_wstring(std::clamp(v * 2, 0, 64)));
    for (auto const& [k, value] : h)
        if (k.rfind(L"bind", 0) == 0 || k.rfind(L"exec", 0) == 0) {
            out->skipped.push_back(L"binds and exec lines (Keys and rules reads Hyprland's binds; paste them there)");
            break;
        }
}

}  // namespace

namespace {

// The wallpaper Windows has now, as a file: the one it was set from, else its own copy (which has no extension
// and is a JPEG).
std::wstring WallpaperNow(std::wstring* type) {
    wchar_t value[MAX_PATH * 2]{};
    DWORD size = sizeof(value);
    std::wstring path;
    if (RegGetValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper", RRF_RT_REG_SZ, nullptr, value, &size) == ERROR_SUCCESS)
        path = value;
    for (auto t : kPictures)
        if (!path.empty() && EndsWith(path, t) && GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) return *type = t, path;
    PWSTR roaming = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming)))
        path = std::wstring(roaming) + L"\\Microsoft\\Windows\\Themes\\TranscodedWallpaper";
    CoTaskMemFree(roaming);
    *type = L".jpg";
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES ? path : L"";
}

// Where a pack's wallpaper is kept once read in: %LOCALAPPDATA%\Tourne\Looks\<name>.
std::wstring LookFolder(std::wstring name) {
    for (auto& c : name)
        if (wcschr(L"<>:\"/\\|?*", c) || c < 32) c = L'_';
    if (name.empty() || name == L"." || name == L"..") name = L"look";
    PWSTR local = nullptr;
    std::wstring folder;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) folder = std::wstring(local) + L"\\Tourne\\Looks";
    CoTaskMemFree(local);
    if (folder.empty()) return L"";
    CreateDirectoryW(folder.c_str(), nullptr);
    folder += L"\\" + name;
    CreateDirectoryW(folder.c_str(), nullptr);
    return folder;
}

// A .tourne file's text read into a look.
void ReadTourne(const std::wstring& text, DotsLook* out) {
    out->from = L"Tourne";
    Settings read;
    ParseConfig(text, read, nullptr);
    for (auto const& [key, value] : read) {
        if (key == L"dots.name") out->name = value;
        else if (key == L"dots.author") out->author = value;
        else if (key.rfind(L"dots.extension", 0) == 0) out->extensions.push_back(WideToUtf8(value));
        else if (LookSetting(key) && !Local(value)) out->settings.push_back({key, value});
        else out->skipped.push_back(key);
    }
}

// A look pack read: its look, and its wallpaper unpacked beside the other looks'.
bool ReadPack(const std::wstring& path, DotsLook* out, std::wstring* error) {
    std::string zip;
    if (!ReadBytes(path, &zip, kMostWallpaper + (16 << 20))) return *error = L"it didn't read (or it's too big)", false;
    mz_zip_archive archive{};
    if (!mz_zip_reader_init_mem(&archive, zip.data(), zip.size(), 0)) return *error = L"it isn't a look pack (not a zip)", false;
    auto entry = [&](const char* name, size_t most, std::string* data) {
        int i = mz_zip_reader_locate_file(&archive, name, nullptr, 0);
        mz_zip_archive_file_stat stat{};
        if (i < 0 || !mz_zip_reader_file_stat(&archive, i, &stat) || stat.m_uncomp_size > most) return false;
        data->resize((size_t)stat.m_uncomp_size);
        return mz_zip_reader_extract_to_mem(&archive, i, data->data(), data->size(), 0) != 0;
    };
    std::string look, plugins, picture;
    bool ok = entry("look.tourne", 8 << 20, &look);
    if (ok) {
        if (look.size() >= 3 && (unsigned char)look[0] == 0xEF) look.erase(0, 3);
        ReadTourne(Utf8ToWide(look), out);
        if (entry("plugins.txt", 1 << 20, &plugins)) {
            std::wstring one;
            for (wchar_t c : Utf8ToWide(plugins) + L"\n") {
                if (c != L'\n') {
                    one += c;
                    continue;
                }
                one = Trimmed(one);
                if (!one.empty() && one[0] != L'#') out->plugins.push_back(one);
                one.clear();
            }
        }
        if (entry("preview.png", 32 << 20, &picture)) {
            std::wstring folder = LookFolder(out->name.empty() ? L"look" : out->name);
            if (!folder.empty() && WriteBytes(folder + L"\\preview.png", picture.data(), picture.size())) out->preview = folder + L"\\preview.png";
        }
        for (auto type : kPictures) {
            std::string name = "wallpaper" + WideToUtf8(type);
            if (!entry(name.c_str(), (size_t)kMostWallpaper, &picture)) continue;
            std::wstring folder = LookFolder(out->name.empty() ? L"look" : out->name);
            std::wstring file = folder + L"\\wallpaper" + type;
            if (!folder.empty() && WriteBytes(file, picture.data(), picture.size())) out->wallpaper = file;
            break;
        }
    }
    mz_zip_reader_end(&archive);
    if (!ok) return *error = L"there's no look.tourne in it", false;
    return true;
}

// The main screen as it is now, a PNG at most 1280 wide (for a look's preview). Empty if it couldn't be taken.
std::string ScreenPreview() {
    std::string png;
    int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
    if (w <= 0 || h <= 0) return png;
    int ow = std::min(w, 1280), oh = MulDiv(h, ow, w);
    HDC screen = GetDC(nullptr), dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), ow, -oh, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (bmp && bits) {
        HGDIOBJ old = SelectObject(dc, bmp);
        SetStretchBltMode(dc, HALFTONE);
        SetBrushOrgEx(dc, 0, 0, nullptr);
        StretchBlt(dc, 0, 0, ow, oh, screen, 0, 0, w, h, SRCCOPY | CAPTUREBLT);
        SelectObject(dc, old);
        GdiFlush();
        for (int i = 0; i < ow * oh; i++) ((DWORD*)bits)[i] |= 0xFF000000;  // (GDI leaves alpha at 0)
        IWICImagingFactory* f = nullptr;
        IStream* stream = nullptr;
        IWICBitmapEncoder* enc = nullptr;
        IWICBitmapFrameEncode* frame = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f))) &&
            SUCCEEDED(CreateStreamOnHGlobal(nullptr, TRUE, &stream)) && SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &enc)) &&
            SUCCEEDED(enc->Initialize(stream, WICBitmapEncoderNoCache)) && SUCCEEDED(enc->CreateNewFrame(&frame, nullptr)) &&
            SUCCEEDED(frame->Initialize(nullptr)) && SUCCEEDED(frame->SetSize(ow, oh))) {
            WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
            frame->SetPixelFormat(&fmt);
            HGLOBAL mem = nullptr;
            STATSTG st{};
            if (SUCCEEDED(frame->WritePixels(oh, ow * 4, (UINT)(ow * oh * 4), (BYTE*)bits)) && SUCCEEDED(frame->Commit()) &&
                SUCCEEDED(enc->Commit()) && SUCCEEDED(stream->Stat(&st, STATFLAG_NONAME)) && SUCCEEDED(GetHGlobalFromStream(stream, &mem))) {
                png.assign((const char*)GlobalLock(mem), (size_t)st.cbSize.QuadPart);
                GlobalUnlock(mem);
            }
        }
        if (frame) frame->Release();
        if (enc) enc->Release();
        if (stream) stream->Release();
        if (f) f->Release();
    }
    if (bmp) DeleteObject(bmp);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    return png;
}

bool ExportWith(const std::wstring& path, const std::wstring& name, std::string const& preview, std::wstring* error);

}  // namespace

bool DotsExport(const std::wstring& path, const std::wstring& name, std::wstring* error) {
    return ExportWith(path, name, "", error);
}

std::wstring DotsGalleryFolder() {
    std::wstring folder = ConfigFolder() + L"\\looks";
    CreateDirectoryW(folder.c_str(), nullptr);
    return folder;
}

bool DotsSaveToGallery(std::wstring name, std::wstring* saved, std::wstring* error) {
    if (name.empty()) {
        SYSTEMTIME t;
        GetLocalTime(&t);
        WCHAR line[64];
        swprintf_s(line, L"look %04d-%02d-%02d %02d%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute);
        name = line;
    }
    for (auto& c : name)
        if (wcschr(L"<>:\"/\\|?*", c) || c < 32) c = L'_';
    std::string preview = ScreenPreview();
    std::wstring base = DotsGalleryFolder() + L"\\" + name;
    *saved = base + L".tournelook";
    if (!ExportWith(*saved, name, preview, error)) return false;
    if (!preview.empty()) WriteBytes(base + L".png", preview.data(), preview.size());
    return true;
}

namespace {

bool ExportWith(const std::wstring& path, const std::wstring& name, std::string const& preview, std::wstring* error) {
    std::string text;
    if (!ReadUtf8(ConfigFolder() + L"\\tourne-gui.conf", &text)) return *error = L"your settings (tourne-gui.conf) didn't read", false;
    Settings mine;
    ParseConfig(Utf8ToWide(text), mine, nullptr);
    Settings look;
    look[L"dots.name"] = name;
    for (auto const& [key, value] : mine)
        if (LookSetting(key) && !Local(value)) look[key] = value;
    int n = 0;
    for (auto const& e : ExtensionList())
        if (!e.builtIn) {
            std::string json;
            if (ReadUtf8(e.folder + L"\\extension.json", &json)) {
                std::string one;
                for (char c : json) one += (c == '\r' || c == '\n' || c == '\t') ? ' ' : c;
                look[L"dots.extension." + std::to_wstring(++n)] = Utf8ToWide(one);
            }
        }
    std::wstring out = WriteConfig(look, L"# Tourne dots: a whole look. Import it in Tourne Studio > Presets > Import a look.\n\n");
    std::string bytes = WideToUtf8(out);
    if (!EndsWith(path, L".tournelook"))
        return WriteBytes(path, bytes.data(), bytes.size()) || (*error = L"the file couldn't be written", false);

    // A look pack: the same file, the wallpaper and the plugins, zipped.
    std::string plugins = "# The plugins this look was made with (install them on the Studio's Plugins page)\n";
    for (auto const& p : PluginCatalogue())
        if (PluginStateOf(p) == PluginState::Installed) plugins += WideToUtf8(p.id) + "\n";
    std::wstring type, wallpaper = WallpaperNow(&type);
    std::string picture;
    if (!wallpaper.empty() && !ReadBytes(wallpaper, &picture, kMostWallpaper)) picture.clear();
    mz_zip_archive archive{};
    if (!mz_zip_writer_init_heap(&archive, 0, 0)) return *error = L"the pack couldn't be made", false;
    bool ok = mz_zip_writer_add_mem(&archive, "look.tourne", bytes.data(), bytes.size(), MZ_BEST_COMPRESSION) &&
              mz_zip_writer_add_mem(&archive, "plugins.txt", plugins.data(), plugins.size(), MZ_BEST_COMPRESSION) &&
              (picture.empty() ||  // (pictures are compressed already: stored as they are)
               mz_zip_writer_add_mem(&archive, ("wallpaper" + WideToUtf8(type)).c_str(), picture.data(), picture.size(), MZ_NO_COMPRESSION)) &&
              (preview.empty() || mz_zip_writer_add_mem(&archive, "preview.png", preview.data(), preview.size(), MZ_NO_COMPRESSION));
    void* zip = nullptr;
    size_t size = 0;
    ok = ok && mz_zip_writer_finalize_heap_archive(&archive, &zip, &size);
    ok = ok && WriteBytes(path, zip, size);
    mz_zip_writer_end(&archive);
    if (zip) mz_free(zip);  // (finished, the archive's memory is the caller's)
    return ok || (*error = L"the pack couldn't be written", false);
}

}  // namespace

bool DotsRead(const std::wstring& path, DotsLook* out, std::wstring* error) {
    std::wstring file = Lowered(path.substr(path.find_last_of(L"\\/") + 1));
    if (EndsWith(file, L".tournelook")) {
        if (!ReadPack(path, out, error)) return false;
        if (out->name.empty()) out->name = file;
    } else {
        std::string bytes;
        if (!ReadUtf8(path, &bytes)) return *error = L"it didn't read", false;
        std::wstring text = Utf8ToWide(bytes);
        if (!EndsWith(file, L".tourne")) {
            out->from = L"Hyprland";
            out->name = L"a Hyprland config";
            FromHyprland(text, out);
        } else {
            ReadTourne(text, out);
            if (out->name.empty()) out->name = file;
        }
    }
    if (out->settings.empty() && out->extensions.empty()) return *error = L"there's nothing in it Tourne reads", false;
    return true;
}

}  // namespace td
