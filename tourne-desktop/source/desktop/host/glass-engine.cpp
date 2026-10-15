#include "glass-engine.h"

#include "screen-effects.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wincodec.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace td {

namespace {

// The shader. Positions are screen pixels from the backdrop's origin; the
// shape is a rounded rectangle (or a ring round a window) given as a signed
// distance: negative inside, so the edge is where it crosses zero.
const char kShader[] = R"(
cbuffer Glass : register(b0) {
    float4 shape;       // x0 y0 x1 y1
    float4 hole;        // x0 y0 x1 y1 (x1 <= x0: none)
    float4 view;        // x0 y0 of the bitmap, w h
    float4 tint;        // rgb, amount
    float2 backSize;    // the backdrop texture, in its pixels
    float radius;
    float holeRadius;
    float blur;
    float refraction;
    float fringe;
    float edgeGlow;
    float highlight;
    float bevel;
    float lens;
    float saturation;
    float brightness;
    float shadow;
    float oil;
    float time;
    float glow;
    float scale;        // the screen's scaling
    float backScale;    // backdrop pixels per screen pixel
    float effect;       // screen-effects.h's number (0: none)
    float effectAmount;
    float3 pad;
};

Texture2D back : register(t0);
SamplerState smp : register(s0);

float4 VS(uint id : SV_VertexID) : SV_Position {
    float2 uv = float2((id << 1) & 2, id & 2);
    return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

float Box(float2 p, float4 r, float rad) {
    float2 c = (r.xy + r.zw) * 0.5, h = (r.zw - r.xy) * 0.5;
    rad = min(rad, min(h.x, h.y));
    float2 q = abs(p - c) - h + rad;
    return length(max(q, 0)) + min(max(q.x, q.y), 0) - rad;
}

float Shape(float2 p) {
    float d = Box(p, shape, radius * scale);
    if (hole.z > hole.x) d = max(d, -Box(p, hole, holeRadius * scale));
    return d;
}

float3 Take(float2 px, float lod) {
    return back.SampleLevel(smp, px * backScale / backSize, lod).rgb;
}

// A soft blur: twelve taps on a spiral, from a smaller copy for wide blurs.
float3 Blurred(float2 px) {
    float b = blur * scale;
    if (b < 0.75) return Take(px, 0);
    float lod = max(log2(b * backScale) - 1.5, 0);
    float3 sum = 0;
    float weights = 0;
    [unroll] for (int i = 0; i < 12; i++) {
        float a = i * 2.39996323 + 0.7;
        float r = sqrt((i + 0.5) / 12.0) * b;
        float w = exp(-2.0 * r * r / (b * b));
        sum += Take(px + float2(cos(a), sin(a)) * r, lod) * w;
        weights += w;
    }
    return sum / weights;
}

float Hash(float2 p) { return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453); }
float Noise(float2 p) {
    float2 i = floor(p), f = frac(p), u = f * f * (3 - 2 * f);
    return lerp(lerp(Hash(i), Hash(i + float2(1, 0)), u.x), lerp(Hash(i + float2(0, 1)), Hash(i + float2(1, 1)), u.x), u.y);
}

float4 PS(float4 pos : SV_Position) : SV_Target {
    float2 p = view.xy + pos.xy;
    float d = Shape(p);

    // A soft shadow under it, a little lower.
    float shade = 0;
    if (shadow > 0 && hole.z <= hole.x) {
        float s = 14 * scale, ds = max(Shape(p - float2(0, 4 * scale)), 0);
        shade = shadow * 0.45 * exp(-ds * ds / (2 * s * s));
    }
    float a = saturate(0.5 - d);
    if (a <= 0) return float4(0, 0, 0, shade);

    // Which way the edge faces (outwards), and how far into the curve.
    float e = 0.75;
    float2 n = float2(Shape(p + float2(e, 0)) - Shape(p - float2(e, 0)), Shape(p + float2(0, e)) - Shape(p - float2(0, e)));
    n = dot(n, n) > 1e-8 ? normalize(n) : float2(0, 0);
    float bev = max(bevel * scale, 1);
    float t = saturate(-d / bev);
    float rim = 1 - t;
    float bend = rim * rim;

    // A lens: the middle magnified.
    float2 c = (shape.xy + shape.zw) * 0.5, h = max((shape.zw - shape.xy) * 0.5, 1);
    float2 q = p;
    if (lens > 0) {
        float2 rel = (p - c) / h;
        q = c + (p - c) * (1 - lens * 0.35 * saturate(1 - dot(rel, rel)));
    }
    int kind = (int)effect;
    if (kind > 0) q = EffectWarp(q, kind, effectAmount, time, scale);

    // Refraction at the edge, with the colours split apart (fringe).
    float2 off = n * bend * refraction * bev * 0.9;
    float2 split = n * bend * fringe * bev * 0.35;
    float3 col;
    col.r = Blurred(q + off + split).r;
    col.g = Blurred(q + off).g;
    col.b = Blurred(q + off - split).b;

    float lum = dot(col, float3(0.2126, 0.7152, 0.0722));
    col = lerp(lum.xxx, col, saturation) + brightness;
    col = lerp(col, tint.rgb, tint.a);
    if (kind > 0) col = EffectColour(col, q, c, h, kind, effectAmount, time, scale, tint.rgb);

    // Edge glow, the light from above, and a thin bright line at the edge.
    float2 light = normalize(float2(-0.45, -1));
    float facing = dot(n, light);
    col += edgeGlow * 0.55 * pow(rim, 3) * lerp(float3(1, 1, 1), tint.rgb, 0.25);
    col += highlight * 0.8 * pow(saturate(facing), 4) * pow(rim, 1.5);
    col += highlight * 0.22 * pow(saturate(-facing), 4) * pow(rim, 2);
    col += highlight * 0.35 * saturate(1 + d / (1.2 * scale)) * (0.4 + 0.6 * saturate(facing * 0.5 + 0.5));

    // The oil film: slow swirls of thin-film colour.
    if (oil > 0) {
        float2 o = p / (180 * scale);
        float ph = Noise(o + time * 0.05) * 2.5 + Noise(o * 2.3 - time * 0.08) * 1.5 + t * 0.8;
        float3 irid = 0.5 + 0.5 * cos(6.2831853 * (ph + float3(0, 0.33, 0.67)));
        col = lerp(col, lerp(col, irid, 0.55) + 0.06, oil * (0.45 + 0.55 * rim));
    }
    col += glow * 0.5 * pow(rim, 1.2);

    col = saturate(col);
    return float4(col * a, a + shade * (1 - a));
}
)";

