#include "mcv_platform.h"
#include "modules/module_animations.h"


void CModuleAnimations::update(float delta) {

}
void CModuleAnimations::render() {

}

void CModuleAnimations::registerModelToHandle(CalModel* model, CHandle handle) {
	this->mapModelToHandle[model] = handle;
}

CHandle CModuleAnimations::getHandleByModel(CalModel* model) {

	return this->mapModelToHandle[model];
}

void AnimationCallback::AnimationUpdate(float anim_time, CalModel *model) {
	
	CHandle owner = EngineAnimations.mapModelToHandle[model];
	CEntity* e = owner;
	TMsgAnimationCallback msg;
	msg.function_to_call = this->luaFunction;
	e->sendMsg(msg);
	dbg("module:     %s      name: %s\n",this->luaFunction.c_str() , e->getName());
}

void AnimationCallback::AnimationComplete(CalModel *model) {

}