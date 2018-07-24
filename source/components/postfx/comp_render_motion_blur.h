#ifndef INC_COMPONENT_RENDER_MOTION_BLUR_H_
#define INC_COMPONENT_RENDER_MOTION_BLUR_H_

#include <vector>
#include "components/comp_base.h"
#include "render/render_objects.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompRenderMotionBlur : public TCompBase {

    bool                    enabled;
    CRenderToTexture*       rt = nullptr;
    const CRenderTechnique* tech = nullptr;
    const CRenderMesh*      mesh = nullptr;

    TCompRenderMotionBlur();
    ~TCompRenderMotionBlur();

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    CTexture* apply(CTexture* in_texture);
};

#endif
