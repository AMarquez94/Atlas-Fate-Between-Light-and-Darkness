#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_shooter.h"
#include "components/comp_transform.h"
#include "components/physics/comp_collider.h"
#include "render/render_objects.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"

DECL_OBJ_MANAGER("shooter", TCompShooter);

void TCompShooter::shoot()
{
    //Think if the bullet is going to be a raycast (inf. speed) or entity

    //TEntityParseContext ctxBullet;
    //parseScene("data/prefabs/bullet.prefab", ctxBullet);
    //CEntity *eCapsule = ctxBullet.entities_loaded[0];
}

void TCompShooter::onScenePaused(const TMsgScenePaused & msg)
{
    paused = msg.isPaused;
}

void TCompShooter::debugInMenu() {
}

void TCompShooter::load(const json& j, TEntityParseContext& ctx) {
    precission = j.value("precission", 0.3f);
    firing_rate_cooldown = j.value("firing_rate_cooldown", 1.f);
    bullet_damage = j.value("bullet_damage", 10.f);
    //bullet_speed = j.value("bullet_speed", 10.f);
    bullet_range = j.value("bullet_range", 30.f);
    state = EShooterState::STOPPED;

    physx::PxFilterData filterdata;
    filterdata.word0 = FilterGroup::All;
    pxQueryFilterData.data = filterdata;
}

void TCompShooter::registerMsgs() {
    DECL_MSG(TCompShooter, TMsgScenePaused, onScenePaused);
}

void TCompShooter::setIsFiring(bool isFiring, CHandle h_entityToFire)
{
    state = isFiring ? EShooterState::FIRING : EShooterState::STOPPED;
    h_entity_to_shoot = h_entityToFire;
    if (isFiring) {
        firing_timer = 0.f;
    }
}

void TCompShooter::update(float dt) {
    if (!paused) {
        if (state == EShooterState::FIRING) {
            firing_timer += dt;
            if (firing_timer > firing_rate_cooldown) {
                CEntity* entityToShoot = h_entity_to_shoot;
                TCompTransform* epos = entityToShoot->get<TCompTransform>();
                TCompCollider* eCollider = entityToShoot->get<TCompCollider>();
                CPhysicsCapsule * capsuleCollider = (CPhysicsCapsule *)eCollider->config;
                capsuleCollider->height;


                TCompTransform* mypos = get<TCompTransform>();
                VEC3 shootingDir = epos->getPosition() + VEC3(0, capsuleCollider->height * 2 / 3, 0) - mypos->getPosition();
                shootingDir.x += urand(-precission, precission);
                shootingDir.y += urand(-precission, precission);
                shootingDir.z += urand(-precission, precission);
                shootingDir.Normalize();
                //EngineSound.playSound2D("drone_shot");    //TODO: shoot
                Engine.get().getParticles().launchSystem("data/particles/muzzleflash.particles", CHandle(this).getOwner());
                Engine.get().getParticles().launchSystem("data/particles/muzzleflash_glow.particles", CHandle(this).getOwner());
                physx::PxRaycastHit hit;
                //dbg("SHOOT\n");
                if (EnginePhysics.Raycast(mypos->getPosition(), shootingDir, bullet_range, hit, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), pxQueryFilterData)) {
                    CHandle hitCollider;
                    hitCollider.fromVoidPtr(hit.actor->userData);
                    if (hitCollider.isValid()) {
                        CEntity* entityShooted = hitCollider.getOwner();
                        TCompTags* tagShooted = entityShooted->get<TCompTags>();
                        if (tagShooted) {
                            TMsgBulletHit msg;
                            msg.h_sender = CHandle(this).getOwner();
                            msg.damage = bullet_damage;
                            if (tagShooted->hasTag(getID("player"))) {
                                /* Player has been shooted */
                                entityShooted->sendMsg(msg);
                                EngineLogic.execScriptDelayed("playSound2D(\"hitmarker\")", 0.4f);
                                //dbg("Auch\n");
                            }
                            else {
                                /* TODO: to complete */
                                EngineLogic.execScriptDelayed("exeShootImpactSound()", 0.5f);
                            }
                        }
                        else {
                            /* TODO: to complete */
                            EngineLogic.execScriptDelayed("exeShootImpactSound()", 0.1f);
                        }
                        TCompName* myName = entityShooted->get<TCompName>();
                        //dbg("HIT entity %s\n", myName->getName());
                    }
                }
                else {
                    dbg("MISSED\n");
                }
                //TODO: Shoot
                firing_timer = 0;
            }
        }
    }
}