//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef INC_NAVMESH_RENDER_H_
#define INC_NAVMESH_RENDER_H_

#include "navmesh/recast/DebugDraw.h"
#include "navmesh/recast/RecastDebugDraw.h"
//#include "draw_utils.h"

/// OpenGL debug draw implementation.
class DebugDrawGL : public duDebugDraw {
private:
  duDebugDrawPrimitives   primitive;
  float                   primitive_size;

  static const int        MAX_VERTICES = 200000;
  //CUSTOMVERTEX            colored_vertices[ MAX_VERTICES ];
  //TTexturedColoredVertex  textured_colored_vertices[ MAX_VERTICES ];
  int                     num_colored_vertices;
  int                     num_textured_colored_vertices;

public:
	virtual void depthMask(bool state);
	virtual void texture(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
};

#endif
