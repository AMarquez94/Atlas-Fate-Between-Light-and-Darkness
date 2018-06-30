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
    Particles::TCoreSystem * _main_system;

    int _internal_index;
    std::vector<std::string> _particles_files;

    // Main debug draw functions

    void readFiles();
    void debugLoader();
    void debugSystem();
    void debugEmission();
    void debugShape();
    void debugVelocity();
    void debugColor();
    void debugSize();
    void debugRenderer();
    void debugNoise();

    void saveParticleSystem();
    void LoadParticleSystem();
};
