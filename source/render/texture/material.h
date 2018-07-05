#pragma once

#include "resources/resource.h"
#include "render/cte_buffer.h"
#include "ctes.h"                     // texture slots

class CTexture;

// ----------------------------------------------
class CMaterial : public IResource {

protected:
	static const int max_textures = TS_NUM_MATERIALS_SLOTS;
    CRenderCte<CCteMaterial> cb_material;

	VEC4 color;
	bool  cast_shadows = true;

public:

	const CTexture* textures[max_textures];
	const CRenderTechnique* tech = nullptr;


	CMaterial();

    virtual void activate() const;
    virtual bool create(const json& j);

	void destroy() override;
	void debugInMenu() override;
	void onFileChanged(const std::string& filename) override;
    void activateTextures(int slot) const;
    void setCBMaterial(float alpha_outline);
	bool castsShadows() const { return cast_shadows; }

protected:

	const ID3D11ShaderResourceView* srvs[max_textures];

};


// ----------------------------------------------
class CMaterialMixing : public CMaterial {
    const CMaterial*  mats[3] = { nullptr, nullptr, nullptr };
    const CTexture*   mix_blend_weights = nullptr;
public:
    void activate() const override;
    bool create(const json& j) override;
    void debugInMenu() override;
};
