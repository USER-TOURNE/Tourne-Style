// The bar drawn from a YASB theme (styles.css + config.yaml in a folder under %LOCALAPPDATA%\Tourne\Yasb). The theme's
// layout lists say what goes where; each widget it names becomes the element tree YASB gives it (.widget,
// .clock-widget, .widget-container, .icon, .label...), which css-engine.cpp styles and css-draw.cpp draws. The buttons,
// tray, clock and Start button stay the suite's own (clicks, jump lists, previews, the relay): their boxes are the theme's.
//
// Inlined into taskbar-host.cpp before Redraw.

struct CssTheme {
    bool loaded = false;
    std::wstring name, dir;
    css::Sheet sheet;
    yaml::Node cfg;
    std::wstring position = L"top";
    int height = 36, padTop = 0, padLeft = 0, padBottom = 0, padRight = 0;  // css px; the bar floats off the screen by its padding
    bool blur = false, blurDark = true, roundCorners = false;
    std::wstring borderColor = L"None";
    bool hasTaskbar = false, hasTray = false, hasStart = false;
    std::set<std::wstring> alt;  // widgets showing their label_alt (a click with "toggle_label" swaps)
};

CssTheme g_css;
constexpr UINT_PTR kTimerCssFade = 13;  // a hover fading in or out

bool CssActive() { return g_css.loaded; }

static std::wstring CssReadFile(std::wstring const& path) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return L"";
    DWORD size = GetFileSize(h, nullptr), got = 0;
    if (size == INVALID_FILE_SIZE || size > 8 * 1024 * 1024) {
        CloseHandle(h);
        return L"";
    }
    std::string bytes(size, 0);
    ReadFile(h, bytes.data(), size, &got, nullptr);
    CloseHandle(h);
    size_t skip = bytes.size() >= 3 && (uint8_t)bytes[0] == 0xEF && (uint8_t)bytes[1] == 0xBB && (uint8_t)bytes[2] == 0xBF ? 3 : 0;
    int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), nullptr, 0);
    std::wstring out(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), out.data(), n);
    return out;
}

static bool CssDarkMode() {
    DWORD light = 1, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme",
                 RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light == 0;
}

static yaml::Node const* CssBarConfig() {
    yaml::Node const* bars = g_css.cfg.Get(L"bars");
    return bars && !bars->map.empty() ? &bars->map[0].second : nullptr;
}

static yaml::Node const* CssWidgetDef(std::wstring const& name) {
    yaml::Node const* widgets = g_css.cfg.Get(L"widgets");
    return widgets ? widgets->Get(name) : nullptr;
}

static bool CssIsType(yaml::Node const& def, wchar_t const* type) { return def.Str(L"type").find(type) != std::wstring::npos; }

// The system accent as YASB's own variables have it, for the themes that use them without saying what they are. A theme's
// own definition of the same name wins. The lighter and darker shades are the accent moved toward white and black.
static void CssAccentVars(std::map<std::wstring, std::wstring>& vars) {
    DWORD abgr = 0, size = sizeof(abgr);
    if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM", L"AccentColor", RRF_RT_REG_DWORD, nullptr, &abgr, &size) != ERROR_SUCCESS) {
        BOOL opaque = FALSE;
        DWORD argb = 0;
        if (SUCCEEDED(DwmGetColorizationColor(&argb, &opaque))) abgr = ((argb & 0xFF) << 16) | (argb & 0xFF00) | ((argb >> 16) & 0xFF);
        else abgr = 0x00D47800;  // Windows blue
    }
    int r = abgr & 0xFF, g = (abgr >> 8) & 0xFF, b = (abgr >> 16) & 0xFF;
    auto hex = [](int rr, int gg, int bb) {
        wchar_t text[16];
        swprintf(text, 16, L"#%02X%02X%02X", std::clamp(rr, 0, 255), std::clamp(gg, 0, 255), std::clamp(bb, 0, 255));
        return std::wstring(text);
    };
    auto mix = [&](double toward, double amount) {
        return hex((int)std::lround(r + (toward - r) * amount), (int)std::lround(g + (toward - g) * amount), (int)std::lround(b + (toward - b) * amount));
    };
    vars.emplace(L"--yasb-accent", hex(r, g, b));
    for (int i = 1; i <= 3; i++) {
        vars.emplace(L"--yasb-accent-light" + std::to_wstring(i), mix(255, 0.2 * i));
        vars.emplace(L"--yasb-accent-dark" + std::to_wstring(i), mix(0, 0.2 * i));
    }
    vars.emplace(L"--yasb-icon-fg", CssDarkMode() ? L"#FFFFFF" : L"#1A1A1A");
}

// Fonts dropped in %LOCALAPPDATA%\Tourne\Fonts, for this process only: the Nerd Font a theme names, without installing it.
static std::vector<std::wstring> g_cssFontFiles;

static void CssLoadFonts() {
    for (auto const& f : g_cssFontFiles) RemoveFontResourceExW(f.c_str(), FR_PRIVATE, nullptr);
    g_cssFontFiles.clear();
    std::wstring folder = TourneFonts();
    WIN32_FIND_DATAW fd;
    HANDLE find = FindFirstFileW((folder + L"*").c_str(), &fd);
    if (find == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        std::wstring name = fd.cFileName, lower = name;
        for (auto& c : lower) c = (wchar_t)towlower(c);
        if (lower.size() > 4 && (lower.ends_with(L".ttf") || lower.ends_with(L".otf") || lower.ends_with(L".ttc")))
            if (AddFontResourceExW((folder + name).c_str(), FR_PRIVATE, nullptr) > 0) g_cssFontFiles.push_back(folder + name);
    } while (FindNextFileW(find, &fd));
    FindClose(find);
}

void CssUnloadFonts() {
    for (auto const& f : g_cssFontFiles) RemoveFontResourceExW(f.c_str(), FR_PRIVATE, nullptr);
    g_cssFontFiles.clear();
}

