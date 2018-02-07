#include "mcv_platform.h"
#include "ai_controller.h"
#include "entity/entity.h"

void CAIController::Init()
{

}

void CAIController::Recalc()
{
	// this is a trusted jump as we've tested for coherence in ChangeState
	(this->*current_state)();
}


void CAIController::ChangeState(std::string newstate)
{
	// try to find a state with the suitable name
	if (statemap.find(newstate) == statemap.end())
	{
		exit(-1);
	}

	state = newstate;
	current_state = statemap[state];
}


void CAIController::AddState(std::string name, statehandler sh)
{
	// try to find a state with the suitable name
	if (statemap.find(name) != statemap.end())
	{
		exit(-1);
	}

	statemap[name] = sh;
}

void CAIController::SetEntity(TEntity * target)
{
	entity_self = target;
}

void CAIController::Configure(const json& j)
{
	std::string name = j["target"];
	entity_target = getEntityByName(name.c_str());
}

const std::string& CAIController::GetState()
{
	return state;
}