#include "PxTkStream.h"
#include "PhysXInterface.h"
#include <osg/TriangleIndexFunctor>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/GUIEventHandler>
#include <osgUtil/SmoothingVisitor>

using namespace physx;

struct TriangleCollector
{
    void operator()( unsigned int i1, unsigned int i2, unsigned int i3 )
    {
        if ( i1==i2 || i1==i3 || i2==i3 ) return;
        triangles.push_back( i1 );
        triangles.push_back( i2 );
        triangles.push_back( i3 );
    }
    std::vector<PxU32> triangles;
};

class SampleClothUpdater : public osgGA::GUIEventHandler
{
public:
    SampleClothUpdater( osg::Geometry* cloth=0 )
    : _cloth(0), _clothGeometry(cloth), _initialized(false)
    { _collisionPlane.normal = PxVec3(0.0f, 0.0f, 1.0f); _collisionPlane.distance = 0.0f; }
    
    void setClothGeometry( osg::Geometry* cloth ) { _clothGeometry = cloth; }
    void setInitialPose( const PxTransform& pose ) { _initialPose = pose; }
    
    void setCollisionPlane( const osg::Plane& p )
    { osg::Vec3 n = p.getNormal(); _collisionPlane.normal = PxVec3(n[0], n[1], n[2]); _collisionPlane.distance = p[3]; }
    
    void setCollisionData( const std::vector<PxClothCollisionSphere>& spheres, const std::vector<PxU32>& indices )
    { _collisionSpheres = spheres; _capsuleIndices = indices; }
    
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        PxPhysics* sdk = PhysXInterface::instance()->getPhysicsSDK();
        PxScene* scene = PhysXInterface::instance()->getScene();
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::QUIT_APPLICATION:
            if ( scene && _cloth ) scene->removeActor( *_cloth );
            break;
        case osgGA::GUIEventAdapter::FRAME:
            if ( !_initialized )
            {
                if ( sdk && scene )
                {
                    createClothActor( sdk, scene, _initialPose );
                    _initialized = true;
                }
            }
            else if ( _cloth )
            {
                PhysXInterface::instance()->simulate( 0.02 );
                updateClothGeometry();
            }
            break;
        default: break;
        }
        return false;
    }
    
    bool createClothActor( PxPhysics* sdk, PxScene* scene, const PxTransform& pose )
    {
        // Construct mesh
        PxClothMeshDesc meshDesc;
        meshDesc.setToDefault();
        if ( !_clothGeometry )
        {
            OSG_WARN << "No valid geometry found." << std::endl;
            return false;
        }
        
        osg::Vec3Array* va = dynamic_cast<osg::Vec3Array*>( _clothGeometry->getVertexArray() );
        if ( !va )
        {
            OSG_WARN << "No vertices for the cloth" << std::endl;
            return false;
        }
        
        // Get all points
        std::vector<PxVec3> points( va->size() );
        for ( unsigned int i=0; i<va->size(); ++i )
        {
            const osg::Vec3& v = (*va)[i];
            points[i] = PxVec3( v.x(), v.y(), v.z() );
        }
        
        // Get all triangles
        osg::TriangleIndexFunctor<TriangleCollector> tif;
        _clothGeometry->accept( tif );
        if ( !points.size() || !tif.triangles.size() )
        {
            OSG_WARN << "No topology data for the cloth" << std::endl;
            return false;
        }
        
        // Reset the geometry so it could be updated directly during simulation
        osg::DrawElementsUInt* de = new osg::DrawElementsUInt(GL_TRIANGLES, tif.triangles.size());
        for ( unsigned int i=0; i<de->size(); ++i ) (*de)[i] = tif.triangles[i];
        
        _clothGeometry->setUseDisplayList( false );
        _clothGeometry->setUseVertexBufferObjects( true );
        _clothGeometry->removePrimitiveSet( 0, _clothGeometry->getNumPrimitiveSets() );
        _clothGeometry->addPrimitiveSet( de );
        
        // Set up mesh data
        meshDesc.points.data = &(points.front());
        meshDesc.points.count = static_cast<PxU32>(points.size());
        meshDesc.points.stride = sizeof(PxVec3);
        meshDesc.triangles.data = &(tif.triangles.front());
        meshDesc.triangles.count = static_cast<PxU32>(tif.triangles.size() / 3);
        meshDesc.triangles.stride = sizeof(PxU32) * 3;
        
        // Cook cloth mesh
        PxClothFabric* fabric = cookClothMesh( sdk, scene, meshDesc );
        if ( !fabric )
        {
            OSG_WARN << "Can't create cloth fabric." << std::endl;
            return NULL;
        }
        
        // Create the cloth particle, then the actor
        PxClothParticle* clothParticles = (PxClothParticle*)malloc(
            sizeof(PxClothParticle) * meshDesc.points.count );
        const PxVec3* srcPoints = reinterpret_cast<const PxVec3*>( meshDesc.points.data );
        for ( PxU32 i=0; i<meshDesc.points.count; ++i )
        {
            clothParticles[i].pos = srcPoints[i]; 
            clothParticles[i].invWeight = 1.0f;
        }
        
        PxClothFlags flags = PxClothFlag::eSWEPT_CONTACT/* | PxClothFlag::eGPU*/;
        _cloth = sdk->createCloth( pose, *fabric, clothParticles, flags );
        if ( !_cloth )
        {
            OSG_WARN << "Can't create the final cloth" << std::endl;
            return false;
        }
        
        // Add collision data
        for ( unsigned int i=0; i<_collisionSpheres.size(); ++i )
        {
            _cloth->addCollisionSphere( _collisionSpheres[i] );
        }

        for ( unsigned int i=0; i<_capsuleIndices.size(); i+=2 )
        {
            _cloth->addCollisionCapsule( _capsuleIndices[i], _capsuleIndices[i+1] );
        }

        // Config the actor and add it to scene
        _cloth->addCollisionPlane( _collisionPlane );
        _cloth->addCollisionConvex( 1 );  // Convex references the first plane only
        createVirtualParticles( meshDesc, 4 );
        configureCloth();
        scene->addActor( *_cloth );
        return true;
    }
    
    void updateClothGeometry()
    {
        if ( !_clothGeometry ) return;
        osg::Vec3Array* va = dynamic_cast<osg::Vec3Array*>( _clothGeometry->getVertexArray() );
        if ( va )
        {
            PxClothParticleData* data = _cloth->lockParticleData();
            const PxClothParticle* clothParticles = data->particles;
            for ( unsigned int i=0; i<va->size(); ++i )
            {
                const PxClothParticle& cp = clothParticles[i];
                (*va)[i].set( cp.pos.x, cp.pos.y, cp.pos.z );
            }
            data->unlock();
            va->dirty();
        }
        osgUtil::SmoothingVisitor::smooth( *_clothGeometry );  // FIXME: not too efficient
        _clothGeometry->dirtyBound();
    }
    
