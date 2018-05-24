#include "mcv_platform.h"
#include "render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_utils.h"
#include "render/render_objects.h"      // createDepthStencil

CRenderToTexture* CRenderToTexture::current_rt = nullptr;

CRenderToTexture::~CRenderToTexture() {

}

void CRenderToTexture::destroy() {

	SAFE_RELEASE(render_target_view);
	SAFE_RELEASE(depth_stencil_view);
	SAFE_RELEASE(depth_resource);
	CTexture::destroy();
}

void CRenderToTexture::debugInMenu() {

	// If we have a color buffer...
	if (render_target_view)
		CTexture::debugInMenu();

	// Show the Depth Buffer if exists
	if (depth_stencil_view && getZTexture())
		getZTexture()->debugInMenu();
}


bool CRenderToTexture::createRT(

	const char* new_name
	, int new_xres
	, int new_yres
	, DXGI_FORMAT new_color_format
	, DXGI_FORMAT new_depth_format
	, bool        uses_depth_of_backbuffer
) {

	xres = new_xres;
	yres = new_yres;
	setNameAndClass(new_name, getResourceClassOf<CTexture>());

	// Create color buffer
	color_format = new_color_format;
	if (color_format != DXGI_FORMAT_UNKNOWN) {
		if (!create(new_xres, new_yres, color_format, CREATE_RENDER_TARGET))
			return false;

		// The part of the render target view
		HRESULT hr = Render.device->CreateRenderTargetView((ID3D11Resource*)texture, nullptr, &render_target_view);
		if (FAILED(hr))
			return false;
		setDXName(render_target_view, getName().c_str());
	}
	else {
		Resources.registerResource(this);
	}

	// Create ZBuffer 
	depth_format = new_depth_format;
	if (depth_format != DXGI_FORMAT_UNKNOWN) {
		if (!createDepthStencil(getName(), xres, yres, new_depth_format, &depth_resource, &depth_stencil_view, &ztexture))
			return false;
	}
	else {
		// Create can have the option to use the ZBuffer of the backbuffer
		if (uses_depth_of_backbuffer) {
			assert(xres == Render.width);
			assert(yres == Render.height);
			depth_stencil_view = Render.depthStencilView;
			depth_stencil_view->AddRef();
		}
	}

	return true;
}

CRenderToTexture* CRenderToTexture::activateRT() {

	CRenderToTexture* prev_rt = current_rt;
	Render.ctx->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
	activateViewport();
	current_rt = this;
	return prev_rt;
}

void CRenderToTexture::activateViewport() {

	D3D11_VIEWPORT vp;
	vp.Width = (float)xres;
	vp.Height = (float)yres;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	Render.ctx->RSSetViewports(1, &vp);
}

void CRenderToTexture::clear(VEC4 clear_color) {

	assert(render_target_view);
	Render.ctx->ClearRenderTargetView(render_target_view, &clear_color.x);
}

void CRenderToTexture::clearZ() {

	if (depth_stencil_view)
        Render.ctx->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
