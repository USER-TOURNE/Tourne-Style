// Shows Windows' view of the Desktop folder the way desktop-host.cpp does, off screen, and prints the
// icons it ends up with, with and without the filter (ICommDlgBrowser::IncludeObject), so the filter
// can be checked without touching the running desktop.
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode
//     -DUNICODE -D_UNICODE -O1 tools\desktop\desktop-view-test.cpp -o tools\desktop\desktop-view-test.exe
//     -lshell32 -lole32 -luuid -lcomctl32 -lshlwapi -ladvapi32
// Run: desktop-view-test.exe [filter]

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <cstdio>
#include <string>

constexpr GUID kTopLevelBrowser = {0x4C96BE40, 0x915C, 0x11CF, {0x99, 0xD3, 0x00, 0xAA, 0x00, 0x4A, 0xE8, 0x37}};

bool g_filter = false;

std::wstring ParsingName(IShellFolder* folder, PCUITEMID_CHILD item) {
    STRRET ret;
    wchar_t* text = nullptr;
    if (FAILED(folder->GetDisplayNameOf(item, SHGDN_FORPARSING, &ret)) || FAILED(StrRetToStrW(&ret, item, &text))) return L"";
    std::wstring out = text;
    CoTaskMemFree(text);
    return out;
}

std::wstring KnownPath(REFKNOWNFOLDERID id) {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path))) out = path;
    CoTaskMemFree(path);
    return out;
}

// What desktop-host.cpp shows: files and folders that are in a Desktop folder (yours or the public
// one); Windows' own items only where Explorer's desktop icons settings show them
// (HideDesktopIcons\NewStartPanel = 0, as the Recycle Bin is by default).
bool DesktopShows(std::wstring const& parsing) {
    if (parsing.rfind(L"::{", 0) == 0) {
        std::wstring clsid = parsing.substr(2, parsing.find(L'}') - 1);
        DWORD value = 1, size = sizeof(value);
        bool set = RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel",
                                clsid.c_str(), RRF_RT_REG_DWORD, nullptr, &value, &size) == ERROR_SUCCESS;
        return set && value == 0;
    }
    std::wstring folder = parsing.substr(0, parsing.find_last_of(L'\\'));
    return !_wcsicmp(folder.c_str(), KnownPath(FOLDERID_Desktop).c_str()) ||
           !_wcsicmp(folder.c_str(), KnownPath(FOLDERID_PublicDesktop).c_str());
}

