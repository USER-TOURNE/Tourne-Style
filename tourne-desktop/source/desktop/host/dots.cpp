#include "dots.h"

#include <windows.h>

#include <algorithm>
#include <cwctype>
#include <map>

#include "config.h"
#include "extensions.h"
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

bool ReadUtf8(const std::wstring& path, std::string* out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    GetFileSizeEx(f, &size);
    if (size.QuadPart > 8 << 20) return CloseHandle(f), false;
    out->resize((size_t)size.QuadPart);
    DWORD got = 0;
    bool ok = ReadFile(f, out->data(), (DWORD)out->size(), &got, nullptr);
    CloseHandle(f);
    if (out->size() >= 3 && (unsigned char)(*out)[0] == 0xEF) out->erase(0, 3);
    return ok;
}

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

bool DotsExport(const std::wstring& path, const std::wstring& name, std::wstring* error) {
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
    HANDLE f = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return *error = L"the file couldn't be written", false;
    DWORD wrote = 0;
    bool ok = WriteFile(f, bytes.data(), (DWORD)bytes.size(), &wrote, nullptr) && wrote == bytes.size();
    CloseHandle(f);
    return ok || (*error = L"the file couldn't be written", false);
}

bool DotsRead(const std::wstring& path, DotsLook* out, std::wstring* error) {
    std::string bytes;
    if (!ReadUtf8(path, &bytes)) return *error = L"it didn't read", false;
    std::wstring text = Utf8ToWide(bytes);
    std::wstring file = Lowered(path.substr(path.find_last_of(L"\\/") + 1));
    bool tourne = file.size() > 7 && file.substr(file.size() - 7) == L".tourne";
    if (!tourne) {
        out->from = L"Hyprland";
        out->name = L"a Hyprland config";
        FromHyprland(text, out);
    } else {
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
        if (out->name.empty()) out->name = file;
    }
    if (out->settings.empty() && out->extensions.empty()) return *error = L"there's nothing in it Tourne reads", false;
    return true;
}

}  // namespace td
