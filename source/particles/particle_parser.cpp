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
            cps->n_system.start_size = loadVEC3(system.value("start_size", "1 1 1"));
            cps->n_system.start_rotation = loadVEC3(system.value("start_rotation", "0 0 0"));
            cps->n_system.random_rotation = system.value("random_rotation", cps->n_system.random_rotation);
            cps->n_system.start_color = loadVEC4(system.value("start_color", "1 1 1 1"));
            cps->n_system.gravity = system.value("gravity", cps->n_system.gravity);
            cps->n_system.simulation_speed = system.value("simulation_speed", cps->n_system.simulation_speed);
            cps->n_system.max_particles = system.value("max_particles", cps->n_system.max_particles);
            cps->n_system.start_rotation = VEC3(deg2rad(cps->n_system.start_rotation.x), cps->n_system.start_rotation.y, cps->n_system.start_rotation.z);
            cps->n_system.offset = loadVEC3(system.value("offset", "0 0 0"));
        }
        
        // emission
        const json& emission = data["emission"];
        {
            cps->n_emission.rate_time = emission.value("rate_time", cps->n_emission.rate_time);
            cps->n_emission.rate_distance = emission.value("rate_distance", cps->n_emission.rate_distance);
            cps->n_emission.interval = emission.value("interval", cps->n_emission.interval);
            cps->n_emission.variation = emission.value("variation", cps->n_emission.variation);
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
        const json& velocity = data["velocity"];
        {
            cps->n_velocity.constant_velocity = loadVEC3(velocity.value("constant_velocity", "1 1 1"));
            for (auto& vlt : velocity["velocity"])
            {
                float time = vlt[0];
                VEC3 value = loadVEC3(vlt[1]);
                cps->n_velocity.velocity.set(time, value);
            }

            for (auto& rot : velocity["rotation"])
            {
                float time = rot[0];
                VEC3 value = loadVEC3(rot[1]);
                cps->n_velocity.rotation.set(time, value);
            }
            cps->n_velocity.rotation.sort();

            cps->n_velocity.acceleration = velocity.value("acceleration", cps->n_velocity.acceleration);
            cps->n_velocity.wind = velocity.value("wind", cps->n_velocity.wind);
        }

        // render
        const json& render = data["renderer"];
        {
            const std::string renderMode = render.value("mode", "billboard");
            if (renderMode == "billboard")        cps->n_renderer.mode = TCoreSystem::TNRenderer::BILLBOARD;
            if (renderMode == "horizontal")        cps->n_renderer.mode = TCoreSystem::TNRenderer::HORIZONTAL;
            if (renderMode == "vertical")        cps->n_renderer.mode = TCoreSystem::TNRenderer::VERTICAL;

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

        // collision
        const json& collision = data["collision"];
        {
            cps->n_collision.collision = collision.value("collision", cps->n_collision.collision);
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
                VEC3 value = loadVEC3(sz[1]);
                cps->n_size.sizes.set(time, value);
            }
            cps->n_size.sizes.sort();
        }

        return cps;
    }

    void CParser::writeFile(const TCoreSystem * system) {

        nlohmann::json jsonfile;
        TCoreSystem * n_system = const_cast<TCoreSystem*>(system);

        // Write System
        {
            jsonfile["system"]["duration"] = E_ROUND(system->n_system.duration);
            jsonfile["system"]["looping"] = system->n_system.looping;
            jsonfile["system"]["start_delay"] = system->n_system.start_delay;
            jsonfile["system"]["start_lifetime"] = E_ROUND(system->n_system.start_lifetime);
            jsonfile["system"]["start_speed"] = E_ROUND(system->n_system.start_speed);
            jsonfile["system"]["start_size"] = stringify(system->n_system.start_size); 
            jsonfile["system"]["start_rotation"] = stringify(system->n_system.start_rotation);
            jsonfile["system"]["random_rotation"] = E_ROUND(system->n_system.random_rotation);
            jsonfile["system"]["start_color"] = stringify(system->n_system.start_color);
            jsonfile["system"]["gravity"] = E_ROUND(system->n_system.gravity);
            jsonfile["system"]["simulation_speed"] = E_ROUND(system->n_system.simulation_speed);
            jsonfile["system"]["max_particles"] =system->n_system.max_particles;
            jsonfile["system"]["offset"] = stringify(system->n_system.offset);
        }

        // Write emission
        {
            jsonfile["emission"]["rate_time"] = E_ROUND(system->n_emission.rate_time);
            jsonfile["emission"]["rate_distance"] = E_ROUND(system->n_emission.rate_distance);
            jsonfile["emission"]["variation"] = E_ROUND(system->n_emission.variation);
            jsonfile["emission"]["interval"] = E_ROUND(system->n_emission.interval);
        }

        // Write shape
        {
            jsonfile["shape"]["type"] = std::to_string(system->n_shape.type);
            jsonfile["shape"]["size"] = stringify(system->n_shape.size);
            jsonfile["shape"]["angle"] = system->n_shape.angle;
        }
        
        // Write velocity
        {
            jsonfile["velocity"]["constant_velocity"] = stringify(system->n_velocity.constant_velocity);
            json j_objects = json::array();
            for (int i = 0; i < system->n_velocity.velocity.keyframes.size(); i++) {
                j_objects.push_back(stringify(system->n_velocity.velocity.keyframes[i].value));
            }
            jsonfile["velocity"]["velocity"] = j_objects;         
            jsonfile["velocity"]["acceleration"] = E_ROUND(system->n_velocity.acceleration);
            jsonfile["velocity"]["wind"] = E_ROUND(system->n_velocity.wind);
        }
        
        // Write noise
        {
            jsonfile["noise"]["texture"] = system->n_noise.texture->getName();
            jsonfile["noise"]["strength"] = E_ROUND(system->n_noise.strength);
            jsonfile["noise"]["frequency"] = E_ROUND(system->n_noise.frequency);
            jsonfile["noise"]["scroll_speed"] = E_ROUND(system->n_noise.scroll_speed);
            jsonfile["noise"]["damping"] = E_ROUND(system->n_noise.damping);
            jsonfile["noise"]["octaves"] = system->n_noise.octaves;
        }
        
        // Write collision
        {
            jsonfile["collision"]["collision"] = system->n_collision.collision;
        }

        // Write render
        {
            jsonfile["renderer"]["mode"] = system->n_renderer.mode;
            jsonfile["renderer"]["texture"] = system->n_renderer.texture->getName();
            jsonfile["renderer"]["frame_size"] = stringify(system->n_renderer.frameSize);
            jsonfile["renderer"]["initial_frame"] = system->n_renderer.initialFrame;
            jsonfile["renderer"]["num_frames"] = system->n_renderer.numFrames;
            jsonfile["renderer"]["frame_speed"] = E_ROUND(system->n_renderer.frameSpeed);
        }

        // Write size
        {
            jsonfile["size"]["scale"] = round(system->n_size.scale);
            jsonfile["size"]["scale_variation"] = round(system->n_size.scale_variation);

            json j_objects = json::array();
            for (int i = 0; i < n_system->n_size.sizes.keyframes.size(); i++) {
                j_objects.push_back(stringify(n_system->n_size.sizes.keyframes[i].value));
            }
            //j_objects.push_back(n_system->size.sizes.keyframes[i].value);

            jsonfile["size"]["sizes"] = j_objects;
        }

        // Write color
        {
            jsonfile["color"]["opacity"] = round(system->n_color.opacity);
            json j_objects = json::array();
            for (int i = 0; i < n_system->n_color.colors.keyframes.size(); i++) {
                j_objects.push_back(stringify(n_system->n_color.colors.keyframes[i].value));
            }
            //j_objects.push_back(n_system->size.sizes.keyframes[i].value);

            jsonfile["color"]["colors"] = j_objects;
        }

        std::string finalname = system->getName() + ".tmp";
        std::ofstream out(finalname);
        out << jsonfile.dump();
        out.close();
        /*
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
        file << jsonfile;*/
    }
}
