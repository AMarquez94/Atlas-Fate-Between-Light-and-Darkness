#include "mcv_platform.h"
#include "particle_system.h"
#include "particle_parser.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"
#include <random>
#include "render/render_manager.h"
#include "noise/FastNoiseSIMD.h"
#include "noise/Perlin.h"

// ----------------------------------------------
class CParticleResourceClass : public CResourceClass {
public:
    CParticleResourceClass() {
        class_name = "Particles";
        extensions = { ".particles" };
    }
    IResource* create(const std::string& name) const override {
        dbg("Creating particles %s\n", name.c_str());
        Particles::CParser parser;
        Particles::TCoreSystem* res = parser.parseParticlesFile(name);
        assert(res);
        return res;
    }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<Particles::TCoreParticleSystem>, use this function:
template<>
const CResourceClass* getResourceClassOf<Particles::TCoreSystem>() {
    static CParticleResourceClass the_resource_class;
    return &the_resource_class;
}
// ----------------------------------------------

namespace
{
    float random(float start, float end)
    {
        static std::default_random_engine e;
        static std::uniform_real<float> d{ 0, 1 };
        return lerp(start, end, d(e));
    }

    const VEC3 kGravity(0.f, -9.8f, 0.f);
}

namespace Particles
{
    TParticleHandle CSystem::_lastHandle = 0;

    void Particles::TCoreSystem::onFileChanged(const std::string& filename) {
        std::string fname = getName();
        if (filename != fname)
            return;

        destroy();
        Particles::CParser parser;
        *this = *(parser.parseParticlesFile(filename));
        this->setNameAndClass(filename, getResourceClassOf<Particles::TCoreSystem>());
    }

    void Particles::TCoreSystem::destroy()
    {

    }

    CSystem::CSystem(const TCoreSystem* core, CHandle entity)
        : _core(core)
        , _entity(entity)
    {
        assert(_core);
        _handle = ++_lastHandle;
    }

    void CSystem::launch()
    {
        _enabled = true;
        _time = 0.f;
        _deploy_time = 0.f;

        if(!_core->n_system.looping)
            emit(_core->n_emission.rate_time);

        // Copy the bursts to our structure
        for (auto it = _core->n_emission.bursts.begin(); it != _core->n_emission.bursts.end();)
        {
            // Do a deep copy instead of this...
            Particles::TNBurst p = Particles::TNBurst();
            p.count = it->count;
            p.cycles = it->cycles;
            p.interval = it->interval;
            p.i_elapsed = it->i_elapsed;
            p.time = it->time;

            bursts.push_back(p);
            ++it;
        }

        // Fast trick to determine additive type. Find an smarter way.
        if (_core->n_renderer.tech.find("additive") != std::string::npos)
            this->type = ADD;
        
    }

