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

    //ImGui::Text("Particles handle: %d", _particles);
    //if (_core) ImGui::Text("Core: %s", _core->getName().c_str());   
    //Engine.getParticles().getSystem(_particles)->debugInMenu();
}

void TCompParticles::load(const json& j, TEntityParseContext& ctx) {

    _fadeOut = j.value("fade_out", 0.f);
    auto& particlesName = j.value("core", "");
    _core = Resources.get(particlesName)->as<Particles::TCoreSystem>();
    assert(_core);
}

void TCompParticles::onCreated(const TMsgEntityCreated&) {

    if (_core && !_particles) {
        _particles = Engine.getParticles().launchSystem(_core, CHandle(this).getOwner());
    }
}

void TCompParticles::onGroupCreated(const TMsgEntitiesGroupCreated&) {
    if (_core && !_particles) {
        _particles = Engine.getParticles().launchSystem(_core, CHandle(this).getOwner());
    }
}


void TCompParticles::onDestroyed(const TMsgEntityDestroyed&) {

    if (_particles) {
        Engine.getParticles().kill(_particles, _fadeOut);
    }
}
