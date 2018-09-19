#include "mcv_platform.h"
#include "comp_inhibitor.h"
#include "components/comp_transform.h"
#include "render/render_objects.h"
#include "entity/entity_parser.h"
#include "components/comp_render.h"
#include "components/lighting/comp_light_point.h"
#include "components/comp_audio.h"

DECL_OBJ_MANAGER("inhibitor", TCompInhibitor);

void TCompInhibitor::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
    CEntity* parent = h_parent;
    TCompTransform* c_parent_trans = parent->get<TCompTransform>();

    TCompTransform *c_my_trans = get<TCompTransform>();
    c_my_trans->setPosition(c_parent_trans->getPosition() + VEC3(0, 2, 0));

    CEntity* player = EngineEntities.getPlayerHandle();
    TCompTransform* playerPos = player->get<TCompTransform>();
    dest = playerPos->getPosition() + VEC3(0,1.5f,0);

    parent = CHandle(this).getOwner();
    TCompLightPoint * point = parent->get<TCompLightPoint>();
    assert(point);
    initial_intensity = point->getIntensity();

    c_my_trans->lookAt(c_my_trans->getPosition(), dest);
}

void TCompInhibitor::debugInMenu() {

}

void TCompInhibitor::load(const json& j, TEntityParseContext& ctx) {
    h_parent = ctx.entity_starting_the_parse;
    speed = j.value("speed", 15.0f);
    fading_speed = j.value("fade_speed", 1.5f);
}

void TCompInhibitor::registerMsgs() {

    DECL_MSG(TCompInhibitor, TMsgEntityCreated, onMsgEntityCreated);
}

void TCompInhibitor::update(float dt) {

    CHandle parent = CHandle(this).getOwner();
    if (parent.isValid()) {
        if (fading) {
            CEntity * t_parent = parent;
            TCompLightPoint * point = t_parent->get<TCompLightPoint>();
            float new_intensity = Clamp(point->getIntensity() - ((dt / fading_speed) * initial_intensity), 0.f, initial_intensity);
            point->setIntensity(new_intensity);
        }
        else if (!exploding) {
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
                CHandle player = EngineEntities.getPlayerHandle();
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
                this->fading = true;

                TCompAudio* my_audio = get<TCompAudio>();
                my_audio->playEvent("event:/Sounds/Enemies/Patrol/InhibitorExplosion");

                EngineParticles.launchSystem("data/particles/def_projectile_explosion.particles", CHandle(this).getOwner());
                EngineParticles.launchSystem("data/particles/def_projectile_explosion_trails.particles", CHandle(this).getOwner());
                EngineParticles.launchSystem("data/particles/def_projectile_explosion_trails_large.particles", CHandle(this).getOwner());
                execDelayedScript("destroyHandle(" + CHandle(this).getOwner().asString() + ")", 6);
                
                //CEntity * player = EngineEntities.getPlayerHandle();
                //TCompTransform* mypos = player->get<TCompTransform>();
                //EngineParticles.launchDynamicSystem("data/particles/def_amb_ground_slam.particles", mypos->getPosition());
                //EngineInstancing.addDynamicInstance("data/meshes/decal_damage.instanced_mesh", "data/materials/mtl_decal_damage.material", mypos->asMatrix(), 4);
            }
        }
    }
}