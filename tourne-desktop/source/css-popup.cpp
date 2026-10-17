// The popups of a YASB theme's widgets: the calendar under the clock, the volume slider and output list under the volume,
// and the power menu. Each is a small window of its own, its nodes styled by the theme's own rules (.calendar, .day-label,
// .audio-menu, .volume-slider, .power-menu-popup .button...) and drawn like the bar. Inlined after css-bar.cpp.

enum class CssPopupKind { None, Calendar, Volume, Power };

struct CssPopup {
    HWND hwnd = nullptr;
    CssPopupKind kind = CssPopupKind::None;
    RECT anchor{};  // the widget it belongs to, on the screen
    struct Item {
        RECT rc;
        std::wstring action;
    };
    std::vector<Item> items;
    int hot = -1;
    int monthOffset = 0;
    bool dragging = false;
    RECT slider{};    // the volume slider's track
    ULONGLONG shownAt = 0, closedAt = 0;
    CssPopupKind closedKind = CssPopupKind::None;
    double scale = 1;
    int width = 0, height = 0;
};

static CssPopup g_popup;
static HINSTANCE g_popupInstance = nullptr;
constexpr WCHAR kPopupClass[] = L"TourneCssPopup";
constexpr UINT_PTR kTimerPopupWatch = 1;

// The widget of the theme's layout with this in its type, for its options.
static yaml::Node const* CssFindWidget(wchar_t const* typePart, std::wstring* nameOut = nullptr) {
    yaml::Node const* bar = CssBarConfig();
    yaml::Node const* widgets = bar ? bar->Get(L"widgets") : nullptr;
    if (!widgets) return nullptr;
    for (wchar_t const* side : {L"left", L"center", L"right"})
        if (auto const* list = widgets->Get(side))
            for (auto const& item : list->seq)
                if (auto const* def = CssWidgetDef(item.s))
                    if (CssIsType(*def, typePart)) {
                        if (nameOut) *nameOut = item.s;
                        return def;
                    }
    return nullptr;
}

// Acrylic (or none) behind the popup, and its corners, as its options ask.
static void CssPopupBackdrop(HWND wnd, yaml::Node const* opts) {
    struct AccentPolicy {
        int state, flags;
        DWORD color;
        int animation;
    };
    struct CompositionData {
        int attribute;
        void* data;
        SIZE_T size;
    };
    using SetWindowCompositionAttribute_t = BOOL(WINAPI*)(HWND, CompositionData*);
    static auto set = (SetWindowCompositionAttribute_t)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute");
    bool blur = opts && opts->Bool(L"blur", false), round = !opts || opts->Bool(L"round_corners", true);
    std::wstring type = opts ? opts->Str(L"round_corners_type", L"normal") : L"normal";
    DWORD corners = !round ? 1 : type == L"small" ? 3 : 2;
    DwmSetWindowAttribute(wnd, 33, &corners, sizeof(corners));
    COLORREF border = 0xFFFFFFFE;
    std::wstring want = css::Lower(opts ? opts->Str(L"border_color", L"None") : L"None");
    css::Rgba c;
    if (want == L"system") border = 0xFFFFFFFF;
    else if (want.size() > 1 && want[0] == L'#' && css::ParseColor(want, &c)) border = RGB(c.r, c.g, c.b);
    DwmSetWindowAttribute(wnd, 34, &border, sizeof(border));
    if (!set) return;
    AccentPolicy policy{blur ? 4 : 0, 0, 0x01000000, 0};
    CompositionData data{19, &policy, sizeof(policy)};
    set(wnd, &data);
}

// ---------------------------------------------------------------------------------------------------- the trees

struct CssPopupBuild {
    std::unique_ptr<css::Node> root;
    std::vector<std::pair<css::Node*, std::wstring>> actions;  // the nodes that can be pressed
    css::Node* handle = nullptr;                                // the volume slider's handle
    css::Node* track = nullptr;
    int volume = 0;
};

static css::Node* CssPopupRoot(CssPopupBuild& b, std::initializer_list<std::wstring> classes) {
    b.root = std::make_unique<css::Node>();
    b.root->classes = classes;
    if (CssDarkMode()) b.root->classes.push_back(L"dark");
    return b.root.get();
}

