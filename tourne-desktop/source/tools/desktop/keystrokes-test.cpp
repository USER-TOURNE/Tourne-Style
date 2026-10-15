// Draws Keystrokes' keycaps (desktop/host/keystrokes.cpp) for made-up keys, each style in turn, into pictures:
// no keys are sent and no hook is set. keystrokes-<style>.bmp in the folder given.
//
// Build (Windhawk's clang):
//   "C:\Program Files\Windhawk\Compiler\bin\clang++.exe" --target=x86_64-w64-mingw32 -std=c++23 -municode -DUNICODE
//     -D_UNICODE -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -O1 -static -Idesktop\host tools\desktop\keystrokes-test.cpp
//     -o tools\desktop\keystrokes-test.exe -ld2d1 -ldwrite -lgdi32 -luser32 -lole32
// Run: keystrokes-test.exe <folder>
#include <windows.h>

#include <cstdio>
#include <map>
#include <string>

#include "config.h"

namespace td {
std::map<std::wstring, std::wstring> g_values;
int GetInt(const wchar_t*, const std::wstring& key) { return _wtoi(g_values[key].c_str()); }
const wchar_t* GetString(const wchar_t*, const std::wstring& key) { return _wcsdup(g_values[key].c_str()); }
void FreeString(const wchar_t* s) { free((void*)s); }
bool FlyoutPalette(uint32_t* p) {
    uint32_t tourne[11] = {0x121C21, 0x1B2A31, 0x1E2F36, 0x4F8F6A, 0x66EA97, 0x15FFA2, 0x64AA89, 0x3A5048, 0xFFB86C, 0xFF6F77, 0x1F3D34};
    for (int i = 0; i < 11; i++) p[i] = tourne[i];
    return true;
}
void LogV(const wchar_t*, const wchar_t*, va_list) {}
void OpenStudio(std::wstring const&, std::wstring const&) {}
}  // namespace td

#include "keystrokes.cpp"

void Save(std::wstring const& path) {
    using namespace td;
    BITMAP bm{};
    GetObjectW(g_bitmap, sizeof(bm), &bm);
    int w = bm.bmWidth, h = bm.bmHeight;
    // On a dark grey, so the keys' own see-through edges show.
    std::vector<uint32_t> px(w * h);
    BITMAPINFO bi{};
    bi.bmiHeader = {sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB};
    GetDIBits(g_memDC, g_bitmap, 0, h, px.data(), &bi, DIB_RGB_COLORS);
    for (auto& p : px) {
        uint32_t a = p >> 24, back = 0x30;
        auto ch = [&](int s) { return ((p >> s) & 0xFF) + back * (255 - a) / 255; };
        p = 0xFF000000 | ch(16) << 16 | ch(8) << 8 | ch(0);
    }
    BITMAPFILEHEADER fh{0x4D42, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader) + w * h * 4), 0, 0, (DWORD)(sizeof(fh) + sizeof(bi.bmiHeader))};
    bi.bmiHeader.biHeight = -h;
    if (FILE* f = _wfopen(path.c_str(), L"wb")) {
        fwrite(&fh, sizeof(fh), 1, f);
        fwrite(&bi.bmiHeader, sizeof(bi.bmiHeader), 1, f);
        fwrite(px.data(), 4, px.size(), f);
        fclose(f);
    }
}

int wmain(int argc, wchar_t** argv) {
    using namespace td;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    Settings defaults;
    KeystrokesDefaults(defaults);
    for (auto const& [k, v] : defaults) g_values[k.substr(11)] = v;  // ("keystrokes." off)
    g_values[L"animation"] = L"none";
    g_values[L"linger"] = L"20000";
    std::wstring dir = argc > 1 ? argv[1] : L".";
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    for (const wchar_t* style : {L"minimal", L"laptop", L"lowprofile", L"pbt"}) {
        g_values[L"style"] = style;
        g_cfg = ReadConfig();
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_d2d);
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&g_dw);
        D2D1_RENDER_TARGET_PROPERTIES p =
            D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        g_d2d->CreateDCRenderTarget(&p, &g_rt);
        g_memDC = CreateCompatibleDC(nullptr);
        WNDCLASSW wc{};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"KsTest";
        RegisterClassW(&wc);
        g_wnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, L"KsTest", L"", WS_POPUP, -4000, -4000, 1, 1, nullptr, nullptr,
                                wc.hInstance, nullptr);
        g_rows.clear();
        g_monitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        // Typed text, a counted combination, a click with Ctrl.
        for (DWORD vk : {'H', 'E', 'L', 'L', 'O'}) Add({KeyLabel(vk, 0, false), false, 1, vk}, 0);
        Add({L"S", false, 1, 'S'}, kCtrl | kShift);
        Add({L"S", false, 1, 'S'}, kCtrl | kShift);
        Add({L"S", false, 1, 'S'}, kCtrl | kShift);
        Add({L"Left click", false, 1, 0x1000}, kCtrl);
        for (auto& r : g_rows) r.pressed = 0;  // (drawn at rest)
        Draw();
        Save(dir + L"\\keystrokes-" + style + L".bmp");
        DestroyWindow(g_wnd);
        Release(g_textFormat);
        Release(g_badgeFormat);
        Release(g_rt);
        Release(g_dw);
        Release(g_d2d);
        if (g_bitmap) SelectObject(g_memDC, g_oldBitmap), DeleteObject(g_bitmap), g_bitmap = nullptr;
        g_bitmapW = g_bitmapH = 0;
        DeleteDC(g_memDC);
        wprintf(L"%ls drawn\n", style);
    }
    return 0;
}
