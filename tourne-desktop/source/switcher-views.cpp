// Switcher views: Alt+Tab's windows as pictures in a 3D scene, over a dimmed,
// blurred copy of the screen. Flip 3D stacks them on a slant with the chosen
// one in front; Cover Flow lines them up with the chosen one facing you, the
// rest turned to the sides and reflected on the floor. Grid lays them all out
// at once, and Mission Control does too but keeps them roughly where they are
// on the screen; the chosen one is lifted and lit. Ring stands them in a
// turning circle, and Shelf in one row like a dock, magnified under the
// pointer. Opening, the windows
// fly from where they are into place. Tab, the arrows and the wheel move
// through them as in the list; a click switches, a middle click closes.
//
// Direct3D 11 draws the scene, and DirectWrite the chosen window's title.
// The pictures are live where Windows' own screen capture can take them
// (Windows.Graphics.Capture, Windows 10 1903 and later; nothing is injected),
// else stills taken as the view opens; minimised windows show their icon on a
// card. How things move follows the motion settings' "switcher" line.
//
// Inlined into switcher-host.cpp after Undrill: it uses the switcher's
// entries, selection, settings and Commit/CloseItem/Step.
//
// Idle cost: none. Its captures and its frame timer exist only while it's
// shown; its device and window are made once the view is chosen and kept,
// hidden, for the next Alt+Tab.

#include <d2d1_1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <mutex>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

// The capture API's two COM doors (declared here: not every SDK has them).
struct ITourneCaptureItemInterop : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE CreateForWindow(HWND window, REFIID riid, void** result) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateForMonitor(HMONITOR monitor, REFIID riid, void** result) = 0;
};
struct ITourneDxgiAccess : IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetInterface(REFIID iid, void** p) = 0;
};
constexpr GUID kIidCaptureItemInterop = {0x3628E81B, 0x3CAC, 0x4C60, {0xB7, 0xF4, 0x23, 0xCE, 0x0E, 0x0C, 0x33, 0x56}};
constexpr GUID kIidDxgiAccess = {0xA9B3D012, 0x3DF2, 0x4EE3, {0xB8, 0xD1, 0x86, 0x95, 0xF4, 0x57, 0xD3, 0xC1}};

constexpr wchar_t kViewClass[] = L"TourneSwitcherView";
constexpr UINT_PTR kViewTimerFrame = 1;

// The scene's shaders: each picture a quad (from the vertex number, no
// buffer), its corners rounded by a distance field; shadows and reflections
// are the same quad drawn another way, and the backdrop is the screen's copy
// read from a smaller mip with a few taps, for a cheap soft blur.
constexpr char kViewShaders[] = R"(
cbuffer Quad : register(b0) {
    row_major float4x4 M;
    float4 uvRect;  // u0 v0 u1 v1
    float4 size;    // the quad in px, its corner radius, the mode
    float4 tint;    // colour multiplied in; a: opacity
    float4 extra;   // x: shadow spread; y: reflection strength; z: blur (mip); w: texel size of mip 0
};
Texture2D T : register(t0);
SamplerState S : register(s0);
struct V { float4 pos : SV_Position; float2 uv : TEXCOORD0; float2 q : TEXCOORD1; };

V VS(uint id : SV_VertexID) {
    float2 c = float2(id & 1, id >> 1);
    V v;
    v.pos = mul(float4(c.x - 0.5, 0.5 - c.y, 0, 1), M);
    v.uv = lerp(uvRect.xy, uvRect.zw, c);
    v.q = c;
    return v;
}

float Box(float2 p, float2 half, float r) {
    float2 d = abs(p) - (half - r);
    return length(max(d, 0)) + min(max(d.x, d.y), 0) - r;
}

float4 PS(V v) : SV_Target {
    int mode = (int)(size.w + 0.5);
    float2 p = (v.q - 0.5) * size.xy;
    if (mode == 3) {  // the backdrop
        float lod = extra.z, step = extra.w * exp2(lod) * 1.5;
        float3 c = 0;
        [unroll] for (int y = -1; y <= 1; y++)
            [unroll] for (int x = -1; x <= 1; x++)
                c += T.SampleLevel(S, v.uv + float2(x, y) * step, lod).rgb;
        return float4(c / 9 * tint.rgb, 1);
    }
    if (mode == 1) {  // a shadow: soft around the picture's shape
        float spread = extra.x;
        float d = Box(p, size.xy * 0.5 - spread, size.z);
        float a = 1 - smoothstep(-spread * 0.4, spread, d);
        return float4(tint.rgb, a * tint.a);  // (black, or a glow's colour)
    }
    float d = Box(p, size.xy * 0.5, size.z);
    float a = saturate(0.5 - d) * tint.a;
    if (mode == 2) a *= extra.y * v.q.y * v.q.y;  // a reflection: strongest at the floor
    float4 c = T.Sample(S, v.uv);
    return float4(c.rgb * tint.rgb, a);
}
)";

using D3DCompileFn = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT,
                                      UINT, ID3DBlob**, ID3DBlob**);
using CreateWinrtDeviceFn = HRESULT(WINAPI*)(IDXGIDevice*, IUnknown**);  // (an IInspectable)

struct ViewQuad {
    float m[16];
    float uvRect[4];
    float size[4];
    float tint[4];
    float extra[4];
};

template <class T> void ViewRelease(T*& p) {
    if (p) p->Release();
    p = nullptr;
}

// One window's live picture: the capture's newest frame, waiting for the
// next draw (frames arrive on a capture thread; the drawing is done here).
struct ViewCapture {
    std::mutex lock;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame pending{nullptr};
    bool closed = false;
};

struct ViewTile {
    HWND wnd = nullptr;
    ID3D11Texture2D* tex = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    int texW = 0, texH = 0;  // the texture
    int w = 0, h = 0;        // the picture in it
    bool card = true;        // the icon card (no picture yet)
    RECT real{};             // the window on the screen, monitor-relative
    bool iconic = false;
    std::shared_ptr<ViewCapture> cap;
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{nullptr};
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool pool{nullptr};
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{nullptr};
    winrt::Windows::Graphics::SizeInt32 poolSize{};
    POINT quad[4]{};  // where it was last drawn, for clicks
    bool drawn = false;
    float order = 0;  // and how far back (the nearest wins a click)
};

