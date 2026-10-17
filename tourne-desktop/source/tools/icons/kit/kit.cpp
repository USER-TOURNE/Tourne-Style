// Tourne'Icons Kit: Tourne'Icons Lasting as a program of its own, without
// Windhawk. Open it, change icons (or apply a pack), close it: nothing of it
// runs after that, and nothing starts with Windows. The changes are written
// where Windows keeps them (shortcuts, file types, Windows' own icon
// settings), so they stay. Opening it again puts the icons back on shortcuts
// made or remade since (an app installed, pinned or updated), and its picker
// puts any change back as it was.
//
// It is tourne-icons.wh.cpp built with TOURNE_ICONS_KIT 1; this file stands
// in for Windhawk's few functions and runs the picker. Build: build.sh.

#define TOURNE_ICONS_LASTING 1
#define TOURNE_ICONS_KIT 1
#include "../../../tourne-icons.wh.cpp"

#include <cstdarg>
#include <cstdio>
#include <urlmon.h>

namespace kit {

// %LOCALAPPDATA%\Tourne\Icons Kit: the list, what each change replaced
// (lasting.txt), and the icons made.
std::wstring g_storage;

std::wstring Storage() {
    if (!g_storage.empty()) return g_storage;
    PWSTR local = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &local)) && local) {
        std::wstring tourne = std::wstring(local) + L"\\Tourne";
        CreateDirectoryW(tourne.c_str(), nullptr);
        std::wstring folder = tourne + L"\\Icons Kit";
        if (CreateDirectoryW(folder.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS) g_storage = folder;
    }
    CoTaskMemFree(local);
    return g_storage;
}

// Putting back what was changed on shortcuts since the Kit was last open, on
// a thread of its own so the window opens at once.
DWORD WINAPI PutBackThread(LPVOID) {
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool separated = SeparateUnknownFileTypes() > 0;
    PublishButtons(ReadList());  // (Tourne Desktop's taskbar buttons)
    EnumWindows(TellBarIcons, TRUE);
    int changed = lasting::ApplyAll();
    if (separated && !changed) TellWindowsIconsChanged();
    if (changed) TellWindowsIconsChanged();
    if (picker::g_window) PostMessageW(picker::g_window, picker::kPutBackMessage, changed, 0);
    lasting::StartSettings();
    if (SUCCEEDED(com)) CoUninitialize();
    return 0;
}

}  // namespace kit

void KitLog(PCWSTR format, ...) {
    WCHAR text[1024];
    va_list args;
    va_start(args, format);
    _vsnwprintf_s(text, ARRAYSIZE(text), _TRUNCATE, format, args);
    va_end(args);
    OutputDebugStringW((std::wstring(L"[Tourne'Icons Kit] ") + text + L"\n").c_str());
}

// The Kit has no settings of its own: the picker holds every change.
int Wh_GetIntSetting(PCWSTR, ...) {
    return 0;
}

PCWSTR Wh_GetStringSetting(PCWSTR, ...) {
    return L"";
}

void Wh_FreeStringSetting(PCWSTR) {}

// Nothing is hooked.
BOOL Wh_SetFunctionHook(void*, void*, void**) {
    return FALSE;
}

size_t Wh_GetModStoragePath(PWSTR buffer, size_t chars) {
    std::wstring storage = kit::Storage();
    if (storage.empty() || storage.size() >= chars) return 0;
    wcscpy_s(buffer, chars, storage.c_str());
    return storage.size();
}

// Only icon themes are downloaded, straight to a file.
const WH_URL_CONTENT* Wh_GetUrlContent(PCWSTR url, const WH_GET_URL_CONTENT_OPTIONS* options) {
    if (!options || !options->targetFilePath) return nullptr;
    auto* content = new WH_URL_CONTENT{};
    content->statusCode = SUCCEEDED(URLDownloadToFileW(nullptr, url, options->targetFilePath, 0, nullptr)) ? 200 : 0;
    return content;
}

void Wh_FreeUrlContent(const WH_URL_CONTENT* content) {
    delete content;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    // One at a time: opening it again brings the open one forward.
    HANDLE single = CreateMutexW(nullptr, TRUE, L"TourneIconsKit");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (HWND open = FindWindowW(picker::kClass, nullptr)) {
            if (IsIconic(open)) ShowWindow(open, SW_RESTORE);
            SetForegroundWindow(open);
        }
        return 0;
    }
    WCHAR root[MAX_PATH];
    if (GetSystemWindowsDirectoryW(root, ARRAYSIZE(root))) g_systemRoot = Lower(root);
    WCHAR self[MAX_PATH * 2];
    DWORD selfLength = GetModuleFileNameW(nullptr, self, ARRAYSIZE(self));
    if (selfLength && selfLength < ARRAYSIZE(self)) g_selfPath = Lower(self);

    if (!api::Load() || kit::Storage().empty()) {
        MessageBoxW(nullptr, L"Tourne'Icons Kit couldn't start: this Windows is missing something it needs.", kModTitle,
                    MB_OK | MB_ICONERROR);
        return 1;
    }
    HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX controls{.dwSize = sizeof(controls), .dwICC = ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
    api::InitCommonControlsEx(&controls);
    // Start and other Store apps may read the icons the Kit makes.
    store::LetAppsRead(kit::Storage());
    g_publishButtons = true;

    picker::Open();
    HANDLE putBack = CreateThread(nullptr, 0, kit::PutBackThread, nullptr, 0, nullptr);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (picker::IsDialogMessageHere(&message)) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    // Closed: let what's being written finish, then end.
    if (putBack) {
        WaitForSingleObject(putBack, INFINITE);
        CloseHandle(putBack);
    }
    while (lasting::g_settingsRunning) Sleep(50);
    {
        lasting::Locked done;
    }
    if (SUCCEEDED(com)) CoUninitialize();
    if (single) CloseHandle(single);
    return 0;
}
