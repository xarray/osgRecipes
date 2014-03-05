#include <agg_arc.h>
#include <agg_arrowhead.h>
#include <agg_rounded_rect.h>
#include <agg_ellipse.h>
#include <agg_conv_bspline.h>
#include <agg_conv_contour.h>
#include <agg_conv_curve.h>
#include <agg_conv_dash.h>
#include <agg_conv_marker.h>
#include <agg_conv_smooth_poly1.h>
#include <agg_conv_stroke.h>
#include <agg_conv_transform.h>
#include <agg_vcgen_markers_term.h>
#include <agg_span_image_filter_gray.h>
#include <agg_span_image_filter_rgb.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_allocator.h>
#include <agg_span_interpolator_linear.h>
#include <agg_image_accessors.h>
#include <agg_scanline_p.h>
#include <util/agg_color_conv_rgb8.h>
#include <util/agg_color_conv_rgb16.h>
#include "Drawer2D.h"

/* DrawAdapter */

template<typename PixelFormat, typename SpanGeneratorType>
class DrawAdapter : public AggDrawer::DrawAdapterBase
{
public:
    typedef agg::renderer_base<PixelFormat> RendererBaseType;
    typedef agg::renderer_scanline_aa_solid< agg::renderer_base<PixelFormat> > RendererType;
    typedef agg::span_interpolator_linear<> InterpolatorType;
    typedef agg::image_accessor_clone<PixelFormat> ImageAccessorType;
    
    DrawAdapter( AggDrawer* d=0 ) : AggDrawer::DrawAdapterBase(d)
    {
        setAntiAlias( true );
        rasterizer.clip_box( 0.0, 0.0, d->s(), d->t() );
    }
    
    virtual void setAntiAlias( bool flag )
    {
        if ( flag ) rasterizer.gamma( agg::gamma_linear() );
        else rasterizer.gamma( agg::gamma_threshold(0.5) );
    }
    
    virtual void clear( const agg::rgba8& color, bool resetClipBox )
    {
        if ( !drawer->getRenderBuffer() )
        {
            OSG_NOTICE << "[AggDrawer] The rendering buffer is not allocated" << std::endl;
            return;
        }
        
        if ( resetClipBox ) drawer->clip( 0, 0, drawer->s() - 1, drawer->t() - 1 );
        PixelFormat pixelFormat( *(drawer->getRenderBuffer()) );
        RendererBaseType rendererBase( pixelFormat );
        
        const agg::rect_i& rect = drawer->getClipBox();
        rendererBase.clip_box( rect.x1, rect.y1, rect.x2, rect.y2 );
        rendererBase.clear( color );
    }
    
    virtual void drawPixels( int x0, int y0, int x1, int y1, const agg::rgba8& color )
    {
        if ( !drawer->getRenderBuffer() )
        {
            OSG_NOTICE << "[AggDrawer] The rendering buffer is not allocated" << std::endl;
            return;
        }
        
        PixelFormat pixelFormat( *(drawer->getRenderBuffer()) );
        RendererBaseType rendererBase( pixelFormat );
        
        const agg::rect_i& rect = drawer->getClipBox();
        rendererBase.clip_box( rect.x1, rect.y1, rect.x2, rect.y2 );
        for ( int y=y0; y<y1; ++y )
        {
            rendererBase.copy_hline( x0, y, x1, color );
        }
    }
    
    virtual void drawData( agg::path_storage& canvas, agg::trans_affine& matrix, agg::rendering_buffer& data )
    {
        if ( !drawer->getRenderBuffer() )
        {
            OSG_NOTICE << "[AggDrawer] The rendering buffer is not allocated" << std::endl;
            return;
        }
        
        PixelFormat pixelFormat( *(drawer->getRenderBuffer()) );
        RendererBaseType rendererBase( pixelFormat );
        
        const agg::rect_i& rect = drawer->getClipBox();
        rendererBase.clip_box( rect.x1, rect.y1, rect.x2, rect.y2 );
        
        agg::trans_affine imageMatrix;
        if ( drawer->getTransform() ) imageMatrix = *(drawer->getTransform());
        imageMatrix *= matrix;
        imageMatrix.invert();
        
        typedef agg::span_allocator<RendererBaseType::color_type> AllocatorType;
        AllocatorType allocator;

        PixelFormat pixelFormatOfData( data );
        InterpolatorType interpolator( imageMatrix );
        ImageAccessorType imageAccessor( pixelFormatOfData );
        SpanGeneratorType generator( imageAccessor, interpolator );
        
        rasterizer.reset();
        agg::conv_contour<agg::path_storage> converter( canvas );
        rasterizer.add_path( converter );
        agg::render_scanlines_aa( rasterizer, scanline, rendererBase, allocator, generator );
    }
    
