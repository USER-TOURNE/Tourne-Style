// The calendar's new-event window outside Windhawk: the time parser, the Google Calendar link it would
// open (built, not opened), and the window itself saved as evtest.bmp (it opens off screen).
#include "sysicons-inlined.cpp"
#include <cstdio>

// The window is made off screen, so nothing shows on the desktop.
static LRESULT CALLBACK OffScreen(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HCBT_CREATEWND) {
        auto* create = ((CBT_CREATEWNDW*)lParam)->lpcs;
        if (create->lpszClass && !IS_INTRESOURCE(create->lpszClass) && !wcscmp(create->lpszClass, kEventClass)) {
            create->x = -5000;
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

int wmain() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SetWindowsHookExW(WH_CBT, OffScreen, nullptr, GetCurrentThreadId());
    for (PCWSTR t : {L"9", L"9:30", L"0930", L"9pm", L"9:30 am", L"21:15", L"12am", L"12pm", L"25", L"9:5", L"abc"})
        wprintf(L"time \"%ls\" -> %d\n", t, ParseEventTime(t));
    SYSTEMTIME day{};
    wprintf(L"date ok %d, bad %d\n", ParseEventDate(L"2026-10-03", &day), ParseEventDate(L"10/03/2026", &day));
    SYSTEMTIME local{2026, 10, 0, 3, 14, 30, 0, 0};
    wprintf(L"14:30 local on 2026-10-03 -> %ls\n", GoogleUtc(local).c_str());
    wprintf(L"encoded: %ls\n", UrlEncode(L"Dentist & café, 2nd floor").c_str());

    g_settings = LoadSettings();
    g_settings.calendarAccounts = {L"calendar.person@gmail.com", L"other.person@gmail.com"};
    SYSTEMTIME now;
    GetLocalTime(&now);
    OpenEventEditor(DayStart(MinutesOf(now)) + 3 * 24 * 60);
    if (!g_event.wnd) {
        wprintf(L"no window\n");
        return 1;
    }
    RECT rc;
    GetWindowRect(g_event.wnd, &rc);
    SetWindowPos(g_event.wnd, nullptr, -5000, 100, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    SetDlgItemTextW(g_event.wnd, kEvTitle, L"Dentist");
    SetDlgItemTextW(g_event.wnd, kEvWhere, L"Main St & 2nd");
    SetDlgItemTextW(g_event.wnd, kEvNotes, L"Bring the forms");
    g_event.importance = 2;
    MSG msg;
    for (int i = 0; i < 40; i++) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
        Sleep(10);
    }
    // The link, as Add would open it (the same steps, without opening it).
    std::wstring title = EventText(kEvTitle);
    wprintf(L"fields: title \"%ls\", day \"%ls\", from \"%ls\" to \"%ls\"\n", title.c_str(), EventText(kEvDate).c_str(),
            EventText(kEvStart).c_str(), EventText(kEvEnd).c_str());
    SendMessageW(g_event.wnd, WM_COMMAND, kEvAccount, 0);  // the second account
    std::wstring url;
    wprintf(L"link: %ls\n", EventUrl(&url) ? url.c_str() : g_event.error.c_str());
    SendMessageW(g_event.wnd, WM_COMMAND, kEvAccount, 0);  // back to the first
    RECT client;
    GetClientRect(g_event.wnd, &client);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
    void* bits = nullptr;
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bmp);
    PrintWindow(g_event.wnd, dc, 2 /* PW_RENDERFULLCONTENT */);
    GdiFlush();
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(BITMAPINFOHEADER) + w * h * 4), 0, 0,
                        sizeof(fh) + sizeof(BITMAPINFOHEADER)};
    FILE* f = _wfopen(L"evtest.bmp", L"wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(bits, (size_t)w * h * 4, 1, f);
    fclose(f);
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
    wprintf(L"window %dx%d saved\n", w, h);
    DestroyWindow(g_event.wnd);
    return 0;
}
