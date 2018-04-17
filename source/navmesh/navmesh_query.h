#ifndef _NAVMESH_QUERY_INC
#define _NAVMESH_QUERY_INC

#include "navmesh/recast/DetourNavMesh.h"
#include "navmesh/recast/DetourNavMeshQuery.h"
#include "navmesh/recast/DetourCommon.h"
#include "navmesh.h"

class CNavmeshQuery {
public:

  CNavmesh* data;

public:
  CNavmeshQuery( CNavmesh* anavmesh ) : data( anavmesh ) { }

  const std::vector<VEC3> findPath( VEC3 start, VEC3 end );
  float wallDistance( VEC3 pos );
  bool raycast( VEC3 start, VEC3 end, VEC3& hitPos );

private:

  // the dt data
  dtQueryFilter m_filter;

  static const int MAX_POLYS = 256;
  static const int MAX_SMOOTH = 2048;

  dtPolyRef m_polys[ MAX_POLYS ];
  dtPolyRef m_parent[ MAX_POLYS ];
  dtPolyRef m_straightPathPolys[ MAX_POLYS ];
  float m_straightPath[ MAX_POLYS * 3 ];
  float m_smoothPath[ MAX_SMOOTH * 3 ];
  unsigned char m_straightPathFlags[ MAX_POLYS ];
  float m_queryPoly[ 4 * 3 ];

  static const int MAX_RAND_POINTS = 64;
  float m_randPoints[ MAX_RAND_POINTS * 3 ];

  bool m_hitResult;
  float m_distanceToWall;
  float m_neighbourhoodRadius;
  float m_randomRadius;
  dtPolyRef m_pathIterPolys[ MAX_POLYS ];
  int m_pathIterPolyCount;
};

#endif
