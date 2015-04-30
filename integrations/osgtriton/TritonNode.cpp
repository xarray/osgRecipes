#include <osg/Depth>
#include <osgDB/FileNameUtils>
#include <osgUtil/CullVisitor>
#include "TritonNode.h"

#ifdef USE_SILVERLINING_SKY
#include "../osgsilverlining/SilverLiningNode.h"
#endif

/* TritonNode::OceanDrawable */

TritonNode::OceanDrawable::OceanDrawable( TritonNode* s )
:   _triton(s), _environmentMapHandle(0)
{}

TritonNode::OceanDrawable::OceanDrawable( const OceanDrawable& copy, const osg::CopyOp& copyop )
:   osg::Drawable(copy, copyop), _oceanMeshes(copy._oceanMeshes),
    _triton(copy._triton), _environmentMapHandle(copy._environmentMapHandle)
{}

void TritonNode::OceanDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    osg::State* state = renderInfo.getState();
    state->disableAllVertexArrays();
    
    _triton->initializeTriton( renderInfo );
#ifdef USE_SILVERLINING_SKY
    if ( !_environmentMapHandle )
    {
        if ( _triton->atmosphere() && _triton->environment() )
        {
            if ( _triton->atmosphere()->GetEnvironmentMap(_environmentMapHandle) )
                _triton->environment()->SetEnvironmentMap( (Triton::TextureHandle)_environmentMapHandle );
        }
    }
#endif
    
    if ( _triton->ocean() )
    {
        double time = renderInfo.getView()->getFrameStamp()->getSimulationTime();
        Triton::Ocean* ocean = _triton->ocean();
        Triton::Environment* environment = _triton->environment();
        if ( _oceanMeshes.size()>0 )
        {
            for ( unsigned int i=0; i<_oceanMeshes.size(); ++i )
            {
                const OceanMeshData& oceanMesh = _oceanMeshes[i];
                environment->SetSeaLevel( oceanMesh.boundingBox.zMin() );
                
                // Bind VBO before SetPatchShader()
                state->setVertexPointer( oceanMesh.vertices.get() );
                
                // Render mesh in an ocean favor
                ocean->SetPatchShader( time, 3 * sizeof(float), 0, false, NULL );
                oceanMesh.primitiveSet->draw( *state, true );
                ocean->UnsetPatchShader();
            }
            state->unbindVertexBufferObject();
            state->unbindElementBufferObject();
        }
        else
            ocean->Draw( time );
    }
    state->dirtyAllVertexArrays();
}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
osg::BoundingBox TritonNode::OceanDrawable::computeBoundingBox() const
#else
osg::BoundingBox TritonNode::OceanDrawable::computeBound() const
#endif
{
    osg::BoundingBox bbox;
    if ( _oceanMeshes.size()>0 )
    {
        for ( unsigned int i=0; i<_oceanMeshes.size(); ++i )
        {
            osg::Vec3Array* va = _oceanMeshes[i].vertices;
            if ( va && va->size() )
            {
                for ( unsigned int j=0; j<va->size(); ++j )
                    bbox.expandBy( (*va)[j] );
            }
        }
    }
    return bbox;
}

/* TritonNode::OceanUpdater */

void TritonNode::OceanUpdater::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    TritonNode* triton = static_cast<TritonNode*>( node );
    if ( triton )
    {
        if ( nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR )
        {
            if ( triton->isOceanValid() && nv->getFrameStamp() )
                triton->ocean()->UpdateSimulation( nv->getFrameStamp()->getSimulationTime() );
            triton->updateGlobalLight();
        }
        else if ( nv->getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
        {
            osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>( nv );
            if ( triton->isOceanValid() )
            {
                Triton::Environment* env = triton->environment();
                env->SetCameraMatrix( cv->getModelViewMatrix()->ptr() );
                env->SetProjectionMatrix( cv->getProjectionMatrix()->ptr() );
            }
        }
    }
    traverse( node, nv );
}

/* TritonNode */

TritonNode::TritonNode( const char* licenseUser, const char* licenseKey )
:   _resourceLoader(0), _environment(0), _ocean(0), _atmosphere(0),
    _licenseName(licenseUser), _licenseKey(licenseKey), _initialized(false)
{
    _oceanDrawable = new OceanDrawable(this);
    _oceanDrawable->setUseVertexBufferObjects( false );
    _oceanDrawable->setUseDisplayList( false );
    addDrawable( _oceanDrawable.get() );
    
    OceanUpdater* updater = new OceanUpdater;
    setUpdateCallback( updater );
    setCullCallback( updater );
    getOrCreateStateSet()->setAttribute( new osg::Program );
    
    const char* slPath = getenv( "TRITON_PATH" );
    if ( slPath )
        _resourcePath = osgDB::convertFileNameToNativeStyle(std::string(slPath) + "/resources/");
}

