#include "mcv_platform.h"
#include "mesh.h"
#include "mesh_loader.h"

// ----------------------------------------------
class CRenderMeshResourceClass : public CResourceClass {
public:
  CRenderMeshResourceClass() {
    class_name = "Meshes";
    extension = ".mesh";
  }
  IResource* create(const std::string& name) const override {
    dbg("Creating mesh %s\n", name.c_str());
    CRenderMesh* res = loadMesh(name.c_str());
    return res;
  }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<CRenderMesh>() {
  static CRenderMeshResourceClass the_resource_class;
  return &the_resource_class;
}



bool CRenderMesh::create(
  const void* vertex_data,
  size_t      num_bytes,
  const std::string& vtx_decl_name,
  eTopology   new_topology,
  const void* index_data,
  size_t      num_index_bytes,
  size_t      bytes_per_index
) {
  HRESULT hr;

  assert(vertex_data != nullptr);
  assert(num_bytes > 0);
  assert(new_topology != eTopology::UNDEFINED);

  // Save the parameter
  topology = new_topology;

  // Prepare a struct to create the buffer in gpu memory
  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = (UINT)num_bytes;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  // This is the initial data for the vertexs
  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = vertex_data;
  hr = Render.device->CreateBuffer(&bd, &InitData, &vb);
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
    num_indices = (UINT) (num_index_bytes / bytes_per_index);
    assert(num_indices * bytes_per_index == num_index_bytes);
    index_fmt = ( bytes_per_index == 2 ) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
  }

  vtx_decl = CVertexDeclManager::get().getByName(vtx_decl_name);
  assert(vtx_decl);
  assert(vtx_decl->bytes_per_vertex > 0);

  num_vertexs = (UINT)(num_bytes / vtx_decl->bytes_per_vertex);
  assert(num_vertexs * vtx_decl->bytes_per_vertex == num_bytes);

  return true;
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
  Render.ctx->IASetPrimitiveTopology( (D3D11_PRIMITIVE_TOPOLOGY)topology );

  if (ib) 
    Render.ctx->IASetIndexBuffer(ib, index_fmt, 0);

}

void CRenderMesh::render() const {
  if (ib)
    Render.ctx->DrawIndexed(num_indices, 0, 0);
  else
    Render.ctx->Draw(num_vertexs, 0);
}

void CRenderMesh::activateAndRender() const {
  activate();
  render();
}

void CRenderMesh::debugInMenu() {
  ImGui::Text("%d vertexs", num_vertexs);
  // ...
}

