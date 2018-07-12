#include "mcv_platform.h"
#include "module_cameras.h"
#include "components/comp_camera.h"
#include "utils/utils.h"

void CModuleCameras::TMixedCamera::blendIn(float duration)
{
	blendInTime = duration;
	state = blendInTime == 0.f ? ST_IDLE : ST_BLENDING_IN;
	time = 0.f;
}

void CModuleCameras::TMixedCamera::blendOut(float duration)
{
	blendOutTime = duration;
	state = ST_BLENDING_OUT;
	time = 0.f;
}

CModuleCameras::CModuleCameras(const std::string& name)
	: IModule(name)
{}

bool CModuleCameras::start()
{
	return true;
}

bool CModuleCameras::stop()
{
	return true;
}

void CModuleCameras::update(float delta)
{
	CCamera resultCamera;

	if (_defaultCamera.isValid())
	{
		CEntity* e = _defaultCamera;
		TCompCamera* c_camera = e->get<TCompCamera>();
		blendCameras(c_camera, c_camera, 1.f, &resultCamera);
		_currentCamera = _defaultCamera;
	}

	for (int i = 0; i < NUM_PRIORITIES; ++i)
	{
		for (auto& mc : _mixedCameras)
		{
			if (mc.type != i)
				continue;

			mc.time += delta;
			if (mc.state == TMixedCamera::ST_BLENDING_IN)
			{
				mc.weight = Clamp(mc.time / mc.blendInTime, 0.f, 1.f);
				if (mc.weight >= 1.f)
				{
					mc.state = TMixedCamera::ST_IDLE;
					mc.time = 0.f;
					
					TMsgCameraFullyActivated msg;
					mc.camera.sendMsg(msg);
				}
			}
			else if (mc.state == TMixedCamera::ST_BLENDING_OUT)
			{
				mc.weight = 1.f - Clamp(mc.time / mc.blendOutTime, 0.f, 1.f);
			}
			else if (mc.state == TMixedCamera::ST_CANCELLED) {
				mc.weight = 0.f;
			}

			if (mc.weight > 0.f)
			{
				float ratio = mc.weight;
				if (mc.interpolator)
					ratio = mc.interpolator->blend(0.f, 1.f, ratio);

				// blend them
				CEntity* e = mc.camera;
				TCompCamera* c_camera = e->get<TCompCamera>();
				blendCameras(&resultCamera, c_camera, ratio, &resultCamera);
				_currentCamera = mc.camera;
			}
		}
	}

	checkDeprecated();

	// remove deprecated ones
	VMixedCameras::iterator it = _mixedCameras.begin();
	while (it != _mixedCameras.end())
	{
		if (it->weight <= 0.f) {
			TMsgCameraDeprecated msg;
			it->camera.sendMsg(msg);
			it = _mixedCameras.erase(it);
		}
		else {
			++it;
		}
	}

	// save the result
	if (_outputCamera.isValid())
	{
		CEntity* e = _outputCamera;
		TCompCamera* c_camera = e->get<TCompCamera>();
		blendCameras(&resultCamera, &resultCamera, 1.f, c_camera);
	}
}

void CModuleCameras::setDefaultCamera(CHandle camera)
{
	_defaultCamera = camera;
}

void CModuleCameras::setOutputCamera(CHandle camera)
{
	_outputCamera = camera;
}

void CModuleCameras::blendInCamera(CHandle camera, float blendTime, EPriority priority, Interpolator::IInterpolator* interpolator)
{
  if (!camera.isValid())
    return;

	TMixedCamera* mc = getMixedCamera(camera);
	if (!mc)
	{

		TMsgCameraActivated msg;
        msg.previousCamera = getCurrentCamera();

		TMixedCamera new_mc;
		new_mc.camera = camera;
		new_mc.type = priority;
		new_mc.interpolator = interpolator;
		new_mc.blendIn(blendTime);

		camera.sendMsg(msg);

		if (blendTime == 0.f) {
			new_mc.weight = 1.f;
			TMsgCameraFullyActivated msg;
			camera.sendMsg(msg);
		}

		_mixedCameras.push_back(new_mc);
	}
}

void CModuleCameras::cancelCamera(CHandle camera)
{
	TMixedCamera* mc = getMixedCamera(camera);
	if (mc)
	{
		mc->state = TMixedCamera::ST_CANCELLED;
	}
}

void CModuleCameras::deleteAllCameras()
{
	_mixedCameras.clear();
}

CHandle CModuleCameras::getOutputCamera()
{
	return _outputCamera;
}

CHandle CModuleCameras::getCurrentCamera()
{
	return _currentCamera;
}

void CModuleCameras::blendOutCamera(CHandle camera, float blendTime)
{
	TMixedCamera* mc = getMixedCamera(camera);
	if (mc)
	{
		mc->blendOut(blendTime);
	}
}

CModuleCameras::TMixedCamera* CModuleCameras::getMixedCamera(CHandle camera)
{
	for (auto& mc : _mixedCameras)
	{
		if (mc.camera == camera)
		{
			return &mc;
		}
	}
	return nullptr;
}

