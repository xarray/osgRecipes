#include <osg/Transform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/TriangleFunctor>
#include "RecastManager.h"

namespace
{

struct GeometryDataCollector : public osg::NodeVisitor
{
    GeometryDataCollector( const osg::Matrix& matrix )
    : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _offset(matrix) {}
    
    virtual void apply( osg::Transform& transform );
    virtual void apply( osg::Geode& node );
    virtual void apply( osg::Node& node ) { traverse(node); }
    
    inline void pushMatrix( osg::Matrix& matrix ) { _matrixStack.push_back(matrix); }
    inline void popMatrix() { _matrixStack.pop_back(); }
    
    typedef std::map<osg::Vec3, unsigned int> VertexMap;
    VertexMap vertexMap;
    std::vector<float> vertices;
    std::vector<int> faces;
    osg::BoundingBoxf bound;
    osg::Matrix _offset;
    
    typedef std::vector<osg::Matrix> MatrixStack;
    MatrixStack _matrixStack;
};

struct CollectFaceOperator
{
    void operator()( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool temp )
    {
        if ( v1==v2 || v2==v3 || v3==v1 ) return;
        collector->faces.push_back( getOrCreateVertex(v1 * matrix) );
        collector->faces.push_back( getOrCreateVertex(v2 * matrix) );
        collector->faces.push_back( getOrCreateVertex(v3 * matrix) );
    }
    
    unsigned int getOrCreateVertex( const osg::Vec3& p )
    {
        unsigned int index = 0;
        if ( collector->vertexMap.find(p)==collector->vertexMap.end() )
        {
            index = collector->vertexMap.size();
            collector->vertexMap[p] = index;
            collector->vertices.push_back( p[0] );
            collector->vertices.push_back( p[1] );
            collector->vertices.push_back( p[2] );
            collector->bound.expandBy( p );
        }
        else
            index = collector->vertexMap[p];
        return index;
    }
    
    GeometryDataCollector* collector;
    osg::Matrix matrix;
};

void GeometryDataCollector::apply( osg::Transform& transform )
{
    osg::Matrix matrix;
    if ( !_matrixStack.empty() ) matrix = _matrixStack.back();
    transform.computeLocalToWorldMatrix( matrix, this );

    pushMatrix( matrix );
    traverse( transform );
    popMatrix();
}

void GeometryDataCollector::apply( osg::Geode& node )
{
    osg::Matrix matrix = _offset;
    if ( _matrixStack.size()>0 ) matrix *= _matrixStack.back();
    for ( unsigned int i=0; i<node.getNumDrawables(); ++i )
    {
        osg::Geometry* geom = node.getDrawable(i)->asGeometry();
        if ( geom )
        {
            osg::TriangleFunctor<CollectFaceOperator> functor;
            functor.collector = this;
            functor.matrix = matrix;
            geom->accept( functor );
        }
        else
        {
            osg::TriangleFunctor<CollectFaceOperator> functor;
            functor.collector = this;
            functor.matrix = matrix;
            node.getDrawable(i)->accept( functor );
        }
    }
    traverse( node );
}

}

/* RecastManager */

RecastManager::RecastManager( const osg::Matrix& matrix )
:   _chunkyMesh(NULL), _mesh(NULL), _detailMesh(NULL),
    _navMesh(NULL), _navQuery(NULL), _crowd(NULL),
    _agentHeight(2.0f), _agentRadius(0.6f), _agentMaxClimb(0.9f)
{
    _globalOffset = matrix;
    _globalOffsetInv = osg::Matrix::inverse(matrix);
    
    memset( &_config, 0, sizeof(_config) );
    _config.cs = 0.3f;
    _config.ch = 0.2f;
    _config.walkableSlopeAngle = 45.0f;
    _config.walkableHeight = (int)ceilf(_agentHeight / _config.ch);
    _config.walkableClimb = (int)floorf(_agentRadius / _config.ch);
    _config.walkableRadius = (int)ceilf(_agentMaxClimb / _config.cs);
    _config.maxEdgeLen = (int)(12.0f / _config.cs);
    _config.maxSimplificationError = 1.3f;
    _config.minRegionArea = (int)rcSqr(8.0f);        // Note: area = size*size
    _config.mergeRegionArea = (int)rcSqr(20.0f);    // Note: area = size*size
    _config.maxVertsPerPoly = (int)6.0f;
    _config.detailSampleDist = _config.cs * 6.0f;
    _config.detailSampleMaxError = _config.ch * 1.0f;
}

RecastManager::~RecastManager()
{
    destroy( true );
}

