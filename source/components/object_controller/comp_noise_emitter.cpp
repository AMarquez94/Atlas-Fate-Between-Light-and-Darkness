#include "mcv_platform.h"
#include "comp_noise_emitter.h"
#include "components/comp_transform.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"

DECL_OBJ_MANAGER("noise_emitter", TCompNoiseEmitter);

void TCompNoiseEmitter::debugInMenu() {}

void TCompNoiseEmitter::load(const json& j, TEntityParseContext& ctx) {
	_timeToRepeatNoise = -1.f;
	_once = true;
  emitter_sphere.radius = j.value("radius", 2.f);

  physx::PxFilterData pxFilterData;
  pxFilterData.word0 = FilterGroup::Enemy;
  PxNoiseEmitterFilterData.data = pxFilterData;
  PxNoiseEmitterFilterData.flags = physx::PxQueryFlag::eDYNAMIC;
}

void TCompNoiseEmitter::registerMsgs()
{
	DECL_MSG(TCompNoiseEmitter, TMsgScenePaused, onMsgScenePaused);
}

void TCompNoiseEmitter::onMsgScenePaused(const TMsgScenePaused & msg)
{
	paused = msg.isPaused;
}

void TCompNoiseEmitter::makeNoise(float noiseRadius, float timeToRepeat, bool isNoise, bool isOnlyOnce, bool isArtificial)
{
  _isNoise = isNoise;
  _timeToRepeatNoise = timeToRepeat;
  _once = isOnlyOnce;
  _artificial = isArtificial;
  emitter_sphere.radius = noiseRadius;

  _timer = INFINITY;
  _onceNoiseMade = false;
}

void TCompNoiseEmitter::changeNoiseRadius(float newRadius)
{
  emitter_sphere.radius = newRadius;
}



void TCompNoiseEmitter::update(float dt)
{
	if (!paused && emitter_sphere.isValid() && CHandle(this).getOwner().isValid()) {

		_timer += dt;

		if (_isNoise && _timer > _timeToRepeatNoise) {
			if (!_once || (_once && !_onceNoiseMade)) {
			
				TMsgNoiseMade msg;
				msg.hNoiseSource = CHandle(this).getOwner();
				TCompTransform * tPos = get<TCompTransform>();
				msg.noiseOrigin = tPos->getPosition();
				msg.isArtificialNoise = _artificial;

        std::vector<physx::PxOverlapHit> hits;
        if (EnginePhysics.Overlap(emitter_sphere, tPos->getPosition(), hits, PxNoiseEmitterFilterData)) {
          for (int i = 0; i < hits.size(); i++) {
            CHandle hitCollider;
            hitCollider.fromVoidPtr(hits[i].actor->userData);
            if (hitCollider.isValid()) {
              /* TODO: lanzar raycast y todo eso para sacar la atenuación del sonido y cosas así */
              CHandle enemy = hitCollider.getOwner();
              if (enemy.isValid() && enemy != CHandle(this).getOwner()) {
                enemy.sendMsg(msg);
              }
            }
          }
        }

				_timer = 0;
				if (_once) {
					_onceNoiseMade = true;
				}
			}
		}
	}
}
