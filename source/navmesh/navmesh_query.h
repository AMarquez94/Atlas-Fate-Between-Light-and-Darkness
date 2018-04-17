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
};

#endif