bool RecastManager::buildScene( osg::Node* node, int maxAgents, int chunkSize )
{
    if ( !node || chunkSize<=0 ) return false;
    GeometryDataCollector collector(_globalOffset);
    node->accept( collector );
    
    // Create mesh data
    unsigned int numVertices = collector.vertices.size() / 3;
    unsigned int numTriangles = collector.faces.size() / 3;
    if ( !numVertices || !numTriangles ) return false;
    if ( _chunkyMesh ) delete _chunkyMesh;
    
    _chunkyMesh = new rcChunkyTriMesh;
    rcCalcBounds( &(collector.vertices[0]), numVertices, _meshBoundMin, _meshBoundMax );
    if ( !rcCreateChunkyTriMesh(&(collector.vertices[0]), &(collector.faces[0]),
                                numTriangles, chunkSize, _chunkyMesh) )
    {
        OSG_NOTICE << "[RecastManager] Failed to build chunky mesh" << std::endl;
        return false;
    }
    
    // Initialize build _configure
    rcVcopy( _config.bmin, _meshBoundMin );
    rcVcopy( _config.bmax, _meshBoundMax );
    rcCalcGridSize( _config.bmin, _config.bmax, _config.cs, &_config.width, &_config.height );
    destroy( false );
    
    // Rasterize input polygon soup
    rcHeightfield* solid = rcAllocHeightfield();
    if ( !rcCreateHeightfield(&_context, *solid, _config.width, _config.height,
                              _config.bmin, _config.bmax, _config.cs, _config.ch) )
    {
        OSG_NOTICE << "[RecastManager] Could not create solid height-field" << std::endl;
        return false;
    }
    
    unsigned char* triAreas = new unsigned char[numTriangles];
    memset( triAreas, 0, numTriangles * sizeof(unsigned char) );
    rcMarkWalkableTriangles( &_context, _config.walkableSlopeAngle, &(collector.vertices[0]), numVertices,
                             &(collector.faces[0]), numTriangles, triAreas );
    rcRasterizeTriangles( &_context, &(collector.vertices[0]), numVertices,
                          &(collector.faces[0]), triAreas, numTriangles, *solid, _config.walkableClimb );
    delete[] triAreas;
    
    // Filter walkable surfaces
    rcFilterLowHangingWalkableObstacles( &_context, _config.walkableClimb, *solid );
    rcFilterLedgeSpans( &_context, _config.walkableHeight, _config.walkableClimb, *solid );
    rcFilterWalkableLowHeightSpans( &_context, _config.walkableHeight, *solid );
    
    // Partition walkable surface to simple regions
    rcCompactHeightfield* compact = rcAllocCompactHeightfield();
    if ( !rcBuildCompactHeightfield(&_context, _config.walkableHeight,
                                    _config.walkableClimb, *solid, *compact) )
    {
        OSG_NOTICE << "[RecastManager] Could not build compact data" << std::endl;
        return false;
    }
    rcFreeHeightField( solid );
    
    // Erode the walkable area by agent radius
    if ( !rcErodeWalkableArea(&_context, _config.walkableRadius, *compact) )
    {
        OSG_NOTICE << "[RecastManager] Could not erode compact data" << std::endl;
        return false;
    }
    
    // (Optional) Mark areas
    // TODO
    //rcMarkConvexPolyArea( &_context, subVerts, numSubVerts, subHmin, subHmax, subArea, *compact );
    
    // Use the classic Watershed partitioning for large open areas
    if ( !rcBuildDistanceField(&_context, *compact) )
    {
        OSG_NOTICE << "[RecastManager] Could not build distance field" << std::endl;
        return false;
    }
    
    if ( !rcBuildRegions(&_context, *compact, 0, _config.minRegionArea, _config.mergeRegionArea) )
    {
        OSG_NOTICE << "[RecastManager] Could not build watershed regions" << std::endl;
        return false;
    }
    
    // Trace and simplify region contours
    rcContourSet* contourSet = rcAllocContourSet();
    if ( !rcBuildContours(&_context, *compact, _config.maxSimplificationError,
                          _config.maxEdgeLen, *contourSet) )
    {
        OSG_NOTICE << "[RecastManager] Could not create contours" << std::endl;
        return false;
    }
    
    // Build polygons mesh from contours
    _mesh = rcAllocPolyMesh();
    if ( !rcBuildPolyMesh(&_context, *contourSet, _config.maxVertsPerPoly, *_mesh) )
    {
        OSG_NOTICE << "[RecastManager] Could not triangulate contours" << std::endl;
        return false;
    }
    
    // Create detail mesh which allows to access approximate height on each polygon
    _detailMesh = rcAllocPolyMeshDetail();
    if (!rcBuildPolyMeshDetail(&_context, *_mesh, *compact, _config.detailSampleDist,
                               _config.detailSampleMaxError, *_detailMesh) )
    {
        OSG_NOTICE << "[RecastManager] Could not build detail mesh" << std::endl;
        return false;
    }
    rcFreeCompactHeightfield( compact );
    rcFreeContourSet( contourSet );
    
    // Only build the detour nav-mesh if we do not exceed the limit of maximum points per polygon
    if ( _config.maxVertsPerPoly>DT_VERTS_PER_POLYGON )
    {
        OSG_NOTICE << "[RecastManager] Too many points per polygon for Detour to handle" << std::endl;
        return false;
    }
    
    // Update poly flags from areas (which can be marked before optionally)
    for ( int i=0; i<_mesh->npolys; ++i )
    {
        if ( _mesh->areas[i]==RC_WALKABLE_AREA )
            _mesh->flags[i] = POLYFLAGS_WALK;
        
        /*if ( _mesh->areas[i]==POLYAREA_WATER )
            _mesh->flags[i] = POLYFLAGS_SWIM;
        else if ( _mesh->areas[i]==POLYAREA_DOOR )
            _mesh->flags[i] = POLYFLAGS_WALK | POLYFLAGS_DOOR;*/
    }
    
    // Create Detour data from Recast poly mesh
    dtNavMeshCreateParams params;
    memset( &params, 0, sizeof(params) );
    params.verts = _mesh->verts;
    params.vertCount = _mesh->nverts;
    params.polys = _mesh->polys;
    params.polyAreas = _mesh->areas;
    params.polyFlags = _mesh->flags;
    params.polyCount = _mesh->npolys;
    params.nvp = _mesh->nvp;
    params.detailMeshes = _detailMesh->meshes;
    params.detailVerts = _detailMesh->verts;
    params.detailVertsCount = _detailMesh->nverts;
    params.detailTris = _detailMesh->tris;
    params.detailTriCount = _detailMesh->ntris;
    /*params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
    params.offMeshConRad = m_geom->getOffMeshConnectionRads();
    params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
    params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
    params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
    params.offMeshConUserID = m_geom->getOffMeshConnectionId();
    params.offMeshConCount = m_geom->getOffMeshConnectionCount();*/
    params.walkableHeight = _agentHeight;
    params.walkableRadius = _agentRadius;
    params.walkableClimb = _agentMaxClimb;
    params.cs = _config.cs;
    params.ch = _config.ch;
    params.buildBvTree = true;
    rcVcopy( params.bmin, _mesh->bmin );
    rcVcopy( params.bmax, _mesh->bmax );
    
    unsigned char* navData = NULL;
    int navDataSize = 0;
    if ( !dtCreateNavMeshData(&params, &navData, &navDataSize) )
    {
        OSG_NOTICE << "[RecastManager] Could not build Detour navmesh" << std::endl;
        return false;
    }
    
    _navMesh = dtAllocNavMesh();
    dtStatus status = _navMesh->init( navData, navDataSize, DT_TILE_FREE_DATA );
    if ( dtStatusFailed(status) )
    {
        OSG_NOTICE << "[RecastManager] Could not initialize Detour navmesh" << std::endl;
        dtFree( navData );
        return false;
    }
    
    _navQuery = dtAllocNavMeshQuery();
    status = _navQuery->init( _navMesh, 2048 );
    if ( dtStatusFailed(status) )
    {
        OSG_NOTICE << "[RecastManager] Could not initialize Detour navmesh query" << std::endl;
        dtFree( navData );
        return false;
    }
    
    // Initialize crowd data
    _crowd = dtAllocCrowd();
    if ( !_crowd->init(maxAgents, _agentRadius, _navMesh) )
    {
        OSG_NOTICE << "[RecastManager] Could not initialize Detour crowd" << std::endl;
        return false;
    }
    
    // Make polygons with 'disabled' flag invalid
    _crowd->getEditableFilter(0)->setExcludeFlags( POLYFLAGS_DISABLED );
    
    // Setup local avoidance params to different qualities
    {
        dtObstacleAvoidanceParams params;
        memcpy( &params, _crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams) );
        
        // Low (11)
        params.velBias = 0.5f; params.adaptiveDivs = 5;
        params.adaptiveRings = 2; params.adaptiveDepth = 1;
        _crowd->setObstacleAvoidanceParams( 0, &params );
        
        // Medium (22)
        params.velBias = 0.5f; params.adaptiveDivs = 5; 
        params.adaptiveRings = 2; params.adaptiveDepth = 2;
        _crowd->setObstacleAvoidanceParams( 1, &params );
        
        // Good (45)
        params.velBias = 0.5f; params.adaptiveDivs = 7;
        params.adaptiveRings = 2; params.adaptiveDepth = 3;
        _crowd->setObstacleAvoidanceParams( 2, &params );
        
        // High (66)
        params.velBias = 0.5f; params.adaptiveDivs = 7;
        params.adaptiveRings = 3; params.adaptiveDepth = 3;
        _crowd->setObstacleAvoidanceParams( 3, &params );
    }
    return true;
}

