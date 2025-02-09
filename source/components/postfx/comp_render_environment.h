#ifndef INC_COMPONENT_RENDER_ENVIRONMENT_H_
#define INC_COMPONENT_RENDER_ENVIRONMENT_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompRenderEnvironment : public TCompBase {

    static CRenderToTexture*             rt;

    bool                          enabled = true;
    float                         amount = 1.f;
    const CRenderTechnique*       tech = nullptr;
    const CRenderTechnique*       post_tech = nullptr;
    const CRenderMesh*            mesh = nullptr;

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    CTexture* apply(CTexture* in_texture);
};

#endif
