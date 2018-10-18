#ifndef INC_COMPONENT_VIGNETTE_H_
#define INC_COMPONENT_VIGNETTE_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompVignette : public TCompBase {

    bool                          enabled = true;
    float                         amount = 0.95f;
    float                         softness_amount = 0.66f;
    float                         contrast = 1.0f;
    float                         brightness = 0.5f;
    float                         lut_amount = 0.6f;
    const CRenderTechnique*       tech = nullptr;
    const CRenderMesh*            mesh = nullptr;
    CRenderToTexture*             rt = nullptr;
    const CTexture*               lut1 = nullptr;

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    CTexture* apply(CTexture* in_texture);
};

#endif
