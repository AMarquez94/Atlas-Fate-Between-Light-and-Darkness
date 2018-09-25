#pragma once

#include "gui/widgets/gui_image.h"
#include "gui/gui_widget.h"


namespace GUI {

	class CSprite : public CWidget {

	public:
		CSprite() = default;
		void initializeSprite();
		void render() override;
		void update(float dt) override;
		TImageParams* getImageParams() override;
		TSpriteParams* getSpriteParams() override;

	private:
		TImageParams _imageParams;
		TSpriteParams _spriteParams;
		float _time_since_start = 0.f;
		int _actual_horizontal_frame = 0;
		int _actual_vertical_frame = 0;
		int _actual_frame = 1;
		friend class CParser;
		
	};
}