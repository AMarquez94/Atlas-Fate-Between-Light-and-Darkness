#ifndef INC_COMPONENT_RENDER_BLOOM_H_
#define INC_COMPONENT_RENDER_BLOOM_H_

#include "comp_render_blur.h"
#include "render/cte_buffer.h"
#include "ctes.h"

// ------------------------------------
struct TCompRenderBloom : public TCompRenderBlur {

    CRenderCte< CCteBloom >       cte_bloom;
    CRenderToTexture*             rt_highlights = nullptr;
    const CRenderTechnique*       tech_filter = nullptr;
    const CRenderTechnique*       tech_add = nullptr;
    const CRenderMesh*            mesh = nullptr;
    VEC4                          add_weights;
    float                         threshold_min = 0.890;
    float                         threshold_max = 0.670f;
    float                         multiplier = 1.110f;
    TCompRenderBlur * renderblur;

    TCompRenderBloom();
    ~TCompRenderBloom();

    void generateHighlights(CTexture* in_texture);
    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    void addBloom();

};

#endif
