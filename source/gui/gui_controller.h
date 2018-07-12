#pragma once

#include "mcv_platform.h"

namespace GUI
{
  class CController
  {
  public:

    virtual void update(float delta) { (void)delta; }
  };

  using VControllers = std::vector<CController*>;
}