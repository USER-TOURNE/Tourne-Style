// Stub: mingw ships d2d1_1.h only. Enough of 1.2 / 1.3 to syntax-check.
#pragma once
#include <d2d1_1.h>

typedef enum D2D1_SPRITE_OPTIONS {
    D2D1_SPRITE_OPTIONS_NONE = 0,
    D2D1_SPRITE_OPTIONS_CLAMP_TO_SOURCE_RECTANGLE = 1,
} D2D1_SPRITE_OPTIONS;

interface ID2D1SpriteBatch : public ID2D1Resource {
    STDMETHOD(AddSprites)(UINT32 spriteCount, const D2D1_RECT_F* destinationRectangles,
                          const D2D1_RECT_U* sourceRectangles, const D2D1_COLOR_F* colors,
                          const D2D1_MATRIX_3X2_F* transforms, UINT32 destinationRectanglesStride,
                          UINT32 sourceRectanglesStride, UINT32 colorsStride,
                          UINT32 transformsStride) PURE;
    STDMETHOD_(void, Clear)() PURE;
    STDMETHOD_(UINT32, GetSpriteCount)() PURE;
};

interface ID2D1DeviceContext3 : public ID2D1DeviceContext {
    STDMETHOD(CreateSpriteBatch)(ID2D1SpriteBatch** spriteBatch) PURE;
    STDMETHOD_(void, DrawSpriteBatch)(ID2D1SpriteBatch* spriteBatch, UINT32 startIndex,
                                      UINT32 spriteCount, ID2D1Bitmap* bitmap,
                                      D2D1_BITMAP_INTERPOLATION_MODE interpolationMode,
                                      D2D1_SPRITE_OPTIONS spriteOptions) PURE;
};

extern "C" HRESULT WINAPI SetThreadDescription(HANDLE hThread, PCWSTR lpThreadDescription);