// The theme's folder read: its styles, its layout, and what the bar itself asks of the window. Set from ApplySettings.
void CssApply() {
    g_css = CssTheme();
    if (g_settings.yasbTheme.empty()) {
        cssdraw::FontCache().Clear();
        return;
    }
    CssLoadFonts();
    cssdraw::FontCache().Clear();
    std::wstring dir = TourneYasb() + g_settings.yasbTheme + L"\\";
    std::wstring styles = CssReadFile(dir + L"styles.css");
    if (styles.empty()) return;
    CssTheme t;
    t.name = g_settings.yasbTheme;
    t.dir = dir;
    // A stylesheet imported by @import url("...") next to it: read first, so the theme's own rules win.
    std::wstring imports;
    for (size_t at = styles.find(L"@import"); at != std::wstring::npos; at = styles.find(L"@import", at + 1)) {
        size_t end = styles.find(L';', at);
        if (end == std::wstring::npos) break;
        std::wstring spec = styles.substr(at + 7, end - at - 7);
        size_t a = spec.find_first_of(L"\"'"), b = spec.find_last_of(L"\"'");
        if (a == std::wstring::npos || b <= a) continue;
        std::wstring file = spec.substr(a + 1, b - a - 1);
        if (file.find(L"..") == std::wstring::npos && file.find(L':') == std::wstring::npos) imports += CssReadFile(dir + file) + L"\n";
    }
    css::ParseSheetInto(t.sheet, imports + styles);
    CssAccentVars(t.sheet.vars);
    t.cfg = yaml::Parse(CssReadFile(dir + L"config.yaml"));
    css::IndexSheet(t.sheet);
    g_css = std::move(t);
    yaml::Node const* bar = CssBarConfig();
    if (bar) {
        if (auto const* al = bar->Get(L"alignment")) g_css.position = al->Str(L"position", L"top");
        if (auto const* d = bar->Get(L"dimensions")) g_css.height = std::clamp((int)d->Num(L"height", 36), 16, 200);
        if (auto const* p = bar->Get(L"padding")) {
            g_css.padTop = (int)p->Num(L"top", 0);
            g_css.padLeft = (int)p->Num(L"left", 0);
            g_css.padBottom = (int)p->Num(L"bottom", 0);
            g_css.padRight = (int)p->Num(L"right", 0);
        }
        if (auto const* b = bar->Get(L"blur_effect")) {
            g_css.blur = b->Bool(L"enabled", false);
            g_css.blurDark = b->Bool(L"dark_mode", true);
            g_css.roundCorners = b->Bool(L"round_corners", false);
            g_css.borderColor = b->Str(L"border_color", L"None");
        }
        if (auto const* w = bar->Get(L"widgets"))
            for (wchar_t const* side : {L"left", L"center", L"right"})
                if (auto const* list = w->Get(side))
                    for (auto const& item : list->seq)
                        if (auto const* def = CssWidgetDef(item.s)) {
                            if (CssIsType(*def, L"TaskbarWidget")) g_css.hasTaskbar = true;
                            if (CssIsType(*def, L"SystrayWidget")) g_css.hasTray = true;
                            if (CssIsType(*def, L"HomeWidget")) g_css.hasStart = true;
                            if (auto const* opt = def->Get(L"options"))
                                if (auto const* cb = opt->Get(L"callbacks"))
                                    if (cb->Str(L"on_left").find(L"start_menu") != std::wstring::npos) g_css.hasStart = true;
                        }
    }
    g_css.loaded = true;
    // The window is the bar's body; the bar's padding is its distance from the screen's edge.
    g_settings.edge = g_css.position == L"bottom" ? Edge::Bottom : Edge::Top;
    g_settings.thickness = g_css.height + g_css.padTop + g_css.padBottom;
    g_settings.segments = Segments::Off;
}

// The bar's window inside the space it reserves: inset by the theme's padding.
RECT CssBody(RECT const& r) {
    int l = Px(g_css.padLeft), t = Px(g_css.padTop), rr = Px(g_css.padRight), b = Px(g_css.padBottom), h = Px(g_css.height);
    if (g_settings.edge == Edge::Bottom) return {r.left + l, r.bottom - b - h, r.right - rr, r.bottom - b};
    return {r.left + l, r.top + t, r.right - rr, r.top + t + h};
}

// ---------------------------------------------------------------------------------------------------- data for labels

// strftime's letters as Windows formats them (so month and day names follow the user's language).
static std::wstring CssStrftime(std::wstring const& fmt) {
    SYSTEMTIME t;
    GetLocalTime(&t);
    std::wstring out;
    for (size_t i = 0; i < fmt.size(); i++) {
        if (fmt[i] != L'%' || i + 1 >= fmt.size()) {
            out += fmt[i];
            continue;
        }
        switch (fmt[++i]) {
            case L'a': out += FormatDateText(t, L"ddd"); break;
            case L'A': out += FormatDateText(t, L"dddd"); break;
            case L'b': out += FormatDateText(t, L"MMM"); break;
            case L'B': out += FormatDateText(t, L"MMMM"); break;
            case L'd': out += FormatDateText(t, L"dd"); break;
            case L'e': out += FormatDateText(t, L"d"); break;
            case L'm': out += FormatDateText(t, L"MM"); break;
            case L'y': out += FormatDateText(t, L"yy"); break;
            case L'Y': out += FormatDateText(t, L"yyyy"); break;
            case L'H': out += FormatTimeText(t, L"HH"); break;
            case L'I': out += FormatTimeText(t, L"hh"); break;
            case L'M': out += FormatTimeText(t, L"mm"); break;
            case L'S': out += FormatTimeText(t, L"ss"); break;
            case L'p': out += FormatTimeText(t, L"tt"); break;
            case L'%': out += L'%'; break;
            default: out += fmt[i];
        }
    }
    return out;
}

// The master volume, read once a second (an endpoint kept, asked again when it stops answering).
static bool CssVolume(int* percent, bool* muted) {
    static IAudioEndpointVolume* volume = nullptr;
    static ULONGLONG lastTry = 0;
    if (!volume && GetTickCount64() - lastTry > 5000) {
        lastTry = GetTickCount64();
        IMMDeviceEnumerator* enumerator = nullptr;
        if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator))) {
            IMMDevice* device = nullptr;
            if (SUCCEEDED(enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device))) {
                device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&volume);
                device->Release();
            }
            enumerator->Release();
        }
    }
    if (!volume) return false;
    float level = 0;
    BOOL mute = FALSE;
    if (FAILED(volume->GetMasterVolumeLevelScalar(&level)) || FAILED(volume->GetMute(&mute))) {
        volume->Release();
        volume = nullptr;
        return false;
    }
    *percent = (int)std::lround(level * 100);
    *muted = mute != FALSE;
    return true;
}

