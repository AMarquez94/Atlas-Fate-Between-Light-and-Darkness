#include "mcv_platform.h"
#include "comp_curve_controller.h"
#include "components/comp_transform.h"
#include "geometry/curve.h"
#include "entity/common_msgs.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("curve_controller", TCompCurve);

void TCompCurve::debugInMenu() {

    ImGui::DragInt("Curve Smoothness", &_dbg_smooth, 1, 1, 500);
    ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
    ImGui::DragFloat("Ratio", &_ratio, 0.01f, 0.f, 1.f);

    for (int i = 0; i < _dbg_smooth; ++i) {

        VEC3 pos = _curve->evaluateAsCatmull((float)i / (float)_dbg_smooth);
        VEC3 pos2 = _curve->evaluateAsCatmull((float)(i + 1) / (float)_dbg_smooth);
        renderLine(pos, pos2, VEC4(1.f, 1.f, 1.f, 1.f));
    }

    if (ImGui::CollapsingHeader("Pos Historic")) {
        for (int i = 0; i < historicPos.size(); i++) {
            ImGui::Text("Pos: (%f, %f, %f) - %f", historicPos[i].x, historicPos[i].y, historicPos[i].z, historicRatio[i]);
        }
    }
}

void TCompCurve::load(const json& j, TEntityParseContext& ctx) {

    std::string curve_name = j["curve"];
    _curve = Resources.get(curve_name)->as<CCurve>();
    _speed = j.value<float>("speed", 0.f);
    _targetName = j.value("target", "");
    _loop = j.value("loop", false);

    _dbg_smooth = j.value("debug_smooth", 100);
    _sensitivity = j.value("sensitivity", 1);
}

void TCompCurve::registerMsgs() {

	DECL_MSG(TCompCurve, TMsgCameraActivated, onMsgCameraActive);
	DECL_MSG(TCompCurve, TMsgCameraFullyActivated, onMsgCameraFullActive);
	DECL_MSG(TCompCurve, TMsgCameraDeprecated, onMsgCameraDeprecated);
}

void TCompCurve::update(float dt) {

    if (!_curve || !_active)
        return;

    _ratio = Clamp(_ratio + _speed * dt, 0.f, 1.f);
    if (_loop && _ratio >= 1.f)
        _ratio = 0.f;

    // evaluar curva con dicho ratio
    VEC3 pos = _curve->evaluate(_ratio);
    if (_ratio != 1.f) {
        historicPos.push_back(pos);
        historicRatio.push_back(_ratio);
    }

    // obtener la posicion del target
    VEC3 targetPos = getTargetPos();

    // actualizar la transform con la nueva posicion
    TCompTransform* c_transform = get<TCompTransform>();
    c_transform->lookAt(pos, targetPos);

    if (_ratio >= 1.f && !_finished) {
        // TODO: Script camara finalizada
        _finished = true;
    }
}

VEC3 TCompCurve::getTargetPos() {

    if (!_target.isValid())
        _target = getEntityByName(_targetName);
 
    if (_target.isValid())
    {
        CEntity* e_target = _target;
        TCompTransform* c_transform = e_target->get<TCompTransform>();
        return c_transform->getPosition();
    }
    return VEC3::Zero;
}

void TCompCurve::onMsgCameraActive(const TMsgCameraActivated & msg) {

	//VEC3 pos = Engine.getCameras().getResultPos();
	//_curve->addKnotAtIndex(pos, 1);
}

void TCompCurve::onMsgCameraFullActive(const TMsgCameraFullyActivated & msg) {

	_active = true;
}

void TCompCurve::onMsgCameraDeprecated(const TMsgCameraDeprecated & msg) {

	_active = false;
	_ratio = 0.f;
}