struct Constants {
    float shape[4], hole[4], view[4], tint[4];
    float backSize[2], radius, holeRadius;
    float blur, refraction, fringe, edgeGlow;
    float highlight, bevel, lens, saturation;
    float brightness, shadow, oil, time;
    float glow, scale, backScale, effect;
    float effectAmount, pad[3];
};
static_assert(sizeof(Constants) % 16 == 0, "constant buffers are whole 16-byte rows");

template <class T> void Release(T*& p) {
    if (p) p->Release();
    p = nullptr;
}

using D3DCompileFn = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT,
                                      ID3DBlob**, ID3DBlob**);

// d3dcompiler_47 comes with Windows 10 and 11; taken when needed, not linked.
ID3DBlob* Compile(const char* entry, const char* target) {
    static D3DCompileFn compile = [] {
        HMODULE m = LoadLibraryExW(L"d3dcompiler_47.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        return m ? (D3DCompileFn)(void*)GetProcAddress(m, "D3DCompile") : nullptr;
    }();
    if (!compile) return nullptr;
    ID3DBlob *code = nullptr, *errors = nullptr;
    // The effects (screen-effects.h) go in before PS, with what they take the picture from.
    std::string src = kShader;
    size_t ps = src.find("float4 PS(");
    src.insert(ps, std::string("float3 EffTake(float2 px) { return Blurred(px); }\nfloat3 EffBloom(float2 px) { return Take(px, 4); }\n") +
                       kEffectsHlsl);
    HRESULT hr = compile(src.data(), src.size(), "glass", nullptr, nullptr, entry, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
                         &code, &errors);
    if (errors) {
        OutputDebugStringA((const char*)errors->GetBufferPointer());
        errors->Release();
    }
    return SUCCEEDED(hr) ? code : nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Pictures

IWICImagingFactory* Wic() {
    IWICImagingFactory* f = nullptr;
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&f));
    return f;
}