void RecastManager::destroy( bool includeGeom )
{
    if ( includeGeom )
    {
        if ( _chunkyMesh ) delete _chunkyMesh;
        _chunkyMesh = NULL;
    }
    
    rcFreePolyMesh(_mesh); _mesh = NULL;
    rcFreePolyMeshDetail(_detailMesh); _detailMesh = NULL;
    dtFreeNavMesh(_navMesh); _navMesh = NULL;
    dtFreeNavMeshQuery(_navQuery); _navQuery = NULL;
    dtFreeCrowd(_crowd); _crowd = NULL;
}

void RecastManager::update( float deltaTime )
{
    if ( !_navMesh || !_crowd ) return;
    _crowd->update( deltaTime, NULL );
    
    for ( AgentDataMap::iterator itr=_agentMap.begin();
          itr!=_agentMap.end(); ++itr )
    {
        const dtCrowdAgent* ag = _crowd->getAgent( itr->second.id );
        const float* pos = ag->npos;
        
        osg::Matrix matrix = itr->first->getMatrix();
        matrix.setTrans( osg::Vec3(pos[0], pos[1], pos[2]) * _globalOffsetInv );
        itr->first->setMatrix( matrix );
    }
}

int RecastManager::addAgent( const osg::Vec3f& pos, osg::MatrixTransform* node,
                             float maxSpeed, float maxAcc )
{
    if ( !node ) return -1;
    if ( _agentMap.find(node)!=_agentMap.end() )
    {
        OSG_NOTICE << "[RecastManager] Already added agent node" << std::endl;
        return -1;
    }
    
    dtCrowdAgentParams ap;
    memset( &ap, 0, sizeof(ap) );
    ap.radius = _agentRadius;
    ap.height = _agentHeight;
    ap.maxAcceleration = maxAcc;
    ap.maxSpeed = maxSpeed;
    ap.collisionQueryRange = ap.radius * 12.0f;
    ap.pathOptimizationRange = ap.radius * 30.0f;
    ap.updateFlags = DT_CROWD_ANTICIPATE_TURNS|DT_CROWD_OPTIMIZE_VIS|
                     DT_CROWD_OPTIMIZE_TOPO|DT_CROWD_OBSTACLE_AVOIDANCE;
    ap.obstacleAvoidanceType = (unsigned char)3;
    ap.separationWeight = 2.0f;
    int id = _crowd->addAgent( (pos * _globalOffset).ptr(), &ap );
    if ( id!=-1 ) _agentMap[node] = AgentData(id);
    return id;
}