    virtual void drawPath( agg::path_storage& path, bool usePen, bool useBrush )
    {
        if ( !drawer->getRenderBuffer() )
        {
            OSG_NOTICE << "[AggDrawer] The rendering buffer is not allocated" << std::endl;
            return;
        }
        
        PixelFormat pixelFormat( *(drawer->getRenderBuffer()) );
        RendererBaseType rendererBase( pixelFormat );
        RendererType renderer( rendererBase );
        
        const agg::rect_i& rect = drawer->getClipBox();
        rendererBase.clip_box( rect.x1, rect.y1, rect.x2, rect.y2 );
        
        agg::path_storage* realPath = NULL;
        if ( drawer->getTransform() )
        {
            realPath = new agg::path_storage;
            agg::conv_transform<agg::path_storage, agg::trans_affine> converter(
                path, *(drawer->getTransform()) );
            realPath->concat_path( converter );
        }
        else
            realPath = &path;
        
        AggDrawer::Pen& pen = drawer->getPen();
        AggDrawer::Brush& brush = drawer->getBrush();
        if ( useBrush && brush.enabled )
        {
            agg::conv_contour<agg::path_storage> contour( *realPath );
            contour.auto_detect_orientation( true );
            if ( usePen && pen.enabled ) contour.width( pen.width/2.0 );
            else contour.width( 0.5 );
            
            rasterizer.reset();
            rasterizer.add_path( contour );
            renderer.color( brush.color );
            agg::render_scanlines( rasterizer, scanline, renderer );
        }
        
        if ( usePen && pen.enabled )
        {
            rasterizer.reset();
            if ( pen.dashStyle.size()>0 )
            {
                agg::conv_dash<agg::path_storage> dash( *realPath );
                for ( unsigned int i=0; i<pen.dashStyle.size()-1; i+=2 )
                    dash.add_dash( pen.dashStyle[i]*pen.width, pen.dashStyle[i+1]*pen.width );
                
                agg::conv_stroke< agg::conv_dash<agg::path_storage> > stroke( dash );
                stroke.width( pen.width );
                rasterizer.add_path( stroke );
            }
            else
            {
                agg::conv_stroke<agg::path_storage> stroke( *realPath );
                stroke.width( pen.width );
                rasterizer.add_path( stroke );
            }
            renderer.color( pen.color );
            agg::render_scanlines( rasterizer, scanline, renderer );
        }
        if ( drawer->getTransform() ) delete realPath;
    }
    
