#include "mcv_platform.h"
#include "comp_bt_player.h"
#include "btnode.h"
#include "components/comp_name.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"
#include "components/comp_render.h"
#include "components/comp_group.h"
#include "components/object_controller/comp_cone_of_light.h"
#include "components/physics/comp_rigidbody.h"
#include "components/physics/comp_collider.h"
#include "components/lighting/comp_emission_controller.h"
#include "physics/physics_collider.h"
#include "render/render_utils.h"
#include "render/render_objects.h"
#include "components/player_controller/comp_player_animator.h"

DECL_OBJ_MANAGER("ai_player", TCompAIPlayer);

void TCompAIPlayer::debugInMenu() {

	TCompIAController::debugInMenu();
	CEntity * me = getEntityByName("The Player");
	TCompTransform *tpos = me->get<TCompTransform>();


	ImGui::Text("My Pos: (%f, %f, %f)", tpos->getPosition().x, tpos->getPosition().y, tpos->getPosition().z);
	ImGui::Text("Waypoint: (%f, %f, %f)", _waypoints[0].position.x, _waypoints[0].position.y, _waypoints[0].position.z);
	ImGui::Text("Enabled player AI: %s", enabledPlayerAI ? "TRUE" : "FALSE");
	ImGui::Text("Speed: %f", _speed);
	ImGui::Text("Rotation Speed: %f", _rotationSpeed);

}

void TCompAIPlayer::preUpdate(float dt)
{
}

void TCompAIPlayer::postUpdate(float dt)
{
}

