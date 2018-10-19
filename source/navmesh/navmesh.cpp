#include "mcv_platform.h"
#include "navmesh.h"
#include "navmesh/recast/DetourNavMeshBuilder.h"
#include "navmesh/recast/DetourDebugDraw.h"

CNavmesh::CNavmesh()
    : m_navMesh(nullptr)
{
    m_navQuery = dtAllocNavMeshQuery();
}

void CNavmesh::loadAll(const char* path)
{
    const std::vector<char> &file_data = EngineFiles.loadResourceFile(path);
    int file_pos = 0;

    //FILE* fp = fopen(path, "rb");
    //if (!fp) return;

    // Read header.
    NavMeshSetHeader header;
    memcpy_s(&header, sizeof(NavMeshSetHeader), &file_data[file_pos], sizeof(NavMeshSetHeader));
    file_pos += sizeof(NavMeshSetHeader);

    //size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
    if (header.magic != NAVMESHSET_MAGIC)
    {
        //fclose(fp);
        return;
    }
    if (header.version != NAVMESHSET_VERSION)
    {
        //fclose(fp);
        return;
    }

    dtNavMesh* mesh = dtAllocNavMesh();
    if (!mesh)
    {
        //fclose(fp);
        return;
    }
    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status))
    {
        //fclose(fp);
        return;
    }

    // Read tiles.
    for (int i = 0; i < header.numTiles; ++i)
    {
        NavMeshTileHeader tileHeader;
        memcpy_s(&tileHeader, sizeof(NavMeshTileHeader), &file_data[file_pos], sizeof(NavMeshTileHeader));
        file_pos += sizeof(NavMeshTileHeader);

        //readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
        //if (readLen != 1)
        //{
        //    fclose(fp);
        //    return;
        //}

        if (!tileHeader.tileRef || !tileHeader.dataSize)
            break;

        unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
        if (!data) break;
        memset(data, 0, tileHeader.dataSize);

        memcpy_s(data, tileHeader.dataSize, &file_data[file_pos], tileHeader.dataSize);
        file_pos += tileHeader.dataSize;


        //readLen = fread(data, tileHeader.dataSize, 1, fp);
        //if (readLen != 1)
        //{
        //    dtFree(data);
        //    fclose(fp);
        //    return;
        //}

        mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
    }

    //fclose(fp);

    assert(mesh);
    m_navMesh = mesh;
    m_draw = new DebugDrawGL();
    EngineFiles.addPendingResourceFile(path, false);
}

void CNavmesh::prepareQueries() {
    dtStatus status = m_navQuery->init(m_navMesh, 2048);
    if (dtStatusFailed(status))
        fatal("NAVMESH PREPARATION FAILED!");
}

void CNavmesh::destroy() {
    if (m_navMesh != nullptr) {
        delete[] m_draw;
    }
    dtFreeNavMesh(m_navMesh);
    m_navMesh = 0;
}

void CNavmesh::render()
{
    if (m_navMesh != nullptr) {
        duDebugDrawNavMeshPolysWithFlags(m_draw, *m_navMesh, 0xffff, duRGBA(0, 0, 255, 128));
    }
}