// ---------------------------------------------------------------------------------------------------- the tree

struct CssPrepared {
    std::unique_ptr<css::Node> root;
    css::Node* start = nullptr;
    css::Node* clock = nullptr;
    std::vector<css::Node*> apps;   // a node per task button
    std::vector<css::Node*> trays;  // and per tray icon
    struct Act {
        css::Node* node;
        CssTarget target;
    };
    std::vector<Act> acts;
    std::map<css::Node const*, HICON> icons;   // the icon a node shows
    std::map<css::Node const*, int> trayOf;    // or the tray icon
};

// "<span>{icon}</span> {temp}": the spans become .icon labels and the rest .label ones.
static void CssMarkup(css::Node* parent, std::wstring markup, std::function<std::wstring(std::wstring const&)> fill,
                      std::wstring const& iconClass = L"", std::wstring const& labelClass = L"") {
    std::wstring out;
    for (size_t i = 0; i < markup.size(); i++) {
        if (markup[i] == L'{') {
            size_t close = markup.find(L'}', i);
            if (close != std::wstring::npos) {
                out += fill(markup.substr(i + 1, close - i - 1));
                i = close;
                continue;
            }
        }
        out += markup[i];
    }
    auto label = [&](std::wstring text, bool icon) {
        text = css::Trim(text);
        if (text.empty()) return;
        css::Node* n = parent->Add({icon ? L"icon" : L"label"}, text);
        if (icon && !iconClass.empty()) n->classes.push_back(iconClass);
        if (!icon && !labelClass.empty()) n->classes.push_back(labelClass);
    };
    size_t at = 0;
    while (at < out.size()) {
        size_t open = out.find(L"<span", at);
        if (open == std::wstring::npos) {
            label(out.substr(at), false);
            break;
        }
        label(out.substr(at, open - at), false);
        size_t gt = out.find(L'>', open), end = out.find(L"</span>", gt);
        if (gt == std::wstring::npos || end == std::wstring::npos) break;
        label(out.substr(gt + 1, end - gt - 1), true);
        at = end + 7;
    }
}

static std::wstring CssWidgetClass(std::wstring const& type, yaml::Node const& opt) {
    if (type.find(L"CustomWidget") != std::wstring::npos) {
        std::wstring cls = opt.Str(L"class_name");
        return cls.empty() ? L"custom-widget" : cls;
    }
    static const std::pair<wchar_t const*, wchar_t const*> known[] = {
        {L"ClockWidget", L"clock-widget"},           {L"VolumeWidget", L"volume-widget"},
        {L"PowerMenuWidget", L"power-menu-widget"},  {L"ActiveWindowWidget", L"active-window-widget"},
        {L"ApplicationsWidget", L"apps-widget"},     {L"TaskbarWidget", L"taskbar-widget"},
        {L"HomeWidget", L"home-widget"},             {L"SystrayWidget", L"systray"},
        {L"ActiveLayoutWidget", L"komorebi-active-layout"}, {L"StartWidget", L"home-widget"},
        {L"komorebi.workspaces", L"komorebi-workspaces"},   {L"GlazewmWorkspacesWidget", L"glazewm-workspaces"},
        {L"windows_desktops", L"windows-desktops"},
    };
    for (auto const& [needle, cls] : known)
        if (type.find(needle) != std::wstring::npos) return cls;
    // "yasb.github.GithubWidget" is ".github-widget", "yasb.cpu.CpuWidget" ".cpu-widget": the class name kebab-cased.
    std::wstring last = type.substr(type.find_last_of(L'.') == std::wstring::npos ? 0 : type.find_last_of(L'.') + 1);
    std::wstring out;
    for (size_t i = 0; i < last.size(); i++) {
        if (i && iswupper(last[i]) && !iswupper(last[i - 1])) out += L'-';
        out += (wchar_t)towlower(last[i]);
    }
    return out.empty() ? L"widget-x" : out;
}

// :hover reaches the pointed-at node and everything around it.
static void CssHover(css::Node* n) {
    for (; n; n = n->parent) n->states.insert(L"hover");
}

// The theme's callbacks as actions RunCssAction knows: the widget's own label swap and popups get the widget's name.
static std::wstring CssAction(std::wstring a, std::wstring const& name, wchar_t const* menu) {
    a = css::Trim(a);
    if (a == L"toggle_label") return L"toggle_label:" + name;
    if (a == L"toggle_calendar") return L"popup:calendar";
    if (a == L"toggle_menu" || a == L"toggle_volume_menu" || a == L"toggle_audio_menu") return menu ? L"popup:" + std::wstring(menu) : L"";
    return a;
}

static void CssCallbacks(CssPrepared::Act& act, yaml::Node const& opt, std::wstring const& name, wchar_t const* menu = nullptr,
                         wchar_t const* defaultLeft = nullptr) {
    if (auto const* cb = opt.Get(L"callbacks")) {
        act.target.left = CssAction(cb->Str(L"on_left"), name, menu);
        act.target.middle = CssAction(cb->Str(L"on_middle"), name, menu);
        act.target.right = CssAction(cb->Str(L"on_right"), name, menu);
    }
    if (act.target.left.empty() && defaultLeft) act.target.left = defaultLeft;
}

// The label to show: label, or label_alt after a toggle_label click.
static std::wstring CssLabelOf(yaml::Node const& opt, std::wstring const& name) {
    std::wstring label = g_css.alt.count(name) ? opt.Str(L"label_alt") : L"";
    return label.empty() ? opt.Str(L"label") : label;
}

