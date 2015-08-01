#include <osg/io_utils>
#include <osg/MatrixTransform>
#include "FlexDrawable.h"

static const char* flexVertCode = {
    "void main() {\n"
    "    vec4 vertex = vec4(vec3(gl_Vertex), 1.0);\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vertex;\n"
    "}\n"
};

static const char* flexFragCode = {
    "void main() {\n"
    "    gl_FragColor = vec4(0.8, 0.8, 0.8, 1.0);\n"
    "}\n"
};

/* FlexUpdater */

class FlexUpdater : public osg::Drawable::CullCallback
{
public:
    static FlexUpdater* instance()
    {
        static osg::ref_ptr<FlexUpdater> s_instance = new FlexUpdater;
        return s_instance.get();
    }
    
    static void errorCallback( const char* msg, const char* file, int line )
    {
        OSG_WARN << "Flex Error: " << msg
                 << " (" << file << " - " << line << ")" << std::endl;
    }
    
    virtual bool cull( osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* ) const
    {
        const osg::FrameStamp* fs = nv->getFrameStamp();
        if ( fs )
        {
            float dt = 1.0f / 60.0f;
            if ( fs->getFrameNumber()<=_lastFrameNumber ) return false;
            else dt *= float(fs->getFrameNumber() - _lastFrameNumber);
            
            FlexDrawable* flex = static_cast<FlexDrawable*>( drawable );
            if ( flex && flex->getSolver() )
                flexUpdateSolver( flex->getSolver(), dt, 2, NULL );
            _lastFrameNumber = fs->getFrameNumber();
        }
        return false;
    }
    
protected:
    FlexUpdater()
    :   _lastFrameNumber(0)
    {
        FlexError err = flexInit( FLEX_VERSION, FlexUpdater::errorCallback );
        if ( err!=eFlexErrorNone )
        {
            OSG_WARN << "Flex Initialization Error: " << err << std::endl;
        }
    }
    
    virtual ~FlexUpdater()
    {
        flexShutdown();
    }
    
    mutable unsigned int _lastFrameNumber;
};

/* FlexDrawable::ConvexManager */

void FlexDrawable::ConvexManager::add( const osg::Vec3& pos, const osg::Quat& q,
                                       const std::vector<osg::Plane>& faces,
                                       const osg::BoundingBox& bb, bool isStatic )
{
    unsigned int startIndex = starts.size();
    for ( unsigned int i=0; i<faces.size(); ++i )
    {
        const osg::Plane& plane = faces[i];
        planes.push_back( osg::Vec4f(plane[0], plane[1], plane[2], plane[3]) );
    }
    
    starts.push_back( startIndex );
    lengths.push_back( faces.size() );
    flags.push_back( isStatic ? 0 : 1 );
    
    positions.push_back( osg::Vec4(pos, 0.0f) );
    rotations.push_back( q.asVec4() );
    prevPositions.push_back( positions.back() );
    prevRotations.push_back( rotations.back() );
    
    aabbMin.push_back( osg::Vec4(bb._min, 0.0f) );
    aabbMax.push_back( osg::Vec4(bb._max, 0.0f) );
}

void FlexDrawable::ConvexManager::clear()
{
    positions.clear(); rotations.clear();
    prevPositions.clear(); prevRotations.clear();
    planes.clear(); aabbMin.clear(); aabbMax.clear();
    starts.clear(); lengths.clear(); flags.clear();
}

/* FlexDrawable::RigidManager */

void FlexDrawable::RigidManager::add( FlexDrawable* flex,
                                      const std::vector<FlexDrawable::Particle>& particles,
                                      float stiffness, int phase )
{
    unsigned int startIndex = flex->getPositions().size();
    if ( indices.empty() ) offsets.push_back( 0 );
    
    for ( unsigned int i=0; i<particles.size(); ++i )
        indices.push_back( int(startIndex + i) );
    flex->prepareParticles( particles, phase );
    
    coefficients.push_back( stiffness );
    offsets.push_back( int(indices.size()) );
}

void FlexDrawable::RigidManager::clear()
{
    offsets.clear(); indices.clear(); coefficients.clear();
    translations.clear(); localPositions.clear(); localNormals.clear();
    rotations.clear();
}

/* FlexDrawable */

FlexDrawable::FlexDrawable()
:   _solver(NULL)
{
    setUseDisplayList( false );
    setUseVertexBufferObjects( false );
    setCullCallback( FlexUpdater::instance() );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, flexVertCode) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, flexFragCode) );
    
    osg::StateSet* ss = getOrCreateStateSet();
    ss->setAttributeAndModes( program.get() );
}

FlexDrawable::~FlexDrawable()
{
    if ( _solver ) flexDestroySolver( _solver );
}

