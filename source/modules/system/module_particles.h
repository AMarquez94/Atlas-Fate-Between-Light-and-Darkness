#pragma once

#include "modules/module.h"

namespace Particles
{
	struct TCoreParticleSystem;
	class CParticleSystem;
}

class CModuleParticles : public IModule
{
public:
	CModuleParticles(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;

	void registerSystem(const std::string& name, const Particles::TCoreParticleSystem* cps);
	const Particles::TCoreParticleSystem* getCoreSystem(const std::string& name);
	void launchSystem(const std::string& name);

private:
	std::map<const std::string, const Particles::TCoreParticleSystem*> _coreSystems;
	std::vector<const Particles::CParticleSystem*> _activeSystems;
};