// A picture as BGRA, made w x h (stretched to it; the caller keeps its shape).
bool LoadScaled(IWICImagingFactory* f, const std::wstring& path, UINT w, UINT h, std::vector<uint32_t>* out) {
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICBitmapScaler* scaler = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool ok = false;
    if (SUCCEEDED(f->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
        SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(f->CreateBitmapScaler(&scaler)) &&
        SUCCEEDED(scaler->Initialize(frame, w, h, WICBitmapInterpolationModeFant)) && SUCCEEDED(f->CreateFormatConverter(&converter)) &&
        SUCCEEDED(converter->Initialize(scaler, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom))) {
        out->assign((size_t)w * h, 0);
        ok = SUCCEEDED(converter->CopyPixels(nullptr, w * 4, w * h * 4, (BYTE*)out->data()));
    }
    if (converter) converter->Release();
    if (scaler) scaler->Release();
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    return ok;
}

bool PictureSize(IWICImagingFactory* f, const std::wstring& path, UINT* w, UINT* h) {
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    bool ok = SUCCEEDED(f->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)) &&
              SUCCEEDED(decoder->GetFrame(0, &frame)) && SUCCEEDED(frame->GetSize(w, h)) && *w && *h;
    if (frame) frame->Release();
    if (decoder) decoder->Release();
    return ok;
}

// The picture copied into the backdrop at (x, y), only inside clip.
void Blit(Backdrop* b, const std::vector<uint32_t>& src, int sw, int sh, int x, int y, RECT clip) {
    clip.left = std::max<LONG>(clip.left, 0), clip.top = std::max<LONG>(clip.top, 0);
    clip.right = std::min<LONG>(clip.right, b->width), clip.bottom = std::min<LONG>(clip.bottom, b->height);
    for (int row = std::max<int>(y, clip.top); row < std::min<int>(y + sh, clip.bottom); row++) {
        int from = std::max<int>(x, clip.left), to = std::min<int>(x + sw, clip.right);
        if (to <= from) continue;
        memcpy(&b->pixels[(size_t)row * b->width + from], &src[(size_t)(row - y) * sw + (from - x)], (size_t)(to - from) * 4);
    }
}

// Windows' placements (DESKTOP_WALLPAPER_POSITION) of a w x h picture in a
// box: the picture's size and where it goes.
void Place(int position, UINT iw, UINT ih, const RECT& box, int* w, int* h, int* x, int* y) {
    int bw = box.right - box.left, bh = box.bottom - box.top;
    double sx = (double)bw / iw, sy = (double)bh / ih, s = 1;
    switch (position) {
        case DWPOS_STRETCH: *w = bw, *h = bh; break;
        case DWPOS_FIT: s = std::min(sx, sy), *w = (int)std::lround(iw * s), *h = (int)std::lround(ih * s); break;
        case DWPOS_CENTER:
        case DWPOS_TILE: *w = (int)iw, *h = (int)ih; break;
        default: s = std::max(sx, sy), *w = (int)std::lround(iw * s), *h = (int)std::lround(ih * s); break;  // fill, span
    }
    *x = box.left + (bw - *w) / 2;
    *y = box.top + (bh - *h) / 2;
    if (position == DWPOS_TILE) *x = box.left, *y = box.top;
}

void Fill(Backdrop* b, uint32_t argb) { b->pixels.assign((size_t)b->width * b->height, argb | 0xFF000000); }

std::wstring TranscodedWallpaper() {
    PWSTR path = nullptr;
    std::wstring out;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path)))
        out = std::wstring(path) + L"\\Microsoft\\Windows\\Themes\\TranscodedWallpaper";
    CoTaskMemFree(path);
    return GetFileAttributesW(out.c_str()) == INVALID_FILE_ATTRIBUTES ? L"" : out;
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// Backdrops

void GradientBackdrop(int width, int height, Backdrop* out) {
    out->width = width, out->height = height, out->scale = 1;
    out->pixels.resize((size_t)width * height);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            float u = (float)x / width, v = (float)y / height;
            int r = (int)(30 + 120 * u * (1 - v)), g = (int)(40 + 90 * v), b = (int)(90 + 140 * (1 - u) * v + 40 * u);
            out->pixels[(size_t)y * width + x] = 0xFF000000 | r << 16 | g << 8 | b;
        }
}

bool PictureBackdrop(const std::wstring& path, int width, int height, Backdrop* out) {
    IWICImagingFactory* f = Wic();
    if (!f) return false;
    UINT iw, ih;
    bool ok = false;
    if (PictureSize(f, path, &iw, &ih)) {
        out->width = width, out->height = height, out->scale = 1, out->origin = {0, 0};
        Fill(out, 0);
        int w, h, x, y;
        Place(DWPOS_FILL, iw, ih, {0, 0, width, height}, &w, &h, &x, &y);
        std::vector<uint32_t> pic;
        if ((ok = LoadScaled(f, path, w, h, &pic))) Blit(out, pic, w, h, x, y, {0, 0, width, height});
    }
    f->Release();
    return ok;
}

