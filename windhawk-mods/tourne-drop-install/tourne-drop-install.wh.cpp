// ==WindhawkMod==
// @id              tourne-drop-install
// @name            Tourne Drop Install
// @description     Drop .wh.cpp files on a small window and they're compiled and installed as mods, new ones turned off
// @version         1.0.0
// @author          USER-TOURNE
// @github          https://github.com/USER-TOURNE
// @donateUrl       https://ko-fi.com/tourne
// @include         windhawk-mod-elevated.exe
// @include         windhawk-mod.exe
// @compilerOptions -luser32 -lshell32 -lcomdlg32 -lgdi32 -ladvapi32
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tourne Drop Install

Install a mod from its `.wh.cpp` file by dropping it on a window, instead of
Create a new mod, turning off Enable mod, pasting the code in, Compile mod and
closing the editor.

## How to use it

1. Click **Tourne Drop Install** in the tray (a green arrow). A small window
   opens.
2. Drag one or more `.wh.cpp` files (or a folder holding them) onto it, or
   click **Choose files**.
3. Each one is compiled and installed, one after another, and the window
   says how it went (with the compiler's messages when one doesn't compile).

A mod you didn't have before is installed **turned off**, as Enable mod off
in the editor does; turn it on in Windhawk when you're ready (or turn on
**Turn new mods on** in this mod's settings). A mod you already had is
updated in place and keeps its settings and its on or off state.

They're installed as your own mods (as Create a new mod makes them), so
they're under Installed mods with the rest, and you can open them in the
editor as usual.

## Good to know

- It needs Windhawk 2, which comes with the `windhawk-cli.exe` it uses to do
  the work (the same steps the editor's Compile mod takes).
- Windhawk runs it in a process of its own, as administrator when it can,
  since installing a mod writes to Windhawk's own folders. If it can't, each
  install asks for administrator once (the usual Windows prompt).
- Nothing runs while the window is closed and nothing is dropped.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- trayIcon: true
  $name: Tourne Drop Install in the tray
  $description: >-
    Click it to open the drop window. Off: the window opens when Windhawk
    starts instead.
- turnOnNewMods: false
  $name: Turn new mods on
  $description: >-
    Off: a mod you didn't have before is installed turned off, as the editor
    does with Enable mod off. A mod you already had keeps its state either
    way.
- keepOnTop: false
  $name: Keep the window on top
  $description: Keeps the drop window above other windows while it's open.
*/
// ==/WindhawkModSettings==

// How it works: Windhawk 2 starts a host process (windhawk-mod-elevated.exe,
// or windhawk-mod.exe when it can't elevate) for a mod that names one in its
// includes, and the mod keeps it: the host's own entry point is stopped and
// the mod's window thread runs there. Each dropped file goes to Windhawk's
// own command line tool, which compiles and installs it the way the editor
// does:
//   windhawk-cli.exe mod install --file <path> [--disabled]

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>

#include <cmath>
#include <deque>
#include <string>
#include <vector>

namespace {

constexpr WCHAR kClass[] = L"TourneDropInstall";
constexpr WCHAR kTitle[] = L"Tourne Drop Install";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kSettingsMessage = WM_APP + 2;
constexpr UINT kLogMessage = WM_APP + 3;  // lParam: a new std::wstring to add.
constexpr UINT kQuitMessage = WM_APP + 4;
constexpr UINT kTrayId = 1;
constexpr int kChooseId = 101, kLogId = 102, kDropId = 103;
constexpr int kMenuOpen = 1, kMenuClose = 2;

HWND g_window, g_drop, g_choose, g_log;
HANDLE g_thread;
HICON g_icon;
HFONT g_font, g_bigFont;
UINT g_taskbarCreated;
bool g_trayShown;
int g_dpi = 96;

bool Setting(PCWSTR name) {
    return Wh_GetIntSetting(name) != 0;
}

int Scale(int value) {
    return MulDiv(value, g_dpi, 96);
}

bool EndsWithI(std::wstring const& s, std::wstring const& suffix) {
    return s.size() >= suffix.size() && _wcsicmp(s.c_str() + s.size() - suffix.size(), suffix.c_str()) == 0;
}

std::wstring FileName(std::wstring const& path) {
    size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? path : path.substr(slash + 1);
}

void Log(std::wstring text) {
    if (g_window) {
        PostMessageW(g_window, kLogMessage, 0, (LPARAM) new std::wstring(std::move(text)));
    }
}

////////////////////////////////////////////////////////////////////////////////
// Installing

std::wstring CliPath() {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) {
        return {};
    }
    std::wstring folder(path, length);
    folder.resize(folder.find_last_of(L'\\') + 1);
    return folder + L"windhawk-cli.exe";
}

bool IsElevated() {
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return false;
    }
    TOKEN_ELEVATION elevation{};
    DWORD size;
    bool elevated = GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size) &&
                    elevation.TokenIsElevated;
    CloseHandle(token);
    return elevated;
}

