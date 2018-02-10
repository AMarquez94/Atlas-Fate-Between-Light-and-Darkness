#include "mcv_platform.h"
#include "render_technique.h"

// ----------------------------------------------
class CRenderTechniqueResourceClass : public CResourceClass {
public:
	CRenderTechniqueResourceClass() {
		class_name = "Techniques";
		extension = ".tech";
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
bool CRenderTechnique::create(const std::string& name, json& j) {

	// --------------------------------------------
	std::string vs_file = j.value("vs_file", "data/shaders/shaders.fx");
	std::string vs_entry_point = j.value("vs_entry_point", "VS");
	std::string vertex_type = j["vertex_type"];
	std::string ps_file = j.value("ps_file", vs_file);
	std::string ps_entry_point = j.value("ps_entry_point", "PS");

	if (!vs.create(vs_file, vs_entry_point, vertex_type))
		return false;

	if (!ps.create(ps_file, ps_entry_point))
		return false;

	setNameAndClass(name, getResourceClassOf<CRenderTechnique>());

	return true;
}

void CRenderTechnique::destroy() {
	vs.destroy();
	ps.destroy();
}

void CRenderTechnique::activate() const {
	vs.activate();
	ps.activate();
}

void CRenderTechnique::debugInMenu() {
	ImGui::Text("Technique %s", name.c_str());
}
