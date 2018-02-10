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

		/*
		setDXName(all_samplers[SAMPLER_WRAP_LINEAR], "WRAP_LINEAR");

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

// Activate just one
void activateSampler(int slot, eSamplerType sample) {
	Render.ctx->PSSetSamplers(slot, 1, &samplers.all_samplers[sample]);
}

// Activate all at once
void activateAllSamplers() {
	Render.ctx->PSSetSamplers(0, SAMPLERS_COUNT, samplers.all_samplers);
}

// -----------------------------------------------
bool createRenderUtils() {

	bool is_ok = true;

	is_ok &= samplers.create();

	activateDefaultRenderState();
	return is_ok;
}

void destroyRenderUtils() {
	samplers.destroy();
}

void activateDefaultRenderState() {
	activateAllSamplers();
}


