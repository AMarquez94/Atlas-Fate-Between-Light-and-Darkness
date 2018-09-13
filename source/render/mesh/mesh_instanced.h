#ifndef INC_RENDER_MESHES_INSTANCED_MESH_H_
#define INC_RENDER_MESHES_INSTANCED_MESH_H_

#include "mesh.h"

// -------------------------------------------------------------------
// I' will hold the instances data. instanced_mesh is
// a pointer to the other data, the billboard
// This part is independent of the type of instance of the mesh, which has
// been moved to a .cpp file
class CRenderMeshInstanced : public CRenderMesh {
protected:
    const CRenderMesh* instanced_mesh = nullptr;    // This is the teapot, billboard, ..
    size_t             num_instances_allocated_in_gpu = 0;
public:

    void configure(const json& j);
    void renderSubMesh(uint32_t sub_group_idx) const override;
    void reserveGPUInstances(size_t new_max_instances);
    void setInstancesData(const void* data, size_t total_instances, size_t bytes_per_instance);
    void setSubGroupSize(uint32_t num_subgroup, uint32_t new_size_for_subgroup);

    const VMeshSubGroups& getSubGroups() const override;
};

#endif
