#ifndef _NAVMESH_INC
#define _NAVMESH_INC

#include "navmesh/recast/Recast.h"
#include "navmesh/recast/DetourNavMesh.h"
#include "navmesh/recast/DetourNavMeshQuery.h"
#include "navmesh/navmesh_render.h"

class CNavmesh {

public:

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
  DebugDrawGL*           m_draw;

  CNavmesh( );
  void loadAll(const char* path);
  void prepareQueries( );
  void destroy( );
  void render();
};

#endif