    virtual void drawText( float x, float y, const wchar_t* text )
    {
        AggDrawer::Font& font = drawer->getFont();
        if ( !drawer->getRenderBuffer() )
        {
            OSG_NOTICE << "[AggDrawer] The rendering buffer is not allocated" << std::endl;
            return;
        }
        
        PixelFormat pixelFormat( *(drawer->getRenderBuffer()) );
        RendererBaseType rendererBase( pixelFormat );
        RendererType renderer( rendererBase );
        
        const agg::rect_i& rect = drawer->getClipBox();
        rendererBase.clip_box( rect.x1, rect.y1, rect.x2, rect.y2 );
        
        typedef agg::conv_curve<AggDrawer::Font::ManagerType::path_adaptor_type> CurveType;
        CurveType curves( font.manager->path_adaptor() );
        curves.approximation_scale( 1.0f );
        
        if ( font.dirty )
        {
            // Reload font if necessary
            updateFont( font );
            font.dirty = false;
        }
        
        if ( !font.engine->num_faces() ) return;
        font.engine->hinting( true );
        font.engine->flip_y( drawer->getFlipped() );
        font.engine->width( font.width );
        font.engine->height( font.height );
        
        // Start rendering the text
        renderer.color( drawer->getPen().color );
        const wchar_t* ptr = text;
        double px = x, py = y;
        while ( *ptr )
        {
            const agg::glyph_cache* glyph = font.manager->glyph( (unsigned int)*ptr );
            if ( glyph )
            {
                font.manager->add_kerning( &px, &py );
                font.manager->init_embedded_adaptors( glyph, px, py );
                
                switch ( glyph->data_type )
                {
                case agg::glyph_data_gray8:
                    agg::render_scanlines(
                        font.manager->gray8_adaptor(), font.manager->gray8_scanline(), renderer );
                    break;
                case agg::glyph_data_outline:
                    rasterizer.reset();
                    if ( drawer->getTransform() )
                    {
                        agg::conv_transform<CurveType, agg::trans_affine> converter(
                            curves, *(drawer->getTransform()) );
                        rasterizer.add_path( converter );
                    }
                    else
                        rasterizer.add_path( curves );
                    agg::render_scanlines( rasterizer, scanline, renderer );
                    break;
                default: break;
                }
                px += glyph->advance_x;
                py += glyph->advance_y;
            }
            ptr++;
        }
    }
    
    void measureText( float x, float y, const wchar_t* text, float& w, float& h )
    {
        AggDrawer::Font& font = drawer->getFont();
        if ( font.dirty )
        {
            // Reload font if necessary
            updateFont( font );
            font.dirty = false;
        }
        
        if ( !font.engine->num_faces() ) return;
        font.engine->hinting( true );
        font.engine->flip_y( drawer->getFlipped() );
        font.engine->width( font.width );
        font.engine->height( font.height );
        
        // Start measuring text
        const wchar_t* ptr = text;
        double px = x, py = y;
        while ( *ptr )
        {
            const agg::glyph_cache* glyph = font.manager->glyph( (unsigned int)*ptr );
            if ( glyph )
            {
                font.manager->add_kerning( &px, &py );
                font.manager->init_embedded_adaptors( glyph, px, py );
                px += glyph->advance_x;
                py += glyph->advance_y;
            }
            ptr++;
        }
        w = px - x; if ( w==0.0f ) w = font.width;
        h = py - y; if ( h==0.0f ) h = font.height;
    }
    
protected:
    void updateFont( AggDrawer::Font& font )
    {
        bool fontLoaded = false;
        switch ( font.drawMode )
        {
        case AggDrawer::Font::GRAY_GRAPH:
            fontLoaded = font.engine->load_font( font.file.c_str(), 0, agg::glyph_ren_native_gray8 );
            break;
        case AggDrawer::Font::OUTLINE_GRAPH:
            fontLoaded = font.engine->load_font( font.file.c_str(), 0, agg::glyph_ren_outline );
            break;
        default:
            break;
        }
        
        if ( !fontLoaded )
        {
            OSG_NOTICE << "[AggDrawer] Unable to load font file " << font.file << std::endl;
        }
    }
    
    agg::rasterizer_scanline_aa<> rasterizer;
    agg::scanline_p8 scanline;
};

/* AggDrawer */

AggDrawer::AggDrawer()
:   _adapter(NULL), _renderBuffer(NULL), _transform(NULL), _flipped(false)
{
}

AggDrawer::~AggDrawer()
{
    if ( _renderBuffer ) delete _renderBuffer;
    if ( _transform ) delete _transform;
    if ( _adapter ) delete _adapter;
}

AggDrawer::AggDrawer( const AggDrawer& copy, const osg::CopyOp& op )
:   osg::Image(copy, op), _adapter(copy._adapter),
    _renderBuffer(copy._renderBuffer), _transform(copy._transform),
    _clipBox(copy._clipBox), _lastLinePoint(copy._lastLinePoint),
    _pen(copy._pen), _brush(copy._brush), _font(copy._font),
    _flipped(copy._flipped)
{
}

