/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 9 Recipe 4
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/BlendFunc>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>

#include "CommonFunctions"
#include "CEGUIDrawable"

class CEGUIEventHandler : public osgGA::GUIEventHandler
{
public:
    CEGUIEventHandler( osg::Camera* camera ) : _camera(camera) {}
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        int x = ea.getX(), y = ea.getY(), width = ea.getWindowWidth(), height = ea.getWindowHeight();
        if ( ea.getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS )
            y = ea.getWindowHeight() - y;
        if ( !CEGUI::System::getSingletonPtr() )
            return false;
        
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::PUSH:
            CEGUI::System::getSingleton().injectMousePosition( x, y );
            CEGUI::System::getSingleton().injectMouseButtonDown( convertMouseButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            CEGUI::System::getSingleton().injectMousePosition( x, y );
            CEGUI::System::getSingleton().injectMouseButtonUp( convertMouseButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::SCROLL:
            if ( ea.getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN )
                CEGUI::System::getSingleton().injectMouseWheelChange( -1 );
            else if ( ea.getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP )
                CEGUI::System::getSingleton().injectMouseWheelChange( +1 );
            break;
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
            CEGUI::System::getSingleton().injectMousePosition( x, y );
            break;
        case osgGA::GUIEventAdapter::RESIZE:
            if ( _camera.valid() )
            {
                _camera->setProjectionMatrix( osg::Matrixd::ortho2D(0.0, width, 0.0, height) );
                _camera->setViewport( 0.0, 0.0, width, height );
            }
            break;
        default:
            return false;
        }
        
        CEGUI::Window* rootWindow = CEGUI::System::getSingleton().getGUISheet();
        if ( rootWindow )
        {
            CEGUI::Window* anyWindow = rootWindow->getChildAtPosition( CEGUI::Vector2(x, y) );
            if ( anyWindow ) return true;
        }
        return false;
    }
    
protected:
    CEGUI::MouseButton convertMouseButton( int button )
    {
        switch ( button )
        {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
            return CEGUI::LeftButton;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
            return CEGUI::MiddleButton;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
            return CEGUI::RightButton;
        default: break;
        }
        return static_cast<CEGUI::MouseButton>(button);
    }
    
    osg::observer_ptr<osg::Camera> _camera;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->setCullingActive( false );
    geode->addDrawable( new CEGUIDrawable );
    geode->getOrCreateStateSet()->setAttributeAndModes( new osg::BlendFunc );
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    
    osg::ref_ptr<osg::Camera> hudCamera = osgCookBook::createHUDCamera(0, 800, 0, 600);
    hudCamera->setAllowEventFocus( true );
    hudCamera->addChild( geode.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( osgDB::readNodeFile("cow.osg") );
    root->addChild( hudCamera.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new CEGUIEventHandler(hudCamera.get()) );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    return viewer.run();
}
