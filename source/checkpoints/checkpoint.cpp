#include "mcv_platform.h"
#include "checkpoint.h"
#include "components/comp_tags.h"
#include "components/comp_transform.h"
#include "components/ia/comp_bt_patrol.h"
#include "components/ia/comp_bt_mimetic.h"
#include "components/ia/comp_bt_drone.h"
#include "entity/common_msgs.h"
#include "components/physics/comp_rigidbody.h"

CCheckpoint::CCheckpoint() {
    saved = false;
}

//bool CCheckpoint::init() {
//	saved = false;
//	return true;
//}

bool CCheckpoint::saveCheckPoint(VEC3 playerPos, QUAT playerRotation)
{
    bool error = false;
    enemies.clear();
    entities.clear();

    /* Player */
    player.playerPos = playerPos;
    player.playerRot = playerRotation;
    player.saved = true;

    /* Enemies */
    VHandles v_enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
    for (int i = 0; i < v_enemies.size(); i++) {
        CEntity* e_enemy = v_enemies[i];

        EnemyStatus enemy;

        TCompTransform* enemyTransform = e_enemy->get<TCompTransform>();
        assert(enemyTransform);
        enemy.enemyPosition = enemyTransform->getPosition();
        enemy.enemyRotation = enemyTransform->getRotation();

        assert(e_enemy->getName());
        enemy.enemyName = std::string(e_enemy->getName());

        TCompAIPatrol * enemyPatrol = e_enemy->get<TCompAIPatrol>();
        TCompAIMimetic * enemyMimetic = e_enemy->get<TCompAIMimetic>();
        TCompAIDrone * enemyDrone = e_enemy->get<TCompAIDrone>();
        assert(enemyPatrol || enemyMimetic || enemyDrone);
        if (enemyPatrol != nullptr) {
            enemy.enemyType = TCompIAController::BTType::PATROL;
            enemy.enemyIAStateName = enemyPatrol->getStateForCheckpoint();
        }
        else if (enemyMimetic != nullptr) {
            enemy.enemyType = TCompIAController::BTType::MIMETIC;
            enemy.enemyIAStateName = enemyMimetic->getStateForCheckpoint();
        }
        else if (enemyDrone != nullptr) {
            enemy.enemyType = TCompIAController::BTType::DRONE;
            enemy.enemyIAStateName = enemyDrone->getStateForCheckpoint();
        }

        enemy.saved = true;
        enemies.push_back(enemy);
    }

    /* Entities (movable p.e) */
    VHandles v_entities = CTagsManager::get().getAllEntitiesByTag(getID("movable"));
    for (int i = 0; i < v_entities.size(); i++) {
        CEntity* entity = v_entities[i];

        EntityStatus entityStatus;

        TCompTransform* entityTransform = entity->get<TCompTransform>();
        entityStatus.entityName = entity->getName();
        entityStatus.entityPos = entityTransform->getPosition();
        entityStatus.entityRot = entityTransform->getRotation();
        entityStatus.saved = true;

        entities.push_back(entityStatus);
    }

    saved = true;

    return !error;
}

