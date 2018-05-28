#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_auxcamera_shadowmerge.h"
#include "components/comp_transform.h"
#include "components/comp_name.h"
#include "components/comp_hierarchy.h"
#include "components/camera_controller/comp_camera_shadowmerge.h"
#include "components/camera_controller/comp_camera_thirdperson.h"
#include "components/physics/comp_collider.h"

DECL_OBJ_MANAGER("auxcamera_shadowmerge", TCompAuxCameraShadowMerge);

// Same as the thirdperson camera by now
// This will need to be changed for vertical shadow merging.

void TCompAuxCameraShadowMerge::debugInMenu()
{
	ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
	ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
	ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
	ImGui::DragFloat("BendIn Time", &_blendInTime, 0.f, 0.1f, 4.f);
}

void TCompAuxCameraShadowMerge::load(const json& j, TEntityParseContext& ctx)
{
	active = false;

	physx::PxFilterData pxFilterData;
	pxFilterData.word1 = FilterGroup::Scenario;
	cameraFilter.data = pxFilterData;

	_blendInTime = j.value("bendInTime", .4f);
}

void TCompAuxCameraShadowMerge::registerMsgs()
{
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgCameraActivated, onMsgCameraActive);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgCameraDeprecated, onMsgCameraDeprecated);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgCameraFullyActivated, onMsgCameraFullActive);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgSetCameraActive, onMsgCameraSetActive);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgSetCameraCancelled, onMsgCameraSetCancelled);
  DECL_MSG(TCompAuxCameraShadowMerge, TMsgScenePaused, onMsgScenePaused);
}

void TCompAuxCameraShadowMerge::onMsgCameraActive(const TMsgCameraActivated &msg)
{
	active = true;
}

void TCompAuxCameraShadowMerge::onMsgCameraFullActive(const TMsgCameraFullyActivated & msg)
{
	TCompTransform* myTrans = get<TCompTransform>();
	CEntity * eCamera = hCamera;
	TCompTransform *parentTrans = eCamera->get<TCompTransform>();
	TCompCameraShadowMerge *parentController = eCamera->get<TCompCameraShadowMerge>();

	parentTrans->setPosition(myTrans->getPosition());
	parentTrans->setRotation(myTrans->getRotation());
	parentController->setCurrentEuler(_current_euler.x , _current_euler.y);

	Engine.getCameras().blendOutCamera(CHandle(this).getOwner(), .0f);
}

void TCompAuxCameraShadowMerge::onMsgCameraDeprecated(const TMsgCameraDeprecated &msg)
{
	active = false;
  CEntity * eCamera = getEntityByName("TPCamera");
  TCompCameraThirdPerson* tpController = eCamera->get<TCompCameraThirdPerson>();
  tpController->setCurrentEuler(_current_euler.x);

  eCamera = getEntityByName("TPCameraCrouched");
  tpController = eCamera->get<TCompCameraThirdPerson>();
  tpController->setCurrentEuler(_current_euler.x);

  eCamera = getEntityByName("SMCameraHor");
  TCompCameraShadowMerge* smController = eCamera->get<TCompCameraShadowMerge>();
  smController->setCurrentEuler(_current_euler.x);

  eCamera = getEntityByName("SMCameraVer");
  smController = eCamera->get<TCompCameraShadowMerge>();
  smController->setCurrentEuler(_current_euler.x);
}

