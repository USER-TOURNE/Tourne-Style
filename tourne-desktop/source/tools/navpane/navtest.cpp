// Hosts Explorer's navigation pane (an ExplorerBrowser with its frames) in a window of its own, then
// prints the tree's rows (height, depth, text) and saves the pane as navpane.bmp, to see where
// Windows draws the separator lines between the pane's groups.
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <cstdio>
#include <string>
#include <vector>

static HWND FindTree(HWND root) {
    HWND found = nullptr;
    EnumChildWindows(
        root,
        [](HWND h, LPARAM p) -> BOOL {
            WCHAR cls[64];
            GetClassNameW(h, cls, 64);
            if (!wcscmp(cls, L"SysTreeView32")) {
                *(HWND*)p = h;
                return FALSE;
            }
            return TRUE;
        },
        (LPARAM)&found);
    return found;
}

static void SaveBmp(PCWSTR path, HWND h) {
    RECT r;
    GetClientRect(h, &r);
    int w = r.right, hh = r.bottom;
    BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -hh, 1, 32, BI_RGB}};
    void* bits = nullptr;
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bmp);
    SendMessageW(h, WM_PRINTCLIENT, (WPARAM)dc, PRF_CLIENT | PRF_ERASEBKGND);
    GdiFlush();
    int stride = (w * 3 + 3) & ~3;
    std::vector<BYTE> out((size_t)stride * hh);
    auto* px = (uint32_t*)bits;
    for (int y = 0; y < hh; y++)
        for (int x = 0; x < w; x++) {
            uint32_t p = px[(size_t)y * w + x];
            BYTE* o = &out[(size_t)(hh - 1 - y) * stride + x * 3];
            o[0] = p & 0xFF, o[1] = (p >> 8) & 0xFF, o[2] = (p >> 16) & 0xFF;
        }
    BITMAPINFOHEADER h24{sizeof(h24), w, hh, 1, 24, BI_RGB};
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(h24) + out.size()), 0, 0, sizeof(fh) + sizeof(h24)};
    FILE* f = _wfopen(path, L"wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&h24, sizeof(h24), 1, f);
    fwrite(out.data(), out.size(), 1, f);
    fclose(f);
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
}

static void Pump(DWORD ms) {
    MSG m;
    for (DWORD until = GetTickCount() + ms; GetTickCount() < until; Sleep(10))
        while (PeekMessageW(&m, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
}

int wmain() {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    OleInitialize(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"NavTestHost";
    RegisterClassW(&wc);
    HWND host = CreateWindowExW(0, L"NavTestHost", L"navtest", WS_OVERLAPPEDWINDOW, 0, 0, 900, 900, nullptr, nullptr,
                                wc.hInstance, nullptr);
    IExplorerBrowser* browser = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ExplorerBrowser, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&browser)))) {
        wprintf(L"no ExplorerBrowser\n");
        return 1;
    }
    RECT rc{0, 0, 900, 900};
    FOLDERSETTINGS fs{FVM_DETAILS, 0};
    browser->Initialize(host, &rc, &fs);
    browser->SetOptions(EBO_SHOWFRAMES | EBO_NOBORDER);
    PIDLIST_ABSOLUTE pidl = nullptr;
    SHGetKnownFolderIDList(FOLDERID_ComputerFolder, 0, nullptr, &pidl);
    HRESULT hr = browser->BrowseToIDList(pidl, SBSP_ABSOLUTE);
    wprintf(L"browse %08lx\n", hr);
    ShowWindow(host, SW_SHOWNOACTIVATE);
    Pump(3000);
    HWND tree = FindTree(host);
    wprintf(L"tree %p\n", tree);
    if (!tree) return 1;
    wprintf(L"item height %d, style %08lx\n", (int)SendMessageW(tree, TVM_GETITEMHEIGHT, 0, 0),
            (DWORD)GetWindowLongPtrW(tree, GWL_STYLE));
    SendMessageW(tree, WM_VSCROLL, SB_TOP, 0);
    Pump(500);
    // Every visible row.
    for (HTREEITEM item = TreeView_GetFirstVisible(tree); item; item = TreeView_GetNextVisible(tree, item)) {
        RECT r{};
        *(HTREEITEM*)&r = item;
        SendMessageW(tree, TVM_GETITEMRECT, FALSE, (LPARAM)&r);
        WCHAR text[128] = L"";
        TVITEMEXW tv{};
        tv.mask = TVIF_TEXT | TVIF_INTEGRAL | TVIF_HANDLE;
        tv.hItem = item;
        tv.pszText = text;
        tv.cchTextMax = 128;
        SendMessageW(tree, TVM_GETITEMW, 0, (LPARAM)&tv);
        int depth = 0;
        for (HTREEITEM p = TreeView_GetParent(tree, item); p; p = TreeView_GetParent(tree, p)) depth++;
        wprintf(L"row y %4ld..%4ld h %3ld integral %d depth %d  %ls\n", r.top, r.bottom, r.bottom - r.top, tv.iIntegral, depth,
                text);
        if (r.top > 900) break;
    }
    SaveBmp(L"navpane.bmp", tree);
    browser->Destroy();
    browser->Release();
    return 0;
}
