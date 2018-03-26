#include "mcv_platform.h"
#include "comp_curve_controller.h"
#include "components/comp_transform.h"
#include "geometry/curve.h"

DECL_OBJ_MANAGER("curve_controller", TCompCurve);

void TCompCurve::debugInMenu() {
  //ImGui::DragFloat("Sensitivity", &_sensitivity, 0.01f, 0.001f, 0.1f);
}

void TCompCurve::load(const json& j, TEntityParseContext& ctx) {
  /*_sensitivity = j.value("sensitivity", _sensitivity);*/

  std::string curve_name = j["curve"];
  _curve = Resources.get(curve_name)->as<CCurve>();

  _speed = j.value<float>("speed", 0.f);

  _targetName = j["target"];
}

void TCompCurve::update(float dt)
{
  if (!_curve)
    return;

  // actualizar ratio
  if (_automove)
  {
    _ratio += _speed * dt;
    if (_ratio >= 1.f)
      _ratio = 0.f;
  }

  // evaluar curva con dicho ratio
  VEC3 pos = _curve->evaluate(_ratio);

  // obtener la posicion del target
  VEC3 targetPos = getTargetPos();

  // actualizar la transform con la nueva posicion
  TCompTransform* c_transform = get<TCompTransform>();
  c_transform->lookAt(pos, targetPos);
}

VEC3 TCompCurve::getTargetPos()
{
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
