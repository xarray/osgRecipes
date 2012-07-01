#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>
#include <queue>

#include <AntTweakBar.h>
TwBar* g_twBar = NULL;
char g_fileName[64] = "cow.osg";
float position[3] = {0.0f};
float rotation[4] = {0.0f, 0.0f, 0.0f, 1.0f};

class TwGUIManager : public osgGA::GUIEventHandler, public osg::Camera::DrawCallback
{
public:
    TwGUIManager( osg::MatrixTransform* node=0 ) : _scene(node)
    {
        TwInit( TW_OPENGL, NULL );
        g_twBar = TwNewBar( "OSGDemo" );
        initializeTwGUI();
    }
    
    TwGUIManager( const TwGUIManager& copy, const osg::CopyOp& op=osg::CopyOp::SHALLOW_COPY )
    :   _eventsToHandle(copy._eventsToHandle), _scene(copy._scene)
    {
    }
    
    META_Object( osg, TwGUIManager )
    
    static void TW_CALL loadModelFunc( void* clientData )
    {
        TwGUIManager* manager = (TwGUIManager*)clientData;
        if ( manager && manager->_scene.valid() )
        {
            manager->_scene->removeChild( 0, manager->_scene->getNumChildren() );
            manager->_scene->addChild( osgDB::readNodeFile(g_fileName) );
        }
    }
    
    void initializeTwGUI()
    {
        TwDefine(" OSGDemo size='240 400' color='96 216 224' ");
        
        TwAddVarRW( g_twBar, "ModelName", TW_TYPE_CSSTRING(sizeof(g_fileName)), g_fileName, " label='Model name' " );
        TwAddButton( g_twBar, "LoadButton", TwGUIManager::loadModelFunc, this, " label='Load model from file' " );
        TwAddSeparator( g_twBar, NULL, NULL );
        TwAddVarRW( g_twBar, "PosX", TW_TYPE_FLOAT, &(position[0]), " step=0.1 " );
        TwAddVarRW( g_twBar, "PosY", TW_TYPE_FLOAT, &(position[1]), " step=0.1 " );
        TwAddVarRW( g_twBar, "PosZ", TW_TYPE_FLOAT, &(position[2]), " step=0.1 " );
        TwAddVarRW( g_twBar, "Rotation", TW_TYPE_QUAT4F, &(rotation[0]), NULL );
    }
    
    void updateEvents() const
    {
        unsigned int size = _eventsToHandle.size();
        for ( unsigned int i=0; i<size; ++i )
        {
            const osgGA::GUIEventAdapter& ea = *(_eventsToHandle.front());
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
            const_cast<TwGUIManager*>(this)->_eventsToHandle.pop();
        }
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::FRAME:  // Update transform values
            if ( _scene.valid() )
            {
                osg::Vec3 pos(position[0], position[1], position[2]);
                osg::Quat quat(rotation[0], rotation[1], rotation[2], rotation[3]);
                _scene->setMatrix( osg::Matrix::rotate(quat) * osg::Matrix::translate(pos) );
            }
            return false;
        }
        
        // As AntTweakBar handle all events within the OpenGL context, we have to record the event here
        // and process it later in the draw callback
        _eventsToHandle.push( &ea );
        return false;
    }
    
    virtual void operator()( osg::RenderInfo& renderInfo ) const
    {
        osg::Viewport* viewport = renderInfo.getCurrentCamera()->getViewport();
        if ( viewport ) TwWindowSize( viewport->width(), viewport->height() );
        updateEvents();
        TwDraw();
    }
    
protected:
    virtual ~TwGUIManager()
    {
        TwTerminate();
    }
    
    TwMouseButtonID getTwButton( int button ) const
    {
        switch ( button )
        {
        case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON: return TW_MOUSE_LEFT;
        case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON: return TW_MOUSE_MIDDLE;
        case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON: return TW_MOUSE_RIGHT;
        }
        return static_cast<TwMouseButtonID>(0);
    }
    
    int getTwKey( int key, bool useCtrl ) const
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
    
    int getTwModKeyMask( int modkey ) const
    {
        int twModkey = 0;
        if ( modkey&osgGA::GUIEventAdapter::MODKEY_SHIFT ) twModkey |= TW_KMOD_SHIFT;
        if ( modkey&osgGA::GUIEventAdapter::MODKEY_ALT ) twModkey |= TW_KMOD_ALT;
        if ( modkey&osgGA::GUIEventAdapter::MODKEY_CTRL ) twModkey |= TW_KMOD_CTRL;
        return twModkey;
    }
    
    std::queue< osg::ref_ptr<const osgGA::GUIEventAdapter> > _eventsToHandle;
    osg::observer_ptr<osg::MatrixTransform> _scene;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( scene.get() );
    
    osg::ref_ptr<TwGUIManager> twGUI = new TwGUIManager( scene.get() ); 
    viewer.addEventHandler( twGUI.get() );
    viewer.getCamera()->setFinalDrawCallback( twGUI.get() );
    return viewer.run();
}
