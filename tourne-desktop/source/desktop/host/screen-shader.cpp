#include "screen-shader.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dcomp.h>
#include <dxgi1_2.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "screen-effects.h"
#include "td.h"

namespace td {

namespace {

void Log(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    LogV(L"shader", format, args);
    va_end(args);
}

const char kScreenShader[] = R"(
cbuffer Screen : register(b0) {
    float2 size;      // the screen as the desktop has it, in pixels
    float2 srcSize;   // the picture Desktop Duplication gives (the panel's own way round)
    float rot;        // DXGI_MODE_ROTATION: 1 none, 2 a quarter, 3 a half, 4 three quarters
    float kind;
    float amt;
    float time;
    float3 tint;
    float s;          // the screen's scaling
};
Texture2D src : register(t0);
SamplerState smp : register(s0);

float2 SrcUv(float2 px) {
    px = clamp(px, 0.5, size - 0.5);
    float2 q = px;
    if (rot == 2) q = float2(px.y, size.x - px.x);
    else if (rot == 3) q = size - px;
    else if (rot == 4) q = float2(size.y - px.y, px.x);
    return q / srcSize;
}
float3 EffTake(float2 px) { return src.SampleLevel(smp, SrcUv(px), 0).rgb; }
float3 EffBloom(float2 px) {
    float3 sum = 0;
    [unroll] for (int i = 0; i < 8; i++) {
        float a = i * 0.785398;
        sum += EffTake(px + float2(cos(a), sin(a)) * 12 * s);
    }
    return sum / 8;
}
)";

const char kScreenMain[] = R"(
float4 VS(uint id : SV_VertexID) : SV_Position {
    float2 uv = float2((id << 1) & 2, id & 2);
    return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}
float4 PS(float4 pos : SV_Position) : SV_Target {
    int k = (int)kind;
    float2 p = pos.xy;
    float2 q = k > 0 ? EffectWarp(p, k, amt, time, s) : p;
    float3 col = EffTake(q);
    if (k > 0) col = EffectColour(col, q, size * 0.5, size * 0.5, k, amt, time, s, tint);
    return float4(saturate(col), 1);
}
)";

struct Constants {
    float size[2], srcSize[2];
    float rot, kind, amt, time;
    float tint[3], s;
};

template <class T> void Release(T*& p) {
    if (p) p->Release();
    p = nullptr;
}

using D3DCompileFn = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT,
                                      ID3DBlob**, ID3DBlob**);

