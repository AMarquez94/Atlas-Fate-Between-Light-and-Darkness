#pragma once

#include "modules/module.h"
class ParticlesEditor;

namespace Particles
{
    struct TCoreSystem;
    class CSystem;
    using TParticleHandle = int;
}

class CModuleParticles : public IModule
{
public:

    ParticlesEditor * p_editor;

    float rate_min_time = 1.0f;
    float rate_min_dist = 0.05f;
    bool  particles_enabled = false;

    CModuleParticles(const std::string& name);
    bool start() override;
    bool stop() override;
    void update(float delta) override;
    void renderMain();
    void renderDeferred();

    Particles::TParticleHandle launchSystem(const std::string& name, CHandle entity = CHandle());
    Particles::TParticleHandle launchSystem(const Particles::TCoreSystem* cps, CHandle entity = CHandle());
    Particles::TParticleHandle launchDynamicSystem(const std::string& name, VEC3 pos);
    Particles::CSystem* getSystem(Particles::TParticleHandle ph);

    const VEC3& getWindVelocity() const;
    void kill(Particles::TParticleHandle ph, float fade_out = 0.f);
    void killAll();

    CModuleParticles* getPointer() { return this; }
private:
    std::vector<Particles::CSystem*> _activeSystems;
    VEC3                             _windVelocity = VEC3::Zero;
    Particles::TParticleHandle       _lastHandle;
};
