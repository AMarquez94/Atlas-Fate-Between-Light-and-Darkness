#pragma once

class CVertexShader;
class CPixelShader;

class CRenderTechnique : public IResource {

	std::string vs_file;
	std::string ps_file;
	std::string vs_entry_point;
	std::string ps_entry_point;
	std::string vertex_type;

	std::string category = "default";
	uint32_t category_id = 0;

	bool reloadVS();
	bool reloadPS();

public:
	CVertexShader * vs = nullptr;
	CPixelShader*  ps = nullptr;
	static const CRenderTechnique* current;
	// CTexture* textures;

	void activate() const;
	bool create(const std::string& name, json& j);
	void debugInMenu() override;
	void destroy() override;
	void onFileChanged(const std::string& filename) override;

	uint32_t getCategoryID() const { return category_id; }
	const std::string& getCategory() const { return category; }
};

