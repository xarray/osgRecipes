#ifndef H_DRAWER2D
#define H_DRAWER2D

#include <osg/Image>
#include <font_freetype/agg_font_freetype.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_renderer_base.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_gray.h>
#include <agg_pixfmt_rgb.h>
#include <agg_pixfmt_rgba.h>

// TODO
// Image effects (blend, blur...)
// Boolean operation
// Font cache pool (FreeType side?)

/** The 2D drawing class using Agg to render directly on images */
class AggDrawer : public osg::Image
{
public:
    struct Pen
    {
        std::vector<float> dashStyle;
        agg::rgba8 color;
        float width;
        bool enabled;
        Pen() : color(255, 255, 255, 255), width(1.0f), enabled(true) {}
    };
    
    struct Brush
    {
        agg::rgba8 color;
        bool enabled;
        Brush() : color(255, 255, 255, 255), enabled(true) {}
    };
    
    struct Font
    {
        enum DrawMode { GRAY_GRAPH, OUTLINE_GRAPH };
        typedef agg::font_cache_manager<agg::font_engine_freetype_int32> ManagerType;
        
        agg::font_engine_freetype_int32* engine;
        ManagerType* manager;
        std::string file;
        DrawMode drawMode;
        float width, height;
        bool dirty;
        
        Font() : drawMode(GRAY_GRAPH), width(1.0f), height(1.0f), dirty(true)
        { engine = new agg::font_engine_freetype_int32; manager = new ManagerType(*engine); }
        
        ~Font() { if (manager) delete manager; if (engine) delete engine; }
    };
    
    class DrawAdapterBase
    {
    public:
        DrawAdapterBase( AggDrawer* d ) : drawer(d) {}
        virtual void setAntiAlias( bool flag ) = 0;
        virtual void clear( const agg::rgba8& color, bool resetClipBox ) = 0;
        virtual void drawPixels( int x0, int y0, int x1, int y1, const agg::rgba8& color ) = 0;
        virtual void drawData( agg::path_storage& canvas, agg::trans_affine& matrix,
                               agg::rendering_buffer& data ) = 0;
        virtual void drawPath( agg::path_storage& path, bool usePen, bool useBrush ) = 0;
        virtual void drawText( float x, float y, const wchar_t* text ) = 0;
        virtual void measureText( float x, float y, const wchar_t* text, float& w, float& h ) = 0;
        
    protected:
        AggDrawer* drawer;
    };
    
    AggDrawer();
    AggDrawer( const AggDrawer& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY );
    META_Object( nwCore, AggDrawer )
    
    virtual void allocateImage( int s, int t, int r, GLenum pixelFormat, GLenum type, int packing=1 );
    virtual void setImage( int s, int t, int r, GLint internalTextureformat, GLenum pixelFormat,
                           GLenum type, unsigned char* data, AllocationMode mode, int packing=1 );
    
    /** Set the image as a flipped one so to fit Y-increasing downwards coordinates */
    void setFlipped( bool b ) { _flipped = b; }
    bool getFlipped() const { return _flipped; }
    
    /** Set pen color */
    void setPenColor( int r, int g, int b, int a=255 ) { _pen.color = agg::rgba8(r, g, b, a); }
    const agg::rgba8& getPenColor() const { return _pen.color; }
    
    /** Set pen width */
    void setPenWidth( float w ) { _pen.width = w; }
    float getPenWidth() const { return _pen.width; }
    
    /** Set pen style (a list of dash and gap lengths) */
    void setPenStyle( const std::vector<float>& dashAndGaps ) { _pen.dashStyle = dashAndGaps; }
    const std::vector<float>& getPenStyle() const { return _pen.dashStyle; }
    
    /** Predefined pen styles */
    enum PenStyle { SOLID_LINE, DASH_LINE, DOT_LINE, DASH_DOT_LINE, DASH_DOT_DOT_LINE };
    
    /** Set pen style as a certain predefined one */
    void setPenStyle( PenStyle style, float scale=1.0f );
    
    /** Enable/disable pen */
    void setPenEnabled( bool b ) { _pen.enabled = b; }
    bool getPenEnabled() const { return _pen.enabled; }
    
    /** Set brush color */
    void setBrushColor( int r, int g, int b, int a=255 ) { _brush.color = agg::rgba8(r, g, b, a); }
    const agg::rgba8& getBrushColor() const { return _brush.color; }
    
    /** Enable/disable brush */
    void setBrushEnabled( bool b ) { _brush.enabled = b; }
    bool getBrushEnabled() const { return _brush.enabled; }
    
