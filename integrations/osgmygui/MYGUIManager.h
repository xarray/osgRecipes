#ifndef H_MYGUIDRAWABLE
#define H_MYGUIDRAWABLE

#include <MYGUI/MyGUI.h>
#include <MYGUI/MyGUI_OpenGLPlatform.h>
#include <osg/Camera>
#include <osg/Drawable>
#include <osgGA/GUIEventHandler>
#include <queue>

class MYGUIManager;

class MYGUIHandler : public osgGA::GUIEventHandler
{
public:
    MYGUIHandler( osg::Camera* c, MYGUIManager* m ) : _camera(c), _manager(m) {}
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
    
protected:
    osg::observer_ptr<osg::Camera> _camera;
    MYGUIManager* _manager;
};

class MYGUIManager : public osg::Drawable, public MyGUI::OpenGLImageLoader
{
public:
    MYGUIManager();
    MYGUIManager( const MYGUIManager& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, MYGUIManager )
    
    void setResourcePathFile( const std::string& file ) { _resourcePathFile = file; }
    const std::string& getResourcePathFile() const { return _resourcePathFile; }
    
    void setResourceCoreFile( const std::string& file ) { _resourceCoreFile = file; }
    const std::string& getResourceCoreFile() const { return _resourceCoreFile; }
    
    void pushEvent( const osgGA::GUIEventAdapter* ea )
    { _eventsToHandle.push( ea ); }
    
    // image loader methods
    virtual void* loadImage( int& width, int& height, MyGUI::PixelFormat& format, const std::string& filename );
    virtual void saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const std::string& filename );
    
    // drawable methods
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
    virtual void releaseGLObjects( osg::State* state=0 ) const;
    
protected:
    virtual ~MYGUIManager() {}
    
    virtual void updateEvents() const;
    virtual void setupResources();
    virtual void initializeControls() {}
    
    MyGUI::MouseButton convertMouseButton( int button ) const;
    MyGUI::KeyCode convertKeyCode( int key ) const;
    
    std::queue< osg::ref_ptr<const osgGA::GUIEventAdapter> > _eventsToHandle;
    MyGUI::Gui* _gui;
    MyGUI::OpenGLPlatform* _platform;
    std::string _resourcePathFile;
    std::string _resourceCoreFile;
    std::string _rootMedia;
    unsigned int _activeContextID;
    bool _initialized;
};

#endif
