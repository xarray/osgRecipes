#ifndef H_FLEX_DRAWBLE
#define H_FLEX_DRAWBLE

#include <osg/Version>
#include <osg/Drawable>
#include <flex.h>

class FlexDrawable : public osg::Drawable
{
public:
    FlexDrawable();
    virtual ~FlexDrawable();
    
    FlexParams createDefaultParameters();
    FlexSolver* getSolver() { return _solver; }
    
    const std::vector<osg::Vec4f>& getPositions() const { return _positions; }
    const std::vector<osg::Vec4f>& getNormals() const { _normals; }
    const std::vector<osg::Vec3f>& getVelocities() const { _velocities; }
    
    struct Particle
    {
        osg::Vec3 pos, normal, velocity;
        float mass;
        
        Particle() : mass(0.0f) {}
        Particle( const osg::Vec3& p, float m=0.0f ) : pos(p), mass(m) {}
        Particle( const osg::Vec3& p, const osg::Vec3& v, float m ) : pos(p), velocity(v), mass(m) {}
        Particle( const osg::Vec3& p, const osg::Vec3& n,
                  const osg::Vec3& v, float m ) : pos(p), normal(n), velocity(v), mass(m) {}
    };
    
    struct ConvexManager : public osg::Referenced
    {
        std::vector<osg::Vec4f> positions, rotations;
        std::vector<osg::Vec4f> prevPositions, prevRotations;
        std::vector<osg::Vec4f> aabbMin, aabbMax;
        std::vector<osg::Vec4f> planes;
        std::vector<unsigned int> starts, lengths;
        std::vector<int> flags;
        
        void add( const osg::Vec3& pos, const osg::Quat& q, const std::vector<osg::Plane>& faces,
                  const osg::BoundingBox& bb, bool isStatic=true );
        void clear();
    };
    
    struct RigidManager : public osg::Referenced
    {
        std::vector<int> offsets, indices;
        std::vector<float> coefficients;
        std::vector<osg::Vec3> translations, localPositions, localNormals;
        std::vector<osg::Matrix3> rotations;
        
        void add( FlexDrawable* flex, const std::vector<Particle>& particles,
                  float stiffness, int phase );
        void clear();
    };
    
    /** Add particle points before initializing */
    void prepareParticles( const std::vector<Particle>& particles, int phase );
    
    /** Initialize the solver after preparing all scene elements */
    void initialize( int extraParticles, int maxDiffuseParticles, FlexParams* params=NULL,
                     ConvexManager* convexes=NULL, RigidManager* rigids=NULL );
    void clear();
    
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
    
#if OSG_MIN_VERSION_REQUIRED(3,3,2)
    virtual osg::BoundingBox computeBoundingBox() const;
#else
    virtual osg::BoundingBox computeBound() const;
#endif
    
protected:
    std::vector<osg::Vec4f> _positions, _normals;
    std::vector<osg::Vec3f> _velocities;
    std::vector<int> _activeIndices, _phases;
    
    osg::ref_ptr<ConvexManager> _convexes;
    osg::ref_ptr<RigidManager> _rigids;
    FlexSolver* _solver;
};

#endif
