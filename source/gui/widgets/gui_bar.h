#pragma once

#include "gui/gui_widget.h"
#include "geometry/geometry.h"
namespace GUI
{
    class CBar : public CWidget
    {
    public:
        CBar() = default;
        void render() override;
        void update(float dt) override;
        TImageParams* getImageParams() override;
        TBarParams* CBar::getBarParams() override;

    private:
        TImageParams _imageParams;
        TBarParams _barParams;

        friend class CParser;
    };
}