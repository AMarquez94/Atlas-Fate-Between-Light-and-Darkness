#include "mcv_platform.h"
#include "gui/gui_parser.h"
#include "gui/widgets/gui_image.h"
#include "gui/widgets/gui_text.h"
#include "gui/widgets/gui_bar.h"
#include "gui/widgets/gui_button.h"
#include "gui/effects/gui_animate_uv.h"

namespace
{
  json mergeJson(const json& j1, const std::string& key)
  {
    json newData = j1;
    if (j1.count(key) > 0)
    {
      const json& j2 = j1[key];
      for (const auto &j : json::iterator_wrapper(j2))
      {
        newData[j.key()] = j.value();
      }
    }
    return newData;
  }
}

using namespace GUI;

std::string CParser::parseFile(const std::string& filename)
{
  std::ifstream file_json(filename);
  json json_data;
  file_json >> json_data;
  bool first_wdgt = true;
  std::string main_name;

  for (auto& j_element : json_data)
  {
    CWidget* wdgt = parseWidget(j_element, nullptr);

    // computeAbsolute will propagate calculations to the children recursively
    wdgt->computeAbsolute();

    // register the widget within the manager
    Engine.getGUI().registerWidget(wdgt);
	if (first_wdgt) {
		main_name = wdgt->getName();
		first_wdgt = false;
	}
  }

  return main_name;
}

CWidget* CParser::parseWidget(const json& data, CWidget* parent)
{
  const std::string name = data.value("name", "");
  const std::string type = data.value("type", "widget");
  CWidget* wdgt = nullptr;

  // create and parse the widget
  if (type == "image")        wdgt = parseImage(data);
  else if (type == "text")    wdgt = parseText(data);
  else if (type == "bar")     wdgt = parseBar(data);
  else if (type == "button")  wdgt = parseButton(data);
  else                        wdgt = parseWidget(data);

  wdgt->_name = name;

  // create and parse effects
  if (data.count("effects") > 0)
  {
    for (auto& fxData : data["effects"])
    {
      parseEffect(fxData, wdgt);
    }
  }

  // add to parent
  if (parent)
  {
    parent->addChild(wdgt);
  }

  // create and parse children
  if (data.count("children") > 0)
  {
    for (auto& child : data["children"])
    {
      parseWidget(child, wdgt);
    }
  }

  return wdgt;
}

// ------------- WIDGETS
CWidget* CParser::parseWidget(const json& data) {
  CWidget* wdgt = new CWidget();

  parseParams(wdgt->_params, data);

  return wdgt;
}

CWidget* CParser::parseImage(const json& data) {
  CImage* wdgt = new CImage();

  parseParams(wdgt->_params, data);
  parseImageParams(wdgt->_imageParams, data);

  return wdgt;
}

CWidget* CParser::parseText(const json& data) {
  CText* wdgt = new CText();

  parseParams(wdgt->_params, data);
  parseTextParams(wdgt->_textParams, data);

  return wdgt;
}

CWidget* CParser::parseButton(const json& data) {
  CButton* wdgt = new CButton();

  parseParams(wdgt->_params, data);
  parseParams(wdgt->_states[CButton::EState::ST_Idle]._params, data);
  parseImageParams(wdgt->_states[CButton::EState::ST_Idle]._imageParams, data);
  parseTextParams(wdgt->_states[CButton::EState::ST_Idle]._textParams, data);

  json jSelected = mergeJson(data, "selected");
  parseParams(wdgt->_states[CButton::EState::ST_Selected]._params, jSelected);
  parseImageParams(wdgt->_states[CButton::EState::ST_Selected]._imageParams, jSelected);
  parseTextParams(wdgt->_states[CButton::EState::ST_Selected]._textParams, jSelected);

  json jPressed = mergeJson(data, "pressed");
  parseParams(wdgt->_states[CButton::EState::ST_Pressed]._params, jPressed);
  parseImageParams(wdgt->_states[CButton::EState::ST_Pressed]._imageParams, jPressed);
  parseTextParams(wdgt->_states[CButton::EState::ST_Pressed]._textParams, jPressed);

  return wdgt;
}

CEffect* CParser::parseEffect(const json& data, CWidget* wdgt)
{
  const std::string type = data.value("type", "");
  CEffect* fx = nullptr;

  // create and parse the widget
  if (type == "animate_uv")   fx = parseAnimateUVEffect(data);

  // add to parent
  if (wdgt && fx)
  {
    wdgt->addEffect(fx);
  }

  return fx;
}

CEffect* CParser::parseAnimateUVEffect(const json& data)
{
  CAnimateUV* fx = new CAnimateUV();

  fx->_speed = loadVEC2(data.value("speed", "0 0"));

  return fx;
}

CWidget* CParser::parseBar(const json& data) {
  CBar* wdgt = new CBar();

  parseParams(wdgt->_params, data);
  parseImageParams(wdgt->_imageParams, data);
  parseBarParams(wdgt->_barParams, data);

  return wdgt;
}

void CParser::parseParams(TParams& params, const json& data)
{
  params._visible = data.value("visible", true);
  params._position = loadVEC2(data.value("position","0 0"));
  params._rotation = deg2rad(data.value("rotation", 0.f));
  params._scale = loadVEC2(data.value("scale", "1 1"));
  params._size = loadVEC2(data.value("size", "1 1"));
}

void CParser::parseImageParams(TImageParams& params, const json& data)
{
  params._color = loadVEC4(data.value("color", "1 1 1 1"));
  params._minUV = loadVEC2(data.value("minUV", "0 0"));
  params._maxUV = loadVEC2(data.value("maxUV", "1 1"));
  const std::string textureFile = data.value("texture", "");
  params._texture = Resources.get(textureFile)->as<CTexture>();
}

void CParser::parseTextParams(TTextParams& params, const json& data)
{
  params._color = loadVEC4(data.value("font_color", "1 1 1 1"));
  params._size = data.value("font_size", 1.f);
  params._text = data.value("text", "");
  const std::string& hAlign = data.value("halign", "");
  if (hAlign == "center")      params._hAlign = TTextParams::Center;
  else if (hAlign == "right")  params._hAlign = TTextParams::Right;
  else                         params._hAlign = TTextParams::Left;
  const std::string& vAlign = data.value("valign", "");
  if (vAlign == "center")      params._vAlign = TTextParams::Center;
  else if (vAlign == "bottom") params._vAlign = TTextParams::Bottom;
  else                         params._vAlign = TTextParams::Top;
}

void CParser::parseBarParams(TBarParams& params, const json& data)
{
  params._variable = data.value("variable", "");
  params._processValue = data.value("progress_bar", 1.0f);
  const std::string direction = data.value("direction", "horizontal");
  params._direction = direction == "vertical" ? TBarParams::Vertical : TBarParams::Horizontal;
}
