#include "mcv_platform.h"
#include "comp_player_attack_cast.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/ia/comp_bt_mimetic.h"
#include "components/player_controller/comp_shadow_controller.h"
#include "components/object_controller/comp_button.h"
#include "components/player_controller/comp_player_tempcontroller.h"

DECL_OBJ_MANAGER("player_attack_cast", TCompPlayerAttackCast);

void TCompPlayerAttackCast::debugInMenu() {
}

void TCompPlayerAttackCast::load(const json& j, TEntityParseContext& ctx) {
    geometryAttack.radius = j.value("radiusAttack", 2.f);
    geometryMoveObjects.radius = j.value("radiusMoveObject", 1.f);
    geometryButtons.radius = j.value("radiusPressButton", 1.0f);
    attack_fov = deg2rad(j.value("attack_fov", 120.f));
    moveObjects_fov = deg2rad(j.value("moveObjects_fov", 20.f));
    button_fov = deg2rad(j.value("button_fov", 60.f));

    physx::PxFilterData pxFilterData;

    pxFilterData.word0 = FilterGroup::Enemy;
    PxPlayerAttackQueryFilterData.data = pxFilterData;
    PxPlayerAttackQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;

    pxFilterData.word0 = FilterGroup::MovableObject;
    PxPlayerMoveObjectsQueryFilterData.data = pxFilterData;
    PxPlayerMoveObjectsQueryFilterData.flags = physx::PxQueryFlag::eDYNAMIC;

    pxFilterData.word0 = FilterGroup::Button;
    PxPlayerButtonInteractQueryFilterData.data = pxFilterData;
    PxPlayerButtonInteractQueryFilterData.flags = physx::PxQueryFlag::eSTATIC;

    EngineGUI.enableWidget("press_button_e", false);
    EngineGUI.enableWidget("press_button_a", false);
    EngineGUI.enableWidget("grab_enemy_e", false);
    EngineGUI.enableWidget("grab_enemy_a", false);
    EngineGUI.enableWidget("attack_enemy_mouse", false);
    EngineGUI.enableWidget("attack_enemy_x", false);
}

void TCompPlayerAttackCast::registerMsgs()
{
    DECL_MSG(TCompPlayerAttackCast, TMsgScenePaused, onMsgScenePaused);
}

void TCompPlayerAttackCast::onMsgScenePaused(const TMsgScenePaused & msg)
{
    paused = msg.isPaused;
}

const std::vector<CHandle> TCompPlayerAttackCast::getEnemiesInRange()
{
    std::vector<CHandle> enemies_in_range;

    TCompTransform* tPos = get<TCompTransform>();

    if (geometryAttack.isValid()) {

        std::vector<physx::PxOverlapHit> hits;
        if (EnginePhysics.Overlap(geometryAttack, tPos->getPosition(), hits, PxPlayerAttackQueryFilterData)) {
            for (int i = 0; i < hits.size(); i++) {
                CHandle hitCollider;
                hitCollider.fromVoidPtr(hits[i].actor->userData);
                if (hitCollider.isValid()) {
                    CHandle enemy = hitCollider.getOwner();
                    if (enemy.isValid()) {
                        enemies_in_range.push_back(enemy);
                    }
                }
            }
        }
    }

    return enemies_in_range;
}

const bool TCompPlayerAttackCast::canAttackEnemiesInRange(CHandle& closestEnemyToAttack)
{
    bool canAttackNow = false;
    const std::vector<CHandle> enemies = getEnemiesInRange();
    closestEnemyToAttack = CHandle();

    TCompTransform * mypos = get<TCompTransform>();

    for (int i = 0; i < enemies.size() && !canAttackNow; i++) {

        CEntity * enemy = enemies[i];
        TCompTransform *ePos = enemy->get<TCompTransform>();
        TCompTags * eTag = enemy->get<TCompTags>();

        if (eTag->hasTag(getID("patrol"))) {
            TCompAIPatrol * cPatrol = enemy->get<TCompAIPatrol>();
            if (mypos->isInHorizontalFov(ePos->getPosition(), attack_fov) &&
                !ePos->isInFront(mypos->getPosition()) &&
                !cPatrol->isStunned()) {
                canAttackNow = true;
                closestEnemyToAttack = enemies[i];
            }
        }
        else if (eTag->hasTag(getID("mimetic"))) {
            TCompAIMimetic * cMimetic = enemy->get<TCompAIMimetic>();
            if (mypos->isInHorizontalFov(ePos->getPosition(), attack_fov) && !cMimetic->isStunned()) {
                canAttackNow = true;
                closestEnemyToAttack = enemies[i];
            }
        }
    }
    return canAttackNow;
}

