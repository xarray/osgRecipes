#include <osg/Image>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "GuichanWrapper.h"

class EventHandler : public osgGA::GUIEventHandler
{
public:
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
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
            if ( _camera.valid() )
                _guichan->setContainerSize( 0, 0, width, height );
            break;
        default:
            if ( _camera.valid() ) _guichan->pushInput( ea );
            break;
        }
        return false;
    }
    
    EventHandler( GuichanDrawable* guichan, osg::Camera* hudCam )
    : _guichan(guichan), _camera(hudCam) {}
    
protected:
    osg::observer_ptr<GuichanDrawable> _guichan;
    osg::observer_ptr<osg::Camera> _camera;
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    osg::ref_ptr<GuichanDrawable> guichan = new GuichanDrawable;
    
    gcn::Button* button = new gcn::Button( "Button" );
    button->setSize( 240, 60 );
    guichan->addWidget( button, 200, 100 );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setCullingActive( false );
    geode->addDrawable( guichan.get() );
    geode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearMask( GL_STENCIL_BUFFER_BIT );
    camera->setClearStencil( 1 );
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0) );
    camera->addChild( geode.get() );
    
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFiles(arguments) );
    scene->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new EventHandler(guichan.get(), camera.get()) );
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
