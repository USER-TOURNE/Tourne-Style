// Reads a real Explorer window's navigation pane from outside: every visible row (its rectangle,
// integral height and depth, through memory in Explorer's process) and a picture of the pane
// (PrintWindow, which works while the screen is locked), saved as navprobe.bmp.
// Usage: navprobe <CabinetWClass hwnd, hex>
#include <windows.h>
#include <commctrl.h>
#include <cstdio>
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

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (argc < 2) return 1;
    HWND frame = (HWND)(ULONG_PTR)wcstoull(argv[1], nullptr, 16);
    HWND tree = FindTree(frame);
    wprintf(L"tree %p\n", tree);
    if (!tree) return 1;
    DWORD pid = 0;
    GetWindowThreadProcessId(tree, &pid);
    HANDLE proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
    if (!proc) {
        wprintf(L"no access to Explorer\n");
        return 1;
    }
    void* remote = VirtualAllocEx(proc, nullptr, 4096, MEM_COMMIT, PAGE_READWRITE);
    wprintf(L"item height %d\n", (int)SendMessageW(tree, TVM_GETITEMHEIGHT, 0, 0));
    for (HTREEITEM item = (HTREEITEM)SendMessageW(tree, TVM_GETNEXTITEM, TVGN_FIRSTVISIBLE, 0); item;
         item = (HTREEITEM)SendMessageW(tree, TVM_GETNEXTITEM, TVGN_NEXTVISIBLE, (LPARAM)item)) {
        RECT r{};
        *(HTREEITEM*)&r = item;
        WriteProcessMemory(proc, remote, &r, sizeof(r), nullptr);
        SendMessageW(tree, TVM_GETITEMRECT, FALSE, (LPARAM)remote);
        ReadProcessMemory(proc, remote, &r, sizeof(r), nullptr);
        // Its integral height and text.
        TVITEMEXW tv{};
        tv.mask = TVIF_TEXT | TVIF_INTEGRAL | TVIF_HANDLE | TVIF_STATE;
        tv.stateMask = 0xFFFFFFFF;
        tv.hItem = item;
        tv.pszText = (LPWSTR)((BYTE*)remote + 512);
        tv.cchTextMax = 200;
        WriteProcessMemory(proc, remote, &tv, sizeof(tv), nullptr);
        SendMessageW(tree, TVM_GETITEMW, 0, (LPARAM)remote);
        ReadProcessMemory(proc, remote, &tv, sizeof(tv), nullptr);
        WCHAR text[200] = L"";
        ReadProcessMemory(proc, (BYTE*)remote + 512, text, sizeof(text) - 2, nullptr);
        int depth = 0;
        for (HTREEITEM p = (HTREEITEM)SendMessageW(tree, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)item); p;
             p = (HTREEITEM)SendMessageW(tree, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)p))
            depth++;
        wprintf(L"row y %4ld..%4ld h %3ld integral %d depth %d  %ls\n", r.top, r.bottom, r.bottom - r.top, tv.iIntegral, depth, text);
        if (r.top > 1400) break;
    }
    VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
    CloseHandle(proc);
    // The pane's picture.
    RECT wr;
    GetWindowRect(tree, &wr);
    int w = wr.right - wr.left, h = wr.bottom - wr.top;
    BITMAPINFO bi{{sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB}};
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, bmp);
    BOOL ok = PrintWindow(tree, dc, PW_RENDERFULLCONTENT);
    wprintf(L"printed %d (%dx%d)\n", ok, w, h);
    GdiFlush();
    int stride = (w * 3 + 3) & ~3;
    std::vector<BYTE> out((size_t)stride * h);
    auto* px = (uint32_t*)bits;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            uint32_t p = px[(size_t)y * w + x];
            BYTE* o = &out[(size_t)(h - 1 - y) * stride + x * 3];
            o[0] = p & 0xFF, o[1] = (p >> 8) & 0xFF, o[2] = (p >> 16) & 0xFF;
        }
    BITMAPINFOHEADER h24{sizeof(h24), w, h, 1, 24, BI_RGB};
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(h24) + out.size()), 0, 0, sizeof(fh) + sizeof(h24)};
    FILE* f = _wfopen(L"navprobe.bmp", L"wb");
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&h24, sizeof(h24), 1, f);
    fwrite(out.data(), out.size(), 1, f);
    fclose(f);
    SelectObject(dc, old);
    DeleteObject(bmp);
    DeleteDC(dc);
    return 0;
}
