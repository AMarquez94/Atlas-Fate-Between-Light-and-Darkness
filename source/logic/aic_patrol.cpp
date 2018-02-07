#include "mcv_platform.h"
#include "aic_patrol.h"

const float CAIPatrol::VISIONANGLE = 0.785;
const float CAIPatrol::VISIONDISTANCE = 4;

void CAIPatrol::Init()
{
	// insert all states in the map
	AddState("idle",(statehandler)&CAIPatrol::IdleState);
	AddState("seekwpt",(statehandler)&CAIPatrol::SeekWptState);
	AddState("nextwpt",(statehandler)&CAIPatrol::NextWptState);
	AddState("turnstate", (statehandler)&CAIPatrol::TurnState);
	AddState("followstate", (statehandler)&CAIPatrol::FollowState);
	ChangeState("idle");
}


void CAIPatrol::IdleState()
{
	//if (GetAsyncKeyState('Q')!=0) ChangeState("seekwpt");

	if (VEC3::Distance(entity_self->transform.getPosition(), entity_target->transform.getPosition()) < VISIONDISTANCE)
	{
		float deltaYaw = entity_self->transform.getDeltaYawToAimTo(entity_target->transform.getPosition());

		if (fabsf(deltaYaw) < VISIONANGLE)
			ChangeState("turnstate");
	}
}

void CAIPatrol::TurnState(void)
{
	float yaw, pitch, roll;
	entity_self->transform.getYawPitchRoll(&yaw, &pitch, &roll);
	float deltaYaw = entity_self->transform.getDeltaYawToAimTo(entity_target->transform.getPosition());

	if (fabsf(deltaYaw) < 0.05)
	{
		ChangeState("followstate");
	}
	else
	{
		if (entity_self->transform.isInLeft(entity_target->transform.getPosition()))
			entity_self->transform.setYawPitchRoll(yaw + 0.1, pitch, roll);
		else
			entity_self->transform.setYawPitchRoll(yaw - 0.1, pitch, roll);
	}
}

void CAIPatrol::FollowState()
{
	VEC3 my_pos = entity_self->transform.getPosition();
	VEC3 his_pos = entity_target->transform.getPosition();
	VEC3 dir = (his_pos - my_pos);
	dir.Normalize();
	VEC3 new_pos = my_pos + dir * 0.01f;

	entity_self->transform.setPosition(new_pos);

	if (VEC3::Distance(entity_self->transform.getPosition(), entity_target->transform.getPosition()) < 0.1)
	{
		ChangeState("idle");
	}
}

void CAIPatrol::SeekWptState()
{
	if (GetAsyncKeyState('E')!=0) ChangeState("nextwpt");
}

void CAIPatrol::NextWptState()
{
	MessageBox(NULL,TEXT("nextwpt"),TEXT("cambio estado"),MB_ICONASTERISK | MB_YESNOCANCEL );
	ChangeState("idle");
}