// App Styler: see app-styler.h.

#include "app-styler.h"

#include <aclapi.h>
#include <sddl.h>

#include <algorithm>
#include <cstdarg>
#include <cwctype>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#include "td.h"

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"app styler", format, args);
    va_end(args);
}

std::wstring Text(const wchar_t* key) {
    const wchar_t* v = GetString(L"appStyler", key);
    std::wstring out = v ? v : L"";
    FreeString(v);
    return out;
}

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

struct Config {
    bool on = false;
    std::set<std::wstring> programs;  // lower case
    std::wstring init;                // what the styler is handed: "ground=#..;...;square=1;font=..."
};

std::mutex g_lock;
Config g_cfg;
std::set<DWORD> g_done;  // processes looked at (styled or not), so each is looked at once
HWINEVENTHOOK g_hook = nullptr;
DWORD g_thread = 0;
std::wstring g_attach;  // tourne-appstyler-attach.exe, beside this exe

// A role's colour: the setting's own when it has one, else the Tourne theme's (HKCU\Software\Tourne\Theme).
std::wstring Colour(const wchar_t* setting, const wchar_t* role) {
    std::wstring own = Text(setting);
    size_t hash = own.find(L'#');
    if (hash != std::wstring::npos && hash + 7 <= own.size()) return own.substr(hash, 7);
    DWORD v = 0, size = sizeof(v);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Tourne\\Theme", role, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS) return L"";
    wchar_t hex[8];
    swprintf(hex, 8, L"#%06X", (unsigned)(v & 0xFFFFFF));
    return hex;
}

Config ReadConfig() {
    Config c;
    c.on = GetInt(L"appStyler", L"enabled") != 0;
    std::wstring list = Text(L"programs");
    size_t at = 0;
    while (at <= list.size()) {
        size_t comma = list.find(L',', at);
        std::wstring item = list.substr(at, comma == std::wstring::npos ? std::wstring::npos : comma - at);
        item.erase(0, item.find_first_not_of(L" \t"));
        item.erase(item.find_last_not_of(L" \t") + 1);
        if (!item.empty()) c.programs.insert(Lower(item));
        if (comma == std::wstring::npos) break;
        at = comma + 1;
    }
    struct { const wchar_t* name; const wchar_t* setting; const wchar_t* role; } const roles[] = {
        {L"ground", L"ground", L"Ground"}, {L"raised", L"raised", L"Raised"}, {L"text", L"text", L"Text"},
        {L"muted", L"muted", L"Muted"},    {L"accent", L"accent", L"Accent"}, {L"line", L"line", L"Line"}};
    for (auto const& r : roles)
        if (std::wstring hex = Colour(r.setting, r.role); !hex.empty()) c.init += std::wstring(r.name) + L"=" + hex + L";";
    if (GetInt(L"appStyler", L"square")) c.init += L"square=1;";
    if (Lower(Text(L"icons")) == L"hide") c.init += L"icons=hide;";
    if (std::wstring font = Text(L"font"); !font.empty() && Lower(font) != L"windows") {
        c.init += L"font=" + font + L";";
        // Pixel fonts' own sizes, which text snaps to so it stays sharp.
        std::wstring f = Lower(font);
        if (f.rfind(L"cozette", 0) == 0) c.init += L"fontGrid=13;";
        else if (f.rfind(L"scientifica", 0) == 0) c.init += L"fontGrid=11;";
    }
    return c;
}

std::wstring ProgramOf(DWORD pid) {
    std::wstring name;
    if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
        wchar_t path[MAX_PATH];
        DWORD n = MAX_PATH;
        if (QueryFullProcessImageNameW(p, 0, path, &n)) name = wcsrchr(path, L'\\') ? wcsrchr(path, L'\\') + 1 : path;
        CloseHandle(p);
    }
    return Lower(name);
}

// The attach, on a thread of its own (it waits for the app's XAML to be up).
void Attach(DWORD pid, std::wstring const& program, std::wstring const& init) {
    std::thread([pid, program, init] {
        std::wstring cmd = L"\"" + g_attach + L"\" " + std::to_wstring(pid) + L" \"" + init + L"\"";
        STARTUPINFOW si{sizeof(si)};
        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(g_attach.c_str(), cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            Log(L"couldn't start tourne-appstyler-attach.exe (%lu)", GetLastError());
            return;
        }
        WaitForSingleObject(pi.hProcess, 20000);
        DWORD code = 1;
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Log(code == 0 ? L"%s (%lu) styled" : L"%s (%lu) couldn't be styled", program.c_str(), pid);
    }).detach();
}

// A window of a program not looked at yet: styled if it's listed.
void Consider(HWND w) {
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    if (!pid || pid == GetCurrentProcessId()) return;
    std::wstring init;
    {
        std::lock_guard<std::mutex> lock(g_lock);
        if (!g_cfg.on || g_done.count(pid)) return;
        g_done.insert(pid);
        if (g_done.size() > 4096) g_done.clear();  // (old ones long gone: pids come round again)
        init = g_cfg.init;
    }
    std::wstring program = ProgramOf(pid);
    bool listed;
    {
        std::lock_guard<std::mutex> lock(g_lock);
        listed = g_cfg.programs.count(program) != 0;
    }
    if (listed) Attach(pid, program, init);
}

