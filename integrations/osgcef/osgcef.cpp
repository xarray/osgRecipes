#include <windows.h>

#include <osg/ImageUtils>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_frame.h>
#include <include/cef_runnable.h>

int main( int argc, char** argv )
{
    // TODO
#if 0
    CefInitialize();
    
    osg::ArgumentParser arguments( &argc, argv );
    CefMainArgs mainArgs( ::GetModuleHandle(NULL) );
    
    CefRefPtr<ClientApp> app( new ClientApp );
    
    int exitCode = CefExecuteProcess( mainArgs, app.get(), NULL );
    if ( exitCode>0 ) return exitCode;
    
    CefSettings settings;
    settings.no_sandbox = true;
    settings.multi_threaded_message_loop = false;
    CefInitialize( mainArgs, settings, app.get(), NULL );
    
    std::string url("http://www.openscenegraph.com");
    arguments.read( "--url", url );
    
    int w = 800, h = 600;
    arguments.read( "--width", w );
    arguments.read( "--height", h );
    
    osg::ref_ptr<ChromeImage> image = new ChromeImage;
    image->loadURL( url, w, h );
    
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
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
	viewer.setCameraManipulator( new osgGA::TrackballManipulator );
	viewer.realize();
	
	while ( !viewer.done() )
	{
        CefDoMessageLoopWork();
        viewer.frame();
	}
    
	image = NULL;  // Destroy the web view
    CefShutdown();
#endif
	return 0;
}