CHandle TCompPlayerAttackCast::closestEnemyToMerge(bool goingToMerge)
{
    CHandle closestEnemy = CHandle();
    const std::vector<CHandle> enemies = getEnemiesInRange();

    TCompTransform * mypos = get<TCompTransform>();

    for (int i = 0; i < enemies.size() && !closestEnemy.isValid(); i++) {

        CEntity * enemy = enemies[i];
        TCompTransform *ePos = enemy->get<TCompTransform>();
        TCompTags * eTag = enemy->get<TCompTags>();

        if (eTag->hasTag(getID("patrol"))) {
            TCompAIPatrol * cPatrol = enemy->get<TCompAIPatrol>();
            TCompShadowController* shadow_controller = get<TCompShadowController>();
            if (mypos->isInHorizontalFov(ePos->getPosition(), attack_fov) && cPatrol->isStunned() &&
                (!goingToMerge || (goingToMerge && shadow_controller->IsPointInShadows(ePos->getPosition() + VEC3(0, 0.1f, 0), false)))) {
                closestEnemy = enemies[i];
            }
        }
    }

    return closestEnemy;
}

const std::vector<CHandle> TCompPlayerAttackCast::getMovableObjectsInRange()
{
    std::vector<CHandle> movable_objects_in_range;

    TCompTransform* tPos = get<TCompTransform>();

    if (geometryMoveObjects.isValid()) {

        std::vector<physx::PxOverlapHit> hits;
        if (EnginePhysics.Overlap(geometryMoveObjects, tPos->getPosition(), hits, PxPlayerMoveObjectsQueryFilterData)) {
            for (int i = 0; i < hits.size(); i++) {
                CHandle hitCollider;
                hitCollider.fromVoidPtr(hits[i].actor->userData);
                if (hitCollider.isValid()) {
                    CHandle object = hitCollider.getOwner();
                    if (object.isValid()) {
                        movable_objects_in_range.push_back(object);
                    }
                }
            }
        }
    }

    return movable_objects_in_range;
}

CHandle TCompPlayerAttackCast::getClosestMovableObjectInRange()
{
    bool found = false;
    CHandle closestMovableObject = CHandle();
    const std::vector<CHandle> movable_objects = getMovableObjectsInRange();

    TCompTransform * mypos = get<TCompTransform>();

    for (int i = 0; i < movable_objects.size() && !found; i++) {

        CEntity * movable_object = movable_objects[i];
        TCompTransform *ePos = movable_object->get<TCompTransform>();
        TCompRigidbody *eRigidbody = movable_object->get<TCompRigidbody>();

        if (mypos->isInHorizontalFov(ePos->getPosition(), moveObjects_fov) && eRigidbody->is_grounded) {
            found = true;
            closestMovableObject = movable_objects[i];
        }
    }
    return closestMovableObject;
}

const std::vector<CHandle> TCompPlayerAttackCast::getButtonsInRange() {
    std::vector<CHandle> buttons_in_range;

    TCompTransform* tPos = get<TCompTransform>();

    if (geometryButtons.isValid()) {

        std::vector<physx::PxOverlapHit> hits;
        if (EnginePhysics.Overlap(geometryButtons, tPos->getPosition(), hits, PxPlayerButtonInteractQueryFilterData)) {
            for (int i = 0; i < hits.size(); i++) {
                CHandle hitCollider;
                hitCollider.fromVoidPtr(hits[i].actor->userData);
                if (hitCollider.isValid()) {
                    CHandle button = hitCollider.getOwner();
                    CEntity* e = button;
                    //std::string name = e->getName();
                    TCompCollider* collider_button = e->get<TCompCollider>();

                    if (button.isValid() && collider_button->config->group == FilterGroup::Button) {
                        //dbg("Name of the hit nº %d : %s \n", i, name.c_str());
                        buttons_in_range.push_back(button);
                    }
                }
            }
        }
    }

    return buttons_in_range;
}

