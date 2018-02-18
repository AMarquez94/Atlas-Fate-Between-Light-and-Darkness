#pragma once

class CVertexShader;
class CPixelShader;

class CRenderTechnique : public IResource {

	std::string vs_file;
	std::string ps_file;
	std::string vs_entry_point;
	std::string ps_entry_point;
	std::string vertex_type;

	bool reloadVS();
	bool reloadPS();

public:
	CVertexShader * vs = nullptr;
	CPixelShader*  ps = nullptr;

	// CTexture* textures;

	void activate() const;
	bool create(const std::string& name, json& j);
	void debugInMenu() override;
	void destroy() override;
	void onFileChanged(const std::string& filename) override;
};

