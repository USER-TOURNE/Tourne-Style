// Windows' virtual desktops, through the manager Task view uses ("// @inline virtual-desktops.cpp", in
// desktops-pager.cpp and binds.cpp): switch to one, rename one, send a window to one. Plus the
// desktops' ids and the current one, as Windows keeps them in the registry.

////////////////////////////////////////////////////////////////////////////////
// Windows' own desktop manager, the one Task view uses (IVirtualDesktopManagerInternal, undocumented, from
// the immersive shell). Its layout changes between Windows builds and its id changes with it, so the id
// here (24H2's and 25H2's, read from twinui.pcshell.dll with the slots below) only answers on a build
// where those slots are what they say; anywhere else nothing is called.

constexpr CLSID kClsidImmersiveShell = {0xC2F03A33, 0x21F5, 0x47FA, {0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39}};
constexpr GUID kSidVdmInternal = {0xC5E0CDCA, 0x7B6E, 0x41B2, {0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B}};
constexpr IID kIidVdmInternal = {0x53F5CA0B, 0x158F, 0x4124, {0x90, 0x0C, 0x05, 0x71, 0x58, 0x06, 0x0B, 0x27}};
constexpr int kVdmMoveView = 4, kVdmSwitchDesktop = 9, kVdmSwitchAndMoveForeground = 10, kVdmCreateDesktop = 11,
              kVdmFindDesktop = 14, kVdmSetDesktopName = 16;
constexpr int kVdGetId = 4;  // IVirtualDesktop's own, on those builds
// The shell's windows as it sees them (IApplicationViewCollection, the same on those builds), for a window's view.
constexpr IID kIidViewCollection = {0x1841C6D7, 0x4F9D, 0x42C0, {0xAF, 0x41, 0x87, 0x47, 0x53, 0x8F, 0x10, 0xE5}};
constexpr int kViewForHwnd = 6;

template <typename F>
F VdmSlot(IUnknown* p, int slot) {
    return (*(F**)p)[slot];
}

// The desktop with this id and the manager (both released by the caller); false if either isn't there.
bool VdmFind(GUID const& id, IUnknown** vdm, IUnknown** desktop) {
    *vdm = *desktop = nullptr;
    IServiceProvider* shell = nullptr;
    if (FAILED(CoCreateInstance(kClsidImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&shell)))) return false;
    shell->QueryService(kSidVdmInternal, kIidVdmInternal, (void**)vdm);
    shell->Release();
    if (!*vdm) return false;
    using Find = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, GUID const*, IUnknown**);
    if (SUCCEEDED(VdmSlot<Find>(*vdm, kVdmFindDesktop)(*vdm, &id, desktop)) && *desktop) return true;
    (*vdm)->Release();
    *vdm = nullptr;
    return false;
}

// Straight to a desktop, as Task view goes (no keys pressed for it, so keys held for a hotkey don't matter); VdmSwitchTo
// (desktop-slide.cpp, below) animates it when a motion line says so.
bool VdmSwitchNow(GUID const& id) {
    IUnknown *vdm, *desktop;
    if (!VdmFind(id, &vdm, &desktop)) return false;
    using Switch = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, IUnknown*);
    bool ok = SUCCEEDED(VdmSlot<Switch>(vdm, kVdmSwitchDesktop)(vdm, desktop));
    desktop->Release();
    vdm->Release();
    return ok;
}

// A new desktop at the end, as Task view's "New desktop" makes it; its id, or GUID_NULL if Windows didn't make one.
GUID VdmCreate() {
    GUID id{};
    IServiceProvider* shell = nullptr;
    if (FAILED(CoCreateInstance(kClsidImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&shell)))) return id;
    IUnknown* vdm = nullptr;
    shell->QueryService(kSidVdmInternal, kIidVdmInternal, (void**)&vdm);
    shell->Release();
    if (!vdm) return id;
    using Create = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, IUnknown**);
    using GetId = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, GUID*);
    IUnknown* desktop = nullptr;
    if (SUCCEEDED(VdmSlot<Create>(vdm, kVdmCreateDesktop)(vdm, &desktop)) && desktop) {
        VdmSlot<GetId>(desktop, kVdGetId)(desktop, &id);
        desktop->Release();
    }
    vdm->Release();
    return id;
}

