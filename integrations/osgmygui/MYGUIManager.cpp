#include "MYGUIManager.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

bool MYGUIHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    int width = ea.getWindowWidth(), height = ea.getWindowHeight();
    switch ( ea.getEventType() )
    {
    case osgGA::GUIEventAdapter::RESIZE:
        if ( _camera.valid() )
        {
            _camera->setProjectionMatrix( osg::Matrixd::ortho2D(0.0, width, 0.0, height) );
            _camera->setViewport( 0.0, 0.0, width, height );
        }
        break;
    default:
        break;
    }
    
    // As MyGUI handle all events within the OpenGL context, we have to record the event here
    // and process it later in the draw implementation
    if ( ea.getEventType()!=osgGA::GUIEventAdapter::FRAME )
        _manager->pushEvent( &ea );
    return false;
}

MYGUIManager::MYGUIManager()
:   _gui(0), _platform(0),
    _resourcePathFile("resources.xml"), _resourceCoreFile("MyGUI_Core.xml"),
    _activeContextID(0), _initialized(false)
{
    setSupportsDisplayList( false );
    getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
}

MYGUIManager::MYGUIManager( const MYGUIManager& copy,const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop), _eventsToHandle(copy._eventsToHandle),
    _gui(copy._gui), _platform(copy._platform),
    _resourcePathFile(copy._resourcePathFile),
    _resourceCoreFile(copy._resourceCoreFile),
    _rootMedia(copy._rootMedia),
    _activeContextID(copy._activeContextID),
    _initialized(copy._initialized)
{}

void* MYGUIManager::loadImage( int& width, int& height, MyGUI::PixelFormat& format, const std::string& filename )
{
    std::string fullname = MyGUI::OpenGLDataManager::getInstance().getDataPath( filename );
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( fullname );
    void* result = NULL;
    if ( image.valid() )
    {
        width = image->s();
        height = image->t();
        if ( image->getDataType()!=GL_UNSIGNED_BYTE || image->getPacking()!=1 )
        {
            format = MyGUI::PixelFormat::Unknow;
            return result;
        }
        
        unsigned int num = 0;
        switch ( image->getPixelFormat() )
        {
        case GL_LUMINANCE: case GL_ALPHA: format = MyGUI::PixelFormat::L8; num = 1; break;
        case GL_LUMINANCE_ALPHA: format = MyGUI::PixelFormat::L8A8; num = 2; break;
        case GL_RGB: format = MyGUI::PixelFormat::R8G8B8; num = 3; break;
        case GL_RGBA: format = MyGUI::PixelFormat::R8G8B8A8; num = 4; break;
        default: format = MyGUI::PixelFormat::Unknow; return result;
        }
        
        unsigned int size = width * height * num;
        unsigned char* dest = new unsigned char[size];
        image->flipVertical();
        if ( image->getPixelFormat()==GL_RGB || image->getPixelFormat()==GL_RGBA )
        {
            // FIXME: I don't an additional conversion here but...
            // MyGUI will automatically consider it as BGR so I should do such stupid thing
            unsigned int step = (image->getPixelFormat()==GL_RGB ? 3 : 4);
            unsigned char* src = image->data();
            for ( unsigned int i=0; i<size; i+=step )
            {
                dest[i+0] = src[i+2];
                dest[i+1] = src[i+1];
                dest[i+2] = src[i+0];
                if ( step==4 ) dest[i+3] = src[i+3];
            }
        }
        else
            memcpy( dest, image->data(), size );
        result = dest;
    }
    return result;
}

void MYGUIManager::saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const std::string& filename )
{
    GLenum pixelFormat = 0;
    unsigned int internalFormat = 0;
    switch ( format.getValue() )
    {
    case MyGUI::PixelFormat::L8: pixelFormat = GL_ALPHA; internalFormat = 1; break;
    case MyGUI::PixelFormat::L8A8: pixelFormat = GL_LUMINANCE_ALPHA; internalFormat = 2; break;
    case MyGUI::PixelFormat::R8G8B8: pixelFormat = GL_BGR; internalFormat = 3; break;
    case MyGUI::PixelFormat::R8G8B8A8: pixelFormat = GL_BGRA; internalFormat = 4; break;
    default: return;
    }
    
    unsigned int size = width * height * internalFormat;
    unsigned char* imageData = new unsigned char[size];
    memcpy( imageData, texture, size );
    
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->setImage( width, height, 1, internalFormat, pixelFormat, GL_UNSIGNED_BYTE,
        static_cast<unsigned char*>(imageData), osg::Image::USE_NEW_DELETE );
    image->flipVertical();
    osgDB::writeImageFile( *image, filename );
}

