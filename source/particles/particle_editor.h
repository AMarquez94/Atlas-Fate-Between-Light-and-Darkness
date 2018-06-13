#pragma once

#include "particle.h"
#include "utils/track.h"
#include "resources/resource.h"

namespace ParticleEditor {

    class PEditor {

    public:

        std::vector<Particles::CSystem*> _current_systems;
    private:

        void Init();
        void particleDestroy();

        // Main debug draw functions
        void debugMain();
        void debugSystem();
        void debugEmission();
        void debugShape();
        void debugVelocity();
        void debugRenderer();
        void debugNoise();


        void saveCurrent();
    };

}
