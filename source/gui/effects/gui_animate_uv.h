#pragma once

#include "gui/gui_effect.h"

namespace GUI
{
  class CAnimateUV : public CEffect
  {
  public:
    virtual void update(float delta) override;
    virtual void onDeactivate() override;

  private:
    VEC2 _speed;

    friend class CParser;
  };
}
