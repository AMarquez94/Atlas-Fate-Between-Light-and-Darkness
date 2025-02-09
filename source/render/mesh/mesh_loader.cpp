#include "mcv_platform.h"
#include "mesh_loader.h"
#include "utils/data_provider.h"
#include "physics/physics_mesh.h"
#include "utils/data_saver.h"

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
			assert(chunk.num_bytes == header.num_vertexs * header.bytes_per_vtx);
			vtxs.resize(chunk.num_bytes);
			dp.readBytes(vtxs.data(), chunk.num_bytes);
			break;

		case magicIdxs:
			assert(chunk.num_bytes == header.num_indices * header.bytes_per_idx);
			idxs.resize(chunk.num_bytes);
			dp.readBytes(idxs.data(), chunk.num_bytes);
			break;

		case magicSubGroups: {
			static const int bytes_per_subgroup = sizeof(TMeshSubGroup);
			assert(chunk.num_bytes == header.num_subgroups * bytes_per_subgroup);
			subgroups.resize(header.num_subgroups);
			dp.readBytes(subgroups.data(), chunk.num_bytes);
			break; }

		case magicEoF:
			eof_found = true;
			break;

		default:
			fatal("Unknown chunk data type %08x of %d bytes while reading file %s\n", chunk.magic_id, chunk.num_bytes, dp.getName().c_str());
			break;
		}
	}

	assert(!idxs.empty());
	assert(!vtxs.empty());
	assert(strlen(header.vertex_type_name) > 0);

	// In case the mesh is old...
	if (subgroups.empty())
		subgroups.push_back({ 0, header.num_indices, 0, 0 });

	return true;
}

bool TMeshLoader::load(const std::string & filename)
{
    const std::vector<char> &data = EngineFiles.loadResourceFile(filename);
    int file_pos = 0;

    bool eof_found = false;
    while (!eof_found) {

        TChunk chunk;
        memcpy_s(&chunk, sizeof(TChunk), &data[file_pos], sizeof(TChunk));
        file_pos += sizeof(TChunk);

        switch (chunk.magic_id) {

        case magicHeader:
            assert(chunk.num_bytes == sizeof(THeader));
            memcpy_s(&header, sizeof(THeader), &data[file_pos], sizeof(THeader));
            file_pos += sizeof(THeader);
            //dp.read(header);
            break;

        case magicVtxs:
            assert(chunk.num_bytes == header.num_vertexs * header.bytes_per_vtx);
            vtxs.resize(chunk.num_bytes);
            memcpy_s(vtxs.data(), chunk.num_bytes, &data[file_pos], chunk.num_bytes);
            file_pos += chunk.num_bytes;
            //dp.readBytes(vtxs.data(), chunk.num_bytes);
            break;

        case magicIdxs:
            assert(chunk.num_bytes == header.num_indices * header.bytes_per_idx);
            idxs.resize(chunk.num_bytes);
            memcpy_s(idxs.data(), chunk.num_bytes, &data[file_pos], chunk.num_bytes);
            file_pos += chunk.num_bytes;
            //dp.readBytes(idxs.data(), chunk.num_bytes);
            break;

        case magicSubGroups: {
            static const int bytes_per_subgroup = sizeof(TMeshSubGroup);
            assert(chunk.num_bytes == header.num_subgroups * bytes_per_subgroup);
            subgroups.resize(header.num_subgroups);
            memcpy_s(subgroups.data(), chunk.num_bytes, &data[file_pos], chunk.num_bytes);
            file_pos += chunk.num_bytes;
            //dp.readBytes(subgroups.data(), chunk.num_bytes);
            break; }

        case magicEoF:
            eof_found = true;
            break;

        default:
            fatal("Unknown chunk data type %08x of %d bytes while reading file %s\n", chunk.magic_id, chunk.num_bytes, filename.c_str());
            break;
        }
    }

    assert(!idxs.empty());
    assert(!vtxs.empty());
    assert(strlen(header.vertex_type_name) > 0);

    // In case the mesh is old...
    if (subgroups.empty())
        subgroups.push_back({ 0, header.num_indices, 0, 0 });

    EngineFiles.addPendingResourceFile(filename, false);

    return true;
}

void saveChunk(CDataSaver& ds, uint32_t magic, uint32_t num_bytes, const void* data) {
	TMeshLoader::TChunk s;
	s.magic_id = magic;
	s.num_bytes = num_bytes;
	ds.write(s);
	if (data)
		ds.writeBytes(data, num_bytes);
}

bool TMeshLoader::save(CDataSaver& ds) {
	assert(ds.isValid());
	saveChunk(ds, magicHeader, sizeof(THeader), &header);
	saveChunk(ds, magicVtxs, (uint32_t)vtxs.size(), vtxs.data());
	saveChunk(ds, magicIdxs, (uint32_t)idxs.size(), idxs.data());
	saveChunk(ds, magicEoF, 0, nullptr);
	return true;
}


CRenderMesh* loadMesh(const char* filename) {
	//CFileDataProvider fdp(filename);
	//assert(fdp.isValid());

	TMeshLoader loader;
	if (!loader.load(filename))
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
		, &loader.subgroups
	))
		return nullptr;

    EngineFiles.addPendingResourceFile(filename, false);

	return mesh;
}

CPhysicsMesh* loadPhysicsMesh(const char* filename) {
	//CFileDataProvider fdp(filename);
	//assert(fdp.isValid());

	TMeshLoader loader;
	if (!loader.load(filename))
		return nullptr;

	CPhysicsMesh * physx_mesh = new CPhysicsMesh();
	if (!physx_mesh->create(
		loader.vtxs.data()
		, loader.idxs.data()
		, loader.vtxs.size()
		, loader.idxs.size()
		, loader.header.bytes_per_vtx
		, loader.header.bytes_per_idx
	))
		return nullptr;

	return physx_mesh;
}

TMeshLoader* loadCollider(const char* filename) {
	//CFileDataProvider fdp(filename);
	//assert(fdp.isValid());

	TMeshLoader* loader = new TMeshLoader();
	if (!loader->load(filename)) {
		return nullptr;
	}
	return loader;
}