#include "mcv_platform.h"
#include "module_particles.h"
#include "particles/particle_system.h"
#include "particles/particle_parser.h"
#include "particles/particle_editor.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

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

    // Set the particle billboard mesh to be globaly used.
    auto rmesh = Resources.get("data/meshes/quad_volume_particles.instanced_mesh")->as<CRenderMesh>();
    instanced_particle = (CRenderMeshInstanced*)rmesh;
    instanced_particle->vtx_decl = CVertexDeclManager::get().getByName("CpuParticleInstance");

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
            if (ps->_destroy_entity) {
                CHandle h_entity = ps->getHandleEntity();
                if (h_entity.isValid()) {
                    h_entity.destroy();
                }
                
            }

            delete ps;
            it = _activeSystems.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void CModuleParticles::renderAdditive()
{
    auto technique = Resources.get("particles_instanced_additive.tech")->as<CRenderTechnique>();
    technique->activate();

    for (auto& ps : _activeSystems)
    {
        if (ps->type == Particles::CSystem::ADD)
            ps->render();
    };
}

void CModuleParticles::renderCombinative()
{
    auto technique = Resources.get("particles_instanced_combinative.tech")->as<CRenderTechnique>();
    technique->activate();

    for (auto& ps : _activeSystems)
    {
        if (ps->type != Particles::CSystem::ADD)
            ps->render();
    };
}

void CModuleParticles::renderMain()
{
    if(particles_enabled)
        p_editor->debugMenu();
}

Particles::TParticleHandle CModuleParticles::launchSystem(const std::string& name, CHandle entity)
{
    const Particles::TCoreSystem* cps = Resources.get(name)->as<Particles::TCoreSystem>();
    return launchSystem(cps, entity);
}

Particles::TParticleHandle CModuleParticles::launchSystem(const Particles::TCoreSystem* cps, CHandle entity)
{
    assert(cps);
    Particles::CSystem* ps = new Particles::CSystem(cps, entity);
    ps->launch();
    _activeSystems.push_back(ps);

    return ps->getHandle();
}

Particles::TParticleHandle CModuleParticles::launchDynamicSystem(const std::string& name, VEC3 pos, bool persistent)
{
    const Particles::TCoreSystem* cps = Resources.get(name)->as<Particles::TCoreSystem>();
    assert(cps);
    
    CHandle h_e;
    h_e.create< CEntity >();
    CEntity* e = h_e;

    CHandle h_comp;
    h_comp = getObjectManager<TCompTransform>()->createHandle();
    e->set(h_comp.getType(), h_comp);

    h_comp = getObjectManager<TCompName>()->createHandle();
    e->set(h_comp.getType(), h_comp);
    TCompName* p_name = e->get<TCompName>();
    p_name->setName(std::string("Dynamic_Particle_" + h_e.asString()).c_str());
    
    if (persistent) {
        CHandle h_tag = getObjectManager<TCompTags>()->createHandle();
        h_tag.setOwner(h_e);
        e->set(h_tag.getType(), h_tag);
        TCompTags* c_tag = h_tag;
        CTagsManager::get().registerTagName(getID("persistent"), "persistent");
        c_tag->addTag(getID("persistent"));
    }
       

    Particles::CSystem* ps = new Particles::CSystem(cps, h_e);
    ps->_destroy_entity = true;
    ps->launch();

    // Finally we set the desired position and rotation
    TCompTransform * c_transform = e->get<TCompTransform>();
    c_transform->setPosition(pos);

    _activeSystems.push_back(ps);

    return ps->getHandle();
    //return Particles::TParticleHandle();
}

Particles::TParticleHandle CModuleParticles::launchDynamicSystemRot(const std::string& name, VEC3 pos, QUAT rot, bool persistent)
{
    const Particles::TCoreSystem* cps = Resources.get(name)->as<Particles::TCoreSystem>();
    assert(cps);

    CHandle h_e;
    h_e.create< CEntity >();
    CEntity* e = h_e;

    CHandle h_comp;
    h_comp = getObjectManager<TCompTransform>()->createHandle();
    e->set(h_comp.getType(), h_comp);

    h_comp = getObjectManager<TCompName>()->createHandle();
    e->set(h_comp.getType(), h_comp);
    TCompName* p_name = e->get<TCompName>();
    p_name->setName(std::string("Dynamic_Particle_" + h_e.asString()).c_str());

    if (persistent) {
        CHandle h_tag = getObjectManager<TCompTags>()->createHandle();
        h_tag.setOwner(h_e);
        e->set(h_tag.getType(), h_tag);
        TCompTags* c_tag = h_tag;
        CTagsManager::get().registerTagName(getID("persistent"), "persistent");
        c_tag->addTag(getID("persistent"));
    }


    // Finally we set the desired position and rotation
    TCompTransform * c_transform = e->get<TCompTransform>();
    c_transform->setPosition(pos);
    c_transform->setRotation(rot);

    Particles::CSystem* ps = new Particles::CSystem(cps, h_e);
    ps->_destroy_entity = true;
    ps->launch();

    _activeSystems.push_back(ps);

    return ps->getHandle();
    //return Particles::TParticleHandle();
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
            if ((*it)->getHandle()) {
                delete (*it);
                it = _activeSystems.erase(it);
            }
        }
    }
}

void CModuleParticles::killAll()
{
    for (auto it = _activeSystems.begin(); it != _activeSystems.end();)
    {
        Particles::CSystem* ps = *it;

        ps->getHandle();

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