struct ViewState {
    HWND wnd = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    IDXGISwapChain1* chain = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11Buffer* cb = nullptr;
    ID3D11BlendState* blend = nullptr;
    ID3D11SamplerState* sampler = nullptr;
    ID3D11RasterizerState* raster = nullptr;
    ID2D1Factory1* d2d = nullptr;
    ID2D1DeviceContext* dc = nullptr;
    ID2D1Bitmap1* target = nullptr;
    IDWriteFactory* dwrite = nullptr;
    IDWriteTextFormat* titleFormat = nullptr;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice rtDevice{nullptr};
    bool captureOk = false, captureChecked = false;
    int chainW = 0, chainH = 0;
    ID3D11Texture2D* back = nullptr;  // the screen's copy, with mips
    ID3D11ShaderResourceView* backSrv = nullptr;
    int backMips = 1;
    RECT mon{};
    std::vector<std::unique_ptr<ViewTile>> tiles;
    bool shown = false;
    int kind = 0;  // 1 Flip 3D, 2 Cover Flow, 3 Grid, 4 Mission Control, 5 Ring, 6 Shelf
    double pos = 0, from = 0, to = 0;
    ULONGLONG moveStart = 0, openStart = 0;
    int lastSel = 0;
    int selFrom = -1;  // the grids and Shelf: the one the light is leaving
    float focusX = 0;          // Shelf: where it's magnified (world x), following the pointer or the selection
    bool focusSet = false, pointer = false;
    POINT pointerAt{};
    motion::Anim anim;
};
ViewState g_view;

// The view for this opening: Win+Tab's Mission Control (g_viewFor), else the one set.
int ViewKind() { return g_viewFor >= 0 ? g_viewFor : g_settings.view; }
bool ViewWanted() { return ViewKind() != 0; }

double ViewMs() {
    if (g_view.anim.set) return motion::Ms(g_view.anim);
    return 320;
}

double ViewEase(double t) {
    if (g_view.anim.set) return g_view.anim.on ? motion::Ease(g_view.anim, t) : 1;
    motion::Anim a;  // Hyprland's default curve
    return motion::Ease(a, t);
}

double ViewProgress(ULONGLONG start) {
    double ms = ViewMs();
    if (ms <= 0) return 1;
    return ViewEase(std::min(1.0, (double)(GetTickCount64() - start) / ms));
}

// The picture's texture, made again when its size changes.
bool ViewTexture(ViewTile& t, int w, int h, const void* pixels) {
    if (!t.tex || t.texW != w || t.texH != h || pixels) {
        ViewRelease(t.srv);
        ViewRelease(t.tex);
        D3D11_TEXTURE2D_DESC d{};
        d.Width = std::max(1, w), d.Height = std::max(1, h), d.MipLevels = 1, d.ArraySize = 1;
        d.Format = DXGI_FORMAT_B8G8R8A8_UNORM, d.SampleDesc.Count = 1, d.Usage = D3D11_USAGE_DEFAULT;
        d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA init{pixels, (UINT)(w * 4), 0};
        if (FAILED(g_view.device->CreateTexture2D(&d, pixels ? &init : nullptr, &t.tex))) return false;
        if (FAILED(g_view.device->CreateShaderResourceView(t.tex, nullptr, &t.srv))) return false;
        t.texW = w, t.texH = h;
    }
    return true;
}

// A 32-bit top-down DIB to draw into with GDI.
struct ViewDib {
    HDC dc = nullptr;
    HBITMAP bmp = nullptr, old = nullptr;
    uint32_t* bits = nullptr;
    int w = 0, h = 0;
    ViewDib(int width, int height) : w(width), h(height) {
        BITMAPINFO bi{};
        bi.bmiHeader = {sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB};
        HDC screen = GetDC(nullptr);
        dc = CreateCompatibleDC(screen);
        ReleaseDC(nullptr, screen);
        bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
        if (bmp) old = (HBITMAP)SelectObject(dc, bmp);
    }
    ~ViewDib() {
        if (old) SelectObject(dc, old);
        if (bmp) DeleteObject(bmp);
        if (dc) DeleteDC(dc);
    }
    void Opaque() {
        for (int i = 0; i < w * h; i++) bits[i] |= 0xFF000000;
    }
};

// A minimised window (or one whose picture hasn't come yet): its icon and
// title on a card the shape of the window.
void ViewCard(ViewTile& t, Entry const& e) {
    int w = std::max(1, (int)(t.real.right - t.real.left)), h = std::max(1, (int)(t.real.bottom - t.real.top));
    if (t.iconic || w < 64 || h < 48) w = 480, h = 300;
    double scale = std::min(1.0, 480.0 / std::max(w, h));
    w = std::max(64, (int)(w * scale)), h = std::max(48, (int)(h * scale));
    ViewDib dib(w, h);
    if (!dib.bits) return;
    RECT all{0, 0, w, h};
    HBRUSH ground = CreateSolidBrush(Rgb(g_settings.look.ground));
    FillRect(dib.dc, &all, ground);
    DeleteObject(ground);
    HICON icon = WindowIcon(t.wnd, true);
    if (!icon) icon = e.icon;
    int size = std::min(96, h / 2);
    if (icon) DrawIconEx(dib.dc, (w - size) / 2, h / 2 - size * 3 / 4, icon, size, size, 0, nullptr, DI_NORMAL);
    SetBkMode(dib.dc, TRANSPARENT);
    SetTextColor(dib.dc, Rgb(g_settings.look.text));
    HFONT old = g_font ? (HFONT)SelectObject(dib.dc, g_font) : nullptr;
    RECT text{12, h / 2 + size / 3, w - 12, h - 8};
    std::wstring title = Title(t.wnd);
    DrawTextW(dib.dc, title.c_str(), -1, &text, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);
    if (old) SelectObject(dib.dc, old);
    dib.Opaque();
    if (ViewTexture(t, w, h, dib.bits)) t.w = w, t.h = h, t.card = true;
}

// A still of the window, as it draws itself (for when live capture can't).
bool ViewStill(ViewTile& t) {
    RECT wr, fr;
    if (!GetWindowRect(t.wnd, &wr)) return false;
    if (FAILED(DwmGetWindowAttribute(t.wnd, DWMWA_EXTENDED_FRAME_BOUNDS, &fr, sizeof(fr)))) fr = wr;
    int w = wr.right - wr.left, h = wr.bottom - wr.top;
    if (w <= 0 || h <= 0 || w > 16384 || h > 16384) return false;
    ViewDib dib(w, h);
    if (!dib.bits || !PrintWindow(t.wnd, dib.dc, 2 /* PW_RENDERFULLCONTENT */)) return false;
    // Only the visible frame (Windows 10 and 11 draw invisible borders around it).
    int x = std::clamp((int)(fr.left - wr.left), 0, w - 1), y = std::clamp((int)(fr.top - wr.top), 0, h - 1);
    int cw = std::clamp((int)(fr.right - fr.left), 1, w - x), ch = std::clamp((int)(fr.bottom - fr.top), 1, h - y);
    std::vector<uint32_t> px((size_t)cw * ch);
    size_t lit = 0;
    for (int row = 0; row < ch; row++)
        for (int col = 0; col < cw; col++) {
            uint32_t c = dib.bits[(size_t)(row + y) * w + col + x] & 0xFFFFFF;
            lit += c != 0;
            px[(size_t)row * cw + col] = c | 0xFF000000;
        }
    if (lit < px.size() / 50) return false;  // (it drew nothing: the card instead)
    if (!ViewTexture(t, cw, ch, px.data())) return false;
    t.w = cw, t.h = ch, t.card = false;
    return true;
}

