#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "TUIO/TuioClient.h"
#include "TUIO/TuioListener.h"

class ClientListener : public TUIO::TuioListener
{
public:
    ClientListener() : active(false) {}
    bool active;
    
    virtual void addTuioObject( TUIO::TuioObject* tobj ) { active = true; }
    virtual void updateTuioObject( TUIO::TuioObject* tobj ) { active = true; }
    virtual void removeTuioObject( TUIO::TuioObject* tobj ) { active = true; }
    virtual void addTuioCursor( TUIO::TuioCursor* tcur ) { active = true; }
    virtual void updateTuioCursor( TUIO::TuioCursor* tcur ) { active = true; }
    virtual void removeTuioCursor( TUIO::TuioCursor* tcur ) { active = true; }
    virtual void addTuioBlob( TUIO::TuioBlob* tblb ) { active = true; }
    virtual void updateTuioBlob( TUIO::TuioBlob* tblb ) { active = true; }
    virtual void removeTuioBlob( TUIO::TuioBlob* tblb ) { active = true; }
    virtual void refresh( TUIO::TuioTime ftime ) {}
};

class TUIOClientHandler : public osgGA::GUIEventHandler
{
public:
    TUIOClientHandler( int port=3333 )
    {
        _listener = new ClientListener;
        _client = new TUIO::TuioClient( port );
        _client->addTuioListener( _listener );
        _client->connect();
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>( &aa );
        if ( !view ) return false;
        
        osgGA::EventQueue* queue = view->getEventQueue();
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::FRAME:
            if ( _listener->active )
            {
                _listener->active = false;  // FIXME: is that safe enough for multithreading?
                std::map<unsigned int, osg::Vec2d> lastIDs = _clientIDs;
                std::list<TUIO::TuioCursor*> cursors = _client->getTuioCursors();
                
                osg::ref_ptr<osgGA::GUIEventAdapter> touched;
                for ( std::list<TUIO::TuioCursor*>::iterator itr=cursors.begin();
                      itr!=cursors.end(); ++itr )
                {
                    TUIO::TuioCursor* c = *itr;
                    if ( !c ) continue;
                    
                    unsigned int id = c->getCursorID();
                    double x = 0.1 * c->getX() * ea.getWindowWidth() + ea.getWindowX();
                    double y = 0.1 * c->getY() * ea.getWindowHeight() + ea.getWindowY();
                    if ( _clientIDs.find(id)!=_clientIDs.end() )
                    {
                        if ( !touched ) touched = queue->touchMoved(id, osgGA::GUIEventAdapter::TOUCH_MOVED, x, y);
                        else touched->addTouchPoint(id, osgGA::GUIEventAdapter::TOUCH_MOVED, x, y);
                        lastIDs.erase( id );
                    }
                    else
                    {
                        if ( !touched ) touched = queue->touchBegan(id, osgGA::GUIEventAdapter::TOUCH_BEGAN, x, y);
                        else touched->addTouchPoint(id, osgGA::GUIEventAdapter::TOUCH_BEGAN, x, y);
                    }
                    _clientIDs[id] = osg::Vec2d(x, y);
                }
                
                for ( std::map<unsigned int, osg::Vec2d>::iterator itr=lastIDs.begin();
                      itr!=lastIDs.end(); ++itr )
                {
                    // Unset IDs are going to be removed this time
                    double x = itr->second.x(), y = itr->second.y();
                    if ( !touched ) touched = queue->touchEnded(itr->first, osgGA::GUIEventAdapter::TOUCH_ENDED, x, y, 1);
                    else touched->addTouchPoint(itr->first, osgGA::GUIEventAdapter::TOUCH_ENDED, x, y, 1);
                    _clientIDs.erase( itr->first );
                }
            }
            break;
        default: break;
        }
        return false;
    }
    
protected:
    virtual ~TUIOClientHandler()
    {
        if ( _client )
        {
            _client->removeTuioListener( _listener );
            _client->disconnect();
            delete _client;
        }
        if ( _listener ) delete _listener;
    }
    
    std::map<unsigned int, osg::Vec2d> _clientIDs;
    TUIO::TuioClient* _client;
    ClientListener* _listener;
};

int main( int argc, char** argv )
{
    osg::ref_ptr<osg::MatrixTransform> scene = new osg::MatrixTransform;
    scene->addChild( osgDB::readNodeFile("cow.osg") );
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( new TUIOClientHandler(3333) );
    viewer.setCameraManipulator( new osgGA::MultiTouchTrackballManipulator );
    viewer.setSceneData( scene.get() );
    return viewer.run();
}
