#include <osg/Image>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <font_freetype/agg_font_freetype.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_renderer_base.h>
#include <agg_pixfmt_rgb.h>
#include <agg_pixfmt_rgba.h>

osg::Image* gradientImage()
{
    unsigned char* buffer = new unsigned char[640 * 480 * 3];
    agg::rendering_buffer renderingBuf( buffer, 640, 480, 640*3 );
    agg::pixfmt_rgb24 pixelFormat( renderingBuf );
    agg::renderer_base<agg::pixfmt_rgb24> renderer( pixelFormat );
    renderer.clear( agg::rgba8(150, 150, 150) );
    
    agg::rgba8 span[640];
    for ( unsigned int i=0; i<640; ++i )
    {
        agg::rgba c(380.0 + 400.0 * (float)i / 640.0, 0.8);
        span[i] = agg::rgba8(c);
    }
    for ( unsigned int i=0; i<480; ++i )
    {
        renderer.blend_color_hspan( 0, i, 640, span, NULL );
    }
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->setImage( 640, 480, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, buffer, osg::Image::USE_NEW_DELETE );
    return image.release();
}

osg::Image* textImage()
{
    unsigned char* buffer = new unsigned char[640 * 480 * 4];
    agg::rendering_buffer renderingBuf( buffer, 640, 480, 640*4 );
    agg::pixfmt_rgba32 pixelFormat( renderingBuf );
    agg::renderer_base<agg::pixfmt_rgba32> renderer( pixelFormat );
    renderer.clear( agg::rgba8(0, 0, 0, 0) );
    
    agg::renderer_scanline_bin_solid< agg::renderer_base<agg::pixfmt_rgba32> > solidBin( renderer );
    solidBin.color( agg::rgba8(255, 255, 255, 255) );
    
    agg::font_engine_freetype_int32 font;
    agg::font_cache_manager<agg::font_engine_freetype_int32> fontManager(font);
    if ( font.load_font("arial.ttf", 0, agg::glyph_ren_native_mono) )
    {
        font.hinting( true );
        font.height( 64 );
        font.width( 64 );
        font.flip_y( true );
        
        agg::trans_affine transformation;
        transformation *= agg::trans_affine_rotation( agg::deg2rad(-4.0) );
        font.transform( transformation );
        
        double x = 30.0, y = 450.0;
        const char* text = "Hello World!";
        const char* ptr = text;
        while ( *ptr )
        {
            const agg::glyph_cache* glyph = fontManager.glyph( *ptr );
            if ( glyph )
            {
                fontManager.add_kerning( &x, &y );
                fontManager.init_embedded_adaptors( glyph, x, y );
                
                switch ( glyph->data_type )
                {
                case agg::glyph_data_mono:
                    agg::render_scanlines(
                        fontManager.mono_adaptor(), fontManager.mono_scanline(), solidBin );
                    break;
                default: break;
                }
                
                x += glyph->advance_x;
                y += glyph->advance_y;
            }
            ptr++;
        }
    }
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->setImage( 640, 480, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, buffer, osg::Image::USE_NEW_DELETE );
    return image.release();
}

osg::Node* createImageQuad( osg::Image* image, const osg::Vec3& corner )
{
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( image );
    texture->setResizeNonPowerOfTwoHint( false );
    
    osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(
        corner, osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f), 0.0f, 1.0f, 1.0f, 0.0f );
    quad->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    if ( image->isImageTranslucent() )
    {
        quad->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
        quad->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    }
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    return geode.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( createImageQuad(gradientImage(), osg::Vec3()) );
    scene->addChild( createImageQuad(textImage(), osg::Vec3(1.1f, 0.0f, 0.0f)) );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
