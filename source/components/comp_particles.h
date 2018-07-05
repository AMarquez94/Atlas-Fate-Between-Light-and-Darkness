#pragma once

#include "components/comp_base.h"
#include "particles/particle_system.h"

struct TCompParticles : public TCompBase
{
public:
    Particles::TParticlesHandle   _particles = 0;
    const Particles::TCoreSystem* _core = nullptr;
    std::map<const Particles::TCoreSystem*, Particles::TParticlesHandle> _cores;

    bool                          _on_start = false;
    bool                          _launched = false;
    float                         _fadeout = 0.f;

    TCompParticles() = default;
    static void registerMsgs();

    void load(const json& j, TEntityParseContext& ctx);
    void onCreated(const TMsgEntityCreated&);
    void onGroupCreated(const TMsgEntitiesGroupCreated&);
    void onDestroyed(const TMsgEntityDestroyed&);
    void debugInMenu();

    void playSystem();
    void setSystemState(bool state);
};

