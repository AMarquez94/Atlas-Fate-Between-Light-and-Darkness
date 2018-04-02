#include "mcv_platform.h"
#include "comp_noise_emitter.h"
#include "components/comp_transform.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"

DECL_OBJ_MANAGER("noise_emitter", TCompNoiseEmitter);

void TCompNoiseEmitter::debugInMenu() {
}

void TCompNoiseEmitter::load(const json& j, TEntityParseContext& ctx) {
	_timeToMakeNoise = 2.f;
}

void TCompNoiseEmitter::registerMsgs()
{
	DECL_MSG(TCompNoiseEmitter, TMsgTriggerEnter, onMsgTriggerEnter);
	DECL_MSG(TCompNoiseEmitter, TMsgTriggerExit, onMsgTriggerExit);
}

void TCompNoiseEmitter::onMsgTriggerEnter(const TMsgTriggerEnter & msg)
{

}

void TCompNoiseEmitter::onMsgTriggerExit(const TMsgTriggerExit & msg)
{

}

void TCompNoiseEmitter::update(float dt)
{
	if (_timer == -1) {
		TCompRigidbody *tCol = get<TCompRigidbody>();
		tCol->Resize(0.1f);
		_timer = 0.f;
	}
	_timer += dt;
	if (_timer > _timeToMakeNoise) {
		_timer = -1;
		TCompRigidbody *tCol = get<TCompRigidbody>();
		tCol->Resize(5.f);
	}
}
