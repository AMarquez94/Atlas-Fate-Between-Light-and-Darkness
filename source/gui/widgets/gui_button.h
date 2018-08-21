#pragma once

#include "gui/gui_widget.h"

namespace GUI
{
  class CButton : public CWidget
  {
  public:
    enum EState { ST_Idle = 0, ST_Selected, ST_Pressed, NUM_STATES };
    
    CButton() = default;
    void render() override;
    TImageParams* getImageParams() override;
    TTextParams* getTextParams() override;
    TParams* getTParams() override;
    void setCurrentState(EState newState);
    EState getCurrentState();

  private:
    TButtonParams _states[NUM_STATES];
    EState _currentState = ST_Idle;

    friend class CParser;
  };
}