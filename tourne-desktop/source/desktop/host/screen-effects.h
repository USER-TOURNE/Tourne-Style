// Tourne's shader effects, written for Tourne (nothing copied): HLSL put into a
// shader that defines, before them, float3 EffTake(float2 px) (the picture at
// a pixel) and float3 EffBloom(float2 px) (a wide soft copy of it). Pixels are
// screen pixels; s is the screen's scaling, so a scanline is a scanline at any
// size. The glass (glass-engine.cpp) runs them after its own pass.
#pragma once

#include <string>

namespace td {

// The effects by name, in their numbers' order (0 is none).
inline const wchar_t* const kEffectIds[] = {L"none",     L"crt",     L"vhs",      L"grain",  L"chroma",   L"pixelate", L"dither",
                                            L"halftone", L"glitch",  L"vignette", L"duotone", L"bloom",   L"invert",   L"dark"};
inline const wchar_t* const kEffectNames[] = {L"None",     L"CRT",       L"VHS",         L"Film grain", L"Chromatic split",
                                              L"Pixelate", L"Dither",    L"Halftone",    L"Glitch",     L"Vignette",
                                              L"Duotone",  L"Bloom",     L"Invert",      L"Dark (keeps colours)"};
constexpr int kEffectCount = 14;

inline int EffectByName(std::wstring const& name) {
    for (int i = 0; i < kEffectCount; i++)
        if (name == kEffectIds[i]) return i;
    return 0;
}

// The ones that move by themselves, so they're drawn again while they show.
inline bool EffectMoves(int kind) { return kind == 2 || kind == 3 || kind == 8; }

inline const char kEffectsHlsl[] = R"(
float EHash(float2 p) { return frac(sin(dot(p, float2(12.9898, 78.233))) * 43758.5453); }
float ELum(float3 c) { return dot(c, float3(0.2126, 0.7152, 0.0722)); }

// Where to take pixel p from, before any colour work (pixelate, VHS's wobble, glitch's blocks).
float2 EffectWarp(float2 p, int kind, float amt, float t, float s) {
    if (kind == 5) {
        float size = max(2.0, round(2 + amt * 14)) * s;
        return (floor(p / size) + 0.5) * size;
    }
    if (kind == 2) {
        float row = floor(p.y / (2 * s));
        float tear = step(0.985 - amt * 0.05, EHash(float2(floor(t * 8), floor(row / 18))));
        return p + float2(sin(p.y * 0.02 / s + t * 3) * amt * 1.5 * s + tear * (EHash(float2(row, floor(t * 20))) - 0.5) * amt * 24 * s, 0);
    }
    if (kind == 8) {
        float2 block = floor(p / (float2(80, 14) * s));
        if (EHash(block + floor(t * 6)) > 1 - amt * 0.35) p.x += (EHash(block.yx + floor(t * 6)) - 0.5) * 60 * s * amt;
    }
    return p;
}

// The colour after: col is the picture at p; centre and halfSize are the surface's middle and half its size; tint is the
// theme's colour (duotone's light end).
float3 EffectColour(float3 col, float2 p, float2 centre, float2 halfSize, int kind, float amt, float t, float s, float3 tint) {
    float lum = ELum(col);
    if (kind == 1) {  // CRT: scanlines, an aperture-grille mask, darker toward the edges
        float scan = 0.5 + 0.5 * sin(p.y / s * 3.14159);
        col *= lerp(1, 0.55 + 0.45 * scan, amt);
        int m = (int)abs(floor(p.x / s)) % 3;
        float3 mask = m == 0 ? float3(1, 0.72, 0.72) : m == 1 ? float3(0.72, 1, 0.72) : float3(0.72, 0.72, 1);
        col *= lerp(float3(1, 1, 1), mask * 1.22, amt * 0.8);
        float2 rel = (p - centre) / max(halfSize, 1);
        col *= lerp(1, saturate(1 - dot(rel, rel) * 0.22), amt);
    } else if (kind == 2) {  // VHS: noise, colour bleeding sideways, faint lines
        col += (EHash(floor(p / s) + frac(t * 13.7) * 97) - 0.5) * 0.16 * amt;
        col.r = lerp(col.r, EffTake(p + float2(3 * s, 0)).r, amt * 0.8);
        col.b = lerp(col.b, EffTake(p - float2(3 * s, 0)).b, amt * 0.8);
        col *= 1 - amt * 0.1 * step(0.5, frac(p.y / (2 * s)));
    } else if (kind == 3) {  // film grain
        col += (EHash(floor(p / s) + frac(t * 7.3) * 91.7) - 0.5) * 0.24 * amt;
    } else if (kind == 4) {  // chromatic split, wider toward the edges
        float2 o = (p - centre) / max(max(halfSize.x, halfSize.y), 1) * amt * 8 * s;
        col.r = EffTake(p + o).r;
        col.b = EffTake(p - o).b;
    } else if (kind == 6) {  // ordered (Bayer) dither to a few levels
        static const float bayer[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5};
        int2 q = int2(abs(floor(p / max(1, s * (1 + amt * 2))))) & 3;
        float th = (bayer[q.y * 4 + q.x] + 0.5) / 16;
        float levels = lerp(8, 2, amt);
        col = floor(col * levels + th) / levels;
    } else if (kind == 7) {  // halftone dots, bigger where it's lighter
        float cell = (4 + amt * 8) * s;
        float2 c = (floor(p / cell) + 0.5) * cell;
        float3 at = EffTake(c);
        float r = sqrt(ELum(at)) * cell * 0.72;
        float inside = saturate(r - length(p - c) + 0.5);
        col = lerp(col, lerp(float3(0.04, 0.04, 0.05), at * 1.1, inside), amt);
    } else if (kind == 8) {  // glitch: some blocks' colours swapped
        if (EHash(floor(p / (float2(80, 14) * s)) + floor(t * 6) + 3.1) > 1 - amt * 0.08) col = col.gbr;
    } else if (kind == 9) {  // vignette
        float2 rel = (p - centre) / max(halfSize, 1);
        col *= saturate(1 - dot(rel, rel) * amt * 0.6);
    } else if (kind == 10) {  // duotone: dark to the tint
        col = lerp(col, lerp(float3(0.05, 0.05, 0.07), tint, lum), amt);
    } else if (kind == 11) {  // bloom: the bright parts glow
        col += max(EffBloom(p) - 0.55, 0) * amt * 2.2;
    } else if (kind == 12) {  // invert
        col = lerp(col, 1 - col, amt);
    } else if (kind == 13) {  // dark: light and dark swapped, colours kept
        col = lerp(col, saturate(col - lum + (1 - lum)), amt);
    }
    return col;
}
)";

}  // namespace td