bool ViewCaptureSupported() {
    if (g_view.captureChecked) return g_view.captureOk;
    g_view.captureChecked = true;
    try {
        g_view.captureOk = winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported();
    } catch (...) {
        g_view.captureOk = false;
    }
    if (g_view.captureOk) {
        static CreateWinrtDeviceFn create = [] {
            HMODULE m = GetModuleHandleW(L"d3d11.dll");
            return m ? (CreateWinrtDeviceFn)(void*)GetProcAddress(m, "CreateDirect3D11DeviceFromDXGIDevice") : nullptr;
        }();
        IDXGIDevice* dxgi = nullptr;
        IUnknown* insp = nullptr;
        if (create && SUCCEEDED(g_view.device->QueryInterface(IID_PPV_ARGS(&dxgi))) && SUCCEEDED(create(dxgi, &insp))) {
            try {
                winrt::Windows::Foundation::IInspectable i{insp, winrt::take_ownership_from_abi};
                g_view.rtDevice = i.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
            } catch (...) {
            }
        } else if (insp) {
            insp->Release();
        }
        ViewRelease(dxgi);
        g_view.captureOk = g_view.rtDevice != nullptr;
    }
    return g_view.captureOk;
}

// Live capture of a window: its frames come in on a capture thread and wait
// in ViewCapture until the next draw takes the newest.
void ViewStartCapture(ViewTile& t) {
    using namespace winrt::Windows::Graphics::Capture;
    using winrt::Windows::Graphics::DirectX::DirectXPixelFormat;
    try {
        auto factory = winrt::get_activation_factory<GraphicsCaptureItem>();
        ITourneCaptureItemInterop* interop = nullptr;
        if (FAILED(((IUnknown*)winrt::get_abi(factory))->QueryInterface(kIidCaptureItemInterop, (void**)&interop))) return;
        void* raw = nullptr;
        winrt::guid iid = winrt::guid_of<GraphicsCaptureItem>();
        HRESULT hr = interop->CreateForWindow(t.wnd, reinterpret_cast<GUID const&>(iid), &raw);
        interop->Release();
        if (FAILED(hr) || !raw) return;
        t.item = GraphicsCaptureItem{raw, winrt::take_ownership_from_abi};
        t.poolSize = t.item.Size();
        if (t.poolSize.Width <= 0 || t.poolSize.Height <= 0) return;
        t.pool = Direct3D11CaptureFramePool::CreateFreeThreaded(g_view.rtDevice, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2,
                                                                t.poolSize);
        t.cap = std::make_shared<ViewCapture>();
        std::weak_ptr<ViewCapture> weak = t.cap;
        t.pool.FrameArrived([weak](Direct3D11CaptureFramePool const& pool, auto const&) {
            auto frame = pool.TryGetNextFrame();
            auto cap = weak.lock();
            if (!frame) return;
            if (!cap) {
                frame.Close();
                return;
            }
            std::lock_guard<std::mutex> hold(cap->lock);
            if (cap->closed) {
                frame.Close();
                return;
            }
            if (cap->pending) cap->pending.Close();
            cap->pending = frame;
        });
        t.session = t.pool.CreateCaptureSession(t.item);
        try {
            t.session.IsCursorCaptureEnabled(false);  // (Windows 10 2004 and later)
        } catch (...) {
        }
        try {
            t.session.IsBorderRequired(false);  // (Windows 11: no yellow border)
        } catch (...) {
        }
        t.session.StartCapture();
    } catch (...) {
        t.session = nullptr, t.pool = nullptr, t.item = nullptr, t.cap = nullptr;
    }
}

void ViewStopCapture(ViewTile& t) {
    if (t.cap) {
        std::lock_guard<std::mutex> hold(t.cap->lock);
        t.cap->closed = true;
        if (t.cap->pending) t.cap->pending.Close();
        t.cap->pending = nullptr;
    }
    try {
        if (t.session) t.session.Close();
        if (t.pool) t.pool.Close();
    } catch (...) {
    }
    t.session = nullptr, t.pool = nullptr, t.item = nullptr, t.cap = nullptr;
}

// The newest frame, copied into the picture.
void ViewTakeFrame(ViewTile& t) {
    if (!t.cap) return;
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame frame{nullptr};
    {
        std::lock_guard<std::mutex> hold(t.cap->lock);
        frame = t.cap->pending;
        t.cap->pending = nullptr;
    }
    if (!frame) return;
    try {
        auto content = frame.ContentSize();
        auto surface = frame.Surface();
        ITourneDxgiAccess* access = nullptr;
        ID3D11Texture2D* src = nullptr;
        if (SUCCEEDED(((IUnknown*)winrt::get_abi(surface))->QueryInterface(kIidDxgiAccess, (void**)&access)) &&
            SUCCEEDED(access->GetInterface(IID_PPV_ARGS(&src)))) {
            D3D11_TEXTURE2D_DESC d;
            src->GetDesc(&d);
            int w = std::clamp((int)content.Width, 1, (int)d.Width), h = std::clamp((int)content.Height, 1, (int)d.Height);
            if (ViewTexture(t, w, h, nullptr)) {
                D3D11_BOX box{0, 0, 0, (UINT)w, (UINT)h, 1};
                g_view.ctx->CopySubresourceRegion(t.tex, 0, 0, 0, 0, src, 0, &box);
                t.w = w, t.h = h, t.card = false;
            }
        }
        ViewRelease(src);
        ViewRelease(access);
        // The window changed size: the pool follows.
        if (content.Width != t.poolSize.Width || content.Height != t.poolSize.Height) {
            t.poolSize = content;
            t.pool.Recreate(g_view.rtDevice, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2,
                            content);
        }
        frame.Close();
    } catch (...) {
    }
}

void ViewFreeTile(ViewTile& t) {
    ViewStopCapture(t);
    ViewRelease(t.srv);
    ViewRelease(t.tex);
}

void ViewFreeScene() {
    for (auto& t : g_view.tiles) ViewFreeTile(*t);
    g_view.tiles.clear();
    ViewRelease(g_view.backSrv);
    ViewRelease(g_view.back);
}

void ViewFreeTarget() {
    if (g_view.dc) g_view.dc->SetTarget(nullptr);
    ViewRelease(g_view.target);
    ViewRelease(g_view.rtv);
}

// Everything, back to nothing (the view turned off, or the switcher closing).
void ViewFree() {
    ViewFreeScene();
    ViewFreeTarget();
    g_view.rtDevice = nullptr;
    g_view.captureChecked = g_view.captureOk = false;
    ViewRelease(g_view.titleFormat);
    ViewRelease(g_view.dwrite);
    ViewRelease(g_view.dc);
    ViewRelease(g_view.d2d);
    ViewRelease(g_view.chain);
    ViewRelease(g_view.raster);
    ViewRelease(g_view.sampler);
    ViewRelease(g_view.blend);
    ViewRelease(g_view.cb);
    ViewRelease(g_view.ps);
    ViewRelease(g_view.vs);
    ViewRelease(g_view.ctx);
    ViewRelease(g_view.device);
    if (g_view.wnd) DestroyWindow(g_view.wnd);
    g_view.wnd = nullptr;
    g_view.shown = false;
    g_view.chainW = g_view.chainH = 0;
}