ID3DBlob* Compile(const char* entry, const char* target) {
    static D3DCompileFn compile = [] {
        HMODULE m = LoadLibraryExW(L"d3dcompiler_47.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        return m ? (D3DCompileFn)(void*)GetProcAddress(m, "D3DCompile") : nullptr;
    }();
    if (!compile) return nullptr;
    std::string src = std::string(kScreenShader) + kEffectsHlsl + kScreenMain;
    ID3DBlob *code = nullptr, *errors = nullptr;
    HRESULT hr = compile(src.data(), src.size(), "screen", nullptr, nullptr, entry, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &errors);
    if (errors) {
        if (FAILED(hr)) Log(L"the effect: %hs", (const char*)errors->GetBufferPointer());
        errors->Release();
    }
    return SUCCEEDED(hr) ? code : nullptr;
}

struct Config {
    int effect = 0;
    float amount = 0.6f;
    bool allScreens = true;
    bool pauseFullscreen = true;
    uint32_t tint = 0x64AA89;
};

// One screen: its duplication, the effect, and where it's drawn.
struct Pipeline {
    HMONITOR monitor = nullptr;
    RECT rect{};
    float scale = 1;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    IDXGIOutput1* output = nullptr;
    IDXGIOutputDuplication* dup = nullptr;
    DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;
    ID3D11Texture2D* frame = nullptr;  // the last picture, kept for effects that move between pictures
    ID3D11ShaderResourceView* frameView = nullptr;
    UINT frameW = 0, frameH = 0;
    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11Buffer* constants = nullptr;
    ID3D11SamplerState* sampler = nullptr;

    ~Pipeline() {
        Release(sampler), Release(constants), Release(ps), Release(vs), Release(frameView), Release(frame);
        Release(dup), Release(output), Release(context), Release(device);
    }

    bool Start(HMONITOR m) {
        monitor = m;
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(m, &mi);
        rect = mi.rcMonitor;
        UINT dpiX = 96, dpiY = 96;
        using GetDpi = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
        if (auto f = (GetDpi)(void*)GetProcAddress(LoadLibraryExW(L"shcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32), "GetDpiForMonitor"))
            f(m, 0, &dpiX, &dpiY);
        scale = dpiX / 96.f;
        IDXGIFactory1* factory = nullptr;
        if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory))) return false;
        IDXGIAdapter1* adapter = nullptr;
        for (UINT a = 0; !output && factory->EnumAdapters1(a, &adapter) != DXGI_ERROR_NOT_FOUND; a++) {
            IDXGIOutput* o = nullptr;
            for (UINT i = 0; adapter->EnumOutputs(i, &o) != DXGI_ERROR_NOT_FOUND; i++) {
                DXGI_OUTPUT_DESC d;
                if (SUCCEEDED(o->GetDesc(&d)) && d.Monitor == m) {
                    o->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output);
                    rotation = d.Rotation;
                }
                o->Release();
                if (output) break;
            }
            if (output) {
                D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
                D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, ARRAYSIZE(levels),
                                  D3D11_SDK_VERSION, &device, nullptr, &context);
            }
            adapter->Release();
        }
        factory->Release();
        if (!output || !device) return Log(L"no graphics output for a screen"), false;
        bool level11 = device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0;
        ID3DBlob* v = Compile("VS", level11 ? "vs_5_0" : "vs_4_0");
        ID3DBlob* p = Compile("PS", level11 ? "ps_5_0" : "ps_4_0");
        bool ok = v && p && SUCCEEDED(device->CreateVertexShader(v->GetBufferPointer(), v->GetBufferSize(), nullptr, &vs)) &&
                  SUCCEEDED(device->CreatePixelShader(p->GetBufferPointer(), p->GetBufferSize(), nullptr, &ps));
        if (v) v->Release();
        if (p) p->Release();
        if (!ok) return Log(L"the effect didn't compile"), false;
        D3D11_BUFFER_DESC cb{};
        cb.ByteWidth = (sizeof(Constants) + 15) & ~15u;
        cb.Usage = D3D11_USAGE_DEFAULT;
        cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device->CreateBuffer(&cb, nullptr, &constants)) || FAILED(device->CreateSamplerState(&sd, &sampler))) return false;
        return Duplicate();
    }

    bool Duplicate() {
        Release(dup);
        HRESULT hr = output->DuplicateOutput(device, &dup);
        if (FAILED(hr)) Log(L"Desktop Duplication refused (%08lx)", hr);
        return SUCCEEDED(hr);
    }

    // A new picture, if the screen changed within the wait. -1: lost (duplicate again), 0: none, 1: new.
    int Grab(UINT waitMs) {
        if (!dup && !Duplicate()) return -1;
        DXGI_OUTDUPL_FRAME_INFO info{};
        IDXGIResource* res = nullptr;
        HRESULT hr = dup->AcquireNextFrame(waitMs, &info, &res);
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) return 0;
        if (FAILED(hr)) {
            Release(dup);
            return -1;
        }
        int got = 0;
        ID3D11Texture2D* tex = nullptr;
        if (info.LastPresentTime.QuadPart && SUCCEEDED(res->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex))) {
            D3D11_TEXTURE2D_DESC d;
            tex->GetDesc(&d);
            if (!frame || d.Width != frameW || d.Height != frameH) {
                Release(frameView), Release(frame);
                D3D11_TEXTURE2D_DESC own = d;
                own.Usage = D3D11_USAGE_DEFAULT;
                own.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                own.CPUAccessFlags = 0;
                own.MiscFlags = 0;
                own.MipLevels = 1;
                own.ArraySize = 1;
                if (SUCCEEDED(device->CreateTexture2D(&own, nullptr, &frame))) device->CreateShaderResourceView(frame, nullptr, &frameView);
                frameW = d.Width, frameH = d.Height;
            }
            if (frame) context->CopyResource(frame, tex), got = 1;
            tex->Release();
        }
        res->Release();
        dup->ReleaseFrame();
        return got;
    }

    // The effect over the last picture, into target (the screen's size).
    void Render(ID3D11RenderTargetView* target, Config const& cfg, float seconds) {
        if (!frameView) return;
        int w = rect.right - rect.left, h = rect.bottom - rect.top;
        Constants c{};
        c.size[0] = (float)w, c.size[1] = (float)h;
        c.srcSize[0] = (float)frameW, c.srcSize[1] = (float)frameH;
        c.rot = (float)rotation;
        c.kind = (float)cfg.effect, c.amt = cfg.amount, c.time = seconds;
        c.tint[0] = ((cfg.tint >> 16) & 255) / 255.f, c.tint[1] = ((cfg.tint >> 8) & 255) / 255.f, c.tint[2] = (cfg.tint & 255) / 255.f;
        c.s = scale;
        context->UpdateSubresource(constants, 0, nullptr, &c, 0, 0);
        D3D11_VIEWPORT vp{0, 0, (float)w, (float)h, 0, 1};
        context->OMSetRenderTargets(1, &target, nullptr);
        context->RSSetViewports(1, &vp);
        context->IASetInputLayout(nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->VSSetShader(vs, nullptr, 0);
        context->PSSetShader(ps, nullptr, 0);
        context->PSSetConstantBuffers(0, 1, &constants);
        context->PSSetShaderResources(0, 1, &frameView);
        context->PSSetSamplers(0, 1, &sampler);
        context->Draw(3, 0);
        ID3D11ShaderResourceView* none = nullptr;
        context->PSSetShaderResources(0, 1, &none);
    }
};

