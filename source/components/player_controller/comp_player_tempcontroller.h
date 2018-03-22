#pragma once

#include "components/comp_base.h"
class TCompTempPlayerController;

typedef void (TCompTempPlayerController::*actionhandler)(float);

struct TMsgChangeState {
	actionhandler action_state;
	std::string meshname;
	float speed;

	DECL_MSG_ID();
};

class TCompTempPlayerController : public TCompBase
{
	/* DEPRECATED */
	std::map<std::string, CRenderMesh*> mesh_states;

	actionhandler state;
	physx::PxQueryFilterData shadowMergeFilter;
	physx::PxQueryFilterData playerFilter;

	float currentSpeed = 4.f;
	float rotationSpeed = 10.f;

	void onChangeState(const TMsgChangeState& msg);
	//void onPlayerShadows(const TMsgShadowState & msg);

	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	/* State functions */
	void playerMotion(float dt);
	void playerShadowMotion(float dt);
	void idleState(float dt);

	/* Auxiliar functions */
	void shadowState();
	const bool ConcaveTest(void);
	const bool ConvexTest(void);

	static void registerMsgs();
};