    void CSystem::debugInMenu() {
    
        TCoreSystem * system = const_cast<TCoreSystem*>(_core);

        ImGui::Checkbox("Enabled", &_enabled);

        /*if (ImGui::CollapsingHeader("Life")) {

            ImGui::DragFloat("Duration", &system->life.duration, 0.01f, 0.f, 50.f);
            ImGui::DragFloat("Duration Variation", &system->life.durationVariation, 0.01f, 0.f, 100.f);
            ImGui::DragFloat("Time Factor", &system->life.timeFactor, 0.01f, 0, 100.f);
            ImGui::DragInt("Max. Particles", &system->life.maxParticles, 1, 1, 100);
            //ImGui::ColorPicker4("##dummypicker", &_core->color.colors.get().x);
            ImGui::Separator();
        }

        if (ImGui::CollapsingHeader("Emission")) {

            ImGui::Checkbox("Looping", &system->emission.cyclic);
            
            ImGui::DragInt("Count", &system->emission.count, 1, 1, 500);
            ImGui::DragFloat3("Size", &system->emission.size.x, 0.1f, 0.f, 100.f);
            ImGui::DragFloat("Angle", &system->emission.angle, 0.1f, 0.1f, 500.f);
            ImGui::DragFloat("Interval", &system->emission.interval, 0.01f, 0.f, 100.f);
            ImGui::Separator();
        }

        if (ImGui::CollapsingHeader("Movement")) {

            ImGui::DragFloat("Velocity", &system->movement.velocity, 0.1f, 0.f, 100.f);
            ImGui::DragFloat("Acceleration", &system->movement.acceleration, 0.1f, 0.1f, 500.f);
            ImGui::DragFloat("Spin", &system->movement.spin, 0.01f, 0.f, 100.f);
            ImGui::DragFloat("Gravity", &system->movement.gravity, 0.01f, -100.f, 100.f);
            ImGui::DragFloat("Wind", &system->movement.wind, 0.01f, 0.f, 100.f);
            ImGui::Checkbox("Limit by ground", &system->movement.ground);
            ImGui::Separator();
        }

        if (ImGui::CollapsingHeader("Render")) {

            //((CTexture*)system->render.texture->debugInMenu();
            ImGui::DragFloat2("Frame size", &system->render.frameSize.x, 0.025f, 0.1f, 100.f);
            ImGui::DragInt("Frame num.", &system->render.numFrames, 1, 1, 500);
            ImGui::DragInt("Frame init.", &system->render.initialFrame, 1, 1, 100);
            ImGui::DragFloat("Frame speed", &system->render.frameSpeed, 0.01f, 0.f, 100.f);
            ImGui::Separator();
        }

        if (ImGui::CollapsingHeader("Size")) {

            ImGui::DragFloat("Scale", &system->size.scale, 0.01f, 0.f, 100.f);
            ImGui::DragFloat("Scale variation", &system->size.scale_variation, 0.01f, 0.f, 100.f);
            //TTrack<float> sizes;            // track of sizes along the particle lifetime

            ImGui::Separator();
        }

        if (ImGui::CollapsingHeader("Color")) {

            //ImGui::ColorPicker4("Color", &system->color.colors.get(0.f).x);
            ImGui::DragFloat("Opacity", &system->color.opacity, 0.01f, 0.f, 100.f);
            ImGui::Separator();
        }

        ImGui::Button("Save Configuration");
        if (ImGui::IsItemClicked(0))
        {
            // Save the resource to a new file.
            Particles::CParser parser;
            parser.writeFile(_core);
        }

        ImGui::Button("Reset to default");
        if (ImGui::IsItemClicked(0))
        {
            // Create a new brand resource and destroy the old one.
            TCoreSystem * resource = (TCoreSystem *)_core->getClass()->create(_core->getName());
            resource->setNameAndClass(_core->getName(), _core->getClass());
            ((IResource*)_core)->destroy(); 

            _core = resource;
            system = resource;
            
            // Reset the resource to it's original values
        }

        _core = system;*/
    }

    bool CSystem::update(float delta)
    {
        // Handle deployment
        {
            if (!_entity.isValid() || !_enabled) return true;

            // Handle start delay
            _deploy_time += delta;
            if (_deploy_time < _core->n_system.start_delay) return true;
        }

        // Handle core update
        {
            updateFading(delta);
            delta *= _core->n_system.simulation_speed;

            updateSystem(delta);
            updateEmission(delta);
        }

        return _fadeRatio > 0.f && (!_particles.empty() || _core->n_system.looping);
    }

