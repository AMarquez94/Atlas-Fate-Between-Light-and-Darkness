#ifndef INC_RENDER_RENDER_TO_TEXTURE_H_
#define INC_RENDER_RENDER_TO_TEXTURE_H_

#include "texture.h"
#include <d3d11.h>

class CRenderToTexture : public CTexture {

	DXGI_FORMAT             color_format = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT             depth_format = DXGI_FORMAT_UNKNOWN;

	ID3D11RenderTargetView* render_target_view = nullptr;
	ID3D11DepthStencilView* depth_stencil_view = nullptr;

	ID3D11Texture2D*        depth_resource = nullptr;
	CTexture*               ztexture = nullptr;

	static CRenderToTexture* current_rt;

public:

	~CRenderToTexture();

	bool createRT(const char* name, int new_xres, int new_yres
		, DXGI_FORMAT new_color_format
		, DXGI_FORMAT new_depth_format = DXGI_FORMAT_UNKNOWN
		, bool        uses_depth_of_backbuffer = false
	);
	void destroy() override;

	CRenderToTexture* activateRT();
	void activateViewport();

	void clear(VEC4 clear_color);
	void clearZ();

	ID3D11RenderTargetView* getRenderTargetView() {
		return render_target_view;
	}
    ID3D11DepthStencilView* getDepthStencilView() {
        return depth_stencil_view;
    }

	CTexture* getZTexture() { return ztexture; }
    static CRenderToTexture* getCurrentRT() { return current_rt; }
    static void setNullRT();

	void debugInMenu() override;
};

#endif