void AggDrawer::allocateImage( int s, int t, int r, GLenum pixelFormat, GLenum type, int packing )
{
    osg::Image::allocateImage( s, t, r, pixelFormat, type, packing );
    resetRenderer( s, t, pixelFormat, type, packing );
}

void AggDrawer::setImage( int s, int t, int r, GLint internalTextureformat, GLenum pixelFormat,
                          GLenum type, unsigned char* data, AllocationMode mode, int packing )
{
    osg::Image::setImage( s, t, r, internalTextureformat, pixelFormat, type, data, mode, packing );
    resetRenderer( s, t, pixelFormat, type, packing );
}

void AggDrawer::setPenStyle( AggDrawer::PenStyle style, float scale )
{
    _pen.dashStyle.clear();
    switch ( style )
    {
    case DASH_LINE:
        _pen.dashStyle.push_back( 4.0f ); _pen.dashStyle.push_back( 1.0f );
        break;
    case DOT_LINE:
        _pen.dashStyle.push_back( 1.0f ); _pen.dashStyle.push_back( 1.0f );
        break;
    case DASH_DOT_LINE:
        _pen.dashStyle.push_back( 4.0f ); _pen.dashStyle.push_back( 1.0f );
        _pen.dashStyle.push_back( 1.0f ); _pen.dashStyle.push_back( 1.0f );
        break;
    case DASH_DOT_DOT_LINE:
        _pen.dashStyle.push_back( 4.0f ); _pen.dashStyle.push_back( 1.0f );
        _pen.dashStyle.push_back( 1.0f ); _pen.dashStyle.push_back( 1.0f );
        _pen.dashStyle.push_back( 1.0f ); _pen.dashStyle.push_back( 1.0f );
        break;
    default: break;
    }
}

void AggDrawer::resetRenderer( int s, int t, GLenum pixelFormat, GLenum type, int packing )
{
    int stride = 0;
    destroyRenderer();
    if ( type!=GL_UNSIGNED_BYTE )
    {
        OSG_NOTICE << "[AggDrawer] Unsupported data type" << std::endl;
        return;
    }
    
    typedef agg::span_interpolator_linear<> InterpolatorType;
    switch ( pixelFormat )
    {
    case GL_ALPHA: case GL_LUMINANCE: case GL_INTENSITY:
        typedef agg::image_accessor_clone<agg::pixfmt_gray8> GrayImageAccessorType;
        typedef agg::span_image_filter_gray_bilinear<GrayImageAccessorType, InterpolatorType> GraySpanType;
        _adapter = new DrawAdapter<agg::pixfmt_gray8, GraySpanType>(this);
        stride = s;
        break;
    case GL_RGB:
        typedef agg::image_accessor_clone<agg::pixfmt_rgb24> RGBImageAccessorType;
        typedef agg::span_image_filter_rgb_bilinear<RGBImageAccessorType, InterpolatorType> RGBSpanType;
        _adapter = new DrawAdapter<agg::pixfmt_rgb24, RGBSpanType>(this);
        stride = s * 3;
        break;
    case GL_RGBA:
        typedef agg::image_accessor_clone<agg::pixfmt_rgba32> RGBAImageAccessorType;
        typedef agg::span_image_filter_rgba_bilinear<RGBAImageAccessorType, InterpolatorType> RGBASpanType;
        _adapter = new DrawAdapter<agg::pixfmt_rgba32, RGBASpanType>(this);
        stride = s * 4;
        break;
    default:
        OSG_NOTICE << "[AggDrawer] Unsupported pixel format" << std::endl;
        return;
    }
    
    unsigned char* data = this->data();
    _renderBuffer = new agg::rendering_buffer( data, s, t, _flipped ? -stride : stride );
    _clipBox = agg::rect_i(0, 0, s - 1, t - 1);
}

void AggDrawer::destroyRenderer()
{
    if ( _renderBuffer ) delete _renderBuffer;
    if ( _transform ) delete _transform;
    if ( _adapter ) delete _adapter;
}

#define CHECK_DRAWER() \
    if (!canDraw()) { OSG_NOTICE << "[AggDrawer] The drawer is invalid" << std::endl; return; }
