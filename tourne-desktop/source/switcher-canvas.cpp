////////////////////////////////////////////////////////////////////////////////
// Canvas (view 7): every window as a live picture on an infinite canvas you
// pan, flick and zoom, as PrinceWM does it (Alt+Tab replacement by prince,
// MIT licence: third_party/PrinceWM-LICENSE.txt; ported from its C# 1.4.1).
// Tiles keep the places you give them (by app, from one opening to the next);
// an app's windows make one stack, opened with a middle click; one tile
// dropped on another puts both windows side by side on the screen. Notes and
// pasted pictures pin to the canvas (the pinboard), and paint mode draws on it.
//
// Inlined into switcher-host.cpp after switcher-views.cpp: the same window,
// device, captures and tiles (one per entry, as ViewSync makes them), drawn
// flat with a 2D camera instead of a 3D scene. The switcher stays open until
// something is picked (sticky), as PrinceWM's canvas does.
//
// Mouse: left drag pans (and flicks), the wheel zooms at the pointer, a click
// switches, middle or Alt+left drag moves a tile (Shift: the tiles touching
// it too), middle click opens a stack, right click: menus. Keys: arrows move
// the selection by direction (Shift: the tile), Enter or Space switches, Esc
// backs out, W fits all, F fills with the chosen one, C centres it, Z or 0
// zooms to 1:1, + and - zoom, R lays them out again, N a new note, Ctrl+V
// pastes a picture, D paint mode, Q or Delete closes.

#include <cfloat>
#include <map>

void PlaceWindowFrame(HWND h, RECT const& want);  // (binds.cpp, inlined later)

constexpr UINT WM_APP_CANVASKEY = WM_APP + 20;  // wParam: the key; lParam: scan code, Shift (bit 16), Ctrl (bit 17)

namespace canvas {

constexpr float kGap = 56, kMinZoom = 0.12f, kMaxZoom = 8, kHoverLift = 0.03f, kDragThreshold = 6, kNudge = 20;
constexpr float kCommitSeconds = 0.2f, kOpenSeconds = 0.16f;
constexpr float kStrokeSizes[] = {2, 4, 8, 16, 28};

struct Vec {
    float x = 0, y = 0;
};
Vec operator+(Vec a, Vec b) { return {a.x + b.x, a.y + b.y}; }
Vec operator-(Vec a, Vec b) { return {a.x - b.x, a.y - b.y}; }
Vec operator*(Vec a, float k) { return {a.x * k, a.y * k}; }
Vec Lerp(Vec a, Vec b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}; }
float Length2(Vec a) { return a.x * a.x + a.y * a.y; }

// PrinceWM's Camera: where the view looks (world units are the windows' own pixels) and how close.
struct Camera {
    Vec view, center, target, vel;
    float zoom = 1, targetZoom = 1;
    bool tween = false, easeOut = false;
    float tweenT = 0, tweenDur = 0, power = 5;
    Vec fromC, toC;
    float fromZ = 1, toZ = 1;

    Vec ToScreen(Vec w) const { return (w - center) * zoom + view * 0.5f; }
    Vec ToWorld(Vec s) const { return (s - view * 0.5f) * (1 / zoom) + center; }
    Vec ToWorldTarget(Vec s) const { return (s - view * 0.5f) * (1 / targetZoom) + target; }
    void SnapTo(Vec c, float z) {
        tween = false;
        center = target = c;
        zoom = targetZoom = std::clamp(z, kMinZoom, kMaxZoom);
        vel = {};
    }
    void CancelTween() {
        if (!tween) return;
        tween = false;
        target = center;
        targetZoom = zoom;
    }
    void PanBy(Vec screen) {
        CancelTween();
        target = target - screen * (1 / targetZoom);
        vel = {};
    }
    void Flick(Vec screenVel) {
        CancelTween();
        vel = screenVel * (-1 / targetZoom);
    }
    void ZoomAt(Vec anchor, float factor) {
        CancelTween();
        Vec before = ToWorldTarget(anchor);
        targetZoom = std::clamp(targetZoom * factor, kMinZoom, kMaxZoom);
        target = target + (before - ToWorldTarget(anchor));
    }
    void TweenTo(Vec c, float z, float seconds, bool out = false, float pow = 5) {
        fromC = center, fromZ = zoom, toC = c, toZ = std::clamp(z, kMinZoom, kMaxZoom);
        tweenT = 0, tweenDur = std::max(0.0001f, seconds), easeOut = out, power = pow, tween = true;
        vel = {};
        target = toC, targetZoom = toZ;
    }
    void PanTargetTo(Vec c) {
        CancelTween();
        target = c;
        vel = {};
    }
    void Step(float dt) {
        if (tween) {
            tweenT += dt;
            float u = std::clamp(tweenT / tweenDur, 0.f, 1.f);
            float e = easeOut ? 1 - std::pow(1 - u, power) : u * u * u * (u * (u * 6 - 15) + 10);
            center = Lerp(fromC, toC, e);
            zoom = fromZ * std::pow(toZ / fromZ, e);
            if (u >= 1) tween = false, center = toC, zoom = toZ;
            return;
        }
        if (Length2(vel) > 0.01f) {
            target = target + vel * dt;
            vel = vel * std::exp(-dt * 3.2f);
            if (Length2(vel) < 0.01f) vel = {};
        }
        float t = 1 - std::exp(-dt * 16);
        center = Lerp(center, target, t);
        zoom *= std::pow(targetZoom / std::max(zoom, 1e-4f), t);
    }
};

// One tile: an entry of the switcher (an app's stack, or one window when opened).
struct Item {
    int entry = 0;
    std::wstring key;
    Vec pos, size;
    bool sliding = false;
    Vec slideTo;
    float spring = 0, springV = 0;  // the hover lift
    Vec Center() const { return pos + size * 0.5f; }
    bool Has(Vec p) const { return p.x >= pos.x && p.y >= pos.y && p.x <= pos.x + size.x && p.y <= pos.y + size.y; }
};

enum class PinKind { Image, Note };
struct Pin {
    PinKind kind = PinKind::Note;
    float x = 0, y = 0, w = 240, h = 180;
    std::wstring file, text;
    bool locked = false;
    ID2D1Bitmap* bitmap = nullptr;
    Vec Pos() const { return {x, y}; }
    Vec Size() const { return {w, h}; }
    bool Has(Vec p) const { return p.x >= x && p.y >= y && p.x <= x + w && p.y <= y + h; }
};

enum class Tool { Pen, Line, Rect, Ellipse, Fill, Eraser };
struct Stroke {
    int kind = 0;  // 0 free, 1 line, 2 rect, 3 ellipse
    uint32_t color = 0xFFFFFF;
    float size = 4;
    int64_t fill = -1;
    std::vector<Vec> pts;
};

struct State {
    bool shown = false, overview = false;
    std::vector<Item> items;
    std::map<std::wstring, Vec> places;  // remembered tile places, by app
    bool placesLoaded = false;
    Camera cam;
    LARGE_INTEGER last{};
    int hover = -1;
    bool hoverClose = false;
    // the mouse
    bool down = false, dragged = false;
    POINT downAt{}, lastAt{};
    Vec dragVel;
    LARGE_INTEGER lastMove{};
    int tileDrag = -1, drillOnClick = -1, snapTarget = -1;
    std::vector<int> cluster;
    POINT tileDownAt{};
    // opening and switching
    bool committing = false, opening = false;
    float commitT = 0, openT = 0;
    int committed = -1;
    // drilling into a stack
    bool drilled = false;
    Vec drillCenter, returnCenter;
    float returnZoom = 1;
    size_t lastCount = 0;
    std::vector<HWND> lastWindows;
    // the pinboard
    std::vector<Pin> pins;
    bool pinsLoaded = false;
    int pinDrag = -1, pinHover = -1, pinResize = -1;
    bool pinHoverClose = false, pinDragged = false;
    POINT pinDownAt{};
    Vec pinOffset;
    int editing = -1;
    // paint
    bool draw = false, drawPan = false, erasing = false;
    Tool tool = Tool::Pen;
    int colorAt = 0, sizeAt = 1;
    std::vector<Stroke> strokes;
    bool strokesLoaded = false;
    Stroke inProgress;
    bool stroking = false;
    int toolHover = -1;
    // drawing
    ID2D1SolidColorBrush* brush = nullptr;
    ID2D1StrokeStyle* round = nullptr;
    IDWriteTextFormat *titleFmt = nullptr, *noteFmt = nullptr, *hintFmt = nullptr, *badgeFmt = nullptr;
};
State g;

////////////////////////////////////////////////////////////////////////////////
// Kept between openings: the places, the pins and the drawing, in the storage folder.

std::wstring Folder() {
    WCHAR path[MAX_PATH] = L"";
    if (!Wh_GetModStoragePath(path, ARRAYSIZE(path))) return L"";
    std::wstring dir = std::wstring(path) + L"\\canvas";
    CreateDirectoryW(path, nullptr);
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir;
}

std::vector<std::wstring> ReadLines(std::wstring const& file) {
    std::vector<std::wstring> out;
    HANDLE f = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return out;
    std::string bytes;
    char buf[8192];
    DWORD n = 0;
    while (ReadFile(f, buf, sizeof(buf), &n, nullptr) && n) bytes.append(buf, n);
    CloseHandle(f);
    int wide = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), nullptr, 0);
    std::wstring text(wide, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), (int)bytes.size(), text.data(), wide);
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find(L'\n', start);
        if (end == std::wstring::npos) end = text.size();
        std::wstring line = text.substr(start, end - start);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (!line.empty()) out.push_back(line);
        start = end + 1;
    }
    return out;
}

void WriteLines(std::wstring const& file, std::vector<std::wstring> const& lines) {
    std::wstring text;
    for (auto const& l : lines) text += l + L"\n";
    int n = WideCharToMultiByte(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0, nullptr, nullptr);
    std::string bytes(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), (int)text.size(), bytes.data(), n, nullptr, nullptr);
    std::wstring temp = file + L".new";
    HANDLE f = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return;
    DWORD wrote = 0;
    WriteFile(f, bytes.data(), (DWORD)bytes.size(), &wrote, nullptr);
    CloseHandle(f);
    MoveFileExW(temp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING);
}

