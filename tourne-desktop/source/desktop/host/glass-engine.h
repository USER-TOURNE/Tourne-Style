// Tourne's glass: a Direct3D 11 pixel shader that draws liquid glass over
// the wallpaper. The glass bends what's behind it at its edges (refraction),
// splits the colours there (fringe), glows at the rim (edge glow, as a
// fresnel term), catches light from above (highlight), blurs, tints, can bulge
// like a lens, and can carry a moving oil film.
//
// What's behind the glass is the wallpaper Windows shows (composed here from
// each screen's picture and how it's placed), not the windows in between:
// Windows doesn't let one program see another's pixels cheaply, so this is
// the honest version, and it costs nothing per frame. A surface is drawn
// again only when it moves, changes, or animates.
//
// The result is a premultiplied BGRA bitmap for a layered window, or for a
// window's own drawing (the Studio).
#pragma once

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

namespace td {

// How the glass looks. Sizes are in pixels at 100% scaling; strengths 0 to 1.
struct GlassLook {
    float blur = 6;          // backdrop blur radius
    float refraction = 0.55f;
    float fringe = 0.3f;     // colours split at the edge
    float edgeGlow = 0.4f;
    float highlight = 0.6f;
    float bevel = 18;        // how far in from the edge it curves
    float lens = 0.2f;       // the middle magnified, like a dome
    uint32_t tint = 0xFFFFFF;
    float tintAmount = 0.08f;
    float saturation = 1.1f; // 1 leaves it, 0 is grey
    float brightness = 0;    // -0.5 to 0.5
    float radius = 16;       // corners
    float shadow = 0.5f;
    float oil = 0;           // the oil film's strength (0: off)
    int effect = 0;          // screen-effects.h: crt, vhs... (0: none)
    float effectAmount = 0.6f;
};

// What is drawn: a shape of glass on the screen, maybe with a window-sized
// hole (a rim round a window), and the part of it the bitmap covers.
// Rectangles are screen pixels.
struct GlassShape {
    RECT shape{};
    RECT hole{};         // empty: none
    float holeRadius = 0;
    RECT view{};         // the bitmap's rectangle
    float scale = 1;     // the screen's scaling (dpi / 96)
    float time = 0;      // seconds, for the oil film
    float glow = 0;      // 0 to 1, a glow that fades (a click, a focus)
};

// A picture of the whole desktop as Windows draws its wallpaper, in screen
// pixels from origin (maybe made smaller: backScale screen pixels to one).
struct Backdrop {
    std::vector<uint32_t> pixels;  // 0xAARRGGBB (BGRA in memory)
    int width = 0, height = 0;
    POINT origin{};
    float scale = 1;  // backdrop pixels per screen pixel
};

// Every screen's wallpaper, placed as Windows places it (fill, fit, stretch,
// tile, centre, span), on its background colour. Needs COM on the thread.
bool ComposeWallpaper(Backdrop* out, const wchar_t* chosen = nullptr);  // chosen: this picture on every screen
// One picture filling width x height (for previews and tests).
bool PictureBackdrop(const std::wstring& path, int width, int height, Backdrop* out);
// A gradient, when there's no picture to be had.
void GradientBackdrop(int width, int height, Backdrop* out);

class GlassEngine {
public:
    GlassEngine() = default;
    ~GlassEngine();
    GlassEngine(const GlassEngine&) = delete;
    GlassEngine& operator=(const GlassEngine&) = delete;

    bool Start();  // the device and shaders; false if Direct3D isn't there
    bool Started() const { return device_ != nullptr; }
    bool SetBackdrop(const Backdrop& backdrop);
    bool HasBackdrop() const { return backdrop_ != nullptr; }
    const Backdrop& CurrentBackdrop() const { return copy_; }
    // The view's pixels, premultiplied BGRA, view width x height.
    bool Render(const GlassLook& look, const GlassShape& shape, std::vector<uint32_t>* out);
    void Stop();

private:
    ::ID3D11Device* device_ = nullptr;
    ::ID3D11DeviceContext* context_ = nullptr;
    ::ID3D11VertexShader* vs_ = nullptr;
    ::ID3D11PixelShader* ps_ = nullptr;
    ::ID3D11Buffer* constants_ = nullptr;
    ::ID3D11SamplerState* sampler_ = nullptr;
    ::ID3D11Texture2D* backdrop_ = nullptr;
    ::ID3D11ShaderResourceView* backdropView_ = nullptr;
    ::ID3D11Texture2D* target_ = nullptr;
    ::ID3D11RenderTargetView* targetView_ = nullptr;
    ::ID3D11Texture2D* staging_ = nullptr;
    int targetW_ = 0, targetH_ = 0;
    Backdrop copy_;  // (its pixels dropped once on the GPU; the rest kept)
    bool Target(int w, int h);
};

// Writes premultiplied BGRA pixels to a PNG (for previews).
bool SavePng(const std::wstring& path, const std::vector<uint32_t>& pixels, int width, int height);

// Over: src (premultiplied) onto dst at x, y.
void BlendOnto(std::vector<uint32_t>& dst, int dstW, int dstH, const std::vector<uint32_t>& src, int srcW, int srcH, int x, int y);

}  // namespace td