LRESULT CALLBACK ViewProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ViewMakeDevice() {
    if (g_view.device) return true;
    static D3DCompileFn compile = [] {
        HMODULE m = LoadLibraryExW(L"d3dcompiler_47.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        return m ? (D3DCompileFn)(void*)GetProcAddress(m, "D3DCompile") : nullptr;
    }();
    if (!compile) return false;
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                                 &g_view.device, nullptr, &g_view.ctx)) &&
        FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                                 &g_view.device, nullptr, &g_view.ctx)))
        return false;
    ID3DBlob *vsCode = nullptr, *psCode = nullptr, *errors = nullptr;
    bool ok = SUCCEEDED(compile(kViewShaders, sizeof(kViewShaders) - 1, "views", nullptr, nullptr, "VS", "vs_4_0", 0, 0, &vsCode,
                                &errors));
    if (errors) Wh_Log(L"Views' vertex shader: %S", (const char*)errors->GetBufferPointer());
    ViewRelease(errors);
    ok = ok && SUCCEEDED(compile(kViewShaders, sizeof(kViewShaders) - 1, "views", nullptr, nullptr, "PS", "ps_4_0", 0, 0, &psCode,
                                 &errors));
    if (errors) Wh_Log(L"Views' pixel shader: %S", (const char*)errors->GetBufferPointer());
    ViewRelease(errors);
    ok = ok && SUCCEEDED(g_view.device->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, &g_view.vs));
    ok = ok && SUCCEEDED(g_view.device->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), nullptr, &g_view.ps));
    ViewRelease(vsCode);
    ViewRelease(psCode);
    D3D11_BUFFER_DESC b{sizeof(ViewQuad), D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0};
    ok = ok && SUCCEEDED(g_view.device->CreateBuffer(&b, nullptr, &g_view.cb));
    D3D11_BLEND_DESC bd{};
    auto& rt = bd.RenderTarget[0];
    rt.BlendEnable = TRUE;
    rt.SrcBlend = D3D11_BLEND_SRC_ALPHA, rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA, rt.BlendOp = D3D11_BLEND_OP_ADD;
    rt.SrcBlendAlpha = D3D11_BLEND_ONE, rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA, rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ok = ok && SUCCEEDED(g_view.device->CreateBlendState(&bd, &g_view.blend));
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    ok = ok && SUCCEEDED(g_view.device->CreateSamplerState(&sd, &g_view.sampler));
    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID, rd.CullMode = D3D11_CULL_NONE, rd.DepthClipEnable = TRUE;
    ok = ok && SUCCEEDED(g_view.device->CreateRasterizerState(&rd, &g_view.raster));
    if (!ok) {
        ViewFree();
        return false;
    }
    // (Capture frames come in on another thread; the device is shared with them.)
    ID3D10Multithread* mt = nullptr;
    if (SUCCEEDED(g_view.ctx->QueryInterface(IID_PPV_ARGS(&mt)))) mt->SetMultithreadProtected(TRUE);
    ViewRelease(mt);
    // Titles: Direct2D on the same device (no titles if it can't).
    IDXGIDevice* dxgi = nullptr;
    ID2D1Device* d2dDevice = nullptr;
    D2D1_FACTORY_OPTIONS fo{};
    if (SUCCEEDED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &fo, (void**)&g_view.d2d)) &&
        SUCCEEDED(g_view.device->QueryInterface(IID_PPV_ARGS(&dxgi))) && SUCCEEDED(g_view.d2d->CreateDevice(dxgi, &d2dDevice)))
        d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &g_view.dc);
    ViewRelease(d2dDevice);
    ViewRelease(dxgi);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&g_view.dwrite);
    return true;
}

bool ViewMakeWindow() {
    if (g_view.wnd) return true;
    HINSTANCE instance = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&ViewProc,
                       &instance);
    WNDCLASSW wc{};
    wc.lpfnWndProc = ViewProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kViewClass;
    RegisterClassW(&wc);
    g_view.wnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kViewClass, L"Tourne switcher view",
                                 WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, instance, nullptr);
    if (g_view.wnd) KeepOutOfPeek(g_view.wnd);
    return g_view.wnd != nullptr;
}

// The swap chain, sized to the screen the view opens on.
bool ViewMakeChain(int w, int h) {
    if (g_view.chain && g_view.chainW == w && g_view.chainH == h) return true;
    ViewFreeTarget();
    if (g_view.chain) {
        if (FAILED(g_view.chain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0))) ViewRelease(g_view.chain);
    }
    if (!g_view.chain) {
        IDXGIDevice* dxgi = nullptr;
        IDXGIAdapter* adapter = nullptr;
        IDXGIFactory2* factory = nullptr;
        if (SUCCEEDED(g_view.device->QueryInterface(IID_PPV_ARGS(&dxgi))) && SUCCEEDED(dxgi->GetAdapter(&adapter)) &&
            SUCCEEDED(adapter->GetParent(IID_PPV_ARGS(&factory)))) {
            DXGI_SWAP_CHAIN_DESC1 d{};
            d.Width = w, d.Height = h, d.Format = DXGI_FORMAT_B8G8R8A8_UNORM, d.SampleDesc.Count = 1;
            d.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            for (auto effect : {DXGI_SWAP_EFFECT_FLIP_DISCARD, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL, DXGI_SWAP_EFFECT_DISCARD}) {
                d.SwapEffect = effect;
                d.BufferCount = effect == DXGI_SWAP_EFFECT_DISCARD ? 1 : 2;
                if (SUCCEEDED(factory->CreateSwapChainForHwnd(g_view.device, g_view.wnd, &d, nullptr, nullptr, &g_view.chain))) break;
            }
            if (g_view.chain) factory->MakeWindowAssociation(g_view.wnd, DXGI_MWA_NO_ALT_ENTER);
        }
        ViewRelease(factory);
        ViewRelease(adapter);
        ViewRelease(dxgi);
        if (!g_view.chain) return false;
    }
    ID3D11Texture2D* buffer = nullptr;
    if (FAILED(g_view.chain->GetBuffer(0, IID_PPV_ARGS(&buffer)))) return false;
    HRESULT hr = g_view.device->CreateRenderTargetView(buffer, nullptr, &g_view.rtv);
    ViewRelease(buffer);
    if (FAILED(hr)) return false;
    if (g_view.dc) {
        IDXGISurface* surface = nullptr;
        if (SUCCEEDED(g_view.chain->GetBuffer(0, IID_PPV_ARGS(&surface)))) {
            D2D1_BITMAP_PROPERTIES1 bp{};
            bp.pixelFormat = {DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE};
            bp.dpiX = bp.dpiY = 96;
            bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
            if (SUCCEEDED(g_view.dc->CreateBitmapFromDxgiSurface(surface, &bp, &g_view.target))) g_view.dc->SetTarget(g_view.target);
        }
        ViewRelease(surface);
    }
    g_view.chainW = w, g_view.chainH = h;
    return true;
}