// One widget of the theme's layout into a container; the ones the bar can't be (or hasn't yet) are left out.
static void CssAddWidget(CssPrepared& p, css::Node* container, std::wstring const& name, yaml::Node const* def, bool synthetic = false) {
    yaml::Node empty;
    yaml::Node const* opt = def ? def->Get(L"options") : nullptr;
    if (!opt) opt = &empty;
    std::wstring type = def ? def->Str(L"type") : L"";
    if (synthetic) type = name;  // "TaskbarWidget" or "SystrayWidget", added by us
    std::wstring cls = CssWidgetClass(type, *opt);
    bool mainBar = IsMainBar();
    auto has = [&](wchar_t const* t) { return type.find(t) != std::wstring::npos; };
    if (has(L"TaskbarWidget")) {
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        double size = opt->Num(L"icon_size", 16);
        bool titles = opt->Bool(L"show_title", false);
        (void)titles;
        for (int i = 0; i < (int)g_buttons.size(); i++) {
            Button const& b = g_buttons[i];
            css::Node* app = c->Add({L"app-container"});
            int state = ButtonState(i);
            if (!b.windows.empty()) app->classes.push_back(L"running");
            if (state >= kStateActive && state <= kStateActivePressed) app->classes.push_back(L"foreground");
            if (state == kStateFlashing) app->classes.push_back(L"flashing");
            if (b.windows.size() > 1) app->classes.push_back(L"grouped");
            if (g_hot == i) CssHover(app);
            if (g_hot == i && g_pressed == i) app->states.insert(L"pressed");
            css::Node* icon = app->Add({L"app-icon"});
            icon->iconSize = size * g_scale;
            HICON hicon = nullptr;
            if (!b.windows.empty()) {
                if (Win* w0 = FindWin(b.windows[0])) hicon = ButtonIcon(*w0, b.pin >= 0 || b.windows.size() > 1);
            }
            if (b.pin >= 0 && (b.windows.empty() || !hicon)) hicon = PinIcon(b.pin, (int)icon->iconSize);
            p.icons[icon] = hicon;
            p.apps.push_back(app);
        }
    } else if (has(L"SystrayWidget")) {
        if (!mainBar && !g_settings.trayEverywhere) return;
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        for (int i = 0; i < (int)g_tray.size(); i++) {
            css::Node* cell = c->Add({L"tray-icon"});
            cell->iconSize = std::max(16, g_tray[i].icon.size) ;
            if (g_hot == kHitTray + i) CssHover(cell);
            p.trayOf[cell] = i;
            p.trays.push_back(cell);
        }
    } else if (has(L"ClockWidget")) {
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        std::wstring clockLabel = CssLabelOf(*opt, name);
        CssMarkup(c, clockLabel.empty() ? L"{%H:%M}" : clockLabel, [](std::wstring const& k) { return k.size() && k[0] == L'%' ? CssStrftime(k) : L""; });
        if (g_hot == kHitClock) CssHover(w);
        p.clock = w;
    } else if (has(L"VolumeWidget")) {
        int level = 0;
        bool muted = false;
        if (!CssVolume(&level, &muted)) return;
        yaml::Node const* icons = opt->Get(L"icons");
        std::wstring glyph;
        if (icons && !icons->seq.empty()) {
            size_t idx = muted ? 0 : level <= 10 ? 1 : level <= 30 ? 2 : level <= 60 ? 3 : 4;
            glyph = icons->seq[std::min(idx, icons->seq.size() - 1)].s;
        }
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        CssMarkup(c, CssLabelOf(*opt, name).empty() ? L"{level}" : CssLabelOf(*opt, name), [&](std::wstring const& k) {
            return k == L"icon" ? glyph : k == L"level" ? (muted ? L"muted" : std::to_wstring(level) + L"%") : k == L"volume" ? std::to_wstring(level) : L"";
        });
        CssPrepared::Act act{w, {}};
        CssCallbacks(act, *opt, name, L"volume", L"popup:volume");
        if (g_hot == kHitCss + (int)p.acts.size()) CssHover(w);
        p.acts.push_back(act);
    } else if (has(L"ActiveWindowWidget")) {
        HWND fg = ForegroundRoot();
        if (!fg || !IsWindowVisible(fg)) return;
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        Win* win = FindWin(fg);
        if (win && opt->Bool(L"label_icon", true)) {
            css::Node* icon = c->Add({L"icon"});
            icon->iconSize = opt->Num(L"label_icon_size", 14) * g_scale;
            p.icons[icon] = ButtonIcon(*win, false);
        }
        std::wstring title = Title(fg);
        size_t maxLen = (size_t)opt->Num(L"max_length", 0);
        if (maxLen && title.size() > maxLen) title = title.substr(0, maxLen) + opt->Str(L"max_length_ellipsis", L"...");
        c->Add({L"label"}, title);
    } else if (has(L"StartWidget")) {
        // Ours, for a theme with no Start button: the Windows logo (a Nerd Font glyph, as the theme's own icons are).
        if (!mainBar) return;
        css::Node* w = container->Add({L"widget", cls, L"start-widget"});
        css::Node* c = w->Add({L"widget-container"});
        c->Add({L"label"}, L"\uF17A");
        p.start = w;
        if (g_hot == -1) CssHover(w);
    } else if (has(L"WorkspaceWidget") || has(L"GlazewmWorkspacesWidget")) {
        // The workspaces of komorebi, GlazeWM or YASB itself are Windows' virtual desktops here (Win+Tab): a button for
        // each, the current one .active, the ones with windows .populated, a click switching to it.
        CssDesktopInfo d = CssDesktops();
        // A desktop's name as Task view has it; unnamed, its number ({name}) or "Desktop N" (its tip).
        auto nameOf = [&d](int i) { return i < (int)d.names.size() ? d.names[i] : std::wstring(); };
        bool glaze = has(L"Glazewm"), zero = opt->Bool(L"label_zero_index", false);
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        for (int i = 0; i < d.count; i++) {
            bool active = i == d.current, populated = i < (int)d.windows.size() && d.windows[i] > 0;
            std::wstring key = active ? L"label_workspace_active_btn" : populated ? L"label_workspace_populated_btn" : L"label_workspace_btn";
            std::wstring label = opt->Str(key.c_str());
            if (label.empty()) label = opt->Str(L"label_workspace_btn");
            if (label.empty()) label = L"{index}";
            std::wstring shown;
            for (size_t k = 0; k < label.size(); k++) {
                if (label.compare(k, 7, L"{index}") == 0) { shown += std::to_wstring(i + (zero ? 0 : 1)); k += 6; }
                else if (label.compare(k, 6, L"{name}") == 0) { std::wstring n = nameOf(i); shown += n.empty() ? std::to_wstring(i + 1) : n; k += 5; }
                else shown += label[k];
            }
            css::Node* btn = c->Add({L"ws-btn"}, shown);
            if (glaze) btn->classes.push_back(active ? (populated ? L"active_populated" : L"active_empty") : populated ? L"populated" : L"empty");
            else btn->classes.push_back(active ? L"active" : populated ? L"populated" : L"empty");
            CssPrepared::Act act{btn, {}};
            act.target.left = L"desktop:" + std::to_wstring(i);
            act.target.tip = nameOf(i).empty() ? L"Desktop " + std::to_wstring(i + 1) : nameOf(i);
            if (g_hot == kHitCss + (int)p.acts.size()) CssHover(btn);
            p.acts.push_back(act);
        }
    } else if (has(L"ActiveLayoutWidget")) {
        // komorebi's tiling layouts: nothing to show on Windows' own desktops.
    } else if (has(L"CpuWidget") || has(L"MemoryWidget") || has(L"BatteryWidget") || has(L"TrafficWidget") || has(L"WifiWidget") || has(L"WeatherWidget")) {
        std::function<std::wstring(std::wstring const&)> fill;
        std::wstring iconClass, labelClass;
        if (has(L"CpuWidget")) {
            int percent = CssCpuPercent();
            yaml::Node const* icons = opt->Get(L"histogram_icons");
            int columns = std::clamp((int)opt->Num(L"histogram_num_columns", 10), 1, 32);
            fill = [=](std::wstring const& k) -> std::wstring {
                if (k == L"info[percent][total]") return std::to_wstring(percent);
                if (k == L"info[histograms][cpu_percent]") {
                    std::wstring out;
                    size_t from = g_cpuHistory.size() > (size_t)columns ? g_cpuHistory.size() - columns : 0;
                    for (size_t i = from; i < g_cpuHistory.size(); i++) {
                        if (icons && !icons->seq.empty()) out += icons->seq[std::min(icons->seq.size() - 1, (size_t)g_cpuHistory[i] * icons->seq.size() / 101)].s;
                        else out += std::to_wstring(g_cpuHistory[i]) + L" ";
                    }
                    return out;
                }
                return L"";
            };
        } else if (has(L"MemoryWidget")) {
            fill = [](std::wstring const& k) { return CssMemoryField(k); };
        } else if (has(L"BatteryWidget")) {
            CssBattery b = CssBatteryNow();
            if (!b.present) return;
            fill = [b](std::wstring const& k) -> std::wstring {
                if (k == L"percent") return std::to_wstring(b.percent);
                if (k == L"is_charging") return b.charging ? L"true" : L"false";
                if (k == L"icon" || k == L"charging_icon") {
                    if (b.charging) return L"\uf0e7";
                    return b.percent >= 88 ? L"\uf240" : b.percent >= 63 ? L"\uf241" : b.percent >= 38 ? L"\uf242" : b.percent >= 13 ? L"\uf243" : L"\uf244";
                }
                if (k == L"time_remaining") {
                    if (b.secondsLeft < 0) return b.charging ? L"charging" : L"";
                    return std::to_wstring(b.secondsLeft / 3600) + L"h " + std::to_wstring(b.secondsLeft / 60 % 60) + L"m";
                }
                return L"";
            };
        } else if (has(L"TrafficWidget")) {
            double down = 0, up = 0;
            CssTraffic(&down, &up);
            fill = [=](std::wstring const& k) -> std::wstring { return k == L"download_speed" ? CssSpeed(down) : k == L"upload_speed" ? CssSpeed(up) : L""; };
        } else if (has(L"WifiWidget")) {
            CssWifi wf = CssWifiNow();
            fill = [wf](std::wstring const& k) -> std::wstring {
                if (k == L"wifi_icon") {
                    if (!wf.connected) return L"\xdb82\xdd2f";
                    return wf.strength >= 75 ? L"\xdb82\xdd28" : wf.strength >= 50 ? L"\xdb82\xdd25" : wf.strength >= 25 ? L"\xdb82\xdd22" : L"\xdb82\xdd1f";
                }
                if (k == L"wifi_name") return wf.name;
                if (k == L"wifi_strength") return std::to_wstring(wf.strength);
                return L"";
            };
        } else {
            // The weather: what was last read (nothing until the setting lets it ask). Its icon and label wear the condition
            // as a class (.sunnyDay, .clearNight...), as the theme's own rules expect.
            CssWeatherData wd = CssWeatherNow(*opt);
            std::wstring key = wd.ok ? CssWeatherIconKey(wd) : L"default";
            iconClass = labelClass = key;
            yaml::Node const* icons = opt->Get(L"icons");
            bool imperial = opt->Str(L"units", L"metric") == L"imperial", decimal = !opt->Bool(L"hide_decimal", false);
            auto temp = [&](double v) {
                wchar_t text[32];
                if (decimal) swprintf(text, 32, L"%.1f\u00B0%lc", v, imperial ? L'F' : L'C');
                else swprintf(text, 32, L"%.0f\u00B0%lc", v, imperial ? L'F' : L'C');
                return std::wstring(text);
            };
            std::wstring glyph = icons ? icons->Str(key.c_str(), icons->Str(L"default")) : L"";
            fill = [=](std::wstring const& k) -> std::wstring {
                if (k == L"icon") return glyph;
                if (!wd.ok) return k == L"temp" || k == L"min_temp" || k == L"max_temp" ? L"--" : L"";
                if (k == L"temp") return temp(imperial ? wd.tempF : wd.tempC);
                if (k == L"min_temp") return temp(imperial ? wd.minF : wd.minC);
                if (k == L"max_temp") return temp(imperial ? wd.maxF : wd.maxC);
                if (k == L"temp_f") return temp(wd.tempF);
                if (k == L"location") return wd.location;
                if (k == L"humidity") return std::to_wstring(wd.humidity) + L"%";
                if (k == L"conditions") return wd.conditions;
                return L"";
            };
        }
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        CssMarkup(c, CssLabelOf(*opt, name), fill, iconClass, labelClass);
        CssPrepared::Act act{w, {}};
        CssCallbacks(act, *opt, name);
        if (!act.target.left.empty() || !act.target.right.empty() || !act.target.middle.empty()) {
            if (g_hot == kHitCss + (int)p.acts.size()) CssHover(w);
            p.acts.push_back(act);
        }
    } else if (has(L"MediaWidget")) {
        CssMedia m = CssMediaNow();
        if (!m.found && opt->Bool(L"hide_empty", true)) return;
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        yaml::Node const* icons = opt->Get(L"icons");
        auto glyph = [&](wchar_t const* key, wchar_t const* fallback) { return icons && !icons->Str(key).empty() ? icons->Str(key) : std::wstring(fallback); };
        auto button = [&](wchar_t const* which, std::wstring const& text) {
            css::Node* b = c->Add({L"btn", which}, text);
            if (!m.found) b->classes.push_back(L"disabled");
            CssPrepared::Act act{b, {}};
            act.target.left = std::wstring(L"media:") + which;
            if (g_hot == kHitCss + (int)p.acts.size()) CssHover(b);
            p.acts.push_back(act);
        };
        auto controls = [&] {
            button(L"prev", glyph(L"prev_track", L"\ue892"));
            button(L"play", m.playing ? glyph(L"pause", L"\ue769") : glyph(L"play", L"\ue768"));
            button(L"next", glyph(L"next_track", L"\ue893"));
        };
        bool left = opt->Bool(L"controls_left", true);
        if (left) controls();
        if (!opt->Bool(L"controls_only", false)) {
            yaml::Node const* limits = opt->Get(L"max_field_size");
            size_t limit = limits ? (size_t)limits->Num(g_css.alt.count(name) ? L"label_alt" : L"label", 0) : 0;
            auto cut = [&](std::wstring s) { return limit && s.size() > limit ? s.substr(0, limit) + L"..." : s; };
            std::wstring text = CssLabelOf(*opt, name), out;
            for (size_t k = 0; k < text.size(); k++) {
                size_t close = text[k] == L'{' ? text.find(L'}', k) : std::wstring::npos;
                if (close == std::wstring::npos) { out += text[k]; continue; }
                std::wstring key = text.substr(k + 1, close - k - 1);
                out += key == L"title" ? cut(m.title) : key == L"artist" ? cut(m.artist) : L"";
                k = close;
            }
            // "title - " with no artist known: without the dangling dash.
            out = css::Trim(out);
            if (out.size() > 2 && out.compare(out.size() - 2, 2, L" -") == 0) out = css::Trim(out.substr(0, out.size() - 2));
            if (out.rfind(L"- ", 0) == 0) out = css::Trim(out.substr(2));
            if (!out.empty()) c->Add({L"label"}, out);
        }
        if (!left) controls();    } else if (has(L"PowerMenuWidget") || has(L"HomeWidget") || has(L"CustomWidget")) {
        // The Start button is the main bar's only: the other screens' bars have none.
        yaml::Node const* callbacks = opt->Get(L"callbacks");
        bool startish = has(L"HomeWidget") || (callbacks && callbacks->Str(L"on_left").find(L"start_menu") != std::wstring::npos);
        if (startish && !mainBar) return;
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        // A custom widget's command output ({data}, {data[key]}); only with the setting that lets a theme run commands.
        std::string data = has(L"CustomWidget") ? CssExecData(name, opt->Get(L"exec_options")) : std::string();
        std::wstring dataText = CssFromUtf8(data);
        CssMarkup(c, CssLabelOf(*opt, name), [&](std::wstring const& k) {
            if (k.size() && k[0] == L'%') return CssStrftime(k);
            if (k == L"data") return dataText;
            if (k.rfind(L"data[", 0) == 0 && k.back() == L']') return CssFromUtf8(CssJsonValue(data, CssNarrow(k.substr(5, k.size() - 6))));
            return std::wstring();
        });
        CssPrepared::Act act{w, {}};
        CssCallbacks(act, *opt, name, L"power");
        if (has(L"HomeWidget") || act.target.left.find(L"start_menu") != std::wstring::npos) {
            if (mainBar) {
                p.start = w;
                if (g_hot == -1) CssHover(w);
            }
        } else if (has(L"PowerMenuWidget") || !act.target.left.empty() || !act.target.right.empty() || !act.target.middle.empty()) {
            if (has(L"PowerMenuWidget") && (act.target.left.empty() || act.target.left == L"popup:power")) act.target.left = L"popup:power";
            if (g_hot == kHitCss + (int)p.acts.size()) CssHover(w);
            p.acts.push_back(act);
        }
    } else if (has(L"ApplicationsWidget")) {
        css::Node* w = container->Add({L"widget", cls});
        css::Node* c = w->Add({L"widget-container"});
        if (auto const* apps = opt->Get(L"app_list"))
            for (auto const& a : apps->seq) {
                css::Node* label = c->Add({L"label"}, a.Str(L"icon"));
                CssPrepared::Act act{label, {}};
                act.target.left = a.Str(L"launch");
                act.target.tip = a.Str(L"tooltip");
                if (g_hot == kHitCss + (int)p.acts.size()) CssHover(label);
                p.acts.push_back(act);
            }
    }
    (void)name;
}

