#include "mcv_platform.h"
#include "comp_noise_emitter.h"
#include "components/comp_transform.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"

DECL_OBJ_MANAGER("noise_emitter", TCompNoiseEmitter);

void TCompNoiseEmitter::debugInMenu() {
}

void TCompNoiseEmitter::load(const json& j, TEntityParseContext& ctx) {
	_timeToRepeatNoise = 2.f;
	_once = true;
}

void TCompNoiseEmitter::registerMsgs()
{
	DECL_MSG(TCompNoiseEmitter, TMsgTriggerEnter, onMsgTriggerEnter);
	DECL_MSG(TCompNoiseEmitter, TMsgTriggerExit, onMsgTriggerExit);
	DECL_MSG(TCompNoiseEmitter, TMsgMakeNoise, onMsgMakeNoise);
}

void TCompNoiseEmitter::onMsgTriggerEnter(const TMsgTriggerEnter & msg)
{
	if (msg.h_other_entity != _hSource) {
		bool found = false;
		for (int i = 0; !found && i < hEntitiesInNoiseRadius.size(); i++) {
			if (hEntitiesInNoiseRadius[i] == msg.h_other_entity) {
				found = true;
			}
		}

		if (!found) {
			hEntitiesInNoiseRadius.emplace_back(msg.h_other_entity);
		}
	}
}

void TCompNoiseEmitter::onMsgTriggerExit(const TMsgTriggerExit & msg)
{
	if (msg.h_other_entity != _hSource) {
		bool found = false;
		for (int i = 0; !found && i < hEntitiesInNoiseRadius.size(); i++) {
			if (hEntitiesInNoiseRadius[i] == msg.h_other_entity) {
				hEntitiesInNoiseRadius.erase(hEntitiesInNoiseRadius.begin() + i);
				found = true;
			}
		}
	}
}

void TCompNoiseEmitter::onMsgMakeNoise(const TMsgMakeNoise & msg)
{
	_timer = INFINITY;
	_timeToRepeatNoise = msg.timeToRepeat;
	_onceNoiseMade = false;
	_once = msg.isOnlyOnce;
	resizeEmitter(msg.noiseRadius);

	dbg("Noise: radius %f - timeToRepeat %f - once %s\n", msg.noiseRadius, _timeToRepeatNoise, _once ? "true" : "false");
}

void TCompNoiseEmitter::resizeEmitter(float radius)
{
	TCompTransform * cTransform = get<TCompTransform>();
	TCompCollider * cCollider = get<TCompCollider>();
	CPhysicsSphere * sphere = (CPhysicsSphere *)cCollider->config;
	sphere->radius = radius;
	physx::PxShape * shape = cCollider->config->createShape();
	cCollider->config->actor->detachShape(*cCollider->config->shape);
	cCollider->config->actor->attachShape(*shape);
	cCollider->config->shape = shape;
	shape->release();
}

void TCompNoiseEmitter::update(float dt)
{
	_timer += dt;

	if (_timer > _timeToRepeatNoise) {
		if (!_once || _once && !_onceNoiseMade) {
			
			TCompTransform * tPos = get<TCompTransform>();
			TMsgNoiseMade msg;
			msg.hNoiseSource = _hSource;
			msg.noiseOrigin = tPos->getPosition();
			for (int i = 0; i < hEntitiesInNoiseRadius.size(); i++) {

				/* TODO: tirar rayos y esas vergas para amortiguar sonido con paredes */
				CEntity *e = hEntitiesInNoiseRadius[i];
				e->sendMsg(msg);
			}

			_timer = 0;
			if (_once) {
				_onceNoiseMade = true;
			}
		}
	}
}
