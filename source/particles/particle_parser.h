#pragma once

namespace Particles
{
	struct TCoreParticleSystem;

	class CParser
	{
	public:
		void parseFile(const std::string& file);
		TCoreParticleSystem* parseParticleSystem(const json& data);
	};
}