class Browser : public IShellBrowser, public IServiceProvider, public ICommDlgBrowser {
   public:
    explicit Browser(HWND wnd) : wnd_(wnd) {}
    IShellView* view = nullptr;
    STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleWindow) || IsEqualIID(riid, IID_IShellBrowser))
            *out = static_cast<IShellBrowser*>(this);
        else if (IsEqualIID(riid, IID_IServiceProvider))
            *out = static_cast<IServiceProvider*>(this);
        else if (g_filter && IsEqualIID(riid, IID_ICommDlgBrowser))
            *out = static_cast<ICommDlgBrowser*>(this);
        else {
            *out = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&refs_); }
    STDMETHODIMP_(ULONG) Release() override { return InterlockedDecrement(&refs_); }
    STDMETHODIMP GetWindow(HWND* out) override { return *out = wnd_, S_OK; }
    STDMETHODIMP ContextSensitiveHelp(BOOL) override { return E_NOTIMPL; }
    STDMETHODIMP InsertMenusSB(HMENU, LPOLEMENUGROUPWIDTHS) override { return E_NOTIMPL; }
    STDMETHODIMP SetMenuSB(HMENU, HOLEMENU, HWND) override { return E_NOTIMPL; }
    STDMETHODIMP RemoveMenusSB(HMENU) override { return E_NOTIMPL; }
    STDMETHODIMP SetStatusTextSB(LPCWSTR) override { return S_OK; }
    STDMETHODIMP EnableModelessSB(BOOL) override { return S_OK; }
    STDMETHODIMP TranslateAcceleratorSB(MSG*, WORD) override { return S_FALSE; }
    STDMETHODIMP BrowseObject(PCUIDLIST_RELATIVE, UINT) override { return E_FAIL; }
    STDMETHODIMP GetViewStateStream(DWORD, IStream** out) override { return *out = nullptr, E_NOTIMPL; }
    STDMETHODIMP GetControlWindow(UINT, HWND* out) override { return *out = nullptr, E_NOTIMPL; }
    STDMETHODIMP SendControlMsg(UINT, UINT, WPARAM, LPARAM, LRESULT* r) override { if (r) *r = 0; return E_NOTIMPL; }
    STDMETHODIMP QueryActiveShellView(IShellView** out) override {
        *out = view;
        if (!view) return E_FAIL;
        view->AddRef();
        return S_OK;
    }
    STDMETHODIMP OnViewWindowActive(IShellView*) override { return S_OK; }
    STDMETHODIMP SetToolbarItems(LPTBBUTTONSB, UINT, UINT) override { return S_OK; }
    STDMETHODIMP QueryService(REFGUID service, REFIID riid, void** out) override {
        if (IsEqualGUID(service, kTopLevelBrowser) || IsEqualGUID(service, IID_IShellBrowser) ||
            (g_filter && IsEqualGUID(service, IID_ICommDlgBrowser)))
            return QueryInterface(riid, out);
        *out = nullptr;
        return E_NOINTERFACE;
    }
    // ICommDlgBrowser
    STDMETHODIMP OnDefaultCommand(IShellView*) override { return S_FALSE; }  // (the view opens it as usual)
    STDMETHODIMP OnStateChange(IShellView*, ULONG) override { return S_OK; }
    STDMETHODIMP IncludeObject(IShellView*, PCUITEMID_CHILD item) override {
        IShellFolder* desktop = nullptr;
        if (FAILED(SHGetDesktopFolder(&desktop))) return S_OK;
        std::wstring parsing = ParsingName(desktop, item);
        desktop->Release();
        bool shows = DesktopShows(parsing);
        wprintf(L"  asked: %-60ls %ls\n", parsing.c_str(), shows ? L"show" : L"hide");
        return shows ? S_OK : S_FALSE;
    }

   private:
    HWND wnd_;
    LONG refs_ = 1;
};

int wmain(int argc, wchar_t** argv) {
    g_filter = argc > 1 && !wcscmp(argv[1], L"filter");
    OleInitialize(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneDesktopViewTest";
    RegisterClassW(&wc);
    HWND wnd = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, L"", WS_POPUP | WS_CLIPCHILDREN, -4000, -4000, 1200, 900,
                               nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(wnd, SW_SHOWNOACTIVATE);
    IShellFolder* desktop = nullptr;
    SHGetDesktopFolder(&desktop);
    IShellView* view = nullptr;
    desktop->CreateViewObject(wnd, IID_IShellView, (void**)&view);
    auto* browser = new Browser(wnd);
    FOLDERSETTINGS settings{FVM_ICON, FWF_DESKTOP | FWF_TRANSPARENT | FWF_NOCLIENTEDGE | FWF_NOSCROLL | FWF_BESTFITWINDOW | FWF_SNAPTOGRID};
    RECT rc{0, 0, 1200, 900};
    HWND viewWnd = nullptr;
    HRESULT hr = view->CreateViewWindow(nullptr, &settings, browser, &rc, &viewWnd);
    browser->view = view;
    view->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
    wprintf(L"filter %ls, view 0x%08lx\n", g_filter ? L"on" : L"off", hr);
    DWORD until = GetTickCount() + 3000;
    MSG msg;
    while (GetTickCount() < until) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        Sleep(20);
    }
    HWND list = viewWnd ? FindWindowExW(viewWnd, nullptr, L"SysListView32", nullptr) : nullptr;
    int count = list ? (int)SendMessageW(list, LVM_GETITEMCOUNT, 0, 0) : -1;
    wprintf(L"icons shown: %d\n", count);
    for (int i = 0; i < count; i++) {
        wchar_t text[260] = L"";
        LVITEMW item{};
        item.iSubItem = 0;
        item.pszText = text;
        item.cchTextMax = 260;
        SendMessageW(list, LVM_GETITEMTEXTW, i, (LPARAM)&item);
        wprintf(L"  %ls\n", text);
    }
    fflush(stdout);
    ExitProcess(0);
}
