#pragma once

#include "gui/gui_widget.h"
#include "geometry/geometry.h"
namespace GUI
{
    class CRadialBar : public CWidget
    {
    public:
        CRadialBar() = default;
        void render() override;
        void update(float dt) override;
        TImageParams* getImageParams() override;
        TBarParams* CRadialBar::getBarParams() override;

    private:
        TImageParams _imageParams;
        TBarParams _barParams;

        friend class CParser;
    };
}