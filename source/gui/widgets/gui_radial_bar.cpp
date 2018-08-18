#include "mcv_platform.h"
#include "gui_radial_bar.h"

using namespace GUI;

void CRadialBar::render()
{
    //float ratio = _barParams._processValue;
    float ratio = Engine.getGUI().getVariables().getFloat(_barParams._variable);
    ratio = clamp(ratio, 0.f, 1.f);
    MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
    MAT44 w = sz * _absolute;

    ConfigParams c_params = ConfigParams();
    c_params.color = _imageParams._color;
    c_params.minUV = _imageParams._minUV;
    c_params.maxUV = _imageParams._maxUV;
    c_params.var = ratio;

    Engine.get().getGUI().renderCustomTexture("gui_radial.tech", w, _imageParams._texture, c_params );
}

TImageParams* CRadialBar::getImageParams()
{
    return &_imageParams;
}

TBarParams* CRadialBar::getBarParams()
{
    return &_barParams;
}

void CRadialBar::update(float dt)
{

}
