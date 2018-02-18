#include "mcv_platform.h"
#include "comp_transform.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("transform", TCompTransform);

void TCompTransform::debugInMenu() {
  CTransform::debugInMenu();
}


void TCompTransform::set(const CTransform& new_tmx) {
	*(CTransform*)this = new_tmx;
}

void TCompTransform::load(const json& j, TEntityParseContext& ctx) {
  CTransform::load( j );
  set(ctx.root_transform.combineWith(*this));
}