// The screen as it is, for the backdrop (blurred by reading smaller mips).
void ViewTakeBackdrop() {
    int w = g_view.mon.right - g_view.mon.left, h = g_view.mon.bottom - g_view.mon.top;
    ViewDib dib(w, h);
    if (!dib.bits) return;
    HDC screen = GetDC(nullptr);
    BitBlt(dib.dc, 0, 0, w, h, screen, g_view.mon.left, g_view.mon.top, SRCCOPY | CAPTUREBLT);
    ReleaseDC(nullptr, screen);
    dib.Opaque();
    D3D11_TEXTURE2D_DESC d{};
    d.Width = w, d.Height = h, d.MipLevels = 0, d.ArraySize = 1, d.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    d.SampleDesc.Count = 1, d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    d.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if (FAILED(g_view.device->CreateTexture2D(&d, nullptr, &g_view.back))) return;
    g_view.ctx->UpdateSubresource(g_view.back, 0, nullptr, dib.bits, w * 4, 0);
    if (FAILED(g_view.device->CreateShaderResourceView(g_view.back, nullptr, &g_view.backSrv))) return;
    g_view.ctx->GenerateMips(g_view.backSrv);
    g_view.back->GetDesc(&d);
    g_view.backMips = d.MipLevels;
}

// The tiles for the switcher's entries (each app's most recent window), kept
// for windows still there so their captures carry on.
void ViewSync() {
    std::vector<std::unique_ptr<ViewTile>> next;
    for (auto const& e : g_entries) {
        HWND h = e.windows.empty() ? nullptr : e.windows[0];
        std::unique_ptr<ViewTile> tile;
        for (auto& old : g_view.tiles)
            if (old && old->wnd == h) tile = std::move(old);
        if (!tile) {
            tile = std::make_unique<ViewTile>();
            tile->wnd = h;
            tile->iconic = IsIconic(h);
            RECT r{};
            if (FAILED(DwmGetWindowAttribute(h, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(r)))) GetWindowRect(h, &r);
            OffsetRect(&r, -g_view.mon.left, -g_view.mon.top);
            tile->real = r;
            // Live where it can be (a card stands in until the first frame),
            // else a still, else (minimised) the card.
            if (g_settings.viewLive && !tile->iconic && ViewCaptureSupported()) ViewStartCapture(*tile);
            if (tile->session || tile->iconic || !ViewStill(*tile)) ViewCard(*tile, e);
        }
        next.push_back(std::move(tile));
    }
    for (auto& old : g_view.tiles)
        if (old) ViewFreeTile(*old);
    g_view.tiles = std::move(next);
}

////////////////////////////////////////////////////////////////////////////////
// The scene: where each picture goes. World units are the screen's pixels,
// x right and y up from its centre; depth 0 is the screen itself (a picture
// there is drawn at its own size), more is further away.

struct ViewPlace {
    float x = 0, y = 0, depth = 0, turn = 0;  // turn: degrees about the vertical
    float w = 0, h = 0, alpha = 1, shade = 1;
    bool reflect = false;
    float floorY = 0;
};

ViewPlace Lerp(ViewPlace const& a, ViewPlace const& b, float t) {
    ViewPlace r = b;
    auto mix = [t](float p, float q) { return p + (q - p) * t; };
    r.x = mix(a.x, b.x), r.y = mix(a.y, b.y), r.depth = mix(a.depth, b.depth), r.turn = mix(a.turn, b.turn);
    r.w = mix(a.w, b.w), r.h = mix(a.h, b.h), r.alpha = mix(a.alpha, b.alpha), r.shade = mix(a.shade, b.shade);
    return r;
}

void Fit(ViewTile const& t, float boxW, float boxH, float* w, float* h) {
    float tw = (float)std::max(1, t.w), th = (float)std::max(1, t.h);
    float k = std::min(boxW / tw, boxH / th);
    *w = tw * k, *h = th * k;
}

// Flip 3D: a slanted stack going back and up to the right, the chosen window
// in front; one leaving the front flies past you and fades.
ViewPlace FlipPlace(ViewTile const& t, float s, float W, float H) {
    ViewPlace p;
    Fit(t, W * 0.5f, H * 0.55f, &p.w, &p.h);
    p.turn = 32;
    if (s >= 0) {
        p.x = -W * 0.12f + s * W * 0.09f;
        p.y = -H * 0.02f + s * H * 0.035f;
        p.depth = s * H * 0.38f;
        p.alpha = std::clamp(1 - (s - 7) / 2, 0.f, 1.f);
        p.shade = 1 - std::min(s * 0.06f, 0.45f);
    } else {
        float u = -s;
        p.x = -W * 0.12f - u * W * 0.22f;
        p.y = -H * 0.02f - u * H * 0.08f;
        p.depth = -u * H * 0.45f;
        p.alpha = 1 - u;
    }
    return p;
}

// Cover Flow: the chosen one flat in the middle, the rest turned to face it,
// standing on a floor that reflects them.
ViewPlace CoverPlace(ViewTile const& t, float s, float W, float H) {
    ViewPlace p;
    Fit(t, W * 0.36f, H * 0.42f, &p.w, &p.h);
    float a = std::clamp(s, -1.f, 1.f), beyond = s - a;
    p.x = a * W * 0.27f + beyond * W * 0.065f;
    p.depth = std::fabs(a) * H * 0.32f;
    p.turn = -a * 60;
    p.floorY = -H * 0.14f;
    p.y = p.floorY + p.h / 2;
    p.alpha = std::clamp(1 - (std::fabs(s) - 6) / 2, 0.f, 1.f);
    p.shade = 1 - std::fabs(a) * 0.3f;
    p.reflect = true;
    return p;
}

// Grid and Mission Control: every window at once in a grid filling the
// screen, the last row centred; the chosen one lifted toward you and lit.
ViewPlace GridPlace(ViewTile const& t, int slot, int n, float lift, float W, float H) {
    ViewPlace p;
    n = std::max(n, 1);
    int cols = std::clamp((int)std::ceil(std::sqrt(n * (W / H) / 1.6f)), 1, n);
    int rows = (n + cols - 1) / cols;
    float cw = W * 0.9f / cols, ch = H * 0.76f / rows;
    Fit(t, cw * 0.86f, ch * 0.82f, &p.w, &p.h);
    int r = slot / cols, c = slot % cols;
    int inRow = r == rows - 1 ? n - r * cols : cols;
    p.x = -inRow * cw / 2 + (c + 0.5f) * cw;
    p.y = H / 2 - H * 0.06f - (r + 0.5f) * ch;
    p.depth = -lift * H * 0.05f;
    p.shade = 0.74f + 0.26f * lift;
    return p;
}

