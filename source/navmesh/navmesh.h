#ifndef _NAVMESH_INC
#define _NAVMESH_INC

#include <vector>
#include "navmesh/recast/Recast.h"
#include "navmesh/recast/DetourNavMesh.h"
#include "navmesh/recast/DetourNavMeshQuery.h"
#include "navmesh_builder.h"
#include "navmesh_input.h"
#include "navmesh_render.h"

class CNavmesh {
public:
  enum {
    FLAG_WALK = 0x01
  , FLAG_SWIM = 0x02
  , FLAG_DISABLED = 0x10
  , ALL_FLAGS = 0xffff
  };

  enum EDrawMode {
      NAVMESH_DRAW_NONE = 0
    , NAVMESH_DRAW_TRANS
    , NAVMESH_DRAW_BVTREE
    , NAVMESH_DRAW_NODES
    , NAVMESH_DRAW_INVIS
    , NAVMESH_DRAW_MESH
    //, NAVMESH_DRAW_VOXELS
    //, NAVMESH_DRAW_VOXELS_WALKABLE
    //, NAVMESH_DRAW_COMPACT
    //, NAVMESH_DRAW_COMPACT_DISTANCE
    //, NAVMESH_DRAW_COMPACT_REGIONS
    //, NAVMESH_DRAW_REGION_CONNECTIONS
    //, NAVMESH_DRAW_RAW_CONTOURS
    //, NAVMESH_DRAW_BOTH_CONTOURS
    , NAVMESH_DRAW_COUNTOURS
    , NAVMESH_DRAW_POLYMESH
    , NAVMESH_DRAW_POLYMESH_DETAILS
    , NAVMESH_DRAW_TYPE_COUNT
  };

  static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
  static const int NAVMESHSET_VERSION = 1;

  struct NavMeshSetHeader
  {
    int magic;
    int version;
    int numTiles;
    dtNavMeshParams params;
  };

  struct NavMeshTileHeader
  {
    dtTileRef tileRef;
    int dataSize;
  };

  dtNavMesh*            m_navMesh;
  dtNavMeshQuery*       m_navQuery;
  DebugDrawGL           m_draw;

private:
  rcHeightfield*        m_solid;
  rcCompactHeightfield* m_chf;
  rcContourSet*         m_cset;
  rcPolyMesh*           m_pmesh;
  rcConfig              m_cfg;
  rcPolyMeshDetail*     m_dmesh;
  rcBuildContext*       m_ctx;
  unsigned char*        m_triareas;

  rcBuildContext        m_context;

public:
  CNavmeshInput         m_input;
  EDrawMode             m_draw_mode;

public:
  CNavmesh( );
  void build( );
  dtNavMesh* create( const rcConfig& cfg );
  dtNavMesh* loadAll(const char* path);
  void prepareQueries( );
  void destroy( );
  void dumpLog( );
  void render( bool use_z_test );
};


#endif