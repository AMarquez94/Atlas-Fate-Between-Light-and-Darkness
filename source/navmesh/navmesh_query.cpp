#include "mcv_platform.h"
#include "navmesh_query.h"

inline bool inRange( const float* v1, const float* v2, const float r, const float h ) {
  const float dx = v2[ 0 ] - v1[ 0 ];
  const float dy = v2[ 1 ] - v1[ 1 ];
  const float dz = v2[ 2 ] - v1[ 2 ];
  return (dx*dx + dz*dz) < r*r && fabsf( dy ) < h;
}

static int fixupCorridor( dtPolyRef* path, const int npath, const int maxPath,
                          const dtPolyRef* visited, const int nvisited ) {
  int furthestPath = -1;
  int furthestVisited = -1;

  // Find furthest common polygon.
  for( int i = npath - 1; i >= 0; --i ) {
    bool found = false;
    for( int j = nvisited - 1; j >= 0; --j ) {
      if( path[ i ] == visited[ j ] ) {
        furthestPath = i;
        furthestVisited = j;
        found = true;
      }
    }
    if( found )
      break;
  }

  // If no intersection found just return current path. 
  if( furthestPath == -1 || furthestVisited == -1 )
    return npath;

  // Concatenate paths.	

  // Adjust beginning of the buffer to include the visited.
  const int req = nvisited - furthestVisited;
  const int orig = rcMin( furthestPath + 1, npath );
  int size = rcMax( 0, npath - orig );
  if( req + size > maxPath )
    size = maxPath - req;
  if( size )
    memmove( path + req, path + orig, size*sizeof( dtPolyRef ) );

  // Store visited
  for( int i = 0; i < req; ++i )
    path[ i ] = visited[ (nvisited - 1) - i ];

  return req + size;
}

// This function checks if the path has a small U-turn, that is,
// a polygon further in the path is adjacent to the first polygon
// in the path. If that happens, a shortcut is taken.
// This can happen if the target (T) location is at tile boundary,
// and we're (S) approaching it parallel to the tile edge.
// The choice at the vertex can be arbitrary, 
//  +---+---+
//  |:::|:::|
//  +-S-+-T-+
//  |:::|   | <-- the step can end up in here, resulting U-turn path.
//  +---+---+
static int fixupShortcuts( dtPolyRef* path, int npath, dtNavMeshQuery* navQuery ) {
  if( npath < 3 )
    return npath;

  // Get connected polygons
  static const int maxNeis = 16;
  dtPolyRef neis[ maxNeis ];
  int nneis = 0;

  const dtMeshTile* tile = 0;
  const dtPoly* poly = 0;
  if( dtStatusFailed( navQuery->getAttachedNavMesh( )->getTileAndPolyByRef( path[ 0 ], &tile, &poly ) ) )
    return npath;

  for( unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[ k ].next ) {
    const dtLink* link = &tile->links[ k ];
    if( link->ref != 0 ) {
      if( nneis < maxNeis )
        neis[ nneis++ ] = link->ref;
    }
  }

  // If any of the neighbour polygons is within the next few polygons
  // in the path, short cut to that polygon directly.
  static const int maxLookAhead = 6;
  int cut = 0;
  for( int i = dtMin( maxLookAhead, npath ) - 1; i > 1 && cut == 0; i-- ) {
    for( int j = 0; j < nneis; j++ ) {
      if( path[ i ] == neis[ j ] ) {
        cut = i;
        break;
      }
    }
  }
  if( cut > 1 ) {
    int offset = cut - 1;
    npath -= offset;
    for( int i = 1; i < npath; i++ )
      path[ i ] = path[ i + offset ];
  }

  return npath;
}

