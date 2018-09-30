#pragma once

#include "gui/gui_effect.h"

//class CTexture;

namespace GUI
{
    class CChangeTexture : public CEffect
    {
    public:
        virtual void update(float delta) override;

    private:
        std::vector<const CTexture*> textures;
        std::vector<float> timers;
        int _index;
        float _timer;

        friend class CParser;
    };
}
