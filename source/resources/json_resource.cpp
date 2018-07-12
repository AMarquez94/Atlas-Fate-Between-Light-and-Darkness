#include "mcv_platform.h"
#include "json_resource.h"

// ----------------------------------------------
class CJsonResourceClass : public CResourceClass {
public:
	CJsonResourceClass() {
		class_name = "Json";
		extensions = { ".json", ".scene", ".prefab", ".bt" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating json resource %s\n", name.c_str());
		CJsonResource* res = new CJsonResource();
		bool is_ok = res->create(name);
		assert(is_ok);
		return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CJsonResource>() {
	static CJsonResourceClass the_resource_class;
	return &the_resource_class;
}

// ----------------------------------------------------------
bool CJsonResource::create(const std::string& name) {
	json_contents = loadJson(name);
	return true;
}

void CJsonResource::debugInMenu() {
	ImGui::Text("%s", json_contents.dump(2).c_str());
}

void CJsonResource::onFileChanged(const std::string& filename) {
	if (filename != getName())
		return;
	create(filename);
}