// Mission Control's order: the grid read the way the windows sit on the
// screen (in bands from the top, then left to right), minimised ones last.
std::vector<int> MissionSlots(float H) {
    int n = (int)g_view.tiles.size();
    std::vector<int> order(n), slots(n);
    for (int i = 0; i < n; i++) order[i] = i;
    auto key = [H](ViewTile const& t) {
        if (t.iconic || IsRectEmpty(&t.real)) return std::make_pair(1e9f, 0.f);
        float band = std::floor((t.real.top + t.real.bottom) / 2.f / (H / 3));
        return std::make_pair(band, (t.real.left + t.real.right) / 2.f);
    };
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return key(*g_view.tiles[a]) < key(*g_view.tiles[b]); });
    for (int k = 0; k < n; k++) slots[order[k]] = k;
    return slots;
}

// Ring: a circle standing on the floor, facing out, turning to bring the
// chosen one to the front; the far side dims and fades.
ViewPlace RingPlace(ViewTile const& t, float s, int n, float W, float H) {
    ViewPlace p;
    Fit(t, W * 0.3f, H * 0.36f, &p.w, &p.h);
    constexpr float kPi = 3.14159265f;
    float a = 2 * kPi * s / std::max(n, 1);
    float R = std::max(W * 0.28f, std::max(n, 1) * W * 0.3f / (2 * kPi));
    p.x = R * std::sin(a);
    p.depth = R * (1 - std::cos(a));
    p.turn = a * 180 / kPi;
    p.floorY = -H * 0.16f;
    p.y = p.floorY + p.h / 2;
    float facing = std::cos(a);
    p.shade = 0.4f + 0.6f * std::max(0.f, facing);
    p.alpha = std::clamp((facing + 0.5f) / 0.5f, 0.f, 1.f);
    p.reflect = true;
    return p;
}

// Shelf: one row on the floor like a dock, every window in it, magnified
// under the pointer (or round the chosen one, from the keys), the neighbours
// moving aside.
float ShelfCell(int n, float W) { return std::min(W * 0.9f / std::max(n, 1), W * 0.16f); }
float ShelfX(int i, int n, float W) { return (i - (n - 1) / 2.f) * ShelfCell(n, W); }

ViewPlace ShelfPlace(ViewTile const& t, int i, int n, float lit, float W, float H) {
    ViewPlace p;
    float cell = ShelfCell(n, W), x = ShelfX(i, n, W);
    float d = (x - g_view.focusX) / cell, nearness = std::exp(-d * d * 0.6f), mag = 1 + 0.7f * nearness;
    Fit(t, cell * 0.82f * mag, H * 0.2f * mag, &p.w, &p.h);
    p.x = x + cell * 0.32f * std::tanh(d * 0.9f);
    p.floorY = -H * 0.22f;
    p.y = p.floorY + p.h / 2 + lit * H * 0.02f;
    p.shade = std::max(0.72f + 0.28f * nearness, 0.72f + 0.28f * lit);
    p.reflect = true;
    return p;
}

bool ViewLights() { return g_view.kind == 3 || g_view.kind == 4 || g_view.kind == 6; }  // a glow on the chosen one

bool ViewGoesRound() { return g_view.kind == 1 || g_view.kind == 5; }  // Flip 3D and Ring
bool ViewIsGrid() { return g_view.kind == 3 || g_view.kind == 4 || g_view.kind == 6; }  // (all in sight: a click switches)

// Where it is on the screen now: where the scene flies in from.
ViewPlace RealPlace(ViewTile const& t, ViewPlace const& placed, float W, float H) {
    ViewPlace p = placed;
    if (t.iconic || IsRectEmpty(&t.real)) {
        p.alpha = 0;
        p.w *= 0.8f, p.h *= 0.8f;
        return p;
    }
    p.x = (t.real.left + t.real.right) / 2.f - W / 2;
    p.y = H / 2 - (t.real.top + t.real.bottom) / 2.f;
    p.depth = 0, p.turn = 0;
    p.w = (float)(t.real.right - t.real.left), p.h = (float)(t.real.bottom - t.real.top);
    p.alpha = 1, p.shade = 1;
    return p;
}

// Row-vector matrices (as the shader's mul(v, M)).
struct Mat {
    float m[16];
    static Mat Identity() { return {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}}; }
    Mat operator*(Mat const& b) const {
        Mat r{};
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) {
                float v = 0;
                for (int k = 0; k < 4; k++) v += m[i * 4 + k] * b.m[k * 4 + j];
                r.m[i * 4 + j] = v;
            }
        return r;
    }
};

Mat Scale(float x, float y) {
    Mat r = Mat::Identity();
    r.m[0] = x, r.m[5] = y;
    return r;
}
Mat TurnY(float degrees) {
    float a = degrees * 3.14159265f / 180, c = std::cos(a), s = std::sin(a);
    Mat r = Mat::Identity();
    r.m[0] = c, r.m[2] = -s, r.m[8] = s, r.m[10] = c;
    return r;
}
Mat Move(float x, float y, float z) {
    Mat r = Mat::Identity();
    r.m[12] = x, r.m[13] = y, r.m[14] = z;
    return r;
}
// The camera D px from the screen: something at depth 0 is drawn 1:1.
Mat Project(float W, float H, float D) {
    Mat r{};
    r.m[0] = 2 * D / W, r.m[5] = 2 * D / H, r.m[10] = 0.5f, r.m[11] = 1;
    return r;
}

void ViewDrawQuad(Mat const& m, float w, float h, float radius, int mode, float const tint[4], float const extra[4],
                  ID3D11ShaderResourceView* srv, float u1, float v1) {
    ViewQuad q{};
    memcpy(q.m, m.m, sizeof(q.m));
    q.uvRect[0] = 0, q.uvRect[1] = 0, q.uvRect[2] = u1, q.uvRect[3] = v1;
    q.size[0] = w, q.size[1] = h, q.size[2] = radius, q.size[3] = (float)mode;
    memcpy(q.tint, tint, sizeof(q.tint));
    memcpy(q.extra, extra, sizeof(q.extra));
    g_view.ctx->UpdateSubresource(g_view.cb, 0, nullptr, &q, 0, 0);
    g_view.ctx->PSSetShaderResources(0, 1, &srv);
    g_view.ctx->Draw(4, 0);
}