std::atomic<bool> g_stop;
std::vector<std::thread> g_threads;
Config g_cfg;
HWND g_host;

LRESULT CALLBACK OverlayProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    if (msg == WM_MOUSEACTIVATE) return MA_NOACTIVATE;
    return DefWindowProcW(wnd, msg, wParam, lParam);
}

// A full-screen program in front on this screen (not the desktop itself).
bool FullscreenOn(HMONITOR m) {
    HWND fg = GetForegroundWindow();
    if (!fg || MonitorFromWindow(fg, MONITOR_DEFAULTTONULL) != m) return false;
    WCHAR cls[64] = L"";
    GetClassNameW(fg, cls, ARRAYSIZE(cls));
    if (!wcscmp(cls, L"Progman") || !wcscmp(cls, L"WorkerW") || !wcsncmp(cls, L"Tourne", 6)) return false;
    RECT r, both;
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(m, &mi);
    GetWindowRect(fg, &r);
    IntersectRect(&both, &r, &mi.rcMonitor);
    return EqualRect(&both, &mi.rcMonitor) && !(GetWindowLongW(fg, GWL_STYLE) & WS_CAPTION);
}

void RunScreen(HMONITOR m, Config cfg) {
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    auto pipe = std::make_unique<Pipeline>();
    if (!pipe->Start(m)) return;
    RECT r = pipe->rect;
    int w = r.right - r.left, h = r.bottom - r.top;

    WNDCLASSW wc{};
    wc.lpfnWndProc = OverlayProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"TourneScreenShader";
    RegisterClassW(&wc);
    HWND wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED |
                                   WS_EX_NOREDIRECTIONBITMAP,
                               wc.lpszClassName, L"", WS_POPUP, r.left, r.top, w, h, nullptr, nullptr, wc.hInstance, nullptr);
    if (!wnd) return;
    SetLayeredWindowAttributes(wnd, 0, 255, LWA_ALPHA);
    // Left out of every capture, Desktop Duplication's too, so it never reads itself.
    if (!SetWindowDisplayAffinity(wnd, 0x11 /* WDA_EXCLUDEFROMCAPTURE */)) {
        Log(L"Windows won't leave the overlay out of captures (before Windows 10 2004?): not shown");
        DestroyWindow(wnd);
        return;
    }

    IDXGIDevice* dxgiDevice = nullptr;
    IDXGIAdapter* adapter = nullptr;
    IDXGIFactory2* factory = nullptr;
    IDXGISwapChain1* swap = nullptr;
    IDCompositionDevice* dcomp = nullptr;
    IDCompositionTarget* target = nullptr;
    IDCompositionVisual* visual = nullptr;
    pipe->device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (dxgiDevice) dxgiDevice->GetAdapter(&adapter);
    if (adapter) adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);
    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.Width = w, sd.Height = h;
    sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    bool ok = factory && SUCCEEDED(factory->CreateSwapChainForComposition(pipe->device, &sd, nullptr, &swap)) &&
              SUCCEEDED(DCompositionCreateDevice(dxgiDevice, __uuidof(IDCompositionDevice), (void**)&dcomp)) &&
              SUCCEEDED(dcomp->CreateTargetForHwnd(wnd, TRUE, &target)) && SUCCEEDED(dcomp->CreateVisual(&visual)) &&
              SUCCEEDED(visual->SetContent(swap)) && SUCCEEDED(target->SetRoot(visual)) && SUCCEEDED(dcomp->Commit());
    if (!ok) Log(L"couldn't make the overlay's surface");

    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    bool shown = false, paused = false, moves = EffectMoves(cfg.effect);
    ULONGLONG nextCheck = 0;
    int frames = 0;
    ULONGLONG since = GetTickCount64();
    while (ok && !g_stop) {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) DispatchMessageW(&msg);
        if (GetTickCount64() >= nextCheck) {
            nextCheck = GetTickCount64() + 500;
            bool want = cfg.pauseFullscreen && FullscreenOn(m);
            if (want != paused) {
                paused = want;
                ShowWindow(wnd, paused ? SW_HIDE : SW_SHOWNOACTIVATE);
                Log(paused ? L"a full-screen program in front: stepped aside" : L"back");
            }
            if (shown && !paused) SetWindowPos(wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        if (paused) {
            pipe->Grab(100);  // (kept reading, so it's fresh when it comes back)
            continue;
        }
        int got = pipe->Grab(moves ? 8 : 100);
        if (got < 0) {
            Sleep(200);
            continue;
        }
        if (!got && !(moves && pipe->frameView)) continue;
        ID3D11Texture2D* back = nullptr;
        ID3D11RenderTargetView* rtv = nullptr;
        if (SUCCEEDED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back)) &&
            SUCCEEDED(pipe->device->CreateRenderTargetView(back, nullptr, &rtv))) {
            QueryPerformanceCounter(&now);
            pipe->Render(rtv, cfg, (float)((now.QuadPart - start.QuadPart) / (double)freq.QuadPart));
            swap->Present(1, 0);
            frames++;
            if (!shown) {
                ShowWindow(wnd, SW_SHOWNOACTIVATE);
                shown = true;
            }
        }
        Release(rtv);
        Release(back);
        if (GetTickCount64() - since >= 60000) {
            Log(L"screen at %d,%d: %d frames in a minute", (int)r.left, (int)r.top, frames);
            frames = 0, since = GetTickCount64();
        }
    }
    Release(visual), Release(target), Release(dcomp), Release(swap), Release(factory), Release(adapter), Release(dxgiDevice);
    DestroyWindow(wnd);
}