bool ComposeWallpaper(Backdrop* out, const wchar_t* chosen) {
    RECT virt{GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), 0, 0};
    virt.right = virt.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    virt.bottom = virt.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int vw = virt.right - virt.left, vh = virt.bottom - virt.top;
    if (vw <= 0 || vh <= 0) return false;
    // Big desks made smaller: blur and bend don't need every pixel.
    float s = std::min(1.f, 8192.f / std::max(vw, vh));
    out->scale = s;
    out->origin = {virt.left, virt.top};
    out->width = std::max(1, (int)std::lround(vw * s));
    out->height = std::max(1, (int)std::lround(vh * s));
    auto toBack = [&](RECT r) {
        return RECT{(LONG)std::lround((r.left - virt.left) * s), (LONG)std::lround((r.top - virt.top) * s),
                    (LONG)std::lround((r.right - virt.left) * s), (LONG)std::lround((r.bottom - virt.top) * s)};
    };

    IWICImagingFactory* f = Wic();
    if (!f) return false;
    IDesktopWallpaper* dw = nullptr;
    COLORREF bg = 0;
    bool any = false;
    if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&dw)))) {
        dw->GetBackgroundColor(&bg);
        Fill(out, GetRValue(bg) << 16 | GetGValue(bg) << 8 | GetBValue(bg));
        DESKTOP_WALLPAPER_POSITION position = DWPOS_FILL;
        dw->GetPosition(&position);
        UINT count = 0;
        dw->GetMonitorDevicePathCount(&count);
        std::wstring spanned;
        for (UINT i = 0; i < count; i++) {
            LPWSTR id = nullptr;
            if (FAILED(dw->GetMonitorDevicePathAt(i, &id))) continue;
            RECT mon{};
            LPWSTR file = nullptr;
            if (SUCCEEDED(dw->GetMonitorRECT(id, &mon)) && SUCCEEDED(dw->GetWallpaper(id, &file))) {
                std::wstring path = chosen && *chosen ? chosen : file ? file : L"";
                if (path.empty() || GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) path = TranscodedWallpaper();
                UINT iw, ih;
                if (position == DWPOS_SPAN) {
                    spanned = path;
                } else if (!path.empty() && PictureSize(f, path, &iw, &ih)) {
                    RECT box = toBack(mon);
                    int w, h, x, y;
                    Place(position, iw, ih, box, &w, &h, &x, &y);
                    if (position == DWPOS_TILE) w = std::max(1, (int)std::lround(iw * s)), h = std::max(1, (int)std::lround(ih * s));
                    std::vector<uint32_t> pic;
                    if (w > 0 && h > 0 && LoadScaled(f, path, w, h, &pic)) {
                        if (position == DWPOS_TILE)
                            for (int ty = box.top; ty < box.bottom; ty += h)
                                for (int tx = box.left; tx < box.right; tx += w) Blit(out, pic, w, h, tx, ty, box);
                        else
                            Blit(out, pic, w, h, x, y, box);
                        any = true;
                    }
                }
            }
            CoTaskMemFree(file);
            CoTaskMemFree(id);
        }
        UINT iw, ih;
        if (!spanned.empty() && PictureSize(f, spanned, &iw, &ih)) {
            RECT all{0, 0, out->width, out->height};
            int w, h, x, y;
            Place(DWPOS_FILL, iw, ih, all, &w, &h, &x, &y);
            std::vector<uint32_t> pic;
            if (LoadScaled(f, spanned, w, h, &pic)) Blit(out, pic, w, h, x, y, all), any = true;
        }
        dw->Release();
    }
    if (!any) {
        // No wallpaper service (or none set): Windows' own copy, filling the main screen's size everywhere.
        std::wstring copy = TranscodedWallpaper();
        UINT iw, ih;
        if (out->pixels.empty()) Fill(out, 0x202428);
        if (!copy.empty() && PictureSize(f, copy, &iw, &ih)) {
            RECT all{0, 0, out->width, out->height};
            int w, h, x, y;
            Place(DWPOS_FILL, iw, ih, all, &w, &h, &x, &y);
            std::vector<uint32_t> pic;
            if (LoadScaled(f, copy, w, h, &pic)) Blit(out, pic, w, h, x, y, all), any = true;
        }
    }
    f->Release();
    return any || bg;
}

