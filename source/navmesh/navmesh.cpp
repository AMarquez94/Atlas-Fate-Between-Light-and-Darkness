#include "mcv_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include "navmesh.h"
#include "navmesh/recast/DetourNavMeshBuilder.h"
#include "navmesh/recast/RecastDump.h"
#include "navmesh/recast/DetourDebugDraw.h"

CNavmesh::CNavmesh( ) 
  : m_navMesh( nullptr )
{
  m_navQuery = dtAllocNavMeshQuery( );
  m_ctx = &m_context;
}

void CNavmesh::build( ) {
  destroy( );
  m_ctx->resetLog( );

  rcConfig config;
  rcVcopy( config.bmin, &m_input.aabb_min.x );
  rcVcopy( config.bmax, &m_input.aabb_max.x );
  config.tileSize = 32;
  config.cs = 0.05f;
  config.ch = 0.05f;
  config.walkableHeight = 2;
  config.walkableRadius = 0;
  config.walkableClimb = 0.1;
  config.walkableSlopeAngle = 45;
  config.minRegionArea = 1;
  config.mergeRegionArea = 1;
  config.maxEdgeLen = 10;
  config.maxSimplificationError = 1.0f;
  config.maxVertsPerPoly = 6;
  config.detailSampleDist = 1.0f;
  config.detailSampleMaxError = 0.1f;

  m_navMesh = create( config );
  if( m_navMesh )
    prepareQueries( );

  dumpLog( );
}

dtNavMesh* CNavmesh::loadAll(const char* path)
{
  FILE* fp = fopen(path, "rb");
  if (!fp) return 0;

  // Read header.
  NavMeshSetHeader header;
  size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
  if (readLen != 1)
  {
    fclose(fp);
    return 0;
  }
  if (header.magic != NAVMESHSET_MAGIC)
  {
    fclose(fp);
    return 0;
  }
  if (header.version != NAVMESHSET_VERSION)
  {
    fclose(fp);
    return 0;
  }

  dtNavMesh* mesh = dtAllocNavMesh();
  if (!mesh)
  {
    fclose(fp);
    return 0;
  }
  dtStatus status = mesh->init(&header.params);
  if (dtStatusFailed(status))
  {
    fclose(fp);
    return 0;
  }

  // Read tiles.
  for (int i = 0; i < header.numTiles; ++i)
  {
    NavMeshTileHeader tileHeader;
    readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
    if (readLen != 1)
    {
      fclose(fp);
      return 0;
    }

    if (!tileHeader.tileRef || !tileHeader.dataSize)
      break;

    unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
    if (!data) break;
    memset(data, 0, tileHeader.dataSize);
    readLen = fread(data, tileHeader.dataSize, 1, fp);
    if (readLen != 1)
    {
      dtFree(data);
      fclose(fp);
      return 0;
    }

    mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
  }

  fclose(fp);

  if (m_navMesh)
    prepareQueries();

  return mesh;
}

