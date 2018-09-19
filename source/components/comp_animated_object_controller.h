#pragma once

#include "components/comp_base.h"
#include "components/skeleton/comp_skeleton.h"
#include "mcv_platform.h"

class TCompAnimatedObjController : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:

	struct AnimationInfo {

		std::string animationName = "";
		std::string track_name = "";
		std::string source = "";
		float speedFactor = 1.0f;
		bool loop = false;
	};

	std::vector<AnimationInfo> animationInfos;
	std::vector<std::string> object_names;
	std::vector<CHandle> object_handles;

	bool getNames = false;

	std::map<std::string, int> name_to_id_animations;

	std::string starting_animation;

	static void registerMsgs();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void onGroupCreated(const TMsgEntitiesGroupCreated &msg);

	void playAnimation(std::string anim_name);

};