void MYGUIManager::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    if ( !_initialized )
    {
        MYGUIManager* constMe = const_cast<MYGUIManager*>(this);
        constMe->_platform = new MyGUI::OpenGLPlatform;
        constMe->_platform->initialise( constMe );
        constMe->setupResources();
        
        constMe->_gui = new MyGUI::Gui;
        constMe->_gui->initialise( _resourceCoreFile );
        constMe->initializeControls();
        
        constMe->_activeContextID = contextID;
        constMe->_initialized = true;
    }
    else if ( contextID==_activeContextID )
    {
        osg::State* state = renderInfo.getState();
        state->disableAllVertexArrays();
        state->disableTexCoordPointer( 0 );
        
        glPushMatrix();
        glPushAttrib( GL_ALL_ATTRIB_BITS );
		if ( _platform )
		{
		    updateEvents();
		    _platform->getRenderManagerPtr()->drawOneFrame();
        }
        glPopAttrib();
        glPopMatrix();
    }
}

void MYGUIManager::releaseGLObjects( osg::State* state ) const
{
    if ( state && state->getGraphicsContext() )
    {
        osg::GraphicsContext* gc = state->getGraphicsContext();
        if ( gc->makeCurrent() )
        {
            MYGUIManager* constMe = const_cast<MYGUIManager*>(this);
            if ( constMe->_gui )
            {
                constMe->_gui->shutdown();
                delete constMe->_gui;
                constMe->_gui = nullptr;
            }
            if ( constMe->_platform )
            {
                constMe->_platform->shutdown();
                delete constMe->_platform;
                constMe->_platform = nullptr;
            }
            gc->releaseContext();
        }
    }
}

void MYGUIManager::updateEvents() const
{
    unsigned int size = _eventsToHandle.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgGA::GUIEventAdapter& ea = *(_eventsToHandle.front());
        int x = ea.getX(), y = ea.getY(), key = ea.getKey();
        if ( ea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS )
            y = ea.getWindowHeight() - y;
        
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::PUSH:
            MyGUI::InputManager::getInstance().injectMousePress( x, y, convertMouseButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            MyGUI::InputManager::getInstance().injectMouseRelease( x, y, convertMouseButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
            MyGUI::InputManager::getInstance().injectMouseMove( x, y, 0 );
            break;
        case osgGA::GUIEventAdapter::KEYDOWN:
            if ( key<127 )
                MyGUI::InputManager::getInstance().injectKeyPress( convertKeyCode(key), (char)key );
            else
                MyGUI::InputManager::getInstance().injectKeyPress( convertKeyCode(key) );
            break;
        case osgGA::GUIEventAdapter::KEYUP:
            MyGUI::InputManager::getInstance().injectKeyRelease( convertKeyCode(key) );
            break;
        case osgGA::GUIEventAdapter::RESIZE:
            _platform->getRenderManagerPtr()->setViewSize( ea.getWindowWidth(), ea.getWindowHeight() );
            break;
        default:
            break;
        }
        const_cast<MYGUIManager*>(this)->_eventsToHandle.pop();
    }
}

void MYGUIManager::setupResources()
{
    MyGUI::xml::Document doc;
    if ( !_platform || !doc.open(_resourcePathFile) ) doc.getLastError();
    
    MyGUI::xml::ElementPtr root = doc.getRoot();
    if ( root==nullptr || root->getName()!="Paths" ) return;
    
    MyGUI::xml::ElementEnumerator node = root->getElementEnumerator();
    while ( node.next() )
    {
        if ( node->getName()=="Path" )
        {
            bool root = false;
            if ( node->findAttribute("root")!="" )
            {
                root = MyGUI::utility::parseBool( node->findAttribute("root") );
                if ( root ) _rootMedia = node->getContent();
            }
            _platform->getDataManagerPtr()->addResourceLocation( node->getContent(), false );
        }
    }
    _platform->getDataManagerPtr()->addResourceLocation( _rootMedia + "/Common/Base", false );
}

MyGUI::MouseButton MYGUIManager::convertMouseButton( int button ) const
{
    switch ( button )
    {
    case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
        return MyGUI::MouseButton::Left;
    case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
        return MyGUI::MouseButton::Middle;
    case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
        return MyGUI::MouseButton::Right;
    default: break;
    }
    return MyGUI::MouseButton::None;
}