static bool getSteerTarget( dtNavMeshQuery* navQuery, const float* startPos, const float* endPos,
                            const float minTargetDist,
                            const dtPolyRef* path, const int pathSize,
                            float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef,
                            float* outPoints = 0, int* outPointCount = 0 ) {
  // Find steer target.
  static const int MAX_STEER_POINTS = 3;
  float steerPath[ MAX_STEER_POINTS * 3 ];
  unsigned char steerPathFlags[ MAX_STEER_POINTS ];
  dtPolyRef steerPathPolys[ MAX_STEER_POINTS ];
  int nsteerPath = 0;
  navQuery->findStraightPath( startPos, endPos, path, pathSize,
                              steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS );
  if( !nsteerPath )
    return false;

  if( outPoints && outPointCount ) {
    *outPointCount = nsteerPath;
    for( int i = 0; i < nsteerPath; ++i )
      dtVcopy( &outPoints[ i * 3 ], &steerPath[ i * 3 ] );
  }


  // Find vertex far enough to steer to.
  int ns = 0;
  while( ns < nsteerPath ) {
    // Stop at Off-Mesh link or when point is further than slop away.
    if( (steerPathFlags[ ns ] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
        !inRange( &steerPath[ ns * 3 ], startPos, minTargetDist, 1000.0f ) )
        break;
    ns++;
  }
  // Failed to find good point to steer to.
  if( ns >= nsteerPath )
    return false;

  dtVcopy( steerPos, &steerPath[ ns * 3 ] );
  steerPos[ 1 ] = startPos[ 1 ];
  steerPosFlag = steerPathFlags[ ns ];
  steerPosRef = steerPathPolys[ ns ];

  return true;
}

const std::vector<VEC3> CNavmeshQuery::findPath( VEC3 start, VEC3 end ) {

  std::vector<VEC3> path;
  //path.push_back(start);
  int m_npolys, m_nsmoothPath = 0;
  dtPolyRef m_startRef, m_endRef;
  VEC3 m_smoothPath[MAX_SMOOTH];
  dtPolyRef m_polys[MAX_POLYS];

  dtStatus m_pathFindStatus = DT_FAILURE;


  data->m_navQuery->findNearestPoly(&start.x, &nearestPolyExtents.x, &m_filter, &m_startRef, 0);
  data->m_navQuery->findNearestPoly(&end.x, &nearestPolyExtents.x, &m_filter, &m_endRef, 0);

  data->m_navQuery->findPath( m_startRef, m_endRef, &start.x, &end.x, &m_filter, m_polys, &m_npolys, MAX_POLYS );

  if( m_npolys ) {
    // Iterate over the path to find smooth path on the detail mesh surface.
    dtPolyRef polys[ MAX_POLYS ];
    memcpy( polys, m_polys, sizeof( dtPolyRef )*m_npolys );
    int npolys = m_npolys;

    VEC3 iterPos, targetPos;
    data->m_navQuery->closestPointOnPoly( m_startRef, &start.x, &iterPos.x, 0 );
    data->m_navQuery->closestPointOnPoly( polys[ npolys - 1 ], &end.x, &targetPos.x, 0 );

    static const float STEP_SIZE = 2.f;
    static const float SLOP = 1.f;

    m_nsmoothPath = 0;

    m_smoothPath[m_nsmoothPath] = iterPos;
    m_nsmoothPath++;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while( npolys && m_nsmoothPath < MAX_SMOOTH ) {
      // Find location to steer towards.
      float steerPos[ 3 ];
      unsigned char steerPosFlag;
      dtPolyRef steerPosRef;

      if( !getSteerTarget( data->m_navQuery, &iterPos.x, &targetPos.x, SLOP,
        polys, npolys, steerPos, steerPosFlag, steerPosRef ) )
        break;

      bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
      bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

      // Find movement delta.
      float delta[ 3 ], len;
      dtVsub( delta, steerPos, &iterPos.x );
      len = dtMathSqrtf( dtVdot( delta, delta ) );
      // If the steer target is end of path or off-mesh link, do not move past the location.
      if( (endOfPath || offMeshConnection) && len < STEP_SIZE )
        len = 1;
      else
        len = STEP_SIZE / len;
      float moveTgt[ 3 ];
      dtVmad( moveTgt, &iterPos.x, delta, len );

      // Move
      float result[ 3 ];
      dtPolyRef visited[ 16 ];
      int nvisited = 0;
      data->m_navQuery->moveAlongSurface( polys[ 0 ], &iterPos.x, moveTgt, &m_filter,
                                                  result, visited, &nvisited, 16 );

      npolys = fixupCorridor( polys, npolys, MAX_POLYS, visited, nvisited );
      npolys = fixupShortcuts( polys, npolys, data->m_navQuery );

      float h = 0;
      data->m_navQuery->getPolyHeight( polys[ 0 ], result, &h );
      result[ 1 ] = h;
      dtVcopy( &iterPos.x, result );

      // Handle end of path and off-mesh links when close enough.
      if( endOfPath && inRange( &iterPos.x, steerPos, SLOP, 1.0f ) ) {
        // Reached end of path.
        iterPos = targetPos;
        if( m_nsmoothPath < MAX_SMOOTH ) {
          path.push_back(iterPos);
          m_smoothPath[m_nsmoothPath] = iterPos;
          m_nsmoothPath++;
        }
        break;
      }
      else if( offMeshConnection && inRange( &iterPos.x, steerPos, SLOP, 1.0f ) ) {
        // Reached off-mesh connection.
        VEC3 startPos, endPos;

        // Advance the path up to and over the off-mesh connection.
        dtPolyRef prevRef = 0, polyRef = polys[ 0 ];
        int npos = 0;
        while( npos < npolys && polyRef != steerPosRef ) {
          prevRef = polyRef;
          polyRef = polys[ npos ];
          npos++;
        }
        for( int i = npos; i < npolys; ++i )
          polys[ i - npos ] = polys[ i ];
        npolys -= npos;

        // Handle the connection.
        dtStatus status = data->m_navMesh->getOffMeshConnectionPolyEndPoints( prevRef, polyRef, &startPos.x, &endPos.x );
        if( dtStatusSucceed( status ) ) {
          if( m_nsmoothPath < MAX_SMOOTH ) {
            path.push_back(startPos);
            m_smoothPath[m_nsmoothPath] = startPos;
            m_nsmoothPath++;
            // Hack to make the dotted path not visible during off-mesh connection.
            if( m_nsmoothPath & 1 ) {
              path.push_back(startPos);
              m_smoothPath[m_nsmoothPath] = startPos;
              m_nsmoothPath++;
            }
          }
          // Move position at the other side of the off-mesh link.
          iterPos = endPos;
          float eh = 0.0f;
          data->m_navQuery->getPolyHeight( polys[ 0 ], &iterPos.x, &eh );
          iterPos.y = eh;
        }
      }

      // Store results.
      if( m_nsmoothPath < MAX_SMOOTH ) {
        path.push_back(iterPos);
        m_smoothPath[m_nsmoothPath] = iterPos;
        m_nsmoothPath++;
      }
    }
  }

  return path;
}

float CNavmeshQuery::wallDistance( VEC3 pos ) {

    /* TODO: note - only returns the distance. If we want the hit pos or the hit normal,
      return it as a reference or something like that */
    float m_distanceToWall = -1.f;
    VEC3 m_hitPos, m_hitNormal;

    dtPolyRef m_startRef;
    data->m_navQuery->findNearestPoly(&pos.x, &nearestPolyExtents.x, &m_filter, &m_startRef, 0);
    data->m_navQuery->findDistanceToWall( m_startRef, &pos.x, 100.0f, &m_filter, &m_distanceToWall, &m_hitPos.x, &m_hitNormal.x );
    return m_distanceToWall;
}

bool CNavmeshQuery::raycast( VEC3 start,VEC3 end, VEC3& hitPos ) {

    int m_npolys = 0;
    float t = 0;
    VEC3 m_hitPos, m_hitNormal;
    bool m_hitResult;
    dtPolyRef m_polys[MAX_POLYS];

    dtPolyRef m_startRef;
    data->m_navQuery->findNearestPoly(&start.x, &nearestPolyExtents.x, &m_filter, &m_startRef, 0);

    data->m_navQuery->raycast( m_startRef, &start.x, &end.x, &m_filter, &t, &m_hitNormal.x, m_polys, &m_npolys, MAX_POLYS );

    if( t > 1 ) {
      // No hit
      hitPos = end;
      m_hitResult = false;
    }
    else {
      // Hit
      dtVlerp( &hitPos.x, &start.x, &end.x, t );
      m_hitResult = true;
    }
    // Adjust height.
    if( m_npolys > 0 ) {
      float h = 0;
      data->m_navQuery->getPolyHeight( m_polys[ m_npolys - 1 ], &hitPos.x, &h );
      hitPos.y = h;
    }

    return m_hitResult;
}

VEC3 CNavmeshQuery::closestNavmeshPoint(VEC3 currentPoint)
{
  int m_nsmoothPath = 0;
  VEC3 closestPoint;
  dtPolyRef m_startRef;
  data->m_navQuery->findNearestPoly(&currentPoint.x, &nearestPolyExtents.x, &m_filter, &m_startRef, &closestPoint.x);
  return closestPoint;
}