    void CSystem::updateSystem(float delta) {

        MAT44 world = MAT44::Identity;
        MAT44 world_rot = MAT44::Identity;
        const siv::PerlinNoise perlin(12345);

        if (_entity.isValid())
        {
            CEntity* e = _entity;
            TCompTransform* e_transform = e->get<TCompTransform>();
            world = e_transform->asMatrix();
            VEC3 proj_vector = e_transform->getFront();

            if(_core->n_velocity.type == 0) // Local coordinates...
                world_rot = MAT44::CreateFromQuaternion(e_transform->getRotation());

            const VEC3& kWindVelocity = EngineParticles.getWindVelocity();

            int amount = 0;
            for (auto it = _particles.begin(); it != _particles.end();)
            {
                TParticle& p = *it;
                p.lifetime += delta;

                if (p.max_lifetime > 0.f && p.lifetime >= p.max_lifetime)
                {
                    it = _particles.erase(it);
                }
                else
                {
                    float life_ratio = p.max_lifetime > 0.f ? clamp(p.lifetime / p.max_lifetime, 0.f, 1.f) : 1.f;
                    p.velocity = _core->n_system.start_speed * p.origin_velocity;
                    // Compute the noise, disable if it gives bad fps
                    p.velocity += VEC3::Transform(_core->n_velocity.velocity.get(life_ratio), world_rot) * _core->n_velocity.acceleration * delta;
                    p.downforce += kGravity * _core->n_system.gravity * delta;
                    p.velocity += p.downforce;

                    if (_core->n_noise.strength > 0)
                    {
                        p.random_direction = VEC3(p.random_direction.x + random(-0.1, 0.1), p.random_direction.y + random(-0.1, 0.1), p.random_direction.z + random(-0.1, 0.1));
                        p.velocity += p.random_direction * _core->n_noise.strength * delta;
                    }

                    // Inherit velocity if needed
                    p.position = _core->n_velocity.inherit_velocity ? VEC3::Transform(p.origin_position, world) : p.position;
                    p.position += p.velocity * delta;
                    p.position += kWindVelocity * _core->n_velocity.wind * delta;
                    p.rotation += _core->n_velocity.rotation.get(life_ratio) * delta;

                    p.color = _core->n_color.colors.get(life_ratio) * _fadeRatio;
                    p.color.w *= _core->n_color.opacity;
                    p.size = _core->n_size.sizes.get(life_ratio);

                    int frame_idx = (int)(p.lifetime * _core->n_renderer.frameSpeed);
                    p.frame = p.init_frame + _core->n_renderer.initialFrame + (frame_idx % _core->n_renderer.numFrames);

                    ++it;
                }

                amount = amount + 3;
            }

        }
    }

    void CSystem::updateEmission(float delta) {

        // Rate over time (time elpased)
        if (_core->n_system.looping && _core->n_emission.rate_time > 0.f) {
            _time += delta;

            if (_time > _core->n_emission.time_ratio) {

                _time = 0;
                emit(1);
            }
        }

        // Rate over distance (unit elapsed)
        if (_core->n_emission.rate_distance > 0) {

            CEntity * ent = _entity;
            TCompTransform * c_ent_transform = ent->get<TCompTransform>();
            _deploy_distance += (_lastSystemPosition - c_ent_transform->getPosition()).Length();

            if (_deploy_distance > EngineParticles.rate_min_dist)
            {
                _deploy_distance = 0;
                emit(_core->n_emission.rate_distance);
            }

            _lastSystemPosition = c_ent_transform->getPosition();
        }

        // Bursts (concrete deploy)
        for (auto it = _core->n_emission.bursts.begin(); it != _core->n_emission.bursts.end();)
        {
            Particles::TNBurst p = *it;
            Particles::TNBurst & c = bursts[it - _core->n_emission.bursts.begin()];

            if (_deploy_time > p.time) {
                c.i_elapsed += delta; // Update the burst

                // New burst to be deployed
                if (c.i_elapsed > p.interval && c.cycles > 0) {

                    emit(p.count);
                    c.cycles--;
                    c.i_elapsed = 0;
                }
            }

            ++it;
        }
    }

    void CSystem::updateFading(float delta) {

        if (_fadeDuration != 0.f)
        {
            _fadeTime += delta;
            _fadeRatio = (1.f - (_fadeTime / _fadeDuration));
        }

        _fadeRatio = 1.0f;
    }

    void CSystem::updateCollision(float delta) {


    }

    // Emit given a concrete amount
    void CSystem::emit(int amount)
    {
        MAT44 world = MAT44::Identity;
        MAT44 world_rot = MAT44::Identity;

        if (_entity.isValid())
        {
            CEntity* e = _entity;
            TCompTransform* e_transform = e->get<TCompTransform>();
            world = e_transform->asMatrix();
            world_rot = MAT44::CreateFromQuaternion(e_transform->getRotation());
        }

        // Emit given an amount with the default initial values.
        for (int i = 0; i < amount && _particles.size() < _core->n_system.max_particles; ++i)
        {
            TParticle particle;
            particle.origin_position = generatePosition() + _core->n_system.offset;
            particle.position = VEC3::Transform(particle.origin_position, world);
            particle.velocity = VEC3::Transform(generateVelocity(), world_rot);
            particle.random_direction = AddNoiseOnAngle(-180, 180);
            particle.origin_velocity = particle.velocity;
            particle.color = _core->n_color.colors.get(0.f);
            particle.size = _core->n_system.start_size * _core->n_size.sizes.get(0.f);
            particle.scale = _core->n_size.scale + random(-_core->n_size.scale_variation, _core->n_size.scale_variation);
            particle.frame = _core->n_renderer.initialFrame;
            particle.init_frame = random(0, _core->n_renderer.numFrames);
            //particle.rotation = (1 - _core->n_system.random_rotation) * M_PI * VEC3(deg2rad(_core->n_system.start_rotation.x), deg2rad(_core->n_system.start_rotation.y), deg2rad(_core->n_system.start_rotation.z));
            particle.rotation = _core->n_system.start_rotation + _core->n_system.random_rotation * M_PI * _core->n_system.start_rotation;
            particle.lifetime = 0.f;
            particle.max_lifetime = _core->n_system.duration + random(-_core->n_emission.variation, _core->n_emission.variation);
            particle.is_update = false;

            _particles.push_back(particle);
        }
    }

