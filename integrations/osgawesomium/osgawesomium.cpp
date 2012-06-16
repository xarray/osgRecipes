#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <Awesomium/awesomium_capi.h>
#include <Awesomium/KeyboardCodes.h>

class AwesomiumImage : public osg::Image
{
public:
    AwesomiumImage()
    : _webView(0), _lastButtonMask(0) {}
    
    AwesomiumImage( const AwesomiumImage& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY )
    : osg::Image(copy, op), _webView(copy._webView), _lastButtonMask(copy._lastButtonMask) {}
    
    META_Object( osg, AwesomiumImage )
    
    void loadURL( const std::string& url, int w=0, int h=0 )
    {
        if ( !_webView )
            _webView = awe_webcore_create_webview(w>0 ? w : 512, h>0 ? h : 512, false);
        else if ( w>0 && h>0 )
            awe_webview_resize( _webView, w, h, true, 300 );
        
        awe_string* aweURL = awe_string_create_from_ascii(url.c_str(), url.size());
        awe_webview_load_url( _webView, aweURL, awe_string_empty(),
                              awe_string_empty(), awe_string_empty() );
        awe_string_destroy( aweURL );
        
        awe_webview_focus( _webView );
    }
    
    virtual bool requiresUpdateCall() const { return true; }
    
    virtual void update( osg::NodeVisitor* nv )
    {
        if ( !_webView ) return;
        const awe_renderbuffer* buffer = awe_webview_render(_webView);
        setImage( awe_renderbuffer_get_width(buffer), awe_renderbuffer_get_height(buffer), 1,
                  4, GL_BGRA, GL_UNSIGNED_BYTE,
                  const_cast<unsigned char*>(awe_renderbuffer_get_buffer(buffer)),
                  osg::Image::NO_DELETE );
    }
    
    virtual bool sendPointerEvent( int x, int y, int buttonMask )
    {
        if ( !_webView ) return false;
        switch ( buttonMask )
        {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
            if ( !_lastButtonMask ) awe_webview_inject_mouse_down( _webView, AWE_MB_LEFT );
            else awe_webview_inject_mouse_move( _webView, x, y );
            break;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
            if ( !_lastButtonMask ) awe_webview_inject_mouse_down( _webView, AWE_MB_MIDDLE );
            else awe_webview_inject_mouse_move( _webView, x, y );
            break;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            if ( !_lastButtonMask ) awe_webview_inject_mouse_down( _webView, AWE_MB_RIGHT );
            else awe_webview_inject_mouse_move( _webView, x, y );
            break;
        default:
            awe_webview_inject_mouse_move( _webView, x, y );
            break;
        }
        
        if ( _lastButtonMask!=0 && buttonMask==0 )
        {
            switch ( _lastButtonMask )
            {
            case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
                awe_webview_inject_mouse_up( _webView, AWE_MB_LEFT );
                break;
            case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
                awe_webview_inject_mouse_up( _webView, AWE_MB_MIDDLE );
                break;
            case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
                awe_webview_inject_mouse_up( _webView, AWE_MB_RIGHT );
                break;
            }
        }
        _lastButtonMask = buttonMask;
        return true;
    }
    
    virtual bool sendKeyEvent( int key, bool keyDown )
    {
        awe_webkey_type type = keyDown ? AWE_WKT_KEYDOWN : AWE_WKT_KEYUP;
        if ( !_webView ) return false;
        
        switch ( key )
        {
        case osgGA::GUIEventAdapter::KEY_Tab:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_TAB );
            break;
        case osgGA::GUIEventAdapter::KEY_BackSpace:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_BACK );
            break;
        case osgGA::GUIEventAdapter::KEY_Return:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_RETURN );
            break;
        case osgGA::GUIEventAdapter::KEY_Left:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_LEFT );
            break;
        case osgGA::GUIEventAdapter::KEY_Right:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_RIGHT );
            break;
        case osgGA::GUIEventAdapter::KEY_Up:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_UP );
            break;
        case osgGA::GUIEventAdapter::KEY_Down:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_DOWN );
            break;
        case osgGA::GUIEventAdapter::KEY_Page_Down:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_NEXT );
            break;
        case osgGA::GUIEventAdapter::KEY_Page_Up:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_PRIOR );
            break;
        case osgGA::GUIEventAdapter::KEY_Home:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_HOME );
            break;
        case osgGA::GUIEventAdapter::KEY_End:
            injectAwesomiumKey( type, Awesomium::KeyCodes::AK_END );
            break;
        default:
            if ( keyDown ) injectAwesomiumKey( AWE_WKT_CHAR, key );
            break;
        }
        return true;
    }
    
protected:
    virtual ~AwesomiumImage()
    {
        if ( _webView )
            awe_webview_destroy( _webView );
    }
    
    void injectAwesomiumKey( awe_webkey_type type, int key )
    {
        awe_webkeyboardevent keyEvent;
        keyEvent.type = type;
        keyEvent.virtual_key_code = key;
        keyEvent.native_key_code = key;
        keyEvent.text[0] = (char)key; keyEvent.text[1] = '\0';
        keyEvent.unmodified_text[0] = (char)key; keyEvent.unmodified_text[1] = '\0';
        keyEvent.modifiers = 0;
        keyEvent.is_system_key = false;
        awe_webview_inject_keyboard_event( _webView, keyEvent );
    }
    
    awe_webview* _webView;
    int _lastButtonMask;
};

int main( int argc, char** argv )
{
    awe_webcore_initialize_default();
    
    osg::ref_ptr<AwesomiumImage> image = new AwesomiumImage;
    image->loadURL( "http://www.openscenegraph.com", 800, 600 );
    
    osg::ref_ptr<osgViewer::InteractiveImageHandler> handler =
        new osgViewer::InteractiveImageHandler( image.get() );
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
	viewer.setCameraManipulator( new osgGA::TrackballManipulator );
	viewer.realize();
	
	while ( !viewer.done() )
	{
	    awe_webcore_update();
	    viewer.frame();
	}
	
	image = NULL;  // Destroy the web view
	//awe_webcore_shutdown();
	return 0;
}
