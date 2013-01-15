#include "GuichanWrapper.h"
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgGA/EventVisitor>

gcn::Image* GuichanImageLoader::load( const std::string& filename, bool convertToDisplayFormat )
{
    osg::Image* osgImage = osgDB::readImageFile( filename );
    if ( !osgImage )
    {
        throw GCN_EXCEPTION( std::string("Unable to load ") + filename );
    }
    
    unsigned int s = osgImage->s(), t = osgImage->t();
    unsigned int* pixels = new unsigned int[s * t];
    for ( unsigned int y=0; y<t; ++y )
    {
        for ( unsigned int x=0; x<s; ++x )
        {
            unsigned int c = 0;
            osg::Vec4 color = osgImage->getColor(x, t-y-1);
#ifdef GUICHAN_BIG_ENDIAN
            // RGBA
            c = (unsigned int)(color[0] * 255.0); c <<= 8;
            c += (unsigned int)(color[1] * 255.0); c <<= 8;
            c += (unsigned int)(color[2] * 255.0); c <<= 8;
            c += (unsigned int)(color[3] * 255.0);
#else
            // AGBR
            c = (unsigned int)(color[3] * 255.0); c <<= 8;
            c += (unsigned int)(color[2] * 255.0); c <<= 8;
            c += (unsigned int)(color[1] * 255.0); c <<= 8;
            c += (unsigned int)(color[0] * 255.0);
#endif
            pixels[x + y * s] = c;
        }
    }
    
    gcn::OpenGLImage* image = new gcn::OpenGLImage( pixels, s, t, convertToDisplayFormat );
    delete[] pixels;
    return image;
}

void GuichanEventInput::pushInput( const osgGA::GUIEventAdapter& ea )
{
    float x = ea.getX(), y = ea.getY();
    if ( ea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS )
        y = ea.getWindowHeight() - y;
    
    gcn::KeyInput keyInput;
    gcn::MouseInput mouseInput;
    switch ( ea.getEventType() )
    {
    case osgGA::GUIEventAdapter::PUSH:
        mouseInput.setX( x );
        mouseInput.setY( y );
        mouseInput.setType( gcn::MouseInput::Pressed );
        mouseInput.setButton( convertMouseButton(ea.getButton()) );
        mouseInput.setTimeStamp( ea.getTime() );
        _mouseInputQueue.push( mouseInput );
        break;
    case osgGA::GUIEventAdapter::RELEASE:
        mouseInput.setX( x );
        mouseInput.setY( y );
        mouseInput.setType( gcn::MouseInput::Released );
        mouseInput.setButton( convertMouseButton(ea.getButton()) );
        mouseInput.setTimeStamp( ea.getTime() );
        _mouseInputQueue.push( mouseInput );
        break;
    case osgGA::GUIEventAdapter::MOVE:
        mouseInput.setX( x );
        mouseInput.setY( y );
        mouseInput.setType( gcn::MouseInput::Moved );
        mouseInput.setButton( gcn::MouseInput::Empty );
        mouseInput.setTimeStamp( ea.getTime() );
        _mouseInputQueue.push( mouseInput );
        break;
    case osgGA::GUIEventAdapter::SCROLL:
        mouseInput.setX( x );
        mouseInput.setY( y );
        if ( ea.getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP )
            mouseInput.setType( gcn::MouseInput::WheelMovedUp );
        else if ( ea.getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN )
            mouseInput.setType( gcn::MouseInput::WheelMovedDown );
        mouseInput.setButton( convertMouseButton(ea.getButton()) );
        mouseInput.setTimeStamp( ea.getTime() );
        _mouseInputQueue.push( mouseInput );
        break;
    case osgGA::GUIEventAdapter::KEYDOWN:
        convertKeyValue( keyInput, ea.getKey() );
        convertModKeyValue( keyInput, ea.getModKeyMask() );
        keyInput.setType( gcn::KeyInput::Pressed );
        _keyInputQueue.push( keyInput );
        break;
    case osgGA::GUIEventAdapter::KEYUP:
        convertKeyValue( keyInput, ea.getKey() );
        convertModKeyValue( keyInput, ea.getModKeyMask() );
        keyInput.setType( gcn::KeyInput::Released );
        _keyInputQueue.push( keyInput );
        break;
    case osgGA::GUIEventAdapter::FRAME:
        if ( _guichan->isValid() )
            _guichan->getGUIElement()->logic();
        break;
    default: break;
    }
}

