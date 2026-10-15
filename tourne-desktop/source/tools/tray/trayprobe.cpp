// Asks Tourne Desktop's tray (shell mode) for its icons, as a bar does, and
// prints each one's program, id, version and tooltip: for finding why an icon
// shows no tooltip or the wrong picture.
//
//   clang++ -std=c++23 -municode -static tools/tray/trayprobe.cpp -luser32 -o trayprobe.exe
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

std::vector<BYTE> g_packet;

LRESULT CALLBACK Proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_COPYDATA) {
        auto* c = (COPYDATASTRUCT*)lp;
        if (c->dwData == 0x54484931 && c->lpData) g_packet.assign((BYTE*)c->lpData, (BYTE*)c->lpData + c->cbData);
        return TRUE;
    }
    return DefWindowProcW(wnd, msg, wp, lp);
}

std::string Utf8(const std::wstring& w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

int wmain() {
    SetConsoleOutputCP(CP_UTF8);
    WNDCLASSW wc{};
    wc.lpfnWndProc = Proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneTaskbarTest_probe";  // (the tray answers bars only)
    RegisterClassW(&wc);
    HWND me = CreateWindowExW(0, wc.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
    UINT ask = RegisterWindowMessageW(L"TourneSuite_RelayOverflow");
    DWORD_PTR r = 0;
    SendMessageTimeoutW(tray, ask, (WPARAM)me, 1, SMTO_ABORTIFHUNG, 3000, &r);
    MSG m;
    while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&m);
    if (g_packet.size() < 4) return printf("no answer from the tray\n"), 1;
    size_t at = 4;
    UINT count = *(UINT*)g_packet.data();
    printf("%u icons\n", count);
    for (UINT i = 0; i < count && at < g_packet.size(); i++) {
        uint64_t owner;
        UINT id, callback, version, flags, size, chars;
        memcpy(&owner, &g_packet[at], 8), at += 8;
        memcpy(&id, &g_packet[at], 4), at += 4;
        memcpy(&callback, &g_packet[at], 4), at += 4;
        memcpy(&version, &g_packet[at], 4), at += 4;
        memcpy(&flags, &g_packet[at], 4), at += 4;
        memcpy(&size, &g_packet[at], 4), at += 4;
        memcpy(&chars, &g_packet[at], 4), at += 4;
        std::wstring tip((wchar_t*)&g_packet[at], chars);
        at += chars * 2 + (size_t)size * size * 4;
        DWORD pid = 0;
        GetWindowThreadProcessId((HWND)(ULONG_PTR)owner, &pid);
        wchar_t path[MAX_PATH] = L"?";
        if (HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
            DWORD n = MAX_PATH;
            QueryFullProcessImageNameW(p, 0, path, &n);
            CloseHandle(p);
        }
        const wchar_t* name = wcsrchr(path, L'\\') ? wcsrchr(path, L'\\') + 1 : path;
        printf("  %-28s id %-5u v%u size %-3u tip [%s]\n", Utf8(name).c_str(), id, version, size, Utf8(tip).c_str());
    }
    return 0;
}
