// Which uxtheme calls draw the desktop icons' selection, hot and focus looks in Windows' view of the Desktop
// folder (as desktop-host.cpp shows it): hooks OpenThemeData*, DrawThemeBackground(Ex) and DrawFocusRect in
// this process, selects and hovers icon 0, prints the calls. Built with desktop-look-test.cpp's Browser.
#define main_skip
#include <windows.h>
#include <uxtheme.h>
#include <cstdio>
#include <map>
#include <string>
#include "MinHook.h"
std::map<HTHEME, std::wstring> g_names;
decltype(&OpenThemeData) oOpen;
decltype(&OpenThemeDataForDpi) oOpenDpi;
decltype(&DrawThemeBackground) oDraw;
decltype(&DrawThemeBackgroundEx) oDrawEx;
decltype(&DrawFocusRect) oFocus;
HTHEME WINAPI hOpen(HWND w, LPCWSTR c) { HTHEME t = oOpen(w, c); WCHAR k[64] = L""; GetClassNameW(w, k, 64); g_names[t] = std::wstring(c) + L" @" + k; return t; }
HTHEME WINAPI hOpenDpi(HWND w, LPCWSTR c, UINT d) { HTHEME t = oOpenDpi(w, c, d); WCHAR k[64] = L""; if (w) GetClassNameW(w, k, 64); g_names[t] = std::wstring(c) + L" @" + k; return t; }
HRESULT WINAPI hDraw(HTHEME t, HDC dc, int p, int s, LPCRECT r, LPCRECT c) {
    auto it = g_names.find(t);
    wprintf(L"DrawThemeBackground %ls part %d state %d rc %ld,%ld,%ld,%ld\n", it == g_names.end() ? L"?" : it->second.c_str(), p, s, r->left, r->top, r->right, r->bottom);
    return oDraw(t, dc, p, s, r, c);
}
HRESULT WINAPI hDrawEx(HTHEME t, HDC dc, int p, int s, LPCRECT r, const DTBGOPTS* o) {
    auto it = g_names.find(t);
    wprintf(L"DrawThemeBackgroundEx %ls part %d state %d rc %ld,%ld,%ld,%ld\n", it == g_names.end() ? L"?" : it->second.c_str(), p, s, r->left, r->top, r->right, r->bottom);
    return oDrawEx(t, dc, p, s, r, o);
}
BOOL WINAPI hFocus(HDC dc, const RECT* r) { wprintf(L"DrawFocusRect %ld,%ld,%ld,%ld\n", r->left, r->top, r->right, r->bottom); return oFocus(dc, r); }
#include <commctrl.h>
#include <shlobj.h>
#include "browser.inc"
#include "probe2.inc"
void Pump(DWORD ms) { DWORD u = GetTickCount() + ms; MSG m; while (GetTickCount() < u) { while (PeekMessageW(&m, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&m); DispatchMessageW(&m); } Sleep(10); } }
int wmain(int argc, wchar_t** argv) {
    if (argc > 1) { ACTCTXW ctx{sizeof(ctx)}; ctx.lpSource = argv[1]; HANDLE h = CreateActCtxW(&ctx); ULONG_PTR ck; ActivateActCtx(h, &ck); }
    MH_Initialize();
    HMODULE ux = LoadLibraryW(L"uxtheme.dll");
    MH_CreateHook((void*)GetProcAddress(ux, "OpenThemeData"), (void*)hOpen, (void**)&oOpen);
    MH_CreateHook((void*)GetProcAddress(ux, "OpenThemeDataForDpi"), (void*)hOpenDpi, (void**)&oOpenDpi);
    MH_CreateHook((void*)GetProcAddress(ux, "DrawThemeBackground"), (void*)hDraw, (void**)&oDraw);
    MH_CreateHook((void*)GetProcAddress(ux, "DrawThemeBackgroundEx"), (void*)hDrawEx, (void**)&oDrawEx);
    MH_CreateHook((void*)DrawFocusRect, (void*)hFocus, (void**)&oFocus);
    MH_CreateHook((void*)FillRect, (void*)hFill, (void**)&oFill);
    MH_EnableHook(MH_ALL_HOOKS);
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES}; InitCommonControlsEx(&icc);
    OleInitialize(nullptr);
    WNDCLASSW wc{}; wc.lpfnWndProc = DefWindowProcW; wc.hInstance = GetModuleHandleW(0); wc.lpszClassName = L"ThemeProbe"; RegisterClassW(&wc);
    HWND wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_LAYERED, wc.lpszClassName, L"", WS_POPUP | WS_CLIPCHILDREN, 0, 0, 900, 500, 0, 0, wc.hInstance, 0);
    SetLayeredWindowAttributes(wnd, 0, 1, LWA_ALPHA);
    ShowWindow(wnd, SW_SHOWNOACTIVATE);
    IShellFolder* desktop; SHGetDesktopFolder(&desktop);
    IShellView* view; desktop->CreateViewObject(wnd, IID_IShellView, (void**)&view);
    auto* browser = new Browser(wnd);
    FOLDERSETTINGS fs{FVM_ICON, FWF_DESKTOP | FWF_TRANSPARENT | FWF_NOCLIENTEDGE | FWF_NOSCROLL | FWF_BESTFITWINDOW | FWF_SNAPTOGRID};
    RECT rc{0, 0, 900, 500}; HWND vw = nullptr;
    view->CreateViewWindow(nullptr, &fs, browser, &rc, &vw); browser->view = view; view->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
    Pump(2000);
    HWND list = FindWindowExW(vw, nullptr, L"SysListView32", nullptr);
    HWND fg = GetForegroundWindow(); DWORD t = GetWindowThreadProcessId(fg, 0);
    AttachThreadInput(GetCurrentThreadId(), t, TRUE); SetForegroundWindow(wnd); SetFocus(list); AttachThreadInput(GetCurrentThreadId(), t, FALSE);
    Pump(300);
    wprintf(L"focus is list: %d\n", GetFocus() == list);
    LVITEMW st{}; st.stateMask = LVIS_SELECTED | LVIS_FOCUSED; st.state = st.stateMask;
    SendMessageW(list, LVM_SETITEMSTATE, 0, (LPARAM)&st);
    wprintf(L"--- select 0 (state 0x%x)\n", (UINT)SendMessageW(list, LVM_GETITEMSTATE, 0, 3));
    RedrawWindow(list, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW); Pump(300);
    wprintf(L"--- unfocused\n");
    SetFocus(wnd); RedrawWindow(list, 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW); Pump(300);
    wprintf(L"--- hot 1\n");
    SendMessageW(list, LVM_SETHOTITEM, 1, 0); RedrawWindow(list, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW); Pump(300);
    fflush(stdout); ExitProcess(0);
}
