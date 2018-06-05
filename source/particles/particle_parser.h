#pragma once

namespace Particles
{
  struct TCoreSystem;

  class CParser
  {
  public:
    void parseFile(const std::string& file);
    TCoreSystem* parseParticlesFile(const std::string& file);
    TCoreSystem* parseParticleSystem(const json& data);
  };
}