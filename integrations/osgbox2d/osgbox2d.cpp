#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include "Box2D/Box2D.h"

class Box2DUpdater : public osgGA::GUIEventHandler
{
public:
    Box2DUpdater( b2World* world ) : _world(world) {}
    
    osg::Node* createBoundary( float halfWidth, float halfHeight )
    {
        b2Vec2 vertices[4] = {
            b2Vec2(-halfWidth,-halfHeight), b2Vec2(halfWidth,-halfHeight),
            b2Vec2(halfWidth, halfHeight), b2Vec2(-halfWidth, halfHeight)
        };
        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position.Set( 0.0f, 0.0f );
        b2ChainShape borders;
        borders.CreateLoop( &(vertices[0]), 4 );
        b2Body* body = _world->CreateBody( &bodyDef );
        body->CreateFixture( &borders, 0.0f );
        
        osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
        hints->setCreateTop( false );
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(
            new osg::Box(osg::Vec3(), halfWidth*2.0f, halfHeight*2.0f, 0.1f), hints.get()) );
        return geode.release();
    }
    
    osg::MatrixTransform* createBoxObject( const osg::Vec2& pos, const osg::Vec2& size, float density,
                                           const osg::Vec2& vel=osg::Vec2(), float damping=0.3f )
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set( pos.x(), pos.y() );
        b2PolygonShape boxShape;
        boxShape.SetAsBox( size[0]*0.5f, size[1]*0.5f );
        
        b2Body* body = _world->CreateBody( &bodyDef );
        body->CreateFixture( &boxShape, density );
        body->SetLinearVelocity( b2Vec2(vel.x(), vel.y()) );
        body->SetLinearDamping( damping );
        return createShapeTransform(pos, new osg::Box(osg::Vec3(), size[0], size[1], 0.1f), body);
    }
    
    osg::MatrixTransform* createDiskObject( const osg::Vec2& pos, float radius, float density,
                                            const osg::Vec2& vel=osg::Vec2(), float damping=0.3f )
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set( pos.x(), pos.y() );
        b2CircleShape circleShape;
        circleShape.m_radius = radius;
        
        b2Body* body = _world->CreateBody( &bodyDef );
        body->CreateFixture( &circleShape, density );
        body->SetLinearVelocity( b2Vec2(vel.x(), vel.y()) );
        body->SetLinearDamping( damping );
        return createShapeTransform(pos, new osg::Cylinder(osg::Vec3(), radius, 0.1f), body);
    }
    
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = static_cast<osgViewer::View*>( &aa );
        if ( !view || !view->getSceneData() ) return false;
        
        osg::Group* root = view->getSceneData()->asGroup();
        float randomPos = (float)rand()/(RAND_MAX+1.0f) * 9.6f - 4.8f;
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::KEYUP:
            switch ( ea.getKey() )
            {
            case osgGA::GUIEventAdapter::KEY_Left:
                root->addChild( createDiskObject(osg::Vec2(4.8f, randomPos), 0.2f, 2.0f, osg::Vec2(-2.0f, 0.0f), 0.02f) );
                break;
            case osgGA::GUIEventAdapter::KEY_Right:
                root->addChild( createDiskObject(osg::Vec2(-4.8f, randomPos), 0.2f, 2.0f, osg::Vec2(2.0f, 0.0f), 0.02f) );
                break;
            case osgGA::GUIEventAdapter::KEY_Up:
                root->addChild( createDiskObject(osg::Vec2(randomPos, -4.8f), 0.2f,2.0f, osg::Vec2(0.0f, 2.0f), 0.02f) );
                break;
            case osgGA::GUIEventAdapter::KEY_Down:
                root->addChild( createDiskObject(osg::Vec2(randomPos, 4.8f), 0.2f, 2.0f, osg::Vec2(0.0f, -2.0f), 0.02f) );
                break;
            default: break;
            }
            break;
        case osgGA::GUIEventAdapter::FRAME:
            _world->Step( 1.0f/60.0f, 6, 2 );
            for ( BodyMap::iterator itr=_bodyMap.begin(); itr!=_bodyMap.end(); ++itr )
            {
                b2Body* body = itr->first;
                const b2Vec2& pos = body->GetPosition();
                itr->second->setMatrix( osg::Matrix::rotate(body->GetAngle(), osg::Z_AXIS)
                                      * osg::Matrix::translate(pos.x, pos.y, 0.0f) );
            }
            break;
        default: break;
        }
        return false;
    }
    
protected:
    osg::MatrixTransform* createShapeTransform( const osg::Vec2& pos, osg::Shape* shape, b2Body* body )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(shape) );
        
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->setMatrix( osg::Matrix::translate(osg::Vec3(pos, 0.0f)) );
        mt->addChild( geode.get() );
        _bodyMap[body] = mt.get();
        return mt.get();
    }
    
    typedef std::map<b2Body*, osg::ref_ptr<osg::MatrixTransform> > BodyMap;
    BodyMap _bodyMap;
    b2World* _world;
};

int main( int argc, char** argv )
{
    b2World world( b2Vec2(0.0f, 0.0f) );
    osg::ref_ptr<Box2DUpdater> updater = new Box2DUpdater( &world );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( updater->createBoundary(5.0f, 5.0f) );
    for ( int i=-8; i<8; ++i )
    {
        if ( i!=0 )
            root->addChild( updater->createBoxObject(osg::Vec2(i*0.5f,-i*0.5f), osg::Vec2(0.4f, 0.4f), 1.0f) );
        root->addChild( updater->createBoxObject(osg::Vec2(i*0.5f, i*0.5f), osg::Vec2(0.4f, 0.4f), 1.0f) );
    }
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.addEventHandler( updater.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}
