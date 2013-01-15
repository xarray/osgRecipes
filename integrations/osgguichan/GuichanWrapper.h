#ifndef H_GUICHAN_WRAPPER
#define H_GUICHAN_WRAPPER

#include <guichan.hpp>
#include <guichan/exception.hpp>
#include <guichan/input.hpp>
#include <guichan/keyinput.hpp>
#include <guichan/mouseinput.hpp>
#include <guichan/platform.hpp>
#include <guichan/opengl.hpp>
#include <guichan/opengl/openglimage.hpp>

#include <osg/Camera>
#include <osg/Drawable>
#include <osgGA/GUIEventHandler>
#include <queue>

class GuichanDrawable;

class GuichanImageLoader : public gcn::ImageLoader
{
public:
    virtual gcn::Image* load( const std::string& filename, bool convertToDisplayFormat=true );
};

class GuichanEventInput : public gcn::Input
{
public:
    virtual void pushInput( const osgGA::GUIEventAdapter& ea );
    virtual void _pollInput() {}
    
    virtual bool isKeyQueueEmpty() { return _keyInputQueue.empty(); }
    virtual gcn::KeyInput dequeueKeyInput();
    
    virtual bool isMouseQueueEmpty() { return _mouseInputQueue.empty(); }
    virtual gcn::MouseInput dequeueMouseInput();
    
    GuichanEventInput( GuichanDrawable* g=NULL ) : _guichan(g) {}
    
protected:
    int convertMouseButton( int button );
    void convertKeyValue( gcn::KeyInput&, int key );
    void convertModKeyValue( gcn::KeyInput&, int modkey );
    
    GuichanDrawable* _guichan;
    std::queue<gcn::KeyInput> _keyInputQueue;
    std::queue<gcn::MouseInput> _mouseInputQueue;
};

class GuichanDrawable : public osg::Drawable
{
public:
    GuichanDrawable();
    GuichanDrawable( const GuichanDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osgGuichanGUI, GuichanDrawable )
    
    void addWidget( gcn::Widget* widget ) { _container->add( widget ); }
    void addWidget( gcn::Widget* widget, int x, int y ) { _container->add( widget, x, y ); }
    
    void setContainerSize( int x, int y, int width, int height );
    void pushInput( const osgGA::GUIEventAdapter& ea ) { _input->pushInput(ea); }
    bool isValid() const { return _initialized; }
    
    gcn::Gui* getGUIElement() { return _gui; }
    const gcn::Gui* getGUIElement() const { return _gui; }
    
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
    
protected:
    virtual ~GuichanDrawable();
    
    GuichanEventInput* _input;
    gcn::Gui* _gui;
    gcn::Container* _container;
    gcn::Graphics* _graphics;
    gcn::ImageLoader* _imageLoader;
    mutable unsigned int _activeContextID;
    mutable bool _initialized;
};

#endif
