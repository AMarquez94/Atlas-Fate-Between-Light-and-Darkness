#include "mcv_platform.h"
#include "material.h"
#include "texture.h"

// ----------------------------------------------
class CMaterialResourceClass : public CResourceClass {
public:
	CMaterialResourceClass() {
		class_name = "Materials";
		extensions = { ".material" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating material %s\n", name.c_str());
		CMaterial* res = new CMaterial();
		bool is_ok = res->create(name);
		assert(is_ok);
		return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CMaterial>() {
	static CMaterialResourceClass the_resource_class;
	return &the_resource_class;
}

// ----------------------------------------------------------
bool CMaterial::create(const std::string& name) {

	auto j = loadJson(name);

	std::string technique_name = j["technique"];
	tech = Resources.get(technique_name)->as<CRenderTechnique>();

	cast_shadows = j.value("shadows", true);

	auto& j_textures = j["textures"];
	for (auto it = j_textures.begin(); it != j_textures.end(); ++it) {
		std::string slot = it.key();
		std::string texture_name = it.value();

		eTextureSlot ts = TS_COUNT;
		if (slot == "albedo")
			ts = TS_ALBEDO;
		else if (slot == "normal")
			ts = TS_NORMAL;
		// ...

		assert(ts != TS_COUNT || fatal("Material %s has an invalid texture slot %s\n", name.c_str(), slot.c_str()));

		textures[ts] = Resources.get(texture_name)->as<CTexture>();

		// To update all textures in a single DX call
		srvs[ts] = textures[ts]->getShaderResourceView();
	}

	return true;
}

void CMaterial::onFileChanged(const std::string& filename) {
	if (filename == getName()) {
		create(filename);
	}
	else {
		// Maybe a texture has been updated, get the new shader resource view
		for (int i = 0; i < TS_COUNT; ++i)
			srvs[i] = textures[i] ? textures[i]->getShaderResourceView() : nullptr;
	}
}

void CMaterial::activate() const {
	tech->activate();
	Render.ctx->PSSetShaderResources(0, TS_COUNT, (ID3D11ShaderResourceView**)srvs);
}

void CMaterial::debugInMenu() {
	((CRenderTechnique*)tech)->debugInMenu();
	ImGui::Checkbox("Cast Shadows", &cast_shadows);
	for (int i = 0; i < TS_COUNT; ++i)
		if (textures[i])
			((CTexture*)textures[i])->debugInMenu();
}
