#include "extensions.h"

#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cwctype>

#include "config.h"
#include "json-lite.h"
#include "studio.h"

namespace td {

namespace {

std::vector<Extension> BuiltIn() {
    auto make = [](const wchar_t* id, const wchar_t* name, const wchar_t* kind, const wchar_t* about,
                   std::vector<std::pair<std::wstring, std::wstring>> settings) {
        Extension e;
        e.id = id, e.name = name, e.kind = kind, e.description = about, e.settings = std::move(settings);
        e.author = L"Tourne", e.licence = L"MIT", e.version = L"1.0", e.builtIn = true;
        return e;
    };
    return {
        make(L"arcade-crt", L"Arcade CRT", L"Shader", L"Scanlines and an aperture grille over every screen, like a cabinet's tube.",
             {{L"shader.screen", L"crt"}, {L"shader.amount", L"70"}}),
        make(L"lofi-vhs", L"Lo-fi VHS", L"Shader", L"A worn tape: wobble, bleeding colour and noise over every screen.",
             {{L"shader.screen", L"vhs"}, {L"shader.amount", L"45"}}),
        make(L"newsprint", L"Newsprint", L"Shader", L"Every screen printed in halftone dots.",
             {{L"shader.screen", L"halftone"}, {L"shader.amount", L"55"}}),
        make(L"film-grain", L"Film grain", L"Shader", L"A fine moving grain, as on film stock.",
             {{L"shader.screen", L"grain"}, {L"shader.amount", L"35"}}),
        make(L"pocket", L"Pocket", L"Shader", L"Few colours and an ordered dither, like an old handheld.",
             {{L"shader.screen", L"dither"}, {L"shader.amount", L"80"}}),
        make(L"md3-motion", L"Material motion", L"Motion",
             L"end-4's Material curves: windows pop in from 60%, desktops slide and fade a little, everything decelerates.",
             {{L"motion.enabled", L"true"},
              {L"motion.bezier.md3_decel", L"0.05, 0.7, 0.1, 1"},
              {L"motion.windowsIn", L"1, 3, md3_decel, popin 60%"},
              {L"motion.windowsMove", L"1, 3, md3_decel"},
              {L"motion.workspaces", L"1, 7, md3_decel, slidefade 15%"},
              {L"motion.layers", L"1, 2, md3_decel, slide"}}),
        make(L"bouncy-motion", L"Bouncy", L"Motion", L"Springs everywhere: windows bounce in, desktops and tiles settle with a wobble.",
             {{L"motion.enabled", L"true"},
              {L"motion.windowsIn", L"1, 5, bouncy, popin 80%"},
              {L"motion.windowsMove", L"1, 4, bouncy"},
              {L"motion.workspaces", L"1, 5, gentle, slide"}}),
        make(L"expose", L"Exposé", L"View", L"Alt+Tab as Mission Control: every window spread out over a dark, soft screen.",
             {{L"switcher.view.style", L"mission"}, {L"switcher.view.dim", L"65"}, {L"switcher.view.blur", L"8"}}),
        make(L"vista-flip", L"Vista Flip", L"View", L"Alt+Tab as Flip 3D, the stack on a slant, as in Windows Vista.",
             {{L"switcher.view.style", L"flip3d"}, {L"switcher.view.dim", L"40"}, {L"switcher.view.blur", L"4"}}),
        make(L"glass-tube", L"Glass tube", L"Glass", L"Tourne's glass with scanlines in it.",
             {{L"glass.enabled", L"true"}, {L"glass.effect", L"crt"}, {L"glass.effectAmount", L"60"}}),
    };
}

std::wstring Lowered(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

bool ReadFileUtf8(const std::wstring& path, std::string* out) {
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    GetFileSizeEx(f, &size);
    if (size.QuadPart > 1 << 20) return CloseHandle(f), false;  // (a description, not a program)
    out->resize((size_t)size.QuadPart);
    DWORD got = 0;
    bool ok = ReadFile(f, out->data(), (DWORD)out->size(), &got, nullptr) && got == out->size();
    CloseHandle(f);
    if (out->size() >= 3 && (unsigned char)(*out)[0] == 0xEF) out->erase(0, 3);
    return ok;
}

// extension.json as an Extension; false with why.
bool Read(const std::wstring& file, Extension* e, std::wstring* error) {
    std::string text;
    json::Value v;
    if (!ReadFileUtf8(file, &text) || !json::Parse(text, &v) || v.kind != json::Value::Object)
        return *error = L"its extension.json isn't JSON", false;
    auto s = [&](const char* k) { return Utf8ToWide(v[k].text); };
    e->id = s("id"), e->name = s("name"), e->author = s("author"), e->licence = s("licence").empty() ? s("license") : s("licence");
    e->kind = s("kind"), e->version = s("version"), e->description = s("description");
    if (e->id.empty() || e->id.find_first_not_of(L"abcdefghijklmnopqrstuvwxyz0123456789-_.") != std::wstring::npos)
        return *error = L"its id must be lowercase letters, digits and - _ .", false;
    if (e->name.empty()) e->name = e->id;
    if (e->kind.empty()) e->kind = L"Look";
    for (auto const& [key, value] : v["settings"].fields) {
        std::wstring k = Utf8ToWide(key);
        if (!ExtensionMaySet(k)) return *error = L"it sets " + k + L", which an extension may not", false;
        std::wstring val = value.kind == json::Value::String ? Utf8ToWide(value.text)
                           : value.kind == json::Value::Bool ? (value.boolean ? L"true" : L"false")
                                                              : std::to_wstring((long long)value.number);
        if (value.kind == json::Value::Number && value.number != (long long)value.number) val = std::to_wstring(value.number);
        e->settings.push_back({k, val});
    }
    if (e->settings.empty()) return *error = L"it sets nothing", false;
    return true;
}

}  // namespace

std::wstring ExtensionsFolder() { return ConfigFolder() + L"\\extensions"; }

bool ExtensionMaySet(const std::wstring& key) {
    std::wstring k = Lowered(key);
    static const wchar_t* const kAllowed[] = {L"motion.",          L"glass.",           L"shader.", L"switcher.view.", L"switcher.tiling.",
                                              L"switcher.borders.", L"switcher.theme.", L"theme.",  L"identity."};
    bool allowed = false;
    for (auto p : kAllowed) allowed |= k.rfind(p, 0) == 0;
    // Nothing that names a program to run or a file to load into one.
    for (auto bad : {L".exe", L".command", L".args", L".path", L"glass.app."})
        if (k.find(bad) != std::wstring::npos) allowed = false;
    return allowed;
}

const std::vector<Extension>& ExtensionList(bool reload) {
    static std::vector<Extension> list;
    static bool read = false;
    if (read && !reload) return list;
    read = true;
    list = BuiltIn();
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((ExtensionsFolder() + L"\\*").c_str(), &fd);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
            Extension e;
            std::wstring error, folder = ExtensionsFolder() + L"\\" + fd.cFileName;
            if (!Read(folder + L"\\extension.json", &e, &error)) continue;
            e.folder = folder;
            list.push_back(std::move(e));
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }
    return list;
}

bool ExtensionAdd(const std::wstring& path, std::wstring* error) {
    std::wstring from = path;
    DWORD at = GetFileAttributesW(from.c_str());
    if (at == INVALID_FILE_ATTRIBUTES) return *error = L"it isn't there", false;
    if (!(at & FILE_ATTRIBUTE_DIRECTORY)) from = from.substr(0, from.find_last_of(L"\\/"));
    Extension e;
    if (!Read(from + L"\\extension.json", &e, error)) return false;
    for (auto const& b : BuiltIn())
        if (b.id == e.id) return *error = L"Tourne has one called " + e.id + L" already", false;
    std::wstring to = ExtensionsFolder() + L"\\" + e.id;
    CreateDirectoryW(ConfigFolder().c_str(), nullptr);
    CreateDirectoryW(ExtensionsFolder().c_str(), nullptr);
    CreateDirectoryW(to.c_str(), nullptr);
    // Its description and pictures; anything else in the folder stays where it was.
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((from + L"\\*").c_str(), &fd);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            std::wstring name = Lowered(fd.cFileName);
            size_t dot = name.rfind(L'.');
            std::wstring ext = dot == std::wstring::npos ? L"" : name.substr(dot);
            if (name == L"extension.json" || ext == L".png" || ext == L".jpg" || ext == L".gif" || ext == L".md" || ext == L".txt")
                CopyFileW((from + L"\\" + fd.cFileName).c_str(), (to + L"\\" + fd.cFileName).c_str(), FALSE);
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }
    ExtensionList(true);
    return true;
}

bool ExtensionRemove(const Extension& e) {
    if (e.builtIn || e.folder.empty()) return false;
    std::wstring from = e.folder + L'\0';
    SHFILEOPSTRUCTW op{};
    op.wFunc = FO_DELETE;
    op.pFrom = from.c_str();
    op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
    bool ok = SHFileOperationW(&op) == 0;
    ExtensionList(true);
    return ok;
}

}  // namespace td
