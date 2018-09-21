#ifndef INC_RENDER_UTILS_H_
#define INC_RENDER_UTILS_H_

DXGI_FORMAT readFormat(const json& j, const std::string& label);

// ---------------------------------------
enum ZConfig {
	ZCFG_DEFAULT
	, ZCFG_DISABLE_ALL
	, ZCFG_TEST_BUT_NO_WRITE
    , ZCFG_TEST_RAYMARCH
	, ZCFG_TEST_EQUAL
	, ZCFG_INVERSE_TEST_NO_WRITE
    , ZCFG_TEST_NO_WRITES_MARK_STENCIL
    , ZCFG_ON_NON_ZERO_STENCIL
	, ZCFG_COUNT
};
void activateZConfig(ZConfig cfg, UINT color_mask = 255);

// ---------------------------------------
enum RSConfig {
	RSCFG_DEFAULT
	, RSCFG_REVERSE_CULLING
	, RSCFG_CULL_NONE
	, RSCFG_SHADOWS
	, RSCFG_WIREFRAME
    , RSCFG_DEPTH_BIAS
	, RSCFG_COUNT
};
void activateRSConfig(RSConfig cfg);

// ---------------------------------------
enum BlendConfig {
	BLEND_CFG_DEFAULT
	, BLEND_CFG_ADDITIVE
	, BLEND_CFG_ADDITIVE_BY_SRC_ALPHA
	, BLEND_CFG_COMBINATIVE
    , BLEND_CFG_COMBINATIVE_GBUFFER
    , BLEND_CFG_MULTIPLICATIVE_GBUFFER
	, BLEND_CFG_COUNT
};
void activateBlendConfig(BlendConfig cfg);

// ---------------------------------------
// Read from name
ZConfig ZConfigFromString(const std::string& aname);
RSConfig RSConfigFromString(const std::string& aname);
BlendConfig BlendConfigFromString(const std::string& aname);

// ---------------------------------------
// Edit in ImGui
bool renderInMenu(ZConfig& cfg);
bool renderInMenu(RSConfig& cfg);
bool renderInMenu(BlendConfig& cfg);


// --------------------------------------------
enum eSamplerType {
	  SAMPLER_WRAP_LINEAR = 0
	, SAMPLER_BORDER_LINEAR
	, SAMPLER_PCF_SHADOWS
	, SAMPLER_CLAMP_LINEAR
    , SAMPLER_PCF_SHADOWS_WHITE
	, SAMPLER_CLAMP_BILINEAR
	, SAMPLER_CLAMP_POINT
	, SAMPLERS_COUNT
};
void activateSampler(int slot, eSamplerType cfg);

// --------------------------------------------
bool createRenderUtils();
void destroyRenderUtils();
void activateDefaultRenderState();

#endif

