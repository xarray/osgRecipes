#include <osg/Image>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "GuichanWrapper.h"

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    osg::ref_ptr<GuichanDrawable> guichan = new GuichanDrawable;
    guichan->setViewport( 0, 0, 1280, 960 );
    
    gcn::Button* button = new gcn::Button( "Button" );
    button->setSize( 240, 60 );
    guichan->addWidget( button, 200, 100 );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setCullingActive( false );
    geode->addDrawable( guichan.get() );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFiles(arguments) );
    scene->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( scene.get() );
    viewer.realize();
    
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>( viewer.getCamera()->getGraphicsContext() );
    if ( gw )
    {
        // Send a window size event for resizing the UI
        int x, y, w, h; gw->getWindowRectangle( x, y, w, h );
        viewer.getEventQueue()->windowResize( x, y, w, h );
    }
    return viewer.run();
}
