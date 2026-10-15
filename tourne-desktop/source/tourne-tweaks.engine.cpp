// ==WindhawkMod==
// @id              tourne-tweaks
// @name            Tourne'Suite Tweaks
// @description     Winaero Tweaker's options as settings (a part of Tourne'Suite)
// @version         1.0.0
// @author          USER-TOURNE
// @include         windhawk.exe
// @license         MIT
// ==/WindhawkMod==

// This file is generated from tourne-tweaks.engine.cpp and tweaks.def by
// Build-TourneSuiteRedirect.ps1; edit those, not this.

// ==WindhawkModSettings==
/*
{{TWEAKS_SETTINGS}}
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// The table

// A setting and what applying each of its values does. Ops are lines of the
// small language described at the top of tweaks.def.
struct Tweak {
    PCWSTR id;
    PCWSTR title;          // as the settings page names it
    char kind;             // 't' toggle, 'n' number, 's' text, 'c' choice
    char after;            // 'e' Explorer restart, 's' sign-out, 'r' restart
    bool admin;            // some of its ops need admin
    bool keepEmpty;        // text: empty, number: 0, means leave the system alone
    PCWSTR windowsDefault; // number: the value that runs the unset ops
    PCWSTR a;              // on / set / choice options
    PCWSTR b;              // off / unset
    PCWSTR uses;           // other settings the ops read, space separated
};

{{TWEAKS_TABLE}}

////////////////////////////////////////////////////////////////////////////////
// Settings

const Tweak* FindTweak(std::wstring_view id) {
    for (const Tweak& t : kTweaks) {
        if (id == t.id) {
            return &t;
        }
    }
    return nullptr;
}

std::wstring SettingValue(const Tweak& t) {
    if (t.kind == 's' || t.kind == 'c') {
        PCWSTR value = Wh_GetStringSetting(L"%s", t.id);
        std::wstring result = value ? value : L"";
        Wh_FreeStringSetting(value);
        return result;
    }
    return std::to_wstring(Wh_GetIntSetting(L"%s", t.id));
}

std::vector<std::wstring> Words(PCWSTR text) {
    std::vector<std::wstring> words;
    std::wstring word;
    for (PCWSTR p = text ? text : L"";; p++) {
        if (!*p || *p == L' ') {
            if (!word.empty()) {
                words.push_back(word);
            }
            word.clear();
            if (!*p) {
                break;
            }
        } else {
            word += *p;
        }
    }
    return words;
}

// What the stored fingerprint covers: the value and every setting it uses.
std::wstring Signature(const Tweak& t) {
    std::wstring sig = SettingValue(t);
    for (const std::wstring& id : Words(t.uses)) {
        const Tweak* used = FindTweak(id);
        sig += L'\x1D';
        sig += used ? SettingValue(*used) : L"";
    }
    return sig;
}

std::wstring Hash(const std::wstring& text) {
    unsigned long long h = 14695981039346656037ull;
    for (wchar_t c : text) {
        h = (h ^ (unsigned)c) * 1099511628211ull;
    }
    WCHAR buffer[20];
    swprintf_s(buffer, L"%016llx", h);
    return buffer;
}

// Fingerprints of what was last applied live in the mod's storage, one value
// per tweak.
std::wstring StoredHash(const Tweak& t) {
    WCHAR name[160], value[64] = L"";
    swprintf_s(name, L"tweaks.%ls", t.id);
    Wh_GetStringValue(name, value, ARRAYSIZE(value));
    return value;
}

void StoreHash(const Tweak& t, const std::wstring& hash) {
    WCHAR name[160];
    swprintf_s(name, L"tweaks.%ls", t.id);
    Wh_SetStringValue(name, hash.c_str());
}

// Each group has a "Use these tweaks" switch, off to begin with; General has
// none (its settings change nothing in Windows).
bool GroupOn(const Tweak& t) {
    std::wstring_view id = t.id;
    std::wstring group(id.substr(0, id.find(L'.')));
    if (group == L"general") {
        return true;
    }
    return Wh_GetIntSetting(L"%s.use", group.c_str()) != 0;
}

////////////////////////////////////////////////////////////////////////////////
// Text helpers

std::wstring Trim(std::wstring_view s) {
    size_t a = s.find_first_not_of(L" \t"), b = s.find_last_not_of(L" \t");
    return a == std::wstring_view::npos ? L"" : std::wstring(s.substr(a, b - a + 1));
}

void ReplaceAll(std::wstring& s, std::wstring_view from, std::wstring_view to) {
    for (size_t at = 0; (at = s.find(from, at)) != std::wstring::npos; at += to.size()) {
        s.replace(at, from.size(), to);
    }
}

std::vector<std::wstring> Split(std::wstring_view s, wchar_t sep) {
    std::vector<std::wstring> parts;
    size_t start = 0;
    for (size_t at; (at = s.find(sep, start)) != std::wstring_view::npos; start = at + 1) {
        parts.push_back(std::wstring(s.substr(start, at - start)));
    }
    parts.push_back(std::wstring(s.substr(start)));
    return parts;
}

std::wstring Expand(const std::wstring& s) {
    WCHAR buffer[2048];
    DWORD n = ExpandEnvironmentStringsW(s.c_str(), buffer, ARRAYSIZE(buffer));
    return n && n <= ARRAYSIZE(buffer) ? buffer : s;
}

// "#RRGGBB" to the ABGR DWORD Windows stores colours in.
std::wstring ToAbgr(const std::wstring& color) {
    std::wstring hex = Trim(color);
    if (!hex.empty() && hex[0] == L'#') {
        hex.erase(0, 1);
    }
    unsigned rgb = wcstoul(hex.c_str(), nullptr, 16);
    unsigned abgr = 0xFF000000u | ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb >> 16) & 0xFF);
    return std::to_wstring(abgr);
}

// "F,H,I" to the NoDrives bit mask.
std::wstring ToDriveMask(const std::wstring& letters) {
    unsigned mask = 0;
    for (wchar_t c : letters) {
        wchar_t up = (wchar_t)towupper(c);
        if (up >= L'A' && up <= L'Z') {
            mask |= 1u << (up - L'A');
        }
    }
    return std::to_wstring(mask);
}

std::wstring Substitute(std::wstring op, const std::wstring& value) {
    if (op.find(L'{') == std::wstring::npos) {
        return op;
    }
    size_t at = 0;
    while ((at = op.find(L"{@", at)) != std::wstring::npos) {
        size_t end = op.find(L'}', at);
        if (end == std::wstring::npos) {
            break;
        }
        const Tweak* used = FindTweak(std::wstring_view(op).substr(at + 2, end - at - 2));
        std::wstring text = used ? SettingValue(*used) : L"";
        op.replace(at, end - at + 1, text);
        at += text.size();
    }
    int number = _wtoi(value.c_str());
    ReplaceAll(op, L"{twips}", std::to_wstring(-15 * number));
    ReplaceAll(op, L"{ms}", std::to_wstring(number * 1000));
    ReplaceAll(op, L"{drives}", ToDriveMask(value));
    ReplaceAll(op, L"{abgr}", ToAbgr(value));
    ReplaceAll(op, L"{cmd}", _wcsicmp(value.c_str(), L"none") == 0 ? L"" : value);
    ReplaceAll(op, L"{}", value);
    return op;
}

////////////////////////////////////////////////////////////////////////////////
// Registry

struct RegOp {
    HKEY root = nullptr;  // HKCU, HKLM or HKU
    std::wstring rootName, path, name, type, data;
    char action = 0;      // 's' set, 'v' delete value, 'k' delete key, 'c' create
};

bool ParseRegOp(const std::wstring& line, RegOp& op) {
    size_t bar = line.find(L" | ");
    if (bar == std::wstring::npos) {
        return false;
    }
    std::wstring key = Trim(std::wstring_view(line).substr(0, bar));
    std::wstring spec = Trim(std::wstring_view(line).substr(bar + 3));
    struct {
        PCWSTR prefix;
        HKEY root;
        PCWSTR name;
    } roots[] = {
        {L"HKCU\\", HKEY_CURRENT_USER, L"HKEY_CURRENT_USER"},
        {L"HKLM\\", HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE"},
        {L"HKU\\", HKEY_USERS, L"HKEY_USERS"},
    };
    for (auto& r : roots) {
        if (key.rfind(r.prefix, 0) == 0) {
            op.root = r.root;
            op.rootName = r.name;
            op.path = key.substr(wcslen(r.prefix));
        }
    }
    if (!op.root) {
        return false;
    }
    if (spec == L"-") {
        op.action = 'k';
        return true;
    }
    if (spec == L"+") {
        op.action = 'c';
        return true;
    }
    size_t eq = spec.find(L" = ");
    if (eq != std::wstring::npos) {
        op.action = 's';
        op.name = spec.substr(0, eq);
        std::wstring value = spec.substr(eq + 3);
        size_t colon = value.find(L':');
        if (colon == std::wstring::npos) {
            return false;
        }
        op.type = value.substr(0, colon);
        op.data = value.substr(colon + 1);
        return true;
    }
    if (spec.size() > 2 && spec.compare(spec.size() - 2, 2, L" -") == 0) {
        op.action = 'v';
        op.name = spec.substr(0, spec.size() - 2);
        return true;
    }
    return false;
}

PCWSTR ValueName(const RegOp& op) {
    return op.name == L"@" ? nullptr : op.name.c_str();
}

std::vector<BYTE> ValueBytes(const RegOp& op, DWORD& type) {
    std::vector<BYTE> bytes;
    auto putString = [&](const std::wstring& s) {
        const BYTE* p = (const BYTE*)s.c_str();
        bytes.assign(p, p + (s.size() + 1) * sizeof(WCHAR));
    };
    if (op.type == L"dword") {
        type = REG_DWORD;
        DWORD v = (DWORD)wcstoul(op.data.c_str(), nullptr, 0);
        bytes.assign((BYTE*)&v, (BYTE*)&v + 4);
    } else if (op.type == L"qword") {
        type = REG_QWORD;
        unsigned long long v = wcstoull(op.data.c_str(), nullptr, 0);
        bytes.assign((BYTE*)&v, (BYTE*)&v + 8);
    } else if (op.type == L"expand") {
        type = REG_EXPAND_SZ;
        putString(op.data);
    } else if (op.type == L"multi") {
        type = REG_MULTI_SZ;
        std::wstring multi;
        for (const std::wstring& part : Split(op.data, L';')) {
            multi += part;
            multi += L'\0';
        }
        multi += L'\0';
        const BYTE* p = (const BYTE*)multi.data();
        bytes.assign(p, p + multi.size() * sizeof(WCHAR));
    } else {
        type = REG_SZ;
        putString(op.data);
    }
    return bytes;
}

bool ApplyUserRegOp(const RegOp& op) {
    HKEY key;
    switch (op.action) {
        case 'k':
            RegDeleteTreeW(op.root, op.path.c_str());
            RegDeleteKeyExW(op.root, op.path.c_str(), KEY_WOW64_64KEY, 0);
            return true;
        case 'c':
        case 's':
            if (RegCreateKeyExW(op.root, op.path.c_str(), 0, nullptr, 0,
                                KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &key,
                                nullptr) != ERROR_SUCCESS) {
                return false;
            }
            if (op.action == 's') {
                DWORD type;
                std::vector<BYTE> bytes = ValueBytes(op, type);
                RegSetValueExW(key, ValueName(op), 0, type, bytes.data(), (DWORD)bytes.size());
            }
            RegCloseKey(key);
            return true;
        case 'v':
            if (RegOpenKeyExW(op.root, op.path.c_str(), 0, KEY_SET_VALUE | KEY_WOW64_64KEY,
                              &key) == ERROR_SUCCESS) {
                RegDeleteValueW(key, ValueName(op));
                RegCloseKey(key);
            }
            return true;
    }
    return false;
}

std::wstring RegQuote(const std::wstring& s) {
    std::wstring out = L"\"";
    for (wchar_t c : s) {
        if (c == L'\\' || c == L'"') {
            out += L'\\';
        }
        out += c;
    }
    return out + L"\"";
}

// One op in .reg syntax, for the elevated step.
std::wstring RegFileLines(const RegOp& op) {
    std::wstring key = op.rootName + L"\\" + op.path;
    if (op.action == 'k') {
        return L"[-" + key + L"]\r\n";
    }
    std::wstring out = L"[" + key + L"]\r\n";
    if (op.action == 'c') {
        return out;
    }
    std::wstring name = op.name == L"@" ? L"@" : RegQuote(op.name);
    if (op.action == 'v') {
        return out + name + L"=-\r\n";
    }
    if (op.type == L"sz") {
        return out + name + L"=" + RegQuote(op.data) + L"\r\n";
    }
    DWORD type;
    std::vector<BYTE> bytes = ValueBytes(op, type);
    if (type == REG_DWORD) {
        WCHAR hex[16];
        swprintf_s(hex, L"%08x", *(DWORD*)bytes.data());
        return out + name + L"=dword:" + hex + L"\r\n";
    }
    std::wstring line = name + (type == REG_EXPAND_SZ   ? L"=hex(2):"
                                : type == REG_MULTI_SZ  ? L"=hex(7):"
                                : type == REG_QWORD     ? L"=hex(b):"
                                                        : L"=hex:");
    for (size_t i = 0; i < bytes.size(); i++) {
        WCHAR hex[4];
        swprintf_s(hex, i ? L",%02x" : L"%02x", bytes[i]);
        line += hex;
    }
    return out + line + L"\r\n";
}

std::vector<std::wstring> SubKeys(HKEY root, const std::wstring& path) {
    std::vector<std::wstring> names;
    HKEY key;
    if (RegOpenKeyExW(root, path.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS) {
        return names;
    }
    WCHAR name[256];
    for (DWORD i = 0;; i++) {
        DWORD len = ARRAYSIZE(name);
        if (RegEnumKeyExW(key, i, name, &len, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) {
            break;
        }
        names.push_back(name);
    }
    RegCloseKey(key);
    return names;
}

std::wstring ReadString(HKEY root, const std::wstring& path, PCWSTR name) {
    WCHAR buffer[1024] = L"";
    DWORD size = sizeof(buffer) - sizeof(WCHAR);
    RegGetValueW(root, path.c_str(), name, RRF_RT_REG_SZ | RRF_NOEXPAND,
                 nullptr, buffer, &size);
    return buffer;
}

////////////////////////////////////////////////////////////////////////////////
// One apply pass

struct Pass {
    // Machine-wide changes, per tweak, for the elevated step.
    std::map<size_t, std::wstring> regFiles;
    std::map<size_t, std::vector<std::wstring>> adminCommands;
    std::vector<std::wstring> notify;
    std::vector<std::wstring> afterRuns;
    std::map<std::wstring, std::wstring> metrics;
    bool shellChanged = false;
    bool iconsChanged = false;
    size_t current = 0;
};

void RunHidden(const std::wstring& command, DWORD waitMs) {
    std::wstring line = Expand(command);
    STARTUPINFOW si{sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi;
    if (CreateProcessW(nullptr, line.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                       nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, waitMs);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        Wh_Log(L"Couldn't run %s", line.c_str());
    }
}

void DoRegOp(Pass& pass, const RegOp& op) {
    if (op.root == HKEY_CURRENT_USER) {
        ApplyUserRegOp(op);
    } else {
        pass.regFiles[pass.current] += RegFileLines(op);
    }
    if (op.path.find(L"Classes") != std::wstring::npos ||
        op.path.find(L"Explorer") != std::wstring::npos) {
        pass.shellChanged = true;
    }
}

void DoOp(Pass& pass, const std::wstring& line);

void DoOps(Pass& pass, std::initializer_list<std::wstring> lines) {
    for (const std::wstring& line : lines) {
        DoOp(pass, line);
    }
}

// A blank icon for "none" overlays, kept in the mod's storage folder.
std::wstring BlankIconPath() {
    WCHAR folder[MAX_PATH];
    if (!Wh_GetModStoragePath(folder, ARRAYSIZE(folder))) {
        return L"";
    }
    std::wstring path = std::wstring(folder) + L"\\blank.ico";
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        // A 16x16 32-bit icon, every pixel transparent.
        std::vector<BYTE> ico(6 + 16 + 40 + 16 * 16 * 4 + 16 * 4, 0);
        BYTE* p = ico.data();
        p[2] = 1;
        p[4] = 1;
        p[6] = 16;
        p[7] = 16;
        p[10] = 1;
        p[12] = 32;
        DWORD size = (DWORD)ico.size() - 22, offset = 22;
        memcpy(p + 14, &size, 4);
        memcpy(p + 18, &offset, 4);
        BITMAPINFOHEADER bih{sizeof(bih), 16, 32, 1, 32};
        memcpy(p + 22, &bih, sizeof(bih));
        HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(file, ico.data(), (DWORD)ico.size(), &written, nullptr);
            CloseHandle(file);
        }
    }
    return path;
}

// Deterministic GUID for a folder the user listed, so the same row keeps the
// same identity across saves.
std::wstring FolderGuid(const std::wstring& parent, const std::wstring& name) {
    std::wstring h1 = Hash(L"tourne-folder:" + parent + L":" + name);
    std::wstring h2 = Hash(L"tourne-folder-2:" + name + L":" + parent);
    WCHAR guid[40];
    swprintf_s(guid, L"{%.8ls-%.4ls-4%.3ls-8%.3ls-%.12ls}", h1.c_str(), h1.c_str() + 8,
               h1.c_str() + 12, h2.c_str(), h2.c_str() + 3);
    return guid;
}

// Folders added to This PC or the navigation pane: the rows are the whole
// list, so entries added before (by the suite or by Winaero) are removed
// first, then the rows are written.
void ApplyFolders(Pass& pass, const std::wstring& parent, const std::wstring& list) {
    const std::wstring ns = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\" + parent +
                            L"\\NameSpace";
    for (const std::wstring& guid : SubKeys(HKEY_CURRENT_USER, ns)) {
        std::wstring by = ReadString(HKEY_CURRENT_USER, ns + L"\\" + guid, L"CreatedBy");
        std::wstring classBy = ReadString(HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" + guid, L"CreatedBy");
        if (by == L"Winaero Tweaker" || by == L"Tourne'Suite" || classBy == L"Winaero Tweaker" ||
            classBy == L"Tourne'Suite") {
            DoOps(pass, {L"HKCU\\" + ns + L"\\" + guid + L" | -",
                         L"HKCU\\Software\\Classes\\CLSID\\" + guid + L" | -",
                         L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel | " + guid + L" -"});
        }
    }
    for (const std::wstring& row : Split(list, L'|')) {
        std::vector<std::wstring> fields = Split(row, L'>');
        std::wstring name = Trim(fields[0]);
        std::wstring path = fields.size() > 1 ? Trim(fields[1]) : L"";
        std::wstring icon = fields.size() > 2 ? Trim(fields[2]) : L"";
        if (name.empty() || path.empty()) {
            continue;
        }
        if (icon.empty()) {
            icon = L"%SystemRoot%\\System32\\imageres.dll,-3";
        }
        std::wstring guid = FolderGuid(parent, name);
        std::wstring cls = L"HKCU\\Software\\Classes\\CLSID\\" + guid;
        DoOps(pass, {
            cls + L" | @ = sz:" + name,
            cls + L" | CreatedBy = sz:Tourne'Suite",
            cls + L" | InfoTip = sz:" + path,
            cls + L" | DescriptionID = dword:3",
            cls + L" | System.IsPinnedToNameSpaceTree = dword:1",
            cls + L"\\DefaultIcon | @ = expand:" + icon,
            cls + L"\\InProcServer32 | @ = expand:%SystemRoot%\\System32\\shell32.dll",
            cls + L"\\InProcServer32 | ThreadingModel = sz:Both",
            cls + L"\\Instance | CLSID = sz:{0E5AAE11-A475-4c5b-AB00-C66DE400274E}",
            cls + L"\\Instance\\InitPropertyBag | Attributes = dword:0x11",
            cls + L"\\Instance\\InitPropertyBag | TargetFolderPath = expand:" + path,
            cls + L"\\ShellFolder | Attributes = dword:0xF080004D",
            cls + L"\\ShellFolder | FolderValueFlags = dword:0x28",
            L"HKCU\\" + ns + L"\\" + guid + L" | @ = sz:" + name,
            L"HKCU\\" + ns + L"\\" + guid + L" | CreatedBy = sz:Tourne'Suite",
        });
        if (parent == L"Desktop") {
            DoOp(pass, L"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel | " + guid + L" = dword:1");
        }
    }
}

void DoOp(Pass& pass, const std::wstring& line) {
    std::wstring op = Trim(line);
    if (op.empty()) {
        return;
    }
    size_t space = op.find(L' ');
    std::wstring word = op.substr(0, space);
    std::wstring rest = space == std::wstring::npos ? L"" : Trim(std::wstring_view(op).substr(space + 1));

    if (word == L"run") {
        pass.afterRuns.push_back(rest);
    } else if (word == L"admin") {
        pass.adminCommands[pass.current].push_back(rest);
    } else if (word == L"notify") {
        pass.notify.push_back(rest);
    } else if (word == L"spi") {
        // spi ui|pv CODE VALUE: saved to the profile and broadcast.
        std::vector<std::wstring> parts = Words(rest.c_str());
        if (parts.size() == 3) {
            UINT code = wcstoul(parts[1].c_str(), nullptr, 0);
            UINT value = wcstoul(parts[2].c_str(), nullptr, 0);
            BOOL ok = parts[0] == L"ui"
                          ? SystemParametersInfoW(code, value, nullptr, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)
                          : SystemParametersInfoW(code, 0, (PVOID)(UINT_PTR)value, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
            if (!ok) {
                Wh_Log(L"SystemParametersInfo 0x%x failed: %u", code, GetLastError());
            }
        }
    } else if (word == L"metric" || word == L"font") {
        size_t at = rest.find(L' ');
        std::wstring field = rest.substr(0, at);
        pass.metrics[word + L"." + field] = at == std::wstring::npos ? L"" : Trim(std::wstring_view(rest).substr(at + 1));
    } else if (word == L"style") {
        using SetSystemVisualStyle_t = HRESULT(WINAPI*)(PCWSTR, PCWSTR, PCWSTR, UINT);
        HMODULE uxtheme = LoadLibraryW(L"uxtheme.dll");
        auto setStyle = uxtheme ? (SetSystemVisualStyle_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(65)) : nullptr;
        if (setStyle) {
            HRESULT hr = setStyle(Expand(rest).c_str(), L"NormalColor", L"NormalSize", 0);
            Wh_Log(L"Visual style %s: 0x%08x", rest.c_str(), hr);
        }
    } else if (word == L"accents") {
        // Up to eight colours for Settings > Colors.
        std::vector<std::wstring> colors = Split(rest, L',');
        for (int i = 0; i < 8; i++) {
            std::wstring key = L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Accents\\0\\Theme" + std::to_wstring(i);
            std::wstring color = i < (int)colors.size() ? Trim(colors[i]) : L"";
            DoOp(pass, color.empty() ? key + L" | -" : key + L" | Color = dword:" + ToAbgr(color));
        }
    } else if (word == L"pwsh") {
        // PowerShell 7 entries for the "Run or modify" menu.
        if (rest == L"1") {
            std::wstring base = L"HKCU\\Software\\Classes\\SystemFileAssociations\\.ps1\\Shell\\TourneRunModify\\shell\\";
            DoOps(pass, {
                base + L"1bpwsh | MUIVerb = sz:Run with PowerShell 7",
                base + L"1bpwsh\\command | @ = sz:pwsh.exe -NoProfile -ExecutionPolicy Bypass -NoExit -File \"%1\"",
                base + L"2bpwshadmin | MUIVerb = sz:Run with PowerShell 7 as administrator",
                base + L"2bpwshadmin | HasLUAShield = sz:",
                base + L"2bpwshadmin\\command | @ = sz:powershell.exe -NoProfile -WindowStyle Hidden -Command \"Start-Process pwsh.exe -Verb RunAs -ArgumentList '-NoProfile','-ExecutionPolicy','Bypass','-NoExit','-File','\\\"%1\\\"'\"",
            });
        }
    } else if (word == L"terminal") {
        // terminal CLASS MODE: a Windows Terminal submenu with the standard profiles.
        std::vector<std::wstring> parts = Words(rest.c_str());
        if (parts.size() == 2) {
            std::wstring key = L"HKCU\\Software\\Classes\\" + parts[0] + L"\\shell\\TourneTerminal";
            DoOps(pass, {
                key + L" | MUIVerb = sz:Open in Terminal",
                key + L" | SubCommands = sz:",
                key + L"\\shell\\1default | MUIVerb = sz:Default profile",
                key + L"\\shell\\1default\\command | @ = sz:wt.exe -d \"%V\"",
                key + L"\\shell\\2cmd | MUIVerb = sz:Command Prompt",
                key + L"\\shell\\2cmd\\command | @ = sz:wt.exe -d \"%V\" -p \"Command Prompt\"",
                key + L"\\shell\\3ps | MUIVerb = sz:Windows PowerShell",
                key + L"\\shell\\3ps\\command | @ = sz:wt.exe -d \"%V\" -p \"Windows PowerShell\"",
                key + L"\\shell\\4pwsh | MUIVerb = sz:PowerShell",
                key + L"\\shell\\4pwsh\\command | @ = sz:wt.exe -d \"%V\" -p \"PowerShell\"",
            });
            if (parts[1] == L"shift") {
                DoOp(pass, key + L" | Extended = sz:");
            }
        }
    } else if (word == L"shellicon") {
        // shellicon N [VALUE]: an Explorer overlay or stock icon.
        size_t at = rest.find(L' ');
        std::wstring index = rest.substr(0, at);
        std::wstring value = at == std::wstring::npos ? L"" : Trim(std::wstring_view(rest).substr(at + 1));
        std::wstring key = L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Icons | " + index;
        if (value.empty()) {
            DoOp(pass, key + L" -");
        } else if (_wcsicmp(value.c_str(), L"none") == 0) {
            DoOp(pass, key + L" = sz:" + BlankIconPath());
        } else if (_wcsicmp(value.c_str(), L"classic") == 0) {
            DoOp(pass, key + L" = expand:%SystemRoot%\\System32\\shell32.dll,-30");
        } else {
            DoOp(pass, key + L" = sz:" + value);
        }
        pass.iconsChanged = true;
    } else if (word == L"driveicons") {
        std::map<wchar_t, std::wstring> icons;
        for (const std::wstring& pair : Split(rest, L'|')) {
            size_t eq = pair.find(L'=');
            std::wstring letter = Trim(std::wstring_view(pair).substr(0, eq == std::wstring::npos ? 0 : eq));
            if (eq != std::wstring::npos && letter.size() == 1) {
                icons[(wchar_t)towupper(letter[0])] = Trim(std::wstring_view(pair).substr(eq + 1));
            }
        }
        for (wchar_t d = L'A'; d <= L'Z'; d++) {
            std::wstring key = L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DriveIcons\\" + std::wstring(1, d);
            auto it = icons.find(d);
            DoOp(pass, it == icons.end() ? key + L" | -" : key + L"\\DefaultIcon | @ = sz:" + it->second);
        }
        pass.iconsChanged = true;
    } else if (word == L"folders") {
        size_t at = rest.find(L' ');
        ApplyFolders(pass, rest.substr(0, at), at == std::wstring::npos ? L"" : rest.substr(at + 1));
    } else {
        RegOp reg;
        if (ParseRegOp(op, reg)) {
            DoRegOp(pass, reg);
        } else {
            Wh_Log(L"Can't read op: %s", op.c_str());
        }
    }
}

// The ops for a tweak's current value.
std::wstring OpsFor(const Tweak& t, const std::wstring& value) {
    switch (t.kind) {
        case 't':
            return value != L"0" ? t.a : t.b;
        case 'n':
            // keepEmpty on a number: 0 means off, leave the system alone.
            if (t.keepEmpty && value == L"0") {
                return L"";
            }
            return t.b && *t.b && t.windowsDefault && value == t.windowsDefault ? t.b : t.a;
        case 's':
            if (value.empty()) {
                return t.keepEmpty ? L"" : (t.b ? t.b : L"");
            }
            return t.a;
        case 'c':
            // Options are "key\036ops", separated by \037.
            for (const std::wstring& option : Split(t.a, L'\037')) {
                size_t sep = option.find(L'\036');
                if (sep != std::wstring::npos && option.compare(0, sep, value) == 0) {
                    return option.substr(sep + 1);
                }
            }
            return L"";
    }
    return L"";
}

////////////////////////////////////////////////////////////////////////////////
// Window metrics and fonts, applied live in one call each

void ParseFont(const std::wstring& spec, LOGFONTW& lf) {
    std::vector<std::wstring> parts = Split(spec, L',');
    std::wstring face = Trim(parts[0]);
    if (!face.empty()) {
        wcsncpy_s(lf.lfFaceName, face.c_str(), _TRUNCATE);
    }
    if (parts.size() > 1) {
        int size = _wtoi(Trim(parts[1]).c_str());
        if (size > 0) {
            lf.lfHeight = -size;
            lf.lfWidth = 0;
        }
    }
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = FALSE;
    for (size_t i = 2; i < parts.size(); i++) {
        std::wstring flag = Trim(parts[i]);
        if (_wcsicmp(flag.c_str(), L"bold") == 0) {
            lf.lfWeight = FW_BOLD;
        } else if (_wcsicmp(flag.c_str(), L"italic") == 0) {
            lf.lfItalic = TRUE;
        }
    }
}

void ApplyMetrics(const std::map<std::wstring, std::wstring>& changes) {
    if (changes.empty()) {
        return;
    }
    // At 96 DPI, which is what the settings are in.
    DPI_AWARENESS_CONTEXT old = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    NONCLIENTMETRICSW ncm{sizeof(ncm)};
    ICONMETRICSW im{sizeof(im)};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    SystemParametersInfoW(SPI_GETICONMETRICS, sizeof(im), &im, 0);
    bool nonClient = false, icons = false;
    for (const auto& [field, value] : changes) {
        int n = _wtoi(value.c_str());
        if (field == L"metric.iconSpacingH") {
            im.iHorzSpacing = n;
            icons = true;
        } else if (field == L"metric.iconSpacingV") {
            im.iVertSpacing = n;
            icons = true;
        } else if (field == L"font.icon") {
            ParseFont(value, im.lfFont);
            icons = true;
        } else if (field == L"metric.menuHeight") {
            ncm.iMenuHeight = ncm.iMenuWidth = n;
            nonClient = true;
        } else if (field == L"metric.scrollWidth") {
            ncm.iScrollWidth = n;
            nonClient = true;
        } else if (field == L"metric.scrollHeight") {
            ncm.iScrollHeight = n;
            nonClient = true;
        } else if (field == L"metric.captionHeight") {
            ncm.iCaptionHeight = ncm.iCaptionWidth = n;
            nonClient = true;
        } else if (field == L"metric.borderWidth") {
            ncm.iBorderWidth = n;
            nonClient = true;
        } else if (field == L"metric.borderPadding") {
            ncm.iPaddedBorderWidth = n;
            nonClient = true;
        } else if (field == L"font.caption") {
            ParseFont(value, ncm.lfCaptionFont);
            nonClient = true;
        } else if (field == L"font.menu") {
            ParseFont(value, ncm.lfMenuFont);
            nonClient = true;
        } else if (field == L"font.message") {
            ParseFont(value, ncm.lfMessageFont);
            nonClient = true;
        } else if (field == L"font.status") {
            ParseFont(value, ncm.lfStatusFont);
            nonClient = true;
        }
    }
    if (icons && !SystemParametersInfoW(SPI_SETICONMETRICS, sizeof(im), &im,
                                        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        Wh_Log(L"SPI_SETICONMETRICS failed: %u", GetLastError());
    }
    if (nonClient && !SystemParametersInfoW(SPI_SETNONCLIENTMETRICS, sizeof(ncm), &ncm,
                                            SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)) {
        Wh_Log(L"SPI_SETNONCLIENTMETRICS failed: %u", GetLastError());
    }
    SetThreadDpiAwarenessContext(old);
}

////////////////////////////////////////////////////////////////////////////////
// The elevated step

bool WriteFileText(const std::wstring& path, const std::string& bytes) {
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD written;
    bool ok = WriteFile(file, bytes.data(), (DWORD)bytes.size(), &written, nullptr);
    CloseHandle(file);
    return ok;
}

std::string Utf8(const std::wstring& s) {
    int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

std::atomic<bool> g_stopping;

// Runs every machine-wide change in one elevated cmd.exe, one UAC prompt.
// Returns the tweaks whose part failed; all of them if the prompt was
// declined.
std::vector<size_t> RunElevated(const Pass& pass) {
    std::vector<size_t> ids;
    for (const auto& [index, text] : pass.regFiles) {
        ids.push_back(index);
    }
    for (const auto& [index, commands] : pass.adminCommands) {
        if (std::find(ids.begin(), ids.end(), index) == ids.end()) {
            ids.push_back(index);
        }
    }
    if (ids.empty()) {
        return {};
    }

    WCHAR temp[MAX_PATH];
    GetTempPathW(ARRAYSIZE(temp), temp);
    WCHAR stamp[32];
    swprintf_s(stamp, L"TourneTweaks-%llu", GetTickCount64());
    std::wstring dir = std::wstring(temp) + stamp;
    CreateDirectoryW(dir.c_str(), nullptr);
    std::wstring log = dir + L"\\log.txt";

    std::wstring script = L"@echo off\r\nchcp 65001 >nul\r\n";
    for (size_t index : ids) {
        script += L"echo [" + std::to_wstring(index) + L"]>>\"" + log + L"\"\r\n";
        auto reg = pass.regFiles.find(index);
        if (reg != pass.regFiles.end()) {
            std::wstring file = dir + L"\\t" + std::to_wstring(index) + L".reg";
            std::wstring content = L"\xFEFFWindows Registry Editor Version 5.00\r\n\r\n" + reg->second;
            WriteFileText(file, std::string((const char*)content.data(), content.size() * sizeof(WCHAR)));
            script += L"reg.exe import \"" + file + L"\" >>\"" + log + L"\" 2>&1 || echo FAILED>>\"" + log + L"\"\r\n";
        }
        auto commands = pass.adminCommands.find(index);
        if (commands != pass.adminCommands.end()) {
            for (const std::wstring& command : commands->second) {
                script += command + L" >>\"" + log + L"\" 2>&1 || echo FAILED>>\"" + log + L"\"\r\n";
            }
        }
    }
    std::wstring scriptPath = dir + L"\\apply.cmd";
    WriteFileText(scriptPath, Utf8(script));

    std::wstring params = L"/d /c \"\"" + scriptPath + L"\"\"";
    SHELLEXECUTEINFOW sei{sizeof(sei)};
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    sei.lpVerb = L"runas";
    sei.lpFile = L"cmd.exe";
    sei.lpParameters = params.c_str();
    sei.nShow = SW_HIDE;
    if (!ShellExecuteExW(&sei) || !sei.hProcess) {
        Wh_Log(L"The admin step didn't run (%u); its changes apply when their settings change again",
               GetLastError());
        return ids;
    }
    // Polled, so unloading the suite never waits on a slow admin step.
    while (WaitForSingleObject(sei.hProcess, 200) == WAIT_TIMEOUT) {
        if (g_stopping) {
            CloseHandle(sei.hProcess);
            return ids;
        }
    }
    CloseHandle(sei.hProcess);

    // Which sections logged a failure.
    std::vector<size_t> failed;
    HANDLE file = CreateFileW(log.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        std::string text(GetFileSize(file, nullptr), '\0');
        DWORD read = 0;
        ReadFile(file, text.data(), (DWORD)text.size(), &read, nullptr);
        CloseHandle(file);
        long long section = -1;
        size_t start = 0;
        while (start < text.size()) {
            size_t end = text.find('\n', start);
            std::string line = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
            start = end == std::string::npos ? text.size() : end + 1;
            if (!line.empty() && line[0] == '[') {
                section = atoll(line.c_str() + 1);
                if (section < 0 || section >= (long long)ARRAYSIZE(kTweaks)) {
                    section = -1;
                }
            } else if (line.rfind("FAILED", 0) == 0 && section >= 0) {
                failed.push_back((size_t)section);
                Wh_Log(L"%s: the admin step reported a failure", kTweaks[section].id);
            } else if (!line.empty() && line != "\r" && section >= 0) {
                Wh_Log(L"%s: %hs", kTweaks[section].id, line.c_str());
            }
        }
    }
    return failed;
}

////////////////////////////////////////////////////////////////////////////////
// Explorer

void RestartExplorer() {
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    DWORD pid = 0;
    if (!tray || !GetWindowThreadProcessId(tray, &pid)) {
        return;
    }
    HANDLE process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!process) {
        return;
    }
    TerminateProcess(process, 1);
    WaitForSingleObject(process, 5000);
    CloseHandle(process);
    // Windows restarts the shell by itself; start it if it hasn't.
    for (int i = 0; i < 30 && !FindWindowW(L"Shell_TrayWnd", nullptr); i++) {
        Sleep(100);
    }
    if (!FindWindowW(L"Shell_TrayWnd", nullptr)) {
        RunHidden(L"%SystemRoot%\\explorer.exe", 0);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Applying

// Asks before applying: always at start (nothing the user just did), and on
// a Save that changes more than a few tweaks at once. Yes applies them; No
// leaves Windows as it is and takes the settings as they are, so they're
// only applied when changed again; Cancel (or the suite stopping meanwhile)
// leaves them pending.
constexpr size_t kAskAbove = 5;
bool g_askAgain;  // the last box was answered Cancel (only the apply thread uses it)

BOOL CALLBACK CancelWindow(HWND wnd, LPARAM) {
    PostMessageW(wnd, WM_COMMAND, IDCANCEL, 0);
    return TRUE;
}

// Runs inside the message box's loop: closes it when the suite stops.
VOID CALLBACK CloseAskOnStop(HWND, UINT, UINT_PTR, DWORD) {
    if (g_stopping) {
        EnumThreadWindows(GetCurrentThreadId(), CancelWindow, 0);
    }
}

int AskToApply(const std::vector<std::wstring>& lines, bool atStart) {
    std::wstring text = atStart ? L"These Tweaks settings were changed while the suite wasn't running, and aren't applied yet:\n\n"
                                : L"This Save changes " + std::to_wstring(lines.size()) + L" Tweaks settings:\n\n";
    for (size_t i = 0; i < lines.size(); i++) {
        if (i == 15) {
            text += L"and " + std::to_wstring(lines.size() - i) + L" more\n";
            break;
        }
        text += L"  \x2022 " + lines[i] + L"\n";
    }
    text += L"\nApply them to Windows now?\n\nNo leaves Windows as it is: these settings are then only applied once you change them again. Cancel decides later: they come up again at the next start or Save.";
    UINT_PTR timer = SetTimer(nullptr, 0, 200, CloseAskOnStop);
    int answer = MessageBoxW(nullptr, text.c_str(), L"Tourne'Suite Tweaks",
                             MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION | MB_TOPMOST | MB_SETFOREGROUND);
    KillTimer(nullptr, timer);
    return g_stopping ? IDCANCEL : answer;
}

// Applies every tweak whose setting changed since it was last applied, in
// the groups switched on. Nothing is applied the first time a tweak is seen
// (its setting may not match Windows yet), nor when its group is switched
// on: both only note how it's set. A group switched off forgets its tweaks,
// so switching it back on starts afresh. At start, tweaks that need admin
// wait for a Save, so the admin prompt only ever follows something the user
// did; whatever its outcome, nothing is retried until changed again.
void ApplyChanges(bool atStart) {
    Pass pass;
    struct Pending {
        size_t index;
        std::wstring hash;
        std::wstring value;
    };
    std::vector<Pending> pending;
    bool explorer = false;

    for (size_t i = 0; i < ARRAYSIZE(kTweaks); i++) {
        const Tweak& t = kTweaks[i];
        std::wstring stored = StoredHash(t);
        if (!GroupOn(t)) {
            if (!stored.empty()) {
                StoreHash(t, L"");
            }
            continue;
        }
        std::wstring hash = Hash(Signature(t));
        if (stored.empty()) {
            StoreHash(t, hash);
            continue;
        }
        if (stored == hash) {
            continue;
        }
        if (atStart && t.admin) {
            Wh_Log(L"%s changed while the suite wasn't running; it needs admin, so it waits for a Save", t.id);
            continue;
        }
        pending.push_back({i, hash, SettingValue(t)});
    }
    if (pending.empty()) {
        return;
    }

    // Tweaks put off with Cancel are asked about again, however few.
    if (atStart || g_askAgain || pending.size() > kAskAbove) {
        std::vector<std::wstring> lines;
        for (const Pending& p : pending) {
            const Tweak& t = kTweaks[p.index];
            std::wstring value = t.kind == 't' ? (p.value == L"0" ? L"off" : L"on")
                                 : p.value.empty() ? L"(empty)" : p.value;
            lines.push_back(std::wstring(t.title) + L": " + value);
        }
        int answer = AskToApply(lines, atStart);
        if (answer == IDCANCEL) {
            Wh_Log(L"Applying %zu tweaks put off until the next Save", pending.size());
            g_askAgain = true;
            return;
        }
        g_askAgain = false;
        if (answer != IDYES) {
            Wh_Log(L"Not applying %zu tweaks; taken as they are", pending.size());
            for (const Pending& p : pending) {
                StoreHash(kTweaks[p.index], p.hash);
            }
            return;
        }
    }

    for (const Pending& p : pending) {
        const Tweak& t = kTweaks[p.index];
        Wh_Log(L"Applying %s = %s", t.id, p.value.c_str());
        pass.current = p.index;
        for (const std::wstring& line : Split(OpsFor(t, p.value), L'\n')) {
            DoOp(pass, Substitute(line, p.value));
        }
        explorer |= t.after == 'e';
    }

    ApplyMetrics(pass.metrics);
    // A declined prompt or a failed admin step is logged, not retried: the
    // tweak is taken as applied until its setting changes again.
    std::vector<size_t> failed = RunElevated(pass);
    for (const Pending& p : pending) {
        // Cut short by the suite stopping: whether the admin part ran isn't
        // known, so those tweaks stay pending.
        if (g_stopping && kTweaks[p.index].admin) {
            continue;
        }
        if (std::find(failed.begin(), failed.end(), p.index) != failed.end()) {
            Wh_Log(L"%s: the admin part didn't apply; change the setting again to retry", kTweaks[p.index].id);
        }
        StoreHash(kTweaks[p.index], p.hash);
    }

    for (const std::wstring& command : pass.afterRuns) {
        RunHidden(command, 30000);
    }
    if (pass.iconsChanged) {
        RunHidden(L"%SystemRoot%\\System32\\ie4uinit.exe -show", 30000);
    }
    std::sort(pass.notify.begin(), pass.notify.end());
    pass.notify.erase(std::unique(pass.notify.begin(), pass.notify.end()), pass.notify.end());
    for (const std::wstring& area : pass.notify) {
        DWORD_PTR result;
        SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)area.c_str(),
                            SMTO_ABORTIFHUNG, 2000, &result);
    }
    if (pass.shellChanged || pass.iconsChanged) {
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }
    if (explorer && Wh_GetIntSetting(L"general.restartExplorer")) {
        RestartExplorer();
    }
}

// Applying runs on its own thread: the admin prompt can wait on the user.
HANDLE g_applyThread;
std::atomic<bool> g_applyAgain;
std::atomic<bool> g_applyRunning;
std::atomic<bool> g_applyAtStart;

DWORD WINAPI ApplyThread(LPVOID) {
    for (;;) {
        do {
            g_applyAgain = false;
            ApplyChanges(g_applyAtStart.exchange(false));  // (a pass asked for meanwhile is a Save's)
        } while (g_applyAgain && !g_stopping);
        g_applyRunning = false;
        // A Save that came in just as this pass ended: run it, unless Apply
        // has already started a thread for it.
        if (!g_applyAgain || g_stopping || g_applyRunning.exchange(true)) {
            return 0;
        }
    }
}

// Called in the launcher at start (atStart) and after each settings save.
void Apply(bool atStart) {
    g_applyAgain = true;  // (before the check, so a pass that is just ending sees it)
    if (g_applyRunning.exchange(true)) {
        return;
    }
    g_applyAtStart = atStart;
    if (g_applyThread) {
        CloseHandle(g_applyThread);
    }
    g_applyThread = CreateThread(nullptr, 0, ApplyThread, nullptr, 0, nullptr);
    if (!g_applyThread) {
        g_applyRunning = false;
    }
}

// The thread can be inside the UAC prompt, which only the user ends; give
// them a while before Windhawk unloads the code under it.
void Stop() {
    g_stopping = true;
    if (g_applyThread) {
        WaitForSingleObject(g_applyThread, 120000);
        CloseHandle(g_applyThread);
        g_applyThread = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Live system font: swaps Segoe UI for the chosen font in programs that
// create fonts through GDI, so the change shows before signing out.

WCHAR g_liveFont[LF_FACESIZE];

using CreateFontIndirectExW_t = HFONT(WINAPI*)(const ENUMLOGFONTEXDVW*);
CreateFontIndirectExW_t CreateFontIndirectExW_Original;

bool IsSegoeUi(PCWSTR face) {
    return _wcsicmp(face, L"Segoe UI") == 0 || _wcsnicmp(face, L"Segoe UI Variable", 17) == 0;
}

HFONT WINAPI CreateFontIndirectExW_Hook(const ENUMLOGFONTEXDVW* font) {
    if (font && IsSegoeUi(font->elfEnumLogfontEx.elfLogFont.lfFaceName)) {
        ENUMLOGFONTEXDVW copy = *font;
        wcsncpy_s(copy.elfEnumLogfontEx.elfLogFont.lfFaceName, g_liveFont, _TRUNCATE);
        return CreateFontIndirectExW_Original(&copy);
    }
    return CreateFontIndirectExW_Original(font);
}

std::wstring LiveFontSetting() {
    if (!Wh_GetIntSetting(L"general.liveFontSwap")) {
        return L"";
    }
    PCWSTR font = Wh_GetStringSetting(L"metrics.systemFont");
    std::wstring face = font ? Trim(font) : L"";
    Wh_FreeStringSetting(font);
    return face;
}

// Everywhere but the launcher, the part only exists for the live font.
bool ShouldLoadInThisProcess() {
    return !LiveFontSetting().empty();
}

bool NeedsReload() {
    return LiveFontSetting() != g_liveFont;
}

BOOL Wh_ModInit() {
    std::wstring face = LiveFontSetting();
    if (face.empty()) {
        return TRUE;
    }
    wcsncpy_s(g_liveFont, face.c_str(), _TRUNCATE);
    HMODULE gdi = GetModuleHandleW(L"gdi32full.dll");
    if (!gdi) {
        gdi = LoadLibraryW(L"gdi32.dll");
    }
    void* target = gdi ? (void*)GetProcAddress(gdi, "CreateFontIndirectExW") : nullptr;
    if (!target) {
        return FALSE;
    }
    return Wh_SetFunctionHook(target, (void*)CreateFontIndirectExW_Hook,
                              (void**)&CreateFontIndirectExW_Original);
}

void Wh_ModUninit() {}

void Wh_ModSettingsChanged() {}
