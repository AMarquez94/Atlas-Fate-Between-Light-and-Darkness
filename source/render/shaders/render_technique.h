#pragma once
#include "render/render_utils.h"

class CVertexShader;
class CPixelShader;
class CTexture;

// ----------------------------------------------------------
struct TTextureSlots {
    struct TSlot {
        int             slot = -1;
        const CTexture* texture;
    };
    // Textures associated to this technique
    std::vector< TSlot > textures;
    void load(const std::string& name, const json& j);
    void activate() const;
};

class CRenderTechnique : public IResource {

	std::string vs_file;
	std::string ps_file;
	std::string vs_entry_point;
	std::string ps_entry_point;
	std::string vertex_type;

	bool uses_skin = false;

	std::string category = "default";
	uint32_t category_id = 0;
    uint32_t priority = 0;
    UINT     color_mask = 255;

	RSConfig rs_config = RSCFG_DEFAULT;
	ZConfig z_config = ZCFG_DEFAULT;
	BlendConfig blend_config = BLEND_CFG_DEFAULT;

	bool reloadVS();
	bool reloadPS();

    TTextureSlots texture_slots;

public:
	CVertexShader * vs = nullptr;
	CPixelShader*  ps = nullptr;
	static const CRenderTechnique* current;

	void activate() const;
	bool create(const std::string& name, json& j);
	void debugInMenu() override;
	void destroy() override;
	void onFileChanged(const std::string& filename) override;
	bool usesSkin() const { return uses_skin; }
    bool usesInstancing() const;
    
    uint32_t getPriority() const { return priority; }
    uint32_t getCategoryID() const { return category_id; }
	const std::string& getCategory() const { return category; }
};