// ---------------------------------------------------------------------------------------------------- fades (transition lines)

// What a node was showing and what it is going to show, for the four things a theme's "transition" can name. Kept by node
// key from one build of the tree to the next, so a hover that starts changes the colours over the time the theme names.
struct CssFade {
    css::Rgba bgFrom, bgTo, colorFrom, colorTo, borderFrom[4], borderTo[4];
    double opacityFrom = 1, opacityTo = 1;
    double bgSec = 0, colorSec = 0, borderSec = 0, opacitySec = 0;
    ULONGLONG start = 0, seen = 0;
};

static std::map<std::wstring, CssFade> g_cssFades;
static bool g_cssFading = false;

static double CssEase(std::wstring const& name, double t) {
    t = std::clamp(t, 0.0, 1.0);
    double x1 = 0.25, y1 = 0.1, x2 = 0.25, y2 = 1;  // ease
    if (name == L"linear") return t;
    if (name == L"ease-in") { x1 = 0.42; y1 = 0; x2 = 1; y2 = 1; }
    else if (name == L"ease-out") { x1 = 0; y1 = 0; x2 = 0.58; y2 = 1; }
    else if (name == L"ease-in-out") { x1 = 0.42; y1 = 0; x2 = 0.58; y2 = 1; }
    else if (motion::Anim curve; !name.empty() && name != L"ease" && motion::CurveByName(name, &curve))
        return motion::Ease(curve, t);  // a motion curve by name: Hyprland's, easings.net's, or the settings' own
    auto bez = [](double a, double b, double s) { return 3 * a * s * (1 - s) * (1 - s) + 3 * b * s * s * (1 - s) + s * s * s; };
    double lo = 0, hi = 1, s = t;
    for (int i = 0; i < 24; i++) {  // solve x(s) = t
        s = (lo + hi) / 2;
        if (bez(x1, x2, s) < t) lo = s;
        else hi = s;
    }
    return bez(y1, y2, s);
}

