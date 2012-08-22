#ifndef H_DIRECTWRITE_IMAGE
#define H_DIRECTWRITE_IMAGE

#include <windows.h>
#include <intsafe.h>
#include <dwrite.h>
#include <osg/Image>

class DirectWriteSingleton : public osg::Referenced
{
public:
    static DirectWriteSingleton* instance();
    
    IDWriteFactory* factory;
    IDWriteGdiInterop* gdiInterop;
    
protected:
    DirectWriteSingleton();
    virtual ~DirectWriteSingleton();
};

class DirectWriteImage : public osg::Image
{
public:
    DirectWriteImage();
    
    DirectWriteImage( const DirectWriteImage& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY );
    
    META_Object( osg, DirectWriteImage )
    
    bool initialize( const std::wstring& text, const std::wstring& fontFamily, const std::wstring& fontLocale,
                     float fontSize, int w, int h );
    void resizeRenderTarget( int w, int h );
    
    /** Set text content (all content editing functions will recreate the text layout) */
    void setContent( const std::wstring& text );
    
    /** Insert text at specified position (-1 means to append) and inherit last layout */
    void insertText( int pos, const std::wstring& text );
    
    /** Remove a few text at specified position and length */
    void removeText( int pos, int length ) { replaceText(pos, length, L""); }
    
    /** Replace a few text at specified position and length and inherit last layout */
    void replaceText( int pos, int length, const std::wstring& text );
    
    /** Get current content */
    const std::wstring& getContent() const { return _content; }
    
    /** Set text color */
    void setTextColor( int r, int g, int b );
    void getTextColor( int& r, int& g, int& b ) const;
    
    /** Set text origin */
    void setTextOrigin( const osg::Vec2& origin ) { _textOrigin = origin; _dirty = true; }
    const osg::Vec2& getTextOrigin() const { return _textOrigin; }
    
    /** Set alignment option of text relative to layout box's leading and trailing edge */
    void setTextAlignment( DWRITE_TEXT_ALIGNMENT a ) { _textFormat->SetTextAlignment(a); _dirty = true; }
    DWRITE_TEXT_ALIGNMENT getTextAlignment() const { return _textFormat->GetTextAlignment(); }
    