#define CHECK_TRANSFORM() \
    if (!_transform) _transform = new agg::trans_affine;

void AggDrawer::setAntiAlias( bool flag )
{
    CHECK_DRAWER();
    _adapter->setAntiAlias( flag );
}

void AggDrawer::clear( int r, int g, int b, int a, bool resetClipBox )
{
    CHECK_DRAWER();
    _adapter->clear( agg::rgba8(r, g, b, a), resetClipBox );
}

void AggDrawer::drawPixels( const osg::Vec2& p0, const osg::Vec2& p1, int r, int g, int b, int a )
{
    CHECK_DRAWER();
    _adapter->drawPixels( p0.x(), p0.y(), p1.x(), p1.y(), agg::rgba8(r, g, b, a) );
}

void AggDrawer::drawImage( const osg::Vec4& src, const osg::Vec4& dst, osg::Image* image )
{
    CHECK_DRAWER();
    if ( !image ) return;
    if ( image->getDataType()!=getDataType() )
    {
        OSG_NOTICE << "[AggDrawer] The sub-image to be drawn does not have the same data type "
                   << "with the target one. Automatic conversion is not supported either, "
                   << "so we have to give up" << std::endl;
        return;
    }
    
    int stride = 0, s = image->s(), t = image->t();
    switch ( image->getPixelFormat() )
    {
    case GL_ALPHA: case GL_LUMINANCE: case GL_INTENSITY: stride = s; break;
    case GL_RGB: stride = s * 3; break;
    case GL_RGBA: stride = s * 4; break;
    default: break;
    }
    
    unsigned char* data = image->data();
    agg::rendering_buffer imageBuffer( data, s, t, stride );
    
    agg::path_storage canvas;
    canvas.move_to( dst.x(), dst.y() );
    canvas.line_to( dst.x() + dst.z(), dst.y() );
    canvas.line_to( dst.x() + dst.z(), dst.y() + dst.w() );
    canvas.line_to( dst.x(), dst.y() + dst.w() );
    canvas.close_polygon();
    
    agg::trans_affine imageMatrix;
    imageMatrix.multiply( agg::trans_affine_translation(-src.x(), -src.y()) );
    imageMatrix.multiply( agg::trans_affine_scaling(dst.z() / src.z(), dst.w() / src.w()) );
    imageMatrix.multiply( agg::trans_affine_translation(dst.x(), dst.y()) );
    
    if ( image->getPixelFormat()!=getPixelFormat() )
    {
        switch ( getPixelFormat() )
        {
        case GL_ALPHA: case GL_LUMINANCE: case GL_INTENSITY: stride = s; break;
        case GL_RGB: stride = s * 3; break;
        case GL_RGBA: stride = s * 4; break;
        default: stride = 0; break;
        }
        agg::int8u* newData = new agg::int8u[t * stride];
        agg::rendering_buffer newBuffer( newData, s, t, stride );
        
        // Automatic pixel format conversion
        GLenum srcFormat = image->getPixelFormat();
        GLenum dstFormat = getPixelFormat();
        if ( srcFormat==GL_RGB && dstFormat==GL_RGBA )
        {
            agg::color_conv( &newBuffer, &imageBuffer, agg::color_conv_rgb24_to_rgba32() );
        }
        else if ( srcFormat==GL_RGBA && dstFormat==GL_RGB )
        {
            agg::color_conv( &newBuffer, &imageBuffer, agg::color_conv_rgba32_to_rgb24() );
        }
        else
        {
            OSG_NOTICE << "[AggDrawer] The sub-image to be drawn does not have the same pixel format "
                       << "with the target one. Automatic conversion is not supported either, "
                       << "so we have to give up" << std::endl;
            delete[] newData;
            return;
        }
        _adapter->drawData( canvas, imageMatrix, newBuffer );
        delete[] newData;
    }
    else
        _adapter->drawData( canvas, imageMatrix, imageBuffer );
}

void AggDrawer::drawLine( const osg::Vec2& p0, const osg::Vec2& p1 )
{
    CHECK_DRAWER();
    agg::path_storage path;
    path.move_to( p0.x(), p0.y() );
    path.line_to( p1.x(), p1.y() );
    _adapter->drawPath( path, true, false );
    _lastLinePoint = p1;
}

