#include "mcv_platform.h"
#include "comp_inhibitor.h"
#include "components/comp_transform.h"
#include "render/render_objects.h"
#include "entity/entity_parser.h"
#include "components/comp_render.h"

DECL_OBJ_MANAGER("inhibitor", TCompInhibitor);

void TCompInhibitor::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    CEntity* parent = h_parent;
    TCompTransform* c_parent_trans = parent->get<TCompTransform>();

    TCompTransform *c_my_trans = get<TCompTransform>();
    c_my_trans->setPosition(c_parent_trans->getPosition() + VEC3(0, 2, 0));

    CEntity* player = getEntityByName("The Player");
    TCompTransform* playerPos = player->get<TCompTransform>();
    dest = playerPos->getPosition() + VEC3(0,1.5f,0);

    c_my_trans->lookAt(c_my_trans->getPosition(), dest);
}

void TCompInhibitor::debugInMenu() {

}

void TCompInhibitor::load(const json& j, TEntityParseContext& ctx) {
    h_parent = ctx.entity_starting_the_parse;
    speed = j.value("speed", 15.0f);
}

void TCompInhibitor::registerMsgs() {
    DECL_MSG(TCompInhibitor, TMsgEntityCreated, onMsgEntityCreated);
}

void TCompInhibitor::update(float dt) {
    if (!exploding) {
        TCompTransform* mypos = get<TCompTransform>();
        mypos->setPosition(mypos->getPosition() + mypos->getFront() * speed * dt);
        if (VEC3::Distance(mypos->getPosition(), dest) < speed * dt) {
            exploding = true;
        }
    }
    else {
        /* manageExplosion */
        /* TODO: in each update, test if the player is inside the explosion range or the explosion has reached it's maximum. 
        Destroy the entity after this. */

        /* if(!playerInhibited && distance(playerpos, mypos) < actualRange) => setPlayerInhibited 
           if(actualRange >= maxRange) => destroy() */


        if (!playerWasInhibited) {
            CHandle player = getEntityByName("The Player");
            if (player.isValid()) {
                TMsgInhibitorShot msg;
                msg.h_sender = CHandle(this).getOwner();
                player.sendMsg(msg);
            }
            playerWasInhibited = true;
        }
        else {
            //execDelayedScript("");
            TCompRender * render = get<TCompRender>();
            render->visible = false;

            Engine.get().getParticles().launchSystem("data/particles/def_projectile_explosion.particles", CHandle(this).getOwner());
            Engine.get().getParticles().launchSystem("data/particles/def_projectile_explosion_trails.particles", CHandle(this).getOwner());
            Engine.get().getParticles().launchSystem("data/particles/def_projectile_explosion_trails_large.particles", CHandle(this).getOwner());
            //execDelayedScript("");
            //CHandle(this).getOwner().destroy();
        }
    }
}