void TCompAuxCameraShadowMerge::onMsgCameraSetActive(const TMsgSetCameraActive & msg)
{
	/* Set the aux camera with the smcamera */
	bool differentCameras = msg.actualCamera.compare(msg.previousCamera) != 0;

	if (hCamera.isValid() && active && differentCameras) {
		//dbg("Cancel previous camera %s - new camera %s\n", ((CEntity *)hCamera)->getName(), msg.actualCamera.c_str());
		Engine.getCameras().cancelCamera(hCamera);
		Engine.getCameras().cancelCamera(CHandle(this).getOwner());
	}

	TCompTransform* myTrans = get<TCompTransform>();

	hCamera = getEntityByName(msg.actualCamera);

	CEntity * eCamera = hCamera;

	TCompTransform *tCameraPos = eCamera->get<TCompTransform>();
	TCompCameraShadowMerge *tCameraController = eCamera->get<TCompCameraShadowMerge>();

	myTrans->setPosition(tCameraPos->getPosition());
	myTrans->setRotation(tCameraPos->getRotation());

	_speed = tCameraController->getSpeed();
	_h_target = tCameraController->getCameraTarget();
	_clamp_angle = tCameraController->getClampAngle();
	_clipping_offset = tCameraController->getClippingOffset();
	_clamp_angle = tCameraController->getClampAngle();
	_current_euler = tCameraController->getCurrentEuler();
	_starting_pitch = tCameraController->getStartingPitch();

	/* Set the aux camera with the player */

	TCompTransform* targetTrans = ((CEntity*)_h_target)->get<TCompTransform>();
	TCompCollider* targetCollider = ((CEntity*)_h_target)->get<TCompCollider>();

	VEC3 dist = myTrans->getPosition() - targetTrans->getPosition();

	myTrans->setPosition(myTrans->getPosition() - dist);

	VEC3 vecToAim = msg.directionToLookAt;
	vecToAim.Normalize();

	float prey, prep, y, p;
	myTrans->getYawPitchRoll(&prey,&prep,0);
	myTrans->lookAt(myTrans->getPosition(), myTrans->getPosition() + vecToAim);

	float deltayaw = myTrans->getDeltaYawToAimTo(myTrans->getPosition() + vecToAim);

	myTrans->getYawPitchRoll(&y, &p, 0);

	_current_euler.x = _current_euler.x + (y - prey);
	_current_euler.y = _current_euler.y + (p - prep);
	myTrans->setPosition(myTrans->getPosition() + dist);
	
	Engine.getCameras().blendInCamera(CHandle(this).getOwner(), _blendInTime, CModuleCameras::EPriority::TEMPORARY);

	if (differentCameras) {
		tCameraPos->setPosition(myTrans->getPosition());
		tCameraPos->setRotation(myTrans->getRotation());
		tCameraController->setCurrentEuler(_current_euler.x, _current_euler.y);
		Engine.getCameras().blendInCamera(hCamera, _blendInTime, CModuleCameras::EPriority::GAMEPLAY);
	}
}

void TCompAuxCameraShadowMerge::onMsgCameraSetCancelled(const TMsgSetCameraCancelled & msg)
{
	if (active) {
		Engine.getCameras().cancelCamera(CHandle(this).getOwner());
	}
}

void TCompAuxCameraShadowMerge::onMsgScenePaused(const TMsgScenePaused & msg)
{
  paused = msg.isPaused;
}

void TCompAuxCameraShadowMerge::update(float dt)
{

	if (!paused && active) {
		if (!_h_target.isValid())
			return;

		TCompTransform* self_transform = get<TCompTransform>();
		TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>(); // we will need to consume this.
		assert(self_transform);
		assert(target_transform);

		float horizontal_delta = 0.f;
		float vertical_delta = 0.f;

		// To remove in the future.
		horizontal_delta = EngineInput.mouse()._position_delta.x;
		vertical_delta = -EngineInput.mouse()._position_delta.y;
		if (btRHorizontal.isPressed()) horizontal_delta = btRHorizontal.value;
		if (btRVertical.isPressed()) vertical_delta = btRVertical.value;

		// Verbose code
		_current_euler.x -= horizontal_delta * _speed * dt;
		_current_euler.y += vertical_delta * _speed * dt;
		_current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);

		// EulerAngles method based on mcv class
		VEC3 vertical_offset = 0.1f * target_transform->getUp() * _clipping_offset.y; // Change VEC3::up, for the players vertical angle, (TARGET VERTICAL)
		VEC3 horizontal_offset = self_transform->getLeft() * _clipping_offset.x;
		VEC3 target_position = target_transform->getPosition() + vertical_offset + horizontal_offset;

		self_transform->setPosition(target_position);
		self_transform->setYawPitchRoll(_current_euler.x, _current_euler.y, 0);

		float z_distance = CameraClipping(target_position, -self_transform->getFront());
		VEC3 new_pos = target_position + z_distance * -self_transform->getFront();
		self_transform->setPosition(new_pos);

		float inputSpeed = Clamp(fabs(btHorizontal.value) + fabs(btVertical.value), 0.f, 1.f);
		float current_fov = 70 + inputSpeed * 30; // Just doing some testing with the fov and speed
		setPerspective(deg2rad(current_fov), 0.1f, 1000.f);
	}
}

float TCompAuxCameraShadowMerge::CameraClipping(const VEC3 & origin, const VEC3 & dir)
{
	physx::PxRaycastHit hit;

	if (EnginePhysics.Raycast(origin, dir, _clipping_offset.z, hit, physx::PxQueryFlag::eSTATIC, cameraFilter))
		return Clamp(hit.distance - 0.1f, 0.5f, _clipping_offset.z);

	return _clipping_offset.z;
}
