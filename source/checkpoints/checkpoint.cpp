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

bool CCheckpoint::saveCheckPoint(const std::string& name, VEC3 playerPos, VEC3 playerLookAt)
{
    bool error = false;
    enemies.clear();
    entities.clear();

    checkpoint_name = name;

    /* Player */
    player.playerPos = playerPos;
    player.playerLookAt = playerLookAt;
    player.saved = true;

    /* Enemies */
    VHandles v_enemies = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));
    for (int i = 0; i < v_enemies.size(); i++) {
        CEntity* e_enemy = v_enemies[i];
        TCompAIPatrol * enemyPatrol = e_enemy->get<TCompAIPatrol>();
        if (enemyPatrol != nullptr) {

            EnemyStatus enemy;
            enemy.enemyName = std::string(e_enemy->getName());
            enemy.enemyType = TCompIAController::BTType::PATROL;
            enemy.enemyIAStateName = enemyPatrol->getStateForCheckpoint();
            if (enemy.enemyIAStateName != "") {
                TCompTransform* enemyTransform = e_enemy->get<TCompTransform>();
                enemy.enemyPosition = enemyTransform->getPosition();
                enemy.enemyRotation = enemyTransform->getRotation();
            }
            enemy.saved = true;
            enemies.push_back(enemy);
        }
    }

    ///* Entities (movable p.e) */
    //VHandles v_entities = CTagsManager::get().getAllEntitiesByTag(getID("movable"));
    //for (int i = 0; i < v_entities.size(); i++) {
    //    CEntity* entity = v_entities[i];

    //    EntityStatus entityStatus;

    //    TCompTransform* entityTransform = entity->get<TCompTransform>();
    //    entityStatus.entityName = entity->getName();
    //    entityStatus.entityPos = entityTransform->getPosition();
    //    entityStatus.entityRot = entityTransform->getRotation();
    //    entityStatus.saved = true;

    //    entities.push_back(entityStatus);
    //}

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
            playerTransform->lookAt(player.playerPos, player.playerLookAt);
            TCompRigidbody * playerRigidbody = e_player->get<TCompRigidbody>();
            playerRigidbody->setGlobalPose(player.playerPos, playerTransform->getRotation());

            /* Player Cameras */
            EngineLogic.execScript("resetMainCameras()");
        }

        /* Patrol loading */
        VHandles v_enemies = CTagsManager::get().getAllEntitiesByTag(getID("patrol"));
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
                if (enemies[j].enemyIAStateName != "") {
                    TCompTransform * enemyTransform = e_enemy->get<TCompTransform>();
                    enemyTransform->setPosition(enemies[j].enemyPosition);
                    enemyTransform->setRotation(enemies[j].enemyRotation);
                    TCompRigidbody * enemyRigidbody = e_enemy->get<TCompRigidbody>();
                    enemyRigidbody->setGlobalPose(enemies[j].enemyPosition, enemies[j].enemyRotation);
                    TMsgEnemyStunnedCheckpoint msg;
                    e_enemy->sendMsg(msg);
                    //TCompAIPatrol* enemyAI = e_enemy->get<TCompAIPatrol>();
                    //enemyAI->setCurrentByName(enemies[j].enemyIAStateName);
                }
            }
            else {
                /* delete entity */
                v_enemies[i].destroy();
            }
        }

        EngineLogic.execEvent(CModuleLogic::Events::CHECKPOINT_LOADED, checkpoint_name);

        ///* Entities loading */
        //VHandles v_entities = CTagsManager::get().getAllEntitiesByTag(getID("movable"));
        //for (int i = 0; i < v_entities.size(); i++) {
        //    CEntity* entity = v_entities[i];

        //    /* Iterate through all the entities in the checkpoint. */
        //    bool found = false;
        //    int j = 0;
        //    while (j < entities.size() && !found) {
        //        if (entities[j].saved && entities[j].entityName.compare(entity->getName()) == 0) {
        //            found = true;
        //        }
        //        else {
        //            j++;
        //        }
        //    }

        //    if (found) {
        //        TCompTransform * entityTransform = entity->get<TCompTransform>();
        //        entityTransform->setPosition(entities[j].entityPos);
        //        entityTransform->setRotation(entities[j].entityRot);
        //        TCompRigidbody * entityRigidbody = entity->get<TCompRigidbody>();
        //        if (entityRigidbody) {
        //            entityRigidbody->setGlobalPose(entities[j].entityPos, entities[j].entityRot);
        //        }
        //    }
        //}

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
            ImGui::Text("Name: %s", checkpoint_name.c_str());
            if (ImGui::TreeNode("Player")) {
                ImGui::Text("Position: (%f, %f, %f)", player.playerPos.x, player.playerPos.y, player.playerPos.z);
                ImGui::Text("LookAt: (%f, %f, %f)", player.playerLookAt.x, player.playerLookAt.y, player.playerLookAt.z);
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
