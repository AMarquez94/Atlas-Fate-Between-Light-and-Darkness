#ifndef INC_COMPONENT_RENDER_FOCUS_H_
#define INC_COMPONENT_RENDER_FOCUS_H_

#include <vector>
#include "components/comp_base.h"
#include "render/render_objects.h"
#include "ctes.h"

class CTexture;
class CRenderToTexture;

// ------------------------------------
struct TCompRenderFocus : public TCompBase {

    bool                    enabled;
    CRenderCte< CCteFocus > cte_focus;
    CRenderToTexture*       rt = nullptr;
    const CRenderTechnique* tech = nullptr;
    const CRenderMesh*      mesh = nullptr;

    TCompRenderFocus();
    ~TCompRenderFocus();

    void load(const json& j, TEntityParseContext& ctx);
    void debugInMenu();
    CTexture* apply(CTexture* focus_texture, CTexture* blur_texture);
};

#endif
