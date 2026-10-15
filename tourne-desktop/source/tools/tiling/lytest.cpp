// Window layouts and places outside Windhawk (switcher-inlined.cpp: switcher-host.cpp with its @inline
// files; Windhawk's functions return their defaults, WH_EDITING). "lytest child" is the program whose
// windows are moved: two see-through windows, so nothing shows, without a resizing border, so the tiling (if it runs on this PC) leaves them alone. No tiling runs: every screen's layout
// is set to off first, and only this test's own lines are applied.
#include "switcher-inlined.cpp"
#include <cstdio>

constexpr UINT WM_NEW_A = WM_APP + 50;

LRESULT CALLBACK ChildProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
    if (msg == WM_NEW_A) {
        HWND a = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT, L"LyTestA", L"Test A", WS_CAPTION | WS_SYSMENU, 50, 50, 500,
                                 300, nullptr, nullptr, nullptr, nullptr);
        SetLayeredWindowAttributes(a, 0, 1, LWA_ALPHA);
        ShowWindow(a, SW_SHOWNOACTIVATE);
        return (LRESULT)a;
    }
    WCHAR cls[16] = L"";
    GetClassNameW(h, cls, 16);
    if (msg == WM_DESTROY && !wcscmp(cls, L"LyTestB")) PostQuitMessage(0);
    return DefWindowProcW(h, msg, w, l);
}

int Child() {
    for (PCWSTR cls : {L"LyTestA", L"LyTestB"}) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = ChildProc;
        wc.lpszClassName = cls;
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        RegisterClassW(&wc);
    }
    HWND b = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT, L"LyTestB", L"Test B", WS_CAPTION | WS_SYSMENU, 700, 120, 400,
                             250, nullptr, nullptr, nullptr, nullptr);
    SetLayeredWindowAttributes(b, 0, 1, LWA_ALPHA);
    ShowWindow(b, SW_SHOWNOACTIVATE);
    SendMessageW(b, WM_NEW_A, 0, 0);
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) DispatchMessageW(&msg);
    return 0;
}

void Frame(PCWSTR what, HWND h) {
    RECT r = FrameOf(h);
    wprintf(L"%ls: %ld,%ld %ldx%ld\n", what, r.left, r.top, r.right - r.left, r.bottom - r.top);
}

// Only this test's lines from a layout.
std::wstring OwnLines(std::wstring const& text) {
    std::wstring out;
    size_t at = 0;
    while (at < text.size()) {
        size_t end = text.find(L'\n', at);
        std::wstring line = text.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at + 1);
        at = end == std::wstring::npos ? text.size() : end + 1;
        if (line.find(L"lytest.exe|") != std::wstring::npos) out += line;
    }
    return out;
}

void Pump(int ms) {
    for (int i = 0; i < ms / 10; i++) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (LayoutsDialogMessage(&msg)) continue;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        Sleep(10);
    }
}