    /** Set font file */
    void setFontFile( const std::string& file, bool outline=true )
    {
        if ( file==_font.file && outline==(_font.drawMode==Font::OUTLINE_GRAPH) )
            return;
        _font.file = file;
        _font.drawMode = (outline?Font::OUTLINE_GRAPH:Font::GRAY_GRAPH);
        _font.dirty = true;
    }
    const std::string& getFontFile() const { return _font.file; }
    bool isFontOutline() const { return _font.drawMode==Font::OUTLINE_GRAPH; }
    
    /** Set font width and height */
    void setFontSize( float w, float h ) { _font.width = w; _font.height = h; }
    void getFontSize( float& w, float& h ) const { w = _font.width; h = _font.height; }
    
    /** Will be done automatically in allocateImage() or setImage() */
    void resetRenderer( int s, int t, GLenum pixelFormat, GLenum type, int packing );
    
    /** Will be done automatically in allocateImage() or setImage() */
    void destroyRenderer();
    
    /** Set to use anti-alias or not */
    void setAntiAlias( bool flag );
    
    /** Clear the buffer */
    void clear( int r, int g, int b, int a=255, bool resetClipBox=false );
    
    /** Draw direct pixels in specified area */
    void drawPixels( const osg::Vec2& p0, const osg::Vec2& p1, int r, int g, int b, int a=255 );
    
    /** Draw an image at specified left-bottom position with scalable width and height */
    void drawImage( const osg::Vec2& pos, float w, float h, osg::Image* image )
    {
        osg::Vec4 dst(pos.x(), pos.y(), w, h);
        if (image) drawImage(osg::Vec4(0.0f, 0.0f, image->s(), image->t()), dst, image);
    }
    
    /** Draw an image at src(x, y, w, h) onto current one at dst(x, y, w, h) */
    void drawImage( const osg::Vec4& src, const osg::Vec4& dst, osg::Image* image );
    
    /** Draw a line from p0 to p1 */
    void drawLine( const osg::Vec2& p0, const osg::Vec2& p1 );
    
    /** Draw a line strip with a list of points */
    void drawLineStrip( const std::vector<osg::Vec2>& points );
    
    /** Draw continuous line from last recorded line point to this point
        NOTE: only last point of drawLine*() functions will be recorded
     */
    void drawLineTo( const osg::Vec2& p, bool onlyMove=false );
    
    /** Draw a B-spline with a list of points */
    void drawSpline( const std::vector<osg::Vec2>& points );
    
    /** Draw an arc with center and radius and angles */
    void drawArc( const osg::Vec2& center, float r, float startAngle, float endAngle, bool ccw=true )
    { drawComplexArc(center, r, r, startAngle, endAngle, false, ccw); }
    
    /** Draw a filled chord with center and radius and angles */
    void drawChord( const osg::Vec2& center, float r, float startAngle, float endAngle, bool ccw=true )
    { drawComplexArc(center, r, r, startAngle, endAngle, true, ccw); }
    
    /** Draw a filled pie with center and radius and angles */
    void drawPie( const osg::Vec2& center, float r, float startAngle, float endAngle, bool ccw=true )
    { drawComplexPie(center, r, r, startAngle, endAngle, ccw); }
    
    void drawComplexArc( const osg::Vec2& center, float r1, float r2,
                         float startAngle, float endAngle, bool filled, bool ccw=true );
    void drawComplexPie( const osg::Vec2& center, float r1, float r2,
                         float startAngle, float endAngle, bool ccw=true );
    
    /** Draw a circle with center and radius */
    void drawCircle( const osg::Vec2& center, float r, bool filled=true )
    { drawEllipse(center, r, r, filled); }
    
    /** Draw an ellipse with center and two radii */
    void drawEllipse( const osg::Vec2& center, float r1, float r2, bool filled=true );
    
    /** Draw a rectangle with center and widths */
    void drawRectangle( const osg::Vec2& center, float w, float h, bool filled=true )
    { osg::Vec2 halfL(w*0.5f, h*0.5f); drawRectangle( center-halfL, center+halfL, filled ); }
    
    /** Draw a rectangle with its left-bottom and right-top points */
    void drawRectangle( const osg::Vec2& p0, const osg::Vec2& p1, bool filled=true );
    
    /** Draw a rounded rectangle with center and widths and radius */
    void drawRoundedRectangle( const osg::Vec2& center, float w, float h, float r, bool filled=true )
    { osg::Vec2 halfL(w*0.5f, h*0.5f); drawRoundedRectangle( center-halfL, center+halfL, r, filled ); }
    
    /** Draw a rounded rectangle with its left-bottom and right-top points and radius */
    void drawRoundedRectangle( const osg::Vec2& p0, const osg::Vec2& p1, float r, bool filled=true );
    