std::vector<std::wstring> Fields(std::wstring const& line) {
    std::vector<std::wstring> out;
    size_t start = 0;
    while (true) {
        size_t tab = line.find(L'\t', start);
        out.push_back(line.substr(start, tab == std::wstring::npos ? std::wstring::npos : tab - start));
        if (tab == std::wstring::npos) break;
        start = tab + 1;
    }
    return out;
}

std::wstring Escape(std::wstring s) {
    std::wstring out;
    for (wchar_t c : s) out += c == L'\n' ? L"\\n" : c == L'\t' ? L"\\t" : c == L'\\' ? L"\\\\" : std::wstring(1, c);
    return out;
}
std::wstring Unescape(std::wstring const& s) {
    std::wstring out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == L'\\' && i + 1 < s.size()) {
            wchar_t c = s[++i];
            out += c == L'n' ? L'\n' : c == L't' ? L'\t' : c;
        } else {
            out += s[i];
        }
    }
    return out;
}

void LoadTilePlaces() {
    if (g.placesLoaded) return;
    g.placesLoaded = true;
    for (auto const& line : ReadLines(Folder() + L"\\places.txt")) {
        auto f = Fields(line);
        if (f.size() >= 3) g.places[f[0]] = {(float)_wtof(f[1].c_str()), (float)_wtof(f[2].c_str())};
    }
}
void SaveTilePlaces() {
    std::vector<std::wstring> lines;
    for (auto const& [key, p] : g.places) {
        WCHAR buf[64];
        swprintf_s(buf, L"\t%.1f\t%.1f", p.x, p.y);
        lines.push_back(key + buf);
    }
    WriteLines(Folder() + L"\\places.txt", lines);
}

void FreePinBitmaps() {
    for (auto& p : g.pins) ViewRelease(p.bitmap);
}

void LoadPins() {
    if (g.pinsLoaded) return;
    g.pinsLoaded = true;
    std::wstring dir = Folder();
    for (auto const& line : ReadLines(dir + L"\\pins.txt")) {
        auto f = Fields(line);
        if (f.size() < 8) continue;
        Pin p;
        p.kind = f[0] == L"image" ? PinKind::Image : PinKind::Note;
        p.x = (float)_wtof(f[1].c_str()), p.y = (float)_wtof(f[2].c_str());
        p.w = (float)_wtof(f[3].c_str()), p.h = (float)_wtof(f[4].c_str());
        p.locked = f[5] == L"1";
        p.file = f[6];
        p.text = Unescape(f[7]);
        if (p.kind == PinKind::Image && GetFileAttributesW((dir + L"\\" + p.file).c_str()) == INVALID_FILE_ATTRIBUTES) continue;
        g.pins.push_back(std::move(p));
    }
}
void SavePins() {
    std::vector<std::wstring> lines;
    for (auto const& p : g.pins) {
        WCHAR buf[160];
        swprintf_s(buf, L"\t%.1f\t%.1f\t%.1f\t%.1f\t%d\t", p.x, p.y, p.w, p.h, p.locked ? 1 : 0);
        lines.push_back((p.kind == PinKind::Image ? L"image" : L"note") + std::wstring(buf) + p.file + L"\t" + Escape(p.text));
    }
    WriteLines(Folder() + L"\\pins.txt", lines);
}

void LoadStrokes() {
    if (g.strokesLoaded) return;
    g.strokesLoaded = true;
    for (auto const& line : ReadLines(Folder() + L"\\drawing.txt")) {
        auto f = Fields(line);
        if (f.size() < 5) continue;
        Stroke s;
        s.kind = _wtoi(f[0].c_str());
        s.color = (uint32_t)wcstoul(f[1].c_str(), nullptr, 16);
        s.size = (float)_wtof(f[2].c_str());
        s.fill = _wtoi64(f[3].c_str());
        for (size_t i = 4; i + 1 < f.size(); i += 2) s.pts.push_back({(float)_wtof(f[i].c_str()), (float)_wtof(f[i + 1].c_str())});
        if (!s.pts.empty()) g.strokes.push_back(std::move(s));
    }
}
void SaveStrokes() {
    std::vector<std::wstring> lines;
    for (auto const& s : g.strokes) {
        WCHAR buf[96];
        swprintf_s(buf, L"%d\t%06x\t%.1f\t%lld", s.kind, s.color & 0xFFFFFF, s.size, (long long)s.fill);
        std::wstring line = buf;
        for (auto const& p : s.pts) {
            swprintf_s(buf, L"\t%.1f\t%.1f", p.x, p.y);
            line += buf;
        }
        lines.push_back(line);
    }
    WriteLines(Folder() + L"\\drawing.txt", lines);
}

// A picture pinned: kept as a 32-bit top-down BMP beside the list.
bool SaveBmp(std::wstring const& file, int w, int h, uint32_t const* px) {
    BITMAPFILEHEADER fh{};
    BITMAPINFOHEADER ih{sizeof(ih), w, -h, 1, 32, BI_RGB};
    DWORD bytes = (DWORD)w * h * 4;
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + bytes;
    HANDLE f = CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    DWORD n = 0;
    bool ok = WriteFile(f, &fh, sizeof(fh), &n, nullptr) && WriteFile(f, &ih, sizeof(ih), &n, nullptr) &&
              WriteFile(f, px, bytes, &n, nullptr);
    CloseHandle(f);
    return ok;
}

bool LoadBmp(std::wstring const& file, int* w, int* h, std::vector<uint32_t>* px) {
    HANDLE f = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE) return false;
    BITMAPFILEHEADER fh{};
    BITMAPINFOHEADER ih{};
    DWORD n = 0;
    bool ok = ReadFile(f, &fh, sizeof(fh), &n, nullptr) && n == sizeof(fh) && ReadFile(f, &ih, sizeof(ih), &n, nullptr) &&
              n == sizeof(ih) && fh.bfType == 0x4D42 && ih.biBitCount == 32 && ih.biWidth > 0 && ih.biHeight < 0 &&
              ih.biWidth <= 8192 && -ih.biHeight <= 8192;
    if (ok) {
        *w = ih.biWidth, *h = -ih.biHeight;
        px->resize((size_t)*w * *h);
        SetFilePointer(f, fh.bfOffBits, nullptr, FILE_BEGIN);
        ok = ReadFile(f, px->data(), (DWORD)(px->size() * 4), &n, nullptr) && n == px->size() * 4;
    }
    CloseHandle(f);
    return ok;
}

ID2D1Bitmap* PinBitmap(Pin& p) {
    if (p.bitmap || !g_view.dc || p.kind != PinKind::Image) return p.bitmap;
    int w = 0, h = 0;
    std::vector<uint32_t> px;
    if (!LoadBmp(Folder() + L"\\" + p.file, &w, &h, &px)) return nullptr;
    for (auto& c : px) c |= 0xFF000000;
    D2D1_BITMAP_PROPERTIES1 bp{};
    bp.pixelFormat = {DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED};
    bp.dpiX = bp.dpiY = 96;
    ID2D1Bitmap1* b = nullptr;
    if (SUCCEEDED(g_view.dc->CreateBitmap(D2D1::SizeU(w, h), px.data(), w * 4, &bp, &b))) p.bitmap = b;
    return p.bitmap;
}

////////////////////////////////////////////////////////////////////////////////
// The tiles: one per entry, sized as its window is, placed where they were last time.

bool Overlaps(Vec ap, Vec as, Vec bp, Vec bs, float gap) {
    return ap.x < bp.x + bs.x + gap && ap.x + as.x + gap > bp.x && ap.y < bp.y + bs.y + gap && ap.y + as.y + gap > bp.y;
}

std::vector<std::pair<Vec, Vec>> Obstacles(int skipItem, int skipPin) {
    std::vector<std::pair<Vec, Vec>> out;
    for (int i = 0; i < (int)g.items.size(); i++)
        if (i != skipItem) out.push_back({g.items[i].pos, g.items[i].size});
    for (int i = 0; i < (int)g.pins.size(); i++)
        if (i != skipPin) out.push_back({g.pins[i].Pos(), g.pins[i].Size()});
    return out;
}

bool HitsAny(Vec p, Vec s, std::vector<std::pair<Vec, Vec>> const& obstacles, float gap) {
    for (auto const& [op, os] : obstacles)
        if (Overlaps(p, s, op, os, gap)) return true;
    return false;
}

// The nearest free place to pos (PrinceWM's Collision.Resolve).
Vec Resolve(Vec pos, Vec size, std::vector<std::pair<Vec, Vec>> const& obstacles, float gap) {
    if (!HitsAny(pos, size, obstacles, gap)) return pos;
    float step = std::max(24.f, std::min(size.x, size.y) * 0.35f);
    for (int ring = 1; ring <= 80; ring++) {
        float rad = ring * step, best = FLT_MAX;
        Vec found;
        for (int a = 0; a < 24; a++) {
            float ang = a / 24.f * 6.2831853f;
            Vec c = pos + Vec{std::cos(ang), std::sin(ang)} * rad;
            if (HitsAny(c, size, obstacles, gap)) continue;
            float d = Length2(c - pos);
            if (d < best) best = d, found = c;
        }
        if (best < FLT_MAX) return found;
    }
    return pos;
}

// Rows about as wide as the whole is tall, in the order given (PrinceWM's NativeLayout).
void RowLayout(std::vector<Item*> const& items) {
    float area = 0;
    for (auto* it : items) area += (it->size.x + kGap) * (it->size.y + kGap);
    float rowWidth = std::sqrt(area) * 1.25f, x = 0, y = 0, rowH = 0;
    for (auto* it : items) {
        if (x > 0 && x + it->size.x > rowWidth) x = 0, y += rowH + kGap, rowH = 0;
        it->pos = {x, y};
        x += it->size.x + kGap;
        rowH = std::max(rowH, it->size.y);
    }
}