void TCompAIPlayer::load(const json& j, TEntityParseContext& ctx) {

	loadActions();
	loadConditions();
	loadAsserts();

    /* TODO: */
    // crouch => DONE
    // sm => ALMOST (shader)
    // sm ver => ALMOST (shader, wall, floor)
    // sm fall => ALMOST (shader, floor)
    // attack => ALMOST (enemy)
    // inhibitor => DONE

	createRoot("player", BTNode::EType::PRIORITY, nullptr, nullptr, nullptr);
    addChild("player", "playerActivated", BTNode::EType::PRIORITY, (BTCondition)&TCompAIPlayer::conditionHasBeenEnabled, nullptr, nullptr);

    addChild("playerActivated", "crouchTutorial", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPlayer::conditionCrouchTutorial, nullptr, nullptr);
    addChild("crouchTutorial", "resetTimersCrouchTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetTimersCrouchTutorial, nullptr);
    addChild("crouchTutorial", "walkCrouchTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationWalk, nullptr);
    addChild("crouchTutorial", "crouchCrouchTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationCrouch, nullptr);
    addChild("crouchTutorial", "resetBTCrouchTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetBT, nullptr);

    addChild("playerActivated", "smTutorial", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPlayer::conditionSMTutorial, nullptr, nullptr);
    addChild("smTutorial", "resetTimersSMTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetTimersSMTutorial, nullptr);
    addChild("smTutorial", "idleSMTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationIdle, nullptr);
    addChild("smTutorial", "beginSMTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionStartSM, nullptr);
    addChild("smTutorial", "smSMTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationSM, nullptr);
    addChild("smTutorial", "exitSMTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionEndSM, nullptr);
    addChild("smTutorial", "resetBTSMTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetBT, nullptr);

    addChild("playerActivated", "removeInhibitorTutorial", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPlayer::conditionRemoveInhibitorTutorial, nullptr, nullptr);
    addChild("removeInhibitorTutorial", "resetTimersInhibitorTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetTimersRemoveInhibitorTutorial, nullptr);
    addChild("removeInhibitorTutorial", "idleInhibitorTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationIdle, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorStartInhibitorTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitor, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_1", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorMiddleInhibitorTutorial_1", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitorMiddle, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_2", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorMiddleInhibitorTutorial_2", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitorMiddle, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_3", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorMiddleInhibitorTutorial_3", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitorMiddle, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_4", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorMiddleInhibitorTutorial_5", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitorMiddle, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_5", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorMiddleInhibitorTutorial_6", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitorMiddle, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_6", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "inhibitorFinnishInhibitorTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationInhibitorFinnish, nullptr);
    addChild("removeInhibitorTutorial", "waitInhibitorTutorial_7", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("removeInhibitorTutorial", "resetBTInhibitorTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetBT, nullptr);

    addChild("playerActivated", "attackTutorial", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPlayer::conditionAttackTutorial, nullptr, nullptr);
    addChild("attackTutorial", "resetTimersAttackTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetTimersAttackTutorial, nullptr);
    addChild("attackTutorial", "crouchAttackTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationStandingCrouch, nullptr);
    addChild("attackTutorial", "attackAttackTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationAttack, nullptr);
    addChild("attackTutorial", "waitAttackTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("attackTutorial", "resetBTAttackTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetBT, nullptr);

    addChild("playerActivated", "smfallTutorial", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPlayer::conditionSMFallTutorial, nullptr, nullptr);
    addChild("smfallTutorial", "resetTimersSMFallTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetTimersSMFallTutorial, nullptr);
    addChild("smfallTutorial", "fallingSMFallTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationFalling, nullptr);
    addChild("smfallTutorial", "beginsmSMFallTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionStartSM, nullptr);
    addChild("smfallTutorial", "smSMFallTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationSM, nullptr);
    addChild("smfallTutorial", "exitsmSMFallTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionEndSM, nullptr);
    addChild("smfallTutorial", "resetBTSMFallTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetBT, nullptr);

    addChild("playerActivated", "smVerTutorial", BTNode::EType::SEQUENCE, (BTCondition)&TCompAIPlayer::conditionSMVerTutorial, nullptr, nullptr);
    addChild("smVerTutorial", "resetTimersSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetTimersSMVerTutorial, nullptr);
    addChild("smVerTutorial", "beginSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionStartSM, nullptr);
    addChild("smVerTutorial", "smSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionAnimationSM, nullptr);
    addChild("smVerTutorial", "moveSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionSMVerMove, nullptr);
    addChild("smVerTutorial", "exitsmSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionEndSM, nullptr);
    addChild("smVerTutorial", "waitSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionWait, nullptr);
    addChild("smVerTutorial", "resetBTSMVerTutorial", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionResetBT, nullptr);

	addChild("playerActivated", "goToWpt", BTNode::EType::ACTION, (BTCondition)&TCompAIPlayer::conditionCinematicMode, (BTAction)&TCompAIPlayer::actionGoToWpt, nullptr);
    //addChild("playerActivated", "default", BTNode::EType::ACTION, nullptr, (BTAction)&TCompAIPlayer::actionDefault, nullptr);

	enabledPlayerAI = j.value("enabled", false);
	_speed = j.value("speed", 1.0f);
	_rotationSpeed = j.value("rotationSpeed", 1.0f);

	if (j.count("waypoints") > 0) {
		auto& j_waypoints = j["waypoints"];
		_waypoints.clear();
		for (auto it = j_waypoints.begin(); it != j_waypoints.end(); ++it) {

			Waypoint wpt;
			assert(it.value().count("position") == 1);
			assert(it.value().count("lookAt") == 1);

			wpt.position = loadVEC3(it.value()["position"]);
			wpt.lookAt = loadVEC3(it.value()["lookAt"]);
			wpt.minTime = it.value().value("minTime", 5.f);
			_waypoints.push_back(wpt);
		}
	}

	_currentWaypoint = 0;
}

void TCompAIPlayer::onMsgPlayerAIEnabled(const TMsgPlayerAIEnabled& msg) {
	enabledPlayerAI = msg.enableAI;
    _currentState = getStateEnumFromString(msg.state);
}

void TCompAIPlayer::onMsgEntityCreated(const TMsgEntityCreated& msg) {

	myHandle = CHandle(this);
    TCompTransform* mypos = get<TCompTransform>();
    initial_pos = mypos->getPosition();
    initial_rot = mypos->getRotation();
}


void TCompAIPlayer::registerMsgs()
{
	DECL_MSG(TCompAIPlayer, TMsgPlayerAIEnabled, onMsgPlayerAIEnabled);
	DECL_MSG(TCompAIPlayer, TMsgEntityCreated, onMsgEntityCreated);

}

void TCompAIPlayer::loadActions() {
	actions_initializer.clear();
}

void TCompAIPlayer::loadConditions() {
	conditions_initializer.clear();
}

void TCompAIPlayer::loadAsserts() {
	asserts_initializer.clear();
}

TCompAIPlayer::EState TCompAIPlayer::getStateEnumFromString(const std::string & stateName)
{
    /* TODO: Add more */
    if (stateName.compare("cinematic") == 0) {
        return TCompAIPlayer::EState::CINEMATIC;
    }
    else if (stateName.compare("crouch_tutorial") == 0) {
        return TCompAIPlayer::EState::TUT_CROUCH;
    }
    else if (stateName.compare("sm_tutorial") == 0) {
        return TCompAIPlayer::EState::TUT_SM;
    }
    else if (stateName.compare("remove_inhibitor_tutorial") == 0) {
        return TCompAIPlayer::EState::TUT_INHIBITOR;
    }
    else if (stateName.compare("attack_tutorial") == 0) {
        return TCompAIPlayer::EState::TUT_ATTACK;
    }
    else if (stateName.compare("sm_fall_tutorial") == 0) {
        return TCompAIPlayer::EState::TUT_SM_FALL;
    }
    else if (stateName.compare("sm_ver_tutorial") == 0) {
        return TCompAIPlayer::EState::TUT_SM_VER;
    }
    else {
        return TCompAIPlayer::EState::NUM_STATES;
    }
}

/* ACTIONS */

BTNode::ERes TCompAIPlayer::actionGoToWpt(float dt)
{

	CEntity* e = CHandle(this).getOwner();
	e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::WALK, 1.0 });
	return move(dt) ? BTNode::ERes::LEAVE : BTNode::ERes::STAY;

}

BTNode::ERes TCompAIPlayer::actionDefault(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::CROUCH_WALK);
    return BTNode::ERes::STAY;
}

BTNode::ERes TCompAIPlayer::actionResetTimersCrouchTutorial(float dt)
{
    _timer = 0.f;
    _maxTimer = 1.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionResetTimersSMTutorial(float dt)
{
    _timer = 0.f;
    _maxTimer = 1.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionResetTimersRemoveInhibitorTutorial(float dt)
{
    _timer = 0.f;
    _maxTimer = 1.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionResetTimersAttackTutorial(float dt)
{
    _timer = 0.f;
    _maxTimer = 1.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionResetTimersSMFallTutorial(float dt)
{
    _timer = 0.f;
    _maxTimer = 1.f;
    TCompTransform* mypos = get<TCompTransform>();
    mypos->setPosition(initial_pos + VEC3(0, 4.f, 0.f));
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionResetTimersSMVerTutorial(float dt)
{
    _timer = 0.f;
    _maxTimer = 2.f;
    TCompTransform* mypos = get<TCompTransform>();
    mypos->setPosition(initial_pos);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionResetBT(float dt)
{
    setCurrent(nullptr);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionAnimationWalk(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        _maxTimer = 2.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
        my_anim->playAnimation(TCompPlayerAnimator::EAnimation::WALK);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionAnimationCrouch(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
        my_anim->playAnimation(TCompPlayerAnimator::EAnimation::CROUCH_WALK);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionAnimationIdle(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
        my_anim->playAnimation(TCompPlayerAnimator::EAnimation::IDLE);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionStartSM(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::SM_POSE);
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::SM_ENTER);

    CHandle(this).getOwner().sendMsg(TMsgFadeBody{ false });
    CHandle(this).getOwner().sendMsg(TMsgFadeBody{ false });

    // Replace this when weapons are finished
    CHandle h_tutorial_weap_left = getEntityByName("tuto_weap_disc_left");
    CHandle h_tutorial_weap_right = getEntityByName("tuto_weap_disc_right");
    h_tutorial_weap_left.sendMsg(TMsgFadeBody{ false });
    h_tutorial_weap_right.sendMsg(TMsgFadeBody{ false });

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionAnimationSM(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        //TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
        //my_anim->playAnimation(TCompPlayerAnimator::EAnimation::SM_POSE);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionEndSM(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::IDLE);

    CHandle(this).getOwner().sendMsg(TMsgFadeBody{ true });

    // Replace this when weapons are finished
    CHandle h_tutorial_weap_left = getEntityByName("tuto_weap_disc_left");
    CHandle h_tutorial_weap_right = getEntityByName("tuto_weap_disc_right");
    h_tutorial_weap_left.sendMsg(TMsgFadeBody{ true });
    h_tutorial_weap_right.sendMsg(TMsgFadeBody{ true });

    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionAnimationInhibitor(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::METRALLA_START);
    _maxTimer = my_anim->getAnimationDuration((TCompAnimator::EAnimation)TCompPlayerAnimator::EAnimation::METRALLA_START) / 2.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionAnimationInhibitorMiddle(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::METRALLA_MIDDLE);
    _maxTimer = my_anim->getAnimationDuration((TCompAnimator::EAnimation)TCompPlayerAnimator::EAnimation::METRALLA_MIDDLE) / 4.f;
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionAnimationInhibitorFinnish(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::METRALLA_FINISH);
    _maxTimer = my_anim->getAnimationDuration((TCompAnimator::EAnimation)TCompPlayerAnimator::EAnimation::METRALLA_FINISH);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionAnimationStandingCrouch(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
        my_anim->playAnimation(TCompPlayerAnimator::EAnimation::CROUCH_IDLE);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionAnimationAttack(float dt)
{
    TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
    my_anim->playAnimation(TCompPlayerAnimator::EAnimation::ATTACK);
    _maxTimer = my_anim->getAnimationDuration((TCompAnimator::EAnimation)TCompPlayerAnimator::EAnimation::ATTACK);
    return BTNode::ERes::LEAVE;
}

BTNode::ERes TCompAIPlayer::actionWait(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionAnimationFalling(float dt)
{
    _timer += dt;
    if (_timer > _maxTimer) {
        _timer = 0.f;
        return BTNode::ERes::LEAVE;
    }
    else {
        TCompPlayerAnimator* my_anim = get<TCompPlayerAnimator>();
        my_anim->playAnimation(TCompPlayerAnimator::EAnimation::FALL);
        TCompTransform* mypos = get<TCompTransform>();
        mypos->setPosition(mypos->getPosition() + VEC3(0.f, -4.f, 0.f) * dt);
        return BTNode::ERes::STAY;
    }
}

BTNode::ERes TCompAIPlayer::actionSMVerMove(float dt)
{
    _timer = 0.f;
    _maxTimer = 1.f;
    TCompTransform* mypos = get<TCompTransform>();
    mypos->setPosition(initial_pos + 1.f * mypos->getFront() + VEC3(0.f, 1.f, 0.f));
    return BTNode::ERes::LEAVE;
}

/* CONDITIONS */

bool TCompAIPlayer::conditionHasBeenEnabled(float dt) {
	return enabledPlayerAI;
}

bool TCompAIPlayer::conditionCinematicMode(float dt)
{
    return _currentState == EState::CINEMATIC;
}

bool TCompAIPlayer::conditionCrouchTutorial(float dt)
{
    return _currentState == EState::TUT_CROUCH;
}

bool TCompAIPlayer::conditionSMTutorial(float dt)
{
    return _currentState == EState::TUT_SM;
}

bool TCompAIPlayer::conditionRemoveInhibitorTutorial(float dt)
{
    return _currentState == EState::TUT_INHIBITOR;
}

bool TCompAIPlayer::conditionAttackTutorial(float dt)
{
    return _currentState == TCompAIPlayer::EState::TUT_ATTACK;
}

bool TCompAIPlayer::conditionSMFallTutorial(float dt)
{
    return _currentState == TCompAIPlayer::EState::TUT_SM_FALL;
}

bool TCompAIPlayer::conditionSMVerTutorial(float dt)
{
    return _currentState == TCompAIPlayer::EState::TUT_SM_VER;
}

/* ASSERTS */


/* AUX FUNCTIONS */
bool TCompAIPlayer::move(float dt) {
	TCompTransform* mypos = get<TCompTransform>();
	float yaw, pitch, roll;
	VEC3 dir = VEC3(_waypoints[_currentWaypoint].position.x - mypos->getPosition().x, _waypoints[_currentWaypoint].position.y - mypos->getPosition().y, _waypoints[_currentWaypoint].position.z - mypos->getPosition().z);
	dir.Normalize();
	VEC3 newPos = mypos->getPosition() + (dir * dt * _speed);
	float amountMoved = VEC3::Distance(newPos, mypos->getPosition());

	if (VEC3::Distance(mypos->getPosition(), _waypoints[_currentWaypoint].position) > VEC3::Distance(newPos, _waypoints[_currentWaypoint].position)) {
		mypos->getYawPitchRoll(&yaw, &pitch, &roll);

		float dir_yaw = getYawFromVector(dir);
		Quaternion my_rotation = mypos->getRotation();
		Quaternion new_rotation = Quaternion::CreateFromYawPitchRoll(dir_yaw, pitch, 0);
		Quaternion quat = Quaternion::Lerp(my_rotation, new_rotation, _rotationSpeed * dt);
		mypos->setRotation(quat);
		mypos->setPosition(newPos);

		return false;
	}
	else {
		CEntity* e = CHandle(this).getOwner();
		e->sendMsg(TCompPlayerAnimator::TMsgExecuteAnimation{ TCompPlayerAnimator::EAnimation::IDLE, 1.0 });
		mypos->setPosition(_waypoints[_currentWaypoint].position);
		_currentWaypoint++;

		if (_currentWaypoint < _waypoints.size()) {
			return false;
		}
		else {
			enabledPlayerAI = false;
			return true;

		}
	}
}