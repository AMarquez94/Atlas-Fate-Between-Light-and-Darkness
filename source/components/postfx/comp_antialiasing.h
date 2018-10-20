#ifndef INC_COMPONENT_VOLUME_LIGHT_H_
#define INC_COMPONENT_VOLUME_LIGHT_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompAntiAliasing : public TCompBase {

    static CRenderToTexture*             rt;

    bool                          enabled = true;
    float                         amount = 1.f;
    const CRenderTechnique*       tech = nullptr;
    const CRenderMesh*            mesh = nullptr;

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    CTexture* apply(CTexture* in_texture);
};

#endif
