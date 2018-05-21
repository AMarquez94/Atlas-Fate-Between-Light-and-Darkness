#pragma once

#include "modules/module.h"
#include "render/mesh/mesh_instanced.h"

class CModuleTestInstancing : public IModule {
    // -------------------------------------------------------------------
    struct TInstance {
        MAT44 world;
    };
    CRenderMeshInstanced* instances_mesh = nullptr;
    std::vector< TInstance > instances;

    // -------------------------------------------------------------------
    struct TInstanceBlood {
        MAT44 world;
        VEC4  color;
    };
    CRenderMeshInstanced* blood_instances_mesh = nullptr;
    std::vector< TInstanceBlood > blood_instances;

    // -------------------------------------------------------------------
    struct TRenderParticle {
        VEC3  pos;      // Maps to iCenter.xyz
        float angle;    // Maps to iCenter.w
        VEC4  color;
        float scale_x;
        float scale_y;
        float nframe;
    };
    CRenderMeshInstanced* particles_instances_mesh = nullptr;
    std::vector< TRenderParticle > particles_instances;

public:
    CModuleTestInstancing(const std::string& name)
        : IModule(name)
    {}
    bool start() override;
    void render() override;
    void update(float delta) override;
};