void CALLBACK Shown(HWINEVENTHOOK, DWORD, HWND w, LONG object, LONG child, DWORD, DWORD) {
    if (object != OBJID_WINDOW || child != CHILDID_SELF || !w || GetAncestor(w, GA_ROOT) != w) return;
    Consider(w);
}

// Packaged apps (Windows Security, Calculator) run in an app container: the DLL is opened up to them, read and run.
void OpenToApps(std::wstring const& file) {
    PSID apps = nullptr;
    if (!ConvertStringSidToSidW(L"S-1-15-2-1", &apps)) return;  // ALL APPLICATION PACKAGES
    PACL old = nullptr, made = nullptr;
    PSECURITY_DESCRIPTOR sd = nullptr;
    if (GetNamedSecurityInfoW(file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &old, nullptr, &sd) == ERROR_SUCCESS) {
        EXPLICIT_ACCESSW ea{};
        ea.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
        ea.grfAccessMode = GRANT_ACCESS;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName = (LPWSTR)apps;
        if (SetEntriesInAclW(1, &ea, old, &made) == ERROR_SUCCESS)
            SetNamedSecurityInfoW((LPWSTR)file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, made, nullptr);
        if (made) LocalFree(made);
        LocalFree(sd);
    }
    LocalFree(apps);
}

}  // namespace

void AppStylerDefaults(Settings& out) {
    out[L"appStyler.enabled"] = L"0";
    out[L"appStyler.programs"] = L"SystemSettings.exe";
    out[L"appStyler.square"] = L"1";
    out[L"appStyler.font"] = L"CozetteVector";
    out[L"appStyler.icons"] = L"hide";
    for (const wchar_t* role : {L"ground", L"raised", L"text", L"muted", L"accent", L"line"}) out[std::wstring(L"appStyler.") + role] = L"";
}

void AppStylerDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"appStyler", L"App styler",
        L"Windows' own apps (Settings first) in the Tourne theme's colours, Tourne's font and square corners. Each is styled "
        L"as it opens, through Windows' XAML diagnostics: nothing is hooked, and closing the app puts it back.");
    add(L"appStyler.enabled", L"App styler", L"On: the programs below are styled as they open.").isSwitch = true;
    add(L"appStyler.programs", L"Programs",
        L"Their file names, with commas: SystemSettings.exe (Settings); SecHealthUI.exe, CalculatorApp.exe and others are "
        L"still being tried.");
    add(L"appStyler.square", L"Square corners", L"Every corner in them square, as Tourne's are.").isSwitch = true;
    auto& icons = add(L"appStyler.icons", L"The apps' icons",
                      L"Their own coloured pictures (the nav pane's, device pictures): hidden, or kept. Your picture stays.");
    icons.options = {{L"hide", L"Hidden"}, {L"keep", L"Kept"}};
    auto& font = add(L"appStyler.font", L"Font", L"The text's font (icons keep theirs); Windows: their own.");
    font.options = {{L"CozetteVector", L"Cozette"}, {L"scientifica", L"scientifica"}, {L"Delicatus", L"Delicatus"}, {L"windows", L"Windows' own"}};
    for (auto [role, name] : {std::pair{L"ground", L"Ground"}, {L"raised", L"Cards and panels"}, {L"text", L"Text"},
                              {L"muted", L"Secondary text"}, {L"accent", L"Accent"}, {L"line", L"Lines"}})
        add(std::wstring(L"appStyler.") + role, name, L"#RRGGBB, or empty for the Tourne theme's.");
}

void AppStylerStart(HWND) {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring dir = exe;
    dir = dir.substr(0, dir.find_last_of(L'\\') + 1);
    g_attach = dir + L"tourne-appstyler-attach.exe";
    {
        std::lock_guard<std::mutex> lock(g_lock);
        g_cfg = ReadConfig();
    }
    OpenToApps(dir + L"TourneAppStyler.dll");
    std::thread([] {
        g_thread = GetCurrentThreadId();
        g_hook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr, Shown, 0, 0, WINEVENT_OUTOFCONTEXT);
        // The listed apps already open when this starts.
        EnumWindows([](HWND w, LPARAM) -> BOOL {
            if (IsWindowVisible(w)) Consider(w);
            return TRUE;
        }, 0);
        MSG m;
        while (GetMessageW(&m, nullptr, 0, 0) > 0) DispatchMessageW(&m);
        if (g_hook) UnhookWinEvent(g_hook);
        g_hook = nullptr;
    }).detach();
    Log(g_cfg.on ? L"on" : L"off (appStyler.enabled)");
}

void AppStylerApply() {
    std::lock_guard<std::mutex> lock(g_lock);
    g_cfg = ReadConfig();
}

void AppStylerStop() {
    if (g_thread) PostThreadMessageW(g_thread, WM_QUIT, 0, 0);
    g_thread = 0;
}

}  // namespace td
