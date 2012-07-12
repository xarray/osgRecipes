#include <osg/ref_ptr>
#include "BulletInterface.h"

BulletInterface* BulletInterface::instance()
{
    static osg::ref_ptr<BulletInterface> s_registry = new BulletInterface;
    return s_registry.get();
}

BulletInterface::BulletInterface()
:   _scene(NULL)
{
    _configuration = new btDefaultCollisionConfiguration;
    _dispatcher = new btCollisionDispatcher( _configuration );
    _overlappingPairCache = new btDbvtBroadphase;
    _solver = new btSequentialImpulseConstraintSolver;
}

BulletInterface::~BulletInterface()
{
    if ( _scene )
    {
        for ( int i=_scene->getNumCollisionObjects()-1; i>=0; --i )
        {
            btCollisionObject* obj = _scene->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if ( body && body->getMotionState() )
                delete body->getMotionState();
            
            _scene->removeCollisionObject( obj );
            delete obj;
        }
        delete _scene;
    }
    
    delete _solver;
    delete _overlappingPairCache;
    delete _dispatcher;
    delete _configuration;
}

void BulletInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity )
{
    _scene = new btDiscreteDynamicsWorld( _dispatcher, _overlappingPairCache, _solver, _configuration );
    _scene->setGravity( btVector3(gravity[0], gravity[1], gravity[2]) );
    
    osg::Vec3 norm = plane.getNormal();
    btCollisionShape* groundShape = new btStaticPlaneShape( btVector3(norm[0], norm[1], norm[2]), plane[3] );
    btTransform groundTransform;
	groundTransform.setIdentity();
	
    btDefaultMotionState* motionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( 0.0, motionState, groundShape, btVector3(0.0, 0.0, 0.0) );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body );
}

void BulletInterface::createBox( int id, const osg::Vec3& dim, double density )
{
    btCollisionShape* boxShape = new btBoxShape( btVector3(dim[0], dim[1], dim[2]) );
    btTransform boxTransform;
	boxTransform.setIdentity();
	
	btVector3 localInertia(0.0, 0.0, 0.0);
	if ( density>0.0 )
	    boxShape->calculateLocalInertia( density, localInertia );
	
    btDefaultMotionState* motionState = new btDefaultMotionState(boxTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( density, motionState, boxShape, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body );
    _actors[id] = body;
}

void BulletInterface::createSphere( int id, double radius, double density )
{
    btCollisionShape* sphereShape = new btSphereShape( radius );
    btTransform sphereTransform;
	sphereTransform.setIdentity();
	
	btVector3 localInertia(0.0, 0.0, 0.0);
	if ( density>0.0 )
	    sphereShape->calculateLocalInertia( density, localInertia );
	
    btDefaultMotionState* motionState = new btDefaultMotionState(sphereTransform);
    btRigidBody::btRigidBodyConstructionInfo rigidInfo( density, motionState, sphereShape, localInertia );
    btRigidBody* body = new btRigidBody(rigidInfo);
    _scene->addRigidBody( body );
    _actors[id] = body;
}

void BulletInterface::setVelocity( int id, const osg::Vec3& vec )
{
    btRigidBody* actor = _actors[id];
    if ( actor )
        actor->setLinearVelocity( btVector3(vec.x(), vec.y(), vec.z()) );
}

void BulletInterface::setMatrix( int id, const osg::Matrix& matrix )
{
    btRigidBody* actor = _actors[id];
    if ( actor )
    {
        btTransform trans;
        trans.setFromOpenGLMatrix( osg::Matrixf(matrix).ptr() );
        actor->setWorldTransform( trans );
    }
}

osg::Matrix BulletInterface::getMatrix( int id )
{
    btRigidBody* actor = _actors[id];
    if ( actor )
    {
        btTransform trans = actor->getWorldTransform();
        osg::Matrixf matrix;
        trans.getOpenGLMatrix( matrix.ptr() );
        return matrix;
    }
    return osg::Matrix();
}

void BulletInterface::simulate( double step )
{
    _scene->stepSimulation( step, 10 );
}