static void CssBuildCalendar(CssPopupBuild& b, int monthOffset, int hot) {
    SYSTEMTIME now;
    GetLocalTime(&now);
    css::Node* root = CssPopupRoot(b, {L"calendar"});
    // The month shown: this one, moved by the wheel.
    int month = (now.wYear * 12 + (now.wMonth - 1)) + monthOffset;
    SYSTEMTIME first{};
    first.wYear = (WORD)(month / 12);
    first.wMonth = (WORD)(month % 12 + 1);
    first.wDay = 1;
    FILETIME ft;
    SystemTimeToFileTime(&first, &ft);
    FileTimeToSystemTime(&ft, &first);  // fills wDayOfWeek
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int count = days[first.wMonth - 1] + (first.wMonth == 2 && ((first.wYear % 4 == 0 && first.wYear % 100 != 0) || first.wYear % 400 == 0) ? 1 : 0);
    int startCol = (first.wDayOfWeek + 6) % 7;  // Monday first, as the theme's calendar
    css::Node* left = root->Add({L"calendar-left"});
    left->vertical = true;
    left->Add({L"day-label"}, FormatDateText(now, L"dddd"));
    left->Add({L"date-label"}, std::to_wstring(now.wDay));
    left->Add({L"month-label"}, FormatDateText(first, L"MMMM yyyy"));
    css::Node* grid = root->Add({L"calendar-grid"});
    grid->vertical = true;
    static const wchar_t* names[] = {L"Mo", L"Tu", L"We", L"Th", L"Fr", L"Sa", L"Su"};
    css::Node* row = grid->Add({L"calendar-row"});
    for (int col = 0; col < 7; col++) row->Add({L"calendar-table", L"pe-item", L"weekday"}, names[col]);
    int cell = 0;
    for (int r = 0; r < 6; r++) {
        row = grid->Add({L"calendar-row"});
        for (int col = 0; col < 7; col++, cell++) {
            int day = cell - startCol + 1;
            bool inMonth = day >= 1 && day <= count;
            int shown = inMonth ? day : day < 1 ? days[(first.wMonth + 10) % 12] + day : day - count;
            css::Node* n = row->Add({L"calendar-table", L"pe-item"}, std::to_wstring(shown));
            if (!inMonth) n->classes.push_back(L"other-month");
            else if (monthOffset == 0 && day == now.wDay) n->states.insert(L"selected");
        }
    }
    (void)hot;
}

// The output devices (their names), and which one is the default.
struct CssDevice {
    std::wstring name;
    bool selected = false;
};

static std::vector<CssDevice> CssOutputDevices() {
    std::vector<CssDevice> out;
    static const PROPERTYKEY friendlyName = {{0xa45c254e, 0xdf1c, 0x4efd, {0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0}}, 14};
    IMMDeviceEnumerator* enumerator = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator))) return out;
    std::wstring defaultId;
    IMMDevice* def = nullptr;
    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &def))) {
        LPWSTR id = nullptr;
        if (SUCCEEDED(def->GetId(&id))) {
            defaultId = id;
            CoTaskMemFree(id);
        }
        def->Release();
    }
    IMMDeviceCollection* devices = nullptr;
    if (SUCCEEDED(enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices))) {
        UINT count = 0;
        devices->GetCount(&count);
        for (UINT i = 0; i < count && i < 12; i++) {
            IMMDevice* d = nullptr;
            if (FAILED(devices->Item(i, &d))) continue;
            CssDevice dev;
            LPWSTR id = nullptr;
            if (SUCCEEDED(d->GetId(&id))) {
                dev.selected = defaultId == id;
                CoTaskMemFree(id);
            }
            IPropertyStore* store = nullptr;
            if (SUCCEEDED(d->OpenPropertyStore(STGM_READ, &store))) {
                PROPVARIANT v;
                PropVariantInit(&v);
                if (SUCCEEDED(store->GetValue(friendlyName, &v)) && v.vt == VT_LPWSTR && v.pwszVal) dev.name = v.pwszVal;
                PropVariantClear(&v);
                store->Release();
            }
            d->Release();
            if (!dev.name.empty()) out.push_back(dev);
        }
        devices->Release();
    }
    enumerator->Release();
    return out;
}

static void CssBuildVolume(CssPopupBuild& b) {
    css::Node* root = CssPopupRoot(b, {L"audio-menu"});
    root->vertical = true;
    int level = 0;
    bool muted = false;
    CssVolume(&level, &muted);
    b.volume = level;
    css::Node* row = root->Add({L"audio-slider-row"});
    b.track = row->Add({L"volume-slider"});
    b.handle = b.track->Add({L"volume-slider", L"pe-handle"});
    css::Node* list = root->Add({L"audio-container"});
    list->vertical = true;
    for (auto const& d : CssOutputDevices()) {
        css::Node* n = list->Add({L"device"}, d.name);
        if (d.selected) n->classes.push_back(L"selected");
        b.actions.push_back({n, L"sound"});
    }
}

