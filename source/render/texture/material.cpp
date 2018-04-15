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

CMaterial::CMaterial() : cb_material("Material") {}

// ----------------------------------------------------------
bool CMaterial::create(const std::string& name) {

	auto j = loadJson(name);

	// If nothing is set, use 'pbr.tech'
	std::string technique_name = "pbr.tech";
	if (j.count("technique"))
		technique_name = j["technique"];

	tech = Resources.get(technique_name)->as<CRenderTechnique>();

	cast_shadows = j.value("shadows", true);

	auto& j_textures = j["textures"];
	for (auto it = j_textures.begin(); it != j_textures.end(); ++it) {
		std::string slot = it.key();
		std::string texture_name = it.value();

		int ts = TS_NUM_MATERIALS_SLOTS;
		if (slot == "albedo")
			ts = TS_ALBEDO;
		else if (slot == "normal")
			ts = TS_NORMAL;
		else if (slot == "lightmap")
			ts = TS_LIGHTMAP;
		else if (slot == "metallic")
			ts = TS_METALLIC;
		else if (slot == "roughness")
			ts = TS_ROUGHNESS;
		else if (slot == "emissive")
			ts = TS_EMISSIVE;

		assert(ts != TS_NUM_MATERIALS_SLOTS || fatal("Material %s has an invalid texture slot %s\n", name.c_str(), slot.c_str()));
		textures[ts] = Resources.get(texture_name)->as<CTexture>();

		// To update all textures in a single DX call
		srvs[ts] = textures[ts]->getShaderResourceView();
	}

	if (!cb_material.create(CB_MATERIAL))
		return false;

	// Load values from json
	cb_material.scalar_emission = j.value("emission", 10.0f);
	cb_material.scalar_metallic = j.value("metallic", 0.0f);
	cb_material.scalar_roughness = j.value("smoothness", 0.5f);
	cb_material.scalar_irradiance_vs_mipmaps = 0.f;
	cb_material.color_emission = VEC4(1, 1, 1, 1);

	if (j.count("color"))
		cb_material.color_emission = loadVEC4(j["color"]);

	cb_material.updateGPU();

	return true;
}

void CMaterial::onFileChanged(const std::string& filename) {
	if (filename == getName()) {
		create(filename);
	}
	else {
		// Maybe a texture has been updated, get the new shader resource view
		for (int i = 0; i < max_textures; ++i)
			srvs[i] = textures[i] ? textures[i]->getShaderResourceView() : nullptr;
	}
}

void CMaterial::destroy() {

	cb_material.destroy();
}

void CMaterial::activate() const {

	cb_material.activate();

	tech->activate();
	Render.ctx->PSSetShaderResources(0, max_textures, (ID3D11ShaderResourceView**)srvs);
}

void CMaterial::debugInMenu() {

	((CRenderTechnique*)tech)->debugInMenu();
	ImGui::Checkbox("Cast Shadows", &cast_shadows);
	for (int i = 0; i < max_textures; ++i)
		if (textures[i])
			((CTexture*)textures[i])->debugInMenu();

	ImGui::DragFloat("Metallic", &cb_material.scalar_metallic, 0.01f, 0.f, 1.f);
	ImGui::DragFloat("Roughness", &cb_material.scalar_roughness, 0.01f, 0.f, 1.f);
	ImGui::DragFloat("Emissive Intensity", &cb_material.scalar_emission, 0.01f, 0.f, 1000.f);
	ImGui::ColorEdit3("Emissive Color", &cb_material.color_emission.x);
	ImGui::DragFloat("irradiance_vs_mipmaps", &cb_material.scalar_irradiance_vs_mipmaps, 0.01f, 0.f, 1.f);

	cb_material.updateGPU();
}
