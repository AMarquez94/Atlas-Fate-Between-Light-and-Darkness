#include "mcv_platform.h"
#include "render_technique.h"
#include "pixel_shader.h"
#include "vertex_shader.h"

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
	//if (ps_name.empty())
	//  return true;

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
bool CRenderTechnique::create(const std::string& name, json& j) {

	// --------------------------------------------
	vs_file = j.value("vs_file", "data/shaders/shaders.fx");
	vs_entry_point = j.value("vs_entry_point", "VS");
	vertex_type = j.value("vertex_type", "");
	ps_file = j.value("ps_file", vs_file);
	ps_entry_point = j.value("ps_entry_point", "PS");

	if (!reloadVS())
		return false;
	if (!reloadPS())
		return false;

	category = j.value("category", "default");
	category_id = getID(category.c_str());
	uses_skin = j.value("uses_skin", false);

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
	assert(ps);
	ps->activate();
	// Save me as the current active technique
	current = this;
}

void CRenderTechnique::debugInMenu() {
	ImGui::LabelText("VS FX", "%s", vs_file.c_str());
	ImGui::LabelText("VS", "%s", vs_entry_point.c_str());
	ImGui::LabelText("PS FX", "%s", ps_file.c_str());
	ImGui::LabelText("VS", "%s", ps_entry_point.c_str());
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