TritonNode::TritonNode( const TritonNode& copy, const osg::CopyOp& copyop )
:   osg::Geode(copy, copyop), _oceanDrawable(copy._oceanDrawable), _light(copy._light),
    _resourceLoader(copy._resourceLoader), _environment(copy._environment), _ocean(copy._ocean),
    _atmosphere(copy._atmosphere), _licenseName(copy._licenseName), _licenseKey(copy._licenseKey),
    _resourcePath(copy._resourcePath), _initialized(copy._initialized)
{}

TritonNode::~TritonNode()
{
    if ( _ocean ) delete _ocean;
    if ( _environment ) delete _environment;
    if ( _resourceLoader ) delete _resourceLoader;
}

void TritonNode::addOceanMesh( osg::Vec3Array* vertices, osg::PrimitiveSet* primitiveSet )
{
    OceanDrawable::OceanMeshData mesh;
    mesh.vertices = vertices;
    mesh.primitiveSet = primitiveSet;
    
    // Apply VBO
    vertices->setBinding( osg::Array::BIND_PER_VERTEX );
    if ( !vertices->getVertexBufferObject() )
        vertices->setVertexBufferObject( new osg::VertexBufferObject );
    
    // Apply EBO
    osg::DrawElements* de = primitiveSet->getDrawElements();
    if ( de )
    {
        if ( !de->getElementBufferObject() )
            de->setElementBufferObject( new osg::ElementBufferObject );
    }
    
    _oceanDrawable->getOceanMeshList().push_back( mesh );
    _oceanDrawable->dirtyBound();
}

bool TritonNode::initializeTriton( osg::RenderInfo& renderInfo )
{
    if ( _initialized ) return true;
    _resourceLoader = new Triton::ResourceLoader( _resourcePath.c_str() );
    _environment = new Triton::Environment;
    _environment->SetLicenseCode( _licenseName.c_str(), _licenseKey.c_str() );
    
    // Create an environment for the water, using an OpenGL 2.0 capable context
    Triton::EnvironmentError err = _environment->Initialize( Triton::FLAT_ZUP, Triton::OPENGL_2_0, _resourceLoader );
    if ( err!=Triton::SUCCEEDED )
    {
        std::cout << "Triton failed to initialize: " << err << std::endl;
        return false;
    }
    
    _ocean = Triton::Ocean::Create( _environment, Triton::JONSWAP );
    if ( !_ocean )
    {
        std::cout << "Unabel to create Triton ocean" << std::endl;
        return false;
    }
    _initialized = true;
    return true;
}

void TritonNode::updateGlobalLight()
{
    if ( _initialized && _light.valid() )
    {
        osg::Vec4 ambient = _light->getAmbient();
        osg::Vec4 diffuse = _light->getDiffuse();
        osg::Vec4 position = _light->getPosition();  // FIXME: should consider light's l2w matrix here?
        
        // Diffuse direction and color
        _environment->SetDirectionalLight( Triton::Vector3(position[0], position[1], position[2]),
                                           Triton::Vector3(diffuse[0], diffuse[1], diffuse[2]) );
        _environment->SetAmbientLight( Triton::Vector3(ambient[0], ambient[1], ambient[2]) );
    }
}

/* WakeGeneratorCallback */

WakeGeneratorCallback::WakeGeneratorCallback( TritonNode* t )
:   _triton(t), _lastFrameTime(0.0), _dirty(true)
{
}

void WakeGeneratorCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    bool firstRun = false;
    if ( _dirty )
    {
        if ( _wakeGenerator!=NULL )
        {
            delete _wakeGenerator;
            _wakeGenerator = NULL;
        }
        
        if ( _triton.valid() && _triton->ocean() )
        {
            _wakeGenerator = new Triton::WakeGenerator( _triton->ocean(), _parameters );
            _dirty = false;
            firstRun = true;
        }
    }
    
    if ( _wakeGenerator!=NULL )
    {
        osg::Vec3 dir, pos = node->getWorldMatrices()[0].getTrans();
        double velocity = 0.0, time = nv->getFrameStamp()->getSimulationTime();
        if ( !firstRun )
        {
            dir = pos - _lastPosition;
            velocity = dir.normalize();
            velocity /= (time > _lastFrameTime) ? (time - _lastFrameTime) : 1.0;
        }
        
        _wakeGenerator->Update( Triton::Vector3(pos[0], pos[1], pos[2]), Triton::Vector3(dir[0], dir[1], dir[2]), velocity, time );
        _lastPosition = pos;
        _lastFrameTime = time;
    }
    traverse( node, nv );
}
