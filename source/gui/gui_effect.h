#pragma once

#include "mcv_platform.h"

namespace GUI
{
  class CWidget;

  class CEffect
  {
  public:
    void setWidget(CWidget* wdgt) { _widget = wdgt; }
    virtual void update(float delta) { (void)delta; }

  protected:
    CWidget * _widget = nullptr;
  };

  using VEffects = std::vector<CEffect*>;
}