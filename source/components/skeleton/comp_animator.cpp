#include "mcv_platform.h"
#include "comp_animator.h"
#include "components/comp_transform.h"
#include "components/comp_fsm.h"

DECL_OBJ_MANAGER("animator", TCompAnimator);

void TCompAnimator::debugInMenu() {
	
}

void TCompAnimator::load(const json& j, TEntityParseContext& ctx) {

}

void TCompAnimator::initializeAnimations() {
	
	
}

bool TCompAnimator::initializeAnimation(EAnimation animation, EAnimationType animationType, EAnimationSize animationSize, std::string animationName, std::string secondAnimationName, float weight, float speed, bool rootMovement, bool rootRotationMovement) {
	
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	AnimationSet auxAnimSet;
	auxAnimSet.animation = animation;
	auxAnimSet.animationType = animationType;
	auxAnimSet.animationSize = animationSize;
	auxAnimSet.animationName = animationName;
	auxAnimSet.animationId = compSkeleton->getAnimationIdByName(auxAnimSet.animationName);
	if (auxAnimSet.animationId == -1) {
		fatal("The first animation non exists");
		return false;
	}
	auxAnimSet.secondAnimationName = secondAnimationName;
	auxAnimSet.secondAnimationId = compSkeleton->getAnimationIdByName(auxAnimSet.secondAnimationName);
	if (auxAnimSet.animationSize == EAnimationSize::DOUBLE && auxAnimSet.secondAnimationId == -1) {
		fatal("The second animation non exists");
		return false;
	}
	auxAnimSet.weight = weight;
	auxAnimSet.speed = speed;
	auxAnimSet.rootMovement = rootMovement;
	auxAnimSet.rootRotationMovement = rootRotationMovement;

	stringToAnimationsMap[animationName] = animation;
	animationsMap[animation] = auxAnimSet;
	return false;
}

void TCompAnimator::update(float dt)
{

}

void TCompAnimator::registerMsgs() {
//	DECL_MSG(TCompAnimator, TMsgEntityCreated, onCreated);
}

bool TCompAnimator::playAnimationConverted(EAnimation animation, float speed) {

	assert(animationsMap.find(animation) != animationsMap.end());
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	AnimationSet animSet = animationsMap[animation];

	if (animSet.animationType == TCompAnimator::EAnimationType::CYCLIC && this->isPlayingAnimation(animSet.animation)) {
		return false;
	}

	int anim1id = animSet.animationId;
	int anim2id = animSet.secondAnimationId;
	float weight = animSet.weight;
	float aux_speed = animSet.speed;
	bool rootMov = animSet.rootMovement;
	bool rootRot = animSet.rootRotationMovement;
	if (speed != 1.0f) aux_speed = speed;

	switch (animSet.animationType)
	{
	    case EAnimationType::CYCLIC:
		    compSkeleton->changeCyclicAnimation(anim1id, aux_speed, anim2id, weight);
            return true;
		    break;
	    case EAnimationType::ACTION:
		    compSkeleton->executeActionAnimation(anim1id, aux_speed, rootMov, rootRot);
            return true;
		    break;
		case EAnimationType::CYCLIC_PARTIAL:
			compSkeleton->playPartialCyclicAnimation(anim1id);
			break;
        default:
            return false;
            break;
	}
}

bool  TCompAnimator::isCyclic(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	return animSet.animationType == EAnimationType::CYCLIC;
}

bool  TCompAnimator::isComposed(EAnimation animation) {

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	return animSet.animationSize == EAnimationSize::DOUBLE;
}

TCompAnimator::EAnimation TCompAnimator::actualAnimation() {

	return actualCyclicAnimation.animation;
}

void  TCompAnimator::setFeetIds(std::vector<int> feetBonesId) {

	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();

	compSkeleton->setFeetId(feetBonesId);
}

void TCompAnimator::setFeetNumAndCalculate(int numFeet) {

	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();

	compSkeleton->guessFeetBonesId(numFeet);
}

std::vector<VEC3> TCompAnimator::getFeetPositions() {

	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();

	return compSkeleton->getFeetPositions();
}


float  TCompAnimator::getAnimationDuration(EAnimation animation) {

	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	
	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];

	return compSkeleton->getAnimationDuration(animSet.animationId);
}

bool TCompAnimator::isPlayingAnimation(EAnimation animation) {
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();

	if (animationsMap.find(animation) == animationsMap.end()) {
		fatal("Animation doesn't exists");
	}
	AnimationSet animSet = animationsMap[animation];
	bool toReturn = false;
	if (animSet.animationType == TCompAnimator::EAnimationType::ACTION) {
		if (animSet.rootRotationMovement) {
			toReturn = compSkeleton->isExecutingActionAnimationForRoot(animSet.animationName);
		}
		else {
			toReturn = compSkeleton->isExecutingActionAnimation(animSet.animationName);
		}
		
	}
	if (animSet.animationType == TCompAnimator::EAnimationType::CYCLIC) {
		toReturn = compSkeleton->isExecutingCyclicAnimation(animSet.animationId);
	}
	return toReturn;
}

TCompAnimator::EAnimation TCompAnimator::getAnimationByName(std::string animation_name) {

	return stringToAnimationsMap[animation_name];
}

void TCompAnimator::removeAction(EAnimation animation) {

	AnimationSet aux_animSet = animationsMap[animation];
	CEntity* e = ownHandle;
	TCompSkeleton * compSkeleton = e->get<TCompSkeleton>();
	compSkeleton->removeActionAnimation(aux_animSet.animationId);
}