bool CCheckpoint::loadCheckPoint()
{
    if (saved) {

        /* Player loading */
        CHandle h_player = EngineEntities.getPlayerHandle();
        if (h_player.isValid() && player.saved) {
            CEntity * e_player = h_player;
            TCompTransform * playerTransform = e_player->get<TCompTransform>();
            playerTransform->setPosition(player.playerPos);
            playerTransform->setRotation(player.playerRot);
            TCompRigidbody * playerRigidbody = e_player->get<TCompRigidbody>();
            playerRigidbody->setGlobalPose(player.playerPos, player.playerRot);

            /* Player Cameras */
            VHandles v_cameras = CTagsManager::get().getAllEntitiesByTag(getID("main_camera"));
            for (int i = 0; i < v_cameras.size(); i++) {
                CEntity* camera = v_cameras[i];
                TMsgCameraReset msg;
                msg.both_angles = true;
                camera->sendMsg(msg);
            }
        }

        /* Enemies loading */
        VHandles v_enemies = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
        for (int i = 0; i < v_enemies.size(); i++) {
            CEntity * e_enemy = v_enemies[i];

            /* Iterate through all the enemies in the checkpoint. If someone is missing, delete it (because it means it has been stunned) */
            bool found = false;
            int j = 0;
            while (j < enemies.size() && !found) {
                if (enemies[j].saved && enemies[j].enemyName.compare(e_enemy->getName()) == 0) {
                    found = true;
                }
                else {
                    j++;
                }
            }

            if (found) {
                /* update its position and state */
                TCompTransform * enemyTransform = e_enemy->get<TCompTransform>();
                enemyTransform->setPosition(enemies[j].enemyPosition);
                enemyTransform->setRotation(enemies[j].enemyRotation);
                TCompRigidbody * enemyRigidbody = e_enemy->get<TCompRigidbody>();
                enemyRigidbody->setGlobalPose(enemies[j].enemyPosition, enemies[j].enemyRotation);

                switch (enemies[j].enemyType) {
                case TCompAIMimetic::BTType::PATROL:
                {
                    TCompAIPatrol* enemyAI = e_enemy->get<TCompAIPatrol>();
                    enemyAI->setCurrentByName(enemies[j].enemyIAStateName);
                    break;
                }
                case TCompAIMimetic::BTType::MIMETIC:
                {
                    TCompAIMimetic* enemyAI = e_enemy->get<TCompAIMimetic>();
                    enemyAI->setCurrentByName(enemies[j].enemyIAStateName);
                    break;
                }
                case TCompAIDrone::BTType::DRONE:
                {
                    TCompAIDrone* enemyAI = e_enemy->get<TCompAIDrone>();
                    enemyAI->setCurrentByName(enemies[j].enemyIAStateName);
                    break;
                }
                }
            }
            else {
                /* delete entity */
                v_enemies[i].destroy();
            }
        }

        /* Entities loading */
        VHandles v_entities = CTagsManager::get().getAllEntitiesByTag(getID("movable"));
        for (int i = 0; i < v_entities.size(); i++) {
            CEntity* entity = v_entities[i];

            /* Iterate through all the entities in the checkpoint. */
            bool found = false;
            int j = 0;
            while (j < entities.size() && !found) {
                if (entities[j].saved && entities[j].entityName.compare(entity->getName()) == 0) {
                    found = true;
                }
                else {
                    j++;
                }
            }

            if (found) {
                TCompTransform * entityTransform = entity->get<TCompTransform>();
                entityTransform->setPosition(entities[j].entityPos);
                entityTransform->setRotation(entities[j].entityRot);
                TCompRigidbody * entityRigidbody = entity->get<TCompRigidbody>();
                if (entityRigidbody) {
                    entityRigidbody->setGlobalPose(entities[j].entityPos, entities[j].entityRot);
                }
            }
        }

        return true;
    }
    else {
        return false;
    }
}

bool CCheckpoint::deleteCheckPoint()
{
    player.saved = false;
    enemies.clear();
    entities.clear();
    saved = false;

    return true;
}

void CCheckpoint::debugInMenu()
{
    if (ImGui::TreeNode("Checkpoint")) {

        ImGui::Text("Saved: ");
        ImGui::SameLine();
        if (saved) {
            ImGui::TextColored(ImVec4(0, 255, 0, 255), "TRUE");
            if (ImGui::TreeNode("Player")) {
                ImGui::Text("Position: (%f, %f, %f)", player.playerPos.x, player.playerPos.y, player.playerPos.z);
                ImGui::Text("Rotation: (%f, %f, %f)", player.playerRot.x, player.playerRot.y, player.playerRot.z, player.playerRot.w);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Enemies")) {
                for (int i = 0; i < enemies.size(); i++) {
                    if (ImGui::TreeNode(enemies[i].enemyName.c_str())) {
                        ImGui::Text("Position: (%f, %f, %f)", enemies[i].enemyPosition.x, enemies[i].enemyPosition.y, enemies[i].enemyPosition.z);
                        ImGui::Text("State: %s", enemies[i].enemyIAStateName);
                        switch (enemies[i].enemyType) {
                        case TCompAIMimetic::BTType::PATROL:
                        {
                            ImGui::Text("Type: Patrol");
                            break;
                        }
                        case TCompAIMimetic::BTType::MIMETIC:
                        {
                            ImGui::Text("Type: Mimetic");
                            break;
                        }
                        case TCompAIMimetic::BTType::DRONE:
                        {
                            ImGui::Text("Type: Drone");
                            break;
                        }
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Entities")) {
                for (int i = 0; i < entities.size(); i++) {
                    if (ImGui::TreeNode(entities[i].entityName.c_str())) {
                        ImGui::Text("Position: (%f, %f, %f)", entities[i].entityPos.x, entities[i].entityPos.y, entities[i].entityPos.z);
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        else {
            ImGui::TextColored(ImVec4(255, 0, 0, 255), "FALSE");

        }

        ImGui::TreePop();
    }
}
