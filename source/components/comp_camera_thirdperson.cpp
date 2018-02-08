#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_thirdperson.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("camera_thirdperson", TCompCameraThirdPerson);

void TCompCameraThirdPerson::debugInMenu()
{
	ImGui::DragFloat("Vertical offset", &_clipping_offset.x, 0.1f, 0.f, 20.f);
	ImGui::DragFloat("Horizontal offset", &_clipping_offset.y, 0.1f, 0.f, 20.f);
}

void TCompCameraThirdPerson::load(const json& j, TEntityParseContext& ctx)
{
	_target_name = j.value("target", "");
	_clipping_offset = loadVEC2(j["offset"]);
	_h_target = ctx.findEntityByName(_target_name);
}

void TCompCameraThirdPerson::update(float dt)
{
	TCompTransform* self_transform = get<TCompTransform>();
	TCompTransform* target_transform = ((CEntity*)_h_target)->get<TCompTransform>();
	assert(self_transform);
	assert(target_transform);

	VEC3 vertical_offset = _clipping_offset.x * VEC3::Up;
	VEC3 horizontal_offset = _clipping_offset.y * -target_transform->getFront();
	VEC3 newpos = target_transform->getPosition() + vertical_offset + horizontal_offset;//target_transform->getPosition() + _clipping_offset * angled_director;
	VEC3 target_forwarded = target_transform->getPosition() + target_transform->getFront() * 4.0f;

	self_transform->lookAt(newpos, target_forwarded);
}

void TCompCameraThirdPerson::mouseMovement(void)
{
	//VEC2 prev_cursor = cursor;
	//cursor = VEC2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	//VEC2 delta_cursor = cursor - prev_cursor;
}
