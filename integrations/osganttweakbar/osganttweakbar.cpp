#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <AntTweakBar.h>
TwBar* g_twBar = NULL;

class TwGUIHandler : public osgGA::GUIEventHandler
{
public:
    TwGUIHandler()
    {
        TwInit( TW_OPENGL, NULL );
        g_twBar = TwNewBar( "OSGDemo" );
        initializeTwGUI();
    }
    
    void initializeTwGUI()
    {
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        float x = ea.getX(), y = ea.getWindowHeight() - ea.getY();
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::PUSH:
            TwMouseMotion( x, y );
            TwMouseButton( TW_MOUSE_PRESSED, getTwButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::RELEASE:
            TwMouseMotion( x, y );
            TwMouseButton( TW_MOUSE_RELEASED, getTwButton(ea.getButton()) );
            break;
        case osgGA::GUIEventAdapter::DRAG:
        case osgGA::GUIEventAdapter::MOVE:
            TwMouseMotion( x, y );
            break;
        case osgGA::GUIEventAdapter::KEYDOWN:
            {
                bool useCtrl = false;
                if ( ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL ) useCtrl = true;
                TwKeyPressed( getTwKey(ea.getKey(), useCtrl), getTwModKeyMask(ea.getModKeyMask()) );
            }
            break;
        default: break;
        }
        return false;
    }
    
protected:
    virtual ~TwGUIHandler()
    {
        TwTerminate();
    }
    
    TwMouseButtonID getTwButton( int button )
    {
        switch ( button )
        {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON: return TW_MOUSE_LEFT;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON: return TW_MOUSE_MIDDLE;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON: return TW_MOUSE_RIGHT;
        }
        return static_cast<TwMouseButtonID>(0);
    }
    
    int getTwKey( int key, bool useCtrl )
    {
        switch ( key )
        {
        case osgGA::GUIEventAdapter::KEY_BackSpace: return TW_KEY_BACKSPACE;
        case osgGA::GUIEventAdapter::KEY_Tab: return TW_KEY_TAB;
        case osgGA::GUIEventAdapter::KEY_Return: return TW_KEY_RETURN;
        case osgGA::GUIEventAdapter::KEY_Escape: return TW_KEY_ESCAPE;
        case osgGA::GUIEventAdapter::KEY_Left: return TW_KEY_LEFT;
        case osgGA::GUIEventAdapter::KEY_Right: return TW_KEY_RIGHT;
        case osgGA::GUIEventAdapter::KEY_Up: return TW_KEY_UP;
        case osgGA::GUIEventAdapter::KEY_Down: return TW_KEY_DOWN;
        case osgGA::GUIEventAdapter::KEY_Home: return TW_KEY_HOME;
        case osgGA::GUIEventAdapter::KEY_End: return TW_KEY_END;
        case osgGA::GUIEventAdapter::KEY_Insert: return TW_KEY_INSERT;
        case osgGA::GUIEventAdapter::KEY_Delete: return TW_KEY_DELETE;
        case osgGA::GUIEventAdapter::KEY_Page_Up: return TW_KEY_PAGE_UP;
        case osgGA::GUIEventAdapter::KEY_Page_Down: return TW_KEY_PAGE_DOWN;
        case osgGA::GUIEventAdapter::KEY_F1: return TW_KEY_F1;
        case osgGA::GUIEventAdapter::KEY_F2: return TW_KEY_F2;
        case osgGA::GUIEventAdapter::KEY_F3: return TW_KEY_F3;
        case osgGA::GUIEventAdapter::KEY_F4: return TW_KEY_F4;
        case osgGA::GUIEventAdapter::KEY_F5: return TW_KEY_F5;
        case osgGA::GUIEventAdapter::KEY_F6: return TW_KEY_F6;
        case osgGA::GUIEventAdapter::KEY_F7: return TW_KEY_F7;
        case osgGA::GUIEventAdapter::KEY_F8: return TW_KEY_F8;
        case osgGA::GUIEventAdapter::KEY_F9: return TW_KEY_F9;
        case osgGA::GUIEventAdapter::KEY_F10: return TW_KEY_F10;
        case osgGA::GUIEventAdapter::KEY_F11: return TW_KEY_F11;
        case osgGA::GUIEventAdapter::KEY_F12: return TW_KEY_F12;
        }
        if ( useCtrl && key<27 ) key += 'a' - 1;
        return key;
    }
    
    int getTwModKeyMask( int modkey )
    {
        int twModkey = 0;
        if ( modkey&osgGA::GUIEventAdapter::MODKEY_SHIFT ) twModkey |= TW_KMOD_SHIFT;
        if ( modkey&osgGA::GUIEventAdapter::MODKEY_ALT ) twModkey |= TW_KMOD_ALT;
        if ( modkey&osgGA::GUIEventAdapter::MODKEY_CTRL ) twModkey |= TW_KMOD_CTRL;
        return twModkey;
    }
};

class TwGUIDrawCallback : public osg::Camera::DrawCallback
{
public:
    virtual void operator()( osg::RenderInfo& renderInfo ) const
    {
        osg::Viewport* viewport = renderInfo.getCurrentCamera()->getViewport();
        if ( viewport ) TwWindowSize( viewport->width(), viewport->height() );
        TwDraw();
    }
};

class MyTrackballManipulator : public osgGA::TrackballManipulator
{
public:
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( g_twBar )
        {
            int iconified = 0;
            TwGetParam( g_twBar, NULL, "iconified", TW_PARAM_INT32, 1, &iconified );
            if ( !iconified ) return false;  // Camera manipulating is disabled if the GUI is opend
        }
        return osgGA::TrackballManipulator::handle(ea, aa);
    }
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
	viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
	viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    
    viewer.addEventHandler( new TwGUIHandler );
    viewer.getCamera()->setFinalDrawCallback( new TwGUIDrawCallback );
    viewer.setCameraManipulator( new MyTrackballManipulator );
	return viewer.run();
}