dtNavMesh* CNavmesh::create( const rcConfig& cfg ) {
  // WARNING: We will admit animated meshes, but the first snapshot will be the used to generate the navmesh
  //assert( mesh->header.nsnapshots == 1 ); // must be a static mesh!

  dtNavMesh* m_nav = nullptr;

  bool m_keepInterResults = true;
  bool m_monotonePartitioning = false;

  // -------------------------------------------
  // Step 1. Initialize build config.
  // -------------------------------------------

  // Init build configuration from GUI
  memset( &m_cfg, 0, sizeof( m_cfg ) );
  m_cfg.cs = cfg.cs;
  m_cfg.ch = cfg.ch;
  m_cfg.walkableSlopeAngle = cfg.walkableSlopeAngle;
  m_cfg.walkableHeight = (int)ceilf( cfg.walkableHeight / m_cfg.ch );
  m_cfg.walkableClimb = (int)floorf( cfg.walkableClimb / m_cfg.ch );
  m_cfg.walkableRadius = (int)ceilf( cfg.walkableRadius / m_cfg.cs );
  m_cfg.maxEdgeLen = (int)(cfg.maxEdgeLen / cfg.cs);
  m_cfg.maxSimplificationError = cfg.maxSimplificationError;
  m_cfg.minRegionArea = (int)rcSqr( cfg.minRegionArea );		// Note: area = size*size
  m_cfg.mergeRegionArea = (int)rcSqr( cfg.mergeRegionArea );	// Note: area = size*size
  m_cfg.maxVertsPerPoly = (int)cfg.maxVertsPerPoly;
  m_cfg.detailSampleDist = cfg.detailSampleDist < 0.9f ? 0 : cfg.cs * cfg.detailSampleDist;
  m_cfg.detailSampleMaxError = cfg.ch * cfg.detailSampleMaxError;

  // Set the area where the navigation will be build.
  // Here the bounds of the input mesh are used, but the
  // area could be specified by an user defined box, etc.
  rcVcopy( m_cfg.bmin, cfg.bmin );
  rcVcopy( m_cfg.bmax, cfg.bmax );
  rcCalcGridSize( m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height );

  // Reset build times gathering.
  m_ctx->resetTimers( );

  // Start the build process.	
  m_ctx->startTimer( RC_TIMER_TOTAL );

  m_ctx->log( RC_LOG_PROGRESS, "Building navigation:" );
  m_ctx->log( RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height );
  m_ctx->log( RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", m_input.nverts / 1000.0f, m_input.ntris / 1000.0f );

  // -------------------------------------------
  // Step 2. Rasterize input polygon soup.
  // -------------------------------------------

  // Allocate voxel heightfield where we rasterize our input data to.
  m_solid = rcAllocHeightfield( );
  if( !m_solid ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'." );
    return nullptr;
  }
  if( !rcCreateHeightfield( m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch ) ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield." );
    return nullptr;
  }

  // Allocate array that can hold triangle area types.
  // If you have multiple meshes you need to process, allocate
  // and array which can hold the max number of triangles you need to process.
  m_triareas = new unsigned char[ m_input.ntris_total ];
  if( !m_triareas ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", m_input.ntris_total );
    return nullptr;
  }

  // Find triangles which are walkable based on their slope and rasterize them.
  // If your input data is multiple meshes, you can transform them here, calculate
  // the are type for each of the meshes and rasterize them.
  memset( m_triareas, 0, m_input.ntris*sizeof( unsigned char ) );

  for( size_t i = 0; i<m_input.inputs.size( ); ++i ) {
    m_input.prepareInput( m_input.inputs[ i ] );

    rcMarkWalkableTriangles( m_ctx, m_cfg.walkableSlopeAngle, m_input.verts, m_input.nverts, m_input.tris, m_input.ntris, m_triareas );
    rcRasterizeTriangles( m_ctx, m_input.verts, m_input.nverts, m_input.tris, m_triareas, m_input.ntris, *m_solid, m_cfg.walkableClimb );

    m_input.unprepareInput( );
  }

  if( !m_keepInterResults ) {
    delete[ ] m_triareas;
    m_triareas = 0;
  }

  // -------------------------------------------
  // Step 3. Filter walkables surfaces.
  // -------------------------------------------

  // Once all geoemtry is rasterized, we do initial pass of filtering to
  // remove unwanted overhangs caused by the conservative rasterization
  // as well as filter spans where the character cannot possibly stand.
  rcFilterLowHangingWalkableObstacles( m_ctx, m_cfg.walkableClimb, *m_solid );
  rcFilterLedgeSpans( m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid );
  rcFilterWalkableLowHeightSpans( m_ctx, m_cfg.walkableHeight, *m_solid );


  // -------------------------------------------
  // Step 4. Partition walkable surface to simple regions.
  // -------------------------------------------

  // Compact the heightfield so that it is faster to handle from now on.
  // This will result more cache coherent data as well as the neighbours
  // between walkable cells will be calculated.
  m_chf = rcAllocCompactHeightfield( );
  if( !m_chf ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'." );
    return nullptr;
  }
  if( !rcBuildCompactHeightfield( m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf ) ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not build compact data." );
    return nullptr;
  }

  if( !m_keepInterResults ) {
    rcFreeHeightField( m_solid );
    m_solid = 0;
  }

  // Erode the walkable area by agent radius.
  if( !rcErodeWalkableArea( m_ctx, m_cfg.walkableRadius, *m_chf ) ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not erode." );
    return nullptr;
  }

  if( m_monotonePartitioning ) {
    // Partition the walkable surface into simple regions without holes.
    // Monotone partitioning does not need distancefield.
    if( !rcBuildRegionsMonotone( m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea ) ) {
      m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not build regions." );
      return nullptr;
    }
  }
  else {
    // Prepare for region partitioning, by calculating distance field along the walkable surface.
    if( !rcBuildDistanceField( m_ctx, *m_chf ) ) {
      m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not build distance field." );
      return nullptr;
    }

    // Partition the walkable surface into simple regions without holes.
    if( !rcBuildRegions( m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea ) ) {
      m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not build regions." );
      return nullptr;
    }
  }

  // -------------------------------------------
  // Step 5. Trace and simplify region contours.
  // -------------------------------------------

  // Create contours.
  m_cset = rcAllocContourSet( );
  if( !m_cset ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'." );
    return nullptr;
  }
  if( !rcBuildContours( m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset ) ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not create contours." );
    return nullptr;
  }

  // -------------------------------------------
  // Step 6. Build polygons mesh from contours.
  // -------------------------------------------

  // Build polygon navmesh from the contours.
  m_pmesh = rcAllocPolyMesh( );
  if( !m_pmesh ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'." );
    return nullptr;
  }
  if( !rcBuildPolyMesh( m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh ) ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not triangulate contours." );
    return nullptr;
  }

  // -------------------------------------------
  // Step 7. Create detail mesh which allows to access approximate height on each polygon.
  // -------------------------------------------

  m_dmesh = rcAllocPolyMeshDetail( );
  if( !m_dmesh ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'." );
    return nullptr;
  }

  if( !rcBuildPolyMeshDetail( m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh ) ) {
    m_ctx->log( RC_LOG_ERROR, "buildNavigation: Could not build detail mesh." );
    return nullptr;
  }

  if( !m_keepInterResults ) {
    rcFreeCompactHeightfield( m_chf );
    m_chf = 0;
    rcFreeContourSet( m_cset );
    m_cset = 0;
  }

  // At this point the navigation mesh data is ready, you can access it from m_pmesh.
  // See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

  // -------------------------------------------
  // (Optional) Step 8. Create Detour data from Recast poly mesh.
  // -------------------------------------------

  // The GUI may allow more max points per polygon than Detour can handle.
  // Only build the detour navmesh if we do not exceed the limit.
  if( m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON ) {
    unsigned char* navData = 0;
    int navDataSize = 0;

    // Update poly flags from areas.
    for( int i = 0; i < m_pmesh->npolys; ++i ) {
      if( m_pmesh->areas[ i ] == RC_WALKABLE_AREA )
        m_pmesh->flags[ i ] = FLAG_WALK;
    }

    dtNavMeshCreateParams params;
    memset( &params, 0, sizeof( params ) );
    params.verts = m_pmesh->verts;
    params.vertCount = m_pmesh->nverts;
    params.polys = m_pmesh->polys;
    params.polyAreas = m_pmesh->areas;
    params.polyFlags = m_pmesh->flags;
    params.polyCount = m_pmesh->npolys;
    params.nvp = m_pmesh->nvp;
    params.detailMeshes = m_dmesh->meshes;
    params.detailVerts = m_dmesh->verts;
    params.detailVertsCount = m_dmesh->nverts;
    params.detailTris = m_dmesh->tris;
    params.detailTriCount = m_dmesh->ntris;
    params.walkableHeight = (float)cfg.walkableHeight;
    params.walkableRadius = (float)cfg.walkableRadius;
    params.walkableClimb = (float)cfg.walkableClimb;
    rcVcopy( params.bmin, m_pmesh->bmin );
    rcVcopy( params.bmax, m_pmesh->bmax );
    params.cs = m_cfg.cs;
    params.ch = m_cfg.ch;
    params.buildBvTree = true;

    if( !dtCreateNavMeshData( &params, &navData, &navDataSize ) ) {
      m_ctx->log( RC_LOG_ERROR, "Could not build Detour navmesh." );
      return nullptr;
    }

    m_nav = dtAllocNavMesh( );
    if( !m_nav ) {
      dtFree( navData );
      m_ctx->log( RC_LOG_ERROR, "Could not create Detour navmesh" );
      return nullptr;
    }

    dtStatus status;

    status = m_nav->init( navData, navDataSize, DT_TILE_FREE_DATA );
    if( dtStatusFailed( status ) ) {
      dtFree( navData );
      m_ctx->log( RC_LOG_ERROR, "Could not init Detour navmesh" );
      return nullptr;
    }
  }

  m_ctx->stopTimer( RC_TIMER_TOTAL );

  // Show performance stats.
  duLogBuildTimes( *m_ctx, m_ctx->getAccumulatedTime( RC_TIMER_TOTAL ) );
  m_ctx->log( RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys );

  return m_nav;
}

void CNavmesh::prepareQueries( ) {
  dtStatus status = m_navQuery->init( m_navMesh, 2048 );
  if( dtStatusFailed( status ) )
    printf( "NAVMESH PREPARATION FAILED!\n" );
}

void CNavmesh::destroy( ) {
  delete[ ] m_triareas;
  m_triareas = 0;
  rcFreeHeightField( m_solid );
  m_solid = 0;
  rcFreeCompactHeightfield( m_chf );
  m_chf = 0;
  rcFreeContourSet( m_cset );
  m_cset = 0;
  rcFreePolyMesh( m_pmesh );
  m_pmesh = 0;
  rcFreePolyMeshDetail( m_dmesh );
  m_dmesh = 0;
  dtFreeNavMesh( m_navMesh );
  m_navMesh = 0;
}

void CNavmesh::dumpLog( ) {
  printf( "\n" );
  for( int i = 0; i < m_ctx->getLogCount( ); ++i )
    printf( "%s\n", m_ctx->getLogText( i ) );
  printf( "\n" );
}

void CNavmesh::render( bool use_z_test ) {
  /*if( !use_z_test )
    Renderer.disableZTest( );*/

  /*TPoint aabb_min, aabb_max;
  input_data.aabb.getMinMax( aabb_min, aabb_max );*/

  // render the bounding box
  /*duDebugDrawBoxWire( &m_draw, aabb_min.x, aabb_min.y, aabb_min.z, aabb_max.x, aabb_max.y, aabb_max.z, duRGBA( 255, 255, 255, 128 ), 1.0f );
  m_draw.begin( DU_DRAW_POINTS, 5.0f );
  m_draw.vertex( aabb_min.x, aabb_min.y, aabb_min.z, duRGBA( 255, 255, 255, 128 ) );
  m_draw.end( );*/

  unsigned char m_navMeshDrawFlags( DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_CLOSEDLIST | DU_DRAWNAVMESH_COLOR_TILES );
  const int SAMPLE_POLYFLAGS_DISABLED = 0xffff;

  //Renderer.disableZWrite( );

  if( m_navMesh && m_navQuery &&
      (m_draw_mode == NAVMESH_DRAW_MESH ||
      m_draw_mode == NAVMESH_DRAW_TRANS ||
      m_draw_mode == NAVMESH_DRAW_BVTREE ||
      m_draw_mode == NAVMESH_DRAW_NODES ||
      m_draw_mode == NAVMESH_DRAW_INVIS) ) {
    if( m_draw_mode != NAVMESH_DRAW_INVIS )
      duDebugDrawNavMeshWithClosedList( &m_draw, *m_navMesh, *m_navQuery, m_navMeshDrawFlags );
    if( m_draw_mode == NAVMESH_DRAW_BVTREE )
      duDebugDrawNavMeshBVTree( &m_draw, *m_navMesh );
    if( m_draw_mode == NAVMESH_DRAW_NODES )
      duDebugDrawNavMeshNodes( &m_draw, *m_navQuery );
    duDebugDrawNavMeshPolysWithFlags( &m_draw, *m_navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA( 0, 0, 255, 128 ) );
  }

  //Renderer.enableZWrite( );

  if( m_cset && m_draw_mode == NAVMESH_DRAW_COUNTOURS ) {
    //Renderer.disableZWrite( );
    duDebugDrawContours( &m_draw, *m_cset );
    //Renderer.enableZWrite( );
  }

  if( m_pmesh && m_draw_mode == NAVMESH_DRAW_POLYMESH ) {
    //Renderer.disableZWrite( );
    duDebugDrawPolyMesh( &m_draw, *m_pmesh );
    //Renderer.enableZWrite( );
  }
  if( m_dmesh && m_draw_mode == NAVMESH_DRAW_POLYMESH_DETAILS ) {
    //Renderer.disableZWrite( );
    duDebugDrawPolyMeshDetail( &m_draw, *m_dmesh );
    //Renderer.enableZWrite( );
  }

  /*if( type == NAVMESH_STATIC_MESH )
    static_mesh.render( m_draw, draw_mode );*/

  /*Renderer.enableZWrite( );
  if( !use_z_test )
    Renderer.enableZTest( );*/
}
