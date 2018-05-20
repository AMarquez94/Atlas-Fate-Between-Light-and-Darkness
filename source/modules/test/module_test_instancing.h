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

public:
    CModuleTestInstancing(const std::string& name)
        : IModule(name)
    {}
    bool start() override;
    void render() override;
    void update(float delta) override;
};