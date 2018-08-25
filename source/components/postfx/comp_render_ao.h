#ifndef INC_COMPONENT_RENDER_AO_H_
#define INC_COMPONENT_RENDER_AO_H_

#include "components/comp_base.h"
#include "comp_render_blur.h"
#include <vector>

class CRenderToTexture;
class CRenderTechnique;
class CRenderMesh;
class CTexture;

// ------------------------------------
class TCompRenderAO : public TCompRenderBlur {

	CRenderToTexture*       rt_output = nullptr;
	const CTexture*         white = nullptr;
	const CRenderTechnique* tech = nullptr;
	const CRenderMesh*      mesh = nullptr;
	float                   amount = 1.f;
	float                   radius = 0.110f;
	float                   zrange_discard = 0.100f;
	float                   amount_spreading = 0.960f;
	int                     xres = 0;
	int                     yres = 0;

public:

	void  load(const json& j, TEntityParseContext& ctx);
	void  debugInMenu();
	const CTexture* compute(CTexture* linear_depth_texture);

    bool const getState() { return enabled; }
    void setState(bool state) { enabled = state; }
};

#endif