std::wstring FromUtf8(std::string const& text) {
    if (text.empty()) {
        return {};
    }
    int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0);
    std::wstring wide(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), wide.data(), length);
    return wide;
}

std::string ReadFileText(std::wstring const& path, DWORD limit = 4 * 1024 * 1024) {
    std::string text;
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                              OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return text;
    }
    LARGE_INTEGER size;
    if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart <= limit) {
        text.resize((size_t)size.QuadPart);
        DWORD read = 0;
        if (!ReadFile(file, text.data(), (DWORD)text.size(), &read, nullptr)) {
            read = 0;
        }
        text.resize(read);
    }
    CloseHandle(file);
    return text;
}

// The source's "// @id" value, or empty.
std::wstring ModIdOf(std::string const& source) {
    size_t start = source.find("==WindhawkMod==");
    size_t end = source.find("==/WindhawkMod==");
    if (start == std::string::npos || end == std::string::npos || end < start) {
        return {};
    }
    for (size_t line = start; line < end;) {
        size_t next = source.find('\n', line);
        if (next == std::string::npos || next > end) {
            next = end;
        }
        std::string text = source.substr(line, next - line);
        size_t at = text.find("@id");
        if (text.rfind("//", 0) == 0 && at != std::string::npos &&
            (at + 3 == text.size() || text[at + 3] == ' ' || text[at + 3] == '\t')) {
            std::string id = text.substr(at + 3);
            while (!id.empty() && strchr(" \t\r", id.front())) id.erase(0, 1);
            while (!id.empty() && strchr(" \t\r", id.back())) id.pop_back();
            return FromUtf8(id);
        }
        line = next + 1;
    }
    return {};
}

