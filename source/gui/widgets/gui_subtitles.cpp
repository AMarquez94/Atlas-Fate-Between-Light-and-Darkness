#include "mcv_platform.h"
#include "gui/widgets/gui_subtitles.h"

using namespace GUI;

void CSubtitles::render() {

	MAT44 sz = MAT44::CreateScale(_params._size.x, _params._size.y, 1.f);

	Engine.get().getGUI().renderTexture(sz * _absolute,
		_subtitleParams._current_texture,
		_imageParams._minUV,
		_imageParams._maxUV,
		_imageParams._color);
}

TImageParams* CSubtitles::getImageParams() {

	return &_imageParams;
}

TSubtitlesParams* CSubtitles::getSubtitleParams() {

	return &_subtitleParams;
}

void CSubtitles::update(float dt) {

}

void CSubtitles::activateSubtitles(int subtitle_num) {
	const CTexture* text = _subtitleParams._map_to_textures[subtitle_num];
	if (text != nullptr) {
		_subtitleParams._current_subtitles = subtitle_num;
		_subtitleParams._current_texture = text;
	}
}