#ifndef INC_RENDER_MESH_H_
#define INC_RENDER_MESH_H_

#include "resources/resource.h"
#include "mesh_subgroup.h"

class CVertexDecl;

class CRenderMesh : public IResource {

public:

    enum eTopology {
        UNDEFINED = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
        TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
        LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
        TRIANGLE_STRIP = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
        POINT_LIST = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST
    };

    bool create(
        const void* vertex_data,
        size_t      num_bytes,
        const std::string& vtx_decl_name,
        eTopology   new_topology,
        const void* index_data = nullptr,
        size_t      num_index_bytes = 0,
        size_t      bytes_per_index = 0,
        VMeshSubGroups* subgroups = nullptr,
        bool            new_is_dynamic = false
    );
    void destroy() override;
    void activate() const;
    void render() const;
    virtual void renderSubMesh(uint32_t subgroup_idx) const;
    void activateAndRender() const;
    void activateIndexBuffer() const;

    void debugInMenu();
    void setNameAndClass(const std::string& new_name, const CResourceClass* new_class) override;

    virtual const VMeshSubGroups& getSubGroups() const {
        return subgroups;
    }

    const CVertexDecl* getVertexDecl() const { return vtx_decl; }
    const AABB& getAABB() const { return aabb; }

    bool  isValid() const;
    void  updateFromCPU(const void *new_cpu_data, size_t num_bytes_to_update);

    eTopology getTopology() const { return topology; }
    int32_t getVertexsCount() const { return num_vertexs; }
    int32_t getIndicesCount() const { return num_indices; }
    ID3D11Buffer* getVB() const { return vb; }
    ID3D11Buffer* getIB() const { return ib; }

public:

    ID3D11Buffer * vb = nullptr;
    ID3D11Buffer*      ib = nullptr;      // index buffer
    const CVertexDecl* vtx_decl = nullptr;
    eTopology          topology = eTopology::UNDEFINED;
    UINT               num_vertexs = 0;
    UINT               num_indices = 0;
    DXGI_FORMAT        index_fmt = DXGI_FORMAT_UNKNOWN;
    AABB               aabb;
    VMeshSubGroups     subgroups;
    bool               is_dynamic = false;

};

#endif

