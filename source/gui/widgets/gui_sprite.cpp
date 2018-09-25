#include "mcv_platform.h"
#include "gui/widgets/gui_sprite.h"

using namespace GUI;


void CSprite::initializeSprite() {
	_imageParams._minUV = VEC2(0,0);
	_imageParams._maxUV = _spriteParams._frame_size/_params._size;
	
}

void CSprite::render() {

	MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);
	Engine.get().getGUI().renderTexture(sz * _absolute,
		_imageParams._texture,
		_imageParams._minUV,
		_imageParams._maxUV,
		_imageParams._color);
}

TImageParams* CSprite::getImageParams() {

	return &_imageParams;
}

TSpriteParams* CSprite::getSpriteParams() {

	return &_spriteParams;
}

void CSprite::update(float dt) {
	_time_since_start += dt;

	if (_time_since_start >= (1.f / (float)_spriteParams._frames_per_second)) {

		_actual_horizontal_frame++;

		VEC2 _aux_minUV;
		_aux_minUV.x = (_spriteParams._frame_size.x) / (_params._size.x) * _actual_horizontal_frame;
		_aux_minUV.y = (_spriteParams._frame_size.y) / (_params._size.y) * _actual_vertical_frame;

		VEC2 _aux_maxUV;
		_aux_maxUV.x = (_spriteParams._frame_size.x) / (_params._size.x) * (_actual_horizontal_frame + 1);
		_aux_maxUV.y = (_spriteParams._frame_size.y) / (_params._size.y) * (_actual_vertical_frame + 1);

		if (_aux_maxUV.x > 1.0f) {
			_actual_horizontal_frame = 0;
			_actual_vertical_frame++;

			_aux_minUV;
			_aux_minUV.x = (_spriteParams._frame_size.x) / (_params._size.x) * _actual_horizontal_frame;
			_aux_minUV.y = (_spriteParams._frame_size.y) / (_params._size.y) * _actual_vertical_frame;

			_aux_maxUV;
			_aux_maxUV.x = (_spriteParams._frame_size.x) / (_params._size.x) * (_actual_horizontal_frame + 1);
			_aux_maxUV.y = (_spriteParams._frame_size.y) / (_params._size.y) * (_actual_vertical_frame + 1);
		}

		if (_aux_maxUV.y > 1.0f) {
			_actual_vertical_frame = 0;

			_aux_minUV;
			_aux_minUV.x = (_spriteParams._frame_size.x) / (_params._size.x) * _actual_horizontal_frame;
			_aux_minUV.y = (_spriteParams._frame_size.y) / (_params._size.y) * _actual_vertical_frame;

			_aux_maxUV;
			_aux_maxUV.x = (_spriteParams._frame_size.x) / (_params._size.x) * (_actual_horizontal_frame + 1);
			_aux_maxUV.y = (_spriteParams._frame_size.y) / (_params._size.y) * (_actual_vertical_frame + 1);
		}


		_imageParams._minUV = _aux_minUV;
		_imageParams._maxUV = _aux_maxUV;

		_time_since_start = 0.0f;
	}
}