#ifndef INC_COMP_RENDER_CUBE_H_
#define INC_COMP_RENDER_CUBE_H_

#include "comp_base.h"

class CRenderToCube;
class CDeferredRenderer;

struct TCompRenderCube : public TCompBase {

    CRenderToCube*   rt = nullptr;
    int              resolution = 0;
    DXGI_FORMAT      color_fmt = DXGI_FORMAT_UNKNOWN;
    DXGI_FORMAT      depth_fmt = DXGI_FORMAT_UNKNOWN;
    std::string      name;
    TCompRenderCube();
    void load(const json& j, TEntityParseContext& ctx);
    void generate(CDeferredRenderer& renderer);

protected:
    void init();
    bool isValid() const { return rt != nullptr; }
};

#endif
