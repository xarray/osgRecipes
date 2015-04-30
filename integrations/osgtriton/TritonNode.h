#ifndef H_TRITONNODE
#define H_TRITONNODE

#include <osg/Version>
#include <osg/Geode>
#include <Triton.h>

namespace SilverLining
{
class Atmosphere;
}

/** The ocean rendering node */
class TritonNode : public osg::Geode
{
    class OceanDrawable : public osg::Drawable
    {
    public:
        virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
#if OSG_MIN_VERSION_REQUIRED(3,3,2)
        virtual osg::BoundingBox computeBoundingBox() const;
#else
        virtual osg::BoundingBox computeBound() const;
#endif
        
        OceanDrawable( TritonNode* s=NULL );
        OceanDrawable( const OceanDrawable& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
        META_Object( osg, OceanDrawable )
        
        struct OceanMeshData
        {
            osg::ref_ptr<osg::Vec3Array> vertices;
            osg::ref_ptr<osg::PrimitiveSet> primitiveSet;
            osg::BoundingBox boundingBox;
        };
        
        std::vector<OceanMeshData>& getOceanMeshList() { return _oceanMeshes; }
        const std::vector<OceanMeshData>& getOceanMeshList() const { return _oceanMeshes; }
        
        void dirtyEnvironmentMap() { _environmentMapHandle = 0; }
        
    protected:
        std::vector<OceanMeshData> _oceanMeshes;
        TritonNode* _triton;
        mutable void* _environmentMapHandle;
    };
    
    struct OceanUpdater : public osg::NodeCallback
    {
    public:
        virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );
    };
    
public:
    TritonNode( const char* licenseUser=NULL, const char* licenseKey=NULL );
    TritonNode( const TritonNode& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Node( osg, TritonNode )
    
    void addOceanMesh( osg::Vec3Array* vertices, osg::PrimitiveSet* primitiveSet );
    
    osg::Drawable* oceanDrawable() { return _oceanDrawable.get(); }
    const osg::Drawable* oceanDrawable() const { return _oceanDrawable.get(); }
    
    Triton::Environment* environment() { return _environment; }
    const Triton::Environment* environment() const { return _environment; }
    
    Triton::Ocean* ocean() { return _ocean; }
    const Triton::Ocean* ocean() const { return _ocean; }
    
    bool isOceanValid() const { return _initialized; }
    
    void setResourcePath( const std::string& path ) { _resourcePath = path; }
    const std::string& getResourcePath() const { return _resourcePath; }
    
    void setAtmosphere( SilverLining::Atmosphere* a ) { _atmosphere = a; }
    SilverLining::Atmosphere* atmosphere() { return _atmosphere; }
    const SilverLining::Atmosphere* atmosphere() const { return _atmosphere; }
    
    void setGlobalLight( osg::Light* l ) { _light = l; }
    osg::Light* getGlobalLight() { return _light.get(); }
    const osg::Light* getGlobalLight() const { return _light.get(); }
    
    // Called internally to start Triton environment
    virtual bool initializeTriton( osg::RenderInfo& renderInfo );
    
    // Called internally to update attached light according to SilverLining sky
    virtual void updateGlobalLight();
    
protected:
    virtual ~TritonNode();
    
    osg::observer_ptr<OceanDrawable> _oceanDrawable;
    osg::observer_ptr<osg::Light> _light;
    Triton::ResourceLoader* _resourceLoader;
    Triton::Environment* _environment;
    Triton::Ocean* _ocean;
    SilverLining::Atmosphere* _atmosphere;
    std::string _licenseName;
    std::string _licenseKey;
    std::string _resourcePath;
    bool _initialized;
};

/** Update callback to update a node as a ship/wake generator */
class WakeGeneratorCallback : public osg::NodeCallback
{
public:
    WakeGeneratorCallback( TritonNode* t );
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );
    
    void setTritonNode( TritonNode* t ) { _triton = t; dirty(); }
    TritonNode* getTritonNode() { return _triton.get(); }
    const TritonNode* getTritonNode() const { return _triton.get(); }
    
    Triton::WakeGenerator* getWakeGenerator() { return _wakeGenerator; }
    const Triton::WakeGenerator* getWakeGenerator() const { return _wakeGenerator; }
    
    Triton::WakeGeneratorParameters& getParameters() { return _parameters; }
    const Triton::WakeGeneratorParameters& getParameters() const { return _parameters; }
    
    void dirty() { _dirty = true; }
    
protected:
    osg::observer_ptr<TritonNode> _triton;
    Triton::WakeGenerator* _wakeGenerator;
    Triton::WakeGeneratorParameters _parameters;
    osg::Vec3 _lastPosition;
    double _lastFrameTime;
    bool _dirty;
};

#endif