FlexParams FlexDrawable::createDefaultParameters()
{
    FlexParams localParams;
    localParams.mGravity[0] = 0.0f;
    localParams.mGravity[1] = 0.0f;
    localParams.mGravity[2] = -9.8f;
    localParams.mWind[0] = 0.0f;
    localParams.mWind[1] = 0.0f;
    localParams.mWind[2] = 0.0f;
    
    localParams.mRadius = 0.15f;
    localParams.mViscosity = 0.0f;
    localParams.mDynamicFriction = 0.0f;
    localParams.mStaticFriction = 0.0f;
    localParams.mParticleFriction = 0.0f;
    localParams.mFreeSurfaceDrag = 0.0f;
    localParams.mDrag = 0.0f;
    localParams.mLift = 0.0f;
    localParams.mNumIterations = 3;
    localParams.mFluidRestDistance = 0.0f;
    localParams.mSolidRestDistance = 0.0f;
    localParams.mAnisotropyScale = 1.0f;
    localParams.mDissipation = 0.0f;
    localParams.mDamping = 0.0f;
    localParams.mParticleCollisionMargin = 0.0f;
    localParams.mShapeCollisionMargin = 0.0f;
    localParams.mCollisionDistance = 0.0f;
    localParams.mPlasticThreshold = 0.0f;
    localParams.mPlasticCreep = 0.0f;
    localParams.mFluid = false;
    localParams.mSleepThreshold = 0.0f;
    localParams.mShockPropagation = 0.0f;
    localParams.mRestitution = 0.0f;
    localParams.mSmoothing = 1.0f;
    localParams.mMaxVelocity = FLT_MAX;
    localParams.mRelaxationMode = eFlexRelaxationLocal;
    localParams.mRelaxationFactor = 1.0f;
    localParams.mSolidPressure = 1.0f;
    localParams.mAdhesion = 0.0f;
    localParams.mCohesion = 0.025f;
    localParams.mSurfaceTension = 0.0f;
    localParams.mVorticityConfinement = 0.0f;
    localParams.mBuoyancy = 1.0f;
    localParams.mDiffuseThreshold = 100.0f;
    localParams.mDiffuseBuoyancy = 1.0f;
    localParams.mDiffuseDrag = 0.8f;
    localParams.mDiffuseBallistic = 16;
    localParams.mDiffuseSortAxis[0] = 0.0f;
    localParams.mDiffuseSortAxis[1] = 0.0f;
    localParams.mDiffuseSortAxis[2] = 0.0f;
    localParams.mEnableCCD = false;
    
    // update collision planes
    localParams.mNumPlanes = 1;
    localParams.mPlanes[0][0] = 0.0f;
    localParams.mPlanes[0][1] = 0.0f;
    localParams.mPlanes[0][2] = 1.0f;
    localParams.mPlanes[0][3] = 0.0f;
    return localParams;
}

void FlexDrawable::prepareParticles( const std::vector<FlexDrawable::Particle>& particles, int phase )
{
    for ( unsigned int i=0; i<particles.size(); ++i )
    {
        const Particle& p = particles[i];
        _positions.push_back(
            osg::Vec4(p.pos[0], p.pos[1], p.pos[2], (p.mass>0.0f ? 1.0f/p.mass : 0.0f)) );
        _normals.push_back( osg::Vec4(p.normal[0], p.normal[1], p.normal[2], 0.0f) );
        _velocities.push_back( p.velocity );
        _phases.push_back( phase );
    }
}

