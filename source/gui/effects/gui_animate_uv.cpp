#include "mcv_platform.h"
#include "gui_animate_uv.h"

namespace GUI
{
  void CAnimateUV::update(float delta)
  {
    TImageParams* params = _widget->getImageParams();
    if (params)
    {
      params->_minUV += _speed * delta;
      params->_maxUV += _speed * delta;
    }
  }
  void CAnimateUV::onDeactivate()
  {
      TImageParams* params = _widget->getImageParams();
      if (params)
      {
          params->_minUV = VEC2(0.f, 0.f);
          params->_maxUV = VEC2(1.f, 1.f);
      }
  }
}
