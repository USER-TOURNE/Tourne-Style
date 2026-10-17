// Captures a rectangle of the desktop the way the display shows it (DXGI desktop duplication), unlike BitBlt from the
// screen DC, which draws blur-behind (acrylic) bars in the wrong colours. Usage: dxgicap out.bmp x y w h [tries]
#define COBJMACROS
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

template <class T> static void Release(T*& p) { if (p) { p->Release(); p = nullptr; } }

int wmain(int argc, wchar_t** argv) {
    if (argc < 6) { fwprintf(stderr, L"usage: dxgicap out.bmp x y w h\n"); return 2; }
    SetProcessDPIAware();
    int rx = _wtoi(argv[2]), ry = _wtoi(argv[3]), rw = _wtoi(argv[4]), rh = _wtoi(argv[5]);
    int tries = argc > 6 ? _wtoi(argv[6]) : 12;

    ID3D11Device* dev = nullptr; ID3D11DeviceContext* ctx = nullptr;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &dev, nullptr, &ctx))) {
        fwprintf(stderr, L"no D3D11 device\n"); return 3;
    }
    IDXGIDevice* dxgiDev = nullptr; dev->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDev);
    IDXGIAdapter* adapter = nullptr; dxgiDev->GetAdapter(&adapter);
    IDXGIOutput* output = nullptr;
    DXGI_OUTPUT_DESC desc{};
    for (UINT i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++) {
        output->GetDesc(&desc);
        RECT d = desc.DesktopCoordinates;
        if (rx >= d.left && rx < d.right && ry >= d.top && ry < d.bottom) break;
        Release(output);
    }
    if (!output) {
        // Other adapters (a laptop's second GPU): walk them all.
        IDXGIFactory1* factory = nullptr;
        CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
        IDXGIAdapter* a = nullptr;
        for (UINT ai = 0; factory && !output && factory->EnumAdapters(ai, &a) != DXGI_ERROR_NOT_FOUND; ai++) {
            for (UINT i = 0; a->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++) {
                output->GetDesc(&desc);
                RECT d = desc.DesktopCoordinates;
                if (rx >= d.left && rx < d.right && ry >= d.top && ry < d.bottom) break;
                Release(output);
            }
            if (output) { Release(adapter); adapter = a; break; }
            Release(a);
        }
    }
    if (!output) { fwprintf(stderr, L"no output at %d,%d\n", rx, ry); return 4; }
    // The duplication must be made on the device of the adapter that owns the output.
    IDXGIAdapter* owner = nullptr; output->GetParent(__uuidof(IDXGIAdapter), (void**)&owner);
    if (owner != adapter) {
        Release(ctx); Release(dev);
        if (FAILED(D3D11CreateDevice(owner, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &dev, nullptr, &ctx))) {
            fwprintf(stderr, L"no device for the output's adapter\n"); return 3;
        }
    }
    IDXGIOutput1* output1 = nullptr;
    output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    IDXGIOutputDuplication* dup = nullptr;
    if (!output1 || FAILED(output1->DuplicateOutput(dev, &dup))) { fwprintf(stderr, L"DuplicateOutput failed\n"); return 5; }

    ID3D11Texture2D* shot = nullptr;
    for (int i = 0; i < tries && !shot; i++) {
        DXGI_OUTDUPL_FRAME_INFO info{};
        IDXGIResource* res = nullptr;
        HRESULT hr = dup->AcquireNextFrame(400, &info, &res);
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) continue;
        if (FAILED(hr)) break;
        // The first frame is the whole desktop; later ones may only carry changes.
        ID3D11Texture2D* tex = nullptr;
        res->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex);
        D3D11_TEXTURE2D_DESC td{};
        tex->GetDesc(&td);
        td.Usage = D3D11_USAGE_STAGING; td.BindFlags = 0; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ; td.MiscFlags = 0;
        dev->CreateTexture2D(&td, nullptr, &shot);
        ctx->CopyResource(shot, tex);
        Release(tex); Release(res);
        dup->ReleaseFrame();
    }
    if (!shot) { fwprintf(stderr, L"no frame\n"); return 6; }

    D3D11_MAPPED_SUBRESOURCE map{};
    if (FAILED(ctx->Map(shot, 0, D3D11_MAP_READ, 0, &map))) { fwprintf(stderr, L"map failed\n"); return 7; }
    RECT d = desc.DesktopCoordinates;
    int ox = rx - d.left, oy = ry - d.top;
    std::vector<unsigned char> out((size_t)rw * rh * 3);
    for (int y = 0; y < rh; y++) {
        const unsigned char* src = (const unsigned char*)map.pData + (size_t)(oy + y) * map.RowPitch + (size_t)ox * 4;
        unsigned char* dst = out.data() + (size_t)(rh - 1 - y) * rw * 3;  // bottom-up
        for (int x = 0; x < rw; x++) { dst[x * 3] = src[x * 4]; dst[x * 3 + 1] = src[x * 4 + 1]; dst[x * 3 + 2] = src[x * 4 + 2]; }
    }
    ctx->Unmap(shot, 0);
    int stride = rw * 3;
    if (stride % 4) { // rows are padded to 4 bytes
        int padded = (stride + 3) & ~3;
        std::vector<unsigned char> p((size_t)padded * rh, 0);
        for (int y = 0; y < rh; y++) memcpy(&p[(size_t)y * padded], &out[(size_t)y * stride], stride);
        out.swap(p); stride = padded;
    }
    FILE* f = _wfopen(argv[1], L"wb");
    if (!f) return 8;
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(54 + out.size()), 0, 0, 54};
    BITMAPINFOHEADER bi{sizeof(bi), rw, rh, 1, 24, BI_RGB, 0, 0, 0, 0, 0};
    fwrite(&fh, sizeof(fh), 1, f); fwrite(&bi, sizeof(bi), 1, f); fwrite(out.data(), out.size(), 1, f);
    fclose(f);
    return 0;
}
