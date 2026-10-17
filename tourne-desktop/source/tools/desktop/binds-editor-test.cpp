// Opens Keys and rules (desktop/host/binds-editor.cpp) on its own, with the lines from a file standing in for the
// user's settings, and saves a picture of each tab; OK/Apply print what would be written instead of writing it.
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -O1 -static -Idesktop\host tools\desktop\binds-editor-test.cpp desktop\host\binds-editor.cpp
//     -o tools\desktop\binds-editor-test.exe -lcomctl32 -lcomdlg32 -lgdi32 -luser32 -lole32 -lshell32 -luxtheme
// Run: binds-editor-test.exe lines.conf [shots-folder]   (no folder: it stays open to try by hand)
#include <windows.h>
#include <commctrl.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "binds-editor.h"

std::vector<std::pair<std::wstring, std::wstring>> g_settings;

namespace td {
std::vector<std::pair<std::wstring, std::wstring>> SettingsWithPrefix(const std::wstring& prefix) {
    std::vector<std::pair<std::wstring, std::wstring>> out;
    for (auto const& kv : g_settings)
        if (kv.first.rfind(prefix, 0) == 0) out.push_back(kv);
    return out;
}
bool WriteStudioSettings(std::vector<std::pair<std::wstring, std::wstring>> const& set, std::wstring const&, std::wstring const& why) {
    for (auto const& [k, v] : set) wprintf(L"would set %ls = %ls (%ls)\n", k.c_str(), v.c_str(), why.c_str());
    return true;
}
}  // namespace td

void Shot(HWND w, std::wstring const& path) {
    RECT r;
    GetWindowRect(w, &r);
    int cw = r.right - r.left, ch = r.bottom - r.top;
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), cw, ch, 1, 32, BI_RGB};
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bmp);
    PrintWindow(w, dc, 2);
    GdiFlush();
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader) + cw * ch * 4), 0, 0, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader))};
    if (FILE* f = _wfopen(path.c_str(), L"wb")) {
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&bi.bmiHeader, sizeof(bi.bmiHeader), 1, f);
        fwrite(bits, 4, (size_t)cw * ch, f);
        fclose(f);
    }
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
}

DWORD WINAPI Shooter(LPVOID p) {
    std::wstring dir = (const wchar_t*)p;
    HWND w = nullptr;
    for (int i = 0; i < 50 && !(w = FindWindowW(L"TourneBindsSettings", nullptr)); i++) Sleep(100);
    if (!w) return 1;
    Sleep(600);
    HWND list = GetDlgItem(w, 101);  // (the binds' list: the first row picked)
    ListView_SetItemState(list, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    Sleep(300);
    HWND tabs = GetDlgItem(w, 100);
    for (int t = 0; t < 4; t++) {
        TabCtrl_SetCurSel(tabs, t);
        NMHDR h{tabs, 100, TCN_SELCHANGE};
        SendMessageW(w, WM_NOTIFY, 100, (LPARAM)&h);
        if (t == 1) {
            HWND rules = GetDlgItem(w, 127);
            ListView_SetItemState(rules, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
        Sleep(400);
        Shot(w, dir + L"\\tab" + std::to_wstring(t) + L".bmp");
    }
    PostMessageW(w, WM_CLOSE, 0, 0);
    Sleep(300);
    if (HWND ask = FindWindowW(L"#32770", L"Keys and rules")) PostMessageW(ask, WM_COMMAND, IDNO, 0);
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (argc > 1) {
        std::wifstream in(argv[1]);
        std::wstring line;
        int i = 0;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            g_settings.push_back({L"switcher.binds.lines[" + std::to_wstring(i++) + L"]", line});
        }
    }
    g_settings.push_back({L"switcher.binds.enabled", L"1"});
    if (argc > 2) CreateThread(nullptr, 0, Shooter, argv[2], 0, nullptr);
    td::RunBindsEditor();
    return 0;
}
