#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_shadow_controller.h"
#include "components/comp_transform.h"
#include "components/comp_render.h"
#include "entity/common_msgs.h"
#include "utils/utils.h"
#include "render/mesh/mesh_loader.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/physics/comp_collider.h"
#include "components/comp_tags.h"
#include "components/lighting/comp_light_dir.h"
#include "components/lighting/comp_light_spot.h"
#include "components/lighting/comp_light_point.h"
#include "components/comp_name.h"
#include "physics/physics_collider.h"

DECL_OBJ_MANAGER("shadow_controller", TCompShadowController);

void TCompShadowController::debugInMenu() {
}

void TCompShadowController::load(const json& j, TEntityParseContext& ctx) {

    Init();
}

void TCompShadowController::update(float dt) {

    TCompTransform * c_my_transform = get<TCompTransform>();
    VEC3 new_pos = c_my_transform->getPosition() + 0.01f * c_my_transform->getUp();
    bool shadow_test = IsPointInShadows(new_pos) && enemies_illuminating_me.size() == 0;
    //dbg("bool test %d\n", shadow_test);

    // We have entered or left a shadow, notify this.
    if (shadow_test != is_shadow) {
        is_shadow = shadow_test;

        TMsgShadowChange msgToSend;
        msgToSend.is_shadowed = is_shadow;
        CEntity* e = CHandle(this).getOwner();
        e->sendMsg(msgToSend);
    }

    // Update shader constants
    //cb_player.player_pos = c_my_transform->getPosition();
    //cb_player.player_next_pos = c_my_transform->getPosition() + 0.35f * c_my_transform->getFront();
    //cb_player.updateGPU();
}

void TCompShadowController::Init() {

    is_shadow = false;
    shadow_sphere.radius = 0.05;
}

void TCompShadowController::onSceneCreated(const TMsgSceneCreated& msg) {

    // Retrieve all scene lights
    auto& light_handles = CTagsManager::get().getAllEntitiesByTag(getID("light"));

    for (auto h : light_handles) {
        CEntity* current_light = h;
        TCompLightDir * c_light_dir = current_light->get<TCompLightDir>();
        TCompLightSpot * c_light_spot = current_light->get<TCompLightSpot>();
        TCompLightPoint* c_light_point = current_light->get<TCompLightPoint>();

        if (c_light_dir) // by now we will only retrieve directional lights
        {
            static_lights.push_back(h);
        }
        else if (c_light_spot || c_light_point) // by now we will only retrieve directional lights
        {
            TCompCollider * c_collider = current_light->get<TCompCollider>();
            //c_collider->config->shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            //c_collider->config->shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            if (c_collider != NULL)
                dynamic_lights.push_back(h);
        }
    }

    physx::PxFilterData pxFilterData;
    pxFilterData.word0 = FilterGroup::Scenario | FilterGroup::DItem | FilterGroup::Enemy | FilterGroup::MovableObject;
    shadowDetectionFilter.data = pxFilterData;

    pxFilterData.word0 = FilterGroup::Scenario | FilterGroup::DItem | FilterGroup::MovableObject;
    shadowDetectionFilterEnemy.data = pxFilterData;

    pxFilterData.word0 = FilterGroup::Light;
    lightDetectionFilter.data = pxFilterData;
}

void TCompShadowController::onPlayerExposed(const TMsgPlayerIlluminated& msg) {

    if (msg.h_sender.isValid()) {
        bool found = false;
        for (int i = 0; i < enemies_illuminating_me.size() && !found; i++) {
            if (enemies_illuminating_me[i] == msg.h_sender) {
                found = true;
            }
        }
        if (!found && msg.isIlluminated) {
            enemies_illuminating_me.push_back(msg.h_sender);
        }
        else if (found && !msg.isIlluminated) {
            enemies_illuminating_me.erase(std::remove(enemies_illuminating_me.begin(), enemies_illuminating_me.end(), msg.h_sender));
        }
    }
}

void TCompShadowController::onPlayerInShadows(const TMsgPlayerInShadows & msg)
{
    hackShadows = !hackShadows;
}

void TCompShadowController::registerMsgs() {

    DECL_MSG(TCompShadowController, TMsgSceneCreated, onSceneCreated);
    DECL_MSG(TCompShadowController, TMsgPlayerIlluminated, onPlayerExposed);
    DECL_MSG(TCompShadowController, TMsgPlayerInShadows, onPlayerInShadows);
}