// Renamed as Task view renames it (empty: back to "Desktop N"). The name is an HSTRING, made by combase.
bool VdmRename(GUID const& id, std::wstring const& name) {
    using Create = HRESULT(WINAPI*)(PCWSTR, UINT32, void**);
    using Delete = HRESULT(WINAPI*)(void*);
    HMODULE combase = GetModuleHandleW(L"combase.dll");
    auto create = combase ? (Create)(void*)GetProcAddress(combase, "WindowsCreateString") : nullptr;
    auto destroy = combase ? (Delete)(void*)GetProcAddress(combase, "WindowsDeleteString") : nullptr;
    IUnknown *vdm, *desktop;
    if (!create || !destroy || !VdmFind(id, &vdm, &desktop)) return false;
    void* text = nullptr;
    using SetName = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, IUnknown*, void*);
    bool ok = SUCCEEDED(create(name.c_str(), (UINT32)name.size(), &text)) &&
              SUCCEEDED(VdmSlot<SetName>(vdm, kVdmSetDesktopName)(vdm, desktop, text));
    destroy(text);
    desktop->Release();
    vdm->Release();
    return ok;
}

// A window sent to a desktop, as dragging it there in Task view does; with follow, switched to with it (the window
// staying in front). False if Windows' manager doesn't answer or won't move it.
bool VdmSendWindow(HWND h, GUID const& id, bool follow) {
    IServiceProvider* shell = nullptr;
    if (FAILED(CoCreateInstance(kClsidImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&shell)))) return false;
    IUnknown* views = nullptr;
    shell->QueryService(kIidViewCollection, kIidViewCollection, (void**)&views);
    shell->Release();
    if (!views) return false;
    IUnknown* view = nullptr;
    using ViewFor = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, HWND, IUnknown**);
    HRESULT hr = VdmSlot<ViewFor>(views, kViewForHwnd)(views, h, &view);
    views->Release();
    if (FAILED(hr) || !view) return false;
    bool ok = false;
    IUnknown *vdm, *desktop;
    if (VdmFind(id, &vdm, &desktop)) {
        using Along = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, IUnknown*);
        using Move = HRESULT(STDMETHODCALLTYPE*)(IUnknown*, IUnknown*, IUnknown*);
        if (follow && h == GetAncestor(GetForegroundWindow(), GA_ROOTOWNER))
            ok = SUCCEEDED(VdmSlot<Along>(vdm, kVdmSwitchAndMoveForeground)(vdm, desktop));
        if (!ok) {
            ok = SUCCEEDED(VdmSlot<Move>(vdm, kVdmMoveView)(vdm, view, desktop));
            if (ok && follow) VdmSlot<Along>(vdm, kVdmSwitchDesktop)(vdm, desktop);
        }
        desktop->Release();
        vdm->Release();
    }
    view->Release();
    return ok;
}

// The desktops' ids (none until a second desktop has ever been made), or the current one's.
std::vector<GUID> VdReadGuids(wchar_t const* value) {
    std::vector<GUID> ids;
    BYTE data[16 * 64];
    DWORD size = sizeof(data);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops", value,
                     RRF_RT_REG_BINARY, nullptr, data, &size) != ERROR_SUCCESS)
        return ids;
    for (DWORD at = 0; at + 16 <= size; at += 16) {
        GUID g;
        memcpy(&g, data + at, 16);
        ids.push_back(g);
    }
    return ids;
}

// The current desktop's place among them (0 when there is only one).
int VdCurrentIndex(std::vector<GUID> const& ids) {
    std::vector<GUID> current = VdReadGuids(L"CurrentVirtualDesktop");
    for (size_t i = 0; i < ids.size() && !current.empty(); i++)
        if (!memcmp(&ids[i], &current[0], sizeof(GUID))) return (int)i;
    return 0;
}

// @inline desktop-slide.cpp