// A colour between two, in premultiplied terms (a fade from clear to a colour doesn't pass through grey).
static css::Rgba CssMix(css::Rgba const& a, css::Rgba const& b, double t) {
    double pa = a.a * (1 - t) + b.a * t;
    if (pa <= 0.0001) return {b.r, b.g, b.b, 0};
    auto ch = [&](int x, int y) { return (int)std::lround((x * a.a * (1 - t) + y * b.a * t) / pa); };
    return {ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b), pa};
}

static double CssFadeSeconds(css::Style const& st, wchar_t const* prop, wchar_t const* shorthand, std::wstring* easing) {
    double best = 0;
    for (auto const& tr : st.transitions)
        if (tr.property == L"all" || tr.property == prop || tr.property == shorthand) {
            best = std::max(best, tr.seconds + tr.delay);
            *easing = tr.easing;
        }
    return best;
}

static void CssKeys(css::Node& n, std::wstring const& key) {
    n.key = key;
    for (size_t i = 0; i < n.kids.size(); i++)
        CssKeys(*n.kids[i], key + L"/" + std::to_wstring(i) + (n.kids[i]->classes.empty() ? L"" : n.kids[i]->classes[0]));
}

static void CssFadeNode(css::Node& n, ULONGLONG now) {
    css::Style& st = n.style;
    std::wstring easing = L"ease";
    double dBg = CssFadeSeconds(st, L"background-color", L"background", &easing), dColor = CssFadeSeconds(st, L"color", L"color", &easing),
           dBorder = CssFadeSeconds(st, L"border-color", L"border", &easing), dOpacity = CssFadeSeconds(st, L"opacity", L"opacity", &easing);
    if (g_settings.motionBar.set && !g_settings.motionBar.on) dBg = dColor = dBorder = dOpacity = 0;  // (the bar's motion off: no fades)
    if (dBg > 0 || dColor > 0 || dBorder > 0 || dOpacity > 0) {
        auto it = g_cssFades.find(n.key);
        if (it == g_cssFades.end()) {
            CssFade f;
            f.bgFrom = f.bgTo = st.background;
            f.colorFrom = f.colorTo = st.color;
            for (int i = 0; i < 4; i++) f.borderFrom[i] = f.borderTo[i] = st.borderColor[i];
            f.opacityFrom = f.opacityTo = st.opacity;
            f.seen = now;
            g_cssFades[n.key] = f;
        } else {
            CssFade& f = it->second;
            f.seen = now;
            auto progress = [&](double sec) { return sec > 0 ? CssEase(easing, (double)(now - f.start) / (sec * 1000)) : 1.0; };
            bool borderChanged = false;
            for (int i = 0; i < 4; i++) borderChanged |= !(f.borderTo[i] == st.borderColor[i]);
            if (!(f.bgTo == st.background) || !(f.colorTo == st.color) || borderChanged || f.opacityTo != st.opacity) {
                // Starts from what is showing now, so a hover let go half way turns round from there.
                double tb = progress(f.bgSec), tc = progress(f.colorSec), tr = progress(f.borderSec), to = progress(f.opacitySec);
                f.bgFrom = CssMix(f.bgFrom, f.bgTo, tb);
                f.colorFrom = CssMix(f.colorFrom, f.colorTo, tc);
                for (int i = 0; i < 4; i++) f.borderFrom[i] = CssMix(f.borderFrom[i], f.borderTo[i], tr);
                f.opacityFrom = f.opacityFrom + (f.opacityTo - f.opacityFrom) * to;
                f.bgTo = st.background;
                f.colorTo = st.color;
                for (int i = 0; i < 4; i++) f.borderTo[i] = st.borderColor[i];
                f.opacityTo = st.opacity;
                f.bgSec = dBg;
                f.colorSec = dColor;
                f.borderSec = dBorder;
                f.opacitySec = dOpacity;
                f.start = now;
            }
            auto t = [&](double sec) {
                if (sec <= 0) return 1.0;
                double x = (double)(now - f.start) / (sec * 1000);
                if (x < 1) g_cssFading = true;
                return CssEase(easing, x);
            };
            if (!st.hasGradient) st.background = CssMix(f.bgFrom, f.bgTo, t(f.bgSec));
            st.color = CssMix(f.colorFrom, f.colorTo, t(f.colorSec));
            double tr = t(f.borderSec);
            for (int i = 0; i < 4; i++) st.borderColor[i] = CssMix(f.borderFrom[i], f.borderTo[i], tr);
            st.opacity = f.opacityFrom + (f.opacityTo - f.opacityFrom) * t(f.opacitySec);
        }
    }
    for (auto& k : n.kids) CssFadeNode(*k, now);
}

