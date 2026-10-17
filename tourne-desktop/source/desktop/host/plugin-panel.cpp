#include "plugin-panel.h"

#include <commctrl.h>
#include <d2d1_3.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <wincodec.h>
#include <shlwapi.h>
#include <windowsx.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>

#include "config.h"
#include "motion.cpp"
#include "td.h"

namespace td::panel {

namespace {

ULONGLONG g_lastDismissed = 0;
constexpr wchar_t kClass[] = L"TourneDesktopPluginPanel";
constexpr UINT_PTR kInputSubclass = 1;

std::wstring W(const std::string& s) { return Utf8ToWide(s); }
std::string U(const std::wstring& s) { return WideToUtf8(s); }

////////////////////////////////////////////////////////////////////////////////
// Coming and going, as Hyprland moves its layers (motion.cpp): layersIn / layersOut move it (fade, or slide in
// from its bar's side; slidefade both), fadeLayersIn / fadeLayersOut time its fade when set. No line: at once.

motion::Anim PanelMotion(const wchar_t* leaf) {
    return motion::Resolve(leaf, [](const std::wstring& key) {
        const wchar_t* v = GetString(L"desktop", key);  // (motion.* is a full name)
        std::wstring s = v ? v : L"";
        FreeString(v);
        return s;
    });
}

struct PanelMove {
    bool closing = false;
    ULONGLONG start = 0;
    motion::Anim move, fade;
    bool slides = false, fades = false;
    POINT place{}, away{};  // where it sits, and where a slide starts (opening) or ends (closing)
};
std::map<HWND, PanelMove> g_panelMoves;  // (the host's thread only)
constexpr UINT_PTR kPanelMoveTimer = 0x7D01;

void CALLBACK PanelMoveStep(HWND wnd, UINT, UINT_PTR id, DWORD) {
    auto it = g_panelMoves.find(wnd);
    if (it == g_panelMoves.end() || !IsWindow(wnd)) {
        KillTimer(wnd, id);
        if (it != g_panelMoves.end()) g_panelMoves.erase(it);
        return;
    }
    PanelMove& m = it->second;
    double elapsed = (double)(GetTickCount64() - m.start);
    double tMove = std::min(1.0, elapsed / std::max(1.0, motion::Ms(m.move)));
    double tFade = std::min(1.0, elapsed / std::max(1.0, motion::Ms(m.fade)));
    double p = motion::Ease(m.move, tMove), f = std::clamp(motion::Ease(m.fade, tFade), 0.0, 1.0);
    if (m.closing) p = 1 - p, f = 1 - f;  // (from in place to away)
    if (m.slides) {
        int x = (int)std::lround(m.away.x + (m.place.x - m.away.x) * p), y = (int)std::lround(m.away.y + (m.place.y - m.away.y) * p);
        SetWindowPos(wnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    SetLayeredWindowAttributes(wnd, 0, (BYTE)std::lround(255 * (m.fades ? f : 1.0)), LWA_ALPHA);
    if (tMove < 1 || tFade < 1) return;
    KillTimer(wnd, id);
    bool closing = m.closing;
    g_panelMoves.erase(it);
    if (closing) {
        DestroyWindow(wnd);
        return;
    }
    SetWindowLongPtrW(wnd, GWL_EXSTYLE, GetWindowLongPtrW(wnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);  // (opaque again, as before)
    RedrawWindow(wnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}

// Starts it moving in (closing false: just shown at place) or out (closing: then destroyed); false when no
// line says to (or it's off), and the caller shows or destroys it at once. side: the way it slides out.
bool PanelMoveStart(HWND wnd, bool closing, POINT side, int distance) {
    motion::Anim move = PanelMotion(closing ? L"layersOut" : L"layersIn");
    motion::Anim fade = PanelMotion(closing ? L"fadeLayersOut" : L"fadeLayersIn");
    if ((move.set && !move.on) || (!move.set && !(fade.set && fade.on))) return false;
    PanelMove m;
    m.closing = closing;
    m.start = GetTickCount64();
    std::wstring style = motion::StyleWord(move);
    m.slides = move.set && style.rfind(L"slide", 0) == 0;
    m.fades = !m.slides || style.find(L"fade") != std::wstring::npos || (fade.set && fade.on);
    m.move = move.set ? move : fade;
    m.fade = fade.set && fade.on ? fade : m.move;
    RECT r;
    GetWindowRect(wnd, &r);
    if (auto old = g_panelMoves.find(wnd); old != g_panelMoves.end()) {  // (going the other way part way through)
        r.left = old->second.place.x, r.top = old->second.place.y;
        g_panelMoves.erase(old);
    }
    m.place = {r.left, r.top};
    m.away = {r.left + side.x * distance, r.top + side.y * distance};
    SetWindowLongPtrW(wnd, GWL_EXSTYLE, GetWindowLongPtrW(wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    g_panelMoves[wnd] = m;
    PanelMoveStep(wnd, 0, kPanelMoveTimer, 0);  // (its first frame now, before it shows)
    if (g_panelMoves.count(wnd)) SetTimer(wnd, kPanelMoveTimer, 10, PanelMoveStep);
    return true;
}

// Props.
const json::Value* Prop(const Node& n, const char* name) {
    for (auto const& [k, v] : n.props.fields)
        if (k == name) return &v;
    return nullptr;
}
double Num(const Node& n, const char* name, double fallback) {
    auto v = Prop(n, name);
    return v && v->kind == json::Value::Number ? v->number : fallback;
}
bool Has(const Node& n, const char* name) {
    auto v = Prop(n, name);
    return v && v->kind != json::Value::Null;
}
bool Bool(const Node& n, const char* name, bool fallback) {
    auto v = Prop(n, name);
    return v && v->kind == json::Value::Bool ? v->boolean : fallback;
}
std::string Str(const Node& n, const char* name, const std::string& fallback = "") {
    auto v = Prop(n, name);
    if (!v) return fallback;
    if (v->kind == json::Value::String) return v->text;
    if (v->kind == json::Value::Number) {
        char b[32];
        snprintf(b, sizeof(b), "%.15g", v->number);
        return b;
    }
    return fallback;
}
bool Container(const std::string& t) { return t == "column" || t == "row" || t == "scroll" || t == "dragSource" || t == "dropZone"; }
bool Across(const Node& n) {  // lays its children out in a row
    return n.type == "row" || (n.type == "dropZone" && Str(n, "direction") == "row");
}

// Fonts: the flyouts' pixel fonts when they're installed (System Icons' choice), else Consolas.
bool FontInstalled(const wchar_t* face) {
    LOGFONTW lf{};
    lf.lfCharSet = DEFAULT_CHARSET;
    wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
    bool found = false;
    HDC dc = GetDC(nullptr);
    EnumFontFamiliesExW(dc, &lf, [](const LOGFONTW*, const TEXTMETRICW*, DWORD, LPARAM p) -> int { return *(bool*)p = true, 0; },
                        (LPARAM)&found, 0);
    ReleaseDC(nullptr, dc);
    return found;
}

HFONT MakeFont(std::initializer_list<const wchar_t*> faces, int pixels, bool bold, bool pixelFont = true) {
    LOGFONTW lf{};
    lf.lfHeight = -pixels;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    for (const wchar_t* face : faces)
        if (FontInstalled(face)) {
            lf.lfQuality = pixelFont ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
            wcsncpy_s(lf.lfFaceName, face, _TRUNCATE);
            return CreateFontIndirectW(&lf);
        }
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, L"Consolas");
    return CreateFontIndirectW(&lf);
}

struct Fonts {
    int scale = 0;
    std::map<std::tuple<int, bool, int>, HFONT> text;  // (size tier, bold, family)
    std::map<int, HFONT> glyphs;                       // by pixel size
    ~Fonts() { Clear(); }
    void Clear() {
        for (auto& [k, f] : text) DeleteObject(f);
        for (auto& [k, f] : glyphs) DeleteObject(f);
        text.clear(), glyphs.clear();
    }
    // A plugin's fontSize, to the nearest of the flyouts' three: 11 (scientifica), 13 (Cozette), 16 (Proggy).
    // fontFamily "mono" (Consolas) or "smooth" (Segoe UI) instead: those sizes, smooth.
    HFONT Text(double size, bool bold, const std::string& family = "") {
        int tier = size <= 11.5 ? 11 : size <= 14.5 ? 13 : 16;
        int kind = family == "mono" ? 1 : family == "smooth" ? 2 : 0;
        HFONT& f = text[{tier, bold, kind}];
        if (!f) {
            if (kind == 1) f = MakeFont({L"Consolas", L"Cascadia Mono"}, (tier + 2) * scale, bold, false);
            else if (kind == 2) f = MakeFont({L"Segoe UI", L"Tahoma"}, (tier + 2) * scale, bold, false);
            else if (tier == 11) f = MakeFont({L"scientifica", L"CozetteVector"}, 11 * scale, bold);
            else if (tier == 13) f = MakeFont({L"CozetteVector", L"scientifica"}, 13 * scale, bold);
            else f = MakeFont({L"ProggyClean Nerd Font Mono", L"ProggyCleanTT", L"CozetteVector"}, 16 * scale, bold);
        }
        return f;
    }
    HFONT Glyph(int px) {
        HFONT& f = glyphs[px];
        if (!f)
            f = CreateFontW(-px, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, DEFAULT_PITCH, L"tabler-icons");
        return f;
    }
};

// Pictures: an image node's file (its path), read once at most 1600 pixels wide (an SVG drawn at four times its
// own size) and kept, with a copy at the size it was last drawn. Read again when the file changes.
struct Picture {
    bool failed = false;
    bool vector = false;  // an SVG: drawn to fill the room it's given
    UINT w = 0, h = 0;
    double nw = 0, nh = 0;  // its own size (96-dpi pixels)
    std::vector<uint32_t> pixels;  // premultiplied BGRA, top-down
    FILETIME written{};
    int drawnW = 0, drawnH = 0;
    HBITMAP drawn = nullptr;
};

IWICImagingFactory* Wic() {
    static IWICImagingFactory* factory = nullptr;  // (the runtime's thread has COM)
    if (!factory) CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    return factory;
}

// WIC's pictures (PNG, JPEG, BMP, GIF, ICO, TIFF, and what codecs Windows has), at most `longest` on the longer side.
bool ReadRaster(const std::wstring& path, int longest, std::vector<uint32_t>* pixels, UINT* outW, UINT* outH) {
    IWICImagingFactory* f = Wic();
    if (!f) return false;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    IWICFormatConverter* convert = nullptr;
    UINT w = 0, h = 0;
    bool ok = SUCCEEDED(f->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
              SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(frame->GetSize(&w, &h)) && w && h;
    IWICBitmapSource* source = frame;
    if (ok && longest > 0 && std::max(w, h) > (UINT)longest) {
        double k = (double)longest / std::max(w, h);
        UINT nw = std::max(1u, (UINT)std::lround(w * k)), nh = std::max(1u, (UINT)std::lround(h * k));
        ok = SUCCEEDED(f->CreateBitmapScaler(&scaler)) && SUCCEEDED(scaler->Initialize(frame, nw, nh, WICBitmapInterpolationModeFant));
        source = scaler, w = nw, h = nh;
    }
    if (ok)
        ok = SUCCEEDED(f->CreateFormatConverter(&convert)) &&
             SUCCEEDED(convert->Initialize(source, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom));
    if (ok) {
        pixels->assign((size_t)w * h, 0);
        ok = SUCCEEDED(convert->CopyPixels(nullptr, w * 4, w * h * 4, (BYTE*)pixels->data()));
    }
    for (IUnknown* u : std::initializer_list<IUnknown*>{convert, scaler, frame, decoder})
        if (u) u->Release();
    *outW = ok ? w : 0, *outH = ok ? h : 0;
    if (!ok) pixels->clear();
    return ok;
}

bool IsSvg(const std::wstring& path) {
    return path.size() > 4 && (_wcsicmp(path.c_str() + path.size() - 4, L".svg") == 0);
}

// An SVG's root element: its width and height (96-dpi pixels; 0 when it gives none or a percentage), and its
// viewBox's width and height.
void SvgSize(const std::string& text, double* w, double* h, double* boxW, double* boxH) {
    *w = *h = *boxW = *boxH = 0;
    size_t at = 0;
    for (;;) {  // (past "<?xml", comments and the doctype)
        at = text.find("<svg", at);
        if (at == std::string::npos) return;
        char next = at + 4 < text.size() ? text[at + 4] : 0;
        if (next == ' ' || next == '\t' || next == '\r' || next == '\n' || next == '>') break;
        at += 4;
    }
    size_t end = text.find('>', at);
    std::string tag = text.substr(at, end == std::string::npos ? std::string::npos : end - at);
    auto attribute = [&](const char* name) -> std::string {
        std::string n = name;
        for (size_t p = tag.find(n); p != std::string::npos; p = tag.find(n, p + 1)) {
            if (p == 0 || !isspace((unsigned char)tag[p - 1])) continue;
            size_t q = p + n.size();
            while (q < tag.size() && isspace((unsigned char)tag[q])) q++;
            if (q >= tag.size() || tag[q] != '=') continue;
            q++;
            while (q < tag.size() && isspace((unsigned char)tag[q])) q++;
            if (q >= tag.size() || (tag[q] != '"' && tag[q] != '\'')) continue;
            size_t close = tag.find(tag[q], q + 1);
            if (close == std::string::npos) return "";
            return tag.substr(q + 1, close - q - 1);
        }
        return "";
    };
    auto length = [](const std::string& v) {
        char* rest = nullptr;
        double n = strtod(v.c_str(), &rest);
        std::string unit = rest ? rest : "";
        while (!unit.empty() && isspace((unsigned char)unit.back())) unit.pop_back();
        if (unit == "%" || n <= 0) return 0.0;
        if (unit == "pt") return n * 96 / 72;
        if (unit == "pc") return n * 16;
        if (unit == "mm") return n * 96 / 25.4;
        if (unit == "cm") return n * 96 / 2.54;
        if (unit == "in") return n * 96;
        if (unit == "em" || unit == "rem") return n * 16;
        return n;
    };
    *w = length(attribute("width")), *h = length(attribute("height"));
    std::string box = attribute("viewBox");
    for (auto& c : box)
        if (c == ',') c = ' ';
    double v[4] = {0, 0, 0, 0};
    const char* p = box.c_str();
    for (int i = 0; i < 4 && *p; i++) {
        char* next = nullptr;
        v[i] = strtod(p, &next);
        if (next == p) break;
        p = next;
    }
    *boxW = std::max(0.0, v[2]), *boxH = std::max(0.0, v[3]);
}

// An SVG drawn by Direct2D (ID2D1SvgDocument, Windows 10 1703 and later) at `longest` pixels on its longer side
// (0: four times its own size, 64 to 1024).
bool ReadSvg(const std::wstring& path, int longest, std::vector<uint32_t>* pixels, UINT* outW, UINT* outH, double* naturalW, double* naturalH) {
    std::string text;
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    if (GetFileSizeEx(file, &size) && size.QuadPart > 0 && size.QuadPart < (8 << 20)) {
        text.resize((size_t)size.QuadPart);
        DWORD got = 0;
        if (!ReadFile(file, text.data(), (DWORD)text.size(), &got, nullptr)) got = 0;
        text.resize(got);
    }
    CloseHandle(file);
    if (text.empty()) return false;
    double w, h, boxW, boxH;
    SvgSize(text, &w, &h, &boxW, &boxH);
    if (w <= 0 && h > 0 && boxW > 0 && boxH > 0) w = h * boxW / boxH;
    if (h <= 0 && w > 0 && boxW > 0 && boxH > 0) h = w * boxH / boxW;
    if (w <= 0 || h <= 0) w = boxW, h = boxH;
    if (w <= 0 || h <= 0) w = h = 24;
    if (naturalW) *naturalW = w;
    if (naturalH) *naturalH = h;
    if (longest <= 0) longest = std::clamp((int)std::lround(std::max(w, h) * 4), 64, 1024);
    double k = longest / std::max(w, h);
    UINT pw = std::max(1u, (UINT)std::lround(w * k)), ph = std::max(1u, (UINT)std::lround(h * k));

    static ID2D1Factory1* factory = [] {
        ID2D1Factory1* f = nullptr;
        D2D1_FACTORY_OPTIONS options{};
        D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), &options, (void**)&f);
        return f;
    }();
    IWICImagingFactory* wic = Wic();
    if (!factory || !wic) return false;
    IWICBitmap* bitmap = nullptr;
    ID2D1RenderTarget* target = nullptr;
    ID2D1DeviceContext5* dc = nullptr;
    IStream* stream = nullptr;
    ID2D1SvgDocument* doc = nullptr;
    ID2D1SvgElement* root = nullptr;
    D2D1_RENDER_TARGET_PROPERTIES props{D2D1_RENDER_TARGET_TYPE_SOFTWARE, {DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED}, 96, 96,
                                        D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT};
    bool ok = SUCCEEDED(wic->CreateBitmap(pw, ph, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bitmap)) &&
              SUCCEEDED(factory->CreateWicBitmapRenderTarget(bitmap, &props, &target)) &&
              SUCCEEDED(target->QueryInterface(__uuidof(ID2D1DeviceContext5), (void**)&dc)) &&
              (stream = SHCreateMemStream((const BYTE*)text.data(), (UINT)text.size())) != nullptr &&
              SUCCEEDED(dc->CreateSvgDocument(stream, D2D1_SIZE_F{(float)pw, (float)ph}, &doc));
    if (ok) {
        // Drawn to fill the picture: the root as big as it, seen through a viewBox of its own size when it had none.
        doc->GetRoot(&root);
        if (root) {
            if (boxW <= 0 || boxH <= 0) {
                D2D1_SVG_VIEWBOX box{0, 0, (float)w, (float)h};
                root->SetAttributeValue(L"viewBox", D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX, &box, sizeof(box));
            }
            D2D1_SVG_LENGTH lw{(float)pw, D2D1_SVG_LENGTH_UNITS_NUMBER}, lh{(float)ph, D2D1_SVG_LENGTH_UNITS_NUMBER};
            root->SetAttributeValue(L"width", D2D1_SVG_ATTRIBUTE_POD_TYPE_LENGTH, &lw, sizeof(lw));
            root->SetAttributeValue(L"height", D2D1_SVG_ATTRIBUTE_POD_TYPE_LENGTH, &lh, sizeof(lh));
        }
        dc->BeginDraw();
        dc->Clear(D2D1_COLOR_F{0, 0, 0, 0});
        dc->DrawSvgDocument(doc);
        ok = SUCCEEDED(dc->EndDraw());
    }
    if (ok) {
        pixels->assign((size_t)pw * ph, 0);
        ok = SUCCEEDED(bitmap->CopyPixels(nullptr, pw * 4, pw * ph * 4, (BYTE*)pixels->data()));
    }
    for (IUnknown* u : std::initializer_list<IUnknown*>{root, doc, stream, dc, target, bitmap})
        if (u) u->Release();
    *outW = ok ? pw : 0, *outH = ok ? ph : 0;
    if (!ok) pixels->clear();
    return ok;
}

bool ReadPicture(const std::wstring& path, Picture* out) {
    out->vector = IsSvg(path);
    bool ok = out->vector ? ReadSvg(path, 0, &out->pixels, &out->w, &out->h, &out->nw, &out->nh)
                          : ReadRaster(path, 1600, &out->pixels, &out->w, &out->h);
    if (ok && !out->vector) out->nw = out->w, out->nh = out->h;
    return ok;
}

// The picture at a size, as a 32-bit DIB section (scaled once, kept until the size changes).
HBITMAP DrawnAt(Picture& p, int w, int h) {
    if (p.drawn && p.drawnW == w && p.drawnH == h) return p.drawn;
    if (p.drawn) DeleteObject(p.drawn), p.drawn = nullptr;
    IWICImagingFactory* f = Wic();
    if (!f || p.pixels.empty() || w <= 0 || h <= 0) return nullptr;
    IWICBitmap* bitmap = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    bool ok = SUCCEEDED(f->CreateBitmapFromMemory(p.w, p.h, GUID_WICPixelFormat32bppPBGRA, p.w * 4, p.w * p.h * 4,
                                                  (BYTE*)p.pixels.data(), &bitmap));
    IWICBitmapSource* source = bitmap;
    if (ok && ((UINT)w != p.w || (UINT)h != p.h)) {
        ok = SUCCEEDED(f->CreateBitmapScaler(&scaler)) && SUCCEEDED(scaler->Initialize(bitmap, w, h, WICBitmapInterpolationModeFant));
        source = scaler;
    }
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    void* bits = nullptr;
    HBITMAP out = ok ? CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS, &bits, nullptr, 0) : nullptr;
    if (out && FAILED(source->CopyPixels(nullptr, w * 4, w * h * 4, (BYTE*)bits))) DeleteObject(out), out = nullptr;
    if (scaler) scaler->Release();
    if (bitmap) bitmap->Release();
    p.drawn = out, p.drawnW = w, p.drawnH = h;
    return out;
}

// The paths of the files on the clipboard (copied in File Explorer).
std::vector<std::string> ClipboardFiles(HWND owner) {
    std::vector<std::string> out;
    if (!OpenClipboard(owner)) return out;
    if (HDROP drop = (HDROP)GetClipboardData(CF_HDROP))
        for (UINT i = 0, n = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0); i < n; i++) {
            std::wstring path(DragQueryFileW(drop, i, nullptr, 0), L'\0');
            DragQueryFileW(drop, i, path.data(), (UINT)path.size() + 1);
            out.push_back(WideToUtf8(path));
        }
    CloseClipboard();
    return out;
}

// Wallpaper depth (SetDepthMask): per screen, the wallpaper and its mask as read, and how Windows places it.
struct Depth {
    std::wstring screen;
    RECT monitor{};
    std::vector<uint32_t> wall;  // the wallpaper, premultiplied BGRA
    UINT ww = 0, wh = 0;
    double ow = 0, oh = 0;       // its own size (centred and tiled pictures aren't scaled)
    std::vector<uint8_t> mask;   // 255: the wallpaper shows over desktop widgets
    UINT mw = 0, mh = 0;
    int style = 10;              // Windows' WallpaperStyle: 10 fill, 6 fit, 2 stretch, 0 centre (1: tile), 22 span
};
std::mutex g_depthLock;
std::vector<Depth> g_depths;
std::set<HWND> g_desktopWindows;  // desktop widgets now (repainted when a mask changes)

// Where a screen's pixel (x, y from its corner) is in the wallpaper (0..1 across and down); false off it.
bool WallpaperAt(const Depth& d, double x, double y, double* u, double* v) {
    double mw = d.monitor.right - d.monitor.left, mh = d.monitor.bottom - d.monitor.top;
    double sw = d.ow > 0 ? d.ow : d.ww, sh = d.oh > 0 ? d.oh : d.wh;
    if (mw <= 0 || mh <= 0 || sw <= 0 || sh <= 0) return false;
    double dw, dh;
    if (d.style == 2) dw = mw, dh = mh;
    else if (d.style == 0 || d.style == 1) dw = sw, dh = sh;
    else {
        double k = d.style == 6 ? std::min(mw / sw, mh / sh) : std::max(mw / sw, mh / sh);
        dw = sw * k, dh = sh * k;
    }
    if (d.style == 1) {
        *u = std::fmod(x, dw) / dw, *v = std::fmod(y, dh) / dh;
        return true;
    }
    *u = (x - (mw - dw) / 2) / dw, *v = (y - (mh - dh) / 2) / dh;
    return *u >= 0 && *u < 1 && *v >= 0 && *v < 1;
}

// The wallpaper's foreground drawn over a desktop widget's pixels (top-down, `w` wide), where it is on screen.
void ApplyDepth(uint32_t* bits, int w, int h, RECT screen) {
    std::lock_guard lock(g_depthLock);
    for (auto const& d : g_depths) {
        RECT both;
        if (!IntersectRect(&both, &screen, &d.monitor) || d.mask.empty() || d.wall.empty()) continue;
        for (int y = both.top; y < both.bottom; y++)
            for (int x = both.left; x < both.right; x++) {
                double u, v;
                if (!WallpaperAt(d, x - d.monitor.left + 0.5, y - d.monitor.top + 0.5, &u, &v)) continue;
                uint32_t m = d.mask[std::min(d.mh - 1, (UINT)(v * d.mh)) * (size_t)d.mw + std::min(d.mw - 1, (UINT)(u * d.mw))];
                if (!m) continue;
                uint32_t p = d.wall[std::min(d.wh - 1, (UINT)(v * d.wh)) * (size_t)d.ww + std::min(d.ww - 1, (UINT)(u * d.ww))];
                int bx = x - screen.left, by = y - screen.top;
                if (bx < 0 || by < 0 || bx >= w || by >= h) continue;
                uint32_t& dst = bits[(size_t)by * w + bx];
                uint32_t out = 0xFF000000;
                for (int shift = 0; shift < 24; shift += 8) {
                    uint32_t a = (dst >> shift) & 0xFF, b = (p >> shift) & 0xFF;
                    out |= ((a * (255 - m) + b * m + 127) / 255) << shift;
                }
                dst = out;
            }
    }
}

// A node placed: where it is, and its children's places.
struct Laid {
    const Node* node = nullptr;
    std::string id;  // its identity: keys (or type and place) from the root
    RECT rc{};
    std::vector<Laid> kids;
    int scroll = 0, contentHeight = 0;  // a scroll container's
    HWND input = nullptr;               // an input's edit control
};

std::wstring GlyphText(const std::string& name);  // (below: the runtime's map, through a hook)

struct InputState {
    HWND edit = nullptr;
    bool seen = false;  // still in the tree after this render
    bool multiline = false, submitOnEnter = false;
};

// A key, with the modifiers held now, as the capture_keys chord that names it ("Shift+Tab" or "Backtab",
// "Return" or "Enter", "Ctrl+K"), the first one in the list that does; empty if none. In a text box, only keys
// a text box doesn't use itself.
std::string CapturedChord(const std::vector<std::string>& list, WPARAM vk, bool inTextBox) {
    if (list.empty() || vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU || vk == VK_LWIN || vk == VK_RWIN) return {};
    auto lower = [](std::string t) {
        for (auto& c : t) c = (char)tolower((unsigned char)c);
        return t;
    };
    bool ctrl = GetKeyState(VK_CONTROL) < 0, alt = GetKeyState(VK_MENU) < 0, shift = GetKeyState(VK_SHIFT) < 0,
         super = GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0;
    std::vector<std::string> names;  // what this key may be called
    if (vk >= 'A' && vk <= 'Z') names.push_back(std::string(1, (char)tolower((int)vk)));
    else if (vk >= '0' && vk <= '9') names.push_back(std::string(1, (char)vk));
    else if (vk >= VK_F1 && vk <= VK_F24) names.push_back("f" + std::to_string(vk - VK_F1 + 1));
    else
        switch (vk) {
            case VK_TAB: names = {"tab"}; break;
            case VK_RETURN: names = {"return", "enter", "kp_enter"}; break;
            case VK_SPACE: names = {"space"}; break;
            case VK_ESCAPE: names = {"escape", "esc"}; break;
            case VK_UP: names = {"up"}; break;
            case VK_DOWN: names = {"down"}; break;
            case VK_LEFT: names = {"left"}; break;
            case VK_RIGHT: names = {"right"}; break;
            case VK_BACK: names = {"backspace"}; break;
            case VK_DELETE: names = {"delete", "del"}; break;
            case VK_INSERT: names = {"insert"}; break;
            case VK_HOME: names = {"home"}; break;
            case VK_END: names = {"end"}; break;
            case VK_PRIOR: names = {"page_up", "pageup", "prior"}; break;
            case VK_NEXT: names = {"page_down", "pagedown", "next"}; break;
            default: return {};
        }
    if (inTextBox) {
        bool editing = ctrl && !alt && (vk == 'A' || vk == 'C' || vk == 'V' || vk == 'X' || vk == 'Z' || vk == 'Y' ||
                                         vk == VK_LEFT || vk == VK_RIGHT || vk == VK_BACK || vk == VK_DELETE || vk == VK_HOME || vk == VK_END);
        bool free = vk == VK_ESCAPE || vk == VK_TAB || vk == VK_UP || vk == VK_DOWN || vk == VK_PRIOR || vk == VK_NEXT ||
                    (vk >= VK_F1 && vk <= VK_F24) || ((ctrl || alt) && !editing);
        if (!free) return {};
    }
    for (auto const& chord : list) {
        std::vector<std::string> parts;
        std::string all = lower(chord);
        for (size_t at = 0; at <= all.size();) {
            size_t plus = all.find('+', at + 1);  // ("Ctrl++" ends with the key "+")
            if (plus == std::string::npos) plus = all.size();
            parts.push_back(all.substr(at, plus - at));
            at = plus + 1;
        }
        if (parts.empty()) continue;
        std::string key = parts.back();
        bool wantCtrl = false, wantAlt = false, wantShift = false, wantSuper = false, known = true;
        for (size_t i = 0; i + 1 < parts.size(); i++) {
            const std::string& m = parts[i];
            if (m == "ctrl" || m == "control") wantCtrl = true;
            else if (m == "alt") wantAlt = true;
            else if (m == "shift") wantShift = true;
            else if (m == "super" || m == "win" || m == "meta" || m == "logo" || m == "mod4") wantSuper = true;
            else known = false;
        }
        if (key == "backtab") key = "tab", wantShift = true;  // (Shift+Tab's own name)
        if (!known || wantCtrl != ctrl || wantAlt != alt || wantShift != shift || wantSuper != super) continue;
        for (auto const& n : names)
            if (n == key) return chord;
    }
    return {};
}

class PanelWindow final : public Window {
public:
    PanelWindow(const Placement& where, Host host) : host_(std::move(host)) { Create(where); }
    ~PanelWindow() override {
        if (backdrop_) DestroyWindow(backdrop_);
        for (auto& [id, in] : inputs_)
            if (in.edit) DestroyWindow(in.edit);
        if (hwnd_) {
            SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);  // (it paints no more: a closing move shows what it last drew)
            if (desktop_) {
                std::lock_guard lock(g_depthLock);
                g_desktopWindows.erase(hwnd_);
            }
            if (desktop_ || !IsWindowVisible(hwnd_) || !PanelMoveStart(hwnd_, true, slideSide_, Px(24))) DestroyWindow(hwnd_);
        }
        if (brush_) DeleteObject(brush_);
        for (auto& [path, p] : pictures_)
            if (p.drawn) DeleteObject(p.drawn);
    }

    void Render(Node tree) override {
        tree_ = std::move(tree);
        Relayout();
        if (desktop_) FitToContent();
    }

    const Handler* Find(const std::string& identity, const std::string& handler) const override {
        const Laid* l = FindLaid(root_, identity);
        if (!l) return nullptr;
        auto it = l->node->on.find(handler);
        return it != l->node->on.end() && it->second.Set() ? &it->second : nullptr;
    }

    std::vector<int> Refs() const override {
        std::vector<int> out;
        std::function<void(const Node&)> walk = [&](const Node& n) {
            for (auto const& [k, h] : n.on)
                if (h.ref >= 0) out.push_back(h.ref);
            for (auto const& c : n.children) walk(c);
        };
        walk(tree_);
        return out;
    }

    HWND Hwnd() const override { return hwnd_; }

    bool InsertText(const std::string& key, const std::string& text, bool ownLine) override {
        HWND edit = nullptr;
        std::function<void(const Laid&)> walk = [&](const Laid& l) {
            if (!edit && l.node && l.node->type == "input" && Str(*l.node, "key") == key) edit = l.input;
            for (auto const& k : l.kids) walk(k);
        };
        walk(root_);
        if (!edit) return false;
        bool multiline = GetWindowLongW(edit, GWL_STYLE) & ES_MULTILINE;
        std::wstring typed;
        for (wchar_t c : W(text)) {
            if (c == L'\r') continue;
            if (c == L'\n') {
                typed += multiline ? L"\r\n" : L" ";
                continue;
            }
            typed += c;
        }
        if (ownLine && multiline) {  // (a line break before and after it, where there isn't one)
            DWORD from = 0, to = 0;
            SendMessageW(edit, EM_GETSEL, (WPARAM)&from, (LPARAM)&to);
            int n = GetWindowTextLengthW(edit);
            std::wstring all(n, L'\0');
            GetWindowTextW(edit, all.data(), n + 1);
            if (from > 0 && from <= all.size() && all[from - 1] != L'\n') typed = L"\r\n" + typed;
            if (to >= all.size() || all[to] != L'\r') typed += L"\r\n";
        }
        SetFocus(edit);
        SendMessageW(edit, EM_REPLACESEL, TRUE, (LPARAM)typed.c_str());
        if (GetFocus() != edit) {  // (EN_CHANGE told the plugin only if it has the focus)
            std::string id = InputId(edit);
            if (!id.empty()) host_.event(id, "onChange", {EditText(edit)});
        }
        return true;
    }

private:
    Host host_;
    HWND hwnd_ = nullptr;
    Node tree_;
    Node scrolled_;  // tree_ in a scroll container, when it's taller than the panel and has none of its own
    Laid root_;
    Fonts fonts_;
    int s_ = 1;  // whole-number scale, as the flyouts use
    std::map<std::string, InputState> inputs_;
    std::map<std::wstring, Picture> pictures_;  // by path
    std::string hover_, pressed_, hoverReported_;
    std::string dragging_;  // a slider being dragged
    HBRUSH brush_ = nullptr;  // the inputs' background
    COLORREF inputInk_ = 0, inputGround_ = 0;
    bool tracking_ = false;
    // The strip along its top: drag it to move the panel, its pin keeps it open after a click elsewhere.
    std::wstring remember_;  // where its size, place and pin are kept (Placement::remember)
    bool pinned_ = false, pinHover_ = false, pinPressed_ = false;
    RECT sizing_{};  // the window when a move or resize began
    POINT slideSide_{0, 1};  // the way it slides in from and out to (its bar's side)
    // A desktop widget's: where it goes the first time (its top right corner) and the most it's wide.
    bool desktop_ = false, fitting_ = false;
    POINT corner_{};
    int widest_ = 0;
    std::vector<std::string> captureKeys_;  // (Placement::captureKeys)
    // A fullscreen panel's: over a blurred, dimmed copy of the screen (backdrop_).
    bool fullscreen_ = false;
    HWND backdrop_ = nullptr;
    bool swallowChar_ = false;

    // A capture_keys chord pressed or released: to the plugin, and nothing else happens with it.
    bool Captured(UINT msg, WPARAM wp, bool inTextBox) {
        if (msg != WM_KEYDOWN && msg != WM_KEYUP && msg != WM_SYSKEYDOWN && msg != WM_SYSKEYUP) return false;
        std::string chord = CapturedChord(captureKeys_, wp, inTextBox);
        if (chord.empty() || !host_.key) return false;
        bool pressed = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
        host_.key(chord, pressed);
        if (pressed) swallowChar_ = true;  // (the character the key makes, already on its way, isn't typed)
        return true;
    }

    int Strip() const { return desktop_ || fullscreen_ ? 0 : Px(24); }
    RECT PinRect() const {
        if (!Strip()) return {};
        RECT c;
        GetClientRect(hwnd_, &c);
        return {c.right - Px(28), Px(2), c.right - Px(6), Px(22)};
    }
    // Kept per panel in Tourne Desktop's values (HKCU\Software\Tourne\Desktop\Values\plugins).
    int Kept(const wchar_t* what, int fallback) const {
        return remember_.empty() ? fallback : GetIntValue(L"plugins", (remember_ + L"." + what).c_str(), fallback);
    }
    void Keep(const wchar_t* what, int value) const {
        if (!remember_.empty()) SetIntValue(L"plugins", (remember_ + L"." + what).c_str(), value);
    }
    void Forget(const wchar_t* what) const {
        if (!remember_.empty()) DeleteValue(L"plugins", (remember_ + L"." + what).c_str());
    }

    int Px(double v) const { return (int)std::lround(v * s_); }
    COLORREF Role(const std::string& role, COLORREF fallback) const {
        if (role.empty()) return fallback;
        COLORREF c = host_.color(role);
        return c == CLR_INVALID ? fallback : c;
    }
    COLORREF Ground() const { return Role("surface", RGB(0x12, 0x1C, 0x21)); }
    COLORREF Raised() const { return Role("surface_variant", RGB(0x1B, 0x2A, 0x31)); }
    COLORREF Ink() const { return Role("on_surface", RGB(0x66, 0xEA, 0x97)); }
    COLORREF Muted() const { return Role("on_surface_variant", RGB(0x64, 0xAA, 0x89)); }
    COLORREF Accent() const { return Role("primary", RGB(0x15, 0xFF, 0xA2)); }
    COLORREF Edge() const { return Role("outline", RGB(0x4F, 0x8F, 0x6A)); }
    COLORREF Line() const { return Role("outline_variant", RGB(0x1E, 0x2F, 0x36)); }
    COLORREF Hover() const { return Role("hover", RGB(0x1F, 0x3D, 0x34)); }
    COLORREF Warn() const { return Role("error", RGB(0xFF, 0x6F, 0x77)); }

    // The window.
    void Create(const Placement& where) {
        static bool registered = [] {
            WNDCLASSW wc{};
            wc.lpfnWndProc = Proc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = kClass;
            wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            return RegisterClassW(&wc) != 0;
        }();
        (void)registered;
        remember_ = where.remember;
        captureKeys_ = where.captureKeys;
        if (where.desktop) return CreateOnDesktop(where);
        if (where.placement == "fullscreen") return CreateFullscreen(where);
        pinned_ = Kept(L"panelPinned", 0) != 0;
        // Where the user last moved it to (screen pixels), if they did: its screen, else the pointer's.
        bool placed = Kept(L"panelPlaced", 0) != 0;
        POINT at = placed ? POINT{Kept(L"panelX", 0), Kept(L"panelY", 0)} : where.anchor;
        HMONITOR monitor = MonitorFromPoint(at, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(monitor, &mi);
        UINT dpiX = 96, dpiY = 96;
        using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
        if (auto f = (GetDpiForMonitor_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor")) f(monitor, 0, &dpiX, &dpiY);
        s_ = std::max(1, (int)std::lround(dpiX / 96.0));
        fonts_.scale = s_;
        RECT work = mi.rcWork, screen = mi.rcMonitor;
        int margin = Px(8);
        // The size the user last gave it (in 96-dpi pixels), else the manifest's.
        int keptW = Kept(L"panelWidth", 0), keptH = Kept(L"panelHeight", 0);
        int w = std::min(Px(keptW > 0 ? keptW : where.width), (int)(work.right - work.left) - 2 * margin);
        int h = where.fillHeight && keptH <= 0 ? (int)(work.bottom - work.top) - 2 * margin
                                               : std::min(Px(keptH > 0 ? keptH : where.height), (int)(work.bottom - work.top) - 2 * margin);
        int x, y;
        slideSide_ = {0, 1};  // (a slide comes from below, or from its bar's side)
        if (placed) {
            x = at.x, y = at.y;
        } else if (where.placement == "floating") {
            const std::string& p = where.position;
            bool left = p.find("left") != std::string::npos, right = p.find("right") != std::string::npos;
            bool top = p.rfind("top", 0) == 0, bottom = p.rfind("bottom", 0) == 0;
            x = left ? work.left + margin : right ? work.right - margin - w : (work.left + work.right - w) / 2;
            y = top ? work.top + margin : bottom ? work.bottom - margin - h : (work.top + work.bottom - h) / 2;
        } else {
            // Attached: beside the bar it came from, at the pointer, kept on the screen.
            x = where.anchor.x - w / 2;
            if (work.bottom < screen.bottom) y = work.bottom - margin - h;         // a bar at the bottom
            else if (work.top > screen.top) y = work.top + margin, slideSide_ = {0, -1};  // at the top
            else y = std::clamp((int)where.anchor.y - h / 2, (int)work.top + margin, (int)work.bottom - margin - h);
            if (work.left > screen.left) x = work.left + margin, slideSide_ = {-1, 0};    // at the left
            if (work.right < screen.right) x = work.right - margin - w, slideSide_ = {1, 0};  // at the right
        }
        x = std::clamp(x, (int)work.left + margin, std::max((int)work.left + margin, (int)work.right - margin - w));
        y = std::clamp(y, (int)work.top + margin, std::max((int)work.top + margin, (int)work.bottom - margin - h));
        hwnd_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kClass, L"Tourne plugin panel", WS_POPUP | WS_CLIPCHILDREN, x, y, w, h,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!hwnd_) return;
        SetWindowLongPtrW(hwnd_, GWLP_USERDATA, (LONG_PTR)this);
        DragAcceptFiles(hwnd_, TRUE);  // (files dropped on a node with onDrop)
        DWORD corners = 1;  // DWMWCP_DONOTROUND: square, like the flyouts
        DwmSetWindowAttribute(hwnd_, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
        PanelMoveStart(hwnd_, false, slideSide_, Px(24));
        ShowWindow(hwnd_, SW_SHOW);
        SetForegroundWindow(hwnd_);
        SetFocus(hwnd_);
    }

    // A desktop widget: owned by the desktop window (Explorer's or Tourne's Progman), so it's always just above it
    // and under every other window, and Show desktop leaves it there. It never takes the focus.
    void CreateOnDesktop(const Placement& where) {
        desktop_ = true;
        pinned_ = true;  // (never closes on a click elsewhere)
        corner_ = where.anchor;
        bool placed = Kept(L"panelPlaced", 0) != 0;
        POINT at = placed ? POINT{Kept(L"panelX", 0), Kept(L"panelY", 0)} : corner_;
        HMONITOR monitor = MonitorFromPoint(at, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(monitor, &mi);
        UINT dpiX = 96, dpiY = 96;
        using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
        if (auto f = (GetDpiForMonitor_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor")) f(monitor, 0, &dpiX, &dpiY);
        s_ = std::max(1, (int)std::lround(dpiX / 96.0));
        fonts_.scale = s_;
        widest_ = Px(where.width > 0 ? where.width : 420);
        int w = Px(160), h = Px(80);  // (until it renders: then as big as what it shows)
        int x = placed ? at.x : at.x - w, y = at.y;
        RECT work = mi.rcWork;
        x = std::clamp(x, (int)work.left, std::max((int)work.left, (int)work.right - w));
        y = std::clamp(y, (int)work.top, std::max((int)work.top, (int)work.bottom - h));
        HWND owner = GetShellWindow();
        if (!owner) owner = FindWindowW(L"Progman", nullptr);
        hwnd_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kClass, L"Tourne desktop widget", WS_POPUP | WS_CLIPCHILDREN, x, y, w, h,
                                owner, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!hwnd_) return;
        SetWindowLongPtrW(hwnd_, GWLP_USERDATA, (LONG_PTR)this);
        DragAcceptFiles(hwnd_, TRUE);
        DWORD corners = 1;
        DwmSetWindowAttribute(hwnd_, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
        SetWindowPos(hwnd_, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        std::lock_guard lock(g_depthLock);
        g_desktopWindows.insert(hwnd_);
    }

    // Fullscreen: the screen under the pointer, as it is now, blurred and dimmed behind it; the panel in the middle.
    static LRESULT CALLBACK BackdropProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(wnd, &ps);
                if (auto shot = (HBITMAP)GetPropW(wnd, L"TourneShot")) {
                    HDC mem = CreateCompatibleDC(dc);
                    HGDIOBJ old = SelectObject(mem, shot);
                    RECT c;
                    GetClientRect(wnd, &c);
                    BitBlt(dc, 0, 0, c.right, c.bottom, mem, 0, 0, SRCCOPY);
                    SelectObject(mem, old);
                    DeleteDC(mem);
                }
                EndPaint(wnd, &ps);
                return 0;
            }
            case WM_ERASEBKGND: return 1;
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
                if (auto panel = (HWND)GetPropW(wnd, L"TournePanel")) PostMessageW(panel, WM_CLOSE, 0, 0);
                return 0;
            case WM_KEYDOWN:
                if (wp == VK_ESCAPE)
                    if (auto panel = (HWND)GetPropW(wnd, L"TournePanel")) PostMessageW(panel, WM_CLOSE, 0, 0);
                return 0;
            case WM_DESTROY:
                if (auto shot = (HBITMAP)RemovePropW(wnd, L"TourneShot")) DeleteObject(shot);
                RemovePropW(wnd, L"TournePanel");
                return 0;
        }
        return DefWindowProcW(wnd, msg, wp, lp);
    }
    void CreateFullscreen(const Placement& where) {
        fullscreen_ = true;
        static bool registered = [] {
            WNDCLASSW wc{};
            wc.lpfnWndProc = BackdropProc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = L"TourneDesktopPanelBackdrop";
            wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            return RegisterClassW(&wc) != 0;
        }();
        (void)registered;
        HMONITOR monitor = MonitorFromPoint(where.anchor, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(monitor, &mi);
        UINT dpiX = 96, dpiY = 96;
        using GetDpiForMonitor_t = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
        if (auto f = (GetDpiForMonitor_t)GetProcAddress(LoadLibraryW(L"shcore.dll"), "GetDpiForMonitor")) f(monitor, 0, &dpiX, &dpiY);
        s_ = std::max(1, (int)std::lround(dpiX / 96.0));
        fonts_.scale = s_;
        RECT screen = mi.rcMonitor;
        int sw = screen.right - screen.left, sh = screen.bottom - screen.top;
        // Blurred: shrunk to a sixteenth and grown back, both smoothly; then darkened to about 45%.
        HDC desk = GetDC(nullptr), big = CreateCompatibleDC(desk), small = CreateCompatibleDC(desk);
        HBITMAP shot = CreateCompatibleBitmap(desk, sw, sh), tiny = CreateCompatibleBitmap(desk, std::max(1, sw / 16), std::max(1, sh / 16));
        HGDIOBJ oldBig = SelectObject(big, shot), oldSmall = SelectObject(small, tiny);
        BitBlt(big, 0, 0, sw, sh, desk, screen.left, screen.top, SRCCOPY | CAPTUREBLT);
        SetStretchBltMode(small, HALFTONE), SetBrushOrgEx(small, 0, 0, nullptr);
        StretchBlt(small, 0, 0, std::max(1, sw / 16), std::max(1, sh / 16), big, 0, 0, sw, sh, SRCCOPY);
        SetStretchBltMode(big, HALFTONE), SetBrushOrgEx(big, 0, 0, nullptr);
        StretchBlt(big, 0, 0, sw, sh, small, 0, 0, std::max(1, sw / 16), std::max(1, sh / 16), SRCCOPY);
        HDC black = CreateCompatibleDC(desk);
        HBITMAP dot = CreateCompatibleBitmap(desk, 1, 1);
        HGDIOBJ oldBlack = SelectObject(black, dot);
        SetPixel(black, 0, 0, RGB(0, 0, 0));
        BLENDFUNCTION dim{AC_SRC_OVER, 0, 140, 0};
        AlphaBlend(big, 0, 0, sw, sh, black, 0, 0, 1, 1, dim);
        SelectObject(black, oldBlack), DeleteObject(dot), DeleteDC(black);
        SelectObject(small, oldSmall), DeleteObject(tiny), DeleteDC(small);
        SelectObject(big, oldBig), DeleteDC(big);
        ReleaseDC(nullptr, desk);
        backdrop_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"TourneDesktopPanelBackdrop", L"Tourne plugin backdrop", WS_POPUP,
                                    screen.left, screen.top, sw, sh, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (backdrop_) {
            SetPropW(backdrop_, L"TourneShot", shot);
            ShowWindow(backdrop_, SW_SHOWNOACTIVATE);
        } else {
            DeleteObject(shot);
        }
        int w = std::min(Px(where.width), sw - Px(32)), h = std::min(where.fillHeight ? sh : Px(where.height), sh - Px(32));
        hwnd_ = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, kClass, L"Tourne plugin panel", WS_POPUP | WS_CLIPCHILDREN,
                                screen.left + (sw - w) / 2, screen.top + (sh - h) / 2, w, h, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!hwnd_) return;
        if (backdrop_) SetPropW(backdrop_, L"TournePanel", hwnd_);
        SetWindowLongPtrW(hwnd_, GWLP_USERDATA, (LONG_PTR)this);
        DragAcceptFiles(hwnd_, TRUE);
        DWORD corners = 2;  // DWMWCP_ROUND
        DwmSetWindowAttribute(hwnd_, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &corners, sizeof(corners));
        ShowWindow(hwnd_, SW_SHOW);
        SetForegroundWindow(hwnd_);
        if (GetForegroundWindow() != hwnd_) {
            HWND front = GetForegroundWindow();
            DWORD frontThread = front ? GetWindowThreadProcessId(front, nullptr) : 0, me = GetCurrentThreadId();
            bool attached = frontThread && frontThread != me && AttachThreadInput(me, frontThread, TRUE);
            SetForegroundWindow(hwnd_);
            if (attached) AttachThreadInput(me, frontThread, FALSE);
        }
        SetFocus(hwnd_);
    }
    // As big as what it shows (no wider than its manifest's width), its top right corner kept where it was the
    // first time, its top left once the user moved it.
    void FitToContent() {
        if (!hwnd_ || fitting_ || !root_.node) return;
        int pad = Px(12);
        Size want = Measure(tree_, widest_ - 2 * pad);
        RECT now, work;
        GetWindowRect(hwnd_, &now);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(MonitorFromRect(&now, MONITOR_DEFAULTTONEAREST), &mi);
        work = mi.rcWork;
        int w = std::clamp(want.w + 2 * pad, Px(40), std::max(Px(40), widest_));
        int h = std::clamp(want.h + 2 * pad, Px(24), std::max(Px(24), (int)(work.bottom - work.top)));
        if (w == now.right - now.left && h == now.bottom - now.top) return;
        int x = Kept(L"panelPlaced", 0) ? now.left : now.right - w;
        x = std::clamp(x, (int)work.left, std::max((int)work.left, (int)work.right - w));
        int y = std::clamp((int)now.top, (int)work.top, std::max((int)work.top, (int)work.bottom - h));
        fitting_ = true;
        SetWindowPos(hwnd_, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
        fitting_ = false;
        Relayout();
    }
    void DesktopMenu() {
        HMENU menu = CreatePopupMenu();
        AppendMenuW(menu, MF_STRING, 1, L"Back where it started");
        AppendMenuW(menu, MF_STRING, 2, L"Off the desktop");
        POINT at;
        GetCursorPos(&at);
        int picked = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, at.x, at.y, 0, hwnd_, nullptr);
        DestroyMenu(menu);
        if (picked == 1) {
            ForgetPlace();
            RECT now;
            GetWindowRect(hwnd_, &now);
            SetWindowPos(hwnd_, nullptr, corner_.x - (now.right - now.left), corner_.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        } else if (picked == 2) {
            host_.closed();
        }
    }

    // An image node's picture (its path, or source), read when first needed or changed; null if it doesn't read.
    Picture* PictureFor(const Node& n) {
        std::wstring path = W(Str(n, "path", Str(n, "source")));
        if (path.empty()) return nullptr;
        WIN32_FILE_ATTRIBUTE_DATA a{};
        bool exists = GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &a) && !(a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        auto it = pictures_.find(path);
        if (it != pictures_.end() && (!exists || CompareFileTime(&it->second.written, &a.ftLastWriteTime) == 0))
            return it->second.failed ? nullptr : &it->second;
        Picture& p = pictures_[path];
        if (p.drawn) DeleteObject(p.drawn), p.drawn = nullptr;
        p.written = a.ftLastWriteTime;
        p.failed = !exists || !ReadPicture(path, &p);
        return p.failed ? nullptr : &p;
    }
    // Where a picture goes in its place: its own size (at the panel's scale), smaller to fit, never stretched;
    // grow: an SVG as big as fits (it's sharp at any size).
    RECT PictureRect(const Picture& p, RECT r, bool grow = false) const {
        int w = std::max(1, (int)std::lround(p.nw * s_)), h = std::max(1, (int)std::lround(p.nh * s_)), roomW = r.right - r.left, roomH = r.bottom - r.top;
        if (grow && p.vector && roomW > 0 && roomH > 0) {
            double k = std::min((double)roomW / w, (double)roomH / h);
            w = std::max(1, (int)std::lround(w * k)), h = std::max(1, (int)std::lround(h * k));
        }
        if (w > roomW && roomW > 0) h = (int)((long long)h * roomW / w), w = roomW;
        if (h > roomH && roomH > 0) w = (int)((long long)w * roomH / h), h = roomH;
        return {r.left, r.top, r.left + std::max(1, w), r.top + std::max(1, h)};
    }
    std::wstring PictureMissing(const Node& n) const {
        std::wstring alt = W(Str(n, "alt"));
        return L"[picture" + (alt.empty() ? L"" : L": " + alt) + L"]";
    }

    // Layout: each node's natural size for a width, then places.
    struct Size {
        int w = 0, h = 0;
    };

    int Pad(const Node& n, bool horizontal) const {
        double all = Num(n, "padding", 0);
        return Px(Num(n, horizontal ? "paddingH" : "paddingV", all));
    }
    int ControlHeight(const Node& n) const {
        std::string size = Str(n, "controlSize", "md");
        return Px(size == "sm" ? 28 : size == "lg" ? 38 : 32);
    }
    HFONT TextFont(const Node& n) {
        std::string weight = Str(n, "fontWeight");
        return fonts_.Text(Num(n, "fontSize", 16), weight == "bold" || weight == "semibold" || weight == "heavy", Str(n, "fontFamily"));
    }
    std::wstring LabelText(const Node& n) const {
        std::wstring t = W(Str(n, "text"));
        if (n.type == "markdown") {  // (read as plain text: its marks dropped)
            std::wstring out;
            for (size_t i = 0; i < t.size(); i++) {
                bool lineStart = i == 0 || t[i - 1] == L'\n';
                if (lineStart)
                    while (i < t.size() && (t[i] == L'#' || (t[i] == L'>' ))) i++;
                if (i < t.size() && (t[i] == L'*' || t[i] == L'`' || t[i] == L'_')) continue;
                if (i < t.size()) out += t[i];
            }
            t = out;
        }
        return t;
    }
    Size Text(HFONT font, const std::wstring& text, int maxW, int maxLines) {
        HDC dc = GetDC(hwnd_);
        HGDIOBJ old = SelectObject(dc, font);
        TEXTMETRICW tm{};
        GetTextMetricsW(dc, &tm);
        RECT r{0, 0, std::max(1, maxW), 0};
        Size out;
        if (maxLines == 1 || text.empty()) {
            SIZE e{};
            GetTextExtentPoint32W(dc, text.c_str(), (int)text.size(), &e);
            out = {std::min((int)e.cx, std::max(0, maxW)), (int)tm.tmHeight};
        } else {
            DrawTextW(dc, text.c_str(), (int)text.size(), &r, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
            int h = r.bottom;
            if (maxLines > 0) h = std::min(h, maxLines * (int)tm.tmHeight);
            out = {(int)std::min(r.right, (LONG)maxW), h};
        }
        SelectObject(dc, old);
        ReleaseDC(hwnd_, dc);
        return out;
    }
    int GlyphSize(const Node& n, const char* prop = "size") const { return Px(Num(n, prop, 16)); }

    Size Measure(const Node& n, int maxW) {
        if (!Bool(n, "visible", true)) return {};
        Size s;
        const std::string& t = n.type;
        int padH = Pad(n, true), padV = Pad(n, false);
        if (Container(t)) {
            int gap = Px(Num(n, "gap", 0)), inner = std::max(0, maxW - 2 * padH);
            bool row = Across(n);
            int count = 0, fixed = 0, flexes = 0;
            if (row)
                for (auto const& c : n.children)
                    if (Bool(c, "visible", true)) {
                        if (Num(c, "flexGrow", c.type == "spacer" ? 1 : 0) > 0) flexes++;
                        else fixed += Measure(c, inner).w;
                    }
            for (auto const& c : n.children) {
                if (!Bool(c, "visible", true)) continue;
                bool flex = Num(c, "flexGrow", c.type == "spacer" ? 1 : 0) > 0;
                int avail = row && flex ? std::max(0, (inner - fixed) / std::max(1, flexes)) : inner;
                Size k = Measure(c, avail);
                if (row) s.w += k.w, s.h = std::max(s.h, k.h);
                else s.h += k.h, s.w = std::max(s.w, k.w);
                count++;
            }
            if (count > 1) (row ? s.w : s.h) += gap * (count - 1);
            s.w += 2 * padH, s.h += 2 * padV;
            s.w = std::max(s.w, Px(Num(n, "minWidth", 0)));
            s.h = std::max(s.h, Px(Num(n, "minHeight", 0)));
        } else if (t == "label" || t == "markdown") {
            int cap = Has(n, "maxWidth") ? std::min(maxW, Px(Num(n, "maxWidth", 0))) : maxW;
            s = Text(TextFont(n), LabelText(n), cap, (int)Num(n, "maxLines", t == "markdown" ? 0 : 0));
        } else if (t == "glyph") {
            s.w = s.h = GlyphSize(n);
        } else if (t == "button") {
            int h = ControlHeight(n);
            std::wstring text = W(Str(n, "text"));
            int gw = Str(n, "glyph").empty() ? 0 : GlyphSize(n, "glyphSize");
            int tw = text.empty() ? 0 : Text(fonts_.Text(Num(n, "fontSize", 16), false), text, 10000, 1).w;
            s.h = h;
            s.w = text.empty() ? std::max(h, gw + Px(12)) : Px(12) * 2 + gw + (gw ? Px(6) : 0) + tw;
        } else if (t == "input") {
            s.h = Bool(n, "multiline", false) ? Px(120) : ControlHeight(n);
            s.w = Px(160);
        } else if (t == "select") {
            s.h = ControlHeight(n), s.w = Px(160);
        } else if (t == "toggle") {
            s.w = Px(36), s.h = Px(20);
        } else if (t == "slider") {
            s.w = Px(140), s.h = Px(20);
        } else if (t == "progress") {
            s.w = Px(100), s.h = Px(6);
        } else if (t == "separator") {
            int thick = std::max(1, Px(Num(n, "thickness", 1))), spacing = Px(Num(n, "spacing", 4));
            s.w = s.h = thick + 2 * spacing;  // (its length is the parent's: it stretches)
        } else if (t == "graph") {
            s.w = Px(120), s.h = Px(40);
        } else if (t == "image") {
            s.w = s.h = Px(48);
            int cap = Has(n, "maxWidth") ? std::min(maxW, Px(Num(n, "maxWidth", 0))) : maxW;
            if (Picture* p = PictureFor(n)) {
                RECT fit = PictureRect(*p, {0, 0, std::max(1, cap), Has(n, "maxHeight") ? Px(Num(n, "maxHeight", 0)) : 1 << 20});
                s.w = fit.right, s.h = fit.bottom;
            } else if (Has(n, "path") || Has(n, "source")) {
                s = Text(fonts_.Text(13, false), PictureMissing(n), cap, 0);
            }
        }
        if (Has(n, "width")) s.w = Px(Num(n, "width", 0));
        if (Has(n, "height")) s.h = Px(Num(n, "height", 0));
        return s;
    }

    void Place(const Node& n, const std::string& id, RECT rc, Laid* out) {
        out->node = &n;
        out->id = id;
        out->rc = rc;
        out->kids.clear();
        if (!Container(n.type)) return;
        bool row = Across(n);
        int padH = Pad(n, true), padV = Pad(n, false), gap = Px(Num(n, "gap", 0));
        RECT inner{rc.left + padH, rc.top + padV, rc.right - padH, rc.bottom - padV};
        int innerW = std::max(0, (int)(inner.right - inner.left)), innerH = std::max(0, (int)(inner.bottom - inner.top));
        std::string align = Str(n, "align", "stretch"), justify = Str(n, "justify", "start");
        struct Kid {
            const Node* node;
            std::string id;
            Size size;
            double grow;
        };
        std::vector<Kid> kids;
        std::map<std::string, int> seen;
        int index = 0;
        for (auto const& c : n.children) {
            index++;
            if (!Bool(c, "visible", true)) continue;
            std::string key = Str(c, "key");
            std::string part = key.empty() ? c.type + "#" + std::to_string(index) : "k:" + key;
            if (int dup = seen[part]++) part += "~" + std::to_string(dup);
            kids.push_back({&c, id + "/" + part, {}, Num(c, "flexGrow", c.type == "spacer" ? 1 : 0)});
        }
        // Main axis: natural sizes, then what's over (or short) goes to the growing ones.
        int fixed = 0;
        double grow = 0;
        for (auto& k : kids)
            if (k.grow <= 0) {
                k.size = Measure(*k.node, innerW);
                fixed += row ? k.size.w : k.size.h;
            } else {
                grow += k.grow;
            }
        int gaps = kids.size() > 1 ? gap * (int)(kids.size() - 1) : 0;
        int room = (row ? innerW : innerH) - fixed - gaps;
        bool scroll = n.type == "scroll";
        for (auto& k : kids) {
            if (k.grow <= 0) continue;
            int share = grow > 0 ? (int)(std::max(0, room) * k.grow / grow) : 0;
            k.size = Measure(*k.node, row ? std::max(0, share) : innerW);
            if (row) k.size.w = std::max(share, Has(*k.node, "width") ? k.size.w : 0);
            else if (!scroll) k.size.h = std::max(share, Has(*k.node, "height") ? k.size.h : 0);
        }
        int used = gaps;
        for (auto& k : kids) used += row ? k.size.w : k.size.h;
        int start = row ? inner.left : inner.top, extra = (row ? innerW : innerH) - used, between = gap;
        if (extra > 0 && grow <= 0) {
            if (justify == "center") start += extra / 2;
            else if (justify == "end") start += extra;
            else if (justify == "space_between" && kids.size() > 1) between += extra / (int)(kids.size() - 1);
        }
        int contentEnd = start;
        int offset = scroll ? -out->scroll : 0;
        for (auto& k : kids) {
            Laid child;
            RECT r;
            int along = row ? k.size.w : k.size.h;
            int crossSize = row ? k.size.h : k.size.w, crossRoom = row ? innerH : innerW;
            bool stretch = align == "stretch" && !(row ? Has(*k.node, "height") : Has(*k.node, "width"));
            if (stretch) crossSize = crossRoom;
            crossSize = std::min(crossSize, crossRoom);
            int crossAt = align == "center" ? (crossRoom - crossSize) / 2 : align == "end" ? crossRoom - crossSize : 0;
            if (row) r = {start, inner.top + crossAt, start + along, inner.top + crossAt + crossSize};
            else r = {inner.left + crossAt, start + offset, inner.left + crossAt + crossSize, start + offset + along};
            Place(*k.node, k.id, r, &child);
            out->kids.push_back(std::move(child));
            start += along + between;
            contentEnd = start - between;
        }
        if (scroll) out->contentHeight = contentEnd - inner.top + padV;
    }

    void Relayout() {
        if (!hwnd_) return;
        RECT client;
        GetClientRect(hwnd_, &client);
        int pad = Px(12);
        RECT area{pad, Strip(), client.right - pad, client.bottom - pad};
        // Scroll offsets survive a render (by identity). (The old places' nodes are the old tree's,
        // already gone: only their ids and offsets are read, and only scroll containers have an offset.)
        std::map<std::string, int> scrolls;
        std::function<void(const Laid&)> keep = [&](const Laid& l) {
            if (l.scroll) scrolls[l.id] = l.scroll;
            for (auto const& k : l.kids) keep(k);
        };
        keep(root_);
        // Taller than the panel with nothing that scrolls: the whole of it scrolls, so nothing is cut off.
        const Node* top = &tree_;
        std::function<bool(const Node&)> scrolls_in = [&](const Node& n) {
            if (n.type == "scroll") return true;
            for (auto const& c : n.children)
                if (scrolls_in(c)) return true;
            return false;
        };
        if (!desktop_ && !scrolls_in(tree_) && Measure(tree_, area.right - area.left).h > area.bottom - area.top) {
            scrolled_ = Node{};
            scrolled_.type = "scroll";
            scrolled_.children = {tree_};
            top = &scrolled_;
        }
        Laid fresh;
        PlaceWithScrolls(*top, "", area, &fresh, scrolls);
        root_ = std::move(fresh);
        SyncInputs();
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
    void PlaceWithScrolls(const Node& n, const std::string& id, RECT rc, Laid* out, std::map<std::string, int> const& scrolls) {
        // Place once to learn the identities and content heights, then again with each scroll's offset (clamped).
        Place(n, id, rc, out);
        bool any = false;
        std::function<void(Laid&)> set = [&](Laid& l) {
            if (l.node->type == "scroll") {
                auto it = scrolls.find(l.id);
                int viewport = l.rc.bottom - l.rc.top;
                int most = std::max(0, l.contentHeight - viewport);
                int want = it != scrolls.end() ? it->second : 0;
                if (Bool(*l.node, "stickToBottom", false) || Has(*l.node, "scrollToBottomRev")) want = most;
                l.scroll = std::clamp(want, 0, most);
                if (l.scroll) any = true;
            }
            for (auto& k : l.kids) set(k);
        };
        set(*out);
        if (any) Replace(*out);
    }
    // Places a scroll container's children again at its offset.
    void Replace(Laid& l) {
        if (l.node->type == "scroll" && l.scroll) {
            int scroll = l.scroll;
            std::map<std::string, int> inner;
            std::function<void(const Laid&)> keep = [&](const Laid& x) {
                if (x.node->type == "scroll") inner[x.id] = x.scroll;
                for (auto const& k : x.kids) keep(k);
            };
            keep(l);
            l.scroll = scroll;
            Place(*l.node, l.id, l.rc, &l);
            std::function<void(Laid&)> restore = [&](Laid& x) {
                if (&x != &l && x.node->type == "scroll") x.scroll = inner[x.id];
                for (auto& k : x.kids) restore(k);
            };
            restore(l);
        }
        for (auto& k : l.kids) Replace(k);
    }

    static const Laid* FindLaid(const Laid& l, const std::string& id) {
        if (l.id == id && l.node) return &l;
        for (auto const& k : l.kids)
            if (auto f = FindLaid(k, id)) return f;
        return nullptr;
    }

    // Inputs: an edit control each, kept by identity so typing survives a render ("uncontrolled").
    void SyncInputs() {
        for (auto& [id, in] : inputs_) in.seen = false;
        std::function<void(Laid&, RECT)> walk = [&](Laid& l, RECT clip) {
            if (l.node->type == "scroll") IntersectRect(&clip, &clip, &l.rc);
            if (l.node->type == "input") {
                InputState& in = inputs_[l.id];
                in.seen = true;
                bool multiline = Bool(*l.node, "multiline", false);
                if (in.edit && in.multiline != multiline) DestroyWindow(in.edit), in.edit = nullptr;
                if (!in.edit) {
                    // (A multiline input wraps its lines at its width: no ES_AUTOHSCROLL.)
                    DWORD style = WS_CHILD | (multiline ? ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL : ES_AUTOHSCROLL) |
                                  (Bool(*l.node, "password", false) && !multiline ? ES_PASSWORD : 0);
                    in.edit = CreateWindowExW(0, L"EDIT", L"", style, 0, 0, 0, 0, hwnd_, nullptr, GetModuleHandleW(nullptr), nullptr);
                    in.multiline = multiline;
                    SetWindowTheme(in.edit, L"DarkMode_Explorer", nullptr);  // (a dark scroll bar, not Windows' light one)
                    SendMessageW(in.edit, WM_SETFONT, (WPARAM)fonts_.Text(Num(*l.node, "fontSize", 16), false, Str(*l.node, "fontFamily")), FALSE);
                    std::wstring value = W(Str(*l.node, "value"));
                    if (multiline) {  // (the edit control wants \r\n)
                        std::wstring crlf;
                        for (wchar_t c : value) {
                            if (c == L'\n' && (crlf.empty() || crlf.back() != L'\r')) crlf += L'\r';
                            crlf += c;
                        }
                        value = crlf;
                    }
                    SetWindowTextW(in.edit, value.c_str());
                    if (multiline) SendMessageW(in.edit, EM_SETSEL, value.size(), value.size());  // (the caret at the end, to carry on)
                    std::wstring cue = W(Str(*l.node, "placeholder"));
                    if (!cue.empty() && !multiline) SendMessageW(in.edit, EM_SETCUEBANNER, FALSE, (LPARAM)cue.c_str());
                    SetWindowSubclass(in.edit, InputProc, kInputSubclass, (DWORD_PTR)this);
                    SetWindowLongPtrW(in.edit, GWLP_ID, 0);
                    if (Bool(*l.node, "focus", false)) SetFocus(in.edit);
                }
                in.submitOnEnter = Bool(*l.node, "submitOnEnter", false);
                EnableWindow(in.edit, Bool(*l.node, "enabled", true));
                l.input = in.edit;
                RECT r = l.rc;
                InflateRect(&r, -Px(6), -Px(multiline ? 6 : 0));
                if (!multiline) {  // a line's height, centred
                    int lineH = Text(fonts_.Text(Num(*l.node, "fontSize", 16), false, Str(*l.node, "fontFamily")), L"Ag", 1000, 1).h;
                    r.top = l.rc.top + (l.rc.bottom - l.rc.top - lineH) / 2, r.bottom = r.top + lineH;
                }
                RECT visible;
                bool shown = IntersectRect(&visible, &r, &clip) && EqualRect(&visible, &r);
                SetWindowPos(in.edit, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
                ShowWindow(in.edit, shown ? SW_SHOWNA : SW_HIDE);
            }
            for (auto& k : l.kids) walk(k, clip);
        };
        RECT all{-100000, -100000, 100000, 100000};
        if (root_.node) walk(root_, all);
        for (auto it = inputs_.begin(); it != inputs_.end();) {
            if (it->second.seen) {
                ++it;
                continue;
            }
            if (it->second.edit) DestroyWindow(it->second.edit);
            it = inputs_.erase(it);
        }
    }
    std::string InputId(HWND edit) const {
        for (auto const& [id, in] : inputs_)
            if (in.edit == edit) return id;
        return {};
    }
    static std::string EditText(HWND edit) {
        int n = GetWindowTextLengthW(edit);
        std::wstring t(n, L'\0');
        GetWindowTextW(edit, t.data(), n + 1);
        std::wstring out;  // (back to \n)
        for (wchar_t c : t)
            if (c != L'\r') out += c;
        return U(out);
    }
    static LRESULT CALLBACK InputProc(HWND edit, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR data) {
        auto* self = (PanelWindow*)data;
        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) self->swallowChar_ = false;
        if (self->Captured(msg, wp, true)) return 0;
        if ((msg == WM_CHAR || msg == WM_SYSCHAR) && self->swallowChar_) return self->swallowChar_ = false, 0;
        if (msg == WM_KEYDOWN && wp == VK_ESCAPE) {
            self->Dismiss();
            return 0;
        }
        // Ctrl+V (or Shift+Insert, or Paste) with no text copied: files or a picture, for a plugin that takes them.
        if (msg == WM_PASTE && !IsClipboardFormatAvailable(CF_UNICODETEXT)) {
            std::string id = self->InputId(edit);
            static const UINT png = RegisterClipboardFormatW(L"PNG");
            if (IsClipboardFormatAvailable(CF_HDROP) && self->Find(id, "onPasteFiles")) {
                std::vector<std::string> paths = ClipboardFiles(edit);
                if (!paths.empty()) {
                    self->host_.event(id, "onPasteFiles", paths);
                    return 0;
                }
            }
            if ((IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_DIBV5) || IsClipboardFormatAvailable(CF_BITMAP) ||
                 IsClipboardFormatAvailable(png)) &&
                self->Find(id, "onPasteImage")) {
                self->host_.event(id, "onPasteImage", {});
                return 0;
            }
        }
        if (msg == WM_CHAR && (wp == L'\r' || wp == 10)) {
            std::string id = self->InputId(edit);
            auto it = self->inputs_.find(id);
            bool ctrl = GetKeyState(VK_CONTROL) < 0, shift = GetKeyState(VK_SHIFT) < 0;
            bool submit = it == self->inputs_.end() || !it->second.multiline || ctrl || (it->second.submitOnEnter && !shift);
            if (submit) {
                self->host_.event(id, "onSubmit", {EditText(edit)});
                return 0;
            }
        }
        return DefSubclassProc(edit, msg, wp, lp);
    }

    // Painting.
    void Fill(HDC dc, RECT r, COLORREF c, int radius = 0) {
        if (radius <= 0) {
            HBRUSH b = CreateSolidBrush(c);
            FillRect(dc, &r, b);
            DeleteObject(b);
            return;
        }
        HBRUSH b = CreateSolidBrush(c);
        HPEN p = CreatePen(PS_SOLID, 1, c);
        HGDIOBJ ob = SelectObject(dc, b), op = SelectObject(dc, p);
        RoundRect(dc, r.left, r.top, r.right, r.bottom, radius * 2, radius * 2);
        SelectObject(dc, ob), SelectObject(dc, op);
        DeleteObject(b), DeleteObject(p);
    }
    void Frame(HDC dc, RECT r, COLORREF c, int width, int radius = 0) {
        HPEN p = CreatePen(PS_INSIDEFRAME, std::max(1, width), c);
        HGDIOBJ op = SelectObject(dc, p), ob = SelectObject(dc, GetStockObject(NULL_BRUSH));
        if (radius > 0) RoundRect(dc, r.left, r.top, r.right, r.bottom, radius * 2, radius * 2);
        else Rectangle(dc, r.left, r.top, r.right, r.bottom);
        SelectObject(dc, op), SelectObject(dc, ob);
        DeleteObject(p);
    }
    void DrawGlyph(HDC dc, const std::string& name, RECT r, int px, COLORREF c) {
        std::wstring g = GlyphText(name);
        if (g.empty()) return;
        HGDIOBJ old = SelectObject(dc, fonts_.Glyph(px));
        SetTextColor(dc, c);
        DrawTextW(dc, g.c_str(), (int)g.size(), &r, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP);
        SelectObject(dc, old);
    }
    void Paint(HDC dc, const Laid& l, RECT clip) {
        const Node& n = *l.node;
        RECT r = l.rc, visible;
        if (!IntersectRect(&visible, &r, &clip)) return;
        HRGN region = CreateRectRgnIndirect(&clip);
        SelectClipRgn(dc, region);
        DeleteObject(region);
        int radius = Px(Num(n, "radius", 0));
        const std::string& t = n.type;
        bool hot = hover_ == l.id, down = pressed_ == l.id && hot;
        if (Has(n, "fill") && t != "progress") Fill(dc, r, Role(Str(n, "fill"), Raised()), radius);
        if (Has(n, "border")) Frame(dc, r, Role(Str(n, "border"), Edge()), Px(Num(n, "borderWidth", 1)), radius);
        if (t == "label" || t == "markdown") {
            HGDIOBJ old = SelectObject(dc, TextFont(n));
            SetTextColor(dc, Role(Str(n, "color"), Ink()));
            std::wstring text = LabelText(n);
            std::string a = Str(n, "textAlign", "start");
            UINT how = DT_NOPREFIX | (a == "center" ? DT_CENTER : a == "end" ? DT_RIGHT : DT_LEFT);
            how |= Num(n, "maxLines", 0) == 1 ? DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER : DT_WORDBREAK | DT_EDITCONTROL | DT_END_ELLIPSIS;
            DrawTextW(dc, text.c_str(), (int)text.size(), &r, how);
            SelectObject(dc, old);
        } else if (t == "glyph") {
            DrawGlyph(dc, Str(n, "name"), r, GlyphSize(n), Role(Str(n, "color"), Ink()));
        } else if (t == "image") {
            if (Picture* p = PictureFor(n)) {
                RECT at = PictureRect(*p, r, true);
                if (HBITMAP b = DrawnAt(*p, at.right - at.left, at.bottom - at.top)) {
                    HDC mem = CreateCompatibleDC(dc);
                    HGDIOBJ old = SelectObject(mem, b);
                    BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
                    AlphaBlend(dc, at.left, at.top, at.right - at.left, at.bottom - at.top, mem, 0, 0, at.right - at.left, at.bottom - at.top, blend);
                    SelectObject(mem, old);
                    DeleteDC(mem);
                }
                if (hot && n.on.count("onClick")) Frame(dc, at, Accent(), Px(1));
            } else if (Has(n, "path") || Has(n, "source")) {
                HGDIOBJ old = SelectObject(dc, fonts_.Text(13, false));
                SetTextColor(dc, Muted());
                std::wstring text = PictureMissing(n);
                DrawTextW(dc, text.c_str(), (int)text.size(), &r, DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
                SelectObject(dc, old);
            }
        } else if (t == "button") {
            bool enabled = Bool(n, "enabled", true);
            std::string variant = Str(n, "variant", "default");
            COLORREF fill = Raised(), ink = Ink();
            bool filled = true;
            if (variant == "primary") fill = Accent(), ink = Ground();
            else if (variant == "secondary") fill = Muted(), ink = Ground();
            else if (variant == "destructive") fill = Warn(), ink = Ground();
            else if (variant == "ghost" || variant == "outline") filled = false;
            if (Bool(n, "selected", false)) filled = true, fill = Hover(), ink = Accent();
            if (filled) Fill(dc, r, hot && enabled ? (variant == "default" ? Hover() : fill) : fill, radius);
            else if (hot && enabled) Fill(dc, r, Hover(), radius);
            if (variant == "outline") Frame(dc, r, Edge(), Px(1), radius);
            if (down && enabled) Frame(dc, r, Accent(), Px(1), radius);
            if (!enabled) ink = Line();
            std::wstring text = W(Str(n, "text"));
            std::string glyph = Str(n, "glyph");
            int gw = glyph.empty() ? 0 : GlyphSize(n, "glyphSize");
            HFONT font = fonts_.Text(Num(n, "fontSize", 16), false);
            int tw = text.empty() ? 0 : Text(font, text, 10000, 1).w;
            int total = gw + (gw && tw ? Px(6) : 0) + tw;
            std::string align = Str(n, "contentAlign", "center");
            int x = align == "start" ? r.left + Px(12) : align == "end" ? r.right - Px(12) - total : (r.left + r.right - total) / 2;
            if (gw) DrawGlyph(dc, glyph, {x, r.top, x + gw, r.bottom}, gw, ink), x += gw + (tw ? Px(6) : 0);
            if (tw) {
                HGDIOBJ old = SelectObject(dc, font);
                SetTextColor(dc, ink);
                RECT tr{x, r.top, r.right - Px(4), r.bottom};
                DrawTextW(dc, text.c_str(), (int)text.size(), &tr, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
                SelectObject(dc, old);
            }
        } else if (t == "input") {
            if (Bool(n, "frameVisible", true)) {
                Fill(dc, r, Raised(), radius);
                bool focused = l.input && GetFocus() == l.input;
                Frame(dc, r, focused ? Accent() : Line(), Px(1), radius);
            }
        } else if (t == "select") {
            Fill(dc, r, hot ? Hover() : Raised(), radius);
            Frame(dc, r, Line(), Px(1), radius);
            auto const* options = Prop(n, "options");
            int chosen = (int)Num(n, "selectedIndex", 0);
            std::wstring text = options && chosen >= 1 && chosen <= (int)options->items.size()
                                    ? W(options->items[chosen - 1].text)
                                    : W(Str(n, "placeholder"));
            HGDIOBJ old = SelectObject(dc, fonts_.Text(16, false));
            SetTextColor(dc, Ink());
            RECT tr{r.left + Px(10), r.top, r.right - Px(28), r.bottom};
            DrawTextW(dc, text.c_str(), (int)text.size(), &tr, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
            SelectObject(dc, old);
            DrawGlyph(dc, "chevron-down", {r.right - Px(26), r.top, r.right - Px(6), r.bottom}, Px(16), Muted());
        } else if (t == "toggle") {
            bool on = Bool(n, "checked", false);
            RECT track{r.left, r.top + (r.bottom - r.top - Px(20)) / 2, r.left + Px(36), r.top + (r.bottom - r.top + Px(20)) / 2};
            Fill(dc, track, on ? Accent() : Raised(), Px(10));
            Frame(dc, track, on ? Accent() : Edge(), Px(1), Px(10));
            int knob = Px(14), kx = on ? track.right - Px(3) - knob : track.left + Px(3);
            Fill(dc, {kx, track.top + Px(3), kx + knob, track.top + Px(3) + knob}, on ? Ground() : Muted(), knob / 2);
        } else if (t == "slider") {
            double lo = Num(n, "min", 0), hi = Num(n, "max", 1), v = std::clamp(Num(n, "value", lo), std::min(lo, hi), std::max(lo, hi));
            double f = hi > lo ? (v - lo) / (hi - lo) : 0;
            int cy = (r.top + r.bottom) / 2, x = r.left + (int)((r.right - r.left) * f);
            Fill(dc, {r.left, cy - Px(2), r.right, cy + Px(2)}, Raised());
            Fill(dc, {r.left, cy - Px(2), x, cy + Px(2)}, Accent());
            Fill(dc, {x - Px(5), cy - Px(7), x + Px(5), cy + Px(7)}, hot || dragging_ == l.id ? Accent() : Ink());
        } else if (t == "progress") {
            double f = std::clamp(Num(n, "progress", 0), 0.0, 1.0);
            Fill(dc, r, Role(Str(n, "track"), Raised()), radius);
            RECT done = r;
            done.right = r.left + (int)((r.right - r.left) * f);
            if (done.right > done.left) Fill(dc, done, Role(Str(n, "fill"), Accent()), radius);
        } else if (t == "separator") {
            int thick = std::max(1, Px(Num(n, "thickness", 1)));
            COLORREF c = Role(Str(n, "color"), Line());
            std::string o = Str(n, "orientation", "auto");
            bool vertical = o == "vertical" || (o == "auto" && r.bottom - r.top > r.right - r.left);
            if (vertical) {
                int x = (r.left + r.right - thick) / 2;
                Fill(dc, {x, r.top, x + thick, r.bottom}, c);
            } else {
                int y = (r.top + r.bottom - thick) / 2;
                Fill(dc, {r.left, y, r.right, y + thick}, c);
            }
        } else if (t == "graph") {
            for (int set = 0; set < 2; set++) {
                auto const* values = Prop(n, set ? "values2" : "values");
                if (!values || values->items.size() < 2) continue;
                COLORREF c = Role(Str(n, set ? "color2" : "color"), set ? Muted() : Accent());
                HPEN p = CreatePen(PS_SOLID, std::max(1, Px(Num(n, "lineWidth", 1))), c);
                HGDIOBJ op = SelectObject(dc, p);
                size_t count = values->items.size();
                for (size_t i = 0; i < count; i++) {
                    double v = std::clamp(values->items[i].number, 0.0, 1.0);
                    int x = r.left + (int)((r.right - r.left - 1) * i / (count - 1)), y = r.bottom - 1 - (int)((r.bottom - r.top - 1) * v);
                    if (i) LineTo(dc, x, y);
                    else MoveToEx(dc, x, y, nullptr);
                }
                SelectObject(dc, op);
                DeleteObject(p);
            }
        } else if (Container(t) && hot && n.on.count("onClick") && !Has(n, "fill")) {
            Fill(dc, r, Hover(), radius);  // (a clickable row, lit)
        }
        RECT inner = clip;
        if (t == "scroll") IntersectRect(&inner, &clip, &r);
        for (auto const& k : l.kids) Paint(dc, k, inner);
        if (t == "scroll" && l.contentHeight > r.bottom - r.top) {  // where it's scrolled to
            int viewport = r.bottom - r.top, bar = std::max(Px(16), viewport * viewport / l.contentHeight);
            int y = r.top + (viewport - bar) * l.scroll / std::max(1, l.contentHeight - viewport);
            HRGN all = CreateRectRgnIndirect(&clip);
            SelectClipRgn(dc, all);
            DeleteObject(all);
            Fill(dc, {r.right - Px(3), y, r.right, y + bar}, Line());
        }
    }
    void OnPaint() {
        PAINTSTRUCT ps;
        HDC screen = BeginPaint(hwnd_, &ps);
        RECT client;
        GetClientRect(hwnd_, &client);
        HDC dc = CreateCompatibleDC(screen);
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), std::max(1L, client.right), -std::max(1L, client.bottom), 1, 32, BI_RGB};
        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
        HGDIOBJ old = SelectObject(dc, bmp);
        SetBkMode(dc, TRANSPARENT);
        Fill(dc, client, Ground());
        if (root_.node) Paint(dc, root_, client);
        SelectClipRgn(dc, nullptr);
        if (!desktop_) PaintStrip(dc);
        Frame(dc, client, Edge(), Px(1));
        if (desktop_ && bits) {  // (the wallpaper's foreground over it, when a plugin set a depth mask)
            GdiFlush();
            RECT onScreen;
            GetWindowRect(hwnd_, &onScreen);
            ApplyDepth((uint32_t*)bits, client.right, client.bottom, onScreen);
        }
        BitBlt(screen, 0, 0, client.right, client.bottom, dc, 0, 0, SRCCOPY);
        SelectObject(dc, old);
        DeleteObject(bmp);
        DeleteDC(dc);
        EndPaint(hwnd_, &ps);
    }

    void PaintStrip(HDC dc) {
        RECT pin = PinRect();
        if (pinHover_ || pinPressed_) Fill(dc, pin, Hover());
        DrawGlyph(dc, pinned_ ? "pinned" : "pin", pin, Px(14), pinned_ ? Accent() : pinHover_ ? Ink() : Muted());
    }
    void SetPinned(bool on) {
        pinned_ = on;
        Keep(L"panelPinned", on ? 1 : 0);
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
    // After a move or resize by the user: keep the new size (96-dpi pixels) and, if it moved, its place.
    void KeepPlace() {
        RECT now;
        GetWindowRect(hwnd_, &now);
        if (now.right - now.left != sizing_.right - sizing_.left || now.bottom - now.top != sizing_.bottom - sizing_.top) {
            Keep(L"panelWidth", (int)std::lround((now.right - now.left) / (double)s_));
            Keep(L"panelHeight", (int)std::lround((now.bottom - now.top) / (double)s_));
        }
        if (now.left != sizing_.left || now.top != sizing_.top) {
            Keep(L"panelX", now.left), Keep(L"panelY", now.top), Keep(L"panelPlaced", 1);
        }
    }
    // Right-click on the pin: back to the manifest's size and place (from the next time it opens).
    void ForgetPlace() {
        for (auto what : {L"panelWidth", L"panelHeight", L"panelX", L"panelY", L"panelPlaced"}) Forget(what);
    }

    // The pointer.
    // The innermost node at a point that takes clicks (and the innermost with a hover handler or look).
    void HitTest(const Laid& l, POINT pt, RECT clip, const Laid** click, const Laid** hover) const {
        RECT r;
        if (!IntersectRect(&r, &l.rc, &clip) || !PtInRect(&r, pt)) return;
        const std::string& t = l.node->type;
        bool enabled = Bool(*l.node, "enabled", true);
        bool clicks = enabled && (l.node->on.count("onClick") || l.node->on.count("onRightClick") || t == "toggle" || t == "select" ||
                                  t == "slider" || (t == "button"));
        if (clicks) *click = &l;
        if (clicks || l.node->on.count("onHover")) *hover = &l;
        RECT inner = clip;
        if (t == "scroll") IntersectRect(&inner, &clip, &l.rc);
        for (auto const& k : l.kids) HitTest(k, pt, inner, click, hover);
    }
    const Laid* ScrollAt(const Laid& l, POINT pt) const {
        if (!PtInRect(&l.rc, pt)) return nullptr;
        for (auto const& k : l.kids)
            if (auto s = ScrollAt(k, pt)) return s;
        return l.node->type == "scroll" && l.contentHeight > l.rc.bottom - l.rc.top ? &l : nullptr;
    }
    std::string KeyOf(const std::string& id) const {
        const Laid* l = FindLaid(root_, id);
        return l ? Str(*l->node, "key") : "";
    }
    void OnMove(POINT pt) {
        if (!tracking_) {
            TRACKMOUSEEVENT t{sizeof(t), TME_LEAVE, hwnd_, 0};
            tracking_ = TrackMouseEvent(&t);
        }
        if (!dragging_.empty()) return SliderTo(dragging_, pt, false);
        RECT pin = PinRect();
        if (bool on = PtInRect(&pin, pt) != 0; on != pinHover_) pinHover_ = on, InvalidateRect(hwnd_, &pin, FALSE);
        const Laid *click = nullptr, *hover = nullptr;
        RECT all{-100000, -100000, 100000, 100000};
        if (root_.node) HitTest(root_, pt, all, &click, &hover);
        std::string now = hover ? hover->id : "";
        if (now == hover_) return;
        std::string was = hover_;
        hover_ = now;
        InvalidateRect(hwnd_, nullptr, FALSE);
        // onHover: every "true" matched by a "false" (to the node that had it, even if it's gone since).
        if (!hoverReported_.empty()) host_.event(hoverReported_, "onHover", {"false", KeyOf(hoverReported_)}), hoverReported_.clear();
        if (hover && hover->node->on.count("onHover")) {
            hoverReported_ = hover->id;
            host_.event(hover->id, "onHover", {"true", Str(*hover->node, "key")});
        }
    }
    void OnLeave() {
        tracking_ = false;
        pinHover_ = false;
        if (!dragging_.empty()) return;
        hover_.clear();
        if (!hoverReported_.empty()) host_.event(hoverReported_, "onHover", {"false", KeyOf(hoverReported_)}), hoverReported_.clear();
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
    void SliderTo(const std::string& id, POINT pt, bool end) {
        const Laid* l = FindLaid(root_, id);
        if (!l) return;
        double lo = Num(*l->node, "min", 0), hi = Num(*l->node, "max", 1), step = Num(*l->node, "step", 0);
        double f = std::clamp((double)(pt.x - l->rc.left) / std::max(1L, l->rc.right - l->rc.left), 0.0, 1.0);
        double v = lo + (hi - lo) * f;
        if (step > 0) v = lo + std::round((v - lo) / step) * step;
        char b[32];
        snprintf(b, sizeof(b), "%.6g", v);
        host_.event(id, "onChange", {b});
        if (end) host_.event(id, "onDragEnd", {});
    }
    void OnClick(POINT pt, bool right) {
        const Laid *click = nullptr, *hover = nullptr;
        RECT all{-100000, -100000, 100000, 100000};
        if (root_.node) HitTest(root_, pt, all, &click, &hover);
        if (!click) return;
        const Node& n = *click->node;
        if (right) {
            if (n.on.count("onRightClick")) host_.event(click->id, "onRightClick", {});
            return;
        }
        if (n.type == "toggle") return host_.event(click->id, "onChange", {Bool(n, "checked", false) ? "false" : "true"});
        if (n.type == "select") {
            // Copied first: the menu runs messages of its own, and nothing of the tree is read after it.
            std::vector<std::string> options;
            if (auto const* list = Prop(n, "options"))
                for (auto const& o : list->items) options.push_back(o.text);
            if (options.empty()) return;
            HMENU menu = CreatePopupMenu();
            int chosen = (int)Num(n, "selectedIndex", 0);
            for (size_t i = 0; i < options.size(); i++)
                AppendMenuW(menu, MF_STRING | ((int)i + 1 == chosen ? MF_CHECKED : 0), i + 1, W(options[i]).c_str());
            POINT at{click->rc.left, click->rc.bottom};
            ClientToScreen(hwnd_, &at);
            std::string id = click->id;
            int picked = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY, at.x, at.y, 0, hwnd_, nullptr);
            DestroyMenu(menu);
            if (picked > 0 && options.size() >= (size_t)picked) host_.event(id, "onChange", {std::to_string(picked), options[picked - 1]});
            return;
        }
        if (n.on.count("onClick")) host_.event(click->id, "onClick", {});
    }
    void Dismiss() {
        g_lastDismissed = GetTickCount64();
        host_.closed();
    }

    static LRESULT CALLBACK Proc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
        auto* self = (PanelWindow*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
        if (!self) return DefWindowProcW(wnd, msg, wp, lp);
        switch (msg) {
            case WM_PAINT: self->OnPaint(); return 0;
            case WM_ERASEBKGND: return 1;
            case WM_MOUSEMOVE: self->OnMove({GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}); return 0;
            case WM_MOUSELEAVE: self->OnLeave(); return 0;
            case WM_LBUTTONDOWN: {
                POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                RECT pin = self->PinRect();
                if (PtInRect(&pin, pt)) {  // (the editor keeps the focus)
                    self->pinPressed_ = true;
                    SetCapture(wnd);
                    InvalidateRect(wnd, &pin, FALSE);
                    return 0;
                }
                const Laid *click = nullptr, *hover = nullptr;
                RECT all{-100000, -100000, 100000, 100000};
                if (self->root_.node) self->HitTest(self->root_, pt, all, &click, &hover);
                self->pressed_ = click ? click->id : "";
                if (click && click->node->type == "slider") {
                    self->dragging_ = click->id;
                    self->SliderTo(click->id, pt, false);
                }
                SetCapture(wnd);
                SetFocus(wnd);
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            case WM_LBUTTONUP: {
                POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                ReleaseCapture();
                if (self->pinPressed_) {
                    self->pinPressed_ = false;
                    RECT pin = self->PinRect();
                    if (PtInRect(&pin, pt)) self->SetPinned(!self->pinned_);
                    InvalidateRect(wnd, &pin, FALSE);
                    return 0;
                }
                std::string pressed = self->pressed_;
                self->pressed_.clear();
                if (!self->dragging_.empty()) {
                    std::string id = self->dragging_;
                    self->dragging_.clear();
                    self->SliderTo(id, pt, true);
                    return 0;
                }
                const Laid *click = nullptr, *hover = nullptr;
                RECT all{-100000, -100000, 100000, 100000};
                if (self->root_.node) self->HitTest(self->root_, pt, all, &click, &hover);
                InvalidateRect(wnd, nullptr, FALSE);
                if (click && click->id == pressed) self->OnClick(pt, false);
                return 0;
            }
            case WM_RBUTTONUP: {
                POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                RECT pin = self->PinRect();
                if (PtInRect(&pin, pt)) {
                    self->ForgetPlace();
                    return 0;
                }
                self->OnClick(pt, true);
                return 0;
            }
            // Its edges resize it and its top strip (but the pin) moves it.
            case WM_NCHITTEST: {
                POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                ScreenToClient(wnd, &pt);
                if (self->desktop_) {  // (any part that isn't a control drags it)
                    const Laid *click = nullptr, *hover = nullptr;
                    RECT all{-100000, -100000, 100000, 100000};
                    if (self->root_.node) self->HitTest(self->root_, pt, all, &click, &hover);
                    return click || hover ? HTCLIENT : HTCAPTION;
                }
                RECT c, pin = self->PinRect();
                GetClientRect(wnd, &c);
                int g = self->Px(6);
                bool l = pt.x < g, r = pt.x >= c.right - g, t = pt.y < g, b = pt.y >= c.bottom - g;
                if (t && l) return HTTOPLEFT;
                if (t && r) return HTTOPRIGHT;
                if (b && l) return HTBOTTOMLEFT;
                if (b && r) return HTBOTTOMRIGHT;
                if (l) return HTLEFT;
                if (r) return HTRIGHT;
                if (t) return HTTOP;
                if (b) return HTBOTTOM;
                if (pt.y < self->Strip() && !PtInRect(&pin, pt)) return HTCAPTION;
                return HTCLIENT;
            }
            case WM_GETMINMAXINFO: {
                auto* mm = (MINMAXINFO*)lp;
                mm->ptMinTrackSize = {self->Px(220), self->Px(160)};
                return 0;
            }
            case WM_SIZE:
                if (wp != SIZE_MINIMIZED && self->root_.node && !self->fitting_) self->Relayout();
                return 0;
            case WM_ENTERSIZEMOVE: GetWindowRect(wnd, &self->sizing_); return 0;
            case WM_EXITSIZEMOVE: self->KeepPlace(); return 0;
            case WM_MOUSEWHEEL: {
                POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
                ScreenToClient(wnd, &pt);
                const Laid* s = self->root_.node ? self->ScrollAt(self->root_, pt) : nullptr;
                if (!s) return 0;
                std::string id = s->id;
                int step = self->Px(48) * GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA;
                Laid* target = const_cast<Laid*>(s);
                int most = std::max(0, target->contentHeight - (int)(target->rc.bottom - target->rc.top));
                int to = std::clamp(target->scroll - step, 0, most);
                if (to == target->scroll) return 0;
                target->scroll = to;
                self->Replace(self->root_);
                self->SyncInputs();
                InvalidateRect(wnd, nullptr, FALSE);
                return 0;
            }
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
                if (self->Captured(msg, wp, false)) return 0;
                if (msg == WM_KEYDOWN && wp == VK_ESCAPE && !self->desktop_) self->Dismiss();
                if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) break;  // (Alt+F4, Alt+Space)
                return 0;
            case WM_NCRBUTTONUP:
                if (self->desktop_) return self->DesktopMenu(), 0;
                break;
            case WM_MOUSEACTIVATE:
                if (self->desktop_) return MA_NOACTIVATE;
                break;
            case WM_WINDOWPOSCHANGING:
                if (self->desktop_) {  // (as low as it goes: just above the desktop that owns it)
                    auto* pos = (WINDOWPOS*)lp;
                    if (!(pos->flags & SWP_NOZORDER)) pos->hwndInsertAfter = HWND_BOTTOM;
                }
                break;
            case WM_COMMAND:
                if (HIWORD(wp) == EN_CHANGE) {
                    HWND edit = (HWND)lp;
                    std::string id = self->InputId(edit);
                    if (!id.empty() && GetFocus() == edit) self->host_.event(id, "onChange", {EditText(edit)});
                } else if (HIWORD(wp) == EN_SETFOCUS || HIWORD(wp) == EN_KILLFOCUS) {
                    InvalidateRect(wnd, nullptr, FALSE);
                }
                return 0;
            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORSTATIC: {
                HDC dc = (HDC)wp;
                COLORREF ground = self->Raised(), ink = self->Ink();
                if (!self->brush_ || ground != self->inputGround_) {
                    if (self->brush_) DeleteObject(self->brush_);
                    self->brush_ = CreateSolidBrush(ground);
                    self->inputGround_ = ground;
                }
                SetTextColor(dc, IsWindowEnabled((HWND)lp) ? ink : self->Muted());
                SetBkColor(dc, ground);
                return (LRESULT)self->brush_;
            }
            case WM_ACTIVATE:
                // A click elsewhere closes it, as a flyout (not a menu or dialog of its own), unless it's pinned.
                if (LOWORD(wp) == WA_INACTIVE && !self->pinned_) {
                    HWND to = (HWND)lp;
                    if (!to || GetAncestor(to, GA_ROOTOWNER) != wnd) PostMessageW(wnd, WM_APP, 0, 0);
                }
                return 0;
            case WM_APP:
                if (!self->pinned_ && GetForegroundWindow() != wnd && !IsChild(wnd, GetForegroundWindow())) self->Dismiss();
                return 0;
            case WM_CLOSE:
                if (!self->desktop_) self->Dismiss();
                return 0;
            case WM_DROPFILES: {
                HDROP drop = (HDROP)wp;
                std::vector<std::string> paths;
                for (UINT i = 0, n = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0); i < n; i++) {
                    std::wstring path(DragQueryFileW(drop, i, nullptr, 0), L'\0');
                    DragQueryFileW(drop, i, path.data(), (UINT)path.size() + 1);
                    paths.push_back(U(path));
                }
                DragFinish(drop);
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(wnd, &pt);
                // The innermost node there that takes drops.
                const Laid* target = nullptr;
                std::function<void(const Laid&)> walk = [&](const Laid& l) {
                    if (!l.node || !PtInRect(&l.rc, pt)) return;
                    if (l.node->on.count("onDrop")) target = &l;
                    for (auto const& k : l.kids) walk(k);
                };
                walk(self->root_);
                if (target && !paths.empty()) self->host_.event(target->id, "onDrop", paths);
                return 0;
            }
        }
        return DefWindowProcW(wnd, msg, wp, lp);
    }
};

std::function<std::wstring(const std::string&)> g_glyphs;
std::wstring GlyphText(const std::string& name) { return g_glyphs ? g_glyphs(name) : L""; }

}  // namespace

void SetGlyphLookup(std::function<std::wstring(const std::string&)> lookup) { g_glyphs = std::move(lookup); }

std::unique_ptr<Window> Window::Open(const Placement& where, Host host) {
    auto w = std::make_unique<PanelWindow>(where, std::move(host));
    if (!w->Hwnd()) return nullptr;
    return w;
}

ULONGLONG LastDismissed() { return g_lastDismissed; }

bool ReadPicturePixels(const std::wstring& path, int longest, std::vector<uint32_t>* pixels, UINT* w, UINT* h, double* naturalW,
                       double* naturalH) {
    if (IsSvg(path)) return ReadSvg(path, longest, pixels, w, h, naturalW, naturalH);
    std::vector<uint32_t> full;
    UINT fw = 0, fh = 0;
    if (!ReadRaster(path, 0, &full, &fw, &fh)) return false;
    if (naturalW) *naturalW = fw;
    if (naturalH) *naturalH = fh;
    if (longest <= 0 || std::max(fw, fh) <= (UINT)longest) {
        *pixels = std::move(full), *w = fw, *h = fh;
        return true;
    }
    return ReadRaster(path, longest, pixels, w, h);
}

void SetDepthMask(const std::wstring& screen, const std::wstring& maskPath, const std::wstring& wallpaperPath) {
    auto repaint = [] {
        std::lock_guard lock(g_depthLock);
        for (HWND w : g_desktopWindows) InvalidateRect(w, nullptr, FALSE);
    };
    if (maskPath.empty()) {
        {
            std::lock_guard lock(g_depthLock);
            std::erase_if(g_depths, [&](const Depth& d) { return !_wcsicmp(d.screen.c_str(), screen.c_str()); });
        }
        return repaint();
    }
    // Read off the caller's thread (a 4K wallpaper takes a moment), then swapped in.
    std::thread([=] {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        Depth d;
        d.screen = screen;
        std::vector<uint32_t> mask;
        UINT mw = 0, mh = 0;
        bool ok = ReadPicturePixels(wallpaperPath, 2560, &d.wall, &d.ww, &d.wh, &d.ow, &d.oh) &&
                  ReadPicturePixels(maskPath, 2560, &mask, &mw, &mh);
        struct Find {
            std::wstring want;
            RECT found{};
            bool ok = false;
        } find{L"\\\\.\\" + screen};
        EnumDisplayMonitors(
            nullptr, nullptr,
            [](HMONITOR m, HDC, LPRECT, LPARAM p) -> BOOL {
                auto& f = *(Find*)p;
                MONITORINFOEXW info{};
                info.cbSize = sizeof(info);
                if (GetMonitorInfoW(m, &info) && !_wcsicmp(info.szDevice, f.want.c_str())) f.found = info.rcMonitor, f.ok = true;
                return !f.ok;
            },
            (LPARAM)&find);
        if (ok && find.ok) {
            d.monitor = find.found;
            d.mask.resize((size_t)mw * mh);
            for (size_t i = 0; i < d.mask.size(); i++) {  // (grey: its red, as it's premultiplied by its alpha)
                uint32_t px = mask[i];
                d.mask[i] = (uint8_t)std::max({px >> 16 & 0xFF, px >> 8 & 0xFF, px & 0xFF});
            }
            d.mw = mw, d.mh = mh;
            wchar_t text[16] = L"";
            DWORD size = sizeof(text);
            RegGetValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallpaperStyle", RRF_RT_REG_SZ, nullptr, text, &size);
            d.style = text[0] ? _wtoi(text) : 10;
            size = sizeof(text);
            text[0] = 0;
            RegGetValueW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"TileWallpaper", RRF_RT_REG_SZ, nullptr, text, &size);
            if (_wtoi(text)) d.style = 1;
            if (d.style == 22) d.style = 10;  // (spanned: as filled, screen by screen)
            {
                std::lock_guard lock(g_depthLock);
                std::erase_if(g_depths, [&](const Depth& o) { return !_wcsicmp(o.screen.c_str(), screen.c_str()); });
                g_depths.push_back(std::move(d));
            }
            repaint();
        }
        CoUninitialize();
    }).detach();
}

}  // namespace td::panel
