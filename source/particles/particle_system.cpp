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
    _time = 0.f;
    emit();
  }

  bool CSystem::update(float delta)
  {
    const VEC3& kWindVelocity = Engine.get().getParticles().getWindVelocity();

    float fadeRatio = 1.f;
    if (_fadeDuration != 0.f)
    {
      _fadeTime += delta;
      fadeRatio = 1.f - (_fadeTime / _fadeDuration);
    }

    delta *= _core->life.timeFactor;

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
        VEC3 dir = p.velocity;
        dir.Normalize();
        p.velocity += dir * _core->movement.acceleration * delta;
        p.velocity += kGravity * _core->movement.gravity * delta;
        p.position += p.velocity * delta;
        p.position += kWindVelocity * _core->movement.wind * delta;
        p.rotation += _core->movement.spin * delta;
        if (_core->movement.ground)
        {
          p.position.y = std::max(0.f, p.position.y);
        }

        float life_ratio = p.max_lifetime > 0.f ? clamp(p.lifetime / p.max_lifetime, 0.f, 1.f) : 1.f;
        p.color = _core->color.colors.get(life_ratio) * fadeRatio;
        p.color.z *= _core->color.opacity;
        p.size = _core->size.sizes.get(life_ratio);

        int frame_idx = (int)(p.lifetime * _core->render.frameSpeed);
        p.frame = _core->render.initialFrame + (frame_idx % _core->render.numFrames);

        ++it;
      }
    }

    // check if a new batch of particles needs to be generated
    if (_core->emission.cyclic && _core->emission.interval > 0.f) {
      _time += delta;
      if (_time > _core->emission.interval) {
        emit();
        _time -= _core->emission.interval;
      }
    }

    return fadeRatio > 0.f && (!_particles.empty() || _core->emission.cyclic);
  }

  void CSystem::render()
  {
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
    }
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
    }
  }

  VEC3 CSystem::generatePosition() const
  {
    const float& size = _core->emission.size;

    switch (_core->emission.type)
    {
    case TCoreSystem::TEmission::Point:
      return VEC3::Zero;

    case TCoreSystem::TEmission::Line:
      return VEC3(random(-size, size), 0.f, 0.f);

    case TCoreSystem::TEmission::Square:
      return VEC3(random(-size, size), 0.f, random(-size, size));

    case TCoreSystem::TEmission::Box:
      return VEC3(random(-size, size), random(-size, size), random(-size, size));

    case TCoreSystem::TEmission::Sphere:
    {
      VEC3 dir(random(-1, 1), random(-1, 1), random(-1, 1));
      dir.Normalize();
      return dir * random(0, size);
    }
    }

    return VEC3::Zero;
  }

  VEC3 CSystem::generateVelocity() const
  {
    const float& angle = _core->emission.angle;
    const float velocity = _core->movement.velocity;

    if (angle != 0.f)
    {
      float radius = tan(angle);
      float x = sqrtf(radius) * cosf(angle) * random(-1, 1);
      float z = sqrtf(radius) * sinf(angle) * random(-1, 1);
      VEC3 pos(x, 1.f, z);
      pos.Normalize();
      return pos * velocity;
    }

    return VEC3::Up * velocity;
  }

  TParticleHandle CSystem::getHandle() const
  {
    return _handle;
  }

  void CSystem::fadeOut(float duration)
  {
    _fadeDuration = duration;
    _fadeTime = 0.f;
  }

}