void AggDrawer::drawLineStrip( const std::vector<osg::Vec2>& points )
{
    CHECK_DRAWER();
    unsigned int size = points.size();
    if ( size<2 )
    {
        OSG_NOTICE << "[AggDrawer] Cannot draw line strip with less than 2 points" << std::endl;
        return;
    }
    
    agg::path_storage path;
    path.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        path.line_to( points[i].x(), points[i].y() );
    _adapter->drawPath( path, true, false );
    _lastLinePoint = points.back();
}

void AggDrawer::drawLineTo( const osg::Vec2& p, bool onlyMove )
{
    if ( !onlyMove )
    {
        CHECK_DRAWER();
        agg::path_storage path;
        path.move_to( _lastLinePoint.x(), _lastLinePoint.y() );
        path.line_to( p.x(), p.y() );
        _adapter->drawPath( path, true, false );
    }
    _lastLinePoint = p;
}

void AggDrawer::drawSpline( const std::vector<osg::Vec2>& points )
{
    CHECK_DRAWER();
    unsigned int size = points.size();
    if ( size<2 )
    {
        OSG_NOTICE << "[AggDrawer] Cannot draw spline with less than 2 points" << std::endl;
        return;
    }
    
    agg::path_storage controls;
    controls.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        controls.line_to( points[i].x(), points[i].y() );
    
    agg::conv_bspline<agg::path_storage> converter( controls );
    agg::path_storage path;
    path.concat_path( converter );
    _adapter->drawPath( path, true, false );
}