// Runs the command line tool and returns its exit code, with what it printed.
// Elevated, it runs straight away with its output read back; otherwise
// through the administrator prompt, its output caught in a file.
DWORD RunCli(std::wstring const& args, std::wstring* output) {
    std::wstring cli = CliPath();
    output->clear();
    if (cli.empty() || GetFileAttributesW(cli.c_str()) == INVALID_FILE_ATTRIBUTES) {
        *output = L"windhawk-cli.exe isn't beside Windhawk (it comes with Windhawk 2).";
        return (DWORD)-1;
    }

    if (!IsElevated()) {
        WCHAR temp[MAX_PATH], file[MAX_PATH];
        if (!GetTempPathW(ARRAYSIZE(temp), temp) || !GetTempFileNameW(temp, L"tdi", 0, file)) {
            *output = L"Couldn't make a temporary file.";
            return (DWORD)-1;
        }
        WCHAR system[MAX_PATH];
        GetSystemDirectoryW(system, ARRAYSIZE(system));
        std::wstring cmd = std::wstring(system) + L"\\cmd.exe";
        std::wstring parameters = L"/d /s /c \"\"" + cli + L"\" " + args + L" > \"" + file + L"\" 2>&1\"";
        SHELLEXECUTEINFOW info{.cbSize = sizeof(info),
                               .fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC,
                               .hwnd = g_window,
                               .lpVerb = L"runas",
                               .lpFile = cmd.c_str(),
                               .lpParameters = parameters.c_str(),
                               .nShow = SW_HIDE};
        if (!ShellExecuteExW(&info) || !info.hProcess) {
            DeleteFileW(file);
            *output = L"Windhawk needs administrator to install a mod, and the prompt was turned down.";
            return (DWORD)-1;
        }
        WaitForSingleObject(info.hProcess, INFINITE);
        DWORD code = (DWORD)-1;
        GetExitCodeProcess(info.hProcess, &code);
        CloseHandle(info.hProcess);
        *output = FromUtf8(ReadFileText(file));
        DeleteFileW(file);
        return code;
    }

    SECURITY_ATTRIBUTES inherit{.nLength = sizeof(inherit), .bInheritHandle = TRUE};
    HANDLE read, write;
    if (!CreatePipe(&read, &write, &inherit, 0)) {
        *output = L"Couldn't start windhawk-cli.exe.";
        return (DWORD)-1;
    }
    SetHandleInformation(read, HANDLE_FLAG_INHERIT, 0);
    HANDLE nothing = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &inherit, OPEN_EXISTING, 0,
                                 nullptr);
    STARTUPINFOW startup{.cb = sizeof(startup), .dwFlags = STARTF_USESTDHANDLES};
    startup.hStdInput = nothing;
    startup.hStdOutput = write;
    startup.hStdError = write;
    std::wstring commandLine = L"\"" + cli + L"\" " + args;
    PROCESS_INFORMATION process;
    bool started = CreateProcessW(cli.c_str(), commandLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr,
                                  nullptr, &startup, &process);
    CloseHandle(write);
    if (nothing != INVALID_HANDLE_VALUE) {
        CloseHandle(nothing);
    }
    if (!started) {
        CloseHandle(read);
        *output = L"Couldn't start windhawk-cli.exe.";
        return (DWORD)-1;
    }
    std::string text;
    char buffer[4096];
    DWORD got;
    while (ReadFile(read, buffer, sizeof(buffer), &got, nullptr) && got) {
        text.append(buffer, got);
    }
    CloseHandle(read);
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD code = (DWORD)-1;
    GetExitCodeProcess(process.hProcess, &code);
    CloseHandle(process.hProcess);
    CloseHandle(process.hThread);
    *output = FromUtf8(text);
    return code;
}

std::wstring Trimmed(std::wstring s) {
    while (!s.empty() && iswspace(s.back())) s.pop_back();
    while (!s.empty() && iswspace(s.front())) s.erase(0, 1);
    return s;
}

// Line ends as an edit control wants them.
std::wstring ForEdit(std::wstring const& text) {
    std::wstring out;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == L'\n' && (i == 0 || text[i - 1] != L'\r')) out += L'\r';
        out += text[i];
    }
    return out;
}

void InstallOne(std::wstring const& path) {
    std::wstring name = FileName(path);
    std::wstring id = ModIdOf(ReadFileText(path));
    if (id.empty()) {
        Log(name + L": not a Windhawk mod (no // @id line).");
        return;
    }
    if (path.find(L'"') != std::wstring::npos) {
        Log(name + L": the file's path has a quote in it; rename the folder and try again.");
        return;
    }
    Log(name + L": compiling " + id + L"...");
    std::wstring shown;
    bool had = RunCli(L"--quiet mod show \"local@" + id + L"\"", &shown) == 0;
    bool off = !had && !Setting(L"turnOnNewMods");
    std::wstring output;
    DWORD code = RunCli(L"mod install --file \"" + path + L"\"" + (off ? L" --disabled" : L""), &output);
    output = Trimmed(output);
    if (code == 0) {
        Log(name + L": installed" + (had ? L" (updated, same on or off state as before)." : off ? L", turned off." : L", turned on.") +
            (output.empty() ? L"" : L"\n" + output));
    } else {
        Log(name + L": not installed (code " + std::to_wstring((int)code) + L")." + (output.empty() ? L"" : L"\n" + output));
    }
}

