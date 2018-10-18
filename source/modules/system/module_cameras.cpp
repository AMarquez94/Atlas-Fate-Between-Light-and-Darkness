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
    /* Create interpolators */
    //static Interpolator::TLinearInterpolator linear_interpolator;
    //static Interpolator::TQuadInInterpolator quadin_interpolator;
    //static Interpolator::TQuadOutInterpolator quadout_interpolator;
    //static Interpolator::TQuadInOutInterpolator quadinout_interpolator;
    //static Interpolator::TCubicInInterpolator cubicin_interpolator;
    //static Interpolator::TCubicOutInterpolator cubicout_interpolator;
    //static Interpolator::TCubicInOutInterpolator cubicinout_interpolator;
    //static Interpolator::TQuartInInterpolator quartin_interpolator;
    //static Interpolator::TQuartOutInterpolator quartout_interpolator;
    //static Interpolator::TQuadInOutInterpolator quartinout_interpolator;
    //static Interpolator::TQuintInInterpolator quintin_interpolator;
    //static Interpolator::TQuintOutInterpolator quintout_interpolator;
    //static Interpolator::TQuintInOutInterpolator quintinout_interpolator;
    //static Interpolator::TBackInInterpolator backin_interpolator;
    //static Interpolator::TBackOutInterpolator backout_interpolator;
    //static Interpolator::TBackInOutInterpolator backinout_interpolator;
    //static Interpolator::TElasticInInterpolator elasticin_interpolator;
    //static Interpolator::TElasticOutInterpolator elasticout_interpolator;
    //static Interpolator::TElasticInOutInterpolator elasticinout_interpolator;
    //static Interpolator::TBounceInInterpolator bouncein_interpolator;
    //static Interpolator::TBounceOutInterpolator bounceout_interpolator;
    //static Interpolator::TBounceInOutInterpolator bounceinout_interpolator;
    //static Interpolator::TCircularInInterpolator circularin_interpolator;
    //static Interpolator::TCircularOutInterpolator circularout_interpolator;
    //static Interpolator::TCircularInOutInterpolator circularinout_interpolator;
    //static Interpolator::TExpoInInterpolator expoin_interpolator;
    //static Interpolator::TExpoOutInterpolator expoout_interpolator;
    //static Interpolator::TExpoInOutInterpolator expoinout_interpolator;
    //static Interpolator::TSineInInterpolator sinein_interpolator;
    //static Interpolator::TSineOutInterpolator sineout_interpolator;
    //static Interpolator::TSineInOutInterpolator sineinout_interpolator;

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
    if (_currentCamera.isValid()) {
        return _currentCamera;
    }
    else {
        return _defaultCamera;
    }
}

CModuleCameras::EPriority CModuleCameras::getPriorityFromString(const std::string & priority)
{
    if (priority.compare("default") == 0) {
        return EPriority::DEFAULT;
    }
    else if (priority.compare("gameplay") == 0) {
        return EPriority::GAMEPLAY;
    }
    else if (priority.compare("temporary") == 0) {
        return EPriority::TEMPORARY;
    }
    else if (priority.compare("cinematic") == 0) {
        return EPriority::CINEMATIC;
    }
    else if (priority.compare("debug") == 0) {
        return EPriority::DEBUG;
    }
    return EPriority::NUM_PRIORITIES;
}

