#pragma once

#include "components/ia/comp_bt.h"

struct EnemyStatus {
    std::string enemyName;
    std::string enemyIAStateName;
    TCompIAController::BTType enemyType;
    VEC3 enemyPosition;
    QUAT enemyRotation;
    bool saved = false;
};

struct PlayerStatus {
    VEC3 playerPos;
    VEC3 playerLookAt;
    bool saved = false;
};

struct EntityStatus {
    std::string entityName;
    VEC3 entityPos;
    QUAT entityRot;
    bool saved = true;
};

class CCheckpoint {

public:

    CCheckpoint();

    //bool init();
    bool saveCheckPoint(const std::string& name, VEC3 playerPos, VEC3 playerLookAt);
    bool loadCheckPoint();
    bool deleteCheckPoint();
    const bool isSaved() { return saved; };
    void debugInMenu();

private:

    std::string checkpoint_name;
    PlayerStatus player;
    std::vector<EnemyStatus> enemies;
    std::vector<EntityStatus> entities;
    bool saved;

};