#pragma once

#include "resources/resource.h"
#include "render/cte_buffer.h"
#include "ctes.h"                     // texture slots

class CTexture;

// ----------------------------------------------
class CMaterial : public IResource {

	static const int max_textures = TS_NUM_MATERIALS_SLOTS;
	CRenderCte<CCteMaterial> cb_material;

	bool  cast_shadows = true;

public:

	const CTexture* textures[max_textures];
	const CRenderTechnique* tech = nullptr;

	CMaterial();

	void activate() const;
	bool create(const std::string& name);
	void destroy() override;
	void debugInMenu() override;
	void onFileChanged(const std::string& filename) override;
	bool castsShadows() const { return cast_shadows; }

protected:

	const ID3D11ShaderResourceView* srvs[max_textures];

};