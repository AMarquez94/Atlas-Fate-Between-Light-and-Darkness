#ifndef INC_CALLBACK_DEF_H_
#define INC_CALLBACK_DEF_H_

#include "cal3d/cal3d.h"
#include "handle/handle_def.h"


class CallbackDefinition {
	std::map<CalModel, CHandle> mapModelToHandle;
	static CallbackDefinition* _instance;

	static CallbackDefinition* GetInstance();
	struct AnimationCallback : public CalAnimationCallback {

		const char* luaFunction;
		void AnimationUpdate(float anim_time, CalModel *model);
		void AnimationComplete(CalModel *model);
	};

};

#endif
