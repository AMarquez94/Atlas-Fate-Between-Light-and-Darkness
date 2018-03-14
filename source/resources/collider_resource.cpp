#include "mcv_platform.h"
#include "collider_resource.h"

// ----------------------------------------------
class ColliderMeshResourceClass : public CResourceClass {
public:
	ColliderMeshResourceClass() {
		class_name = "olliders";
		extensions = { ".collider" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating mesh %s\n", name.c_str());
		CPhysicsCollider* res = NULL;// loadCollider(name.c_str());
		return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CPhysicsCollider>() {
	static ColliderMeshResourceClass the_resource_class;
	return &the_resource_class;
}
