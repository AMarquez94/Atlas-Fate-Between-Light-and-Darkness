#pragma once

#include "gui/gui_effect.h"

namespace GUI
{
  class CAnimateUV : public CEffect
  {
  public:
    virtual void update(float delta) override;

  private:
    VEC2 _speed;

    friend class CParser;
  };
}