static void CssBuildPower(CssPopupBuild& b, yaml::Node const* opts, int hot) {
    css::Node* root = CssPopupRoot(b, {L"power-menu-popup"});
    root->vertical = true;
    root->centerKids = true;
    if (!opts || opts->Bool(L"uptime", true)) {
        ULONGLONG s = GetTickCount64() / 1000;
        std::wstring text = L"Uptime: ";
        if (s >= 86400) text += std::to_wstring(s / 86400) + L"d ";
        text += std::to_wstring(s / 3600 % 24) + L"h " + std::to_wstring(s / 60 % 60) + L"m";
        root->Add({L"uptime"}, text);
    }
    struct Btn {
        std::wstring key, icon, label;
    };
    std::vector<Btn> buttons;
    if (yaml::Node const* list = opts ? opts->Get(L"buttons") : nullptr) {
        for (auto const& [key, val] : list->map)
            if (val.seq.size() >= 2) buttons.push_back({key, val.seq[0].s, val.seq[1].s});
    }
    if (buttons.empty())
        buttons = {{L"lock", L"\uEA75", L"Lock"},         {L"signout", L"\xDB80\xDF43", L"Sign out"}, {L"shutdown", L"\uF011", L"Shut Down"},
                   {L"restart", L"\uEAD2", L"Restart"},   {L"hibernate", L"\uF28E", L"Hibernate"},    {L"cancel", L"\xDB81\xDF3A", L"Cancel"}};
    int perRow = opts ? std::clamp((int)opts->Num(L"button_row", 3), 1, 5) : 3;
    css::Node* row = nullptr;
    for (size_t i = 0; i < buttons.size(); i++) {
        if (i % perRow == 0) row = root->Add({L"power-row"});
        css::Node* btn = row->Add({L"button", buttons[i].key});
        btn->vertical = true;
        btn->centerKids = true;
        btn->Add({L"icon"}, buttons[i].icon);
        btn->Add({L"label"}, buttons[i].label);
        if ((int)i == hot) {
            btn->classes.push_back(L"hover");
            btn->states.insert(L"hover");
        }
        b.actions.push_back({btn, L"power:" + buttons[i].key});
    }
}

// ---------------------------------------------------------------------------------------------------- showing them