// One install at a time, in the order they were dropped.
CRITICAL_SECTION g_queueLock;
std::deque<std::wstring> g_queue;
bool g_working;

DWORD WINAPI InstallThread(LPVOID) {
    for (;;) {
        std::wstring path;
        EnterCriticalSection(&g_queueLock);
        if (g_queue.empty()) {
            g_working = false;
            LeaveCriticalSection(&g_queueLock);
            return 0;
        }
        path = g_queue.front();
        g_queue.pop_front();
        LeaveCriticalSection(&g_queueLock);
        InstallOne(path);
    }
}

void AddFile(std::vector<std::wstring>& files, std::wstring const& path) {
    DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return;
    }
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        files.push_back(path);
        return;
    }
    // A folder: the .wh.cpp files in it.
    WIN32_FIND_DATAW find;
    HANDLE search = FindFirstFileW((path + L"\\*.wh.cpp").c_str(), &find);
    if (search == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        if (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            files.push_back(path + L"\\" + find.cFileName);
        }
    } while (FindNextFileW(search, &find));
    FindClose(search);
}

void Install(std::vector<std::wstring> const& dropped) {
    std::vector<std::wstring> files;
    for (auto const& path : dropped) {
        AddFile(files, path);
    }
    if (files.empty()) {
        Log(L"Nothing to install there: drop .wh.cpp files, or a folder holding them.");
        return;
    }
    EnterCriticalSection(&g_queueLock);
    for (auto const& file : files) {
        if (EndsWithI(file, L".cpp")) {
            g_queue.push_back(file);
        } else {
            Log(FileName(file) + L": not a .wh.cpp file, left alone.");
        }
    }
    bool start = !g_working && !g_queue.empty();
    if (start) {
        g_working = true;
    }
    LeaveCriticalSection(&g_queueLock);
    if (start) {
        HANDLE thread = CreateThread(nullptr, 0, InstallThread, nullptr, 0, nullptr);
        if (thread) {
            CloseHandle(thread);
        } else {
            EnterCriticalSection(&g_queueLock);
            g_working = false;
            g_queue.clear();
            LeaveCriticalSection(&g_queueLock);
            Log(L"Couldn't start installing.");
        }
    }
}

void ChooseFiles() {
    std::vector<WCHAR> buffer(32768);
    OPENFILENAMEW open{.lStructSize = sizeof(open),
                       .hwndOwner = g_window,
                       .lpstrFilter = L"Windhawk mods (*.wh.cpp)\0*.wh.cpp\0C++ files (*.cpp)\0*.cpp\0",
                       .lpstrFile = buffer.data(),
                       .nMaxFile = (DWORD)buffer.size(),
                       .lpstrTitle = L"Choose mods to install",
                       .Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY};
    if (!GetOpenFileNameW(&open)) {
        return;
    }
    // One file: its path. Several: the folder, then each name.
    std::vector<std::wstring> parts;
    for (PCWSTR p = buffer.data(); *p; p += wcslen(p) + 1) {
        parts.push_back(p);
    }
    std::vector<std::wstring> files;
    if (parts.size() == 1) {
        files.push_back(parts[0]);
    } else {
        for (size_t i = 1; i < parts.size(); i++) {
            files.push_back(parts[0] + L"\\" + parts[i]);
        }
    }
    Install(files);
}

////////////////////////////////////////////////////////////////////////////////
// The window and the tray icon