protected:
    PxClothFabric* cookClothMesh( PxPhysics* sdk, PxScene* scene, PxClothMeshDesc& meshDesc )
    {
        PxTolerancesScale ts;
        PxCookingParams cp(ts);
        PxCooking* cooking = PxCreateCooking( PX_PHYSICS_VERSION, sdk->getFoundation(), cp );
        if ( !cooking )
        {
            OSG_WARN << "Unable to create cooking object." << std::endl;
            return NULL;
        }
        return PxClothFabricCreate( *sdk, meshDesc, scene->getGravity() );
    }
    
    void createVirtualParticles( PxClothMeshDesc& meshDesc, int numSamples )
    {
        static PxVec3 s_virtualParticleWeights[] = 
        { 
            // center point
            PxVec3(1.0f / 3, 1.0f / 3, 1.0f / 3),
            // center of sub triangles
            PxVec3(2.0f / 3, 1.0f / 6, 1.0f / 6),
            PxVec3(1.0f / 6, 2.0f / 3, 1.0f / 6),
            PxVec3(1.0f / 6, 1.0f / 6, 2.0f / 3),
            // edge mid points
            PxVec3(1.0f / 2, 1.0f / 2, 0.0f),
            PxVec3(0.0f, 1.0f / 2, 1.0f / 2),
            PxVec3(1.0f / 2, 0.0f, 1.0f / 2),
            // further subdivision of mid points
            PxVec3(1.0f / 4, 3.0f / 4, 0.0f),
            PxVec3(3.0f / 4, 1.0f / 4, 0.0f),
            PxVec3(0.0f, 1.0f / 4, 3.0f / 4),
            PxVec3(0.0f, 3.0f / 4, 1.0f / 4),
            PxVec3(1.0f / 4, 0.0f, 3.0f / 4),
            PxVec3(3.0f / 4, 0.0f, 1.0f / 4)
        };
        
        PxU32 numFaces = meshDesc.triangles.count;
        PxU8* triangles = (PxU8*)meshDesc.triangles.data;
        PxU32 numParticles = numFaces * numSamples;
        std::vector<PxU32> virtualParticleIndices;
        virtualParticleIndices.reserve( 4*numParticles );
        
        for ( PxU32 i=0; i<numFaces; ++i )
        {
            for ( int s=0; s<numSamples; ++s )
            {
                PxU32 v0, v1, v2;
                if ( meshDesc.flags & PxMeshFlag::e16_BIT_INDICES )
                {
                    PxU16* triangle = (PxU16*)triangles;
                    v0 = triangle[0];
                    v1 = triangle[1];
                    v2 = triangle[2];
                }
                else
                {
                    PxU32* triangle = (PxU32*)triangles;
                    v0 = triangle[0];
                    v1 = triangle[1];
                    v2 = triangle[2];
                }
                
                virtualParticleIndices.push_back( v0 );
                virtualParticleIndices.push_back( v1 );
                virtualParticleIndices.push_back( v2 );
                virtualParticleIndices.push_back( s );
            }
            triangles += meshDesc.triangles.stride;
        }
        _cloth->setVirtualParticles( numParticles, &(virtualParticleIndices.front()),
                                     numSamples, s_virtualParticleWeights );
    }
    
    void configureCloth()
    {
        _cloth->setSolverFrequency( 240.0f );
        _cloth->setInertiaScale( 0.5f );
        _cloth->setDampingCoefficient( physx::PxVec3(0.2f, 0.2f, 0.2f) );
        _cloth->setDragCoefficient( 0.1f );
        _cloth->setFrictionCoefficient( 0.5f );
        _cloth->setCollisionMassScale( 20.0f );
    }
    
    PxCloth* _cloth;
    osg::observer_ptr<osg::Geometry> _clothGeometry;
    std::vector<PxClothCollisionSphere> _collisionSpheres;
    std::vector<PxU32> _capsuleIndices;
    PxTransform _initialPose;
    PxClothCollisionPlane _collisionPlane;
    bool _initialized;
};

