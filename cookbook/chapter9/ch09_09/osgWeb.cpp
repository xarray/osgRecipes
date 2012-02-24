/**********************************************************\

  Auto-generated osgWeb.cpp

  This file contains the auto-generated main plugin object
  implementation for the osgWeb project

\**********************************************************/

#include "osgWebAPI.h"

#include "osgWeb.h"

///////////////////////////////////////////////////////////////////////////////
/// @fn osgWeb::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginInitialize()
///
/// @see FB::FactoryBase::globalPluginInitialize
///////////////////////////////////////////////////////////////////////////////
void osgWeb::StaticInitialize()
{
    // Place one-time initialization stuff here; As of FireBreath 1.4 this should only
    // be called once per process
}

///////////////////////////////////////////////////////////////////////////////
/// @fn osgWeb::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginDeinitialize()
///
/// @see FB::FactoryBase::globalPluginDeinitialize
///////////////////////////////////////////////////////////////////////////////
void osgWeb::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here. As of FireBreath 1.4 this should
    // always be called just before the plugin library is unloaded
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  osgWeb constructor.  Note that your API is not available
///         at this point, nor the window.  For best results wait to use
///         the JSAPI object until the onPluginReady method is called
///////////////////////////////////////////////////////////////////////////////
osgWeb::osgWeb()
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  osgWeb destructor.
///////////////////////////////////////////////////////////////////////////////
osgWeb::~osgWeb()
{
    // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
    // root object) and tell the host to free the retained JSAPI objects then
    // unless you are holding another shared_ptr reference to your JSAPI object
    // they will be released here.
    releaseRootJSAPI();
    m_host->freeRetainedObjects();
}

void osgWeb::onPluginReady()
{
    // When this is called, the BrowserHost is attached, the JSAPI object is
    // created, and we are ready to interact with the page and such.  The
    // PluginWindow may or may not have already fire the AttachedEvent at
    // this point.
}

void osgWeb::shutdown()
{
    // This will be called when it is time for the plugin to shut down;
    // any threads or anything else that may hold a shared_ptr to this
    // object should be released here so that this object can be safely
    // destroyed. This is the last point that shared_from_this and weak_ptr
    // references to this object will be valid
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an instance of the JSAPI object that provides your main
///         Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<osgWeb> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
///////////////////////////////////////////////////////////////////////////////
FB::JSAPIPtr osgWeb::createJSAPI()
{
    // m_host is the BrowserHost
    return boost::make_shared<osgWebAPI>(FB::ptr_cast<osgWeb>(shared_from_this()), m_host);
}

bool osgWeb::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool osgWeb::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    /*osg::Group* root = dynamic_cast<osg::Group*>( _viewer.getSceneData() );
	if ( root )
	{
		root->removeChildren( 0, root->getNumChildren() );
	    switch ( evt->m_Btn )
		{
		case FB::MouseUpEvent::MouseButton_Left:
			root->addChild( osgDB::readNodeFile("cow.osg") ); break;
		case FB::MouseUpEvent::MouseButton_Middle:
			root->addChild( osgDB::readNodeFile("cessna.osg") ); break;
		case FB::MouseUpEvent::MouseButton_Right:
			root->addChild( osgDB::readNodeFile("dumptruck.osg") ); break;
		}
	}*/
	if ( _commandHandler.valid() )
	{
		switch ( evt->m_Btn )
		{
		case FB::MouseUpEvent::MouseButton_Left:
			_commandHandler->addCommand("cow.osg"); break;
		case FB::MouseUpEvent::MouseButton_Middle:
			_commandHandler->addCommand("cessna.osg"); break;
		case FB::MouseUpEvent::MouseButton_Right:
			_commandHandler->addCommand("dumptruck.osg"); break;
		}
	}
    return false;
}

bool osgWeb::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}
bool osgWeb::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *win)
{
    FB::PluginWindowWin* window = reinterpret_cast<FB::PluginWindowWin*>(win);
    if ( window )
    {
        osg::ref_ptr<osg::Referenced> windata =
            new osgViewer::GraphicsWindowWin32::WindowData( window->getHWND() );
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->x = 0;
        traits->y = 0;
        traits->width = 800;
        traits->height = 600;
        traits->windowDecoration = false;
        traits->doubleBuffer = true;
        traits->inheritedWindowData = windata;
        
        osg::ref_ptr<osg::GraphicsContext> gc =
            osg::GraphicsContext::createGraphicsContext( traits.get() );
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext( gc );
        camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
        camera->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::Vec4f(0.2f, 0.2f, 0.6f, 1.0f) );
        camera->setProjectionMatrixAsPerspective(
            30.0f, (double)traits->width/(double)traits->height, 1.0, 1000.0 );
        
        osg::ref_ptr<osg::Group> root = new osg::Group;
        root->addChild( osgDB::readNodeFile("cessna.osg") );
        
        _commandHandler = new CommandHandler;
		_viewer.addEventHandler( _commandHandler.get() );
        
        _viewer.setCamera( camera.get() );
        _viewer.setSceneData( root.get() );
        _viewer.setKeyEventSetsDone( 0 );
        _viewer.setCameraManipulator( new osgGA::TrackballManipulator );
        
        _thread = new RenderingThread;
        _thread->viewerPtr = &_viewer;
        _thread->start();
    }
    return false;
}

bool osgWeb::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    delete _thread;
    return false;
}