MyGUI::KeyCode MYGUIManager::convertKeyCode( int key ) const
{
    static std::map<int, MyGUI::KeyCode> s_keyCodeMap;
    if ( !s_keyCodeMap.size() )
    {
        #define ADD_CHAR_PAIR(c, k) s_keyCodeMap[c] = MyGUI::KeyCode::##k
        #define ADD_KEY_PAIR(k) s_keyCodeMap[osgGA::GUIEventAdapter::KEY_##k] = MyGUI::KeyCode::##k
        #define ADD_KEY_PAIR2(k1, k2) s_keyCodeMap[osgGA::GUIEventAdapter::KEY_##k1] = MyGUI::KeyCode::##k2
        
        ADD_CHAR_PAIR('1', One); ADD_CHAR_PAIR('2', Two); ADD_CHAR_PAIR('3', Three); ADD_CHAR_PAIR('4', Four);
        ADD_CHAR_PAIR('5', Five); ADD_CHAR_PAIR('6', Six); ADD_CHAR_PAIR('7', Seven); ADD_CHAR_PAIR('8', Eight);
        ADD_CHAR_PAIR('9', Nine); ADD_CHAR_PAIR('0', Zero);
        ADD_CHAR_PAIR('a', A); ADD_CHAR_PAIR('b', B); ADD_CHAR_PAIR('c', C); ADD_CHAR_PAIR('d', D);
        ADD_CHAR_PAIR('e', E); ADD_CHAR_PAIR('f', F); ADD_CHAR_PAIR('g', G); ADD_CHAR_PAIR('h', H);
        ADD_CHAR_PAIR('i', I); ADD_CHAR_PAIR('j', J); ADD_CHAR_PAIR('k', K); ADD_CHAR_PAIR('l', L);
        ADD_CHAR_PAIR('m', M); ADD_CHAR_PAIR('n', N); ADD_CHAR_PAIR('o', O); ADD_CHAR_PAIR('p', P);
        ADD_CHAR_PAIR('q', Q); ADD_CHAR_PAIR('r', R); ADD_CHAR_PAIR('S', S); ADD_CHAR_PAIR('t', T);
        ADD_CHAR_PAIR('u', U); ADD_CHAR_PAIR('v', V); ADD_CHAR_PAIR('w', W); ADD_CHAR_PAIR('x', X);
        ADD_CHAR_PAIR('y', Y); ADD_CHAR_PAIR('z', Z);
        
        ADD_KEY_PAIR(F1); ADD_KEY_PAIR(F2); ADD_KEY_PAIR(F3); ADD_KEY_PAIR(F4); ADD_KEY_PAIR(F5);
        ADD_KEY_PAIR(F6); ADD_KEY_PAIR(F7); ADD_KEY_PAIR(F8); ADD_KEY_PAIR(F9); ADD_KEY_PAIR(F10);
        ADD_KEY_PAIR(Escape); ADD_KEY_PAIR(Tab); ADD_KEY_PAIR(Return); ADD_KEY_PAIR(Space);
        ADD_KEY_PAIR(Minus); ADD_KEY_PAIR(Equals); ADD_KEY_PAIR(Backslash); ADD_KEY_PAIR(Slash);
        ADD_KEY_PAIR(Semicolon); ADD_KEY_PAIR(Equals); ADD_KEY_PAIR(Comma); ADD_KEY_PAIR(Period);
        ADD_KEY_PAIR(Insert); ADD_KEY_PAIR(Delete); ADD_KEY_PAIR(Home); ADD_KEY_PAIR(End);
        
        ADD_KEY_PAIR2(Num_Lock, NumLock); ADD_KEY_PAIR2(Scroll_Lock, ScrollLock); ADD_KEY_PAIR2(Caps_Lock, Capital);
        ADD_KEY_PAIR2(BackSpace, Backspace); ADD_KEY_PAIR2(Page_Down, PageDown); ADD_KEY_PAIR2(Page_Up, PageUp);
        ADD_KEY_PAIR2(Leftbracket, LeftBracket); ADD_KEY_PAIR2(Rightbracket, RightBracket); ADD_KEY_PAIR2(Quotedbl, Apostrophe);
        ADD_KEY_PAIR2(Left, ArrowLeft); ADD_KEY_PAIR2(Right, ArrowRight);
        ADD_KEY_PAIR2(Up, ArrowUp); ADD_KEY_PAIR2(Down, ArrowDown);
        ADD_KEY_PAIR2(KP_1, Numpad1); ADD_KEY_PAIR2(KP_2, Numpad2); ADD_KEY_PAIR2(KP_3, Numpad3);
        ADD_KEY_PAIR2(KP_4, Numpad4); ADD_KEY_PAIR2(KP_5, Numpad5); ADD_KEY_PAIR2(KP_6, Numpad6);
        ADD_KEY_PAIR2(KP_7, Numpad7); ADD_KEY_PAIR2(KP_8, Numpad8); ADD_KEY_PAIR2(KP_9, Numpad9);
        ADD_KEY_PAIR2(KP_0, Numpad0); ADD_KEY_PAIR2(KP_Enter, NumpadEnter);
        ADD_KEY_PAIR2(Control_L, LeftControl); ADD_KEY_PAIR2(Control_R, RightControl);
        ADD_KEY_PAIR2(Alt_L, LeftAlt); ADD_KEY_PAIR2(Alt_R, RightAlt);
        ADD_KEY_PAIR2(Shift_L, LeftShift); ADD_KEY_PAIR2(Shift_R, RightShift);
    }
    
    std::map<int, MyGUI::KeyCode>::iterator itr = s_keyCodeMap.find(key);
    if ( itr!=s_keyCodeMap.end() ) return itr->second;
    return MyGUI::KeyCode::None;
}
