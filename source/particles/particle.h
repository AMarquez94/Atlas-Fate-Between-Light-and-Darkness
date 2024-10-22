#pragma once

#include "mcv_platform.h"

namespace Particles
{
    struct TParticle
    {
        VEC4 color;
        VEC3 position;
        VEC3 origin_position;
        VEC3 velocity;
        VEC3 origin_velocity;
        VEC3 rotation;
        VEC3 random_direction;
        VEC3 size;
        float lifetime = 0.f;
        float max_lifetime = 0.f;
        float scale = 1.f;
        VEC3 downforce = VEC3::Zero;
        int frame = 0;
        int init_frame = 0;
        bool is_update = true;
    };

    struct TIParticle
    {
        MAT44 world;
        VEC3 particle_minUV;
        VEC3 particle_maxUV;
        VEC4 particle_color;
    };
}