gcn::KeyInput GuichanEventInput::dequeueKeyInput()
{
    if ( _keyInputQueue.empty() )
    {
        throw GCN_EXCEPTION("The key queue is empty.");
    }
    gcn::KeyInput keyInput = _keyInputQueue.front();
    _keyInputQueue.pop();
    return keyInput;
}

gcn::MouseInput GuichanEventInput::dequeueMouseInput()
{
    if ( _mouseInputQueue.empty() )
    {
        throw GCN_EXCEPTION("The mouse queue is empty.");
    }
    gcn::MouseInput mouseInput = _mouseInputQueue.front();
    _mouseInputQueue.pop();
    return mouseInput;
}

int GuichanEventInput::convertMouseButton( int button )
{
    switch ( button )
    {
    case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
        return gcn::MouseInput::Left;
    case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
        return gcn::MouseInput::Middle;
    case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
        return gcn::MouseInput::Right;
    default: break;
    }
    return button;
}

void GuichanEventInput::convertKeyValue( gcn::KeyInput& keyInput, int key )
{
    int result = key;
    switch ( key )
    {
    case osgGA::GUIEventAdapter::KEY_Space: result = gcn::Key::Space; break;
    case osgGA::GUIEventAdapter::KEY_Tab: result = gcn::Key::Tab; break;
    case osgGA::GUIEventAdapter::KEY_Alt_L: result = gcn::Key::LeftAlt; break;
    case osgGA::GUIEventAdapter::KEY_Alt_R: result = gcn::Key::RightAlt; break;
    case osgGA::GUIEventAdapter::KEY_Shift_L: result = gcn::Key::LeftShift; break;
    case osgGA::GUIEventAdapter::KEY_Shift_R: result = gcn::Key::RightShift; break;
    case osgGA::GUIEventAdapter::KEY_Control_L: result = gcn::Key::LeftControl; break;
    case osgGA::GUIEventAdapter::KEY_Control_R: result = gcn::Key::RightControl; break;
    case osgGA::GUIEventAdapter::KEY_Meta_L: result = gcn::Key::LeftMeta; break;
    case osgGA::GUIEventAdapter::KEY_Meta_R: result = gcn::Key::RightMeta; break;
    case osgGA::GUIEventAdapter::KEY_Super_L: result = gcn::Key::LeftSuper; break;
    case osgGA::GUIEventAdapter::KEY_Super_R: result = gcn::Key::RightSuper; break;
    case osgGA::GUIEventAdapter::KEY_Insert: result = gcn::Key::Insert; break;
    case osgGA::GUIEventAdapter::KEY_Home: result = gcn::Key::Home; break;
    case osgGA::GUIEventAdapter::KEY_Page_Up: result = gcn::Key::PageUp; break;
    case osgGA::GUIEventAdapter::KEY_Delete: result = gcn::Key::Delete; break;
    case osgGA::GUIEventAdapter::KEY_End: result = gcn::Key::End; break;
    case osgGA::GUIEventAdapter::KEY_Page_Down: result = gcn::Key::PageDown; break;
    case osgGA::GUIEventAdapter::KEY_Escape: result = gcn::Key::Escape; break;
    case osgGA::GUIEventAdapter::KEY_Caps_Lock: result = gcn::Key::CapsLock; break;
    case osgGA::GUIEventAdapter::KEY_BackSpace: result = gcn::Key::Backspace; break;
    case osgGA::GUIEventAdapter::KEY_F1: result = gcn::Key::F1; break;
    case osgGA::GUIEventAdapter::KEY_F2: result = gcn::Key::F2; break;
    case osgGA::GUIEventAdapter::KEY_F3: result = gcn::Key::F3; break;
    case osgGA::GUIEventAdapter::KEY_F4: result = gcn::Key::F4; break;
    case osgGA::GUIEventAdapter::KEY_F5: result = gcn::Key::F5; break;
    case osgGA::GUIEventAdapter::KEY_F6: result = gcn::Key::F6; break;
    case osgGA::GUIEventAdapter::KEY_F7: result = gcn::Key::F7; break;
    case osgGA::GUIEventAdapter::KEY_F8: result = gcn::Key::F8; break;
    case osgGA::GUIEventAdapter::KEY_F9: result = gcn::Key::F9; break;
    case osgGA::GUIEventAdapter::KEY_F10: result = gcn::Key::F10; break;
    case osgGA::GUIEventAdapter::KEY_F11: result = gcn::Key::F11; break;
    case osgGA::GUIEventAdapter::KEY_F12: result = gcn::Key::F12; break;
    case osgGA::GUIEventAdapter::KEY_F13: result = gcn::Key::F13; break;
    case osgGA::GUIEventAdapter::KEY_F14: result = gcn::Key::F14; break;
    case osgGA::GUIEventAdapter::KEY_F15: result = gcn::Key::F15; break;
    case osgGA::GUIEventAdapter::KEY_Print: result = gcn::Key::PrintScreen; break;
    case osgGA::GUIEventAdapter::KEY_Scroll_Lock: result = gcn::Key::ScrollLock; break;
    case osgGA::GUIEventAdapter::KEY_Pause: result = gcn::Key::Pause; break;
    case osgGA::GUIEventAdapter::KEY_Mode_switch: result = gcn::Key::AltGr; break;
    case osgGA::GUIEventAdapter::KEY_Left: result = gcn::Key::Left; break;
    case osgGA::GUIEventAdapter::KEY_Right: result = gcn::Key::Right; break;
    case osgGA::GUIEventAdapter::KEY_Up: result = gcn::Key::Up; break;
    case osgGA::GUIEventAdapter::KEY_Down: result = gcn::Key::Down; break;
    case osgGA::GUIEventAdapter::KEY_Return:
    case osgGA::GUIEventAdapter::KEY_KP_Enter:
        result = gcn::Key::Enter;
        break;
    case osgGA::GUIEventAdapter::KEY_Num_Lock:
        result = gcn::Key::NumLock;
        keyInput.setNumericPad( true );
        break;
    default: break;
    }
    keyInput.setKey( gcn::Key(result) );
}

