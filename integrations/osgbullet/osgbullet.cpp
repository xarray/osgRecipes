#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "BulletInterface.h"

class SampleRigidUpdater : public osgGA::GUIEventHandler
{
public:
    SampleRigidUpdater( osg::Group* root ) : _root(root) {}
    
    void addGround( const osg::Vec3& gravity )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(
            new osg::Box(osg::Vec3(0.0f, 0.0f,-0.5f), 100.0f, 100.0f, 1.0f)) );
        
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( geode.get() );
        _root->addChild( mt.get() );
        
        BulletInterface::instance()->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), gravity );
    }
    
    void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        BulletInterface::instance()->createBox( id, shape->getHalfLengths(), mass );
        addPhysicsData( id, shape, pos, vel, mass );
    }
    
    void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        BulletInterface::instance()->createSphere( id, shape->getRadius(), mass );
        addPhysicsData( id, shape, pos, vel, mass );
    }
    
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = static_cast<osgViewer::View*>( &aa );
        if ( !view || !_root ) return false;
        
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::KEYUP:
            if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Return )
            {
                osg::Vec3 eye, center, up, dir;
                view->getCamera()->getViewMatrixAsLookAt( eye, center, up );
                dir = center - eye; dir.normalize();
                addPhysicsSphere( new osg::Sphere(osg::Vec3(), 0.5f), eye, dir * 60.0f, 2.0 );
            }
            break;
        case osgGA::GUIEventAdapter::FRAME:
            BulletInterface::instance()->simulate( 0.02 );
            for ( NodeMap::iterator itr=_physicsNodes.begin();
                  itr!=_physicsNodes.end(); ++itr )
            {
                osg::Matrix matrix = BulletInterface::instance()->getMatrix(itr->first);
                itr->second->setMatrix( matrix );
            }
            break;
        default: break;
        }
        return false;
    }
    
protected:
    void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
                         const osg::Vec3& vel, double mass )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(shape) );
        
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( geode.get() );
        _root->addChild( mt.get() );
        
        BulletInterface::instance()->setMatrix( id, osg::Matrix::translate(pos) );
        BulletInterface::instance()->setVelocity( id, vel );
        _physicsNodes[id] = mt;
    }
    
    typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
    NodeMap _physicsNodes;
    osg::observer_ptr<osg::Group> _root;
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    int mode = 0;
    if ( arguments.read("--rigid") ) mode = 0;
    //else if ( arguments.read("--cloth") ) mode = 1;
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osgGA::GUIEventHandler> updater;
    switch ( mode )
    {
    case 0:
        {
            SampleRigidUpdater* rigidUpdater = new SampleRigidUpdater( root.get() );
            rigidUpdater->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );
            for ( unsigned int i=0; i<10; ++i )
            {
                for ( unsigned int j=0; j<10; ++j )
                {
                    rigidUpdater->addPhysicsBox( new osg::Box(osg::Vec3(), 0.99f),
                        osg::Vec3((float)i, 0.0f, (float)j+0.5f), osg::Vec3(), 1.0f );
                }
            }
            updater = rigidUpdater;
        }
        break;
    case 1:
        break;
    default: break;
    }
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    if ( updater.valid() )
        viewer.addEventHandler( updater.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
