// Explorer Style's folder layouts (explorer-layout.cpp) made from this PC's shell32 for each preview pane
// variant (PreviewPaneLayout), printed, and parsed by DirectUI's own parser to see that each is valid XML.
#include <windows.h>
#include <shobjidl.h>
#include <atomic>
#include <cstdio>
#include <string>
#include <string_view>

// What explorer-layout.cpp takes from Windhawk and from Explorer Style.
static BOOL Wh_SetFunctionHook(void*, void*, void**) { return TRUE; }
static PCWSTR Wh_GetStringSetting(PCWSTR) { return L""; }
static void Wh_FreeStringSetting(PCWSTR) {}
static int Wh_GetIntSetting(PCWSTR) { return 0; }
static std::wstring ExplorerPreset() { return L""; }
static std::atomic<bool> g_styleBars{false};
#define Wh_Log(...)

#include "../../explorer-layout.cpp"

int wmain(int argc, wchar_t** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    UINT id = argc > 1 ? _wtoi(argv[1]) : 4;
    HMODULE shell = LoadLibraryExW(L"shell32.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    std::wstring xml = BottomDetailsLayout(shell, id);
    if (xml.empty()) {
        wprintf(L"layout %u: nothing to change\n", id);
        return 0;
    }
    wprintf(L"%ls\n", xml.c_str());
    return 0;
}
