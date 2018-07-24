#ifndef INC_COMPONENT_RENDER_FLARES_H_
#define INC_COMPONENT_RENDER_FLARES_H_

#include "components/comp_base.h"
#include "comp_render_blur.h"
#include <vector>

class CRenderToTexture;
class CRenderTechnique;
class CRenderMesh;
class CTexture;

// ------------------------------------
class TCompRenderFlares : public TCompRenderBlur {

	CRenderToTexture*       rt_output = nullptr;
    CRenderToTexture*       rt_output2 = nullptr;
	const CRenderTechnique* tech = nullptr;
	const CRenderMesh*      mesh = nullptr;
	int                     xres = 0;
	int                     yres = 0;

public:

	void  load(const json& j, TEntityParseContext& ctx);
	void  debugInMenu();
	CTexture* apply(CTexture* in_color, CTexture * in_lights);

    bool const getState() { return enabled; }
    void setState(bool state) { enabled = state; }
};

#endif
