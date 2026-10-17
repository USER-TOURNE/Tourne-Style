// The spike: reads a YASB theme (styles.css + config.yaml), builds the bar's element tree the way YASB does, styles it
// with css-engine.cpp, lays it out and draws it with css-draw.cpp into a picture.
//
//   cssspike <theme dir> <out.bmp> [hover-class]     draw one theme (the bar on a stand-in wallpaper)
//   cssspike --survey <themes dir>                    parse every theme, report what is understood and what isn't
#include <windows.h>

#include <shellapi.h>

#include <cstdio>
#include <ctime>
#include <functional>

#include "..\..\css-engine.cpp"
#include "..\..\yaml-lite.cpp"
#include "..\..\css-draw.cpp"

using css::Node;

static std::wstring ReadText(std::wstring const& path) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return L"";
    DWORD size = GetFileSize(h, nullptr), got = 0;
    std::string bytes(size, 0);
    ReadFile(h, bytes.data(), size, &got, nullptr);
    CloseHandle(h);
    size_t skip = bytes.size() >= 3 && (uint8_t)bytes[0] == 0xEF && (uint8_t)bytes[1] == 0xBB && (uint8_t)bytes[2] == 0xBF ? 3 : 0;
    int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), nullptr, 0);
    std::wstring out(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, bytes.data() + skip, (int)(bytes.size() - skip), out.data(), n);
    return out;
}

// ---------------------------------------------------------------------------------------------------- stand-in data

