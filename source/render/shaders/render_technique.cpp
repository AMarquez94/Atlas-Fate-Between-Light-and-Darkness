#include "mcv_platform.h"
#include "render_technique.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "render/render_manager.h"    // forceDirty
#include "render/texture/texture.h"
#include "ctes.h"              // TS_CUBEMAP...

const CRenderTechnique* CRenderTechnique::current = nullptr;

// ----------------------------------------------
class CRenderTechniqueResourceClass : public CResourceClass {
public:
	CRenderTechniqueResourceClass() {
		class_name = "Techniques";
		extensions = { ".tech" };
	}
	IResource* create(const std::string& name) const override {
		fatal("Can't create a technique '%s'. All techniques should be declared in the data/techniques.json", name.c_str());
		return nullptr;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CRenderTechnique>() {
	static CRenderTechniqueResourceClass the_resource_class;
	return &the_resource_class;
}

// ----------------------------------------------------------
bool CRenderTechnique::reloadVS() {
	auto new_vs = new CVertexShader;
	if (!new_vs->create(vs_file.c_str(), vs_entry_point.c_str(), vertex_type))
		return false;

	// Delete the old one if it was valid
	if (vs)
		delete vs;

	// Keep the new one
	vs = new_vs;
	return true;
}

// ----------------------------------------------------------
bool CRenderTechnique::reloadPS() {

	// To support shadow map generation which does not use any PS
	if (ps_entry_point.empty())
		return true;

	auto new_ps = new CPixelShader;
	if (!new_ps->create(ps_file.c_str(), ps_entry_point.c_str()))
		return false;

	// Delete the old one if it was valid
	if (ps)
		delete ps;

	// Keep the new one
	ps = new_ps;
	return true;
}

// ----------------------------------------------------------
void TTextureSlots::load(const std::string& name, const json& j) {

    // Load textures associated to this technique
    if (j.count("textures")) {
        auto& j_textures = j["textures"];
        for (auto it = j_textures.begin(); it != j_textures.end(); ++it) {
            TSlot s;
            std::string k = it.key();
            if (k == "environment")
                s.slot = TS_ENVIRONMENT_MAP;
            else if (k == "irradiance")
                s.slot = TS_IRRADIANCE_MAP;
            else if (k == "noise")
                s.slot = TS_NOISE_MAP;
            else if (k == "noise2")
                s.slot = TS_NOISE_MAP2;
 
            if (k == "albedo")
                s.slot = TS_ALBEDO;
            else if (k == "normal")
                s.slot = TS_NORMAL;
            else if (k == "metallic")
                s.slot = TS_METALLIC;
            else if (k == "roughness")
                s.slot = TS_ROUGHNESS;

            if (s.slot == -1) {
                fatal("Invalid key '%s' in textures for %s\n", k.c_str(), name.c_str());
                continue;
            }

            s.texture = Resources.get(it.value())->as<CTexture>();
            textures.push_back(s);
        }
    }
}

void TTextureSlots::activate() const {
    for (auto& t : textures)
        t.texture->activate(t.slot);
}

// ----------------------------------------------------------
bool CRenderTechnique::create(const std::string& name, json& j) {

	// --------------------------------------------
	vs_file = j.value("vs_file", "");
	if (vs_file.empty()) fatal("vs_file of technique %s must not be empty", name.c_str());
	vs_entry_point = j.value("vs_entry_point", "VS");
	vertex_type = j.value("vertex_type", "");
	ps_file = j.value("ps_file", vs_file);
	ps_entry_point = j.value("ps_entry_point", "PS");
    color_mask = j.value("color_mask", 255);

	if (!reloadVS())
		return false;
	if (!reloadPS())
		return false;

	category = j.value("category", "default");
	category_id = getID(category.c_str());
	uses_skin = j.value("uses_skin", false);
    priority = j.value("priority", 100);

	rs_config = RSConfigFromString(j.value("rs_config", "default"));
	z_config = ZConfigFromString(j.value("z", "default"));
	blend_config = BlendConfigFromString(j.value("blend", "default"));

    texture_slots.load(name, j);
	setNameAndClass(name, getResourceClassOf<CRenderTechnique>());

	return true;
}

void CRenderTechnique::destroy() {
	if (vs)
		vs->destroy(), vs = nullptr;
	if (ps)
		ps->destroy(), ps = nullptr;
}

void CRenderTechnique::activate() const {

	// If I'm the current active tech, no need to reactive myself in DX
	if (current == this)
		return;
	assert(vs);
	vs->activate();

	// We might not have a valid ps, for example, when rendering the
	// shadows maps
	if (ps)
		ps->activate();
	else
		Render.ctx->PSSetShader(nullptr, nullptr, 0);
	
	activateRSConfig(rs_config);
	activateZConfig(z_config, color_mask);
	activateBlendConfig(blend_config);

	// Activate the textures associated to this technique
    texture_slots.activate();

	// Save me as the current active technique
	current = this;
}

void CRenderTechnique::debugInMenu() {

	ImGui::LabelText("VS FX", "%s", vs_file.c_str());
	ImGui::LabelText("VS", "%s", vs_entry_point.c_str());
	ImGui::LabelText("PS FX", "%s", ps_file.c_str());
	ImGui::LabelText("VS", "%s", ps_entry_point.c_str());
	::renderInMenu(z_config);
	::renderInMenu(rs_config);
	::renderInMenu(blend_config);
    if (ImGui::DragInt("Priority", (int*)&priority, 1, 0, 1000))
        CRenderManager::get().forceDirty();
}

bool CRenderTechnique::usesInstancing() const {

    return vs->getVertexDecl()->instancing;
}

void CRenderTechnique::onFileChanged(const std::string& filename) {

	if (filename == vs_file) {
		dbg("Reloading vs of tech %s\n", this->getName().c_str());
		reloadVS();
	}

	if (filename == ps_file) {
		dbg("Reloading ps of tech %s\n", this->getName().c_str());
		reloadPS();
	}
}
