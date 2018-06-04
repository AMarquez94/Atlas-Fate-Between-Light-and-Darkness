#include "mcv_platform.h"
#include "module_particles.h"

using namespace Particles;

CModuleParticles::CModuleParticles(const std::string& name)
	: IModule(name)
{}

bool CModuleParticles::start()
{


	return true;
}

bool CModuleParticles::stop()
{
	return true;
}

void CModuleParticles::update(float delta)
{
	for (auto& ps : _activeSystems)
	{
		ps.update(delta);
	}

	// remove dead ones
	// ...
}

void CModuleParticles::render()
{
}

void CModuleParticles::registerSystem(const std::string& name, const Particles::TCoreParticleSystem* cps)
{
	_coreSystems[name] = cps;
}

const Particles::TCoreParticleSystem* CModuleParticles::getCoreSystem(const std::string& name)
{
	return _coreSystems[name];
}

void CModuleParticles::launchSystem(const std::string& name)
{
	const Particles::TCoreParticleSystem* cps = getCoreSystem(name);
	Particles::CParticleSystem ps(cps);
	ps.launch();
	_activeSystems.push_back(ps);
}

