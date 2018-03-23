#pragma once

#include "components/comp_base.h"
class TCompTempPlayerController;

typedef void (TCompTempPlayerController::*actionhandler)(float);
typedef void (TCompTempPlayerController::*actionfinish)();

struct TMsgStateStart {
	actionhandler action_start;
	std::string meshname;
	float speed;

	DECL_MSG_ID();
};

struct TMsgStateFinish {
	actionfinish action_finish;
	std::string meshname;
	float speed;

	DECL_MSG_ID();
};

class TCompTempPlayerController : public TCompBase
{
	/* DEPRECATED */
	std::map<std::string, CRenderMesh*> mesh_states;

	physx::PxQueryFilterData shadowMergeFilter;
	physx::PxQueryFilterData playerFilter;

	actionhandler state;
	CHandle current_camera;

	float currentSpeed = 4.f;
	float rotationSpeed = 10.f;

	float stamina = 100.f;
	float maxStamina = 100.f;

	void onStateStart(const TMsgStateStart& msg);
	void onStateFinish(const TMsgStateFinish& msg);
	//void onPlayerShadows(const TMsgShadowState & msg);

	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void renderDebug();

	/* State functions */
	void walkState(float dt);
	void mergeState(float dt);
	void idleState(float dt);

	/* Auxiliar functions */
	void shadowState();
	void resetState();

	const bool ConcaveTest(void);
	const bool ConvexTest(void);
	const bool StaminaTest(void);

	static void registerMsgs();
};

