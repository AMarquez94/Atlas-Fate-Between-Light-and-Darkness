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
        int _index;
        float _timeToChange;
        float _timer;

        friend class CParser;
    };
}
