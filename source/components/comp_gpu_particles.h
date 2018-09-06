#pragma once

#include "comp_base.h"
#include "render/compute/particles.h"
#include "particles/particle_system.h"

class CRenderMeshInstanced;

struct TCompGPUParticles : public TCompBase {

    GPUParticles::TSystemInstance* system_instance = nullptr;
    CRenderMeshInstanced* instances_mesh = nullptr;

    Particles::TParticlesHandle   _particles = 0;
    const Particles::TCoreSystem* _core = nullptr;
    std::map<const Particles::TCoreSystem*, Particles::TParticlesHandle> _cores;

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    void onCreated(const TMsgEntityCreated&);
    void onGroupCreated(const TMsgEntitiesGroupCreated&);
    void onDestroyed(const TMsgEntityDestroyed&);

    void playSystem();
    void setSystemState(bool state);

    DECL_SIBLING_ACCESS();
};
