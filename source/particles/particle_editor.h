#pragma once

#include "particle.h"
#include "utils/track.h"
#include "resources/resource.h"

class ParticlesEditor {

public:


    ParticlesEditor();
    ~ParticlesEditor();
        
    void Init();
    void debugMenu();

private:
        
    CHandle _main_debug;

    // Main debug draw functions

    void debugSystem();
    void debugEmission();
    void debugShape();
    void debugVelocity();
    void debugRenderer();
    void debugNoise();


    void saveCurrent();
    void LoadParticle();
};
