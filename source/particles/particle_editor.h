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
    ImVec2 _values_size[10];
    ImVec2 _values_velocity[10];

    int _index_shape;
    int _internal_index;
    int _tx_internal_index;
    std::vector<std::string> _images_files;
    std::vector<std::string> _particles_files;

    // Main debug draw functions

    std::vector<std::string> readFiles(const std::string & path);

    void debugLoader();
    void debugSystem();
    void debugEmission();
    void debugShape();
    void debugVelocity();
    void debugColor();
    void debugSize();
    void debugRenderer();
    void debugNoise();
    void debugRender();
    Particles::TCoreSystem::TNShape::EType mappings(int index);

    void saveParticleSystem();
    void LoadParticleSystem();
    void LoadTextureFromSource(const std::string & name);
};