static void CssFadeTree(css::Node& root, ULONGLONG now) {
    g_cssFading = false;
    CssFadeNode(root, now);
    if (g_cssFades.size() > 300)
        for (auto it = g_cssFades.begin(); it != g_cssFades.end();) it = now - it->second.seen > 3000 ? g_cssFades.erase(it) : std::next(it);
}
static CssPrepared CssPrepare(HWND wnd) {
    CssPrepared p;
    css::LengthScale() = g_scale;
    RECT rc;
    GetClientRect(wnd, &rc);
    p.root = std::make_unique<css::Node>();
    css::Node& root = *p.root;
    root.classes = {L"yasb-bar"};
    if (CssDarkMode()) root.classes.push_back(L"dark");
    yaml::Node const* bar = CssBarConfig();
    if (bar) {
        std::wstring cls = bar->Str(L"class_name", L"yasb-bar");
        if (cls != L"yasb-bar") root.classes.push_back(cls);
    }
    const wchar_t* sides[] = {L"left", L"center", L"right"};
    css::Node* containers[3];
    for (int i = 0; i < 3; i++) containers[i] = root.Add({L"container", std::wstring(L"container-") + sides[i]});
    if (g_settings.yasbFill && !g_css.hasStart && IsMainBar()) CssAddWidget(p, containers[0], L"StartWidget", nullptr, true);
    yaml::Node const* widgets = bar ? bar->Get(L"widgets") : nullptr;
    for (int i = 0; i < 3; i++) {
        yaml::Node const* list = widgets ? widgets->Get(sides[i]) : nullptr;
        if (!list) continue;
        for (auto const& item : list->seq) {
            yaml::Node const* def = CssWidgetDef(item.s);
            if (def) CssAddWidget(p, containers[i], item.s, def);
        }
    }
    // A theme without task buttons or a tray gets ours, so the bar is still a taskbar (the setting can turn it off).
    if (g_settings.yasbFill) {
        if (!g_css.hasTaskbar) CssAddWidget(p, containers[0], L"TaskbarWidget", nullptr, true);
        if (!g_css.hasTray && (IsMainBar() || g_settings.trayEverywhere)) CssAddWidget(p, containers[2], L"SystrayWidget", nullptr, true);
    }
    css::ComputeStyle(root, g_css.sheet);
    CssKeys(root, std::to_wstring((uintptr_t)wnd));
    if (g_settings.yasbFades) CssFadeTree(root, GetTickCount64());
    else g_cssFading = false;

    // Laid out in the window: the bar's own padding, the three containers across it (the middle one centred).
    css::Style const& rs = root.style;
    double w = rc.right, h = rc.bottom;
    root.x = 0;
    root.y = 0;
    root.w = w;
    root.h = h;
    cssdraw::RectD content = cssdraw::ContentBox(root);
    double widths[3];
    for (int i = 0; i < 3; i++) widths[i] = cssdraw::LayoutMeasure(*containers[i]);
    double xs[3] = {content.x0, (w - widths[1]) / 2, content.x1 - widths[2]};
    for (int i = 0; i < 3; i++) cssdraw::LayoutPlace(*containers[i], xs[i], content.y0, content.y1 - content.y0);
    (void)rs;
    return p;
}