Interpolator::IInterpolator * CModuleCameras::getInterpolatorFromString(const std::string & interpolator)
{
    if (interpolator.compare("linear") == 0) {
        static Interpolator::TLinearInterpolator linear_interpolator;
        return &linear_interpolator;
    }
    else if (interpolator.compare("quadin") == 0) {
        static Interpolator::TQuadInInterpolator quadin_interpolator;
        return &quadin_interpolator;
    }
    else if (interpolator.compare("quadout") == 0) {
        static Interpolator::TQuadOutInterpolator quadout_interpolator;
        return &quadout_interpolator;
    }
    else if (interpolator.compare("quadinout") == 0) {
        static Interpolator::TQuadInOutInterpolator quadinout_interpolator;
        return &quadinout_interpolator;
    }
    else if (interpolator.compare("cubicin") == 0) {
        static Interpolator::TCubicInInterpolator cubicin_interpolator;
        return &cubicin_interpolator;
    }
    else if (interpolator.compare("cubicout") == 0) {
        static Interpolator::TCubicOutInterpolator cubicout_interpolator;
        return &cubicout_interpolator;
    }
    else if (interpolator.compare("cubicinout") == 0) {
        static Interpolator::TCubicInOutInterpolator cubicinout_interpolator;
        return &cubicinout_interpolator;
    }
    else if (interpolator.compare("quartin") == 0) {
        static Interpolator::TQuartInInterpolator quartin_interpolator;
        return &quartin_interpolator;
    }
    else if (interpolator.compare("quartout") == 0) {
        static Interpolator::TQuartInInterpolator quartout_interpolator;
        return &quartout_interpolator;
    }
    else if (interpolator.compare("quartinout") == 0) {
        static Interpolator::TQuartInInterpolator quartinout_interpolator;
        return &quartinout_interpolator;
    }
    else if (interpolator.compare("quintin") == 0) {
        static Interpolator::TQuintInInterpolator quintin_interpolator;
        return &quintin_interpolator;
    }
    else if (interpolator.compare("quintout") == 0) {
        static Interpolator::TQuintOutInterpolator quintout_interpolator;
        return &quintout_interpolator;
    }
    else if (interpolator.compare("quintinout") == 0) {
        static Interpolator::TQuintInOutInterpolator quintinout_interpolator;
        return &quintinout_interpolator;
    }
    else if (interpolator.compare("backin") == 0) {
        static Interpolator::TBackInInterpolator backin_interpolator;
        return &backin_interpolator;
    }
    else if (interpolator.compare("backout") == 0) {
        static Interpolator::TBackOutInterpolator backout_interpolator;
        return &backout_interpolator;
    }
    else if (interpolator.compare("backinout") == 0) {
        static Interpolator::TBackInOutInterpolator backinout_interpolator;
        return &backinout_interpolator;
    }
    else if (interpolator.compare("elasticin") == 0) {
        static Interpolator::TElasticInInterpolator elasticin_interpolator;
        return &elasticin_interpolator;
    }
    else if (interpolator.compare("elasticout") == 0) {
        static Interpolator::TElasticOutInterpolator elasticout_interpolator;
        return &elasticout_interpolator;
    }
    else if (interpolator.compare("elasticinout") == 0) {
        static Interpolator::TElasticInOutInterpolator elasticinout_interpolator;
        return &elasticinout_interpolator;
    }
    else if (interpolator.compare("bouncein") == 0) {
        static Interpolator::TBounceInInterpolator bouncein_interpolator;
        return &bouncein_interpolator;
    }
    else if (interpolator.compare("bounceout") == 0) {
        static Interpolator::TBounceOutInterpolator bounceout_interpolator;
        return &bounceout_interpolator;
    }
    else if (interpolator.compare("bounceinout") == 0) {
        static Interpolator::TBounceInOutInterpolator bounceinout_interpolator;
        return &bounceinout_interpolator;
    }
    else if (interpolator.compare("circularin") == 0) {
        static Interpolator::TCircularInInterpolator circularin_interpolator;
        return &circularin_interpolator;
    }
    else if (interpolator.compare("circularout") == 0) {
        static Interpolator::TCircularOutInterpolator circularout_interpolator;
        return &circularout_interpolator;
    }
    else if (interpolator.compare("circularinout") == 0) {
        static Interpolator::TCircularInOutInterpolator circularinout_interpolator;
        return &circularinout_interpolator;
    }
    else if (interpolator.compare("expoin") == 0) {
        static Interpolator::TExpoInInterpolator expoin_interpolator;
        return &expoin_interpolator;
    }
    else if (interpolator.compare("expoout") == 0) {
        static Interpolator::TExpoOutInterpolator expoout_interpolator;
        return &expoout_interpolator;
    }
    else if (interpolator.compare("expoinout") == 0) {
        static Interpolator::TExpoInOutInterpolator expoinout_interpolator;
        return &expoinout_interpolator;
    }
    else if (interpolator.compare("sinein") == 0) {
        static Interpolator::TSineInInterpolator sinein_interpolator;
        return &sinein_interpolator;
    }
    else if (interpolator.compare("sineout") == 0) {
        static Interpolator::TSineOutInterpolator sineout_interpolator;
        return &sineout_interpolator;
    }
    else if (interpolator.compare("sineinout") == 0) {
        static Interpolator::TSineInOutInterpolator sineinout_interpolator;
        return &sineinout_interpolator;
    }
    else {
        return nullptr;
    }
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
        if (mc.type == GAMEPLAY && mc.weight >= 1.f && mc.state != TMixedCamera::ST_CANCELLED)
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
