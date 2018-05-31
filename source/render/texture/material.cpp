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

        auto j = loadJson(name);
        std::string mat_type = j.value("type", "std");

        CMaterial* res = nullptr;
        if (mat_type == "std") {
            res = new CMaterial();
        }
        else if (mat_type == "mix") {
            res = new CMaterialMixing();
        }
        else {
            fatal("Invalid material type %s", mat_type.c_str());
        }

		bool is_ok = res->create(j);
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
bool CMaterial::create(const json& j) {

	// If nothing is set, use 'pbr.tech'
	std::string technique_name = "pbr.tech";
	if (j.count("technique"))
		technique_name = j.value("technique", "");

	tech = Resources.get(technique_name)->as<CRenderTechnique>();
	cast_shadows = j.value("shadows", true);

	// Setting default textures
	textures[TS_EMISSIVE] = Resources.get("data/textures/default_emissive.dds")->as<CTexture>();
	textures[TS_HEIGHT] = Resources.get("data/textures/default_white.dds")->as<CTexture>();

    if (j.count("textures")) {
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
            else if (slot == "aocclusion")
                ts = TS_AOCCLUSION;
            else if (slot == "height")
                ts = TS_HEIGHT;

            assert(ts != TS_NUM_MATERIALS_SLOTS || fatal("Material %s has an invalid texture slot %s\n", name.c_str(), slot.c_str()));
            textures[ts] = Resources.get(texture_name)->as<CTexture>();

            // To update all textures in a single DX call
            srvs[ts] = textures[ts]->getShaderResourceView();
        }
    }

	if (!cb_material.create(CB_MATERIAL))
		return false;

	// Load default values
	cb_material.scalar_metallic = -1.f;
	cb_material.scalar_roughness = -1.f;
	cb_material.scalar_irradiance_vs_mipmaps = 0.0f;
	cb_material.color_emission = VEC4(1, 1, 1, 1);
	cb_material.scalar_emission = j.value("emission", 10.0f);

    cb_material.mix_boost_r = 0;
    cb_material.mix_boost_g = 0;
    cb_material.mix_boost_b = 0;
    cb_material.mat_alpha_outline = 1;

	if (j.count("self_color"))
		cb_material.color_emission = loadVEC4(j["self_color"]);

	cb_material.color_material = VEC4(1, 1, 1, 1);
	if (j.count("color"))
		cb_material.color_material = loadVEC4(j["color"]);

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
    activateTextures(TS_ALBEDO);
}

void CMaterial::activateTextures(int slot0) const {

    Render.ctx->PSSetShaderResources(slot0, max_textures, (ID3D11ShaderResourceView**)srvs);
}


void CMaterial::debugInMenu() {

	((CRenderTechnique*)tech)->debugInMenu();
	ImGui::Checkbox("Cast Shadows", &cast_shadows);
	for (int i = 0; i < max_textures; ++i)
		if (textures[i])
			((CTexture*)textures[i])->debugInMenu();

	// Allow overwrite the metallic and roughness of the material
	bool enabled;

	enabled = (cb_material.scalar_metallic >= 0.f);
	if (ImGui::Checkbox("Custom Metallic", &enabled))
		cb_material.scalar_metallic = enabled ? 0.f : -1.f;
	if (cb_material.scalar_metallic >= 0.f)
		ImGui::DragFloat("Metallic", &cb_material.scalar_metallic, 0.01f, 0.f, 1.f);

	enabled = (cb_material.scalar_roughness >= 0.f);
	if (ImGui::Checkbox("Custom Roughness", &enabled))
		cb_material.scalar_roughness = enabled ? 0.f : -1.f;
	if (cb_material.scalar_roughness >= 0.f)
		ImGui::DragFloat("Roughness", &cb_material.scalar_roughness, 0.01f, 0.f, 1.f);

	ImGui::DragFloat("Emissive Intensity", &cb_material.scalar_emission, 0.01f, 0.f, 1000.f);
	ImGui::ColorEdit3("Emissive Color", &cb_material.color_emission.x);
	ImGui::DragFloat("irradiance_vs_mipmaps", &cb_material.scalar_irradiance_vs_mipmaps, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("outline_amount", &cb_material.mat_alpha_outline, 0.01f, 0.f, 1.f);

	cb_material.updateGPU();
}


// ---------------------------------------------------------
void CMaterialMixing::activate() const {

    // Upload ctes
    cb_material.activate();

    // Tech
    tech->activate();

    // 3 materials sets
    mats[0]->activateTextures(TS_FIRST_SLOT_MATERIAL_0);
    mats[1]->activateTextures(TS_FIRST_SLOT_MATERIAL_1);
    mats[2]->activateTextures(TS_FIRST_SLOT_MATERIAL_2);

    mix_blend_weights->activate(TS_MIX_BLEND_WEIGHTS);
}

bool CMaterialMixing::create(const json& j) {
    if (!CMaterial::create(j))
        return false;
    for (int i = 0; i < 3; ++i) {
        std::string mat_name = j["mats"][i];
        mats[i] = Resources.get(mat_name)->as< CMaterial >();
        assert(mats[i]);
    }

    mix_blend_weights = Resources.get(j.value("mix_blend_weights", "data/textures/black.dds"))->as<CTexture>();
    assert(mix_blend_weights);

    return true;
}

void CMaterialMixing::debugInMenu() {
    ((CRenderTechnique*)tech)->debugInMenu();
    ImGui::DragFloat("Boost R", &cb_material.mix_boost_r, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("Boost G", &cb_material.mix_boost_g, 0.01f, 0.f, 1.f);
    ImGui::DragFloat("Boost B", &cb_material.mix_boost_b, 0.01f, 0.f, 1.f);
    for (int i = 0; i < 3; ++i)
        ((CMaterial*)mats[i])->debugInMenu();
    cb_material.updateGPU();
}
