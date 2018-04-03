#include "mcv_platform.h"
#include "render_utils.h"
#include "render_objects.h"

void renderLine(VEC3 src, VEC3 dst, VEC4 color) {
  MAT44 world = MAT44::CreateLookAt(src, dst, VEC3(0, 1, 0)).Invert();
  float distance = VEC3::Distance(src, dst);
  world = MAT44::CreateScale(1, 1, -distance) * world;
  cb_object.obj_world = world;
  cb_object.obj_color = color;
  cb_object.updateGPU();

  auto mesh = Resources.get("line.mesh")->as<CRenderMesh>();
  mesh->activateAndRender();
}


// ---------------------------------------------------------------
// Reads a DX11 format from a string
DXGI_FORMAT readFormat(const json& j, const std::string& label) {
	std::string format = j.value(label, "");
	assert(!format.empty());

	if (format == "none")
		return DXGI_FORMAT_UNKNOWN;
	if (format == "R8G8B8A8_UNORM")
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	if (format == "R32_TYPELESS")
		return DXGI_FORMAT_R32_TYPELESS;

	return DXGI_FORMAT_UNKNOWN;
}

// -----------------------------------------------
struct CRasterizers {

	ID3D11RasterizerState *rasterize_states[RSCFG_COUNT];
	const char*            names[RSCFG_COUNT];

	bool add(const D3D11_RASTERIZER_DESC& desc, RSConfig cfg, const char* name) {
		// Create the dx obj in the slot 'cfg'
		HRESULT hr = Render.device->CreateRasterizerState(&desc, &rasterize_states[cfg]);
		if (FAILED(hr))
			return false;
		// Assing the name
		setDXName(rasterize_states[cfg], name);
		// Save also the name for the ui
		names[cfg] = name;
		return true;
	}

	bool create() {

		rasterize_states[RSCFG_DEFAULT] = nullptr;
		names[RSCFG_DEFAULT] = "default";

		// Depth bias options when rendering the shadows
		D3D11_RASTERIZER_DESC sdesc = {
			D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
			D3D11_CULL_BACK,  // D3D11_CULL_MODE CullMode;
			FALSE,            // BOOL FrontCounterClockwise;
			13,               // INT DepthBias;
			0.0f,             // FLOAT DepthBiasClamp;
			2.0f,             // FLOAT SlopeScaledDepthBias;
			TRUE,             // BOOL DepthClipEnable;
			FALSE,            // BOOL ScissorEnable;
			FALSE,            // BOOL MultisampleEnable;
			FALSE,            // BOOL AntialiasedLineEnable;
		};
		if (!add(sdesc, RSCFG_SHADOWS, "shadows"))
			return false;

		// --------------------------------------------------
		// No culling at all
		D3D11_RASTERIZER_DESC desc = {
			D3D11_FILL_SOLID, // D3D11_FILL_MODE FillMode;
			D3D11_CULL_NONE,  // D3D11_CULL_MODE CullMode;
			FALSE,            // BOOL FrontCounterClockwise;
			0,                // INT DepthBias;
			0.0f,             // FLOAT DepthBiasClamp;
			0.0,              // FLOAT SlopeScaledDepthBias;
			TRUE,             // BOOL DepthClipEnable;
			FALSE,            // BOOL ScissorEnable;
			FALSE,            // BOOL MultisampleEnable;
			FALSE,            // BOOL AntialiasedLineEnable;
		};
		if (!add(desc, RSCFG_CULL_NONE, "cull_none"))
			return false;

		// Culling is reversed. Used when rendering the light volumes
		desc.CullMode = D3D11_CULL_FRONT;
		if (!add(desc, RSCFG_REVERSE_CULLING, "reverse_culling"))
			return false;

		// Wireframe and default culling back
		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_BACK;
		if (!add(desc, RSCFG_WIREFRAME, "wireframe"))
			return false;

		return true;
	}

	void destroy() {
		for (int i = 0; i < RSCFG_COUNT; ++i)
			SAFE_RELEASE(rasterize_states[i]);
	}

};


// ------------------------------------------------------------
struct CSamplers {
	ID3D11SamplerState* all_samplers[SAMPLERS_COUNT];