    /** Set alignment option of paragraph relative to layout box's top and bottom edge */
    void setParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT a ) { _textFormat->SetParagraphAlignment(a); _dirty = true; }
    DWRITE_PARAGRAPH_ALIGNMENT getParagraphAlignment() const { return _textFormat->GetParagraphAlignment(); }
    
    /** Set word wrapping option */
    void setWordWrapping( DWRITE_WORD_WRAPPING w ) { _textFormat->SetWordWrapping(w); _dirty = true; }
    DWRITE_WORD_WRAPPING getWordWrapping() const { return _textFormat->GetWordWrapping(); }
    
    /** Set paragraph reading direction */
    void setReadingDirection( DWRITE_READING_DIRECTION d ) { _textFormat->SetReadingDirection(d); _dirty = true; }
    DWRITE_READING_DIRECTION getReadingDirection() const { return _textFormat->GetReadingDirection(); }
    
    /** Set paragraph flow direction */
    void setFlowDirection( DWRITE_FLOW_DIRECTION d ) { _textFormat->SetFlowDirection(d); _dirty = true; }
    DWRITE_FLOW_DIRECTION getFlowDirection() const { return _textFormat->GetFlowDirection(); }
    
    /** Set incremental tab stop position */
    void setIncrementalTabStop( float v ) { _textFormat->SetIncrementalTabStop(v); _dirty = true; }
    float getIncrementalTabStop() const { return _textFormat->GetIncrementalTabStop(); }
    
    /** Set line spacing */
    void setLineSpacing( DWRITE_LINE_SPACING_METHOD m, float lineSpacing, float baseLine )
    { _textFormat->SetLineSpacing(m, lineSpacing, baseLine); _dirty = true; }
    void getLineSpacing( DWRITE_LINE_SPACING_METHOD& m, float& lineSpacing, float& baseLine ) const
    { _textFormat->GetLineSpacing(&m, &lineSpacing, &baseLine); }
    
    /** Set layout maximum width */
    void setMaxWidth( float w ) { _textLayout->SetMaxWidth(w); _dirty = true; }
    float getMaxWidth() const { return _textLayout->GetMaxWidth(); }
    
    /** Set layout maximum height */
    void setMaxHeight( float h ) { _textLayout->SetMaxHeight(h); _dirty = true; }
    float getMaxHeight() const { return _textLayout->GetMaxHeight(); }
    
    /** Set null-terminated font family name */
    void setFontFamilyName( const std::wstring& family, int pos, int length )
    { _textLayout->SetFontFamilyName(family.c_str(), getTextRange(pos, length)); }
    std::wstring getFontFamilyName( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    {
        wchar_t fontFamilyName[100]; fontFamilyName[0] = '\0';
        _textLayout->GetFontFamilyName(pos, &fontFamilyName[0], ARRAYSIZE(fontFamilyName), range);
        return std::wstring(fontFamilyName);
    }
    
    /** Set locale name */
    void setLocaleName( const std::wstring& locale, int pos, int length )
    { _textLayout->SetLocaleName(locale.c_str(), getTextRange(pos, length)); }
    std::wstring getLocaleName( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    {
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH]; localeName[0] = '\0';
        _textLayout->GetLocaleName(pos, &localeName[0], ARRAYSIZE(localeName), range);
        return std::wstring(localeName);
    }
    
    /** Set font weight */
    void setFontWeight( DWRITE_FONT_WEIGHT weight, int pos, int length )
    { _textLayout->SetFontWeight(weight, getTextRange(pos, length)); }
    DWRITE_FONT_WEIGHT getFontWeight( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    { DWRITE_FONT_WEIGHT weight; _textLayout->GetFontWeight(pos, &weight, range); return weight; }
    
    /** Set font style */
    void setFontStyle( DWRITE_FONT_STYLE style, int pos, int length )
    { _textLayout->SetFontStyle(style, getTextRange(pos, length)); }
    DWRITE_FONT_STYLE getFontStyle( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    { DWRITE_FONT_STYLE style; _textLayout->GetFontStyle(pos, &style, range); return style; }
    
    /** Set font stretch */
    void setFontStretch( DWRITE_FONT_STRETCH stretch, int pos, int length )
    { _textLayout->SetFontStretch(stretch, getTextRange(pos, length)); }
    DWRITE_FONT_STRETCH getFontStretch( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    { DWRITE_FONT_STRETCH stretch; _textLayout->GetFontStretch(pos, &stretch, range); return stretch; }
    
    /** Set font em height */
    void setFontSize( float size, int pos, int length )
    { _textLayout->SetFontSize(size, getTextRange(pos, length)); }
    float getFontSize( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    { FLOAT size; _textLayout->GetFontSize(pos, &size, range); return size; }
    
    /** Set underline */
    void setUnderline( bool underline, int pos, int length )
    { _textLayout->SetUnderline(underline?1:0, getTextRange(pos, length)); }
    bool getUnderline( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    { BOOL underline; _textLayout->GetUnderline(pos, &underline, range); return (!!underline); }
    
    /** Set strikethrough */
    void setStrikethrough( bool strike, int pos, int length )
    { _textLayout->SetStrikethrough(strike?1:0, getTextRange(pos, length)); }
    bool getStrikethrough( unsigned int pos=0, DWRITE_TEXT_RANGE* range=0 ) const
    { BOOL strike; _textLayout->GetStrikethrough(pos, &strike, range); return (!!strike); }
    
    virtual bool requiresUpdateCall() const { return true; }
    virtual void update( osg::NodeVisitor* nv );
    
protected:
    virtual ~DirectWriteImage();
    IDWriteTextLayout* createNewTextLayout( const std::wstring& text );
    void copyLayoutProperties( IDWriteTextLayout* oldLayout, UINT32 oldPos,
                               IDWriteTextLayout* newLayout, UINT32 newPos, UINT32 newLength );
    
    DWRITE_TEXT_RANGE getTextRange( int pos, int length )
    {
        UINT32 startIndex = 0, textLength = 0;
        int size = (int)_content.size();
        
        if ( pos<0 || pos>=size ) startIndex = 0;
        else startIndex = (UINT32)pos;
        
        if ( length<=0 ) textLength = 1;
        else if ( pos+length<size ) textLength = (UINT32)length;
        else textLength = (UINT32)(size - pos);
        
        DWRITE_TEXT_RANGE range = {startIndex, textLength};
        return range;
    }
    
    IDWriteTextRenderer* _textRenderer;
    IDWriteTextFormat* _textFormat;
    IDWriteTextLayout* _textLayout;
    IDWriteBitmapRenderTarget* _bitmapRenderTarget;
    IDWriteRenderingParams* _renderingParams;
    std::wstring _content;
    osg::Vec2 _textOrigin;
    bool _dirty;
};

#endif
