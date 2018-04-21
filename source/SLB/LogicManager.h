#include <SLB/SLB.hpp>

class LogicManager
{
	int playerlife;
	float px, py, pz;

public:
	int numagents;

	LogicManager();
	void RespawnPlayer();
	void TeleportPlayer(float, float, float);
	float GetPlayerLife();
	void GetPlayerPos(float &, float &, float &);

	static void BootLuaSLB(SLB::Manager *m);
};

// ARRANQUE DE SLB
