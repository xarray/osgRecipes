#ifndef H_PHYSXINTERFACE
#define H_PHYSXINTERFACE

#include <PxPhysicsAPI.h> 
#include <extensions/PxExtensionsAPI.h> 

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Plane>
#include <osg/Matrix>
#include <string>
#include <map>

class PhysXInterface : public osg::Referenced
{
public:
    static PhysXInterface* instance();
    physx::PxPhysics* getPhysicsSDK() { return _physicsSDK; }
    physx::PxScene* getScene() { return _scene; }
    
    void createWorld( const osg::Plane& plane, const osg::Vec3& gravity );
    void createBox( int id, const osg::Vec3& dim, double mass );
    void createSphere( int id, double radius, double mass );
    
    void setVelocity( int id, const osg::Vec3& pos );
    void setMatrix( int id, const osg::Matrix& matrix );
    osg::Matrix getMatrix( int id );
    
    void simulate( double step );
    
protected:
    PhysXInterface();
    virtual ~PhysXInterface();
    
    typedef std::map<int, physx::PxRigidActor*> ActorMap;
    ActorMap _actors;
    physx::PxPhysics* _physicsSDK;
    physx::PxScene* _scene;
    physx::PxMaterial* _material;
};

#endif
