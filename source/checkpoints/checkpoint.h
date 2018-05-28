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
	QUAT playerRot;
	bool saved = false;
};

struct EntityStatus {

};

class CCheckpoint {

public:

	CCheckpoint();

	//bool init();
	bool saveCheckPoint(VEC3 playerPos, QUAT playerRotation);
	bool loadCheckPoint();
	bool deleteCheckPoint();
  void debugInMenu();

private:

	PlayerStatus player;
	std::vector<EnemyStatus> enemies;
	std::vector<EntityStatus> entities;
	bool saved;

};