int wmain(int argc, wchar_t** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    if (argc > 1 && !wcscmp(argv[1], L"child")) return Child();
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_settings = LoadSettings();
    g_tiling = {};
    g_tiling.rememberAll = true;
    g_autoTile = false;
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR m, HDC, LPRECT, LPARAM) -> BOOL {
            g_layoutChoices.push_back({ChoiceKey(m), TileLayout::Off});
            return TRUE;
        },
        0);
    WCHAR self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    std::wstring cmd = L"\"" + std::wstring(self) + L"\" child";
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) return 1;
    HWND a = nullptr, b = nullptr;
    for (int i = 0; i < 100 && !(a && b); i++, Sleep(30)) {
        a = FindWindowW(L"LyTestA", nullptr);
        b = FindWindowW(L"LyTestB", nullptr);
    }
    if (!a || !b) return 2;
    wprintf(L"keys: %ls / %ls\n", KeptKey(a).c_str(), KeptKey(b).c_str());
    wprintf(L"placeable: %d %d\n", Placeable(a), Placeable(b));

    // 1. A layout: taken, the windows moved away, applied: back where they were.
    std::wstring layout = OwnLines(SnapshotLayout());
    wprintf(L"--- layout (this test's lines)\n%ls", layout.c_str());
    Frame(L"A before", a);
    SetWindowPos(a, nullptr, 300, 260, 640, 380, SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(b, nullptr, 20, 500, 300, 200, SWP_NOZORDER | SWP_NOACTIVATE);
    Frame(L"A moved", a);
    wprintf(L"applied: %d\n", ApplyLayout(layout));
    Frame(L"A after apply", a);
    Frame(L"B after apply", b);

    // 2. Places: A followed, moved, closed; a new A opens where the old one closed.
    TrackOpenWindows();
    SetWindowPos(a, nullptr, 420, 330, 520, 360, SWP_NOZORDER | SWP_NOACTIVATE);
    PlacesEvent(EVENT_SYSTEM_MOVESIZEEND, a);  // (as a drag by hand ends)
    Frame(L"A before closing", a);
    SendMessageW(a, WM_CLOSE, 0, 0);
    PlacesEvent(EVENT_OBJECT_DESTROY, a);
    wprintf(L"places saved: %zu, first: %ls\n", g_places.size(), g_places.empty() ? L"" : g_places.back().key.c_str());
    HWND a2 = (HWND)SendMessageW(b, WM_NEW_A, 0, 0);
    Frame(L"new A as opened", a2);
    PlacesEvent(EVENT_OBJECT_SHOW, a2);
    Frame(L"new A after", a2);

    // 3. The layouts window: a layout saved from the windows, renamed, saved; then pictured.
    OpenLayoutsWindow();
    if (!g_ly.wnd) return 3;
    SetWindowPos(g_ly.wnd, nullptr, -5000, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    Pump(200);
    SendMessageW(g_ly.wnd, WM_COMMAND, kLyNew, 0);
    SetDlgItemTextW(g_ly.wnd, kLyName, L"Coding");
    SendMessageW(g_ly.wnd, WM_COMMAND, kLySave, 0);
    SendMessageW(g_ly.wnd, WM_COMMAND, kLyNew, 0);
    wprintf(L"layouts: %zu, first \"%ls\", second \"%ls\", status \"%ls\"\n", g_layouts.size(), g_layouts[0].name.c_str(),
            g_layouts.size() > 1 ? g_layouts[1].name.c_str() : L"", ControlText(g_ly.wnd, kLyStatus).c_str());
    SendMessageW(g_ly.wnd, WM_COMMAND, MAKEWPARAM(kLyList, LBN_SELCHANGE), 0);
    SendDlgItemMessageW(g_ly.wnd, kLyList, LB_SETCURSEL, 0, 0);
    SendMessageW(g_ly.wnd, WM_COMMAND, MAKEWPARAM(kLyList, LBN_SELCHANGE), 0);
    // The editor shows only this test's lines in the picture.
    SetDlgItemTextW(g_ly.wnd, kLyText, OwnLines(g_layouts[0].text).c_str());
    Pump(300);
    RECT rc;
    GetWindowRect(g_ly.wnd, &rc);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bmp);
    PrintWindow(g_ly.wnd, dc, 2);
    GdiFlush();
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(BITMAPINFOHEADER) + w * h * 4), 0, 0,
                        sizeof(fh) + sizeof(BITMAPINFOHEADER)};
    FILE* f = _wfopen(L"lytest.bmp", L"wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(bits, (size_t)w * h * 4, 1, f);
    fclose(f);
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    wprintf(L"window %dx%d saved\n", w, h);
    DestroyWindow(g_ly.wnd);
    PostMessageW(a2, WM_CLOSE, 0, 0);
    PostMessageW(b, WM_CLOSE, 0, 0);
    WaitForSingleObject(pi.hProcess, 3000);
    TerminateProcess(pi.hProcess, 0);
    return 0;
}
