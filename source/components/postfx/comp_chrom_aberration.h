#ifndef INC_COMPONENT_CHROMATIC_ABERRATION_H_
#define INC_COMPONENT_CHROMATIC_ABERRATION_H_

#include "components/comp_base.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompChromaticAberration : public TCompBase {
    bool                          enabled = true;
    float                         amount = 1.f;
    float                         block_amount;
    float                         scanline_drift;
    float                         scanline_jitter;
    const CRenderTechnique*       tech = nullptr;
    const CRenderMesh*            mesh = nullptr;
    CRenderToTexture*             rt = nullptr;

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);

    CTexture* apply(CTexture* in_texture);
};

#endif
