#include "mcv_platform.h"
#include "particle_parser.h"
#include "particles/particle_system.h"

namespace Particles
{
  void CParser::parseFile(const std::string& filename)
  {
    std::ifstream file_json(filename);
    json json_data;
    file_json >> json_data;

    for (auto& pFile : json_data)
    {
      const TCoreSystem* cps = Resources.get(pFile)->as<TCoreSystem>();
      assert(cps);
    }
  }

  TCoreSystem* CParser::parseParticlesFile(const std::string& filename)
  {
    std::ifstream file_json(filename);
    json json_data;
    file_json >> json_data;

    return parseParticleSystem(json_data);
  }

  TCoreSystem* CParser::parseParticleSystem(const json& data)
  {
    TCoreSystem* cps = new TCoreSystem();

    // life
    const json& life = data["life"];
    cps->life.duration = life.value("duration", cps->life.duration);
    cps->life.durationVariation = life.value("duration_variation", cps->life.durationVariation);
    cps->life.maxParticles = life.value("max_particles", cps->life.maxParticles);
    cps->life.timeFactor = life.value("time_factor", cps->life.timeFactor);
    // emission
    const json& emission = data["emission"];
    cps->emission.cyclic = emission.value("cyclic", cps->emission.cyclic);
    cps->emission.interval = emission.value("interval", cps->emission.interval);
    cps->emission.count = emission.value("count", cps->emission.count);
    const std::string emitterType = emission.value("type", "point");
    if (emitterType == "line")        cps->emission.type = TCoreSystem::TEmission::Line;
    else if (emitterType == "square") cps->emission.type = TCoreSystem::TEmission::Square;
    else if (emitterType == "box")    cps->emission.type = TCoreSystem::TEmission::Box;
    else if (emitterType == "sphere") cps->emission.type = TCoreSystem::TEmission::Sphere;
    else                              cps->emission.type = TCoreSystem::TEmission::Point;
    cps->emission.size = emission.value("size", cps->emission.size);
    cps->emission.angle = deg2rad(emission.value("angle", rad2deg(cps->emission.angle)));
    // movement
    const json& movement = data["movement"];
    cps->movement.velocity = movement.value("velocity", cps->movement.velocity);
    cps->movement.acceleration = movement.value("acceleration", cps->movement.acceleration);
    cps->movement.spin = deg2rad(movement.value("spin", rad2deg(cps->movement.spin)));
    cps->movement.wind = movement.value("wind", cps->movement.wind);
    cps->movement.gravity = movement.value("gravity", cps->movement.gravity);
    cps->movement.ground = movement.value("ground", cps->movement.ground);
    // render
    const json& render = data["render"];
    cps->render.initialFrame = render.value("initial_frame", cps->render.initialFrame);
    cps->render.frameSize = loadVEC2(render.value("frame_size", "1 1"));
    cps->render.numFrames = render.value("num_frames", cps->render.numFrames);
    cps->render.frameSpeed = render.value("frame_speed", cps->render.frameSpeed);
    cps->render.texture = Resources.get(render.value("texture", ""))->as<CTexture>();
    // color
    const json& color = data["color"];
    cps->color.opacity = color.value("opacity", cps->color.opacity);
    for (auto& clr : color["colors"])
    {
      float time = clr[0];
      VEC4 value = loadVEC4(clr[1]);
      cps->color.colors.set(time, value);
    }
    cps->color.colors.sort();
    // size
    const json& size = data["size"];
    cps->size.scale = size.value("scale", cps->size.scale);
    cps->size.scale_variation = size.value("scale_variation", cps->size.scale_variation);
    for (auto& sz : size["sizes"])
    {
      float time = sz[0];
      float value = sz[1];
      cps->size.sizes.set(time, value);
    }
    cps->size.sizes.sort();

    return cps;
  }
}