// A green circle with a white arrow into a tray, drawn at the size asked.
HICON MakeIcon(int size) {
    BITMAPINFO info{};
    info.bmiHeader = {.biSize = sizeof(BITMAPINFOHEADER), .biWidth = size, .biHeight = -size, .biPlanes = 1,
                      .biBitCount = 32, .biCompression = BI_RGB};
    void* bits = nullptr;
    HDC screen = GetDC(nullptr);
    HBITMAP color = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, screen);
    if (!color || !bits) {
        return LoadIconW(nullptr, IDI_APPLICATION);
    }
    auto* pixels = (DWORD*)bits;
    double c = (size - 1) / 2.0, r = size / 2.0 - 0.5;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            double dx = x - c, dy = y - c;
            double inside = r - sqrt(dx * dx + dy * dy) + 0.5;
            double a = inside <= 0 ? 0 : inside >= 1 ? 1 : inside;
            // The arrow: a shaft and a head, then a tray under it.
            double u = (double)x / size, v = (double)y / size;
            bool shaft = fabs(u - 0.5) < 0.07 && v > 0.2 && v < 0.55;
            bool head = v >= 0.42 && v < 0.64 && fabs(u - 0.5) < (0.64 - v) * 1.1;
            bool tray = (v > 0.7 && v < 0.78 && u > 0.26 && u < 0.74) ||
                        (v > 0.6 && v < 0.78 && ((u > 0.26 && u < 0.33) || (u > 0.67 && u < 0.74)));
            bool white = shaft || head || tray;
            BYTE R = white ? 255 : 0x4D, G = white ? 255 : 0x86, B = white ? 255 : 0x71;
            BYTE A = (BYTE)(a * 255);
            pixels[y * size + x] = ((DWORD)A << 24) | ((DWORD)(R * a) << 16) | ((DWORD)(G * a) << 8) | (DWORD)(B * a);
        }
    }
    HBITMAP mask = CreateBitmap(size, size, 1, 1, nullptr);
    ICONINFO iconInfo{.fIcon = TRUE, .hbmMask = mask, .hbmColor = color};
    HICON icon = CreateIconIndirect(&iconInfo);
    DeleteObject(mask);
    DeleteObject(color);
    return icon ? icon : LoadIconW(nullptr, IDI_APPLICATION);
}

void ShowTrayIcon(bool show) {
    NOTIFYICONDATAW data{.cbSize = sizeof(data), .hWnd = g_window, .uID = kTrayId};
    if (!show) {
        if (g_trayShown) Shell_NotifyIconW(NIM_DELETE, &data);
        g_trayShown = false;
        return;
    }
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
    data.uCallbackMessage = kTrayMessage;
    data.hIcon = g_icon;
    wcscpy_s(data.szTip, kTitle);
    if (Shell_NotifyIconW(g_trayShown ? NIM_MODIFY : NIM_ADD, &data)) {
        g_trayShown = true;
        data.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &data);
    }
}

void MakeFonts() {
    NONCLIENTMETRICSW metrics{.cbSize = sizeof(metrics)};
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, g_dpi);
    if (g_font) DeleteObject(g_font);
    if (g_bigFont) DeleteObject(g_bigFont);
    g_font = CreateFontIndirectW(&metrics.lfMessageFont);
    LOGFONTW big = metrics.lfMessageFont;
    big.lfHeight = big.lfHeight * 3 / 2;
    big.lfWeight = FW_SEMIBOLD;
    g_bigFont = CreateFontIndirectW(&big);
    for (HWND control : {g_choose, g_log}) {
        if (control) SendMessageW(control, WM_SETFONT, (WPARAM)g_font, TRUE);
    }
    if (g_drop) SendMessageW(g_drop, WM_SETFONT, (WPARAM)g_bigFont, TRUE);
}

void Layout() {
    RECT client;
    GetClientRect(g_window, &client);
    int pad = Scale(12), width = client.right - 2 * pad;
    int dropHeight = Scale(110), buttonHeight = Scale(30);
    MoveWindow(g_drop, pad, pad, width, dropHeight, TRUE);
    MoveWindow(g_choose, pad, pad * 2 + dropHeight, Scale(130), buttonHeight, TRUE);
    int logTop = pad * 3 + dropHeight + buttonHeight;
    MoveWindow(g_log, pad, logTop, width, client.bottom - logTop - pad, TRUE);
}

