#include "mcv_platform.h"
#include "gui_change_textures.h"

namespace GUI
{
    void CChangeTexture::update(float delta)
    {
        _timer += delta;
        if (_timer > _timeToChange) {
            _timer = 0.f;
            _index = (_index + 1) % textures.size();
            TImageParams* params = _widget->getImageParams();
            if (params)
            {
                params->_texture = textures[_index];
            }
        }
    }
}