Config ReadConfig() {
    Config c;
    const wchar_t* e = GetString(L"shader", L"screen");
    c.effect = EffectByName(e ? e : L"none");
    FreeString(e);
    c.amount = std::clamp(GetInt(L"shader", L"amount"), 0, 100) / 100.f;
    const wchar_t* which = GetString(L"shader", L"screens");
    c.allScreens = !which || std::wstring(which) != L"main";
    FreeString(which);
    c.pauseFullscreen = GetInt(L"shader", L"pauseFullscreen") != 0;
    uint32_t palette[11];
    if (FlyoutPalette(palette)) c.tint = palette[4];
    return c;
}

void StopThreads() {
    g_stop = true;
    for (auto& t : g_threads)
        if (t.joinable()) t.join();
    g_threads.clear();
    g_stop = false;
}

std::vector<HMONITOR> Screens(bool all) {
    std::vector<HMONITOR> out;
    if (!all) {
        out.push_back(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY));
        return out;
    }
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR m, HDC, LPRECT, LPARAM p) -> BOOL {
        ((std::vector<HMONITOR>*)p)->push_back(m);
        return TRUE;
    }, (LPARAM)&out);
    return out;
}

}  // namespace

void ScreenShaderDefaults(Settings& out) {
    out[L"shader.screen"] = L"none";
    out[L"shader.amount"] = L"60";
    out[L"shader.screens"] = L"all";
    out[L"shader.pauseFullscreen"] = L"1";
}

void ScreenShaderDescribe(SchemaInfo& info) {
    auto add = [&](const std::wstring& key, const wchar_t* name, const wchar_t* description) -> SettingInfo& {
        SettingInfo& si = info[key];
        si.name = name;
        si.description = description;
        if (!si.order) si.order = (int)info.size();
        return si;
    };
    add(L"shader", L"Screen shader",
        L"One of Tourne's effects over your whole screen, as ShaderGlass does: CRT scanlines, VHS, film grain, pixelate, dither, "
        L"halftone and more. Each screen is read with Windows' own Desktop Duplication and drawn again on top, through the effect, "
        L"in a window clicks go through and captures leave out (your recordings and screenshots stay clean). It costs some graphics "
        L"time while on; nothing while off.");
    SettingInfo& e = add(L"shader.screen", L"Effect", L"None turns it off.");
    for (int i = 0; i < kEffectCount; i++) e.options.push_back({kEffectIds[i], kEffectNames[i]});
    SettingInfo& a = add(L"shader.amount", L"Strength", L"0 to 100.");
    a.hasMin = a.hasMax = true, a.min = 0, a.max = 100;
    SettingInfo& s = add(L"shader.screens", L"Screens", L"");
    s.options = {{L"all", L"Every screen"}, {L"main", L"The main screen"}};
    add(L"shader.pauseFullscreen", L"Step aside for full-screen programs", L"Games and videos in full screen show as they are.")
        .isSwitch = true;
}

