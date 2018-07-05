#include "mcv_platform.h"
#include "navmesh_render.h"
#include "navmesh/recast/DetourDebugDraw.h"
#include "render/render_objects.h"

void DebugDrawGL::depthMask(bool state)
{
}

void DebugDrawGL::texture( bool state )
{
}

void DebugDrawGL::begin(duDebugDrawPrimitives prim, float size)
{
  primitive = prim;
  primitive_size = size;
  num_colored_vertices = 0;
}

void DebugDrawGL::vertex(const float* pos, unsigned int color)
{
  assert( num_colored_vertices < MAX_VERTICES-1 );
  colored_vertices[ num_colored_vertices ].assign( pos[0], pos[1], pos[2], color );
  ++num_colored_vertices;
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color)
{
  assert( num_colored_vertices < MAX_VERTICES-1 );
  colored_vertices[ num_colored_vertices ].assign( x, y, z, color );
  ++num_colored_vertices;
}

void DebugDrawGL::vertex(const float* pos, unsigned int color, const float* uv)
{
  vertex( pos, color );
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
  vertex( x, y, z, color );
}

void DebugDrawGL::end()
{
    for (int i = 0; i < num_colored_vertices - 2; i += 3) {
        renderLine(VEC3(colored_vertices[i].x, colored_vertices[i].y, colored_vertices[i].z), 
            VEC3(colored_vertices[i + 1].x, colored_vertices[i + 1].y, colored_vertices[i + 1].z), 
            VEC4(1,0,0,1));

        renderLine(VEC3(colored_vertices[i + 1].x, colored_vertices[i + 1].y, colored_vertices[i + 1].z),
            VEC3(colored_vertices[i + 2].x, colored_vertices[i + 2].y, colored_vertices[i + 2].z),
            VEC4(0, 1, 0, 1));

        renderLine(VEC3(colored_vertices[i + 2].x, colored_vertices[i + 2].y, colored_vertices[i + 2].z),
            VEC3(colored_vertices[i].x, colored_vertices[i].y, colored_vertices[i].z),
            VEC4(0, 0, 1, 1));
    }

    num_colored_vertices = 0;
    num_textured_colored_vertices = 0;
}

