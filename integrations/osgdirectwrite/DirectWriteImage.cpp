#include "GdiTextRenderer.h"
#include "DirectWriteImage.h"

DirectWriteSingleton* DirectWriteSingleton::instance()
{
    osg::ref_ptr<DirectWriteSingleton> s_instance = new DirectWriteSingleton;
    return s_instance.get();
}

DirectWriteSingleton::DirectWriteSingleton()
:   factory(NULL), gdiInterop(NULL)
{
    HRESULT hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&factory)
    );
    if ( SUCCEEDED(hr) )
        hr = factory->GetGdiInterop( &gdiInterop );
    if ( FAILED(hr) )
        OSG_WARN << "Can't initialize global DirectWrite variables." << std::endl;
}

DirectWriteSingleton::~DirectWriteSingleton()
{
    if ( gdiInterop ) gdiInterop->Release();
    if ( factory ) factory->Release();
}

DirectWriteImage::DirectWriteImage()
:   _textRenderer(NULL), _textFormat(NULL), _textLayout(NULL),
    _bitmapRenderTarget(NULL), _renderingParams(NULL), _dirty(true)
{
}

DirectWriteImage::DirectWriteImage( const DirectWriteImage& copy, const osg::CopyOp& op )
:   osg::Image(copy, op), _textRenderer(copy._textRenderer), _textFormat(copy._textFormat),
    _textLayout(copy._textLayout), _bitmapRenderTarget(copy._bitmapRenderTarget),
    _renderingParams(copy._renderingParams), _dirty(copy._dirty)
{
}

DirectWriteImage::~DirectWriteImage()
{
    if ( _bitmapRenderTarget ) _bitmapRenderTarget->Release();
    if ( _renderingParams ) _renderingParams->Release();
    if ( _textRenderer ) _textRenderer->Release();
    if ( _textFormat ) _textFormat->Release();
    if ( _textLayout ) _textLayout->Release();
}

bool DirectWriteImage::initialize( const std::wstring& text, const std::wstring& fontFamily, const std::wstring& fontLocale,
                                   float fontSize, int w, int h )
{
    HRESULT hr = DirectWriteSingleton::instance()->factory->CreateTextFormat(
        fontFamily.c_str(),
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        fontLocale.c_str(),
        &_textFormat );
    if ( FAILED(hr) )
    {
        OSG_WARN << "Can't initialize the text format." << std::endl;
        return false;
    }
    
    _content = text;
    hr = DirectWriteSingleton::instance()->factory->CreateTextLayout(
        text.c_str(), text.length(),
        _textFormat,
        (float)w, (float)h,
        &_textLayout );
    if ( FAILED(hr) )
    {
        OSG_WARN << "Can't initialize the text layer." << std::endl;
        return false;
    }
    
    hr = DirectWriteSingleton::instance()->gdiInterop->CreateBitmapRenderTarget(
        NULL, (float)w, (float)h, &_bitmapRenderTarget );
    if ( FAILED(hr) )
    {
        OSG_WARN << "Can't initialize the text renderer." << std::endl;
        return false;
    }
    
    hr = DirectWriteSingleton::instance()->factory->CreateRenderingParams( &_renderingParams );
    if ( FAILED(hr) )
    {
        OSG_WARN << "Can't initialize rendering parameters." << std::endl;
        return false;
    }
    
    _textRenderer = new GdiTextRenderer( _bitmapRenderTarget, _renderingParams );
    _dirty = true;
    return true;
}

void DirectWriteImage::resizeRenderTarget( int w, int h )
{
    _dirty = true;
    if ( _bitmapRenderTarget )
        _bitmapRenderTarget->Resize( w, h );
}

void DirectWriteImage::setContent( const std::wstring& text )
{
    IDWriteTextLayout* newTextLayout = createNewTextLayout( text );
    if ( newTextLayout )
    {
        if ( _textLayout )
        {
            copyLayoutProperties( _textLayout, 0, newTextLayout, 0, text.length() );
            _textLayout->Release();
        }
        _textLayout = newTextLayout;
    }
}

void DirectWriteImage::insertText( int pos, const std::wstring& text )
{
    int appendingIndex = -1;
    std::wstring completedText = _content;
    if ( pos>=0 && pos<(int)_content.size() )
        completedText.insert( pos, text );
    else
    {
        completedText.append( text );
        appendingIndex = _content.size();
    }
    
    IDWriteTextLayout* newTextLayout = createNewTextLayout( completedText );
    if ( newTextLayout )
    {
        if ( _textLayout )
        {
            if ( appendingIndex<0 )
                copyLayoutProperties( _textLayout, pos, newTextLayout, pos, text.length() );
            else
                copyLayoutProperties( _textLayout, appendingIndex-1, newTextLayout, appendingIndex, text.length() );
            _textLayout->Release();
        }
        _textLayout = newTextLayout;
    }
}

void DirectWriteImage::replaceText( int pos, int length, const std::wstring& text )
{
    std::wstring completedText = _content;
    if ( pos<0 || pos>(int)_content.size() || length<=0 )
        return;
    
    if ( pos+length<(int)_content.size() ) completedText.erase( pos, length );
    else completedText.erase( pos, std::string::npos );
    if ( !text.empty() )
    {
        if ( pos<(int)completedText.size() ) completedText.insert( pos, text );
        else completedText.append( text );
    }
    
    IDWriteTextLayout* newTextLayout = createNewTextLayout( completedText );
    if ( newTextLayout )
    {
        if ( _textLayout )
        {
            copyLayoutProperties( _textLayout, pos, newTextLayout, pos, text.length() );
            _textLayout->Release();
        }
        _textLayout = newTextLayout;
    }
}

