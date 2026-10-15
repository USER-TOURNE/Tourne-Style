constexpr GUID kTopLevelBrowser = {0x4C96BE40, 0x915C, 0x11CF, {0x99, 0xD3, 0x00, 0xAA, 0x00, 0x4A, 0xE8, 0x37}};
constexpr COLORREF kText = RGB(0x64, 0xAA, 0x89), kFill = RGB(0x1E, 0x36, 0x32), kEdge = RGB(0x15, 0x20, 0x26);
bool g_look = false;
int g_prepaints = 0, g_items = 0;

class Browser : public IShellBrowser, public IServiceProvider {
   public:
    explicit Browser(HWND wnd) : wnd_(wnd) {}
    IShellView* view = nullptr;
    STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IOleWindow) || IsEqualIID(riid, IID_IShellBrowser))
            *out = static_cast<IShellBrowser*>(this);
        else if (IsEqualIID(riid, IID_IServiceProvider))
            *out = static_cast<IServiceProvider*>(this);
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
        if (IsEqualGUID(service, kTopLevelBrowser) || IsEqualGUID(service, IID_IShellBrowser)) return QueryInterface(riid, out);
        *out = nullptr;
        return E_NOINTERFACE;
    }

   private:
    HWND wnd_;
    LONG refs_ = 1;
};
