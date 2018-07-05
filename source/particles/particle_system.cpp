#include "mcv_platform.h"
#include "particle_system.h"
#include "particle_parser.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"
#include <random>

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
        emit();
    }

    void CSystem::debugInMenu() {
    
        TCoreSystem * system = const_cast<TCoreSystem*>(_core);

        ImGui::Checkbox("Enabled", &_enabled);

        if (ImGui::CollapsingHeader("Life")) {

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

        _core = system;
    }

    bool CSystem::update(float delta)
    {
        if (!_enabled) return true;
        
        // Handle start delay
        _deploy_time += delta;
        if (_deploy_time < _core->n_system.start_delay) return true;

        const VEC3& kWindVelocity = Engine.get().getParticles().getWindVelocity();

        float fadeRatio = 1.f;
        if (_fadeDuration != 0.f)
        {
            _fadeTime += delta;
            fadeRatio = 1.f - (_fadeTime / _fadeDuration);
        }

        delta *= _core->n_system.simulation_speed;

        auto it = _particles.begin();
        while (it != _particles.end())
        {
            TParticle& p = *it;

            p.lifetime += delta;
           
            if (p.max_lifetime > 0.f && p.lifetime >= p.max_lifetime)
            {
                it = _particles.erase(it);
            }
            else
            {
                CEntity * ent = _entity;
                TCompTransform * c_ent_transform = ent->get<TCompTransform>();

                float life_ratio = p.max_lifetime > 0.f ? clamp(p.lifetime / p.max_lifetime, 0.f, 1.f) : 1.f;

                VEC3 dir = p.velocity;
                dir.Normalize();
                p.velocity += dir * _core->n_velocity.acceleration * delta * _core->n_velocity.velocity.get(life_ratio); //Velocity over lifetime
                p.velocity += kGravity * _core->n_system.gravity * delta;
                p.velocity += AddNoiseOnAngle(-180, 180) * _core->n_noise.strength;
                p.position += p.velocity * delta;
                p.position += kWindVelocity * _core->n_velocity.wind * delta;
                p.rotation += delta * _core->n_velocity.rotation.get(life_ratio);

                if (_core->n_collision.collision)
                    p.position.y = std::max(0.f, p.position.y);

                p.color = _core->n_color.colors.get(life_ratio) * fadeRatio;
                p.color.w *= _core->n_color.opacity;
                p.size = _core->n_size.sizes.get(life_ratio);

                int frame_idx = (int)(p.lifetime * _core->n_renderer.frameSpeed);
                p.frame = _core->n_renderer.initialFrame + (frame_idx % _core->n_renderer.numFrames);

                ++it;
            }
        }

        // Rate over time
        if (_core->n_system.looping && _core->n_emission.interval > 0.f) {
            _time += delta;
            if (_time > _core->n_emission.interval) {
                emit();
                _time -= _core->n_emission.interval;
            }
        }

        // Rate over distance
        if (_core->n_emission.rate_distance > 0) {

            CEntity * ent = _entity;
            TCompTransform * c_ent_transform = ent->get<TCompTransform>();
            _deploy_distance += (_lastSystemPosition - c_ent_transform->getPosition()).Length();
            if (_deploy_distance > 0.05)
            {
                _deploy_distance = 0;
                emit(_core->n_emission.rate_distance);
            }

            _lastSystemPosition = c_ent_transform->getPosition();
        }

        return fadeRatio > 0.f && (!_particles.empty() || _core->n_system.looping);
    }

    // To update this with the compute shader.
    void CSystem::render()
    {
        if (!_enabled) return;
        if (_deploy_time < _core->n_system.start_delay) return;

        const CRenderTechnique* technique = Resources.get("particles.tech")->as<CRenderTechnique>();
        const CRenderMesh* quadMesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
        CEntity* eCurrentCamera = Engine.getCameras().getOutputCamera();
        assert(technique && quadMesh && eCurrentCamera);
        TCompCamera* camera = eCurrentCamera->get< TCompCamera >();
        assert(camera);
        VEC3 cameraPos = camera->getPosition();
        VEC3 cameraUp = camera->getUp();

        //Hardcoded, move it from here.
        CEntity * ent = _entity;
        TCompTransform * c_ent_transform = ent->get<TCompTransform>();
        if (_core->n_renderer.mode == TCoreSystem::TNRenderer::EMODE::HORIZONTAL) {
            cameraPos = c_ent_transform->getPosition() + c_ent_transform->getUp();
            cameraUp = c_ent_transform->getFront();
        }

        const int frameCols = static_cast<int>(_core->n_renderer.frameSize.x);

        technique->activate();
        _core->n_renderer.texture->activate(TS_ALBEDO);

        for (auto& p : _particles)
        {
            VEC3 pos = p.is_update ? c_ent_transform->getPosition() + p.position : p.position;
            MAT44 bb = MAT44::CreateBillboard(pos, cameraPos, cameraUp);
            MAT44 sc = MAT44::CreateScale(p.size * p.scale);
            MAT44 rt = MAT44::CreateFromYawPitchRoll(p.rotation.x, p.rotation.y, p.rotation.z);

            int row = p.frame / frameCols;
            int col = p.frame % frameCols;
            VEC2 minUV = VEC2(col * (1/_core->n_renderer.frameSize.x), row * (1 / _core->n_renderer.frameSize.y));
            VEC2 maxUV = minUV + VEC2(1/_core->n_renderer.frameSize.x, 1/ _core->n_renderer.frameSize.y);

            cb_object.obj_world = rt * sc * bb;
            cb_object.obj_color = VEC4(1, 1, 1, 1);
            cb_object.updateGPU();

            cb_particles.particle_minUV = minUV;
            cb_particles.particle_maxUV = maxUV;
            cb_particles.particle_color = p.color;
            cb_particles.updateGPU();

            quadMesh->activateAndRender();
        }

        /*
        const CRenderTechnique* technique = Resources.get("particles.tech")->as<CRenderTechnique>();
        const CRenderMesh* quadMesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
        CEntity* eCurrentCamera = Engine.getCameras().getOutputCamera();
        assert(technique && quadMesh && eCurrentCamera);
        TCompCamera* camera = eCurrentCamera->get< TCompCamera >();
        assert(camera);
        const VEC3 cameraPos = camera->getPosition();
        const VEC3 cameraUp = camera->getUp();

        const int frameCols = static_cast<int>(1.f / _core->render.frameSize.x);

        technique->activate();
        _core->render.texture->activate(TS_ALBEDO);

        for (auto& p : _particles)
        {
            MAT44 bb = MAT44::CreateBillboard(p.position, cameraPos, cameraUp);
            MAT44 sc = MAT44::CreateScale(p.size * p.scale);
            MAT44 rt = MAT44::CreateFromYawPitchRoll(0.f, 0.f, p.rotation);

            int row = p.frame / frameCols;
            int col = p.frame % frameCols;
            VEC2 minUV = VEC2(col * _core->render.frameSize.x, row * _core->render.frameSize.y);
            VEC2 maxUV = minUV + _core->render.frameSize;

            cb_object.obj_world = rt * sc * bb;
            cb_object.obj_color = VEC4(1, 1, 1, 1);
            cb_object.updateGPU();

            cb_particles.particle_minUV = minUV;
            cb_particles.particle_maxUV = maxUV;
            cb_particles.particle_color = p.color;
            cb_particles.updateGPU();

            quadMesh->activateAndRender();
        }*/
    }

    void CSystem::emit()
    {
        MAT44 world = MAT44::Identity;
        QUAT rotation = QUAT::Identity;
        if (_entity.isValid())
        {
            CEntity* e = _entity;
            TCompTransform* e_transform = e->get<TCompTransform>();
            world = e_transform->asMatrix();
            rotation = e_transform->getRotation();
        }

        for (int i = 0; i < _core->n_emission.rate_time && _particles.size() < _core->n_system.max_particles; ++i)
        {
            TParticle particle;
            particle.position = generatePosition();
            particle.velocity = generateVelocity();
            particle.color = _core->n_color.colors.get(0.f);
            particle.size = _core->n_system.start_size;
            particle.scale = _core->n_size.scale + random(-_core->n_size.scale_variation, _core->n_size.scale_variation);
            particle.frame = _core->n_renderer.initialFrame;
            particle.rotation = (1 - _core->n_system.random_rotation) * M_PI * VEC3(deg2rad(_core->n_system.start_rotation.x), deg2rad(_core->n_system.start_rotation.y), deg2rad(_core->n_system.start_rotation.z));
            particle.lifetime = 0.f;
            particle.max_lifetime = _core->n_system.duration + random(-_core->n_emission.variation, _core->n_emission.variation);

            _particles.push_back(particle);
        }

        /*
        
        for (int i = 0; i < _core->emission.count && _particles.size() < _core->life.maxParticles; ++i)
        {
            TParticle particle;
            particle.position = VEC3::Transform(generatePosition(), world);
            particle.velocity = generateVelocity();
            particle.color = _core->color.colors.get(0.f);
            particle.size = _core->size.sizes.get(0.f);
            particle.scale = _core->size.scale + random(-_core->size.scale_variation, _core->size.scale_variation);
            particle.frame = _core->render.initialFrame;
            particle.rotation = 0.f;
            particle.lifetime = 0.f;
            particle.max_lifetime = _core->life.duration + random(-_core->life.durationVariation, _core->life.durationVariation);

            _particles.push_back(particle);
        }*/
    }

    // Emit given a concrete amount
    void CSystem::emit(int amount)
    {
        MAT44 world = MAT44::Identity;
        QUAT rotation = QUAT::Identity;
        if (_entity.isValid())
        {
            CEntity* e = _entity;
            TCompTransform* e_transform = e->get<TCompTransform>();
            world = e_transform->asMatrix();
            rotation = e_transform->getRotation();
        }

        for (int i = 0; i < amount && _particles.size() < _core->n_system.max_particles; ++i)
        {
            TParticle particle;
            particle.position = VEC3::Transform(generatePosition(), world);
            particle.velocity = generateVelocity();
            particle.color = _core->n_color.colors.get(0.f);
            particle.size = _core->n_size.sizes.get(0.f);
            particle.scale = _core->n_size.scale + random(-_core->n_size.scale_variation, _core->n_size.scale_variation);
            particle.frame = _core->n_renderer.initialFrame;
            particle.rotation = _core->n_system.random_rotation * M_PI * _core->n_system.start_rotation;
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
                return dir * random(0, size.x);
            }

            case TCoreSystem::TNShape::Circle:
            {
                VEC3 dir(random(-1, 1), 0, random(-1, 1));
                dir.Normalize();
                return dir * random(0, size.x);
            }

            case TCoreSystem::TNShape::Cone:
            {
                VEC3 dir(random(-1, 1), 0, random(-1, 1));
                dir.Normalize();
                return dir * random(0, size.x);
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
                return VEC3::Right;

            case TCoreSystem::TNShape::Square:
                return VEC3::Up;

            case TCoreSystem::TNShape::Box:
                return VEC3::Up;

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
                return VEC3::Up;
                //float angle = 1 - (rad2deg(_core->n_shape.angle) / 90);
                //VEC3 dir(random(angle, 1), 1, random(angle, 1));
                //dir.Normalize();
                //return dir;
            }
        }

        return VEC3::Zero;
    }

    VEC3 CSystem::generateVelocity() const
    {
        const float& angle = _core->n_shape.angle;
        const VEC3 velocity = _core->n_velocity.constant_velocity; // TO-REFACTOR

        if (angle != 0.f)
        {
            float radius = tan(angle);
            float x = sqrtf(radius) * cosf(angle) * random(-1, 1);
            float z = sqrtf(radius) * sinf(angle) * random(-1, 1);
            VEC3 pos(x, 1.f, z);
            pos.Normalize();
            return pos * velocity;
        }

        return generateDirection() * velocity;
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

        // Convert Angle to Vector3
        VEC3 noise = VEC3(
            sin(2 * M_PI * xNoise / 360),
            sin(2 * M_PI * yNoise / 360),
            sin(2 * M_PI * zNoise / 360)
        );
        return noise;
    }
}
