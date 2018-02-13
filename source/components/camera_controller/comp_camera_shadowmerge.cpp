#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_camera_shadowmerge.h"
#include "../comp_transform.h"

DECL_OBJ_MANAGER("camera_shadowmerge", TCompCameraShadowMerge);

void TCompCameraShadowMerge::debugInMenu()
{

}

void TCompCameraShadowMerge::load(const json& j, TEntityParseContext& ctx)
{
	// Read from the json all the input data

}

void TCompCameraShadowMerge::update(float dt)
{

}

VEC3 TCompCameraShadowMerge::CameraClipping(void)
{
	return VEC3();
}
