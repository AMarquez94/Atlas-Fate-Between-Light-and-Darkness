#include "mcv_platform.h"
#include "mesh.h"
#include "mesh_loader.h"
#include "render/shaders/render_technique.h"
#include "render/shaders/vertex_shader.h"

CRenderMesh* loadMeshInstanced(const std::string& name);

// ----------------------------------------------
class CRenderMeshResourceClass : public CResourceClass {
public:
    CRenderMeshResourceClass() {
        class_name = "Meshes";
        extensions = { ".mesh", ".instanced_mesh" };
    }
    IResource* create(const std::string& name) const override {
        if (name.find(".instanced_mesh") != std::string::npos)
            return loadMeshInstanced(name);
        dbg("Creating mesh %s\n", name.c_str());
        return loadMesh(name.c_str());
    }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CRenderMesh>() {
    static CRenderMeshResourceClass the_resource_class;
    return &the_resource_class;
}

void CRenderMesh::setNameAndClass(const std::string& new_name, const CResourceClass* new_class) {
    IResource::setNameAndClass(new_name, new_class);
    // Use the name to assign the same name to the DX objects
    setDXName(vb, new_name.c_str());
    if (ib)
        setDXName(ib, new_name.c_str());
}

bool CRenderMesh::create(
    const void* vertex_data,
    size_t      num_bytes,
    const std::string& vtx_decl_name,
    eTopology   new_topology,
    const void* index_data,
    size_t      num_index_bytes,
    size_t      bytes_per_index,
    VMeshSubGroups* new_subgroups,
    bool            new_is_dynamic
) {
    HRESULT hr;

    // Dynamic meshes are allowed to NOT provide initial vertex data
    assert(vertex_data != nullptr || new_is_dynamic);
    assert(num_bytes > 0);
    assert(new_topology != eTopology::UNDEFINED);

    // Save the parameter
    topology = new_topology;
    is_dynamic = new_is_dynamic;

    // Prepare a struct to create the buffer in gpu memory
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = (UINT)num_bytes;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    // Because we want to write from the CPU and to modify it (write, not read)
    if (is_dynamic) {
        bd.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
        bd.Usage = D3D11_USAGE_DYNAMIC;
    }

    // Check if we have initial data for the vertexs
    const D3D11_SUBRESOURCE_DATA* init_data_ptr = nullptr;
    D3D11_SUBRESOURCE_DATA InitData;
    if (vertex_data) {
        ZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = vertex_data;
        init_data_ptr = &InitData;
    }

    // Create the Vertex Buffer
    hr = Render.device->CreateBuffer(&bd, init_data_ptr, &vb);
    if (FAILED(hr))
        return false;

    // -----------------------------------------------
    // Prepare a struct to create the index buffer in gpu memory
    if (num_index_bytes > 0) {
        assert(bytes_per_index == 2 || bytes_per_index == 4);
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = (UINT)num_index_bytes;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        // This is the initial data for the indices
        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory(&InitData, sizeof(InitData));
        InitData.pSysMem = index_data;
        hr = Render.device->CreateBuffer(&bd, &InitData, &ib);
        if (FAILED(hr))
            return false;

        // Deduce the number of indices based on the index buffer size and the bytes per index
        num_indices = (UINT)(num_index_bytes / bytes_per_index);
        assert(num_indices * bytes_per_index == num_index_bytes);
        index_fmt = (bytes_per_index == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    }

    vtx_decl = CVertexDeclManager::get().getByName(vtx_decl_name);
    assert(vtx_decl);
    assert(vtx_decl->bytes_per_vertex > 0);

    num_vertexs = (UINT)(num_bytes / vtx_decl->bytes_per_vertex);
    assert(num_vertexs * vtx_decl->bytes_per_vertex == num_bytes);

    // Save group information if given
    if (new_subgroups) {
        subgroups = *new_subgroups;
    }
    else {
        if (num_indices > 0)
            subgroups.push_back({ 0, num_indices, 0, 0 });
        else
            subgroups.push_back({ 0, num_vertexs, 0, 0 });
    }

    // Recompute aabb
    if (!is_dynamic)
        AABB::CreateFromPoints(aabb, num_vertexs, (const VEC3*)vertex_data, vtx_decl->bytes_per_vertex);

    return true;
}

bool CRenderMesh::isValid() const {

    return vb != nullptr && vtx_decl != nullptr;
}

void CRenderMesh::destroy() {

    SAFE_RELEASE(vb);
    SAFE_RELEASE(ib);
}

void CRenderMesh::activate() const {

    assert(vb);
    assert(vtx_decl);

    // Set vertex buffer based on my vertex type
    UINT stride = vtx_decl->bytes_per_vertex;
    UINT offset = 0;
    Render.ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

    // Set primitive topology
    Render.ctx->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);

    activateIndexBuffer();
}

void CRenderMesh::render() const {

    assert(CRenderTechnique::current);
    assert(CRenderTechnique::current->vs);
    assert(vtx_decl);
    assert((CRenderTechnique::current->vs->getVertexDecl() == vtx_decl)
        || fatal("Current tech %s expect vertex decl %s, but this mesh uses %s\n"
            , CRenderTechnique::current->getName().c_str()
            , CRenderTechnique::current->vs->getVertexDecl()->name.c_str()
            , vtx_decl->name.c_str()
        ));

    if (ib)
        Render.ctx->DrawIndexed(num_indices, 0, 0);
    else
        Render.ctx->Draw(num_vertexs, 0);
}

void CRenderMesh::renderSubMesh(uint32_t subgroup_idx) const {

    assert(subgroup_idx < subgroups.size());
    auto& g = subgroups[subgroup_idx];
    if (ib)
        Render.ctx->DrawIndexed(g.num_indices, g.first_idx, 0);
    else
        Render.ctx->Draw(g.num_indices, g.first_idx);
}

void CRenderMesh::activateAndRender() const {

    activate();
    render();
}

void CRenderMesh::activateIndexBuffer() const {

    if (ib)
        Render.ctx->IASetIndexBuffer(ib, index_fmt, 0);
}

void CRenderMesh::debugInMenu() {

    ImGui::Text("%d vertexs", num_vertexs);
    if (ib)
        ImGui::Text("%d Indices", num_indices);
    if (is_dynamic)
        ImGui::Text("Dynamic");
    ImGui::Text("Vtx Size %d : %s", vtx_decl->bytes_per_vertex, vtx_decl->name.c_str());
}

// --------------------------------------
void CRenderMesh::updateFromCPU(const void *new_cpu_data, size_t num_bytes_to_update) {

    assert(is_dynamic);
    assert(new_cpu_data != nullptr);

    // If no bytes are given, update the whole buffer
    if (num_bytes_to_update == 0)
        num_bytes_to_update = num_vertexs * vtx_decl->bytes_per_vertex;

    // Don't copy beyond the vb capacity
    assert(num_bytes_to_update <= num_vertexs * vtx_decl->bytes_per_vertex);

    D3D11_MAPPED_SUBRESOURCE mapped_resource;

    // Get CPU access to the GPU buffer
    HRESULT hr = Render.ctx->Map(
        vb
        , 0     // Vertex buffers only have one subresource. In case of texture, each mipmap is a subresource
        , D3D11_MAP_WRITE_DISCARD     // 
        , 0
        , &mapped_resource);
    assert(hr == D3D_OK);

    // Copy from CPU to GPU
    memcpy(mapped_resource.pData, new_cpu_data, num_bytes_to_update);

    // Close the map
    Render.ctx->Unmap(vb, 0);
}
