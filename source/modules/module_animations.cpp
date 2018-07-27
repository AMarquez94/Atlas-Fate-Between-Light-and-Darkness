#include "mcv_platform.h"
#include "modules/module_animations.h"


void CModuleAnimations::update(float delta) {

}
void CModuleAnimations::render() {

}

void CModuleAnimations::registerModelToHandle(CalModel* model, CHandle handle) {
	this->mapModelToHandle[model] = handle;
	dbg("registraaaaaa");
}

CHandle CModuleAnimations::getHandleByModel(CalModel* model) {

	return this->mapModelToHandle[model];
}

void AnimationCallback::AnimationUpdate(float anim_time, CalModel *model) {
	
	dbg("%s\n",this->luaFunction.c_str() );
}

void AnimationCallback::AnimationComplete(CalModel *model) {

}