// Each tile at its remembered place when that's free, else the nearest free one (PrinceWM's LayoutManager).
void Arrange() {
    LoadTilePlaces();
    std::vector<Item*> placed, toPlace;
    for (auto& it : g.items) {
        auto at = g.places.find(it.key);
        bool free = at != g.places.end();
        if (free)
            for (auto* p : placed)
                if (Overlaps(at->second, it.size, p->pos, p->size, 4)) free = false;
        if (free) {
            it.pos = at->second;
            placed.push_back(&it);
        } else {
            toPlace.push_back(&it);
        }
    }
    if (placed.empty()) {
        std::vector<Item*> all;
        for (auto& it : g.items) all.push_back(&it);
        RowLayout(all);
    } else {
        for (auto* it : toPlace) {
            std::vector<std::pair<Vec, Vec>> obstacles;
            for (auto* p : placed) obstacles.push_back({p->pos, p->size});
            for (auto const& pin : g.pins) obstacles.push_back({pin.Pos(), pin.Size()});
            Vec want = placed.back()->pos + Vec{placed.back()->size.x + kGap, 0};
            if (auto at = g.places.find(it->key); at != g.places.end()) want = at->second;
            it->pos = Resolve(want, it->size, obstacles, kGap);
            placed.push_back(it);
        }
    }
    for (auto& it : g.items) g.places[it.key] = it.pos;
}

// An opened stack: its windows in a grid round where the stack was.
void ArrangeGrid(Vec center) {
    int n = (int)g.items.size();
    if (!n) return;
    float cw = 0, ch = 0;
    for (auto& it : g.items) cw = std::max(cw, it.size.x), ch = std::max(ch, it.size.y);
    int cols = (int)std::ceil(std::sqrt((float)n)), rows = (n + cols - 1) / cols;
    float y0 = center.y - (rows * ch + (rows - 1) * kGap) / 2;
    int at = 0;
    for (int r = 0; r < rows && at < n; r++) {
        int inRow = std::min(cols, n - at);
        float x0 = center.x - (inRow * cw + (inRow - 1) * kGap) / 2;
        for (int c = 0; c < inRow; c++) {
            Item& it = g.items[at++];
            Vec cell{x0 + c * (cw + kGap) + cw / 2, y0 + r * (ch + kGap) + ch / 2};
            it.pos = cell - it.size * 0.5f;
        }
    }
}

// The window's own size (its normal size when minimised).
Vec WindowSize(HWND h) {
    RECT r{};
    if (IsIconic(h)) {
        WINDOWPLACEMENT wp{sizeof(wp)};
        if (GetWindowPlacement(h, &wp)) r = wp.rcNormalPosition;
    } else if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) {
        GetWindowRect(h, &r);
    }
    float w = (float)(r.right - r.left), hh = (float)(r.bottom - r.top);
    if (w < 150 || hh < 120 || w > 16000 || hh > 16000) w = 960, hh = 600;
    return {w, hh};
}

void BuildItems() {
    std::vector<Item> next;
    for (int i = 0; i < (int)g_entries.size(); i++) {
        auto const& e = g_entries[i];
        if (e.windows.empty()) continue;
        Item it;
        it.entry = i;
        it.key = Lower(e.key.empty() ? ProgramOf(e.windows[0]) : e.key);
        it.size = WindowSize(e.windows[0]);
        for (auto const& old : g.items)  // (the hover lift carries on)
            if (old.key == it.key && old.entry < (int)g_entries.size()) it.spring = old.spring;
        next.push_back(std::move(it));
    }
    g.items = std::move(next);
    if (g.drilled) ArrangeGrid(g.drillCenter);
    else Arrange();
    g.lastCount = g_entries.size();
    g.lastWindows.clear();
    for (auto const& e : g_entries) g.lastWindows.push_back(e.windows.empty() ? nullptr : e.windows[0]);
}

int ItemOf(int entry) {
    for (int i = 0; i < (int)g.items.size(); i++)
        if (g.items[i].entry == entry) return i;
    return -1;
}

void Remember(Item const& it) {
    if (!g.drilled) g.places[it.key] = it.sliding ? it.slideTo : it.pos;
}

////////////////////////////////////////////////////////////////////////////////
// Camera helpers (PrinceWM's FitAll, FocusZoom, FocusFill and the frame-perfect start).

void FitAll(bool animate) {
    if (g.items.empty()) {
        if (animate) g.cam.TweenTo({}, 1, 0.4f);
        else g.cam.SnapTo({}, 1);
        return;
    }
    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    for (auto const& it : g.items) {
        minX = std::min(minX, it.pos.x), minY = std::min(minY, it.pos.y);
        maxX = std::max(maxX, it.pos.x + it.size.x), maxY = std::max(maxY, it.pos.y + it.size.y);
    }
    Vec c{(minX + maxX) / 2, (minY + maxY) / 2};
    float pad = 1.24f;
    float z = std::min(g.cam.view.x / (std::max(1.f, maxX - minX) * pad), g.cam.view.y / (std::max(1.f, maxY - minY) * pad));
    if (animate) g.cam.TweenTo(c, z, 0.38f);
    else g.cam.SnapTo(c, z);
}

float FocusZoom(Item const& it, float pad = 1.6f) {
    return std::clamp(std::min(g.cam.view.x / (it.size.x * pad), g.cam.view.y / (it.size.y * pad)), kMinZoom, kMaxZoom);
}

// The camera that shows the tile exactly where its window is on the screen, at its size.
bool FramePerfect(Item const& it, Vec* center, float* zoom) {
    HWND h = g_entries[it.entry].windows[0];
    RECT r{};
    if (IsIconic(h)) {
        WINDOWPLACEMENT wp{sizeof(wp)};
        if (!GetWindowPlacement(h, &wp)) return false;
        r = wp.rcNormalPosition;
    } else if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r))) && !GetWindowRect(h, &r)) {
        return false;
    }
    float w = (float)(r.right - r.left), hh = (float)(r.bottom - r.top);
    if (w <= 0 || hh <= 0 || r.left <= -30000) return false;
    Vec onScreen{(r.left - g_view.mon.left) + w / 2, (r.top - g_view.mon.top) + hh / 2};
    *zoom = w / std::max(1.f, it.size.x);
    *center = it.Center() - (onScreen - g.cam.view * 0.5f) * (1 / *zoom);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing.

float Dpi() { return g_dpi / 96.f; }

D2D1_COLOR_F Color(uint32_t rgb, float a = 1) {
    return D2D1::ColorF(((rgb >> 16) & 255) / 255.f, ((rgb >> 8) & 255) / 255.f, (rgb & 255) / 255.f, a);
}

ID2D1SolidColorBrush* Brush(uint32_t rgb, float a = 1) {
    if (!g.brush) g_view.dc->CreateSolidColorBrush(Color(rgb, a), &g.brush);
    else g.brush->SetColor(Color(rgb, a));
    return g.brush;
}

