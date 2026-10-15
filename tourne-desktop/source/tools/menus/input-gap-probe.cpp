// Input gap probe: while the mouse moves, logs every gap between raw mouse reports that's longer than it should be,
// and what was on screen (a menu open, which window under the pointer). Raw input reaches a program only after every
// low-level mouse hook has let the movement go, so a hook that's slow (its thread busy) shows up here as a gap; if the
// gaps stay small while the pointer still looks like it hitches, the hitch is in what's drawn, not in the input.
//
//   input-gap-probe.exe <seconds> <log> [<csv>]     (runs hidden; the csv has every report)
#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

static FILE* g_log;
static FILE* g_csv;  // (argv[3]: every report)
static LARGE_INTEGER g_freq, g_last{};
static int g_moves = 0, g_gaps = 0;
static double g_worst = 0;
static std::vector<double> g_all;
// The pointer itself: when it last moved on screen, and the longest it stood still while the mouse was moving.
static POINT g_pt{};
static LARGE_INTEGER g_ptMoved{};
static int g_stalls = 0;
static double g_worstStall = 0;

static bool MenuOpen() {
    return FindWindowW(L"#32768", nullptr) && IsWindowVisible(FindWindowW(L"#32768", nullptr));
}

static LRESULT CALLBACK Proc(HWND w, UINT m, WPARAM wp, LPARAM lp) {
    if (m == WM_INPUT) {
        RAWINPUT ri;
        UINT size = sizeof(ri);
        if (GetRawInputData((HRAWINPUT)lp, RID_INPUT, &ri, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1 && ri.header.dwType == RIM_TYPEMOUSE &&
            (ri.data.mouse.lLastX || ri.data.mouse.lLastY)) {
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);
            POINT cur;
            GetCursorPos(&cur);
            // Every report, for looking at afterwards: when (ms), how far, the pointer, a menu open.
            static LARGE_INTEGER s_start = now;
            if (g_csv)
                fprintf(g_csv, "%.3f,%ld,%ld,%ld,%ld,%d\n", (now.QuadPart - s_start.QuadPart) * 1000.0 / g_freq.QuadPart, ri.data.mouse.lLastX,
                        ri.data.mouse.lLastY, cur.x, cur.y, MenuOpen() ? 1 : 0);
            if (cur.x != g_pt.x || cur.y != g_pt.y) {
                double still = g_ptMoved.QuadPart ? (now.QuadPart - g_ptMoved.QuadPart) * 1000.0 / g_freq.QuadPart : 0;
                // (moving the whole time, yet the pointer stood still this long: it hitched)
                if (still > 12 && still < 150 && g_last.QuadPart && (now.QuadPart - g_last.QuadPart) * 1000.0 / g_freq.QuadPart < 12) {
                    g_stalls++;
                    if (still > g_worstStall) g_worstStall = still;
                    fwprintf(g_log, L"pointer stood still %6.1f ms while the mouse moved  menu %ls\n", still, MenuOpen() ? L"open" : L"closed");
                }
                g_pt = cur;
                g_ptMoved = now;
            }
            if (g_last.QuadPart) {
                double ms = (now.QuadPart - g_last.QuadPart) * 1000.0 / g_freq.QuadPart;
                // (a pause in the hand isn't a hitch: only gaps between moves 150 ms apart or closer count)
                if (ms < 150) {
                    g_moves++;
                    g_all.push_back(ms);
                    if (ms > g_worst) g_worst = ms;
                    if (ms > 12) {
                        g_gaps++;
                        POINT pt;
                        GetCursorPos(&pt);
                        HWND at = WindowFromPoint(pt);
                        wchar_t cls[64] = L"", exe[MAX_PATH] = L"";
                        if (at) GetClassNameW(at, cls, 64);
                        DWORD pid = 0;
                        GetWindowThreadProcessId(at, &pid);
                        if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
                            DWORD n = MAX_PATH;
                            QueryFullProcessImageNameW(p, 0, exe, &n);
                            CloseHandle(p);
                        }
                        const wchar_t* name = wcsrchr(exe, L'\\');
                        fwprintf(g_log, L"gap %6.1f ms  menu %ls  under %ls (%ls)\n", ms, MenuOpen() ? L"open  " : L"closed", cls,
                                 name ? name + 1 : exe);
                    }
                }
            }
            g_last = now;
        }
        return 0;
    }
    if (m == WM_TIMER) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(w, m, wp, lp);
}

int wmain(int argc, wchar_t** argv) {
    int seconds = argc > 1 ? _wtoi(argv[1]) : 60;
    g_log = _wfopen(argc > 2 ? argv[2] : L"input-gap-probe.log", L"w, ccs=UTF-8");
    if (!g_log) return 1;
    if (argc > 3) g_csv = _wfopen(argv[3], L"w");
    timeBeginPeriod(1);
    QueryPerformanceFrequency(&g_freq);
    WNDCLASSW wc{};
    wc.lpfnWndProc = Proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"InputGapProbe";
    RegisterClassW(&wc);
    HWND w = CreateWindowExW(0, wc.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
    RAWINPUTDEVICE rid{1, 2, RIDEV_INPUTSINK, w};
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    SetTimer(w, 1, seconds * 1000, nullptr);
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) DispatchMessageW(&msg);
    std::sort(g_all.begin(), g_all.end());
    double median = g_all.empty() ? 0 : g_all[g_all.size() / 2], p99 = g_all.empty() ? 0 : g_all[g_all.size() * 99 / 100];
    fwprintf(g_log, L"\n%d moves, median %.2f ms, 99%% %.2f ms, worst %.1f ms, %d gaps over 12 ms; pointer stalls %d, worst %.1f ms\n", g_moves, median, p99, g_worst, g_gaps,
             g_stalls, g_worstStall);
    fclose(g_log);
    if (g_csv) fclose(g_csv);
    return 0;
}