////////////////////////////////////////////////////////////////////////////////
// The engine

GlassEngine::~GlassEngine() { Stop(); }

bool GlassEngine::Start() {
    if (device_) return true;
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                                 &device_, nullptr, &context_)) &&
        FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, flags, levels, ARRAYSIZE(levels), D3D11_SDK_VERSION, &device_,
                                 nullptr, &context_)))
        return false;
    D3D_FEATURE_LEVEL level = device_->GetFeatureLevel();
    const char* vsTarget = level >= D3D_FEATURE_LEVEL_11_0 ? "vs_5_0" : "vs_4_0";
    const char* psTarget = level >= D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0";
    ID3DBlob* vs = Compile("VS", vsTarget);
    ID3DBlob* ps = Compile("PS", psTarget);
    bool ok = vs && ps &&
              SUCCEEDED(device_->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vs_)) &&
              SUCCEEDED(device_->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &ps_));
    if (vs) vs->Release();
    if (ps) ps->Release();
    D3D11_BUFFER_DESC cb{};
    cb.ByteWidth = sizeof(Constants);
    cb.Usage = D3D11_USAGE_DEFAULT;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    ok = ok && SUCCEEDED(device_->CreateBuffer(&cb, nullptr, &constants_)) && SUCCEEDED(device_->CreateSamplerState(&sd, &sampler_));
    if (!ok) Stop();
    return ok;
}

void GlassEngine::Stop() {
    Release(staging_);
    Release(targetView_);
    Release(target_);
    Release(backdropView_);
    Release(backdrop_);
    Release(sampler_);
    Release(constants_);
    Release(ps_);
    Release(vs_);
    Release(context_);
    Release(device_);
    targetW_ = targetH_ = 0;
}

bool GlassEngine::SetBackdrop(const Backdrop& b) {
    if (!device_ || b.width <= 0 || b.height <= 0 || b.pixels.size() < (size_t)b.width * b.height) return false;
    Release(backdropView_);
    Release(backdrop_);
    D3D11_TEXTURE2D_DESC td{};
    td.Width = b.width;
    td.Height = b.height;
    td.MipLevels = 0;  // every level, for wide blurs
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if (FAILED(device_->CreateTexture2D(&td, nullptr, &backdrop_)) ||
        FAILED(device_->CreateShaderResourceView(backdrop_, nullptr, &backdropView_))) {
        Release(backdrop_);
        return false;
    }
    context_->UpdateSubresource(backdrop_, 0, nullptr, b.pixels.data(), b.width * 4, 0);
    context_->GenerateMips(backdropView_);
    copy_.width = b.width, copy_.height = b.height, copy_.origin = b.origin, copy_.scale = b.scale;
    copy_.pixels.clear();
    return true;
}

bool GlassEngine::Target(int w, int h) {
    if (target_ && w == targetW_ && h == targetH_) return true;
    Release(staging_);
    Release(targetView_);
    Release(target_);
    D3D11_TEXTURE2D_DESC td{};
    td.Width = w;
    td.Height = h;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (FAILED(device_->CreateTexture2D(&td, nullptr, &target_)) || FAILED(device_->CreateRenderTargetView(target_, nullptr, &targetView_)))
        return false;
    td.Usage = D3D11_USAGE_STAGING;
    td.BindFlags = 0;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device_->CreateTexture2D(&td, nullptr, &staging_))) return false;
    targetW_ = w, targetH_ = h;
    return true;
}

