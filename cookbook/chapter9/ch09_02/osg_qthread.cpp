/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 2
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <osgQt/GraphicsWindowQt>

#include "CommonFunctions"

osg::Camera* createCamera( int x, int y, int w, int h )
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowDecoration = false;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext( new osgQt::GraphicsWindowQt(traits.get()) );
    camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
    return camera.release();
}

class RenderThread : public QThread
{
public:
    RenderThread() : QThread(), viewerPtr(0) {}
    
    virtual ~RenderThread()
    { if (viewerPtr) viewerPtr->setDone(true); wait(); }
    
    osgViewer::Viewer* viewerPtr;
    
protected:
    virtual void run()
    { if (viewerPtr) viewerPtr->run(); }
};

class ViewerWidget : public QWidget
{
public:
    ViewerWidget( osg::Camera* camera, osg::Node* scene )
    :   QWidget()
    {
        _viewer.setCamera( camera );
        _viewer.setSceneData( scene );
        _viewer.addEventHandler( new osgViewer::StatsHandler );
        _viewer.setCameraManipulator( new osgGA::TrackballManipulator );
        _viewer.setThreadingModel( osgViewer::Viewer::SingleThreaded );
        
        osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
        if ( gw )
        {
            QVBoxLayout* layout = new QVBoxLayout;
            layout->addWidget( gw->getGLWidget() );
            setLayout( layout );
        }
        
        _thread.viewerPtr = &_viewer;
        _thread.start();
    }
    
protected:
    osgViewer::Viewer _viewer;
    RenderThread _thread;
};

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    osg::Camera* camera = createCamera( 50, 50, 640, 480 );
    osg::Node* scene = osgDB::readNodeFile("cow.osg");
    
    ViewerWidget* widget = new ViewerWidget(camera, scene);
    widget->setGeometry( 100, 100, 800, 600 );
    widget->show();
    return app.exec();
}
