#include "mcv_platform.h"
#include "physics_mesh.h"
#include "components/comp_transform.h"
#include "render/mesh/mesh_loader.h"

// ----------------------------------------------
class ColliderMeshResourceClass : public CResourceClass {
public:
	ColliderMeshResourceClass() {
		class_name = "colliders";
		extensions = { ".collider" };
	}
	IResource* create(const std::string& name) const override {
		dbg("Creating mesh %s\n", name.c_str());
		CPhysicsMesh* res = loadPhysicsMesh(name.c_str());
		return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CPhysicsMesh>() {
	static ColliderMeshResourceClass the_resource_class;
	return &the_resource_class;
}

bool CPhysicsMesh::create(
	const void* vertex_data
	, const void* index_data
	, uint32_t vertex_size
	, uint32_t index_size
	, uint32_t tbytes_per_vertex
	, uint32_t tbytes_per_idx) {

	assert(vertex_data != nullptr);
	assert(index_data != nullptr);

	num_vertexs = vertex_size;
	num_indices = index_size;
	bytes_per_vtx = tbytes_per_vertex;
	bytes_per_idx = tbytes_per_idx;

	uint8_t *vertex_buffer = (uint8_t*)vertex_data;
	uint8_t *index_buffer = (uint8_t*)index_data;

	vtxs = std::vector<uint8_t>(vertex_buffer, vertex_buffer + vertex_size);
	idxs = std::vector<uint8_t>(index_buffer, index_buffer + index_size);

	return true;
}

void CPhysicsMesh::destroy()
{
	// Release data
}

void CPhysicsMesh::debugInMenu() {

	ImGui::Text("%d vertexs", num_vertexs);
	ImGui::Text("%d vertexs", num_indices);
}
