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

        const json& system = data["system"];
        {
            cps->n_system.duration = system.value("duration", cps->n_system.duration);
            cps->n_system.looping = system.value("looping", cps->n_system.looping);
            cps->n_system.start_delay = system.value("start_delay", cps->n_system.start_delay);
            cps->n_system.start_lifetime = system.value("start_lifetime", cps->n_system.start_lifetime);
            cps->n_system.start_speed = system.value("start_speed", cps->n_system.start_speed);
            cps->n_system.d_start_size = loadVEC3(system.value("d_start_size", "1 1 1"));
            cps->n_system.start_size = system.value("start_size", cps->n_system.start_size);
            cps->n_system.d_start_rotation = loadVEC3(system.value("d_start_rotation", "0 0 0"));
            cps->n_system.start_rotation = system.value("start_rotation", cps->n_system.start_rotation);
            cps->n_system.random_rotation = system.value("random_rotation", cps->n_system.random_rotation);
            cps->n_system.start_color = loadVEC4(system.value("start_color", "1 1 1 1"));
            cps->n_system.gravity = system.value("gravity", cps->n_system.gravity);
            cps->n_system.simulation_speed = system.value("simulation_speed", cps->n_system.simulation_speed);
            cps->n_system.max_particles = system.value("max_particles", cps->n_system.max_particles);
        }
        
        // emission
        const json& emission = data["emission"];
        {
            cps->n_emission.rate_time = emission.value("rate_time", cps->n_emission.rate_time);
            cps->n_emission.rate_distance = emission.value("rate_distance", cps->n_emission.rate_distance);
            // Add bursts support
        }
        
        // shape
        const json& shape = data["shape"];
        {
            const std::string emitterType = shape.value("type", "point");
            if (emitterType == "line")        cps->n_shape.type = TCoreSystem::TNShape::Line;
            else if (emitterType == "square") cps->n_shape.type = TCoreSystem::TNShape::Square;
            else if (emitterType == "box")    cps->n_shape.type = TCoreSystem::TNShape::Box;
            else if (emitterType == "sphere") cps->n_shape.type = TCoreSystem::TNShape::Sphere;
            else if (emitterType == "cone") cps->n_shape.type = TCoreSystem::TNShape::Cone;
            else if (emitterType == "circle") cps->n_shape.type = TCoreSystem::TNShape::Circle;
            else                              cps->n_shape.type = TCoreSystem::TNShape::Point;

            cps->n_shape.size = loadVEC3(shape.value("size", "1 1 1"));
            cps->n_shape.angle = deg2rad(shape.value("angle", rad2deg(cps->n_shape.angle)));
        }
        
        // velocity
        const json& velocity = data["emission"];
        {
            cps->n_velocity.constant_velocity = loadVEC3(velocity.value("velocity", "1 1 1"));
            //cps->n_velocity.acceleration = velocity.value("acceleration", cps->movement.acceleration);
            //cps->n_velocity.spin = deg2rad(velocity.value("spin", rad2deg(cps->movement.spin)));
            //cps->n_velocity.wind = velocity.value("wind", cps->movement.wind);
        }

        // render
        const json& render = data["renderer"];
        {
            const std::string renderMode = render.value("mode", "billboard");
            if (renderMode == "billboard")        cps->n_renderer.mode = TCoreSystem::TNRenderer::BILLBOARD;

            cps->n_renderer.initialFrame = render.value("initial_frame", cps->n_renderer.initialFrame);
            cps->n_renderer.frameSize = loadVEC2(render.value("frame_size", "1 1"));
            cps->n_renderer.numFrames = render.value("num_frames", cps->n_renderer.numFrames);
            cps->n_renderer.frameSpeed = render.value("frame_speed", cps->n_renderer.frameSpeed);
            cps->n_renderer.texture = Resources.get(render.value("texture", ""))->as<CTexture>();
        }

        // noise
        const json& noise = data["noise"];
        {
            cps->n_noise.texture = Resources.get(render.value("texture", ""))->as<CTexture>();

            cps->n_noise.strength = noise.value("strength", cps->n_noise.strength);
            cps->n_noise.frequency = noise.value("strength", cps->n_noise.frequency);
            cps->n_noise.scroll_speed = noise.value("num_frames", cps->n_noise.scroll_speed);
            cps->n_noise.damping = noise.value("num_frames", cps->n_noise.damping);
            cps->n_noise.octaves = noise.value("frame_speed", cps->n_noise.octaves);
        }

        // color
        const json& color = data["color"];
        {
            cps->n_color.opacity = color.value("opacity", cps->n_color.opacity);
            for (auto& clr : color["colors"])
            {
                float time = clr[0];
                VEC4 value = loadVEC4(clr[1]);
                cps->n_color.colors.set(time, value);
            }
            cps->n_color.colors.sort();
        }

        // size
        const json& size = data["size"];
        {
            cps->n_size.scale = size.value("scale", cps->n_size.scale);
            cps->n_size.scale_variation = size.value("scale_variation", cps->n_size.scale_variation);
            for (auto& sz : size["sizes"])
            {
                float time = sz[0];
                float value = sz[1];
                cps->n_size.sizes.set(time, value);
            }
            cps->n_size.sizes.sort();
        }

        return cps;

        // life
        /*const json& life = data["life"];
        {
            cps->life.duration = life.value("duration", cps->life.duration);
            cps->life.durationVariation = life.value("duration_variation", cps->life.durationVariation);
            cps->life.maxParticles = life.value("max_particles", cps->life.maxParticles);
            cps->life.timeFactor = life.value("time_factor", cps->life.timeFactor);
        }

        // emission
        const json& emission = data["emission"];
        {
            cps->emission.cyclic = emission.value("cyclic", cps->emission.cyclic);
            cps->emission.interval = emission.value("interval", cps->emission.interval);
            cps->emission.count = emission.value("count", cps->emission.count);
            const std::string emitterType = emission.value("type", "point");
            if (emitterType == "line")        cps->emission.type = TCoreSystem::TEmission::Line;
            else if (emitterType == "square") cps->emission.type = TCoreSystem::TEmission::Square;
            else if (emitterType == "box")    cps->emission.type = TCoreSystem::TEmission::Box;
            else if (emitterType == "sphere") cps->emission.type = TCoreSystem::TEmission::Sphere;
            else                              cps->emission.type = TCoreSystem::TEmission::Point;

            cps->emission.size = loadVEC3(emission.value("size", "1 1 1"));
            cps->emission.angle = deg2rad(emission.value("angle", rad2deg(cps->emission.angle)));
        }

        // movement
        const json& movement = data["movement"];
        {
            cps->movement.velocity = movement.value("velocity", cps->movement.velocity);
            cps->movement.acceleration = movement.value("acceleration", cps->movement.acceleration);
            cps->movement.spin = deg2rad(movement.value("spin", rad2deg(cps->movement.spin)));
            cps->movement.wind = movement.value("wind", cps->movement.wind);
            cps->movement.gravity = movement.value("gravity", cps->movement.gravity);
            cps->movement.ground = movement.value("ground", cps->movement.ground);
        }

        // render
        const json& render = data["render"];
        {
            cps->render.initialFrame = render.value("initial_frame", cps->render.initialFrame);
            cps->render.frameSize = loadVEC2(render.value("frame_size", "1 1"));
            cps->render.numFrames = render.value("num_frames", cps->render.numFrames);
            cps->render.frameSpeed = render.value("frame_speed", cps->render.frameSpeed);
            cps->render.texture = Resources.get(render.value("texture", ""))->as<CTexture>();
        }

        // color
        const json& color = data["color"];
        {
            cps->color.opacity = color.value("opacity", cps->color.opacity);
            for (auto& clr : color["colors"])
            {
                float time = clr[0];
                VEC4 value = loadVEC4(clr[1]);
                cps->color.colors.set(time, value);
            }
            cps->color.colors.sort();
        }

        // size
        const json& size = data["size"];
        {
            cps->size.scale = size.value("scale", cps->size.scale);
            cps->size.scale_variation = size.value("scale_variation", cps->size.scale_variation);
            for (auto& sz : size["sizes"])
            {
                float time = sz[0];
                float value = sz[1];
                cps->size.sizes.set(time, value);
            }
            cps->size.sizes.sort();
        }

        return cps;*/
    }

    void CParser::writeFile(const TCoreSystem * system) {

        nlohmann::json jsonfile;
        TCoreSystem * n_system = const_cast<TCoreSystem*>(system);

        // Write life
        {
            jsonfile["life"]["duration"] = E_ROUND(system->life.duration);
            jsonfile["life"]["duration_variation"] = E_ROUND(system->life.durationVariation);
            jsonfile["life"]["max_particles"] = system->life.maxParticles;
            jsonfile["life"]["time_factor"] = E_ROUND(system->life.timeFactor);
        }

        // Write emission
        {
            jsonfile["emission"]["type"] = std::to_string(((int)system->emission.type));
            jsonfile["emission"]["size"] = std::to_string(E_ROUND(system->emission.size.x)) + " " + std::to_string(E_ROUND(system->emission.size.y)) + " " + std::to_string(E_ROUND(system->emission.size.z));
            jsonfile["emission"]["interval"] = E_ROUND(system->emission.interval);
            jsonfile["emission"]["count"] = system->emission.count;
            jsonfile["emission"]["cyclic"] = system->emission.cyclic;
        }

        // Write movement
        {
            jsonfile["movement"]["velocity"] = E_ROUND(system->movement.velocity);
            jsonfile["movement"]["acceleration"] = E_ROUND(system->movement.acceleration);
            jsonfile["movement"]["spin"] = E_ROUND(system->movement.spin);
            jsonfile["movement"]["wind"] = E_ROUND(system->movement.wind);
            jsonfile["movement"]["gravity"] = E_ROUND(system->movement.gravity);
        }

        // Write render
        {
            jsonfile["render"]["texture"] = system->render.texture->getName();
            jsonfile["render"]["frame_size"] = std::to_string(E_ROUND(system->render.frameSize.x)) + " " + std::to_string(E_ROUND(system->render.frameSize.y));
            jsonfile["render"]["initial_frame"] = system->render.initialFrame;
            jsonfile["render"]["num_frames"] = system->render.numFrames;
            jsonfile["render"]["frame_speed"] = E_ROUND(system->render.frameSpeed);
        }

        // Write size
        {
            jsonfile["size"]["scale"] = round(system->size.scale);
            jsonfile["size"]["scale_variation"] = round(system->size.scale_variation);

            json j_objects = json::array();
            for (int i = 0; i < n_system->size.sizes.keyframes.size(); i++) {
                j_objects.push_back(n_system->size.sizes.keyframes[i].value);
            }
                    //j_objects.push_back(n_system->size.sizes.keyframes[i].value);

            jsonfile["size"]["sizes"] = j_objects;
        }

        // Write color
        {
            jsonfile["color"]["opacity"] = round(system->color.opacity);
            //jsonfile["color"]["acceleration"] = system->color.colors;
        }

        std::string finalname = system->getName() + ".tmp";
        std::fstream file(system->getName());
        file << jsonfile;
    }
}

