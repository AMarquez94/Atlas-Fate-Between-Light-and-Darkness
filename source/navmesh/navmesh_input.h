#ifndef _NAVMESH_INPUT_INC
#define _NAVMESH_INPUT_INC

#include <vector>
//#include "ground.h"

class CNavmeshInput {
public:
  struct TInput {
    VEC3 pmin;
    VEC3 pmax;
    TInput( ) : pmin( 0, 0, 0 ), pmax( 0, 0, 0 ) { }
  };
  static const int MAX_INPUTS = 1024;
  typedef std::vector< TInput > VInputs;

public:
  VInputs               inputs;
  VEC3           aabb_min;
  VEC3           aabb_max;
  
  float*                verts;
  int*                  tris;
  int                   nverts;
  int                   ntris;
  int                   nverts_total;
  int                   ntris_total;

public:
  CNavmeshInput( );

  void clearInput( );
  //void addInput( const ground& g );
  void addInput( const VEC3& p0, const VEC3& p1 );
  void prepareInput( const TInput& input );
  void unprepareInput( );
  void computeBoundaries(  );
};

#endif
