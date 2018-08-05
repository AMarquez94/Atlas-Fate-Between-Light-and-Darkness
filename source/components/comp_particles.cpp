#include "mcv_platform.h"
#include "comp_particles.h"
#include "resources/resources_manager.h"

DECL_OBJ_MANAGER("particles", TCompParticles);

void TCompParticles::registerMsgs() {

    DECL_MSG(TCompParticles, TMsgEntityCreated, onCreated);
    DECL_MSG(TCompParticles, TMsgEntitiesGroupCreated, onGroupCreated);
    DECL_MSG(TCompParticles, TMsgEntityDestroyed, onDestroyed);
}

void TCompParticles::debugInMenu() {

    ImGui::Text("Particles handle: %d", _particles);
    if (_core) ImGui::Text("Core: %s", _core->getName().c_str());   
    //Engine.getParticles().getSystem(_particles)->debugInMenu();
}

void TCompParticles::load(const json& j, TEntityParseContext& ctx) {

    _fadeout = j.value("fade_out", 0.f);
    _on_start = j.value("on_start", true);

    if(j.count("core"))
        _core = Resources.get(j.value("core", ""))->as<Particles::TCoreSystem>();

    if (j.count("cores")) {
        auto& j_cores = j["cores"];
        for (auto it = j_cores.begin(); it != j_cores.end(); ++it) {
            auto p = Resources.get(it.value())->as<Particles::TCoreSystem>();
            assert(p);
            _cores.insert(std::pair<const Particles::TCoreSystem*, Particles::TParticlesHandle>(p, 0));
        }
    }
}

void TCompParticles::onCreated(const TMsgEntityCreated&) {

    if (_core && !_particles) {
        _particles = Engine.getParticles().launchSystem(_core, CHandle(this).getOwner());
    }

    // Launch the set of cores
    for (auto p : _cores) {
        _cores[p.first] = Engine.getParticles().launchSystem(p.first, CHandle(this).getOwner());
    }
}

void TCompParticles::onGroupCreated(const TMsgEntitiesGroupCreated&) {
    if (_core && !_particles) {
        _particles = Engine.getParticles().launchSystem(_core, CHandle(this).getOwner());
    }
}


void TCompParticles::onDestroyed(const TMsgEntityDestroyed&) {

    if (_particles) {
        Engine.getParticles().kill(_particles, _fadeout);
    }

    for (auto p : _cores) {
        Engine.getParticles().kill(_particles, _fadeout);
    }
}

void TCompParticles::playSystem() {

    if (_core && !_particles) {
        _particles = Engine.getParticles().launchSystem(_core, CHandle(this).getOwner());
    }

    // Launch the set of cores
    for (auto p : _cores) {
        p.second = Engine.getParticles().launchSystem(p.first, CHandle(this).getOwner());
    }
}

void TCompParticles::setSystemState(bool state) {

    for (auto p : _cores) {
        Particles::CSystem * system = Engine.get().getParticles().getSystem(p.second);
        if (system)
            system->setActive(state);
    }
}