#include "mcv_platform.h"
#include "gui_video.h"

using namespace GUI;

void CUIVideo::render()
{
    MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
    MAT44 w = sz * _absolute;

    ConfigParams c_params = ConfigParams();
    c_params.color = VEC4(1,1,1,1);
    c_params.minUV = VEC2(0,0);
    c_params.maxUV = VEC2(1, 1);
    c_params.var = 1;

    Engine.get().getGUI().renderCustomTexture("gui.tech", w, _imageParams._texture, c_params );
}

TImageParams* CUIVideo::getImageParams()
{
    return &_imageParams;
}

void CUIVideo::update(float dt)
{

}