static std::wstring StrfTime(std::wstring const& fmt) {
    static const wchar_t* days[] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday"};
    static const wchar_t* months[] = {L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December"};
    SYSTEMTIME t;
    GetLocalTime(&t);
    std::wstring out;
    wchar_t buf[16];
    for (size_t i = 0; i < fmt.size(); i++) {
        if (fmt[i] != L'%' || i + 1 >= fmt.size()) { out += fmt[i]; continue; }
        wchar_t c = fmt[++i];
        switch (c) {
            case L'a': out += std::wstring(days[t.wDayOfWeek]).substr(0, 3); break;
            case L'A': out += days[t.wDayOfWeek]; break;
            case L'b': out += std::wstring(months[t.wMonth - 1]).substr(0, 3); break;
            case L'B': out += months[t.wMonth - 1]; break;
            case L'd': swprintf(buf, 16, L"%02d", t.wDay); out += buf; break;
            case L'e': swprintf(buf, 16, L"%2d", t.wDay); out += buf; break;
            case L'H': swprintf(buf, 16, L"%02d", t.wHour); out += buf; break;
            case L'I': swprintf(buf, 16, L"%02d", t.wHour % 12 ? t.wHour % 12 : 12); out += buf; break;
            case L'M': swprintf(buf, 16, L"%02d", t.wMinute); out += buf; break;
            case L'S': swprintf(buf, 16, L"%02d", t.wSecond); out += buf; break;
            case L'p': out += t.wHour < 12 ? L"AM" : L"PM"; break;
            case L'Y': swprintf(buf, 16, L"%04d", t.wYear); out += buf; break;
            case L'y': swprintf(buf, 16, L"%02d", t.wYear % 100); out += buf; break;
            case L'm': swprintf(buf, 16, L"%02d", t.wMonth); out += buf; break;
            default: out += c;
        }
    }
    return out;
}

static std::vector<HICON> g_icons;

static HICON FileIcon(wchar_t const* path) {
    HICON small = nullptr;
    ExtractIconExW(path, 0, nullptr, &small, 1);
    return small;
}

// ---------------------------------------------------------------------------------------------------- the element tree

struct Env {
    yaml::Node const* widgets = nullptr;
    int specific = 0, generic = 0;
    std::map<std::wstring, int> genericTypes;
};

static std::wstring ClassOfType(std::wstring const& type) {
    static const std::map<std::wstring, std::wstring> known = {
        {L"clock.ClockWidget", L"clock-widget"},      {L"volume.VolumeWidget", L"volume-widget"},
        {L"media.MediaWidget", L"media-widget"},      {L"weather.WeatherWidget", L"weather-widget"},
        {L"power_menu.PowerMenuWidget", L"power-menu-widget"}, {L"workspaces.WorkspaceWidget", L"komorebi-workspaces"},
        {L"active_layout.ActiveLayoutWidget", L"komorebi-active-layout"}, {L"active_window.ActiveWindowWidget", L"active-window-widget"},
        {L"applications.ApplicationsWidget", L"apps-widget"}, {L"taskbar.TaskbarWidget", L"taskbar-widget"},
        {L"home.HomeWidget", L"home-widget"},         {L"memory.MemoryWidget", L"memory-widget"},
        {L"cpu.CpuWidget", L"cpu-widget"},            {L"notifications.NotificationsWidget", L"notification-widget"},
        {L"systray.SystrayWidget", L"systray"},       {L"wifi.WifiWidget", L"wifi-widget"},
        {L"battery.BatteryWidget", L"battery-widget"}, {L"disk.DiskWidget", L"disk-widget"},
    };
    size_t dot = type.find(L'.');
    std::wstring tail = dot == std::wstring::npos ? type : type.substr(dot + 1);
    auto it = known.find(tail);
    if (it != known.end()) return it->second;
    // yasb.something.SomethingWidget: "something-widget"
    size_t a = tail.find(L'.');
    std::wstring base = a == std::wstring::npos ? tail : tail.substr(0, a);
    for (auto& c : base)
        if (c == L'_') c = L'-';
    return base + L"-widget";
}

// "<span>{icon}</span> {temp}": the spans become .icon labels, the rest .label ones.
static void AddMarkup(Node* parent, std::wstring markup, std::function<std::wstring(std::wstring const&)> fill,
                      std::wstring const& iconClass = L"") {
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
    size_t at = 0;
    auto label = [&](std::wstring text, bool icon) {
        text = css::Trim(text);
        if (text.empty()) return;
        Node* n = icon ? parent->Add({L"icon"}, text) : parent->Add({L"label"}, text);
        if (icon && !iconClass.empty()) n->classes.push_back(iconClass);
    };
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

static Node* AddWidget(Node* container, std::wstring const& name, Env& env) {
    yaml::Node const* def = env.widgets ? env.widgets->Get(name) : nullptr;
    if (!def) return nullptr;
    std::wstring type = def->Str(L"type");
    yaml::Node const* opt = def->Get(L"options");
    yaml::Node empty;
    if (!opt) opt = &empty;
    std::wstring cls = ClassOfType(type);
    if (type.find(L"CustomWidget") != std::wstring::npos && !opt->Str(L"class_name").empty()) cls = opt->Str(L"class_name") + L"-widget";
    if (type.find(L"CustomWidget") != std::wstring::npos && !opt->Str(L"class_name").empty()) cls = opt->Str(L"class_name");
    if (type.find(L"MediaWidget") != std::wstring::npos && opt->Bool(L"hide_empty", false)) {
        env.specific++;
        return nullptr;  // nothing playing
    }
    Node* w = container->Add({L"widget", cls});
    Node* c = w->Add({L"widget-container"});
    auto plain = [](std::wstring const& key) -> std::wstring {
        if (key.size() && key[0] == L'%') return StrfTime(key);
        return L"";
    };
    bool built = true;
    if (type.find(L"ClockWidget") != std::wstring::npos) {
        AddMarkup(c, opt->Str(L"label"), plain);
    } else if (type.find(L"CustomWidget") != std::wstring::npos || type.find(L"PowerMenuWidget") != std::wstring::npos) {
        AddMarkup(c, opt->Str(L"label"), plain);
    } else if (type.find(L"VolumeWidget") != std::wstring::npos) {
        yaml::Node const* icons = opt->Get(L"icons");
        std::wstring glyph = icons && icons->seq.size() > 3 ? icons->seq[3].s : L"";
        AddMarkup(c, opt->Str(L"label"), [&](std::wstring const& k) { return k == L"icon" ? glyph : k == L"level" ? L"36%" : L"36"; });
    } else if (type.find(L"WeatherWidget") != std::wstring::npos) {
        yaml::Node const* icons = opt->Get(L"icons");
        std::wstring glyph = icons ? icons->Str(L"sunnyDay") : L"";
        AddMarkup(c, opt->Str(L"label"), [&](std::wstring const& k) { return k == L"icon" ? glyph : k == L"temp" ? L"33\u00B0C" : L"?"; }, L"sunnyDay");
    } else if (type.find(L"ActiveWindowWidget") != std::wstring::npos) {
        Node* icon = c->Add({L"icon"});
        icon->payload = 1;
        icon->iconSize = opt->Num(L"label_icon_size", 14);
        AddMarkup(c, opt->Str(L"label"), [&](std::wstring const& k) { return k.find(L"title") != std::wstring::npos ? L"amnweb/yasb: A highly configurable cross-platform (Windo..." : L""; });
    } else if (type.find(L"komorebi.workspaces") != std::wstring::npos || type.find(L"WorkspaceWidget") != std::wstring::npos) {
        std::wstring off = opt->Str(L"label_workspace_btn", L"1"), on = opt->Str(L"label_workspace_active_btn", off);
        for (int i = 0; i < 4; i++) {
            Node* b = c->Add(i == 0 ? std::initializer_list<std::wstring>{L"ws-btn", L"active", L"populated"} : std::initializer_list<std::wstring>{L"ws-btn"}, i == 0 ? on : off);
            (void)b;
        }
    } else if (type.find(L"ActiveLayoutWidget") != std::wstring::npos) {
        AddMarkup(c, opt->Str(L"label"), [&](std::wstring const&) { return L"BSP"; });
    } else if (type.find(L"ApplicationsWidget") != std::wstring::npos) {
        yaml::Node const* apps = opt->Get(L"app_list");
        if (apps)
            for (auto const& a : apps->seq) c->Add({L"label"}, a.Str(L"icon"));
    } else if (type.find(L"TaskbarWidget") != std::wstring::npos) {
        int size = (int)opt->Num(L"icon_size", 16);
        for (int i = 0; i < 2; i++) {
            Node* app = c->Add({L"app-container"});
            Node* icon = app->Add({L"app-icon"});
            icon->payload = 2 + i;
            icon->iconSize = size;
        }
    } else {
        built = false;
        AddMarkup(c, opt->Str(L"label"), [&](std::wstring const&) { return L"0"; });
    }
    if (built) env.specific++;
    else {
        env.generic++;
        env.genericTypes[type]++;
    }
    return w;
}

struct Bar {
    std::unique_ptr<Node> root;
    int height = 36, top = 8, left = 8, right = 8, bottom = 8;
    std::wstring cls = L"yasb-bar";
    std::wstring position = L"top";
};

static Bar BuildBar(yaml::Node const& cfg, Env& env) {
    Bar bar;
    env.widgets = cfg.Get(L"widgets");
    yaml::Node const* bars = cfg.Get(L"bars");
    yaml::Node const* first = bars && !bars->map.empty() ? &bars->map[0].second : nullptr;
    bar.root = std::make_unique<Node>();
    if (first) {
        bar.cls = first->Str(L"class_name", L"yasb-bar");
        if (auto const* al = first->Get(L"alignment")) bar.position = al->Str(L"position", L"top");
        if (auto const* d = first->Get(L"dimensions")) bar.height = (int)d->Num(L"height", 36);
        if (auto const* p = first->Get(L"padding")) {
            bar.top = (int)p->Num(L"top", 0); bar.left = (int)p->Num(L"left", 0);
            bar.right = (int)p->Num(L"right", 0); bar.bottom = (int)p->Num(L"bottom", 0);
        }
    }
    bar.root->classes = {L"yasb-bar"};
    if (bar.cls != L"yasb-bar") bar.root->classes.push_back(bar.cls);
    const wchar_t* sides[] = {L"left", L"center", L"right"};
    for (auto side : sides) {
        Node* c = bar.root->Add({L"container", std::wstring(L"container-") + side});
        yaml::Node const* widgets = first ? first->Get(L"widgets") : nullptr;
        yaml::Node const* list = widgets ? widgets->Get(side) : nullptr;
        if (list)
            for (auto const& name : list->seq) AddWidget(c, name.s, env);
    }
    return bar;
}

// ---------------------------------------------------------------------------------------------------- layout and paint

// Drawing the tree: an icon node draws the icon its payload names (1 the window's, 2+ the taskbar's).
static void Paint(cssdraw::Surface& sf, Node& root) {
    cssdraw::PaintTree(sf, root, [&](Node& n, cssdraw::RectD c) {
        int idx = n.payload - 1;
        if (idx < 0 || idx >= (int)g_icons.size()) return;
        int size = (int)n.iconSize;
        cssdraw::PaintIcon(sf, g_icons[idx], (int)std::lround(c.x0 + ((c.x1 - c.x0) - size) / 2), (int)std::lround(c.y0 + ((c.y1 - c.y0) - size) / 2), size);
    });
}
static void SetHover(Node& root, std::wstring spec) {
    int skip = 0;
    size_t colon = spec.find(L':');
    if (colon != std::wstring::npos) {
        skip = _wtoi(spec.c_str() + colon + 1);
        spec.resize(colon);
    }
    std::wstring cls = spec;
    std::function<Node*(Node&)> find = [&](Node& n) -> Node* {
        if (n.Has(cls) && n.kids.empty() && skip-- == 0) return &n;
        for (auto& k : n.kids)
            if (Node* f = find(*k)) return f;
        return nullptr;
    };
    if (Node* target = find(root))
        for (Node* p = target; p; p = p->parent) p->states.insert(L"hover");
}

static bool SaveBmp(cssdraw::Surface const& sf, wchar_t const* path) {
    int stride = (sf.w * 3 + 3) & ~3;
    std::vector<uint8_t> rows((size_t)stride * sf.h, 0);
    for (int y = 0; y < sf.h; y++)
        for (int x = 0; x < sf.w; x++) {
            uint32_t p = sf.px[(size_t)y * sf.w + x];
            uint8_t* d = &rows[(size_t)(sf.h - 1 - y) * stride + x * 3];
            d[0] = p & 255; d[1] = (p >> 8) & 255; d[2] = (p >> 16) & 255;
        }
    FILE* f = _wfopen(path, L"wb");
    if (!f) return false;
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + rows.size()), 0, 0, 54};
    BITMAPINFOHEADER ih{sizeof(ih), sf.w, sf.h, 1, 24, BI_RGB};
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&ih, sizeof(ih), 1, f);
    fwrite(rows.data(), rows.size(), 1, f);
    fclose(f);
    return true;
}

