#include "mcv_platform.h"
#include "mesh_instanced.h"

CRenderMesh* loadMeshInstanced(const std::string& name) {

    dbg("Creating instanced mesh %s\n", name.c_str());
    auto mesh = new CRenderMeshInstanced();
    json j = loadJson(name);
    mesh->configure(j);
    return mesh;
}

// -----------------------------------------------------------------
void CRenderMeshInstanced::configure(const json& j) {

    instanced_mesh = Resources.get(j["instanced_mesh"])->as<CRenderMesh>();
    vtx_decl = CVertexDeclManager::get().getByName(j["instances_type"]);
    reserveGPUInstances(j.value("num_instances_reserved", 4));
}

// -----------------------------------------------------------------
void CRenderMeshInstanced::reserveGPUInstances(size_t new_max_instances) {

    // Allocate only if we require more than what we actually have allocated
    if (new_max_instances > num_instances_allocated_in_gpu) {

        assert(vtx_decl);

        // Do we have an initial value?, start from 4
        if (!num_instances_allocated_in_gpu)
            num_instances_allocated_in_gpu = 4;

        // Keep increasing in powers of 2
        while (new_max_instances > num_instances_allocated_in_gpu)
            num_instances_allocated_in_gpu *= 2;

        // We are about to change the ib/vb, so free the old version
        if (isValid())
            destroy();

        dbg("Reallocating %ld GPU instances for mesh %s\n", num_instances_allocated_in_gpu, getName().c_str());

        // Create the VB as a dynamic buffer to hold a maximum of N instances
        bool is_ok = create(
            nullptr,                  // no vertex data provided, we might be just allocating
            num_instances_allocated_in_gpu * vtx_decl->bytes_per_vertex,  // Total bytes required
            vtx_decl->name,
            CRenderMesh::POINT_LIST,
            nullptr, 0, 0,            // No index data
            nullptr,                  // No specify group
            true                      // is dynamic
        );

        assert(is_ok && isValid());
    }
}

// -----------------------------------------------------------------
// Update the GPU with the new data
void CRenderMeshInstanced::setInstancesData(
    const void* data
    , size_t total_instances
    , size_t bytes_per_instance
) {
    assert(bytes_per_instance > 0);
    assert(bytes_per_instance == getVertexDecl()->bytes_per_vertex);
    reserveGPUInstances(total_instances);
    if (data)
        updateFromCPU(data, total_instances * bytes_per_instance);
    setSubGroupSize(0, (uint32_t)total_instances);
}

// --------------------------------------
void CRenderMeshInstanced::setSubGroupSize(uint32_t num_subgroup, uint32_t new_size_for_subgroup) {

    assert(num_subgroup < (uint32_t)subgroups.size());
    // Ensure we are not programmed to render more indices/vertexs than our size
    assert(ib ? (new_size_for_subgroup <= num_indices) : (new_size_for_subgroup <= num_vertexs));
    subgroups[num_subgroup].num_indices = new_size_for_subgroup;
}

// -----------------------------------------------------------------
// Configure the two streams and send the mesh to render
void CRenderMeshInstanced::renderSubMesh(uint32_t sub_group_idx) const {

    assert(isValid());
    assert(instanced_mesh);
    assert(instanced_mesh->isValid());
    assert(sub_group_idx < subgroups.size());

    auto& sb = subgroups[sub_group_idx];
    if (sb.num_indices == 0)
        return;

    // Set the source of both streams
    ID3D11Buffer* vbs[2] = {
      instanced_mesh->getVB(),      // 0 -> Instanced mesh
      getVB()                       // 1 -> Instances data
    };

    // Saltos entre vertices para cada stream
    UINT strides[2] = {
      instanced_mesh->getVertexDecl()->bytes_per_vertex,
      getVertexDecl()->bytes_per_vertex
    };

    // Send vertexs from the start of each vertex buffer
    UINT offsets[2] = {
      0,
      0
    };
    Render.ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);

    // Set primitive topology
    Render.ctx->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)instanced_mesh->getTopology());

    // Is Indexed?
    if (instanced_mesh->getIB()) {
        instanced_mesh->activateIndexBuffer();
        Render.ctx->DrawIndexedInstanced(instanced_mesh->getIndicesCount(), sb.num_indices, 0, 0, 0);
    }
    else {
        Render.ctx->DrawInstanced(instanced_mesh->getVertexsCount(), sb.num_indices, 0, 0);
    }

}