#pragma once

#include "gui/gui_widget.h"
#include "geometry/geometry.h"
namespace GUI
{
	class CMiddleBar : public CWidget
	{
	public:
		CMiddleBar() = default;
		void render() override;
		void update(float dt) override;
		TImageParams* getImageParams() override;
		TBarParams* CMiddleBar::getBarParams() override;

	private:
		TImageParams _imageParams;
		TBarParams _barParams;

		friend class CParser;
	};
}