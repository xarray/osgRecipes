#include <osg/ImageUtils>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <berkelium/Berkelium.hpp>
#include <berkelium/Context.hpp>
#include <berkelium/Window.hpp>
#include <berkelium/WindowDelegate.hpp>

class BerkeliumImage : public osg::Image
{
public:
    class BerkeliumWindowDelegate: public Berkelium::WindowDelegate
    {
    public:
        BerkeliumWindowDelegate( BerkeliumImage* bi )
        : Berkelium::WindowDelegate(), _image(bi) {}
        
        virtual void onPaint( Berkelium::Window* window,
            const unsigned char* bitmap, const Berkelium::Rect& rect,
            size_t num_copy_rects, const Berkelium::Rect* copy_rects,
            int dx, int dy, const Berkelium::Rect& scroll_rect )
        {
            // TODO: handle scroll_rect and copy_rects
            // FIXME: is it acceptable to allocate sub images here every time?
            osg::ref_ptr<osg::Image> subImage = new osg::Image;
            subImage->setImage( rect.width(), rect.height(), 1, 4, GL_RGBA, GL_UNSIGNED_BYTE,
                                const_cast<unsigned char*>(bitmap), osg::Image::NO_DELETE );
            osg::copyImage( subImage.get(), 0, 0, 0, rect.width(), rect.height(), 1,
                            _image, rect.x(), rect.y(), 0 );
            _image->dirty();
        }
        
    protected:
        BerkeliumImage* _image;
    };
    
    BerkeliumImage() : _lastButtonMask(0)
    {
        _window = Berkelium::Window::create( Berkelium::Context::create() );
        _window->setDelegate( new BerkeliumWindowDelegate(this) );
    }
    
    BerkeliumImage( const BerkeliumImage& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY )
    : osg::Image(copy, op), _window(copy._window), _lastButtonMask(copy._lastButtonMask) {}
    
    META_Object( osg, BerkeliumImage )
    
    void loadURL( const std::string& url, int w=0, int h=0 )
    {
        allocateImage( w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE );
        _window->resize( w, h );
        _window->navigateTo( Berkelium::URLString::point_to(url.data(), url.length()) );
    }
    
    virtual bool sendPointerEvent( int x, int y, int buttonMask )
    {
        switch ( buttonMask )
        {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
            if ( !_lastButtonMask ) _window->mouseButton( 0, true );
            else _window->mouseMoved( x, y );
            break;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
            if ( !_lastButtonMask ) _window->mouseButton( 1, true );
            else _window->mouseMoved( x, y );
            break;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            if ( !_lastButtonMask ) _window->mouseButton( 2, true );
            else _window->mouseMoved( x, y );
            break;
        default:
            _window->mouseMoved( x, y );
            break;
        }
        
        if ( _lastButtonMask!=0 && buttonMask==0 )
        {
            switch ( _lastButtonMask )
            {
            case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
                _window->mouseButton( 0, false );
                break;
            case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
                _window->mouseButton( 1, false );
                break;
            case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
                _window->mouseButton( 2, false );
                break;
            }
        }
        _lastButtonMask = buttonMask;
        return true;
    }
    
    virtual bool sendKeyEvent( int key, bool keyDown )
    {
        // TODO: confused about how Berkelium handle key events...
        return true;
    }
    
protected:
    virtual ~BerkeliumImage()
    {
        delete _window;
    }
    
    Berkelium::Window* _window;
    int _lastButtonMask;
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    if ( !Berkelium::init(Berkelium::FileString::empty()) )
    {
        std::cout << "Couldn't initialize Berkelium." << std::endl;
        return 1;
    }
    
    std::string url("http://www.openscenegraph.com");
    arguments.read( "--url", url );
    
    int w = 800, h = 600;
    arguments.read( "--width", w );
    arguments.read( "--height", h );
    
    osg::ref_ptr<BerkeliumImage> image = new BerkeliumImage;
    image->loadURL( url, w, h );
    
    osg::ref_ptr<osgViewer::InteractiveImageHandler> handler = new osgViewer::InteractiveImageHandler( image.get() );
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( image.get() );
    texture->setResizeNonPowerOfTwoHint( false );
    
    osg::ref_ptr<osg::Geometry> quad =
        osg::createTexturedQuadGeometry(osg::Vec3(), osg::X_AXIS,-osg::Z_AXIS);
    quad->setEventCallback( handler.get() );
    quad->setCullCallback( handler.get() );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( quad.get() );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    //scene->addChild( osgDB::readNodeFile("cow.osg") );
    scene->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
	viewer.setCameraManipulator( new osgGA::TrackballManipulator );
	viewer.realize();
	
	while ( !viewer.done() )
	{
	    Berkelium::update();
        viewer.frame();
	}
    
	image = NULL;  // Destroy the web view
    Berkelium::destroy();
	return 0;
}
