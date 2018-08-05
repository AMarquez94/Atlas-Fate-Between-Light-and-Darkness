#pragma once

#include "comp_base.h"
#include "render/compute/particles.h"

class CRenderMeshInstanced;

struct TCompGPUParticles : public TCompBase {
    GPUParticles::TSystemInstance* system_instance = nullptr;
    CRenderMeshInstanced* instances_mesh = nullptr;

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    DECL_SIBLING_ACCESS();
};
