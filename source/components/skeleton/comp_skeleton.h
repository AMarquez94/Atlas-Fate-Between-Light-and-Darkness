#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "../comp_base.h"
#include "render/cte_buffer.h"
#include "ctes.h"

class CalModel;

struct TCompSkeleton : public TCompBase {

	int actualCycleAnimId1 = -1;
	int actualCycleAnimId2 = -1;
	float cyclicAnimationWeight;
	CRenderCte<CCteSkinBones> cb_bones;
	CalModel* model = nullptr;
	TCompSkeleton();
	~TCompSkeleton();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void debugInMenu();
	void renderDebug();
	void updateCtesBones();

	void changeCyclicAnimation(int animId);
	void changeCyclicAnimation(int anim1Id, int anim2Id, float weight);
	void executeActionAnimation(int animId);
	void setCyclicAnimationWeight(float new_value);
	float getCyclicAnimationWeight();
	int getAnimationIdByName(std::string animName);

	DECL_SIBLING_ACCESS();
};

#endif
