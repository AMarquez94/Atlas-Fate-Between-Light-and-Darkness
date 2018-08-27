#include "mcv_platform.h"
#include "gui/widgets/gui_image.h"

using namespace GUI;

void CImage::render() {

    MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
    Engine.get().getGUI().renderTexture(sz * _absolute,
        _imageParams._texture,
        _imageParams._minUV,
        _imageParams._maxUV,
        _imageParams._color);
}

TImageParams* CImage::getImageParams() {

    return &_imageParams;
}
