#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/EventVisitor>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <IGizmo.h>

class GizmoDrawable : public osg::Drawable
{
public:
    struct GizmoEventCallback : public osg::Drawable::EventCallback
    {
        virtual void event( osg::NodeVisitor* nv, osg::Drawable* drawable )
        {
            osgGA::EventVisitor* ev = static_cast<osgGA::EventVisitor*>( nv );
            GizmoDrawable* gizmoDrawable = dynamic_cast<GizmoDrawable*>( drawable );
            if ( !ev || !gizmoDrawable ) return;
            
            const osgGA::EventQueue::Events& events = ev->getEvents();
            for ( osgGA::EventQueue::Events::const_iterator itr=events.begin();
                  itr!=events.end(); ++itr )
            {
                const osgGA::GUIEventAdapter* ea = (*itr)->asGUIEventAdapter();
                int x = ea->getX(), y = ea->getY();
                if ( ea->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS )
                    y = ea->getWindowHeight() - y;
                
                switch ( ea->getEventType() )
                {
                case osgGA::GUIEventAdapter::SCROLL:
                    // you would have other methods to select among gizmos, this is only an example
                    {
                        int mode = gizmoDrawable->getGizmoMode();
                        if ( ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP )
                            mode = (mode==GizmoDrawable::NO_GIZMO ? GizmoDrawable::SCALE_GIZMO : mode-1);
                        else if ( ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN )
                            mode = (mode==GizmoDrawable::SCALE_GIZMO ? GizmoDrawable::NO_GIZMO : mode+1);
                        gizmoDrawable->setGizmoMode( (GizmoDrawable::Mode)mode );
                    }
                    break;
                case osgGA::GUIEventAdapter::PUSH:
                    if ( gizmoDrawable->getGizmoObject() )
                        gizmoDrawable->getGizmoObject()->OnMouseDown( x, y );
                    break;
                case osgGA::GUIEventAdapter::RELEASE:
                    if ( gizmoDrawable->getGizmoObject() )
                        gizmoDrawable->getGizmoObject()->OnMouseUp( x, y );
                    break;
                case osgGA::GUIEventAdapter::MOVE:
                case osgGA::GUIEventAdapter::DRAG:
                    if ( gizmoDrawable->getGizmoObject() )
                        gizmoDrawable->getGizmoObject()->OnMouseMove( x, y );
                    break;
                case osgGA::GUIEventAdapter::RESIZE:
                    gizmoDrawable->setScreenSize( ea->getWindowWidth(), ea->getWindowHeight() );
                    break;
                case osgGA::GUIEventAdapter::FRAME:
                    gizmoDrawable->applyTransform();
                    break;
                default: break;
                }
            }
        }
    };
    
    GizmoDrawable() : _gizmo(0), _mode(NO_GIZMO)
    {
        osg::Matrix matrix;
        for ( int i=0; i<16; ++i )
            _editMatrix[i] = *(matrix.ptr() + i);
        _screenSize[0] = 800;
        _screenSize[1] = 600;
        
        setEventCallback( new GizmoEventCallback );
        setSupportsDisplayList( false );
        getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
    }
    
    GizmoDrawable( const GizmoDrawable& copy, osg::CopyOp op=osg::CopyOp::SHALLOW_COPY )
    :   osg::Drawable(copy, op),
        _transform(copy._transform), _gizmo(copy._gizmo), _mode(copy._mode)
    {
    }
    
    enum Mode { NO_GIZMO=0, MOVE_GIZMO, ROTATE_GIZMO, SCALE_GIZMO };
    void setGizmoMode( Mode m, IGizmo::LOCATION loc=IGizmo::LOCATE_LOCAL )
    {
        _mode = m;
        if ( _gizmo ) delete _gizmo;
        switch ( m )
        {
        case MOVE_GIZMO: _gizmo = CreateMoveGizmo(); break;
        case ROTATE_GIZMO: _gizmo = CreateRotateGizmo(); break;
        case SCALE_GIZMO: _gizmo = CreateScaleGizmo(); break;
        default: _gizmo = NULL; return;
        }
        
        if ( _gizmo )
        {
            _gizmo->SetEditMatrix( _editMatrix );
            _gizmo->SetScreenDimension( _screenSize[0], _screenSize[1] );
            _gizmo->SetLocation( loc );
            //_gizmo->SetDisplayScale( 0.5f );
        }
    }
    
    Mode getGizmoMode() const { return _mode; }
    IGizmo* getGizmoObject() { return _gizmo; }
    const IGizmo* getGizmoObject() const { return _gizmo; }
    
    void setTransform( osg::MatrixTransform* node )
    {
        _transform = node;
        if ( !node ) return;
        
        const osg::Matrix& matrix = node->getMatrix();
        for ( int i=0; i<16; ++i )
            _editMatrix[i] = *(matrix.ptr() + i);
    }
    
    osg::MatrixTransform* getTransform() { return _transform.get(); }
    const osg::MatrixTransform* getTransform() const { return _transform.get(); }
    
    void setScreenSize( int w, int h )
    {
        _screenSize[0] = w;
        _screenSize[1] = h;
        if ( _gizmo )
            _gizmo->SetScreenDimension( w, h );
    }
    
    void applyTransform()
    {
        if ( _gizmo && _transform.valid() )
            _transform->setMatrix( osg::Matrix(_editMatrix) );
    }
    
    META_Object( osg, GizmoDrawable );
    
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const
    {
        osg::State* state = renderInfo.getState();
        state->disableAllVertexArrays();
        state->disableTexCoordPointer( 0 );
        
        glPushMatrix();
        glPushAttrib( GL_ALL_ATTRIB_BITS );
		if ( _gizmo )
		{
		    _gizmo->SetCameraMatrix( osg::Matrixf(state->getModelViewMatrix()).ptr(),
		                             osg::Matrixf(state->getProjectionMatrix()).ptr() );
		    _gizmo->Draw();
        }
        glPopAttrib();
        glPopMatrix();
    }
    
protected:
    virtual ~GizmoDrawable() {}
    
    osg::observer_ptr<osg::MatrixTransform> _transform;
    IGizmo* _gizmo;
    float _editMatrix[16];
    int _screenSize[2];
    Mode _mode;
};

class MyTrackballManipulator : public osgGA::TrackballManipulator
{
public:
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        if ( !(ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_CTRL) )
            return false;  // force using CTRL when manipulating
        return osgGA::TrackballManipulator::handle(ea, aa);
    }
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    
    osg::ref_ptr<GizmoDrawable> gizmo = new GizmoDrawable;
    gizmo->setTransform( scene.get() );
    gizmo->setGizmoMode( GizmoDrawable::MOVE_GIZMO );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( gizmo.get() );
    geode->setCullingActive( false );  // allow gizmo to always display
    geode->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );  // always show at last
    
    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    root->addChild( scene.get() );
    root->addChild( geode.get() );
    
    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.setCameraManipulator( new MyTrackballManipulator );
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
	viewer.realize();
    
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>( viewer.getCamera()->getGraphicsContext() );
    if ( gw )
    {
        // Send window size for libGizmo to initialize
        int x, y, w, h; gw->getWindowRectangle( x, y, w, h );
        viewer.getEventQueue()->windowResize( x, y, w, h );
    }
	return viewer.run();
}