CHandle TCompPlayerAttackCast::getClosestButtonInRange() {

    bool found = false;
    CHandle closestButton = CHandle();
    const std::vector<CHandle> buttons = getButtonsInRange();

    TCompTransform * mypos = get<TCompTransform>();

    for (int i = 0; i < buttons.size() && !found; i++) {

        CEntity * button = buttons[i];
        TCompButton* tButton = button->get <TCompButton>();
        if (tButton && tButton->canBePressed) {
            TCompTransform *ePos = button->get<TCompTransform>();
            if (mypos->isInHorizontalFov(ePos->getPosition(), button_fov)) {
                found = true;
                closestButton = buttons[i];
            }
        }
    }
    return closestButton;
}

void TCompPlayerAttackCast::update(float dt)
{
    TCompTempPlayerController* player_controller = get<TCompTempPlayerController>();
    bool isMerged = player_controller->isMerged;
    bool isDead = player_controller->isDead();
    bool isGrounded = player_controller->isGrounded;
    bool isInhibited = player_controller->isInhibited;

    /* Button gui */
    CHandle newClosestButton;
    if (!isMerged && !isDead && isGrounded) {
        newClosestButton = getClosestButtonInRange();
    }
    if (!isInhibited && newClosestButton.isValid()) {
        /* Activate gui button */
        if (!EngineInput.pad().connected) {
            EngineGUI.enableWidget("press_button_e", true);
        }
        else {
            EngineGUI.enableWidget("press_button_a", true);
        }
    }
    else {
        EngineGUI.enableWidget("press_button_e", false);
        EngineGUI.enableWidget("press_button_a", false);
    }
    //if (newClosestButton.isValid() && !closestButton.isValid()) {
    //    /* Activate gui button */
    //    if (!EngineInput.pad().connected) {
    //        EngineGUI.enableWidget("press_button_e", true);
    //    }
    //    else {
    //        EngineGUI.enableWidget("press_button_a", true);
    //    }
    //}
    //else if (closestButton.isValid() && !newClosestButton.isValid()) {
    //    /* Deactivate gui button */
    //    EngineGUI.enableWidget("press_button_e", false);
    //    EngineGUI.enableWidget("press_button_a", false);
    //}
    closestButton = newClosestButton;

    /* Attack gui */
    CHandle newEnemyToAttack;
    if (!isMerged && !isDead && isGrounded) {
        canAttackEnemiesInRange(newEnemyToAttack);
    }
    if(!isInhibited && newEnemyToAttack.isValid()){
    //if (newEnemyToAttack.isValid() && !closestEnemyToAttack.isValid()) {
        /* Activate gui button */
        if (!EngineInput.pad().connected) {
            EngineGUI.enableWidget("attack_enemy_mouse", true);
        }
        else {
            EngineGUI.enableWidget("attack_enemy_x", true);
        }
    }
    else{ /*if (closestEnemyToAttack.isValid() && !newEnemyToAttack.isValid()) {*/
        /* Deactivate gui button */
        EngineGUI.enableWidget("attack_enemy_mouse", false);
        EngineGUI.enableWidget("attack_enemy_x", false);
    }
    closestEnemyToAttack = newEnemyToAttack;


    /* Grab gui */
    CHandle newClosestEnemy;
    if (!isDead) {
        newClosestEnemy = closestEnemyToMerge(false);
    }
    CHandle newClosestEnemyToMerge = CHandle();
    CHandle newClosestMergingEnemy = CHandle();
    if (newClosestEnemy.isValid()) {
        newClosestMergingEnemy = closestEnemyToMerge(true);
        if (!isMerged) {
            newClosestEnemyToMerge = newClosestMergingEnemy;
        }
        if(!isInhibited && newClosestEnemyToMerge.isValid()){
        //if (newClosestEnemyToMerge.isValid() && !closestEnemyMergeable.isValid()) {
            /* Activate gui button */
            if (!EngineInput.pad().connected) {
                EngineGUI.enableWidget("grab_enemy_e", true);
            }
            else {
                EngineGUI.enableWidget("grab_enemy_a", true);
            }
        }
    }
    if(isInhibited || !newClosestEnemyToMerge.isValid()){
    //if (closestEnemyMergeable.isValid() && !newClosestEnemyToMerge.isValid()) {
        /* Deactivate gui button */
        EngineGUI.enableWidget("grab_enemy_e", false);
        EngineGUI.enableWidget("grab_enemy_a", false);
    }
    closestEnemyToGrab = newClosestEnemy;
    closestEnemyMergeable = newClosestEnemyToMerge;
    closestMergingEnemy = newClosestMergingEnemy;
}