void FlexDrawable::initialize( int extraParticles, int maxDiffuseParticles, FlexParams* params,
                               FlexDrawable::ConvexManager* convexes, FlexDrawable::RigidManager* rigids )
{
    unsigned int initedParticles = _positions.size();
    int maxParticles = (int)initedParticles + extraParticles;
    if ( _solver ) flexDestroySolver( _solver );
    _solver = flexCreateSolver( maxParticles, maxDiffuseParticles );
    
    // Setup parameters
    if ( !params )
    {
        FlexParams localParams = createDefaultParameters();
        flexSetParams( _solver, &localParams );
    }
    else
    {
        // by default solid particles use the maximum radius
        if ( params->mFluid && params->mSolidRestDistance==0.0f )
            params->mSolidRestDistance = params->mFluidRestDistance;
        else
            params->mSolidRestDistance = params->mRadius;
        
        // collision distance with shapes half the radius
        if ( params->mCollisionDistance==0.0f )
        {
            params->mCollisionDistance = params->mRadius * 0.5f;
            if ( params->mFluid )
                params->mCollisionDistance = params->mFluidRestDistance * 0.5f;
        }
        
        // default particle friction to 10% of shape friction
        if ( params->mParticleFriction==0.0f )
            params->mParticleFriction = params->mDynamicFriction * 0.1f;
        
        // add a margin for detecting contacts between particles and shapes
        if ( params->mShapeCollisionMargin==0.0f )
            params->mShapeCollisionMargin = params->mCollisionDistance * 0.25f;
        flexSetParams( _solver, params );
    }
    
    // Setup positions/velocities
    _positions.resize( maxParticles );
    _normals.resize( maxParticles );
    _velocities.resize( maxParticles );
    for ( int i=0; i<maxParticles; ++i )
    {
        osg::Vec3 normal = osg::Vec3(_normals[i].x(), _normals[i].y(), _normals[i].z());
        if ( normal.length2()>0.0f ) normal.normalize(); else normal = osg::Z_AXIS;
        _normals[i] = osg::Vec4(normal, 0.0f);
    }
    
    flexSetParticles( _solver, (float*)&_positions[0], initedParticles, eFlexMemoryHost );
    flexSetNormals( _solver, (float*)&_normals[0], initedParticles, eFlexMemoryHost );
    flexSetVelocities( _solver, (float*)&_velocities[0], initedParticles, eFlexMemoryHost );
    
    // Setup active indices
    _activeIndices.resize( maxParticles );
    for ( int i=0; i<maxParticles; ++i ) _activeIndices[i] = i;
    flexSetActive( _solver, &_activeIndices[0], initedParticles, eFlexMemoryHost );
    
    // Setup input convexes
    if ( convexes && !convexes->starts.empty() )
    {
        flexSetConvexes(
            _solver, (float*)&(convexes->aabbMin[0]), (float*)&(convexes->aabbMax[0]),
            (int*)&(convexes->starts[0]), (int*)&(convexes->lengths[0]), (float*)&(convexes->planes[0]),
            (float*)&(convexes->positions[0]), (float*)&(convexes->rotations[0]), 
            (float*)&(convexes->prevPositions[0]), (float*)&(convexes->prevRotations[0]),
            &(convexes->flags[0]), convexes->starts.size(), convexes->planes.size(), eFlexMemoryHost );
    }
    _convexes = convexes;
    
    // Setup input rigid bodies
    if ( rigids && !rigids->offsets.empty() )
    {
        // calculate local rest space positions
        unsigned int numRigids = rigids->offsets.size() - 1, count = 0;
        rigids->localPositions.resize( rigids->offsets.back() );
        for ( unsigned int r=0; r<numRigids; ++r )
        {
            const int startIndex = rigids->offsets[r], endIndex = rigids->offsets[r + 1];
            const int n = endIndex - startIndex;
            osg::Vec4 com, temp;
            for ( int i=startIndex; i<endIndex; ++i )
                com += _positions[ rigids->indices[i] ];
            com *= 1.0f / float(n);
            
            for ( int i=startIndex; i<endIndex; ++i )
            {
                temp = _positions[ rigids->indices[i] ] - com;
                rigids->localPositions[count++] = osg::Vec3(temp[0], temp[1], temp[2]);
            }
        }
        
        rigids->rotations.resize( numRigids );
        rigids->translations.resize( numRigids );
        flexSetRigids(
            _solver, &(rigids->offsets[0]), &(rigids->indices[0]), (float*)&(rigids->localPositions[0]),
            (rigids->localNormals.size() ? (float*)&(rigids->localNormals[0]) : NULL),
            &(rigids->coefficients[0]), (float*)&(rigids->rotations[0]), numRigids, eFlexMemoryHost );
    }
    _rigids = rigids;
    
    // Setup phases
    flexSetPhases( _solver, &_phases[0], _phases.size(), eFlexMemoryHost );
}

void FlexDrawable::clear()
{
    _positions.resize( 0 );
    _normals.resize( 0 );
    _velocities.resize( 0 );
    _phases.resize( 0 );
}

void FlexDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    osg::State* state = renderInfo.getState();
    state->disableAllVertexArrays();
    
    // Make sure the client unit and active unit are unified
    state->setClientActiveTextureUnit( 0 );
    state->setActiveTextureUnit( 0 );
    if ( !_solver ) return;
    
    int numParticles = flexGetActiveCount(_solver);
    if ( 1 )  // CPU readback
    {
        flexGetParticles( _solver, (float*)&_positions[0], numParticles, eFlexMemoryHostAsync );
        flexGetVelocities( _solver, (float*)&_velocities[0], numParticles, eFlexMemoryHostAsync );

        // Wait for GPU to finish working (can perform async. CPU work here)
        flexSetFence();
        flexWaitFence();
        
        state->lazyDisablingOfVertexAttributes();
        state->setVertexPointer( 4, GL_FLOAT, 0, (float*)&_positions[0] );
        state->applyDisablingOfVertexAttributes();
        glDrawArrays( GL_POINTS, 0, numParticles );
    }
    else  // CUDA to OpenGL drawing
    {
        // TODO
    }
}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
osg::BoundingBox FlexDrawable::computeBoundingBox() const
#else
osg::BoundingBox FlexDrawable::computeBound() const
#endif
{
    return osg::BoundingBox();
}
