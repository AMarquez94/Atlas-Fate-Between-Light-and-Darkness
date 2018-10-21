#include "mcv_platform.h"
#include "gui_middle_bar.h"

using namespace GUI;

void CMiddleBar::render() {

	//float ratio = _barParams._processValue;
	float ratio = _barParams._ratio;

	VEC2 offset;
	offset.x = _params._size.x * 0.5f * ratio;
	offset.y = _params._size.y * 0.5f;


	MAT44 tr = MAT44::CreateTranslation(offset.x, offset.y, 0.f);

	ratio = clamp(ratio, 0.f, 1.f);
	MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
	MAT44 w = MAT44::CreateScale(ratio, 1.f, 1.f) * sz * _absolute;
	VEC2 maxUV = _imageParams._maxUV;
	maxUV.x *= ratio;
	Engine.get().getGUI().renderTexture(w,
		_imageParams._texture,
		_imageParams._minUV, maxUV,
		_imageParams._color);
}

TImageParams* CMiddleBar::getImageParams() {

	return &_imageParams;
}

TBarParams* CMiddleBar::getBarParams() {

	return &_barParams;
}

void CMiddleBar::update(float dt) {

}
