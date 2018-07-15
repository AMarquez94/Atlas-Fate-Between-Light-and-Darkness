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
    CModuleParticles(const std::string& name);
    bool start() override;
    bool stop() override;
    void update(float delta) override;
    void renderMain();
    void renderDeferred();
    CModuleParticles* getPointer() { return this; }

    Particles::TParticleHandle launchSystem(const std::string& name, CHandle entity = CHandle());
    Particles::TParticleHandle launchSystem(const Particles::TCoreSystem* cps, CHandle entity = CHandle());
    void kill(Particles::TParticleHandle ph, float fade_out = 0.f);
    void killAll();
    Particles::CSystem* getSystem(Particles::TParticleHandle ph);

    const VEC3& getWindVelocity() const;
    ParticlesEditor * p_editor;
    bool particles_enabled = false;

private:
    std::vector<Particles::CSystem*> _activeSystems;
    VEC3                             _windVelocity = VEC3::Zero;
    Particles::TParticleHandle       _lastHandle;
};
