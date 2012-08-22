// Copied and modified from DirectWrite sample 'GdiInterop' form the Microsoft SDK

#ifndef H_GDI_TEXT_RENDERER
#define H_GDI_TEXT_RENDERER

#include <dwrite.h>
#include <D2D1.h>
#include <intsafe.h>
#include <comdef.h>

class GdiTextRenderer : public IDWriteTextRenderer
{
public:
    GdiTextRenderer( IDWriteBitmapRenderTarget* bitmapRenderTarget,
                     IDWriteRenderingParams* renderingParams );
    ~GdiTextRenderer();

    IFACEMETHOD(IsPixelSnappingDisabled)(
        __maybenull void* clientDrawingContext,
        __out BOOL* isDisabled
    );

    IFACEMETHOD(GetCurrentTransform)(
        __maybenull void* clientDrawingContext,
        __out DWRITE_MATRIX* transform
    );

    IFACEMETHOD(GetPixelsPerDip)(
        __maybenull void* clientDrawingContext,
        __out FLOAT* pixelsPerDip
    );

    IFACEMETHOD(DrawGlyphRun)(
        __maybenull void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        __in DWRITE_GLYPH_RUN const* glyphRun,
        __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
    );

    IFACEMETHOD(DrawUnderline)(
        __maybenull void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        __in DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
    );

    IFACEMETHOD(DrawStrikethrough)(
        __maybenull void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        __in DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
    );

    IFACEMETHOD(DrawInlineObject)(
        __maybenull void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
    );

public:
    IFACEMETHOD_(unsigned long, AddRef) ();
    IFACEMETHOD_(unsigned long, Release) ();
    IFACEMETHOD(QueryInterface)(
        IID const& riid,
        void** ppvObject
    );
    
    void SetTextColor( int r, int g, int b ) { _textColor = RGB(r, g, b); }
    COLORREF GetTextColor() const { return _textColor; }

private:
    unsigned long _refCount;
    COLORREF _textColor;
    IDWriteBitmapRenderTarget* _renderTarget;
    IDWriteRenderingParams* _renderingParams;
};

#endif