static void CssPopupRender(bool reposition) {
    if (!g_popup.hwnd) return;
    yaml::Node const* opts = nullptr;
    yaml::Node const* def = nullptr;
    CssPopupBuild b;
    css::LengthScale() = g_popup.scale;
    double S = g_popup.scale;
    switch (g_popup.kind) {
        case CssPopupKind::Calendar:
            def = CssFindWidget(L"ClockWidget");
            opts = def && def->Get(L"options") ? def->Get(L"options")->Get(L"calendar") : nullptr;
            CssBuildCalendar(b, g_popup.monthOffset, g_popup.hot);
            break;
        case CssPopupKind::Volume:
            def = CssFindWidget(L"VolumeWidget");
            opts = def && def->Get(L"options") ? def->Get(L"options")->Get(L"audio_menu") : nullptr;
            CssBuildVolume(b);
            break;
        case CssPopupKind::Power:
            def = CssFindWidget(L"PowerMenuWidget");
            opts = def ? def->Get(L"options") : nullptr;
            CssBuildPower(b, opts, g_popup.hot >= 0 && g_popup.hot < (int)g_popup.items.size() ? g_popup.hot : -1);
            break;
        default: return;
    }
    css::Node& root = *b.root;
    css::ComputeStyle(root, g_css.sheet);
    // The theme's own rules decide the look; the sizes Qt gives its widgets by default are ours where the theme says nothing.
    auto fixSize = [&](css::Node& n, double w, double h) {
        if (n.style.width < 0) n.style.width = w * S;
        if (n.style.height < 0) n.style.height = h * S;
    };
    if (g_popup.kind == CssPopupKind::Calendar) {
        for (auto& col : root.kids)
            for (auto& r : col->kids)
                if (col->classes[0] == L"calendar-grid")
                    for (auto& cell : r->kids) {
                        fixSize(*cell, 34, 30);
                        if (cell->Has(L"other-month")) cell->style.opacity = 0.35;
                        if (cell->Has(L"weekday")) cell->style.opacity = 0.7;
                    }
        css::Node* left = root.kids[0].get();
        left->style.margin = {0, 12 * S, 0, 0};
        if (root.style.padding.top + root.style.padding.left == 0) root.style.padding = {14 * S, 14 * S, 14 * S, 14 * S};
    } else if (g_popup.kind == CssPopupKind::Volume) {
        if (root.style.padding.top + root.style.padding.left == 0) root.style.padding = {12 * S, 12 * S, 12 * S, 12 * S};
        fixSize(*b.track, 260, 22);
        fixSize(*b.handle, 14, 14);
        for (auto& row : root.kids)
            if (row->Has(L"audio-container"))
                for (auto& dev : row->kids) {
                    dev->style.minWidth = std::max(dev->style.minWidth, 260 * S);
                    if (dev->style.padding.top + dev->style.padding.left == 0) dev->style.padding = {6 * S, 8 * S, 6 * S, 4 * S};
                }
    }
    cssdraw::PopupSize total = cssdraw::PopupMeasure(root);
    // Where it opens: under (or over) the widget, aligned as its options say, kept on the screen.
    int width = (int)std::ceil(total.w), height = (int)std::ceil(total.h);
    if (reposition || width != g_popup.width || height != g_popup.height) {
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(MonitorFromRect(&g_popup.anchor, MONITOR_DEFAULTTONEAREST), &mi);
        RECT work = mi.rcWork;
        int gap = (int)std::lround(6 * S), x, y;
        std::wstring align = opts ? css::Lower(opts->Str(L"alignment", L"center")) : L"center";
        bool up = (opts ? css::Lower(opts->Str(L"direction", L"down")) : L"down") == L"up" || g_settings.edge == Edge::Bottom;
        if (g_popup.kind == CssPopupKind::Power) {
            x = (work.left + work.right - width) / 2;
            y = (work.top + work.bottom - height) / 2;
        } else {
            x = align == L"left" ? g_popup.anchor.left : align == L"right" ? g_popup.anchor.right - width : (g_popup.anchor.left + g_popup.anchor.right - width) / 2;
            y = up ? g_popup.anchor.top - gap - height : g_popup.anchor.bottom + gap;
        }
        x = std::clamp<int>(x, work.left + gap, std::max<int>(work.left + gap, work.right - width - gap));
        y = std::clamp<int>(y, work.top + gap, std::max<int>(work.top + gap, work.bottom - height - gap));
        SetWindowPos(g_popup.hwnd, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE);
        g_popup.width = width;
        g_popup.height = height;
    }
    cssdraw::PopupPlace(root, 0, 0);
    RECT wr;
    GetWindowRect(g_popup.hwnd, &wr);
    // Rects for the pointer, in the popup's own pixels.
    g_popup.items.clear();
    for (auto const& [node, action] : b.actions) g_popup.items.push_back({{(LONG)node->x, (LONG)node->y, (LONG)std::ceil(node->x + node->w), (LONG)std::ceil(node->y + node->h)}, action});
    if (b.track) {
        g_popup.slider = {(LONG)b.track->x, (LONG)b.track->y, (LONG)std::ceil(b.track->x + b.track->w), (LONG)std::ceil(b.track->y + b.track->h)};
        // The handle sits at the volume along the track.
        double hw = b.handle->w, hh = b.handle->h, span = std::max(1.0, b.track->w - hw);
        b.handle->x = b.track->x + span * b.volume / 100.0;
        b.handle->y = b.track->y + (b.track->h - hh) / 2;
    }
    cssdraw::Surface sf(width, height);
    // The groove under the handle: the handle's colour, dim where it hasn't reached and full where it has.
    if (b.track) {
        css::Rgba hc = b.handle->style.background.a > 0 ? b.handle->style.background : b.handle->style.color;
        double gy = b.track->y + b.track->h / 2, gh = std::max(2.0, 4 * S);
        css::Node groove;
        groove.style.radius[0] = groove.style.radius[1] = groove.style.radius[2] = groove.style.radius[3] = gh / 2;
        groove.style.background = {hc.r, hc.g, hc.b, 0.3};
        groove.x = b.track->x;
        groove.y = gy - gh / 2;
        groove.w = b.track->w;
        groove.h = gh;
        cssdraw::PaintBox(sf, groove);
    }
    cssdraw::PaintTree(sf, root, [](css::Node&, cssdraw::RectD) {});
    if (b.track) {
        css::Rgba hc = b.handle->style.background.a > 0 ? b.handle->style.background : b.handle->style.color;
        double gy = b.track->y + b.track->h / 2, gh = std::max(2.0, 4 * S);
        css::Node filled;
        filled.style.radius[0] = filled.style.radius[1] = filled.style.radius[2] = filled.style.radius[3] = gh / 2;
        filled.style.background = hc;
        filled.x = b.track->x;
        filled.y = gy - gh / 2;
        filled.w = b.handle->x - b.track->x + b.handle->w / 2;
        filled.h = gh;
        cssdraw::PaintBox(sf, filled);
        cssdraw::PaintBox(sf, *b.handle);  // over the fill
    }
    HDC screen = GetDC(nullptr);
    Surface out(screen, width, height);
    memcpy(out.bits, sf.px.data(), sf.px.size() * sizeof(uint32_t));
    GdiFlush();
    POINT dst{wr.left, wr.top}, src{0, 0};
    SIZE size{width, height};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(g_popup.hwnd, screen, &dst, &size, out.dc, &src, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
}

void CssPopupClose() {
    if (g_popup.hwnd) {
        HWND h = g_popup.hwnd;
        g_popup.hwnd = nullptr;
        g_popup.closedAt = GetTickCount64();
        g_popup.closedKind = g_popup.kind;
        g_popup.kind = CssPopupKind::None;
        DestroyWindow(h);
    }
}

static void CssPower(std::wstring const& key) {
    auto run = [](wchar_t const* args) { ShellExecuteW(nullptr, L"open", L"shutdown.exe", args, nullptr, SW_HIDE); };
    if (key == L"cancel") return;
    if (key == L"lock") LockWorkStation();
    else if (key == L"signout") ExitWindowsEx(EWX_LOGOFF, 0);
    else if (key == L"shutdown") run(L"/s /t 0");
    else if (key == L"restart") run(L"/r /t 0");
    else if (key == L"force_shutdown") run(L"/s /f /t 0");
    else if (key == L"force_restart") run(L"/r /f /t 0");
    else if (key == L"hibernate") run(L"/h");
    else if (key == L"sleep") ShellExecuteW(nullptr, L"open", L"rundll32.exe", L"powrprof.dll,SetSuspendState 0,1,0", nullptr, SW_HIDE);
}

static void CssSetVolume(int x) {
    // x in the popup's pixels along the slider's track.
    double hw = 14 * g_popup.scale, span = std::max(1.0, (double)(g_popup.slider.right - g_popup.slider.left) - hw);
    double f = std::clamp((x - g_popup.slider.left - hw / 2) / span, 0.0, 1.0);
    IMMDeviceEnumerator* enumerator = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator))) return;
    IMMDevice* device = nullptr;
    if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device))) {
        IAudioEndpointVolume* volume = nullptr;
        if (SUCCEEDED(device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&volume))) {
            volume->SetMasterVolumeLevelScalar((float)f, nullptr);
            volume->SetMute(FALSE, nullptr);
            volume->Release();
        }
        device->Release();
    }
    enumerator->Release();
}