void MakeFormats() {
    if (!g_view.dwrite || g.titleFmt) return;
    std::wstring font = g_settings.font.empty() ? L"Segoe UI" : g_settings.font;
    float s = Dpi(), base = std::max(11.f, g_settings.fontSize * 1.6f);
    auto make = [&](float size, DWRITE_FONT_WEIGHT weight, IDWriteTextFormat** out) {
        g_view.dwrite->CreateTextFormat(font.c_str(), nullptr, weight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                        size * s, L"", out);
    };
    make(base, g_settings.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL, &g.titleFmt);
    make(base, DWRITE_FONT_WEIGHT_NORMAL, &g.noteFmt);
    make(base * 0.9f, DWRITE_FONT_WEIGHT_NORMAL, &g.hintFmt);
    make(base * 0.9f, DWRITE_FONT_WEIGHT_BOLD, &g.badgeFmt);
    if (g.titleFmt) g.titleFmt->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    if (g.hintFmt) g.hintFmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    if (g.badgeFmt) {
        g.badgeFmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        g.badgeFmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    if (!g.round && g_view.d2d) {
        D2D1_STROKE_STYLE_PROPERTIES p{D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_LINE_JOIN_ROUND};
        g_view.d2d->CreateStrokeStyle(p, nullptr, 0, &g.round);
    }
}

void FreeFormats() {
    ViewRelease(g.titleFmt);
    ViewRelease(g.noteFmt);
    ViewRelease(g.hintFmt);
    ViewRelease(g.badgeFmt);
    ViewRelease(g.brush);
    ViewRelease(g.round);
}

D2D1_MATRIX_3X2_F WorldMatrix() {
    return D2D1::Matrix3x2F::Translation(-g.cam.center.x, -g.cam.center.y) * D2D1::Matrix3x2F::Scale(g.cam.zoom, g.cam.zoom) *
           D2D1::Matrix3x2F::Translation(g.cam.view.x / 2, g.cam.view.y / 2);
}

// A tile on the screen, with its hover lift.
D2D1_RECT_F TileRect(Item const& it) {
    float lift = 1 + kHoverLift * it.spring;
    Vec grow = it.size * ((lift - 1) / 2);
    Vec tl = g.cam.ToScreen(it.pos - grow), br = g.cam.ToScreen(it.pos + it.size + grow);
    return {tl.x, tl.y, br.x, br.y};
}

D2D1_RECT_F CloseRect(D2D1_RECT_F r) {
    float s = 22 * Dpi();
    return {r.right - s, r.top, r.right, r.top + s};
}

bool In(D2D1_RECT_F r, POINT p) { return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom; }

// The paint toolbar: pen, line, box, ellipse, fill, eraser, colour, size.
constexpr int kTools = 8;
D2D1_RECT_F ToolRect(int i) {
    float s = Dpi(), size = 40 * s, gap = 8 * s, total = kTools * size + (kTools - 1) * gap;
    float x = (g.cam.view.x - total) / 2 + i * (size + gap), y = g.cam.view.y - size - 56 * s;
    return {x, y, x + size, y + size};
}

uint32_t PaintColor(int at) {
    auto const& l = g_settings.look;
    uint32_t colors[] = {l.selectedText, l.highlight, l.highlightBorder, l.count, l.text, 0x15FFA2, 0xFF6F77, 0xFFFFFF};
    return colors[((at % 8) + 8) % 8];
}

void StepSprings(float dt) {
    for (int i = 0; i < (int)g.items.size(); i++) {
        auto& it = g.items[i];
        float target = i == g.hover && !g.committing ? 1.f : 0.f;
        it.springV += ((target - it.spring) * 320 - it.springV * 17) * dt;
        it.spring += it.springV * dt;
        if (it.sliding) {
            float k = 1 - std::exp(-dt * 13);
            it.pos = Lerp(it.pos, it.slideTo, k);
            if (Length2(it.slideTo - it.pos) < 0.25f) it.pos = it.slideTo, it.sliding = false;
        }
    }
}

void D3DState(float W, float H) {
    D3D11_VIEWPORT vp{0, 0, W, H, 0, 1};
    g_view.ctx->RSSetViewports(1, &vp);
    g_view.ctx->RSSetState(g_view.raster);
    g_view.ctx->OMSetRenderTargets(1, &g_view.rtv, nullptr);
    g_view.ctx->OMSetBlendState(g_view.blend, nullptr, 0xFFFFFFFF);
    g_view.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    g_view.ctx->IASetInputLayout(nullptr);
    g_view.ctx->VSSetShader(g_view.vs, nullptr, 0);
    g_view.ctx->PSSetShader(g_view.ps, nullptr, 0);
    g_view.ctx->VSSetConstantBuffers(0, 1, &g_view.cb);
    g_view.ctx->PSSetConstantBuffers(0, 1, &g_view.cb);
    g_view.ctx->PSSetSamplers(0, 1, &g_view.sampler);
}

void DrawStroke(Stroke const& s, float opacity) {
    if (s.pts.empty()) return;
    float w = std::max(0.5f, s.size);
    auto* b = Brush(s.color, opacity);
    if (s.kind == 0 || s.kind == 1) {
        if (s.pts.size() == 1) {
            g_view.dc->FillEllipse(D2D1::Ellipse({s.pts[0].x, s.pts[0].y}, w / 2, w / 2), b);
            return;
        }
        for (size_t i = 1; i < s.pts.size(); i++)
            g_view.dc->DrawLine({s.pts[i - 1].x, s.pts[i - 1].y}, {s.pts[i].x, s.pts[i].y}, b, w, g.round);
        return;
    }
    if (s.pts.size() < 2) return;
    Vec a = s.pts[0], c = s.pts.back();
    D2D1_RECT_F r{std::min(a.x, c.x), std::min(a.y, c.y), std::max(a.x, c.x), std::max(a.y, c.y)};
    D2D1_ELLIPSE e = D2D1::Ellipse({(r.left + r.right) / 2, (r.top + r.bottom) / 2}, (r.right - r.left) / 2, (r.bottom - r.top) / 2);
    if (s.fill >= 0) {
        auto* f = Brush((uint32_t)s.fill, opacity);
        if (s.kind == 2) g_view.dc->FillRectangle(r, f);
        else g_view.dc->FillEllipse(e, f);
    }
    b = Brush(s.color, opacity);
    if (s.kind == 2) g_view.dc->DrawRectangle(r, b, w, g.round);
    else g_view.dc->DrawEllipse(e, b, w, g.round);
}

void Text(std::wstring const& text, IDWriteTextFormat* fmt, D2D1_RECT_F r, uint32_t color, float a) {
    if (!fmt || text.empty()) return;
    g_view.dc->DrawText(text.c_str(), (UINT32)text.size(), fmt, r, Brush(color, a), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

void Render() {
    float W = (float)g_view.chainW, H = (float)g_view.chainH, D = H * 1.15f;
    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);
    float dt = g.last.QuadPart ? std::clamp((float)((now.QuadPart - g.last.QuadPart) / (double)freq.QuadPart), 0.f, 0.05f) : 0.f;
    g.last = now;
    g.cam.view = {W, H};
    for (auto& t : g_view.tiles) ViewTakeFrame(*t);

    bool finish = false;
    if (g.committing) {
        g.commitT += dt;
        if (g.commitT >= kCommitSeconds) finish = true;
    }
    float open = 1;
    if (g.opening) {
        g.openT += dt;
        float u = std::clamp(g.openT / kOpenSeconds, 0.f, 1.f);
        open = 1 - std::pow(1 - u, 3.f);
        if (u >= 1) g.opening = false;
    }
    g.cam.Step(dt);
    StepSprings(dt);
    float commit = g.committing ? std::clamp(g.commitT / kCommitSeconds, 0.f, 1.f) : 0.f;
    float others = (1 - commit) * open;
    auto const& look = g_settings.look;
    float s = Dpi();
    MakeFormats();

    // The ground.
    D3DState(W, H);
    float ground[4] = {((look.ground >> 16) & 255) / 255.f, ((look.ground >> 8) & 255) / 255.f, (look.ground & 255) / 255.f, 1};
    g_view.ctx->ClearRenderTargetView(g_view.rtv, ground);

    // Behind the tiles: the dot grid and the drawing.
    if (g_view.dc && g_view.target) {
        g_view.dc->BeginDraw();
        g_view.dc->SetTransform(D2D1::Matrix3x2F::Identity());
        float spacing = 80;
        while (spacing * g.cam.zoom < 26) spacing *= 2;
        while (spacing * g.cam.zoom > 90) spacing /= 2;
        Vec tl = g.cam.ToWorld({0, 0}), br = g.cam.ToWorld({W, H});
        float radius = 1.5f * s;
        auto* dot = Brush(look.border, 0.55f * (1 - commit * 0.9f) * open);
        for (float wx = std::floor(tl.x / spacing) * spacing; wx <= br.x; wx += spacing)
            for (float wy = std::floor(tl.y / spacing) * spacing; wy <= br.y; wy += spacing) {
                Vec p = g.cam.ToScreen({wx, wy});
                g_view.dc->FillRectangle({p.x - radius, p.y - radius, p.x + radius, p.y + radius}, dot);  // (square: Tourne's pixels)
            }
        if (!g.drilled && (!g.strokes.empty() || g.stroking)) {
            g_view.dc->SetTransform(WorldMatrix());
            for (auto const& st : g.strokes) DrawStroke(st, others);
            if (g.stroking) DrawStroke(g.inProgress, others);
            g_view.dc->SetTransform(D2D1::Matrix3x2F::Identity());
        }
        g_view.dc->EndDraw();
    }

    // The tiles: the chosen one last (on top), each a shadow, a glow when lit, and its picture.
    D3DState(W, H);
    Mat proj = Project(W, H, D);
    float radius = std::max(0, g_settings.radius) * s * g.cam.zoom;
    int sel = ItemOf(g_sel), lit = g.hover >= 0 ? g.hover : sel;
    std::vector<int> order;
    for (int i = 0; i < (int)g.items.size(); i++)
        if (i != sel) order.push_back(i);
    if (sel >= 0) order.push_back(sel);
    for (int i : order) {
        Item const& it = g.items[i];
        if (it.entry >= (int)g_view.tiles.size()) continue;
        ViewTile& t = *g_view.tiles[it.entry];
        if (!t.srv) continue;
        D2D1_RECT_F r = TileRect(it);
        float sw = r.right - r.left, sh = r.bottom - r.top;
        if (r.right < 0 || r.bottom < 0 || r.left > W || r.top > H || sw < 1 || sh < 1) continue;
        float alpha = i == sel ? open : others;
        float cornerR = i == sel ? radius * (1 - commit) : radius;
        Mat place = Move((r.left + r.right) / 2 - W / 2, H / 2 - (r.top + r.bottom) / 2, D);
        float spread = 22 * s;
        float stint[4] = {0, 0, 0, 0.45f * alpha}, sextra[4] = {spread, 0, 0, 0};
        Mat shadow = Scale(sw + spread * 2, sh + spread * 2) * Move(0, -spread * 0.35f, 0) * place * proj;
        ViewDrawQuad(shadow, sw + spread * 2, sh + spread * 2, cornerR, 1, stint, sextra, t.srv, 1, 1);
        if (i == lit && !g.committing) {
            float glow = 9 * s;
            uint32_t c = look.highlightBorder;
            float gt[4] = {((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, 0.85f * alpha};
            float ge[4] = {glow, 0, 0, 0};
            Mat ring = Scale(sw + glow * 2, sh + glow * 2) * place * proj;
            ViewDrawQuad(ring, sw + glow * 2, sh + glow * 2, cornerR + glow, 1, gt, ge, t.srv, 1, 1);
        }
        float u1 = (float)t.w / std::max(1, t.texW), v1 = (float)t.h / std::max(1, t.texH);
        float tint[4] = {1, 1, 1, alpha}, extra[4] = {0, 0, 0, 0};
        ViewDrawQuad(Scale(sw, sh) * place * proj, sw, sh, cornerR, 0, tint, extra, t.srv, u1, v1);
    }

    // Over the tiles: their titles and badges, the drop target, the pins, the toolbar and the hints.
    if (g_view.dc && g_view.target) {
        g_view.dc->BeginDraw();
        g_view.dc->SetTransform(D2D1::Matrix3x2F::Identity());
        float chrome = 1 - commit;
        for (int i = 0; i < (int)g.items.size(); i++) {
            Item const& it = g.items[i];
            D2D1_RECT_F r = TileRect(it);
            float sw = r.right - r.left, sh = r.bottom - r.top;
            if (sw < 110 * s || sh < 72 * s || chrome <= 0.01f) continue;
            auto const& e = g_entries[it.entry];
            bool chosen = i == sel;
            // The title above the tile.
            std::wstring label = e.label.empty() ? Title(e.windows[0]) : e.label;
            Text(label, g.titleFmt, {r.left, r.top - 22 * s, r.right, r.top - 2 * s}, chosen ? look.selectedText : look.text,
                     (chosen ? 1.f : 0.75f) * chrome * open);
            // The stack's count, top left.
            if (e.windows.size() > 1 && !g.drilled) {
                std::wstring n = std::to_wstring(e.windows.size());
                float bw = std::max(24 * s, (14 + n.size() * 9) * s), bh = 24 * s;
                D2D1_RECT_F b{r.left + 8 * s, r.top + 8 * s, r.left + 8 * s + bw, r.top + 8 * s + bh};
                g_view.dc->FillRectangle(b, Brush(look.ground, 0.9f * chrome));
                g_view.dc->DrawRectangle(b, Brush(look.highlightBorder, chrome), 1.4f * s);
                Text(n, g.badgeFmt, b, look.selectedText, chrome);
            }
            // Close, on the one under the pointer.
            if (i == g.hover) {
                D2D1_RECT_F c = CloseRect(r);
                g_view.dc->FillRectangle(c, Brush(g.hoverClose ? 0xE57474 : look.ground, (g.hoverClose ? 1.f : 0.7f) * chrome));
                float cx = (c.left + c.right) / 2, cy = (c.top + c.bottom) / 2, d = 4 * s;
                auto* x = Brush(look.selectedText, chrome);
                g_view.dc->DrawLine({cx - d, cy - d}, {cx + d, cy + d}, x, 1.7f * s, g.round);
                g_view.dc->DrawLine({cx - d, cy + d}, {cx + d, cy - d}, x, 1.7f * s, g.round);
            }
        }
        // Dropping here puts both side by side: the half each will take.
        if (g.snapTarget >= 0 && g.snapTarget < (int)g.items.size() && g.tileDrag >= 0) {
            D2D1_RECT_F r = TileRect(g.items[g.snapTarget]);
            bool left = g.items[g.tileDrag].Center().x <= g.items[g.snapTarget].Center().x;
            float mid = (r.left + r.right) / 2;
            D2D1_RECT_F half = left ? D2D1_RECT_F{r.left, r.top, mid, r.bottom} : D2D1_RECT_F{mid, r.top, r.right, r.bottom};
            g_view.dc->FillRectangle(half, Brush(look.highlight, 0.3f));
            g_view.dc->DrawRectangle(r, Brush(look.highlightBorder), 2.5f * s);
        }
        // The pinboard.
        if (!g.drilled && !g.pins.empty()) {
            g_view.dc->SetTransform(WorldMatrix());
            float z = std::max(g.cam.zoom, 0.001f);
            for (int i = 0; i < (int)g.pins.size(); i++) {
                Pin& p = g.pins[i];
                D2D1_RECT_F r{p.x, p.y, p.x + p.w, p.y + p.h};
                if (p.kind == PinKind::Image) {
                    if (auto* b = PinBitmap(p)) g_view.dc->DrawBitmap(b, r, others, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
                    g_view.dc->DrawRectangle(r, Brush(look.border, 0.8f * others), 1.5f / z);
                } else {
                    g_view.dc->FillRectangle(r, Brush(look.iconBack, 0.95f * others));
                    g_view.dc->DrawRectangle(r, Brush(i == g.editing ? look.highlightBorder : look.border, others), 1.5f / z);
                    std::wstring text = p.text;
                    if (i == g.editing && (GetTickCount() / 530) % 2 == 0) text += L"|";
                    D2D1_RECT_F in{r.left + 12, r.top + 12, r.right - 12, r.bottom - 12};
                    g_view.dc->SetTransform(D2D1::Matrix3x2F::Identity());
                    Vec a = g.cam.ToScreen({in.left, in.top}), b = g.cam.ToScreen({in.right, in.bottom});
                    if (g.noteFmt && b.x - a.x > 20 && b.y - a.y > 12)
                        g_view.dc->DrawText(text.c_str(), (UINT32)text.size(), g.noteFmt, {a.x, a.y, b.x, b.y},
                                            Brush(look.selectedText, others), D2D1_DRAW_TEXT_OPTIONS_CLIP);
                    g_view.dc->SetTransform(WorldMatrix());
                }
                if (p.locked) {
                    float m = 8 / z;
                    g_view.dc->FillRectangle({r.right - m * 2, r.bottom - m * 2, r.right - m, r.bottom - m}, Brush(look.highlightBorder, others));
                }
            }
            g_view.dc->SetTransform(D2D1::Matrix3x2F::Identity());
            if (g.pinHover >= 0 && g.pinHover < (int)g.pins.size()) {
                Pin const& p = g.pins[g.pinHover];
                Vec a = g.cam.ToScreen(p.Pos()), b = g.cam.ToScreen(p.Pos() + p.Size());
                D2D1_RECT_F c = CloseRect({a.x, a.y, b.x, b.y});
                g_view.dc->FillRectangle(c, Brush(g.pinHoverClose ? 0xE57474 : look.ground, g.pinHoverClose ? 1.f : 0.7f));
                float cx = (c.left + c.right) / 2, cy = (c.top + c.bottom) / 2, d = 4 * s;
                g_view.dc->DrawLine({cx - d, cy - d}, {cx + d, cy + d}, Brush(look.selectedText), 1.7f * s, g.round);
                g_view.dc->DrawLine({cx - d, cy + d}, {cx + d, cy - d}, Brush(look.selectedText), 1.7f * s, g.round);
                if (!p.locked) {  // the corner to size it by
                    float hs = 12 * s;
                    g_view.dc->FillRectangle({b.x - hs, b.y - hs, b.x, b.y}, Brush(look.highlightBorder, 0.8f));
                }
            }
        }
        // Paint's toolbar.
        if (g.draw) {
            PCWSTR names[kTools] = {L"PEN", L"LINE", L"BOX", L"OVAL", L"FILL", L"ERASE", L"", L""};
            for (int i = 0; i < kTools; i++) {
                D2D1_RECT_F r = ToolRect(i);
                bool on = i < 6 && (int)g.tool == i;
                g_view.dc->FillRectangle(r, Brush(on ? look.highlight : look.ground, on ? 0.45f : 0.92f));
                g_view.dc->DrawRectangle(r, Brush(i == g.toolHover || on ? look.highlightBorder : look.border), 1.5f * s);
                if (i == 6) {
                    float m = 10 * s;
                    g_view.dc->FillRectangle({r.left + m, r.top + m, r.right - m, r.bottom - m}, Brush(PaintColor(g.colorAt)));
                } else if (i == 7) {
                    float d = std::min(kStrokeSizes[g.sizeAt], 28.f) * s / 2;
                    float cx = (r.left + r.right) / 2, cy = (r.top + r.bottom) / 2;
                    g_view.dc->FillRectangle({cx - d / 2 - 1, cy - d / 2 - 1, cx + d / 2 + 1, cy + d / 2 + 1}, Brush(look.selectedText));
                } else if (g.badgeFmt) {
                    Text(names[i], g.badgeFmt, r, look.selectedText, 1);
                }
            }
        }
        // What the keys do, at the foot.
        if (g.hintFmt && chrome > 0.01f) {
            std::wstring hint = g.draw ? L"PAINT   left: draw   right: pan   wheel: zoom   D or Esc: done"
                              : g.drilled ? L"Esc: back to the canvas   click: switch   middle-drag: move"
                                          : L"drag: pan   wheel: zoom   middle-drag: move   middle-click: open a stack   drop on a tile: side by side"
                                            L"   W fit   R arrange   N note   Ctrl+V paste   D paint";
            Text(hint, g.hintFmt, {0, H - 34 * s, W, H - 10 * s}, look.text, 0.55f * chrome * open);
        }
        g_view.dc->EndDraw();
    }
    g_view.chain->Present(1, 0);
    if (finish) {
        int idx = g.committed;
        g.committing = false;
        if (g_wnd) Commit(g_wnd, idx);  // (Hide closes the view)
    }
}

////////////////////////////////////////////////////////////////////////////////
// What it does.

void BeginCommit(int item) {
    if (item < 0 || item >= (int)g.items.size() || g.committing) return;
    g_sel = g.items[item].entry;
    g.committed = g_sel;
    g.committing = true;
    g.commitT = 0;
    Vec c;
    float z;
    if (FramePerfect(g.items[item], &c, &z)) g.cam.TweenTo(c, z, kCommitSeconds, true);
    else {
        Item const& it = g.items[item];
        g.cam.TweenTo(it.Center(), std::max(g.cam.view.x / it.size.x, g.cam.view.y / it.size.y), kCommitSeconds, true);
    }
}

int HitTest(POINT p) {
    Vec w = g.cam.ToWorld({(float)p.x, (float)p.y});
    for (int i = (int)g.items.size() - 1; i >= 0; i--)
        if (g.items[i].Has(w)) return i;
    return -1;
}

void UpdateHover(POINT p) {
    g.hover = -1, g.hoverClose = false;
    for (int i = (int)g.items.size() - 1; i >= 0; i--) {
        D2D1_RECT_F r = TileRect(g.items[i]);
        if (r.right - r.left < 60 || r.bottom - r.top < 40) continue;
        if (In(CloseRect(r), p)) {
            g.hover = i, g.hoverClose = true;
            return;
        }
        if (In(r, p)) {
            g.hover = i;
            return;
        }
    }
}

void UpdatePinHover(POINT p) {
    g.pinHover = -1, g.pinHoverClose = false;
    if (g.drilled) return;
    for (int i = (int)g.pins.size() - 1; i >= 0; i--) {
        Vec a = g.cam.ToScreen(g.pins[i].Pos()), b = g.cam.ToScreen(g.pins[i].Pos() + g.pins[i].Size());
        D2D1_RECT_F r{a.x, a.y, b.x, b.y};
        if (b.x - a.x < 30 || b.y - a.y < 24) continue;
        if (In(CloseRect(r), p)) {
            g.pinHover = i, g.pinHoverClose = true;
            return;
        }
        if (In(r, p)) {
            g.pinHover = i;
            return;
        }
    }
}

bool OnResizeCorner(int pin, POINT p) {
    if (pin < 0 || g.pins[pin].locked) return false;
    Vec b = g.cam.ToScreen(g.pins[pin].Pos() + g.pins[pin].Size());
    float hs = 14 * Dpi();
    return p.x >= b.x - hs && p.y >= b.y - hs && p.x <= b.x && p.y <= b.y;
}

// The tile dragged lies mostly over another: that one's the drop target (PrinceWM's drag to tile).
void UpdateSnapTarget() {
    g.snapTarget = -1;
    if (g.tileDrag < 0 || !g.cluster.empty() || g.drilled) return;
    Item const& a = g.items[g.tileDrag];
    float aArea = std::max(1.f, a.size.x * a.size.y), best = 0;
    for (int i = 0; i < (int)g.items.size(); i++) {
        if (i == g.tileDrag) continue;
        Item const& b = g.items[i];
        float ox = std::max(0.f, std::min(a.pos.x + a.size.x, b.pos.x + b.size.x) - std::max(a.pos.x, b.pos.x));
        float oy = std::max(0.f, std::min(a.pos.y + a.size.y, b.pos.y + b.size.y) - std::max(a.pos.y, b.pos.y));
        float overlap = ox * oy, frac = overlap / std::min(aArea, std::max(1.f, b.size.x * b.size.y));
        if (frac > 0.4f && overlap > best) best = overlap, g.snapTarget = i;
    }
}

// The tiles touching this one, and those touching them (PrinceWM's Snapping.GetCluster).
std::vector<int> Cluster(int start) {
    std::vector<int> out{start};
    for (size_t k = 0; k < out.size(); k++) {
        Item const& a = g.items[out[k]];
        for (int i = 0; i < (int)g.items.size(); i++) {
            if (std::find(out.begin(), out.end(), i) != out.end()) continue;
            Item const& b = g.items[i];
            float aL = a.pos.x, aR = aL + a.size.x, aT = a.pos.y, aB = aT + a.size.y;
            float bL = b.pos.x, bR = bL + b.size.x, bT = b.pos.y, bB = bT + b.size.y;
            bool yo = aT < bB && aB > bT, xo = aL < bR && aR > bL, eps = 26;
            if ((yo && (std::fabs(aR - bL) <= eps || std::fabs(aL - bR) <= eps)) ||
                (xo && (std::fabs(aB - bT) <= eps || std::fabs(aT - bB) <= eps)))
                out.push_back(i);
        }
    }
    return out;
}

// Two windows side by side on the screen the tile's window is on, the left half and the right.
void SideBySide(HWND left, HWND right) {
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(MonitorFromWindow(left, MONITOR_DEFAULTTONEAREST), &mi);
    RECT wa = mi.rcWork;
    int half = (wa.right - wa.left) / 2;
    for (HWND h : {left, right})
        if (IsIconic(h) || IsZoomed(h)) ShowWindow(h, SW_RESTORE);
    PlaceWindowFrame(left, {wa.left, wa.top, wa.left + half, wa.bottom});
    PlaceWindowFrame(right, {wa.left + half, wa.top, wa.right, wa.bottom});
    Activate(right);
    Activate(left);
}

void NewNote(POINT at) {
    LoadPins();
    float w = 240, h = 180;
    Vec wp = g.cam.ToWorld({(float)at.x, (float)at.y});
    Vec p0 = Resolve(wp - Vec{w / 2, h / 2}, {w, h}, Obstacles(-1, -1), 10);
    Pin p;
    p.kind = PinKind::Note, p.x = p0.x, p.y = p0.y, p.w = w, p.h = h;
    g.pins.push_back(p);
    g.editing = (int)g.pins.size() - 1;
    SavePins();
}

void PasteImage(POINT at) {
    if (!OpenClipboard(g_view.wnd)) return;
    HANDLE data = GetClipboardData(CF_DIB);
    BITMAPINFO* bi = data ? (BITMAPINFO*)GlobalLock(data) : nullptr;
    if (bi) {
        int w = bi->bmiHeader.biWidth, h = std::abs(bi->bmiHeader.biHeight);
        if (w > 0 && h > 0 && w <= 8192 && h <= 8192) {
            BYTE* bits = (BYTE*)bi + bi->bmiHeader.biSize + bi->bmiHeader.biClrUsed * sizeof(RGBQUAD) +
                         (bi->bmiHeader.biCompression == BI_BITFIELDS ? 12 : 0);
            ViewDib dib(w, h);
            if (dib.bits) {
                StretchDIBits(dib.dc, 0, 0, w, h, 0, 0, w, h, bits, bi, DIB_RGB_COLORS, SRCCOPY);
                dib.Opaque();
                WCHAR name[64];
                swprintf_s(name, L"pin-%llx.bmp", (unsigned long long)GetTickCount64() ^ ((unsigned long long)GetCurrentThreadId() << 40));
                if (SaveBmp(Folder() + L"\\" + name, w, h, dib.bits)) {
                    LoadPins();
                    float k = std::min(1.f, 520.f / std::max(w, h));
                    float pw = w * k, ph = h * k;
                    Vec wp = g.cam.ToWorld({(float)at.x, (float)at.y});
                    Vec p0 = Resolve(wp - Vec{pw / 2, ph / 2}, {pw, ph}, Obstacles(-1, -1), 10);
                    Pin p;
                    p.kind = PinKind::Image, p.x = p0.x, p.y = p0.y, p.w = pw, p.h = ph, p.file = name;
                    g.pins.push_back(p);
                    SavePins();
                }
            }
        }
        GlobalUnlock(data);
    }
    CloseClipboard();
}

void RemovePin(int i) {
    if (i < 0 || i >= (int)g.pins.size()) return;
    ViewRelease(g.pins[i].bitmap);
    if (g.pins[i].kind == PinKind::Image) DeleteFileW((Folder() + L"\\" + g.pins[i].file).c_str());
    g.pins.erase(g.pins.begin() + i);
    g.editing = g.pinHover = g.pinDrag = g.pinResize = -1;
    SavePins();
}

void GrowNote(int i) {
    if (i < 0 || i >= (int)g.pins.size() || g.pins[i].kind != PinKind::Note || !g_view.dwrite || !g.noteFmt) return;
    Pin& p = g.pins[i];
    std::wstring text = p.text + L"|";
    IDWriteTextLayout* layout = nullptr;
    if (SUCCEEDED(g_view.dwrite->CreateTextLayout(text.c_str(), (UINT32)text.size(), g.noteFmt, std::max(10.f, (p.w - 24) * g.cam.zoom),
                                                  100000, &layout))) {
        DWRITE_TEXT_METRICS m{};
        layout->GetMetrics(&m);
        float need = m.height / std::max(g.cam.zoom, 0.01f) + 24;
        if (need > p.h) p.h = need;
        layout->Release();
    }
}

void Rearrange() {
    for (auto const& it : g.items) g.places.erase(it.key);
    BuildItems();
    FitAll(true);
}

void MoveSelection(int dx, int dy) {
    if (g.items.empty()) return;
    int at = ItemOf(g_sel);
    if (at < 0) {
        g_sel = g.items[0].entry;
        g.cam.PanTargetTo(g.items[0].Center());
        return;
    }
    Vec from = g.items[at].Center();
    int best = -1;
    float bestScore = FLT_MAX;
    for (int i = 0; i < (int)g.items.size(); i++) {
        if (i == at) continue;
        Vec d = g.items[i].Center() - from;
        bool ok = dx < 0 ? d.x < -1 : dx > 0 ? d.x > 1 : dy < 0 ? d.y < -1 : d.y > 1;
        if (!ok) continue;
        float along = dx ? std::fabs(d.x) : std::fabs(d.y), across = dx ? std::fabs(d.y) : std::fabs(d.x);
        float score = along + across * 2;
        if (score < bestScore) bestScore = score, best = i;
    }
    if (best >= 0) {
        g_sel = g.items[best].entry;
        g.cam.PanTargetTo(g.items[best].Center());
    }
}

UINT Menu(HMENU menu, POINT screen) {
    using Styled = UINT (*)(HMENU, UINT, int, int, HWND);
    static Styled styled = (Styled)GetProcAddress(GetModuleHandleW(nullptr), "TourneStyledMenu");
    SetForegroundWindow(g_view.wnd);
    UINT flags = TPM_RETURNCMD | TPM_RIGHTBUTTON;
    UINT cmd = styled ? styled(menu, flags, screen.x, screen.y, g_view.wnd)
                      : (UINT)TrackPopupMenuEx(menu, flags, screen.x, screen.y, g_view.wnd, nullptr);
    DestroyMenu(menu);
    return cmd;
}

void ToggleDraw() {
    g.draw = !g.draw;
    g.stroking = g.erasing = g.drawPan = false;
    g.editing = -1;
    if (g.draw) LoadStrokes();
}

void RightClick(POINT p) {
    POINT screen = p;
    ClientToScreen(g_view.wnd, &screen);
    UpdatePinHover(p);
    if (g.pinHover >= 0) {
        int i = g.pinHover;
        HMENU m = CreatePopupMenu();
        if (g.pins[i].kind == PinKind::Note) AppendMenuW(m, MF_STRING, 1, L"Edit note");
        AppendMenuW(m, MF_STRING, 2, g.pins[i].locked ? L"Let it move" : L"Pin it in place");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(m, MF_STRING, 3, L"Remove");
        UINT cmd = Menu(m, screen);
        if (cmd == 1) g.editing = i;
        if (cmd == 2) g.pins[i].locked = !g.pins[i].locked, SavePins();
        if (cmd == 3) RemovePin(i);
        return;
    }
    int hit = HitTest(p);
    if (hit >= 0) {
        auto const& e = g_entries[g.items[hit].entry];
        size_t n = e.windows.size();
        HMENU m = CreatePopupMenu();
        AppendMenuW(m, MF_STRING, 1, L"Switch to");
        if (n > 1 && !g.drilled) AppendMenuW(m, MF_STRING, 5, L"Show its windows");
        AppendMenuW(m, MF_STRING, 2, L"Minimise");
        AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(m, MF_STRING, 3, n > 1 && !g.drilled ? L"Close the newest" : L"Close");
        if (n > 1 && !g.drilled) AppendMenuW(m, MF_STRING, 4, (L"Close all (" + std::to_wstring(n) + L")").c_str());
        int entry = g.items[hit].entry;
        UINT cmd = Menu(m, screen);
        if (cmd == 1) BeginCommit(hit);
        if (cmd == 2) ShowWindow(g_entries[entry].windows[0], SW_MINIMIZE);
        if ((cmd == 3 || cmd == 4) && g_wnd) {
            bool all = g_settings.closeAll;
            g_settings.closeAll = cmd == 4;
            CloseItem(g_wnd, entry);
            g_settings.closeAll = all;
        }
        if (cmd == 5 && g_wnd) {
            g_sel = entry;
            Drill(g_wnd);
        }
        return;
    }
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING, 1, L"New note");
    AppendMenuW(m, MF_STRING | (IsClipboardFormatAvailable(CF_DIB) ? 0 : MF_GRAYED), 2, L"Paste picture");
    AppendMenuW(m, MF_STRING | (g.draw ? MF_CHECKED : 0), 3, L"Paint");
    AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m, MF_STRING, 4, L"Fit all");
    AppendMenuW(m, MF_STRING, 5, L"Lay them out again");
    if (!g.strokes.empty()) AppendMenuW(m, MF_STRING, 6, L"Clear the drawing");
    UINT cmd = Menu(m, screen);
    if (cmd == 1) NewNote(p);
    if (cmd == 2) PasteImage(p);
    if (cmd == 3) ToggleDraw();
    if (cmd == 4) FitAll(true);
    if (cmd == 5) Rearrange();
    if (cmd == 6) g.strokes.clear(), SaveStrokes();
}

// Paint: a stroke started, carried on, ended.
void DrawDown(POINT p, bool right) {
    for (int i = 0; i < kTools && !right; i++)
        if (In(ToolRect(i), p)) {
            if (i < 6) g.tool = (Tool)i;
            if (i == 6) g.colorAt++;
            if (i == 7) g.sizeAt = (g.sizeAt + 1) % (int)ARRAYSIZE(kStrokeSizes);
            return;
        }
    g.lastAt = p;
    if (right) {
        g.drawPan = true;
        return;
    }
    Vec wp = g.cam.ToWorld({(float)p.x, (float)p.y});
    if (g.tool == Tool::Fill) {
        for (int i = (int)g.strokes.size() - 1; i >= 0; i--) {
            Stroke& s = g.strokes[i];
            if ((s.kind != 2 && s.kind != 3) || s.pts.size() < 2) continue;
            Vec a = s.pts[0], c = s.pts.back();
            float x0 = std::min(a.x, c.x), x1 = std::max(a.x, c.x), y0 = std::min(a.y, c.y), y1 = std::max(a.y, c.y);
            bool in = s.kind == 2 ? wp.x >= x0 && wp.x <= x1 && wp.y >= y0 && wp.y <= y1 : [&] {
                float rx = (x1 - x0) / 2, ry = (y1 - y0) / 2;
                if (rx <= 0 || ry <= 0) return false;
                float dx = (wp.x - (x0 + rx)) / rx, dy = (wp.y - (y0 + ry)) / ry;
                return dx * dx + dy * dy <= 1;
            }();
            if (in) {
                s.fill = PaintColor(g.colorAt);
                SaveStrokes();
                return;
            }
        }
        return;
    }
    if (g.tool == Tool::Eraser) {
        g.erasing = true;
        return;
    }
    g.inProgress = {};
    g.inProgress.kind = g.tool == Tool::Line ? 1 : g.tool == Tool::Rect ? 2 : g.tool == Tool::Ellipse ? 3 : 0;
    g.inProgress.color = PaintColor(g.colorAt);
    g.inProgress.size = kStrokeSizes[g.sizeAt];
    g.inProgress.pts = {wp};
    if (g.inProgress.kind) g.inProgress.pts.push_back(wp);
    g.stroking = true;
}

void EraseAt(Vec wp) {
    float r = std::max(10.f, kStrokeSizes[g.sizeAt] * 2.5f) / std::max(g.cam.zoom, 0.001f);
    std::erase_if(g.strokes, [&](Stroke const& s) {
        if (s.kind <= 1) {
            for (auto const& p : s.pts)
                if (Length2(p - wp) < r * r) return true;
            return false;
        }
        Vec a = s.pts.front(), c = s.pts.back();
        return wp.x >= std::min(a.x, c.x) - r && wp.x <= std::max(a.x, c.x) + r && wp.y >= std::min(a.y, c.y) - r &&
               wp.y <= std::max(a.y, c.y) + r;
    });
}

void DrawMove(POINT p) {
    g.toolHover = -1;
    for (int i = 0; i < kTools; i++)
        if (In(ToolRect(i), p)) g.toolHover = i;
    if (g.drawPan) {
        g.cam.PanBy({(float)(p.x - g.lastAt.x), (float)(p.y - g.lastAt.y)});
        g.lastAt = p;
        return;
    }
    Vec wp = g.cam.ToWorld({(float)p.x, (float)p.y});
    if (g.stroking) {
        if (g.inProgress.kind == 0) g.inProgress.pts.push_back(wp);
        else g.inProgress.pts.back() = wp;
    } else if (g.erasing) {
        EraseAt(wp);
    }
    g.lastAt = p;
}

void DrawUp() {
    if (g.drawPan) {
        g.drawPan = false;
        return;
    }
    if (g.erasing) {
        g.erasing = false;
        SaveStrokes();
        return;
    }
    if (!g.stroking) return;
    g.stroking = false;
    Stroke s = std::move(g.inProgress);
    bool keep = s.kind == 0 ? s.pts.size() >= 2
                            : std::fabs(s.pts.back().x - s.pts[0].x) > 2 || std::fabs(s.pts.back().y - s.pts[0].y) > 2;
    if (keep) {
        g.strokes.push_back(std::move(s));
        SaveStrokes();
    }
}


////////////////////////////////////////////////////////////////////////////////
// The canvas as the view's other half (called from switcher-views.cpp and the switcher).


// Opened: the tiles laid out, the camera starting where the window in front is (it shrinks into its place),
// or, for the overview, every tile in sight.
void OpenCanvas(bool overview) {
    g.shown = true;
    g.overview = overview;
    g.drilled = !g_outer.empty();
    g.committing = false;
    g.hover = g.tileDrag = g.pinDrag = g.pinResize = g.editing = g.snapTarget = -1;
    g.down = g.dragged = false;
    g.last = {};
    g.cam.view = {(float)g_view.chainW, (float)g_view.chainH};
    LoadPins();
    LoadStrokes();
    BuildItems();
    g.opening = true;
    g.openT = 0;
    if (g.items.empty()) {
        FitAll(false);
        return;
    }
    if (overview) {
        FitAll(false);
        Vec c = g.cam.center;
        float z = g.cam.zoom;
        g.cam.SnapTo(c, z * 1.1f);
        g.cam.TweenTo(c, z, 0.5f, true);
        return;
    }
    Item const& here = g.items[0];
    Vec c;
    float z;
    if (FramePerfect(here, &c, &z)) {
        g.cam.SnapTo(c, z);
        g.cam.TweenTo(here.Center(), FocusZoom(here), 0.52f, true, 3);
        g.opening = false;
    } else {
        g.cam.SnapTo(here.Center(), FocusZoom(here));
    }
}

void CloseCanvas() {
    if (!g.shown) return;
    g.shown = false;
    g.draw = g.stroking = false;
    if (g.editing >= 0) SavePins();
    g.editing = -1;
    if (g.placesLoaded) SaveTilePlaces();
    FreePinBitmaps();
    FreeFormats();
}


// The entries changed (Tab, a drill, a closed window) or only the selection.
void RefitCanvas() {
    if (!g.shown) return;
    bool drilled = !g_outer.empty();
    std::vector<HWND> windows;
    for (auto const& e : g_entries) windows.push_back(e.windows.empty() ? nullptr : e.windows[0]);
    if (drilled != g.drilled) {
        if (drilled) {
            int at = -1;
            std::wstring key = g_entries.empty() ? L"" : Lower(g_entries[0].key);
            for (int i = 0; i < (int)g.items.size(); i++)
                if (g.items[i].key == key) at = i;
            g.returnCenter = g.cam.target, g.returnZoom = g.cam.targetZoom;
            g.drillCenter = at >= 0 ? g.items[at].Center() : g.cam.target;
            g.drilled = true;
            BuildItems();
            FitAll(false);
            Vec c = g.cam.center;
            float z = g.cam.zoom;
            g.cam.SnapTo(g.returnCenter, g.returnZoom);
            g.cam.TweenTo(c, z, 0.44f, true);
        } else {
            g.drilled = false;
            BuildItems();
            int at = ItemOf(g_sel);
            if (at >= 0) g.cam.TweenTo(g.items[at].Center(), FocusZoom(g.items[at]), 0.42f, true);
            else FitAll(true);
        }
        return;
    }
    if (windows != g.lastWindows) {
        BuildItems();
        g.hover = -1;
    }
    int at = ItemOf(g_sel);
    if (at >= 0) g.cam.PanTargetTo(g.items[at].Center());
}

void MoveKey(int dx, int dy, bool shift) {
    if (!g.shown || g.committing) return;
    if (!shift) return MoveSelection(dx, dy);
    int at = ItemOf(g_sel);
    if (at < 0 || g.drilled) return;
    g.items[at].pos = g.items[at].pos + Vec{dx * kNudge, dy * kNudge};
    Remember(g.items[at]);
}

void WheelZoom(short delta) {
    if (!g.shown || g.committing) return;
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(g_view.wnd, &p);
    g.cam.ZoomAt({(float)p.x, (float)p.y}, std::pow(1.2f, delta / 120.f));
}

// A key while the canvas is up (the switcher's hook passes them all on).
void KeyDown(UINT vk, UINT scan, bool shift, bool ctrl) {
    if (!g.shown || g.committing) return;
    HWND sw = g_wnd;
    if (g.editing >= 0 && g.editing < (int)g.pins.size()) {
        Pin& p = g.pins[g.editing];
        if (vk == VK_ESCAPE || (vk == VK_RETURN && !shift)) {
            g.editing = -1;
            SavePins();
        } else if (vk == VK_RETURN) {
            p.text += L"\n";
        } else if (vk == VK_BACK) {
            if (!p.text.empty()) p.text.pop_back();
        } else {
            BYTE keys[256] = {};
            if (shift) keys[VK_SHIFT] = 0x80;
            if (GetKeyState(VK_CAPITAL) & 1) keys[VK_CAPITAL] = 1;
            if (GetAsyncKeyState(VK_RMENU) < 0) keys[VK_CONTROL] = keys[VK_MENU] = 0x80;  // (AltGr)
            WCHAR out[4] = {};
            HKL layout = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), nullptr));
            int n = ToUnicodeEx(vk, scan, keys, out, 4, 4, layout);
            for (int i = 0; i < n; i++)
                if (out[i] >= 32) p.text += out[i];
        }
        GrowNote(g.editing);
        return;
    }
    switch (vk) {
        case VK_ESCAPE:
            if (g.draw) ToggleDraw();
            else if (sw) PostMessageW(sw, WM_APP_CANCEL, 0, 0);
            return;
        case VK_RETURN:
        case VK_SPACE:
            BeginCommit(ItemOf(g_sel));
            return;
        case VK_LEFT: return MoveKey(-1, 0, shift);
        case VK_RIGHT: return MoveKey(1, 0, shift);
        case VK_UP: return MoveKey(0, -1, shift);
        case VK_DOWN: return MoveKey(0, 1, shift);
        case 'W': FitAll(true); return;
        case 'F':
        case 'M':
            if (int at = ItemOf(g_sel); at >= 0)
                g.cam.TweenTo(g.items[at].Center(), std::max(g.cam.view.x / g.items[at].size.x, g.cam.view.y / g.items[at].size.y), 0.28f, true);
            return;
        case 'C':
            if (int at = ItemOf(g_sel); at >= 0) g.cam.TweenTo(g.items[at].Center(), g.cam.targetZoom, 0.22f);
            return;
        case '0':
        case VK_NUMPAD0:
        case 'Z': g.cam.TweenTo(g.cam.target, 1, 0.24f); return;
        case VK_OEM_PLUS:
        case VK_ADD: g.cam.ZoomAt(g.cam.view * 0.5f, 1.18f); return;
        case VK_OEM_MINUS:
        case VK_SUBTRACT: g.cam.ZoomAt(g.cam.view * 0.5f, 1 / 1.18f); return;
        case 'R': Rearrange(); return;
        case 'D': ToggleDraw(); return;
        case 'N': {
            POINT p;
            GetCursorPos(&p);
            ScreenToClient(g_view.wnd, &p);
            NewNote(p);
            return;
        }
        case 'V':
            if (ctrl) {
                POINT p;
                GetCursorPos(&p);
                ScreenToClient(g_view.wnd, &p);
                PasteImage(p);
            }
            return;
        case 'Q':
        case VK_DELETE:
            if (sw) CloseItem(sw, g_sel);
            return;
    }
}

