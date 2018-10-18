#pragma once

#include "modules/module.h"
#include "geometry/interpolators.h"

class CModuleCameras : public IModule
{
public:
    enum EPriority { DEFAULT = 0, GAMEPLAY, TEMPORARY, CINEMATIC, DEBUG, NUM_PRIORITIES };

	CModuleCameras(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;

	void setDefaultCamera(CHandle camera);
	void setOutputCamera(CHandle camera);
	void blendInCamera(CHandle camera, float blendTime = 0.f, EPriority priority = EPriority::DEFAULT, Interpolator::IInterpolator* interpolator = nullptr);
	void blendOutCamera(CHandle camera, float blendTime = 0.f);
	void cancelCamera(CHandle camera);

	void deleteAllCameras();

	CHandle getOutputCamera();
	CHandle getCurrentCamera();
	VEC3 getResultPos() { return resultPos; };

    EPriority getPriorityFromString(const std::string& priority);
    Interpolator::IInterpolator* getInterpolatorFromString(const std::string& interpolator);

private:
	void renderInMenu();

	struct TMixedCamera
	{
		enum EState { ST_BLENDING_IN, ST_IDLE, ST_BLENDING_OUT, ST_CANCELLED };

		CHandle camera;
		EState state = EState::ST_IDLE;
		EPriority type = EPriority::DEFAULT;
		float blendInTime = 0.f; // gets to full ratio (1.f) in n seconds
		float blendOutTime = 0.f; // gets to full ratio (1.f) in n seconds
		float weight = 0.f;  // blend weight ratio
		float time = 0.f; // current blending time
		Interpolator::IInterpolator* interpolator = nullptr;

		void blendIn(float duration);
		void blendOut(float duration);
	};

	using VMixedCameras = std::vector<TMixedCamera>;

	TMixedCamera* getMixedCamera(CHandle camera);
	void blendCameras(const CCamera* camera1, const CCamera* camera2, float ratio, CCamera* output);
	void checkDeprecated();

	VMixedCameras _mixedCameras;
	CHandle _defaultCamera;
	CHandle _outputCamera;
	CHandle _currentCamera;

	VEC3 resultPos;
};
