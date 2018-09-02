#include "mcv_platform.h"
#include "comp_speaker.h"
#include "components/object_controller/comp_landing.h"
#include "components/comp_particles.h"

DECL_OBJ_MANAGER("landing", TCompLanding);

void TCompLanding::onMsgGroupCreated(const TMsgEntitiesGroupCreated & msg)
{
    h_parent = getEntityByName(parentName);
    TCompParticles * c_e_particle = get<TCompParticles>();
    c_e_particle->setSystemState(false);
}

void TCompLanding::onMsgEntityCanLandSM(const TMsgEntityCanLandSM & msg)
{
    is_active = msg.canSM;
    TCompParticles * c_e_particle = get<TCompParticles>();
    c_e_particle->setSystemState(is_active);
}

void TCompLanding::debugInMenu()
{

}

void TCompLanding::load(const json & j, TEntityParseContext & ctx)
{
    parentName = j.value("parent", "The Player");
    //soundEventName = j.value("soundEventName", "event:/Sounds/Debug/Helicopter");
}

void TCompLanding::update(float dt)
{
    if (h_parent.isValid()) {
        CEntity* e_parent = h_parent;
        TCompTransform* t_parent = e_parent->get<TCompTransform>();
        VEC3 p_pos = t_parent->getPosition();

        TCompTransform* my_trans = get<TCompTransform>();
        float ypos = p_pos.y - 1000.f;

        physx::PxRaycastHit hit;
        if (EnginePhysics.Raycast(p_pos, -t_parent->getUp(), 1000.f, hit)) {
            ypos = hit.position.y;
        }

        p_pos.y = ypos;
        my_trans->setPosition(p_pos);
    }
}

void TCompLanding::registerMsgs()
{
    DECL_MSG(TCompLanding, TMsgEntitiesGroupCreated, onMsgGroupCreated);
    DECL_MSG(TCompLanding, TMsgEntityCanLandSM, onMsgEntityCanLandSM);
}