static int RenderTheme(std::wstring const& dir, std::wstring const& outPath, std::wstring const& hover) {
    css::Sheet sheet = css::ParseSheet(ReadText(dir + L"\\styles.css"));
    yaml::Node cfg = yaml::Parse(ReadText(dir + L"\\config.yaml"));
    g_icons = {FileIcon(L"C:\\Windows\\explorer.exe"), FileIcon(L"C:\\Windows\\System32\\notepad.exe"), FileIcon(L"C:\\Windows\\System32\\cmd.exe")};
    Env env;
    Bar bar = BuildBar(cfg, env);
    if (!hover.empty()) SetHover(*bar.root, hover);
    LARGE_INTEGER q0, q1, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&q0);
    for (int rep = 0; rep < 20; rep++) {
        Env e2;
        Bar again = BuildBar(cfg, e2);
        css::ComputeStyle(*again.root, sheet);
    }
    QueryPerformanceCounter(&q1);
    wprintf(L"build + style, 20 times: %.2f ms each\n", (q1.QuadPart - q0.QuadPart) * 1000.0 / freq.QuadPart / 20);
    css::ComputeStyle(*bar.root, sheet);

    int canvasW = 1920, barW = canvasW - bar.left - bar.right, canvasH = bar.top + bar.height + std::max(bar.bottom, 30);
    cssdraw::Surface sf(canvasW, canvasH);
    // A stand-in wallpaper: a purple gradient, opaque.
    for (int y = 0; y < canvasH; y++)
        for (int x = 0; x < canvasW; x++) {
            double t = (x / (double)canvasW) * 0.6 + (y / (double)canvasH) * 0.4;
            int r = (int)(74 - 30 * t), g = (int)(24 - 10 * t), b = (int)(120 - 40 * t);
            sf.px[(size_t)y * canvasW + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    Node& root = *bar.root;
    // The bar's own box, then the three containers inside its padding.
    double innerLeft = 0;
    (void)innerLeft;
    css::Style const& rs = root.style;
    double contentH = bar.height - rs.padding.top - rs.padding.bottom - rs.borderWidth.top - rs.borderWidth.bottom;
    double contentX = bar.left + rs.borderWidth.left + rs.padding.left;
    double contentW = barW - rs.borderWidth.left - rs.borderWidth.right - rs.padding.left - rs.padding.right;
    root.x = bar.left;
    root.y = bar.top;
    root.w = barW;
    root.h = bar.height;
    double widths[3];
    for (int i = 0; i < 3; i++) widths[i] = cssdraw::LayoutMeasure(*root.kids[i]);
    double xs[3] = {contentX, bar.left + (barW - widths[1]) / 2, contentX + contentW - widths[2]};
    for (int i = 0; i < 3; i++) cssdraw::LayoutPlace(*root.kids[i], xs[i], root.y + rs.borderWidth.top + rs.padding.top, contentH);
    Paint(sf, root);
    std::wstring name = outPath;
    SaveBmp(sf, name.c_str());
    wprintf(L"drew %ls: %d rules kept of %d, %d widgets built, %d generic\n", dir.c_str(), (int)sheet.rules.size(), sheet.report.rules, env.specific, env.generic);
    return 0;
}

// ---------------------------------------------------------------------------------------------------- survey

static int Survey(std::wstring const& dir) {
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
    int themes = 0, cfgOk = 0, decls = 0, understood = 0, rules = 0, kept = 0, skipped = 0;
    std::map<std::wstring, int> unknown, why, genericTypes;
    int specific = 0, generic = 0, fullyClean = 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == L'.') continue;
        std::wstring d = dir + L"\\" + fd.cFileName;
        std::wstring cssText = ReadText(d + L"\\styles.css");
        if (cssText.empty()) continue;
        themes++;
        css::Sheet sheet = css::ParseSheet(cssText);
        css::Report scan;
        for (auto const& r : sheet.rules)
            for (auto const& dcl : r.decls) {
                css::Style dummy;
                size_t before = scan.unknownProps.size();
                int total = 0;
                for (auto const& kv : scan.unknownProps) total += kv.second;
                css::ApplyDecl(dummy, dcl.name, css::ResolveVars(dcl.value, sheet.vars), &scan);
                int after = 0;
                for (auto const& kv : scan.unknownProps) after += kv.second;
                (void)before;
                decls++;
                if (after == total) understood++;
            }
        rules += sheet.report.rules;
        kept += (int)sheet.rules.size();
        skipped += sheet.report.skippedRules;
        for (auto const& kv : scan.unknownProps) unknown[kv.first] += kv.second;
        for (auto const& kv : sheet.report.skippedSelectors) why[kv.first] += kv.second;
        if (scan.unknownProps.empty() && sheet.report.skippedSelectors.empty()) fullyClean++;
        yaml::Node cfg = yaml::Parse(ReadText(d + L"\\config.yaml"));
        Env env;
        Bar bar = BuildBar(cfg, env);
        int count = 0;
        for (auto& c : bar.root->kids) count += (int)c->kids.size();
        if (count > 0) cfgOk++;
        specific += env.specific;
        generic += env.generic;
        for (auto const& kv : env.genericTypes) genericTypes[kv.first] += kv.second;
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    wprintf(L"themes %d, config layouts read %d\n", themes, cfgOk);
    wprintf(L"rules %d (kept %d, skipped or partly skipped %d); declarations %d, acted on %d (%d%%)\n", rules, kept, skipped, decls, understood,
            decls ? 100 * understood / decls : 0);
    wprintf(L"themes with nothing we skip or ignore: %d of %d\n", fullyClean, themes);
    wprintf(L"widgets built with their own shape %d, generic %d\n", specific, generic);
    auto top = [](std::map<std::wstring, int> const& m, wchar_t const* title, size_t n) {
        std::vector<std::pair<int, std::wstring>> v;
        for (auto const& kv : m) v.push_back({kv.second, kv.first});
        std::sort(v.rbegin(), v.rend());
        wprintf(L"%ls:", title);
        for (size_t i = 0; i < std::min(n, v.size()); i++) wprintf(L" %ls=%d", v[i].second.c_str(), v[i].first);
        wprintf(L"\n");
    };
    top(unknown, L"ignored properties", 30);
    top(why, L"skipped selector parts", 20);
    top(genericTypes, L"widget types still generic", 25);
    return 0;
}

