#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_shooter.h"
#include "components/comp_transform.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("shooter", TCompShooter);

void TCompShooter::shoot()
{
    //Think if the bullet is going to be a raycast (inf. speed) or entity

    //TEntityParseContext ctxBullet;
    //parseScene("data/prefabs/bullet.prefab", ctxBullet);
    //CEntity *eCapsule = ctxBullet.entities_loaded[0];
}

void TCompShooter::debugInMenu() {
}

void TCompShooter::load(const json& j, TEntityParseContext& ctx) {
    precission = j.value("precission", 0.8f);
    firing_rate_cooldown = j.value("firing_rate_cooldown", 1.f);
    bullet_damage = j.value("bullet_damage", 10.f);
    bullet_speed = j.value("bullet_speed", 10.f);
    state = EShooterState::STOPPED;
}

void TCompShooter::registerMsgs() {
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
    if (state == EShooterState::FIRING) {
        firing_timer += dt;
        if (firing_timer > firing_rate_cooldown) {
            CEntity* entityToShoot = h_entity_to_shoot;
            TCompTransform* epos = entityToShoot->get<TCompTransform>();
            dbg("SHOOT\n");
            //TODO: Shoot
            firing_timer = 0;
        }
    }
}