void ScreenShaderStart(HWND host) {
    g_host = host;
    ScreenShaderApply();
}

void ScreenShaderApply() {
    Config c = ReadConfig();
    bool same = c.effect == g_cfg.effect && c.amount == g_cfg.amount && c.allScreens == g_cfg.allScreens &&
                c.pauseFullscreen == g_cfg.pauseFullscreen && c.tint == g_cfg.tint && (!g_threads.empty() || !c.effect);
    if (same) return;
    StopThreads();
    g_cfg = c;
    if (!c.effect) return;
    Log(L"on: %ls at %d%%", kEffectIds[c.effect], (int)(c.amount * 100));
    for (HMONITOR m : Screens(c.allScreens)) g_threads.emplace_back(RunScreen, m, c);
}

void ScreenShaderStop() {
    StopThreads();
    g_cfg = Config{};
}

int ScreenShaderShot(const std::wstring& effect, int monitorIndex, const std::wstring& path) {
    std::vector<HMONITOR> all = Screens(true);
    std::sort(all.begin(), all.end(), [](HMONITOR a, HMONITOR b) {
        MONITORINFO ma{sizeof(ma)}, mb{sizeof(mb)};
        GetMonitorInfoW(a, &ma), GetMonitorInfoW(b, &mb);
        return ma.rcMonitor.left < mb.rcMonitor.left;
    });
    if (monitorIndex < 1 || monitorIndex > (int)all.size()) return fwprintf(stderr, L"no screen %d\n", monitorIndex), 1;
    Pipeline pipe;
    if (!pipe.Start(all[monitorIndex - 1])) return fwprintf(stderr, L"couldn't start\n"), 1;
    for (int i = 0; i < 20 && pipe.Grab(200) != 1; i++) {}
    if (!pipe.frameView) return fwprintf(stderr, L"no picture\n"), 1;
    int w = pipe.rect.right - pipe.rect.left, h = pipe.rect.bottom - pipe.rect.top;
    D3D11_TEXTURE2D_DESC d{};
    d.Width = w, d.Height = h, d.MipLevels = d.ArraySize = 1, d.Format = DXGI_FORMAT_B8G8R8A8_UNORM, d.SampleDesc.Count = 1;
    d.Usage = D3D11_USAGE_DEFAULT, d.BindFlags = D3D11_BIND_RENDER_TARGET;
    ID3D11Texture2D *out = nullptr, *staging = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    pipe.device->CreateTexture2D(&d, nullptr, &out);
    d.Usage = D3D11_USAGE_STAGING, d.BindFlags = 0, d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    pipe.device->CreateTexture2D(&d, nullptr, &staging);
    if (!out || !staging || FAILED(pipe.device->CreateRenderTargetView(out, nullptr, &rtv))) return 1;
    Config cfg;
    cfg.effect = EffectByName(effect);
    cfg.amount = 0.8f;
    pipe.Render(rtv, cfg, 2.5f);
    pipe.context->CopyResource(staging, out);
    D3D11_MAPPED_SUBRESOURCE map;
    if (FAILED(pipe.context->Map(staging, 0, D3D11_MAP_READ, 0, &map))) return 1;
    BITMAPINFOHEADER bh{sizeof(bh), w, -h, 1, 32, BI_RGB};
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bh) + (size_t)w * h * 4), 0, 0, (DWORD)(sizeof(fh) + sizeof(bh))};
    if (FILE* f = _wfopen(path.c_str(), L"wb")) {
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&bh, sizeof(bh), 1, f);
        for (int y = 0; y < h; y++) fwrite((BYTE*)map.pData + (size_t)y * map.RowPitch, 4, w, f);
        fclose(f);
    }
    pipe.context->Unmap(staging, 0);
    Release(rtv), Release(staging), Release(out);
    wprintf(L"screen %d (%dx%d, rotation %d, picture %ux%u) -> %ls\n", monitorIndex, w, h, (int)pipe.rotation, pipe.frameW, pipe.frameH,
            path.c_str());
    return 0;
}

}  // namespace td