// --fonts: which fonts the theme's families and the icon fallbacks turn out to be here, and who has a glyph.
static int FontReport() {
    cssdraw::Fonts& fc = cssdraw::FontCache();
    for (wchar_t const* fam : {L"Segoe UI", L"JetBrainsMono NFP", L"FiraCode Nerd Font Propo", L"FiraCode Nerd Font", L"Segoe Fluent Icons", L"Segoe UI Symbol"}) {
        HFONT f = fc.Get(fam, 13, 600);
        HGDIOBJ old = SelectObject(fc.dc, f);
        WCHAR got[LF_FACESIZE] = L"";
        GetTextFaceW(fc.dc, LF_FACESIZE, got);
        SelectObject(fc.dc, old);
        wprintf(L"%ls -> face \"%ls\" installed=%d has E62A=%d has F011=%d has 'A'=%d\n", fam, got, fc.Installed(fam), fc.Has(f, 0xE62A), fc.Has(f, 0xF011), fc.Has(f, L'A'));
    }
    css::Style st;
    st.fontFamily = {L"Segoe UI", L"JetBrainsMono NFP"};
    for (auto const& r : cssdraw::Shape(L"a", st)) {
        LOGFONTW lf{};
        GetObjectW(r.font, sizeof(lf), &lf);
        wprintf(L"run \"%ls\" (%zu chars) in %ls\n", r.text.c_str(), r.text.size(), lf.lfFaceName);
    }
    return 0;
}

int wmain(int argc, wchar_t** argv) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (argc >= 2 && !wcscmp(argv[1], L"--fonts")) return FontReport();
    if (argc >= 3 && !wcscmp(argv[1], L"--survey")) return Survey(argv[2]);
    if (argc >= 3) return RenderTheme(argv[1], argv[2], argc > 3 ? argv[3] : L"");
    wprintf(L"usage: cssspike <theme dir> <out.bmp> [hover-class] | --survey <themes dir>\n");
    return 1;
}