void Open() {
    ShowWindow(g_window, SW_SHOWNORMAL);
    SetForegroundWindow(g_window);
}

void ApplySettings() {
    ShowTrayIcon(Setting(L"trayIcon"));
    SetWindowPos(g_window, Setting(L"keepOnTop") ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void ShowMenu() {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, kMenuOpen, L"Open the drop window");
    AppendMenuW(menu, MF_STRING, kMenuClose, L"Close it");
    SetMenuDefaultItem(menu, kMenuOpen, FALSE);
    POINT point;
    GetCursorPos(&point);
    SetForegroundWindow(g_window);
    int choice = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, g_window, nullptr);
    DestroyMenu(menu);
    if (choice == kMenuOpen) {
        Open();
    } else if (choice == kMenuClose) {
        ShowWindow(g_window, SW_HIDE);
    }
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == g_taskbarCreated && g_taskbarCreated) {
        g_trayShown = false;
        ShowTrayIcon(Setting(L"trayIcon"));
        return 0;
    }
    switch (message) {
        case WM_CREATE: {
            g_window = window;
            g_dpi = GetDpiForWindow(window);
            HINSTANCE instance = GetModuleHandleW(nullptr);
            g_drop = CreateWindowExW(0, L"STATIC", L"Drop .wh.cpp files here",
                                     WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | SS_SUNKEN, 0, 0, 0, 0, window,
                                     (HMENU)(INT_PTR)kDropId, instance, nullptr);
            g_choose = CreateWindowExW(0, L"BUTTON", L"Choose files...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                       0, 0, 0, 0, window, (HMENU)(INT_PTR)kChooseId, instance, nullptr);
            g_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL, 0, 0,
                                    0, 0, window, (HMENU)(INT_PTR)kLogId, instance, nullptr);
            SendMessageW(g_log, EM_SETLIMITTEXT, 1024 * 1024, 0);
            MakeFonts();
            DragAcceptFiles(window, TRUE);
            // Windhawk runs this as administrator, and Windows keeps drops from
            // Explorer (not administrator) out of such windows unless asked.
            ChangeWindowMessageFilterEx(window, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
            ChangeWindowMessageFilterEx(window, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
            ChangeWindowMessageFilterEx(window, 0x0049 /* WM_COPYGLOBALDATA */, MSGFLT_ALLOW, nullptr);
            return 0;
        }
        case WM_SIZE:
            Layout();
            return 0;
        case WM_DPICHANGED: {
            g_dpi = HIWORD(wParam);
            MakeFonts();
            auto* rect = (RECT*)lParam;
            SetWindowPos(window, nullptr, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_GETMINMAXINFO: {
            auto* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize = {Scale(360), Scale(300)};
            return 0;
        }
        case WM_CTLCOLORSTATIC:
            if ((HWND)lParam == g_drop) {
                HDC dc = (HDC)wParam;
                SetTextColor(dc, RGB(0x4D, 0x86, 0x71));
                SetBkColor(dc, GetSysColor(COLOR_WINDOW));
                return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
            }
            break;
        case WM_DROPFILES: {
            auto drop = (HDROP)wParam;
            UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
            std::vector<std::wstring> files;
            for (UINT i = 0; i < count; i++) {
                UINT length = DragQueryFileW(drop, i, nullptr, 0);
                std::wstring path(length, L'\0');
                DragQueryFileW(drop, i, path.data(), length + 1);
                files.push_back(path);
            }
            DragFinish(drop);
            Install(files);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == kChooseId) ChooseFiles();
            return 0;
        case kLogMessage: {
            auto* text = (std::wstring*)lParam;
            std::wstring line = ForEdit(*text) + L"\r\n";
            delete text;
            int end = GetWindowTextLengthW(g_log);
            SendMessageW(g_log, EM_SETSEL, end, end);
            SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
            return 0;
        }
        case kTrayMessage:
            switch (LOWORD(lParam)) {
                case NIN_SELECT:
                case NIN_KEYSELECT:
                case WM_LBUTTONUP:
                    Open();
                    return 0;
                case WM_CONTEXTMENU:
                case WM_RBUTTONUP:
                    ShowMenu();
                    return 0;
            }
            return 0;
        case kSettingsMessage:
            ApplySettings();
            return 0;
        case kQuitMessage:
            DestroyWindow(window);
            return 0;
        case WM_CLOSE:
            // Closing hides it; the tray icon brings it back.
            if (g_trayShown) {
                ShowWindow(window, SW_HIDE);
                return 0;
            }
            break;
        case WM_DESTROY:
            ShowTrayIcon(false);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

DWORD WINAPI WindowThread(LPVOID) {
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    HINSTANCE instance = GetModuleHandleW(nullptr);
    g_icon = MakeIcon(GetSystemMetrics(SM_CXSMICON) * 2);
    WNDCLASSEXW wc{.cbSize = sizeof(wc),
                   .lpfnWndProc = WindowProc,
                   .hInstance = instance,
                   .hIcon = g_icon,
                   .hCursor = LoadCursorW(nullptr, IDC_ARROW),
                   .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
                   .lpszClassName = kClass,
                   .hIconSm = g_icon};
    RegisterClassExW(&wc);
    HWND window = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, kClass, kTitle, WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT, CW_USEDEFAULT, 100, 100, nullptr, nullptr, instance, nullptr);
    if (!window) {
        return 1;
    }
    SetWindowPos(window, nullptr, 0, 0, Scale(480), Scale(420), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    ChangeWindowMessageFilterEx(window, g_taskbarCreated, MSGFLT_ALLOW, nullptr);
    ApplySettings();
    if (!g_trayShown) {
        Open();
    }
    Log(IsElevated() ? L"Ready. Drop .wh.cpp files above."
                     : L"Ready. Windhawk couldn't run this as administrator, so each install will ask for it.");

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (IsDialogMessageW(window, &message)) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}

bool IsHost() {
    WCHAR path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, ARRAYSIZE(path));
    if (!length || length >= ARRAYSIZE(path)) {
        return false;
    }
    PCWSTR name = wcsrchr(path, L'\\');
    name = name ? name + 1 : path;
    return _wcsnicmp(name, L"windhawk-mod", 12) == 0;
}

void WINAPI EntryPointHook() {
    ExitThread(0);
}

}  // namespace

BOOL Wh_ModInit() {
    if (!IsHost()) {
        return FALSE;
    }
    InitializeCriticalSection(&g_queueLock);
    // The host's own entry point only says the engine never reached it; the
    // window thread keeps the process instead.
    auto* dos = (IMAGE_DOS_HEADER*)GetModuleHandleW(nullptr);
    auto* nt = (IMAGE_NT_HEADERS*)((BYTE*)dos + dos->e_lfanew);
    Wh_SetFunctionHook((BYTE*)dos + nt->OptionalHeader.AddressOfEntryPoint, (void*)EntryPointHook, nullptr);
    return TRUE;
}

void Wh_ModAfterInit() {
    g_thread = CreateThread(nullptr, 0, WindowThread, nullptr, 0, nullptr);
}

BOOL Wh_ModSettingsChanged(BOOL* reload) {
    if (g_window) PostMessageW(g_window, kSettingsMessage, 0, 0);
    *reload = FALSE;
    return TRUE;
}

void Wh_ModUninit() {
    if (g_window) PostMessageW(g_window, kQuitMessage, 0, 0);
    if (g_thread) WaitForSingleObject(g_thread, 2000);
    ExitProcess(0);
}
