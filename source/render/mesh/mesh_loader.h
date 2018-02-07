#pragma once

class CDataProvider;
class CRenderMesh;

struct TMeshLoader {

  struct TChunk {
    uint32_t magic_id;
    uint32_t num_bytes;
  };

  // ----------------------
  static const uint32_t magicHeader = 0x44444444;
  static const uint32_t magicVtxs = 0x55554433;
  static const uint32_t magicIdxs = 0x55556677;
  static const uint32_t magicEoF = 0x55558888;

  // ----------------------
  struct THeader {
    uint32_t num_vertexs = 0;
    uint32_t num_indices = 0;
    CRenderMesh::eTopology primitive_type = CRenderMesh::UNDEFINED;
    uint32_t bytes_per_vtx = 0;
    uint32_t bytes_per_idx = 0;
    uint32_t dummy1 = 0;
    uint32_t dummy2 = 0;
    uint32_t dummy3 = 0;
    char     vertex_type_name[32];
  };

  THeader                header;
  std::vector< uint8_t > vtxs;
  std::vector< uint8_t > idxs;

  bool load(CDataProvider& dp);

};

CRenderMesh* loadMesh(const char* filename);