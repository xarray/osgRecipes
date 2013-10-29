#ifndef OSG_EFFECTCOMPOSITOR
#define OSG_EFFECTCOMPOSITOR

#include <osg/Geode>
#include <osg/Texture>
#include <osg/Program>
#include <osg/Camera>
#include <osgDB/Options>
#include <osgDB/XmlParser>

namespace osgFX
{


/** The compositor class integrates myltiple forward and deferred passes to create complex visual effects */
class EffectCompositor : public osg::Group
{
public:
    EffectCompositor();
    EffectCompositor( const EffectCompositor& copy, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Node( osgFX, EffectCompositor );
    
    enum PassType { FORWARD_PASS, DEFERRED_PASS };
    struct PassData
    {
        bool activated;
        PassType type;
        std::string name;
        osg::ref_ptr<osg::Camera> pass;
        
        PassData() : activated(true), type(FORWARD_PASS) {}
        
        PassData& operator=( const PassData& pd )
        {
            activated = pd.activated; type = pd.type;
            name = pd.name; pass = pd.pass;
            return *this;
        }
        
        /** Check if the pass works like an HUD camera to display results */
        bool isDisplayPass() const
        { return pass.valid() && pass->getRenderOrder()!=osg::Camera::PRE_RENDER; }
    };
    
    /** Set the render target implementation used for new pass by default */
    void setRenderTargetImplementation( osg::Camera::RenderTargetImplementation r ) { _renderTargetImpl = r; }
    osg::Camera::RenderTargetImplementation getRenderTargetImplementation() const { return _renderTargetImpl; }
    
    /** Set default output buffer size */
    void setRenderTargetResolution( const osg::Vec3& r ) { _renderTargetResolution = r; }
    const osg::Vec3& getRenderTargetResolution() const { return _renderTargetResolution; }
    
    /** Set current technique (pass list) */
    void setCurrentTechnique( const std::string& tech ) { _currentTechnique = tech; }
    
    /** Remove current technique (pass list) */
    const std::string& getCurrentTechnique() { return _currentTechnique; }
    
    typedef std::vector<PassData> PassList;
    typedef std::map<std::string, PassList> PassListMap;
    const PassListMap& getAllTechniques() const { return _passLists; }
    
    /** Create new pass and add it to the end of the pass list */
    osg::Camera* createNewPass( PassType type, const std::string& name );
    
    /** Remove a specified pass */
    bool removePass( const std::string& name );
    
    /** Clear all passes in current technique */
    void clearPassList( bool removeTechnique=false )
    { getPassList().clear(); if (removeTechnique) _passLists.erase(_currentTechnique); }
    
    /** Get a specified pass */
    bool getPassData( const std::string& name, PassData& data ) const;
    
    /** Set the index/priority of a specified pass */
    bool setPassIndex( const std::string& name, unsigned int index );
    
    /** Get the index/priority of a specified pass */
    unsigned int getPassIndex( const std::string& name ) const;
    
    /** Set if the pass should be activated (so to update its contents) */
    bool setPassActivated( const std::string& name, bool activated );
    
    /** Get if the pass should be activated (so to update its contents) */
    bool getPassActivated( const std::string& name ) const;
    
    /** Get number of passes in this compositor */
    unsigned int getNumPasses() const { return getPassList().size(); }
    
    /** Get pass list for current technique */
    PassList& getPassList() { return _passLists[_currentTechnique]; }
    const PassList& getPassList() const;
    
    /** Convenient method to obtain all cameras used in forward/deferred passes */
    osg::NodeList getCameras( PassType type ) const;
    
    /** Set a global texture/buffer object */
    bool setTexture( const std::string& name, osg::Texture* tex );
    
    /** Remove a global texture/buffer object */
    bool removeTexture( const std::string& name );
    
    /** Get a global texture/buffer object */
    osg::Texture* getTexture( const std::string& name );
    const osg::Texture* getTexture( const std::string& name ) const;
    
    typedef std::map<std::string, osg::ref_ptr<osg::Texture> > TextureMap;
    const TextureMap& getTextureMap() const { return _textureMap; }
    
    /** Set a global parameter object */
    bool setUniform( const std::string& name, osg::Uniform* uniform );
    
    /** Remove a global parameter object */
    bool removeUniform( const std::string& name );
    
    /** Get a global parameter object */
    osg::Uniform* getUniform( const std::string& name );
    const osg::Uniform* getUniform( const std::string& name ) const;
    
    typedef std::map<std::string, osg::ref_ptr<osg::Uniform> > UniformMap;
    const UniformMap& getUniformMap() const { return _uniformMap; }
    
    enum InbuiltUniformType
    {
        UNKNOWN_UNIFORM = 0,
        EYE_POSITION,
        VIEW_POINT,
        LOOK_VECTOR,
        UP_VECTOR,
        LEFT_VECTOR,
        VIEWPORT_X,
        VIEWPORT_Y,
        VIEWPORT_WIDTH,
        VIEWPORT_HEIGHT,
        WINDOW_MATRIX,
        INV_WINDOW_MATRIX,
        FRUSTUM_NEAR_PLANE,
        FRUSTUM_FAR_PLANE,
        SCENE_FOV_IN_RADIANS,
        SCENE_ASPECT_RATIO,
        SCENE_MODELVIEW_MATRIX,
        SCENE_INV_MODELVIEW_MATRIX,
        SCENE_PROJECTION_MATRIX,
        SCENE_INV_PROJECTION_MATRIX
    };
    
    /** Add an inbuilt uniform for automatical updating */
    void addInbuiltUniform( InbuiltUniformType t, osg::Uniform* u ) { _inbuiltUniforms.push_back(InbuiltUniformPair(t, u)); }
    
    /** Remove specified type of inbuilt uniform */
    void removeInbuiltUniform( InbuiltUniformType t );
    
    /** Remove an inbuilt uniform object */
    void removeInbuiltUniform( osg::Uniform* u );
    
    /** Remove all inbuilt uniforms */
    void clearInbuiltUniforms() { _inbuiltUniforms.clear(); }
    
    /** Check if an uniform is inbuilt */
    bool isInbuiltUniform( osg::Uniform* u ) const;
    
    typedef std::pair<InbuiltUniformType, osg::observer_ptr<osg::Uniform> > InbuiltUniformPair;
    typedef std::vector<InbuiltUniformPair> InbuiltUniformList;
    const InbuiltUniformList& getInbuiltUniforms() { return _inbuiltUniforms; }
    
    /** Set a global GLSL shader object */
    bool setShader( const std::string& name, osg::Shader* shader );
    
    /** Remove a global GLSL shader object */
    bool removeShader( const std::string& name );
    
    /** Get a global GLSL shader object */
    osg::Shader* getShader( const std::string& name );
    const osg::Shader* getShader( const std::string& name ) const;
    
    typedef std::map<std::string, osg::ref_ptr<osg::Shader> > ShaderMap;
    const ShaderMap& getShaderMap() const { return _shaderMap; }
    
    /** Get or create the quad for rendering deferred data */
    osg::Geode* getOrCreateQuad();
    
    /** Set computed near and far plane values, which can be set to inbuilt uniforms then
        The reason we don't use the main camera's zear/zfar is because the compositor will
        manage the scene using internal cameras, and won't return back znear/zfar to the
        main camera in osgUtil::CullVisitor
    */
    void setPreservedNearAndFar( unsigned int frame, double zn, double zf );
    void getPreservedNearAndFar( double& zn, double& zf ) { zn = _preservedZNear; zf = _preservedZFar; }
    
    /** Create a new pass from XML
        A typical definition is:
          <pass name="..." type="...">  <!-- or <forward_pass>, <deferred_pass> -->
            <shader>...</shader>
            <uniform>...</uniform>
            <texture unit="">...</texture>
            <input unit="">...</input>
            <output target="">...</output>
          </pass>
    */
    osg::Camera* createPassFromXML( osgDB::XmlNode* xmlNode );
    
    /** Create a global/local buffer or texture object from XML
        A typical definition of a buffer object is:
          <buffer name="..." type="...">
            <width>...</width>
            <height>...</height>
            <source_format>...</source_format>
            <internal_format>...</internal_format>
          </buffer>
        
        A typical definition of a texture object is:
          <texture name="..." type="...">
            <file>...</file>
            <wrap param="s">...</wrap>
          </texture>
    */
    osg::Texture* createTextureFromXML( osgDB::XmlNode* xmlNode, bool asGlobal );
    
    /** Create a global/local parameter object from XML
        A typical definition is:
          <uniform name="..." type="...">
            <value>...</value>  <!-- or <inbuilt_value> -->
          </uniform>
    */
    osg::Uniform* createUniformFromXML( osgDB::XmlNode* xmlNode, bool asGlobal );
    
    /** Create a global/local GLSL shader object from XML
        A typical definition is:
          <shader name="..." type="vertex">
            <source>...</source>  <!-- or <file> -->
          </shader>
    */
    osg::Shader* createShaderFromXML( osgDB::XmlNode* xmlNode, bool asGlobal );
    
    typedef std::map<std::string, osg::ref_ptr<osgDB::XmlNode> > XmlTemplateMap;
    
    /** Load effect data from XML, can be executed multiple times to load various data */
    bool loadFromXML( osgDB::XmlNode* xmlNode, XmlTemplateMap& templateMap, const osgDB::Options* options );
    
    /** Traverse the node and all its children */
    virtual void traverse( osg::NodeVisitor& nv );
    
protected:
    osg::Geode* createScreenQuad( float width, float height, float scale=1.0f );
    void traverseAllPasses( osg::NodeVisitor& nv )
    {
        PassList& passList = getPassList();
        for ( unsigned int i=0; i<passList.size(); ++i )
        {
            PassData& data = passList[i];
            if ( data.activated && data.pass.valid() )
                data.pass->accept( nv );
        }
    }
    
    PassListMap _passLists;
    TextureMap _textureMap;
    UniformMap _uniformMap;
    ShaderMap _shaderMap;
    InbuiltUniformList _inbuiltUniforms;
    std::string _currentTechnique;
    
    osg::ref_ptr<osg::Geode> _quad;
    osg::Vec3 _renderTargetResolution;
    osg::Camera::RenderTargetImplementation _renderTargetImpl;
    double _preservedZNear;
    double _preservedZFar;
    unsigned int _preservingNearFarFrameNumber;
};

/** Read effect compositor from the XML file/stream */
EffectCompositor* readEffectFile( const std::string& filename, const osgDB::Options* options=NULL );
EffectCompositor* readEffectStream( std::istream& stream, const osgDB::Options* options=NULL );


}

#endif
