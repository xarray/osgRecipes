#include <extensions/PxVisualDebuggerExt.h>
#include <osg/ref_ptr>
#include "PhysXInterface.h"

using namespace physx;

class ErrorCallback: public PxErrorCallback
{
public:
    virtual void reportError( PxErrorCode::Enum code, const char* message, const char* file, int line )
    {
        OSG_WARN << "Error " << code << " was found in (" << file << ", " << line << "): "
                 << message << std::endl;
    }
};
ErrorCallback errorHandler;
PxDefaultAllocator defaultAllocator;

PhysXInterface* PhysXInterface::instance()
{
    static osg::ref_ptr<PhysXInterface> s_registry = new PhysXInterface;
    return s_registry.get();
}

PhysXInterface::PhysXInterface()
:   _scene(NULL), _material(NULL)
{
    PxFoundation* foundation = PxCreateFoundation( PX_PHYSICS_VERSION, defaultAllocator, errorHandler );
    if ( !foundation )
    {
        OSG_WARN << "Unable to initialize PhysX foundation." << std::endl;
        return;
    }
    
    _physicsSDK = PxCreatePhysics( PX_PHYSICS_VERSION, *foundation, PxTolerancesScale() );
    if ( !_physicsSDK ) 
    {
        OSG_WARN << "Unable to initialize PhysX SDK." << std::endl;
        return;
    }
    
    if ( !PxInitExtensions(*_physicsSDK) )
    {
        OSG_WARN << "Unable to initialize PhysX extensions." << std::endl;
    }
    
#ifdef _DEBUG
    if ( _physicsSDK->getPvdConnectionManager() )
    {
        PxVisualDebuggerExt::createConnection(
            _physicsSDK->getPvdConnectionManager(), "localhost", 5425, 10000 );
    }
    else
    {
        OSG_WARN << "Unable to start the PhysX visual debugger." << std::endl;
    }
#endif
}

PhysXInterface::~PhysXInterface()
{
    if ( _scene )
    {
        for ( ActorMap::iterator itr=_actors.begin(); itr!=_actors.end(); ++itr )
            _scene->removeActor( *(itr->second) );
        _scene->release();
    }
    _physicsSDK->release();
}

void PhysXInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity )
{
    PxSceneDesc sceneDesc( _physicsSDK->getTolerancesScale() );
    sceneDesc.gravity = PxVec3(gravity[0], gravity[1], gravity[2]);
    sceneDesc.filterShader = &PxDefaultSimulationFilterShader;
    if ( !sceneDesc.cpuDispatcher )
    {
        PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
        if ( !mCpuDispatcher )
           OSG_WARN << "Failed to create default Cpu dispatcher." << std::endl;
        sceneDesc.cpuDispatcher = mCpuDispatcher;
    }
    
    _scene = _physicsSDK->createScene( sceneDesc );
    if ( !_scene )
    {
        OSG_WARN <<"Failed to create the physics world." << std::endl;
        return;
    }
    _scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f );
    _scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f );
    _material = _physicsSDK->createMaterial( 0.5, 0.5, 0.5 );
    
    // Create the ground
    osg::Quat q; q.makeRotate( osg::Vec3f(1.0f, 0.0f, 0.0f), osg::Vec3f(plane[0], plane[1], plane[2]) );
    PxTransform pose( PxVec3(0.0f, 0.0f, plane[3]), PxQuat(q[0], q[1], q[2], q[3]) );
    PxRigidStatic* actor = _physicsSDK->createRigidStatic( pose );
    if ( !actor )
    {
        OSG_WARN <<"Failed to create the static plane." << std::endl;
        return;
    }
    
    PxShape* shape = actor->createShape( PxPlaneGeometry(), *_material );
    if ( !shape )
    {
        OSG_WARN <<"Failed to create the plane shape." << std::endl;
        return;
    }
    _scene->addActor( *actor );
}

void PhysXInterface::createBox( int id, const osg::Vec3& dim, double density )
{
    PxBoxGeometry geometry( PxVec3(dim[0], dim[1], dim[2]) );
    PxRigidDynamic* actor = PxCreateDynamic( *_physicsSDK, PxTransform::createIdentity(), geometry, *_material, density );
    if ( actor )
    {
        actor->setAngularDamping( 0.75 );
        _scene->addActor( *actor );
        _actors[id] = actor;
    }
}

void PhysXInterface::createSphere( int id, double radius, double density )
{
    PxSphereGeometry geometry( radius );
    PxRigidDynamic* actor = PxCreateDynamic( *_physicsSDK, PxTransform::createIdentity(), geometry, *_material, density );
    if ( actor )
    {
        actor->setAngularDamping( 0.75 );
        _scene->addActor( *actor );
        _actors[id] = actor;
    }
}

void PhysXInterface::setVelocity( int id, const osg::Vec3& vec )
{
    PxRigidDynamic* actor = (PxRigidDynamic*)_actors[id];
    if ( actor )
        actor->setLinearVelocity( PxVec3(vec.x(), vec.y(), vec.z()) );
}

void PhysXInterface::setMatrix( int id, const osg::Matrix& matrix )
{
    PxReal d[16];
    for ( int i=0; i<16; ++i ) d[i] = *(matrix.ptr() + i);
    
    PxRigidActor* actor = _actors[id];
    if ( actor ) actor->setGlobalPose( PxTransform(PxMat44(d)) );
}

osg::Matrix PhysXInterface::getMatrix( int id )
{
    PxRigidActor* actor = _actors[id];
    if ( actor )
    {
        float m[16];
        PxMat44 pxMat( actor->getGlobalPose() );
        for ( int i=0; i<16; ++i ) m[i] = *(pxMat.front() + i);
        return osg::Matrix(&m[0]);
    }
    return osg::Matrix();
}

void PhysXInterface::simulate( double step )
{
    _scene->simulate( step );
    while( !_scene->fetchResults() ) { /* do nothing but wait */ }
}
