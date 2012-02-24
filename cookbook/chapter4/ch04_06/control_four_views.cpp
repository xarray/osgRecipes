/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 6
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/Camera>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/CompositeViewer>

#include "CommonFunctions"

class AuxiliaryViewUpdater : public osgGA::GUIEventHandler
{
public:
    AuxiliaryViewUpdater()
    :   _distance(-1.0), _offsetX(0.0f), _offsetY(0.0f),
        _lastDragX(-1.0f), _lastDragY(-1.0f)
    {}
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = static_cast<osgViewer::View*>(&aa);
        if ( view )
        {
            switch ( ea.getEventType() )
            {
            case osgGA::GUIEventAdapter::PUSH:
                _lastDragX = -1.0f;
                _lastDragY = -1.0f;
                break;
            case osgGA::GUIEventAdapter::DRAG:
                if ( _lastDragX>0.0f && _lastDragY>0.0f )
                {
                    if ( ea.getButtonMask()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
                    {
                        _offsetX += ea.getX() - _lastDragX;
                        _offsetY += ea.getY() - _lastDragY;
                    }
                    else if ( ea.getButtonMask()==osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON )
                    {
                        float dy = ea.getY() - _lastDragY;
                        _distance *= 1.0 + dy / ea.getWindowHeight();
                        if ( _distance<1.0 ) _distance = 1.0;
                    }
                }
                _lastDragX = ea.getX();
                _lastDragY = ea.getY();
                break;
            case osgGA::GUIEventAdapter::FRAME:
                if ( view->getCamera() )
                {
                    osg::Vec3d eye, center, up;
                    view->getCamera()->getViewMatrixAsLookAt( eye, center, up );
                    
                    osg::Vec3d lookDir = center - eye; lookDir.normalize();
                    osg::Vec3d side = lookDir ^ up; side.normalize();
                    
                    const osg::BoundingSphere& bs = view->getSceneData()->getBound();
                    if ( _distance<0.0 ) _distance = bs.radius() * 3.0;
                    center = bs.center();
                    
                    center -= (side * _offsetX + up * _offsetY) * 0.1;
                    view->getCamera()->setViewMatrixAsLookAt( center-lookDir*_distance, center, up );
                }
                break;
            }
        }
        return false;
    }
    
protected:
    double _distance;
    float _offsetX, _offsetY;
    float _lastDragX, _lastDragY;
};

osgViewer::View* create2DView( int x, int y, int width, int height,
                               const osg::Vec3& lookDir, const osg::Vec3& up,
                               osg::GraphicsContext* gc, osg::Node* scene )
{
    osg::ref_ptr<osgViewer::View> view = new osgViewer::View;
    view->getCamera()->setGraphicsContext( gc );
    view->getCamera()->setViewport( x, y, width, height );
    view->setSceneData( scene );
    
    osg::Vec3 center = scene->getBound().center();
    double radius = scene->getBound().radius();
    view->getCamera()->setViewMatrixAsLookAt( center - lookDir*(radius*3.0), center, up );
    view->getCamera()->setProjectionMatrixAsPerspective(
        30.0f, static_cast<double>(width)/static_cast<double>(height), 1.0f, 10000.0f );
    return view.release();
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles( arguments );
    if ( !scene ) scene = osgDB::readNodeFile("cessna.osg");
    
    unsigned int width = 800, height = 600;
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if ( wsi )
        wsi->getScreenResolution( osg::GraphicsContext::ScreenIdentifier(0), width, height );
    
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
    if ( !gc || !scene ) return 1;
    
    int w_2 = width/2, h_2 = height/2;
    osg::ref_ptr<osgViewer::View> top = create2DView(
        0, h_2, w_2, h_2,-osg::Z_AXIS, osg::Y_AXIS, gc.get(), scene.get());
    osg::ref_ptr<osgViewer::View> front = create2DView(
        w_2, h_2, w_2, h_2, osg::Y_AXIS, osg::Z_AXIS, gc.get(), scene.get());
    osg::ref_ptr<osgViewer::View> left = create2DView(
        0, 0, w_2, h_2, osg::X_AXIS, osg::Z_AXIS, gc.get(), scene.get());
    
    osg::ref_ptr<osgViewer::View> mainView = new osgViewer::View;
    mainView->getCamera()->setGraphicsContext( gc.get() );
    mainView->getCamera()->setViewport( w_2, 0, w_2, h_2 );
    mainView->setSceneData( scene.get() );
    mainView->setCameraManipulator( new osgGA::TrackballManipulator );
    
    osgViewer::CompositeViewer viewer;
    viewer.addView( top.get() );
    viewer.addView( front.get() );
    viewer.addView( left.get() );
    viewer.addView( mainView.get() );
    
    top->addEventHandler( new AuxiliaryViewUpdater );
    front->addEventHandler( new AuxiliaryViewUpdater );
    left->addEventHandler( new AuxiliaryViewUpdater );
    
    while ( !viewer.done() )
    {
        viewer.frame();
    }
    return 0;
}
