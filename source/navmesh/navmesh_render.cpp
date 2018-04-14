#include "mcv_platform.h"
#include "navmesh_render.h"
//#define _USE_MATH_DEFINES
//#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "navmesh/recast/DetourDebugDraw.h"
#include <assert.h>


class GLCheckerTexture
{
	unsigned int m_texId;
public:
	GLCheckerTexture() : m_texId(0)
	{
	}
	
	~GLCheckerTexture()
	{
		/*if (m_texId != 0)
			glDeleteTextures(1, &m_texId);*/
	}
	void bind()
	{
		//if (m_texId == 0)
		//{
		//	// Create checker pattern.
		//	const unsigned int col0 = duRGBA(215,215,215,255);
		//	const unsigned int col1 = duRGBA(255,255,255,255);
		//	static const int TSIZE = 64;
		//	unsigned int data[TSIZE*TSIZE];
		//	
		//	glGenTextures(1, &m_texId);
		//	glBindTexture(GL_TEXTURE_2D, m_texId);

		//	int level = 0;
		//	int size = TSIZE;
		//	while (size > 0)
		//	{
		//		for (int y = 0; y < size; ++y)
		//			for (int x = 0; x < size; ++x)
		//				data[x+y*size] = (x==0 || y==0) ? col0 : col1;
		//		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size,size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//		size /= 2;
		//		level++;
		//	}
		//	
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//}
		//else
		//{
		//	glBindTexture(GL_TEXTURE_2D, m_texId);
		//}
	}
};
GLCheckerTexture g_tex;


void DebugDrawGL::depthMask(bool state)
{
  /*if( state )
    Renderer.enableZWrite( );
  else
    Renderer.disableZWrite( );*/
}

void DebugDrawGL::texture( bool state )
{
	/*if (state)
	{
		glEnable(GL_TEXTURE_2D);
		g_tex.bind();
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}*/
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
  //colored_vertices[ num_colored_vertices ].assign( pos[0], pos[1], pos[2], color );
  ++num_colored_vertices;
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color)
{
  assert( num_colored_vertices < MAX_VERTICES-1 );
  //colored_vertices[ num_colored_vertices ].assign( x, y, z, color );
  ++num_colored_vertices;
}

void DebugDrawGL::vertex(const float* pos, unsigned int color, const float* uv)
{
  vertex( pos, color );
  /*assert( num_textured_colored_vertices < MAX_VERTICES-1 );
  textured_colored_vertices[ num_textured_colored_vertices ].assign( pos[0], pos[1], pos[2], color, uv[0], uv[1] );
  ++num_textured_colored_vertices;*/
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
  vertex( x, y, z, color );
  /*assert( num_textured_colored_vertices < MAX_VERTICES - 1 );
  textured_colored_vertices[ num_textured_colored_vertices ].assign( x, y, z, color, u, v );
  ++num_textured_colored_vertices;*/
}

void DebugDrawGL::end()
{
  /*g_pd3dDevice->SetTexture( 0, NULL );
  g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
  g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
  g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
  g_pd3dDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
  g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
  g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

  if( primitive == DU_DRAW_POINTS ) {
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_POINTLIST, num_colored_vertices, colored_vertices, sizeof( CUSTOMVERTEX ) );
  }
  else if( primitive == DU_DRAW_LINES ) {
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, num_colored_vertices / 2, colored_vertices, sizeof( CUSTOMVERTEX ) );
  }
  else if( primitive == DU_DRAW_TRIS ) {
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, num_colored_vertices / 3, colored_vertices, sizeof( CUSTOMVERTEX ) );
  }
  else if( primitive == DU_DRAW_QUADS ) {
    int num_quads = num_colored_vertices / 4;
    CUSTOMVERTEX vs[ 6 ];

    for( int i = 0; i<num_quads; ++i ) {
      int idx = i * 4;
      CUSTOMVERTEX& v1 = colored_vertices[ idx ];
      CUSTOMVERTEX& v2 = colored_vertices[ idx + 1 ];
      CUSTOMVERTEX& v3 = colored_vertices[ idx + 2 ];
      CUSTOMVERTEX& v4 = colored_vertices[ idx + 3 ];

      vs[ 0 ].assign( v1.x, v1.y, v1.z, v1.color );
      vs[ 1 ].assign( v3.x, v3.y, v3.z, v3.color );
      vs[ 2 ].assign( v2.x, v2.y, v2.z, v2.color );

      vs[ 3 ].assign( v1.x, v1.y, v1.z, v1.color );
      vs[ 4 ].assign( v4.x, v4.y, v4.z, v4.color );
      vs[ 5 ].assign( v2.x, v2.y, v2.z, v2.color );
   
      g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, vs, sizeof( CUSTOMVERTEX ) );
    }
  }
  g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
  g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

  num_colored_vertices = 0;
  num_textured_colored_vertices = 0;*/
}