struct ClothCollisionSceneBuilder
{
    unsigned int addSphereJoint( const osg::Vec3& c, double r )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(new osg::Sphere(c, r * _displayScale)) );
        root->addChild( geode.get() );
        
        PxClothCollisionSphere sphere;
        sphere.pos = PxVec3(c[0], c[1], c[2]);
        sphere.radius = r;
        spheres.push_back( sphere );
        return spheres.size() - 1;
    }
    
    void addCapsule( unsigned int i1, unsigned int i2 )
    {
        const PxVec3& c1 = spheres[i1].pos; osg::Vec3 center1(c1.x, c1.y, c1.z);
        const PxVec3& c2 = spheres[i2].pos; osg::Vec3 center2(c2.x, c2.y, c2.z);
        double r1 = spheres[i1].radius * _displayScale, r2 = spheres[i2].radius * _displayScale;
        
        osg::Vec3 dir = center2 - center1; 
        double length = dir.normalize();
        osg::Quat quat; quat.makeRotate( osg::Z_AXIS, dir );
        
        osg::ref_ptr<osg::Vec3Array> va = new osg::Vec3Array;
        for ( unsigned int i=0; i<=16; ++i )
        {
            float angle = 2.0 * osg::PI * (float)i / 16.0;
            osg::Vec3 v1(r1 * cosf(angle), r1 * sinf(angle), 0.0f);
            osg::Vec3 v2(r2 * cosf(angle), r2 * sinf(angle), length);
            va->push_back( quat * v2 + center1 );
            va->push_back( quat * v1 + center1 );
        }
        
        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        geom->setVertexArray( va.get() );
        geom->addPrimitiveSet( new osg::DrawArrays(GL_QUAD_STRIP, 0, va->size()) );
        osgUtil::SmoothingVisitor::smooth( *geom );
        
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( geom.get() );
        root->addChild( geode.get() );
        
        indices.push_back( i1 );
        indices.push_back( i2 );
    }
    
    ClothCollisionSceneBuilder( osg::Group* r ) : root(r), _displayScale(0.9f) {}
    osg::observer_ptr<osg::Group> root;
    std::vector<PxClothCollisionSphere> spheres;
    std::vector<PxU32> indices;
    float _displayScale;
};

