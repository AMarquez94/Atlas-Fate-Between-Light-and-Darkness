#ifndef INC_RENDER_UTILS_H_
#define INC_RENDER_UTILS_H_

void renderLine(VEC3 src, VEC3 dst, VEC4 color);

// --------------------------------------------
enum eSamplerType {
	SAMPLER_WRAP_LINEAR = 0
	/*
	, SAMPLER_CLAMP_LINEAR
	, SAMPLER_BORDER_LINEAR
	, SAMPLER_PCF_SHADOWS
	, SAMPLER_CLAMP_BILINEAR
	, SAMPLER_CLAMP_POINT
	*/
	, SAMPLERS_COUNT
};
void activateSampler(int slot, eSamplerType cfg);

// --------------------------------------------
bool createRenderUtils();
void destroyRenderUtils();
void activateDefaultRenderState();

#endif

