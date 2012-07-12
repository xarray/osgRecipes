#ifndef H_BULLETINTERFACE
#define H_BULLETINTERFACE

#include <bullet/btBulletDynamicsCommon.h> 

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Plane>
#include <osg/Matrix>
#include <string>
#include <map>

class BulletInterface : public osg::Referenced
{
public:
    static BulletInterface* instance();
    btDiscreteDynamicsWorld* getScene() { return _scene; }
    
    void createWorld( const osg::Plane& plane, const osg::Vec3& gravity );
    void createBox( int id, const osg::Vec3& dim, double mass );
    void createSphere( int id, double radius, double mass );
    
    void setVelocity( int id, const osg::Vec3& pos );
    void setMatrix( int id, const osg::Matrix& matrix );
    osg::Matrix getMatrix( int id );
    
    void simulate( double step );
    
protected:
    BulletInterface();
    virtual ~BulletInterface();
    
    typedef std::map<int, btRigidBody*> ActorMap;
    ActorMap _actors;
    btDiscreteDynamicsWorld* _scene;
    btDefaultCollisionConfiguration* _configuration;
    btCollisionDispatcher* _dispatcher;
    btBroadphaseInterface* _overlappingPairCache;
    btSequentialImpulseConstraintSolver* _solver;
};

#endif
