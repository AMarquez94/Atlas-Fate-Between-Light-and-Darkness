#pragma once

#include "mcv_platform.h"

namespace Particles
{
	struct TParticle
	{
		VEC4 color;
		VEC3 position;
		VEC3 rotation;
		VEC3 velocity;
		VEC3 scale;
		VEC2 minUV;
		VEC2 maxUV;
		float lifetime = 0.f;
	};
}