void DirectWriteImage::setTextColor( int r, int g, int b )
{
    GdiTextRenderer* renderer = dynamic_cast<GdiTextRenderer*>(_textRenderer);
    if ( renderer ) renderer->SetTextColor( r, g, b );
    _dirty = true;
}

void DirectWriteImage::getTextColor( int& r, int& g, int& b ) const
{
    GdiTextRenderer* renderer = dynamic_cast<GdiTextRenderer*>(_textRenderer);
    if ( renderer )
    {
        COLORREF color = renderer->GetTextColor();
        r = GetRValue( color );
        g = GetGValue( color );
        b = GetBValue( color );
    }
}

void DirectWriteImage::update( osg::NodeVisitor* nv )
{
    if ( !_dirty ) return;
    _dirty = false;
    
    HDC hdc = NULL;
    if ( _bitmapRenderTarget )
    {
        hdc = _bitmapRenderTarget->GetMemoryDC();
        SetBoundsRect( hdc, NULL, DCB_ENABLE|DCB_RESET );
        if ( _textLayout && _textRenderer )
            _textLayout->Draw( NULL, _textRenderer, _textOrigin.x(), _textOrigin.y() );
    }
    
    if ( hdc!=NULL )
    {
        HBITMAP hbm = (HBITMAP)GetCurrentObject( hdc, OBJ_BITMAP );
        if ( hbm==NULL )
        {
            OSG_WARN << "Can't get bitmap from text device context." << std::endl;
            return;
        }
        
        BITMAP bitmap;
        GetObject( hbm, sizeof(BITMAP), &bitmap );
        switch ( bitmap.bmBitsPixel )
        {
        case 24:
            setImage( bitmap.bmWidth, bitmap.bmHeight, 1, 3, GL_BGR, GL_UNSIGNED_BYTE,
                      reinterpret_cast<unsigned char*>(bitmap.bmBits), osg::Image::NO_DELETE, 4 );
            break;
        case 32:
            setImage( bitmap.bmWidth, bitmap.bmHeight, 1, 4, GL_BGRA, GL_UNSIGNED_BYTE,
                      reinterpret_cast<unsigned char*>(bitmap.bmBits), osg::Image::NO_DELETE, 4 );
            break;
        default: break;
        }
    }
}

IDWriteTextLayout* DirectWriteImage::createNewTextLayout( const std::wstring& text )
{
    _dirty = true;
    _content = text;
    if ( !_textFormat ) return NULL;
    
    float w = 800.0f, h = 600.0f;
    if ( _textLayout )
    {
        w = _textLayout->GetMaxWidth();
        h = _textLayout->GetMaxHeight();
    }
    
    IDWriteTextLayout* newTextLayout = NULL;
    HRESULT hr = DirectWriteSingleton::instance()->factory->CreateTextLayout(
        text.c_str(), text.length(),
        _textFormat, w, h,
        &newTextLayout );
    if ( SUCCEEDED(hr) ) return newTextLayout;
    else OSG_WARN << "Can't recreate the text layer." << std::endl;
    return NULL;
}

void DirectWriteImage::copyLayoutProperties(
    IDWriteTextLayout* oldLayout, UINT32 oldPos, IDWriteTextLayout* newLayout, UINT32 newPos, UINT32 newLength )
{
    DWRITE_TEXT_RANGE range = {newPos, newLength};
    if ( !oldLayout || !newLayout || !newLength )
        return;
    
    IDWriteFontCollection* fontCollection = NULL;
    oldLayout->GetFontCollection( oldPos, &fontCollection );
    newLayout->SetFontCollection( fontCollection, range );
    if ( fontCollection ) fontCollection->Release();
    
    wchar_t fontFamilyName[100]; fontFamilyName[0] = '\0';
    oldLayout->GetFontFamilyName( oldPos, &fontFamilyName[0], ARRAYSIZE(fontFamilyName) );
    newLayout->SetFontFamilyName( fontFamilyName, range );
    
    wchar_t localeName[LOCALE_NAME_MAX_LENGTH]; localeName[0] = '\0';
    oldLayout->GetLocaleName( oldPos, &localeName[0], ARRAYSIZE(localeName) );
    newLayout->SetLocaleName( localeName, range );
    
    DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
    DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
    oldLayout->GetFontWeight( oldPos, &weight );
    oldLayout->GetFontStyle( oldPos, &style );
    oldLayout->GetFontStretch( oldPos, &stretch );
    newLayout->SetFontWeight( weight, range );
    newLayout->SetFontStyle( style, range );
    newLayout->SetFontStretch( stretch, range );
    
    FLOAT fontSize = 12.0f;
    oldLayout->GetFontSize( oldPos, &fontSize );
    newLayout->SetFontSize( fontSize, range );
    
    BOOL value = FALSE;
    oldLayout->GetUnderline( oldPos, &value );
    newLayout->SetUnderline( value, range );
    oldLayout->GetStrikethrough( oldPos, &value );
    newLayout->SetStrikethrough( value, range );
    
    IUnknown* drawingEffect = NULL;
    oldLayout->GetDrawingEffect( oldPos, &drawingEffect );
    newLayout->SetDrawingEffect( drawingEffect, range );
    if ( drawingEffect ) drawingEffect->Release();
    
    IDWriteInlineObject* inlineObject = NULL;
    oldLayout->GetInlineObject( oldPos, &inlineObject );
    newLayout->SetInlineObject( inlineObject, range );
    if ( inlineObject ) inlineObject->Release();
    
    IDWriteTypography* typography = NULL;
    oldLayout->GetTypography( oldPos, &typography );
    newLayout->SetTypography( typography, range );
    if ( typography ) typography->Release();
}