static RECT CssRect(css::Node const& n) {
    return {(LONG)std::floor(n.x), (LONG)std::floor(n.y), (LONG)std::ceil(n.x + n.w), (LONG)std::ceil(n.y + n.h)};
}

void CssLayout(HWND wnd) {
    CssPrepared p = CssPrepare(wnd);
    (void)wnd;
    // Where the suite's own hit testing looks: the Start button, task buttons, tray icons, clock, and the theme's other buttons.
    g_orbRect = p.start ? CssRect(*p.start) : RECT{};
    for (int i = 0; i < (int)g_buttons.size(); i++) {
        g_buttons[i].rc = i < (int)p.apps.size() ? CssRect(*p.apps[i]) : RECT{};
        g_buttons[i].stack = 0;
    }
    for (int i = 0; i < (int)g_tray.size(); i++) g_tray[i].rc = i < (int)p.trays.size() ? CssRect(*p.trays[i]) : RECT{};
    g_clockRect = p.clock ? CssRect(*p.clock) : RECT{};
    g_cssTargets.clear();
    for (auto& a : p.acts) {
        CssTarget t = a.target;
        t.rc = CssRect(*a.node);
        g_cssTargets.push_back(t);
    }
    g_islands.clear();
    g_centralIsland = -1;
    g_labels = false;
    UpdateTooltips(wnd);
}

// Blends a tray icon's own pixels (already multiplied by their alpha) into the surface.
static void CssBlendTray(cssdraw::Surface& sf, HiddenIcon const& icon, int x0, int y0) {
    int size = icon.size;
    if (!size || icon.pixels.size() < (size_t)size * size) return;
    for (int y = 0; y < size; y++) {
        int ty = y0 + y;
        if (ty < 0 || ty >= sf.h) continue;
        for (int x = 0; x < size; x++) {
            int tx = x0 + x;
            if (tx < 0 || tx >= sf.w) continue;
            uint32_t src = icon.pixels[y * size + x];
            uint32_t a = src >> 24;
            if (!a) continue;
            uint32_t& dst = sf.px[(size_t)ty * sf.w + tx];
            uint32_t out = 0;
            for (int shift = 0; shift < 32; shift += 8) {
                uint32_t s = (src >> shift) & 0xFF, d = (dst >> shift) & 0xFF;
                out |= std::min<uint32_t>(255, s + d * (255 - a) / 255) << shift;
            }
            dst = out;
        }
    }
}

void CssRedraw(HWND wnd) {
    RECT rc;
    GetClientRect(wnd, &rc);
    if (rc.right <= 0 || rc.bottom <= 0) return;
    CssPrepared p = CssPrepare(wnd);
    if (g_cssFading) SetTimer(wnd, kTimerCssFade, 16, nullptr);
    else KillTimer(wnd, kTimerCssFade);
    cssdraw::Surface sf(rc.right, rc.bottom);
    cssdraw::PaintTree(sf, *p.root, [&](css::Node& n, cssdraw::RectD c) {
        int size = (int)std::lround(n.iconSize);
        int x = (int)std::lround(c.x0 + ((c.x1 - c.x0) - size) / 2), y = (int)std::lround(c.y0 + ((c.y1 - c.y0) - size) / 2);
        auto it = p.icons.find(&n);
        if (it != p.icons.end()) {
            if (it->second) cssdraw::PaintIcon(sf, it->second, x, y, size);
            return;
        }
        auto tray = p.trayOf.find(&n);
        if (tray != p.trayOf.end() && tray->second < (int)g_tray.size()) {
            HiddenIcon const& icon = g_tray[tray->second].icon;
            CssBlendTray(sf, icon, (int)std::lround(c.x0 + ((c.x1 - c.x0) - icon.size) / 2), (int)std::lround(c.y0 + ((c.y1 - c.y0) - icon.size) / 2));
        }
    });
    HDC screen = GetDC(nullptr);
    Surface out(screen, rc.right, rc.bottom);
    memcpy(out.bits, sf.px.data(), sf.px.size() * sizeof(uint32_t));
    // Between the theme's islands: all but see-through (alpha 1), so the bar still takes clicks there as its empty
    // space (a double-click shows the desktop) instead of passing them to the desktop under it.
    std::vector<RECT> through = BarPassThrough(wnd);  // (Tourne'Table's strip under it keeps its clicks)
    for (size_t i = 0, n = sf.px.size(); i < n; i++)
        if (!(out.bits[i] >> 24) && !InPassThrough(through, (int)(i % rc.right), (int)(i / rc.right))) out.bits[i] = 0x01000000;
    GdiFlush();
    RECT wr;
    GetWindowRect(wnd, &wr);
    POINT dst{wr.left, wr.top}, src{0, 0};
    SIZE size{rc.right, rc.bottom};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, g_barAlpha, AC_SRC_ALPHA};
    UpdateLayeredWindow(wnd, screen, &dst, &size, out.dc, &src, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
}
