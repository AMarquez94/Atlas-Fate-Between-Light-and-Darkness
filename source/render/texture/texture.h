#pragma once

#include "resources/resource.h"

// ----------------------------------------------
class CTexture : public IResource {

protected:
	// DX
	ID3D11Resource * texture = nullptr;
	ID3D11ShaderResourceView* shader_resource_view = nullptr;
	int                       xres = 0;
	int                       yres = 0;

public:
	void activate(int slot) const;
	bool create(const std::string& name);
	void debugInMenu() override;
	void destroy() override;
	void onFileChanged(const std::string& filename) override;
	static void setNullTexture(int slot);

	int  getWidth() const { return xres; }
	int  getHeight() const { return yres; }

	// DX stuff for the multimaterial
	const ID3D11ShaderResourceView* getShaderResourceView() const {
		return shader_resource_view;
	}

    ID3D11ShaderResourceView* getShaderResourceViewNonConst() {
        return shader_resource_view;
    }

	// Create a new texture from params
	enum TCreateOptions {
		CREATE_STATIC
		, CREATE_DYNAMIC
		, CREATE_RENDER_TARGET
        , CREATE_COMPUTE_OUTPUT
	};
	bool create(int new_xres, int new_yres, DXGI_FORMAT new_color_format, TCreateOptions create_options = CREATE_STATIC);
	void setDXParams(int new_xres, int new_yres, ID3D11Texture2D* new_texture, ID3D11ShaderResourceView* new_srv);

    ID3D11UnorderedAccessView* uav = nullptr;
};

