#pragma once

#include "gui/widgets/gui_image.h"
#include "gui/gui_widget.h"


namespace GUI {

	class CSubtitles : public CWidget {

	public:
		CSubtitles() = default;
		void render() override;
		void update(float dt) override;
		TImageParams* getImageParams() override;
		TSubtitlesParams* getSubtitleParams() override;

	private:
		TImageParams _imageParams;
		TSubtitlesParams _subtitleParams;
		friend class CParser;

	};
}