// We can also use this public method from outside this class.
bool TCompShadowController::IsPointInShadows(const VEC3 & point, bool player)
{
    if (hackShadows) {
        return true;
    }

    physx::PxRaycastHit hit;
    for (unsigned int i = 0; i < static_lights.size(); i++) {
        CEntity * c_entity = static_lights[i];
        TCompLightDir* c_light_dir = c_entity->get<TCompLightDir>();

        if (!c_light_dir || (c_light_dir && !c_light_dir->isEnabled)) {
            continue;
        }

        TCompTransform * c_trans = c_entity->get<TCompTransform>();

        float distance = VEC3::Distance(c_trans->getPosition(), point);

        if (player) {
            if (!EnginePhysics.Raycast(point, -c_trans->getFront(), distance, hit, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), shadowDetectionFilter))
                return false;
        }
        else {
            if (EnginePhysics.Raycast(point, -c_trans->getFront(), distance, hit, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), shadowDetectionFilterEnemy)) {
                CHandle h;
                h.fromVoidPtr(hit.actor->userData);
                CEntity* e = h.getOwner();
                dbg("Collided with %s\n", e->getName());
            }

            if (!EnginePhysics.Raycast(point, -c_trans->getFront(), distance, hit, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), shadowDetectionFilterEnemy))
                return false;
        }
    }

    std::vector<physx::PxOverlapHit> hits;
    if (EnginePhysics.Overlap(shadow_sphere, point, hits, lightDetectionFilter)) {
        for (int i = 0; i < hits.size(); i++) {
            CHandle hitCollider;
            hitCollider.fromVoidPtr(hits[i].actor->userData);
            if (hitCollider.isValid()) {

                CEntity * collider = hitCollider.getOwner();
                TCompLightSpot* c_light_spot = collider->get<TCompLightSpot>();
                TCompLightPoint* c_light_point = collider->get<TCompLightPoint>();

                if ((!c_light_spot && !c_light_point) ||
                    (c_light_spot && !c_light_spot->isEnabled) ||
                    (c_light_point && !c_light_point->isEnabled)) {
                    continue;
                }

                TCompCollider * c_collider = collider->get<TCompCollider>();
                TCompTransform * c_transform = collider->get<TCompTransform>();
                VEC3 dir = point - c_transform->getPosition();
                dir.Normalize();
                float distance = Clamp(VEC3::Distance(c_transform->getPosition(), point), 0.21f, 10000.f);
                physx::PxRaycastHit hit2;
                if (!EnginePhysics.Raycast(c_transform->getPosition(), dir, distance - 0.2f, hit2, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), shadowDetectionFilter)) {
                    //dbg("no collision, we are in light\n");
                    return false;
                }
                //dbg("max distance %f\n", distance);
                //CHandle hitCol;
                //hitCol.fromVoidPtr(hit2.actor->userData);
                //if (hitCol.isValid()) {
                //    CEntity* entityShooted = hitCol.getOwner();
                //    TCompName * name = collider->get<TCompName>();
                //    dbg("hitted at %s\n", name->getName());
                //}
            }
        }
    }

    /*for (unsigned int i = 0; i < dynamic_lights.size(); i++)
    {
        CEntity * c_entity = dynamic_lights[i];

        //Checking for hacks regarding spotlights and pointlights activation
         TCompLightSpot* c_light_spot = c_entity->get<TCompLightSpot>();
        TCompLightPoint* c_light_point = c_entity->get<TCompLightPoint>();
        if ((!c_light_spot && !c_light_point) ||
            (c_light_spot && !c_light_spot->isEnabled) ||
            (c_light_point && !c_light_point->isEnabled)) {
            continue;
        }

        TCompCollider * c_collider = c_entity->get<TCompCollider>();
        TCompTransform * c_transform = c_entity->get<TCompTransform>();
        if (c_collider->player_inside)
        {
            VEC3 dir = point - c_transform->getPosition();
            dir.Normalize();
            float distance = Clamp(VEC3::Distance(c_transform->getPosition(), point), 0.21f, 10000.f);
            if (!EnginePhysics.Raycast(c_transform->getPosition(), dir, distance - 0.2f, hit, (physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC), shadowDetectionFilter))
                return false;
        }
    }*/

    return true;
}

/* Method used to generate local points, we will apply transform later on */
// DEPRECATED.
void TCompShadowController::GenerateSurroundingPoints(const VEC3 & point)
{
    float radius = 0.35f;
    float nradius = (2 * (radius - 0.02f)); // Hardcoded for colliding purposes
    for (int x = 0; x < test_levels; x++)
    {
        for (int y = 0; y < test_amount; y++)
        {
            float lRadius = nradius / (x + 1);
            float t = y / (float)test_amount;
            float h = .5f * lRadius * cosf(t * 2.f * (float)M_PI) + point.x;
            float v = .5f * lRadius * sinf(t * 2.f * (float)M_PI) + point.z;
        }
    }
}
