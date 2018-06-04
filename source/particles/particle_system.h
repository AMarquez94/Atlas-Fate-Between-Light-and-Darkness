#pragma once

#include "particle.h"
#include "utils/track.h"

namespace Particles
{
	struct TCoreParticleSystem
	{
		float lifetime = 1.f; // duration of each particle

		float emission_time = 0.f; // generation interval
		int emission_count = 1; // number of particles each generation

		float initial_speed = 0.f;
		float acceleration = 0.f;

		TTrack<VEC4> colors;
		TTrack<float> sizes;

		const CTexture* texture = nullptr;
		int blending = 0;
	};

	class CParticleSystem
	{
	public:
		CParticleSystem(const TCoreParticleSystem* core);
		void update(float delta);
		void launch();

	private:
		void emit();

		const TCoreParticleSystem* _core;
		std::vector<TParticle> _particles;
		MAT44 _location;
		float _time = 0.f;
	};

}
