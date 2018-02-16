#include "mcv_platform.h"
#include "comp_transform.h"

DECL_OBJ_MANAGER("transform", TCompTransform);

void TCompTransform::debugInMenu() {
  CTransform::debugInMenu();
}


void TCompTransform::set(const CTransform& new_tmx) {
	*(CTransform*)this = new_tmx;
}

void TCompTransform::load(const json& j, TEntityParseContext& ctx) {
  CTransform::load( j );
}