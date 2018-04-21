#include "mcv_platform.h"
#include "LogicManager.h"

LogicManager::LogicManager()
{
	numagents = 37;
}

void LogicManager::RespawnPlayer()
{
	playerlife = 100;
	px = 0;
	py = 0;
	pz = 0;
}

void LogicManager::TeleportPlayer(float x, float y, float z)
{
	px = x;
	py = y;
	pz = z;
}

float LogicManager::GetPlayerLife()
{
	return playerlife;
}

void LogicManager::GetPlayerPos(float &x, float &y, float &z)
{
	x = px;
	y = py;
	z = pz;
}

/* Might take this out of here... */
void LogicManager::BootLuaSLB(SLB::Manager *m)
{
	SLB::Class< LogicManager >("LogicManager", m)
		// a comment/documentation for the class [optional]
		.comment("This is our wrapper of LogicManager class")
		// empty constructor, we can also wrapper constructors
		// with arguments using .constructor<TypeArg1,TypeArg2,..>()
		.constructor()
		// a method/function/value...
		.set("RespawnPlayer", &LogicManager::RespawnPlayer)
		.set("TeleportPlayer", &LogicManager::TeleportPlayer)
		.set("GetPlayerLife", &LogicManager::GetPlayerLife)
		//.set("GetPlayerPos", &LogicManager::GetPlayerPos)
		.property("numagents", &LogicManager::numagents)
		;
}