    VEC3 CSystem::generatePosition() const
    {
        const VEC3& size = _core->n_shape.size;

        switch (_core->n_shape.type)
        {
            case TCoreSystem::TNShape::Point:
                return VEC3::Zero;

            case TCoreSystem::TNShape::Line:
                return VEC3(random(-size.x, size.x), 0.f, 0.f)*.5f;

            case TCoreSystem::TNShape::Square:
                return VEC3(random(-size.x, size.x), 0.f, random(-size.z, size.z)) *.5f;

            case TCoreSystem::TNShape::Box:
                return VEC3(random(-size.x, size.x), random(-size.y, size.y), random(-size.z, size.z)) *.5f;

            case TCoreSystem::TNShape::Sphere:
            {
                VEC3 dir(random(-1, 1), random(-1, 1), random(-1, 1));
                dir.Normalize();
                return _core->n_shape.shell_emit ? dir  * size.x : dir * random(0, size.x);
            }

            case TCoreSystem::TNShape::Circle:
            {
                VEC3 dir(random(-1, 1), 0, random(-1, 1));
                dir.Normalize();
                return _core->n_shape.shell_emit ? dir * size.x : dir * random(0, size.x);
            }

            case TCoreSystem::TNShape::Cone:
            {
                VEC3 dir(random(-1, 1), 0, random(-1, 1));
                dir.Normalize();
                return _core->n_shape.shell_emit ? dir * size.x : dir * random(0, size.x);
            }
        }

        return VEC3::Zero;
    }

    VEC3 CSystem::generateDirection() const
    {
        switch (_core->n_shape.type)
        {
            case TCoreSystem::TNShape::Point:
                return VEC3::Up;

            case TCoreSystem::TNShape::Line:
                return VEC3::Up;

            case TCoreSystem::TNShape::Square:
                return VEC3::Up;

            case TCoreSystem::TNShape::Box:
                return VEC3::Forward;

            case TCoreSystem::TNShape::Sphere:
            {
                VEC3 dir(random(-1, 1), random(-1, 1), random(-1, 1));
                dir.Normalize();
                return dir;
            }

            case TCoreSystem::TNShape::Circle:
            {
                VEC3 dir(random(-1, 1), 0, random(-1, 1));
                dir.Normalize();
                return dir;
            }

            case TCoreSystem::TNShape::Cone:
            {                
                float angle = _core->n_shape.angle / M_PI_2;
                VEC3 dir(random(-angle, angle), 1, random(-angle, angle));
                dir.Normalize();
                return dir;
            }
        }

        return VEC3::Zero;
    }

    VEC3 CSystem::generateVelocity() const
    {
        const float& angle = _core->n_shape.angle;
        const VEC3 velocity = _core->n_velocity.constant_velocity; // TO-REFACTOR

        /*if (angle != 0.f)
        {
            float radius = tan(angle);
            float x = sqrtf(radius) * cosf(angle) * random(-1, 1);
            float z = sqrtf(radius) * sinf(angle) * random(-1, 1);
            VEC3 pos(x, 1.f, z);
            pos.Normalize();
            return pos * velocity;
        }*/

        return generateDirection();// *velocity;
    }

