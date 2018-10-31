#include "mcv_platform.h"
#include "gui_video.h"
#include "render/video/video_texture.h"

using namespace GUI;

void CUIVideo::render()
{
    MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
    MAT44 w = sz * _absolute;

    ConfigParams c_params = ConfigParams();
    c_params.color = _imageParams._color;
	if (c_params.color.x != 1.0f || c_params.color.y != 1.0f | c_params.color.z != 1.0f) c_params.color = VEC4(1,1,1,1);
    c_params.minUV = VEC2(0,0);
    c_params.maxUV = VEC2(1, 1);
    c_params.var = 1;

    if (_technique != nullptr)
        Engine.get().getGUI().renderCustomTexture(_technique->getName(), w, _imageParams._texture, c_params);
    else
        Engine.get().getGUI().renderCustomTexture("gui.tech", w, _imageParams._texture, c_params );
}

TImageParams* CUIVideo::getImageParams()
{
    return &_imageParams;
}

void CUIVideo::update(float dt)
{

}

void CUIVideo::onActivate() {

    ((CVideoTexture*)(_imageParams._texture))->setActive(true);
}

void CUIVideo::onDeactivate() {

    ((CVideoTexture*)(_imageParams._texture))->setActive(false);
    ((CVideoTexture*)(_imageParams._texture))->setToBeReset(true);
}
