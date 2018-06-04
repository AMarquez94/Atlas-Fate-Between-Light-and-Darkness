#include "mcv_platform.h"
#include "particle_parser.h"
#include "particles/particle_system.h"
#include "resources/json_resource.h"

namespace Particles
{
	void CParser::parseFile(const std::string& filename)
	{
		const json& j = Resources.get(filename)->as<CJsonResource>()->getJson();

		TCoreParticleSystem* cps = parseParticleSystem(j);
		Engine.getParticles().registerSystem(filename, cps);
	}

	TCoreParticleSystem* CParser::parseParticleSystem(const json& data)
	{
		TCoreParticleSystem* cps = new TCoreParticleSystem();

		cps->lifetime = data.value("lifetime", cps->lifetime);
		cps->initial_speed = data.value("initial_speed", cps->initial_speed);
		cps->acceleration = data.value("acceleration", cps->acceleration);
		cps->emission_time = data.value("emission_time", cps->emission_time);
		cps->emission_count = data.value("emission_count", cps->emission_count);
		//cps->blending = data.value("blending", cps->blending);
		cps->texture = Resources.get(data.value("texture", ""))->as<CTexture>();
		for (auto& clr : data.value["colors"])
		{
			float time = clr[0];
			VEC4 value = loadVEC4(clr[1]);
			cps->colors.set(time, value);
		}
		for (auto& sz : data.value["sizes"])
		{
			float time = sz[0];
			float value = sz[1];
			cps->sizes.set(time, value);
		}

		return cps;
	}
}