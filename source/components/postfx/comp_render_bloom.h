#ifndef INC_COMPONENT_RENDER_BLOOM_H_
#define INC_COMPONENT_RENDER_BLOOM_H_

#include "components/comp_base.h"
#include "comp_render_blur.h"

struct CBlurStep;
class  CTexture;

// ------------------------------------
class TCompRenderBloom : public TCompRenderBlur {

	CRenderToTexture*       rt_output = nullptr;
	const CRenderTechnique* tech = nullptr;
	const CRenderMesh*      mesh = nullptr;

	int                     xres = 0;
	int                     yres = 0;

public:
	void load(const json& j, TEntityParseContext& ctx);
	void debugInMenu();
	CTexture* apply(CTexture* in_texture, CTexture* in_light_texture);

};

#endif