void RecastManager::removeAgent( osg::MatrixTransform* node )
{
    AgentDataMap::iterator itr = _agentMap.find(node);
    if ( itr!=_agentMap.end() )
        _crowd->removeAgent( itr->second.id );
}

void RecastManager::moveTo( const osg::Vec3f& pos, osg::MatrixTransform* node )
{
    const dtQueryFilter* filter = _crowd->getFilter(0);
    const float* ext = _crowd->getQueryExtents();
    
    dtPolyRef targetRef;
    float targetPos[3];
    _navQuery->findNearestPoly( (pos * _globalOffset).ptr(), ext, filter,
                                &targetRef, targetPos );
    if ( !node )
    {
        for ( int i=0; i<_crowd->getAgentCount(); ++i )
        {
            const dtCrowdAgent* ag = _crowd->getAgent(i);
            if ( ag && ag->active )
                _crowd->requestMoveTarget( i, targetRef, targetPos );
        }
    }
    else
    {
        AgentDataMap::iterator itr = _agentMap.find(node);
        if ( itr==_agentMap.end() ) return;
        
        const dtCrowdAgent* ag = _crowd->getAgent( itr->second.id );
        if ( ag && ag->active )
            _crowd->requestMoveTarget( itr->second.id, targetRef, targetPos );
    }
}
