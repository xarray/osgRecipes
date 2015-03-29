#ifndef H_SPARKDRAWABLE
#define H_SPARKDRAWABLE

#include <osg/Texture>
#include <osg/Drawable>

#include <SPK.h>
#include <SPK_GL.h>
#include <iostream>

/** The spark drawable contains one or more particle systems of the same base type */
class SparkDrawable : public osg::Drawable
{
public:
    class DeferredSystemHandler : public osg::Drawable::UpdateCallback
    {
    public:
        virtual void update( osg::NodeVisitor* nv, osg::Drawable* drawable );
        
        struct PosAndRotate
        {
            SPK::Vector3D position;
            SPK::Vector3D rotationAxis;
            float rotationAngle;
        };
        std::vector<PosAndRotate> _newSystemsToAdd;
    };
    
    struct SortParticlesOperator
    {
        SortParticlesOperator( const osg::Vec3d& eye )
        { _eye.x = eye.x(); _eye.y = eye.y(); _eye.z = eye.z(); }
        
        virtual bool operator()( SPK::System* lhs, SPK::System* rhs )
        {
            return SPK::getSqrDist(lhs->getWorldTransformPos(), _eye) >
                   SPK::getSqrDist(rhs->getWorldTransformPos(), _eye);
        }
        
        SPK::Vector3D _eye;
    };
    
    struct ImageAttribute
    {
        osg::ref_ptr<osg::Image> image;
        GLuint type;
        GLuint clamp;
    };
    
    typedef std::vector<SPK::System*> ParticleSystemList;
    typedef std::map<std::string, ImageAttribute> TextureObjMap;
    typedef std::map<std::string, GLuint> TextureIDMap;
    typedef SPK::SPK_ID (*CreateBaseSystemFunc)( const TextureIDMap&, int width, int height );
    
public:
    SparkDrawable();
    SparkDrawable( const SparkDrawable& copy,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, SparkDrawable )
    
    bool isValid() const { return getBaseSystemID()!=SPK::NO_ID && _protoSystem!=NULL; }
    unsigned int getNumParticles() const;
    
    void setBaseSystemCreator( CreateBaseSystemFunc func, bool useProtoSystem=false )
    { _baseSystemCreator = func; _useProtoSystem = useProtoSystem; _dirty = true; }
    
    void setBaseSystemID( SPK::SPK_ID id ) { _baseSystemID = id; }
    SPK::SPK_ID getBaseSystemID() const { return _baseSystemID; }
    
    void setSortParticles( bool b ) { _sortParticles = b; }
    bool getSortParticles() const { return _sortParticles; }
    
    void setAutoUpdateBound( bool b ) { _autoUpdateBound = b; }
    bool getAutoUpdateBound() const { return _autoUpdateBound; }
    
    SPK::System* getProtoSystem() { return _protoSystem; }
    const SPK::System* getProtoSystem() const { return _protoSystem; }
    
    SPK::System* getParticleSystem( unsigned int i ) { return _particleSystems[i]; }
    const SPK::System* getParticleSystem( unsigned int i ) const { return _particleSystems[i]; }
    unsigned int getNumParticleSystems() const { return _particleSystems.size(); }
    
    void addExternalParticleSystem( SPK::System* system )
    { if (system) _particleSystems.push_back(system); }
    
    void destroyParticleSystem( SPK::System* system, bool removeFromList=true )
    {
        if (system) SPK_Destroy(system);
        if (removeFromList)
        {
            unsigned int i = 0, size = _particleSystems.size();
            for (; i<size; ++i) { if (_particleSystems[i]==system) break; }
            if (i<size) _particleSystems.erase(_particleSystems.begin() + i);
        }
    }
    
    void setGlobalTransformMatrix( const osg::Matrix& matrix, bool useOffset=false );
    void setTransformMatrix( SPK::System* system, const SPK::Vector3D& pos, const SPK::Vector3D& rot,
                             float angle, bool useOffset=false );
    
    /** Add a new system cloned from the base system and return the index for deferred use */
    unsigned int addParticleSystem( const osg::Vec3& p=osg::Vec3(), const osg::Quat& r=osg::Quat() );
    
    /** Add an image for the creator func to use, must be done before the creator func starting */
    void addImage( const std::string& name, osg::Image* image, GLuint type=GL_RGB, GLuint clamp=GL_CLAMP );
    
    /** Update the system, will be called by the SparkUpdatingHandler */
    virtual bool update( double currentTime, const osg::Vec3d& eye );
    
    virtual osg::BoundingSphere computeBound() const;
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
    
protected:
    virtual ~SparkDrawable();
    
    SPK::System* createParticleSystem( const SPK::Vector3D& pos, const SPK::Vector3D& rot, float angle );
    GLuint compileInternalTexture( osg::Image* image, GLuint type, GLuint clamp ) const;
    void convertData( osg::Image* image, GLuint type, unsigned int numCurrent, unsigned int numRequired ) const;
    
    TextureObjMap _textureObjMap;
    ParticleSystemList _particleSystems;
    CreateBaseSystemFunc _baseSystemCreator;
    mutable SPK::SPK_ID _baseSystemID;
    
    mutable SPK::System* _protoSystem;
    mutable unsigned int _activeContextID;
    double _lastTime;
    bool _sortParticles;
    bool _useProtoSystem;
    bool _autoUpdateBound;
    mutable bool _dirty;
};

#endif