	bool create() {

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;

		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		HRESULT hr = Render.device->CreateSamplerState(
			&sampDesc, &all_samplers[SAMPLER_WRAP_LINEAR]);
		if (FAILED(hr))
			return false;

		
		setDXName(all_samplers[SAMPLER_WRAP_LINEAR], "WRAP_LINEAR");
		/*
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		hr = ::Render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_CLAMP_LINEAR]);
		if (FAILED(hr))
		return false;
		setDXName(all_samplers[SAMPLER_CLAMP_LINEAR], "CLAMP_LINEAR");

		// For FXAA Bilinear becasue in the mips we do point
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		hr = Render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_CLAMP_BILINEAR]);
		if (FAILED(hr))
		return false;
		setDXName(all_samplers[SAMPLER_CLAMP_BILINEAR], "CLAMP_BILINEAR");
		*/

		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.BorderColor[0] = 0;
		sampDesc.BorderColor[1] = 0;
		sampDesc.BorderColor[2] = 0;
		sampDesc.BorderColor[3] = 0;
		hr = Render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_BORDER_LINEAR]);
		if (FAILED(hr))
		return false;
		setDXName(all_samplers[SAMPLER_BORDER_LINEAR], "BORDER_LINEAR");

		/*
		// PCF sampling
		D3D11_SAMPLER_DESC sampler_desc = {
		D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,// D3D11_FILTER Filter;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
		D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
		0,//FLOAT MipLODBias;
		0,//UINT MaxAnisotropy;
		D3D11_COMPARISON_LESS, //D3D11_COMPARISON_FUNC ComparisonFunc;
		0.0, 0.0, 0.0, 0.0,//FLOAT BorderColor[ 4 ];
		0,//FLOAT MinLOD;
		0//FLOAT MaxLOD;
		};
		hr = Render.device->CreateSamplerState(
		&sampler_desc, &all_samplers[SAMPLER_PCF_SHADOWS]);
		if (FAILED(hr))
		return false;
		setDXName(all_samplers[SAMPLER_PCF_SHADOWS], "PCF_SHADOWS");

		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		hr = Render.device->CreateSamplerState(
		&sampDesc, &all_samplers[SAMPLER_CLAMP_POINT]);
		if (FAILED(hr))
		return false;
		setDXName(all_samplers[SAMPLER_CLAMP_POINT], "CLAMP_POINT");
		*/

		return true;
	}

	void destroy() {
		for (int i = 0; i < SAMPLERS_COUNT; ++i)
			SAFE_RELEASE(all_samplers[i]);
	}

	void activateTextureSamplers() {
		Render.ctx->PSSetSamplers(0, SAMPLERS_COUNT, all_samplers);
	}
};



// -----------------------------------------------
static CSamplers    samplers;
static CRasterizers rasterizers;

// Activate just one
void activateSampler(int slot, eSamplerType sample) {
	Render.ctx->PSSetSamplers(slot, 1, &samplers.all_samplers[sample]);
}

// Activate all at once
void activateAllSamplers() {
	Render.ctx->PSSetSamplers(0, SAMPLERS_COUNT, samplers.all_samplers);
}

void activateRSConfig(enum RSConfig cfg) {
	Render.ctx->RSSetState(rasterizers.rasterize_states[cfg]);
}

// ---------------------------------------
RSConfig RSConfigFromString(const std::string& aname) {
	for (int i = 0; i < RSCFG_COUNT; ++i) {
		if (rasterizers.names[i] == aname)
			return RSConfig(i);
	}
	fatal("Invalid rsconfig name %s\n", aname.c_str());
	return RSCFG_DEFAULT;
}

bool renderInMenu(RSConfig& cfg) {
	return ImGui::Combo("RSConfig", (int*)&cfg, rasterizers.names, RSCFG_COUNT);
}

// -----------------------------------------------
bool createRenderUtils() {

	bool is_ok = true;

	is_ok &= samplers.create();
	is_ok &= rasterizers.create();

	activateDefaultRenderState();
	return is_ok;
}

void destroyRenderUtils() {
	rasterizers.destroy();
	samplers.destroy();
}

void activateDefaultRenderState() {
	activateAllSamplers();
	activateRSConfig(RSCFG_DEFAULT);
}


