#pragma once

#include "modules/module.h"
#include "entity/entity.h"

struct AnimationCallback : public CalAnimationCallback {
	std::string luaFunction;
	std::string animationName;
	void AnimationUpdate(float anim_time, CalModel *model);
	void AnimationComplete(CalModel *model);
};

struct AnimationAudioCallback : public CalAnimationCallback {
    std::string audioName;
    bool relativeToPlayer = true;
    void AnimationUpdate(float anim_time, CalModel *model);
    void AnimationComplete(CalModel *model);
};

class CModuleAnimations : public IModule
{
public:
	std::map<CalModel*, CHandle> mapModelToHandle;

	CModuleAnimations(const std::string& aname) : IModule(aname) { }
	void update(float delta) override;
	void render() override;

	void registerModelToHandle(CalModel* model, CHandle handle);
	CHandle getHandleByModel(CalModel* model);
};

