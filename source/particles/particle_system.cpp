#include "mcv_platform.h"
#include "particle_system.h"

namespace Particles
{

	CParticleSystem::CParticleSystem(const TCoreParticleSystem* core)
		: _core(core)
	{
		assert(_core);
	}

	void CParticleSystem::launch()
	{
		emit();
		_time = 0.f;
	}

	void CParticleSystem::update(float delta)
	{
		auto it = _particles.begin();
		while (it != _particles.end())
		{
			TParticle& particle = *it;

			particle.lifetime += delta;

			float ratio = clamp(particle.lifetime / _core->lifetime, 0.f, 1.f);
			particle.color = _core->colors.get(ratio);
			particle.scale = VEC3(_core->sizes.get(ratio));
			particle.velocity = particle.velocity + VEC3(_core->acceleration) * delta;
			// ...
		}
	}

	void CParticleSystem::emit()
	{
		for (int i = 0; i < _core->emission_count; ++i)
		{
			TParticle particle;
			particle.color = _core->colors.get(0.f);
			particle.scale = VEC3(_core->sizes.get(0.f));
			particle.velocity = VEC3::Up * _core->initial_speed;
			particle.minUV = VEC2(0, 0);
			particle.minUV = VEC2(1, 1);
			particle.rotation = VEC3::Zero;
			particle.lifetime = 0.f;

			_particles.push_back(particle);
		}
	}
}
