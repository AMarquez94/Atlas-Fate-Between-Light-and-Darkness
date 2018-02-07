#ifndef INC_RENDER_MESH_H_
#define INC_RENDER_MESH_H_

#include "resources/resource.h"

class CVertexDecl;

class CRenderMesh : public IResource {

public:

  enum eTopology {
    UNDEFINED = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
  };

  bool create(
    const void* vertex_data,
    size_t      num_bytes,
    const std::string& vtx_decl_name,
    eTopology   new_topology,
    const void* index_data = nullptr,
    size_t      num_index_bytes = 0,
    size_t      bytes_per_index = 0 
  );
  void destroy() override;
  void activate() const;
  void render() const;
  void activateAndRender() const;

  void debugInMenu();

private:
  
  ID3D11Buffer*      vb = nullptr;
  ID3D11Buffer*      ib = nullptr;      // index buffer
  const CVertexDecl* vtx_decl = nullptr;
  eTopology          topology = eTopology::UNDEFINED;
  UINT               num_vertexs = 0;
  UINT               num_indices = 0;
  DXGI_FORMAT        index_fmt = DXGI_FORMAT_UNKNOWN;
};


#endif

