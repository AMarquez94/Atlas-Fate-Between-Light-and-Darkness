#include "mcv_platform.h"
#include "mesh_loader.h"
#include "utils/data_provider.h"

bool TMeshLoader::load(CDataProvider& dp) {

  bool eof_found = false;
  while (!eof_found) {

    TChunk chunk;
    dp.read(chunk);

    switch (chunk.magic_id) {

    case magicHeader:
      assert(chunk.num_bytes == sizeof(THeader));
      dp.read(header);
      break;

    case magicVtxs:
      assert(chunk.num_bytes == header.num_vertexs * header.bytes_per_vtx );
      vtxs.resize(chunk.num_bytes);
      dp.readBytes(vtxs.data(), chunk.num_bytes);
      break;

    case magicIdxs:
      assert(chunk.num_bytes == header.num_indices * header.bytes_per_idx);
      idxs.resize(chunk.num_bytes);
      dp.readBytes(idxs.data(), chunk.num_bytes);
      break;

    case magicEoF:
      eof_found = true;
      break;

    default:
      fatal("Unknown chunk data type %08x of %d bytes while reading file %s\n", chunk.magic_id, chunk.num_bytes, dp.getName().c_str() );
      break;
    }
  }

  assert(!idxs.empty());
  assert(!vtxs.empty());
  assert( strlen( header.vertex_type_name ) > 0 );

  return true;
}

CRenderMesh* loadMesh(const char* filename) {
  CFileDataProvider fdp(filename);
  assert(fdp.isValid());

  TMeshLoader loader;
  if (!loader.load(fdp))
    return nullptr;

  CRenderMesh* mesh = new CRenderMesh();
  if (!mesh->create(
      loader.vtxs.data()
    , loader.vtxs.size()
    , loader.header.vertex_type_name
    , loader.header.primitive_type
    , loader.idxs.data()
    , loader.idxs.size()
    , loader.header.bytes_per_idx
  )) 
    return nullptr;

  return mesh;
}
