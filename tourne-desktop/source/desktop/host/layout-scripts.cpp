// Layouts people write themselves (layout-scripts.h): a .luau file handed the
// screen and the windows, answering with where each window goes.
//
// This file is the Windows half: the folder, the files and their write times,
// the examples, and turning the tiling's rectangles into the plain boxes the
// script sees and back. The Luau half, the VM and its limits and the table the
// script is handed, is layout-script-core.inl, which has no Windows in it so
// that tools\layouts\layouttest.cpp runs the real thing.
//
// A script is compiled the first time it is asked for and kept, with its VM, so
// a call is one lua_pcall. The file's write time is looked at each call, so
// saving the file is all it takes to see the change: the next tiling uses it.
#include "layout-scripts.h"

#include <shlobj.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstring>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "td.h"

namespace td {
namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"layouts", format, args);
    va_end(args);
}

#include "layout-script-core.inl"
#include "layout-examples.inl"

struct Script {
    LsVm vm;
    FILETIME written{};  // the file when it was compiled
    bool said = false;   // its problem is in the log already
};

std::mutex g_lock;
std::map<std::wstring, Script> g_scripts;  // by file name, lower case, without .luau

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

std::string Utf8(std::wstring const& s) {
    if (s.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

std::wstring Wide(std::string const& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}

// The name as a file in the folder, or empty when it isn't one of its .luau
// files (a name with a path in it, or one that climbs out of the folder).
std::wstring FileFor(std::wstring const& name) {
    std::wstring bare = name;
    if (bare.size() > 5 && Lower(bare.substr(bare.size() - 5)) == L".luau") bare.resize(bare.size() - 5);
    if (bare.empty() || bare.find_first_of(L"\\/:*?\"<>|") != std::wstring::npos) return {};
    if (bare.find(L"..") != std::wstring::npos) return {};
    std::wstring folder = LayoutScriptsFolder();
    return folder.empty() ? std::wstring() : folder + L"\\" + bare + L".luau";
}

bool WriteTime(std::wstring const& file, FILETIME* out) {
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &data)) return false;
    *out = data.ftLastWriteTime;
    return true;
}

bool ReadAll(std::wstring const& file, std::string* out) {
    HANDLE h = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    bool ok = GetFileSizeEx(h, &size) && size.QuadPart < (1 << 20);
    if (ok) {
        out->resize((size_t)size.QuadPart);
        DWORD read = 0;
        ok = ReadFile(h, out->data(), (DWORD)out->size(), &read, nullptr) && read == out->size();
    }
    CloseHandle(h);
    return ok;
}

LsBox BoxOf(RECT const& r) {
    return LsBox{(double)r.left, (double)r.top, (double)(r.right - r.left), (double)(r.bottom - r.top)};
}

// Back to whole pixels, and inside the screen after the rounding too.
RECT RectOf(LsBox const& box, RECT screen) {
    LONG left = std::clamp((LONG)std::lround(box.x), screen.left, screen.right - 1);
    LONG top = std::clamp((LONG)std::lround(box.y), screen.top, screen.bottom - 1);
    LONG right = std::clamp((LONG)std::lround(box.x + box.w), left + 1, screen.right);
    LONG bottom = std::clamp((LONG)std::lround(box.y + box.h), top + 1, screen.bottom);
    return RECT{left, top, right, bottom};
}

}  // namespace

std::wstring LayoutScriptsFolder() {
    static std::wstring folder = [] {
        wchar_t test[MAX_PATH];  // (the console check lays them out somewhere else)
        std::wstring root;
        if (GetEnvironmentVariableW(L"TOURNE_LAYOUTS_DIR", test, MAX_PATH)) {
            root = test;
        } else {
            PWSTR local = nullptr;
            if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local)))
                root = std::wstring(local) + L"\\Tourne\\Layouts";
            CoTaskMemFree(local);
        }
        if (root.empty()) return root;
        std::error_code e;
        bool fresh = !std::filesystem::exists(root, e);
        std::filesystem::create_directories(root, e);
        if (fresh)
            for (LsExample const& x : kLsExamples) {
                std::wstring file = root + L"\\" + Wide(x.name);
                HANDLE h = CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                                       nullptr);
                if (h == INVALID_HANDLE_VALUE) continue;
                DWORD written = 0;
                WriteFile(h, x.text, (DWORD)strlen(x.text), &written, nullptr);
                CloseHandle(h);
            }
        return root;
    }();
    return folder;
}

std::vector<std::wstring> LayoutScriptNames() {
    std::vector<std::wstring> names;
    std::wstring folder = LayoutScriptsFolder();
    if (folder.empty()) return names;
    std::error_code e;
    for (auto const& entry : std::filesystem::directory_iterator(folder, e)) {
        std::wstring name = entry.path().filename().wstring();
        if (name.size() > 5 && Lower(name.substr(name.size() - 5)) == L".luau")
            names.push_back(name.substr(0, name.size() - 5));
    }
    std::sort(names.begin(), names.end(), [](auto const& a, auto const& b) { return Lower(a) < Lower(b); });
    return names;
}

std::wstring LayoutScriptProblem(std::wstring const& script) {
    std::lock_guard<std::mutex> hold(g_lock);
    auto it = g_scripts.find(Lower(script));
    return it == g_scripts.end() ? std::wstring() : Wide(it->second.vm.problem);
}

std::vector<RECT> LayoutScriptRects(LayoutScriptAsk const& ask) {
    if (ask.windows.empty() || ask.windows.size() > kLsMostWindows) return {};
    std::wstring file = FileFor(ask.script);
    if (file.empty()) return {};
    std::lock_guard<std::mutex> hold(g_lock);
    Script& s = g_scripts[Lower(ask.script)];
    FILETIME now{};
    if (!WriteTime(file, &now)) {
        if (s.vm.problem.empty()) s.vm.problem = "isn't in the Layouts folder";
    } else if (!s.vm.L || CompareFileTime(&now, &s.written) != 0) {
        s.written = now;
        s.said = false;
        std::string source;
        if (!ReadAll(file, &source)) {
            LsClose(s.vm);
            s.vm.problem = "couldn't be read";
        } else {
            LsCompile(s.vm, source, Utf8(file.substr(file.rfind(L'\\') + 1)));
        }
    }
    LsAsk inner;
    inner.screen = BoxOf(ask.screen);
    inner.gap = ask.gap;
    inner.ratio = ask.ratio;
    inner.options = Utf8(ask.options);
    for (LayoutScriptWindow const& w : ask.windows)
        inner.windows.push_back(
            LsWindow{BoxOf(w.rect), Utf8(w.program), Utf8(w.cls), Utf8(w.title), w.focused});
    std::vector<LsBox> boxes;
    std::vector<RECT> out;
    if (LsRun(s.vm, inner, &boxes)) {
        s.said = false;
        for (LsBox const& box : boxes) out.push_back(RectOf(box, ask.screen));
    } else if (!s.vm.problem.empty() && !s.said) {
        s.said = true;
        Log(L"%s %s. Laying the windows out in columns instead.", ask.script.c_str(),
            Wide(s.vm.problem).c_str());
    }
    return out;
}

}  // namespace td
