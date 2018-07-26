#include "mcv_platform.h"
#include "components/skeleton/callback_def.h"


CallbackDefinition* CallbackDefinition::GetInstance() {
	if (_instance == nullptr) {
		_instance = ;
	}
	return _instance;
}

void CallbackDefinition::AnimationCallback::AnimationUpdate(float anim_time, CalModel *model) {
	EngineLogic.execScript("hola");
	//mapModelToHandle;
	//model->ownerHandler.sendMsg();
}

void CallbackDefinition::AnimationCallback::AnimationComplete(CalModel *model) {

}