    void drawComplexRoundedRectangle( const osg::Vec2& p0, const osg::Vec2& p1,
                                      const std::vector<osg::Vec2>& radiusVecs, bool filled=true );
    
    /** Draw a polygon with a list of points */
    void drawPolygon( const std::vector<osg::Vec2>& points, float smoothValue=0.0f, bool filled=true );
    
    /** The ShapeDef base class */
    struct ShapeDef : public osg::Referenced
    {
        unsigned int size() const { return path.total_vertices(); }
        
        void setVertex( unsigned int i, const osg::Vec2& p, bool asLineTo=false )
        {
            if (asLineTo) path.modify_vertex(i, p[0], p[1], agg::path_cmd_line_to);
            else path.modify_vertex(i, p[0], p[1]);
        }
        
        osg::Vec2 getVertex( unsigned int i ) const
        { double x, y; path.vertex(i, &x, &y); return osg::Vec2(x, y); }
        
        virtual bool createPath() = 0;
        agg::path_storage path;
    };
    typedef std::vector< osg::ref_ptr<ShapeDef> > ComplexShapeList;
    
    /** The line strip shape */
    struct ShapeLineStrip : public ShapeDef
    {
        std::vector<osg::Vec2> points;
        virtual bool createPath();
    };
    
    /** The smoothed line strip shape */
    struct ShapeSmoothedLineStrip : public ShapeDef
    {
        float smoothValue;
        std::vector<osg::Vec2> points;
        ShapeSmoothedLineStrip( float s=0.0f ) : smoothValue(s) {}
        virtual bool createPath();
    };
    
    /** The spline shape */
    struct ShapeSpline : public ShapeDef
    {
        std::vector<osg::Vec2> points;
        virtual bool createPath();
    };
    
    /** Draw complex shape using a list of ShapeDef objects */
    void drawComplexShape( const ComplexShapeList& shapes, bool closed=false, bool filled=true );
    
    /** The type of the contour lines */
    enum ContourType { MITER, ROUND, BEVEL };
    
    /** Draw the contour of a list of points */
    void drawContour( const std::vector<osg::Vec2>& points, float width,
                      ContourType type=MITER, float smoothValue=0.0f, bool filled=true );
    
    /** Draw text at specified left-bottom position */
    void drawText( const osg::Vec2& pos, const wchar_t* text );
    
    /** Compute size of text but not really render them */
    osg::Vec2 measureText( const osg::Vec2& pos, const wchar_t* text );
    
    /** Draw a filled arrowhead with provided paramters (p-to-top, p-to-back, height, sunken) */
    void drawArrowHead( const osg::Vec2& p0, const osg::Vec4& head,
                        const osg::Vec2& p1, const osg::Vec4& tail );
    
    /** Add translation transform */
    void translate( float x, float y, bool postMultiply=true );
    
    /** Add rotation transform */
    void rotate( float angle, bool postMultiply=true );
    
    /** Add scale transform */
    void scale( float x, float y, bool postMultiply=true );
    
    /** Add skewing transform */
    void skew( float x, float y, bool postMultiply=true );
    
    /** Set transform affine matrix directly */
    void setTransform( float sx, float shy, float shx, float sy, float tx, float ty );
    
    /** Set transform affine matrix (include scale, rotation and translatation) along a line segment */
    void setTransformAlongLineSegment( const osg::Vec2& s, const osg::Vec2& e,
                                       float ratio=1.0f, bool postMultiply=true );
    
    /** Reset current transform */
    void resetTransform() { if (_transform) delete _transform; _transform = NULL; }
    
    /** Clip rendering area */
    void clip( int x1, int y1, int x2, int y2 ) { _clipBox = agg::rect_i(x1, y1, x2, y2); }
    
    /** Check if the drawer is valid */
    bool canDraw() const { return _adapter!=NULL; }
    
    agg::rendering_buffer* getRenderBuffer() { return _renderBuffer; }
    agg::trans_affine* getTransform() { return _transform; }
    const agg::rect_i& getClipBox() const { return _clipBox; }
    Pen& getPen() { return _pen; }
    Brush& getBrush() { return _brush; }
    Font& getFont() { return _font; }
    
protected:
    virtual ~AggDrawer();
    
    DrawAdapterBase* _adapter;
    agg::rendering_buffer* _renderBuffer;
    agg::trans_affine* _transform;
    agg::rect_i _clipBox;
    osg::Vec2 _lastLinePoint;
    Pen _pen;
    Brush _brush;
    Font _font;
    bool _flipped;
};

#endif