// The mouse on the view's window, while it's the canvas.
bool MouseMsg(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!g.shown) return false;
    POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    bool shift = wParam & MK_SHIFT, alt = GetKeyState(VK_MENU) < 0;
    if (g.committing) return true;
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            SetCapture(g_view.wnd);
            bool left = msg == WM_LBUTTONDOWN, middle = msg == WM_MBUTTONDOWN;
            if (g.draw) {
                if (!middle) DrawDown(p, !left);
                return true;
            }
            if (!left && !middle) return true;  // (the right button acts as it comes up)
            if (left) {
                UpdatePinHover(p);
                if (g.pinHover >= 0) {
                    int i = g.pinHover;
                    if (g.pinHoverClose) {
                        RemovePin(i);
                        return true;
                    }
                    if (OnResizeCorner(i, p)) {
                        g.pinResize = i;
                        return true;
                    }
                    if (g.editing != i && g.editing >= 0) SavePins();
                    g.editing = -1;
                    if (g.pins[i].locked) return true;
                    g.pinDrag = i, g.pinDragged = false, g.pinDownAt = g.lastAt = p;
                    g.pinOffset = g.cam.ToWorld({(float)p.x, (float)p.y}) - g.pins[i].Pos();
                    return true;
                }
                if (g.editing >= 0) {
                    g.editing = -1;
                    SavePins();
                }
            }
            if (middle || alt) {
                int hit = HitTest(p);
                if (hit >= 0) {
                    g.tileDrag = hit;
                    g.items[hit].sliding = false;
                    g_sel = g.items[hit].entry;
                    g.lastAt = g.tileDownAt = p;
                    g.drillOnClick = middle && !g.drilled && g_entries[g.items[hit].entry].windows.size() > 1 ? hit : -1;
                    g.cluster = shift && !g.drilled ? Cluster(hit) : std::vector<int>{};
                }
                return true;
            }
            g.down = true, g.dragged = false;
            g.downAt = g.lastAt = p;
            g.dragVel = {};
            QueryPerformanceCounter(&g.lastMove);
            return true;
        }
        case WM_MOUSEMOVE: {
            if (g.draw) {
                DrawMove(p);
                return true;
            }
            if (g.pinResize >= 0 && g.pinResize < (int)g.pins.size()) {
                Vec w = g.cam.ToWorld({(float)p.x, (float)p.y});
                Pin& pin = g.pins[g.pinResize];
                pin.w = std::max(60.f, w.x - pin.x), pin.h = std::max(40.f, w.y - pin.y);
                return true;
            }
            if (g.pinDrag >= 0 && g.pinDrag < (int)g.pins.size()) {
                if (std::abs(p.x - g.pinDownAt.x) + std::abs(p.y - g.pinDownAt.y) > kDragThreshold) g.pinDragged = true;
                if (g.pinDragged) {
                    Vec w = g.cam.ToWorld({(float)p.x, (float)p.y}) - g.pinOffset;
                    g.pins[g.pinDrag].x = w.x, g.pins[g.pinDrag].y = w.y;
                }
                return true;
            }
            if (g.tileDrag >= 0 && g.tileDrag < (int)g.items.size()) {
                if (std::abs(p.x - g.tileDownAt.x) + std::abs(p.y - g.tileDownAt.y) <= kDragThreshold) {
                    g.lastAt = p;
                    return true;
                }
                g.drillOnClick = -1;
                Vec d = Vec{(float)(p.x - g.lastAt.x), (float)(p.y - g.lastAt.y)} * (1 / g.cam.zoom);
                if (!g.cluster.empty()) {
                    for (int i : g.cluster) g.items[i].pos = g.items[i].pos + d, Remember(g.items[i]);
                } else {
                    g.items[g.tileDrag].pos = g.items[g.tileDrag].pos + d;
                    Remember(g.items[g.tileDrag]);
                    UpdateSnapTarget();
                }
                g.lastAt = p;
                return true;
            }
            if (!g.down) {
                UpdateHover(p);
                UpdatePinHover(p);
                return true;
            }
            if (std::abs(p.x - g.downAt.x) + std::abs(p.y - g.downAt.y) > kDragThreshold) g.dragged = true;
            if (g.dragged) {
                Vec delta{(float)(p.x - g.lastAt.x), (float)(p.y - g.lastAt.y)};
                g.cam.PanBy(delta);
                LARGE_INTEGER now, freq;
                QueryPerformanceCounter(&now);
                QueryPerformanceFrequency(&freq);
                float dt = (float)((now.QuadPart - g.lastMove.QuadPart) / (double)freq.QuadPart);
                g.lastMove = now;
                if (dt > 0.0001f) g.dragVel = Lerp(g.dragVel, delta * (1 / dt), 0.4f);
            }
            g.lastAt = p;
            return true;
        }
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            ReleaseCapture();
            if (g.draw) {
                DrawUp();
                return true;
            }
            if (msg == WM_RBUTTONUP) {
                RightClick(p);
                return true;
            }
            if (g.pinResize >= 0) {
                g.pinResize = -1;
                SavePins();
                return true;
            }
            if (g.pinDrag >= 0) {
                if (g.pinDragged && g.pinDrag < (int)g.pins.size()) {
                    Pin& pin = g.pins[g.pinDrag];
                    Vec at = Resolve(pin.Pos(), pin.Size(), Obstacles(-1, g.pinDrag), 10);
                    pin.x = at.x, pin.y = at.y;
                } else if (g.pinDrag < (int)g.pins.size() && g.pins[g.pinDrag].kind == PinKind::Note) {
                    g.editing = g.pinDrag;  // (a click on a note writes in it)
                }
                g.pinDrag = -1;
                g.pinDragged = false;
                SavePins();
                return true;
            }
            if (g.tileDrag >= 0) {
                int dragged = g.tileDrag;
                g.tileDrag = -1;
                if (g.drillOnClick >= 0 && g.drillOnClick < (int)g.items.size()) {
                    g.drillOnClick = -1;
                    g.cluster.clear();
                    if (g_wnd) Drill(g_wnd);  // (g_sel is the stack: Drill opens it)
                    return true;
                }
                if (g.cluster.empty() && g.snapTarget >= 0 && g.snapTarget < (int)g.items.size() && g.snapTarget != dragged) {
                    Item const& a = g.items[dragged];
                    Item const& b = g.items[g.snapTarget];
                    bool aLeft = a.Center().x <= b.Center().x;
                    HWND left = g_entries[(aLeft ? a : b).entry].windows[0], right = g_entries[(aLeft ? b : a).entry].windows[0];
                    g.snapTarget = -1;
                    for (auto& it : g.items) Remember(it);
                    if (g_wnd) Hide(g_wnd);
                    SideBySide(left, right);
                    return true;
                }
                if (g.cluster.empty() && dragged < (int)g.items.size()) {
                    Item& moved = g.items[dragged];
                    Vec at = Resolve(moved.pos, moved.size, Obstacles(dragged, -1), 12);
                    if (Length2(at - moved.pos) > 0.5f) moved.slideTo = at, moved.sliding = true;
                }
                for (auto& it : g.items) Remember(it);
                g.cluster.clear();
                g.snapTarget = -1;
                g.drillOnClick = -1;
                return true;
            }
            if (msg == WM_MBUTTONUP) return true;
            bool wasDrag = g.dragged;
            g.down = g.dragged = false;
            if (wasDrag) {
                if (Length2(g.dragVel) > 400) g.cam.Flick(g.dragVel);
                return true;
            }
            UpdateHover(p);
            if (g.hoverClose && g.hover >= 0 && g.hover < (int)g.items.size()) {
                if (g_wnd) CloseItem(g_wnd, g.items[g.hover].entry);
                return true;
            }
            if (int hit = HitTest(p); hit >= 0) BeginCommit(hit);
            return true;
        }
    }
    return false;
}

}  // namespace canvas

bool CanvasShown() { return canvas::g.shown; }
void CanvasOpen(bool overview) { canvas::OpenCanvas(overview); }
void CanvasClose() { canvas::CloseCanvas(); }
void CanvasRender() { canvas::Render(); }
void CanvasRefit() { canvas::RefitCanvas(); }
void CanvasMove(int dx, int dy, bool shift) { canvas::MoveKey(dx, dy, shift); }
void CanvasWheel(short delta) { canvas::WheelZoom(delta); }
void CanvasKey(UINT vk, UINT scan, bool shift, bool ctrl) { canvas::KeyDown(vk, scan, shift, ctrl); }
bool CanvasMouse(UINT msg, WPARAM wParam, LPARAM lParam) { return canvas::MouseMsg(msg, wParam, lParam); }
