// Copied and modified from DirectWrite sample 'GdiInterop' form the Microsoft SDK

#include "GdiTextRenderer.h"

GdiTextRenderer::GdiTextRenderer( IDWriteBitmapRenderTarget* bitmapRenderTarget,
                                  IDWriteRenderingParams* renderingParams )
:   _refCount(0),
    _textColor(RGB(0, 200, 255)),
    _renderTarget(bitmapRenderTarget),
    _renderingParams(renderingParams)
{
    _renderTarget->AddRef();
    _renderingParams->AddRef();
}

GdiTextRenderer::~GdiTextRenderer()
{
    _renderTarget->Release();
    _renderingParams->Release();
}

STDMETHODIMP GdiTextRenderer::DrawGlyphRun(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    __in DWRITE_GLYPH_RUN const* glyphRun,
    __in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect )
{
    // Pass on the drawing call to the render target to do the real work.
    RECT dirtyRect = {0};
    HRESULT hr = _renderTarget->DrawGlyphRun(
        baselineOriginX, baselineOriginY,
        measuringMode, glyphRun,
        _renderingParams, _textColor,
        &dirtyRect
    );
    return hr;
}

STDMETHODIMP GdiTextRenderer::DrawUnderline(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    __in DWRITE_UNDERLINE const* underline,
    IUnknown* clientDrawingEffect )
{
    // Not implemented
    return E_NOTIMPL;
}

STDMETHODIMP GdiTextRenderer::DrawStrikethrough(
    __maybenull void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    __in DWRITE_STRIKETHROUGH const* strikethrough,
    IUnknown* clientDrawingEffect )
{
    // Not implemented
    return E_NOTIMPL;
}

STDMETHODIMP GdiTextRenderer::DrawInlineObject(
    __maybenull void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect )
{
    // Not implemented
    return E_NOTIMPL;
}

STDMETHODIMP_(unsigned long) GdiTextRenderer::AddRef()
{
    return InterlockedIncrement(&_refCount);
}

STDMETHODIMP_(unsigned long) GdiTextRenderer::Release()
{
    long newCount = InterlockedDecrement(&_refCount);
    if ( newCount==0 )
    {
        delete this;
        return 0;
    }
    return newCount;
}

STDMETHODIMP GdiTextRenderer::IsPixelSnappingDisabled(
    __maybenull void* clientDrawingContext,
    __out BOOL* isDisabled )
{
    *isDisabled = FALSE;
    return S_OK;
}

STDMETHODIMP GdiTextRenderer::GetCurrentTransform(
    __maybenull void* clientDrawingContext,
    __out DWRITE_MATRIX* transform )
{
    _renderTarget->GetCurrentTransform( transform );
    return S_OK;
}

STDMETHODIMP GdiTextRenderer::GetPixelsPerDip(
    __maybenull void* clientDrawingContext,
    __out FLOAT* pixelsPerDip )
{
    *pixelsPerDip = _renderTarget->GetPixelsPerDip();
    return S_OK;
}

STDMETHODIMP GdiTextRenderer::QueryInterface(
    IID const& riid, void** ppvObject )
{
    if ( __uuidof(IDWriteTextRenderer)==riid )
    {
        *ppvObject = this;
    }
    else if ( __uuidof(IDWritePixelSnapping)==riid )
    {
        *ppvObject = this;
    }
    else if ( __uuidof(IUnknown)==riid )
    {
        *ppvObject = this;
    }
    else
    {
        *ppvObject = NULL;
        return E_FAIL;
    }
    return S_OK;
}