void CModuleCameras::blendCameras(const CCamera* camera1, const CCamera* camera2, float ratio, CCamera* output)
{
	assert(camera1 && camera2 && output);

	resultPos = VEC3::Lerp(camera1->getPosition(), camera2->getPosition(), ratio);
	VEC3 newFront = VEC3::Lerp(camera1->getFront(), camera2->getFront(), ratio);
	float newFov = lerp(camera1->getFov(), camera2->getFov(), ratio);
	float newZnear = lerp(camera1->getZNear(), camera2->getZNear(), ratio);
	float newZfar = lerp(camera1->getZFar(), camera2->getZFar(), ratio);

	output->setPerspective(newFov, newZnear, newZfar);
	output->lookAt(resultPos, resultPos + newFront);
}

void CModuleCameras::checkDeprecated()
{
	// checks if there are cameras that should be removed
	for (int i = (int)_mixedCameras.size() - 1; i >= 0; --i)
	{
		TMixedCamera& mc = _mixedCameras[i];
		if (mc.type == GAMEPLAY && mc.weight >= 1.f && mc.state != TMixedCamera::ST_CANCELLED )
		{
			// check if there's another gameplay camera
			// if so, and prepare it to be removed
			for (size_t j = 0; j < i; ++j)
			{
				TMixedCamera& otherMc = _mixedCameras[j];
				if (otherMc.type == GAMEPLAY)
				{
					otherMc.weight = 0.f;
				}
			}
		}
		else if (TMixedCamera::ST_CANCELLED) {
			//mc.weight = 0.f;
		}
	}
}

void CModuleCameras::render()
{
	renderInMenu();
}

void renderInterpolator(const char* name, Interpolator::IInterpolator& interpolator)
{
	const int nsamples = 50;
	float values[nsamples];
	for (int i = 0; i < nsamples; ++i)
	{
		values[i] = interpolator.blend(0.f, 1.f, (float)i / (float)nsamples);
	}
	ImGui::PlotLines(name, values, nsamples, 0, 0,
		std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
		ImVec2(150, 80));
}

void CModuleCameras::renderInMenu()
{
	if (ImGui::TreeNode("Cameras"))
	{
		ImGui::Columns(5);
		ImGui::SetColumnWidth(0, 200);
		for (auto& mc : _mixedCameras)
		{
			CEntity* e = mc.camera;
			ImGui::Text(e->getName());
			ImGui::NextColumn();
			ImGui::Text("TYPE: %d", mc.type);
			ImGui::NextColumn();
			if (mc.state == TMixedCamera::ST_BLENDING_IN)
				ImGui::Text("Blending In");
			else if (mc.state == TMixedCamera::ST_BLENDING_OUT)
				ImGui::Text("Blending Out");
			else
				ImGui::Text("Idle");
			ImGui::NextColumn();
			ImGui::DragFloat("Time", &mc.time);
			ImGui::NextColumn();
			ImGui::ProgressBar(mc.weight);
			ImGui::NextColumn();
		}
		ImGui::TreePop();
		ImGui::Columns();
	}

	if (ImGui::TreeNode("Interpolators"))
	{
		renderInterpolator("Linear", Interpolator::TLinearInterpolator());
		renderInterpolator("Quad in", Interpolator::TQuadInInterpolator());
		renderInterpolator("Quad out", Interpolator::TQuadOutInterpolator());
		renderInterpolator("Quad in out", Interpolator::TQuadInOutInterpolator());
		renderInterpolator("Cubic in", Interpolator::TCubicInInterpolator());
		renderInterpolator("Cubic out", Interpolator::TCubicOutInterpolator());
		renderInterpolator("Cubic in out", Interpolator::TCubicInOutInterpolator());
		renderInterpolator("Quart in", Interpolator::TQuartInInterpolator());
		renderInterpolator("Quart out", Interpolator::TQuartOutInterpolator());
		renderInterpolator("Quart in out", Interpolator::TQuartInOutInterpolator());
		renderInterpolator("Quint in", Interpolator::TQuintInInterpolator());
		renderInterpolator("Quint out", Interpolator::TQuintOutInterpolator());
		renderInterpolator("Quint in out", Interpolator::TQuintInOutInterpolator());
		renderInterpolator("Back in", Interpolator::TBackInInterpolator());
		renderInterpolator("Back out", Interpolator::TBackOutInterpolator());
		renderInterpolator("Back in out", Interpolator::TBackInOutInterpolator());
		renderInterpolator("Elastic in", Interpolator::TElasticInInterpolator());
		renderInterpolator("Elastic out", Interpolator::TElasticOutInterpolator());
		renderInterpolator("Elastic in out", Interpolator::TElasticInOutInterpolator());
		renderInterpolator("Bounce in", Interpolator::TBounceInInterpolator());
		renderInterpolator("Bounce out", Interpolator::TBounceOutInterpolator());
		renderInterpolator("Bounce in out", Interpolator::TBounceInOutInterpolator());
		renderInterpolator("Circular in", Interpolator::TCircularInInterpolator());
		renderInterpolator("Circular out", Interpolator::TCircularOutInterpolator());
		renderInterpolator("Circular in out", Interpolator::TCircularInOutInterpolator());
		renderInterpolator("Expo in", Interpolator::TExpoInInterpolator());
		renderInterpolator("Expo out", Interpolator::TExpoOutInterpolator());
		renderInterpolator("Expo in out", Interpolator::TExpoInOutInterpolator());
		renderInterpolator("Sine in", Interpolator::TSineInInterpolator());
		renderInterpolator("Sine out", Interpolator::TSineOutInterpolator());
		renderInterpolator("Sine in out", Interpolator::TSineInOutInterpolator());

		ImGui::TreePop();
	}
}
