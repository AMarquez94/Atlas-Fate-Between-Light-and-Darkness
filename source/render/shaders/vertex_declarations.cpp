#include "mcv_platform.h"

static CVertexDeclManager vtx_decl_manager;

CVertexDeclManager& CVertexDeclManager::get() {
  return vtx_decl_manager;
}

// ----------------------------------------------------------
CVertexDecl* CVertexDeclManager::createNew(
  const std::string& name
  , D3D11_INPUT_ELEMENT_DESC* layout
  , UINT numElements
) {
  CVertexDecl* decl = new CVertexDecl();
  decl->name = name; 
  decl->cpu_layout = layout;
  decl->numElements = numElements;
  decls[name] = decl;

  // Automatically try to find the bytes per vertex based on the 
  // size of each element in the vertex layout
  UINT total_bytes = 0;
  for (UINT i = 0; i < numElements; ++i) {
    auto d = layout + i;
    switch (d->Format) {
    case DXGI_FORMAT_R32G32B32A32_FLOAT: total_bytes += 4 * 4; break;
    case DXGI_FORMAT_R32G32B32_FLOAT: total_bytes += 3 * 4; break;
    case DXGI_FORMAT_R32G32_FLOAT: total_bytes += 2 * 4; break;
    default:
      fatal("Unknown size of vertex element %08x while declaring vtx decl %s.%s\n", d->Format, name.c_str(), d->SemanticName);
    }
  }
  decl->bytes_per_vertex = total_bytes;

  return decl;
}

// ----------------------------------------------------------
// Create & register each vertex declr
bool CVertexDeclManager::create() {

  {
    static D3D11_INPUT_ELEMENT_DESC layout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    createNew("Pos", layout, ARRAYSIZE(layout));
  }

  {
    static D3D11_INPUT_ELEMENT_DESC layout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    createNew("PosClr", layout, ARRAYSIZE(layout));
  }

  {
    static D3D11_INPUT_ELEMENT_DESC layout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    createNew("PosNUv", layout, ARRAYSIZE(layout));
  }

  return true;
}

const CVertexDecl* CVertexDeclManager::getByName(const std::string& name) {
  auto it = decls.find(name);
  if (it == decls.end()) {
    fatal("Can't find vertex declaration named '%s'", name.c_str());
    return nullptr;
  }
  return it->second;
}


