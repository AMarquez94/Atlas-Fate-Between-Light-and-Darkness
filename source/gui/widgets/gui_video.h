#pragma once

#include "gui/gui_widget.h"
#include "geometry/geometry.h"
namespace GUI
{
    class CUIVideo : public CWidget
    {
    public:
        CUIVideo() = default;
        void render() override;
        void update(float dt) override;
        TImageParams* getImageParams() override;

    private:
        TImageParams _imageParams;

        friend class CParser;
    };
}