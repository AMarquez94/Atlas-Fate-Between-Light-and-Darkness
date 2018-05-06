#include "mcv_platform.h"
#include "gui_text.h"

using namespace GUI;

void CText::render()
{
  float textWidth = _textParams._text.size() * _textParams._size;
  float textHeight = _textParams._size;
  VEC2 offset;
  if (_textParams._hAlign == TTextParams::Center)
    offset.x = _params._size.x * 0.5f - textWidth * 0.5f;
  else if (_textParams._hAlign == TTextParams::Right)
    offset.x = _params._size.x - textWidth;
  if (_textParams._vAlign == TTextParams::Center)
    offset.y = _params._size.y * 0.5f - textHeight * 0.5f;
  else if (_textParams._vAlign == TTextParams::Bottom)
    offset.y = _params._size.y - textHeight;

  MAT44 tr = MAT44::CreateTranslation(offset.x, offset.y, 0.f);
  MAT44 w = MAT44::CreateScale(_textParams._size) * tr * _absolute;
  Engine.get().getGUI().renderText(w, _textParams._text, _textParams._color);
}

TTextParams* CText::getTextParams()
{
  return &_textParams;
}