void AggDrawer::drawComplexArc( const osg::Vec2& center, float r1, float r2,
                                float startAngle, float endAngle, bool filled, bool ccw )
{
    CHECK_DRAWER();
    agg::arc arc( center.x(), center.y(), r1, r2, startAngle, endAngle, ccw );
    arc.approximation_scale( 1.0f );
    
    agg::path_storage path;
    path.concat_path( arc );
    if ( filled ) path.close_polygon();
    _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawComplexPie( const osg::Vec2& center, float r1, float r2,
                                float startAngle, float endAngle, bool ccw )
{
    CHECK_DRAWER();
    agg::arc arc( center.x(), center.y(), r1, r2, startAngle, endAngle, ccw );
    arc.approximation_scale( 1.0f );
    
    agg::path_storage path;
    path.concat_path( arc );
    path.line_to( center.x(), center.y() );
    path.close_polygon();
    _adapter->drawPath( path, true, true );
}

void AggDrawer::drawEllipse( const osg::Vec2& center, float r1, float r2, bool filled )
{
    CHECK_DRAWER();
    agg::ellipse ellipse( center.x(), center.y(), r1, r2, 0 );
    ellipse.approximation_scale( 1.0f );
    
    agg::path_storage path;
    path.concat_path( ellipse );
    path.close_polygon();
    _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawRectangle( const osg::Vec2& p0, const osg::Vec2& p1, bool filled )
{
    CHECK_DRAWER();
    agg::path_storage path;
    path.move_to( p0.x(), p0.y() );
    path.line_to( p1.x(), p0.y() );
    path.line_to( p1.x(), p1.y() );
    path.line_to( p0.x(), p1.y() );
    path.close_polygon();
    _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawRoundedRectangle( const osg::Vec2& p0, const osg::Vec2& p1, float r, bool filled )
{
    CHECK_DRAWER();
    agg::rounded_rect rect( p0.x(), p0.y(), p1.x(), p1.y(), r );
    agg::path_storage path;
    path.concat_path( rect );
    path.close_polygon();
    _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawComplexRoundedRectangle( const osg::Vec2& p0, const osg::Vec2& p1,
                                             const std::vector<osg::Vec2>& radiusVecs, bool filled )
{
    CHECK_DRAWER();
    unsigned int size = radiusVecs.size();
    if ( size<4 )
    {
        OSG_NOTICE << "[AggDrawer] Cannot draw rounded rectangle without 4 radius vectors" << std::endl;
        return;
    }
    
    agg::rounded_rect rect( p0.x(), p0.y(), p1.x(), p1.y(), 0.0f );
    rect.radius( radiusVecs[0].x(), radiusVecs[0].y(),
                 radiusVecs[1].x(), radiusVecs[1].y(),
                 radiusVecs[2].x(), radiusVecs[2].y(), 
                 radiusVecs[3].x(), radiusVecs[3].y() );
    
    agg::path_storage path;
    path.concat_path( rect );
    path.close_polygon();
    _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawPolygon( const std::vector<osg::Vec2>& points, float smoothValue, bool filled )
{
    CHECK_DRAWER();
    unsigned int size = points.size();
    if ( size<3 )
    {
        OSG_NOTICE << "[AggDrawer] Cannot draw polygon with less than 3 points" << std::endl;
        return;
    }
    
    agg::path_storage path;
    path.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        path.line_to( points[i].x(), points[i].y() );
    path.close_polygon();
    
    if ( smoothValue>0.0f )
    {
        agg::conv_smooth_poly1_curve<agg::path_storage> converter( path );
        converter.smooth_value( smoothValue );
        
        agg::path_storage path2;
        path2.concat_path( converter );
        _adapter->drawPath( path2, true, filled );
    }
    else
        _adapter->drawPath( path, true, filled );
}

bool AggDrawer::ShapeLineStrip::createPath()
{
    unsigned int size = points.size();
    if ( size<2 ) return false;
    
    path.remove_all();
    path.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        path.line_to( points[i].x(), points[i].y() );
    return true;
}

bool AggDrawer::ShapeSmoothedLineStrip::createPath()
{
    unsigned int size = points.size();
    if ( size<2 ) return false;
    
    agg::path_storage originalPath;
    originalPath.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        originalPath.line_to( points[i].x(), points[i].y() );
    
    agg::conv_smooth_poly1_curve<agg::path_storage> converter( originalPath );
    converter.smooth_value( smoothValue );
    path.remove_all();
    path.concat_path( converter );
    return true;
}

bool AggDrawer::ShapeSpline::createPath()
{
    unsigned int size = points.size();
    if ( size<2 ) return false;
    
    agg::path_storage controls;
    controls.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        controls.line_to( points[i].x(), points[i].y() );
    
    agg::conv_bspline<agg::path_storage> converter( controls );
    path.remove_all();
    path.concat_path( converter );
    return true;
}

void AggDrawer::drawComplexShape( const ComplexShapeList& shapes, bool closed, bool filled )
{
    CHECK_DRAWER();
    if ( !shapes.size() ) return;
    
    agg::path_storage path;
    for ( unsigned int i=0; i<shapes.size(); ++i )
    {
        agg::path_storage& subPath = shapes[i]->path;
        unsigned int pathSize = subPath.total_vertices();
        if ( pathSize>0 )
        {
            // We have to keep every last point as 'line-to' to make paths connected correctly
            subPath.modify_command( pathSize-1, agg::path_cmd_line_to );
            path.join_path( subPath );
        }
    }
    
    if ( closed ) path.close_polygon();
    _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawContour( const std::vector<osg::Vec2>& points, float width,
                             ContourType type, float smoothValue, bool filled )
{
    CHECK_DRAWER();
    unsigned int size = points.size();
    if ( size<3 )
    {
        OSG_NOTICE << "[AggDrawer] Cannot draw contour with less than 3 points" << std::endl;
        return;
    }
    
    agg::path_storage contour;
    contour.move_to( points.front().x(), points.front().y() );
    for ( unsigned int i=1; i<size; ++i )
        contour.line_to( points[i].x(), points[i].y() );
    
    agg::conv_contour<agg::path_storage> converter( contour );
    converter.auto_detect_orientation( true );
    converter.width( width );
    
    switch ( type )
    {
    case BEVEL: converter.line_join( agg::bevel_join ); break;
    case ROUND: converter.line_join( agg::round_join ); break;
    default: converter.line_join( agg::miter_join ); break;
    }
    
    agg::path_storage path;
    path.concat_path( converter );
    path.close_polygon();
    
    if ( smoothValue>0.0f )
    {
        agg::conv_smooth_poly1_curve<agg::path_storage> converter( path );
        converter.smooth_value( smoothValue );
        
        agg::path_storage path2;
        path2.concat_path( converter );
        _adapter->drawPath( path2, true, filled );
    }
    else
        _adapter->drawPath( path, true, filled );
}

void AggDrawer::drawText( const osg::Vec2& pos, const wchar_t* text )
{
    CHECK_DRAWER();
    _adapter->drawText( pos.x(), pos.y(), text );
}

osg::Vec2 AggDrawer::measureText( const osg::Vec2& pos, const wchar_t* text )
{
    float w = 0.0f, h = 0.0f;
    if ( !canDraw() )
    {
        OSG_NOTICE << "[AggDrawer] The drawer is invalid" << std::endl;
        return osg::Vec2(w, h);
    }
    
    _adapter->measureText( pos.x(), pos.y(), text, w, h );
    return osg::Vec2(w, h);
}

void AggDrawer::drawArrowHead( const osg::Vec2& p0, const osg::Vec4& head,
                               const osg::Vec2& p1, const osg::Vec4& tail )
{
    CHECK_DRAWER();
    agg::path_storage line;
    line.move_to( p1.x(), p1.y() );
    line.line_to( p0.x(), p0.y() );
    agg::conv_stroke<agg::path_storage, agg::vcgen_markers_term> lineConv( line );
    
    agg::arrowhead arrowHead;
    if ( head[3]==0.0f ) arrowHead.no_head();
    else arrowHead.head( head[0], head[1], head[2], head[3] );
    if ( tail[3]==0.0f ) arrowHead.no_tail();
    else arrowHead.tail( tail[0], tail[1], tail[2], tail[3] );
    agg::conv_marker<agg::vcgen_markers_term, agg::arrowhead> arrowConv( lineConv.markers(), arrowHead );
    
    agg::path_storage path;
    path.concat_path( lineConv );
    path.concat_path( arrowConv );
    
    agg::rgba8 lastPenColor = _pen.color;
    _pen.color = _brush.color;  // Use the same color to draw arrow line (pen) and head (brush)
    _adapter->drawPath( path, true, true );
    _pen.color = lastPenColor;
}

void AggDrawer::translate( float x, float y, bool postMultiply )
{
    CHECK_TRANSFORM();
    if ( postMultiply ) _transform->multiply( agg::trans_affine_translation(x, y) );
    else _transform->premultiply( agg::trans_affine_translation(x, y) );
}

void AggDrawer::rotate( float angle, bool postMultiply )
{
    CHECK_TRANSFORM();
    if ( postMultiply ) _transform->multiply( agg::trans_affine_rotation(angle) );
    else _transform->premultiply( agg::trans_affine_rotation(-angle) );
}

void AggDrawer::scale( float x, float y, bool postMultiply )
{
    CHECK_TRANSFORM();
    if ( postMultiply ) _transform->multiply( agg::trans_affine_scaling(x, y) );
    else _transform->premultiply( agg::trans_affine_scaling(x, y) );
}

void AggDrawer::skew( float x, float y, bool postMultiply )
{
    CHECK_TRANSFORM();
    if ( postMultiply ) _transform->multiply( agg::trans_affine_skewing(x, y) );
    else _transform->premultiply( agg::trans_affine_skewing(x, y) );
}

void AggDrawer::setTransform( float sx, float shy, float shx, float sy, float tx, float ty )
{
    CHECK_TRANSFORM();
    _transform->sx = sx; _transform->sy = sy;
    _transform->shx = shx; _transform->shy = shy;
    _transform->tx = tx; _transform->ty = ty;
}

void AggDrawer::setTransformAlongLineSegment( const osg::Vec2& s, const osg::Vec2& e,
                                              float ratio, bool postMultiply )
{
    CHECK_TRANSFORM();
    float length = (s - e).length() * ratio;
    if ( postMultiply )
        _transform->multiply( agg::trans_affine_line_segment(s.x(), s.y(), e.x(), e.y(), length) );
    else
        _transform->premultiply( agg::trans_affine_line_segment(s.x(), s.y(), e.x(), e.y(), length) );
}
