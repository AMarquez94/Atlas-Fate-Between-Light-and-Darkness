#include "mcv_platform.h"
#include "module_particles.h"
#include "particles/particle_system.h"
#include "particles/particle_parser.h"
#include "particles/particle_editor.h"

CModuleParticles::CModuleParticles(const std::string& name)
    : IModule(name)
    , _lastHandle(0)
{}

bool CModuleParticles::start()
{
    Resources.registerResourceClass(getResourceClassOf<Particles::TCoreSystem>());

    /*Particles::CParser parser;
    parser.parseFile("data/particles.json");

    const Particles::TCoreSystem* fire = Resources.get("data/particles/fire.particles")->as<Particles::TCoreSystem>();
    launchSystem(fire);*/

    _windVelocity = VEC3(1, 0, 0);

    p_editor = new ParticlesEditor();
    p_editor->Init();

    return true;
}

bool CModuleParticles::stop()
{
    return true;
}

void CModuleParticles::update(float delta)
{
    for (auto it = _activeSystems.begin(); it != _activeSystems.end();)
    {
        Particles::CSystem* ps = *it;

        bool active = ps->update(delta);
        if (!active)
        {
            // Destroy the entity if it's marked as destroyable entity
            if (ps->_destroy_entity)
                EngineLogic.execScript("destroyHandle(" + std::to_string(ps->getHandle()) + ")");

            delete ps;
            it = _activeSystems.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void CModuleParticles::renderDeferred()
{
    for (auto& ps : _activeSystems)
    {
        ps->render();
    };
}

void CModuleParticles::renderMain()
{
    //for (auto& ps : _activeSystems)
    //{
    //    ps->render();
    //};

    if(particles_enabled)
        p_editor->debugMenu();
}

Particles::TParticleHandle CModuleParticles::launchSystem(const std::string& name, CHandle entity)
{
    const Particles::TCoreSystem* cps = Resources.get(name)->as<Particles::TCoreSystem>();
    return launchSystem(cps, entity);
}

//Particles::TParticleHandle CModuleParticles::launchSystem(const std::string& name, CHandle entity, const VEC3 offset)
//{
//    const Particles::TCoreSystem* cps = Resources.get(name)->as<Particles::TCoreSystem>();
//    cps->n_system.offset = offset;
//
//    return launchSystem(cps, entity);
//}


Particles::TParticleHandle CModuleParticles::launchSystem(const Particles::TCoreSystem* cps, CHandle entity)
{
    assert(cps);
    Particles::CSystem* ps = new Particles::CSystem(cps, entity);
    ps->launch();
    _activeSystems.push_back(ps);
    //dbg("total size %d\n", _activeSystems.size());

    return ps->getHandle();
}

Particles::TParticleHandle CModuleParticles::launchDynamicSystem(const std::string& name, VEC3 pos)
{
    const Particles::TCoreSystem* cps = Resources.get(name)->as<Particles::TCoreSystem>();
    assert(cps);

    CHandle h_e;
    h_e.create< CEntity >();
    CEntity* e = h_e;

    CHandle h_comp;
    h_comp = getObjectManager<TCompTransform>()->createHandle();
    e->set(h_comp.getType(), h_comp);

    Particles::CSystem* ps = new Particles::CSystem(cps, h_e);
    ps->_destroy_entity = true;
    ps->launch();

    // Finally we set the desired position and rotation
    TCompTransform * c_transform = e->get<TCompTransform>();
    c_transform->setPosition(pos);

    _activeSystems.push_back(ps);

    return ps->getHandle();
}

void CModuleParticles::kill(Particles::TParticleHandle ph, float fadeOutTime) {

    auto it = std::find_if(_activeSystems.begin(), _activeSystems.end(), [&ph](const Particles::CSystem* ps)
    {
        return ps->getHandle() == ph;
    });

    if (it != _activeSystems.end())
    {
        if (fadeOutTime > 0.f)
        {
            (*it)->fadeOut(fadeOutTime);
        }
        else
        {
            delete *it;
            _activeSystems.erase(it);
        }
    }
}

void CModuleParticles::killAll()
{
    for (auto it = _activeSystems.begin(); it != _activeSystems.end();)
    {
        Particles::CSystem* ps = *it;

        if (ps)
        {
            delete ps;
            it = _activeSystems.erase(it);
        }
    }

    _activeSystems.clear();
}

Particles::CSystem* CModuleParticles::getSystem(Particles::TParticleHandle ph) {

    auto it = std::find_if(_activeSystems.begin(), _activeSystems.end(), [&ph](const Particles::CSystem* ps)
    {
        return ps->getHandle() == ph;
    });

    return *it;
}

const VEC3& CModuleParticles::getWindVelocity() const
{
    return _windVelocity;
}
