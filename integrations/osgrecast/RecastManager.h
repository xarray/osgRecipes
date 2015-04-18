#ifndef H_RECASTMANAGER
#define H_RECASTMANAGER

#include "Recast/Recast.h"
#include "Recast/DetourNavMesh.h"
#include "Recast/DetourNavMeshQuery.h"
#include "Recast/DetourNavMeshBuilder.h"
#include "Recast/DetourCrowd.h"
#include "ChunkyTriMesh.h"
#include <osg/MatrixTransform>

class RecastManager : public osg::Referenced
{
public:
    enum PolyFlags
    {
        POLYFLAGS_WALK = 0x01,        // Ability to walk (ground, grass, road)
        POLYFLAGS_SWIM = 0x02,        // Ability to swim (water).
        POLYFLAGS_DOOR = 0x04,        // Ability to move through doors.
        POLYFLAGS_JUMP = 0x08,        // Ability to jump.
        POLYFLAGS_DISABLED = 0x10,    // Disabled polygon
        POLYFLAGS_ALL = 0xffff        // All abilities.
    };
    
    /** We may have to set an offset matrix because Recast uses Y-axis as the height */
    RecastManager( const osg::Matrix& globalOffset );
    
    /** Build new navigation scene from node, all configurations should be done before this method */
    bool buildScene( osg::Node* node, int maxAgents=128, int chunkSize=256 );
    
    /** Destroy current scene */
    void destroy( bool includeGeom );
    
    /** Update scene data every frame */
    void update( float deltaTime );
    
    /** Add new agent and return its unique ID */
    int addAgent( const osg::Vec3f& pos, osg::MatrixTransform* node,
                  float maxSpeed=3.5f, float maxAcc=8.0f );
    
    /** Remove agent */
    void removeAgent( osg::MatrixTransform* node );
    
    /** Move specified agent (or NULL for all) to position */
    void moveTo( const osg::Vec3f& pos, osg::MatrixTransform* node=NULL );
    
protected:
    virtual ~RecastManager();
    
    rcContext _context;
    rcConfig _config;
    rcChunkyTriMesh* _chunkyMesh;
    rcPolyMesh* _mesh;
    rcPolyMeshDetail* _detailMesh;
    dtNavMesh* _navMesh;
    dtNavMeshQuery* _navQuery;
    dtCrowd* _crowd;
    osg::Matrix _globalOffset, _globalOffsetInv;
    float _meshBoundMin[3], _meshBoundMax[3];
    float _agentHeight, _agentRadius, _agentMaxClimb;
    
    struct AgentData
    {
        AgentData( int i=0 ) : id(i) {}
        int id;
    };
    typedef std::map<osg::MatrixTransform*, AgentData> AgentDataMap;
    AgentDataMap _agentMap;
};

#endif
