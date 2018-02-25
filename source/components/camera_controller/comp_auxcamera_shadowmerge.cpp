#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_auxcamera_shadowmerge.h"
#include "components/comp_transform.h"
#include "components/comp_name.h"
#include "components/comp_hierarchy.h"
#include "components/camera_controller/comp_camera_shadowmerge.h"
#include "components/physics/comp_collider.h"

DECL_OBJ_MANAGER("auxcamera_shadowmerge", TCompAuxCameraShadowMerge);

// Same as the thirdperson camera by now
// This will need to be changed for vertical shadow merging.

void TCompAuxCameraShadowMerge::debugInMenu()
{
	ImGui::DragFloat3("Offsets", &_clipping_offset.x, 0.1f, -20.f, 20.f);
	ImGui::DragFloat2("Angles", &_clamp_angle.x, 0.1f, -90.f, 90.f);
	ImGui::DragFloat("Speed", &_speed, 0.1f, 0.f, 20.f);
}

void TCompAuxCameraShadowMerge::load(const json& j, TEntityParseContext& ctx)
{
	//// Read from the json all the input data
	//_speed = j.value("speed", 1.5f);
	//_target_name = j.value("target", "");
	//_clamp_angle = loadVEC2(j["clampangle"]);
	//_clipping_offset = loadVEC3(j["offset"]);
	//_clamp_angle = VEC2(deg2rad(_clamp_angle.x), deg2rad(_clamp_angle.y));

	//// Load the target and set his axis as our axis.
	//_h_target = ctx.findEntityByName(_target_name);
	//TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();

	//float yaw, pitch, roll;
	//target_transform->getYawPitchRoll(&yaw, &pitch, &roll);
	//_original_euler = VEC2(yaw, pitch + deg2rad(j.value("starting_pitch", -60.f)));
	//_current_euler = _original_euler;

	pause = false;
	active = false;
}

void TCompAuxCameraShadowMerge::registerMsgs()
{
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgEntityCreated, onMsgEntityCreated);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgCameraActivated, onMsgCameraActive);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgCameraDeprecated, onMsgCameraDeprecated);
	DECL_MSG(TCompAuxCameraShadowMerge, TMsgCameraFullyActivated, onMsgCameraFullActive);
}

void TCompAuxCameraShadowMerge::onMsgEntityCreated(const TMsgEntityCreated & msg)
{
	TCompHierarchy * tHierarchy = get<TCompHierarchy>();
	_h_parent = getEntityByName(tHierarchy->parent_name);
	TCompCameraShadowMerge *tParentController = ((CEntity *)_h_parent)->get<TCompCameraShadowMerge>();

	_speed = tParentController->getSpeed();
	_h_target = tParentController->getCameraTarget();
	_clamp_angle = tParentController->getClampAngle();
	_clipping_offset = tParentController->getClippingOffset();
	_clamp_angle = tParentController->getClampAngle();
}

void TCompAuxCameraShadowMerge::onMsgCameraActive(const TMsgCameraActivated &msg)
{
	active = true;
	/*TCompTransform* targetTrans = ((CEntity*)_h_target)->get<TCompTransform>();
	TCompCollider* targetCollider = ((CEntity*)_h_target)->get<TCompCollider>();
	TCompTransform* myTrans = get<TCompTransform>();


	VEC3 dist = myTrans->getPosition() - targetTrans->getPosition();

	dbg("Dist (%f, %f, %f)\n", dist.x, dist.y, dist.z);
	dbg("PPos (%f, %f, %f)\n", targetTrans->getPosition().x, targetTrans->getPosition().y, targetTrans->getPosition().z);
	myTrans->setPosition(myTrans->getPosition() - dist);
	dbg("New pos (%f, %f, %f)\n", myTrans->getPosition().x, myTrans->getPosition().y, myTrans->getPosition().z);
	float deltayaw = myTrans->getDeltaYawToAimTo(myTrans->getPosition() + targetCollider->normal_gravity);
	dbg("deltayaw %f\n", deltayaw);
	_current_euler.x = _current_euler.x + deltayaw;
	myTrans->setPosition(myTrans->getPosition() + dist);*/
}

void TCompAuxCameraShadowMerge::onMsgCameraFullActive(const TMsgCameraFullyActivated & msg)
{
	TCompTransform* myTrans = get<TCompTransform>();
	TCompTransform *parentTrans = ((CEntity*)_h_parent)->get<TCompTransform>();
	TCompCameraShadowMerge *parentController = ((CEntity*)_h_parent)->get<TCompCameraShadowMerge>();
	std::string parentName = ((CEntity*)_h_parent)->getName();

	parentTrans->setPosition(myTrans->getPosition());
	parentController->setCurrentEuler(_current_euler.x, _current_euler.y);

	Engine.getCameras().blendOutCamera(CHandle(this).getOwner(), 0.f);
	Engine.getCameras().blendInCamera(getEntityByName(parentName), .0f, CModuleCameras::EPriority::GAMEPLAY);

	//Engine.getCameras().blendInCamera();
}

void TCompAuxCameraShadowMerge::onMsgCameraDeprecated(const TMsgCameraDeprecated &msg)
{
	active = false;
	_current_euler.y = _original_euler.y;
	dbg("Camera inactive %s\n", ((TCompName*)get<TCompName>())->getName());
}

void TCompAuxCameraShadowMerge::update(float dt)
{

	if (!pause) {
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

		//VEC2 current_clamp = VEC2::Zero;
		//float c_angle = rad2deg(cos(target_transform->getUp().Dot(-EnginePhysics.gravity)));
		//if (target_transform->getFront().Dot(-EnginePhysics.gravity) > 0) current_clamp = VEC2(_clamp_angle.x - c_angle, _clamp_angle.y - c_angle);
		//else current_clamp = VEC2(_clamp_angle.x + c_angle, _clamp_angle.y + c_angle);

		// Verbose code
		_current_euler.x -= horizontal_delta * _speed * dt;

		if (active) {
			_current_euler.y += vertical_delta * _speed * dt;
			_current_euler.y = Clamp(_current_euler.y, -_clamp_angle.y, -_clamp_angle.x);
		}

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

	if (btDebugPause.getsPressed()) {
		pause = !pause;
	}
}

float TCompAuxCameraShadowMerge::CameraClipping(const VEC3 & origin, const VEC3 & dir)
{
	CModulePhysics::RaycastHit hit;
	if (EnginePhysics.Raycast(origin, dir, _clipping_offset.z, hit, EnginePhysics.eSTATIC, EnginePhysics.getFilterByName("scenario")))
		return Clamp(hit.distance - 0.1f, 0.5f, _clipping_offset.z);

	return _clipping_offset.z;
}
