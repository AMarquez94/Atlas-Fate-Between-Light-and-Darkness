#include "mcv_platform.h"
#include "comp_suspect_element.h"
#include "entity/entity_parser.h"
#include "components/comp_render.h"

DECL_OBJ_MANAGER("suspect_ui_element", TCompSuspectElement);

void TCompSuspectElement::onEnemySuspecting(const TMsgEnemySuspecting & msg)
{
    parent = EngineEntities.getPlayerHandle();
    h_suspecting = msg.enemy_suspecting;
    active = true;
}

void TCompSuspectElement::debugInMenu()
{

}

void TCompSuspectElement::load(const json & j, TEntityParseContext & ctx)
{
    offset_dist = j.value("offset_dist", 1.f);
    active = false;
}

void TCompSuspectElement::update(float dt)
{
    if (!CHandle(this).getOwner().isValid())
        return;

    if (active && CHandle(this).isValid()) {
        TCompTransform* my_pos = get<TCompTransform>();
        if (!my_pos) {
            return;
        }

        CEntity* e_parent = parent;
        TCompTransform* p_pos = e_parent->get<TCompTransform>();
        CEntity* e_suspecting = h_suspecting;
        TCompTransform* s_pos = e_suspecting->get<TCompTransform>();

        my_pos->setPosition(p_pos->getPosition());
        float delta_yaw = my_pos->getDeltaYawToAimTo(s_pos->getPosition());
        float yaw, pitch;
        my_pos->getYawPitchRoll(&yaw, &pitch);
        my_pos->setYawPitchRoll(yaw + delta_yaw, pitch);
        my_pos->setPosition(p_pos->getPosition() + my_pos->getFront() * offset_dist + VEC3(0.f,0.75f,0.f));
        my_pos->setYawPitchRoll(yaw + delta_yaw + deg2rad(180.f), pitch);

        TCompAIPatrol* ai_patrol = e_suspecting->get<TCompAIPatrol>();
        if (ai_patrol) {
            TCompRender* my_render = get<TCompRender>();
            my_render->self_color = VEC4::Lerp(VEC4(1.f, 1.f, 0.f, 0.5f), VEC4(1.f, 0.f, 0.f, 1.f), ai_patrol->getSuspectLevel());
        }
    }
}

void TCompSuspectElement::registerMsgs()
{
    DECL_MSG(TCompSuspectElement, TMsgEnemySuspecting, onEnemySuspecting);
}