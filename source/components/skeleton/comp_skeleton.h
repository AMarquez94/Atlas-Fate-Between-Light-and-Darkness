#ifndef INC_COMP_SKELETON_H_
#define INC_COMP_SKELETON_H_

#include "../comp_base.h"
#include "render/cte_buffer.h"
#include "ctes.h"
#include "cal3d/animcallback.h"

class CalModel;

struct TCompSkeleton : public TCompBase {

	std::map<std::string, int> stringAnimationIdMap;
	std::vector<int> feetBonesId;
	int actualCycleAnimId[2];
	float cyclicAnimationWeight;
	float lastFrameCyclicAnimationWeight;
	CRenderCte<CCteSkinBones> cb_bones;
	CalModel* model = nullptr;
	std::list<int> cyclicPartialAnimationsPlaying;

	//Variables to root movement
	std::string animationToRootName = "";
	bool rotatingRoot = false;
	bool movingRoot = false;
	VEC3 lastAcum = VEC3(0, 0, 0);
	bool endingRoot = false;

	TCompSkeleton();
	~TCompSkeleton();

	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void debugInMenu();
	void renderDebug();
	void updateCtesBones();
	static void registerMsgs();


	void onMsgEntityCreated(const TMsgEntityCreated& msg);
	void onMsgAnimationCallback(const TMsgAnimationCallback& msg);
	void onMsgAnimationCompleted(const TMsgAnimationCompleted& msg);
	void onMsgSceneStop(const TMsgScenePaused& msg);
	
	void changeCyclicAnimation(int anim1Id, float speed = 1.0f, int anim2Id = -1, float weight = 1.0f, float in_delay = 0.15f, float out_delay = 0.15f);
	void executeActionAnimation(int animId, float speed = 1.0f, bool rootMovement = false, bool rootRot = false, float in_delay = 0.15f, float out_delay = 0.15f);

	void playPartialCyclicAnimation(int animId, float in_delay = 0.15f);
	void clearPartialCyclicAnimation(int animId, float out_delay = 0.15f);
	void clearAllPartialCyclicAnimation(float out_delay = 0.15f);


	bool removeActionAnimation(int animId);
	void setCyclicAnimationWeight(float new_value);
	float getCyclicAnimationWeight();
	int getAnimationIdByName(std::string animName);
	bool actionAnimationOnExecution();
	bool isExecutingCyclicAnimation(int animId);
	bool isExecutingActionAnimation(std::string animName);
	bool isExecutingActionAnimationForRoot(std::string animName);
	bool isExecutingActionAnimation(int animId);
	float getAnimationDuration(int animId);

	void guessFeetBonesId(int feetNum);
	void setFeetId(std::vector<int> feetId);

	std::vector<VEC3> getFeetPositions();
    VEC3 getBonePosition(const std::string & name);
	VEC3 getBonePositionById(int id);
	QUAT getBoneRotation(const std::string & name);
	void setBonePositionById(int id, VEC3 position);

	void executingMoveRootAnimation();
	void endingMoveRootAnimation();


	

	DECL_SIBLING_ACCESS();
};

#endif
