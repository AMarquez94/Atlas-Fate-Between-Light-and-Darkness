#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "../comp_base.h"
#include "render/cte_buffer.h"
#include "ctes.h"

class CalModel;

struct TCompSkeleton : public TCompBase {

	std::map<std::string, int> stringAnimationIdMap;
	int actualCycleAnimId[2];
	float cyclicAnimationWeight;
	float lastFrameCyclicAnimationWeight;
	CRenderCte<CCteSkinBones> cb_bones;
	CalModel* model = nullptr;

	TCompSkeleton();
	~TCompSkeleton();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void debugInMenu();
	void renderDebug();
	void updateCtesBones();

	void changeCyclicAnimation(int animId, float in_delay = 0.15f, float out_delay = 0.15f);
	void changeCyclicAnimation(int animId, float speed, float in_delay = 0.15f, float out_delay = 0.15f);
	void changeCyclicAnimation(int anim1Id, int anim2Id, float weight, float in_delay = 0.15f, float out_delay = 0.15f);
	void changeCyclicAnimation(int anim1Id, int anim2Id, float weight, float speed, float in_delay = 0.15f, float out_delay = 0.15f);
	void executeActionAnimation(int animId, float in_delay = 0.15f, float out_delay = 0.15f);
	void executeActionAnimation(int animId, float speed, float in_delay = 0.15f, float out_delay = 0.15f);
	void setCyclicAnimationWeight(float new_value);
	float getCyclicAnimationWeight();
	int getAnimationIdByName(std::string animName);
	bool actionAnimationOnExecution();
	bool isExecutingAnimation(int animId);

	DECL_SIBLING_ACCESS();
};

#endif