void ViewRender() {
    if (!g_view.shown || !g_view.rtv) return;
    float W = (float)g_view.chainW, H = (float)g_view.chainH, D = H * 1.15f;
    for (auto& t : g_view.tiles) ViewTakeFrame(*t);

    // Where the selection is, on its way to the new one.
    double move = ViewProgress(g_view.moveStart);
    g_view.pos = g_view.from + (g_view.to - g_view.from) * move;
    float open = (float)ViewProgress(g_view.openStart);

    D3D11_VIEWPORT vp{0, 0, W, H, 0, 1};
    g_view.ctx->RSSetViewports(1, &vp);
    g_view.ctx->RSSetState(g_view.raster);
    g_view.ctx->OMSetRenderTargets(1, &g_view.rtv, nullptr);
    float black[4] = {0, 0, 0, 1};
    g_view.ctx->ClearRenderTargetView(g_view.rtv, black);
    g_view.ctx->OMSetBlendState(g_view.blend, nullptr, 0xFFFFFFFF);
    g_view.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    g_view.ctx->IASetInputLayout(nullptr);
    g_view.ctx->VSSetShader(g_view.vs, nullptr, 0);
    g_view.ctx->PSSetShader(g_view.ps, nullptr, 0);
    g_view.ctx->VSSetConstantBuffers(0, 1, &g_view.cb);
    g_view.ctx->PSSetConstantBuffers(0, 1, &g_view.cb);
    g_view.ctx->PSSetSamplers(0, 1, &g_view.sampler);

    Mat proj = Project(W, H, D);
    // The backdrop: the screen, dimmed and blurred as the view opens.
    if (g_view.backSrv) {
        float dim = 1 - open * g_settings.viewDim / 100.f;
        float lod = std::min(open * g_settings.viewBlur * 0.6f, (float)g_view.backMips - 1);
        float tint[4] = {dim, dim, dim, 1}, extra[4] = {0, 0, lod, 1.f / W};
        ViewDrawQuad(Scale(W, H) * Move(0, 0, D) * proj, W, H, 0, 3, tint, extra, g_view.backSrv, 1, 1);
    }

    // Each tile's place, then drawn back to front.
    int n = (int)g_view.tiles.size();
    struct Item {
        ViewTile* t;
        ViewPlace p;
        float order;
        int index;
        float lit;
    };
    std::vector<Item> items;
    std::vector<int> slots;
    if (g_view.kind == 4) slots = MissionSlots(H);
    int to = (int)std::lround(g_view.to);
    if (g_view.kind == 6 && n > 0) {  // Shelf: the magnified spot glides to the pointer, or to the chosen one
        float target = g_view.pointer ? g_view.pointerAt.x - W / 2 : ShelfX(std::clamp(to, 0, n - 1), n, W);
        if (!g_view.focusSet) g_view.focusX = target, g_view.focusSet = true;
        g_view.focusX += (target - g_view.focusX) * 0.25f;
    }
    for (int i = 0; i < n; i++) {
        ViewTile& t = *g_view.tiles[i];
        t.drawn = false;
        float s = (float)(i - g_view.pos);
        if (g_view.kind == 1 && n > 1) {
            s = std::fmod(s, (float)n);
            if (s <= -1) s += n;
            if (s > n - 1) s -= n;
        } else if (g_view.kind == 5 && n > 1) {  // (the nearer way round)
            s = std::fmod(s, (float)n);
            if (s > n / 2.f) s -= n;
            if (s <= -n / 2.f) s += n;
        }
        ViewPlace p;
        float order = std::fabs(s);
        float lit = ViewLights() ? (i == to ? (float)move : 0) + (i == g_view.selFrom && g_view.selFrom != to ? 1 - (float)move : 0) : 0;
        switch (g_view.kind) {
            case 1:
                p = FlipPlace(t, s, W, H), order = s;
                break;
            case 3:
            case 4:
                p = GridPlace(t, slots.empty() ? i : slots[i], n, lit, W, H);
                order = 1 - lit;
                break;
            case 5:
                p = RingPlace(t, s, n, W, H), order = p.depth;
                break;
            case 6:
                p = ShelfPlace(t, i, n, lit, W, H);
                order = 1 - p.w / std::max(1.f, ShelfCell(n, W));  // (the bigger in front)
                break;
            default:
                p = CoverPlace(t, s, W, H);
        }
        if (open < 1) p = Lerp(RealPlace(t, p, W, H), p, open);
        t.order = order;
        if (p.alpha <= 0.01f || !t.srv) continue;
        items.push_back({&t, p, order, i, lit});
    }
    std::sort(items.begin(), items.end(), [](Item const& a, Item const& b) { return a.order > b.order; });
    float scale = g_dpi / 96.f;
    float radius = (g_settings.radius > 0 ? g_settings.radius : 8) * scale;
    for (auto& it : items) {
        ViewTile& t = *it.t;
        ViewPlace const& p = it.p;
        Mat place = TurnY(p.turn) * Move(p.x, p.y, D + p.depth);
        float u1 = (float)t.w / std::max(1, t.texW), v1 = (float)t.h / std::max(1, t.texH);
        if (p.reflect && open > 0) {
            // Mirrored below its floor line, fading away from it.
            float mirrorY = 2 * p.floorY - p.y;
            Mat m = Scale(p.w, -p.h) * TurnY(p.turn) * Move(p.x, mirrorY, D + p.depth) * proj;
            float tint[4] = {p.shade, p.shade, p.shade, p.alpha * open}, extra[4] = {0, 0.32f, 0, 0};
            ViewDrawQuad(m, p.w, p.h, radius, 2, tint, extra, t.srv, u1, v1);
        }
        // A soft shadow, then the picture.
        float spread = 26 * scale;
        Mat shadow = Scale(p.w + spread * 2, p.h + spread * 2) * Move(0, -spread * 0.4f, 0) * place * proj;
        float stint[4] = {0, 0, 0, 0.5f * p.alpha}, sextra[4] = {spread, 0, 0, 0};
        ViewDrawQuad(shadow, p.w + spread * 2, p.h + spread * 2, radius, 1, stint, sextra, t.srv, 1, 1);
        if (it.lit > 0) {
            // The chosen one (grids, Shelf): a glow in the highlight's colour round it.
            float glow = 10 * scale, lit = it.lit * p.alpha * open;
            uint32_t c = g_settings.look.highlightBorder;
            Mat ring = Scale(p.w + glow * 2, p.h + glow * 2) * place * proj;
            float gtint[4] = {((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, 0.9f * lit};
            float gextra[4] = {glow, 0, 0, 0};
            ViewDrawQuad(ring, p.w + glow * 2, p.h + glow * 2, radius + glow, 1, gtint, gextra, t.srv, 1, 1);
        }
        Mat m = Scale(p.w, p.h) * place * proj;
        float tint[4] = {p.shade, p.shade, p.shade, p.alpha}, extra[4] = {0, 0, 0, 0};
        ViewDrawQuad(m, p.w, p.h, radius, 0, tint, extra, t.srv, u1, v1);
        // Its corners on the screen, for clicks.
        float corners[4][2] = {{-0.5f, 0.5f}, {0.5f, 0.5f}, {0.5f, -0.5f}, {-0.5f, -0.5f}};
        for (int c = 0; c < 4; c++) {
            float v[4] = {corners[c][0], corners[c][1], 0, 1}, o[4] = {};
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++) o[j] += v[k] * m.m[k * 4 + j];
            float w = o[3] > 0.001f ? o[3] : 0.001f;
            t.quad[c] = {(LONG)((o[0] / w + 1) / 2 * W), (LONG)((1 - o[1] / w) / 2 * H)};
        }
        t.drawn = true;
    }

    // The chosen window's title (and its app's, when that's different).
    if (g_view.dc && g_view.target && g_view.dwrite && g_sel >= 0 && g_sel < (int)g_entries.size() && open > 0) {
        if (!g_view.titleFormat)
            g_view.dwrite->CreateTextFormat(g_settings.font.empty() ? L"Segoe UI" : g_settings.font.c_str(), nullptr,
                                            g_settings.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                                            DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                                            std::max(14.f, g_settings.fontSize * 2.2f) * scale, L"", &g_view.titleFormat);
        if (g_view.titleFormat) {
            g_view.titleFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            g_view.titleFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            g_view.dc->BeginDraw();
            ID2D1SolidColorBrush *text = nullptr, *shade = nullptr;
            uint32_t c = g_settings.look.selectedText;
            g_view.dc->CreateSolidColorBrush(D2D1::ColorF(((c >> 16) & 255) / 255.f, ((c >> 8) & 255) / 255.f, (c & 255) / 255.f, open),
                                             &text);
            g_view.dc->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0.6f * open), &shade);
            std::wstring const& title = g_entries[g_sel].label;
            float y = g_view.kind == 6 ? H * 0.84f : ViewIsGrid() || g_view.kind == 1 ? H * 0.88f : H * 0.80f;
            D2D1_RECT_F r{W * 0.1f, y, W * 0.9f, y + 60 * scale};
            D2D1_RECT_F under{r.left + 2 * scale, r.top + 2 * scale, r.right + 2 * scale, r.bottom + 2 * scale};
            if (shade) g_view.dc->DrawText(title.c_str(), (UINT32)title.size(), g_view.titleFormat, under, shade);
            if (text) g_view.dc->DrawText(title.c_str(), (UINT32)title.size(), g_view.titleFormat, r, text);
            ViewRelease(text);
            ViewRelease(shade);
            g_view.dc->EndDraw();
        }
    }
    g_view.chain->Present(1, 0);
}

