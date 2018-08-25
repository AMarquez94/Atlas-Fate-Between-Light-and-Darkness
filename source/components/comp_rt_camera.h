#ifndef INC_COMP_RENDER_RT_CAMERA_H_
#define INC_COMP_RENDER_RT_CAMERA_H_

#include "comp_base.h"

class CDeferredRenderer;

struct TCompRTCamera : public TCompBase {

    CRenderToTexture*       rt = nullptr;

    std::string      name;
    VEC2             resolution;
    DXGI_FORMAT      color_fmt = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT      depth_fmt = DXGI_FORMAT_UNKNOWN;
    std::vector<std::string> custom_categories;

    TCompRTCamera();
    void load(const json& j, TEntityParseContext& ctx);
    void generate(CDeferredRenderer& renderer);

protected:
    void init();
    bool isValid() const { return rt != nullptr; }
};

#endif