osg::Geometry* createSampleClothGeometry()
{
    std::map<int, int> indexMap;
    osg::ref_ptr<osg::Vec3Array> clothVertices = new osg::Vec3Array;
    int numHeight = 40, numLine = 100;
    for ( int z=0; z<numHeight; ++z )
    {
        for ( int i=0; i<numLine; ++i )
        {
            int index = i + z * numLine;
            indexMap[index] = clothVertices->size();
            if ( i<40 )
            {
                if ( !(i>10 && i<30 && z>15 && z<25) )
                    clothVertices->push_back( osg::Vec3(-4.0f, i*0.1f-2.0f, z*0.3f-5.0f) );
                else indexMap[index] = -1;
            }
            else if ( i<60 )
            {
                clothVertices->push_back( osg::Vec3(i*0.4-20.0f, 2.0f, z*0.3f-5.0f) );
            }
            else
            {
                if ( !(i>70 && i<90 && z>15 && z<25) )
                    clothVertices->push_back( osg::Vec3(4.0f, 8.0f-i*0.1f, z*0.3f-5.0f) );
                else indexMap[index] = -1;
            }
        }
    }
    
    osg::ref_ptr<osg::Geometry> cloth = new osg::Geometry;
    cloth->setVertexArray( clothVertices.get() );
    for ( int z=0; z<numHeight-1; ++z )
    {
        osg::ref_ptr<osg::DrawElementsUShort> de = new osg::DrawElementsUShort(GL_QUADS);
        for ( int i=0; i<numLine-1; ++i )
        {
            int id1 = indexMap[(i+0) + (z+0) * numLine], id2 = indexMap[(i+0) + (z+1) * numLine];
            int id3 = indexMap[(i+1) + (z+1) * numLine], id4 = indexMap[(i+1) + (z+0) * numLine];
            if ( id1>=0 && id2>=0 && id3>=0 && id4>=0 )
            {
                de->push_back( id1 ); de->push_back( id2 );
                de->push_back( id3 ); de->push_back( id4 );
            }
        }
        cloth->addPrimitiveSet( de.get() );
    }
    
    osgUtil::SmoothingVisitor::smooth( *cloth );
    return cloth.release();
}

osgGA::GUIEventHandler* createSampleCloth( osg::Group* root )
{
    PhysXInterface::instance()->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), osg::Vec3(0.0f, 0.0f,-9.8f) );
    ClothCollisionSceneBuilder builder(root);
    unsigned int i0 = builder.addSphereJoint( osg::Vec3(0.0f, 0.0f, 3.0f), 0.5f );
    unsigned int i1 = builder.addSphereJoint( osg::Vec3(0.0f, 0.0f, 0.0f), 1.0f );
    unsigned int i2 = builder.addSphereJoint( osg::Vec3(5.0f, 0.0f, 1.0f), 0.5f );
    unsigned int i3 = builder.addSphereJoint( osg::Vec3(-5.0f, 0.0f, 1.0f), 0.5f );
    builder.addCapsule( i0, i1 );
    builder.addCapsule( i1, i2 );
    builder.addCapsule( i1, i3 );
    //builder.addCapsule( i2, i3 );
    
    // FIXME: I notice that with PhysX 3.2, we can't have more than 4096 points for a cloth,
    // otherwise the simulation will be a mess... The same problem happens for PhysX's own example
    osg::Geometry* cloth = createSampleClothGeometry();
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( cloth );
    root->addChild( geode.get() );
    
    osg::ref_ptr<SampleClothUpdater> updater = new SampleClothUpdater(cloth);
    updater->setInitialPose( PxTransform::createIdentity() );
    updater->setCollisionPlane( osg::Plane(osg::Vec3(0.0f, 0.0f, 1.0f), 10.0f) );
    updater->setCollisionData( builder.spheres, builder.indices );
    return updater.release();
}