static LRESULT CALLBACK CssPopupProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEMOVE: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, wnd, 0};
            TrackMouseEvent(&tme);
            if (g_popup.dragging) {
                CssSetVolume(pt.x);
                CssPopupRender(false);
                return 0;
            }
            int hot = -1;
            for (size_t i = 0; i < g_popup.items.size(); i++)
                if (PtInRect(&g_popup.items[i].rc, pt)) hot = (int)i;
            if (hot != g_popup.hot) {
                g_popup.hot = hot;
                CssPopupRender(false);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            if (g_popup.hot != -1 && !g_popup.dragging) {
                g_popup.hot = -1;
                CssPopupRender(false);
            }
            return 0;
        case WM_LBUTTONDOWN: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_popup.kind == CssPopupKind::Volume && PtInRect(&g_popup.slider, pt)) {
                g_popup.dragging = true;
                SetCapture(wnd);
                CssSetVolume(pt.x);
                CssPopupRender(false);
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_popup.dragging) {
                g_popup.dragging = false;
                ReleaseCapture();
                return 0;
            }
            for (auto const& item : g_popup.items) {
                if (!PtInRect(&item.rc, pt)) continue;
                std::wstring action = item.action;
                if (action == L"sound") {
                    CssPopupClose();
                    ShellExecuteW(nullptr, L"open", L"ms-settings:sound", nullptr, nullptr, SW_SHOWNORMAL);
                } else if (action.rfind(L"power:", 0) == 0) {
                    CssPopupClose();
                    CssPower(action.substr(6));
                }
                break;
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? -1 : 1;
            if (g_popup.kind == CssPopupKind::Calendar) {
                g_popup.monthOffset += delta;
                CssPopupRender(false);
            } else if (g_popup.kind == CssPopupKind::Volume) {
                int level = 0;
                bool muted = false;
                CssVolume(&level, &muted);
                double hw = 14 * g_popup.scale, span = std::max(1.0, (double)(g_popup.slider.right - g_popup.slider.left) - hw);
                CssSetVolume((int)(g_popup.slider.left + hw / 2 + span * std::clamp(level - delta * 4, 0, 100) / 100.0));
                CssPopupRender(false);
            }
            return 0;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) CssPopupClose();
            return 0;
        case WM_TIMER:
            // It goes away when something else has the focus (a click anywhere else).
            if (wParam == kTimerPopupWatch && GetTickCount64() - g_popup.shownAt > 400 && GetForegroundWindow() != wnd && !g_popup.dragging) CssPopupClose();
            return 0;
        case WM_DESTROY:
            KillTimer(wnd, kTimerPopupWatch);
            return 0;
        case WM_MOUSEACTIVATE:
            return MA_ACTIVATE;
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// The popup for a widget opened, or closed when it is that one's already. The anchor is the widget's rectangle on the screen.
void CssPopupToggle(CssPopupKind kind, RECT const& anchor, double scale) {
    ULONGLONG now = GetTickCount64();
    if (g_popup.hwnd && g_popup.kind == kind) {
        CssPopupClose();
        return;
    }
    // Clicking the widget again is what took the focus and closed it: not to open it again.
    if (g_popup.closedKind == kind && now - g_popup.closedAt < 300) {
        g_popup.closedKind = CssPopupKind::None;
        return;
    }
    CssPopupClose();
    static bool registered = false;
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&CssPopupProc, &instance);
    if (!registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = CssPopupProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kPopupClass;
        RegisterClassW(&wc);
        g_popupInstance = instance;
        registered = true;
    }
    g_popup.kind = kind;
    g_popup.anchor = anchor;
    g_popup.scale = scale;
    g_popup.hot = -1;
    g_popup.monthOffset = 0;
    g_popup.dragging = false;
    g_popup.width = g_popup.height = 0;
    g_popup.closedKind = CssPopupKind::None;
    g_popup.hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED, kPopupClass, L"", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
    if (!g_popup.hwnd) {
        g_popup.kind = CssPopupKind::None;
        return;
    }
    yaml::Node const* def = CssFindWidget(kind == CssPopupKind::Calendar ? L"ClockWidget" : kind == CssPopupKind::Volume ? L"VolumeWidget" : L"PowerMenuWidget");
    yaml::Node const* opt = def ? def->Get(L"options") : nullptr;
    yaml::Node const* look = !opt ? nullptr : kind == CssPopupKind::Calendar ? opt->Get(L"calendar") : kind == CssPopupKind::Volume ? opt->Get(L"audio_menu") : opt;
    CssPopupBackdrop(g_popup.hwnd, look);
    CssPopupRender(true);
    g_popup.shownAt = GetTickCount64();
    ShowWindow(g_popup.hwnd, SW_SHOWNA);
    SetForegroundWindow(g_popup.hwnd);
    SetTimer(g_popup.hwnd, kTimerPopupWatch, 150, nullptr);
}

void CssPopupShutdown() {
    CssPopupClose();
    if (g_popupInstance) UnregisterClassW(kPopupClass, g_popupInstance);
    g_popupInstance = nullptr;
}

// What a click on the clock does in a theme's bar: its callbacks, else YASB's own (the calendar on a left click, the
// alternative label on a right one). button: 0 left, 1 right, 2 middle.
std::wstring CssClockAction(int button) {
    std::wstring name;
    yaml::Node const* def = CssFindWidget(L"ClockWidget", &name);
    yaml::Node const* opt = def ? def->Get(L"options") : nullptr;
    yaml::Node const* cb = opt ? opt->Get(L"callbacks") : nullptr;
    wchar_t const* key = button == 0 ? L"on_left" : button == 1 ? L"on_right" : L"on_middle";
    std::wstring action = cb ? cb->Str(key) : L"";
    if (action.empty() && !(cb && cb->Get(key))) action = button == 0 ? L"toggle_calendar" : button == 1 ? L"toggle_label" : L"";
    return CssAction(action, name, nullptr);
}