void GuichanEventInput::convertModKeyValue( gcn::KeyInput& keyInput, int modkey )
{
    keyInput.setShiftPressed( (modkey&osgGA::GUIEventAdapter::MODKEY_SHIFT)!=0 );
    keyInput.setControlPressed( (modkey&osgGA::GUIEventAdapter::MODKEY_CTRL)!=0 );
    keyInput.setAltPressed( (modkey&osgGA::GUIEventAdapter::MODKEY_ALT)!=0 );
    keyInput.setMetaPressed( (modkey&osgGA::GUIEventAdapter::MODKEY_META)!=0 );
}

GuichanDrawable::GuichanDrawable()
:   _activeContextID(0), _initialized(false)
{
    _container = new gcn::Container;
    _container->setOpaque( false );
    
    _graphics = new gcn::OpenGLGraphics;
    _imageLoader = new GuichanImageLoader;
    _input = new GuichanEventInput(this);
    
    _gui = new gcn::Gui;
    _gui->setTop( _container );
    _gui->setGraphics( _graphics );
    _gui->setInput( _input );
    gcn::Image::setImageLoader( _imageLoader );
    
    setSupportsDisplayList( false );
    getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
}

GuichanDrawable::GuichanDrawable( const GuichanDrawable& copy, const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop),
    _input(copy._input), _gui(copy._gui), _container(copy._container),
    _graphics(copy._graphics), _imageLoader(copy._imageLoader),
    _activeContextID(copy._activeContextID), _initialized(copy._initialized)
{
}

GuichanDrawable::~GuichanDrawable()
{
    delete _input;
    delete _gui;
    delete _container;
    delete _graphics;
    delete _imageLoader;
}

void GuichanDrawable::setContainerSize( int x, int y, int width, int height )
{
    _container->setDimension( gcn::Rectangle(x, y, width, height) );
    static_cast<gcn::OpenGLGraphics*>(_graphics)->setTargetPlane( width, height );
}

void GuichanDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    if ( !_initialized )
    {
        std::string fixedFontFile = osgDB::findDataFile("fixedfont.bmp");
        if ( !fixedFontFile.empty() )
        {
            // FIXME: naive font type here
            gcn::Font* font = new gcn::ImageFont(
                fixedFontFile.c_str(), " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" );
            gcn::Widget::setGlobalFont( font );
            
            _activeContextID = contextID;
            _initialized = true;
        }
    }
    else if ( contextID==_activeContextID )
    {
        osg::State* state = renderInfo.getState();
        state->disableAllVertexArrays();
        state->disableTexCoordPointer( 0 );
        
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        _gui->draw();
        glPopAttrib();
    }
    else
        std::cout << "Multiple contexts are not supported at present!" << std::endl;
}