    // To update this with the compute shader.
    void CSystem::render()
    {
        if (!_enabled || !_entity.isValid()) return;
        if (_deploy_time < _core->n_system.start_delay) return;

        //Hardcoded, move it from here.
        CEntity * ent = _entity;
        TCompTransform * c_ent_transform = ent->get<TCompTransform>();
        VEC3 proj_vector = c_ent_transform->getFront();

        std::vector<Particles::TIParticle> particles_instances;
        particles_instances.reserve(_particles.size());

        float frameX = 1 / _core->n_renderer.frameSize.x;
        float frameY = 1 / _core->n_renderer.frameSize.y;
        const int frameCols = static_cast<int>(_core->n_renderer.frameSize.x);

        float length = 1;
        VEC3 camera_pos, camera_up;
        getRenderMode(camera_pos, camera_up, length);

        for (auto& p : _particles)
        {
            // We need to update the strecthed billboard with the given info for each particle.
            VEC3 pos = p.is_update ? c_ent_transform->getPosition() + p.position : p.position;
            VEC3 def_position = pos + camera_pos;

            if (_core->n_renderer.mode == TCoreSystem::TNRenderer::EMODE::STRETCHED)
            {
                camera_up = p.velocity;
                camera_up.Normalize();
                def_position = p.velocity.Cross(camera_pos);
                def_position.Normalize();
                def_position = p.position + def_position;
            }

            MAT44 bb = MAT44::CreateBillboard(pos, def_position, camera_up);
            MAT44 sc = MAT44::CreateScale(p.size * p.scale * VEC3(1, length, 1));
            MAT44 rt = MAT44::CreateFromYawPitchRoll(p.rotation.x, p.rotation.y, p.rotation.z);

            int row = p.frame / frameCols;
            int col = p.frame % frameCols;
            VEC3 minUV = VEC3(col * frameX, row * frameY, 0);
            VEC3 maxUV = minUV + VEC3(frameX, frameY, _core->n_renderer.softness);

            Particles::TIParticle t_struct = { sc * rt * bb, minUV, maxUV, p.color };
            particles_instances.push_back(t_struct);
        }

        // Replace this when needed.
        {
            _core->n_renderer.texture->activate(TS_ALBEDO1);
            EngineParticles.instanced_particle->setInstancesData(particles_instances.data(), particles_instances.size(), sizeof(Particles::TIParticle));
            EngineParticles.instanced_particle->renderSubMesh(EngineParticles.instanced_particle->subgroups[0].first_idx);
        }
    }


    void CSystem::getRenderMode(VEC3 & camera_pos, VEC3 & camera_up, float & length)
    {
        CEntity * ent = _entity;
        TCompTransform * c_ent_transform = ent->get<TCompTransform>();
        assert(c_ent_transform);

        CEntity* eCurrentCamera = Engine.getCameras().getOutputCamera();
        assert(eCurrentCamera);
        TCompCamera* camera = eCurrentCamera->get<TCompCamera>();
        assert(camera);

        if (_core->n_renderer.mode == TCoreSystem::TNRenderer::EMODE::HORIZONTAL) {
            camera_pos = VEC3(0, 1, 0);
            camera_up = VEC3(1,0,0);
        }
        else if (_core->n_renderer.mode == TCoreSystem::TNRenderer::EMODE::VERTICAL) {
            camera_pos = VEC3(-camera->getFront().x, 0, -camera->getFront().z);
            camera_up = VEC3(0, 1, 0);
        }
        else if (_core->n_renderer.mode == TCoreSystem::TNRenderer::EMODE::STRETCHED) {
            camera_pos = camera->getLeft();
            camera_up = camera->getUp();
            length = _core->n_renderer.length;
        }
        else {
            camera_pos = -camera->getFront();
            camera_up = camera->getUp();
        }

    }

    TParticleHandle CSystem::getHandle() const
    {
        return _handle;
    }

    void CSystem::setActive(bool active) {

        _enabled = active;
    }

    void CSystem::fadeOut(float duration)
    {
        _fadeDuration = duration;
        _fadeTime = 0.f;
    }

    VEC3 CSystem::AddNoiseOnAngle(float min, float max) {

        float xNoise = random(min, max);
        float yNoise = random(min, max);
        float zNoise = random(min, max);
        //VEC3 noize = VEC3(xNoise, yNoise, zNoise);
        //noize.Normalize();
        //return noize;

        // Convert Angle to Vector3
        VEC3 noise = VEC3(
            sin(2 * M_PI * xNoise / 360),
            sin(2 * M_PI * yNoise / 360),
            sin(2 * M_PI * zNoise / 360)
        );
        return noise;
    }
}
