// Captures a rectangle of a monitor with Windows Graphics Capture: the same path Snipping Tool uses, so the colours are
// the ones the user sees (BitBlt and DXGI duplication show StartAllBack's acrylic bar in the wrong colours).
// Usage: graphicscap out.bmp x y w h    (x, y in virtual-desktop pixels). Start it outside the Claude package.
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <unknwn.h>
#include <inspectable.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <windows.graphics.capture.interop.h>
#include <cstdio>
#include <vector>

extern "C" HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(::IDXGIDevice* dxgi, ::IInspectable** device);

static const GUID kIidTexture2D = {0x6f15aaf2, 0xd208, 0x4e89, {0x9a, 0xb4, 0x48, 0x95, 0x35, 0xd3, 0x4f, 0x9c}};
static const GUID kIidDxgiDevice = {0x54ec77fa, 0x1377, 0x44e6, {0x8c, 0x32, 0x88, 0xfd, 0x5f, 0x44, 0xc8, 0x4c}};
static const GUID kIidDxgiInterfaceAccess = {0xA9B3D012, 0x3DF2, 0x4EE3, {0xB8, 0xD1, 0x86, 0x95, 0xF4, 0x57, 0xD3, 0xC1}};
struct IDirect3DDxgiInterfaceAccess : ::IUnknown {
    virtual HRESULT __stdcall GetInterface(REFIID iid, void** object) = 0;
};

using namespace winrt;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

int wmain(int argc, wchar_t** argv) {
    if (argc < 6) { fwprintf(stderr, L"usage: graphicscap out.bmp x y w h\n"); return 2; }
    SetProcessDPIAware();
    int rx = _wtoi(argv[2]), ry = _wtoi(argv[3]), rw = _wtoi(argv[4]), rh = _wtoi(argv[5]);
    init_apartment(apartment_type::multi_threaded);

    com_ptr<ID3D11Device> dev;
    com_ptr<ID3D11DeviceContext> ctx;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
                                 D3D11_SDK_VERSION, dev.put(), nullptr, ctx.put()))) {
        fwprintf(stderr, L"no D3D11 device\n"); return 3;
    }
    com_ptr<IDXGIDevice> dxgi;
    dev->QueryInterface(kIidDxgiDevice, dxgi.put_void());
    com_ptr<::IInspectable> inspectable;
    if (FAILED(CreateDirect3D11DeviceFromDXGIDevice(dxgi.get(), inspectable.put()))) { fwprintf(stderr, L"no WinRT device\n"); return 3; }
    auto device = inspectable.as<IDirect3DDevice>();

    POINT pt{rx, ry};
    HMONITOR mon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    auto interop = get_activation_factory<GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
    GraphicsCaptureItem item{nullptr};
    if (FAILED(interop->CreateForMonitor(mon, guid_of<GraphicsCaptureItem>(), put_abi(item)))) { fwprintf(stderr, L"no capture item\n"); return 4; }

    auto pool = Direct3D11CaptureFramePool::CreateFreeThreaded(device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, item.Size());
    auto session = pool.CreateCaptureSession(item);
    try { session.IsCursorCaptureEnabled(false); } catch (...) {}
    try { session.IsBorderRequired(false); } catch (...) {}
    session.StartCapture();

    com_ptr<ID3D11Texture2D> shot;
    for (int i = 0; i < 60 && !shot; i++) {
        Sleep(50);
        if (auto frame = pool.TryGetNextFrame()) {
            com_ptr<IDirect3DDxgiInterfaceAccess> access;
            get_unknown(frame.Surface())->QueryInterface(kIidDxgiInterfaceAccess, access.put_void());
            com_ptr<ID3D11Texture2D> tex;
            access->GetInterface(kIidTexture2D, tex.put_void());
            D3D11_TEXTURE2D_DESC td{};
            tex->GetDesc(&td);
            td.Usage = D3D11_USAGE_STAGING; td.BindFlags = 0; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ; td.MiscFlags = 0;
            dev->CreateTexture2D(&td, nullptr, shot.put());
            ctx->CopyResource(shot.get(), tex.get());
            frame.Close();
        }
    }
    session.Close();
    pool.Close();
    if (!shot) { fwprintf(stderr, L"no frame\n"); return 6; }

    D3D11_MAPPED_SUBRESOURCE map{};
    if (FAILED(ctx->Map(shot.get(), 0, D3D11_MAP_READ, 0, &map))) { fwprintf(stderr, L"map failed\n"); return 7; }
    int ox = rx - mi.rcMonitor.left, oy = ry - mi.rcMonitor.top;
    int stride = (rw * 3 + 3) & ~3;
    std::vector<unsigned char> out((size_t)stride * rh, 0);
    for (int y = 0; y < rh; y++) {
        const unsigned char* src = (const unsigned char*)map.pData + (size_t)(oy + y) * map.RowPitch + (size_t)ox * 4;
        unsigned char* dst = out.data() + (size_t)(rh - 1 - y) * stride;
        for (int x = 0; x < rw; x++) { dst[x * 3] = src[x * 4]; dst[x * 3 + 1] = src[x * 4 + 1]; dst[x * 3 + 2] = src[x * 4 + 2]; }
    }
    ctx->Unmap(shot.get(), 0);
    FILE* f = _wfopen(argv[1], L"wb");
    if (!f) return 8;
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + out.size()), 0, 0, 54};
    BITMAPINFOHEADER bi{sizeof(bi), rw, rh, 1, 24, BI_RGB, 0, 0, 0, 0, 0};
    fwrite(&fh, sizeof(fh), 1, f); fwrite(&bi, sizeof(bi), 1, f); fwrite(out.data(), out.size(), 1, f);
    fclose(f);
    return 0;
}
