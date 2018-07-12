#pragma once

#include "gui/gui_controller.h"

namespace GUI
{
  class CButton;

  class CMenuButtonsController : public CController
  {
  public:
    virtual void update(float delta) override;
    void registerOption(const std::string& name, GUICallback cb);
    void setCurrentOption(int newOption);

  private:
    struct TOption
    {
      CButton* button;
      GUICallback callback;
    };
    std::vector<TOption> _options;
    int _currentOption = 0;
  };
}
