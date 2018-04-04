#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "../comp_base.h"
#include "render/cte_buffer.h"
#include "ctes.h"

class CalModel;

struct TCompSkeleton : public TCompBase {

	struct AnimationSet {
		int* animations;
		float weight = 1.0f;
	};

  int actualCycleId;
  float weights;
  CRenderCte<CCteSkinBones> cb_bones;
  CalModel* model = nullptr;
  TCompSkeleton();
  ~TCompSkeleton();
  void load(const json& j, TEntityParseContext& ctx);
  void update(float dt);
  void debugInMenu();
  void renderDebug();
  void updateCtesBones();
  DECL_SIBLING_ACCESS();
};

#endif
