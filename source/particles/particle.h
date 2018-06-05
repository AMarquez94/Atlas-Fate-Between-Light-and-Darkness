#pragma once

#include "mcv_platform.h"

namespace Particles
{
  struct TParticle
  {
    VEC4 color;
    VEC3 position;
    VEC3 velocity;
    float rotation = 0.f;
    float size = 1.f;
    float lifetime = 0.f;
    float max_lifetime = 0.f;
    float scale = 1.f;
    int frame = 0;
  };
}