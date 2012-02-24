/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 5 Recipe 12
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include <osg/ref_ptr>
#include "PhysXInterface"

PhysXInterface* PhysXInterface::instance()
{
    static osg::ref_ptr<PhysXInterface> s_registry = new PhysXInterface;
    return s_registry.get();
}

PhysXInterface::PhysXInterface()
:   _scene(NULL)
{
    NxPhysicsSDKDesc desc;
    NxSDKCreateError errorCode = NXCE_NO_ERROR;
    _physicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, NULL, desc, &errorCode);
    if ( !_physicsSDK ) 
    {
        OSG_WARN << "Unable to initialize the PhysX SDK, error code: "
                 << errorCode << std::endl;
    }
}

PhysXInterface::~PhysXInterface()
{
    if ( _scene )
    {
        for ( ActorMap::iterator itr=_actors.begin(); itr!=_actors.end(); ++itr )
            _scene->releaseActor( *(itr->second) );
        _physicsSDK->releaseScene( *_scene );
    }
    NxReleasePhysicsSDK( _physicsSDK );
}

void PhysXInterface::createWorld( const osg::Plane& plane, const osg::Vec3& gravity )
{
    NxSceneDesc sceneDesc;
    sceneDesc.gravity = NxVec3(gravity.x(), gravity.y(), gravity.z());
    _scene = _physicsSDK->createScene( sceneDesc );
    
    NxMaterial* defaultMaterial = _scene->getMaterialFromIndex(0);
    defaultMaterial->setRestitution( 0.5f );
    defaultMaterial->setStaticFriction( 0.5f );
    defaultMaterial->setDynamicFriction( 0.5f );
    
    // Create the ground plane
    NxPlaneShapeDesc shapeDesc;
    shapeDesc.normal = NxVec3(plane[0], plane[1], plane[2]);
    shapeDesc.d = plane[3];
    createActor( -1, &shapeDesc, NULL );
}

void PhysXInterface::createBox( int id, const osg::Vec3& dim, double mass )
{
    NxBoxShapeDesc shapeDesc; shapeDesc.dimensions = NxVec3(dim.x(), dim.y(), dim.z());
    NxBodyDesc bodyDesc; bodyDesc.mass = mass;
    createActor( id, &shapeDesc, &bodyDesc );
}

void PhysXInterface::createSphere( int id, double radius, double mass )
{
    NxSphereShapeDesc shapeDesc; shapeDesc.radius = radius;
    NxBodyDesc bodyDesc; bodyDesc.mass = mass;
    createActor( id, &shapeDesc, &bodyDesc );
}

void PhysXInterface::setVelocity( int id, const osg::Vec3& vec )
{
    NxActor* actor = _actors[id];
    actor->setLinearVelocity( NxVec3(vec.x(), vec.y(), vec.z()) );
}

void PhysXInterface::setMatrix( int id, const osg::Matrix& matrix )
{
    NxF32 d[16];
    for ( int i=0; i<16; ++i )
        d[i] = *(matrix.ptr() + i);
    NxMat34 nxMat; nxMat.setColumnMajor44( &d[0] );
    
    NxActor* actor = _actors[id];
    actor->setGlobalPose( nxMat );
}

osg::Matrix PhysXInterface::getMatrix( int id )
{
    float mat[16];
    NxActor* actor = _actors[id];
    actor->getGlobalPose().getColumnMajor44( mat );
    return osg::Matrix(&mat[0]);
}

void PhysXInterface::simulate( double step )
{
    _scene->simulate( step );
    _scene->flushStream();
    _scene->fetchResults( NX_RIGID_BODY_FINISHED, true );
}

void PhysXInterface::createActor( int id, NxShapeDesc* shape, NxBodyDesc* body )
{
    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack( shape );
    actorDesc.body = body;
    
    NxActor* actor = _scene->createActor( actorDesc );
    _actors[id] = actor;
}
