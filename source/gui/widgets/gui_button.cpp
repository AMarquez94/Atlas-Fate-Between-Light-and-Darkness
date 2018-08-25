#include "mcv_platform.h"
#include "gui_button.h"

using namespace GUI;

void CButton::render()
{
  CModuleGUI& gui = Engine.get().getGUI();

  TButtonParams& btParams = _states[_currentState];
  //btParams._imageParams._color.w = 0.3f;

  // render image
  MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
  Engine.get().getGUI().renderTexture(sz * _absolute,
    btParams._imageParams._texture,
    btParams._imageParams._minUV,
    btParams._imageParams._maxUV,
    btParams._imageParams._color);

  // render text
  float textWidth = btParams._textParams._text.size() * btParams._textParams._size;
  float textHeight = btParams._textParams._size;
  VEC2 offset;
  if (btParams._textParams._hAlign == TTextParams::Center)
    offset.x = _params._size.x * 0.5f - textWidth * 0.5f;
  else if (btParams._textParams._hAlign == TTextParams::Right)
    offset.x = _params._size.x - textWidth;
  if (btParams._textParams._vAlign == TTextParams::Center)
    offset.y = _params._size.y * 0.5f - textHeight * 0.5f;
  else if (btParams._textParams._vAlign == TTextParams::Bottom)
    offset.y = _params._size.y - textHeight;

  MAT44 tr = MAT44::CreateTranslation(offset.x, offset.y, 0.f);
  MAT44 w = MAT44::CreateScale(btParams._textParams._size) * tr * _absolute;
  gui.renderText(w, btParams._textParams._text, btParams._textParams._color);
}

TImageParams* CButton::getImageParams()
{
  return &_states[_currentState]._imageParams;
}

TTextParams* CButton::getTextParams()
{
  return &_states[_currentState]._textParams;
}

TParams * CButton::getTParams()
{
    return &_states[_currentState]._params;
}

void CButton::setCurrentState(EState newState)
{
  _currentState = newState;
}

CButton::EState GUI::CButton::getCurrentState()
{
    return _currentState;
}