bool PointIn(POINT const q[4], POINT p) {
    int sign = 0;
    for (int i = 0; i < 4; i++) {
        POINT a = q[i], b = q[(i + 1) % 4];
        long long cross = (long long)(b.x - a.x) * (p.y - a.y) - (long long)(b.y - a.y) * (p.x - a.x);
        int s = cross > 0 ? 1 : cross < 0 ? -1 : 0;
        if (!s) continue;
        if (sign && s != sign) return false;
        sign = s;
    }
    return true;
}

// The tile under a point, the frontmost first.
int ViewItemAt(POINT pt) {
    int best = -1;
    float bestOrder = 1e9f;
    int n = (int)g_view.tiles.size();
    for (int i = 0; i < n; i++) {
        auto& t = *g_view.tiles[i];
        if (!t.drawn || !PointIn(t.quad, pt)) continue;
        if (t.order < bestOrder) bestOrder = t.order, best = i;
    }
    return best;
}

LRESULT CALLBACK ViewProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_TIMER:
            if (wParam == kViewTimerFrame) ViewRender();
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            ValidateRect(wnd, nullptr);
            ViewRender();
            return 0;
        case WM_LBUTTONUP: {
            HWND sw = g_wnd;
            int at = ViewItemAt({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (at < 0) {
                if (sw) Hide(sw);  // (a click on the backdrop: back where you were)
            } else if (!ViewIsGrid() && g_view.kind != 1 && at != g_sel) {
                g_sel = at;  // Cover Flow, Ring, Shelf: a side one comes to the middle
                if (sw) Refit(sw);
            } else if (sw) {
                Commit(sw, at);
            }
            return 0;
        }
        case WM_MOUSEMOVE: {
            POINT at{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (g_view.pointer || std::abs(at.x - g_view.pointerAt.x) + std::abs(at.y - g_view.pointerAt.y) > 4) {
                if (g_view.pointerAt.x || g_view.pointerAt.y) g_view.pointer = true;  // (moved, not just there as it opened)
                g_view.pointerAt = at;
            }
            return 0;
        }
        case WM_MBUTTONUP: {
            int at = ViewItemAt({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
            if (at >= 0 && g_wnd) CloseItem(g_wnd, at);
            return 0;
        }
    }
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// Shows the view for Alt+Tab instead of the list; false if it can't (the
// list is shown then).
bool ViewOpen() {
    if (!ViewWanted() || g_entries.empty()) return false;
    if (!ViewMakeWindow() || !ViewMakeDevice()) return false;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(TargetMonitor(), &mi);
    g_view.mon = mi.rcMonitor;
    int w = mi.rcMonitor.right - mi.rcMonitor.left, h = mi.rcMonitor.bottom - mi.rcMonitor.top;
    if (!ViewMakeChain(w, h)) {
        ViewFree();
        return false;
    }
    g_view.kind = ViewKind();
    ViewRelease(g_view.titleFormat);  // (the font may have changed)
    motion::Forget();
    g_view.anim = motion::Get(L"switcher");
    ViewFreeScene();
    ViewTakeBackdrop();
    ViewSync();
    g_view.pos = g_view.from = g_view.to = g_sel;
    g_view.lastSel = g_view.selFrom = g_sel;
    g_view.moveStart = g_view.openStart = GetTickCount64();
    g_view.focusSet = g_view.pointer = false;
    g_view.pointerAt = {};
    g_view.shown = true;
    SetWindowPos(g_view.wnd, HWND_TOPMOST, g_view.mon.left, g_view.mon.top, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ViewRender();
    SetTimer(g_view.wnd, kViewTimerFrame, 15, nullptr);
    return true;
}

bool ViewShown() { return g_view.shown; }

// Made ready ahead (the view chosen): the first Alt+Tab opens at once.
void ViewPrepare() {
    if (ViewWanted() && ViewMakeWindow()) ViewMakeDevice();
}

// The selection or the list changed (Tab, a closed window, Ctrl into an app).
void ViewRefit() {
    if (!g_view.shown) return;
    bool same = g_view.tiles.size() == g_entries.size();
    for (size_t i = 0; same && i < g_entries.size(); i++)
        same = !g_entries[i].windows.empty() && g_view.tiles[i]->wnd == g_entries[i].windows[0];
    int n = (int)g_entries.size();
    if (!same) {
        ViewSync();
        g_view.pos = g_view.from = g_view.to = g_sel;
        g_view.selFrom = g_sel;
    } else if (g_sel != g_view.lastSel && n > 0) {
        double target = g_sel;
        g_view.selFrom = g_view.lastSel;
        g_view.pointer = false;  // (the keys take the Shelf's magnifier back)
        if (ViewGoesRound()) {
            // Flip 3D and Ring go round: the shortest way to the new one.
            int d = ((g_sel - g_view.lastSel) % n + n) % n;
            if (d > n / 2) d -= n;
            target = g_view.to + d;
        }
        g_view.from = g_view.pos;
        g_view.to = target;
        g_view.moveStart = GetTickCount64();
    }
    g_view.lastSel = g_sel;
}

void ViewClose() {
    if (!g_view.wnd) return;
    KillTimer(g_view.wnd, kViewTimerFrame);
    ShowWindow(g_view.wnd, SW_HIDE);
    g_view.shown = false;
    ViewFreeScene();
}