bool GlassEngine::Render(const GlassLook& look, const GlassShape& s, std::vector<uint32_t>* out) {
    int w = s.view.right - s.view.left, h = s.view.bottom - s.view.top;
    if (!device_ || !backdrop_ || w <= 0 || h <= 0 || w > 16384 || h > 16384 || !Target(w, h)) return false;
    float ox = (float)copy_.origin.x, oy = (float)copy_.origin.y;
    auto rect = [&](float* dst, const RECT& r) {
        dst[0] = r.left - ox, dst[1] = r.top - oy, dst[2] = r.right - ox, dst[3] = r.bottom - oy;
    };
    Constants c{};
    rect(c.shape, s.shape);
    rect(c.hole, s.hole);
    c.view[0] = s.view.left - ox, c.view[1] = s.view.top - oy, c.view[2] = (float)w, c.view[3] = (float)h;
    c.tint[0] = ((look.tint >> 16) & 255) / 255.f, c.tint[1] = ((look.tint >> 8) & 255) / 255.f, c.tint[2] = (look.tint & 255) / 255.f;
    c.tint[3] = std::clamp(look.tintAmount, 0.f, 1.f);
    c.backSize[0] = (float)copy_.width, c.backSize[1] = (float)copy_.height;
    c.radius = look.radius, c.holeRadius = s.holeRadius;
    c.blur = std::max(look.blur, 0.f), c.refraction = look.refraction, c.fringe = look.fringe, c.edgeGlow = look.edgeGlow;
    c.highlight = look.highlight, c.bevel = look.bevel, c.lens = look.lens, c.saturation = look.saturation;
    c.brightness = look.brightness, c.shadow = look.shadow, c.oil = look.oil, c.time = s.time;
    c.glow = s.glow, c.scale = s.scale > 0 ? s.scale : 1, c.backScale = copy_.scale;
    c.effect = (float)look.effect, c.effectAmount = std::clamp(look.effectAmount, 0.f, 1.f);
    context_->UpdateSubresource(constants_, 0, nullptr, &c, 0, 0);

    D3D11_VIEWPORT vp{0, 0, (float)w, (float)h, 0, 1};
    float clear[4] = {0, 0, 0, 0};
    context_->ClearRenderTargetView(targetView_, clear);
    context_->OMSetRenderTargets(1, &targetView_, nullptr);
    context_->RSSetViewports(1, &vp);
    context_->IASetInputLayout(nullptr);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vs_, nullptr, 0);
    context_->PSSetShader(ps_, nullptr, 0);
    context_->PSSetConstantBuffers(0, 1, &constants_);
    context_->PSSetShaderResources(0, 1, &backdropView_);
    context_->PSSetSamplers(0, 1, &sampler_);
    context_->Draw(3, 0);
    ID3D11RenderTargetView* none = nullptr;
    context_->OMSetRenderTargets(1, &none, nullptr);
    context_->CopyResource(staging_, target_);
    D3D11_MAPPED_SUBRESOURCE map;
    if (FAILED(context_->Map(staging_, 0, D3D11_MAP_READ, 0, &map))) return false;
    out->resize((size_t)w * h);
    for (int y = 0; y < h; y++) memcpy(&(*out)[(size_t)y * w], (BYTE*)map.pData + (size_t)y * map.RowPitch, (size_t)w * 4);
    context_->Unmap(staging_, 0);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

void BlendOnto(std::vector<uint32_t>& dst, int dstW, int dstH, const std::vector<uint32_t>& src, int srcW, int srcH, int x, int y) {
    for (int row = std::max(0, -y); row < srcH && y + row < dstH; row++)
        for (int col = std::max(0, -x); col < srcW && x + col < dstW; col++) {
            uint32_t s = src[(size_t)row * srcW + col], &d = dst[(size_t)(y + row) * dstW + x + col];
            uint32_t a = s >> 24;
            if (!a) continue;
            auto ch = [&](int shift) {
                uint32_t v = ((s >> shift) & 255) + ((d >> shift) & 255) * (255 - a) / 255;
                return std::min<uint32_t>(v, 255) << shift;
            };
            d = 0xFF000000 | ch(16) | ch(8) | ch(0);
        }
}

bool SavePng(const std::wstring& path, const std::vector<uint32_t>& pixels, int width, int height) {
    IWICImagingFactory* f = Wic();
    if (!f) return false;
    IWICStream* stream = nullptr;
    IWICBitmapEncoder* encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppPBGRA;
    bool ok = SUCCEEDED(f->CreateStream(&stream)) && SUCCEEDED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE)) &&
              SUCCEEDED(f->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder)) &&
              SUCCEEDED(encoder->Initialize(stream, WICBitmapEncoderNoCache)) && SUCCEEDED(encoder->CreateNewFrame(&frame, nullptr)) &&
              SUCCEEDED(frame->Initialize(nullptr)) && SUCCEEDED(frame->SetSize(width, height)) &&
              SUCCEEDED(frame->SetPixelFormat(&format)) &&
              SUCCEEDED(frame->WritePixels(height, width * 4, (UINT)(pixels.size() * 4), (BYTE*)pixels.data())) &&
              SUCCEEDED(frame->Commit()) && SUCCEEDED(encoder->Commit());
    if (frame) frame->Release();
    if (encoder) encoder->Release();
    if (stream) stream->Release();
    f->Release();
    return ok;
}

}  // namespace td
