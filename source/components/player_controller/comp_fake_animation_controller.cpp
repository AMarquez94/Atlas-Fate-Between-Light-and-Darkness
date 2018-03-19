#include "mcv_platform.h"
#include "comp_fake_animation_controller.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("fake_animation", TCompFakeAnimation);

void TCompFakeAnimation::debugInMenu() {
	//ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
	ImGui::Text("Animation name: %s", _animationName);
}

void TCompFakeAnimation::load(const json& j, TEntityParseContext& ctx) {
	//_sensitivity = j.value("sensitivity", _sensitivity);
	_animationName = "";
}

void TCompFakeAnimation::update(float dt)
{
	TCompTransform* c_transform = get<TCompTransform>();
	VEC3 pos = c_transform->getPosition();
	VEC3 front = c_transform->getFront();
	float yaw, pitch;
	getYawPitchFromVector(front, &yaw, &pitch);

	_time += dt;

	if (_animationName == "idle")
	{
		pos.y = 0.5f * sinf(_time);
		yaw = 0.f;
		pitch = 0.f;
	}
	else if (_animationName == "idleAction")
	{
		yaw = M_2_PI * _time;
	}
	else if (_animationName == "jump")
	{
		const float duration = 1.f;
		float ratio = _time / duration;
		pos.y = ratio * 5.f;

		TMsgSetFSMVariable groundMsg;
		groundMsg.variant.setName("onGround");
		groundMsg.variant.setBool(ratio >= 1.f);
		CEntity* e = CHandle(this).getOwner();
		e->sendMsg(groundMsg);
	}
	else if (_animationName == "walk")
	{
		const float speed = 5.f;
		pos.x += speed * dt;
		pos.y = 0.f;
	}

	// final values
	VEC3 newFront = getVectorFromYawPitch(yaw, pitch);
	c_transform->lookAt(pos, pos + newFront);
}

void TCompFakeAnimation::registerMsgs() {

	DECL_MSG(TCompFakeAnimation, TMsgAnimation, onAnimation);
}

void TCompFakeAnimation::onAnimation(const TMsgAnimation& msg)
{
	_animationName = msg.animationName;
	_time = 0.f;
}
